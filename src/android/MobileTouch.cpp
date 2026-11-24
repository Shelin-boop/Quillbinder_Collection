#include "android/Mobile.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QElapsedTimer>
#include <QFontMetrics>
#include <QHeaderView>
#include <QLineF>
#include <QLoggingCategory>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPointingDevice>
#include <QScrollBar>
#include <QSplitter>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>
#include <QTouchEvent>
#include <QWheelEvent>
#include <cmath>

// How a touch reaches the program (Qt 6.12, Widgets): Qt sends the touch to the window; no widget of the program takes
// touches, so Qt then makes a mouse event of the first finger (press, moves, release with the left button; the event
// names the touch screen as its device). Without this file a finger is a mouse: a drag over a text selects it and
// nothing scrolls.
//
// This filter stands before all widgets and looks at those mouse events of a finger only (a mouse, a touch pad and
// a pen stay as they are):
//  - A press inside something that scrolls waits. If the finger moves away at once, the view under it scrolls with
//    the finger and the widgets never hear of the press (no cursor set, nothing selected, no keyboard). If the finger
//    goes up where it came down, the widget gets the press and the release: a tap. If the finger rests for 300 ms,
//    the widget gets the press, and what follows is a mouse drag (it selects text, moves a card).
//  - Two fingers: the touches themselves are watched (the filter sees them on their way to the window); their
//    distance becomes Ctrl + wheel steps for the widget between them, which is what the program's zoom listens to.
//  - A press close to the line between two panes (a splitter's handle, a few pixels wide) waits as well: a finger
//    that then moves across the line takes the line along (the panes change their sizes); along the line, or with a
//    tap, the touch is for what is under the finger, as everywhere else.
//  - The right mouse button (a tablet has none): a finger that rests half a second without moving and then goes up
//    asks for the menu of what is under it, as a right click does (a document in the binder, a card, a toolbar).
//    Not in a text that can be typed in: there Android's own long press selects the word (with the handles), and
//    a tap on selected text opens the text's menu (the spelling's suggestions, the ways to paste, the styles).
//    Buttons, sliders and the like keep their own press.
// Android's own long press (a word selected, with the handles) does not come through here and works as before.

