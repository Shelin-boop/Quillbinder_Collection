#pragma once
// Live Sync's status model (0.99.0): what the user interface of Live Sync (options page, toolbar button, inspector tab,
// binder marks, bars, dialogs) shows, in one read-only place. The service of a program fills it from its sessions;
// the interface never looks into a session itself and never changes anything through this class. What a person can
// do from the interface (ask to edit, take over, request a backup, stop sharing) goes through StatusActions below.
//
// The interface is fixed: later packages add to it only by new functions, never by changing a name or a meaning. A
// model answers on the thread of the interface; a signal says "read again", it carries no data of its own.
//
//   projects()          every project this device shares or has joined (host or client), one ProjectStatus each
//   project(uuid)       one of them (valid == false when unknown)
//   projectOfFolder()   the same for a window's folder (the host's project folder or the replica folder)
//   devices()           the paired devices of this device (not per project)
//   lockInfo()          who holds one resource (for the selected document) as seen from this device
//
// Signals: projectsChanged() (a project came, went or changed its host), projectChanged(uuid) (state, participants,
// backup or drafts of one), devicesChanged(), locksChanged(uuid) (the lock table of one project).
//
// FakeStatusModel at the end fills everything by hand and fires the signals: for the interface's own suites.
#include <QCoreApplication>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {

// The five states of the toolbar button (design 8). The order is the order of the severity of what the writer must
// know; NotShared is also the answer for a project that is not shared at all.
enum class ShareState {
    NotShared,    // the project is not shared (or Live Sync is off)
    UpToDate,     // connected, nothing waits to be sent
    Sending,      // changes are on their way
    NotConnected, // the host cannot be reached: read-only, the changes made are kept as drafts
    Problem       // something needs the writer's attention; ProjectStatus::problemText says what
};

// The words of the five states, as the button and the inspector show them (translated at the place of use).
inline QString shareStateText(ShareState s)
{
    switch (s) {
    case ShareState::NotShared:
        return QCoreApplication::translate("LiveSync", "Not shared");
    case ShareState::UpToDate:
        return QCoreApplication::translate("LiveSync", "Up to date");
    case ShareState::Sending:
        return QCoreApplication::translate("LiveSync", "Sending changes...");
    case ShareState::NotConnected:
        return QCoreApplication::translate("LiveSync", "Not connected (draft kept)");
    case ShareState::Problem:
        return QCoreApplication::translate("LiveSync", "Problem");
    }
    return QString();
}

// This device's part in one project.
enum class ShareRole {
    Host,   // this device hosts it
    Editor, // a client that may change it
    Reader  // a client that may only read
};

// One device taking part in a project.
struct ParticipantStatus {
    QString deviceId;
    QString name;           // as the others see it
    bool online = false;
    bool thisDevice = false; // this device itself
    bool isHost = false;
    bool reader = false;     // the Reader role
    // what it edits now: resources of its locks ("doc:<id>", "structure", "settings", "object:<oid>", "mindmap:<id>",
    // "project"); the interface turns a "doc:" resource into the document's title
    QStringList editing;
    QDateTime lastSeen;      // invalid while online
};

// The state of the last (or the running) backup of a project, which only the host makes.
struct BackupStatus {
    enum State { None, Running, Done, Waiting, Incomplete, Failed };
    State state = None;
    QDateTime lastSuccess;   // invalid when there is none yet
    QString detail;          // a short reason for Waiting, Incomplete, Failed (translated; "" otherwise)
};

// Everything the interface shows about one shared project.
struct ProjectStatus {
    bool valid = false;
    QString uuid;
    QString title;
    QString folder;           // the host's project folder, or the replica's folder
    ShareRole role = ShareRole::Host;
    QString hostDeviceId;
    QString hostName;
    bool thisDeviceHosts() const { return role == ShareRole::Host; }
    qint64 epoch = 0;
    ShareState state = ShareState::NotShared;
    QString problemText;      // for Problem (a sentence for the writer); "" otherwise
    int pendingChanges = 0;   // requests kept and not yet confirmed (Sending, NotConnected)
    int draftCount = 0;       // recovery drafts of this project on this device
    bool readOnly = false;    // this window may not change the project now (Reader, not connected, not ready)
    QString readOnlyReason;   // part of a sentence ("" when it may be changed)
    QList<ParticipantStatus> participants; // the host first, then the others by name; this device included
    BackupStatus backup;
    QStringList notices;      // warnings of this project (an unknown device announced hosting ...), newest last; additive field
};

// A paired device (the options page's list, the join dialog, the access list of the hosted projects).
struct DeviceStatus {
    QString deviceId;
    QString name;
    QString fingerprint;     // short, for the details line
    bool online = false;
    QDateTime lastSeen;      // invalid when never seen after pairing
};

// Who holds a resource as this device sees it (the selected document's line of the inspector tab).
struct LockInfo {
    QString resource;
    bool held = false;
    bool mine = false;          // held by this device (a window of this device, this one or another)
    QString holderDeviceId;
    QString holderName;
    bool unreachable = false;   // the holder has not been heard of for a while
    QDateTime since;            // when the lock was granted, as far as known
    // an edit request of this device for the resource is open: the time left until taking over is possible (0 = may now)
    bool askOpen = false;
    qint64 askWaitMs = 0;
    bool declined = false;      // the holder declined the open request
};

class StatusModel : public QObject
{
    Q_OBJECT
public:
    explicit StatusModel(QObject *parent = nullptr) : QObject(parent) {}
    ~StatusModel() override = default;

