// Checks of Android's interface code (src/android), in a preview on the desktop: the main window in the sizes of
// tablets, fingers simulated with Qt's touch test events (they take the same way through Qt as a real touch screen's).
// Built only with -DQB_MOBILE_PREVIEW=ON; part of no test run of Linux or Windows.
//  - Toolbars (android/MobileBars.cpp): no toolbar runs over the window's edge at any width; a row that is too wide
//    hands its last toolbars to the next row; more than three rows: the Themes toolbar makes room and comes back; a
//    writer who switches it on keeps it; the writer's own order and rows are the ones a wide window shows again.
//  - Fingers (android/MobileTouch.cpp): a swipe scrolls the text, a list, a tree, a page of controls, and selects
//    nothing; a throw; a tap puts the cursor and chooses a line; a finger that rests drags as a mouse; a double tap
//    selects a word; a mouse still selects by dragging; two fingers zoom the text and the binder and click nothing;
//    buttons and menus answer at once.
//
//   xvfb-run -a ./qb_mobile_tests /tmp/shots

#include "android/Mobile.h"
#include "compile/PluginManager.h"
#include "core/Backup.h"
#include "core/BinderItem.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "text/DisplayFormat.h"
#include "ui/AppSettings.h"
#include "ui/BarDock.h"
#include "ui/BarPlaces.h"
#include "ui/BinderPanel.h"
#include "ui/EditorPane.h"
#include "ui/GripBar.h"
#include "ui/MainWindow.h"
#include "ui/TextColumnView.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"
#include "ui/ToolbarConfig.h"
#include "ui/Zoom.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPointingDevice>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QInputMethodQueryEvent>
#include <QSplitter>
#include <QTransform>
#include <QSlider>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <cstdio>

using namespace qb;
using namespace qb::barplaces;

static int failures = 0;
static QString shotDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!detail.isEmpty() && (!ok || detail.startsWith(QLatin1Char('('))))
        std::printf("      %s\n", qPrintable(detail.left(2000)));
    if (!ok)
        ++failures;
    std::fflush(stdout);
}

static void info(const QString &s)
{
    std::printf("      %s\n", qPrintable(s));
    std::fflush(stdout);
}

static void settle(int ms = 60)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(5);
    }
}

