#pragma once
// The boundary between Live Sync's protocol and the network (0.99.0): a Link carries frames to one peer. TlsLink is the
// real one (TLS 1.3 over a TCP socket); MemLink is a fake network for the tests; a transport over the internet would be
// one more Link. Nothing above this boundary knows what carries the frames.
#include "core/collab/net/Frames.h"

#include <QObject>
#include <QString>

namespace qb {
namespace collab {
namespace net {

// what a link knows of the other end
struct PeerInfo {
    enum class Trust {
        None,    // not known yet (the handshake is not done)
        Unknown, // a key we have not paired with and do not take now (the link says NOT_PAIRED and closes)
        Pairing, // a key we have not paired with, taken only for a pairing exchange
        Paired,  // the key is in the peer store
        Member,  // the key is in the signed membership of the project asked for
    };
    QByteArray publicKey; // the peer's public key (DER SubjectPublicKeyInfo), for signed records (membership)
    QByteArray pin;      // SHA-256 of the peer's public key (DER SubjectPublicKeyInfo), 32 bytes
    QString deviceId;    // the first 16 bytes of the pin as hex (32 characters); what the peer's HELLO must say
    QString address;     // the peer's network address (for the writer's information only)
    Trust trust = Trust::None;
};

class Link : public QObject {
    Q_OBJECT
public:
    enum class State { Connecting, Handshaking, Ready, Closed };
    explicit Link(QObject *parent = nullptr) : QObject(parent) {}

    virtual State state() const = 0;
    virtual PeerInfo peer() const = 0;
    // false when the link is not ready or the frame is not allowed (too big, unknown kind): nothing is sent then
    virtual bool send(const Frame &frame) = 0;
    // bytes accepted but not yet on the wire (for the protocol's flow control)
    virtual qint64 bytesWaiting() const = 0;
    // closes at once; the code (BAD_FRAME, ...) is what closed() reports
    virtual void close(const QString &code = QString()) = 0;
    // the code the link was closed with ("" for a normal close); valid once state() is Closed
    QString closeCode() const { return code; }

    bool sendJson(const QJsonObject &o) { return send(Frame::json(o)); }
    bool sendData(const QByteArray &transfer16, quint64 offset, const QByteArray &bytes)
    {
        return send(Frame::data(transfer16, offset, bytes));
    }

signals:
    void stateChanged(qb::collab::net::Link::State state);
    // a frame of the peer (only once the link is Ready)
    void frameReceived(const qb::collab::net::Frame &frame);
    void closed(const QString &code);

protected:
    QString code;
};

} // namespace net
} // namespace collab
} // namespace qb

Q_DECLARE_METATYPE(qb::collab::net::Frame)
Q_DECLARE_METATYPE(qb::collab::net::Link::State)
