// Android 0.92.0: what a tablet with an external keyboard and mouse needs from every window (see Mobile.h,
// installCare). Compiled into the Android program and into the preview tests on a desktop (QB_MOBILE_PREVIEW), never
// into the Linux or Windows program.
#include "android/Mobile.h"
#include "ui/WindowCare.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QBoxLayout>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QGuiApplication>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLayout>
#include <QLoggingCategory>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QPointingDevice>
#include <QProgressDialog>
#include <QProxyStyle>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSplitter>
#include <QStackedWidget>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>
#include <QWheelEvent>

Q_LOGGING_CATEGORY(lcCare, "qb.tablet")

namespace qb {
namespace mobile {

namespace {

constexpr int kMouseReach = 9;     // a mouse pressed this close beside the line between two panes takes the line
constexpr int kSplitterWidth = 7;  // the line between two panes (Fusion: 3 px), with a grip in its middle
const char *kScrolled = "qbTabletScrolls";
const char *kClosable = "qbTabletClosable";

// Menus that scroll instead of wrapping into columns, and a wider line between panes with a visible grip (Qt for
// Android shows no mouse pointer shapes, so the line must be seen to be found).
class TabletStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *ret) const override
    {
        if (hint == SH_Menu_Scrollable)
            return 1;
        return QProxyStyle::styleHint(hint, option, widget, ret);
    }

    int pixelMetric(PixelMetric m, const QStyleOption *option, const QWidget *widget) const override
    {
        if (m == PM_SplitterWidth)
            return qMax(kSplitterWidth, QProxyStyle::pixelMetric(m, option, widget));
        return QProxyStyle::pixelMetric(m, option, widget);
    }