static void shot(QWidget *w, const QString &name)
{
    if (shotDir.isEmpty() || !w)
        return;
    settle(80);
    w->grab().save(shotDir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

// ------------------------------------------------------------------ toolbars

struct Bars
{
    MainWindow *w = nullptr;
    BarDock *dock = nullptr;
    QList<GripBar *> all;
    QPoint at(QWidget *bar) const { return bar->mapTo(w, QPoint(0, 0)); }
    // the rows as they show: the visible toolbars by where they stand
    QList<QList<GripBar *>> rows() const
    {
        QList<GripBar *> v;
        for (GripBar *b : all)
            if (b->isVisible())
                v << b;
        std::sort(v.begin(), v.end(), [this](GripBar *x, GripBar *y) {
            const QPoint px = at(x), py = at(y);
            return px.y() != py.y() ? px.y() < py.y() : px.x() < py.x();
        });
        QList<QList<GripBar *>> out;
        for (int i = 0; i < v.size(); ++i) {
            if (i == 0 || at(v[i]).y() != at(v[i - 1]).y())
                out << QList<GripBar *>();
            out.last() << v[i];
        }
        return out;
    }
    QString seen() const
    {
        QStringList lines;
        for (const QList<GripBar *> &row : rows()) {
            QStringList names;
            for (GripBar *b : row)
                names << b->objectName();
            lines << names.join(QLatin1Char(' '));
        }
        return lines.join(QLatin1String(" / "));
    }
    // No toolbar ends beyond the window's right edge, and none is squeezed (its last buttons in the overflow menu)
    // unless it stands alone in its row.
    bool inside(QString *why) const
    {
        bool ok = true;
        QStringList notes;
        for (const QList<GripBar *> &row : rows())
            for (GripBar *b : row) {
                const int right = at(b).x() + b->width();
                const bool squeezed = b->width() < b->naturalWidth();
                if (right > w->width() || (squeezed && row.size() > 1)) {
                    ok = false;
                    notes << QStringLiteral("%1: x %2, width %3 (needs %4), window %5")
                                 .arg(b->objectName()).arg(at(b).x()).arg(b->width()).arg(b->naturalWidth()).arg(w->width());
                }
            }
        if (why)
            *why = notes.join(QLatin1String("; "));
        return ok;
    }
    GripBar *named(const QString &name) const
    {
        for (GripBar *b : all)
            if (b->objectName() == name)
                return b;
        return nullptr;
    }
};

// The rule, worked out here from the rows the writer has and what each toolbar needs: a row keeps its toolbars while
// they fit, the rest goes before the toolbars of the next row. `without` is left out.
static QStringList expectedRows(const Bars &b, const Places &wanted, int width, const QString &without = QString())
{
    // (the Themes toolbar counts as shown unless it is the one left out: it may be off for room at this moment)
    const QString themes = toolbars::barObjectName(toolbars::Bar::Themes);
    QStringList out;
    QStringList carry;
    for (int r = 0; r < wanted.top.size() || !carry.isEmpty(); ++r) {
        QStringList row = carry;
        carry.clear();
        if (r < wanted.top.size())
            for (const Slot &s : wanted.top[r])
                row << s.name;
        QStringList keep;
        int used = 0;
        bool full = false;
        for (const QString &name : std::as_const(row)) {
            GripBar *bar = b.named(name);
            if (!bar || name == without || (bar->isHidden() && name != themes))
                continue;
            const int need = bar->naturalWidth() + 2;
            if (!full && !keep.isEmpty() && used + need > width)
                full = true;
            if (full) {
                carry << name;
                continue;
            }
            keep << name;
            used += need;
        }
        if (!keep.isEmpty())
            out << keep.join(QLatin1Char(' '));
    }
    return out;
}

static void barChecks(MainWindow &w)
{
    std::printf("== Toolbars that fit the window's width\n");
    Bars b;
    b.w = &w;
    b.dock = w.findChild<BarDock *>();
    for (toolbars::Bar bar : toolbars::allBars())
        b.all << w.findChild<GripBar *>(toolbars::barObjectName(bar));
    b.all << w.findChild<GripBar *>(QStringLiteral("iconBar"));
    check(b.dock && !b.all.contains(nullptr), QStringLiteral("(the window has its toolbars and their place keeper)"));
    if (!b.dock || b.all.contains(nullptr))
        return;
    const QString themesName = toolbars::barObjectName(toolbars::Bar::Themes);
    GripBar *themes = b.named(themesName);
    QAction *showThemes = w.findChild<QAction *>(toolbars::barCommand(toolbars::Bar::Themes));
    QAction *showIcons = w.findChild<QAction *>(QStringLiteral("showIconBar"));
    if (showIcons && !showIcons->isChecked())
        showIcons->trigger();
    w.resize(1900, 950);
    settle(300);
    const QString standard = b.seen();
    const Places wanted = b.dock->places();
    {
        QStringList needs;
        int sum = 0;
        for (GripBar *bar : b.all) {
            needs << QStringLiteral("%1 %2").arg(bar->objectName()).arg(bar->naturalWidth());
            sum += bar->isVisible() ? bar->naturalWidth() : 0;
        }
        info(QStringLiteral("what the toolbars need (px): %1; together %2").arg(needs.join(QLatin1String(", "))).arg(sum));
    }

    mobile::fitToolbars(&w);
    settle(300);
    QString why;
    check(b.seen() == standard && b.inside(&why) && b.rows().size() == 2 && showThemes && showThemes->isChecked(),
          QStringLiteral("in a window wide enough for them (1900 px) the two toolbar rows stay as they are"),
          QStringLiteral("%1 | %2").arg(b.seen(), why));

    // the sizes of tablets, in the units the program counts in: 12.7" (1472 x 920), 11" (1280 x 800), with a larger
    // display size or held upright (1178, 960, 920, 800), a small window (600)
    int themesGoesAt = 0;
    for (int width : {1472, 1280, 1178, 960, 920, 800, 600}) {
        w.resize(width, width > 1000 ? 800 : 1100);
        settle(400);
        if (w.width() != width) {
            info(QStringLiteral("(the window does not get narrower than %1 px: its smallest size is %2 x %3)").arg(w.width())
                     .arg(w.minimumSizeHint().width()).arg(w.minimumSizeHint().height()));
            width = w.width();
        }
        const QStringList with = expectedRows(b, wanted, width);
        const bool tooMany = with.size() > 3;
        const QStringList expected = tooMany ? expectedRows(b, wanted, width, themesName) : with;
        const bool ok = b.inside(&why) && b.seen() == expected.join(QLatin1String(" / ")) && themes->isVisible() == !tooMany
                        && showThemes->isChecked() == !tooMany && (b.rows().size() <= 3 || tooMany);
        check(ok,
              QStringLiteral("%1 px wide: no toolbar runs over the edge, %2 rows%3")
                  .arg(width).arg(b.rows().size()).arg(tooMany ? QStringLiteral(", the Themes toolbar made room") : QString()),
              QStringLiteral("%1 | expected %2 | %3").arg(b.seen(), expected.join(QLatin1String(" / ")), why));
        info(b.seen());
        shot(&w, QStringLiteral("bars-%1").arg(width));
        if (tooMany && !themesGoesAt)
            themesGoesAt = width;
    }
    check(themesGoesAt > 0, QStringLiteral("(one of the widths needs more than three rows, so the Themes toolbar's part is checked)"));
    if (!themesGoesAt)
        themesGoesAt = 600;

    // wide again: everything as the writer had it
    w.resize(1900, 950);
    settle(400);
    check(b.seen() == standard && themes->isVisible() && showThemes->isChecked() && b.inside(&why),
          QStringLiteral("the window wide again: the two rows as before, the Themes toolbar back"),
          QStringLiteral("%1 | %2").arg(b.seen(), why));
    check(QSettings().value(QStringLiteral("window/mobileBarRows")).toStringList() == toText(wanted)
              && !QSettings().value(QStringLiteral("window/mobileThemesOffForRoom")).toBool(),
          QStringLiteral("the writer's rows are kept apart from the fitted ones"),
          QSettings().value(QStringLiteral("window/mobileBarRows")).toStringList().join(QLatin1String(" ; ")));

    // the writer switches the Themes toolbar on where it made room: it stays, also after the next turn of the tablet
    w.resize(themesGoesAt, 1100);
    settle(400);
    const bool offForRoom = !themes->isVisible() && !showThemes->isChecked()
                            && QSettings().value(QStringLiteral("window/mobileThemesOffForRoom")).toBool();
    showThemes->trigger();
    settle(400);
    const bool onByHand = themes->isVisible() && showThemes->isChecked() && b.inside(&why);
    const QString rowsByHand = b.seen();
    w.resize(1900, 950);
    settle(300);
    w.resize(themesGoesAt, 1100);
    settle(400);
    check(offForRoom && onByHand && themes->isVisible() && b.seen() == rowsByHand && b.inside(&why),
          QStringLiteral("switched on by the writer at %1 px, the Themes toolbar stays (the rows become %2), also after the window "
                         "was wide and narrow again").arg(themesGoesAt).arg(b.rows().size()),
          QStringLiteral("off for room %1, on by hand %2 | %3 | %4").arg(offForRoom).arg(onByHand).arg(b.seen(), why));
    shot(&w, QStringLiteral("bars-themes-kept"));
    // ... and off by the writer is off at every width
    showThemes->trigger();
    settle(300);
    w.resize(1900, 950);
    settle(400);
    check(!themes->isVisible() && !showThemes->isChecked(), QStringLiteral("switched off by the writer, it stays off in the wide window too"),
          b.seen());
    showThemes->trigger();
    settle(300);
    check(b.seen() == standard, QStringLiteral("(on again: the two rows as before)"), b.seen());

    // the writer's own order: Tools taken to the front of the second row; a narrow window flows from that, a wide
    // one shows it again
    const QString searchName = toolbars::barObjectName(toolbars::Bar::Search);
    const QString toolsName = toolbars::barObjectName(toolbars::Bar::Tools);
    Target to;
    to.valid = true;
    to.row = 1;
    to.before = b.dock->places().top.value(1).value(0).name;
    const Places own = moved(b.dock->places(), toolsName, to);
    b.dock->setPlaces(own); // (as the end of a drag does it)
    settle(400);
    const QString ownSeen = b.seen();
    const Places ownKept = b.dock->places();
    w.resize(960, 1100);
    settle(400);
    const QStringList ownNarrow = expectedRows(b, own, 960);
    const bool narrowOk = b.inside(&why) && b.seen() == ownNarrow.join(QLatin1String(" / "));
    const QString narrowSeen = b.seen();
    w.resize(1900, 950);
    settle(400);
    const QStringList keptText = QSettings().value(QStringLiteral("window/mobileBarRows")).toStringList();
    check(ownSeen != standard && ownSeen.contains(QLatin1String("/ ") + toolsName) && narrowOk && b.seen() == ownSeen
              && keptText == toText(ownKept),
          QStringLiteral("a toolbar the writer moved (Tools to the front of the second row) keeps its place: fitted at 960 px, as the "
                         "writer put it in the wide window"),
          QStringLiteral("%1 | 960: %2 | expected %3 | wide: %4 | kept: %5 | %6")
              .arg(ownSeen, narrowSeen, ownNarrow.join(QLatin1String(" / ")), b.seen(), keptText.join(QLatin1String(" ; ")), why));

    // Reset Toolbar Places gives the standard rows, fitted
    if (QAction *reset = w.findChild<QAction *>(QStringLiteral("resetToolbarPlaces")))
        reset->trigger();
    settle(400);
    const bool resetWide = b.seen() == standard;
    w.resize(1178, 800);
    settle(400);
    check(resetWide && b.inside(&why) && b.seen() == expectedRows(b, wanted, 1178).join(QLatin1String(" / ")),
          QStringLiteral("Reset Toolbar Places: the standard rows, fitted to the width"), QStringLiteral("%1 | %2").arg(b.seen(), why));

    // a toolbar switched off gives its room to the others
    QAction *showSearch = w.findChild<QAction *>(toolbars::barCommand(toolbars::Bar::Search));
    const int rowsWith = b.rows().size();
    if (showSearch)
        showSearch->trigger();
    settle(400);
    const bool offOk = showSearch && !b.named(searchName)->isVisible() && b.inside(&why)
                       && b.seen() == expectedRows(b, wanted, 1178).join(QLatin1String(" / "));
    const int rowsWithout = b.rows().size();
    if (showSearch)
        showSearch->trigger();
    settle(400);
    check(offOk && b.inside(&why) && b.rows().size() == rowsWith && b.seen() == expectedRows(b, wanted, 1178).join(QLatin1String(" / ")),
          QStringLiteral("a toolbar switched off and on again (Search): the rows are fitted anew each time"),
          QStringLiteral("rows %1 -> %2 -> %3 | %4 | %5").arg(rowsWith).arg(rowsWithout).arg(b.rows().size()).arg(b.seen(), why));
    w.resize(1280, 800);
    settle(400);
}

// ------------------------------------------------------------------ fingers

// QB_MOBILE_TRACE=1: the mouse events as a widget gets them (after the finger filter)
class Trace : public QObject
{
public:
    using QObject::QObject;
    bool eventFilter(QObject *, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove: {
            auto *me = static_cast<QMouseEvent *>(e);
            std::printf("      trace: %s at %d,%d buttons %d spontaneous %d device %d\n",
                        e->type() == QEvent::MouseButtonPress ? "press" : e->type() == QEvent::MouseButtonRelease ? "release"
                        : e->type() == QEvent::MouseMove ? "move" : "double click",
                        int(me->position().x()), int(me->position().y()), int(me->buttons()), int(e->spontaneous()),
                        int(me->pointingDevice()->type()));
            break;
        }
        default:
            break;
        }
        return false;
    }
};
static void trace(QWidget *w)
{
    if (qEnvironmentVariableIsSet("QB_MOBILE_TRACE"))
        w->installEventFilter(new Trace(w));
}

struct Hand
{
    QPointingDevice *dev = nullptr;
    QWidget *win = nullptr; // positions are this window's
    // Qt takes a press of a finger within 400 ms of the one before for the second half of a double tap: touches that
    // are not meant as one begin after that time (`soon`: at once, as a writer's quick second tap)
    static QElapsedTimer &sinceLast()
    {
        static QElapsedTimer t;
        return t;
    }
    static void fresh()
    {
        while (sinceLast().isValid() && sinceLast().elapsed() < 470)
            QTest::qWait(10);
    }
    void down(const QPoint &p, bool soon = false) const
    {
        if (!soon)
            fresh();
        sinceLast().start();
        QTest::touchEvent(win, dev).press(0, p, win);
    }
    void to(const QPoint &p) const { QTest::touchEvent(win, dev).move(0, p, win); }
    void up(const QPoint &p) const { QTest::touchEvent(win, dev).release(0, p, win); }
    void tap(const QPoint &p, bool soon = false) const
    {
        down(p, soon);
        QTest::qWait(40);
        up(p);
        settle(80);
    }
    // from `a` to `b` in steps; `restMs`: the finger rests there before it goes up (no throw)
    void swipe(const QPoint &a, const QPoint &b, int steps = 12, int stepMs = 10, int restMs = 0) const
    {
        down(a);
        for (int k = 1; k <= steps; ++k) {
            QTest::qWait(stepMs);
            to(a + (b - a) * k / steps);
        }
        if (restMs)
            QTest::qWait(restMs);
        up(b);
    }
    // two fingers on a horizontal line around `c`, from `from` px apart to `to` px apart
    void pinch(const QPoint &c, int from, int toSpan, int steps = 10) const
    {
        const auto left = [&](int span) { return c - QPoint(span / 2, 0); };
        const auto right = [&](int span) { return c + QPoint(span / 2, 0); };
        fresh();
        sinceLast().start();
        QTest::touchEvent(win, dev).press(0, left(from), win);
        QTest::qWait(20);
        QTest::touchEvent(win, dev).move(0, left(from), win).press(1, right(from), win);
        for (int k = 1; k <= steps; ++k) {
            QTest::qWait(12);
            const int span = from + (toSpan - from) * k / steps;
            QTest::touchEvent(win, dev).move(0, left(span), win).move(1, right(span), win);
        }
        QTest::qWait(20);
        // the first finger goes up, the other moves a little more and goes up too
        QTest::touchEvent(win, dev).release(0, left(toSpan), win).move(1, right(toSpan), win);
        QTest::qWait(15);
        QTest::touchEvent(win, dev).move(1, right(toSpan) + QPoint(6, 3), win);
        QTest::qWait(15);
        QTest::touchEvent(win, dev).release(1, right(toSpan) + QPoint(6, 3), win);
        settle(120);
    }
};

static bool waitStill(QScrollBar *bar, int ms = 2500)
{
    QElapsedTimer t;
    t.start();
    int last = bar->value(), same = 0;
    while (t.elapsed() < ms) {
        settle(60);
        same = bar->value() == last ? same + 1 : 0;
        last = bar->value();
        if (same >= 4)
            return true;
    }
    return false;
}

static void fingerChecks(MainWindow &w, BinderItem *longDoc)
{
    std::printf("== Fingers: scrolling, taps, zoom\n");
    Project *p = w.project();
    Hand hand;
    hand.dev = QTest::createTouchDevice();
    hand.win = &w;

    EditorPane *pane = nullptr;
    for (EditorPane *ep : w.findChildren<EditorPane *>())
        if (ep->isVisible())
            pane = ep;
    auto *binder = w.findChild<BinderPanel *>();
    if (pane && binder) {
        binder->select({longDoc}, false);
        pane->showItems({longDoc}, false, int(ViewMode::Single));
    }
    settle(500);
    TextColumnView *view = nullptr;
    if (pane)
        for (TextColumnView *v : pane->findChildren<TextColumnView *>())
            if (v->isVisible())
                view = v;
    TextEditor *ed = view ? view->editors().value(0) : nullptr;
    QScrollBar *bar = view ? view->verticalScrollBar() : nullptr;
    check(ed && bar && bar->maximum() > 2000, QStringLiteral("(a long text is open in the editor)"),
          QStringLiteral("maximum %1").arg(bar ? bar->maximum() : -1));
    if (!ed || !bar)
        return;
    trace(ed->viewport());
    // a point in the text, in the window's coordinates
    const auto inText = [&](int x, int y) { return view->viewport()->mapTo(&w, QPoint(view->viewport()->width() / 2 + x, y)); };
    const auto cursorAt = [&](const QPoint &windowPos) {
        return ed->cursorForPosition(ed->viewport()->mapFrom(&w, windowPos)).position();
    };

    // ---- a swipe scrolls and selects nothing
    QTextCursor c = ed->textCursor();
    c.setPosition(0);
    ed->setTextCursor(c);
    bar->setValue(0);
    settle(100);
    hand.swipe(inText(0, 520), inText(0, 220), 12, 10, 250); // 300 px up, the finger rests before it goes up
    settle(100);
    const int slow = bar->value();
    settle(400);
    check(slow >= 260 && slow <= 300 && bar->value() == slow && ed->textCursor().position() == 0 && !ed->textCursor().hasSelection(),
          QStringLiteral("a finger moved 300 px up over the text scrolls it that far; nothing is selected, the cursor stays; the view "
                         "stops with a finger that rested"),
          QStringLiteral("scrolled %1 (then %2), cursor %3, selection %4").arg(slow).arg(bar->value()).arg(ed->textCursor().position())
              .arg(ed->textCursor().hasSelection()));
    shot(&w, QStringLiteral("finger-scrolled"));

    hand.swipe(inText(0, 520), inText(0, 220), 10, 8); // the same without a rest: thrown
    const int atRelease = bar->value();
    const bool still = waitStill(bar);
    check(atRelease >= slow + 250 && bar->value() > atRelease + 150 && still && !ed->textCursor().hasSelection(),
          QStringLiteral("a finger that goes up while it moves throws the text: it runs on and comes to rest"),
          QStringLiteral("at the release %1, at rest %2, still %3").arg(atRelease).arg(bar->value()).arg(still));

    {
        const int before = bar->value();
        hand.swipe(inText(0, 220), inText(0, 470), 12, 10, 250); // back down
        settle(150);
        check(bar->value() <= before - 210 && bar->value() >= before - 250, QStringLiteral("a swipe downward scrolls back"),
              QStringLiteral("%1 -> %2").arg(before).arg(bar->value()));
    }
    {
        // a finger put on the running text stops it and is no tap
        hand.swipe(inText(0, 560), inText(0, 200), 8, 6);
        settle(60);
        const int running = bar->value();
        const int cursorBefore = ed->textCursor().position();
        hand.down(inText(0, 400));
        settle(40);
        const int stopped = bar->value();
        settle(200);
        const bool stays = bar->value() == stopped;
        hand.up(inText(0, 400));
        settle(120);
        check(stopped >= running && stays && ed->textCursor().position() == cursorBefore,
              QStringLiteral("a finger put on the running text stops it, and puts no cursor"),
              QStringLiteral("running %1, stopped %2, stays %3, cursor %4 -> %5").arg(running).arg(stopped).arg(stays).arg(cursorBefore)
                  .arg(ed->textCursor().position()));
    }
    {
        const int before = bar->value(), cursorBefore = ed->textCursor().position();
        hand.swipe(inText(-150, 400), inText(150, 404), 12, 10);
        settle(200);
        check(bar->value() == before && !ed->textCursor().hasSelection() && ed->textCursor().position() == cursorBefore,
              QStringLiteral("a swipe sideways over the text does nothing (no selection)"),
              QStringLiteral("scroll %1 -> %2, selection %3").arg(before).arg(bar->value()).arg(ed->textCursor().selectedText().left(40)));
    }

    // ---- a tap, a rest, a double tap, the mouse
    {
        const QPoint at = inText(-60, 300);
        const int want = cursorAt(at), before = ed->textCursor().position();
        hand.down(at);
        settle(80);
        const int during = ed->textCursor().position();
        hand.up(at);
        settle(120);
        check(want != before && during == before && ed->textCursor().position() == want && !ed->textCursor().hasSelection()
                  && ed->hasFocus(),
              QStringLiteral("a tap puts the cursor where the finger was (when the finger goes up)"),
              QStringLiteral("before %1, finger down %2, after %3, wanted %4, focus %5").arg(before).arg(during)
                  .arg(ed->textCursor().position()).arg(want).arg(ed->hasFocus()));
    }
    {
        const QPoint at = inText(-120, 360);
        const int want = cursorAt(at), scroll = bar->value();
        hand.down(at);
        settle(420); // the finger rests
        const int rested = ed->textCursor().position();
        for (int k = 1; k <= 10; ++k) {
            QTest::qWait(10);
            hand.to(at + QPoint(22 * k, 5 * k));
        }
        settle(60);
        const QString sel = ed->textCursor().selectedText();
        hand.up(at + QPoint(220, 50));
        settle(120);
        check(rested == want && sel.size() > 5 && bar->value() == scroll && ed->textCursor().selectedText() == sel,
              QStringLiteral("a finger that rests for a moment has pressed: moved then, it selects text as a mouse does"),
              QStringLiteral("cursor after the rest %1 (wanted %2), selected '%3', scroll %4 -> %5").arg(rested).arg(want).arg(sel.left(60))
                  .arg(scroll).arg(bar->value()));
        shot(&w, QStringLiteral("finger-selected"));
    }
    {
        // on the word "harbor" of the paragraph in the middle of the view
        QTextCursor word = ed->cursorForPosition(ed->viewport()->mapFrom(&w, inText(0, 420)));
        const int in = word.block().text().indexOf(QLatin1String("harbor"));
        word.setPosition(word.block().position() + qMax(0, in) + 3);
        const QPoint at = ed->viewport()->mapTo(&w, ed->cursorRect(word).center());
        // (nothing selected first: a tap on selected text opens the text's menu, see menuChecks)
        QTextCursor none = ed->textCursor();
        none.clearSelection();
        ed->setTextCursor(none);
        hand.tap(at);
        QTest::qWait(60);
        hand.tap(at, true);
        settle(150);
        const QString sel = ed->textCursor().selectedText();
        check(in >= 0 && sel == QLatin1String("harbor"), QStringLiteral("a double tap selects the word"),
              QStringLiteral("'%1'").arg(sel.left(60)));
    }
    {
        // a quick tap somewhere else is a tap of its own (Qt alone would take it for the second half of a double tap)
        hand.tap(inText(-100, 250));
        QTest::qWait(50);
        const QPoint at = inText(60, 520);
        hand.tap(at, true);
        settle(150);
        check(!ed->textCursor().hasSelection() && ed->textCursor().position() == cursorAt(at),
              QStringLiteral("two quick taps at two places are two taps: the cursor is at the second, no word selected"),
              QStringLiteral("selected '%1', cursor %2, wanted %3").arg(ed->textCursor().selectedText().left(40))
                  .arg(ed->textCursor().position()).arg(cursorAt(at)));
    }
    {
        // after a swipe, a quick tap at the place the swipe began is a tap, not the second half of a double tap
        const QPoint at = inText(-40, 500);
        hand.swipe(at, at + QPoint(0, -120), 6, 8, 80);
        QTest::qWait(30);
        hand.tap(at, true);
        settle(150);
        check(!ed->textCursor().hasSelection() && ed->textCursor().position() == cursorAt(at),
              QStringLiteral("a tap right after a swipe puts the cursor (it is not taken for a double tap)"),
              QStringLiteral("selected '%1'").arg(ed->textCursor().selectedText().left(40)));
    }
    {
        const int scroll = bar->value();
        QWidget *vp = ed->viewport();
        const QPoint a = vp->mapFrom(&w, inText(-150, 300)), z = vp->mapFrom(&w, inText(120, 380));
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, a);
        for (int k = 1; k <= 8; ++k)
            QTest::mouseMove(vp, a + (z - a) * k / 8, 5);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, z);
        settle(120);
        check(ed->textCursor().selectedText().size() > 5 && bar->value() == scroll,
              QStringLiteral("a mouse still selects by dragging and scrolls nothing"),
              QStringLiteral("selected '%1', scroll %2 -> %3").arg(ed->textCursor().selectedText().left(40)).arg(scroll).arg(bar->value()));
    }

    // ---- two fingers
    {
        QTextCursor none = ed->textCursor();
        none.clearSelection();
        ed->setTextCursor(none);
        hand.tap(inText(-60, 300));
        const int cursorBefore = ed->textCursor().position();
        const double z0 = p->docs()->zoom();
        hand.pinch(inText(0, 400), 100, 150); // half as far apart again: four steps
        const double z1 = p->docs()->zoom();
        shot(&w, QStringLiteral("finger-zoomed"));
        hand.pinch(inText(0, 400), 150, 100); // and together again: the same four steps back
        const double z2 = p->docs()->zoom();
        hand.pinch(inText(0, 400), 90, 180); // twice as far apart: seven steps
        const double z3 = p->docs()->zoom();
        hand.pinch(inText(0, 400), 240, 120);
        check(qAbs(z1 - (z0 + 0.4)) < 0.01 && qAbs(z2 - z0) < 0.01 && qAbs(z3 - (z0 + 0.7)) < 0.01 && qAbs(p->docs()->zoom() - z0) < 0.01
                  && !ed->textCursor().hasSelection() && ed->textCursor().position() == cursorBefore,
              QStringLiteral("two fingers moved apart zoom the text in, moved together out by the same steps (twice as far apart: "
                             "seven steps of 10 %); nothing is clicked or selected by it"),
              QStringLiteral("zoom %1 -> %2 -> %3 -> %4 -> %5, cursor %6 -> %7, selection %8").arg(z0).arg(z1).arg(z2).arg(z3)
                  .arg(p->docs()->zoom()).arg(cursorBefore).arg(ed->textCursor().position()).arg(ed->textCursor().hasSelection()));
        const QPoint at = inText(-80, 340);
        hand.tap(at);
        check(ed->textCursor().position() == cursorAt(at) && !ed->textCursor().hasSelection(),
              QStringLiteral("after the two fingers, a tap is a tap again"),
              QStringLiteral("cursor %1, wanted %2").arg(ed->textCursor().position()).arg(cursorAt(at)));
    }

    // ---- the binder: a tree
    if (binder) {
        QTreeView *tree = binder->tree();
        tree->expandAll();
        settle(300);
        QScrollBar *tb = tree->verticalScrollBar();
        const auto inTree = [&](int x, int y) { return tree->viewport()->mapTo(&w, QPoint(x, y)); };
        const QModelIndex current = tree->currentIndex();
        const int before = tb->value();
        hand.swipe(inTree(90, 420), inTree(90, 220), 12, 10, 250);
        settle(150);
        const int after = tb->value();
        check(tb->maximum() > 300 && tree->verticalScrollMode() == QAbstractItemView::ScrollPerPixel && after - before >= 160
                  && after - before <= 200 && tree->currentIndex() == current,
              QStringLiteral("the binder: a swipe scrolls the tree with the finger (by pixels) and chooses nothing"),
              QStringLiteral("maximum %1, %2 -> %3, per pixel %4, current kept %5").arg(tb->maximum()).arg(before).arg(after)
                  .arg(tree->verticalScrollMode() == QAbstractItemView::ScrollPerPixel).arg(tree->currentIndex() == current));
        const QPoint at = inTree(90, 300);
        const QModelIndex under = tree->indexAt(tree->viewport()->mapFrom(&w, at));
        hand.tap(at);
        settle(300);
        check(under.isValid() && under != current && tree->currentIndex() == under,
              QStringLiteral("the binder: a tap chooses the document under the finger"),
              QStringLiteral("'%1'").arg(tree->currentIndex().data().toString()));
        const double f0 = zoom::factor(QLatin1String(zoom::kBinder));
        hand.pinch(inTree(120, 300), 80, 107); // a third wider: three steps
        check(qAbs(zoom::factor(QLatin1String(zoom::kBinder)) - (f0 + 0.3)) < 0.01 && tree->currentIndex() == under,
              QStringLiteral("the binder: two fingers zoom it (its own zoom, as Ctrl + wheel)"),
              QStringLiteral("%1 -> %2").arg(f0).arg(zoom::factor(QLatin1String(zoom::kBinder))));
        zoom::setFactor(QLatin1String(zoom::kBinder), f0);
        shot(&w, QStringLiteral("finger-binder"));
    }

    // ---- buttons and menus answer at once
    {
        GripBar *tools = w.findChild<GripBar *>(toolbars::barObjectName(toolbars::Bar::Tools));
        QAction *inspector = w.findChild<QAction *>(QStringLiteral("showInspector"));
        auto *button = tools && inspector ? qobject_cast<QToolButton *>(tools->widgetForAction(inspector)) : nullptr;
        if (button && button->isVisible()) {
            const bool was = inspector->isChecked();
            const QPoint at = button->mapTo(&w, button->rect().center());
            hand.down(at);
            settle(30);
            const bool isDown = button->isDown();
            hand.up(at);
            settle(200);
            const bool toggled = inspector->isChecked() != was;
            hand.tap(button->mapTo(&w, button->rect().center()));
            settle(200);
            check(isDown && toggled && inspector->isChecked() == was,
                  QStringLiteral("a toolbar button is pressed the moment the finger is on it; a tap is one click"),
                  QStringLiteral("down %1, toggled %2, back %3").arg(isDown).arg(toggled).arg(inspector->isChecked() == was));
        } else {
            check(false, QStringLiteral("(the Inspector button is on the Tools toolbar)"));
        }
        QMenuBar *menus = w.menuBar();
        QAction *first = menus->actions().value(0);
        const QPoint at = menus->mapTo(&w, menus->actionGeometry(first).center());
        hand.tap(at);
        settle(300);
        auto *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        check(menu && menu == first->menu(), QStringLiteral("a tap on the menu bar opens the menu"));
        if (menu)
            menu->close();
        settle(200);
    }
}

