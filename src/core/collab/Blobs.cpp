#include "core/collab/Blobs.h"

#include "core/SafeSave.h"
#include "core/collab/proto/Messages.h"
#include "core/crypto/Vault.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStorageInfo>

namespace qb {
namespace collab {

using namespace proto;

QString sha256Hex(const QByteArray &bytes) { return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex()); }

bool isHashHex(const QString &s)
{
    static const QRegularExpression re(QStringLiteral("^[0-9a-f]{64}$"));
    return re.match(s).hasMatch();
}

// ---------------------------------------------------------------- BlobStore

QString BlobStore::path(const QString &hash) const { return m_dir + QLatin1Char('/') + hash; }
QString BlobStore::partialPath(const QString &hash) const { return m_dir + QStringLiteral("/partial/") + hash + QStringLiteral(".part"); }
bool BlobStore::has(const QString &hash) const { return isHashHex(hash) && QFileInfo(path(hash)).isFile(); }
qint64 BlobStore::size(const QString &hash) const { return has(hash) ? QFileInfo(path(hash)).size() : -1; }
qint64 BlobStore::partialSize(const QString &hash) const
{
    const QFileInfo fi(partialPath(hash));
    return isHashHex(hash) && fi.isFile() ? fi.size() : 0;
}
void BlobStore::dropPartial(const QString &hash) { safesave::removeFile(partialPath(hash)); }

QString BlobStore::put(const QByteArray &bytes, QString *error)
{
    const QString hash = sha256Hex(bytes);
    if (has(hash))
        return hash;
    QDir().mkpath(m_dir);
    if (!safesave::writeFile(path(hash), bytes, error))
        return QString();
    return hash;
}

bool BlobStore::read(const QString &hash, QByteArray *bytes, QString *error) const
{
    if (!isHashHex(hash)) {
        if (error)
            *error = QStringLiteral("not a hash");
        return false;
    }
    return vault::readFile(path(hash), bytes, error);
}

bool BlobStore::remove(const QString &hash) { return isHashHex(hash) && safesave::removeFile(path(hash)); }

QStringList BlobStore::hashes() const
{
    QStringList out;
    for (const QString &n : QDir(m_dir).entryList(QDir::Files))
        if (isHashHex(n))
            out << n;
    return out;
}

// ---------------------------------------------------------------- BlobHub

BlobHub::BlobHub(net::Link *link, BlobStore *store, QObject *parent) : QObject(parent), m_link(link), m_store(store)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(10);
    connect(&m_timer, &QTimer::timeout, this, &BlobHub::pump);
    m_free = [store] {
        QDir().mkpath(store->dir());
        return QStorageInfo(store->dir()).bytesAvailable();
    };
}

bool BlobHub::isReceiving(const QString &hash) const
{
    for (const In &in : m_in)
        if (in.hash == hash)
            return true;
    return false;
}

bool BlobHub::handleMessage(const QJsonObject &m)
{
    const QString type = m.value(QStringLiteral("t")).toString();
    if (is(type, T::BLOB_PUT)) {
        const QString transfer = m.value(QStringLiteral("transfer")).toString();
        const QString hash = m.value(QStringLiteral("hash")).toString();
        const qint64 size = qint64(m.value(QStringLiteral("size")).toDouble());
        const qint64 offset = qint64(m.value(QStringLiteral("offset")).toDouble());
        const QString re = m.value(QStringLiteral("id")).toString();
        auto refuse = [&](const char *code) {
            m_link->sendJson(errorMessage(QLatin1String(code), re, QString(), m.value(QStringLiteral("p")).toString()));
            emit failed(hash, QLatin1String(code));
            return true;
        };
        if (size > kMaxBlobBytes)
            return refuse("TOO_BIG");
        if (m_in.size() >= 64)
            return refuse("HOST_BUSY");
        if (m_store->has(hash)) { // already here: take the data frames and drop them, finish at BLOB_DONE
            m_in.insert(transfer, In{hash, size, size});
            return true;
        }
        if (m_free() < size - m_store->partialSize(hash) + kSpareBytes)
            return refuse("NO_SPACE");
        QDir().mkpath(m_store->dir() + QStringLiteral("/partial"));
        const qint64 have = m_store->partialSize(hash);
        if (offset != have) { // not where the partial file ends: start again if the sender starts at 0, else refuse
            if (offset != 0)
                return refuse("BAD_FRAME");
            m_store->dropPartial(hash);
        }
        if (offset == 0 && size == 0) {
            // an empty blob has no data frames; BLOB_DONE finishes it
        }
        m_in.insert(transfer, In{hash, size, offset});
        return true;
    }
    if (is(type, T::BLOB_DONE)) {
        const QString transfer = m.value(QStringLiteral("transfer")).toString();
        if (!m_in.contains(transfer))
            return true;
        const In in = m_in.take(transfer);
        finishIn(transfer, in);
        return true;
    }
    return false;
}

