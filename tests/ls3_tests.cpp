// Live Sync, work package 3 (0.99.0): the core write gate. A follower's window never writes the shared files: its writes are
// captured. Checks: the path classes; the gate in safesave, savebatch, the vault and the remove helpers, with one scripted
// action per write path; follower mode of Project::open; the project file as operations (IndexDiff) and their application
// (OpApply, to the XML and to a Project), also on random edits; the reasons a document cannot be changed; DocumentStore::reload;
// the authority's extra batch files; the source audit; and that Live Sync off changes nothing.
//
//   xvfb-run -a ./qb_ls3_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/PathSafety.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/SaveBatch.h"
#include "core/collab/Gate.h"
#include "core/collab/IndexDiff.h"
#include "core/collab/OpApply.h"
#include "core/collab/PathClass.h"
#include "core/crypto/KeyFolder.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/ProjectCrypto.h"
#include "core/crypto/Vault.h"
#include "core/Objects.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>

#include <cstdio>
#include <memory>

using namespace qb;
using namespace qb::collab;

static int failures = 0;
static int passes = 0;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (ok) {
        ++passes;
    } else {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

// ---------------------------------------------------------------- helpers

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray("<missing>");
}

static bool writeAll(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(data) == data.size();
}

// every file of the folder with its hash; the device-local files (settings/ui-*.json) are left out
static QMap<QString, QByteArray> folderState(const QString &folder)
{
    QMap<QString, QByteArray> out;
    QDirIterator it(folder, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString p = it.next();
        const QString rel = QDir(folder).relativeFilePath(p);
        if (rel.startsWith(QLatin1String("settings/ui")))
            continue;
        out.insert(rel, QCryptographicHash::hash(readAll(p), QCryptographicHash::Sha256).toHex());
    }
    return out;
}

static QStringList setPaths(const QList<ChangeSet> &sets)
{
    QStringList out;
    for (const ChangeSet &s : sets)
        out << s.paths();
    return out;
}

static const FileChange *findIn(const QList<ChangeSet> &sets, const QString &path)
{
    for (const ChangeSet &s : sets)
        if (const FileChange *c = s.find(path))
            return c;
    return nullptr;
}

struct Made {
    std::unique_ptr<Project> p;
    QString ch1, ch2, folder, sub;
};

// a project with a few documents, saved
static Made makeProject(const QString &parent, const QString &name)
{
    Made m;
    QString err;
    m.p = Project::create(parent, name, &err);
    if (!m.p)
        return m;
    BinderModel *model = m.p->model();
    BinderItem *a = model->createItem(m.p->draft(), -1, ItemType::Text, QStringLiteral("Chapter One"));
    BinderItem *b = model->createItem(m.p->draft(), -1, ItemType::Text, QStringLiteral("Chapter Two"));
    BinderItem *f = model->createItem(m.p->draft(), -1, ItemType::Folder, QStringLiteral("Part"));
    BinderItem *s = model->createItem(f, -1, ItemType::Text, QStringLiteral("Scene"));
    m.ch1 = a->id;
    m.ch2 = b->id;
    m.folder = f->id;
    m.sub = s->id;
    m.p->docs()->text(a->id)->setPlainText(QStringLiteral("The first chapter has some words in it."));
    m.p->docs()->text(b->id)->setPlainText(QStringLiteral("The second chapter."));
    m.p->docs()->text(s->id)->setPlainText(QStringLiteral("A scene."));
    m.p->save(&err);
    return m;
}

// ---------------------------------------------------------------- 1. path classes

static void testPathClasses()
{
    struct Row { const char *path; PathKind kind; const char *resource; bool replicated; };
    const Row rows[] = {
        {"Book.qbx", PathKind::Index, "index", true},
        {"content/ABC-1/text.md", PathKind::Text, "doc:ABC-1", true},
        {"content/ABC-1/text.rtf", PathKind::Text, "doc:ABC-1", true},
        {"content/ABC-1/notes.md", PathKind::Notes, "doc:ABC-1", true},
        {"content/ABC-1/file.pdf", PathKind::MediaFile, "doc:ABC-1", true},
        {"content/ABC-1/images/a1b2.png", PathKind::DocPicture, "doc:ABC-1", true},
        {"content/ABC-1/synopsis.png", PathKind::DocPicture, "doc:ABC-1", true},
        {"content/ABC-1/mindmap.scap", PathKind::MindMap, "mindmap:ABC-1", true},
        {"content/ABC-1/snapshots/2026.md", PathKind::Snapshot, "snapshots:ABC-1", true},
        {"snapshots/ABC-1/index.json", PathKind::Snapshot, "snapshots:ABC-1", true},
        {"objects/characters/OBJ1.json", PathKind::Object, "object:OBJ1", true},
        {"objects/images/p.png", PathKind::ObjectImage, "object:images", true},
        {"objects/schema.json", PathKind::ObjectSchema, "object:schema", true},
        {"settings/look.json", PathKind::Settings, "settings", true},
        {"encryption.json", PathKind::Marker, "project", true},
        {"contents-by-title.txt", PathKind::Derived, "", true},
        {"settings/ui-abcdef.json", PathKind::DeviceLocal, "", false},
        {"settings/lock", PathKind::DeviceLocal, "", false},
        {"settings/lock-work-1234", PathKind::DeviceLocal, "", false},
        {"settings/journal.json", PathKind::DeviceLocal, "", false},
        {"settings/conflicts/x (copy).rtf", PathKind::DeviceLocal, "", false},
        {"settings/spare-copy.qbx", PathKind::DeviceLocal, "", false},
        {"content/ABC-1/text.md.qbstage", PathKind::DeviceLocal, "", false},
        {"livesync/state.json", PathKind::Internal, "", false},
        {"livesync-local/outbox/1.json", PathKind::Internal, "", false},
        {"livesync-replica.json", PathKind::Internal, "", false},
        {"stray.txt", PathKind::Unknown, "", false},
        {"content/text.md", PathKind::Unknown, "", false},
        {"content/../x/text.md", PathKind::Unknown, "", false},
        {"/etc/passwd", PathKind::Unknown, "", false},
        {"objects/x/y/z.json", PathKind::Unknown, "", false},
        {"settings/deep/x.json", PathKind::Unknown, "", false},
    };
    int bad = 0;
    for (const Row &r : rows) {
        const PathInfo i = classify(QString::fromLatin1(r.path));
        if (i.kind != r.kind || i.resource != QLatin1String(r.resource) || i.replicated() != r.replicated) {
            ++bad;
            std::printf("      %s: kind %d resource %s\n", r.path, int(i.kind), qPrintable(i.resource));
        }
    }
    check(bad == 0, QStringLiteral("%1 paths have the class, the resource and the replication the design's table gives").arg(sizeof(rows) / sizeof(rows[0])));
    check(classify(QStringLiteral("content/A/snapshots/x.md")).hostOnly() && classify(QStringLiteral("encryption.json")).hostOnly()
              && !classify(QStringLiteral("content/A/text.md")).hostOnly(),
          QStringLiteral("snapshots and the marker are the host's jobs only; a document's text is not"));
    check(!classify(QStringLiteral("contents-by-title.txt")).capturable() && classify(QStringLiteral("content/A/text.md")).capturable(),
          QStringLiteral("the derived list is never taken from a client; a text is"));

    QString why;
    const char *bads[] = {"", "/abs", "../x", "a/../b", "a\\b", "C:/x", "a//b", "CON", "content/nul.txt", "x/COM3.md", "x/lpt9",
                          "a/b ", "a/b.", "a/\x01z"};
    bool allRefused = true;
    for (const char *b : bads)
        if (isAcceptablePath(QString::fromLatin1(b), &why)) {
            allRefused = false;
            std::printf("      accepted: %s\n", b);
        }
    check(allRefused, QStringLiteral("empty, absolute, parent, backslash, drive letter, reserved Windows names, trailing space or dot, control characters: refused"));
    check(!isAcceptablePath(QString(241, QLatin1Char('a'))) && isAcceptablePath(QString(240, QLatin1Char('a'))),
          QStringLiteral("a path of 241 bytes is refused, 240 accepted"));
    check(isAcceptablePath(QStringLiteral("content/ABC-1/text.md")) && isAcceptablePath(QStringLiteral("objects/characters/O1.json")),
          QStringLiteral("ordinary project paths are accepted"));
    check(caseCollisions({QStringLiteral("a/Text.md"), QStringLiteral("b/x"), QStringLiteral("a/text.md")}) == QStringList{QStringLiteral("a/text.md")}
              && caseCollisions({QStringLiteral("a"), QStringLiteral("a")}).isEmpty(),
          QStringLiteral("two paths equal but for the case are found; the same path twice is no collision"));
    QTemporaryDir t;
    QDir(t.path()).mkpath(QStringLiteral("real"));
    check(QFile::link(t.path() + QLatin1String("/real"), t.path() + QLatin1String("/link")), QStringLiteral("a symbolic link made"));
    writeAll(t.path() + QLatin1String("/real/f.txt"), "x");
    check(symlinkOnTheWay(t.path(), QStringLiteral("link/f.txt")) && !symlinkOnTheWay(t.path(), QStringLiteral("real/f.txt"))
              && !symlinkOnTheWay(t.path(), QStringLiteral("real/new/f.txt")),
          QStringLiteral("a symbolic link on the way is found; none on a plain way or a way that does not exist yet"));
}

