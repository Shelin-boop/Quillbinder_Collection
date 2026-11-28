#pragma once
// Live Sync, the project file as operations (0.99.0). A follower's save writes a whole new project file, which is never
// sent: the file is compared with the replica's, and what differs becomes operations by the items' stable ids and by
// section (labels, statuses, keywords ...). The comparison is generic over the XML, so a new item field needs no code
// here. Derived attributes (modified, words, chars of an item; the writing history; the session counts of the targets;
// the program's version stamp) are left out: the host computes them, a client's values are never taken.
#include <QByteArray>
#include <QList>
#include <QString>

namespace qb {
namespace collab {

struct Op {
    enum Kind {
        ItemCreate, // id, parent, after, element (the item's XML without children)
        ItemMove,   // id, parent, after
        ItemTrash,  // id, parent (the Trash folder), after: a move into the Trash
        ItemFields, // id, element
        ItemDelete, // id (the item with everything inside it)
        Section     // name (an element name of the project file, or "@root" for the root's own attributes), element ("" = removed)
    };
    Kind kind = ItemFields;
    QString id;
    QString parent; // "" = the top level of the binder
    QString after;  // the id of the previous sibling; "" = first
    QString name;
    QString element;
    bool operator==(const Op &o) const
    {
        return kind == o.kind && id == o.id && parent == o.parent && after == o.after && name == o.name && element == o.element;
    }
};
using OpList = QList<Op>;

// "item.create" ... "project.section" (the names of the protocol, section 3.3)
QString opName(Op::Kind kind);

// the operations that turn the project file `from` into `to`; empty with *error set when one is no project file
OpList diffIndex(const QByteArray &from, const QByteArray &to, QString *error = nullptr);

// the project file in a fixed form without the derived attributes, for comparing two files (tests, the host's check that
// a result equals what was meant)
QByteArray canonicalIndex(const QByteArray &index, QString *error = nullptr);

// the attributes of an item element that the host computes
QStringList derivedItemAttributes();

} // namespace collab
} // namespace qb
