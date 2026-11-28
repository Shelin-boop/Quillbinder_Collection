// The paired devices: see PeerStore.h.
#include "core/collab/net/PeerStore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace qb {
namespace collab {
namespace net {

PeerStore::PeerStore(const QString &dir, QObject *parent) : QObject(parent), file(QDir(dir).filePath(QStringLiteral("peers.json")))
{
    reload();
}

void PeerStore::reload()
{
    list.clear();
    QFile f(file);
    if (!f.exists())
        return;
    bool good = f.open(QIODevice::ReadOnly);
    QList<PeerRecord> read;
    if (good) {
        QJsonParseError e;
        const QJsonDocument d = QJsonDocument::fromJson(f.readAll(), &e);
        f.close();
        good = e.error == QJsonParseError::NoError && d.isObject();
        if (good) {
            for (const QJsonValue &v : d.object().value(QStringLiteral("peers")).toArray()) {
                const QJsonObject o = v.toObject();
                PeerRecord r;
                r.pin = QByteArray::fromHex(o.value(QStringLiteral("pin")).toString().toLatin1());
                if (r.pin.size() != 32) {
                    good = false;
                    break;
                }
                r.name = o.value(QStringLiteral("name")).toString();
                r.instanceId = o.value(QStringLiteral("instance")).toString();
                r.publicKey = QByteArray::fromBase64(o.value(QStringLiteral("key")).toString().toLatin1());
                r.pairedAt = o.value(QStringLiteral("pairedAt")).toInteger();
                read.append(r);
            }
        }
    }
    if (!good) {
        QFile::remove(file + QStringLiteral(".bad"));
        QFile::rename(file, file + QStringLiteral(".bad"));
        return;
    }
    list = read;
}

bool PeerStore::contains(const QByteArray &pin) const { return find(pin); }

bool PeerStore::find(const QByteArray &pin, PeerRecord *out) const
{
    for (const PeerRecord &r : list) {
        if (r.pin == pin) {
            if (out)
                *out = r;
            return true;
        }
    }
    return false;
}

bool PeerStore::save(const QList<PeerRecord> &next, QString *error) const
{
    QJsonArray a;
    for (const PeerRecord &r : next) {
        QJsonObject o;
        o[QStringLiteral("pin")] = QString::fromLatin1(r.pin.toHex());
        o[QStringLiteral("name")] = r.name;
        o[QStringLiteral("instance")] = r.instanceId;
        o[QStringLiteral("key")] = QString::fromLatin1(r.publicKey.toBase64());
        o[QStringLiteral("pairedAt")] = r.pairedAt;
        a.append(o);
    }
    QJsonObject root;
    root[QStringLiteral("v")] = 1;
    root[QStringLiteral("peers")] = a;
    QDir().mkpath(QFileInfo(file).absolutePath());
    QSaveFile f(file);
    f.setDirectWriteFallback(false);
    if (!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(root).toJson()) < 0 || !f.commit()) {
        if (error)
            *error = f.errorString();
        return false;
    }
    return true;
}

bool PeerStore::add(const PeerRecord &peer, QString *error)
{
    if (peer.pin.size() != 32) {
        if (error)
            *error = QStringLiteral("pin");
        return false;
    }
    QList<PeerRecord> next = list;
    PeerRecord r = peer;
    if (r.pairedAt == 0)
        r.pairedAt = QDateTime::currentSecsSinceEpoch();
    bool replaced = false;
    for (PeerRecord &x : next) {
        if (x.pin == r.pin) {
            x = r;
            replaced = true;
        }
    }
    if (!replaced)
        next.append(r);
    if (!save(next, error))
        return false;
    list = next;
    emit changed();
    return true;
}

bool PeerStore::remove(const QByteArray &pin, QString *error)
{
    QList<PeerRecord> next;
    for (const PeerRecord &r : list)
        if (r.pin != pin)
            next.append(r);
    if (next.size() == list.size())
        return false;
    if (!save(next, error))
        return false;
    list = next;
    emit changed();
    return true;
}

} // namespace net
} // namespace collab
} // namespace qb
