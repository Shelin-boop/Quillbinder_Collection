#include "ui/LiveSyncServiceControl.h"

#include "core/collab/Authority.h"
#include "core/collab/net/PeerStore.h"
#include "ui/AppSettings.h"
#include "ui/MainWindowLiveSync.h"

#include "core/collab/LocalLink.h"
#include "core/collab/ServiceProtocol.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QNetworkInterface>
#include <QProcess>

namespace qb {
namespace livesync {

using collab::Role;
using collab::Service;

ServiceControl::ServiceControl(Service *external, QObject *parent) : LiveSyncControl(parent)
{
    if (external) {
        m_svc = external;
    } else {
        Service::Config cfg; // the defaults: this device's identity and data folder, the listener, discovery and the windows' socket
        cfg.deviceName = prefDeviceName();
        cfg.backupDir = [] { return AppSettings::instance().backupDir; }; // the writer's backup folder and number kept, as they are then
        cfg.backupKeep = [] { return AppSettings::instance().backupKeep; };
        m_svc = new Service(cfg, this);
        m_owned = true;
    }
    wire();
    if (m_owned) // the last window is closed and the program ends: hosting may go on in a service process
        connect(qApp, &QCoreApplication::aboutToQuit, this, [this] { handOffHosting(); });
}

bool ServiceControl::handOffHosting(const QString &program)
{
    if (!m_svc || !m_svc->isRunning() || !m_svc->keepHosting() || m_svc->hostedCount() == 0)
        return false;
    const QString app = program.isEmpty() ? QCoreApplication::applicationFilePath() : program;
    if (ui::inProcessLiveSyncService() == m_svc)
        ui::setInProcessLiveSyncService(nullptr);
    m_svc->stop(); // its listener, its socket and the projects' claims are free for the service process
    return QProcess::startDetached(app, {QStringLiteral("--live-sync-service")});
}

bool ServiceControl::takeBackFromServiceProcess(int timeoutMs)
{
    const QString name = collab::LocalLink::serverName(m_svc->dir().isEmpty() ? collab::net::DeviceIdentity::standardDir() : m_svc->dir());
    if (!collab::LocalServer::isServed(name, 300))
        return true; // none runs
    collab::LocalLink *link = collab::LocalLink::connectTo(name);
    QElapsedTimer t;
    t.start();
    while (link->state() == collab::net::Link::State::Connecting && t.elapsed() < 2000) {
        QEventLoop l;
        QTimer::singleShot(10, &l, &QEventLoop::quit);
        l.exec();
    }
    bool stopping = false, answered = false;
    connect(link, &collab::net::Link::frameReceived, link, [&](const collab::net::Frame &f) {
        bool ok = false;
        const QJsonObject m = f.kind == collab::net::Frame::Json ? f.object(&ok) : QJsonObject();
        if (ok && collab::local::typeOf(m) == QLatin1String(collab::local::PONG) && m.value(QStringLiteral("re")).toString() == QLatin1String("stop")) {
            answered = true;
            stopping = m.value(QStringLiteral("stopping")).toBool();
        }
    });
    if (link->state() == collab::net::Link::State::Ready)
        link->send(collab::net::Frame::json(collab::local::msg(collab::local::STOP, {{"id", "stop"}})));
    t.restart();
    while (!answered && t.elapsed() < 2000 && link->state() == collab::net::Link::State::Ready) {
        QEventLoop l;
        QTimer::singleShot(10, &l, &QEventLoop::quit);
        l.exec();
    }
    delete link;
    if (!stopping)
        return false; // another program runs the service: it is not ours to end
    t.restart();
    while (collab::LocalServer::isServed(name, 200) && t.elapsed() < timeoutMs) {
        QEventLoop l;
        QTimer::singleShot(50, &l, &QEventLoop::quit);
        l.exec();
    }
    return !collab::LocalServer::isServed(name, 200);
}

ServiceControl::~ServiceControl()
{
    if (ui::inProcessLiveSyncService() == m_svc)
        ui::setInProcessLiveSyncService(nullptr);
    if (m_owned && m_svc)
        m_svc->stop();
}

void ServiceControl::wire()
{
    connect(m_svc, &Service::pairingCodeReady, this, [this](const QString &code, const QString &name, bool) { emit pairingCode(name, code); });
    connect(m_svc, &Service::pairingFinished, this, [this](bool ok, const QString &why) {
        emit pairingFinished(ok, ok ? tr("The devices are paired.") : (why.isEmpty() ? tr("Pairing did not finish.") : why));
        emit discoveredChanged();
    });
    connect(m_svc, &Service::discoveredChanged, this, &LiveSyncControl::discoveredChanged);
    connect(m_svc, &Service::remoteProjectsChanged, this, &LiveSyncControl::discoveredChanged);
    connect(m_svc, &Service::editAsked, this, [this](const QString &uuid, const QString &resource, const QString &asker) { emit editAsked(uuid, resource, asker); });
    connect(m_svc, &Service::editAskEnded, this, &LiveSyncControl::editAskEnded);
    connect(m_svc, &Service::unknownHostRefused, this, &LiveSyncControl::foundUnknownHostRefused);
    connect(m_svc, &Service::projectChanged, this, [this](const QString &uuid) { emit accessChanged(uuid); });
    connect(m_svc, &Service::devicesChanged, this, &LiveSyncControl::discoveredChanged);
}

void ServiceControl::startIfEnabled()
{
    if (prefEnabled() && !m_svc->isRunning())
        setEnabled(true);
}

void ServiceControl::setEnabled(bool on)
{
    setPrefEnabled(on);
    m_unavailable.clear();
    if (on && !m_svc->isRunning()) {
        QString err;
        if (!m_svc->start(&err) && m_owned && err.contains(QLatin1String("already")) && takeBackFromServiceProcess()) {
            err.clear();
            m_svc->start(&err); // the hosting a service process kept goes on here
        }
        if (!m_svc->isRunning()) {
            m_unavailable = err.contains(QLatin1String("already")) ? tr("Another program of this device runs Live Sync's service.") : err;
            setPrefEnabled(false);
        } else {
            m_svc->setKeepHosting(prefKeepHosting());
            ui::setInProcessLiveSyncService(m_svc);
        }
    } else if (!on && m_svc->isRunning()) {
        if (ui::inProcessLiveSyncService() == m_svc)
            ui::setInProcessLiveSyncService(nullptr);
        m_svc->stop();
    }
    emit enabledChanged();
}

void ServiceControl::setDeviceName(const QString &name)
{
    setPrefDeviceName(name);
    m_svc->setDeviceName(prefDeviceName());
}

void ServiceControl::setKeepHosting(bool on)
{
    setPrefKeepHosting(on);
    m_svc->setKeepHosting(on);
}

QStringList ServiceControl::addresses() const
{
    QStringList l;
    for (const QHostAddress &a : QNetworkInterface::allAddresses())
        if (a.protocol() == QAbstractSocket::IPv4Protocol && !a.isLoopback())
            l << a.toString();
    return l;
}

QString ServiceControl::protocolText() const { return m_svc->isRunning() ? tr("Live Sync 1, TLS 1.3") : QString(); }

void ServiceControl::removeDevice(const QString &deviceId)
{
    if (!m_svc->peers())
        return;
    for (const collab::net::PeerRecord &r : m_svc->peers()->peers())
        if (r.deviceId() == deviceId) {
            m_svc->peers()->remove(r.pin);
            break;
        }
}

QList<DiscoveredDevice> ServiceControl::discovered() const
{
    QList<DiscoveredDevice> l;
    for (const Service::DiscoveredPeer &p : m_svc->discovered()) {
        DiscoveredDevice d;
        d.key = p.key;
        d.name = p.name;
        d.address = p.address;
        d.pairingOpen = p.pairingOpen;
        d.paired = p.paired;
        l << d;
    }
    return l;
}

void ServiceControl::openPairingWindow()
{
    QString why;
    if (!m_svc->openPairing(&why))
        emit pairingFinished(false, why);
    emit pairingWindowChanged();
}

void ServiceControl::closePairingWindow()
{
    m_svc->closePairing();
    emit pairingWindowChanged();
}

void ServiceControl::pairWith(const QString &key)
{
    for (const Service::DiscoveredPeer &p : m_svc->discovered())
        if (p.key == key) {
            m_svc->pairWith(p.address, p.port);
            return;
        }
    emit pairingFinished(false, tr("The device is not found in the network any more."));
}

void ServiceControl::pairWithAddress(const QString &address)
{
    const QString host = address.section(QLatin1Char(':'), 0, 0).trimmed();
    const quint16 port = address.contains(QLatin1Char(':')) ? quint16(address.section(QLatin1Char(':'), 1).toUInt()) : collab::net::TlsServer::kPreferredPort;
    m_svc->pairWith(host, port);
}

bool ServiceControl::shareProject(const QString &folder, QString *error)
{
    const QString uuid = m_svc->shareProject(folder, error);
    emit accessChanged(uuid);
    return !uuid.isEmpty();
}

QList<RemoteProject> ServiceControl::remoteProjects() const
{
    // asking the paired devices is done in the background when the list is read and is some seconds old
    if (m_svc->isRunning() && (!m_fetched.isValid() || m_fetched.elapsed() > 4000)) {
        m_fetched.start();
        QMetaObject::invokeMethod(m_svc, [svc = m_svc] { svc->fetchRemoteProjects(); }, Qt::QueuedConnection);
    }
    QList<RemoteProject> l;
    for (const Service::RemoteProject &r : m_svc->remoteProjects()) {
        RemoteProject p;
        p.uuid = r.uuid;
        p.title = r.title;
        p.hostDeviceId = r.hostDeviceId;
        p.hostName = r.hostName;
        p.hostOnline = true;
        l << p;
    }
    return l;
}

void ServiceControl::join(const QString &deviceId, const QString &uuid)
{
    Service::RemoteProject rp;
    bool found = false;
    for (const Service::RemoteProject &r : m_svc->remoteProjects())
        if (r.uuid == uuid && r.hostDeviceId == deviceId) {
            rp = r;
            found = true;
        }
    if (!found) {
        emit joinFinished(uuid, false, tr("The project is not offered by that device any more."));
        return;
    }
    QString err;
    emit joinProgress(uuid, 10, tr("Connecting..."));
    const QString folder = m_svc->joinProject(deviceId, uuid, rp.title, rp.address, rp.port, &err);
    if (folder.isEmpty()) {
        emit joinFinished(uuid, false, err);
        return;
    }
    // done when the copy is complete (or the service says it cannot)
    auto *timer = new QTimer(this);
    m_joins.insert(uuid, timer);
    auto *tries = new int(0);
    connect(timer, &QTimer::timeout, this, [this, uuid, folder, timer, tries] {
        collab::ClientSession *c = m_svc->clientSession(uuid);
        const collab::ProjectStatus st = m_svc->statusModel()->project(uuid);
        ++*tries;
        if (c && c->state() == collab::ClientSession::State::Ready && c->replicaReady()) {
            emit joinFinished(uuid, true, folder);
        } else if (c && c->state() == collab::ClientSession::State::Failed) {
            emit joinFinished(uuid, false, st.problemText.isEmpty() ? tr("The host did not let this device in.") : st.problemText);
        } else if (*tries < 120) {
            emit joinProgress(uuid, qMin(90, 10 + *tries), tr("Copying the project..."));
            return;
        } else {
            emit joinFinished(uuid, false, tr("The copy did not finish in time."));
        }
        m_joins.remove(uuid);
        delete tries;
        timer->deleteLater();
    });
    timer->start(500);
}

QList<AccessEntry> ServiceControl::access(const QString &uuid) const
{
    QList<AccessEntry> l;
    collab::RegistryEntry e;
    if (!m_svc->registry() || !m_svc->registry()->find(uuid, &e))
        return l;
    const collab::Authority *a = m_svc->authority(uuid);
    for (const collab::net::PeerRecord &r : m_svc->peers()->peers()) {
        AccessEntry ae;
        ae.deviceId = r.deviceId();
        ae.name = r.name;
        const QString role = e.access.value(ae.deviceId).toString();
        ae.role = role == QLatin1String("editor") ? AccessEntry::Editor : role == QLatin1String("reader") ? AccessEntry::Reader : AccessEntry::None;
        for (const collab::DeviceStatus &d : m_svc->statusModel()->devices())
            if (d.deviceId == ae.deviceId)
                ae.online = d.online;
        ae.mayHost = !a || a->membership().mayHost(ae.deviceId) || !a->membership().isMember(ae.deviceId);
        l << ae;
    }
    return l;
}

void ServiceControl::setAccess(const QString &uuid, const QString &deviceId, AccessEntry::Role role)
{
    m_svc->setAccess(uuid, deviceId, role == AccessEntry::Editor ? Role::Editor : role == AccessEntry::Reader ? Role::Reader : Role::None);
    emit accessChanged(uuid);
}

void ServiceControl::transferHosting(const QString &uuid, const QString &deviceId)
{
    QString why;
    if (!m_svc->transferHosting(uuid, deviceId, &why))
        emit m_svc->notice(tr("Hosting cannot be handed over now: %1").arg(why));
}

QList<DraftInfo> ServiceControl::drafts(const QString &uuid) const
{
    QList<DraftInfo> l;
    for (const Service::DraftEntry &e : m_svc->drafts(uuid)) {
        DraftInfo d;
        d.id = e.id;
        d.uuid = e.uuid;
        d.docId = e.docId;
        d.docTitle = e.docTitle.isEmpty() ? e.docId : e.docTitle;
        d.when = QDateTime::fromMSecsSinceEpoch(e.whenMs);
        d.reason = e.reason == QLatin1String("split") ? tr("made after the split") : tr("not sent before the takeover");
        l << d;
    }
    return l;
}

void ServiceControl::deleteDraft(const QString &id)
{
    if (!m_svc->deleteDraft(id))
        emit draftNote(tr("The draft could not be deleted."));
    emit draftsChanged();
}

// Compare, Keep as Copy and Take Over need the project open in a window of this program (the text goes in as a change of the
// shared project): the window that has it open carries the action out
void ServiceControl::draftAction(DraftAction::Kind kind, const QString &id)
{
    const Service::DraftContent c = m_svc->draftContent(id);
    if (!c.ok) {
        emit draftNote(c.why.isEmpty() ? tr("This draft cannot be used.") : c.why);
        return;
    }
    DraftAction a;
    a.kind = kind;
    a.uuid = c.uuid;
    a.docId = c.docId;
    a.docTitle = c.docTitle;
    a.text = c.text;
    a.rtf = c.rtf;
    emit draftActionRequested(&a);
    if (!a.handled)
        emit draftNote(tr("Open the project \"%1\" in a window first: the draft's text is taken into it from there.").arg(m_svc->statusModel()->project(c.uuid).title));
}

} // namespace livesync
} // namespace qb
