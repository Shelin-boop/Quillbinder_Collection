// 0.99.0, to-do item 113 (part 2): the encrypted project and the encrypted backups. The vault (every read and write of an
// encrypted project), round trips of every kind of file, the walk over a written encrypted project (every file OpenPGP but
// the marker and the lock), the save journal and the waiting files, the emergency copy, encrypted backups of both project
// formats (names, the newest-N rule, restore, Move Old Backups, the check through decryption), turning the encryption on
// and off and a conversion cut off after n files, a wrong passphrase, a missing key (nothing written), a damaged file (the
// document says so, the rest opens), conflict copies that stay encrypted, a backup and a text opened by GnuPG with the
// exported secret key (each gpg on the PATH and QB_TEST_GPG_EXTRA; skipped with a message where there is none), the
// window's unlock dialog, the message with Import Keys, and the speed of the encrypted autosave.
//
//   xvfb-run -a ./qb_v0990f_tests /tmp/shots       (QB_MEASURE=1: only the speed figures)
// Exit code = number of failed checks.
#include "core/Backup.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/IconCatalog.h"
#include "core/MindMap.h"
#include "core/Objects.h"
#include "core/OutlineProject.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/SaveBatch.h"
#include "core/Search.h"
#include "core/SettingsLayer.h"
#include "core/Snapshots.h"
#include "core/SyncSafety.h"
#include "core/WorkMarks.h"
#include "core/Zip.h"
#include "core/crypto/KeyFolder.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/ProjectSwitches.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/ProjectCrypto.h"
#include "core/crypto/Vault.h"
#include "import/ScrivenerFormat.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "ui/ProjectCryptoUi.h"
#include "text/TextUtil.h"

#include <QAction>
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QRegularExpression>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>
#include <cstdio>
#include <functional>

using namespace qb;
using namespace qb::crypto;

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

static void info(const QString &s)
{
    std::printf("      %s\n", qPrintable(s));
    std::fflush(stdout);
}

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

static bool writeAll(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(data) == data.size();
}

static const QByteArray kWriterPass = "correct horse battery staple again";
static const QByteArray kRecoveryPass = "the recovery words are kept on paper";
static QString g_writer, g_recovery;
static QString g_recoverySecret; // the exported secret part of the recovery key (Quillbinder keeps only the public part)

static Recipients recipients() { return Recipients{g_writer, g_recovery}; }

// every file of a folder that is not OpenPGP of Quillbinder's kind (but the marker and the lock)
static QStringList plainFilesIn(const QString &folder, const QStringList &allowed = vault::projectPlainNames())
{
    QStringList out;
    QDirIterator it(folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QString rel = QDir(folder).relativeFilePath(path);
        if (allowed.contains(rel))
            continue;
        if (!vault::isSealedData(readAll(path).left(64)))
            out << rel;
    }
    out.sort();
    return out;
}

static int filesIn(const QString &folder)
{
    int n = 0;
    QDirIterator it(folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        ++n;
    }
    return n;
}

// every file with the SHA-256 of its bytes (a folder that must not change)
static QMap<QString, QByteArray> fingerprintOf(const QString &folder)
{
    QMap<QString, QByteArray> m;
    QDirIterator it(folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        m.insert(QDir(folder).relativeFilePath(path), QCryptographicHash::hash(readAll(path), QCryptographicHash::Sha256));
    }
    return m;
}

static void setText(Project *p, BinderItem *it, const QString &text)
{
    QTextDocument *d = p->docs()->text(it->id);
    QTextCursor c(d);
    c.select(QTextCursor::Document);
    c.insertText(text);
}

static QImage picture(const QColor &c, int w = 40, int h = 30)
{
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(c);
    QPainter pt(&img);
    pt.fillRect(5, 5, 10, 10, Qt::black);
    return img;
}

struct Rich {
    QString folder, index;
    QString t1, t2, t3, image, objectId, pictureName, mapId;
};

// a project with a file of every kind the program writes into a project folder
static Rich makeRich(const QString &parent, const QString &name)
{
    Rich r;
    QString err;
    auto p = Project::create(parent, name, &err);
    if (!p) {
        check(false, QStringLiteral("(the project %1 is made)").arg(name), err);
        return r;
    }
    r.folder = p->folderPath();
    r.index = p->indexPath();
    BinderItem *a = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter One"));
    BinderItem *b = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter Two"));
    BinderItem *c = p->model()->createItem(p->research(), -1, ItemType::Text, QStringLiteral("Research Note"));
    r.t1 = a->id;
    r.t2 = b->id;
    r.t3 = c->id;
    setText(p.get(), a, QStringLiteral("The lighthouse keeper counted the ships. Secretword1 is in here."));
    setText(p.get(), b, QStringLiteral("A second chapter with Secretword2 and more words."));
    setText(p.get(), c, QStringLiteral("Research about tides: Secretword3."));
    {
        QTextCursor n(p->docs()->notes(a->id));
        n.insertText(QStringLiteral("Notes of chapter one: Secretword4."));
        QTextCursor pn(p->docs()->projectNotes());
        pn.insertText(QStringLiteral("Project notes: Secretword5."));
    }
    // a picture inside a text (content/<id>/images/...)
    {
        QTextCursor cur(p->docs()->text(a->id));
        cur.movePosition(QTextCursor::End);
        const QString rel = p->docs()->importImage(a->id, picture(Qt::red));
        QTextImageFormat f;
        f.setName(rel);
        cur.insertImage(f);
    }
    // a picture document (media file)
    {
        BinderItem *img = p->model()->createItem(p->research(), -1, ItemType::Image, QStringLiteral("A Picture"));
        img->fileExt = QStringLiteral("png");
        r.image = img->id;
        QDir().mkpath(p->contentDir(img->id));
        picture(Qt::blue, 64, 48).save(p->mediaFilePath(img));
    }
    a->synopsis = QStringLiteral("Synopsis Secretword6");
    p->markIndexDirty();
    // objects with a picture
    {
        objects::Store *st = p->objects();
        r.objectId = st->create(QStringLiteral("characters"), QString(), QStringLiteral("Captain Secretword7"));
        const QString picFile = parent + QStringLiteral("/face.png");
        picture(Qt::green).save(picFile);
        r.pictureName = st->importPicture(r.objectId, QStringLiteral("picture"), picFile);
    }
    // a mind map board of a document
    {
        r.mapId = c->id;
        mindmap::Board board;
        mindmap::Note n;
        n.id = 1;
        n.text = QStringLiteral("Map note Secretword8");
        board.notes.append(n);
        QDir().mkpath(p->contentDir(c->id));
        board.save(mindmap::pathFor(p->contentDir(c->id)));
    }
    // an icon of the project's own
    icons::addCustom(p.get(), QStringLiteral("Lamp"), picture(Qt::yellow, 32, 32));
    if (!p->save(&err))
        check(false, QStringLiteral("(the project %1 is saved)").arg(name), err);
    // a snapshot, the objects, the project's own options and look
    snapshots::take(p.get(), a->id, QStringLiteral("First snapshot Secretword9"));
    p->objects()->save(&err);
    {
        const QString own = settingslayer::fileFor(p->settingsDir());
        QSettings s(own, settingslayer::formatFor(own));
        s.setValue(QStringLiteral("editor/testKey"), QStringLiteral("project value Secretword10"));
    }
    writeAll(p->settingsDir() + QStringLiteral("/look.json"), "{\"border\": 2}\n");
    p->uiState.insert(QStringLiteral("testState"), QStringLiteral("Secretword11"));
    p->markUiStateDirty();
    p->markIndexDirty();
    p->save(&err);
    return r;
}

// whether the first picture inside a text is shown (its resource is there)
static bool pictureShownIn(QTextDocument *d, QString *detail = nullptr)
{
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
        for (auto f = b.begin(); !f.atEnd(); ++f)
            if (f.fragment().charFormat().isImageFormat()) {
                const QString name = f.fragment().charFormat().toImageFormat().name();
                const QVariant v = d->resource(QTextDocument::ImageResource, QUrl(name));
                if (detail)
                    *detail = QStringLiteral("%1: %2").arg(name, QString::fromLatin1(v.typeName()));
                return (v.canConvert<QImage>() && !v.value<QImage>().isNull()) || (v.canConvert<QPixmap>() && !v.value<QPixmap>().isNull());
            }
    if (detail)
        *detail = QStringLiteral("no picture in the text");
    return false;
}

static std::unique_ptr<Project> openOk(const QString &index, const QString &what)
{
    QString err;
    auto p = Project::open(index, &err);
    check(p != nullptr, what, err);
    return p;
}

// ----------------------------------------------------------------------------------------------------- the vault

