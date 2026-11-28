#include "core/collab/Locks.h"

#include "core/PathSafety.h"

#include <QJsonObject>
#include <QUuid>

namespace qb {
namespace collab {

LockKind lockKindOf(const QString &r)
{
    if (r == QLatin1String("structure"))
        return LockKind::Structure;
    if (r == QLatin1String("settings"))
        return LockKind::Settings;
    if (r == QLatin1String("project"))
        return LockKind::Project;
    auto idOk = [](const QString &s) { return !s.isEmpty() && s.size() <= 128 && pathsafety::isSafeId(s); };
    if (r.startsWith(QLatin1String("doc:")) && idOk(r.mid(4)))
        return LockKind::Doc;
    if (r.startsWith(QLatin1String("object:")) && idOk(r.mid(7)))
        return LockKind::Object;
    if (r.startsWith(QLatin1String("mindmap:")) && idOk(r.mid(8)))
        return LockKind::Object;
    return LockKind::Invalid;
}

bool validResource(const QString &resource) { return lockKindOf(resource) != LockKind::Invalid; }

bool conflicts(const QString &wanted, const QString &held)
{
    const LockKind w = lockKindOf(wanted), h = lockKindOf(held);
    if (w == LockKind::Invalid || h == LockKind::Invalid)
        return true; // what is not understood is never let through
    if (w == LockKind::Project || h == LockKind::Project)
        return true;
    return wanted == held; // the other pairs of the matrix are free
}

// ---------------------------------------------------------------- LockTable

void LockTable::setGenerationReserve(qint64 reservedUpTo, std::function<bool(qint64)> persist)
{
    m_reserve = reservedUpTo;
    m_last = reservedUpTo; // after a restart nothing of the old block is used again
    m_persist = std::move(persist);
}

qint64 LockTable::nextGeneration(bool *failed)
{
    if (m_last >= m_reserve) {
        const qint64 more = m_reserve + kGenerationBlock;
        if (m_persist && !m_persist(more)) {
            if (failed)
                *failed = true;
            return 0;
        }
        m_reserve = more;
    }
    return ++m_last;
}

QList<Lock> LockTable::blockers(const QString &wanted, const Owner &requester) const
{
    QList<Lock> out;
    for (const Lock &l : m_locks)
        if (l.owner != requester && conflicts(wanted, l.resource))
            out << l;
    return out;
}

const Lock *LockTable::find(const QString &resource) const
{
    const auto it = m_locks.constFind(resource);
    return it == m_locks.constEnd() ? nullptr : &it.value();
}

QList<Lock> LockTable::locksOf(const Owner &owner) const
{
    QList<Lock> out;
    for (const Lock &l : m_locks)
        if (l.owner == owner)
            out << l;
    return out;
}

QList<Lock> LockTable::locksOfDevice(const QString &device) const
{
    QList<Lock> out;
    for (const Lock &l : m_locks)
        if (l.owner.device == device)
            out << l;
    return out;
}

LockTable::Attempt LockTable::acquire(const Owner &owner, const QString &ownerName, const QStringList &wanted)
{
    Attempt a;
    QStringList sorted = wanted;
    sorted.removeDuplicates();
    sorted.sort(); // one step, always in the same order: no deadlock
    for (const QString &r : sorted) {
        if (!validResource(r)) {
            a.wanted = r;
            return a;
        }
        const QList<Lock> b = blockers(r, owner);
        if (!b.isEmpty()) {
            a.wanted = r;
            a.heldResource = b.first().resource;
            a.holder = b.first().owner;
            a.holderName = b.first().ownerName;
            return a;
        }
    }
    const qint64 now = m_clock->now();
    QList<Lock> fresh;
    for (const QString &r : sorted) {
        const auto it = m_locks.constFind(r);
        if (it != m_locks.constEnd() && it->owner == owner) {
            a.locks << *it;
            continue;
        }
        bool failed = false;
        const qint64 g = nextGeneration(&failed);
        if (failed) { // nothing of this step is granted
            a.locks.clear();
            a.storageFailed = true;
            return a;
        }
        Lock l;
        l.resource = r;
        l.owner = owner;
        l.ownerName = ownerName;
        l.epoch = m_epoch;
        l.generation = g;
        l.lease = QUuid::createUuid().toString(QUuid::WithoutBraces);
        l.grantedAt = l.renewedAt = now;
        fresh << l;
        a.locks << l;
    }
    for (const Lock &l : fresh) {
        m_locks.insert(l.resource, l);
        m_flagged.remove(l.resource);
    }
    a.ok = true;
    return a;
}

bool LockTable::release(const Owner &owner, const QString &resource)
{
    const auto it = m_locks.constFind(resource);
    if (it == m_locks.constEnd() || it->owner != owner)
        return false;
    m_locks.erase(it);
    m_flagged.remove(resource);
    return true;
}

QList<Lock> LockTable::releaseAllOf(const Owner &owner)
{
    QList<Lock> out = locksOf(owner);
    for (const Lock &l : out) {
        m_locks.remove(l.resource);
        m_flagged.remove(l.resource);
    }
    return out;
}

Lock LockTable::takeOver(const QString &resource, const Owner &to, const QString &toName, Lock *previous, bool *storageFailed)
{
    const auto old = m_locks.constFind(resource);
    if (previous)
        *previous = old == m_locks.constEnd() ? Lock() : *old;
    bool failed = false;
    const qint64 g = nextGeneration(&failed);
    if (failed) {
        if (storageFailed)
            *storageFailed = true;
        return Lock();
    }
    Lock l;
    l.resource = resource;
    l.owner = to;
    l.ownerName = toName;
    l.epoch = m_epoch;
    l.generation = g;
    l.lease = QUuid::createUuid().toString(QUuid::WithoutBraces);
    l.grantedAt = l.renewedAt = m_clock->now();
    m_locks.insert(resource, l);
    m_flagged.remove(resource);
    return l;
}

int LockTable::moveSessions(const QString &device, const std::function<bool(const Owner &)> &isGone, const Owner &to)
{
    int n = 0;
    for (Lock &l : m_locks)
        if (l.owner.device == device && l.owner != to && isGone(l.owner)) {
            l.owner = to;
            ++n;
        }
    return n;
}

void LockTable::renewDevice(const QString &device)
{
    const qint64 now = m_clock->now();
    for (Lock &l : m_locks)
        if (l.owner.device == device) {
            l.renewedAt = now;
            m_flagged.remove(l.resource);
        }
}

QList<Lock> LockTable::expire()
{
    QList<Lock> out;
    const qint64 now = m_clock->now();
    for (auto it = m_locks.begin(); it != m_locks.end();) {
        if (now - it->renewedAt >= kLeaseExpireMs) {
            out << *it;
            m_flagged.remove(it->resource);
            it = m_locks.erase(it);
        } else
            ++it;
    }
    return out;
}

QList<Lock> LockTable::newlyUnreachable()
{
    QList<Lock> out;
    for (const Lock &l : m_locks)
        if (!isReachable(l) && !m_flagged.value(l.resource)) {
            m_flagged.insert(l.resource, true);
            out << l;
        }
    return out;
}

bool LockTable::valid(const proto::Token &t) const
{
    const Lock *l = find(t.resource);
    return l && l->epoch == t.epoch && l->generation == t.generation && l->lease == t.lease;
}

QJsonArray LockTable::toJson() const
{
    QList<Lock> list = m_locks.values();
    std::sort(list.begin(), list.end(), [](const Lock &a, const Lock &b) { return a.resource < b.resource; });
    QJsonArray out;
    for (const Lock &l : list)
        out.append(QJsonObject{{QStringLiteral("resource"), l.resource},
                               {QStringLiteral("device"), l.owner.device},
                               {QStringLiteral("session"), l.owner.session},
                               {QStringLiteral("name"), l.ownerName},
                               {QStringLiteral("generation"), double(l.generation)},
                               {QStringLiteral("epoch"), double(l.epoch)},
                               {QStringLiteral("lease"), l.lease},
                               {QStringLiteral("unreachable"), !isReachable(l)}});
    return out;
}

// ---------------------------------------------------------------- EditRequests

EditRequest &EditRequests::open(const QString &resource, const Owner &asker, const QString &askerName)
{
    if (EditRequest *r = find(resource, asker))
        return *r;
    EditRequest r;
    r.resource = resource;
    r.asker = asker;
    r.askerName = askerName;
    r.openedAt = m_clock->now();
    r.deadline = r.openedAt + kEditWaitMs;
    m_list << r;
    return m_list.last();
}

EditRequest *EditRequests::find(const QString &resource, const Owner &asker)
{
    for (EditRequest &r : m_list)
        if (r.resource == resource && r.asker == asker)
            return &r;
    return nullptr;
}

QList<EditRequest *> EditRequests::forResource(const QString &resource)
{
    QList<EditRequest *> out;
    for (EditRequest &r : m_list)
        if (r.resource == resource)
            out << &r;
    return out;
}

EditRequest *EditRequests::decline(const QString &resource, const Owner &asker)
{
    EditRequest *r = nullptr;
    if (asker.isEmpty()) {
        const QList<EditRequest *> l = forResource(resource);
        if (!l.isEmpty())
            r = l.first();
    } else
        r = find(resource, asker);
    if (!r)
        return nullptr;
    r->deadline = m_clock->now() + kEditWaitMs; // the waiting time starts again
    r->offered = false;
    ++r->declines;
    return r;
}

QList<EditRequest> EditRequests::dueNow()
{
    QList<EditRequest> out;
    for (EditRequest &r : m_list)
        if (!r.offered && mayForce(r)) {
            r.offered = true;
            out << r;
        }
    return out;
}

bool EditRequests::cancel(const QString &resource, const Owner &asker)
{
    for (int i = 0; i < m_list.size(); ++i)
        if (m_list.at(i).resource == resource && m_list.at(i).asker == asker) {
            m_list.removeAt(i);
            return true;
        }
    return false;
}

void EditRequests::dropOf(const Owner &asker)
{
    for (int i = m_list.size() - 1; i >= 0; --i)
        if (m_list.at(i).asker == asker)
            m_list.removeAt(i);
}

void EditRequests::dropResource(const QString &resource)
{
    for (int i = m_list.size() - 1; i >= 0; --i)
        if (m_list.at(i).resource == resource)
            m_list.removeAt(i);
}

} // namespace collab
} // namespace qb
