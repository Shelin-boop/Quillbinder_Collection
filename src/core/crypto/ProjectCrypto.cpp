// The encrypted project and the encrypted backups (0.99.0, item 113, part 2): see ProjectCrypto.h.
#include "core/crypto/ProjectCrypto.h"

#include "core/Platform.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/SaveBatch.h"
#include "core/SyncSafety.h"
#include "core/TestSwitches.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/Vault.h"

#include "AppInfo.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>

namespace qb {
namespace crypto {

namespace {
QString say(const char *text) { return QCoreApplication::translate("qb::crypto", text); }

const char *kFormat = "quillbinder-encryption";
const QLatin1String kConvertSuffix(".qbconvert");

bool isFingerprint(const QString &f)
{
    static const QRegularExpression hex(QStringLiteral("^[0-9A-F]{40}$"));
    return hex.match(f).hasMatch();
}

bool writeMarker(const QString &folder, const ProjectSettings &s, QString *error)
{
    QJsonObject o;
    o.insert(QStringLiteral("format"), QLatin1String(kFormat));
    o.insert(QStringLiteral("version"), 1);
    o.insert(QStringLiteral("encryptedProject"), s.encryptedProject);
    o.insert(QStringLiteral("encryptedBackups"), s.encryptedBackups || s.encryptedProject);
    o.insert(QStringLiteral("writerKey"), s.recipients.writer);
    o.insert(QStringLiteral("recoveryKey"), s.recipients.recovery);
    if (!s.converting.isEmpty()) {
        o.insert(QStringLiteral("converting"), s.converting);
        o.insert(QStringLiteral("convertingDevice"), s.convertingDevice);
        o.insert(QStringLiteral("convertingSince"), s.convertingSince);
    }
    QString why;
    if (!safesave::writeFile(markerPath(folder), QJsonDocument(o).toJson(), &why)) {
        if (error)
            *error = why;
        return false;
    }
    savebatch::syncDir(folder);
    return true;
}

// no switch on: the marker goes
bool removeMarker(const QString &folder, QString *error)
{
    const QString m = markerPath(folder);
    if (QFile::exists(m) && !QFile::remove(m)) {
        if (error)
            *error = say("%1 could not be removed.").arg(QDir::toNativeSeparators(m));
        return false;
    }
    savebatch::syncDir(folder);
    return true;
}

bool publicKeysHere(const Recipients &r, QString *error)
{
    if (!r.isComplete()) {
        if (error)
            *error = say("The project needs the writer's key and the recovery key.");
        return false;
    }
    std::lock_guard<std::recursive_mutex> g(deviceRingLock());
    for (const QString &f : r.list()) {
        if (!KeyRing::device().contains(f)) {
            if (error)
                *error = say("The key %1 is not in Quillbinder's key ring.").arg(groupedFingerprint(f));
            return false;
        }
    }
    return true;
}

// every file of the folder that the conversion handles: all but the plain ones; leftovers of a conversion cut off go
QStringList filesToConvert(const QString &folder)
{
    const QString root = QDir::cleanPath(QFileInfo(folder).absoluteFilePath());
    const QStringList plain = vault::projectPlainNames();
    QStringList out;
    QDirIterator it(root, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QString rel = path.mid(root.size() + 1);
        if (plain.contains(rel))
            continue;
        const QString name = it.fileName();
        if (name.endsWith(kConvertSuffix) || name.contains(kConvertSuffix + QLatin1Char('.'))) {
            QFile::remove(path); // never project data: what a conversion cut off left
            continue;
        }
        out << path;
    }
    std::sort(out.begin(), out.end());
    return out;
}

bool headSealed(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    return vault::isSealedData(f.read(64));
}

// one file into its new form: written whole beside it, synced (QSaveFile in the layer), renamed over it in one step
Error convertOne(const QString &path, bool toEncrypted, const QStringList &recipients)
{
    const QString tmp = path + kConvertSuffix;
    Error e;
    {
        std::lock_guard<std::recursive_mutex> g(deviceRingLock());
        e = toEncrypted ? KeyRing::device().encryptFile(path, tmp, recipients) : KeyRing::device().decryptFile(path, tmp);
    }
    if (e != Error::None) {
        QFile::remove(tmp);
        return e;
    }
    QFile::setPermissions(tmp, QFile::permissions(path));
    if (!savebatch::replace(tmp, path)) {
        QFile::remove(tmp);
        return Error::Io;
    }
    return Error::None;
}

// The conversion itself, in the direction the marker names (written by the caller)
bool runConversion(const QString &folder, bool toEncrypted, const QStringList &recipients, const Progress &progress,
                   QString *error)
{
    const QStringList files = filesToConvert(folder);
    const int total = int(files.size());
    int done = 0, converted = 0;
    QStringList failed;
    QSet<QString> dirs;
    // the tests' switch: the conversion stops after this many files as if the program were killed (nothing more is done)
    bool stopOk = false;
    const int stopAfter = QB_TEST_ENV("QB_TEST_CONVERT_STOP").toInt(&stopOk);
    for (const QString &f : files) {
        if (progress && !progress(done, total)) {
            if (error)
                *error = say("The conversion was stopped after %1 of %2 files; the next opening of the project finishes it.")
                             .arg(done)
                             .arg(total);
            return false;
        }
        if (headSealed(f) != toEncrypted) {
            if (stopOk && converted >= stopAfter) {
                if (error)
                    *error = QStringLiteral("stopped by QB_TEST_CONVERT_STOP after %1 files").arg(converted);
                return false;
            }
            const Error e = convertOne(f, toEncrypted, recipients);
            if (e != Error::None)
                failed << QStringLiteral("%1: %2").arg(QDir(folder).relativeFilePath(f), errorText(e));
            else
                ++converted;
            dirs.insert(QFileInfo(f).absolutePath());
        }
        ++done;
    }
    if (progress)
        progress(total, total);
    for (const QString &d : std::as_const(dirs))
        savebatch::syncDir(d);
    if (!failed.isEmpty()) {
        if (error) {
            QStringList named = failed.mid(0, 10);
            if (failed.size() > 10)
                named << say("and %1 more").arg(failed.size() - 10);
            *error = (toEncrypted ? say("%1 of %2 files could not be encrypted; the project opens as it is and the next opening "
                                        "tries again:\n%3")
                                  : say("%1 of %2 files could not be decrypted; the project opens as it is and the next opening "
                                        "tries again:\n%3"))
                         .arg(failed.size())
                         .arg(total)
                         .arg(named.join(QLatin1Char('\n')));
        }
        return false;
    }
    return true;
}

bool journalWaiting(const QString &folder, QString *error)
{
    if (QFile::exists(QDir(folder).filePath(QStringLiteral("settings/journal.json")))) {
        if (error)
            *error = say("A save of another computer is still arriving in the project folder (settings/journal.json). Open the "
                         "project once, or try again in a few minutes.");
        return true;
    }
    return false;
}

QSet<QString> &keyStoreTried()
{
    static QSet<QString> tried;
    return tried;
}

ImportKeysHandler &importHandler()
{
    static ImportKeysHandler h;
    return h;
}
} // namespace

bool Recipients::isComplete() const { return isFingerprint(writer) && isFingerprint(recovery) && writer != recovery; }

QStringList Recipients::list() const { return {writer, recovery}; }

QString markerName() { return QStringLiteral("encryption.json"); }

QString markerPath(const QString &projectFolder) { return QDir(projectFolder).filePath(markerName()); }

ProjectSettings settingsOf(const QString &projectFolder, QString *error)
{
    ProjectSettings s;
    QFile f(markerPath(projectFolder));
    if (!f.exists())
        return s;
    if (!f.open(QIODevice::ReadOnly)) {
        s.unreadable = true;
        if (error)
            *error = f.errorString();
        return s;
    }
    QJsonParseError pe;
    const QJsonDocument d = QJsonDocument::fromJson(f.read(64 * 1024), &pe);
    const QJsonObject o = d.object();
    if (pe.error != QJsonParseError::NoError || o.value(QStringLiteral("format")).toString() != QLatin1String(kFormat)) {
        s.unreadable = true;
        if (error)
            *error = say("%1 is not Quillbinder's encryption marker.").arg(QDir::toNativeSeparators(f.fileName()));
        return s;
    }
    s.recipients.writer = o.value(QStringLiteral("writerKey")).toString().toUpper();
    s.recipients.recovery = o.value(QStringLiteral("recoveryKey")).toString().toUpper();
    s.encryptedProject = o.value(QStringLiteral("encryptedProject")).toBool();
    s.encryptedBackups = o.value(QStringLiteral("encryptedBackups")).toBool() || s.encryptedProject;
    s.converting = o.value(QStringLiteral("converting")).toString();
    if (s.converting != QLatin1String("encrypt") && s.converting != QLatin1String("decrypt"))
        s.converting.clear();
    s.convertingDevice = o.value(QStringLiteral("convertingDevice")).toString();
    s.convertingSince = o.value(QStringLiteral("convertingSince")).toString();
    return s;
}

QString markerProblem(const QString &projectFolder)
{
    const ProjectSettings s = settingsOf(projectFolder);
    if (s.unreadable)
        return say("The file %1 of this project, which says whether its files and backups are encrypted, cannot be read. "
                   "Nothing was changed, and no backup is made, until it is back (an earlier version of it may be in a "
                   "backup or in your sync program's version history).")
            .arg(markerName());
    if (s.isEncryptedFolder()) {
        if (!s.recipients.isComplete())
            return say("The file %1 of this project does not name both keys of the project. Nothing was changed, and no backup "
                       "is made, until it is back (an earlier version of it may be in a backup or in your sync program's "
                       "version history).")
                .arg(markerName());
        return QString();
    }
    // no switch on: an encrypted project file says that the marker went missing
    const QStringList files = QDir(projectFolder).entryList({QStringLiteral("*") + QLatin1String(QB_PROJECT_FILE_SUFFIX)}, QDir::Files);
    for (const QString &f : files) {
        QFile in(QDir(projectFolder).filePath(f));
        if (in.open(QIODevice::ReadOnly) && vault::isSealedData(in.read(64)))
            return say("The project file of this project is encrypted, but its file %1 is missing. Nothing was changed, and "
                       "no backup is made, until it is back (an earlier version of it may be in a backup or in your sync "
                       "program's version history).")
                .arg(markerName());
    }
    return QString();
}

bool unlockedHere(const Recipients &recipients)
{
    if (!available() || !recipients.isComplete())
        return false;
    std::lock_guard<std::recursive_mutex> g(deviceRingLock());
    for (const QString &f : recipients.list())
        if (KeyRing::device().hasSecret(f) && KeyRing::device().isUnlocked(f))
            return true;
    return false;
}

QString tempFolderBase() { return vault::active() ? vault::privateFolder() : QDir::tempPath(); }

// ---- switch 1

bool backupsEncrypted(const Project *p) { return p && settingsOf(p->folderPath()).encryptedBackups; }

bool canChangeBackupEncryption(const Project *p, QString *reason)
{
    if (!available()) {
        if (reason)
            *reason = errorText(Error::Unavailable);
        return false;
    }
    if (p && settingsOf(p->folderPath()).encryptedProject) {
        if (reason)
            *reason = say("The backups of an encrypted project are always encrypted.");
        return false;
    }
    return p != nullptr;
}

bool setBackupsEncrypted(Project *p, bool on, const Recipients &recipients, QString *error)
{
    if (!p)
        return false;
    ProjectSettings s = settingsOf(p->folderPath());
    if (s.encryptedProject) {
        if (!on || (recipients.isComplete() && recipients != s.recipients)) {
            if (error)
                *error = say("The backups of an encrypted project are always encrypted, to the project's keys.");
            return false;
        }
        return true;
    }
    if (!on) {
        if (!s.encryptedBackups)
            return true;
        return removeMarker(p->folderPath(), error);
    }
    if (!available()) {
        if (error)
            *error = errorText(Error::Unavailable);
        return false;
    }
    if (!publicKeysHere(recipients, error))
        return false;
    s.encryptedBackups = true;
    s.recipients = recipients;
    return writeMarker(p->folderPath(), s, error);
}

// ---- switch 2

bool projectEncrypted(const Project *p) { return p && settingsOf(p->folderPath()).encryptedProject; }

bool canChangeProjectEncryption(const Project *p, QString *reason)
{
    if (!p)
        return false;
    if (!available()) {
        if (reason)
            *reason = errorText(Error::Unavailable);
        return false;
    }
    if (!p->scrivenerPath.isEmpty()) {
        if (reason)
            *reason = say("A project in Scrivener format cannot be encrypted: Scrivener cannot read encrypted files. Its "
                          "backups can be encrypted.");
        return false;
    }
    // 0.99.0 (encryption review M1): another computer or window that has the project open would write its files in the old
    // way while they are converted (a late file would lie in a folder no longer in the vault)
    {
        const syncsafe::LockState st = syncsafe::judge(syncsafe::readLock(p->settingsDir() + QStringLiteral("/lock")),
                                                       syncsafe::staleAfterSeconds(), platform::processAlive);
        if (st == syncsafe::LockState::OtherDevice || st == syncsafe::LockState::OtherWindow) {
            if (reason)
                *reason = say("Another computer or another Quillbinder window has this project open. Close it there first: its "
                              "saves would otherwise reach the project's folder in the old way while its files are "
                              "converted.");
            return false;
        }
    }
    return !journalWaiting(p->folderPath(), reason);
}

Recipients recipientsOf(const Project *p) { return p ? settingsOf(p->folderPath()).recipients : Recipients(); }

bool encryptFolder(const QString &folder, const Recipients &recipients, const Progress &progress, QString *error)
{
    if (!available()) {
        if (error)
            *error = errorText(Error::Unavailable);
        return false;
    }
    ProjectSettings s = settingsOf(folder);
    if (s.encryptedProject && s.converting.isEmpty())
        return true;
    const Recipients r = s.encryptedProject && s.recipients.isComplete() ? s.recipients : recipients;
    if (!publicKeysHere(r, error) || journalWaiting(folder, error))
        return false;
    // 1. the marker first, with the direction: from now on every reader takes both kinds of files, and an opening finishes
    s.encryptedProject = true;
    s.encryptedBackups = true;
    s.recipients = r;
    s.converting = QStringLiteral("encrypt");
    s.convertingDevice = syncsafe::machineKey();
    s.convertingSince = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    if (!writeMarker(folder, s, error))
        return false;
    vault::add(folder, r.list(), vault::projectPlainNames(), true);
    // 2. every file on its own; 3. the marker's final state only when all are done
    const bool ok = runConversion(folder, true, r.list(), progress, error);
    vault::remove(folder);
    if (!ok)
        return false;
    s.converting.clear();
    s.convertingDevice.clear();
    s.convertingSince.clear();
    return writeMarker(folder, s, error);
}

bool decryptFolder(const QString &folder, const Progress &progress, QString *error)
{
    ProjectSettings s = settingsOf(folder);
    if (!s.encryptedProject)
        return true;
    QString fp;
    const Access a = accessFor(folder, &fp);
    if (a != Access::Ready) {
        if (error)
            *error = a == Access::NeedsPassphrase ? errorText(Error::Locked) : accessText(a, folder);
        return false;
    }
    if (journalWaiting(folder, error))
        return false;
    s.converting = QStringLiteral("decrypt");
    s.convertingDevice = syncsafe::machineKey();
    s.convertingSince = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    if (!writeMarker(folder, s, error))
        return false;
    vault::add(folder, s.recipients.list(), vault::projectPlainNames(), false);
    const bool ok = runConversion(folder, false, s.recipients.list(), progress, error);
    vault::remove(folder);
    if (!ok)
        return false;
    // the backups' switch stays as it was before the project was encrypted: on (the writer turns it off on its own)
    s.encryptedProject = false;
    s.converting.clear();
    s.convertingDevice.clear();
    s.convertingSince.clear();
    return writeMarker(folder, s, error);
}

bool finishConversion(const QString &folder, const Progress &progress, QString *error)
{
    if (error)
        error->clear();
    const ProjectSettings s = settingsOf(folder);
    if (s.converting == QLatin1String("encrypt"))
        return encryptFolder(folder, s.recipients, progress, error);
    if (s.converting == QLatin1String("decrypt"))
        return decryptFolder(folder, progress, error);
    return true;
}

// ---- opening

Access accessFor(const QString &projectFolder, QString *fingerprint)
{
    const ProjectSettings s = settingsOf(projectFolder);
    if (!markerProblem(projectFolder).isEmpty())
        return Access::MarkerDamaged;
    if (!s.isEncryptedFolder())
        return Access::Plain;
    if (fingerprint)
        *fingerprint = s.recipients.writer;
    if (!available())
        return Access::Unavailable;
    std::lock_guard<std::recursive_mutex> g(deviceRingLock());
    KeyRing &ring = KeyRing::device();
    // 0.99.0 (encryption review H3): every write is encrypted to both keys, so both public keys must be here; a device
    // with only its secret key would open the project and fail at every save
    QString missing;
    for (const QString &f : s.recipients.list())
        if (!ring.contains(f)) {
            missing = f;
            break;
        }
    auto ready = [&] {
        if (!missing.isEmpty()) {
            if (fingerprint)
                *fingerprint = missing;
            return Access::PublicKeyMissing;
        }
        return Access::Ready;
    };
    QStringList locked;
    for (const QString &f : s.recipients.list()) {
        if (f.isEmpty() || !ring.hasSecret(f))
            continue;
        if (ring.isUnlocked(f))
            return ready();
        locked << f;
    }
    // the passphrase kept on this device (a wrong one, or a store that refused, is not tried again in this session; a
    // passphrase typed later and kept is: unlockAndRemember)
    for (const QString &f : std::as_const(locked)) {
        if (keyStoreTried().contains(f))
            continue;
        QByteArray pass;
        const KeyStore::Status st = KeyStore::system().read(KeyStore::passphraseEntry(f), &pass);
        if (st == KeyStore::Status::Ok) {
            const Error e = ring.unlock(f, pass);
            wipe(pass);
            if (e == Error::None)
                return ready();
            keyStoreTried().insert(f);
        } else if (st == KeyStore::Status::Denied) {
            keyStoreTried().insert(f);
        }
    }
    if (!locked.isEmpty()) {
        if (fingerprint)
            *fingerprint = locked.first();
        return Access::NeedsPassphrase;
    }
    return Access::NoSecretKey;
}

Error unlockAndRemember(const QString &fingerprint, const QByteArray &passphrase, bool *kept)
{
    if (kept)
        *kept = false;
    std::lock_guard<std::recursive_mutex> g(deviceRingLock());
    const Error e = KeyRing::device().unlock(fingerprint, passphrase);
    if (e != Error::None)
        return e;
    const KeyStore::Status st = KeyStore::system().write(KeyStore::passphraseEntry(fingerprint), passphrase);
    if (kept)
        *kept = st == KeyStore::Status::Ok;
    if (st == KeyStore::Status::Ok)
        keyStoreTried().remove(fingerprint); // the store holds the right one now
    return e;
}

QString accessText(Access access, const QString &projectFolder)
{
    const ProjectSettings s = settingsOf(projectFolder);
    switch (access) {
    case Access::Plain:
    case Access::Ready:
        return QString();
    case Access::NeedsPassphrase:
        return say("This project is encrypted. Enter the passphrase of your key to open it.");
    case Access::NoSecretKey:
        return say("This project is encrypted to keys whose secret part is not on this device:\n\n%1 (your key)\n%2 (the "
                   "recovery key)\n\nImport your key on this device to open the project here. Nothing in the project was "
                   "changed.")
            .arg(groupedFingerprint(s.recipients.writer), groupedFingerprint(s.recipients.recovery));
    case Access::Unavailable:
        return say("This project is encrypted, and this build of Quillbinder has no encryption. Nothing in the project was "
                   "changed.");
    case Access::PublicKeyMissing: {
        QStringList absent;
        {
            std::lock_guard<std::recursive_mutex> g(deviceRingLock());
            for (const QString &f : s.recipients.list())
                if (!KeyRing::device().contains(f))
                    absent << groupedFingerprint(f);
        }
        return say("This project is encrypted to two keys, and the public part of %1 is not on this device, so nothing could be "
                   "saved in it here. Import it (the file recovery-key-public.asc or my-key-public.asc of your keys folder) to "
                   "open the project here. Nothing in the project was changed.")
            .arg(absent.join(QStringLiteral(", ")));
    }
    case Access::MarkerDamaged:
        return markerProblem(projectFolder);
    }
    return QString();
}

void setImportKeysHandler(ImportKeysHandler handler) { importHandler() = std::move(handler); }

const ImportKeysHandler &importKeysHandler() { return importHandler(); }

std::recursive_mutex &deviceRingLock()
{
    static std::recursive_mutex m;
    return m;
}

} // namespace crypto
} // namespace qb
