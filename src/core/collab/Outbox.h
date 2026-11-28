#pragma once
// Live Sync's outbox (0.99.0), section 3.4: a follower's change requests, kept on disk before they are sent. An entry is the
// list of units of one save and the plain bytes it refers to (blobs by hash); it stays until the host's result says it is in
// the project (or was rejected), so a crash of the window, the service or the connection loses nothing. An entry that cannot be
// sent any more (the lock was taken over, the host forgot the operation) moves to the drafts, where it stays until someone
// decides. Files are written through safesave, so an encrypted project's outbox is sealed like the project's files.
//   <root>/outbox/<order>-<opId>.json   <root>/drafts/<opId>.json   <root>/blobs/<hash>
#include "core/collab/Blobs.h"

#include <QHash>
#include <QJsonArray>
#include <QList>
#include <QString>

namespace qb {
namespace collab {

struct OutboxEntry {
    qint64 order = 0;
    QString opId;
    QJsonArray units;
    QStringList blobs;   // the hashes the units refer to
    qint64 createdMs = 0;
    QString reason;      // drafts: why it was not sent
};

class Outbox {
public:
    explicit Outbox(const QString &root);
    // reads the folders; entries that are damaged are moved to the drafts with a reason (never dropped)
    bool load(QString *error = nullptr);
    BlobStore &blobs() { return m_blobs; }
    const BlobStore &blobs() const { return m_blobs; }

    // keeps a new entry (the bytes first, then the entry) and returns its operation id; empty with *error when it could not be kept
    QString add(const QJsonArray &units, const QHash<QString, QByteArray> &blobsByHash, qint64 createdMs, QString *error = nullptr,
                const QString &opId = QString());
    QList<OutboxEntry> pending() const { return m_pending; }
    const OutboxEntry *find(const QString &opId) const;
    // the entry's units replaced (a new base revision after an earlier commit of its own)
    bool updateUnits(const QString &opId, const QJsonArray &units, QString *error = nullptr);
    // the entry is in the project (or answered for good): removed, and the blobs nothing refers to any more
    bool done(const QString &opId);
    // the entry moves to the drafts with its blobs
    bool toDraft(const QString &opId, const QString &reason, QString *error = nullptr);
    // some entry or draft names the blob
    bool referenced(const QString &hash) const;
    QList<OutboxEntry> drafts() const { return m_drafts; }
    bool removeDraft(const QString &opId);

private:
    bool write(const QString &path, const OutboxEntry &e, QString *error);
    void collectBlobs();
    QString m_root;
    BlobStore m_blobs;
    QList<OutboxEntry> m_pending, m_drafts;
    QStringList m_released; // blobs of entries that left, to be removed when nothing refers to them
    qint64 m_nextOrder = 1;
};

} // namespace collab
} // namespace qb
