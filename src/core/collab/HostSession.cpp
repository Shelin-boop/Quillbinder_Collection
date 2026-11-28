#include "core/collab/HostSession.h"

#include "core/PathSafety.h"
#include "core/SafeSave.h"
#include "core/SaveBatch.h"
#include "core/collab/OpApply.h"
#include "core/collab/PathClass.h"
#include "core/crypto/Vault.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QPointer>
#include <QRegularExpression>
#include <QStorageInfo>
#include <algorithm>

namespace qb {
namespace collab {

using namespace proto;

QString roleName(Role r) { return r == Role::Editor ? QStringLiteral("editor") : r == Role::Reader ? QStringLiteral("reader") : QStringLiteral("none"); }

struct HostSession::Peer {
    QPointer<net::Link> link; // (a link deleted under the session reads as gone)
    Owner owner;
    QString name;
    Role role = Role::None;
    bool hello = false;
    bool joined = false;
    QSet<QString> seen;
    QList<QString> seenOrder;
    QPointer<BlobHub> hub;
    QList<QMetaObject::Connection> connections;
};

namespace {
QJsonObject toObject(const QHash<QString, qint64> &h)
{
    QJsonObject o;
    for (auto it = h.constBegin(); it != h.constEnd(); ++it)
        o.insert(it.key(), double(it.value()));
    return o;
}
QHash<QString, qint64> toHash(const QJsonObject &o)
{
    QHash<QString, qint64> h;
    for (auto it = o.constBegin(); it != o.constEnd(); ++it)
        h.insert(it.key(), qint64(it.value().toDouble()));
    return h;
}
QJsonObject object(std::initializer_list<std::pair<QString, QJsonValue>> l)
{
    QJsonObject o;
    for (const auto &p : l)
        o.insert(p.first, p.second);
    return o;
}
} // namespace

HostSession::HostSession(const Options &o, QObject *parent)
    : QObject(parent), m_folder(QDir::cleanPath(QDir(o.folder).absolutePath())), m_uuid(o.uuid), m_hostName(o.hostName), m_clock(o.clock),
      m_epoch(o.epoch), m_locks(o.clock, o.epoch), m_requests(o.clock), m_incoming(QDir::cleanPath(QDir(o.folder).absolutePath()) + QStringLiteral("/livesync/incoming"))
{
    if (m_clock)
        connect(m_clock, &Clock::ticked, this, &HostSession::tick);
    m_free = [this] { return QStorageInfo(m_folder).bytesAvailable(); };
}

HostSession::~HostSession()
{
    for (Peer *p : m_peers) {
        for (const QMetaObject::Connection &c : p->connections)
            disconnect(c);
        delete p;
    }
}

QString HostSession::logPath(qint64 seq) const { return m_folder + QStringLiteral("/livesync/log/%1.json").arg(seq, 12, 10, QLatin1Char('0')); }
qint64 HostSession::freeBytes() const { return m_free ? m_free() : 0; }

QByteArray HostSession::stateJson(qint64 seq, const QHash<QString, qint64> &revs, qint64 genReserve) const
{
    return QJsonDocument(object({{QStringLiteral("v"), 1},
                                 {QStringLiteral("p"), m_uuid},
                                 {QStringLiteral("epoch"), double(m_epoch)},
                                 {QStringLiteral("seq"), double(seq)},
                                 {QStringLiteral("genReserve"), double(genReserve)},
                                 {QStringLiteral("revs"), toObject(revs)}}))
        .toJson(QJsonDocument::Compact);
}

bool HostSession::persistReserve(qint64 reserve)
{
    QString err;
    if (!safesave::writeFile(m_folder + QStringLiteral("/livesync/state.json"), stateJson(m_seq, m_revs, reserve), &err))
        return false;
    m_genReserve = reserve;
    return true;
}

bool HostSession::start(QString *error)
{
    auto fail = [&](const QString &w) {
        if (error)
            *error = w;
        return false;
    };
    if (!m_clock)
        return fail(QStringLiteral("no clock"));
    if (!QDir(m_folder).exists())
        return fail(QStringLiteral("the project folder is not there"));
    m_indexPath.clear();
    for (const QString &n : QDir(m_folder).entryList({QStringLiteral("*.qbx")}, QDir::Files, QDir::Name)) {
        m_indexPath = m_folder + QLatin1Char('/') + n;
        break;
    }
    if (m_indexPath.isEmpty())
        return fail(QStringLiteral("the folder holds no project file"));
    // an interrupted save: the state before or after it, as at every opening
    const savebatch::Recovery rec = savebatch::recover(m_folder, m_folder + QStringLiteral("/settings/journal.json"), m_folder + QStringLiteral("/settings"));
    if (!rec.unreadable.isEmpty() || rec.waiting())
        return fail(QStringLiteral("an earlier save could not be finished: %1").arg(rec.text()));
    QDir().mkpath(m_folder + QStringLiteral("/livesync/log"));
    const QString statePath = m_folder + QStringLiteral("/livesync/state.json");
    if (QFileInfo::exists(statePath)) {
        QByteArray raw;
        if (!vault::readFile(statePath, &raw))
            return fail(QStringLiteral("the state of Live Sync cannot be read"));
        const QJsonObject o = QJsonDocument::fromJson(raw).object();
        if (o.value(QStringLiteral("v")).toInt() != 1 || o.value(QStringLiteral("p")).toString() != m_uuid)
            return fail(QStringLiteral("the state of Live Sync belongs to another project"));
        m_epoch = qint64(o.value(QStringLiteral("epoch")).toDouble(1));
        m_seq = qint64(o.value(QStringLiteral("seq")).toDouble());
        m_genReserve = qint64(o.value(QStringLiteral("genReserve")).toDouble());
        m_revs = toHash(o.value(QStringLiteral("revs")).toObject());
    } else if (!safesave::writeFile(statePath, stateJson(0, {}, 0), error)) {
        return false;
    }
    m_locks.setEpoch(m_epoch);
    // a new block of generations at once: nothing of the old one is used again
    const qint64 reserve = m_genReserve;
    m_locks.setGenerationReserve(reserve, [this](qint64 r) { return persistReserve(r); });
    rebuildFromLog();
    // the removals of the last commit are done again (they follow the batch and may have been cut off)
    if (m_seq > 0) {
        const QJsonObject last = logEntry(m_seq);
        for (const QJsonValue &v : last.value(QStringLiteral("files")).toArray()) {
            const QJsonObject f = v.toObject();
            const QString path = f.value(QStringLiteral("path")).toString();
            const QString kind = f.value(QStringLiteral("kind")).toString();
            if (!isAcceptablePath(path) || !classify(path).capturable())
                continue;
            if (kind == QLatin1String("remove"))
                safesave::removeFile(m_folder + QLatin1Char('/') + path);
            else if (kind == QLatin1String("removeTree") && QFileInfo(m_folder + QLatin1Char('/') + path).isDir())
                pathsafety::removeTreeInside(m_folder + QLatin1Char('/') + path, m_folder);
        }
    }
    QString err;
    Manifest mf;
    if (!Manifest::scan(m_folder, m_folder + QStringLiteral("/livesync/hashes.json"), m_revs, &mf, &err))
        return fail(err);
    m_manifest = mf;
    m_started = true;
    return true;
}

void HostSession::stop()
{
    const QList<Peer *> peers = m_peers;
    for (Peer *p : peers) {
        send(p, make(QLatin1String(T::BYE)));
        if (p->link)
            p->link->close();
    }
    m_started = false;
}

void HostSession::setEpoch(qint64 e)
{
    m_epoch = e;
    m_locks.setEpoch(e);
}

Manifest HostSession::manifest() const
{
    Manifest m = m_manifest;
    m.epoch = m_epoch;
    m.seq = m_seq;
    m.revs = m_revs;
    for (ManifestEntry &e : m.entries) {
        const PathInfo i = classify(e.path);
        e.rev = i.resource.isEmpty() ? 0 : m_revs.value(i.resource);
    }
    return m;
}

QJsonObject HostSession::logEntry(qint64 seq) const
{
    QByteArray raw;
    if (!vault::readFile(logPath(seq), &raw))
        return QJsonObject();
    return QJsonDocument::fromJson(raw).object();
}

void HostSession::rebuildFromLog()
{
    m_ops.clear();
    m_opOrder.clear();
    m_logSeqs.clear();
    QList<qint64> seqs;
    for (const QString &n : QDir(m_folder + QStringLiteral("/livesync/log")).entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name)) {
        bool ok = false;
        const qint64 s = n.left(n.size() - 5).toLongLong(&ok);
        if (ok)
            seqs << s;
    }
    std::sort(seqs.begin(), seqs.end());
    m_horizon = seqs.isEmpty() ? m_seq : seqs.first() - 1; // what was pruned (or never kept) is older than this
    if (seqs.isEmpty() || seqs.first() <= 1)
        m_horizon = 0;
    for (qint64 s : seqs) {
        const QJsonObject e = logEntry(s);
        const QString opId = e.value(QStringLiteral("opId")).toString();
        const qint64 at = qint64(e.value(QStringLiteral("at")).toDouble());
        m_logSeqs << std::make_pair(s, at);
        if (opId.isEmpty())
            continue;
        StoredResult r;
        r.seq = s;
        r.revs = e.value(QStringLiteral("revs")).toObject();
        r.wallMs = at;
        m_ops.insert(opId, r);
        m_opOrder << std::make_pair(s, opId);
    }
}

void HostSession::pruneLog()
{
    const qint64 now = m_clock->wallMs();
    const qint64 maxAge = kKeepDays * 24 * 3600 * 1000;
    while (m_logSeqs.size() > kKeepCommits && now - m_logSeqs.first().second > maxAge) {
        const qint64 s = m_logSeqs.takeFirst().first;
        safesave::removeFile(logPath(s));
        while (!m_opOrder.isEmpty() && m_opOrder.first().first <= s)
            m_ops.remove(m_opOrder.takeFirst().second);
        m_horizon = qMax(m_horizon, s);
    }
    // blobs that came in and were not used any more
    const qint64 cutoff = QDateTime::currentMSecsSinceEpoch() - 3600 * 1000;
    for (const QString &h : m_incoming.hashes())
        if (QFileInfo(m_incoming.path(h)).lastModified().toMSecsSinceEpoch() < cutoff)
            m_incoming.remove(h);
}

// ---------------------------------------------------------------- connections

int HostSession::joinedCount() const
{
    int n = 0;
    for (const Peer *p : m_peers)
        n += p->joined;
    return n;
}

QList<HostSession::PeerSummary> HostSession::joinedPeers() const
{
    QList<PeerSummary> out;
    for (const Peer *p : m_peers)
        if (p->joined)
            out.append(PeerSummary{p->owner.device, p->owner.session, p->name, p->role});
    return out;
}

Role HostSession::roleOf(const QString &device) const
{
    if (m_roleProvider)
        return m_roleProvider(device);
    return m_roles.value(device, Role::None);
}

HostSession::Peer *HostSession::peerOf(net::Link *link) const
{
    for (Peer *p : m_peers)
        if (p->link == link)
            return p;
    return nullptr;
}

QList<HostSession::Peer *> HostSession::peersOfOwner(const Owner &o) const
{
    QList<Peer *> out;
    for (Peer *p : m_peers)
        if (p->owner == o)
            out << p;
    return out;
}

HostSession::Peer *HostSession::peerByOwner(const Owner &o) const
{
    const QList<Peer *> l = peersOfOwner(o);
    return l.isEmpty() ? nullptr : l.first();
}

void HostSession::addLink(net::Link *link)
{
    Peer *p = new Peer;
    p->link = link;
    p->owner.device = link->peer().deviceId;
    p->owner.session = QStringLiteral("main");
    p->hub = new BlobHub(link, &m_incoming, link);
    p->hub->setFreeSpaceProbe([this] { return freeBytes(); });
    connect(p->hub, &BlobHub::failed, this, [this, p](const QString &, const QString &code) { emit refused(p->owner.device, code); });
    m_peers << p;
    p->connections << connect(link, &net::Link::frameReceived, this, [this, p](const net::Frame &f) { onFrame(p, f); });
    p->connections << connect(link, &net::Link::closed, this, [this, p](const QString &) { onClosed(p); });
}

void HostSession::onClosed(Peer *p)
{
    if (!m_peers.contains(p))
        return;
    m_peers.removeOne(p);
    for (const QMetaObject::Connection &c : p->connections)
        disconnect(c);
    // the leases stay for their time (a window that crashed or lost the network keeps them for a while); the open requests of
    // this connection do not
    m_requests.dropOf(p->owner);
    const QString device = p->owner.device;
    const bool joined = p->joined;
    if (p->hub)
        p->hub->deleteLater();
    delete p;
    if (joined)
        emit peerLeft(device);
}

void HostSession::send(Peer *p, const QJsonObject &m)
{
    if (p && p->link && p->link->state() == net::Link::State::Ready)
        p->link->sendJson(m);
}

void HostSession::sendError(Peer *p, const QString &code, const QString &re, const QString &text)
{
    emit refused(p->owner.device, code);
    send(p, errorMessage(code, re, text, m_uuid));
}

void HostSession::broadcast(const QJsonObject &m, Peer *except)
{
    for (Peer *p : m_peers)
        if (p->joined && p != except)
            send(p, m);
}

void HostSession::onFrame(Peer *p, const net::Frame &f)
{
    if (f.kind == net::Frame::Data) {
        p->hub->handleData(f);
        return;
    }
    bool ok = false;
    const QJsonObject m = f.object(&ok);
    if (!ok) {
        p->link->close(QStringLiteral("BAD_FRAME"));
        return;
    }
    QString code, why;
    if (!validate(m, &code, &why)) {
        sendError(p, code, m.value(QStringLiteral("id")).toString(), why);
        if (code == QLatin1String("UNSUPPORTED_VERSION"))
            p->link->close(code);
        return;
    }
    // a message that was sent twice (the same id) is not done twice
    const QString id = m.value(QStringLiteral("id")).toString();
    if (p->seen.contains(id)) {
        sendError(p, QStringLiteral("DUPLICATE"), id);
        return;
    }
    p->seen.insert(id);
    p->seenOrder << id;
    if (p->seenOrder.size() > 512)
        p->seen.remove(p->seenOrder.takeFirst());
    onMessage(p, m);
}

void HostSession::onMessage(Peer *p, const QJsonObject &m)
{
    const QString t = m.value(QStringLiteral("t")).toString();
    const QString re = m.value(QStringLiteral("id")).toString();
    if (is(t, T::HELLO)) {
        onHello(p, m);
        return;
    }
    if (is(t, T::BYE)) {
        p->link->close();
        return;
    }
    if (!p->hello) {
        sendError(p, QStringLiteral("BAD_OP"), re, QStringLiteral("HELLO first"));
        return;
    }
    if (is(t, T::BLOB_PUT) || is(t, T::BLOB_DONE)) {
        if (!p->joined || p->role != Role::Editor) {
            sendError(p, p->joined ? QStringLiteral("READ_ONLY") : QStringLiteral("BAD_OP"), re);
            return;
        }
        p->hub->handleMessage(m);
        return;
    }
    if (is(t, T::HB)) {
        m_locks.renewDevice(p->owner.device);
        return;
    }
    if (is(t, T::PROJECT_LIST)) {
        QJsonArray list;
        if (p->role != Role::None)
            list.append(object({{QStringLiteral("p"), m_uuid}, {QStringLiteral("name"), m_hostName}, {QStringLiteral("role"), roleName(p->role)}}));
        send(p, replyTo(m, QLatin1String(T::PROJECTS), object({{QStringLiteral("projects"), list}})));
        return;
    }
    if (is(t, T::JOIN)) {
        onJoin(p, m);
        return;
    }
    if (is(t, T::LEAVE)) {
        p->joined = false;
        m_requests.dropOf(p->owner);
        return;
    }
    if (!isSessionType(t)) {
        if (m_extension && m_extension(p->owner.device, p->link, m))
            return;
        sendError(p, QStringLiteral("BAD_OP"), re, QStringLiteral("not handled by this host"));
        return;
    }
    if (is(t, T::WELCOME) || is(t, T::JOINED) || is(t, T::PROJECTS) || is(t, T::MANIFEST) || is(t, T::COMMIT_RESULT) || is(t, T::COMMIT_EVENT)
        || is(t, T::LOCK_GRANTED) || is(t, T::LOCK_DENIED) || is(t, T::LOCK_REVOKED) || is(t, T::LOCK_TABLE) || is(t, T::EDIT_ASKED)
        || is(t, T::EDIT_STATUS) || is(t, T::ERROR)) {
        // what only a host sends
        if (is(t, T::ERROR)) {
            p->hub->requestFailed(m.value(QStringLiteral("re")).toString(), m.value(QStringLiteral("code")).toString());
            return;
        }
        sendError(p, QStringLiteral("NOT_HOST"), re);
        return;
    }
    if (!p->joined) {
        sendError(p, QStringLiteral("BAD_OP"), re, QStringLiteral("JOIN first"));
        return;
    }
    if (m.contains(QStringLiteral("p")) && m.value(QStringLiteral("p")).toString() != m_uuid) {
        sendError(p, QStringLiteral("UNKNOWN_PROJECT"), re);
        return;
    }
    if (is(t, T::MANIFEST_GET))
        onManifestGet(p, m);
    else if (is(t, T::BLOB_GET))
        onBlobGet(p, m);
    else if (is(t, T::RESYNC))
        onResync(p, m);
    else if (is(t, T::COMMIT_REQUEST))
        onCommit(p, m);
    else if (is(t, T::LOCK_REQUEST))
        onLockRequest(p, m);
    else if (is(t, T::LOCK_RELEASE))
        onLockRelease(p, m);
    else if (is(t, T::EDIT_REQUEST))
        onEditRequest(p, m);
    else if (is(t, T::EDIT_ANSWER))
        onEditAnswer(p, m);
    else if (is(t, T::EDIT_FORCE))
        onEditForce(p, m);
    else if (is(t, T::EDIT_CANCEL))
        onEditCancel(p, m);
    else
        sendError(p, QStringLiteral("BAD_OP"), re);
}

void HostSession::onHello(Peer *p, const QJsonObject &m)
{
    const qint64 lo = qint64(m.value(QStringLiteral("protoMin")).toDouble()), hi = qint64(m.value(QStringLiteral("protoMax")).toDouble());
    if (lo > kProtocol || hi < kProtocol) {
        sendError(p, QStringLiteral("UNSUPPORTED_VERSION"), m.value(QStringLiteral("id")).toString());
        p->link->close(QStringLiteral("UNSUPPORTED_VERSION"));
        return;
    }
    p->hello = true;
    p->name = m.value(QStringLiteral("name")).toString().left(100);
    const QString session = m.value(QStringLiteral("session")).toString();
    if (!session.isEmpty() && session.size() <= 64)
        p->owner.session = session;
    p->role = roleOf(p->owner.device);
    send(p, replyTo(m, QLatin1String(T::WELCOME),
                    object({{QStringLiteral("proto"), kProtocol},
                            {QStringLiteral("name"), m_hostName},
                            {QStringLiteral("caps"), QJsonArray{QStringLiteral("blob-resume")}}})));
}

void HostSession::onJoin(Peer *p, const QJsonObject &m)
{
    const QString re = m.value(QStringLiteral("id")).toString();
    if (m.value(QStringLiteral("p")).toString() != m_uuid) {
        sendError(p, QStringLiteral("UNKNOWN_PROJECT"), re);
        return;
    }
    p->role = roleOf(p->owner.device); // read again: the writer may have changed it
    if (p->role == Role::None) {
        sendError(p, QStringLiteral("NO_ACCESS"), re);
        return;
    }
    p->joined = true;
    // a new window of this device takes over the leases of one that is gone
    m_locks.moveSessions(p->owner.device, [this, p](const Owner &o) { return o != p->owner && peersOfOwner(o).isEmpty(); }, p->owner);
    send(p, replyTo(m, QLatin1String(T::JOINED),
                    object({{QStringLiteral("epoch"), double(m_epoch)},
                            {QStringLiteral("head"), double(m_seq)},
                            {QStringLiteral("role"), roleName(p->role)},
                            {QStringLiteral("membership"), m_membership},
                            {QStringLiteral("proofs"), m_proofs},
                            {QStringLiteral("locks"), m_locks.toJson()}})));
    emit peerJoined(p->owner.device, p->name);
}

void HostSession::onManifestGet(Peer *p, const QJsonObject &m)
{
    const Manifest mf = manifest();
    const int perPage = 2000;
    const int pages = qMax(1, (mf.entries.size() + perPage - 1) / perPage);
    for (int i = 0; i < pages; ++i) {
        Manifest part;
        part.epoch = mf.epoch;
        part.seq = mf.seq;
        if (i == 0)
            part.revs = mf.revs;
        part.entries = mf.entries.mid(i * perPage, perPage);
        send(p, replyTo(m, QLatin1String(T::MANIFEST),
                        object({{QStringLiteral("manifest"), part.toJson()}, {QStringLiteral("page"), i}, {QStringLiteral("pages"), pages}})));
    }
}

void HostSession::onBlobGet(Peer *p, const QJsonObject &m)
{
    const QString hash = m.value(QStringLiteral("hash")).toString();
    const qint64 offset = qint64(m.value(QStringLiteral("offset")).toDouble());
    const QString re = m.value(QStringLiteral("id")).toString();
    // a stored project file with this hash
    for (const ManifestEntry &e : m_manifest.entries)
        if (e.sha256 == hash) {
            const QString path = m_folder + QLatin1Char('/') + e.path;
            if (p->hub->send(hash, re, path, offset))
                return;
        }
    sendError(p, QStringLiteral("BLOB_MISSING"), re);
}

void HostSession::sendEvent(Peer *p, const QJsonObject &event)
{
    if (!p->joined)
        return;
    if (p->link->bytesWaiting() > qint64(32) * 1024 * 1024) { // a connection that cannot keep up is closed, it resyncs when it returns
        p->link->close(QStringLiteral("HOST_BUSY"));
        return;
    }
    send(p, event);
}

void HostSession::onResync(Peer *p, const QJsonObject &m)
{
    const qint64 from = qint64(m.value(QStringLiteral("fromSeq")).toDouble());
    const QString re = m.value(QStringLiteral("id")).toString();
    if (from < 1 || from > m_seq + 1 || m_seq - from >= kKeepCommits) {
        sendError(p, QStringLiteral("GAP"), re);
        return;
    }
    QList<QJsonObject> events;
    for (qint64 s = from; s <= m_seq; ++s) {
        const QJsonObject e = logEntry(s);
        if (e.isEmpty()) {
            sendError(p, QStringLiteral("GAP"), re);
            return;
        }
        // the files are described as they are now: the replica gets the newest bytes of each path, which is where the later
        // events lead it as well
        QJsonArray files;
        for (const QJsonValue &v : e.value(QStringLiteral("files")).toArray()) {
            const QJsonObject f = v.toObject();
            const QString path = f.value(QStringLiteral("path")).toString();
            if (f.value(QStringLiteral("kind")).toString() == QLatin1String("put")) {
                const ManifestEntry *me = m_manifest.find(path);
                if (!me)
                    continue;
                files.append(object({{QStringLiteral("path"), path}, {QStringLiteral("kind"), QStringLiteral("put")}, {QStringLiteral("hash"), me->sha256},
                                     {QStringLiteral("size"), double(me->size)}}));
            } else
                files.append(f);
        }
        QJsonObject ev = make(QLatin1String(T::COMMIT_EVENT), m_uuid);
        ev.insert(QStringLiteral("epoch"), double(m_epoch));
        ev.insert(QStringLiteral("seq"), double(s));
        ev.insert(QStringLiteral("opId"), e.value(QStringLiteral("opId")));
        ev.insert(QStringLiteral("actor"), e.value(QStringLiteral("actor")));
        ev.insert(QStringLiteral("ops"), e.value(QStringLiteral("ops")));
        ev.insert(QStringLiteral("files"), files);
        ev.insert(QStringLiteral("revs"), e.value(QStringLiteral("revs")));
        ev.insert(QStringLiteral("replayed"), true);
        events << ev;
    }
    for (const QJsonObject &ev : events)
        send(p, ev);
}

QStringList HostSession::descendantsOf(const QByteArray &index, const QString &id) const
{
    QStringList out;
    QDomDocument dom;
    if (!dom.setContent(index))
        return out;
    std::function<QDomElement(const QDomElement &)> find = [&](const QDomElement &e) -> QDomElement {
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            if (c.tagName() == QLatin1String("Item") && c.attribute(QStringLiteral("id")) == id)
                return c;
            const QDomElement r = find(c);
            if (!r.isNull())
                return r;
        }
        return QDomElement();
    };
    const QDomElement item = find(dom.documentElement());
    if (item.isNull())
        return out;
    std::function<void(const QDomElement &)> walk = [&](const QDomElement &e) {
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            if (c.tagName() == QLatin1String("Item"))
                out << c.attribute(QStringLiteral("id"));
            walk(c);
        }
    };
    walk(item);
    return out;
}

