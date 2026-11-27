#include "core/AiEngine.h"

#include "core/Addons.h"
#include "core/ModelMemory.h"
#include "core/TestSwitches.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>
#include <QMutexLocker>
#include <QPointer>
#include <QSet>
#include <QThread>
#include <algorithm>
#include <cstring>

#if defined(Q_OS_LINUX)
#include <link.h>
#endif

namespace qb {
namespace ai {

QString aiEngineId() { return QStringLiteral("ai-engine"); }
QString handwritingEngineId() { return QStringLiteral("handwriting-engine"); }

PackageState package(const QString &id)
{
    PackageState s;
    for (const addons::Installed &a : addons::installedAddons()) {
        if (a.manifest.id != id)
            continue;
        s.installed = true;
        s.enabled = a.enabled;
        s.fitsRuntime = !a.otherRuntime;
        s.version = a.manifest.version;
        s.dir = a.dir;
        break;
    }
    return s;
}

namespace {

QString engineLibDir(const PackageState &p) { return p.dir + QStringLiteral("/engine/lib"); }

// The registry of ModelMemory belongs to its thread (the main thread): a call from a worker waits for it.
template <typename F>
auto onThreadOf(QObject *context, F fn) -> decltype(fn())
{
    using R = decltype(fn());
    if (!context || QThread::currentThread() == context->thread())
        return fn();
    if constexpr (std::is_void_v<R>) {
        QMetaObject::invokeMethod(context, fn, Qt::BlockingQueuedConnection);
    } else {
        R r{};
        QMetaObject::invokeMethod(context, [&r, &fn] { r = fn(); }, Qt::BlockingQueuedConnection);
        return r;
    }
}

#if defined(Q_OS_LINUX)
// The libraries the engine brings, by the names the system's loader knows them by. When an add-on has loaded a library of
// one of these names from a folder of its own (Speech Transcription up to 1.9 brings its own whisper.cpp), the loader would
// give libqbai that one instead of the engine's (it goes by the name), and the two ggml versions would meet: refused.
struct LoadedScan {
    QString ownDir;
    QStringList foreign;
};

int scanLoaded(struct dl_phdr_info *info, size_t, void *data)
{
    auto *scan = static_cast<LoadedScan *>(data);
    if (!info->dlpi_name || !*info->dlpi_name)
        return 0;
    const QFileInfo fi(QString::fromLocal8Bit(info->dlpi_name));
    const QString name = fi.fileName();
    static const QStringList clash = {QStringLiteral("libwhisper.so.1"), QStringLiteral("libllama.so.1"),
                                      QStringLiteral("libmtmd.so.1"), QStringLiteral("libggml.so.1"),
                                      QStringLiteral("libggml-base.so.1"), QStringLiteral("libqwentts.so.0"),
                                      QStringLiteral("libsonic.so.0")};
    if (!clash.contains(name))
        return 0;
    const QString dir = QFileInfo(fi.absolutePath()).canonicalFilePath();
    if (!scan->ownDir.isEmpty() && dir != scan->ownDir)
        scan->foreign << QDir::toNativeSeparators(fi.absoluteFilePath());
    return 0;
}

QStringList foreignEngineLibraries(const QString &libraryPath)
{
    LoadedScan scan;
    scan.ownDir = QFileInfo(QFileInfo(libraryPath).absolutePath()).canonicalFilePath();
    dl_iterate_phdr(scanLoaded, &scan);
    return scan.foreign;
}
#else
QStringList foreignEngineLibraries(const QString &) { return {}; }
#endif

} // namespace

QString onnxRuntimeLibrary()
{
    const PackageState p = package(handwritingEngineId());
    if (!p.usable())
        return QString();
#if defined(Q_OS_WIN)
    const QString f = engineLibDir(p) + QStringLiteral("/onnxruntime.dll");
    return QFileInfo::exists(f) ? f : QString();
#else
    const QDir dir(engineLibDir(p));
    const QStringList found = dir.entryList({QStringLiteral("libonnxruntime.so.1*")}, QDir::Files, QDir::Name);
    return found.isEmpty() ? QString() : dir.filePath(found.first());
#endif
}

// ------------------------------------------------------------------ devices

QString Device::where() const
{
    return gpu ? QStringLiteral("gpu:") + name : QStringLiteral("ram");
}

Device pickDevice(const QList<Device> &devices, const QString &choice, qint64 bytes, QString *why)
{
    if (why)
        why->clear();
    Device cpu;
    cpu.name = QCoreApplication::translate("qb::ai", "Processor (CPU)");
    if (!devices.isEmpty() && !devices.first().gpu)
        cpu = devices.first();
    if (choice.startsWith(QLatin1String("gpu:"))) {
        for (const Device &d : devices)
            if (d.gpu && d.where() == choice)
                return d;
        if (why)
            *why = QCoreApplication::translate("qb::ai", "The graphics card \"%1\" was not found; the processor is used.")
                       .arg(choice.mid(4));
        return cpu;
    }
    if (choice != QLatin1String("auto"))
        return cpu;
    QList<Device> fits;
    bool anyCard = false;
    for (const Device &d : devices) {
        if (!d.gpu)
            continue;
        anyCard = true;
        if (bytes > 0 && d.free >= bytes)
            fits << d;
    }
    if (fits.isEmpty()) {
        if (why && anyCard)
            *why = QCoreApplication::translate("qb::ai", "The model does not fit into the free memory of the graphics card; "
                                                         "the processor is used.");
        return cpu;
    }
    std::stable_sort(fits.begin(), fits.end(), [](const Device &a, const Device &b) {
        if (a.integrated != b.integrated)
            return !a.integrated;   // a card with memory of its own first
        return a.free > b.free;
    });
    return fits.first();
}

QList<QPair<QString, QString>> deviceChoices(const QList<Device> &devices)
{
    QList<QPair<QString, QString>> out;
    out << qMakePair(QStringLiteral("auto"), QCoreApplication::translate("qb::ai", "Automatic"));
    out << qMakePair(QStringLiteral("cpu"), QCoreApplication::translate("qb::ai", "Processor (CPU)"));
    for (const Device &d : devices)
        if (d.gpu)
            out << qMakePair(d.where(), d.name);
    return out;
}

// ------------------------------------------------------------------ the engine

Engine &Engine::instance()
{
    // kept for the whole run (the engine's libraries are not unloaded at exit); its signals belong to the main thread
    static Engine *e = [] {
        auto *x = new Engine;
        if (QCoreApplication::instance())
            x->moveToThread(QCoreApplication::instance()->thread());
        return x;
    }();
    return *e;
}

QString Engine::libraryPath() const
{
    if (QB_TEST_ENV_SET("QB_AI_ENGINE_LIB")) {
        const QString f = QB_TEST_ENV("QB_AI_ENGINE_LIB");
        return QFileInfo::exists(f) ? QFileInfo(f).absoluteFilePath() : QString();
    }
    const PackageState p = package(aiEngineId());
    if (!p.usable())
        return QString();
#if defined(Q_OS_WIN)
    const QString f = engineLibDir(p) + QLatin1Char('/') + QLatin1String(QBAI_LIBRARY_WINDOWS);
#else
    const QString f = engineLibDir(p) + QLatin1Char('/') + QLatin1String(QBAI_LIBRARY_LINUX);
#endif
    return QFileInfo::exists(f) ? f : QString();
}

bool Engine::ensureLoaded(QString *error)
{
    QMutexLocker g(&m_mutex);
    if (m_api)
        return true;
    auto failed = [&](const QString &why) {
        m_error = why;
        if (error)
            *error = why;
        return false;
    };
    const QString path = libraryPath();
    if (path.isEmpty())
        return failed(QCoreApplication::translate("qb::ai", "The AI engine is not installed."));
    const QStringList foreign = foreignEngineLibraries(path);
    if (!foreign.isEmpty())
        return failed(QCoreApplication::translate("qb::ai", "An add-on has loaded a speech engine of its own (%1). Restart "
                                                            "Quillbinder, or update that add-on, to use the AI engine.")
                          .arg(foreign.join(QLatin1String(", "))));
    auto *lib = new QLibrary(path);
    if (!lib->load()) {
        const QString why = lib->errorString();
        delete lib;
        return failed(QCoreApplication::translate("qb::ai", "The AI engine could not be loaded: %1").arg(why));
    }
    auto get = reinterpret_cast<qbai_get_api_fn>(lib->resolve("qbai_get_api"));
    // version 1 is enough for the program; a tool that wants version 2's functions asks for it (apiFor)
    const qbai_api *api = get ? get(1) : nullptr;
    if (!api || api->version < 1 || api->size < int(QBAI_API_V1_SIZE)) {
        lib->unload();
        delete lib;
        return failed(QCoreApplication::translate("qb::ai", "The installed AI engine is older than this version of Quillbinder "
                                                            "needs. Install the engine again from the settings page."));
    }
    m_lib = lib;
    m_api = api;
    m_loadedFrom = path;
    m_error.clear();
    m_api->set_threads(modelmem::ModelMemory::instance().aiThreads());
    g.unlock();
    QMetaObject::invokeMethod(this, [this] { emit stateChanged(); }, Qt::QueuedConnection);
    return true;
}

const qbai_api *Engine::apiFor(int version)
{
    if (!ensureLoaded())
        return nullptr;
    auto get = reinterpret_cast<qbai_get_api_fn>(m_lib->resolve("qbai_get_api"));
    return get ? get(version) : nullptr;
}

QString Engine::version() const
{
    return m_api ? QString::fromUtf8(m_api->engine_version()) : QString();
}

QList<Device> Engine::devices(bool refresh)
{
    QList<Device> out;
    if (!m_api)
        return out;
    const int n = m_api->device_count(refresh ? 1 : 0);
    QSet<QString> names;
    for (int i = 0; i < n; ++i) {
        qbai_device d{};
        if (m_api->device_get(i, &d) != QBAI_OK)
            continue;
        Device e;
        e.index = d.index;
        e.gpu = d.type != QBAI_DEVICE_CPU;
        e.integrated = d.type == QBAI_DEVICE_IGPU;
        e.id = QString::fromUtf8(d.id);
        e.name = QString::fromUtf8(d.name).trimmed();
        if (e.name.isEmpty())
            e.name = e.id;
        if (!e.gpu)
            e.name = QCoreApplication::translate("qb::ai", "Processor (CPU)");
        else if (names.contains(e.name))
            e.name += QStringLiteral(" #%1").arg(e.index);   // two cards of one kind
        names.insert(e.name);
        e.free = d.memory_free;
        e.total = d.memory_total;
        out << e;
    }
    auto *mm = &modelmem::ModelMemory::instance();
    for (const Device &d : out)
        if (d.gpu && d.total > 0)
            onThreadOf(mm, [mm, d] { mm->reportDeviceMemory(d.where(), d.total, d.free); });
    return out;
}

void Engine::applyThreads()
{
    if (m_api)
        m_api->set_threads(modelmem::ModelMemory::instance().aiThreads());
}

void Engine::packageChanged()
{
    if (m_api)
        m_restartNeeded = true;
    emit stateChanged();
}

void Engine::unloadForTests()
{
    QMutexLocker g(&m_mutex);
    if (m_lib) {
        m_lib->unload();
        delete m_lib;
    }
    m_lib = nullptr;
    m_api = nullptr;
    m_loadedFrom.clear();
    m_restartNeeded = false;
}

// ------------------------------------------------------------------ a model

Model::Model(const QString &id, modelmem::ModelMemory *memory, QObject *parent)
    : QObject(parent), m_id(id), m_memory(memory ? memory : &modelmem::ModelMemory::instance())
{
}

Model::~Model()
{
    unload();
}

Device Model::device() const
{
    QMutexLocker g(&m_infoLock);
    return m_device;
}

ModelFiles Model::files() const
{
    QMutexLocker g(&m_infoLock);
    return m_files;
}

qint64 Model::bytes() const
{
    QMutexLocker g(&m_infoLock);
    return m_bytes;
}

int Model::sampleRate() const
{
    QMutexLocker g(&m_infoLock);
    return m_loaded ? m_rate : 0;
}

bool Model::canCloneVoice() const
{
    QMutexLocker g(&m_infoLock);
    return m_loaded && m_clone;
}

QStringList Model::voices() const
{
    QMutexLocker g(&m_infoLock);
    return m_loaded ? m_voices : QStringList();
}

QStringList Model::languages() const
{
    QMutexLocker g(&m_infoLock);
    return m_loaded ? m_languages : QStringList();
}

void Model::freeModel()
{
    m_loaded = false;
    if (m_model) {
        if (const qbai_api *api = Engine::instance().api())
            api->unload(m_model);
        m_model = nullptr;
    }
}

bool Model::unloadIfIdle()
{
    if (m_busy.load() > 0 || !m_lock.tryLock())
        return false;
    const bool had = m_model != nullptr;
    freeModel();
    m_lock.unlock();
    if (had)
        QMetaObject::invokeMethod(this, [this] { emit loadedChanged(false); }, Qt::QueuedConnection);
    return true;
}

void Model::unload()
{
    m_lock.lock();
    const bool had = m_model != nullptr;
    freeModel();
    m_lock.unlock();
    if (had) {
        auto *mm = m_memory;
        const QString id = m_id;
        onThreadOf(mm, [mm, id] { mm->modelUnloaded(id); });
        emit loadedChanged(false);
    }
}

bool Model::load(const ModelFiles &files, const QString &deviceChoice, QString *error)
{
    auto failed = [error](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    Engine &engine = Engine::instance();
    QString why;
    if (!engine.ensureLoaded(&why))
        return failed(why);
    const qbai_api *api = engine.api();
    if (isLoaded()) {
        const ModelFiles had = this->files();
        if (had.model == files.model && had.extra == files.extra && had.kind == files.kind)
            return true;
        unload();
    }
    const QList<Device> devices = engine.devices(true);
    const QByteArray model = QFile::encodeName(files.model), extra = QFile::encodeName(files.extra);
    qint64 bytes = files.memoryBytes;
    if (bytes <= 0)
        bytes = api->model_memory(files.kind, model.constData(), extra.isEmpty() ? nullptr : extra.constData(), 0);
    Device device = pickDevice(devices, deviceChoice, bytes, &why);
    QPointer<Model> self(this);
    auto unloader = [self](const QString &) { return self ? self->unloadIfIdle() : true; };
    auto *mm = m_memory;
    const QString id = m_id, name = files.name.isEmpty() ? QFileInfo(files.model).completeBaseName() : files.name;
    auto request = [&](const Device &d) {
        return onThreadOf(mm, [mm, id, name, bytes, d, unloader] { return mm->requestLoad(id, name, bytes, d.where(), unloader); });
    };
    const bool automatic = deviceChoice == QLatin1String("auto");
    const Device cpu = devices.isEmpty() ? Device() : devices.first();
    modelmem::Decision decision = request(device);
    if (!decision.allowed && automatic && device.gpu) {
        device = cpu;   // the card is refused by the memory rule: the computer's memory and the processor
        decision = request(device);
    }
    if (!decision.allowed)
        return failed(decision.reason);
    auto tryLoad = [&](const Device &d) -> qbai_model * {
        qbai_load_params p{};
        p.size = int(sizeof p);
        p.kind = files.kind;
        p.model_path = model.constData();
        p.extra_path = extra.isEmpty() ? nullptr : extra.constData();
        p.device = d.index;
        p.threads = 0;
        return api->load(&p);
    };
    m_busy.fetch_add(1);
    qbai_model *loaded = tryLoad(device);
    if (!loaded && automatic && device.gpu) {
        // the card did not take it after all (its driver's memory): once more on the processor
        onThreadOf(mm, [mm, id] { mm->cancelLoad(id); });
        device = cpu;
        decision = request(device);
        if (decision.allowed)
            loaded = tryLoad(device);
    }
    m_busy.fetch_sub(1);
    if (!loaded) {
        if (!decision.allowed)
            return failed(decision.reason);
        const QString text = QString::fromUtf8(api->last_error());   // the engine's error of this thread
        onThreadOf(mm, [mm, id] { mm->cancelLoad(id); });
        return failed(QCoreApplication::translate("qb::ai", "The model \"%1\" could not be loaded: %2").arg(name, text));
    }
    // the facts, read once (later readers never wait for a job)
    qbai_model_info info{};
    info.size = int(sizeof info);
    const bool haveInfo = api->model_info(loaded, &info) == QBAI_OK;
    QStringList voices, languages;
    for (int i = 0; const char *v = api->voice_name(loaded, i); ++i)
        voices << QString::fromUtf8(v);
    for (int i = 0; const char *l = api->language_name(loaded, i); ++i)
        languages << QString::fromUtf8(l);
    {
        QMutexLocker g(&m_lock);
        m_model = loaded;
        QMutexLocker f(&m_infoLock);
        m_device = device;
        m_files = files;
        m_bytes = bytes;
        m_rate = haveInfo ? info.sample_rate : 0;
        m_clone = haveInfo && info.voice_clone != 0;
        m_voices = voices;
        m_languages = languages;
        m_loaded = true;
    }
    onThreadOf(mm, [mm, id, unloader] { mm->modelLoaded(id, unloader); });
    emit loadedChanged(true);
    return true;
}

namespace {

struct Busy {
    std::atomic<int> &n;
    explicit Busy(std::atomic<int> &c) : n(c) { n.fetch_add(1); }
    ~Busy() { n.fetch_sub(1); }
};

struct SynthCall {
    const std::function<bool(const float *, qint64)> *chunk = nullptr;
    const std::atomic_bool *cancel = nullptr;
};

int synthChunk(const float *pcm, int64_t samples, void *user)
{
    auto *c = static_cast<SynthCall *>(user);
    return (*c->chunk)(pcm, qint64(samples)) ? 1 : 0;
}

int synthCancel(void *user)
{
    auto *c = static_cast<SynthCall *>(user);
    return c->cancel && c->cancel->load() ? 1 : 0;
}

struct AsrCall {
    const std::function<void(const QString &, qint64, qint64)> *segment = nullptr;
    const std::atomic_bool *cancel = nullptr;
};

void asrSegment(const char *text, int64_t from, int64_t to, void *user)
{
    auto *c = static_cast<AsrCall *>(user);
    if (c->segment && *c->segment)
        (*c->segment)(QString::fromUtf8(text), qint64(from), qint64(to));
}

int asrCancel(void *user)
{
    auto *c = static_cast<AsrCall *>(user);
    return c->cancel && c->cancel->load() ? 1 : 0;
}

} // namespace

bool Model::synthesize(const QString &text, const QString &language, const Voice &voice,
                       const std::function<bool(const float *, qint64)> &chunk, const std::atomic_bool *cancel,
                       QVector<float> *pcm, QString *error)
{
    Busy busy(m_busy);   // from here ModelMemory's request to unload is refused
    const qbai_api *api = Engine::instance().api();
    auto *mm = m_memory;
    const QString id = m_id;
    auto notLoaded = [error] {
        if (error)
            *error = QCoreApplication::translate("qb::ai", "The model is not loaded.");
        return false;
    };
    if (!isLoaded() || !api)
        return notLoaded();
    onThreadOf(mm, [mm, id] { mm->modelUsed(id); });   // before the lock: the main thread may wait for it
    const QByteArray t = text.toUtf8(), lang = language.toUtf8(), preset = voice.preset.toUtf8(),
                     clipText = voice.clipText.toUtf8(), instruct = voice.instruct.toUtf8();
    qbai_voice v{};
    v.size = int(sizeof v);
    v.preset = preset.isEmpty() ? nullptr : preset.constData();
    v.clip = voice.clip.isEmpty() ? nullptr : voice.clip.constData();
    v.clip_samples = voice.clip.size();
    v.clip_rate = voice.clipRate;
    v.clip_text = clipText.isEmpty() ? nullptr : clipText.constData();
    v.instruct = instruct.isEmpty() ? nullptr : instruct.constData();
    SynthCall call;
    call.chunk = &chunk;
    call.cancel = cancel;
    qbai_synth_params p{};
    p.size = int(sizeof p);
    p.text = t.constData();
    p.language = lang.isEmpty() ? nullptr : lang.constData();
    p.voice = &v;
    p.seed = -1;
    p.chunk = chunk ? synthChunk : nullptr;
    p.cancel = synthCancel;
    p.user = &call;
    qbai_pcm out{};
    int rc = QBAI_ERROR;
    {
        QMutexLocker g(&m_lock);
        if (!m_model)
            return notLoaded();
        rc = api->synthesize(m_model, &p, &out);
    }
    onThreadOf(mm, [mm, id] { mm->modelUsed(id); });
    if (rc != QBAI_OK) {
        api->free_pcm(&out);
        if (error)
            *error = rc == QBAI_CANCELLED ? QCoreApplication::translate("qb::ai", "Stopped.") : QString::fromUtf8(api->last_error());
        return false;
    }
    if (pcm) {
        pcm->resize(int(out.count));
        if (out.count > 0)
            std::memcpy(pcm->data(), out.samples, size_t(out.count) * sizeof(float));
    }
    api->free_pcm(&out);
    return true;
}

bool Model::transcribe(const QVector<float> &pcm16k, const QString &language, bool translate,
                       const std::function<void(const QString &, qint64, qint64)> &segment, const std::atomic_bool *cancel,
                       QString *text, QString *error)
{
    Busy busy(m_busy);
    const qbai_api *api = Engine::instance().api();
    auto *mm = m_memory;
    const QString id = m_id;
    auto notLoaded = [error] {
        if (error)
            *error = QCoreApplication::translate("qb::ai", "The model is not loaded.");
        return false;
    };
    if (!isLoaded() || !api)
        return notLoaded();
    onThreadOf(mm, [mm, id] { mm->modelUsed(id); });
    const QByteArray lang = language.isEmpty() ? QByteArray("auto") : language.toUtf8();
    AsrCall call;
    call.segment = &segment;
    call.cancel = cancel;
    qbai_transcribe_params p{};
    p.size = int(sizeof p);
    p.language = lang.constData();
    p.translate = translate ? 1 : 0;
    p.segment = asrSegment;
    p.cancel = asrCancel;
    p.user = &call;
    char *out = nullptr;
    int rc = QBAI_ERROR;
    {
        QMutexLocker g(&m_lock);
        if (!m_model)
            return notLoaded();
        rc = api->transcribe(m_model, pcm16k.constData(), pcm16k.size(), &p, &out);
    }
    onThreadOf(mm, [mm, id] { mm->modelUsed(id); });
    if (rc != QBAI_OK) {
        api->free_text(out);
        if (error)
            *error = rc == QBAI_CANCELLED ? QCoreApplication::translate("qb::ai", "Stopped.") : QString::fromUtf8(api->last_error());
        return false;
    }
    if (text)
        *text = QString::fromUtf8(out ? out : "").trimmed();
    api->free_text(out);
    return true;
}

bool stretch(const QVector<float> &in, int rate, double speed, QVector<float> *out, QString *error)
{
    Engine &engine = Engine::instance();
    QString why;
    if (!engine.ensureLoaded(&why)) {
        if (error)
            *error = why;
        return false;
    }
    const qbai_api *api = engine.api();
    qbai_pcm pcm{};
    const int rc = api->stretch(in.constData(), in.size(), rate, float(speed), &pcm);
    if (rc != QBAI_OK) {
        if (error)
            *error = QString::fromUtf8(api->last_error());
        api->free_pcm(&pcm);
        return false;
    }
    if (out) {
        out->resize(int(pcm.count));
        if (pcm.count > 0)
            std::memcpy(out->data(), pcm.samples, size_t(pcm.count) * sizeof(float));
    }
    api->free_pcm(&pcm);
    return true;
}

} // namespace ai
} // namespace qb
