// Scripted run for version 0.95.0, first part (Linux):
//  - item 94: the sizes of the three parts of the Bookmarks tab are kept (also while the preview is hidden or the
//    inspector is not shown) and come back after a restart.
//  - item 93: Ctrl+wheel zooms each of the three parts of the Bookmarks tab on its own, with the zoom of the inspector's
//    tabs separate or not; the old common zoom of the two lists is taken for both.
//
//   xvfb-run -a ./qb_v0950a_tests /tmp/shots
#include "compile/PluginManager.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "import/FileImporter.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/Corkboard.h"
#include "ui/EditorPane.h"
#include "ui/FooterStatus.h"
#include "ui/Inspector.h"
#include "ui/BookmarkList.h"
#include "ui/TextEditor.h"
#include "ui/MainWindow.h"
#include "ui/MenuGuard.h"
#include "ui/MindMapDoc.h"
#include "ui/MindMapScene.h"
#include "ui/MindMapController.h"
#include "ui/MindMapUi.h"
#include "ui/MindMapView.h"
#include "ui/QuickRef.h"
#include "ui/Theme.h"
#include "ui/ToolDialogs.h"
#include "ui/ViewMemory.h"
#include "ui/Zoom.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QWheelEvent>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>

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
    settle(300);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static QList<int> listOf(const QVariant &v)
{
    QList<int> out;
    for (const QVariant &x : v.toList())
        out << x.toInt();
    return out;
}

