#include "android/Mobile.h"

#include <QApplication>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QInputMethodQueryEvent>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QMouseEvent>
#include <QPointingDevice>
#include <QString>
#include <QTimer>
#include <QTransform>
#include <QWidget>

// Android's handles of a text: the drop under the cursor, and the two under the ends of a selection. Qt's Android
// part draws them as small windows of their own above everything, where the text widget says its cursor is
// (QAndroidInputContext::updateSelectionHandles). As Qt 6.12.0 does it, they got in the writer's way on the tablets:
//  - A text scrolled inside its pane (the inspector's bookmark preview) took the handle along, out of the pane and
//    over the panes above: Qt never asks whether the cursor can still be seen.
//  - The main text is moved as a whole inside its scroll area, and Qt learns of a widget's new place only when the
//    widget itself is moved, not what it stands in: the handle stayed where the cursor had been.
//  - The drop under the cursor came at every touch of the text (also one that only scrolls), and every movement of
//    the cursor gave it four more seconds: while typing it never left, and covered the line below.
//
// What this file does, with what Qt offers from outside (no part of Qt is replaced):
//  - It answers Qt's question for the text's input hints (QInputMethodQueryEvent, Qt::ImHints) with "no handles now"
//    (Qt::ImhNoTextHandles) while the handles should not show; Qt asks at every update of the handles and hides
//    them then, but keeps in mind what it would show. They should not show: without a selection, when the cursor
//    is out of sight or no finger tapped the text lately; with a selection, when both its ends are out of sight.
//  - The drop under the cursor is for the time after a tap into the text (kCursorHandleMs, longer while the cursor
//    is moved by its drop); typing, a swipe and two fingers send it away at once. Android's own text fields do the
//    same.
//  - When something the text stands in moves or changes its size, Qt is told the text's new place in the window
//    (QInputMethod::setInputItemTransform, as QWidget does for the text widget's own moves): the handles follow.

namespace qb {
namespace mobile {

namespace {

Q_LOGGING_CATEGORY(lcHandles, "qb.handles", QtInfoMsg)

constexpr int kCursorHandleMs = 3000; // how long the drop under the cursor stays after a tap

class Handles : public QObject
{
public:
    explicit Handles(QObject *parent)
        : QObject(parent)
    {
        m_leave.setSingleShot(true);
        m_leave.setInterval(kCursorHandleMs);
        connect(&m_leave, &QTimer::timeout, this, [this] { allowCursorHandle(false, "its time is over"); });
        m_place.setSingleShot(true);
        m_place.setInterval(0);
        connect(&m_place, &QTimer::timeout, this, [this] { tellPlace(); });
        // the cursor moved while its drop shows (the writer drags the drop): it stays that much longer
        connect(QGuiApplication::inputMethod(), &QInputMethod::cursorRectangleChanged, this, [this] {
            if (m_cursorHandle && !m_telling)
                m_leave.start();
        });
        connect(qApp, &QGuiApplication::focusObjectChanged, this, [this] {
            m_marked.clear();
            allowCursorHandle(false, nullptr);
        });
    }

    void away(const char *why) { allowCursorHandle(false, why); }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::InputMethodQuery: {
            // Qt's question to the text that has the keyboard, with the hints among what it asks for
            auto *query = static_cast<QInputMethodQueryEvent *>(e);
            if (m_asking || !(query->queries() & Qt::ImHints) || !o->isWidgetType() || o != QApplication::focusWidget())
                return false;
            m_asking = true;
            QCoreApplication::sendEvent(o, e); // the text's own answer first
            const bool hide = wantHidden(static_cast<QWidget *>(o));
            m_asking = false;
            if (hide)
                query->setValue(Qt::ImHints, QVariant(int(query->value(Qt::ImHints).toUInt() | Qt::ImhNoTextHandles)));
            return true;
        }
        case QEvent::MouseButtonPress: {
            // a finger's press that reached the text which has the keyboard: a tap, or a finger that rests (a swipe
            // never gets this far: android/MobileTouch.cpp)
            auto *me = static_cast<QMouseEvent *>(e);
            const QPointingDevice *device = me->pointingDevice();
            if (o->isWidgetType() && device && device->type() == QInputDevice::DeviceType::TouchScreen && inFocus(o))
                allowCursorHandle(true, "a tap");
            return false;
        }
        case QEvent::InputMethod: {
            // Typing, as the keyboard program sends it. Not typing: the program marking the word at the cursor as
            // the one being written (the word comes again as "preedit" and replaces itself; Android's keyboards do
            // that about a second after a tap), and its handing that word back unchanged when the cursor leaves it.
            auto *ime = static_cast<QInputMethodEvent *>(e);
            const QString marked = ime->preeditString();
            bool typed;
            if (!ime->commitString().isEmpty())
                typed = ime->replacementLength() > 0 || ime->commitString() != m_marked;
            else if (!marked.isEmpty())
                typed = m_marked.isEmpty() ? ime->replacementLength() != marked.length() : marked != m_marked;
            else
                typed = ime->replacementLength() > 0; // (something deleted)
            m_marked = marked;
            if (typed && m_cursorHandle)
                allowCursorHandle(false, "typing");
            return false;
        }
        case QEvent::KeyPress: {
            if (!m_cursorHandle)
                return false;
            const int key = static_cast<QKeyEvent *>(e)->key();
            if (key != Qt::Key_Shift && key != Qt::Key_Control && key != Qt::Key_Alt && key != Qt::Key_Meta && key != Qt::Key_AltGr)
                allowCursorHandle(false, "a key");
            return false;
        }
        case QEvent::Show:
        case QEvent::Hide:
            // a popup came or went: Qt looks again whether the handles may show (see wantHidden())
            if (o->isWidgetType() && static_cast<QWidget *>(o)->windowType() == Qt::Popup && QApplication::focusWidget())
                QTimer::singleShot(0, this, [] { QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle); });
            return false;
        case QEvent::Move:
        case QEvent::Resize:
            // something the text stands in moved (the main text in its scroll area, a pane resized)
            if (o->isWidgetType() && !m_place.isActive()) {
                const QWidget *focus = QApplication::focusWidget();
                if (focus && o != focus && static_cast<QWidget *>(o)->isAncestorOf(focus))
                    m_place.start();
            }
            return false;
        default:
            return false;
        }
    }

private:
    static bool inFocus(const QObject *o)
    {
        const QWidget *focus = QApplication::focusWidget();
        const auto *w = static_cast<const QWidget *>(o);
        return focus && (w == focus || focus->isAncestorOf(w));
    }

