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
//    around them), with a tool tip; a list without a sign of life is not shown; the list goes at closing;
//  - the check for updates: the signed file and what it offers, the delays and "ignore", the first start's note, at
//    most one look a day, the window for functional and for security updates, download and install, a new AppImage
//    in the place of the one that runs, an address that cannot be reached, a missing, forged or older file. The
//    project's page is a small server inside this test;
//  - the add-on "Translation and Proofreading" as one of the project's own (its place, version and model address).
//
//
// Windows 0.92.0 (2026-11-20): on Windows the whole program anew is its setup program, not an AppImage. The checks
// of the rules take the system's names for it; in the place of the AppImage that is swapped, Windows checks the
// hand-over to the setup program (downloaded into <data>/update-setup, checked, started for the program's folder),
// and the tool that writes the update file is also run for Windows.
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
#include "core/UpdateCheck.h"
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
#include "ui/UpdateChecker.h"
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
#ifdef Q_OS_WIN
#include <QThread>
#endif
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QHelpEvent>
#include <QIcon>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QSslSocket>
#include <QStyleFactory>
#include <QTcpServer>
#include <QTcpSocket>
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

// The checks of Windows' setup programs that need no Windows (the rules, the tool): on Windows always; defined by
// hand on another system they can be tried there (the hand-over itself is in an #ifdef Q_OS_WIN part).
#if defined(Q_OS_WIN) && !defined(QB_V0920_SETUP_RULES)
#define QB_V0920_SETUP_RULES
#endif

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

static QByteArray runTool(const QString &prog, const QStringList &args, const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment())
{
    QProcess p;
    p.setProcessEnvironment(env);
    p.start(prog, args);
    if (!p.waitForFinished(60000))
        return QByteArray();
    return p.readAllStandardOutput();
}

