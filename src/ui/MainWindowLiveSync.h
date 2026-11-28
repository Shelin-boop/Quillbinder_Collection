#pragma once
// Live Sync in a window (0.99.0): what MainWindow does when its project is shared (section 4 of the design). The window opens
// the folder as a follower of its program's service: the service holds the sessions and writes the shared files; the window
// captures what its saves would write, hands it to the service, applies what the service says arrived (a remote commit into
// the open documents without overwriting unsaved text), takes the document's lock at the first key and gives it up when the
// document is no longer shown.
#include "core/Project.h"
#include "core/collab/ServiceClient.h"

#include <QObject>
#include <QSet>
#include <QTimer>

namespace qb {
namespace livesync {
class LiveSyncControl;
}
class MainWindow;
class Project;
namespace collab {
class Service;
}

namespace ui {
// Where a window finds its service. A program with Live Sync on a desktop reaches the service process through its local
// socket (started when needed); Android and the tests give the service they run in their own process.
void setInProcessLiveSyncService(collab::Service *service);
collab::Service *inProcessLiveSyncService();
// the program's own control of Live Sync (made at the first call; a service of its own that starts when Live Sync is on)
livesync::LiveSyncControl *programLiveSyncControl();
// Live Sync's folder as this program uses it (the in-process service's, else the program's own)
QString liveSyncDir();
// "Keep hosting when the last window is closed" (File > Options > Live Sync): the windows tell it to the service when they attach
void setLiveSyncKeepHosting(bool on);
bool liveSyncKeepHosting();
} // namespace ui

class LiveSyncGlue : public QObject {
    Q_OBJECT
public:
    // `client` is attached (the project's folder is taken by the service); it becomes this object's child
    LiveSyncGlue(MainWindow *window, collab::ServiceClient *client);
    ~LiveSyncGlue() override;

    collab::ServiceClient *client() const { return m_client; }
    // after the window has adopted the project: the capture, the events, the lock gate
    void begin(Project *project);
    // before the project goes: pending changes sent, locks given up, the gate removed
    void end();

    // for the status of a window: the reasons a document may not be changed here
    QString whyReadOnly() const;

private:
    void onEvent(qint64 seq, const QString &opId, const QString &actor, bool own, const QJsonArray &ops, const QStringList &paths);
    void onState();
    void onLockResult(const QString &resource, bool granted, const QString &text);
    void releaseUnused();
    QString titleOf(const QString &id) const;

    MainWindow *m_window;
    collab::ServiceClient *m_client;
    QPointer<Project> m_project;
    QTimer m_release;
    QSet<QString> m_wanted; // documents whose lock was asked for by the editor
    bool m_ended = false;
    int m_lastState = -1;
};

} // namespace qb