namespace qb {
namespace mobile {

namespace {

// one line per touch in the system's log (Android: logcat), to see what a finger's touch was taken for
Q_LOGGING_CATEGORY(lcFingers, "qb.fingers", QtInfoMsg)

constexpr int kHoldMs = 300;         // a finger that rests this long has pressed
constexpr int kSlop = 12;            // a finger that moved this far before that swipes (the window's pixels)
constexpr double kFlingFrom = 250;   // px/s at the release: slower, and the view stops with the finger
constexpr double kFlingMost = 7000;  // px/s
constexpr double kFlingEnd = 40;     // px/s: the throw is over
constexpr double kFlingTime = 0.35;  // s in which the speed of a throw falls to about a third
constexpr double kZoomDouble = 7;    // steps of the zoom (10 % each) for fingers twice as far apart; half as far: as many back
constexpr int kDoubleTap = 30;       // px between the two taps of a double tap, at most
constexpr int kRestMs = 40;          // a finger that stood still this long before it went up throws nothing
constexpr int kDividerReach = 18;    // px on either side of the line between two panes where a finger still takes it
constexpr int kMenuMs = 500;         // a finger that rested this long without moving asks for the menu when it goes up

bool fromFinger(const QMouseEvent *me)
{
    const QPointingDevice *d = me->pointingDevice();
    return d && d->type() == QInputDevice::DeviceType::TouchScreen;
}

bool hasRange(const QScrollBar *bar) { return bar && bar->maximum() > bar->minimum(); }

// is w the area's viewport or something on it? (not its scroll bars, not its header)
bool holds(const QAbstractScrollArea *area, const QWidget *w)
{
    const QWidget *vp = area->viewport();
    return vp && (vp == w || vp->isAncestorOf(w));
}

// In a text, a list or a tree a swipe never does what a mouse drag does there (select, take a line along): when
// nothing scrolls the way the finger goes, it does nothing. A finger that rests first drags.
bool inText(const QWidget *w)
{
    for (const QWidget *x = w; x; x = x->parentWidget()) {
        if (qobject_cast<const QTextEdit *>(x) || qobject_cast<const QPlainTextEdit *>(x)
            || qobject_cast<const QAbstractItemView *>(x))
            return true;
        if (x->isWindow())
            break;
    }
    return false;
}

// how many pixels one step of a scroll bar is
double unitOf(QAbstractScrollArea *area, Qt::Orientation o)
{
    if (o == Qt::Vertical && qobject_cast<QPlainTextEdit *>(area))
        return qMax(1, QFontMetrics(area->font()).lineSpacing()); // (it scrolls by lines)
    if (auto *view = qobject_cast<QAbstractItemView *>(area)) {
        const bool perItem = (o == Qt::Vertical ? view->verticalScrollMode() : view->horizontalScrollMode())
                             == QAbstractItemView::ScrollPerItem;
        if (perItem)
            return o == Qt::Vertical ? qMax(16, view->sizeHintForRow(0)) : qMax(40, view->sizeHintForColumn(0));
    }
    return 1;
}

// A text that can be typed in (it, or the scroll area whose viewport was touched, takes the keyboard's input):
// a long press there is Android's own (the word selected, with its handles).
bool typedInto(const QWidget *w)
{
    // (a list or tree also takes the keyboard's input while its current line can be renamed: that is no text)
    for (int level = 0; w && level < 2; ++level, w = w->parentWidget()) {
        if (w->testAttribute(Qt::WA_InputMethodEnabled) && !qobject_cast<const QAbstractItemView *>(w))
            return true;
    }
    return false;
}

// May a finger that rested and went up open the menu of this widget? Not where a press means something of its own.
bool takesMenu(const QWidget *w)
{
    if (!w || typedInto(w))
        return false;
    for (const QWidget *x = w; x; x = x->parentWidget()) {
        if (qobject_cast<const QAbstractButton *>(x) || qobject_cast<const QComboBox *>(x)
            || qobject_cast<const QAbstractSlider *>(x) || qobject_cast<const QAbstractSpinBox *>(x)
            || qobject_cast<const QMenu *>(x) || qobject_cast<const QMenuBar *>(x)
            || qobject_cast<const QSplitterHandle *>(x))
            return false;
        if (x->isWindow() || qobject_cast<const QAbstractScrollArea *>(x))
            break;
    }
    return true;
}

// Is p (in the viewport's coordinates) on the selected part of the text edit whose viewport was touched?
template <class Edit> bool onSelectionOf(const Edit *edit, const QWidget *viewport, const QPoint &p)
{
    if (!edit || edit->viewport() != viewport)
        return false;
    const QTextCursor selected = edit->textCursor();
    if (!selected.hasSelection())
        return false;
    const QTextCursor at = edit->cursorForPosition(p);
    if (at.position() < selected.selectionStart() || at.position() > selected.selectionEnd())
        return false;
    // (the nearest place in the text may be far from the finger: beside a short line, below the last one)
    return edit->cursorRect(at).adjusted(-24, -6, 24, 6).contains(p);
}

class Fingers : public QObject
{
public:
    explicit Fingers(QObject *parent)
        : QObject(parent)
    {
        m_hold.setSingleShot(true);
        m_hold.setInterval(kHoldMs);
        // The finger rests: from here on it is a mouse. Not at once when the time is over: the program may have been
        // busy meanwhile, and moves of the finger may wait to be handled (timers come before input in Qt's loop;
        // on the emulated tablet a swipe was taken for a rest that way, run 12). One more turn of the loop handles
        // them first; a move that is a swipe by its own time then decides.
        m_holdCheck.setSingleShot(true);
        m_holdCheck.setInterval(20);
        connect(&m_hold, &QTimer::timeout, &m_holdCheck, qOverload<>(&QTimer::start));
        connect(&m_holdCheck, &QTimer::timeout, this, [this] {
            if (m_mode != Mode::Pending)
                return;
            if (m_divider) { // (beside a line between panes: the finger's first move says what the touch is for)
                m_rested = true;
                return;
            }
            m_mode = Mode::Pass;
            qCInfo(lcFingers, "fingers: rested %lld ms, pressed", static_cast<long long>(m_clock.elapsed() - m_pressWall));
            deliverPress();
        });
        m_fling.setInterval(16);
        connect(&m_fling, &QTimer::timeout, this, [this] { flingStep(); });
        m_clock.start();
    }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
            if (o->isWindowType())
                watchTouch(static_cast<QTouchEvent *>(e));
            return false;
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::MouseButtonRelease: {
            if (m_delivering || !o->isWidgetType())
                return false;
            auto *me = static_cast<QMouseEvent *>(e);
            return fromFinger(me) && mouse(static_cast<QWidget *>(o), me);
        }
        case QEvent::Show:
            // lists and trees scroll by pixels (Qt's standard is a line at a time, which jumps under a finger)
            if (auto *view = qobject_cast<QAbstractItemView *>(o); view && !qobject_cast<QHeaderView *>(view)) {
                if (view->verticalScrollMode() != QAbstractItemView::ScrollPerPixel)
                    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                if (view->horizontalScrollMode() != QAbstractItemView::ScrollPerPixel)
                    view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            }
            return false;
        default:
            return false;
        }
    }

private:
    enum class Mode {
        Idle,
        Pending, // the press waits: a swipe, a tap or a rest?
        Scroll,  // the view follows the finger
        Swallow, // a swipe over a text or list that cannot scroll that way: nothing
        Pass,    // as a mouse
        Divider, // the line between two panes follows the finger
        Pinch    // two fingers
    };
    struct Axis
    {
        QPointer<QScrollBar> bar;
        double unit = 1;  // pixels per step of the bar
        double value = 0; // where the bar stands, with the part of a step
        double speed = 0; // steps of pixels per second while thrown
    };