void HostSession::tick()
{
    if (!m_started)
        return;
    bool changed = false;
    for (const Lock &l : m_locks.expire()) {
        changed = true;
        for (Peer *p : peersOfOwner(l.owner))
            send(p, make(QLatin1String(T::LOCK_REVOKED), m_uuid, object({{QStringLiteral("resource"), l.resource}, {QStringLiteral("reason"), QStringLiteral("expired")}})));
        grantWaiting(l.resource);
    }
    if (!m_locks.newlyUnreachable().isEmpty())
        changed = true;
    for (const EditRequest &r : m_requests.dueNow()) {
        if (Peer *a = peerByOwner(r.asker))
            send(a, make(QLatin1String(T::EDIT_STATUS), m_uuid,
                         object({{QStringLiteral("resource"), r.resource}, {QStringLiteral("declined"), r.declines > 0}, {QStringLiteral("waitMs"), 0},
                                 {QStringLiteral("mayForce"), true}})));
    }
    if (changed)
        broadcastLocks();
}

void HostSession::broadcastLocks()
{
    broadcast(make(QLatin1String(T::LOCK_TABLE), m_uuid, object({{QStringLiteral("locks"), m_locks.toJson()}})));
    emit locksChanged();
}

// ---------------------------------------------------------------- commits

