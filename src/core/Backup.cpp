#include "core/Backup.h"
#include "AppInfo.h"
#include "core/PathSafety.h"
#include "core/Paths.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/SaveBatch.h"
#include "core/Zip.h"
#include "core/crypto/ProjectCrypto.h"
#include "core/crypto/Vault.h"
#include "text/TextUtil.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QThread>

#include <algorithm>
#include <memory>
#include <atomic>
#include <cerrno>
#include <cstring>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QUuid>
#ifdef Q_OS_UNIX
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <io.h>
#endif

#include "core/TestSwitches.h"
#include <QSet>

namespace qb {
namespace backup {

namespace {
std::function<void(const QString &, qint64, qint64, bool *)> &progressHandler()
{
    static std::function<void(const QString &, qint64, qint64, bool *)> h;
    return h;
}

qint64 sizeOf(const QString &dir)
{
    qint64 n = 0;
    QDirIterator it(dir, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        n += it.fileInfo().size();
    }
    return n;
}

bool isIncomplete(const QString &file) { return QFileInfo(file).completeBaseName().contains(QLatin1String(" (incomplete")); }

// 0.81.0 (audit of 0.80.0, F-01): the age of a backup is the date and time in its name, "<title> bak yyyy-MM-dd
// HH-mm-ss", then the number of a second backup in that second ("... (2)"), then the file's time. Before, the whole
// name was compared, title first: after the title changed to one that sorts earlier ("Zebra" to "Apple"), every new
// backup counted as the oldest and the clean-up deleted it at once, while the backup reported success.
QString ageKey(const QFileInfo &fi)
{
    static const QRegularExpression stamp(QStringLiteral(" bak (\\d{4}-\\d{2}-\\d{2} \\d{2}-\\d{2}-\\d{2})(?: \\((\\d+)\\))?"));
    const QString name = fi.completeBaseName();
    QString date, number = QStringLiteral("1");
    QRegularExpressionMatchIterator it = stamp.globalMatch(name);
    while (it.hasNext()) { // the last one: a title may hold " bak <date>" itself
        const QRegularExpressionMatch m = it.next();
        date = m.captured(1);
        number = m.captured(2).isEmpty() ? QStringLiteral("1") : m.captured(2);
    }
    if (date.isEmpty())
        date = fi.lastModified().toString(QStringLiteral("yyyy-MM-dd HH-mm-ss"));
    return date + QLatin1Char(' ') + number.rightJustified(3, QLatin1Char('0')) + QLatin1Char(' ')
           + fi.lastModified().toString(QStringLiteral("yyyyMMddHHmmsszzz"));
}

// 0.99.0: the names of backups, plain and encrypted ("<name>.zip", "<name>.zip.gpg")
QStringList backupPatterns(const QString &pattern = QStringLiteral("* bak *"))
{
    return {pattern + QLatin1String(".zip"), pattern + QLatin1String(".zip.gpg")};
}

// ".zip.gpg" or ".zip"
QString backupExtension(const QString &fileName)
{
    return fileName.endsWith(QLatin1String(".zip.gpg"), Qt::CaseInsensitive) ? QStringLiteral(".zip.gpg") : QStringLiteral(".zip");
}

QStringList zipsIn(const QString &dir, const QStringList &patterns)
{
    QStringList out;
    QFileInfoList files = QDir(dir).entryInfoList(patterns, QDir::Files);
    // oldest first
    QHash<QString, QString> key;
    for (const QFileInfo &fi : std::as_const(files))
        key.insert(fi.absoluteFilePath(), ageKey(fi));
    std::sort(files.begin(), files.end(), [&key](const QFileInfo &a, const QFileInfo &b) {
        const QString &ka = key[a.absoluteFilePath()], &kb = key[b.absoluteFilePath()];
        return ka != kb ? ka < kb : a.fileName() < b.fileName();
    });
    for (const QFileInfo &fi : std::as_const(files))
        out << fi.absoluteFilePath();
    return out;
}

// ------------------------------------------------------------------ 0.90.3: names, the work folder, putting in place

// "<title> bak <date time>[ (n)][ (incomplete[ n])].zip": 1 = up to the time, 2 = n, 3 = the incomplete note, 4 = its n
const QRegularExpression &ownForm()
{
    static const QRegularExpression own(QStringLiteral(
        "^(.+ bak \\d{4}-\\d{2}-\\d{2} \\d{2}-\\d{2}-\\d{2})(?: \\((\\d+)\\))?( \\(incomplete(?: (\\d+))?\\))?\\.zip(?:\\.gpg)?$"));
    return own;
}

// The title in a backup's name of the program's own form ("<title> bak <date time>..."); "" for any other name.
QString ownTitle(const QString &fileName)
{
    const QRegularExpressionMatch m = ownForm().match(fileName);
    if (!m.hasMatch())
        return QString();
    const QString upToTime = m.captured(1); // "<title> bak yyyy-MM-dd HH-mm-ss"
    return upToTime.left(upToTime.size() - 24);
}

// 0.91.0: the backups in dir that this program made of the project with this title (its safe file name form), oldest
// first: the only files the clean-up counts and may delete.
QStringList ownBackupsOf(const QString &dir, const QString &title)
{
    QStringList out;
    const QStringList all = zipsIn(dir, backupPatterns());
    for (const QString &f : all) {
        const QString name = QFileInfo(f).fileName();
        if (isOwnBackupName(name) && ownTitle(name) == title)
            out << f;
    }
    return out;
}

// The backup's name with another number of its second ("... (3).zip"; the note "(incomplete)" stays behind it).
QString numberedName(const QString &fileName, int n)
{
    const QRegularExpressionMatch m = ownForm().match(fileName);
    const QString ext = backupExtension(fileName);
    if (!m.hasMatch())
        return fileName.left(fileName.size() - (fileName.endsWith(ext, Qt::CaseInsensitive) ? ext.size() : 0))
               + QStringLiteral(" (%1)").arg(n) + ext;
    return m.captured(1) + QStringLiteral(" (%1)").arg(n) + m.captured(3) + ext;
}

QByteArray sha1Of(const QString &file, qint64 *size)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    QCryptographicHash h(QCryptographicHash::Sha1);
    if (!h.addData(&f) || f.error() != QFileDevice::NoError)
        return {};
    if (size)
        *size = f.size();
    return h.result();
}

bool syncFile(QFile &f)
{
#ifdef Q_OS_UNIX
    return ::fsync(f.handle()) == 0 || errno == EINVAL || errno == ENOTSUP || errno == EOPNOTSUPP || errno == ENOSYS;
#elif defined(Q_OS_WIN)
    return ::_commit(f.handle()) == 0;
#else
    Q_UNUSED(f);
    return true;
#endif
}

enum class Put { Done, Failed, Cancelled };

// Copies a finished backup straight to its final name (never over a file that is there: NewOnly), synced, then read
// again and compared with what was sent. No temporary name in the backup folder: a program that watches the folder
// sees one file that is written once and closed. A copy that fails, differs or is cancelled is removed again.
Put copyInto(const QString &staged, const QString &target, QString *why, const std::atomic<bool> *cancel)
{
    QFile in(staged);
    if (!in.open(QIODevice::ReadOnly)) {
        *why = in.errorString();
        return Put::Failed;
    }
    QFile out(target);
    if (!out.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
        *why = out.errorString();
        return Put::Failed;
    }
    QCryptographicHash sent(QCryptographicHash::Sha1);
    qint64 copied = 0;
    bool ok = true, stopped = false;
    for (;;) {
        if (cancel && cancel->load()) {
            stopped = true;
            break;
        }
        const QByteArray piece = in.read(1 << 20);
        if (piece.isEmpty())
            break;
        if (out.write(piece) != piece.size()) {
            ok = false;
            *why = out.errorString();
            break;
        }
        sent.addData(piece);
        copied += piece.size();
    }
    if (!stopped && ok) {
        if (in.error() != QFileDevice::NoError) {
            ok = false;
            *why = in.errorString();
        } else if (copied != in.size()) {
            ok = false;
            *why = QObject::tr("the copy differs from the backup");
        } else if (!out.flush()) {
            ok = false;
            *why = out.errorString();
        } else if (!syncFile(out)) {
            ok = false;
            *why = QString::fromLocal8Bit(std::strerror(errno));
        }
    }
    out.close();
    if (!stopped && ok && out.error() != QFileDevice::NoError) {
        ok = false;
        *why = out.errorString();
    }
    if (!stopped && ok) {
        qint64 size = -1;
        if (sha1Of(target, &size) != sent.result() || size != copied) {
            ok = false;
            *why = QObject::tr("the copy differs from the backup");
        }
    }
    if (stopped || !ok) {
        QFile::remove(target);
        return stopped ? Put::Cancelled : Put::Failed;
    }
    return Put::Done;
}

// Why one step under the final name: docs/design-decisions.md, entry 4.
// Puts a finished backup of the work folder into the backup folder, in one step: renamed there (one drive), else
// copied straight to the final name. *finalPath: where it is now. The waiting file is gone only when the backup is
// in the folder, whole.
Put putInPlace(const QString &staged, const QString &dir, QString *finalPath, QString *why, const std::atomic<bool> *cancel)
{
    if (!QDir().mkpath(dir)) {
        *why = QObject::tr("the folder cannot be created");
        return Put::Failed;
    }
    const QString ownName = QFileInfo(staged).fileName();
    QString target = QDir(dir).filePath(ownName);
    if (QFileInfo::exists(target)) {
        // a file of this name came into the folder after the name was chosen
        qint64 a = -1, b = -2;
        const QByteArray ours = sha1Of(staged, &a);
        if (!ours.isEmpty() && ours == sha1Of(target, &b) && a == b) {
            // this very backup: it was put in place before, and the program ended before the waiting file was removed
            QFile::remove(staged);
            *finalPath = target;
            return Put::Done;
        }
        // (0.99.0: an encrypted backup is judged by its packets: no key is needed for it)
        if (target.endsWith(QLatin1String(".gpg")) ? !crypto::isWholeMessage(target) : ZipWriter::verify(target) < 0) {
            // not a whole ZIP file: what an earlier copy of this backup left when the program ended in the middle
            if (!QFile::remove(target)) {
                *why = QObject::tr("a file of this name is in the way and cannot be removed");
                return Put::Failed;
            }
        } else {
            // another whole backup of this name stays; ours gets the next free number of its second
            for (int n = 2; QFileInfo::exists(target) && n < 100; ++n)
                target = QDir(dir).filePath(numberedName(ownName, n));
            if (QFileInfo::exists(target)) {
                *why = QObject::tr("a file of this name is in the way and cannot be removed");
                return Put::Failed;
            }
        }
    }
    // (test builds: QB_TEST_BACKUP_COPY takes the way of two drives on one)
    if (QB_TEST_ENV_SET("QB_TEST_BACKUP_COPY") || !savebatch::replace(staged, target)) {
        const Put r = copyInto(staged, target, why, cancel);
        if (r != Put::Done)
            return r;
        QFile::remove(staged);
    }
    // 0.70.0 (audit A F-11): the new backup's name reaches the disk before older backups are deleted
    savebatch::syncDir(dir);
    *finalPath = target;
    return Put::Done;
}

// The work folder says where its backups belong (for the writer who finds one waiting there).
void noteTarget(const QString &work, const QString &dir)
{
    const QString note = work + QLatin1String("/target.txt");
    const QByteArray text = QDir::toNativeSeparators(QFileInfo(dir).absoluteFilePath()).toUtf8() + '\n';
    QFile f(note);
    if (f.open(QIODevice::ReadOnly) && f.readAll() == text)
        return;
    f.close();
    safesave::writeFile(note, text);
}

// What a backup left that was cut off while it was packed (a crash, the power): removed once it is an hour old (a
// younger one may be the backup of another window into the same folder, still running).
void sweepWork(const QString &work)
{
    const QDateTime limit = QDateTime::currentDateTime().addSecs(-3600);
    const QFileInfoList parts = QDir(work).entryInfoList({QStringLiteral("*.part")}, QDir::Files);
    for (const QFileInfo &fi : parts)
        if (fi.lastModified() < limit)
            QFile::remove(fi.absoluteFilePath());
}
} // namespace

void setProgressHandler(std::function<void(const QString &, qint64, qint64, bool *)> handler)
{
    progressHandler() = std::move(handler);
}

QString defaultDirectory()
{
    return dataDir() + QLatin1String("/Backups");
}

QString workDirectory()
{
    return dataDir() + QLatin1String("/backup-work");
}

QString workFolderFor(const QString &backupFolder)
{
    const QString place = QDir::cleanPath(QFileInfo(backupFolder).absoluteFilePath());
    return workDirectory() + QLatin1Char('/')
           + QString::fromLatin1(QCryptographicHash::hash(place.toUtf8(), QCryptographicHash::Sha1).toHex().left(12));
}

QString idFromSeed(const QString &seed)
{
    static const char alphabet[] = "23456789ABCDEFGHJKMNPQRSTUVWXYZ"; // 31: no 0, O, 1, I, L
    const QByteArray h = QCryptographicHash::hash(seed.toUtf8(), QCryptographicHash::Sha256);
    quint64 n = 0;
    for (int i = 0; i < 8; ++i)
        n = (n << 8) | quint8(h.at(i));
    QString id;
    for (int i = 0; i < 5; ++i) {
        id += QLatin1Char(alphabet[n % 31]);
        n /= 31;
    }
    return id;
}

QString projectId(const Project *p)
{
    // a project in Scrivener format: the Identifier of its .scrivx, as the working copy's state keeps it
    QString seed;
    QByteArray bytes; // (0.99.0: these files are read through the vault: decrypted in an encrypted project)
    if (!p->scrivenerPath.isEmpty() && vault::readFile(p->settingsDir() + QLatin1String("/scrivener-sync.json"), &bytes))
        seed = QJsonDocument::fromJson(bytes).object().value(QStringLiteral("identifier")).toString().toUpper();
    // Quillbinder's own format: the seed of a copy the program made (giveNewId), else the id of the Draft folder
    // (made once with the project, and it cannot be deleted)
    if (seed.isEmpty() && vault::readFile(p->settingsDir() + QLatin1String("/backup-id.txt"), &bytes))
        seed = QString::fromUtf8(bytes.left(200)).trimmed();
    if (seed.isEmpty() && p->draft())
        seed = p->draft()->id;
    if (seed.isEmpty())
        seed = p->title();
    return idFromSeed(seed);
}

bool giveNewId(const QString &projectFolder)
{
    const QString dir = projectFolder + QLatin1String("/settings");
    if (!QDir().mkpath(dir))
        return false;
    // 0.99.0: the copy of an encrypted project keeps it encrypted (its marker came with it); written checked
    vault::Scope encrypted(projectFolder);
    const QByteArray seed = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8() + '\n';
    return safesave::writeFile(dir + QLatin1String("/backup-id.txt"), seed);
}

QString nameStart(const Project *p)
{
    return text::safeFileName(p->title()) + QLatin1String(" #") + projectId(p);
}

namespace {
// "<title> #<ID>" without its ID; the title itself when it carries none
QString withoutId(const QString &title)
{
    static const QRegularExpression id(QStringLiteral(" #[2-9A-HJKMNP-Z]{5}$"));
    const QRegularExpressionMatch m = id.match(title);
    return m.hasMatch() ? title.left(m.capturedStart()) : title;
}
} // namespace

bool isOwnBackupName(const QString &fileName)
{
    const QRegularExpressionMatch m = ownForm().match(fileName);
    if (!m.hasMatch())
        return false;
    // the second backup of a second is "(2)" and the second incomplete one "(incomplete 2)": a "(1)" was made elsewhere
    for (int group : {2, 4})
        if (!m.captured(group).isEmpty() && m.captured(group).toInt() < 2)
            return false;
    return true;
}

QString projectKey(const Project *p)
{
    QString place = QFileInfo(p->displayFolder()).absoluteFilePath();
    const QString canonical = QFileInfo(place).canonicalFilePath();
    if (!canonical.isEmpty())
        place = canonical;
    return QString::fromLatin1(QCryptographicHash::hash(QDir::cleanPath(place).toUtf8(), QCryptographicHash::Sha1).toHex().left(8));
}

QString formerProjectFolder(const Project *p, const QString &dirIn)
{
    const QString dir = dirIn.isEmpty() ? defaultDirectory() : dirIn;
    // the folder kept its name when the project's title changed: it is found by its key
    const QStringList found = QDir(dir).entryList({QStringLiteral("* ") + projectKey(p)}, QDir::Dirs | QDir::NoDotAndDotDot);
    return found.isEmpty() ? QString() : QDir(dir).filePath(found.first());
}

namespace {
// 0.93.0: the files of `all` (backups of any project) that are this project's, in the way Restore From Backup offers them
QStringList mineOf(const Project *p, const QStringList &all)
{
    const QString mine = text::safeFileName(p->title());
    const QString begins = mine + QLatin1String(" bak "), beginsWithId = mine + QLatin1String(" #");
    QStringList out;
    for (const QString &f : all) {
        const QString name = QFileInfo(f).fileName();
        // a name in the program's own form says exactly which title it is of; any other one (a sync program's copy)
        // is taken by how it begins. 0.92.0: with this project's ID, with another ID (a project of the same title)
        // or without one (a backup of a version before): all are offered, so that nothing gets out of reach
        const QString title = ownTitle(name);
        if (title.isEmpty() ? (name.startsWith(begins) || (name.startsWith(beginsWithId) && withoutId(name.left(name.indexOf(QLatin1String(" bak ")))) == mine))
                            : withoutId(title) == mine)
            out << f;
    }
    return out;
}
} // namespace

QStringList backupsFor(const Project *p, const QString &dirIn)
{
    const QString dir = dirIn.isEmpty() ? defaultDirectory() : dirIn;
    return mineOf(p, zipsIn(dir, backupPatterns()));
}

QStringList waitingBackupsFor(const Project *p, const QString &dirIn)
{
    const QString dir = dirIn.isEmpty() ? defaultDirectory() : dirIn;
    // the work folders a backup of this project can wait in: that of the backup folder, and (0.70.0 to 0.90.4) those of
    // the project's own folder in it
    QStringList works{workFolderFor(dir)};
    const QString plainTitle = text::safeFileName(p->title());
    works << workFolderFor(QDir(dir).filePath(plainTitle + QLatin1Char(' ') + projectKey(p)));
    const QString former = formerProjectFolder(p, dir);
    if (!former.isEmpty())
        works << workFolderFor(former);
    works.removeDuplicates();
    QStringList out;
    for (const QString &w : std::as_const(works))
        out << mineOf(p, zipsIn(w, backupPatterns()));
    return out;
}

QStringList formerBackupsFor(const Project *p, const QString &dir)
{
    const QString former = formerProjectFolder(p, dir);
    return former.isEmpty() ? QStringList() : zipsIn(former, backupPatterns());
}

// 0.93.0: one backup of the project's own folder of 0.70.0 to 0.90.4 into the backup folder, under its name, never
// over a file that is there: a hard link where the drive allows (one step, no copy; fails when the name exists), else a
// copy straight to the final name that is read again and compared (copyInto, NewOnly). The original goes after that.
namespace {
bool moveOne(const QString &from, const QString &to, QString *why)
{
#ifdef Q_OS_UNIX
    if (::link(QFile::encodeName(from).constData(), QFile::encodeName(to).constData()) == 0) {
        if (!QFile::remove(from)) {
            *why = QObject::tr("it is in the backup folder now, but the original could not be removed");
            return false;
        }
        return true;
    }
    if (errno == EEXIST) {
        *why = QObject::tr("a backup of this name is already in the backup folder");
        return false;
    }
#endif
    if (QFileInfo::exists(to)) {
        *why = QObject::tr("a backup of this name is already in the backup folder");
        return false;
    }
    if (copyInto(from, to, why, nullptr) != Put::Done)
        return false;
    if (!QFile::remove(from)) {
        *why = QObject::tr("it is in the backup folder now, but the original could not be removed");
        return false;
    }
    return true;
}
} // namespace

MoveReport moveFormerBackups(const Project *p, const QString &dirIn)
{
    MoveReport r;
    const QString dir = dirIn.isEmpty() ? defaultDirectory() : dirIn;
    r.from = formerProjectFolder(p, dir);
    if (r.from.isEmpty())
        return r;
    QStringList all = zipsIn(r.from, backupPatterns());
    if (!all.isEmpty() && !QDir().mkpath(dir)) {
        r.left << QObject::tr("the backup folder cannot be created");
        return r;
    }
    for (const QString &f : std::as_const(all)) {
        const QString name = QFileInfo(f).fileName();
        QString why;
        if (moveOne(f, QDir(dir).filePath(name), &why))
            r.moved << name;
        else
            r.left << name + QLatin1String(": ") + why;
    }
    if (!r.moved.isEmpty())
        savebatch::syncDir(dir);
    // the folder goes when nothing is left in it (a rmdir: it fails by itself when something is there)
    r.folderRemoved = QDir().rmdir(r.from);
    return r;
}

namespace {
// 0.93.0: a backup of the clean-up that cannot be deleted is told, once per file and session, with the reason
// (tests: QB_TEST_FAIL_REMOVE holds a part of the file's name)
bool removeBackupFile(const QString &path, QStringList *notes)
{
    static QSet<QString> told;
    QString why;
    bool ok;
    const QString fake = QB_TEST_ENV("QB_TEST_FAIL_REMOVE");
    if (!fake.isEmpty() && path.contains(fake)) {
        ok = false;
        why = QObject::tr("Permission denied");
    } else {
        QFile f(path);
        ok = f.remove() || !QFile::exists(path);
        why = f.errorString();
    }
    if (!ok && notes && !told.contains(path)) {
        told.insert(path);
        *notes << QObject::tr("%1: %2").arg(QDir::toNativeSeparators(path), why);
    }
    return ok;
}
} // namespace

bool backupProject(Project *p, const QString &dirIn, int keep, QString *outPath, QString *error, Where where, bool *waiting,
                   QStringList *cleanupNotes)
{
    if (cleanupNotes)
        cleanupNotes->clear();
    if (waiting)
        *waiting = false;
    // (0.90.3: an empty *error after a backup means "complete": a text left in it from an earlier step is not kept)
    if (error)
        error->clear();
    // 0.91.0: every backup goes straight into the folder given (see Backup.h); the backups of this project are those
    // that carry its title, and only they are counted and cleaned up
    const QString dir = dirIn.isEmpty() ? defaultDirectory() : dirIn;
    // 0.92.0: "<title> #<ID>": the backups of this project, and no other project's of the same title (Backup.h)
    const QString plainTitle = text::safeFileName(p->title());
    const QString mine = nameStart(p);
    // Backups that waited for the project's own folder of 0.70.0 to 0.90.4 (it could not be reached then) wait in
    // that folder's work folder; they are put into the backup folder with this backup.
    QStringList formerWork;
    if (where == Where::BackupFolder) {
        const QString wouldBe = QDir(dir).filePath(plainTitle + QLatin1Char(' ') + projectKey(p));
        formerWork << workFolderFor(wouldBe);
        const QString former = formerProjectFolder(p, dir);
        if (!former.isEmpty() && QDir::cleanPath(former) != QDir::cleanPath(wouldBe))
            formerWork << workFolderFor(former);
    }
    // 0.64.1: a backup is made only of a project whose latest changes were saved (before, a failed save
    // was passed over and the backup reported success without them)
    {
        QString why;
        if (!p->save(&why)) {
            if (error)
                *error = QObject::tr("The project could not be saved first, so no backup was made: %1").arg(why);
            return false;
        }
    }
    // 0.90.3: packed and checked in the program's own work folder for this backup folder; the backup folder gets
    // only the finished file (see Backup.h). Before, the backup was written as "<name>.zip.part" inside the backup
    // folder and renamed there, in view of every program that watches the folder.
    const QString work = workFolderFor(dir);
    if (!QDir().mkpath(work)) {
        if (error)
            *error = QObject::tr("Cannot create the work folder for backups %1").arg(QDir::toNativeSeparators(work));
        return false;
    }
    noteTarget(work, dir);
    sweepWork(work);
    // 0.99.0 (item 113): encrypted backups ("<name>.zip.gpg", to the project's two keys). Only the public keys are needed:
    // the packing thread gets a key ring of its own with them (KeyRing::device() stays the window thread's). Never
    // written plain instead: a backup that cannot be encrypted is not made.
    const crypto::ProjectSettings cs = crypto::settingsOf(p->folderPath());
    const bool sealBackup = cs.encryptedBackups;
    const QStringList recipients = cs.recipients.list();
    std::unique_ptr<crypto::KeyRing> sealRing;
    bool canCheck = false; // a recipient's secret key is unlocked here: the check decrypts what was written
    if (sealBackup) {
        QString why;
        if (!cs.recipients.isComplete()) {
            why = QObject::tr("the project's keys are not named in %1").arg(crypto::markerName());
        } else {
            sealRing = std::make_unique<crypto::KeyRing>();
            std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
            crypto::KeyRing &device = crypto::KeyRing::device();
            for (const QString &f : recipients) {
                const QByteArray pub = device.exportPublicKey(f);
                if (pub.isEmpty() || !sealRing->importKeys(pub).imported.contains(f)) {
                    why = QObject::tr("the key %1 is not in Quillbinder's key ring").arg(crypto::groupedFingerprint(f));
                    break;
                }
                canCheck = canCheck || (device.hasSecret(f) && device.isUnlocked(f));
            }
        }
        if (!why.isEmpty()) {
            if (error)
                *error = QObject::tr("The backups of this project are encrypted, but no encrypted backup can be made: %1. No "
                                     "backup was made.")
                             .arg(why);
            return false;
        }
    }
    const QString ext = sealBackup ? QStringLiteral(".zip.gpg") : QStringLiteral(".zip");
    const QString stem = mine + QLatin1String(" bak ")
                         + QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH-mm-ss"));
    // two backups in the same second (File > Save twice with "back up with each manual save",
    // 0.21): the second gets a number instead of failing. The name must be free in the backup folder and among
    // the backups that wait in the work folder.
    auto taken = [&dir, &work](const QString &name) {
        return QFile::exists(QDir(dir).filePath(name)) || QFile::exists(QDir(work).filePath(name));
    };
    QString name = stem + ext;
    QString numbered = stem; // the name without its extension
    for (int n = 2; taken(name) && n < 100; ++n) {
        numbered = stem + QStringLiteral(" (%1)").arg(n);
        name = numbered + ext;
    }
    // written under a temporary name, checked, then renamed: a backup that was cut off
    // (a crash, a full disk) never looks like a finished one (0.20)
    // (0.99.0: an encrypted backup is packed as "<name>.zip.part" too, then encrypted beside it; the plain one goes)
    const QString part = QDir(work).filePath(numbered + QLatin1String(".zip")) + QLatin1String(".part");
    // what the work needs from the project, taken here in the window's thread
    const QString display = p->displayFolder();
    const QString folderName = QFileInfo(display).fileName();
    const QString working = p->scrivenerPath.isEmpty() ? QString() : p->folderPath();
    const QString workingName = working.isEmpty() ? QString()
                                                  : QObject::tr("%1 - working copy of %2").arg(QFileInfo(display).completeBaseName(),
                                                                                              QLatin1String(QB_APP_NAME))
                                                        + QLatin1Char('/') + QFileInfo(working).fileName();
    struct Result {
        bool packed = false; // finished and checked in the work folder
        bool cancelled = false;
        QStringList skipped;
        QString why;            // why it could not be packed
        QString staged;         // the finished backup in the work folder
        Put put = Put::Failed;  // whether it reached the backup folder
        QString putWhy;
        QString path;           // where it is in the backup folder
    } res;
    std::atomic<qint64> done{0};
    std::atomic<bool> cancel{false};
    // 0.70.0 (audit A U-06): packing and checking run in a thread of their own; the window's thread waits with
    // the progress window of the handler (drawn, its Cancel button working, the freeze watchdog fed)
    auto work_ = [&] {
        {
            ZipWriter zw(part);
            if (!zw.isOpen()) {
                res.why = zw.errorString();
                return;
            }
            zw.setProgress(&done, &cancel);
            // a project in Scrivener format: the .scriv is the project; the working copy holds
            // what only this program knows (keywords, collections, styles, objects ...)
            bool ok = zw.addDirectory(display, folderName);
            if (!working.isEmpty() && !zw.cancelled())
                ok = zw.addDirectory(working, workingName) && ok;
            res.skipped = zw.skipped();
            res.cancelled = zw.cancelled();
            if (!zw.close() || !ok) {
                res.why = zw.errorString();
                QFile::remove(part);
                return;
            }
        }
        QString why;
        if (ZipWriter::verify(part, &why) < 0) {
            QFile::remove(part);
            res.why = why;
            return;
        }
        // 0.65.0 (audit F03): a backup with files missing says so in its name, and never counts as one of the backups
        // to keep: older complete backups are deleted only after a complete one
        QString finished = name;
        if (!res.skipped.isEmpty()) {
            const QString base = numbered;
            finished = base + QLatin1String(" (incomplete)") + ext;
            for (int n = 2; taken(finished) && n < 100; ++n)
                finished = base + QStringLiteral(" (incomplete %1)").arg(n) + ext;
        }
        res.staged = QDir(work).filePath(finished);
        if (sealBackup) {
            // 0.99.0: encrypted from the checked ZIP file; then the backup's own check of what was written: decrypted again
            // and compared with the ZIP file where a recipient's key is unlocked here, else its packets read (whole, not cut
            // off: without the secret key nothing more can be told)
            const QString sealedPart = res.staged + QLatin1String(".part");
            QByteArray plainSha;
            {
                QFile z(part);
                QCryptographicHash h(QCryptographicHash::Sha256);
                if (z.open(QIODevice::ReadOnly) && h.addData(&z))
                    plainSha = h.result();
            }
            const crypto::Error e = plainSha.isEmpty() ? crypto::Error::Io : sealRing->encryptFile(part, sealedPart, recipients);
            QFile::remove(part);
            QString checkWhy;
            if (e != crypto::Error::None) {
                checkWhy = QObject::tr("it could not be encrypted: %1").arg(crypto::errorText(e));
            } else if (canCheck) {
                crypto::Error d = crypto::Error::None;
                QByteArray back;
                {
                    std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
                    back = crypto::KeyRing::device().decryptFileSha256(sealedPart, &d);
                }
                if (d != crypto::Error::None)
                    checkWhy = QObject::tr("the encrypted backup could not be decrypted again: %1").arg(crypto::errorText(d));
                else if (back != plainSha)
                    checkWhy = QObject::tr("the encrypted backup, decrypted again, differs from the backup");
            } else if (!crypto::isWholeMessage(sealedPart)) {
                checkWhy = QObject::tr("the encrypted backup is not whole");
            }
            if (!checkWhy.isEmpty()) {
                QFile::remove(sealedPart);
                res.why = checkWhy;
                res.staged.clear();
                return;
            }
            if (!savebatch::replace(sealedPart, res.staged)) {
                QFile::remove(sealedPart);
                res.staged.clear();
                return;
            }
        } else if (!savebatch::replace(part, res.staged)) {
            QFile::remove(part);
            res.staged.clear();
            return;
        }
        res.packed = true;
        // backups that could not be put into the backup folder before go first, oldest first (those of every
        // project: they all belong into this folder)
        bool takes = true;
        const QStringList waitingOnes = zipsIn(work, backupPatterns(QStringLiteral("*")));
        for (const QString &w : waitingOnes) {
            if (QFileInfo(w).fileName() == finished)
                continue;
            QString at, whyNot;
            if (putInPlace(w, dir, &at, &whyNot, &cancel) != Put::Done) {
                takes = false;
                break; // the folder does not take them now; the new backup says why
            }
        }
        // 0.91.0: and those that waited for this project's own folder of 0.70.0 to 0.90.4
        for (const QString &fw : std::as_const(formerWork)) {
            if (!takes)
                break;
            const QStringList formerOnes = zipsIn(fw, backupPatterns(QStringLiteral("*")));
            for (const QString &w : formerOnes) {
                QString at, whyNot;
                if (putInPlace(w, dir, &at, &whyNot, &cancel) != Put::Done) {
                    takes = false;
                    break;
                }
            }
            if (takes && QDir(fw).exists()) { // nothing waits there any more: the folder goes with its note
                QFile::remove(fw + QLatin1String("/target.txt"));
                QDir().rmdir(fw);
            }
        }
        if (cancel.load()) {
            res.cancelled = true;
            return;
        }
        res.put = putInPlace(res.staged, dir, &res.path, &res.putWhy, &cancel);
        if (res.put == Put::Cancelled)
            res.cancelled = true;
    };
    const qint64 total = sizeOf(display) + (working.isEmpty() ? 0 : sizeOf(working));
    if (progressHandler()) {
        QThread *t = QThread::create(work_);
        t->start();
        QElapsedTimer clock;
        clock.start();
        while (!t->wait(50)) {
            bool stop = false;
            progressHandler()(QObject::tr("Backing up \"%1\"...").arg(p->title()), done.load(), total, &stop);
            if (stop)
                cancel.store(true);
        }
        delete t;
        progressHandler()(QString(), total, total, nullptr); // done: the progress window goes
    } else {
        work_();
    }
    if (res.cancelled) {
        QFile::remove(part);
        if (!res.staged.isEmpty())
            QFile::remove(res.staged);
        if (error)
            *error = QObject::tr("The backup was cancelled.");
        return false;
    }
    if (!res.packed) {
        QFile::remove(part); // (0.99.0: never a plain ZIP file left of an encrypted backup)
        const QString shown = QDir(work).filePath(name);
        if (error)
            *error = res.why.isEmpty() ? QObject::tr("Could not write the backup %1").arg(shown)
                                       : QObject::tr("Could not write or check the backup %1: %2").arg(shown, res.why);
        return false;
    }
    if (res.put != Put::Done) {
        const QString shownDir = QDir::toNativeSeparators(dir);
        if (where == Where::AsChosen) {
            // the writer chose this folder a moment ago and can choose another: nothing is kept for it
            QFile::remove(res.staged);
            if (error)
                *error = QObject::tr("The backup could not be put into %1: %2").arg(shownDir, res.putWhy);
            return false;
        }
        // The backup is whole and stays in the work folder; the next backup into this folder puts it in place. Of
        // those of this project that wait, the newest `keep` stay (as in the backup folder).
        if (keep > 0) {
            QStringList waitingOnes = ownBackupsOf(work, mine); // oldest first
            while (waitingOnes.size() > keep) {
                const QString oldest = waitingOnes.takeFirst();
                if (QFileInfo(oldest).fileName() != QFileInfo(res.staged).fileName())
                    QFile::remove(oldest);
            }
        }
        if (waiting)
            *waiting = true;
        if (outPath)
            *outPath = res.staged;
        if (error)
            *error = QObject::tr("The backup was made, but it could not be put into the backup folder %1: %2\n\nIt is kept "
                                 "in %3, and the next backup will put it in place.")
                         .arg(shownDir, res.putWhy, QDir::toNativeSeparators(work));
        return false;
    }
    const QString path = res.path;
    if (outPath)
        *outPath = path;
    if (error && !res.skipped.isEmpty())
        *error = QObject::tr("The backup is incomplete: %n file(s) could not be read and are missing from it: %1. Older "
                             "backups were kept.", nullptr, int(res.skipped.size()))
                     .arg(res.skipped.join(QLatin1String(", ")));
    if (keep > 0 && where == Where::BackupFolder) {
        QStringList complete, incomplete; // oldest first
        // 0.90.3: only files named as this program names its backups; a copy a sync program made of a backup
        // ("... bak <date>-<computer>-safeBackup-0001.zip") counted as a backup of that moment, and six of them pushed
        // every real older backup out of a folder that keeps five
        // 0.91.0: and of those only the ones with this project's title (the folder holds every project's backups)
        const QStringList found = ownBackupsOf(dir, mine);
        for (const QString &f : found)
            (isIncomplete(f) ? incomplete : complete) << f;
        const QString made = QFileInfo(path).absoluteFilePath();
        // the backup just made is never one of those deleted, whatever its name says (0.81.0)
        auto trim = [&made, cleanupNotes](QStringList &list, int keepCount) {
            for (int i = 0; list.size() > keepCount && i < list.size();) {
                if (list.at(i) == made) {
                    ++i;
                    continue;
                }
                removeBackupFile(list.takeAt(i), cleanupNotes);
            }
        };
        if (res.skipped.isEmpty()) {
            trim(complete, keep);
            // incomplete ones older than the oldest complete backup that stays are of no more use
            if (!complete.isEmpty()) {
                const QString oldest = ageKey(QFileInfo(complete.first()));
                for (const QString &f : std::as_const(incomplete))
                    if (f != made && ageKey(QFileInfo(f)) < oldest)
                        removeBackupFile(f, cleanupNotes);
            }
        } else {
            // 0.81.0: older complete backups stay until a complete one is made; of the incomplete ones, the newest
            // `keep` stay (before, they were never deleted, so a project with a file that cannot be read collected
            // incomplete backups without end)
            trim(incomplete, keep);
        }
        // 0.81.0 (audit of 0.80.0, F-01): the clean-up must have left the new backup in place
        if (!QFileInfo::exists(made)) {
            if (outPath)
                outPath->clear();
            if (error)
                *error = QObject::tr("The new backup %1 is not there after older backups were cleaned up. No backup was made; "
                                     "please make one with File > Back Up > Back Up To...")
                             .arg(QDir::toNativeSeparators(made));
            return false;
        }
    }
    return true;
}

namespace {
QString dueFile() { return dataDir() + QLatin1String("/backups-due.txt"); }
QStringList dueList()
{
    QFile f(dueFile());
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
}
QString placeOf(const QString &projectFile)
{
    const QString canonical = QFileInfo(projectFile).canonicalFilePath();
    return canonical.isEmpty() ? QFileInfo(projectFile).absoluteFilePath() : canonical;
}
} // namespace

bool markDue(const QString &projectFile)
{
    QStringList list = dueList();
    const QString place = placeOf(projectFile);
    if (list.contains(place))
        return true;
    list << place;
    QDir().mkpath(QFileInfo(dueFile()).absolutePath());
    return safesave::writeFile(dueFile(), list.join(QLatin1Char('\n')).toUtf8() + '\n'); // 0.93.0: the result is told
}

bool isDue(const QString &projectFile) { return dueList().contains(placeOf(projectFile)); }

bool clearDue(const QString &projectFile)
{
    QStringList list = dueList();
    if (!list.removeAll(placeOf(projectFile)))
        return false;
    if (list.isEmpty())
        QFile::remove(dueFile());
    else
        safesave::writeFile(dueFile(), list.join(QLatin1Char('\n')).toUtf8() + '\n');
    return true;
}

bool takeDue(const QString &projectFile)
{
    if (!isDue(projectFile))
        return false;
    clearDue(projectFile);
    return true;
}

QString restore(const QString &zipFile, const QString &intoParent, QString *projectFile, QString *error, crypto::Error *kind)
{
    if (kind)
        *kind = crypto::Error::None;
    // 0.99.0 (item 113): an encrypted backup is decrypted into the work folder (needs a recipient's secret key, unlocked:
    // the window asks first), restored from there as any backup, and the plain copy goes
    if (zipFile.endsWith(QLatin1String(".gpg"), Qt::CaseInsensitive)) {
        const QString work = workDirectory();
        QDir().mkpath(work);
        const QString plain = work + QStringLiteral("/restore-") + QUuid::createUuid().toString(QUuid::Id128) + QStringLiteral(".zip");
        crypto::Error e = crypto::Error::None;
        auto decrypt = [&] {
            std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
            e = crypto::KeyRing::device().decryptFile(zipFile, plain);
        };
        if (progressHandler()) {
            QThread *t = QThread::create(decrypt);
            t->start();
            while (!t->wait(50)) {
                bool stop = false;
                progressHandler()(QObject::tr("Decrypting \"%1\"...").arg(QFileInfo(zipFile).completeBaseName()), 0, 0, &stop);
            }
            delete t;
            progressHandler()(QString(), 0, 0, nullptr);
        } else {
            decrypt();
        }
        if (e != crypto::Error::None) {
            QFile::remove(plain);
            if (kind)
                *kind = e;
            if (error)
                *error = QObject::tr("%1 could not be decrypted: %2 Nothing was restored.")
                             .arg(QDir::toNativeSeparators(zipFile), crypto::errorText(e));
            return QString();
        }
        const QString dest = restore(plain, intoParent, projectFile, error, nullptr);
        QFile::remove(plain);
        return dest;
    }
    ZipReader zr(zipFile);
    if (!zr.isValid()) {
        // 0.81.0 (audit of 0.80.0, U-01): the reader knows ZIP64 now (backups of 2 GiB and more, 65,535 files and
        // more), so this is a damaged file; the reason is given, and the way that may still help
        if (error)
            *error = QObject::tr("%1 could not be read as a ZIP file (%2). If the file is whole, an archive program of "
                                 "your system may still unpack it.")
                         .arg(QDir::toNativeSeparators(zipFile), zr.errorString());
        return QString();
    }
    // the backup's first folder is the project (a .scriv or a project folder); a project in Scrivener format
    // has its working copy beside it
    QString top;
    for (const QString &n : zr.fileNames()) {
        const QString first = n.section(QLatin1Char('/'), 0, 0);
        if (!first.isEmpty() && n.contains(QLatin1Char('/'))) {
            top = first;
            break;
        }
    }
    if (top.isEmpty()) {
        if (error)
            *error = QObject::tr("The backup holds no project folder.");
        return QString();
    }
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH-mm"));
    QString dest = QDir(intoParent).filePath(QObject::tr("%1 (restored %2)").arg(QFileInfo(top).completeBaseName(), stamp));
    for (int n = 2; QFileInfo::exists(dest) && n < 100; ++n)
        dest = QDir(intoParent).filePath(QObject::tr("%1 (restored %2, %3)").arg(QFileInfo(top).completeBaseName(), stamp).arg(n));
    // 0.81.0 (audit of 0.80.0, F-02): a restore that cannot fit is not started
    const quint64 needed = zr.totalSize();
    const QStorageInfo disk(intoParent);
    if (disk.isValid() && disk.bytesAvailable() >= 0 && needed > quint64(disk.bytesAvailable())) {
        if (error)
            *error = QObject::tr("The backup needs %1 MB, but only %2 MB are free on the drive of %3. Nothing was restored.")
                         .arg(QLocale().toString(double(needed) / 1e6, 'f', 1), QLocale().toString(double(disk.bytesAvailable()) / 1e6, 'f', 1),
                              QDir::toNativeSeparators(intoParent));
        return QString();
    }
    if (!QDir().mkpath(dest)) {
        if (error)
            *error = QObject::tr("Cannot create %1").arg(QDir::toNativeSeparators(dest));
        return QString();
    }
    // unpacked in a thread of its own, as a backup is made: the window shows the progress and Cancel
    QStringList failed;
    bool unpacked = false;
    std::atomic<qint64> done{0};
    std::atomic<bool> cancel{false};
    zr.setProgress(&done, &cancel);
    auto work = [&] { unpacked = zr.extractAll(dest, &failed); };
    if (progressHandler()) {
        QThread *t = QThread::create(work);
        t->start();
        while (!t->wait(50)) {
            bool stop = false;
            progressHandler()(QObject::tr("Restoring \"%1\"...").arg(QFileInfo(zipFile).completeBaseName()), done.load(),
                              qint64(needed), &stop);
            if (stop)
                cancel.store(true);
        }
        delete t;
        progressHandler()(QString(), qint64(needed), qint64(needed), nullptr);
    } else {
        work();
    }
    if (zr.cancelled()) {
        pathsafety::removeTreeInside(dest, intoParent);
        if (error)
            *error = QObject::tr("The restore was cancelled; the files written so far were removed.");
        return QString();
    }
    if (!unpacked) {
        // 0.81.0 (audit of 0.80.0, F-02, F-03): the files that are missing are named, with the reason
        constexpr int kNamed = 20;
        QStringList named = failed.mid(0, kNamed);
        if (failed.size() > kNamed)
            named << QObject::tr("and %n more", nullptr, int(failed.size() - kNamed));
        if (error)
            *error = QObject::tr("Not every file of the backup could be restored into %1. Not restored (%n):\n%2", nullptr,
                                 int(failed.size()))
                         .arg(QDir::toNativeSeparators(dest), named.join(QLatin1Char('\n')));
        // what was written stays: the writer can look at it
    }
    const QString projectDir = QDir(dest).filePath(top);
    // 0.92.0: the restored project is a project of its own, with its own backups (not for a .scriv: its ID comes
    // from Scrivener's identifier)
    if (!top.endsWith(QLatin1String(".scriv"), Qt::CaseInsensitive) && QDir(projectDir).exists())
        giveNewId(projectDir);
    if (projectFile) {
        if (top.endsWith(QLatin1String(".scriv"), Qt::CaseInsensitive)) {
            *projectFile = projectDir;
        } else {
            const QStringList qbx = QDir(projectDir).entryList({QStringLiteral("*") + QLatin1String(QB_PROJECT_FILE_SUFFIX)}, QDir::Files);
            *projectFile = qbx.isEmpty() ? QString() : QDir(projectDir).filePath(qbx.first());
        }
    }
    return dest;
}

} // namespace backup
} // namespace qb
