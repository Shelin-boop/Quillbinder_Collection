#include "core/collab/Service.h"

#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/collab/PathClass.h"
#include "core/collab/ScopedLink.h"
#include "core/collab/ServiceProtocol.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
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

struct Service::Proj {
    RegistryEntry entry;
    std::unique_ptr<HostSession> host;
    std::unique_ptr<ProjectHostBackend> backend;
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
        Q_UNUSED(uuid); // backups of hosted projects are package 8's: nothing to ask yet
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
    m_status = std::make_unique<ServiceStatus>(this);
    if (m_cfg.listen) {
        net::TrustPolicy trust = TrustPolicy::pairedOrPairing(m_peers.get(), [this] { return m_pairWindow.isOpen(); });
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
    }
    m_localName = m_cfg.localName.isEmpty() ? LocalLink::serverName(m_cfg.dir) : m_cfg.localName;
    if (m_cfg.localSocket) {
        m_local = std::make_unique<LocalServer>();
        QString why;
        if (!m_local->listen(m_localName, &why)) {
            m_local.reset();
            return fail(why);
        }
        connect(m_local.get(), &LocalServer::newLink, this, [this](LocalLink *l) { attachWindow(l); });
    }
    m_running = true;
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

void Service::startHost(const RegistryEntry &entry)
{
    if (m_projects.contains(entry.uuid))
        return;
    auto *p = new Proj;
    p->entry = entry;
    m_projects.insert(entry.uuid, p);
    auto failWith = [&](const QString &why) {
        p->problem = why;
        emit notice(tr("Hosting \"%1\" could not start: %2").arg(entry.title, why));
    };
    QString why;
    if (!m_registry->lockHost(entry.uuid, &why)) {
        failWith(why);
        return;
    }
    p->scope = std::make_unique<vault::Scope>(entry.folder);
    HostSession::Options o;
    o.folder = entry.folder;
    o.uuid = entry.uuid;
    o.hostName = m_cfg.deviceName.isEmpty() ? m_id->name() : m_cfg.deviceName;
    o.clock = m_scopedClock.get();
    o.epoch = qMax<qint64>(1, entry.epoch);
    p->host = std::make_unique<HostSession>(o);
    {
        ServiceScope scope;
        if (!p->host->start(&why)) {
            failWith(why);
            p->host.reset();
            m_registry->releaseHost(entry.uuid);
            return;
        }
        p->backend = std::make_unique<ProjectHostBackend>(entry.folder);
        if (!p->backend->open(&why)) {
            failWith(why);
            p->backend.reset();
            p->host.reset();
            m_registry->releaseHost(entry.uuid);
            return;
        }
        p->host->setBackend(p->backend.get());
    }
    const QString uuid = entry.uuid;
    p->host->setRoleProvider([this, uuid](const QString &device) {
        if (device == m_id->deviceId())
            return Role::Editor;
        RegistryEntry e;
        if (!m_registry->find(uuid, &e))
            return Role::None;
        const QString r = e.access.value(device).toString();
        return r == QLatin1String("editor") ? Role::Editor : r == QLatin1String("reader") ? Role::Reader : Role::None;
    });
    connect(p->host.get(), &HostSession::peerJoined, this, [this, uuid](const QString &device, const QString &name) {
        if (Proj *q = m_projects.value(uuid)) {
            q->online.insert(device);
            q->peerNames.insert(device, name);
        }
        emit projectChanged(uuid);
    });
    connect(p->host.get(), &HostSession::peerLeft, this, [this, uuid](const QString &device) {
        if (Proj *q = m_projects.value(uuid)) {
            bool still = false;
            for (const HostSession::PeerSummary &s : q->host->joinedPeers())
                still = still || s.deviceId == device;
            if (!still)
                q->online.remove(device);
        }
        emit projectChanged(uuid);
    });
    connect(p->host.get(), &HostSession::locksChanged, this, [this, uuid] {
        emit locksChanged(uuid);
        scheduleState(uuid);
    });
    connect(p->host.get(), &HostSession::committed, this, [this, uuid] { emit projectChanged(uuid); });
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
        failWith(why);
        return;
    }
    const QByteArray hostPin = QCryptographicHash::hash(QByteArray("loopback-host") + entry.uuid.toUtf8(), QCryptographicHash::Sha256);
    MemLink::Pair pair = MemLink::createPair(hostPin, m_id->pin(), p->host.get());
    p->loop = pair.a;
    p->host->addLink(ScopedLink::wrap(pair.a));
    wireClient(p);
    p->client->attach(ScopedLink::wrap(pair.b));
    emit projectsChanged();
}

