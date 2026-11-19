#include "core/Platform.h"

#include <QList>
#include "AppInfo.h"

#include <QFile>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_UNIX) && !defined(Q_OS_LINUX)
#include <signal.h>
#endif

namespace qb {
namespace platform {

bool processAlive(qint64 pid)
{
    if (pid <= 0)
        return false;
#if defined(Q_OS_LINUX)
    return QFile::exists(QStringLiteral("/proc/") + QString::number(pid));
#elif defined(Q_OS_WIN)
    HANDLE h = ::OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, DWORD(pid));
    if (!h)
        return ::GetLastError() == ERROR_ACCESS_DENIED; // there, but not ours to look at
    const bool running = ::WaitForSingleObject(h, 0) == WAIT_TIMEOUT;
    ::CloseHandle(h);
    return running;
#else
    return ::kill(pid_t(pid), 0) == 0;
#endif
}

QStringList libraryPatterns()
{
#ifdef Q_OS_WIN
    return {QStringLiteral("*.dll")};
#else
    return {QStringLiteral("*.so")};
#endif
}

bool isLibraryFile(const QString &path)
{
#ifdef Q_OS_WIN
    return path.endsWith(QLatin1String(".dll"), Qt::CaseInsensitive);
#else
    return path.endsWith(QLatin1String(".so"));
#endif
}

QString programFileName()
{
#ifdef Q_OS_WIN
    return QStringLiteral(QB_APP_ID ".exe");
#else
    return QStringLiteral(QB_APP_ID);
#endif
}

QString packagePlatform()
{
#ifdef Q_OS_WIN
    return QStringLiteral("windows-x64");
#elif defined(Q_OS_ANDROID)
    // A0 (2026-11-14): no update or add-on package is made for Android (one full APK); with a name of its own a
    // package made for Linux is refused instead of installed
#if defined(Q_PROCESSOR_X86_64)
    return QStringLiteral("android-x86_64"); // the emulated tablets of the checks (0.91.3)
#else
    return QStringLiteral("android-arm64");
#endif
#else
    return QStringLiteral("linux-x86_64");
#endif
}

QByteArray qpaWithAltGr(const QByteArray &current)
{
    if (current.isEmpty())
        return QByteArrayLiteral("windows:altgr");
    const int colon = current.indexOf(':');
    const QByteArray name = colon < 0 ? current : current.left(colon);
    if (name != "windows")
        return QByteArray();
    const QList<QByteArray> switches = colon < 0 ? QList<QByteArray>() : current.mid(colon + 1).split(',');
    if (switches.contains("altgr"))
        return QByteArray();
    if (colon < 0 || current.endsWith(':'))
        return name + ":altgr";
    return current + ",altgr";
}

} // namespace platform
} // namespace qb