// The project's page, on this computer: serves the files it was given, and keeps what was asked for.
class TestServer : public QObject
{
public:
    QTcpServer server;
    QHash<QString, QByteArray> files;
    QStringList requests;
    QList<QByteArray> raw;
    QHash<QTcpSocket *, QByteArray> incoming;
    TestServer()
    {
        connect(&server, &QTcpServer::newConnection, this, [this] {
            while (QTcpSocket *s = server.nextPendingConnection()) {
                connect(s, &QTcpSocket::readyRead, this, [this, s] {
                    incoming[s] += s->readAll();
                    if (!incoming[s].contains("\r\n\r\n"))
                        return;
                    const QByteArray request = incoming.take(s);
                    raw << request;
                    const QString path = QString::fromLatin1(request.split(' ').value(1));
                    requests << path;
                    if (files.contains(path)) {
                        const QByteArray body = files.value(path);
                        s->write("HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + QByteArray::number(body.size())
                                 + "\r\nConnection: close\r\n\r\n" + body);
                    } else {
                        s->write("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
                    }
                    s->disconnectFromHost();
                });
                connect(s, &QTcpSocket::disconnected, s, &QObject::deleteLater);
            }
        });
    }
    bool start() { return server.isListening() || server.listen(QHostAddress::LocalHost, 0); }
    void stop() { server.close(); }
    QString base() const { return QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort()); }
};

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
#ifdef Q_OS_WIN
            // Windows: QTest's move also sets the real pointer, and on the build machine's desktop the binder is then
            // told that the mouse left (run of f7bb321: the click fixed the item, but no row counted as under the
            // mouse). The move is handed to the binder itself once more and looked at before the desktop answers.
            if (tree->hoverIndex() != otherRow) {
                const QPoint at = tree->visualRect(otherRow).center();
                QMouseEvent move(QEvent::MouseMove, at, tree->viewport()->mapToGlobal(at), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
                QApplication::sendEvent(tree->viewport(), &move);
            }
#endif
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
            // 0.93.0: the line "Fixed: ..." stands in the bar and is shortened when the bar is narrow; the item's whole
            // name is in its tooltip, so that is what names the item here (the shown text depends on the window's width).
            check(panel->itemId() == other->id && panel->isFixed() && panel->doc() != nullptr && !mindmaps::hasMap(p, other->id)
                      && fixedLine && fixedLine->isVisible() && fixedLine->toolTip().contains(QLatin1String("Other")) && fixButton
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
              notice ? notice->text() : QStringLiteral("no notice; status bar: ") + w.statusNote());
        shot(&w, QStringLiteral("v0920-manual-backup"));
        shot(notice, QStringLiteral("v0920-manual-backup-notice"));
        if (notice)
            notice->accept();
        settle(200);
        typeInto(p, other, QStringLiteral(" And once more."));
        const bool saved2 = w.manualSave();
        settle(500);
        check(saved2 && !visibleBox(QStringLiteral("manualBackupNotice")) && !w.statusNote().isEmpty(),
              QStringLiteral("the same cause at the next save: no second notice, the status bar says it"),
              w.statusNote());
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

    // ================================================================ the check for updates (to-do item 7)
    {
        using namespace updates;
        const QString src = QStringLiteral(QB_TEST_SOURCE_DIR "/..");
        // a release key of the test, and a signed file as the release workflow makes one
        const QString keyFile = W + QStringLiteral("/update-key.pem");
        runTool(QStringLiteral("openssl"), {QStringLiteral("genpkey"), QStringLiteral("-algorithm"), QStringLiteral("ed25519"), QStringLiteral("-out"), keyFile});
        const QByteArray pub = runTool(QStringLiteral("openssl"), {QStringLiteral("pkey"), QStringLiteral("-in"), keyFile, QStringLiteral("-pubout"),
                                                                    QStringLiteral("-outform"), QStringLiteral("DER")}).right(32);
        qputenv("QB_TEST_SIGNING_KEY", pub.toHex());
        auto sign = [&](const QByteArray &bytes) {
            writeAll(W + QStringLiteral("/to-sign"), bytes);
            QFile::remove(W + QStringLiteral("/to-sign.sig"));
            runTool(QStringLiteral("openssl"), {QStringLiteral("pkeyutl"), QStringLiteral("-sign"), QStringLiteral("-rawin"), QStringLiteral("-inkey"), keyFile,
                                               QStringLiteral("-in"), W + QStringLiteral("/to-sign"), QStringLiteral("-out"), W + QStringLiteral("/to-sign.sig")});
            return readAll(W + QStringLiteral("/to-sign.sig"));
        };
        auto fileEntry = [](const QString &url, const QByteArray &bytes, const QString &name) {
            QJsonObject o;
            o.insert(QStringLiteral("url"), url);
            o.insert(QStringLiteral("name"), name);
            o.insert(QStringLiteral("size"), bytes.size());
            o.insert(QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex()));
            return o;
        };
        const QString t0 = QStringLiteral("2026-11-20T08:00:00Z");
        const QDateTime start = QDateTime::fromString(t0, Qt::ISODate);
        auto feedBytes = [&](const QJsonObject &program, const QJsonArray &addonList, const QString &date) {
            QJsonObject sys, systems, root;
            if (!program.isEmpty())
                sys.insert(QStringLiteral("program"), program);
            sys.insert(QStringLiteral("addons"), addonList);
            systems.insert(systemName(), sys);
            root.insert(QStringLiteral("format"), 1);
            root.insert(QStringLiteral("date"), date);
            root.insert(QStringLiteral("systems"), systems);
            return QJsonDocument(root).toJson();
        };

        // ---- the rules, without any network
        {
            const QByteArray file = "https://example.org/file";
            QJsonObject program;
            program.insert(QStringLiteral("version"), QStringLiteral("9.1.0"));
            program.insert(QStringLiteral("runtime"), QStringLiteral(QB_RUNTIME_ID));
            program.insert(QStringLiteral("security"), QJsonArray{QStringLiteral("9.0.5")});
            program.insert(QStringLiteral("whatsNew"), QJsonArray{QStringLiteral("Something new.")});
            program.insert(QStringLiteral("update"), fileEntry(QStringLiteral("https://example.org/u.qbpkg"), "update", QStringLiteral("u.qbpkg")));
#ifdef Q_OS_WIN
            // Windows: the whole program anew is named by its setup programs
            const QString kPlain = QStringLiteral("setup"), kFull = QStringLiteral("setupFull");
            const Offer::Kind kWhole = Offer::Kind::NewSetup;
#else
            const QString kPlain = QStringLiteral("appimage"), kFull = QStringLiteral("appimageFull");
            const Offer::Kind kWhole = Offer::Kind::NewAppImage;
#endif
            program.insert(kPlain, fileEntry(QStringLiteral("https://example.org/a.AppImage"), "plain", QStringLiteral("a.AppImage")));
            program.insert(kFull, fileEntry(QStringLiteral("https://example.org/f.AppImage"), "full", QStringLiteral("f.AppImage")));
            QJsonObject addonA, addonB, addonC;
            addonA.insert(QStringLiteral("id"), QStringLiteral("have-it"));
            addonA.insert(QStringLiteral("name"), QStringLiteral("Have It"));
            addonA.insert(QStringLiteral("version"), QStringLiteral("2.0"));
            addonA.insert(QStringLiteral("minApp"), QStringLiteral("0.80.0"));
            addonA.insert(QStringLiteral("file"), fileEntry(QStringLiteral("https://example.org/a.qbpkg"), "a", QStringLiteral("a.qbpkg")));
            addonB = addonA;
            addonB.insert(QStringLiteral("id"), QStringLiteral("not-installed"));
            addonC = addonA;
            addonC.insert(QStringLiteral("id"), QStringLiteral("other-runtime"));
            addonC.insert(QStringLiteral("runtime"), QStringLiteral("some-other-runtime"));
            const QByteArray json = feedBytes(program, QJsonArray{addonA, addonB, addonC}, t0);
            const QByteArray sig = sign(json);
            Feed feed;
            QString why;
            QByteArray changed = json;
            changed.replace("9.1.0", "9.9.0");
            check(sig.size() == 64 && readFeed(json, sig, &feed, &why) && readFeed(json, sig.toBase64() + '\n', &feed, &why)
                      && !readFeed(changed, sig, nullptr, &why) && !readFeed(json, QByteArray(64, 'x'), nullptr, &why)
                      && !readFeed(json, QByteArray(), nullptr, &why) && feed.date == start,
                  QStringLiteral("the update file counts only with the release key's signature of exactly its bytes"), why);
            Installed here;
            here.appVersion = QStringLiteral("9.0.0");
            here.runtime = QStringLiteral(QB_RUNTIME_ID);
            here.addons = {{QStringLiteral("have-it"), QStringLiteral("1.0")}, {QStringLiteral("other-runtime"), QStringLiteral("1.0")}};
            const QList<Offer> offers = offersFor(feed, systemName(), here);
            const Offer prog = offers.value(0), add = offers.value(1);
            check(offers.size() == 2 && prog.id == QLatin1String("program") && prog.kind == Offer::Kind::ProgramUpdate && prog.security
                      && prog.file.name == QLatin1String("u.qbpkg") && prog.whatsNew.size() == 1 && add.id == QLatin1String("have-it")
                      && add.kind == Offer::Kind::Addon && !add.security && add.installed == QLatin1String("1.0"),
                  QStringLiteral("a newer program (as a program update on the same runtime; a security update lies between) and the "
                                 "newer version of an installed add-on are offered; an add-on that is not installed or is made "
                                 "for another runtime is not"),
                  QString::number(offers.size()));
            here.runtime = QStringLiteral("an-older-runtime");
            here.appVersion = QStringLiteral("9.0.5");
            here.fullAppImage = true;
            const QList<Offer> image = offersFor(feed, systemName(), here);
            here.appVersion = QStringLiteral("9.1.0");
            here.addons = {{QStringLiteral("have-it"), QStringLiteral("2.0")}};
            check(image.size() == 2 && image.first().kind == kWhole && image.first().file.name == QLatin1String("f.AppImage")
                      && !image.first().security && image.last().id == QLatin1String("have-it") && offersFor(feed, systemName(), here).isEmpty()
                      && offersFor(feed, QStringLiteral("some-other-system"), here).isEmpty(),
                  QStringLiteral("another runtime: the new AppImage is offered (the full one to the full version) beside the add-on "
                                 "that fits the program as it is; who has the newest versions is offered nothing"),
                  QString::number(image.size()));

            // the delays of a functional update, with the clock moved
            State st;
            st.enabled = true;
            Offer f = add;
            QStringList seen;
            QDateTime t = start;
            for (int round = 0; round < 6; ++round) {
                const Shown s = toShow({f}, st, t, false);
                if (s.offers.isEmpty()) {
                    seen << QStringLiteral("none");
                    break;
                }
                // not a minute before it is due
                seen << QStringLiteral("%1%2").arg(s.delayDays).arg(s.lastReminder ? QStringLiteral(" last") : QString());
                applyDelay(s, &st, t);
                const QDateTime due = st.versions.value(f.key()).next;
                if (due.isValid() && !st.versions.value(f.key()).off) {
                    if (!toShow({f}, st, due.addSecs(-60), false).offers.isEmpty())
                        seen << QStringLiteral("too early");
                    t = due.addSecs(60);
                } else {
                    t = t.addDays(400);
                }
            }
            check(seen == QStringList{QStringLiteral("1"), QStringLiteral("7"), QStringLiteral("14"), QStringLiteral("28"), QStringLiteral("28 last"), QStringLiteral("none")}
                      && st.versions.value(f.key()).off && toShow({f}, st, t, true).offers.size() == 1,
                  QStringLiteral("a functional update delayed: 1 day, 1 week, 2 weeks, 4 weeks, one last reminder, then no more; "
                                 "Check for Updates Now still shows it"),
                  seen.join(QLatin1String(", ")));
            // a security update: no end, and "ignore" does not touch it
            State ss;
            Offer sec = prog;
            seen.clear();
            t = start;
            for (int round = 0; round < 7; ++round) {
                const Shown s = toShow({sec}, ss, t, false);
                seen << (s.offers.isEmpty() ? QStringLiteral("none") : QString::number(s.delayDays) + (s.lastReminder ? QStringLiteral(" last") : QString()));
                applyIgnore(s, &ss);
                applyDelay(s, &ss, t);
                t = ss.versions.value(sec.key()).next.addSecs(60);
            }
            check(seen == QStringList{QStringLiteral("1"), QStringLiteral("7"), QStringLiteral("14"), QStringLiteral("28"), QStringLiteral("28"), QStringLiteral("28"), QStringLiteral("28")}
                      && !ss.versions.value(sec.key()).ignored && !ss.versions.value(sec.key()).off && toShow({sec}, ss, t, false).security,
                  QStringLiteral("a security update delayed: 1 day, 1 week, 2 weeks, then every 4 weeks, without an end; it cannot be ignored"),
                  seen.join(QLatin1String(", ")));
            // ignore, and a newer version starts over; together with a security update the window follows its rules
            State si;
            applyIgnore(toShow({f}, si, start, false), &si);
            Offer newer = f;
            newer.version = QStringLiteral("2.1");
            const Shown both = toShow({f, sec}, State(), start, false);
            check(toShow({f}, si, start.addDays(90), false).offers.isEmpty() && toShow({newer}, si, start, false).offers.size() == 1
                      && both.offers.size() == 2 && both.security && !both.lastReminder,
                  QStringLiteral("an ignored version stays away, a newer one is offered again; a security update beside a functional "
                                 "one makes the window follow the security rules"));
            State sd;
            sd.enabled = true;
            const bool beforeNote = autoCheckDue(sd, start);
            sd.noteShown = true;
            const bool first = autoCheckDue(sd, start);
            sd.lastCheck = start;
            const bool sameDay = autoCheckDue(sd, start.addSecs(23 * 3600)), nextDay = autoCheckDue(sd, start.addSecs(24 * 3600 + 1));
            sd.enabled = false;
            check(!beforeNote && first && !sameDay && nextDay && !autoCheckDue(sd, start.addDays(30)),
                  QStringLiteral("the automatic look: not before the first start's note was seen, at most once in 24 hours, never "
                                 "when switched off"));
        }

        // ---- the release workflow's tool writes the file the program reads
        {
            const QString dist = W + QStringLiteral("/dist");
            const QString version = QStringLiteral(QB_APP_VERSION);
            const QByteArray plain = "plain AppImage", full = "the full AppImage, larger";
            writeAll(dist + QStringLiteral("/Quillbinder-%1-beta-x86_64.AppImage").arg(version), plain);
            writeAll(dist + QStringLiteral("/Quillbinder-%1-full-beta-x86_64.AppImage").arg(version), full);
            writeAll(dist + QStringLiteral("/SHA256SUMS.txt"), "not a package");
            auto package = [&](const QString &name, const QJsonObject &manifest) {
                ZipWriter zw(dist + QLatin1Char('/') + name);
                zw.addFile(QStringLiteral("manifest.json"), QJsonDocument(manifest).toJson());
                zw.close();
            };
            QJsonObject core, addon, bundle, windows;
            core.insert(QStringLiteral("id"), QStringLiteral("core"));
            core.insert(QStringLiteral("kind"), QStringLiteral("core"));
            core.insert(QStringLiteral("version"), version);
            package(QStringLiteral("quillbinder-%1-update.qbpkg").arg(version), core);
            addon.insert(QStringLiteral("id"), QStringLiteral("feed-addon"));
            addon.insert(QStringLiteral("kind"), QStringLiteral("addon"));
            addon.insert(QStringLiteral("name"), QStringLiteral("Feed Add-on"));
            addon.insert(QStringLiteral("version"), QStringLiteral("3.1"));
            addon.insert(QStringLiteral("minApp"), QStringLiteral("0.80.0"));
            addon.insert(QStringLiteral("runtime"), QStringLiteral(QB_RUNTIME_ID));
            addon.insert(QStringLiteral("whatsNew"), QJsonArray{QStringLiteral("One thing.")});
            package(QStringLiteral("feed-addon.qbpkg"), addon);
            windows = addon;
            windows.insert(QStringLiteral("id"), QStringLiteral("feed-addon-windows"));
            windows.insert(QStringLiteral("platform"), QStringLiteral("windows-x64"));
            package(QStringLiteral("feed-addon-windows.qbpkg"), windows);
            bundle.insert(QStringLiteral("id"), QStringLiteral("a-bundle"));
            bundle.insert(QStringLiteral("kind"), QStringLiteral("bundle"));
            bundle.insert(QStringLiteral("version"), QStringLiteral("1.0"));
            package(QStringLiteral("a-bundle.qbpkg"), bundle);
            // the file published so far: another system's block, and an add-on of this system released on its own
            QJsonObject oldAddon;
            oldAddon.insert(QStringLiteral("id"), QStringLiteral("released-alone"));
            oldAddon.insert(QStringLiteral("version"), QStringLiteral("1.4"));
            QJsonObject oldProgram;
            oldProgram.insert(QStringLiteral("version"), QStringLiteral("0.1.0"));
            oldProgram.insert(QStringLiteral("security"), QJsonArray{QStringLiteral("0.0.9")});
            QJsonObject oldSystems, oldRoot, oldLinux, oldWindows;
            oldLinux.insert(QStringLiteral("program"), oldProgram);
            oldLinux.insert(QStringLiteral("addons"), QJsonArray{oldAddon});
            oldWindows.insert(QStringLiteral("addons"), QJsonArray());
            oldSystems.insert(QStringLiteral("linux-x86_64"), oldLinux);
            oldSystems.insert(QStringLiteral("windows-x64"), oldWindows);
            oldRoot.insert(QStringLiteral("format"), 1);
            oldRoot.insert(QStringLiteral("date"), QStringLiteral("2026-01-01T00:00:00Z"));
            oldRoot.insert(QStringLiteral("systems"), oldSystems);
            writeAll(W + QStringLiteral("/previous.json"), QJsonDocument(oldRoot).toJson());
            const QString out = W + QStringLiteral("/made-updates-1.json");
            QStringList toolArgs{QStringLiteral("-I"), src + QStringLiteral("/tools/update-feed.py"), QStringLiteral("--dist"), dist,
                                 QStringLiteral("--tag"), QStringLiteral("v%1-beta").arg(version), QStringLiteral("--repo"),
                                 QStringLiteral("Some-One/Some_Repo"), QStringLiteral("--previous"), W + QStringLiteral("/previous.json"),
                                 QStringLiteral("--date"), QStringLiteral("2026-11-20T10:00:00Z"), QStringLiteral("--out"), out};
#ifdef Q_OS_WIN
            // (the tool's own runtime is the Linux one of CMakeLists.txt; this check compares with the program's)
            toolArgs << QStringLiteral("--runtime") << QStringLiteral(QB_RUNTIME_ID);
#endif
            const QByteArray said = runTool(QStringLiteral("python3"), toolArgs);
            const QByteArray made = readAll(out);
            Feed feed;
            QString why;
            const bool read = readFeed(made, sign(made), &feed, &why);
            Installed here;
            here.appVersion = QStringLiteral("0.0.5");
            here.runtime = QStringLiteral(QB_RUNTIME_ID);
            here.addons = {{QStringLiteral("feed-addon"), QStringLiteral("3.0")}, {QStringLiteral("released-alone"), QStringLiteral("1.4")},
                           {QStringLiteral("feed-addon-windows"), QStringLiteral("1.0")}, {QStringLiteral("a-bundle"), QStringLiteral("0.5")}};
            const QList<Offer> offers = offersFor(feed, QStringLiteral("linux-x86_64"), here);
            const Offer prog = offers.value(0), add = offers.value(1);
            const QString start = QStringLiteral("https://github.com/Some-One/Some_Repo/releases/download/v%1-beta/").arg(version);
            check(read && offers.size() == 2 && prog.kind == Offer::Kind::ProgramUpdate && prog.version == version && prog.security
                      && prog.file.url == start + QStringLiteral("quillbinder-%1-update.qbpkg").arg(version)
                      && prog.file.size == QFileInfo(dist + QStringLiteral("/quillbinder-%1-update.qbpkg").arg(version)).size()
                      && add.id == QLatin1String("feed-addon") && add.version == QLatin1String("3.1") && add.whatsNew == QStringList{QStringLiteral("One thing.")}
                      && feed.systems.contains(QStringLiteral("windows-x64")) && feed.date == QDateTime::fromString(QStringLiteral("2026-11-20T10:00:00Z"), Qt::ISODate)
                      && made.contains("released-alone"),
                  QStringLiteral("tools/update-feed.py writes the file from a release's files: the program update with address, size and "
                                 "SHA-256, the add-ons of this system, the security versions and the other systems of the file before"),
                  QString::fromUtf8(said) + why + QStringLiteral(" offers: %1").arg(offers.size()));
            here.runtime = QStringLiteral("another-runtime");
            here.fullAppImage = true;
            const QList<Offer> image = offersFor(feed, QStringLiteral("linux-x86_64"), here);
            check(image.value(0).kind == Offer::Kind::NewAppImage && image.value(0).file.size == full.size()
                      && image.value(0).file.sha256 == QString::fromLatin1(QCryptographicHash::hash(full, QCryptographicHash::Sha256).toHex())
                      && image.value(0).file.url.endsWith(QStringLiteral("-full-beta-x86_64.AppImage")),
                  QStringLiteral("... and both AppImages, so that the full version is offered the full one"));
            check(QSslSocket::supportsSsl(), QStringLiteral("Qt's TLS part finds this system's OpenSSL (secure connections for the check)"),
                  QSslSocket::sslLibraryBuildVersionString());
#ifdef QB_V0920_SETUP_RULES
            // ---- Windows: the same tool writes Windows' block from the files of a Windows release, beside Linux's
            const QString distW = W + QStringLiteral("/dist-windows");
            const QString winRuntime = QStringLiteral("win64-test-runtime");
            const QByteArray setupPlain = "the setup program", setupFull = "the full setup program, larger";
            const QString nameW = QStringLiteral("Quillbinder-%1-experimental-windows-x64").arg(version);
            const QString nameWFull = QStringLiteral("Quillbinder-%1-full-experimental-windows-x64").arg(version);
            writeAll(distW + QLatin1Char('/') + nameW + QStringLiteral("-setup.exe"), setupPlain);
            writeAll(distW + QLatin1Char('/') + nameWFull + QStringLiteral("-setup.exe"), setupFull);
            writeAll(distW + QLatin1Char('/') + nameW + QStringLiteral(".zip"), "a zip, not named in the file");
            writeAll(distW + QStringLiteral("/SHA256SUMS-windows.txt"), "not a package");
            auto packageW = [&](const QString &name, const QJsonObject &manifest) {
                ZipWriter zw(distW + QLatin1Char('/') + name);
                zw.addFile(QStringLiteral("manifest.json"), QJsonDocument(manifest).toJson());
                zw.close();
            };
            const QString coreW = QStringLiteral("quillbinder-%1-experimental-windows-program.qbpkg").arg(version);
            packageW(coreW, core);
            bundle.insert(QStringLiteral("version"), version);
            packageW(QStringLiteral("quillbinder-%1-experimental-windows-update.qbpkg").arg(version), bundle);
            QJsonObject withLibraries = addon, plainData, linuxLibraries = addon;
            withLibraries.insert(QStringLiteral("version"), QStringLiteral("3.2"));
            withLibraries.insert(QStringLiteral("platform"), QStringLiteral("windows-x64"));
            withLibraries.insert(QStringLiteral("runtime"), winRuntime);
            packageW(QStringLiteral("feed-addon-windows.qbpkg"), withLibraries);
            plainData.insert(QStringLiteral("id"), QStringLiteral("plain-data"));
            plainData.insert(QStringLiteral("kind"), QStringLiteral("addon"));
            plainData.insert(QStringLiteral("version"), QStringLiteral("2.0"));
            packageW(QStringLiteral("plain-data-windows.qbpkg"), plainData);
            linuxLibraries.insert(QStringLiteral("id"), QStringLiteral("linux-libraries"));
            packageW(QStringLiteral("linux-libraries.qbpkg"), linuxLibraries);
            const QString outW = W + QStringLiteral("/made-updates-1-windows.json");
            const QString tagW = QStringLiteral("v%1-windows-experimental").arg(version);
            const QByteArray saidW = runTool(QStringLiteral("python3"),
                                             {QStringLiteral("-I"), src + QStringLiteral("/tools/update-feed.py"), QStringLiteral("--dist"), distW, QStringLiteral("--tag"), tagW,
                                              QStringLiteral("--repo"), QStringLiteral("Some-One/Some_Repo"), QStringLiteral("--previous"), out,
                                              QStringLiteral("--system"), QStringLiteral("windows-x64"), QStringLiteral("--runtime"), winRuntime,
                                              QStringLiteral("--label"), QStringLiteral("experimental"), QStringLiteral("--date"),
                                              QStringLiteral("2026-11-21T10:00:00Z"), QStringLiteral("--out"), outW});
            const QByteArray madeW = readAll(outW);
            Feed feedW;
            QString whyW;
            const bool readW = readFeed(madeW, sign(madeW), &feedW, &whyW);
            const QJsonObject blockW = feedW.systems.value(QStringLiteral("windows-x64")).toObject();
            const QJsonObject progW = blockW.value(QStringLiteral("program")).toObject();
            Installed hereW;
            hereW.appVersion = QStringLiteral("0.0.5");
            hereW.runtime = winRuntime;
            hereW.addons = {{QStringLiteral("feed-addon"), QStringLiteral("3.0")}, {QStringLiteral("plain-data"), QStringLiteral("1.0")},
                          {QStringLiteral("linux-libraries"), QStringLiteral("1.0")}, {QStringLiteral("a-bundle"), QStringLiteral("0.5")}};
            const QList<Offer> offersW = offersFor(feedW, QStringLiteral("windows-x64"), hereW);
            const QString startW = QStringLiteral("https://github.com/Some-One/Some_Repo/releases/download/%1/").arg(tagW);
            QStringList offered;
            for (const Offer &o : offersW)
                offered << o.id + QLatin1Char(' ') + o.version;
            check(readW && feedW.systems.value(QStringLiteral("linux-x86_64")) == feed.systems.value(QStringLiteral("linux-x86_64"))
                      && progW.value(QStringLiteral("label")).toString() == QLatin1String("experimental")
                      && progW.value(QStringLiteral("runtime")).toString() == winRuntime && !progW.contains(QStringLiteral("appimage"))
                      && offered == QStringList{QStringLiteral("program ") + version, QStringLiteral("feed-addon 3.2"), QStringLiteral("plain-data 2.0")}
                      && offersW.value(0).kind == Offer::Kind::ProgramUpdate && offersW.value(0).file.url == startW + coreW
                      && offersW.value(1).file.url == startW + QStringLiteral("feed-addon-windows.qbpkg"),
                  QStringLiteral("Windows: tools/update-feed.py writes Windows' block from a Windows release's files (the program update, the "
                                 "add-ons made for Windows and those without libraries; not the bundle, not Linux libraries) and leaves "
                                 "Linux's block as it was"),
                  QString::fromUtf8(saidW) + whyW + QStringLiteral(" offered: ") + offered.join(QLatin1String(", ")));
            hereW.runtime = QStringLiteral("another-runtime");
            const QList<Offer> plainW = offersFor(feedW, QStringLiteral("windows-x64"), hereW);
            hereW.fullAppImage = true;
            const QList<Offer> fullW = offersFor(feedW, QStringLiteral("windows-x64"), hereW);
            check(plainW.value(0).kind == Offer::Kind::NewSetup && plainW.value(0).file.url == startW + nameW + QStringLiteral("-setup.exe")
                      && plainW.value(0).file.size == setupPlain.size() && fullW.value(0).kind == Offer::Kind::NewSetup
                      && fullW.value(0).file.url == startW + nameWFull + QStringLiteral("-setup.exe")
                      && fullW.value(0).file.sha256 == QString::fromLatin1(QCryptographicHash::hash(setupFull, QCryptographicHash::Sha256).toHex())
                      && offersFor(feed, QStringLiteral("windows-x64"), hereW).isEmpty(),
                  QStringLiteral("Windows: on another runtime the new version is offered as its setup program, the full one to the full "
                                 "version; a file that names no Windows program offers Windows none"),
                  QStringLiteral("%1 / %2").arg(plainW.value(0).file.url, fullW.value(0).file.url));
            check(UpdateChecker::setupArguments(QStringLiteral("C:/Schöne Bücher/Quill binder/"))
                          == QStringLiteral("/SILENT /NOCANCEL /QBUPDATE=1 /DIR=\"C:\\Schöne Bücher\\Quill binder\"")
                      || QDir::separator() != QLatin1Char('\\'),
                  QStringLiteral("Windows: the setup program is started for the program's folder, without pages, as an update run; a "
                                 "folder's blanks and a backslash at its end do not break the command"),
                  UpdateChecker::setupArguments(QStringLiteral("C:/Schöne Bücher/Quill binder/")));
#endif
        }

        // ---- with a server on this computer in the place of the project's page
        TestServer server;
        const bool listening = server.start();
        const QString base = server.base();
        qputenv("QB_TEST_UPDATE_URL", (base + QStringLiteral("/updates-1.json")).toUtf8());
        qputenv("QB_TEST_UPDATE_RETRY_MS", "300");
        qputenv("QB_TEST_NO_RESTART", "1");
        qputenv("QB_TEST_UPDATE_NOW", t0.toUtf8());
        UpdateChecker *upd = w.updateChecker();
        QAction *atStart = w.findChild<QAction *>(QStringLiteral("updateCheckAuto"));
        QAction *nowAct = w.findChild<QAction *>(QStringLiteral("updateCheckNow"));
        check(listening && upd && atStart && nowAct && atStart->isCheckable() && atStart->isChecked() && upd->isEnabled(),
              QStringLiteral("Help has \"Check for Updates at Startup\" (on at first) and \"Check for Updates Now\""));
        int looks = 0, reached = 0, lastOffers = -1;
        QObject::connect(upd, &UpdateChecker::finished, &app, [&](bool ok, int n) {
            ++looks;
            reached += ok ? 1 : 0;
            lastOffers = n;
        });
        auto publish = [&](const QJsonObject &program, const QJsonArray &addonList, const QString &date) {
            const QByteArray json = feedBytes(program, addonList, date);
            server.files.insert(QStringLiteral("/updates-1.json"), json);
            server.files.insert(QStringLiteral("/updates-1.json.sig"), sign(json));
        };
        auto setState = [](bool enabled, bool noteShown) {
            QFile::remove(State::file());
            State st;
            st.enabled = enabled;
            st.noteShown = noteShown;
            st.save();
        };
        auto lookAndWait = [&](bool manual) {
            const int before = looks;
            if (manual)
                upd->checkNow();
            else
                upd->startAutomatic();
#ifdef Q_OS_WIN
            // (Windows takes about two seconds to report a refused connection, four with the second try; the look of
            // the unreachable address took 8.2 s in the run of f7bb321)
            waitFor([&] { return looks > before; }, 30000);
#else
            waitFor([&] { return looks > before; }, 8000);
#endif
            settle(200);
            return looks > before;
        };
        // version 1.1 of the add-on "test-state" is installed (above); the page offers 1.2
        const QByteArray themeNew = "{\"name\":\"Way Back\",\"colors\":{\"window\":\"#404040\",\"text\":\"#f0f0f0\"}}";
        const QString pkg12 = addonPackage(W, QStringLiteral("test-state"), QStringLiteral("1.2"), themeNew);
        {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("QB_SIGNING_KEY_FILE"), keyFile);
            runTool(QStringLiteral("python3"), {QStringLiteral("-I"), src + QStringLiteral("/tools/make-package.py"), QStringLiteral("sign"), pkg12}, env);
        }
        const QByteArray pkgBytes = readAll(pkg12);
        server.files.insert(QStringLiteral("/files/test-state-1.2.qbpkg"), pkgBytes);
        auto addonEntry = [&](const QString &version, bool security, const QByteArray &bytes) {
            QJsonObject a;
            a.insert(QStringLiteral("id"), QStringLiteral("test-state"));
            a.insert(QStringLiteral("name"), QStringLiteral("Way Back Test"));
            a.insert(QStringLiteral("version"), version);
            a.insert(QStringLiteral("minApp"), QStringLiteral("0.80.0"));
            a.insert(QStringLiteral("security"), security ? QJsonArray{version} : QJsonArray());
            a.insert(QStringLiteral("whatsNew"), QJsonArray{QStringLiteral("A darker window color.")});
            a.insert(QStringLiteral("file"), fileEntry(base + QStringLiteral("/files/test-state-1.2.qbpkg"), bytes, QStringLiteral("test-state-1.2.qbpkg")));
            return a;
        };
        publish(QJsonObject(), QJsonArray{addonEntry(QStringLiteral("1.2"), false, pkgBytes)}, t0);

        // switched off: nothing leaves the program
        setState(false, true);
        upd->startAutomatic();
        settle(900);
        check(upd->requests() == 0 && server.requests.isEmpty() && looks == 0 && !upd->isBusy(),
              QStringLiteral("switched off: the program's start contacts nothing"));

        // the first start: the note, and no contact before it was answered
        setState(true, false);
        upd->startAutomatic();
        settle(900);
        QPointer<QMessageBox> note = visibleBox(QStringLiteral("updateFirstNote"));
        const bool quietBefore = upd->requests() == 0 && server.requests.isEmpty();
        auto *offButton = note ? note->findChild<QPushButton *>(QStringLiteral("updateNoteOff")) : nullptr;
        check(note && quietBefore && !note->isModal() && offButton && note->text().contains(QLatin1String("Nothing about you is sent")),
              QStringLiteral("the first start shows a note with OK and \"Turn Off\"; nothing is contacted before it is answered"),
              note ? note->text() : QStringLiteral("no note"));
        shot(note, QStringLiteral("v0920-update-first-note"));
        if (offButton)
            offButton->click();
        settle(500);
        check(!State::load().enabled && State::load().noteShown && !atStart->isChecked() && upd->requests() == 0,
              QStringLiteral("\"Turn Off\" switches the check off (the Help menu shows it) and nothing was contacted"));
        setState(true, false);
        atStart->setChecked(true);
        upd->startAutomatic();
        settle(600);
        note = visibleBox(QStringLiteral("updateFirstNote"));
        auto *okButton = note ? note->findChild<QPushButton *>(QStringLiteral("updateNoteOk")) : nullptr;
        const int looksBefore = looks;
        if (okButton)
            okButton->click();
        waitFor([&] { return looks > looksBefore; }, 8000);
        settle(300);
        QPointer<QDialog> win = upd->window();
        auto buttonOf = [&](const char *name) { return win ? win->findChild<QPushButton *>(QLatin1String(name)) : nullptr; };
        check(okButton && reached == 1 && lastOffers == 1 && server.requests == QStringList{QStringLiteral("/updates-1.json"), QStringLiteral("/updates-1.json.sig")}
                  && upd->requests() == 2,
              QStringLiteral("after OK the program fetches the one small file and its signature, nothing else"), server.requests.join(QLatin1Char(' ')));
        {
            // what the request carries: the program's name as the sender, no cookie, no version, nothing about the system
            const QByteArray raw = server.raw.value(0);
            QStringList headers;
            for (const QByteArray &line : raw.split('\n'))
                if (line.contains(':'))
                    headers << QString::fromLatin1(line.left(line.indexOf(':'))).trimmed().toLower();
            headers.sort();
            const QString lower = QString::fromLatin1(raw).toLower();
            check(raw.startsWith("GET /updates-1.json HTTP/1.1") && lower.contains(QLatin1String("user-agent: quillbinder\r\n"))
                      && !lower.contains(QLatin1String("cookie")) && !lower.contains(QLatin1String(QB_APP_VERSION)) && !lower.contains(QLatin1String("linux"))
                      && !lower.contains(QLatin1String("x11")) && !raw.contains(syncsafe::deviceName().toUtf8()),
                  QStringLiteral("the request is a plain GET with the program's name as sender: no version, no system, no cookie, "
                                 "no name of this computer"),
                  headers.join(QLatin1String(", ")));
        }
        check(win && win->isVisible() && !win->isModal() && buttonOf("updateInstall") && buttonOf("updateIgnore") && buttonOf("updateDelay")
                  && buttonOf("updateDelay")->text() == QLatin1String("Delay (1 day)")
                  && win->findChild<QLabel *>(QStringLiteral("updateList"))->text().contains(QLatin1String("functional"))
                  && win->findChild<QLabel *>(QStringLiteral("updateList"))->text().contains(QLatin1String("1.1"))
                  && win->findChild<QLabel *>(QStringLiteral("updateNotes"))->text().contains(QLatin1String("1 day, 1 week, 2 weeks, 4 weeks")),
              QStringLiteral("a functional update: a window with \"Download and Install\", \"Ignore This Version\" and \"Delay (1 day)\", "
                             "the installed and the new version, the kind, and the timeline of the delays"));
        shot(win, QStringLiteral("v0920-update-window"));
        if (QPushButton *b = buttonOf("updateDelay"))
            b->click();
        settle(300);
        // once a day: the same day nothing is fetched; a day later the delay has passed too
        const int requestsBefore = upd->requests();
        upd->startAutomatic();
        settle(700);
        const bool sameDayQuiet = upd->requests() == requestsBefore && !upd->window();
        qputenv("QB_TEST_UPDATE_NOW", start.addSecs(25 * 3600).toString(Qt::ISODate).toUtf8());
        const bool looked = lookAndWait(false);
        win = upd->window();
        check(sameDayQuiet && looked && upd->requests() == requestsBefore + 2 && win && buttonOf("updateDelay")
                  && buttonOf("updateDelay")->text() == QLatin1String("Delay (1 week)"),
              QStringLiteral("at most one look a day; a day after \"Delay\" the window is back and offers \"Delay (1 week)\""));
        if (QPushButton *b = buttonOf("updateIgnore"))
            b->click();
        settle(300);
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(40).toString(Qt::ISODate).toUtf8());
        const bool lookedLater = lookAndWait(false);
        const bool ignoredQuiet = lookedLater && !upd->window() && lastOffers == 0;
        // Check for Updates Now shows it all the same
        nowAct->trigger();
        waitFor([&] { return upd->window() != nullptr; }, 8000);
        win = upd->window();
        check(ignoredQuiet && win && win->isVisible(),
              QStringLiteral("\"Ignore This Version\": no window any more at the start; Help > Check for Updates Now still shows it"));
        if (win)
            win->close();
        settle(200);

        // a security update: two buttons, marked as recommended
        publish(QJsonObject(), QJsonArray{addonEntry(QStringLiteral("1.2"), true, pkgBytes)}, QStringLiteral("2026-12-30T09:00:00Z"));
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(42).toString(Qt::ISODate).toUtf8());
        lookAndWait(false);
        win = upd->window();
        check(win && buttonOf("updateInstall") && buttonOf("updateInstall")->text().contains(QLatin1String("recommended: security"))
                  && !buttonOf("updateIgnore") && buttonOf("updateDelay")
                  && win->findChild<QLabel *>(QStringLiteral("updateHeadline"))->text().contains(QLatin1String("recommended"))
                  && win->findChild<QLabel *>(QStringLiteral("updateList"))->text().contains(QLatin1String("recommended (security)"))
                  && win->findChild<QLabel *>(QStringLiteral("updateNotes"))->text().contains(QLatin1String("cannot be ignored")),
              QStringLiteral("a security update comes although the version was ignored as a functional one: \"Download and Install "
                             "(recommended: security)\" and \"Delay\" only, no \"Ignore\""));
        shot(win, QStringLiteral("v0920-update-window-security"));

        // Download and Install: the signed package of the page is installed
        auto answerBox = [&](const char *boxName, const char *button, QString *text) {
            auto *poll = new QTimer(&app);
            poll->setInterval(100);
            QObject::connect(poll, &QTimer::timeout, &app, [poll, boxName, button, text] {
                if (QMessageBox *b = visibleBox(QLatin1String(boxName))) {
                    if (text)
                        *text = b->text() + QLatin1Char(' ') + b->informativeText();
                    poll->stop();
                    poll->deleteLater();
                    QPushButton *named = b->findChild<QPushButton *>(QLatin1String(button));
                    if (named)
                        named->click();
                    else
                        b->accept();
                }
            });
            poll->start();
        };
        auto answerResult = [&](const char *button, QString *text) { answerBox("updateResult", button, text); };
        auto installedVersion = [](const QString &id) {
            for (const addons::Installed &a : addons::installedAddons())
                if (a.manifest.id == id)
                    return a.manifest.version;
            return QString();
        };
        int installs = 0;
        QObject::connect(upd, &UpdateChecker::installed, &app, [&installs] { ++installs; });
        QString resultText;
        answerResult("none", &resultText);
        if (QPushButton *b = buttonOf("updateInstall"))
            b->click();
        waitFor([&] { return installs == 1; }, 15000);
        settle(300);
        check(installs == 1 && installedVersion(QStringLiteral("test-state")) == QLatin1String("1.2")
                  && readAll(addons::addonsRoot() + QStringLiteral("/test-state/themes/test.json")) == themeNew
                  && server.requests.contains(QStringLiteral("/files/test-state-1.2.qbpkg")) && resultText.contains(QLatin1String("1.2")),
              QStringLiteral("\"Download and Install\" fetches the add-on's package, checks it and installs it; a message says what was installed"),
              resultText);
        // asked for by hand with nothing new: a message says so
        QString upToDate;
        answerBox("updateNothing", "none", &upToDate);
        lookAndWait(true);
        waitFor([&] { return !upToDate.isEmpty(); }, 5000);
        check(upToDate.contains(QLatin1String("up to date")) && !upd->window(),
              QStringLiteral("Help > Check for Updates Now with nothing new says that everything is up to date"), upToDate);
        // a file that is not the one the signed list names is not installed
        const QString pkg13 = addonPackage(W, QStringLiteral("test-state"), QStringLiteral("1.3"), themeNew);
        server.files.insert(QStringLiteral("/files/test-state-1.2.qbpkg"), readAll(pkg13));
        publish(QJsonObject(), QJsonArray{addonEntry(QStringLiteral("1.3"), false, pkgBytes)}, QStringLiteral("2026-12-31T09:00:00Z"));
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(44).toString(Qt::ISODate).toUtf8());
        lookAndWait(false);
        win = upd->window();
        resultText.clear();
        answerResult("none", &resultText);
        if (QPushButton *b = buttonOf("updateInstall"))
            b->click();
        waitFor([&] { return installs == 2; }, 15000);
        settle(300);
        check(installs == 2 && installedVersion(QStringLiteral("test-state")) == QLatin1String("1.2")
                  && resultText.contains(QLatin1String("not the one the signed list names")),
              QStringLiteral("a downloaded file that is not the one the signed list names is not installed, and the message says so"), resultText);

        // a file with a wrong signature, and an older file, are ignored
        const QDateTime accepted = State::load().feedDate;
        QByteArray forged = feedBytes(QJsonObject(), QJsonArray{addonEntry(QStringLiteral("7.0"), true, pkgBytes)}, QStringLiteral("2027-01-05T09:00:00Z"));
        server.files.insert(QStringLiteral("/updates-1.json"), forged);   // (the signature of the file before)
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(46).toString(Qt::ISODate).toUtf8());
        lookAndWait(false);
        const bool forgedIgnored = !upd->window() && State::load().feedDate == accepted && lastOffers == 0;
        publish(QJsonObject(), QJsonArray{addonEntry(QStringLiteral("6.0"), true, pkgBytes)}, QStringLiteral("2026-12-01T09:00:00Z"));
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(48).toString(Qt::ISODate).toUtf8());
        lookAndWait(false);
        check(forgedIgnored && !upd->window() && State::load().feedDate == accepted && lastOffers == 0,
              QStringLiteral("a file whose signature does not fit is ignored, and so is a signed file older than the newest one seen"));

        // the address answers, but the file is not there: quiet (no note)
        server.files.remove(QStringLiteral("/updates-1.json"));
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(50).toString(Qt::ISODate).toUtf8());
        int requestsNow = upd->requests();
        lookAndWait(false);
        const bool quiet404 = upd->requests() == requestsNow + 1 && !w.findChild<QLabel *>(QStringLiteral("fadingNote")) && !upd->window();
        // the address cannot be reached: two tries, then a small note in the status bar that fades in and out twice
        server.stop();
        requestsNow = upd->requests();
        QElapsedTimer took;
        took.start();
        lookAndWait(false);
        const qint64 ms = took.elapsed();
        QPointer<QLabel> fading = w.findChild<QLabel *>(QStringLiteral("fadingNote"));
        const bool modalNow = QApplication::activeModalWidget() != nullptr;
        check(quiet404 && upd->requests() == requestsNow + 2 && ms >= 300 && fading && fading->isVisible() && fading->property("fades").toInt() == 2
                  && fading->text().contains(QLatin1String("Could not look for updates")) && !modalNow && !upd->window(),
              QStringLiteral("a missing file is no failure; an address that cannot be reached is tried twice, then a small text below "
                             "the editor fades in and out twice (no window)"),
              QStringLiteral("requests +%1, %2 ms, note %3").arg(upd->requests() - requestsNow).arg(ms).arg(fading != nullptr));
        shot(&w, QStringLiteral("v0920-update-not-reached"));
        requestsNow = upd->requests();
        lookAndWait(false);
        const int notes = int(w.findChildren<QLabel *>(QStringLiteral("fadingNote")).size());
