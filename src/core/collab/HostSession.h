#pragma once
// Live Sync's host side of a project (0.99.0), sections 3, 5 and 6.1 of the design. One HostSession serves one hosted project
// to any number of connections (links): it answers the session messages, keeps the lock table and the edit requests, and
// turns change requests into commits, one after the other, each written in one SaveBatch together with the host's state
// (`livesync/state.json`) and its commit log (`livesync/log/<seq>.json`). A result goes out only after the batch was
// committed; then the commit goes to every joined connection as an event.
//
// The host applies a request to the project folder itself (operations to the project file through collab::applyToIndex,
// files as they are). What the host's program computes for its own saves (counts, modified times, the title list) is not
// recomputed here: the events carry the files the host really wrote, so every replica holds exactly them. A Project-based
// host (the service of package 5) plugs in through the hooks below.
#include "core/collab/Blobs.h"
#include "core/collab/Clock.h"
#include "core/collab/Locks.h"
#include "core/collab/Manifest.h"
#include "core/collab/net/Link.h"
#include "core/collab/proto/Messages.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <functional>
#include <memory>

namespace qb {
namespace collab {

enum class Role { None, Reader, Editor };

// The host's program computes data of its own when it saves a project: modified times, word and character counts of the
// documents, the list of documents with their titles. A host that has a Project (the service of a device, package 5)
// plugs it in here, so that those values are recomputed by the Project's own code when the host applies a commit, and
// every replica receives the result as it receives any file. Without a backend the host writes what the commit says and
// leaves derived data as it was.
class HostBackend {
public:
    virtual ~HostBackend() = default;
    // the paths (relative to the project folder) this backend may rewrite at a commit, besides the project file itself;
    // they are listed in the commit's log record and event with their hash as it is afterwards
    virtual QStringList derivedPaths() const = 0;
    // Called when a commit's batch is on disk (the files, the project file made by the operations, the state and log) and
    // before anyone is told. `putPaths` and `removedPaths` are the commit's files; `ops` its operations. The backend takes
    // the new state into its Project, computes what depends on it and rewrites the project file and its derived paths. A
    // false return (with a reason) leaves the commit as it is, valid but with stale derived data, and is logged.
    virtual bool afterCommit(const OpList &ops, const QStringList &putPaths, const QStringList &removedPaths, QString *error) = 0;
};
QString roleName(Role r);

constexpr int kKeepCommits = 10000;
constexpr qint64 kKeepDays = 30;

class HostBackup; // the host jobs (backups, snapshots) publish their own commits through the private parts

class HostSession : public QObject {
    Q_OBJECT
    friend class HostBackup;
public:
    struct Options {
        QString folder;   // the project folder
        QString uuid;     // the project's uuid
        QString hostName; // shown to the others
        Clock *clock = nullptr;
        qint64 epoch = 1; // used when the folder has no state yet
    };
    HostSession(const Options &options, QObject *parent = nullptr);
    ~HostSession() override;

    // makes the folder ready: finishes an interrupted save, reads the state and the log, makes the list of files. False with
    // a reason when the folder cannot be hosted (no project file, the state is damaged ...).
    bool start(QString *error = nullptr);
    bool isStarted() const { return m_started; }
    void stop(); // says BYE to every connection and closes it

    // a connection whose handshake is done (the link is Ready); the session does not own it
    void addLink(net::Link *link);
    int linkCount() const { return m_peers.size(); }
    int joinedCount() const;
    // the connections that joined: device, session, name and role (for the status model)
    struct PeerSummary {
        QString deviceId, session, name;
        Role role = Role::None;
    };
    QList<PeerSummary> joinedPeers() const;

