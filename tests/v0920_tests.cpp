// Scripted run for version 0.92.0.
// Part 1, the fixes:
//  - a file whose time alone changed (a sync program sets it after its upload) is no other version: nothing is kept,
//    nothing is told and nothing is written for it; a real change elsewhere is still kept and told; the notice can be
//    turned off until the program quits;
//  - a project file that holds what a save would write is not written again;
//  - backups carry the project's ID in their name, so two projects of one title keep their own backups;
//  - a file that cannot be copied at an import is told, and a re-read of a Scrivener project that cannot carry a file
//    over does not open the project;
//  - the plain reason of a failed save follows the failure's kind and not the words of the system's error text;
//  - a manual save whose backup fails shows a notice;
//  - the add-on and program-version states say when they could not be written, and the way back of a failed
//    install says what was left where.
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
// Part 2, the new functions:
//  - the fixed mind map: a pin on the binder's rows (the fixed row, and the row under the mouse), the pin button of
//    the inspector's Mind Map tab, the bar at the binder's bottom with "Release", the two menu commands; kept per
//    project, released when the item goes into the Trash;
//  - the shown lock: each device writes which documents it has open or has changed beside the project's lock, only
//    when that list changes; the binder marks what other devices work on with a padlock (documents and the folders
//    around them), with a tool tip; a list without a sign of life is not shown; the list goes at closing.
//
//   xvfb-run -a ./qb_v0920_tests /tmp/shots

#include "core/Addons.h"
#include "core/Backup.h"
#include "core/BinderItem.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/FailureKind.h"
#include "core/Launcher.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/Snapshots.h"
#include "core/SyncSafety.h"
#include "core/WorkMarks.h"
#include "core/Zip.h"
#include "compile/PluginManager.h"
#include "import/FileImporter.h"
#include "import/ScrivenerFormat.h"
#include "import/ScrivenerImporter.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/MindMapDoc.h"
#include "ui/MindMapUi.h"
#include "ui/Theme.h"

#include "AppInfo.h"

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QColor>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QHelpEvent>
#include <QIcon>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QStatusBar>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QTranslator>

#include <cerrno>
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
            std::printf("      %s\n", qPrintable(detail.left(3000)));
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

static bool waitFor(const std::function<bool()> &f, int ms)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        if (f())
            return true;
        settle(50);
    }
    return f();
}

static void shot(QWidget *w, const QString &name)
{
    if (w)
        w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static QMessageBox *visibleBox(const QString &name)
{
    for (QWidget *tl : QApplication::topLevelWidgets())
        if (auto *b = qobject_cast<QMessageBox *>(tl))
            if (b->isVisible() && b->objectName() == name)
                return b;
    return nullptr;
}

static QByteArray readAll(const QString &f)
{
    QFile file(f);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

static void writeAll(const QString &f, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(f).absolutePath());
    QFile file(f);
    if (file.open(QIODevice::WriteOnly))
        file.write(data);
}

static BinderItem *textDoc(Project *p, BinderItem *parent, const QString &title, const QString &text)
{
    BinderItem *it = p->model()->createItem(parent, -1, ItemType::Text, title);
    QTextCursor c(p->docs()->text(it->id));
    c.insertText(text);
    return it;
}

static void typeInto(Project *p, const BinderItem *it, const QString &more)
{
    QTextCursor c(p->docs()->text(it->id));
    c.movePosition(QTextCursor::End);
    c.insertText(more);
}

// What a sync program does after its upload: the file's time moves, its bytes stay.
static bool setTimeLater(const QString &path, int seconds)
{
    const QDateTime now = QFileInfo(path).lastModified();
    QFile f(path);
    if (!f.open(QIODevice::ReadWrite | QIODevice::ExistingOnly))
        return false;
    // (a whole second, as the measured sync program sets it)
    const bool ok = f.setFileTime(QDateTime::fromSecsSinceEpoch(now.toSecsSinceEpoch() + seconds), QFileDevice::FileModificationTime);
    f.close();
    return ok && QFileInfo(path).lastModified() != now;
}

static QStringList filesIn(const QString &dir)
{
    QStringList out;
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        out << QDir(dir).relativeFilePath(it.next());
    out.sort();
    return out;
}

static QByteArray pngBytes(const QColor &c)
{
    QImage img(24, 16, QImage::Format_RGB32);
    img.fill(c);
    QByteArray out;
    QBuffer b(&out);
    b.open(QIODevice::WriteOnly);
    img.save(&b, "PNG");
    return out;
}

// A small add-on package, built as tools/make-package.py builds one.
static QString addonPackage(const QString &dir, const QString &id, const QString &version, const QByteArray &theme)
{
    const QString path = dir + QLatin1Char('/') + id + QLatin1Char('-') + version + QLatin1String(".qbpkg");
    QJsonObject file;
    file.insert(QStringLiteral("path"), QStringLiteral("themes/test.json"));
    file.insert(QStringLiteral("size"), theme.size());
    file.insert(QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(theme, QCryptographicHash::Sha256).toHex()));
    QJsonObject m;
    m.insert(QStringLiteral("format"), 1);
    m.insert(QStringLiteral("id"), id);
    m.insert(QStringLiteral("kind"), QStringLiteral("addon"));
    m.insert(QStringLiteral("version"), version);
    m.insert(QStringLiteral("name"), QStringLiteral("Way Back Test"));
    m.insert(QStringLiteral("files"), QJsonArray{file});
    ZipWriter zw(path);
    zw.addFile(QStringLiteral("manifest.json"), QJsonDocument(m).toJson());
    zw.addFile(QStringLiteral("payload/themes/test.json"), theme);
    zw.close();
    return path;
}

