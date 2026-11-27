#pragma once
// 0.99.0 (item 114): Read Aloud, the program's own reading of the text with a speech model (Pocket TTS or Qwen3-TTS of the AI
// engine, ai::Model). Built into the program on every system, off by default: nothing is installed and nothing loaded until the
// writer installs the engine and a model on File > Options > Read Aloud and presses Play. This file is the part without
// windows: the choices, the models, the voices, the sentences and the reading itself (Session). The toolbar and the page are
// in ui/ReadAloud*.h.
//
//  - The choices (Settings, kept per device in the program's settings, never in a project): language, model ("" = the default
//    model), device ("auto", "cpu", "gpu:<name>"), voice (Female 1 to 3, Male 1 to 3), style (Calm, Neutral, Action), speed
//    (Slow, Medium, Fast).
//  - The default model is the first of modelOrder() that is installed, has the language, fits this computer's memory and (when
//    measured) reaches real time. The order is ONE table in ReadAloud.cpp (kModelOrder): change the order there.
//  - Voices: Pocket TTS reads with its licensed preset voices only (pocketVoice(): the table of Female 1-3 / Male 1-3; never
//    a voice without a license), no style. Qwen3-TTS reads with designed reference clips (qwenClipFile()); a voice or style
//    whose clip is not there yet is greyed with "voice not installed yet".
//  - Speed: the finished audio of a sentence is stretched with the pitch kept (ai::stretch, Sonic): Medium is the model's own
//    pace, Slow and Fast about 15 % slower and faster.
//  - Session reads sentence by sentence on one worker thread at low priority; it generates only a few sentences ahead of what is
//    playing (lookAheadFor()), never a whole chapter. The audio is only played (Output), never written to a file.
#include "core/AiCatalog.h"
#include "core/AiEngine.h"

#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVector>
#include <atomic>
#include <functional>

class QThread;
class QTimer;

namespace qb {
namespace modelmem {
class ModelMemory;
}
namespace readaloud {

// ---- the choices -------------------------------------------------------------------------------------------------------
QStringList voiceIds();                  // "female1" .. "female3", "male1" .. "male3"
QStringList styleIds();                  // "calm", "neutral", "action"
QStringList speedIds();                  // "slow", "medium", "fast"
QString voiceLabel(const QString &id);   // "Female 1"
QString styleLabel(const QString &id);
QString speedLabel(const QString &id);
double speedFactor(const QString &id);   // 0.85, 1.0, 1.15 (Sonic's speed: more is faster)

struct Settings {
    QString language;                    // "" = the program's language (effectiveLanguage())
    QString model;                       // "" = the default model
    QString device = QStringLiteral("auto");
    QString voice = QStringLiteral("female1");
    QString style = QStringLiteral("neutral");
    QString speed = QStringLiteral("medium");
    bool barShown = true;                // the toolbar (View > Toolbars > Read Aloud): Android off, else on
    bool settingsOpened = false;         // the first click of Play has opened File > Options > Read Aloud
    static Settings defaults(bool android);
    static Settings load();
    void save() const;
    // "Female 2 · Calm · Medium"
    QString summary() const;
};

class Config : public QObject
{
    Q_OBJECT
public:
    static Config &instance();
    const Settings &get() const { return m_s; }
    void set(const Settings &s);          // saves and tells
    void reload();                        // from QSettings (the checks change the settings' place)
    static bool isAndroid();
    static void forceAndroid(int on);     // -1: the real system; 0 / 1: the checks
signals:
    void changed();

private:
    Config();
    Settings m_s;
};

// ---- languages and models ----------------------------------------------------------------------------------------------
// The languages some speech model of the catalog speaks (the program's other languages are greyed on the page)
bool languageHasModel(const QString &code);
// the language of a Settings: its own, else the program's language when it has a model, else English
QString effectiveLanguage(const Settings &s);
// The order of the models, best sounding first: ids (or the beginning of ids, "pocket-tts-" for the five languages' models)
const QStringList &modelOrder();
// the catalog's speech models of a language, in modelOrder() (then the others)
QList<const ai::CatalogModel *> modelsFor(const QString &language, const ai::Catalog &catalog = ai::Catalog::builtIn());

struct Fit {
    bool fits = false;
    qint64 need = 0;          // bytes (the catalog's figure)
    QString where;            // ModelMemory's name of the place: "ram", "gpu:<name>"
    QString text;             // "fits on <card>", "fits in RAM", "does not fit (needs 4.2 GB)"
};
// On this computer: `choice` is a device setting; `devices` is ai::Engine::devices() (empty without an engine: RAM only)
Fit fitOf(const ai::CatalogModel &m, const QString &choice, const QList<ai::Device> &devices);

// The measured speed of a model on a device: seconds of work per second of audio (below 1: faster than real time); 0 = never
// measured. Kept per device with the model.
double measuredSpeed(const QString &modelId, const QString &deviceChoice);
void rememberSpeed(const QString &modelId, const QString &deviceChoice, double ratio);
// The model that Read Aloud uses for these settings: the chosen one, or the default. `onlyInstalled`: a model that is not
// installed does not count (for reading); without it the best candidate is named (for the page). Null: no model at all.
const ai::CatalogModel *resolveModel(const Settings &s, const QList<ai::Device> &devices, bool onlyInstalled,
                                     const ai::Catalog &catalog = ai::Catalog::builtIn());

// ---- voices and styles -------------------------------------------------------------------------------------------------
// Pocket TTS's preset voice for a place of the menu; the same table for every language (the model names its voices; a model
// that lacks one greys the place out)
QString pocketVoice(const QString &language, const QString &voiceId);
// Where the reference clips of Qwen3-TTS' voices are: <models>/read-aloud-voices/<voice>-<style>.wav with the spoken text in
// <voice>-<style>.txt beside it (they come with the voices of release models-5)
QString qwenClipFile(const QString &voiceId, const QString &styleId);
bool readWav(const QString &file, QVector<float> *mono, int *rate);

struct Availability {
    bool ok = true;
    QString reason;           // when not ok: the words of the greyed menu entry's tool tip
};
Availability voiceAvailability(const ai::CatalogModel *m, const QString &language, const QString &voiceId, const QString &styleId);
Availability styleAvailability(const ai::CatalogModel *m, const QString &language, const QString &voiceId, const QString &styleId);
// What synthesize() takes for these choices; false with the reason when the voice cannot be made
bool makeVoice(const ai::CatalogModel &m, const QString &language, const QString &voiceId, const QString &styleId, ai::Voice *out,
               QString *why);

// ---- sentences ---------------------------------------------------------------------------------------------------------
struct Span {
    int start = 0, length = 0;     // in the text that was split
};
// The sentences of a text with their places; sentences longer than `maxLength` are cut at a comma or a blank.
QList<Span> splitSentences(const QString &text, int maxLength = 280);

// ---- where the sound goes ----------------------------------------------------------------------------------------------
// Mono float PCM that is only played. The real one is made by the media add-on (ui/ReadAloudOutput.cpp); the checks set their own.
class Output
{
public:
    virtual ~Output() = default;
    virtual bool open(int rate, QString *error) = 0;
    virtual void write(const float *samples, qint64 count) = 0;   // queued, never blocks
    virtual qint64 playedMs() const = 0;                          // of what was written, played so far
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;                                      // drops what is queued
};
using OutputFactory = std::function<Output *(QString *why)>;
void setOutputFactory(OutputFactory f);
Output *makeOutput(QString *why);

// ---- reading -----------------------------------------------------------------------------------------------------------
// How many sentences are made ahead of the one that plays and how many are ready before the sound starts: a model that
// reaches real time needs little, a slower one more (never more than a few)
struct LookAhead {
    int ahead = 2;
    int prebuffer = 1;
};
LookAhead lookAheadFor(double ratio);   // ratio: seconds of work per second of audio (0: not measured)

class Session : public QObject
{
    Q_OBJECT
public:
    enum State { Idle, Loading, Playing, Paused };
    Q_ENUM(State)
    explicit Session(modelmem::ModelMemory *memory = nullptr, QObject *parent = nullptr);
    ~Session() override;