#ifdef Q_OS_WIN
        // (Windows needs seconds to refuse each connection, so the first note can be gone before the second look
        // has ended: then there is no note, and never a second one)
        const bool once = notes <= 1;
#else
        const bool once = notes == 1;
#endif
        waitFor([&] { return fading.isNull(); }, 12000);
        check(upd->requests() == requestsNow + 2 && once && fading.isNull(),
              QStringLiteral("the note comes at most once a day, and it is gone after its two rounds"));

#ifdef Q_OS_WIN
        // Windows: a new version that needs the whole program anew comes as its setup program. It is downloaded into
        // <data>/update-setup, checked, and started for the program's folder when the writer says so
        server.start();
        const QString baseNow = server.base();
        qputenv("QB_TEST_UPDATE_URL", (baseNow + QStringLiteral("/updates-1.json")).toUtf8());
        const QByteArray setupBytes = QByteArray("the new setup program ") + QByteArray(5000, 's');
        server.files.insert(QStringLiteral("/files/new-setup.exe"), setupBytes);
        const QString setupName = QStringLiteral("Quillbinder-99.0.0-experimental-windows-x64-setup.exe");
        const QString setupLog = W + QStringLiteral("/setup-started.txt");
        qputenv("QB_TEST_SETUP_LOG", setupLog.toUtf8());
        QJsonObject program;
        program.insert(QStringLiteral("version"), QStringLiteral("99.0.0"));
        program.insert(QStringLiteral("runtime"), QStringLiteral("a-later-runtime"));
        program.insert(QStringLiteral("whatsNew"), QJsonArray{QStringLiteral("New libraries.")});
        program.insert(QStringLiteral("setup"), fileEntry(baseNow + QStringLiteral("/files/new-setup.exe"), setupBytes, setupName));
        publish(program, QJsonArray(), QStringLiteral("2027-02-01T09:00:00Z"));
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(80).toString(Qt::ISODate).toUtf8());
        lookAndWait(false);
        win = upd->window();
        QLabel *notesLabel = win ? win->findChild<QLabel *>(QStringLiteral("updateNotes")) : nullptr;
        const QString setupNote = notesLabel ? notesLabel->text() : QString();
        resultText.clear();
        answerResult("updateSetupNow", &resultText);
        if (QPushButton *b = buttonOf("updateInstall"))
            b->click();
        waitFor([&] { return installs == 3; }, 15000);
        settle(300);
        const QString kept = updates::setupDownloadDir() + QLatin1Char('/') + setupName;
        const QString folder = QDir::toNativeSeparators(launcher::appDir());
        const QStringList started = QString::fromUtf8(readAll(setupLog)).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        check(installs == 3 && setupNote.contains(folder) && setupNote.contains(QLatin1String("setup program")) && readAll(kept) == setupBytes
                  && resultText.contains(QLatin1String("downloaded and checked")) && resultText.contains(QLatin1String("closes now"))
                  && started.size() == 2 && started.value(0) == QDir::toNativeSeparators(kept)
                  && started.value(1) == QStringLiteral("/SILENT /NOCANCEL /QBUPDATE=1 /DIR=\"%1\"").arg(folder),
              QStringLiteral("Windows: a new version that needs the whole program anew is downloaded as its setup program into the "
                             "program's own data folder, checked against the signed list, and started for the program's folder, "
                             "without pages, when the writer chooses \"Close and Update Now\""),
              resultText + QStringLiteral(" || ") + started.join(QLatin1String(" | ")) + QStringLiteral(" || ") + setupNote);
        shot(&w, QStringLiteral("v0920-update-setup"));
        // a setup program that is not the one the signed list names is neither kept nor started
        QFile::remove(setupLog);
        QByteArray otherBytes = setupBytes;
        otherBytes[otherBytes.size() - 1] = 'X';
        server.files.insert(QStringLiteral("/files/new-setup.exe"), otherBytes);
        publish(program, QJsonArray(), QStringLiteral("2027-02-02T09:00:00Z"));
        lookAndWait(true);
        win = upd->window();
        resultText.clear();
        answerResult("none", &resultText);
        if (QPushButton *b = buttonOf("updateInstall"))
            b->click();
        waitFor([&] { return installs == 4; }, 15000);
        settle(300);
        check(installs == 4 && !QFile::exists(kept) && !QFile::exists(setupLog) && resultText.contains(QLatin1String("not the one the signed list names")),
              QStringLiteral("Windows: a setup program that is not the one the signed list names is not kept and not started"), resultText);
        // what was downloaded goes at the program's next start
        writeAll(updates::setupDownloadDir() + QStringLiteral("/an-older-setup.exe"), "left over");
        updates::clearSetupDownload();
        check(!QDir(updates::setupDownloadDir()).exists(), QStringLiteral("Windows: a setup program downloaded before is removed at the next start"));
        qunsetenv("QB_TEST_SETUP_LOG");
