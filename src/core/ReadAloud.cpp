#include "core/ReadAloud.h"

#include "core/ModelMemory.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QLocale>
#include <QMutex>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>
#include <cmath>
#include <cstring>

namespace qb {
namespace readaloud {

static QString tr(const char *text)
{
    return QCoreApplication::translate("readaloud", text);
}

// ======================================================================================================== the choices

QStringList voiceIds()
{
    return {QStringLiteral("female1"), QStringLiteral("female2"), QStringLiteral("female3"),
            QStringLiteral("male1"),   QStringLiteral("male2"),   QStringLiteral("male3")};
}
QStringList styleIds() { return {QStringLiteral("calm"), QStringLiteral("neutral"), QStringLiteral("action")}; }
QStringList speedIds() { return {QStringLiteral("slow"), QStringLiteral("medium"), QStringLiteral("fast")}; }

QString voiceLabel(const QString &id)
{
    if (id == QLatin1String("female1")) return tr("Female 1");
    if (id == QLatin1String("female2")) return tr("Female 2");
    if (id == QLatin1String("female3")) return tr("Female 3");
    if (id == QLatin1String("male1")) return tr("Male 1");
    if (id == QLatin1String("male2")) return tr("Male 2");
    if (id == QLatin1String("male3")) return tr("Male 3");
    return id;
}
QString styleLabel(const QString &id)
{
    if (id == QLatin1String("calm")) return tr("Calm");
    if (id == QLatin1String("neutral")) return tr("Neutral");
    if (id == QLatin1String("action")) return tr("Action");
    return id;
}
QString speedLabel(const QString &id)
{
    if (id == QLatin1String("slow")) return tr("Slow");
    if (id == QLatin1String("medium")) return tr("Medium");
    if (id == QLatin1String("fast")) return tr("Fast");
    return id;
}
double speedFactor(const QString &id)
{
    if (id == QLatin1String("slow")) return 0.85;
    if (id == QLatin1String("fast")) return 1.15;
    return 1.0;
}

Settings Settings::defaults(bool android)
{
    Settings s;
    s.barShown = !android;   // shown on Linux and Windows, hidden on Android
    return s;
}

static QString pick(const QString &v, const QStringList &allowed, const QString &fallback)
{
    return allowed.contains(v) ? v : fallback;
}

Settings Settings::load()
{
    Settings s = defaults(Config::isAndroid());
    QSettings q;
    q.beginGroup(QStringLiteral("readaloud"));
    s.language = q.value(QStringLiteral("language"), s.language).toString();
    s.model = q.value(QStringLiteral("model"), s.model).toString();
    s.device = q.value(QStringLiteral("device"), s.device).toString();
    s.voice = pick(q.value(QStringLiteral("voice"), s.voice).toString(), voiceIds(), s.voice);
    s.style = pick(q.value(QStringLiteral("style"), s.style).toString(), styleIds(), s.style);
    s.speed = pick(q.value(QStringLiteral("speed"), s.speed).toString(), speedIds(), s.speed);
    s.barShown = q.value(QStringLiteral("barShown"), s.barShown).toBool();
    s.settingsOpened = q.value(QStringLiteral("settingsOpened"), false).toBool();
    q.endGroup();
    return s;
}

void Settings::save() const
{
    QSettings q;
    q.beginGroup(QStringLiteral("readaloud"));
    q.setValue(QStringLiteral("language"), language);
    q.setValue(QStringLiteral("model"), model);
    q.setValue(QStringLiteral("device"), device);
    q.setValue(QStringLiteral("voice"), voice);
    q.setValue(QStringLiteral("style"), style);
    q.setValue(QStringLiteral("speed"), speed);
    q.setValue(QStringLiteral("barShown"), barShown);
    q.setValue(QStringLiteral("settingsOpened"), settingsOpened);
    q.endGroup();
}

QString Settings::summary() const
{
    return QStringLiteral("%1 · %2 · %3").arg(voiceLabel(voice), styleLabel(style), speedLabel(speed));
}

static int androidForce = -1;
bool Config::isAndroid()
{
    if (androidForce >= 0)
        return androidForce != 0;
#ifdef Q_OS_ANDROID
    return true;
#else
    return false;
#endif
}
void Config::forceAndroid(int on) { androidForce = on; }

Config::Config() { m_s = Settings::load(); }
Config &Config::instance()
{
    static Config c;
    return c;
}
void Config::set(const Settings &s)
{
    m_s = s;
    m_s.save();
    emit changed();
}
void Config::reload()
{
    m_s = Settings::load();
    emit changed();
}

// ================================================================================================ languages and models

bool languageHasModel(const QString &code)
{
    static const QStringList spoken = {QStringLiteral("en"), QStringLiteral("de"), QStringLiteral("es"), QStringLiteral("fr"),
                                       QStringLiteral("pt"), QStringLiteral("it"), QStringLiteral("ru")};
    return spoken.contains(code);
}

QString effectiveLanguage(const Settings &s)
{
    if (!s.language.isEmpty())
        return s.language;
    const QString own = QLocale().name().left(2);
    return languageHasModel(own) ? own : QStringLiteral("en");
}

// THE ONE TABLE of the order of the models, best sounding first (the first that is installed, fits the memory and reaches real
// time becomes the default model). An entry is an id or the beginning of ids. Change the order here and nowhere else.
static const char *const kModelOrder[] = {"pocket-tts-", "qwen3-tts-0.6b-q8", "qwen3-tts-1.7b-q4", "qwen3-tts-1.7b-q8"};

const QStringList &modelOrder()
{
    static const QStringList order = [] {
        QStringList l;
        for (const char *e : kModelOrder)
            l << QString::fromLatin1(e);
        return l;
    }();
    return order;
}

static int rankOf(const QString &id)
{
    const QStringList &o = modelOrder();
    for (int i = 0; i < o.size(); ++i)
        if (id == o[i] || id.startsWith(o[i]))
            return i;
    return int(o.size());
}

QList<const ai::CatalogModel *> modelsFor(const QString &language, const ai::Catalog &catalog)
{
    QList<const ai::CatalogModel *> out;
    for (const ai::CatalogModel &m : catalog.models)
        if (m.is(QStringLiteral("tts")) && (m.languages.contains(language) || m.languages.contains(QStringLiteral("*"))))
            out << &m;
    std::stable_sort(out.begin(), out.end(), [](const ai::CatalogModel *a, const ai::CatalogModel *b) { return rankOf(a->id) < rankOf(b->id); });
    return out;
}

Fit fitOf(const ai::CatalogModel &m, const QString &choice, const QList<ai::Device> &devices)
{
    Fit f;
    f.need = qint64(m.memoryMB) * 1024 * 1024;
    auto size = [](qint64 b) { return QLocale().formattedDataSize(b, 1, QLocale::DataSizeSIFormat); };
    if (choice == QLatin1String("auto") || choice.startsWith(QLatin1String("gpu:"))) {
        const ai::Device d = ai::pickDevice(devices, choice, f.need);
        if (d.gpu && d.free >= f.need) {
            f.fits = true;
            f.where = d.where();
            f.text = tr("fits on %1").arg(d.name);
            return f;
        }
        if (choice.startsWith(QLatin1String("gpu:")) && d.gpu) {
            f.where = d.where();
            f.text = tr("does not fit (needs %1)").arg(size(f.need));
            return f;
        }
    }
    f.where = QStringLiteral("ram");
    const modelmem::DeviceFigures ram = modelmem::ModelMemory::instance().device(QStringLiteral("ram"));
    if (!ram.valid() || f.need <= 0 || ram.available >= f.need) {
        f.fits = true;
        f.text = tr("fits in RAM");
    } else {
        f.text = tr("does not fit (needs %1)").arg(size(f.need));
    }
    return f;
}

static QString speedKey(const QString &id, const QString &device)
{
    QString d = device;
    d.replace(QLatin1Char('/'), QLatin1Char('_')).replace(QLatin1Char(':'), QLatin1Char('_'));
    return QStringLiteral("readaloud/rtf/") + id + QLatin1Char('/') + d;
}
double measuredSpeed(const QString &modelId, const QString &deviceChoice)
{
    return QSettings().value(speedKey(modelId, deviceChoice), 0.0).toDouble();
}
void rememberSpeed(const QString &modelId, const QString &deviceChoice, double ratio)
{
    QSettings().setValue(speedKey(modelId, deviceChoice), ratio);
}

const ai::CatalogModel *resolveModel(const Settings &s, const QList<ai::Device> &devices, bool onlyInstalled,
                                     const ai::Catalog &catalog)
{
    const QString lang = effectiveLanguage(s);
    if (!s.model.isEmpty()) {
        const ai::CatalogModel *m = catalog.model(s.model);
        if (m && (m->languages.contains(lang) || m->languages.contains(QStringLiteral("*"))) && (!onlyInstalled || ai::isInstalled(*m)))
            return m;
    }
    const QList<const ai::CatalogModel *> all = modelsFor(lang, catalog);
    for (int pass = 0; pass < 2; ++pass)
        for (const ai::CatalogModel *m : all) {
            if (onlyInstalled && !ai::isInstalled(*m))
                continue;
            if (!fitOf(*m, s.device, devices).fits)
                continue;
            const double r = measuredSpeed(m->id, s.device);
            if (pass == 0 && r > 1.0)
                continue;   // slower than real time: only when no other model does
            return m;
        }
    return onlyInstalled || all.isEmpty() ? nullptr : all.first();
}

// ================================================================================================ voices and styles

QString pocketVoice(const QString &, const QString &voiceId)
{
    // Pocket TTS's licensed preset voices only (CC BY 4.0 or CC0; the catalog lists what a model has). Never a voice
    // without a stated license.
    static const QHash<QString, QString> table = {
        {QStringLiteral("female1"), QStringLiteral("alba")},   {QStringLiteral("female2"), QStringLiteral("anna")},
        {QStringLiteral("female3"), QStringLiteral("azelma")}, {QStringLiteral("male1"), QStringLiteral("charles")},
        {QStringLiteral("male2"), QStringLiteral("george")},   {QStringLiteral("male3"), QStringLiteral("michael")}};
    return table.value(voiceId);
}

QString qwenClipFile(const QString &voiceId, const QString &styleId)
{
    return ai::modelsRoot() + QStringLiteral("/read-aloud-voices/") + voiceId + QLatin1Char('-') + styleId + QStringLiteral(".wav");
}

static quint32 le32(const QByteArray &b, int p) { return quint32(uchar(b[p])) | quint32(uchar(b[p + 1])) << 8 | quint32(uchar(b[p + 2])) << 16 | quint32(uchar(b[p + 3])) << 24; }
static quint16 le16(const QByteArray &b, int p) { return quint16(uchar(b[p]) | uchar(b[p + 1]) << 8); }

bool readWav(const QString &file, QVector<float> *mono, int *rate)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QByteArray b = f.readAll();
    if (b.size() < 44 || b.mid(0, 4) != "RIFF" || b.mid(8, 4) != "WAVE")
        return false;
    int format = 0, channels = 1, bits = 16, hz = 0;
    int pos = 12;
    while (pos + 8 <= b.size()) {
        const QByteArray id = b.mid(pos, 4);
        const qint64 size = le32(b, pos + 4);
        const int body = pos + 8;
        if (id == "fmt " && body + 16 <= b.size()) {
            format = le16(b, body);
            channels = qMax<int>(1, le16(b, body + 2));
            hz = int(le32(b, body + 4));
            bits = le16(b, body + 14);
        } else if (id == "data") {
            const qint64 n = qMin<qint64>(size, b.size() - body);
            const int bytes = bits / 8;
            if (hz <= 0 || bytes <= 0 || !((format == 1 && bits == 16) || (format == 3 && bits == 32)))
                return false;
            const qint64 frames = n / (bytes * channels);
            QVector<float> out;
            out.resize(int(frames));
            for (qint64 i = 0; i < frames; ++i) {
                const int p = body + int(i) * bytes * channels;
                if (format == 1) {
                    out[int(i)] = float(qint16(le16(b, p))) / 32768.0f;
                } else {
                    const quint32 u = le32(b, p);
                    float v;
                    std::memcpy(&v, &u, 4);
                    out[int(i)] = v;
                }
            }
            if (mono)
                *mono = out;
            if (rate)
                *rate = hz;
            return !out.isEmpty();
        }
        pos = body + int(size) + int(size & 1);
    }
    return false;
}

static bool isPocket(const ai::CatalogModel *m) { return m && m->engine == QLatin1String("pocket-tts"); }
static bool isQwen(const ai::CatalogModel *m) { return m && m->engine == QLatin1String("qwen3-tts"); }

Availability voiceAvailability(const ai::CatalogModel *m, const QString &language, const QString &voiceId, const QString &)
{
    Availability a;
    if (isPocket(m)) {
        const QString preset = pocketVoice(language, voiceId);
        if (preset.isEmpty() || (!m->voices.isEmpty() && !m->voices.contains(preset))) {
            a.ok = false;
            a.reason = tr("This model does not have this voice.");
        }
    } else if (isQwen(m)) {
        if (!QFileInfo::exists(qwenClipFile(voiceId, QStringLiteral("neutral")))) {
            a.ok = false;
            a.reason = tr("Voice not installed yet.");
        }
    }
    return a;
}

Availability styleAvailability(const ai::CatalogModel *m, const QString &, const QString &voiceId, const QString &styleId)
{
    Availability a;
    if (isPocket(m)) {
        a.ok = false;
        a.reason = tr("This model has no style control.");
    } else if (isQwen(m)) {
        if (!QFileInfo::exists(qwenClipFile(voiceId, styleId))) {
            a.ok = false;
            a.reason = tr("Voice not installed yet.");
        }
    }
    return a;
}

bool makeVoice(const ai::CatalogModel &m, const QString &language, const QString &voiceId, const QString &styleId, ai::Voice *out,
               QString *why)
{
    *out = ai::Voice();
    const Availability va = voiceAvailability(&m, language, voiceId, styleId);
    if (!va.ok) {
        if (why)
            *why = va.reason;
        return false;
    }
    if (isPocket(&m)) {
        out->preset = pocketVoice(language, voiceId);
        return true;
    }
    if (isQwen(&m)) {
        QString style = styleId;
        if (!QFileInfo::exists(qwenClipFile(voiceId, style)))
            style = QStringLiteral("neutral");
        const QString file = qwenClipFile(voiceId, style);
        if (!readWav(file, &out->clip, &out->clipRate)) {
            if (why)
                *why = tr("The voice's reference clip cannot be read.");
            return false;
        }
        QFile t(file.left(file.size() - 4) + QStringLiteral(".txt"));
        if (t.open(QIODevice::ReadOnly))
            out->clipText = QString::fromUtf8(t.readAll()).trimmed();
    }
    return true;
}

// ======================================================================================================== sentences

QList<Span> splitSentences(const QString &text, int maxLength)
{
    QList<Span> out;
    const int n = int(text.size());
    auto add = [&](int a, int b) {   // [a, b) trimmed; cut when too long
        while (a < b && text[a].isSpace())
            ++a;
        while (b > a && text[b - 1].isSpace())
            --b;
        while (b - a > maxLength) {
            int cut = -1;
            for (int i = a + maxLength; i > a + maxLength / 2; --i)
                if (text[i - 1] == QLatin1Char(',') || text[i - 1] == QLatin1Char(';') || text[i - 1] == QLatin1Char(':')) {
                    cut = i;
                    break;
                }
            if (cut < 0)
                for (int i = a + maxLength; i > a + maxLength / 2; --i)
                    if (text[i - 1].isSpace()) {
                        cut = i;
                        break;
                    }
            if (cut < 0)
                cut = a + maxLength;
            int e = cut;
            while (e > a && text[e - 1].isSpace())
                --e;
            out.append({a, e - a});
            a = cut;
            while (a < b && text[a].isSpace())
                ++a;
        }
        if (b > a)
            out.append({a, b - a});
    };
    int start = 0;
    int i = 0;
    while (i < n) {
        const QChar c = text[i];
        if (c == QLatin1Char('\n') || c == QChar(0x2029)) {
            add(start, i);
            start = ++i;
            continue;
        }
        if (c == QLatin1Char('.') || c == QLatin1Char('!') || c == QLatin1Char('?') || c == QChar(0x2026)) {
            int j = i + 1;
            while (j < n && (text[j] == QLatin1Char('.') || text[j] == QLatin1Char('!') || text[j] == QLatin1Char('?')))
                ++j;
            while (j < n && (text[j] == QLatin1Char('"') || text[j] == QLatin1Char('\'') || text[j] == QLatin1Char(')') || text[j] == QChar(0x201D)
                             || text[j] == QChar(0x2019) || text[j] == QChar(0x00BB)))
                ++j;
            bool boundary = j >= n || text[j].isSpace();
            if (boundary && j < n && c == QLatin1Char('.')) {
                int k = j;
                while (k < n && text[k].isSpace() && text[k] != QLatin1Char('\n'))
                    ++k;
                // "etc. and", "Dr. smith": a lower-case letter goes on; a word of one or two letters before the dot (Dr, Mr, St) too
                int w = i;
                while (w > start && text[w - 1].isLetter())
                    --w;
                const int wordLen = i - w;
                if (k < n && text[k].isLower())
                    boundary = false;
                else if (wordLen >= 1 && wordLen <= 2 && text[w].isUpper() && k < n && text[k].isUpper() && wordLen == 2)
                    boundary = false;
            }
            if (boundary) {
                add(start, j);
                start = j;
            }
            i = j;
            continue;
        }
        ++i;
    }
    add(start, n);
    return out;
}

// ============================================================================================= where the sound goes

namespace {
// the sound output of the AI engine (libqbai, miniaudio): mono float PCM, only played
class EngineOutput : public Output
{
public:
    ~EngineOutput() override
    {
        if (m_out)
            m_api->output_close(m_out);
    }
    bool init(const qbai_api *api) { m_api = api; return api && api->size >= int(sizeof(qbai_api)) && api->output_open; }
    bool open(int rate, QString *error) override
    {
        m_rate = rate;
        m_out = m_api->output_open(rate, 1);
        if (!m_out) {
            if (error)
                *error = QString::fromUtf8(m_api->last_error());
            return false;
        }
        return true;
    }
    void write(const float *samples, qint64 count) override { if (m_out) m_api->output_write(m_out, samples, count); }
    qint64 playedMs() const override { return m_out && m_rate > 0 ? m_api->output_played(m_out) * 1000 / m_rate : 0; }
    void pause() override { if (m_out) m_api->output_pause(m_out); }
    void resume() override { if (m_out) m_api->output_resume(m_out); }
    void stop() override { if (m_out) m_api->output_stop(m_out); }

private:
    const qbai_api *m_api = nullptr;
    qbai_output *m_out = nullptr;
    int m_rate = 0;
};
} // namespace

Output *makeOutput(QString *why)
{
    ai::Engine &e = ai::Engine::instance();
    if (!e.ensureLoaded(why))
        return nullptr;
    auto *o = new EngineOutput;
    if (!o->init(e.apiFor(QBAI_API_VERSION))) {
        delete o;
        if (why)
            *why = tr("The installed AI engine cannot play sound; install its newest version.");
        return nullptr;
    }
    return o;
}

// ============================================================================================================ reading

LookAhead lookAheadFor(double ratio)
{
    LookAhead l;
    if (ratio > 1.0) {
        l.ahead = 4;
        l.prebuffer = 3;
    }
    return l;
}

static QString testSentence(const QString &lang)
{
    static const QHash<QString, QString> t = {
        {QStringLiteral("en"), QStringLiteral("The quick brown fox jumps over the lazy dog.")},
        {QStringLiteral("de"), QStringLiteral("Der schnelle braune Fuchs springt über den faulen Hund.")},
        {QStringLiteral("es"), QStringLiteral("El rápido zorro marrón salta sobre el perro perezoso.")},
        {QStringLiteral("fr"), QStringLiteral("Le rapide renard brun saute par-dessus le chien paresseux.")},
        {QStringLiteral("pt"), QStringLiteral("A rápida raposa marrom pula sobre o cão preguiçoso.")},
        {QStringLiteral("it"), QStringLiteral("La rapida volpe marrone salta sopra il cane pigro.")},
        {QStringLiteral("ru"), QStringLiteral("Быстрая коричневая лиса прыгает через ленивую собаку.")}};
    return t.value(lang, t.value(QStringLiteral("en")));
}

static QHash<QString, ai::Model *> &models()
{
    static QHash<QString, ai::Model *> m;
    return m;
}

ai::Model *Session::modelFor(const QString &catalogId, modelmem::ModelMemory *memory, bool create)
{
    const QString id = QStringLiteral("read-aloud:") + catalogId;
    ai::Model *m = models().value(id);
    if (!m && create) {
        m = new ai::Model(id, memory);
        models().insert(id, m);
    }
    return m;
}

void Session::forgetModels()
{
    for (ai::Model *m : std::as_const(models())) {
        m->unload();
        delete m;
    }
    models().clear();
}

// The worker thread: loads the model (when it is not loaded) and makes the sentences one after the other, no more than `ahead`
// before the one that plays.
class Session::Worker : public QThread
{
public:
    ai::Model *model = nullptr;
    ai::ModelFiles files;
    QString device, language;
    ai::Voice voice;
    double speed = 1.0;
    QStringList texts;
    int ahead = 2;
    bool probe = false;
    std::atomic_bool cancel{false};
    std::atomic<int> consumed{0};
    std::function<void(int, const QVector<float> &, int)> ready;     // called on the worker thread
    std::function<void(bool, const QString &, double)> done;