    // Asked while the text answers Qt: should the handles stay away now?
    bool wantHidden(QWidget *text) const
    {
        // A menu or another popup of the program is open: Android draws the handles (and its Cut / Copy bubble)
        // above every window of the app, so they would lie on the menu (seen on the emulated tablet, run 21: the
        // text's own menu under the selection's handles). They come back when the popup has gone.
        if (QApplication::activePopupWidget())
            return true;
        QInputMethodQueryEvent ask(Qt::ImCursorRectangle | Qt::ImAnchorRectangle | Qt::ImInputItemClipRectangle
                                   | Qt::ImCursorPosition | Qt::ImAnchorPosition);
        QCoreApplication::sendEvent(text, &ask);
        // all three in the text widget's own measures: the part of it that can be seen, and the two ends
        const QRectF seen = ask.value(Qt::ImInputItemClipRectangle).toRectF();
        const QRectF cursor = ask.value(Qt::ImCursorRectangle).toRectF();
        const QVariant anchorValue = ask.value(Qt::ImAnchorRectangle);
        const QRectF anchor = anchorValue.isValid() ? anchorValue.toRectF() : cursor;
        const QVariant cursorAt = ask.value(Qt::ImCursorPosition), anchorAt = ask.value(Qt::ImAnchorPosition);
        const bool selection = cursorAt.isValid() && anchorAt.isValid() && cursorAt.toInt() != anchorAt.toInt();
        // a handle hangs at the foot of its end
        auto inSight = [&seen](const QRectF &end) {
            return seen.adjusted(0, 0, 0, 1).contains(QPointF(end.center().x(), end.bottom()));
        };
        if (selection)
            return !inSight(cursor) && !inSight(anchor);
        return !m_cursorHandle || !inSight(cursor);
    }

    void allowCursorHandle(bool on, const char *why)
    {
        if (on)
            m_leave.start();
        else
            m_leave.stop();
        if (m_cursorHandle == on)
            return;
        m_cursorHandle = on;
        if (why)
            qCInfo(lcHandles, "handles: the cursor's handle %s (%s)", on ? "may show" : "goes", why);
        // Qt looks at the handles again (and asks for the hints on the way)
        if (QGuiApplication::focusObject()) {
            m_telling = true;
            QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle);
            m_telling = false;
        }
    }

    // Where the text widget stands in its window, told to Qt as QWidgetPrivate::updateWidgetTransform() tells it
    // when the widget itself moves.
    void tellPlace()
    {
        QWidget *focus = QApplication::focusWidget();
        if (!focus || focus != QGuiApplication::focusObject())
            return;
        const QPoint at = focus->mapTo(focus->window(), QPoint(0, 0));
        QTransform place;
        place.translate(at.x(), at.y());
        QInputMethod *im = QGuiApplication::inputMethod();
        if (im->inputItemTransform() == place)
            return;
        m_telling = true;
        im->setInputItemTransform(place); // (Qt moves the handles, and asks for the hints on the way)
        im->setInputItemRectangle(focus->rect());
        m_telling = false;
    }

    bool m_cursorHandle = false; // the drop under the cursor may show
    bool m_asking = false;
    bool m_telling = false;
    QString m_marked; // the word the keyboard program has marked as the one being written (the text's "preedit")
    QTimer m_leave, m_place;
};

Handles *handles()
{
    return qApp ? static_cast<Handles *>(qApp->findChild<QObject *>(QStringLiteral("mobileHandles"), Qt::FindDirectChildrenOnly))
                : nullptr;
}

} // namespace

void installHandles()
{
    if (!qApp || handles())
        return;
    auto *h = new Handles(qApp);
    h->setObjectName(QStringLiteral("mobileHandles"));
    qApp->installEventFilter(h);
}

void handlesAway()
{
    if (Handles *h = handles())
        h->away("a swipe or two fingers");
}

} // namespace mobile
} // namespace qb