void HostSession::commitResult(Peer *p, const QJsonObject &request, const QString &status, const QString &code, const QJsonArray &rejected, qint64 seq,
                               const QJsonObject &revs, bool duplicate)
{
    QJsonObject body = object({{QStringLiteral("opId"), request.value(QStringLiteral("opId")).toString()}, {QStringLiteral("status"), status}});
    if (!code.isEmpty())
        body.insert(QStringLiteral("code"), code);
    if (!rejected.isEmpty())
        body.insert(QStringLiteral("rejected"), rejected);
    if (seq > 0)
        body.insert(QStringLiteral("seq"), double(seq));
    if (!revs.isEmpty())
        body.insert(QStringLiteral("revs"), revs);
    if (duplicate)
        body.insert(QStringLiteral("duplicate"), true);
    if (status != QLatin1String("ok"))
        emit refused(p->owner.device, code);
    send(p, replyTo(request, QLatin1String(T::COMMIT_RESULT), body));
}

namespace {
QJsonObject rejectJson(const QString &unit, const QString &code, const QString &holder, const QString &detail)
{
    QJsonObject o{{QStringLiteral("unit"), unit}, {QStringLiteral("code"), code}};
    if (!holder.isEmpty())
        o.insert(QStringLiteral("holder"), holder);
    if (!detail.isEmpty())
        o.insert(QStringLiteral("detail"), detail.left(300));
    return o;
}
} // namespace

