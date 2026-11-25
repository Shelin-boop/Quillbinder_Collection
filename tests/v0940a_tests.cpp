// Scripted run for version 0.94.0 (shared code of the run's to-do list):
//  - item 63: a Quick Reference window (and any window that closes itself) is not deleted while one of its own menus
//    runs; the close request is held and carried out after the menu;
//  - item 64: a new mind map note gets the keyboard focus in its editor;
//  - item 88: documents moved to the Trash leave a corkboard that showed them;
//  - item 32: after a choice in the binder the inspector shows the chosen document;
//  - item 73: imported text, a paste of more than 200 words and the first opening of a project are no session words;
//    Reset Session Now refreshes the tooltip of the quick search field at once;
//  - item 85: (moved to qb_v0940e_tests: Back Up Now's message in the message line);
//  - item 89: Project Targets keeps the room of its bars; a small screen opens the window maximized the first time;
//    the window's place is saved when it changes.
//
//   xvfb-run -a ./qb_v0940a_tests /tmp/shots
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

static QAction *byKey(QWidget *w, const QString &key) { return w->findChild<QAction *>(key); }

static QString words(int n)
{
    QStringList l;
    for (int i = 0; i < n; ++i)
        l << QStringLiteral("word%1").arg(i % 50);
    return l.join(QLatin1Char(' '));
}

// ---------------------------------------------------------------- item 63
static void menuGuard(MainWindow &w, BinderPanel *binder, BinderItem *d1)
{
    std::printf("== a window is not deleted while its own menu runs (item 63)\n");
    binder->select({d1}, true);
    binder->tree()->setFocus();
    settle(200);
    byKey(&w, QStringLiteral("quickRef"))->trigger();
    settle(500);
    QPointer<QuickRefWindow> qr;
    for (QWidget *tl : QApplication::topLevelWidgets())
        if (auto *q = qobject_cast<QuickRefWindow *>(tl))
            if (q->isVisible())
                qr = q;
    check(qr != nullptr, QStringLiteral("(a Quick Reference window is open)"));
    if (!qr)
        return;
    auto *icon = qr->findChild<QToolButton *>(QStringLiteral("quickRefIcon"));
    check(icon != nullptr, QStringLiteral("(its document icon is found)"));
    if (!icon)
        return;
    bool menuShown = false, alive = false, stillVisible = false;
    QTimer::singleShot(400, qApp, [&] {
        auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        menuShown = m != nullptr;
        if (qr)
            qr->close(); // the window manager's close request, while the menu is open
        QTimer::singleShot(400, qApp, [&] {
            alive = !qr.isNull();
            stillVisible = alive && qr->isVisible();
            if (auto *m2 = qobject_cast<QMenu *>(QApplication::activePopupWidget()))
                m2->close();
        });
    });
    icon->click(); // runs the menu until it is closed
    check(menuShown, QStringLiteral("the document icon of the header opens its menu"));
    check(alive && stillVisible, QStringLiteral("a close request while the menu is open is held: the window is still there"));
    settle(300);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    check(qr.isNull(), QStringLiteral("... and it is closed once the menu has ended"));

    // the same for any window that deletes itself on closing and runs a menu of its own
    auto *win = new QWidget(nullptr, Qt::Window);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->resize(300, 200);
    win->show();
    QPointer<QWidget> guarded(win);
    QMenu menu;
    menu.addAction(QStringLiteral("One"));
    bool heldAlive = false;
    QTimer::singleShot(300, qApp, [&] {
        if (guarded)
            guarded->close();
        QTimer::singleShot(300, qApp, [&] {
            heldAlive = !guarded.isNull();
            menu.close();
        });
    });
    qb::execGuarded(menu, win->mapToGlobal(QPoint(10, 10)), win);
    check(heldAlive, QStringLiteral("execGuarded: a window with Qt::WA_DeleteOnClose survives a close request during its menu"));
    settle(200);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    check(guarded.isNull(), QStringLiteral("execGuarded: it is closed after the menu"));
    // without a close request nothing is closed
    auto *win2 = new QWidget(nullptr, Qt::Window);
    win2->setAttribute(Qt::WA_DeleteOnClose);
    win2->show();
    QPointer<QWidget> g2(win2);
    QMenu menu2;
    menu2.addAction(QStringLiteral("One"));
    QTimer::singleShot(200, qApp, [&] { menu2.close(); });
    qb::execGuarded(menu2, win2->mapToGlobal(QPoint(10, 10)), win2);
    settle(100);
    check(!g2.isNull() && g2->isVisible(), QStringLiteral("execGuarded: a window with no close request stays open"));
    delete g2.data();
}

