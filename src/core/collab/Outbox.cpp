#include "core/collab/Outbox.h"

#include "core/SafeSave.h"
#include "core/collab/proto/Messages.h"
#include "core/crypto/Vault.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <algorithm>

namespace qb {
namespace collab {

namespace {
QJsonObject toObject(const OutboxEntry &e)
{
    QJsonArray blobs;
    for (const QString &b : e.blobs)
        blobs.append(b);
    return QJsonObject{{QStringLiteral("opId"), e.opId},         {QStringLiteral("order"), double(e.order)}, {QStringLiteral("units"), e.units},
                       {QStringLiteral("blobs"), blobs},         {QStringLiteral("created"), double(e.createdMs)},
                       {QStringLiteral("reason"), e.reason}};
}

bool fromObject(const QJsonObject &o, OutboxEntry *e)
{
    if (!o.value(QStringLiteral("opId")).isString() || !o.value(QStringLiteral("units")).isArray() || !o.value(QStringLiteral("blobs")).isArray())
        return false;
    e->opId = o.value(QStringLiteral("opId")).toString();
    e->order = qint64(o.value(QStringLiteral("order")).toDouble());
    e->units = o.value(QStringLiteral("units")).toArray();
    e->blobs.clear();
    for (const QJsonValue &v : o.value(QStringLiteral("blobs")).toArray())
        e->blobs << v.toString();
    e->createdMs = qint64(o.value(QStringLiteral("created")).toDouble());
    e->reason = o.value(QStringLiteral("reason")).toString();
    return !e->opId.isEmpty();
}
} // namespace

Outbox::Outbox(const QString &root) : m_root(root), m_blobs(root + QStringLiteral("/blobs")) {}

bool Outbox::write(const QString &path, const OutboxEntry &e, QString *error)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    return safesave::writeFile(path, QJsonDocument(toObject(e)).toJson(QJsonDocument::Compact), error);
}

bool Outbox::load(QString *error)
{
    m_pending.clear();
    m_drafts.clear();
    QDir().mkpath(m_root + QStringLiteral("/outbox"));
    QDir().mkpath(m_root + QStringLiteral("/drafts"));
    for (const QString &name : QDir(m_root + QStringLiteral("/outbox")).entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name)) {
        const QString path = m_root + QStringLiteral("/outbox/") + name;
        QByteArray raw;
        OutboxEntry e;
        if (vault::readFile(path, &raw) && fromObject(QJsonDocument::fromJson(raw).object(), &e)) {
            m_pending << e;
            m_nextOrder = qMax(m_nextOrder, e.order + 1);
            continue;
        }
        // a damaged entry is kept, set aside among the drafts under its file name
        e = OutboxEntry();
        e.opId = QFileInfo(name).completeBaseName();
        e.reason = QStringLiteral("DAMAGED");
        QFile::rename(path, m_root + QStringLiteral("/drafts/") + name);
    }
    for (const QString &name : QDir(m_root + QStringLiteral("/drafts")).entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name)) {
        QByteArray raw;
        OutboxEntry e;
        if (vault::readFile(m_root + QStringLiteral("/drafts/") + name, &raw) && fromObject(QJsonDocument::fromJson(raw).object(), &e))
            m_drafts << e;
    }
    std::sort(m_pending.begin(), m_pending.end(), [](const OutboxEntry &a, const OutboxEntry &b) { return a.order < b.order; });
    Q_UNUSED(error);
    return true;
}

QString Outbox::add(const QJsonArray &units, const QHash<QString, QByteArray> &byHash, qint64 createdMs, QString *error, const QString &id)
{
    OutboxEntry e;
    e.opId = id.isEmpty() ? proto::newId() : id;
    e.order = m_nextOrder;
    e.units = units;
    e.createdMs = createdMs;
    for (auto it = byHash.constBegin(); it != byHash.constEnd(); ++it) {
        const QString h = m_blobs.put(it.value(), error);
        if (h.isEmpty())
            return QString();
        if (h != it.key()) {
            if (error)
                *error = QStringLiteral("a blob does not match its hash");
            return QString();
        }
        e.blobs << h;
    }
    const QString path = QStringLiteral("%1/outbox/%2-%3.json").arg(m_root).arg(e.order, 12, 10, QLatin1Char('0')).arg(e.opId);
    if (!write(path, e, error))
        return QString();
    ++m_nextOrder;
    m_pending << e;
    return e.opId;
}

const OutboxEntry *Outbox::find(const QString &opId) const
{
    for (const OutboxEntry &e : m_pending)
        if (e.opId == opId)
            return &e;
    return nullptr;
}

bool Outbox::updateUnits(const QString &opId, const QJsonArray &units, QString *error)
{
    for (OutboxEntry &e : m_pending)
        if (e.opId == opId) {
            OutboxEntry n = e;
            n.units = units;
            const QString path = QStringLiteral("%1/outbox/%2-%3.json").arg(m_root).arg(e.order, 12, 10, QLatin1Char('0')).arg(e.opId);
            if (!write(path, n, error))
                return false;
            e = n;
            return true;
        }
    return false;
}

bool Outbox::referenced(const QString &hash) const
{
    for (const OutboxEntry &e : m_pending)
        if (e.blobs.contains(hash))
            return true;
    for (const OutboxEntry &e : m_drafts)
        if (e.blobs.contains(hash))
            return true;
    return false;
}

void Outbox::collectBlobs()
{
    QSet<QString> used;
    for (const OutboxEntry &e : m_pending)
        for (const QString &h : e.blobs)
            used.insert(h);
    for (const OutboxEntry &e : m_drafts)
        for (const QString &h : e.blobs)
            used.insert(h);
    // only the blobs that an entry once named are ours to remove: the folder also holds blobs downloaded for the replica,
    // which their session removes itself
    for (const QString &h : m_released)
        if (!used.contains(h))
            m_blobs.remove(h);
    m_released.clear();
}

bool Outbox::done(const QString &opId)
{
    for (int i = 0; i < m_pending.size(); ++i)
        if (m_pending.at(i).opId == opId) {
            const OutboxEntry e = m_pending.takeAt(i);
            const QString path = QStringLiteral("%1/outbox/%2-%3.json").arg(m_root).arg(e.order, 12, 10, QLatin1Char('0')).arg(e.opId);
            safesave::removeFile(path);
            m_released += e.blobs;
            collectBlobs();
            return true;
        }
    return false;
}

bool Outbox::toDraft(const QString &opId, const QString &reason, QString *error)
{
    for (int i = 0; i < m_pending.size(); ++i)
        if (m_pending.at(i).opId == opId) {
            OutboxEntry e = m_pending.at(i);
            e.reason = reason;
            if (!write(m_root + QStringLiteral("/drafts/") + e.opId + QStringLiteral(".json"), e, error))
                return false;
            m_pending.removeAt(i);
            safesave::removeFile(QStringLiteral("%1/outbox/%2-%3.json").arg(m_root).arg(e.order, 12, 10, QLatin1Char('0')).arg(e.opId));
            m_drafts << e;
            return true;
        }
    return false;
}

bool Outbox::removeDraft(const QString &opId)
{
    for (int i = 0; i < m_drafts.size(); ++i)
        if (m_drafts.at(i).opId == opId) {
            const OutboxEntry e = m_drafts.takeAt(i);
            safesave::removeFile(m_root + QStringLiteral("/drafts/") + opId + QStringLiteral(".json"));
            m_released += e.blobs;
            collectBlobs();
            return true;
        }
    return false;
}

} // namespace collab
} // namespace qb
