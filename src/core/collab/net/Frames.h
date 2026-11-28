#pragma once
// Live Sync's frames on a link (0.99.0): [u32 length][u8 kind][payload]; the length counts the kind byte and the payload.
//   kind 0: one JSON object (at most 1 MiB)
//   kind 1: data [16-byte transfer id][u64 offset][bytes], at most 256 KiB of bytes
// The reader is strict: anything else closes the link with a code (BAD_FRAME, TOO_BIG), and so does a flood of control
// frames (RATE_LIMIT, more than 200 in one second).
#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <functional>

namespace qb {
namespace collab {
namespace net {

struct Frame {
    enum Kind : quint8 { Json = 0, Data = 1 };
    quint8 kind = Json;
    QByteArray payload; // Json: the JSON text; Data: the whole data payload (transfer id, offset, bytes)

    static Frame json(const QJsonObject &o);
    static Frame data(const QByteArray &transfer16, quint64 offset, const QByteArray &bytes);
    // Json frames: the object (empty and ok = false when the payload is no JSON object)
    QJsonObject object(bool *ok = nullptr) const;
    // Data frames: the three parts (false when the payload is too short)
    bool dataParts(QByteArray *transfer16, quint64 *offset, QByteArray *bytes) const;
};

namespace limits {
constexpr qint64 kMaxJson = 1024 * 1024;
constexpr qint64 kMaxDataBytes = 256 * 1024;
constexpr int kDataHeader = 16 + 8;
constexpr int kMaxControlPerSecond = 200;
} // namespace limits

enum class FrameError { None, BadFrame, TooBig, RateLimit };
// the code a link is closed with
QString frameErrorCode(FrameError e);

// the bytes of a frame (empty when the frame is too big or of an unknown kind: nothing is sent then)
QByteArray encodeFrame(const Frame &f);

class FrameReader {
public:
    // the clock in milliseconds (a fake one in the tests)
    explicit FrameReader(std::function<qint64()> nowMs = nullptr);
    // adds bytes; returns the complete frames; after an error nothing more is read (error() says which)
    QList<Frame> feed(const QByteArray &bytes);
    FrameError error() const { return err; }
    // the bytes kept for a frame not complete yet
    qint64 pending() const { return buffer.size(); }

private:
    bool allowControl();
    std::function<qint64()> clock;
    QByteArray buffer;
    QList<qint64> stamps; // times of the control frames of the last second
    FrameError err = FrameError::None;
};

} // namespace net
} // namespace collab
} // namespace qb
