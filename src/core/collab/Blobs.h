#pragma once
// Live Sync's blobs (0.99.0), section 3.5: file contents travel by their SHA-256, in data frames of at most 256 KiB, checked
// when they are whole, and resumable (a partial file is kept and the next BLOB_GET names the offset).
//   BlobStore  a folder of blobs named by their hash (the host's `livesync/incoming`, a client's `livesync-local/blobs`)
//   BlobHub    the blob traffic of one link: receives BLOB_PUT / data / BLOB_DONE, sends a blob, asks for one
// Limits: a blob is at most 2 GiB; before a transfer starts the free space must cover its size + 64 MiB (NO_SPACE); a sender
// does not queue more than 8 MiB on a link (it waits until the link has sent them).
#include "core/collab/net/Link.h"

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QString>
#include <QTimer>
#include <functional>

namespace qb {
namespace collab {

constexpr qint64 kMaxBlobBytes = qint64(2) * 1024 * 1024 * 1024;
constexpr qint64 kSpareBytes = qint64(64) * 1024 * 1024;
constexpr qint64 kSendWindowBytes = qint64(8) * 1024 * 1024;

QString sha256Hex(const QByteArray &bytes);
bool isHashHex(const QString &s);

class BlobStore {
public:
    explicit BlobStore(const QString &dir) : m_dir(dir) {}
    QString dir() const { return m_dir; }
    QString path(const QString &hash) const;
    QString partialPath(const QString &hash) const;
    bool has(const QString &hash) const;
    qint64 size(const QString &hash) const; // -1 when not there
    // stores `bytes` under their hash (checked); the hash is returned (empty with *error on failure)
    QString put(const QByteArray &bytes, QString *error = nullptr);
    // the bytes of a blob (false with a reason when it is not there or not readable)
    bool read(const QString &hash, QByteArray *bytes, QString *error = nullptr) const;
    bool remove(const QString &hash);
    QStringList hashes() const;
    qint64 partialSize(const QString &hash) const; // 0 when none
    void dropPartial(const QString &hash);

private:
    QString m_dir;
};

class BlobHub : public QObject {
    Q_OBJECT
public:
    BlobHub(net::Link *link, BlobStore *store, QObject *parent = nullptr);
    // the free bytes of the drive that holds the store (a test replaces it)
    void setFreeSpaceProbe(std::function<qint64()> probe) { m_free = std::move(probe); }

    // BLOB_PUT and BLOB_DONE: true when the message was one of them (consumed). A failure is reported by failed().
    bool handleMessage(const QJsonObject &message);
    // a data frame; true when it belonged to a transfer under way
    bool handleData(const net::Frame &frame);

    // Sends a blob: BLOB_PUT {transfer, hash, size, offset}, the data frames and BLOB_DONE. `re` is the id of the request it
    // answers ("" for an unprompted upload). `sourcePath`: a file whose raw bytes are the blob (a stored project file);
    // empty: the store's blob. Returns false (nothing sent) when the blob is not there.
    bool send(const QString &hash, const QString &re, const QString &sourcePath = QString(), qint64 offset = 0);
    // BLOB_GET for `hash`, from the end of its partial file; returns the request's message id
    QString request(const QString &hash);
    // a BLOB_MISSING or other error answered a request: reports failed(hash, code)
    void requestFailed(const QString &re, const QString &code);
    bool isReceiving(const QString &hash) const;
    int receivingCount() const { return m_in.size(); }
    int sendingCount() const { return m_out.size(); }

signals:
    void received(const QString &hash);
    void failed(const QString &hash, const QString &code);
    // everything queued for sending has been sent
    void idle();

private:
    struct In {
        QString hash;
        qint64 size = 0;
        qint64 have = 0;
    };
    struct Out {
        QString transfer, hash, sourcePath;
        QByteArray bytes; // the whole blob when it comes from a sealed store
        bool fromMemory = false;
        qint64 size = 0, offset = 0;
    };
    void pump();
    void finishIn(const QString &transfer, const In &in);
    void failIn(const QString &transfer, const QString &code);
    net::Link *m_link;
    BlobStore *m_store;
    QHash<QString, In> m_in;           // transfer id -> state
    QHash<QString, QString> m_requests; // message id -> hash
    QList<Out> m_out;
    QTimer m_timer;
    std::function<qint64()> m_free;
};

} // namespace collab
} // namespace qb
