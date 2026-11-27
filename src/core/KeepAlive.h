#pragma once
// 0.99.0: "keep the program running" while a long job runs with the window out of sight (tool interface 1.3,
// ToolHost::keepRunning). Linux and Windows need nothing (KeepAliveNone.cpp does nothing); Android's foreground service
// is started and stopped by KeepAliveAndroid.cpp (a compiling stub in 0.99.0; the Android part completes it).
//
// The reasons are kept here (one per running job); the platform is told only when the first one comes and the last one
// goes.

#include <QString>
#include <QStringList>

namespace qb {
namespace keepalive {

// A job with this reason starts (on) or ends (off). The same reason twice counts once.
void set(const QString &reason, bool on);
bool active();
QStringList reasons();

// The platform's part (KeepAliveNone.cpp or KeepAliveAndroid.cpp): `anyReason` is true from the first reason to the end
// of the last one.
void platformChanged(bool anyReason, const QStringList &reasons);

} // namespace keepalive
} // namespace qb
