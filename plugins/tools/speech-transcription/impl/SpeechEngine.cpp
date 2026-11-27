// The engine of the add-on "Speech Transcription" (libqb-speech-impl.so): the microphone
// through Qt Multimedia, pieces between pauses (SpeechLogic), and the recognition through the program's AI engine (tool
// interface 1.3: ToolHost::aiEngine, aiPickDevice and the registry of loaded models; 1.10 of the add-on has no whisper.cpp of
// its own) in a thread of its own. Loaded by the writing tool (libqb-speech.so) when it is first needed; see SpeechApi.h.
#include "../SpeechApi.h"
#include "core/TestSwitches.h"
#include "../SpeechLogic.h"
#include "core/AudioBackend.h"
#include "tools/QbAi.h"
#include "tools/ToolApi.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMediaDevices>
#include <QMutex>
#include <QPermissions>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QtEndian>

#include <atomic>
#include <cmath>
#include <cstring>

Q_DECLARE_METATYPE(speech::EngineSettings)

namespace {

using speech::EngineSettings;

// A path as the AI engine takes it: on Windows it reads char paths as UTF-8 (a folder with letters such as "ue" in the
// writer's name); elsewhere the file system's encoding.
QByteArray pathBytes(const QString &path)
{
#ifdef Q_OS_WIN
    return QDir::toNativeSeparators(path).toUtf8();
#else
    return QFile::encodeName(path);
#endif
}

// a WAV file (PCM 8/16/24/32 bit or float) as 16 kHz mono
bool readWav(const QString &file, QVector<float> *out, QString *error)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        *error = QObject::tr("Cannot read %1").arg(file);
        return false;
    }
    const QByteArray d = f.readAll();
    if (d.size() < 44 || !d.startsWith("RIFF") || d.mid(8, 4) != "WAVE") {
        *error = QObject::tr("%1 is no WAV file").arg(file);
        return false;
    }
    int pos = 12, fmt = 0, channels = 0, rate = 0, bits = 0;
    QByteArray data;
    while (pos + 8 <= d.size()) {
        const QByteArray id = d.mid(pos, 4);
        const int len = int(qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(d.constData()) + pos + 4));
        const auto *p = reinterpret_cast<const uchar *>(d.constData()) + pos + 8;
        if (id == "fmt ") {
            fmt = qFromLittleEndian<quint16>(p);
            channels = qFromLittleEndian<quint16>(p + 2);
            rate = int(qFromLittleEndian<quint32>(p + 4));
            bits = qFromLittleEndian<quint16>(p + 14);
            if (fmt == 0xfffe && len >= 26) // extensible: the sub-format's first two bytes
                fmt = qFromLittleEndian<quint16>(p + 24);
        } else if (id == "data") {
            data = d.mid(pos + 8, qMin(len, int(d.size()) - pos - 8));
        }
        pos += 8 + len + (len & 1);
    }
    if (channels <= 0 || rate <= 0 || data.isEmpty() || (fmt != 1 && fmt != 3)) {
        *error = QObject::tr("%1: a WAV format that is not read here").arg(file);
        return false;
    }
    const int bytes = bits / 8;
    const int frames = int(data.size()) / (bytes * channels);
    QVector<float> inter(frames * channels);
    const auto *p = reinterpret_cast<const uchar *>(data.constData());
    for (int i = 0; i < frames * channels; ++i, p += bytes) {
        float v = 0;
        if (fmt == 3 && bits == 32)
            v = qFromLittleEndian<float>(p);
        else if (bits == 16)
            v = qFromLittleEndian<qint16>(p) / 32768.f;
        else if (bits == 8)
            v = (int(*p) - 128) / 128.f;
        else if (bits == 24)
            v = float(qint32(quint32(p[0]) << 8 | quint32(p[1]) << 16 | quint32(p[2]) << 24) >> 8) / 8388608.f;
        else if (bits == 32)
            v = qFromLittleEndian<qint32>(p) / 2147483648.f;
        inter[i] = v;
    }
    *out = speech::toMono16k(inter.constData(), frames, channels, rate);
    return true;
}

