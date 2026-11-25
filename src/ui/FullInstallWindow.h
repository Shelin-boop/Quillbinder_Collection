#pragma once
// The full AppImage's first start (0.94.0): its add-ons are unpacked into the data folder before the program starts.
// Before, that happened silently (the start script called the program once per package, and the first start took
// long without a sign of life); now one call installs the folder's packages and shows a window with the program's
// logo, a progress bar and what is going on. Without a display the same runs without the window.
//   quillbinder --install-folder <folder>
// The marker full-addons-installed-<version> keeps its rules (written when every package installed; nothing happens
// again for that version; an add-on removed afterwards stays removed).
#include <QString>
#include <QWidget>

class QLabel;
class QProgressBar;

namespace qb {

class FullInstallWindow : public QWidget
{
    Q_OBJECT
public:
    FullInstallWindow(int count, qint64 bytes, QWidget *parent = nullptr);
    // The message of the window ("Installing 13 add-ons (159 MB)...").
    static QString message(int count, qint64 bytes);
    void setProgress(int done, int total, const QString &name, qint64 doneBytes);
    int progressValue() const;
    int progressMaximum() const;
    QString currentText() const;

private:
    QLabel *m_logo = nullptr;
    QLabel *m_message = nullptr;
    QLabel *m_current = nullptr;
    QProgressBar *m_bar = nullptr;
};

// The command line's work: returns the exit code (0: everything installed or nothing to do).
int runInstallFolder(int &argc, char **argv, const QString &folder);

} // namespace qb
