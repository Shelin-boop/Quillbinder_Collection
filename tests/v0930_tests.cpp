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
#include "core/FailureKind.h"
#include "core/Project.h"
#include "core/SaveBatch.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "ui/MindMapDoc.h"

#include "AppInfo.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
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

#ifdef QB_FAULT_SHIM // the fault library is an LD_PRELOAD library: Linux only (as in v0903)
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
    // the fault is switched on after the opening (the fault library acts once this file exists)
    const QString arm = qEnvironmentVariable("QB_SHIM_ARM");
    if (!arm.isEmpty())
        writeAll(arm, "on");
    const bool retry = qEnvironmentVariableIsSet("QB_CHILD_RETRY");
    if (qEnvironmentVariableIsSet("QB_CHILD_CONVERT")) {
        QString why;
        const bool ok = p->docs()->convertStorage(StorageFormat::Rtf, &why);
        const bool disk = readAll(p->indexPath()).contains("storage=\"rtf\"");
        std::printf("convert=%d memoryRtf=%d diskRtf=%d\nwhy=%s\n", ok ? 1 : 0, p->storage == StorageFormat::Rtf ? 1 : 0, disk ? 1 : 0, qPrintable(why));
        std::fflush(stdout);
        std::_Exit(0);
    }
    int conflictsKept = 0;
    QObject::connect(p.get(), &Project::conflictKept, [&conflictsKept] { ++conflictsKept; });
    int n = 0;
    BinderItem *first = nullptr;
    for (BinderItem *it : p->allItems())
        if (it->hasText() && n < 2) {
            if (!first)
                first = it;
            typeInto(p.get(), it, QStringLiteral(" child%1").arg(++n));
        }
    if (retry && first) {
        QTextDocument *doc = p->docs()->text(first->id);
        QImage img(8, 8, QImage::Format_RGB32);
        img.fill(Qt::red);
        doc->addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("data:pasted1")), img);
        QTextCursor c(doc);
        c.movePosition(QTextCursor::End);
        c.insertImage(QStringLiteral("data:pasted1"));
    }
    QString why;
    const bool ok = p->save(&why);
    std::printf("saved=%d journal=%d unsaved=%d\nwhy=%s\n", ok ? 1 : 0, QFile::exists(p->journalFile()) ? 1 : 0,
                p->hasUnsavedChanges() ? 1 : 0, qPrintable(why));
    if (retry && first) {
        typeInto(p.get(), first, QStringLiteral(" again"));
        QString why2;
        const bool second = p->save(&why2); // the fault lasts: never "saved" without a checked sync
        QFile::remove(arm);                 // the fault is gone
        QString why3;
        const bool third = p->save(&why3);
        int images = 0;
        QDirIterator it(p->contentDir(first->id), {QStringLiteral("*.png")}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            ++images;
        }
        const int files = int(QDir(p->settingsDir() + QStringLiteral("/conflicts")).entryList(QDir::Files | QDir::NoDotAndDotDot).size());
        std::printf("second=%d third=%d thirdJournal=%d conflictSignals=%d conflictFiles=%d images=%d unsavedAfter=%d\n", second ? 1 : 0,
                    third ? 1 : 0, QFile::exists(p->journalFile()) ? 1 : 0, conflictsKept, files, images, p->hasUnsavedChanges() ? 1 : 0);
    }
    std::fflush(stdout);
    std::_Exit(0); // no closing
}

static QString runChild(const QString &projectFile, const QString &folder, const QString &mode, bool armed = true, bool retry = false,
                        bool convert = false)
{
    QProcess pr;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("LD_PRELOAD"), QStringLiteral(QB_FAULT_SHIM));
    env.insert(QStringLiteral("QB_SHIM_DIR"), folder);
    env.insert(QStringLiteral("QB_SHIM_MODE"), mode);
    if (armed)
        env.insert(QStringLiteral("QB_SHIM_ARM"), folder + QStringLiteral("/../shim-armed-") + QFileInfo(projectFile).baseName());
    if (retry)
        env.insert(QStringLiteral("QB_CHILD_RETRY"), QStringLiteral("1"));
    if (convert)
        env.insert(QStringLiteral("QB_CHILD_CONVERT"), QStringLiteral("1"));
    env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    pr.setProcessEnvironment(env);
    pr.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--child-save"), projectFile});
    pr.waitForFinished(120000);
    return QString::fromUtf8(pr.readAllStandardOutput());
}