bool BlobHub::handleData(const net::Frame &frame)
{
    QByteArray t16, bytes;
    quint64 offset = 0;
    if (!frame.dataParts(&t16, &offset, &bytes))
        return false;
    const QString transfer = QString::fromLatin1(t16.toHex());
    auto it = m_in.find(transfer);
    if (it == m_in.end())
        return false;
    if (m_store->has(it->hash)) { // already here
        it->have += bytes.size();
        return true;
    }
    if (qint64(offset) != it->have || it->have + bytes.size() > it->size) {
        failIn(transfer, QStringLiteral("BAD_FRAME"));
        return true;
    }
    QFile f(m_store->partialPath(it->hash));
    if (!f.open(QIODevice::Append) || f.write(bytes) != bytes.size()) {
        failIn(transfer, QStringLiteral("STORAGE_FAILED"));
        return true;
    }
    it->have += bytes.size();
    return true;
}

void BlobHub::failIn(const QString &transfer, const QString &code)
{
    const In in = m_in.take(transfer);
    m_link->sendJson(errorMessage(code, QString(), QString()));
    if (code != QLatin1String("STORAGE_FAILED"))
        m_store->dropPartial(in.hash);
    emit failed(in.hash, code);
}

void BlobHub::finishIn(const QString &, const In &in)
{
    if (m_store->has(in.hash)) {
        emit received(in.hash);
        return;
    }
    const QString part = m_store->partialPath(in.hash);
    if (in.have != in.size || QFileInfo(part).size() != in.size) {
        m_store->dropPartial(in.hash);
        m_link->sendJson(errorMessage(QStringLiteral("BAD_FRAME"), QString(), QStringLiteral("the blob is not whole")));
        emit failed(in.hash, QStringLiteral("BAD_FRAME"));
        return;
    }
    // the hash over the whole file, in pieces
    QFile f(part);
    QCryptographicHash h(QCryptographicHash::Sha256);
    if (in.size > 0) {
        if (!f.open(QIODevice::ReadOnly)) {
            emit failed(in.hash, QStringLiteral("STORAGE_FAILED"));
            return;
        }
        while (!f.atEnd())
            h.addData(f.read(1 << 20));
        f.close();
    }
    if (QString::fromLatin1(h.result().toHex()) != in.hash) {
        m_store->dropPartial(in.hash);
        m_link->sendJson(errorMessage(QStringLiteral("HASH_MISMATCH"), QString(), QString()));
        emit failed(in.hash, QStringLiteral("HASH_MISMATCH"));
        return;
    }
    bool ok = false;
    if (in.size == 0) {
        ok = !m_store->put(QByteArray()).isEmpty();
    } else if (!vault::seals(m_store->path(in.hash))) {
        QDir().mkpath(m_store->dir());
        ok = QFile::rename(part, m_store->path(in.hash));
    } else {
        QByteArray all;
        QFile r(part);
        if (r.open(QIODevice::ReadOnly)) {
            all = r.readAll();
            r.close();
            ok = !m_store->put(all).isEmpty();
            if (ok)
                m_store->dropPartial(in.hash);
        }
    }
    if (!ok) {
        emit failed(in.hash, QStringLiteral("STORAGE_FAILED"));
        return;
    }
    emit received(in.hash);
}