void Service::stopProject(const QString &uuid, bool removeEntry)
{
    Proj *p = m_projects.take(uuid);
    if (!p)
        return;
    p->retry.stop();
    if (p->client) {
        p->client->detach();
        p->client.reset();
    }
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
    if (Proj *p = m_projects.value(uuid))
        p->entry = e;
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
    if (m_projects.contains(entry.uuid))
        return;
    auto *p = new Proj;
    p->entry = entry;
    m_projects.insert(entry.uuid, p);
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
        emit notice(tr("The copy of \"%1\" cannot be used: %2").arg(entry.title, why));
        return;
    }
    wireClient(p);
    p->retry.setSingleShot(true);
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
    connect(c, &ClientSession::stateChanged, this, [this, uuid](ClientSession::State) {
        scheduleState(uuid);
        emit projectChanged(uuid);
    });
    connect(c, &ClientSession::readyToUse, this, [this, uuid] { scheduleState(uuid); });
    connect(c, &ClientSession::eventApplied, this,
            [this, uuid](qint64 seq, const QString &opId, const QString &actor, bool own, const QJsonArray &ops, const QStringList &paths, const QJsonObject &revs) {
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
        tellWindows(uuid, local::msg(local::LOCK_EVT, {{"uuid", uuid}, {"kind", "revoked"}, {"resource", resource}, {"reason", reason}}));
        scheduleState(uuid);
        emit locksChanged(uuid);
    });
    connect(c, &ClientSession::locksChanged, this, [this, uuid] {
        scheduleState(uuid);
        emit locksChanged(uuid);
    });
    connect(c, &ClientSession::editAsked, this, [this, uuid](const QString &resource, const QString &asker, qint64 waitMs) {
        tellWindows(uuid, local::msg(local::EDIT_EVT, {{"uuid", uuid}, {"kind", "asked"}, {"resource", resource}, {"asker", asker}, {"waitMs", double(waitMs)}}));
    });
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
    if (!found) {
        p->problem = tr("The host is not paired with this device any more.");
        emit projectChanged(p->entry.uuid);
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

// ---------------------------------------------------------------- the listener

void Service::onTlsLink(TlsLink *link)
{
    connect(link, &Link::stateChanged, this, [this, link](Link::State st) {
        if (st != Link::State::Ready)
            return;
        const PeerInfo info = link->peer();
        if (info.trust == PeerInfo::Trust::Pairing) {
            if (m_pairing)
                return link->close(QStringLiteral("PAIRING_BUSY"));
            m_pairLink = link;
            m_pairing = new net::PairingSession(net::PairingSession::Role::Host, link, m_id, m_peers.get(), &m_pairWindow, link);
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

void Service::route(TlsLink *link, const Frame &first)
{
    bool ok = false;
    const QJsonObject m = first.kind == Frame::Json ? first.object(&ok) : QJsonObject();
    const QString project = m.value(QStringLiteral("p")).toString();
    Proj *p = ok && m.value(QStringLiteral("t")).toString() == QLatin1String("HELLO") ? m_projects.value(project) : nullptr;
    if (!p || !p->host) {
        link->send(Frame::json(proto::errorMessage(QStringLiteral("UNKNOWN_PROJECT"), m.value(QStringLiteral("id")).toString())));
        link->close(QStringLiteral("UNKNOWN_PROJECT"));
        return;
    }
    ScopedLink *scoped = ScopedLink::wrap(link);
    p->host->addLink(scoped);
    emit scoped->frameReceived(first); // the session sees the message that named it
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
    } else if (ClientSession *c = clientSession(uuid)) {
        QStringList res;
        for (const QJsonValue &v : m.value(QStringLiteral("resources")).toArray())
            res << v.toString();
        const QString resource = m.value(QStringLiteral("resource")).toString();
        if (t == QLatin1String(local::LOCK))
            c->lock(res);
        else if (t == QLatin1String(local::UNLOCK)) {
            c->unlock(res);
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
    // the backup state is the host's job (package 8): none yet
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
