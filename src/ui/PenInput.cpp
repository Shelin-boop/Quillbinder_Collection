#include "ui/PenInput.h"

#include <QElapsedTimer>
#include <QTabletEvent>

namespace qb {

PenInput &PenInput::instance()
{
    static PenInput pen;
    return pen;
}

PenInput::PenInput(QObject *parent) : QObject(parent) {}

qint64 PenInput::defaultClock()
{
    static QElapsedTimer timer = [] {
        QElapsedTimer t;
        t.start();
        return t;
    }();
    return timer.elapsed();
}

void PenInput::feed(const Sample &sample)
{
    const bool wasOn = m_sample.tipDown || m_sample.inRange;
    const bool wasDown = m_sample.tipDown;
    m_sample = sample;
    const qint64 t = now();
    if (sample.inRange || sample.tipDown)
        m_lastSeen = t;
    if ((wasDown && !sample.tipDown) || (wasOn && !sample.inRange && !sample.tipDown))
        m_lastLifted = m_lastSeen = t;
    emit changed();
}

void PenInput::feedTablet(const QTabletEvent *e)
{
    Sample s;
    switch (e->type()) {
    case QEvent::TabletPress:
    case QEvent::TabletMove:
        s.inRange = true;
        s.tipDown = e->type() == QEvent::TabletPress || (e->buttons() & Qt::LeftButton);
        break;
    case QEvent::TabletRelease:
        s.inRange = true;
        s.tipDown = false;
        break;
    case QEvent::TabletEnterProximity:
        s.inRange = true;
        break;
    default: // leaving the range
        break;
    }
    s.sideButton = (e->buttons() & (Qt::RightButton | Qt::MiddleButton)) != 0;
    s.eraser = e->pointerType() == QPointingDevice::PointerType::Eraser;
    s.pressure = e->pressure();
    feed(s);
}

bool PenInput::inUse(qint64 withinMs) const
{
    if (m_sample.inRange || m_sample.tipDown)
        return true;
    return m_lastSeen >= 0 && now() - m_lastSeen <= withinMs;
}

bool PenInput::wroteWithin(qint64 withinMs) const
{
    if (m_sample.tipDown)
        return true;
    return m_lastLifted >= 0 && now() - m_lastLifted <= withinMs;
}

void PenInput::reset()
{
    m_sample = Sample();
    m_lastSeen = m_lastLifted = -1;
}

} // namespace qb