#else
        // a new AppImage: it takes the place of the AppImage that runs, and the one before is kept beside it
        server.start();
        const QString baseNow = server.base();
        qputenv("QB_TEST_UPDATE_URL", (baseNow + QStringLiteral("/updates-1.json")).toUtf8());
        const QString own = W + QStringLiteral("/somewhere/Quillbinder-test-x86_64.AppImage");
        writeAll(own, "the AppImage that runs");
        qputenv("QB_TEST_APPIMAGE", own.toUtf8());
        const QByteArray image = QByteArray("the new AppImage ") + QByteArray(5000, 'n');
        server.files.insert(QStringLiteral("/files/new.AppImage"), image);
        QJsonObject program;
        program.insert(QStringLiteral("version"), QStringLiteral("99.0.0"));
        program.insert(QStringLiteral("runtime"), QStringLiteral("a-later-runtime"));
        program.insert(QStringLiteral("whatsNew"), QJsonArray{QStringLiteral("New libraries.")});
        program.insert(QStringLiteral("appimage"), fileEntry(baseNow + QStringLiteral("/files/new.AppImage"), image, QStringLiteral("Quillbinder-99.0.0-x86_64.AppImage")));
        publish(program, QJsonArray(), QStringLiteral("2027-02-01T09:00:00Z"));
        qputenv("QB_TEST_UPDATE_NOW", start.addDays(80).toString(Qt::ISODate).toUtf8());
        lookAndWait(false);
        win = upd->window();
        const QString imageNote = win ? win->findChild<QLabel *>(QStringLiteral("updateNotes"))->text() : QString();
        resultText.clear();
        answerResult("updateRestart", &resultText);
        if (QPushButton *b = buttonOf("updateInstall"))
            b->click();
        waitFor([&] { return installs == 3; }, 15000);
        settle(300);
        check(installs == 3 && imageNote.contains(QDir::toNativeSeparators(own)) && imageNote.contains(QLatin1String(".before-update"))
                  && readAll(own) == image && (QFileInfo(own).permissions() & QFileDevice::ExeOwner)
                  && readAll(own + QStringLiteral(".before-update")) == "the AppImage that runs"
                  && QDir(QFileInfo(own).absolutePath()).entryList(QDir::Files | QDir::Hidden).size() == 2
                  && resultText.contains(QLatin1String("in the place of your AppImage")) && resultText.contains(QLatin1String("after a restart")),
              QStringLiteral("a new AppImage is downloaded, checked and put in the place of the AppImage that runs, wherever that is; "
                             "the one before is kept beside it, and the program offers to start again"),
              resultText + QStringLiteral(" || ") + QDir(QFileInfo(own).absolutePath()).entryList(QDir::Files | QDir::Hidden).join(QLatin1String(", ")));
        // the way back when the new file cannot take the place
        QString backup, whyNot;
        writeAll(W + QStringLiteral("/somewhere/own2"), "own");
        const bool refused = !UpdateChecker::replaceOwnFile(W + QStringLiteral("/somewhere/own2"), W + QStringLiteral("/somewhere/not-there"), &backup, &whyNot);
        check(refused && readAll(W + QStringLiteral("/somewhere/own2")) == "own" && !QFile::exists(W + QStringLiteral("/somewhere/own2.before-update"))
                  && whyNot.contains(QLatin1String("Nothing was changed")),
              QStringLiteral("when the new file cannot take the place, the AppImage that runs is put back and the message says so"), whyNot);