// The model loaded in the program's AI engine. One per add-on: the live pieces, the model check and the speed test
// share it. Its mutex is held while it is loaded or used; the program's request to unload (the main thread) only tries
// the mutex, so that it never waits for a job that waits for the program.
class Holder
{
public:
    QMutex mutex;
    qb::tools::ToolHost *host = nullptr;

    static QString registryId(const EngineSettings &s)
    {
        return QStringLiteral("speech-transcription:") + (s.modelId.isEmpty() ? QStringLiteral("file") : s.modelId);
    }
    bool loaded() const { return m_model != nullptr; }
    QString loadedId() const { return m_id; }
    QString engineVersion() const
    {
        const qbai_api *api = host ? host->aiEngine(QBAI_API_VERSION) : nullptr;
        return api && api->engine_version ? QString::fromUtf8(api->engine_version()) : QString();
    }

    // The mutex is held. Loads the model for these settings (or keeps the one that is loaded).
    bool ensure(const EngineSettings &s, QString *error)
    {
        const QString id = registryId(s);
        // the file's size and time count too: a model checked under a name that an earlier file had is another model
        const QFileInfo info(s.modelFile);
        const QString stamp = QStringLiteral("%1/%2").arg(info.size()).arg(info.lastModified().toMSecsSinceEpoch());
        if (m_model && m_file == s.modelFile && m_stamp == stamp && m_device == s.device && m_id == id) {
            host->modelUsed(id);
            return true;
        }
        drop(true);
        auto fail = [&](const QString &e) {
            if (error)
                *error = e;
            return false;
        };
        const qbai_api *api = host ? host->aiEngine(QBAI_API_VERSION) : nullptr;
        if (!api)
            return fail(QObject::tr("The AI engine is not installed (or could not be loaded). Install it under File > "
                                    "Options > Speech."));
        const QByteArray path = pathBytes(s.modelFile);
        qint64 bytes = api->model_memory ? api->model_memory(QBAI_ASR_WHISPER, path.constData(), nullptr, 0) : 0;
        if (bytes <= 0)
            bytes = QFileInfo(s.modelFile).size();
        QString where;
        const int device = host->aiPickDevice(s.device, bytes, &where);
        if (device < 0)
            return fail(QObject::tr("The AI engine is not installed (or could not be loaded). Install it under File > "
                                    "Options > Speech."));
        host->modelMessage(true);
        const qb::tools::ModelDecision d = host->requestModelLoad(id, QObject::tr("Speech Transcription"), bytes, where);
        if (!d.allowed) {
            host->modelMessage(false);
            return fail(d.reason.isEmpty() ? QObject::tr("The model was not loaded: the memory is needed elsewhere.") : d.reason);
        }
        qbai_load_params lp;
        memset(&lp, 0, sizeof lp);
        lp.size = sizeof lp;
        lp.kind = QBAI_ASR_WHISPER;
        lp.model_path = path.constData();
        lp.device = device;
        lp.threads = host->aiThreads();
        qbai_model *m = api->load(&lp);
        if (!m) {
            const QString detail = QString::fromUtf8(api->last_error ? api->last_error() : "");
            host->modelUnloaded(id);
            host->modelMessage(false);
            return fail((device > 0 ? QObject::tr("The model could not be loaded onto the graphics card. Choose the processor "
                                                  "under Options > Speech > Processing, or a smaller model.")
                                    : QObject::tr("The model could not be loaded (damaged file, or not enough memory)."))
                        + (detail.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(detail)));
        }
        host->modelLoaded(id);
        host->modelMessage(false);
        m_api = api;
        m_model = m;
        m_file = s.modelFile;
        m_stamp = stamp;
        m_device = s.device;
        m_id = id;
        return true;
    }