// ---------------------------------------------------------------- 2. Live Sync off

static void testOff(const QString &tmp)
{
    check(!gate().active(), QStringLiteral("no follower attached: the gate is off"));
    Made m = makeProject(tmp + QLatin1String("/off"), QStringLiteral("Off"));
    check(bool(m.p), QStringLiteral("a project made with Live Sync off"));
    const QString folder = m.p->folderPath();
    check(QFileInfo(m.p->indexPath()).isFile() && QFileInfo(folder + QLatin1String("/content/") + m.ch1 + QLatin1String("/text.md")).isFile(),
          QStringLiteral("the project file and the text are on disk after a save, as before"));
    check(!QFileInfo::exists(folder + QLatin1String("/livesync")) && !QFileInfo::exists(folder + QLatin1String("/livesync-local"))
              && !QFileInfo::exists(folder + QLatin1String("/livesync-replica.json")),
          QStringLiteral("no Live Sync file or folder appears"));
    check(!m.p->isFollower() && m.p->follower() == nullptr && m.p->canChange(QStringLiteral("doc:") + m.ch1) && !gate().active(),
          QStringLiteral("a project that is no follower's may change everything, and the gate stays off"));
    // the writes of every kind pass as before
    const QString dir = tmp + QLatin1String("/off-files");
    QString err;
    check(safesave::writeFile(dir + QLatin1String("/a.txt"), "one", &err) || (QDir().mkpath(dir) && safesave::writeFile(dir + QLatin1String("/a.txt"), "one", &err)),
          QStringLiteral("safesave::writeFile writes"), err);
    check(readAll(dir + QLatin1String("/a.txt")) == "one" && safesave::copyFile(dir + QLatin1String("/a.txt"), dir + QLatin1String("/b.txt"), &err)
              && readAll(dir + QLatin1String("/b.txt")) == "one" && safesave::removeFile(dir + QLatin1String("/b.txt"))
              && !QFileInfo::exists(dir + QLatin1String("/b.txt")),
          QStringLiteral("safesave copy and remove work as before"), err);
    check(pathsafety::removeTreeInside(dir, tmp) && !QFileInfo::exists(dir), QStringLiteral("removeTreeInside removes a folder inside its root as before"));
    {
        savebatch::Batch b(folder, folder + QLatin1String("/settings/journal.json"));
        const bool a = b.stage(folder + QLatin1String("/x1.txt"), "1", QStringLiteral("x1"), QString(), &err);
        const bool c = b.stage(folder + QLatin1String("/x2.txt"), "2", QStringLiteral("x2"), QString(), &err);
        check(a && c && b.size() == 2 && b.capturedCount() == 0 && b.commit(&err) && readAll(folder + QLatin1String("/x1.txt")) == "1"
                  && readAll(folder + QLatin1String("/x2.txt")) == "2" && !QFileInfo::exists(folder + QLatin1String("/settings/journal.json")),
              QStringLiteral("a batch stages and commits two files as before (journal gone at the end)"), err);
    }
    // extra batch files: a hook adds files to the save's batch
    int called = 0;
    m.p->extraBatchFiles = [&](savebatch::Batch &batch, QString *e) {
        ++called;
        return batch.stage(m.p->folderPath() + QLatin1String("/livesync/state.json"), "{\"seq\":1}", QStringLiteral("state"), QString(), e);
    };
    m.p->docs()->text(m.ch1)->setPlainText(QStringLiteral("changed once more"));
    check(m.p->save(&err) && called == 1 && readAll(folder + QLatin1String("/livesync/state.json")) == "{\"seq\":1}"
              && readAll(folder + QLatin1String("/content/") + m.ch1 + QLatin1String("/text.md")).contains("changed once more"),
          QStringLiteral("the extra batch files of a save land with its texts in one batch"), err);
    m.p->extraBatchFiles = [&](savebatch::Batch &, QString *e) {
        if (e)
            *e = QStringLiteral("the state could not be staged");
        return false;
    };
    m.p->docs()->text(m.ch1)->setPlainText(QStringLiteral("this one must not be saved"));
    const bool failed = !m.p->save(&err);
    check(failed && err.contains(QLatin1String("could not be staged")) && !readAll(folder + QLatin1String("/content/") + m.ch1 + QLatin1String("/text.md")).contains("must not"),
          QStringLiteral("an extra file that cannot be staged stops the whole save and nothing of it is written"), err);
    m.p->extraBatchFiles = nullptr;
    check(m.p->save(&err) && readAll(folder + QLatin1String("/content/") + m.ch1 + QLatin1String("/text.md")).contains("must not"),
          QStringLiteral("without the hook the save goes on and writes what waited"), err);
}

// ---------------------------------------------------------------- 3. the gate on its own

