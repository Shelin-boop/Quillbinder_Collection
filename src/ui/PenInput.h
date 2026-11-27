#pragma once
// 0.99.0 (handwriting, item 112): what the program knows about the pen. A small interface: on Linux and Windows the
// writing surface feeds it from Qt's tablet events (feedTablet); Android's own code feeds it later through feed() with
// what Android's stylus events say (the barrel button, the eraser end, the hover). Everything that must tell a pen from
// a finger (the writing area, the rest zone of the hand protection, the choice "one finger writes or scrolls") asks
// this object, never the event classes.
#include <QObject>
#include <functional>

class QTabletEvent;

namespace qb {

class PenInput : public QObject
{
    Q_OBJECT
public:
    struct Sample {
        bool inRange = false;      // the pen is near the surface (hovering) or on it
        bool tipDown = false;      // the tip touches
        bool sideButton = false;   // the barrel button is held
        bool eraser = false;       // the eraser end is used
        qreal pressure = 0;        // 0 .. 1
    };

    static PenInput &instance();
    explicit PenInput(QObject *parent = nullptr);   // the program uses instance(); the tests may make their own

    // The one entry: Android's code calls it; so does feedTablet().
    void feed(const Sample &sample);
    // Linux and Windows: Qt's tablet event (type, buttons, pressure) becomes a Sample. The side button is the barrel
    // button Qt reports as the right or the middle mouse button.
    void feedTablet(const QTabletEvent *event);

    bool inRange() const { return m_sample.inRange; }
    bool tipDown() const { return m_sample.tipDown; }
    bool sideButton() const { return m_sample.sideButton; }
    const Sample &sample() const { return m_sample; }
    // The clock's time (milliseconds) of the last event of the pen; -1: never.
    qint64 lastSeenMs() const { return m_lastSeen; }
    // The time the tip last left the surface or the pen left the range; -1: never.
    qint64 lastLiftedMs() const { return m_lastLifted; }
    // The pen is in use: near, touching, or seen within `withinMs`.
    bool inUse(qint64 withinMs) const;
    // The pen wrote within `withinMs` (tip down now, or up less than that ago).
    bool wroteWithin(qint64 withinMs) const;
    qint64 now() const { return m_clock ? m_clock() : defaultClock(); }
    void setClock(std::function<qint64()> clock) { m_clock = std::move(clock); }   // the tests
    void reset();

signals:
    void changed();

private:
    static qint64 defaultClock();
    Sample m_sample;
    qint64 m_lastSeen = -1;
    qint64 m_lastLifted = -1;
    std::function<qint64()> m_clock;
};

} // namespace qb
