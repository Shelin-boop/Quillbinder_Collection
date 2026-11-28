#pragma once
// The Quillbinder wallet's passphrase dialogs (0.99.0, Live Sync): the key store's fallback where the system has no
// store. Creating asks for a new master passphrase twice and checks its strength as the key wizard does; unlocking asks
// once per program session. The dialogs are set through Wallet::setUnlockHandler (installWalletDialogs()).
#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;

namespace qb {
namespace livesync {

class WalletPassphraseDialog : public QDialog
{
    Q_OBJECT
public:
    // creating: a new wallet (the passphrase twice, with the check); else the existing wallet is unlocked
    explicit WalletPassphraseDialog(bool creating, QWidget *parent = nullptr);
    QString passphrase() const;
    bool creating() const { return m_creating; }

private:
    void update();
    bool m_creating;
    QLineEdit *m_one = nullptr;
    QLineEdit *m_two = nullptr;
    QLabel *m_strength = nullptr;
    QPushButton *m_ok = nullptr;
};

// sets Wallet::setUnlockHandler to the dialogs above (when no handler is set yet; the call is safe more than once). A
// call from another thread than the program's main one waits for the dialog on the main thread.
void installWalletDialogs();

// the line for the Live Sync and the Encryption pages: where this device keeps its keys (the system's key store or the
// Quillbinder wallet). Does not ask the key store for anything (no dialog of KWallet).
QString keyPlaceText();
// the same for a given store name (what KeyStore::name() says) and whether the wallet file exists (for the suites)
QString keyPlaceTextFor(const QString &storeName, bool walletExists, bool walletAvailable);

} // namespace livesync
} // namespace qb