// ---------------------------------------------------------------- item 64
static void mindMapFocus(MainWindow &w, BinderPanel *binder, Inspector *inspector, BinderItem *folder)
{
    std::printf("== a new mind map note has the keyboard focus (item 64)\n");
    binder->select({folder}, true);
    settle(300);
    auto *btn = inspector->findChild<QToolButton *>(QStringLiteral("inspectorMindMapButton"));
    if (btn)
        btn->click();
    settle(300);
    MindMapPanel *panel = inspector->mindMapPanel();
    MindMapDoc *doc = panel ? panel->doc() : nullptr;
    MindMapView *view = panel ? panel->view() : nullptr;
    MindMapController *ctl = panel ? panel->controller() : nullptr;
    check(doc && view && ctl && panel->isVisible(), QStringLiteral("(the mind map of the inspector is shown)"));
    if (!doc || !view || !ctl)
        return;
    w.activateWindow();
    binder->tree()->setFocus();
    settle(200);
    check(QApplication::focusWidget() == binder->tree() || binder->tree()->hasFocus(), QStringLiteral("(the focus is in the binder)"));
    ctl->action(QStringLiteral("mm_newNote"))->trigger();
    settle(200);
    QWidget *fw = QApplication::focusWidget();
    check(fw && (fw == view || fw == view->viewport()), QStringLiteral("New Note from the menu: the focus is in the mind map"),
          fw ? QString::fromLatin1(fw->metaObject()->className()) : QStringLiteral("(none)"));
    MindMapScene *scene = doc->scene();
    check(scene->editingNote() != nullptr, QStringLiteral("... and the new note is in edit mode"));
    if (fw)
        QTest::keyClicks(fw, QStringLiteral("Lamp"));
    settle(100);
    const auto *editing = scene->editingNote();
    check(editing && editing->textItem()->toPlainText() == QLatin1String("Lamp"),
          QStringLiteral("... and what is typed replaces the selected text of the note"),
          editing ? editing->textItem()->toPlainText() : QString());
    shot(&w, QStringLiteral("v0940a-mindmap-new-note"));
    scene->endEditing();
    // a second note from the binder's focus again, by a double-click on the board
    binder->tree()->setFocus();
    settle(100);
    ctl->newNoteAt(QPointF(200, 120));
    settle(150);
    fw = QApplication::focusWidget();
    check(fw && (fw == view || fw == view->viewport()), QStringLiteral("a note made at a place gets the focus too"));
    scene->endEditing();
}

