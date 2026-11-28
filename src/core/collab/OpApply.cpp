#include "core/collab/OpApply.h"

#include "core/BinderModel.h"
#include "core/PathSafety.h"
#include "core/Project.h"

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QSet>
#include <QTextStream>

namespace qb {
namespace collab {

namespace {

QDomElement parseElement(const QString &xml, QDomDocument &into, QString *error)
{
    QDomDocument d;
    QString msg;
    if (!d.setContent(xml, &msg)) {
        if (error)
            *error = QStringLiteral("an operation's element is not XML (%1)").arg(msg);
        return QDomElement();
    }
    return into.importNode(d.documentElement(), true).toElement();
}

// the item elements by id, kept in step with the tree
struct Tree {
    QDomDocument dom;
    QDomElement root;
    QDomElement binder;
    QHash<QString, QDomElement> items;

    void index(const QDomElement &list)
    {
        QList<QDomElement> queue{list};
        for (qsizetype q = 0; q < queue.size(); ++q)
            for (QDomElement e = queue.at(q).firstChildElement(QStringLiteral("Item")); !e.isNull();
                 e = e.nextSiblingElement(QStringLiteral("Item"))) {
                items.insert(e.attribute(QStringLiteral("id")), e);
                const QDomElement ch = e.firstChildElement(QStringLiteral("Children"));
                if (!ch.isNull())
                    queue.append(ch);
            }
    }
    QDomElement listOf(const QString &parentId, bool create)
    {
        if (parentId.isEmpty())
            return binder;
        QDomElement p = items.value(parentId);
        if (p.isNull())
            return QDomElement();
        QDomElement ch = p.firstChildElement(QStringLiteral("Children"));
        if (ch.isNull() && create) {
            ch = dom.createElement(QStringLiteral("Children"));
            p.appendChild(ch);
        }
        return ch;
    }
    bool isUnder(QDomElement e, const QDomElement &ancestor) const
    {
        for (; !e.isNull(); e = e.parentNode().parentNode().toElement()) // Item -> Children/Binder -> Item
            if (e == ancestor)
                return true;
        return false;
    }
    void tidy(const QDomElement &list)
    {
        if (list.tagName() == QLatin1String("Children") && !list.firstChildElement().isElement())
            list.parentNode().removeChild(list);
    }
    bool place(QDomElement item, const QString &parentId, const QString &afterId, QString *error)
    {
        QDomElement list = listOf(parentId, true);
        if (list.isNull()) {
            if (error)
                *error = QStringLiteral("the parent %1 is not there").arg(parentId);
            return false;
        }
        QDomElement after;
        if (!afterId.isEmpty()) {
            after = items.value(afterId);
            if (after.isNull() || after.parentNode().toElement() != list) {
                if (error)
                    *error = QStringLiteral("the item %1 is not in the place the operation names").arg(afterId);
                return false;
            }
        }
        if (after.isNull())
            list.insertBefore(item, list.firstChild());
        else
            list.insertAfter(item, after);
        return true;
    }
};

bool derivedAttr(const QString &n) { return derivedItemAttributes().contains(n); }

} // namespace

bool applyToIndex(QByteArray *index, const OpList &ops, QString *error)
{
    Tree t;
    QString msg;
    if (!t.dom.setContent(*index, &msg) || t.dom.documentElement().tagName() != QLatin1String("QuillbinderProject")) {
        if (error)
            *error = QStringLiteral("not a project file: %1").arg(msg);
        return false;
    }
    t.root = t.dom.documentElement();
    t.binder = t.root.firstChildElement(QStringLiteral("Binder"));
    if (t.binder.isNull()) {
        t.binder = t.dom.createElement(QStringLiteral("Binder"));
        t.root.appendChild(t.binder);
    }
    t.index(t.binder);

    for (const Op &op : ops) {
        switch (op.kind) {
        case Op::ItemCreate: {
            if (t.items.contains(op.id)) {
                if (error)
                    *error = QStringLiteral("the item %1 is there already").arg(op.id);
                return false;
            }
            QDomElement e = parseElement(op.element, t.dom, error);
            if (e.isNull() || e.tagName() != QLatin1String("Item"))
                return false;
            e.setAttribute(QStringLiteral("id"), op.id);
            // what the host computes starts at its default
            if (!e.hasAttribute(QStringLiteral("modified")))
                e.setAttribute(QStringLiteral("modified"), e.attribute(QStringLiteral("created")));
            if (!e.hasAttribute(QStringLiteral("words")))
                e.setAttribute(QStringLiteral("words"), 0);
            if (!e.hasAttribute(QStringLiteral("chars")))
                e.setAttribute(QStringLiteral("chars"), 0);
            const QDomElement kids = e.firstChildElement(QStringLiteral("Children"));
            if (!kids.isNull())
                e.removeChild(kids);
            if (!t.place(e, op.parent, op.after, error))
                return false;
            t.items.insert(op.id, e);
            break;
        }
        case Op::ItemMove:
        case Op::ItemTrash: {
            QDomElement e = t.items.value(op.id);
            if (e.isNull()) {
                if (error)
                    *error = QStringLiteral("the item %1 is not there").arg(op.id);
                return false;
            }
            const QDomElement target = op.parent.isEmpty() ? t.binder : t.items.value(op.parent);
            if (!op.parent.isEmpty() && target.isNull()) {
                if (error)
                    *error = QStringLiteral("the parent %1 is not there").arg(op.parent);
                return false;
            }
            if (!op.parent.isEmpty() && t.isUnder(target, e)) {
                if (error)
                    *error = QStringLiteral("the item %1 cannot go into itself").arg(op.id);
                return false;
            }
            QDomElement oldList = e.parentNode().toElement();
            oldList.removeChild(e);
            t.tidy(oldList);
            if (!t.place(e, op.parent, op.after, error))
                return false;
            break;
        }
        case Op::ItemFields: {
            QDomElement e = t.items.value(op.id);
            if (e.isNull()) {
                if (error)
                    *error = QStringLiteral("the item %1 is not there").arg(op.id);
                return false;
            }
            QDomElement fresh = parseElement(op.element, t.dom, error);
            if (fresh.isNull() || fresh.tagName() != QLatin1String("Item"))
                return false;
            // the host's own attributes stay; everything else is the operation's, the children stay as they are
            for (const QString &a : derivedItemAttributes())
                if (e.hasAttribute(a))
                    fresh.setAttribute(a, e.attribute(a));
            const QDomElement kids = e.firstChildElement(QStringLiteral("Children"));
            fresh.setAttribute(QStringLiteral("id"), op.id);
            if (!kids.isNull())
                fresh.appendChild(e.removeChild(kids));
            e.parentNode().replaceChild(fresh, e);
            t.items.insert(op.id, fresh);
            // (the children's parent element changed: the index keeps pointing at the same child elements)
            break;
        }
        case Op::ItemDelete: {
            QDomElement e = t.items.value(op.id);
            if (e.isNull()) {
                if (error)
                    *error = QStringLiteral("the item %1 is not there").arg(op.id);
                return false;
            }
            QList<QDomElement> queue{e};
            for (qsizetype q = 0; q < queue.size(); ++q) {
                t.items.remove(queue.at(q).attribute(QStringLiteral("id")));
                const QDomElement ch = queue.at(q).firstChildElement(QStringLiteral("Children"));
                for (QDomElement c = ch.firstChildElement(QStringLiteral("Item")); !c.isNull();
                     c = c.nextSiblingElement(QStringLiteral("Item")))
                    queue.append(c);
            }
            QDomElement list = e.parentNode().toElement();
            list.removeChild(e);
            t.tidy(list);
            break;
        }
        case Op::Section: {
            if (op.name == QLatin1String("@root")) {
                QDomElement fresh = parseElement(op.element, t.dom, error);
                if (fresh.isNull())
                    return false;
                const QDomNamedNodeMap m = fresh.attributes();
                for (int i = 0; i < m.size(); ++i)
                    t.root.setAttribute(m.item(i).nodeName(), m.item(i).nodeValue());
                break;
            }
            if (op.name.isEmpty() || op.name == QLatin1String("Binder") || op.name == QLatin1String("History")) {
                if (error)
                    *error = QStringLiteral("the section %1 cannot be changed by an operation").arg(op.name);
                return false;
            }
            QDomElement old = t.root.firstChildElement(op.name);
            if (op.element.isEmpty()) {
                if (!old.isNull())
                    t.root.removeChild(old);
                break;
            }
            QDomElement fresh = parseElement(op.element, t.dom, error);
            if (fresh.isNull() || fresh.tagName() != op.name) {
                if (error && error->isEmpty())
                    *error = QStringLiteral("the section %1 holds another element").arg(op.name);
                return false;
            }
            if (old.isNull()) {
                t.root.appendChild(fresh);
            } else {
                // the attributes the host computes stay
                const QDomNamedNodeMap m = old.attributes();
                static const QStringList targetsOwn = {QStringLiteral("sessionStart"), QStringLiteral("sessionStartDraft"),
                                                       QStringLiteral("sessionStartTotal"), QStringLiteral("sessionStartDraftChars"),
                                                       QStringLiteral("sessionStartTotalChars")};
                if (op.name == QLatin1String("Targets"))
                    for (const QString &a : targetsOwn)
                        if (old.hasAttribute(a))
                            fresh.setAttribute(a, old.attribute(a));
                Q_UNUSED(m);
                t.root.replaceChild(fresh, old);
            }
            break;
        }
        }
    }
    QByteArray out;
    {
        QTextStream ts(&out);
        ts.setEncoding(QStringConverter::Utf8);
        t.dom.save(ts, 1);
    }
    *index = out;
    return true;
}

QStringList touchedIds(const OpList &ops)
{
    QStringList out;
    for (const Op &op : ops)
        if (op.kind != Op::Section && !op.id.isEmpty() && !out.contains(op.id))
            out << op.id;
    return out;
}

bool applyToProject(Project *p, const OpList &ops, QString *error)
{
    if (!p || !p->model()) {
        if (error)
            *error = QStringLiteral("no project");
        return false;
    }
    struct Scope {
        Project *p;
        explicit Scope(Project *project) : p(project) { p->beginRemoteApply(); }
        ~Scope() { p->endRemoteApply(); }
    } scope(p);
    BinderModel *model = p->model();
    auto parentOf = [&](const QString &id) { return id.isEmpty() ? p->root() : p->item(id); };
    auto rowAfter = [](BinderItem *parent, const QString &after) {
        if (after.isEmpty())
            return 0;
        for (int i = 0; i < parent->children.size(); ++i)
            if (parent->children.at(i)->id == after)
                return i + 1;
        return int(parent->children.size());
    };
    for (const Op &op : ops) {
        switch (op.kind) {
        case Op::ItemCreate: {
            BinderItem *parent = parentOf(op.parent);
            if (!parent || p->item(op.id) || !pathsafety::isSafeId(op.id)) {
                if (error)
                    *error = QStringLiteral("cannot create %1 (no parent, there already, or an unsafe id)").arg(op.id);
                return false;
            }
            BinderItem *it = Project::itemFromXml(op.element);
            if (!it) {
                if (error)
                    *error = QStringLiteral("the element of %1 is not an item").arg(op.id);
                return false;
            }
            it->id = op.id;
            it->wordCount = 0;
            it->charCount = 0;
            it->modified = it->created;
            model->insertExisting(parent, rowAfter(parent, op.after), it);
            break;
        }
        case Op::ItemMove:
        case Op::ItemTrash: {
            BinderItem *it = p->item(op.id);
            BinderItem *parent = parentOf(op.parent);
            if (!it || !parent || it->isRootFolder() || parent == it || parent->isDescendantOf(it)) {
                if (error)
                    *error = QStringLiteral("cannot move %1").arg(op.id);
                return false;
            }
            model->moveItems({it}, parent, rowAfter(parent, op.after));
            if (it->parent != parent) {
                if (error)
                    *error = QStringLiteral("%1 could not be moved there").arg(op.id);
                return false;
            }
            break;
        }
        case Op::ItemFields: {
            BinderItem *it = p->item(op.id);
            BinderItem *fresh = Project::itemFromXml(op.element);
            if (!it || !fresh) {
                delete fresh;
                if (error)
                    *error = QStringLiteral("cannot change the fields of %1").arg(op.id);
                return false;
            }
            const QDateTime modified = it->modified;
            const int words = it->wordCount, chars = it->charCount;
            Project::copyItemFields(it, *fresh);
            it->modified = modified;
            it->wordCount = words;
            it->charCount = chars;
            delete fresh;
            model->refresh(it);
            emit p->itemChanged(it);
            break;
        }
        case Op::ItemDelete: {
            BinderItem *it = p->item(op.id);
            if (!it || it->isRootFolder()) {
                if (error)
                    *error = QStringLiteral("cannot delete %1").arg(op.id);
                return false;
            }
            model->deletePermanently(it);
            break;
        }
        case Op::Section: {
            if (op.name == QLatin1String("@root")) {
                QDomDocument d;
                if (d.setContent(op.element) && d.documentElement().hasAttribute(QStringLiteral("title")))
                    p->setTitle(d.documentElement().attribute(QStringLiteral("title")));
                break;
            }
            QByteArray bytes = p->indexBytes();
            if (!applyToIndex(&bytes, {op}, error) || !p->applySectionsFromIndex(bytes, error))
                return false;
            break;
        }
        }
    }
    return true;
}

} // namespace collab
} // namespace qb