static void testGateDirect(const QString &tmp)
{
    Made m = makeProject(tmp + QLatin1String("/gd"), QStringLiteral("Gate"));
    const QString folder = m.p->folderPath();
    const QString id = m.ch1;
    m.p.reset();
    Follower f;
    gate().attach(&f, folder);
    check(gate().active() && f.folder() == QDir::cleanPath(folder) && gate().followerOfFolder(folder) == &f,
          QStringLiteral("a follower attached to a folder activates the gate"));
    const auto before = folderState(folder);
    const QString text = folder + QLatin1String("/content/") + id + QLatin1String("/text.md");
    QString err;
    check(safesave::writeFile(text, "plain bytes", &err), QStringLiteral("safesave::writeFile into a follower's folder counts as written"), err);
    check(folderState(folder) == before, QStringLiteral("... and no file of the folder changed"));
    {
        QSaveFile sf(text);
        check(sf.open(QIODevice::WriteOnly) && safesave::write(sf, "via qsavefile") == 13 && safesave::commit(sf),
              QStringLiteral("safesave::write and commit of a QSaveFile in the folder count as written"));
    }
    check(folderState(folder) == before, QStringLiteral("... and no file changed, no temporary file left"));
    check(safesave::copyFile(text, folder + QLatin1String("/content/") + id + QLatin1String("/file.bin"), &err), QStringLiteral("safesave::copyFile into the folder counts as done"), err);
    QImage img(4, 4, QImage::Format_RGB32);
    img.fill(Qt::red);
    check(vault::saveImage(img, folder + QLatin1String("/content/") + id + QLatin1String("/images/p.png")), QStringLiteral("vault::saveImage into the folder counts as written"));
    f.capture().flush();
    const QList<ChangeSet> written = f.capture().takeReady();
    check(safesave::removeFile(folder + QLatin1String("/content/") + id + QLatin1String("/text.rtf")), QStringLiteral("safesave::removeFile in the folder counts as done"));
    check(pathsafety::removeTreeInside(folder + QLatin1String("/content/") + id, folder), QStringLiteral("removeTreeInside of a document's folder counts as done"));
    check(!pathsafety::removeTreeInside(folder, folder), QStringLiteral("the project folder itself is never removed (refused)"));
    check(QFileInfo(text).isFile() && folderState(folder).contains(QLatin1String("content/") + id + QLatin1String("/text.md")), QStringLiteral("everything is still on disk"));
    check(folderState(folder) == before, QStringLiteral("... unchanged: nothing of all this reached the disk"));
    f.capture().flush();
    const QList<ChangeSet> sets = f.capture().takeReady();
    const FileChange *t = findIn(written, QLatin1String("content/") + id + QLatin1String("/text.md"));
    check(t && t->kind == FileChange::Put && t->data == "via qsavefile" && t->info.kind == PathKind::Text && t->sha256 == QCryptographicHash::hash(t->data, QCryptographicHash::Sha256).toHex(),
          QStringLiteral("the last write of a path is kept as plain bytes with its hash and class"));
    const FileChange *c = findIn(written, QLatin1String("content/") + id + QLatin1String("/file.bin"));
    check(c && c->data == readAll(text) && c->data != "via qsavefile", QStringLiteral("a copy is captured with the bytes of its source file"));
    const FileChange *im = findIn(written, QLatin1String("content/") + id + QLatin1String("/images/p.png"));
    check(im && QImage::fromData(im->data).size() == QSize(4, 4), QStringLiteral("a picture is captured as encoded bytes"));
    const FileChange *rm = findIn(sets, QLatin1String("content/") + id);
    check(rm && rm->kind == FileChange::RemoveTree, QStringLiteral("the removal of a document's folder is captured as a tree removal"));
    {
        // writes before a tree removal in the same set are cancelled by it
        f.capture().put(QLatin1String("content/") + id + QLatin1String("/text.md"), "x", classify(QLatin1String("content/") + id + QLatin1String("/text.md")));
        f.capture().removeTree(QLatin1String("content/") + id, classify(QLatin1String("content/") + id + QLatin1String("/_")));
        f.capture().flush();
        const QList<ChangeSet> s2 = f.capture().takeReady();
        check(s2.size() == 1 && s2.first().files.size() == 1 && s2.first().files.first().kind == FileChange::RemoveTree,
              QStringLiteral("a tree removal cancels the writes of the files below it in the same set"));
    }

    // device-local files pass; unknown paths are refused and counted
    const QString ui = folder + QLatin1String("/settings/ui-test.json");
    check(safesave::writeFile(ui, "{}", &err) && readAll(ui) == "{}", QStringLiteral("a device-local file (settings/ui-*.json) is written to the disk as always"), err);
    const QString stray = folder + QLatin1String("/stray.txt");
    check(!safesave::writeFile(stray, "x", &err) && !QFileInfo::exists(stray) && f.refusedCount() == 1 && f.refusedPaths() == QStringList{QStringLiteral("stray.txt")},
          QStringLiteral("a path of no class is refused, not written, and counted"), err);
    {
        QSaveFile sf(stray);
        const bool opened = sf.open(QIODevice::WriteOnly);
        check(opened && safesave::write(sf, "x") < 0 && !safesave::commit(sf) && !QFileInfo::exists(stray) && f.refusedCount() == 2,
              QStringLiteral("a QSaveFile of no class is refused too"));
    }
    // a file outside every follower folder passes
    const QString other = tmp + QLatin1String("/elsewhere/z.txt");
    QDir().mkpath(tmp + QLatin1String("/elsewhere"));
    check(safesave::writeFile(other, "z", &err) && readAll(other) == "z", QStringLiteral("a file outside every follower folder is written as always"), err);

    // the vault's backstop
    QByteArray data = "x";
    QString sealErr;
    const int refusedBefore = f.refusedCount();
    check(!vault::seal(folder + QLatin1String("/content/") + id + QLatin1String("/text.md"), data, &sealErr) && f.refusedCount() == refusedBefore + 1
              && !sealErr.isEmpty(),
          QStringLiteral("vault::seal for a follower's capturable path is refused (the gate's backstop), and counted"), sealErr);
    data = "x";
    check(vault::seal(ui, data, &sealErr), QStringLiteral("... but it passes for a device-local file"));

    // suspended capture drops writes
    f.capture().setSuspended(true);
    safesave::writeFile(text, "while suspended", &err);
    f.capture().flush();
    check(!f.capture().hasReady(), QStringLiteral("while a remote commit is applied, writes are dropped, not captured"));
    f.capture().setSuspended(false);

    // batch: staged files become one change set at the commit; a dropped batch gives none
    {
        savebatch::Batch b(folder, folder + QLatin1String("/settings/journal.json"));
        check(b.stage(text, "from a batch", QStringLiteral("t"), QString(), &err) && b.stage(folder + QLatin1String("/Gate.qbx"), "<x/>", QStringLiteral("p"), QString(), &err)
                  && b.capturedCount() == 2 && b.contains(text) && !f.capture().hasReady() && folderState(folder) == before,
              QStringLiteral("a batch keeps the staged files of a follower's folder (plain), nothing on disk, nothing captured before the commit"), err);
        check(b.commit(&err), QStringLiteral("the commit of such a batch succeeds"), err);
        const QList<ChangeSet> s = f.capture().takeReady();
        check(s.size() == 1 && s.first().files.size() == 2 && findIn(s, QStringLiteral("Gate.qbx")) && folderState(folder) == before
                  && !QFileInfo::exists(folder + QLatin1String("/settings/journal.json")),
              QStringLiteral("... and gives one change set with both files; the disk is unchanged and no journal was written"));
    }
    {
        savebatch::Batch b(folder, folder + QLatin1String("/settings/journal.json"));
        b.stage(text, "dropped", QStringLiteral("t"), QString(), &err);
        b.drop();
        check(!f.capture().hasReady() && f.capture().openCount() == 0, QStringLiteral("a dropped batch leaves nothing captured"));
    }
    {
        savebatch::Batch b(folder, folder + QLatin1String("/settings/journal.json"));
        const bool ok = b.stage(stray, "x", QStringLiteral("t"), QString(), &err);
        check(!ok && err.contains(QLatin1String("not part of a shared project")), QStringLiteral("a batch refuses a path of no class with a reason"), err);
    }
    // savebatch::put outside a batch
    check(savebatch::put(text, "put alone", &err) && findIn((f.capture().flush(), f.capture().takeReady()), QLatin1String("content/") + id + QLatin1String("/text.md")),
          QStringLiteral("savebatch::put without a batch is captured too"), err);

    gate().detach(&f);
    check(!gate().active(), QStringLiteral("detached: the gate is off again"));
    check(safesave::writeFile(text, "written again", &err) && readAll(text) == "written again", QStringLiteral("after the detach the same call writes to the disk"), err);
}

// ---------------------------------------------------------------- 4. follower mode: one scripted action per write path

