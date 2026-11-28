#include "core/collab/Service.h"
#include "core/collab/HostBackup.h"

#include "core/Project.h"
#include "core/TestSwitches.h"
#include "core/crypto/Vault.h"
#include "core/collab/proto/Messages.h"
#include "core/SafeSave.h"
#include "core/collab/PathClass.h"
#include "core/collab/ScopedLink.h"
#include "core/collab/ServiceProtocol.h"

#include <QCoreApplication>
#include <cstdio>
#include <QCryptographicHash>
#include <QDomDocument>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QPointer>
#include <QStorageInfo>

namespace qb {
namespace collab {

using net::Frame;
using net::Link;
using net::MemLink;
using net::PeerInfo;
using net::TlsLink;
using net::TrustPolicy;

// ---------------------------------------------------------------- the parts of the service

struct Service::MeshPeer {
    QPointer<TlsLink> link;    // the one connection that is Ready
    QPointer<TlsLink> dialing; // our attempt
    bool outgoing = false;
    qint64 nextTry = 0;
    int tries = 0, turn = 0;
    qint64 lastFrame = 0;
};

struct Service::Proj {
    RegistryEntry entry;
    std::unique_ptr<HostSession> host;
    std::unique_ptr<ProjectHostBackend> backend;
    std::unique_ptr<HostBackup> jobs;           // the host's backups and snapshot jobs (package 8); gone before the host
    std::unique_ptr<HostJobsClient> jobsClient; // a replica's side of them: the backup state and the snapshot calls
    std::unique_ptr<vault::Scope> scope; // the host's folder is in the vault while it is hosted (an encrypted project)
    std::unique_ptr<ClientSession> client; // a replica's session, or the loopback session of a hosted project
    QPointer<TlsLink> link;                // a client's connection to its host
    QPointer<MemLink> loop;                // the host's loopback end
    QTimer retry;
    int tries = 0;
    int addressTurn = 0;
    QString problem;                       // why it does not run (a host that could not start ...)
    QSet<QString> online;                  // device ids of the joined peers (a host)
    QHash<QString, QString> peerNames;     // device id -> name
    QHash<QString, QString> lastNote;
    // who hosts (package 6)
    std::unique_ptr<HostingBackend> hb;
    std::unique_ptr<Authority> auth;
    std::unique_ptr<CommitLedger> ledger;
    QPointer<Link> rawLink;                // the old host's connection after this device took over (carries the hand-over's last message)
    QString rawPeer;
    bool retired = false;                  // a host that handed over: commits are refused until it follows the new host
    QStringList notices;
    // direct links to the other members and what their signed presence messages said (package 10)
    QHash<QString, MeshPeer> mesh;
    QHash<QString, QPair<QJsonObject, QString>> book; // device id -> its last presence record and the address it was seen at
    QHash<QString, double> bookAt;                    // device id -> the time of that record (older ones are ignored)
};

struct Service::Window {
    QPointer<Link> link;
    QSet<QString> uuids;
    QHash<QString, PendingSubmit> submits; // by message id
};

// ---------------------------------------------------------------- the status model of this service

class ServiceStatus : public StatusModel, public StatusActions {
    Q_OBJECT
public:
    explicit ServiceStatus(Service *s) : StatusModel(s), m_s(s)
    {
        connect(s, &Service::projectsChanged, this, &StatusModel::projectsChanged);
        connect(s, &Service::projectChanged, this, &StatusModel::projectChanged);
        connect(s, &Service::devicesChanged, this, &StatusModel::devicesChanged);
        connect(s, &Service::locksChanged, this, &StatusModel::locksChanged);
    }
    bool serviceRunning() const override { return m_s->isRunning(); }
    QList<ProjectStatus> projects() const override
    {
        QList<ProjectStatus> l;
        for (const QString &u : m_s->m_projects.keys())
            l << project(u);
        std::sort(l.begin(), l.end(), [](const ProjectStatus &a, const ProjectStatus &b) { return a.title.toLower() < b.title.toLower(); });
        return l;
    }
    ProjectStatus project(const QString &uuid) const override;
    ProjectStatus projectOfFolder(const QString &folder) const override
    {
        const QString c = QDir::cleanPath(QFileInfo(folder).absoluteFilePath());
        for (auto it = m_s->m_projects.constBegin(); it != m_s->m_projects.constEnd(); ++it)
            if (QDir::cleanPath(it.value()->entry.folder) == c)
                return project(it.key());
        return ProjectStatus();
    }
    QList<DeviceStatus> devices() const override
    {
        QList<DeviceStatus> l;
        if (!m_s->m_peers)
            return l;
        for (const net::PeerRecord &r : m_s->m_peers->peers()) {
            DeviceStatus d;
            d.deviceId = r.deviceId();
            d.name = r.name;
            d.fingerprint = QString::fromLatin1(r.pin.left(4).toHex());
            // online: a session of some project is connected to it
            for (const Service::Proj *p : m_s->m_projects) {
                if (p->online.contains(d.deviceId) || (!p->entry.host && p->entry.hostDeviceId == d.deviceId && p->client && p->client->state() == ClientSession::State::Ready))
                    d.online = true;
            }
            d.lastSeen = QDateTime::fromSecsSinceEpoch(r.pairedAt);
            l << d;
        }
        return l;
    }
    LockInfo lockInfo(const QString &uuid, const QString &resource) const override;

