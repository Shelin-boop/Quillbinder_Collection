#pragma once
// 0.99.0, Live Sync: Android drops multicast packets unless the program holds the Wi-Fi multicast lock while it
// looks for other devices (permission CHANGE_WIFI_MULTICAST_STATE). Other systems need nothing: MulticastLockNone.cpp
// does nothing. The holders are kept here (one per running discovery); the platform is told only when the first one
// comes and the last one goes.

#include <QString>
#include <QStringList>

namespace qb {
namespace multicastlock {

void acquire(const QString &holder);   // the same holder twice counts once
void release(const QString &holder);
bool held();
QStringList holders();

// The platform's part (MulticastLockNone.cpp or MulticastLockAndroid.cpp).
void platformChanged(bool held);

} // namespace multicastlock
} // namespace qb