static void testFollowerProject(const QString &tmp)
{
    Made m = makeProject(tmp + QLatin1String("/fp"), QStringLiteral("Follow"));
    const QString folder = m.p->folderPath();
    const QString indexPath = m.p->indexPath();
    const QString rel = QDir(folder).relativeFilePath(indexPath);
    // objects exist in the project (a category's first object), saved
    {
        const QString cat = m.p->objects()->schema().categories.first().id;
        m.p->objects()->create(cat, QString(), QStringLiteral("Someone"));
        QString e;
        m.p->save(&e);
    }
    const QString A = m.ch1, B = m.ch2, F = m.folder, S = m.sub;
    m.p.reset();
    // stale files that only the authority deals with
    writeAll(folder + QLatin1String("/settings/journal.json"), "{\"stale\":true}");
    writeAll(folder + QLatin1String("/settings/spare-copy.qbx"), "spare");
    writeAll(folder + QLatin1String("/stale.qbx.tmp"), "x");

    Follower f;
    QString err;
    auto p = Project::open(folder, &err, &f);
    check(bool(p) && p->isFollower() && p->follower() == &f && gate().active(), QStringLiteral("Project::open with a follower opens in follower mode"), err);
    check(QFileInfo::exists(folder + QLatin1String("/settings/journal.json")) && readAll(folder + QLatin1String("/settings/spare-copy.qbx")) == "spare",
          QStringLiteral("follower mode does not recover a cut-off save and does not write the spare copy"));
    check(p->recoverUnlistedDocuments() == 0 && p->saveRecoveryText().isEmpty(), QStringLiteral("follower mode recovers no unlisted documents and tells no recovery"));
    const auto base = folderState(folder);
    auto changed = [&]() {
        QStringList out;
        const auto now = folderState(folder);
        for (auto it = now.constBegin(); it != now.constEnd(); ++it)
            if (base.value(it.key()) != it.value())
                out << it.key();
        for (auto it = base.constBegin(); it != base.constEnd(); ++it)
            if (!now.contains(it.key()))
                out << it.key();
        return out;
    };
    auto take = [&]() { f.capture().flush(); return f.capture().takeReady(); };

    // 1. a text edit
    p->docs()->text(A)->setPlainText(QStringLiteral("The first chapter, edited in a follower."));
    check(p->save(&err), QStringLiteral("a save in follower mode succeeds"), err);
    QList<ChangeSet> sets = take();
    const QString textRel = QLatin1String("content/") + A + QLatin1String("/text.md");
    const FileChange *tc = findIn(sets, textRel);
    check(tc && tc->data.contains("edited in a follower") && findIn(sets, rel), QStringLiteral("text edit: the text and the project file (counts) are captured"));
    check(changed().isEmpty(), QStringLiteral("text edit: nothing of the folder changed on disk"), changed().join(QLatin1Char(',')));
    check(!QFileInfo::exists(folder + QLatin1String("/contents-by-title.txt")) || base.contains(QStringLiteral("contents-by-title.txt")),
          QStringLiteral("the follower does not write the list of documents"));
    check(!p->hasUnsavedChanges(), QStringLiteral("the save counts as done in the window (the commit is the service's business)"));
    // a second save in follower mode: no "changed elsewhere" conflict copy
    QSignalSpy conflicts(p.get(), &Project::conflictKept);
    p->docs()->text(A)->setPlainText(QStringLiteral("The first chapter, edited twice."));
    check(p->save(&err) && conflicts.isEmpty() && changed().isEmpty() && !QDir(folder + QLatin1String("/settings/conflicts")).exists(),
          QStringLiteral("a second save keeps no conflict copy (the replica's file is the service's, not another computer's)"),
          err + QStringLiteral(" conflicts=%1 changed=%2").arg(conflicts.size()).arg(changed().join(QLatin1Char(','))));
    sets = take();
    check(findIn(sets, textRel) && findIn(sets, textRel)->data.contains("edited twice"), QStringLiteral("the second save is captured with its newer text"));

    // 2. notes
    p->docs()->notes(B)->setPlainText(QStringLiteral("a note"));
    check(p->save(&err), QStringLiteral("notes: saved"), err);
    sets = take();
    const FileChange *nc = findIn(sets, QLatin1String("content/") + B + QLatin1String("/notes.md"));
    check(nc && nc->info.kind == PathKind::Notes && changed().isEmpty(), QStringLiteral("notes edit: captured as notes, disk unchanged"),
          setPaths(sets).join(QLatin1Char(',')) + QStringLiteral(" changed=") + changed().join(QLatin1Char(',')));

    // 3. a new item with text, a move and a trash
    BinderItem *n = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("New Scene"));
    const QString N = n->id;
    p->docs()->text(N)->setPlainText(QStringLiteral("brand new"));
    p->model()->moveItems({p->item(B)}, p->item(F), 0);
    p->model()->moveToTrash({p->item(S)});
    check(p->save(&err), QStringLiteral("structure: saved"), err);
    sets = take();
    const FileChange *ic = findIn(sets, rel);
    check(findIn(sets, QLatin1String("content/") + N + QLatin1String("/text.md")) && ic && ic->data.contains(N.toUtf8()) && changed().isEmpty(),
          QStringLiteral("new item, move, trash: the text and the whole project file are captured, the disk is unchanged"),
          setPaths(sets).join(QLatin1Char(',')) + QStringLiteral(" changed=") + changed().join(QLatin1Char(',')));
    // the captured project file is the plain XML of the window's model
    check(ic && canonicalIndex(ic->data) == canonicalIndex(p->indexBytes()), QStringLiteral("the captured project file equals the window's index"));

    // 4. project settings
    p->labels.append({99, QStringLiteral("Extra"), QColor(Qt::green)});
    p->notifySettingsChanged();
    check(p->save(&err), QStringLiteral("settings: saved"), err);
    sets = take();
    check(findIn(sets, rel) && findIn(sets, rel)->data.contains("Extra") && changed().isEmpty(), QStringLiteral("a project setting: the project file is captured"),
          setPaths(sets).join(QLatin1Char(',')) + QStringLiteral(" changed=") + changed().join(QLatin1Char(',')));

    // 5. an object
    {
        const QString cat = p->objects()->schema().categories.first().id;
        const QString oid = p->objects()->create(cat, QString(), QStringLiteral("Another"));
        check(p->save(&err), QStringLiteral("objects: saved"), err);
        sets = take();
        check(findIn(sets, QLatin1String("objects/") + cat + QLatin1Char('/') + oid + QLatin1String(".json")) && changed().isEmpty(),
              QStringLiteral("a new object: its file is captured, the disk unchanged"), setPaths(sets).join(QLatin1Char(',')) + QStringLiteral(" changed=") + changed().join(QLatin1Char(',')));
        p->objects()->remove(oid);
        check(p->save(&err), QStringLiteral("objects: removed and saved"), err);
        sets = take();
        check(changed().isEmpty(), QStringLiteral("a removed object: the disk is unchanged"), changed().join(QLatin1Char(',')));
    }

    // 6. a picture in a text
    {
        QImage img(8, 8, QImage::Format_RGB32);
        img.fill(Qt::blue);
        const QString name = p->docs()->importImage(A, img);
        sets = take();
        check(!name.isEmpty() && findIn(sets, QLatin1String("content/") + A + QLatin1Char('/') + name) && changed().isEmpty(),
              QStringLiteral("a picture imported into a document is captured, not written"), name + QStringLiteral(" ") + setPaths(sets).join(QLatin1Char(',')) + QStringLiteral(" changed=") + changed().join(QLatin1Char(',')));
    }

    // 7. deleting a document for good
    p->model()->deletePermanently(p->item(S));
    p->save(&err);
    sets = take();
    const FileChange *rc = findIn(sets, QLatin1String("content/") + S);
    check(rc && rc->kind == FileChange::RemoveTree && QFileInfo(folder + QLatin1String("/content/") + S).exists() && changed().isEmpty(),
          QStringLiteral("deleting a document: its folder's removal is captured, the files stay on disk"), setPaths(sets).join(QLatin1Char(',')) + QStringLiteral(" changed=") + changed().join(QLatin1Char(',')));

    // 8. the document store's storage format change would write everything: done through the same writes
    // (Markdown/RTF conversion is not available while shared: the window never offers it)

    // 9. device-local state still written
    p->uiState.insert(QStringLiteral("x"), 1);
    p->markUiStateDirty();
    p->save(&err);
    check(QFileInfo::exists(p->uiStateFile()) && !findIn(take(), QDir(folder).relativeFilePath(p->uiStateFile())),
          QStringLiteral("the device's own window state is written to the disk and not captured"));

    // 10. remote apply: nothing counts as the writer's change and nothing is captured
    {
        p->docs()->text(A)->setPlainText(QStringLiteral("a text only to dirty the save")); // (so that a save would capture)
        p->save(&err);
        take();
        p->beginRemoteApply();
        p->markIndexDirty();
        p->docs()->removeDocumentFiles(S);
        safesave::writeFile(folder + QLatin1String("/content/") + A + QLatin1String("/notes.md"), "remote", &err);
        p->endRemoteApply();
        check(!f.capture().hasReady() && f.capture().openCount() == 0 && !p->hasUnsavedChanges(),
              QStringLiteral("while a remote commit is applied: the binder is not marked unsaved and the capture drops what is written"));
    }
    p.reset();
    check(!gate().active() && f.folder() == QDir::cleanPath(folder), QStringLiteral("closing the follower's project detaches it from the gate"));
}

// ---------------------------------------------------------------- 5. operations