    void askToEdit(const QString &uuid, const QString &resource) override
    {
        if (ClientSession *c = m_s->clientSession(uuid)) {
            m_s->m_asks.insert(uuid + QLatin1Char('|') + resource);
            m_s->m_declined.remove(uuid + QLatin1Char('|') + resource);
            c->askToEdit(resource);
            emit m_s->locksChanged(uuid);
        }
    }
    void cancelAsk(const QString &uuid, const QString &resource) override
    {
        if (ClientSession *c = m_s->clientSession(uuid)) {
            m_s->m_asks.remove(uuid + QLatin1Char('|') + resource);
            c->cancelAsk(resource);
            emit m_s->locksChanged(uuid);
        }
    }
    void takeOver(const QString &uuid, const QString &resource) override
    {
        if (ClientSession *c = m_s->clientSession(uuid))
            c->takeOver(resource);
    }
    void stopEditing(const QString &uuid, const QString &resource) override
    {
        if (ClientSession *c = m_s->clientSession(uuid))
            c->unlock({resource});
    }
    void requestBackup(const QString &uuid) override
    {
        Service::Proj *p = m_s->m_projects.value(uuid);
        if (!p)
            return;
        ServiceScope scope;
        if (p->jobs) {
            QString why;
            if (!p->jobs->requestBackup(&why) && !why.isEmpty())
                emit m_s->notice(why); // (a backup that runs already, a project that cannot be backed up now)
        } else if (p->jobsClient) {
            if (!p->jobsClient->requestBackup())
                emit m_s->notice(tr("The backup could not be asked for: this device is not connected to the host."));
        }
    }
    void stopSharing(const QString &uuid) override { m_s->stopSharing(uuid); }

private:
    Service *m_s;
};

static QString clean(const QString &p) { return QDir::cleanPath(QFileInfo(p).absoluteFilePath()); }

// ---------------------------------------------------------------- start and stop

Service::Service(const Config &config, QObject *parent)
    : QObject(parent), m_cfg(config), m_pairWindow([this] { return m_clock ? m_clock->now() : qint64(0); })
{
    m_idleTimer.setSingleShot(true);
    m_idleTimer.setInterval(config.idleMs);
    connect(&m_idleTimer, &QTimer::timeout, this, [this] {
        if (m_windows.isEmpty() && !m_keepHosting)
            emit idle();
    });
    m_status = std::make_unique<ServiceStatus>(this); // (stable from here on: the screens connect to it before the service runs)
    connect(&m_pairWindow, &net::PairingWindow::closed, this, [this] {
        if (m_discovery)
            m_discovery->setPairingOpen(false);
    });
}

Service::~Service() { stop(); }

StatusModel *Service::statusModel() const { return m_status.get(); }
StatusActions *Service::statusActions() const { return m_status.get(); }

bool Service::start(QString *error)
{
    if (m_running)
        return true;
    auto fail = [&](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    if (m_cfg.dir.isEmpty())
        m_cfg.dir = net::DeviceIdentity::standardDir();
    m_cfg.dir = clean(m_cfg.dir);
    QDir().mkpath(m_cfg.dir);
    if (m_cfg.replicasDir.isEmpty())
        m_cfg.replicasDir = m_cfg.dir + QStringLiteral("/replicas");
    if (m_cfg.identity) {
        m_id = m_cfg.identity;
    } else {
        QString why;
        m_ownId = net::DeviceIdentity::forThisDevice(&why);
        if (!m_ownId)
            return fail(why);
        m_id = m_ownId.get();
    }
    if (m_cfg.clock)
        m_clock = m_cfg.clock;
    else {
        m_ownClock = std::make_unique<SystemClock>();
        m_clock = m_ownClock.get();
    }
    m_scopedClock = std::make_unique<ScopedClock>(m_clock); // what the sessions do on a tick is the service's work
    m_peers = std::make_unique<net::PeerStore>(m_cfg.dir);
    connect(m_peers.get(), &net::PeerStore::changed, this, &Service::devicesChanged);
    m_registry = std::make_unique<Registry>(m_cfg.dir);
    if (m_cfg.listen) {
        net::TrustPolicy trust = memberTrust();
        m_server = std::make_unique<net::TlsServer>(m_id, trust);
        if (!m_server->listen(m_cfg.address, m_cfg.port) && m_cfg.port != 0 && !m_server->listen(m_cfg.address, 0)) {
            m_server.reset();
            return fail(QStringLiteral("the service cannot listen for other devices"));
        }
        connect(m_server.get(), &net::TlsServer::newLink, this, &Service::onTlsLink);
    }
    if (m_cfg.discovery) {
        m_discovery.reset(Discovery::createUdp());
        if (m_discovery && m_discovery->start() && m_server)
            m_discovery->publish(m_id->instanceId(), m_server->port(), m_pairWindow.isOpen());
        if (m_discovery) {
            connect(m_discovery.get(), &Discovery::peerFound, this, &Service::discoveredChanged);
            connect(m_discovery.get(), &Discovery::peerChanged, this, &Service::discoveredChanged);
            connect(m_discovery.get(), &Discovery::peerLost, this, &Service::discoveredChanged);
        }
    }
    m_localName = m_cfg.localName.isEmpty() ? LocalLink::serverName(m_cfg.dir) : m_cfg.localName;
    if (m_cfg.localSocket) {
        m_local = std::make_unique<LocalServer>();
        QString why;
        if (!m_local->listen(m_localName, &why)) {
            m_local.reset();
            // (nothing that was opened stays open: another service of this device is running)
            if (m_discovery) {
                m_discovery->unpublish();
                m_discovery->stop();
            }
            m_discovery.reset();
            m_server.reset();
            return fail(why);
        }
        connect(m_local.get(), &LocalServer::newLink, this, [this](LocalLink *l) { attachWindow(l); });
    }
    m_running = true;
    m_presence.setInterval(1000);
    connect(&m_presence, &QTimer::timeout, this, [this] {
        for (Proj *p : m_projects) {
            if (!p->auth)
                continue;
            ServiceScope scope;
            for (const QString &d : p->online)
                p->auth->heardFrom(d); // a connected device was heard of: the connection is the heartbeat
        }
        meshTick();
    });
    m_presence.start();
    // the projects of the registry resume
    for (const RegistryEntry &e : m_registry->entries()) {
        if (e.stopped || !QFileInfo(e.folder).isDir())
            continue;
        if (e.host)
            startHost(e);
        else
            startClient(e);
    }
    if (m_cfg.quitWhenIdle)
        m_idleTimer.start();
    emit projectsChanged();
    return true;
}

void Service::stop()
{
    if (!m_running && m_projects.isEmpty())
        return;
    m_running = false;
    m_presence.stop();
    const QList<Window *> windows = m_windows; // (closing a link removes its window from the list)
    for (Window *w : windows) {
        if (m_windows.contains(w) && w->link)
            w->link->close();
    }
    const QStringList ids = m_projects.keys();
    for (const QString &u : ids)
        stopProject(u, false);
    for (Window *w : m_windows)
        delete w;
    m_windows.clear();
    if (m_discovery) {
        m_discovery->unpublish();
        m_discovery->stop();
    }
    m_discovery.reset();
    if (m_server)
        m_server->close();
    m_server.reset();
    if (m_local)
        m_local->close();
    m_local.reset();
    m_pairWindow.close();
}

void Service::setKeepHosting(bool on)
{
    m_keepHosting = on;
    if (!on)
        checkIdle();
}

void Service::checkIdle()
{
    if (!m_cfg.quitWhenIdle)
        return;
    if (m_windows.isEmpty() && !m_keepHosting)
        m_idleTimer.start();
    else
        m_idleTimer.stop();
}

int Service::hostedCount() const
{
    int n = 0;
    for (const Proj *p : m_projects)
        n += p->entry.host && p->host;
    return n;
}

bool Service::isHosting(const QString &uuid) const
{
    const Proj *p = m_projects.value(uuid);
    return p && p->entry.host && p->host;
}

HostSession *Service::hostSession(const QString &uuid) const
{
    const Proj *p = m_projects.value(uuid);
    return p ? p->host.get() : nullptr;
}

ClientSession *Service::clientSession(const QString &uuid) const
{
    const Proj *p = m_projects.value(uuid);
    return p ? p->client.get() : nullptr;
}

// ---------------------------------------------------------------- hosted projects

// the files Live Sync keeps in a hosted folder for one project's authority
static void removeAuthorityFiles(const QString &folder)
{
    for (const char *n : {"/livesync/authority.json", "/livesync/membership.json", "/livesync-local/ledger.json"})
        QFile::remove(folder + QLatin1String(n));
}

static bool writeHostState(const QString &folder, const QString &uuid, qint64 epoch, qint64 seq, const QHash<QString, qint64> &revs)
{
    QDir(folder + QStringLiteral("/livesync/log")).removeRecursively();
    QFile::remove(folder + QStringLiteral("/livesync/hashes.json"));
    QDir().mkpath(folder + QStringLiteral("/livesync/log"));
    QJsonObject r;
    for (auto it = revs.constBegin(); it != revs.constEnd(); ++it)
        r.insert(it.key(), double(it.value()));
    const QByteArray b = QJsonDocument(QJsonObject{{QStringLiteral("v"), 1}, {QStringLiteral("p"), uuid}, {QStringLiteral("epoch"), double(epoch)},
                                                   {QStringLiteral("seq"), double(seq)}, {QStringLiteral("genReserve"), 0}, {QStringLiteral("revs"), r}})
                             .toJson(QJsonDocument::Compact);
    return safesave::writeFile(folder + QStringLiteral("/livesync/state.json"), b, nullptr);
}

static bool writeReplicaState(const QString &folder, const QString &uuid, qint64 epoch, qint64 seq, const QHash<QString, qint64> &revs)
{
    QJsonObject r;
    for (auto it = revs.constBegin(); it != revs.constEnd(); ++it)
        r.insert(it.key(), double(it.value()));
    const QByteArray b = QJsonDocument(QJsonObject{{QStringLiteral("p"), uuid}, {QStringLiteral("epoch"), double(epoch)}, {QStringLiteral("seq"), double(seq)},
                                                   {QStringLiteral("ready"), true}, {QStringLiteral("revs"), r}})
                             .toJson(QJsonDocument::Compact);
    return safesave::writeFile(folder + QStringLiteral("/livesync-replica.json"), b, nullptr);
}

// ---------------------------------------------------------------- what the authority asks of the service

class ServiceHosting : public HostingBackend {
public:
    ServiceHosting(Service *s, Service::Proj *p) : m_s(s), m_p(p) {}
    qint64 seq() const override { return m_p->host ? m_p->host->seq() : m_p->client ? m_p->client->seq() : 0; }
    QString stateHash() const override
    {
        if (m_p->host)
            return m_p->host->manifest().stateHash();
        Manifest mf;
        const QString folder = m_p->entry.folder;
        if (!Manifest::scan(folder, folder + QStringLiteral("/livesync-local/hashes.json"), m_p->client ? m_p->client->revs() : QHash<QString, qint64>(), &mf))
            return QString();
        return mf.stateHash();
    }
    bool replicaReady() const override
    {
        return m_p->client && !m_p->client->isLoopback() && m_p->client->replicaReady(); // (the copy is complete; the connection may be gone)
    }
    void hostingAfterFailover() override { m_failover = true; }
    bool startHosting(qint64 epoch, const QJsonArray &locks, QString *error) override
    {
        // a planned handover brings the lock table along (the editors keep their locks); after a failover the table starts empty and
        // the devices that were editing ask for their locks again during a short grace time
        const bool failover = m_failover;
        m_failover = false;
        return m_s->activateHost(m_p, epoch, error, locks, failover);
    }
    void stopHosting() override { m_s->retireHost(m_p); }
    void followHost(const QString &hostDeviceId, qint64 epoch) override { m_s->followNewHost(m_p, hostDeviceId, epoch); }
    void resyncFromHost() override
    {
        if (m_p->client && !m_p->client->isLoopback())
            m_p->client->resync();
    }
    void setAcceptingLocks(bool on) override
    {
        if (m_p->host)
            m_p->host->setAcceptingLocks(on);
    }
    void setAcceptingCommits(bool on) override
    {
        if (m_p->host)
            m_p->host->setAcceptingCommits(on);
    }
    bool drained() const override { return !m_p->client || !m_p->client->hasPending(); }
    QJsonArray lockTable() const override { return m_p->host ? m_p->host->locks().toJson() : QJsonArray(); }
    bool busy() const override { return m_p->auth && m_p->auth->handoverRunning(); }
    QList<AuthoredCommit> authoredCommits(qint64 afterSeq, qint64 beforeEpoch) override
    {
        return m_p->ledger ? m_p->ledger->after(afterSeq, beforeEpoch) : QList<AuthoredCommit>();
    }
    int outboxToDrafts(const QString &reason) override { return m_p->client ? m_p->client->pendingToDrafts(reason) : 0; }
    void reunionDone(qint64 afterSeq, qint64 newEpoch) override
    {
        if (m_p->ledger)
            m_p->ledger->forget(afterSeq, newEpoch);
    }

private:
    Service *m_s;
    Service::Proj *m_p;
    bool m_failover = false;
};

Service::Proj *Service::openProject(const RegistryEntry &entry)
{
    if (m_projects.contains(entry.uuid))
        return nullptr;
    auto *p = new Proj;
    p->entry = entry;
    m_projects.insert(entry.uuid, p);
    QDir().mkpath(entry.folder + QStringLiteral("/livesync-local"));
    p->hb = std::make_unique<ServiceHosting>(this, p);
    p->ledger = std::make_unique<CommitLedger>(entry.folder + QStringLiteral("/livesync-local/ledger.json"));
    p->ledger->load();
    makeAuthority(p);
    return p;
}

void Service::failHost(Proj *p, const QString &why)
{
    p->problem = why;
    emit notice(tr("Hosting \"%1\" could not start: %2").arg(p->entry.title, why));
    emit projectChanged(p->entry.uuid);
}

Membership Service::membershipFor(const RegistryEntry &e) const
{
    Membership m;
    m.project = e.uuid;
    auto add = [&](const QString &id, const QByteArray &key, const QString &name, bool editor) {
        Member mb;
        mb.id = id;
        mb.key = key;
        mb.name = name;
        mb.editor = editor;
        mb.mayHost = Membership::defaultMayHost(editor, false);
        m.members << mb;
    };
    add(m_id->deviceId(), m_id->publicKeyDer(), m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName, true);
    for (const net::PeerRecord &r : m_peers->peers()) {
        const QString role = e.access.value(r.deviceId()).toString();
        if (role.isEmpty() || r.publicKey.isEmpty())
            continue; // (a device paired before the keys were kept cannot be a member: pair it again)
        add(r.deviceId(), r.publicKey, r.name, role == QLatin1String("editor"));
    }
    m.version = 1;
    m.signWith(*m_id);
    return m;
}

void Service::publishAccess(Proj *p)
{
    if (!p->auth || !p->auth->isHost())
        return;
    Membership m = membershipFor(p->entry);
    m.version = p->auth->membership().version; // (publishMembership counts up)
    // keep the order of priority already agreed: known members first, new ones after
    QList<Member> list;
    for (const Member &old : p->auth->membership().members) {
        bool found = false;
        for (const Member &mb : m.members)
            if (mb.id == old.id) {
                list << mb;
                found = true;
            }
        if (!found && old.id != m_id->deviceId() && !m_peers->contains(old.pin()))
            list << old; // a member this device never paired with (it came with a failover): only a device that is paired here can be removed here
    }
    for (const Member &mb : m.members) {
        bool known = false;
        for (const Member &o : list)
            known = known || o.id == mb.id;
        if (!known)
            list << mb;
    }
    if (list.size() == p->auth->membership().members.size()) {
        bool same = true;
        for (int i = 0; i < list.size(); ++i)
            same = same && list.at(i).id == p->auth->membership().members.at(i).id && list.at(i).editor == p->auth->membership().members.at(i).editor &&
                   list.at(i).name == p->auth->membership().members.at(i).name;
        if (same)
            return;
    }
    QString why;
    if (!p->auth->publishMembership(list, &why))
        emit notice(tr("The list of devices of \"%1\" could not be saved: %2").arg(p->entry.title, why));
    refreshHostInfo(p);
}

void Service::refreshHostInfo(Proj *p)
{
    if (!p->host || !p->auth)
        return;
    p->host->setMembership(p->auth->membership().toJson());
    QJsonArray chain;
    for (const Proof &pr : p->auth->chain())
        chain.append(pr.toJson());
    p->host->setProofs(chain);
}

void Service::makeAuthority(Proj *p)
{
    const QString uuid = p->entry.uuid;
    Authority::Config ac;
    ac.project = uuid;
    ac.title = p->entry.title;
    ac.folder = p->entry.folder;
    ac.localRoot = p->entry.folder + QStringLiteral("/livesync-local");
    ac.identity = m_id;
    ac.clock = m_scopedClock.get();
    ac.backend = p->hb.get();
    ac.send = [this, uuid](const QString &device, const QJsonObject &m) { return authSend(uuid, device, m); };
    p->auth = std::make_unique<Authority>(ac);
    QString why;
    if (!p->auth->load(&why))
        p->problem = why;
    Authority *a = p->auth.get();
    if (QB_TEST_ENV_SET("QB_LS_TRACE")) {
        connect(a, &Authority::electionStarted, this, [this](qint64 e) { std::fprintf(stderr, "[%s] election started %lld\n", qPrintable(m_id->deviceId().left(4)), (long long)e); });
        connect(a, &Authority::electionFinished, this, [this](bool w, qint64 e) { std::fprintf(stderr, "[%s] election finished won=%d %lld\n", qPrintable(m_id->deviceId().left(4)), w, (long long)e); });
        connect(a, &Authority::hostChanged, this, [this](const QString &h, qint64 e) { std::fprintf(stderr, "[%s] host now %s %lld\n", qPrintable(m_id->deviceId().left(4)), qPrintable(h.left(4)), (long long)e); });
    }
    connect(a, &Authority::unknownDeviceRefused, this, [this, uuid](const QString &, const QString &) {
        Proj *q = m_projects.value(uuid);
        if (!q)
            return;
        const QString w = q->auth->unknownDeviceWarning();
        if (!q->notices.contains(w))
            q->notices << w;
        while (q->notices.size() > 3)
            q->notices.removeFirst();
        emit notice(w);
        emit unknownHostRefused(q->entry.title);
        emit projectChanged(uuid);
    });
    connect(a, &Authority::hostChanged, this, [this, uuid] {
        if (Proj *q = m_projects.value(uuid)) {
            refreshHostInfo(q);
            emit projectChanged(uuid);
        }
    });
    connect(a, &Authority::membershipChanged, this, [this, uuid] {
        if (Proj *q = m_projects.value(uuid))
            refreshHostInfo(q);
    });
    connect(a, &Authority::stateChanged, this, [this, uuid] { emit projectChanged(uuid); });
    connect(a, &Authority::handoverFinished, this, [this, uuid](bool ok, const QString &why) {
        if (!ok && !why.isEmpty())
            emit notice(tr("Hosting was not handed over: %1").arg(why));
        emit projectChanged(uuid);
    });
    connect(a, &Authority::failed, this, [this, uuid](const QString &what) {
        if (Proj *q = m_projects.value(uuid)) {
            q->problem = what;
            emit projectChanged(uuid);
        }
    });
}

bool Service::activateHost(Proj *p, qint64 epoch, QString *why, const QJsonArray &locks, bool afterFailover)
{
    QString w;
    const bool ok = beginHost(p, epoch, &w);
    if (!ok) {
        if (why)
            *why = w;
        failHost(p, w);
        return false;
    }
    if (p->host) {
        ServiceScope scope;
        if (!locks.isEmpty())
            p->host->adoptLocks(locks);
        else if (afterFailover)
            p->host->setLockGrace(kLockGraceMs);
    }
    // a replica that takes over knows who may join from the signed list of devices
    if (p->auth) {
        for (const Member &mb : p->auth->membership().members)
            if (mb.id != m_id->deviceId() && !p->entry.access.contains(mb.id))
                p->entry.access.insert(mb.id, mb.editor ? QStringLiteral("editor") : QStringLiteral("reader"));
    }
    p->entry.host = true;
    p->entry.hostDeviceId = m_id->deviceId();
    p->entry.hostName = m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName;
    p->entry.epoch = epoch;
    p->retired = false;
    m_registry->upsert(p->entry);
    emit projectsChanged();
    return true;
}

// a hand-over's old host: no more commits; it stays connected until it follows the new host (the last messages travel on it)
void Service::retireHost(Proj *p)
{
    p->retired = true;
    if (p->host) {
        p->host->setAcceptingCommits(false);
        p->host->setAcceptingLocks(false);
    }
    emit projectChanged(p->entry.uuid);
}

void Service::followNewHost(Proj *p, const QString &hostDeviceId, qint64 epoch)
{
    // (called from inside a message of the authority or a session: the sessions are changed one turn of the event loop later)
    QTimer::singleShot(0, this, [this, uuid = p->entry.uuid, hostDeviceId, epoch] {
        Proj *p = m_projects.value(uuid);
        if (!p || !m_running)
            return;
        const bool wasHost = p->host != nullptr;
        const bool changed = hostDeviceId != p->entry.hostDeviceId;
        qint64 seq = 0;
        QHash<QString, qint64> revs;
        if (wasHost) {
            seq = p->host->seq();
            revs = p->host->revs();
            p->jobs.reset();
            p->host->stop();
            p->host.reset();
            p->backend.reset();
            m_registry->releaseHost(uuid);
            p->retired = false;
            if (p->client) {
                p->client->detach();
                p->client.reset();
            }
            p->scope.reset();
            p->online.clear();
        }
        p->entry.host = false;
        p->entry.hostDeviceId = hostDeviceId;
        p->entry.epoch = epoch;
        for (const net::PeerRecord &r : m_peers->peers())
            if (r.deviceId() == hostDeviceId)
                p->entry.hostName = r.name;
        if (p->auth)
            if (const Member *mb = p->auth->membership().find(hostDeviceId); mb && p->entry.hostName.isEmpty())
                p->entry.hostName = mb->name; // (a host this device never paired with: its name comes from the signed list)
        if (wasHost || changed) {
            if (m_peerAddr.contains(hostDeviceId)) {
                p->entry.lastAddress = m_peerAddr.value(hostDeviceId).first;
                p->entry.lastPort = m_peerAddr.value(hostDeviceId).second;
            } else {
                p->entry.lastAddress.clear();
            }
        }
        m_registry->upsert(p->entry);
        if (wasHost) {
            if (p->link)
                p->link->close();
            writeReplicaState(p->entry.folder, uuid, epoch, seq, revs);
            p->tries = 0;
            startClientPart(p);
        } else if (changed && p->client) {
            QPointer<TlsLink> old = p->link;
            p->link = nullptr;
            p->client->detach();
            if (old)
                old->close();
            p->online.clear();
            p->tries = 0;
            connectClient(p);
        }
        emit projectsChanged();
        emit projectChanged(uuid);
    });
}

bool Service::authSend(const QString &uuid, const QString &deviceId, const QJsonObject &m)
{
    Proj *p = m_projects.value(uuid);
    if (!p)
        return false;
    if (p->host && p->host->sendToDevice(deviceId, m))
        return true;
    if (p->rawLink && p->rawPeer == deviceId && p->rawLink->state() == Link::State::Ready) {
        p->rawLink->send(Frame::json(m));
        return true;
    }
    if (p->client && !p->client->isLoopback() && deviceId == p->entry.hostDeviceId && p->client->state() == ClientSession::State::Ready)
        return p->client->sendMessage(m);
    // no main connection: the direct link between the two members (votes and proofs travel here when the host is lost)
    const auto it = p->mesh.constFind(deviceId);
    if (it != p->mesh.constEnd() && it->link && it->link->state() == Link::State::Ready)
        return it->link->send(Frame::json(m));
    if (p->client && !p->client->isLoopback() && deviceId == p->entry.hostDeviceId)
        return p->client->sendMessage(m);
    return false;
}

void Service::setPeerAddress(const QString &deviceId, const QString &host, quint16 port) { m_peerAddr.insert(deviceId, qMakePair(host, port)); }

Authority *Service::authority(const QString &uuid) const
{
    const Proj *p = m_projects.value(uuid);
    return p ? p->auth.get() : nullptr;
}

bool Service::transferHosting(const QString &uuid, const QString &targetDeviceId, QString *why)
{
    Proj *p = m_projects.value(uuid);
    if (!p || !p->auth)
        return false;
    return p->auth->transferHosting(targetDeviceId, why);
}

// ---------------------------------------------------------------- hosted projects

void Service::startHost(const RegistryEntry &entry)
{
    Proj *p = openProject(entry);
    if (!p)
        return;
    if (!p->problem.isEmpty()) {
        failHost(p, p->problem); // (a damaged authority file is never overwritten)
        return;
    }
    QString why;
    if (!m_registry->lockHost(entry.uuid, &why)) {
        failHost(p, why);
        return;
    }
    m_registry->releaseHost(entry.uuid); // (beginHost takes it again)
    if (p->auth->record().chain.isEmpty()) {
        // a project shared before the authority existed: this device founds it now
        if (!p->auth->found(membershipFor(entry), qMax<qint64>(1, entry.epoch), &why) && p->problem.isEmpty())
            failHost(p, why);
        return;
    }
    p->auth->start();
    if (p->auth->isHost()) {
        if (!activateHost(p, p->auth->epoch(), &why))
            return;
        refreshHostInfo(p);
    } else {
        p->entry.host = false;
        p->entry.hostDeviceId = p->auth->host();
        m_registry->upsert(p->entry);
        startClientPart(p);
    }
}

bool Service::beginHost(Proj *p, qint64 epoch, QString *whyOut)
{
    const RegistryEntry &entry = p->entry;
    QString why;
    auto fail = [&](const QString &w) {
        if (whyOut)
            *whyOut = w;
        return false;
    };
    if (!m_registry->lockHost(entry.uuid, &why))
        return fail(why);
    p->scope = std::make_unique<vault::Scope>(entry.folder);
    // a replica that becomes the host: its state is the host's state from now on
    if (p->client && !p->client->isLoopback()) {
        if (!writeHostState(entry.folder, entry.uuid, epoch, p->client->seq(), p->client->revs())) {
            m_registry->releaseHost(entry.uuid);
            return fail(QStringLiteral("the state of Live Sync cannot be written"));
        }
        p->retry.stop();
        QFile::remove(entry.folder + QStringLiteral("/livesync-local/loopback.json")); // (the loopback copy starts again from the host's state)
        p->rawPeer = p->entry.hostDeviceId;
        p->rawLink = p->link; // (kept: the old host's DONE message comes on it)
        p->client->detach();
        p->client.reset();
        if (p->rawLink) {
            Link *raw = p->rawLink;
            connect(raw, &Link::frameReceived, this, [this, uuid = entry.uuid](const Frame &f) {
                Proj *q = m_projects.value(uuid);
                bool ok = false;
                const QJsonObject m = f.kind == Frame::Json ? f.object(&ok) : QJsonObject();
                if (q && q->auth && ok)
                    q->auth->handleMessage(q->rawPeer, m);
            });
            connect(raw, &Link::closed, this, [this, uuid = entry.uuid](const QString &) {
                if (Proj *q = m_projects.value(uuid))
                    q->rawLink = nullptr;
            });
        }
        p->link = nullptr;
    }
    HostSession::Options o;
    o.folder = entry.folder;
    o.uuid = entry.uuid;
    o.hostName = m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName;
    o.clock = m_scopedClock.get();
    o.epoch = qMax<qint64>(1, epoch);
    p->host = std::make_unique<HostSession>(o);
    {
        ServiceScope scope;
        if (!p->host->start(&why)) {
            p->host.reset();
            m_registry->releaseHost(entry.uuid);
            return fail(why);
        }
        p->backend = std::make_unique<ProjectHostBackend>(entry.folder);
        if (!p->backend->open(&why)) {
            p->backend.reset();
            p->host.reset();
            m_registry->releaseHost(entry.uuid);
            return fail(why);
        }
        p->host->setBackend(p->backend.get());
    }
    p->host->setEpoch(qMax<qint64>(1, epoch));
    const QString uuid = entry.uuid;
    p->host->setRoleProvider([this, uuid](const QString &device) {
        if (device == m_id->deviceId())
            return Role::Editor;
        RegistryEntry e;
        if (!m_registry->find(uuid, &e))
            return Role::None;
        QString r = e.access.value(device).toString();
        if (r.isEmpty()) { // a device that joins a host it never paired with: the signed list of devices says what it may do
            if (Proj *q = m_projects.value(uuid); q && q->auth)
                if (const Member *mb = q->auth->membership().find(device))
                    r = mb->editor ? QStringLiteral("editor") : QStringLiteral("reader");
        }
        return r == QLatin1String("editor") ? Role::Editor : r == QLatin1String("reader") ? Role::Reader : Role::None;
    });
    p->host->setExtensionHandler([this, uuid](const QString &device, Link *link, const QJsonObject &m) {
        Proj *q = m_projects.value(uuid);
        if (!q)
            return false;
        if (m.value(QStringLiteral("t")).toString() == QLatin1String(proto::T::PRESENCE)) {
            onPresence(q, device, m, link ? link->peer().address : QString());
            return true;
        }
        if (q->auth && q->auth->handleMessage(device, m))
            return true;
        return q->jobs && q->jobs->handle(device, link, m); // BACKUP_REQUEST, SNAPSHOT_OP
    });
    {
        HostBackup::Config jc;
        jc.host = p->host.get();
        ProjectHostBackend *backend = p->backend.get();
        jc.projectOf = [backend] { return backend->project(); };
        jc.backupDirOf = m_cfg.backupDir;
        jc.keepOf = m_cfg.backupKeep;
        p->jobs = std::make_unique<HostBackup>(jc);
        connect(p->jobs.get(), &HostBackup::statusChanged, this, [this, uuid] {
            scheduleState(uuid);
            emit projectChanged(uuid);
        });
        p->host->setSnapshotHook(p->jobs->protectiveSnapshotHook());
        p->jobs->removeOldFreezes();
        QString jobErr;
        if (!p->jobs->recoverJobs(&jobErr)) // a job that was cut off between its files and its commit is published now
            p->notices << tr("A snapshot job that was cut off could not be finished: %1").arg(jobErr);
    }
    connect(p->host.get(), &HostSession::peerJoined, this, [this, uuid](const QString &device, const QString &name) {
        if (Proj *q = m_projects.value(uuid)) {
            q->online.insert(device);
            q->peerNames.insert(device, name);
            if (q->auth && device != m_id->deviceId()) {
                q->auth->peerConnected(device);
                q->auth->setPeerInSync(device, true); // (helper's choice: a joined device is taken as in step; it catches up at once)
            }
        }
        emit projectChanged(uuid);
    });
    connect(p->host.get(), &HostSession::peerLeft, this, [this, uuid](const QString &device) {
        if (Proj *q = m_projects.value(uuid)) {
            bool still = false;
            if (q->host)
                for (const HostSession::PeerSummary &s : q->host->joinedPeers())
                    still = still || s.deviceId == device;
            if (!still) {
                q->online.remove(device);
                if (q->auth && device != m_id->deviceId()) {
                    q->auth->setPeerInSync(device, false);
                    if (!meshReady(q, device))
                        q->auth->peerDisconnected(device);
                }
            }
        }
        emit projectChanged(uuid);
    });
    connect(p->host.get(), &HostSession::locksChanged, this, [this, uuid] {
        emit locksChanged(uuid);
        scheduleState(uuid);
    });
    connect(p->host.get(), &HostSession::committed, this, [this, uuid] { emit projectChanged(uuid); });
    refreshHostInfo(p);
    // the loopback session: this device's windows are clients of its own host
    ClientSession::Config cc;
    cc.folder = entry.folder;
    cc.uuid = entry.uuid;
    cc.deviceId = m_id->deviceId();
    cc.deviceName = o.hostName;
    cc.clock = m_scopedClock.get();
    cc.loopback = true;
    p->client = std::make_unique<ClientSession>(cc);
    if (!p->client->open(&why)) {
        p->client.reset();
        p->jobs.reset();
        p->host->stop();
        p->host.reset();
        m_registry->releaseHost(entry.uuid);
        return fail(why);
    }
    const QByteArray hostPin = QCryptographicHash::hash(QByteArray("loopback-host") + entry.uuid.toUtf8(), QCryptographicHash::Sha256);
    MemLink::Pair pair = MemLink::createPair(hostPin, m_id->pin(), p->host.get());
    p->loop = pair.a;
    p->host->addLink(ScopedLink::wrap(pair.a));
    wireClient(p);
    p->client->attach(ScopedLink::wrap(pair.b));
    emit projectsChanged();
    return true;
}

void Service::stopProject(const QString &uuid, bool removeEntry)
{
    Proj *p = m_projects.take(uuid);
    if (!p)
        return;
    p->retry.stop();
    p->auth.reset();
    if (p->client) {
        p->client->detach();
        p->client.reset();
    }
    if (p->rawLink)
        p->rawLink->close();
    p->jobsClient.reset();
    p->jobs.reset();
    if (p->host) {
        p->host->stop();
        p->host.reset();
        m_registry->releaseHost(uuid);
    }
    p->backend.reset();
    p->scope.reset();
    if (p->link)
        p->link->close();
    delete p;
    if (removeEntry && m_registry)
        m_registry->remove(uuid);
    emit projectsChanged();
}

QString Service::shareProject(const QString &folder, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error)
            *error = why;
        return QString();
    };
    if (!m_running)
        return fail(tr("Live Sync is not running."));
    const QString file = Project::resolveProjectFile(folder);
    if (file.isEmpty() || !file.endsWith(QLatin1String(".qbx")))
        return fail(tr("Only projects in Quillbinder's own format can be shared."));
    const QString dir = QFileInfo(file).absolutePath();
    RegistryEntry known;
    if (m_registry->findByFolder(dir, &known))
        return known.uuid;
    RegistryEntry e;
    e.uuid = Registry::newUuid();
    e.host = true;
    e.folder = dir;
    e.title = QFileInfo(file).completeBaseName();
    e.hostDeviceId = m_id->deviceId();
    e.hostName = m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName;
    e.epoch = 1;
    QString why;
    if (!m_registry->upsert(e, &why))
        return fail(why);
    startHost(e);
    Proj *p = m_projects.value(e.uuid);
    if (!p || !p->host) {
        const QString reason = p ? p->problem : QStringLiteral("unknown");
        stopProject(e.uuid, true);
        removeAuthorityFiles(dir);
        return fail(reason);
    }
    return e.uuid;
}

bool Service::stopSharing(const QString &uuid, QString *error)
{
    Q_UNUSED(error);
    RegistryEntry e;
    if (!m_registry || !m_registry->find(uuid, &e))
        return false;
    const bool host = e.host;
    stopProject(uuid, false);
    if (host)
        removeAuthorityFiles(e.folder);
    // the entry stays, marked, so that the folder is never taken for a copy of a project that is shared elsewhere
    e.stopped = true;
    if (host)
        m_registry->remove(uuid);
    else
        m_registry->upsert(e);
    return true;
}

void Service::setAccess(const QString &uuid, const QString &deviceId, Role role)
{
    RegistryEntry e;
    if (!m_registry->find(uuid, &e))
        return;
    if (role == Role::None)
        e.access.remove(deviceId);
    else
        e.access.insert(deviceId, role == Role::Editor ? QStringLiteral("editor") : QStringLiteral("reader"));
    m_registry->upsert(e);
    if (Proj *p = m_projects.value(uuid)) {
        p->entry = e;
        publishAccess(p);
    }
    emit projectChanged(uuid);
}

// ---------------------------------------------------------------- joined projects

QString Service::replicaFolderFor(const QString &uuid, const QString &title) const
{
    QString name = title;
    name.replace(QRegularExpression(QStringLiteral("[^\\w .-]")), QStringLiteral("_"));
    if (name.isEmpty())
        name = QStringLiteral("project");
    return m_cfg.replicasDir + QLatin1Char('/') + uuid + QLatin1Char('/') + name + QStringLiteral(".quillbinder");
}

QString Service::joinProject(const QString &hostDeviceId, const QString &uuid, const QString &title, const QString &address, quint16 port,
                             QString *error)
{
    auto fail = [&](const QString &why) {
        if (error)
            *error = why;
        return QString();
    };
    if (!m_running)
        return fail(tr("Live Sync is not running."));
    RegistryEntry known;
    if (m_registry->find(uuid, &known) && !known.stopped)
        return known.folder;
    bool paired = false;
    net::PeerRecord rec;
    for (const net::PeerRecord &r : m_peers->peers())
        if (r.deviceId() == hostDeviceId) {
            paired = true;
            rec = r;
        }
    if (!paired)
        return fail(tr("This device is not paired with the host."));
    RegistryEntry e;
    e.uuid = uuid;
    e.host = false;
    e.folder = replicaFolderFor(uuid, title);
    e.title = title;
    e.hostDeviceId = hostDeviceId;
    e.hostName = rec.name;
    e.lastAddress = address;
    e.lastPort = port;
    QString why;
    if (!QDir().mkpath(e.folder))
        return fail(tr("The folder for the copy could not be made."));
    if (!m_registry->upsert(e, &why))
        return fail(why);
    startClient(e);
    return e.folder;
}

void Service::startClient(const RegistryEntry &entry)
{
    Proj *p = openProject(entry);
    if (!p)
        return;
    if (!p->problem.isEmpty()) {
        emit notice(tr("The copy of \"%1\" cannot be used: %2").arg(entry.title, p->problem));
        return;
    }
    if (!p->auth->record().chain.isEmpty()) {
        p->auth->start();
        QString why;
        if (p->auth->isHost()) {
            // this replica became the host (a failover or a hand-over) and the service ended before the registry said so
            p->client.reset();
            if (!activateHost(p, p->auth->epoch(), &why))
                return;
            refreshHostInfo(p);
            return;
        }
        if (!p->auth->host().isEmpty() && p->auth->host() != p->entry.hostDeviceId) {
            p->entry.hostDeviceId = p->auth->host();
            m_registry->upsert(p->entry);
        }
    }
    startClientPart(p);
}

void Service::startClientPart(Proj *p)
{
    const RegistryEntry &entry = p->entry;
    ClientSession::Config cc;
    cc.folder = entry.folder;
    cc.uuid = entry.uuid;
    cc.deviceId = m_id->deviceId();
    cc.deviceName = m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName;
    cc.clock = m_scopedClock.get();
    p->client = std::make_unique<ClientSession>(cc);
    QString why;
    if (!p->client->open(&why)) {
        p->problem = why;
        p->client.reset();
        emit notice(tr("The copy of \"%1\" cannot be used: %2").arg(entry.title, why));
        return;
    }
    wireClient(p);
    p->retry.setSingleShot(true);
    disconnect(&p->retry, nullptr, this, nullptr);
    connect(&p->retry, &QTimer::timeout, this, [this, uuid = entry.uuid] {
        if (Proj *q = m_projects.value(uuid))
            connectClient(q);
    });
    connectClient(p);
    emit projectsChanged();
}

void Service::wireClient(Proj *p)
{
    const QString uuid = p->entry.uuid;
    ClientSession *c = p->client.get();
    connect(c, &ClientSession::stateChanged, this, [this, uuid](ClientSession::State st) {
        if (Proj *q = m_projects.value(uuid); q && q->auth && q->client && !q->client->isLoopback() && !q->entry.hostDeviceId.isEmpty()) {
            const QString h = q->entry.hostDeviceId;
            if (st == ClientSession::State::Ready && !q->online.contains(h)) {
                q->online.insert(h);
                q->auth->peerConnected(h);
            } else if (st != ClientSession::State::Ready && q->online.contains(h)) {
                q->online.remove(h);
                if (!meshReady(q, h))
                    q->auth->peerDisconnected(h);
            }
        }
        scheduleState(uuid);
        emit projectChanged(uuid);
    });
    if (!c->isLoopback()) {
        c->setExtensionHandler([this, uuid](const QJsonObject &m) {
            Proj *q = m_projects.value(uuid);
            if (!q)
                return false;
            if (m.value(QStringLiteral("t")).toString() == QLatin1String(proto::T::PRESENCE)) {
                onPresence(q, q->entry.hostDeviceId, m, QString());
                return true;
            }
            if (q->auth && q->auth->handleMessage(q->entry.hostDeviceId, m))
                return true;
            return q->jobsClient && q->jobsClient->handle(m); // BACKUP_STATUS, SNAPSHOT_RESULT
        });
        p->jobsClient = std::make_unique<HostJobsClient>(c, uuid);
        connect(p->jobsClient.get(), &HostJobsClient::statusChanged, this, [this, uuid] {
            scheduleState(uuid);
            emit projectChanged(uuid);
        });
        connect(c, &ClientSession::joinedInfo, this, [this, uuid](const QJsonObject &membership, const QJsonArray &proofs) {
            Proj *q = m_projects.value(uuid);
            if (!q || !q->auth || !q->auth->record().chain.isEmpty() || membership.isEmpty() || proofs.isEmpty())
                return;
            Membership m;
            QString why;
            QList<Proof> chain;
            bool ok = Membership::fromJson(membership, &m, &why);
            for (const QJsonValue &v : proofs) {
                Proof pr;
                ok = ok && Proof::fromJson(v.toObject(), &pr, &why);
                chain << pr;
            }
            if (ok && q->auth->joinWith(chain, m, &why)) {
                q->auth->start();
            } else {
                q->problem = tr("The host's list of devices cannot be used: %1").arg(why);
                emit projectChanged(uuid);
            }
        });
    } else {
        p->jobsClient.reset();
    }
    connect(c, &ClientSession::readyToUse, this, [this, uuid] {
        if (Proj *q = m_projects.value(uuid); q && q->jobsClient)
            q->jobsClient->queryStatus(); // the host's backup state, for the inspector tab and the tooltip
        scheduleState(uuid);
    });
    connect(c, &ClientSession::eventApplied, this,
            [this, uuid](qint64 seq, const QString &opId, const QString &actor, bool own, const QJsonArray &ops, const QStringList &paths, const QJsonObject &revs) {
                if (own) {
                    if (Proj *q = m_projects.value(uuid); q && q->ledger && q->client) {
                        AuthoredCommit ac;
                        ac.seq = seq;
                        ac.epoch = q->client->epoch();
                        ac.paths = paths;
                        ac.ops = ops;
                        q->ledger->add(ac);
                    }
                }
                tellWindows(uuid, local::msg(local::EVENT, {{"uuid", uuid}, {"seq", double(seq)}, {"opId", opId}, {"actor", actor}, {"own", own},
                                                              {"ops", ops}, {"paths", QJsonArray::fromStringList(paths)}, {"revs", revs}}));
                scheduleState(uuid);
            });
    connect(c, &ClientSession::replicaReplaced, this, [this, uuid] {
        tellWindows(uuid, local::msg(local::REPLACED, {{"uuid", uuid}}));
        scheduleState(uuid);
    });
    connect(c, &ClientSession::committed, this, [this, uuid](const QString &opId, qint64 seq) {
        tellWindows(uuid, local::msg(local::RESULT, {{"uuid", uuid}, {"opId", opId}, {"kind", "committed"}, {"seq", double(seq)}}));
        scheduleState(uuid);
    });
    connect(c, &ClientSession::rejected, this, [this, uuid](const QString &opId, const QJsonArray &rejects) {
        tellWindows(uuid, local::msg(local::RESULT, {{"uuid", uuid}, {"opId", opId}, {"kind", "rejected"}, {"rejects", rejects}}));
        scheduleState(uuid);
    });
    connect(c, &ClientSession::draftKept, this, [this, uuid](const QString &opId, const QString &reason) {
        tellWindows(uuid, local::msg(local::RESULT, {{"uuid", uuid}, {"opId", opId}, {"kind", "draft"}, {"reason", reason}}));
        scheduleState(uuid);
    });
    connect(c, &ClientSession::problem, this, [this, uuid](const QString &opId, const QString &code) {
        tellWindows(uuid, local::msg(local::RESULT, {{"uuid", uuid}, {"opId", opId}, {"kind", "problem"}, {"code", code}}));
        scheduleState(uuid);
    });
    connect(c, &ClientSession::lockGranted, this, [this, uuid](const QString &resource) {
        m_asks.remove(uuid + QLatin1Char('|') + resource);
        tellWindows(uuid, local::msg(local::LOCK_EVT, {{"uuid", uuid}, {"kind", "granted"}, {"resource", resource}}));
        scheduleState(uuid);
        emit locksChanged(uuid);
    });
    connect(c, &ClientSession::lockDenied, this, [this, uuid](const QString &resource, const QString &holder, const QString &reason) {
        tellWindows(uuid, local::msg(local::LOCK_EVT, {{"uuid", uuid}, {"kind", "denied"}, {"resource", resource}, {"holder", holder}, {"reason", reason}}));
        scheduleState(uuid);
    });
    connect(c, &ClientSession::lockRevoked, this, [this, uuid](const QString &resource, const QString &reason) {
        emit editAskEnded(uuid, resource); // (a request for a lock this device lost is over)
        tellWindows(uuid, local::msg(local::LOCK_EVT, {{"uuid", uuid}, {"kind", "revoked"}, {"resource", resource}, {"reason", reason}}));
        scheduleState(uuid);
        emit locksChanged(uuid);
    });
    connect(c, &ClientSession::locksChanged, this, [this, uuid] {
        scheduleState(uuid);
        emit locksChanged(uuid);
    });
    connect(c, &ClientSession::editAsked, this, [this, uuid](const QString &resource, const QString &asker, qint64 waitMs) {
        emit editAsked(uuid, resource, asker);
        tellWindows(uuid, local::msg(local::EDIT_EVT, {{"uuid", uuid}, {"kind", "asked"}, {"resource", resource}, {"asker", asker}, {"waitMs", double(waitMs)}}));
    });
    connect(c, &ClientSession::editEnded, this, [this, uuid](const QString &resource, const QString &) { emit editAskEnded(uuid, resource); });
    connect(c, &ClientSession::editStatus, this, [this, uuid](const QString &resource, bool declined, qint64 waitMs, bool mayForce, const QString &holder) {
        const QString key = uuid + QLatin1Char('|') + resource;
        if (declined)
            m_declined.insert(key);
        tellWindows(uuid, local::msg(local::EDIT_EVT, {{"uuid", uuid}, {"kind", "status"}, {"resource", resource}, {"declined", declined},
                                                         {"waitMs", double(waitMs)}, {"mayForce", mayForce}, {"holder", holder}}));
        emit locksChanged(uuid);
    });
}

void Service::scheduleReconnect(Proj *p)
{
    if (!m_running || !p->client)
        return;
    const int delay = qMin(10000, 1000 << qMin(p->tries, 4));
    ++p->tries;
    p->retry.start(delay);
}

void Service::connectClient(Proj *p)
{
    if (!m_running || p->entry.host || !p->client || (p->link && p->link->state() != Link::State::Closed))
        return;
    net::PeerRecord rec;
    bool found = false;
    for (const net::PeerRecord &r : m_peers->peers())
        if (r.deviceId() == p->entry.hostDeviceId) {
            rec = r;
            found = true;
        }
    if (!found && p->auth) {
        // a host this device never paired with (it took over after a failover): its key is in the signed list of devices
        if (const Member *mb = p->auth->membership().find(p->entry.hostDeviceId)) {
            rec.pin = mb->pin();
            rec.publicKey = mb->key;
            rec.name = mb->name;
            found = true;
        }
    }
    if (!found) {
        p->problem = tr("The host is not paired with this device any more.");
        emit projectChanged(p->entry.uuid);
        return;
    }
    if (blocked(p->entry.hostDeviceId)) {
        scheduleReconnect(p);
        return;
    }
    p->problem.clear();
    // where to knock: the last address, then what discovery knows of the host
    QList<std::pair<QString, quint16>> where;
    if (!p->entry.lastAddress.isEmpty())
        where << std::make_pair(p->entry.lastAddress, quint16(p->entry.lastPort ? p->entry.lastPort : net::TlsServer::kPreferredPort));
    if (m_discovery)
        for (const mdns::PeerInfo &pi : m_discovery->peers())
            if (pi.id == rec.instanceId)
                for (const QHostAddress &a : pi.addresses)
                    where << std::make_pair(a.toString(), pi.port);
    for (const auto &a : m_addrs.value(rec.deviceId())) // (what the signed presence messages of the host named)
        where << std::make_pair(a.first, a.second);
    if (m_peerAddr.contains(rec.deviceId()) && p->entry.lastAddress.isEmpty())
        where << std::make_pair(m_peerAddr.value(rec.deviceId()).first, m_peerAddr.value(rec.deviceId()).second);
    if (where.isEmpty()) {
        scheduleReconnect(p);
        return;
    }
    const auto target = where.at(p->addressTurn++ % where.size());
    TlsLink *link = TlsLink::connectTo(target.first, target.second, m_id, TrustPolicy::pinned(rec.pin), this);
    p->link = link;
    const QString uuid = p->entry.uuid;
    connect(link, &Link::stateChanged, this, [this, uuid, link, target](Link::State st) {
        Proj *q = m_projects.value(uuid);
        if (!q || q->link != link)
            return;
        if (st == Link::State::Ready) {
            q->tries = 0;
            q->entry.lastAddress = target.first;
            q->entry.lastPort = target.second;
            m_peerAddr.insert(q->entry.hostDeviceId, qMakePair(target.first, target.second));
            m_registry->upsert(q->entry);
            q->client->attach(ScopedLink::wrap(link));
        }
    });
    connect(link, &Link::closed, this, [this, uuid, link](const QString &) {
        Proj *q = m_projects.value(uuid);
        if (!q || q->link != link)
            return;
        q->client->detach();
        scheduleState(uuid);
        emit projectChanged(uuid);
        scheduleReconnect(q);
        link->deleteLater();
    });
}

// ---------------------------------------------------------------- direct links between the members of a project (package 10)

namespace {
constexpr qint64 kPresenceMs = 3000;      // every device tells its addresses and name this often
constexpr qint64 kMeshDeadMs = 10000;     // a direct link that was silent this long is closed
constexpr qint64 kMeshBackoffMaxMs = 15000;

QString cleanName(const QString &n)
{
    QString out;
    for (const QChar c : n.left(100))
        if (!c.isNonCharacter() && c.category() != QChar::Other_Control)
            out.append(c);
    return out.trimmed();
}

// "::ffff:127.0.0.1" and "127.0.0.1" both read 127.0.0.1
QString plainAddress(const QString &a)
{
    QHostAddress h(a);
    bool ok = false;
    const quint32 v4 = h.toIPv4Address(&ok);
    return ok ? QHostAddress(v4).toString() : QString();
}

QByteArray presenceBytes(const QString &project, const QJsonObject &rec)
{
    QStringList addrs;
    for (const QJsonValue &v : rec.value(QStringLiteral("addrs")).toArray())
        addrs << v.toString();
    return QByteArray("QB-LS-PRESENCE-1\n") + project.toUtf8() + '\n' + rec.value(QStringLiteral("device")).toString().toUtf8() + '\n' +
           rec.value(QStringLiteral("name")).toString().toUtf8() + '\n' + QByteArray::number(qint64(rec.value(QStringLiteral("port")).toDouble())) + '\n' +
           addrs.join(QLatin1Char(',')).toUtf8() + '\n' + QByteArray::number(qint64(rec.value(QStringLiteral("at")).toDouble()));
}
} // namespace

bool Service::blocked(const QString &device) const
{
    // a test build can cut the network between devices: the names of the devices this one may not talk to are in a file of its folder
    if (!QB_TEST_ENV_SET("QB_LS_BLOCK") || device.isEmpty())
        return false;
    QFile f(m_cfg.dir + QStringLiteral("/ls-block.txt"));
    if (!f.open(QIODevice::ReadOnly))
        return false;
    return QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts).contains(device);
}

