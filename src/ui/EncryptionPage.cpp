#include "ui/EncryptionPage.h"

#include "core/Backup.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/Passphrase.h"
#include "core/crypto/ProjectSwitches.h"
#include "ui/AppSettings.h"
#include "ui/HintLabel.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QRandomGenerator>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace qb {

namespace qc = qb::crypto;
namespace kf = qb::crypto::keyfolder;

namespace encui {

Hooks &hooks()
{
    static Hooks h;
    return h;
}

QStringList projectFolders()
{
    QStringList out = AppSettings::instance().recentProjects;
    if (qc::projectSwitches().projectFolders)
        out << qc::projectSwitches().projectFolders();
    return out;
}

QString backupFolder()
{
    const QString d = AppSettings::instance().backupDir;
    return d.isEmpty() ? backup::defaultDirectory() : d;
}

QString rememberPassphrase(const QString &fingerprint, const QByteArray &passphrase)
{
    QString err;
    const auto st = qc::KeyStore::system().write(qc::KeyStore::passphraseEntry(fingerprint), passphrase, &err);
    switch (st) {
    case qc::KeyStore::Status::Ok: return QString();
    case qc::KeyStore::Status::NoBackend:
        return QCoreApplication::translate("EncryptionPage", "This system has no key store, so the passphrase is asked for once in each session.");
    case qc::KeyStore::Status::Denied:
        return QCoreApplication::translate("EncryptionPage", "The system's key store refused to keep the passphrase, so it is asked for once in each session.");
    default:
        return QCoreApplication::translate("EncryptionPage", "The passphrase could not be kept in the system's key store, so it is asked for once in each session.");
    }
}

namespace {

QString pickFolderDialog(QWidget *parent, const QString &title)
{
    if (hooks().pickFolder)
        return hooks().pickFolder(parent, title);
    return QFileDialog::getExistingDirectory(parent, title);
}

void notifyBox(QWidget *parent, const QString &title, const QString &text, bool warning)
{
    if (hooks().notify) {
        hooks().notify(parent, title, text, warning);
        return;
    }
    if (warning)
        QMessageBox::warning(parent, title, text);
    else
        QMessageBox::information(parent, title, text);
}

bool confirmBox(QWidget *parent, const QString &title, const QString &text)
{
    if (hooks().confirm)
        return hooks().confirm(parent, title, text);
    return QMessageBox::question(parent, title, text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

Hooks::Answer askPassphraseBox(QWidget *parent, const QString &title, const QString &text)
{
    if (hooks().askPassphrase)
        return hooks().askPassphrase(parent, title, text);
    Hooks::Answer a;
    QDialog dlg(parent);
    dlg.setWindowTitle(title);
    auto *v = new QVBoxLayout(&dlg);
    auto *l = new QLabel(text, &dlg);
    l->setWordWrap(true);
    v->addWidget(l);
    auto *edit = new QLineEdit(&dlg);
    edit->setEchoMode(QLineEdit::Password);
    edit->setObjectName(QStringLiteral("encPassphrase"));
    v->addWidget(edit);
    auto *remember = new QCheckBox(QCoreApplication::translate("EncryptionPage", "Remember the passphrase on this device"), &dlg);
    remember->setChecked(true);
    v->addWidget(remember);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted) {
        a.ok = true;
        a.passphrase = edit->text();
        a.remember = remember->isChecked();
    }
    return a;
}

// a page that asks the wizard through functions (no Q_OBJECT needed)
class FnPage : public QWizardPage
{
public:
    std::function<bool()> complete;
    std::function<bool()> validate;
    std::function<void()> init;
    bool isComplete() const override { return complete ? complete() : true; }
    bool validatePage() override { return validate ? validate() : true; }
    void initializePage() override
    {
        if (init)
            init();
    }
    using QWizardPage::setCommitPage;
};

QString T(const char *s) { return QCoreApplication::translate("KeyWizard", s); }

QLabel *wrapLabel(const QString &text, QWidget *parent, const QString &name = QString())
{
    auto *l = new QLabel(text, parent);
    l->setWordWrap(true);
    if (!name.isEmpty())
        l->setObjectName(name);
    return l;
}

QFont bigFont(const QWidget *w)
{
    QFont f = w->font();
    f.setPointSizeF(f.pointSizeF() * 1.5);
    f.setBold(true);
    return f;
}

} // namespace
} // namespace encui

using encui::hooks;
using encui::T;

// ===================================================================================================== the wizard

KeyWizard::KeyWizard(QWidget *parent) : QWizard(parent)
{
    setObjectName(QStringLiteral("keyWizard"));
    setWindowTitle(tr("Create Keys"));
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::NoCancelButtonOnLastPage, false);
    m_date = hooks().today ? hooks().today() : QDate::currentDate();
    setPage(PassphrasePage, makePassphrasePage());
    setPage(PlacePage, makePlacePage());
    setPage(RecoveryPage, makeRecoveryPage());
    setPage(CheckPage, makeCheckPage());
    setPage(CopyPage, makeCopyPage());
    setButtonText(QWizard::CommitButton, tr("Make Keys"));
    resize(640, 480);
}