static bool addonEnabled(const QString &id, bool *found = nullptr)
{
    for (const addons::Installed &a : addons::installedAddons())
        if (a.manifest.id == id) {
            if (found)
                *found = true;
            return a.enabled;
        }
    if (found)
        *found = false;
    return false;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_WORK_MARKS_MS", "300");   // the shown lock: its list is written 0.3 s after a change (5 s in the program)
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0920Test"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0920Test"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);

    check(Project::compareVersionText(QLatin1String(QB_APP_VERSION), QStringLiteral("0.92.0")) >= 0,
          QStringLiteral("version 0.92.0 or newer"), QStringLiteral(QB_APP_VERSION_SHOWN));
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    as.save();
    theme::apply();
    compile::PluginManager::instance().load();
    templates::Info blank;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("blank"))
            blank = t;

    QTemporaryDir work;
    const QString W = work.path();
    QString err;

    // ================================================================ the project and the window
    auto proj = templates::createProject(blank, W, QStringLiteral("Sync"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    Project *p = proj.get();
    const QString projFolder = p->folderPath();
    const QString indexFile = p->indexPath();
    const QString conflicts = projFolder + QStringLiteral("/settings/conflicts");
    BinderItem *synced = textDoc(p, p->draft(), QStringLiteral("Synced"), QStringLiteral("First text."));
    BinderItem *other = textDoc(p, p->draft(), QStringLiteral("Other"), QStringLiteral("Other text."));
    check(p->save(&err), QStringLiteral("(saved)"), err);

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.move(0, 0);
    w.resize(1300, 850);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(800);
    check(p->save(&err), QStringLiteral("(saved in the window)"), err);
    int told = 0;
    QObject::connect(p, &Project::conflictKept, &w, [&told] { ++told; });

    // ================================================================ a text whose time alone changed (to-do item 2)
    {
        const QString path = p->docs()->textPath(synced->id);
        const QByteArray before = readAll(path);
        const bool moved = setTimeLater(path, 5);
        typeInto(p, synced, QStringLiteral(" Written after the sync program touched the file."));
        const bool saved = p->save(&err);
        settle(400);
        check(moved && saved && readAll(path) != before && readAll(path).contains("after the sync program")
                  && filesIn(conflicts).isEmpty() && snapshots::list(p, synced->id).isEmpty() && told == 0
                  && !visibleBox(QStringLiteral("conflictNotice")),
              QStringLiteral("a text whose time alone was changed elsewhere is saved as usual: no copy, no snapshot, no notice"),
              QStringLiteral("moved %1, saved %2 (%3), copies: %4, snapshots %5, told %6")
                  .arg(moved).arg(saved).arg(err, filesIn(conflicts).join(QLatin1String(", ")))
                  .arg(snapshots::list(p, synced->id).size()).arg(told));
        // the same again, and again: the pattern of a sync program that touches every file this program writes
        bool calm = true;
        for (int round = 0; round < 3; ++round) {
            calm = setTimeLater(path, 7 + round) && calm;
            typeInto(p, synced, QStringLiteral(" Round %1.").arg(round));
            calm = p->save(&err) && calm;
            settle(150);
        }
        check(calm && filesIn(conflicts).isEmpty() && snapshots::list(p, synced->id).isEmpty() && told == 0
                  && !visibleBox(QStringLiteral("conflictNotice")),
              QStringLiteral("three more rounds of write here, time changed there: still nothing kept and nothing told"),
              filesIn(conflicts).join(QLatin1String(", ")));
    }

    // ================================================================ the project file whose time alone changed
    {
        const bool moved = setTimeLater(indexFile, 5);
        BinderItem *fresh = textDoc(p, p->draft(), QStringLiteral("Made after the touch"), QStringLiteral("New."));
        const bool saved = p->save(&err);
        settle(400);
        check(moved && saved && fresh && readAll(indexFile).contains("Made after the touch") && filesIn(conflicts).isEmpty() && told == 0
                  && !visibleBox(QStringLiteral("conflictNotice")),
              QStringLiteral("the project file whose time alone was changed elsewhere is written as usual: no copy, no notice"),
              QStringLiteral("moved %1, saved %2 (%3), copies: %4").arg(moved).arg(saved).arg(err, filesIn(conflicts).join(QLatin1String(", "))));
    }

    // ================================================================ no round of save, sync, changed time, save
    {
        // a save that changes nothing in the project file leaves the file alone, whatever its time says
        const QByteArray bytes = readAll(indexFile);
        bool same = true;
        QStringList times;
        for (int round = 0; round < 3; ++round) {
            const bool moved = setTimeLater(indexFile, 9 + round);
            const QDateTime touched = QFileInfo(indexFile).lastModified();
            p->markIndexDirty();
            const bool saved = p->save(&err);
            settle(100);
            same = same && moved && saved && QFileInfo(indexFile).lastModified() == touched && readAll(indexFile) == bytes;
            times << QFileInfo(indexFile).lastModified().toString(Qt::ISODateWithMs);
        }
        check(same && filesIn(conflicts).isEmpty() && told == 0,
              QStringLiteral("a project file that holds what the save would write is not written again (no round of save, "
                             "sync, changed time, save)"),
              times.join(QLatin1String(" | ")) + QLatin1Char(' ') + err);
        // and a text that was not changed here is not written either
        const QString path = p->docs()->textPath(other->id);
        const bool moved = setTimeLater(path, 5);
        const QDateTime touched = QFileInfo(path).lastModified();
        typeInto(p, other, QStringLiteral("x"));
        QTextCursor c(p->docs()->text(other->id));
        c.movePosition(QTextCursor::End);
        c.deletePreviousChar();
        const bool saved = p->save(&err);
        check(moved && saved && QFileInfo(path).lastModified() == touched && filesIn(conflicts).isEmpty() && told == 0,
              QStringLiteral("a text that is as it was saved is not written again after its time changed"), err);
    }

    // ================================================================ the measured pattern, with the program idle
    {
        // after an upload every file of the project has a later time; the saves that follow write no file at all
        auto stamps = [&projFolder] {
            QStringList out;
            for (const QString &f : filesIn(projFolder)) {
                const QFileInfo fi(projFolder + QLatin1Char('/') + f);
                if (!f.startsWith(QLatin1String("settings/lock")))
                    out << QStringLiteral("%1 %2 %3").arg(f).arg(fi.size()).arg(fi.lastModified().toMSecsSinceEpoch());
            }
            return out;
        };
        bool moved = true;
        for (const QString &f : filesIn(projFolder))
            if (!f.startsWith(QLatin1String("settings/lock")))
                moved = setTimeLater(projFolder + QLatin1Char('/') + f, 3) && moved;
        const QStringList before = stamps();
        bool saved = true;
        for (int round = 0; round < 3; ++round) {
            p->markIndexDirty();
            saved = p->save(&err) && saved;
            settle(300);
        }
        const QStringList after = stamps();
        QStringList differ;
        for (const QString &line : after)
            if (!before.contains(line))
                differ << line;
        check(moved && saved && after == before && told == 0 && !visibleBox(QStringLiteral("conflictNotice")),
              QStringLiteral("every file's time changed elsewhere, the program idle: three saves write no file, keep nothing, tell nothing"),
              differ.join(QLatin1String(" | ")) + QLatin1Char(' ') + err);
    }

    // ================================================================ a real change elsewhere is still kept and told
    auto changeElsewhere = [&](const QByteArray &theirs, const QString &ours) {
        const QString path = p->docs()->textPath(synced->id);
        QTest::qWait(1100);
        writeAll(path, theirs);
        typeInto(p, synced, ours);
        const bool saved = p->save(&err);
        settle(500);
        return saved;
    };
    {
        const bool saved = changeElsewhere("Changed on the other computer.\n", QStringLiteral(" Changed here too."));
        const QStringList copies = filesIn(conflicts);
        QPointer<QMessageBox> notice = visibleBox(QStringLiteral("conflictNotice"));
        auto *quiet = notice ? notice->findChild<QPushButton *>(QStringLiteral("conflictNoticeOff")) : nullptr;
        check(saved && copies.size() == 1 && readAll(conflicts + QLatin1Char('/') + copies.value(0)) == "Changed on the other computer.\n"
                  && snapshots::list(p, synced->id).size() == 1 && told == 1 && notice
                  && readAll(p->docs()->textPath(synced->id)).contains("Changed here too."),
              QStringLiteral("a text really changed elsewhere and here: the other version is kept (copy and snapshot) and told, as before"),
              QStringLiteral("saved %1 (%2), copies: %3, snapshots %4, told %5, notice %6")
                  .arg(saved).arg(err, copies.join(QLatin1String(", "))).arg(snapshots::list(p, synced->id).size()).arg(told).arg(notice != nullptr));
        check(quiet && quiet->text() == QStringLiteral("Don't Show Again Until %1 Is Closed").arg(QStringLiteral(QB_APP_NAME))
                  && !quiet->toolTip().isEmpty() && !MainWindow::conflictNoticeOff() && notice && !notice->isModal(),
              QStringLiteral("the notice has the button \"Don't Show Again Until Quillbinder Is Closed\""),
              quiet ? quiet->text() : QStringLiteral("no button"));
        shot(notice, QStringLiteral("v0920-conflict-notice"));
        if (quiet)
            quiet->click();
        settle(300);
        check(MainWindow::conflictNoticeOff() && !visibleBox(QStringLiteral("conflictNotice")),
              QStringLiteral("the button closes the notice and turns it off"));
        // the next other version: kept, not told in a window
        const bool saved2 = changeElsewhere("Changed there a second time.\n", QStringLiteral(" And here again."));
        check(saved2 && filesIn(conflicts).size() == 2 && snapshots::list(p, synced->id).size() == 2 && told == 2
                  && !visibleBox(QStringLiteral("conflictNotice")),
              QStringLiteral("turned off: the next other version is still kept (copy and snapshot), and no notice comes"),
              filesIn(conflicts).join(QLatin1String(", ")));
        // the program started again: the notice is back
        MainWindow::setConflictNoticeOff(false);
        const bool saved3 = changeElsewhere("Changed there a third time.\n", QStringLiteral(" And a third time here."));
        QPointer<QMessageBox> again = visibleBox(QStringLiteral("conflictNotice"));
        check(saved3 && filesIn(conflicts).size() == 3 && again,
              QStringLiteral("not kept beyond the program's run: after a new start the notice comes again"));
        if (again)
            again->accept();
        settle(200);
    }

    // ================================================================ a Scrivener project whose binder's time alone changed
    {
        auto sp = templates::createProject(blank, W, QStringLiteral("Harbor"), &err);
        BinderItem *scene = sp ? textDoc(sp.get(), sp->draft(), QStringLiteral("Scene"), QStringLiteral("The tide came in.")) : nullptr;
        const QString scriv = W + QStringLiteral("/Harbor.scriv");
        std::unique_ptr<Project> wc = sp && sp->save(&err) ? scrivener::convertToScrivener(sp.get(), scriv, &err) : nullptr;
        check(wc != nullptr && scene, QStringLiteral("(a project in Scrivener format)"), err);
        if (wc) {
            int binderTold = 0;
            QObject::connect(wc.get(), &Project::conflictKept, &app, [&binderTold] { ++binderTold; });
            const QString scrivx = scrivener::scrivxIn(scriv);
            const QString wcConflicts = wc->settingsDir() + QStringLiteral("/conflicts");
            bool ok = wc->save(&err);
            const bool moved = setTimeLater(scrivx, 5);
            textDoc(wc.get(), wc->draft(), QStringLiteral("Second scene"), QStringLiteral("The tide went out."));
            ok = wc->save(&err) && ok;
            check(ok && moved && binderTold == 0 && filesIn(wcConflicts).isEmpty() && readAll(scrivx).contains("Second scene"),
                  QStringLiteral("Scrivener format: the binder (.scrivx) whose time alone changed is written as usual, nothing kept or told"),
                  QStringLiteral("told %1, copies: %2 %3").arg(binderTold).arg(filesIn(wcConflicts).join(QLatin1String(", ")), err));
            // other bytes: kept and told as before
            QTest::qWait(1100);
            QByteArray theirs = readAll(scrivx);
            theirs.append("\n<!-- written elsewhere -->\n");
            writeAll(scrivx, theirs);
            textDoc(wc.get(), wc->draft(), QStringLiteral("Third scene"), QStringLiteral("Gulls."));
            ok = wc->save(&err);
            check(ok && binderTold == 1 && filesIn(wcConflicts).size() == 1,
                  QStringLiteral("Scrivener format: a binder really written elsewhere is still kept and told"),
                  QStringLiteral("told %1, copies: %2 %3").arg(binderTold).arg(filesIn(wcConflicts).join(QLatin1String(", ")), err));
        }
    }

    // ================================================================ the project's ID in the names of its backups (item 9)
    {
        using namespace backup;
        const QString id = idFromSeed(QStringLiteral("a-seed"));
        static const QRegularExpression form(QStringLiteral("^[2-9A-HJKMNP-Z]{5}$"));
        check(form.match(id).hasMatch() && id == idFromSeed(QStringLiteral("a-seed")) && id != idFromSeed(QStringLiteral("b-seed")),
              QStringLiteral("a project ID: five characters (no 0, O, 1, I, L), the same for the same project, another for another"), id);
        auto a = templates::createProject(blank, W + QStringLiteral("/one"), QStringLiteral("Novel"), &err);
        auto b = templates::createProject(blank, W + QStringLiteral("/two"), QStringLiteral("Novel"), &err);
        check(a && b && a->save(&err) && b->save(&err), QStringLiteral("(two projects of one title)"), err);
        if (a && b) {
            const QString idA = projectId(a.get()), idB = projectId(b.get());
            const QString folderA = a->folderPath();
            a.reset();
            a = Project::open(folderA, &err);
            check(a && idA != idB && projectId(a.get()) == idA && nameStart(a.get()) == QStringLiteral("Novel #") + idA,
                  QStringLiteral("two projects of one title have different IDs; a project keeps its ID when it is opened again"),
                  idA + QLatin1Char(' ') + idB);
            const QString dir = W + QStringLiteral("/backups");
            QDir().mkpath(dir);
            // a backup of a version before (no ID in its name)
            const QString older = dir + QStringLiteral("/Novel bak 2026-01-01 10-00-00.zip");
            writeAll(older, "an older backup");
            QString made, why;
            bool ok = a != nullptr;
            for (int i = 0; ok && i < 3; ++i)
                ok = backupProject(a.get(), dir, 2, &made, &why);
            const bool named = QFileInfo(made).fileName().startsWith(QStringLiteral("Novel #") + idA + QStringLiteral(" bak "))
                               && isOwnBackupName(QFileInfo(made).fileName());
            ok = ok && backupProject(b.get(), dir, 2, &made, &why);
            auto with = [&dir](const QString &projectId) {
                return QDir(dir).entryList({QStringLiteral("Novel #") + projectId + QStringLiteral(" bak *.zip")}, QDir::Files);
            };
            check(ok && named && with(idA).size() == 2 && with(idB).size() == 1,
                  QStringLiteral("backups are named \"<title> #<ID> bak <date>.zip\"; the clean-up of one project (keep 2) leaves the "
                                 "other project's backup alone"),
                  why + QLatin1Char(' ') + QDir(dir).entryList(QDir::Files).join(QLatin1String(" | ")));
            for (int i = 0; ok && i < 2; ++i)
                ok = backupProject(b.get(), dir, 2, &made, &why);
            check(ok && with(idA).size() == 2 && with(idB).size() == 2 && QFile::exists(older),
                  QStringLiteral("each project keeps its own two; a backup without an ID (made by a version before) is not deleted"),
                  QDir(dir).entryList(QDir::Files).join(QLatin1String(" | ")));
            const QStringList offered = a ? backupsFor(a.get(), dir) : QStringList();
            check(offered.contains(older) && offered.size() == 5,
                  QStringLiteral("Restore From Backup offers them all: this project's, the other project's of the same title, "
                                 "and the one without an ID"),
                  offered.join(QLatin1String(" | ")));

            // copies the program makes itself are projects of their own (they carry their original's Draft)
            QString cErr;
            for (const char *sub : {"/t1", "/t2", "/copies", "/restored"})
                QDir().mkpath(W + QLatin1String(sub));
            const bool templ = a && templates::saveAsTemplate(a.get(), QStringLiteral("ID test template"), QStringLiteral("My Templates"),
                                                              QStringLiteral("for the ID"), &cErr);
            templates::Info mine;
            for (const auto &t : templates::all())
                if (t.name == QLatin1String("ID test template"))
                    mine = t;
            auto fromT1 = templates::createProject(mine, W + QStringLiteral("/t1"), QStringLiteral("Novel"), &cErr);
            auto fromT2 = templates::createProject(mine, W + QStringLiteral("/t2"), QStringLiteral("Novel"), &cErr);
            auto copy = a ? scrivener::copyAsQuillbinder(a.get(), W + QStringLiteral("/copies"), QStringLiteral("Novel"), &cErr) : nullptr;
            QString restoredFile;
            restore(dir + QLatin1Char('/') + with(idA).value(0), W + QStringLiteral("/restored"), &restoredFile, &cErr);
            auto restored = restoredFile.isEmpty() ? nullptr : Project::open(restoredFile, &cErr);
            QSet<QString> ids{idA, idB};
            for (Project *x : {fromT1.get(), fromT2.get(), copy.get(), restored.get()})
                if (x)
                    ids.insert(projectId(x));
            check(templ && fromT1 && fromT2 && copy && restored && ids.size() == 6
                      && fromT1->draft()->id == a->draft()->id && copy->draft()->id == a->draft()->id,
                  QStringLiteral("a project made from a template file, a copy saved as a project and a project restored from a "
                                 "backup each get an ID of their own"),
                  cErr + QStringLiteral(" ids: ") + QStringList(ids.cbegin(), ids.cend()).join(QLatin1Char(' ')));
            if (copy) {
                const QString copyId = projectId(copy.get()), copyFolder = copy->folderPath();
                copy.reset();
                copy = Project::open(copyFolder, &cErr);
                check(copy && projectId(copy.get()) == copyId, QStringLiteral("such an ID stays when the project is opened again"), cErr);
            }
        }
    }

    // ================================================================ a file that cannot be copied at an import (item 8)
    {
        const QString pic = W + QStringLiteral("/harbor-picture.png");
        writeAll(pic, pngBytes(QColor(20, 110, 150)));
        const int before = int(p->research()->children.size());
        QStringList warnings;
        qputenv("QB_TEST_FAIL_COPY", "harbor-picture");
        const QList<BinderItem *> none = importer::importFiles(p, {pic}, p->research(), -1, &warnings);
        qunsetenv("QB_TEST_FAIL_COPY");
        check(none.isEmpty() && int(p->research()->children.size()) == before && warnings.size() == 1
                  && warnings.first().contains(QLatin1String("harbor-picture")),
              QStringLiteral("File > Import: a picture that cannot be copied into the project is told and leaves no empty item behind"),
              warnings.join(QLatin1String(" | ")));
        warnings.clear();
        const QList<BinderItem *> one = importer::importFiles(p, {pic}, p->research(), -1, &warnings);
        check(one.size() == 1 && warnings.isEmpty() && QFile::exists(p->mediaFilePath(one.value(0))),
              QStringLiteral("(the same picture is imported when the copy works)"), warnings.join(QLatin1String(" | ")));
        check(p->save(&err), QStringLiteral("(saved)"), err);

        // a Scrivener project with that picture, imported while the picture cannot be copied
        const QString scriv = W + QStringLiteral("/Pictures.scriv");
        auto sp = templates::createProject(blank, W, QStringLiteral("Pictures"), &err);
        if (sp) {
            textDoc(sp.get(), sp->draft(), QStringLiteral("Scene"), QStringLiteral("Text."));
            importer::importFiles(sp.get(), {pic}, sp->research(), -1, &warnings);
        }
        std::unique_ptr<Project> wc = sp && sp->save(&err) ? scrivener::convertToScrivener(sp.get(), scriv, &err) : nullptr;
        check(wc && wc->save(&err), QStringLiteral("(a Scrivener project with a picture)"), err);
        if (wc) {
            ScrivenerImportReport full, part;
            auto whole = ScrivenerImporter::importProject(scriv, W + QStringLiteral("/import-whole"), QStringLiteral("Whole"),
                                                          StorageFormat::Markdown, &full, &err);
            qputenv("QB_TEST_FAIL_COPY", "/Files/Data/");
            auto lacking = ScrivenerImporter::importProject(scriv, W + QStringLiteral("/import-part"), QStringLiteral("Part"),
                                                            StorageFormat::Markdown, &part, &err);
            qunsetenv("QB_TEST_FAIL_COPY");
            bool named = false;
            for (const QString &warn : std::as_const(part.warnings))
                named = named || (warn.contains(QLatin1String("harbor-picture")) && warn.contains(QLatin1String("could not be copied")));
            check(whole && lacking && full.mediaFiles == 1 && part.mediaFiles == 0 && named,
                  QStringLiteral("Scrivener import: a file that cannot be copied is told by name and not counted as imported"),
                  QStringLiteral("whole: %1 file(s); lacking: %2 file(s), warnings: %3")
                      .arg(full.mediaFiles).arg(part.mediaFiles).arg(part.warnings.join(QLatin1String(" | "))));

            // read again after a change elsewhere, while a file only the working copy holds cannot be carried over:
            // the project does not open and the working copy stays as it is
            const QString wcFolder = wc->folderPath();
            writeAll(wcFolder + QStringLiteral("/objects/only-here.json"), "{\"kept\": true}");
            check(wc->save(&err), QStringLiteral("(working copy saved)"), err);
            wc.reset();
            // (the spare copies of the project file are made anew whenever a project is opened: not compared)
            auto wcFiles = [&wcFolder] {
                QStringList out;
                for (const QString &f : filesIn(wcFolder))
                    if (!f.startsWith(QLatin1String("settings/spare-copy")))
                        out << f + QLatin1Char(':') + QString::fromLatin1(QCryptographicHash::hash(readAll(wcFolder + QLatin1Char('/') + f), QCryptographicHash::Sha1).toHex().left(8));
                return out;
            };
            const QStringList wcBefore = wcFiles();
            QTest::qWait(1100);
            QString changed;
            QDirIterator it(scriv + QStringLiteral("/Files/Data"), {QStringLiteral("content.rtf")}, QDir::Files, QDirIterator::Subdirectories);
            if (it.hasNext())
                changed = it.next();
            writeAll(changed, "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Arial;}}\\f0\\fs24 Changed in Scrivener.\\par}");
            qputenv("QB_TEST_FAIL_COPY", "only-here");
            QString openErr;
            scrivener::OpenResult refused = scrivener::open(scriv, &openErr);
            qunsetenv("QB_TEST_FAIL_COPY");
            check(!changed.isEmpty() && !refused.project && openErr.contains(QLatin1String("only-here.json"))
                      && openErr.contains(QLatin1String("was not opened")) && wcFiles() == wcBefore
                      && !QDir(QFileInfo(wcFolder).absolutePath() + QStringLiteral("/.reading")).exists(),
                  QStringLiteral("a Scrivener project read again: when a file of the working copy cannot be carried over, the "
                                 "project does not open, the message names the file, and the working copy is as it was"),
                  openErr + QStringLiteral(" || before: ") + wcBefore.join(QLatin1String(", ")) + QStringLiteral(" || after: ")
                      + wcFiles().join(QLatin1String(", ")) + QStringLiteral(" || in the folder: ")
                      + QDir(QFileInfo(wcFolder).absolutePath()).entryList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot).join(QLatin1String(", ")));
            scrivener::OpenResult opened = scrivener::open(scriv, &openErr);
            check(opened.project && opened.reread
                      && readAll(opened.project->folderPath() + QStringLiteral("/objects/only-here.json")) == "{\"kept\": true}",
                  QStringLiteral("opened again when the copy works: read again, with the working copy's own file"), openErr);
        }
    }

    // ================================================================ the fixed mind map (to-do item 1)
    {
        auto *binder = w.findChild<BinderPanel *>();
        auto *insp = w.findChild<Inspector *>();
        BinderTree *tree = binder ? binder->tree() : nullptr;
        QAction *fixAct = w.findChild<QAction *>(QStringLiteral("fixMindMap"));
        QAction *releaseAct = w.findChild<QAction *>(QStringLiteral("releaseMindMap"));
        QAction *showInspector = w.findChild<QAction *>(QStringLiteral("showInspector"));
        check(binder && insp && tree && fixAct && releaseAct && showInspector, QStringLiteral("(binder, inspector and the two commands found)"));
        if (binder && insp && tree && fixAct && releaseAct && showInspector) {
            if (!showInspector->isChecked())
                showInspector->trigger();
            insp->showTab(Inspector::MindMap);
            MindMapPanel *panel = insp->mindMapPanel();
            auto *bar = binder->findChild<QWidget *>(QStringLiteral("binderFixedMindMapBar"));
            auto *barTitle = binder->findChild<QLabel *>(QStringLiteral("binderFixedMindMapTitle"));
            auto *barRelease = binder->findChild<QToolButton *>(QStringLiteral("binderReleaseMindMap"));
            auto rowOf = [&](const BinderItem *it) { return p->model()->indexFor(it); };
            auto selectOne = [&](BinderItem *it) {
                binder->select({it}, true);
                settle(400);
            };
            tree->expandAll();
            selectOne(synced);
            check(bar && barTitle && barRelease && !bar->isVisible() && panel->itemId() == synced->id && !panel->isFixed()
                      && w.fixedMindMapId().isEmpty() && fixAct->isEnabled() && !releaseAct->isEnabled(),
                  QStringLiteral("nothing fixed: the Mind Map tab shows the selected item's board, the binder has no bar, "
                                 "\"Fix\" can be used and \"Release\" cannot"));

            // the pin: only on the row under the mouse; a click fixes without selecting
            const QModelIndex otherRow = rowOf(other);
            tree->scrollTo(otherRow);
            settle(100);
            QTest::mouseMove(tree->viewport(), tree->visualRect(otherRow).center());
            settle(200);
            const bool hovered = tree->hoverIndex() == otherRow;
            shot(binder, QStringLiteral("v0920-binder-pin-hover"));
            QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, tree->pinRectOf(otherRow).center());
            settle(500);
            auto *fixedLine = panel->findChild<QLabel *>(QStringLiteral("mmFixedTitle"));
            auto *fixButton = panel->findChild<QToolButton *>(QStringLiteral("mmFixButton"));
            check(hovered && w.fixedMindMapId() == other->id && tree->fixedMindMapId() == other->id
                      && binder->selectedItems() == QList<BinderItem *>{synced},
                  QStringLiteral("a click on the pin of the row under the mouse fixes that item; the selection stays"),
                  QStringLiteral("hovered %1, fixed %2").arg(hovered).arg(w.fixedMindMapId()));
            check(panel->itemId() == other->id && panel->isFixed() && panel->doc() != nullptr && !mindmaps::hasMap(p, other->id)
                      && fixedLine && fixedLine->isVisible() && fixedLine->text().contains(QLatin1String("Other")) && fixButton
                      && fixButton->isChecked() && insp->item() == synced,
                  QStringLiteral("the Mind Map tab shows the fixed item's board (an empty one to start on), names the item, and "
                                 "its pin button is pressed; the inspector's other tabs stay with the selected document"),
                  fixedLine ? fixedLine->text() : QStringLiteral("no line"));
            check(bar->isVisible() && barTitle->text().contains(QLatin1String("Other")) && releaseAct->isEnabled() && fixAct->isEnabled(),
                  QStringLiteral("the binder shows the slim bar with the item's title and \"Release\""), barTitle->text());
            shot(&w, QStringLiteral("v0920-mind-map-fixed"));
            shot(binder, QStringLiteral("v0920-binder-fixed-bar"));

            // another selection: the tab stays
            BinderItem *third = textDoc(p, p->draft(), QStringLiteral("Third"), QStringLiteral("Third text."));
            selectOne(third);
            check(panel->itemId() == other->id && insp->item() == third && w.fixedMindMapId() == other->id,
                  QStringLiteral("another document selected: the tab still shows the fixed board"));
            // fixing another item moves the fixing (at most one in the project)
            const QModelIndex syncedRow = rowOf(synced);
            QTest::mouseMove(tree->viewport(), tree->visualRect(syncedRow).center());
            settle(150);
            QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, tree->pinRectOf(syncedRow).center());
            settle(400);
            check(w.fixedMindMapId() == synced->id && panel->itemId() == synced->id && barTitle->text().contains(QLatin1String("Synced")),
                  QStringLiteral("the pin of another row moves the fixing there: one fixed item per project"), barTitle->text());
            // the fixed row's pin releases
            QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, tree->pinRectOf(syncedRow).center());
            settle(400);
            check(w.fixedMindMapId().isEmpty() && !bar->isVisible() && panel->itemId() == third->id && !panel->isFixed()
                      && !releaseAct->isEnabled(),
                  QStringLiteral("a click on the fixed row's pin releases: the tab follows the selected item again, the bar goes"));

            // the tab's own button works both ways
            fixButton = panel->findChild<QToolButton *>(QStringLiteral("mmFixButton"));
            if (fixButton)
                fixButton->click();
            settle(400);
            const bool fixedByTab = w.fixedMindMapId() == third->id && panel->isFixed() && bar->isVisible();
            selectOne(other);
            const bool stays = panel->itemId() == third->id;
            fixButton = panel->findChild<QToolButton *>(QStringLiteral("mmFixButton"));
            if (fixButton)
                fixButton->click();
            settle(400);
            check(fixedByTab && stays && w.fixedMindMapId().isEmpty() && panel->itemId() == other->id,
                  QStringLiteral("the pin button of the Mind Map tab fixes the board it shows and releases it again"));

            // the menu commands, and the bar's button
            fixAct->trigger();
            settle(300);
            const bool byMenu = w.fixedMindMapId() == other->id && !fixAct->isEnabled() && releaseAct->isEnabled();
            barRelease->click();
            settle(300);
            const bool byBar = w.fixedMindMapId().isEmpty() && fixAct->isEnabled() && !releaseAct->isEnabled();
            fixAct->trigger();
            settle(200);
            releaseAct->trigger();
            settle(300);
            check(byMenu && byBar && w.fixedMindMapId().isEmpty(),
                  QStringLiteral("Inspect > Fix the Mind Map of This Item and Release the Fixed Mind Map work, each enabled only "
                                 "when it can act; the bar's \"Release\" releases"),
                  QStringLiteral("menu %1, bar %2").arg(byMenu).arg(byBar));

            // into the Trash: released
            fixAct->trigger();   // "Other" is selected
            settle(200);
            const bool wasFixed = w.fixedMindMapId() == other->id;
            BinderItem *gone = textDoc(p, p->draft(), QStringLiteral("Goes away"), QStringLiteral("Soon in the Trash."));
            selectOne(gone);
            w.fixMindMap(gone);
            settle(200);
            const bool moved = w.fixedMindMapId() == gone->id;
            p->model()->moveToTrash({gone});
            settle(400);
            check(wasFixed && moved && w.fixedMindMapId().isEmpty() && !bar->isVisible() && !tree->canFixMindMap(rowOf(gone)),
                  QStringLiteral("the fixed item moved to the Trash: the fixing is released, and an item in the Trash cannot be fixed"));
            // fixed for the reopening check at the end
            w.fixMindMap(other);
            settle(200);
        }
    }

    // ================================================================ the shown lock (to-do item 3)
    {
        auto *binder = w.findChild<BinderPanel *>();
        BinderTree *tree = binder ? binder->tree() : nullptr;
        const QString lockFile = p->settingsDir() + QStringLiteral("/lock");
        const QString ownFile = workmarks::fileFor(lockFile);
        auto docsIn = [](const QString &file) {
            QStringList out;
            for (const QJsonValue &v : QJsonDocument::fromJson(readAll(file)).object().value(QStringLiteral("docs")).toArray())
                out << v.toString();
            return out;
        };
        binder->select({other}, true);
        settle(1200);
        const QStringList mine = docsIn(ownFile);
        check(QFile::exists(ownFile) && mine.contains(other->id) && mine.contains(synced->id)
                  && QJsonDocument::fromJson(readAll(ownFile)).object().value(QStringLiteral("key")).toString() == syncsafe::machineKey(),
              QStringLiteral("this device writes beside the lock which documents it has open or has changed since it opened the project"),
              mine.join(QLatin1Char(' ')));
        // more typing in a document that is already on the list: the file stays as it is
        const QByteArray bytes = readAll(ownFile);
        const QDateTime stamp = QFileInfo(ownFile).lastModified();
        for (int i = 0; i < 4; ++i) {
            typeInto(p, other, QStringLiteral(" more"));
            settle(400);
        }
        check(readAll(ownFile) == bytes && QFileInfo(ownFile).lastModified() == stamp,
              QStringLiteral("typing on does not write the list again: it is written only when it changes"));
        // another document opened: the list changes once
        BinderItem *opened = textDoc(p, p->draft(), QStringLiteral("Opened later"), QStringLiteral("Read only."));
        binder->select({opened}, true);
        settle(1200);
        check(docsIn(ownFile).contains(opened->id) && readAll(ownFile) != bytes,
              QStringLiteral("another document opened in the editor: the list is written again and names it"));

        // another device says it works on "Synced"
        qputenv("QB_DEVICE_NAME", "Laptop");
        const QString theirFile = workmarks::fileFor(lockFile);
        const bool wrote = workmarks::write(lockFile, QDateTime::currentDateTimeUtc().addSecs(-600), {synced->id, QStringLiteral("not-in-this-project")});
        qunsetenv("QB_DEVICE_NAME");
        w.readWorkMarks();
        settle(200);
        tree->expandAll();
        settle(100);
        const QModelIndex syncedRow = p->model()->indexFor(synced);
        check(wrote && theirFile != ownFile && tree->lockTip(synced->id).contains(QLatin1String("Laptop"))
                  && tree->lockTip(synced->id).contains(QLatin1String("nothing is blocked")) && tree->lockTip(other->id).isEmpty()
                  && tree->lockTip(p->draft()->id).contains(QLatin1String("Laptop")) && tree->lockTip(p->draft()->id).contains(QLatin1String("in here")),
              QStringLiteral("a document another device works on gets the padlock, and so does the folder around it; the tool tip "
                             "names the device and says that nothing is blocked"),
              tree->lockTip(synced->id) + QStringLiteral(" || ") + tree->lockTip(p->draft()->id));
        shot(binder, QStringLiteral("v0920-binder-padlock"));
        {
            // the padlock is painted at the row's right end: the row looks different there than without the mark
            const QRect at = tree->lockRectOf(syncedRow);
            const QImage with = tree->viewport()->grab(at).toImage();
            binder->setLockMarks({});   // (no wait: the window reads the lists again every 0.3 s in this test)
            const QImage without = tree->viewport()->grab(at).toImage();
            w.readWorkMarks();
            settle(100);
            QHelpEvent tipEvent(QEvent::ToolTip, at.center(), tree->viewport()->mapToGlobal(at.center()));
            QCoreApplication::sendEvent(tree->viewport(), &tipEvent);
            settle(100);
            check(with != without && at.right() < tree->viewport()->width() && QToolTip::text().contains(QLatin1String("Laptop")),
                  QStringLiteral("the padlock is drawn at the right end of the row, and pointing at it shows the tool tip"),
                  QToolTip::text());
            QToolTip::hideText();
        }
        // it is a hint: the document can be edited and saved as ever
        typeInto(p, synced, QStringLiteral(" Edited under the padlock."));
        check(p->save(&err) && readAll(p->docs()->textPath(synced->id)).contains("Edited under the padlock."),
              QStringLiteral("a hint, not a guard: a marked document is edited and saved as ever"), err);

        // without a sign of life for longer than a lock counts as left over: not shown
        QJsonObject theirs = QJsonDocument::fromJson(readAll(theirFile)).object();
        theirs.insert(QStringLiteral("alive"), QDateTime::currentDateTimeUtc().addSecs(-20 * 60).toString(Qt::ISODate));
        writeAll(theirFile, QJsonDocument(theirs).toJson(QJsonDocument::Compact));
        w.readWorkMarks();
        const bool gone = tree->lockTip(synced->id).isEmpty() && tree->lockTip(p->draft()->id).isEmpty();
        // ... unless that device holds the lock, which it renews: the lock's sign of life counts
        const QByteArray ourLock = readAll(lockFile);
        qputenv("QB_DEVICE_NAME", "Laptop");
        syncsafe::writeLock(lockFile, QDateTime::currentDateTimeUtc().addSecs(-600));
        qunsetenv("QB_DEVICE_NAME");
        w.readWorkMarks();
        const bool byLock = tree->lockTip(synced->id).contains(QLatin1String("Laptop"));
        writeAll(lockFile, ourLock);   // (this window's lock again, before anything looks at it)
        check(gone && byLock,
              QStringLiteral("a list without a sign of life for more than 15 minutes is not shown, unless its device holds the "
                             "lock and renews that"),
              QStringLiteral("gone %1, by the lock %2").arg(gone).arg(byLock));
        // their list is left alone by this device; only its own goes at closing (checked at the end)
        QFile::remove(theirFile);
        w.readWorkMarks();
        check(tree->lockTip(synced->id).isEmpty(), QStringLiteral("the other device closed the project (its list is gone): the padlock goes"));
        qputenv("QB_DEVICE_NAME", "Laptop");
        workmarks::write(lockFile, QDateTime::currentDateTimeUtc(), {other->id});
        qunsetenv("QB_DEVICE_NAME");
    }

    // ================================================================ a manual save whose backup cannot be made (item 10)
    {
        writeAll(W + QStringLiteral("/a-file"), "not a folder");
        as.backupOnManualSave = true;
        as.backupDir = W + QStringLiteral("/a-file/backups");
        as.backupKeep = 3;
        typeInto(p, other, QStringLiteral(" Saved by hand."));
        const bool saved = w.manualSave();
        settle(500);
        QPointer<QMessageBox> notice = visibleBox(QStringLiteral("manualBackupNotice"));
        check(saved && notice && !notice->isModal() && QApplication::activeModalWidget() == nullptr
                  && !notice->text().trimmed().isEmpty() && readAll(p->docs()->textPath(other->id)).contains("Saved by hand."),
              QStringLiteral("File > Save with a backup that cannot be put into its folder: the text is saved and a notice says "
                             "what happened to the backup (not modal)"),
              notice ? notice->text() : QStringLiteral("no notice; status bar: ") + w.statusBar()->currentMessage());
        shot(&w, QStringLiteral("v0920-manual-backup"));
        shot(notice, QStringLiteral("v0920-manual-backup-notice"));
        if (notice)
            notice->accept();
        settle(200);
        typeInto(p, other, QStringLiteral(" And once more."));
        const bool saved2 = w.manualSave();
        settle(500);
        check(saved2 && !visibleBox(QStringLiteral("manualBackupNotice")) && !w.statusBar()->currentMessage().isEmpty(),
              QStringLiteral("the same cause at the next save: no second notice, the status bar says it"),
              w.statusBar()->currentMessage());
        if (QMessageBox *b = visibleBox(QStringLiteral("manualBackupNotice")))
            b->accept();
        as.backupOnManualSave = false;
        as.backupDir.clear();
    }

    // ================================================================ the states of add-ons and program versions (item 10)
    {
        const QByteArray themeOne = "{\"name\":\"Way Back\",\"colors\":{\"window\":\"#202020\",\"text\":\"#f0f0f0\"}}";
        const QByteArray themeTwo = "{\"name\":\"Way Back\",\"colors\":{\"window\":\"#303030\",\"text\":\"#f0f0f0\"}}";
        QStringList log;
        bool restart = false;
        QString why;

        // ---- the choice in the Add-on Manager that cannot be written
        const QString sid = QStringLiteral("test-state");
        const QString stateDir = addons::addonsRoot() + QLatin1Char('/') + sid;
        bool found = false;
        check(addons::install(addonPackage(W, sid, QStringLiteral("1.0"), themeOne), &log, &err, &restart) && addonEnabled(sid, &found) && found
                  && readAll(stateDir + QStringLiteral("/themes/test.json")) == themeOne,
              QStringLiteral("(a small add-on installed)"), err + log.join(QLatin1Char(' ')));
        qputenv("QB_TEST_FAIL_STATE", "addons");
        const bool switched = addons::setEnabled(sid, false, &why);
        qunsetenv("QB_TEST_FAIL_STATE");
        check(!switched && why.contains(QLatin1String("state.json")) && addonEnabled(sid),
              QStringLiteral("an add-on switched off while the state cannot be written: the call says so, with the file's name "
                             "(before: nothing, and the add-on was on again after the restart)"),
              why);
        check(addons::setEnabled(sid, false, &why) && !addonEnabled(sid),
              QStringLiteral("(switched off when the state can be written)"), why);
        // an install whose state cannot be written is not reported as done
        qputenv("QB_TEST_FAIL_STATE", "addons");
        QString instErr;
        const bool reported = addons::install(addonPackage(W, sid, QStringLiteral("1.1"), themeTwo), &log, &instErr, &restart);
        qunsetenv("QB_TEST_FAIL_STATE");
        check(!reported && instErr.contains(QLatin1String("could not be switched on")) && !addonEnabled(sid),
              QStringLiteral("an add-on installed while its state cannot be written: the install is told as failed, with what to do "
                             "(before: reported as installed)"),
              instErr);
        check(addons::setEnabled(sid, true, &why) && addonEnabled(sid) && readAll(stateDir + QStringLiteral("/themes/test.json")) == themeTwo,
              QStringLiteral("(switched on afterwards: the new version is there)"), why);

        // ---- the way back of a failed install
        const QString id = QStringLiteral("test-wayback");
        const QString target = addons::addonsRoot() + QLatin1Char('/') + id;
        const QString two = addonPackage(W, id, QStringLiteral("1.1"), themeTwo);
        check(addons::install(addonPackage(W, id, QStringLiteral("1.0"), themeOne), &log, &err, &restart)
                  && readAll(target + QStringLiteral("/themes/test.json")) == themeOne,
              QStringLiteral("(a second small add-on installed)"), err);
        qputenv("QB_TEST_FAIL_RENAME", ".new-|0");
        QString backErr;
        const bool failed = !addons::install(two, &log, &backErr, &restart);
        qunsetenv("QB_TEST_FAIL_RENAME");
        const QStringList asides = QDir(addons::addonsRoot()).entryList({id + QStringLiteral(".old-*")}, QDir::Dirs | QDir::Hidden);
        check(failed && readAll(target + QStringLiteral("/themes/test.json")) == themeOne && asides.isEmpty()
                  && !backErr.contains(QLatin1String("Not everything could be put back")),
              QStringLiteral("an install that cannot put the new version in place: the version before is back, nothing is left over"),
              backErr + QLatin1Char(' ') + asides.join(QLatin1String(", ")));
        qputenv("QB_TEST_FAIL_RENAME", "test-wayback|1");
        QString leftErr;
        const bool failed2 = !addons::install(two, &log, &leftErr, &restart);
        qunsetenv("QB_TEST_FAIL_RENAME");
        const QStringList left = QDir(addons::addonsRoot()).entryList({id + QStringLiteral(".old-*")}, QDir::Dirs | QDir::Hidden);
        check(failed2 && left.size() == 1 && leftErr.contains(QLatin1String("Not everything could be put back"))
                  && leftErr.contains(left.value(0)) && leftErr.contains(QLatin1String("rename that folder"))
                  && readAll(addons::addonsRoot() + QLatin1Char('/') + left.value(0) + QStringLiteral("/themes/test.json")) == themeOne,
              QStringLiteral("when the version before cannot be put back either, the message says where it is and what to do "
                             "(before: only the first failure was named)"),
              leftErr + QStringLiteral(" || ") + left.join(QLatin1String(", ")));

        // the list of program versions
        QJsonObject st = launcher::readState();
        st.insert(QStringLiteral("probe"), 1);
        qputenv("QB_TEST_FAIL_STATE", "core");
        const bool wrote = launcher::writeState(st, &why);
        QString coreErr;
        const bool chosen = addons::setActiveCore(QString(), &coreErr);
        qunsetenv("QB_TEST_FAIL_STATE");
        check(!wrote && !why.isEmpty() && !launcher::readState().contains(QStringLiteral("probe")),
              QStringLiteral("the list of program versions that cannot be written: the call says so, and the old list stays"), why);
        check(!chosen && coreErr.contains(QLatin1String("could not be saved")),
              QStringLiteral("choosing a program version while the list cannot be written is told as failed (before: reported as done)"),
              coreErr);
        check(launcher::writeState(st, &why) && launcher::readState().contains(QStringLiteral("probe")) && addons::setActiveCore(QString(), &coreErr),
              QStringLiteral("(written when it can be)"), why + coreErr);
    }

    // ================================================================ the reason of a failed save, by its kind (item 10)
    {
        using failure::Kind;
        const QString german = QStringLiteral("Auf dem Gerät ist kein Speicherplatz mehr verfügbar");
        check(MainWindow::plainSaveReason(german, Kind::DiskFull) == QLatin1String("The disk is full.")
                  && MainWindow::plainSaveReason(german, Kind::None) == QLatin1String("The files could not be written.")
                  && MainWindow::plainSaveReason(QStringLiteral("Keine Berechtigung"), Kind::NoRights).contains(QLatin1String("may not write"))
                  && MainWindow::plainSaveReason(QStringLiteral("x"), Kind::ReadOnly).contains(QLatin1String("only be read"))
                  && MainWindow::plainSaveReason(QStringLiteral("x"), Kind::DriveError).contains(QLatin1String("reported an error"))
                  && MainWindow::plainSaveReason(QStringLiteral("x"), Kind::FolderGone).contains(QLatin1String("not there any more"))
                  && MainWindow::plainSaveReason(QStringLiteral("y: No space left on device"), Kind::None) == QLatin1String("The disk is full."),
              QStringLiteral("the plain reason of a failed save follows the failure's kind; the error's words decide only without a kind"));
        check(failure::ofErrno(ENOSPC) == Kind::DiskFull && failure::ofErrno(EROFS) == Kind::ReadOnly && failure::ofErrno(EACCES) == Kind::NoRights
                  && failure::ofErrno(EIO) == Kind::DriveError && failure::ofErrno(ENOENT) == Kind::None,
              QStringLiteral("the system's error numbers give the kinds (disk full, read only, no rights, drive error)"));

        // the German interface, and a system that words its errors in German: a failing save while typing
        QTranslator de, qtDe;
        const bool loaded = de.load(QStringLiteral(":/translations/quillbinder_de.qm")) && qtDe.load(QStringLiteral(":/translations/qt_de.qm"));
        QCoreApplication::installTranslator(&de);
        QCoreApplication::installTranslator(&qtDe);
        const QString full = QCoreApplication::translate("qb::MainWindow", "The disk is full.");
        const QString unknown = QCoreApplication::translate("qb::MainWindow", "The files could not be written.");
        qputenv("QB_TEST_WRITE_FAILS", QByteArray::number(ENOSPC) + '|' + german.toUtf8() + '|' + QFileInfo(p->docs()->textPath(other->id)).absolutePath().section(QLatin1Char('/'), -1).toUtf8());
        typeInto(p, other, QStringLiteral(" Typed while the disk is full."));
        waitFor([&] { return w.saveFailedSince().isValid(); }, 20000);
        settle(400);
        QPointer<QMessageBox> notice = w.findChild<QMessageBox *>(QStringLiteral("saveFailedNotice"));
        check(loaded && full != QLatin1String("The disk is full.") && notice && notice->isVisible() && notice->text().contains(full)
                  && !notice->text().contains(unknown) && notice->detailedText().contains(german),
              QStringLiteral("German interface, German system error: the notice of the failed save says that the disk is full "
                             "(before: only \"the files could not be written\")"),
              (notice ? notice->text() : QStringLiteral("no notice")) + QStringLiteral(" || loaded %1, \"%2\"").arg(loaded).arg(full));
        shot(notice, QStringLiteral("v0920-save-failed-german"));
        qunsetenv("QB_TEST_WRITE_FAILS");
        const bool saved = p->save(&err);
        settle(400);
        check(saved && readAll(p->docs()->textPath(other->id)).contains("Typed while the disk is full."),
              QStringLiteral("(saved once the disk takes the file again)"), err);
        if (notice)
            notice->close();
        QCoreApplication::removeTranslator(&de);
        QCoreApplication::removeTranslator(&qtDe);
    }

    // ================================================================ the fixed mind map is kept with the project
    {
        const QString otherId = other->id;
        const bool fixedBefore = w.fixedMindMapId() == otherId;
        check(p->save(&err), QStringLiteral("(saved)"), err);
        const QString lockFile = p->settingsDir() + QStringLiteral("/lock");
        const QString ownWork = workmarks::fileFor(lockFile);
        qputenv("QB_DEVICE_NAME", "Laptop");
        const QString theirWork = workmarks::fileFor(lockFile);
        qunsetenv("QB_DEVICE_NAME");
        const bool both = QFile::exists(ownWork) && QFile::exists(theirWork);
        w.closeProject();
        settle(400);
        check(both && !QFile::exists(ownWork) && QFile::exists(theirWork) && !QFile::exists(lockFile),
              QStringLiteral("closing the project removes this device's list with its lock, and leaves the other device's alone"));
        QFile::remove(theirWork);
        const bool cleared = w.fixedMindMapId().isEmpty();
        const bool opened = w.openProject(projFolder);
        settle(900);
        auto *binder = w.findChild<BinderPanel *>();
        auto *bar = binder ? binder->findChild<QWidget *>(QStringLiteral("binderFixedMindMapBar")) : nullptr;
        check(fixedBefore && cleared && opened && w.project() && w.fixedMindMapId() == otherId && bar && bar->isVisible()
                  && binder->tree()->fixedMindMapId() == otherId,
              QStringLiteral("the fixing is kept with the project: closed and opened again, the same item is fixed"),
              QStringLiteral("before %1, cleared %2, opened %3, now \"%4\"").arg(fixedBefore).arg(cleared).arg(opened).arg(w.fixedMindMapId()));
        w.releaseMindMap();
    }

    w.closeProject();
    settle(300);
    delete wp;
    std::printf("%d failure(s)\n", failures);
    return failures;
}