    // The mutex is held. tell: the registry is told (not when the program itself asked for the unload).
    bool drop(bool tell)
    {
        if (!m_model)
            return false;
        if (tell)
            host->modelMessage(true);
        m_api->unload(m_model);
        m_model = nullptr;
        if (tell) {
            host->modelUnloaded(m_id);
            host->modelMessage(false);
        }
        m_file.clear();
        m_id.clear();
        return true;
    }

    // One run of the model over a piece. `language` empty: the engine finds the language.
    QString run(const EngineSettings &s, const QVector<float> &audio, const QString &language, bool translate,
                const QString &context, QString *error)
    {
        if (!m_model)
            return QString();
        host->modelUsed(m_id);
        QString prompt = s.hint.simplified();
        if (s.useContext && !context.isEmpty())
            prompt = (prompt.isEmpty() ? QString() : prompt + QLatin1Char(' ')) + context.right(200);
        const QByteArray lang = language.isEmpty() ? QByteArrayLiteral("auto") : language.toLatin1();
        const QByteArray promptBytes = prompt.toUtf8();
        qbai_transcribe_params tp;
        memset(&tp, 0, sizeof tp);
        tp.size = sizeof tp;
        tp.language = lang.constData();
        tp.translate = translate ? 1 : 0;
        tp.prompt = promptBytes.isEmpty() ? nullptr : promptBytes.constData();
        char *text = nullptr;
        const int rc = m_api->transcribe(m_model, audio.constData(), audio.size(), &tp, &text);
        QString out = text ? QString::fromUtf8(text) : QString();
        if (text)
            m_api->free_text(text);
        if (rc != QBAI_OK) {
            if (error)
                *error = QObject::tr("The AI engine could not transcribe a piece.");
            return QString();
        }
        return speech::cleanText(out, s.silenceGuard);
    }

private:
    const qbai_api *m_api = nullptr;
    qbai_model *m_model = nullptr;
    QString m_file, m_stamp, m_device, m_id;
};

// ------------------------------------------------------------------ the thread with the model

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(Holder *holder) : m_h(holder) {}

public slots:
    void prepare(const speech::EngineSettings &s)
    {
        m_s = s;
        m_pieces = 0;
        // tests: QB_SPEECH_FAKE writes "Piece <n> (<seconds> s)" for each piece instead of running
        // the model (the pieces, the queue and the file can be checked without a real model)
        if (QB_TEST_ENV_SET("QB_SPEECH_FAKE")) {
            m_fake = true;
            emit ready();
            return;
        }
        m_fake = false;
        emit state(QStringLiteral("loading"));
        QString error;
        {
            QMutexLocker l(&m_h->mutex);
            if (!m_h->ensure(s, &error)) {
                emit failed(error);
                return;
            }
        }
        m_context.clear();
        emit ready();
    }

    void process(const QVector<float> &audio, bool newParagraph)
    {
        ++m_pieces;
        if (m_fake) {
            QThread::msleep(50);
            emit done(QStringLiteral("Piece %1 (%2 s)").arg(m_pieces).arg(audio.size() / 16000.0, 0, 'f', 1), newParagraph);
            return;
        }
        QMutexLocker l(&m_h->mutex);
        if (!m_h->loaded()) {
            emit done(QString(), newParagraph);
            return;
        }
        const bool translate = speech::canTranslate(m_s.mainLanguage, m_s.fallbackLanguage, m_s.modelTranslates);
        const bool two = !m_s.fallbackLanguage.isEmpty() && m_s.fallbackLanguage != m_s.mainLanguage;
        // Into English: the main language as the spoken one, and the model translates (one run per piece). Two languages
        // without a translation: the engine finds the language of each piece (it cannot be limited to the two).
        QString language = m_s.mainLanguage;
        if (two && !translate)
            language.clear();
        QString error;
        QString text = m_h->run(m_s, audio, language, translate, m_context, &error);
        if (!error.isEmpty())
            emit failed(error);
        if (translate && two && m_s.keepOriginal && !text.isEmpty()) {
            const QString original = m_h->run(m_s, audio, QString(), false, QString(), nullptr);
            if (!original.isEmpty() && original != text)
                text += QStringLiteral(" [%1: %2]").arg(m_s.fallbackLanguage, original);
        }
        if (!text.isEmpty())
            m_context += QLatin1Char(' ') + text;
        emit done(text, newParagraph);
    }

