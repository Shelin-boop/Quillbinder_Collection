#pragma once
// Failover (0.99.0), section 6.4 of the design: when the host has been out of contact for about 30 seconds, the next device by
// priority that may host takes over. Two devices: the other one alone. Three or more: it needs the signed votes of more than half
// of all the project's devices, itself counted; a voter says yes only when it too has lost the host, has not voted for that
// epoch for someone else (the vote is written before the answer), the candidate holds at least as many commits as it does, and
// it reaches no candidate of higher priority. A failed round is repeated with a higher epoch, so a vote given to a candidate
// that never won cannot block the project.
#include "core/collab/Authority.h"

#include <QJsonObject>
#include <QSet>

namespace qb {
namespace collab {

class Election {
public:
    explicit Election(Authority &authority) : A(authority) {}
    void tick();
    void onRequest(const QString &from, const QJsonObject &m);
    void onVote(const QString &from, const QJsonObject &m);
    void cancel() { m_active = false; }
    bool candidateActive() const { return m_active; }

private:
    void beginRound(qint64 epoch);
    void becomeHost(qint64 epoch, const QList<Vote> &votes, qint64 baseSeq, const QString &baseHash);
    qint64 nextEpoch() const;

    Authority &A;
    bool m_active = false;
    qint64 m_epoch = 0, m_started = 0, m_nextAllowed = 0, m_baseSeq = 0;
    QString m_baseHash;
    QList<Vote> m_votes;
    QSet<QString> m_voters;
};

} // namespace collab
} // namespace qb
