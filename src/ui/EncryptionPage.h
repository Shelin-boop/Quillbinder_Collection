#pragma once
// File > Options > Encryption (0.99.0, to-do item 113, part 3): the keys of this device, the two switches of the open
// project, Create Keys (the key wizard), Import Keys, Import a Key File, Export Keys and "Forget the passphrase on this
// device". The files and keys are made by core/crypto/KeyFolder.*; the switches' logic is the project layer's
// (core/crypto/ProjectSwitches.h).
#include "core/crypto/KeyFolder.h"

#include <QDate>
#include <QString>
#include <QWidget>
#include <QWizard>

#include <functional>

class QCheckBox;
class QLabel;
class QPushButton;
class QTreeWidget;

namespace qb {

namespace encui {

// What opens a dialog of the system or asks the writer. The test suites set these (a folder chosen without a dialog,
// the passphrase typed without a box); the program leaves them empty.
struct Hooks {
    std::function<QString(QWidget *parent, const QString &title)> pickFolder;  // "" = canceled
    std::function<QString(QWidget *parent, const QString &title)> pickFile;
    struct Answer {
        bool ok = false;
        QString passphrase;
        bool remember = true;
    };
    std::function<Answer(QWidget *parent, const QString &title, const QString &text)> askPassphrase;
    std::function<void(QWidget *parent, const QString &title, const QString &text, bool warning)> notify;
    std::function<bool(QWidget *parent, const QString &title, const QString &text)> confirm;
    std::function<void(const QString &)> recoveryShown;  // the recovery passphrase, when the wizard shows it
    std::function<QDate()> today;
};
Hooks &hooks();

// the folders keys must not be written into: the open project's, the recent projects', the backup folder is separate
QStringList projectFolders();
QString backupFolder();
// writes the passphrase of the writer's key into the system's key store ("" when it was kept, else why not)
QString rememberPassphrase(const QString &fingerprint, const QByteArray &passphrase);

} // namespace encui

// Create Keys: the passphrase, the place, the recovery passphrase on paper, the check, a second copy.
class KeyWizard : public QWizard
{
    Q_OBJECT
public:
    enum PageId { PassphrasePage, PlacePage, RecoveryPage, CheckPage, CopyPage };
    explicit KeyWizard(QWidget *parent = nullptr);
    ~KeyWizard() override;

    QString folder() const { return m_folder; }     // the key folder, once written
    QString copyFolder() const { return m_copy; }   // the second copy, once made
    bool committed() const { return m_committed; }  // the keys are on this device

private:
    QWizardPage *makePassphrasePage();
    QWizardPage *makePlacePage();
    QWizardPage *makeRecoveryPage();
    QWizardPage *makeCheckPage();
    QWizardPage *makeCopyPage();
    QString chosenPassphrase() const;
    void newProposal();
    void updatePlaceMessage();
    bool makeKeysAndFolder();
    void runCheck();
    bool commitToDevice();
    void makeSecondCopy();

    qb::crypto::KeyRing m_ring;   // the keys while the wizard runs (the device's ring gets them after the check)
    qb::crypto::keyfolder::Roles m_keys;
    QByteArray m_myPass, m_recPass;
    QString m_recoveryText;
    QString m_folder, m_copy;
    bool m_committed = false;
    bool m_checkOk = false;
    int m_askA = 0, m_askB = 1;
    QDate m_date;
};

// the page of File > Options
class EncryptionPage : public QWidget
{
    Q_OBJECT
public:
    explicit EncryptionPage(QWidget *parent = nullptr);
    // the switches of the open project, as the writer set them; false + sentences when one could not be applied
    bool apply(QStringList *errors = nullptr);
    void refresh();

private:
    void createKeys();
    void importKeys();
    void importKeyFile();
    void exportKeys();
    void forgetPassphrase();
    void say(const QString &title, const QString &text, bool warning = false);
    bool refusedHere(const QString &folder);

    QTreeWidget *m_list = nullptr;
    QLabel *m_empty = nullptr;
    QLabel *m_reason = nullptr;
    QCheckBox *m_backups = nullptr;
    QCheckBox *m_project = nullptr;
    QPushButton *m_create = nullptr;
    QPushButton *m_import = nullptr;
    QPushButton *m_importFile = nullptr;
    QPushButton *m_export = nullptr;
    QPushButton *m_forget = nullptr;
};

} // namespace qb
