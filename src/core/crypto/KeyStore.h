#pragma once
// The system's key store (0.99.0, to-do item 113): the passphrase of the writer's key is remembered per device, so
// that it is typed once and the key is unlocked at the start of each session. Behind this small interface:
//   - system(): QtKeychain 0.18.0 (src/third_party/qtkeychain): Linux KWallet (kwalletd6 on Plasma 6, kwalletd5) or
//     the Secret Service (GNOME and others; libsecret loaded at run time), Windows Credential Manager, Android
//     Keystore. Never QtKeychain's "insecure fallback" (a plain file). A build without QtKeychain, or a system
//     without a key store: Status::NoBackend, and the writer types the passphrase once per session.
//   - memory(): kept in memory only, for the tests (builds with the test switches: QB_TEST_KEYSTORE=memory makes
//     system() one too, shared by the whole program run).
// The calls wait for the store's answer (the store may show a dialog of its own, e.g. KWallet asking for the wallet's
// password); meanwhile the program's windows are drawn, but take no input.
#include <QByteArray>
#include <QString>
#include <memory>

namespace qb {
namespace crypto {

class KeyStore {
public:
    enum class Status {
        Ok,
        NotFound,  // nothing kept under that name
        NoBackend, // no key store on this system (or in this build)
        Denied,    // the store refused (locked, or the writer said no)
        Failed,    // anything else (the store's words in *error)
    };
    virtual ~KeyStore() = default;
    // what it is, for the writer ("KWallet or the Secret Service", "Windows Credential Manager", ...)
    virtual QString name() const = 0;
    virtual Status write(const QString &entry, const QByteArray &secret, QString *error = nullptr) = 0;
    virtual Status read(const QString &entry, QByteArray *secret, QString *error = nullptr) = 0;
    virtual Status remove(const QString &entry, QString *error = nullptr) = 0;

    // the service name entries are kept under ("Quillbinder")
    static QString service();
    // the entry of a key's passphrase: "OpenPGP passphrase <fingerprint>"
    static QString passphraseEntry(const QString &fingerprint);
    // the system's key store (see above); the program keeps one
    static KeyStore &system();
    // a store in memory (a new, empty one each time)
    static std::unique_ptr<KeyStore> memory();
};

} // namespace crypto
} // namespace qb
