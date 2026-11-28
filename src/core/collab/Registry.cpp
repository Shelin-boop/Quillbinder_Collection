#include "core/collab/Registry.h"

#include "core/SafeSave.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>

namespace qb {
namespace collab {

namespace {
QString clean(const QString &p) { return p.isEmpty() ? p : QDir::cleanPath(QFileInfo(p).absoluteFilePath()); }
}

QJsonObject RegistryEntry::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("uuid"), uuid);
    o.insert(QStringLiteral("role"), host ? QStringLiteral("host") : QStringLiteral("client"));
    o.insert(QStringLiteral("folder"), folder);
    o.insert(QStringLiteral("title"), title);
    o.insert(QStringLiteral("hostDeviceId"), hostDeviceId);
    o.insert(QStringLiteral("hostName"), hostName);
    o.insert(QStringLiteral("lastAddress"), lastAddress);
    o.insert(QStringLiteral("lastPort"), lastPort);
    o.insert(QStringLiteral("epoch"), double(epoch));
    o.insert(QStringLiteral("stopped"), stopped);
    if (!access.isEmpty())
        o.insert(QStringLiteral("access"), access);
    if (!authority.isEmpty())
        o.insert(QStringLiteral("authority"), authority);
    return o;
}

RegistryEntry RegistryEntry::fromJson(const QJsonObject &o)
{
    RegistryEntry e;
    e.uuid = o.value(QStringLiteral("uuid")).toString();
    e.host = o.value(QStringLiteral("role")).toString() != QLatin1String("client");
    e.folder = o.value(QStringLiteral("folder")).toString();
    e.title = o.value(QStringLiteral("title")).toString();
    e.hostDeviceId = o.value(QStringLiteral("hostDeviceId")).toString();
    e.hostName = o.value(QStringLiteral("hostName")).toString();
    e.lastAddress = o.value(QStringLiteral("lastAddress")).toString();
    e.lastPort = o.value(QStringLiteral("lastPort")).toInt();
    e.epoch = qint64(o.value(QStringLiteral("epoch")).toDouble());
    e.stopped = o.value(QStringLiteral("stopped")).toBool();
    e.access = o.value(QStringLiteral("access")).toObject();
    e.authority = o.value(QStringLiteral("authority")).toObject();
    return e;
}

Registry::Registry(const QString &dir, QObject *parent) : QObject(parent), m_dir(dir), m_file(dir + QStringLiteral("/registry.json"))
{
    QDir().mkpath(m_dir);
    reload();
}

Registry::~Registry()
{
    for (auto it = m_locks.begin(); it != m_locks.end(); ++it)
        (*it)->unlock();
}

void Registry::reload()
{
    QList<RegistryEntry> list;
    QFile f(m_file);
    if (f.open(QIODevice::ReadOnly)) {
        const QByteArray raw = f.readAll();
        f.close();
        QJsonParseError pe;
        const QJsonDocument d = QJsonDocument::fromJson(raw, &pe);
        if (pe.error != QJsonParseError::NoError || !d.isObject()) {
            // kept aside (never silently dropped), the list starts empty
            safesave::writeFile(m_file + QStringLiteral(".bad"), raw);
            safesave::removeFile(m_file);
        } else {
            for (const QJsonValue &v : d.object().value(QStringLiteral("projects")).toArray()) {
                const RegistryEntry e = RegistryEntry::fromJson(v.toObject());
                if (!e.uuid.isEmpty())
                    list << e;
            }
        }
    }
    m_list = list;
}

bool Registry::find(const QString &uuid, RegistryEntry *out) const
{
    for (const RegistryEntry &e : m_list)
        if (e.uuid == uuid) {
            if (out)
                *out = e;
            return true;
        }
    return false;
}

bool Registry::findByFolder(const QString &folder, RegistryEntry *out) const
{
    const QString c = clean(folder);
    for (const RegistryEntry &e : m_list)
        if (!e.stopped && clean(e.folder) == c) {
            if (out)
                *out = e;
            return true;
        }
    return false;
}

bool Registry::save(const QList<RegistryEntry> &next, QString *error)
{
    QJsonArray a;
    for (const RegistryEntry &e : next)
        a.append(e.toJson());
    QJsonObject o;
    o.insert(QStringLiteral("v"), 1);
    o.insert(QStringLiteral("projects"), a);
    if (!safesave::writeFile(m_file, QJsonDocument(o).toJson(QJsonDocument::Indented), error))
        return false;
    return true;
}

bool Registry::upsert(const RegistryEntry &entry, QString *error)
{
    reload(); // another program of this device may have written since
    RegistryEntry e = entry;
    e.folder = clean(e.folder);
    QList<RegistryEntry> next = m_list;
    bool known = false;
    for (RegistryEntry &x : next)
        if (x.uuid == e.uuid) {
            x = e;
            known = true;
        }
    if (!known)
        next << e;
    if (!save(next, error))
        return false;
    m_list = next;
    emit changed();
    return true;
}

bool Registry::remove(const QString &uuid, QString *error)
{
    reload();
    QList<RegistryEntry> next;
    for (const RegistryEntry &x : m_list)
        if (x.uuid != uuid)
            next << x;
    if (next.size() == m_list.size())
        return true;
    if (!save(next, error))
        return false;
    m_list = next;
    emit changed();
    return true;
}

bool Registry::isCopy(const QString &folder, const QString &uuid) const
{
    RegistryEntry e;
    if (!find(uuid, &e))
        return false; // unknown here: this device never shared it; the caller decides (a foreign folder is no copy of ours)
    return !e.stopped && clean(e.folder) != clean(folder);
}

bool Registry::lockHost(const QString &uuid, QString *error)
{
    if (m_locks.contains(uuid))
        return true;
    auto lock = std::make_shared<QLockFile>(m_dir + QStringLiteral("/host-") + uuid + QStringLiteral(".lock"));
    lock->setStaleLockTime(0); // a crashed program's lock is detected by its process id, never by its age
    if (!lock->tryLock(0)) {
        if (error)
            *error = QStringLiteral("another program of this device already hosts this project");
        return false;
    }
    m_locks.insert(uuid, lock);
    return true;
}

void Registry::releaseHost(const QString &uuid)
{
    auto it = m_locks.find(uuid);
    if (it == m_locks.end())
        return;
    (*it)->unlock();
    m_locks.erase(it);
}

QString Registry::newUuid() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

} // namespace collab
} // namespace qb