void HostSession::onCommit(Peer *p, const QJsonObject &m)
{
    static const QRegularExpression idOk(QStringLiteral("^[0-9A-Za-z-]{8,64}$"));
    const QString opId = m.value(QStringLiteral("opId")).toString();
    if (!idOk.match(opId).hasMatch()) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("BAD_OP"));
        return;
    }
    if (qint64(m.value(QStringLiteral("epoch")).toDouble()) != m_epoch) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("WRONG_EPOCH"));
        return;
    }
    // the same operation again: the stored result, never a second application
    const auto known = m_ops.constFind(opId);
    if (known != m_ops.constEnd()) {
        commitResult(p, m, QStringLiteral("ok"), QStringLiteral("DUPLICATE"), QJsonArray(), known->seq, known->revs, true);
        return;
    }
    if (qint64(m.value(QStringLiteral("baseSeq")).toDouble()) < m_horizon) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("UNKNOWN_OLD"));
        return;
    }
    if (p->role != Role::Editor) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("READ_ONLY"));
        return;
    }
    if (m_broken) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("STORAGE_FAILED"));
        return;
    }
    if (QJsonDocument(m).toJson(QJsonDocument::Compact).size() > 512 * 1024) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("TOO_BIG"));
        return;
    }
    // ---- the units
    QList<Unit> units;
    QString why;
    for (const QJsonValue &v : m.value(QStringLiteral("units")).toArray()) {
        Unit u;
        if (!v.isObject() || !fromJson(v.toObject(), &u, &why)) {
            commitResult(p, m, QStringLiteral("error"), QStringLiteral("BAD_OP"), QJsonArray{rejectJson(QString(), QStringLiteral("BAD_OP"), QString(), why)});
            return;
        }
        units << u;
    }
    if (units.isEmpty()) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("BAD_OP"));
        return;
    }
    QHash<QString, Token> tokens;
    for (const QJsonValue &v : m.value(QStringLiteral("locks")).toArray()) {
        const Token t = tokenFromJson(v.toObject());
        if (t.valid())
            tokens.insert(t.resource, t);
    }
    QByteArray index, newIndex;
    QString indexErr;
    if (!vault::readFile(m_indexPath, &index, &indexErr)) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("STORAGE_FAILED"));
        return;
    }
    newIndex = index;
    QList<Reject> rejects;
    QSet<QString> direct;  // the resources the request changes (their revision goes up)
    QSet<QString> implied; // locks only (the documents below a trashed item)
    QList<FileOp> files;   // the final list of the request, a later entry of a path replaces an earlier one
    QSet<QString> existingLower;
    for (const ManifestEntry &e : m_manifest.entries)
        existingLower.insert(e.path.toLower());
    QHash<QString, QString> requestLower; // lower-case path -> path, of this request
    QStringList reacquire;
    qint64 putBytes = 0;
    for (const Unit &u : units) {
        QSet<QString> mine, mineImplied;
        auto bad = [&](const QString &code, const QString &holder = QString(), const QString &detail = QString()) {
            rejects << Reject{u.name, code, holder, detail};
        };
        const int before = rejects.size();
        for (const Op &op : u.ops) {
            for (const QString &r : resourcesOfOp(op))
                mine.insert(r);
            if (op.kind == Op::ItemTrash || op.kind == Op::ItemDelete)
                for (const QString &d : descendantsOf(newIndex, op.id))
                    mineImplied.insert(QStringLiteral("doc:") + d);
        }
        for (const FileOp &f : u.files) {
            QString pathWhy;
            if (!isAcceptablePath(f.path, &pathWhy)) {
                bad(QStringLiteral("BAD_PATH"), QString(), pathWhy);
                continue;
            }
            bool refusedPath = false;
            const QStringList res = resourcesOfPath(f.path, &refusedPath);
            if (refusedPath) {
                bad(QStringLiteral("BAD_PATH"), QString(), QStringLiteral("%1 may not be changed from here").arg(f.path));
                continue;
            }
            if (symlinkOnTheWay(m_folder, f.path)) {
                bad(QStringLiteral("BAD_PATH"), QString(), QStringLiteral("a symbolic link lies on the way to %1").arg(f.path));
                continue;
            }
            const QString lower = f.path.toLower();
            const bool exact = m_manifest.find(f.path) != nullptr;
            if ((!exact && existingLower.contains(lower)) || (requestLower.contains(lower) && requestLower.value(lower) != f.path)) {
                bad(QStringLiteral("BAD_PATH"), QString(), QStringLiteral("%1 differs from another path only by case").arg(f.path));
                continue;
            }
            requestLower.insert(lower, f.path);
            if (f.kind == FileOp::Put) {
                if (!m_incoming.has(f.hash)) {
                    bad(QStringLiteral("BLOB_MISSING"), QString(), f.path);
                    continue;
                }
                if (m_incoming.size(f.hash) != f.size && !vault::seals(m_incoming.path(f.hash))) {
                    bad(QStringLiteral("HASH_MISMATCH"), QString(), f.path);
                    continue;
                }
                putBytes += f.size;
            }
            for (const QString &r : res)
                mine.insert(r);
            // a later entry for the same path replaces an earlier one
            for (int i = files.size() - 1; i >= 0; --i)
                if (files.at(i).path == f.path)
                    files.removeAt(i);
            files << f;
        }
        // revisions and locks, resource by resource
        QStringList sorted = mine.values();
        sorted.sort();
        for (const QString &r : sorted) {
            if (!u.bases.contains(r)) {
                bad(QStringLiteral("BASE_MISMATCH"), QString(), QStringLiteral("no revision given for %1").arg(r));
                continue;
            }
            const bool baseOk = u.bases.value(r) == m_revs.value(r);
            const QList<Lock> blockers = m_locks.blockers(r, p->owner);
            if (!blockers.isEmpty()) {
                bad(QStringLiteral("LOCK_HELD"), blockers.first().ownerName, r);
                continue;
            }
            const Lock *mineLock = m_locks.find(r);
            if (tokens.contains(r)) {
                const bool good = mineLock && mineLock->owner == p->owner && m_locks.valid(tokens.value(r));
                if (!good) {
                    // an old token is never accepted; its holder gets a new lock only when nothing changed under it
                    if (baseOk && !mineLock) {
                        reacquire << r;
                    } else if (baseOk && mineLock && mineLock->owner == p->owner) {
                        // the token is old but the lock is the owner's own, newer one: fine, the lock is the owner's
                    } else {
                        bad(QStringLiteral("LOCK_STALE"), QString(), r);
                        continue;
                    }
                }
            }
            if (!baseOk) {
                bad(QStringLiteral("BASE_MISMATCH"), QString(), r);
                continue;
            }
        }
        for (const QString &r : mineImplied) {
            const QList<Lock> blockers = m_locks.blockers(r, p->owner);
            if (!blockers.isEmpty())
                bad(QStringLiteral("LOCK_HELD"), blockers.first().ownerName, r);
        }
        if (rejects.size() == before && !u.ops.isEmpty()) {
            QString err;
            if (!applyToIndex(&newIndex, u.ops, &err))
                bad(QStringLiteral("BAD_OP"), QString(), err);
        }
        if (rejects.size() == before) {
            direct.unite(mine);
            implied.unite(mineImplied);
        }
    }
    if (!rejects.isEmpty()) {
        QJsonArray list;
        for (const Reject &r : rejects)
            list.append(rejectJson(r.unit, r.code, r.holder, r.detail));
        commitResult(p, m, QStringLiteral("rejected"), rejects.first().code, list);
        return;
    }
    if (freeBytes() < putBytes + newIndex.size() + kSpareBytes) {
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("NO_SPACE"));
        return;
    }
    // locks that were given up for a request whose base still holds are taken again now (nobody else holds them)
    for (const QString &r : reacquire) {
        const LockTable::Attempt a = m_locks.acquire(p->owner, p->name, {r});
        if (a.ok && !a.locks.isEmpty())
            sendLockGranted(p, a.locks.first(), QString());
    }
    if (!reacquire.isEmpty())
        broadcastLocks();

    // ---- one batch: the files, the project file, the state and the log record
    const qint64 newSeq = m_seq + 1;
    QHash<QString, qint64> newRevs = m_revs;
    QJsonObject changedRevs;
    for (const QString &r : direct) {
        newRevs[r] = m_revs.value(r) + 1;
        changedRevs.insert(r, double(newRevs.value(r)));
    }
    OpList allOps;
    for (const Unit &u : units)
        allOps << u.ops;
    const QString actor = p->owner.device;
    const qint64 at = m_clock->wallMs();
    QJsonArray opsJson, logFiles;
    for (const Op &op : allOps)
        opsJson.append(toJson(op));
    const bool indexChanged = newIndex != index;
    QStringList putPaths;
    for (const FileOp &f : files) {
        logFiles.append(object({{QStringLiteral("path"), f.path},
                                {QStringLiteral("kind"), f.kind == FileOp::Put ? QStringLiteral("put") : f.kind == FileOp::Remove ? QStringLiteral("remove") : QStringLiteral("removeTree")}}));
        if (f.kind == FileOp::Put)
            putPaths << f.path;
    }
    // with a backend the project file and the derived files may change although the operations do not (a text's counts)
    const bool indexListed = indexChanged || m_backend;
    if (indexListed)
        logFiles.append(object({{QStringLiteral("path"), QFileInfo(m_indexPath).fileName()}, {QStringLiteral("kind"), QStringLiteral("put")}}));
    const QStringList derived = m_backend ? m_backend->derivedPaths() : QStringList();
    for (const QString &d : derived)
        logFiles.append(object({{QStringLiteral("path"), d}, {QStringLiteral("kind"), QStringLiteral("put")}}));
    const QJsonObject logEntryJson = object({{QStringLiteral("seq"), double(newSeq)},
                                             {QStringLiteral("opId"), opId},
                                             {QStringLiteral("actor"), actor},
                                             {QStringLiteral("at"), double(at)},
                                             {QStringLiteral("ops"), opsJson},
                                             {QStringLiteral("files"), logFiles},
                                             {QStringLiteral("revs"), changedRevs}});
    savebatch::Batch batch(m_folder, m_folder + QStringLiteral("/settings/journal.json"));
    QString err;
    bool staged = true;
    for (const FileOp &f : files) {
        if (f.kind != FileOp::Put)
            continue;
        QByteArray bytes;
        if (!m_incoming.read(f.hash, &bytes, &err) || !batch.stage(m_folder + QLatin1Char('/') + f.path, bytes, f.path, QStringLiteral("Live Sync"), &err)) {
            staged = false;
            break;
        }
    }
    if (staged && indexChanged)
        staged = batch.stage(m_indexPath, newIndex, QFileInfo(m_indexPath).fileName(), QStringLiteral("Live Sync"), &err);
    if (staged)
        staged = batch.stage(m_folder + QStringLiteral("/livesync/state.json"), [&] {
            const qint64 keep = m_genReserve;
            QByteArray b = QJsonDocument(object({{QStringLiteral("v"), 1},
                                                 {QStringLiteral("p"), m_uuid},
                                                 {QStringLiteral("epoch"), double(m_epoch)},
                                                 {QStringLiteral("seq"), double(newSeq)},
                                                 {QStringLiteral("genReserve"), double(keep)},
                                                 {QStringLiteral("revs"), toObject(newRevs)}}))
                                 .toJson(QJsonDocument::Compact);
            return b;
        }(), QStringLiteral("state"), QStringLiteral("Live Sync"), &err);
    if (staged)
        staged = batch.stage(logPath(newSeq), QJsonDocument(logEntryJson).toJson(QJsonDocument::Compact), QStringLiteral("log"), QStringLiteral("Live Sync"), &err);
    if (!staged || !batch.commit(&err)) {
        if (!staged)
            batch.drop();
        else // a commit that failed may have left a journal (kept for the next opening): no more commits until then
            m_broken = true;
        commitResult(p, m, QStringLiteral("error"), QStringLiteral("STORAGE_FAILED"), QJsonArray{rejectJson(QString(), QStringLiteral("STORAGE_FAILED"), QString(), err)});
        return;
    }
    // ---- after the batch: removals, then what the others are told
    for (const FileOp &f : files) {
        const QString abs = m_folder + QLatin1Char('/') + f.path;
        if (f.kind == FileOp::Remove)
            safesave::removeFile(abs);
        else if (f.kind == FileOp::RemoveTree && QFileInfo(abs).isDir())
            pathsafety::removeTreeInside(abs, m_folder);
    }
    if (m_backend) {
        QStringList removed;
        for (const FileOp &f : files)
            if (f.kind != FileOp::Put)
                removed << f.path;
        QString backendErr;
        if (!m_backend->afterCommit(allOps, putPaths, removed, &backendErr))
            std::fprintf(stderr, "Live Sync: derived data of commit %lld could not be recomputed: %s\n", static_cast<long long>(newSeq), qPrintable(backendErr));
    }
    m_seq = newSeq;
    m_revs = newRevs;
    QJsonArray eventFiles;
    auto upsert = [&](const QString &path) {
        qint64 size = 0;
        const QString sha = Manifest::fileSha256(m_folder + QLatin1Char('/') + path, &size);
        if (sha.isEmpty())
            return;
        auto it = std::lower_bound(m_manifest.entries.begin(), m_manifest.entries.end(), path, [](const ManifestEntry &e, const QString &s) { return e.path < s; });
        if (it != m_manifest.entries.end() && it->path == path) {
            it->size = size;
            it->sha256 = sha;
        } else {
            ManifestEntry e;
            e.path = path;
            e.size = size;
            e.sha256 = sha;
            m_manifest.entries.insert(it, e);
        }
        const PathInfo info = classify(path);
        eventFiles.append(object({{QStringLiteral("path"), path},
                                  {QStringLiteral("kind"), QStringLiteral("put")},
                                  {QStringLiteral("hash"), sha},
                                  {QStringLiteral("size"), double(size)},
                                  {QStringLiteral("rev"), double(info.resource.isEmpty() ? 0 : m_revs.value(info.resource))}}));
    };
    for (const FileOp &f : files) {
        if (f.kind == FileOp::Put)
            upsert(f.path);
        else if (f.kind == FileOp::Remove) {
            const int i = [&] {
                for (int k = 0; k < m_manifest.entries.size(); ++k)
                    if (m_manifest.entries.at(k).path == f.path)
                        return k;
                return -1;
            }();
            if (i >= 0)
                m_manifest.entries.removeAt(i);
            eventFiles.append(object({{QStringLiteral("path"), f.path}, {QStringLiteral("kind"), QStringLiteral("remove")}}));
        } else {
            const QString prefix = f.path + QLatin1Char('/');
            for (int k = m_manifest.entries.size() - 1; k >= 0; --k)
                if (m_manifest.entries.at(k).path.startsWith(prefix))
                    m_manifest.entries.removeAt(k);
            eventFiles.append(object({{QStringLiteral("path"), f.path}, {QStringLiteral("kind"), QStringLiteral("removeTree")}}));
        }
    }
    if (indexListed)
        upsert(QFileInfo(m_indexPath).fileName());
    for (const QString &d : derived) {
        if (QFileInfo::exists(m_folder + QLatin1Char('/') + d))
            upsert(d);
    }
    StoredResult sr;
    sr.seq = newSeq;
    sr.revs = changedRevs;
    sr.wallMs = at;
    m_ops.insert(opId, sr);
    m_opOrder << std::make_pair(newSeq, opId);
    m_logSeqs << std::make_pair(newSeq, at);
    if (++m_sincePrune >= 100) {
        m_sincePrune = 0;
        pruneLog();
    }
    commitResult(p, m, QStringLiteral("ok"), QString(), QJsonArray(), newSeq, changedRevs);
    QJsonObject ev = make(QLatin1String(T::COMMIT_EVENT), m_uuid);
    ev.insert(QStringLiteral("epoch"), double(m_epoch));
    ev.insert(QStringLiteral("seq"), double(newSeq));
    ev.insert(QStringLiteral("opId"), opId);
    ev.insert(QStringLiteral("actor"), actor);
    ev.insert(QStringLiteral("actorName"), p->name);
    ev.insert(QStringLiteral("ops"), opsJson);
    ev.insert(QStringLiteral("files"), eventFiles);
    ev.insert(QStringLiteral("revs"), changedRevs);
    ev.insert(QStringLiteral("derived"), QJsonObject());
    for (Peer *q : m_peers)
        sendEvent(q, ev);
    emit committed(newSeq, opId, actor);
}

