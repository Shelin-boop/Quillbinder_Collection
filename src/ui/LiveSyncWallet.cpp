#include "ui/LiveSyncWallet.h"

#include "core/crypto/KeyStore.h"
#include "core/crypto/Passphrase.h"
#include "core/crypto/Wallet.h"
#include "ui/HintLabel.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>

namespace qb {
namespace livesync {

namespace qc = qb::crypto;

WalletPassphraseDialog::WalletPassphraseDialog(bool creating, QWidget *parent) : QDialog(parent), m_creating(creating)
{
    setObjectName(QStringLiteral("walletDialog"));
    setWindowTitle(creating ? tr("Create the Quillbinder Wallet") : tr("Unlock the Quillbinder Wallet"));
    auto *v = new QVBoxLayout(this);
    auto *intro = new QLabel(creating ? tr("This device has no system key store, so Quillbinder keeps its keys in its own wallet, "
                                           "a file locked with a master passphrase. Choose the passphrase now; it is asked once "
                                           "each time you start Quillbinder. It is never stored, and nobody can recover it for "
                                           "you: if you lose it, the keys in the wallet are lost.")
                                        : tr("Type the master passphrase of the Quillbinder wallet. It is asked once each time you "
                                             "start Quillbinder."),
                             this);
    intro->setWordWrap(true);
    v->addWidget(intro);
    auto *form = new QFormLayout;
    m_one = new QLineEdit(this);
    m_one->setObjectName(QStringLiteral("walletPass"));
    m_one->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Passphrase:"), m_one);
    if (creating) {
        m_two = new QLineEdit(this);
        m_two->setObjectName(QStringLiteral("walletPass2"));
        m_two->setEchoMode(QLineEdit::Password);
        form->addRow(tr("Once more:"), m_two);
    }
    v->addLayout(form);
    m_strength = new QLabel(this);
    m_strength->setObjectName(QStringLiteral("walletStrength"));
    m_strength->setWordWrap(true);
    v->addWidget(m_strength);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_ok = box->button(QDialogButtonBox::Ok);
    m_ok->setObjectName(QStringLiteral("walletOk"));
    v->addWidget(box);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_one, &QLineEdit::textChanged, this, [this] { update(); });
    if (m_two)
        connect(m_two, &QLineEdit::textChanged, this, [this] { update(); });
    update();
}

QString WalletPassphraseDialog::passphrase() const { return m_one->text(); }

void WalletPassphraseDialog::update()
{
    if (!m_creating) {
        m_ok->setEnabled(!m_one->text().isEmpty());
        return;
    }
    if (m_one->text().isEmpty()) {
        m_strength->clear();
        m_ok->setEnabled(false);
        return;
    }
    const auto st = qc::passphrase::check(m_one->text());
    if (!st.ok) {
        m_strength->setText(st.reason);
        m_ok->setEnabled(false);
    } else if (m_one->text() != m_two->text()) {
        m_strength->setText(tr("The two entries are not the same."));
        m_ok->setEnabled(false);
    } else {
        m_strength->setText(tr("This passphrase is accepted."));
        m_ok->setEnabled(true);
    }
}

namespace {

bool askOnMainThread(bool creating, QString *passphrase)
{
    if (!QCoreApplication::instance() || !qobject_cast<QApplication *>(QCoreApplication::instance()))
        return false; // no windows: the wallet stays locked
    WalletPassphraseDialog d(creating, QApplication::activeWindow());
    if (d.exec() != QDialog::Accepted)
        return false;
    *passphrase = d.passphrase();
    return true;
}

} // namespace

void installWalletDialogs()
{
    if (qc::Wallet::hasUnlockHandler())
        return;
    qc::Wallet::setUnlockHandler([](bool creating, QString *passphrase) -> bool {
        if (QThread::currentThread() == QCoreApplication::instance()->thread())
            return askOnMainThread(creating, passphrase);
        bool ok = false;
        QMetaObject::invokeMethod(
            QCoreApplication::instance(), [&] { ok = askOnMainThread(creating, passphrase); }, Qt::BlockingQueuedConnection);
        return ok;
    });
}

QString keyPlaceText()
{
    return keyPlaceTextFor(qc::KeyStore::system().name(), qc::Wallet::standard().exists(), qc::Wallet::available());
}

QString keyPlaceTextFor(const QString &name, bool walletExists, bool walletAvailable)
{
    const bool wallet = name == QCoreApplication::translate("qb::crypto", "The Quillbinder wallet") || name == QLatin1String("no key store")
                        || name == QCoreApplication::translate("qb::crypto", "no key store");
    if (wallet) {
        if (!walletAvailable)
            return QCoreApplication::translate("LiveSyncWallet", "This device has no place to keep keys: the system has no key store and this build has no wallet.");
        return QCoreApplication::translate("LiveSyncWallet", "Keys are kept in the Quillbinder wallet: a file in the program's data folder, "
                                                             "locked with a master passphrase that is asked once each time you start Quillbinder.");
    }
    QString text = QCoreApplication::translate("LiveSyncWallet", "Keys are kept in the system's key store (%1); where a device has none, "
                                                                 "Quillbinder keeps them in its own wallet, locked with a master passphrase.")
                       .arg(name);
    if (walletExists)
        text += QLatin1Char(' ') + QCoreApplication::translate("LiveSyncWallet", "This device has a wallet already.");
    return text;
}

} // namespace livesync
} // namespace qb