    // the one the toolbar and the settings page share (made on first use; a child of the application)
    static Session &shared();
    State state() const { return m_state; }
    // Reads `text` (the document's characters from `offset` on: the signals name places in the document). The sentences are
    // made one by one on a worker thread. false with the reason when nothing can start.
    bool start(const QString &text, int offset, const Settings &s, QString *error);
    void pause();
    void resume();
    void stop();
    // The speed test of a model: loads it (the writer's device choice), makes one test sentence and measures the work against
    // the audio. The result is remembered (measuredSpeed) and told by speedMeasured(). false when busy.
    bool measureSpeed(const Settings &s, QString *error);
    bool measuring() const { return m_probe != nullptr; }

    // what the checks look at
    int generated() const { return m_generated; }          // sentences made in this reading
    int maxAhead() const { return m_maxAhead; }            // the most sentences that were made and not yet played
    int sentenceCount() const { return int(m_spans.size()); }
    LookAhead lookAhead() const { return m_look; }
    // One model per id is kept (registered with ModelMemory as "read-aloud:<id>"); null: none made yet
    static ai::Model *modelFor(const QString &catalogId, modelmem::ModelMemory *memory = nullptr, bool create = false);
    static void forgetModels();   // the checks: drops the objects (after unloading them)

signals:
    void stateChanged(qb::readaloud::Session::State state);
    void sentence(int start, int length);      // the sentence that plays now (document places); (-1, 0): none
    void finished(bool completed);             // the end of the text (true) or stop() (false)
    void failed(const QString &message);
    void speedMeasured(const QString &modelId, double ratio, const QString &error);

private:
    class Worker;
    struct Ready {
        int index = 0;
        QVector<float> pcm;
    };
    void setState(State s);
    void onReady(int index, const QVector<float> &pcm, int rate);
    void onWorkerDone(bool ok, const QString &error);
    void poll();
    void flushPending();
    void endReading(bool completed);
    void dropWorker();

    modelmem::ModelMemory *m_memory = nullptr;
    State m_state = Idle;
    QList<Span> m_spans;            // in document places
    int m_offset = 0;
    Settings m_settings;
    LookAhead m_look;
    Worker *m_worker = nullptr;
    Worker *m_probe = nullptr;
    Output *m_out = nullptr;
    QTimer *m_timer = nullptr;
    int m_rate = 0;
    QList<qint64> m_endMs;          // the end of each written sentence in the output's time (cumulative)
    QList<Ready> m_pending;         // ready, not yet written (the sound starts with the prebuffer)
    int m_written = 0;              // sentences written to the output
    int m_playing = -1;             // the sentence that plays now
    int m_generated = 0, m_maxAhead = 0;
    bool m_workerDone = false;
    bool m_started = false;         // the output got its first data
};

} // namespace readaloud
} // namespace qb