// ---------------------------------------------------------------- locks and edit requests

void HostSession::sendLockGranted(Peer *to, const Lock &l, const QString &re)
{
    QJsonObject body = object({{QStringLiteral("resource"), l.resource},
                               {QStringLiteral("epoch"), double(l.epoch)},
                               {QStringLiteral("generation"), double(l.generation)},
                               {QStringLiteral("lease"), l.lease}});
    QJsonObject o = make(QLatin1String(T::LOCK_GRANTED), m_uuid, body);
    if (!re.isEmpty())
        o.insert(QStringLiteral("re"), re);
    send(to, o);
}

void HostSession::onLockRequest(Peer *p, const QJsonObject &m)
{
    const QString re = m.value(QStringLiteral("id")).toString();
    if (p->role != Role::Editor) {
        sendError(p, QStringLiteral("READ_ONLY"), re);
        return;
    }
    if (!m_acceptLocks) {
        sendError(p, QStringLiteral("HOST_BUSY"), re);
        return;
    }
    QStringList resources;
    for (const QJsonValue &v : m.value(QStringLiteral("resources")).toArray()) {
        if (!v.isString() || !validResource(v.toString())) {
            sendError(p, QStringLiteral("BAD_OP"), re, QStringLiteral("not a resource"));
            return;
        }
        resources << v.toString();
    }
    if (resources.isEmpty()) {
        sendError(p, QStringLiteral("BAD_OP"), re);
        return;
    }
    // a lock that is asked for again after a reconnect: only when the resource is as the asker last saw it
    const QJsonObject bases = m.value(QStringLiteral("bases")).toObject();
    for (const QString &r : resources)
        if (bases.contains(r) && qint64(bases.value(r).toDouble()) != m_revs.value(r)) {
            send(p, replyTo(m, QLatin1String(T::LOCK_DENIED),
                            object({{QStringLiteral("resource"), r}, {QStringLiteral("holder"), QString()}, {QStringLiteral("reason"), QStringLiteral("BASE_MISMATCH")}})));
            return;
        }
    const LockTable::Attempt a = m_locks.acquire(p->owner, p->name, resources);
    if (a.storageFailed) {
        sendError(p, QStringLiteral("STORAGE_FAILED"), re);
        return;
    }
    if (!a.ok) {
        send(p, replyTo(m, QLatin1String(T::LOCK_DENIED),
                        object({{QStringLiteral("resource"), a.wanted},
                                {QStringLiteral("held"), a.heldResource},
                                {QStringLiteral("holder"), a.holderName},
                                {QStringLiteral("holderDevice"), a.holder.device},
                                {QStringLiteral("reason"), a.heldResource.isEmpty() ? QStringLiteral("BAD_OP") : QStringLiteral("LOCK_HELD")}})));
        return;
    }
    for (const Lock &l : a.locks)
        sendLockGranted(p, l, re);
    broadcastLocks();
}

