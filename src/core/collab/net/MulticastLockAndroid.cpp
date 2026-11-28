// Android: WifiManager.MulticastLock through QJniObject. Compiled on Android only (CMakeLists.txt); the manifest needs
// the permission CHANGE_WIFI_MULTICAST_STATE (packaging/android). Not run on a device in 0.99.0's Linux part.
#include "core/collab/net/MulticastLock.h"

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QJniObject>
#include <QCoreApplication>
#include <QDebug>

namespace qb {
namespace multicastlock {

namespace {
QJniObject g_lock;
}

void platformChanged(bool held)
{
    if (held) {
        if (g_lock.isValid())
            return;
        QJniObject context = QNativeInterface::QAndroidApplication::context();
        if (!context.isValid())
            return;
        const QJniObject service = QJniObject::getStaticObjectField("android/content/Context", "WIFI_SERVICE", "Ljava/lang/String;");
        QJniObject wifi = context.callObjectMethod("getApplicationContext", "()Landroid/content/Context;")
                              .callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", service.object<jstring>());
        if (!wifi.isValid())
            return;
        QJniObject lock = wifi.callObjectMethod("createMulticastLock", "(Ljava/lang/String;)Landroid/net/wifi/WifiManager$MulticastLock;",
                                                QJniObject::fromString(QStringLiteral("quillbinder-live-sync")).object<jstring>());
        if (!lock.isValid())
            return;
        lock.callMethod<void>("setReferenceCounted", "(Z)V", jboolean(false));
        lock.callMethod<void>("acquire", "()V");
        QJniEnvironment env;
        if (env->ExceptionCheck()) {   // for example, the permission is missing
            env->ExceptionClear();
            qWarning() << "multicast lock: not acquired";
            return;
        }
        g_lock = lock;
    } else if (g_lock.isValid()) {
        g_lock.callMethod<void>("release", "()V");
        QJniEnvironment env;
        if (env->ExceptionCheck())
            env->ExceptionClear();
        g_lock = QJniObject();
    }
}

} // namespace multicastlock
} // namespace qb
#endif
