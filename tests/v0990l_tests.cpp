// Checks of 0.99.0 (item 122): the small interface, a general mode for small screens and for use without a keyboard.
//  - the rule for Android (Full, Small, Automatic below 1920 x 1080 physical pixels) as a function of the screen size, and
//    the 9 mm target, tested without an Android build; Linux and Windows show only the check item, off at first;
//  - the mode on and off gives the window back exactly (pane sizes, toolbars, inspector, menu bar), in a wide window and in
//    an upright one;
//  - the round buttons: each does its action; the binder and the notes open and close as slide-in panels; portrait
//    (1080 x 1920: the buttons in a row at the bottom, panels as sheets) and a small landscape (1280 x 800);
//  - record only is off, with the reason, without the speech add-on, and on with it;
//  - a profile keeps the mode.
//
//   xvfb-run -a -s "-screen 0 1920x1080x24" ./qb_v0990l_tests /tmp/shots
#include "compile/PluginManager.h"
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/FocusLine.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/Profiles.h"
#include "ui/SmallInterface.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"
#include "ui/ToolHost.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QMenu>
#include <QMenuBar>
#include <QProcess>
#include <QScreen>
#include <QSettings>
#include <QSplitter>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
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
            std::printf("      %s\n", qPrintable(detail.left(1500)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 250)
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

static const char *kBars[] = {"mainToolbar", "themesBar", "searchBar", "toolsBar", "formatBar", "characterBar", "paragraphBar",
                              "iconBar", "profilesBar", "labelsBar"};

static QMap<QString, bool> barVisibility(MainWindow &w)
{
    QMap<QString, bool> m;
    for (const char *name : kBars)
        if (auto *bar = w.findChild<QToolBar *>(QLatin1String(name)))
            m[QLatin1String(name)] = bar->isVisible();
    return m;
}

static QMap<QString, QRect> barGeometry(MainWindow &w)
{
    QMap<QString, QRect> m;
    for (const char *name : kBars)
        if (auto *bar = w.findChild<QToolBar *>(QLatin1String(name)); bar && bar->isVisible())
            m[QLatin1String(name)] = QRect(bar->mapTo(&w, QPoint(0, 0)), bar->size());
    return m;
}

static QToolButton *button(MainWindow &w, const QString &name)
{
    return qobject_cast<QToolButton *>(w.smallButton(name));
}

static const QStringList kNames = {QStringLiteral("menu"),   QStringLiteral("new"),    QStringLiteral("bold"),   QStringLiteral("italic"),
                                   QStringLiteral("style"),  QStringLiteral("binder"), QStringLiteral("labels"), QStringLiteral("record"),
                                   QStringLiteral("notes")};

static QAction *viewAction(MainWindow &w, const QString &text)
{
    for (QAction *top : w.menuBar()->actions())
        if (top->menu() && top->menu()->title().remove(QLatin1Char('&')) == QLatin1String("View"))
            for (QAction *ac : top->menu()->actions())
                if (ac->text().remove(QLatin1Char('&')) == text)
                    return ac;
    return nullptr;
}

static TextEditor *visibleEditor(MainWindow &w)
{
    QWidget *area = w.findChild<QWidget *>(QStringLiteral("editorArea"));
    for (TextEditor *e : area->findChildren<TextEditor *>())
        if (e->isVisible() && !e->isReadOnly())
            return e;
    return nullptr;
}

// the window's own binder and inspector (other binders and inspectors live in other windows' parts)
static BinderPanel *binderOf(MainWindow &w)
{
    for (BinderPanel *b : w.findChildren<BinderPanel *>()) {
        QWidget *p = b->parentWidget();
        if (p == w.paneSplitter() || p == w.sideSplitter() || (w.smallPanel(false) && p == w.smallPanel(false)->contentArea()))
            return b;
    }
    return nullptr;
}

static Inspector *inspectorOf(MainWindow &w)
{
    for (Inspector *i : w.findChildren<Inspector *>()) {
        QWidget *p = i->parentWidget();
        if (p == w.paneSplitter() || p == w.sideSplitter() || (w.smallPanel(true) && p == w.smallPanel(true)->contentArea()))
            return i;
    }
    return nullptr;
}

static BinderItem *firstDocument(BinderItem *it)
{
    for (BinderItem *c : it->children) {
        if (!c->isFolder())
            return c;
        if (BinderItem *d = firstDocument(c))
            return d;
    }
    return nullptr;
}

static int countItems(BinderItem *it)
{
    int n = 1;
    for (BinderItem *c : it->children)
        n += countItems(c);
    return n;
}

// ------------------------------------------------------------------ the rule

static void rule()
{
    std::printf("== the rule for Android and the finger-sized target\n");
    using smallui::Choice;
    check(smallui::belowFullHd(QSize(1280, 800)) && smallui::belowFullHd(QSize(800, 1280)), QStringLiteral("1280 x 800 is below 1920 x 1080, held either way"));
    check(!smallui::belowFullHd(QSize(1920, 1080)) && !smallui::belowFullHd(QSize(1080, 1920)),
          QStringLiteral("1920 x 1080 is not below it, held either way"));
    check(smallui::belowFullHd(QSize(1919, 1080)) && smallui::belowFullHd(QSize(1920, 1079)),
          QStringLiteral("one pixel less on either side is below it"));
    check(!smallui::belowFullHd(QSize(2560, 1600)) && !smallui::belowFullHd(QSize(3840, 2160)) && !smallui::belowFullHd(QSize(1920, 1200)),
          QStringLiteral("larger screens are not below it"));
    check(smallui::belowFullHd(QSize(2560, 720)), QStringLiteral("a wide, low screen is below it (the short side counts too)"));
    for (const QSize &px : {QSize(1280, 800), QSize(1920, 1080), QSize(3840, 2160)})
        check(!smallui::resolve(Choice::Full, px) && smallui::resolve(Choice::Small, px),
              QStringLiteral("Full never and Small always (%1 x %2)").arg(px.width()).arg(px.height()));
    check(smallui::resolve(Choice::Automatic, QSize(1280, 800)) && !smallui::resolve(Choice::Automatic, QSize(1920, 1080))
              && !smallui::resolve(Choice::Automatic, QSize(2560, 1600)) && smallui::resolve(Choice::Automatic, QSize(1600, 2400 / 2)),
          QStringLiteral("Automatic follows the screen: small below 1920 x 1080 only"));
    check(smallui::choiceFromKey(smallui::choiceKey(Choice::Full)) == Choice::Full && smallui::choiceFromKey(smallui::choiceKey(Choice::Small)) == Choice::Small
              && smallui::choiceFromKey(smallui::choiceKey(Choice::Automatic)) == Choice::Automatic && smallui::choiceFromKey(QString()) == Choice::Automatic
              && smallui::choiceFromKey(QStringLiteral("x")) == Choice::Automatic,
          QStringLiteral("the choice is kept as full, small or auto; anything else is Automatic"));
    check(smallui::touchTargetPx(96) == 34 && smallui::touchTargetPx(254) == 90 && smallui::touchTargetPx(160, 9.0) == 57,
          QStringLiteral("9 mm is 34 px at 96 dpi, 57 px at 160 dpi and 90 px at 254 dpi"),
          QStringLiteral("%1 %2 %3").arg(smallui::touchTargetPx(96)).arg(smallui::touchTargetPx(254)).arg(smallui::touchTargetPx(160)));
    const QSize own = smallui::physicalSize(QGuiApplication::primaryScreen());
    check(own.width() > 0 && own.height() > 0, QStringLiteral("the screen's physical size is known"));
}

// ------------------------------------------------------------------ the window

struct Before {
    QList<int> split, side;
    QList<int> paneSizes;
    QMap<QString, bool> bars;
    QMap<QString, QRect> geometry;
    bool menuBar = false, binder = false, inspector = false, binderChecked = false, inspectorChecked = false;
    int tab = 0;
    bool upright = false;
    QRect binderRect, editorRect;
    QByteArray state;
};

static Before capture(MainWindow &w)
{
    Before b;
    b.split = w.paneSplitter()->sizes();
    b.side = w.sideSplitter()->sizes();
    b.paneSizes = w.paneSizes();
    b.bars = barVisibility(w);
    b.geometry = barGeometry(w);
    b.menuBar = w.menuBar()->isVisible();
    BinderPanel *binder = binderOf(w);
    Inspector *inspector = inspectorOf(w);
    b.binder = binder->isVisible();
    b.inspector = inspector->isVisible();
    b.binderChecked = w.findChild<QAction *>(QStringLiteral("showBinder"))->isChecked();
    b.inspectorChecked = w.findChild<QAction *>(QStringLiteral("showInspector"))->isChecked();
    b.tab = inspector->currentTab();
    b.upright = w.isUpright();
    b.binderRect = QRect(binder->mapTo(&w, QPoint(0, 0)), binder->size());
    b.state = w.saveState();
    return b;
}

static void sameAs(MainWindow &w, const Before &b, const QString &what)
{
    const Before n = capture(w);
    check(n.split == b.split && n.side == b.side, QStringLiteral("%1: the panes have the sizes they had").arg(what),
          QStringLiteral("main %1 -> %2, side %3 -> %4")
              .arg(QStringLiteral("%1").arg(b.split.size()), QStringLiteral("%1").arg(n.split.size()))
              .arg(b.side.size())
              .arg(n.side.size()));
    QString sizes;
    for (int v : b.split)
        sizes += QString::number(v) + QLatin1Char(' ');
    sizes += QStringLiteral("-> ");
    for (int v : n.split)
        sizes += QString::number(v) + QLatin1Char(' ');
    check(n.split == b.split, QStringLiteral("%1: the main splitter's sizes are the same").arg(what), sizes);
    check(n.paneSizes == b.paneSizes, QStringLiteral("%1: the stored pane sizes are the same").arg(what));
    check(n.bars == b.bars, QStringLiteral("%1: the same toolbars are shown").arg(what));
    check(n.geometry == b.geometry, QStringLiteral("%1: the toolbars stand where they stood").arg(what));
    check(n.menuBar == b.menuBar, QStringLiteral("%1: the menu bar is back").arg(what));
    check(n.binder == b.binder && n.inspector == b.inspector && n.binderChecked == b.binderChecked && n.inspectorChecked == b.inspectorChecked,
          QStringLiteral("%1: the binder and the inspector are shown or hidden as they were").arg(what));
    check(n.tab == b.tab, QStringLiteral("%1: the inspector shows the tab it showed").arg(what));
    check(n.binderRect == b.binderRect, QStringLiteral("%1: the binder stands where it stood").arg(what));
    check(n.state == b.state, QStringLiteral("%1: the window's saved state is the same").arg(what));
}

static void defaults(MainWindow &w)
{
    std::printf("== off at first, the menu item\n");
    check(!w.smallInterface(), QStringLiteral("the program starts in the full interface"));
    QAction *item = viewAction(w, QStringLiteral("Small Interface"));
    check(item && item->isCheckable() && !item->isChecked() && item->isEnabled(), QStringLiteral("View > Small Interface is a check item, off and enabled"));
    check(w.findChild<QAction *>(QStringLiteral("smallChoiceFull")) == nullptr && w.findChild<QAction *>(QStringLiteral("smallChoiceAuto")) == nullptr,
          QStringLiteral("Linux and Windows show only the check item (the choices Full, Small, Automatic are Android's)"));
    check(w.smallBar() && !w.smallBar()->isVisible(), QStringLiteral("the round buttons' bar is not shown in the full interface"));
    for (const QString &n : kNames)
        check(button(w, n) != nullptr, QStringLiteral("the button \"%1\" exists").arg(n));
}

static void cycle(MainWindow &w, bool upright, const QString &what, bool hideInspector)
{
    std::printf("== the mode on and off: %s\n", qPrintable(what));
    w.resize(upright ? QSize(1080, 1920) : QSize(1280, 800));
    settle(500);
    check(w.isUpright() == upright, QStringLiteral("(the window is %1)").arg(upright ? QStringLiteral("upright") : QStringLiteral("wide")));
    QAction *inspectorSwitch = w.findChild<QAction *>(QStringLiteral("showInspector"));
    if (hideInspector)
        inspectorSwitch->setChecked(false);
    settle(300);
    const Before b = capture(w);
    check(b.menuBar && b.bars.value(QStringLiteral("mainToolbar")), QStringLiteral("(the full interface shows the menu bar and the main toolbar)"));
    QAction *item = viewAction(w, QStringLiteral("Small Interface"));
    item->trigger();
    settle(500);
    check(w.smallInterface() && item->isChecked(), QStringLiteral("the check item switches the mode on"));
    bool anyBar = false;
    const QMap<QString, bool> shownBars = barVisibility(w);
    for (auto it = shownBars.cbegin(); it != shownBars.cend(); ++it)
        anyBar = anyBar || it.value();
    check(!anyBar, QStringLiteral("no toolbar is shown"));
    check(!w.menuBar()->isVisible(), QStringLiteral("the menu bar is hidden (the menu button has its menus)"));
    BinderPanel *binder = binderOf(w);
    Inspector *inspector = inspectorOf(w);
    check(!binder->isVisible() && !inspector->isVisible(), QStringLiteral("the binder and the inspector are not beside the text"));
    check(w.smallBar()->isVisible(), QStringLiteral("the round buttons are shown"));
    check(w.toolBarArea(w.smallBar()) == (upright ? Qt::BottomToolBarArea : Qt::RightToolBarArea),
          upright ? QStringLiteral("upright: the buttons are in a row at the bottom") : QStringLiteral("wide: the buttons are in a column at the right edge"));
    QWidget *editorArea = w.findChild<QWidget *>(QStringLiteral("editorArea"));
    const int shortWidth = upright ? w.width() : w.width() - w.smallBar()->width();
    check(editorArea->isVisible() && editorArea->width() >= shortWidth - 8,
          QStringLiteral("the text has the whole width of the window beside the buttons"),
          QStringLiteral("editor %1, window %2, bar %3").arg(editorArea->width()).arg(w.width()).arg(w.smallBar()->width()));
    check(!w.findChild<QAction *>(QStringLiteral("showBinder"))->isEnabled() && !w.findChild<QAction *>(QStringLiteral("showInspector"))->isEnabled(),
          QStringLiteral("Show Binder and Show Inspector are off while the mode runs (the panels have their buttons)"));
    shot(&w, upright ? QStringLiteral("portrait-small-%1").arg(hideInspector ? 2 : 1) : QStringLiteral("landscape-small-%1").arg(hideInspector ? 2 : 1));
    item->trigger();
    settle(600);
    check(!w.smallInterface() && !item->isChecked(), QStringLiteral("the check item switches the mode off"));
    check(!w.smallBar()->isVisible(), QStringLiteral("the buttons are gone"));
    check((upright ? w.sideSplitter()->indexOf(binder) >= 0 : w.paneSplitter()->indexOf(binder) >= 0)
              && (upright ? w.sideSplitter()->indexOf(inspector) >= 0 : w.paneSplitter()->indexOf(inspector) >= 0),
          QStringLiteral("the binder and the inspector are back in the splitters"));
    sameAs(w, b, what);
    if (hideInspector)
        inspectorSwitch->setChecked(true);
    settle(300);
}

static void buttons(MainWindow &w, Project *p, bool upright)
{
    const QString mode = upright ? QStringLiteral("portrait") : QStringLiteral("landscape");
    std::printf("== the round buttons, %s\n", qPrintable(mode));
    w.resize(upright ? QSize(1080, 1920) : QSize(1280, 800));
    settle(500);
    w.setSmallInterface(true);
    settle(600);
    BinderPanel *binder = binderOf(w);
    Inspector *inspector = inspectorOf(w);
    QScreen *sc = w.screen() ? w.screen() : QGuiApplication::primaryScreen();
    const int target = smallui::touchTargetPx(sc->physicalDotsPerInch() / qMax(1.0, sc->devicePixelRatio()));
    QStringList small;
    for (const QString &n : kNames)
        if (auto *b = button(w, n); !b || b->width() < target || b->height() < target || b->width() != b->height())
            small << n;
    check(small.isEmpty(), QStringLiteral("%1: every button is round-sized and at least 9 mm (%2 px) wide and high").arg(mode).arg(target),
          small.join(QLatin1Char(' ')) + QStringLiteral(" (round button %1)").arg(w.smallTargetPx()));
    // the order and the row or column
    bool aligned = true;
    for (int i = 1; i < kNames.size(); ++i) {
        const QPoint a = button(w, kNames[i - 1])->mapTo(&w, QPoint(0, 0)), c = button(w, kNames[i])->mapTo(&w, QPoint(0, 0));
        aligned = aligned && (upright ? (a.y() == c.y() && a.x() < c.x()) : (a.x() == c.x() && a.y() < c.y()));
    }
    check(aligned, upright ? QStringLiteral("portrait: the buttons are in one row, left to right")
                           : QStringLiteral("landscape: the buttons are in one column, top to bottom"));
    bool inside = true;
    for (const QString &n : kNames) {
        QWidget *b = button(w, n);
        inside = inside && b->isVisible() && w.rect().contains(QRect(b->mapTo(&w, QPoint(0, 0)), b->size()));
    }
    check(inside, QStringLiteral("%1: all the buttons are on the screen").arg(mode));
    bool focusable = true;
    for (const QString &n : kNames)
        focusable = focusable && button(w, n)->focusPolicy() != Qt::NoFocus;
    check(focusable, QStringLiteral("every button can take the keyboard (the focus line follows it)"));

    // the buttons' look follows the theme: palette colors in the style sheet
    check(button(w, QStringLiteral("bold"))->styleSheet().contains(QStringLiteral("palette(highlight)")) && button(w, QStringLiteral("bold"))->styleSheet().contains(QStringLiteral("border-radius")),
          QStringLiteral("the buttons are round and use the theme's colors"));

    // new document (next to a document, so that the text shows)
    BinderItem *doc = firstDocument(p->draft());
    if (doc)
        binder->select({doc}, true);
    settle(300);
    const int before = countItems(p->draft());
    button(w, QStringLiteral("new"))->click();
    settle(300);
    check(countItems(p->draft()) == before + 1, QStringLiteral("%1: the new-document button makes a document").arg(mode),
          QStringLiteral("%1 -> %2").arg(before).arg(countItems(p->draft())));

    // bold and italic act on the selected text
    TextEditor *ed = visibleEditor(w);
    {
        QString what;
        for (TextEditor *e : w.findChildren<TextEditor *>())
            what += QStringLiteral("%1 vis %2 ro %3 parent %4; ").arg(e->objectName()).arg(e->isVisible()).arg(e->isReadOnly()).arg(e->parentWidget() ? e->parentWidget()->metaObject()->className() : "");
        check(ed != nullptr, QStringLiteral("(an editor is shown)"), what);
    }
    if (ed) {
        ed->setFocus();
        ed->setPlainText(QStringLiteral("Hello world"));
        ed->selectAll();
        settle(150);
        button(w, QStringLiteral("bold"))->click();
        settle(150);
        check(ed->textCursor().charFormat().fontWeight() >= QFont::DemiBold, QStringLiteral("%1: the bold button makes the selected text bold").arg(mode));
        button(w, QStringLiteral("italic"))->click();
        settle(150);
        check(ed->textCursor().charFormat().fontItalic(), QStringLiteral("%1: the italic button makes the selected text italic").arg(mode));
        check(button(w, QStringLiteral("bold"))->isChecked() && button(w, QStringLiteral("italic"))->isChecked(),
              QStringLiteral("the two buttons show that the selected text is bold and italic"));
        button(w, QStringLiteral("bold"))->click();
        button(w, QStringLiteral("italic"))->click();
        settle(150);
    }
    // the style choice is the Format menu's style list
    QToolButton *style = button(w, QStringLiteral("style"));
    check(style->menu() && style->menu()->objectName() == QLatin1String("styleMenu") && style->popupMode() == QToolButton::InstantPopup,
          QStringLiteral("the style button opens the style choice"));
    if (style->menu()) {
        QMetaObject::invokeMethod(style->menu(), "aboutToShow", Qt::DirectConnection);
        int entries = 0;
        for (QAction *ac : style->menu()->actions())
            if (!ac->isSeparator() && ac->isEnabled())
                ++entries;
        check(entries >= 2, QStringLiteral("the style choice lists the project's styles"), QString::number(entries));
    }
    // labels and icons
    QToolButton *labels = button(w, QStringLiteral("labels"));
    int subs = 0;
    for (QAction *ac : labels->menu()->actions())
        if (ac->menu())
            ++subs;
    check(subs == 2, QStringLiteral("the labels button offers the labels and the icons"), QString::number(subs));
    // the menu button: every menu of the menu bar and the way back
    QToolButton *menu = button(w, QStringLiteral("menu"));
    QMetaObject::invokeMethod(menu->menu(), "aboutToShow", Qt::DirectConnection);
    int menus = 0, tops = 0;
    for (QAction *ac : menu->menu()->actions())
        if (ac->menu())
            ++menus;
    for (QAction *top : w.menuBar()->actions())
        if (top->menu())
            ++tops;
    check(menus == tops && tops >= 8, QStringLiteral("the menu button has all %1 menus of the menu bar").arg(tops), QString::number(menus));
    check(menu->menu()->actions().last()->text().remove(QLatin1Char('&')) == QLatin1String("Switch to the Full Interface"),
          QStringLiteral("the menu button ends with the way back to the full interface"));

    // the binder panel
    smallui::SlidePanel *bp = w.smallPanel(false), *np = w.smallPanel(true);
    QToolButton *bb = button(w, QStringLiteral("binder")), *nb = button(w, QStringLiteral("notes"));
    check(!bp->isOpen() && !np->isOpen(), QStringLiteral("(both panels are closed)"));
    bb->click();
    settle(500);
    const QRect area = w.centralWidget()->geometry();
    check(bp->isOpen() && bp->isVisible() && binder->isVisible() && bb->isChecked(), QStringLiteral("%1: the binder button opens the binder panel").arg(mode));
    check(binder->parentWidget() && bp->isAncestorOf(binder), QStringLiteral("the panel holds the program's own binder"),
          QStringLiteral("parent %1 %2, grandparent %3 %4, panel's content area %5")
              .arg(binder->parentWidget()->metaObject()->className(), binder->parentWidget()->objectName(),
                   binder->parentWidget()->parentWidget() ? binder->parentWidget()->parentWidget()->metaObject()->className() : "-",
                   binder->parentWidget()->parentWidget() ? binder->parentWidget()->parentWidget()->objectName() : QString())
              .arg(quintptr(bp->contentArea())));
    if (upright)
        check(bp->geometry() == area && bp->edge() == smallui::SlidePanel::Edge::Sheet, QStringLiteral("portrait: the binder is a sheet over the whole text area"),
              QStringLiteral("panel %1x%2 area %3x%4").arg(bp->width()).arg(bp->height()).arg(area.width()).arg(area.height()));
    else
        check(bp->geometry().left() == area.left() && bp->width() < area.width() && bp->width() >= area.width() * 4 / 10 && bp->height() == area.height(),
              QStringLiteral("landscape: the binder slides in from the left and leaves room for the text"),
              QStringLiteral("panel %1x%2 area %3x%4").arg(bp->width()).arg(bp->height()).arg(area.width()).arg(area.height()));
    shot(&w, QStringLiteral("%1-binder-panel").arg(mode));
    nb->click();
    settle(500);
    check(np->isOpen() && !bp->isOpen() && !bb->isChecked() && nb->isChecked(), QStringLiteral("the notes button opens the notes and closes the binder panel"));
    check(inspector->isVisible() && np->isAncestorOf(inspector) && inspector->currentTab() == Inspector::Notes, QStringLiteral("the notes panel shows the notes tab of the inspector"),
          QString::number(inspector->currentTab()));
    {
        FocusLine *fl = w.findChild<FocusLine *>();
        check(fl && !fl->lineRect().isEmpty() && np->geometry().adjusted(-2, -2, 2, 2).contains(fl->lineRect()),
              QStringLiteral("%1: the focus line is around the part with the keyboard, inside the panel after it slid in").arg(mode),
              fl ? QStringLiteral("line %1,%2 %3x%4 part %5").arg(fl->lineRect().x()).arg(fl->lineRect().y()).arg(fl->lineRect().width()).arg(fl->lineRect().height()).arg(fl->part())
                 : QString());
    }
    if (!upright)
        check(np->geometry().right() == area.right() && np->width() < area.width(), QStringLiteral("landscape: the notes slide in from the right"));
    shot(&w, QStringLiteral("%1-notes-panel").arg(mode));
    nb->click();
    settle(300);
    check(!np->isOpen() && !np->isVisible() && !nb->isChecked(), QStringLiteral("the notes button closes the panel again"));
    bb->click();
    settle(300);
    QTest::mouseClick(bp->closeButton(), Qt::LeftButton);
    settle(300);
    check(!bp->isOpen() && !bb->isChecked(), QStringLiteral("a panel's Close button closes it and the button follows"));
    bb->click();
    settle(300);
    QTest::keyClick(bp, Qt::Key_Escape);
    settle(300);
    check(!bp->isOpen() && !bb->isChecked(), QStringLiteral("Esc closes a panel (keyboard use)"));
    // a command that shows the binder by itself opens its panel
    if (QAction *search = w.findChild<QAction *>(QStringLiteral("projectSearch"))) {
        search->trigger();
        settle(500);
        check(bp->isOpen() && binder->isVisible(), QStringLiteral("%1: Project Search opens the binder panel").arg(mode));
        QTest::keyClick(bp, Qt::Key_Escape);
        settle(300);
        if (binder->isSearching())
            search->trigger(); // (the search field is closed again)
        settle(300);
        if (bp->isOpen())
            bp->closePanel(false);
    }
    // the way back
    w.findChild<QAction *>(QStringLiteral("smallToFull"))->trigger();
    settle(500);
    check(!w.smallInterface() && w.menuBar()->isVisible(), QStringLiteral("%1: \"Switch to the Full Interface\" in the menu button leads back").arg(mode));
}

static void record(MainWindow &w, bool installed)
{
    std::printf("== record only (%s)\n", installed ? "the speech add-on is loaded" : "no add-on");
    w.setSmallInterface(true);
    settle(300);
    QToolButton *rec = button(w, QStringLiteral("record"));
    if (!installed) {
        check(rec && !rec->isEnabled(), QStringLiteral("without the speech add-on the record button is off"));
        check(rec->toolTip().contains(QStringLiteral("not installed")) && rec->toolTip().contains(QStringLiteral("Speech Transcription")),
              QStringLiteral("and says why"), rec->toolTip());
    } else {
        check(rec && rec->isEnabled(), QStringLiteral("with the speech add-on the record button is on"));
        check(rec->toolTip().contains(QStringLiteral("Record only")), QStringLiteral("and says what it does"), rec->toolTip());
        bool has = false;
        for (const tools::Command &c : toolhost::host().allCommands())
            has = has || c.key == QLatin1String("speech-transcription.record-only");
        check(has, QStringLiteral("the add-on has the command that records only"));
    }
    w.setSmallInterface(false);
    settle(300);
}

static void profilesKeep(MainWindow &w)
{
    std::printf("== a profile keeps the mode\n");
    QString err;
    check(profiles::createFromCurrent(QStringLiteral("Roomy"), &err), QStringLiteral("(a profile is made from the full interface)"), err);
    w.setSmallInterface(true);
    settle(400);
    check(QSettings().value(QLatin1String(smallui::kKeyOn)).toBool(), QStringLiteral("the mode is a program setting that a profile takes"));
    check(profiles::createFromCurrent(QStringLiteral("Compact"), &err), QStringLiteral("(a profile is made from the small interface)"), err);
    profiles::Content c;
    check(profiles::read(QStringLiteral("Compact"), &c) && c.values.value(QLatin1String(smallui::kKeyOn)).toBool(),
          QStringLiteral("the profile file holds the mode"));
    check(profiles::read(QStringLiteral("Default"), &c) && !c.values.value(QLatin1String(smallui::kKeyOn)).toBool(),
          QStringLiteral("a profile made before holds the full interface"));
    check(w.switchProfile(QStringLiteral("Default")), QStringLiteral("(switch to the full profile)"));
    settle(600);
    check(!w.smallInterface() && !w.smallBar()->isVisible() && w.menuBar()->isVisible() && barVisibility(w).value(QStringLiteral("mainToolbar")),
          QStringLiteral("a profile of the full interface brings it back: bars, menu bar"));
    check(w.switchProfile(QStringLiteral("Compact")), QStringLiteral("(switch to the small profile)"));
    settle(600);
    check(w.smallInterface() && w.smallBar()->isVisible() && !w.menuBar()->isVisible() && !barVisibility(w).value(QStringLiteral("mainToolbar")),
          QStringLiteral("a profile of the small interface brings it"));
    check(viewAction(w, QStringLiteral("Small Interface")) && viewAction(w, QStringLiteral("Small Interface"))->isChecked(), QStringLiteral("and the menu item shows it"));
    check(w.switchProfile(QStringLiteral("Default")), QStringLiteral("(back to the full profile)"));
    settle(600);
    check(!w.smallInterface() && w.paneSplitter()->indexOf(binderOf(w)) >= 0, QStringLiteral("leaving by a profile puts the binder back in its place"));
}

static void android(MainWindow &w)
{
    std::printf("== the Android rule in the window\n");
    using smallui::Choice;
    w.setSmallChoice(Choice::Automatic);
    w.applySmallRule(QSize(1280, 800));
    settle(300);
    check(w.smallInterface(), QStringLiteral("Automatic on a 1280 x 800 screen: small"));
    w.applySmallRule(QSize(1920, 1080));
    settle(300);
    check(!w.smallInterface(), QStringLiteral("Automatic on a 1920 x 1080 screen: full"));
    w.setSmallChoice(Choice::Small);
    w.applySmallRule(QSize(3840, 2160));
    settle(300);
    check(w.smallInterface(), QStringLiteral("the choice Small on a large screen: small"));
    w.setSmallChoice(Choice::Full);
    w.applySmallRule(QSize(1280, 800));
    settle(300);
    check(!w.smallInterface() && w.smallChoice() == Choice::Full, QStringLiteral("the choice Full on a small screen: full"));
    w.setSmallChoice(Choice::Automatic);
    w.setSmallInterface(false);
    settle(300);
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
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990lTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990lTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    outDir = argc > 1 && QString::fromLocal8Bit(argv[1]) != QLatin1String("--addon") ? QString::fromLocal8Bit(argv[1])
                                                                                      : QDir::currentPath() + QLatin1String("/shots");
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

    const bool addonRun = argc > 1 && QString::fromLocal8Bit(argv[1]) == QLatin1String("--addon");
    if (addonRun) {
        outDir = argc > 2 ? QString::fromLocal8Bit(argv[2]) : outDir;
        qputenv("QB_PLUGIN_PATH", (QStringLiteral(QB_TEST_BUILD_DIR) + QStringLiteral("/tool-plugins")).toUtf8());
    } else {
        rule();
    }

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
    w.setWindowState(Qt::WindowNoState);
    w.move(0, 0);
    w.resize(1280, 800);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(500);

    if (addonRun) {
        record(w, true);
        delete wp;
        std::printf("\n%d failure(s)\n", failures);
        return failures;
    }
    defaults(w);
    shot(&w, QStringLiteral("landscape-full"));
    cycle(w, false, QStringLiteral("wide window"), false);
    cycle(w, false, QStringLiteral("wide window, inspector hidden"), true);
    cycle(w, true, QStringLiteral("upright window"), false);
    cycle(w, true, QStringLiteral("upright window, inspector hidden"), true);
    buttons(w, p, false);
    buttons(w, p, true);
    w.resize(1280, 800);
    settle(500);
    record(w, false);
    profilesKeep(w);
    android(w);

    delete wp;
    // the speech add-on: the tools are loaded once per program, so a second run of this program has it (QB_PLUGIN_PATH)
    {
        QProcess child;
        child.setProcessChannelMode(QProcess::MergedChannels);
        child.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--addon"), outDir});
        const bool done = child.waitForFinished(180000);
        const QString text = QString::fromLocal8Bit(child.readAll());
        std::printf("%s", qPrintable(text));
        check(done && child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0 && text.contains(QStringLiteral("0 failure(s)")),
              QStringLiteral("(the run with the speech add-on ends with 0 failures)"));
    }
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