static QByteArray withDerivedChanged(QByteArray xml)
{
    xml.replace("words=\"", "words=\"9");
    xml.replace(QRegularExpression(QStringLiteral("modified=\"[^\"]*\"")).pattern().toLatin1(), "modified=\"2001-02-03T04:05:06\"");
    return xml;
}

static void testOps(const QString &tmp)
{
    Made m = makeProject(tmp + QLatin1String("/ops"), QStringLiteral("Ops"));
    Project *p = m.p.get();
    const QByteArray A = p->indexBytes();
    QString err;
    check(diffIndex(A, A, &err).isEmpty() && err.isEmpty(), QStringLiteral("the same project file gives no operations"));
    // derived attributes only
    {
        QByteArray d = A;
        d.replace(QRegularExpression(QStringLiteral("words=\"(\\d+)\"")).pattern().toLatin1(), "words=\"1\"");
        QByteArray d2 = A;
        d2.replace("modified=\"", "modified=\"1999-01-01T00:00:00\" x-ignored=\"");
        const QByteArray dd = withDerivedChanged(A);
        check(diffIndex(A, dd).isEmpty() && canonicalIndex(A) == canonicalIndex(dd),
              QStringLiteral("changed counts and modification times alone give no operations (the host computes them)"));
    }
    QByteArray bad = "<nope/>";
    diffIndex(A, bad, &err);
    check(!err.isEmpty(), QStringLiteral("a file that is no project file is reported"), err);

    // edits: create nested, move, fields, section, trash, delete
    BinderModel *model = p->model();
    BinderItem *c1 = p->item(m.ch1);
    BinderItem *fold = p->item(m.folder);
    BinderItem *newTop = model->createItem(p->draft(), 0, ItemType::Folder, QStringLiteral("Prologue"));
    BinderItem *newKid = model->createItem(newTop, -1, ItemType::Text, QStringLiteral("Opening"));
    model->moveItems({c1}, fold, 0);
    p->item(m.ch2)->title = QStringLiteral("Chapter Two, renamed");
    p->item(m.ch2)->synopsis = QStringLiteral("what happens");
    p->notifyItemChanged(p->item(m.ch2));
    p->labels.append({77, QStringLiteral("New Label"), QColor(Qt::cyan)});
    p->setTitle(QStringLiteral("Ops Renamed"));
    p->notifySettingsChanged();
    model->moveToTrash({p->item(m.sub)});
    const QByteArray B = p->indexBytes();
    const OpList ops = diffIndex(A, B, &err);
    QStringList names;
    for (const Op &op : ops)
        names << opName(op.kind);
    check(!ops.isEmpty() && names.contains(QStringLiteral("item.create")) && names.contains(QStringLiteral("item.move"))
              && names.contains(QStringLiteral("item.fields")) && names.contains(QStringLiteral("item.trash"))
              && names.contains(QStringLiteral("project.section")),
          QStringLiteral("a create, a move, a fields change, a trash and sections give the operations of their names"), names.join(QLatin1Char(' ')));
    check(ops.first().kind == Op::ItemCreate && ops.first().id == newTop->id && ops.first().parent == p->draft()->id && ops.first().after.isEmpty(),
          QStringLiteral("a create names its parent and its place; parents come before children"));
    bool kidAfterParent = false;
    for (int i = 0; i < ops.size(); ++i)
        if (ops.at(i).kind == Op::ItemCreate && ops.at(i).id == newKid->id)
            for (int j = 0; j < i; ++j)
                if (ops.at(j).id == newTop->id)
                    kidAfterParent = true;
    check(kidAfterParent, QStringLiteral("the child's create comes after its parent's"));
    check(!ops.first().element.contains(QLatin1String("Children")) && !ops.first().element.contains(QLatin1String("words=")),
          QStringLiteral("an item's element holds no children and no derived attribute"));
    QByteArray applied = A;
    check(applyToIndex(&applied, ops, &err), QStringLiteral("the operations apply to the old project file"), err);
    check(canonicalIndex(applied) == canonicalIndex(B), QStringLiteral("diff then apply gives the new project file (derived attributes masked)"),
          QString::fromUtf8(canonicalIndex(applied)).left(400));

    // to a Project: a second project from the old state
    {
        const QString copy = tmp + QLatin1String("/ops-copy");
        QDir().mkpath(copy);
        // the old state: A written into a copy of the folder
        const QString src = p->folderPath();
        QDirIterator it(src, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString f = it.next();
            writeAll(copy + QLatin1Char('/') + QDir(src).relativeFilePath(f), readAll(f));
        }
        writeAll(copy + QLatin1Char('/') + QFileInfo(p->indexPath()).fileName(), A);
        auto q = Project::open(copy, &err);
        check(bool(q), QStringLiteral("a second project opened on the old state"), err);
        if (q) {
            check(canonicalIndex(q->indexBytes()) == canonicalIndex(A), QStringLiteral("... it equals the old project file"));
            QSignalSpy rows(q->model(), &QAbstractItemModel::rowsInserted);
            check(applyToProject(q.get(), ops, &err), QStringLiteral("the operations apply to the project in memory"), err);
            check(canonicalIndex(q->indexBytes()) == canonicalIndex(B), QStringLiteral("... and its index equals the new project file"),
                  QString::fromUtf8(canonicalIndex(q->indexBytes())).left(300));
            check(!rows.isEmpty() && !q->hasUnsavedChanges() && q->labels.last().id == 77 && q->title() == QLatin1String("Ops Renamed"),
                  QStringLiteral("the model signalled the new rows; nothing counts as unsaved; the label and the title arrived"));
            check(q->item(newKid->id) && q->item(newKid->id)->parent == q->item(newTop->id) && q->item(m.sub)->parent == q->trash(),
                  QStringLiteral("the new item is below its parent; the trashed item is in the Trash"));
        }
    }
    // failures
    {
        QByteArray x = A;
        Op op;
        op.kind = Op::ItemMove;
        op.id = QStringLiteral("NOPE");
        check(!applyToIndex(&x, {op}, &err) && x == A && !err.isEmpty(), QStringLiteral("an operation on an item that is not there fails and leaves the file as it was"), err);
        op.id = m.folder;
        op.parent = m.sub; // the folder into its own child
        check(!applyToIndex(&x, {op}, &err) && x == A, QStringLiteral("an item moved into its own subtree is refused"), err);
        Op c;
        c.kind = Op::ItemCreate;
        c.id = m.ch1; // exists
        c.element = QStringLiteral("<Item id=\"x\" type=\"text\"><Title>T</Title></Item>");
        check(!applyToIndex(&x, {c}, &err) && x == A, QStringLiteral("a create of an id that exists is refused"), err);
        Op s;
        s.kind = Op::Section;
        s.name = QStringLiteral("Binder");
        s.element = QStringLiteral("<Binder/>");
        check(!applyToIndex(&x, {s}, &err) && x == A, QStringLiteral("the binder cannot be replaced as a section"), err);
        c.id = QStringLiteral("../evil");
        c.element = QStringLiteral("<Item id=\"x\" type=\"text\"/>");
        auto q = Project::open(p->folderPath(), &err);
        check(q && !applyToProject(q.get(), {c}, &err), QStringLiteral("an unsafe id is refused when a project applies a create"), err);
    }
}