KeyWizard::~KeyWizard()
{
    qb::crypto::wipe(m_myPass);
    qb::crypto::wipe(m_recPass);
    m_recoveryText.fill(QLatin1Char(' '));
}

QString KeyWizard::chosenPassphrase() const
{
    if (auto *own = findChild<QRadioButton *>(QStringLiteral("wizUseOwn")); own && own->isChecked())
        return findChild<QLineEdit *>(QStringLiteral("wizOwn"))->text();
    return findChild<QLabel *>(QStringLiteral("wizWords"))->text();
}

void KeyWizard::newProposal()
{
    findChild<QLabel *>(QStringLiteral("wizWords"))->setText(qc::passphrase::generate(6));
}

QWizardPage *KeyWizard::makePassphrasePage()
{
    auto *p = new encui::FnPage;
    p->setTitle(tr("Your passphrase"));
    p->setSubTitle(tr("The passphrase locks your key. Quillbinder can remember it on this device, so you type it once on each "
                      "other device."));
    auto *v = new QVBoxLayout(p);
    auto *prop = new QRadioButton(tr("Use these six random words (the safest choice and easy to type):"), p);
    prop->setObjectName(QStringLiteral("wizUseProposed"));
    prop->setChecked(true);
    v->addWidget(prop);
    auto *words = new QLabel(p);
    words->setObjectName(QStringLiteral("wizWords"));
    words->setFont(encui::bigFont(p));
    words->setWordWrap(true);
    words->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(words);
    auto *other = new QPushButton(tr("Other words"), p);
    other->setObjectName(QStringLiteral("wizNewWords"));
    auto *row = new QHBoxLayout;
    row->addWidget(other);
    row->addStretch(1);
    v->addLayout(row);
    auto *own = new QRadioButton(tr("Use a passphrase of my own:"), p);
    own->setObjectName(QStringLiteral("wizUseOwn"));
    v->addWidget(own);
    auto *form = new QFormLayout;
    auto *own1 = new QLineEdit(p);
    own1->setObjectName(QStringLiteral("wizOwn"));
    own1->setEchoMode(QLineEdit::Password);
    auto *own2 = new QLineEdit(p);
    own2->setObjectName(QStringLiteral("wizOwn2"));
    own2->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Passphrase:"), own1);
    form->addRow(tr("Once more:"), own2);
    v->addLayout(form);
    v->addWidget(encui::wrapLabel(QString(), p, QStringLiteral("wizStrength")));
    auto *remember = new QCheckBox(tr("Remember the passphrase on this device (in the system's key store)"), p);
    remember->setObjectName(QStringLiteral("wizRemember"));
    remember->setChecked(true);
    v->addWidget(remember);
    const auto existing = kf::roles();
    if (!existing.mine.isEmpty())
        v->addWidget(encui::wrapLabel(tr("This device already has keys. New keys cannot open files that were locked for the old ones; "
                                         "keep the old key folder."),
                                      p, QStringLiteral("wizExisting")));
    v->addStretch(1);
    words->setText(qc::passphrase::generate(6));
    own1->setEnabled(false);
    own2->setEnabled(false);
    auto update = [this, p, prop, own1, own2]() {
        own1->setEnabled(!prop->isChecked());
        own2->setEnabled(!prop->isChecked());
        auto *s = findChild<QLabel *>(QStringLiteral("wizStrength"));
        if (prop->isChecked()) {
            s->clear();
        } else if (own1->text().isEmpty()) {
            s->clear();
        } else {
            const auto st = qc::passphrase::check(own1->text());
            if (!st.ok)
                s->setText(st.reason);
            else if (own1->text() != own2->text())
                s->setText(tr("The two entries are not the same."));
            else
                s->setText(tr("This passphrase is accepted."));
        }
        emit p->completeChanged();
    };
    connect(prop, &QRadioButton::toggled, this, update);
    connect(own1, &QLineEdit::textChanged, this, update);
    connect(own2, &QLineEdit::textChanged, this, update);
    connect(other, &QPushButton::clicked, this, [this] { newProposal(); });
    p->complete = [prop, own1, own2]() {
        if (prop->isChecked())
            return true;
        return qc::passphrase::check(own1->text()).ok && own1->text() == own2->text();
    };
    p->validate = [this]() {
        m_myPass = chosenPassphrase().toUtf8();
        return true;
    };
    return p;
}

