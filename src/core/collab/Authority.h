#pragma once
// Who hosts a shared project, and how that changes (0.99.0), sections 6.3 to 6.6 of the design. One Authority object per project
// and device. It keeps the device's view in `livesync/authority.json` (written before any message that depends on it is sent),
// the signed membership, and the chain of proofs that says why the current host may host:
//   founding   the device that shared the project
//   handover   the old host's signed hand-over to a named target (planned: DRAINING -> PREPARED -> RETIRED, target FOLLOWER ->
//              PREPARED -> ACTIVE)
//   failover   a member's own proof after the host was lost, with the signed votes of a majority (three or more devices)
// A message that announces hosting counts only when its chain verifies against the signed membership; anything else comes from an
// unknown device and is refused with a warning. The service of package 5 drives an Authority through the small interface at the
// end of this file (HostingBackend: what the device can do, Authority's public functions: what happened).
#include "core/collab/Clock.h"
#include "core/collab/Drafts.h"
#include "core/collab/Membership.h"
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
namespace net {
class DeviceIdentity;
}

// ---- times (developer values, not options; the tests move a ManualClock over them)
constexpr qint64 kFailoverMs = 30000;       // the host is lost after this long without contact
constexpr qint64 kRankStaggerMs = 5000;     // rank k starts 30 s + 5 s x (k - 1) after the last contact
constexpr qint64 kElectionRoundMs = 10000;  // a candidate waits this long for votes
constexpr qint64 kElectionRetryMs = 5000;   // and this long after a failed round
constexpr qint64 kDrainMs = 10000;          // a planned handover lets the clients flush for at most this long
constexpr qint64 kHandoverWaitMs = 15000;   // the old host waits this long for READY
constexpr qint64 kTargetWaitMs = 10000;     // the target waits this long for its replica to reach the handover state
constexpr qint64 kPendingMs = 30000;        // a PREPARED target without COMMIT goes back to FOLLOWER
constexpr qint64 kResendMs = 3000;          // the old host resends a proof that was not answered
constexpr qint64 kHandbackMs = 60000;       // the highest-priority device must be online and in sync this long

enum class AuthState { Follower, Active, Draining, Prepared, Retired };
QString authStateName(AuthState s);

// ---------------------------------------------------------------- proofs

struct Vote {
    QString voter;
    QByteArray signature;
};

struct Proof {
    enum Kind { Founding, Handover, Failover };
    Kind kind = Founding;
    QString project;
    qint64 epoch = 0;     // the epoch this proof opens
    qint64 previous = 0;  // the epoch it follows (0 for the founding)
    QString host;         // the device that hosts from `epoch` on
    QString signer;       // founding: the host; handover: the old host; failover: the new host
    qint64 baseSeq = 0;   // the state the new host starts from
    QString baseHash;     // its state hash
    QString locksDigest;  // handover: SHA-256 of the lock table that moves along
    QList<Vote> votes;    // failover with three or more devices
    QByteArray signature;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &o, Proof *out, QString *why = nullptr);
    QByteArray signingBytes() const;
    bool operator==(const Proof &o) const { return toJson() == o.toJson(); }
};

// what a voter signs: the project, the epoch asked for, the candidate and the state it offers
QByteArray voteBytes(const QString &project, qint64 epoch, const QString &candidate, qint64 baseSeq, const QString &baseHash, const QString &voter);
// "more than half of all the devices of the project, the candidate counted"; 0 for two devices (the other takes over alone)
int votesNeeded(int deviceCount);

Proof makeFounding(const QString &project, qint64 epoch, const net::DeviceIdentity &host);
Proof makeHandover(const QString &project, qint64 previous, qint64 epoch, const QString &target, qint64 seq, const QString &hash,
                   const QString &locksDigest, const net::DeviceIdentity &oldHost);
Proof makeFailover(const QString &project, qint64 previous, qint64 epoch, qint64 baseSeq, const QString &baseHash, const QList<Vote> &votes,
                   const net::DeviceIdentity &newHost);
QString locksDigestOf(const QJsonArray &locks);

struct ChainResult {
    bool ok = false;
    QString why;
    qint64 epoch = 0;
    QString host;
    qint64 baseSeq = 0;   // where the last step forks from the one before
    QString baseHash;
};
// Checks a chain from its founding against the membership. Steps that are identical to one of `trusted` (checked earlier) are
// taken as they are; a chain that starts with another founding than the trusted one fails.
ChainResult verifyChain(const QList<Proof> &chain, const Membership &members, const QList<Proof> &trusted = {});

// ---------------------------------------------------------------- the persisted view

