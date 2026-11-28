#pragma once
// Live Sync's client side of a project (0.99.0): the replica of a project hosted on another device. One ClientSession serves
// one joined project of this device. It keeps the replica folder equal to the host's (by events, by replay of the host's log
// or by comparing manifests), keeps the outbox of change requests and sends them one after the other, holds the locks of
// this device's windows and tells the windows what happened (signals). It is the only writer of the replica folder; the
// windows that show it only ever capture their changes (collab/Gate.h) and hand them in through submit().
//
// A session never writes the shared files of a project it does not host except as the host's files arrive: the bytes of the
// host's events are placed unchanged (an encrypted project's replica holds the host's sealed files).
//   <replica>/livesync-replica.json   {p, epoch, seq, ready, revs}: ready is true only when the full copy is whole
//   <replica>/livesync-local/outbox, drafts, blobs, hashes.json
#include "core/collab/Blobs.h"
#include "core/collab/Clock.h"
#include "core/collab/Locks.h"
#include "core/collab/Manifest.h"
#include "core/collab/Outbox.h"
#include "core/collab/net/Link.h"
#include "core/collab/proto/Messages.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <memory>

namespace qb {
namespace collab {

class ClientSession : public QObject {
    Q_OBJECT
public:
    enum class State { Idle, Connecting, Joining, Syncing, Ready, Offline, Failed };
    Q_ENUM(State)
    struct Config {
        QString folder;     // the replica folder
        QString uuid;       // the project's uuid
        QString deviceId;   // this device's id (what the host sees)
        QString deviceName; // shown to the others
        QString session = QStringLiteral("main");
        Clock *clock = nullptr;
        // The folder is the host's own project folder (a window of the hosting device): the files are already there and
        // are never written or fetched; only the bookkeeping (sequence, revisions, locks, the outbox) is kept, in
        // livesync-local/loopback.json instead of livesync-replica.json.
        bool loopback = false;
    };
    explicit ClientSession(const Config &config, QObject *parent = nullptr);
    ~ClientSession() override;

    // reads the replica's state and the outbox (false with a reason when the folder cannot be used)
    bool open(QString *error = nullptr);
    // a link that is Ready: says HELLO, joins, brings the replica up to date; the outbox follows
    void attach(net::Link *link);
    // the link is gone (also called when it closes): the session is Offline, the locks are kept as far as known
    void detach();
    bool isAttached() const { return !m_link.isNull(); }

    State state() const { return m_state; }
    QString failure() const { return m_failure; }
    bool replicaReady() const { return m_ready; }
    qint64 seq() const { return m_seq; }
    qint64 epoch() const { return m_epoch; }
    QString role() const { return m_role; }
    qint64 revOf(const QString &resource) const { return m_revs.value(resource); }
    Outbox &outbox() { return *m_outbox; }
    const Outbox &outbox() const { return *m_outbox; }
    QString folder() const { return m_cfg.folder; }

    // ---- changes
    // Keeps the request in the outbox (units and the plain bytes the units name by hash) and sends it as soon as the session is
    // ready. Revisions the units leave out are filled in with the ones this session knows. Returns the operation id; empty with
    // *error when it could not be kept (nothing is sent then).
    QString submit(QList<proto::Unit> units, const QHash<QString, QByteArray> &blobsByHash, QString *error = nullptr);
    bool hasPending() const { return !m_outbox->pending().isEmpty() || !m_inFlight.isEmpty(); }

    // ---- locks (this device's session holds them)
    void lock(const QStringList &resources);
    // given up as soon as nothing of the outbox refers to them any more (a confirmed flush)
    void unlock(const QStringList &resources);
    bool holds(const QString &resource) const { return m_held.contains(resource); }
    proto::Token token(const QString &resource) const { return m_held.value(resource); }
    // who holds the resource now (""): a device of another session
    QString heldElsewhere(const QString &resource) const { return m_others.value(resource).name; }
    bool isUnreachable(const QString &resource) const { return m_others.value(resource).unreachable; }
    // the locks of other devices as this session knows them: resource -> the holder's name; and the resources of this session's own
    QHash<QString, QString> othersLocks() const
    {
        QHash<QString, QString> h;
        for (auto it = m_others.constBegin(); it != m_others.constEnd(); ++it)
            h.insert(it.key(), it->name);
        return h;
    }
    QHash<QString, QString> othersDevices() const
    {
        QHash<QString, QString> h;
        for (auto it = m_others.constBegin(); it != m_others.constEnd(); ++it)
            h.insert(it.key(), it->device);
        return h;
    }
    QStringList unreachableLocks() const
    {
        QStringList l;
        for (auto it = m_others.constBegin(); it != m_others.constEnd(); ++it)
            if (it->unreachable)
                l << it.key();
        return l;
    }
    QStringList heldLocks() const { return m_held.keys(); }
    // the sequence of the host's newest commit as far as known
    qint64 headSeq() const { return m_head; }
    QHash<QString, qint64> revs() const { return m_revs; }
    // every change request not yet confirmed becomes a recovery draft (the old host took commits after the split); the number
    int pendingToDrafts(const QString &reason);
    bool isLoopback() const { return m_cfg.loopback; }
    // edit requests
    void askToEdit(const QString &resource);
    void cancelAsk(const QString &resource);
    void takeOver(const QString &resource);
    // the holder's answer to a request: Save and Hand Over (true; the outbox is flushed first) or Decline
    void answerEdit(const QString &resource, bool handOver, const QString &askerDevice = QString());
    // milliseconds of waiting left for a request of this session (0 = may take over)
    qint64 editWaitMs(const QString &resource) const;