void KeyWizard::updatePlaceMessage()
{
    auto *edit = findChild<QLineEdit *>(QStringLiteral("wizFolder"));
    auto *msg = findChild<QLabel *>(QStringLiteral("wizPlaceMessage"));
    if (!edit || !msg)
        return;
    const QString why = edit->text().trimmed().isEmpty()
                            ? QString()
                            : qb::crypto::keyfolder::refusedPlace(edit->text(), encui::projectFolders(), encui::backupFolder());
    msg->setText(why);
    if (auto *page = qobject_cast<QWizardPage *>(edit->parentWidget()))
        emit page->completeChanged();
}

QWizardPage *KeyWizard::makePlacePage()
{
    auto *p = new encui::FnPage;
    p->setTitle(tr("Where to keep the keys"));
    p->setSubTitle(tr("Quillbinder writes the folder \"%1\" there. A USB stick is best; project, backup and sync folders are "
                      "refused.")
                       .arg(kf::folderName(m_date)));
    p->setCommitPage(true);
    auto *v = new QVBoxLayout(p);
    auto *row = new QHBoxLayout;
    auto *edit = new QLineEdit(p);
    edit->setObjectName(QStringLiteral("wizFolder"));
    auto *choose = new QPushButton(tr("Choose..."), p);
    choose->setObjectName(QStringLiteral("wizChoose"));
    row->addWidget(edit, 1);
    row->addWidget(choose);
    v->addLayout(row);
    auto *msg = encui::wrapLabel(QString(), p, QStringLiteral("wizPlaceMessage"));
    msg->setStyleSheet(QStringLiteral("color: #b00020;"));
    v->addWidget(msg);
    v->addWidget(encui::wrapLabel(tr("When you press Next, your key and the recovery key are made. This takes a few seconds."), p));
    v->addStretch(1);
    connect(edit, &QLineEdit::textChanged, this, [this] { updatePlaceMessage(); });
    connect(choose, &QPushButton::clicked, this, [this, edit] {
        const QString d = encui::pickFolderDialog(this, tr("Folder for the keys"));
        if (!d.isEmpty())
            edit->setText(d);
    });
    p->complete = [edit]() {
        return !edit->text().trimmed().isEmpty()
               && kf::refusedPlace(edit->text(), encui::projectFolders(), encui::backupFolder()).isEmpty();
    };
    p->validate = [this]() { return makeKeysAndFolder(); };
    return p;
}

bool KeyWizard::makeKeysAndFolder()
{
    auto *edit = findChild<QLineEdit *>(QStringLiteral("wizFolder"));
    auto *msg = findChild<QLabel *>(QStringLiteral("wizPlaceMessage"));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_recoveryText = qc::passphrase::generate(6);
    m_recPass = m_recoveryText.toUtf8();
    const auto made = kf::makeKeys(m_ring, m_myPass, m_recPass);
    if (made.mine.isEmpty()) {
        QApplication::restoreOverrideCursor();
        msg->setText(made.error.isEmpty() ? tr("The keys could not be made.") : made.error);
        return false;
    }
    m_keys.mine = made.mine;
    m_keys.recovery = made.recovery;
    const auto w = kf::writeFolder(m_ring, m_keys, edit->text().trimmed(), m_date, true);
    QApplication::restoreOverrideCursor();
    if (!w.ok) {
        msg->setText(w.error);
        return false;
    }
    m_folder = w.folder;
    return true;
}

