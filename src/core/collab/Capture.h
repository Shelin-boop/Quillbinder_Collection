#pragma once
// Live Sync, what a follower's writes become (0.99.0): the files the program wanted to write into a shared project's
// folder, kept as plain bytes (before any encryption) instead of reaching the disk. A save makes one change set (the
// files of one batch); writes outside a batch join the open set until flush() closes it.
#include "core/collab/PathClass.h"

#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QString>

#include <functional>

namespace qb {
namespace collab {

struct FileChange {
    enum Kind { Put, Remove, RemoveTree };
    Kind kind = Put;
    QString path;       // relative to the project folder, "/" separators
    QByteArray data;    // Put: the plain bytes
    QByteArray sha256;  // Put: hex SHA-256 of data
    PathInfo info;
};

struct ChangeSet {
    QList<FileChange> files;
    bool isEmpty() const { return files.isEmpty(); }
    // the change for `path`, or nullptr
    const FileChange *find(const QString &path) const;
    QStringList paths() const;
};

class Capture
{
public:
    // Put: a later put of the same path replaces the earlier one; a remove cancels an earlier put of the path (and of
    // everything below it for RemoveTree) and is kept itself
    void put(const QString &relativePath, const QByteArray &plain, const PathInfo &info);
    void remove(const QString &relativePath, const PathInfo &info);
    void removeTree(const QString &relativePath, const PathInfo &info);
    // the files of one batch, joined to the open set (not closed: the batch's own follow-up steps may add more)
    void add(const QList<FileChange> &files);
    // the files of one batch, joined to the open set; then the open set is closed
    void commitBatch(const QList<FileChange> &files);
    // closes the open set (when it holds something) and tells the listener
    void flush();
    // the closed sets, oldest first; they are removed from here
    QList<ChangeSet> takeReady();
    bool hasReady() const;
    int openCount() const;
    // While suspended every write is dropped (a remote commit being applied to the window: its files are already there)
    void setSuspended(bool on);
    bool isSuspended() const;
    // runs (in the thread that closed a set) whenever a set is ready
    void setListener(std::function<void()> listener);

private:
    void apply(const FileChange &c);
    mutable QMutex m_lock;
    ChangeSet m_open;
    QList<ChangeSet> m_ready;
    std::function<void()> m_listener;
    bool m_suspended = false;
};

} // namespace collab
} // namespace qb
