// Scripted run for version 0.94.0 (shared code, fourth part of the run's to-do list):
//  - item 65: composition mode's bar is readable in every built-in theme (text at least 4.5:1, WCAG 2.2);
//  - item 81: the corkboard's card text and the ruler's numbers are at least 4.5:1 in every built-in theme;
//  - item 80: ONE theme button with a menu (Light, Dark, My Look 1, My Look 2, all themes) on the Themes toolbar, which
//    is off at first, also after an update;
//  - item 66: the manual's window has a contents list and a search; F1 opens the section of the view or dialog; the
//    developer matter is in the developer guide;
//  - item 82: the first compile with a title page asks for the author's name; without it the "by" line is left out;
//  - item 90: the add-on install window names what the add-on does first, the technical lines are under "Details"
//    (collapsed); the full AppImage's window names each add-on by its manifest's name;
//  - item 83: the download page lists the add-ons of each system and says that the desktop may ask to execute the file;
//    the release workflow writes full-addons-size;
//  - item 61: the Linux system test's harness (checked as text; it runs only when the writer asks);
//  - the rename "Quick-Add Keys in the Binder", the German texts.
//
//   xvfb-run -a ./qb_v0940d_tests /tmp/shots
#include "AppInfo.h"
#include "compile/CompileFormat.h"
#include "compile/Compiler.h"
#include "core/Addons.h"
#include "core/Paths.h"
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "ui/AddonDialogs.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/CompileDialog.h"
#include "ui/Corkboard.h"
#include "ui/EditorPane.h"
#include "ui/FullInstallWindow.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/ManualWindow.h"
#include "ui/ProjectLook.h"
#include "ui/TextEditor.h"
#include "ui/Ruler.h"
#include "ui/ThemeDialogs.h"
#include "ui/ThemePicker.h"
#include "ui/ToolDialogs.h"
#include "ui/Theme.h"
#include "ui/Windows.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QToolBar>
#include <QToolButton>
#include <QTranslator>
#include <QTreeWidget>

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

