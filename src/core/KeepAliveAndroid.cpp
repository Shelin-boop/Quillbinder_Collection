// Android: a compiling stub (0.99.0). The Android part of the run adds the foreground service here: start it when the
// first reason comes (with a notification that names the reasons), stop it when the last one goes.
#include "core/KeepAlive.h"

#include <QDebug>

namespace qb {
namespace keepalive {

void platformChanged(bool anyReason, const QStringList &reasons)
{
    // TODO(Android part): start or stop the foreground service
    qDebug() << "keepalive (stub):" << (anyReason ? "on" : "off") << reasons;
}

} // namespace keepalive
} // namespace qb
