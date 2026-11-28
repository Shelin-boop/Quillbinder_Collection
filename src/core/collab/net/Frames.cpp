// Live Sync's frames: see Frames.h.
#include "core/collab/net/Frames.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QtEndian>

namespace qb {
namespace collab {
namespace net {

Frame Frame::json(const QJsonObject &o)
{
    Frame f;
    f.kind = Json;
    f.payload = QJsonDocument(o).toJson(QJsonDocument::Compact);
    return f;
}

Frame Frame::data(const QByteArray &transfer16, quint64 offset, const QByteArray &bytes)
{
    Frame f;
    f.kind = Data;
    QByteArray id = transfer16.left(16);
    id.resize(16);
    char off[8];
    qToBigEndian<quint64>(offset, off);
    f.payload = id + QByteArray(off, 8) + bytes;
    return f;
}

QJsonObject Frame::object(bool *ok) const
{
    QJsonParseError e;
    const QJsonDocument d = kind == Json ? QJsonDocument::fromJson(payload, &e) : QJsonDocument();
    const bool good = kind == Json && e.error == QJsonParseError::NoError && d.isObject();
    if (ok)
        *ok = good;
    return good ? d.object() : QJsonObject();
}

bool Frame::dataParts(QByteArray *transfer16, quint64 *offset, QByteArray *bytes) const
{
    if (kind != Data || payload.size() < limits::kDataHeader)
        return false;
    if (transfer16)
        *transfer16 = payload.left(16);
    if (offset)
        *offset = qFromBigEndian<quint64>(payload.constData() + 16);
    if (bytes)
        *bytes = payload.mid(limits::kDataHeader);
    return true;
}

QString frameErrorCode(FrameError e)
{
    switch (e) {
    case FrameError::BadFrame: return QStringLiteral("BAD_FRAME");
    case FrameError::TooBig: return QStringLiteral("TOO_BIG");
    case FrameError::RateLimit: return QStringLiteral("RATE_LIMIT");
    case FrameError::None: break;
    }
    return QString();
}

QByteArray encodeFrame(const Frame &f)
{
    const qint64 n = f.payload.size();
    if (f.kind == Frame::Json) {
        if (n > limits::kMaxJson)
            return QByteArray();
    } else if (f.kind == Frame::Data) {
        if (n < limits::kDataHeader || n - limits::kDataHeader > limits::kMaxDataBytes)
            return QByteArray();
    } else {
        return QByteArray();
    }
    char head[5];
    qToBigEndian<quint32>(quint32(n + 1), head);
    head[4] = char(f.kind);
    return QByteArray(head, 5) + f.payload;
}

FrameReader::FrameReader(std::function<qint64()> nowMs) : clock(std::move(nowMs))
{
    if (!clock)
        clock = [] { return QDateTime::currentMSecsSinceEpoch(); };
}

bool FrameReader::allowControl()
{
    const qint64 now = clock();
    while (!stamps.isEmpty() && now - stamps.first() >= 1000)
        stamps.removeFirst();
    stamps.append(now);
    return stamps.size() <= limits::kMaxControlPerSecond;
}

QList<Frame> FrameReader::feed(const QByteArray &bytes)
{
    QList<Frame> out;
    if (err != FrameError::None)
        return out;
    buffer.append(bytes);
    qsizetype pos = 0;
    while (buffer.size() - pos >= 5) {
        const quint32 len = qFromBigEndian<quint32>(buffer.constData() + pos);
        const quint8 kind = quint8(buffer.at(pos + 4));
        // the length is judged from its first bytes on: a frame announced too big is refused before it is read
        if (len < 1) {
            err = FrameError::BadFrame;
            break;
        }
        if (kind == Frame::Json) {
            if (len - 1 > quint64(limits::kMaxJson)) {
                err = FrameError::TooBig;
                break;
            }
        } else if (kind == Frame::Data) {
            if (len - 1 > quint64(limits::kMaxDataBytes + limits::kDataHeader)) {
                err = FrameError::TooBig;
                break;
            }
            if (len - 1 < quint64(limits::kDataHeader)) {
                err = FrameError::BadFrame;
                break;
            }
        } else {
            err = FrameError::BadFrame;
            break;
        }
        if (buffer.size() - pos < qsizetype(4 + len))
            break; // not complete yet
        Frame f;
        f.kind = kind;
        f.payload = buffer.mid(pos + 5, qsizetype(len - 1));
        pos += 4 + len;
        if (kind == Frame::Json) {
            bool ok = false;
            f.object(&ok);
            if (!ok) {
                err = FrameError::BadFrame; // not a JSON object
                break;
            }
            if (!allowControl()) {
                err = FrameError::RateLimit;
                break;
            }
        }
        out.append(f);
    }
    if (err != FrameError::None)
        buffer.clear();
    else
        buffer.remove(0, pos);
    return out;
}

} // namespace net
} // namespace collab
} // namespace qb
