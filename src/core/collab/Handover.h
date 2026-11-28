#pragma once
// The planned handover of hosting (0.99.0), section 6.5 of the design ("Transfer Hosting..." and the handback to the highest-
// priority device). Old host: ACTIVE -> DRAINING -> PREPARED -> RETIRED; target: FOLLOWER -> PREPARED -> ACTIVE. Every state is
// written to `livesync/authority.json` before the message that depends on it goes out. The old host stores RETIRED together with
// its signed proof before it sends HANDOVER_COMMIT and never hosts that epoch again; until then the handover can be aborted. A lost
// DONE makes the old host resend the same proof; the target answers DONE again and never starts a second time.
#include "core/collab/Authority.h"

#include <QJsonArray>
#include <QJsonObject>

namespace qb {
namespace collab {

class Handover {
public:
    explicit Handover(Authority &authority) : A(authority) {}
    // old host
    bool start(const QString &target, QString *why = nullptr);
    bool running() const;
    void abort(const QString &reason, bool tellTarget);
    void tick();
    void reset();
    void onReady(const QString &from, const QJsonObject &m);
    void onDone(const QString &from, const QJsonObject &m);
    // target
    void onPrepare(const QString &from, const QJsonObject &m);
    void onCommit(const QString &from, const QJsonObject &m);
    void onAbort(const QString &from, const QJsonObject &m);

private:
    void enterPrepared();
    void tryPrepare();
    void sendCommit();
    void abortTarget(const QString &to, qint64 epoch, const QString &reason);

    Authority &A;
    qint64 m_drainStart = 0, m_preparedAt = 0, m_lastResend = -1000000;
    QJsonArray m_locks;
    // target
    struct Pending {
        QString from, hash, msgId;
        qint64 epoch = 0, seq = 0, since = 0;
        QJsonArray locks;
    } m_pend;
    bool m_hasPend = false;
    qint64 m_prepSince = 0;
};

} // namespace collab
} // namespace qb
