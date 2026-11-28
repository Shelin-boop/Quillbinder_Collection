#pragma once
// The Quillbinder wallet (0.99.0, Live Sync): the key store's fallback on every system. One file in the program's data
// folder holds the entries (the device key of Live Sync, and where no system key store exists the encryption
// passphrase that KeyStore keeps). The file is sealed with AES-256/GCM under a key made from a master passphrase by
// Argon2id (Botan, already linked); the passphrase is asked once per program session, is never stored, and the entries
// are never written in plain form. The system's key store (KWallet, Secret Service, Credential Manager, Android
// Keystore) stays first where there is one: KeyStore::system() asks this wallet only when the system has no store.
//
// The window code sets the unlock handler (a dialog asking for the passphrase, or for a new one when the wallet does
// not exist yet); without a handler the wallet stays locked and a KeyStore call says "no key store", as before.
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>

namespace qb {
namespace crypto {

class Wallet {
public:
    enum class Result {
        Ok,
        NotFound,        // no such entry
        Locked,          // not unlocked in this session
        WrongPassphrase, // unlock: the passphrase does not open the file (or the file is damaged: see error)
        Exists,          // create: there is a wallet already
        Failed,          // anything else (the words in *error)
    };

    // a wallet kept in <file>; the program's own is standard()
    explicit Wallet(const QString &file);
    ~Wallet();
    Wallet(const Wallet &) = delete;
    Wallet &operator=(const Wallet &) = delete;

    // false in a build without Botan's Argon2 and AES-GCM (an old encryption pack): every call then fails
    static bool available();
    static Wallet &standard();
    static QString standardPath();

    QString filePath() const;
    bool exists() const;
    bool isUnlocked() const;

    // a new, empty wallet under this master passphrase (stays unlocked); not over an existing file
    Result create(const QString &passphrase, QString *error = nullptr);
    Result unlock(const QString &passphrase, QString *error = nullptr);
    // forgets the key and the entries (overwritten in memory)
    void lock();
    // the file is made anew under the new passphrase (fresh salt); the wallet must be unlocked
    Result changePassphrase(const QString &oldPassphrase, const QString &newPassphrase, QString *error = nullptr);

    Result write(const QString &entry, const QByteArray &secret, QString *error = nullptr);
    Result read(const QString &entry, QByteArray *secret, QString *error = nullptr) const;
    Result remove(const QString &entry, QString *error = nullptr);
    QStringList entries() const;

    // what the handler is asked: creating = there is no wallet yet (ask for a new passphrase and its repetition);
    // returns false when the writer cancels
    using UnlockHandler = std::function<bool(bool creating, QString *passphrase)>;
    static void setUnlockHandler(UnlockHandler handler);
    static bool hasUnlockHandler();
    // unlocks (or creates) through the handler; Ok when it is unlocked afterwards. Locked: no handler or cancelled.
    Result ensureUnlocked(QString *error = nullptr);

    // Argon2id cost in the file's header; the limits a file may name (a damaged or planted file cannot make the
    // program allocate more): memory in KiB, passes, lanes
    static constexpr quint32 kMaxMemoryKiB = 256 * 1024;
    static constexpr quint32 kMaxPasses = 10;
    static constexpr quint32 kMaxLanes = 4;

    struct Impl; // (inside: Wallet.cpp)

private:
    std::unique_ptr<Impl> d;
};

} // namespace crypto
} // namespace qb
