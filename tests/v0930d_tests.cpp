// Scripted run for version 0.93.0 beta, fourth suite: to-do item 22 (the panes placed by the window's shape).
//  - (a) a wide window: the six orders of the three panes through View > Layout, the start order, widths kept;
//  - (b) an upright window: the editor's row upper or lower, the binder left or right, the heights and widths;
//  - (c) the window follows its shape by itself (taller than wide, back, a square window is wide), each shape comes back as
//    it was left; a hidden binder or inspector leaves the row to the other, both hidden: the editor alone;
//  - (d) the choices and sizes of both shapes are kept per device after a restart, never in the project (its files are
//    unchanged byte by byte); Reset Layout; the menu;
//  - (e) the least widths of the binder and the inspector beside each other.
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0930d_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QLayout>
#include <QMenuBar>
#include <QMouseEvent>
#include <QSettings>
#include <QSplitter>
#include <QSplitterHandle>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>

#include <cstdio>

using namespace qb;

static int failures = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(3000)));
    }
    std::fflush(stdout);
}

static void info(const QString &t)
{
    std::printf("INFO  %s\n", qPrintable(t));
    std::fflush(stdout);
}

static void settle(int ms)
{
    QTest::qWait(ms);
}

static void shot(QWidget *w, const QString &name)
{
    settle(120);
    w->grab().save(outDir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

namespace {
struct Rects {
    QRect binder, editor, inspector;
};

QWidget *editorArea(MainWindow &w)
{
    return w.findChild<QWidget *>(QStringLiteral("editorArea"));
}

Rects rects(MainWindow &w)
{
    auto r = [&](QWidget *x) { return QRect(x->mapTo(&w, QPoint(0, 0)), x->size()); };
    return {r(w.findChild<BinderPanel *>()), r(editorArea(w)), r(w.findChild<Inspector *>())};
}

QAction *action(MainWindow &w, const char *key)
{
    return w.findChild<QAction *>(QLatin1String(key));
}

void trigger(MainWindow &w, const char *key)
{
    if (QAction *a = action(w, key))
        a->trigger();
    settle(150);
}

void resizeTo(MainWindow &w, int width, int height)
{
    w.resize(width, height);
    settle(300);
}

// the order of the three panes by their places, left to right
QString orderOf(MainWindow &w)
{
    const Rects r = rects(w);
    QList<QPair<int, QChar>> v{{r.binder.x(), QLatin1Char('B')}, {r.editor.x(), QLatin1Char('E')}, {r.inspector.x(), QLatin1Char('I')}};
    std::sort(v.begin(), v.end(), [](auto &a, auto &b) { return a.first < b.first; });
    return QString(v[0].second) + v[1].second + v[2].second;
}

// The writer's hand on the line between two panes (handle index h, 1 = after the first pane): moved by dx pixels (along the
// splitter's direction)
void dragLine(QSplitter *s, int h, int dx)
{
    QSplitterHandle *hd = s->handle(h);
    const bool horizontal = s->orientation() == Qt::Horizontal;
    const QPointF from = hd->rect().center();
    auto send = [&](QEvent::Type t, const QPointF &pos, Qt::MouseButton b, Qt::MouseButtons bs) {
        const QPointF g = hd->mapToGlobal(pos);
        QMouseEvent e(t, pos, hd->window()->mapFromGlobal(g.toPoint()), g, b, bs, Qt::NoModifier);
        QApplication::sendEvent(hd, &e);
    };
    const QPointF to = from + (horizontal ? QPointF(dx, 0) : QPointF(0, dx));
    send(QEvent::MouseButtonPress, from, Qt::LeftButton, Qt::LeftButton);
    send(QEvent::MouseMove, (from + to) / 2, Qt::NoButton, Qt::LeftButton);
    send(QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
    send(QEvent::MouseButtonRelease, to, Qt::LeftButton, Qt::NoButton);
    settle(150);
}

QMap<QString, QByteArray> snapshot(const QString &folder)
{
    QMap<QString, QByteArray> m;
    QDirIterator it(folder, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString f = it.next();
        QFile file(f);
        if (file.open(QIODevice::ReadOnly))
            m.insert(f.mid(folder.size()), QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256));
    }
    return m;
}
} // namespace

static bool near(int a, int b, int tol = 4)
{
    return qAbs(a - b) <= tol;
}

// ------------------------------------------------------------------ (a) the wide window

static void wideChecks(MainWindow &w)
{
    std::printf("== the wide window (item 22 a)\n");
    resizeTo(w, 1300, 820);
    check(!w.isUpright(), QStringLiteral("1300 x 820 is a wide window"));
    check(orderOf(w) == QLatin1String("BEI") && w.wideOrder() == QLatin1String("BEI"),
          QStringLiteral("the start order is binder, editor, inspector"), orderOf(w));
    check(action(w, "layoutOrderBEI")->isChecked(), QStringLiteral("the menu shows the start order"));
    shot(&w, QStringLiteral("wide-BEI"));
    const char *orders[] = {"BIE", "EBI", "EIB", "IBE", "IEB", "BEI"};
    for (const char *o : orders) {
        trigger(w, (QByteArray("layoutOrder") + o).constData());
        const bool ok = orderOf(w) == QLatin1String(o);
        check(ok, QStringLiteral("View > Layout: the order %1 is shown").arg(QLatin1String(o)), orderOf(w));
        int checkedCount = 0;
        for (const char *k : {"BEI", "BIE", "EBI", "EIB", "IBE", "IEB"})
            if (action(w, (QByteArray("layoutOrder") + k).constData())->isChecked())
                ++checkedCount;
        check(checkedCount == 1 && action(w, (QByteArray("layoutOrder") + o).constData())->isChecked(),
              QStringLiteral("only the order %1 is checked").arg(QLatin1String(o)));
        const Rects r = rects(w);
        check(r.binder.width() > 50 && r.editor.width() > 300 && r.inspector.width() > 100,
              QStringLiteral("the order %1 gives every pane room").arg(QLatin1String(o)),
              QStringLiteral("%1 %2 %3").arg(r.binder.width()).arg(r.editor.width()).arg(r.inspector.width()));
        if (QByteArray(o) == "IEB" || QByteArray(o) == "EIB")
            shot(&w, QStringLiteral("wide-%1").arg(QLatin1String(o)));
    }
    // a width set by the writer's hand (the line between two panes) is kept through a change of the order
    trigger(w, "layoutOrderEIB");
    const int before = rects(w).binder.width();
    dragLine(w.paneSplitter(), 2, -150);
    settle(150);
    const int moved = rects(w).binder.width();
    check(moved > before + 50, QStringLiteral("the line between inspector and binder moves the binder wider"), QStringLiteral("%1 -> %2").arg(before).arg(moved));
    trigger(w, "layoutOrderBEI");
    check(near(rects(w).binder.width(), moved, 6), QStringLiteral("the binder keeps its width when the order changes"),
          QStringLiteral("%1 vs %2").arg(rects(w).binder.width()).arg(moved));
}

// ------------------------------------------------------------------ (b) the upright window

static void uprightChecks(MainWindow &w)
{
    std::printf("== the upright window (item 22 b)\n");
    resizeTo(w, 800, 1100);
    check(w.isUpright(), QStringLiteral("800 x 1100 is an upright window"));
    Rects r = rects(w);
    check(r.editor.width() >= w.centralWidget()->width() - 2 && r.editor.height() > 300,
          QStringLiteral("the editor's row has the full width"), QStringLiteral("%1 x %2").arg(r.editor.width()).arg(r.editor.height()));
    check(r.binder.top() > r.editor.bottom() - 2 && r.inspector.top() > r.editor.bottom() - 2 && near(r.binder.top(), r.inspector.top(), 3),
          QStringLiteral("start values: the editor in the middle (upper), the binder and the inspector below it"),
          QStringLiteral("editor bottom %1, binder top %2, inspector top %3").arg(r.editor.bottom()).arg(r.binder.top()).arg(r.inspector.top()));
    check(r.binder.right() <= r.inspector.left() + 1, QStringLiteral("start values: the binder left, the inspector right"));
    check(action(w, "layoutEditorMiddle")->isChecked() && action(w, "layoutBinderLeft")->isChecked(),
          QStringLiteral("the menu shows the start choices"));
    shot(&w, QStringLiteral("upright-middle-binder-left"));

    trigger(w, "layoutBinderRight");
    r = rects(w);
    check(r.inspector.right() <= r.binder.left() + 1 && near(r.binder.top(), r.inspector.top(), 3),
          QStringLiteral("Binder Right: the inspector left, the binder right"));
    check(action(w, "layoutBinderRight")->isChecked() && !action(w, "layoutBinderLeft")->isChecked(), QStringLiteral("(its check shows)"));
    shot(&w, QStringLiteral("upright-middle-binder-right"));

    trigger(w, "layoutEditorBottom");
    r = rects(w);
    check(r.editor.top() >= r.binder.bottom() - 2 && r.editor.top() >= r.inspector.bottom() - 2,
          QStringLiteral("Editor at the Bottom: the binder and the inspector above the editor"),
          QStringLiteral("editor top %1, binder bottom %2").arg(r.editor.top()).arg(r.binder.bottom()));
    check(r.editor.width() >= w.centralWidget()->width() - 2, QStringLiteral("the editor keeps the full width"));
    check(action(w, "layoutEditorBottom")->isChecked() && !action(w, "layoutEditorMiddle")->isChecked(), QStringLiteral("(its check shows)"));
    shot(&w, QStringLiteral("upright-bottom-binder-right"));

    trigger(w, "layoutBinderLeft");
    r = rects(w);
    check(r.binder.right() <= r.inspector.left() + 1 && r.editor.top() >= r.binder.bottom() - 2,
          QStringLiteral("Editor at the Bottom with the binder left"));
    shot(&w, QStringLiteral("upright-bottom-binder-left"));

    // the lines: the height of the rows and the widths of the two
    QSplitter *rows = w.paneSplitter(), *side = w.sideSplitter();
    check(rows->orientation() == Qt::Vertical && side->orientation() == Qt::Horizontal && rows->count() == 2 && side->count() == 2,
          QStringLiteral("two rows (a line between them) and two panes beside each other (a line between them)"));
    const int sideH = rects(w).binder.height();
    dragLine(rows, 1, 120); // the side row (upper) grows
    settle(150);
    check(rects(w).binder.height() > sideH + 80, QStringLiteral("the line between the rows moves the height"),
          QStringLiteral("%1 -> %2").arg(sideH).arg(rects(w).binder.height()));
    const int bw = rects(w).binder.width();
    dragLine(side, 1, 100);
    settle(150);
    check(rects(w).binder.width() > bw + 60, QStringLiteral("the line between the binder and the inspector moves the widths"),
          QStringLiteral("%1 -> %2").arg(bw).arg(rects(w).binder.width()));
    // with the window shorter (the on-screen keyboard): the side row keeps its height, the editor's row loses it
    const int sideNow = rects(w).binder.height();
    const int editorNow = rects(w).editor.height();
    resizeTo(w, 800, 1000);
    check(w.isUpright() && near(rects(w).binder.height(), sideNow, 3) && rects(w).editor.height() < editorNow - 50,
          QStringLiteral("a shorter window: the side row keeps its height, the editor's row takes the loss"),
          QStringLiteral("side %1 -> %2, editor %3 -> %4").arg(sideNow).arg(rects(w).binder.height()).arg(editorNow).arg(rects(w).editor.height()));
    trigger(w, "layoutEditorMiddle");
}

// ------------------------------------------------------------------ (c) following the shape, hiding

static void shapeChecks(MainWindow &w)
{
    std::printf("== the window's shape (item 22 c)\n");
    resizeTo(w, 1300, 820);
    trigger(w, "layoutOrderIBE");
    const QList<int> wideBefore = w.paneSizes();
    dragLine(w.paneSplitter(), 1, 40); // the inspector 40 px wider
    settle(100);
    const QList<int> wideSet = w.paneSizes();
    resizeTo(w, 700, 1000);
    check(w.isUpright(), QStringLiteral("the window made taller than wide switches to the upright arrangement"));
    trigger(w, "layoutEditorBottom");
    trigger(w, "layoutBinderRight");
    dragLine(w.sideSplitter(), 1, 60);
    settle(100);
    const int upBinder = rects(w).binder.width();
    const int upEditor = rects(w).editor.height();
    resizeTo(w, 1300, 820);
    check(!w.isUpright() && orderOf(w) == QLatin1String("IBE"), QStringLiteral("back to wide: the order the writer left (IBE)"), orderOf(w));
    check(near(rects(w).inspector.width(), wideSet[2], 6) && near(rects(w).binder.width(), wideSet[0], 6),
          QStringLiteral("back to wide: the widths the writer left"),
          QStringLiteral("inspector %1 vs %2, binder %3 vs %4 (before the move %5)").arg(rects(w).inspector.width()).arg(wideSet[2])
              .arg(rects(w).binder.width()).arg(wideSet[0]).arg(wideBefore[2]));
    shot(&w, QStringLiteral("wide-IBE-again"));
    resizeTo(w, 700, 1000);
    check(w.isUpright() && !w.uprightEditorFirst() && !w.uprightBinderLeft(), QStringLiteral("upright again: editor at the bottom, binder right"));
    const Rects r = rects(w);
    check(r.editor.top() >= r.binder.bottom() - 2 && r.inspector.right() <= r.binder.left() + 1,
          QStringLiteral("upright again: the places as left"));
    check(near(r.binder.width(), upBinder, 6) && near(r.editor.height(), upEditor, 6), QStringLiteral("upright again: the sizes as left"),
          QStringLiteral("binder %1 vs %2, editor %3 vs %4").arg(r.binder.width()).arg(upBinder).arg(r.editor.height()).arg(upEditor));
    // a square window counts as wide
    resizeTo(w, 900, 900);
    check(!w.isUpright(), QStringLiteral("a square window is wide"));
    resizeTo(w, 900, 901);
    check(w.isUpright(), QStringLiteral("one pixel taller than wide is upright"));
    resizeTo(w, 700, 1000);

    // hiding in the upright arrangement
    trigger(w, "layoutEditorMiddle");
    trigger(w, "layoutBinderLeft");
    QAction *showBinder = action(w, "showBinder"), *showInspector = action(w, "showInspector");
    const int full = w.centralWidget()->width();
    showBinder->setChecked(false);
    settle(200);
    Rects h = rects(w);
    check(w.findChild<BinderPanel *>()->isHidden() && h.inspector.width() >= full - 4 && h.editor.width() >= full - 2,
          QStringLiteral("binder hidden (upright): the inspector has the full width of its row"),
          QStringLiteral("inspector %1 of %2").arg(h.inspector.width()).arg(full));
    shot(&w, QStringLiteral("upright-binder-hidden"));
    showBinder->setChecked(true);
    showInspector->setChecked(false);
    settle(200);
    h = rects(w);
    check(h.binder.width() >= full - 4, QStringLiteral("inspector hidden (upright): the binder has the full width of its row"),
          QStringLiteral("binder %1 of %2").arg(h.binder.width()).arg(full));
    shot(&w, QStringLiteral("upright-inspector-hidden"));
    showBinder->setChecked(false);
    settle(200);
    h = rects(w);
    check(!w.sideSplitter()->isVisible() && h.editor.height() >= w.centralWidget()->height() - 2,
          QStringLiteral("both hidden (upright): the editor alone, no empty row"), QStringLiteral("editor height %1 of %2").arg(h.editor.height()).arg(w.centralWidget()->height()));
    shot(&w, QStringLiteral("upright-both-hidden"));
    showBinder->setChecked(true);
    showInspector->setChecked(true);
    settle(250);
    h = rects(w);
    check(w.sideSplitter()->isVisible() && h.binder.width() > 80 && h.inspector.width() > 150 && near(h.binder.top(), h.inspector.top(), 3),
          QStringLiteral("both shown again: the row is back with both panes"),
          QStringLiteral("%1 %2").arg(h.binder.width()).arg(h.inspector.width()));
    // wide: as today
    resizeTo(w, 1300, 820);
    showInspector->setChecked(false);
    settle(200);
    check(w.findChild<Inspector *>()->isHidden() && rects(w).editor.right() >= w.centralWidget()->width() - 4,
          QStringLiteral("inspector hidden (wide): the editor takes its room"));
    showInspector->setChecked(true);
    settle(200);
    check(!w.findChild<Inspector *>()->isHidden() && rects(w).inspector.width() > 150, QStringLiteral("inspector shown again (wide)"));
}

// ------------------------------------------------------------------ (d) kept per device, the project untouched, reset

static void keepChecks(const QString &W)
{
    std::printf("== kept per device, the project untouched, Reset Layout (item 22 d)\n");
    QString err;
    const QString folder = W + QStringLiteral("/proj");
    QDir().mkpath(folder);
    auto p = Project::create(folder, QStringLiteral("Pane Book"), &err);
    const QString pf = p->folderPath();
    BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Scene"));
    QTextCursor c(p->docs()->text(it->id));
    c.insertText(QStringLiteral("The text of the scene."));
    p->save(&err);
    p.reset();

    QSettings().clear();
    QWidget *shape = nullptr;
    Q_UNUSED(shape);
    QMap<QString, QByteArray> files0;
    int wideBinder = 0, upEditor = 0;
    {
        MainWindow w;
        w.resize(1300, 820);
        w.show();
        settle(300);
        w.openProject(pf);
        settle(500);
        files0 = snapshot(pf);
        trigger(w, "layoutOrderEIB");
        dragLine(w.paneSplitter(), 2, -90);
        settle(100);
        wideBinder = rects(w).binder.width();
        resizeTo(w, 760, 1000);
        trigger(w, "layoutEditorBottom");
        trigger(w, "layoutBinderRight");
        dragLine(w.paneSplitter(), 1, -80);
        settle(100);
        upEditor = rects(w).editor.height();
        resizeTo(w, 1300, 820);
        check(snapshot(pf) == files0, QStringLiteral("after the layout was changed, the project's files are unchanged byte by byte"));
        w.manualSave();
        settle(200);
        bool inFiles = false;
        QDirIterator it2(pf, QDir::Files, QDirIterator::Subdirectories);
        while (it2.hasNext()) {
            QFile f(it2.next());
            if (f.open(QIODevice::ReadOnly)) {
                const QByteArray all = f.readAll();
                if (all.contains("paneOrder") || all.contains("uprightEditorFirst") || all.contains("uprightBinderLeft") || all.contains("EIB"))
                    inFiles = true;
            }
        }
        check(!inFiles, QStringLiteral("no place or size of a pane is written into the project"));
        w.close();
        settle(200);
    }
    QSettings s;
    check(s.value(QStringLiteral("window/paneOrder")).toString() == QLatin1String("EIB")
              && !s.value(QStringLiteral("window/uprightEditorFirst")).toBool() && !s.value(QStringLiteral("window/uprightBinderLeft")).toBool(),
          QStringLiteral("the program's settings hold the order and the upright choices"));
    {
        MainWindow w;
        w.resize(1300, 820);
        w.show();
        settle(400);
        check(!w.isUpright() && orderOf(w) == QLatin1String("EIB") && action(w, "layoutOrderEIB")->isChecked(),
              QStringLiteral("after a restart the wide order is the one left"), orderOf(w));
        check(near(rects(w).binder.width(), wideBinder, 8), QStringLiteral("after a restart the wide widths are the ones left"),
              QStringLiteral("%1 vs %2").arg(rects(w).binder.width()).arg(wideBinder));
        resizeTo(w, 760, 1000);
        const Rects r = rects(w);
        check(w.isUpright() && r.editor.top() >= r.binder.bottom() - 2 && r.inspector.right() <= r.binder.left() + 1,
              QStringLiteral("after a restart the upright places are the ones left (editor at the bottom, binder right)"));
        check(near(r.editor.height(), upEditor, 10), QStringLiteral("after a restart the upright sizes are the ones left"),
              QStringLiteral("%1 vs %2").arg(r.editor.height()).arg(upEditor));
        // Reset Layout puts back the shown shape only
        trigger(w, "layoutReset");
        check(w.uprightEditorFirst() && w.uprightBinderLeft() && w.wideOrder() == QLatin1String("EIB"),
              QStringLiteral("Reset Layout (upright): the upright start values, the wide choices untouched"));
        const Rects r2 = rects(w);
        check(r2.editor.top() < r2.binder.top() && r2.binder.right() <= r2.inspector.left() + 1,
              QStringLiteral("Reset Layout (upright): editor in the middle, binder left"));
        check(near(r2.editor.height(), int(r2.editor.height() + r2.binder.height()) * 62 / 100, 40),
              QStringLiteral("Reset Layout (upright): the start heights"), QStringLiteral("editor %1, side %2").arg(r2.editor.height()).arg(r2.binder.height()));
        resizeTo(w, 1300, 820);
        check(orderOf(w) == QLatin1String("EIB"), QStringLiteral("the wide order is still the one chosen"));
        trigger(w, "layoutReset");
        check(w.wideOrder() == QLatin1String("BEI") && orderOf(w) == QLatin1String("BEI"), QStringLiteral("Reset Layout (wide): the start order"));
        const Rects r3 = rects(w);
        check(near(r3.binder.width(), 260 * w.centralWidget()->width() / 1360, 20) || near(r3.binder.width(), 260, 25),
              QStringLiteral("Reset Layout (wide): the start widths"), QStringLiteral("binder %1, inspector %2").arg(r3.binder.width()).arg(r3.inspector.width()));
        check(w.uprightEditorFirst() && w.uprightBinderLeft(), QStringLiteral("(the upright choices were reset before)"));
        w.close();
        settle(100);
    }
}

// ------------------------------------------------------------------ (e) the menu, the least widths

static void menuChecks(MainWindow &w)
{
    std::printf("== the menu and the least widths (item 22 e)\n");
    resizeTo(w, 1300, 820);
    QMenu *m = w.findChild<QMenu *>(QStringLiteral("paneLayoutMenu"));
    check(m != nullptr && m->title().contains(QLatin1String("yout")), QStringLiteral("View has a Layout menu"));
    if (m) {
        QStringList names;
        int sections = 0;
        for (QAction *a : m->actions()) {
            if (a->isSeparator()) {
                if (!a->text().isEmpty())
                    ++sections;
            } else {
                names << a->objectName();
                check(a->isEnabled(), QStringLiteral("%1 is enabled (both groups always available)").arg(a->objectName()));
            }
        }
        // 0.94.0: View > Layout also holds the saved layouts (before: Window > Layouts) after the two groups
        check(sections == 3 && names.size() >= 13 && names.mid(0, 11).join(QLatin1Char(' ')).startsWith(QLatin1String("layoutEditorMiddle"))
                  && !names.mid(0, 11).join(QLatin1Char(' ')).contains(QLatin1String("layout:"))
                  && names.contains(QStringLiteral("layoutSave")) && names.contains(QStringLiteral("layoutManage")),
              QStringLiteral("two groups, eleven commands, then the saved layouts"), names.join(QLatin1Char(' ')));
        shot(&w, QStringLiteral("window"));
    }
    // upright and wide at the sizes the writer named (736 px wide: a phone-sized tablet window)
    resizeTo(w, 736, 1100);
    resizeTo(w, 736, 1100); // (the first one is held at the wide shape's least width, 808 px; the shape then changes)
    QWidget *b = w.findChild<BinderPanel *>(), *i = w.findChild<Inspector *>();
    const int bmin = b->minimumSizeHint().width(), imin = i->minimumSizeHint().width();
    info(QStringLiteral("window %1 x %2 asked for 736 x 1100; least widths: binder %3, inspector %4, editor %5; window's least size %6 x %7 (layout %8 x %9; set least %10 x %11)")
             .arg(w.width()).arg(w.height()).arg(bmin).arg(imin).arg(editorArea(w)->minimumSizeHint().width())
             .arg(w.minimumSizeHint().width()).arg(w.minimumSizeHint().height()).arg(w.layout()->minimumSize().width()).arg(w.layout()->minimumSize().height()).arg(w.minimumWidth()).arg(w.minimumHeight()));
    const Rects r = rects(w);
    check(w.isUpright() && r.binder.width() >= bmin && r.inspector.width() >= imin && r.binder.right() <= r.inspector.left() + 1,
          QStringLiteral("at 736 px the binder and the inspector fit beside each other"),
          QStringLiteral("window %1 wide: binder %2 (least %3), inspector %4 (least %5), sum of least %6").arg(w.width()).arg(r.binder.width()).arg(bmin).arg(r.inspector.width()).arg(imin).arg(bmin + imin));
    shot(&w, QStringLiteral("upright-736"));
    for (int want : {600, 500}) {
        resizeTo(w, want, 1100);
        info(QStringLiteral("asked %1 px wide: the window is %2 px wide, binder %3, inspector %4").arg(want).arg(w.width()).arg(rects(w).binder.width()).arg(rects(w).inspector.width()));
    }
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0930dTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0930dTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    QTemporaryDir work;
    {
        MainWindow w;
        w.resize(1300, 820);
        w.show();
        settle(400);
        wideChecks(w);
        uprightChecks(w);
        shapeChecks(w);
        menuChecks(w);
        w.close();
        settle(100);
    }
    keepChecks(work.path());
    std::printf("%d failure(s)\n", failures);
    return failures;
}
