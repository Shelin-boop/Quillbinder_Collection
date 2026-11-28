#pragma once
// The devices this device has paired with (0.99.0): `peers.json` in the Live Sync folder of the data folder. Each entry is a
// pin (the SHA-256 of the peer's public key), the name the peer gave itself and its instance id (the opaque id it publishes
// for discovery). A key that is not in this list is not trusted by anything; removing an entry is how a device is unpaired.
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>

namespace qb {
namespace collab {
namespace net {

struct PeerRecord {
    QByteArray pin; // 32 bytes
    QString name;
    QString instanceId;
    qint64 pairedAt = 0; // seconds since 1970
    QString deviceId() const { return QString::fromLatin1(pin.left(16).toHex()); }
};

class PeerStore : public QObject {
    Q_OBJECT
public:
    // the list in <dir>/peers.json (read now; a missing file is an empty list, a damaged one is kept aside as peers.json.bad
    // and the list starts empty: nothing is trusted that cannot be read)
    explicit PeerStore(const QString &dir, QObject *parent = nullptr);

    QString filePath() const { return file; }
    QList<PeerRecord> peers() const { return list; }
    bool contains(const QByteArray &pin) const;
    bool find(const QByteArray &pin, PeerRecord *out = nullptr) const;
    // false when the file cannot be written (the list in memory is then left as it was)
    bool add(const PeerRecord &peer, QString *error = nullptr);
    // false when there is no such pin or the file cannot be written
    bool remove(const QByteArray &pin, QString *error = nullptr);
    // reads the file again (another process may have changed it)
    void reload();

signals:
    void changed();

private:
    bool save(const QList<PeerRecord> &next, QString *error) const;
    QString file;
    QList<PeerRecord> list;
};

} // namespace net
} // namespace collab
} // namespace qb
