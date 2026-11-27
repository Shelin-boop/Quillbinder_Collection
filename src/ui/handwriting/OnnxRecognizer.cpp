#include "ui/handwriting/OnnxRecognizer.h"

#include "core/AiCatalog.h"
#include "core/AiEngine.h"
#include "ui/AiInstallPanel.h"
#include "ui/handwriting/HandwritingSettings.h"

#include "third_party/onnxruntime/onnxruntime_c_api.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>
#include <QPainter>
#include <QPointer>
#include <QThread>
#include <QVBoxLayout>

namespace qb {
namespace handwriting {

namespace {

constexpr int kModelHeight = 96;
constexpr int kPadding = 16;
constexpr int kWidthMultiple = 8;
constexpr int kStepWidth = 8;
constexpr qint64 kModelBytes = qint64(150) * 1024 * 1024;
constexpr qint64 kListBytes = qint64(40) * 1024 * 1024;
const char *const kModelId = "handwriting-ppocrv6-small";

QString tr(const char *s) { return QCoreApplication::translate("qb::handwriting::OnnxRecognizer", s); }

// The C library, opened once and kept for the life of the program (an ONNX Runtime is not unloaded: its threads may still wind down).
struct Runtime {
    QLibrary lib;
    const OrtApi *api = nullptr;
    QString path;
};

Runtime *openRuntime(const QString &path, QString *error)
{
    static QHash<QString, Runtime *> opened;
    if (Runtime *r = opened.value(path))
        return r;
    auto *r = new Runtime;
    r->path = path;
    r->lib.setFileName(path);
    if (!r->lib.load()) {
        if (error)
            *error = tr("The handwriting engine could not be loaded: %1").arg(r->lib.errorString());
        delete r;
        return nullptr;
    }
    using GetBase = const OrtApiBase *(ORT_API_CALL *)();
    auto getBase = reinterpret_cast<GetBase>(r->lib.resolve("OrtGetApiBase"));
    const OrtApiBase *base = getBase ? getBase() : nullptr;
    // the header's version, or an older one the library may know (the structure only grows)
    for (int v = ORT_API_VERSION; base && !r->api && v >= 16; --v)
        r->api = base->GetApi(uint32_t(v));
    if (!r->api) {
        if (error)
            *error = tr("The handwriting engine does not offer the interface this version needs.");
        delete r;
        return nullptr;
    }
    opened.insert(path, r);
    return r;
}

} // namespace

struct OnnxRecognizer::Session {
    Runtime *rt = nullptr;
    OrtEnv *env = nullptr;
    OrtSession *session = nullptr;
    OrtMemoryInfo *memory = nullptr;
    QByteArray inputName, outputName;
    QString lastError;

