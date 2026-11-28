#pragma once
// Live Sync's locks (0.99.0), section 5 of the design. The host keeps one LockTable per hosted project; nothing here knows a
// network or a disk (the host persists the generation reserve through a callback), so the rules are checked by the tests
// with a fake clock.
//   Resources are strings: "doc:<id>" (text, notes, fields of one document), "structure" (create, move, order, trash),
//   "settings", "object:<oid>" / "object:schema" / "object:images" / "mindmap:<id>" (an object, the schema, a mind map board),
//   "project" (exclusive, whole-project jobs). The matrix of 5.1 is `conflicts`.
//   A lock is a lease: the owner's heartbeat renews all its leases; 10 s without a heartbeat shows the lock as not reachable,
//   30 s expire it. Fencing: a lock's token is (epoch, generation, lease id); generations come from blocks of 100 that the
//   host stores before it uses them, so a generation is never used twice, also after a restart.
//   An unused lock is never given up by itself (the writer's decision): others ask, the holder answers, and after 20 s
//   without an answer the asker may take it over (EditRequests).
#include "core/collab/Clock.h"
#include "core/collab/proto/Messages.h"

#include <QHash>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <functional>

namespace qb {
namespace collab {

struct Owner {
    QString device;  // the authenticated device id
    QString session; // the window or service session on that device
    QString key() const { return device + QLatin1Char('/') + session; }
    bool isEmpty() const { return device.isEmpty(); }
    bool operator==(const Owner &o) const { return device == o.device && session == o.session; }
    bool operator!=(const Owner &o) const { return !(*this == o); }
};

enum class LockKind { Doc, Structure, Settings, Object, Project, Invalid };
LockKind lockKindOf(const QString &resource);
bool validResource(const QString &resource);
// the matrix of 5.1: whether a lock `held` by another owner stops the wish for `wanted`
bool conflicts(const QString &wanted, const QString &held);

struct Lock {
    QString resource;
    Owner owner;
    QString ownerName; // the device's name as the others see it
    qint64 epoch = 0;
    qint64 generation = 0;
    QString lease;
    qint64 grantedAt = 0;
    qint64 renewedAt = 0;
    proto::Token token() const { return proto::Token{resource, epoch, generation, lease}; }
};

constexpr qint64 kHeartbeatMs = 3000;
constexpr qint64 kUnreachableMs = 10000;
constexpr qint64 kLeaseExpireMs = 30000;
constexpr qint64 kEditWaitMs = 20000;
constexpr qint64 kGenerationBlock = 100;

class LockTable {
public:
    explicit LockTable(Clock *clock, qint64 epoch = 1) : m_clock(clock), m_epoch(epoch) {}
    qint64 epoch() const { return m_epoch; }
    void setEpoch(qint64 e) { m_epoch = e; }

    // The next generation is taken from the reserved block; when the block is used up a new one (100 more) is reserved and
    // `persist(newReserve)` runs first: false from it means the reserve could not be stored and nothing is granted.
    void setGenerationReserve(qint64 reservedUpTo, std::function<bool(qint64)> persist);
    qint64 generationReserve() const { return m_reserve; }
    qint64 lastGeneration() const { return m_last; }

    struct Attempt {
        bool ok = false;
        QString wanted;       // the resource that could not be locked
        QString heldResource; // the lock that stood in the way
        Owner holder;
        QString holderName;
        QList<Lock> locks;    // the locks the owner has now for the resources asked (new or kept)
        bool storageFailed = false;
    };
    // all or none, in one step. A resource the owner holds already stays as it is (same token).
    Attempt acquire(const Owner &owner, const QString &ownerName, const QStringList &resources);
    // the locks of other owners that stand in the way of `wanted`
    QList<Lock> blockers(const QString &wanted, const Owner &requester) const;
    const Lock *find(const QString &resource) const;
    QList<Lock> all() const { return m_locks.values(); }
    QList<Lock> locksOf(const Owner &owner) const;
    QList<Lock> locksOfDevice(const QString &device) const;

    // false when the owner does not hold it
    bool release(const Owner &owner, const QString &resource);
    // the locks released
    QList<Lock> releaseAllOf(const Owner &owner);
    // A takeover: the resource goes to `to` with the next generation; the old lock is returned (resource empty when there was
    // none). Never refused here: the host checks the time and makes the protective snapshot first.
    Lock takeOver(const QString &resource, const Owner &to, const QString &toName, Lock *previous = nullptr, bool *storageFailed = nullptr);
    // a lock given to another owner of the same device (a new window takes over the leases of a crashed one)
    int moveSessions(const QString &device, const std::function<bool(const Owner &)> &isGone, const Owner &to);

    // the heartbeat of a device renews all its leases
    void renewDevice(const QString &device);
    bool isReachable(const Lock &l) const { return m_clock->now() - l.renewedAt < kUnreachableMs; }
    // the locks whose lease ran out; they are removed
    QList<Lock> expire();
    // the locks that became unreachable since the last call (to tell the others once)
    QList<Lock> newlyUnreachable();

    // whether the token is exactly the current lock of its resource
    bool valid(const proto::Token &t) const;
    QJsonArray toJson() const;
    void clear() { m_locks.clear(); m_flagged.clear(); }

private:
    qint64 nextGeneration(bool *failed);
    Clock *m_clock;
    qint64 m_epoch;
    QHash<QString, Lock> m_locks;
    QHash<QString, bool> m_flagged; // resource -> already told as unreachable
    qint64 m_reserve = 0;
    qint64 m_last = 0;
    std::function<bool(qint64)> m_persist;
};

// A request to edit what another device holds (5.4). One list of open requests per host.
struct EditRequest {
    QString resource;
    Owner asker;
    QString askerName;
    qint64 openedAt = 0;
    qint64 deadline = 0;   // when the asker may take over
    int declines = 0;
    bool offered = false;  // the asker was told that taking over is possible now
};

class EditRequests {
public:
    explicit EditRequests(Clock *clock) : m_clock(clock) {}
    // opens a request (or returns the asker's open one for that resource, unchanged)
    EditRequest &open(const QString &resource, const Owner &asker, const QString &askerName);
    EditRequest *find(const QString &resource, const Owner &asker);
    // the open requests for a resource, oldest first
    QList<EditRequest *> forResource(const QString &resource);
    // The holder said no: the waiting time starts again, for the asker named (or the oldest request when none is named).
    // Returns the request, or nullptr.
    EditRequest *decline(const QString &resource, const Owner &asker = Owner());
    // the asker may take over now: the time has run out
    bool mayForce(const EditRequest &r) const { return m_clock->now() >= r.deadline; }
    qint64 remainingMs(const EditRequest &r) const { return qMax<qint64>(0, r.deadline - m_clock->now()); }
    // requests whose time ran out and whose asker was not told yet (marked as told)
    QList<EditRequest> dueNow();
    bool cancel(const QString &resource, const Owner &asker);
    void dropOf(const Owner &asker);
    void dropResource(const QString &resource);
    int count() const { return m_list.size(); }

private:
    Clock *m_clock;
    QList<EditRequest> m_list;
};

} // namespace collab
} // namespace qb
