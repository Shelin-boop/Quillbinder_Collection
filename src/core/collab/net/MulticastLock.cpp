#include "core/collab/net/MulticastLock.h"

#include <QMutex>
#include <QMutexLocker>

namespace qb {
namespace multicastlock {

namespace {
QMutex &mutex()
{
    static QMutex m;
    return m;
}
QStringList &list()
{
    static QStringList l;
    return l;
}
} // namespace

void acquire(const QString &holder)
{
    QMutexLocker lock(&mutex());
    if (list().contains(holder))
        return;
    const bool first = list().isEmpty();
    list() << holder;
    if (first)
        platformChanged(true);
}

void release(const QString &holder)
{
    QMutexLocker lock(&mutex());
    if (!list().removeOne(holder))
        return;
    if (list().isEmpty())
        platformChanged(false);
}

bool held()
{
    QMutexLocker lock(&mutex());
    return !list().isEmpty();
}

QStringList holders()
{
    QMutexLocker lock(&mutex());
    return list();
}

} // namespace multicastlock
} // namespace qb
