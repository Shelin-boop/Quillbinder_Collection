// Android 0.92.0: the tablet's windows in a preview on the desktop (android/MobileCare.cpp, the tablet mode of
// windowcare). Built only with -DQB_MOBILE_PREVIEW=ON; part of no test run of Linux or Windows.
//
// The screen of the preview is the tablet's usable area: run it once per size (tools/run-tablet-fit.sh does it):
//   QT_FONT_DPI=72 xvfb-run -a -s "-screen 0 1280x720x24" ./qb_mobile_fit_tests /tmp/shots
// (72 dpi: Android's measure for the style; Roboto 14 px, Android's font, when it is installed.)
//
//  - Every menu of the menu bar, opened by a click on its title: inside the screen, below the menu bar (not over it),
//    in one column, every entry reachable.
//  - Every window a command opens (the commands with "..." and the Help menu): inside the screen, its buttons inside
//    it, a button that closes it, and Android's Back closes it.
//  - About: a window whose text scrolls, its buttons in view.
//  - Back: closes a menu, a tip, a window; with the main window alone it is left to Android.
//  - The keys Ctrl and Shift held while the mouse clicks (Qt 6.12.0 for Android sends the mouse without them): Ctrl +
//    click and Shift + click choose several documents, Ctrl + wheel zooms the binder.
//  - The line between the binder and the text: a mouse pressed beside it takes it along; the binder's width can be
//    changed at every size, upright too.
//  - The pin of the fixed mind map on the selected row, by a click on it and by the row's menu; a tap on a row's
//    padlock shows its text.

#include "android/Mobile.h"
#include "compile/PluginManager.h"
#include "core/Addons.h"
#include "core/BinderItem.h"
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/UpdateCheck.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/MainWindow.h"
#include "ui/Theme.h"
#include "ui/WindowCare.h"
#include "ui/Zoom.h"

#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSplitter>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QTimer>
#include <QToolTip>
#include <QWheelEvent>
#include <QWindow>

#include <cstdio>
#include <functional>

using namespace qb;

static int failures = 0;
static QString shotDir;
static QString sizeName;

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
    w->grab().save(shotDir + QLatin1Char('/') + sizeName + QLatin1Char('-') + name + QStringLiteral(".png"));
}

static QString rectText(const QRect &r)
{
    return QStringLiteral("%1,%2 %3x%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
}

static QRect screenArea() { return QGuiApplication::primaryScreen()->availableGeometry(); }

// Android's Back key as Qt for Android delivers it (a press, then a release); true when the press was taken
static bool back(QWidget *to)
{
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Back, Qt::NoModifier);
    QApplication::sendEvent(to, &press);
    const bool taken = press.isAccepted();
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Back, Qt::NoModifier);
    QApplication::sendEvent(to, &release);
    return taken;
}

// the window a command opened: the modal one, else the newest visible window that is neither the main window nor a
// popup
static QWidget *frontWindow(QWidget *main)
{
    if (QWidget *m = QApplication::activeModalWidget())
        return m;
    QWidget *best = nullptr;
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (w == main || !w->isVisible() || (w->windowFlags() & Qt::Popup) == Qt::Popup || w->windowType() == Qt::ToolTip)
            continue;
        if (w->windowType() != Qt::Dialog && w->windowType() != Qt::Window && w->windowType() != Qt::Sheet)
            continue;
        best = w;
    }
    return best;
}

// ------------------------------------------------------------------ windows

struct Seen {
    QString command, window, why;
    bool fits = false, buttonsIn = false, closable = false, byBack = false;
};

static bool buttonInside(const QAbstractButton *b, const QRect &area)
{
    // a button inside a part that scrolls is reached by scrolling: what counts is that part's place
    for (QWidget *p = b->parentWidget(); p && !p->isWindow(); p = p->parentWidget())
        if (auto *sa = qobject_cast<QAbstractScrollArea *>(p))
            return area.intersects(QRect(sa->mapToGlobal(QPoint(0, 0)), sa->size()));
    const QRect r(b->mapToGlobal(QPoint(0, 0)), b->size());
    return area.contains(r);
}

