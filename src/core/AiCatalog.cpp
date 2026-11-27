#include "core/AiCatalog.h"

#include "AppInfo.h"
#include "core/Paths.h"
#include "core/SystemMemory.h"
#include "core/TestSwitches.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStorageInfo>
#include <QUrl>

namespace qb {
namespace ai {

namespace {

QStringList strings(const QJsonValue &v)
{
    QStringList out;
    for (const QJsonValue &x : v.toArray())
        out << x.toString();
    return out;
}

CatalogFile fileFrom(const QJsonObject &o)
{
    CatalogFile f;
    f.name = o.value(QStringLiteral("name")).toString();
    f.asset = o.value(QStringLiteral("asset")).toString();
    f.size = qint64(o.value(QStringLiteral("size")).toDouble());
    f.sha256 = o.value(QStringLiteral("sha256")).toString().toLower();
    for (const QJsonValue &p : o.value(QStringLiteral("parts")).toArray())
        f.parts << fileFrom(p.toObject());
    return f;
}

bool safeName(const QString &n)
{
    return !n.isEmpty() && !n.contains(QLatin1Char('/')) && !n.contains(QLatin1Char('\\')) && n != QLatin1String(".")
           && n != QLatin1String("..") && !n.startsWith(QLatin1Char('.'));
}

QString markerFile(const QString &id) { return modelDir(id) + QStringLiteral("/.checked"); }

} // namespace

int CatalogModel::engineKind() const
{
    if (engine == QLatin1String("whisper"))
        return QBAI_ASR_WHISPER;
    if (engine == QLatin1String("pocket-tts"))
        return QBAI_TTS_POCKET;
    if (engine == QLatin1String("qwen3-tts"))
        return QBAI_TTS_QWEN;
    return 0;
}

qint64 CatalogModel::downloadBytes() const
{
    qint64 n = 0;
    for (const CatalogFile &f : files)
        n += f.size;
    return n;
}

bool CatalogModel::published() const
{
    for (const CatalogFile &f : files) {
        if (f.sha256.size() != 64)
            return false;
        for (const CatalogFile &p : f.parts)
            if (p.sha256.size() != 64 || p.asset.isEmpty())
                return false;
        if (f.parts.isEmpty() && f.asset.isEmpty())
            return false;
    }
    return !files.isEmpty();
}

Catalog Catalog::fromJson(const QByteArray &json, QString *error)
{
    Catalog c;
    QJsonParseError pe{};
    const QJsonDocument doc = QJsonDocument::fromJson(json, &pe);
    if (!doc.isObject()) {
        if (error)
            *error = pe.errorString();
        return c;
    }
    const QJsonObject root = doc.object();
    c.format = root.value(QStringLiteral("format")).toInt();
    c.release = root.value(QStringLiteral("release")).toString();
    if (c.format != 1) {
        if (error)
            *error = QStringLiteral("unknown catalog format %1").arg(c.format);
        c.format = 0;
        return c;
    }
    for (const QJsonValue &v : root.value(QStringLiteral("models")).toArray()) {
        const QJsonObject o = v.toObject();
        CatalogModel m;
        m.id = o.value(QStringLiteral("id")).toString();
        if (!safeName(m.id))
            continue;   // the id is a folder name
        m.title = o.value(QStringLiteral("title")).toString(m.id);
        m.summary = o.value(QStringLiteral("summary")).toString();
        m.engine = o.value(QStringLiteral("engine")).toString();
        m.kinds = strings(o.value(QStringLiteral("kinds")));
        m.languages = strings(o.value(QStringLiteral("languages")));
        m.voices = strings(o.value(QStringLiteral("voices")));
        m.license = o.value(QStringLiteral("license")).toString();
        m.licenseUrl = o.value(QStringLiteral("licenseUrl")).toString();
        m.credit = o.value(QStringLiteral("credit")).toString();
        m.source = o.value(QStringLiteral("source")).toString();
        m.release = o.value(QStringLiteral("release")).toString(c.release);
        m.nonCommercial = o.value(QStringLiteral("nonCommercial")).toBool();
        m.sizeMB = o.value(QStringLiteral("sizeMB")).toInt();
        m.memoryMB = o.value(QStringLiteral("memoryMB")).toInt();
        m.model = o.value(QStringLiteral("model")).toString();
        m.extra = o.value(QStringLiteral("extra")).toString();
        bool ok = true;
        for (const QJsonValue &f : o.value(QStringLiteral("files")).toArray()) {
            const CatalogFile cf = fileFrom(f.toObject());
            if (!safeName(cf.name))
                ok = false;
            m.files << cf;
        }
        if (!ok)
            continue;
        if (m.model.isEmpty() && !m.files.isEmpty())
            m.model = m.files.first().name;
        c.models << m;
    }
    return c;
}

const Catalog &Catalog::builtIn()
{
    static const Catalog c = [] {
        QFile f(QStringLiteral(":/ai/models.json"));
        if (QB_TEST_ENV_SET("QB_AI_CATALOG"))
            f.setFileName(QB_TEST_ENV("QB_AI_CATALOG"));
        if (!f.open(QIODevice::ReadOnly))
            return Catalog();
        return fromJson(f.readAll());
    }();
    return c;
}

QList<CatalogModel> Catalog::ofKinds(const QStringList &kinds) const
{
    QList<CatalogModel> out;
    for (const CatalogModel &m : models) {
        bool want = kinds.isEmpty();
        for (const QString &k : kinds)
            want = want || m.is(k);
        if (want)
            out << m;
    }
    return out;
}

const CatalogModel *Catalog::model(const QString &id) const
{
    for (const CatalogModel &m : models)
        if (m.id == id)
            return &m;
    return nullptr;
}

QString modelsRoot()
{
    return QDir::cleanPath(dataDir() + QStringLiteral("/ai-models"));
}

QString modelDir(const QString &id) { return modelsRoot() + QLatin1Char('/') + id; }

bool isInstalled(const CatalogModel &m)
{
    if (m.files.isEmpty() || !QFileInfo::exists(markerFile(m.id)))
        return false;
    for (const CatalogFile &f : m.files)
        if (QFileInfo(modelDir(m.id) + QLatin1Char('/') + f.name).size() != f.size)
            return false;
    return true;
}

QString modelFile(const CatalogModel &m, const QString &role)
{
    if (!isInstalled(m))
        return QString();
    const QString name = role == QLatin1String("extra") ? m.extra : m.model;
    return name.isEmpty() ? QString() : modelDir(m.id) + QLatin1Char('/') + name;
}

ModelFiles modelFiles(const CatalogModel &m)
{
    ModelFiles f;
    f.kind = m.engineKind();
    f.model = modelFile(m, QStringLiteral("model"));
    f.extra = modelFile(m, QStringLiteral("extra"));
    f.name = m.title;
    f.memoryBytes = qint64(m.memoryMB) * 1024 * 1024;
    return f;
}

bool removeModel(const CatalogModel &m, QString *error)
{
    const QString dir = modelDir(m.id);
    if (!QFileInfo::exists(dir))
        return true;
    QFile::remove(markerFile(m.id));   // first: a half-removed model never counts as installed
    if (!QDir(dir).removeRecursively()) {
        if (error)
            *error = QCoreApplication::translate("qb::ai", "%1 could not be removed completely.").arg(QDir::toNativeSeparators(dir));
        return false;
    }
    return true;
}

QString downloadProblem(const CatalogModel &m)
{
    if (!m.published())
        return QCoreApplication::translate("qb::ai", "%1 is not published yet.").arg(m.title);
    const sysmem::Figures ram = sysmem::ram();
    // the rule of the AI's memory: a model that would take more than 80 % of the computer's memory never loads
    if (ram.valid() && m.memoryMB > 0 && double(m.memoryMB) * 1024 * 1024 > 0.8 * double(ram.total))
        return QCoreApplication::translate("qb::ai", "%1 needs about %2 GB of memory; this computer has %3 GB.")
            .arg(m.title, QString::number(m.memoryMB / 1024.0, 'f', 1), QString::number(double(ram.total) / (1024.0 * 1024 * 1024), 'f', 1));
    QDir().mkpath(modelsRoot());
    const QStorageInfo disk(modelsRoot());
    const qint64 need = m.downloadBytes() + 64LL * 1024 * 1024;
    if (disk.isValid() && disk.bytesAvailable() >= 0 && disk.bytesAvailable() < need)
        return QCoreApplication::translate("qb::ai", "There is not enough room on the disk for %1 (%2 MB are needed).")
            .arg(m.title).arg(need / (1024 * 1024));
    return QString();
}

namespace {

QString fromSource(const QString &source, const QString &asset)
{
    if (source.contains(QLatin1String("://")))
        return source + asset;
    return QUrl::fromLocalFile(source + QLatin1Char('/') + asset).toString();
}

} // namespace

QString fileAddress(const CatalogModel &m, const CatalogFile &f)
{
    const QString source = QB_TEST_ENV("QB_AI_MODEL_SOURCE");
    if (!source.isEmpty())
        return fromSource(source, f.asset);
    return QStringLiteral(QB_APP_HOMEPAGE "/releases/download/") + m.release + QLatin1Char('/') + f.asset;
}

QString releaseTag()
{
    const QString label = QStringLiteral(QB_APP_VERSION_LABEL);
    return QStringLiteral("v" QB_APP_VERSION) + (label.isEmpty() ? QString() : QLatin1Char('-') + label);
}

QString engineAsset(const QString &engineId)
{
#if defined(Q_OS_WIN)
    return engineId + QStringLiteral("-windows.qbpkg");
#elif defined(Q_OS_ANDROID)
    return engineId + QStringLiteral("-android.qbpkg");
#else
    return engineId + QStringLiteral(".qbpkg");
#endif
}

QString engineAddress(const QString &engineId)
{
    const QString source = QB_TEST_ENV("QB_AI_ENGINE_SOURCE");
    if (!source.isEmpty())
        return fromSource(source, engineAsset(engineId));
    return QStringLiteral(QB_APP_HOMEPAGE "/releases/download/") + releaseTag() + QLatin1Char('/') + engineAsset(engineId);
}

} // namespace ai
} // namespace qb
