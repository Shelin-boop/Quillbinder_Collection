#include "core/SystemMemory.h"

#include <QMutex>
#include <QMutexLocker>

namespace qb {
namespace sysmem {

namespace {
QMutex &fakeMutex() { static QMutex m; return m; }
bool &fakeOn() { static bool b = false; return b; }
Figures &fakeFigures() { static Figures f; return f; }
} // namespace

Figures ram()
{
    {
        QMutexLocker l(&fakeMutex());
        if (fakeOn())
            return fakeFigures();
    }
    return platformRam();
}

void setFakeRam(const Figures &f)
{
    QMutexLocker l(&fakeMutex());
    fakeFigures() = f;
    fakeOn() = true;
}

void clearFakeRam()
{
    QMutexLocker l(&fakeMutex());
    fakeOn() = false;
}

LowMemoryMonitor::LowMemoryMonitor(QObject *parent) : QObject(parent) {}

LowMemoryMonitor::~LowMemoryMonitor()
{
    stop();
}

bool LowMemoryMonitor::start()
{
    if (m_active)
        return true;
    m_active = platformStart();
    return m_active;
}

void LowMemoryMonitor::stop()
{
    if (!m_active)
        return;
    platformStop();
    m_active = false;
    m_method.clear();
}

} // namespace sysmem
} // namespace qb