struct AuthorityRecord {
    QString project;
    qint64 epoch = 0;
    QString host;
    AuthState state = AuthState::Follower;
    qint64 highest = 0;           // the highest epoch this device has seen, promised or voted for (never lowered)
    QList<Proof> chain;
    // old host, while handing over
    QString hoTarget;
    qint64 hoEpoch = 0, hoSeq = 0;
    QString hoHash;
    bool hoHasProof = false;
    Proof hoProof;                 // valid in RETIRED
    QJsonArray hoLocks;            // the lock table that moves along (kept so that a restarted old host can resend the same proof)
    // target, while prepared
    qint64 pendEpoch = 0, pendSeq = 0;
    QString pendFrom, pendHash;
    QHash<qint64, QString> votes;  // epoch -> candidate this device voted for

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &o, AuthorityRecord *out, QString *why = nullptr);
};

// ---------------------------------------------------------------- what the service gives

// What the device can do for a project. Implemented by the service (package 5); the tests have a fake. Functions that start or
// stop hosting return after the change is made.
class HostingBackend {
public:
    virtual ~HostingBackend() = default;
    virtual qint64 seq() const = 0;         // the commits this device holds (host: its head; client: its replica's)
    virtual QString stateHash() const = 0;  // the state hash of those files
    virtual bool replicaReady() const = 0;  // the copy is complete and may be hosted from
    virtual bool canHost(QString *why) const { Q_UNUSED(why); return true; } // writable folder, free space ...
    // this device starts hosting `epoch` (a new host session; the lock table starts from `locks`, which is empty after a failover)
    virtual bool startHosting(qint64 epoch, const QJsonArray &locks, QString *error) = 0;
    // this device stops hosting at once (BYE to its clients); it is a client from now on
    virtual void stopHosting() = 0;
    // the clients connect to this device from now on (also at start-up and after every change of host)
    virtual void followHost(const QString &hostDeviceId, qint64 epoch) = 0;
    // after a host change: make the copy again from the new host's manifest
    virtual void resyncFromHost() {}
    // planned handover, old host
    virtual void setAcceptingLocks(bool on) { Q_UNUSED(on); }
    virtual void setAcceptingCommits(bool on) { Q_UNUSED(on); }
    virtual bool drained() const { return true; }       // the clients flushed what they had
    virtual QJsonArray lockTable() const { return QJsonArray(); }
    virtual bool busy() const { return false; }         // a handover or a backup is running (no handback now)
    // reunion
    virtual bool backupBeforeRejoin(QString *error) { Q_UNUSED(error); return true; } // a ZIP backup of what the old host holds
    // the commits this device authored after `afterSeq` in an epoch before `beforeEpoch` (its log or its ledger)
    virtual QList<AuthoredCommit> authoredCommits(qint64 afterSeq, qint64 beforeEpoch) { Q_UNUSED(afterSeq); Q_UNUSED(beforeEpoch); return {}; }
    // the unconfirmed outbox entries become drafts; returns how many
    virtual int outboxToDrafts(const QString &reason) { Q_UNUSED(reason); return 0; }
    // the commits above were turned into drafts: they must not be turned again
    virtual void reunionDone(qint64 afterSeq, qint64 newEpoch) { Q_UNUSED(afterSeq); Q_UNUSED(newEpoch); }
};

class Election;
class Handover;

class Authority : public QObject {
    Q_OBJECT
public:
    struct Config {
        QString project;          // the project's uuid
        QString title;            // shown in warnings
        QString folder;           // the project folder or the replica folder: `livesync/` inside it holds authority.json
        QString localRoot;        // device-local files (recovery drafts): <replica>/livesync-local
        net::DeviceIdentity *identity = nullptr;
        Clock *clock = nullptr;
        HostingBackend *backend = nullptr;
        // sends a message to a member through its connection; false when not connected
        std::function<bool(const QString &deviceId, const QJsonObject &message)> send;
        bool android = false;     // an Android device hosts only when the membership says so (nothing else changes here)
    };
    Authority(const Config &config, QObject *parent = nullptr);
    ~Authority() override;

    // ---- beginning
    // reads authority.json and membership.json (absent = a project this device has not joined yet; false with a reason when a file is damaged)
    bool load(QString *error = nullptr);
    // this device shares the project: founding proof for `epoch`, ACTIVE, hosting started
    bool found(const Membership &signedMembership, qint64 epoch, QString *error = nullptr);
    // this device joined: it takes the chain and the signed membership its paired host gave (verified; the host's key is
    // known from pairing), FOLLOWER
    bool joinWith(const QList<Proof> &chain, const Membership &signedMembership, QString *error = nullptr);
    // the restart rules (6.5): ACTIVE/DRAINING/PREPARED host -> ACTIVE (abort), RETIRED -> resend the proof then client, a target
    // with a proof -> ACTIVE, PREPARED without one -> FOLLOWER. Call after load()
    void start();
    bool isStarted() const { return m_started; }

