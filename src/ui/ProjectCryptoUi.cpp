// The window's part of the encrypted project (0.99.0, item 113, part 2): see ProjectCryptoUi.h.
#include "ui/ProjectCryptoUi.h"

#include "core/Project.h"
#include "core/SaveBatch.h"
#include "core/SyncSafety.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/KeyFolder.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/ProjectSwitches.h"
#include "ui/MainWindow.h"
#include "ui/OptionsDialog.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QTimer>
#include <QVBoxLayout>

#include <memory>

namespace qb {
namespace ui {

namespace {
QString say(const char *text) { return QCoreApplication::translate("qb::ui::ProjectCrypto", text); }

// the message for keys that are not on this device; true when keys were imported (try again)
bool missingKeys(QWidget *parent, const QString &what, const QString &text, const QStringList &needed)
{
    QMessageBox box(QMessageBox::Warning, what, text, QMessageBox::NoButton, parent);
    box.setObjectName(QStringLiteral("encryptionKeyMissing"));
    QPushButton *import = nullptr;
    if (crypto::importKeysHandler()) {
        import = box.addButton(say("Import Keys..."), QMessageBox::AcceptRole);
        import->setObjectName(QStringLiteral("encryptionImportKeys"));
    }
    box.addButton(QMessageBox::Cancel);
    box.exec();
    if (import && box.clickedButton() == import)
        return crypto::importKeysHandler()(parent, needed);
    return false;
}

// the secret keys of this device's ring whose encryption subkey is one of these key ids
QStringList keysFor(const QStringList &keyIds, bool *anyUnlocked)
{
    *anyUnlocked = false;
    QStringList out;
    std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
    for (const crypto::KeyInfo &k : crypto::KeyRing::device().keys()) {
        if (!k.secret || !keyIds.contains(k.encryptionFingerprint.right(16).toUpper()))
            continue;
        if (k.unlocked)
            *anyUnlocked = true;
        out << k.fingerprint;
    }
    return out;
}
} // namespace

bool unlockKey(QWidget *parent, const QString &fingerprint, const QString &what, const QString &why)
{
    crypto::KeyInfo info;
    {
        std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
        info = crypto::KeyRing::device().key(fingerprint);
        if (info.unlocked)
            return true;
    }
    QDialog dlg(parent);
    dlg.setObjectName(QStringLiteral("unlockDialog"));
    dlg.setWindowTitle(what);
    auto *lay = new QVBoxLayout(&dlg);
    auto *intro = new QLabel(why + QStringLiteral("\n\n")
                                 + say("Key: %1\n%2").arg(info.userId, crypto::groupedFingerprint(fingerprint)),
                             &dlg);
    intro->setWordWrap(true);
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lay->addWidget(intro);
    auto *pass = new QLineEdit(&dlg);
    pass->setObjectName(QStringLiteral("unlockPassphrase"));
    pass->setEchoMode(QLineEdit::Password);
    pass->setPlaceholderText(say("Passphrase"));
    lay->addWidget(pass);
    auto *wrong = new QLabel(&dlg);
    wrong->setObjectName(QStringLiteral("unlockWrong"));
    wrong->setStyleSheet(QStringLiteral("color: #c0392b;"));
    wrong->hide();
    lay->addWidget(wrong);
    auto *kept = new QLabel(say("Quillbinder keeps the passphrase in the key store of this device (%1), so that it is asked "
                                "for only once.")
                                .arg(crypto::KeyStore::system().name()),
                            &dlg);
    kept->setWordWrap(true);
    lay->addWidget(kept);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(say("Unlock"));
    lay->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    bool done = false;
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&] {
        QByteArray p = pass->text().toUtf8();
        const crypto::Error e = crypto::unlockAndRemember(fingerprint, p);
        crypto::wipe(p);
        pass->clear();
        if (e == crypto::Error::None) {
            done = true;
            dlg.accept();
            return;
        }
        wrong->setText(crypto::errorText(e));
        wrong->show();
        pass->setFocus();
    });
    dlg.resize(520, dlg.sizeHint().height());
    dlg.exec();
    return done;
}

