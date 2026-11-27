#pragma once
// Between the two libraries of the add-on "Speech Transcription" (0.61):
//  - libqb-speech.so       the writing tool the program loads (Qt Widgets, Network): commands,
//                          the inspector tab, the options, models and their download
//  - libqb-speech-impl.so  the engine, loaded by the tool when it is needed: microphone (Qt
//                          Multimedia, brought by the add-on), pauses; the recognition itself is done by the
//                          program's AI engine (tool interface 1.3, 1.10 of the add-on): no whisper.cpp of its own
// The tool finds the engine in the add-on's speech/ folder and calls qb_speech_impl_create().
// Both are built from this source together, so this header may change with them.
#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <functional>

namespace qb { namespace tools { class ToolHost; } }

namespace speech {

struct AudioInput {
    QByteArray id;        // QAudioDevice::id()
    QString name;
    bool isDefault = false;
};

struct EngineSettings {
    QString modelFile;
    QString modelId;          // 1.10: the catalog's id (or "file"): the name under which the program's registry knows the model
    QString device = QStringLiteral("auto");   // 1.10: "auto", "cpu" or "gpu:<name>" (ToolHost::aiPickDevice)
    QString mainLanguage = QStringLiteral("en");      // the text's language
    QString fallbackLanguage = QStringLiteral("de");  // also spoken; empty: none
    bool modelTranslates = true;  // false: the model cannot translate (turbo)
    bool keepOriginal = false;    // translated pieces keep what was said, in brackets after them
    QString hint;                 // words and names the text uses
    bool useContext = false;      // the text before as context for the next piece
    bool silenceGuard = true;     // leave out what whisper invents in silence or noise
    int sensitivity = 1;          // 0 low, 1 medium, 2 high: how quiet speech may be
    int pauseMs = 700;            // a pause this long ends a piece
    int paragraphMs = 3000;       // a pause this long starts a new paragraph (0: never)
    QByteArray audioInput;        // empty: the system's default input
    // 0.64, "Record only": the microphone goes into recordFile (WAV, 16 kHz mono 16 bit) and
    // nothing else runs (no model, no recognition): little power, the text comes later
    bool recordOnly = false;
    QString recordFile;
    // 0.64, "Transcribe now" for a recording: the WAV file is read as fast as the model takes
    // its pieces (not in real time)
    bool fileAsFast = false;
};

struct TestResult {
    bool ok = false;
    QString text;          // what whisper wrote
    double audioSeconds = 0;
    double seconds = 0;    // time taken (loading not counted)
    double loadSeconds = 0;
    QString modelType;     // the engine's name for the loaded model ("whisper")
    QString error;
};

class EngineInterface
{
public:
    virtual ~EngineInterface() = default;
    virtual QList<AudioInput> audioInputs() = 0;
    // 1.10: the program's host (tool interface 1.3): the AI engine, the device choice and the registry of loaded models.
    // Set once, before anything else is called.
    virtual void setHost(qb::tools::ToolHost *host) = 0;
    // Transcribes a WAV file (16-bit PCM) with these settings (the model's own test, the speed
    // test); blocks: call it outside the main thread.
    virtual TestResult transcribeFile(const QString &wavFile, const EngineSettings &settings, bool translate) = 0;

    // Recording: from the microphone (wavFile empty) or from a WAV file played in real time
    // (wavFile, used by the tests). The callbacks come in the thread that called start().
    std::function<void(const QString &text, bool newParagraph)> onText;
    std::function<void(float level)> onLevel;          // 0..1, about 20 times a second while recording
    std::function<void(const QString &state)> onState; // "loading", "listening", "recording" (record only),
                                                       // "transcribing <n>", "reading <percent>", "finished"
    std::function<void(const QString &message)> onError;
    virtual bool start(const EngineSettings &settings, QString *error, const QString &wavFile = QString()) = 0;
    // Stops the microphone; the pieces waiting are still written, then onState("finished").
    virtual void stop() = 0;
    virtual bool isRecording() const = 0;
    virtual bool isBusy() const = 0;     // recording or still transcribing
    virtual int waiting() const = 0;     // pieces not transcribed yet
    // Frees the model (the program's rule does it after the time without use: ToolModels::unloadModel). false: it is in use now.
    virtual void unloadModel() = 0;
    // The program asks to unload the model of this registry id; true when it is gone or was not loaded (never waits for a job).
    virtual bool unloadModelOfId(const QString &registryId) = 0;
    virtual QString version() const = 0; // the AI engine's version line
};

} // namespace speech

using SpeechCreateFn = speech::EngineInterface *(*)();
// extern "C" speech::EngineInterface *qb_speech_impl_create();
