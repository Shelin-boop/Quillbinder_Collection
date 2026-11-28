#pragma once
// A fake network for the tests (0.99.0): two Links that hand each other frames in memory. The frames go through the same
// encoder and strict reader as on the wire, so the limits hold here too. The test can hold the frames back (a partition),
// drop them, or deliver by hand.
#include "core/collab/net/Link.h"

#include <QList>
#include <memory>

namespace qb {
namespace collab {
namespace net {

class MemLink : public Link {
    Q_OBJECT
public:
    struct Pair {
        MemLink *a = nullptr;
        MemLink *b = nullptr;
    };
    // two linked ends; each reports the other's pin (and its id) as peer(), trust Paired unless set
    static Pair createPair(const QByteArray &pinOfA, const QByteArray &pinOfB, QObject *parent = nullptr);
    static QString deviceIdOfPin(const QByteArray &pin);

    State state() const override { return st; }
    PeerInfo peer() const override { return info; }
    bool send(const Frame &frame) override;
    qint64 bytesWaiting() const override { return held.size(); }
    void close(const QString &code = QString()) override;

    void setPeerTrust(PeerInfo::Trust t) { info.trust = t; }
    // frames sent from this end are kept (a partition) until released; released = delivered in order
    void setHeld(bool hold);
    // frames sent from this end vanish
    void setDropping(bool drop) { dropping = drop; }
    // delivers at once instead of through the event loop
    void setSynchronous(bool sync) { synchronous = sync; }
    int sentFrames() const { return sent; }

private:
    explicit MemLink(QObject *parent) : Link(parent) {}
    void deliver(const QByteArray &bytes);
    void receive(const QByteArray &bytes);
    void gone(const QString &code);

    MemLink *other = nullptr;
    PeerInfo info;
    State st = State::Ready;
    bool holding = false;
    bool dropping = false;
    bool synchronous = false;
    int sent = 0;
    QList<QByteArray> held;
    FrameReader reader;
};

} // namespace net
} // namespace collab
} // namespace qb
