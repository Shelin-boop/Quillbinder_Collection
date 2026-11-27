#pragma once
// 0.99.0: the system's own memory figures and its low-memory signal, for the rule of the AI models (ModelMemory).
//
// ram(): the whole computer's memory (not Quillbinder's own): total and available bytes.
//   Linux: /proc/meminfo (MemTotal, MemAvailable); Windows: GlobalMemoryStatusEx; Android: ActivityManager.MemoryInfo
//   (through QJniObject, read at each call: only TRIM_MEMORY_UI_HIDDEN and TRIM_MEMORY_BACKGROUND still arrive on
//   Android 14 and later, so the figures are asked for before each load).
// LowMemoryMonitor: the system's signal that memory is short. Linux: a PSI trigger on /proc/pressure/memory when it can
//   be opened (a trigger of an unprivileged process needs a window that is a multiple of 2 seconds), watched with a
//   QSocketNotifier; Windows: the low-memory resource notification; Android: none here (the Java side may call
//   injectLowMemory() on a trim signal).
// A graphics card's figures are not read here: the engine reports them (budget as the driver reports it) through the
// tool interface (ToolHost::reportDeviceMemory); the program has no Vulkan of its own.
//
// The files SystemMemoryLinux.cpp, SystemMemoryWin.cpp and SystemMemoryAndroid.cpp each define platformRam() and
// the LowMemoryMonitor::platform...() functions; CMakeLists.txt compiles the one of the system.

#include <QObject>
#include <QtGlobal>

class QSocketNotifier;
class QTimer;

namespace qb {
namespace sysmem {

struct Figures {
    qint64 total = 0;       // bytes; 0 = not known
    qint64 available = 0;   // bytes that can be used without swapping
    bool valid() const { return total > 0 && available >= 0 && available <= total; }
    // Share in use, 0 to 1 (0 when the figures are not valid).
    double usedShare() const { return valid() ? double(total - available) / double(total) : 0.0; }
};

// The figures of the whole computer now (the injected ones while a test has set them).
Figures ram();

// Test hook: ram() returns these figures until clearFakeRam().
void setFakeRam(const Figures &f);
void clearFakeRam();

// The platform's own reading (SystemMemory<System>.cpp); ram() uses it when no figures are injected.
Figures platformRam();

class LowMemoryMonitor : public QObject
{
    Q_OBJECT
public:
    explicit LowMemoryMonitor(QObject *parent = nullptr);
    ~LowMemoryMonitor() override;
    // Starts watching; false when the system offers no signal (it can be called again later).
    bool start();
    void stop();
    bool active() const { return m_active; }
    // What watches: "psi" (Linux), "notification" (Windows), "" (none).
    QString method() const { return m_method; }
    // A signal as the system would send it (tests; Android's Java side on a trim signal).
    void injectLowMemory() { emit lowMemory(); }

signals:
    void lowMemory();

private:
    bool platformStart();
    void platformStop();
    bool m_active = false;
    QString m_method;
    int m_fd = -1;                       // Linux: the PSI file
    QSocketNotifier *m_notifier = nullptr;
    void *m_handle = nullptr;            // Windows: the notification handle
    QObject *m_winNotifier = nullptr;    // Windows: QWinEventNotifier
    QTimer *m_rearm = nullptr;
};

} // namespace sysmem
} // namespace qb