// the lines between panes (splitter handles), which are too thin for a finger
static void dividerChecks(MainWindow &w)
{
    std::printf("== Fingers on the lines between panes\n");
    Hand hand;
    hand.dev = QTest::createTouchDevice();

    // ---- two lists side by side, without scroll bars (so that the lists reach the line)
    QWidget box;
    box.setWindowTitle(QStringLiteral("Panes"));
    auto *lay = new QVBoxLayout(&box);
    auto *split = new QSplitter(Qt::Horizontal);
    QListWidget *lists[2];
    for (QListWidget *&l : lists) {
        l = new QListWidget;
        for (int i = 0; i < 300; ++i)
            l->addItem(QStringLiteral("Line %1").arg(i + 1));
        l->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        l->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        split->addWidget(l);
    }
    auto *below = new QSplitter(Qt::Vertical);
    auto *upper = new QPlainTextEdit(QStringLiteral("upper"));
    auto *lower = new QPlainTextEdit(QStringLiteral("lower"));
    below->addWidget(upper);
    below->addWidget(lower);
    split->addWidget(below);
    lay->addWidget(split);
    box.move(0, 0);
    box.resize(1200, 600);
    box.show();
    box.activateWindow();
    (void)QTest::qWaitForWindowActive(&box, 2000);
    split->setSizes({400, 400, 400});
    below->setSizes({300, 300});
    settle(200);
    hand.win = &box;
    QSplitterHandle *line = split->handle(1);
    const auto lineX = [&] { return line->mapTo(&box, QPoint(0, 0)).x(); };
    const int y = split->mapTo(&box, QPoint(0, 0)).y() + 250;
    check(line->width() <= 8, QStringLiteral("(the line between the panes is a few pixels wide: %1)").arg(line->width()));

    {
        const int before = split->sizes().value(0), row = lists[0]->currentRow(), top = lists[0]->verticalScrollBar()->value();
        const int x = lineX() - 10; // on the left list, 10 px beside the line
        hand.swipe(QPoint(x, y), QPoint(x + 100, y + 6), 10, 10, 200);
        settle(150);
        const int now = split->sizes().value(0);
        check(qAbs(now - before - 100) <= 4 && lists[0]->currentRow() == row && lists[0]->verticalScrollBar()->value() == top,
              QStringLiteral("a finger put 10 px beside the line between two panes and moved 100 px across it takes the line along; "
                             "the list under the finger is neither clicked nor scrolled"),
              QStringLiteral("left pane %1 -> %2, row %3 -> %4").arg(before).arg(now).arg(row).arg(lists[0]->currentRow()));
        shot(&box, QStringLiteral("divider-moved"));
        const int x2 = lineX() + line->width() + 12; // from the other side, on the right list, and back
        hand.swipe(QPoint(x2, y), QPoint(x2 - 100, y - 5), 10, 10, 200);
        settle(150);
        check(qAbs(split->sizes().value(0) - before) <= 4, QStringLiteral("and back from the other side of the line"),
              QStringLiteral("left pane %1, was %2").arg(split->sizes().value(0)).arg(before));
    }
    {
        const QList<int> before = split->sizes();
        QScrollBar *bar = lists[0]->verticalScrollBar();
        bar->setValue(0);
        const int x = lineX() - 10;
        hand.swipe(QPoint(x, y + 150), QPoint(x + 5, y - 50), 10, 10, 250); // along the line: the list scrolls
        settle(150);
        check(split->sizes() == before && bar->value() >= 170 && bar->value() <= 210,
              QStringLiteral("a finger moved along the line scrolls the list under it; the panes keep their sizes"),
              QStringLiteral("scrolled %1").arg(bar->value()));
        bar->setValue(0);
        settle(50);
        hand.tap(QPoint(x, y));
        const QListWidgetItem *under = lists[0]->itemAt(lists[0]->viewport()->mapFrom(&box, QPoint(x, y)));
        check(under && lists[0]->currentItem() == under && split->sizes() == before,
              QStringLiteral("a tap beside the line chooses the list's line under the finger"),
              QStringLiteral("current %1").arg(lists[0]->currentRow()));
    }
    {
        const int before = split->sizes().value(0);
        const int x = lineX() - 8;
        hand.down(QPoint(x, y));
        QTest::qWait(450); // the finger rests first
        for (int k = 1; k <= 8; ++k) {
            QTest::qWait(10);
            hand.to(QPoint(x - 10 * k, y));
        }
        hand.up(QPoint(x - 80, y));
        settle(150);
        check(qAbs(split->sizes().value(0) - (before - 80)) <= 4, QStringLiteral("a finger that rests beside the line and then moves across it takes it along too"),
              QStringLiteral("left pane %1 -> %2").arg(before).arg(split->sizes().value(0)));
    }
    {
        const QList<int> before = split->sizes();
        const int x = lineX() - 60; // far from the line: a swipe sideways over a list is nothing
        hand.swipe(QPoint(x, y), QPoint(x + 100, y), 10, 10, 200);
        settle(150);
        check(split->sizes() == before, QStringLiteral("60 px away from the line a swipe moves no line"));
    }
    {
        // a lying line (panes one above the other): moved up and down
        QSplitterHandle *lying = below->handle(1);
        const QPoint at = lying->mapTo(&box, QPoint(lying->width() / 2, 0));
        const int before = below->sizes().value(0);
        hand.swipe(at + QPoint(0, lying->height() + 9), at + QPoint(4, lying->height() + 9 - 70), 10, 10, 200);
        settle(150);
        check(qAbs(below->sizes().value(0) - (before - 70)) <= 4,
              QStringLiteral("the line between two panes one above the other follows a finger moved up"),
              QStringLiteral("upper pane %1 -> %2").arg(before).arg(below->sizes().value(0)));
        const QList<int> sides = split->sizes();
        hand.swipe(at + QPoint(0, 9), at + QPoint(90, 12), 10, 10, 200); // along it: nothing moves
        settle(150);
        check(below->sizes().value(0) == before - 70 || qAbs(below->sizes().value(0) - (before - 70)) <= 1,
              QStringLiteral("a finger moved along that line moves nothing"));
        check(split->sizes() == sides, QStringLiteral("(the other panes kept their sizes)"));
    }
    box.close();
    settle(150);

    // ---- the program's own window: the line between the binder and the text
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    hand.win = &w;
    auto *binder = w.findChild<BinderPanel *>();
    QSplitterHandle *main = nullptr;
    for (QSplitterHandle *h : w.findChildren<QSplitterHandle *>()) {
        if (!h->isVisible() || h->orientation() != Qt::Horizontal || !binder || !h->splitter()->isAncestorOf(binder))
            continue;
        if (!main || h->mapTo(&w, QPoint(0, 0)).x() < main->mapTo(&w, QPoint(0, 0)).x())
            main = h;
    }
    check(main != nullptr, QStringLiteral("(the line between the binder and the text is found)"));
    if (main) {
        const int before = binder->width();
        const QPoint at = main->mapTo(&w, QPoint(main->width() + 9, main->height() / 2));
        hand.swipe(at, at + QPoint(90, 4), 10, 10, 200);
        settle(250);
        const int wide = binder->width();
        shot(&w, QStringLiteral("divider-binder-wider"));
        const QPoint at2 = main->mapTo(&w, QPoint(main->width() + 9, main->height() / 2));
        hand.swipe(at2, at2 - QPoint(90, 3), 10, 10, 200);
        settle(250);
        check(qAbs(wide - before - 90) <= 5 && qAbs(binder->width() - before) <= 5,
              QStringLiteral("the binder gets 90 px wider with a finger moved across its edge, and narrower again"),
              QStringLiteral("binder %1 -> %2 -> %3 (the line is %4 px wide)").arg(before).arg(wide).arg(binder->width()).arg(main->width()));
    }
}

