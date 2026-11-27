// Scripted run for version 0.94.0 (shared code, second part of the run's to-do list):
//  - item 68: one place per topic in the menus (View > Layout holds the positions and the saved layouts, View > Split
//    Editor, fonts in Format > Font, links in Insert, the add-ons and updates in Help > Add-ons and Updates) and
//    Help > Find Command;
//  - item 86: Help > Troubleshooting and Development;
//  - items 60 and 69: the first start's one page (backups, application menu, check for updates), no confirmation
//    box after the entry, Help > Add to / Remove From Application Menu following the state;
//  - item 70: the AppImage moved to ~/Applications (the running program and its updater follow the new place);
//  - item 71: the full AppImage's installation window and its one call (the start script, the folder's packages).
//
//   xvfb-run -a ./qb_v0940b_tests /tmp/shots
#include "AppInfo.h"
#include "core/Addons.h"
#include "core/Paths.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/UpdateCheck.h"
#include "ui/AppMenuEntry.h"
#include "ui/AppSettings.h"
#include "ui/CommandFinder.h"
#include "ui/FirstStartDialog.h"
#include "ui/FullInstallWindow.h"
#include "ui/MainWindow.h"
#include "ui/Theme.h"
#include "ui/UpdateChecker.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

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

static QString plain(QString t)
{
    t.remove(QLatin1Char('&'));
    t.remove(QChar(0x2026));
    return t.simplified();
}

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

static QStringList allMenuTexts(QMenu *m)
{
    QStringList out;
    if (!m)
        return out;
    for (QAction *a : m->actions()) {
        if (a->isSeparator())
            continue;
        out << plain(a->text());
        if (a->menu())
            out << allMenuTexts(a->menu());
    }
    return out;
}

static int countSubmenus(QMenu *m, const QString &name)
{
    int n = 0;
    for (QAction *a : m->actions())
        if (a->menu()) {
            if (plain(a->text()) == name)
                ++n;
            n += countSubmenus(a->menu(), name);
        }
    return n;
}

