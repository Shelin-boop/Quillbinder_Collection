#pragma once
// Live Sync's membership record (0.99.0), section 6.3 of the design: who belongs to a shared project, in the order of their
// priority to host it. The record is signed by the device that hosts the project when it is written (ECDSA over the record, with
// the device key); every member carries its public key, so a member can verify a new host it has never talked to. A record
// replaces the one before it only when its version is higher and a member of the earlier record signed it.
#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {
namespace net {
class DeviceIdentity;
}

struct Member {
    QString id;        // the device id (the first 16 bytes of the pin as hex)
    QByteArray key;    // the public key, DER SubjectPublicKeyInfo
    QString name;
    bool editor = true; // false = reader
    bool mayHost = true;
    QByteArray pin() const;
};

class Membership {
public:
    QString project;
    qint64 version = 0;
    QList<Member> members; // in the order of priority: the first one hosts when it can
    QString signer;
    QByteArray signature;

    bool isEmpty() const { return members.isEmpty(); }
    int count() const { return int(members.size()); }
    const Member *find(const QString &deviceId) const;
    bool isMember(const QString &deviceId) const { return find(deviceId) != nullptr; }
    bool mayHost(const QString &deviceId) const;
    // 0 = highest priority; -1 = not a member
    int priorityIndex(const QString &deviceId) const;
    // the devices that may host, best first, without `except`
    QStringList hostCandidates(const QString &except = QString()) const;

    // the tests and the service: desktops host by default, readers and Android devices only when allowed
    static bool defaultMayHost(bool editor, bool android) { return editor && !android; }

    QJsonObject toJson() const;
    // false with a reason for anything that is not a well-formed record (the keys and ids are checked against each other here;
    // the signature is checked by signatureValid)
    static bool fromJson(const QJsonObject &o, Membership *out, QString *why = nullptr);
    // signs the record as `signer` (sets signer and signature)
    void signWith(const net::DeviceIdentity &signerIdentity);
    // the signature fits the record and the signer's key (the signer must be one of the members)
    bool signatureValid(QString *why = nullptr) const;
    // `next` may replace this record: same project, a higher version, valid, signed by a member of this record (and by
    // `requiredSigner` when it is not empty: the device that hosts now)
    bool acceptsSuccessor(const Membership &next, const QString &requiredSigner, QString *why = nullptr) const;

private:
    QByteArray signingBytes() const;
};

} // namespace collab
} // namespace qb