signals:
    void state(const QString &s);
    void ready();
    void failed(const QString &message);
    void done(const QString &text, bool newParagraph);

private:
    Holder *m_h;
    speech::EngineSettings m_s;
    QString m_context;
    int m_pieces = 0;
    bool m_fake = false;
};

// ------------------------------------------------------------------ the engine

class Engine : public QObject, public speech::EngineInterface
{
    Q_OBJECT
public:
    Engine()
    {
        qRegisterMetaType<speech::EngineSettings>("speech::EngineSettings");
        qRegisterMetaType<QVector<float>>("QVector<float>");
        m_worker = new Worker(&m_holder);
        m_worker->moveToThread(&m_thread);
        connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
        connect(m_worker, &Worker::state, this, [this](const QString &s) { report(s); });
        connect(m_worker, &Worker::failed, this, [this](const QString &m) {
            if (onError)
                onError(m);
            if (!m_modelReady) { // the model never came: nothing more to do
                stopAudio();
                m_feedTimer.stop();
                m_recording = false;
                setKeep(false);
                m_fileFast = false;
                m_waiting = 0;
                m_busy = false;
                report(QStringLiteral("finished"));
            }
        });
        connect(m_worker, &Worker::ready, this, [this] {
            m_modelReady = true;
            report(m_recording ? activeState() : QStringLiteral("transcribing %1").arg(m_waiting));
        });
        connect(m_worker, &Worker::done, this, &Engine::pieceDone);
        m_thread.setObjectName(QStringLiteral("speech-transcription"));
        m_thread.start();
        m_feedTimer.setInterval(100);
        connect(&m_feedTimer, &QTimer::timeout, this, &Engine::feedFromFile);
    }

    ~Engine() override
    {
        stopAudio();
        setKeep(false);
        m_thread.quit();
        m_thread.wait();
        QMutexLocker l(&m_holder.mutex);
        m_holder.drop(false);
    }

    void setHost(qb::tools::ToolHost *host) override
    {
        m_holder.host = host;
        m_host = host;
        // the worker's computations start from its thread and inherit its priority (tool interface 1.3)
        QMetaObject::invokeMethod(m_worker, [host] { host->lowerThreadPriority(); }, Qt::QueuedConnection);
    }

    QList<speech::AudioInput> audioInputs() override
    {
        QList<speech::AudioInput> out;
        const QAudioDevice def = QMediaDevices::defaultAudioInput();
        for (const QAudioDevice &d : QMediaDevices::audioInputs())
            out << speech::AudioInput{d.id(), d.description(), d.id() == def.id()};
        return out;
    }

    speech::TestResult transcribeFile(const QString &wavFile, const EngineSettings &s, bool translate) override
    {
        speech::TestResult r;
        QVector<float> audio;
        if (!readWav(wavFile, &audio, &r.error))
            return r;
        r.audioSeconds = audio.size() / 16000.0;
        QElapsedTimer t;
        t.start();
        QMutexLocker l(&m_holder.mutex);
        if (!m_holder.ensure(s, &r.error))
            return r;
        r.loadSeconds = t.elapsed() / 1000.0;
        r.modelType = QStringLiteral("whisper");
        t.restart();
        r.text = m_holder.run(s, audio, translate ? QString() : QStringLiteral("en"), translate, QString(), &r.error);
        r.seconds = t.elapsed() / 1000.0;
        r.ok = r.error.isEmpty();
        return r;
    }