// ---------------------------------------------------------------- item 88
static void corkboardTrash(MainWindow &w, Project *p, BinderPanel *binder, BinderItem *a, BinderItem *b, BinderItem *c,
                           BinderItem *folder)
{
    std::printf("== documents moved to the Trash leave the corkboard (item 88)\n");
    EditorPane *pane = w.editorPane(0);
    binder->select({a, b, c}, true);
    settle(300);
    pane->setViewMode(ViewMode::Corkboard);
    settle(300);
    check(pane->mode() == ViewMode::Corkboard && pane->corkboard()->shownItems().size() == 3,
          QStringLiteral("(three documents are shown as cards)"), QString::number(pane->corkboard()->shownItems().size()));
    shot(&w, QStringLiteral("v0940a-corkboard-three"));
    binder->tree()->setFocus();
    byKey(&w, QStringLiteral("trash"))->trigger();
    settle(500);
    check(pane->corkboard()->shownItems().isEmpty() && pane->items().isEmpty(),
          QStringLiteral("Delete: the three cards are gone at once, no other choice is needed"),
          QString::number(pane->corkboard()->shownItems().size()));
    shot(&w, QStringLiteral("v0940a-corkboard-after-trash"));
    // the same through the corkboard of a folder
    const auto kids = folder->children;
    binder->select({folder}, true);
    settle(300);
    pane->setViewMode(ViewMode::Corkboard);
    settle(300);
    const int before = pane->corkboard()->shownItems().size();
    pane->corkboard()->selectItems(pane->corkboard()->shownItems());
    pane->corkboard()->setFocus();
    settle(100);
    byKey(&w, QStringLiteral("trash"))->trigger();
    settle(500);
    check(before > 0 && pane->corkboard()->shownItems().isEmpty(),
          QStringLiteral("a folder's corkboard with all its cards moved to the Trash shows none"),
          QStringLiteral("%1 -> %2").arg(before).arg(pane->corkboard()->shownItems().size()));
    pane->setViewMode(ViewMode::Single);
    Q_UNUSED(p);
    Q_UNUSED(kids);
}

// ---------------------------------------------------------------- item 32
static void inspectorFollows(MainWindow &w, BinderPanel *binder, Inspector *inspector, BinderItem *d1, BinderItem *d2, BinderItem *d3)
{
    std::printf("== the inspector shows what the binder chose (item 32)\n");
    binder->select({d3}, true);
    settle(300);
    EditorPane *pane = w.editorPane(0);
    pane->showItems({d1}); // the editor shows d1 ...
    settle(200);
    inspector->setItem(d2); // ... while the inspector shows another document
    settle(100);
    check(inspector->item() == d2 && !pane->items().isEmpty() && pane->items().first() == d1,
          QStringLiteral("(editor on one document, inspector on another)"));
    binder->select({d1}, true);
    settle(300);
    check(inspector->item() == d1, QStringLiteral("a click in the binder on the document the editor shows moves the inspector to it"),
          inspector->item() ? inspector->item()->title : QStringLiteral("(none)"));
    binder->select({d2}, true);
    settle(300);
    check(inspector->item() == d2 && pane->items().first() == d2,
          QStringLiteral("a choice of another document moves editor and inspector as before"));
}

