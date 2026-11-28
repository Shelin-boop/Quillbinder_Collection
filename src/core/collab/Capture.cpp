#include "core/collab/Capture.h"

#include <QCryptographicHash>
#include <QMutexLocker>

namespace qb {
namespace collab {

const FileChange *ChangeSet::find(const QString &path) const
{
    for (qsizetype i = files.size() - 1; i >= 0; --i)
        if (files.at(i).path == path)
            return &files.at(i);
    return nullptr;
}

QStringList ChangeSet::paths() const
{
    QStringList out;
    for (const FileChange &c : files)
        out << c.path;
    return out;
}

void Capture::apply(const FileChange &c)
{
    // earlier entries this one makes obsolete
    for (qsizetype i = m_open.files.size() - 1; i >= 0; --i) {
        const FileChange &o = m_open.files.at(i);
        const bool same = o.path == c.path;
        const bool below = c.kind == FileChange::RemoveTree && o.path.startsWith(c.path + QLatin1Char('/'));
        if (same || below)
            m_open.files.removeAt(i);
    }
    m_open.files.append(c);
}

void Capture::put(const QString &path, const QByteArray &plain, const PathInfo &info)
{
    FileChange c;
    c.kind = FileChange::Put;
    c.path = path;
    c.data = plain;
    c.sha256 = QCryptographicHash::hash(plain, QCryptographicHash::Sha256).toHex();
    c.info = info;
    QMutexLocker l(&m_lock);
    if (!m_suspended)
        apply(c);
}

void Capture::remove(const QString &path, const PathInfo &info)
{
    FileChange c;
    c.kind = FileChange::Remove;
    c.path = path;
    c.info = info;
    QMutexLocker l(&m_lock);
    if (!m_suspended)
        apply(c);
}

void Capture::removeTree(const QString &path, const PathInfo &info)
{
    FileChange c;
    c.kind = FileChange::RemoveTree;
    c.path = path;
    c.info = info;
    QMutexLocker l(&m_lock);
    if (!m_suspended)
        apply(c);
}

void Capture::add(const QList<FileChange> &files)
{
    QMutexLocker l(&m_lock);
    if (m_suspended)
        return;
    for (const FileChange &c : files)
        apply(c);
}

void Capture::commitBatch(const QList<FileChange> &files)
{
    std::function<void()> listener;
    {
        QMutexLocker l(&m_lock);
        if (m_suspended)
            return;
        for (const FileChange &c : files)
            apply(c);
        if (!m_open.isEmpty()) {
            m_ready.append(m_open);
            m_open = ChangeSet();
            listener = m_listener;
        }
    }
    if (listener)
        listener();
}

void Capture::flush() { commitBatch({}); }

QList<ChangeSet> Capture::takeReady()
{
    QMutexLocker l(&m_lock);
    QList<ChangeSet> out;
    out.swap(m_ready);
    return out;
}

bool Capture::hasReady() const
{
    QMutexLocker l(&m_lock);
    return !m_ready.isEmpty();
}

int Capture::openCount() const
{
    QMutexLocker l(&m_lock);
    return int(m_open.files.size());
}

void Capture::setSuspended(bool on)
{
    QMutexLocker l(&m_lock);
    m_suspended = on;
}

bool Capture::isSuspended() const
{
    QMutexLocker l(&m_lock);
    return m_suspended;
}

void Capture::setListener(std::function<void()> listener)
{
    QMutexLocker l(&m_lock);
    m_listener = std::move(listener);
}

} // namespace collab
} // namespace qb
