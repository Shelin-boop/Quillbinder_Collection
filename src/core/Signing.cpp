#include "core/Signing.h"

#include "core/third_party/monocypher/monocypher-ed25519.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace qb {
namespace signing {

namespace {
// The project's release key (public half), made by the writer on 2026-11-10 with OpenSSL (Ed25519); the private
// half is the secret QB_SIGNING_KEY of the project's repository and never leaves the release workflow.
constexpr char kReleaseKey[] = "bc3c5800083f66e66044ee544a5fd107a4c956c21bf2db6f73d2c41f15c908f8";
} // namespace

bool verify(const QByteArray &message, const QByteArray &signature, const QByteArray &publicKey)
{
    if (signature.size() != 64 || publicKey.size() != 32)
        return false;
    return crypto_ed25519_check(reinterpret_cast<const uint8_t *>(signature.constData()),
                                reinterpret_cast<const uint8_t *>(publicKey.constData()),
                                reinterpret_cast<const uint8_t *>(message.constData()), size_t(message.size()))
           == 0;
}

QString keyId(const QByteArray &publicKey)
{
    return QString::fromLatin1(QCryptographicHash::hash(publicKey, QCryptographicHash::Sha256).toHex().left(16));
}

QList<QByteArray> trustedKeys()
{
    QList<QByteArray> keys{QByteArray::fromHex(kReleaseKey)};
#ifdef QB_TEST_SWITCHES
    const QByteArray test = QByteArray::fromHex(qgetenv("QB_TEST_SIGNING_KEY"));
    if (test.size() == 32)
        keys << test;
#endif
    return keys;
}

QByteArray certificateText(const QString &role, const QByteArray &publicKey, const QString &name, const QString &issued)
{
    return QStringLiteral("Quillbinder key certificate 1\nrole: %1\nkey: %2\nname: %3\nissued: %4\n")
        .arg(role, QString::fromLatin1(publicKey.toHex()), name, issued)
        .toUtf8();
}

Check checkManifest(const QByteArray &manifest, const QByteArray &signature)
{
    Check c;
    if (signature.trimmed().isEmpty())
        return c;
    const QJsonObject o = QJsonDocument::fromJson(signature).object();
    c.key = o.value(QStringLiteral("key")).toString();
    const QByteArray sig = QByteArray::fromBase64(o.value(QStringLiteral("signature")).toString().toLatin1());
    if (o.value(QStringLiteral("algorithm")).toString() != QLatin1String("ed25519") || sig.size() != 64
        || o.value(QStringLiteral("signs")).toString() != QLatin1String("manifest.json")) {
        c.state = State::Invalid;
        return c;
    }
    const QList<QByteArray> release = trustedKeys();
    for (const QByteArray &k : release) {
        if (keyId(k) != c.key)
            continue;
        c.state = verify(manifest, sig, k) ? State::Valid : State::Invalid;
        c.releaseKey = c.state == State::Valid;
        return c;
    }
    // 0.91.3: an add-on key that a release key vouches for. A certificate of another form, of another role or by a
    // key this program does not know leaves the package as not signed; one that names a release key but does not
    // match it, or a description that does not match the add-on key, was changed.
    c.state = State::UnknownKey;
    const QJsonObject cert = o.value(QStringLiteral("certificate")).toObject();
    if (cert.isEmpty())
        return c;
    const QString role = cert.value(QStringLiteral("role")).toString(), name = cert.value(QStringLiteral("name")).toString(),
                  issued = cert.value(QStringLiteral("issued")).toString(), by = cert.value(QStringLiteral("by")).toString();
    const QByteArray addonKey = QByteArray::fromHex(cert.value(QStringLiteral("key")).toString().toLatin1());
    const QByteArray certSig = QByteArray::fromBase64(cert.value(QStringLiteral("signature")).toString().toLatin1());
    static const QRegularExpression dateRe(QStringLiteral("^\\d{4}-\\d{2}-\\d{2}$"));
    if (cert.value(QStringLiteral("format")).toInt() != 1 || role != QLatin1String("addons") || addonKey.size() != 32
        || keyId(addonKey) != c.key || name.trimmed().isEmpty() || name.size() > 120 || name.contains(QLatin1Char('\n'))
        || name.contains(QLatin1Char('\r')) || !dateRe.match(issued).hasMatch() || certSig.size() != 64)
        return c;
    for (const QByteArray &k : release) {
        if (keyId(k) != by)
            continue;
        if (!verify(certificateText(role, addonKey, name, issued), certSig, k)) {
            c.state = State::Invalid;
            return c;
        }
        c.state = verify(manifest, sig, addonKey) ? State::Valid : State::Invalid;
        if (c.state == State::Valid)
            c.signer = name;
        return c;
    }
    return c;
}

} // namespace signing
} // namespace qb
