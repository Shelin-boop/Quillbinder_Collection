#pragma once
// Pairing two devices (0.99.0): the writer opens a window on the host, the other device connects, both show a six-digit code
// and the writer compares them. The code comes from a commit-and-reveal exchange inside the TLS connection:
//   H = host, C = other device; fpH and fpC are the pins each side saw; nH and nC are random 32-byte nonces
//   1. H sends PAIR_COMMIT {c = SHA-256("QB-LS-PAIR-C1" | fpH | fpC | nH)} (and its name and instance id)
//   2. C answers PAIR_NONCE {n = nC} (and its name and instance id), only after the commitment
//   3. H sends PAIR_OPEN {n = nH}; C checks the commitment
//   4. both compute code = u64(first 8 bytes of SHA-256("QB-LS-PAIR-S1" | fpH | fpC | nH | nC)) mod 10^6
// Each nonce is fixed before the other is known, and the pins differ on the two legs of a man in the middle (he has to use
// a key of his own on each), so he cannot make both codes equal: one try succeeds with a chance of 1 in 10^6.
// 5. both writers compare; each side sends PAIR_CONFIRM {ok}; only when both said yes do both store the pin
// (PeerStore) and close; one "no" stores nothing, and on the host it closes the window.
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/net/Link.h"
#include "core/collab/net/PeerStore.h"

#include <QObject>
#include <QTimer>
#include <functional>

namespace qb {
namespace collab {
namespace net {

QByteArray pairingCommitment(const QByteArray &pinHost, const QByteArray &pinGuest, const QByteArray &nonceHost);
// "123 456"
QString pairingCode(const QByteArray &pinHost, const QByteArray &pinGuest, const QByteArray &nonceHost, const QByteArray &nonceGuest);

// the host's pairing window: 120 s, one attempt at a time, 3 attempts per window, a mismatch closes it, 5 windows per 10 minutes
class PairingWindow : public QObject {
    Q_OBJECT
public:
    static constexpr qint64 kWindowMs = 120000;
    static constexpr int kMaxAttempts = 3;
    static constexpr int kMaxWindows = 5;
    static constexpr qint64 kWindowsSpanMs = 10 * 60 * 1000;
    enum class Attempt { Ok, Closed, Busy, Limit };

    explicit PairingWindow(std::function<qint64()> nowMs = nullptr, QObject *parent = nullptr);
    // false (with *why: "RATE") when five windows were opened in the last ten minutes; an open window stays open
    bool open(QString *why = nullptr);
    void close();
    bool isOpen();
    qint64 remainingMs();
    // a pairing exchange starts / ends (mismatch: the writer said the codes differ, or the commitment failed)
    Attempt beginAttempt();
    void endAttempt(bool mismatch);
    int attemptsUsed() const { return used; }

signals:
    void opened();
    void closed();

private:
    qint64 now() const;
    std::function<qint64()> clock;
    QTimer timer;
    bool isOpened = false;
    qint64 openedAt = 0;
    int used = 0;
    bool busy = false;
    QList<qint64> openings;
};

// one pairing exchange on a link (the link must have been taken for pairing: its trust is Pairing)
class PairingSession : public QObject {
    Q_OBJECT
public:
    enum class Role { Host, Guest };
    // host: `window` is the open window (the session asks it for the attempt); guest: window = nullptr
    PairingSession(Role role, Link *link, DeviceIdentity *self, PeerStore *peers, PairingWindow *window = nullptr,
                   QObject *parent = nullptr);
    ~PairingSession() override;

    // the code to compare, once codeReady() came
    QString code() const { return shownCode; }
    // the writer's answer to "Codes Match" (true) or "They Differ" (false); after codeReady()
    void confirm(bool match);
    // the name and instance id the other side told (known from the commitment or the nonce on)
    QString peerName() const { return otherName; }

signals:
    void codeReady(const QString &code);
    // done: both said yes and the pin is stored; otherwise `reason` is MISMATCH (one said no), COMMITMENT (the reveal did
    // not fit the commitment), PAIRING_CLOSED, PAIRING_BUSY, PAIRING_LIMIT or LINK (the link closed too early)
    void finished(bool paired, const QString &reason);

private:
    void onFrame(const Frame &f);
    void start();
    void end(bool ok, const QString &reason);
    void abort(const QString &reason, bool mismatch = false);
    void proceed();
    static bool sane(const QString &name);

    Role role;
    Link *link;
    DeviceIdentity *self;
    PeerStore *peers;
    PairingWindow *window;
    QByteArray nonceMine, nonceTheirs, commitment;
    QString otherName, otherInstance;
    QString shownCode;
    bool decided = false, mine = false, theirsKnown = false, theirs = false, done = false, attempting = false;
    int step = 0;
};

} // namespace net
} // namespace collab
} // namespace qb