void HostSession::onLockRelease(Peer *p, const QJsonObject &m)
{
    bool any = false;
    for (const QJsonValue &v : m.value(QStringLiteral("resources")).toArray())
        if (v.isString() && m_locks.release(p->owner, v.toString())) {
            any = true;
            grantWaiting(v.toString());
        }
    if (any)
        broadcastLocks();
}

void HostSession::askHolder(const QString &resource)
{
    const Lock *l = m_locks.find(resource);
    if (!l)
        return;
    for (EditRequest *r : m_requests.forResource(resource))
        for (Peer *h : peersOfOwner(l->owner))
            send(h, make(QLatin1String(T::EDIT_ASKED), m_uuid,
                         object({{QStringLiteral("resource"), resource},
                                 {QStringLiteral("asker"), r->asker.device},
                                 {QStringLiteral("askerName"), r->askerName},
                                 {QStringLiteral("waitMs"), double(m_requests.remainingMs(*r))}})));
}

// a lock became free: the oldest asker that is still here gets it
void HostSession::grantWaiting(const QString &resource)
{
    if (m_locks.find(resource))
        return;
    for (EditRequest *r : m_requests.forResource(resource)) {
        Peer *a = peerByOwner(r->asker);
        if (!a) {
            continue;
        }
        const LockTable::Attempt at = m_locks.acquire(r->asker, r->askerName, {resource});
        if (at.ok && !at.locks.isEmpty()) {
            sendLockGranted(a, at.locks.first(), QString());
            m_requests.cancel(resource, r->asker);
            askHolder(resource); // the others go on asking, now the new holder
            return;
        }
    }
}

