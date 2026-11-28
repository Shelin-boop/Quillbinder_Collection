#pragma once
// Live Sync's clock (0.99.0). Everything that waits (leases, the time of an edit request, heartbeats) asks a Clock, never
// the system, so the tests can move time by hand. now() is monotonic milliseconds and is only ever compared with itself
// (order of events comes from sequence numbers, never from clocks); wallMs() is the calendar time and is used only to
// decide how long old commits are kept. The sessions listen to ticked() and look at their deadlines then.
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

namespace qb {
namespace collab {

class Clock : public QObject {
    Q_OBJECT
public:
    explicit Clock(QObject *parent = nullptr) : QObject(parent) {}
    virtual qint64 now() const = 0;    // monotonic, milliseconds
    virtual qint64 wallMs() const = 0; // milliseconds since 1970-01-01 UTC
signals:
    void ticked();
};

// the real one: ticks every 250 ms while someone listens
class SystemClock : public Clock {
    Q_OBJECT
public:
    explicit SystemClock(QObject *parent = nullptr);
    qint64 now() const override { return m_elapsed.elapsed(); }
    qint64 wallMs() const override;

private:
    QElapsedTimer m_elapsed;
    QTimer m_timer;
};

// the tests': time stands still until advance()
class ManualClock : public Clock {
    Q_OBJECT
public:
    explicit ManualClock(QObject *parent = nullptr) : Clock(parent) {}
    qint64 now() const override { return m_now; }
    qint64 wallMs() const override { return m_wall + m_now; }
    // moves time forward in steps of at most `step` ms, ticking after each (so that deadlines fire in their order)
    void advance(qint64 ms, qint64 step = 250);
    void setWallBase(qint64 wallAtZero) { m_wall = wallAtZero; }

private:
    qint64 m_now = 0;
    qint64 m_wall = 1700000000000LL;
};

} // namespace collab
} // namespace qb
