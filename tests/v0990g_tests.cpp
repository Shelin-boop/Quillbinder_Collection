// Scripted run for version 0.99.0 (the menu walk): the names of the commands and the shape of the menus.
//  - Navigate > Inspect has short names; Appearance and View name the same switch alike; the Search button's menu shows
//    the Find command itself; no entry starts with a lower-case letter; one capitals rule;
//  - Format > Style lists the style list's styles (at most 25 entries, also with more styles); the item right-click menu
//    leaves out what cannot act; Edit > Select is gone; Show Format Bar is also in View > Text Editing; View > Toolbars and
//    the toolbars' right-click menu have the same groups;
//  - the menus go three levels deep at most (the mind map's Format > Note Style lists the styles right in the menu);
//  - the "..." only on commands that ask first; every command has an Alt letter; Find Command skips the menus that
//    are filled with binder titles; F1 in Project Statistics opens section 10.
//
//   xvfb-run -a ./qb_v0990g_tests /tmp/shots
#include "AppInfo.h"
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "text/Styles.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/CommandFinder.h"
#include "ui/EditorPane.h"
#include "ui/Inspector.h"
#include "ui/LookMenu.h"
#include "ui/MainWindow.h"
#include "ui/ManualWindow.h"
#include "ui/MindMapController.h"
#include "ui/MindMapDoc.h"
#include "ui/MindMapUi.h"
#include "ui/MindMapView.h"
#include "ui/Theme.h"
#include "ui/ToolDialogs.h"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDir>
#include <QElapsedTimer>
#include <QMenu>
#include <QMenuBar>
#include <QSet>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include <cstdio>
#include <functional>
#include <memory>

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

// the text as the writer reads it: without the shortcut, the Alt marks and the ellipsis
static QString plain(QString t)
{
    const int tab = int(t.indexOf(QLatin1Char('\t')));
    if (tab >= 0)
        t.truncate(tab);
    t.remove(QLatin1Char('&'));
    t.remove(QChar(0x2026));
    return t.simplified();
}

// the text without the shortcut only
static QString shown(QString t)
{
    const int tab = int(t.indexOf(QLatin1Char('\t')));
    if (tab >= 0)
        t.truncate(tab);
    return t;
}

static QString mnemonicOf(const QString &text)
{
    const QString main = shown(text);
    for (int i = 0; i + 1 < main.size(); ++i)
        if (main.at(i) == QLatin1Char('&')) {
            if (main.at(i + 1) == QLatin1Char('&')) {
                ++i;
                continue;
            }
            return main.at(i + 1).toLower();
        }
    return {};
}

static QAction *act(MainWindow &w, const char *key) { return w.findChild<QAction *>(QLatin1String(key)); }

static QMenu *topMenu(MainWindow &w, const QString &name)
{
    for (QAction *a : w.menuBar()->actions())
        if (a->menu() && plain(a->text()) == name)
            return a->menu();
    return nullptr;
}

static QMenu *subMenu(QMenu *m, const QString &name)
{
    if (!m)
        return nullptr;
    for (QAction *a : m->actions())
        if (a->menu() && plain(a->text()) == name)
            return a->menu();
    return nullptr;
}

static QStringList texts(QMenu *m)
{
    QStringList out;
    if (m)
        for (QAction *a : m->actions())
            if (!a->isSeparator() && !a->text().isEmpty())
                out << plain(a->text());
    return out;
}

// the entries of a menu as the writer sees them, with a "|" for a separator
static QStringList layout(QMenu *m)
{
    QStringList out;
    if (m)
        for (QAction *a : m->actions())
            out << (a->isSeparator() ? QStringLiteral("|") : plain(a->text()));
    return out;
}

static void collectMenus(QMenu *m, QList<QMenu *> *out)
{
    emit m->aboutToShow();
    settle(20);
    *out << m;
    for (QAction *a : m->actions())
        if (a->menu())
            collectMenus(a->menu(), out);
}

static QList<QMenu *> allMenus(MainWindow &w)
{
    QList<QMenu *> out;
    for (QAction *a : w.menuBar()->actions())
        if (a->menu())
            collectMenus(a->menu(), &out);
    return out;
}

