#include "core/collab/ServiceClient.h"

#include "core/collab/ServiceProtocol.h"
#include "core/collab/net/MemLink.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QPointer>
#include <QUuid>

namespace qb {
namespace collab {

using net::Frame;

ServiceClient::ServiceClient(net::Link *link, QObject *parent) : QObject(parent), m_link(link)
{
    link->setParent(this);
    connect(link, &net::Link::frameReceived, this, &ServiceClient::onFrame);
    connect(link, &net::Link::closed, this, [this](const QString &) {
        m_gone = true;
        m_follower.setReadOnly(true, tr("the connection to the Live Sync service was lost"));
        emit serviceGone();
        emit stateChanged();
    });
}

ServiceClient::~ServiceClient()
{
    if (m_link && m_link->state() != net::Link::State::Closed) {
        // the service's end is told at once (an in-memory link would tell it later, when this end is gone), so that it lets go of
        // this window
        if (auto *mem = qobject_cast<net::MemLink *>(m_link))
            mem->setSynchronous(true);
        m_link->close();
    }
}

bool ServiceClient::linkAlive() const { return !m_gone && m_link && m_link->state() == net::Link::State::Ready; }

QString ServiceClient::nextId() { return QString::number(++m_counter) + QLatin1Char('-') + QUuid::createUuid().toString(QUuid::Id128).left(8); }

void ServiceClient::sendMsg(const QJsonObject &m)
{
    if (m_link)
        m_link->send(Frame::json(m));
}

ServiceClient::Attached ServiceClient::attach(const QString &folder, bool keepHosting, int timeoutMs)
{
    Attached a;
    m_folder = QFileInfo(folder).absoluteFilePath();
    const QString id = nextId();
    sendMsg(local::msg(local::ATTACH, {{"id", id}, {"folder", m_folder}, {"keepHosting", keepHosting}}));
    QElapsedTimer t;
    t.start();
    while (!m_answers.contains(id) && t.elapsed() < timeoutMs && !m_gone) {
        QEventLoop l;
        QTimer::singleShot(10, &l, &QEventLoop::quit);
        l.exec();
    }
    if (!m_answers.contains(id)) {
        a.code = m_gone ? QStringLiteral("LINK") : QStringLiteral("TIMEOUT");
        return a;
    }
    const QJsonObject r = m_answers.take(id);
    a.ok = r.value(QStringLiteral("ok")).toBool();
    a.code = r.value(QStringLiteral("code")).toString();
    a.text = r.value(QStringLiteral("text")).toString();
    if (a.ok) {
        m_uuid = r.value(QStringLiteral("uuid")).toString();
        applyState(r);
        a.uuid = m_uuid;
        a.title = r.value(QStringLiteral("title")).toString();
        a.hostName = m_hostName;
        a.host = m_host;
        a.ready = m_ready;
    }
    return a;
}

void ServiceClient::detach()
{
    if (!m_uuid.isEmpty())
        sendMsg(local::msg(local::DETACH, {{"uuid", m_uuid}}));
    m_uuid.clear();
}

void ServiceClient::applyState(const QJsonObject &m)
{
    m_host = m.value(QStringLiteral("host")).toBool();
    m_hostName = m.value(QStringLiteral("hostName")).toString();
    m_state = m.value(QStringLiteral("state")).toInt();
    m_ready = m.value(QStringLiteral("ready")).toBool();
    m_readOnly = m.value(QStringLiteral("readOnly")).toBool();
    m_readOnlyReason = m.value(QStringLiteral("readOnlyReason")).toString();
    m_pending = m.value(QStringLiteral("pending")).toInt();
    m_drafts = m.value(QStringLiteral("drafts")).toInt();
    m_seq = qint64(m.value(QStringLiteral("seq")).toDouble());
    m_epoch = qint64(m.value(QStringLiteral("epoch")).toDouble());
    m_hostId = m.value(QStringLiteral("hostDeviceId")).toString();
    QHash<QString, QString> held;
    const QJsonObject h = m.value(QStringLiteral("held")).toObject();
    for (auto it = h.constBegin(); it != h.constEnd(); ++it)
        held.insert(it.key(), it.value().toString());
    // the follower's picture of locks of others
    for (auto it = m_held.constBegin(); it != m_held.constEnd(); ++it)
        if (!held.contains(it.key()))
            m_follower.clearHeld(it.key());
    for (auto it = held.constBegin(); it != held.constEnd(); ++it)
        m_follower.setHeldElsewhere(it.key(), it.value());
    m_held = held;
    m_unreachable.clear();
    for (const QJsonValue &v : m.value(QStringLiteral("unreachable")).toArray())
        m_unreachable.insert(v.toString());
    QStringList mine;
    for (const QJsonValue &v : m.value(QStringLiteral("mine")).toArray())
        mine << v.toString();
    m_follower.setMine(mine);
    m_follower.setReadOnly(m_readOnly, m_readOnlyReason);
}

void ServiceClient::onFrame(const Frame &f)
{
    bool ok = false;
    const QJsonObject m = f.kind == Frame::Json ? f.object(&ok) : QJsonObject();
    if (!ok)
        return;
    const QString t = local::typeOf(m);
    const QString uuid = m.value(QStringLiteral("uuid")).toString();
    if (t == QLatin1String(local::SUBMITTED)) {
        m_submitIds.remove(m.value(QStringLiteral("re")).toString());
        if (m.value(QStringLiteral("ok")).toBool())
            m_open << m.value(QStringLiteral("opId")).toString();
        else {
            m_error = m.value(QStringLiteral("error")).toString();
            emit problem(QString(), m_error);
        }
        return;
    }
    if (t == QLatin1String(local::ATTACHED) || t == QLatin1String(local::PONG)) {
        m_answers.insert(m.value(QStringLiteral("re")).toString(), m);
        return;
    }
    if (t == QLatin1String(local::QUIT)) {
        emit quitRequested();
        return;
    }
    if (uuid != m_uuid)
        return;
    if (t == QLatin1String(local::STATE)) {
        applyState(m);
        emit stateChanged();
    } else if (t == QLatin1String(local::EVENT)) {
        QStringList paths;
        for (const QJsonValue &v : m.value(QStringLiteral("paths")).toArray())
            paths << v.toString();
        emit eventApplied(qint64(m.value(QStringLiteral("seq")).toDouble()), m.value(QStringLiteral("opId")).toString(),
                          m.value(QStringLiteral("actor")).toString(), m.value(QStringLiteral("own")).toBool(),
                          m.value(QStringLiteral("ops")).toArray(), paths, m.value(QStringLiteral("revs")).toObject());
    } else if (t == QLatin1String(local::REPLACED)) {
        emit replicaReplaced();
    } else if (t == QLatin1String(local::RESULT)) {
        const QString opId = m.value(QStringLiteral("opId")).toString();
        m_open.removeAll(opId);
        const QString kind = m.value(QStringLiteral("kind")).toString();
        if (kind == QLatin1String("committed"))
            emit committed(opId, qint64(m.value(QStringLiteral("seq")).toDouble()));
        else if (kind == QLatin1String("rejected"))
            emit rejected(opId, m.value(QStringLiteral("rejects")).toArray());
        else if (kind == QLatin1String("draft"))
            emit draftKept(opId, m.value(QStringLiteral("reason")).toString());
        else if (kind == QLatin1String("problem"))
            emit problem(opId, m.value(QStringLiteral("code")).toString());
    } else if (t == QLatin1String(local::LOCK_EVT)) {
        const QString kind = m.value(QStringLiteral("kind")).toString();
        const QString r = m.value(QStringLiteral("resource")).toString();
        if (kind == QLatin1String("granted"))
            emit lockGranted(r);
        else if (kind == QLatin1String("denied"))
            emit lockDenied(r, m.value(QStringLiteral("holder")).toString(), m.value(QStringLiteral("reason")).toString());
        else if (kind == QLatin1String("revoked"))
            emit lockRevoked(r, m.value(QStringLiteral("reason")).toString());
    } else if (t == QLatin1String(local::EDIT_EVT)) {
        const QString kind = m.value(QStringLiteral("kind")).toString();
        const QString r = m.value(QStringLiteral("resource")).toString();
        if (kind == QLatin1String("asked"))
            emit editAsked(r, m.value(QStringLiteral("asker")).toString(), qint64(m.value(QStringLiteral("waitMs")).toDouble()));
        else
            emit editStatus(r, m.value(QStringLiteral("declined")).toBool(), qint64(m.value(QStringLiteral("waitMs")).toDouble()),
                            m.value(QStringLiteral("mayForce")).toBool(), m.value(QStringLiteral("holder")).toString());
    }
}

int ServiceClient::sendCaptured()
{
    m_error.clear();
    if (m_uuid.isEmpty() || !m_link)
        return 0;
    int made = 0;
    for (const ChangeSet &set : m_follower.capture().takeReady()) {
        const UnitsResult ur = unitsFromChanges(set, m_folder);
        if (!ur.ok) {
            m_error = ur.error;
            continue;
        }
        if (ur.isEmpty())
            continue;
        QJsonArray units, blobs;
        for (const proto::Unit &u : ur.units)
            units.append(proto::toJson(u));
        for (auto it = ur.blobs.constBegin(); it != ur.blobs.constEnd(); ++it)
            blobs.append(QJsonObject{{"hash", it.key()}, {"size", double(it.value().size())}});
        const QString id = nextId();
        sendMsg(local::msg(local::SUBMIT, {{"id", id}, {"uuid", m_uuid}, {"units", units}, {"blobs", blobs}}));
        for (auto it = ur.blobs.constBegin(); it != ur.blobs.constEnd(); ++it) {
            const QByteArray tid = local::transferOfHash(it.key());
            const QByteArray &d = it.value();
            qint64 off = 0;
            do {
                const QByteArray chunk = d.mid(off, net::limits::kMaxDataBytes);
                m_link->send(Frame::data(tid, quint64(off), chunk));
                off += chunk.size();
            } while (off < d.size());
        }
        m_submitIds.insert(id);
        ++made;
    }
    return made;
}

bool ServiceClient::flush(int timeoutMs)
{
    QElapsedTimer t;
    t.start();
    while ((!m_open.isEmpty() || !m_submitIds.isEmpty()) && t.elapsed() < timeoutMs && !m_gone) {
        QEventLoop l;
        QTimer::singleShot(10, &l, &QEventLoop::quit);
        l.exec();
    }
    return m_open.isEmpty() && m_submitIds.isEmpty();
}

void ServiceClient::lock(const QStringList &r) { sendMsg(local::msg(local::LOCK, {{"uuid", m_uuid}, {"resources", QJsonArray::fromStringList(r)}})); }
void ServiceClient::unlock(const QStringList &r) { sendMsg(local::msg(local::UNLOCK, {{"uuid", m_uuid}, {"resources", QJsonArray::fromStringList(r)}})); }
void ServiceClient::askToEdit(const QString &r) { sendMsg(local::msg(local::ASK, {{"uuid", m_uuid}, {"resource", r}})); }
void ServiceClient::cancelAsk(const QString &r) { sendMsg(local::msg(local::CANCEL_ASK, {{"uuid", m_uuid}, {"resource", r}})); }
void ServiceClient::takeOver(const QString &r) { sendMsg(local::msg(local::TAKEOVER, {{"uuid", m_uuid}, {"resource", r}})); }
void ServiceClient::answerEdit(const QString &r, bool handOver, const QString &asker)
{
    sendMsg(local::msg(local::ANSWER, {{"uuid", m_uuid}, {"resource", r}, {"handOver", handOver}, {"asker", asker}}));
}
void ServiceClient::requestBackup() { sendMsg(local::msg(local::ACTION, {{"uuid", m_uuid}, {"action", "requestBackup"}})); }
void ServiceClient::transferHosting(const QString &targetDeviceId)
{
    sendMsg(local::msg(local::ACTION, {{"uuid", m_uuid}, {"action", "transferHosting"}, {"target", targetDeviceId}}));
}
void ServiceClient::stopSharing() { sendMsg(local::msg(local::ACTION, {{"uuid", m_uuid}, {"action", "stopSharing"}})); }

} // namespace collab
} // namespace qb