    // ---- who may do what
    void setRole(const QString &deviceId, Role role) { m_roles.insert(deviceId, role); }
    void setRoleProvider(std::function<Role(const QString &)> provider) { m_roleProvider = std::move(provider); }
    Role roleOf(const QString &deviceId) const;
    void setMembership(const QJsonObject &membership) { m_membership = membership; }
    void setProofs(const QJsonArray &proofs) { m_proofs = proofs; }
    // while false no new lock is granted (HOST_BUSY): a planned handover drains
    void setAcceptingLocks(bool on) { m_acceptLocks = on; }
    // After a failover the table starts empty and the devices that were editing ask for their locks again (a request that names
    // the revision it was based on). For `ms` milliseconds a first request for another lock waits (HOST_BUSY), so that nobody
    // takes a document from a device that is still on its way back.
    void setLockGrace(qint64 ms);
    // After a planned handover: the old host's lock table moves along; the owners keep their locks (the same lease, this
    // epoch's generation). Returns how many were taken over.
    int adoptLocks(const QJsonArray &table);
    // a device shows another name from now on: its connections, its locks and the people that read the lock table
    void renameDevice(const QString &deviceId, const QString &name);
    void setHostName(const QString &name) { m_hostName = name; }
    // closes every connection of a device (a test build's cut network)
    void closeDevice(const QString &deviceId, const QString &code);
    // while false a change request is answered HOST_BUSY (a planned handover is preparing: no commits)
    void setAcceptingCommits(bool on) { m_acceptCommits = on; }
    // sends a message to the first joined connection of this device; false when there is none
    bool sendToDevice(const QString &deviceId, const QJsonObject &message);
    void setEpoch(qint64 epoch);

    // ---- hooks
    // Before a takeover: keeps the last committed state of the resource (a protective snapshot). The default keeps copies of
    // the document's text and notes files in `livesync/protective`. false with *error refuses the takeover.
    using SnapshotHook = std::function<bool(const QString &resource, const QString &label, QString *error)>;
    void setSnapshotHook(SnapshotHook hook) { m_snapshot = std::move(hook); }
    // Messages of the types of other packages (hosting, host jobs): called with the sending connection's device id; true =
    // handled. Without one, or when it returns false, the answer is BAD_OP.
    using Extension = std::function<bool(const QString &deviceId, net::Link *link, const QJsonObject &message)>;
    void setExtensionHandler(Extension handler) { m_extension = std::move(handler); }
    void setFreeSpaceProbe(std::function<qint64()> probe) { m_free = std::move(probe); }
    // see HostBackend; not owned
    void setBackend(HostBackend *backend) { m_backend = backend; }
    HostBackend *backend() const { return m_backend; }

    // ---- state
    QString folder() const { return m_folder; }
    QString uuid() const { return m_uuid; }
    qint64 epoch() const { return m_epoch; }
    qint64 seq() const { return m_seq; }
    qint64 revOf(const QString &resource) const { return m_revs.value(resource); }
    QHash<QString, qint64> revs() const { return m_revs; }
    LockTable &locks() { return m_locks; }
    EditRequests &editRequests() { return m_requests; }
    BlobStore &incoming() { return m_incoming; }
    // the files as the host holds them now (kept up to date by every commit)
    Manifest manifest() const;
    // the opId the host remembers (tests)
    bool knowsOp(const QString &opId) const { return m_ops.contains(opId); }
    int knownOpCount() const { return m_ops.size(); }
    qint64 opHorizon() const { return m_horizon; }
    // the log entry of a commit (empty object when pruned or missing)
    QJsonObject logEntry(qint64 seq) const;
    // one pass over the deadlines (also run on every tick of the clock)
    void tick();

signals:
    void committed(qint64 seq, const QString &opId, const QString &actorDevice);
    void peerJoined(const QString &deviceId, const QString &name);
    void peerLeft(const QString &deviceId);
    void locksChanged();
    // a message that was refused with this code (for the log and the tests)
    void refused(const QString &deviceId, const QString &code);

private:
    struct Peer;
    struct StoredResult {
        qint64 seq = 0;
        QJsonObject revs;
        qint64 wallMs = 0;
    };
    struct Reject {
        QString unit, code, holder;
        QString detail;
    };
    Peer *peerOf(net::Link *link) const;
    void onFrame(Peer *p, const net::Frame &f);
    void onMessage(Peer *p, const QJsonObject &m);
    void onClosed(Peer *p);
    void send(Peer *p, const QJsonObject &m);
    void sendError(Peer *p, const QString &code, const QString &re, const QString &text = QString());
    void broadcast(const QJsonObject &m, Peer *except = nullptr);

