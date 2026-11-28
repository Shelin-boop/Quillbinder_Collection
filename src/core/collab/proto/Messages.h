#pragma once
// Live Sync's messages (0.99.0), section 3 of the design: the envelope, the types, the error codes, the validator and the
// units of a commit request. Every message is one JSON object {"t": type, "v": 1, "id": message id, "re": answered id?,
// "p": project uuid?, ...}. Unknown fields are ignored; an unknown type is BAD_OP. Nothing here touches a network or a
// disk: the sessions use these functions on the objects a Link hands over.
#include "core/collab/IndexDiff.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {
namespace proto {

constexpr int kProtocol = 1;

// the message types as they are written on the wire
namespace T {
// session
constexpr const char *HELLO = "HELLO", *WELCOME = "WELCOME", *HB = "HB", *BYE = "BYE", *ERROR = "ERROR";
// pairing (carried by net/Pairing, never by a session)
constexpr const char *PAIR_HELLO = "PAIR_HELLO", *PAIR_COMMIT = "PAIR_COMMIT", *PAIR_NONCE = "PAIR_NONCE", *PAIR_OPEN = "PAIR_OPEN",
                     *PAIR_CONFIRM = "PAIR_CONFIRM", *PAIR_RESULT = "PAIR_RESULT";
// projects
constexpr const char *PROJECT_LIST = "PROJECT_LIST", *PROJECTS = "PROJECTS", *JOIN = "JOIN", *JOINED = "JOINED", *LEAVE = "LEAVE";
// copies
constexpr const char *MANIFEST_GET = "MANIFEST_GET", *MANIFEST = "MANIFEST", *BLOB_GET = "BLOB_GET", *BLOB_PUT = "BLOB_PUT",
                     *BLOB_DONE = "BLOB_DONE";
// commits
constexpr const char *COMMIT_REQUEST = "COMMIT_REQUEST", *COMMIT_RESULT = "COMMIT_RESULT", *COMMIT_EVENT = "COMMIT_EVENT", *RESYNC = "RESYNC";
// locks
constexpr const char *LOCK_REQUEST = "LOCK_REQUEST", *LOCK_GRANTED = "LOCK_GRANTED", *LOCK_DENIED = "LOCK_DENIED",
                     *LOCK_RELEASE = "LOCK_RELEASE", *LOCK_REVOKED = "LOCK_REVOKED", *LOCK_TABLE = "LOCK_TABLE";
// edit requests (EDIT_CANCEL: the asker gives up; the design's list had no name for it)
constexpr const char *EDIT_REQUEST = "EDIT_REQUEST", *EDIT_ASKED = "EDIT_ASKED", *EDIT_ANSWER = "EDIT_ANSWER", *EDIT_STATUS = "EDIT_STATUS",
                     *EDIT_FORCE = "EDIT_FORCE", *EDIT_CANCEL = "EDIT_CANCEL";
// host jobs (package 8)
constexpr const char *SNAPSHOT_OP = "SNAPSHOT_OP", *SNAPSHOT_RESULT = "SNAPSHOT_RESULT", *BACKUP_REQUEST = "BACKUP_REQUEST",
                     *BACKUP_STATUS = "BACKUP_STATUS";
// hosting (package 6)
constexpr const char *PRESENCE = "PRESENCE", *MEMBERSHIP = "MEMBERSHIP", *HANDOVER_PREPARE = "HANDOVER_PREPARE",
                     *HANDOVER_READY = "HANDOVER_READY", *HANDOVER_COMMIT = "HANDOVER_COMMIT", *HANDOVER_DONE = "HANDOVER_DONE",
                     *HANDOVER_ABORT = "HANDOVER_ABORT", *ELECTION_REQUEST = "ELECTION_REQUEST", *VOTE = "VOTE", *AUTHORITY = "AUTHORITY";
} // namespace T

inline bool is(const QString &type, const char *name) { return type == QLatin1String(name); }

QStringList messageTypes();
bool knownType(const QString &type);
// the types a session of package 4 answers itself; the others are known but belong to pairing (WP1), hosting (WP6) or the host
// jobs (WP8) and go to the session's extension handler
bool isSessionType(const QString &type);
QStringList errorCodes();
bool knownError(const QString &code);

QString newId();         // a message or operation id (a UUID without braces)
QString newTransferId(); // 32 hex characters (16 bytes)
QByteArray transferBytes(const QString &hex32);

// {"t","v","id","p"} + the fields of body
QJsonObject make(const QString &type, const QString &project = QString(), const QJsonObject &body = QJsonObject());
// the same, answering `request` ("re" = its id, "p" = its project)
QJsonObject replyTo(const QJsonObject &request, const QString &type, const QJsonObject &body = QJsonObject());
QJsonObject errorMessage(const QString &code, const QString &re = QString(), const QString &text = QString(), const QString &project = QString());

// Checks the envelope and the fields the type needs (their JSON types and sizes). False with the error code to answer
// (BAD_OP for an unknown type or a wrong field, TOO_BIG for a list that is too long) and a reason for the log.
bool validate(const QJsonObject &message, QString *code = nullptr, QString *why = nullptr);

// ---------------------------------------------------------------- units (section 3.3)

struct FileOp {
    enum Kind { Put, Remove, RemoveTree };
    Kind kind = Put;
    QString path;   // relative to the project folder
    QString hash;   // Put: SHA-256 (hex) of the plain bytes, sent beforehand as a blob
    qint64 size = 0;
};

// What one follower save changes in one resource group: its operations on the project file, its files, and the revision of
// every resource it touches as the follower knew it (0 for a resource that is not there yet).
struct Unit {
    QString name;
    QHash<QString, qint64> bases;
    OpList ops;
    QList<FileOp> files;
};

QJsonObject toJson(const Op &op);
bool fromJson(const QJsonObject &o, Op *op);
QJsonObject toJson(const FileOp &f);
bool fromJson(const QJsonObject &o, FileOp *f);
QJsonObject toJson(const Unit &u);
bool fromJson(const QJsonObject &o, Unit *u, QString *why = nullptr);

// The lock resources that an operation or a path needs, as far as they follow from the operation alone: "doc:<id>",
// "structure", "settings", "mindmap:<id>", "object:<oid>", "object:schema", "object:images". A path that is no replicated,
// capturable class gives an empty list and *refused set.
QStringList resourcesOfOp(const Op &op);
QStringList resourcesOfPath(const QString &relativePath, bool *refused = nullptr);

// a lock token as it travels in a request: {resource, epoch, generation, lease}
struct Token {
    QString resource;
    qint64 epoch = 0;
    qint64 generation = 0;
    QString lease;
    bool valid() const { return !resource.isEmpty() && generation > 0; }
    bool operator==(const Token &o) const
    {
        return resource == o.resource && epoch == o.epoch && generation == o.generation && lease == o.lease;
    }
};
QJsonObject toJson(const Token &t);
Token tokenFromJson(const QJsonObject &o);

} // namespace proto
} // namespace collab
} // namespace qb
