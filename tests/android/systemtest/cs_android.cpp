// The system test for Android: cases only Android has (2026-11-16). Some need the tablet script
// (packaging/android/android-system-test.sh) to act from outside, named by the case's "needs":
//   K  the app is killed (kill -9, as Android ends an app it needs the memory of) once the case writes "kill-now"
//      into its folder; the check afterwards (G3, G4) compares the project on the tablet with the last model.json
//   H  the script presses Home once the case writes "home-now", waits, then kills the app (Android's usual way: an app
//      put away is ended later without a word)
// A-A01 asks for a sub-run in the same app data ("loaded": the add-on it installed is loaded at the start, as after
// a restart) through subcases.txt (QB_ST_KEEP_DATA). On the desktop build the cases run as far as a desktop can.
#include "cs.h"

#include "AppInfo.h"
#include "core/Addons.h"
#include "core/Backup.h"
#include "core/Platform.h"
#include "compile/PluginManager.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QMessageBox>
#include "ui/ToolHost.h"

namespace cs {

namespace {

bool onTablet()
{
#ifdef Q_OS_ANDROID
    return true;
#else
    return false;
#endif
}

// the tablet's shared Documents folder (Android 11 and later); "" on the desktop
QString sharedDocuments()
{
    return onTablet() ? QStringLiteral("/storage/emulated/0/Documents") : QString();
}

// types the text in pieces with pauses, so that the program's saves come between them
void typeInPieces(TextEditor *e, const QString &text, int pieceChars, int pauseMs, const std::function<bool()> &stop = nullptr)
{
    for (int i = 0; i < text.size(); i += pieceChars) {
        keysInto(e, text.mid(i, pieceChars));
        settle(pauseMs);
        if (stop && stop())
            return;
    }
}

void endOf(TextEditor *e)
{
    QTextCursor c = e->textCursor();
    c.movePosition(QTextCursor::End);
    e->setTextCursor(c);
    e->window()->activateWindow();
    e->setFocus(Qt::OtherFocusReason);
    settle(20);
}

QString writerText(quint32 seed, int words)
{
    return atParagraphs(seed, words).join(QLatin1Char('\n'));
}

} // namespace

// ================================================================ A-F01 the tablet's folders
void caseAF01()
{
    const AppSettings &s = AppSettings::instance();
    const QString docs = sharedDocuments();
    if (!onTablet()) {
        note(QStringLiteral("(not on a tablet: the shared folders are Android's; the case checks only the window)"));
    } else {
        const QString projects = docs + QStringLiteral("/Quillbinder");
        check(QDir::cleanPath(s.lastProjectDir) == projects, QStringLiteral("projects are made in the tablet's Documents/Quillbinder"), s.lastProjectDir);
        check(QFileInfo(projects).isDir(), QStringLiteral("... the folder is there"), projects);
        check(QDir::cleanPath(s.backupDir) == projects + QStringLiteral("/Backups"), QStringLiteral("backups go to Documents/Quillbinder/Backups"), s.backupDir);
        QSettings qs;
        check(QDir::cleanPath(qs.value(QStringLiteral("export/location")).toString()) == projects + QStringLiteral("/Exports"),
              QStringLiteral("exports go to Documents/Quillbinder/Exports"), qs.value(QStringLiteral("export/location")).toString());
        const QString own = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        check(addons::addonsRoot().startsWith(own) && !addons::addonsRoot().startsWith(QStringLiteral("/storage")),
              QStringLiteral("add-ons are kept in the app's private folder (Android runs no code from the shared storage)"),
              addons::addonsRoot());
    }
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")));
    Project *p = w->project();
    if (!p)
        return;
    if (onTablet()) {
        check(action(w, QStringLiteral("filesOnTablet")) != nullptr, QStringLiteral("File > Files on This Tablet is there"));
        check(action(w, QStringLiteral("installPackage")) && action(w, QStringLiteral("installPackage"))->isVisible(),
              QStringLiteral("Help > Add-ons and Updates > Install Update or Add-on is offered (add-ons, 0.91.3)"));
        // a backup by the command lands in the shared backup folder
        const QStringList before = QDir(s.backupDir).entryList(QDir::Files);
        answerBoxLater(800, QMessageBox::Ok);
        chooseByMenu(w, QStringLiteral("backupNow"));
        waitFor([&s, &before] { return QDir(s.backupDir).entryList(QDir::Files).size() > before.size(); }, 60000);
        settle(500);
        const QStringList after = QDir(s.backupDir).entryList(QDir::Files);
        check(after.size() > before.size(), QStringLiteral("Back Up Now writes a backup into Documents/Quillbinder/Backups"),
              after.join(QLatin1String(", ")).right(200));
    }
    finishProject(w);
    dropWindow(w);
}

// ================================================================ A-A01 add-ons on Android, A-A01@loaded the add-on at work
namespace {

// where the script puts the add-on's packages and the model's files: source/addons, source/models (on the desktop
// QB_ST_EXTRA names another folder, so that nothing lands in the repository)
QString extraDir()
{
    const QString e = qEnvironmentVariable("QB_ST_EXTRA");
    return e.isEmpty() ? ctx().sourceDir : e;
}

QString packageForThisTablet()
{
    const QString dir = extraDir() + QStringLiteral("/addons");
    const QString want = platform::packagePlatform(); // android-arm64 or android-x86_64
    for (const QFileInfo &fi : QDir(dir).entryInfoList({QStringLiteral("*.qbpkg")}, QDir::Files))
        if (fi.fileName().contains(want))
            return fi.absoluteFilePath();
    return QString();
}

// a package of the given system and kind, made by the test (cs_zip.cpp), with one library
QString madePackage(const QString &platformName, const QString &kind)
{
    const QString folder = ctx().work + QStringLiteral("/pkg-") + platformName + QLatin1Char('-') + kind;
    QDir(folder).removeRecursively();
    QDir().mkpath(folder + QStringLiteral("/payload/plugins"));
    writeText(folder + QStringLiteral("/payload/plugins/libcs-tool.so"), "\x7f" "ELF not a real library");
    const QString out = folder + QStringLiteral(".qbpkg");
    if (kind == QLatin1String("core")) {
        makeCorePackage(QByteArray("\x7f" "ELF a stand-in", 14), QStringLiteral("quillbinder"), QStringLiteral("99.0.0"),
                        QStringLiteral(QB_RUNTIME_ID), out);
        return out;
    }
    QJsonObject m{{QStringLiteral("id"), QStringLiteral("cs.") + platformName},
                  {QStringLiteral("name"), QStringLiteral("CS ") + platformName},
                  {QStringLiteral("version"), QStringLiteral("1.0")},
                  {QStringLiteral("minApp"), QStringLiteral("0.80.0")},
                  {QStringLiteral("platform"), platformName}}; // (the system the package claims to be for)
    writeText(folder + QStringLiteral("/manifest.json"), QJsonDocument(m).toJson());
    makeAddonPackage(folder, out);
    return out;
}

} // namespace

void caseAA01()
{
    if (variant() == QLatin1String("loaded")) {
        // ---- the add-on installed by A-A01 is loaded at this start; a model from a folder; a German sentence translated
        MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")));
        if (!w->project())
            return;
        QAction *win = action(w, QStringLiteral("tool:translation-proofreading.window"));
        check(win != nullptr, QStringLiteral("the add-on is loaded at the next start: its command is in the menu"));
        const QString models = extraDir() + QStringLiteral("/models/opus-mt-de-en");
        if (!win || !QFileInfo::exists(models + QStringLiteral("/opus-mt-de-en--model.bin"))) {
            note(QStringLiteral("(no model files for the check: %1)").arg(models));
            finishProject(w);
            dropWindow(w);
            return;
        }
        // the add-on's interface 1.1 reached (its section in File > Options, run 43856407155: missing on the tablet)
        if (tools::ToolInterface *t = compile::PluginManager::instance().tool(QStringLiteral("translation-proofreading"))) {
            check(toolhost::Host::extension(t) != nullptr, QStringLiteral("the add-on's tool interface 1.1 is reached (its options section, its buttons)"));
            fact(QStringLiteral("A-A01 the add-on's options page"), t->optionsPage());
        }
        // File > Options, the add-on's section: the model from a folder
        bool took = false;
        withModal<QDialog>(1200, [&took, models](QDialog *d) {
            QPushButton *dl = d->findChild<QPushButton *>(QStringLiteral("tpDownload:opus-mt-de-en"));
            if (dl && dl->parentWidget()) {
                for (QPushButton *b : dl->parentWidget()->findChildren<QPushButton *>())
                    if (b->text().startsWith(QLatin1String("From Folder"))) {
                        fileDialogLater(400, models);
                        b->click();
                        settle(15000);
                        took = true;
                        break;
                    }
            }
            shot(d, QStringLiteral("addon-options"));
            d->accept();
        });
        toolhost::host().showOptions(QStringLiteral("translation-proofreading"));
        settle(1500);
        check(took, QStringLiteral("the model is taken from a folder in the add-on's options"));
        // the window: translate
        win->trigger();
        settle(1500);
        QWidget *panel = nullptr;
        for (QWidget *tl : QApplication::topLevelWidgets())
            if (QWidget *p = tl->findChild<QWidget *>(QStringLiteral("tpWindowPanel")))
                panel = p;
        check(panel != nullptr, QStringLiteral("the add-on's window opens"));
        if (panel) {
            // (Android 0.93.2: since 1.3 the add-on translates both ways; the model given is German to English, and
            // the last "Translate ..." entry, English to German, had none: run 45557268944)
            if (auto *task = panel->findChild<QComboBox *>(QStringLiteral("tpTask"))) {
                int pick = -1;
                for (int i = 0; i < task->count(); ++i)
                    if (task->itemText(i).contains(QLatin1String("German to English")))
                        pick = i;
                for (int i = 0; pick < 0 && i < task->count(); ++i)
                    if (task->itemText(i).contains(QLatin1String("ranslat")))
                        pick = i;
                if (pick >= 0)
                    task->setCurrentIndex(pick);
            }
            auto *in = panel->findChild<QPlainTextEdit *>(QStringLiteral("tpInput"));
            auto *out = panel->findChild<QTextEdit *>(QStringLiteral("tpOutput"));
            auto *go = panel->findChild<QAbstractButton *>(QStringLiteral("tpGo"));
            if (in && out && go) {
                in->setPlainText(QStringLiteral("Das Haus ist alt und das Wetter ist schön."));
                QElapsedTimer t;
                t.start();
                go->click();
                waitFor([out] { return !out->toPlainText().trimmed().isEmpty(); }, 180000);
                const QString got = out->toPlainText().simplified();
                fact(QStringLiteral("A-A01 the translation on the tablet"), QStringLiteral("%1 (%2 s)").arg(got.left(160)).arg(t.elapsed() / 1000));
                check(got.contains(QLatin1String("house"), Qt::CaseInsensitive) && got.contains(QLatin1String("weather"), Qt::CaseInsensitive),
                      QStringLiteral("the add-on translates a German sentence on the tablet (OPUS-MT, the engine with Ruy)"), got.left(200));
            } else {
                check(false, QStringLiteral("the add-on's window has its input, output and button"));
            }
            shot(panel->window(), QStringLiteral("addon-window"));
            // Known program defect (found here 2026-11-16 with valgrind, recorded for a Linux job): the main window's
            // undo watcher (MainWindowUndo.cpp) keeps a "destroyed" connection to the field that had the keyboard last;
            // when that field is in the add-on's panel, it goes away after the watcher's timer in ~MainWindow, and the
            // timer is started after it was freed (heap damage, "corrupted double-linked list" at the end). The case
            // gives the keyboard back to the writer's text first, as a writer does, so that the rest of the run is not
            // hit by it; the fact says so.
            if (QWidget *f = QApplication::focusWidget(); f && panel->isAncestorOf(f)) {
                fact(QStringLiteral("A-A01 known defect"), QStringLiteral("the keyboard was in the add-on's panel at the end (Linux job: ~MainWindow, undo watcher)"));
                TextEditor *back = nullptr;
                for (TextEditor *e : w->findChildren<TextEditor *>())
                    if (e->isVisible() && !back)
                        back = e;
                w->activateWindow(); // (without a window manager a widget gets the keyboard only in the active window)
                settle(50);
                if (back)
                    back->setFocus(Qt::OtherFocusReason);
                else
                    w->setFocus(Qt::OtherFocusReason);
                settle(100);
                QWidget *now = QApplication::focusWidget();
                check(!now || !panel->isAncestorOf(now), QStringLiteral("the keyboard is back in the writer's text"),
                      now ? now->metaObject()->className() : "none");
            }
            if (panel->window() != w)
                panel->window()->close();
        }
        finishProject(w);
        dropWindow(w);
        return;
    }
    // ---- the package for this tablet: looked at, installed
    const QString pkg = packageForThisTablet();
    if (pkg.isEmpty()) {
        if (onTablet())
            check(false, QStringLiteral("the translation add-on's package for this tablet is there (the script puts it into source/addons)"),
                  ctx().sourceDir + QStringLiteral("/addons"));
        else
            note(QStringLiteral("(no package of the translation add-on for %1 in %2)").arg(platform::packagePlatform(), extraDir() + QStringLiteral("/addons")));
    } else {
        const addons::PackageInfo info = addons::inspect(pkg);
        check(info.error.isEmpty(), QStringLiteral("the add-on for this tablet (%1) fits the program").arg(platform::packagePlatform()), info.error);
        QStringList log;
        QString err;
        bool restart = false;
        const bool ok = addons::install(pkg, &log, &err, &restart);
        check(ok, QStringLiteral("the add-on installs"), err);
        const QString dir = addons::addonsRoot() + QStringLiteral("/translation-proofreading");
        check(QFileInfo::exists(dir + QStringLiteral("/translation-proofreading/libqb-tp-engine.so"))
                  && QFileInfo::exists(dir + QStringLiteral("/plugins/libqb-translation-proofreading.so")),
              QStringLiteral("... its engine and plugin are in the app's private folder"), dir);
        fact(QStringLiteral("A-A01 the install asks for a restart"), restart);
        if (ok)
            subCase(QStringLiteral("loaded"), {{QStringLiteral("QB_ST_KEEP_DATA"), QStringLiteral("1")}});
    }
    // ---- packages for another system or processor, and a program update: refused, each with its reason
    const QString linuxPkg = madePackage(QStringLiteral("linux-x86_64"), QStringLiteral("addon"));
    const QString otherCpu = madePackage(platform::packagePlatform() == QLatin1String("android-arm64") ? QStringLiteral("android-x86_64")
                                                                                                    : QStringLiteral("android-arm64"),
                                         QStringLiteral("addon"));
    const QString core = madePackage(QStringLiteral("android"), QStringLiteral("core"));
    if (onTablet()) {
        const QString e1 = addons::inspect(linuxPkg).error, e2 = addons::inspect(otherCpu).error, e3 = addons::inspect(core).error;
        check(e1.contains(QLatin1String("Android version")), QStringLiteral("an add-on made for Linux is refused: use its Android version"), e1);
        check(e2.contains(QLatin1String("processor")), QStringLiteral("an add-on for the other kind of processor is refused"), e2);
        check(e3.contains(QLatin1String("APK")), QStringLiteral("a program update is refused: the program comes as an APK"), e3);
    } else {
        note(QStringLiteral("(not on a tablet: the refusals are Android's)"));
    }
}

// ================================================================ A-K01 killed while typing, A-H01 put away and ended
namespace {

// the model of the project as saved: written right after each save (and only then), so that the check afterwards
// compares what is on the tablet with what the program had saved when it was killed
void modelAtEverySave(Project *p, int *saves)
{
    QObject::connect(p, &Project::saved, qApp, [p, saves] {
        ++*saves;
        writeModel(p);
    });
}

bool fileThere(const QString &name)
{
    return QFileInfo::exists(ctx().dir + QLatin1Char('/') + name);
}

} // namespace

void caseAK01()
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")));
    Project *p = w->project();
    if (!p)
        return;
    TextEditor *e = showDoc(w, itemByTitle(p, QStringLiteral("Scene 1")));
    if (!e)
        return;
    int saves = 0;
    modelAtEverySave(p, &saves);
    writeModel(p);
    endOf(e);
    const QString text = writerText(ctx().seed, 400);
    // first a part with pauses (saves come between), then the script is told, then typing goes on until the end
    typeInPieces(e, text.left(600), 40, 700);
    check(saves > 0, QStringLiteral("the program saved while the writer typed"), QString::number(saves));
    writeText(ctx().dir + QStringLiteral("/kill-now"), "typing");
    note(QStringLiteral("kill-now written after %1 saves; typing goes on").arg(saves));
    typeInPieces(e, text.mid(600), 20, 150, [] { return false; });
    // still here: no script ended the app (the desktop build), so the case ends as any other
    note(QStringLiteral("(the app was not ended from outside: not on a tablet, or the script did not act)"));
    finishProject(w);
    dropWindow(w);
}

