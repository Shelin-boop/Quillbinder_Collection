#pragma once
// This device's identity for Live Sync (0.99.0): a key pair and a self-signed certificate made at the first switch-on.
// The device id and the pin come from the public key alone (the certificate is only the envelope TLS needs), so a
// session's actor is always the authenticated key.
//   key        ECDSA P-256 (see "Ed25519 probe" in docs/dev-notes/runtime-and-addons.md: Botan 3.14's TLS 1.3 cannot sign with Ed25519
//              certificates); the private key (PKCS#8) lives in the key store, entry "Live Sync device key"
//   device.json  in the Live Sync folder: the certificate, the name shown to paired devices and the instance id (the
//              opaque id that is published for discovery; random, tied to this key)
//   pin        SHA-256 over the public key's DER SubjectPublicKeyInfo (32 bytes)
//   deviceId   the first 16 bytes of the pin as hex (32 characters)
// A new key is a new device: it has to be paired again. There is no key rotation in this version.
#include "core/crypto/KeyStore.h"

#include <QByteArray>
#include <QString>
#include <memory>

namespace qb {
namespace collab {
namespace net {

class DeviceIdentity {
public:
    ~DeviceIdentity();
    DeviceIdentity(const DeviceIdentity &) = delete;
    DeviceIdentity &operator=(const DeviceIdentity &) = delete;

    // false in a build whose encryption pack has no Botan TLS (the pack of tools/get-crypto.sh before 0.99.0)
    static bool available();
    static QString keyEntry(); // "Live Sync device key"
    static QString keyAlgorithm(); // "ECDSA P-256"

    // loads the identity kept in <dir> and the key store, or makes a new one (key first, then the file). nullptr with
    // *error when the key store does not answer (*keyStatus then says how: NoBackend = no store and no wallet to ask,
    // Denied = locked or cancelled), when the key entry is damaged (never replaced silently) or the files cannot be written
    static std::unique_ptr<DeviceIdentity> loadOrCreate(const QString &dir, crypto::KeyStore &keys, QString *error = nullptr,
                                                       crypto::KeyStore::Status *keyStatus = nullptr);
    // this program's: <data folder>/livesync and crypto::KeyStore::system()
    static std::unique_ptr<DeviceIdentity> forThisDevice(QString *error = nullptr, crypto::KeyStore::Status *keyStatus = nullptr);
    static QString standardDir();
    // a fresh identity that is kept nowhere (the tests, and what a man in the middle would use)
    static std::unique_ptr<DeviceIdentity> createEphemeral(const QString &name = QString(), QString *error = nullptr);

    QString name() const { return deviceName; }
    QString instanceId() const { return instance; }
    QString deviceId() const;
    QByteArray pin() const { return pinBytes; }
    QByteArray certificateDer() const { return certDer; }
    QByteArray publicKeyDer() const { return spki; }

    // a signature over data (ECDSA P-256 with SHA-256, fixed r||s form), for proofs, votes and membership records
    QByteArray sign(const QByteArray &data) const;
    static bool verify(const QByteArray &publicKeyDer, const QByteArray &data, const QByteArray &signature);
    // the pin of a public key (DER SubjectPublicKeyInfo) and the id of a pin
    static QByteArray pinOfPublicKey(const QByteArray &publicKeyDer);
    static QString deviceIdOfPin(const QByteArray &pin);

    struct Impl; // (the Botan key and certificate; TlsLink.cpp reads it)
    Impl *impl() const { return d.get(); }

private:
    DeviceIdentity();
    std::unique_ptr<Impl> d;
    QString deviceName, instance;
    QByteArray pinBytes, certDer, spki;
};

} // namespace net
} // namespace collab
} // namespace qb
