#include "core/collab/PathClass.h"

#include "core/PathSafety.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QSet>

namespace qb {
namespace collab {

namespace {

PathInfo make(PathKind k, const QString &resource = QString(), const QString &id = QString())
{
    PathInfo p;
    p.kind = k;
    p.resource = resource;
    p.id = id;
    return p;
}

PathInfo classifyContent(const QStringList &parts)
{
    // content/<id>/<rest...>
    if (parts.size() < 3 || !pathsafety::isSafeId(parts.at(1)))
        return make(PathKind::Unknown);
    const QString id = parts.at(1);
    const QString name = parts.at(2);
    if (parts.size() >= 4 && name == QLatin1String("snapshots"))
        return make(PathKind::Snapshot, QStringLiteral("snapshots:") + id, id);
    if (parts.size() == 3) {
        if (name.startsWith(QLatin1String("text.")) && name.size() > 5)
            return make(PathKind::Text, QStringLiteral("doc:") + id, id);
        if (name.startsWith(QLatin1String("notes.")) && name.size() > 6)
            return make(PathKind::Notes, QStringLiteral("doc:") + id, id);
        if (name == QLatin1String("mindmap.scap"))
            return make(PathKind::MindMap, QStringLiteral("mindmap:") + id, id);
        if (name.startsWith(QLatin1String("file."))) {
            if (!pathsafety::isSafeExtension(name.mid(5)))
                return make(PathKind::Unknown);
            return make(PathKind::MediaFile, QStringLiteral("doc:") + id, id);
        }
    }
    // other files below the document's folder (pictures of the text, the synopsis image): its pictures
    return make(PathKind::DocPicture, QStringLiteral("doc:") + id, id);
}

PathInfo classifySettings(const QStringList &parts)
{
    const QString name = parts.at(1);
    if (parts.size() > 2) // settings/conflicts/..., any other folder
        return make(name == QLatin1String("conflicts") ? PathKind::DeviceLocal : PathKind::Unknown);
    if (name == QLatin1String("journal.json") || name == QLatin1String("lock") || name.startsWith(QLatin1String("lock-work-"))
        || name.startsWith(QLatin1String("lock."))
        || name == QLatin1String("ui.json") || (name.startsWith(QLatin1String("ui-")) && name.endsWith(QLatin1String(".json")))
        || name.startsWith(QLatin1String("spare-copy")) || name == QLatin1String("scrivener-sync.json")
        || name == QLatin1String("backup-id.txt") || name == QLatin1String("options.ini"))
        return make(PathKind::DeviceLocal);
    if (name.endsWith(QLatin1String(".json")) || name.endsWith(QLatin1String(".ini")) || name.endsWith(QLatin1String(".xml")))
        return make(PathKind::Settings, QStringLiteral("settings"));
    return make(PathKind::Unknown);
}

} // namespace

PathInfo classify(const QString &rel)
{
    if (rel.isEmpty() || rel.startsWith(QLatin1Char('/')) || rel.contains(QLatin1Char('\\')))
        return make(PathKind::Unknown);
    // temporary files of a save in progress, wherever they are
    if (rel.endsWith(QLatin1String(".qbstage")))
        return make(PathKind::DeviceLocal);
    const QStringList parts = rel.split(QLatin1Char('/'));
    for (const QString &p : parts)
        if (p.isEmpty() || p == QLatin1String(".") || p == QLatin1String(".."))
            return make(PathKind::Unknown);
    const QString top = parts.first();
    if (parts.size() == 1) {
        if (top.endsWith(QLatin1String(".qbx")) && top.size() > 4)
            return make(PathKind::Index, QStringLiteral("index"));
        if (top == QLatin1String("contents-by-title.txt"))
            return make(PathKind::Derived);
        if (top == QLatin1String("encryption.json"))
            return make(PathKind::Marker, QStringLiteral("project"));
        if (top == QLatin1String("livesync-replica.json"))
            return make(PathKind::Internal);
        return make(PathKind::Unknown);
    }
    if (top == QLatin1String("content"))
        return classifyContent(parts);
    if (top == QLatin1String("snapshots")) {
        if (parts.size() >= 3 && pathsafety::isSafeId(parts.at(1)))
            return make(PathKind::Snapshot, QStringLiteral("snapshots:") + parts.at(1), parts.at(1));
        return make(PathKind::Unknown);
    }
    if (top == QLatin1String("settings"))
        return classifySettings(parts);
    if (top == QLatin1String("livesync") || top == QLatin1String("livesync-local"))
        return make(PathKind::Internal);
    if (top == QLatin1String("objects")) {
        if (parts.size() == 2 && parts.at(1) == QLatin1String("schema.json"))
            return make(PathKind::ObjectSchema, QStringLiteral("object:schema"));
        if (parts.at(1) == QLatin1String("images") && parts.size() >= 3)
            return make(PathKind::ObjectImage, QStringLiteral("object:images"));
        if (parts.size() == 3 && parts.at(2).endsWith(QLatin1String(".json"))) {
            const QString oid = parts.at(2).left(parts.at(2).size() - 5);
            if (pathsafety::isSafeId(oid) && pathsafety::isSafeId(parts.at(1)))
                return make(PathKind::Object, QStringLiteral("object:") + oid, oid);
        }
        return make(PathKind::Unknown);
    }
    return make(PathKind::Unknown);
}

bool isAcceptablePath(const QString &rel, QString *why)
{
    auto no = [&](const char *text) {
        if (why)
            *why = QLatin1String(text);
        return false;
    };
    if (!pathsafety::isSafeRelative(rel))
        return no("not a plain relative path");
    if (rel.toUtf8().size() > 240)
        return no("longer than 240 bytes");
    for (const QChar c : rel)
        if (c.unicode() < 0x20 || c.unicode() == 0x7f)
            return no("holds a control character");
    static const QSet<QString> reserved = {QStringLiteral("con"),  QStringLiteral("prn"),  QStringLiteral("aux"),
                                           QStringLiteral("nul"),  QStringLiteral("com1"), QStringLiteral("com2"),
                                           QStringLiteral("com3"), QStringLiteral("com4"), QStringLiteral("com5"),
                                           QStringLiteral("com6"), QStringLiteral("com7"), QStringLiteral("com8"),
                                           QStringLiteral("com9"), QStringLiteral("lpt1"), QStringLiteral("lpt2"),
                                           QStringLiteral("lpt3"), QStringLiteral("lpt4"), QStringLiteral("lpt5"),
                                           QStringLiteral("lpt6"), QStringLiteral("lpt7"), QStringLiteral("lpt8"),
                                           QStringLiteral("lpt9")};
    for (const QString &part : rel.split(QLatin1Char('/'))) {
        const QString stem = part.section(QLatin1Char('.'), 0, 0).toLower();
        if (reserved.contains(stem))
            return no("holds a name reserved on Windows");
        if (part.endsWith(QLatin1Char(' ')) || part.endsWith(QLatin1Char('.')))
            return no("a part ends in a space or a dot");
    }
    return true;
}

QStringList caseCollisions(const QStringList &paths)
{
    QStringList out;
    QHash<QString, QString> seen;
    for (const QString &p : paths) {
        const QString key = p.toLower();
        auto it = seen.constFind(key);
        if (it == seen.constEnd())
            seen.insert(key, p);
        else if (*it != p)
            out << p;
    }
    return out;
}

bool symlinkOnTheWay(const QString &folder, const QString &rel)
{
    QString cur = QDir::cleanPath(folder);
    for (const QString &part : rel.split(QLatin1Char('/'), Qt::SkipEmptyParts)) {
        cur += QLatin1Char('/') + part;
        if (QFileInfo(cur).isSymLink())
            return true;
        if (!QFileInfo::exists(cur))
            return false;
    }
    return false;
}

} // namespace collab
} // namespace qb
