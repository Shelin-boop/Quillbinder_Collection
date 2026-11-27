#include "core/ModelMemory.h"

#include "core/SystemMemory.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QThread>
#include <QTimer>

#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace qb {
namespace modelmem {

namespace {
QString megabytes(qint64 bytes)
{
    return QString::number(double(bytes) / (1024.0 * 1024.0), 'f', 0);
}

QString whereLabel(const QString &where)
{
    if (where.startsWith(QLatin1String("gpu:")))
        return QCoreApplication::translate("ModelMemory", "the memory of the graphics card %1").arg(where.mid(4));
    return QCoreApplication::translate("ModelMemory", "the computer's memory");
}
} // namespace

ModelMemory &ModelMemory::instance()
{
    static ModelMemory *m = new ModelMemory;   // lives until the process ends
    return *m;
}

ModelMemory::ModelMemory(QObject *parent) : QObject(parent) {}

int ModelMemory::defaultThreads(int cores)
{
    if (cores <= 6)
        return qMax(1, cores - 1);
    return qMax(1, (cores * 75) / 100);   // rounded down
}

int ModelMemory::defaultThreads()
{
    return defaultThreads(QThread::idealThreadCount());
}

int ModelMemory::aiThreads() const
{
    return m_threads > 0 ? m_threads : defaultThreads();
}

qint64 ModelMemory::now() const
{
    return m_clock ? m_clock() : QDateTime::currentMSecsSinceEpoch();
}

qint64 ModelMemory::idleMs() const
{
    return m_idleMsOverride >= 0 ? m_idleMsOverride : qint64(m_idleMinutes) * 60 * 1000;
}

DeviceFigures ModelMemory::device(const QString &where) const
{
    DeviceFigures d;
    if (where == QLatin1String("ram")) {
        const sysmem::Figures f = sysmem::ram();
        d.total = f.total;
        d.available = f.available;
        return d;
    }
    return m_cards.value(where);
}

QStringList ModelMemory::devices() const
{
    QStringList out{QStringLiteral("ram")};
    QStringList cards = m_cards.keys();
    cards.sort();
    return out + cards;
}

void ModelMemory::reportDeviceMemory(const QString &where, qint64 total, qint64 available)
{
    if (where.isEmpty() || where == QLatin1String("ram"))
        return;   // the program reads the RAM itself
    DeviceFigures d;
    d.total = total;
    d.available = available;
    m_cards.insert(where, d);
}

QList<LoadedModel> ModelMemory::models() const
{
    QList<LoadedModel> out;
    for (const Entry &e : m_models)
        out << e.info;
    std::sort(out.begin(), out.end(), [](const LoadedModel &a, const LoadedModel &b) {
        return a.lastUsedMs != b.lastUsedMs ? a.lastUsedMs < b.lastUsedMs : a.id < b.id;
    });
    return out;
}

qint64 ModelMemory::loadedBytes(const QString &where) const
{
    qint64 sum = 0;
    for (const Entry &e : m_models)
        if (e.info.where == where)
            sum += e.info.bytes;
    return sum;
}

QList<QString> ModelMemory::byAge(const QString &where) const
{
    QList<QString> out;
    for (const LoadedModel &m : models())
        if (m.where == where)
            out << m.id;
    return out;
}

bool ModelMemory::unloadOne(const QString &id)
{
    const auto it = m_models.constFind(id);
    if (it == m_models.constEnd())
        return false;
    const Unloader fn = it->unloader;
    changeBegin();
    const bool ok = fn ? fn(id) : false;
    if (ok)
        m_models.remove(id);
    changeEnd();
    if (ok)
        emit modelsChanged();
    return ok;
}

Decision ModelMemory::requestLoad(const QString &id, const QString &name, qint64 bytes, const QString &where,
                                               Unloader unloader)
{
    Decision d;
    if (m_models.contains(id)) {   // loaded already: nothing to make room for
        modelUsed(id);
        return d;
    }
    if (m_pending.contains(id)) {  // asked twice: counted once
        m_pending[id].unloader = unloader;
        return d;
    }
    changeBegin();   // the unloads and the load, until the new model is ready (or the load is refused)
    const DeviceFigures fig = device(where);
    if (fig.valid()) {
        qint64 avail = fig.available;
        const qint64 total = fig.total;
        auto used = [&] { return double(total - avail) / double(total); };
        // 1. above half: the longest unused model of this memory goes first (more while it stays above half)
        QList<QString> order = byAge(where);
        int next = 0;
        while (used() > kReduceAt && next < order.size()) {
            const QString victim = order.at(next++);
            const qint64 freed = m_models.value(victim).info.bytes;
            if (unloadOne(victim)) {
                avail = qMin(total, avail + freed);
                d.unloaded << victim;
            }
        }
        // 2. the new model must not bring the memory over 80 %
        auto fits = [&] { return double(total - avail + bytes) / double(total) <= kRefuseAt; };
        if (!fits()) {
            qint64 reachable = avail;
            for (int i = next; i < order.size(); ++i)
                reachable += m_models.value(order.at(i)).info.bytes;
            if (double(total - qMin(total, reachable) + bytes) / double(total) <= kRefuseAt) {
                while (!fits() && next < order.size()) {
                    const QString victim = order.at(next++);
                    const qint64 freed = m_models.value(victim).info.bytes;
                    if (unloadOne(victim)) {
                        avail = qMin(total, avail + freed);
                        d.unloaded << victim;
                    }
                }
            }
        }
        if (!fits()) {
            d.allowed = false;
            d.reason = QCoreApplication::translate("ModelMemory", "Not enough memory for the model \"%1\" (%2 MB): %3 is %4 % used, and the model would bring it over %5 %.")
                           .arg(name, megabytes(bytes), whereLabel(where))
                           .arg(int(used() * 100 + 0.5))
                           .arg(int(kRefuseAt * 100 + 0.5));
            changeEnd();
            emit refused(d.reason);
            return d;
        }
    }
    Pending p;
    p.name = name;
    p.bytes = bytes;
    p.where = where;
    p.unloader = std::move(unloader);
    m_pending.insert(id, p);
    return d;
}

void ModelMemory::cancelLoad(const QString &id)
{
    if (m_pending.remove(id))
        changeEnd();
}

void ModelMemory::modelLoaded(const QString &id, Unloader unloader)
{
    Entry e;
    e.info.id = id;
    e.info.lastUsedMs = now();
    const auto p = m_pending.constFind(id);
    if (p != m_pending.constEnd()) {
        e.info.name = p->name;
        e.info.bytes = p->bytes;
        e.info.where = p->where;
        e.unloader = p->unloader ? p->unloader : unloader;
        m_pending.erase(p);
        changeEnd();
    } else {
        const auto old = m_models.constFind(id);
        if (old != m_models.constEnd()) {
            e = *old;
            e.info.lastUsedMs = now();
        } else {
            e.info.name = id;
            e.info.where = QStringLiteral("ram");
        }
        if (unloader)
            e.unloader = unloader;
    }
    m_models.insert(id, e);
    emit modelsChanged();
}

void ModelMemory::modelUsed(const QString &id)
{
    const auto it = m_models.find(id);
    if (it != m_models.end())
        it->info.lastUsedMs = now();
}

void ModelMemory::modelUnloaded(const QString &id)
{
    if (m_pending.remove(id))
        changeEnd();
    if (m_models.remove(id))
        emit modelsChanged();
}

QStringList ModelMemory::unloadIdle()
{
    QStringList out;
    const qint64 t = now();
    for (const LoadedModel &m : models())
        if (t - m.lastUsedMs >= idleMs() && unloadOne(m.id))
            out << m.id;
    return out;
}

QStringList ModelMemory::onLowMemory()
{
    QStringList out;
    const qint64 t = now();
    for (const LoadedModel &m : models())
        if (t - m.lastUsedMs >= kBusyMs && unloadOne(m.id))
            out << m.id;
    return out;
}

void ModelMemory::startWatching(int checkIntervalMs)
{
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, [this] { unloadIdle(); });
    }
    m_timer->start(checkIntervalMs);
    if (!m_monitor) {
        m_monitor = new sysmem::LowMemoryMonitor(this);
        connect(m_monitor, &sysmem::LowMemoryMonitor::lowMemory, this, [this] { onLowMemory(); });
    }
    m_monitor->start();
}

void ModelMemory::changeBegin()
{
    if (++m_changing == 1)
        emit changeActiveChanged(true);
}

void ModelMemory::changeEnd()
{
    if (m_changing <= 0)
        return;
    if (--m_changing == 0)
        emit changeActiveChanged(false);
}

void ModelMemory::reset()
{
    m_models.clear();
    m_pending.clear();
    m_cards.clear();
    m_idleMsOverride = -1;
    m_clock = nullptr;
    const bool was = m_changing > 0;
    m_changing = 0;
    if (was)
        emit changeActiveChanged(false);
    emit modelsChanged();
}

void ModelMemory::lowerThreadPriority()
{
#ifdef Q_OS_WIN
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
#elif defined(Q_OS_LINUX)
    // Linux and Android: nice is a property of the thread
    setpriority(PRIO_PROCESS, id_t(::syscall(SYS_gettid)), 10);
#endif
}

} // namespace modelmem
} // namespace qb