QWizardPage *KeyWizard::makeRecoveryPage()
{
    auto *p = new encui::FnPage;
    p->setTitle(tr("The recovery passphrase"));
    p->setSubTitle(tr("Write these six words on paper now. They are shown only this once and are saved nowhere, not even in "
                      "the key folder."));
    auto *v = new QVBoxLayout(p);
    auto *words = new QLabel(p);
    words->setObjectName(QStringLiteral("wizRecoveryWords"));
    words->setFont(encui::bigFont(p));
    words->setWordWrap(true);
    words->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(words);
    v->addWidget(encui::wrapLabel(tr("The recovery key opens your files if you lose your own key or forget its passphrase. Keep "
                                     "the paper in another place than the USB stick: the stick alone or the paper alone opens "
                                     "nothing. If both are lost, your text is gone for good."),
                                  p));
    auto *written = new QCheckBox(tr("I have written the six words on paper"), p);
    written->setObjectName(QStringLiteral("wizWritten"));
    v->addWidget(written);
    v->addStretch(1);
    connect(written, &QCheckBox::toggled, p, &QWizardPage::completeChanged);
    p->complete = [written]() { return written->isChecked(); };
    p->init = [this, words]() {
        words->setText(m_recoveryText);
        if (hooks().recoveryShown)
            hooks().recoveryShown(m_recoveryText);
    };
    return p;
}

QWizardPage *KeyWizard::makeCheckPage()
{
    auto *p = new encui::FnPage;
    p->setTitle(tr("The check"));
    p->setSubTitle(tr("Type two of the recovery words from your paper. Quillbinder reads the written files back and opens a test "
                      "file with each key."));
    auto *v = new QVBoxLayout(p);
    auto *form = new QFormLayout;
    auto *a = new QLineEdit(p);
    a->setObjectName(QStringLiteral("wizWord1"));
    auto *b = new QLineEdit(p);
    b->setObjectName(QStringLiteral("wizWord2"));
    auto *la = new QLabel(p);
    la->setObjectName(QStringLiteral("wizWord1Label"));
    auto *lb = new QLabel(p);
    lb->setObjectName(QStringLiteral("wizWord2Label"));
    form->addRow(la, a);
    form->addRow(lb, b);
    v->addLayout(form);
    auto *result = encui::wrapLabel(QString(), p, QStringLiteral("wizCheckResult"));
    v->addWidget(result);
    v->addStretch(1);
    connect(a, &QLineEdit::textChanged, p, &QWizardPage::completeChanged);
    connect(b, &QLineEdit::textChanged, p, &QWizardPage::completeChanged);
    p->init = [this]() { runCheck(); };
    p->complete = [this, a, b]() {
        const QStringList w = m_recoveryText.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (!m_checkOk || w.size() < 6)
            return false;
        return a->text().trimmed().compare(w.at(m_askA), Qt::CaseInsensitive) == 0
               && b->text().trimmed().compare(w.at(m_askB), Qt::CaseInsensitive) == 0;
    };
    p->validate = [this]() { return commitToDevice(); };
    return p;
}

void KeyWizard::runCheck()
{
    m_askA = int(QRandomGenerator::system()->bounded(3));      // word 1 to 3
    m_askB = 3 + int(QRandomGenerator::system()->bounded(3));  // word 4 to 6
    findChild<QLabel *>(QStringLiteral("wizWord1Label"))->setText(tr("Word %1:").arg(m_askA + 1));
    findChild<QLabel *>(QStringLiteral("wizWord2Label"))->setText(tr("Word %1:").arg(m_askB + 1));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const auto r = kf::checkFolder(m_folder, m_myPass, m_recPass);
    QApplication::restoreOverrideCursor();
    m_checkOk = r.ok();
    auto line = [](bool ok, const QString &text) { return (ok ? QStringLiteral("[OK] ") : QStringLiteral("[FAILED] ")) + text; };
    QStringList lines;
    lines << line(r.filesRead, tr("The key files were read back."));
    lines << line(r.readmeOk, tr("README.txt is in the folder."));
    lines << line(r.myKeyWorks, tr("A test file opens with your key and your passphrase."));
    lines << line(r.recoveryWorks, tr("A test file opens with the recovery key and the recovery passphrase."));
    if (!r.error.isEmpty() && !r.ok())
        lines << r.error;
    findChild<QLabel *>(QStringLiteral("wizCheckResult"))->setText(lines.join(QLatin1Char('\n')));
    if (auto *page = qobject_cast<QWizardPage *>(findChild<QLineEdit *>(QStringLiteral("wizWord1"))->parentWidget()))
        emit page->completeChanged();
}