    bool start(const EngineSettings &s, QString *error, const QString &wavFile) override
    {
        if (m_busy) {
            if (error)
                *error = tr("A transcription is still running.");
            return false;
        }
        if (s.recordOnly)
            return startRecordOnly(s, error, wavFile);
        if (!QFileInfo::exists(s.modelFile)) {
            if (error)
                *error = tr("No model is installed.");
            return false;
        }
        m_s = s;
        speech::Segmenter::Params sp;
        sp.sensitivity = s.sensitivity;
        sp.pauseMs = s.pauseMs;
        sp.paragraphMs = s.paragraphMs;
        m_seg.reset(new speech::Segmenter(sp));
        m_waiting = 0;
        m_modelReady = false;
        m_levelClock.start();
        if (!wavFile.isEmpty()) {
            QString e;
            if (!readWav(wavFile, &m_fileAudio, &e)) {
                if (error)
                    *error = e;
                return false;
            }
            m_filePos = 0;
            // a recording transcribed later: as fast as the model takes the pieces (feedFromFile
            // waits while pieces are queued); tests: faster than real time
            m_fileFast = s.fileAsFast;
            m_feedTimer.setInterval(s.fileAsFast ? 0 : QB_TEST_ENV_SET("QB_SPEECH_FAST") ? 5 : 100);
            m_feedTimer.start();
        } else if (!startAudio(s.audioInput, error)) {
            return false;
        }
        m_recording = true;
        setKeep(true);
        m_busy = true;
        QMetaObject::invokeMethod(m_worker, "prepare", Qt::QueuedConnection, Q_ARG(speech::EngineSettings, s));
        return true;
    }

    void stop() override
    {
        if (!m_recording)
            return;
        if (m_writer) {
            // record only: the file is complete; nothing else to wait for
            stopAudio();
            m_feedTimer.stop();
            m_recording = false;
            setKeep(false);
            m_writer->close();
            m_writer.reset();
            m_busy = false;
            report(QStringLiteral("finished"));
            return;
        }
        stopAudio();
        m_feedTimer.stop();
        m_recording = false;
        setKeep(false);
        if (m_seg) {
            m_seg->flush();
            sendPieces();
        }
        if (m_waiting == 0)
            finish();
        else
            report(QStringLiteral("transcribing %1").arg(m_waiting));
    }

    bool isRecording() const override { return m_recording; }
    bool isBusy() const override { return m_busy; }
    int waiting() const override { return m_waiting; }

    void unloadModel() override
    {
        if (m_busy)
            return;
        if (m_holder.mutex.tryLock()) {
            m_holder.drop(true);
            m_holder.mutex.unlock();
        }
    }
    // the program's rule asks (ToolModels::unloadModel, the main thread): never waits for the worker
    bool unloadModelOfId(const QString &registryId) override
    {
        if (!m_holder.mutex.tryLock())
            return false;
        bool ok = true;
        if (m_holder.loaded() && m_holder.loadedId() == registryId) {
            if (m_busy)
                ok = false;
            else
                m_holder.drop(false);
        }
        m_holder.mutex.unlock();
        return ok;
    }