QString BlobHub::request(const QString &hash)
{
    const QJsonObject m = make(QLatin1String(T::BLOB_GET), QString(),
                               QJsonObject{{QStringLiteral("hash"), hash}, {QStringLiteral("offset"), double(m_store->partialSize(hash))}});
    const QString id = m.value(QStringLiteral("id")).toString();
    m_requests.insert(id, hash);
    m_link->sendJson(m);
    return id;
}

void BlobHub::requestFailed(const QString &re, const QString &code)
{
    const auto it = m_requests.find(re);
    if (it == m_requests.end())
        return;
    const QString hash = it.value();
    m_requests.erase(it);
    emit failed(hash, code);
}

bool BlobHub::send(const QString &hash, const QString &re, const QString &sourcePath, qint64 offset)
{
    Out o;
    o.hash = hash;
    o.transfer = newTransferId();
    if (!sourcePath.isEmpty()) {
        const QFileInfo fi(sourcePath);
        if (!fi.isFile())
            return false;
        o.sourcePath = sourcePath;
        o.size = fi.size();
    } else {
        if (!m_store->has(hash))
            return false;
        if (vault::seals(m_store->path(hash))) {
            if (!m_store->read(hash, &o.bytes))
                return false;
            o.fromMemory = true;
            o.size = o.bytes.size();
        } else {
            o.sourcePath = m_store->path(hash);
            o.size = QFileInfo(o.sourcePath).size();
        }
    }
    if (offset < 0 || offset > o.size)
        offset = 0;
    o.offset = offset;
    QJsonObject put = make(QLatin1String(T::BLOB_PUT), QString(),
                           QJsonObject{{QStringLiteral("transfer"), o.transfer},
                                       {QStringLiteral("hash"), hash},
                                       {QStringLiteral("size"), double(o.size)},
                                       {QStringLiteral("offset"), double(offset)}});
    if (!re.isEmpty())
        put.insert(QStringLiteral("re"), re);
    if (!m_link->sendJson(put))
        return false;
    m_out << o;
    pump();
    return true;
}

void BlobHub::pump()
{
    while (!m_out.isEmpty()) {
        Out &o = m_out.first();
        bool stalled = false;
        QFile f;
        if (!o.fromMemory && o.size > 0 && o.offset < o.size) {
            f.setFileName(o.sourcePath);
            if (!f.open(QIODevice::ReadOnly) || !f.seek(o.offset)) {
                m_link->sendJson(errorMessage(QStringLiteral("STORAGE_FAILED"), QString(), QStringLiteral("a blob cannot be read")));
                m_out.removeFirst();
                continue;
            }
        }
        while (o.offset < o.size) {
            if (m_link->state() != net::Link::State::Ready)
                return; // the link is gone: the sender asks again after reconnecting
            if (m_link->bytesWaiting() > kSendWindowBytes) {
                stalled = true;
                break;
            }
            const qint64 n = qMin<qint64>(net::limits::kMaxDataBytes, o.size - o.offset);
            const QByteArray chunk = o.fromMemory ? o.bytes.mid(int(o.offset), int(n)) : f.read(n);
            if (chunk.size() != n) {
                m_link->sendJson(errorMessage(QStringLiteral("STORAGE_FAILED"), QString(), QStringLiteral("a blob cannot be read")));
                o.offset = o.size + 1;
                break;
            }
            m_link->sendData(transferBytes(o.transfer), quint64(o.offset), chunk);
            o.offset += n;
        }
        if (stalled) {
            m_timer.start();
            return;
        }
        if (o.offset == o.size)
            m_link->sendJson(make(QLatin1String(T::BLOB_DONE), QString(), QJsonObject{{QStringLiteral("transfer"), o.transfer}}));
        m_out.removeFirst();
    }
    emit idle();
}

} // namespace collab
} // namespace qb