    // ---- one finger

    // a press here waits when a swipe could scroll something, or when it is in a text, a list or a tree (where a
    // swipe never selects or drags)
    bool waits(const QWidget *w) const
    {
        bool inside = false;
        for (const QWidget *x = w; x; x = x->parentWidget()) {
            if (auto *area = qobject_cast<const QAbstractScrollArea *>(x); area && holds(area, w)) {
                inside = true;
                if (hasRange(area->verticalScrollBar()) || hasRange(area->horizontalScrollBar()))
                    return true;
            }
            if (x->isWindow())
                break;
        }
        return inside && inText(w);
    }

    // when the touch happened (the event's own time; the program may get to it later)
    qint64 timeOf(const QMouseEvent *me) const { return me->timestamp() ? qint64(me->timestamp()) : m_clock.elapsed(); }

    void stopHold()
    {
        m_hold.stop();
        m_holdCheck.stop();
    }

    bool mouse(QWidget *w, QMouseEvent *me)
    {
        const QEvent::Type t = me->type();
        const bool press = t == QEvent::MouseButtonPress || t == QEvent::MouseButtonDblClick;
        if (m_mode == Mode::Pinch) {
            if (!press || m_down > 1)
                return true; // (two fingers: none of it is a click or a drag, also what Qt makes of the last finger)
            m_mode = Mode::Idle;
        }
        if (press) {
            stopHold();
            const bool thrown = stopFling();
            // The second tap of a double tap goes on as it is. Qt takes every press of a finger within 400 ms of
            // the one before for it, wherever it is (it compares the place with the new touch's own): only one
            // close to a tap that reached a widget is the second tap; after a swipe, or elsewhere, it is a first.
            const QPointF away = me->globalPosition() - m_clickAt;
            const bool second = t == QEvent::MouseButtonDblClick && m_clicked && qMax(qAbs(away.x()), qAbs(away.y())) <= kDoubleTap;
            m_mode = Mode::Pass;
            m_downWidget = nullptr;
            if (second)
                return false;
            m_clickAt = me->globalPosition();
            if (me->button() != Qt::LeftButton)
                return false;
            // (for the menu of a finger that rests and goes up)
            m_downWidget = w;
            m_downLocal = me->position();
            m_downGlobal = me->globalPosition();
            m_downAt = timeOf(me);
            m_strayed = false;
            m_mods = me->modifiers();
            m_divider = dividerNear(me->globalPosition().toPoint(), w);
            m_rested = false;
            m_plain = !waits(w); // (it waits only because of the line beside it)
            if (!m_divider && m_plain) {
                m_clicked = true;
                return false;
            }
            m_widget = w;
            m_local = me->position();
            m_scene = me->scenePosition();
            m_global = me->globalPosition();
            m_mods = me->modifiers();
            m_device = me->pointingDevice();
            m_stamp = me->timestamp();
            m_pressAt = timeOf(me);
            m_pressWall = m_clock.elapsed();
            m_noTap = thrown; // (a finger put on a moving view stops it and chooses nothing)
            m_mode = Mode::Pending;
            m_hold.start();
            return true;
        }
        if (t == QEvent::MouseMove) {
            if (!m_strayed) {
                const QPointF from = me->globalPosition() - m_downGlobal;
                m_strayed = qMax(qAbs(from.x()), qAbs(from.y())) >= kSlop;
            }
            switch (m_mode) {
            case Mode::Pending: {
                const QPointF d = me->globalPosition() - m_global;
                if (qMax(qAbs(d.x()), qAbs(d.y())) < kSlop)
                    return true;
                stopHold();
                if (m_divider && across(m_divider, d)) {
                    m_mode = Mode::Divider;
                    m_clicked = false;
                    m_dividerFrom = dividerPlace();
                    toDivider(QEvent::MouseButtonPress, m_global, Qt::LeftButton, Qt::LeftButton);
                    toDivider(QEvent::MouseMove, me->globalPosition(), Qt::NoButton, Qt::LeftButton);
                    return true;
                }
                if (m_plain) { // not across the line, and nothing here waits otherwise: as a mouse (a scroll bar, a button)
                    m_mode = Mode::Pass;
                    deliverPress();
                    return false;
                }
                if (m_rested || timeOf(me) - m_pressAt > kHoldMs) { // it rested first (the program only got to it now)
                    m_mode = Mode::Pass;
                    qCInfo(lcFingers, "fingers: rested %lld ms, then moved: a drag", static_cast<long long>(timeOf(me) - m_pressAt));
                    deliverPress();
                    return false;
                }
                if (beginScroll(d, me->globalPosition(), timeOf(me))) {
                    m_mode = Mode::Scroll;
                    m_clicked = false;
                    return true;
                }
                if (m_widget && inText(m_widget)) {
                    m_mode = Mode::Swallow;
                    m_clicked = false;
                    qCInfo(lcFingers, "fingers: a swipe over a text or list that does not scroll that way: nothing");
                    return true;
                }
                m_mode = Mode::Pass; // nothing scrolls that way: a mouse drag (a slider, a card)
                qCInfo(lcFingers, "fingers: nothing scrolls that way: a drag");
                deliverPress();
                return false;
            }
            case Mode::Scroll:
                scrollTo(me->globalPosition(), timeOf(me));
                return true;
            case Mode::Divider:
                toDivider(QEvent::MouseMove, me->globalPosition(), Qt::NoButton, Qt::LeftButton);
                return true;
            case Mode::Swallow:
                return true;
            default:
                return false;
            }
        }
        // the release
        const Mode was = m_mode;
        m_mode = Mode::Idle;
        stopHold();
        // rested without moving, then up: the menu of what is under the finger (a right click)
        const bool held = m_downWidget && !m_strayed && timeOf(me) - m_downAt >= kMenuMs && takesMenu(m_downWidget);
        switch (was) {
        case Mode::Pending:
            if (m_noTap) {
                m_clicked = false;
                qCInfo(lcFingers, "fingers: a touch that stopped the running view");
                return true;
            }
            if (onSelection()) { // a tap on selected text: the text's menu; the selection stays
                m_clicked = false;
                qCInfo(lcFingers, "fingers: tap on selected text: its menu");
                menuLater(m_widget, m_local, m_global);
                return true;
            }
            qCInfo(lcFingers, "fingers: tap after %lld ms (handled %lld ms after the press)",
                   static_cast<long long>(timeOf(me) - m_pressAt), static_cast<long long>(m_clock.elapsed() - m_pressWall));
            deliverPress(); // a tap: the press that waited, then this release
            if (held) // (it rested beside a line between panes and never moved)
                menuAfterHold(timeOf(me));
            return false;
        case Mode::Scroll:
            if (me->globalPosition() != m_last)
                scrollTo(me->globalPosition(), timeOf(me));
            throwOn(timeOf(me));
            qCInfo(lcFingers, "fingers: swipe, scrolled %d x %d px in %lld ms, thrown on at %d x %d px/s", qRound(m_scrolled.x()),
                   qRound(m_scrolled.y()), static_cast<long long>(timeOf(me) - m_pressAt), qRound(m_h.speed), qRound(m_v.speed));
            return true;
        case Mode::Divider:
            endDivider(me->globalPosition());
            return true;
        case Mode::Swallow:
            return true;
        case Mode::Pass:
            if (held)
                menuAfterHold(timeOf(me));
            return false;
        default:
            return false;
        }
    }

