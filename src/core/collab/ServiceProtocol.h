#pragma once
// Live Sync, the words between a program's windows and its service (0.99.0). Frames of net/Frames.h over a LocalLink (or a
// MemLink where the service runs in the window's own process); every message is a JSON object whose "t" is one of these. The
// messages of the service to the windows carry the project's uuid in "uuid". This is no network protocol: both ends belong to
// one user on one device, and they are always the same program version (a service of another version is told to quit).
//
//   window -> service
//     W_ATTACH     {id, folder, keepHosting}           the window opens this folder; answered by W_ATTACHED
//     W_DETACH     {uuid}                              the window closed the project
//     W_SUBMIT     {id, uuid, units[], blobs[{hash,size}]}  a change request; the bytes of the blobs follow as data frames whose
//                                                      transfer id is the first 16 bytes of the SHA-256; answered by W_SUBMITTED
//     W_LOCK, W_UNLOCK {uuid, resources[]}             explicit locks (the editor's first key)
//     W_ASK, W_CANCEL_ASK, W_TAKEOVER {uuid, resource} edit requests
//     W_ANSWER     {uuid, resource, handOver, asker}   the holder's answer to a request
//     W_ACTION     {uuid, action}                      "requestBackup" or "stopSharing"
//     W_PING       {id}                                answered by W_PONG
//     W_JOB        {id, uuid, op, doc, file, title, allowEmpty}  a snapshot job (take, rename, delete, rollBack) for the host of the
//                                                      project, which may be this device; answered by W_JOB_RESULT
//     W_STOP       {}                                  asks a service process to end (a program that wants to host itself); ignored
//                                                      by a service that lives inside a program
//   service -> window
//     W_ATTACHED   {re, ok, code, uuid, host, state...}
//     W_SUBMITTED  {re, ok, opId, error}
//     W_JOB_RESULT {re, ok, code, text, file}
//     W_STATE      {uuid, ...}                         the state of the project as the window needs it (see Service::stateJson)
//     W_EVENT      {uuid, seq, opId, actor, own, ops[], paths[], revs{}}   a commit is in the folder
//     W_REPLACED   {uuid}                              the folder was made again from the host's files
//     W_RESULT     {uuid, opId, kind, seq, code, rejects[]}   kind: committed, rejected, draft, problem
//     W_LOCK_EVT   {uuid, kind, resource, holder, reason}     kind: granted, denied, revoked
//     W_EDIT_EVT   {uuid, kind, resource, asker, waitMs, declined, mayForce, holder}   kind: asked, status
//     W_QUIT       {}                                  the service ends (another version, or "Stop Hosting and Quit")
#include <QByteArray>
#include <QJsonObject>
#include <QString>

namespace qb {
namespace collab {
namespace local {

constexpr const char *ATTACH = "W_ATTACH", *DETACH = "W_DETACH", *SUBMIT = "W_SUBMIT", *LOCK = "W_LOCK", *UNLOCK = "W_UNLOCK",
                     *ASK = "W_ASK", *CANCEL_ASK = "W_CANCEL_ASK", *TAKEOVER = "W_TAKEOVER", *ANSWER = "W_ANSWER",
                     *ACTION = "W_ACTION", *PING = "W_PING", *JOB = "W_JOB", *STOP = "W_STOP";
constexpr const char *ATTACHED = "W_ATTACHED", *SUBMITTED = "W_SUBMITTED", *STATE = "W_STATE", *EVENT = "W_EVENT",
                     *REPLACED = "W_REPLACED", *RESULT = "W_RESULT", *LOCK_EVT = "W_LOCK_EVT", *EDIT_EVT = "W_EDIT_EVT",
                     *PONG = "W_PONG", *QUIT = "W_QUIT", *JOB_RESULT = "W_JOB_RESULT";

inline QJsonObject msg(const char *type, const QJsonObject &body = QJsonObject())
{
    QJsonObject o = body;
    o.insert(QStringLiteral("t"), QLatin1String(type));
    return o;
}
inline QString typeOf(const QJsonObject &o) { return o.value(QStringLiteral("t")).toString(); }

// the data frames of a blob: the transfer id of a hash (the first 16 of its 32 bytes)
inline QByteArray transferOfHash(const QString &hashHex) { return QByteArray::fromHex(hashHex.left(32).toLatin1()); }

} // namespace local
} // namespace collab
} // namespace qb