// ---------------------------------------------------------------- item 73
static void sessionWords(MainWindow &w, Project *p, BinderItem *doc, const QString &work)
{
    std::printf("== session words (item 73)\n");
    p->startSession(true);
    check(p->sessionWords() == 0, QStringLiteral("(a new session starts at 0)"));
    // typed words count
    {
        QTextCursor c(p->docs()->text(doc->id));
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" ") + words(10));
    }
    settle(1000);
    check(p->sessionWords() == 10, QStringLiteral("ten typed words are ten session words"), QString::number(p->sessionWords()));
    // a paste of 300 words does not
    {
        QTextCursor c(p->docs()->text(doc->id));
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" ") + words(300));
    }
    settle(1000);
    check(p->sessionWords() == 10, QStringLiteral("a paste of 300 words is no session words"), QString::number(p->sessionWords()));
    {
        QTextCursor c(p->docs()->text(doc->id));
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" ") + words(150));
    }
    settle(1000);
    check(p->sessionWords() == 160, QStringLiteral("a paste of 150 words counts"), QString::number(p->sessionWords()));
    // Import and Split
    {
        QFile f(work + QStringLiteral("/long.txt"));
        f.open(QIODevice::WriteOnly);
        for (int i = 0; i < 3; ++i) {
            f.write(words(400).toUtf8());
            f.write("\n\n***\n\n");
        }
        f.close();
        const int before = p->sessionWords();
        QString err;
        const auto items = importer::importAndSplit(p, f.fileName(), QStringLiteral("***"), false, p->draft(), -1, &err);
        settle(1000);
        check(!items.isEmpty() && p->draftWordCount() > 1000, QStringLiteral("(a long text was imported and split)"),
              QStringLiteral("%1 documents, %2 words, %3").arg(items.size()).arg(p->draftWordCount()).arg(err));
        check(p->sessionWords() == before, QStringLiteral("Import and Split: the imported words are no session words"),
              QStringLiteral("%1 -> %2").arg(before).arg(p->sessionWords()));
    }
    // Reset Session Now: the tooltip of the quick search field follows at once
    {
        QLineEdit *quick = nullptr;
        for (QLineEdit *e : w.findChildren<QLineEdit *>())
            if (e->toolTip().startsWith(QLatin1String("Draft:")))
                quick = e;
        QTextCursor c(p->docs()->text(doc->id));
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" ") + words(25));
        settle(1000);
        const QString tipBefore = quick ? quick->toolTip() : QString();
        check(quick && tipBefore.contains(QStringLiteral("This session: 185")), QStringLiteral("(the tooltip shows the session's words)"), tipBefore);
        auto *dlg = new TargetsDialog(p, &w);
        dlg->show();
        settle(200);
        QPushButton *reset = nullptr;
        for (QPushButton *b : dlg->findChildren<QPushButton *>())
            if (b->text() == QLatin1String("Reset Session Now"))
                reset = b;
        if (reset)
            reset->click();
        settle(200);
        check(quick && quick->toolTip().contains(QStringLiteral("This session: 0 words")),
              QStringLiteral("Reset Session Now: the tooltip says 0 words at once"), quick ? quick->toolTip() : QString());
        dlg->close();
    }
}