net::TrustPolicy Service::memberTrust()
{
    TrustPolicy t;
    t.decide = [this](const QByteArray &pin, bool isServer) {
        using Trust = PeerInfo::Trust;
        if (m_peers && m_peers->contains(pin))
            return Trust::Paired;
        // a device of the signed list of a shared project: taken for the direct links of that project (and a join of a host it
        // never paired with); the first message says which, and anything else is refused there
        for (const Proj *p : m_projects)
            if (p->auth)
                for (const Member &mb : p->auth->membership().members)
                    if (mb.pin() == pin)
                        return Trust::Member;
        if (m_pairWindow.isOpen())
            return Trust::Pairing;
        return isServer ? Trust::Unknown : TrustPolicy::Reject;
    };
    return t;
}

QStringList Service::ownAddresses() const
{
    QStringList out;
    const QString forced = QB_TEST_ENV("QB_LS_ADDRESS"); // (a suite on one machine)
    if (!forced.isEmpty())
        out << forced;
    for (const QNetworkInterface &i : QNetworkInterface::allInterfaces()) {
        if (!(i.flags() & QNetworkInterface::IsUp) || !(i.flags() & QNetworkInterface::IsRunning) || (i.flags() & QNetworkInterface::IsLoopBack))
            continue;
        for (const QNetworkAddressEntry &e : i.addressEntries()) {
            const QString a = plainAddress(e.ip().toString());
            if (!a.isEmpty() && !out.contains(a) && out.size() < 4)
                out << a;
        }
    }
    return out;
}

