#pragma once
// Live Sync's registry of this device (0.99.0, section 4.8 and 6.1 of the design): which projects this device shares or has
// joined, in one file of the program's Live Sync folder (registry.json). The uuid of a project lives here and in the host's
// livesync/state.json, never in the project file. A project that is found in a folder the registry places elsewhere is a
// copy (a backup, a Save As, a folder copied by hand): its Live Sync part is not used.
//
// Several programs of one device read and write the file; every change is read again first and written whole (safesave), and
// one host session per uuid and device is held by a lock file (hostLock).
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QLockFile>
#include <QObject>
#include <QString>
#include <memory>

namespace qb {
namespace collab {

struct RegistryEntry {
    QString uuid;
    bool host = true;        // this device hosts it (else it is a replica of another device's)
    QString folder;          // the project folder (host) or the replica folder (client)
    QString title;
    QString hostDeviceId;    // the host's device id as this device knows it (its own for a host)
    QString hostName;
    QString lastAddress;     // where the host was reached last (a client)
    int lastPort = 0;
    qint64 epoch = 0;
    bool stopped = false;    // sharing was stopped here; kept so that the folder is not taken for a copy later
    QJsonObject access;      // host: device id -> "editor" or "reader" (a device that is not listed has no access)
    QJsonObject authority;   // the authority record (package 6 writes and reads it; this class keeps it as it is)
    QJsonObject toJson() const;
    static RegistryEntry fromJson(const QJsonObject &o);
};

class Registry : public QObject {
    Q_OBJECT
public:
    // <dir>/registry.json (a damaged file is kept aside as registry.json.bad and the list starts empty)
    explicit Registry(const QString &dir, QObject *parent = nullptr);
    ~Registry() override;

    QString filePath() const { return m_file; }
    QList<RegistryEntry> entries() const { return m_list; }
    bool find(const QString &uuid, RegistryEntry *out = nullptr) const;
    bool findByFolder(const QString &folder, RegistryEntry *out = nullptr) const;
    // adds or replaces the entry of its uuid; false with a reason when the file cannot be written (the list stays as it was)
    bool upsert(const RegistryEntry &e, QString *error = nullptr);
    bool remove(const QString &uuid, QString *error = nullptr);
    // reads the file again (another program may have changed it)
    void reload();

    // The folder holds Live Sync data of `uuid` but the registry places that project at another folder: it is a copy.
    bool isCopy(const QString &folder, const QString &uuid) const;

    // one host session per uuid and device: the lock is held until releaseHost or the end of this object
    bool lockHost(const QString &uuid, QString *error = nullptr);
    void releaseHost(const QString &uuid);

    static QString newUuid();

signals:
    void changed();

private:
    bool save(const QList<RegistryEntry> &next, QString *error);
    QString m_dir, m_file;
    QList<RegistryEntry> m_list;
    QHash<QString, std::shared_ptr<QLockFile>> m_locks;
};

} // namespace collab
} // namespace qb