#endif
        qunsetenv("QB_TEST_APPIMAGE");
        qunsetenv("QB_TEST_UPDATE_URL");
        qunsetenv("QB_TEST_UPDATE_NOW");
        setState(false, true);
    }

    // ================================================================ the translator as one of the project's own add-ons (item 6)
    {
        const QString src = QStringLiteral(QB_TEST_SOURCE_DIR "/..");
        const QString addon = src + QStringLiteral("/addons/translation-proofreading");
        const QByteArray top = readAll(addon + QStringLiteral("/manifest.json")), inner = readAll(addon + QStringLiteral("/source/packaging/manifest.json"));
        const QJsonObject m = QJsonDocument::fromJson(top).object();
        const QByteArray store = readAll(addon + QStringLiteral("/source/src/ModelStore.cpp"));
        const QByteArray cmake = readAll(addon + QStringLiteral("/source/CMakeLists.txt"));
        check(!top.isEmpty() && top == inner && m.value(QStringLiteral("id")).toString() == QLatin1String("translation-proofreading")
                  && m.value(QStringLiteral("version")).toString() == QLatin1String("1.3") && cmake.contains("set(TP_VERSION \"1.3\")")
                  && m.value(QStringLiteral("minApp")).toString() == QLatin1String(QB_RUNTIME_SINCE),
              QStringLiteral("Translation and Proofreading 1.3 is in addons/ with its source; its two manifests and its build agree on the version"),
              m.value(QStringLiteral("version")).toString());
        check(store.contains("https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/")
                  && !store.contains("Quillbinder_Translation_Addon")
                  && readAll(addon + QStringLiteral("/source/resources/catalog.json")).contains("\"release\": \"models-1\""),
              QStringLiteral("it fetches its models from the releases models-1, models-2 and models-3 of this repository"));
        {
            // 0.93.0: the pinned entry describes the last published package (the manifest's version, or a lower one while a
            // release is pending: the entry is updated after the release) and this repository; no file names the former repositories
            const QJsonObject entry = QJsonDocument::fromJson(readAll(src + QStringLiteral("/official-addons/translation-proofreading.json"))).object();
            const QByteArray official = readAll(src + QStringLiteral("/docs/official-addons.md"));
            check(!entry.value(QStringLiteral("version")).toString().isEmpty()
                      && Project::compareVersionText(entry.value(QStringLiteral("version")).toString(), m.value(QStringLiteral("version")).toString()) <= 0
                      && entry.value(QStringLiteral("repository")).toString() == QLatin1String("Shelin-boop/Quillbinder_Collection")
                      && !official.contains("Quillbinder_Translation_Addon") && !official.contains("addon-translation-proofreading-1.1")
                      && !QFileInfo::exists(src + QStringLiteral("/.github/workflows/models-mirror.yml")),
                  QStringLiteral("the pinned entry and docs/official-addons.md describe the add-on's version (or the last published one) in this repository; "
                                 "no link leads to the former repositories"));
        }
#ifdef Q_OS_WIN
        // Windows: the add-on is staged by packaging/windows/win-addons.sh, not by stage.sh (a Linux script): left out
        // without its engine libraries, which win-ci.sh builds with the add-on's own script; a release build fails without it
        const QByteArray winAddons = readAll(src + QStringLiteral("/packaging/windows/win-addons.sh"));
        const QByteArray winCi = readAll(src + QStringLiteral("/packaging/windows/win-ci.sh"));
        const QByteArray winRelease = readAll(src + QStringLiteral("/.github/workflows/release-windows.yml"));
        check(winAddons.contains("left out (its engine libraries are missing") && winAddons.contains("make-addon-windows.sh")
                  && winCi.contains("build-deps-windows.sh") && winCi.contains("stage/packages/translation-proofreading-windows.qbpkg")
                  && winRelease.contains("translator-deps-windows-msvc-")
                  && readAll(src + QStringLiteral("/.github/workflows/models-check.yml")).contains("releases/tags/$tag") && !readAll(src + QStringLiteral("/.github/workflows/models-check.yml")).contains("Quillbinder_Translation_Addon"),
              QStringLiteral("Windows: the Windows build stages the add-on with its own scripts and leaves it out when its engine "
                             "libraries are not built; a release build fails without the add-on; a workflow checks the model files here"));
#else
        // without its engine libraries it is left out of a local build (exit code 3), as the packing script expects
        QProcess stage;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("QB_TRANSLATOR_DEPS"), W + QStringLiteral("/no-deps-here"));
        stage.setProcessEnvironment(env);
        QDir().mkpath(W + QStringLiteral("/stage-build"));
        stage.start(QStringLiteral("bash"), {addon + QStringLiteral("/stage.sh"), W + QStringLiteral("/stage-build"), W + QStringLiteral("/stage-out")});
        stage.waitForFinished(60000);
        const QByteArray release = readAll(src + QStringLiteral("/.github/workflows/release.yml"));
        check(stage.exitStatus() == QProcess::NormalExit && stage.exitCode() == 3
                  && release.contains("tools/get-translator-deps.sh") && release.contains("for must in speech-transcription translation-proofreading")
                  && readAll(src + QStringLiteral("/.github/workflows/models-check.yml")).contains("releases/tags/$tag") && !readAll(src + QStringLiteral("/.github/workflows/models-check.yml")).contains("Quillbinder_Translation_Addon"),
              QStringLiteral("stage.sh leaves the add-on out when its engine libraries are not built; the release workflow builds "
                             "them and fails without the add-on; a workflow checks the model files here"),
              QString::fromUtf8(stage.readAllStandardError()) + QStringLiteral(" exit %1").arg(stage.exitCode()));