bool KeyWizard::commitToDevice()
{
    qc::KeyRing &dev = qc::KeyRing::device();
    qc::Error e = qc::Error::None;
    const QByteArray mySecret = m_ring.exportSecretKey(m_keys.mine, &e);
    const QByteArray myPublic = m_ring.exportPublicKey(m_keys.mine, &e);
    const QByteArray recPublic = m_ring.exportPublicKey(m_keys.recovery, &e);
    if (mySecret.isEmpty() || myPublic.isEmpty() || recPublic.isEmpty() || dev.importKeys(mySecret).withSecret.isEmpty()
        || dev.importKeys(recPublic).imported.isEmpty()) {
        encui::notifyBox(this, tr("Create Keys"), tr("The keys could not be put on this device."), true);
        return false;
    }
    dev.importKeys(myPublic);
    QString err;
    dev.save(qc::KeyRing::deviceDir(), &err);
    kf::setRoles(m_keys);
    dev.unlock(m_keys.mine, m_myPass);
    auto *remember = findChild<QCheckBox *>(QStringLiteral("wizRemember"));
    if (remember && remember->isChecked()) {
        const QString why = encui::rememberPassphrase(m_keys.mine, m_myPass);
        if (!why.isEmpty())
            encui::notifyBox(this, tr("Create Keys"), why, false);
    }
    m_committed = true;
    return true;
}

QWizardPage *KeyWizard::makeCopyPage()
{
    auto *p = new encui::FnPage;
    p->setTitle(tr("A second copy"));
    p->setSubTitle(tr("Your keys are on this device and in the folder. Keep a second copy of the folder at another place, for "
                      "example on a second USB stick. You can skip this."));
    auto *v = new QVBoxLayout(p);
    auto *row = new QHBoxLayout;
    auto *edit = new QLineEdit(p);
    edit->setObjectName(QStringLiteral("wizCopyFolder"));
    auto *choose = new QPushButton(tr("Choose..."), p);
    choose->setObjectName(QStringLiteral("wizCopyChoose"));
    auto *make = new QPushButton(tr("Copy the folder here"), p);
    make->setObjectName(QStringLiteral("wizCopyMake"));
    row->addWidget(edit, 1);
    row->addWidget(choose);
    row->addWidget(make);
    v->addLayout(row);
    v->addWidget(encui::wrapLabel(QString(), p, QStringLiteral("wizCopyResult")));
    v->addStretch(1);
    connect(choose, &QPushButton::clicked, this, [this, edit] {
        const QString d = encui::pickFolderDialog(this, tr("Place for the second copy"));
        if (!d.isEmpty())
            edit->setText(d);
    });
    connect(make, &QPushButton::clicked, this, [this] { makeSecondCopy(); });
    return p;
}

