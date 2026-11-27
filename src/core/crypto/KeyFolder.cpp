#include "core/crypto/KeyFolder.h"

#include "core/Paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSaveFile>

namespace qb {
namespace crypto {
namespace keyfolder {

namespace {

QString say(const char *text) { return QCoreApplication::translate("qb::crypto::keyfolder", text); }

QString ringFolder(const QString &dir) { return dir.isEmpty() ? KeyRing::deviceDir() : dir; }

bool writeFile(const QString &path, const QByteArray &data, bool secret, QString *error)
{
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(data) != data.size() || !f.commit()) {
        if (error)
            *error = say("The file %1 could not be written.").arg(QDir::toNativeSeparators(path));
        return false;
    }
    if (secret)
        QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    return true;
}

QByteArray readFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

bool samePlaceOrInside(const QString &path, const QString &folder)
{
    if (folder.isEmpty())
        return false;
    const QString a = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    const QString b = QDir::cleanPath(QFileInfo(folder).absoluteFilePath());
    return a.compare(b, Qt::CaseSensitive) == 0 || a.startsWith(b + QLatin1Char('/'));
}

} // namespace

QString folderName(const QDate &date) { return QStringLiteral("Quillbinder Keys ") + date.toString(QStringLiteral("yyyy-MM-dd")); }

// ------------------------------------------------------------------------------------------ roles

Roles roles(const QString &ringDir)
{
    Roles r;
    const QJsonObject o = QJsonDocument::fromJson(readFile(ringFolder(ringDir) + QStringLiteral("/roles.json"))).object();
    r.mine = o.value(QStringLiteral("mine")).toString();
    r.recovery = o.value(QStringLiteral("recovery")).toString();
    return r;
}

bool setRoles(const Roles &r, const QString &ringDir)
{
    const QString dir = ringFolder(ringDir);
    if (!QDir().mkpath(dir))
        return false;
    QFile::setPermissions(dir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    const QJsonObject o{{QStringLiteral("mine"), r.mine}, {QStringLiteral("recovery"), r.recovery}};
    return writeFile(dir + QStringLiteral("/roles.json"), QJsonDocument(o).toJson(), true, nullptr);
}

// ------------------------------------------------------------------------------------------ places

QString syncProgramIn(const QString &path)
{
    static const struct {
        const char *prefix;
        const char *name;
    } known[] = {{"onedrive", "OneDrive"}, {"dropbox", "Dropbox"},         {"nextcloud", "Nextcloud"},
                 {"google drive", "Google Drive"}, {"googledrive", "Google Drive"}, {"my drive", "Google Drive"},
                 {"icloud", "iCloud"},     {"syncthing", "Syncthing"},     {"pcloud", "pCloud"},
                 {"mega", "MEGA"}};
    const QStringList parts = QDir::fromNativeSeparators(QFileInfo(path).absoluteFilePath()).split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        const QString p = part.toLower();
        for (const auto &k : known) {
            const QString prefix = QLatin1String(k.prefix);
            if (!p.startsWith(prefix))
                continue;
            const QString rest = p.mid(prefix.size());
            // the program's own folder name, or the start of it ("OneDrive - Company", "MEGAsync"), not "Megapixels"
            if (rest.isEmpty() || !rest.at(0).isLetter() || (prefix == QLatin1String("mega") && rest == QLatin1String("sync")))
                return QLatin1String(k.name);
        }
    }
    return QString();
}

QString refusedPlace(const QString &folder, const QStringList &projectFolders, const QString &backupFolder)
{
    if (folder.trimmed().isEmpty())
        return say("Choose a folder first.");
    for (const QString &p : projectFolders)
        if (!p.isEmpty() && samePlaceOrInside(folder, p))
            return say("This is a project's folder. Keys must not be kept with the files they lock. Choose another place, "
                       "for example a USB stick.");
    if (samePlaceOrInside(folder, backupFolder))
        return say("This is the backup folder. Keys must not be kept with the backups they open. Choose another place, "
                   "for example a USB stick.");
    const QString sync = syncProgramIn(folder);
    if (!sync.isEmpty())
        return say("This folder belongs to %1, which copies it to other computers or to the internet. Choose another place, "
                   "for example a USB stick.")
            .arg(sync);
    return QString();
}

// ------------------------------------------------------------------------------------------ README

QString readmeText(const QString &myFingerprint, const QString &recoveryFingerprint, const QDate &date, bool withRecoverySecret)
{
    QStringList p;
    p << say("QUILLBINDER KEYS");
    p << say("Made by Quillbinder on %1. This folder holds the two keys that lock your encrypted Quillbinder projects and "
             "backups. There is no password and no secret written in this file.")
             .arg(date.toString(QStringLiteral("yyyy-MM-dd")));
    p << say("The files:");
    p << say("my-key-secret.asc: your key. It is locked by your passphrase. With this file and the passphrase you can open "
             "your encrypted files.");
    p << say("my-key-public.asc: the public part of your key. It can only lock files, never open them. It is not secret.");
    if (withRecoverySecret)
        p << say("recovery-key-secret.asc: the recovery key, locked by its own passphrase. The recovery passphrase was shown "
                 "once on the screen and was not saved anywhere: it is only on the paper you wrote it on.");
    else
        p << say("The secret part of the recovery key is not in this folder: it is only in the folder that was made when the "
                 "keys were created, and its passphrase is only on paper.");
    if (!recoveryFingerprint.isEmpty())
        p << say("recovery-key-public.asc: the public part of the recovery key. It is not secret.");
    p << say("Where to keep this folder: on a USB stick or another drive that stays away from your computer, in a safe place. "
             "Do not keep it in a project folder, in a backup folder or in a folder that a sync program (OneDrive, Dropbox, "
             "Nextcloud, Google Drive, iCloud and others) copies. Keep the paper with the recovery passphrase in another "
             "place than the stick: the stick alone or the paper alone opens nothing.");
    p << say("If you lose your key and the recovery key, or the passphrases for both, your encrypted text is gone. Nobody can "
             "get it back, not even the makers of Quillbinder.");
    p << say("Opening your files without Quillbinder: the files are standard OpenPGP files, so Kleopatra, Gpg4win (Windows) "
             "and GnuPG (the gpg command) open them. A backup is a file named like <project>.zip.gpg: it is a ZIP file "
             "locked by your key.");
    p << say("With Kleopatra or Gpg4win: choose File > Import, pick my-key-secret.asc (or recovery-key-secret.asc), then "
             "choose File > Decrypt/Verify, pick the .zip.gpg file and type the passphrase. You get a .zip file, which you "
             "open like any ZIP file.");
    p << say("With gpg on the command line: gpg --import my-key-secret.asc, then gpg --output backup.zip --decrypt "
             "backup.zip.gpg (gpg asks for the passphrase), then unzip backup.zip. For the recovery key use "
             "recovery-key-secret.asc and the recovery passphrase.");
    p << say("On another computer or phone: in Quillbinder choose File > Options > Encryption > Import Keys, pick this "
             "folder and type your passphrase once. Quillbinder takes your key and the public part of the recovery key; "
             "the recovery key's secret part stays on the stick.");
    p << say("Fingerprints (to check that a key is the right one):");
    p << say("Your key: %1").arg(groupedFingerprint(myFingerprint));
    if (!recoveryFingerprint.isEmpty())
        p << say("Recovery key: %1").arg(groupedFingerprint(recoveryFingerprint));
    return p.join(QStringLiteral("\n\n")) + QLatin1Char('\n');
}

// ------------------------------------------------------------------------------------------ making

NewKeys makeKeys(KeyRing &ring, const QByteArray &myPassphrase, const QByteArray &recoveryPassphrase)
{
    NewKeys n;
    Error e = Error::None;
    n.mine = ring.createKey(QStringLiteral("Quillbinder key"), myPassphrase, &e);
    if (n.mine.isEmpty()) {
        n.error = errorText(e);
        return n;
    }
    n.recovery = ring.createKey(QStringLiteral("Quillbinder recovery key"), recoveryPassphrase, &e);
    if (n.recovery.isEmpty()) {
        n.error = errorText(e);
        n.mine.clear();
    }
    return n;
}

Written writeFolder(const KeyRing &ring, const Roles &keys, const QString &parent, const QDate &date, bool withRecoverySecret)
{
    Written w;
    Error e = Error::None;
    const QByteArray mySecret = ring.exportSecretKey(keys.mine, &e);
    if (mySecret.isEmpty()) {
        w.error = errorText(e == Error::None ? Error::UnknownKey : e);
        return w;
    }
    const QByteArray myPublic = ring.exportPublicKey(keys.mine, &e);
    const bool haveRecovery = !keys.recovery.isEmpty();
    const QByteArray recPublic = haveRecovery ? ring.exportPublicKey(keys.recovery, &e) : QByteArray();
    QByteArray recSecret;
    if (withRecoverySecret)
        recSecret = ring.exportSecretKey(keys.recovery, &e);
    if (myPublic.isEmpty() || (haveRecovery && recPublic.isEmpty()) || (withRecoverySecret && recSecret.isEmpty())) {
        w.error = errorText(e == Error::None ? Error::UnknownKey : e);
        return w;
    }
    QDir().mkpath(parent);
    QString dir = QDir(parent).filePath(folderName(date));
    for (int n = 2; QFileInfo::exists(dir); ++n)
        dir = QDir(parent).filePath(folderName(date) + QStringLiteral(" (%1)").arg(n));
    if (!QDir().mkpath(dir)) {
        w.error = say("The folder %1 could not be made.").arg(QDir::toNativeSeparators(dir));
        return w;
    }
    bool ok = writeFile(dir + QLatin1Char('/') + myKeySecretName(), mySecret, true, &w.error)
              && writeFile(dir + QLatin1Char('/') + myKeyPublicName(), myPublic, false, &w.error);
    if (ok && haveRecovery)
        ok = writeFile(dir + QLatin1Char('/') + recoveryPublicName(), recPublic, false, &w.error);
    if (ok && withRecoverySecret)
        ok = writeFile(dir + QLatin1Char('/') + recoverySecretName(), recSecret, true, &w.error);
    if (ok) {
        const QString text = readmeText(keys.mine, keys.recovery, date, withRecoverySecret);
        ok = writeFile(dir + QLatin1Char('/') + readmeName(), text.toUtf8(), false, &w.error);
    }
    if (!ok) {
        QDir(dir).removeRecursively();
        return w;
    }
    w.ok = true;
    w.folder = dir;
    return w;
}

// ------------------------------------------------------------------------------------------ checking

CheckReport checkFolder(const QString &folder, const QByteArray &myPassphrase, const QByteArray &recoveryPassphrase)
{
    CheckReport r;
    const QDir d(folder);
    const QByteArray mySecret = readFile(d.filePath(myKeySecretName()));
    const QByteArray myPublic = readFile(d.filePath(myKeyPublicName()));
    const QByteArray recSecret = readFile(d.filePath(recoverySecretName()));
    const QByteArray recPublic = readFile(d.filePath(recoveryPublicName()));
    r.readmeOk = QFileInfo(d.filePath(readmeName())).size() > 0;
    if (mySecret.isEmpty() || myPublic.isEmpty() || recSecret.isEmpty() || recPublic.isEmpty()) {
        r.error = say("A key file is missing or empty.");
        return r;
    }
    KeyRing a;     // what a device that takes the folder sees: the writer's key (secret), the recovery key (public)
    KeyRing b;     // the recovery key as its owner would open it
    const ImportReport i1 = a.importKeys(mySecret);
    const ImportReport i2 = a.importKeys(recPublic);
    const ImportReport i3 = a.importKeys(myPublic);
    const ImportReport i4 = b.importKeys(recSecret);
    if (i1.imported.size() != 1 || i1.withSecret.size() != 1 || i2.imported.size() != 1 || i3.imported.size() != 1
        || i4.imported.size() != 1 || i4.withSecret.size() != 1) {
        r.error = say("A key file could not be read back.");
        return r;
    }
    r.filesRead = true;
    QByteArray test(1024, 0);
    for (int i = 0; i < test.size(); i += 4)
        *reinterpret_cast<quint32 *>(test.data() + i) = QRandomGenerator::system()->generate();
    Error e = Error::None;
    const QByteArray cipher = a.encrypt(test, {i1.imported.first(), i2.imported.first()}, &e);
    if (cipher.isEmpty()) {
        r.error = errorText(e);
        return r;
    }
    if (a.unlock(i1.imported.first(), myPassphrase) == Error::None) {
        const QByteArray plain = a.decrypt(cipher, &e);
        r.myKeyWorks = !plain.isEmpty() && plain == test;
    } else {
        r.error = errorText(Error::WrongPassphrase);
    }
    if (b.unlock(i4.imported.first(), recoveryPassphrase) == Error::None) {
        const QByteArray plain = b.decrypt(cipher, &e);
        r.recoveryWorks = !plain.isEmpty() && plain == test;
    } else if (r.error.isEmpty()) {
        r.error = errorText(Error::WrongPassphrase);
    }
    return r;
}

Written copyFolder(const QString &folder, const QString &parent)
{
    Written w;
    QDir().mkpath(parent);
    const QString name = QFileInfo(folder).fileName();
    QString dir = QDir(parent).filePath(name);
    for (int n = 2; QFileInfo::exists(dir); ++n)
        dir = QDir(parent).filePath(name + QStringLiteral(" (%1)").arg(n));
    if (!QDir().mkpath(dir)) {
        w.error = say("The folder %1 could not be made.").arg(QDir::toNativeSeparators(dir));
        return w;
    }
    const QStringList files = QDir(folder).entryList(QDir::Files);
    for (const QString &f : files) {
        const bool secret = f.endsWith(QLatin1String("secret.asc"));
        if (!writeFile(dir + QLatin1Char('/') + f, readFile(QDir(folder).filePath(f)), secret, &w.error)) {
            QDir(dir).removeRecursively();
            return w;
        }
    }
    w.ok = true;
    w.folder = dir;
    return w;
}

// ------------------------------------------------------------------------------------------ importing

namespace {
Imported refuse(const ImportReport &r, const QString &fallback)
{
    Imported out;
    out.code = r.error == Error::None ? Error::NotAKey : r.error;
    if (!r.refused.isEmpty())
        out.error = r.refused.first().second;
    else
        out.error = fallback;
    return out;
}
} // namespace

Imported importFolder(KeyRing &ring, const QString &folder, const QByteArray &passphrase)
{
    Imported out;
    const QDir d(folder);
    const QByteArray mySecret = readFile(d.filePath(myKeySecretName()));
    const QByteArray recPublic = readFile(d.filePath(recoveryPublicName()));
    if (mySecret.isEmpty()) {
        out.code = Error::NotAKey;
        out.error = say("This folder has no %1. Choose the folder \"Quillbinder Keys ...\" that Create Keys wrote.").arg(myKeySecretName());
        return out;
    }
    // first into a ring of its own: a wrong passphrase must leave this device's ring as it was
    KeyRing probe;
    const ImportReport pr = probe.importKeys(mySecret);
    if (pr.imported.isEmpty() || pr.withSecret.isEmpty())
        return refuse(pr, say("The file %1 holds no usable key.").arg(myKeySecretName()));
    const Error u = probe.unlock(pr.withSecret.first(), passphrase);
    if (u != Error::None) {
        out.code = u;
        out.error = errorText(u);
        return out;
    }
    const ImportReport r1 = ring.importKeys(mySecret);
    if (r1.withSecret.isEmpty())
        return refuse(r1, say("The file %1 holds no usable key.").arg(myKeySecretName()));
    out.mine = r1.withSecret.first();
    const QByteArray myPublic = readFile(d.filePath(myKeyPublicName()));
    if (!myPublic.isEmpty())
        ring.importKeys(myPublic);
    if (!recPublic.isEmpty()) {
        const ImportReport r2 = ring.importKeys(recPublic);
        if (!r2.imported.isEmpty())
            out.recovery = r2.imported.first();
        else if (!r2.refused.isEmpty())
            out.notes << r2.refused.first().second;
    } else {
        out.notes << say("The folder has no %1: files cannot be locked for the recovery key until it is added.").arg(recoveryPublicName());
    }
    ring.unlock(out.mine, passphrase);
    out.ok = true;
    return out;
}

Imported importKeyFile(KeyRing &ring, const QByteArray &data, const QByteArray &passphrase)
{
    Imported out;
    KeyRing probe;
    const ImportReport pr = probe.importKeys(data);
    if (pr.imported.isEmpty())
        return refuse(pr, errorText(Error::NotAKey));
    if (!pr.withSecret.isEmpty()) {
        const Error u = probe.unlock(pr.withSecret.first(), passphrase);
        if (u != Error::None) {
            out.code = u;
            out.error = errorText(u);
            return out;
        }
    }
    const ImportReport r = ring.importKeys(data);
    if (r.imported.isEmpty())
        return refuse(r, errorText(Error::NotAKey));
    if (!r.withSecret.isEmpty()) {
        out.mine = r.withSecret.first();
        ring.unlock(out.mine, passphrase);
    } else {
        out.recovery = r.imported.first();
    }
    for (const auto &x : r.refused)
        out.notes << x.second;
    out.ok = true;
    return out;
}

Written exportKeys(const KeyRing &ring, const Roles &keys, const QString &parent, const QDate &date)
{
    return writeFolder(ring, keys, parent, date, false);
}

} // namespace keyfolder
} // namespace crypto
} // namespace qb