    // ---- the menu of what is under the finger

    bool onSelection() const
    {
        const QWidget *w = m_widget;
        if (!w || !w->parentWidget())
            return false;
        const QPoint p = m_local.toPoint();
        return onSelectionOf(qobject_cast<const QTextEdit *>(w->parentWidget()), w, p)
               || onSelectionOf(qobject_cast<const QPlainTextEdit *>(w->parentWidget()), w, p);
    }

    void menuAfterHold(qint64 now)
    {
        qCInfo(lcFingers, "fingers: rested %lld ms without moving, then up: the menu of what is under the finger",
               static_cast<long long>(now - m_downAt));
        menuLater(m_downWidget, m_downLocal, m_downGlobal);
    }

    // The menu a right click would open there. After this release has reached its widget, and not while another
    // menu is open (a button that opened its own).
    void menuLater(QWidget *widget, const QPointF &local, const QPointF &global)
    {
        const QPointer<QWidget> w = widget;
        const Qt::KeyboardModifiers mods = m_mods;
        QTimer::singleShot(0, this, [w, local, global, mods] {
            if (!w || !w->isVisible() || QApplication::activePopupWidget())
                return;
            QContextMenuEvent menu(QContextMenuEvent::Mouse, local.toPoint(), global.toPoint(), mods);
            QApplication::sendEvent(w, &menu);
        });
    }