void HostSession::onEditRequest(Peer *p, const QJsonObject &m)
{
    const QString re = m.value(QStringLiteral("id")).toString();
    const QString resource = m.value(QStringLiteral("resource")).toString();
    if (!validResource(resource)) {
        sendError(p, QStringLiteral("BAD_OP"), re);
        return;
    }
    if (p->role != Role::Editor) {
        sendError(p, QStringLiteral("READ_ONLY"), re);
        return;
    }
    const QList<Lock> blockers = m_locks.blockers(resource, p->owner);
    if (blockers.isEmpty()) { // nobody holds it: it is simply given
        if (!m_acceptLocks) {
            sendError(p, QStringLiteral("HOST_BUSY"), re);
            return;
        }
        const LockTable::Attempt a = m_locks.acquire(p->owner, p->name, {resource});
        if (a.ok) {
            for (const Lock &l : a.locks)
                sendLockGranted(p, l, re);
            broadcastLocks();
        } else
            sendError(p, QStringLiteral("STORAGE_FAILED"), re);
        return;
    }
    const Lock &held = blockers.first();
    if (held.resource != resource) { // a whole-project lock is not something to ask for
        send(p, replyTo(m, QLatin1String(T::LOCK_DENIED),
                        object({{QStringLiteral("resource"), resource}, {QStringLiteral("held"), held.resource}, {QStringLiteral("holder"), held.ownerName},
                                {QStringLiteral("reason"), QStringLiteral("LOCK_HELD")}})));
        return;
    }
    const bool isNew = !m_requests.find(resource, p->owner);
    EditRequest &r = m_requests.open(resource, p->owner, p->name);
    send(p, make(QLatin1String(T::EDIT_STATUS), m_uuid,
                 object({{QStringLiteral("resource"), resource},
                         {QStringLiteral("declined"), r.declines > 0},
                         {QStringLiteral("waitMs"), double(m_requests.remainingMs(r))},
                         {QStringLiteral("mayForce"), m_requests.mayForce(r)},
                         {QStringLiteral("holder"), held.ownerName}})));
    if (isNew) {
        for (Peer *h : peersOfOwner(held.owner))
            send(h, make(QLatin1String(T::EDIT_ASKED), m_uuid,
                         object({{QStringLiteral("resource"), resource},
                                 {QStringLiteral("asker"), p->owner.device},
                                 {QStringLiteral("askerName"), p->name},
                                 {QStringLiteral("waitMs"), double(m_requests.remainingMs(r))}})));
    }
}

