#include "core/collab/proto/Messages.h"

#include "core/collab/PathClass.h"

#include <QJsonValue>
#include <QRegularExpression>
#include <QSet>
#include <QUuid>
#include <cmath>

namespace qb {
namespace collab {
namespace proto {

namespace {

enum FieldKind { S, I, B, A, O };
struct Field {
    const char *name;
    FieldKind kind;
};
struct Rule {
    const char *type;
    QList<Field> fields;
    int maxArray; // longest list among the array fields
};

const QList<Rule> &rules()
{
    static const QList<Rule> r = {
        {T::HELLO, {{"protoMin", I}, {"protoMax", I}}, 64},
        {T::WELCOME, {{"proto", I}}, 64},
        {T::ERROR, {{"code", S}}, 0},
        {T::PROJECTS, {{"projects", A}}, 1000},
        {T::JOIN, {{"epoch", I}, {"seq", I}}, 0},
        {T::JOINED, {{"epoch", I}, {"head", I}, {"role", S}}, 0},
        {T::MANIFEST, {{"manifest", O}}, 0},
        {T::BLOB_GET, {{"hash", S}, {"offset", I}}, 0},
        {T::BLOB_PUT, {{"transfer", S}, {"hash", S}, {"size", I}}, 0},
        {T::BLOB_DONE, {{"transfer", S}}, 0},
        {T::COMMIT_REQUEST, {{"opId", S}, {"epoch", I}, {"baseSeq", I}, {"units", A}, {"locks", A}}, 1000},
        {T::COMMIT_RESULT, {{"opId", S}, {"status", S}}, 0},
        {T::COMMIT_EVENT, {{"epoch", I}, {"seq", I}, {"opId", S}, {"actor", S}, {"files", A}, {"ops", A}}, 100000},
        {T::RESYNC, {{"fromSeq", I}}, 0},
        {T::LOCK_REQUEST, {{"resources", A}}, 64},
        {T::LOCK_GRANTED, {{"resource", S}, {"generation", I}, {"epoch", I}, {"lease", S}}, 0},
        {T::LOCK_DENIED, {{"resource", S}}, 0},
        {T::LOCK_RELEASE, {{"resources", A}}, 64},
        {T::LOCK_REVOKED, {{"resource", S}}, 0},
        {T::LOCK_TABLE, {{"locks", A}}, 10000},
        {T::EDIT_REQUEST, {{"resource", S}}, 0},
        {T::EDIT_ASKED, {{"resource", S}}, 0},
        {T::EDIT_ANSWER, {{"resource", S}, {"handOver", B}}, 0},
        {T::EDIT_STATUS, {{"resource", S}}, 0},
        {T::EDIT_FORCE, {{"resource", S}}, 0},
        {T::EDIT_CANCEL, {{"resource", S}}, 0},
    };
    return r;
}

bool isInt(const QJsonValue &v)
{
    if (!v.isDouble())
        return false;
    const double d = v.toDouble();
    return std::isfinite(d) && d == std::floor(d) && std::fabs(d) < 9.0e15;
}

bool fits(const QJsonValue &v, FieldKind k)
{
    switch (k) {
    case S: return v.isString() && v.toString().size() <= 4096;
    case I: return isInt(v);
    case B: return v.isBool();
    case A: return v.isArray();
    case O: return v.isObject();
    }
    return false;
}

} // namespace

QStringList messageTypes()
{
    return {T::HELLO,        T::WELCOME,       T::HB,           T::BYE,           T::ERROR,        T::PAIR_HELLO,   T::PAIR_COMMIT,
            T::PAIR_NONCE,   T::PAIR_OPEN,     T::PAIR_CONFIRM, T::PAIR_RESULT,   T::PROJECT_LIST, T::PROJECTS,     T::JOIN,
            T::JOINED,       T::LEAVE,         T::MANIFEST_GET, T::MANIFEST,      T::BLOB_GET,     T::BLOB_PUT,     T::BLOB_DONE,
            T::COMMIT_REQUEST, T::COMMIT_RESULT, T::COMMIT_EVENT, T::RESYNC,      T::LOCK_REQUEST, T::LOCK_GRANTED, T::LOCK_DENIED,
            T::LOCK_RELEASE, T::LOCK_REVOKED,  T::LOCK_TABLE,   T::EDIT_REQUEST,  T::EDIT_ASKED,   T::EDIT_ANSWER,  T::EDIT_STATUS,
            T::EDIT_FORCE,   T::EDIT_CANCEL,   T::SNAPSHOT_OP,  T::SNAPSHOT_RESULT, T::BACKUP_REQUEST, T::BACKUP_STATUS, T::PRESENCE,
            T::MEMBERSHIP,   T::HANDOVER_PREPARE, T::HANDOVER_READY, T::HANDOVER_COMMIT, T::HANDOVER_DONE, T::HANDOVER_ABORT,
            T::ELECTION_REQUEST, T::VOTE,      T::AUTHORITY};
}

bool knownType(const QString &type)
{
    static const QSet<QString> set = [] {
        QSet<QString> s;
        for (const QString &t : messageTypes())
            s.insert(t);
        return s;
    }();
    return set.contains(type);
}

bool isSessionType(const QString &type)
{
    static const QSet<QString> pass = {QLatin1String(T::PAIR_HELLO), QLatin1String(T::PAIR_COMMIT), QLatin1String(T::PAIR_NONCE),
                                       QLatin1String(T::PAIR_OPEN),  QLatin1String(T::PAIR_CONFIRM), QLatin1String(T::PAIR_RESULT),
                                       QLatin1String(T::SNAPSHOT_OP), QLatin1String(T::SNAPSHOT_RESULT), QLatin1String(T::BACKUP_REQUEST),
                                       QLatin1String(T::BACKUP_STATUS), QLatin1String(T::PRESENCE), QLatin1String(T::MEMBERSHIP),
                                       QLatin1String(T::HANDOVER_PREPARE), QLatin1String(T::HANDOVER_READY), QLatin1String(T::HANDOVER_COMMIT),
                                       QLatin1String(T::HANDOVER_DONE), QLatin1String(T::HANDOVER_ABORT), QLatin1String(T::ELECTION_REQUEST),
                                       QLatin1String(T::VOTE), QLatin1String(T::AUTHORITY)};
    return knownType(type) && !pass.contains(type);
}

QStringList errorCodes()
{
    return {"BAD_FRAME",   "TOO_BIG",     "RATE_LIMIT",   "UNSUPPORTED_VERSION", "NOT_PAIRED",   "IDENTITY_MISMATCH", "UNKNOWN_DEVICE",
            "NO_ACCESS",   "READ_ONLY",   "UNKNOWN_PROJECT", "WRONG_EPOCH",      "NOT_HOST",     "LOCK_HELD",         "LOCK_STALE",
            "LOCK_NEEDED", "BASE_MISMATCH", "BAD_PATH",   "BAD_OP",              "BLOB_MISSING", "HASH_MISMATCH",     "NO_SPACE",
            "STORAGE_FAILED", "HOST_BUSY", "DUPLICATE",   "UNKNOWN_OLD",         "GAP",          "PAIRING_CLOSED",    "PAIRING_MISMATCH",
            "HOST_ALIVE",  "ALREADY_VOTED", "BEHIND"};
}

bool knownError(const QString &code) { return errorCodes().contains(code); }

QString newId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }
QString newTransferId() { return QUuid::createUuid().toString(QUuid::Id128); }
QByteArray transferBytes(const QString &hex32)
{
    static const QRegularExpression ok(QStringLiteral("^[0-9a-f]{32}$"));
    return ok.match(hex32).hasMatch() ? QByteArray::fromHex(hex32.toLatin1()) : QByteArray();
}

QJsonObject make(const QString &type, const QString &project, const QJsonObject &body)
{
    QJsonObject o = body;
    o.insert(QStringLiteral("t"), type);
    o.insert(QStringLiteral("v"), kProtocol);
    o.insert(QStringLiteral("id"), newId());
    if (!project.isEmpty())
        o.insert(QStringLiteral("p"), project);
    return o;
}

QJsonObject replyTo(const QJsonObject &request, const QString &type, const QJsonObject &body)
{
    QJsonObject o = make(type, request.value(QStringLiteral("p")).toString(), body);
    o.insert(QStringLiteral("re"), request.value(QStringLiteral("id")).toString());
    return o;
}

QJsonObject errorMessage(const QString &code, const QString &re, const QString &text, const QString &project)
{
    QJsonObject o = make(QLatin1String(T::ERROR), project);
    o.insert(QStringLiteral("code"), code);
    if (!re.isEmpty())
        o.insert(QStringLiteral("re"), re);
    if (!text.isEmpty())
        o.insert(QStringLiteral("msg"), text.left(500));
    return o;
}

bool validate(const QJsonObject &m, QString *code, QString *why)
{
    auto fail = [&](const char *c, const QString &w) {
        if (code)
            *code = QLatin1String(c);
        if (why)
            *why = w;
        return false;
    };
    const QJsonValue t = m.value(QStringLiteral("t"));
    if (!t.isString() || !knownType(t.toString()))
        return fail("BAD_OP", QStringLiteral("unknown type"));
    if (m.contains(QStringLiteral("v"))) {
        if (!isInt(m.value(QStringLiteral("v"))))
            return fail("BAD_OP", QStringLiteral("v is not a number"));
        if (m.value(QStringLiteral("v")).toInt() != kProtocol)
            return fail("UNSUPPORTED_VERSION", QStringLiteral("v is %1").arg(m.value(QStringLiteral("v")).toInt()));
    }
    for (const char *k : {"id", "re", "p"}) {
        const QJsonValue v = m.value(QLatin1String(k));
        if (m.contains(QLatin1String(k)) && (!v.isString() || v.toString().size() > 64))
            return fail("BAD_OP", QStringLiteral("%1 is not a short string").arg(QLatin1String(k)));
    }
    if (!m.value(QStringLiteral("id")).isString() || m.value(QStringLiteral("id")).toString().isEmpty())
        return fail("BAD_OP", QStringLiteral("no message id"));
    const QString type = t.toString();
    if (is(type, T::JOIN) && !m.value(QStringLiteral("p")).isString())
        return fail("BAD_OP", QStringLiteral("JOIN names no project"));
    for (const Rule &r : rules()) {
        if (type != QLatin1String(r.type))
            continue;
        for (const Field &f : r.fields) {
            const QJsonValue v = m.value(QLatin1String(f.name));
            if (!fits(v, f.kind))
                return fail("BAD_OP", QStringLiteral("%1 is missing or of the wrong kind").arg(QLatin1String(f.name)));
            if (f.kind == A && v.toArray().size() > r.maxArray)
                return fail("TOO_BIG", QStringLiteral("%1 is too long").arg(QLatin1String(f.name)));
        }
        break;
    }
    if (is(type, T::BLOB_PUT) || is(type, T::BLOB_DONE)) {
        if (transferBytes(m.value(QStringLiteral("transfer")).toString()).isEmpty())
            return fail("BAD_OP", QStringLiteral("transfer is no 16-byte id"));
    }
    if (is(type, T::BLOB_GET) || is(type, T::BLOB_PUT)) {
        static const QRegularExpression hex(QStringLiteral("^[0-9a-f]{64}$"));
        if (!hex.match(m.value(QStringLiteral("hash")).toString()).hasMatch())
            return fail("BAD_OP", QStringLiteral("hash is no SHA-256"));
        if (m.value(is(type, T::BLOB_GET) ? QStringLiteral("offset") : QStringLiteral("size")).toDouble() < 0)
            return fail("BAD_OP", QStringLiteral("negative size or offset"));
    }
    return true;
}

// ---------------------------------------------------------------- units

QJsonObject toJson(const Op &op)
{
    QJsonObject o{{QStringLiteral("k"), opName(op.kind)}};
    if (!op.id.isEmpty())
        o.insert(QStringLiteral("id"), op.id);
    if (!op.parent.isEmpty())
        o.insert(QStringLiteral("parent"), op.parent);
    if (!op.after.isEmpty())
        o.insert(QStringLiteral("after"), op.after);
    if (!op.name.isEmpty())
        o.insert(QStringLiteral("name"), op.name);
    if (!op.element.isEmpty())
        o.insert(QStringLiteral("element"), op.element);
    return o;
}

bool fromJson(const QJsonObject &o, Op *op)
{
    static const QList<Op::Kind> kinds = {Op::ItemCreate, Op::ItemMove, Op::ItemTrash, Op::ItemFields, Op::ItemDelete, Op::Section};
    const QString k = o.value(QStringLiteral("k")).toString();
    bool found = false;
    for (Op::Kind kind : kinds)
        if (opName(kind) == k) {
            op->kind = kind;
            found = true;
        }
    if (!found)
        return false;
    for (const char *f : {"id", "parent", "after", "name", "element"})
        if (o.contains(QLatin1String(f)) && !o.value(QLatin1String(f)).isString())
            return false;
    op->id = o.value(QStringLiteral("id")).toString();
    op->parent = o.value(QStringLiteral("parent")).toString();
    op->after = o.value(QStringLiteral("after")).toString();
    op->name = o.value(QStringLiteral("name")).toString();
    op->element = o.value(QStringLiteral("element")).toString();
    return true;
}

QJsonObject toJson(const FileOp &f)
{
    static const char *names[] = {"put", "remove", "removeTree"};
    QJsonObject o{{QStringLiteral("path"), f.path}, {QStringLiteral("kind"), QLatin1String(names[f.kind])}};
    if (f.kind == FileOp::Put) {
        o.insert(QStringLiteral("hash"), f.hash);
        o.insert(QStringLiteral("size"), double(f.size));
    }
    return o;
}

bool fromJson(const QJsonObject &o, FileOp *f)
{
    const QString k = o.value(QStringLiteral("kind")).toString();
    if (k == QLatin1String("put"))
        f->kind = FileOp::Put;
    else if (k == QLatin1String("remove"))
        f->kind = FileOp::Remove;
    else if (k == QLatin1String("removeTree"))
        f->kind = FileOp::RemoveTree;
    else
        return false;
    if (!o.value(QStringLiteral("path")).isString())
        return false;
    f->path = o.value(QStringLiteral("path")).toString();
    f->hash = o.value(QStringLiteral("hash")).toString();
    f->size = qint64(o.value(QStringLiteral("size")).toDouble());
    if (f->kind == FileOp::Put) {
        static const QRegularExpression hex(QStringLiteral("^[0-9a-f]{64}$"));
        if (!hex.match(f->hash).hasMatch() || f->size < 0)
            return false;
    }
    return true;
}

QJsonObject toJson(const Unit &u)
{
    QJsonObject bases;
    for (auto it = u.bases.constBegin(); it != u.bases.constEnd(); ++it)
        bases.insert(it.key(), double(it.value()));
    QJsonArray ops, files;
    for (const Op &op : u.ops)
        ops.append(toJson(op));
    for (const FileOp &f : u.files)
        files.append(toJson(f));
    return QJsonObject{{QStringLiteral("name"), u.name}, {QStringLiteral("bases"), bases}, {QStringLiteral("ops"), ops}, {QStringLiteral("files"), files}};
}

bool fromJson(const QJsonObject &o, Unit *u, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why)
            *why = w;
        return false;
    };
    if (!o.value(QStringLiteral("name")).isString() || !o.value(QStringLiteral("bases")).isObject() || !o.value(QStringLiteral("ops")).isArray()
        || !o.value(QStringLiteral("files")).isArray())
        return fail(QStringLiteral("a unit needs name, bases, ops and files"));
    *u = Unit();
    u->name = o.value(QStringLiteral("name")).toString();
    const QJsonObject b = o.value(QStringLiteral("bases")).toObject();
    for (auto it = b.constBegin(); it != b.constEnd(); ++it) {
        if (!isInt(it.value()) || it.value().toDouble() < 0)
            return fail(QStringLiteral("a base revision is no number"));
        u->bases.insert(it.key(), qint64(it.value().toDouble()));
    }
    for (const QJsonValue &v : o.value(QStringLiteral("ops")).toArray()) {
        Op op;
        if (!v.isObject() || !fromJson(v.toObject(), &op))
            return fail(QStringLiteral("an operation is not valid"));
        u->ops << op;
    }
    for (const QJsonValue &v : o.value(QStringLiteral("files")).toArray()) {
        FileOp f;
        if (!v.isObject() || !fromJson(v.toObject(), &f))
            return fail(QStringLiteral("a file entry is not valid"));
        u->files << f;
    }
    return true;
}

