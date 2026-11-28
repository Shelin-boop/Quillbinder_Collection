// This device's identity for Live Sync: see DeviceIdentity.h.
#include "core/collab/net/DeviceIdentity.h"

#include "core/Paths.h"
#include "core/collab/net/TlsInternal.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSysInfo>

#ifdef QB_HAVE_BOTAN_TLS
#include <botan/data_src.h>
#include <botan/ec_group.h>
#include <botan/ecdsa.h>
#include <botan/pk_algs.h>
#include <botan/pk_ops.h>
#include <botan/pkcs8.h>
#include <botan/pubkey.h>
#include <botan/x509_key.h>
#include <botan/x509self.h>
#endif

namespace qb {
namespace collab {
namespace net {

namespace {
QString say(const char *text) { return QCoreApplication::translate("qb::collab", text); }
} // namespace

#ifndef QB_HAVE_BOTAN_TLS
struct DeviceIdentity::Impl {}; // (a build without Botan's TLS: nothing to hold)
#endif

DeviceIdentity::DeviceIdentity() : d(new Impl) {}
DeviceIdentity::~DeviceIdentity() = default;

QString DeviceIdentity::keyEntry() { return QStringLiteral("Live Sync device key"); }
QString DeviceIdentity::keyAlgorithm() { return QStringLiteral("ECDSA P-256"); }
QString DeviceIdentity::deviceId() const { return deviceIdOfPin(pinBytes); }
QString DeviceIdentity::deviceIdOfPin(const QByteArray &pin) { return QString::fromLatin1(pin.left(16).toHex()); }
QString DeviceIdentity::standardDir() { return QDir::cleanPath(dataDir() + QStringLiteral("/livesync")); }

QByteArray DeviceIdentity::pinOfPublicKey(const QByteArray &publicKeyDer)
{
    return QCryptographicHash::hash(publicKeyDer, QCryptographicHash::Sha256);
}

#ifndef QB_HAVE_BOTAN_TLS

bool DeviceIdentity::available() { return false; }
static std::unique_ptr<DeviceIdentity> none(QString *error)
{
    if (error)
        *error = say("This build of Quillbinder has no Live Sync.");
    return nullptr;
}
std::unique_ptr<DeviceIdentity> DeviceIdentity::loadOrCreate(const QString &, crypto::KeyStore &, QString *e,
                                                             crypto::KeyStore::Status *)
{
    return none(e);
}
std::unique_ptr<DeviceIdentity> DeviceIdentity::forThisDevice(QString *e, crypto::KeyStore::Status *) { return none(e); }
std::unique_ptr<DeviceIdentity> DeviceIdentity::createEphemeral(const QString &, QString *e) { return none(e); }
QByteArray DeviceIdentity::sign(const QByteArray &) const { return QByteArray(); }
bool DeviceIdentity::verify(const QByteArray &, const QByteArray &, const QByteArray &) { return false; }

#else

std::shared_ptr<Botan::RandomNumberGenerator> sharedRng()
{
    static const std::shared_ptr<Botan::RandomNumberGenerator> r = std::make_shared<Botan::AutoSeeded_RNG>();
    return r;
}

bool DeviceIdentity::available() { return true; }

static QByteArray toQ(const std::vector<uint8_t> &v)
{
    return QByteArray(reinterpret_cast<const char *>(v.data()), qsizetype(v.size()));
}
static std::vector<uint8_t> fromQ(const QByteArray &b)
{
    return std::vector<uint8_t>(b.begin(), b.end());
}

// the certificate: only the envelope TLS wants; nothing reads its dates or its name for trust
static std::shared_ptr<Botan::X509_Certificate> makeCertificate(const Botan::Private_Key &key)
{
    Botan::X509_Cert_Options o;
    o.common_name = "Quillbinder device";
    o.not_before("20260101000000Z");
    o.not_after("21260101000000Z");
    o.constraints = Botan::Key_Constraints::DigitalSignature;
    o.add_ex_constraint("PKIX.ServerAuth");
    o.add_ex_constraint("PKIX.ClientAuth");
    return std::make_shared<Botan::X509_Certificate>(Botan::X509::create_self_signed_cert(o, key, "SHA-256", *sharedRng()));
}

static void finish(DeviceIdentity::Impl &d, QByteArray *spki, QByteArray *pin, QByteArray *certDer)
{
    *certDer = toQ(d.cert->BER_encode());
    *spki = toQ(d.key->subject_public_key());
    *pin = QCryptographicHash::hash(*spki, QCryptographicHash::Sha256);
}

static QString randomHex(int bytes)
{
    QByteArray b(bytes, '\0');
    sharedRng()->randomize(reinterpret_cast<uint8_t *>(b.data()), size_t(bytes));
    return QString::fromLatin1(b.toHex());
}

std::unique_ptr<DeviceIdentity> DeviceIdentity::createEphemeral(const QString &name, QString *error)
{
    try {
        std::unique_ptr<DeviceIdentity> id(new DeviceIdentity());
        id->d->key = std::make_shared<Botan::ECDSA_PrivateKey>(*sharedRng(), Botan::EC_Group::from_name("secp256r1"));
        id->d->cert = makeCertificate(*id->d->key);
        finish(*id->d, &id->spki, &id->pinBytes, &id->certDer);
        id->deviceName = name.isEmpty() ? QStringLiteral("Quillbinder device") : name;
        id->instance = randomHex(8);
        return id;
    } catch (const std::exception &e) {
        if (error)
            *error = QString::fromUtf8(e.what());
        return nullptr;
    }
}

std::unique_ptr<DeviceIdentity> DeviceIdentity::forThisDevice(QString *error, crypto::KeyStore::Status *keyStatus)
{
    return loadOrCreate(standardDir(), crypto::KeyStore::system(), error, keyStatus);
}

std::unique_ptr<DeviceIdentity> DeviceIdentity::loadOrCreate(const QString &dir, crypto::KeyStore &keys, QString *error,
                                                             crypto::KeyStore::Status *keyStatus)
{
    using Status = crypto::KeyStore::Status;
    const auto fail = [&](const QString &text, Status st) -> std::unique_ptr<DeviceIdentity> {
        if (error)
            *error = text;
        if (keyStatus)
            *keyStatus = st;
        return nullptr;
    };
    try {
        const QString jsonPath = QDir(dir).filePath(QStringLiteral("device.json"));
        QJsonObject info;
        {
            QFile f(jsonPath);
            if (f.open(QIODevice::ReadOnly))
                info = QJsonDocument::fromJson(f.readAll()).object();
        }
        QByteArray stored;
        QString why;
        const Status st = keys.read(keyEntry(), &stored, &why);
        std::unique_ptr<DeviceIdentity> id(new DeviceIdentity());
        bool madeKey = false;
        if (st == Status::Ok) {
            const QByteArray der = QByteArray::fromBase64(stored);
            Botan::DataSource_Memory src(reinterpret_cast<const uint8_t *>(der.constData()), size_t(der.size()));
            std::unique_ptr<Botan::Private_Key> k;
            try {
                k = Botan::PKCS8::load_key(src);
            } catch (const std::exception &) {
            }
            if (!k || k->algo_name() != "ECDSA")
                return fail(say("The device key of Live Sync in the key store is damaged."), Status::Failed);
            id->d->key = std::move(k);
        } else if (st == Status::NotFound) {
            id->d->key = std::make_shared<Botan::ECDSA_PrivateKey>(*sharedRng(), Botan::EC_Group::from_name("secp256r1"));
            const auto ber = Botan::PKCS8::BER_encode(*id->d->key);
            QByteArray b64 = QByteArray(reinterpret_cast<const char *>(ber.data()), qsizetype(ber.size())).toBase64();
            QString werr;
            const Status ws = keys.write(keyEntry(), b64, &werr);
            b64.fill('\0');
            if (ws != Status::Ok)
                return fail(werr.isEmpty() ? say("The device key of Live Sync cannot be kept in the key store.") : werr, ws);
            madeKey = true;
        } else {
            return fail(why.isEmpty() ? say("The key store does not answer.") : why, st);
        }
        // the certificate kept in device.json is used when it belongs to this key, otherwise it is made again (same
        // key = same device)
        const QByteArray keyInfo = toQ(id->d->key->subject_public_key());
        if (!madeKey && info.contains(QStringLiteral("cert"))) {
            try {
                const QByteArray der = QByteArray::fromBase64(info.value(QStringLiteral("cert")).toString().toLatin1());
                auto c = std::make_shared<Botan::X509_Certificate>(fromQ(der));
                if (toQ(c->subject_public_key_info()) == keyInfo)
                    id->d->cert = c;
            } catch (const std::exception &) {
            }
        }
        if (!id->d->cert)
            id->d->cert = makeCertificate(*id->d->key);
        finish(*id->d, &id->spki, &id->pinBytes, &id->certDer);
        id->deviceName = madeKey ? QString() : info.value(QStringLiteral("name")).toString();
        id->instance = madeKey ? QString() : info.value(QStringLiteral("instance")).toString();
        bool rewrite = madeKey || id->certDer != QByteArray::fromBase64(info.value(QStringLiteral("cert")).toString().toLatin1());
        if (id->deviceName.isEmpty()) {
            id->deviceName = QSysInfo::machineHostName();
            if (id->deviceName.isEmpty())
                id->deviceName = QStringLiteral("Quillbinder device");
            rewrite = true;
        }
        if (id->instance.size() != 16) {
            id->instance = randomHex(8);
            rewrite = true;
        }
        if (rewrite) {
            QJsonObject o;
            o[QStringLiteral("v")] = 1;
            o[QStringLiteral("name")] = id->deviceName;
            o[QStringLiteral("instance")] = id->instance;
            o[QStringLiteral("cert")] = QString::fromLatin1(id->certDer.toBase64());
            QDir().mkpath(dir);
            QSaveFile f(jsonPath);
            f.setDirectWriteFallback(false);
            if (!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(o).toJson()) < 0 || !f.commit())
                return fail(say("The files of Live Sync cannot be written.") + QLatin1Char(' ') + f.errorString(), Status::Failed);
        }
        if (keyStatus)
            *keyStatus = Status::Ok;
        return id;
    } catch (const std::exception &e) {
        return fail(QString::fromUtf8(e.what()), crypto::KeyStore::Status::Failed);
    }
}

QByteArray DeviceIdentity::sign(const QByteArray &data) const
{
    try {
        Botan::PK_Signer signer(*d->key, *sharedRng(), "SHA-256", Botan::Signature_Format::Standard);
        signer.update(reinterpret_cast<const uint8_t *>(data.constData()), size_t(data.size()));
        return toQ(signer.signature(*sharedRng()));
    } catch (const std::exception &) {
        return QByteArray();
    }
}

bool DeviceIdentity::verify(const QByteArray &publicKeyDer, const QByteArray &data, const QByteArray &signature)
{
    try {
        const auto key = Botan::X509::load_key(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(publicKeyDer.constData()),
                                                                          size_t(publicKeyDer.size())));
        if (!key || key->algo_name() != "ECDSA")
            return false;
        Botan::PK_Verifier v(*key, "SHA-256", Botan::Signature_Format::Standard);
        v.update(reinterpret_cast<const uint8_t *>(data.constData()), size_t(data.size()));
        return v.check_signature(reinterpret_cast<const uint8_t *>(signature.constData()), size_t(signature.size()));
    } catch (const std::exception &) {
        return false;
    }
}

#endif

} // namespace net
} // namespace collab
} // namespace qb