// the number of menus on the longest path from m down (a menu without a submenu: 1)
static int depthOf(QMenu *m)
{
    int deepest = 0;
    for (QAction *a : m->actions())
        if (a->menu()) {
            // (Go To, Back To and the like list the binder's own tree: the writer's data, not menus of commands)
            const QString n = a->menu()->objectName();
            if (n == QLatin1String("goToMenu") || n == QLatin1String("openQuickRefMenu") || n == QLatin1String("historyBackMenu") ||
                n == QLatin1String("historyForwardMenu"))
                continue;
            deepest = qMax(deepest, depthOf(a->menu()));
        }
    return deepest + 1;
}

// the popup menu that opens while fn runs (its entries), closed again
struct Popup
{
    QString name;
    QStringList entries;       // plain texts, "|" for a separator
    QStringList disabled;      // plain texts of the entries that are grayed
};

static Popup popupOf(const std::function<void()> &fn)
{
    auto result = std::make_shared<Popup>();
    auto tries = std::make_shared<int>(0);
    auto poll = std::make_shared<std::function<void()>>();
    *poll = [result, tries, poll] {
        if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
            result->name = m->objectName();
            for (QAction *a : m->actions()) {
                result->entries << (a->isSeparator() ? QStringLiteral("|") : plain(a->text()));
                if (!a->isSeparator() && !a->isEnabled())
                    result->disabled << plain(a->text());
            }
            m->close();
            return;
        }
        if (++*tries < 40)
            QTimer::singleShot(50, qApp, *poll);
    };
    QTimer::singleShot(50, qApp, *poll);
    fn();
    settle(100);
    return *result;
}

