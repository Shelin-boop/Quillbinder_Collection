#pragma once
// Live Sync's service (0.99.0), section 4.1 of the design. One service runs per device: it holds the device's identity, the
// listener, discovery, pairing, the sessions of every shared project (a HostSession and a loopback ClientSession for a project
// this device hosts, a ClientSession with a replica for one it joined) and the registry. The windows of the programs never
// write the shared files: they hand their changes to the service through a local link (collab/ServiceProtocol.h) and show
// what the service tells them (collab/ServiceClient.h).
//
// On a desktop the service is the program itself started as `quillbinder --live-sync-service` (no window; a small status
// window while it keeps hosting after the last window closed), reached through a local socket named after the data folder. On
// Android and in the tests the same class runs inside the window's process: a window then attaches through a MemLink.
//
// Nothing is started, listened on or written until a project is shared or joined: with no registry entry the service is never
// started by a window, and the program behaves as it did before.
#include "core/collab/Authority.h"
#include "core/collab/ChangeUnits.h"
#include "core/collab/ClientSession.h"
#include "core/collab/Clock.h"
#include "core/collab/Drafts.h"
#include "core/collab/HostSession.h"
#include "core/collab/LocalLink.h"
#include "core/collab/Membership.h"
#include "core/collab/ProjectHostBackend.h"
#include "core/collab/Registry.h"
#include "core/collab/StatusModel.h"
#include "core/collab/net/Discovery.h"
#include "core/collab/net/MemLink.h"
#include "core/collab/net/Pairing.h"
#include "core/collab/net/TlsLink.h"
#include "core/crypto/Vault.h"

#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <memory>

namespace qb {
namespace collab {

class ServiceStatus;

class Service : public QObject {
    Q_OBJECT
public:
    struct Config {
        QString dir;                 // Live Sync's folder (the registry, peers, device.json, replicas); default: DeviceIdentity::standardDir()
        net::DeviceIdentity *identity = nullptr; // not owned; default: this device's own (loaded at start)
        QString deviceName;          // "" = the identity's
        Clock *clock = nullptr;      // not owned; default: a SystemClock of the service
        bool listen = true;          // the TLS listener
        QHostAddress address = QHostAddress::AnyIPv4;
        quint16 port = net::TlsServer::kPreferredPort; // 0 = any free port
        bool discovery = true;       // mDNS
        bool localSocket = true;     // the socket of the windows
        QString localName;           // "" = LocalLink::serverName(dir)
        bool quitWhenIdle = false;   // the service process: ends after the last window unless it keeps hosting
        int idleMs = 10000;          // ... after this many milliseconds
        QString replicasDir;         // default <dir>/replicas
    };
    explicit Service(const Config &config, QObject *parent = nullptr);
    ~Service() override;

    bool start(QString *error = nullptr);
    void stop();
    bool isRunning() const { return m_running; }

    // ---- this device
    net::DeviceIdentity *identity() const { return m_id; }
    net::PeerStore *peers() const { return m_peers.get(); }
    Registry *registry() const { return m_registry.get(); }
    Clock *clock() const { return m_clock; }
    quint16 port() const { return m_server ? m_server->port() : 0; }
    QString localName() const { return m_localName; }
    QString dir() const { return m_cfg.dir; }
    // the writer's choice "Keep hosting when the last window is closed"
    void setKeepHosting(bool on);
    bool keepHosting() const { return m_keepHosting; }

    // ---- pairing (the host opens its window, the other device pairs with it; both writers compare the code)
    bool openPairing(QString *why = nullptr);
    void closePairing();
    bool pairingOpen() { return m_pairWindow.isOpen(); }
    // the guest side: connects to a device whose window is open
    void pairWith(const QString &host, quint16 port);
    void confirmPairing(bool match);
    QString pairingCode() const { return m_pairCode; }

    // ---- hosting
    // makes the project in `folder` shared by this device; returns its uuid (empty with *error)
    QString shareProject(const QString &folder, QString *error = nullptr);
    // ends sharing of the project (the host) or leaves it (a client; the replica stays as an independent folder)
    bool stopSharing(const QString &uuid, QString *error = nullptr);
    // who may do what in a hosted project; Role::None removes the access
    void setAccess(const QString &uuid, const QString &deviceId, Role role);
    // the host's whole people list is read from the registry
    bool isHosting(const QString &uuid) const;
    HostSession *hostSession(const QString &uuid) const;
    ClientSession *clientSession(const QString &uuid) const; // also the loopback session of a hosted project

    // ---- joining
    // joins `uuid` hosted by the paired device `hostDeviceId`; the replica is made under the replicas folder. The address is where
    // to connect first (discovery fills in later). Returns the replica's folder (empty with *error).
    QString joinProject(const QString &hostDeviceId, const QString &uuid, const QString &title, const QString &address, quint16 port,
                        QString *error = nullptr);