    void wake()
    {
        QMutexLocker g(&m_mutex);
        m_cond.wakeAll();
    }

protected:
    void run() override
    {
        modelmem::ModelMemory::lowerThreadPriority();
        QString err;
        const ai::ModelFiles cur = model->files();
        if (!model->isLoaded() || cur.model != files.model || cur.extra != files.extra) {
            if (!model->load(files, device, &err)) {
                done(false, err, 0);
                return;
            }
        }
        if (cancel.load()) {
            done(true, QString(), 0);
            return;
        }
        const int rate = model->sampleRate();
        double ratio = 0;
        for (int i = 0; i < int(texts.size()) && !cancel.load(); ++i) {
            {
                QMutexLocker g(&m_mutex);
                while (!cancel.load() && !probe && i >= consumed.load() + ahead)
                    m_cond.wait(&m_mutex, 100);
            }
            if (cancel.load())
                break;
            QVector<float> pcm;
            QElapsedTimer t;
            t.start();
            if (!model->synthesize(texts[i], language, voice, nullptr, &cancel, &pcm, &err)) {
                if (cancel.load())
                    break;
                done(false, err, 0);
                return;
            }
            const double work = double(t.nsecsElapsed()) / 1e9;
            if (probe) {
                ratio = pcm.isEmpty() ? 0 : work / (double(pcm.size()) / double(rate));
                break;
            }
            if (!qFuzzyCompare(speed, 1.0)) {
                QVector<float> out;
                if (!ai::stretch(pcm, rate, speed, &out, &err)) {
                    done(false, err, 0);
                    return;
                }
                pcm = out;
            }
            ready(i, pcm, rate);
        }
        done(true, QString(), ratio);
    }

private:
    QMutex m_mutex;
    QWaitCondition m_cond;
};

Session &Session::shared()
{
    static Session *s = new Session(nullptr, QCoreApplication::instance());
    return *s;
}

Session::Session(modelmem::ModelMemory *memory, QObject *parent) : QObject(parent), m_memory(memory)
{
    if (!m_memory)
        m_memory = &modelmem::ModelMemory::instance();
    m_timer = new QTimer(this);
    m_timer->setInterval(40);
    connect(m_timer, &QTimer::timeout, this, &Session::poll);
}

Session::~Session()
{
    stop();
    // a worker that still loads waits for the main thread: keep it going while we wait
    for (QThread *t : findChildren<QThread *>()) {
        auto *w = static_cast<Worker *>(t);
        w->cancel = true;
        w->wake();
        while (!w->wait(10))
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void Session::setState(State s)
{
    if (s == m_state)
        return;
    m_state = s;
    emit stateChanged(s);
}

void Session::dropWorker()
{
    if (!m_worker)
        return;
    m_worker->cancel = true;
    m_worker->wake();
    connect(m_worker, &QThread::finished, m_worker, &QObject::deleteLater);
    m_worker = nullptr;
}

bool Session::start(const QString &text, int offset, const Settings &s, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    if (m_state != Idle || m_probe)
        return fail(tr("Read Aloud is busy."));
    ai::Engine &engine = ai::Engine::instance();
    QString why;
    if (!engine.ensureLoaded(&why))
        return fail(why.isEmpty() ? tr("The AI engine is not installed.") : why);
    const QList<ai::Device> devices = engine.devices();
    const ai::CatalogModel *cm = resolveModel(s, devices, true);
    if (!cm)
        return fail(tr("No speech model is installed for this language."));
    const QString lang = effectiveLanguage(s);
    ai::Voice voice;
    if (!makeVoice(*cm, lang, s.voice, s.style, &voice, &why))
        return fail(why);
    const QList<Span> spans = splitSentences(text);
    if (spans.isEmpty())
        return fail(tr("There is no text to read."));
    QString outWhy;
    Output *out = makeOutput(&outWhy);
    if (!out)
        return fail(outWhy);

    m_settings = s;
    m_spans.clear();
    QStringList texts;
    for (const Span &sp : spans) {
        m_spans.append({sp.start + offset, sp.length});
        texts << text.mid(sp.start, sp.length);
    }
    m_offset = offset;
    m_out = out;
    m_rate = 0;
    m_endMs.clear();
    m_pending.clear();
    m_written = 0;
    m_playing = -1;
    m_generated = 0;
    m_maxAhead = 0;
    m_workerDone = false;
    m_started = false;
    m_look = lookAheadFor(measuredSpeed(cm->id, s.device));

    auto *w = new Worker;
    w->setParent(this);
    w->model = modelFor(cm->id, m_memory, true);
    w->files = ai::modelFiles(*cm);
    w->device = s.device;
    w->language = lang;
    w->voice = voice;
    w->speed = speedFactor(s.speed);
    w->texts = texts;
    w->ahead = m_look.ahead;
    m_worker = w;
    QPointer<Worker> guard(w);
    w->ready = [this, guard](int i, const QVector<float> &pcm, int rate) {
        QMetaObject::invokeMethod(this, [this, guard, i, pcm, rate] {
            if (guard && guard == m_worker)
                onReady(i, pcm, rate);
        }, Qt::QueuedConnection);
    };
    w->done = [this, guard](bool ok, const QString &err, double) {
        QMetaObject::invokeMethod(this, [this, guard, ok, err] {
            if (guard && guard == m_worker)
                onWorkerDone(ok, err);
        }, Qt::QueuedConnection);
    };
    setState(Loading);
    m_timer->start();
    w->start();
    return true;
}

void Session::onReady(int index, const QVector<float> &pcm, int rate)
{
    if (!m_out || m_state == Idle)
        return;
    ++m_generated;
    m_maxAhead = qMax(m_maxAhead, index - (m_worker ? m_worker->consumed.load() : 0) + 1);
    if (m_rate == 0) {
        m_rate = rate;
        QString err;
        if (!m_out->open(rate, &err)) {
            const QString msg = err;
            stop();
            emit failed(msg);
            return;
        }
    }
    Ready r;
    r.index = index;
    r.pcm = pcm;
    m_pending.append(r);
    flushPending();
}

void Session::flushPending()
{
    const int total = int(m_spans.size());
    const bool ready = int(m_pending.size()) + m_written >= qMin(m_look.prebuffer, total) || m_workerDone;
    if (!m_started && !ready)
        return;
    while (!m_pending.isEmpty()) {
        const Ready r = m_pending.takeFirst();
        m_out->write(r.pcm.constData(), r.pcm.size());
        const qint64 prev = m_endMs.isEmpty() ? 0 : m_endMs.last();
        m_endMs.append(prev + qint64(std::llround(1000.0 * double(r.pcm.size()) / double(m_rate))));
        ++m_written;
    }
    if (!m_started) {
        m_started = true;
        if (m_state == Loading)
            setState(Playing);
        poll();
    }
}

void Session::onWorkerDone(bool ok, const QString &error)
{
    m_workerDone = true;
    if (!ok) {
        stop();
        emit failed(error);
        return;
    }
    flushPending();
    if (m_written == 0 && m_pending.isEmpty() && m_state != Idle)
        endReading(true);
}

void Session::poll()
{
    if (!m_out || m_state == Idle)
        return;
    if (m_state != Playing && m_state != Paused)
        return;
    const qint64 p = m_out->playedMs();
    int k = -1;
    for (int i = 0; i < int(m_endMs.size()); ++i)
        if (p + 30 < m_endMs[i]) {   // (a few ms of rounding between the output's clock and the lengths)
            k = i;
            break;
        }
    const bool underrun = k < 0;
    if (k < 0) {
        if (m_workerDone && m_written >= int(m_spans.size())) {
            endReading(true);
            return;
        }
        k = m_playing < 0 ? 0 : m_playing;   // waiting for the next sentence: the last one stays marked
        if (m_endMs.isEmpty())
            return;
    }
    // the worker may make sentences up to `ahead` past the one that plays (all written sentences are played: the next one)
    if (m_worker) {
        const int consumed = underrun ? int(m_endMs.size()) : k;
        if (m_worker->consumed.load() != consumed) {
            m_worker->consumed = consumed;
            m_worker->wake();
        }
    }
    if (k != m_playing) {
        m_playing = k;
        emit sentence(m_spans[k].start, m_spans[k].length);
    }
}

void Session::pause()
{
    if (m_state != Playing || !m_out)
        return;
    m_out->pause();
    setState(Paused);
}

void Session::resume()
{
    if (m_state != Paused || !m_out)
        return;
    m_out->resume();
    setState(Playing);
}

void Session::endReading(bool completed)
{
    m_timer->stop();
    dropWorker();
    if (m_out) {
        m_out->stop();
        delete m_out;
        m_out = nullptr;
    }
    m_pending.clear();
    const bool was = m_state != Idle;
    setState(Idle);
    emit sentence(-1, 0);
    if (was)
        emit finished(completed);
}

void Session::stop()
{
    if (m_state == Idle && !m_out && !m_worker)
        return;
    endReading(false);
}

bool Session::measureSpeed(const Settings &s, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    if (m_state != Idle || m_probe)
        return fail(tr("Read Aloud is busy."));
    ai::Engine &engine = ai::Engine::instance();
    QString why;
    if (!engine.ensureLoaded(&why))
        return fail(why.isEmpty() ? tr("The AI engine is not installed.") : why);
    const ai::CatalogModel *cm = resolveModel(s, engine.devices(), true);
    if (!cm)
        return fail(tr("No speech model is installed for this language."));
    const QString lang = effectiveLanguage(s);
    ai::Voice voice;
    if (!makeVoice(*cm, lang, s.voice, s.style, &voice, &why))
        return fail(why);
    auto *w = new Worker;
    w->setParent(this);
    w->probe = true;
    w->model = modelFor(cm->id, m_memory, true);
    w->files = ai::modelFiles(*cm);
    w->device = s.device;
    w->language = lang;
    w->voice = voice;
    w->texts = {testSentence(lang)};
    m_probe = w;
    const QString id = cm->id, device = s.device;
    QPointer<Worker> guard(w);
    w->ready = [](int, const QVector<float> &, int) {};
    w->done = [this, guard, id, device](bool ok, const QString &err, double ratio) {
        QMetaObject::invokeMethod(this, [this, guard, id, device, ok, err, ratio] {
            if (!guard)
                return;
            m_probe = nullptr;
            connect(guard.data(), &QThread::finished, guard.data(), &QObject::deleteLater);
            if (ok && ratio > 0)
                rememberSpeed(id, device, ratio);
            emit speedMeasured(id, ok ? ratio : 0.0, ok ? QString() : err);
        }, Qt::QueuedConnection);
    };
    w->start();
    return true;
}

} // namespace readaloud
} // namespace qb