static QString readText(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

static const QString kRoot = QStringLiteral(QB_TEST_SOURCE_DIR);

static QString python()
{
    return QStringLiteral("python3");
}

static QString plain(QString t)
{
    t.remove(QLatin1Char('&'));
    return t.section(QLatin1Char('\t'), 0, 0).simplified();
}

static QList<theme::Entry> builtInThemes()
{
    QList<theme::Entry> out;
    for (const theme::Entry &e : theme::available())
        if (e.kind == theme::Kind::BuiltIn)
            out << e;
    return out;
}

// the contrast of the darkest or lightest pixel of a picture against its most common color (what a text on a flat
// background looks like); 1 when the picture has one color only
static double pixelContrast(const QImage &img)
{
    QHash<QRgb, int> count;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            count[img.pixel(x, y) | 0xff000000u]++;
    QRgb bg = 0;
    int best = -1;
    for (auto it = count.constBegin(); it != count.constEnd(); ++it)
        if (it.value() > best) {
            best = it.value();
            bg = it.key();
        }
    double worst = 1;
    for (auto it = count.constBegin(); it != count.constEnd(); ++it)
        worst = qMax(worst, theme::contrastRatio(QColor::fromRgb(it.key()), QColor::fromRgb(bg)));
    return worst;
}

// ---------------------------------------------------------------- items 65 and 81
static void contrasts(MainWindow &w, Project *p, BinderItem *doc)
{
    std::printf("== composition bar, card text and ruler numbers: at least 4.5:1 in every built-in theme (items 65, 81)\n");
    const QList<theme::Entry> themes = builtInThemes();
    check(themes.size() >= 8, QStringLiteral("the built-in themes are listed"), QString::number(themes.size()));
    auto *binder = w.findChild<BinderPanel *>();
    binder->select({doc}, true);
    settle(200);
    QStringList barBad, cardBad, rulerBad;
    double lightest = 99;
    for (const theme::Entry &e : themes) {
        themeui::choose(&w, e.id, false);
        settle(150);
        // composition mode, opened in this theme
        w.findChild<QAction *>(QStringLiteral("composition"))->trigger();
        settle(400);
        CompositionWindow *comp = nullptr;
        for (QWidget *t : QApplication::topLevelWidgets())
            if (auto *cw = qobject_cast<CompositionWindow *>(t))
                if (cw->isVisible())
                    comp = cw;
        if (!comp) {
            barBad << e.id + QStringLiteral(": no composition window");
            continue;
        }
        int checked = 0;
        for (QToolButton *b : comp->findChildren<QToolButton *>()) {
            if (b->text().isEmpty())
                continue;
            ++checked;
            const double r = theme::contrastRatio(b->palette().color(QPalette::ButtonText), b->palette().color(QPalette::Button));
            lightest = qMin(lightest, r);
            if (r < 4.5)
                barBad << QStringLiteral("%1: button %2 %3").arg(e.id, b->text()).arg(r, 0, 'f', 1);
        }
        for (QComboBox *c : comp->findChildren<QComboBox *>()) {
            ++checked;
            const double r = theme::contrastRatio(c->palette().color(QPalette::ButtonText), c->palette().color(QPalette::Button));
            const double r2 = theme::contrastRatio(c->palette().color(QPalette::Text), c->palette().color(QPalette::Base));
            lightest = qMin(lightest, qMin(r, r2));
            if (r < 4.5 || r2 < 4.5)
                barBad << QStringLiteral("%1: zoom box %2 %3").arg(e.id).arg(r, 0, 'f', 1).arg(r2, 0, 'f', 1);
        }
        for (QLabel *l : comp->findChildren<QLabel *>()) {
            if (l->text().isEmpty() || !l->parentWidget() || !l->parentWidget()->autoFillBackground() || l->parentWidget() == comp)
                continue;
            const double r = theme::contrastRatio(l->palette().color(QPalette::WindowText), l->palette().color(QPalette::Window));
            ++checked;
            lightest = qMin(lightest, r);
            if (r < 4.5)
                barBad << QStringLiteral("%1: label %2 %3").arg(e.id, l->text()).arg(r, 0, 'f', 1);
        }
        if (checked < 4)
            barBad << e.id + QStringLiteral(": only %1 widgets in the bar").arg(checked);
        if (e.id == QLatin1String("light")) {
            // the picture: the bar shown, and its buttons measured by their pixels
            for (QWidget *c : comp->findChildren<QWidget *>())
                if (c->layout() && c->autoFillBackground() && c->findChild<QToolButton *>()) {
                    c->show();
                    c->raise();
                    settle(200);
                    c->resize(comp->width(), qMax(c->sizeHint().height(), 40));
                    shot(c, QStringLiteral("composition-bar-light"));
                    for (QToolButton *b : c->findChildren<QToolButton *>())
                        if (!b->text().isEmpty()) {
                            const double r = pixelContrast(b->grab().toImage());
                            if (r < 4.5)
                                barBad << QStringLiteral("light: the picture of button %1 has %2:1").arg(b->text()).arg(r, 0, 'f', 1);
                        }
                    break;
                }
        }
        comp->close();
        settle(300);
        // the card text and the ruler numbers
        const QColor paper = theme::color(theme::Role::CardPaper), text = theme::color(theme::Role::CardText);
        for (const bool fromDoc : {false, true}) {
            const double r = theme::contrastRatio(CardItem::bodyTextColor(paper, text, fromDoc), paper);
            if (r < 4.5)
                cardBad << QStringLiteral("%1 %2 %3").arg(e.id, fromDoc ? QStringLiteral("(text of the document)") : QStringLiteral("(synopsis)")).arg(r, 0, 'f', 2);
        }
        if (auto *ruler = w.findChild<Ruler *>()) {
            const double r = theme::contrastRatio(ruler->numberColor(), ruler->palette().color(QPalette::Window));
            if (r < 4.5)
                rulerBad << QStringLiteral("%1 %2").arg(e.id).arg(r, 0, 'f', 2);
        } else {
            rulerBad << QStringLiteral("no ruler");
        }
    }
    check(barBad.isEmpty(), QStringLiteral("the composition bar's buttons, zoom box and texts: at least 4.5:1 in every built-in theme (%1 themes)").arg(themes.size()),
          barBad.join(QStringLiteral("; ")));
    check(cardBad.isEmpty(), QStringLiteral("the corkboard's card text (synopsis and the text taken from the document): at least 4.5:1 on the card in every theme"),
          cardBad.join(QStringLiteral("; ")));
    check(rulerBad.isEmpty(), QStringLiteral("the ruler's numbers: at least 4.5:1 in every theme"), rulerBad.join(QStringLiteral("; ")));
    // the numbers of the report: the card of the default theme was 3.1:1 and the ruler 2.5:1
    themeui::choose(&w, QStringLiteral("system"), false);
    settle(150);
    check(theme::contrastRatio(QColor(144, 144, 141), QColor(253, 252, 247)) < 4.0 && theme::contrastRatio(QColor(107, 107, 107), QColor(253, 252, 247)) >= 4.5,
          QStringLiteral("(the old card text was below 4.5:1, a gray of #6B6B6B is above)"));
    check(theme::contrastRatio(theme::withContrast(QColor(144, 144, 141), QColor(253, 252, 247)), QColor(253, 252, 247)) >= 4.5
              && theme::withContrast(QColor(10, 10, 10), QColor(250, 250, 250)) == QColor(10, 10, 10),
          QStringLiteral("withContrast darkens a gray until it reaches 4.5:1 and leaves a color that has it alone"));
    // a card on the corkboard, a picture of it
    BinderItem *card = doc;
    card->synopsis = QStringLiteral("The lamp is lit and the town is asleep.");
    CardItem item(p, card, QSizeF(200, 140));
    QImage img(600, 420, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter pp(&img);
    pp.scale(3, 3);
    item.paint(&pp, nullptr, nullptr);
    pp.end();
    img.save(outDir + QStringLiteral("/card.png"));
    check(!img.isNull(), QStringLiteral("(a card was drawn: card.png)"));
    card->synopsis.clear();
}

// ---------------------------------------------------------------- item 80
static void themeButton(MainWindow &w)
{
    std::printf("== one theme button with a menu (item 80)\n");
    // off at first, also after an update
    {
        QSettings s;
        s.beginGroup(QStringLiteral("toolbars"));
        s.setValue(QStringLiteral("split0910"), true);
        s.setValue(QStringLiteral("hidden"), QStringList());
        s.remove(QStringLiteral("themesOff0940"));
        s.endGroup();
        s.sync();
        AppSettings::instance().toolbarHidden.clear();
        AppSettings::instance().load();
        check(AppSettings::instance().toolbarHidden.contains(QStringLiteral("themes")),
              QStringLiteral("a program that showed the Themes toolbar: it is off after the update (once)"));
        AppSettings::instance().toolbarHidden.removeAll(QStringLiteral("themes"));
        AppSettings::instance().save();
        AppSettings::instance().load();
        check(!AppSettings::instance().toolbarHidden.contains(QStringLiteral("themes")),
              QStringLiteral("... and when the writer switches it on again, it stays on"));
        AppSettings::instance().toolbarHidden << QStringLiteral("themes");
        AppSettings::instance().save();
    }
    auto *bar = w.findChild<QToolBar *>(QStringLiteral("themesBar"));
    auto *picker = w.findChild<ThemePicker *>(QStringLiteral("themePicker"));
    check(bar && picker && bar->isAncestorOf(picker), QStringLiteral("the theme control is on the Themes toolbar"));
    if (!bar || !picker)
        return;
    check(bar->isHidden(), QStringLiteral("the Themes toolbar is off at first"));
    w.findChild<QAction *>(QStringLiteral("showBarThemes"))->setChecked(true);
    settle(300);
    check(!bar->isHidden() && picker->isVisible(), QStringLiteral("View > Toolbars > Themes shows it"));
    auto *button = picker->findChild<QToolButton *>(QStringLiteral("themeButton"));
    check(button && picker->findChildren<QToolButton *>().size() == 1 && !picker->findChild<QComboBox *>(),
          QStringLiteral("one button, no list and no row of four buttons"));
    check(picker->sizeHint().width() <= 140, QStringLiteral("it takes a small part of the toolbar (not 333 px)"), QString::number(picker->sizeHint().width()));
    if (!button || !button->menu())
        return;
    QMenu *menu = button->menu();
    emit menu->aboutToShow();
    QStringList names;
    for (QAction *a : menu->actions())
        if (!a->isSeparator() && a->objectName().startsWith(QStringLiteral("themeUser")) | a->objectName().startsWith(QStringLiteral("themeBright")) | a->objectName().startsWith(QStringLiteral("themeDark")))
            names << plain(a->text());
    check(names.size() == 4 && names[0] == QLatin1String("Light") && names[1] == QLatin1String("Dark") && names[2].startsWith(QLatin1String("My Look 1"))
              && names[3].startsWith(QLatin1String("My Look 2")),
          QStringLiteral("the menu starts with Light, Dark, My Look 1 and My Look 2"), names.join(QStringLiteral(" | ")));
    check(names.size() == 4 && names[2].contains(QLatin1String("empty")), QStringLiteral("an empty My Look says so in the menu"), names.join(QStringLiteral(" | ")));
    QStringList tips;
    for (QAction *a : menu->actions())
        tips << a->toolTip() + a->statusTip();
    check(tips.join(QLatin1Char(' ')).contains(QLatin1String("saves the look you see now")), QStringLiteral("the empty entry explains what it does (tool tip and status tip)"));
    int ids = 0;
    for (const theme::Entry &e : theme::available())
        ids += picker->shownIds().contains(e.id) ? 1 : 0;
    check(ids == theme::available().size(), QStringLiteral("below them every theme is listed"), QString::number(ids));
    for (QAction *a : menu->actions())
        for (const QString &bad : {QStringLiteral("U1"), QStringLiteral("U2"), QStringLiteral("Bright")})
            if (a->text().contains(bad))
                check(false, QStringLiteral("no entry says \"%1\"").arg(bad), a->text());
    const QString all = [&] {
        QStringList t;
        for (QAction *a : menu->actions())
            t << plain(a->text());
        return t.join(QStringLiteral("\n"));
    }();
    check(all.contains(QStringLiteral("Save the Current Look as My Look 1")) && all.contains(QStringLiteral("Save the Current Look as My Look 2")),
          QStringLiteral("... and at the end the two entries that save the look you see now"));
    // Light switches to Light, not to the theme that follows the desktop
    themeui::choose(&w, QStringLiteral("system"), false);
    settle(200);
    emit menu->aboutToShow();
    QAction *light = menu->findChild<QAction *>(QStringLiteral("themeBright"));
    QAction *dark = menu->findChild<QAction *>(QStringLiteral("themeDark"));
    check(light && dark, QStringLiteral("(the entries Light and Dark)"));
    if (!light || !dark)
        return;
    light->trigger();
    settle(400);
    check(look::active().theme == QLatin1String("light"), QStringLiteral("Light shows the theme \"Light\""), look::active().theme);
    emit menu->aboutToShow();
    check(menu->findChild<QAction *>(QStringLiteral("themeBright"))->isChecked(), QStringLiteral("the theme in use is checked in the menu"));
    shot(&w, QStringLiteral("theme-button-window"));
    menu->popup(button->mapToGlobal(QPoint(0, button->height())));
    settle(300);
    shot(menu, QStringLiteral("theme-button-menu"));
    menu->hide();
    // My Look 1 (empty): saves the look shown now
    QAction *one = menu->findChild<QAction *>(QStringLiteral("themeUser1"));
    one->trigger();
    settle(400);
    check(picker->themeOf(ThemePicker::User1) == QLatin1String("user:User_1"), QStringLiteral("choosing the empty My Look 1 saves the look shown now there"));
    emit menu->aboutToShow();
    dark = menu->findChild<QAction *>(QStringLiteral("themeDark"));
    dark->trigger();
    settle(400);
    check(look::active().theme == QLatin1String("dark") && theme::isDark(), QStringLiteral("Dark shows the dark theme"), look::active().theme);
    emit menu->aboutToShow();
    QAction *again = menu->findChild<QAction *>(QStringLiteral("themeUser1"));
    check(!plain(again->text()).contains(QLatin1String("empty")) && plain(again->text()) == QLatin1String("My Look 1"), QStringLiteral("a filled My Look 1 has no \"(empty)\""), again->text());
    again->trigger();
    settle(400);
    check(look::active().theme == QLatin1String("user:User_1"), QStringLiteral("My Look 1 brings its look back"), look::active().theme);
    bool listedAsMyLook = false;
    for (const theme::Entry &e : theme::available())
        if (e.id == QLatin1String("user:User_1"))
            listedAsMyLook = e.name == QLatin1String("My Look 1");
    check(listedAsMyLook, QStringLiteral("in the list of all themes (and View > Themes) the saved look is named \"My Look 1\""));
    // an entry of the list
    emit menu->aboutToShow();
    QAction *sepia = nullptr;
    for (QAction *a : menu->actions())
        if (a->data().toString() == QLatin1String("sepia") && a->objectName() == QLatin1String("themeEntry"))
            sepia = a;
    if (sepia) {
        sepia->trigger();
        settle(400);
    }
    check(sepia && look::active().theme == QLatin1String("sepia"), QStringLiteral("a theme chosen in the list is used"), look::active().theme);
    // the way back to the standard for what follows
    themeui::choose(&w, QStringLiteral("light"), false);
    QString e2;
    theme::removeUserTheme(QStringLiteral("user:User_1"), &e2);
    theme::removeUserTheme(QStringLiteral("user:User_2"), &e2);
    settle(200);
    w.findChild<QAction *>(QStringLiteral("showBarThemes"))->setChecked(false);
    settle(200);
}

// ---------------------------------------------------------------- item 66

// Item "placeholders in the other help windows": the Quick Start and the Developer Guide reach their last heading, and the
// View menu has the themes (Window has none).
static void helpWindows(MainWindow &w)
{
    std::printf("== the Quick Start and Developer Guide windows; View > Themes\n");
    for (const char *key : {"quickStart", "developerGuide"}) {
        QFile f(QLatin1String(std::strcmp(key, "quickStart") == 0 ? ":/docs/quick-start.md" : ":/docs/developer-guide.md"));
        f.open(QIODevice::ReadOnly);
        QString last;
        for (const QString &line : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n')))
            if (line.startsWith(QLatin1String("#")))
                last = line.mid(line.indexOf(QLatin1Char(' ')) + 1).trimmed();
        w.findChild<QAction *>(QLatin1String(key))->trigger();
        settle(500);
        QTextBrowser *tb = nullptr;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *t = top->findChild<QTextBrowser *>(QLatin1String(key)))
                tb = t;
        const QString text = tb ? tb->toPlainText() : QString();
        check(tb && !last.isEmpty() && text.contains(last), QStringLiteral("the %1 window shows the text up to its last heading").arg(QLatin1String(key)), last);
        if (tb)
            tb->window()->close();
    }
    check(escapePlaceholders(QStringLiteral("a <computer> b `<computer>`")).contains(QLatin1String("&lt;computer&gt; b `<computer>`")),
          QStringLiteral("placeholders are escaped outside code spans only"));
    auto *view = w.menuBar()->findChild<QMenu *>(QStringLiteral("themeMenu"));
    check(view && qobject_cast<QMenu *>(view->parent()) && qobject_cast<QMenu *>(view->parent())->title().remove(QLatin1Char('&')) == QLatin1String("View"),
          QStringLiteral("the themes are in the View menu"));
    bool inWindow = false;
    if (auto *wm = w.findChild<QMenu *>(QStringLiteral("windowMenu")))
        for (QAction *a : wm->actions())
            inWindow = inWindow || a->text().remove(QLatin1Char('&')) == QLatin1String("Themes");
    check(!inWindow, QStringLiteral("the Window menu has no Themes entry"));
}

static void manual(MainWindow &w, Project *p, BinderItem *doc)
{
    std::printf("== the manual's window: contents, search, F1 (item 66)\n");
    showManual(&w, QString());
    settle(500);
    ManualWindow *mw = manualWindow();
    check(mw && mw->isVisible(), QStringLiteral("the manual opens in its window"));
    if (!mw)
        return;
    auto *contents = mw->findChild<QTreeWidget *>(QStringLiteral("manualContents"));
    auto *search = mw->findChild<QLineEdit *>(QStringLiteral("manualSearch"));
    auto *text = mw->findChild<QTextBrowser *>(QStringLiteral("manual"));
    const QStringList heads = mw->headings();
    check(contents && search && text && heads.size() > 80, QStringLiteral("a contents list of the manual's headings, a search field and the text"), QString::number(heads.size()));
    bool numbered = false, hasCork = false, hasTroubles = false;
    for (const QString &h : heads) {
        numbered = numbered || h.startsWith(QLatin1String("1. First steps"));
        hasCork = hasCork || h.startsWith(QLatin1String("Corkboard"));
        hasTroubles = hasTroubles || h.contains(QLatin1String("Troubleshooting"));
    }
    check(numbered && hasCork && hasTroubles, QStringLiteral("the contents list holds the sections and their parts"));
    shot(mw, QStringLiteral("manual-window"));
    // a click on a heading jumps to it
    QTreeWidgetItem *target = nullptr;
    for (QTreeWidgetItemIterator it(contents); *it; ++it)
        if ((*it)->text(0).startsWith(QLatin1String("12. Composition mode")))
            target = *it;
    check(target != nullptr, QStringLiteral("(\"12. Composition mode\" is in the list)"));
    if (target) {
        QTreeWidgetItem *parent = target->parent();
        while (parent) {
            parent->setExpanded(true);
            parent = parent->parent();
        }
        emit contents->itemClicked(target, 0);
        settle(300);
        check(mw->topHeading().startsWith(QLatin1String("12. Composition mode")), QStringLiteral("a click in the contents list jumps to the heading"), mw->topHeading());
    }
    // the search: Ctrl+F, Return, Shift+Return, the count
    search->clear();
    mw->activateWindow();
    QTest::keyClick(text, Qt::Key_F, Qt::ControlModifier);
    settle(100);
    check(search->hasFocus() || mw->focusWidget() == search, QStringLiteral("Ctrl+F goes to the search field"));
    QTest::keyClicks(search, QStringLiteral("corkboard"));
    settle(300);
    auto *count = mw->findChild<QLabel *>(QStringLiteral("manualHits"));
    const int hits = mw->hitCount();
    check(hits > 5 && mw->currentHit() >= 0 && count && count->text().contains(QStringLiteral(" of %1").arg(hits)),
          QStringLiteral("typing a word marks the hits and says which one of how many"), count ? count->text() : QString());
    const int first = mw->currentHit();
    QTest::keyClick(search, Qt::Key_Return);
    settle(100);
    const int second = mw->currentHit();
    QTest::keyClick(search, Qt::Key_Return, Qt::ShiftModifier);
    settle(100);
    check(second == (first + 1) % hits && mw->currentHit() == first, QStringLiteral("Return shows the next hit, Shift+Return the previous one"),
          QStringLiteral("%1 %2 %3").arg(first).arg(second).arg(mw->currentHit()));
    mw->findChild<QToolButton *>(QStringLiteral("manualNext"))->click();
    mw->findChild<QToolButton *>(QStringLiteral("manualNext"))->click();
    mw->findChild<QToolButton *>(QStringLiteral("manualPrevious"))->click();
    check(mw->currentHit() == (first + 1) % hits, QStringLiteral("the buttons Next and Previous do the same"), QString::number(mw->currentHit()));
    check(!text->extraSelections().isEmpty() && text->extraSelections().size() == hits, QStringLiteral("every hit is marked in the text"));
    shot(mw, QStringLiteral("manual-search"));
    check(mw->search(QStringLiteral("qwertzuiopasdfgh")) == 0 && count->text().contains(QStringLiteral("No hits")), QStringLiteral("no hits: the line says so"), count->text());
    mw->search(QString());
    // F1: the section of the view
    auto *binder = w.findChild<BinderPanel *>();
    binder->select({doc}, true);
    settle(200);
    EditorPane *pane = nullptr;
    for (EditorPane *c : w.findChildren<EditorPane *>())
        if (c->isVisible() && !pane)
            pane = c;
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    auto sectionIn = [&](ViewMode m) {
        pane->setViewMode(m);
        settle(200);
        pane->setFocus();
        return w.manualSection();
    };
    check(sectionIn(ViewMode::Single) == QLatin1String("Writing"), QStringLiteral("F1 in the document: the writing section"));
    check(sectionIn(ViewMode::Corkboard) == QLatin1String("Corkboard"), QStringLiteral("F1 on the corkboard: the corkboard section"));
    check(sectionIn(ViewMode::Outliner) == QLatin1String("Outliner"), QStringLiteral("F1 in the outliner: the outliner section"));
    check(sectionIn(ViewMode::Combined) == QLatin1String("Combined Text"), QStringLiteral("F1 in Combined Text: its section"));
    pane->setViewMode(ViewMode::Single);
    settle(100);
    binder->setFocus();
    settle(100);
    check(w.manualSection() == QLatin1String("The binder"), QStringLiteral("F1 in the binder: the binder section"), w.manualSection());
    auto *insp = w.findChild<Inspector *>();
    struct Row { Inspector::Tab tab; const char *section; };
    for (const Row &r : {Row{Inspector::Bookmarks, "Bookmarks"}, Row{Inspector::Snapshots, "Snapshots"}, Row{Inspector::Comments, "Comments and footnotes"},
                         Row{Inspector::MindMap, "Mind maps"}, Row{Inspector::Objects, "Objects: characters, locations, and more"}, Row{Inspector::Notes, "The inspector"}}) {
        insp->showTab(r.tab);
        settle(100);
        QWidget *inside = insp->findChild<QWidget *>();
        for (QWidget *c : insp->findChildren<QWidget *>())
            if (c->isVisible() && c->focusPolicy() != Qt::NoFocus) {
                inside = c;
                break;
            }
        if (inside)
            inside->setFocus();
        settle(100);
        check(w.manualSection() == QLatin1String(r.section), QStringLiteral("F1 in the inspector's tab %1: %2").arg(int(r.tab)).arg(QLatin1String(r.section)), w.manualSection());
    }
    // the sections exist in the manual
    int missing = 0;
    QStringList gone;
    for (const QString &s : {QStringLiteral("Writing"), QStringLiteral("Corkboard"), QStringLiteral("Outliner"), QStringLiteral("Combined Text"), QStringLiteral("The binder"),
                             QStringLiteral("The inspector"), QStringLiteral("Bookmarks"), QStringLiteral("Snapshots"), QStringLiteral("Comments and footnotes"),
                             QStringLiteral("Mind maps"), QStringLiteral("Objects: characters, locations, and more"), QStringLiteral("Custom metadata"),
                             QStringLiteral("Speech Transcription"), QStringLiteral("Audio and video"), QStringLiteral("Options"),
                             QStringLiteral("Settings for one project"), QStringLiteral("Compiling: making the manuscript"), QStringLiteral("Compare and transfer"),
                             QStringLiteral("Targets, statistics, and writing tools"), QStringLiteral("Text Statistics"), QStringLiteral("Keywords"),
                             QStringLiteral("Project Replace"), QStringLiteral("Find by Formatting"), QStringLiteral("Themes, colors, and fonts"),
                             QStringLiteral("Updates and add-ons"), QStringLiteral("Crash reports and system information"), QStringLiteral("Toolbars of your own"),
                             QStringLiteral("The icon bar"), QStringLiteral("Layouts"), QStringLiteral("Styles"), QStringLiteral("Fonts"), QStringLiteral("Paragraph formatting"),
                             QStringLiteral("Default formatting"), QStringLiteral("Tables"), QStringLiteral("Pictures beside the text, captions, and descriptions"),
                             QStringLiteral("Printing"), QStringLiteral("Importing files"), QStringLiteral("Exporting documents"), QStringLiteral("Corrections while typing"),
                             QStringLiteral("Scriptwriting"), QStringLiteral("Icons"), QStringLiteral("The menus"), QStringLiteral("Composition mode"),
                             QStringLiteral("Editing a compile format")})
        if (!mw->showSection(s)) {
            ++missing;
            gone << s;
        }
    check(missing == 0, QStringLiteral("every section that F1 can name is a heading of the manual"), gone.join(QStringLiteral("; ")));
    mw->showSection(QStringLiteral("Corkboard"));
    settle(200);
    check(mw->topHeading().startsWith(QLatin1String("Corkboard")), QStringLiteral("showing a section scrolls to it"), mw->topHeading());
    // F1 in dialogs
    {
        auto *dlg = new CompileDialog(p, &w);
        dlg->show();
        dlg->activateWindow();
        (void)QTest::qWaitForWindowActive(dlg, 2000);
        check(manualhelp::sectionForWindow(dlg) == QLatin1String("Compiling: making the manuscript"), QStringLiteral("the compile window has its section"));
        mw->showSection(QString());
        // (a key press needs an active dialog, which a run without a window manager cannot give: the step F1 takes is made here)
        showManualAt(dlg, manualhelp::sectionForWindow(dlg));
        settle(500);
        ManualWindow *m2 = manualWindow();
        check(m2 && m2->isVisible() && m2->parentWidget() == dlg && m2->topHeading().contains(QLatin1String("Compiling")), QStringLiteral("F1 in the compile window opens the manual at its section (as a child of the window)"),
              m2 ? m2->topHeading() : QString());
        if (m2 && !m2->isVisible())
            check(false, QStringLiteral("(manual hidden)"));
        shot(dlg, QStringLiteral("compile-window-with-manual"));
        if (m2)
            m2->close();
        delete dlg;
        settle(200);
    }
    {
        QDialog plainDlg(&w);
        plainDlg.setWindowTitle(QStringLiteral("Some dialog"));
        plainDlg.show();
        plainDlg.activateWindow();
        (void)QTest::qWaitForWindowActive(&plainDlg, 2000);
        check(manualhelp::sectionForWindow(&plainDlg).isEmpty(), QStringLiteral("a dialog without a section has none"));
        showManualAt(&plainDlg, manualhelp::sectionForWindow(&plainDlg));
        settle(500);
        ManualWindow *m3 = manualWindow();
        check(m3 && m3->isVisible() && m3->topHeading().contains(QLatin1String("User Manual")), QStringLiteral("F1 in such a dialog opens the start of the manual"),
              m3 ? m3->topHeading() : QString());
        if (m3)
            m3->close();
        plainDlg.hide();
        settle(200);
    }
    // the developer matter moved
    const QString man = readText(kRoot + QStringLiteral("/docs/manual.md"));
    const QString guide = readText(kRoot + QStringLiteral("/docs/developer-guide.md"));
    check(!man.contains(QStringLiteral("design-decisions")) && !man.contains(QStringLiteral("Appendix: environment variables")) && !man.contains(QStringLiteral("For helpers"))
              && !man.contains(QStringLiteral("QB_ALLOW_UNSIGNED_CORE")) && !man.contains(QStringLiteral("Inside a project folder")) && !man.contains(QStringLiteral("since 0.81 no longer")),
          QStringLiteral("docs/manual.md no longer holds the developer matter (design-decisions.md, environment variables, the project's file layout)"));
    check(guide.contains(QStringLiteral("design-decisions.md")) && guide.contains(QStringLiteral("QB_ALLOW_UNSIGNED_CORE")) && guide.contains(QStringLiteral("Inside a project folder"))
              && guide.contains(QStringLiteral("Media engines")),
          QStringLiteral("docs/developer-guide.md holds it"));
    check(!man.contains(QRegularExpression(QStringLiteral("section 2[1-9]\\b"))) && man.contains(QStringLiteral("## 20. Troubleshooting")),
          QStringLiteral("the sections are numbered again: troubleshooting is section 20"));
}

// ---------------------------------------------------------------- item 82
static void authorName(MainWindow &w, Project *p)
{
    std::printf("== the author's name for the title page (item 82)\n");
    (void)w;
    AppSettings &as = AppSettings::instance();
    const compile::Format fmt = compile::builtinFormats().first();
    compile::AuthorInfo none;
    compile::Compiler c(p, fmt, p->compile, none);
    check(c.hasTitlePage(), QStringLiteral("the project (from the novel template) has a title page that the compile writes"));
    auto paragraphTexts = [](const compile::Manuscript &m) {
        QStringList out;
        for (const compile::Paragraph &pa : m.paragraphs) {
            QString t;
            for (const compile::Run &r : pa.runs)
                t += r.text;
            out << t.trimmed();
        }
        return out;
    };
    const QStringList without = paragraphTexts(c.build());
    check(!without.contains(QStringLiteral("by")) && !without.contains(QStringLiteral("by ")), QStringLiteral("without an author the title page has no \"by\" line"), without.join(QStringLiteral(" | ")).left(300));
    compile::AuthorInfo named{QStringLiteral("Ada"), QStringLiteral("Lovelace")};
    compile::Compiler c2(p, fmt, p->compile, named);
    const QStringList with = paragraphTexts(c2.build());
    check(with.contains(QStringLiteral("by")) && with.contains(QStringLiteral("Ada Lovelace")), QStringLiteral("with an author: \"by\" and the name"), with.join(QStringLiteral(" | ")).left(300));
    // the question
    as.forename.clear();
    as.surname.clear();
    as.authorAsked = false;
    p->compile.authorOverride.clear();
    CompileDialog::setAuthorAnswerForTests(QStringLiteral("Ada"), QStringLiteral("Lovelace"));
    check(CompileDialog::askAuthorIfNeeded(&w, p, fmt, p->compile) && as.forename == QLatin1String("Ada") && as.surname == QLatin1String("Lovelace") && as.authorAsked,
          QStringLiteral("the first compile with a title page asks for the name and keeps it with the author information"), as.forename);
    QSettings s;
    check(s.value(QStringLiteral("app/forename")).toString() == QLatin1String("Ada") && s.value(QStringLiteral("app/surname")).toString() == QLatin1String("Lovelace"),
          QStringLiteral("it is saved in the settings (File > Options > General shows it)"));
    check(!CompileDialog::askAuthorIfNeeded(&w, p, fmt, p->compile), QStringLiteral("... and the question comes once"));
    // left empty: kept as empty, asked no more
    as.forename.clear();
    as.surname.clear();
    as.authorAsked = false;
    CompileDialog::setAuthorAnswerForTests(QString(), QString());
    check(CompileDialog::askAuthorIfNeeded(&w, p, fmt, p->compile) && as.forename.isEmpty() && as.authorAsked && !CompileDialog::askAuthorIfNeeded(&w, p, fmt, p->compile),
          QStringLiteral("left empty: nothing is kept and the question does not come again"));
    // cancelled: asked again the next time
    as.authorAsked = false;
    CompileDialog::setAuthorAnswerForTests(QStringLiteral("X"), QStringLiteral("Y"), false);
    check(CompileDialog::askAuthorIfNeeded(&w, p, fmt, p->compile) && !as.authorAsked && as.forename.isEmpty(), QStringLiteral("Cancel keeps nothing and asks again next time"));
    // an author of the project's compile settings: no question
    p->compile.authorOverride = QStringLiteral("Someone");
    check(!CompileDialog::askAuthorIfNeeded(&w, p, fmt, p->compile), QStringLiteral("no question when the compile settings name an author"));
    p->compile.authorOverride.clear();
    // the dialog itself
    {
        as.authorAsked = false;
        as.forename.clear();
        as.surname.clear();
        // (the hook is off: the real window)
    }
    as.authorAsked = true;
    as.forename = QStringLiteral("Ada");
    as.surname = QStringLiteral("Lovelace");
    as.save();
}

// ---------------------------------------------------------------- item 90
static void addonWindow(MainWindow &w, const QString &work)
{
    std::printf("== the add-on install window, the full AppImage's window (item 90)\n");
    const QString folder = work + QStringLiteral("/speech-like");
    QDir().mkpath(folder + QStringLiteral("/payload/themes"));
    {
        QFile f(folder + QStringLiteral("/payload/themes/x.json"));
        f.open(QIODevice::WriteOnly);
        f.write("{}");
        QFile m(folder + QStringLiteral("/manifest.json"));
        m.open(QIODevice::WriteOnly);
        m.write("{\"id\": \"check-v0940d\", \"name\": \"Check Speech\", \"version\": \"1.0\", \"description\": \"Speech becomes text, on this computer only.\","
                " \"details\": [\"Recognition by whisper.cpp.\", \"Graphics cards through Vulkan.\", \"Brings Qt Multimedia 6.12.1 and the runtime u2404-qt6.12-r2.\"]}");
    }
    const QString out = work + QStringLiteral("/check-v0940d.qbpkg");
    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(python(), {kRoot + QStringLiteral("/tools/make-package.py"), QStringLiteral("addon"), folder, out});
    pr.waitForFinished(120000);
    const addons::PackageInfo info = addons::inspect(out);
    check(info.isValid() && info.manifest.details.size() == 3, QStringLiteral("a package's manifest can carry \"details\""), info.error);
    const QString writers = describePackage(info);
    check(writers.contains(QStringLiteral("Speech becomes text")) && !writers.contains(QStringLiteral("whisper")) && !writers.contains(QStringLiteral("Vulkan"))
              && !writers.contains(QStringLiteral("u2404")),
          QStringLiteral("the text for writers has no technical line"), writers);
    check(describeDetails(info).contains(QStringLiteral("whisper.cpp")) && describeDetails(info).contains(QStringLiteral("u2404-qt6.12-r2")),
          QStringLiteral("the technical lines are in the details"));
    // the real manifest of Speech Transcription
    const QString real = readText(kRoot + QStringLiteral("/addons/speech-transcription/manifest.json"));
    const QJsonObject mo = QJsonDocument::fromJson(real.toUtf8()).object();
    const QString desc = mo.value(QStringLiteral("description")).toString();
    bool technical = false;
    for (const QString &t : {QStringLiteral("whisper.cpp"), QStringLiteral("Vulkan"), QStringLiteral("Qt Multimedia"), QStringLiteral("TLS"), QStringLiteral("u2404")})
        technical = technical || desc.contains(t) || QJsonDocument(mo.value(QStringLiteral("whatsNew")).toArray()).toJson().contains(t.toUtf8());
    const QString det = QJsonDocument(mo.value(QStringLiteral("details")).toArray()).toJson();
    check(!technical && det.contains(QStringLiteral("whisper.cpp")) && det.contains(QStringLiteral("Vulkan")) && det.contains(QStringLiteral("Qt Multimedia 6.12.1"))
              && det.contains(QStringLiteral("TLS")) && det.contains(QStringLiteral("u2404-qt6.12-r2")),
          QStringLiteral("Speech Transcription: whisper.cpp, Vulkan, Qt Multimedia, the TLS plugin and the runtime are under details; the description is for writers"), desc);
    // the window
    bool seen = false, collapsed = false, opened = false, textOk = false;
    QTimer::singleShot(500, [&] {
        QDialog *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!d)
            return;
        auto *toggle = d->findChild<QToolButton *>(QStringLiteral("addonDetailsToggle"));
        auto *details = d->findChild<QTextBrowser *>(QStringLiteral("addonDetails"));
        seen = toggle && details;
        collapsed = seen && !details->isVisible();
        shot(d, QStringLiteral("addon-install-window"));
        if (seen) {
            toggle->click();
            settle(200);
            opened = details->isVisible();
            textOk = details->toPlainText().contains(QStringLiteral("whisper.cpp"));
            shot(d, QStringLiteral("addon-install-window-details"));
        }
        d->reject();
    });
    bool restart = false;
    runInstallPackage(&w, out, &restart);
    check(seen && collapsed, QStringLiteral("the install window has \"Details\", collapsed"));
    check(opened && textOk, QStringLiteral("a click on Details shows the technical lines"));
    // the full AppImage's window names the add-on by its manifest
    check(addons::packageDisplayName(out) == QLatin1String("Check Speech"), QStringLiteral("the add-on's name comes from its manifest"), addons::packageDisplayName(out));
    FullInstallWindow win(13, 159000000);
    win.show();
    win.setProgress(4, 13, addons::packageDisplayName(out), 60000000);
    check(win.currentText().contains(QStringLiteral("Check Speech")) && !win.currentText().contains(QStringLiteral("check-v0940d")), QStringLiteral("the window shows the add-on's name, not its id"), win.currentText());
    const QString pkgs = work + QStringLiteral("/full");
    QDir().mkpath(pkgs);
    QFile::copy(out, pkgs + QStringLiteral("/check-v0940d.qbpkg"));
    QStringList order;
    QStringList log;
    QString err;
    addons::installBundled(pkgs, &log, &err, [&](int done, int total, const QString &name, qint64) { order << QStringLiteral("%1/%2 %3").arg(done).arg(total).arg(name); });
    check(order.size() == 2 && order.first() == QLatin1String("0/1 Check Speech"), QStringLiteral("installBundled reports the add-on's name"), order.join(QStringLiteral(" | ")) + err);
    QDir(addons::addonsRoot() + QStringLiteral("/check-v0940d")).removeRecursively();
    QFile::remove(dataDir() + QStringLiteral("/full-addons-installed-" QB_APP_VERSION));
    shot(&win, QStringLiteral("full-install-window"));
    win.hide();
}

// ---------------------------------------------------------------- item 83
static QString run(const QStringList &args, int *rc = nullptr)
{
    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(python(), args);
    pr.waitForFinished(60000);
    if (rc)
        *rc = pr.exitCode();
    return QString::fromUtf8(pr.readAll());
}

static void downloadPage(const QString &work)
{
    std::printf("== the download page and the release workflow (item 83)\n");
    const QString names = work + QStringLiteral("/names"), site = work + QStringLiteral("/site");
    QDir().mkpath(names);
    auto touch = [&](const QString &n, const QByteArray &content) {
        QFile f(names + QLatin1Char('/') + n);
        f.open(QIODevice::WriteOnly);
        f.write(content);
    };
    for (const char *n : {"Quillbinder-0.94.0-beta-x86_64.AppImage", "Quillbinder-0.94.0-full-beta-x86_64.AppImage", "Quillbinder-0.94.0-beta-update.qbpkg",
                          "media-all-formats.qbpkg", "themes-extra.qbpkg", "template-memoir-windows.qbpkg", "Quillbinder-0.94.0-windows-x64-full-setup.exe"})
        touch(QString::fromLatin1(n), QByteArray());
    touch(QStringLiteral("speech-transcription.qbpkg"), "12345678\n");
    touch(QStringLiteral("full-addons-size"), "250000000\n");
    int rc = 0;
    const QString log = run({QStringLiteral("-I"), kRoot + QStringLiteral("/tools/download-page.py"), names, QStringLiteral("v0.94.0-beta"), site}, &rc);
    const QString page = readText(site + QStringLiteral("/index.html"));
    check(rc == 0 && !page.isEmpty(), QStringLiteral("tools/download-page.py writes the page"), log);
    check(page.contains(QStringLiteral("Execute")) && page.contains(QStringLiteral("KDE Plasma")) && page.contains(QStringLiteral("may ask whether to execute the file")),
          QStringLiteral("the Linux steps say that the desktop may ask whether to execute the file"));
    check(page.contains(QStringLiteral("data-addon-file=\"speech-transcription.qbpkg\"")) && page.contains(QStringLiteral("Speech Transcription"))
              && page.contains(QStringLiteral("12 MB")) && page.contains(QStringLiteral("data-addon-file=\"themes-extra.qbpkg\"")),
          QStringLiteral("the add-ons of the release are listed on the Linux card with name, one line and size"));
    const int linuxAt = int(page.indexOf(QStringLiteral("data-system=\"linux\"")));
    const int winAt = int(page.indexOf(QStringLiteral("data-system=\"windows\"")));
    const int speechAt = int(page.indexOf(QStringLiteral("data-addon-file=\"speech-transcription.qbpkg\"")));
    const int memoirAt = int(page.indexOf(QStringLiteral("data-addon-file=\"template-memoir-windows.qbpkg\"")));
    check(speechAt > linuxAt && memoirAt > winAt && linuxAt >= 0 && winAt >= 0,
          QStringLiteral("each add-on is listed under its own system (a package with \"windows\" in its name under Windows)"), QStringLiteral("%1 %2 %3 %4").arg(linuxAt).arg(winAt).arg(speechAt).arg(memoirAt));
    check(page.contains(QStringLiteral("250 MB")), QStringLiteral("the page reads full-addons-size (250 MB)"));
    check(!page.contains(QStringLiteral("data-addon-file=\"media-all-formats.qbpkg\"")), QStringLiteral("the audio and video package keeps its own line"));
    // the size tool
    const QString pk = work + QStringLiteral("/pk");
    QDir().mkpath(pk);
    const QString addonsDir = work + QStringLiteral("/addons-src");
    QDir().mkpath(addonsDir + QStringLiteral("/one"));
    QDir().mkpath(addonsDir + QStringLiteral("/two"));
    QProcess mk;
    mk.setProcessChannelMode(QProcess::MergedChannels);
    for (const char *id : {"one", "two"}) {
        const QString f = work + QStringLiteral("/src-") + QLatin1String(id);
        QDir().mkpath(f + QStringLiteral("/payload"));
        QFile pf(f + QStringLiteral("/payload/data.bin"));
        pf.open(QIODevice::WriteOnly);
        pf.write(QByteArray(1000, 'a'));
        pf.close();
        QFile m(f + QStringLiteral("/manifest.json"));
        m.open(QIODevice::WriteOnly);
        m.write(QStringLiteral("{\"id\": \"%1\", \"name\": \"%1\", \"version\": \"1.0\"}").arg(QLatin1String(id)).toUtf8());
        m.close();
        mk.start(python(), {kRoot + QStringLiteral("/tools/make-package.py"), QStringLiteral("addon"), f, pk + QLatin1Char('/') + QLatin1String(id) + QStringLiteral(".qbpkg")});
        mk.waitForFinished(60000);
    }
    const QString sizeOut = work + QStringLiteral("/size.txt");
    const QString sl = run({QStringLiteral("-I"), kRoot + QStringLiteral("/tools/full-addons-size.py"), pk, sizeOut, addonsDir}, &rc);
    check(rc == 0 && readText(sizeOut).trimmed() == QLatin1String("2000"), QStringLiteral("tools/full-addons-size.py: the unpacked size of the add-on packages"), sl + readText(sizeOut));
    // the workflow
    const QString flow = readText(kRoot + QStringLiteral("/.github/workflows/release.yml"));
    const qsizetype fullStep = flow.indexOf(QStringLiteral("- name: Full version (the AppImage with all signed add-ons inside)"));
    const qsizetype nextStep = flow.indexOf(QStringLiteral("- name: Check the full version"));
    const QString step = fullStep >= 0 && nextStep > fullStep ? flow.mid(fullStep, nextStep - fullStep) : QString();
    check(step.contains(QStringLiteral("tools/full-addons-size.py dist full-addons-size")),
          QStringLiteral("the step that builds the full AppImage writes full-addons-size"));
    check(flow.contains(QStringLiteral("            full-addons-size\n")) && flow.contains(QStringLiteral("cp assembled/full-addons-size release/full-addons-size"))
              && flow.contains(QStringLiteral("release-files/full-addons-size release-names/full-addons-size")),
          QStringLiteral("... it travels with the artifacts to the page job, which puts it beside the names"));
    // GitHub's runner of the release workflow has no PyYAML: parse with it where it is installed, else look for
    // the job's line in the text
    int hasYaml = 0;
    run({QStringLiteral("-I"), QStringLiteral("-c"), QStringLiteral("import yaml")}, &hasYaml);
    int yrc = 0;
    if (hasYaml == 0)
        run({QStringLiteral("-I"), QStringLiteral("-c"), QStringLiteral("import yaml,sys; d = yaml.safe_load(open(sys.argv[1])); assert 'jobs' in d and 'download-page' in d['jobs']"),
             kRoot + QStringLiteral("/.github/workflows/release.yml")}, &yrc);
    else
        yrc = flow.contains(QStringLiteral("\njobs:\n")) && flow.contains(QStringLiteral("\n  download-page:\n")) ? 0 : 1;
    check(yrc == 0, hasYaml == 0 ? QStringLiteral("the workflow is valid YAML and has its download-page job")
                                 : QStringLiteral("the workflow has its download-page job (no PyYAML here: the text is read)"));
}

// ---------------------------------------------------------------- item 61 and the rename and the translations
static void harness()
{
    std::printf("== the system test's harness, the rename, the German texts (item 61)\n");
    const QString linuxCopy = readText(kRoot + QStringLiteral("/tests/crashsafe/cs_sys_projects.cpp"));
    const QString android = readText(kRoot + QStringLiteral("/tests/android/systemtest/cs_sys_projects.cpp"));
    const qsizetype at = linuxCopy.indexOf(QStringLiteral(" Written in Scrivener format."));
    const QString after = at >= 0 ? linuxCopy.mid(at, 700) : QString();
    check(after.contains(QStringLiteral("chooseByMenu(w, QStringLiteral(\"save\"))")) && after.indexOf(QStringLiteral("\"save\"")) < after.indexOf(QStringLiteral("\"saveAs\"")),
          QStringLiteral("S-P03 (Linux copy): the sentence typed in Scrivener format is saved before the second Save As"));
    check(android.contains(QStringLiteral("chooseByMenu(w, QStringLiteral(\"save\"))")), QStringLiteral("(the Android copy has it)"));
    const QString common = readText(kRoot + QStringLiteral("/tests/crashsafe/cs_common.cpp"));
    check(common.contains(QStringLiteral("qobject_cast<QScrollArea *>(up)")) && common.contains(QStringLiteral("sa->ensureVisible(")),
          QStringLiteral("S-T03 (Linux copy): a right click brings the place into sight in a scroll area around the editor"));
    // the rename
    int bad = 0;
    for (const QString &f : {QStringLiteral("/docs/manual.md"), QStringLiteral("/src/ui/MainWindow.cpp"), QStringLiteral("/translations/quillbinder_de.ts")})
        if (readText(kRoot + f).contains(QStringLiteral("Scrivener Keys")) || readText(kRoot + f).contains(QStringLiteral("Scrivener &amp;Keys")) || readText(kRoot + f).contains(QStringLiteral("Scrivener-Tasten")))
            ++bad;
    check(bad == 0, QStringLiteral("\"Scrivener Keys in the Binder\" is nowhere in the manual, the source or the German texts"));
    // the German texts
    QTranslator tr;
    check(tr.load(kRoot + QStringLiteral("/translations/quillbinder_de.qm")), QStringLiteral("(the German file loads)"));
    auto de = [&](const char *context, const char *source) { return tr.translate(context, source); };
    struct T { const char *ctx, *src; };
    QStringList missing;
    for (const T &t : {T{"qb::ThemePicker", "Light"}, T{"qb::ThemePicker", "My Look 1"}, T{"qb::ThemePicker", "My Look 2"}, T{"qb::ThemePicker", "%1 (empty)"},
                       T{"qb::ThemePicker", "Save the Current Look as %1"}, T{"qb::ThemePicker", "Save in My Look 1"},
                       T{"qb::ManualWindow", "Search the manual (Ctrl+F)"}, T{"qb::ManualWindow", "Previous"}, T{"qb::ManualWindow", "Next"}, T{"qb::ManualWindow", "No hits"},
                       T{"qb::ManualWindow", "%1 of %2"}, T{"qb::ManualWindow", "Contents"},
                       T{"qb::CompileDialog", "Author's Name"}, T{"qb::CompileDialog", "First name:"},
                       T{"QObject", "My Look 1"},
                       T{"qb::MainWindow", "Quick-Add &Keys in the Binder"}, T{"qb::MainWindow", "Return adds a new document below; Space opens a Quick Reference window"}})
        if (de(t.ctx, t.src).isEmpty() || de(t.ctx, t.src) == QString::fromUtf8(t.src))
            missing << QStringLiteral("%1: %2").arg(QLatin1String(t.ctx), QLatin1String(t.src));
    check(missing.isEmpty(), QStringLiteral("the new texts have their German entries"), missing.join(QStringLiteral("; ")));
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    const QString homeDir = home.path() + QLatin1String("/h");
    QDir().mkpath(homeDir);
    qputenv("HOME", homeDir.toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QB_KEYBOARD_LAYOUT", "us");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0940dTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0940dTest"));
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
    BinderItem *doc = nullptr;
    for (BinderItem *it : p->allItems())
        if (it->type == ItemType::Text && !p->isInTrash(it)) {
            doc = it;
            break;
        }
    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);

    contrasts(w, p, doc);
    themeButton(w);
    manual(w, p, doc);
    helpWindows(w);
    authorName(w, p);
    addonWindow(w, work.path());
    downloadPage(work.path());
    harness();

    std::printf("%d failure(s)\n", failures);
    return failures;
}