// the handles of a text (the drop under the cursor, the two of a selection): when Qt is told to keep them away
static void handleChecks(MainWindow &w, BinderItem *longDoc)
{
    std::printf("== The handles of a text\n");
    Hand hand;
    hand.dev = QTest::createTouchDevice();
    hand.win = &w;
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    EditorPane *pane = nullptr;
    for (EditorPane *ep : w.findChildren<EditorPane *>())
        if (ep->isVisible())
            pane = ep;
    auto *binder = w.findChild<BinderPanel *>();
    if (pane && binder) {
        binder->select({longDoc}, false);
        pane->showItems({longDoc}, false, int(ViewMode::Single));
    }
    settle(500);
    TextColumnView *view = nullptr;
    if (pane)
        for (TextColumnView *v : pane->findChildren<TextColumnView *>())
            if (v->isVisible())
                view = v;
    TextEditor *ed = view ? view->editors().value(0) : nullptr;
    QScrollBar *bar = view ? view->verticalScrollBar() : nullptr;
    check(ed && bar && bar->maximum() > 2000, QStringLiteral("(the long text is open)"),
          QStringLiteral("maximum %1").arg(bar ? bar->maximum() : -1));
    if (!ed || !bar)
        return;
    const auto inText = [&](int x, int y) { return view->viewport()->mapTo(&w, QPoint(view->viewport()->width() / 2 + x, y)); };
    // what Qt's Android part asks the text before it shows a handle: "no handles now"?
    const auto away = [&] {
        QInputMethodQueryEvent q(Qt::ImHints);
        QCoreApplication::sendEvent(ed, &q);
        return bool(q.value(Qt::ImHints).toUInt() & Qt::ImhNoTextHandles);
    };
    bar->setValue(0);
    settle(150);

    hand.tap(inText(0, 300));
    settle(100);
    check(QApplication::focusWidget() == ed && !away(), QStringLiteral("after a tap into the text the cursor's handle may show"),
          QStringLiteral("focus on the text: %1").arg(QApplication::focusWidget() == ed));
    {
        // What Android's keyboards do about a second after a tap: the word at the cursor is marked as the one being
        // written (it comes again as "preedit" and replaces itself). That is no typing.
        QTextCursor c = ed->textCursor();
        c.setPosition(c.block().position() + 3); // inside "Paragraph"
        ed->setTextCursor(c);
        const QString before = ed->toPlainText();
        QInputMethodEvent mark(QStringLiteral("Paragraph"), {});
        mark.setCommitString(QString(), -3, 9);
        QCoreApplication::sendEvent(ed, &mark);
        const bool afterMark = !away();
        QInputMethodEvent back; // the cursor leaves the word: the keyboard hands it back as it was
        back.setCommitString(QStringLiteral("Paragraph"));
        QCoreApplication::sendEvent(ed, &back);
        settle(50);
        check(afterMark && !away() && ed->toPlainText() == before,
              QStringLiteral("the keyboard marking the word at the cursor, and handing it back unchanged, is no typing: the handle stays"),
              QStringLiteral("after the mark: shown %1; text unchanged %2").arg(afterMark).arg(ed->toPlainText() == before));
        QInputMethodEvent letter(QStringLiteral("q"), {}); // a letter typed on the on-screen keyboard
        QCoreApplication::sendEvent(ed, &letter);
        const bool gone = away();
        QInputMethodEvent undo; // (and taken back: an empty commit ends the marked letter)
        undo.setCommitString(QString());
        QCoreApplication::sendEvent(ed, &undo);
        settle(50);
        check(gone && ed->toPlainText() == before, QStringLiteral("a letter from the on-screen keyboard sends the cursor's handle away"),
              QStringLiteral("gone %1, text unchanged %2").arg(gone).arg(ed->toPlainText() == before));
        hand.tap(inText(0, 300));
        settle(100);
    }
    QTest::keyClick(ed, Qt::Key_A);
    settle(50);
    check(away(), QStringLiteral("a key sends the cursor's handle away at once"));
    QTest::keyClick(ed, Qt::Key_B);
    QTest::keyClick(ed, Qt::Key_Left);
    settle(50);
    check(away(), QStringLiteral("and it stays away while the cursor moves on with the typing"));

    hand.tap(inText(0, 500));
    settle(100);
    const bool shown = !away();
    hand.swipe(inText(0, 520), inText(0, 440), 8, 10, 250); // 80 px: the cursor is still in sight
    settle(150);
    check(shown && away(), QStringLiteral("a swipe over the text sends the cursor's handle away (the cursor still in sight)"),
          QStringLiteral("shown after the tap: %1").arg(shown));

    hand.tap(inText(0, 400));
    settle(100);
    const bool shown2 = !away();
    QTest::qWait(3400);
    check(shown2 && away(), QStringLiteral("left alone, the cursor's handle goes after three seconds"));

    // a selection: its handles show without a tap, as long as one of its ends is in sight
    bar->setValue(0);
    settle(150);
    QTextCursor c = ed->cursorForPosition(ed->viewport()->mapFrom(&w, inText(0, 300)));
    c.select(QTextCursor::WordUnderCursor);
    ed->setTextCursor(c);
    settle(100);
    const bool selShown = c.hasSelection() && !away();
    bar->setValue(1500);
    settle(150);
    const bool selGone = away();
    bar->setValue(0);
    settle(150);
    check(selShown && selGone && !away(),
          QStringLiteral("the handles of a selection show while it is in sight, stay away while it is scrolled out of sight, and "
                         "show again when it comes back"),
          QStringLiteral("in sight %1, scrolled out: away %2, back: away %3").arg(selShown).arg(selGone).arg(away()));

    // where the text stands in the window is told to Qt when the text is moved inside its scroll area
    {
        bar->setValue(700);
        settle(200);
        const QPoint at = ed->mapTo(&w, QPoint(0, 0));
        const QTransform told = QGuiApplication::inputMethod()->inputItemTransform();
        check(QGuiApplication::focusObject() == ed && qRound(told.dx()) == at.x() && qRound(told.dy()) == at.y(),
              QStringLiteral("Qt knows where the text stands after it was scrolled (the handles follow the text)"),
              QStringLiteral("told %1, %2; the text is at %3, %4").arg(told.dx()).arg(told.dy()).arg(at.x()).arg(at.y()));
        bar->setValue(0);
        settle(150);
    }
    QTextCursor none = ed->textCursor();
    none.clearSelection();
    ed->setTextCursor(none);
}

