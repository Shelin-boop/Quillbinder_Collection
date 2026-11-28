#pragma once
// Live Sync, the host's classification of project paths (0.99.0): which resource a path of a shared project belongs to,
// and whether it is replicated at all. A path outside every class is never written for a follower and never accepted
// from one. Paths are relative to the project folder and use "/".
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {

enum class PathKind {
    Index,        // <name>.qbx: item and section operations only, never the file itself
    Text,         // content/<id>/text.*                      DOCUMENT(id)
    Notes,        // content/<id>/notes.*                     DOCUMENT(id)
    MediaFile,    // content/<id>/file.<safe extension>       DOCUMENT(id)
    DocPicture,   // any other file below content/<id>/      DOCUMENT(id)
    MindMap,      // content/<id>/mindmap.scap                OBJECT(mindmap:<id>)
    Snapshot,     // snapshots/<id>/** or content/<id>/snapshots/**: host jobs only
    Object,       // objects/<kind>/<oid>.json                OBJECT(<oid>)
    ObjectImage,  // objects/images/**
    ObjectSchema, // objects/schema.json                      OBJECT(schema)
    Settings,     // the shared settings files below settings/ (look.json ...)  SETTINGS
    Marker,       // encryption.json: replicated, never written by a follower
    Derived,      // contents-by-title.txt: computed by the host, a client's version is ignored
    DeviceLocal,  // settings/ui-*.json, the lock and its work marks, the journal, conflicts, spare copies, *.qbstage
    Internal,     // livesync/, livesync-local/, livesync-replica.json: Live Sync's own state, never replicated
    Unknown       // anything else: refused
};

struct PathInfo {
    PathKind kind = PathKind::Unknown;
    QString resource; // "doc:<id>", "mindmap:<id>", "object:<oid>", "object:schema", "object:images", "settings", "snapshots:<id>", "index", "project"; "" if none
    QString id;       // the document's id for Text, Notes, MediaFile, DocPicture, MindMap, Snapshot
    // a file that goes to the other devices (everything except DeviceLocal, Internal and Unknown)
    bool replicated() const { return kind != PathKind::DeviceLocal && kind != PathKind::Internal && kind != PathKind::Unknown; }
    // a file a follower's write is captured for (replicated and not derived)
    bool capturable() const { return replicated() && kind != PathKind::Derived; }
    // only the host's own jobs write it (snapshots, the marker): a client's change set may not hold it
    bool hostOnly() const { return kind == PathKind::Snapshot || kind == PathKind::Marker; }
};

// the class of `relativePath` (relative to the project folder)
PathInfo classify(const QString &relativePath);

// Section 3.5's rules, checked before anything else: false with the reason (an error code of the protocol in *code:
// "BAD_PATH") for a path that is not plain, relative and safe: pathsafety::isSafeRelative, at most 240 bytes, no
// control character, no Windows reserved name in any part (CON, PRN, AUX, NUL, COM1-9, LPT1-9, with or without an
// extension), no part ending in a space or a dot
bool isAcceptablePath(const QString &relativePath, QString *why = nullptr);
// the paths of the list that equal another one of it except for the case (all but the first of each group, in order)
QStringList caseCollisions(const QStringList &relativePaths);
// whether a symbolic link lies on the way from `folder` to the path (the receiver checks it before it writes)
bool symlinkOnTheWay(const QString &folder, const QString &relativePath);

} // namespace collab
} // namespace qb