QStringList Service::directLinks(const QString &uuid) const
{
    QStringList out;
    if (const Proj *p = m_projects.value(uuid))
        for (auto it = p->mesh.constBegin(); it != p->mesh.constEnd(); ++it)
            if (it->link && it->link->state() == Link::State::Ready)
                out << it.key();
    out.sort();
    return out;
}

bool Service::meshReady(const Proj *p, const QString &device) const
{
    const auto it = p->mesh.constFind(device);
    return it != p->mesh.constEnd() && it->link && it->link->state() == Link::State::Ready;
}

bool Service::mainReach(const Proj *p, const QString &device) const
{
    if (p->host && p->online.contains(device))
        return true;
    return !p->entry.host && p->client && !p->client->isLoopback() && p->entry.hostDeviceId == device && p->client->state() == ClientSession::State::Ready;
}

void Service::meshLost(Proj *p, const QString &device)
{
    if (p->auth && !mainReach(p, device))
        p->auth->peerDisconnected(device);
    scheduleState(p->entry.uuid);
}

void Service::learnAddress(const QString &device, const QString &address, quint16 port)
{
    if (device == m_id->deviceId() || address.isEmpty() || port == 0)
        return;
    QList<QPair<QString, quint16>> &l = m_addrs[device];
    const QPair<QString, quint16> a = qMakePair(address, port);
    l.removeAll(a);
    l.prepend(a);
    while (l.size() > 4)
        l.removeLast();
    m_peerAddr.insert(device, a);
}

