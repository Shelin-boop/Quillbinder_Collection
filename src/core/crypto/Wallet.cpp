// The Quillbinder wallet: see Wallet.h.
#include "core/crypto/Wallet.h"

#include "core/Paths.h"
#include "core/TestSwitches.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSaveFile>
#include <QStringList>

#ifdef QB_HAVE_BOTAN_TLS
#include <botan/aead.h>
#include <botan/auto_rng.h>
#include <botan/exceptn.h>
#include <botan/pwdhash.h>
#include <botan/secmem.h>
#endif

namespace qb {
namespace crypto {

namespace {

const char kMagic[] = "QBWALLET1\n";
constexpr int kSaltBytes = 16;
constexpr int kNonceBytes = 12;
constexpr int kKeyBytes = 32;
constexpr int kTagBytes = 16;
constexpr qint64 kMaxFile = 16 * 1024 * 1024;

QString say(const char *text) { return QCoreApplication::translate("qb::crypto", text); }

QMutex handlerMutex;
Wallet::UnlockHandler &handler()
{
    static Wallet::UnlockHandler h;
    return h;
}

} // namespace

struct Wallet::Impl {
    QString file;
    bool unlocked = false;
    QHash<QString, QByteArray> entries;
    quint32 m = 0, t = 0, p = 0;
    QByteArray salt;
#ifdef QB_HAVE_BOTAN_TLS
    Botan::secure_vector<uint8_t> key;
#endif

