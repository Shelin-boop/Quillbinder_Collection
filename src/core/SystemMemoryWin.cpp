// Windows: GlobalMemoryStatusEx and the low-memory resource notification (see SystemMemory.h). Compiled on Windows only.
#include "core/SystemMemory.h"

#include <QTimer>
#include <QWinEventNotifier>

#include <windows.h>

namespace qb {
namespace sysmem {

Figures platformRam()
{
    Figures f;
    MEMORYSTATUSEX st;
    st.dwLength = sizeof(st);
    if (GlobalMemoryStatusEx(&st)) {
        f.total = qint64(st.ullTotalPhys);
        f.available = qMin(qint64(st.ullAvailPhys), f.total);
    }
    return f;
}

bool LowMemoryMonitor::platformStart()
{
    HANDLE h = CreateMemoryResourceNotification(LowMemoryResourceNotification);
    if (!h)
        return false;
    m_handle = h;
    auto *n = new QWinEventNotifier(h, this);
    m_winNotifier = n;
    // the handle stays signaled while memory is low: after one signal, wait 30 s before listening again
    m_rearm = new QTimer(this);
    m_rearm->setSingleShot(true);
    m_rearm->setInterval(30000);
    connect(m_rearm, &QTimer::timeout, n, [n] { n->setEnabled(true); });
    connect(n, &QWinEventNotifier::activated, this, [this, n] {
        n->setEnabled(false);
        m_rearm->start();
        emit lowMemory();
    });
    m_method = QStringLiteral("notification");
    return true;
}

void LowMemoryMonitor::platformStop()
{
    delete m_winNotifier;
    m_winNotifier = nullptr;
    delete m_rearm;
    m_rearm = nullptr;
    if (m_handle)
        CloseHandle(HANDLE(m_handle));
    m_handle = nullptr;
}

} // namespace sysmem
} // namespace qb