// ---------------------------------------------------------------- names
static void names(MainWindow &w)
{
    std::printf("== names of commands\n");
    // Navigate > Inspect: short names
    QMenu *inspect = subMenu(topMenu(w, QStringLiteral("Navigate")), QStringLiteral("Inspect"));
    emit inspect->aboutToShow();
    QStringList tabs;
    QString longest;
    bool dots = false;
    for (int i = 0; i < 7; ++i)
        if (QAction *a = act(w, qPrintable(QStringLiteral("inspect%1").arg(i)))) {
            const QString t = plain(a->text());
            tabs << t;
            if (t.size() > longest.size())
                longest = t;
            dots = dots || a->text().endsWith(QLatin1String("...")) || a->text().endsWith(QChar(0x2026));
        }
    check(tabs.size() == 7 && longest.size() <= 25 && !dots, QStringLiteral("Navigate > Inspect: no name longer than 25 characters, none ends in \"...\""),
          tabs.join(QStringLiteral(" | ")));
    check(tabs == QStringList({QStringLiteral("Synopsis and Notes"), QStringLiteral("Bookmarks"), QStringLiteral("Metadata"),
                               QStringLiteral("Snapshots"), QStringLiteral("Comments and Footnotes"), QStringLiteral("Mind Map"),
                               QStringLiteral("Objects")}),
          QStringLiteral("Navigate > Inspect: Synopsis and Notes, Bookmarks, Metadata, Snapshots, Comments and Footnotes, Mind Map, Objects"),
          tabs.join(QStringLiteral(" | ")));

    // Paste: one name for each of the two commands
    QAction *applyFormat = act(w, "pasteFormat");
    QAction *keep = act(w, "pasteKeep");
    check(applyFormat && plain(applyFormat->text()) == QLatin1String("Apply Copied Formatting") &&
              applyFormat->shortcut() == QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_V),
          QStringLiteral("Format: \"Apply Copied Formatting\" (Ctrl+Alt+V), not \"Paste Formatting\" beside \"Paste With Formatting\""),
          applyFormat ? applyFormat->text() : QString());
    check(keep && plain(keep->text()) == QLatin1String("Paste With Formatting"), QStringLiteral("Edit: \"Paste With Formatting\""),
          keep ? keep->text() : QString());

    // one capitals rule
    struct Name { const char *key; const char *text; };
    for (const Name &n : {Name{"settingsImport", "Load Project Settings From File"}, Name{"iconFromText", "Icon From Text"},
                          Name{"snapshotTitle", "Take Snapshot With Title"}, Name{"image", "Picture From File"},
                          Name{"defaultTextFormatting", "Project Formatting Settings"}, Name{"toText", "Convert to Text Document"},
                          Name{"sheetToObject", "Convert Sheet to Object"}, Name{"selectAnnotation", "Select Annotation or Note"}}) {
        QAction *a = act(w, n.key);
        check(a && plain(a->text()) == QLatin1String(n.text), QStringLiteral("%1 is called \"%2\"").arg(QLatin1String(n.key), QLatin1String(n.text)),
              a ? a->text() : QStringLiteral("(no such command)"));
    }
    emit act(w, "integrate")->changed();
    check(act(w, "integrate") && (plain(act(w, "integrate")->text()) == QLatin1String("Add to Application Menu") ||
                                  plain(act(w, "integrate")->text()) == QLatin1String("Remove From Application Menu")),
          QStringLiteral("Help: \"Add to Application Menu\" / \"Remove From Application Menu\""), act(w, "integrate")->text());

    // the sub-menus of Navigate > Open and Documents > Split: every entry starts with a capital
    QMenu *open = subMenu(topMenu(w, QStringLiteral("Navigate")), QStringLiteral("Open"));
    check(texts(open) == QStringList({QStringLiteral("In Editor"), QStringLiteral("In Other Editor"), QStringLiteral("In Copyholder"),
                                      QStringLiteral("As Quick Reference")}),
          QStringLiteral("Navigate > Open: In Editor, In Other Editor, In Copyholder, As Quick Reference"), texts(open).join(QStringLiteral(" | ")));
    QMenu *split = subMenu(topMenu(w, QStringLiteral("Documents")), QStringLiteral("Split"));
    check(texts(split) == QStringList({QStringLiteral("At Selection"), QStringLiteral("With Selection as Title")}),
          QStringLiteral("Documents > Split: At Selection, With Selection as Title"), texts(split).join(QStringLiteral(" | ")));
    QMenu *move = subMenu(topMenu(w, QStringLiteral("Edit")), QStringLiteral("Move"));
    check(texts(move) == QStringList({QStringLiteral("Move Up"), QStringLiteral("Move Down"), QStringLiteral("Move Left"), QStringLiteral("Move Right")}) &&
              move->toolTipsVisible() && act(w, "moveLeft")->toolTip() == QLatin1String("Out of the folder") &&
              act(w, "moveRight")->toolTip() == QLatin1String("Into the item above"),
          QStringLiteral("Edit > Move: Move Left and Move Right, the tooltips say \"Out of the folder\" and \"Into the item above\""),
          texts(move).join(QStringLiteral(" | ")));

    // Edit > Select is gone
    QMenu *edit = topMenu(w, QStringLiteral("Edit"));
    const QStringList editTexts = texts(edit);
    const int selectAll = int(editTexts.indexOf(QStringLiteral("Select All")));
    check(!editTexts.contains(QStringLiteral("Select")) && selectAll >= 0 && editTexts.value(selectAll + 1) == QLatin1String("Select Annotation or Note"),
          QStringLiteral("Edit: no \"Select\" menu of one entry; \"Select Annotation or Note\" follows Select All"), editTexts.join(QStringLiteral(" | ")));

    // line spacing: the menu has 1.09 as the format bar's list has
    QMenu *para = subMenu(topMenu(w, QStringLiteral("Format")), QStringLiteral("Paragraph"));
    QMenu *lines = subMenu(para, QStringLiteral("Line Spacing"));
    const QStringList lineTexts = texts(lines);
    bool has109 = false;
    for (const QString &t : lineTexts)
        has109 = has109 || t.contains(QLatin1String("1.09")) || t.contains(QLatin1String("1,09"));
    check(has109 && lineTexts.size() >= 7, QStringLiteral("Format > Paragraph > Line Spacing lists 1.09 as the format bar does"), lineTexts.join(QStringLiteral(" | ")));
}