static void firstOpening(const QString &work)
{
    std::printf("== the first opening of a project (item 73)\n");
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work, QStringLiteral("Opening"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return;
    BinderItem *d = proj->model()->createItem(proj->draft(), -1, ItemType::Text, QStringLiteral("Scene"));
    {
        QTextCursor c(proj->docs()->text(d->id));
        c.insertText(words(500));
    }
    proj->docs()->flushCounts();
    proj->docs()->saveDirty(&err);
    d->wordCount = 0; // a project file without counts, as one from another program has
    d->charCount = 0;
    proj->targets.sessionStart = QDateTime();
    proj->save(&err);
    const QString file = proj->displayFile();
    proj.reset();
    QString oerr;
    auto again = Project::open(file, &oerr);
    check(again != nullptr, QStringLiteral("(project opened again)"), oerr);
    if (!again)
        return;
    BinderItem *d2 = nullptr;
    for (BinderItem *it : again->allItems())
        if (it->title == QLatin1String("Scene"))
            d2 = it;
    again->docs()->recount(d2 ? d2->id : QString()); // the text is read when a document is shown
    check(d2 && again->draftWordCount() >= 500 && again->sessionWords() == 0,
          QStringLiteral("the words a project holds when it is opened the first time are no session words"),
          QStringLiteral("draft %1, session %2").arg(again->draftWordCount()).arg(again->sessionWords()));
}

// ---------------------------------------------------------------- item 85
// (0.94.0, reworked: Back Up Now's message is in the message line under the binder; qb_v0940e_tests checks it)

// ---------------------------------------------------------------- item 89
static void targetsLayout(MainWindow &w, Project *p)
{
    std::printf("== Project Targets keeps its room (item 89)\n");
    p->targets.draftTarget = 0;
    p->targets.sessionTarget = 0;
    p->targets.autoFromDeadline = false;
    auto *dlg = new TargetsDialog(p, &w);
    dlg->show();
    settle(300);
    auto spins = dlg->findChildren<QSpinBox *>();
    QSpinBox *draftSpin = spins.value(0), *sessionSpin = spins.value(1);
    check(draftSpin && sessionSpin, QStringLiteral("(the target fields are found)"));
    if (!draftSpin || !sessionSpin) {
        dlg->close();
        return;
    }
    const int y1 = draftSpin->mapTo(dlg, QPoint()).y(), y2 = sessionSpin->mapTo(dlg, QPoint()).y();
    const QSize size1 = dlg->size();
    shot(dlg, QStringLiteral("v0940a-targets-none"));
    draftSpin->setValue(5000);
    sessionSpin->setValue(500);
    settle(300);
    const int y1b = draftSpin->mapTo(dlg, QPoint()).y(), y2b = sessionSpin->mapTo(dlg, QPoint()).y();
    check(y1 == y1b && y2 == y2b && dlg->size() == size1, QStringLiteral("setting a target moves no field of the window"),
          QStringLiteral("%1/%2 -> %3/%4, %5x%6 -> %7x%8")
              .arg(y1).arg(y2).arg(y1b).arg(y2b).arg(size1.width()).arg(size1.height()).arg(dlg->width()).arg(dlg->height()));
    shot(dlg, QStringLiteral("v0940a-targets-set"));
    dlg->close();
}

static void windowPlace(MainWindow &w)
{
    std::printf("== the window's place (item 89)\n");
    QScreen *sc = QGuiApplication::primaryScreen();
    const bool small = sc && sc->availableGeometry().width() < 1600;
    check(w.isMaximized() == small,
          QStringLiteral("the first start opens maximized on a screen narrower than 1600 px (screen %1 px wide)")
              .arg(sc ? sc->availableGeometry().width() : 0),
          QString::number(w.isMaximized()));
    w.showNormal();
    settle(300);
    w.resize(1111, 733);
    settle(1500);
    QSettings s;
    const QByteArray saved = s.value(QStringLiteral("window/geometry")).toByteArray();
    check(!saved.isEmpty() && saved == w.saveGeometry(), QStringLiteral("a change of the window's size is saved after a moment, not only at the end"));
    w.showMaximized();
    settle(1500);
    QSettings s2;
    const QByteArray saved2 = s2.value(QStringLiteral("window/geometry")).toByteArray();
    check(saved2 != saved && saved2 == w.saveGeometry(), QStringLiteral("... and so is maximizing it"));
    w.showNormal();
    settle(300);
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0940aTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0940aTest"));
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

    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("Lamp"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    Project *p = proj.get();
    BinderItem *ch = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Chapter One"));
    BinderItem *d1 = p->model()->createItem(ch, -1, ItemType::Text, QStringLiteral("The Storm"));
    BinderItem *d2 = p->model()->createItem(ch, -1, ItemType::Text, QStringLiteral("The Calm"));
    BinderItem *d3 = p->model()->createItem(ch, -1, ItemType::Text, QStringLiteral("The Ships"));
    BinderItem *ch2 = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Chapter Two"));
    BinderItem *e1 = p->model()->createItem(ch2, -1, ItemType::Text, QStringLiteral("The Bell"));
    BinderItem *e2 = p->model()->createItem(ch2, -1, ItemType::Text, QStringLiteral("The Light"));
    for (BinderItem *it : {d1, d2, d3, e1, e2}) {
        QTextCursor c(p->docs()->text(it->id));
        c.insertText(QStringLiteral("The wind rose in the night and the sea came up over the rocks."));
    }
    p->save(&err);

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);
    auto *binder = w.findChild<BinderPanel *>();
    auto *inspector = w.findChild<Inspector *>();
    if (!binder || !inspector)
        return 1;

    windowPlace(w);
    menuGuard(w, binder, d1);
    mindMapFocus(w, binder, inspector, ch);
    inspectorFollows(w, binder, inspector, d1, d2, d3);
    sessionWords(w, p, d1, work.path());
    firstOpening(work.path() + QStringLiteral("/second"));
    targetsLayout(w, p);
    corkboardTrash(w, p, binder, d2, d3, e1, ch2);

    std::printf("%d failure(s)\n", failures);
    return failures;
}
