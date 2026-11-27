// Checks of 0.99.0 (item 116): one size for all toolbars. View > Toolbars > Toolbar Size (Small, Normal, Large), the same in a
// bar's right-click menu and in Customize Toolbars; the icons of every bar (the format bars too, the icon bar, Profiles and
// Labels) and the font of the boxes follow it; a row of bars has one height at each size; the size is kept, a profile switch
// applies it; Reset in Customize Toolbars gives the standard buttons; the default is Normal (Large on Android).
//
//   xvfb-run -a ./qb_v0990c_tests /tmp/shots

#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "ui/Profiles.h"
#include "ui/Theme.h"
#include "ui/ToolbarConfig.h"

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QToolBar>
#include <QToolButton>
#include <cstdio>

using namespace qb;

static int failures = 0;
static QString shotDir;

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

static void settle(int ms = 200)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

static const char *kBarNames[] = {"mainToolbar", "themesBar", "searchBar", "toolsBar", "formatBar", "characterBar", "paragraphBar",
                                  "iconBar", "profilesBar", "labelsBar"};

static QList<QToolBar *> allBarsOf(MainWindow &w)
{
    QList<QToolBar *> bars;
    for (const char *name : kBarNames)
        if (auto *bar = w.findChild<QToolBar *>(QLatin1String(name)))
            bars << bar;
    return bars;
}

static QString sizes(MainWindow &w)
{
    QString s;
    for (QToolBar *bar : allBarsOf(w))
        s += QStringLiteral("%1 %2; ").arg(bar->objectName()).arg(bar->iconSize().width());
    return s;
}

static QList<QWidget *> boxesOf(MainWindow &w)
{
    QList<QWidget *> boxes;
    for (const char *name : {"styleCombo", "quickSearch", "themePicker", "profileButton"})
        if (auto *b = w.findChild<QWidget *>(QLatin1String(name)))
            boxes << b;
    for (QComboBox *c : w.findChildren<QComboBox *>())
        if (c->toolTip() == QObject::tr("Font of the selected text") || c->toolTip() == QObject::tr("Line spacing of the paragraph"))
            boxes << c;
    for (QDoubleSpinBox *s : w.findChildren<QDoubleSpinBox *>())
        if (s->toolTip() == QObject::tr("Font size of the selected text in points"))
            boxes << s;
    return boxes;
}

static QAction *sizeAction(MainWindow &w, int side)
{
    return w.findChild<QAction *>(side == toolbars::kSmall ? QStringLiteral("toolbarSizeSmall")
                                  : side == toolbars::kNormal ? QStringLiteral("toolbarSizeNormal") : QStringLiteral("toolbarSizeLarge"));
}

// every bar and every button on a bar has the size, and the boxes the font that goes with it
static bool barsHave(MainWindow &w, int side, QString *why)
{
    bool ok = true;
    for (QToolBar *bar : allBarsOf(w)) {
        if (bar->iconSize() != QSize(side, side)) {
            ok = false;
            *why += QStringLiteral("bar %1 has %2; ").arg(bar->objectName()).arg(bar->iconSize().width());
        }
        for (QToolButton *b : bar->findChildren<QToolButton *>())
            if (b->iconSize() != QSize(side, side) && b->objectName() != QLatin1String("qt_toolbar_ext_button")
                && b->objectName() != QLatin1String("qt_toolbar_grip")) {
                ok = false;
                *why += QStringLiteral("button %1 on %2 has %3; ").arg(b->objectName(), bar->objectName()).arg(b->iconSize().width());
            }
    }
    const double want = QApplication::font().pointSizeF() * toolbars::boxFontFactor(side);
    for (QWidget *box : boxesOf(w))
        if (qAbs(box->font().pointSizeF() - want) > 0.05) {
            ok = false;
            *why += QStringLiteral("box %1 font %2 (want %3); ").arg(box->objectName()).arg(box->font().pointSizeF()).arg(want);
        }
    return ok;
}

