#include "ui/AiDownloads.h"

#include "AppInfo.h"
#include "core/Addons.h"
#include "core/AiCatalog.h"
#include "core/AiEngine.h"
#include "core/ReadAloud.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QTimer>
#include <QUrl>

namespace qb {

namespace {
constexpr qint64 kEngineLimit = 1024LL * 1024 * 1024;   // an engine package is far smaller; more is not ours

QString okMarker(const QString &dir, const QString &name) { return dir + QStringLiteral("/.") + name + QStringLiteral(".ok"); }

QString engineName(const QString &engineId)
{
    return engineId == ai::handwritingEngineId() ? QCoreApplication::translate("qb::AiDownloads", "Handwriting Engine")
                                                 : QCoreApplication::translate("qb::AiDownloads", "AI Engine");
}

bool sha256Of(const QString &path, QString *hex)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    QCryptographicHash h(QCryptographicHash::Sha256);
    if (!h.addData(&f))
        return false;
    *hex = QString::fromLatin1(h.result().toHex());
    return true;
}
} // namespace

AiDownloads &AiDownloads::instance()
{
    static AiDownloads *d = new AiDownloads;   // lives as long as the program
    return *d;
}

AiDownloads::AiDownloads(QObject *parent) : QObject(parent) {}

QNetworkReply *AiDownloads::get(const QString &url)
{
    if (!m_net)
        m_net = new QNetworkAccessManager(this);
    QNetworkRequest req{QUrl(url)};
    // as the check for updates: the program's name as the sender's name, nothing about this computer, no cookies
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral(QB_APP_NAME));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    req.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    req.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, QNetworkRequest::Manual);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    req.setTransferTimeout(60000);   // no data for a minute: the connection is gone
    ++m_requests;
    return m_net->get(req);
}