static QString fileText(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

static void writeFile(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(data);
}

static QString python()
{
    for (const char *name : {"python3", "python"}) {
        const QString p = QStandardPaths::findExecutable(QLatin1String(name));
        if (!p.isEmpty())
            return p;
    }
    return QStringLiteral("python3");
}

static QString makeAddon(const QString &dir, const QString &id)
{
    const QString folder = dir + QLatin1Char('/') + id;
    writeFile(folder + QLatin1String("/payload/themes/") + id + QLatin1String(".json"), "{}");
    writeFile(folder + QLatin1String("/manifest.json"),
              QStringLiteral("{\"id\": \"%1\", \"name\": \"Check %1\", \"version\": \"1.0\"}").arg(id).toUtf8());
    const QString out = dir + QLatin1Char('/') + id + QLatin1String(".qbpkg");
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(python(), {QStringLiteral(QB_TEST_SOURCE_DIR "/tools/make-package.py"), QStringLiteral("addon"), folder, out});
    p.waitForFinished(120000);
    return out;
}

// ---------------------------------------------------------------- items 68 and 86
static void menus(MainWindow &w)
{
    std::printf("== menus: one place per topic (items 68 and 86)\n");
    QMenu *view = topMenu(w, QStringLiteral("View"));
    QMenu *split = subMenu(view, QStringLiteral("Split Editor"));
    QMenu *layout = subMenu(view, QStringLiteral("Layout"));
    check(split && texts(split).contains(QStringLiteral("Split Horizontally")) && !subMenu(view, QStringLiteral("Editor Layout")),
          QStringLiteral("View > Editor Layout is View > Split Editor"));
    int layoutMenus = 0;
    for (QAction *top : w.menuBar()->actions())
        if (top->menu())
            layoutMenus += countSubmenus(top->menu(), QStringLiteral("Layout")) + countSubmenus(top->menu(), QStringLiteral("Layouts"))
                           + countSubmenus(top->menu(), QStringLiteral("Editor Layout"));
    check(layoutMenus == 1 && layout, QStringLiteral("\"Layout\" is the name of one menu only (View > Layout)"), QString::number(layoutMenus));
    emit layout->aboutToShow();
    const QStringList lt = texts(layout);
    check(lt.contains(QStringLiteral("Binder Left")) || lt.size() > 8, QStringLiteral("View > Layout holds the positions"), lt.join(QStringLiteral(" | ")));
    check(lt.contains(QStringLiteral("Default")) && lt.contains(QStringLiteral("Planning")) && lt.contains(QStringLiteral("Save Current Layout"))
              && lt.contains(QStringLiteral("Manage Layouts")),
          QStringLiteral("... and the saved layouts, Save Current Layout and Manage Layouts"), lt.join(QStringLiteral(" | ")));
    QMenu *window = topMenu(w, QStringLiteral("Window"));
    check(window && !subMenu(window, QStringLiteral("Layouts")) && !subMenu(window, QStringLiteral("Themes")) && subMenu(topMenu(w, QStringLiteral("View")), QStringLiteral("Themes")),
          QStringLiteral("Window has no Layouts and no Themes any more (View has Themes)"));
    for (const char *k : {"layoutSave", "layoutManage", "layoutReset", "layoutBinderLeft"})
        check(w.findChild<QAction *>(QLatin1String(k)) != nullptr, QStringLiteral("the action's object name stays: %1").arg(QLatin1String(k)));

    QMenu *nav = topMenu(w, QStringLiteral("Navigate"));
    QMenu *format = topMenu(w, QStringLiteral("Format"));
    QMenu *font = subMenu(format, QStringLiteral("Font"));
    check(!subMenu(nav, QStringLiteral("Fonts")) && font && texts(font).contains(QStringLiteral("Install Font Files"))
              && texts(font).contains(QStringLiteral("Open Font Folder")) && texts(font).contains(QStringLiteral("Open Font Manager of the Desktop"))
              && texts(font).contains(QStringLiteral("Reload Font List")) && texts(font).contains(QStringLiteral("Bold")),
          QStringLiteral("Navigate > Fonts is in Format > Font"), texts(font).join(QStringLiteral(" | ")));

    QMenu *edit = topMenu(w, QStringLiteral("Edit"));
    QMenu *insert = topMenu(w, QStringLiteral("Insert"));
    const QStringList links = {QStringLiteral("Link to Document"), QStringLiteral("Link to Object"), QStringLiteral("Insert Web Link")};
    bool inInsert = true, notInEdit = true;
    for (const QString &l : links) {
        inInsert = inInsert && texts(insert).contains(l);
        notInEdit = notInEdit && !allMenuTexts(edit).contains(l);
    }
    check(inInsert && notInEdit, QStringLiteral("the link commands are in Insert, not in Edit"), texts(insert).join(QStringLiteral(" | ")));

    QMenu *help = topMenu(w, QStringLiteral("Help"));
    QMenu *addons = subMenu(help, QStringLiteral("Add-ons and Updates"));
    check(addons
              && texts(addons) == QStringList({QStringLiteral("Add-on Manager"), QStringLiteral("Install Update or Add-on"),
                                               QStringLiteral("Check for Updates Now"), QStringLiteral("Check for Updates at Startup")}),
          QStringLiteral("Help > Add-ons and Updates: Add-on Manager, Install Update or Add-on, Check for Updates Now, at Startup"),
          texts(addons).join(QStringLiteral(" | ")));
    QMenu *file = topMenu(w, QStringLiteral("File"));
    QMenu *writing = subMenu(edit, QStringLiteral("Writing Tools"));
    check(!texts(file).contains(QStringLiteral("Install Update or Add-on")) && !texts(help).contains(QStringLiteral("Add-on Manager"))
              && !texts(help).contains(QStringLiteral("Check for Updates Now")) && writing && !texts(writing).contains(QStringLiteral("Add-on Manager")),
          QStringLiteral("File, Help and Edit > Writing Tools hold no add-on command of their own any more"));
    QMenu *trouble = subMenu(help, QStringLiteral("Troubleshooting and Development"));
    const QStringList tt = texts(trouble);
    check(trouble && tt.contains(QStringLiteral("Open Plugin Folder")) && tt.contains(QStringLiteral("Developer Guide: Updates and Add-ons"))
              && tt.contains(QStringLiteral("Crash Reports")) && tt.contains(QStringLiteral("This Computer")) && tt.contains(QStringLiteral("System Information")),
          QStringLiteral("Help > Troubleshooting and Development holds the five commands"), tt.join(QStringLiteral(" | ")));
    const int helpEntries = texts(help).size();
    check(helpEntries <= 13, QStringLiteral("Help has %1 entries (18 before)").arg(helpEntries));
    for (const char *k : {"installPackage", "docLink", "objectLink", "hyperlink", "addonManager", "updateCheckAuto", "updateCheckNow", "pluginFolder",
                          "developerGuide", "crashReports", "thisComputer", "systemInfo", "installFonts", "fontFolder", "noSplit", "integrate"})
        check(w.findChild<QAction *>(QLatin1String(k)) != nullptr, QStringLiteral("the action's object name stays: %1").arg(QLatin1String(k)));
    // every command of the menus has a place (the registry of the commands knows the same list)
    check(!help->actions().isEmpty() && help->actions().first()->objectName() == QLatin1String("findCommand"),
          QStringLiteral("Help starts with Find Command"));
    help->setProperty("qbShot", true);
    shot(help, QStringLiteral("menu-help"));
}

static void findCommand(MainWindow &w, Project *p)
{
    std::printf("== Help > Find Command (item 68)\n");
    QAction *fc = w.findChild<QAction *>(QStringLiteral("findCommand"));
    check(fc && fc->shortcut().isEmpty(), QStringLiteral("Find Command has no shortcut (Ctrl+Shift+P is the mind map's Page Setup)"));
    const QList<CommandFinder::Entry> all = CommandFinder::collect(w.menuBar());
    bool hasSplit = false, hasManual = false;
    for (const CommandFinder::Entry &e : all) {
        hasSplit = hasSplit || (e.label == QLatin1String("Split Horizontally") && e.path == QLatin1String("View > Split Editor"));
        hasManual = hasManual || (e.label == QLatin1String("Install Update or Add-on") && e.path == QLatin1String("Help > Add-ons and Updates"));
    }
    check(all.size() > 150 && hasSplit && hasManual, QStringLiteral("the list holds every menu command with its menu path (%1 commands)").arg(all.size()));
    int shortcuts = 0;
    for (const CommandFinder::Entry &e : all)
        shortcuts += e.shortcut.isEmpty() ? 0 : 1;
    check(shortcuts > 30, QStringLiteral("... and the shortcuts"), QString::number(shortcuts));
    check(CommandFinder::matching(all, QStringLiteral("zzzzqq")).isEmpty() && CommandFinder::matching(all, QString()).size() == all.size(),
          QStringLiteral("typing narrows the list (an empty field shows all, nonsense none)"));
    check(CommandFinder::matching(all, QStringLiteral("editor split")).size() >= 3 && CommandFinder::matching(all, QStringLiteral("SPLIT editor")).size() >= 3,
          QStringLiteral("... by words in any order, upper or lower case"));

    // a project is open: Show Inspector toggles
    QAction *showInspector = w.findChild<QAction *>(QStringLiteral("showInspector"));
    const bool before = showInspector->isChecked();
    auto *dlg = new CommandFinder(w.menuBar(), &w);
    dlg->show();
    settle(200);
    dlg->setFilter(QStringLiteral("show inspector"));
    auto *list = dlg->findChild<QListWidget *>(QStringLiteral("commandList"));
    check(list && list->count() >= 1 && dlg->visibleRows() == list->count(), QStringLiteral("the window filters its list"), QString::number(list ? list->count() : -1));
    shot(dlg, QStringLiteral("find-command-filtered"));
    auto *field = dlg->findChild<QLineEdit *>(QStringLiteral("commandSearch"));
    QTest::keyClick(field, Qt::Key_Return);
    settle(300);
    check(showInspector->isChecked() != before && !dlg->isVisible(), QStringLiteral("Return runs the chosen command and closes the window"));
    showInspector->trigger(); // back
    settle(100);
    delete dlg;

    // a double-click runs it as well
    QAction *showBinder = w.findChild<QAction *>(QStringLiteral("showBinder"));
    const bool bBefore = showBinder->isChecked();
    dlg = new CommandFinder(w.menuBar(), &w);
    dlg->show();
    dlg->setFilter(QStringLiteral("show binder"));
    list = dlg->findChild<QListWidget *>(QStringLiteral("commandList"));
    emit list->itemActivated(list->item(0));
    settle(300);
    check(showBinder->isChecked() != bBefore, QStringLiteral("a double-click runs the command"));
    showBinder->trigger();
    settle(100);
    delete dlg;

    // an off command is shown greyed and does not run
    w.closeProject();
    settle(300);
    dlg = new CommandFinder(w.menuBar(), &w);
    dlg->show();
    dlg->setFilter(QStringLiteral("Project Targets"));
    list = dlg->findChild<QListWidget *>(QStringLiteral("commandList"));
    check(list && list->count() >= 1 && !(list->item(0)->flags() & Qt::ItemIsEnabled), QStringLiteral("a command that is off is shown greyed"));
    check(list && !dlg->runRow(list->item(0)->data(Qt::UserRole).toInt()) && dlg->isVisible(), QStringLiteral("... and does not run"));
    shot(dlg, QStringLiteral("find-command-greyed"));
    delete dlg;
    (void)p;
}

// ---------------------------------------------------------------- items 60, 69 and 70
static void appMenu(MainWindow &w, const QString &home)
{
    std::printf("== the entry in the application menu (items 60, 69 and 70)\n");
#ifdef Q_OS_WIN
    // Windows 0.94.0: no AppImage and no application menu of the Linux desktop; the command is hidden (the setup program
    // and the portable program's guide make the Start menu entry)
    {
        QAction *integrate = w.findChild<QAction *>(QStringLiteral("integrate"));
        check(integrate && !integrate->isVisible() && !integrate->isEnabled(), QStringLiteral("Windows: Help shows no \"Add to Application Menu\""));
        qputenv("APPIMAGE", (home + QLatin1String("/Downloads/Quillbinder-9.9.9-test-x86_64.AppImage")).toUtf8());
        check(!appmenu::runsFromAppImage() && !appmenu::canMove(), QStringLiteral("Windows: $APPIMAGE set makes no AppImage of the program and offers no move"));
        qunsetenv("APPIMAGE");
        return;
    }
#endif
    const QString data = qEnvironmentVariable("XDG_DATA_HOME");
    // an AppImage in the Downloads folder
    const QString dl = home + QLatin1String("/Downloads/Quillbinder-9.9.9-test-x86_64.AppImage");
    writeFile(dl, QByteArray(40000, 'x'));
    QFile::setPermissions(dl, QFile::permissions(dl) | QFileDevice::ExeOwner);
    qputenv("APPIMAGE", dl.toUtf8());
    check(appmenu::runsFromAppImage() && appmenu::canMove() && !appmenu::isInApplications(dl) && !appmenu::hasEntry(),
          QStringLiteral("an AppImage outside ~/Applications may be moved; there is no entry yet"));

    // Help > Add to Application Menu: the question with the check box, checked by default
    QAction *integrate = w.findChild<QAction *>(QStringLiteral("integrate"));
    QMenu *help = topMenu(w, QStringLiteral("Help"));
    emit help->aboutToShow();
    check(plain(integrate->text()) == QLatin1String("Add to Application Menu"), QStringLiteral("Help offers \"Add to Application Menu\" while there is no entry"));
    bool sawBox = false, checkedByDefault = false;
    int boxes = 0;
    auto *poll = new QTimer(&w);
    poll->setInterval(50);
    QObject::connect(poll, &QTimer::timeout, poll, [&] {
        auto *mb = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (!mb)
            return;
        ++boxes;
        if (auto *cb = mb->findChild<QCheckBox *>(QStringLiteral("moveAppImage"))) {
            sawBox = true;
            checkedByDefault = cb->isChecked();
            shot(mb, QStringLiteral("help-add-to-menu"));
            for (QAbstractButton *b : mb->buttons())
                if (mb->buttonRole(b) == QMessageBox::AcceptRole) {
                    b->click();
                    return;
                }
        }
        mb->accept();
    });
    poll->start();
    integrate->trigger();
    settle(500);
    poll->stop();
    const QString moved = appmenu::applicationsFolder() + QLatin1String("/Quillbinder-9.9.9-test-x86_64.AppImage");
    check(sawBox && checkedByDefault, QStringLiteral("Help > Add to Application Menu offers the move, checked by default"));
    check(boxes == 1, QStringLiteral("... and no confirmation box follows (one box in all)"), QString::number(boxes));
    check(QFile::exists(moved) && !QFile::exists(dl) && QFileInfo(moved).permissions() & QFileDevice::ExeOwner,
          QStringLiteral("the AppImage is in ~/Applications now (still executable), no longer in Downloads"));
    check(qEnvironmentVariable("APPIMAGE") == moved && UpdateChecker::ownAppImage() == moved,
          QStringLiteral("$APPIMAGE and the updater's idea of the program file follow the new path"), UpdateChecker::ownAppImage());
    const QString desktop = fileText(data + QLatin1String("/applications/" QB_APP_ID ".desktop"));
    check(desktop.contains(QStringLiteral("Exec=\"%1\"").arg(moved)), QStringLiteral("the menu entry points to the new place"), desktop.left(300));
    check(QFile::exists(data + QLatin1String("/icons/hicolor/scalable/apps/" QB_APP_ID ".svg"))
              && QFile::exists(data + QLatin1String("/mime/packages/" QB_APP_ID ".xml")),
          QStringLiteral("the icon and the file types are made"));
    check(QSettings().value(QStringLiteral("app/integrated")).toString() == moved, QStringLiteral("the setting that names the AppImage follows"));
    // the program update puts its ".before-update" copy beside the new place
    {
        const QString fresh = home + QLatin1String("/fresh.AppImage");
        writeFile(fresh, QByteArray(30000, 'y'));
        QString backupPath, err;
        const bool ok = UpdateChecker::replaceOwnFile(UpdateChecker::ownAppImage(), fresh, &backupPath, &err);
        check(ok && QFile::exists(moved + QLatin1String(".before-update")) && QFileInfo(moved).size() == 30000,
              QStringLiteral("a program update replaces the file in ~/Applications and keeps the .before-update copy beside it"), err);
        QFile::remove(moved + QLatin1String(".before-update"));
    }
    emit help->aboutToShow();
    check(plain(integrate->text()) == QLatin1String("Remove From Application Menu"), QStringLiteral("Help now offers \"Remove From Application Menu\""));
    check(appmenu::hasEntry() && !appmenu::canMove(), QStringLiteral("the entry exists; there is nothing left to move"));

    // Remove From Application Menu: what adding made is gone, the AppImage stays
    integrate->trigger();
    settle(300);
    check(!appmenu::hasEntry() && !QFile::exists(data + QLatin1String("/icons/hicolor/scalable/apps/" QB_APP_ID ".svg"))
              && !QFile::exists(data + QLatin1String("/mime/packages/" QB_APP_ID ".xml")) && QFile::exists(moved),
          QStringLiteral("Remove From Application Menu takes away the entry file, the icon and the file types; the AppImage stays"));
    emit help->aboutToShow();
    check(plain(integrate->text()) == QLatin1String("Add to Application Menu") && QSettings().value(QStringLiteral("app/integrated")).toString().isEmpty(),
          QStringLiteral("... and Help offers \"Add\" again"));

    // a second AppImage: the move fails because of a file of that name (nothing is lost, the entry points to the old place)
    const QString dl2 = home + QLatin1String("/Downloads/Other.AppImage");
    writeFile(dl2, QByteArray(20000, 'z'));
    writeFile(appmenu::applicationsFolder() + QLatin1String("/Other.AppImage"), "in the way");
    qputenv("APPIMAGE", dl2.toUtf8());
    const appmenu::Result r = appmenu::add(true, {});
    check(r.ok && !r.moved && !r.moveError.isEmpty() && QFile::exists(dl2) && qEnvironmentVariable("APPIMAGE") == dl2
              && fileText(data + QLatin1String("/applications/" QB_APP_ID ".desktop")).contains(QStringLiteral("Exec=\"%1\"").arg(dl2)),
          QStringLiteral("a move that cannot be made loses nothing: the file stays, the entry points to the old place, the reason is given"),
          r.moveError);
    QString err;
    check(appmenu::remove(&err) && !appmenu::hasEntry(), QStringLiteral("remove() after it"), err);
    // already in ~/Applications: no move is offered
    qputenv("APPIMAGE", moved.toUtf8());
    check(!appmenu::canMove() && appmenu::isInApplications(moved), QStringLiteral("a file in ~/Applications is not offered for a move"));
    // without an AppImage (a plain build) the entry is made without any box
    qunsetenv("APPIMAGE");
    check(!appmenu::runsFromAppImage() && !appmenu::canMove(), QStringLiteral("a program that does not run from an AppImage has no move"));
    // the first start's choice: no box, the entry made
    QFile::remove(appmenu::applicationsFolder() + QLatin1String("/Other.AppImage"));
    qputenv("APPIMAGE", dl2.toUtf8());
    boxes = 0;
    poll->start();
    w.applyFirstStartMenuChoice(true, true);
    settle(400);
    poll->stop();
    const QString moved2 = appmenu::applicationsFolder() + QLatin1String("/Other.AppImage");
    check(boxes == 0 && QFile::exists(moved2) && !QFile::exists(dl2) && appmenu::hasEntry() && qEnvironmentVariable("APPIMAGE") == moved2,
          QStringLiteral("the first start's choice makes the entry and moves the file without a box"), QString::number(boxes));
    appmenu::remove(&err);
    QFile::remove(moved2);
    QFile::remove(moved);
    qunsetenv("APPIMAGE");
}

static void firstStart(MainWindow &w, const QString &home)
{
    std::printf("== the first start's one page (items 60 and 69)\n");
    qunsetenv("QB_NO_UPDATE_CHECK");
    const QString dl = home + QLatin1String("/Downloads/First.AppImage");
    writeFile(dl, QByteArray(20000, 'f'));
    qputenv("APPIMAGE", dl.toUtf8());
#ifdef Q_OS_WIN
    // Windows 0.94.0 (item 69): the page has no question about the application menu, even with $APPIMAGE set
    {
        FirstStartDialog dlg(&w);
        dlg.show();
        settle(300);
        auto *upd = dlg.findChild<QCheckBox *>(QStringLiteral("firstUpdates"));
        auto *backup = dlg.findChild<QCheckBox *>(QStringLiteral("firstBackup"));
        check(!dlg.findChild<QCheckBox *>(QStringLiteral("firstMenuAdd")) && !dlg.findChild<QCheckBox *>(QStringLiteral("firstMenuMove"))
                  && !dlg.findChild<QLabel *>(QStringLiteral("firstMenuHint")) && upd && backup && upd->isChecked() && !dlg.hasMenuQuestion(),
              QStringLiteral("Windows: the page holds the backups and the check for updates, nothing of the application menu"));
        check(dlg.findChildren<QDialogButtonBox *>().size() == 1 && dlg.findChildren<QDialogButtonBox *>().first()->buttons().size() == 1,
              QStringLiteral("one OK"));
        shot(&dlg, QStringLiteral("first-start-page"));
        check(dlg.hasUpdateQuestion() && dlg.checkForUpdates(), QStringLiteral("the answers are read from the page"));
        if (upd)
            upd->setChecked(false);
        check(!dlg.checkForUpdates(), QStringLiteral("the check for updates can be switched off on the page"));
    }
#else
    {
        FirstStartDialog dlg(&w);
        dlg.show();
        settle(300);
        auto *add = dlg.findChild<QCheckBox *>(QStringLiteral("firstMenuAdd"));
        auto *move = dlg.findChild<QCheckBox *>(QStringLiteral("firstMenuMove"));
        auto *upd = dlg.findChild<QCheckBox *>(QStringLiteral("firstUpdates"));
        auto *backup = dlg.findChild<QCheckBox *>(QStringLiteral("firstBackup"));
        check(add && move && upd && backup && add->isChecked() && move->isChecked() && upd->isChecked(),
              QStringLiteral("the page holds the backups, the application menu (with the move) and the check for updates; everything is filled in"));
        auto *hint = dlg.findChild<QLabel *>(QStringLiteral("firstMenuHint"));
        check(hint && hint->text().contains(QLatin1String("Remove From Application Menu")) && hint->text().contains(QLatin1String("Add to Application Menu")),
              QStringLiteral("a line says that Help > Add to / Remove From Application Menu changes it later"));
        check(dlg.findChildren<QDialogButtonBox *>().size() == 1 && dlg.findChildren<QDialogButtonBox *>().first()->buttons().size() == 1,
              QStringLiteral("one OK"));
        shot(&dlg, QStringLiteral("first-start-page"));
        check(dlg.hasMenuQuestion() && dlg.addToMenu() && dlg.moveAppImage() && dlg.hasUpdateQuestion() && dlg.checkForUpdates(),
              QStringLiteral("the answers are read from the page"));
        add->setChecked(false);
        check(!move->isEnabled() && !dlg.moveAppImage() && !dlg.addToMenu(), QStringLiteral("without the entry the move is off"));
        upd->setChecked(false);
        check(!dlg.checkForUpdates(), QStringLiteral("the check for updates can be switched off on the page"));
    }
#endif
    // the page's answer reaches the check for updates
    UpdateChecker *uc = w.updateChecker();
    updates::State st = updates::State::load();
    st.noteShown = false;
    st.enabled = true;
    st.save();
    QAction *atStart = w.findChild<QAction *>(QStringLiteral("updateCheckAuto"));
    if (uc) {
        uc->firstStartAnswered(false);
        check(updates::State::load().noteShown && !updates::State::load().enabled && atStart && !atStart->isChecked(),
              QStringLiteral("\"off\" on the page: noted as seen, switched off, the menu's switch follows"));
        uc->firstStartAnswered(true);
        check(updates::State::load().noteShown && updates::State::load().enabled && atStart->isChecked(), QStringLiteral("\"on\": switched on"));
    }
    // no menu question without an AppImage; none for the update note once it was seen
    qunsetenv("APPIMAGE");
    {
        FirstStartDialog dlg(&w);
        check(!dlg.hasMenuQuestion(), QStringLiteral("without an AppImage the page has no question about the application menu"));
        check(!dlg.hasUpdateQuestion(), QStringLiteral("a note that was seen is not asked again"));
    }
    qputenv("QB_NO_UPDATE_CHECK", "1");
    // declined at the first start: not asked again at the next start
    qputenv("APPIMAGE", dl.toUtf8());
#ifdef Q_OS_WIN
    w.applyFirstStartMenuChoice(true, true);
    check(QSettings().value(QStringLiteral("app/integrated")).toString().isEmpty() && !appmenu::hasEntry() && QFile::exists(dl),
          QStringLiteral("Windows: the first start's menu choice makes no entry and moves nothing"));
#else
    w.applyFirstStartMenuChoice(false, false);
    check(QSettings().value(QStringLiteral("app/integrationDeclined")).toString() == dl && !appmenu::hasEntry(),
          QStringLiteral("\"no\" on the page makes no entry and is remembered"));
#endif
    qunsetenv("APPIMAGE");
}

// ---------------------------------------------------------------- item 71
static void fullInstall(const QString &work)
{
    std::printf("== the full AppImage's installation window (item 71)\n");
    check(FullInstallWindow::message(13, 159000000) == QLatin1String("Installing 13 add-ons (159 MB)..."), QStringLiteral("the message names the count and the size"),
          FullInstallWindow::message(13, 159000000));
    check(FullInstallWindow::message(1, 400000).startsWith(QLatin1String("Installing 1 add-on (")), QStringLiteral("... in the singular for one add-on"),
          FullInstallWindow::message(1, 400000));
    FullInstallWindow win(13, 159000000);
    win.show();
    settle(300);
    auto *logo = win.findChild<QLabel *>(QStringLiteral("fullInstallLogo"));
    check(logo && logo->pixmap().width() >= 128 && win.findChild<QProgressBar *>(QStringLiteral("fullInstallBar")), QStringLiteral("the window has the big logo and a progress bar"));
    win.setProgress(4, 13, QStringLiteral("media-playback.qbpkg"), 60000000);
    check(win.progressMaximum() == 159000 && win.progressValue() == 60000 && win.currentText().contains(QLatin1String("5 of 13"))
              && win.currentText().contains(QLatin1String("media-playback")),
          QStringLiteral("the bar and the line under it follow the packages"), win.currentText());
    shot(&win, QStringLiteral("full-install-window"));
    win.hide();

    // the folder's packages: due only once per version, progress in order
    const QString pkgs = work + QLatin1String("/full-addons");
    QDir().mkpath(pkgs);
    for (const char *id : {"check-v0940b-one", "check-v0940b-two", "check-v0940b-three"}) {
        const QString p = makeAddon(work, QLatin1String(id));
        QFile::copy(p, pkgs + QLatin1Char('/') + QFileInfo(p).fileName());
    }
    int count = 0;
    qint64 bytes = 0;
    check(addons::installBundledDue(pkgs, &count, &bytes) && count == 3 && bytes > 100, QStringLiteral("the folder holds 3 packages still to install"), QString::number(count));
    QStringList order;
    QStringList log;
    QString err;
    const bool ok = addons::installBundled(pkgs, &log, &err, [&](int done, int total, const QString &name, qint64) {
        order << QStringLiteral("%1/%2 %3").arg(done).arg(total).arg(name);
    });
    check(ok && order.size() == 4 && order.first().startsWith(QLatin1String("0/3")) && order.last() == QLatin1String("3/3 "),
          QStringLiteral("installBundled reports the progress, package by package"), order.join(QLatin1String(" | ")) + err);
    check(!addons::installBundledDue(pkgs), QStringLiteral("the marker is written: nothing is due again for this version"));
    for (const char *id : {"check-v0940b-one", "check-v0940b-two", "check-v0940b-three"})
        QDir(addons::addonsRoot() + QLatin1Char('/') + QLatin1String(id)).removeRecursively();
    QFile::remove(dataDir() + QStringLiteral("/full-addons-installed-" QB_APP_VERSION));

    // the program's own call (--install-folder), with the window and without a display
#ifdef Q_OS_WIN
    const QString program = QStringLiteral(QB_TEST_BUILD_DIR "/quillbinder.exe");
#else
    const QString program = QStringLiteral(QB_TEST_BUILD_DIR "/quillbinder");
#endif
    if (QFile::exists(program)) {
        for (const bool display : {true, false}) {
            QProcess p;
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("XDG_DATA_HOME"), qEnvironmentVariable("XDG_DATA_HOME"));
            env.insert(QStringLiteral("XDG_CONFIG_HOME"), qEnvironmentVariable("XDG_CONFIG_HOME"));
            env.insert(QStringLiteral("HOME"), qEnvironmentVariable("HOME"));
            env.insert(QStringLiteral("QB_NO_UPDATE_CHECK"), QStringLiteral("1"));
            env.remove(QStringLiteral("QB_NO_DIALOGS"));
            if (!display) {
                env.remove(QStringLiteral("DISPLAY"));
                env.remove(QStringLiteral("WAYLAND_DISPLAY"));
            } else {
                env.insert(QStringLiteral("QT_QPA_PLATFORM"), qEnvironmentVariable("QT_QPA_PLATFORM", QStringLiteral("xcb")));
            }
            p.setProcessEnvironment(env);
            p.setProcessChannelMode(QProcess::MergedChannels);
            p.start(program, {QStringLiteral("--install-folder"), pkgs});
            const bool done = p.waitForFinished(120000);
            const QString out = QString::fromUtf8(p.readAll());
            const bool all = QFileInfo::exists(addons::addonsRoot() + QLatin1String("/check-v0940b-one"))
                             && QFileInfo::exists(addons::addonsRoot() + QLatin1String("/check-v0940b-three"));
            check(done && p.exitCode() == 0 && all && QFile::exists(dataDir() + QStringLiteral("/full-addons-installed-" QB_APP_VERSION)),
                  QStringLiteral("quillbinder --install-folder installs the folder's packages and writes the marker (%1)")
                      .arg(display ? QStringLiteral("with the window") : QStringLiteral("without a display")),
                  out.right(800));
            // again: nothing happens (the marker), a removed add-on stays removed
            QDir(addons::addonsRoot() + QLatin1String("/check-v0940b-two")).removeRecursively();
            QProcess again;
            again.setProcessEnvironment(env);
            again.start(program, {QStringLiteral("--install-folder"), pkgs});
            again.waitForFinished(60000);
            check(again.exitCode() == 0 && !QFileInfo::exists(addons::addonsRoot() + QLatin1String("/check-v0940b-two")),
                  QStringLiteral("... a second call installs nothing again"));
            for (const char *id : {"check-v0940b-one", "check-v0940b-two", "check-v0940b-three"})
                QDir(addons::addonsRoot() + QLatin1Char('/') + QLatin1String(id)).removeRecursively();
            QFile::remove(dataDir() + QStringLiteral("/full-addons-installed-" QB_APP_VERSION));
        }
        // a broken package: the call fails and writes no marker
        writeFile(pkgs + QLatin1String("/broken.qbpkg"), "not a package");
        QProcess bad;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("QB_NO_DIALOGS"), QStringLiteral("1"));
        bad.setProcessEnvironment(env);
        bad.start(program, {QStringLiteral("--install-folder"), pkgs});
        bad.waitForFinished(60000);
        check(bad.exitCode() != 0 && !QFile::exists(dataDir() + QStringLiteral("/full-addons-installed-" QB_APP_VERSION)),
              QStringLiteral("a package that fails: the call fails and no marker is written"));
        for (const char *id : {"check-v0940b-one", "check-v0940b-two", "check-v0940b-three"})
            QDir(addons::addonsRoot() + QLatin1Char('/') + QLatin1String(id)).removeRecursively();
    } else {
        check(false, QStringLiteral("(the program file is needed for the call: %1)").arg(program));
    }

    // the start script of the full AppImage calls the program once for the folder
    const QString script = fileText(QStringLiteral(QB_TEST_SOURCE_DIR "/packaging/linux/build-full-appimage.sh"));
    check(script.contains(QLatin1String("--install-folder")) && !script.contains(QLatin1String("for p in ")) && script.contains(QLatin1String("MARK")),
          QStringLiteral("the start script makes one call for the folder and keeps the marker file"));
    // the download page names the space of the add-ons when the release gives the number (and says nothing without it)
    {
        QTemporaryDir names, siteWith, siteWithout;
        const QString full = QStringLiteral("Quillbinder-9.9.9-full-test-x86_64.AppImage");
        writeFile(names.path() + QLatin1Char('/') + full, QByteArray());
        auto run = [&](const QString &site) {
            QProcess pr;
            pr.setProcessChannelMode(QProcess::MergedChannels);
            pr.start(python(), {QStringLiteral("-I"), QStringLiteral(QB_TEST_SOURCE_DIR "/tools/download-page.py"), names.path(),
                                QStringLiteral("v9.9.9-test"), site});
            pr.waitForFinished(60000);
            return pr.exitCode() == 0 ? fileText(site + QLatin1String("/index.html")) : QString::fromUtf8(pr.readAll());
        };
        const QString without = run(siteWithout.path());
        writeFile(names.path() + QLatin1String("/full-addons-size"), "159123456\n");
        const QString with = run(siteWith.path());
        check(with.contains(QLatin1String("about 159 MB of disk space")) && with.contains(full) && !without.contains(QLatin1String("disk space"))
                  && without.contains(full) && !with.contains(QLatin1String("full-addons-size")),
              QStringLiteral("the download page names the space of the add-ons beside the Linux button when the release gives it, and says nothing without it"),
              without.left(300));
    }
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
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0940bTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0940bTest"));
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

    menus(w);
    findCommand(w, p);
    appMenu(w, homeDir);
    firstStart(w, homeDir);
    fullInstall(work.path());

    std::printf("%d failure(s)\n", failures);
    return failures;
}