    QString version() const override { return m_holder.engineVersion(); }

private:
    bool startAudio(const QByteArray &id, QString *error)
    {
        // Qt's microphone permission: Linux and Windows answer "granted"; Android asks the writer the first time (the
        // answer comes later, so this recording does not start: press Record again)
        const QMicrophonePermission permission;
        switch (QCoreApplication::instance()->checkPermission(permission)) {
        case Qt::PermissionStatus::Granted:
            break;
        case Qt::PermissionStatus::Undetermined:
            QCoreApplication::instance()->requestPermission(permission, this, [](const QPermission &) {});
            if (error)
                *error = tr("Quillbinder asks for permission to use the microphone. Allow it, then press Record again.");
            return false;
        case Qt::PermissionStatus::Denied:
            if (error)
                *error = tr("The microphone is not allowed for Quillbinder. Allow it in the system's settings.");
            return false;
        }
        QAudioDevice dev = QMediaDevices::defaultAudioInput();
        for (const QAudioDevice &d : QMediaDevices::audioInputs())
            if (!id.isEmpty() && d.id() == id)
                dev = d;
        if (dev.isNull()) {
            if (error)
                *error = tr("No microphone was found.");
            return false;
        }
        QAudioFormat f;
        f.setSampleRate(16000);
        f.setChannelCount(1);
        f.setSampleFormat(QAudioFormat::Float);
        if (!dev.isFormatSupported(f))
            f = dev.preferredFormat();
        m_format = f;
        m_source = new QAudioSource(dev, f, this);
        m_source->setBufferSize(f.bytesForDuration(200000));
        m_io = m_source->start();
        if (!m_io || m_source->error() != QAudio::NoError) {
            if (error)
                *error = tr("The microphone \"%1\" could not be opened.").arg(dev.description());
            stopAudio();
            return false;
        }
        connect(m_io, &QIODevice::readyRead, this, &Engine::readAudio);
        return true;
    }

    void stopAudio()
    {
        if (m_source) {
            m_source->stop();
            m_source->deleteLater();
        }
        m_source = nullptr;
        m_io = nullptr;
    }

    void readAudio()
    {
        if (!m_io)
            return;
        const QByteArray d = m_io->readAll();
        const int bpf = m_format.bytesPerFrame();
        if (bpf <= 0)
            return;
        const int frames = int(d.size()) / bpf;
        const int ch = m_format.channelCount();
        QVector<float> inter(frames * ch);
        const char *p = d.constData();
        for (int i = 0; i < frames * ch; ++i)
            inter[i] = m_format.normalizedSampleValue(p + i * m_format.bytesPerSample());
        const QVector<float> mono = speech::toMono16k(inter.constData(), frames, ch, m_format.sampleRate());
        feed(mono);
    }

    void feedFromFile()
    {
        const int step = 1600; // 100 ms
        if (m_filePos >= m_fileAudio.size()) {
            m_feedTimer.stop();
            stop();
            return;
        }
        // a recording read as fast as possible: at most two pieces wait for the model, so the
        // memory stays small however long the recording is
        if (m_fileFast && m_waiting >= 2) {
            m_feedTimer.stop(); // pieceDone starts it again: no busy waiting
            return;
        }
        for (int round = 0; round < (m_fileFast ? 50 : 1) && m_filePos < m_fileAudio.size(); ++round) {
            const int n = int(qMin<qsizetype>(step, m_fileAudio.size() - m_filePos));
            feed(QVector<float>(m_fileAudio.constData() + m_filePos, m_fileAudio.constData() + m_filePos + n));
            m_filePos += n;
            if (m_fileFast && m_waiting >= 2)
                break;
        }
        if (m_fileFast && m_modelReady) {
            const int pct = int(100 * m_filePos / qMax<qsizetype>(1, m_fileAudio.size()));
            if (pct != m_lastPct) {
                m_lastPct = pct;
                report(QStringLiteral("reading %1").arg(pct));
            }
        }
    }

    // Record only (0.64): the microphone (or, for the tests, a WAV file in real time) into a WAV file
    bool startRecordOnly(const EngineSettings &s, QString *error, const QString &wavFile)
    {
        m_s = s;
        m_levelClock.start();
        m_writer.reset(new speech::WavWriter);
        if (!m_writer->open(s.recordFile, error)) {
            m_writer.reset();
            return false;
        }
        if (!wavFile.isEmpty()) {
            QString e;
            if (!readWav(wavFile, &m_fileAudio, &e)) {
                if (error)
                    *error = e;
                m_writer->close();
                m_writer.reset();
                QFile::remove(s.recordFile);
                return false;
            }
            m_filePos = 0;
            m_fileFast = false;
            m_feedTimer.setInterval(QB_TEST_ENV_SET("QB_SPEECH_FAST") ? 5 : 100);
            m_feedTimer.start();
        } else if (!startAudio(s.audioInput, error)) {
            m_writer->close();
            m_writer.reset();
            QFile::remove(s.recordFile);
            return false;
        }
        m_recording = true;
        setKeep(true);
        m_busy = true;
        report(QStringLiteral("recording"));
        return true;
    }

