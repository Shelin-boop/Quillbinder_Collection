// A fake network for the tests: see MemLink.h.
#include "core/collab/net/MemLink.h"

#include <QCryptographicHash>
#include <QPointer>
#include <QTimer>

namespace qb {
namespace collab {
namespace net {

QString MemLink::deviceIdOfPin(const QByteArray &pin) { return QString::fromLatin1(pin.left(16).toHex()); }

MemLink::Pair MemLink::createPair(const QByteArray &pinOfA, const QByteArray &pinOfB, QObject *parent)
{
    qRegisterMetaType<qb::collab::net::Frame>();
    qRegisterMetaType<qb::collab::net::Link::State>();
    auto *a = new MemLink(parent);
    auto *b = new MemLink(parent);
    a->other = b;
    b->other = a;
    // a sees b's pin as its peer, and the other way round
    a->info.pin = pinOfB;
    a->info.deviceId = deviceIdOfPin(pinOfB);
    a->info.trust = PeerInfo::Trust::Paired;
    b->info.pin = pinOfA;
    b->info.deviceId = deviceIdOfPin(pinOfA);
    b->info.trust = PeerInfo::Trust::Paired;
    a->info.address = b->info.address = QStringLiteral("memory");
    return {a, b};
}

bool MemLink::send(const Frame &frame)
{
    if (st != State::Ready || !other)
        return false;
    const QByteArray bytes = encodeFrame(frame);
    if (bytes.isEmpty())
        return false;
    ++sent;
    if (dropping)
        return true;
    if (holding) {
        held.append(bytes);
        return true;
    }
    deliver(bytes);
    return true;
}

void MemLink::deliver(const QByteArray &bytes)
{
    MemLink *o = other;
    if (!o)
        return;
    if (synchronous) {
        o->receive(bytes);
        return;
    }
    QPointer<MemLink> guard(o);
    QTimer::singleShot(0, o, [guard, bytes] {
        if (guard)
            guard->receive(bytes);
    });
}

void MemLink::receive(const QByteArray &bytes)
{
    if (st != State::Ready)
        return;
    const QList<Frame> frames = reader.feed(bytes);
    for (const Frame &f : frames) {
        if (st != State::Ready)
            return;
        emit frameReceived(f);
    }
    if (reader.error() != FrameError::None) {
        const QString c = frameErrorCode(reader.error());
        if (other)
            other->gone(QString());
        gone(c);
    }
}

void MemLink::setHeld(bool hold)
{
    holding = hold;
    if (!hold) {
        const QList<QByteArray> list = held;
        held.clear();
        for (const QByteArray &b : list)
            deliver(b);
    }
}

void MemLink::gone(const QString &c)
{
    if (st == State::Closed)
        return;
    st = State::Closed;
    code = c;
    held.clear();
    emit stateChanged(st);
    emit closed(c);
}

void MemLink::close(const QString &c)
{
    MemLink *o = other;
    gone(c);
    if (!o)
        return;
    // as on a wire, what was sent before the close arrives before the other end sees it
    if (synchronous) {
        o->gone(QString());
        return;
    }
    QPointer<MemLink> guard(o);
    QTimer::singleShot(0, o, [guard] {
        if (guard)
            guard->gone(QString());
    });
}

} // namespace net
} // namespace collab
} // namespace qb