QJsonObject Service::presenceRecord(Proj *p)
{
    Q_UNUSED(p);
    QJsonArray addrs;
    for (const QString &a : ownAddresses())
        addrs.append(a);
    QJsonObject rec{{QStringLiteral("device"), m_id->deviceId()},
                    {QStringLiteral("name"), m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName},
                    {QStringLiteral("port"), m_server ? int(m_server->port()) : 0},
                    {QStringLiteral("addrs"), addrs},
                    {QStringLiteral("at"), double(m_clock->wallMs() / 1000)}};
    rec.insert(QStringLiteral("sig"), QString::fromLatin1(m_id->sign(presenceBytes(p->entry.uuid, rec)).toBase64()));
    return rec;
}

void Service::sendPresence(Proj *p)
{
    if (!p->auth || p->auth->membership().isEmpty() || !m_server)
        return;
    const QJsonObject rec = presenceRecord(p);
    QJsonObject body{{QStringLiteral("rec"), rec}};
    if (p->host) { // the host passes on what the others told it (signed by them), so that clients learn each other's addresses
        QJsonArray relay;
        for (auto it = p->book.constBegin(); it != p->book.constEnd(); ++it) {
            QJsonObject r = it.value().first;
            if (!it.value().second.isEmpty())
                r.insert(QStringLiteral("seen"), it.value().second);
            relay.append(r);
        }
        if (!relay.isEmpty())
            body.insert(QStringLiteral("relay"), relay);
    }
    const QJsonObject msg = proto::make(proto::T::PRESENCE, p->entry.uuid, body);
    if (QB_TEST_ENV_SET("QB_LS_TRACE"))
        std::fprintf(stderr, "[%s] send presence host=%d online=%d client=%d\n", qPrintable(m_id->deviceId().left(4)), int(p->host != nullptr), int(p->online.size()),
                     int(p->client && !p->client->isLoopback() && p->client->state() == ClientSession::State::Ready));
    if (p->host)
        for (const QString &d : p->online)
            if (d != m_id->deviceId())
                p->host->sendToDevice(d, msg);
    if (p->client && !p->client->isLoopback() && p->client->state() == ClientSession::State::Ready)
        p->client->sendMessage(msg);
    for (auto it = p->mesh.constBegin(); it != p->mesh.constEnd(); ++it)
        if (it->link && it->link->state() == Link::State::Ready)
            it->link->send(Frame::json(msg));
}

void Service::onPresence(Proj *p, const QString &from, const QJsonObject &m, const QString &seenRaw)
{
    if (QB_TEST_ENV_SET("QB_LS_TRACE"))
        std::fprintf(stderr, "[%s] presence from %s seen=%s relay=%d\n", qPrintable(m_id->deviceId().left(4)), qPrintable(from.left(4)), qPrintable(seenRaw), int(m.value(QStringLiteral("relay")).toArray().size()));
    if (!p->auth || p->auth->membership().isEmpty())
        return;
    const Membership &ms = p->auth->membership();
    auto take = [&](const QJsonObject &r, const QString &seen) {
        const QString device = r.value(QStringLiteral("device")).toString();
        const Member *mb = ms.find(device);
        if (!mb || device == m_id->deviceId())
            return;
        const QByteArray sig = QByteArray::fromBase64(r.value(QStringLiteral("sig")).toString().toLatin1());
        if (!net::DeviceIdentity::verify(mb->key, presenceBytes(p->entry.uuid, r), sig))
            return; // only the device itself speaks for its address and name
        const double at = r.value(QStringLiteral("at")).toDouble();
        if (at < p->bookAt.value(device, -1))
            return;
        p->bookAt.insert(device, at);
        const QString shown = plainAddress(seen);
        p->book.insert(device, qMakePair(r, shown));
        const int port = r.value(QStringLiteral("port")).toInt();
        if (port > 0 && port < 65536) {
            QStringList list;
            for (const QJsonValue &v : r.value(QStringLiteral("addrs")).toArray())
                if (!plainAddress(v.toString()).isEmpty())
                    list << plainAddress(v.toString());
            if (!shown.isEmpty())
                list.prepend(shown); // what the other end saw is the best address; what the device says of itself follows
            for (int i = list.size() - 1; i >= 0; --i) // (the first one ends up first)
                learnAddress(device, list.at(i), quint16(port));
        }
        const QString name = cleanName(r.value(QStringLiteral("name")).toString());
        const bool hostShownOld = device == p->entry.hostDeviceId && !p->entry.host && p->entry.hostName != name;
        if (!name.isEmpty() && (name != p->peerNames.value(device, mb->name) || hostShownOld))
            applyDeviceName(p, device, name);
        // the host that this device waits for is on its way: knock at once instead of at the next turn
        if (device == p->entry.hostDeviceId && !p->entry.host && p->client && !p->client->isLoopback() && p->client->state() != ClientSession::State::Ready &&
            (!p->link || p->link->state() == Link::State::Closed)) {
            p->tries = 0;
            connectClient(p);
        }
    };
    if (m.value(QStringLiteral("rec")).toObject().value(QStringLiteral("device")).toString() == from)
        take(m.value(QStringLiteral("rec")).toObject(), seenRaw);
    if (from == p->auth->host() || from == p->entry.hostDeviceId)
        for (const QJsonValue &v : m.value(QStringLiteral("relay")).toArray()) {
            const QJsonObject r = v.toObject();
            take(r, r.value(QStringLiteral("seen")).toString());
        }
}

void Service::applyDeviceName(Proj *p, const QString &device, const QString &name)
{
    p->peerNames.insert(device, name);
    if (p->host)
        p->host->renameDevice(device, name);
    for (const net::PeerRecord &r : m_peers->peers())
        if (r.deviceId() == device && r.name != name) {
            net::PeerRecord n = r;
            n.name = name;
            m_peers->add(n);
        }
    if (device == p->entry.hostDeviceId && !p->entry.host && p->entry.hostName != name) {
        p->entry.hostName = name;
        m_registry->upsert(p->entry);
    }
    if (p->auth && p->auth->isHost())
        publishAccess(p);
    emit devicesChanged();
    emit projectChanged(p->entry.uuid);
    scheduleState(p->entry.uuid);
}

void Service::setDeviceName(const QString &name)
{
    const QString n = cleanName(name);
    m_cfg.deviceName = n;
    if (!m_running || n.isEmpty())
        return;
    ServiceScope scope;
    for (Proj *p : m_projects) {
        if (p->host) {
            p->host->setHostName(n);
            p->host->renameDevice(m_id->deviceId(), n);
        }
        if (p->auth && p->auth->isHost())
            publishAccess(p);
        sendPresence(p);
        emit projectChanged(p->entry.uuid);
    }
    emit devicesChanged();
}

void Service::meshTick()
{
    if (!m_running || !m_server)
        return;
    const qint64 now = m_clock->now();
    const bool presenceDue = now - m_lastPresence >= kPresenceMs;
    if (presenceDue)
        m_lastPresence = now;
    for (Proj *p : m_projects) {
        if (!p->auth || p->auth->membership().isEmpty() || p->auth->record().chain.isEmpty())
            continue;
        ServiceScope scope;
        const QList<Member> members = p->auth->membership().members;
        for (const Member &mb : members) {
            if (mb.id == m_id->deviceId())
                continue;
            MeshPeer &mp = p->mesh[mb.id];
            if (blocked(mb.id)) { // (a cut network of a test build: every connection to the device goes)
                if (mp.link)
                    mp.link->close(QStringLiteral("BLOCKED"));
                if (p->host)
                    p->host->closeDevice(mb.id, QStringLiteral("BLOCKED"));
                if (p->link && p->entry.hostDeviceId == mb.id)
                    p->link->close(QStringLiteral("BLOCKED"));
                continue;
            }
            if (mp.link && mp.link->state() == Link::State::Ready) {
                if (now - mp.lastFrame > kMeshDeadMs)
                    mp.link->close(QStringLiteral("TIMEOUT"));
                continue;
            }
            if (mp.dialing || now < mp.nextTry || mainReach(p, mb.id))
                continue;
            meshDial(p, mb);
        }
        if (presenceDue)
            sendPresence(p);
    }
}

