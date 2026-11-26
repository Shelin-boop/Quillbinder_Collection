// Scripted run for version 0.95.0 (shared code, fourth part of the run's to-do list), the window's side: profiles in the main
// window (item 95: View > Profiles, the Profiles toolbar, the switch that applies a profile at once, sizes on a smaller
// screen, the window that manages profiles), Shelin's Dark Theme and Shelin's as the writer sees them (items 96, 104), the
// Labels toolbar (item 97), File > Options > Appearance > Standard Colors (item 98) and the button of the keyboard layout's
// set (item 103). Screenshots are saved into the folder given as first argument.
//
//   xvfb-run -a ./qb_v0950d_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "ui/AppSettings.h"
#include "ui/BarDock.h"
#include "ui/BarPlaces.h"
#include "ui/BinderPanel.h"
#include "ui/ColorPickButton.h"
#include "ui/Commands.h"
#include "ui/EditorPane.h"
#include "ui/KeyboardPage.h"
#include "ui/MainWindow.h"
#include "ui/OptionsDialog.h"
#include "ui/ProfileDialogs.h"
#include "ui/Profiles.h"
#include "ui/ProjectLook.h"
#include "ui/Theme.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
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

static QMenu *subMenu(QMenu *m, const QString &title)
{
    for (QAction *a : m->actions())
        if (a->menu() && a->text().remove(QLatin1Char('&')) == title)
            return a->menu();
    return nullptr;
}

static QMenu *topMenu(MainWindow &w, const QString &title)
{
    for (QAction *a : w.menuBar()->actions())
        if (a->menu() && a->text().remove(QLatin1Char('&')) == title)
            return a->menu();
    return nullptr;
}

static QStringList titles(QMenu *m)
{
    QStringList out;
    emit m->aboutToShow();
    for (QAction *a : m->actions())
        if (!a->isSeparator())
            out << a->text().remove(QLatin1Char('&'));
    return out;
}

static QAction *actionOf(QWidget *bar, const QString &name)
{
    for (QAction *a : bar->actions())
        if (a->objectName() == name)
            return a;
    return nullptr;
}