// the same switch, the same name: Appearance and View; the Search button; the buttons' menus
static void sameNames(MainWindow &w)
{
    std::printf("== one name for one switch\n");
    struct Pair { lookmenu::Part part; const char *look; const char *key; };
    for (const Pair &p : {Pair{lookmenu::Part::Editor, "lookFixedWidth", "fixedWidth"}, Pair{lookmenu::Part::Binder, "lookLabelBinder", "labelBinder"},
                          Pair{lookmenu::Part::Binder, "lookLabelIcons", "labelIcons"}, Pair{lookmenu::Part::Corkboard, "lookStamps", "stamps"},
                          Pair{lookmenu::Part::Corkboard, "lookTint", "tintCards"}}) {
        QMenu m;
        lookmenu::fill(&m, p.part, &w);
        QAction *look = m.findChild<QAction *>(QLatin1String(p.look));
        QAction *view = act(w, p.key);
        check(look && view && plain(look->text()) == plain(view->text()),
              QStringLiteral("Appearance \"%1\" is called as View calls it").arg(QLatin1String(p.key)),
              QStringLiteral("%1 / %2").arg(look ? look->text() : QStringLiteral("(none)"), view ? view->text() : QStringLiteral("(none)")));
    }
    {
        QMenu m;
        lookmenu::fill(&m, lookmenu::Part::Editor, &w);
        QStringList seen;
        int found = 0;
        std::function<void(QMenu *, bool)> walk = [&](QMenu *x, bool sizes) {
            for (QAction *a : x->actions()) {
                const QString t = a->text();
                const bool ours = t.startsWith(QLatin1String("Choose Font")) || t.startsWith(QLatin1String("Appearance Options")) ||
                                  (sizes && t.startsWith(QLatin1String("Other")));
                if (ours) {
                    ++found;
                    if (t.contains(QLatin1String("...")) || !t.contains(QChar(0x2026)))
                        seen << t;
                }
                if (a->menu())
                    walk(a->menu(), a->menu()->objectName() == QLatin1String("lookEditorWidth") || a->menu()->objectName() == QLatin1String("lookBorder"));
            }
        };
        walk(&m, false);
        check(found >= 4 && seen.isEmpty(), QStringLiteral("the Appearance menu writes \"…\" (one character) in Other, Choose Font and Appearance Options (%1 looked at)").arg(found),
              seen.join(QStringLiteral(" | ")));
    }
    // the Search button: the Find command itself
    QToolButton *searchBtn = w.findChild<QToolButton *>(QStringLiteral("searchButton"));
    QMenu *sm = searchBtn ? searchBtn->menu() : nullptr;
    QAction *find = act(w, "find");
    check(sm && find && sm->actions().contains(find), QStringLiteral("the Search button's menu holds the Find command itself (\"Find…\")"),
          sm ? layout(sm).join(QStringLiteral(" | ")) : QString());
    // every entry of a button's menu has the text of its command
    int checkedEntries = 0;
    QStringList differing;
    for (QToolButton *b : w.findChildren<QToolButton *>())
        if (QMenu *m = b->menu())
            for (QAction *a : m->actions()) {
                if (a->isSeparator() || a->menu())
                    continue;
                auto *original = qobject_cast<QAction *>(a->property("qbAliasOf").value<QObject *>());
                if (!original)
                    continue;
                ++checkedEntries;
                if (plain(a->text()) != plain(original->text()))
                    differing << QStringLiteral("%1 / %2").arg(a->text(), original->text());
            }
    check(differing.isEmpty(), QStringLiteral("no entry of a toolbar button's menu has another text than its command (%1 aliases looked at)").arg(checkedEntries),
          differing.join(QStringLiteral("; ")));
    // Appearance after Ignore: the submenu goes in front of the entry given
    {
        QMenu top;
        top.addAction(QStringLiteral("Ignore"));
        top.addSeparator();
        QMenu *more = top.addMenu(QStringLiteral("More Commands"));
        QAction *sep = top.actions().at(1);
        QMenu *look = lookmenu::add(&top, lookmenu::Part::Editor, &w, sep);
        const QStringList l = layout(&top);
        check(look && l == QStringList({QStringLiteral("Ignore"), QStringLiteral("Appearance"), QStringLiteral("|"), QStringLiteral("More Commands")}) &&
                  !subMenu(more, QStringLiteral("Appearance")),
              QStringLiteral("a misspelled word's short menu: Appearance right after Ignore, not inside More Commands"), l.join(QStringLiteral(" | ")));
    }
}