void Service::meshDial(Proj *p, const Member &mb)
{
    MeshPeer &mp = p->mesh[mb.id];
    if (QB_TEST_ENV_SET("QB_LS_TRACE"))
        std::fprintf(stderr, "[%s] dial %s known=%d\n", qPrintable(m_id->deviceId().left(4)), qPrintable(mb.id.left(4)), int(m_addrs.value(mb.id).size()));
    QList<std::pair<QString, quint16>> where;
    for (const auto &a : m_addrs.value(mb.id))
        where << std::make_pair(a.first, a.second);
    if (m_peerAddr.contains(mb.id))
        where << std::make_pair(m_peerAddr.value(mb.id).first, m_peerAddr.value(mb.id).second);
    if (mb.id == p->entry.hostDeviceId && !p->entry.lastAddress.isEmpty())
        where << std::make_pair(p->entry.lastAddress, quint16(p->entry.lastPort ? p->entry.lastPort : net::TlsServer::kPreferredPort));
    if (m_discovery)
        for (const net::PeerRecord &r : m_peers->peers())
            if (r.deviceId() == mb.id)
                for (const mdns::PeerInfo &pi : m_discovery->peers())
                    if (pi.id == r.instanceId)
                        for (const QHostAddress &a : pi.addresses)
                            where << std::make_pair(a.toString(), pi.port);
    const qint64 now = m_clock->now();
    if (where.isEmpty()) {
        mp.nextTry = now + 2000;
        return;
    }
    const auto target = where.at(mp.turn++ % where.size());
    const QString uuid = p->entry.uuid;
    const QString device = mb.id;
    TlsLink *link = TlsLink::connectTo(target.first, target.second, m_id, TrustPolicy::pinned(mb.pin()), this);
    mp.dialing = link;
    connect(link, &Link::stateChanged, this, [this, uuid, device, link, target](Link::State st) {
        Proj *q = m_projects.value(uuid);
        if (!q || st != Link::State::Ready)
            return;
        MeshPeer &m = q->mesh[device];
        if (m.dialing != link)
            return;
        m.dialing = nullptr;
        if (!q->auth || !q->auth->membership().isMember(device)) {
            link->close(QStringLiteral("UNKNOWN_DEVICE"));
            return;
        }
        ServiceScope scope;
        // the first message names the project (and tells this device's address and name)
        link->send(Frame::json(proto::make(proto::T::PRESENCE, uuid, {{QStringLiteral("rec"), presenceRecord(q)}})));
        if (meshAdopt(q, device, link, true))
            learnAddress(device, target.first, target.second);
    });
    connect(link, &Link::closed, this, [this, uuid, device, link](const QString &code) {
        if (Proj *q = m_projects.value(uuid)) {
            MeshPeer &m = q->mesh[device];
            if (m.dialing == link)
                m.dialing = nullptr;
            const qint64 t = m_clock ? m_clock->now() : 0;
            if (code == QLatin1String("DUPLICATE")) {
                m.nextTry = t + 3000;
            } else {
                ++m.tries;
                m.nextTry = t + qMin<qint64>(kMeshBackoffMaxMs, 1000LL << qMin(m.tries, 4));
            }
        }
        link->deleteLater();
    });
}

bool Service::meshAccept(TlsLink *link, const QJsonObject &first)
{
    Proj *p = m_projects.value(first.value(QStringLiteral("p")).toString());
    const QString device = link->peer().deviceId;
    if (!p || !p->auth || !p->auth->membership().isMember(device) || blocked(device))
        return false;
    ServiceScope scope;
    if (!meshAdopt(p, device, link, false))
        return true; // (the other link of the two is kept: this one was closed as a duplicate)
    onPresence(p, device, first, link->peer().address);
    link->send(Frame::json(proto::make(proto::T::PRESENCE, p->entry.uuid, {{QStringLiteral("rec"), presenceRecord(p)}})));
    return true;
}

bool Service::meshAdopt(Proj *p, const QString &device, TlsLink *link, bool outgoing)
{
    MeshPeer &mp = p->mesh[device];
    if (mp.link && mp.link != link && mp.link->state() == Link::State::Ready) {
        // two links between the same two devices: the one the lower device id opened stays (both ends decide the same way)
        const bool keepOutgoing = m_id->deviceId() < device;
        if (outgoing != keepOutgoing) {
            link->close(QStringLiteral("DUPLICATE"));
            return false;
        }
        TlsLink *old = mp.link;
        mp.link = nullptr;
        old->close(QStringLiteral("DUPLICATE"));
    }
    if (QB_TEST_ENV_SET("QB_LS_TRACE"))
        std::fprintf(stderr, "[%s] direct link to %s (%s)\n", qPrintable(m_id->deviceId().left(4)), qPrintable(device.left(4)), outgoing ? "out" : "in");
    mp.link = link;
    mp.outgoing = outgoing;
    mp.lastFrame = m_clock->now();
    mp.tries = 0;
    const QString uuid = p->entry.uuid;
    connect(link, &Link::frameReceived, this, [this, uuid, device, link](const Frame &f) {
        Proj *q = m_projects.value(uuid);
        if (q && q->mesh.value(device).link == link)
            meshFrame(q, device, f);
    });
    connect(link, &Link::closed, this, [this, uuid, device, link](const QString &) {
        Proj *q = m_projects.value(uuid);
        if (q && q->mesh.value(device).link == link) {
            q->mesh[device].link = nullptr;
            meshLost(q, device);
        }
    });
    if (p->auth)
        p->auth->peerConnected(device); // (sends this device's chain: an old host that was cut off learns of the new epoch here)
    scheduleState(uuid);
    return true;
}

void Service::meshFrame(Proj *p, const QString &device, const Frame &f)
{
    p->mesh[device].lastFrame = m_clock->now();
    bool ok = false;
    const QJsonObject m = f.kind == Frame::Json ? f.object(&ok) : QJsonObject();
    if (!ok || !p->auth)
        return;
    ServiceScope scope;
    if (m.value(QStringLiteral("t")).toString() == QLatin1String(proto::T::PRESENCE)) {
        const auto it = p->mesh.constFind(device);
        onPresence(p, device, m, it != p->mesh.constEnd() && it->link ? it->link->peer().address : QString());
        p->auth->heardFrom(device); // (a sign of life: the host that answers here is not lost)
        return;
    }
    p->auth->handleMessage(device, m);
}

// ---------------------------------------------------------------- the listener

void Service::onTlsLink(TlsLink *link)
{
    connect(link, &Link::stateChanged, this, [this, link](Link::State st) {
        if (st != Link::State::Ready)
            return;
        const PeerInfo info = link->peer();
        if (blocked(info.deviceId))
            return link->close(QStringLiteral("BLOCKED"));
        if (info.trust == PeerInfo::Trust::Pairing) {
            if (m_pairing)
                return link->close(QStringLiteral("PAIRING_BUSY"));
            m_pairLink = link;
            m_pairing = new net::PairingSession(net::PairingSession::Role::Host, link, m_id, m_peers.get(), &m_pairWindow, link);
            endPairingWith(link);
            connect(m_pairing.data(), &net::PairingSession::codeReady, this, [this](const QString &code) {
                m_pairCode = code;
                emit pairingCodeReady(code, m_pairing ? m_pairing->peerName() : QString(), true);
            });
            connect(m_pairing.data(), &net::PairingSession::finished, this, [this](bool ok, const QString &why) {
                m_pairCode.clear();
                emit pairingFinished(ok, why);
                emit devicesChanged();
            });
            return;
        }
        // a paired device: its first message names the project
        auto conn = std::make_shared<QMetaObject::Connection>();
        *conn = connect(link, &Link::frameReceived, this, [this, link, conn](const Frame &f) {
            disconnect(*conn);
            route(link, f);
        });
    });
}

// a paired device asks which projects of this device it may join (before any HELLO)
void Service::answerProjectList(TlsLink *link, const QJsonObject &m)
{
    const QString device = link->peer().deviceId;
    QJsonArray list;
    for (const Proj *p : m_projects) {
        if (!p->entry.host || !p->host || p->retired)
            continue;
        const QString role = p->entry.access.value(device).toString();
        if (role.isEmpty())
            continue;
        list.append(QJsonObject{{QStringLiteral("p"), p->entry.uuid}, {QStringLiteral("title"), p->entry.title},
                                {QStringLiteral("name"), m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName},
                                {QStringLiteral("role"), role}});
    }
    link->send(Frame::json(proto::replyTo(m, QLatin1String(proto::T::PROJECTS), QJsonObject{{QStringLiteral("projects"), list}})));
    QTimer::singleShot(200, link, [link] { link->close(); });
}

void Service::route(TlsLink *link, const Frame &first)
{
    bool ok = false;
    const QJsonObject m = first.kind == Frame::Json ? first.object(&ok) : QJsonObject();
    if (ok && m.value(QStringLiteral("t")).toString() == QLatin1String(proto::T::PROJECT_LIST)) {
        if (link->peer().trust != PeerInfo::Trust::Paired) { // (the list of projects is for paired devices)
            link->send(Frame::json(proto::errorMessage(QStringLiteral("NOT_PAIRED"), m.value(QStringLiteral("id")).toString())));
            link->close(QStringLiteral("NOT_PAIRED"));
            return;
        }
        answerProjectList(link, m);
        return;
    }
    if (ok && m.value(QStringLiteral("t")).toString() == QLatin1String(proto::T::PRESENCE)) {
        // a member opens a direct link (the first message names the project)
        if (!meshAccept(link, m)) {
            link->send(Frame::json(proto::errorMessage(QStringLiteral("UNKNOWN_PROJECT"), m.value(QStringLiteral("id")).toString())));
            link->close(QStringLiteral("UNKNOWN_PROJECT"));
        }
        return;
    }
    const QString project = m.value(QStringLiteral("p")).toString();
    Proj *p = ok && m.value(QStringLiteral("t")).toString() == QLatin1String("HELLO") ? m_projects.value(project) : nullptr;
    // a device that is only a member of the signed list (it never paired with this one) may join a project this device hosts, nothing else
    if (p && link->peer().trust == PeerInfo::Trust::Member && !(p->auth && p->auth->membership().isMember(link->peer().deviceId)))
        p = nullptr;
    if (!p || !p->host) {
        link->send(Frame::json(proto::errorMessage(QStringLiteral("UNKNOWN_PROJECT"), m.value(QStringLiteral("id")).toString())));
        link->close(QStringLiteral("UNKNOWN_PROJECT"));
        return;
    }
    ScopedLink *scoped = ScopedLink::wrap(link);
    p->host->addLink(scoped);
    emit scoped->frameReceived(first); // the session sees the message that named it
}

QString Service::fingerprint() const
{
    const QByteArray h = m_id->pin().left(8).toHex().toUpper();
    QStringList parts;
    for (int i = 0; i + 4 <= h.size(); i += 4)
        parts << QString::fromLatin1(h.mid(i, 4));
    return parts.join(QLatin1Char(' '));
}

QList<Service::DiscoveredPeer> Service::discovered() const
{
    QList<DiscoveredPeer> l;
    if (!m_discovery)
        return l;
    for (const mdns::PeerInfo &pi : m_discovery->peers()) {
        if (pi.id == m_id->instanceId())
            continue;
        DiscoveredPeer d;
        d.key = pi.id;
        d.port = pi.port;
        d.pairingOpen = pi.pairingOpen;
        if (!pi.addresses.isEmpty())
            d.address = pi.addresses.first().toString();
        for (const net::PeerRecord &r : m_peers->peers())
            if (r.instanceId == pi.id) {
                d.paired = true;
                d.name = r.name;
            }
        l << d;
    }
    return l;
}

void Service::fetchRemoteProjects()
{
    m_remote.clear();
    emit remoteProjectsChanged();
    for (const net::PeerRecord &r : m_peers->peers()) {
        QString address;
        quint16 port = 0;
        if (m_peerAddr.contains(r.deviceId())) {
            address = m_peerAddr.value(r.deviceId()).first;
            port = m_peerAddr.value(r.deviceId()).second;
        }
        if (m_discovery)
            for (const mdns::PeerInfo &pi : m_discovery->peers())
                if (pi.id == r.instanceId && !pi.addresses.isEmpty()) {
                    address = pi.addresses.first().toString();
                    port = pi.port;
                }
        if (address.isEmpty())
            continue;
        TlsLink *link = TlsLink::connectTo(address, port, m_id, TrustPolicy::pinned(r.pin), this);
        const QString devId = r.deviceId(), devName = r.name;
        connect(link, &Link::stateChanged, this, [link](Link::State st) {
            if (st == Link::State::Ready)
                link->send(Frame::json(proto::make(QLatin1String(proto::T::PROJECT_LIST), QString(), QJsonObject{{QStringLiteral("id"), Registry::newUuid()}})));
        });
        connect(link, &Link::frameReceived, this, [this, link, devId, devName, address, port](const Frame &f) {
            bool ok = false;
            const QJsonObject m = f.kind == Frame::Json ? f.object(&ok) : QJsonObject();
            if (!ok || m.value(QStringLiteral("t")).toString() != QLatin1String(proto::T::PROJECTS))
                return;
            for (const QJsonValue &v : m.value(QStringLiteral("projects")).toArray()) {
                const QJsonObject o = v.toObject();
                RemoteProject rp;
                rp.uuid = o.value(QStringLiteral("p")).toString();
                rp.title = o.value(QStringLiteral("title")).toString();
                rp.hostDeviceId = devId;
                rp.hostName = devName;
                rp.address = address;
                rp.port = port;
                RegistryEntry known;
                if (!rp.uuid.isEmpty() && !(m_registry->find(rp.uuid, &known) && !known.stopped))
                    m_remote << rp;
            }
            emit remoteProjectsChanged();
            link->close();
        });
        connect(link, &Link::closed, link, &QObject::deleteLater);
    }
}

bool Service::setPriority(const QString &uuid, const QStringList &order, QString *why)
{
    Proj *p = m_projects.value(uuid);
    if (!p || !p->auth || !p->auth->isHost()) {
        if (why)
            *why = tr("Only the host can change the order.");
        return false;
    }
    QList<Member> list;
    for (const QString &id : order)
        for (const Member &mb : p->auth->membership().members)
            if (mb.id == id)
                list << mb;
    for (const Member &mb : p->auth->membership().members) {
        bool in = false;
        for (const Member &o : list)
            in = in || o.id == mb.id;
        if (!in)
            list << mb;
    }
    const bool ok = p->auth->publishMembership(list, why);
    refreshHostInfo(p);
    return ok;
}

