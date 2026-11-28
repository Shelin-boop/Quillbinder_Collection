#include "core/collab/Manifest.h"

#include "core/SafeSave.h"
#include "core/collab/PathClass.h"
#include "core/crypto/Vault.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <algorithm>

namespace qb {
namespace collab {

const ManifestEntry *Manifest::find(const QString &path) const
{
    auto it = std::lower_bound(entries.begin(), entries.end(), path, [](const ManifestEntry &e, const QString &p) { return e.path < p; });
    return it != entries.end() && it->path == path ? &*it : nullptr;
}

QJsonObject Manifest::toJson() const
{
    QJsonArray list;
    for (const ManifestEntry &e : entries)
        list.append(QJsonObject{{QStringLiteral("path"), e.path},
                                {QStringLiteral("size"), double(e.size)},
                                {QStringLiteral("sha256"), e.sha256},
                                {QStringLiteral("rev"), double(e.rev)}});
    QJsonObject r;
    QStringList keys = revs.keys();
    keys.sort();
    for (const QString &k : keys)
        r.insert(k, double(revs.value(k)));
    return QJsonObject{{QStringLiteral("epoch"), double(epoch)}, {QStringLiteral("seq"), double(seq)}, {QStringLiteral("format"), format},
                       {QStringLiteral("entries"), list},    {QStringLiteral("revs"), r}};
}

bool Manifest::fromJson(const QJsonObject &o, Manifest *out, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why)
            *why = w;
        return false;
    };
    if (!o.value(QStringLiteral("entries")).isArray() || !o.value(QStringLiteral("format")).isDouble())
        return fail(QStringLiteral("not a manifest"));
    if (o.value(QStringLiteral("format")).toInt() != kManifestFormat)
        return fail(QStringLiteral("a format of the manifest that this version does not know"));
    Manifest m;
    m.epoch = qint64(o.value(QStringLiteral("epoch")).toDouble());
    m.seq = qint64(o.value(QStringLiteral("seq")).toDouble());
    const QJsonArray list = o.value(QStringLiteral("entries")).toArray();
    if (list.size() > kManifestMaxEntries)
        return fail(QStringLiteral("too many entries"));
    static const QRegularExpression hex(QStringLiteral("^[0-9a-f]{64}$"));
    QStringList paths;
    for (const QJsonValue &v : list) {
        const QJsonObject e = v.toObject();
        ManifestEntry me;
        me.path = e.value(QStringLiteral("path")).toString();
        me.size = qint64(e.value(QStringLiteral("size")).toDouble(-1));
        me.sha256 = e.value(QStringLiteral("sha256")).toString();
        me.rev = qint64(e.value(QStringLiteral("rev")).toDouble());
        QString reason;
        if (!isAcceptablePath(me.path, &reason))
            return fail(QStringLiteral("%1: %2").arg(me.path, reason));
        if (!classify(me.path).replicated())
            return fail(QStringLiteral("%1 is not a replicated path").arg(me.path));
        if (me.size < 0 || !hex.match(me.sha256).hasMatch())
            return fail(QStringLiteral("%1: size or hash is wrong").arg(me.path));
        m.entries << me;
        paths << me.path;
    }
    std::sort(m.entries.begin(), m.entries.end(), [](const ManifestEntry &a, const ManifestEntry &b) { return a.path < b.path; });
    for (int i = 1; i < m.entries.size(); ++i)
        if (m.entries.at(i).path == m.entries.at(i - 1).path)
            return fail(QStringLiteral("%1 is listed twice").arg(m.entries.at(i).path));
    if (!caseCollisions(paths).isEmpty())
        return fail(QStringLiteral("two paths differ only by case"));
    const QJsonObject r = o.value(QStringLiteral("revs")).toObject();
    for (auto it = r.constBegin(); it != r.constEnd(); ++it)
        m.revs.insert(it.key(), qint64(it.value().toDouble()));
    *out = m;
    return true;
}

