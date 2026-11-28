#pragma once
// 0.99.0: installing an engine package and downloading the catalog's models (core/AiCatalog.h), the part with the network.
//
// Network only by an explicit action of the writer (a button of ui/AiInstallPanel.h), as Translation Helper's model download
// (the precedent): no network object is made before the first download, nothing is asked from a server, the addresses are
// built from the release tags (ai::engineAddress, ai::fileAddress). One job at a time.
//
//  - The engine: this version's package from the project's release (or a file the writer chose, for a computer without a
//    connection), checked as the package installer checks every add-on (its signature, the add-on key's certificate, the
//    SHA-256 of every file) and installed with it. A download must be signed by the project; a file the writer chose may
//    be unsigned after the writer's yes (as Help > Add-ons and Updates > Install Update or Add-on allows).
//  - A voice pack (Read Aloud): downloaded as a model (the zip, checked), then unpacked into read-aloud-voices/<lang>/ by
//    readaloud::installVoicePack; a Qwen model's download is followed by the pack of the Read Aloud language.
//  - A model: its files one after the other into <data>/ai-models/<id>, each checked against its size and SHA-256 (a file in
//    parts: each part, then the whole); the model counts as installed when all are checked.
#include <QCryptographicHash>
#include <QFile>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

namespace qb {
namespace ai {
struct CatalogFile;
struct CatalogModel;
}

class AiDownloads : public QObject
{
    Q_OBJECT
public:
    static AiDownloads &instance();
    explicit AiDownloads(QObject *parent = nullptr);   // the program uses instance(); the tests may make their own

    // ---- the engine packages ("ai-engine", "handwriting-engine")
    void installEngine(const QString &engineId);
    // An engine package from a file: true when installed; the package must be that engine's. allowUnsigned: the writer said
    // yes to an unsigned package (a package whose signature does not fit is never installed).
    static bool installEngineFile(const QString &engineId, const QString &file, bool allowUnsigned, QString *error,
                                  bool *unsignedPackage = nullptr);

    // ---- models of the catalog
    void download(const QString &modelId);
    // 0.99.0: the voice pack of a language (catalog kind "voicepack", core/ReadAloud.h) when a Qwen model is installed and the
    // pack is published but not installed: starts its download. False when nothing was started (no Qwen model, installed
    // already, not published yet, or another job is running).
    bool ensureVoicePack(const QString &language);
    void cancel();
    bool busy() const { return !m_job.isEmpty(); }
    QString busyWith() const { return m_job; }

    // the requests that left the program (the tests: none without a download)
    int requests() const { return m_requests; }
    bool hasNetwork() const { return m_net != nullptr; }

signals:
    // `what`: the engine's id or the model's id
    void progress(const QString &what, qint64 done, qint64 total);
    void finished(const QString &what, bool ok, const QString &message);
    void changed();   // an engine or a model was installed or removed

private:
    struct Piece {
        QString url;
        QString target;          // the file it goes into (appended for the second and later part)
        qint64 size = 0;
        QString sha256;
        bool first = true;       // the first piece of its file (the file is begun anew)
        bool last = true;        // the last piece of its file: then the whole file is checked and put in place
        qint64 wholeSize = 0;
        QString wholeSha256;
        QString finalName;       // the file's name in the model's folder
    };
    QNetworkReply *get(const QString &url);
    void nextPiece();
    void finish(bool ok, const QString &message);
    bool placeFile(const Piece &p, QString *error);

    QNetworkAccessManager *m_net = nullptr;
    QPointer<QNetworkReply> m_reply;
    QString m_job;               // the model or engine being fetched
    bool m_engineJob = false;
    QList<Piece> m_pieces;
    QFile m_file;
    QCryptographicHash m_hash{QCryptographicHash::Sha256};
    qint64 m_pieceDone = 0;
    qint64 m_doneBefore = 0, m_total = 0;
    bool m_cancelled = false, m_tooLarge = false;
    int m_requests = 0;
};

} // namespace qb
