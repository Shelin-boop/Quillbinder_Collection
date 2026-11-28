#pragma once
// Live Sync, the service's own work in a process that also has windows (0.99.0). A ScopedLink wraps a Link (or a Clock) and
// delivers what comes from it inside a ServiceScope (collab/Gate.h): whatever the sessions write into a shared folder as a
// result (a replica's files, the host's commit) is the service's, never captured as a window's write. Nothing else changes: frames
// and closing pass through unchanged.
#include "core/collab/Clock.h"
#include "core/collab/net/Link.h"

#include <QPointer>

namespace qb {
namespace collab {

class ScopedLink : public net::Link {
    Q_OBJECT
public:
    // `inner` is not owned (it stays the owner of its socket); the wrapper is a child of `parent` (default: of inner)
    explicit ScopedLink(net::Link *inner, QObject *parent = nullptr);
    static ScopedLink *wrap(net::Link *inner) { return new ScopedLink(inner, inner); }

    State state() const override { return m_inner ? m_inner->state() : State::Closed; }
    net::PeerInfo peer() const override { return m_inner ? m_inner->peer() : net::PeerInfo(); }
    bool send(const net::Frame &frame) override { return m_inner && m_inner->send(frame); }
    qint64 bytesWaiting() const override { return m_inner ? m_inner->bytesWaiting() : 0; }
    void close(const QString &c = QString()) override
    {
        if (m_inner)
            m_inner->close(c);
    }
    net::Link *inner() const { return m_inner; }

private:
    QPointer<net::Link> m_inner;
};

class ScopedClock : public Clock {
    Q_OBJECT
public:
    explicit ScopedClock(Clock *inner, QObject *parent = nullptr);
    qint64 now() const override { return m_inner->now(); }
    qint64 wallMs() const override { return m_inner->wallMs(); }

private:
    Clock *m_inner;
};

} // namespace collab
} // namespace qb