    // false while Live Sync is off or its service is not running: the lists are empty then
    virtual bool serviceRunning() const = 0;
    virtual QList<ProjectStatus> projects() const = 0;
    virtual ProjectStatus project(const QString &uuid) const = 0;
    virtual ProjectStatus projectOfFolder(const QString &folder) const = 0;
    virtual QList<DeviceStatus> devices() const = 0;
    virtual LockInfo lockInfo(const QString &uuid, const QString &resource) const = 0;
    // the worst state of all projects (the toolbar button when no shared project is open): Problem over NotConnected
    // over Sending over UpToDate over NotShared
    ShareState overallState() const
    {
        ShareState worst = ShareState::NotShared;
        for (const ProjectStatus &p : projects())
            if (static_cast<int>(p.state) > static_cast<int>(worst))
                worst = p.state;
        return worst;
    }

signals:
    void projectsChanged();
    void projectChanged(const QString &uuid);
    void devicesChanged();
    void locksChanged(const QString &uuid);
};

// What a person may start from the interface. The service implements it; the call returns at once, the result shows
// in the model's signals (a state, a lock, a backup). A call for an unknown project does nothing.
class StatusActions
{
public:
    virtual ~StatusActions() = default;
    virtual void askToEdit(const QString &uuid, const QString &resource) = 0;
    virtual void cancelAsk(const QString &uuid, const QString &resource) = 0;
    virtual void takeOver(const QString &uuid, const QString &resource) = 0; // only after the waiting time
    virtual void stopEditing(const QString &uuid, const QString &resource) = 0; // gives the lock up after a flush
    virtual void requestBackup(const QString &uuid) = 0;
    virtual void stopSharing(const QString &uuid) = 0; // host: ends sharing; client: leaves
};

// A model filled by hand, with its signals, for the suites of the interface (and the first lines of the others).
class FakeStatusModel : public StatusModel, public StatusActions
{
    Q_OBJECT
public:
    explicit FakeStatusModel(QObject *parent = nullptr) : StatusModel(parent) {}

    bool serviceRunning() const override { return m_running; }
    QList<ProjectStatus> projects() const override { return m_projects; }
    ProjectStatus project(const QString &uuid) const override
    {
        for (const ProjectStatus &p : m_projects)
            if (p.uuid == uuid)
                return p;
        return ProjectStatus();
    }
    ProjectStatus projectOfFolder(const QString &folder) const override
    {
        for (const ProjectStatus &p : m_projects)
            if (p.folder == folder)
                return p;
        return ProjectStatus();
    }
    QList<DeviceStatus> devices() const override { return m_devices; }
    LockInfo lockInfo(const QString &uuid, const QString &resource) const override
    {
        for (const LockInfo &l : m_locks.value(uuid))
            if (l.resource == resource)
                return l;
        LockInfo none;
        none.resource = resource;
        return none;
    }

    // ---- filling it (each fires the signal a real model fires)
    void setRunning(bool on)
    {
        m_running = on;
        emit projectsChanged();
    }
    void setProject(const ProjectStatus &p)
    {
        bool known = false;
        for (ProjectStatus &q : m_projects)
            if (q.uuid == p.uuid) {
                q = p;
                known = true;
            }
        if (!known) {
            m_projects.append(p);
            m_projects.last().valid = true;
            emit projectsChanged();
        }
        emit projectChanged(p.uuid);
    }
    void removeProject(const QString &uuid)
    {
        for (int i = 0; i < m_projects.size(); ++i)
            if (m_projects.at(i).uuid == uuid) {
                m_projects.removeAt(i);
                break;
            }
        m_locks.remove(uuid);
        emit projectsChanged();
    }
    void setState(const QString &uuid, ShareState s, const QString &problem = QString())
    {
        for (ProjectStatus &p : m_projects)
            if (p.uuid == uuid) {
                p.state = s;
                p.problemText = problem;
            }
        emit projectChanged(uuid);
    }
    void setDevices(const QList<DeviceStatus> &d)
    {
        m_devices = d;
        emit devicesChanged();
    }
    void setLock(const QString &uuid, const LockInfo &l)
    {
        QList<LockInfo> &list = m_locks[uuid];
        for (LockInfo &x : list)
            if (x.resource == l.resource) {
                x = l;
                emit locksChanged(uuid);
                return;
            }
        list.append(l);
        emit locksChanged(uuid);
    }
    void clearLocks(const QString &uuid)
    {
        m_locks.remove(uuid);
        emit locksChanged(uuid);
    }

    // ---- StatusActions: the calls are only noted, for the suites to read
    void askToEdit(const QString &uuid, const QString &resource) override { m_calls << QStringLiteral("ask:%1:%2").arg(uuid, resource); }
    void cancelAsk(const QString &uuid, const QString &resource) override { m_calls << QStringLiteral("cancel:%1:%2").arg(uuid, resource); }
    void takeOver(const QString &uuid, const QString &resource) override { m_calls << QStringLiteral("take:%1:%2").arg(uuid, resource); }
    void stopEditing(const QString &uuid, const QString &resource) override { m_calls << QStringLiteral("stop:%1:%2").arg(uuid, resource); }
    void requestBackup(const QString &uuid) override { m_calls << QStringLiteral("backup:%1").arg(uuid); }
    void stopSharing(const QString &uuid) override { m_calls << QStringLiteral("stopsharing:%1").arg(uuid); }
    QStringList calls() const { return m_calls; }

private:
    bool m_running = true;
    QList<ProjectStatus> m_projects;
    QList<DeviceStatus> m_devices;
    QHash<QString, QList<LockInfo>> m_locks;
    QStringList m_calls;
};

} // namespace collab
} // namespace qb