QStringList resourcesOfOp(const Op &op)
{
    switch (op.kind) {
    case Op::ItemCreate: return {QStringLiteral("structure"), QStringLiteral("doc:") + op.id};
    case Op::ItemMove: return {QStringLiteral("structure")};
    case Op::ItemTrash: return {QStringLiteral("structure"), QStringLiteral("doc:") + op.id};
    case Op::ItemDelete: return {QStringLiteral("structure"), QStringLiteral("doc:") + op.id};
    case Op::ItemFields: return {QStringLiteral("doc:") + op.id};
    case Op::Section: return {QStringLiteral("settings")};
    }
    return {};
}

QStringList resourcesOfPath(const QString &relativePath, bool *refused)
{
    const PathInfo info = classify(relativePath);
    if (refused)
        *refused = false;
    if (!info.capturable() || info.hostOnly() || info.kind == PathKind::Index) {
        if (refused)
            *refused = true;
        return {};
    }
    switch (info.kind) {
    case PathKind::Text:
    case PathKind::Notes:
    case PathKind::MediaFile:
    case PathKind::DocPicture: return {QStringLiteral("doc:") + info.id};
    case PathKind::MindMap: return {QStringLiteral("mindmap:") + info.id};
    case PathKind::Object:
    case PathKind::ObjectImage:
    case PathKind::ObjectSchema:
    case PathKind::Settings: return {info.resource};
    default: break;
    }
    if (refused)
        *refused = true;
    return {};
}

QJsonObject toJson(const Token &t)
{
    return QJsonObject{{QStringLiteral("resource"), t.resource},
                       {QStringLiteral("epoch"), double(t.epoch)},
                       {QStringLiteral("generation"), double(t.generation)},
                       {QStringLiteral("lease"), t.lease}};
}

Token tokenFromJson(const QJsonObject &o)
{
    Token t;
    t.resource = o.value(QStringLiteral("resource")).toString();
    t.epoch = qint64(o.value(QStringLiteral("epoch")).toDouble());
    t.generation = qint64(o.value(QStringLiteral("generation")).toDouble());
    t.lease = o.value(QStringLiteral("lease")).toString();
    return t;
}

} // namespace proto
} // namespace collab
} // namespace qb