    // Messages of types this class does not handle (hosting, host jobs): handed to this function; true = handled
    using Extension = std::function<bool(const QJsonObject &)>;
    void setExtensionHandler(Extension e) { m_extension = std::move(e); }
    // sends a message of an extension (false when not attached)
    bool sendMessage(const QJsonObject &m);
    void setFreeSpaceProbe(std::function<qint64()> probe) { m_free = std::move(probe); }
    // makes the replica again from the host's manifest (repairs files that differ; also what a gap in the log leads to)
    void resync() { startFullSync(); }
    // the number of full copies made (tests: a resync by manifest happened)
    int fullSyncCount() const { return m_fullSyncs; }

signals:
    void stateChanged(qb::collab::ClientSession::State state);
    // the replica follows the host as far as the host's events go (fires when a session becomes Ready)
    void readyToUse();
    // an event of the host is in the replica: `own` for this device's own commit (the window needs only the derived data then)
    void eventApplied(qint64 seq, const QString &opId, const QString &actor, bool own, const QJsonArray &ops, const QStringList &paths, const QJsonObject &revs);
    // the replica was made again from the host's manifest (the windows reload everything)
    void replicaReplaced();
    void committed(const QString &opId, qint64 seq);
    // the request was refused (no part of it was applied); it moved to the drafts
    void rejected(const QString &opId, const QJsonArray &rejects);
    void draftKept(const QString &opId, const QString &reason);
    void problem(const QString &opId, const QString &code);
    void lockGranted(const QString &resource);
    void lockDenied(const QString &resource, const QString &holder, const QString &reason);
    void lockRevoked(const QString &resource, const QString &reason);
    void locksChanged();
    // the host says that a request for a document this session holds is over (the asker gave up or took over)
    void editEnded(const QString &resource, const QString &askerDevice);
    void editAsked(const QString &resource, const QString &askerName, qint64 waitMs);
    void editStatus(const QString &resource, bool declined, qint64 waitMs, bool mayForce, const QString &holder);
    void epochChanged(qint64 from, qint64 to);
    // the host's answer to the join: its signed membership and the chain of proofs of its authority (package 6 reads them)
    void joinedInfo(const QJsonObject &membership, const QJsonArray &proofs);
    void failedWith(const QString &code, const QString &why);

private:
    struct Other {
        QString name;
        QString device;
        QString session;
        bool unreachable = false;
    };
    void setState(State s);
    void fail(const QString &code, const QString &why);
    void send(const QJsonObject &m);
    void onFrame(const net::Frame &f);
    void onMessage(const QJsonObject &m);
    void onWelcome(const QJsonObject &m);
    void onJoined(const QJsonObject &m);
    void onManifest(const QJsonObject &m);
    void onEvent(const QJsonObject &m);
    void onResult(const QJsonObject &m);
    void onLockTable(const QJsonObject &m);
    void onError(const QJsonObject &m);
    void onBlobReceived(const QString &hash);
    void onBlobFailed(const QString &hash, const QString &code);
    void tick();

    void startFullSync();
    void pumpDownloads();
    void placeFullSync();
    void processEvents();
    void applyEvent(QJsonObject event); // (by value: the buffered copy is removed while it runs)
    bool placeFiles(const QJsonArray &files, QString *error);
    void checkSyncDone();
    void becomeReady();
    void pumpOutbox();
    void sendRequest(const OutboxEntry &e);
    QJsonArray rebased(const QJsonArray &units);
    QStringList resourcesOfUnits(const QJsonArray &units) const;
    void toDraft(const QString &opId, const QString &reason);
    bool saveReplicaState(bool ready, QString *error = nullptr);
    QString statePath() const;
    void dropHeldFor(const QString &resource, const QString &reason);
    void flushReleases();
    QString sendLockRequest(const QStringList &resources);

    Config m_cfg;
    std::unique_ptr<Outbox> m_outbox;
    QPointer<net::Link> m_link; // (a link that is deleted under the session reads as gone)
    QPointer<BlobHub> m_hub;
    QList<QMetaObject::Connection> m_conns;
    State m_state = State::Idle;
    QString m_failure;
    bool m_ready = false;
    qint64 m_epoch = 0, m_seq = 0, m_head = 0;
    QString m_role;
    QHash<QString, qint64> m_revs;
    QSet<QString> m_ownRevs; // "resource@rev" produced by this session's own commits
    QMap<qint64, QJsonObject> m_events;
    bool m_applying = false;
    bool m_resyncAsked = false;
    // the full copy
    bool m_full = false;
    int m_fullSyncs = 0;
    int m_fullTries = 0;
    QMap<int, Manifest> m_pages;
    int m_pagesExpected = 0;
    QString m_manifestRe;
    Manifest m_target;
    ManifestDiff m_plan;
    QStringList m_toFetch;   // hashes still to get
    QSet<QString> m_inFlightBlobs;
    // the outbox
    QString m_inFlight;       // the operation id sent and not answered
    bool m_requestSent = false; // its request went out (the contents first)
    QHash<QString, QStringList> m_lockReqs; // message id -> resources asked
    QList<std::pair<QString, QString>> m_answers; // hand-overs waiting for the outbox to empty
    QHash<QString, int> m_blobRetries;
    qint64 m_retryAt = 0;
    qint64 m_lastHb = 0;
    // locks
    QHash<QString, proto::Token> m_held;
    QHash<QString, Other> m_others;
    QHash<QString, qint64> m_waitUntil; // resource -> local time when taking over becomes possible
    QStringList m_releasing;
    QSet<QString> m_forceOffered;
    Extension m_extension;
    std::function<qint64()> m_free;
};

} // namespace collab
} // namespace qb