void HostSession::onEditAnswer(Peer *p, const QJsonObject &m)
{
    const QString re = m.value(QStringLiteral("id")).toString();
    const QString resource = m.value(QStringLiteral("resource")).toString();
    const Lock *held = m_locks.find(resource);
    if (!held || held->owner != p->owner) {
        sendError(p, QStringLiteral("LOCK_STALE"), re);
        return;
    }
    // the request named, else the oldest
    EditRequest *req = nullptr;
    const QString askerDevice = m.value(QStringLiteral("asker")).toString();
    for (EditRequest *r : m_requests.forResource(resource))
        if (askerDevice.isEmpty() || r->asker.device == askerDevice) {
            req = r;
            break;
        }
    if (!req)
        return; // the asker gave up meanwhile
    const EditRequest r = *req;
    if (m.value(QStringLiteral("handOver")).toBool()) {
        // the holder's changes came before this answer on the same connection, so they are committed by now
        const Owner old = held->owner;
        m_locks.release(old, resource);
        m_requests.cancel(resource, r.asker);
        Peer *a = peerByOwner(r.asker);
        if (a) {
            const LockTable::Attempt at = m_locks.acquire(r.asker, r.askerName, {resource});
            if (at.ok && !at.locks.isEmpty())
                sendLockGranted(a, at.locks.first(), QString());
            askHolder(resource);
        } else {
            grantWaiting(resource);
        }
        broadcastLocks();
        return;
    }
    // Decline: the asker's waiting time starts again and the asker is told
    m_requests.decline(resource, r.asker);
    if (Peer *a = peerByOwner(r.asker)) {
        const EditRequest *now = m_requests.find(resource, r.asker);
        send(a, make(QLatin1String(T::EDIT_STATUS), m_uuid,
                     object({{QStringLiteral("resource"), resource},
                             {QStringLiteral("declined"), true},
                             {QStringLiteral("waitMs"), double(now ? m_requests.remainingMs(*now) : kEditWaitMs)},
                             {QStringLiteral("mayForce"), false},
                             {QStringLiteral("holder"), p->name}})));
    }
}

void HostSession::onEditForce(Peer *p, const QJsonObject &m)
{
    const QString re = m.value(QStringLiteral("id")).toString();
    const QString resource = m.value(QStringLiteral("resource")).toString();
    EditRequest *r = m_requests.find(resource, p->owner);
    if (!r) {
        sendError(p, QStringLiteral("BAD_OP"), re, QStringLiteral("no open request"));
        return;
    }
    if (!m_requests.mayForce(*r)) { // only after the waiting time with no answer
        sendError(p, QStringLiteral("LOCK_HELD"), re, QStringLiteral("the waiting time has not run out"));
        return;
    }
    const Lock *held = m_locks.find(resource);
    if (!held) { // it was given up meanwhile
        m_requests.cancel(resource, p->owner);
        const LockTable::Attempt a = m_locks.acquire(p->owner, p->name, {resource});
        if (a.ok && !a.locks.isEmpty())
            sendLockGranted(p, a.locks.first(), re);
        broadcastLocks();
        return;
    }
    // the last committed state is kept first; when that fails nothing changes
    QString err;
    const QString label = QStringLiteral("Before takeover by %1").arg(p->name);
    const bool kept = m_snapshot ? m_snapshot(resource, label, &err) : defaultSnapshot(resource, label, &err);
    if (!kept) {
        sendError(p, QStringLiteral("STORAGE_FAILED"), re, err);
        return;
    }
    Lock previous;
    bool failed = false;
    const Lock fresh = m_locks.takeOver(resource, p->owner, p->name, &previous, &failed);
    if (failed) {
        sendError(p, QStringLiteral("STORAGE_FAILED"), re);
        return;
    }
    m_requests.cancel(resource, p->owner);
    for (Peer *h : peersOfOwner(previous.owner))
        send(h, make(QLatin1String(T::LOCK_REVOKED), m_uuid, object({{QStringLiteral("resource"), resource}, {QStringLiteral("reason"), QStringLiteral("takeover")},
                                                                         {QStringLiteral("by"), p->name}})));
    sendLockGranted(p, fresh, re);
    askHolder(resource);
    broadcastLocks();
}

void HostSession::onEditCancel(Peer *p, const QJsonObject &m)
{
    m_requests.cancel(m.value(QStringLiteral("resource")).toString(), p->owner);
}

bool HostSession::defaultSnapshot(const QString &resource, const QString &label, QString *error)
{
    if (!resource.startsWith(QLatin1String("doc:")))
        return true; // only documents have a text to keep
    const QString id = resource.mid(4);
    if (!pathsafety::isSafeId(id))
        return true;
    const QString dir = QStringLiteral("%1/livesync/protective/%2/%3-%4").arg(m_folder, id).arg(m_clock->wallMs()).arg(m_seq);
    const QString src = m_folder + QStringLiteral("/content/") + id;
    QDir().mkpath(dir);
    for (const QString &n : QDir(src).entryList(QDir::Files)) {
        if (!(n.startsWith(QLatin1String("text.")) || n.startsWith(QLatin1String("notes."))))
            continue;
        if (!safesave::copyFile(src + QLatin1Char('/') + n, dir + QLatin1Char('/') + n, error))
            return false;
    }
    return safesave::writeFile(dir + QStringLiteral("/label.txt"), label.toUtf8(), error);
}

} // namespace collab
} // namespace qb
