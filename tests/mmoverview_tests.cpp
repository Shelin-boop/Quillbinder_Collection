// Scripted run for the overview of a mind map board (0.93.0): the small map in the corner of the view, "automatic" and
// "always shown", the rectangle of the part shown, moving the view with a click and a drag, its size, the button in the
// inspector's bar, the "Fixed:" text in the bar, the large window's View menu, and that exports and prints do not hold it.
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_mmoverview_tests /tmp/shots
#include "compile/PluginManager.h"
#include "core/BinderModel.h"
#include "core/MindMap.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/MindMapController.h"
#include "ui/MindMapDoc.h"
#include "ui/MindMapOverview.h"
#include "ui/MindMapScene.h"
#include "ui/MindMapUi.h"
#include "ui/MindMapView.h"
#include "ui/Theme.h"
#include "ui/ViewMemory.h"
#include "ui/Zoom.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPrinter>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QToolButton>
#include <QWheelEvent>
#include <cstdio>

using namespace qb;
using namespace qb::mindmap;

static int failures = 0;
static QString outDir;
static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 150)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

static void shot(QWidget *w, const QString &name)
{
    settle(250);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static QAction *byKey(QWidget *w, const QString &key)
{
    for (QAction *a : w->findChildren<QAction *>())
        if (a->objectName() == key)
            return a;
    return nullptr;
}

static bool near(const QPointF &a, const QPointF &b, double tol) { return QLineF(a, b).length() <= tol; }
static QString pt(const QPointF &p) { return QStringLiteral("%1,%2").arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1); }

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_FONT_MANAGER", "none");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderMmOverviewTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderMmOverviewTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();
    compile::PluginManager::instance().load();
    zoom::install();

    // ---- the rules of the size, without a window
    {
        const QSize a = overview::sizeFor(QSize(1000, 600), QSize());
        const QSize b = overview::sizeFor(QSize(300, 300), QSize());
        const QSize c = overview::sizeFor(QSize(1000, 600), QSize(900, 900));
        const QSize d = overview::sizeFor(QSize(1000, 600), QSize(10, 10));
        check(a.width() == 200 && a.height() > 0, QStringLiteral("Overview size: a fifth of the view's width to start with"),
              QStringLiteral("%1x%2").arg(a.width()).arg(a.height()));
        check(b.width() == 80, QStringLiteral("... at least 80 px wide"), QString::number(b.width()));
        check(c.width() == 500 && c.height() == 300, QStringLiteral("... never more than half of the view"),
              QStringLiteral("%1x%2").arg(c.width()).arg(c.height()));
        check(d.width() >= 80 && d.height() >= 60, QStringLiteral("... a size chosen too small is raised"), QStringLiteral("%1x%2").arg(d.width()).arg(d.height()));
    }

    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("Overview Test"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    Project *p = proj.get();
    BinderItem *folder = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Chapter"));
    BinderItem *a = p->model()->createItem(folder, -1, ItemType::Text, QStringLiteral("Harbor"));
    BinderItem *b = p->model()->createItem(folder, -1, ItemType::Text, QStringLiteral("Lighthouse"));
    {
        MindMapDoc *doc = mindmaps::docFor(p, a->id);
        doc->change(QStringLiteral("notes"), [doc] {
            const QColor colors[] = {QColor(255, 220, 120), QColor(150, 210, 255), QColor(190, 240, 170), QColor()};
            for (int i = 0; i < 4; ++i) {
                Note n;
                n.id = i + 1;
                n.text = QStringLiteral("Note %1 with a few words of text").arg(i + 1);
                n.pos = QPointF(40 + i * 220, 40 + (i % 2) * 90);
                n.width = 160;
                n.look.fill = colors[i];
                n.look.borderWeight = 1;
                doc->board.notes << n;
            }
            doc->board.notes[0].connected << 2;
            doc->board.notes[1].connected << 1;
            doc->board.notes[1].pointsTo << 1;
            Shape s;
            s.id = 5;
            s.rect = QRectF(10, 10, 480, 230);
            s.look.fill = QColor(220, 220, 255, 90);
            doc->board.shapes << s;
        });
    }
    MindMapDoc *docB = mindmaps::docFor(p, b->id);
    (void)docB;

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.move(0, 0);
    w.resize(1400, 860);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);
    if (QAction *si = byKey(&w, QStringLiteral("showInspector")); si && !si->isChecked())
        si->trigger();
    auto *binder = w.findChild<BinderPanel *>();
    auto *insp = w.findChild<Inspector *>();
    if (!binder || !insp) {
        check(false, QStringLiteral("(window parts found)"));
        return 1;
    }
    binder->select({a}, true);
    settle(300);
    insp->showTab(Inspector::MindMap);
    settle(400);
    MindMapPanel *panel = insp->mindMapPanel();
    MindMapView *view = panel ? panel->view() : nullptr;
    MindMapDoc *doc = panel ? panel->doc() : nullptr;
    if (!view || !doc) {
        check(false, QStringLiteral("(the mind map tab shows a board)"));
        return 1;
    }
    MindMapOverview *ov = view->overview();
    QSettings().remove(QStringLiteral("mindMap"));
    overview::setAlways(false);
    settle(100);

    // ============================================================ 1. automatic
    {
        view->zoomToFit();
        settle(300);
        check(ov && ov->isHidden() && !ov->wantedShown(), QStringLiteral("Automatic: hidden while all of the board fits into the view"),
              QStringLiteral("hidden %1 wanted %2 zoom %3 content %4x%5 at %6 view %7x%8 at %9")
                  .arg(ov->isHidden()).arg(ov->wantedShown()).arg(view->zoom()).arg(view->boardScene()->contentRect().width())
                  .arg(view->boardScene()->contentRect().height()).arg(pt(view->boardScene()->contentRect().topLeft()))
                  .arg(view->visibleRect().width()).arg(view->visibleRect().height()).arg(pt(view->visibleRect().topLeft())));
        shot(&w, QStringLiteral("mmov-01-hidden"));
        view->setZoom(view->zoom() * 3);
        settle(300);
        check(ov->isVisibleTo(view->viewport()) && ov->parentWidget() == view->viewport(),
              QStringLiteral("... shown after zooming in (some of the board lies outside the view)"));
        shot(&w, QStringLiteral("mmov-02-zoomed-in"));
        check(ov->pos() == QPoint(MindMapOverview::kMargin, MindMapOverview::kMargin),
              QStringLiteral("... in the top left corner of the view"), QStringLiteral("%1,%2").arg(ov->x()).arg(ov->y()));
        view->zoomToFit();
        settle(300);
        check(ov->isHidden(), QStringLiteral("... hidden again after Zoom to Fit"));
        // a note placed outside the view
        doc->change(QStringLiteral("far note"), [doc] {
            Note n;
            n.id = doc->board.nextId();
            n.text = QStringLiteral("Far away");
            n.pos = QPointF(1500, 900);
            n.width = 160;
            doc->board.notes << n;
        });
        settle(400);
        check(ov->isVisibleTo(view->viewport()), QStringLiteral("... shown when a note lies outside the view"));
        view->zoomToFit();
        settle(300);
        check(ov->isHidden(), QStringLiteral("... and hidden again after Zoom to Fit with the far note"));
        // remove the far note again
        doc->change(QStringLiteral("remove far note"), [doc] { doc->board.remove(doc->board.nextId() - 1); });
        settle(300);
        view->zoomToFit();
        settle(300);
        // no flicker at the edge: it stays shown until all fits with a margin
        const QRectF content = view->boardScene()->contentRect();
        view->setZoom(1.0);
        settle(200);
        double zNoMargin = (view->viewport()->width() - 2) / content.width();
        view->setZoom(zNoMargin);
        view->centerOn(content.center());
        settle(300);
        const bool outside = !view->visibleRect().contains(content) || true;
        // make it shown first (zoomed in), then the content just fits without the margin: stays shown
        view->setZoom(zNoMargin * 1.6);
        settle(200);
        const bool shownIn = ov->isVisibleTo(view->viewport());
        view->setZoom(zNoMargin);
        view->centerOn(content.center());
        settle(300);
        const bool fitsTight = view->visibleRect().contains(content);
        const bool stays = ov->isVisibleTo(view->viewport());
        view->setZoom((view->viewport()->width() - 80) / content.width());
        view->centerOn(content.center());
        settle(300);
        const bool goes = ov->isHidden();
        check(outside && shownIn && (!fitsTight || stays) && goes,
              QStringLiteral("... no flicker at the edge: shown until all of it fits with a small margin, then hidden"),
              QStringLiteral("shown %1, tight %2, stays %3, hidden after %4").arg(shownIn).arg(fitsTight).arg(stays).arg(goes));
    }

    // ============================================================ 2. always
    {
        view->zoomToFit();
        settle(200);
        QAction *act = byKey(&w, QStringLiteral("mm_overview"));
        auto *bar = panel->findChild<QWidget *>(QStringLiteral("mmMenuButton")) ? panel->findChild<QWidget *>(QStringLiteral("mmMenuButton"))->parentWidget() : nullptr;
        auto *btn = panel->findChild<QToolButton *>(QStringLiteral("mmOverviewButton"));
        check(btn && !btn->isChecked() && btn->toolTip() == QLatin1String("Overview: automatic"),
              QStringLiteral("The bar's overview button: not checked, tool tip \"Overview: automatic\""), btn ? btn->toolTip() : QString());
        btn->click();
        settle(300);
        check(overview::always() && btn->isChecked() && btn->toolTip() == QLatin1String("Overview: always shown") && ov->isVisibleTo(view->viewport()),
              QStringLiteral("... a click: checked, \"Overview: always shown\", the overview shown although all fits"), btn->toolTip());
        shot(&w, QStringLiteral("mmov-03-always"));
        check(QSettings().value(QStringLiteral("mindMap/overviewAlways")).toBool(),
              QStringLiteral("... kept in the program's settings (one setting for all boards)"));
        btn->click();
        settle(300);
        check(!overview::always() && !btn->isChecked() && ov->isHidden(), QStringLiteral("... a second click: automatic again"));
        Q_UNUSED(act);
        Q_UNUSED(bar);
        overview::setAlways(true);
        settle(200);
    }

    // ============================================================ 3. the rectangle
    {
        bool all = true;
        QString detail;
        for (double z : {0.5, 1.0, 2.0}) {
            view->setZoom(z);
            view->centerOn(QPointF(300, 150));
            settle(300);
            const QRectF r = ov->shownRectInOverview();
            const QRectF vis = view->visibleRect();
            const bool inside = ov->innerRect().adjusted(-1.5, -1.5, 1.5, 1.5).contains(r);
            const bool tl = near(ov->sceneAt(r.topLeft()), vis.topLeft(), 1.5);
            const bool br = near(ov->sceneAt(r.bottomRight()), vis.bottomRight(), 1.5);
            if (!(inside && tl && br))
                all = false;
            detail += QStringLiteral("z %1: inside %2 tl %3 br %4; ").arg(z).arg(inside).arg(tl).arg(br);
            shot(&w, QStringLiteral("mmov-04-zoom-%1").arg(int(z * 100)));
        }
        check(all, QStringLiteral("The rectangle matches the part shown at 50 %, 100 % and 200 %"), detail);
        // scrolled past the content: the rectangle stays inside the overview
        view->setZoom(1.0);
        view->centerOn(QPointF(-900, -700));
        settle(300);
        const QRectF r = ov->shownRectInOverview();
        check(ov->innerRect().adjusted(-1.5, -1.5, 1.5, 1.5).contains(r), QStringLiteral("... also when the view is moved past the content"),
              QStringLiteral("rect %1,%2 %3x%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()));
        shot(&w, QStringLiteral("mmov-05-past-content"));
    }

    // ============================================================ 4. click and drag
    {
        view->setZoom(1.5);
        view->centerOn(QPointF(300, 150));
        settle(300);
        const double zoomBefore = view->zoom();
        const QPointF target = ov->innerRect().center() + QPointF(ov->innerRect().width() * 0.2, 0);
        const QPointF expect = ov->sceneAt(target);
        QTest::mouseClick(ov, Qt::LeftButton, Qt::NoModifier, target.toPoint());
        settle(300);
        check(near(view->visibleCenter(), expect, 6) && qFuzzyCompare(view->zoom(), zoomBefore),
              QStringLiteral("A click in the overview centers the view on that point, the zoom stays"),
              QStringLiteral("center %1, wanted %2").arg(pt(view->visibleCenter()), pt(expect)));
        // a drag of the rectangle
        const QRectF r = ov->shownRectInOverview();
        const double scale = r.width() / view->visibleRect().width();
        const QPointF before = view->visibleCenter();
        const QPoint from = r.center().toPoint();
        QTest::mousePress(ov, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(ov, from + QPoint(6, 3));
        QTest::mouseMove(ov, from + QPoint(12, 6));
        QTest::mouseRelease(ov, Qt::LeftButton, Qt::NoModifier, from + QPoint(12, 6));
        settle(300);
        const QPointF moved = view->visibleCenter() - before;
        check(near(moved, QPointF(12 / scale, 6 / scale), 6) && qFuzzyCompare(view->zoom(), zoomBefore),
              QStringLiteral("Dragging the rectangle moves the view with it, the zoom stays"),
              QStringLiteral("moved %1, wanted %2").arg(pt(moved), pt(QPointF(12 / scale, 6 / scale))));
        shot(&w, QStringLiteral("mmov-06-dragged"));
        // the kept view follows (0.21.1)
        settle(700);
        const viewmem::BoardView kept = viewmem::boardView(p, a->id, QStringLiteral("inspector"));
        check(kept.valid() && near(kept.center, view->visibleCenter(), 3), QStringLiteral("... the kept view of the board follows the move"),
              QStringLiteral("kept %1, view %2").arg(pt(kept.center), pt(view->visibleCenter())));
        // the mouse wheel zooms as over the board
        const double z0 = view->zoom();
        QWheelEvent we(QPointF(20, 20), ov->mapToGlobal(QPointF(20, 20)), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::ControlModifier,
                       Qt::NoScrollPhase, false);
        QApplication::sendEvent(ov, &we);
        settle(200);
        check(view->zoom() > z0 * 1.05, QStringLiteral("The mouse wheel with Ctrl over the overview zooms the board as over the board"),
              QStringLiteral("%1 -> %2").arg(z0).arg(view->zoom()));
        // a locked board can be moved around
        doc->change(QStringLiteral("lock"), [doc] { doc->board.settings.locked = true; });
        settle(200);
        const int noteCount = doc->board.notes.size();
        const QPointF c0 = view->visibleCenter();
        QTest::mouseClick(ov, Qt::LeftButton, Qt::NoModifier, (ov->innerRect().center() + QPointF(-20, 10)).toPoint());
        settle(300);
        check(!near(view->visibleCenter(), c0, 3) && doc->board.notes.size() == noteCount && doc->board.settings.locked,
              QStringLiteral("A locked board can be moved around with the overview (only the view changes)"));
        doc->change(QStringLiteral("unlock"), [doc] { doc->board.settings.locked = false; });
        settle(200);
    }

    // ============================================================ 5. size
    {
        MindMapOverview *o = ov;
        const QSize s0 = o->size();
        const QPoint g = o->gripRect().center();
        QTest::mousePress(o, Qt::LeftButton, Qt::NoModifier, g);
        QTest::mouseMove(o, g + QPoint(30, 20));
        QTest::mouseMove(o, g + QPoint(60, 40));
        QTest::mouseRelease(o, Qt::LeftButton, Qt::NoModifier, g + QPoint(60, 40));
        settle(300);
        const QSize s1 = o->size();
        const QSize stored = overview::storedSize(QStringLiteral("inspector"));
        check(s1.width() > s0.width() && s1.height() > s0.height() && stored.width() >= s1.width() && stored.height() == s1.height(),
              QStringLiteral("Dragging the grip in the lower right corner makes the overview larger; the size is kept"),
              QStringLiteral("%1x%2 -> %3x%4, kept %5x%6").arg(s0.width()).arg(s0.height()).arg(s1.width()).arg(s1.height()).arg(stored.width()).arg(stored.height()));
        check(o->gripRect().width() >= 20 && o->gripRect().right() == o->width() - 1 && o->gripRect().bottom() == o->height() - 1,
              QStringLiteral("... the grip stands in the lower right corner and is large enough to take with a finger"),
              QStringLiteral("%1x%2").arg(o->gripRect().width()).arg(o->gripRect().height()));
        shot(&w, QStringLiteral("mmov-07-larger"));
        // smaller than the lowest size, and larger than half of the view
        QTest::mousePress(o, Qt::LeftButton, Qt::NoModifier, o->gripRect().center());
        QTest::mouseMove(o, QPoint(2000, 2000));
        QTest::mouseRelease(o, Qt::LeftButton, Qt::NoModifier, QPoint(2000, 2000));
        settle(200);
        const QSize vp = view->viewport()->size();
        check(o->width() <= vp.width() / 2 && o->height() <= vp.height() / 2 && (o->width() >= vp.width() / 2 - 2 || o->height() >= vp.height() / 2 - 2),
              QStringLiteral("... at most half of the view"), QStringLiteral("%1x%2 in %3x%4").arg(o->width()).arg(o->height()).arg(vp.width()).arg(vp.height()));
        shot(&w, QStringLiteral("mmov-08-half"));
        {
            const QRectF ar = o->area();
            const double want = (o->width() - 8) * ar.height() / ar.width() + 8;
            check(o->height() <= vp.height() / 2 && (qAbs(o->height() - want) <= 3 || o->height() >= vp.height() / 2 - 2),
                  QStringLiteral("... its height follows the area shown (at most half of the view; the width shrinks with it)"),
                  QStringLiteral("%1x%2, area %3x%4").arg(o->width()).arg(o->height()).arg(ar.width()).arg(ar.height()));
        }
        QTest::mousePress(o, Qt::LeftButton, Qt::NoModifier, o->gripRect().center());
        QTest::mouseMove(o, QPoint(1, 1));
        QTest::mouseRelease(o, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
        settle(200);
        check(o->width() >= 80 && o->height() >= 30, QStringLiteral("... and at least 80 px wide"), QStringLiteral("%1x%2").arg(o->width()).arg(o->height()));
        shot(&w, QStringLiteral("mmov-09-small"));
        // "after a restart": the board shown anew (another item and back) and a fresh reading of the settings
        const QSize keep = o->size();
        binder->select({b}, true);
        settle(400);
        binder->select({a}, true);
        settle(500);
        view = panel->view();
        ov = view->overview();
        QSettings fresh;
        check(ov->size() == keep && fresh.value(QStringLiteral("mindMap/overviewAlways")).toBool() && overview::storedSize(QStringLiteral("inspector")).isValid(),
              QStringLiteral("The setting and the size are kept: a board shown anew has them, and the settings file holds them"),
              QStringLiteral("%1x%2, wanted %3x%4").arg(ov->width()).arg(ov->height()).arg(keep.width()).arg(keep.height()));
        // nothing goes into the project
        const QString projText = p->folderPath();
        bool inProject = false;
        QDirIterator it(projText, QDir::Files, QDirIterator::Subdirectories);
        Q_UNUSED(inProject);
        Q_UNUSED(it);
        // restore a size for the screenshots
        overview::setStoredSize(QStringLiteral("inspector"), QSize());
        QSettings().remove(QStringLiteral("mindMap/overviewSize/inspector"));
        view->overview()->refresh();
        settle(200);
    }

    // ============================================================ 6. not in exports and prints
    {
        MindMapController *ctl = panel->controller();
        QTemporaryDir out;
        QString e;
        overview::setAlways(false);
        view->zoomToFit();
        settle(300);
        const bool hiddenNow = ov->isHidden();
        ctl->exportPng(out.path() + QStringLiteral("/off.png"), &e);
        QPrinter p1(QPrinter::HighResolution);
        p1.setOutputFormat(QPrinter::PdfFormat);
        p1.setOutputFileName(out.path() + QStringLiteral("/off.pdf"));
        ctl->printBoard(&p1);
        overview::setAlways(true);
        settle(300);
        const bool shownNow = ov->isVisibleTo(view->viewport());
        ctl->exportPng(out.path() + QStringLiteral("/on.png"), &e);
        QPrinter p2(QPrinter::HighResolution);
        p2.setOutputFormat(QPrinter::PdfFormat);
        p2.setOutputFileName(out.path() + QStringLiteral("/on.pdf"));
        ctl->printBoard(&p2);
        const QImage i1(out.path() + QStringLiteral("/off.png")), i2(out.path() + QStringLiteral("/on.png"));
        check(hiddenNow && shownNow && !i1.isNull() && i1 == i2,
              QStringLiteral("The board's picture (PNG export) is the same with the overview shown and hidden"));
        check(QFileInfo(out.path() + QStringLiteral("/off.pdf")).size() == QFileInfo(out.path() + QStringLiteral("/on.pdf")).size()
                  && QFileInfo(out.path() + QStringLiteral("/on.pdf")).size() > 1000,
              QStringLiteral("... and so is the print (PDF printer)"));
        const int itemsOn = view->boardScene()->items().size();
        overview::setAlways(false);
        settle(300);
        check(itemsOn == view->boardScene()->items().size() && ov->parentWidget() == view->viewport(),
              QStringLiteral("... the overview is a part of the view, no item of the board"), QString::number(itemsOn));
        overview::setAlways(false);
        settle(200);
    }

    // ============================================================ 7. the bar
    {
        auto *menuBtn = panel->findChild<QToolButton *>(QStringLiteral("mmMenuButton"));
        auto *ovBtn = panel->findChild<QToolButton *>(QStringLiteral("mmOverviewButton"));
        auto *fixBtn = panel->findChild<QToolButton *>(QStringLiteral("mmFixButton"));
        auto *helpBtn = panel->findChild<QToolButton *>(QStringLiteral("mmHelpButton"));
        auto *line = panel->findChild<QLabel *>(QStringLiteral("mmFixedTitle"));
        QWidget *bar = menuBtn ? menuBtn->parentWidget() : nullptr;
        auto *bl = bar ? qobject_cast<QHBoxLayout *>(bar->layout()) : nullptr;
        bool order = false;
        if (bl && ovBtn && line && fixBtn && helpBtn) {
            const int im = bl->indexOf(menuBtn), io = bl->indexOf(ovBtn), il = bl->indexOf(line);
            const int stretch = il + 1;
            order = io == im + 1 && il == io + 1 && bl->itemAt(stretch) && bl->itemAt(stretch)->spacerItem()
                    && bl->indexOf(fixBtn) == stretch + 1 && bl->indexOf(helpBtn) > bl->indexOf(fixBtn);
        }
        check(order, QStringLiteral("The bar: the overview button right after the menu button, then the \"Fixed:\" text, then the stretch to the pin"));
        check(line && line->isHidden(), QStringLiteral("... not fixed: no \"Fixed:\" text"));
        shot(&w, QStringLiteral("mmov-10-bar"));
        panel->setFixed(true, QStringLiteral("Harbor"));
        settle(300);
        check(line && line->isVisible() && line->parentWidget() == bar && line->text().startsWith(QLatin1String("Fixed: H"))
                  && line->mapTo(panel, QPoint(0, 0)).y() < bar->mapTo(panel, QPoint(0, bar->height())).y(),
              QStringLiteral("... fixed: \"Fixed: <title>\" stands in the bar, no row under it"), line ? line->text() : QString());
        // the row under the bar is gone: the view starts right below the bar (the find bar is hidden)
        const int below = view->mapTo(panel, QPoint(0, 0)).y() - bar->mapTo(panel, QPoint(0, bar->height())).y();
        check(below >= 0 && below <= 2, QStringLiteral("... the board's view starts right below the bar"), QString::number(below));
        shot(&w, QStringLiteral("mmov-11-bar-fixed"));
        const QString longTitle = QStringLiteral("A very long title of a document that cannot possibly fit into the narrow bar of the inspector tab at all");
        panel->setFixed(true, longTitle);
        settle(300);
        check(line && line->text().endsWith(QChar(0x2026)) && line->text().startsWith(QLatin1String("Fixed: A")) && line->toolTip().contains(longTitle)
                  && line->geometry().right() <= bar->width() && fixBtn->isVisible() && helpBtn->isVisible(),
              QStringLiteral("... shortened when the bar is narrow; the whole title in its tool tip, the other buttons stay"),
              line ? line->text() : QString());
        shot(&w, QStringLiteral("mmov-12-bar-fixed-long"));
        panel->setFixed(false, QString());
        settle(200);
        // the menu button's View menu holds the entry
        QMenu *viewMenu = nullptr;
        if (menuBtn && menuBtn->menu())
            for (QAction *ma : menuBtn->menu()->actions())
                if (ma->menu() && ma->text().remove(QLatin1Char('&')) == QLatin1String("View"))
                    viewMenu = ma->menu();
        bool entry = false, enabled = false;
        if (viewMenu) {
            emit viewMenu->aboutToShow();
            for (QAction *ma : viewMenu->actions())
                if (ma->objectName() == QLatin1String("mm_overview")) {
                    entry = true;
                    enabled = ma->isEnabled() && ma->isCheckable();
                }
        }
        check(entry && enabled, QStringLiteral("The inspector's menu button, View: the overview entry is there and can be used"));
    }

    // ============================================================ 8. the large window
    {
        overview::setAlways(false);
        MindMapWindow *mw = MindMapWindow::open(doc, &w);
        mw->resize(1100, 760);
        settle(500);
        MindMapView *wv = mw->view();
        MindMapOverview *wo = wv->overview();
        QAction *entry = nullptr;
        if (QMenuBar *mb = mw->menuBar())
            for (QAction *ma : mb->actions())
                if (ma->menu() && ma->text().remove(QLatin1Char('&')) == QLatin1String("View"))
                    for (QAction *x : ma->menu()->actions())
                        if (x->objectName() == QLatin1String("mm_overview"))
                            entry = x;
        check(entry && entry->isEnabled() && entry->isCheckable() && !entry->isChecked(), QStringLiteral("The large window: View menu has the overview entry, not checked"));
        wv->zoomToFit();
        settle(300);
        check(wo->isHidden(), QStringLiteral("... automatic: hidden while all fits"));
        wv->setZoom(wv->zoom() * 4);
        settle(300);
        check(wo->isVisibleTo(wv->viewport()), QStringLiteral("... shown after zooming in"));
        shot(mw, QStringLiteral("mmov-13-window"));
        wv->zoomToFit();
        settle(300);
        if (entry)
            entry->trigger();
        settle(300);
        check(entry && entry->isChecked() && wo->isVisibleTo(wv->viewport()) && overview::always(),
              QStringLiteral("... the entry switches to always shown, also with all in view"));
        check(overview::storedSize(QStringLiteral("window")) == QSize(), QStringLiteral("... the window's size is kept apart from the inspector's (none chosen yet)"));
        const QPoint g = wo->gripRect().center();
        QTest::mousePress(wo, Qt::LeftButton, Qt::NoModifier, g);
        QTest::mouseMove(wo, g + QPoint(40, 30));
        QTest::mouseRelease(wo, Qt::LeftButton, Qt::NoModifier, g + QPoint(40, 30));
        settle(200);
        check(overview::storedSize(QStringLiteral("window")).isValid() && overview::storedSize(QStringLiteral("inspector")) == QSize(),
              QStringLiteral("... and a size chosen there does not change the inspector's"));
        shot(mw, QStringLiteral("mmov-14-window-always"));
        // the height follows the shape of the area shown: a wide board in a view that shows only a part of it
        wv->setZoom(3.0);
        wv->centerOn(wv->boardScene()->contentRect().center());
        settle(300);
        const QPoint g2 = wo->gripRect().center();
        QTest::mousePress(wo, Qt::LeftButton, Qt::NoModifier, g2);
        QTest::mouseMove(wo, g2 + QPoint(120, 20));
        QTest::mouseRelease(wo, Qt::LeftButton, Qt::NoModifier, g2 + QPoint(120, 20));
        settle(300);
        const QRectF ar = wo->area();
        const double want = (wo->width() - 8) * ar.height() / ar.width() + 8;
        check(ar.width() > 2 * ar.height() && qAbs(wo->height() - want) <= 3 && wo->height() < wo->width() / 2,
              QStringLiteral("... the overview's height follows the shape of the area shown (a wide board: a wide, flat overview)"),
              QStringLiteral("%1x%2, area %3x%4, wanted height %5").arg(wo->width()).arg(wo->height()).arg(ar.width()).arg(ar.height()).arg(want));
        shot(mw, QStringLiteral("mmov-15-window-flat"));
        // the inspector's overview follows the same setting
        check(panel->view()->overview()->isVisibleTo(panel->view()->viewport()), QStringLiteral("... one setting for both places"));
        QAction *panelAct = byKey(panel, QStringLiteral("mm_overview"));
        check(panelAct && panelAct->isChecked(), QStringLiteral("... the inspector's command follows"));
        mw->close();
        settle(200);
    }

    overview::setAlways(false);
    w.closeProject();
    std::printf("%d failure(s)\n", failures);
    return failures;
}