    void wipe()
    {
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            it.value().detach();
            it.value().fill('\0');
        }
        entries.clear();
#ifdef QB_HAVE_BOTAN_TLS
        Botan::zap(key);
        key.clear();
#endif
        unlocked = false;
    }
};

#ifdef QB_HAVE_BOTAN_TLS

namespace {

void costs(quint32 *m, quint32 *t, quint32 *p)
{
    if (QB_TEST_ENV_SET("QB_TEST_WALLET_FAST")) { // the tests: a cheap setting, so that the suite is quick
        *m = 1024;
        *t = 1;
        *p = 1;
        return;
    }
    *m = 64 * 1024; // 64 MiB, 3 passes, one lane: about half a second on a desktop; Android's low-memory phones cope
    *t = 3;
    *p = 1;
}

Botan::secure_vector<uint8_t> derive(const QString &passphrase, const QByteArray &salt, quint32 m, quint32 t, quint32 p)
{
    const auto family = Botan::PasswordHashFamily::create_or_throw("Argon2id");
    const auto hash = family->from_params(m, t, p);
    const QByteArray pw = passphrase.normalized(QString::NormalizationForm_C).toUtf8();
    Botan::secure_vector<uint8_t> out(kKeyBytes);
    hash->derive_key(out.data(), out.size(), pw.constData(), size_t(pw.size()),
                     reinterpret_cast<const uint8_t *>(salt.constData()), size_t(salt.size()));
    return out;
}

QByteArray seal(const Wallet::Impl &d, const QHash<QString, QByteArray> &entries)
{
    QJsonObject header;
    header[QStringLiteral("v")] = 1;
    header[QStringLiteral("kdf")] = QStringLiteral("argon2id");
    header[QStringLiteral("m")] = int(d.m);
    header[QStringLiteral("t")] = int(d.t);
    header[QStringLiteral("p")] = int(d.p);
    header[QStringLiteral("salt")] = QString::fromLatin1(d.salt.toBase64());
    Botan::AutoSeeded_RNG rng;
    QByteArray nonce(kNonceBytes, '\0');
    rng.randomize(reinterpret_cast<uint8_t *>(nonce.data()), size_t(nonce.size()));
    header[QStringLiteral("nonce")] = QString::fromLatin1(nonce.toBase64());
    QByteArray head = QByteArray(kMagic) + QJsonDocument(header).toJson(QJsonDocument::Compact) + '\n';

    QJsonObject list;
    for (auto it = entries.constBegin(); it != entries.constEnd(); ++it)
        list[it.key()] = QString::fromLatin1(it.value().toBase64());
    QJsonObject body;
    body[QStringLiteral("entries")] = list;
    const QByteArray plain = QJsonDocument(body).toJson(QJsonDocument::Compact);

    auto aead = Botan::AEAD_Mode::create_or_throw("AES-256/GCM", Botan::Cipher_Dir::Encryption);
    aead->set_key(d.key);
    aead->set_associated_data(reinterpret_cast<const uint8_t *>(head.constData()), size_t(head.size()));
    aead->start(reinterpret_cast<const uint8_t *>(nonce.constData()), size_t(nonce.size()));
    Botan::secure_vector<uint8_t> buf(plain.begin(), plain.end());
    aead->finish(buf);
    QByteArray out = head;
    out.append(reinterpret_cast<const char *>(buf.data()), qsizetype(buf.size()));
    Botan::zap(buf);
    return out;
}

} // namespace

#endif // QB_HAVE_BOTAN_TLS

Wallet::Wallet(const QString &file) : d(new Impl) { d->file = file; }
Wallet::~Wallet()
{
    d->wipe();
}

bool Wallet::available()
{
#ifdef QB_HAVE_BOTAN_TLS
    return true;
#else
    return false;
#endif
}

QString Wallet::standardPath() { return QDir::cleanPath(dataDir() + QStringLiteral("/wallet.qbw")); }

Wallet &Wallet::standard()
{
    static Wallet w(standardPath());
    return w;
}

QString Wallet::filePath() const { return d->file; }
bool Wallet::exists() const { return QFileInfo::exists(d->file); }
bool Wallet::isUnlocked() const { return d->unlocked; }

void Wallet::lock() { d->wipe(); }

QStringList Wallet::entries() const
{
    QStringList l = d->entries.keys();
    l.sort();
    return l;
}

#ifndef QB_HAVE_BOTAN_TLS

static Wallet::Result notBuilt(QString *error)
{
    if (error)
        *error = say("This build of Quillbinder has no wallet.");
    return Wallet::Result::Failed;
}
Wallet::Result Wallet::create(const QString &, QString *e) { return notBuilt(e); }
Wallet::Result Wallet::unlock(const QString &, QString *e) { return notBuilt(e); }
Wallet::Result Wallet::changePassphrase(const QString &, const QString &, QString *e) { return notBuilt(e); }
Wallet::Result Wallet::write(const QString &, const QByteArray &, QString *e) { return notBuilt(e); }
Wallet::Result Wallet::read(const QString &, QByteArray *, QString *e) const { return notBuilt(e); }
Wallet::Result Wallet::remove(const QString &, QString *e) { return notBuilt(e); }

#else

// writes the whole file; the old one stays whole when this fails
static Wallet::Result save(Wallet::Impl &d, const QHash<QString, QByteArray> &entries, QString *error)
{
    try {
        const QByteArray data = seal(d, entries);
        QDir().mkpath(QFileInfo(d.file).absolutePath());
        QSaveFile f(d.file);
        f.setDirectWriteFallback(false);
        if (!f.open(QIODevice::WriteOnly)) {
            if (error)
                *error = say("The wallet file cannot be written.") + QLatin1Char(' ') + f.errorString();
            return Wallet::Result::Failed;
        }
        f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        if (f.write(data) != data.size() || !f.commit()) {
            if (error)
                *error = say("The wallet file cannot be written.") + QLatin1Char(' ') + f.errorString();
            return Wallet::Result::Failed;
        }
        return Wallet::Result::Ok;
    } catch (const std::exception &e) {
        if (error)
            *error = QString::fromUtf8(e.what());
        return Wallet::Result::Failed;
    }
}

Wallet::Result Wallet::create(const QString &passphrase, QString *error)
{
    if (exists()) {
        if (error)
            *error = say("There is a wallet already.");
        return Result::Exists;
    }
    if (passphrase.isEmpty()) {
        if (error)
            *error = say("The wallet's passphrase must not be empty.");
        return Result::Failed;
    }
    try {
        d->wipe();
        costs(&d->m, &d->t, &d->p);
        Botan::AutoSeeded_RNG rng;
        d->salt = QByteArray(kSaltBytes, '\0');
        rng.randomize(reinterpret_cast<uint8_t *>(d->salt.data()), size_t(d->salt.size()));
        d->key = derive(passphrase, d->salt, d->m, d->t, d->p);
    } catch (const std::exception &e) {
        d->wipe();
        if (error)
            *error = QString::fromUtf8(e.what());
        return Result::Failed;
    }
    const Result r = save(*d, {}, error);
    if (r != Result::Ok) {
        d->wipe();
        return r;
    }
    d->unlocked = true;
    return Result::Ok;
}

Wallet::Result Wallet::unlock(const QString &passphrase, QString *error)
{
    QFile f(d->file);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = say("There is no wallet yet.");
        return Result::NotFound;
    }
    if (f.size() > kMaxFile || f.size() < qint64(sizeof(kMagic)) + kTagBytes) {
        if (error)
            *error = say("The wallet file is damaged.");
        return Result::Failed;
    }
    const QByteArray all = f.readAll();
    f.close();
    const int nl = all.indexOf('\n', int(sizeof(kMagic)) - 1);
    if (!all.startsWith(kMagic) || nl < 0) {
        if (error)
            *error = say("The wallet file is damaged.");
        return Result::Failed;
    }
    const QByteArray head = all.left(nl + 1);
    const QJsonObject h = QJsonDocument::fromJson(head.mid(int(sizeof(kMagic)) - 1)).object();
    const QByteArray salt = QByteArray::fromBase64(h.value(QStringLiteral("salt")).toString().toLatin1());
    const QByteArray nonce = QByteArray::fromBase64(h.value(QStringLiteral("nonce")).toString().toLatin1());
    const qint64 m = h.value(QStringLiteral("m")).toInteger(), t = h.value(QStringLiteral("t")).toInteger(),
                 p = h.value(QStringLiteral("p")).toInteger();
    if (h.value(QStringLiteral("v")).toInt() != 1 || h.value(QStringLiteral("kdf")).toString() != QLatin1String("argon2id")
        || salt.size() != kSaltBytes || nonce.size() != kNonceBytes || m < 8 || m > kMaxMemoryKiB || t < 1
        || t > kMaxPasses || p < 1 || p > kMaxLanes) {
        if (error)
            *error = say("The wallet file is damaged or from a newer version.");
        return Result::Failed;
    }
    const QByteArray sealed = all.mid(nl + 1);
    try {
        d->wipe();
        auto key = derive(passphrase, salt, quint32(m), quint32(t), quint32(p));
        auto aead = Botan::AEAD_Mode::create_or_throw("AES-256/GCM", Botan::Cipher_Dir::Decryption);
        aead->set_key(key);
        aead->set_associated_data(reinterpret_cast<const uint8_t *>(head.constData()), size_t(head.size()));
        aead->start(reinterpret_cast<const uint8_t *>(nonce.constData()), size_t(nonce.size()));
        Botan::secure_vector<uint8_t> buf(sealed.begin(), sealed.end());
        try {
            aead->finish(buf);
        } catch (const Botan::Invalid_Authentication_Tag &) {
            Botan::zap(key);
            if (error)
                *error = say("The passphrase is wrong.");
            return Result::WrongPassphrase;
        }
        const QByteArray plain(reinterpret_cast<const char *>(buf.data()), qsizetype(buf.size()));
        Botan::zap(buf);
        const QJsonObject body = QJsonDocument::fromJson(plain).object().value(QStringLiteral("entries")).toObject();
        for (auto it = body.constBegin(); it != body.constEnd(); ++it)
            d->entries.insert(it.key(), QByteArray::fromBase64(it.value().toString().toLatin1()));
        d->key = std::move(key);
        d->salt = salt;
        d->m = quint32(m);
        d->t = quint32(t);
        d->p = quint32(p);
        d->unlocked = true;
        return Result::Ok;
    } catch (const std::exception &e) {
        d->wipe();
        if (error)
            *error = QString::fromUtf8(e.what());
        return Result::Failed;
    }
}

