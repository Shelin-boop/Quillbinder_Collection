#include "core/collab/ScopedLink.h"

#include "core/collab/Gate.h"

namespace qb {
namespace collab {

ScopedLink::ScopedLink(net::Link *inner, QObject *parent) : net::Link(parent), m_inner(inner)
{
    connect(inner, &net::Link::frameReceived, this, [this](const net::Frame &f) {
        ServiceScope scope;
        emit frameReceived(f);
    });
    connect(inner, &net::Link::stateChanged, this, [this](net::Link::State s) {
        ServiceScope scope;
        emit stateChanged(s);
    });
    connect(inner, &net::Link::closed, this, [this](const QString &c) {
        code = c;
        ServiceScope scope;
        emit closed(c);
    });
}

ScopedClock::ScopedClock(Clock *inner, QObject *parent) : Clock(parent), m_inner(inner)
{
    connect(inner, &Clock::ticked, this, [this] {
        ServiceScope scope;
        emit ticked();
    });
}

} // namespace collab
} // namespace qb