QString Manifest::stateHash() const
{
    QCryptographicHash h(QCryptographicHash::Sha256);
    for (const ManifestEntry &e : entries) { // sorted
        h.addData(e.path.toUtf8());
        h.addData(QByteArray(1, '\0'));
        h.addData(QByteArray::number(e.size));
        h.addData(QByteArray(1, '\0'));
        h.addData(e.sha256.toLatin1());
        h.addData("\n");
    }
    return QString::fromLatin1(h.result().toHex());
}

namespace {
QString hashFile(const QString &path, qint64 *size)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    QCryptographicHash h(QCryptographicHash::Sha256);
    qint64 n = 0;
    while (!f.atEnd()) {
        const QByteArray b = f.read(1 << 20);
        if (b.isEmpty())
            break;
        h.addData(b);
        n += b.size();
    }
    if (f.error() != QFileDevice::NoError)
        return QString();
    *size = n;
    return QString::fromLatin1(h.result().toHex());
}
} // namespace

QString Manifest::fileSha256(const QString &path, qint64 *size) { return hashFile(path, size); }

bool Manifest::scan(const QString &folder, const QString &cacheFile, const QHash<QString, qint64> &revs, Manifest *out, QString *error)
{
    const QDir root(folder);
    if (!root.exists()) {
        if (error)
            *error = QStringLiteral("the folder is not there");
        return false;
    }
    QJsonObject cache;
    if (!cacheFile.isEmpty())
        cache = QJsonDocument::fromJson(vault::readOrEmpty(cacheFile)).object();
    QJsonObject fresh;
    bool changed = false;
    Manifest m;
    m.revs = revs;
    QDirIterator it(folder, QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString abs = it.next();
        const QString rel = root.relativeFilePath(abs);
        const PathInfo info = classify(rel);
        if (!info.replicated())
            continue;
        const QFileInfo fi(abs);
        const qint64 mtime = fi.lastModified().toMSecsSinceEpoch();
        ManifestEntry e;
        e.path = rel;
        const QJsonObject c = cache.value(rel).toObject();
        if (c.value(QStringLiteral("size")).toDouble(-1) == double(fi.size()) && c.value(QStringLiteral("mtime")).toDouble(-1) == double(mtime)
            && !c.value(QStringLiteral("sha")).toString().isEmpty()) {
            e.size = fi.size();
            e.sha256 = c.value(QStringLiteral("sha")).toString();
        } else {
            qint64 size = 0;
            e.sha256 = hashFile(abs, &size);
            if (e.sha256.isEmpty()) {
                if (error)
                    *error = QStringLiteral("%1 cannot be read").arg(rel);
                return false;
            }
            e.size = size;
            changed = true;
        }
        e.rev = info.resource.isEmpty() ? 0 : revs.value(info.resource);
        fresh.insert(rel, QJsonObject{{QStringLiteral("size"), double(e.size)}, {QStringLiteral("mtime"), double(mtime)}, {QStringLiteral("sha"), e.sha256}});
        m.entries << e;
    }
    std::sort(m.entries.begin(), m.entries.end(), [](const ManifestEntry &a, const ManifestEntry &b) { return a.path < b.path; });
    if (!cacheFile.isEmpty() && (changed || fresh.keys() != cache.keys())) {
        QDir().mkpath(QFileInfo(cacheFile).absolutePath());
        safesave::writeFile(cacheFile, QJsonDocument(fresh).toJson(QJsonDocument::Compact)); // (a cache: a failure costs time only)
    }
    *out = m;
    return true;
}

ManifestDiff compare(const Manifest &wanted, const Manifest &local)
{
    ManifestDiff d;
    for (const ManifestEntry &e : wanted.entries) {
        const ManifestEntry *l = local.find(e.path);
        if (!l || l->sha256 != e.sha256 || l->size != e.size)
            d.fetch << e.path;
    }
    for (const ManifestEntry &e : local.entries)
        if (!wanted.find(e.path))
            d.remove << e.path;
    return d;
}

} // namespace collab
} // namespace qb
