#pragma once
// 0.99.0: the catalog of the AI models the program offers (Read Aloud, Handwriting, Speech Transcription through tool
// interface 1.3) and where the downloaded ones are kept. No network here (ui/AiDownloads.h downloads).
//
// The catalog is built into the program (resources/ai/models.json), so the program asks no server what there is: the files
// are on the release named in the catalog ("models-5") of the project's repository, and their addresses are built from that
// release's tag and the file's name there:  <project page>/releases/download/<release>/<asset>.
// Format 1 (the pattern of models-4's catalog.json of Translation Helper):
//   {"format": 1, "release": "models-5",
//    "models": [{
//      "id": "pocket-tts-en",                   unique; the folder <data>/ai-models/<id>
//      "title": "Pocket TTS, English", "summary": "...",
//      "kinds": ["tts"],                        "tts", "asr", "handwriting": which settings pages show it
//      "engine": "pocket-tts",                  "whisper", "pocket-tts", "qwen3-tts" (the AI engine) or "onnx" (handwriting)
//      "languages": ["en"],                     ISO 639-1 codes; ["*"]: many
//      "voices": ["alba", "marius"],            optional: the preset voices (for the page; the engine lists them too)
//      "license": "CC-BY-4.0", "licenseUrl": "...", "credit": "...", "source": "<the original, with its commit>",
//      "nonCommercial": false,                  true: shown with the license before the download
//      "sizeMB": 236,                           the download
//      "memoryMB": 600,                         what it needs loaded (ModelMemory's figure; the engine's estimate without it)
//      "release": "models-5",                   optional: another release than the catalog's
//      "model": "pocket-tts.gguf",              the file the engine loads (a name of "files")
//      "extra": "mmproj-pocket-tts.gguf",       its second file: the mmproj (Pocket TTS), the tokenizer (Qwen); optional
//      "files": [{"name": "pocket-tts.gguf", "asset": "pocket-tts-en--pocket-tts.gguf", "size": 123, "sha256": "..."},
//                {"name": "big.gguf", "size": 3000000000, "sha256": "<of the whole file>",
//                 "parts": [{"asset": "x--big.gguf.part1", "size": ..., "sha256": "..."}, ...]}]   a file above 2 GB in parts
//    }]}
// A model counts as installed when every file is there with its size and its SHA-256 was checked after the download
// (the marker file ".checked" in its folder).
#include "core/AiEngine.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace ai {

struct CatalogFile {
    QString name;                 // in the model's folder
    QString asset;                // on the release (empty for a file in parts)
    qint64 size = 0;
    QString sha256;               // lower-case hex
    QList<CatalogFile> parts;     // a file in parts: each with asset, size and sha256
};

struct CatalogModel {
    QString id, title, summary, engine;
    QStringList kinds, languages, voices;
    QString license, licenseUrl, credit, source, release;
    bool nonCommercial = false;
    int sizeMB = 0, memoryMB = 0;
    QString model, extra;         // names in files
    QList<CatalogFile> files;
    bool is(const QString &kind) const { return kinds.contains(kind); }
    int engineKind() const;       // enum qbai_model_kind, 0 for a model that is not the AI engine's ("onnx")
    qint64 downloadBytes() const;
    bool published() const;       // every file has its SHA-256 (a catalog may name a file that is not published yet)
};

struct Catalog {
    int format = 0;
    QString release;
    QList<CatalogModel> models;

    // The program's own (resources/ai/models.json); the test switch QB_AI_CATALOG names a file instead.
    static const Catalog &builtIn();
    static Catalog fromJson(const QByteArray &json, QString *error = nullptr);
    QList<CatalogModel> ofKinds(const QStringList &kinds) const;   // empty: all
    const CatalogModel *model(const QString &id) const;
};

// <data>/ai-models: ~/.local/share/Quillbinder/ai-models (Windows: the folder "data" of the program folder)
QString modelsRoot();
QString modelDir(const QString &id);
bool isInstalled(const CatalogModel &m);
// the path of an installed model's "model" or "extra" file ("" when it is not installed)
QString modelFile(const CatalogModel &m, const QString &role);
// what Model::load takes
ModelFiles modelFiles(const CatalogModel &m);
bool removeModel(const CatalogModel &m, QString *error = nullptr);
// why the model cannot be downloaded here (not published yet, too large for this computer's memory, no room on the disk), or ""
QString downloadProblem(const CatalogModel &m);

// ---- addresses (built from the release tags; the test switches QB_AI_MODEL_SOURCE and QB_AI_ENGINE_SOURCE name a folder
// or an address ending in "/" instead)
QString fileAddress(const CatalogModel &m, const CatalogFile &f);
// the tag of this program's release: "v<version>-<label>" ("v<version>" without a label)
QString releaseTag();
// the engine package's file on the release of this version: "ai-engine.qbpkg" (Linux), "ai-engine-windows.qbpkg" (Windows)
QString engineAsset(const QString &engineId);
QString engineAddress(const QString &engineId);

} // namespace ai
} // namespace qb
