#pragma once
// 0.99.0 (handwriting, item 112, part 2): the recognizer that reads a line of ink with the line model (PP-OCRv6 small as ONNX)
// and the word layer (WordLayer.h).
//
// ONNX Runtime comes from the installed Handwriting Engine (ai::onnxRuntimeLibrary()): the program opens it with QLibrary and
// asks for OrtGetApiBase only (no link-time dependency; the header src/third_party/onnxruntime/onnxruntime_c_api.h is the
// library's own, MIT). CPU provider only, the cores of the one AI setting (ModelMemory::aiThreads()), the worker thread at low
// priority, the model in ModelMemory's registry ("handwriting:line": the memory rule, the idle unload after the set time).
//
// A line, as the surface gives it (InkCanvas: the strokes of one ruled row, or, without rows, the strokes whose middle lies
// within 0.4 line heights of the line's box), is drawn alone and scaled to 96 px height; 16 px white are added left and right
// and white on the right up to a multiple of 8 px; RGB, value = 1 - pixel / 255.
#include "core/ModelMemory.h"
#include "ui/handwriting/Recognizer.h"
#include "ui/handwriting/WordLayer.h"

#include <QHash>
#include <QMutex>
#include <QThreadPool>
#include <atomic>
#include <functional>
#include <memory>

namespace qb {
namespace handwriting {

class OnnxRecognizer : public Recognizer
{
    Q_OBJECT
public:
    // where the files are (for the languages asked); empty paths: not installed
    struct Sources {
        QString runtimeLibrary;                 // libonnxruntime.so.1 of the Handwriting Engine
        QString model;                          // ppocrv6-small.onnx
        QString codec;                          // ppocrv6-small-codec.json
        QHash<QString, QString> wordLists;      // language -> wordlist-<language>.tsv.gz (only the installed ones)
        qint64 memoryBytes = 0;                 // for the registry; 0: 150 MB and 40 MB per list
    };
    using SourceProvider = std::function<Sources(const QStringList &languages)>;
    static Sources installedSources(const QStringList &languages);   // the catalog's models, the engine package

    explicit OnnxRecognizer(QObject *parent = nullptr, modelmem::ModelMemory *memory = nullptr, SourceProvider provider = nullptr);
    ~OnnxRecognizer() override;

    bool isInstalled() const override;
    QStringList languages() const override;
    void recognize(const Request &request, const std::function<void(const Result &)> &done) override;
    QString memoryId() const override { return QStringLiteral("handwriting:line"); }

    // the word layer on (the default) or off (the characters as read)
    void setWordLayer(bool on) { m_wordLayer = on; }
    bool wordLayer() const { return m_wordLayer; }

    // reads one line now, in the calling thread (loads the model when needed; the registry is not asked)
    Result readNow(const Request &request);
    bool isLoaded() const { return m_loaded.load(); }
    // unloads when no line is being read; false when busy
    bool unloadIfIdle();

    // ---- the input of the model, public for the checks
    struct Input {
        QVector<float> data;     // 1 x 3 x 96 x width, NCHW
        int width = 0;           // a multiple of 8
        int height = 96;
        double scale = 1;        // the model's px per picture px
        int pictureWidth = 0;
    };
    static Input prepareInput(const QImage &picture);

private:
    struct Session;
    bool ensureLoaded(const QStringList &languages, QString *error);
    Result readLocked(const Request &request);
    void unloadLocked();

    modelmem::ModelMemory *m_memory;
    SourceProvider m_provider;
    QThreadPool m_pool;
    QMutex m_mutex;                              // one line at a time; held while loading and reading
    std::unique_ptr<Session> m_session;
    QHash<QString, std::shared_ptr<WordList>> m_lists;
    Codec m_codec;
    std::atomic_bool m_loaded{false};
    std::atomic_bool m_closing{false};
    bool m_wordLayer = true;
};

// Registers an OnnxRecognizer as the program's recognizer and the installer of the Handwriting page's Install button
// (AiInstallPanel with the handwriting engine and the kinds "handwriting" and "wordlist"). Called once by the main window.
OnnxRecognizer *installOnnxRecognizer(QObject *parent);

} // namespace handwriting
} // namespace qb