bool unlockForFolder(QWidget *parent, const QString &projectFolder, const QString &what)
{
    for (int round = 0; round < 3; ++round) {
        QString fp;
        const crypto::Access a = crypto::accessFor(projectFolder, &fp);
        switch (a) {
        case crypto::Access::Plain:
        case crypto::Access::Ready:
            return true;
        case crypto::Access::NeedsPassphrase:
            return unlockKey(parent, fp, what, crypto::accessText(a, projectFolder));
        case crypto::Access::Unavailable: {
            QMessageBox box(QMessageBox::Warning, what, crypto::accessText(a, projectFolder), QMessageBox::Ok, parent);
            box.setObjectName(QStringLiteral("encryptionUnavailable"));
            box.exec();
            return false;
        }
        case crypto::Access::NoSecretKey:
            if (!missingKeys(parent, what, crypto::accessText(a, projectFolder), crypto::settingsOf(projectFolder).recipients.list()))
                return false;
            break; // keys came in: look again
        case crypto::Access::PublicKeyMissing: // 0.99.0 (encryption review H3): the same box; Import Keys brings the public key
            if (!missingKeys(parent, what, crypto::accessText(a, projectFolder), crypto::settingsOf(projectFolder).recipients.list()))
                return false;
            break;
        case crypto::Access::MarkerDamaged: { // 0.99.0 (encryption review M3)
            QMessageBox box(QMessageBox::Warning, what, crypto::accessText(a, projectFolder), QMessageBox::Ok, parent);
            box.setObjectName(QStringLiteral("encryptionMarkerDamaged"));
            box.exec();
            return false;
        }
        }
    }
    return false;
}

bool unlockForFile(QWidget *parent, const QString &file, const QString &what)
{
    const QStringList ids = crypto::messageKeyIds(file);
    if (ids.isEmpty())
        return true; // not encrypted (or not readable: the restore says so)
    for (int round = 0; round < 3; ++round) {
        bool unlocked = false;
        const QStringList keys = keysFor(ids, &unlocked);
        if (unlocked)
            return true;
        if (!keys.isEmpty())
            return unlockKey(parent, keys.first(), what,
                             say("%1 is encrypted. Enter the passphrase of your key to decrypt it.").arg(QFileInfo(file).fileName()));
        if (!missingKeys(parent, what,
                         say("%1 is encrypted to keys whose secret part is not on this device (key ids %2). Import your key "
                             "on this device to decrypt it. Nothing was changed.")
                             .arg(QFileInfo(file).fileName(), ids.join(QStringLiteral(", "))),
                         ids))
            return false;
    }
    return false;
}

bool finishConversion(QWidget *parent, const QString &projectFolder, QString *error)
{
    const crypto::ProjectSettings s = crypto::settingsOf(projectFolder);
    if (s.converting.isEmpty())
        return true;
    const QDateTime since = QDateTime::fromString(s.convertingSince, Qt::ISODate);
    const bool mine = s.convertingDevice == syncsafe::machineKey();
    if (!mine && since.isValid() && since.secsTo(QDateTime::currentDateTimeUtc()) < savebatch::kOtherSaveWait)
        return true; // another computer is at it (its files may still be arriving): it finishes it
    QProgressDialog dlg(s.converting == QLatin1String("encrypt") ? say("Encrypting the project's files...")
                                                                  : say("Decrypting the project's files..."),
                        say("Stop"), 0, 100, parent);
    dlg.setObjectName(QStringLiteral("encryptionProgress"));
    dlg.setWindowModality(Qt::WindowModal);
    dlg.setMinimumDuration(300);
    const bool ok = crypto::finishConversion(
        projectFolder,
        [&dlg](int done, int total) {
            dlg.setMaximum(qMax(1, total));
            dlg.setValue(done);
            QApplication::processEvents();
            return !dlg.wasCanceled();
        },
        error);
    dlg.reset();
    return ok;
}

bool changeProjectEncryption(MainWindow *window, bool on, const crypto::Recipients &recipients, QString *error)
{
    Project *p = window ? window->project() : nullptr;
    if (!p)
        return false;
    QString reason;
    if (!crypto::canChangeProjectEncryption(p, &reason)) {
        if (error)
            *error = reason;
        return false;
    }
    if (on == crypto::projectEncrypted(p) && crypto::settingsOf(p->folderPath()).converting.isEmpty())
        return true;
    const QString title = on ? say("Encrypt Project") : say("Decrypt Project");
    if (!on && !unlockForFolder(window, p->folderPath(), title)) {
        if (error)
            *error = crypto::errorText(crypto::Error::Locked);
        return false;
    }
    if (on && !recipients.isComplete()) {
        if (error)
            *error = say("The project needs the writer's key and the recovery key.");
        return false;
    }
    const QString folder = p->folderPath();
    const QString file = p->indexPath();
    if (!p->save(error))
        return false;
    if (!window->closeProject()) {
        if (error)
            *error = say("The project could not be closed, so its files were not converted.");
        return false;
    }
    QProgressDialog dlg(on ? say("Encrypting the project's files...") : say("Decrypting the project's files..."), say("Stop"), 0,
                        100, window);
    dlg.setObjectName(QStringLiteral("encryptionProgress"));
    dlg.setWindowModality(Qt::WindowModal);
    dlg.setMinimumDuration(300);
    auto progress = [&dlg](int done, int total) {
        dlg.setMaximum(qMax(1, total));
        dlg.setValue(done);
        QApplication::processEvents();
        return !dlg.wasCanceled();
    };
    const bool ok = on ? crypto::encryptFolder(folder, recipients, progress, error) : crypto::decryptFolder(folder, progress, error);
    dlg.reset();
    // opened again in whatever state it is in (a conversion that failed is finished at the next opening)
    window->openProject(file);
    return ok;
}

