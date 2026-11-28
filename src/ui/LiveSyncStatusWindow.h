#pragma once
// Live Sync's status window (0.99.0, section 6.2 of the design) and the entry of the service process. A service that keeps
// hosting after the last window was closed (File > Options > Live Sync, "Keep hosting when the last window is closed") shows
// this small window: how many projects it hosts for other devices, Open Project (starts a window of the program on the project)
// and Stop Hosting and Quit. A tray icon is added only where the system has a tray. It shows only what the status model says.
#include "core/collab/StatusModel.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QSystemTrayIcon;

namespace qb {
namespace ui {

class LiveSyncStatusWindow : public QWidget {
    Q_OBJECT
public:
    explicit LiveSyncStatusWindow(collab::StatusModel *model, QWidget *parent = nullptr);
    // the line the window shows ("Hosting 2 projects for other devices")
    QString summary() const;
    int hostedCount() const;
    // the folders of the hosted projects, in the list's order
    QStringList hostedFolders() const;

signals:
    void openProjectRequested(const QString &folder);
    void stopAndQuitRequested();

public slots:
    void refresh();

private:
    collab::StatusModel *m_model;
    QLabel *m_label = nullptr;
    QListWidget *m_list = nullptr;
    QSystemTrayIcon *m_tray = nullptr;
};

// `quillbinder --live-sync-service`: the service of this device without a window of the program. Returns the exit code. Called
// by main() before anything else of the program starts.
int runLiveSyncService(int argc, char **argv);

} // namespace ui
} // namespace qb
