#pragma once
// 0.95.0 (item 95): the window that manages profiles (use, save the settings as a new profile, rename, copy, delete,
// export, import) and the questions it asks. The work is done by ui/Profiles.*; the main window switches (MainWindow::switchProfile).
#include <QDialog>

class QLabel;
class QListWidget;
class QPushButton;

namespace qb {

class MainWindow;

class ProfileManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProfileManagerDialog(MainWindow *window);
    void select(const QString &name);
    QString selectedName() const;
    void reload(const QString &selectName = QString());

    // the buttons without their questions (also used by the tests)
    bool useSelected();
    QString saveNew(const QString &name, QString *error);
    bool importFile(const QString &file, QString *error);

private:
    void updateButtons();
    MainWindow *m_window;
    QListWidget *m_list;
    QLabel *m_info;
    QPushButton *m_use, *m_new, *m_rename, *m_copy, *m_delete, *m_export, *m_import;
};

namespace profileui {
// A name no other profile has; empty when cancelled.
QString askName(QWidget *parent, const QString &title, const QString &suggestion, const QString &except = QString());
// Import with the question about programs, folders and addresses in the file (as Load Options from File asks). Returns the
// new profile's name or an empty string; `error` is set for a file that is no profile.
QString importWithQuestion(QWidget *parent, const QString &file, QString *error);
// The same question at a switch to a profile that another computer wrote (a file copied or synced into the profiles
// folder): the keys that were refused (all of them, or none). Empty when the profile has nothing to ask about.
QStringList askPlacesAtSwitch(QWidget *parent, const QString &name);
// A switch to a profile that keeps fewer backups: false when the writer says no.
bool confirmLowerBackupKeep(QWidget *parent, const QString &name);
// Test hook: the answer to the question about places (true = take them); unset: ask.
void setPlacesAnswerForTests(int answer);   // -1 ask, 0 no, 1 yes
void setKeepAnswerForTests(int answer);     // the question about fewer backups: -1 ask, 0 no, 1 yes
} // namespace profileui

} // namespace qb
