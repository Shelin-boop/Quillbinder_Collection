// Scripted run for version 0.93.0 beta: to-do item 25, File > Save As... (in place of the menu Project Format) and the copies
// in File > Export.
//  - (a) the menus: File > Save As... (Ctrl+Shift+S) after Save, no menu Project Format, File > Export > Quillbinder Project...;
//  - (b) the dialog: its file types are the two formats, a name with .scriv or .qbx decides, else the type chosen;
//  - (c) Save As in four ways (own to own, own to .scriv, .scriv to .scriv, .scriv to own), each with changes not saved yet:
//    the window works on the new project, the title follows the name, the changes are in the new project only, the original
//    is unchanged byte by byte, the new project has an ID of its own, the lock moves, Open Recent names the new project,
//    later saves go to the new place;
//  - (d) backups: the new project starts its own series, the old one's stay;
//  - (e) a name that exists is refused; a canceled dialog changes nothing; Export > Quillbinder Project... leaves the window on
//    the original.
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_saveas_tests /tmp/shots
#include "core/Backup.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "import/ScrivenerFormat.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"

#include <QAction>
#include <QAbstractButton>
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSet>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTimer>

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

static void info(const QString &t)
{
    std::printf("INFO  %s\n", qPrintable(t));
    std::fflush(stdout);
}

static void settle(int ms) { QTest::qWait(ms); }