// measures the window in front and closes it with Back (or by force when Back does not)
static Seen measure(QWidget *main, const QString &command)
{
    Seen s;
    s.command = command;
    QWidget *w = frontWindow(main);
    if (!w)
        return s;
    s.window = w->objectName().isEmpty() ? QString::fromLatin1(w->metaObject()->className()) : w->objectName();
    const QRect area = screenArea();
    const QRect frame = w->frameGeometry();
    s.fits = area.contains(frame) || (frame.width() <= area.width() && frame.height() <= area.height() && area.intersects(frame));
    s.fits = s.fits && area.contains(w->geometry());
    QList<QAbstractButton *> buttons;
    for (QAbstractButton *b : w->findChildren<QAbstractButton *>())
        if (b->isVisible() && (qobject_cast<QPushButton *>(b) || qobject_cast<QDialogButtonBox *>(b->parentWidget())))
            buttons << b;
    s.buttonsIn = true;
    QStringList outside;
    for (QAbstractButton *b : buttons)
        if (!buttonInside(b, area)) {
            s.buttonsIn = false;
            outside << b->text();
        }
    // a button that closes it: one of a button box, or one named Close, Cancel, OK
    for (QAbstractButton *b : buttons) {
        auto *box = qobject_cast<QDialogButtonBox *>(b->parentWidget());
        const QDialogButtonBox::ButtonRole role = box ? box->buttonRole(b) : QDialogButtonBox::InvalidRole;
        const QString t = b->text().remove(QLatin1Char('&'));
        if (role == QDialogButtonBox::RejectRole || role == QDialogButtonBox::AcceptRole || role == QDialogButtonBox::YesRole
            || role == QDialogButtonBox::NoRole || t == QLatin1String("Close") || t == QLatin1String("Cancel")
            || t == QLatin1String("OK"))
            if (buttonInside(b, area))
                s.closable = true;
    }
    if (!s.fits)
        s.why += QStringLiteral(" frame %1 on %2").arg(rectText(frame), rectText(area));
    if (!outside.isEmpty())
        s.why += QStringLiteral(" buttons outside: %1").arg(outside.join(QStringLiteral(", ")));
    if (!s.closable)
        s.why += QStringLiteral(" no button that closes it");
    shot(w, QStringLiteral("window-") + command);
    QPointer<QWidget> guard = w;
    QWidget *to = QApplication::focusWidget() && QApplication::focusWidget()->window() == w ? QApplication::focusWidget() : w;
    back(to);
    QCoreApplication::processEvents();
    s.byBack = !guard || !guard->isVisible();
    if (!s.byBack) {
        s.why += QStringLiteral(" Back left it open");
        if (auto *d = qobject_cast<QDialog *>(guard.data()))
            d->reject();
        else if (guard)
            guard->close();
    }
    return s;
}

static void collect(QMenu *menu, QList<QAction *> *out)
{
    for (QAction *a : menu->actions()) {
        if (a->menu())
            collect(a->menu(), out);
        else if (!a->isSeparator())
            *out << a;
    }
}

static void windowChecks(MainWindow &w)
{
    info(QStringLiteral("---- every window a command opens (%1)").arg(sizeName));
    const QSet<QString> help = {QStringLiteral("manual"), QStringLiteral("quickStart"), QStringLiteral("placeholders"),
                                QStringLiteral("keyboardShortcuts"), QStringLiteral("addonManager"), QStringLiteral("thisComputer"),
                                QStringLiteral("mindMapHelp"), QStringLiteral("developerGuide"), QStringLiteral("systemInfo"),
                                QStringLiteral("crashReports"), QStringLiteral("licenses"), QStringLiteral("about"),
                                QStringLiteral("objectsWindow")};
    // never: the end, a project of their own, the network, the printer, the desktop
    const QSet<QString> skip = {QStringLiteral("quit"), QStringLiteral("welcome"), QStringLiteral("pluginFolder"),
                                QStringLiteral("integrate"), QStringLiteral("updateCheckNow"), QStringLiteral("updateCheckAuto"),
                                QStringLiteral("print"), QStringLiteral("printPreview"), QStringLiteral("closeProject"),
                                QStringLiteral("restart")};
    QList<QAction *> all;
    for (QAction *top : w.menuBar()->actions())
        if (top->menu())
            collect(top->menu(), &all);
    QList<QAction *> commands;
    QSet<QString> named;
    for (QAction *a : all) {
        const QString n = a->objectName();
        if (n.isEmpty() || named.contains(n) || skip.contains(n) || n.startsWith(QLatin1String("print")))
            continue;
        if (help.contains(n) || a->text().contains(QChar(0x2026)) || a->text().contains(QLatin1String("...")))
            commands << a, named.insert(n);
    }
    int windows = 0, bad = 0;
    QStringList lines;
    for (QAction *a : commands) {
        w.activateWindow();
        settle(30);
        if (!a->isEnabled())
            continue;
        QList<Seen> seen;
        bool busy = true;
        // up to three windows in a row (a question after the first)
        std::function<void(int)> arm = [&](int left) {
            QTimer::singleShot(350, &w, [&, left] {
                if (!frontWindow(&w)) {
                    busy = false;
                    return;
                }
                seen << measure(&w, a->objectName());
                if (left > 1)
                    arm(left - 1);
                else
                    busy = false;
            });
        };
        arm(3);
        a->trigger();
        QElapsedTimer t;
        t.start();
        while (busy && t.elapsed() < 4000)
            settle(40);
        // a window still open (it came after the three): closed by force
        for (int i = 0; i < 3 && frontWindow(&w); ++i) {
            QWidget *f = frontWindow(&w);
            if (auto *d = qobject_cast<QDialog *>(f))
                d->reject();
            else
                f->close();
            settle(50);
        }
        for (const Seen &s : seen) {
            if (s.window.isEmpty())
                continue;
            ++windows;
            const bool ok = s.fits && s.buttonsIn && s.closable && s.byBack;
            if (!ok)
                ++bad;
            lines << QStringLiteral("%1 %2 -> %3%4").arg(ok ? QStringLiteral("ok  ") : QStringLiteral("BAD "), s.command, s.window,
                                                          s.why);
        }
    }
    for (const QString &l : lines)
        info(l);
    check(windows >= 30, QStringLiteral("(%1 windows opened by %2 commands)").arg(windows).arg(commands.size()));
    check(bad == 0, QStringLiteral("every window fits, its buttons are in view, one of them closes it, and Back closes it"),
          QStringLiteral("%1 of %2 windows do not").arg(bad).arg(windows));
}