    // ---- the line between two panes

    // The splitter handle of the touched window that the finger is on or close beside (the nearest one).
    static QSplitterHandle *dividerNear(const QPoint &global, const QWidget *under)
    {
        const QWidget *top = under->window();
        QSplitterHandle *best = nullptr;
        int bestAway = kDividerReach + 1;
        const QList<QSplitterHandle *> all = top->findChildren<QSplitterHandle *>();
        for (QSplitterHandle *h : all) {
            if (!h->isVisible() || !h->isEnabled() || h->window() != top)
                continue;
            const QRect r(h->mapToGlobal(QPoint(0, 0)), h->size());
            const bool standing = h->orientation() == Qt::Horizontal; // (between panes side by side)
            const int along = standing ? global.y() : global.x();
            if (along < (standing ? r.top() : r.left()) || along > (standing ? r.bottom() : r.right()))
                continue;
            const int at = standing ? global.x() : global.y();
            const int from = standing ? r.left() : r.top(), to = standing ? r.right() : r.bottom();
            const int away = at < from ? from - at : (at > to ? at - to : 0);
            if (away < bestAway) {
                best = h;
                bestAway = away;
            }
        }
        return best;
    }

    // does the finger go across the line (and not along it)?
    static bool across(const QSplitterHandle *h, const QPointF &d)
    {
        return h->orientation() == Qt::Horizontal ? qAbs(d.x()) > qAbs(d.y()) : qAbs(d.y()) > qAbs(d.x());
    }

