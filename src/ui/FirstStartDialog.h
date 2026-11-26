#pragma once
// The questions of the first start (0.62): automatic backups (on or off, how many, which folder)
// and which settings come first (a project's own or the program's). Everything is filled in with
// the values in use, so OK alone keeps them; File > Options has the same settings. Shown once
// (key app/firstStartDone), also by an installation older than 0.62.
// Windows (W4b, the portable program): also the guide: where the program keeps everything, warnings for a folder in a
// sync service or with a long path, no standard backup folder, the place of notes and transcriptions, the Start menu
// entry, the desktop shortcut and the project file types (ui/PortableSetup.h). Help > Troubleshooting and Development > This Computer shows it again.
#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QRadioButton;
class QSpinBox;

namespace qb {
class PortableSetup;
}

namespace qb {

class FirstStartDialog : public QDialog
{
    Q_OBJECT
public:
    // again: opened from the Help menu (not the first start)
    explicit FirstStartDialog(QWidget *parent = nullptr, bool again = false);
    // Takes the choices over and remembers that the questions were answered.
    void take();
    // Whether the questions are still to be asked (QB_NO_DIALOGS and QB_SKIP_FIRST_START: never).
    static bool due();
    static void markDone();

    // 0.94.0: the one page also asks about the application menu (Linux, from an AppImage) and takes the place of the
    // note about the check for updates; the main window carries these two out after take()
    bool hasMenuQuestion() const { return m_menuAdd != nullptr; }
    bool addToMenu() const;
    bool moveAppImage() const;
    bool hasUpdateQuestion() const { return m_updates != nullptr; }
    bool checkForUpdates() const;

private:
    QComboBox *m_language = nullptr; // 0.95.0: the program's language (item 102)
    QCheckBox *m_backup;
    QSpinBox *m_keep;
    QLineEdit *m_folder;
    PortableSetup *m_setup = nullptr;
    QCheckBox *m_menuAdd = nullptr;
    QCheckBox *m_menuMove = nullptr;
    QCheckBox *m_updates = nullptr;
};

} // namespace qb