    void onHello(Peer *p, const QJsonObject &m);
    void onJoin(Peer *p, const QJsonObject &m);
    void onManifestGet(Peer *p, const QJsonObject &m);
    void onBlobGet(Peer *p, const QJsonObject &m);
    void onResync(Peer *p, const QJsonObject &m);
    void onCommit(Peer *p, const QJsonObject &m);
    void onLockRequest(Peer *p, const QJsonObject &m);
    void onLockRelease(Peer *p, const QJsonObject &m);
    void onEditRequest(Peer *p, const QJsonObject &m);
    void onEditAnswer(Peer *p, const QJsonObject &m);
    void onEditForce(Peer *p, const QJsonObject &m);
    void onEditCancel(Peer *p, const QJsonObject &m);

    void commitResult(Peer *p, const QJsonObject &request, const QString &status, const QString &code, const QJsonArray &rejected = QJsonArray(),
                      qint64 seq = 0, const QJsonObject &revs = QJsonObject(), bool duplicate = false);
    bool grantTo(Peer *p, const QStringList &resources, const QString &re, QString *deniedHolder = nullptr);
    void sendLockGranted(Peer *to, const Lock &l, const QString &re);
    void grantWaiting(const QString &resource);
    void askHolder(const QString &resource);
    void broadcastLocks();
    QList<Peer *> peersOfOwner(const Owner &o) const;
    Peer *peerByOwner(const Owner &o) const;
    QStringList descendantsOf(const QByteArray &index, const QString &id) const;

    bool saveStateFiles(QList<std::pair<QString, QByteArray>> *extra, QString *error);
    QByteArray stateJson(qint64 seq, const QHash<QString, qint64> &revs, qint64 genReserve) const;
    bool persistReserve(qint64 reserve);
    void rebuildFromLog();
    void pruneLog();
    qint64 freeBytes() const;
    bool defaultSnapshot(const QString &resource, const QString &label, QString *error);
    QString logPath(qint64 seq) const;
    QString indexPath() const { return m_indexPath; }
    void sendEvent(Peer *p, const QJsonObject &event);

    QString m_folder, m_uuid, m_hostName;
    Clock *m_clock;
    qint64 m_epoch;
    qint64 m_seq = 0;
    qint64 m_genReserve = 0;
    qint64 m_horizon = 0;
    bool m_started = false;
    bool m_acceptLocks = true;
    qint64 m_lockGraceUntil = 0;
    bool m_acceptCommits = true;
    bool m_broken = false;
    QString m_indexPath;
    QHash<QString, qint64> m_revs;
    QHash<QString, StoredResult> m_ops;
    QList<std::pair<qint64, QString>> m_opOrder; // (seq, opId), oldest first
    QList<std::pair<qint64, qint64>> m_logSeqs;   // (seq, wall ms), oldest first
    Manifest m_manifest;
    LockTable m_locks;
    EditRequests m_requests;
    BlobStore m_incoming;
    QList<Peer *> m_peers;
    QHash<QString, Role> m_roles;
    std::function<Role(const QString &)> m_roleProvider;
    QJsonObject m_membership;
    QJsonArray m_proofs;
    SnapshotHook m_snapshot;
    HostBackend *m_backend = nullptr;
    Extension m_extension;
    std::function<qint64()> m_free;
    int m_sincePrune = 0;
};

} // namespace collab
} // namespace qb