    int dividerPlace() const
    {
        if (!m_divider)
            return 0;
        const QPoint at = m_divider->mapToGlobal(QPoint(0, 0));
        return m_divider->orientation() == Qt::Horizontal ? at.x() : at.y();
    }

    // The finger's place as a mouse event for the handle (QSplitterHandle goes by the place on the screen; the
    // place inside the handle may lie beside it, which keeps the line from jumping under the finger).
    void toDivider(QEvent::Type type, const QPointF &global, Qt::MouseButton button, Qt::MouseButtons buttons)
    {
        const QPointer<QSplitterHandle> h = m_divider;
        if (!h)
            return;
        QMouseEvent event(type, h->mapFromGlobal(global), h->window()->mapFromGlobal(global), global, button, buttons, m_mods,
                          m_device ? m_device.data() : QPointingDevice::primaryPointingDevice());
        m_last = global;
        m_delivering = true;
        QApplication::sendEvent(h, &event);
        m_delivering = false;
    }

    void endDivider(const QPointF &global)
    {
        toDivider(QEvent::MouseButtonRelease, global, Qt::LeftButton, Qt::NoButton);
        qCInfo(lcFingers, "fingers: the line between two panes moved by %d px", dividerPlace() - m_dividerFrom);
        m_divider = nullptr;
    }

    // The press that waited, to the widget it was for. Qt gave the keyboard focus already when the press came.
    void deliverPress()
    {
        m_clicked = true;
        const QPointer<QWidget> w = m_widget;
        if (!w)
            return;
        QMouseEvent press(QEvent::MouseButtonPress, m_local, m_scene, m_global, Qt::LeftButton, Qt::LeftButton, m_mods,
                          m_device ? m_device.data() : QPointingDevice::primaryPointingDevice());
        press.setTimestamp(m_stamp);
        m_delivering = true;
        QApplication::sendEvent(w, &press);
        m_delivering = false;
    }

    // The nearest view around the pressed widget that can scroll the way the finger went.
    bool beginScroll(const QPointF &d, const QPointF &now, qint64 ms)
    {
        const bool vertical = qAbs(d.y()) >= qAbs(d.x());
        for (QWidget *x = m_widget; x; x = x->parentWidget()) {
            auto *area = qobject_cast<QAbstractScrollArea *>(x);
            if (area && holds(area, m_widget)
                && hasRange(vertical ? area->verticalScrollBar() : area->horizontalScrollBar())) {
                // a list goes one way at a time; a picture or a board follows the finger both ways
                const bool oneWay = qobject_cast<QAbstractItemView *>(area) != nullptr;
                m_v = Axis();
                m_h = Axis();
                if (QScrollBar *bar = area->verticalScrollBar(); hasRange(bar) && (vertical || !oneWay))
                    m_v = Axis{bar, unitOf(area, Qt::Vertical), double(bar->value()), 0};
                if (QScrollBar *bar = area->horizontalScrollBar(); hasRange(bar) && (!vertical || !oneWay))
                    m_h = Axis{bar, unitOf(area, Qt::Horizontal), double(bar->value()), 0};
                m_last = now;
                m_scrolled = QPointF();
                m_samples.clear();
                m_samples.append(Sample{ms, now});
                handlesAway(); // (the drop under the cursor does not ride along)
                return true;
            }
            if (x->isWindow())
                break;
        }
        return false;
    }

    static void moveBy(Axis &axis, double px)
    {
        if (!axis.bar)
            return;
        axis.value = qBound(double(axis.bar->minimum()), axis.value + px / axis.unit, double(axis.bar->maximum()));
        axis.bar->setValue(qRound(axis.value));
    }