    void drawControl(ControlElement ce, const QStyleOption *option, QPainter *p, const QWidget *widget) const override
    {
        QProxyStyle::drawControl(ce, option, p, widget);
        if (ce != CE_Splitter || !option)
            return;
        // three dots across the middle of the line
        const QRect r = option->rect;
        const bool standing = option->state & State_Horizontal; // panes side by side: a standing line
        QColor dot = option->palette.color(QPalette::WindowText);
        dot.setAlpha(110);
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(Qt::NoPen);
        p->setBrush(dot);
        const QPointF c = QRectF(r).center();
        for (int i = -1; i <= 1; ++i) {
            const QPointF at = standing ? QPointF(c.x(), c.y() + i * 7) : QPointF(c.x() + i * 7, c.y());
            p->drawEllipse(at, 1.6, 1.6);
        }
        p->restore();
    }
};

bool isMainWindow(const QWidget *w) { return w && w->inherits("qb::MainWindow"); }

// ---- the main window's panes upright: a pane's pages (text, corkboard, outliner ...; the inspector's parts) are
// stacked, and Qt gives the stack the least width of its widest page, also of the pages not shown. On a tablet only
// the page shown counts, so that an upright tablet (800 px and less) has room for the binder, the text and the
// inspector, and the line between the binder and the text can still be moved.
const char *kOwnPolicy = "qbTabletOwnPolicy";

void slimPages(QStackedWidget *stack)
{
    for (int i = 0; i < stack->count(); ++i) {
        QWidget *page = stack->widget(i);
        if (!page->property(kOwnPolicy).isValid())
            page->setProperty(kOwnPolicy, int(page->sizePolicy().horizontalPolicy()));
        QSizePolicy sp = page->sizePolicy();
        sp.setHorizontalPolicy(i == stack->currentIndex() ? QSizePolicy::Policy(page->property(kOwnPolicy).toInt())
                                                          : QSizePolicy::Ignored);
        page->setSizePolicy(sp);
    }
}

void slimStack(QStackedWidget *stack)
{
    if (!stack || stack->property(kOwnPolicy).toBool() || !isMainWindow(stack->window()))
        return;
    stack->setProperty(kOwnPolicy, true);
    QObject::connect(stack, &QStackedWidget::currentChanged, stack, [stack] { slimPages(stack); });
    QObject::connect(stack, &QStackedWidget::widgetAdded, stack, [stack] { slimPages(stack); });
    slimPages(stack);
}

// ---- a menu of the menu bar that does not fit below the bar: Qt moves it up, over the bar (the other titles are
// hidden then); it stays below its title instead, as high as the room there, and scrolls
void keepBelowBar(QMenu *menu)
{
    auto *bar = qobject_cast<QMenuBar *>(menu->parentWidget());
    if (!bar || !bar->isVisible())
        return;
    QAction *title = menu->menuAction();
    const QRect at = bar->actionGeometry(title);
    if (!at.isValid())
        return;
    const QPoint below = bar->mapToGlobal(at.bottomLeft() + QPoint(0, 1));
    const QRect area = windowcare::usableArea(bar);
    const QRect g = menu->geometry();
    if (g.top() >= below.y() - 1 && g.left() <= below.x() + 2)
        return;
    const int room = area.bottom() - below.y() + 1;
    if (room < 80)
        return;
    const int x = qBound(area.left(), below.x(), qMax(area.left(), area.right() - g.width() + 1));
    menu->setGeometry(x, below.y(), g.width(), qMin(g.height(), room));
    qCInfo(lcCare, "tablet: the menu %s below its title, %d px high", qPrintable(title->text()), menu->height());
}

// a window of the program's own, not one of Qt's ready-made ones (their layouts are their own business)
bool ownDialog(const QDialog *d)
{
    if (qobject_cast<const QMessageBox *>(d) || qobject_cast<const QFileDialog *>(d) || qobject_cast<const QColorDialog *>(d)
        || qobject_cast<const QFontDialog *>(d) || qobject_cast<const QInputDialog *>(d) || qobject_cast<const QProgressDialog *>(d))
        return false;
    const QByteArray cls = d->metaObject()->className();
    return cls == "QDialog" || cls.startsWith("qb::");
}

bool hasClosingButton(const QWidget *w)
{
    return !w->findChildren<QDialogButtonBox *>().isEmpty() || !w->findChildren<QPushButton *>().isEmpty();
}

// the layout (inside `in`) that holds `w` directly
QLayout *holderOf(QLayout *in, QWidget *w)
{
    if (!in)
        return nullptr;
    for (int i = 0; i < in->count(); ++i) {
        QLayoutItem *it = in->itemAt(i);
        if (it->widget() == w)
            return in;
        if (QLayout *sub = it->layout())
            if (QLayout *found = holderOf(sub, w))
                return found;
    }
    return nullptr;
}

// A Close row for a window that has no button at all (on a computer its title bar closes it; Android has none).
void addClosing(QWidget *w)
{
    if (w->property(kClosable).toBool())
        return;
    w->setProperty(kClosable, true);
    if (auto *d = qobject_cast<QDialog *>(w)) {
        windowcare::addCloseButton(d);
        if (d->findChild<QDialogButtonBox *>(QStringLiteral("windowClose")))
            qCInfo(lcCare, "tablet: a Close row for %s", qPrintable(w->objectName()));
        return;
    }
    if (auto *mw = qobject_cast<QMainWindow *>(w)) {
        auto *bar = new QToolBar(mw);
        bar->setObjectName(QStringLiteral("windowCloseBar"));
        bar->setMovable(false);
        bar->setFloatable(false);
        auto *spacer = new QWidget(bar);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        bar->addWidget(spacer);
        auto *box = new QDialogButtonBox(QDialogButtonBox::Close, bar);
        box->setObjectName(QStringLiteral("windowClose"));
        QObject::connect(box, &QDialogButtonBox::rejected, mw, &QWidget::close);
        bar->addWidget(box);
        mw->addToolBar(Qt::BottomToolBarArea, bar);
        qCInfo(lcCare, "tablet: a Close row for %s", qPrintable(w->objectName()));
    }
}

// A dialog that cannot be made as small as the screen: its contents go into a part that scrolls, its button rows
// stay below it, in view.
bool scrollContents(QDialog *d)
{
    if (d->property(kScrolled).toBool() || !d->layout() || !ownDialog(d))
        return false;
    QLayout *old = d->layout();
    QList<QDialogButtonBox *> rows;
    for (QDialogButtonBox *b : d->findChildren<QDialogButtonBox *>())
        if (b->parentWidget() == d && holderOf(old, b))
            rows << b;
    for (QDialogButtonBox *b : rows)
        holderOf(old, b)->removeWidget(b);
    auto *inner = new QWidget;
    inner->setObjectName(QStringLiteral("tabletScrolledPart"));
    inner->setLayout(old); // (takes the layout and its widgets away from the dialog)
    auto *scroll = new QScrollArea(d);
    scroll->setObjectName(QStringLiteral("tabletScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(inner);
    auto *outer = new QVBoxLayout(d);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    outer->addWidget(scroll, 1);
    for (QDialogButtonBox *b : rows) {
        b->setContentsMargins(8, 4, 8, 8);
        outer->addWidget(b);
    }
    d->setProperty(kScrolled, true);
    // the least size the old layout gave the dialog goes with it: the new layout sets its own
    d->setMinimumSize(0, 0);
    outer->activate();
    qCInfo(lcCare, "tablet: %s scrolls (it needs %d x %d)", qPrintable(d->objectName()), inner->minimumSizeHint().width(),
           inner->minimumSizeHint().height());
    return true;
}

// whether the window can be as small as `area`
bool fitsAt(QWidget *w, const QRect &area)
{
    const QSize least = w->minimumSizeHint().expandedTo(w->minimumSize());
    return least.width() + 16 <= area.width() && least.height() + 34 <= area.height();
}

void care(QWidget *w)
{
    if (!w || !w->isWindow() || isMainWindow(w))
        return;
    const Qt::WindowType t = w->windowType();
    if (t != Qt::Dialog && t != Qt::Window && t != Qt::Sheet)
        return;
    auto *d = qobject_cast<QDialog *>(w);
    if ((d && ownDialog(d)) || qobject_cast<QMainWindow *>(w))
        if (!hasClosingButton(w))
            addClosing(w);
    const QRect area = windowcare::usableArea(w);
    if (!area.isValid())
        return;
    if (d && !fitsAt(d, area))
        scrollContents(d);
    windowcare::fitInto(w, area);
}

class Filter : public QObject
{
public:
    explicit Filter(QObject *parent) : QObject(parent) {}

    bool eventFilter(QObject *o, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
            return key(static_cast<QKeyEvent *>(e));
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
            if (o->isWidgetType())
                return mouse(static_cast<QWidget *>(o), static_cast<QMouseEvent *>(e));
            break;
        case QEvent::Show:
            if (auto *stack = qobject_cast<QStackedWidget *>(o))
                slimStack(stack);
            if (auto *menu = qobject_cast<QMenu *>(o))
                keepBelowBar(menu);
            if (o->isWidgetType() && static_cast<QWidget *>(o)->isWindow()) {
                auto *w = static_cast<QWidget *>(o);
                care(w);
                const Qt::WindowType t = w->windowType();
                if (!isMainWindow(w) && (t == Qt::Dialog || t == Qt::Window || t == Qt::Sheet))
                    noteShown(w);
            }
            break;
        default: break;
        }
        return false;
    }

    // the usable area changed (the on-screen keyboard came or went, the tablet was turned)
    void areaChanged()
    {
        for (QWidget *w : QApplication::topLevelWidgets())
            if (w->isVisible())
                care(w);
    }

private:
    // ---- the keys (Android 0.93.0: Qt 6.12.1 for Android sends mouse and wheel events with Ctrl, Shift, Alt and Meta;
    // Qt 6.12.0 sent them without, and the program kept the keys for the mouse until then)
    bool key(QKeyEvent *k)
    {
        const bool down = k->type() == QEvent::KeyPress;
        if (k->key() == Qt::Key_Back) {
            if (down && !k->isAutoRepeat()) {
                m_backTaken = closeTop();
                return m_backTaken;
            }
            if (!down && m_backTaken) {
                if (!k->isAutoRepeat())
                    m_backTaken = false;
                return true;
            }
        }
        return false;
    }

    // ---- Android's Back closes what is in front (a menu, a tip, a window) instead of putting the app away; with only
    // the main window left it does what Android does
    // the program's windows (not the main window) in the order they were shown
    static QList<QPointer<QWidget>> &shown()
    {
        static QList<QPointer<QWidget>> list;
        return list;
    }

public:
    static void noteShown(QWidget *w)
    {
        QList<QPointer<QWidget>> &l = shown();
        l.removeIf([w](const QPointer<QWidget> &p) { return !p || p == w || !p->isVisible(); });
        l << w;
    }

private:
    static bool closeTop()
    {
        if (QWidget *p = QApplication::activePopupWidget()) {
            p->close();
            return true;
        }
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
            return true;
        }
        QWidget *front = QApplication::activeModalWidget();
        if (!front) {
            QWidget *a = QApplication::activeWindow();
            if (a && a->isVisible() && !isMainWindow(a) && (a->windowType() == Qt::Dialog || a->windowType() == Qt::Window))
                front = a;
        }
        // a window in front of the main window that did not get the keyboard (a window that is not modal: Targets,
        // the manual): the one shown last
        for (int i = shown().size() - 1; !front && i >= 0; --i)
            if (QWidget *w = shown().at(i); w && w->isVisible())
                front = w;
        if (!front)
            return false;
        qCInfo(lcCare, "tablet: Back closes %s", qPrintable(front->objectName()));
        if (auto *d = qobject_cast<QDialog *>(front))
            d->reject();
        else
            front->close();
        return true;
    }

    // ---- a mouse pressed close beside the line between two panes takes the line along (the line is a few pixels
    // wide, and Qt for Android shows no pointer shape that says where it is)
    static bool realMouse(const QMouseEvent *m)
    {
        return m->source() == Qt::MouseEventNotSynthesized && m->pointingDevice()
               && m->pointingDevice()->type() == QInputDevice::DeviceType::Mouse;
    }

    static QSplitterHandle *lineNear(const QPoint &global, const QWidget *under)
    {
        const QWidget *top = under->window();
        QSplitterHandle *best = nullptr;
        int bestAway = kMouseReach + 1;
        for (QSplitterHandle *h : top->findChildren<QSplitterHandle *>()) {
            if (!h->isVisible() || !h->isEnabled() || h->window() != top)
                continue;
            const QRect r(h->mapToGlobal(QPoint(0, 0)), h->size());
            const bool standing = h->orientation() == Qt::Horizontal;
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

    void toLine(QEvent::Type type, const QMouseEvent *m, Qt::MouseButton button, Qt::MouseButtons buttons)
    {
        const QPointer<QSplitterHandle> h = m_line;
        if (!h)
            return;
        const QPointF global = m->globalPosition();
        QMouseEvent event(type, h->mapFromGlobal(global), h->window()->mapFromGlobal(global), global, button, buttons,
                          m->modifiers(), m->pointingDevice());
        m_delivering = true;
        QApplication::sendEvent(h, &event);
        m_delivering = false;
    }

    bool mouse(QWidget *w, QMouseEvent *m)
    {
        if (m_delivering)
            return false;
        if (m_line) {
            if (m->type() == QEvent::MouseMove) {
                toLine(QEvent::MouseMove, m, Qt::NoButton, m->buttons());
                return true;
            }
            if (m->type() == QEvent::MouseButtonRelease && m->button() == Qt::LeftButton) {
                toLine(QEvent::MouseButtonRelease, m, Qt::LeftButton, m->buttons());
                m_line = nullptr;
                return true;
            }
            return true;
        }
        if (m->type() != QEvent::MouseButtonPress || m->button() != Qt::LeftButton || !realMouse(m)
            || qobject_cast<QSplitterHandle *>(w) || QApplication::activePopupWidget())
            return false;
        QSplitterHandle *h = lineNear(m->globalPosition().toPoint(), w);
        if (!h)
            return false;
        m_line = h;
        toLine(QEvent::MouseButtonPress, m, Qt::LeftButton, m->buttons());
        return true;
    }

    bool m_backTaken = false;
    QPointer<QSplitterHandle> m_line;
    bool m_delivering = false;
};

} // namespace

void installCare()
{
    static bool done = false;
    if (done || !qApp)
        return;
    done = true;
    windowcare::setTabletMode(true);
    QStyle *base = QStyleFactory::create(QApplication::style() ? QApplication::style()->name() : QStringLiteral("Fusion"));
    if (base)
        QApplication::setStyle(new TabletStyle(base));
    auto *filter = new Filter(qApp);
    qApp->installEventFilter(filter);
    auto watch = [filter](QScreen *s) {
        QObject::connect(s, &QScreen::availableGeometryChanged, filter, [filter] {
            QTimer::singleShot(0, filter, [filter] { filter->areaChanged(); });
        });
    };
    for (QScreen *s : QGuiApplication::screens())
        watch(s);
    QObject::connect(qApp, &QGuiApplication::screenAdded, filter, watch);
}

} // namespace mobile
} // namespace qb
