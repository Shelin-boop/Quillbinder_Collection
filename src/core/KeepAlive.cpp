#include "core/KeepAlive.h"

#include <QMutex>
#include <QMutexLocker>

namespace qb {
namespace keepalive {

namespace {
QMutex &mtx() { static QMutex m; return m; }
QStringList &list() { static QStringList l; return l; }
} // namespace

void set(const QString &reason, bool on)
{
    bool changed = false;
    bool any = false;
    QStringList now;
    {
        QMutexLocker l(&mtx());
        QStringList &r = list();
        const bool had = !r.isEmpty();
        if (on && !r.contains(reason))
            r << reason;
        else if (!on)
            r.removeAll(reason);
        any = !r.isEmpty();
        changed = had != any;
        now = r;
    }
    if (changed)
        platformChanged(any, now);
}

bool active()
{
    QMutexLocker l(&mtx());
    return !list().isEmpty();
}

QStringList reasons()
{
    QMutexLocker l(&mtx());
    return list();
}

} // namespace keepalive
} // namespace qb