static int heightOfRowBars(MainWindow &w, QString *report)
{
    // the visible bars of one row (same y) have one height
    QMap<int, QList<QToolBar *>> rows;
    for (QToolBar *bar : allBarsOf(w))
        if (bar->isVisible())
            rows[bar->mapTo(&w, QPoint(0, 0)).y()] << bar;
    int bad = 0;
    for (auto it = rows.cbegin(); it != rows.cend(); ++it) {
        *report += QStringLiteral("row y=%1:").arg(it.key());
        for (QToolBar *bar : it.value()) {
            *report += QStringLiteral(" %1=%2").arg(bar->objectName()).arg(bar->height());
            if (bar->height() != it.value().first()->height())
                ++bad;
        }
        *report += QStringLiteral("; ");
    }
    return bad;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990cTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990cTest"));
    QApplication app(argc, argv);
    shotDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(shotDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();

    std::printf("== the default and the three sizes (0.99.0)\n");
    check(AppSettings::defaultToolbarIconSize(false) == 22 && AppSettings::defaultToolbarIconSize(true) == 32,
          QStringLiteral("the default is Normal (22 px) on Linux and Windows and Large (32 px) on Android"));
#ifdef Q_OS_ANDROID
    check(as.toolbarIconSize == 32, QStringLiteral("a program with no settings starts with Large on Android"));
#else
    check(as.toolbarIconSize == 22, QStringLiteral("a program with no settings starts with Normal"), QString::number(as.toolbarIconSize));
#endif
    check(toolbars::nearestSize(16) == 16 && toolbars::nearestSize(20) == 22 && toolbars::nearestSize(26) == 22 && toolbars::nearestSize(30) == 32
              && toolbars::nearestSize(48) == 32,
          QStringLiteral("a size between the three belongs to the nearest"));
    check(toolbars::sizeName(16) == QLatin1String("Small") && toolbars::sizeName(22) == QLatin1String("Normal")
              && toolbars::sizeName(32) == QLatin1String("Large"),
          QStringLiteral("the names are Small, Normal and Large"));
    check(toolbars::boxFontFactor(16) < toolbars::boxFontFactor(22) && toolbars::boxFontFactor(22) == 1.0
              && toolbars::boxFontFactor(22) < toolbars::boxFontFactor(32) && toolbars::boxFontFactor(20) > toolbars::boxFontFactor(16)
              && toolbars::boxFontFactor(20) < 1.0,
          QStringLiteral("the boxes' font grows with the size; Normal keeps the program's font"));

    as.theme = QStringLiteral("light");
    as.welcomeShown = true;
    as.spellCheck = false;
    as.backupOnClose = false;
    as.save();
    theme::apply();

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.move(0, 0);
    w.resize(1800, 900);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    settle(400);

    std::printf("== where the choice is\n");
    QMenu *sizeMenu = w.findChild<QMenu *>(QStringLiteral("toolbarSizeMenu"));
    QStringList choices;
    for (int side : {16, 22, 32})
        if (QAction *one = sizeAction(w, side))
            choices << one->text().remove(QLatin1Char('&')) + (one->isChecked() ? QStringLiteral("*") : QString());
    check(sizeMenu && choices == QStringList({QStringLiteral("Small"), QStringLiteral("Normal*"), QStringLiteral("Large")})
              && sizeMenu->actions().size() == 3 && sizeAction(w, 22)->actionGroup() && sizeAction(w, 22)->actionGroup()->isExclusive(),
          QStringLiteral("Toolbar Size: Small, Normal, Large as one group of choices, Normal chosen"), choices.join(QLatin1Char(',')));
    // in View > Toolbars: after the bars' switches, before Reset Toolbar Places
    QMenu *barsMenu = nullptr;
    for (QAction *top : w.menuBar()->actions())
        if (top->text().remove(QLatin1Char('&')) == QLatin1String("View") && top->menu())
            for (QAction *sub : top->menu()->actions())
                if (sub->menu() && sub->text().remove(QLatin1Char('&')) == QLatin1String("Toolbars"))
                    barsMenu = sub->menu();
    int atSize = -1, atReset = -1, atLabels = -1;
    if (barsMenu)
        for (int i = 0; i < barsMenu->actions().size(); ++i) {
            QAction *ac = barsMenu->actions().at(i);
            if (ac->menu() && ac->menu() == sizeMenu)
                atSize = i;
            if (ac->objectName() == QLatin1String("resetToolbarPlaces"))
                atReset = i;
            if (ac->objectName() == QLatin1String("showBarLabels"))
                atLabels = i;
        }
    check(barsMenu && atLabels >= 0 && atSize > atLabels && atReset > atSize,
          QStringLiteral("View > Toolbars > Toolbar Size stands after the bar switches and before Reset Toolbar Places"),
          QStringLiteral("labels %1, size %2, reset %3").arg(atLabels).arg(atSize).arg(atReset));
    {
        QMenu *popup = w.createPopupMenu();
        int pSize = -1, pReset = -1;
        for (int i = 0; i < popup->actions().size(); ++i) {
            if (popup->actions().at(i)->menu() && popup->actions().at(i)->menu() == sizeMenu)
                pSize = i;
            if (popup->actions().at(i)->objectName() == QLatin1String("resetToolbarPlaces"))
                pReset = i;
        }
        check(pSize >= 0 && pReset > pSize, QStringLiteral("a bar's right-click menu has Toolbar Size (the same menu) before Reset Toolbar Places"));
        delete popup;
    }
    {
        ToolbarDialog dlg(&w);
        auto *combo = dlg.findChild<QComboBox *>(QStringLiteral("toolbarIconSize"));
        QStringList names;
        if (combo)
            for (int i = 0; i < combo->count(); ++i)
                names << combo->itemText(i) + QLatin1Char('=') + QString::number(combo->itemData(i).toInt());
        check(combo && names == QStringList({QStringLiteral("Small=16"), QStringLiteral("Normal=22"), QStringLiteral("Large=32")})
                  && combo->currentData().toInt() == 22,
              QStringLiteral("Customize Toolbars offers Small, Normal, Large (it said Medium)"), names.join(QLatin1Char(',')));
    }

    std::printf("== each size: every bar, every button, every box\n");
    for (QAction *ac : w.findChildren<QAction *>(QStringLiteral("showBarProfiles")))
        if (!ac->isChecked())
            ac->setChecked(true);
    if (QAction *labels = w.findChild<QAction *>(QStringLiteral("showBarLabels")))
        if (!labels->isChecked())
            labels->setChecked(true);
    settle(300);
    QMap<int, double> boxFont;
    int shotNo = 0;
    for (int side : {16, 22, 32, 22}) {
        sizeAction(w, side)->trigger();
        settle(500);
        QString why;
        const QString name = side == 16 ? QStringLiteral("small") : side == 22 ? QStringLiteral("normal") : QStringLiteral("large");
        check(AppSettings::instance().toolbarIconSize == side && barsHave(w, side, &why),
              QStringLiteral("%1 (%2 px): the icons of all %3 bars and their buttons, and the font of the boxes").arg(name).arg(side).arg(allBarsOf(w).size()),
              why + sizes(w) + QStringLiteral(" setting %1").arg(AppSettings::instance().toolbarIconSize));
        QString report;
        const int bad = heightOfRowBars(w, &report);
        std::printf("      (%s)\n", qPrintable(report));
        check(bad == 0, QStringLiteral("... and the bars of a row have one height"));
        check(sizeAction(w, side)->isChecked(), QStringLiteral("... the choice shows it"));
        if (auto *quick = w.findChild<QWidget *>(QStringLiteral("quickSearch")))
            boxFont[side] = quick->font().pointSizeF();
        w.grab().save(shotDir + QStringLiteral("/size-%1-%2.png").arg(name).arg(shotNo++));
    }
    check(boxFont.value(16) > 0 && boxFont.value(16) < boxFont.value(22) && boxFont.value(22) < boxFont.value(32),
          QStringLiteral("the boxes' font is smaller at Small and larger at Large than at Normal"),
          QStringLiteral("%1 %2 %3").arg(boxFont.value(16)).arg(boxFont.value(22)).arg(boxFont.value(32)));
    // the format bars: no longer a size of their own (18 px before)
    if (auto *style = w.findChild<QToolBar *>(QStringLiteral("formatBar")))
        check(style->iconSize().width() == 22, QStringLiteral("the format bars have Normal's size (22 px), not 18 px as before"));

    std::printf("== kept, restart, profile switch, Customize Toolbars\n");
    // Customize Toolbars: Large chosen there, then Reset on a bar
    {
        ToolbarDialog dlg(&w, toolbars::Bar::Search);
        auto *combo = dlg.findChild<QComboBox *>(QStringLiteral("toolbarIconSize"));
        const QStringList standard = toolbars::standard(toolbars::Bar::Search);
        check(dlg.removeItem(standard.value(0)) && dlg.items() != standard, QStringLiteral("(a button taken off the Search bar in the editor)"));
        auto *reset = dlg.findChild<QPushButton *>(QStringLiteral("toolbarReset"));
        if (reset)
            reset->click();
        check(reset && reset->text() == QLatin1String("Reset") && dlg.items() == standard,
              QStringLiteral("Reset puts the standard buttons back into the chosen bar (no question)"), dlg.items().join(QLatin1Char(',')));
        if (combo)
            combo->setCurrentIndex(combo->findData(32));
        dlg.accept();
        settle(500);
        QString why;
        check(AppSettings::instance().toolbarIconSize == 32 && barsHave(w, 32, &why) && sizeAction(w, 32)->isChecked(),
              QStringLiteral("Large chosen in Customize Toolbars is shown on every bar and in the menu"), why);
    }
    // kept: a new program reads it
    as.save();
    delete wp;
    as.load();
    check(as.toolbarIconSize == 32 && QSettings().value(QStringLiteral("toolbars/iconSize")).toInt() == 32,
          QStringLiteral("the size is kept in the settings"), QString::number(as.toolbarIconSize));
    wp = new MainWindow;
    MainWindow &w2 = *wp;
    w2.move(0, 0);
    w2.resize(1800, 900);
    w2.show();
    w2.activateWindow();
    (void)QTest::qWaitForWindowActive(&w2, 2000);
    settle(500);
    {
        QString why;
        check(barsHave(w2, 32, &why) && sizeAction(w2, 32)->isChecked() && !sizeAction(w2, 22)->isChecked(),
              QStringLiteral("after a restart every bar has the size and the menu shows it"), why + sizes(w2));
    }
    // a profile switch applies the size of the profile
    {
        QString err;
        // a profile keeps what the settings were when it is left: Tiny is made and left with Small, Roomy with Large
        sizeAction(w2, 16)->trigger();
        settle(300);
        check(profiles::createFromCurrent(QStringLiteral("Tiny"), &err), QStringLiteral("(a profile saved with Small)"), err);
        auto *button = w2.findChild<QToolButton *>(QStringLiteral("profileButton"));
        QMenu *menu = button ? button->menu() : nullptr;
        auto choose = [&](const QString &profile) {
            if (!menu)
                return false;
            emit menu->aboutToShow();
            for (QAction *ac : menu->actions())
                if (ac->text().remove(QLatin1Char('&')) == profile) {
                    ac->trigger();
                    settle(600);
                    return true;
                }
            return false;
        };
        check(choose(QStringLiteral("Default")), QStringLiteral("(left Tiny: it keeps Small)"));
        sizeAction(w2, 32)->trigger();
        settle(300);
        check(profiles::createFromCurrent(QStringLiteral("Roomy"), &err), QStringLiteral("(a profile saved with Large)"), err);
        QString why;
        const bool toTiny = choose(QStringLiteral("Tiny"));
        check(toTiny && AppSettings::instance().toolbarIconSize == 16 && barsHave(w2, 16, &why) && sizeAction(w2, 16)->isChecked(),
              QStringLiteral("switching to a profile with Small gives every bar Small"), why + QString::number(AppSettings::instance().toolbarIconSize));
        why.clear();
        const bool toRoomy = choose(QStringLiteral("Roomy"));
        check(toRoomy && AppSettings::instance().toolbarIconSize == 32 && barsHave(w2, 32, &why) && sizeAction(w2, 32)->isChecked(),
              QStringLiteral("... and back to Large gives every bar Large"), why + QString::number(AppSettings::instance().toolbarIconSize));
    }
    delete wp;
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
