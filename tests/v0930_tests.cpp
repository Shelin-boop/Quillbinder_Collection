// Scripted run for version 0.93.0 beta: three fixes in saving and backups (found by the audit of 0.92.0's saving code).
//  - the emergency copy (item 12): whole or said to be incomplete ("n of m", the files named with the reason), made also
//    when only the binder or the objects changed, holding the project file as it is in memory, never into a folder that
//    is there already; the way back in the copy's README; the notice of an ending by a signal says the same;
//  - saving (item 13): a folder's sync that the drive answers with an error (tests/faultshim.cpp, child runs) keeps the
//    journal and tells the save as failed, the next opening finishes it; a file system that cannot sync is no error;
//    an unreadable journal that cannot be set aside stops the saves (they wait and name the file);
//  - backups (item 14): the note of a backup that a signal left undone stays until the opening's backup is made; an
//    older backup that cannot be deleted is told once; Restore From Backup says restored, restored incompletely, or
//    not restored, and a title that could not be saved.
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0930_tests /tmp/shots
#include "core/Backup.h"
#include "core/BinderItem.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Objects.h"
#include "core/Paths.h"
#include "core/Project.h"
#include "core/SaveBatch.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"

#include "AppInfo.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QStatusBar>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
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

static void shot(QWidget *w, const QString &name)
{
    if (w)
        w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

// A window that comes up (by its object name) is saved as a picture and answered with its escape button.
static void shotLater(const QString &objectName, const QString &name, int ms = 400)
{
    QTimer::singleShot(ms, qApp, [objectName, name] {
        for (QWidget *tl : QApplication::topLevelWidgets())
            if (auto *b = qobject_cast<QMessageBox *>(tl); b && b->isVisible() && b->objectName() == objectName) {
                shot(b, name);
                if (QAbstractButton *e = b->escapeButton())
                    e->click();
                else
                    b->accept();
            }
    });
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

static BinderItem *textDoc(Project *p, const QString &title, const QString &text)
{
    BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, title);
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

static QStringList foldersIn(const QString &dir)
{
    return QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
}

static QString textOfFiles(const QString &dir)
{
    QString all;
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        all += QString::fromUtf8(readAll(it.next())) + QLatin1Char('\n');
    return all;
}

// ------------------------------------------------------------------ child run: a save with the drive's answers faked

static int childSave(int argc, char **argv)
{
    qputenv("QB_NO_DIALOGS", "1");
    QApplication app(argc, argv);
    QString err;
    auto p = Project::open(QString::fromLocal8Bit(argv[2]), &err);
    if (!p) {
        std::printf("opened=0 %s\n", qPrintable(err));
        return 0;
    }
    int n = 0;
    for (BinderItem *it : p->allItems())
        if (it->hasText() && n < 2) {
            typeInto(p.get(), it, QStringLiteral(" child%1").arg(++n));
        }
    QString why;
    const bool ok = p->save(&why);
    std::printf("saved=%d journal=%d unsaved=%d\nwhy=%s\n", ok ? 1 : 0, QFile::exists(p->journalFile()) ? 1 : 0,
                p->hasUnsavedChanges() ? 1 : 0, qPrintable(why));
    std::fflush(stdout);
    std::_Exit(0); // no closing
}

static QString runChild(const QString &projectFile, const QString &folder, const QString &mode)
{
    QProcess pr;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("LD_PRELOAD"), QStringLiteral(QB_FAULT_SHIM));
    env.insert(QStringLiteral("QB_SHIM_DIR"), folder);
    env.insert(QStringLiteral("QB_SHIM_MODE"), mode);
    env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    pr.setProcessEnvironment(env);
    pr.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--child-save"), projectFile});
    pr.waitForFinished(120000);
    return QString::fromUtf8(pr.readAllStandardOutput());
}

// ------------------------------------------------------------------ item 12: the emergency copy

static void emergencyCopy(const QString &W)
{
    QString err;
    {
        auto p = Project::create(W, QStringLiteral("Copy"), &err);
        BinderItem *one = textDoc(p.get(), QStringLiteral("One"), QStringLiteral("Text of one."));
        textDoc(p.get(), QStringLiteral("Two"), QStringLiteral("Text of two."));
        textDoc(p.get(), QStringLiteral("Three"), QStringLiteral("Text of three."));
        Q_UNUSED(one);
        const QString parent = W + QStringLiteral("/emergency");
        qputenv("QB_TEST_WRITE_FAILS", "5|Input/output error|/Two.rtf");
        const EmergencyReport r = p->makeEmergencyCopy(parent);
        qunsetenv("QB_TEST_WRITE_FAILS");
        check(!r.dir.isEmpty() && r.tally.wanted == 4 && r.tally.written == 3 && r.tally.missing.size() == 1
                  && r.tally.missing.first().contains(QLatin1String("Two.rtf")) && r.tally.missing.first().contains(QLatin1String("Input/output error"))
                  && !r.complete(),
              QStringLiteral("a copy of three texts (and the project file) in which the second cannot be written: 3 of 4, and the file is named "
                             "with the reason"),
              QStringLiteral("wanted %1 written %2 missing %3").arg(r.tally.wanted).arg(r.tally.written).arg(r.tally.missing.join(QLatin1Char('|'))));
        const QString text = Project::emergencyText(r);
        check(text.contains(QLatin1String("INCOMPLETE")) && text.contains(QLatin1String("3 of 4")) && text.contains(QLatin1String("Two.rtf"))
                  && !text.contains(QLatin1String("is complete")),
              QStringLiteral("the message says incomplete, \"n of m\" and names what is missing; it never says done"), text);
        const QByteArray readme = readAll(r.dir + QStringLiteral("/README.txt"));
        check(readme.startsWith("THIS COPY IS INCOMPLETE: 3 of 4") && readme.contains("Two.rtf"),
              QStringLiteral("the copy's README is headed as incomplete and names the missing file"), QString::fromUtf8(readme.left(300)));
        QString why;
        qputenv("QB_TEST_WRITE_FAILS", "5|Input/output error|/Two.rtf");
        const QString d2 = p->writeEmergencyCopy(parent, &why);
        qunsetenv("QB_TEST_WRITE_FAILS");
        check(!d2.isEmpty() && why.contains(QLatin1String("Two.rtf")), QStringLiteral("writeEmergencyCopy still returns the folder, and its error names the file"), why);

        // a whole copy, and three copies in one second are three folders
        const EmergencyReport a = p->makeEmergencyCopy(parent);
        const EmergencyReport b = p->makeEmergencyCopy(parent);
        check(a.complete() && b.complete() && a.dir != b.dir && foldersIn(parent).size() == 4,
              QStringLiteral("two copies in one second are two folders: an existing folder is never used again"),
              foldersIn(parent).join(QLatin1Char('|')));
        const QByteArray ra = readAll(a.dir + QStringLiteral("/README.txt"));
        check(ra.contains("is complete: all 4 files") && ra.contains("To put the project file back") && ra.contains(QFileInfo(p->indexPath()).fileName().toUtf8()),
              QStringLiteral("a whole copy's README says it is whole and says in two sentences how to put the project file back"),
              QString::fromUtf8(ra));
        const QByteArray idx = readAll(a.dir + QLatin1Char('/') + QFileInfo(p->indexPath()).fileName());
        check(idx.contains("<QuillbinderProject") && idx.contains("<Title>One</Title>") && idx.contains("<Title>Three</Title>"),
              QStringLiteral("the copy holds the project file as it is in memory (the documents not yet saved are in its binder)"));
        const QString ok = Project::emergencyText(a);
        check(ok.contains(QLatin1String("complete: 4 of 4")) && !ok.contains(QLatin1String("INCOMPLETE")), QStringLiteral("a whole copy is told as whole"), ok);
    }
    {
        // the binder alone
        auto p = Project::create(W, QStringLiteral("Binder"), &err);
        BinderItem *it = textDoc(p.get(), QStringLiteral("Chapter"), QStringLiteral("Words."));
        check(p->save(&err) && !p->hasUnsavedChanges(), QStringLiteral("(saved)"), err);
        EmergencyReport none = p->makeEmergencyCopy(W + QStringLiteral("/e2"));
        check(none.dir.isEmpty() && !QDir(W + QStringLiteral("/e2")).exists(QStringLiteral("x")), QStringLiteral("nothing unsaved: no copy"));
        it->title = QStringLiteral("Renamed Chapter");
        p->notifyItemChanged(it);
        check(!p->docs()->hasDirty() && p->hasUnsavedChanges(), QStringLiteral("(only the binder changed)"));
        const EmergencyReport r = p->makeEmergencyCopy(W + QStringLiteral("/e2"));
        const QString idxName = QFileInfo(p->indexPath()).fileName();
        check(r.complete() && r.tally.wanted == 1 && readAll(r.dir + QLatin1Char('/') + idxName).contains("Renamed Chapter"),
              QStringLiteral("a change of the binder alone leaves a copy with the project file (before: no copy)"),
              QStringLiteral("%1 | %2").arg(r.dir, QString::number(r.tally.wanted)));
        // the objects alone
        BinderItem *x = textDoc(p.get(), QStringLiteral("More"), QStringLiteral("More words."));
        Q_UNUSED(x);
        check(p->save(&err), QStringLiteral("(saved again)"), err);
        const auto &cats = p->objects()->schema().categories;
        if (!cats.isEmpty()) {
            const QString oid = p->objects()->create(cats.first().id, QString(), QStringLiteral("Somebody"));
            check(!p->docs()->hasDirty() && p->hasUnsavedChanges() && p->objects()->isDirty(), QStringLiteral("(only an object changed)"));
            const EmergencyReport ro = p->makeEmergencyCopy(W + QStringLiteral("/e3"));
            check(ro.complete() && textOfFiles(ro.dir).contains(QLatin1String("Somebody")) && QFileInfo::exists(ro.dir + QStringLiteral("/objects")),
                  QStringLiteral("a change of the objects alone leaves a copy with the object's file and the project file"), ro.dir);
            Q_UNUSED(oid);
        }
    }
}

// the window: a save that fails, the end by a signal, the messages
static void emergencyWindow(const QString &W)
{
    QString err;
    auto proj = Project::create(W, QStringLiteral("Signal"), &err);
    BinderItem *one = textDoc(proj.get(), QStringLiteral("One"), QStringLiteral("Text of one."));
    BinderItem *two = textDoc(proj.get(), QStringLiteral("Two"), QStringLiteral("Text of two."));
    check(proj->save(&err), QStringLiteral("(saved)"), err);
    Project *p = proj.get();
    const QString indexName = QFileInfo(p->indexPath()).fileName();
    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.move(0, 0);
    w.resize(1100, 700);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    w.adoptProject(std::move(proj));
    settle(500);
    const QString emergencyDir = qb::dataDir() + QStringLiteral("/Emergency");
    QDir(emergencyDir).removeRecursively();

    // a save that fails (the project file cannot be written), the binder alone changed; then the end by a signal
    one->title = QStringLiteral("Binder only title");
    p->notifyItemChanged(one);
    qputenv("QB_TEST_WRITE_FAILS", ("5|Input/output error|" + indexName + ".qbstage").toUtf8());
    QString failed;
    const bool saved = p->save(&failed);
    check(!saved && p->hasUnsavedChanges(), QStringLiteral("(the save fails)"), failed);
    w.saveForTermination();
    qunsetenv("QB_TEST_WRITE_FAILS");
    QString copy;
    for (const QString &f : foldersIn(emergencyDir))
        copy = emergencyDir + QLatin1Char('/') + f;
    check(!copy.isEmpty() && readAll(copy + QLatin1Char('/') + indexName).contains("Binder only title"),
          QStringLiteral("a save that fails after a change of the binder alone, and the end by a signal: a copy with the project file"), copy);
    w.showEmergencyNotice();
    check(w.lastEmergencyNotice().contains(QLatin1String("1 of 1 files")) && w.lastEmergencyNotice().contains(copy),
          QStringLiteral("the notice of the signal's end says how many files and where"), w.lastEmergencyNotice());

    // the notice of an incomplete copy
    typeInto(p, two, QStringLiteral(" more"));
    typeInto(p, one, QStringLiteral(" more"));
    QDir(emergencyDir).removeRecursively();
    qputenv("QB_TEST_WRITE_FAILS", ("5|Input/output error|" + indexName + ".qbstage").toUtf8());
    (void)p->save(&failed);
    qputenv("QB_TEST_WRITE_FAILS", "5|Input/output error|/Two.rtf");
    QDir().mkpath(emergencyDir);
    const EmergencyReport rep = p->makeEmergencyCopy(QString());
    MainWindow::writeEmergencyNotice(QStringLiteral("The project could not be saved: test"), rep);
    qunsetenv("QB_TEST_WRITE_FAILS");
    w.showEmergencyNotice();
    check(!rep.complete() && w.lastEmergencyNotice().contains(QLatin1String("INCOMPLETE")) && w.lastEmergencyNotice().contains(QLatin1String("Two.rtf")),
          QStringLiteral("the notice of an incomplete copy says incomplete and names the file"), w.lastEmergencyNotice());

    // the message window of the three places that make a copy
    shotLater(QStringLiteral("emergencyCopyMessage"), QStringLiteral("v0930-emergency-incomplete"));
    w.tellEmergencyCopy(rep, QStringLiteral("Save Texts Elsewhere"), QStringLiteral("The project stays open."));
    check(w.lastEmergencyCopyText().contains(QLatin1String("INCOMPLETE")) && w.lastEmergencyCopyText().contains(QLatin1String("The project stays open.")),
          QStringLiteral("the window about an incomplete copy"), w.lastEmergencyCopyText());
    typeInto(p, two, QStringLiteral(" and more"));
    const EmergencyReport whole = p->makeEmergencyCopy(W + QStringLiteral("/whole"));
    shotLater(QStringLiteral("emergencyCopyMessage"), QStringLiteral("v0930-emergency-whole"));
    w.tellEmergencyCopy(whole, QStringLiteral("Save Texts Elsewhere"), QStringLiteral("The project stays open."));
    check(whole.complete() && !w.lastEmergencyCopyText().contains(QLatin1String("INCOMPLETE")), QStringLiteral("the window about a whole copy"),
          w.lastEmergencyCopyText());
    p->save(&failed);
    w.closeProject();
    settle(300);
    delete wp;
}

// ------------------------------------------------------------------ item 13: saving

static void saving(const QString &W)
{
    QString err;
    // a project with two documents
    auto make = [&](const QString &name) {
        auto p = Project::create(W, name, &err);
        textDoc(p.get(), QStringLiteral("A"), QStringLiteral("Alpha."));
        textDoc(p.get(), QStringLiteral("B"), QStringLiteral("Beta."));
        p->save(&err);
        return p->indexPath();
    };
    {
        const QString file = make(QStringLiteral("Fsync"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString out = runChild(file, folder, QStringLiteral("dirsync"));
        const QString journal = folder + QStringLiteral("/settings/journal.json");
        check(out.contains(QLatin1String("saved=0")) && out.contains(QLatin1String("unsaved=1")) && out.contains(QLatin1String("drive reported an error"))
                  && QFile::exists(journal),
              QStringLiteral("a folder's sync that the drive answers with EIO: the save is told as failed, the journal is there"), out.left(600));
        auto q = Project::open(file, &err);
        QString all;
        for (const BinderItem *it : q->allItems())
            if (it->hasText())
                all += q->docs()->plainText(it->id) + QLatin1Char('|');
        check(q && !QFile::exists(journal) && all.contains(QLatin1String("child1")) && all.contains(QLatin1String("child2")),
              QStringLiteral("... the next opening finishes the save from the journal (the journal is gone, both texts are there)"), all);
    }
    {
        const QString file = make(QStringLiteral("Notsup"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString out = runChild(file, folder, QStringLiteral("dirsync-notsup"));
        check(out.contains(QLatin1String("saved=1")) && out.contains(QLatin1String("journal=0")) && out.contains(QLatin1String("unsaved=0")),
              QStringLiteral("a file system that cannot sync folders (ENOTSUP) is no error: saved as before"), out.left(600));
    }
    {
        // an unreadable journal that cannot be set aside: no save writes over it
        const QString file = make(QStringLiteral("Stuck"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString journal = folder + QStringLiteral("/settings/journal.json");
        const QByteArray damaged = "{ this journal was cut off";
        writeAll(journal, damaged);
        writeAll(folder + QStringLiteral("/settings/conflicts"), "a file where the folder should be");
        auto q = Project::open(file, &err);
        check(q != nullptr, QStringLiteral("(opened with the damaged journal)"), err);
        if (q) {
            for (BinderItem *it : q->allItems())
                if (it->hasText())
                    typeInto(q.get(), it, QStringLiteral(" new"));
            QString why;
            const bool ok = q->save(&why);
            check(!ok && why.contains(QLatin1String("journal.json")) && readAll(journal) == damaged,
                  QStringLiteral("a damaged journal whose move fails, then a save of two documents: it waits, names the file, and the journal's bytes "
                                 "are unchanged"),
                  why);
            QFile::remove(folder + QStringLiteral("/settings/conflicts"));
            const bool again = q->save(&why);
            check(again && !QFile::exists(journal) && !QDir(folder + QStringLiteral("/settings/conflicts")).isEmpty(),
                  QStringLiteral("... and once the journal can be set aside, the save goes through (the journal is kept in settings/conflicts)"), why);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc > 2 && qstrcmp(argv[1], "--child-save") == 0)
        return childSave(argc, argv);
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0930Test"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0930Test"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    check(Project::compareVersionText(QLatin1String(QB_APP_VERSION), QStringLiteral("0.93.0")) >= 0, QStringLiteral("version 0.93.0 or newer"),
          QStringLiteral(QB_APP_VERSION_SHOWN));
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    QTemporaryDir work;
    const QString W = work.path();
    emergencyCopy(W);
    emergencyWindow(W);
    saving(W);
    std::printf("%d failure(s)\n", failures);
    return failures;
}