namespace {
// switch 2 asked for while File > Options is open: (folder, on), until the conversion runs
struct Pending {
    QString folder;
    bool on = false;
};
Pending &pending()
{
    static Pending p;
    return p;
}

crypto::Recipients deviceRecipients()
{
    const crypto::keyfolder::Roles r = crypto::keyfolder::roles();
    return crypto::Recipients{r.mine.toUpper(), r.recovery.toUpper()};
}

QSet<QString> secretKeysHere()
{
    QSet<QString> out;
    std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
    for (const crypto::KeyInfo &k : crypto::KeyRing::device().keys())
        if (k.secret)
            out.insert(k.fingerprint);
    return out;
}
} // namespace

void connectWindow(MainWindow *window)
{
    const QPointer<MainWindow> win(window);
    auto project = [win]() -> Project * { return win ? win->project() : nullptr; };
    crypto::ProjectSwitches &s = crypto::projectSwitches();
    s.projectOpen = [project] { return project() != nullptr; };
    s.isScrivenerProject = [project] {
        Project *p = project();
        return p && !p->scrivenerPath.isEmpty();
    };
    s.backupsEncrypted = [project] { return crypto::backupsEncrypted(project()); };
    s.projectEncrypted = [project] {
        Project *p = project();
        if (!p)
            return false;
        if (!pending().folder.isEmpty() && pending().folder == p->folderPath())
            return pending().on;
        return crypto::projectEncrypted(p);
    };
    s.setBackupsEncrypted = [project](bool on, QString *error) {
        Project *p = project();
        return p && crypto::setBackupsEncrypted(p, on, deviceRecipients(), error);
    };
    s.setProjectEncrypted = [win](bool on, QString *error) {
        Project *p = win ? win->project() : nullptr;
        if (!p)
            return false;
        QString reason;
        if (!crypto::canChangeProjectEncryption(p, &reason)) {
            if (error)
                *error = reason;
            return false;
        }
        const crypto::Recipients r = on ? deviceRecipients() : crypto::recipientsOf(p);
        if (on && !r.isComplete()) {
            if (error)
                *error = say("The project needs the writer's key and the recovery key.");
            return false;
        }
        pending() = Pending{p->folderPath(), on};
        auto run = std::make_shared<std::function<void()>>();
        *run = [win, on, r, run] {
            if (!win)
                return;
            if (QApplication::activeModalWidget()) { // File > Options is still open
                QTimer::singleShot(200, win, *run);
                return;
            }
            pending() = Pending();
            QString err;
            if (!changeProjectEncryption(win, on, r, &err) && !err.isEmpty() && !qEnvironmentVariableIsSet("QB_NO_DIALOGS"))
                QMessageBox::warning(win, on ? say("Encrypt Project") : say("Decrypt Project"), err);
        };
        QTimer::singleShot(0, win, *run);
        return true;
    };
    s.projectFolders = [project] {
        QStringList out;
        if (Project *p = project()) {
            out << p->folderPath();
            if (!p->scrivenerPath.isEmpty())
                out << p->scrivenerPath;
        }
        return out;
    };
    // Import Keys from the message of a key that is not here: File > Options > Encryption; true when a secret key came in
    crypto::setImportKeysHandler([](QWidget *parent, const QStringList &) {
        const QSet<QString> before = secretKeysHere();
        OptionsDialog::run(parent, [](OptionsDialog &d) { d.showSection(OptionsDialog::SEncryption); });
        return secretKeysHere() != before;
    });
}

} // namespace ui
} // namespace qb