void caseAH01()
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")));
    Project *p = w->project();
    if (!p)
        return;
    TextEditor *e = showDoc(w, itemByTitle(p, QStringLiteral("Scene 2")));
    if (!e)
        return;
    // the model of what the program has when Android puts the app away (after the program's own save on that
    // moment, which was connected earlier): the check afterwards finds the tablet's files equal to it only if that
    // save took every letter typed
    bool away = false;
    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, qApp, [p, &away](Qt::ApplicationState st) {
        if (st != Qt::ApplicationActive && !away) {
            away = true;
            writeModel(p);
        }
    });
    endOf(e);
    // typed fast, with no pause long enough for the program's own autosave: only the save on going away keeps it
    keysInto(e, QStringLiteral(" ") + writerText(ctx().seed + 7, 60).left(300));
    writeText(ctx().dir + QStringLiteral("/home-now"), "typed");
    note(QStringLiteral("home-now written"));
    waitFor([&away] { return away; }, 20000);
    check(away || !onTablet(), QStringLiteral("Android put the app away (Home)"));
    // the script kills the app a few seconds after Home; on the desktop nothing comes
    settle(onTablet() ? 60000 : 500);
    note(QStringLiteral("(the app was not ended from outside: not on a tablet, or the script did not act)"));
    finishProject(w);
    dropWindow(w);
}

} // namespace cs
