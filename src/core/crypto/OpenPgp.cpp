// The OpenPGP layer over RNP (0.99.0): see OpenPgp.h. Without RNP (QB_HAVE_RNP not set) every call fails with
// Error::Unavailable.
#include "core/crypto/OpenPgp.h"

#include "core/Paths.h"
#include "core/SafeSave.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTimeZone>
#include <cstring>
#include <utility>
#include <vector>

#ifdef QB_HAVE_RNP
#include <rnp/rnp.h>
#include <rnp/rnp_err.h>
#endif

namespace qb {
namespace crypto {

namespace {
QString say(const char *text) { return QCoreApplication::translate("qb::crypto", text); }
} // namespace

void wipe(QByteArray &data)
{
    if (data.isEmpty())
        return;
    volatile char *p = data.data(); // (detaches first: the bytes wiped are this buffer's own)
    for (qsizetype i = 0; i < data.size(); ++i)
        p[i] = 0;
    data.clear();
}

QString errorText(Error error)
{
    switch (error) {
    case Error::None:
        return QString();
    case Error::Unavailable:
        return say("This build of Quillbinder has no encryption.");
    case Error::WrongPassphrase:
        return say("The passphrase is not right.");
    case Error::Locked:
        return say("The key is locked: enter its passphrase first.");
    case Error::NoSecretKey:
        return say("The file is encrypted to a key whose secret part is not on this device.");
    case Error::Damaged:
        return say("The file is damaged or was changed after it was encrypted.");
    case Error::Unprotected:
        return say("The file was encrypted without an integrity check (an old kind of OpenPGP file) and is not opened.");
    case Error::ForeignKey:
        return say("Quillbinder uses only OpenPGP keys of version 4 on Curve25519 (Ed25519 and Cv25519).");
    case Error::NotAKey:
        return say("The file holds no OpenPGP key.");
    case Error::UnknownKey:
        return say("The key is not in Quillbinder's key ring.");
    case Error::NoRecipient:
        return say("There is no key to encrypt to.");
    case Error::TooLarge:
        return say("The decrypted text would be too large.");
    case Error::Io:
        return say("The file could not be read or written.");
    case Error::Failed:
        break;
    }
    return say("The encryption failed.");
}

QString groupedFingerprint(const QString &fingerprint)
{
    static const QRegularExpression hex(QStringLiteral("^[0-9A-Fa-f]{40}$"));
    if (!hex.match(fingerprint).hasMatch())
        return fingerprint;
    const QString f = fingerprint.toUpper();
    QString out;
    for (int i = 0; i < 10; ++i) {
        if (i)
            out += i == 5 ? QLatin1String("  ") : QLatin1String(" ");
        out += f.mid(i * 4, 4);
    }
    return out;
}

bool looksEncrypted(const QByteArray &head)
{
    const QByteArray start = head.left(64).trimmed();
    if (start.startsWith("-----BEGIN PGP MESSAGE-----"))
        return true;
    if (head.isEmpty())
        return false;
    const auto b = static_cast<unsigned char>(head.at(0));
    if (!(b & 0x80))
        return false;
    const int tag = (b & 0x40) ? (b & 0x3f) : ((b >> 2) & 0x0f);
    return tag == 1 || tag == 3; // a public-key or a symmetric-key encrypted session key
}

namespace {

// --- a small reader of OpenPGP packet headers (RFC 4880 section 4.2), to name what a key file holds before RNP reads
// it: the version and the algorithm of each key packet (RNP cannot read keys of version 5 or 6 at all)
struct KeyPacket {
    int tag = 0;      // 5 secret key, 6 public key, 7 secret subkey, 14 public subkey
    int version = 0;
    int algorithm = 0;
    qsizetype start = 0, end = 0; // of the whole packet in the data
};

// every packet's (tag, start, body start, end); false when the data is no sequence of whole packets
bool scanPackets(const QByteArray &data, QList<KeyPacket> *keys, QList<QPair<qsizetype, qsizetype>> *spans,
                 QList<int> *tags)
{
    const auto *p = reinterpret_cast<const unsigned char *>(data.constData());
    const qsizetype n = data.size();
    qsizetype i = 0;
    while (i < n) {
        const qsizetype start = i;
        const unsigned char b = p[i++];
        if (!(b & 0x80))
            return false;
        int tag;
        qsizetype len = 0;
        if (b & 0x40) { // new format
            tag = b & 0x3f;
            if (i >= n)
                return false;
            const unsigned char l0 = p[i++];
            if (l0 < 192) {
                len = l0;
            } else if (l0 < 224) {
                if (i >= n)
                    return false;
                len = ((l0 - 192) << 8) + p[i++] + 192;
            } else if (l0 == 255) {
                if (i + 4 > n)
                    return false;
                len = (qsizetype(p[i]) << 24) | (qsizetype(p[i + 1]) << 16) | (qsizetype(p[i + 2]) << 8) | p[i + 3];
                i += 4;
            } else {
                return false; // partial lengths: never used by key packets
            }
        } else { // old format
            tag = (b >> 2) & 0x0f;
            const int lt = b & 3;
            const int bytes = lt == 0 ? 1 : lt == 1 ? 2 : lt == 2 ? 4 : 0;
            if (bytes == 0) {
                len = n - i; // indeterminate: to the end
            } else {
                if (i + bytes > n)
                    return false;
                for (int k = 0; k < bytes; ++k)
                    len = (len << 8) | p[i + k];
                i += bytes;
            }
        }
        if (len < 0 || i + len > n)
            return false;
        if (tag == 5 || tag == 6 || tag == 7 || tag == 14) {
            KeyPacket k;
            k.tag = tag;
            k.start = start;
            k.end = i + len;
            if (len >= 1)
                k.version = p[i];
            const qsizetype algoAt = k.version == 3 || k.version == 2 ? 7 : 5;
            if (len > algoAt)
                k.algorithm = p[i + algoAt];
            if (keys)
                keys->append(k);
        }
        if (spans)
            spans->append({start, i + len});
        if (tags)
            tags->append(tag);
        i += len;
    }
    return true;
}

QString algorithmName(int algorithm)
{
    switch (algorithm) {
    case 1: case 2: case 3: return QStringLiteral("RSA");
    case 16: case 20: return QStringLiteral("ElGamal");
    case 17: return QStringLiteral("DSA");
    case 18: return QStringLiteral("ECDH");
    case 19: return QStringLiteral("ECDSA");
    case 22: return QStringLiteral("EdDSA");
    case 25: return QStringLiteral("X25519");
    case 26: return QStringLiteral("X448");
    case 27: return QStringLiteral("Ed25519");
    case 28: return QStringLiteral("Ed448");
    default: break;
    }
    return QStringLiteral("%1").arg(algorithm);
}

// why a key packet of this kind is refused ("" when it may be read)
QString packetRefusal(const KeyPacket &k)
{
    if (k.version == 5 || k.version == 6)
        return say("It is a key of version %1 (Quillbinder uses keys of version 4).").arg(k.version);
    if (k.version != 4)
        return say("It is a key of version %1 (Quillbinder uses keys of version 4).").arg(k.version);
    if (k.algorithm == 18 || k.algorithm == 22)
        return QString(); // the curve is looked at once RNP has read the key
    if (k.algorithm >= 29 && k.algorithm != 100)
        return say("It uses an algorithm Quillbinder does not use (%1, possibly post-quantum).").arg(algorithmName(k.algorithm));
    return say("It is an %1 key (Quillbinder uses Ed25519 and Cv25519 keys only).").arg(algorithmName(k.algorithm));
}

// the armored blocks of a text, or the data itself when it is binary
QList<QByteArray> armorBlocks(const QByteArray &data)
{
    QList<QByteArray> blocks;
    qsizetype from = 0;
    for (;;) {
        const qsizetype begin = data.indexOf("-----BEGIN PGP ", from);
        if (begin < 0)
            break;
        const qsizetype endTag = data.indexOf("-----END PGP ", begin);
        if (endTag < 0) {
            blocks.append(data.mid(begin)); // cut off: RNP says so
            break;
        }
        qsizetype end = data.indexOf("-----", endTag + 13);
        end = end < 0 ? data.size() : end + 5;
        blocks.append(data.mid(begin, end - begin));
        from = end;
    }
    if (blocks.isEmpty() && !data.trimmed().isEmpty())
        blocks.append(data);
    return blocks;
}

} // namespace

#ifdef QB_HAVE_RNP

namespace {

QString fromRnp(char *s)
{
    const QString r = s ? QString::fromUtf8(s) : QString();
    rnp_buffer_destroy(s);
    return r;
}

QByteArray normalized(const QByteArray &passphrase)
{
    return QString::fromUtf8(passphrase).normalized(QString::NormalizationForm_C).toUtf8();
}

// a key handle that goes away with its scope
struct Key {
    rnp_key_handle_t h = nullptr;
    Key() = default;
    explicit Key(rnp_key_handle_t k) : h(k) {}
    Key(const Key &) = delete;
    Key &operator=(const Key &) = delete;
    Key(Key &&o) noexcept : h(o.h) { o.h = nullptr; }
    Key &operator=(Key &&o) noexcept
    {
        if (this != &o) {
            if (h)
                rnp_key_handle_destroy(h);
            h = o.h;
            o.h = nullptr;
        }
        return *this;
    }
    ~Key() { if (h) rnp_key_handle_destroy(h); }
    explicit operator bool() const { return h != nullptr; }
};

Key locate(rnp_ffi_t ffi, const char *type, const QString &id)
{
    rnp_key_handle_t h = nullptr;
    if (rnp_locate_key(ffi, type, id.toUpper().toLatin1().constData(), &h) != RNP_SUCCESS)
        return Key();
    return Key(h);
}

QString fingerprintOf(rnp_key_handle_t h)
{
    char *f = nullptr;
    rnp_key_get_fprint(h, &f);
    return fromRnp(f).toUpper();
}

QString stringOf(rnp_result_t (*get)(rnp_key_handle_t, char **), rnp_key_handle_t h)
{
    char *s = nullptr;
    if (get(h, &s) != RNP_SUCCESS)
        return QString();
    return fromRnp(s);
}

bool flag(rnp_result_t (*get)(rnp_key_handle_t, bool *), rnp_key_handle_t h)
{
    bool b = false;
    return get(h, &b) == RNP_SUCCESS && b;
}

// whether a key packet that RNP has read is of the one kind Quillbinder uses; why not, when asked
bool partOk(rnp_key_handle_t h, bool primary, QString *why)
{
    uint32_t version = 0;
    rnp_key_get_version(h, &version);
    const QString alg = stringOf(rnp_key_get_alg, h);
    const QString curve = stringOf(rnp_key_get_curve, h);
    bool ok = version == 4;
    if (primary)
        ok = ok && alg == QLatin1String("EDDSA") && curve == QLatin1String("Ed25519");
    else
        ok = ok && ((alg == QLatin1String("ECDH") && curve == QLatin1String("Curve25519"))
                    || (alg == QLatin1String("EDDSA") && curve == QLatin1String("Ed25519")));
    if (!ok && why) {
        if (version != 4)
            *why = say("It is a key of version %1 (Quillbinder uses keys of version 4).").arg(version);
        else if (!curve.isEmpty())
            *why = say("It uses the curve %1 (Quillbinder uses Ed25519 and Cv25519 keys only).").arg(curve);
        else
            *why = say("It is an %1 key (Quillbinder uses Ed25519 and Cv25519 keys only).").arg(alg);
    }
    return ok;
}

// the newest valid subkey of a primary key that encrypts (Cv25519)
Key encryptionSubkey(rnp_key_handle_t primary)
{
    size_t count = 0;
    rnp_key_get_subkey_count(primary, &count);
    Key best;
    uint32_t bestTime = 0;
    for (size_t i = 0; i < count; ++i) {
        rnp_key_handle_t s = nullptr;
        if (rnp_key_get_subkey_at(primary, i, &s) != RNP_SUCCESS)
            continue;
        Key sub(s);
        bool enc = false, valid = false;
        rnp_key_allows_usage(s, "encrypt", &enc);
        rnp_key_is_valid(s, &valid);
        uint32_t created = 0;
        rnp_key_get_creation(s, &created);
        if (enc && valid && partOk(s, false, nullptr) && stringOf(rnp_key_get_alg, s) == QLatin1String("ECDH")
            && (!best || created >= bestTime)) {
            bestTime = created;
            best = std::move(sub);
        }
    }
    return best;
}

std::vector<Key> subkeys(rnp_key_handle_t primary)
{
    std::vector<Key> list;
    size_t count = 0;
    rnp_key_get_subkey_count(primary, &count);
    for (size_t i = 0; i < count; ++i) {
        rnp_key_handle_t s = nullptr;
        if (rnp_key_get_subkey_at(primary, i, &s) == RNP_SUCCESS)
            list.emplace_back(s);
    }
    return list;
}

QByteArray exportKey(rnp_key_handle_t h, uint32_t flags)
{
    rnp_output_t out = nullptr;
    if (rnp_output_to_memory(&out, 0) != RNP_SUCCESS)
        return QByteArray();
    QByteArray r;
    if (rnp_key_export(h, out, flags | RNP_KEY_EXPORT_SUBKEYS) == RNP_SUCCESS) {
        uint8_t *buf = nullptr;
        size_t len = 0;
        rnp_output_memory_get_buf(out, &buf, &len, false);
        r = QByteArray(reinterpret_cast<const char *>(buf), qsizetype(len));
    }
    rnp_output_destroy(out);
    return r;
}

// the output of a decryption or an encryption: into a buffer (with a limit) or into a QSaveFile
struct Sink {
    QByteArray *buffer = nullptr;
    QSaveFile *file = nullptr;
    qint64 limit = -1;
    qint64 written = 0;
    bool tooLarge = false;
    bool failed = false;
};

bool sinkWrite(void *ctx, const void *buf, size_t len)
{
    auto *s = static_cast<Sink *>(ctx);
    if (s->limit >= 0 && s->written + qint64(len) > s->limit) {
        s->tooLarge = true;
        return false;
    }
    if (s->buffer) {
        s->buffer->append(static_cast<const char *>(buf), qsizetype(len));
    } else if (s->file) {
        if (s->file->write(static_cast<const char *>(buf), qint64(len)) != qint64(len)) {
            s->failed = true;
            return false;
        }
    }
    s->written += qint64(len);
    return true;
}

void sinkClose(void *, bool) {}

struct Source {
    QFile *file = nullptr;
    bool failed = false;
};

bool sourceRead(void *ctx, void *buf, size_t len, size_t *read)
{
    auto *s = static_cast<Source *>(ctx);
    const qint64 n = s->file->read(static_cast<char *>(buf), qint64(len));
    if (n < 0) {
        s->failed = true;
        return false;
    }
    *read = size_t(n);
    return true;
}

void sourceClose(void *) {}

// the password provider of a ring: never gives a passphrase (keys are unlocked with unlock()), but notes that one
// was asked for, which tells a locked key from a missing one
struct Asked {
    bool asked = false;
};

bool noPassphrase(rnp_ffi_t, void *ctx, rnp_key_handle_t, const char *, char *, size_t)
{
    if (ctx)
        static_cast<Asked *>(ctx)->asked = true;
    return false;
}

} // namespace

struct KeyRing::Private {
    rnp_ffi_t ffi = nullptr;
    mutable Asked asked;
    mutable QString detail;
    Private()
    {
        if (rnp_ffi_create(&ffi, RNP_KEYSTORE_GPG, RNP_KEYSTORE_GPG) != RNP_SUCCESS)
            ffi = nullptr;
        if (ffi)
            rnp_ffi_set_pass_provider(ffi, noPassphrase, &asked);
    }
    ~Private()
    {
        if (ffi)
            rnp_ffi_destroy(ffi);
    }
    void note(const char *what, rnp_result_t r) const
    {
        detail = QStringLiteral("%1: %2").arg(QLatin1String(what), QString::fromUtf8(rnp_result_to_string(r)));
    }
    Key primary(const QString &fingerprint) const
    {
        Key k = locate(ffi, "fingerprint", fingerprint);
        if (k && !flag(rnp_key_is_primary, k.h))
            return Key();
        return k;
    }
    // imports binary or armored key data as it is
    rnp_result_t importRaw(const QByteArray &data, uint32_t flags) const
    {
        rnp_input_t in = nullptr;
        rnp_result_t r = rnp_input_from_memory(&in, reinterpret_cast<const uint8_t *>(data.constData()), size_t(data.size()), false);
        if (r != RNP_SUCCESS)
            return r;
        char *results = nullptr;
        r = rnp_import_keys(ffi, in, flags, &results);
        rnp_buffer_destroy(results);
        rnp_input_destroy(in);
        return r;
    }
    // the encryption itself, from input to output (both made by the caller)
    Error runEncrypt(rnp_input_t in, rnp_output_t out, const QStringList &recipients, bool armored)
    {
        if (recipients.isEmpty())
            return Error::NoRecipient;
        rnp_op_encrypt_t op = nullptr;
        rnp_result_t r = rnp_op_encrypt_create(&op, ffi, in, out);
        if (r != RNP_SUCCESS) {
            note("encrypt", r);
            return Error::Failed;
        }
        Error result = Error::None;
        for (const QString &f : recipients) {
            const Key k = primary(f);
            if (!k) {
                result = Error::UnknownKey;
                break;
            }
            if (!partOk(k.h, true, nullptr) || !encryptionSubkey(k.h)) {
                result = Error::ForeignKey;
                break;
            }
            // RNP picks the key's subkey that encrypts
            r = rnp_op_encrypt_add_recipient(op, k.h);
            if (r != RNP_SUCCESS) {
                note("encrypt: recipient", r);
                result = Error::Failed;
                break;
            }
        }
        if (result == Error::None) {
            r = rnp_op_encrypt_set_cipher(op, "AES256");
            if (r == RNP_SUCCESS)
                r = rnp_op_encrypt_set_aead(op, "None"); // SEIPD version 1 with MDC
            if (r == RNP_SUCCESS)
                r = rnp_op_encrypt_set_compression(op, "Uncompressed", 0);
            if (r == RNP_SUCCESS)
                r = rnp_op_encrypt_set_armor(op, armored);
            if (r == RNP_SUCCESS)
                r = rnp_op_encrypt_execute(op);
            if (r != RNP_SUCCESS) {
                note("encrypt", r);
                result = Error::Failed;
            }
        }
        rnp_op_encrypt_destroy(op);
        return result;
    }