void KeyWizard::makeSecondCopy()
{
    auto *edit = findChild<QLineEdit *>(QStringLiteral("wizCopyFolder"));
    auto *res = findChild<QLabel *>(QStringLiteral("wizCopyResult"));
    const QString parent = edit->text().trimmed();
    QString why = kf::refusedPlace(parent, encui::projectFolders(), encui::backupFolder());
    if (why.isEmpty() && QDir::cleanPath(QFileInfo(parent).absoluteFilePath()) == QDir::cleanPath(QFileInfo(m_folder).absolutePath()))
        why = tr("This is the place of the first copy. Choose another place.");
    if (why.isEmpty() && !m_copy.isEmpty())
        why = QString();
    if (!why.isEmpty()) {
        res->setText(why);
        return;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const auto w = kf::copyFolder(m_folder, parent);
    qb::crypto::keyfolder::CheckReport r;
    if (w.ok)
        r = kf::checkFolder(w.folder, m_myPass, m_recPass);
    QApplication::restoreOverrideCursor();
    if (!w.ok) {
        res->setText(w.error);
        return;
    }
    m_copy = w.folder;
    res->setText(r.ok() ? tr("The second copy is made and was checked: %1").arg(QDir::toNativeSeparators(w.folder))
                        : tr("The second copy was made, but the check failed. Make another one."));
}

// ===================================================================================================== the page

EncryptionPage::EncryptionPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("encryptionPage"));
    auto *v = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("Encryption locks the files of a project and its backups with your key, in the OpenPGP format that "
                                "Kleopatra, Gpg4win and GnuPG open without Quillbinder (see the manual's page on opening your "
                                "files without Quillbinder)."),
                             this);
    intro->setWordWrap(true);
    hintlabel::style(intro);
    v->addWidget(intro);

    auto *keys = new QGroupBox(tr("Keys on this device"), this);
    auto *kv = new QVBoxLayout(keys);
    m_list = new QTreeWidget(keys);
    m_list->setObjectName(QStringLiteral("encKeyList"));
    m_list->setRootIsDecorated(false);
    m_list->setColumnCount(3);
    m_list->setHeaderLabels({tr("Key"), tr("Fingerprint"), tr("Here")});
    m_list->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_list->setMinimumHeight(110);
    kv->addWidget(m_list);
    m_empty = new QLabel(keys);
    m_empty->setObjectName(QStringLiteral("encNoKeys"));
    m_empty->setWordWrap(true);
    kv->addWidget(m_empty);
    auto *row = new QHBoxLayout;
    auto button = [&](QPushButton *&b, const QString &text, const char *name, auto slot) {
        b = new QPushButton(text, keys);
        b->setObjectName(QLatin1String(name));
        row->addWidget(b);
        connect(b, &QPushButton::clicked, this, slot);
    };
    button(m_create, tr("Create Keys..."), "encCreate", [this] { createKeys(); });
    button(m_import, tr("Import Keys..."), "encImport", [this] { importKeys(); });
    button(m_importFile, tr("Import a Key File..."), "encImportFile", [this] { importKeyFile(); });
    button(m_export, tr("Export Keys..."), "encExport", [this] { exportKeys(); });
    button(m_forget, tr("Forget the passphrase on this device"), "encForget", [this] { forgetPassphrase(); });
    row->addStretch(1);
    kv->addLayout(row);
    v->addWidget(keys);

    auto *sw = new QGroupBox(tr("The open project"), this);
    auto *sv = new QVBoxLayout(sw);
    m_backups = new QCheckBox(tr("Encrypted backups"), sw);
    m_backups->setObjectName(QStringLiteral("encBackups"));
    m_project = new QCheckBox(tr("Encrypted project"), sw);
    m_project->setObjectName(QStringLiteral("encProject"));
    m_reason = new QLabel(sw);
    m_reason->setObjectName(QStringLiteral("encReason"));
    m_reason->setWordWrap(true);
    hintlabel::style(m_reason);
    sv->addWidget(m_backups);
    sv->addWidget(m_project);
    sv->addWidget(m_reason);
    v->addWidget(sw);
    v->addStretch(1);
    refresh();
}

void EncryptionPage::say(const QString &title, const QString &text, bool warning) { encui::notifyBox(this, title, text, warning); }

bool EncryptionPage::refusedHere(const QString &folder)
{
    const QString why = kf::refusedPlace(folder, encui::projectFolders(), encui::backupFolder());
    if (why.isEmpty())
        return false;
    say(tr("This place is not allowed"), why, true);
    return true;
}