static void vaultBasics(const QString &W)
{
    std::printf("== the vault: seal, unseal, plain names, copies, pictures\n");
    const QString dir = W + QStringLiteral("/vault");
    QDir().mkpath(dir + QStringLiteral("/settings"));
    check(!vault::active(), QStringLiteral("no folder in the vault at the start: plain projects pay nothing"));
    QByteArray data = "plain text";
    check(vault::seal(dir + QStringLiteral("/a.md"), data) && data == "plain text", QStringLiteral("outside the vault, seal() leaves the text as it is"));
    vault::add(dir, recipients().list());
    check(vault::contains(dir) && vault::seals(dir + QStringLiteral("/content/x/text.md")), QStringLiteral("a file in a vault folder is sealed"));
    check(!vault::seals(dir + QStringLiteral("/encryption.json")) && !vault::seals(dir + QStringLiteral("/settings/lock")),
          QStringLiteral("the marker and the lock stay plain"));
    check(!vault::seals(dir + QStringLiteral("x/a.md")) && !vault::seals(W + QStringLiteral("/other/a.md")),
          QStringLiteral("a folder beside it with a name that begins the same is not in the vault"));
    QString why;
    data = "Secret text of a chapter";
    check(vault::seal(dir + QStringLiteral("/a.md"), data, &why) && vault::isSealedData(data) && looksEncrypted(data),
          QStringLiteral("seal() gives an OpenPGP message"), why);
    check(!data.contains("Secret"), QStringLiteral("... without the text in it"));
    QByteArray back = data;
    check(vault::unseal(dir + QStringLiteral("/a.md"), back, &why) && back == "Secret text of a chapter",
          QStringLiteral("unseal() gives the text back"), why);
    QByteArray plain = "a plain file during a conversion";
    check(vault::unseal(dir + QStringLiteral("/b.md"), plain) && plain == "a plain file during a conversion",
          QStringLiteral("a plain file in a vault folder is read as it is"));
    // safesave
    check(safesave::writeFile(dir + QStringLiteral("/c.md"), "written by writeFile") && vault::isSealedData(readAll(dir + QStringLiteral("/c.md"))),
          QStringLiteral("safesave::writeFile writes encrypted"));
    {
        QSaveFile f(dir + QStringLiteral("/d.json"));
        const QByteArray j = "{\"a\": 1}";
        const bool ok = f.open(QIODevice::WriteOnly) && safesave::write(f, j) == j.size() && safesave::commit(f);
        check(ok && vault::isSealedData(readAll(dir + QStringLiteral("/d.json"))), QStringLiteral("safesave::write gives the text's size and writes encrypted"));
    }
    QByteArray rd;
    check(vault::readFile(dir + QStringLiteral("/c.md"), &rd) && rd == "written by writeFile", QStringLiteral("vault::readFile decrypts"));
    // copies
    writeAll(W + QStringLiteral("/outside.txt"), "a file from outside");
    check(safesave::copyFile(W + QStringLiteral("/outside.txt"), dir + QStringLiteral("/in.txt")) && vault::isSealedData(readAll(dir + QStringLiteral("/in.txt"))),
          QStringLiteral("a copy into the vault folder is encrypted"));
    check(safesave::copyFile(dir + QStringLiteral("/in.txt"), dir + QStringLiteral("/in2.txt")) && readAll(dir + QStringLiteral("/in.txt")) == readAll(dir + QStringLiteral("/in2.txt")),
          QStringLiteral("a copy inside it keeps the bytes (no second encryption)"));
    check(safesave::copyFile(dir + QStringLiteral("/in2.txt"), W + QStringLiteral("/out.txt")) && readAll(W + QStringLiteral("/out.txt")) == "a file from outside",
          QStringLiteral("a copy out of it is decrypted"));
    // pictures
    check(vault::saveImage(picture(Qt::red), dir + QStringLiteral("/p.png")) && vault::isSealedData(readAll(dir + QStringLiteral("/p.png"))),
          QStringLiteral("saveImage writes an encrypted picture"));
    check(!vault::readImage(dir + QStringLiteral("/p.png")).isNull() && QImage(dir + QStringLiteral("/p.png")).isNull(),
          QStringLiteral("readImage decrypts it (Qt alone cannot read it)"));
    const QString copy = vault::plainPath(dir + QStringLiteral("/p.png"));
    check(!copy.isEmpty() && copy != dir + QStringLiteral("/p.png") && !QImage(copy).isNull() && QFileInfo(copy).fileName() == QLatin1String("p.png"),
          QStringLiteral("plainPath gives a decrypted copy under the same name"), copy);
    const QString runtime = qEnvironmentVariable("XDG_RUNTIME_DIR");
    check(copy.startsWith(runtime + QStringLiteral("/quillbinder-plain-")), QStringLiteral("... in the private folder of $XDG_RUNTIME_DIR"), copy);
    check((QFileInfo(QFileInfo(copy).absolutePath()).absolutePath() != QString())
              && (QFile::permissions(QFileInfo(QFileInfo(copy).absolutePath()).absolutePath()) & (QFileDevice::ReadGroup | QFileDevice::ReadOther)) == 0,
          QStringLiteral("... whose folder only its owner can read"));
    // tests' switch: a failed encryption writes nothing
    qputenv("QB_TEST_SEAL_FAILS", "refused.md");
    check(!safesave::writeFile(dir + QStringLiteral("/refused.md"), "x", &why) && !QFile::exists(dir + QStringLiteral("/refused.md")),
          QStringLiteral("a file that cannot be encrypted is not written (never plain instead)"), why);
    qunsetenv("QB_TEST_SEAL_FAILS");
    vault::remove(dir);
    check(!vault::active() && !QFile::exists(copy), QStringLiteral("the folder leaves the vault; its decrypted copies go"));
}

// ----------------------------------------------------------------------------------------- encrypted project

