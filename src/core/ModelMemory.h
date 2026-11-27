#pragma once
// 0.99.0: the list of the AI models that are loaded, and the one rule for all of them (Speech Transcription, Translation
// Helper, Read Aloud, Handwriting):
//
//  - A model unused for the set time (default 10 minutes) is unloaded.
//  - Before a model loads: when the memory it goes into (RAM, or one graphics card's memory) is more than 50 % used (the
//    whole computer's use), the model of Quillbinder unused longest is unloaded first. When, after Quillbinder's own idle
//    models are unloaded, the new model would still bring that memory over 80 %, it is not loaded: requestLoad() says
//    why, and the caller can offer the next smaller model or the CPU.
//  - The system's low-memory signal unloads the idle models at once (a model used in the last 30 seconds is not idle).
//
// The program keeps the list; one add-on cannot unload another add-on's model. Program functions use this class
// directly; tools through tool interface 1.3 (ToolHost::requestModelLoad and the like, ToolModels::unloadModel).
//
// `where` is "ram" or "gpu:<name>". RAM is read from the system (SystemMemory.h); a card's figures are reported by the
// engine (reportDeviceMemory, the budget as the driver reports it). Unknown figures never refuse a load.
//
// "Model change in progress": changeBegin() and changeEnd() count the changes under way (an unload, a load until the new
// model is ready, also the first load); changeActive(bool) is emitted when the count goes from 0 to 1 and back, and the
// main window shows its message line while it is on.

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>

class QTimer;

namespace qb {
namespace sysmem {
class LowMemoryMonitor;
}
namespace modelmem {

struct Decision {
    bool allowed = true;
    QString reason;            // when refused: why (a sentence for the message line)
    QStringList unloaded;      // the ids unloaded to make room
};

struct LoadedModel {
    QString id;                // unique, "<tool id>:<name>" for a tool's model
    QString name;              // what the writer sees
    qint64 bytes = 0;
    QString where;             // "ram" or "gpu:<name>"
    qint64 lastUsedMs = 0;     // the clock's time of the last use
};

struct DeviceFigures {
    qint64 total = 0;
    qint64 available = 0;
    bool valid() const { return total > 0 && available >= 0 && available <= total; }
};

class ModelMemory : public QObject
{
    Q_OBJECT
public:
    // Asks a model's owner to unload it; false when it cannot now (busy).
    using Unloader = std::function<bool(const QString &id)>;

    static ModelMemory &instance();
    explicit ModelMemory(QObject *parent = nullptr);   // a program uses instance(); the tests may make their own

    // ---- the settings (File > Options > General > AI Models; AppSettings hands them over)
    // The cores as Qt counts them: 75 % rounded down; 6 or fewer: the cores minus 1; at least 1.
    static int defaultThreads(int cores);
    static int defaultThreads();                       // for QThread::idealThreadCount()
    void setThreadSetting(int threads) { m_threads = threads; }    // 0 = the default
    int aiThreads() const;
    void setIdleMinutes(int minutes) { m_idleMinutes = qBound(1, minutes, 24 * 60); }
    int idleMinutes() const { return m_idleMinutes; }
    static constexpr int kDefaultIdleMinutes = 10;
    // The limits (percent): above kReduceAt the longest-unused model goes first; a load must not bring the memory over kRefuseAt.
    static constexpr double kReduceAt = 0.50;
    static constexpr double kRefuseAt = 0.80;
    // A model used within this time is not idle for the low-memory signal.
    static constexpr qint64 kBusyMs = 30000;

    // ---- the registry
    // Before a load. When allowed, the change is counted as under way until modelLoaded(id) (or modelUnloaded(id) or
    // cancelLoad(id) when the load fails). `unloader` unloads the model later (the tool's ToolModels::unloadModel).
    Decision requestLoad(const QString &id, const QString &name, qint64 bytes, const QString &where, Unloader unloader);
    void cancelLoad(const QString &id);
    // The model is ready. A model that was not asked for is registered with what is given (bytes 0 when unknown).
    void modelLoaded(const QString &id, Unloader unloader = nullptr);
    void modelUsed(const QString &id);
    void modelUnloaded(const QString &id);     // the owner unloaded it itself
    void reportDeviceMemory(const QString &where, qint64 total, qint64 available);
    QList<LoadedModel> models() const;          // by last use, longest unused first
    bool isLoaded(const QString &id) const { return m_models.contains(id); }
    qint64 loadedBytes(const QString &where) const;

    // The figures of a memory now: RAM from the system, a card from its last report; invalid when not known.
    DeviceFigures device(const QString &where) const;
    // All memories with figures: "ram" first, then the cards reported (for the options page).
    QStringList devices() const;

    // ---- unloading
    // Unloads every model unused for the set time; returns the ids. Called by a timer; public for the tests.
    QStringList unloadIdle();
    // The low-memory signal: unloads the idle models at once (not those used in the last kBusyMs).
    QStringList onLowMemory();
    void startWatching(int checkIntervalMs = 30000);   // the idle timer and the system's signal
    sysmem::LowMemoryMonitor *monitor() const { return m_monitor; }

    // ---- the message "Model change in progress"
    void changeBegin();
    void changeEnd();
    bool changeActive() const { return m_changing > 0; }

    // ---- threads
    // Lowers the priority of the calling thread (the AI's worker threads call it first thing): nice 10 on Linux and
    // Android, below normal on Windows.
    static void lowerThreadPriority();

    // ---- tests
    void setClock(std::function<qint64()> clock) { m_clock = std::move(clock); }   // milliseconds
    void setIdleMs(qint64 ms) { m_idleMsOverride = ms; }                           // < 0: from the minutes
    void reset();                                                                   // forgets models, cards, counts

signals:
    void changeActiveChanged(bool on);
    void modelsChanged();
    void refused(const QString &reason);

private:
    struct Entry {
        LoadedModel info;
        Unloader unloader;
    };
    struct Pending {
        QString name;
        qint64 bytes = 0;
        QString where;
        Unloader unloader;
    };
    qint64 now() const;
    qint64 idleMs() const;
    bool unloadOne(const QString &id);
    QList<QString> byAge(const QString &where) const;   // longest unused first

    QHash<QString, Entry> m_models;
    QHash<QString, Pending> m_pending;
    QHash<QString, DeviceFigures> m_cards;
    int m_threads = 0;
    int m_idleMinutes = kDefaultIdleMinutes;
    qint64 m_idleMsOverride = -1;
    int m_changing = 0;
    std::function<qint64()> m_clock;
    sysmem::LowMemoryMonitor *m_monitor = nullptr;
    QTimer *m_timer = nullptr;
};

} // namespace modelmem
} // namespace qb