void EncryptionPage::refresh()
{
    qc::KeyRing &ring = qc::KeyRing::device();
    const kf::Roles roles = kf::roles();
    m_list->clear();
    for (const qc::KeyInfo &k : ring.keys()) {
        QString role = tr("Other key");
        if (k.fingerprint == roles.mine)
            role = tr("My key");
        else if (k.fingerprint == roles.recovery)
            role = tr("Recovery key");
        QString here = k.secret ? (k.unlocked ? tr("secret part, unlocked") : tr("secret part, locked")) : tr("public part only");
        auto *it = new QTreeWidgetItem(m_list, {role, qc::groupedFingerprint(k.fingerprint), here});
        it->setData(0, Qt::UserRole, k.fingerprint);
    }
    m_empty->setVisible(m_list->topLevelItemCount() == 0);
    m_empty->setText(tr("This device has no keys yet. Create Keys makes your key and a recovery key; on another device, use Import Keys "
                        "with the key folder."));
    const bool haveMine = !roles.mine.isEmpty() && ring.hasSecret(roles.mine);
    m_export->setEnabled(haveMine);
    m_forget->setEnabled(haveMine);

    const qc::ProjectSwitches &ps = qc::projectSwitches();
    const bool open = ps.projectOpen && ps.projectOpen();
    const bool keysReady = roles.complete() && ring.contains(roles.mine) && ring.contains(roles.recovery);
    const bool scriv = open && ps.isScrivenerProject && ps.isScrivenerProject();
    QStringList reasons;
    m_backups->blockSignals(true);
    m_project->blockSignals(true);
    m_backups->setChecked(open && ps.backupsEncrypted && ps.backupsEncrypted());
    m_project->setChecked(open && ps.projectEncrypted && ps.projectEncrypted());
    m_backups->blockSignals(false);
    m_project->blockSignals(false);
    if (!open)
        reasons << tr("No project is open: the two switches belong to the open project.");
    else if (!keysReady)
        reasons << tr("Create or import keys first: the switches need your key and the recovery key.");
    if (scriv)
        reasons << tr("A project in Scrivener's format cannot be encrypted: Scrivener could not open its files. Its backups can be "
                      "encrypted.");
    m_backups->setEnabled(open && keysReady);
    m_project->setEnabled(open && keysReady && !scriv);
    m_reason->setText(reasons.join(QLatin1Char(' ')));
    m_reason->setVisible(!reasons.isEmpty());
}

bool EncryptionPage::apply(QStringList *errors)
{
    const qc::ProjectSwitches &ps = qc::projectSwitches();
    bool ok = true;
    auto one = [&](QCheckBox *box, const std::function<bool()> &get, const std::function<bool(bool, QString *)> &set) {
        if (!box->isEnabled() || !get || !set || get() == box->isChecked())
            return;
        QString err;
        if (!set(box->isChecked(), &err)) {
            ok = false;
            if (errors)
                *errors << (err.isEmpty() ? tr("The switch \"%1\" could not be changed.").arg(box->text()) : err);
        }
    };
    one(m_backups, ps.backupsEncrypted, ps.setBackupsEncrypted);
    one(m_project, ps.projectEncrypted, ps.setProjectEncrypted);
    refresh();
    return ok;
}

void EncryptionPage::createKeys()
{
    KeyWizard wiz(this);
    wiz.exec();
    refresh();
}

void EncryptionPage::importKeys()
{
    const QString folder = encui::pickFolderDialog(this, tr("Choose the key folder"));
    if (folder.isEmpty())
        return;
    const auto a = encui::askPassphraseBox(this, tr("Import Keys"), tr("Type the passphrase of your key."));
    if (!a.ok)
        return;
    qc::KeyRing &ring = qc::KeyRing::device();
    const QByteArray pass = a.passphrase.toUtf8();
    const auto r = kf::importFolder(ring, folder, pass);
    if (!r.ok) {
        say(tr("Import Keys"), r.error, true);
        return;
    }
    kf::Roles roles = kf::roles();
    roles.mine = r.mine;
    if (!r.recovery.isEmpty())
        roles.recovery = r.recovery;
    kf::setRoles(roles);
    QString err;
    ring.save(qc::KeyRing::deviceDir(), &err);
    QString text = tr("Your key is on this device now.");
    if (a.remember) {
        const QString why = encui::rememberPassphrase(r.mine, pass);
        if (!why.isEmpty())
            text += QLatin1Char(' ') + why;
    }
    if (!r.notes.isEmpty())
        text += QLatin1Char('\n') + r.notes.join(QLatin1Char('\n'));
    say(tr("Import Keys"), text);
    refresh();
}

