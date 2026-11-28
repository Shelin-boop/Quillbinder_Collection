#pragma once
// Live Sync, backups and snapshots as jobs of the host (0.99.0), section 7 of the design.
//
// Only the device that hosts a shared project makes its backups. A client may ask for one (BACKUP_REQUEST) and sees the state
// (BACKUP_STATUS): running, done, waiting (made, but not yet in the backup folder), incomplete (files could not be read) or failed,
// and the time of the last success. A backup is made from a copy of the project that is whole at one commit: the host looks at
// the commit number, makes the copy (hard links where the file system has them, else real copies; files are only ever replaced by
// renaming, so a linked file never changes), checks that no commit came meanwhile, and packs the copy with the existing backup
// code (backup::backupProject with a frozen view). The copy is removed afterwards; Live Sync's own folders never go into it.
//
// Snapshots of a shared project are jobs of the host too (SNAPSHOT_OP: take, rename, delete, roll back). The host does them
// with its own Project and then publishes what changed on disk as one commit of its own (the files go to every copy as events),
// so the snapshot files replicate like any other. A roll back needs the asker's lock on the document and begins with a protective
// snapshot of the text as it is; a protective snapshot is also made before a takeover (HostSession's hook).
//
// How the service uses it, in short:
//   host:   auto *jobs = new HostBackup(config, parent);   host->setExtensionHandler(... jobs->handle(device, link, message) ...);
//           host->setSnapshotHook(jobs->protectiveSnapshotHook());   jobs->status() / statusChanged() for the status model;
//           jobs->requestBackup() for the host's own window.
//   client: auto *cj = new HostJobsClient(session, uuid, parent); session->setExtensionHandler([cj](m){ return cj->handle(m); });
//           snapshots::setRemoteJobs(folder, cj->remoteJobs()); cj->requestBackup(); cj->status() / statusChanged().
#include "core/Backup.h"
#include "core/Snapshots.h"
#include "core/collab/ClientSession.h"
#include "core/collab/HostSession.h"
#include "core/collab/StatusModel.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QString>
#include <functional>

namespace qb {
class Project;
namespace collab {

// A copy of a project folder that is whole at one commit.
struct FreezeResult {
    bool ok = false;
    QString folder;     // the copy (livesync/freeze-<seq>/ inside the project folder)
    qint64 seq = 0;     // the commit it holds
    int linked = 0;     // files made by hard link
    int copied = 0;     // files made by copy
    QString code;       // when not ok: "no-space", "busy", "failed"
    QString why;        // the reason (untranslated)
};

class HostBackup : public QObject
{
    Q_OBJECT
public:
    struct Config {
        HostSession *host = nullptr;  // not owned
        Project *project = nullptr;   // the host's own Project of the same folder; not owned
        // the host's Project as it is now (it is made again when the model could not follow a commit, so a pointer kept from the
        // start may be stale); used instead of `project` when set
        std::function<Project *()> projectOf;
        QString backupDir;            // "" = the program's default backup folder
        int keep = 5;                 // complete backups kept (backup::backupProject)
        // the writer's settings as they are when a backup is made (the service of a program gives them); used instead of the two above when set
        std::function<QString()> backupDirOf;
        std::function<int()> keepOf;
        qint64 minIntervalMs = 10000; // a device may ask for a backup this often (a host that is asked more often says HOST_BUSY)
        // What the host does around the copy. The copy itself is made in one step of the host's own thread, between two commits
        // (the host commits in that thread); a host that commits in another thread pauses its queue here.
        std::function<void()> pauseCommits, resumeCommits;
        // free bytes of the project's drive (default: QStorageInfo); the copy by real copying needs the size of the project and
        // a spare of 64 MiB
        std::function<qint64()> freeBytes;
        // the backup folder's state is not the host's to hide: when the backup code says "waiting", the status says so
    };
    explicit HostBackup(const Config &config, QObject *parent = nullptr);
    ~HostBackup() override;

    // ---- the host's own calls
    // Starts a backup at once (its steps run from the event loop, never inside the call). False with *why when one runs or the
    // project cannot be backed up now (nothing starts then).
    bool requestBackup(QString *why = nullptr);
    BackupStatus status() const { return m_status; }
    qint64 statusSeq() const { return m_statusSeq; }
    bool running() const { return m_status.state == BackupStatus::Running; }

    // Messages of the host jobs (BACKUP_REQUEST, SNAPSHOT_OP): the host session's extension handler; true = handled.
    bool handle(const QString &deviceId, net::Link *link, const QJsonObject &message);

    // A snapshot job of this device's own window (the host's own windows reach the host's code without a connection): the same
    // job as SNAPSHOT_OP, with the same checks, for the device `deviceId`. `body` holds op, doc, file, title, allowEmpty and opId.
    struct JobAnswer {
        bool ok = false;
        QString code, text, file;
    };
    JobAnswer runJobFor(const QString &deviceId, const QJsonObject &body);

    // before a takeover (and a draft taken over): the last committed text of "doc:<id>" kept as a snapshot of the document, then
    // published to the copies. HostSession::setSnapshotHook(jobs->protectiveSnapshotHook()).
    HostSession::SnapshotHook protectiveSnapshotHook();
    bool keepProtectiveSnapshot(const QString &resource, const QString &label, QString *error);