static void testRandomOps(const QString &tmp, unsigned seed)
{
    QRandomGenerator rng(seed);
    Made m = makeProject(tmp + QLatin1String("/rnd"), QStringLiteral("Rnd"));
    Project *p = m.p.get();
    QString err;
    const QString src = p->folderPath(), copy = tmp + QLatin1String("/rnd-copy");
    {
        QDirIterator it(src, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString f = it.next();
            writeAll(copy + QLatin1Char('/') + QDir(src).relativeFilePath(f), readAll(f));
        }
    }
    auto q = Project::open(copy, &err);
    QByteArray replica = p->indexBytes(); // the replica's file as XML
    bool allEqual = bool(q), xmlEqual = true, noDirty = true;
    QString first;
    for (int round = 0; round < 40 && allEqual; ++round) {
        BinderModel *model = p->model();
        const QByteArray prev = p->indexBytes();
        const int edits = 1 + int(rng.bounded(4));
        for (int e = 0; e < edits; ++e) {
            QList<BinderItem *> all = p->allItems();
            QList<BinderItem *> movable, folders;
            for (BinderItem *it : all) {
                if (it == p->root())
                    continue;
                if (!it->isRootFolder())
                    movable << it;
                if (it->isContainer() || it->isFolder())
                    folders << it;
            }
            folders << p->draft() << p->research();
            switch (rng.bounded(7)) {
            case 0:
                model->createItem(folders.at(int(rng.bounded(int(folders.size())))), int(rng.bounded(4)) - 1,
                                  rng.bounded(2) ? ItemType::Text : ItemType::Folder, QStringLiteral("Item %1").arg(rng.bounded(1000)));
                break;
            case 1:
                if (!movable.isEmpty()) {
                    BinderItem *it = movable.at(int(rng.bounded(int(movable.size()))));
                    BinderItem *to = folders.at(int(rng.bounded(int(folders.size()))));
                    if (to != it && !to->isDescendantOf(it) && !p->isInTrash(it) && to != p->trash())
                        model->moveItems({it}, to, int(rng.bounded(5)) - 1);
                }
                break;
            case 2:
                if (!movable.isEmpty()) {
                    BinderItem *it = movable.at(int(rng.bounded(int(movable.size()))));
                    model->moveToTrash({it});
                }
                break;
            case 3:
                if (p->trash() && !p->trash()->children.isEmpty())
                    model->deletePermanently(p->trash()->children.at(int(rng.bounded(int(p->trash()->children.size())))));
                break;
            case 4:
                if (!movable.isEmpty()) {
                    BinderItem *it = movable.at(int(rng.bounded(int(movable.size()))));
                    it->title = QStringLiteral("T%1").arg(rng.bounded(100000));
                    it->label = int(rng.bounded(4)) - 1;
                    p->notifyItemChanged(it);
                }
                break;
            case 5:
                p->labels.append({100 + round * 10 + e, QStringLiteral("L%1").arg(round), QColor(Qt::magenta)});
                p->notifySettingsChanged();
                break;
            default:
                if (!movable.isEmpty()) {
                    BinderItem *it = movable.at(int(rng.bounded(int(movable.size()))));
                    // a move back to the top of the draft
                    if (!p->isInTrash(it) && it->parent != p->draft())
                        model->moveItems({it}, p->draft(), 0);
                }
                break;
            }
        }
        const QByteArray now = p->indexBytes();
        const OpList ops = diffIndex(prev, now, &err);
        if (!err.isEmpty()) {
            allEqual = false;
            first = QStringLiteral("round %1: diff: %2").arg(round).arg(err);
            break;
        }
        QByteArray x = replica;
        if (!applyToIndex(&x, ops, &err)) {
            xmlEqual = false;
            first = QStringLiteral("round %1: apply to xml: %2").arg(round).arg(err);
            break;
        }
        replica = x;
        if (canonicalIndex(replica) != canonicalIndex(now)) {
            xmlEqual = false;
            first = QStringLiteral("round %1: the xml differs").arg(round);
            break;
        }
        if (!applyToProject(q.get(), ops, &err)) {
            allEqual = false;
            first = QStringLiteral("round %1: apply to project: %2").arg(round).arg(err);
            break;
        }
        if (canonicalIndex(q->indexBytes()) != canonicalIndex(now)) {
            allEqual = false;
            first = QStringLiteral("round %1: the project differs").arg(round);
            break;
        }
        if (q->hasUnsavedChanges())
            noDirty = false;
    }
    check(xmlEqual, QStringLiteral("random edits (seed %1): diff then apply to the XML always gives the new file").arg(seed), first);
    check(allEqual, QStringLiteral("random edits (seed %1): applied to a Project the model always equals the new file").arg(seed), first);
    check(noDirty, QStringLiteral("random edits: applying never marks the project unsaved"));
}

// ---------------------------------------------------------------- 6. reasons, reload

static void testReasonsAndReload(const QString &tmp)
{
    Made m = makeProject(tmp + QLatin1String("/rr"), QStringLiteral("Reasons"));
    const QString folder = m.p->folderPath();
    check(m.p->docs()->whyNotEditable(m.ch1).isEmpty() && m.p->canChange(QStringLiteral("doc:") + m.ch1),
          QStringLiteral("a project that is no follower's: every document may be changed"));
    const QString A = m.ch1;
    m.p.reset();
    Follower f;
    QString err;
    auto p = Project::open(folder, &err, &f);
    check(p->docs()->whyNotEditable(A).isEmpty() && p->canChange(QStringLiteral("doc:") + A) && p->canChange(QStringLiteral("structure")),
          QStringLiteral("a follower that holds no foreign lock may change everything"));
    f.setHeldElsewhere(QStringLiteral("doc:") + A, QStringLiteral("Laptop"));
    check(p->docs()->whyNotEditable(A).contains(QLatin1String("Laptop")) && !p->docs()->whyNotEditable(A).isEmpty()
              && !p->canChange(QStringLiteral("doc:") + A) && p->canChange(QStringLiteral("doc:") + m.ch2),
          QStringLiteral("a document locked on another device says so by its name; the others stay editable"), p->docs()->whyNotEditable(A));
    check(p->docs()->whyNotEditable(A, true).contains(QLatin1String("Laptop")), QStringLiteral("the notes of that document too"));
    f.clearHeld(QStringLiteral("doc:") + A);
    check(p->docs()->whyNotEditable(A).isEmpty(), QStringLiteral("the reason goes with the lock"));
    f.setHeldElsewhere(QStringLiteral("project"), QStringLiteral("Desktop"));
    check(p->docs()->whyNotEditable(m.ch2).contains(QLatin1String("Desktop")) && !p->canChange(QStringLiteral("settings")),
          QStringLiteral("a project-wide lock covers every document and every resource"));
    f.clearHeld(QStringLiteral("project"));
    f.setReadOnly(true);
    check(!p->docs()->whyNotEditable(A).isEmpty() && !p->canChange(QStringLiteral("structure")) && !f.whyNotChangeable(QStringLiteral("settings")).isEmpty(),
          QStringLiteral("a reader's device may change nothing, with a reason"), p->docs()->whyNotEditable(A));
    f.setReadOnly(false);
    check(p->docs()->whyNotEditable(A).isEmpty(), QStringLiteral("the reader's reason goes when the role does"));

    // reload
    QTextDocument *doc = p->docs()->text(A);
    const QFont font = doc->defaultFont();
    const QUrl base = doc->baseUrl();
    const QString path = p->docs()->textPath(A);
    check(doc->toPlainText().contains(QLatin1String("first chapter")), QStringLiteral("the text of the document is loaded"));
    QSignalSpy about(p->docs(), &DocumentStore::aboutToReload), done(p->docs(), &DocumentStore::reloaded);
    check(writeAll(path, "A text the service wrote after a remote commit.\n"), QStringLiteral("the service writes the new file"));
    check(p->docs()->reload(A), QStringLiteral("reload of a clean document works"));
    check(p->docs()->text(A) == doc && doc->toPlainText().contains(QLatin1String("after a remote commit")) && !doc->toPlainText().contains(QLatin1String("first chapter")),
          QStringLiteral("the same QTextDocument shows the new text"), doc->toPlainText());
    check(!doc->isModified() && !p->docs()->hasDirty() && !doc->isUndoAvailable(), QStringLiteral("the reloaded document is clean and has no undo step of the reload"));
    check(doc->defaultFont() == font && doc->baseUrl() == base, QStringLiteral("its font and base address are kept"));
    check(about.size() == 1 && done.size() == 1 && about.first().first() == A && done.first().at(1).toBool() == false,
          QStringLiteral("aboutToReload and reloaded surround the change once, for the text"));
    p->save(&err);
    check(f.capture().openCount() == 0 && !f.capture().hasReady(), QStringLiteral("a reload is no edit: the next save captures nothing"));
    // unsaved local text is never overwritten
    doc->setPlainText(QStringLiteral("local, unsaved words"));
    writeAll(path, "yet another remote text\n");
    check(!p->docs()->reload(A) && doc->toPlainText().contains(QLatin1String("local, unsaved")), QStringLiteral("a document with unsaved changes is skipped by reload (false), its text kept"));
    // a notes document that is loaded and a text that is not
    const QString B = m.ch2;
    p->docs()->notes(B)->setPlainText(QStringLiteral("old note"));
    p->save(&err);
    f.capture().flush();
    f.capture().takeReady();
    writeAll(p->docs()->notesPath(B), "new note from elsewhere\n");
    writeAll(p->docs()->textPath(B), "second chapter from elsewhere\n");
    check(p->docs()->reload(B) && p->docs()->notes(B)->toPlainText().contains(QLatin1String("new note"))
              && p->docs()->text(B)->toPlainText().contains(QLatin1String("from elsewhere")),
          QStringLiteral("a document's text and notes are both read again; one that was not loaded is read at its next use"));
    p.reset();
}