// ---- item 94
static void bookmarkSizes(const QString &dir)
{
    QString err;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    auto proj = templates::createProject(novel, dir, QStringLiteral("Sizes"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return;
    Project *p = proj.get();
    BinderItem *d1 = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("One"));
    QTextCursor c(p->docs()->text(d1->id));
    c.insertText(QStringLiteral("The wind rose in the night and the sea came up over the rocks."));
    p->projectBookmarks.append({d1->title, QStringLiteral("qb://") + d1->id});
    p->save(&err);
    const QString file = p->displayFile();

    auto *w = new MainWindow;
    w->resize(1400, 860);
    w->show();
    w->activateWindow();
    (void)QTest::qWaitForWindowActive(w, 2000);
    w->adoptProject(std::move(proj));
    settle(400);
    auto *insp = w->findChild<Inspector *>();
    if (!insp)
        return;
    insp->showTab(Inspector::Bookmarks);
    settle(200);
    auto *split = insp->findChild<QSplitter *>(QStringLiteral("inspectorBookmarkSplit"));
    check(split && split->count() == 3, QStringLiteral("the Bookmarks tab has its three parts"));
    if (!split)
        return;
    insp->projectBookmarkList()->setCurrentRow(0);
    settle(200);
    check(insp->bookmarkPreview()->isVisible(), QStringLiteral("a bookmark is chosen: the preview is shown"));
    const int total = split->sizes()[0] + split->sizes()[1] + split->sizes()[2];
    const QList<int> set{total / 8, total / 8, total - 2 * (total / 8)};
    split->setSizes(set);
    settle(100);
    const QList<int> shown = split->sizes();
    shot(w, QStringLiteral("94-sizes-set"));

    // the preview is hidden (no bookmark chosen): its size is not written as 0
    insp->projectBookmarkList()->setCurrentRow(-1);
    settle(150);
    check(!insp->bookmarkPreview()->isVisible(), QStringLiteral("no bookmark: the preview is hidden"));
    QList<int> saved = listOf(insp->saveState().value(QStringLiteral("bookmarkSplit")));
    check(saved.size() == 3 && saved[2] > 0 && qAbs(saved[2] - shown[2]) <= 6,
          QStringLiteral("hidden preview: the saved sizes keep its size, not 0"), QString::number(saved.value(2, -1)));

    // the inspector is not shown: nothing is written over the sizes
    QAction *si = nullptr;
    for (QAction *a : w->findChildren<QAction *>())
        if (a->objectName() == QLatin1String("showInspector"))
            si = a;
    if (si && si->isChecked())
        si->trigger();
    settle(200);
    saved = listOf(insp->saveState().value(QStringLiteral("bookmarkSplit")));
    check(saved.size() == 3 && saved[0] > 0 && saved[1] > 0 && saved[2] > 0,
          QStringLiteral("inspector hidden: the saved sizes are the last ones seen"),
          QString::number(saved.value(0, -1)) + QLatin1Char(',') + QString::number(saved.value(1, -1)));
    if (si && !si->isChecked())
        si->trigger();
    settle(200);

    // the sizes are in the file, and come back after a restart (a new window)
    w->closeProject();
    delete w;
    QString oerr;
    auto again = Project::open(file, &oerr);
    check(again != nullptr, QStringLiteral("(project opened again)"), oerr);
    if (!again)
        return;
    const QList<int> inFile = listOf(viewmem::inspectorState(again.get()).value(QStringLiteral("bookmarkSplit")));
    check(inFile.size() == 3 && inFile[2] > 0, QStringLiteral("the project's view file holds a size for the preview"),
          QString::number(inFile.value(2, -1)));
    auto *w2 = new MainWindow;
    w2->resize(1400, 860);
    w2->show();
    w2->activateWindow();
    (void)QTest::qWaitForWindowActive(w2, 2000);
    w2->adoptProject(std::move(again));
    settle(400);
    auto *insp2 = w2->findChild<Inspector *>();
    auto *split2 = insp2 ? insp2->findChild<QSplitter *>(QStringLiteral("inspectorBookmarkSplit")) : nullptr;
    if (!split2)
        return;
    insp2->showTab(Inspector::Bookmarks);
    settle(200);
    insp2->projectBookmarkList()->setCurrentRow(0);
    settle(300);
    const QList<int> back = split2->sizes();
    bool same = back.size() == 3 && insp2->bookmarkPreview()->isVisible();
    for (int i = 0; same && i < 3; ++i)
        same = qAbs(back[i] - shown[i]) <= qMax(8, shown[i] / 10);
    check(same, QStringLiteral("after a restart the three parts have their sizes again"),
          QString::number(back.value(0)) + QLatin1Char('/') + QString::number(back.value(1)) + QLatin1Char('/')
              + QString::number(back.value(2)) + QStringLiteral(" instead of ") + QString::number(shown[0]) + QLatin1Char('/')
              + QString::number(shown[1]) + QLatin1Char('/') + QString::number(shown[2]));
    shot(w2, QStringLiteral("94-sizes-back"));
    w2->closeProject();
    delete w2;
}

// ---- item 93
static void wheel(QWidget *target, int steps)
{
    const QPoint pos = target->rect().center();
    QWheelEvent ev(QPointF(pos), QPointF(target->mapToGlobal(pos)), QPoint(), QPoint(0, 120 * steps), Qt::NoButton, Qt::ControlModifier,
                   Qt::NoScrollPhase, false);
    QApplication::sendEvent(target, &ev);
    settle(80);
}

static void bookmarkZoom(const QString &dir)
{
    QString err;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    auto proj = templates::createProject(novel, dir, QStringLiteral("Zoom"), &err);
    if (!proj)
        return;
    Project *p = proj.get();
    BinderItem *d1 = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("One"));
    BinderItem *d2 = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Two"));
    for (BinderItem *it : {d1, d2}) {
        QTextCursor c(p->docs()->text(it->id));
        c.insertText(QStringLiteral("The wind rose in the night and the sea came up over the rocks."));
    }
    d1->bookmarks.append({d2->title, QStringLiteral("qb://") + d2->id});
    p->projectBookmarks.append({d2->title, QStringLiteral("qb://") + d2->id});
    auto *w = new MainWindow;
    w->resize(1400, 860);
    w->show();
    w->activateWindow();
    (void)QTest::qWaitForWindowActive(w, 2000);
    w->adoptProject(std::move(proj));
    settle(400);
    auto *binder = w->findChild<BinderPanel *>();
    auto *insp = w->findChild<Inspector *>();
    if (!binder || !insp)
        return;
    binder->select({d1}, true);
    settle(200);
    insp->showTab(Inspector::Bookmarks);
    settle(200);
    for (bool separately : {false, true}) {
        const QString tag = separately ? QStringLiteral(" (tabs zoomed separately)") : QStringLiteral(" (one zoom for the tabs)");
        zoom::setPerTab(separately);
        for (const char *k : {zoom::kInspBookmarks, zoom::kInspProjectBookmarks, zoom::kInspPreview})
            zoom::reset(QLatin1String(k));
        insp->bookmarkList()->setCurrentRow(0);
        settle(150);
        auto f = [&](QWidget *x) { return x->font().pointSizeF(); };
        auto pv = [&] { return insp->bookmarkPreview()->document()->defaultFont().pointSizeF(); };
        const double doc0 = f(insp->bookmarkList()), proj0 = f(insp->projectBookmarkList()), pv0 = pv();
        wheel(insp->bookmarkList()->viewport(), 2);
        check(f(insp->bookmarkList()) > doc0 * 1.15 && qFuzzyCompare(f(insp->projectBookmarkList()), proj0) && qFuzzyCompare(pv(), pv0),
              QStringLiteral("Ctrl+wheel over the document bookmarks: only that list grows") + tag);
        const double doc1 = f(insp->bookmarkList());
        wheel(insp->projectBookmarkList()->viewport(), 3);
        check(f(insp->projectBookmarkList()) > proj0 * 1.2 && qFuzzyCompare(f(insp->bookmarkList()), doc1) && qFuzzyCompare(pv(), pv0),
              QStringLiteral("Ctrl+wheel over the project bookmarks: only that list grows") + tag);
        const double proj1 = f(insp->projectBookmarkList());
        wheel(insp->bookmarkPreview()->viewport(), 2);
        check(pv() > pv0 * 1.15 && qFuzzyCompare(f(insp->bookmarkList()), doc1) && qFuzzyCompare(f(insp->projectBookmarkList()), proj1),
              QStringLiteral("Ctrl+wheel over the preview: only the preview grows") + tag);
        check(!qFuzzyCompare(zoom::factor(QLatin1String(zoom::kInspBookmarks)), zoom::factor(QLatin1String(zoom::kInspProjectBookmarks))),
              QStringLiteral("the two lists keep a zoom each") + tag);
        if (separately)
            shot(w, QStringLiteral("93-three-zooms"));
    }
    zoom::setPerTab(false);
    // the zoom list names the part zoomed last
    wheel(insp->projectBookmarkList()->viewport(), 1);
    QLabel *title = nullptr;
    for (QLabel *l : insp->findChildren<QLabel *>())
        if (l->text().startsWith(QLatin1String("Zoom:")))
            title = l;
    check(title && title->text().contains(QLatin1String("Project")), QStringLiteral("the zoom list says which part it zooms: the project list"),
          title ? title->text() : QString());
    w->closeProject();
    delete w;

    // an older settings file: one zoom for both lists is taken for both
    QSettings s;
    s.remove(QStringLiteral("zoom/inspector.bookmarkProjectList"));
    s.remove(QStringLiteral("zoomProjectListMigrated"));
    s.setValue(QStringLiteral("zoom/inspector.bookmarkLists"), 1.5);
    zoom::migrateProjectList();
    check(qFuzzyCompare(zoom::factor(QLatin1String(zoom::kInspProjectBookmarks)), 1.5)
              && qFuzzyCompare(zoom::factor(QLatin1String(zoom::kInspBookmarks)), 1.5),
          QStringLiteral("an older zoom of both lists is taken for both"));
}

// ---- main
int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0950aTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0950aTest"));
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
    zoom::install(); // Ctrl + mouse wheel zooms the part under the mouse, as in the program

    QTemporaryDir work;
    bookmarkSizes(work.path());
    bookmarkZoom(work.path() + QStringLiteral("/z"));
    // @@SECTIONS@@

    std::printf("%d failure(s)\n", failures);
    return failures;
}