#endif

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
        const QByteArray idx = readAll(a.dir + QLatin1Char('/') + QFileInfo(p->indexPath()).fileName() + QStringLiteral(".copy"));
        check(idx.contains("<QuillbinderProject") && idx.contains("<Title>One</Title>") && idx.contains("<Title>Three</Title>"),
              QStringLiteral("the copy holds the project file as it is in memory (the documents not yet saved are in its binder)"));
        const QString ok = Project::emergencyText(a);
        check(ok.contains(QLatin1String("complete: 4 of 4")) && !ok.contains(QLatin1String("INCOMPLETE")), QStringLiteral("a whole copy is told as whole"), ok);
    }
    {
        // two boards with the same title: both are in the copy, and n of m is true
        auto p = Project::create(W, QStringLiteral("Boards"), &err);
        BinderItem *a = textDoc(p.get(), QStringLiteral("Plot"), QStringLiteral("A."));
        BinderItem *b = textDoc(p.get(), QStringLiteral("Plot"), QStringLiteral("B."));
        check(p->save(&err), QStringLiteral("(saved)"), err);
        Project *pp = p.get();
        pp->emergencyExtra = [pp](const QString &dir, QString *e, CopyTally *t) { return mindmaps::writeUnsaved(pp, dir, e, t); };
        for (BinderItem *it : {a, b}) {
            MindMapDoc *d = mindmaps::docFor(pp, it->id);
            const QString text = it == a ? QStringLiteral("Note of the first board") : QStringLiteral("Note of the second board");
            d->change(QStringLiteral("Add"), [d, text] {
                mindmap::Note n;
                n.id = 1;
                n.text = text;
                d->board.notes << n;
            });
        }
        const EmergencyReport r = pp->makeEmergencyCopy(W + QStringLiteral("/e-boards"));
        const QStringList xml = QDir(r.dir).entryList({QStringLiteral("*.xml")}, QDir::Files);
        const QString all = textOfFiles(r.dir);
        check(r.complete() && xml.size() == 2 && r.tally.wanted == 3 && r.tally.written == 3 && all.contains(QLatin1String("first board"))
                  && all.contains(QLatin1String("second board")),
              QStringLiteral("two boards with the same title: each has its own file in the copy, and \"n of m\" is true"),
              xml.join(QLatin1Char('|')) + QStringLiteral(" %1/%2").arg(r.tally.written).arg(r.tally.wanted));
        mindmaps::closeAll(pp, true);
    }
    {
        // the binder alone
        auto p = Project::create(W, QStringLiteral("Binder"), &err);
        BinderItem *it = textDoc(p.get(), QStringLiteral("Chapter"), QStringLiteral("Words."));
        check(p->save(&err) && !p->hasUnsavedChanges(), QStringLiteral("(saved)"), err);
        EmergencyReport none = p->makeEmergencyCopy(W + QStringLiteral("/e2"));
        check(none.dir.isEmpty() && !QDir(W + QStringLiteral("/e2")).exists(QStringLiteral("x")), QStringLiteral("nothing unsaved: no copy"));
        check(none.nothingToWrite && Project::emergencyText(none).contains(QLatin1String("Everything is saved")),
              QStringLiteral("nothing unsaved is no failure: the text says everything is saved and no copy was needed"), Project::emergencyText(none));
        MainWindow::writeEmergencyNotice(QStringLiteral("x"), none);
        {
            QFile n(qb::dataDir() + QStringLiteral("/Emergency/notice.txt"));
            const bool there = n.open(QIODevice::ReadOnly) && QString::fromUtf8(n.readAll()).contains(QLatin1String("Everything is saved"));
            n.close();
            QFile::remove(n.fileName());
            check(there, QStringLiteral("... and so does the notice (not \"No copy of the texts could be written either\")"));
        }
        it->title = QStringLiteral("Renamed Chapter");
        p->notifyItemChanged(it);
        check(!p->docs()->hasDirty() && p->hasUnsavedChanges(), QStringLiteral("(only the binder changed)"));
        const EmergencyReport r = p->makeEmergencyCopy(W + QStringLiteral("/e2"));
        const QString idxName = QFileInfo(p->indexPath()).fileName();
        check(r.complete() && r.tally.wanted == 1 && readAll(r.dir + QLatin1Char('/') + idxName + QStringLiteral(".copy")).contains("Renamed Chapter") && QDir(r.dir).entryList({QStringLiteral("*.qbx")}).isEmpty(),
              QStringLiteral("a change of the binder alone leaves a copy with the project file (before: no copy)"),
              QStringLiteral("%1 | %2").arg(r.dir, QString::number(r.tally.wanted)));
        {
            // a project in Scrivener format: the README says the file belongs to the working copy
            p->scrivenerPath = W + QStringLiteral("/Some.scriv");
            const EmergencyReport rs = p->makeEmergencyCopy(W + QStringLiteral("/e-scriv"));
            const QByteArray rd = readAll(rs.dir + QStringLiteral("/README.txt"));
            check(rd.contains("Scrivener format") && rd.contains("working copy") && !rd.contains("To put the project file back"),
                  QStringLiteral("the README of a project in Scrivener format does not tell to copy the working copy's file over anything"),
                  QString::fromUtf8(rd));
            p->scrivenerPath.clear();
        }
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
    check(!copy.isEmpty() && readAll(copy + QLatin1Char('/') + indexName + QStringLiteral(".copy")).contains("Binder only title"),
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
#ifdef QB_FAULT_SHIM
    {
        const QString file = make(QStringLiteral("Fsync"));
        const QString folder = QFileInfo(file).absolutePath();
        // the text file of the first document, as it is before the save
        QString target;
        QByteArray oldBytes;
        {
            auto look = Project::open(file, &err);
            for (const BinderItem *it : look->allItems())
                if (it->title == QLatin1String("A"))
                    for (const QString &n : {QStringLiteral("text.md"), QStringLiteral("text.rtf")})
                        if (target.isEmpty() && QFileInfo::exists(look->contentDir(it->id) + QLatin1Char('/') + n))
                            target = look->contentDir(it->id) + QLatin1Char('/') + n;
            oldBytes = readAll(target);
        }
        const QString out = runChild(file, folder, QStringLiteral("dirsync"));
        const QString journal = folder + QStringLiteral("/settings/journal.json");
        check(out.contains(QLatin1String("saved=0")) && out.contains(QLatin1String("unsaved=1")) && out.contains(QLatin1String("drive reported an error"))
                  && QFile::exists(journal),
              QStringLiteral("a folder's sync that the drive answers with EIO: the save is told as failed, the journal is there"), out.left(600));
        // the next opening finishes the save from the journal, for real: the old bytes are back in the target and the new
        // ones wait beside it as the temporary file
        const QByteArray newBytes = readAll(target);
        check(newBytes != oldBytes && newBytes.contains("child"), QStringLiteral("(the new text is in place after the failed sync)"));
        writeAll(target, oldBytes);
        writeAll(target + QStringLiteral(".qbstage"), newBytes);
        auto q = Project::open(file, &err);
        QString all;
        for (const BinderItem *it : q->allItems())
            if (it->hasText())
                all += q->docs()->plainText(it->id) + QLatin1Char('|');
        check(q && !QFile::exists(journal) && !QFile::exists(target + QStringLiteral(".qbstage")) && all.contains(QLatin1String("child1")) && all.contains(QLatin1String("child2")),
              QStringLiteral("... the next opening finishes the save from the journal (the journal is gone, the new text is back, both texts are there)"), all);
        q.reset();
    }
    {
        // S2: a folder's sync that fails before anything is replaced: a clean failure, nothing changed
        const QString file = make(QStringLiteral("Early"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString out = runChild(file, folder, QStringLiteral("dirsync-early"));
        int staged = 0;
        QDirIterator it(folder, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
        while (it.hasNext())
            staged += savebatch::isStaged(it.next()) ? 1 : 0;
        auto q = Project::open(file, &err);
        QString all;
        for (const BinderItem *i : q->allItems())
            if (i->hasText())
                all += q->docs()->plainText(i->id) + QLatin1Char('|');
        check(out.contains(QLatin1String("saved=0")) && out.contains(QLatin1String("Nothing was changed")) && out.contains(QLatin1String("journal=0"))
                  && staged == 0 && !all.contains(QLatin1String("child")),
              QStringLiteral("a folder's sync that fails before anything is replaced is a clean failure: no journal, no temporary file, the texts as before"),
              out.left(500) + all);
    }
    {
        // S1: a storage conversion whose files are all in place when the folder's sync reports an error keeps the new format
        const QString file = make(QStringLiteral("Convert"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString out = runChild(file, folder, QStringLiteral("dirsync"), true, false, true);
        check(out.contains(QLatin1String("convert=0")) && out.contains(QLatin1String("memoryRtf=1")) && out.contains(QLatin1String("diskRtf=1")),
              QStringLiteral("a conversion of the storage format under a folder sync that fails leaves the format in memory matching the project file on disk"),
              out.left(500));
    }
    {
        // a lasting fault: save, retry, save: never "saved" without a checked sync; no false conflicts, no second picture
        const QString file = make(QStringLiteral("Lasting"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString out = runChild(file, folder, QStringLiteral("dirsync"), true, true);
        check(out.contains(QLatin1String("saved=0")) && out.contains(QLatin1String("second=0")) && out.contains(QLatin1String("third=1"))
                  && out.contains(QLatin1String("thirdJournal=0")) && out.contains(QLatin1String("unsavedAfter=0")),
              QStringLiteral("a fault that lasts: the first and the retry are told as failed, the save after it is gone works and the journal goes"),
              out.left(800));
        check(out.contains(QLatin1String("conflictSignals=0")) && out.contains(QLatin1String("conflictFiles=0")),
              QStringLiteral("after a failed folder sync the retries make no false conflict (no copy in settings/conflicts, no notice)"), out.left(800));
        check(out.contains(QLatin1String("images=1")), QStringLiteral("a picture pasted as data is written once, not again under a new name at the retry"),
              out.left(800));
    }
    {
        // a file system that cannot sync folders (the fault is on from the opening): remembered, no error at a save
        const QString file = make(QStringLiteral("Remembered"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString out = runChild(file, folder, QStringLiteral("dirsync"), false);
        check(out.contains(QLatin1String("saved=1")) && out.contains(QLatin1String("journal=0")) && out.contains(QLatin1String("unsaved=0")),
              QStringLiteral("a file system that answers a folder's sync with an error already at the opening is remembered as one that cannot "
                             "sync folders: its saves are no failure"),
              out.left(600));
    }
    {
        const QString file = make(QStringLiteral("Notsup"));
        const QString folder = QFileInfo(file).absolutePath();
        const QString out = runChild(file, folder, QStringLiteral("dirsync-notsup"));
        check(out.contains(QLatin1String("saved=1")) && out.contains(QLatin1String("journal=0")) && out.contains(QLatin1String("unsaved=0")),
              QStringLiteral("a file system that cannot sync folders (ENOTSUP) is no error: saved as before"), out.left(600));
    }
#endif
    {
        // S3: a temporary file of an earlier save that is not the save's own is moved aside, never written over or deleted
        const QString file = make(QStringLiteral("Stray"));
        auto q = Project::open(file, &err);
        BinderItem *a = nullptr;
        for (BinderItem *it : q->allItems())
            if (it->title == QLatin1String("A"))
                a = it;
        const QString text = q->contentDir(a->id) + QStringLiteral("/text.md");
        const QString text2 = QFileInfo::exists(text) ? text : q->contentDir(a->id) + QStringLiteral("/text.rtf");
        writeAll(text2 + QStringLiteral(".qbstage"), "old temporary content of an earlier save");
        typeInto(q.get(), a, QStringLiteral(" newer"));
        QString why;
        const bool ok = q->save(&why);
        bool kept = false;
        for (const QString &f : QDir(QFileInfo(text2).absolutePath()).entryList(QDir::Files | QDir::Hidden))
            if (f.contains(QLatin1String(".qbstage.kept-")) && readAll(QFileInfo(text2).absolutePath() + QLatin1Char('/') + f).startsWith("old temporary"))
                kept = true;
        check(ok && kept && !QFile::exists(text2 + QStringLiteral(".qbstage")) && readAll(text2).contains("newer"),
              QStringLiteral("a temporary file of an earlier save in the way is moved aside (its content kept), and the save goes through"), why);
        q.reset();
    }
    {
        // S4: the journal in the way has its own plain sentence
        check(MainWindow::plainSaveReason(QStringLiteral("x"), failure::Kind::JournalInTheWay).contains(QLatin1String("journal"))
                  && !MainWindow::plainSaveReason(QStringLiteral("x"), failure::Kind::JournalInTheWay).contains(QLatin1String("could not be written")),
              QStringLiteral("a journal in the way is told in its own words, not as \"The files could not be written.\""));
    }
    {
        // S8: a temporary file that is gone and a target that was replaced later: nothing "lost" is reported; a target that
        // still holds the old content is
        auto journalFor = [&](const QString &name, bool targetLater) {
            const QString file = make(name);
            const QString folder = QFileInfo(file).absolutePath();
            QString rel;
            {
                auto q = Project::open(file, &err);
                for (BinderItem *it : q->allItems())
                    if (it->title == QLatin1String("A"))
                        for (const QString &n : {QStringLiteral("text.md"), QStringLiteral("text.rtf")})
                            if (rel.isEmpty() && QFileInfo::exists(q->contentDir(it->id) + QLatin1Char('/') + n))
                                rel = QDir(folder).relativeFilePath(q->contentDir(it->id) + QLatin1Char('/') + n);
            }
            const QByteArray old = readAll(folder + QLatin1Char('/') + rel);
            if (targetLater)
                writeAll(folder + QLatin1Char('/') + rel, "replaced later by something else");
            QJsonObject f;
            f.insert(QStringLiteral("file"), rel);
            f.insert(QStringLiteral("staged"), rel + QStringLiteral(".qbstage"));
            f.insert(QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash("never written", QCryptographicHash::Sha256).toHex()));
            f.insert(QStringLiteral("replaces"), QString::fromLatin1(QCryptographicHash::hash(old, QCryptographicHash::Sha256).toHex()));
            f.insert(QStringLiteral("size"), 13);
            f.insert(QStringLiteral("title"), QStringLiteral("A"));
            QJsonObject root;
            root.insert(QStringLiteral("version"), 1);
            root.insert(QStringLiteral("files"), QJsonArray{f});
            writeAll(folder + QStringLiteral("/settings/journal.json"), QJsonDocument(root).toJson());
            auto q = Project::open(file, &err);
            return q->saveRecoveryText();
        };
        const QString later = journalFor(QStringLiteral("Later"), true);
        const QString same = journalFor(QStringLiteral("Same"), false);
        check(!later.contains(QLatin1String("not complete")) && same.contains(QLatin1String("not complete")),
              QStringLiteral("a file replaced after the journal's save is no \"lost\" file; one still at its old content is"),
              later + QLatin1String(" || ") + same);
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

// ------------------------------------------------------------------ item 14: backups

static QByteArray damageAnEntry(const QString &zip)
{
    QByteArray z = readAll(zip);
    const qsizetype eocd = z.lastIndexOf("PK\x05\x06");
    auto u16 = [&](qsizetype at) { return quint16(quint8(z[at]) | (quint8(z[at + 1]) << 8)); };
    auto u32 = [&](qsizetype at) { return quint32(u16(at)) | (quint32(u16(at + 2)) << 16); };
    qsizetype at = qsizetype(u32(eocd + 16));
    const int count = u16(eocd + 10);
    for (int i = 0; i < count; ++i) {
        const quint32 comp = u32(at + 20);
        const int nameLen = u16(at + 28), extraLen = u16(at + 30), commentLen = u16(at + 32);
        const qsizetype local = qsizetype(u32(at + 42));
        const QByteArray name = z.mid(at + 46, nameLen);
        if (comp >= 16 && name.contains("/content/")) {
            const qsizetype data = local + 30 + u16(local + 26) + u16(local + 28);
            for (int k = 4; k < 12; ++k)
                z[data + k] = char(z[data + k] ^ 0x5A);
            writeAll(zip, z);
            return name;
        }
        at += 46 + nameLen + extraLen + commentLen;
    }
    return QByteArray();
}

static void backups(const QString &W)
{
    QString err;
    AppSettings &as = AppSettings::instance();
    {
        // the note of a backup that a signal left undone stays until the opening's backup is made
        auto proj = Project::create(W, QStringLiteral("Due"), &err);
        textDoc(proj.get(), QStringLiteral("Text"), QStringLiteral("Words."));
        proj->save(&err);
        const QString file = proj->indexPath();
        const QString journal = QFileInfo(file).absolutePath() + QStringLiteral("/settings/journal.json");
        as.backupDir = W + QStringLiteral("/bk-due");
        as.backupKeep = 5;
        as.backupOnClose = true;
        as.backupOnOpen = false;
        auto *wp = new MainWindow;
        MainWindow &w = *wp;
        w.show();
        w.adoptProject(std::move(proj));
        settle(400);
        w.saveForTermination();
        check(backup::isDue(file), QStringLiteral("(a signal ended the session: the backup is noted as due)"));
        as.backupOnClose = false;
        w.closeProject();
        as.backupOnClose = true;
        settle(200);
        // another computer's save is going on: the save before the backup waits, so the backup fails
        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("device"), QStringLiteral("Other computer"));
        root.insert(QStringLiteral("key"), QStringLiteral("some-other-computer"));
        root.insert(QStringLiteral("time"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        root.insert(QStringLiteral("files"), QJsonArray());
        writeAll(journal, QJsonDocument(root).toJson());
        auto again = Project::open(file, &err);
        w.adoptProject(std::move(again));
        settle(600);
        check(w.lastDueBackup().isEmpty() && backup::isDue(file) && !w.lastBackupProblem().isEmpty(),
              QStringLiteral("an opening whose backup fails (the project cannot be saved first): the note stays for the next opening"),
              w.lastBackupProblem());
        QFile::remove(journal); // (so that closing is not stopped by the save that waits)
        as.backupOnClose = false;
        w.closeProject();
        as.backupOnClose = true;
        auto third = Project::open(file, &err);
        w.adoptProject(std::move(third));
        settle(600);
        check(!w.lastDueBackup().isEmpty() && QFile::exists(w.lastDueBackup()) && !backup::isDue(file),
              QStringLiteral("... the opening after it makes the backup, and only then the note goes"), w.lastDueBackup());
        as.backupOnClose = false;
        w.closeProject();
        delete wp;
    }
    {
        // a clean-up that cannot delete an older backup
        auto p = Project::create(W, QStringLiteral("Keep"), &err);
        textDoc(p.get(), QStringLiteral("Text"), QStringLiteral("Words."));
        p->save(&err);
        const QString dir = W + QStringLiteral("/bk-keep");
        QString out1, out2, out3, out4, e;
        QStringList notes;
        check(backup::backupProject(p.get(), dir, 2, &out1, &e) && backup::backupProject(p.get(), dir, 2, &out2, &e), QStringLiteral("(two backups)"), e);
        qputenv("QB_TEST_FAIL_REMOVE", QFileInfo(out1).fileName().toUtf8());
        const bool ok3 = backup::backupProject(p.get(), dir, 2, &out3, &e, backup::Where::BackupFolder, nullptr, &notes);
        check(ok3 && e.isEmpty() && notes.size() == 1 && notes.first().contains(QFileInfo(out1).fileName()) && notes.first().contains(QLatin1String("Permission denied"))
                  && QFile::exists(out1),
              QStringLiteral("a backup that cannot be deleted is told with the file and the reason (the new backup is made)"), notes.join(QLatin1Char('|')) + e);
        QStringList notes2;
        const bool ok4 = backup::backupProject(p.get(), dir, 2, &out4, &e, backup::Where::BackupFolder, nullptr, &notes2);
        check(ok4 && notes2.isEmpty(), QStringLiteral("... and told once"), notes2.join(QLatin1Char('|')));
        qunsetenv("QB_TEST_FAIL_REMOVE");
        // the window's message
        auto *wp = new MainWindow;
        wp->show();
        wp->tellBackupCleanup(QStringList() << QStringLiteral("/backups/Keep bak 2026-11-20 10-00-00.zip: Permission denied"));
        check(wp->lastBackupCleanupNote().contains(QLatin1String("Permission denied")) && wp->lastBackupCleanupNote().contains(QLatin1String("could not be deleted")),
              QStringLiteral("the window's note names the backup and the reason"), wp->lastBackupCleanupNote());
        // S9: while the writer goes on typing, the notice is not modal and takes no keys
        qunsetenv("QB_NO_DIALOGS");
        wp->show();
        wp->tellBackupCleanup(QStringList() << QStringLiteral("/backups/Keep bak 2026-11-20 10-00-00.zip: Permission denied"));
        settle(300);
        QMessageBox *notice = nullptr;
        for (QWidget *tl : QApplication::topLevelWidgets())
            if (auto *b = qobject_cast<QMessageBox *>(tl); b && b->objectName() == QLatin1String("backupCleanupNotice"))
                notice = b;
        check(notice && notice->isVisible() && !notice->isModal() && !QApplication::activeModalWidget(),
              QStringLiteral("the note about a backup that cannot be deleted (manual save, opening) is not modal and takes no keys"));
        shot(notice, QStringLiteral("v0930-backup-cleanup-notice"));
        if (notice)
            notice->close();
        qputenv("QB_NO_DIALOGS", "1");
        delete wp;
    }
    {
        // Restore From Backup: restored, restored incompletely, not restored; a title that could not be saved
        auto proj = Project::create(W, QStringLiteral("Source"), &err);
        textDoc(proj.get(), QStringLiteral("First"), QStringLiteral("The first text of the project, long enough to be stored."));
        textDoc(proj.get(), QStringLiteral("Second"), QStringLiteral("The second text of the project, long enough to be stored too."));
        proj->save(&err);
        QString zip, e;
        check(backup::backupProject(proj.get(), W + QStringLiteral("/bk-restore"), 3, &zip, &e), QStringLiteral("(a backup)"), e);
        auto *wp = new MainWindow;
        MainWindow &w = *wp;
        w.show();
        qputenv("QB_NO_DIALOGS", "1");
        w.adoptProject(std::move(proj));
        settle(400);
        check(w.restoreBackupFile(zip) && w.lastRestoreText().startsWith(QLatin1String("The backup was restored into:")) && !w.lastRestoreText().contains(QLatin1String("INCOMPLETELY")),
              QStringLiteral("restore, form 1: restored"), w.lastRestoreText());
        settle(300);
        const QString bad = W + QStringLiteral("/bk-restore/damaged.zip");
        QFile::copy(zip, bad);
        const QByteArray entry = damageAnEntry(bad);
        check(!entry.isEmpty(), QStringLiteral("(an entry of the archive is damaged)"));
        check(w.restoreBackupFile(bad) && w.lastRestoreText().startsWith(QLatin1String("The backup was restored INCOMPLETELY into:"))
                  && w.lastRestoreText().contains(QString::fromUtf8(entry.mid(entry.lastIndexOf('/') + 1))),
              QStringLiteral("restore, form 2: an archive with a damaged entry is restored incompletely, headed so, with the file named"),
              w.lastRestoreText());
        settle(300);
        // the dialog itself
        qunsetenv("QB_NO_DIALOGS");
        shotLater(QStringLiteral("restoreResult"), QStringLiteral("v0930-restore-incomplete"), 600);
        w.restoreBackupFile(bad);
        settle(300);
        qputenv("QB_NO_DIALOGS", "1");
        // a title that cannot be saved
        qputenv("QB_TEST_WRITE_FAILS", "5|Input/output error|.qbstage");
        w.restoreBackupFile(zip);
        qunsetenv("QB_TEST_WRITE_FAILS");
        check(w.lastRestoreText().contains(QLatin1String("could not be marked")) && w.lastRestoreText().contains(QLatin1String("Input/output error")),
              QStringLiteral("a title that could not be saved is said"), w.lastRestoreText());
        settle(300);
        // form 3
        const QString junk = W + QStringLiteral("/junk.zip");
        writeAll(junk, "not a zip file at all");
        shotLater(QStringLiteral("restoreFailed"), QStringLiteral("v0930-restore-not-restored"), 300);
        check(!w.restoreBackupFile(junk) && w.lastRestoreText().startsWith(QLatin1String("The backup could not be restored:")),
              QStringLiteral("restore, form 3: not restored"), w.lastRestoreText());
        as.backupOnClose = false;
        w.closeProject();
        delete wp;
    }
}

int main(int argc, char **argv)
{
#ifdef QB_FAULT_SHIM
    if (argc > 2 && qstrcmp(argv[1], "--child-save") == 0)
        return childSave(argc, argv);
#endif
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
    backups(W);
    std::printf("%d failure(s)\n", failures);
    return failures;
}
