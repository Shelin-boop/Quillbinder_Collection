#include "core/collab/Handover.h"

#include "core/collab/net/DeviceIdentity.h"

namespace qb {
namespace collab {

namespace {
bool wholeNumber(const QJsonValue &v) { return v.isDouble() && v.toDouble() >= 0 && v.toDouble() < 1e15; }
} // namespace

bool Handover::running() const
{
    const AuthorityRecord &r = A.m_rec;
    return r.host == A.selfId() && (r.state == AuthState::Draining || r.state == AuthState::Prepared || r.state == AuthState::Retired);
}

void Handover::reset()
{
    m_hasPend = false;
    m_pend = Pending();
}

// ---------------------------------------------------------------- old host

bool Handover::start(const QString &target, QString *why)
{
    AuthorityRecord &r = A.m_rec;
    auto fail = [&](const QString &w) {
        if (why)
            *why = w;
        return false;
    };
    if (A.m_dead || !A.m_started)
        return fail(QStringLiteral("not running"));
    if (r.state != AuthState::Active || r.host != A.selfId())
        return fail(QStringLiteral("this device does not host the project now"));
    if (target == A.selfId() || !A.m_members.mayHost(target))
        return fail(QStringLiteral("the target may not host"));
    if (!A.online(target))
        return fail(QStringLiteral("the target is not reachable"));
    if (r.highest > r.epoch)
        return fail(QStringLiteral("another epoch is under way"));
    const AuthorityRecord before = r;
    r.state = AuthState::Draining;
    r.hoTarget = target;
    r.hoEpoch = r.epoch + 1;
    r.highest = qMax(r.highest, r.hoEpoch);
    r.hoHasProof = false;
    if (!A.persist()) {
        r = before;
        return fail(QStringLiteral("the files of Live Sync cannot be written"));
    }
    A.m_cfg.backend->setAcceptingLocks(false);
    m_drainStart = A.m_cfg.clock->now();
    emit A.stateChanged(r.state);
    A.killed("draining-persisted");
    return true;
}

void Handover::enterPrepared()
{
    AuthorityRecord &r = A.m_rec;
    HostingBackend *b = A.m_cfg.backend;
    b->setAcceptingCommits(false);
    const qint64 s = b->seq();
    const QString h = b->stateHash();
    m_locks = b->lockTable();
    r.state = AuthState::Prepared;
    r.hoSeq = s;
    r.hoHash = h;
    r.hoLocks = m_locks;
    if (!A.persist()) {
        abort(QStringLiteral("the state could not be written"), true);
        return;
    }
    emit A.stateChanged(r.state);
    if (A.killed("prepared-persisted"))
        return;
    A.sendTo(r.hoTarget, proto::make(proto::T::HANDOVER_PREPARE, A.m_cfg.project,
                                     {{QStringLiteral("epoch"), double(r.hoEpoch)},
                                      {QStringLiteral("from"), double(r.epoch)},
                                      {QStringLiteral("seq"), double(s)},
                                      {QStringLiteral("hash"), h},
                                      {QStringLiteral("locks"), m_locks},
                                      {QStringLiteral("target"), r.hoTarget}}));
    m_preparedAt = A.m_cfg.clock->now();
    A.killed("prepare-sent");
}

void Handover::abort(const QString &reason, bool tellTarget)
{
    AuthorityRecord &r = A.m_rec;
    if (r.host != A.selfId() || (r.state != AuthState::Draining && r.state != AuthState::Prepared))
        return; // from RETIRED on there is no way back
    const QString target = r.hoTarget;
    const qint64 epoch = r.hoEpoch;
    r.state = AuthState::Active;
    r.hoTarget.clear();
    r.hoEpoch = r.hoSeq = 0;
    r.hoHash.clear();
    r.hoLocks = QJsonArray();
    A.persist(); // (if this fails, a restart reads PREPARED/DRAINING and comes back as the host anyway)
    A.m_cfg.backend->setAcceptingLocks(true);
    A.m_cfg.backend->setAcceptingCommits(true);
    if (tellTarget && !target.isEmpty())
        A.sendTo(target, proto::make(proto::T::HANDOVER_ABORT, A.m_cfg.project, {{QStringLiteral("epoch"), double(epoch)}, {QStringLiteral("reason"), reason}}));
    A.m_handbackBlockedUntil = A.m_cfg.clock->now() + kHandbackMs;
    emit A.stateChanged(r.state);
    emit A.handoverFinished(false, reason);
}

void Handover::sendCommit()
{
    const AuthorityRecord &r = A.m_rec;
    A.sendTo(r.hoProof.host, proto::make(proto::T::HANDOVER_COMMIT, A.m_cfg.project, {{QStringLiteral("proof"), r.hoProof.toJson()}, {QStringLiteral("locks"), r.hoLocks}}));
    m_lastResend = A.m_cfg.clock->now();
}

void Handover::onReady(const QString &from, const QJsonObject &m)
{
    AuthorityRecord &r = A.m_rec;
    if (r.state != AuthState::Prepared || r.host != A.selfId() || from != r.hoTarget || !wholeNumber(m.value(QStringLiteral("epoch"))) ||
        qint64(m.value(QStringLiteral("epoch")).toDouble()) != r.hoEpoch)
        return;
    if (A.killed("ready-received"))
        return;
    const AuthorityRecord before = r;
    r.hoProof = makeHandover(A.m_cfg.project, r.epoch, r.hoEpoch, r.hoTarget, r.hoSeq, r.hoHash, locksDigestOf(r.hoLocks), *A.m_cfg.identity);
    r.hoHasProof = true;
    r.state = AuthState::Retired;
    if (!A.persist()) {
        r = before;
        abort(QStringLiteral("the state could not be written"), true);
        return;
    }
    emit A.stateChanged(r.state);
    if (A.killed("retired-persisted"))
        return;
    A.m_cfg.backend->stopHosting();
    sendCommit();
    A.killed("commit-sent");
}

void Handover::onDone(const QString &from, const QJsonObject &m)
{
    AuthorityRecord &r = A.m_rec;
    if (r.state != AuthState::Retired || from != r.hoProof.host || !wholeNumber(m.value(QStringLiteral("epoch"))) ||
        qint64(m.value(QStringLiteral("epoch")).toDouble()) != r.hoProof.epoch)
        return;
    r.chain.append(r.hoProof);
    r.epoch = r.hoProof.epoch;
    r.host = r.hoProof.host;
    r.highest = qMax(r.highest, r.epoch);
    r.state = AuthState::Follower;
    r.hoHasProof = false;
    r.hoLocks = QJsonArray();
    r.hoTarget.clear();
    r.hoEpoch = r.hoSeq = 0;
    r.hoHash.clear();
    if (!A.persist())
        return; // still RETIRED on disk: the proof is resent and answered again
    A.resetHostContact();
    A.m_cfg.backend->followHost(r.host, r.epoch);
    emit A.stateChanged(r.state);
    emit A.hostChanged(r.host, r.epoch);
    emit A.handoverFinished(true, QString());
}

// ---------------------------------------------------------------- target

void Handover::abortTarget(const QString &to, qint64 epoch, const QString &reason)
{
    A.sendTo(to, proto::make(proto::T::HANDOVER_ABORT, A.m_cfg.project, {{QStringLiteral("epoch"), double(epoch)}, {QStringLiteral("reason"), reason}}));
}

void Handover::onPrepare(const QString &from, const QJsonObject &m)
{
    AuthorityRecord &r = A.m_rec;
    if (!wholeNumber(m.value(QStringLiteral("epoch"))) || !wholeNumber(m.value(QStringLiteral("from"))) || !wholeNumber(m.value(QStringLiteral("seq"))) ||
        !m.value(QStringLiteral("hash")).isString() || !m.value(QStringLiteral("locks")).isArray() ||
        m.value(QStringLiteral("target")).toString() != A.selfId()) {
        A.refuse(from, m, QStringLiteral("BAD_OP"), QStringLiteral("not a handover to this device"), false);
        return;
    }
    const qint64 epoch = qint64(m.value(QStringLiteral("epoch")).toDouble());
    const qint64 prev = qint64(m.value(QStringLiteral("from")).toDouble());
    // already prepared for this very handover: the same answer again
    if (r.state == AuthState::Prepared && r.host != A.selfId() && r.pendEpoch == epoch && r.pendFrom == from) {
        A.sendTo(from, proto::replyTo(m, proto::T::HANDOVER_READY, {{QStringLiteral("epoch"), double(epoch)}}));
        return;
    }
    if (from != r.host || prev != r.epoch || epoch != r.epoch + 1) {
        // somebody who is not our host offers a handover: an announcement of hosting that holds no proof
        A.refuse(from, m, QStringLiteral("NOT_HOST"), QStringLiteral("not the host"), false);
        return;
    }
    QString why;
    if (r.state != AuthState::Follower || !A.m_members.mayHost(A.selfId()) || !A.m_cfg.backend->replicaReady() || !A.m_cfg.backend->canHost(&why)) {
        abortTarget(from, epoch, QStringLiteral("not ready"));
        return;
    }
    m_pend.from = from;
    m_pend.epoch = epoch;
    m_pend.seq = qint64(m.value(QStringLiteral("seq")).toDouble());
    m_pend.hash = m.value(QStringLiteral("hash")).toString();
    m_pend.locks = m.value(QStringLiteral("locks")).toArray();
    m_pend.msgId = m.value(QStringLiteral("id")).toString();
    m_pend.since = A.m_cfg.clock->now();
    m_hasPend = true;
    tryPrepare();
}

void Handover::tryPrepare()
{
    AuthorityRecord &r = A.m_rec;
    HostingBackend *b = A.m_cfg.backend;
    if (b->seq() < m_pend.seq)
        return; // the last events are still on their way; tick() looks again
    if (b->seq() > m_pend.seq || b->stateHash() != m_pend.hash) {
        abortTarget(m_pend.from, m_pend.epoch, QStringLiteral("other state"));
        reset();
        return;
    }
    const AuthorityRecord before = r;
    r.state = AuthState::Prepared;
    r.pendEpoch = m_pend.epoch;
    r.pendFrom = m_pend.from;
    r.pendSeq = m_pend.seq;
    r.pendHash = m_pend.hash;
    r.highest = qMax(r.highest, m_pend.epoch);
    if (!A.persist()) {
        r = before;
        abortTarget(m_pend.from, m_pend.epoch, QStringLiteral("not written"));
        reset();
        return;
    }
    m_prepSince = A.m_cfg.clock->now();
    const QString from = m_pend.from;
    const qint64 epoch = m_pend.epoch;
    QJsonObject ready = proto::make(proto::T::HANDOVER_READY, A.m_cfg.project, {{QStringLiteral("epoch"), double(epoch)}});
    ready.insert(QStringLiteral("re"), m_pend.msgId);
    m_hasPend = false;
    emit A.stateChanged(r.state);
    if (A.killed("target-prepared-persisted"))
        return;
    A.sendTo(from, ready);
    A.killed("ready-sent");
}

void Handover::onCommit(const QString &from, const QJsonObject &m)
{
    AuthorityRecord &r = A.m_rec;
    Proof proof;
    QString why;
    if (!Proof::fromJson(m.value(QStringLiteral("proof")).toObject(), &proof, &why) || !m.value(QStringLiteral("locks")).isArray()) {
        A.refuse(from, m, QStringLiteral("BAD_OP"), why, false);
        return;
    }
    // we are the host from this proof already: the old host did not get DONE
    if (r.state == AuthState::Active && r.host == A.selfId()) {
        for (const Proof &p : r.chain)
            if (p == proof) {
                A.sendTo(from, proto::replyTo(m, proto::T::HANDOVER_DONE, {{QStringLiteral("epoch"), double(proof.epoch)}}));
                return;
            }
        return;
    }
    if (r.state != AuthState::Follower && !(r.state == AuthState::Prepared && r.host != A.selfId()))
        return;
    if (proof.kind != Proof::Handover || proof.host != A.selfId() || proof.signer != from) {
        A.refuse(from, m, QStringLiteral("NOT_HOST"), QStringLiteral("a handover that is not for this device"), false);
        return;
    }
    QList<Proof> chain = r.chain;
    chain.append(proof);
    const ChainResult res = verifyChain(chain, A.m_members, r.chain);
    if (!res.ok || res.host != A.selfId()) {
        A.refuse(from, m, QStringLiteral("UNKNOWN_DEVICE"), res.why, true);
        return;
    }
    const QJsonArray locks = m.value(QStringLiteral("locks")).toArray();
    if (locksDigestOf(locks) != proof.locksDigest) {
        A.refuse(from, m, QStringLiteral("BAD_OP"), QStringLiteral("the locks do not match the proof"), false);
        return;
    }
    HostingBackend *b = A.m_cfg.backend;
    if (b->seq() != proof.baseSeq || b->stateHash() != proof.baseHash) {
        A.sendTo(from, proto::errorMessage(QStringLiteral("BEHIND"), m.value(QStringLiteral("id")).toString(), QString(), A.m_cfg.project));
        return;
    }
    const AuthorityRecord before = r;
    r.chain = chain;
    r.epoch = proof.epoch;
    r.host = A.selfId();
    r.highest = qMax(r.highest, proof.epoch);
    r.state = AuthState::Active;
    r.pendEpoch = r.pendSeq = 0;
    r.pendFrom.clear();
    r.pendHash.clear();
    if (!A.persist()) {
        r = before;
        return;
    }
    reset();
    if (A.killed("target-active-persisted"))
        return;
    A.resetHostContact();
    QString err;
    if (!b->startHosting(proof.epoch, locks, &err))
        emit A.failed(err);
    A.sendTo(from, proto::replyTo(m, proto::T::HANDOVER_DONE, {{QStringLiteral("epoch"), double(proof.epoch)}}));
    emit A.stateChanged(r.state);
    emit A.hostChanged(r.host, r.epoch);
    if (A.killed("done-sent"))
        return;
    A.broadcastAuthority();
}

void Handover::onAbort(const QString &from, const QJsonObject &m)
{
    AuthorityRecord &r = A.m_rec;
    if (!wholeNumber(m.value(QStringLiteral("epoch"))))
        return;
    const qint64 epoch = qint64(m.value(QStringLiteral("epoch")).toDouble());
    if (m_hasPend && from == m_pend.from && epoch == m_pend.epoch)
        reset();
    if (r.state == AuthState::Prepared && r.host != A.selfId() && from == r.pendFrom && epoch == r.pendEpoch) {
        r.state = AuthState::Follower;
        r.pendEpoch = r.pendSeq = 0;
        r.pendFrom.clear();
        r.pendHash.clear();
        A.persist();
        emit A.stateChanged(r.state);
        return;
    }
    // the target says no
    if ((r.state == AuthState::Draining || r.state == AuthState::Prepared) && r.host == A.selfId() && from == r.hoTarget && epoch == r.hoEpoch)
        abort(m.value(QStringLiteral("reason")).toString(), false);
}

void Handover::tick()
{
    AuthorityRecord &r = A.m_rec;
    const qint64 now = A.m_cfg.clock->now();
    if (r.host == A.selfId()) {
        if (r.state == AuthState::Draining) {
            const bool timedOut = now - m_drainStart >= kDrainMs;
            if (!A.online(r.hoTarget)) {
                abort(QStringLiteral("the target is not reachable"), true);
            } else if (A.m_cfg.backend->drained() || timedOut) {
                if (!A.m_cfg.backend->drained())
                    emit A.drainTimedOut();
                enterPrepared();
            }
        } else if (r.state == AuthState::Prepared) {
            if (!A.online(r.hoTarget) || now - m_preparedAt >= kHandoverWaitMs)
                abort(QStringLiteral("no answer from the target"), true);
        } else if (r.state == AuthState::Retired) {
            if (now - m_lastResend >= kResendMs && A.online(r.hoProof.host))
                sendCommit();
        }
        return;
    }
    if (m_hasPend) {
        if (A.m_cfg.backend->seq() >= m_pend.seq)
            tryPrepare();
        else if (now - m_pend.since >= kTargetWaitMs) {
            abortTarget(m_pend.from, m_pend.epoch, QStringLiteral("behind"));
            reset();
        }
    } else if (r.state == AuthState::Prepared && now - m_prepSince >= kPendingMs) {
        // no proof came: back to FOLLOWER (a proof that comes later is still taken; the epoch stays promised)
        r.state = AuthState::Follower;
        r.pendEpoch = r.pendSeq = 0;
        r.pendFrom.clear();
        r.pendHash.clear();
        A.persist();
        emit A.stateChanged(r.state);
    }
}

} // namespace collab
} // namespace qb
