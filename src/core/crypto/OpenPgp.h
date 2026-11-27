#pragma once
// The OpenPGP layer of the built-in encryption (0.99.0, to-do item 113): encryption at rest of a project's files, in a
// format that GnuPG, Gpg4win and Kleopatra open without Quillbinder. A small layer over RNP (with Botan, linked
// statically: tools/get-crypto.sh); a build without RNP (Windows and Android until their parts add it) has the same
// interface, and every call fails with Error::Unavailable.
//
// The writer's decisions (item 113): keys of version 4 on Curve25519 only (an Ed25519 primary key that signs and
// certifies, a Cv25519 subkey that encrypts, no expiry); files SEIPD version 1 with MDC, AES-256, no AEAD, nothing of
// version 5 or 6, nothing post-quantum; every file encrypted to the writer's key and the recovery key; the secret key
// protected by RNP's iterated and salted S2K (AES-256, SHA-256, iterations tuned to about 150 ms) and unlocked once
// per session; encrypting needs only the public keys. A key of another kind is refused with a message. Both keys
// lost = the text is gone: there is no back door.
//
// A KeyRing is one RNP context (not thread-safe: use one from one thread). KeyRing::device() is the ring of this
// device in <data folder>/keyring (pubring.gpg, secring.gpg in GnuPG's old key ring format): the public keys of every
// project's recipients, and the writer's secret key only where it was made or imported on this device.
#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <memory>

namespace qb {
namespace crypto {

enum class Error {
    None,
    Unavailable,     // this build has no OpenPGP library
    WrongPassphrase, // unlock: the passphrase does not open the secret key
    Locked,          // decrypt: the secret key is here, but not unlocked
    NoSecretKey,     // decrypt: the file is encrypted to keys whose secret part is not here
    Damaged,         // not an OpenPGP message, cut off, or changed after it was written (integrity check failed)
    Unprotected,     // encrypted without an integrity check (old OpenPGP without MDC): refused
    ForeignKey,      // a key of another kind than v4 Ed25519/Cv25519 (RSA, v5, v6, post-quantum, other curves)
    NotAKey,         // the data holds no OpenPGP key
    UnknownKey,      // a fingerprint that is not in the ring
    NoRecipient,     // encrypt: no recipient given
    TooLarge,        // decrypt: the text would be larger than the limit
    Io,              // a file could not be read or written
    Failed,          // anything else (the library's own message is in errorDetail())
};

// a sentence for the writer (translatable), e.g. "The passphrase is not right."
QString errorText(Error error);

// whether this build has the OpenPGP library; its version ("RNP 0.18.1, Botan 3.14.1"), empty without
bool available();
QString libraryVersion();

// 40 hex digits in ten groups of four, a double space in the middle, as GnuPG and Kleopatra show them:
// "6925 BC66 3047 95E7 DAB8  FEBA 364A 438C 375B B5E0" (anything that is not 40 hex digits comes back unchanged)
QString groupedFingerprint(const QString &fingerprint);

// whether the data starts like an OpenPGP message (armored "-----BEGIN PGP MESSAGE-----", or a binary packet that
// opens an encrypted message: a public-key or symmetric-key session key)
bool looksEncrypted(const QByteArray &head);

struct KeyInfo {
    QString fingerprint;           // of the primary key, 40 upper-case hex digits
    QString encryptionFingerprint; // of its Cv25519 subkey that encrypts (the newest valid one)
    QString userId;                // the primary user id
    QDateTime created;
    bool secret = false;   // the secret part is in this ring
    bool unlocked = false; // ... and unlocked in this session
};

struct ImportReport {
    Error error = Error::None;        // None when every key in the data was taken; else the first refusal's kind
    QStringList imported;             // fingerprints of the keys taken (new or updated)
    QStringList withSecret;           // those of them that came with their secret part
    QList<QPair<QString, QString>> refused; // (fingerprint, or "" when the key could not be read; the reason, a sentence)
};

class KeyRing {
public:
    KeyRing();
    ~KeyRing();
    KeyRing(const KeyRing &) = delete;
    KeyRing &operator=(const KeyRing &) = delete;

    // the ring of this device, loaded from deviceDir() at the first call (a missing folder: an empty ring)
    static KeyRing &device();
    // <data folder>/keyring
    static QString deviceDir();
    // reads pubring.gpg and secring.gpg of dir into this ring (missing files: nothing read, no error)
    bool load(const QString &dir, QString *error = nullptr);
    // writes them safely (each file whole or not at all); the folder is made with access for its owner only
    bool save(const QString &dir, QString *error = nullptr) const;

    QList<KeyInfo> keys() const;
    bool contains(const QString &fingerprint) const;
    KeyInfo key(const QString &fingerprint) const; // an empty KeyInfo when not there
    bool hasSecret(const QString &fingerprint) const;

    // a new key pair (see above) with that user id, its secret part protected by passphrase (UTF-8, normalized to NFC
    // here); returns its fingerprint, empty on failure. Takes about half a second (S2K tuning for each part).
    QString createKey(const QString &userId, const QByteArray &passphrase, Error *error = nullptr);
    // armored keys (.asc): the public key with its subkeys; the secret key as stored (protected by its passphrase)
    QByteArray exportPublicKey(const QString &fingerprint, Error *error = nullptr) const;
    QByteArray exportSecretKey(const QString &fingerprint, Error *error = nullptr) const;
    // armored or binary keys, one or several (public or secret); a key of another kind is refused, the others are
    // taken. A secret key without a passphrase is not taken (its public part is).
    ImportReport importKeys(const QByteArray &data);
    // removes a key (secretOnly: only its secret part, e.g. the recovery key's after it was exported)
    bool removeKey(const QString &fingerprint, bool secretOnly = false);

    // opens the secret key's encryption subkeys with the passphrase for this session (until lock() or the ring ends)
    Error unlock(const QString &fingerprint, const QByteArray &passphrase);
    bool isUnlocked(const QString &fingerprint) const;
    void lock(const QString &fingerprint);
    void lockAll();
    // protects the secret key with a new passphrase (the old one must open it)
    Error changePassphrase(const QString &fingerprint, const QByteArray &oldPassphrase, const QByteArray &newPassphrase);

    // encrypts to the given primary keys (each must be in the ring, the writer's and the recovery key): SEIPD v1 with
    // MDC, AES-256, uncompressed, binary (armored: ASCII armor, for texts shown or mailed)
    QByteArray encrypt(const QByteArray &plain, const QStringList &recipients, Error *error = nullptr,
                       bool armored = false) const;
    // the same from file to file; the target is written safely (whole or not at all)
    Error encryptFile(const QString &source, const QString &target, const QStringList &recipients) const;
    // decrypts with an unlocked secret key of this ring; nothing comes back unless the whole message was read and its
    // integrity check passed. limit: the most bytes the text may have (a compressed message from elsewhere could
    // otherwise grow without end). recipients: the key ids (16 hex digits) the message names, when asked for.
    QByteArray decrypt(const QByteArray &cipher, Error *error = nullptr, qint64 limit = qint64(1) << 30,
                       QStringList *recipients = nullptr) const;
    // the same from file to file (no limit but the disk); the target is written only when all went well
    Error decryptFile(const QString &source, const QString &target) const;

    // the library's own words about the last failure of this ring (for logs, not for the writer)
    QString errorDetail() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};

// wipes a buffer that held a passphrase or a text (the compiler may not leave it out)
void wipe(QByteArray &data);

} // namespace crypto
} // namespace qb