    Error runDecrypt(rnp_input_t in, rnp_output_t out, const Sink &sink, QStringList *recipients)
    {
        rnp_op_verify_t op = nullptr;
        rnp_result_t r = rnp_op_verify_create(&op, ffi, in, out);
        if (r != RNP_SUCCESS) {
            note("decrypt", r);
            return Error::Failed;
        }
        rnp_op_verify_set_flags(op, RNP_VERIFY_IGNORE_SIGS_ON_DECRYPT);
        asked.asked = false;
        r = rnp_op_verify_execute(op);
        char *mode = nullptr, *cipher = nullptr;
        bool valid = false;
        rnp_op_verify_get_protection_info(op, &mode, &cipher, &valid);
        const QString protection = fromRnp(mode);
        rnp_buffer_destroy(cipher);
        // the keys the message names; whether one of them is here, secret and unlocked (then a failure is the message's
        // fault, not a locked key's: RNP also asks for the other recipients' keys when the first one's text is damaged)
        QStringList named;
        size_t count = 0;
        rnp_op_verify_get_recipient_count(op, &count);
        for (size_t i = 0; i < count; ++i) {
            rnp_recipient_handle_t rh = nullptr;
            char *keyid = nullptr;
            if (rnp_op_verify_get_recipient_at(op, i, &rh) == RNP_SUCCESS && rnp_recipient_get_keyid(rh, &keyid) == RNP_SUCCESS)
                named.append(fromRnp(keyid).toUpper());
        }
        rnp_op_verify_destroy(op);
        bool openKey = false;
        for (const QString &id : std::as_const(named)) {
            const Key k = locate(ffi, "keyid", id);
            openKey = openKey || (k && flag(rnp_key_have_secret, k.h) && !flag(rnp_key_is_locked, k.h));
        }
        if (recipients)
            *recipients = named;
        Error result = Error::None;
        if (sink.tooLarge) {
            result = Error::TooLarge;
        } else if (sink.failed) {
            result = Error::Io;
        } else if (r != RNP_SUCCESS) {
            note("decrypt", r);
            if (asked.asked && !openKey)
                result = Error::Locked;
            else if (r == RNP_ERROR_NO_SUITABLE_KEY || r == RNP_ERROR_KEY_NOT_FOUND)
                result = Error::NoSecretKey;
            else
                result = Error::Damaged;
        } else if (protection == QLatin1String("cfb")) {
            result = Error::Unprotected; // no integrity check
        } else if (protection == QLatin1String("none") || protection.isEmpty() || !valid) {
            result = Error::Damaged; // not encrypted at all, or the check failed
        }
        return result;
    }
};

bool available() { return true; }

QString libraryVersion()
{
    return QStringLiteral("RNP %1, %2 %3").arg(QString::fromLatin1(rnp_version_string()),
                                               QString::fromLatin1(rnp_backend_string()),
                                               QString::fromLatin1(rnp_backend_version()));
}

KeyRing::KeyRing() : d(new Private) {}
KeyRing::~KeyRing() = default;

QString KeyRing::errorDetail() const { return d->detail; }

bool KeyRing::load(const QString &dir, QString *error)
{
    if (!d->ffi) {
        if (error)
            *error = errorText(Error::Unavailable);
        return false;
    }
    const struct { const char *name; uint32_t flags; } parts[] = {
        {"pubring.gpg", RNP_LOAD_SAVE_PUBLIC_KEYS}, {"secring.gpg", RNP_LOAD_SAVE_SECRET_KEYS}};
    for (const auto &part : parts) {
        QFile f(dir + QLatin1Char('/') + QLatin1String(part.name));
        if (!f.exists())
            continue;
        if (!f.open(QIODevice::ReadOnly)) {
            if (error)
                *error = QStringLiteral("%1: %2").arg(f.fileName(), f.errorString());
            return false;
        }
        const QByteArray data = f.readAll();
        if (data.isEmpty())
            continue;
        rnp_input_t in = nullptr;
        rnp_input_from_memory(&in, reinterpret_cast<const uint8_t *>(data.constData()), size_t(data.size()), false);
        const rnp_result_t r = rnp_load_keys(d->ffi, RNP_KEYSTORE_GPG, in, part.flags);
        rnp_input_destroy(in);
        if (r != RNP_SUCCESS) {
            d->note("load", r);
            if (error)
                *error = QStringLiteral("%1: %2").arg(f.fileName(), d->detail);
            return false;
        }
    }
    return true;
}

bool KeyRing::save(const QString &dir, QString *error) const
{
    if (!d->ffi) {
        if (error)
            *error = errorText(Error::Unavailable);
        return false;
    }
    if (!QDir().mkpath(dir)) {
        if (error)
            *error = QStringLiteral("%1: not made").arg(dir);
        return false;
    }
    QFile::setPermissions(dir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    const struct { const char *name; uint32_t flags; } parts[] = {
        {"pubring.gpg", RNP_LOAD_SAVE_PUBLIC_KEYS}, {"secring.gpg", RNP_LOAD_SAVE_SECRET_KEYS}};
    for (const auto &part : parts) {
        rnp_output_t out = nullptr;
        rnp_output_to_memory(&out, 0);
        const rnp_result_t r = rnp_save_keys(d->ffi, RNP_KEYSTORE_GPG, out, part.flags);
        QByteArray data;
        if (r == RNP_SUCCESS) {
            uint8_t *buf = nullptr;
            size_t len = 0;
            rnp_output_memory_get_buf(out, &buf, &len, false);
            data = QByteArray(reinterpret_cast<const char *>(buf), qsizetype(len));
        }
        rnp_output_destroy(out);
        if (r != RNP_SUCCESS) {
            d->note("save", r);
            if (error)
                *error = d->detail;
            return false;
        }
        const QString path = dir + QLatin1Char('/') + QLatin1String(part.name);
        if (!safesave::writeFile(path, data, error))
            return false;
        QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }
    return true;
}

QList<KeyInfo> KeyRing::keys() const
{
    QList<KeyInfo> list;
    if (!d->ffi)
        return list;
    rnp_identifier_iterator_t it = nullptr;
    if (rnp_identifier_iterator_create(d->ffi, &it, "fingerprint") != RNP_SUCCESS)
        return list;
    const char *id = nullptr;
    QStringList fingerprints;
    while (rnp_identifier_iterator_next(it, &id) == RNP_SUCCESS && id)
        fingerprints.append(QString::fromLatin1(id).toUpper());
    rnp_identifier_iterator_destroy(it);
    for (const QString &f : std::as_const(fingerprints)) {
        const KeyInfo k = key(f);
        if (!k.fingerprint.isEmpty())
            list.append(k);
    }
    return list;
}

KeyInfo KeyRing::key(const QString &fingerprint) const
{
    KeyInfo info;
    if (!d->ffi)
        return info;
    const Key k = d->primary(fingerprint);
    if (!k)
        return info;
    info.fingerprint = fingerprintOf(k.h);
    info.userId = stringOf(rnp_key_get_primary_uid, k.h);
    uint32_t created = 0;
    rnp_key_get_creation(k.h, &created);
    info.created = QDateTime::fromSecsSinceEpoch(created, QTimeZone(QTimeZone::UTC));
    const Key sub = encryptionSubkey(k.h);
    if (sub) {
        info.encryptionFingerprint = fingerprintOf(sub.h);
        info.secret = flag(rnp_key_have_secret, sub.h);
        info.unlocked = info.secret && !flag(rnp_key_is_locked, sub.h);
    } else {
        info.secret = flag(rnp_key_have_secret, k.h);
    }
    return info;
}

bool KeyRing::contains(const QString &fingerprint) const { return d->ffi && bool(d->primary(fingerprint)); }

bool KeyRing::hasSecret(const QString &fingerprint) const { return key(fingerprint).secret; }

QString KeyRing::createKey(const QString &userId, const QByteArray &passphrase, Error *error)
{
    auto fail = [&](Error e, const char *what, rnp_result_t r) {
        d->note(what, r);
        if (error)
            *error = e;
        return QString();
    };
    if (!d->ffi)
        return fail(Error::Unavailable, "create", RNP_ERROR_NOT_SUPPORTED);
    QByteArray pass = normalized(passphrase);
    const QByteArray uid = userId.toUtf8();
    rnp_op_generate_t op = nullptr;
    rnp_result_t r = rnp_op_generate_create(&op, d->ffi, "EDDSA");
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_userid(op, uid.constData());
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_expiration(op, 0);
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_protection_password(op, pass.constData());
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_protection_cipher(op, "AES256");
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_protection_hash(op, "SHA256");
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_execute(op);
    rnp_key_handle_t ph = nullptr;
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_get_key(op, &ph);
    if (op)
        rnp_op_generate_destroy(op);
    if (r != RNP_SUCCESS) {
        wipe(pass);
        return fail(Error::Failed, "create the primary key", r);
    }
    Key primary(ph);
    const QString fingerprint = fingerprintOf(primary.h);
    // the subkey's binding signature is made with the primary key: open it for that
    r = rnp_key_unlock(primary.h, pass.constData());
    op = nullptr;
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_subkey_create(&op, d->ffi, primary.h, "ECDH");
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_curve(op, "Curve25519");
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_expiration(op, 0);
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_protection_password(op, pass.constData());
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_protection_cipher(op, "AES256");
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_set_protection_hash(op, "SHA256");
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_execute(op);
    rnp_key_handle_t sh = nullptr;
    if (r == RNP_SUCCESS)
        r = rnp_op_generate_get_key(op, &sh);
    if (op)
        rnp_op_generate_destroy(op);
    rnp_key_lock(primary.h);
    if (r != RNP_SUCCESS) {
        rnp_key_remove(primary.h, RNP_KEY_REMOVE_PUBLIC | RNP_KEY_REMOVE_SECRET | RNP_KEY_REMOVE_SUBKEYS);
        wipe(pass);
        return fail(Error::Failed, "create the encryption subkey", r);
    }
    Key sub(sh);
    // GnuPG (and RFC 7748) expect a Cv25519 secret key whose low and high bits are already set; RNP 0.18 makes them so,
    // this only makes sure (the old fault of RNP issue 1018)
    bool tweaked = true;
    if (rnp_key_unlock(sub.h, pass.constData()) == RNP_SUCCESS) {
        rnp_key_25519_bits_tweaked(sub.h, &tweaked);
        rnp_key_lock(sub.h);
    }
    if (!tweaked && rnp_key_unprotect(sub.h, pass.constData()) == RNP_SUCCESS) {
        rnp_key_25519_bits_tweak(sub.h);
        rnp_key_protect(sub.h, pass.constData(), "AES256", nullptr, "SHA256", 0);
    }
    wipe(pass);
    if (error)
        *error = Error::None;
    return fingerprint;
}

QByteArray KeyRing::exportPublicKey(const QString &fingerprint, Error *error) const
{
    const Key k = d->ffi ? d->primary(fingerprint) : Key();
    if (error)
        *error = !d->ffi ? Error::Unavailable : !k ? Error::UnknownKey : Error::None;
    return k ? exportKey(k.h, RNP_KEY_EXPORT_ARMORED | RNP_KEY_EXPORT_PUBLIC) : QByteArray();
}

QByteArray KeyRing::exportSecretKey(const QString &fingerprint, Error *error) const
{
    const Key k = d->ffi ? d->primary(fingerprint) : Key();
    if (error)
        *error = !d->ffi ? Error::Unavailable : !k ? Error::UnknownKey : !flag(rnp_key_have_secret, k.h) ? Error::NoSecretKey : Error::None;
    return k && flag(rnp_key_have_secret, k.h) ? exportKey(k.h, RNP_KEY_EXPORT_ARMORED | RNP_KEY_EXPORT_SECRET) : QByteArray();
}

ImportReport KeyRing::importKeys(const QByteArray &data)
{
    ImportReport report;
    auto refuse = [&](const QString &fingerprint, Error e, const QString &why) {
        report.refused.append({fingerprint, why});
        if (report.error == Error::None)
            report.error = e;
    };
    if (!d->ffi) {
        refuse(QString(), Error::Unavailable, errorText(Error::Unavailable));
        return report;
    }
    const QList<QByteArray> blocks = armorBlocks(data);
    if (blocks.isEmpty())
        refuse(QString(), Error::NotAKey, errorText(Error::NotAKey));
    for (const QByteArray &block : blocks) {
        // binary packets: armored text is decoded first
        QByteArray binary = block;
        if (block.trimmed().startsWith("-----BEGIN PGP ")) {
            rnp_input_t in = nullptr;
            rnp_output_t out = nullptr;
            rnp_input_from_memory(&in, reinterpret_cast<const uint8_t *>(block.constData()), size_t(block.size()), false);
            rnp_output_to_memory(&out, 0);
            const rnp_result_t r = rnp_dearmor(in, out);
            binary.clear();
            if (r == RNP_SUCCESS) {
                uint8_t *buf = nullptr;
                size_t len = 0;
                rnp_output_memory_get_buf(out, &buf, &len, false);
                binary = QByteArray(reinterpret_cast<const char *>(buf), qsizetype(len));
            }
            rnp_input_destroy(in);
            rnp_output_destroy(out);
            if (r != RNP_SUCCESS) {
                refuse(QString(), Error::NotAKey, errorText(Error::NotAKey));
                continue;
            }
        }
        QList<KeyPacket> keyPackets;
        QList<QPair<qsizetype, qsizetype>> spans;
        QList<int> tags;
        if (!scanPackets(binary, &keyPackets, &spans, &tags) || keyPackets.isEmpty()) {
            refuse(QString(), Error::NotAKey, errorText(Error::NotAKey));
            continue;
        }
        // the transferable keys: from one primary key packet to the next; one whose packets are of another kind is
        // left out before RNP sees it
        QByteArray accepted;
        qsizetype groupStart = -1;
        QString groupRefusal, versionRefusal; // (a packet of another version is named first: it says the most)
        auto closeGroup = [&](qsizetype end) {
            if (groupStart < 0)
                return;
            if (groupRefusal.isEmpty() && versionRefusal.isEmpty())
                accepted += binary.mid(groupStart, end - groupStart);
            else
                refuse(QString(), Error::ForeignKey, versionRefusal.isEmpty() ? groupRefusal : versionRefusal);
            groupStart = -1;
            groupRefusal.clear();
            versionRefusal.clear();
        };
        int keyIndex = 0;
        for (int i = 0; i < spans.size(); ++i) {
            const int tag = tags.at(i);
            if (tag == 5 || tag == 6) {
                closeGroup(spans.at(i).first);
                groupStart = spans.at(i).first;
            }
            if (tag == 5 || tag == 6 || tag == 7 || tag == 14) {
                const KeyPacket &kp = keyPackets.at(keyIndex++);
                const QString why = packetRefusal(kp);
                if (!why.isEmpty() && kp.version != 4 && versionRefusal.isEmpty())
                    versionRefusal = why;
                else if (!why.isEmpty() && groupRefusal.isEmpty())
                    groupRefusal = why;
            }
        }
        closeGroup(binary.size());
        if (accepted.isEmpty())
            continue;
        // read by RNP into a ring of its own; each key checked there, then taken into this ring
        KeyRing scratch;
        const rnp_result_t r = scratch.d->importRaw(accepted, RNP_LOAD_SAVE_PUBLIC_KEYS | RNP_LOAD_SAVE_SECRET_KEYS);
        if (r != RNP_SUCCESS) {
            d->note("import", r);
            refuse(QString(), Error::NotAKey, errorText(Error::NotAKey));
            continue;
        }
        for (const KeyInfo &found : scratch.keys()) {
            const Key k = scratch.d->primary(found.fingerprint);
            QString why;
            bool ok = partOk(k.h, true, &why);
            const std::vector<Key> subs = subkeys(k.h);
            for (const Key &s : subs)
                ok = ok && partOk(s.h, false, &why);
            if (ok && flag(rnp_key_is_revoked, k.h)) {
                ok = false;
                why = say("The key was revoked.");
            }
            if (ok && !encryptionSubkey(k.h)) {
                ok = false;
                why = flag(rnp_key_is_expired, k.h) ? say("The key has expired.")
                                                     : say("The key has no valid subkey that encrypts (Cv25519).");
            }
            if (!ok) {
                refuse(found.fingerprint, Error::ForeignKey, why);
                continue;
            }
            // a secret part without a passphrase would lie open in the key ring: only the public part is taken
            bool secret = flag(rnp_key_have_secret, k.h);
            bool open = secret && !flag(rnp_key_is_protected, k.h);
            for (const Key &s : subs)
                if (flag(rnp_key_have_secret, s.h)) {
                    secret = true;
                    open = open || !flag(rnp_key_is_protected, s.h);
                }
            if (open) {
                report.refused.append({found.fingerprint, say("Its secret part has no passphrase and was not taken (its public part was).")});
                secret = false;
            }
            const QByteArray bytes = exportKey(k.h, secret ? RNP_KEY_EXPORT_SECRET : RNP_KEY_EXPORT_PUBLIC);
            const rnp_result_t ri = d->importRaw(bytes, RNP_LOAD_SAVE_PUBLIC_KEYS | RNP_LOAD_SAVE_SECRET_KEYS);
            QByteArray wiped = bytes;
            wipe(wiped);
            if (ri != RNP_SUCCESS) {
                d->note("import", ri);
                refuse(found.fingerprint, Error::Failed, errorText(Error::Failed));
                continue;
            }
            report.imported.append(found.fingerprint);
            if (secret)
                report.withSecret.append(found.fingerprint);
        }
    }
    if (report.imported.isEmpty() && report.error == Error::None)
        report.error = Error::NotAKey;
    return report;
}

bool KeyRing::removeKey(const QString &fingerprint, bool secretOnly)
{
    const Key k = d->ffi ? d->primary(fingerprint) : Key();
    if (!k)
        return false;
    // (RNP refuses to remove a secret part that is not there)
    bool secret = flag(rnp_key_have_secret, k.h);
    for (const Key &s : subkeys(k.h))
        secret = secret || flag(rnp_key_have_secret, s.h);
    if (secretOnly && !secret)
        return true;
    const uint32_t flags = (secretOnly ? 0 : RNP_KEY_REMOVE_PUBLIC) | (secret ? RNP_KEY_REMOVE_SECRET : 0) | RNP_KEY_REMOVE_SUBKEYS;
    const rnp_result_t r = rnp_key_remove(k.h, flags);
    if (r != RNP_SUCCESS)
        d->note("remove", r);
    return r == RNP_SUCCESS;
}

Error KeyRing::unlock(const QString &fingerprint, const QByteArray &passphrase)
{
    if (!d->ffi)
        return Error::Unavailable;
    const Key k = d->primary(fingerprint);
    if (!k)
        return Error::UnknownKey;
    QByteArray pass = normalized(passphrase);
    Error result = Error::NoSecretKey;
    const std::vector<Key> subs = subkeys(k.h);
    std::vector<rnp_key_handle_t> opened; // by this call (locked again when another part fails)
    for (const Key &s : subs) {
        bool enc = false;
        rnp_key_allows_usage(s.h, "encrypt", &enc);
        if (!enc || !flag(rnp_key_have_secret, s.h))
            continue;
        if (!flag(rnp_key_is_locked, s.h)) {
            if (result == Error::NoSecretKey)
                result = Error::None;
            continue;
        }
        const rnp_result_t r = rnp_key_unlock(s.h, pass.constData());
        if (r == RNP_ERROR_BAD_PASSWORD) {
            result = Error::WrongPassphrase;
            break;
        }
        if (r != RNP_SUCCESS) {
            d->note("unlock", r);
            result = Error::Failed;
            break;
        }
        opened.push_back(s.h);
        result = Error::None;
    }
    wipe(pass);
    if (result != Error::None)
        for (rnp_key_handle_t h : opened)
            rnp_key_lock(h);
    return result;
}

bool KeyRing::isUnlocked(const QString &fingerprint) const { return key(fingerprint).unlocked; }

void KeyRing::lock(const QString &fingerprint)
{
    const Key k = d->ffi ? d->primary(fingerprint) : Key();
    if (!k)
        return;
    rnp_key_lock(k.h);
    for (const Key &s : subkeys(k.h))
        rnp_key_lock(s.h);
}

void KeyRing::lockAll()
{
    for (const KeyInfo &k : keys())
        if (k.secret)
            lock(k.fingerprint);
}

Error KeyRing::changePassphrase(const QString &fingerprint, const QByteArray &oldPassphrase, const QByteArray &newPassphrase)
{
    if (!d->ffi)
        return Error::Unavailable;
    const Key k = d->primary(fingerprint);
    if (!k)
        return Error::UnknownKey;
    QByteArray oldPass = normalized(oldPassphrase), newPass = normalized(newPassphrase);
    std::vector<rnp_key_handle_t> parts;
    if (flag(rnp_key_have_secret, k.h))
        parts.push_back(k.h);
    const std::vector<Key> subs = subkeys(k.h);
    for (const Key &s : subs)
        if (flag(rnp_key_have_secret, s.h))
            parts.push_back(s.h);
    Error result = parts.empty() ? Error::NoSecretKey : Error::None;
    // first every part opened with the old passphrase, so that a wrong one changes nothing
    for (rnp_key_handle_t h : parts) {
        const bool wasLocked = flag(rnp_key_is_locked, h);
        const rnp_result_t r = rnp_key_unlock(h, oldPass.constData());
        if (wasLocked)
            rnp_key_lock(h);
        if (r != RNP_SUCCESS) {
            result = r == RNP_ERROR_BAD_PASSWORD ? Error::WrongPassphrase : Error::Failed;
            d->note("change passphrase", r);
            break;
        }
    }
    for (size_t i = 0; result == Error::None && i < parts.size(); ++i) {
        rnp_key_handle_t h = parts[i];
        const bool wasUnlocked = !flag(rnp_key_is_locked, h);
        rnp_result_t r = rnp_key_unprotect(h, oldPass.constData());
        if (r == RNP_SUCCESS)
            r = rnp_key_protect(h, newPass.constData(), "AES256", nullptr, "SHA256", 0);
        if (r != RNP_SUCCESS) {
            d->note("change passphrase", r);
            result = Error::Failed;
            break;
        }
        if (wasUnlocked)
            rnp_key_unlock(h, newPass.constData());
        else
            rnp_key_lock(h);
    }
    wipe(oldPass);
    wipe(newPass);
    return result;
}

QByteArray KeyRing::encrypt(const QByteArray &plain, const QStringList &recipients, Error *error, bool armored) const
{
    if (!d->ffi) {
        if (error)
            *error = Error::Unavailable;
        return QByteArray();
    }
    QByteArray cipher;
    Sink sink;
    sink.buffer = &cipher;
    rnp_input_t in = nullptr;
    rnp_output_t out = nullptr;
    rnp_input_from_memory(&in, reinterpret_cast<const uint8_t *>(plain.constData()), size_t(plain.size()), false);
    rnp_output_to_callback(&out, sinkWrite, sinkClose, &sink);
    const Error e = d->runEncrypt(in, out, recipients, armored);
    rnp_output_finish(out);
    rnp_output_destroy(out);
    rnp_input_destroy(in);
    if (error)
        *error = e;
    if (e != Error::None)
        cipher.clear();
    return cipher;
}

Error KeyRing::encryptFile(const QString &source, const QString &target, const QStringList &recipients) const
{
    if (!d->ffi)
        return Error::Unavailable;
    QFile from(source);
    if (!from.open(QIODevice::ReadOnly))
        return Error::Io;
    QSaveFile to(target);
    if (!to.open(QIODevice::WriteOnly))
        return Error::Io;
    Source src;
    src.file = &from;
    Sink sink;
    sink.file = &to;
    rnp_input_t in = nullptr;
    rnp_output_t out = nullptr;
    rnp_input_from_callback(&in, sourceRead, sourceClose, &src);
    rnp_output_to_callback(&out, sinkWrite, sinkClose, &sink);
    Error e = d->runEncrypt(in, out, recipients, false);
    rnp_output_finish(out);
    rnp_output_destroy(out);
    rnp_input_destroy(in);
    if (e == Error::None && (src.failed || sink.failed))
        e = Error::Io;
    if (e != Error::None) {
        to.cancelWriting();
        return e;
    }
    return safesave::commit(to) ? Error::None : Error::Io;
}

QByteArray KeyRing::decrypt(const QByteArray &cipher, Error *error, qint64 limit, QStringList *recipients) const
{
    if (!d->ffi) {
        if (error)
            *error = Error::Unavailable;
        return QByteArray();
    }
    QByteArray plain;
    Sink sink;
    sink.buffer = &plain;
    sink.limit = limit;
    rnp_input_t in = nullptr;
    rnp_output_t out = nullptr;
    rnp_input_from_memory(&in, reinterpret_cast<const uint8_t *>(cipher.constData()), size_t(cipher.size()), false);
    rnp_output_to_callback(&out, sinkWrite, sinkClose, &sink);
    const Error e = d->runDecrypt(in, out, sink, recipients);
    rnp_output_destroy(out);
    rnp_input_destroy(in);
    if (error)
        *error = e;
    if (e != Error::None)
        wipe(plain); // a part may have come before the integrity check failed
    return plain;
}

Error KeyRing::decryptFile(const QString &source, const QString &target) const
{
    if (!d->ffi)
        return Error::Unavailable;
    QFile from(source);
    if (!from.open(QIODevice::ReadOnly))
        return Error::Io;
    QSaveFile to(target);
    if (!to.open(QIODevice::WriteOnly))
        return Error::Io;
    Source src;
    src.file = &from;
    Sink sink;
    sink.file = &to;
    rnp_input_t in = nullptr;
    rnp_output_t out = nullptr;
    rnp_input_from_callback(&in, sourceRead, sourceClose, &src);
    rnp_output_to_callback(&out, sinkWrite, sinkClose, &sink);
    Error e = d->runDecrypt(in, out, sink, nullptr);
    rnp_output_destroy(out);
    rnp_input_destroy(in);
    if (e == Error::None && src.failed)
        e = Error::Io;
    if (e != Error::None) {
        to.cancelWriting();
        return e;
    }
    return safesave::commit(to) ? Error::None : Error::Io;
}

#else // no RNP in this build

struct KeyRing::Private {};

bool available() { return false; }
QString libraryVersion() { return QString(); }
KeyRing::KeyRing() : d(new Private) {}
KeyRing::~KeyRing() = default;
QString KeyRing::errorDetail() const { return errorText(Error::Unavailable); }
bool KeyRing::load(const QString &, QString *error)
{
    if (error)
        *error = errorText(Error::Unavailable);
    return false;
}
bool KeyRing::save(const QString &, QString *error) const
{
    if (error)
        *error = errorText(Error::Unavailable);
    return false;
}
QList<KeyInfo> KeyRing::keys() const { return {}; }
bool KeyRing::contains(const QString &) const { return false; }
KeyInfo KeyRing::key(const QString &) const { return {}; }
bool KeyRing::hasSecret(const QString &) const { return false; }
QString KeyRing::createKey(const QString &, const QByteArray &, Error *error)
{
    if (error)
        *error = Error::Unavailable;
    return QString();
}
QByteArray KeyRing::exportPublicKey(const QString &, Error *error) const
{
    if (error)
        *error = Error::Unavailable;
    return QByteArray();
}
QByteArray KeyRing::exportSecretKey(const QString &, Error *error) const
{
    if (error)
        *error = Error::Unavailable;
    return QByteArray();
}
ImportReport KeyRing::importKeys(const QByteArray &)
{
    ImportReport r;
    r.error = Error::Unavailable;
    r.refused.append({QString(), errorText(Error::Unavailable)});
    return r;
}
bool KeyRing::removeKey(const QString &, bool) { return false; }
Error KeyRing::unlock(const QString &, const QByteArray &) { return Error::Unavailable; }
bool KeyRing::isUnlocked(const QString &) const { return false; }
void KeyRing::lock(const QString &) {}
void KeyRing::lockAll() {}
Error KeyRing::changePassphrase(const QString &, const QByteArray &, const QByteArray &) { return Error::Unavailable; }
QByteArray KeyRing::encrypt(const QByteArray &, const QStringList &, Error *error, bool) const
{
    if (error)
        *error = Error::Unavailable;
    return QByteArray();
}
Error KeyRing::encryptFile(const QString &, const QString &, const QStringList &) const { return Error::Unavailable; }
QByteArray KeyRing::decrypt(const QByteArray &, Error *error, qint64, QStringList *) const
{
    if (error)
        *error = Error::Unavailable;
    return QByteArray();
}
Error KeyRing::decryptFile(const QString &, const QString &) const { return Error::Unavailable; }

#endif // QB_HAVE_RNP

QString KeyRing::deviceDir() { return dataDir() + QLatin1String("/keyring"); }

KeyRing &KeyRing::device()
{
    static KeyRing ring;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        ring.load(deviceDir());
    }
    return ring;
}

} // namespace crypto
} // namespace qb
