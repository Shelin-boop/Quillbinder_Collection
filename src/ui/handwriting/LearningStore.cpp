#include "ui/handwriting/LearningStore.h"

#include "ui/handwriting/HandwritingSettings.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace qb {
namespace handwriting {

QString LearningStore::folder()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/handwriting-lines");
}

bool LearningStore::add(const QImage &picture, const QString &correctedText)
{
    if (!HandwritingConfig::instance().get().learning || picture.isNull() || correctedText.trimmed().isEmpty())
        return false;
    QDir().mkpath(folder());
    const QString base = QStringLiteral("%1/%2-%3").arg(folder(), QString::number(QDateTime::currentMSecsSinceEpoch()),
                                                        QString::number(count()));
    if (!picture.save(base + QStringLiteral(".png"), "PNG"))
        return false;
    QFile f(base + QStringLiteral(".txt"));
    if (!f.open(QIODevice::WriteOnly)) {
        QFile::remove(base + QStringLiteral(".png"));
        return false;
    }
    f.write(correctedText.toUtf8());
    return true;
}

int LearningStore::count()
{
    return int(QDir(folder()).entryList({QStringLiteral("*.png")}, QDir::Files).size());
}

int LearningStore::removeAll()
{
    QDir d(folder());
    int n = 0;
    for (const QString &f : d.entryList({QStringLiteral("*.png"), QStringLiteral("*.txt")}, QDir::Files))
        if (d.remove(f) && f.endsWith(QLatin1String(".png")))
            ++n;
    return n;
}

} // namespace handwriting
} // namespace qb