// ---------------------------------------------------------------- shape of the menus
static void shapes(MainWindow &w, Project *p, BinderPanel *binder)
{
    std::printf("== shape of the menus\n");
    // Format > Style: the style list's styles, at most 25 entries, also with three styles of the project's own
    {
        StyleSheet sheet = styles::sheet(p->formatting);
        for (int i = 1; i <= 3; ++i) {
            Style s;
            s.name = QStringLiteral("Own Style %1").arg(i);
            sheet.styles << s;
        }
        p->formatting = styles::withSheet(p->formatting, sheet);
        p->notifySettingsChanged();
        settle(200);
        QMenu *styleMenu = subMenu(topMenu(w, QStringLiteral("Format")), QStringLiteral("Style"));
        emit styleMenu->aboutToShow();
        settle(50);
        int entries = 0;
        for (QAction *a : styleMenu->actions())
            if (!a->isSeparator() && !a->text().isEmpty())
                ++entries;
        check(entries > 0 && entries <= 25, QStringLiteral("Format > Style has at most 25 entries with the standard styles and three of the project's own (%1)").arg(entries),
              layout(styleMenu).join(QStringLiteral(" | ")));
        // the format bar's list is the same list
        shot(styleMenu, QStringLiteral("format-style-menu"));
    }
    // the item right-click menu: only what can act
    {
        BinderItem *ch = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Chapter"));
        BinderItem *d1 = p->model()->createItem(ch, -1, ItemType::Text, QStringLiteral("Scene One"));
        p->model()->createItem(ch, -1, ItemType::Text, QStringLiteral("Scene Two"));
        settle(200);
        binder->select({d1}, true);
        settle(300);
        QMenu *gear = nullptr;
        for (QToolButton *b : binder->findChildren<QToolButton *>())
            if (b->menu() && b->menu()->objectName().isEmpty() && b->popupMode() != QToolButton::DelayedPopup) {
                emit b->menu()->aboutToShow();
                if (texts(b->menu()).contains(QStringLiteral("Rename")))
                    gear = b->menu();
            }
        if (gear)
            shot(gear, QStringLiteral("item-menu-one-text"));
        QStringList gray;
        for (QAction *a : gear ? gear->actions() : QList<QAction *>())
            if (!a->isSeparator() && !a->isEnabled() && !a->text().isEmpty())
                gray << plain(a->text());
        const QStringList all = texts(gear);
        bool leftOut = true;
        for (const char *t : {"Convert to Text Document", "Merge Documents", "Convert Sheet to Object"})
            leftOut = leftOut && !gray.contains(QLatin1String(t));
        check(gear && all.contains(QStringLiteral("Rename")) && leftOut, QStringLiteral("the item menu of one text document shows no grayed Convert, Merge or Sheet entry"),
              QStringLiteral("grayed: %1; all: %2").arg(gray.join(QStringLiteral(" | ")), all.join(QStringLiteral(" | "))));
        check(all.contains(QStringLiteral("Convert to Folder")) || !all.isEmpty(), QStringLiteral("(the menu is filled)"));
        // two documents: Merge is there
        binder->select({d1, ch->children.last()}, true);
        settle(300);
        emit gear->aboutToShow();
        check(texts(gear).contains(QStringLiteral("Merge Documents")), QStringLiteral("with two documents selected, Merge is in the item menu"), texts(gear).join(QStringLiteral(" | ")));
        // a folder: Convert to Text Document
        binder->select({ch}, true);
        settle(300);
        emit gear->aboutToShow();
        check(texts(gear).contains(QStringLiteral("Convert to Text Document")) && !texts(gear).contains(QStringLiteral("Convert to Folder")),
              QStringLiteral("with a folder selected, Convert to Text Document is in the item menu and Convert to Folder is not"), texts(gear).join(QStringLiteral(" | ")));
        // the Documents menu keeps them, grayed
        QMenu *convert = subMenu(topMenu(w, QStringLiteral("Documents")), QStringLiteral("Convert"));
        check(texts(convert).contains(QStringLiteral("Convert to Folder")) && texts(convert).contains(QStringLiteral("Convert to Text Document")) &&
                  texts(convert).contains(QStringLiteral("Convert Sheet to Object")),
              QStringLiteral("Documents > Convert keeps all its entries (grayed when they cannot act)"), texts(convert).join(QStringLiteral(" | ")));
        binder->select({d1}, true);
        settle(200);
    }
    // View > Text Editing: Show Format Bar after Show Ruler
    {
        const QStringList te = texts(subMenu(topMenu(w, QStringLiteral("View")), QStringLiteral("Text Editing")));
        const int ruler = int(te.indexOf(QStringLiteral("Show Ruler")));
        check(ruler >= 0 && te.value(ruler + 1) == QLatin1String("Show Format Bar"), QStringLiteral("View > Text Editing: Show Format Bar follows Show Ruler"), te.join(QStringLiteral(" | ")));
    }
    // View > Toolbars and the toolbars' right-click menu: the same groups
    {
        QMenu *bars = subMenu(topMenu(w, QStringLiteral("View")), QStringLiteral("Toolbars"));
        const QStringList want = {QStringLiteral("Show Toolbar"), QStringLiteral("Main"), QStringLiteral("Themes"), QStringLiteral("Profiles"),
                                  QStringLiteral("Search"), QStringLiteral("Tools"), QStringLiteral("|"), QStringLiteral("Show Format Bar"),
                                  QStringLiteral("Style and Font"), QStringLiteral("Character and Alignment"), QStringLiteral("Paragraph and Insert"),
                                  QStringLiteral("|"), QStringLiteral("Icon Bar"), QStringLiteral("Labels"), QStringLiteral("|"),
                                  QStringLiteral("Toolbar Size"), QStringLiteral("Handwriting Button"), QStringLiteral("|"), QStringLiteral("Reset Toolbar Places"), QStringLiteral("|"), QStringLiteral("Customize Toolbars"),
                                  QStringLiteral("Choose Icons for the Icon Bar")};
        const QStringList got = layout(bars);
        check(got == want, QStringLiteral("View > Toolbars: Show Toolbar with its bars, Show Format Bar with its bars, Icon Bar and Labels, Reset, Customize"),
              got.join(QStringLiteral(" | ")));
        QToolBar *tb = w.findChild<QToolBar *>(QStringLiteral("mainToolbar"));
        if (!tb)
            tb = w.findChild<QToolBar *>();
        Popup pop = popupOf([&] {
            QContextMenuEvent ev(QContextMenuEvent::Mouse, QPoint(3, 3), tb->mapToGlobal(QPoint(3, 3)));
            QApplication::sendEvent(tb, &ev);
        });
        QStringList head = pop.entries;
        // (the right-click menu of a bar has "Customize This Toolbar" where the View menu has "Customize Toolbars")
        QStringList wantHead = want;
        // (and no separator before Reset Toolbar Places)
        const int r = int(wantHead.indexOf(QStringLiteral("Reset Toolbar Places")));
        if (r > 0 && wantHead[r - 1] == QLatin1String("|"))
            wantHead.removeAt(r - 1);
        int c = int(wantHead.indexOf(QStringLiteral("Customize Toolbars")));
        if (c >= 0)
            wantHead[c] = QStringLiteral("Customize This Toolbar");
        check(pop.name == QLatin1String("barsMenu") && head == wantHead, QStringLiteral("the toolbars' right-click menu has the same groups"),
              QStringLiteral("%1: %2").arg(pop.name, head.join(QStringLiteral(" | "))));
    }
    // menus go three levels deep at most
    {
        int deepest = 0;
        QString where;
        for (QAction *a : w.menuBar()->actions())
            if (a->menu()) {
                QList<QMenu *> all;
                collectMenus(a->menu(), &all);
                const int d = depthOf(a->menu());
                if (d > deepest) {
                    deepest = d;
                    where = plain(a->text());
                }
            }
        check(deepest <= 3, QStringLiteral("the menus of the main window go three levels deep at most (%1 in %2)").arg(deepest).arg(where));
    }
}