    // ---- what the service tells
    void peerConnected(const QString &deviceId);   // a member's connection is ready (this sends our chain)
    void peerDisconnected(const QString &deviceId);
    void heardFrom(const QString &deviceId);       // any frame of a device (the host's heartbeat renews "contact")
    void setPeerInSync(const QString &deviceId, bool inSync); // the device holds the host's head (presence)
    // a message of the hosting types (AUTHORITY, MEMBERSHIP, ELECTION_REQUEST, VOTE, HANDOVER_*) from an authenticated device;
    // true = it was one of ours
    bool handleMessage(const QString &fromDeviceId, const QJsonObject &message);

    // ---- what the service asks
    QString project() const { return m_cfg.project; }
    QString host() const { return m_rec.host; }
    bool isHost() const { return m_rec.host == selfId() && (m_rec.state == AuthState::Active || m_rec.state == AuthState::Draining || m_rec.state == AuthState::Prepared); }
    qint64 epoch() const { return m_rec.epoch; }
    AuthState state() const { return m_rec.state; }
    const AuthorityRecord &record() const { return m_rec; }
    const Membership &membership() const { return m_members; }
    QList<Proof> chain() const { return m_rec.chain; }
    QString selfId() const;
    // the host writes a new membership (version + 1, signed with this device's key) and tells the members
    bool publishMembership(QList<Member> members, QString *error = nullptr);
    // planned handover ("Transfer Hosting..."); false with a reason when it cannot start
    bool transferHosting(const QString &targetDeviceId, QString *why = nullptr);
    bool handoverRunning() const;
    void abortHandover(const QString &reason = QStringLiteral("cancelled"));
    // the text of the warning for an unknown device (translated)
    QString unknownDeviceWarning() const;
    // the dialog of a drafts list reads this store
    RecoveryDrafts &drafts() { return m_drafts; }

    // ---- tests
    // called at each named point of a step; true = the process dies here (the object does nothing more)
    void setKillHook(std::function<bool(const QString &point)> hook) { m_kill = std::move(hook); }
    bool isDead() const { return m_dead; }
    void tick(); // also run on every tick of the clock

signals:
    void hostChanged(const QString &host, qint64 epoch);
    void stateChanged(qb::collab::AuthState state);
    // a device that is no member of the project (or whose proof does not hold) announced hosting, asked for votes or offered a
    // handover: refused. The service shows unknownDeviceWarning().
    void unknownDeviceRefused(const QString &deviceId, const QString &reason);
    // a member without the right to host (reader, Android not marked) claimed it
    void hostingClaimRefused(const QString &deviceId, const QString &code);
    void electionStarted(qint64 epoch);
    void electionFinished(bool won, qint64 epoch);
    void handoverFinished(bool ok, const QString &reason);
    void drainTimedOut();
    // the old host (or a client that followed it) turned what was accepted after the split into drafts
    void reunited(int drafts, const QString &newHost);
    void membershipChanged();
    void epochConflict(const QString &ours, const QString &theirs); // two hosts in one epoch (should not happen)
    void failed(const QString &what); // a file could not be written; the step was not taken

private:
    friend class Election;
    friend class Handover;
    bool persist();
    bool persistMembership();
    void setState(AuthState s);
    bool sendTo(const QString &deviceId, const QJsonObject &m);
    void sendAuthorityTo(const QString &deviceId);
    void broadcastAuthority();
    bool killed(const char *point);
    bool refuse(const QString &from, const QJsonObject &m, const QString &code, const QString &reason, bool unknown);
    bool online(const QString &deviceId) const { return m_online.contains(deviceId); }
    qint64 floorEpoch() const;
    void onAuthority(const QString &from, const QJsonObject &m);
    void onMembership(const QString &from, const QJsonObject &m);
    // adopt a verified chain that is ahead of ours
    bool adopt(const QList<Proof> &chain, const ChainResult &r, const QString &from);
    void checkHandback();
    void resetHostContact();

    Config m_cfg;
    AuthorityRecord m_rec;
    Membership m_members;
    RecoveryDrafts m_drafts;
    std::unique_ptr<Election> m_election;
    std::unique_ptr<Handover> m_handover;
    QSet<QString> m_online;
    QHash<QString, qint64> m_syncSince;       // device -> clock time it became online and in sync
    QSet<QString> m_inSync;
    QHash<QString, qint64> m_answeredAt;      // "device|epoch" -> a chain was already sent back
    qint64 m_lastHostContact = 0;
    qint64 m_handbackBlockedUntil = 0;
    std::function<bool(const QString &)> m_kill;
    bool m_dead = false;
    bool m_started = false;
    bool m_loaded = false;
};

} // namespace collab
} // namespace qb