#endif
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
#ifdef Q_OS_WIN
        // (Windows takes qputenv's bytes in the system's code page, and the program reads the variable as text)
        qputenv("QB_TEST_WRITE_FAILS", QByteArray::number(ENOSPC) + '|' + german.toLocal8Bit() + '|' + QFileInfo(p->docs()->textPath(other->id)).absolutePath().section(QLatin1Char('/'), -1).toLocal8Bit());
#else
        qputenv("QB_TEST_WRITE_FAILS", QByteArray::number(ENOSPC) + '|' + german.toUtf8() + '|' + QFileInfo(p->docs()->textPath(other->id)).absolutePath().section(QLatin1Char('/'), -1).toUtf8());
#endif
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

#ifdef Q_OS_WIN
    // ================================================================ Windows: a new version's folder that is still held for a moment
    // (the release run of aa9f73a: a program update was started for its self-test from its temporary folder, and the
    // move of that folder into its place, tried once, was refused: "Cannot create ...\data\core\<version>")
    {
        const QString from = W + QStringLiteral("/held-new"), to = W + QStringLiteral("/held-in-place");
        const QString from2 = W + QStringLiteral("/held-new-2"), to2 = W + QStringLiteral("/held-in-place-2");
        QDir().mkpath(from);
        QDir().mkpath(from2);
        writeAll(from + QStringLiteral("/program.bin"), "the new version");
        writeAll(from2 + QStringLiteral("/program.bin"), "the new version");
        // a file that is open in the folder, as the program that was just started from it, or a virus scanner, holds one
        auto *held = new QFile(from + QStringLiteral("/program.bin"));
        auto *held2 = new QFile(from2 + QStringLiteral("/program.bin"));
        const bool opened = held->open(QIODevice::ReadOnly) && held2->open(QIODevice::ReadOnly);
        const bool refusedOnce = !QDir().rename(from, to) && !addons::moveIntoPlace(from2, to2, 1, 0);
        QThread *release = QThread::create([held] {
            QThread::msleep(900);
            held->close();
        });
        release->start();
        QElapsedTimer took;
        took.start();
        const bool moved = addons::moveIntoPlace(from, to);
        const qint64 ms = took.elapsed();
        release->wait(5000);
        delete release;
        delete held;
        held2->close();
        delete held2;
        check(opened && refusedOnce && moved && ms >= 500 && ms < 5000 && readAll(to + QStringLiteral("/program.bin")) == "the new version"
                  && !QFileInfo::exists(from) && QFileInfo::exists(from2) && !QFileInfo::exists(to2),
              QStringLiteral("Windows: a new version's folder that is still held for a moment (the program just started from it, a "
                             "virus scanner) is moved into its place once it is free; a single try is refused"),
              QStringLiteral("opened %1, refused at once %2, moved %3 after %4 ms").arg(opened).arg(refusedOnce).arg(moved).arg(ms));
    }
    // Windows 0.93.0 (item 33, an examination, not a check): can the folder of an add-on whose library is loaded be
    // moved aside, as installing a newer version of it does? (printed as an INFO line)
    {
        const QDir plugins(QCoreApplication::applicationDirPath() + QStringLiteral("/../tool-plugins"));
        const QStringList dlls = plugins.entryList({QStringLiteral("*.dll")}, QDir::Files, QDir::Name);
        if (dlls.isEmpty()) {
            std::printf("INFO item33 no plugin library in %s\n", qPrintable(plugins.absolutePath()));
        } else {
            const QString dir = W + QStringLiteral("/loaded-addon"), aside = W + QStringLiteral("/loaded-addon.old");
            QDir().mkpath(dir);
            QFile::copy(plugins.filePath(dlls.first()), dir + QLatin1Char('/') + dlls.first());
            QLibrary lib(dir + QLatin1Char('/') + dlls.first());
            const bool loaded = lib.load();
            const bool movedLoaded = addons::moveIntoPlace(dir, aside, 1, 0);
            if (movedLoaded)
                addons::moveIntoPlace(aside, dir, 1, 0);
            lib.unload();
            const bool movedAfter = addons::moveIntoPlace(dir, aside, 1, 0);
            std::printf("INFO item33 %s loaded %d: its folder moved aside while loaded %d, after unloading %d\n",
                        qPrintable(dlls.first()), loaded, movedLoaded, movedAfter);
        }
        std::fflush(stdout);
    }
#endif

    w.closeProject();
    settle(300);
    delete wp;
    std::printf("%d failure(s)\n", failures);
    return failures;
}
