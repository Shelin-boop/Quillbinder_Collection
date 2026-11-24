// Scripted run for version 0.93.0 beta, third suite: to-do item 38 (backups and saving: what was not done).
//  - (a) Restore From Backup also lists the backups that wait in the work folder (marked as waiting), of this project
//    only, and restores them as the others;
//  - (b) leftovers of temporary names: Qt's QSaveFile names ("<file>.XXXXXX") in a project are removed at the opening once
//    they are old enough, a ".docx.part" of an export is removed (also after a failed put in place), the speech add-on's
//    ".txt.part" files too;
//  - (c) File > Back Up > Move Old Backups Into the Backup Folder: the backups of the project's own folder of 0.70.0 to
//    0.90.4 go into the backup folder under their names, never over a file that is there; the command is on only when
//    there is something to move;
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0930c_tests /tmp/shots
#include "compile/DocxWriter.h"
#include "compile/ExporterApi.h"
#include "core/Backup.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Paths.h"
#include "core/Project.h"
#include "core/SaveBatch.h"
#include "core/Zip.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "tools/speech-transcription/SpeechLogic.h"

#include "AppInfo.h"

#include <QApplication>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QTextCursor>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
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
            std::printf("      %s\n", qPrintable(detail.left(3000)));
    }
    std::fflush(stdout);
}

static void settle(int ms)
{
    QTest::qWait(ms);
}

static void writeAll(const QString &f, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(f).absolutePath());
    QFile file(f);
    if (file.open(QIODevice::WriteOnly))
        file.write(data);
}