// ---------------------------------------------------------------- 6b. an encrypted replica

static void testEncrypted(const QString &tmp)
{
    if (!crypto::available()) {
        std::printf("SKIPPED  the encrypted-replica checks: this build has no encryption (tools/get-crypto.sh before configure)\n");
        return;
    }
    crypto::KeyRing &ring = crypto::KeyRing::device();
    const QByteArray wp = "correct horse battery staple again", rp = "the recovery words are kept on paper";
    const QString writer = ring.createKey(QStringLiteral("Test Writer"), wp);
    const QString recovery = ring.createKey(QStringLiteral("Test Recovery"), rp);
    ring.removeKey(recovery, true);
    ring.unlock(writer, wp);
    ring.save(crypto::KeyRing::deviceDir());
    crypto::keyfolder::setRoles(crypto::keyfolder::Roles{writer, recovery});
    Made m = makeProject(tmp + QLatin1String("/enc"), QStringLiteral("Enc"));
    const QString folder = m.p->folderPath();
    const QString A = m.ch1;
    m.p.reset();
    QString err;
    check(crypto::encryptFolder(folder, crypto::Recipients{writer, recovery}, {}, &err), QStringLiteral("a project is encrypted (the host's folder)"), err);
    const QString textPath = folder + QLatin1String("/content/") + A + QLatin1String("/text.md");
    check(vault::isSealedData(readAll(textPath).left(64)), QStringLiteral("its text file is encrypted on disk"));
    const auto before = folderState(folder);
    Follower f;
    auto p = Project::open(folder, &err, &f);
    check(bool(p) && p->isEncrypted() && p->isFollower(), QStringLiteral("it opens as an encrypted follower"), err);
    if (!p)
        return;
    p->docs()->text(A)->setPlainText(QStringLiteral("Secretword in a follower's text."));
    p->docs()->notes(A)->setPlainText(QStringLiteral("Secretword note."));
    check(p->save(&err), QStringLiteral("a save in an encrypted follower succeeds"), err);
    f.capture().flush();
    const QList<ChangeSet> sets = f.capture().takeReady();
    const FileChange *tc = findIn(sets, QLatin1String("content/") + A + QLatin1String("/text.md"));
    const FileChange *ic = findIn(sets, QDir(folder).relativeFilePath(p->indexPath()));
    check(tc && tc->data.contains("Secretword") && !vault::isSealedData(tc->data.left(64)) && ic && ic->data.startsWith("<?xml"),
          QStringLiteral("the captured text and project file are plain bytes (the host seals them when it saves)"));
    check(folderState(folder) == before, QStringLiteral("the encrypted replica's files did not change"));
    QByteArray back;
    check(vault::readFile(textPath, &back) && !back.contains("Secretword"), QStringLiteral("the replica still holds the old text, encrypted"));
    check(f.refusedCount() == 0, QStringLiteral("nothing was refused on the way"));
    p.reset();
}

// ---------------------------------------------------------------- 7. source audit

struct AuditEntry {
    const char *file;
    const char *status; // gated | skipped | outside | pending
    const char *how;
};

// How each place of the program that writes, removes, renames or copies a file reaches a follower's folder. "gated": the write
// goes through collab::gate() (safesave, savebatch, the vault, removeFile, removeTreeInside); "skipped": a follower never gets
// there; "outside": it never writes into a project's folder (settings, exports, add-ons, logs, backups, temporary files);
// "pending <package>": writes into a project's folder on its own and must be routed through safesave or the gate by the
// package that owns the file, before sharing can be switched on.
static const AuditEntry kAudit[] = {
    {"src/android/MobileStart.cpp", "outside", "the Android program's own files"},
    {"src/compile/CompileFormat.cpp", "outside", "exports go to a file the writer chooses"},
    {"src/compile/DocxWriter.cpp", "outside", "exports"},
    {"src/compile/EpubExporter.cpp", "outside", "exports"},
    {"src/compile/ScriptExporters.cpp", "outside", "exports"},
    {"src/core/Addons.cpp", "outside", "add-ons are installed in the program's data folder"},
    {"src/core/AiCatalog.cpp", "outside", "the program's data folder"},
    {"src/core/Backup.cpp", "outside", "backups go to the backup folder; a client makes none (the host does)"},
    {"src/core/CrashGuard.cpp", "outside", "crash reports in the program's data folder"},
    {"src/core/CrashGuardCommon.cpp", "outside", "crash reports"},
    {"src/core/CrashGuardWin.cpp", "outside", "crash reports"},
    {"src/core/DocumentStore.cpp", "gated", "texts through savebatch::put, removes through safesave::removeFile; convertFile is the Tools menu's copy outside a project"},
    {"src/core/IconCatalog.cpp", "outside", "the program's data folder"},
    {"src/core/Launcher.cpp", "outside", "launcher files of the desktop"},
    {"src/core/MindMap.cpp", "pending package 5/7", "Board::save writes content/<id>/mindmap.scap with a QSaveFile of its own; to go through safesave"},
    {"src/core/Objects.cpp", "pending package 5/7", "objects are saved through savebatch::put (gated); the removal of an object's file and of replaced pictures uses QFile::remove"},
    {"src/core/OutlineProject.cpp", "outside", "makes a new project (the welcome project, templates): never a follower's"},
    {"src/core/PathSafety.cpp", "gated", "removeTreeInside asks the gate"},
    {"src/core/Portable.cpp", "outside", "the program's own data folder"},
    {"src/core/Project.cpp", "skipped", "the spare copy, the recovery of a cut-off save and the leftovers: a follower skips them; the window state is device-local (settings/ui-*.json); restore of the spare copy runs before a project opens"},
    {"src/core/ProjectTemplates.cpp", "outside", "templates and new projects"},
    {"src/core/ReadAloud.cpp", "outside", "voices in the program's data folder"},
    {"src/core/SafeSave.h", "gated", "the gate itself"},
    {"src/core/SaveBatch.cpp", "gated", "stage asks the gate; commit hands captured files over"},
    {"src/core/SettingsFile.cpp", "outside", "the program's settings file and exported settings files the writer chooses"},
    {"src/core/SettingsLayer.cpp", "outside", "settings/options.ini is a device-local class (the project's own program settings are not shared)"},
    {"src/core/Snapshots.cpp", "pending package 8", "index.json and the snapshot texts are written with a QSaveFile; snapshots are host jobs (taken, renamed, deleted, rolled back on the host)"},
    {"src/core/SyncSafety.cpp", "outside", "the copies kept in settings/conflicts (device-local) and the stamps"},
    {"src/core/UpdateCheck.cpp", "outside", "the update file in the data folder"},
    {"src/core/WebArchive.cpp", "outside", "saves a web page into a temporary place; it joins a project through the import (gated)"},
    {"src/core/WorkMarks.cpp", "skipped", "the work marks of the sync-program lock (device-local settings/lock*): replaced by Live Sync's locks while shared"},
    {"src/core/Zip.cpp", "outside", "archives (backups, exports) go outside the project"},
    {"src/core/collab/Gate.cpp", "gated", "the gate (a QSaveFile appears only as a pointer it remembers)"},
    {"src/core/collab/Gate.h", "gated", "the gate"},
    {"src/core/crypto/KeyFolder.cpp", "outside", "key folders are not project folders"},
    {"src/core/crypto/OpenPgp.cpp", "outside", "encrypts a file to another place (a conversion, a backup)"},
    {"src/core/crypto/ProjectCrypto.cpp", "skipped", "(en|de)crypting a project is not available while it is shared"},
    {"src/core/crypto/Vault.cpp", "gated", "seal, saveImage and copy ask the gate; the rest are decrypted copies in a private folder"},
    {"src/import/FileImporter.cpp", "pending package 5", "an import that fails removes the content folder it made with QDir::removeRecursively (not through the gate)"},
    {"src/import/ScrivenerFormat.cpp", "outside", "Scrivener-format projects are not shared; writes the .scriv"},
    {"src/import/ScrivenerImporter.cpp", "outside", "makes a new project from a .scriv"},
    {"src/text/RtfIO.cpp", "outside", "an image encoded in memory"},
    {"src/text/Script.cpp", "outside", "removes leftover files of the program's own fonts/scripts"},
    {"src/ui/AiDownloads.cpp", "outside", "model downloads in the data folder"},
    {"src/ui/AppMenuEntry.cpp", "outside", "the desktop menu entry"},
    {"src/ui/CompileDialog.cpp", "outside", "a compile output the writer chooses"},
    {"src/ui/CrashDialogs.cpp", "outside", "crash reports"},
    {"src/ui/DragDrop.cpp", "gated", "its own writeFile goes through safesave::write and commit; a dropped picture goes to a temporary folder"},
    {"src/ui/FontInstall.cpp", "outside", "fonts in the data folder"},
    {"src/ui/Inspector.cpp", "pending package 7", "QFile::remove of the synopsis picture and of a replaced file in content/<id>"},
    {"src/ui/MainWindow.cpp", "outside", "temporary folders of imports and removing a temporary file"},
    {"src/ui/MainWindowGroup.cpp", "outside", "a picture saved to a temporary file"},
    {"src/ui/MainWindowHelp.cpp", "outside", "moving a project folder to the trash (Delete Project) is the whole project, not a follower's file"},
    {"src/ui/MainWindowSettings.cpp", "gated", "exported settings go through safesave to a file the writer chooses"},
    {"src/ui/MediaView.cpp", "outside", "temporary folders of the viewer"},
    {"src/ui/MindMapController.cpp", "pending package 5/7", "the board is saved by MindMap's Board::save; exports of the board are files the writer chooses"},
    {"src/ui/OptionsDialog.cpp", "outside", "program options"},
    {"src/ui/Profiles.cpp", "outside", "profiles in the data folder"},
    {"src/ui/ProjectLook.cpp", "pending package 7", "settings/look.json with a QSaveFile of its own; to go through safesave"},
    {"src/ui/Theme.cpp", "outside", "theme files in the data folder"},
    {"src/ui/ToolDialogs.cpp", "outside", "tool dialogs write files the writer chooses"},
    {"src/ui/ToolHost.cpp", "outside", "private temporary folders of tools"},
    {"src/ui/UpdateChecker.cpp", "outside", "the update in the data folder"},
    {"src/ui/WinShell.cpp", "outside", "Windows shell entries"},
    {"src/ui/handwriting/LearningStore.cpp", "outside", "the handwriting store in the data folder"},
};