// the mind map's menus (inspector): Format > Note Style lists the styles right in it
static void mindMap(MainWindow &w, Project *p, BinderPanel *binder)
{
    std::printf("== the mind map's Format menu\n");
    Inspector *inspector = w.findChild<Inspector *>();
    BinderItem *d = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Map Holder"));
    settle(200);
    binder->select({d}, true);
    settle(300);
    inspector->showTab(Inspector::MindMap);
    settle(300);
    MindMapPanel *panel = inspector->mindMapPanel();
    MindMapController *ctl = panel ? panel->controller() : nullptr;
    check(ctl != nullptr, QStringLiteral("(the mind map's controller exists)"));
    if (!ctl)
        return;
    QMenu menu;
    ctl->fillFormatMenu(&menu);
    QMenu *noteStyle = nullptr;
    for (QAction *a : menu.actions())
        if (a->menu() && plain(a->text()) == QLatin1String("Note Style"))
            noteStyle = a->menu();
    check(noteStyle != nullptr, QStringLiteral("Format has Note Style"));
    if (!noteStyle)
        return;
    emit noteStyle->aboutToShow();
    const QStringList l = layout(noteStyle);
    int subs = 0;
    for (QAction *a : noteStyle->actions())
        if (a->menu())
            ++subs;
    check(depthOf(noteStyle) == 1 && subs == 0, QStringLiteral("Format > Note Style: no submenu below it (the Menu button's menus stay three levels deep)"),
          l.join(QStringLiteral(" | ")));
    const int newAt = int(l.indexOf(QStringLiteral("New Note Style From Selection")));
    check(l.size() > 6 && l.contains(QStringLiteral("Redefine Note Style From Selection")) && l.contains(QStringLiteral("Delete Note Style")) &&
              newAt > 1,
          QStringLiteral("Note Style: the styles, then New, Redefine and Delete"), l.join(QStringLiteral(" | ")));
}

