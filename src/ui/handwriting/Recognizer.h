#pragma once
// 0.99.0 (handwriting, item 112): the interface between the writing surface and the engine that reads the ink.
//
// This header is all the surface knows of the recognizer. The engine itself (ONNX Runtime through OrtGetApiBase from the
// engine package, the line model and the word layer) is a later part: it derives from Recognizer, registers itself with
// Recognizer::setCurrent() once its package and model are installed, and registers its model with modelmem::ModelMemory
// (see memoryId() below: the hook the rule of the AI models needs). Until then no recognizer is installed and every use of
// the handwriting button opens File > Options > Handwriting, where the install part of the page calls the installer
// (Installer below) that the later part sets.
//
// What goes in: one line of ink, as strokes (points in the line's own picture coordinates) and as a rendered picture, and
// the languages (the main one first). What comes out: the text read, as words with their alternatives.
#include <QHash>
#include <QImage>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QVector>
#include <functional>

class QWidget;

namespace qb {
namespace handwriting {

struct Request {
    // the strokes of one line; each stroke is its points in the picture's coordinates (pixels, y down)
    QVector<QVector<QPointF>> strokes;
    // the line as a picture: dark ink on a white ground, the line's own box plus a margin
    QImage picture;
    // the main language first, then the fallback ("en", "de", ...)
    QStringList languages;
};

struct Word {
    QString text;
    QStringList alternatives;      // other readings, the most likely first (the chosen text is not repeated)
    QRectF box;                    // where the word was written, in the picture's coordinates
    double confidence = 1.0;
};

struct Result {
    bool ok = false;
    QString error;                 // when !ok: a sentence for the message line
    QList<Word> words;
    QString text() const;          // the words with one space between
};

class Recognizer : public QObject
{
    Q_OBJECT
public:
    explicit Recognizer(QObject *parent = nullptr) : QObject(parent) {}

    // The engine and the model are installed and can read now.
    virtual bool isInstalled() const = 0;
    // The language codes the installed model reads (a subset of language::available()).
    virtual QStringList languages() const = 0;
    // Reads one line. `done` is called exactly once, at once or later (an engine may work in a thread; the call to `done`
    // must come in the thread of the object that asked, as a queued call does).
    virtual void recognize(const Request &request, const std::function<void(const Result &)> &done) = 0;
    // The id this recognizer's model has in modelmem::ModelMemory ("handwriting:<name>"), for requestLoad()/modelLoaded().
    virtual QString memoryId() const { return QStringLiteral("handwriting:line"); }

    // ---- the recognizer in use (none by default)
    static Recognizer *current();
    static void setCurrent(Recognizer *recognizer);   // nullptr: none installed
    static bool installed() { return current() && current()->isInstalled(); }

    // ---- the installer: the install part of File > Options > Handwriting
    // The shared installer for engine packages and models belongs to another part; it sets this function, which opens its
    // own window (or starts the download) from the "Install" button and says in `status` what is there.
    struct Installer {
        std::function<void(QWidget *parent)> install;   // the button "Install"
        std::function<QString()> status;                // "Installed: ..." / "Not installed" (empty: nothing known)
        std::function<void()> remove;                   // the button "Remove" (empty: no such button)
    };
    static void setInstaller(const Installer &installer);
    static const Installer &installer();
};

// The languages a model exists for (a fixed list of the planned model; Russian has none yet) and the others.
QStringList modelLanguages();
bool hasModelFor(const QString &language);

// A recognizer for the checks: returns a fixed text per line (the n-th line it is asked to read gets the n-th text; after
// the last one, the last text again). A word's alternatives are the other words of `alternatives` (a list per word text).
class FakeRecognizer : public Recognizer
{
    Q_OBJECT
public:
    explicit FakeRecognizer(QObject *parent = nullptr) : Recognizer(parent) {}
    bool isInstalled() const override { return m_installed; }
    QStringList languages() const override { return m_languages; }
    void recognize(const Request &request, const std::function<void(const Result &)> &done) override;

    void setInstalled(bool on) { m_installed = on; }
    void setLines(const QStringList &texts) { m_texts = texts; m_next = 0; }
    void setAlternatives(const QString &word, const QStringList &alternatives) { m_alternatives.insert(word, alternatives); }
    void setLanguages(const QStringList &languages) { m_languages = languages; }
    int requests() const { return m_requests; }
    const Request &lastRequest() const { return m_last; }

private:
    bool m_installed = true;
    QStringList m_texts{QStringLiteral("hello world")};
    QStringList m_languages{QStringLiteral("en"), QStringLiteral("de")};
    QHash<QString, QStringList> m_alternatives;
    int m_next = 0;
    int m_requests = 0;
    Request m_last;
};

} // namespace handwriting
} // namespace qb
