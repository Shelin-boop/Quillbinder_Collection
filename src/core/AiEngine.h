#pragma once
// 0.99.0: the program's side of the AI engine packages (decision 3 of the 0.99.0 run).
//
// "ai-engine" (one per system: ggml with its CPU modules and Vulkan, whisper.cpp, llama.cpp with libmtmd, qwentts.cpp, Sonic and
// our wrapper libqbai) and "handwriting-engine" (ONNX Runtime) are add-on packages, installed by the program's package
// installer from the settings pages that need them (ui/AiInstallPanel.h) and never by default. The program has no link-time
// dependency on either: Engine opens libqbai with QLibrary when an AI function is first used, checks the interface version
// (tools/QbAi.h) and hands its table to the program's functions (Model) and to add-ons (ToolHost::aiEngine, tool interface
// 1.3), so one process holds one engine. ONNX Runtime is opened by Handwriting itself (onnxRuntimeLibrary()).
//
// Devices: "auto" (Automatic) takes a graphics card when the model fits into its free memory (the budget its driver reports),
// else the CPU; or a fixed card ("gpu:<name>"); or "cpu". The cards' figures go to ModelMemory (reportDeviceMemory), and every
// model is registered there before it loads (requestLoad), so the one rule for the AI's memory applies to it.
#include "tools/QbAi.h"

#include <QList>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>
#include <atomic>
#include <functional>

class QLibrary;

namespace qb {
namespace modelmem {
class ModelMemory;
}
namespace ai {

// the add-on ids of the two packages
QString aiEngineId();            // "ai-engine"
QString handwritingEngineId();   // "handwriting-engine"

struct PackageState {
    bool installed = false;
    bool enabled = false;
    bool fitsRuntime = true;     // built for this program's libraries
    QString version;
    QString dir;                 // the add-on's folder
    bool usable() const { return installed && enabled && fitsRuntime; }
};
PackageState package(const QString &id);

// The file of ONNX Runtime in the installed package "handwriting-engine" ("" when it is not installed): Handwriting opens it
// with QLibrary and asks for OrtGetApiBase.
QString onnxRuntimeLibrary();

struct Device {
    int index = 0;               // qbai_device.index: 0 = the CPU
    bool gpu = false;
    bool integrated = false;     // a graphics part that shares the computer's memory
    QString id;                  // ggml's name ("Vulkan0")
    QString name;                // what the writer sees
    qint64 free = 0, total = 0;
    // ModelMemory's name of its memory: "ram", or "gpu:<name>"
    QString where() const;
    // "auto", "cpu" or "gpu:<name>": the value of a device setting
    QString choice() const { return gpu ? where() : QStringLiteral("cpu"); }
};

// The device for a model of `bytes` under the writer's choice:
//  - "auto": the card whose free memory holds the model (a card of its own before a graphics part that shares the RAM, then
//    the one with more free memory); none (or bytes unknown, 0): the CPU;
//  - "gpu:<name>": that card, while it is there (else the CPU, and *why says so);
//  - "cpu" (and anything else): the CPU.
// `devices` is Engine::devices(); its first entry is the CPU.
Device pickDevice(const QList<Device> &devices, const QString &choice, qint64 bytes, QString *why = nullptr);
// The entries of a device setting: ("auto", "Automatic"), ("cpu", "Processor (CPU)"), ("gpu:<name>", "<name>") ...
QList<QPair<QString, QString>> deviceChoices(const QList<Device> &devices);

class Engine : public QObject
{
    Q_OBJECT
public:
    static Engine &instance();

    // libqbai of the installed "ai-engine" (its engine/lib folder); "" when it is not installed, switched off or made for
    // other libraries. The test switch QB_AI_ENGINE_LIB names a library to take instead (the suites' fake engine).
    QString libraryPath() const;
    bool isInstalled() const { return !libraryPath().isEmpty(); }
    // Loads libqbai once (from any thread); false with the reason. A second call after a failure tries again.
    bool ensureLoaded(QString *error = nullptr);
    bool isLoaded() const { return m_api != nullptr; }
    const qbai_api *api() const { return m_api; }
    // For tool interface 1.3 (ToolHost::aiEngine): the engine's table for `version`, loaded on demand; null when there is no
    // engine or it is older.
    const qbai_api *apiFor(int version);
    QString version() const;            // the engine's own text ("qbai 1; ggml 1.1.1; ...")
    QString loadedFrom() const { return m_loadedFrom; }
    QString lastError() const { return m_error; }

    // The devices, the CPU first; the cards' figures are reported to ModelMemory. Empty while no engine is loaded.
    QList<Device> devices(bool refresh = false);
    // The shared setting of the AI's cores (ModelMemory::aiThreads) into the engine.
    void applyThreads();