void EncryptionPage::importKeyFile()
{
    const QString file = [&] {
        if (hooks().pickFile)
            return hooks().pickFile(this, tr("Choose a key file"));
        return QFileDialog::getOpenFileName(this, tr("Choose a key file"), QString(),
                                            tr("Key files (*.asc *.gpg *.pgp);;All files (*)"));
    }();
    if (file.isEmpty())
        return;
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        say(tr("Import a Key File"), tr("The file could not be read."), true);
        return;
    }
    const QByteArray data = f.readAll();
    // a secret key needs its passphrase; ask only then (a public key is read without)
    qc::KeyRing probe;
    const qc::ImportReport pr = probe.importKeys(data);
    QByteArray pass;
    bool remember = false;
    if (!pr.withSecret.isEmpty()) {
        const auto a = encui::askPassphraseBox(this, tr("Import a Key File"), tr("Type the passphrase of this key."));
        if (!a.ok)
            return;
        pass = a.passphrase.toUtf8();
        remember = a.remember;
    }
    qc::KeyRing &ring = qc::KeyRing::device();
    const auto r = kf::importKeyFile(ring, data, pass);
    if (!r.ok) {
        say(tr("Import a Key File"), r.error, true);
        return;
    }
    kf::Roles roles = kf::roles();
    QString text;
    if (!r.mine.isEmpty()) {
        roles.mine = r.mine;
        text = tr("The key is on this device as your key.");
        if (remember) {
            const QString why = encui::rememberPassphrase(r.mine, pass);
            if (!why.isEmpty())
                text += QLatin1Char(' ') + why;
        }
        if (roles.recovery.isEmpty())
            text += QLatin1Char(' ') + tr("There is no recovery key yet: import its public file (recovery-key-public.asc) with Import "
                                          "a Key File, or make a new pair with Create Keys.");
    } else if (!roles.mine.isEmpty() && roles.recovery.isEmpty()) {
        roles.recovery = r.recovery;
        text = tr("The key is on this device as the recovery key.");
    } else {
        text = tr("The public key is on this device. It is not your key and not the recovery key.");
    }
    kf::setRoles(roles);
    QString err;
    ring.save(qc::KeyRing::deviceDir(), &err);
    if (!r.notes.isEmpty())
        text += QLatin1Char('\n') + r.notes.join(QLatin1Char('\n'));
    say(tr("Import a Key File"), text);
    refresh();
}

void EncryptionPage::exportKeys()
{
    const QString parent = encui::pickFolderDialog(this, tr("Where to write the keys"));
    if (parent.isEmpty() || refusedHere(parent))
        return;
    const QDate today = hooks().today ? hooks().today() : QDate::currentDate();
    const auto w = kf::exportKeys(qc::KeyRing::device(), kf::roles(), parent, today);
    if (!w.ok) {
        say(tr("Export Keys"), w.error, true);
        return;
    }
    say(tr("Export Keys"), tr("The keys are in the folder %1. It holds your key (locked by your passphrase), the public parts and a "
                              "README.txt. The recovery key's secret part is only in the folder that Create Keys made.")
                               .arg(QDir::toNativeSeparators(w.folder)));
}

void EncryptionPage::forgetPassphrase()
{
    const kf::Roles roles = kf::roles();
    if (roles.mine.isEmpty())
        return;
    if (!encui::confirmBox(this, tr("Forget the passphrase"),
                           tr("Quillbinder forgets the passphrase on this device and locks your key. It will ask for the passphrase "
                              "again when it needs it. Continue?")))
        return;
    QString err;
    const auto st = qc::KeyStore::system().remove(qc::KeyStore::passphraseEntry(roles.mine), &err);
    qc::KeyRing::device().lock(roles.mine);
    if (st == qc::KeyStore::Status::Ok || st == qc::KeyStore::Status::NotFound)
        say(tr("Forget the passphrase"), tr("The passphrase is forgotten on this device."));
    else
        say(tr("Forget the passphrase"), tr("The system's key store would not remove the passphrase: %1").arg(err), true);
    refresh();
}

} // namespace qb
