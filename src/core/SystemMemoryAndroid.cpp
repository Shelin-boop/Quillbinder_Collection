// Android: ActivityManager.MemoryInfo through QJniObject (see SystemMemory.h). Compiled on Android only.
// There is no signal to watch here: since Android 14 only TRIM_MEMORY_UI_HIDDEN and TRIM_MEMORY_BACKGROUND still
// arrive, and the figures are asked for before each load. The Java side may call LowMemoryMonitor::injectLowMemory().
#include "core/SystemMemory.h"

#include <QCoreApplication>
#include <QJniObject>
#include <QtCore/qnativeinterface.h>

namespace qb {
namespace sysmem {

Figures platformRam()
{
    Figures f;
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid())
        return f;
    const QJniObject service = QJniObject::getStaticObjectField("android/content/Context", "ACTIVITY_SERVICE", "Ljava/lang/String;");
    QJniObject am = context.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", service.object<jstring>());
    if (!am.isValid())
        return f;
    QJniObject info("android/app/ActivityManager$MemoryInfo");
    if (!info.isValid())
        return f;
    am.callMethod<void>("getMemoryInfo", "(Landroid/app/ActivityManager$MemoryInfo;)V", info.object());
    const qint64 total = qint64(info.getField<jlong>("totalMem"));
    const qint64 avail = qint64(info.getField<jlong>("availMem"));
    if (total > 0 && avail >= 0) {
        f.total = total;
        f.available = qMin(avail, total);
    }
    return f;
}

bool LowMemoryMonitor::platformStart()
{
    m_method.clear();
    return false;   // no system signal to watch (see above)
}

void LowMemoryMonitor::platformStop() {}

} // namespace sysmem
} // namespace qb