    // Publishes what the host's own code changed on disk (its snapshot jobs) as one commit: state, log and events. False with
    // *error when the commit could not be written (the changes are on disk, the copies catch up by manifest).
    // `afterRestart`: the host has read the folder at its start, so a cut-off job's files are in its list already; every snapshot file and the
    // project file are then published again (copies that have them take them as they are)
    bool publishChanges(const QString &opId, const QString &actorDevice, QString *error, qint64 *seqOut = nullptr, bool afterRestart = false);
    // after a restart: a job that was cut off between its files and its commit is published now
    bool recoverJobs(QString *error = nullptr);

    // ---- the copy
    // Makes the whole copy of `folder` at the commit `host->seq()`. `copyOnly` (and QB_TEST_LS_NO_LINKS=1) forces real copies.
    static FreezeResult freeze(HostSession *host, const QString &destinationParent, bool copyOnly, const std::function<qint64()> &freeBytes);
    // the folders of copies that a crash left behind are removed
    void removeOldFreezes();

signals:
    void statusChanged();
    // a request of a device was answered with an error code (HOST_BUSY, NO_ACCESS ...): for the host's log and the tests
    void requestRefused(const QString &deviceId, const QString &code);

private:
    struct JobResult {
        bool ok = false;
        QString code, text, file;
    };
    friend class HostBackupTestPeer;
    Project *project() const { return m_cfg.projectOf ? m_cfg.projectOf() : m_cfg.project; }
    void begin(const QString &requester);
    void pack(const FreezeResult &view);
    void finish(BackupStatus::State state, const QString &code, const QString &detail);
    void setState(BackupStatus::State state, const QString &code, const QString &detail);
    QJsonObject statusJson(const QString &re = QString()) const;
    void pushStatus();
    void saveState();
    void loadState();
    void onBackupRequest(const QString &deviceId, net::Link *link, const QJsonObject &m);
    void onSnapshotOp(const QString &deviceId, net::Link *link, const QJsonObject &m);
    JobResult runSnapshotJob(const QString &deviceId, const QJsonObject &m);
    void answer(net::Link *link, const QJsonObject &m);
    void refuse(const QString &deviceId, net::Link *link, const QString &code, const QString &re, const QString &text = QString());
    QString statePath() const;
    QString pendingPath() const;

    Config m_cfg;
    BackupStatus m_status;
    QString m_code;                // the reason code of the state ("" for done and running)
    QString m_rawDetail;           // the host's own words for the reason
    qint64 m_statusSeq = 0;        // the commit the last backup (or the running one) holds
    QString m_requester;
    QHash<QString, qint64> m_lastAsk; // device -> time of its last request (the host's clock)
    QList<QPointer<net::Link>> m_watchers; // connections that asked for the state: they are told when it changes
    QHash<QString, JobResult> m_doneJobs;
    QStringList m_doneOrder;
    bool m_inPump = false;
};

// ---------------------------------------------------------------- the client's side

class HostJobsClient : public QObject
{
    Q_OBJECT
public:
    HostJobsClient(ClientSession *session, const QString &projectUuid, QObject *parent = nullptr);
    ~HostJobsClient() override;

    // false when it was not for this class
    bool handle(const QJsonObject &message);
    // asks the host for a backup (it may refuse: the reason arrives with the state). False when not connected.
    bool requestBackup();
    // asks for the state without starting anything (done once when the session becomes ready)
    bool queryStatus();
    BackupStatus status() const { return m_status; }
    // the snapshot hooks for snapshots::setRemoteJobs(folder, ...). Each call waits for the host's answer, at most timeoutMs.
    snapshots::RemoteJobs *remoteJobs() { return &m_remote; }
    void setTimeoutMs(int ms) { m_timeoutMs = ms; }
    // the code of the last refused job (LOCK_HELD, LOCK_NEEDED, READ_ONLY ...)
    QString lastJobCode() const { return m_lastCode; }

signals:
    void statusChanged();
    // a job's answer: code is "" when it was done
    void jobDone(const QString &opId, bool ok, const QString &code, const QString &text);

private:
    class Remote : public snapshots::RemoteJobs
    {
    public:
        explicit Remote(HostJobsClient *owner) : m_o(owner) {}
        bool take(const QString &docId, const QString &title, QString *error) override;
        bool setTitle(const QString &docId, const QString &file, const QString &title, QString *error) override;
        bool remove(const QString &docId, const QString &file, QString *error) override;
        bool rollBack(const QString &docId, const QString &file, bool allowEmpty, QString *error) override;

    private:
        HostJobsClient *m_o;
    };
    bool job(const QJsonObject &body, QString *error);
    QPointer<ClientSession> m_session;
    QString m_uuid;
    BackupStatus m_status;
    Remote m_remote;
    int m_timeoutMs = 8000;
    QString m_lastCode;
    struct Waiting {
        bool done = false, ok = false;
        QString code, text, messageId;
        qint64 seq = 0;
    };
    QHash<QString, Waiting *> m_waiting;
};

// The words for the reason code of a backup's state, in the language of this device (the host sends the code and its own words;
// a code this version does not know shows the host's words).
QString backupReasonText(const QString &code, const QString &hostWords);

} // namespace collab
} // namespace qb
