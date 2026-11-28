#pragma once
// The dialogs of Live Sync (0.99.0, work package 7): pairing, joining a project, the hosted projects and the recovery
// drafts. All four are windows that do not block the program (non-modal): the other device's answer arrives while the
// writer goes on working. Each one is opened through the function at the end, which keeps one window of its kind.
#include "ui/LiveSyncControl.h"

#include <QDialog>
#include <QPointer>

#include <functional>

class QLabel;
class QListWidget;
class QProgressBar;
class QPushButton;
class QTreeWidget;
class QComboBox;
class QTimer;

namespace qb {
namespace livesync {

// What asks the writer for something small. The suites set these; the program leaves them empty (the real boxes).
struct DialogHooks {
    std::function<bool(QWidget *parent, const QString &title, const QString &text, const QString &action)> confirm;
    std::function<QString(QWidget *parent, const QString &title, const QString &label)> inputText;       // "" = canceled
    std::function<QString(QWidget *parent, const QString &title, const QString &label, const QStringList &items)> pickItem; // "" = canceled
};
DialogHooks &dialogHooks();
bool confirmAction(QWidget *parent, const QString &title, const QString &text, const QString &action);
QString inputText(QWidget *parent, const QString &title, const QString &label);
QString pickItem(QWidget *parent, const QString &title, const QString &label, const QStringList &items);

// the backup state in one sentence ("Backup done. Last successful backup: ...")
QString backupText(const collab::BackupStatus &b);
// Stop Sharing (host) or Leave (client): asks first, naming the devices online and the unconfirmed changes. True when done.
bool stopSharingFlow(QWidget *parent, const QString &uuid);
// Transfer Hosting...: asks which online editor device gets it (desktops that may host); true when a transfer was started
bool transferHostingFlow(QWidget *parent, const QString &uuid);

// "123 456" from six digits
QString groupedCode(const QString &digits);

// Pairing, on both sides: the host opens its window, the other device picks it (or connects by address); both then show
// the other's name and the code in large digits and ask Codes Match / They Differ.
class PairingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PairingDialog(QWidget *parent = nullptr);
    ~PairingDialog() override;
    // a code arrived (also when the other device started the pairing): show it
    void showCode(const QString &otherName, const QString &code);

private:
    void refreshList();
    void refreshState();
    void finished(bool ok, const QString &text);
    QLabel *m_state = nullptr;
    QListWidget *m_list = nullptr;
    QPushButton *m_openWindow = nullptr;
    QPushButton *m_pair = nullptr;
    QPushButton *m_address = nullptr;
    QLabel *m_other = nullptr;
    QLabel *m_code = nullptr;
    QPushButton *m_match = nullptr;
    QPushButton *m_differ = nullptr;
    QWidget *m_codeBox = nullptr;
    QTimer *m_timer = nullptr;
    bool m_codeShown = false;
};

// Join a Project: the paired hosts' shared projects; unpaired hosts only as "Pair first".
class JoinDialog : public QDialog
{
    Q_OBJECT
public:
    explicit JoinDialog(QWidget *parent = nullptr);
signals:
    // the copy is whole: open this folder (the window that opened the dialog does)
    void openProject(const QString &folder);

private:
    void refresh();
    QTreeWidget *m_tree = nullptr;
    QPushButton *m_join = nullptr;
    QPushButton *m_pairFirst = nullptr;
    QProgressBar *m_progress = nullptr;
    QLabel *m_note = nullptr;
};

// Hosted Projects: participants, access per device (Editor, Reader, None), priority, backup state, Stop Sharing,
// Transfer Hosting.
class HostedProjectsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HostedProjectsDialog(QWidget *parent = nullptr);
    void selectProject(const QString &uuid);

private:
    void refreshProjects();
    void refreshAccess();
    QString currentUuid() const;
    QComboBox *m_projects = nullptr;
    QTreeWidget *m_devices = nullptr;
    QLabel *m_backup = nullptr;
    QPushButton *m_up = nullptr;
    QPushButton *m_down = nullptr;
    QPushButton *m_stop = nullptr;
    QPushButton *m_transfer = nullptr;
};

// Recovery Drafts: changes kept on this device; never deleted by themselves.
class DraftsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DraftsDialog(QWidget *parent = nullptr);
    // only the drafts of this project ("" = all)
    void setProject(const QString &uuid);

private:
    void refresh();
    QString currentId() const;
    QString m_uuid;
    QTreeWidget *m_tree = nullptr;
    QPushButton *m_compare = nullptr;
    QPushButton *m_keep = nullptr;
    QPushButton *m_take = nullptr;
    QPushButton *m_delete = nullptr;
    QLabel *m_empty = nullptr;
};

// open (or raise) the window of its kind; each is shown without blocking
PairingDialog *showPairingDialog(QWidget *parent);
JoinDialog *showJoinDialog(QWidget *parent);
HostedProjectsDialog *showHostedProjectsDialog(QWidget *parent, const QString &uuid = QString());
DraftsDialog *showDraftsDialog(QWidget *parent, const QString &uuid = QString());

} // namespace livesync
} // namespace qb
