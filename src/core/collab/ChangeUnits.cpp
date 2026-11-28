#include "core/collab/ChangeUnits.h"

#include "core/collab/IndexDiff.h"
#include "core/collab/PathClass.h"
#include "core/crypto/Vault.h"

#include <QFileInfo>
#include <algorithm>

namespace qb {
namespace collab {

namespace {
struct Items {
    QStringList resources;
    QList<Op> ops;
    QList<proto::FileOp> files;
};

QString find(QHash<QString, QString> &parent, const QString &x)
{
    QString r = x;
    while (parent.value(r, r) != r)
        r = parent.value(r);
    // path compression
    QString c = x;
    while (parent.value(c, c) != c) {
        const QString n = parent.value(c);
        parent[c] = r;
        c = n;
    }
    return r;
}
} // namespace

UnitsResult unitsFromChanges(const ChangeSet &set, const QString &folder)
{
    UnitsResult out;
    QList<Items> items;
    for (const FileChange &c : set.files) {
        const PathInfo info = c.info;
        if (info.kind == PathKind::Index) {
            if (c.kind != FileChange::Put) {
                out.skipped << c.path;
                continue;
            }
            QByteArray base;
            QString why;
            if (!vault::readFile(folder + QLatin1Char('/') + c.path, &base, &why)) {
                out.ok = false;
                out.error = why;
                return out;
            }
            QString err;
            const OpList ops = diffIndex(base, c.data, &err);
            if (!err.isEmpty()) {
                out.ok = false;
                out.error = err;
                return out;
            }
            for (const Op &op : ops) {
                Items it;
                it.resources = proto::resourcesOfOp(op);
                it.ops << op;
                items << it;
            }
            continue;
        }
        if (!info.capturable() || info.hostOnly()) {
            out.skipped << c.path;
            continue;
        }
        bool refused = false;
        Items it;
        proto::FileOp f;
        f.path = c.path;
        if (c.kind == FileChange::Put) {
            f.kind = proto::FileOp::Put;
            f.hash = QString::fromLatin1(c.sha256);
            f.size = c.data.size();
            out.blobs.insert(f.hash, c.data);
            it.resources = proto::resourcesOfPath(c.path, &refused);
        } else {
            f.kind = c.kind == FileChange::Remove ? proto::FileOp::Remove : proto::FileOp::RemoveTree;
            // a tree is classified as a file inside it
            it.resources = proto::resourcesOfPath(c.kind == FileChange::RemoveTree ? c.path + QStringLiteral("/_") : c.path, &refused);
        }
        if (refused) {
            out.skipped << c.path;
            continue;
        }
        it.files << f;
        items << it;
    }
    // resources joined by one operation or file are one unit
    QHash<QString, QString> parent;
    for (const Items &it : items)
        for (const QString &r : it.resources) {
            if (!parent.contains(r))
                parent.insert(r, r);
            if (r != it.resources.first())
                parent[find(parent, r)] = find(parent, it.resources.first());
        }
    QHash<QString, int> unitOf;
    for (const Items &it : items) {
        if (it.resources.isEmpty())
            continue;
        const QString root = find(parent, it.resources.first());
        int idx = unitOf.value(root, -1);
        if (idx < 0) {
            idx = int(out.units.size());
            unitOf.insert(root, idx);
            proto::Unit u;
            u.name = root;
            out.units << u;
        }
        proto::Unit &u = out.units[idx];
        u.ops << it.ops;
        u.files << it.files;
    }
    return out;
}

} // namespace collab
} // namespace qb