static void encryptedProject(const QString &W)
{
    std::printf("== an encrypted project: every file encrypted on its own, every kind read back\n");
    const Rich r = makeRich(W, QStringLiteral("Encrypted Book"));
    if (r.folder.isEmpty())
        return;
    {
        auto p = openOk(r.index, QStringLiteral("(the plain project opens)"));
        QString pd;
        check(p && pictureShownIn(p->docs()->text(r.t1), &pd), QStringLiteral("(plain: the picture inside a text is shown)"), pd);
    }
    const QMap<QString, QByteArray> plainBefore = fingerprintOf(r.folder);
    check(!plainFilesIn(r.folder).isEmpty(), QStringLiteral("(the plain project has plain files: %1)").arg(filesIn(r.folder)));
    QString err;
    int calls = 0, lastTotal = 0;
    check(encryptFolder(r.folder, recipients(), [&](int, int total) { ++calls; lastTotal = total; return true; }, &err),
          QStringLiteral("encryptFolder turns the project's encryption on"), err);
    check(calls > 1 && lastTotal == filesIn(r.folder) - 1, QStringLiteral("... with progress over every file (%1 files)").arg(lastTotal));
    QStringList left = plainFilesIn(r.folder);
    check(left.isEmpty(), QStringLiteral("the walk: every file of the project is OpenPGP but the marker and the lock (%1 files)").arg(filesIn(r.folder)),
          left.join(QLatin1String(", ")));
    const ProjectSettings ps = settingsOf(r.folder);
    check(ps.encryptedProject && ps.encryptedBackups && ps.recipients == recipients() && ps.converting.isEmpty(),
          QStringLiteral("the marker says: encrypted project, encrypted backups, the two recipients, no conversion under way"));
    const QByteArray marker = readAll(markerPath(r.folder));
    check(marker.contains(g_writer.toLatin1()) && marker.contains(g_recovery.toLatin1()) && !marker.contains("Secret") && !marker.contains("PRIVATE"),
          QStringLiteral("the marker is plain and names the fingerprints, no secret"));
    bool anyLeak = false;
    QDirIterator it(r.folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
        anyLeak = anyLeak || readAll(it.next()).contains("Secretword");
    check(!anyLeak, QStringLiteral("no file of the folder holds any of the project's words"));

    // opening and reading every kind back
    {
        auto p = openOk(r.index, QStringLiteral("the encrypted project opens (the writer's key is unlocked)"));
        if (!p)
            return;
        check(p->isEncrypted() && vault::contains(r.folder), QStringLiteral("... and its folder is in the vault while it is open"));
        check(p->docs()->text(r.t1)->toPlainText().contains(QLatin1String("Secretword1")) &&
                  p->docs()->text(r.t2)->toPlainText().contains(QLatin1String("Secretword2")) &&
                  p->docs()->text(r.t3)->toPlainText().contains(QLatin1String("Secretword3")),
              QStringLiteral("round trip: the texts"));
        check(p->docs()->notes(r.t1)->toPlainText().contains(QLatin1String("Secretword4")) &&
                  p->docs()->projectNotes()->toPlainText().contains(QLatin1String("Secretword5")),
              QStringLiteral("round trip: notes and project notes"));
        check(p->item(r.t1) && p->item(r.t1)->synopsis.contains(QLatin1String("Secretword6")) && p->item(r.t1)->title == QLatin1String("Chapter One"),
              QStringLiteral("round trip: the project file (binder, titles, synopsis)"));
        QString pd;
        check(pictureShownIn(p->docs()->text(r.t1), &pd), QStringLiteral("round trip: a picture inside a text is shown"), pd);
        BinderItem *img = p->item(r.image);
        check(img && !vault::readImage(p->mediaFilePath(img)).isNull() && QImage(vault::plainPath(p->mediaFilePath(img))).size() == QSize(64, 48),
              QStringLiteral("round trip: a picture document (read in memory, and through its decrypted copy)"));
        const QList<Snapshot> snaps = snapshots::list(p.get(), r.t1);
        QTextDocument snapDoc;
        check(snaps.size() == 1 && snaps.first().title.contains(QLatin1String("Secretword9")) && snapshots::load(p.get(), snaps.first(), &snapDoc, &err)
                  && snapDoc.toPlainText().contains(QLatin1String("Secretword1")),
              QStringLiteral("round trip: a snapshot and its list"), err);
        const objects::Object *o = p->objects()->object(r.objectId);
        check(o && o->name.contains(QLatin1String("Secretword7")) && !vault::readImage(p->objects()->picturePath(r.pictureName)).isNull(),
              QStringLiteral("round trip: an object and its picture"), p->objects()->warnings().join(QLatin1Char('\n')));
        mindmap::Board board;
        check(board.load(mindmap::pathFor(p->contentDir(r.mapId)), &err) && board.notes.size() == 1
                  && board.notes.first().text.contains(QLatin1String("Secretword8")),
              QStringLiteral("round trip: a mind map board"), err);
        check(!icons::iconFor(p.get(), QStringLiteral("custom:Lamp")).pixmap(16, 16).isNull(), QStringLiteral("round trip: an icon of the project's own"));
        {
            const QString own = settingslayer::fileFor(p->settingsDir());
            QSettings s(own, settingslayer::formatFor(own));
            check(s.value(QStringLiteral("editor/testKey")).toString().contains(QLatin1String("Secretword10")),
                  QStringLiteral("round trip: the project's own options (QSettings through the vault)"));
        }
        QByteArray look;
        check(vault::readFile(p->settingsDir() + QStringLiteral("/look.json"), &look) && look.contains("border"),
              QStringLiteral("round trip: the project's look"));
        check(p->uiState.value(QStringLiteral("testState")).toString() == QLatin1String("Secretword11"),
              QStringLiteral("round trip: this computer's window state (ui-<key>.json)"));
        QByteArray list;
        check(vault::readFile(p->titleListFile(), &list) && list.contains("Chapter One"), QStringLiteral("round trip: the list of documents by title"));
        check(vault::isSealedData(readAll(Project::spareCopyOf(r.index))), QStringLiteral("the spare copy of the project file made at the opening is encrypted"));
        SearchQuery q;
        q.text = QStringLiteral("Secretword2");
        check(searchProject(p.get(), q).size() == 1, QStringLiteral("Project Search finds a word of an encrypted text"));
        check(p->docs()->plainText(r.t3).contains(QLatin1String("Secretword3")), QStringLiteral("the plain text for search and counts is read decrypted"));
        check(p->item(r.t2)->wordCount > 3, QStringLiteral("counts work"));

        // writing again: an edited text, new notes, a snapshot, a changed object, a board, a new document
        setText(p.get(), p->item(r.t2), QStringLiteral("The second chapter, edited: Secretword12."));
        QTextCursor n(p->docs()->notes(r.t2));
        n.insertText(QStringLiteral("New notes Secretword13"));
        BinderItem *fresh = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter Three"));
        setText(p.get(), fresh, QStringLiteral("A new chapter Secretword14"));
        snapshots::take(p.get(), r.t2, QStringLiteral("Second snapshot"));
        objects::Object changed = *p->objects()->object(r.objectId);
        changed.name = QStringLiteral("Captain Changed");
        p->objects()->update(changed);
        check(p->save(&err), QStringLiteral("saving the encrypted project after changes"), err);
        check(p->autosave(&err), QStringLiteral("... and an autosave"), err);
        p->objects()->save(&err);
        workmarks::write(p->settingsDir() + QStringLiteral("/lock"), QDateTime::currentDateTime(), {r.t1});
        syncsafe::writeLock(p->settingsDir() + QStringLiteral("/lock"), QDateTime::currentDateTime());
        left = plainFilesIn(r.folder);
        check(left.isEmpty(), QStringLiteral("the walk after the writes (texts, notes, snapshots, objects, the shown lock): no plain file (%1 files)").arg(filesIn(r.folder)),
              left.join(QLatin1String(", ")));
        check(QFile::exists(p->settingsDir() + QStringLiteral("/lock")) && !vault::isSealedData(readAll(p->settingsDir() + QStringLiteral("/lock"))),
              QStringLiteral("the lock stays plain (readable before unlocking)"));

        // the waiting files of a save and its journal: a save cut off between its renames
        {
            const QString t1 = p->contentDir(r.t1) + QStringLiteral("/batch-a.md");
            const QString blocked = p->contentDir(r.t1) + QStringLiteral("/batch-b.md");
            QDir().mkpath(blocked); // a folder in the way: the second rename fails
            writeAll(blocked + QStringLiteral("/x"), "x");
            savebatch::Batch b(p->folderPath(), p->journalFile());
            {
                savebatch::Scope scope(&b);
                check(b.stage(t1, "batch text A Secretword15", QStringLiteral("A"), QString(), &err)
                          && b.stage(blocked, "batch text B Secretword16", QStringLiteral("B"), QString(), &err),
                      QStringLiteral("(two files staged)"), err);
                check(vault::isSealedData(readAll(t1 + QLatin1String(savebatch::kStageSuffix)))
                          && vault::isSealedData(readAll(blocked + QLatin1String(savebatch::kStageSuffix))),
                      QStringLiteral("the waiting files of a save are encrypted"));
                check(!b.commit(&err), QStringLiteral("(the save is cut off at its second rename)"), err);
            }
            check(QFile::exists(p->journalFile()) && vault::isSealedData(readAll(p->journalFile())),
                  QStringLiteral("the save journal is encrypted"));
            QDir(blocked).removeRecursively();
            const savebatch::Recovery rec = savebatch::recover(p->folderPath(), p->journalFile(), p->settingsDir());
            QByteArray got;
            check(rec.unreadable.isEmpty() && !QFile::exists(p->journalFile()) && vault::readFile(blocked, &got) && got.contains("Secretword16"),
                  QStringLiteral("the next opening's recovery reads the encrypted journal and finishes the save"), rec.text());
            QFile::remove(t1);
            QFile::remove(blocked);
        }

        // the emergency copy
        setText(p.get(), p->item(r.t1), QStringLiteral("Unsaved words Secretword17"));
        const EmergencyReport er = p->makeEmergencyCopy(W + QStringLiteral("/emergency"));
        left = plainFilesIn(er.dir, {QStringLiteral("README.txt")});
        check(!er.dir.isEmpty() && er.tally.written > 0 && left.isEmpty(),
              QStringLiteral("the emergency copy of an encrypted project is encrypted, all but its README (%1 files)").arg(er.tally.written),
              left.join(QLatin1String(", ")));
        check(QString::fromUtf8(readAll(er.dir + QStringLiteral("/README.txt"))).contains(QLatin1String("encrypted")),
              QStringLiteral("... and the README says so"));
        p->save(&err);
    }
    check(!vault::contains(r.folder), QStringLiteral("closed: the folder left the vault"));

    // a conflict copy and the snapshot of another computer's version stay encrypted
    {
        auto p = openOk(r.index, QStringLiteral("(opened again)"));
        if (!p)
            return;
        p->docs()->text(r.t3); // read here
        // another computer writes the text (through its sync program): its own encrypted version, a later time
        QByteArray other = "Another computer's version Secretword18";
        vault::seal(p->docs()->textPath(r.t3), other);
        QTest::qWait(1100);
        writeAll(p->docs()->textPath(r.t3), other);
        setText(p.get(), p->item(r.t3), QStringLiteral("This computer's version Secretword19"));
        QStringList kept;
        QObject::connect(p.get(), &Project::conflictKept, [&kept](const QString &, const QString &, const QString &copy, bool) { kept << copy; });
        check(p->save(&err), QStringLiteral("(saved over another computer's version)"), err);
        check(kept.size() == 1 && !kept.first().isEmpty() && vault::isSealedData(readAll(kept.first())),
              QStringLiteral("the other version kept in settings/conflicts is encrypted"), kept.join(QLatin1String(", ")));
        QByteArray keptText;
        check(!kept.isEmpty() && vault::readFile(kept.first(), &keptText) && keptText.contains("Secretword18"),
              QStringLiteral("... and holds that version"));
        const QList<Snapshot> snaps = snapshots::list(p.get(), r.t3);
        check(!snaps.isEmpty() && vault::isSealedData(readAll(snaps.first().file)), QStringLiteral("... and so is its snapshot"));
        // a sync program's own conflict copy of an encrypted file is a copy of the encrypted bytes
        const QString syncCopy = p->docs()->textPath(r.t3) + QStringLiteral(" (conflicted copy)");
        QFile::copy(p->docs()->textPath(r.t3), syncCopy);
        check(vault::isSealedData(readAll(syncCopy)), QStringLiteral("a sync program's conflict copy of a file stays encrypted"));
        QFile::remove(syncCopy);
        left = plainFilesIn(r.folder);
        check(left.isEmpty(), QStringLiteral("the walk after the conflict: no plain file"), left.join(QLatin1String(", ")));
    }

    // turning it off again: every file back as it was
    check(decryptFolder(r.folder, {}, &err), QStringLiteral("decryptFolder turns the project's encryption off"), err);
    check(!settingsOf(r.folder).encryptedProject && settingsOf(r.folder).encryptedBackups,
          QStringLiteral("the marker: no encrypted project, the backups stay encrypted (the writer turns that off on its own)"));
    int plainCount = 0;
    QDirIterator it2(r.folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool sealedLeft = false;
    while (it2.hasNext()) {
        sealedLeft = sealedLeft || vault::isSealedData(readAll(it2.next()).left(64));
        ++plainCount;
    }
    check(!sealedLeft, QStringLiteral("every file is plain again (%1 files)").arg(plainCount));
    {
        auto p = openOk(r.index, QStringLiteral("the decrypted project opens"));
        if (p)
            check(!p->isEncrypted() && p->docs()->text(r.t2)->toPlainText().contains(QLatin1String("Secretword12")),
                  QStringLiteral("... plain, with its latest text"));
    }
    // a plain project that was never encrypted: its files did not change their bytes through a round trip
    {
        const Rich q = makeRich(W, QStringLiteral("Round Trip"));
        const QMap<QString, QByteArray> before = fingerprintOf(q.folder);
        encryptFolder(q.folder, recipients(), {}, &err);
        decryptFolder(q.folder, {}, &err);
        QMap<QString, QByteArray> after = fingerprintOf(q.folder);
        after.remove(markerName());
        check(before == after, QStringLiteral("on and off: every file has its old bytes again (%1 files)").arg(before.size()));
        Q_UNUSED(plainBefore);
    }
}

// ------------------------------------------------------------------------- a conversion cut off after n files

static void cutOff(const QString &W)
{
    std::printf("== a conversion cut off after n files leaves an openable project, and the next opening finishes it\n");
    const Rich r = makeRich(W, QStringLiteral("Cut Off"));
    QString err;
    const int total = filesIn(r.folder);
    qputenv("QB_TEST_CONVERT_STOP", "5");
    check(!encryptFolder(r.folder, recipients(), {}, &err), QStringLiteral("(the encryption is stopped after 5 files, as by a kill)"), err);
    qunsetenv("QB_TEST_CONVERT_STOP");
    const QStringList plainLeft = plainFilesIn(r.folder);
    check(settingsOf(r.folder).converting == QLatin1String("encrypt") && plainLeft.size() == total - 5,
          QStringLiteral("the marker says the encryption is under way; 5 files are new, %1 old").arg(plainLeft.size()));
    {
        auto p = openOk(r.index, QStringLiteral("the project with old and new files opens"));
        if (p) {
            check(p->docs()->text(r.t1)->toPlainText().contains(QLatin1String("Secretword1"))
                      && p->docs()->text(r.t3)->toPlainText().contains(QLatin1String("Secretword3"))
                      && p->docs()->notes(r.t1)->toPlainText().contains(QLatin1String("Secretword4")),
                  QStringLiteral("... and every text reads (plain or encrypted)"));
            check(p->openNotice.contains(QLatin1String("not finished")), QStringLiteral("... and it says that the encryption is not finished"), p->openNotice);
            setText(p.get(), p->item(r.t2), QStringLiteral("Written while half encrypted Secretword20"));
            p->save(&err);
            check(vault::isSealedData(readAll(p->docs()->textPath(r.t2))), QStringLiteral("a file written meanwhile is written encrypted (the direction of the conversion)"));
        }
    }
    check(finishConversion(r.folder, {}, &err) && settingsOf(r.folder).converting.isEmpty() && plainFilesIn(r.folder).isEmpty(),
          QStringLiteral("finishConversion encrypts the rest"), err);
    {
        auto p = openOk(r.index, QStringLiteral("(opened after the finish)"));
        if (p)
            check(p->docs()->text(r.t2)->toPlainText().contains(QLatin1String("Secretword20")), QStringLiteral("nothing was lost"));
    }
    // the other way
    qputenv("QB_TEST_CONVERT_STOP", "4");
    check(!decryptFolder(r.folder, {}, &err), QStringLiteral("(the decryption is stopped after 4 files)"), err);
    qunsetenv("QB_TEST_CONVERT_STOP");
    check(settingsOf(r.folder).converting == QLatin1String("decrypt") && plainFilesIn(r.folder).size() == 4,
          QStringLiteral("the marker says the decryption is under way; 4 files are plain"));
    {
        auto p = openOk(r.index, QStringLiteral("the half-decrypted project opens"));
        if (p) {
            check(p->docs()->text(r.t2)->toPlainText().contains(QLatin1String("Secretword20")) && p->docs()->text(r.t1)->toPlainText().contains(QLatin1String("Secretword1")),
                  QStringLiteral("... and every text reads"));
            setText(p.get(), p->item(r.t3), QStringLiteral("Written while half decrypted Secretword21"));
            p->save(&err);
            check(!vault::isSealedData(readAll(p->docs()->textPath(r.t3))), QStringLiteral("a file written meanwhile is written plain (the direction)"));
        }
    }
    // a leftover of a file cut off in the middle of its own conversion is no project data
    writeAll(r.folder + QStringLiteral("/content/leftover.md.qbconvert"), "half a file");
    check(finishConversion(r.folder, {}, &err) && settingsOf(r.folder).converting.isEmpty(), QStringLiteral("finishConversion decrypts the rest"), err);
    check(!QFile::exists(r.folder + QStringLiteral("/content/leftover.md.qbconvert")), QStringLiteral("... and removes what a conversion cut off left"));
    QDirIterator it(r.folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool sealedLeft = false;
    while (it.hasNext())
        sealedLeft = sealedLeft || vault::isSealedData(readAll(it.next()).left(64));
    check(!sealedLeft, QStringLiteral("... every file is plain"));
}

// ----------------------------------------------------------------- wrong passphrase, missing key, damaged file

static void keysAndDamage(const QString &W)
{
    std::printf("== a wrong passphrase, a missing key (nothing written), a damaged file\n");
    QString err;
    const Rich r = makeRich(W, QStringLiteral("Locked Book"));
    encryptFolder(r.folder, recipients(), {}, &err);
    KeyRing &ring = KeyRing::device();
    ring.lock(g_writer);
    check(unlockAndRemember(g_writer, "not the passphrase") == Error::WrongPassphrase && !ring.isUnlocked(g_writer),
          QStringLiteral("a wrong passphrase does not unlock the key"));
    // the key store holds nothing yet (memory store of the test): the opening needs the passphrase
    KeyStore::system().remove(KeyStore::passphraseEntry(g_writer));
    QString fp;
    check(accessFor(r.folder, &fp) == Access::NeedsPassphrase && fp == g_writer, QStringLiteral("a locked key: the opening needs the passphrase"));
    const QMap<QString, QByteArray> before = fingerprintOf(r.folder);
    auto p = Project::open(r.index, &err);
    check(!p && err == errorText(Error::Locked), QStringLiteral("Project::open refuses while the key is locked"), err);
    check(fingerprintOf(r.folder) == before, QStringLiteral("... and writes nothing (every file as it was, no new one)"));
    bool kept = false;
    check(unlockAndRemember(g_writer, kWriterPass, &kept) == Error::None && kept && ring.isUnlocked(g_writer),
          QStringLiteral("the right passphrase unlocks the key and is kept in the key store of this device"));
    QByteArray stored;
    check(KeyStore::system().read(KeyStore::passphraseEntry(g_writer), &stored) == KeyStore::Status::Ok && stored == kWriterPass,
          QStringLiteral("... under the key's entry"));
    ring.lock(g_writer);
    check(accessFor(r.folder) == Access::Ready && ring.isUnlocked(g_writer), QStringLiteral("the next session unlocks it from the key store, without asking"));

    // a key that is not on this device
    {
        KeyRing other;
        const QString strangerWriter = other.createKey(QStringLiteral("Someone Else"), "a passphrase of someone else");
        const QString strangerRecovery = other.createKey(QStringLiteral("Someone Else Recovery"), "another passphrase of someone");
        ring.importKeys(other.exportPublicKey(strangerWriter) + other.exportPublicKey(strangerRecovery));
        const Rich s = makeRich(W, QStringLiteral("Foreign Book"));
        check(encryptFolder(s.folder, Recipients{strangerWriter, strangerRecovery}, {}, &err),
              QStringLiteral("(a project encrypted to keys whose secret parts are elsewhere: only the public keys are needed)"), err);
        const QMap<QString, QByteArray> fb = fingerprintOf(s.folder);
        check(accessFor(s.folder, &fp) == Access::NoSecretKey && fp == strangerWriter, QStringLiteral("the opening says the key is not here"));
        auto q = Project::open(s.index, &err);
        check(!q && err.contains(groupedFingerprint(strangerWriter)) && err.contains(QLatin1String("Nothing in the project was changed")),
              QStringLiteral("Project::open refuses with the sentence that names the keys"), err);
        check(fingerprintOf(s.folder) == fb, QStringLiteral("a project whose key is missing is not written (no journal recovery, no spare copy, nothing)"));
        // the window: the message with Import Keys
        int asked = 0;
        QStringList named;
        setImportKeysHandler([&](QWidget *, const QStringList &needed) {
            ++asked;
            named = needed;
            return false;
        });
        MainWindow w;
        w.resize(900, 600);
        w.show();
        QTest::qWait(100);
        QTimer::singleShot(300, [&] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            check(box && box->objectName() == QLatin1String("encryptionKeyMissing"), QStringLiteral("the window shows the message that the key is not here"));
            if (!box)
                return;
            box->grab().save(outDir + QStringLiteral("/v0990f-key-missing.png"));
            auto *import = box->findChild<QPushButton *>(QStringLiteral("encryptionImportKeys"));
            check(import != nullptr, QStringLiteral("... with the button Import Keys"));
            if (import)
                import->click();
            else
                box->reject();
        });
        check(!w.openProject(s.index), QStringLiteral("(the window does not open it)"));
        check(asked == 1 && named == Recipients{strangerWriter, strangerRecovery}.list(), QStringLiteral("Import Keys calls the window of the key part with the keys the project needs"));
        check(fingerprintOf(s.folder) == fb, QStringLiteral("... and the project stays as it was"));
        setImportKeysHandler({});
    }

    // a damaged file: the document says so, the rest opens, nothing is written over it
    {
        const QString file = r.folder + QStringLiteral("/content/") + r.t2 + QStringLiteral("/text.md");
        QByteArray bytes = readAll(file);
        bytes[bytes.size() - 30] = char(bytes.at(bytes.size() - 30) ^ 0x55);
        writeAll(file, bytes);
        auto q = openOk(r.index, QStringLiteral("a project with a damaged encrypted file opens"));
        if (!q)
            return;
        QString told;
        QObject::connect(q->docs(), &DocumentStore::unreadableFile, [&told](const QString &, bool, const QString &, const QString &e) { told = e; });
        q->docs()->text(r.t2);
        check(q->docs()->isUnreadable(r.t2) && told == errorText(Error::Damaged), QStringLiteral("the damaged document says so: %1").arg(told));
        check(q->docs()->text(r.t1)->toPlainText().contains(QLatin1String("Secretword1")), QStringLiteral("the other documents open"));
        q->docs()->text(r.t2)->setPlainText(QStringLiteral("typed into a document that cannot be read"));
        q->save(&err);
        check(readAll(file) == bytes, QStringLiteral("the damaged file is never written over"));
    }
}


// ------------------------------------------------------------- the findings of the review of the encrypted project

static QStringList fileNamesIn(const QString &folder)
{
    QStringList out;
    QDirIterator it(folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
        out << QDir(folder).relativeFilePath(it.next());
    out.sort();
    return out;
}

static QString joinedText(const QString &folder)
{
    QString all = fileNamesIn(folder).join(QLatin1Char('\n'));
    return all;
}

static void reviewFindings(const QString &W)
{
    std::printf("== the review's findings: copies stay encrypted, missing keys, damaged marker, stray files, titles hidden\n");
    QString err;
    KeyRing &ring = KeyRing::device();
    ring.unlock(g_writer, kWriterPass);

    // H1: Save As and Save a Copy of an encrypted project are encrypted
    {
        const Rich r = makeRich(W, QStringLiteral("Copy Source"));
        encryptFolder(r.folder, recipients(), {}, &err);
        auto p = openOk(r.index, QStringLiteral("(an encrypted project to copy)"));
        if (p) {
            QDir().mkpath(W + QStringLiteral("/saveas"));
            auto into = [&p](const QString &folder, const QString &indexName, QString *e) {
                vault::Scope encrypted(folder);
                QString why;
                const bool ok = p->writeUnsavedInto(folder, indexName, &why);
                if (!ok && e)
                    *e = why;
                return ok;
            };
            auto np = scrivener::saveAsQuillbinder(p.get(), W + QStringLiteral("/saveas"), QStringLiteral("As Copy"), &err, into);
            check(np != nullptr, QStringLiteral("(Save As of an encrypted project)"), err);
            if (np) {
                const QStringList left = plainFilesIn(np->folderPath());
                check(np->isEncrypted() && left.isEmpty(), QStringLiteral("H1: the Save As copy has no plain file (%1 files)").arg(filesIn(np->folderPath())),
                      left.join(QLatin1String(", ")));
                check(np->docs()->text(r.t1)->toPlainText().contains(QLatin1String("Secretword1")), QStringLiteral("... and opens with its texts"));
            }
            auto cp = scrivener::copyAsQuillbinder(p.get(), W + QStringLiteral("/saveas"), QStringLiteral("A Second Copy"), &err);
            check(cp != nullptr, QStringLiteral("(Save a Copy of an encrypted project)"), err);
            if (cp) {
                const QStringList left = plainFilesIn(cp->folderPath());
                check(cp->isEncrypted() && left.isEmpty(), QStringLiteral("H1: the Save a Copy folder has no plain file"), left.join(QLatin1String(", ")));
            }
        }
    }

    // H3: a recipient's public key missing here: the project is not opened (nothing is written)
    {
        const Rich r = makeRich(W, QStringLiteral("Missing Public Key"));
        encryptFolder(r.folder, recipients(), {}, &err);
        const QByteArray pub = ring.exportPublicKey(g_recovery);
        ring.removeKey(g_recovery, false);
        const QMap<QString, QByteArray> before = fingerprintOf(r.folder);
        QString why;
        auto p = Project::open(r.index, &why);
        check(!p && why.contains(groupedFingerprint(g_recovery)), QStringLiteral("H3: no public key of the recovery key here: the project is not opened"), why);
        check(fingerprintOf(r.folder) == before, QStringLiteral("... and nothing in its folder changed"));
        p.reset();
        check(ring.importKeys(pub).imported.contains(g_recovery), QStringLiteral("(the recovery key's public part is back)"));
        auto q = openOk(r.index, QStringLiteral("... it opens with both public keys"));
    }

    // M1: another computer's encrypted file in a folder that was decrypted here
    {
        const Rich r = makeRich(W, QStringLiteral("Stray Encrypted"));
        encryptFolder(r.folder, recipients(), {}, &err);
        check(decryptFolder(r.folder, {}, &err), QStringLiteral("(the project is decrypted)"), err);
        const QString file = r.folder + QStringLiteral("/content/") + r.t2 + QStringLiteral("/text.md");
        QByteArray stray = "Stray text of another computer Secretword30";
        Error e = Error::None;
        const QByteArray sealed = ring.encrypt(stray, recipients().list(), &e);
        check(e == Error::None && writeAll(file, sealed), QStringLiteral("(an encrypted text arrives in the plain folder)"));
        auto p = openOk(r.index, QStringLiteral("(the decrypted project opens)"));
        if (p) {
            const QString shown = p->docs()->text(r.t2)->toPlainText();
            check(shown.contains(QLatin1String("Secretword30")) || (p->docs()->isUnreadable(r.t2) && shown.isEmpty()),
                  QStringLiteral("M1: the late encrypted file is shown decrypted or read-only, never as ciphertext"), shown.left(80));
            p->docs()->text(r.t2)->setPlainText(QStringLiteral("typed over it"));
            p->save(&err);
        }
        // a lock of another computer: the encryption is not turned on or off then
        QString reason;
        auto q = openOk(r.index, QStringLiteral("(opened again)"));
        if (q) {
            const QString lock = q->settingsDir() + QStringLiteral("/lock");
            writeAll(lock, QStringLiteral("otherhost 4242\ndevice=otherhost\nkey=0123456789abcdef\nalive=%1\n")
                               .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8());
            check(!canChangeProjectEncryption(q.get(), &reason) && !reason.isEmpty(),
                  QStringLiteral("M1: no conversion while another computer has the project open: %1").arg(reason));
            QFile::remove(lock);
            check(canChangeProjectEncryption(q.get(), &reason), QStringLiteral("... and with its lock gone, it can be changed"), reason);
        }
    }

    // M3: a marker that cannot be read, or none beside an encrypted project file: refused, nothing written
    {
        const Rich r = makeRich(W, QStringLiteral("Damaged Marker"));
        encryptFolder(r.folder, recipients(), {}, &err);
        const QString marker = r.folder + QStringLiteral("/") + markerName();
        const QByteArray good = readAll(marker);
        const QString bdir = W + QStringLiteral("/marker-backups");
        // a backup after the marker was damaged
        {
            auto p = openOk(r.index, QStringLiteral("(an encrypted project with its marker)"));
            if (p) {
                writeAll(marker, "{ this is not json");
                QString m, why;
                const bool made = backup::backupProject(p.get(), bdir, 3, &m, &why);
                check(!made && filesIn(bdir) == 0, QStringLiteral("M3: with a damaged marker no backup is made (never a plain one)"), why);
                writeAll(marker, good);
            }
        }
        // a leftover of a save cut off (an hour old): an opening that writes would clean it up
        const QString leftover = r.folder + QStringLiteral("/content/") + r.t1 + QStringLiteral("/text.md.AbCd12");
        writeAll(leftover, "left by a save that was cut off");
        {
            QFile f(leftover);
            f.open(QIODevice::ReadWrite);
            f.setFileTime(QDateTime::currentDateTime().addSecs(-7200), QFileDevice::FileModificationTime);
        }
        writeAll(marker, "{ this is not json");
        const QMap<QString, QByteArray> before = fingerprintOf(r.folder);
        QString why;
        auto p = Project::open(r.index, &why);
        check(!p && !why.isEmpty(), QStringLiteral("M3: a damaged marker: the project is not opened"), why);
        check(fingerprintOf(r.folder) == before, QStringLiteral("... and its folder is as it was (no recovery, no clean-up)"));
        QFile::remove(marker);
        {
            QFile f(leftover);
            f.open(QIODevice::ReadWrite);
            f.setFileTime(QDateTime::currentDateTime().addSecs(-7200), QFileDevice::FileModificationTime);
        }
        const QMap<QString, QByteArray> before2 = fingerprintOf(r.folder);
        auto q = Project::open(r.index, &why);
        check(!q && !why.isEmpty(), QStringLiteral("M3: no marker beside an encrypted project file: not opened"), why);
        check(fingerprintOf(r.folder) == before2, QStringLiteral("... and nothing written"));
        writeAll(marker, good);
        auto ok = openOk(r.index, QStringLiteral("(with the marker back it opens)"));
    }

    // L1: a file whose first bytes are damaged is not taken as plain
    {
        const Rich r = makeRich(W, QStringLiteral("Damaged Head"));
        encryptFolder(r.folder, recipients(), {}, &err);
        const QString file = r.folder + QStringLiteral("/content/") + r.t2 + QStringLiteral("/text.md");
        QByteArray bytes = readAll(file);
        bytes = "Words that are plain now, because the first bytes of the file are damaged. Secretword99";
        writeAll(file, bytes);
        auto q = openOk(r.index, QStringLiteral("(a project with a damaged first byte opens)"));
        if (q) {
            q->docs()->text(r.t2);
            check(q->docs()->isUnreadable(r.t2), QStringLiteral("L1: the file with damaged first bytes is unreadable, not plain text"));
            q->docs()->text(r.t2)->setPlainText(QStringLiteral("typed into a document that cannot be read"));
            q->save(&err);
            check(readAll(file) == bytes, QStringLiteral("... and never written over"));
        }
    }

    // L7: a restore's plain ZIP file left by a crash is swept (once it is an hour old)
    {
        const QString work = backup::workDirectory();
        QDir().mkpath(work);
        const QString oldOne = work + QStringLiteral("/restore-0123456789abcdef.zip");
        const QString newOne = work + QStringLiteral("/restore-fedcba9876543210.zip");
        writeAll(oldOne, "left by a crash");
        writeAll(newOne, "a restore that runs now");
        {
            QFile f(oldOne);
            f.open(QIODevice::ReadWrite);
            f.setFileTime(QDateTime::currentDateTime().addSecs(-7200), QFileDevice::FileModificationTime);
        }
        const Rich r = makeRich(W, QStringLiteral("Sweeper"));
        auto p = openOk(r.index, QStringLiteral("(a project for a backup)"));
        QString m, why;
        if (p)
            check(backup::backupProject(p.get(), W + QStringLiteral("/sweep-backups"), 3, &m, &why), QStringLiteral("(a backup)"), why);
        check(!QFile::exists(oldOne) && QFile::exists(newOne), QStringLiteral("L7: the old restore-*.zip is swept, a young one stays"));
        QFile::remove(newOne);
    }

    // the titles: conflict copies, interrupted-save copies and the emergency copy carry the document's number, not its
    // title, in an encrypted project; a plain project keeps its names
    auto conflictCopy = [&](const QString &name, bool encrypted) {
        const Rich r = makeRich(W, name);
        if (encrypted)
            encryptFolder(r.folder, recipients(), {}, &err);
        auto p = openOk(r.index, QStringLiteral("(a project for a conflict)"));
        QString kept;
        if (!p)
            return std::make_pair(r, kept);
        p->docs()->text(r.t3);
        QByteArray other = "Another computer's version Secretword40";
        vault::seal(p->docs()->textPath(r.t3), other);
        QTest::qWait(1100);
        writeAll(p->docs()->textPath(r.t3), other);
        setText(p.get(), p->item(r.t3), QStringLiteral("This computer's version Secretword41"));
        QObject::connect(p.get(), &Project::conflictKept, [&kept](const QString &, const QString &, const QString &copy, bool) { kept = copy; });
        p->save(&err);
        return std::make_pair(r, kept);
    };
    {
        const auto c = conflictCopy(QStringLiteral("Titles Plain"), false);
        check(!c.second.isEmpty() && QFileInfo(c.second).fileName().contains(QLatin1String("Research Note")),
              QStringLiteral("a plain project keeps the title in its conflict copy's name"), c.second);
        const auto e = conflictCopy(QStringLiteral("Titles Hidden"), true);
        const QString n = QFileInfo(e.second).fileName();
        check(!e.second.isEmpty() && !n.contains(QLatin1String("Research")) && n.contains(e.first.t3),
              QStringLiteral("titles: an encrypted project's conflict copy is named by the document's number"), n);
    }
    {
        const Rich r = makeRich(W, QStringLiteral("Titles Interrupted"));
        encryptFolder(r.folder, recipients(), {}, &err);
        auto p = openOk(r.index, QStringLiteral("(a project for an interrupted save)"));
        if (p) {
            const QString t1 = p->contentDir(r.t1) + QStringLiteral("/batch-a.md");
            const QString blocked = p->contentDir(r.t1) + QStringLiteral("/batch-b.md");
            QDir().mkpath(blocked);
            writeAll(blocked + QStringLiteral("/x"), "x");
            savebatch::Batch b(p->folderPath(), p->journalFile());
            {
                savebatch::Scope scope(&b);
                b.stage(t1, "batch text A", QStringLiteral("Chapter One"), QString(), &err);
                b.stage(blocked, "batch text B", QStringLiteral("Chapter Two"), QString(), &err);
                b.commit(&err); // cut off at the second rename
            }
            QDir(blocked).removeRecursively();
            QByteArray changed = "another computer's batch-b";
            vault::seal(blocked, changed);
            writeAll(blocked, changed);
            const savebatch::Recovery rec = savebatch::recover(p->folderPath(), p->journalFile(), p->settingsDir());
            QStringList copies;
            for (const QString &f : fileNamesIn(p->settingsDir() + QStringLiteral("/conflicts")))
                copies << f;
            bool anyTitle = false;
            for (const QString &f : std::as_const(copies))
                anyTitle = anyTitle || f.contains(QLatin1String("Chapter"));
            check(!copies.isEmpty() && !anyTitle, QStringLiteral("titles: the interrupted-save copy of an encrypted project carries no title"),
                  copies.join(QLatin1String(", ")) + QLatin1Char(' ') + rec.text());
            QFile::remove(t1);
        }
    }
    {
        const Rich r = makeRich(W, QStringLiteral("Hidden Titles Book"));
        encryptFolder(r.folder, recipients(), {}, &err);
        auto p = openOk(r.index, QStringLiteral("(an encrypted project with unsaved text)"));
        if (p) {
            setText(p.get(), p->item(r.t1), QStringLiteral("Unsaved words Secretword42"));
            QTextCursor n(p->docs()->notes(r.t1));
            n.insertText(QStringLiteral(" more notes Secretword43"));
            const EmergencyReport er = p->makeEmergencyCopy(W + QStringLiteral("/emergency-hidden"));
            const QString folderName = QFileInfo(er.dir).fileName();
            static const QRegularExpression dateOnly(QStringLiteral("^\\d{4}-\\d\\d-\\d\\d \\d\\d-\\d\\d-\\d\\d( \\(\\d+\\))?$"));
            check(!er.dir.isEmpty() && dateOnly.match(folderName).hasMatch(),
                  QStringLiteral("titles: the emergency folder of an encrypted project is named by its date"), folderName);
            const QString names = joinedText(er.dir);
            check(!names.isEmpty() && !names.contains(QLatin1String("Chapter")) && !names.contains(QLatin1String("Hidden")) && names.contains(r.t1),
                  QStringLiteral("... its files are named by the document's number"), names);
            const QString readme = QString::fromUtf8(readAll(er.dir + QStringLiteral("/README.txt")));
            check(!readme.isEmpty() && !readme.contains(QLatin1String("Hidden")) && !readme.contains(QLatin1String("Chapter")),
                  QStringLiteral("... and its README.txt names no title"), readme.left(300));
            p->save(&err);
        }
    }
    {
        const Rich r = makeRich(W, QStringLiteral("Plain Titles Book"));
        auto p = openOk(r.index, QStringLiteral("(a plain project with unsaved text)"));
        if (p) {
            setText(p.get(), p->item(r.t1), QStringLiteral("Unsaved words Secretword44"));
            const EmergencyReport er = p->makeEmergencyCopy(W + QStringLiteral("/emergency-plain"));
            check(QFileInfo(er.dir).fileName().contains(QLatin1String("Plain Titles Book")) && QFile::exists(er.dir + QStringLiteral("/Chapter One.rtf")),
                  QStringLiteral("a plain project's emergency copy keeps its names"), QFileInfo(er.dir).fileName());
            p->save(&err);
        }
    }
}

// --------------------------------------------------------------------------------------------- the backups

static QString gpgProgram()
{
    const QString extra = qEnvironmentVariable("QB_TEST_GPG_EXTRA");
    if (!extra.isEmpty() && QFileInfo(extra).isExecutable())
        return extra;
    return QStandardPaths::findExecutable(QStringLiteral("gpg"));
}

static QByteArray gpgDecrypt(const QString &gpg, const QString &home, const QString &file, QString *why)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("GNUPGHOME"), home);
    env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    auto run = [&](const QStringList &args, QByteArray *out) {
        QProcess pr;
        pr.setProcessEnvironment(env);
        pr.start(gpg, QStringList{QStringLiteral("--batch"), QStringLiteral("--no-tty"), QStringLiteral("--yes"),
                                  QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"),
                                  QString::fromUtf8(kWriterPass)} + args);
        pr.waitForFinished(60000);
        if (out)
            *out = pr.readAllStandardOutput();
        *why = QString::fromUtf8(pr.readAllStandardError()).right(600);
        return pr.exitCode() == 0;
    };
    QByteArray out;
    run({QStringLiteral("--decrypt"), file}, &out);
    return out;
}

static void backups(const QString &W)
{
    std::printf("== encrypted backups: both formats, names, the newest-N rule, restore, Move Old Backups, GnuPG\n");
    QString err;
    const QString bdir = W + QStringLiteral("/backups");
    const Rich r = makeRich(W, QStringLiteral("Backed Up"));
    auto p = openOk(r.index, QStringLiteral("(a plain project)"));
    if (!p)
        return;
    QString reason;
    check(canChangeBackupEncryption(p.get(), &reason) && canChangeProjectEncryption(p.get(), &reason), QStringLiteral("both switches can be changed for it"), reason);
    check(setBackupsEncrypted(p.get(), true, recipients(), &err) && backupsEncrypted(p.get()) && !projectEncrypted(p.get()),
          QStringLiteral("switch 1 on: encrypted backups of a plain project"), err);
    QString made;
    const bool madeOk = backup::backupProject(p.get(), bdir, 2, &made, &err);
    check(madeOk && made.endsWith(QLatin1String(".zip.gpg")) && err.isEmpty(),
          QStringLiteral("a backup is <name>.zip.gpg: %1").arg(QFileInfo(made).fileName()), err);
    check(backup::isOwnBackupName(QFileInfo(made).fileName()) && QFileInfo(made).fileName().startsWith(backup::nameStart(p.get())),
          QStringLiteral("... in the program's own name form, with the project's ID"));
    check(isWholeMessage(made) && vault::isSealedData(readAll(made).left(64)) && !readAll(made).contains("Secretword"),
          QStringLiteral("... one whole OpenPGP message, nothing of the project readable in it"));
    check(messageKeyIds(made).size() == 2, QStringLiteral("... encrypted to two keys (the writer's and the recovery key)"));
    const QStringList work = QDir(backup::workFolderFor(bdir)).entryList(QDir::Files);
    check(!work.join(QLatin1Char(' ')).contains(QLatin1String(".zip")), QStringLiteral("no plain ZIP file is left in the work folder"), work.join(QLatin1String(", ")));
    // the check through decryption needs an unlocked key; with the key locked only the public keys are used
    KeyRing::device().lock(g_writer);
    QTest::qWait(1100);
    QString made2;
    check(backup::backupProject(p.get(), bdir, 2, &made2, &err) && made2 != made && made2.endsWith(QLatin1String(".zip.gpg")),
          QStringLiteral("an encrypted backup is made with the key locked (only the public keys are needed)"), err);
    KeyRing::device().unlock(g_writer, kWriterPass);
    check(KeyRing::device().isUnlocked(g_writer), QStringLiteral("(the key is unlocked again)"));
    // a plain backup among them (made before the switch) counts too
    setBackupsEncrypted(p.get(), false, {}, &err);
    QTest::qWait(1100);
    QString plainOne;
    backup::backupProject(p.get(), bdir, 0, &plainOne, &err);
    check(plainOne.endsWith(QLatin1String(".zip")) && !QFile::exists(markerPath(r.folder)), QStringLiteral("switch 1 off: a plain backup, the marker is gone"));
    setBackupsEncrypted(p.get(), true, recipients(), &err);
    QStringList list = backup::backupsFor(p.get(), bdir);
    check(list.size() == 3 && list.contains(made) && list.contains(made2) && list.contains(plainOne),
          QStringLiteral("Restore From Backup lists plain and encrypted backups (%1)").arg(list.size()));
    QTest::qWait(1100);
    QString made3;
    backup::backupProject(p.get(), bdir, 2, &made3, &err);
    list = backup::backupsFor(p.get(), bdir);
    check(list.size() == 2 && list.contains(made3) && !list.contains(made) && !list.contains(made2),
          QStringLiteral("the newest-N rule (2) counts encrypted and plain backups together"), list.join(QLatin1String(", ")));
    // a cut-off copy of an encrypted backup in the way is no backup
    {
        QByteArray whole = readAll(made3);
        check(isWholeMessage(made3) && !whole.isEmpty(), QStringLiteral("(a whole encrypted backup is whole)"));
        writeAll(W + QStringLiteral("/cut.zip.gpg"), whole.left(whole.size() / 2));
        check(!isWholeMessage(W + QStringLiteral("/cut.zip.gpg")), QStringLiteral("half of one is not whole (judged without a key)"));
    }
    // restore
    {
        QString projectFile;
        const QString dest = backup::restore(made3, W + QStringLiteral("/restored"), &projectFile, &err);
        check(!dest.isEmpty() && QFile::exists(projectFile), QStringLiteral("an encrypted backup is restored (decrypted first)"), err);
        auto q = Project::open(projectFile, &err);
        check(q && q->docs()->text(r.t1)->toPlainText().contains(QLatin1String("Secretword1")), QStringLiteral("... and the restored project holds the text"), err);
        const QStringList left = QDir(backup::workDirectory()).entryList({QStringLiteral("restore-*")}, QDir::Files);
        check(left.isEmpty(), QStringLiteral("... and its plain copy is gone"), left.join(QLatin1String(", ")));
        KeyRing::device().lock(g_writer);
        Error kind = Error::None;
        const QString none = backup::restore(made3, W + QStringLiteral("/restored2"), &projectFile, &err, &kind);
        check(none.isEmpty() && kind == Error::Locked && !QDir(W + QStringLiteral("/restored2")).exists(),
              QStringLiteral("with the key locked: nothing is restored, the kind says Locked"), err);
        KeyRing::device().unlock(g_writer, kWriterPass);
    }
    // Move Old Backups knows .zip.gpg
    {
        const QString former = bdir + QLatin1Char('/') + text::safeFileName(p->title()) + QLatin1Char(' ') + backup::projectKey(p.get());
        QDir().mkpath(former);
        const QString old = former + QStringLiteral("/") + text::safeFileName(p->title()) + QStringLiteral(" bak 2026-01-02 03-04-05.zip.gpg");
        QFile::copy(made3, old);
        check(backup::formerBackupsFor(p.get(), bdir).contains(old), QStringLiteral("an encrypted backup in the folder of 0.70 to 0.90 is found"));
        const backup::MoveReport mr = backup::moveFormerBackups(p.get(), bdir);
        check(mr.moved.size() == 1 && QFile::exists(bdir + QLatin1Char('/') + QFileInfo(old).fileName()),
              QStringLiteral("Move Old Backups Into the Backup Folder moves it"), mr.left.join(QLatin1String(", ")));
    }
    // GnuPG opens a backup with the exported secret key
    const QString gpg = gpgProgram();
    if (gpg.isEmpty()) {
        info(QStringLiteral("(no gpg on this system: the check with GnuPG is skipped)"));
    } else {
        QTemporaryDir home(QStringLiteral("/tmp/qbgpgf-XXXXXX"));
        QFile::setPermissions(home.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        writeAll(home.path() + QStringLiteral("/secret.asc"), KeyRing::device().exportSecretKey(g_writer));
        QString why;
        gpgDecrypt(gpg, home.path(), QString(), &why); // (starts the agent)
        QProcess imp;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("GNUPGHOME"), home.path());
        imp.setProcessEnvironment(env);
        imp.start(gpg, {QStringLiteral("--batch"), QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"),
                        QString::fromUtf8(kWriterPass), QStringLiteral("--import"), home.path() + QStringLiteral("/secret.asc")});
        imp.waitForFinished(60000);
        const QByteArray zip = gpgDecrypt(gpg, home.path(), made3, &why);
        writeAll(home.path() + QStringLiteral("/backup.zip"), zip);
        ZipReader zr(home.path() + QStringLiteral("/backup.zip"));
        check(zr.isValid() && zr.fileNames().size() > 5, QStringLiteral("GnuPG (%1) opens an encrypted backup with the exported secret key: a ZIP file of %2 files")
                                                            .arg(gpg).arg(zr.fileNames().size()), why);
        // and a text of an encrypted project
        const Rich e = makeRich(W, QStringLiteral("For GnuPG"));
        encryptFolder(e.folder, recipients(), {}, &err);
        const QByteArray text = gpgDecrypt(gpg, home.path(), e.folder + QStringLiteral("/content/") + e.t1 + QStringLiteral("/text.md"), &why);
        check(text.contains("Secretword1"), QStringLiteral("GnuPG opens a text of an encrypted project"), why);
        QProcess::execute(QStandardPaths::findExecutable(QStringLiteral("gpgconf")).isEmpty() ? QStringLiteral("true") : QStandardPaths::findExecutable(QStringLiteral("gpgconf")),
                          {QStringLiteral("--homedir"), home.path(), QStringLiteral("--kill"), QStringLiteral("all")});
    }
    p.reset();

    // an encrypted project's backup: encrypted files in an encrypted ZIP file; restored, it is encrypted
    {
        const Rich e = makeRich(W, QStringLiteral("Whole Encrypted"));
        encryptFolder(e.folder, recipients(), {}, &err);
        auto q = openOk(e.index, QStringLiteral("(an encrypted project)"));
        if (!q)
            return;
        check(!canChangeBackupEncryption(q.get(), &reason) && backupsEncrypted(q.get()),
              QStringLiteral("switch 1 is on and fixed for an encrypted project: %1").arg(reason));
        QString m;
        check(backup::backupProject(q.get(), bdir, 3, &m, &err) && m.endsWith(QLatin1String(".zip.gpg")), QStringLiteral("its backup is encrypted"), err);
        QString pf;
        const QString dest = backup::restore(m, W + QStringLiteral("/restored-enc"), &pf, &err);
        const QString rf = QFileInfo(pf).absolutePath();
        check(!dest.isEmpty() && settingsOf(rf).encryptedProject && plainFilesIn(rf).isEmpty(),
              QStringLiteral("restored, it is an encrypted project again (the new backup ID too: %1 files)").arg(filesIn(rf)),
              plainFilesIn(rf).join(QLatin1String(", ")));
        auto rq = Project::open(pf, &err);
        check(rq && rq->isEncrypted() && rq->docs()->text(e.t1)->toPlainText().contains(QLatin1String("Secretword1"))
                  && backup::projectId(rq.get()) != backup::projectId(q.get()),
              QStringLiteral("... it opens, with its own backup ID"), err);
    }

    // a project in Scrivener format: encrypted backups yes, encrypted project no
    {
        const Rich s = makeRich(W, QStringLiteral("Scriv Source"));
        auto src = openOk(s.index, QStringLiteral("(a project to save in Scrivener format)"));
        if (!src)
            return;
        auto wc = scrivener::convertToScrivener(src.get(), W + QStringLiteral("/Scriv Book.scriv"), &err);
        check(wc && !wc->scrivenerPath.isEmpty(), QStringLiteral("(a project in Scrivener format)"), err);
        if (!wc)
            return;
        check(!canChangeProjectEncryption(wc.get(), &reason) && reason.contains(QLatin1String("Scrivener")),
              QStringLiteral("switch 2 is greyed for a project in Scrivener format: %1").arg(reason));
        check(setBackupsEncrypted(wc.get(), true, recipients(), &err) && backupsEncrypted(wc.get()),
              QStringLiteral("switch 1 works for it"), err);
        QString m;
        check(backup::backupProject(wc.get(), bdir, 3, &m, &err) && m.endsWith(QLatin1String(".zip.gpg")) && !readAll(m).contains("Secretword"),
              QStringLiteral("its backup is <name>.zip.gpg"), err);
        QString pf;
        const QString dest = backup::restore(m, W + QStringLiteral("/restored-scriv"), &pf, &err);
        check(!dest.isEmpty() && pf.endsWith(QLatin1String(".scriv")), QStringLiteral("... and restores as a .scriv"), err);
    }
}

// ------------------------------------------------------------------------------------------------ the window

static void window(const QString &W)
{
    std::printf("== the window: the passphrase asked at the opening, Save As, turning the encryption on and off\n");
    QString err;
    const Rich r = makeRich(W, QStringLiteral("Window Book"));
    encryptFolder(r.folder, recipients(), {}, &err);
    KeyRing::device().lock(g_writer);
    KeyStore::system().remove(KeyStore::passphraseEntry(g_writer));
    MainWindow w;
    w.resize(1000, 700);
    w.show();
    QTest::qWait(150);
    int rounds = 0;
    std::function<void()> answer = [&] {
        auto *dlg = QApplication::activeModalWidget();
        if (!dlg || dlg->objectName() != QLatin1String("unlockDialog")) {
            if (++rounds < 30)
                QTimer::singleShot(100, answer);
            return;
        }
        auto *pass = dlg->findChild<QLineEdit *>(QStringLiteral("unlockPassphrase"));
        auto *wrong = dlg->findChild<QLabel *>(QStringLiteral("unlockWrong"));
        if (rounds < 100) { // first a wrong one
            rounds = 100;
            pass->setText(QStringLiteral("wrong words"));
            QTest::keyClick(pass, Qt::Key_Return);
            QTimer::singleShot(200, answer);
            return;
        }
        check(wrong && wrong->isVisible() && wrong->text() == errorText(Error::WrongPassphrase),
              QStringLiteral("a wrong passphrase: the dialog says so and stays"));
        dlg->grab().save(outDir + QStringLiteral("/v0990f-unlock-wrong.png"));
        pass->setText(QString::fromUtf8(kWriterPass));
        QTest::keyClick(pass, Qt::Key_Return);
    };
    QTimer::singleShot(200, answer);
    check(w.openProject(r.index) && w.project() && w.project()->isEncrypted(), QStringLiteral("the window opens the encrypted project after the passphrase"));
    QByteArray stored;
    check(KeyStore::system().read(KeyStore::passphraseEntry(g_writer), &stored) == KeyStore::Status::Ok, QStringLiteral("... and keeps the passphrase on this device"));
    QTest::qWait(200);
    w.grab().save(outDir + QStringLiteral("/v0990f-open.png"));
    // the window saves through the vault
    if (w.project()) {
        setText(w.project(), w.project()->item(r.t1), QStringLiteral("Typed in the window Secretword22"));
        check(w.project()->save(&err), QStringLiteral("(saved by the window)"), err);
        const QStringList left = plainFilesIn(r.folder);
        check(left.isEmpty(), QStringLiteral("the walk after the window's save and lock: no plain file"), left.join(QLatin1String(", ")));
    }
    // Save As in Scrivener format is refused for an encrypted project
    QTimer::singleShot(200, [] {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            check(box->objectName() == QLatin1String("encryptedNoScrivener"), QStringLiteral("Save a Copy as Scrivener Project says why not"));
            box->accept();
        } else {
            check(false, QStringLiteral("Save a Copy as Scrivener Project says why not"));
        }
    });
    if (QAction *a = w.findChild<QAction *>(QStringLiteral("saveCopyScrivener")))
        a->trigger();
    QTest::qWait(400);
    // turning the encryption off and on through the window's helper
    check(ui::changeProjectEncryption(&w, false, {}, &err) && w.project() && !w.project()->isEncrypted() && !settingsOf(r.folder).encryptedProject,
          QStringLiteral("turning the encryption off: closed, decrypted, opened again"), err);
    check(ui::changeProjectEncryption(&w, true, recipients(), &err) && w.project() && w.project()->isEncrypted() && plainFilesIn(r.folder).isEmpty(),
          QStringLiteral("turning it on: closed, encrypted, opened again"), err);
    check(w.project() && w.project()->docs()->text(r.t1)->toPlainText().contains(QLatin1String("Secretword22")), QStringLiteral("... with the latest text"));
    // File > Options > Encryption's switches (enc3's ProjectSwitches) act on this window's project
    {
        ProjectSwitches &ps = projectSwitches();
        check(ps.projectOpen && ps.projectOpen() && ps.projectEncrypted() && ps.backupsEncrypted() && !ps.isScrivenerProject()
                  && ps.projectFolders().contains(r.folder),
              QStringLiteral("the switches of File > Options > Encryption show the open project's state"));
        QString why;
        check(!ps.setBackupsEncrypted(false, &why) && !why.isEmpty(), QStringLiteral("switch 1 cannot be turned off for an encrypted project: %1").arg(why));
        check(ps.setProjectEncrypted(false, &why) && !ps.projectEncrypted(), QStringLiteral("switch 2 off: the switch shows the state asked for at once"), why);
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < 20000 && (!w.project() || w.project()->isEncrypted()))
            QTest::qWait(100);
        check(w.project() && !w.project()->isEncrypted() && !settingsOf(r.folder).encryptedProject,
              QStringLiteral("... and the project is decrypted once no dialog is open (the window's own keys: the roles)"));
        check(ps.setProjectEncrypted(true, &why), QStringLiteral("switch 2 on"), why);
        t.restart();
        while (t.elapsed() < 20000 && (!w.project() || !w.project()->isEncrypted()))
            QTest::qWait(100);
        check(w.project() && w.project()->isEncrypted() && settingsOf(r.folder).recipients == recipients() && plainFilesIn(r.folder).isEmpty(),
              QStringLiteral("... and the project is encrypted to this device's keys"));
    }
    // a restore of an encrypted backup asks for the passphrase when the key is locked
    QString made;
    backup::backupProject(w.project(), W + QStringLiteral("/wbackups"), 3, &made, &err);
    KeyRing::device().lock(g_writer);
    QTimer::singleShot(300, [] {
        auto *dlg = QApplication::activeModalWidget();
        check(dlg && dlg->objectName() == QLatin1String("unlockDialog"), QStringLiteral("Restore From Backup asks for the passphrase of an encrypted backup"));
        if (!dlg)
            return;
        dlg->grab().save(outDir + QStringLiteral("/v0990f-unlock-restore.png"));
        auto *pass = dlg->findChild<QLineEdit *>(QStringLiteral("unlockPassphrase"));
        pass->setText(QString::fromUtf8(kWriterPass));
        QTest::keyClick(pass, Qt::Key_Return);
    });
    qputenv("QB_NO_DIALOGS", "1");
    check(w.restoreBackupFile(made), QStringLiteral("... and restores it"), w.lastRestoreText());
    qunsetenv("QB_NO_DIALOGS");
    w.closeProject();
}