    void scrollTo(const QPointF &now, qint64 ms)
    {
        const QPointF d = now - m_last;
        m_last = now;
        m_scrolled -= d;
        moveBy(m_v, -d.y());
        moveBy(m_h, -d.x());
        m_samples.append(Sample{ms, now});
        while (m_samples.size() > 2 && m_samples.last().ms - m_samples.first().ms > 150)
            m_samples.removeFirst();
    }

    // The finger went up while it moved: the view goes on and slows down. The speed is the finger's over the last
    // 100 ms it moved; a finger that stood still before it went up throws nothing.
    void throwOn(qint64 releasedAt)
    {
        m_v.speed = m_h.speed = 0;
        if (m_samples.size() < 2)
            return;
        const Sample newest = m_samples.last();
        if (releasedAt - newest.ms > kRestMs)
            return;
        Sample from = newest;
        for (const Sample &s : std::as_const(m_samples))
            if (newest.ms - s.ms <= 100) {
                from = s;
                break;
            }
        const double dt = (newest.ms - from.ms) / 1000.0;
        if (dt < 0.01)
            return;
        const QPointF v = (newest.pos - from.pos) / dt;
        m_v.speed = m_v.bar ? qBound(-kFlingMost, -v.y(), kFlingMost) : 0;
        m_h.speed = m_h.bar ? qBound(-kFlingMost, -v.x(), kFlingMost) : 0;
        if (qMax(qAbs(m_v.speed), qAbs(m_h.speed)) < kFlingFrom) {
            m_v.speed = m_h.speed = 0;
            return;
        }
        m_flingClock.start();
        m_fling.start();
    }

    void flingStep()
    {
        const double dt = qMin(0.05, m_flingClock.restart() / 1000.0);
        bool moving = false;
        for (Axis *axis : {&m_v, &m_h}) {
            if (!axis->bar || axis->speed == 0)
                continue;
            const double before = axis->value;
            moveBy(*axis, axis->speed * dt);
            axis->speed *= std::exp(-dt / kFlingTime);
            if (axis->value == before || qAbs(axis->speed) < kFlingEnd)
                axis->speed = 0; // (the end of the view, or slow enough)
            else
                moving = true;
        }
        if (!moving)
            m_fling.stop();
    }

    bool stopFling()
    {
        const bool was = m_fling.isActive();
        m_fling.stop();
        return was;
    }

    // ---- two fingers

    void watchTouch(const QTouchEvent *te)
    {
        const QPointingDevice *device = te->pointingDevice();
        if (!device || device->type() != QInputDevice::DeviceType::TouchScreen)
            return;
        if (te->type() == QEvent::TouchCancel) {
            m_down = 0;
            m_pinching = false;
            if (m_mode == Mode::Divider) { // (Android took the touch away: the line stays where it is now)
                m_mode = Mode::Idle;
                endDivider(m_last);
            }
            return;
        }
        QList<const QEventPoint *> down;
        for (const QEventPoint &p : te->points())
            if (p.state() != QEventPoint::Released)
                down << &p;
        m_down = down.size();
        if (m_mode == Mode::Pinch) {
            if (te->type() == QEvent::TouchBegin && m_down < 2) {
                m_mode = Mode::Idle; // a new touch with one finger
                return;
            }
            const QEventPoint *a = nullptr, *b = nullptr;
            for (const QEventPoint *p : std::as_const(down)) {
                if (p->id() == m_a)
                    a = p;
                else if (p->id() == m_b)
                    b = p;
            }
            if (!a || !b) {
                if (m_pinching)
                    qCInfo(lcFingers, "fingers: two fingers, %d steps of zoom", m_sent);
                m_pinching = false; // one of the two went up: over (what the other does until it goes up is nothing)
            } else if (m_pinching)
                pinchTo(QLineF(a->globalPosition(), b->globalPosition()).length());
            return;
        }
        // a second finger beside one that swipes or still waits; not beside one that drags as a mouse or moves a line
        if (m_down >= 2 && m_mode != Mode::Pass && m_mode != Mode::Divider)
            beginPinch(*down[0], *down[1]);
    }