    // ---- who hosts (package 6 through the service)
    // where a paired device can be reached when discovery does not know it (a new host after a handover or a failover)
    void setPeerAddress(const QString &deviceId, const QString &host, quint16 port);
    Authority *authority(const QString &uuid) const;
    // "Transfer Hosting...": false with a reason when it cannot start
    bool transferHosting(const QString &uuid, const QString &targetDeviceId, QString *why = nullptr);

    // ---- windows
    // a window's link is Ready (a LocalLink from the socket, or one end of a MemLink pair in the same process)
    void attachWindow(net::Link *link);
    int windowCount() const { return m_windows.size(); }

    // ---- status
    StatusModel *statusModel() const;
    StatusActions *statusActions() const;
    // the state of a project as the windows and the status model need it
    QJsonObject stateJson(const QString &uuid) const;
    int hostedCount() const;
    QStringList projectUuids() const { return m_projects.keys(); }

signals:
    void pairingCodeReady(const QString &code, const QString &peerName, bool hostSide);
    void pairingFinished(bool paired, const QString &reason);
    void projectsChanged();
    void projectChanged(const QString &uuid);
    void devicesChanged();
    void locksChanged(const QString &uuid);
    // nothing to do any more (no window for 10 s, not keeping the hosting): the service process ends
    void idle();
    void quitRequested();
    // a message line for the writer ("an unknown device tried ...", a hosting that could not start)
    void notice(const QString &text);

private:
    friend class ServiceStatus;
    friend class ServiceHosting;
    struct Proj;
    struct Window;
    struct PendingSubmit {
        QString id, uuid;
        QJsonArray units;
        QHash<QString, qint64> sizes;
        QHash<QString, QByteArray> data;
    };

    Proj *openProject(const RegistryEntry &entry);
    bool beginHost(Proj *p, qint64 epoch, QString *why);
    void failHost(Proj *p, const QString &why);
    void startClientPart(Proj *p);
    void makeAuthority(Proj *p);
    bool activateHost(Proj *p, qint64 epoch, QString *why);
    void retireHost(Proj *p);
    void followNewHost(Proj *p, const QString &hostDeviceId, qint64 epoch);
    bool authSend(const QString &uuid, const QString &deviceId, const QJsonObject &m);
    Membership membershipFor(const RegistryEntry &e) const;
    void publishAccess(Proj *p);
    void refreshHostInfo(Proj *p);
    void startHost(const RegistryEntry &entry);
    void startClient(const RegistryEntry &entry);
    void stopProject(const QString &uuid, bool removeEntry);
    void wireClient(Proj *p);
    void connectClient(Proj *p);
    void scheduleReconnect(Proj *p);
    void onTlsLink(net::TlsLink *link);
    void route(net::TlsLink *link, const net::Frame &first);
    void onWindowFrame(Window *w, const net::Frame &f);
    void onWindowMessage(Window *w, const QJsonObject &m);
    void onWindowClosed(Window *w);
    void windowAttach(Window *w, const QJsonObject &m);
    void windowSubmit(Window *w, const QJsonObject &m);
    void finishSubmit(Window *w, const QString &id);
    void sendWindow(Window *w, const QJsonObject &m);
    void tellWindows(const QString &uuid, const QJsonObject &m);
    void scheduleState(const QString &uuid);
    void emitState(const QString &uuid);
    void checkIdle();
    ShareState shareStateOf(const Proj &p) const;
    QString readOnlyReason(const Proj &p) const;
    void refreshDiscovery();
    QString replicaFolderFor(const QString &uuid, const QString &title) const;

    Config m_cfg;
    bool m_running = false;
    bool m_keepHosting = false;
    net::DeviceIdentity *m_id = nullptr;
    std::unique_ptr<net::DeviceIdentity> m_ownId;
    std::unique_ptr<net::PeerStore> m_peers;
    std::unique_ptr<Registry> m_registry;
    Clock *m_clock = nullptr;
    std::unique_ptr<Clock> m_ownClock;
    std::unique_ptr<Clock> m_scopedClock; // the clock of the sessions: ticks are delivered inside a ServiceScope
    std::unique_ptr<net::TlsServer> m_server;
    std::unique_ptr<Discovery> m_discovery;
    std::unique_ptr<LocalServer> m_local;
    QString m_localName;
    net::PairingWindow m_pairWindow;
    QPointer<net::PairingSession> m_pairing;
    QPointer<net::TlsLink> m_pairLink;
    QString m_pairCode;
    QHash<QString, Proj *> m_projects;
    QList<Window *> m_windows;
    QTimer m_idleTimer;
    QSet<QString> m_statePending;
    QSet<QString> m_asks; // "uuid|resource": an edit request of this device that is open
    QSet<QString> m_declined;
    QHash<QString, QPair<QString, quint16>> m_peerAddr; // device id -> where it was reached or told to be
    QTimer m_presence;                                  // once a second: the connected devices were heard of
    std::unique_ptr<ServiceStatus> m_status;
};

} // namespace collab
} // namespace qb
