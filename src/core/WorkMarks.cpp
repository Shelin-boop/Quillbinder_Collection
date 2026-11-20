#include "core/WorkMarks.h"
#include "core/SafeSave.h"
#include "core/SyncSafety.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace qb {
namespace workmarks {

static QString prefixOf(const QString &lockFile) { return QFileInfo(lockFile).fileName() + QLatin1String("-work-"); }

QString fileFor(const QString &lockFile)
{
    return QFileInfo(lockFile).absolutePath() + QLatin1Char('/') + prefixOf(lockFile) + syncsafe::machineKey() + QLatin1String(".json");
}

bool write(const QString &lockFile, const QDateTime &opened, const QStringList &docs)
{
    QJsonObject o;
    o.insert(QStringLiteral("format"), 1);
    o.insert(QStringLiteral("device"), syncsafe::deviceName());
    o.insert(QStringLiteral("key"), syncsafe::machineKey());
    o.insert(QStringLiteral("opened"), opened.toUTC().toString(Qt::ISODate));
    o.insert(QStringLiteral("alive"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    o.insert(QStringLiteral("docs"), QJsonArray::fromStringList(docs));
    QDir().mkpath(QFileInfo(lockFile).absolutePath());
    QSaveFile f(fileFor(lockFile));
    if (!f.open(QIODevice::WriteOnly))
        return false;
    safesave::write(f, QJsonDocument(o).toJson(QJsonDocument::Compact));
    return safesave::commit(f);
}

void removeOwn(const QString &lockFile)
{
    QFile::remove(fileFor(lockFile));
}

QList<Entry> others(const QString &lockFile, qint64 staleAfterSecs)
{
    QList<Entry> out;
    const QDir dir(QFileInfo(lockFile).absolutePath());
    const syncsafe::LockInfo lock = syncsafe::readLock(lockFile);
    const QDateTime now = QDateTime::currentDateTimeUtc();
    for (const QFileInfo &fi : dir.entryInfoList({prefixOf(lockFile) + QLatin1String("*.json")}, QDir::Files)) {
        if (fi.size() > 2 * 1024 * 1024)
            continue;   // no list of documents is that long
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly))
            continue;
        const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
        Entry e;
        e.key = o.value(QStringLiteral("key")).toString();
        if (e.key.isEmpty() || e.key == syncsafe::machineKey())
            continue;
        e.device = o.value(QStringLiteral("device")).toString().left(200);
        e.opened = QDateTime::fromString(o.value(QStringLiteral("opened")).toString(), Qt::ISODate);
        e.alive = QDateTime::fromString(o.value(QStringLiteral("alive")).toString(), Qt::ISODate);
        if (lock.exists && lock.key == e.key && lock.alive.isValid() && (!e.alive.isValid() || lock.alive > e.alive))
            e.alive = lock.alive;
        if (!e.alive.isValid() || e.alive.secsTo(now) > staleAfterSecs)
            continue;
        for (const QJsonValue &v : o.value(QStringLiteral("docs")).toArray())
            if (!v.toString().isEmpty())
                e.docs << v.toString();
        if (!e.docs.isEmpty())
            out << e;
    }
    return out;
}

} // namespace workmarks
} // namespace qb