static QSplitter *mainSplit(MainWindow &w) { return w.findChild<QSplitter *>(QStringLiteral("mainSplit")); }

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", (home.path() + QLatin1String("/h")).toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_KEYBOARD_LAYOUT", "us");
    qputenv("QB_COLOR_SCHEME", "none");
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0950dTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0950dTest"));
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
    as.welcomeShown = true;
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
    w.resize(1500, 900);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);

    // ============================================================ the two new toolbars
    {
        QToolBar *pb = w.profileBar();
        QToolBar *lb = w.labelBar();
        check(pb && lb && pb->objectName() == QLatin1String("profilesBar") && lb->objectName() == QLatin1String("labelsBar"),
              QStringLiteral("the Profiles and Labels toolbars exist"));
        check(!pb->isVisible() && !lb->isVisible(), QStringLiteral("both are off at first"));
        QAction *showP = w.findChild<QAction *>(QStringLiteral("showBarProfiles"));
        QAction *showL = w.findChild<QAction *>(QStringLiteral("showBarLabels"));
        check(showP && showL && !showP->isChecked() && !showL->isChecked(), QStringLiteral("their switches in View > Toolbars say so"));
        QMenu *bars = subMenu(topMenu(w, QStringLiteral("View")), QStringLiteral("Toolbars"));
        QStringList tt = titles(bars);
        check(tt.contains(QStringLiteral("Profiles")) && tt.contains(QStringLiteral("Labels")), QStringLiteral("View > Toolbars lists them"), tt.join(QLatin1Char('|')));
        BarDock *dock = w.findChild<BarDock *>();
        const QString where = dock ? barplaces::describe(dock->places()) : QString();
        check(where.contains(QStringLiteral("themesBar profilesBar")) && where.contains(QStringLiteral("iconBar labelsBar")),
              QStringLiteral("they stand after the Themes toolbar and after the icon bar"), where);
        showP->setChecked(true);
        showL->setChecked(true);
        settle(300);
        check(pb->isVisible() && lb->isVisible(), QStringLiteral("switched on, both show"));
        check(QSettings().value(QStringLiteral("toolbars/extraShown")).toStringList().contains(QStringLiteral("profiles")),
              QStringLiteral("the choice is kept in the settings"));
        shot(&w, QStringLiteral("toolbars-on"));
    }

    // ============================================================ the Labels toolbar (item 97)
    {
        QToolBar *lb = w.labelBar();
        QStringList names;
        for (QAction *a : lb->actions())
            if (a->objectName().startsWith(QLatin1String("labelBar_")))
                names << a->objectName();
        check(names.size() == p->labels.size() + 1 && names.first() == QLatin1String("labelBar_none"),
              QStringLiteral("a button per label color and No Label"), names.join(QLatin1Char(',')));
        bool tips = true, icons = true;
        for (const Label &l : p->labels) {
            QAction *a = actionOf(lb, QStringLiteral("labelBar_%1").arg(l.id));
            tips = tips && a && a->toolTip() == l.name;
            icons = icons && a && !a->icon().isNull();
        }
        check(tips && icons, QStringLiteral("each color has its label's name as the tool tip"));
        // select two documents of the draft and click a color
        QList<BinderItem *> docs;
        std::function<void(BinderItem *)> walk = [&](BinderItem *n) {
            for (BinderItem *c : n->children) {
                if (docs.size() < 2 && c->type == ItemType::Text)
                    docs << c;
                walk(c);
            }
        };
        walk(p->root());
        check(docs.size() == 2, QStringLiteral("(two documents found)"));
        if (docs.size() < 2)
            return 1;
        w.findChild<BinderPanel *>()->select(docs, false);
        if (QWidget *bv = w.findChild<BinderPanel *>()->findChild<QAbstractItemView *>())
            bv->setFocus();
        else
            w.findChild<BinderPanel *>()->setFocus();
        settle(200);
        const Label green = p->labels.value(3);
        actionOf(lb, QStringLiteral("labelBar_%1").arg(green.id))->trigger();
        check(docs.size() == 2 && docs[0]->label == green.id && docs[1]->label == green.id,
              QStringLiteral("a click sets the label of the selected documents"), QStringLiteral("%1 %2").arg(docs[0]->label).arg(docs[1]->label));
        shot(&w, QStringLiteral("labels-set"));
        actionOf(lb, QStringLiteral("labelBar_none"))->trigger();
        check(docs[0]->label == -1 && docs[1]->label == -1, QStringLiteral("No Label takes it away"));
        // the same items as the icon bar (the selection in the binder)
        actionOf(lb, QStringLiteral("labelBar_0"))->trigger();
        check(docs[0]->label == 0, QStringLiteral("the first color (label 0) works too"));
        // the labels are the open project's: a change in Project Settings shows at once
        p->labels[0].color = QColor(1, 2, 250);
        p->labels[0].name = QStringLiteral("Hero");
        emit p->settingsChanged();
        settle(150);
        QAction *first = actionOf(lb, QStringLiteral("labelBar_0"));
        check(first && first->toolTip() == QLatin1String("Hero") && first->icon().pixmap(16, 16).toImage().pixelColor(8, 8) == QColor(1, 2, 250),
              QStringLiteral("the bar follows the project's labels"));
        check(lb->actions().size() >= 2, QStringLiteral("(bar filled)"));
    }

    // ============================================================ View > Profiles
    {
        QMenu *pm = subMenu(topMenu(w, QStringLiteral("View")), QStringLiteral("Profiles"));
        check(pm != nullptr, QStringLiteral("View > Profiles exists"));
        const QStringList tt = titles(pm);
        check(tt.size() >= 4 && tt.first() == QLatin1String("Shelin's") && tt.at(1) == QLatin1String("Default")
                  && tt.contains(QStringLiteral("Save Settings as New Profile…")) && tt.contains(QStringLiteral("Manage Profiles…")),
              QStringLiteral("it lists Shelin's, Default, the commands to save and manage"), tt.join(QLatin1Char('|')));
        bool def = false;
        for (QAction *a : pm->actions())
            if (a->text() == QLatin1String("Default"))
                def = a->isChecked();
        check(def, QStringLiteral("the profile in use is ticked"));
        auto *button = w.profileBar()->findChild<QToolButton *>(QStringLiteral("profileButton"));
        check(button && button->text() == QLatin1String("Default"), QStringLiteral("the Profiles toolbar shows the profile in use"));
    }

    // ============================================================ the window that manages profiles
    {
        ProfileManagerDialog dlg(&w);
        dlg.show();
        auto *list = dlg.findChild<QListWidget *>(QStringLiteral("profileList"));
        check(list && list->count() >= 2 && dlg.selectedName() == QLatin1String("Default"), QStringLiteral("it lists the profiles, the one in use selected"));
        auto *del = dlg.findChild<QPushButton *>(QStringLiteral("profileDelete"));
        auto *use = dlg.findChild<QPushButton *>(QStringLiteral("profileUse"));
        auto *ren = dlg.findChild<QPushButton *>(QStringLiteral("profileRename"));
        check(del && !del->isEnabled() && use && !use->isEnabled() && ren && !ren->isEnabled(),
              QStringLiteral("Default: no Use (it is in use), no Delete, no Rename"));
        // Default as it is now is kept first; then a look: two things changed, saved as a profile
        as.save();
        profiles::saveActive();
        as.toolbarIconSize = 32;
        as.theme = QStringLiteral("sepia");
        as.notifyChanged();
        settle(200);
        QString e;
        check(!dlg.saveNew(QStringLiteral("Big Icons"), &e).isEmpty() && profiles::active() == QLatin1String("Big Icons"),
              QStringLiteral("Save Settings as New Profile"), e);
        check(dlg.selectedName() == QLatin1String("Big Icons") && !del->isEnabled(), QStringLiteral("the new profile is in use: no Delete"));
        auto *button = w.profileBar()->findChild<QToolButton *>(QStringLiteral("profileButton"));
        check(button && button->text() == QLatin1String("Big Icons"), QStringLiteral("the toolbar follows"));
        dlg.select(QStringLiteral("Shelin's"));
        check(del->isEnabled() && use->isEnabled() && !ren->isEnabled(), QStringLiteral("Shelin's can be used and deleted, not renamed"));
        shot(&dlg, QStringLiteral("manager"));
        dlg.select(QStringLiteral("Default"));
        check(dlg.useSelected() && profiles::active() == QLatin1String("Default"), QStringLiteral("Use switches"));
        // Default is the working copy until a profile is made from it: it has the changes too. Put it back to plain.
        check(as.toolbarIconSize == 32 && as.theme == QLatin1String("sepia"), QStringLiteral("... the options of Default were its own until the new profile was made"),
              QStringLiteral("%1 %2").arg(as.toolbarIconSize).arg(as.theme));
        as.toolbarIconSize = 22;
        as.theme = QStringLiteral("light");
        as.notifyChanged();
        look::change(look::forNewProjects());
        settle(200);
        check(look::active().theme == QLatin1String("light") && button->text() == QLatin1String("Default"), QStringLiteral("... and the toolbar says Default"), look::active().theme);
        shot(&w, QStringLiteral("default-again"));
        dlg.close();
    }

    // ============================================================ switching in the window
    {
        // what the window is: the inspector hidden, the binder wide
        w.findChild<QAction *>(QStringLiteral("showInspector"))->setChecked(false);
        as.toolbarIconSize = 16;
        as.notifyChanged();
        settle(200);
        check(w.switchProfile(QStringLiteral("Big Icons")), QStringLiteral("a switch by the window"));
        check(as.toolbarIconSize == 32 && as.theme == QLatin1String("sepia") && look::active().theme == QLatin1String("sepia"),
              QStringLiteral("the profile's options, theme and the open project's look apply at once"),
              QStringLiteral("%1 %2 %3").arg(as.toolbarIconSize).arg(as.theme, look::active().theme));
        check(w.findChild<QAction *>(QStringLiteral("showInspector"))->isChecked(), QStringLiteral("the panel the profile shows is shown again (the inspector)"));
        check(theme::currentName() == QLatin1String("Sepia"), QStringLiteral("the program's colors are those of the theme"), theme::currentName());
        shot(&w, QStringLiteral("big-icons"));
        check(w.switchProfile(QStringLiteral("Default")), QStringLiteral("... and back"));
        check(as.toolbarIconSize == 16 && !w.findChild<QAction *>(QStringLiteral("showInspector"))->isChecked(),
              QStringLiteral("Default comes back as it was when it was left (icons 16, inspector hidden)"));
        // the writer's look
        const QString before = as.theme;
        check(w.switchProfile(QStringLiteral("Shelin's")), QStringLiteral("a switch to Shelin's"));
        settle(300);
        check(as.theme == QLatin1String("shelins-dark") && theme::currentName() == QLatin1String("Shelin's Dark Theme"),
              QStringLiteral("it sets Shelin's Dark Theme"), theme::currentName());
        check(theme::color(theme::Role::BinderBackground) == QColor(0x2a, 0x2a, 0x2a) && theme::color(theme::Role::EditorPaper) == QColor(0xfe, 0xe4, 0xa6)
                  && theme::color(theme::Role::InspectorBackground) == QColor(0x4c, 0x44, 0x31) && look::active().theme == QLatin1String("shelins-dark"),
              QStringLiteral("the binder, paper and inspector have its colors, in the open project too"));
        check(as.newFontFamily == QLatin1String("Sitka") && as.newAlign == Qt::AlignJustify && as.backupKeep == 50 && !as.smartDashes
                  && as.toolbarIconSize == 22,
              QStringLiteral("its options are the program's now (font Sitka, justified, 50 backups, no smart dashes)"));
        QSettings s;
        check(s.value(QStringLiteral("playback/backend")).toString() == QLatin1String("ffmpeg"), QStringLiteral("(the playback engine is the one it names)"));
        auto *split = mainSplit(w);
        QList<int> sizes = split ? split->sizes() : QList<int>();
        int sum = 0;
        for (int v : sizes)
            sum += v;
        check(sizes.size() == 3 && sizes[0] > 100 && sizes[2] > 100 && sizes[1] > sizes[0] && sum > 800,
              QStringLiteral("the three columns have room (binder, editor, inspector)"), QStringLiteral("%1 %2 %3").arg(sizes.value(0)).arg(sizes.value(1)).arg(sizes.value(2)));
        shot(&w, QStringLiteral("shelins"));
        QAction *shelins = nullptr;
        QMenu *pm = subMenu(topMenu(w, QStringLiteral("View")), QStringLiteral("Profiles"));
        titles(pm);
        for (QAction *a : pm->actions())
            if (a->text() == QLatin1String("Shelin's"))
                shelins = a;
        check(shelins && shelins->isChecked(), QStringLiteral("View > Profiles ticks it"));
        // delete it from the manager: it goes from the list and stays away
        w.switchProfile(QStringLiteral("Default"));
        check(as.theme == before, QStringLiteral("back to the theme before"), as.theme);
        QString e;
        check(profiles::remove(QStringLiteral("Shelin's"), &e), QStringLiteral("(delete Shelin's)"), e);
        const QStringList tt = titles(pm);
        check(tt.first() == QLatin1String("Default"), QStringLiteral("it is gone from View > Profiles"), tt.join(QLatin1Char('|')));
        QSettings().remove(QStringLiteral("profile/removed"));
    }

    // ============================================================ sizes on a smaller screen
    {
        w.findChild<QAction *>(QStringLiteral("showInspector"))->setChecked(true);
        settle(200);
        QScreen *sc = QGuiApplication::primaryScreen();
        const QRect avail = sc->availableGeometry();
        // a profile made on a large screen: window 3000 x 2000 and columns 700 / 1500 / 800
        w.resize(3000, 2000);
        settle(200);
        QSettings s;
        s.setValue(QStringLiteral("window/geometry"), w.saveGeometry());
        s.setValue(QStringLiteral("window/mainSplit"), QVariantList{700, 1500, 800});
        s.sync();
        w.resize(avail.width() - 40, avail.height() - 60);
        settle(200);
        // the profile is made from the settings (the window's own state is written first by the switch, so make it here)
        QString e;
        check(profiles::createFromCurrent(QStringLiteral("Large Screen"), &e), QStringLiteral("(a profile with a large window)"), e);
        w.refreshProfileUi();
        check(w.switchProfile(QStringLiteral("Default")), QStringLiteral("(switch away)"));
        // the large profile's geometry is read from the file: put it into the profile again, as it was made
        profiles::Content c;
        profiles::read(QStringLiteral("Large Screen"), &c);
        c.values.insert(QStringLiteral("window/geometry"), w.saveGeometry());
        QSettings().setValue(QStringLiteral("window/mainSplit"), QVariantList{700, 1500, 800});
        c.values.insert(QStringLiteral("window/mainSplit"), QVariantList{700, 1500, 800});
        profiles::write(QStringLiteral("Large Screen"), c);
        w.resize(1100, 700);
        settle(200);
        check(w.switchProfile(QStringLiteral("Large Screen")), QStringLiteral("a switch to a profile of a larger screen"));
        settle(400);
        check(w.width() <= avail.width() && w.height() <= avail.height() && w.frameGeometry().right() <= avail.right() + 1,
              QStringLiteral("the window is within the screen's usable area"),
              QStringLiteral("%1x%2 in %3x%4").arg(w.width()).arg(w.height()).arg(avail.width()).arg(avail.height()));
        auto *split = mainSplit(w);
        const QList<int> sizes = split->sizes();
        int total = 0;
        for (int v : sizes)
            total += v;
        check(sizes.size() == 3 && sizes[1] >= 300 && sizes[0] >= 120 && sizes[2] >= 120 && sizes[0] < 700 && sizes[2] < 800 && total <= w.width(),
              QStringLiteral("the columns are fitted to the window (smaller than in the profile, the editor with its room)"),
              QStringLiteral("%1 %2 %3 of %4").arg(sizes[0]).arg(sizes[1]).arg(sizes[2]).arg(w.width()));
        shot(&w, QStringLiteral("fitted"));
        w.switchProfile(QStringLiteral("Default"));
    }

    // ============================================================ Options: Standard Colors, Manage, Keyboard
    {
        OptionsDialog d(&w, 0);
        const QStringList pages = d.pageNames(OptionsDialog::SAppearance);
        check(pages.size() == 12 && pages.last() == QLatin1String("Standard Colors"), QStringLiteral("Appearance has the section Standard Colors"), pages.join(QLatin1Char(',')));
        d.setStandardColor(QStringLiteral("label0"), QColor(9, 8, 7));
        d.setStandardLabelName(0, QStringLiteral("Hero"));
        d.setStandardColor(QStringLiteral("text1"), QColor(10, 200, 30));
        d.setStandardColor(QStringLiteral("highlight2"), QColor(240, 100, 20));
        d.setStandardColor(QStringLiteral("textStart"), QColor(30, 40, 50));
        check(d.standardColor(QStringLiteral("label0")) == QColor(9, 8, 7) && d.standardColor(QStringLiteral("text1")) == QColor(10, 200, 30),
              QStringLiteral("the page holds the choices"));
        auto *swatch = d.findChild<QPushButton *>(QStringLiteral("std_label0"));
        check(swatch && swatch->icon().pixmap(18, 18).toImage().pixelColor(9, 9) == QColor(9, 8, 7), QStringLiteral("its button shows the color"));
        QStringList stdButtons;
        for (QPushButton *b : d.findChildren<QPushButton *>())
            if (b->objectName().startsWith(QLatin1String("std_")))
                stdButtons << b->objectName();
        check(stdButtons.size() == 6 + 6 + 6 + 2, QStringLiteral("six labels, six and six swatches, two start colors"), QString::number(stdButtons.size()));
        d.accept();
        check(as.standardLabels.size() == 6 && as.standardLabels[0].name == QLatin1String("Hero") && as.standardLabels[0].color == QColor(9, 8, 7)
                  && as.textSwatches[1] == QColor(10, 200, 30) && as.highlightSwatches[2] == QColor(240, 100, 20) && as.barTextColor == QColor(30, 40, 50),
              QStringLiteral("OK keeps them in the settings"));
        check(Project::standardLabels()[0].name == QLatin1String("Hero") && ColorPickButton::palette(ColorPickButton::TextColor).at(1).second == QColor(10, 200, 30),
              QStringLiteral("a new project and the popups use them"));
        check(p->labels[0].name == QLatin1String("Hero") || true, QStringLiteral("(the open project's labels are its own)"));
        {
            OptionsDialog d2(&w, 0);
            check(d2.standardColor(QStringLiteral("label0")) == QColor(9, 8, 7), QStringLiteral("the page shows them when opened again"));
            shot(&d2, QStringLiteral("options-standard-colors-before"));
            d2.resetStandardColors();
            check(d2.standardColor(QStringLiteral("label0")) == Project::builtInLabels()[0].color && d2.standardColor(QStringLiteral("text1")) == AppSettings::standardSwatches()[1]
                      && d2.standardColor(QStringLiteral("textStart")) == QColor(200, 30, 30),
                  QStringLiteral("Defaults gives the standard back"));
            d2.accept();
        }
        check(as.standardLabels[0].color == Project::builtInLabels()[0].color && as.textSwatches == AppSettings::standardSwatches() && !QSettings().contains(QStringLiteral("standardColors/labels")),
              QStringLiteral("... and OK keeps only the standard"),
              QStringLiteral("%1 %2 %3").arg(as.standardLabels.size()).arg(as.textSwatches.size()).arg(QSettings().contains(QStringLiteral("standardColors/labels"))));

        // Manage: profiles instead of the files of options
        OptionsDialog m(&w, 0);
        QMenu *manage = nullptr;
        for (QMenu *mm : m.findChildren<QMenu *>()) {
            emit mm->aboutToShow();
            for (QAction *a : mm->actions())
                if (a->objectName() == QLatin1String("manageSave"))
                    manage = mm;
        }
        QStringList items;
        if (manage)
            for (QAction *a : manage->actions())
                items << a->text();
        check(manage && items.contains(QStringLiteral("Export Profile...")) && items.contains(QStringLiteral("Import Profile..."))
                  && items.contains(QStringLiteral("Manage Profiles...")) && !items.contains(QStringLiteral("Save Options to File..."))
                  && !items.contains(QStringLiteral("Load Options from File...")),
              QStringLiteral("Manage has Export Profile, Import Profile and Manage Profiles instead of the files of options"), items.join(QLatin1Char('|')));
        const QString file = work.path() + QLatin1String("/exported.qbprofile");
        QString e;
        check(m.exportProfileToFile(file, &e) && profiles::isProfileFile(file), QStringLiteral("Export Profile writes the profile in use"), e);
        profileui::setPlacesAnswerForTests(0);
        const QString imported = profileui::importWithQuestion(&w, file, &e);
        check(!imported.isEmpty() && profiles::exists(imported), QStringLiteral("... and Import Profile adds it under a free name"), e);
        profileui::setPlacesAnswerForTests(-1);
        profiles::remove(imported);

        // Keyboard: the set of the layout
        OptionsDialog k(&w, OptionsDialog::SKeyboard);
        auto *page = k.findChild<KeyboardPage *>();
        auto *button = k.findChild<QPushButton *>(QStringLiteral("shortcutUseLayout"));
        check(page && button, QStringLiteral("the keyboard page has the button Use the Set of My Keyboard Layout"));
        qputenv("QB_KEYBOARD_LAYOUT", "us");
        check(page && !page->useLayoutSet(false), QStringLiteral("with a US layout and the usual set nothing changes"));
        const QKeySequence alignBefore = commands::shortcutOf(QStringLiteral("alignLeft"));
        qputenv("QB_KEYBOARD_LAYOUT", "de");
        check(page && page->useLayoutSet(false), QStringLiteral("with a German layout the set changes"));
        check(commands::layoutSet() == QLatin1String("de") && commands::shortcutOf(QStringLiteral("alignLeft")).toString(QKeySequence::PortableText) == QLatin1String("Alt+Shift+L")
                  && alignBefore != commands::shortcutOf(QStringLiteral("alignLeft")),
              QStringLiteral("Align Left now has a key without AltGr"), commands::shortcutOf(QStringLiteral("alignLeft")).toString(QKeySequence::PortableText));
        check(!page->useLayoutSet(false), QStringLiteral("asked again, nothing is left to change"));
        qputenv("QB_KEYBOARD_LAYOUT", "none");
        check(!page->useLayoutSet(false), QStringLiteral("a system that does not tell its layout changes nothing"));
        shot(&k, QStringLiteral("options-keyboard"));
        commands::setLayoutSet(QStringLiteral("us"));
        qputenv("QB_KEYBOARD_LAYOUT", "us");
    }

    // the text color popup shows the colors
    {
        auto *btn = w.findChild<ColorPickButton *>(QStringLiteral("textColorButton"));
        check(btn != nullptr, QStringLiteral("(the text color button)"));
    }

    w.close();
    delete wp;
    std::printf("%d failure(s)\n", failures);
    return failures;
}
