#include "core/collab/Clock.h"

#include <QDateTime>

namespace qb {
namespace collab {

SystemClock::SystemClock(QObject *parent) : Clock(parent)
{
    m_elapsed.start();
    m_timer.setInterval(250);
    connect(&m_timer, &QTimer::timeout, this, &Clock::ticked);
    m_timer.start();
}

qint64 SystemClock::wallMs() const { return QDateTime::currentMSecsSinceEpoch(); }

void ManualClock::advance(qint64 ms, qint64 step)
{
    while (ms > 0) {
        const qint64 s = qMin(ms, step);
        m_now += s;
        ms -= s;
        emit ticked();
    }
}

} // namespace collab
} // namespace qb