void Service::answerEditRequest(const QString &uuid, const QString &resource, bool handOver)
{
    if (ClientSession *c = clientSession(uuid))
        c->answerEdit(resource, handOver);
    // a Decline leaves the request open (the asker's waiting time starts again): the holder's bar stays until it is handed over, the
    // asker gives up or takes over
    if (handOver)
        emit editAskEnded(uuid, resource);
}

// the title of a binder item as the project file of `folder` has it (a replica's headless view: no Project is opened for it)
static QString itemTitleIn(const QString &folder, const QString &docId)
{
    if (docId.isEmpty())
        return QString();
    const QString file = Project::resolveProjectFile(folder);
    QByteArray raw;
    if (file.isEmpty() || !vault::readFile(file, &raw))
        return docId;
    QDomDocument dom;
    if (!dom.setContent(raw))
        return docId;
    const QDomNodeList items = dom.elementsByTagName(QStringLiteral("Item"));
    for (int i = 0; i < items.size(); ++i) {
        const QDomElement e = items.at(i).toElement();
        if (e.attribute(QStringLiteral("id")) == docId) {
            const QString t = e.firstChildElement(QStringLiteral("Title")).text();
            return t.isEmpty() ? docId : t;
        }
    }
    return docId;
}

// the first document of an outbox entry that has a text file put in it (the units name the files by path)
static QString firstTextOfUnits(const QJsonArray &units, QString *path, QString *hash)
{
    for (const QJsonValue &v : units) {
        proto::Unit u;
        if (!v.isObject() || !proto::fromJson(v.toObject(), &u))
            continue;
        for (const proto::FileOp &f : u.files) {
            const PathInfo info = classify(f.path);
            if (f.kind == proto::FileOp::Put && info.kind == PathKind::Text) {
                *path = f.path;
                *hash = f.hash;
                return info.id;
            }
        }
    }
    return QString();
}

QList<Service::DraftEntry> Service::drafts(const QString &uuid) const
{
    QList<DraftEntry> l;
    for (auto it = m_projects.constBegin(); it != m_projects.constEnd(); ++it) {
        if (!uuid.isEmpty() && it.key() != uuid)
            continue;
        const Proj *p = it.value();
        if (p->auth)
            for (const Draft &d : p->auth->drafts().list()) {
                DraftEntry e;
                e.id = d.id;
                e.uuid = it.key();
                e.docId = d.document;
                e.docTitle = itemTitleIn(p->entry.folder, d.document);
                e.reason = d.reason;
                e.whenMs = d.createdMs;
                l << e;
            }
        if (p->client)
            for (const OutboxEntry &d : p->client->outbox().drafts()) {
                DraftEntry e;
                e.id = QStringLiteral("outbox:") + it.key() + QLatin1Char(':') + d.opId;
                e.uuid = it.key();
                e.reason = d.reason;
                e.whenMs = d.createdMs;
                QString path, hash;
                e.docId = firstTextOfUnits(d.units, &path, &hash);
                e.docTitle = itemTitleIn(p->entry.folder, e.docId);
                l << e;
            }
    }
    return l;
}

Service::DraftContent Service::draftContent(const QString &id) const
{
    DraftContent c;
    auto fill = [&](const QString &uuid, const QString &docId, const QByteArray &text, bool rtf, const QString &reason, qint64 when) {
        const Proj *p = m_projects.value(uuid);
        c.uuid = uuid;
        c.docId = docId;
        c.docTitle = p ? itemTitleIn(p->entry.folder, docId) : docId;
        c.text = text;
        c.rtf = rtf;
        c.reason = reason;
        c.whenMs = when;
        c.ok = !docId.isEmpty() && !text.isNull();
        if (!c.ok)
            c.why = tr("This draft holds no text of a document.");
    };
    if (id.startsWith(QLatin1String("outbox:"))) {
        const QString rest = id.mid(7);
        const QString uuid = rest.section(QLatin1Char(':'), 0, 0), opId = rest.section(QLatin1Char(':'), 1);
        const Proj *p = m_projects.value(uuid);
        if (!p || !p->client) {
            c.why = tr("The project of this draft is not shared on this device any more.");
            return c;
        }
        for (const OutboxEntry &d : p->client->outbox().drafts())
            if (d.opId == opId) {
                QString path, hash;
                const QString doc = firstTextOfUnits(d.units, &path, &hash);
                QByteArray bytes;
                if (!doc.isEmpty())
                    p->client->outbox().blobs().read(hash, &bytes);
                fill(uuid, doc, bytes, path.endsWith(QLatin1String(".rtf")), d.reason, d.createdMs);
                return c;
            }
        c.why = tr("The draft is not there any more.");
        return c;
    }
    for (const Proj *p : m_projects)
        if (p->auth && p->auth->drafts().exists(id))
            for (const Draft &d : p->auth->drafts().list())
                if (d.id == id) {
                    fill(p->entry.uuid, d.document, d.hasText ? p->auth->drafts().text(id) : QByteArray(), d.textPath.endsWith(QLatin1String(".rtf")), d.reason,
                         d.createdMs);
                    return c;
                }
    c.why = tr("The draft is not there any more.");
    return c;
}

bool Service::deleteDraft(const QString &id)
{
    if (id.startsWith(QLatin1String("outbox:"))) {
        const QString rest = id.mid(7);
        const QString uuid = rest.section(QLatin1Char(':'), 0, 0), opId = rest.section(QLatin1Char(':'), 1);
        Proj *p = m_projects.value(uuid);
        if (!p || !p->client)
            return false;
        return p->client->outbox().removeDraft(opId);
    }
    for (Proj *p : m_projects)
        if (p->auth && p->auth->drafts().exists(id)) {
            const bool ok = p->auth->drafts().remove(id);
            return ok;
        }
    return false;
}

// ---------------------------------------------------------------- pairing

bool Service::openPairing(QString *why)
{
    if (!m_pairWindow.open(why))
        return false;
    if (m_discovery)
        m_discovery->setPairingOpen(true);
    return true;
}

// when the pairing connection closes its session goes with it: the next device may pair (before, the first session stayed until the
// service ended and every later attempt was refused as busy)
void Service::endPairingWith(TlsLink *link)
{
    connect(link, &Link::closed, this, [this, link](const QString &) {
        QPointer<net::PairingSession> session = m_pairing;
        QTimer::singleShot(0, this, [this, link, session] {
            if (session && session->parent() == link)
                delete session.data();
            link->deleteLater();
        });
    });
}

void Service::closePairing() { m_pairWindow.close(); }

void Service::pairWith(const QString &host, quint16 port)
{
    if (m_pairing)
        return;
    TlsLink *link = TlsLink::connectTo(host, port, m_id, TrustPolicy::pairing(), this);
    m_pairLink = link;
    connect(link, &Link::stateChanged, this, [this, link](Link::State st) {
        if (st == Link::State::Ready && !m_pairing) {
            m_pairing = new net::PairingSession(net::PairingSession::Role::Guest, link, m_id, m_peers.get(), nullptr, link);
            endPairingWith(link);
            connect(m_pairing.data(), &net::PairingSession::codeReady, this, [this](const QString &code) {
                m_pairCode = code;
                emit pairingCodeReady(code, m_pairing ? m_pairing->peerName() : QString(), false);
            });
            connect(m_pairing.data(), &net::PairingSession::finished, this, [this](bool ok, const QString &why) {
                m_pairCode.clear();
                emit pairingFinished(ok, why);
                emit devicesChanged();
            });
        } else if (st == Link::State::Closed && !m_pairing) {
            emit pairingFinished(false, QStringLiteral("LINK"));
        }
    });
}

void Service::confirmPairing(bool match)
{
    if (m_pairing)
        m_pairing->confirm(match);
}

// ---------------------------------------------------------------- the windows

void Service::attachWindow(Link *link)
{
    auto *w = new Window;
    w->link = link;
    m_windows << w;
    m_idleTimer.stop();
    connect(link, &Link::frameReceived, this, [this, w](const Frame &f) { onWindowFrame(w, f); });
    connect(link, &Link::closed, this, [this, w](const QString &) { onWindowClosed(w); });
}

void Service::onWindowClosed(Window *w)
{
    if (!m_windows.contains(w))
        return;
    m_windows.removeOne(w);
    delete w;
    checkIdle();
}

void Service::sendWindow(Window *w, const QJsonObject &m)
{
    if (w->link)
        w->link->send(Frame::json(m));
}

void Service::tellWindows(const QString &uuid, const QJsonObject &m)
{
    for (Window *w : m_windows)
        if (w->uuids.contains(uuid))
            sendWindow(w, m);
}

void Service::scheduleState(const QString &uuid)
{
    if (m_statePending.contains(uuid))
        return;
    m_statePending.insert(uuid);
    QTimer::singleShot(0, this, [this, uuid] { emitState(uuid); });
}

void Service::emitState(const QString &uuid)
{
    m_statePending.remove(uuid);
    if (!m_projects.contains(uuid))
        return;
    tellWindows(uuid, stateJson(uuid));
}

ShareState Service::shareStateOf(const Proj &p) const
{
    if (!p.problem.isEmpty() || !p.client)
        return ShareState::Problem;
    switch (p.client->state()) {
    case ClientSession::State::Ready:
        return p.client->hasPending() ? ShareState::Sending : ShareState::UpToDate;
    case ClientSession::State::Joining:
    case ClientSession::State::Syncing:
        return ShareState::Sending;
    case ClientSession::State::Failed:
        return ShareState::Problem;
    default:
        return ShareState::NotConnected;
    }
}

QString Service::readOnlyReason(const Proj &p) const
{
    if (!p.client)
        return tr("the project is not available");
    if (p.client->role() == QLatin1String("reader"))
        return tr("this device has read-only access to the project");
    switch (p.client->state()) {
    case ClientSession::State::Ready:
        return QString();
    case ClientSession::State::Failed:
        return tr("Live Sync stopped for this project");
    case ClientSession::State::Idle:
    case ClientSession::State::Connecting:
    case ClientSession::State::Offline:
        return tr("this device is not connected to the host");
    default:
        return tr("the project is being brought up to date");
    }
}

QJsonObject Service::stateJson(const QString &uuid) const
{
    QJsonObject o = local::msg(local::STATE);
    o.insert(QStringLiteral("uuid"), uuid);
    const Proj *p = m_projects.value(uuid);
    if (!p) {
        o.insert(QStringLiteral("gone"), true);
        return o;
    }
    const ClientSession *c = p->client.get();
    o.insert(QStringLiteral("host"), p->entry.host);
    o.insert(QStringLiteral("title"), p->entry.title);
    o.insert(QStringLiteral("hostName"), p->entry.hostName);
    o.insert(QStringLiteral("hostDeviceId"), p->entry.hostDeviceId);
    o.insert(QStringLiteral("epoch"), double(p->auth ? p->auth->epoch() : p->entry.epoch));
    o.insert(QStringLiteral("state"), int(shareStateOf(*p)));
    o.insert(QStringLiteral("problem"), p->problem.isEmpty() && c && c->state() == ClientSession::State::Failed ? c->failure() : p->problem);
    o.insert(QStringLiteral("ready"), c && c->state() == ClientSession::State::Ready && c->replicaReady());
    o.insert(QStringLiteral("seq"), c ? double(c->seq()) : 0.0);
    const QString why = readOnlyReason(*p);
    o.insert(QStringLiteral("readOnly"), !why.isEmpty());
    o.insert(QStringLiteral("readOnlyReason"), why);
    QJsonObject held;
    QJsonArray unreachable, mine;
    if (c) {
        const QHash<QString, QString> others = c->othersLocks();
        for (auto it = others.constBegin(); it != others.constEnd(); ++it)
            held.insert(it.key(), it.value());
        for (const QString &r : c->unreachableLocks())
            unreachable.append(r);
        for (const QString &r : c->heldLocks())
            mine.append(r);
    }
    QJsonObject tokens; // the tokens of this device's own locks (a test looks at the lease: a lock that went along with a handover keeps it)
    if (c)
        for (const QString &r : c->heldLocks()) {
            const proto::Token t = c->token(r);
            tokens.insert(r, QJsonObject{{QStringLiteral("epoch"), double(t.epoch)}, {QStringLiteral("generation"), double(t.generation)}, {QStringLiteral("lease"), t.lease}});
        }
    o.insert(QStringLiteral("mineTokens"), tokens);
    o.insert(QStringLiteral("held"), held);
    o.insert(QStringLiteral("unreachable"), unreachable);
    o.insert(QStringLiteral("mine"), mine);
    o.insert(QStringLiteral("pending"), c ? int(c->outbox().pending().size()) : 0);
    o.insert(QStringLiteral("drafts"), c ? int(c->outbox().drafts().size()) : 0);
    o.insert(QStringLiteral("role"), c ? c->role() : QString());
    return o;
}

void Service::onWindowFrame(Window *w, const Frame &f)
{
    if (f.kind == Frame::Data) {
        QByteArray transfer, bytes;
        quint64 offset = 0;
        if (!f.dataParts(&transfer, &offset, &bytes))
            return;
        for (auto sit = w->submits.begin(); sit != w->submits.end(); ++sit)
            for (auto it = sit->sizes.constBegin(); it != sit->sizes.constEnd(); ++it)
                if (local::transferOfHash(it.key()) == transfer) {
                    QByteArray &buf = sit->data[it.key()];
                    if (quint64(buf.size()) == offset)
                        buf.append(bytes);
                    finishSubmit(w, sit.key());
                    return;
                }
        return;
    }
    bool ok = false;
    const QJsonObject m = f.object(&ok);
    if (ok)
        onWindowMessage(w, m);
}

