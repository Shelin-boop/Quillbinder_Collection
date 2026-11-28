#include "core/collab/ClientSession.h"

#include "core/PathSafety.h"
#include "core/SafeSave.h"
#include "core/SaveBatch.h"
#include "core/collab/PathClass.h"
#include "core/crypto/Vault.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStorageInfo>
#include <algorithm>

namespace qb {
namespace collab {

using namespace proto;

namespace {
QJsonObject object(std::initializer_list<std::pair<QString, QJsonValue>> l)
{
    QJsonObject o;
    for (const auto &p : l)
        o.insert(p.first, p.second);
    return o;
}
QString localDir(const QString &folder) { return folder + QStringLiteral("/livesync-local"); }
} // namespace

ClientSession::ClientSession(const Config &config, QObject *parent) : QObject(parent), m_cfg(config)
{
    m_cfg.folder = QDir::cleanPath(QDir(config.folder).absolutePath());
    m_outbox = std::make_unique<Outbox>(localDir(m_cfg.folder));
    m_free = [this] {
        QDir().mkpath(m_cfg.folder);
        return QStorageInfo(m_cfg.folder).bytesAvailable();
    };
    if (m_cfg.clock)
        connect(m_cfg.clock, &Clock::ticked, this, &ClientSession::tick);
}

ClientSession::~ClientSession() { detach(); }

void ClientSession::setState(State s)
{
    if (m_state == s)
        return;
    m_state = s;
    emit stateChanged(s);
}

void ClientSession::fail(const QString &code, const QString &why)
{
    m_failure = code + QLatin1String(": ") + why;
    setState(State::Failed);
    emit failedWith(code, why);
    if (m_link)
        m_link->close(code);
}

QString ClientSession::statePath() const
{
    return m_cfg.folder + (m_cfg.loopback ? QStringLiteral("/livesync-local/loopback.json") : QStringLiteral("/livesync-replica.json"));
}

bool ClientSession::open(QString *error)
{
    QDir().mkpath(m_cfg.folder);
    QByteArray raw;
    const QString path = statePath();
    if (QFileInfo::exists(path)) {
        if (!vault::readFile(path, &raw)) {
            if (error)
                *error = QStringLiteral("the replica's state cannot be read");
            return false;
        }
        const QJsonObject o = QJsonDocument::fromJson(raw).object();
        if (o.value(QStringLiteral("p")).toString() != m_cfg.uuid) {
            if (error)
                *error = QStringLiteral("the folder holds a copy of another project");
            return false;
        }
        m_epoch = qint64(o.value(QStringLiteral("epoch")).toDouble());
        m_seq = qint64(o.value(QStringLiteral("seq")).toDouble());
        m_ready = o.value(QStringLiteral("ready")).toBool();
        const QJsonObject r = o.value(QStringLiteral("revs")).toObject();
        for (auto it = r.constBegin(); it != r.constEnd(); ++it)
            m_revs.insert(it.key(), qint64(it.value().toDouble()));
    }
    return m_outbox->load(error);
}

bool ClientSession::saveReplicaState(bool ready, QString *error)
{
    QJsonObject r;
    for (auto it = m_revs.constBegin(); it != m_revs.constEnd(); ++it)
        r.insert(it.key(), double(it.value()));
    const QByteArray b = QJsonDocument(object({{QStringLiteral("p"), m_cfg.uuid},
                                               {QStringLiteral("epoch"), double(m_epoch)},
                                               {QStringLiteral("seq"), double(m_seq)},
                                               {QStringLiteral("ready"), ready},
                                               {QStringLiteral("revs"), r}}))
                            .toJson(QJsonDocument::Compact);
    QDir().mkpath(QFileInfo(statePath()).absolutePath());
    if (!safesave::writeFile(statePath(), b, error))
        return false;
    m_ready = ready;
    return true;
}

// ---------------------------------------------------------------- the link

void ClientSession::attach(net::Link *link)
{
    detach();
    m_link = link;
    m_hub = new BlobHub(link, &m_outbox->blobs(), link);
    m_hub->setFreeSpaceProbe([this] { return m_free(); });
    m_conns << connect(link, &net::Link::frameReceived, this, [this](const net::Frame &f) { onFrame(f); });
    m_conns << connect(link, &net::Link::closed, this, [this](const QString &) { detach(); });
    m_conns << connect(m_hub, &BlobHub::received, this, &ClientSession::onBlobReceived);
    m_conns << connect(m_hub, &BlobHub::failed, this, &ClientSession::onBlobFailed);
    m_conns << connect(m_hub, &BlobHub::idle, this, [this] {
        if (!m_inFlight.isEmpty() && m_state == State::Ready) {
            if (const OutboxEntry *e = m_outbox->find(m_inFlight))
                if (!m_requestSent) {
                    m_requestSent = true;
                    sendRequest(*e);
                }
        }
    });
    setState(State::Connecting);
    m_lastHb = m_cfg.clock ? m_cfg.clock->now() : 0;
    send(make(QLatin1String(T::HELLO), m_cfg.uuid,
              object({{QStringLiteral("protoMin"), kProtocol},
                      {QStringLiteral("protoMax"), kProtocol},
                      {QStringLiteral("name"), m_cfg.deviceName},
                      {QStringLiteral("session"), m_cfg.session}})));
}

void ClientSession::detach()
{
    const bool had = !m_conns.isEmpty() || m_link || m_hub;
    if (!had && (m_state == State::Idle || m_state == State::Offline || m_state == State::Failed))
        return;
    for (const QMetaObject::Connection &c : m_conns)
        disconnect(c);
    m_conns.clear();
    if (m_hub)
        m_hub->deleteLater();
    m_hub = nullptr;
    m_link = nullptr;
    m_inFlight.clear();
    m_requestSent = false;
    m_inFlightBlobs.clear();
    m_toFetch.clear();
    m_applying = false;
    m_resyncAsked = false;
    m_full = false;
    m_pages.clear();
    if (m_state != State::Failed)
        setState(State::Offline);
}

void ClientSession::send(const QJsonObject &m)
{
    if (m_link && m_link->state() == net::Link::State::Ready)
        m_link->sendJson(m);
}

bool ClientSession::sendMessage(const QJsonObject &m)
{
    if (!m_link || m_state == State::Offline)
        return false;
    send(m);
    return true;
}

void ClientSession::onFrame(const net::Frame &f)
{
    if (f.kind == net::Frame::Data) {
        if (m_hub)
            m_hub->handleData(f);
        return;
    }
    bool ok = false;
    const QJsonObject m = f.object(&ok);
    QString code, why;
    if (!ok || !validate(m, &code, &why)) {
        fail(ok ? code : QStringLiteral("BAD_FRAME"), ok ? why : QStringLiteral("not a message"));
        return;
    }
    onMessage(m);
}

void ClientSession::onMessage(const QJsonObject &m)
{
    const QString t = m.value(QStringLiteral("t")).toString();
    if (is(t, T::WELCOME))
        onWelcome(m);
    else if (is(t, T::JOINED))
        onJoined(m);
    else if (is(t, T::MANIFEST))
        onManifest(m);
    else if (is(t, T::COMMIT_EVENT))
        onEvent(m);
    else if (is(t, T::COMMIT_RESULT))
        onResult(m);
    else if (is(t, T::LOCK_TABLE))
        onLockTable(m);
    else if (is(t, T::ERROR))
        onError(m);
    else if (is(t, T::BLOB_PUT) || is(t, T::BLOB_DONE)) {
        if (m_hub)
            m_hub->handleMessage(m);
    } else if (is(t, T::LOCK_GRANTED)) {
        Token tk = tokenFromJson(m);
        if (tk.valid()) {
            m_held.insert(tk.resource, tk);
            emit lockGranted(tk.resource);
        }
    } else if (is(t, T::LOCK_DENIED)) {
        const QString r = m.value(QStringLiteral("resource")).toString();
        if (m_held.contains(r) && m.value(QStringLiteral("reason")).toString() == QLatin1String("BASE_MISMATCH"))
            dropHeldFor(r, QStringLiteral("lost"));
        emit lockDenied(r, m.value(QStringLiteral("holder")).toString(), m.value(QStringLiteral("reason")).toString());
    } else if (is(t, T::LOCK_REVOKED)) {
        dropHeldFor(m.value(QStringLiteral("resource")).toString(), m.value(QStringLiteral("reason")).toString());
    } else if (is(t, T::EDIT_ASKED)) {
        emit editAsked(m.value(QStringLiteral("resource")).toString(), m.value(QStringLiteral("askerName")).toString(),
                       qint64(m.value(QStringLiteral("waitMs")).toDouble()));
    } else if (is(t, T::EDIT_STATUS)) {
        const QString r = m.value(QStringLiteral("resource")).toString();
        const qint64 wait = qint64(m.value(QStringLiteral("waitMs")).toDouble());
        const bool may = m.value(QStringLiteral("mayForce")).toBool();
        m_waitUntil.insert(r, (m_cfg.clock ? m_cfg.clock->now() : 0) + wait);
        emit editStatus(r, m.value(QStringLiteral("declined")).toBool(), wait, may, m.value(QStringLiteral("holder")).toString());
    } else if (is(t, T::BYE)) {
        detach();
    } else if (!is(t, T::HB)) {
        if (!(m_extension && m_extension(m)))
            ; // not for us: ignored
    }
}

void ClientSession::onWelcome(const QJsonObject &)
{
    setState(State::Joining);
    send(make(QLatin1String(T::JOIN), m_cfg.uuid, object({{QStringLiteral("epoch"), double(m_epoch)}, {QStringLiteral("seq"), double(m_seq)}})));
}

void ClientSession::onJoined(const QJsonObject &m)
{
    m_role = m.value(QStringLiteral("role")).toString();
    m_head = qint64(m.value(QStringLiteral("head")).toDouble());
    const qint64 epoch = qint64(m.value(QStringLiteral("epoch")).toDouble());
    bool epochDiffers = false;
    if (m_epoch != 0 && epoch != m_epoch) {
        emit epochChanged(m_epoch, epoch);
        epochDiffers = true;
    }
    m_epoch = epoch;
    // the locks: ours are taken as they are (the host kept them), those missing are asked for again where nothing changed
    const QJsonArray table = m.value(QStringLiteral("locks")).toArray();
    QSet<QString> mine;
    m_others.clear();
    for (const QJsonValue &v : table) {
        const QJsonObject o = v.toObject();
        const QString r = o.value(QStringLiteral("resource")).toString();
        if (o.value(QStringLiteral("device")).toString() == m_cfg.deviceId && o.value(QStringLiteral("session")).toString() == m_cfg.session) {
            Token tk = tokenFromJson(o);
            m_held.insert(r, tk);
            mine.insert(r);
        } else {
            m_others.insert(r, Other{o.value(QStringLiteral("name")).toString(), o.value(QStringLiteral("device")).toString(),
                                     o.value(QStringLiteral("session")).toString(), o.value(QStringLiteral("unreachable")).toBool()});
        }
    }
    QStringList again;
    for (auto it = m_held.begin(); it != m_held.end();) {
        if (mine.contains(it.key()))
            ++it;
        else {
            again << it.key();
            it = m_held.erase(it);
        }
    }
    if (!again.isEmpty()) {
        QJsonObject bases;
        for (const QString &r : again)
            bases.insert(r, double(m_revs.value(r)));
        send(make(QLatin1String(T::LOCK_REQUEST), m_cfg.uuid, object({{QStringLiteral("resources"), QJsonArray::fromStringList(again)}, {QStringLiteral("bases"), bases}})));
    }
    emit locksChanged();
    setState(State::Syncing);
    if (!m_ready || epochDiffers) {
        startFullSync();
        return;
    }
    if (m_head > m_seq) {
        if (!m_events.contains(m_seq + 1) && !m_resyncAsked) {
            m_resyncAsked = true;
            send(make(QLatin1String(T::RESYNC), m_cfg.uuid, object({{QStringLiteral("fromSeq"), double(m_seq + 1)}})));
        }
        processEvents();
        return;
    }
    processEvents();
    checkSyncDone();
}

void ClientSession::onError(const QJsonObject &m)
{
    const QString code = m.value(QStringLiteral("code")).toString();
    const QString re = m.value(QStringLiteral("re")).toString();
    if (m_hub)
        m_hub->requestFailed(re, code);
    // the host does not take this device into the project (yet): the session stops; the service tries again later
    if (code == QLatin1String("NO_ACCESS") || code == QLatin1String("UNKNOWN_PROJECT") || code == QLatin1String("UNSUPPORTED_VERSION")) {
        fail(code, m.value(QStringLiteral("text")).toString());
        return;
    }
    if (m_lockReqs.contains(re)) {
        for (const QString &r : m_lockReqs.take(re))
            emit lockDenied(r, QString(), code);
        return;
    }
    if (code == QLatin1String("GAP")) {
        m_resyncAsked = false;
        startFullSync();
        return;
    }
    if (code == QLatin1String("DUPLICATE"))
        return;
    if (!m_inFlight.isEmpty() && (code == QLatin1String("NO_SPACE") || code == QLatin1String("HOST_BUSY") || code == QLatin1String("TOO_BIG")
                                  || code == QLatin1String("STORAGE_FAILED"))) {
        const QString op = m_inFlight;
        m_inFlight.clear();
        m_requestSent = false;
        m_retryAt = (m_cfg.clock ? m_cfg.clock->now() : 0) + 5000;
        emit problem(op, code);
    }
}

// ---------------------------------------------------------------- the full copy

void ClientSession::startFullSync()
{
    if (!m_link)
        return;
    m_full = true;
    ++m_fullSyncs;
    m_pages.clear();
    m_toFetch.clear();
    m_inFlightBlobs.clear();
    m_applying = false;
    m_events.clear(); // (events that come now are buffered again)
    setState(State::Syncing);
    const QJsonObject req = make(QLatin1String(T::MANIFEST_GET), m_cfg.uuid);
    m_manifestRe = req.value(QStringLiteral("id")).toString();
    send(req);
}

void ClientSession::onManifest(const QJsonObject &m)
{
    if (!m_full || m.value(QStringLiteral("re")).toString() != m_manifestRe)
        return;
    Manifest part;
    QString why;
    if (!Manifest::fromJson(m.value(QStringLiteral("manifest")).toObject(), &part, &why)) {
        fail(QStringLiteral("BAD_OP"), why);
        return;
    }
    const int page = m.value(QStringLiteral("page")).toInt();
    m_pagesExpected = m.value(QStringLiteral("pages")).toInt(1);
    m_pages.insert(page, part);
    if (m_pages.size() < m_pagesExpected)
        return;
    Manifest all = m_pages.value(0);
    for (auto it = m_pages.constBegin(); it != m_pages.constEnd(); ++it) {
        if (it.key() == 0)
            continue;
        if (it->seq != all.seq) { // the host moved on between two pages: ask again
            startFullSync();
            return;
        }
        all.entries << it->entries;
    }
    std::sort(all.entries.begin(), all.entries.end(), [](const ManifestEntry &a, const ManifestEntry &b) { return a.path < b.path; });
    QStringList paths;
    for (const ManifestEntry &e : all.entries)
        paths << e.path;
    if (!caseCollisions(paths).isEmpty() || all.entries.size() > kManifestMaxEntries) {
        fail(QStringLiteral("BAD_PATH"), QStringLiteral("the manifest is not acceptable"));
        return;
    }
    m_target = all;
    if (m_target.epoch != m_epoch && m_epoch != 0)
        emit epochChanged(m_epoch, m_target.epoch);
    m_epoch = m_target.epoch;
    if (m_cfg.loopback) { // the files are the host's own: nothing to compare or fetch
        m_plan = ManifestDiff();
        pumpDownloads();
        return;
    }
    Manifest local;
    QString err;
    if (!Manifest::scan(m_cfg.folder, localDir(m_cfg.folder) + QStringLiteral("/hashes.json"), {}, &local, &err)) {
        fail(QStringLiteral("STORAGE_FAILED"), err);
        return;
    }
    m_plan = compare(m_target, local);
    QSet<QString> seen;
    for (const QString &path : m_plan.fetch) {
        const QString h = m_target.find(path)->sha256;
        if (!m_outbox->blobs().has(h) && !seen.contains(h)) {
            seen.insert(h);
            m_toFetch << h;
        }
    }
    pumpDownloads();
}

void ClientSession::pumpDownloads()
{
    if (!m_full || !m_hub)
        return;
    while (m_inFlightBlobs.size() < 4 && !m_toFetch.isEmpty()) {
        const QString h = m_toFetch.takeFirst();
        if (m_outbox->blobs().has(h))
            continue;
        m_inFlightBlobs.insert(h);
        m_hub->request(h);
    }
    if (m_toFetch.isEmpty() && m_inFlightBlobs.isEmpty())
        placeFullSync();
}

void ClientSession::onBlobReceived(const QString &hash)
{
    m_inFlightBlobs.remove(hash);
    if (m_full)
        pumpDownloads();
    else if (m_applying && !m_events.isEmpty())
        applyEvent(m_events.first());
}

void ClientSession::onBlobFailed(const QString &hash, const QString &code)
{
    m_inFlightBlobs.remove(hash);
    if (m_full) {
        if (++m_fullTries <= 3)
            startFullSync();
        else
            fail(code, QStringLiteral("a file of the host could not be fetched"));
    } else if (m_applying) {
        m_applying = false;
        startFullSync(); // the host no longer has that content: the copy is made again from the manifest
    } else if (!m_inFlight.isEmpty() && (code == QLatin1String("NO_SPACE") || code == QLatin1String("HOST_BUSY") || code == QLatin1String("TOO_BIG"))) {
        // (an upload that the host refused)
    }
}

bool ClientSession::placeFiles(const QJsonArray &files, QString *error)
{
    if (m_cfg.loopback) {
        Q_UNUSED(error);
        return true; // the host wrote them
    }
    savebatch::Batch batch(m_cfg.folder, m_cfg.folder + QStringLiteral("/settings/journal.json"));
    QStringList removes, trees;
    for (const QJsonValue &v : files) {
        const QJsonObject f = v.toObject();
        const QString path = f.value(QStringLiteral("path")).toString();
        QString why;
        if (!isAcceptablePath(path, &why) || !classify(path).replicated() || symlinkOnTheWay(m_cfg.folder, path)) {
            if (error)
                *error = QStringLiteral("BAD_PATH: %1").arg(path);
            batch.drop();
            return false;
        }
        const QString kind = f.value(QStringLiteral("kind")).toString();
        if (kind == QLatin1String("put")) {
            QByteArray bytes;
            QString err;
            if (!m_outbox->blobs().read(f.value(QStringLiteral("hash")).toString(), &bytes, &err)
                || !batch.stage(m_cfg.folder + QLatin1Char('/') + path, bytes, path, QStringLiteral("Live Sync"), &err)) {
                if (error)
                    *error = err;
                batch.drop();
                return false;
            }
        } else if (kind == QLatin1String("remove"))
            removes << path;
        else if (kind == QLatin1String("removeTree"))
            trees << path;
    }
    QString err;
    if (!batch.commit(&err)) {
        if (error)
            *error = err;
        return false;
    }
    for (const QString &r : removes)
        safesave::removeFile(m_cfg.folder + QLatin1Char('/') + r);
    for (const QString &t : trees)
        if (QFileInfo(m_cfg.folder + QLatin1Char('/') + t).isDir())
            pathsafety::removeTreeInside(m_cfg.folder + QLatin1Char('/') + t, m_cfg.folder);
    return true;
}

void ClientSession::placeFullSync()
{
    QJsonArray files;
    for (const QString &path : m_plan.fetch) {
        const ManifestEntry *e = m_target.find(path);
        files.append(object({{QStringLiteral("path"), path}, {QStringLiteral("kind"), QStringLiteral("put")}, {QStringLiteral("hash"), e->sha256}}));
    }
    for (const QString &path : m_plan.remove)
        files.append(object({{QStringLiteral("path"), path}, {QStringLiteral("kind"), QStringLiteral("remove")}}));
    QString err;
    if (!placeFiles(files, &err)) {
        fail(QStringLiteral("STORAGE_FAILED"), err);
        return;
    }
    for (const QString &path : m_plan.fetch) {
        const QString h = m_target.find(path)->sha256;
        if (!m_outbox->referenced(h))
            m_outbox->blobs().remove(h);
    }
    m_seq = m_target.seq;
    m_revs = m_target.revs;
    m_head = qMax(m_head, m_seq);
    m_full = false;
    m_fullTries = 0;
    m_resyncAsked = false;
    if (!saveReplicaState(m_ready, &err)) {
        fail(QStringLiteral("STORAGE_FAILED"), err);
        return;
    }
    emit replicaReplaced();
    processEvents();
    checkSyncDone();
}

// ---------------------------------------------------------------- events

void ClientSession::onEvent(const QJsonObject &m)
{
    const qint64 seq = qint64(m.value(QStringLiteral("seq")).toDouble());
    if (seq <= m_seq)
        return;
    if (qint64(m.value(QStringLiteral("epoch")).toDouble()) != m_epoch) {
        emit epochChanged(m_epoch, qint64(m.value(QStringLiteral("epoch")).toDouble()));
        startFullSync();
        return;
    }
    m_events.insert(seq, m);
    m_head = qMax(m_head, seq);
    if (!m_full)
        processEvents();
}

void ClientSession::processEvents()
{
    if (m_full || m_applying || !m_link || (m_state != State::Syncing && m_state != State::Ready))
        return;
    while (!m_events.isEmpty() && m_events.firstKey() <= m_seq)
        m_events.erase(m_events.begin());
    if (m_events.isEmpty()) {
        checkSyncDone();
        return;
    }
    if (m_events.firstKey() > m_seq + 1) {
        if (!m_resyncAsked) {
            m_resyncAsked = true;
            send(make(QLatin1String(T::RESYNC), m_cfg.uuid, object({{QStringLiteral("fromSeq"), double(m_seq + 1)}})));
        }
        return;
    }
    m_resyncAsked = false;
    m_applying = true;
    applyEvent(m_events.first());
}

void ClientSession::applyEvent(QJsonObject ev)
{
    // the files first: every blob must be here
    bool waiting = false;
    for (const QJsonValue &v : ev.value(QStringLiteral("files")).toArray()) {
        const QJsonObject f = v.toObject();
        if (m_cfg.loopback || f.value(QStringLiteral("kind")).toString() != QLatin1String("put"))
            continue;
        const QString h = f.value(QStringLiteral("hash")).toString();
        if (!isHashHex(h)) {
            m_applying = false;
            fail(QStringLiteral("BAD_OP"), QStringLiteral("an event names no valid hash"));
            return;
        }
        if (m_outbox->blobs().has(h) || m_inFlightBlobs.contains(h))
            continue;
        if (m_hub && m_hub->isReceiving(h))
            continue;
        m_inFlightBlobs.insert(h);
        m_hub->request(h);
        waiting = true;
    }
    for (const QString &h : m_inFlightBlobs) {
        for (const QJsonValue &v : ev.value(QStringLiteral("files")).toArray())
            if (v.toObject().value(QStringLiteral("hash")).toString() == h)
                waiting = true;
    }
    if (waiting)
        return;
    QString err;
    const QJsonArray files = ev.value(QStringLiteral("files")).toArray();
    if (!placeFiles(files, &err)) {
        m_applying = false;
        if (err.startsWith(QLatin1String("BAD_PATH")))
            fail(QStringLiteral("BAD_PATH"), err);
        else
            fail(QStringLiteral("STORAGE_FAILED"), err);
        return;
    }
    const qint64 seq = qint64(ev.value(QStringLiteral("seq")).toDouble());
    const QJsonObject revs = ev.value(QStringLiteral("revs")).toObject();
    for (auto it = revs.constBegin(); it != revs.constEnd(); ++it)
        m_revs[it.key()] = qMax(m_revs.value(it.key()), qint64(it.value().toDouble()));
    m_seq = seq;
    QStringList paths;
    for (const QJsonValue &v : files) {
        const QJsonObject f = v.toObject();
        paths << f.value(QStringLiteral("path")).toString();
        const QString h = f.value(QStringLiteral("hash")).toString();
        if (!h.isEmpty() && !m_outbox->referenced(h))
            m_outbox->blobs().remove(h);
    }
    saveReplicaState(m_ready, &err);
    m_events.remove(seq);
    m_applying = false;
    const QString opId = ev.value(QStringLiteral("opId")).toString();
    const bool own = ev.value(QStringLiteral("actor")).toString() == m_cfg.deviceId && !ev.value(QStringLiteral("replayed")).toBool();
    emit eventApplied(seq, opId, ev.value(QStringLiteral("actor")).toString(), own, ev.value(QStringLiteral("ops")).toArray(), paths, revs);
    processEvents();
}

void ClientSession::checkSyncDone()
{
    if (m_full || m_applying || !m_link)
        return;
    if (m_state != State::Syncing && m_state != State::Joining)
        return;
    if (m_seq < m_head || !m_events.isEmpty())
        return;
    becomeReady();
}

void ClientSession::becomeReady()
{
    QString err;
    if (!m_ready && !saveReplicaState(true, &err)) {
        fail(QStringLiteral("STORAGE_FAILED"), err);
        return;
    }
    setState(State::Ready);
    emit readyToUse();
    pumpOutbox();
    flushReleases();
}

// ---------------------------------------------------------------- change requests

QString ClientSession::submit(QList<Unit> units, const QHash<QString, QByteArray> &blobs, QString *error)
{
    for (Unit &u : units) {
        QStringList res;
        for (const Op &op : u.ops)
            res << resourcesOfOp(op);
        for (const FileOp &f : u.files) {
            res << resourcesOfPath(f.path);
            if (f.kind == FileOp::Put && !blobs.contains(f.hash)) {
                if (error)
                    *error = QStringLiteral("the bytes of %1 are missing").arg(f.path);
                return QString();
            }
        }
        for (const QString &r : res)
            if (!u.bases.contains(r))
                u.bases.insert(r, m_revs.value(r));
    }
    QJsonArray arr;
    for (const Unit &u : units)
        arr.append(toJson(u));
    const QString id = m_outbox->add(arr, blobs, m_cfg.clock ? m_cfg.clock->wallMs() : 0, error);
    if (!id.isEmpty())
        pumpOutbox();
    return id;
}

QStringList ClientSession::resourcesOfUnits(const QJsonArray &units) const
{
    QStringList out;
    for (const QJsonValue &v : units) {
        Unit u;
        if (!v.isObject() || !fromJson(v.toObject(), &u))
            continue;
        for (const Op &op : u.ops)
            out << resourcesOfOp(op);
        for (const FileOp &f : u.files)
            out << resourcesOfPath(f.path);
    }
    out.removeDuplicates();
    return out;
}

QJsonArray ClientSession::rebased(const QJsonArray &units)
{
    QJsonArray out;
    for (const QJsonValue &v : units) {
        Unit u;
        if (!v.isObject() || !fromJson(v.toObject(), &u)) {
            out.append(v);
            continue;
        }
        for (auto it = u.bases.begin(); it != u.bases.end(); ++it) {
            // revisions that only this session's own earlier commits produced are no reason for a mismatch
            while (it.value() < m_revs.value(it.key()) && m_ownRevs.contains(it.key() + QLatin1Char('@') + QString::number(it.value() + 1)))
                ++it.value();
        }
        out.append(toJson(u));
    }
    return out;
}

void ClientSession::pumpOutbox()
{
    if (m_state != State::Ready || m_full || !m_inFlight.isEmpty() || !m_link)
        return;
    const qint64 now = m_cfg.clock ? m_cfg.clock->now() : 0;
    if (now < m_retryAt)
        return;
    const QList<OutboxEntry> pending = m_outbox->pending();
    if (pending.isEmpty()) {
        flushReleases();
        return;
    }
    OutboxEntry e = pending.first();
    for (const QString &h : e.blobs)
        if (!m_outbox->blobs().has(h)) { // a blob that is not there cannot be sent: a draft keeps what is left
            toDraft(e.opId, QStringLiteral("BLOB_MISSING"));
            return;
        }
    const QJsonArray units = rebased(e.units);
    if (units != e.units) {
        m_outbox->updateUnits(e.opId, units);
        e.units = units;
    }
    m_inFlight = e.opId;
    m_requestSent = false;
    // the contents first, then the request (the host answers a request whose contents are not there with BLOB_MISSING)
    for (const QString &h : e.blobs)
        m_hub->send(h, QString());
    if (m_hub->sendingCount() == 0 && !m_requestSent) {
        m_requestSent = true;
        sendRequest(e);
    }
}

void ClientSession::sendRequest(const OutboxEntry &e)
{
    QJsonArray tokens;
    for (const QString &r : resourcesOfUnits(e.units))
        if (m_held.contains(r))
            tokens.append(toJson(m_held.value(r)));
    QJsonObject body = object({{QStringLiteral("opId"), e.opId},
                               {QStringLiteral("epoch"), double(m_epoch)},
                               {QStringLiteral("baseSeq"), double(m_seq)},
                               {QStringLiteral("units"), e.units},
                               {QStringLiteral("locks"), tokens}});
    send(make(QLatin1String(T::COMMIT_REQUEST), m_cfg.uuid, body));
}

void ClientSession::toDraft(const QString &opId, const QString &reason)
{
    QString err;
    if (m_outbox->toDraft(opId, reason, &err))
        emit draftKept(opId, reason);
    if (m_inFlight == opId) {
        m_inFlight.clear();
        m_requestSent = false;
    }
    flushReleases();
    pumpOutbox();
}

void ClientSession::onResult(const QJsonObject &m)
{
    const QString opId = m.value(QStringLiteral("opId")).toString();
    if (!m_outbox->find(opId))
        return; // an answer to something that is settled already
    const QString status = m.value(QStringLiteral("status")).toString();
    const QString code = m.value(QStringLiteral("code")).toString();
    if (status == QLatin1String("ok")) {
        const QJsonObject revs = m.value(QStringLiteral("revs")).toObject();
        for (auto it = revs.constBegin(); it != revs.constEnd(); ++it) {
            const qint64 r = qint64(it.value().toDouble());
            m_revs[it.key()] = qMax(m_revs.value(it.key()), r);
            m_ownRevs.insert(it.key() + QLatin1Char('@') + QString::number(r));
        }
        m_outbox->done(opId);
        if (m_inFlight == opId) {
            m_inFlight.clear();
            m_requestSent = false;
        }
        m_blobRetries.remove(opId);
        emit committed(opId, qint64(m.value(QStringLiteral("seq")).toDouble()));
        flushReleases();
        pumpOutbox();
        return;
    }
    if (status == QLatin1String("rejected")) {
        const QJsonArray rejects = m.value(QStringLiteral("rejected")).toArray();
        if (code == QLatin1String("BLOB_MISSING") && m_blobRetries[opId]++ < 3) {
            m_inFlight.clear();
            m_requestSent = false;
            pumpOutbox();
            return;
        }
        emit rejected(opId, rejects);
        toDraft(opId, code.isEmpty() ? QStringLiteral("REJECTED") : code);
        return;
    }
    // an error for the whole request
    if (code == QLatin1String("WRONG_EPOCH")) {
        m_inFlight.clear();
        m_requestSent = false;
        startFullSync();
        return;
    }
    if (code == QLatin1String("NO_SPACE") || code == QLatin1String("HOST_BUSY") || code == QLatin1String("STORAGE_FAILED")) {
        m_inFlight.clear();
        m_requestSent = false;
        m_retryAt = (m_cfg.clock ? m_cfg.clock->now() : 0) + 5000;
        emit problem(opId, code);
        return;
    }
    // UNKNOWN_OLD, READ_ONLY, TOO_BIG, BAD_OP ...: the host will not take it as it is; the content is kept as a draft (an
    // operation the host forgot may be in the project already: the draft is then compared with the replica by the writer)
    toDraft(opId, code);
}

// ---------------------------------------------------------------- locks

QString ClientSession::sendLockRequest(const QStringList &resources)
{
    const QJsonObject m = make(QLatin1String(T::LOCK_REQUEST), m_cfg.uuid, object({{QStringLiteral("resources"), QJsonArray::fromStringList(resources)}}));
    m_lockReqs.insert(m.value(QStringLiteral("id")).toString(), resources);
    send(m);
    return m.value(QStringLiteral("id")).toString();
}

void ClientSession::lock(const QStringList &resources)
{
    QStringList need;
    for (const QString &r : resources)
        if (!m_held.contains(r))
            need << r;
    if (!need.isEmpty())
        sendLockRequest(need);
}

void ClientSession::unlock(const QStringList &resources)
{
    for (const QString &r : resources)
        if (m_held.contains(r) && !m_releasing.contains(r))
            m_releasing << r;
    flushReleases();
}

void ClientSession::flushReleases()
{
    if (m_state != State::Ready)
        return;
    // a lock goes back only when no pending request needs it (the flush is confirmed)
    QStringList busy;
    for (const OutboxEntry &e : m_outbox->pending())
        busy << resourcesOfUnits(e.units);
    QStringList now;
    for (const QString &r : m_releasing)
        if (!busy.contains(r))
            now << r;
    if (!now.isEmpty() && m_inFlight.isEmpty()) {
        for (const QString &r : now) {
            m_releasing.removeAll(r);
            m_held.remove(r);
        }
        send(make(QLatin1String(T::LOCK_RELEASE), m_cfg.uuid, object({{QStringLiteral("resources"), QJsonArray::fromStringList(now)}})));
    }
    // hand-overs that waited for the flush
    if (m_outbox->pending().isEmpty() && m_inFlight.isEmpty()) {
        const QList<std::pair<QString, QString>> answers = m_answers;
        m_answers.clear();
        for (const auto &a : answers)
            answerEdit(a.first, true, a.second);
    }
}

void ClientSession::dropHeldFor(const QString &resource, const QString &reason)
{
    const bool had = m_held.remove(resource) > 0;
    m_releasing.removeAll(resource);
    // what was meant for it can no longer be sent: the pending requests that touch it become drafts
    for (const OutboxEntry &e : m_outbox->pending())
        if (resourcesOfUnits(e.units).contains(resource))
            toDraft(e.opId, QStringLiteral("LOCK_REVOKED"));
    if (had || !reason.isEmpty())
        emit lockRevoked(resource, reason);
}

void ClientSession::onLockTable(const QJsonObject &m)
{
    m_others.clear();
    for (const QJsonValue &v : m.value(QStringLiteral("locks")).toArray()) {
        const QJsonObject o = v.toObject();
        if (o.value(QStringLiteral("device")).toString() == m_cfg.deviceId && o.value(QStringLiteral("session")).toString() == m_cfg.session)
            continue;
        m_others.insert(o.value(QStringLiteral("resource")).toString(),
                        Other{o.value(QStringLiteral("name")).toString(), o.value(QStringLiteral("device")).toString(),
                              o.value(QStringLiteral("session")).toString(), o.value(QStringLiteral("unreachable")).toBool()});
    }
    emit locksChanged();
}

void ClientSession::askToEdit(const QString &resource) { send(make(QLatin1String(T::EDIT_REQUEST), m_cfg.uuid, object({{QStringLiteral("resource"), resource}}))); }
void ClientSession::cancelAsk(const QString &resource)
{
    m_waitUntil.remove(resource);
    send(make(QLatin1String(T::EDIT_CANCEL), m_cfg.uuid, object({{QStringLiteral("resource"), resource}})));
}
void ClientSession::takeOver(const QString &resource) { send(make(QLatin1String(T::EDIT_FORCE), m_cfg.uuid, object({{QStringLiteral("resource"), resource}}))); }

void ClientSession::answerEdit(const QString &resource, bool handOver, const QString &askerDevice)
{
    if (handOver && (!m_outbox->pending().isEmpty() || !m_inFlight.isEmpty())) {
        m_answers << std::make_pair(resource, askerDevice);
        return;
    }
    QJsonObject body = object({{QStringLiteral("resource"), resource}, {QStringLiteral("handOver"), handOver}});
    if (!askerDevice.isEmpty())
        body.insert(QStringLiteral("asker"), askerDevice);
    send(make(QLatin1String(T::EDIT_ANSWER), m_cfg.uuid, body));
    if (handOver) {
        m_held.remove(resource);
        m_releasing.removeAll(resource);
        emit lockRevoked(resource, QStringLiteral("handover"));
    }
}

qint64 ClientSession::editWaitMs(const QString &resource) const
{
    if (!m_waitUntil.contains(resource) || !m_cfg.clock)
        return 0;
    return qMax<qint64>(0, m_waitUntil.value(resource) - m_cfg.clock->now());
}

void ClientSession::tick()
{
    if (!m_cfg.clock || !m_link || m_state == State::Offline || m_state == State::Failed)
        return;
    const qint64 now = m_cfg.clock->now();
    if (now - m_lastHb >= kHeartbeatMs) {
        m_lastHb = now;
        send(make(QLatin1String(T::HB), m_cfg.uuid));
    }
    if (m_retryAt && now >= m_retryAt) {
        m_retryAt = 0;
        pumpOutbox();
    }
}

} // namespace collab
} // namespace qb