// --------------------------------------------------------------------------------------------------- speed

static void speed(const QString &W)
{
    std::printf("== speed: the autosave with and without encryption\n");
    QString err;
    auto words = [](int seed, int n) {
        static const char *w[] = {"tide", "harbor", "lantern", "keeper", "storm", "rope", "gull", "night", "salt", "stone"};
        QString s;
        for (int i = 0; i < n; ++i)
            s += QLatin1String(w[(seed * 7 + i * 3) % 10]) + QLatin1Char(' ');
        return s;
    };
    struct Figure {
        QString name;
        double plainOne = 0, sealedOne = 0, plainTen = 0, sealedTen = 0, plainOpen = 0, sealedOpen = 0;
        int docs = 0;
    };
    QList<Figure> figures;
    auto measure = [&](const QString &name, const QString &index) {
        Figure f;
        f.name = name;
        for (int round = 0; round < 2; ++round) {
            const bool sealed = round == 1;
            QElapsedTimer t;
            t.start();
            auto p = Project::open(index, &err);
            const double open = t.nsecsElapsed() / 1e6;
            if (!p) {
                check(false, QStringLiteral("(%1 opens)").arg(name), err);
                return;
            }
            QList<BinderItem *> texts;
            for (BinderItem *it : p->allItems())
                if (it->type == ItemType::Text)
                    texts << it;
            f.docs = int(texts.size());
            // one document changed (typing), then ten
            double one = 0, ten = 0;
            for (int rep = 0; rep < 5; ++rep) {
                setText(p.get(), texts.value(rep % texts.size()), words(rep, 300) + QStringLiteral("round %1 %2").arg(round).arg(rep));
                t.restart();
                p->autosave(&err);
                one += t.nsecsElapsed() / 1e6;
                for (int k = 0; k < 10; ++k)
                    setText(p.get(), texts.value((rep * 10 + k) % texts.size()), words(rep + k, 300) + QStringLiteral("round %1 %2").arg(round).arg(rep * 100 + k));
                t.restart();
                p->autosave(&err);
                ten += t.nsecsElapsed() / 1e6;
            }
            (sealed ? f.sealedOne : f.plainOne) = one / 5;
            (sealed ? f.sealedTen : f.plainTen) = ten / 5;
            (sealed ? f.sealedOpen : f.plainOpen) = open;
            p.reset();
            if (!sealed)
                encryptFolder(QFileInfo(index).absolutePath(), recipients(), {}, &err);
        }
        figures << f;
    };
    // the welcome project (the program's own small project)
    {
        QString e;
        const QJsonObject o = outline::read(QStringLiteral(":/welcome/outline.json"), &e);
        outline::Result res = outline::build(o, QStringLiteral(":/welcome"), W, QStringLiteral("Welcome Speed"));
        if (res.project) {
            const QString index = res.project->indexPath();
            res.project.reset();
            measure(QStringLiteral("the welcome project"), index);
        } else {
            info(QStringLiteral("(the welcome project could not be made: %1)").arg(res.errors.join(QLatin1String(", ")) + e));
        }
    }
    // a large project as the speed suite makes it (v0605: 30 chapters of 10 scenes and 700 notes, 500 words each)
    {
        auto p = Project::create(W, QStringLiteral("Large Speed"), &err);
        int seed = 1;
        for (int ch = 0; ch < 30; ++ch) {
            BinderItem *folder = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Chapter %1").arg(ch + 1));
            for (int s = 0; s < 10; ++s)
                setText(p.get(), p->model()->createItem(folder, -1, ItemType::Text, QStringLiteral("Scene %1.%2").arg(ch + 1).arg(s + 1)), words(seed++, 500));
        }
        for (int f = 0; f < 10; ++f) {
            BinderItem *folder = p->model()->createItem(p->research(), -1, ItemType::Folder, QStringLiteral("Notes %1").arg(f + 1));
            for (int d = 0; d < 70; ++d)
                setText(p.get(), p->model()->createItem(folder, -1, ItemType::Text, QStringLiteral("Note %1.%2").arg(f + 1).arg(d + 1)), words(seed++, 500));
        }
        p->docs()->recountAll();
        p->markIndexDirty();
        p->save(&err);
        const QString index = p->indexPath();
        p.reset();
        QElapsedTimer t;
        t.start();
        measure(QStringLiteral("a large project (1000 documents, 500,000 words)"), index);
        info(QStringLiteral("(the large project: measured in %1 s, its encryption included)").arg(t.elapsed() / 1000.0, 0, 'f', 1));
        QElapsedTimer c;
        c.start();
        decryptFolder(QFileInfo(index).absolutePath(), {}, &err);
        const double off = c.elapsed() / 1000.0;
        c.restart();
        encryptFolder(QFileInfo(index).absolutePath(), recipients(), {}, &err);
        info(QStringLiteral("turning the large project's encryption on: %1 s, off: %2 s (%3 files)")
                 .arg(c.elapsed() / 1000.0, 0, 'f', 1)
                 .arg(off, 0, 'f', 1)
                 .arg(filesIn(QFileInfo(index).absolutePath())));
    }
    for (const Figure &f : std::as_const(figures)) {
        info(QStringLiteral("%1 (%2 texts): autosave of 1 changed document %3 ms plain, %4 ms encrypted; of 10: %5 ms, %6 ms; "
                            "opening %7 ms, %8 ms")
                 .arg(f.name)
                 .arg(f.docs)
                 .arg(f.plainOne, 0, 'f', 1)
                 .arg(f.sealedOne, 0, 'f', 1)
                 .arg(f.plainTen, 0, 'f', 1)
                 .arg(f.sealedTen, 0, 'f', 1)
                 .arg(f.plainOpen, 0, 'f', 0)
                 .arg(f.sealedOpen, 0, 'f', 0));
        check(f.sealedOne > 0 && f.sealedOne < 2000, QStringLiteral("the encrypted autosave of one document stays under 2 s (%1: %2 ms)").arg(f.name).arg(f.sealedOne, 0, 'f', 1));
    }
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
    qputenv("QB_TEST_KEYSTORE", "memory");
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990fTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990fTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    if (!available()) {
        check(false, QStringLiteral("this build has the encryption (tools/get-crypto.sh before configure)"));
        std::printf("%d failure(s)\n", failures);
        return failures;
    }
    KeyRing &ring = KeyRing::device();
    g_writer = ring.createKey(QStringLiteral("Test Writer"), kWriterPass);
    g_recovery = ring.createKey(QStringLiteral("Test Recovery"), kRecoveryPass);
    g_recoverySecret = QString::fromLatin1(ring.exportSecretKey(g_recovery));
    ring.removeKey(g_recovery, true); // Quillbinder keeps only the public part of the recovery key
    ring.unlock(g_writer, kWriterPass);
    ring.save(KeyRing::deviceDir());
    keyfolder::setRoles(keyfolder::Roles{g_writer, g_recovery}); // which keys are this device's (enc3's File > Options > Encryption)
    check(!g_writer.isEmpty() && !g_recovery.isEmpty() && ring.hasSecret(g_writer) && !ring.hasSecret(g_recovery),
          QStringLiteral("(the writer's key with its secret part, the recovery key's public part only)"));
    QTemporaryDir work;
    if (qEnvironmentVariableIsSet("QB_MEASURE")) {
        speed(work.path());
    } else if (qEnvironmentVariableIsSet("QB_REVIEW_ONLY")) {
        reviewFindings(work.path());
    } else {
        vaultBasics(work.path());
        encryptedProject(work.path());
        cutOff(work.path());
        keysAndDamage(work.path());
        backups(work.path());
        window(work.path());
        reviewFindings(work.path());
        speed(work.path());
    }
    std::printf("%d failure(s)\n", failures);
    return failures;
}