    bool check(OrtStatus *st)
    {
        if (!st)
            return true;
        lastError = QString::fromUtf8(rt->api->GetErrorMessage(st));
        rt->api->ReleaseStatus(st);
        return false;
    }
    ~Session()
    {
        if (!rt)
            return;
        if (session)
            rt->api->ReleaseSession(session);
        if (memory)
            rt->api->ReleaseMemoryInfo(memory);
        if (env)
            rt->api->ReleaseEnv(env);
    }
    bool open(const QString &model, int threads)
    {
        const OrtApi *a = rt->api;
        if (!check(a->CreateEnv(ORT_LOGGING_LEVEL_ERROR, "quillbinder", &env)))
            return false;
        OrtSessionOptions *opts = nullptr;
        if (!check(a->CreateSessionOptions(&opts)))
            return false;
        // CPU provider only (the default); the arena and the memory pattern are off: the width of every line differs
        check(a->SetIntraOpNumThreads(opts, qMax(1, threads)));
        check(a->SetInterOpNumThreads(opts, 1));
        check(a->SetSessionGraphOptimizationLevel(opts, ORT_ENABLE_ALL));
        check(a->DisableCpuMemArena(opts));
        check(a->DisableMemPattern(opts));
#ifdef _WIN32
        const std::wstring path = model.toStdWString();
        const bool ok = check(a->CreateSession(env, path.c_str(), opts, &session));
#else
        const QByteArray path = QFile::encodeName(model);
        const bool ok = check(a->CreateSession(env, path.constData(), opts, &session));
#endif
        a->ReleaseSessionOptions(opts);
        if (!ok)
            return false;
        if (!check(a->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory)))
            return false;
        OrtAllocator *alloc = nullptr;
        if (!check(a->GetAllocatorWithDefaultOptions(&alloc)))
            return false;
        char *name = nullptr;
        if (!check(a->SessionGetInputName(session, 0, alloc, &name)))
            return false;
        inputName = name;
        alloc->Free(alloc, name);
        name = nullptr;
        if (!check(a->SessionGetOutputName(session, 0, alloc, &name)))
            return false;
        outputName = name;
        alloc->Free(alloc, name);
        return true;
    }
    // frames of one input; false with lastError
    bool run(const OnnxRecognizer::Input &in, int classes, Frames *out)
    {
        const OrtApi *a = rt->api;
        const int64_t shape[4] = {1, 3, in.height, in.width};
        OrtValue *input = nullptr;
        if (!check(a->CreateTensorWithDataAsOrtValue(memory, const_cast<float *>(in.data.constData()), size_t(in.data.size()) * sizeof(float),
                                                     shape, 4, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input)))
            return false;
        const char *inNames[1] = {inputName.constData()};
        const char *outNames[1] = {outputName.constData()};
        OrtValue *output = nullptr;
        const bool ran = check(a->Run(session, nullptr, inNames, &input, 1, outNames, 1, &output));
        a->ReleaseValue(input);
        if (!ran)
            return false;
        bool ok = false;
        OrtTensorTypeAndShapeInfo *info = nullptr;
        if (check(a->GetTensorTypeAndShape(output, &info))) {
            size_t n = 0;
            a->GetDimensionsCount(info, &n);
            QVector<int64_t> dims(static_cast<qsizetype>(n));
            a->GetDimensions(info, dims.data(), n);
            a->ReleaseTensorTypeAndShapeInfo(info);
            float *data = nullptr;
            if (n == 4 && dims[0] == 1 && dims[1] == classes && dims[2] == 1 && dims[3] > 0 && check(a->GetTensorMutableData(output, reinterpret_cast<void **>(&data)))) {
                *out = Frames::fromLogits(data, classes, int(dims[3]));
                ok = true;
            } else if (lastError.isEmpty()) {
                lastError = tr("The model's output has an unexpected shape.");
            }
        }
        a->ReleaseValue(output);
        return ok;
    }
};

OnnxRecognizer::Sources OnnxRecognizer::installedSources(const QStringList &languages)
{
    Sources s;
    s.runtimeLibrary = ai::onnxRuntimeLibrary();
    const ai::Catalog &cat = ai::Catalog::builtIn();
    if (const ai::CatalogModel *m = cat.model(QString::fromLatin1(kModelId))) {
        s.model = ai::modelFile(*m, QStringLiteral("model"));
        s.codec = ai::modelFile(*m, QStringLiteral("extra"));
        s.memoryBytes = qint64(m->memoryMB) * 1024 * 1024;
    }
    for (const QString &l : languages) {
        if (const ai::CatalogModel *w = cat.model(QStringLiteral("wordlist-") + l)) {
            const QString f = ai::modelFile(*w, QStringLiteral("model"));
            if (!f.isEmpty()) {
                s.wordLists.insert(l, f);
                s.memoryBytes += qint64(w->memoryMB) * 1024 * 1024;
            }
        }
    }
    return s;
}

OnnxRecognizer::OnnxRecognizer(QObject *parent, modelmem::ModelMemory *memory, SourceProvider provider)
    : Recognizer(parent), m_memory(memory ? memory : &modelmem::ModelMemory::instance()),
      m_provider(provider ? provider : SourceProvider(&OnnxRecognizer::installedSources))
{
    m_pool.setMaxThreadCount(1);
}

OnnxRecognizer::~OnnxRecognizer()
{
    m_closing = true;
    m_pool.clear();
    m_pool.waitForDone();
    const bool was = m_loaded.load();
    {
        QMutexLocker lock(&m_mutex);
        unloadLocked();
    }
    if (was)
        m_memory->modelUnloaded(memoryId());
}

bool OnnxRecognizer::isInstalled() const
{
    const Sources s = m_provider(QStringList());
    return !s.runtimeLibrary.isEmpty() && QFileInfo::exists(s.runtimeLibrary) && !s.model.isEmpty() && QFileInfo::exists(s.model)
        && !s.codec.isEmpty() && QFileInfo::exists(s.codec);
}

QStringList OnnxRecognizer::languages() const
{
    return isInstalled() ? modelLanguages() : QStringList();
}

OnnxRecognizer::Input OnnxRecognizer::prepareInput(const QImage &picture)
{
    Input in;
    in.height = kModelHeight;
    // on white (a see-through picture counts as white where it is empty), one gray channel
    QImage flat(picture.size(), QImage::Format_RGB32);
    flat.fill(Qt::white);
    {
        QPainter p(&flat);
        p.drawImage(0, 0, picture);
    }
    const QImage gray = flat.convertToFormat(QImage::Format_Grayscale8);
    in.pictureWidth = gray.width();
    const int ph = qMax(1, gray.height());
    in.scale = double(kModelHeight) / ph;
    const int w = qMax(1, int(std::lround(gray.width() * in.scale)));
    const QImage scaled = gray.scaled(w, kModelHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    const int total = kPadding + w + kPadding;
    in.width = (total + kWidthMultiple - 1) / kWidthMultiple * kWidthMultiple;
    const qint64 plane = qint64(in.width) * kModelHeight;
    in.data.fill(0.0f, int(3 * plane));   // white is 0
    for (int y = 0; y < kModelHeight; ++y) {
        const uchar *row = scaled.constScanLine(y);
        for (int x = 0; x < w; ++x) {
            const float v = 1.0f - row[x] / 255.0f;
            const qint64 at = qint64(y) * in.width + kPadding + x;
            in.data[int(at)] = v;
            in.data[int(plane + at)] = v;
            in.data[int(2 * plane + at)] = v;
        }
    }
    return in;
}

bool OnnxRecognizer::ensureLoaded(const QStringList &languages, QString *error)
{
    const Sources s = m_provider(languages);
    if (s.runtimeLibrary.isEmpty() || s.model.isEmpty() || s.codec.isEmpty()) {
        if (error)
            *error = tr("Handwriting is not installed.");
        return false;
    }
    if (!m_session) {
        Runtime *rt = openRuntime(s.runtimeLibrary, error);
        if (!rt)
            return false;
        QFile cf(s.codec);
        if (!cf.open(QIODevice::ReadOnly)) {
            if (error)
                *error = tr("The handwriting model's files could not be read.");
            return false;
        }
        QString why;
        m_codec = Codec::fromJson(cf.readAll(), &why);
        if (!m_codec.valid()) {
            if (error)
                *error = tr("The handwriting model's codec could not be read: %1").arg(why);
            return false;
        }
        auto session = std::make_unique<Session>();
        session->rt = rt;
        if (!session->open(s.model, m_memory->aiThreads())) {
            if (error)
                *error = tr("The handwriting model could not be loaded: %1").arg(session->lastError);
            return false;
        }
        m_session = std::move(session);
        m_loaded = true;
    }
    for (const QString &l : languages) {
        const QString f = s.wordLists.value(l);
        if (f.isEmpty() || m_lists.contains(l))
            continue;
        auto list = std::make_shared<WordList>();
        if (list->loadGzip(f))
            m_lists.insert(l, list);
    }
    return true;
}

void OnnxRecognizer::unloadLocked()
{
    m_session.reset();
    m_lists.clear();
    m_codec = Codec();
    m_loaded = false;
}

bool OnnxRecognizer::unloadIfIdle()
{
    if (!m_mutex.tryLock())
        return false;
    unloadLocked();
    m_mutex.unlock();
    return true;
}

Result OnnxRecognizer::readNow(const Request &request)
{
    QMutexLocker lock(&m_mutex);
    return readLocked(request);
}

Result OnnxRecognizer::readLocked(const Request &request)
{
    Result r;
    if (request.picture.isNull()) {
        r.ok = true;
        return r;
    }
    QStringList langs;
    for (const QString &l : request.languages)
        if (hasModelFor(l) && !langs.contains(l))
            langs << l;
    QString error;
    if (!ensureLoaded(langs, &error)) {
        r.error = error;
        return r;
    }
    const Input in = prepareInput(request.picture);
    Frames frames;
    if (!m_session->run(in, m_codec.classes, &frames)) {
        r.error = tr("The line could not be read: %1").arg(m_session->lastError);
        return r;
    }
    // the characters as read, or the best of the languages' word layers
    QList<const WordList *> lists;
    if (m_wordLayer)
        for (const QString &l : langs)
            lists << m_lists.value(l).get();   // none for a language without an installed list: it is skipped
    const LayerResult best = readBest(frames, m_codec, lists);
    r.ok = true;
    const double pictureW = request.picture.width(), pictureH = request.picture.height();
    for (const LayerWord &lw : best.words) {
        Word w;
        w.text = lw.text;
        w.alternatives = lw.alternatives;
        w.confidence = lw.confidence;
        // the steps are 8 px of the model's input; its picture starts 16 px in
        const double x0 = ((lw.frameFrom * kStepWidth - kPadding) / in.scale);
        const double x1 = (((lw.frameTo + 1) * kStepWidth - kPadding) / in.scale);
        const double left = qBound(0.0, x0, pictureW), right = qBound(left, x1, pictureW);
        w.box = QRectF(left, 0, qMax(1.0, right - left), pictureH);
        r.words << w;
    }
    return r;
}

void OnnxRecognizer::recognize(const Request &request, const std::function<void(const Result &)> &done)
{
    auto fail = [this, done](const QString &why) {
        Result r;
        r.error = why;
        QMetaObject::invokeMethod(this, [done, r] { done(r); }, Qt::QueuedConnection);
    };
    if (!isInstalled()) {
        fail(tr("Handwriting is not installed."));
        return;
    }
    QStringList langs;
    for (const QString &l : request.languages)
        if (hasModelFor(l) && !langs.contains(l))
            langs << l;
    // the registry first, on this (the main) thread: the memory rule may refuse, and other models may be unloaded for it
    const Sources s = m_provider(langs);
    qint64 bytes = s.memoryBytes > 0 ? s.memoryBytes : kModelBytes + kListBytes * s.wordLists.size();
    QPointer<OnnxRecognizer> self(this);
    auto unloader = [self](const QString &) { return self ? self->unloadIfIdle() : true; };
    const modelmem::Decision d = m_memory->requestLoad(memoryId(), tr("Handwriting"), bytes, QStringLiteral("ram"), unloader);
    if (!d.allowed) {
        fail(d.reason.isEmpty() ? tr("There is not enough memory to load the handwriting model.") : d.reason);
        return;
    }
    m_pool.start([self, request, done, unloader]() {
        if (!self || self->m_closing)
            return;
        modelmem::ModelMemory::lowerThreadPriority();
        const bool wasLoaded = self->m_loaded.load();
        Result r = self->readNow(request);
        modelmem::ModelMemory *mm = self->m_memory;
        const QString id = self->memoryId();
        const bool nowLoaded = self->m_loaded.load();
        if (self->m_closing)
            return;
        QMetaObject::invokeMethod(self.data(), [self, mm, id, wasLoaded, nowLoaded, unloader, done, r] {
            if (nowLoaded && !wasLoaded)
                mm->modelLoaded(id, unloader);
            else if (nowLoaded)
                mm->modelUsed(id);
            else
                mm->cancelLoad(id);
            done(r);
        }, Qt::QueuedConnection);
    });
}

OnnxRecognizer *installOnnxRecognizer(QObject *parent)
{
    OnnxRecognizer *r = nullptr;
    if (!Recognizer::current()) {
        r = new OnnxRecognizer(parent);
        Recognizer::setCurrent(r);
    }
    Recognizer::Installer inst;
    inst.install = [](QWidget *parentWidget) {
        QDialog dlg(parentWidget);
        dlg.setObjectName(QStringLiteral("hwInstallDialog"));
        dlg.setWindowTitle(QCoreApplication::translate("qb::handwriting::OnnxRecognizer", "Install Handwriting"));
        auto *lay = new QVBoxLayout(&dlg);
        auto *panel = new AiInstallPanel(ai::handwritingEngineId(), {QStringLiteral("handwriting"), QStringLiteral("wordlist")}, &dlg);
        panel->setObjectName(QStringLiteral("hwInstallPanel"));
        lay->addWidget(panel);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        lay->addWidget(buttons);
        dlg.resize(760, 460);
        dlg.exec();
    };
    inst.status = []() -> QString {
        const ai::PackageState p = ai::package(ai::handwritingEngineId());
        const ai::Catalog &cat = ai::Catalog::builtIn();
        const ai::CatalogModel *m = cat.model(QString::fromLatin1(kModelId));
        const bool model = m && ai::isInstalled(*m);
        QStringList lists;
        for (const QString &l : modelLanguages())
            if (const ai::CatalogModel *w = cat.model(QStringLiteral("wordlist-") + l))
                if (ai::isInstalled(*w))
                    lists << l;
        if (p.usable() && model)
            return QCoreApplication::translate("qb::handwriting::OnnxRecognizer", "Installed: the engine, the line model and the word lists of %1.")
                .arg(lists.isEmpty() ? QCoreApplication::translate("qb::handwriting::OnnxRecognizer", "no language") : lists.join(QStringLiteral(", ")));
        QStringList missing;
        if (!p.usable())
            missing << QCoreApplication::translate("qb::handwriting::OnnxRecognizer", "the engine");
        if (!model)
            missing << QCoreApplication::translate("qb::handwriting::OnnxRecognizer", "the line model");
        return QCoreApplication::translate("qb::handwriting::OnnxRecognizer", "Not installed yet: %1. Install it to write by hand.").arg(missing.join(QStringLiteral(", ")));
    };
    inst.remove = []() {
        const ai::Catalog &cat = ai::Catalog::builtIn();
        for (const ai::CatalogModel &m : cat.ofKinds({QStringLiteral("handwriting"), QStringLiteral("wordlist")}))
            ai::removeModel(m);
    };
    Recognizer::setInstaller(inst);
    return r;
}

} // namespace handwriting
} // namespace qb