// lists, plain texts, pages of controls and combo boxes anywhere in the program (dialogs, options)

// ------------------------------------------------------------------ the menu of what is under the finger

static void menuChecks(MainWindow &w, BinderItem *longDoc)
{
    std::printf("== Press, hold and lift: the menu of a right click; a tap on selected text\n");
    Hand hand;
    hand.dev = QTest::createTouchDevice();
    QWidget box;
    box.setWindowTitle(QStringLiteral("Menus"));
    auto *lay = new QHBoxLayout(&box);
    auto *list = new QListWidget;
    for (int i = 0; i < 300; ++i)
        list->addItem(QStringLiteral("Line %1").arg(i + 1));
    auto *button = new QPushButton(QStringLiteral("Button"));
    auto *typed = new QTextEdit;
    typed->setPlainText(QStringLiteral("The harbor lay quiet under the morning fog.\nThe gulls kept their distance.\n\n\n\n"));
    auto *shown = new QTextEdit;
    shown->setReadOnly(true);
    shown->setPlainText(QStringLiteral("A text that is only shown."));
    int listMenus = 0, buttonMenus = 0, typedMenus = 0, shownMenus = 0, clicks = 0;
    QPoint listAt, typedAt;
    for (QWidget *x : {static_cast<QWidget *>(list), static_cast<QWidget *>(button), static_cast<QWidget *>(typed),
                       static_cast<QWidget *>(shown)})
        x->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(list, &QWidget::customContextMenuRequested, &box, [&](const QPoint &p) {
        ++listMenus;
        listAt = p;
    });
    QObject::connect(button, &QWidget::customContextMenuRequested, &box, [&] { ++buttonMenus; });
    QObject::connect(typed, &QWidget::customContextMenuRequested, &box, [&](const QPoint &p) {
        ++typedMenus;
        typedAt = p;
    });
    QObject::connect(shown, &QWidget::customContextMenuRequested, &box, [&] { ++shownMenus; });
    QObject::connect(button, &QPushButton::clicked, &box, [&] { ++clicks; });
    lay->addWidget(list);
    lay->addWidget(button);
    lay->addWidget(typed);
    lay->addWidget(shown);
    box.resize(1100, 420);
    box.move(40, 40);
    box.show();
    box.activateWindow();
    (void)QTest::qWaitForWindowActive(&box, 2000);
    settle(200);
    hand.win = &box;
    const auto in = [&](QWidget *x, const QPoint &p) { return x->mapTo(&box, p); };
    const auto hold = [&](const QPoint &p, int ms, const QPoint &lift = QPoint(-1, -1)) {
        hand.down(p);
        QTest::qWait(ms);
        if (lift.x() >= 0) {
            hand.to((p + lift) / 2);
            QTest::qWait(15);
            hand.to(lift);
            QTest::qWait(15);
        }
        hand.up(lift.x() >= 0 ? lift : p);
        settle(150);
    };

    // a list
    const QPoint line3 = in(list->viewport(), list->visualItemRect(list->item(2)).center());
    hold(line3, 620);
    check(listMenus == 1 && list->currentRow() == 2,
          QStringLiteral("A finger that rests on a line of a list and goes up: the line is chosen and its menu asked for"),
          QStringLiteral("menus %1, line %2").arg(listMenus).arg(list->currentRow() + 1));
    check((listAt - list->visualItemRect(list->item(2)).center()).manhattanLength() <= 2,
          QStringLiteral("The menu is asked for at the finger's place"),
          QStringLiteral("at %1,%2").arg(listAt.x()).arg(listAt.y()));
    hand.tap(in(list->viewport(), list->visualItemRect(list->item(4)).center()));
    check(listMenus == 1 && list->currentRow() == 4, QStringLiteral("A tap asks for no menu"),
          QStringLiteral("menus %1").arg(listMenus));
    hold(in(list->viewport(), list->visualItemRect(list->item(1)).center()), 380);
    check(listMenus == 1, QStringLiteral("A finger that rests only a third of a second asks for no menu"),
          QStringLiteral("menus %1").arg(listMenus));
    hold(line3, 620, line3 + QPoint(0, 60));
    check(listMenus == 1, QStringLiteral("A finger that rests and then moves away drags and asks for no menu"),
          QStringLiteral("menus %1").arg(listMenus));
    // a button keeps its press
    hold(in(button, button->rect().center()), 620);
    check(clicks == 1 && buttonMenus == 0, QStringLiteral("A button pressed long is still pressed once, and no menu is asked for"),
          QStringLiteral("clicks %1, menus %2").arg(clicks).arg(buttonMenus));
    // a text that is only shown
    hold(in(shown->viewport(), QPoint(40, 12)), 620);
    check(shownMenus == 1, QStringLiteral("A text that is only shown: resting on it asks for its menu"),
          QStringLiteral("menus %1").arg(shownMenus));
    // a text that can be typed in: the long press is Android's own there
    hold(in(typed->viewport(), QPoint(60, 12)), 620);
    check(typedMenus == 0, QStringLiteral("A text that can be typed in: resting on it asks for no menu (Android selects the word there)"),
          QStringLiteral("menus %1").arg(typedMenus));
    // ... and a tap on selected text opens the text's menu
    QTextCursor c = typed->textCursor();
    c.setPosition(4);
    c.setPosition(10, QTextCursor::KeepAnchor); // "harbor"
    typed->setTextCursor(c);
    settle(60);
    QTextCursor mid(typed->document());
    mid.setPosition(7);
    const QPoint onWord = typed->cursorRect(mid).center();
    hand.tap(in(typed->viewport(), onWord));
    check(typedMenus == 1 && typed->textCursor().selectedText() == QLatin1String("harbor"),
          QStringLiteral("A tap on selected text asks for the text's menu, and the selection stays"),
          QStringLiteral("menus %1, selected \"%2\"").arg(typedMenus).arg(typed->textCursor().selectedText()));
    check((typedAt - onWord).manhattanLength() <= 2, QStringLiteral("... at the finger's place"),
          QStringLiteral("at %1,%2").arg(typedAt.x()).arg(typedAt.y()));
    // beside the selection, in the same line and far below the text: an ordinary tap
    QTextCursor later(typed->document());
    later.setPosition(30);
    hand.tap(in(typed->viewport(), typed->cursorRect(later).center()));
    check(typedMenus == 1 && !typed->textCursor().hasSelection() && qAbs(typed->textCursor().position() - 30) <= 1,
          QStringLiteral("A tap beside the selection puts the cursor there, as before"),
          QStringLiteral("menus %1, cursor at %2").arg(typedMenus).arg(typed->textCursor().position()));
    c.setPosition(4);
    c.setPosition(10, QTextCursor::KeepAnchor);
    typed->setTextCursor(c);
    settle(60);
    hand.tap(in(typed->viewport(), QPoint(typed->viewport()->width() - 30, typed->viewport()->height() - 20)));
    check(typedMenus == 1, QStringLiteral("A tap far from the selection (the empty part of the page) asks for no menu"),
          QStringLiteral("menus %1").arg(typedMenus));
    // a menu of the program is open: no handles on it (Android draws them above every window of the app)
    {
        c.setPosition(4);
        c.setPosition(10, QTextCursor::KeepAnchor);
        typed->setTextCursor(c);
        typed->setFocus();
        settle(100);
        const auto hidden = [&] {
            QInputMethodQueryEvent q(Qt::ImHints);
            QCoreApplication::sendEvent(typed, &q);
            return bool(q.value(Qt::ImHints).toUInt() & Qt::ImhNoTextHandles);
        };
        const bool before = hidden();
        QMenu popup(&box);
        popup.addAction(QStringLiteral("One"));
        popup.addAction(QStringLiteral("Two"));
        popup.popup(box.mapToGlobal(QPoint(300, 200)));
        settle(200);
        const bool during = hidden();
        popup.close();
        settle(200);
        const bool after = hidden();
        check(!before && during && !after,
              QStringLiteral("While a menu of the program is open, the selection's handles are hidden; they come back with its end"),
              QStringLiteral("hidden before %1, during %2, after %3").arg(before).arg(during).arg(after));
    }
    box.hide();
    settle(100);

    // the program's own window: a document in the binder, and selected text in the main text
    std::printf("== The program's menus by finger\n");
    hand.win = &w;
    w.activateWindow();
    settle(200);
    auto *binder = w.findChild<BinderPanel *>();
    QTreeView *tree = binder ? binder->tree() : nullptr;
    check(tree != nullptr, QStringLiteral("(the binder's tree)"));
    if (tree) {
        tree->scrollToTop();
        settle(100);
        const QModelIndex row = tree->indexAt(QPoint(60, 60));
        QString menuTitle;
        int menus = 0;
        // the menu opens with a loop of its own: close it from a timer and note that it was there
        QTimer closer;
        closer.setInterval(120);
        QObject::connect(&closer, &QTimer::timeout, &w, [&] {
            if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                ++menus;
                menuTitle = QString::number(m->actions().size()) + QStringLiteral(" entries");
                shot(m, QStringLiteral("mobile-menu-binder"));
                m->close();
            }
        });
        closer.start();
        const QPoint at = tree->viewport()->mapTo(&w, tree->visualRect(row).center());
        hand.down(at);
        QTest::qWait(620);
        hand.up(at);
        settle(600);
        check(menus == 1, QStringLiteral("A finger that rests on a document in the binder and goes up opens the binder's menu"),
              QStringLiteral("(menus %1, %2)").arg(menus).arg(menuTitle));

        // the main text: select a word, tap on it
        EditorPane *pane = nullptr;
        for (EditorPane *ep : w.findChildren<EditorPane *>())
            if (ep->isVisible())
                pane = ep;
        if (pane) {
            binder->select({longDoc}, false);
            pane->showItems({longDoc}, false, int(ViewMode::Single));
        }
        settle(500);
        TextColumnView *view = nullptr;
        if (pane)
            for (TextColumnView *v : pane->findChildren<TextColumnView *>())
                if (v->isVisible())
                    view = v;
        TextEditor *ed = view ? view->editors().value(0) : nullptr;
        if (view)
            view->verticalScrollBar()->setValue(0);
        settle(100);
        check(ed != nullptr, QStringLiteral("(the main text)"));
        if (ed) {
            QTextCursor word = ed->textCursor();
            word.setPosition(0);
            word.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor, 4);
            word.select(QTextCursor::WordUnderCursor);
            ed->setTextCursor(word);
            settle(100);
            QTextCursor inside(ed->document());
            inside.setPosition((word.selectionStart() + word.selectionEnd()) / 2);
            const QPoint p = ed->viewport()->mapTo(&w, ed->cursorRect(inside).center());
            menus = 0;
            const QString before = ed->textCursor().selectedText();
            QObject::disconnect(&closer, nullptr, nullptr, nullptr);
            QObject::connect(&closer, &QTimer::timeout, &w, [&] {
                if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                    ++menus;
                    menuTitle = QString::number(m->actions().size()) + QStringLiteral(" entries");
                    shot(&w, QStringLiteral("mobile-menu-text"));
                    m->close();
                }
            });
            hand.tap(p);
            settle(600);
            check(menus == 1 && ed->textCursor().selectedText() == before && !before.isEmpty(),
                  QStringLiteral("A tap on the selected word of the main text opens the text's menu; the word stays selected"),
                  QStringLiteral("(menus %1, %2, selected \"%3\")").arg(menus).arg(menuTitle, ed->textCursor().selectedText()));
        }
        closer.stop();
    }
}