Wallet::Result Wallet::changePassphrase(const QString &oldPassphrase, const QString &newPassphrase, QString *error)
{
    if (!d->unlocked) {
        if (error)
            *error = say("The wallet is locked.");
        return Result::Locked;
    }
    if (newPassphrase.isEmpty()) {
        if (error)
            *error = say("The wallet's passphrase must not be empty.");
        return Result::Failed;
    }
    // the old passphrase is checked again (a wallet left open on a screen must not be re-keyed by a passer-by)
    {
        Wallet probe(d->file);
        const Result r = probe.unlock(oldPassphrase, error);
        if (r != Result::Ok)
            return r;
    }
    try {
        Impl n;
        n.file = d->file;
        costs(&n.m, &n.t, &n.p);
        Botan::AutoSeeded_RNG rng;
        n.salt = QByteArray(kSaltBytes, '\0');
        rng.randomize(reinterpret_cast<uint8_t *>(n.salt.data()), size_t(n.salt.size()));
        n.key = derive(newPassphrase, n.salt, n.m, n.t, n.p);
        const Result r = save(n, d->entries, error);
        if (r != Result::Ok) {
            n.wipe();
            return r;
        }
        d->key = n.key;
        d->salt = n.salt;
        d->m = n.m;
        d->t = n.t;
        d->p = n.p;
        n.wipe();
        return Result::Ok;
    } catch (const std::exception &e) {
        if (error)
            *error = QString::fromUtf8(e.what());
        return Result::Failed;
    }
}

