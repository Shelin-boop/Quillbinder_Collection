#include "core/collab/Election.h"

#include "core/collab/net/DeviceIdentity.h"
#include <cstdio>

namespace qb {
namespace collab {

namespace {
constexpr int kKeepVotes = 16;

bool wholeNumber(const QJsonValue &v) { return v.isDouble() && v.toDouble() >= 0 && v.toDouble() < 1e15; }
} // namespace

qint64 Election::nextEpoch() const
{
    qint64 top = qMax(A.floorEpoch(), A.m_rec.highest);
    for (auto it = A.m_rec.votes.constBegin(); it != A.m_rec.votes.constEnd(); ++it)
        top = qMax(top, it.key());
    return top + 1;
}

void Election::tick()
{
    AuthorityRecord &rec = A.m_rec;
    if (rec.state != AuthState::Follower || rec.host == A.selfId() || rec.chain.isEmpty())
        return;
    const qint64 now = A.m_cfg.clock->now();
    if (m_active) {
        if (now - m_started >= kElectionRoundMs) {
            m_active = false;
            m_nextAllowed = now + kElectionRetryMs;
            emit A.electionFinished(false, m_epoch);
        }
        return;
    }
    if (now < m_nextAllowed || !A.m_members.mayHost(A.selfId()) || !A.m_cfg.backend->replicaReady())
        return;
    const QStringList cands = A.m_members.hostCandidates(rec.host);
    const int rank = int(cands.indexOf(A.selfId())) + 1;
    if (rank == 0)
        return;
    if (now - A.m_lastHostContact < kFailoverMs + kRankStaggerMs * (rank - 1))
        return;
    QString why;
    if (!A.m_cfg.backend->canHost(&why))
        return;
    if (votesNeeded(A.m_members.count()) == 0) { // two devices: the other takes over alone
        m_nextAllowed = now + kElectionRetryMs;
        emit A.electionStarted(nextEpoch());
        becomeHost(nextEpoch(), {}, A.m_cfg.backend->seq(), A.m_cfg.backend->stateHash());
        return;
    }
    beginRound(nextEpoch());
}

void Election::beginRound(qint64 epoch)
{
    AuthorityRecord &rec = A.m_rec;
    m_epoch = epoch;
    m_baseSeq = A.m_cfg.backend->seq();
    m_baseHash = A.m_cfg.backend->stateHash();
    m_votes.clear();
    m_voters.clear();
    // our own vote is written first, like every vote
    rec.votes.insert(epoch, A.selfId());
    rec.highest = qMax(rec.highest, epoch);
    if (!A.persist()) {
        rec.votes.remove(epoch);
        m_nextAllowed = A.m_cfg.clock->now() + kElectionRetryMs;
        return;
    }
    const QByteArray sig = A.m_cfg.identity->sign(voteBytes(A.m_cfg.project, epoch, A.selfId(), m_baseSeq, m_baseHash, A.selfId()));
    m_votes.append({A.selfId(), sig});
    m_voters.insert(A.selfId());
    m_active = true;
    m_started = A.m_cfg.clock->now();
    emit A.electionStarted(epoch);
    const QJsonObject req = proto::make(proto::T::ELECTION_REQUEST, A.m_cfg.project,
                                        {{QStringLiteral("epoch"), double(epoch)},
                                         {QStringLiteral("seq"), double(m_baseSeq)},
                                         {QStringLiteral("stateHash"), m_baseHash},
                                         {QStringLiteral("candidate"), A.selfId()}});
    const QSet<QString> peers = A.m_online;
    for (const QString &d : peers)
        if (d != rec.host && d != A.selfId() && A.m_members.isMember(d))
            A.sendTo(d, req);
}

void Election::onVote(const QString &from, const QJsonObject &m)
{
    if (!m_active || A.m_dead)
        return;
    if (!wholeNumber(m.value(QStringLiteral("epoch"))) || qint64(m.value(QStringLiteral("epoch")).toDouble()) != m_epoch)
        return;
    if (!m.value(QStringLiteral("granted")).toBool() || m.value(QStringLiteral("candidate")).toString() != A.selfId())
        return;
    if (from == A.m_rec.host || m_voters.contains(from) || m.value(QStringLiteral("voter")).toString() != from)
        return;
    const Member *vm = A.m_members.find(from);
    const QByteArray sig = QByteArray::fromBase64(m.value(QStringLiteral("sig")).toString().toLatin1());
    if (!vm || !net::DeviceIdentity::verify(vm->key, voteBytes(A.m_cfg.project, m_epoch, A.selfId(), m_baseSeq, m_baseHash, from), sig))
        return;
    m_votes.append({from, sig});
    m_voters.insert(from);
    if (m_voters.size() >= votesNeeded(A.m_members.count())) {
        m_active = false;
        becomeHost(m_epoch, m_votes, m_baseSeq, m_baseHash);
    }
}

void Election::becomeHost(qint64 epoch, const QList<Vote> &votes, qint64 baseSeq, const QString &baseHash)
{
    AuthorityRecord &rec = A.m_rec;
    const Proof proof = makeFailover(A.m_cfg.project, rec.epoch, epoch, baseSeq, baseHash, votes, *A.m_cfg.identity);
    const AuthorityRecord before = rec;
    rec.chain.append(proof);
    rec.epoch = epoch;
    rec.host = A.selfId();
    rec.highest = qMax(rec.highest, epoch);
    rec.state = AuthState::Active;
    rec.hoTarget.clear();
    rec.hoEpoch = rec.hoSeq = 0;
    rec.hoHasProof = false;
    if (!A.persist()) {
        rec = before;
        return;
    }
    if (A.killed("failover-persisted"))
        return;
    QString err;
    A.resetHostContact();
    A.m_cfg.backend->hostingAfterFailover();
    if (!A.m_cfg.backend->startHosting(epoch, QJsonArray(), &err))
        emit A.failed(err);
    emit A.stateChanged(rec.state);
    emit A.hostChanged(rec.host, epoch);
    emit A.electionFinished(true, epoch);
    A.broadcastAuthority();
}

void Election::onRequest(const QString &from, const QJsonObject &m)
{
    AuthorityRecord &rec = A.m_rec;
    const QString re = m.value(QStringLiteral("id")).toString();
    auto deny = [&](const char *code) { if (qEnvironmentVariableIsSet("QB_LS_TRACE")) std::fprintf(stderr, "[%s] denies %s: %s\n", qPrintable(A.selfId().left(4)), qPrintable(from.left(4)), code); A.sendTo(from, proto::errorMessage(QLatin1String(code), re, QString(), A.m_cfg.project)); };
    if (!wholeNumber(m.value(QStringLiteral("epoch"))) || !wholeNumber(m.value(QStringLiteral("seq"))) || !m.value(QStringLiteral("stateHash")).isString() ||
        m.value(QStringLiteral("candidate")).toString() != from) {
        deny("BAD_OP");
        return;
    }
    if (!A.m_members.mayHost(from) || from == rec.host) {
        A.refuse(from, m, QStringLiteral("NOT_HOST"), QStringLiteral("may not host"), false);
        return;
    }
    const qint64 epoch = qint64(m.value(QStringLiteral("epoch")).toDouble());
    const qint64 seq = qint64(m.value(QStringLiteral("seq")).toDouble());
    const QString hash = m.value(QStringLiteral("stateHash")).toString();
    qint64 maxVoted = 0;
    for (auto it = rec.votes.constBegin(); it != rec.votes.constEnd(); ++it)
        maxVoted = qMax(maxVoted, it.key());
    if (epoch <= A.floorEpoch()) {
        deny("WRONG_EPOCH");
        return;
    }
    if (rec.votes.contains(epoch)) {
        if (rec.votes.value(epoch) != from) {
            deny("ALREADY_VOTED");
            return;
        }
    } else if (epoch < maxVoted) {
        deny("WRONG_EPOCH");
        return;
    }
    const qint64 now = A.m_cfg.clock->now();
    if (rec.state != AuthState::Follower || now - A.m_lastHostContact < kFailoverMs - kVoteSlackMs) {
        deny("HOST_ALIVE");
        return;
    }
    HostingBackend *b = A.m_cfg.backend;
    if (seq < b->seq() || (seq == b->seq() && hash != b->stateHash())) {
        deny("BEHIND");
        return;
    }
    // a candidate of higher priority that we reach should be asked first
    const QStringList cands = A.m_members.hostCandidates(rec.host);
    const int mine = int(cands.indexOf(from));
    for (int i = 0; i < mine; ++i) {
        if (cands.at(i) != A.selfId() && A.online(cands.at(i))) {
            A.sendTo(from, proto::replyTo(m, proto::T::VOTE, {{QStringLiteral("epoch"), double(epoch)}, {QStringLiteral("candidate"), from},
                                                              {QStringLiteral("voter"), A.selfId()}, {QStringLiteral("granted"), false},
                                                              {QStringLiteral("code"), QStringLiteral("HIGHER_PRIORITY")}}));
            return;
        }
    }
    // the vote is kept before it is given
    const bool had = rec.votes.contains(epoch);
    const qint64 oldHighest = rec.highest;
    rec.votes.insert(epoch, from);
    rec.highest = qMax(rec.highest, epoch);
    while (rec.votes.size() > kKeepVotes) {
        qint64 lowest = rec.votes.constBegin().key();
        for (auto it = rec.votes.constBegin(); it != rec.votes.constEnd(); ++it)
            lowest = qMin(lowest, it.key());
        rec.votes.remove(lowest);
    }
    if (!A.persist()) {
        if (!had)
            rec.votes.remove(epoch);
        rec.highest = oldHighest;
        return;
    }
    if (A.killed("vote-persisted"))
        return;
    const QByteArray sig = A.m_cfg.identity->sign(voteBytes(A.m_cfg.project, epoch, from, seq, hash, A.selfId()));
    A.sendTo(from, proto::replyTo(m, proto::T::VOTE, {{QStringLiteral("epoch"), double(epoch)},
                                                      {QStringLiteral("candidate"), from},
                                                      {QStringLiteral("voter"), A.selfId()},
                                                      {QStringLiteral("granted"), true},
                                                      {QStringLiteral("sig"), QString::fromLatin1(sig.toBase64())}}));
}

} // namespace collab
} // namespace qb
