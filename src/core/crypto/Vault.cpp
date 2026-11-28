// The layer under every read and write of an encrypted project (0.99.0, item 113, part 2): see Vault.h.
#include "core/crypto/Vault.h"

#include "core/Platform.h"
#include "core/SafeSave.h"
#include "core/SyncSafety.h"
#include "core/TestSwitches.h"
#include "core/crypto/ProjectCrypto.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QVector>

#include <atomic>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace qb {
namespace vault {

namespace {
QString say(const char *text) { return QCoreApplication::translate("qb::vault", text); }

struct Entry {
    QString folder; // clean, absolute, without a trailing "/"
    QStringList recipients;
    QStringList plainNames; // relative to the folder
    bool sealNew = true;
    int refs = 0;
};

QMutex g_lock;
QVector<Entry> g_folders;
std::atomic<int> g_count{0};

struct PlainCopy {
    QString copy;
    QString folder;
    syncsafe::Stamp stamp;
};
QHash<QString, PlainCopy> g_copies; // original -> its decrypted copy (window's thread only)

QString clean(const QString &path)
{
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

// a session-key packet (tag 1, version 3) at the start: what Quillbinder (RNP) and GnuPG write first
bool sessionKeyPacket(const QByteArray &d)
{
    if (d.size() < 12)
        return false;
    const auto at = [&d](qsizetype i) { return static_cast<unsigned char>(d.at(i)); };
    const unsigned char b = at(0);
    if (!(b & 0x80))
        return false;
    qsizetype body;
    int tag;
    if (b & 0x40) {
        tag = b & 0x3f;
        const unsigned char o1 = at(1);
        if (o1 < 192)
            body = 2;
        else if (o1 < 224)
            body = 3;
        else if (o1 == 255)
            body = 6;
        else
            return false; // partial lengths are not allowed for this packet
    } else {
        tag = (b >> 2) & 0x0f;
        const int type = b & 3;
        if (type == 3)
            return false;
        body = 1 + (type == 0 ? 1 : type == 1 ? 2 : 4);
    }
    if (tag != 1 || d.size() < body + 10)
        return false;
    if (at(body) != 3)
        return false;
    const unsigned char algorithm = at(body + 9);
    return algorithm == 18 || algorithm == 1 || algorithm == 2 || algorithm == 16 || algorithm == 25;
}

QString privateDir()
{
    static QString dir;
    if (!dir.isEmpty())
        return dir;
    QString base = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (base.isEmpty() || !QFileInfo(base).isDir() || !QFileInfo(base).isWritable())
        base = QDir::tempPath();
    const QString prefix = QStringLiteral("quillbinder-plain-");
    // the folders of program runs that ended without removing theirs (a crash)
    const QStringList old = QDir(base).entryList({prefix + QLatin1Char('*')}, QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &o : old) {
        bool ok = false;
        const qint64 pid = o.mid(prefix.size()).toLongLong(&ok);
        if (ok && pid != QCoreApplication::applicationPid() && !platform::processAlive(pid))
            QDir(QDir(base).filePath(o)).removeRecursively();
    }
    const QString made = QDir(base).filePath(prefix + QString::number(QCoreApplication::applicationPid()));
    if (!QDir().mkpath(made))
        return QString();
#ifdef Q_OS_UNIX
    {
        const QFileInfo before(made); // (checked before the permissions are set: a link would send that to its target)
        if (!before.isDir() || before.isSymLink() || before.ownerId() != getuid())
            return QString();
    }
#endif
    QFile::setPermissions(made, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    // 0.99.0 (encryption review L2): in the shared temporary folder another user may have made this name first (a folder of
    // theirs, or a link): it is ours only when it is a real folder, owned by this user, for this user alone
#ifdef Q_OS_UNIX
    {
        const QFileInfo fi(made);
        const bool mine = fi.isDir() && !fi.isSymLink() && fi.ownerId() == getuid();
        const bool alone = (fi.permissions() & (QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther
                                                | QFileDevice::WriteOther | QFileDevice::ExeOther)) == 0;
        if (!mine || !alone)
            return QString();
    }
#endif
    dir = made;
    qAddPostRoutine(removePlainCopies);
    return dir;
}

void dropCopiesOf(const QString &folder)
{
    for (auto it = g_copies.begin(); it != g_copies.end();) {
        if (it->folder == folder) {
            if (it.key().startsWith(QLatin1String("tree:")))
                QDir(QFileInfo(it->copy).absolutePath()).removeRecursively();
            else
                QFile::remove(it->copy);
            QDir().rmdir(QFileInfo(it->copy).absolutePath());
            it = g_copies.erase(it);
        } else {
            ++it;
        }
    }
}
} // namespace

QStringList projectPlainNames()
{
    return {crypto::markerName(), QStringLiteral("settings/lock")};
}

void add(const QString &folderIn, const QStringList &recipients, const QStringList &plainNames, bool sealNew)
{
    const QString folder = clean(folderIn);
    QMutexLocker l(&g_lock);
    for (Entry &e : g_folders) {
        if (e.folder == folder) {
            ++e.refs;
            e.recipients = recipients; // the newest registration says it (a conversion that changes direction)
            e.plainNames = plainNames;
            e.sealNew = sealNew;
            return;
        }
    }
    g_folders.append(Entry{folder, recipients, plainNames, sealNew, 1});
    g_count.store(int(g_folders.size()));
}

static QStringList g_lockWhenIdle; // keys to lock when the last encrypted project closes (guarded by g_lock)

static void lockKeyNow(const QString &fingerprint)
{
    std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
    crypto::KeyRing::device().lock(fingerprint);
}

void lockWhenIdle(const QString &fingerprint)
{
    {
        QMutexLocker l(&g_lock);
        if (!g_folders.isEmpty()) {
            if (!g_lockWhenIdle.contains(fingerprint))
                g_lockWhenIdle.append(fingerprint);
            return;
        }
    }
    lockKeyNow(fingerprint);
}

void remove(const QString &folderIn)
{
    const QString folder = clean(folderIn);
    bool gone = false;
    QStringList toLock;
    {
        QMutexLocker l(&g_lock);
        for (qsizetype i = 0; i < g_folders.size(); ++i) {
            if (g_folders.at(i).folder == folder) {
                if (--g_folders[i].refs <= 0) {
                    g_folders.removeAt(i);
                    gone = true;
                }
                break;
            }
        }
        g_count.store(int(g_folders.size()));
        if (g_folders.isEmpty())
            toLock.swap(g_lockWhenIdle);
    }
    if (gone)
        dropCopiesOf(folder);
    for (const QString &fp : std::as_const(toLock))
        lockKeyNow(fp);
}

bool active() { return g_count.load() > 0; }

bool contains(const QString &folder)
{
    if (!active())
        return false;
    const QString c = clean(folder);
    QMutexLocker l(&g_lock);
    for (const Entry &e : std::as_const(g_folders))
        if (e.folder == c)
            return true;
    return false;
}

Place placeOf(const QString &path)
{
    Place p;
    if (!active() || path.isEmpty())
        return p;
    const QString c = clean(path);
    QMutexLocker l(&g_lock);
    for (const Entry &e : std::as_const(g_folders)) {
        if (!c.startsWith(e.folder + QLatin1Char('/')))
            continue;
        // the innermost folder wins (an emergency copy inside a project folder is not a case, but a copy may be)
        if (p.found && p.folder.size() >= e.folder.size())
            continue;
        p.found = true;
        p.folder = e.folder;
        p.recipients = e.recipients;
        p.sealNew = e.sealNew;
        const QString rel = c.mid(e.folder.size() + 1);
        p.plainName = e.plainNames.contains(rel);
    }
    return p;
}

bool seals(const QString &path) { return placeOf(path).seals(); }

bool isSealedData(const QByteArray &head) { return sessionKeyPacket(head); }

bool seal(const QString &path, QByteArray &data, QString *error)
{
    // 0.99.0 (Live Sync): the gate's backstop: a write into a follower's folder that got here without being captured
    if (collab::gate().active() && !collab::gate().sealAllowed(path, error))
        return false;
    if (!active())
        return true;
    const Place p = placeOf(path);
    if (!p.seals())
        return true;
    // the tests' switch: "<a part of the path>" makes the encryption of such a file fail
    const QString fake = QB_TEST_ENV("QB_TEST_SEAL_FAILS");
    if (!fake.isEmpty() && path.contains(fake)) {
        if (error)
            *error = say("%1: the encryption failed (a test's switch)").arg(QDir::toNativeSeparators(path));
        return false;
    }
    crypto::Error e = crypto::Error::None;
    QByteArray out;
    {
        std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
        out = crypto::KeyRing::device().encrypt(data, p.recipients, &e);
    }
    if (e != crypto::Error::None) {
        if (error)
            *error = say("%1 could not be encrypted: %2").arg(QDir::toNativeSeparators(path), crypto::errorText(e));
        return false;
    }
    crypto::wipe(data);
    data = out;
    return true;
}

bool unseal(const QString &path, QByteArray &data, QString *error, crypto::Error *kind)
{
    if (kind)
        *kind = crypto::Error::None;
    if (!active())
        return true;
    if (!sessionKeyPacket(data)) {
        // a plain file (a conversion under way, another program's file): read as it is. 0.99.0 (encryption review L1): in
        // a project whose marker says its files are encrypted and no conversion is under way, a file that does not start
        // like an encrypted one is a damaged one (its first bytes), not plain text that the next save would write over
        if (!data.isEmpty()) {
            const Place plain = placeOf(path);
            if (plain.seals()) {
                const crypto::ProjectSettings cs = crypto::settingsOf(plain.folder);
                if (cs.encryptedProject && cs.converting.isEmpty()) {
                    if (kind)
                        *kind = crypto::Error::Damaged;
                    if (error)
                        *error = crypto::errorText(crypto::Error::Damaged);
                    data.clear();
                    return false;
                }
            }
        }
        return true;
    }
    const Place p = placeOf(path);
    if (!p.reads())
        return true;
    crypto::Error e = crypto::Error::None;
    QByteArray plain;
    {
        std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
        plain = crypto::KeyRing::device().decrypt(data, &e, qint64(4) << 30);
    }
    if (e != crypto::Error::None) {
        if (kind)
            *kind = e;
        if (error)
            *error = crypto::errorText(e);
        data.clear();
        return false;
    }
    data = plain;
    return true;
}

bool readFile(const QString &path, QByteArray *data, QString *error, crypto::Error *kind)
{
    if (kind)
        *kind = crypto::Error::None;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = f.errorString();
        if (kind)
            *kind = crypto::Error::Io;
        return false;
    }
    QByteArray d = f.readAll();
    if (f.error() != QFileDevice::NoError || (QFileInfo(path).isFile() && d.size() != f.size())) {
        if (error)
            *error = f.error() != QFileDevice::NoError ? f.errorString()
                                                        : say("only %1 of %2 bytes could be read").arg(d.size()).arg(f.size());
        if (kind)
            *kind = crypto::Error::Io;
        return false;
    }
    if (!unseal(path, d, error, kind))
        return false;
    *data = d;
    return true;
}

QByteArray readOrEmpty(const QString &path)
{
    QByteArray d;
    if (!readFile(path, &d))
        return QByteArray();
    return d;
}

QImage readImage(const QString &path)
{
    if (!placeOf(path).reads())
        return QImage(path);
    QByteArray d;
    QImage img;
    if (readFile(path, &d))
        img.loadFromData(d);
    return img;
}

bool saveImage(const QImage &image, const QString &path, const char *format)
{
    if (collab::gate().active() && collab::gate().decide(path).verdict != collab::Verdict::Pass) {
        // a follower's folder: the picture is encoded here and captured (collab/Gate.h)
        QByteArray fmt = format ? QByteArray(format) : QFileInfo(path).suffix().toLower().toLatin1();
        if (fmt == "jpeg")
            fmt = "jpg";
        if (fmt.isEmpty())
            fmt = "png";
        QByteArray bytes;
        QBuffer buf(&bytes);
        if (!buf.open(QIODevice::WriteOnly) || !image.save(&buf, fmt.constData()))
            return false;
        return collab::gate().write(path, bytes) == collab::Verdict::Captured;
    }
    if (!seals(path))
        return image.save(path, format);
    QByteArray fmt = format ? QByteArray(format) : QFileInfo(path).suffix().toLower().toLatin1();
    if (fmt == "jpeg")
        fmt = "jpg";
    if (fmt.isEmpty())
        fmt = "png";
    QByteArray bytes;
    QBuffer buf(&bytes);
    if (!buf.open(QIODevice::WriteOnly) || !image.save(&buf, fmt.constData()))
        return false;
    return safesave::writeFile(path, bytes);
}

bool copy(const QString &from, const QString &to, QString *error, bool *handled)
{
    *handled = false;
    if (collab::gate().active() && !collab::gate().sealAllowed(to, error)) {
        *handled = true; // (the gate's backstop: safesave::copyFile asks the gate first)
        return false;
    }
    if (!active())
        return false;
    const Place a = placeOf(from), b = placeOf(to);
    if (!a.reads() && !b.seals())
        return false;
    QFile in(from);
    QByteArray head;
    if (in.open(QIODevice::ReadOnly))
        head = in.read(64);
    in.close();
    const bool sourceSealed = a.reads() && sessionKeyPacket(head);
    const bool targetSealed = b.seals();
    QStringList ra = a.recipients, rb = b.recipients;
    ra.sort();
    rb.sort();
    if (sourceSealed == targetSealed && (!sourceSealed || ra == rb))
        return false; // the bytes as they are
    *handled = true;
    std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
    crypto::KeyRing &ring = crypto::KeyRing::device();
    crypto::Error e;
    if (!sourceSealed) {
        e = ring.encryptFile(from, to, b.recipients);
    } else if (!targetSealed) {
        e = ring.decryptFile(from, to);
    } else {
        // to other keys: decrypted into the private folder, encrypted from there
        const QString dir = privateDir();
        const QString mid = dir + QStringLiteral("/recrypt-") + QString::number(qHash(from), 16);
        e = dir.isEmpty() ? crypto::Error::Io : ring.decryptFile(from, mid);
        if (e == crypto::Error::None)
            e = ring.encryptFile(mid, to, b.recipients);
        QFile::remove(mid);
    }
    if (e != crypto::Error::None) {
        if (error)
            *error = QStringLiteral("%1: %2").arg(QDir::toNativeSeparators(to), crypto::errorText(e));
        return false;
    }
    return true;
}

QString plainPath(const QString &path, QString *error)
{
    if (!active() || path.isEmpty())
        return path;
    const Place p = placeOf(path);
    if (!p.reads())
        return path;
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly))
        return path; // not there: the viewer says so as for any file
    const QByteArray head = in.read(64);
    in.close();
    if (!sessionKeyPacket(head))
        return path;
    const QString key = clean(path);
    const syncsafe::Stamp now = syncsafe::stampOf(path);
    auto it = g_copies.find(key);
    if (it != g_copies.end() && it->stamp.sameAs(now) && QFileInfo::exists(it->copy))
        return it->copy;
    const QString dir = privateDir();
    if (dir.isEmpty()) {
        if (error)
            *error = say("There is no private folder for the decrypted copy.");
        return QString();
    }
    // a folder per original, so that the copy keeps the original's file name (viewers go by its type)
    const QString sub = dir + QLatin1Char('/')
                        + QString::fromLatin1(QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex().left(16));
    QDir().mkpath(sub);
    const QString copy = sub + QLatin1Char('/') + QFileInfo(path).fileName();
    crypto::Error e;
    {
        std::lock_guard<std::recursive_mutex> g(crypto::deviceRingLock());
        e = crypto::KeyRing::device().decryptFile(path, copy);
    }
    if (e != crypto::Error::None) {
        if (error)
            *error = crypto::errorText(e);
        return QString();
    }
    QFile::setPermissions(copy, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    g_copies.insert(key, PlainCopy{copy, p.folder, now});
    return copy;
}

QString privateFolder() { return privateDir(); }

QString plainTree(const QString &path, QString *error)
{
    const Place p = placeOf(path);
    if (!p.reads())
        return path;
    const QString dir = privateDir();
    if (dir.isEmpty()) {
        if (error)
            *error = say("There is no private folder for the decrypted copy.");
        return QString();
    }
    const QString from = QFileInfo(path).absolutePath();
    const QString to = dir + QStringLiteral("/tree-")
                       + QString::fromLatin1(QCryptographicHash::hash(clean(from).toUtf8(), QCryptographicHash::Sha1).toHex().left(16));
    QDir(to).removeRecursively();
    QDirIterator it(from, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    bool ok = true;
    while (it.hasNext()) {
        const QString f = it.next();
        const QString target = to + QLatin1Char('/') + QDir(from).relativeFilePath(f);
        QDir().mkpath(QFileInfo(target).absolutePath());
        QString why;
        if (!safesave::copyFile(f, target, &why) && clean(f) == clean(path)) {
            ok = false;
            if (error)
                *error = why;
        }
    }
    if (!ok)
        return QString();
    // removed with the project's folder, as the other copies
    g_copies.insert(QStringLiteral("tree:") + clean(from), PlainCopy{to + QStringLiteral("/.tree"), p.folder, syncsafe::Stamp()});
    return to + QLatin1Char('/') + QFileInfo(path).fileName();
}

void removePlainCopies()
{
    for (const PlainCopy &c : std::as_const(g_copies))
        QFile::remove(c.copy);
    g_copies.clear();
    const QString base = qEnvironmentVariable("XDG_RUNTIME_DIR");
    const QString dir = (base.isEmpty() || !QFileInfo(base).isDir() ? QDir::tempPath() : base) + QStringLiteral("/quillbinder-plain-")
                        + QString::number(QCoreApplication::applicationPid());
    if (QFileInfo(dir).isDir())
        QDir(dir).removeRecursively();
}

Scope::Scope(const QString &projectFolder) : m_folder(projectFolder)
{
    const crypto::ProjectSettings s = crypto::settingsOf(projectFolder);
    if (!s.isEncryptedFolder())
        return;
    add(projectFolder, s.recipients.list(), projectPlainNames(), s.converting != QLatin1String("decrypt"));
    m_added = true;
}

Scope::~Scope()
{
    if (m_added)
        remove(m_folder);
}

} // namespace vault
} // namespace qb