Wallet::Result Wallet::write(const QString &entry, const QByteArray &secret, QString *error)
{
    if (!d->unlocked) {
        if (error)
            *error = say("The wallet is locked.");
        return Result::Locked;
    }
    QHash<QString, QByteArray> next = d->entries;
    next.insert(entry, secret);
    const Result r = save(*d, next, error);
    if (r == Result::Ok)
        d->entries = next; // (only after the file is whole: the memory never shows what the file does not hold)
    return r;
}

Wallet::Result Wallet::read(const QString &entry, QByteArray *secret, QString *error) const
{
    if (!d->unlocked) {
        if (error)
            *error = say("The wallet is locked.");
        return Result::Locked;
    }
    const auto it = d->entries.constFind(entry);
    if (it == d->entries.constEnd())
        return Result::NotFound;
    if (secret)
        *secret = it.value();
    return Result::Ok;
}

Wallet::Result Wallet::remove(const QString &entry, QString *error)
{
    if (!d->unlocked) {
        if (error)
            *error = say("The wallet is locked.");
        return Result::Locked;
    }
    if (!d->entries.contains(entry))
        return Result::NotFound;
    QHash<QString, QByteArray> next = d->entries;
    next.remove(entry);
    const Result r = save(*d, next, error);
    if (r == Result::Ok)
        d->entries = next;
    return r;
}

#endif // QB_HAVE_BOTAN_TLS

void Wallet::setUnlockHandler(UnlockHandler h)
{
    QMutexLocker l(&handlerMutex);
    handler() = std::move(h);
}

bool Wallet::hasUnlockHandler()
{
    QMutexLocker l(&handlerMutex);
    return bool(handler()) || QB_TEST_ENV_SET("QB_TEST_WALLET_PASSPHRASE");
}

Wallet::Result Wallet::ensureUnlocked(QString *error)
{
    if (!available()) {
        if (error)
            *error = say("This build of Quillbinder has no wallet.");
        return Result::Failed;
    }
    if (d->unlocked)
        return Result::Ok;
    UnlockHandler h;
    {
        QMutexLocker l(&handlerMutex);
        h = handler();
    }
    if (!h && QB_TEST_ENV_SET("QB_TEST_WALLET_PASSPHRASE")) {
        const QString pw = QB_TEST_ENV("QB_TEST_WALLET_PASSPHRASE");
        h = [pw](bool, QString *out) {
            *out = pw;
            return true;
        };
    }
    if (!h) {
        if (error)
            *error = say("The wallet cannot be asked for its passphrase here.");
        return Result::Locked;
    }
    const bool creating = !exists();
    for (int attempt = 0; attempt < 3; ++attempt) {
        QString pw;
        if (!h(creating, &pw)) {
            if (error)
                *error = say("The wallet stays locked.");
            return Result::Locked;
        }
        const Result r = creating ? create(pw, error) : unlock(pw, error);
        if (r == Result::Ok)
            return r;
        if (r != Result::WrongPassphrase)
            return r;
    }
    return Result::WrongPassphrase;
}

} // namespace crypto
} // namespace qb