// ------------------------------------------------------------------ menus

static void menuChecks(MainWindow &w)
{
    info(QStringLiteral("---- the menus of the menu bar (%1)").arg(sizeName));
    QMenuBar *bar = w.menuBar();
    const QRect area = screenArea();
    const int barBottom = bar->mapToGlobal(QPoint(0, bar->height())).y();
    int bad = 0, menus = 0;
    for (QAction *top : bar->actions()) {
        QMenu *m = top->menu();
        if (!m || !top->isVisible())
            continue;
        ++menus;
        w.activateWindow();
        settle(30);
        QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier, bar->actionGeometry(top).center());
        settle(120);
        QString why;
        if (!m->isVisible()) {
            why = QStringLiteral(" did not open");
        } else {
            const QRect g = m->geometry();
            if (!area.contains(g))
                why += QStringLiteral(" outside the screen (%1)").arg(rectText(g));
            if (g.top() < barBottom - 2)
                why += QStringLiteral(" over the menu bar (top %1, bar ends at %2)").arg(g.top()).arg(barBottom);
            // one column: every entry has the same left edge
            int left = -1;
            bool columns = false;
            for (QAction *a : m->actions()) {
                if (!a->isVisible())
                    continue;
                const QRect r = m->actionGeometry(a);
                if (left < 0)
                    left = r.left();
                else if (r.left() != left)
                    columns = true;
            }
            if (columns)
                why += QStringLiteral(" in columns");
            shot(m, QStringLiteral("menu-") + top->text().remove(QLatin1Char('&')));
            // the last entry can be reached: the menu scrolls to it with the keyboard
            QAction *last = nullptr;
            for (QAction *a : m->actions())
                if (a->isVisible() && !a->isSeparator() && a->isEnabled())
                    last = a;
            if (last) {
                m->setActiveAction(last);
                settle(60);
                const QRect r = m->actionGeometry(last);
                if (!QRect(QPoint(0, 0), m->size()).contains(r.center()))
                    why += QStringLiteral(" its last entry out of reach");
            }
        }
        QTest::keyClick(m, Qt::Key_Escape);
        settle(60);
        if (m->isVisible())
            m->hide();
        if (!why.isEmpty()) {
            ++bad;
            info(QStringLiteral("BAD  %1:%2").arg(top->text().remove(QLatin1Char('&')), why));
        } else {
            info(QStringLiteral("ok   %1: %2").arg(top->text().remove(QLatin1Char('&')), rectText(m->geometry())));
        }
    }
    check(menus >= 8 && bad == 0, QStringLiteral("every menu opens below the menu bar, inside the screen, in one column, its last entry in reach"),
          QStringLiteral("%1 of %2 menus do not").arg(bad).arg(menus));
}

// ------------------------------------------------------------------ Back, keys with the mouse, the line, pin, padlock