    void feed(const QVector<float> &mono)
    {
        if (m_writer) {
            m_writer->write(mono.constData(), int(mono.size()));
            if (onLevel && m_levelClock.isValid() && m_levelClock.elapsed() >= 50 && !mono.isEmpty()) {
                m_levelClock.restart();
                double sum = 0;
                for (float v : mono)
                    sum += double(v) * v;
                const float db = 20.f * std::log10(float(std::sqrt(sum / mono.size())) + 1e-9f);
                onLevel(qBound(0.f, (db + 60.f) / 60.f, 1.f));
            }
            return;
        }
        if (!m_seg)
            return;
        m_seg->feed(mono.constData(), int(mono.size()));
        if (onLevel && m_levelClock.isValid() && m_levelClock.elapsed() >= 50) {
            m_levelClock.restart();
            onLevel(m_seg->level());
        }
        sendPieces();
    }

    void sendPieces()
    {
        for (const speech::Piece &piece : m_seg->take()) {
            ++m_waiting;
            QMetaObject::invokeMethod(m_worker, "process", Qt::QueuedConnection, Q_ARG(QVector<float>, piece.audio),
                                      Q_ARG(bool, piece.newParagraph));
        }
        if (m_waiting > 0 && m_modelReady)
            report(m_recording ? activeState() : QStringLiteral("transcribing %1").arg(m_waiting));
    }

    void pieceDone(const QString &text, bool newParagraph)
    {
        m_waiting = qMax(0, m_waiting - 1);
        if (m_fileFast && m_recording && !m_feedTimer.isActive() && m_waiting < 2)
            m_feedTimer.start();
        if (!text.isEmpty() && onText)
            onText(text, newParagraph);
        if (!m_recording && m_waiting == 0)
            finish();
        else if (!m_recording)
            report(QStringLiteral("transcribing %1").arg(m_waiting));
    }

    void finish()
    {
        m_fileFast = false;
        m_lastPct = -1;
        m_busy = false;
        report(QStringLiteral("finished"));
    }

    QString activeState() const
    {
        return m_fileFast ? QStringLiteral("reading %1").arg(qMax(0, m_lastPct)) : QStringLiteral("listening");
    }

    void report(const QString &s)
    {
        if (onState)
            onState(s);
    }

    void setKeep(bool on)
    {
        if (m_keeping == on || !m_host)
            return;
        m_keeping = on;
        m_host->keepRunning(QStringLiteral("recording"), on);   // tool interface 1.3: the computer is kept awake
    }

    Holder m_holder;
    qb::tools::ToolHost *m_host = nullptr;
    bool m_keeping = false;
    QThread m_thread;
    Worker *m_worker = nullptr;
    EngineSettings m_s;
    QScopedPointer<speech::Segmenter> m_seg;
    QAudioSource *m_source = nullptr;
    QIODevice *m_io = nullptr;
    QAudioFormat m_format;
    QVector<float> m_fileAudio;
    qsizetype m_filePos = 0;
    QTimer m_feedTimer;
    bool m_fileFast = false;
    int m_lastPct = -1;
    QScopedPointer<speech::WavWriter> m_writer;   // record only (0.64)
    QElapsedTimer m_levelClock;
    int m_waiting = 0;
    bool m_recording = false;
    bool m_busy = false;
    bool m_modelReady = false;
};

} // namespace

extern "C" Q_DECL_EXPORT speech::EngineInterface *qb_speech_impl_create()
{
    audiobackend::avoidUnconfiguredPipeWire(); // 0.80.0: before Qt Multimedia looks for a microphone
    return new Engine;
}

#include "SpeechEngine.moc"