    // A package was installed or removed: a loaded engine stays until the program ends (its libraries are in use), so a
    // new one takes effect at the next start (restartNeeded()).
    void packageChanged();
    bool restartNeeded() const { return m_restartNeeded; }

    // Tests: forget the loaded library (only when no model of the program is loaded).
    void unloadForTests();

signals:
    void stateChanged();

private:
    Engine() = default;
    mutable QMutex m_mutex;
    QLibrary *m_lib = nullptr;
    const qbai_api *m_api = nullptr;
    QString m_loadedFrom, m_error;
    bool m_restartNeeded = false;
};

// What a model consists of: the engine's kind (enum qbai_model_kind) and its files.
struct ModelFiles {
    int kind = 0;
    QString model;               // whisper .bin, the Pocket TTS .gguf, the Qwen talker .gguf
    QString extra;               // the mmproj (Pocket TTS), the 12 Hz tokenizer (Qwen); empty for whisper
    QString name;                // what the writer sees (the message line)
    qint64 memoryBytes = 0;      // the catalog's figure; 0: the engine's estimate (qbai_api::model_memory)
};

struct Voice {
    QString preset;              // Pocket TTS's voice or a Qwen CustomVoice speaker ("": the model's default)
    QVector<float> clip;         // a reference clip (Qwen Base), mono
    int clipRate = 0;
    QString clipText;            // what is said in the clip
    QString instruct;            // Qwen CustomVoice: a style
};

// One model of a program function (Read Aloud, ...). It registers with ModelMemory under its id ("read-aloud:<model>") before it
// loads, and unloads when ModelMemory asks for it (idle, memory short) unless a job is running on it then.
// Threads: load(), synthesize() and transcribe() block; call them from one worker thread that lowered its priority first
// (ModelMemory::lowerThreadPriority). The calls to ModelMemory go to the main thread (they wait for it), so the main thread
// must not wait for that worker while it loads. isLoaded() and the facts (voices(), device() ...) may be read from any thread
// at any time; unload() waits for a running job.
class Model : public QObject
{
    Q_OBJECT
public:
    explicit Model(const QString &id, modelmem::ModelMemory *memory = nullptr, QObject *parent = nullptr);
    ~Model() override;

    QString id() const { return m_id; }
    // Loads the model on the device the choice gives ("auto", "cpu", "gpu:<name>"). Automatic falls back to the CPU when the
    // card is refused by the memory rule. Loading again with other files unloads the old model first.
    bool load(const ModelFiles &files, const QString &deviceChoice, QString *error);
    // Waits for a job that runs on the model (cancel it first); never call it while that job waits for this thread.
    void unload();
    // These never wait for a running job (the facts are kept when the model loads).
    bool isLoaded() const { return m_loaded.load(); }
    Device device() const;
    ModelFiles files() const;
    qint64 bytes() const;
    int sampleRate() const;
    QStringList voices() const;
    QStringList languages() const;
    bool canCloneVoice() const;

    // TTS: one sentence. With `chunk` the PCM comes in pieces as it is made (return false to stop; it may run on a thread
    // of the engine), else all of it in *pcm. `cancel` stops it (checked by the engine).
    bool synthesize(const QString &text, const QString &language, const Voice &voice,
                    const std::function<bool(const float *, qint64)> &chunk, const std::atomic_bool *cancel,
                    QVector<float> *pcm, QString *error);
    // ASR: mono PCM at 16 kHz; `segment` gets each piece of text as it is found (on the calling thread).
    bool transcribe(const QVector<float> &pcm16k, const QString &language, bool translate,
                    const std::function<void(const QString &, qint64, qint64)> &segment, const std::atomic_bool *cancel,
                    QString *text, QString *error);

    // ModelMemory's request: unloads when no job runs (true), else false.
    bool unloadIfIdle();

signals:
    void loadedChanged(bool loaded);

private:
    void freeModel();
    QString m_id;
    modelmem::ModelMemory *m_memory = nullptr;
    // m_lock guards m_model against an unload while a job runs. Nothing waits for the main thread while it is held (the calls
    // to ModelMemory are made outside it), so the main thread may take it.
    mutable QMutex m_lock;
    qbai_model *m_model = nullptr;
    std::atomic<int> m_busy{0};
    std::atomic<bool> m_loaded{false};
    mutable QMutex m_infoLock;       // the facts below; held only for a copy
    Device m_device;
    ModelFiles m_files;
    qint64 m_bytes = 0;
    int m_rate = 0;
    bool m_clone = false;
    QStringList m_voices, m_languages;
};

// Sonic: the speed changed (0.5 .. 4.0), the pitch kept. false without an engine.
bool stretch(const QVector<float> &in, int rate, double speed, QVector<float> *out, QString *error);

} // namespace ai
} // namespace qb