static void shot(QWidget *w, const QString &name)
{
    settle(120);
    w->grab().save(outDir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

static QString plain(QString s)
{
    s.remove(QLatin1Char('&'));
    if (s.contains(QLatin1Char('\t')))
        s = s.left(s.indexOf(QLatin1Char('\t')));
    return s.remove(QStringLiteral("…")).remove(QStringLiteral("...")).trimmed();
}

static QAction *menuPath(QMainWindow *w, const QStringList &path)
{
    QList<QAction *> level = w->menuBar()->actions();
    QAction *found = nullptr;
    for (int i = 0; i < path.size(); ++i) {
        found = nullptr;
        for (QAction *a : level)
            if (plain(a->text()) == path[i]) {
                found = a;
                break;
            }
        if (!found)
            return nullptr;
        if (i + 1 < path.size()) {
            if (!found->menu())
                return nullptr;
            emit found->menu()->aboutToShow();
            level = found->menu()->actions();
        }
    }
    return found;
}

static QAction *byKey(QWidget *w, const char *key) { return w->findChild<QAction *>(QLatin1String(key)); }

// Runs fn on the modal window of type T that is there within delayMs
template<typename T>
static void withModal(int delayMs, std::function<void(T *)> fn)
{
    QTimer::singleShot(delayMs, qApp, [fn] {
        if (auto *m = qobject_cast<T *>(QApplication::activeModalWidget())) {
            fn(m);
            return;
        }
        for (QWidget *tl : QApplication::topLevelWidgets())
            if (auto *d = qobject_cast<T *>(tl))
                if (d->isVisible()) {
                    fn(d);
                    return;
                }
        info(QStringLiteral("(the expected window did not open)"));
        if (QWidget *m = QApplication::activeModalWidget())
            m->close();
    });
}

static QString g_filters;            // the file types the last Save As dialog offered
static QString g_dialogShot;         // a screenshot name for the next dialog
static QString g_filterToSelect;     // the file type to choose before accepting (a part of its name)

// a save dialog of Qt: name it and accept
static void saveDialog(int delayMs, const QString &path)
{
    withModal<QFileDialog>(delayMs, [path](QFileDialog *d) {
        g_filters = d->nameFilters().join(QLatin1String(" | "));
        auto *name = d->findChild<QLineEdit *>(QStringLiteral("fileNameEdit"));
        if (d->fileMode() == QFileDialog::Directory) {
            d->setDirectory(path);
            if (name)
                name->setText(path);
        } else {
            d->setDirectory(QFileInfo(path).absolutePath());
            if (!g_filterToSelect.isEmpty())
                for (const QString &f : d->nameFilters())
                    if (f.contains(g_filterToSelect))
                        d->selectNameFilter(f);
            if (name)
                name->setText(QFileInfo(path).fileName());
            else
                d->selectFile(QFileInfo(path).fileName());
        }
        if (!g_dialogShot.isEmpty())
            shot(d, g_dialogShot);
        g_dialogShot.clear();
        g_filterToSelect.clear();
        QMetaObject::invokeMethod(d, "accept", Qt::DirectConnection);
    });
}

// a message box: its text is kept and OK clicked
static void answerBox(int delayMs, QString *seen)
{
    withModal<QMessageBox>(delayMs, [seen](QMessageBox *b) {
        if (seen)
            *seen = b->text();
        b->accept();
    });
}

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// every file of a project (a folder), by its relative name, with its hash; the settings folder and the locks are left out
// (the window layout and the lock are written and removed by the window's own life)
static QMap<QString, QByteArray> snapshot(const QString &folder)
{
    QMap<QString, QByteArray> m;
    const QDir root(folder);
    QDirIterator it(folder, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QString rel = root.relativeFilePath(path);
        if (rel.startsWith(QLatin1String("settings/")) || rel.endsWith(QLatin1String("quillbinder.lock")))
            continue;
        m.insert(rel, QCryptographicHash::hash(readAll(path), QCryptographicHash::Sha1));
    }
    return m;
}

static QString whatDiffers(const QMap<QString, QByteArray> &a, const QMap<QString, QByteArray> &b)
{
    QStringList d;
    for (auto it = a.cbegin(); it != a.cend(); ++it)
        if (!b.contains(it.key()))
            d << QStringLiteral("gone: ") + it.key();
        else if (b.value(it.key()) != it.value())
            d << QStringLiteral("changed: ") + it.key();
    for (auto it = b.cbegin(); it != b.cend(); ++it)
        if (!a.contains(it.key()))
            d << QStringLiteral("new: ") + it.key();
    return d.join(QLatin1String(", "));
}

// does a file below the folder hold the bytes?
static bool holds(const QString &folder, const QByteArray &needle)
{
    QDirIterator it(folder, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (it.hasNext())
        if (readAll(it.next()).contains(needle))
            return true;
    return false;
}

static QString placeOf(const Project *p) { return p->scrivenerPath.isEmpty() ? p->folderPath() : p->scrivenerPath; }

static QStringList lockFiles(const Project *p)
{
    QStringList l{p->settingsDir() + QStringLiteral("/lock")};
    if (!p->scrivenerPath.isEmpty())
        l << p->scrivenerPath + QStringLiteral("/quillbinder.lock");
    return l;
}

static bool anyLock(const QStringList &l)
{
    for (const QString &f : l)
        if (QFile::exists(f))
            return true;
    return false;
}

static QSet<QString> g_ids;

// Changes the open project without saving: a word in a text, and a new document with a text
static void editUnsaved(MainWindow &w, const QByteArray &marker)
{
    Project *p = w.project();
    BinderItem *first = nullptr;
    for (BinderItem *x : p->allItems())
        if (x->hasText() && !p->isInTrash(x)) {
            first = x;
            break;
        }
    if (first)
        QTextCursor(p->docs()->text(first->id)).insertText(QString::fromLatin1(" " + marker + "Word"));
    BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QString::fromLatin1("New " + marker));
    QTextCursor(p->docs()->text(it->id)).insertText(QString::fromLatin1("Text of " + marker + "Doc."));
}

// File > Save As to `file`, with changes not saved yet; the checks that go with every Save As
static void saveAsStep(MainWindow &w, const QString &what, const QString &file, bool toScriv, const QByteArray &marker)
{
    Project *old = w.project();
    const bool oldScriv = !old->scrivenerPath.isEmpty();
    const QString oldPlace = placeOf(old);
    const QString oldId = backup::projectId(old);
    g_ids.insert(oldId);
    const QMap<QString, QByteArray> before = snapshot(oldPlace);
    const QStringList oldLocks = lockFiles(old);
    const bool hadLock = anyLock(oldLocks);
    editUnsaved(w, marker);
    check(old->hasUnsavedChanges(), what + QStringLiteral(": there are changes not saved before"));
    saveDialog(500, file);
    byKey(&w, "saveAs")->trigger();
    settle(3000);
    Project *p = w.project();
    const QString name = QFileInfo(file).completeBaseName();
    const QString wantPlace = toScriv ? QFileInfo(file).absoluteFilePath()
                                      : QFileInfo(QDir(QFileInfo(file).absolutePath()).filePath(name + QStringLiteral(".quillbinder"))).absoluteFilePath();
    check(p && p != old && placeOf(p) == wantPlace && p->scrivenerPath.isEmpty() == !toScriv,
          what + QStringLiteral(": the window works on the new project"), p ? placeOf(p) : QStringLiteral("none"));
    if (!p || placeOf(p) != wantPlace)
        return;
    check(p->title() == name, what + QStringLiteral(": the title follows the name"), p->title());
    check(whatDiffers(before, snapshot(oldPlace)).isEmpty(), what + QStringLiteral(": the original is unchanged byte by byte"),
          whatDiffers(before, snapshot(oldPlace)));
    check(holds(placeOf(p), marker + "Word") && holds(placeOf(p), marker + "Doc.") && !holds(oldPlace, marker + "Word")
              && !holds(oldPlace, marker + "Doc."),
          what + QStringLiteral(": the changes not saved are in the new project only"));
    check(!p->hasUnsavedChanges(), what + QStringLiteral(": nothing is waiting to be saved in the new project"));
    const QString id = backup::projectId(p);
    check(!g_ids.contains(id), what + QStringLiteral(": the new project has an ID of its own"), id + QStringLiteral(" / ") + g_ids.values().join(QLatin1String(",")));
    g_ids.insert(id);
    check(hadLock && !anyLock(oldLocks) && anyLock(lockFiles(p)), what + QStringLiteral(": the lock moved to the new place"),
          QStringLiteral("had %1, old now %2, new now %3").arg(hadLock).arg(anyLock(oldLocks)).arg(anyLock(lockFiles(p))));
    check(AppSettings::instance().recentProjects.value(0) == p->displayFile(), what + QStringLiteral(": Open Recent names the new project"),
          AppSettings::instance().recentProjects.value(0));
    // later saves go to the new place
    const QMap<QString, QByteArray> oldNow = snapshot(oldPlace);
    editUnsaved(w, marker + "Later");
    QString err;
    const bool saved = p->save(&err);
    check(saved && holds(placeOf(p), marker + "LaterWord") && !holds(oldPlace, marker + "LaterWord")
              && whatDiffers(oldNow, snapshot(oldPlace)).isEmpty(),
          what + QStringLiteral(": a later save goes to the new place and the original stays as it was"), err);
    (void)oldScriv;
}

static QString makeProject(const QString &parent, const QString &name)
{
    QString err;
    auto p = Project::create(parent, name, &err);
    if (!p)
        return QString();
    for (const char *t : {"Harbor", "Lighthouse", "Tide table"}) {
        BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QString::fromLatin1(t));
        QTextCursor(p->docs()->text(it->id)).insertText(QString::fromLatin1("The text of the %1 chapter.").arg(QString::fromLatin1(t)));
    }
    p->save(&err);
    return p->indexPath();
}

static int countFiles(const QString &dir, const QString &prefix)
{
    return QDir(dir).entryList({prefix + QLatin1Char('*')}, QDir::Files).size();
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderSaveAsTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderSaveAsTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    as.autosaveSeconds = 600;
    QTemporaryDir work;
    const QString W = work.path();
    const QString backups = W + QStringLiteral("/backups");
    QDir().mkpath(backups);
    as.backupDir = backups;
    const QString projectFile = makeProject(W + QStringLiteral("/start"), QStringLiteral("Harbor Book"));
    check(!projectFile.isEmpty(), QStringLiteral("a project to work on is made"));
    if (projectFile.isEmpty())
        return 1;
    {
        MainWindow w;
        w.resize(1300, 820);
        w.show();
        settle(400);
        w.openProject(projectFile);
        settle(900);
        check(w.project() != nullptr, QStringLiteral("the project is open"));
        if (!w.project())
            return 1;

        // ---------------------------------------------------------------- (a) the menus
        {
            QAction *file = nullptr;
            for (QAction *a : w.menuBar()->actions())
                if (plain(a->text()) == QLatin1String("File"))
                    file = a;
            QStringList names;
            if (file && file->menu())
                for (QAction *a : file->menu()->actions())
                    if (!a->isSeparator())
                        names << plain(a->text());
            const int save = int(names.indexOf(QStringLiteral("Save")));
            check(save >= 0 && names.value(save + 1) == QLatin1String("Save As") && !names.contains(QStringLiteral("Project Format")),
                  QStringLiteral("File > Save As… follows Save; the menu Project Format is gone"), names.join(QLatin1String(" | ")));
            QAction *sa = menuPath(&w, {QStringLiteral("File"), QStringLiteral("Save As")});
            check(sa && sa->isEnabled() && sa->shortcut() == QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S),
                  QStringLiteral("Save As is on with a project and has Ctrl+Shift+S"));
            int sameKey = 0;
            for (QAction *a : w.findChildren<QAction *>())
                if (a->shortcut() == QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S) && a->parent() == &w)
                    ++sameKey;
            check(sameKey == 1, QStringLiteral("no other command of the window uses Ctrl+Shift+S"), QString::number(sameKey));
            QAction *exp = menuPath(&w, {QStringLiteral("File"), QStringLiteral("Export")});
            QStringList e;
            if (exp && exp->menu())
                for (QAction *a : exp->menu()->actions())
                    e << plain(a->text());
            check(e.contains(QStringLiteral("Scrivener Project")) && e.contains(QStringLiteral("Quillbinder Project")),
                  QStringLiteral("File > Export has Scrivener Project… and Quillbinder Project…"), e.join(QLatin1String(" | ")));
            if (file && file->menu()) {
                file->menu()->popup(w.mapToGlobal(QPoint(20, 30)));
                settle(200);
                if (exp && exp->menu()) {
                    file->menu()->setActiveAction(exp);
                    exp->menu()->popup(file->menu()->mapToGlobal(QPoint(file->menu()->width() - 4, 4)));
                    settle(200);
                    shot(file->menu(), QStringLiteral("saveas-file-menu"));
                    shot(exp->menu(), QStringLiteral("saveas-export-menu"));
                    exp->menu()->hide();
                }
                file->menu()->hide();
            }
        }

        // ---------------------------------------------------------------- (b) the dialog
        {
            const QString here = w.project()->folderPath();
            g_dialogShot = QStringLiteral("saveas-dialog");
            withModal<QFileDialog>(400, [](QFileDialog *d) {
                g_filters = d->nameFilters().join(QLatin1String(" | "));
                shot(d, QStringLiteral("saveas-dialog"));
                d->reject();
            });
            byKey(&w, "saveAs")->trigger();
            settle(900);
            g_dialogShot.clear();
            check(g_filters.contains(QStringLiteral("Scrivener 3 Project")) && g_filters.contains(QStringLiteral(".scriv"))
                      && g_filters.contains(QStringLiteral("Quillbinder Project")) && g_filters.contains(QStringLiteral(".qbx")),
                  QStringLiteral("the save dialog offers the two formats as file types"), g_filters);
            check(w.project() && w.project()->folderPath() == here, QStringLiteral("a canceled Save As changes nothing"));
        }

        // ---------------------------------------------------------------- (d) first: the original has backups
        QString outB;
        QString errB;
        const QString origId = backup::projectId(w.project());
        const bool madeOld = backup::backupProject(w.project(), backups, 5, &outB, &errB);
        const QString oldStart = backup::nameStart(w.project());
        check(madeOld && countFiles(backups, oldStart) == 1, QStringLiteral("the original has a backup"), errB);

        // ---------------------------------------------------------------- (c) four ways
        const QString copies = W + QStringLiteral("/copies");
        QDir().mkpath(copies);
        g_ids.clear();
        saveAsStep(w, QStringLiteral("Save As own to own"), copies + QStringLiteral("/Second Draft.qbx"), false, "Alpha");
        const QString afterOwn = w.project() ? w.project()->folderPath() : QString();
        check(QFileInfo(afterOwn + QStringLiteral("/Second Draft.qbx")).isFile(), QStringLiteral("the new folder holds Second Draft.qbx"));
        // (d) the backups: the new project starts its own series
        {
            Project *p = w.project();
            QString o, e;
            const bool made = p && backup::backupProject(p, backups, 5, &o, &e);
            const QString newStart = p ? backup::nameStart(p) : QString();
            check(made && newStart != oldStart && backup::projectId(p) != origId && countFiles(backups, newStart) == 1
                      && countFiles(backups, oldStart) == 1,
                  QStringLiteral("the new project's backups are its own: a new series, the old one's stay"),
                  oldStart + QStringLiteral(" / ") + newStart + QStringLiteral(" ") + e);
        }
        saveAsStep(w, QStringLiteral("Save As own to Scrivener"), copies + QStringLiteral("/Out One.scriv"), true, "Bravo");
        shot(&w, QStringLiteral("saveas-after-scriv"));
        saveAsStep(w, QStringLiteral("Save As Scrivener to Scrivener"), copies + QStringLiteral("/Out Two.scriv"), true, "Charlie");
        saveAsStep(w, QStringLiteral("Save As Scrivener to own"), copies + QStringLiteral("/Back Again.qbx"), false, "Delta");
        {
            // the way a name decides: a name without an ending takes the file type chosen
            g_filterToSelect = QStringLiteral("Scrivener 3");
            saveDialog(500, copies + QStringLiteral("/Typed Name"));
            byKey(&w, "saveAs")->trigger();
            settle(3000);
            check(w.project() && w.project()->scrivenerPath == QFileInfo(copies + QStringLiteral("/Typed Name.scriv")).absoluteFilePath()
                      && w.project()->title() == QLatin1String("Typed Name"),
                  QStringLiteral("a name without an ending takes the file type chosen in the dialog (Scrivener 3)"),
                  w.project() ? placeOf(w.project()) : QString());
            g_filterToSelect = QStringLiteral("Quillbinder");
            saveDialog(500, copies + QStringLiteral("/Typed Own"));
            byKey(&w, "saveAs")->trigger();
            settle(3000);
            check(w.project() && w.project()->scrivenerPath.isEmpty()
                      && w.project()->folderPath() == QFileInfo(copies + QStringLiteral("/Typed Own.quillbinder")).absoluteFilePath(),
                  QStringLiteral("a name without an ending takes the file type chosen in the dialog (Quillbinder)"),
                  w.project() ? placeOf(w.project()) : QString());
        }

        // ---------------------------------------------------------------- (e) refused names, canceled, Export
        {
            Project *p = w.project();
            const QString place = placeOf(p);
            QString seen;
            // a file with the name: the dialog asks first whether to replace it (Yes), then the program refuses the name
            QFile plainFile(copies + QStringLiteral("/Plain.scriv"));
            plainFile.open(QIODevice::WriteOnly);
            plainFile.close();
            withModal<QMessageBox>(1000, [](QMessageBox *b) {
                if (QAbstractButton *y = b->button(QMessageBox::Yes))
                    y->click();
                else
                    b->accept();
            });
            answerBox(2000, &seen);
            saveDialog(500, copies + QStringLiteral("/Plain.scriv"));
            byKey(&w, "saveAs")->trigger();
            settle(2800);
            check(w.project() == p && placeOf(w.project()) == place && seen.contains(QLatin1String("There is a file or folder with this name already")),
                  QStringLiteral("a name that exists (.scriv) is refused"), seen);
            seen.clear();
            answerBox(1500, &seen);
            saveDialog(500, copies + QStringLiteral("/Second Draft.qbx"));
            byKey(&w, "saveAs")->trigger();
            settle(2200);
            check(w.project() == p && placeOf(w.project()) == place && seen.contains(QLatin1String("There is a file or folder with this name already")),
                  QStringLiteral("a project folder that exists is refused"), seen);
            check(QDir(copies + QStringLiteral("/Second Draft.quillbinder")).exists() && QFileInfo(copies + QStringLiteral("/Second Draft.quillbinder/Second Draft.qbx")).isFile(),
                  QStringLiteral("the project that exists is not touched by the refused name"));

            const QMap<QString, QByteArray> before = snapshot(placeOf(p));
            const QString exportDir = W + QStringLiteral("/exported");
            QDir().mkpath(exportDir);
            editUnsaved(w, "Echo");
            QAction *expOwn = menuPath(&w, {QStringLiteral("File"), QStringLiteral("Export"), QStringLiteral("Quillbinder Project")});
            saveDialog(500, exportDir);
            answerBox(3500, nullptr);
            if (expOwn)
                expOwn->trigger();
            settle(4200);
            const QStringList made = QDir(exportDir).entryList({QStringLiteral("*.quillbinder")}, QDir::Dirs);
            check(w.project() == p && placeOf(w.project()) == place && made.size() == 1,
                  QStringLiteral("File > Export > Quillbinder Project…: a copy in the chosen folder, the window stays on the original"),
                  made.join(QLatin1String(", ")));
            check(!made.isEmpty() && holds(QDir(exportDir).filePath(made.value(0)), "EchoWord"),
                  QStringLiteral("the copy holds the project as it is, with the last changes"));
            (void)before;
            w.close();
        }
        settle(100);
    }
    std::printf("%d failure(s)\n", failures);
    return failures;
}