// ---------------------------------------------------------------- every menu
static void audit(MainWindow &w)
{
    std::printf("== every menu\n");
    const QList<QMenu *> menus = allMenus(w);
    settle(400); // (the letters of lists filled on opening are repaired after a pause)

    QStringList lower, noLetter, dots, threeDots;
    int count = 0;
    for (QMenu *m : menus) {
        const QString menuName = plain(m->title());
        QSet<QChar> taken; // the letters of the menu that are in use: a command can only lack one when none is free
        for (QAction *a : m->actions())
            if (!a->isSeparator() && !mnemonicOf(a->text()).isEmpty())
                taken.insert(mnemonicOf(a->text()).at(0));
        for (QAction *a : m->actions()) {
            if (a->isSeparator() || a->text().isEmpty())
                continue;
            ++count;
            const QString t = plain(a->text());
            if (!t.isEmpty() && t.at(0).isLower() && a->objectName().isEmpty() == false)
                lower << QStringLiteral("%1 > %2").arg(menuName, t);
            const QString name = a->objectName();
            const bool command = (!name.isEmpty() && !name.contains(QLatin1Char(':')) && !name.startsWith(QLatin1String("qb"))) ||
                                 a->property("qbWantsMnemonic").toBool();
            bool free = false;
            for (const QChar c : t)
                free = free || (c.isLetterOrNumber() && !taken.contains(c.toLower()));
            if (command && mnemonicOf(a->text()).isEmpty() && free)
                noLetter << QStringLiteral("%1 > %2").arg(menuName, t);
            if (a->text().contains(QLatin1String("...")))
                threeDots << QStringLiteral("%1 > %2").arg(menuName, t);
        }
    }
    // the entries that stand for commands start with a capital (the lists of documents, files and themes are the writer's words)
    QStringList lowerAll;
    for (QMenu *m : menus)
        for (QAction *a : m->actions())
            if (!a->isSeparator() && !a->text().isEmpty() && a->objectName().isEmpty() == false && !a->objectName().contains(QLatin1Char(':'))) {
                const QString t = plain(a->text());
                if (!t.isEmpty() && t.at(0).isLower())
                    lowerAll << QStringLiteral("%1 > %2").arg(plain(m->title()), t);
            }
    QStringList aliasesLower;
    for (QMenu *m : menus)
        for (QAction *a : m->actions())
            if (!a->isSeparator() && a->property("qbAliasOf").isValid()) {
                const QString t = plain(a->text());
                if (!t.isEmpty() && t.at(0).isLower())
                    aliasesLower << QStringLiteral("%1 > %2").arg(plain(m->title()), t);
            }
    check(lowerAll.isEmpty() && aliasesLower.isEmpty(), QStringLiteral("no command in any menu starts with a lower-case letter (%1 entries)").arg(count),
          (lowerAll + aliasesLower).join(QStringLiteral("; ")));
    check(noLetter.isEmpty(), QStringLiteral("every command has an Alt letter (the lists of documents, styles and files stay without)"), noLetter.join(QStringLiteral("; ")));
    check(threeDots.isEmpty(), QStringLiteral("no menu entry ends in three periods"), threeDots.join(QStringLiteral("; ")));

    // no letter twice in a menu
    QStringList twice;
    for (QMenu *m : menus) {
        QSet<QString> used;
        for (QAction *a : m->actions()) {
            if (a->isSeparator())
                continue;
            const QString c = mnemonicOf(a->text());
            if (c.isEmpty())
                continue;
            if (used.contains(c))
                twice << QStringLiteral("%1 > %2").arg(plain(m->title()), plain(a->text()));
            used.insert(c);
        }
    }
    check(twice.isEmpty(), QStringLiteral("no Alt letter twice in one menu"), twice.join(QStringLiteral("; ")));

    // Label and Status: a letter although the names come from the project
    QMenu *docs = topMenu(w, QStringLiteral("Documents"));
    emit docs->aboutToShow();
    settle(300);
    int labelStatus = 0;
    for (QAction *a : docs->actions())
        if (a->menu() && a->property("qbWantsMnemonic").toBool() && !mnemonicOf(a->text()).isEmpty())
            ++labelStatus;
    check(labelStatus == 2, QStringLiteral("Documents > Label and Status have Alt letters"), layout(docs).join(QStringLiteral(" | ")));

    // "…" only where a window asks before the command acts
    for (const char *k : {"reportProblem", "licenses", "systemInfo", "placeholders", "statistics", "history", "printPreview", "objectsWindow"}) {
        QAction *a = act(w, k);
        check(a && !a->text().endsWith(QChar(0x2026)) && !a->text().endsWith(QLatin1String("...")),
              QStringLiteral("%1 only shows something: no \"…\"").arg(QLatin1String(k)), a ? a->text() : QStringLiteral("(no such command)"));
    }
    for (const char *k : {"integrate"}) {
        QAction *a = act(w, k);
        check(a && a->text().endsWith(QChar(0x2026)), QStringLiteral("%1 asks first: \"…\"").arg(QLatin1String(k)), a ? a->text() : QString());
    }

    // Find Command: no entry of the menus that are filled with binder titles
    for (const char *name : {"goToMenu", "openQuickRefMenu", "historyBackMenu", "historyForwardMenu"})
        if (QMenu *m = w.findChild<QMenu *>(QLatin1String(name)))
            emit m->aboutToShow();
    settle(100);
    const QList<CommandFinder::Entry> entries = CommandFinder::collect(w.menuBar());
    int skipped = 0, inFinder = 0;
    QStringList stray;
    for (const char *name : {"goToMenu", "openQuickRefMenu", "historyBackMenu", "historyForwardMenu"})
        if (QMenu *m = w.findChild<QMenu *>(QLatin1String(name))) {
            ++skipped;
            const QString title = plain(m->title());
            for (const CommandFinder::Entry &e : entries)
                if (e.path.endsWith(QLatin1String("> ") + title) || e.path.contains(QLatin1String("> ") + title + QLatin1String(" >"))) {
                    ++inFinder;
                    stray << e.path + QLatin1String(" > ") + e.label;
                }
        }
    check(skipped == 4 && inFinder == 0, QStringLiteral("Find Command lists no entry of Go To, Open Quick Reference, Back To and Forward To (%1 menus, %2 entries)").arg(skipped).arg(inFinder),
          stray.join(QStringLiteral("; ")));
    // ... and nothing twice after a second opening
    for (const char *name : {"goToMenu", "openQuickRefMenu"})
        if (QMenu *m = w.findChild<QMenu *>(QLatin1String(name)))
            emit m->aboutToShow();
    const QList<CommandFinder::Entry> again = CommandFinder::collect(w.menuBar());
    check(again.size() == entries.size(), QStringLiteral("Find Command: the same list after the menus opened again"), QStringLiteral("%1 / %2").arg(entries.size()).arg(again.size()));
}

// ---------------------------------------------------------------- F1
static void help(MainWindow &w, Project *p)
{
    std::printf("== F1 in Project Statistics\n");
    StatisticsDialog d(p, &w);
    const QString section = manualhelp::sectionForWindow(&d);
    check(section == QLatin1String("Targets, statistics, and writing tools"), QStringLiteral("F1 in Project Statistics opens section 10 (Targets, statistics, and writing tools), not the add-on's section"), section);
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    const QString homeDir = home.path() + QLatin1String("/h");
    QDir().mkpath(homeDir);
    qputenv("HOME", homeDir.toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_KEYBOARD_LAYOUT", "us");
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990gTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990gTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    AppSettings &as = AppSettings::instance();
    as.load();
    as.welcomeShown = true;
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();

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
    p->save(&err);
    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);
    BinderPanel *binder = w.findChild<BinderPanel *>();
    check(binder != nullptr, QStringLiteral("(the binder is there)"));
    if (!binder)
        return 1;

    names(w);
    sameNames(w);
    shapes(w, p, binder);
    mindMap(w, p, binder);
    audit(w);
    help(w, p);
    shot(&w, QStringLiteral("main-window"));

    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