// ------------------------------------------------------------------ folders, dictionaries, the start

static void tabletChecks(MainWindow &w, BinderItem *longDoc)
{
    std::printf("== Where the work is kept\n");
    QTemporaryDir tmp;
    const QString root = tmp.path();
    const QString docs = root + QStringLiteral("/shared/Documents");
    const QString own = root + QStringLiteral("/own");
    const QString elsewhere = root + QStringLiteral("/shared/Writing");
    QDir().mkpath(docs);
    QDir().mkpath(own + QStringLiteral("/files/Documents"));
    QDir().mkpath(elsewhere);
    const mobile::Folders f = mobile::foldersIn(docs);
    check(f.projects == docs + QStringLiteral("/Quillbinder") && f.backups == docs + QStringLiteral("/Quillbinder/Backups"),
          QStringLiteral("Projects go to Documents/Quillbinder, backups to Documents/Quillbinder/Backups"),
          f.projects + QStringLiteral(" | ") + f.backups);
    AppSettings &as = AppSettings::instance();
    const QString keepLast = as.lastProjectDir, keepWelcome = as.welcomeFolder, keepBackup = as.backupDir;
    QSettings st;
    as.lastProjectDir.clear();
    as.welcomeFolder = own + QStringLiteral("/files/Documents");
    as.backupDir = own + QStringLiteral("/files/Quillbinder/Backups");
    st.remove(QStringLiteral("export/location"));
    st.setValue(QStringLiteral("tools/scratch-pad/folder"), elsewhere + QStringLiteral("/Notes"));
    bool changed = mobile::settleFolders(f, {own}, true);
    check(changed && as.lastProjectDir == f.projects && as.welcomeFolder == f.projects && as.backupDir == f.backups
              && st.value(QStringLiteral("export/location")).toString() == f.exports,
          QStringLiteral("With the access: settings that are empty or name the app's own folders move to the shared folder"),
          QStringLiteral("%1 | %2 | %3").arg(as.lastProjectDir, as.welcomeFolder, as.backupDir));
    check(st.value(QStringLiteral("tools/scratch-pad/folder")).toString() == elsewhere + QStringLiteral("/Notes"),
          QStringLiteral("A folder the writer chose stays"));
    check(QFileInfo(f.projects).isDir() && QFileInfo(f.backups).isDir(), QStringLiteral("The two folders are made"));
    check(!mobile::settleFolders(f, {own}, true), QStringLiteral("A second start changes nothing"));
    as.lastProjectDir = elsewhere;
    changed = mobile::settleFolders(f, {own}, false);
    check(changed && as.lastProjectDir == elsewhere && as.welcomeFolder.isEmpty() && as.backupDir == backup::defaultDirectory()
              && !st.contains(QStringLiteral("export/location")),
          QStringLiteral("Without the access: settings inside the shared folder go back to the program's standard, the writer's own stay"),
          QStringLiteral("%1 | %2 | %3").arg(as.lastProjectDir, as.welcomeFolder, as.backupDir));

    // the window
    {
        bool allowed = false;
        int asked = 0;
        mobile::FilesSystem sys;
        sys.documents = docs;
        sys.allowed = [&] { return allowed; };
        sys.ask = [&] {
            ++asked;
            QTimer::singleShot(250, &w, [&] { allowed = true; }); // (the writer switches it on in Android's settings)
        };
        QString text;
        QSize size;
        bool keepShown = false;
        QTimer::singleShot(300, &w, [&] {
            auto *d = QApplication::activeModalWidget();
            if (!d || d->objectName() != QLatin1String("filesOnTablet"))
                return;
            if (auto *label = d->findChild<QLabel *>(QStringLiteral("filesText")))
                text = label->text();
            if (auto *keep = d->findChild<QPushButton *>(QStringLiteral("filesKeepInApp")))
                keepShown = keep->isVisible();
            size = d->size();
            shot(d, QStringLiteral("mobile-files-window"));
            if (auto *allow = d->findChild<QPushButton *>(QStringLiteral("filesAllow")))
                allow->click();
        });
        QElapsedTimer t;
        t.start();
        const bool result = mobile::askForFiles(&w, sys);
        check(result && asked == 1 && t.elapsed() < 4000,
              QStringLiteral("The window sends the writer to Android's settings and closes by itself once the access is given"),
              QStringLiteral("(asked %1 time(s), closed after %2 ms)").arg(asked).arg(t.elapsed()));
        check(text.contains(QStringLiteral("Documents/Quillbinder")) && text.contains(QStringLiteral("Documents/Quillbinder/Backups"))
                  && text.contains(QStringLiteral("all files")) && keepShown,
              QStringLiteral("It names the two folders, the access it needs and the way without"), text.left(300));
        check(size.width() <= 760 && size.height() <= 560,
              QStringLiteral("It fits a tablet's screen held either way (800 units wide; 600 high above the keyboard)"),
              QStringLiteral("(%1 x %2)").arg(size.width()).arg(size.height()));
        allowed = false;
        asked = 0;
        QTimer::singleShot(300, &w, [&] {
            if (auto *d = QApplication::activeModalWidget())
                if (auto *keep = d->findChild<QPushButton *>(QStringLiteral("filesKeepInApp")))
                    keep->click();
        });
        check(!mobile::askForFiles(&w, sys) && asked == 0,
              QStringLiteral("\"Keep Everything Inside the App\" closes it without asking Android"));
    }

    // file windows
    {
        const QString standard = own + QStringLiteral("/files/Documents");
        const QString download = root + QStringLiteral("/shared/Download");
        QDir().mkpath(download);
        mobile::watchFileWindows(f, {f.projects, docs, download, root + QStringLiteral("/not there")}, {standard});
        QFileDialog open(&w, QStringLiteral("Open"), standard);
        open.setOption(QFileDialog::DontUseNativeDialog);
        open.show();
        settle(300);
        QStringList side;
        for (const QUrl &u : open.sidebarUrls())
            side << u.toLocalFile();
        check(QDir::cleanPath(open.directory().absolutePath()) == f.projects,
              QStringLiteral("A file window that would open in the app's own Documents folder opens in the projects folder"),
              open.directory().absolutePath());
        check(side == QStringList({f.projects, docs, download}),
              QStringLiteral("The shared folders stand at its side (only those that exist)"), side.join(QStringLiteral(" | ")));
        shot(&open, QStringLiteral("mobile-file-window"));
        open.close();
        QFileDialog save(&w, QStringLiteral("Save"), standard);
        save.setOption(QFileDialog::DontUseNativeDialog);
        save.setAcceptMode(QFileDialog::AcceptSave);
        save.selectFile(QStringLiteral("The Harbor.pdf"));
        save.show();
        settle(300);
        const QStringList chosen = save.selectedFiles();
        check(chosen.size() == 1 && chosen.constFirst() == f.projects + QStringLiteral("/The Harbor.pdf"),
              QStringLiteral("A proposed file name stays when the folder changes"), chosen.join(QLatin1Char(' ')));
        save.close();
        QFileDialog other(&w, QStringLiteral("Open"), elsewhere);
        other.setOption(QFileDialog::DontUseNativeDialog);
        other.show();
        settle(300);
        check(QDir::cleanPath(other.directory().absolutePath()) == elsewhere,
              QStringLiteral("A file window opened in another folder stays there"), other.directory().absolutePath());
        other.close();
        mobile::watchFileWindows(f, {}, {});
    }
    as.lastProjectDir = keepLast;
    as.welcomeFolder = keepWelcome;
    as.backupDir = keepBackup;
    st.remove(QStringLiteral("export/location"));
    st.remove(QStringLiteral("tools/scratch-pad/folder"));
    as.save();

    std::printf("== Dictionaries, saving, commands\n");
    check(mobile::spellLanguageFor(QStringLiteral("de_AT")) == QLatin1String("de_AT")
              && mobile::spellLanguageFor(QStringLiteral("de_LU")) == QLatin1String("de_DE")
              && mobile::spellLanguageFor(QStringLiteral("en_AU")) == QLatin1String("en_GB")
              && mobile::spellLanguageFor(QStringLiteral("en_US")) == QLatin1String("en_US")
              && mobile::spellLanguageFor(QStringLiteral("fr_FR")).isEmpty(),
          QStringLiteral("The first spelling language follows the tablet's language where a dictionary comes with the program"));
    {
        const QString from = root + QStringLiteral("/apk"), to = root + QStringLiteral("/data/dictionaries");
        QDir().mkpath(from);
        const auto put = [](const QString &path, const QByteArray &data) {
            QFile file(path);
            if (file.open(QIODevice::WriteOnly))
                file.write(data);
        };
        put(from + QStringLiteral("/en_GB.aff"), "SET UTF-8\n");
        put(from + QStringLiteral("/en_GB.dic"), "2\nharbor\nquay\n");
        put(from + QStringLiteral("/de_DE.aff"), "SET ISO8859-1\n");
        put(from + QStringLiteral("/de_DE.dic"), "1\nHafen\n");
        put(from + QStringLiteral("/README.txt"), "not a dictionary");
        const int first = mobile::copyDictionaries(from, to);
        check(first == 4 && QFileInfo::exists(to + QStringLiteral("/de_DE.dic")) && !QFileInfo::exists(to + QStringLiteral("/README.txt")),
              QStringLiteral("The dictionary files are copied out of the package into the data folder"),
              QStringLiteral("copied %1").arg(first));
        check(mobile::copyDictionaries(from, to) == 0, QStringLiteral("A later start copies nothing"));
        put(from + QStringLiteral("/en_GB.dic"), "3\nharbor\nquay\ngull\n");
        const int again = mobile::copyDictionaries(from, to);
        QFile now(to + QStringLiteral("/en_GB.dic"));
        check(again == 1 && now.open(QIODevice::ReadOnly) && now.readAll().contains("gull"),
              QStringLiteral("A dictionary that changed with a new version is copied anew"), QStringLiteral("copied %1").arg(again));
        put(root + QStringLiteral("/in-the-way"), "x");
        check(mobile::copyDictionaries(from, root + QStringLiteral("/in-the-way/dictionaries")) == -1,
              QStringLiteral("A data folder that cannot be written is told (not taken for done)"));
    }
    {
        QTextCursor c(w.project()->docs()->text(longDoc->id));
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" Zanzibarquay."));
        settle(100);
        const bool dirty = w.project()->hasUnsavedChanges();
        const bool saved = mobile::saveAllNow(&w);
        check(dirty && saved && !w.project()->hasUnsavedChanges(),
              QStringLiteral("Put away by Android: the open project is saved in full at once"),
              QStringLiteral("unsaved before: %1, saved: %2").arg(dirty).arg(saved));
    }
    mobile::startTablet(&w);
    auto *install = w.findChild<QAction *>(QStringLiteral("installPackage"));
    check(install && !install->isVisible(), QStringLiteral("File > Install Update or Add-on is not offered on a tablet (one full package)"));
}