    void beginPinch(const QEventPoint &a, const QEventPoint &b)
    {
        stopHold();
        stopFling();
        handlesAway();
        m_mode = Mode::Pinch;
        m_pinching = true;
        m_clicked = false;
        m_a = a.id();
        m_b = b.id();
        m_span = qMax(24.0, QLineF(a.globalPosition(), b.globalPosition()).length());
        m_sent = 0;
        m_center = (a.globalPosition() + b.globalPosition()) / 2;
        m_target = QApplication::widgetAt(m_center.toPoint());
    }

    // The fingers' distance against the one they began with, in steps of the zoom; each step is one notch of
    // Ctrl + wheel for the widget that was between the fingers (ui/Zoom.cpp; a mind map board zooms itself). Counted
    // by the ratio, so that fingers moved together take back exactly what the same fingers moved apart gave.
    void pinchTo(double span)
    {
        const int want = qRound(kZoomDouble * std::log2(qMax(span, 1.0) / m_span));
        const int steps = want - m_sent;
        if (steps == 0 || !m_target)
            return;
        m_sent = want;
        const QPointer<QWidget> target = m_target;
        QWheelEvent wheel(target->mapFromGlobal(m_center), m_center, QPoint(), QPoint(0, 120 * steps), Qt::NoButton,
                          Qt::ControlModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(target, &wheel);
    }

    struct Sample
    {
        qint64 ms;
        QPointF pos;
    };

    Mode m_mode = Mode::Idle;
    bool m_delivering = false;
    bool m_clicked = false; // the last touch reached a widget as a click (so a double tap may follow)
    bool m_noTap = false;
    // the finger's press, whatever became of it (for the menu of a finger that rests and goes up)
    QPointer<QWidget> m_downWidget;
    QPointF m_downLocal, m_downGlobal;
    qint64 m_downAt = 0;
    bool m_strayed = false; // it moved away from where it came down
    QTimer m_hold, m_holdCheck;
    qint64 m_pressAt = 0;   // when the finger came down (the event's time)
    qint64 m_pressWall = 0; // when the program handled that (m_clock)
    // the press that waits
    QPointer<QWidget> m_widget;
    QPointF m_local, m_scene, m_global;
    QPointF m_clickAt; // where the last press of a finger was
    Qt::KeyboardModifiers m_mods;
    QPointer<const QPointingDevice> m_device;
    quint64 m_stamp = 0;
    // the line between two panes the press is close to (none: nullptr)
    QPointer<QSplitterHandle> m_divider;
    bool m_rested = false; // the finger beside such a line rested before it moved
    bool m_plain = false;  // the pressed widget would have got the press at once, were it not for the line
    int m_dividerFrom = 0; // where the line stood (px on the screen, across it), for the log
    // the scroll
    Axis m_v, m_h;
    QPointF m_last;
    QPointF m_scrolled; // how far this swipe has scrolled (px), for the log
    QList<Sample> m_samples;
    QElapsedTimer m_clock, m_flingClock;
    QTimer m_fling;
    // the touches
    int m_down = 0;
    bool m_pinching = false;
    int m_a = -1, m_b = -1;
    double m_span = 1;
    int m_sent = 0;
    QPointF m_center;
    QPointer<QWidget> m_target;
};

} // namespace

void installTouch()
{
    if (!qApp)
        return;
    const QString name = QStringLiteral("mobileFingers");
    QObject *fingers = qApp->findChild<QObject *>(name, Qt::FindDirectChildrenOnly);
    if (!fingers) {
        fingers = new Fingers(qApp);
        fingers->setObjectName(name);
        installHandles(); // the cursor's and the selection's handles (android/MobileHandles.cpp)
    }
    // the filter installed last is asked first: a later call puts this one before the filters made since
    qApp->removeEventFilter(fingers);
    qApp->installEventFilter(fingers);
}

} // namespace mobile
} // namespace qb