// A snapshot job of a window: the host does it (this device's own host code, or the host over the network). The answer comes when the
// host has answered and its files are in this device's copy.
void Service::windowJob(Window *w, const QJsonObject &m)
{
    const QString uuid = m.value(QStringLiteral("uuid")).toString();
    const QString id = m.value(QStringLiteral("id")).toString();
    bool ok = false;
    QString code, text, file;
    Proj *p = m_projects.value(uuid);
    const QString op = m.value(QStringLiteral("op")).toString();
    const QString doc = m.value(QStringLiteral("doc")).toString();
    const QString fileName = m.value(QStringLiteral("file")).toString();
    const QString title = m.value(QStringLiteral("title")).toString();
    if (!p) {
        code = QStringLiteral("NOT_SHARED");
    } else if (p->jobs) {
        ServiceScope scope;
        QJsonObject body = m;
        body.remove(QStringLiteral("t"));
        body.remove(QStringLiteral("id"));
        body.remove(QStringLiteral("uuid"));
        const HostBackup::JobAnswer a = p->jobs->runJobFor(m_id->deviceId(), body);
        ok = a.ok;
        code = a.code;
        text = a.text;
        file = a.file;
    } else if (p->jobsClient) {
        QPointer<HostJobsClient> jc(p->jobsClient.get());
        snapshots::RemoteJobs *r = jc->remoteJobs();
        QString err;
        if (op == QLatin1String("take"))
            ok = r->take(doc, title, &err);
        else if (op == QLatin1String("rename"))
            ok = r->setTitle(doc, fileName, title, &err);
        else if (op == QLatin1String("delete"))
            ok = r->remove(doc, fileName, &err);
        else if (op == QLatin1String("rollBack"))
            ok = r->rollBack(doc, fileName, m.value(QStringLiteral("allowEmpty")).toBool(false), &err);
        else
            code = QStringLiteral("BAD_OP");
        text = err;
        if (jc && !ok)
            code = jc->lastJobCode().isEmpty() ? QStringLiteral("FAILED") : jc->lastJobCode();
    } else {
        code = QStringLiteral("NOT_CONNECTED");
        text = tr("The device that hosts the project cannot be reached now.");
    }
    if (!m_windows.contains(w)) // (the window went away while the host was asked)
        return;
    sendWindow(w, local::msg(local::JOB_RESULT, {{"re", id}, {"ok", ok}, {"code", code}, {"text", text}, {"file", file}}));
}

void Service::onWindowMessage(Window *w, const QJsonObject &m)
{
    const QString t = local::typeOf(m);
    const QString uuid = m.value(QStringLiteral("uuid")).toString();
    if (t == QLatin1String(local::ATTACH)) {
        windowAttach(w, m);
    } else if (t == QLatin1String(local::PING)) {
        sendWindow(w, local::msg(local::PONG, {{"re", m.value(QStringLiteral("id"))}}));
    } else if (t == QLatin1String(local::DETACH)) {
        w->uuids.remove(uuid);
    } else if (t == QLatin1String(local::SUBMIT)) {
        windowSubmit(w, m);
    } else if (t == QLatin1String(local::JOB)) {
        windowJob(w, m);
    } else if (t == QLatin1String(local::STOP)) {
        // only a service process (no window of its own) ends on request; the caller wants to host in its own program
        sendWindow(w, local::msg(local::PONG, {{"re", m.value(QStringLiteral("id"))}, {"stopping", m_cfg.quitWhenIdle}}));
        if (m_cfg.quitWhenIdle)
            QTimer::singleShot(0, this, [this] { emit quitRequested(); });
    } else if (ClientSession *c = clientSession(uuid)) {
        QStringList res;
        for (const QJsonValue &v : m.value(QStringLiteral("resources")).toArray())
            res << v.toString();
        const QString resource = m.value(QStringLiteral("resource")).toString();
        if (t == QLatin1String(local::LOCK))
            c->lock(res);
        else if (t == QLatin1String(local::UNLOCK)) {
            c->unlock(res);
            for (const QString &r : res)
                emit editAskEnded(uuid, r); // (a lock this device gives up has no request to answer any more)
            scheduleState(uuid); // the window waits for the list of its locks to shrink
        }
        else if (t == QLatin1String(local::ASK))
            m_status->askToEdit(uuid, resource);
        else if (t == QLatin1String(local::CANCEL_ASK))
            m_status->cancelAsk(uuid, resource);
        else if (t == QLatin1String(local::TAKEOVER))
            c->takeOver(resource);
        else if (t == QLatin1String(local::ANSWER))
            c->answerEdit(resource, m.value(QStringLiteral("handOver")).toBool(), m.value(QStringLiteral("asker")).toString());
        else if (t == QLatin1String(local::ACTION)) {
            const QString action = m.value(QStringLiteral("action")).toString();
            if (action == QLatin1String("stopSharing"))
                QTimer::singleShot(0, this, [this, uuid] { stopSharing(uuid); });
            else if (action == QLatin1String("requestBackup"))
                m_status->requestBackup(uuid);
            else if (action == QLatin1String("transferHosting")) {
                QString why;
                if (!transferHosting(uuid, m.value(QStringLiteral("target")).toString(), &why))
                    emit notice(tr("Hosting cannot be handed over now: %1").arg(why));
            }
        }
    }
}

void Service::windowAttach(Window *w, const QJsonObject &m)
{
    const QString id = m.value(QStringLiteral("id")).toString();
    const QString folder = clean(m.value(QStringLiteral("folder")).toString());
    if (m.contains(QStringLiteral("keepHosting")))
        setKeepHosting(m.value(QStringLiteral("keepHosting")).toBool());
    RegistryEntry e;
    m_registry->reload();
    if (!m_registry->findByFolder(folder, &e) || !m_projects.contains(e.uuid)) {
        // maybe another program of this device shared it since the service started
        if (m_registry->findByFolder(folder, &e) && !m_projects.contains(e.uuid)) {
            if (e.host)
                startHost(e);
            else
                startClient(e);
        }
    }
    if (!m_registry->findByFolder(folder, &e) || !m_projects.contains(e.uuid)) {
        sendWindow(w, local::msg(local::ATTACHED, {{"re", id}, {"ok", false}, {"code", "NOT_SHARED"}}));
        return;
    }
    Proj *p = m_projects.value(e.uuid);
    if (!p->client) {
        sendWindow(w, local::msg(local::ATTACHED, {{"re", id}, {"ok", false}, {"code", "PROBLEM"}, {"text", p->problem}}));
        return;
    }
    w->uuids.insert(e.uuid);
    QJsonObject body = stateJson(e.uuid);
    body.insert(QStringLiteral("t"), QLatin1String(local::ATTACHED));
    body.insert(QStringLiteral("re"), id);
    body.insert(QStringLiteral("ok"), true);
    sendWindow(w, body);
}

void Service::windowSubmit(Window *w, const QJsonObject &m)
{
    PendingSubmit ps;
    ps.id = m.value(QStringLiteral("id")).toString();
    ps.uuid = m.value(QStringLiteral("uuid")).toString();
    ps.units = m.value(QStringLiteral("units")).toArray();
    for (const QJsonValue &v : m.value(QStringLiteral("blobs")).toArray()) {
        const QJsonObject b = v.toObject();
        ps.sizes.insert(b.value(QStringLiteral("hash")).toString(), qint64(b.value(QStringLiteral("size")).toDouble()));
        ps.data.insert(b.value(QStringLiteral("hash")).toString(), QByteArray());
    }
    w->submits.insert(ps.id, ps);
    finishSubmit(w, ps.id);
}

void Service::finishSubmit(Window *w, const QString &id)
{
    auto it = w->submits.find(id);
    if (it == w->submits.end())
        return;
    PendingSubmit &ps = *it;
    for (auto s = ps.sizes.constBegin(); s != ps.sizes.constEnd(); ++s)
        if (ps.data.value(s.key()).size() < s.value())
            return; // more bytes to come
    ClientSession *c = clientSession(ps.uuid);
    QString error;
    bool ok = c != nullptr;
    if (!ok)
        error = QStringLiteral("NOT_SHARED");
    QList<proto::Unit> units;
    if (ok)
        for (const QJsonValue &v : ps.units) {
            proto::Unit u;
            QString why;
            if (!proto::fromJson(v.toObject(), &u, &why)) {
                ok = false;
                error = why;
                break;
            }
            units << u;
        }
    for (auto s = ps.sizes.constBegin(); ok && s != ps.sizes.constEnd(); ++s)
        if (QString::fromLatin1(QCryptographicHash::hash(ps.data.value(s.key()), QCryptographicHash::Sha256).toHex()) != s.key()) {
            ok = false;
            error = QStringLiteral("HASH_MISMATCH");
        }
    QString opId;
    if (ok) {
        ServiceScope scope;
        opId = c->submit(units, ps.data, &error);
        ok = !opId.isEmpty();
    }
    const QString uuid = ps.uuid;
    w->submits.erase(it);
    sendWindow(w, local::msg(local::SUBMITTED, {{"re", id}, {"ok", ok}, {"opId", opId}, {"error", error}}));
    scheduleState(uuid);
}

// ---------------------------------------------------------------- status model

ProjectStatus ServiceStatus::project(const QString &uuid) const
{
    ProjectStatus s;
    const Service::Proj *p = m_s->m_projects.value(uuid);
    if (!p)
        return s;
    s.valid = true;
    s.uuid = uuid;
    s.title = p->entry.title;
    s.folder = p->entry.folder;
    s.role = p->entry.host ? ShareRole::Host : (p->client && p->client->role() == QLatin1String("reader") ? ShareRole::Reader : ShareRole::Editor);
    s.hostDeviceId = p->entry.hostDeviceId;
    s.hostName = p->entry.hostName;
    s.epoch = p->client ? p->client->epoch() : p->entry.epoch;
    s.state = m_s->shareStateOf(*p);
    s.problemText = p->problem.isEmpty() && p->client && p->client->state() == ClientSession::State::Failed ? p->client->failure() : p->problem;
    s.pendingChanges = p->client ? int(p->client->outbox().pending().size()) : 0;
    s.draftCount = p->client ? int(p->client->outbox().drafts().size()) : 0;
    const QString why = m_s->readOnlyReason(*p);
    s.notices = p->notices;
    s.readOnly = !why.isEmpty();
    s.readOnlyReason = why;
    const QString self = m_s->m_id->deviceId();
    ParticipantStatus host;
    host.deviceId = p->entry.hostDeviceId;
    host.name = p->entry.hostName;
    host.isHost = true;
    host.thisDevice = p->entry.host;
    host.online = p->entry.host || (p->client && p->client->state() == ClientSession::State::Ready);
    ParticipantStatus me;
    me.deviceId = self;
    me.name = m_s->m_cfg.deviceName.isEmpty() ? m_s->m_id->name() : m_s->m_cfg.deviceName;
    me.thisDevice = true;
    me.online = true;
    me.reader = s.role == ShareRole::Reader;
    QHash<QString, ParticipantStatus> others;
    if (p->client)
        me.editing = p->client->heldLocks();
    if (p->entry.host && p->host) {
        for (const Lock &l : p->host->locks().all()) {
            if (l.owner.device == self)
                continue;
            others[l.owner.device].editing << l.resource;
        }
        for (const HostSession::PeerSummary &ps : p->host->joinedPeers()) {
            if (ps.deviceId == self)
                continue;
            ParticipantStatus &o = others[ps.deviceId];
            o.deviceId = ps.deviceId;
            o.name = ps.name;
            o.online = true;
            o.reader = ps.role == Role::Reader;
        }
        host.editing = me.editing;
    } else if (p->client) {
        const QHash<QString, QString> names = p->client->othersLocks();
        const QHash<QString, QString> devs = p->client->othersDevices();
        for (auto it = devs.constBegin(); it != devs.constEnd(); ++it) {
            if (it.value() == p->entry.hostDeviceId)
                host.editing << it.key();
            else {
                ParticipantStatus &o = others[it.value()];
                o.deviceId = it.value();
                o.name = names.value(it.key());
                o.online = true;
                o.editing << it.key();
            }
        }
    }
    if (p->entry.host)
        s.participants << me; // this device is the host
    else {
        s.participants << host << me;
    }
    if (p->entry.host)
        s.participants.first().isHost = true;
    QList<ParticipantStatus> rest = others.values();
    std::sort(rest.begin(), rest.end(), [](const ParticipantStatus &a, const ParticipantStatus &b) { return a.name.toLower() < b.name.toLower(); });
    s.participants << rest;
    if (p->jobs)
        s.backup = p->jobs->status();
    else if (p->jobsClient)
        s.backup = p->jobsClient->status();
    return s;
}

LockInfo ServiceStatus::lockInfo(const QString &uuid, const QString &resource) const
{
    LockInfo l;
    l.resource = resource;
    const ClientSession *c = m_s->clientSession(uuid);
    if (!c)
        return l;
    if (c->holds(resource)) {
        l.held = true;
        l.mine = true;
        l.holderDeviceId = m_s->m_id->deviceId();
        l.holderName = m_s->m_cfg.deviceName.isEmpty() ? m_s->m_id->name() : m_s->m_cfg.deviceName;
        return l;
    }
    const QString name = c->heldElsewhere(resource);
    if (!name.isEmpty()) {
        l.held = true;
        l.holderName = name;
        l.holderDeviceId = c->othersDevices().value(resource);
        l.unreachable = c->isUnreachable(resource);
    }
    const QString key = uuid + QLatin1Char('|') + resource;
    if (m_s->m_asks.contains(key)) {
        l.askOpen = true;
        l.askWaitMs = c->editWaitMs(resource);
        l.declined = m_s->m_declined.contains(key);
    }
    return l;
}

} // namespace collab
} // namespace qb

#include "Service.moc"
