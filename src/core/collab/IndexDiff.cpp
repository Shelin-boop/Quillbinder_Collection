#include "core/collab/IndexDiff.h"

#include "core/ProjectTypes.h"

#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QSet>
#include <QTextStream>

namespace qb {
namespace collab {

QStringList derivedItemAttributes() { return {QStringLiteral("modified"), QStringLiteral("words"), QStringLiteral("chars")}; }

QString opName(Op::Kind k)
{
    switch (k) {
    case Op::ItemCreate: return QStringLiteral("item.create");
    case Op::ItemMove: return QStringLiteral("item.move");
    case Op::ItemTrash: return QStringLiteral("item.trash");
    case Op::ItemFields: return QStringLiteral("item.fields");
    case Op::ItemDelete: return QStringLiteral("item.delete");
    case Op::Section: return QStringLiteral("project.section");
    }
    return QString();
}

namespace {

// the attributes of a section's element that the host computes
QStringList derivedSectionAttributes(const QString &name)
{
    if (name == QLatin1String("Targets"))
        return {QStringLiteral("sessionStart"), QStringLiteral("sessionStartDraft"), QStringLiteral("sessionStartTotal"),
                QStringLiteral("sessionStartDraftChars"), QStringLiteral("sessionStartTotalChars")};
    return {};
}
bool derivedSection(const QString &name) { return name == QLatin1String("History") || name == QLatin1String("Binder"); }

QString xmlOf(const QDomElement &e)
{
    QString out;
    QTextStream ts(&out);
    e.save(ts, -1);
    return out;
}

// a copy of `e` without its <Children> and the derived attributes
QDomElement fieldsCopy(const QDomElement &e)
{
    QDomElement c = e.cloneNode(true).toElement();
    const QDomElement ch = c.firstChildElement(QStringLiteral("Children"));
    if (!ch.isNull())
        c.removeChild(ch);
    for (const QString &a : derivedItemAttributes())
        c.removeAttribute(a);
    return c;
}

QDomElement sectionCopy(const QDomElement &e)
{
    QDomElement c = e.cloneNode(true).toElement();
    for (const QString &a : derivedSectionAttributes(e.tagName()))
        c.removeAttribute(a);
    return c;
}

struct Node {
    QString id, parent, after; // after: the previous sibling's id in this tree
    QDomElement element;
    QString fields;            // fieldsCopy as text
    bool isTrash = false;
};

// every item of the binder, level by level (the items of one parent in sibling order); false when an item has no id or an
// id twice. The positions in `index` are those of `order`.
bool collect(const QDomElement &binder, QList<Node> *order, QHash<QString, int> *index, QString *error)
{
    const QString trashType = itemTypeToString(ItemType::Trash);
    struct Work {
        QDomElement list; // Binder or Children
        QString parent;
    };
    QList<Work> queue{{binder, QString()}};
    for (qsizetype q = 0; q < queue.size(); ++q) {
        const Work w = queue.at(q);
        QString prev;
        for (QDomElement e = w.list.firstChildElement(QStringLiteral("Item")); !e.isNull();
             e = e.nextSiblingElement(QStringLiteral("Item"))) {
            Node n;
            n.id = e.attribute(QStringLiteral("id"));
            if (n.id.isEmpty() || index->contains(n.id)) {
                if (error)
                    *error = QStringLiteral("an item has no id or an id twice: %1").arg(n.id);
                return false;
            }
            n.parent = w.parent;
            n.after = prev;
            n.element = e;
            n.fields = xmlOf(fieldsCopy(e));
            n.isTrash = e.attribute(QStringLiteral("type")) == trashType;
            prev = n.id;
            index->insert(n.id, int(order->size()));
            order->append(n);
            const QDomElement ch = e.firstChildElement(QStringLiteral("Children"));
            if (!ch.isNull())
                queue.append({ch, n.id});
        }
    }
    return true;
}

QDomElement rootOf(QDomDocument &dom, const QByteArray &bytes, QString *error)
{
    QString msg;
    if (!dom.setContent(bytes, &msg) || dom.documentElement().tagName() != QLatin1String("QuillbinderProject")) {
        if (error)
            *error = msg.isEmpty() ? QStringLiteral("not a project file") : msg;
        return QDomElement();
    }
    return dom.documentElement();
}

QString rootAttributes(const QDomElement &root)
{
    QDomElement c = root.ownerDocument().createElement(QStringLiteral("QuillbinderProject"));
    const QDomNamedNodeMap m = root.attributes();
    for (int i = 0; i < m.size(); ++i) {
        const QString n = m.item(i).nodeName();
        if (n == QLatin1String("app") || n == QLatin1String("needs") || n == QLatin1String("version"))
            continue;
        c.setAttribute(n, m.item(i).nodeValue());
    }
    return xmlOf(c);
}

} // namespace

OpList diffIndex(const QByteArray &from, const QByteArray &to, QString *error)
{
    OpList ops;
    QDomDocument d1, d2;
    const QDomElement r1 = rootOf(d1, from, error);
    if (r1.isNull())
        return ops;
    const QDomElement r2 = rootOf(d2, to, error);
    if (r2.isNull())
        return ops;

    // ---- items
    QList<Node> oldNodes, newNodes;
    QHash<QString, int> oldIndex, newIndex;
    if (!collect(r1.firstChildElement(QStringLiteral("Binder")), &oldNodes, &oldIndex, error)
        || !collect(r2.firstChildElement(QStringLiteral("Binder")), &newNodes, &newIndex, error))
        return OpList();
    // collect() walks level by level; the new tree must be walked in true document order (parents before children, siblings
    // in order) for the creates and the placing below: sort by a path of sibling numbers
    auto documentOrder = [](QList<Node> &nodes, QHash<QString, int> &index) {
        QHash<QString, QList<int>> children; // parent id -> positions in `nodes`, in sibling order
        for (int i = 0; i < nodes.size(); ++i)
            children[nodes.at(i).parent] << i;
        QList<Node> sorted;
        // iterative preorder: the last pushed is processed first
        struct F { QString parent; int next; };
        QList<F> frames{{QString(), 0}};
        while (!frames.isEmpty()) {
            F &f = frames.last();
            const QList<int> &kids = children[f.parent];
            if (f.next >= kids.size()) {
                frames.removeLast();
                continue;
            }
            const Node &n = nodes.at(kids.at(f.next++));
            sorted << n;
            frames.append({n.id, 0});
        }
        nodes = sorted;
        index.clear();
        for (int i = 0; i < nodes.size(); ++i)
            index.insert(nodes.at(i).id, i);
    };
    documentOrder(newNodes, newIndex);

    QSet<QString> dead;
    for (const Node &n : std::as_const(oldNodes))
        if (!newIndex.contains(n.id))
            dead.insert(n.id);

    // the old tree as a simulation that the moves change: parent -> ordered child ids
    QHash<QString, QString> simParent;
    QHash<QString, QList<QString>> simKids;
    for (const Node &n : std::as_const(oldNodes)) {
        simParent[n.id] = n.parent;
        simKids[n.parent] << n.id;
    }
    auto previousAlive = [&](const QString &id) {
        const QList<QString> &kids = simKids[simParent.value(id)];
        for (qsizetype i = kids.indexOf(id) - 1; i >= 0; --i)
            if (!dead.contains(kids.at(i)))
                return kids.at(i);
        return QString();
    };
    auto isUnder = [&](QString id, const QString &ancestor) { // the old/simulated ancestry
        while (!id.isEmpty()) {
            if (id == ancestor)
                return true;
            id = simParent.value(id);
        }
        return false;
    };
    QString trashId;
    for (const Node &n : std::as_const(newNodes))
        if (n.isTrash)
            trashId = n.id;

    OpList moves, fields, creates;
    for (const Node &n : std::as_const(newNodes)) {
        if (!oldIndex.contains(n.id)) {
            Op op;
            op.kind = Op::ItemCreate;
            op.id = n.id;
            op.parent = n.parent;
            op.after = n.after;
            op.element = n.fields;
            creates << op;
            // it exists from now on in the simulation, at its place
            simParent[n.id] = n.parent;
            QList<QString> &kids = simKids[n.parent];
            kids.insert(n.after.isEmpty() ? 0 : int(kids.indexOf(n.after)) + 1, n.id);
            continue;
        }
        // an existing item: in the right place?
        const bool sameParent = simParent.value(n.id) == n.parent;
        if (!sameParent || previousAlive(n.id) != n.after) {
            Op op;
            op.id = n.id;
            op.parent = n.parent;
            op.after = n.after;
            const bool toTrash = !trashId.isEmpty() && n.parent == trashId && !isUnder(n.id, trashId);
            op.kind = toTrash ? Op::ItemTrash : Op::ItemMove;
            moves << op;
            // the simulation
            simKids[simParent.value(n.id)].removeAll(n.id);
            simParent[n.id] = n.parent;
            QList<QString> &kids = simKids[n.parent];
            // after the previous sibling, counting the dead ones that stay in the list
            kids.insert(n.after.isEmpty() ? 0 : int(kids.indexOf(n.after)) + 1, n.id);
        }
        if (oldNodes.at(oldIndex.value(n.id)).fields != n.fields) {
            Op op;
            op.kind = Op::ItemFields;
            op.id = n.id;
            op.element = n.fields;
            fields << op;
        }
    }
    ops << creates << moves << fields;
    // deletes: the dead items whose parent lives (or is the top level), the deepest last
    for (const Node &n : std::as_const(oldNodes)) {
        if (!dead.contains(n.id) || dead.contains(simParent.value(n.id)))
            continue;
        Op op;
        op.kind = Op::ItemDelete;
        op.id = n.id;
        ops << op;
    }

    // ---- the root's own attributes and the sections
    if (rootAttributes(r1) != rootAttributes(r2)) {
        Op op;
        op.kind = Op::Section;
        op.name = QStringLiteral("@root");
        op.element = rootAttributes(r2);
        ops << op;
    }
    QHash<QString, QString> oldSections;
    QStringList newOrder;
    for (QDomElement e = r1.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
        if (!derivedSection(e.tagName()))
            oldSections.insert(e.tagName(), xmlOf(sectionCopy(e)));
    for (QDomElement e = r2.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (derivedSection(e.tagName()))
            continue;
        newOrder << e.tagName();
        const QString now = xmlOf(sectionCopy(e));
        if (oldSections.value(e.tagName()) != now || !oldSections.contains(e.tagName())) {
            Op op;
            op.kind = Op::Section;
            op.name = e.tagName();
            op.element = now;
            ops << op;
        }
        oldSections.remove(e.tagName());
    }
    for (auto it = oldSections.constBegin(); it != oldSections.constEnd(); ++it) {
        Op op;
        op.kind = Op::Section;
        op.name = it.key();
        ops << op; // removed
    }
    return ops;
}

QByteArray canonicalIndex(const QByteArray &bytes, QString *error)
{
    QDomDocument dom;
    const QDomElement root = rootOf(dom, bytes, error);
    if (root.isNull())
        return QByteArray();
    QString out;
    QTextStream ts(&out);
    ts << rootAttributes(root) << '\n';
    // the items, one line each in document order: id, parent, previous sibling, fields
    QList<Node> nodes;
    QHash<QString, int> idx;
    if (!collect(root.firstChildElement(QStringLiteral("Binder")), &nodes, &idx, error))
        return QByteArray();
    // document order
    QHash<QString, QList<int>> kids;
    for (int i = 0; i < nodes.size(); ++i)
        kids[nodes.at(i).parent] << i;
    struct F { QString parent; int next; };
    QList<F> frames{{QString(), 0}};
    while (!frames.isEmpty()) {
        F &f = frames.last();
        const QList<int> &k = kids[f.parent];
        if (f.next >= k.size()) {
            frames.removeLast();
            continue;
        }
        const Node &n = nodes.at(k.at(f.next++));
        ts << n.parent << '|' << n.after << '|' << n.fields << '\n';
        frames.append({n.id, 0});
    }
    QStringList sections;
    for (QDomElement e = root.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
        if (!derivedSection(e.tagName()))
            sections << xmlOf(sectionCopy(e));
    sections.sort();
    ts << sections.join(QLatin1Char('\n')) << '\n';
    return out.toUtf8();
}

} // namespace collab
} // namespace qb