static void widgetChecks()
{
    std::printf("== Fingers on other lists, texts and pages\n");
    Hand hand;
    hand.dev = QTest::createTouchDevice();
    QWidget box;
    box.setWindowTitle(QStringLiteral("Fingers"));
    auto *lay = new QHBoxLayout(&box);
    auto *list = new QListWidget;
    for (int i = 0; i < 300; ++i)
        list->addItem(QStringLiteral("Line %1").arg(i + 1));
    auto *plain = new QPlainTextEdit;
    {
        QStringList lines;
        for (int i = 0; i < 400; ++i)
            lines << QStringLiteral("Plain line %1").arg(i + 1);
        plain->setPlainText(lines.join(QLatin1Char('\n')));
    }
    auto *shortList = new QListWidget; // nothing to scroll; its lines can be moved by dragging
    for (int i = 0; i < 6; ++i)
        shortList->addItem(QStringLiteral("Short %1").arg(i + 1));
    shortList->setDragDropMode(QAbstractItemView::InternalMove);
    shortList->setFixedWidth(140);
    auto *area = new QScrollArea;
    auto *page = new QWidget;
    auto *pl = new QVBoxLayout(page);
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(10);
    slider->setFixedWidth(260);
    auto *button = new QPushButton(QStringLiteral("Button"));
    auto *combo = new QComboBox;
    for (int i = 0; i < 80; ++i)
        combo->addItem(QStringLiteral("Choice %1").arg(i + 1));
    combo->setMaxVisibleItems(12);
    pl->addWidget(slider);
    pl->addWidget(button);
    pl->addWidget(combo);
    for (int i = 0; i < 60; ++i)
        pl->addWidget(new QPushButton(QStringLiteral("Filler %1").arg(i + 1)));
    area->setWidget(page);
    area->setWidgetResizable(true);
    lay->addWidget(list);
    lay->addWidget(shortList);
    lay->addWidget(plain);
    lay->addWidget(area);
    int clicks = 0;
    QObject::connect(button, &QPushButton::clicked, &box, [&clicks] { ++clicks; });
    box.move(0, 0);
    box.resize(1300, 600);
    box.show();
    box.activateWindow();
    (void)QTest::qWaitForWindowExposed(&box, 2000);
    settle(300);
    hand.win = &box;
    const auto in = [&](QWidget *wd, int x, int y) { return wd->mapTo(&box, QPoint(x, y)); };

    {
        QScrollBar *bar = list->verticalScrollBar();
        hand.swipe(in(list->viewport(), 100, 400), in(list->viewport(), 100, 200), 12, 10, 250);
        settle(150);
        const int scrolled = bar->value();
        const QPoint at = in(list->viewport(), 100, 300);
        const int row = list->row(list->itemAt(list->viewport()->mapFrom(&box, at)));
        const int currentBefore = list->currentRow();
        hand.tap(at);
        check(list->verticalScrollMode() == QAbstractItemView::ScrollPerPixel && scrolled >= 160 && scrolled <= 200 && currentBefore <= 0
                  && row > 5 && list->currentRow() == row,
              QStringLiteral("a list: the swipe scrolls it by pixels and chooses nothing, the tap chooses the line"),
              QStringLiteral("scrolled %1, current before %2, tapped row %3, current %4").arg(scrolled).arg(currentBefore).arg(row)
                  .arg(list->currentRow()));
    }
    {
        QStringList before;
        for (int i = 0; i < shortList->count(); ++i)
            before << shortList->item(i)->text();
        QWidget *vp = shortList->viewport();
        const QRect first = shortList->visualItemRect(shortList->item(0)), fourth = shortList->visualItemRect(shortList->item(3));
        hand.swipe(in(vp, 50, first.center().y()), in(vp, 50, fourth.center().y()), 10, 10, 60); // the first line "taken" down
        settle(200);
        QStringList after;
        for (int i = 0; i < shortList->count(); ++i)
            after << shortList->item(i)->text();
        const bool swipeKept = after == before;
        check(swipeKept && shortList->verticalScrollBar()->maximum() == 0,
              QStringLiteral("a short list whose lines can be moved: a swipe takes no line along"),
              QStringLiteral("%1").arg(after.join(QLatin1String(", "))));
    }
    {
        QScrollBar *bar = plain->verticalScrollBar();
        const int line = QFontMetrics(plain->font()).lineSpacing();
        hand.swipe(in(plain->viewport(), 100, 400), in(plain->viewport(), 100, 200), 12, 10, 250);
        settle(150);
        const int lines = bar->value();
        check(qAbs(lines * line - 180) <= 2 * line + 20 && !plain->textCursor().hasSelection(),
              QStringLiteral("a plain text (it scrolls by lines): the swipe scrolls it about as far as the finger went"),
              QStringLiteral("%1 lines of %2 px").arg(lines).arg(line));
    }
    {
        QScrollBar *bar = area->verticalScrollBar();
        // downward over the slider: the page scrolls, the slider stays
        const QPoint onSlider = in(slider, 130, slider->height() / 2);
        hand.swipe(onSlider + QPoint(0, 200), onSlider + QPoint(0, 60), 10, 10, 250);
        settle(150);
        const int scrolled = bar->value();
        bar->setValue(0);
        settle(100);
        const int v0 = slider->value();
        // sideways on the slider: nothing scrolls that way, so it is a mouse drag of the slider
        const QPoint a = in(slider, 30, slider->height() / 2);
        hand.swipe(a, a + QPoint(180, 2), 12, 10, 100);
        settle(150);
        const int v1 = slider->value();
        hand.tap(in(button, button->width() / 2, button->height() / 2));
        settle(150);
        check(area->horizontalScrollBar()->maximum() == 0 && scrolled >= 100 && scrolled <= 140 && bar->value() == 0 && v1 > v0 + 30
                  && clicks == 1,
              QStringLiteral("a page of controls: a swipe scrolls the page, a finger moved sideways on a slider moves the slider, a "
                             "tap on a button is one click"),
              QStringLiteral("page scrolled %1, slider %2 -> %3, clicks %4").arg(scrolled).arg(v0).arg(v1).arg(clicks));
    }
    {
        hand.tap(in(combo, combo->width() / 2, combo->height() / 2));
        settle(400);
        QWidget *popup = QApplication::activePopupWidget();
        auto *popupList = popup ? popup->findChild<QAbstractItemView *>() : nullptr;
        bool opened = popup && popupList, scrolledOk = false, chosen = false;
        int scrolled = 0, row = -1;
        if (opened) {
            Hand h2 = hand;
            h2.win = popup;
            QWidget *vp = popupList->viewport();
            const auto inPopup = [&](int x, int y) { return vp->mapTo(popup, QPoint(x, y)); };
            const int mid = vp->height() / 2;
            h2.swipe(inPopup(60, mid + 80), inPopup(60, mid - 80), 10, 10, 250);
            settle(200);
            scrolled = popupList->verticalScrollBar()->value();
            scrolledOk = scrolled > 0 && QApplication::activePopupWidget() == popup;
            const QPoint at = inPopup(60, mid);
            row = popupList->indexAt(vp->mapFrom(popup, at)).row();
            h2.tap(at);
            settle(400);
            chosen = QApplication::activePopupWidget() == nullptr && row > 0 && combo->currentIndex() == row;
        }
        if (QWidget *still = QApplication::activePopupWidget())
            still->close();
        check(opened && scrolledOk && chosen,
              QStringLiteral("a combo box: a tap opens its list, a swipe scrolls the list and leaves it open, a tap chooses"),
              QStringLiteral("opened %1, scrolled %2, row %3, chosen %4 (current %5)").arg(opened).arg(scrolled).arg(row).arg(chosen)
                  .arg(combo->currentIndex()));
    }
    shot(&box, QStringLiteral("finger-widgets"));
    box.close();
    settle(100);
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderMobileTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderMobileTest"));
    QApplication app(argc, argv);
    if (argc > 1) {
        shotDir = QString::fromLocal8Bit(argv[1]);
        QDir().mkpath(shotDir);
    }
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.save();
    theme::apply();
    compile::PluginManager::instance().load();
    zoom::install();
    mobile::installTouch();

    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("The Harbor"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    // a long text and a binder long enough to scroll
    BinderItem *longDoc = proj->model()->createItem(proj->draft(), -1, ItemType::Text, QStringLiteral("The Long One"));
    {
        QTextCursor c(proj->docs()->text(longDoc->id));
        for (int i = 0; i < 260; ++i) {
            if (i > 0)
                c.insertBlock(display::normalBlockFormat(proj->formatting), QTextCharFormat());
            c.insertText(QStringLiteral("Paragraph %1 of the long one: the harbor lay quiet under the morning fog, and the gulls "
                                        "kept their distance from the nets drying along the quay.").arg(i + 1));
        }
    }
    for (int i = 0; i < 70; ++i)
        proj->model()->createItem(proj->draft(), -1, ItemType::Text, QStringLiteral("Scene %1").arg(i + 1));

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.move(0, 0);
    w.resize(1900, 950);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);
    mobile::installTouch(); // (again, as the program does: before the filters the window made)

    barChecks(w);
    fingerChecks(w, longDoc);
    handleChecks(w, longDoc);
    dividerChecks(w);
    widgetChecks();
    menuChecks(w, longDoc);
    tabletChecks(w, longDoc);

    w.close();
    delete wp;
    settle(100);
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