static void testSourceAudit()
{
    const QString root = QStringLiteral(QB_TEST_SOURCE_DIR);
    const QRegularExpression patterns[] = {
        QRegularExpression(QStringLiteral("\\bQSaveFile\\b")),
        QRegularExpression(QStringLiteral("QFile::(copy|rename|remove|moveToTrash|link)\\s*\\(")),
        QRegularExpression(QStringLiteral("QDir\\([^()]*\\)\\.(remove|rmdir|rmpath|removeRecursively|rename)\\s*\\(")),
        QRegularExpression(QStringLiteral("\\.removeRecursively\\s*\\(")),
        QRegularExpression(QStringLiteral("\\bQFile\\b[^;]*\\b(WriteOnly|Append|ReadWrite|Truncate)\\b")),
        QRegularExpression(QStringLiteral("\\bQImage\\b[^;]*\\.save\\(|\\bimg\\.save\\(|\\bimage\\.save\\(|\\bpixmap\\.save\\(")),
        QRegularExpression(QStringLiteral("QFile::setPermissions")),
    };
    QSet<QString> found;
    int files = 0;
    QDirIterator it(root + QLatin1String("/src"), {QStringLiteral("*.cpp"), QStringLiteral("*.h"), QStringLiteral("*.mm")}, QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QString rel = QDir(root).relativeFilePath(path);
        if (rel.contains(QLatin1String("third_party")))
            continue;
        ++files;
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            continue;
        const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
        for (const QString &raw : lines) {
            const QString l = raw.trimmed();
            if (l.startsWith(QLatin1String("//")) || l.startsWith(QLatin1Char('*')) || l.startsWith(QLatin1String("/*")))
                continue;
            for (const QRegularExpression &re : patterns)
                if (re.match(l).hasMatch()) {
                    found.insert(rel);
                    break;
                }
        }
    }
    check(files > 300, QStringLiteral("the audit read the sources (%1 files)").arg(files));
    QSet<QString> listed;
    bool empty = false;
    int pending = 0;
    QStringList pendingFiles;
    for (const AuditEntry &e : kAudit) {
        listed.insert(QString::fromLatin1(e.file));
        const QString st = QString::fromLatin1(e.status);
        if (!(st == QLatin1String("gated") || st == QLatin1String("skipped") || st == QLatin1String("outside") || st.startsWith(QLatin1String("pending"))) || !*e.how)
            empty = true;
        if (st.startsWith(QLatin1String("pending"))) {
            ++pending;
            pendingFiles << QString::fromLatin1(e.file) + QLatin1Char(' ') + st;
        }
    }
    QStringList unlisted;
    for (const QString &f : found)
        if (!listed.contains(f))
            unlisted << f;
    unlisted.sort();
    check(unlisted.isEmpty(),
          QStringLiteral("every file of src/ that writes, copies, renames or removes files is on the audit's list, with how it reaches the gate "
                         "(add new ones to kAudit in tests/ls3_tests.cpp)"),
          QStringLiteral("not listed: ") + unlisted.join(QLatin1String(", ")));
    QStringList gone;
    for (const QString &l : listed)
        if (!QFileInfo::exists(root + QLatin1Char('/') + l))
            gone << l;
    check(gone.isEmpty(), QStringLiteral("every file of the list exists"), gone.join(QLatin1String(", ")));
    check(!empty, QStringLiteral("every entry has a status and a reason"));
    std::printf("INFO  %d files still write into a project folder on their own (pending): %s\n", pending, qPrintable(pendingFiles.join(QLatin1String("; "))));
    // the gate's own hooks are present where the design puts them
    auto contains = [&](const char *file, const char *needle) { return readAll(root + QLatin1Char('/') + QLatin1String(file)).contains(needle); };
    check(contains("src/core/SafeSave.h", "collab::gate()") && contains("src/core/SaveBatch.cpp", "collab::gate().decide")
              && contains("src/core/PathSafety.cpp", "collab::gate().removeTree") && contains("src/core/crypto/Vault.cpp", "collab::gate().sealAllowed")
              && contains("src/core/crypto/Vault.cpp", "collab::gate().decide(path)"),
          QStringLiteral("the gate is asked in safesave, savebatch, the remove helper and the vault"));
}

// ---------------------------------------------------------------- main

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_UPDATE_CHECK", "1");
    qputenv("QB_LS_STRICT", "0");
    qputenv("QB_TEST_KEYSTORE", "memory");
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QGuiApplication app(argc, argv);
    QTemporaryDir tmpDir;
    const QString tmp = tmpDir.path();

    testPathClasses();
    testOff(tmp);
    testGateDirect(tmp);
    testFollowerProject(tmp);
    testOps(tmp);
    for (unsigned seed : {1u, 2u, 3u})
        testRandomOps(tmp + QStringLiteral("/s%1").arg(seed), seed);
    testReasonsAndReload(tmp);
    testEncrypted(tmp);
    testSourceAudit();

    std::printf("%d pass\n%d failure(s)\n", passes, failures);
    return failures ? 1 : 0;
}