bool AiDownloads::installEngineFile(const QString &engineId, const QString &file, bool allowUnsigned, QString *error,
                                    bool *unsignedPackage)
{
    auto failed = [error](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    if (unsignedPackage)
        *unsignedPackage = false;
    const addons::PackageInfo info = addons::inspect(file);
    if (!info.isValid())
        return failed(info.error);
    if (info.manifest.kind != QLatin1String("addon") || info.manifest.id != engineId)
        return failed(QCoreApplication::translate("qb::AiDownloads", "%1 is not a package of the %2.")
                          .arg(QFileInfo(file).fileName(), engineName(engineId)));
    if (unsignedPackage)
        *unsignedPackage = !info.trusted() && info.signature != signing::State::Invalid;
    const addons::Unsigned policy = allowUnsigned ? addons::Unsigned::AllowAddons : addons::Unsigned::Refuse;
    const QString problem = addons::trustProblem(info, policy);
    if (!problem.isEmpty())
        return failed(problem);
    QStringList log;
    QString why;
    bool restart = false;
    if (!addons::install(file, &log, &why, &restart, policy))
        return failed(why);
    ai::Engine::instance().packageChanged();
    return true;
}

void AiDownloads::installEngine(const QString &engineId)
{
    if (busy()) {
        emit finished(engineId, false, tr("Another download is running."));
        return;
    }
    const QString url = ai::engineAddress(engineId);
    m_job = engineId;
    m_engineJob = true;
    m_cancelled = false;
    m_pieces.clear();
    const QString dir = QDir::tempPath() + QStringLiteral("/quillbinder-engine-%1").arg(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    Piece p;
    p.url = url;
    p.target = dir + QLatin1Char('/') + ai::engineAsset(engineId);
    p.finalName = ai::engineAsset(engineId);
    m_pieces << p;
    m_doneBefore = 0;
    m_total = 0;   // not known before the answer
    nextPiece();
}

void AiDownloads::download(const QString &modelId)
{
    if (busy()) {
        emit finished(modelId, false, tr("Another download is running."));
        return;
    }
    const ai::CatalogModel *m = ai::Catalog::builtIn().model(modelId);
    if (!m) {
        emit finished(modelId, false, tr("The model %1 is not in the catalog.").arg(modelId));
        return;
    }
    const QString problem = ai::downloadProblem(*m);
    if (!problem.isEmpty()) {
        emit finished(modelId, false, problem);
        return;
    }
    const QString dir = ai::modelDir(m->id);
    if (!QDir().mkpath(dir)) {
        emit finished(modelId, false, tr("%1 could not be made.").arg(QDir::toNativeSeparators(dir)));
        return;
    }
    m_job = modelId;
    m_engineJob = false;
    m_cancelled = false;
    m_pieces.clear();
    m_doneBefore = 0;
    m_total = 0;
    for (const ai::CatalogFile &f : m->files) {
        const QString target = dir + QLatin1Char('/') + f.name;
        if (QFileInfo::exists(okMarker(dir, f.name)) && QFileInfo(target).size() == f.size)
            continue;   // checked in an earlier download that stopped later
        QFile::remove(okMarker(dir, f.name));
        const QList<ai::CatalogFile> parts = f.parts.isEmpty() ? QList<ai::CatalogFile>{f} : f.parts;
        for (int i = 0; i < parts.size(); ++i) {
            Piece p;
            p.url = ai::fileAddress(*m, parts[i]);
            p.target = target + QStringLiteral(".part");
            p.size = parts[i].size;
            p.sha256 = parts[i].sha256;
            p.first = i == 0;
            p.last = i == parts.size() - 1;
            p.wholeSize = f.size;
            p.wholeSha256 = f.sha256;
            p.finalName = f.name;
            m_pieces << p;
            m_total += p.size;
        }
    }
    nextPiece();
}

bool AiDownloads::ensureVoicePack(const QString &language)
{
    if (busy())
        return false;
    const ai::CatalogModel *p = readaloud::voicePackWanted(language);
    if (!p)
        return false;
    download(p->id);
    return true;
}

void AiDownloads::cancel()
{
    if (!busy())
        return;
    m_cancelled = true;
    if (m_reply)
        m_reply->abort();
}

void AiDownloads::nextPiece()
{
    if (m_cancelled) {
        finish(false, tr("The download was cancelled."));
        return;
    }
    if (m_pieces.isEmpty()) {
        if (m_engineJob)
            return;   // the engine job ends in its piece's handler
        // every file is checked: the model counts as installed
        const ai::CatalogModel *m = ai::Catalog::builtIn().model(m_job);
        const QString dir = ai::modelDir(m_job);
        QFile marker(dir + QStringLiteral("/.checked"));
        if (!m || !marker.open(QIODevice::WriteOnly)) {
            finish(false, tr("%1 could not be written.").arg(QDir::toNativeSeparators(dir)));
            return;
        }
        marker.write(QByteArrayLiteral("checked\n"));
        marker.close();
        if (m->is(QStringLiteral("voicepack"))) {
            QString why;
            if (!readaloud::installVoicePackModel(*m, &why)) {
                QFile::remove(marker.fileName());
                finish(false, why);
                return;
            }
        }
        for (const ai::CatalogFile &f : m->files)
            QFile::remove(okMarker(dir, f.name));
        finish(true, tr("%1 is installed.").arg(m->title));
        return;
    }
    const Piece p = m_pieces.first();
    if (p.url.startsWith(QLatin1String("https://")) && !QSslSocket::supportsSsl()) {
        finish(false, tr("This copy of %1 cannot make a secure connection, so it cannot download. Download the current "
                         "AppImage once from the project's page.").arg(QStringLiteral(QB_APP_NAME)));
        return;
    }
    m_file.close();
    m_file.setFileName(p.target);
    if (!m_file.open(p.first ? (QIODevice::WriteOnly | QIODevice::Truncate) : (QIODevice::WriteOnly | QIODevice::Append))) {
        finish(false, tr("%1 could not be written.").arg(QDir::toNativeSeparators(p.target)));
        return;
    }
    m_hash.reset();
    m_pieceDone = 0;
    m_tooLarge = false;
    QNetworkReply *reply = get(p.url);
    m_reply = reply;
    connect(reply, &QNetworkReply::readyRead, this, [this, reply, p] {
        const QByteArray chunk = reply->readAll();
        m_pieceDone += chunk.size();
        const qint64 limit = p.size > 0 ? p.size : kEngineLimit;
        if (m_pieceDone > limit || m_file.write(chunk) != chunk.size()) {
            m_tooLarge = true;   // more than the catalog says, or the disk does not take it
            reply->abort();
            return;
        }
        m_hash.addData(chunk);
        qint64 total = m_total;
        if (total <= 0)
            total = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        emit progress(m_job, m_doneBefore + m_pieceDone, total);
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, p] {
        reply->deleteLater();
        if (reply != m_reply)
            return;
        const bool written = m_file.flush();
        m_file.close();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool local = reply->url().isLocalFile();
        QString problem;
        if (m_cancelled)
            problem = tr("The download was cancelled.");
        else if (m_tooLarge || !written)
            problem = tr("The download failed: the file could not be written, or is larger than announced.");
        else if (reply->error() != QNetworkReply::NoError || (!local && status != 200))
            problem = tr("The download failed (%1).").arg(reply->errorString());
        else if (p.size > 0 && m_pieceDone != p.size)
            problem = tr("The download is incomplete (%1 of %2 bytes).").arg(m_pieceDone).arg(p.size);
        else if (!p.sha256.isEmpty() && QString::fromLatin1(m_hash.result().toHex()) != p.sha256)
            problem = tr("The downloaded file is not the one the catalog names (its SHA-256 differs), so it was not kept.");
        if (!problem.isEmpty()) {
            QFile::remove(p.target);
            finish(false, problem);
            return;
        }
        if (m_engineJob) {
            QString why;
            const bool ok = installEngineFile(m_job, p.target, false, &why);
            QFile::remove(p.target);
            QDir().rmdir(QFileInfo(p.target).absolutePath());
            m_pieces.clear();
            finish(ok, ok ? tr("%1 is installed.").arg(engineName(m_job)) : why);
            return;
        }
        if (p.last) {
            QString why;
            if (!placeFile(p, &why)) {
                QFile::remove(p.target);
                finish(false, why);
                return;
            }
        }
        m_doneBefore += m_pieceDone;
        m_pieces.removeFirst();
        nextPiece();
    });
}

bool AiDownloads::placeFile(const Piece &p, QString *error)
{
    // a file in parts: each part was checked; now the whole file (a single piece was checked as the whole)
    if (QFileInfo(p.target).size() != p.wholeSize) {
        *error = tr("The download is incomplete.");
        return false;
    }
    if (p.wholeSha256 != p.sha256) {
        QString hex;
        if (!sha256Of(p.target, &hex) || hex != p.wholeSha256) {
            *error = tr("The joined file is not the one the catalog names (its SHA-256 differs), so it was not kept.");
            return false;
        }
    }
    const QString dir = QFileInfo(p.target).absolutePath();
    const QString final = dir + QLatin1Char('/') + p.finalName;
    QFile::remove(final);
    if (!QFile::rename(p.target, final)) {
        *error = tr("%1 could not be written.").arg(QDir::toNativeSeparators(final));
        return false;
    }
    QFile ok(okMarker(dir, p.finalName));
    if (ok.open(QIODevice::WriteOnly))
        ok.write(QByteArrayLiteral("ok\n"));
    return true;
}

void AiDownloads::finish(bool ok, const QString &message)
{
    const QString what = m_job;
    const ai::CatalogModel *done = m_engineJob ? nullptr : ai::Catalog::builtIn().model(what);
    const bool qwen = ok && done && done->engine == QLatin1String("qwen3-tts");
    m_job.clear();
    m_pieces.clear();
    m_reply = nullptr;
    m_file.close();
    emit finished(what, ok, message);
    if (ok)
        emit changed();
    // a Qwen model reads with the voices of the Read Aloud language: its pack follows (a moment later: the slots of
    // finished() have run)
    if (qwen)
        QTimer::singleShot(0, this, [this] { ensureVoicePack(readaloud::effectiveLanguage(readaloud::Config::instance().get())); });
}

} // namespace qb