static QByteArray readAll(const QString &f)
{
    QFile file(f);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

static void shot(QWidget *w, const QString &name)
{
    if (!w)
        return;
    settle(80);
    w->grab().save(outDir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

static void makeOld(const QString &f, int secondsAgo)
{
    QFile file(f);
    if (file.open(QIODevice::ReadWrite))
        file.setFileTime(QDateTime::currentDateTime().addSecs(-secondsAgo), QFileDevice::FileModificationTime);
}

// A project with a text, and one whole backup of it made outside the backup folder (a ZIP to copy under other names).
struct Fixture {
    std::unique_ptr<Project> p;
    QString folder, zip, index;
};

static Fixture fixture(const QString &W, const QString &name, const QString &title)
{
    Fixture f;
    QString err;
    f.folder = W + QLatin1Char('/') + name;
    QDir().mkpath(f.folder);
    f.p = Project::create(f.folder, title, &err);
    f.folder = f.p->folderPath();
    f.index = f.p->indexPath();
    BinderItem *it = f.p->model()->createItem(f.p->draft(), -1, ItemType::Text, QStringLiteral("Scene"));
    QTextCursor c(f.p->docs()->text(it->id));
    c.insertText(QStringLiteral("The text of the scene."));
    f.p->save(&err);
    QString why;
    backup::backupProject(f.p.get(), W + QLatin1Char('/') + name + QStringLiteral("-seed"), 0, &f.zip, &why, backup::Where::AsChosen);
    return f;
}

// Answers the modal box that is open: the button with this text, or the default one
static void answer(const QString &objectName, const QString &button, const QString &shotName, QString *text = nullptr)
{
    QTimer::singleShot(250, qApp, [=] {
        QWidget *m = QApplication::activeModalWidget();
        if (!m || m->objectName() != objectName)
            return;
        if (text)
            if (auto *box = qobject_cast<QMessageBox *>(m))
                *text = box->text();
        shot(m, shotName);
        if (auto *box = qobject_cast<QMessageBox *>(m)) {
            for (QAbstractButton *b : box->buttons())
                if (b->text() == button) {
                    b->click();
                    return;
                }
        }
        m->close();
    });
}

// ------------------------------------------------------------------ (a) the restore list and waiting backups

static void restoreList(const QString &W)
{
    std::printf("== Restore From Backup lists backups that wait (item 38 a)\n");
    Fixture a = fixture(W, QStringLiteral("pa"), QStringLiteral("Waiting Book"));
    Fixture other = fixture(W, QStringLiteral("pb"), QStringLiteral("Another Book"));
    const QString dir = W + QStringLiteral("/bk-wait");
    QDir().mkpath(dir);
    const QString work = backup::workFolderFor(dir);
    QDir().mkpath(work);
    // one backup in the folder, one waiting of this project, one waiting of another project
    QString name = backup::nameStart(a.p.get());
    QFile::copy(a.zip, dir + QLatin1Char('/') + name + QStringLiteral(" bak 2026-11-20 10-00-00.zip"));
    QFile::copy(a.zip, work + QLatin1Char('/') + name + QStringLiteral(" bak 2026-11-22 09-30-00.zip"));
    QFile::copy(a.zip, work + QLatin1Char('/') + name + QStringLiteral(" bak 2026-11-22 09-40-00 (incomplete).zip"));
    QFile::copy(other.zip, work + QLatin1Char('/') + backup::nameStart(other.p.get()) + QStringLiteral(" bak 2026-11-22 09-35-00.zip"));
    writeAll(work + QStringLiteral("/target.txt"), dir.toUtf8() + '\n');
    writeAll(work + QStringLiteral("/leftover.zip.part"), "x");
    const QStringList waiting = backup::waitingBackupsFor(a.p.get(), dir);
    check(waiting.size() == 2 && waiting.at(0).contains(QLatin1String("09-30-00")) && !waiting.join(QLatin1Char(' ')).contains(QLatin1String("Another"))
              && backup::backupsFor(a.p.get(), dir).size() == 1,
          QStringLiteral("the waiting backups of this project are found (oldest first), not those of another project or other files"),
          waiting.join(QLatin1String(" | ")));

    AppSettings::instance().backupDir = dir;
    MainWindow w;
    w.show();
    settle(200);
    w.openProject(a.folder);
    settle(400);
    AppSettings::instance().backupDir = dir; // the opening applies the project's own settings: set after it
    QStringList rows;
    int waitingRows = 0;
    bool restorable = false;
    QTimer::singleShot(400, qApp, [&] {
        QWidget *m = QApplication::activeModalWidget();
        if (auto *list = m ? m->findChild<QTreeWidget *>(QStringLiteral("restoreBackupList")) : nullptr) {
            for (int i = 0; i < list->topLevelItemCount(); ++i) {
                rows << list->topLevelItem(i)->text(2);
                if (list->topLevelItem(i)->text(2).contains(QLatin1String("waiting")))
                    ++waitingRows;
            }
            // the waiting row is chosen like any other: its path is the work folder's file
            for (int i = 0; i < list->topLevelItemCount(); ++i)
                if (list->topLevelItem(i)->text(2).contains(QLatin1String("waiting")))
                    restorable = list->topLevelItem(i)->data(0, Qt::UserRole).toString().startsWith(work);
            list->setCurrentItem(list->topLevelItem(list->topLevelItemCount() - 1));
            shot(m, QStringLiteral("restore-list-waiting"));
            m->close();
        }
    });
    w.restoreBackup();
    check(rows.size() == 3 && waitingRows == 2 && restorable && rows.join(QLatin1Char('|')).contains(QLatin1String("incomplete")),
          QStringLiteral("the restore list shows the folder's backup and the two waiting ones, marked as waiting"), rows.join(QLatin1String(" | ")) + QStringLiteral(" waitingRows %1 restorable %2").arg(waitingRows).arg(restorable));
    // restored the same way
    const QString waitingFile = waiting.first();
    const bool ok = w.restoreBackupFile(waitingFile);
    settle(500);
    check(ok && w.project() && w.project()->folderPath() != a.folder && w.project()->title().contains(QLatin1String("(restored")),
          QStringLiteral("a waiting backup is restored and opened like any other"), w.lastRestoreText());
    w.closeProject();
    settle(200);
}

// ------------------------------------------------------------------ (c) the command that moves the old backups

static QAction *moveAction(MainWindow &w)
{
    QAction *a = w.findChild<QAction *>(QStringLiteral("moveOldBackups"));
    for (QMenu *m : w.findChildren<QMenu *>())
        if (m->title().remove(QLatin1Char('&')) == QLatin1String("Back Up")) {
            QMetaObject::invokeMethod(m, "aboutToShow", Qt::DirectConnection);
        }
    return a;
}

static void moveOld(const QString &W)
{
    std::printf("== Move Old Backups Into the Backup Folder (item 38 c)\n");
    Fixture a = fixture(W, QStringLiteral("pc"), QStringLiteral("Old Book"));
    const QString dir = W + QStringLiteral("/bk-old");
    QDir().mkpath(dir);
    const QString former = dir + QStringLiteral("/Old Book ") + backup::projectKey(a.p.get());
    QDir().mkpath(former);
    const QByteArray zipBytes = readAll(a.zip);
    const QStringList names = {QStringLiteral("Old Book bak 2026-08-01 10-00-00.zip"), QStringLiteral("Old Book bak 2026-09-01 10-00-00.zip"),
                               QStringLiteral("Old Book bak 2026-10-01 10-00-00.zip")};
    for (const QString &n : names)
        writeAll(former + QLatin1Char('/') + n, zipBytes);
    // a backup of this name is in the backup folder already (another content): it must not be overwritten
    writeAll(dir + QLatin1Char('/') + names.at(1), "an existing backup");
    // a file that is no backup stays too (and keeps the folder)
    writeAll(former + QStringLiteral("/notes.txt"), "mine");

    AppSettings::instance().backupDir = dir;
    MainWindow w;
    w.show();
    settle(200);
    QAction *act = moveAction(w);
    check(act && !act->isEnabled(), QStringLiteral("without a project the command is off"));
    w.openProject(a.folder);
    settle(400);
    AppSettings::instance().backupDir = dir; // the opening applies the project's own settings: set after it
    act = moveAction(w);
    check(act && act->isEnabled(), QStringLiteral("with old backups in the project's own folder the command is on"),
          QStringLiteral("former %1 / %2 / project %3").arg(backup::formerBackupsFor(w.project(), dir).size()).arg(backup::formerProjectFolder(w.project(), dir), w.project() ? w.project()->folderPath() : QString()));
    QString asked, told;
    answer(QStringLiteral("moveOldBackupsAsk"), QStringLiteral("Move"), QStringLiteral("move-old-backups-ask"), &asked);
    QTimer::singleShot(900, qApp, [&] {
        QWidget *m = QApplication::activeModalWidget();
        if (m && m->objectName() == QLatin1String("moveOldBackupsReport")) {
            if (auto *box = qobject_cast<QMessageBox *>(m))
                told = box->text();
            shot(m, QStringLiteral("move-old-backups-report"));
            m->close();
        }
    });
    w.moveOldBackups();
    settle(300);
    check(asked.contains(QLatin1String("3 old backup")), QStringLiteral("the command asks first and says how many"), asked);
    check(QFile::exists(dir + QLatin1Char('/') + names.at(0)) && QFile::exists(dir + QLatin1Char('/') + names.at(2))
              && !QFile::exists(former + QLatin1Char('/') + names.at(0)) && !QFile::exists(former + QLatin1Char('/') + names.at(2))
              && readAll(dir + QLatin1Char('/') + names.at(0)) == zipBytes,
          QStringLiteral("the old backups are in the backup folder under their names, whole, and gone from the old folder"));
    check(readAll(dir + QLatin1Char('/') + names.at(1)) == QByteArrayLiteral("an existing backup")
              && QFile::exists(former + QLatin1Char('/') + names.at(1)),
          QStringLiteral("a name that is already in the backup folder is never overwritten: the old backup stays where it was"));
    check(told.contains(QLatin1String("2 backup(s) moved")) || told.contains(QLatin1String("2 backups moved")),
          QStringLiteral("the report tells what was moved"), told);
    check(told.contains(names.at(1)) && told.contains(QLatin1String("left")) && QFile::exists(former + QStringLiteral("/notes.txt"))
              && QDir(former).exists(),
          QStringLiteral("the report names what was left and why; the folder stays while something is in it"), told);
    act = moveAction(w);
    check(act && act->isEnabled(), QStringLiteral("the command stays on while an old backup is left"));
    // after taking the blocker away: the rest moves and the empty folder goes
    QFile::remove(dir + QLatin1Char('/') + names.at(1));
    QFile::remove(former + QStringLiteral("/notes.txt"));
    const backup::MoveReport r = backup::moveFormerBackups(w.project(), dir);
    check(r.moved.size() == 1 && r.left.isEmpty() && r.folderRemoved && !QDir(former).exists() && QFile::exists(dir + QLatin1Char('/') + names.at(1)),
          QStringLiteral("the rest is moved and the empty old folder is removed"), r.left.join(QLatin1Char(' ')));
    act = moveAction(w);
    check(act && !act->isEnabled(), QStringLiteral("with nothing left to move the command is off"));
    w.closeProject();
    settle(200);
    act = moveAction(w);
    check(act && !act->isEnabled(), QStringLiteral("after the project is closed the command is off"));
}

// ------------------------------------------------------------------ (b) leftovers

static void leftovers(const QString &W)
{
    std::printf("== Temporary names that a crash can leave (item 38 b)\n");
    Fixture a = fixture(W, QStringLiteral("pd"), QStringLiteral("Leftover Book"));
    const QString folder = a.folder, index = a.index;
    a.p.reset();
    const QString oldJson = folder + QStringLiteral("/settings/ui-x.json.AbC123");
    const QString oldRtf = folder + QStringLiteral("/Files/Data/abc/content.rtf.Zz9Yy8");
    const QString youngJson = folder + QStringLiteral("/settings/ui-y.json.AbC123");
    const QString ownFile = folder + QStringLiteral("/Files/Data/abc/notes.txt.copy1"); // not six characters: not ours
    const QString userFile = folder + QStringLiteral("/Files/Data/abc/photo.png.AbC123");
    for (const QString &f : {oldJson, oldRtf, youngJson, ownFile, userFile})
        writeAll(f, "x");
    for (const QString &f : {oldJson, oldRtf, ownFile, userFile})
        makeOld(f, 3600);
    QString err;
    auto p = Project::open(index, &err);
    check(p != nullptr, QStringLiteral("the project opens"), err);
    check(!QFile::exists(oldJson) && !QFile::exists(oldRtf) && QFile::exists(youngJson) && QFile::exists(ownFile) && QFile::exists(userFile),
          QStringLiteral("the opening removes QSaveFile's old temporary files (not a young one, not other names, not media)"));
    p.reset();

    // the Word writer: no ".part" stays, also when it cannot be put in place; an old one beside the target goes
    const QString out = W + QStringLiteral("/docx");
    QDir().mkpath(out);
    writeAll(out + QStringLiteral("/old-run.docx.part"), "cut off");
    makeOld(out + QStringLiteral("/old-run.docx.part"), 7200);
    writeAll(out + QStringLiteral("/young-run.docx.part"), "running");
    compile::Manuscript m;
    compile::DocxExporter dx;
    QString e;
    const bool ok = dx.exportManuscript(m, out + QStringLiteral("/book.docx"), &e);
    check(ok && QFile::exists(out + QStringLiteral("/book.docx")) && !QFile::exists(out + QStringLiteral("/book.docx.part"))
              && !QFile::exists(out + QStringLiteral("/old-run.docx.part")) && QFile::exists(out + QStringLiteral("/young-run.docx.part")),
          QStringLiteral("a Word export leaves no .part file, and removes an old one of a cut-off export beside it"), e);
    // the target is a folder: it cannot be replaced; the old target stays and no .part is left
    QDir().mkpath(out + QStringLiteral("/blocked.docx/inside"));
    const bool bad = dx.exportManuscript(m, out + QStringLiteral("/blocked.docx"), &e);
    check(!bad && !QFile::exists(out + QStringLiteral("/blocked.docx.part")) && QDir(out + QStringLiteral("/blocked.docx/inside")).exists(),
          QStringLiteral("a Word export that cannot be put in place leaves its .part file removed and the target as it was"), e);

    // the speech add-on's text that was cut off
    const QString sp = W + QStringLiteral("/speech");
    writeAll(sp + QStringLiteral("/2026-11-01 10-00.txt.part"), "half");
    makeOld(sp + QStringLiteral("/2026-11-01 10-00.txt.part"), 7200);
    writeAll(sp + QStringLiteral("/2026-11-23 10-00.txt.part"), "running");
    writeAll(sp + QStringLiteral("/2026-11-02 10-00.txt"), "whole");
    makeOld(sp + QStringLiteral("/2026-11-02 10-00.txt"), 7200);
    const int n = speech::removeLeftoverParts(sp);
    check(n == 1 && !QFile::exists(sp + QStringLiteral("/2026-11-01 10-00.txt.part")) && QFile::exists(sp + QStringLiteral("/2026-11-23 10-00.txt.part"))
              && QFile::exists(sp + QStringLiteral("/2026-11-02 10-00.txt")),
          QStringLiteral("the speech add-on removes the old .txt.part of a cut-off transcription, not a young one, not a text"));
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
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0930cTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0930cTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    QTemporaryDir work;
    const QString W = work.path();
    restoreList(W);
    moveOld(W);
    leftovers(W);
    std::printf("%d failure(s)\n", failures);
    return failures;
}
