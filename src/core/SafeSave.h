#pragma once
// Writing and committing a QSaveFile so that a failed write is never reported as saved (0.64.1, found while the
// fault cases of the system test were written, 2026-11-09): Qt 6.4's QSaveFile::commit() writes the
// last buffered bytes while it closes the file and syncs it to the disk, but it does not look at the
// result of either, so a full disk (ENOSPC) or an I/O error (EIO) there left a cut-off file in place
// of the old one and commit() returned true. Here the buffer is written and checked first, then the
// file is synced (checked), then committed. False: nothing replaced (the temporary file goes when the
// QSaveFile is destroyed).
#include <QFile>
#include <QSaveFile>
#include <QString>
#include <QStringList>

#include "core/TestSwitches.h"
// 0.99.0: the program's own code writes through the vault (core/crypto/Vault.h: an encrypted project's files are
// encrypted). The add-ons' plugins use this file header-only and link no part of the program: their build sets
// QB_SAFESAVE_HEADER_ONLY (plugins/tools, plugins/importers), and they write only into folders of their own.
#ifndef QB_SAFESAVE_HEADER_ONLY
#include "core/crypto/Vault.h"
#define QB_SAFESAVE_VAULT 1
#endif

#ifdef Q_OS_UNIX
#include <cerrno>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <io.h>
#endif

namespace qb {
namespace safesave {

// The bytes as they are (a copy of a file that is already in the form its place needs)
inline qint64 writeRaw(QSaveFile &f, const QByteArray &data)
{
    const qint64 n = f.write(data);
    if (n != data.size())
        f.cancelWriting();
    return n;
}

// Writes data; a short write (a full disk after part of it: Qt reports no error then) cancels the file,
// so the commit below fails and nothing is replaced.
// 0.99.0: a file of an encrypted project (core/crypto/Vault.h) is written encrypted, whole in one call; the size of
// the text is returned when it was written. A failed encryption cancels the file.
inline qint64 write(QSaveFile &f, const QByteArray &data)
{
#ifdef QB_SAFESAVE_VAULT
    if (vault::active() && vault::seals(f.fileName())) {
        QByteArray sealed = data;
        if (!vault::seal(f.fileName(), sealed)) {
            f.cancelWriting();
            return -1;
        }
        return writeRaw(f, sealed) == sealed.size() ? data.size() : -1;
    }
#endif
    return writeRaw(f, data);
}

inline bool commit(QSaveFile &f)
{
    if (!f.isOpen() || f.error() != QFileDevice::NoError)
        return false;
    if (!f.flush() || f.error() != QFileDevice::NoError) {
        f.cancelWriting();
        return false;
    }
    const int fd = f.handle();
#ifdef Q_OS_UNIX
    // Why: docs/design-decisions.md, entry 2.
    // a file system that cannot sync (EINVAL, ENOTSUP on some network and FUSE mounts) is no error
    if (fd >= 0 && ::fsync(fd) != 0 && errno != EINVAL && errno != ENOTSUP && errno != EOPNOTSUPP && errno != ENOSYS) {
        f.cancelWriting();
        return false;
    }
    // Qt closes the temporary file only after it has replaced the old one, and never looks at the
    // result; a file system that reports write errors at close (NFS and other network file systems)
    // reports them for a copy of the descriptor too, so a copy is closed here first
    if (fd >= 0) {
        const int copy = ::dup(fd);
        if (copy >= 0 && ::close(copy) != 0 && errno != EINTR) {
            f.cancelWriting();
            return false;
        }
    }
#elif defined(Q_OS_WIN)
    if (fd >= 0 && ::_commit(fd) != 0) {
        f.cancelWriting();
        return false;
    }
#endif
    const bool done = f.commit();
    return done && f.error() == QFileDevice::NoError;
}

// A whole file written and committed as above (0.65.0): false with a reason, and nothing replaced.
inline bool writeFile(const QString &path, const QByteArray &data, QString *error = nullptr)
{
    // the tests' switch of savebatch (0.92.0), also here (0.93.0, the emergency copy): "<number>|<text>|<a part of the
    // path>" makes the write of such a file fail with that text
    const QStringList fake = QB_TEST_ENV("QB_TEST_WRITE_FAILS").split(QLatin1Char('|'));
    if (fake.size() == 3 && !fake.at(2).isEmpty() && path.contains(fake.at(2))) {
        if (error)
            *error = QStringLiteral("%1: %2").arg(path, fake.at(1));
        return false;
    }
    // 0.99.0: a file of an encrypted project is written encrypted (core/crypto/Vault.h)
    QByteArray sealed = data;
#ifdef QB_SAFESAVE_VAULT
    QString sealError;
    if (!vault::seal(path, sealed, &sealError)) {
        if (error)
            *error = sealError;
        return false;
    }
#endif
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || writeRaw(f, sealed) != sealed.size() || !commit(f)) {
        if (error)
            *error = QStringLiteral("%1: %2").arg(path,
                                                 f.errorString().isEmpty() || f.errorString() == QLatin1String("Unknown error")
                                                     ? QStringLiteral("not written")
                                                     : f.errorString());
        return false;
    }
    return true;
}

// A copy that is either the old target or the whole new file, never neither (0.65.0, audit F05/F11/F25):
// read in pieces into a temporary file beside the target, synced, then renamed over it. The old code
// removed the target first and then copied, so a failed copy left no file at all.
inline bool copyFile(const QString &from, const QString &to, QString *error = nullptr)
{
#ifdef QB_SAFESAVE_VAULT
    // 0.99.0: into or out of an encrypted project, the copy is encrypted or decrypted as its place needs
    {
        bool handled = false;
        const bool ok = vault::copy(from, to, error, &handled);
        if (handled)
            return ok;
    }
#endif
    QFile in(from);
    if (!in.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("%1: %2").arg(from, in.errorString());
        return false;
    }
    QSaveFile out(to);
    if (!out.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("%1: %2").arg(to, out.errorString());
        return false;
    }
    qint64 copied = 0;
    // read until nothing comes (not "until the size": a file whose size says less than it holds is noticed below)
    for (;;) {
        const QByteArray piece = in.read(4 << 20);
        if (piece.isEmpty())
            break;
        if (writeRaw(out, piece) != piece.size())
            break;
        copied += piece.size();
    }
    if (in.error() != QFileDevice::NoError || copied != in.size() || !commit(out)) {
        if (error)
            *error = QStringLiteral("%1: %2").arg(to, in.error() != QFileDevice::NoError ? in.errorString()
                                                      : out.errorString().isEmpty()      ? QStringLiteral("not written")
                                                                                         : out.errorString());
        out.cancelWriting();
        return false;
    }
    return true;
}

// 0.92.0: the copy for imports and for reading a Scrivener project again: copyFile above, whose failure and reason
// the caller tells (before, QFile::copy was called there without a look at its result, so a file that did not
// arrive was counted as imported, or was lost with the old working copy). The tests can make one copy fail:
// QB_TEST_FAIL_COPY holds a part of the source's or the target's path.
inline bool copyChecked(const QString &from, const QString &to, QString *error = nullptr)
{
    const QString refused = QB_TEST_ENV("QB_TEST_FAIL_COPY");
    if (!refused.isEmpty() && (from.contains(refused) || to.contains(refused))) {
        if (error)
            *error = QStringLiteral("%1: the copy failed (a test's switch)").arg(to);
        return false;
    }
    return copyFile(from, to, error);
}

} // namespace safesave
} // namespace qb
