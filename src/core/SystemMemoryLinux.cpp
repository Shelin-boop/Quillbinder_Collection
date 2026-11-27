// Linux: /proc/meminfo and a PSI trigger (see SystemMemory.h).
#include "core/SystemMemory.h"

#include <QFile>
#include <QSocketNotifier>
#include <QTimer>

#include <fcntl.h>
#include <unistd.h>

namespace qb {
namespace sysmem {

Figures platformRam()
{
    Figures f;
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly))
        return f;
    // (a file of /proc has no size: read it whole, atEnd() would say "end" at once)
    const QList<QByteArray> lines = file.readAll().split('\n');
    qint64 total = 0, avail = -1;
    for (const QByteArray &line : lines) {
        auto kb = [&line](const char *key) -> qint64 {
            if (!line.startsWith(key))
                return -1;
            bool ok = false;
            const qint64 v = line.mid(int(qstrlen(key))).trimmed().split(' ').value(0).toLongLong(&ok);
            return ok ? v * 1024 : -1;
        };
        if (const qint64 v = kb("MemTotal:"); v >= 0)
            total = v;
        else if (const qint64 a = kb("MemAvailable:"); a >= 0)
            avail = a;
        if (total > 0 && avail >= 0)
            break;
    }
    if (total > 0 && avail >= 0) {
        f.total = total;
        f.available = qMin(avail, total);
    }
    return f;
}

bool LowMemoryMonitor::platformStart()
{
    // "some 150000 2000000": a stall of 150 ms in a window of 2 s (a multiple of 2 s: what a process without
    // privileges may ask for)
    const int fd = ::open("/proc/pressure/memory", O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0)
        return false;
    static const char trigger[] = "some 150000 2000000";
    if (::write(fd, trigger, sizeof(trigger)) < 0) {
        ::close(fd);
        return false;
    }
    m_fd = fd;
    // the kernel signals a trigger as an exceptional condition (POLLPRI)
    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Exception, this);
    connect(m_notifier, &QSocketNotifier::activated, this, [this] { emit lowMemory(); });
    m_method = QStringLiteral("psi");
    return true;
}

void LowMemoryMonitor::platformStop()
{
    delete m_notifier;
    m_notifier = nullptr;
    if (m_fd >= 0)
        ::close(m_fd);
    m_fd = -1;
}

} // namespace sysmem
} // namespace qb