static BinderTree *treeOf(MainWindow &w) { return w.findChild<BinderTree *>(); }

static QModelIndex rowNamed(BinderTree *tree, const QString &title)
{
    const QModelIndexList found = tree->model()->match(tree->model()->index(0, 0), Qt::DisplayRole, title, 1,
                                                       Qt::MatchExactly | Qt::MatchRecursive);
    return found.value(0);
}

static void backChecks(MainWindow &w)
{
    info(QStringLiteral("---- Back (%1)").arg(sizeName));
    w.activateWindow();
    settle(60);
    QMenuBar *bar = w.menuBar();
    QAction *first = bar->actions().value(0);
    QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier, bar->actionGeometry(first).center());
    settle(120);
    QMenu *open = qobject_cast<QMenu *>(QApplication::activePopupWidget());
    const bool taken = open && back(open);
    settle(60);
    check(open && taken && !open->isVisible(), QStringLiteral("Back closes an open menu"));
    // the main window alone: Back is left to Android (which puts the app away; the program saves then)
    w.activateWindow();
    settle(40);
    QWidget *f = QApplication::focusWidget() ? QApplication::focusWidget() : &w;
    check(!back(f) && w.isVisible(), QStringLiteral("with the main window alone Back is left to Android"));
}

static void keyChecks(MainWindow &w)
{
    info(QStringLiteral("---- Ctrl and Shift with the mouse (%1)").arg(sizeName));
    BinderTree *tree = treeOf(w);
    const QModelIndex a = rowNamed(tree, QStringLiteral("Scene 1")), b = rowNamed(tree, QStringLiteral("Scene 3"));
    check(a.isValid() && b.isValid(), QStringLiteral("(the binder has the scenes)"));
    if (!a.isValid() || !b.isValid())
        return;
    tree->scrollTo(a);
    settle(60);
    QWidget *port = tree->viewport();
    // as Qt 6.12.0 for Android delivers them: the keys and the clicks through the window (the platform's way), the
    // key with its modifier, the click without any (Qt then also forgets the key it was told of)
    QWindow *win = w.windowHandle();
    auto click = [&](const QModelIndex &i) {
        QTest::mouseClick(win, Qt::LeftButton, Qt::NoModifier, port->mapTo(&w, tree->visualRect(i).center()));
        settle(60);
    };
    click(a);
    QTest::keyPress(win, Qt::Key_Control);
    click(b);
    QTest::keyRelease(win, Qt::Key_Control);
    settle(60);
    const int both = tree->selectionModel()->selectedRows().size();
    check(both == 2, QStringLiteral("Ctrl held on the keyboard + a click without it chooses a second document"),
          QStringLiteral("%1 chosen").arg(both));
    click(a);
    QTest::keyPress(win, Qt::Key_Shift);
    click(b);
    QTest::keyRelease(win, Qt::Key_Shift);
    settle(60);
    const int range = tree->selectionModel()->selectedRows().size();
    check(range == 3, QStringLiteral("Shift held + a click chooses the documents between"), QStringLiteral("%1 chosen").arg(range));
    // let go: a click chooses one again
    click(b);
    check(tree->selectionModel()->selectedRows().size() == 1, QStringLiteral("after the key is let go a click chooses one document"));
    // Ctrl + wheel zooms the binder
    const double before = zoom::factor(QLatin1String(zoom::kBinder));
    QTest::keyPress(win, Qt::Key_Control);
    const QPointF at = tree->visualRect(b).center();
    QWheelEvent wheel(at, port->mapToGlobal(at), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(port, &wheel);
    QTest::keyRelease(win, Qt::Key_Control);
    settle(80);
    const double after = zoom::factor(QLatin1String(zoom::kBinder));
    check(after > before + 0.05, QStringLiteral("Ctrl held + the wheel zooms the binder"),
          QStringLiteral("%1 -> %2").arg(before).arg(after));
    zoom::setFactor(QLatin1String(zoom::kBinder), before);
    settle(40);
}

static QSplitterHandle *mainLine(MainWindow &w)
{
    BinderTree *tree = treeOf(w);
    for (QSplitter *s : w.findChildren<QSplitter *>())
        if (s->orientation() == Qt::Horizontal && s->count() >= 3 && tree && s->widget(0)->isAncestorOf(tree))
            return s->handle(1);
    return nullptr;
}

// 0.93.0: an upright window has two rows; the lines between them (the rows, and the binder and the inspector) are found
// and taken along by a mouse pressed 5 px beside them
static void uprightLineChecks(MainWindow &w)
{
    info(QStringLiteral("---- the lines of the upright window (%1)").arg(sizeName));
    QSplitter *rows = w.paneSplitter(), *side = w.sideSplitter();
    check(rows->count() == 2 && side->count() == 2 && rows->orientation() == Qt::Vertical, QStringLiteral("(the two rows and the side row are found)"));
    auto press = [&](QWidget *to, const QPoint &inWindow, QEvent::Type t, Qt::MouseButton b, Qt::MouseButtons bs) {
        const QPoint g = w.mapToGlobal(inWindow);
        QMouseEvent e(t, to->mapFromGlobal(QPointF(g)), w.mapFromGlobal(QPointF(g)), QPointF(g), b, bs, Qt::NoModifier);
        QApplication::sendEvent(to, &e);
    };
    auto drag = [&](const QPoint &from, const QPoint &by) {
        QWidget *under = w.childAt(from);
        if (!under)
            return false;
        press(under, from, QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton);
        press(under, from + by / 2, QEvent::MouseMove, Qt::NoButton, Qt::LeftButton);
        press(under, from + by, QEvent::MouseMove, Qt::NoButton, Qt::LeftButton);
        press(under, from + by, QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton);
        settle(60);
        return true;
    };
    QSplitterHandle *rowLine = rows->handle(1), *sideLine = side->handle(1);
    check(rowLine->height() >= 6 && sideLine->width() >= 6,
          QStringLiteral("the lines are wide enough to be found (%1 and %2 px)").arg(rowLine->height()).arg(sideLine->width()));
    shot(&w, QStringLiteral("upright-lines"));
    const int sideStart = side->sizes().value(0), inspMin = side->widget(1)->minimumSizeHint().width();
    const int room = qMin(40, side->sizes().value(1) - inspMin);
    const bool d1 = drag(sideLine->mapTo(&w, QPoint(sideLine->width() + 5, sideLine->height() / 2)), QPoint(-40, 0));
    check(d1 && side->sizes().value(0) < sideStart - 10, QStringLiteral("a mouse pressed beside the line of the binder and the inspector takes it along"),
          QStringLiteral("%1 -> %2 (room %3)").arg(sideStart).arg(side->sizes().value(0)).arg(room));
    const int rowStart = rows->sizes().value(0);
    const bool d2 = drag(rowLine->mapTo(&w, QPoint(rowLine->width() / 2, rowLine->height() + 5)), QPoint(0, -40));
    check(d2 && rows->sizes().value(0) < rowStart - 10, QStringLiteral("a mouse pressed beside the line between the rows takes it along"),
          QStringLiteral("%1 -> %2").arg(rowStart).arg(rows->sizes().value(0)));
    check(w.width() <= screenArea().width() + 1, QStringLiteral("the main window is not wider than the screen"));
}

static void lineChecks(MainWindow &w)
{
    if (w.isUpright()) {
        uprightLineChecks(w);
        return;
    }
    info(QStringLiteral("---- the line between the binder and the text (%1)").arg(sizeName));
    QSplitterHandle *h = mainLine(w);
    check(h != nullptr, QStringLiteral("(the line is found)"));
    if (!h)
        return;
    QSplitter *s = h->splitter();
    QWidget *binder = s->widget(0), *editor = s->widget(1), *inspector = s->widget(2);
    info(QStringLiteral("least widths: binder %1, text %2, inspector %3 (%4); window %5, sizes %6 %7 %8")
             .arg(binder->minimumSizeHint().width())
             .arg(editor->minimumSizeHint().width())
             .arg(inspector->minimumSizeHint().width())
             .arg(inspector->isVisible() ? QStringLiteral("shown") : QStringLiteral("hidden"))
             .arg(w.width())
             .arg(s->sizes().value(0))
             .arg(s->sizes().value(1))
             .arg(s->sizes().value(2)));
    if (qEnvironmentVariableIsSet("QB_FIT_WIDTHS")) { // which part decides a pane's least width
        std::function<void(QWidget *, int)> walk = [&](QWidget *x, int depth) {
            for (QWidget *c : x->findChildren<QWidget *>(Qt::FindDirectChildrenOnly)) {
                if (!c->isVisible())
                    continue;
                const int mw = qMax(c->minimumSizeHint().width(), c->minimumWidth());
                if (mw >= 200) {
                    info(QStringLiteral("%1%2 %3 min %4").arg(QString(depth * 2, QLatin1Char(' ')), QString::fromLatin1(c->metaObject()->className()), c->objectName()).arg(mw));
                    walk(c, depth + 1);
                }
            }
        };
        walk(editor, 0);
        walk(inspector, 0);
    }
    check(h->width() >= 6, QStringLiteral("the line is wide enough to be found (%1 px)").arg(h->width()));
    shot(h, QStringLiteral("line"));
    // a mouse pressed 5 px beside the line, on the text's side, moved 60 px to the left (the binder narrower), then
    // pressed 6 px beside it on the binder's side and moved back to the right
    auto press = [&](QWidget *to, const QPoint &inWindow, QEvent::Type t, Qt::MouseButton b, Qt::MouseButtons bs) {
        const QPoint g = w.mapToGlobal(inWindow);
        QMouseEvent e(t, to->mapFromGlobal(QPointF(g)), w.mapFromGlobal(QPointF(g)), QPointF(g), b, bs, Qt::NoModifier);
        QApplication::sendEvent(to, &e);
    };
    auto drag = [&](const QPoint &from, int dx) {
        QWidget *under = w.childAt(from);
        if (!under)
            return false;
        press(under, from, QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton);
        press(under, from + QPoint(dx / 2, 0), QEvent::MouseMove, Qt::NoButton, Qt::LeftButton);
        press(under, from + QPoint(dx, 0), QEvent::MouseMove, Qt::NoButton, Qt::LeftButton);
        press(under, from + QPoint(dx, 0), QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton);
        settle(60);
        return true;
    };
    const int start = s->sizes().value(0);
    const int downRoom = qMin(60, start - binder->minimumSizeHint().width());
    const bool d1 = drag(h->mapTo(&w, QPoint(h->width() + 5, h->height() / 2)), -60);
    const int narrower = s->sizes().value(0);
    check(d1 && narrower <= start - downRoom + 4 && narrower < start - qMin(10, downRoom - 1),
          QStringLiteral("a mouse pressed beside the line (on the text's side) takes it along: the binder gets narrower"),
          QStringLiteral("binder %1 -> %2 px (room %3)").arg(start).arg(narrower).arg(downRoom));
    const int upRoom = qMin(60, (s->sizes().value(1) - editor->minimumSizeHint().width())
                                    + (inspector->isVisible() ? s->sizes().value(2) - inspector->minimumSizeHint().width() : 0));
    const bool d2 = drag(h->mapTo(&w, QPoint(-6, h->height() / 2)), 60);
    const int wider = s->sizes().value(0);
    check(d2 && upRoom >= 20 && wider >= narrower + upRoom - 4,
          QStringLiteral("and from the binder's side: the binder gets wider (as far as the other panes leave room)"),
          QStringLiteral("binder %1 -> %2 px (room %3)").arg(narrower).arg(wider).arg(upRoom));
    check(w.width() <= screenArea().width() + 1, QStringLiteral("the main window is not wider than the screen"),
          QStringLiteral("window %1, screen %2").arg(w.width()).arg(screenArea().width()));
}

static void markChecks(MainWindow &w)
{
    info(QStringLiteral("---- the pin and the padlock on a tablet (%1)").arg(sizeName));
    BinderTree *tree = treeOf(w);
    const QModelIndex row = rowNamed(tree, QStringLiteral("Scene 2"));
    if (!row.isValid())
        return;
    tree->scrollTo(row);
    QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, tree->visualRect(row).center());
    settle(80);
    check(tree->pinShownOn(row) && !tree->pinShownOn(rowNamed(tree, QStringLiteral("Scene 4"))),
          QStringLiteral("the pin shows on the selected row (and not on the others)"));
    shot(tree, QStringLiteral("pin-on-selected"));
    const QString id = row.data(BinderModel::IdRole).toString();
    QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, tree->pinRectOf(row).center());
    settle(80);
    check(tree->fixedMindMapId() == id, QStringLiteral("a click on it fixes the mind map on that item"));
    // the row's menu: Release, then Fix again
    QStringList entries;
    bool released = false;
    QTimer::singleShot(300, &w, [&] {
        auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        if (!m)
            return;
        for (QAction *a : m->actions())
            if (!a->objectName().isEmpty())
                entries << a->objectName();
        for (QAction *a : m->actions())
            if (a->objectName() == QLatin1String("releaseMindMap") && a->isEnabled()) {
                m->hide();
                a->trigger();
                released = true;
                return;
            }
        m->hide();
    });
    emit tree->customContextMenuRequested(tree->visualRect(row).center());
    settle(500);
    check(entries.contains(QStringLiteral("fixMindMap")) && entries.contains(QStringLiteral("releaseMindMap")) && released
              && tree->fixedMindMapId().isEmpty(),
          QStringLiteral("the row's menu has Fix and Release: Release lets it go"), entries.join(QLatin1Char(' ')));
    // the padlock: a tap shows its text, Back takes it away
    const QString tip = QStringLiteral("Open on the other tablet since 09:12");
    tree->setLockMarks({{id, tip}});
    settle(60);
    const QRect lock = tree->lockRectOf(row);
    QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, lock.center());
    settle(300);
    check(QToolTip::isVisible() && QToolTip::text() == tip, QStringLiteral("a tap on the padlock shows who works on it"),
          QToolTip::text());
    shot(&w, QStringLiteral("padlock-tip"));
    if (!QToolTip::isVisible()) { // (a tip goes after a while by itself)
        QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, lock.center());
        settle(300);
    }
    const bool before = QToolTip::isVisible();
    const bool taken = back(tree);
    settle(450); // (Qt lets a tip go 300 ms after it was told to)
    check(before && taken && !QToolTip::isVisible(), QStringLiteral("Back takes the text away"),
          QStringLiteral("shown before %1, taken %2").arg(before).arg(taken));
    tree->setLockMarks({});
}

static void aboutChecks(MainWindow &w)
{
    info(QStringLiteral("---- About (%1)").arg(sizeName));
    QAction *about = w.findChild<QAction *>(QStringLiteral("about"));
    QString why = QStringLiteral("not opened");
    bool scrolls = false;
    QTimer::singleShot(400, &w, [&] {
        QWidget *box = QApplication::activeModalWidget();
        if (!box)
            return;
        why.clear();
        scrolls = box->findChild<QTextBrowser *>(QStringLiteral("aboutText")) != nullptr;
        const QRect area = screenArea();
        if (!area.contains(box->geometry()))
            why += QStringLiteral(" outside the screen ") + rectText(box->frameGeometry());
        for (QAbstractButton *b : box->findChildren<QAbstractButton *>())
            if (b->isVisible() && !buttonInside(b, area))
                why += QStringLiteral(" button outside: ") + b->text();
        shot(box, QStringLiteral("about"));
        if (!back(box))
            why += QStringLiteral(" Back not taken");
    });
    about->trigger();
    settle(200);
    check(why.isEmpty() && scrolls, QStringLiteral("About: its text scrolls, its buttons are in view, Back closes it"), why);
}

// ------------------------------------------------------------------ updates and the add-ons the APK carries

static void updateChecks()
{
    info(QStringLiteral("---- the check for updates on Android, the add-ons the APK carries"));
    const QString sum(64, QLatin1Char('a'));
    auto file = [&sum](const QString &name) {
        return QJsonObject{{QStringLiteral("url"), QStringLiteral("https://github.com/o/r/releases/download/t/") + name},
                           {QStringLiteral("name"), name}, {QStringLiteral("size"), 1234}, {QStringLiteral("sha256"), sum}};
    };
    const QJsonObject block{
        {QStringLiteral("program"), QJsonObject{{QStringLiteral("version"), QStringLiteral("0.92.1")},
                                                {QStringLiteral("label"), QStringLiteral("experimental")},
                                                {QStringLiteral("runtime"), QStringLiteral("android-qt6.12-r1")},
                                                {QStringLiteral("apk"), file(QStringLiteral("Quillbinder-0.92.1-experimental-android-arm64.apk"))},
                                                // (a program update, which Android never takes)
                                                {QStringLiteral("update"), file(QStringLiteral("quillbinder-0.92.1-update.qbpkg"))}}},
        {QStringLiteral("addons"), QJsonArray{QJsonObject{{QStringLiteral("id"), QStringLiteral("translation-proofreading")},
                                                          {QStringLiteral("version"), QStringLiteral("1.3")},
                                                          {QStringLiteral("minApp"), QStringLiteral("0.91.3")},
                                                          {QStringLiteral("runtime"), QStringLiteral("android-qt6.12-r1")},
                                                          {QStringLiteral("file"), file(QStringLiteral("translation-proofreading-android-arm64.qbpkg"))}}}}};
    updates::Feed feed;
    feed.format = 1;
    feed.date = QDateTime::currentDateTimeUtc();
    feed.systems = QJsonObject{{QStringLiteral("android-arm64"), block}, {QStringLiteral("linux-x86_64"), block}};
    updates::Installed here;
    here.appVersion = QStringLiteral("0.92.0");
    here.runtime = QStringLiteral("android-qt6.12-r1");
    here.addons.insert(QStringLiteral("translation-proofreading"), QStringLiteral("1.2"));
    const QList<updates::Offer> a = updates::offersFor(feed, QStringLiteral("android-arm64"), here);
    const bool apk = a.size() == 2 && a[0].kind == updates::Offer::Kind::NewApk && a[0].file.name.endsWith(QLatin1String(".apk"))
                     && a[1].kind == updates::Offer::Kind::Addon && a[1].version == QLatin1String("1.3");
    check(apk, QStringLiteral("Android: a newer version is offered as its APK (never as a program update), a newer add-on as its package"),
          QStringLiteral("%1 offers, the first %2").arg(a.size()).arg(a.isEmpty() ? QString() : a[0].file.name));
    // the same block read as Linux's: the program update, as before (the Android way is for Android only)
    const QList<updates::Offer> l = updates::offersFor(feed, QStringLiteral("linux-x86_64"), here);
    check(!l.isEmpty() && l[0].kind == updates::Offer::Kind::ProgramUpdate,
          QStringLiteral("(the same block for Linux gives a program update: the APK way is Android's alone)"));

    // the add-ons the APK carries: installed once per version, before the plug-ins load
    QTemporaryDir from;
    QProcess mk;
    mk.start(QStringLiteral("python3"), {QStringLiteral("-I"), QStringLiteral(QB_SOURCE_DIR "/tools/make-package.py"), QStringLiteral("addon"),
                                          QStringLiteral(QB_SOURCE_DIR "/addons/themes-extra"), from.filePath(QStringLiteral("themes-extra.qbpkg"))});
    mk.waitForFinished(60000);
    const bool made = mk.exitCode() == 0 && QFileInfo::exists(from.filePath(QStringLiteral("themes-extra.qbpkg")));
    check(made, QStringLiteral("(a package made for the check)"), QString::fromLocal8Bit(mk.readAllStandardError()));
    if (!made)
        return;
    const int first = mobile::installFullAddons(from.path());
    bool installed = false;
    for (const addons::Installed &i : addons::installedAddons())
        installed = installed || i.manifest.id == QLatin1String("themes-extra");
    const int second = mobile::installFullAddons(from.path());
    check(first == 1 && installed && second == 0,
          QStringLiteral("the add-ons the APK carries are installed at the first start of a version, and not again"),
          QStringLiteral("first %1, installed %2, second %3").arg(first).arg(installed).arg(second));
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
    const QRect area = screenArea();
    sizeName = QStringLiteral("%1x%2").arg(area.width()).arg(area.height());
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    mobile::installCare(); // as the program does on Android: right after its style
    check(windowcare::tabletMode(), QStringLiteral("(the tablet mode is on)"));
    if (QFontDatabase::families().contains(QStringLiteral("Roboto"))) {
        QFont f(QStringLiteral("Roboto"));
        f.setPointSizeF(14); // (14 px at Android's 72 dpi)
        QApplication::setFont(f);
        info(QStringLiteral("(Roboto 14 px, as on Android)"));
    }
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
    for (int i = 0; i < 12; ++i)
        proj->model()->createItem(proj->draft(), -1, ItemType::Text, QStringLiteral("Scene %1").arg(i + 1));

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.setGeometry(area);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    w.setGeometry(area);
    settle(300);
    mobile::installTouch();
    mobile::fitToolbars(&w);
    settle(200);
    info(QStringLiteral("(screen %1, main window %2)").arg(rectText(area), rectText(w.geometry())));
    shot(&w, QStringLiteral("main"));

    // QB_FIT_ONLY="keys menus ..." runs some parts only (line, keys, marks, back, about, menus, windows)
    const QStringList only = qEnvironmentVariable("QB_FIT_ONLY").split(QLatin1Char(' '), Qt::SkipEmptyParts);
    auto run = [&only](const char *name) { return only.isEmpty() || only.contains(QLatin1String(name)); };
    if (run("line"))
        lineChecks(w);
    if (run("keys"))
        keyChecks(w);
    if (run("marks"))
        markChecks(w);
    if (run("back"))
        backChecks(w);
    if (run("about"))
        aboutChecks(w);
    if (run("menus"))
        menuChecks(w);
    if (run("windows"))
        windowChecks(w);
    if (run("updates"))
        updateChecks();

    w.close();
    delete wp;
    settle(100);
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
