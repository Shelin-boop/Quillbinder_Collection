// Live Sync, work package 10 (0.99.0): the networking behind the writer's failover rules. Checks: direct links between the members
// of a project (a member that never paired with the host is trusted for those links only; a link of a device that is no member is
// refused; a forged presence message is ignored), device names that reach sessions that exist, the lock table that goes along with
// a planned handover (the same lease, the new epoch), the lock grace after a failover, and real programs (three or four service
// processes with their own data folders on localhost): a failover with three devices, a partition 2:1 with the old host in the
// minority, then the reunion (its edits after the split become recovery drafts) and the handback, and a new host's address that
// reaches a client that has no discovery.
//
//   QB_SAMPLE=0 QB_SUITES=ls10 tools/run-all-tests.sh build/test-results-x11 x11       (or ./qb_ls10_tests /tmp/shots)
//   QB_LS10_PART=<mesh|names|locks|failover|partition> runs one part. Exit code = number of failed checks.
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/collab/Authority.h"
#include "core/collab/Clock.h"
#include "core/collab/Drafts.h"
#include "core/collab/LocalLink.h"
#include "core/collab/Manifest.h"
#include "core/collab/Registry.h"
#include "core/collab/Service.h"
#include "core/collab/ServiceClient.h"
#include "core/collab/StatusModel.h"
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/net/MemLink.h"
#include "core/collab/net/PeerStore.h"
#include "core/collab/net/TlsLink.h"
#include "core/collab/proto/Messages.h"
#include "core/crypto/Vault.h"

#include <QCoreApplication>
#include <QTextDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDir>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QProcess>
#include <QSslCertificate>
#include <QSslKey>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QTimer>
#include <QThread>

#include <cstdio>
#include <memory>

using namespace qb;
using namespace qb::collab;
using qb::collab::net::TlsLink;

static int failures = 0;
static int total = 0;
static void check(bool ok, const QString &what, const QString &detail = QString())
{
    ++total;
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static bool waitFor(const std::function<bool()> &cond, int ms = 8000)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        if (cond())
            return true;
        QEventLoop l;
        QTimer::singleShot(10, &l, &QEventLoop::quit);
        l.exec();
    }
    return cond();
}

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

static bool writeAll(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    return safesave::writeFile(path, data, nullptr);
}

static QString treeHash(const QString &folder)
{
    Manifest m;
    Manifest::scan(folder, QString(), {}, &m);
    return m.stateHash();
}

// the paths whose bytes differ between two folders (replicated files only), for a failing check's detail
static QString treeDiff(const QString &a, const QString &b)
{
    Manifest ma, mb;
    Manifest::scan(a, QString(), {}, &ma);
    Manifest::scan(b, QString(), {}, &mb);
    const ManifestDiff d = compare(ma, mb);
    return QStringLiteral("fetch=[%1] remove=[%2]").arg(d.fetch.join(','), d.remove.join(','));
}

struct Made {
    QString folder, ch1, ch2, ch3;
};

// a project with three chapters, saved and closed
static Made makeProject(const QString &parent, const QString &name)
{
    Made m;
    QString err;
    auto p = Project::create(parent, name, &err);
    if (!p)
        return m;
    BinderModel *model = p->model();
    BinderItem *a = model->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter One"));
    BinderItem *b = model->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter Two"));
    BinderItem *c = model->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter Three"));
    m.ch1 = a->id;
    m.ch2 = b->id;
    m.ch3 = c->id;
    p->docs()->text(a->id)->setPlainText(QStringLiteral("The first chapter has some words in it."));
    p->docs()->text(b->id)->setPlainText(QStringLiteral("The second chapter."));
    p->docs()->text(c->id)->setPlainText(QStringLiteral("The third."));
    p->save(&err);
    m.folder = p->folderPath();
    return m;
}

// the word count the project file of `folder` holds for an item (-1: no such item)
static int wordsOfItem(const QString &folder, const QString &id)
{
    const QString file = Project::resolveProjectFile(folder);
    QByteArray raw;
    if (file.isEmpty() || !vault::readFile(file, &raw))
        return -1;
    QDomDocument dom;
    if (!dom.setContent(raw))
        return -1;
    const QDomNodeList items = dom.elementsByTagName(QStringLiteral("Item"));
    for (int i = 0; i < items.size(); ++i) {
        const QDomElement e = items.at(i).toElement();
        if (e.attribute(QStringLiteral("id")) == id)
            return e.attribute(QStringLiteral("words"), QStringLiteral("0")).toInt();
    }
    return -1;
}

struct Dev {
    QString name, dir;
    std::unique_ptr<net::DeviceIdentity> id;
    std::unique_ptr<Service> svc;
    std::unique_ptr<ManualClock> clock; // only when the test moves time itself
    QString deviceId() const { return id->deviceId(); }
};

static std::unique_ptr<Dev> makeDev(const QString &tmp, const QString &name, bool localSocket = false, bool manualClock = false)
{
    auto d = std::make_unique<Dev>();
    d->name = name;
    d->dir = tmp + "/" + name;
    d->id = net::DeviceIdentity::createEphemeral(name);
    Service::Config cfg;
    cfg.dir = d->dir;
    cfg.identity = d->id.get();
    cfg.deviceName = name;
    cfg.address = QHostAddress::LocalHost;
    cfg.port = 0;
    cfg.discovery = false;
    cfg.localSocket = localSocket;
    if (manualClock) {
        d->clock = std::make_unique<ManualClock>();
        cfg.clock = d->clock.get();
    }
    if (localSocket)
        cfg.localName = "qb-ls10-" + name + "-" + QString::number(QCoreApplication::applicationPid());
    d->svc = std::make_unique<Service>(cfg);
    QString err;
    if (!d->svc->start(&err))
        std::printf("      service %s: %s\n", qPrintable(name), qPrintable(err));
    return d;
}

static bool pair(Dev &host, Dev &guest, QString *codeA = nullptr, QString *codeB = nullptr)
{
    QString ca, cb;
    bool fh = false, fg = false, okh = false, okg = false;
    QList<QMetaObject::Connection> cons;
    cons << QObject::connect(host.svc.get(), &Service::pairingCodeReady, host.svc.get(), [&](const QString &c, const QString &, bool) { ca = c; });
    cons << QObject::connect(guest.svc.get(), &Service::pairingCodeReady, guest.svc.get(), [&](const QString &c, const QString &, bool) { cb = c; });
    cons << QObject::connect(host.svc.get(), &Service::pairingFinished, host.svc.get(), [&](bool ok, const QString &) { fh = true; okh = ok; });
    cons << QObject::connect(guest.svc.get(), &Service::pairingFinished, guest.svc.get(), [&](bool ok, const QString &) { fg = true; okg = ok; });
    auto done = [&](bool r) {
        for (const QMetaObject::Connection &c : cons)
            QObject::disconnect(c);
        return r;
    };
    QString whyOpen;
    if (!host.svc->openPairing(&whyOpen)) {
        std::printf("      openPairing: %s\n", qPrintable(whyOpen));
        return done(false);
    }
    guest.svc->pairWith("127.0.0.1", host.svc->port());
    if (!waitFor([&] { return !ca.isEmpty() && !cb.isEmpty(); })) {
        std::printf("      no codes: '%s' '%s'\n", qPrintable(ca), qPrintable(cb));
        return done(false);
    }
    if (codeA)
        *codeA = ca;
    if (codeB)
        *codeB = cb;
    host.svc->confirmPairing(ca == cb);
    guest.svc->confirmPairing(ca == cb);
    waitFor([&] { return fh && fg; });
    if (!(okh && okg))
        std::printf("      finished: %d %d / %d %d\n", fh, okh, fg, okg);
    return done(okh && okg);
}

// a window of a service in this process: a follower project and its client
struct Win {
    std::unique_ptr<ServiceClient> client;
    std::unique_ptr<Project> project;
    QString error;
    bool open(Service &svc, const QString &folder)
    {
        net::MemLink::Pair pr = net::MemLink::createPair(QByteArray(32, 'w'), QByteArray(32, 's'), nullptr);
        svc.attachWindow(pr.a);
        client = std::make_unique<ServiceClient>(pr.b);
        const ServiceClient::Attached a = client->attach(folder, false);
        if (!a.ok) {
            error = a.code;
            return false;
        }
        project = Project::open(folder, &error, client->follower());
        if (!project)
            return false;
        client->follower()->capture().setListener([c = client.get()] { QMetaObject::invokeMethod(c, [c] { c->sendCaptured(); }, Qt::QueuedConnection); });
        return true;
    }
    bool lock(const QString &docId)
    {
        client->lock({"doc:" + docId});
        return waitFor([&] { return client->holds("doc:" + docId); });
    }
    bool setText(const QString &docId, const QString &text)
    {
        project->docs()->text(docId)->setPlainText(text);
        QString err;
        return project->save(&err);
    }
};


namespace {
struct ProcDev {
    QString name, xdg, lsdir, keys, id;
    quint16 port = 0;
    QByteArray spki, cert;
    QString instance;
    std::unique_ptr<QProcess> proc;
};
quint16 freePort()
{
    QTcpServer s;
    s.listen(QHostAddress::LocalHost, 0);
    return s.serverPort();
}
void startProc(ProcDev &d, const QString &app, int idleMs, const QString &peers)
{
    d.proc = std::make_unique<QProcess>();
    d.proc->setProcessChannelMode(QProcess::ForwardedChannels);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("XDG_DATA_HOME", d.xdg);
    env.insert("QB_TEST_KEYSTORE", "file:" + d.keys);
    env.insert("QT_QPA_PLATFORM", "offscreen");
    env.insert("QB_LS_PORT", QString::number(d.port));
    env.insert("QB_LS_NO_DISCOVERY", "1");
    env.insert("QB_LS_IDLE_MS", QString::number(idleMs));
    env.insert("QB_NO_UPDATE_CHECK", "1");
    env.insert("QB_LS_PEERS", peers);
    env.insert("QB_LS_BLOCK", "1");
    if (qEnvironmentVariableIsSet("QB_LS10_TRACE"))
        env.insert("QB_LS_TRACE", "1");
    d.proc->setProcessEnvironment(env);
    d.proc->start(app, {"--live-sync-service"});
}
} // namespace

// ---------------------------------------------------------------- in-process: three devices on localhost

struct Trio {
    std::unique_ptr<Dev> H, B, C;
    QString uuid, folder, replicaB, replicaC;
    Made m;
};

// H hosts a shared project; B and C joined; B and C never paired with each other, only with H
static bool makeTrio(const QString &tmp, Trio &t)
{
    t.H = makeDev(tmp, "Hosty");
    t.B = makeDev(tmp, "Bee");
    t.C = makeDev(tmp, "Cee");
    const bool p1 = pair(*t.H, *t.B);
    waitFor([] { return false; }, 800);
    t.H->svc->closePairing();
    const bool p2 = p1 && pair(*t.H, *t.C);
    if (!p2) {
        std::printf("      pairing failed: first=%d second=%d running=%d,%d,%d port=%d\n", p1, p2, t.H->svc->isRunning(), t.B->svc->isRunning(), t.C->svc->isRunning(), int(t.H->svc->port()));
        return false;
    }
    t.m = makeProject(tmp + "/trioproj", "Trio");
    t.folder = t.m.folder;
    QString err;
    t.uuid = t.H->svc->shareProject(t.folder, &err);
    if (t.uuid.isEmpty()) {
        std::printf("      share failed: %s\n", qPrintable(err));
        return false;
    }
    waitFor([&] { return t.H->svc->clientSession(t.uuid) && t.H->svc->clientSession(t.uuid)->state() == ClientSession::State::Ready; });
    t.H->svc->setAccess(t.uuid, t.B->deviceId(), Role::Editor);
    t.H->svc->setAccess(t.uuid, t.C->deviceId(), Role::Editor);
    t.replicaB = t.B->svc->joinProject(t.H->deviceId(), t.uuid, "Trio", "127.0.0.1", t.H->svc->port(), &err);
    t.replicaC = t.C->svc->joinProject(t.H->deviceId(), t.uuid, "Trio", "127.0.0.1", t.H->svc->port(), &err);
    auto ready = [&](Dev &d) {
        return waitFor([&] { return d.svc->clientSession(t.uuid) && d.svc->clientSession(t.uuid)->state() == ClientSession::State::Ready && d.svc->clientSession(t.uuid)->replicaReady(); }, 30000);
    };
    const bool rb = ready(*t.B), rc = ready(*t.C);
    if (!rb || !rc)
        std::printf("      not ready: B=%d C=%d %s\n", rb, rc, qPrintable(err));
    return rb && rc;
}

static void testMesh(const QString &tmp)
{
    std::printf("== direct links between the members of a project\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the service checks are skipped)"));
        return;
    }
    Trio t;
    check(makeTrio(tmp + "/mesh", t), QStringLiteral("three devices share a project: the host and two that paired with the host only"));
    if (!t.H || t.uuid.isEmpty())
        return;
    // B and C find each other without discovery and without pairing: the host passes on their signed addresses
    check(waitFor([&] { return t.B->svc->directLinks(t.uuid).contains(t.C->deviceId()) && t.C->svc->directLinks(t.uuid).contains(t.B->deviceId()); }, 20000),
          QStringLiteral("the two clients get a direct link to each other (trusted through the signed list of devices)"),
          t.B->svc->directLinks(t.uuid).join(',') + " / " + t.C->svc->directLinks(t.uuid).join(','));
    check(!t.B->svc->peers()->contains(QByteArray::fromHex(t.C->id->pin().toHex())) && t.B->svc->peers()->peers().size() == 1,
          QStringLiteral("... though B never paired with C"));
    check(t.B->svc->authority(t.uuid)->membership().count() == 3 && t.C->svc->authority(t.uuid)->membership().count() == 3,
          QStringLiteral("each client holds the signed list of all three devices"));
    // the addresses come from signed presence messages
    check(waitFor([&] { return !t.B->svc->addressesOf(t.C->deviceId()).isEmpty() && t.B->svc->addressesOf(t.C->deviceId()).first().second == t.C->svc->port(); }, 8000),
          QStringLiteral("B knows C's address and port from C's signed presence message"));

    // a device that is no member: its connection is refused
    {
        auto X = makeDev(tmp + "/mesh", "Xavier");
        TlsLink *l = TlsLink::connectTo("127.0.0.1", t.C->svc->port(), X->id.get(), net::TrustPolicy::pinned(t.C->id->pin()));
        const bool ready = waitFor([&] { return l->state() == net::Link::State::Ready || l->state() == net::Link::State::Closed; }, 5000) && l->state() == net::Link::State::Ready;
        bool answered = false;
        if (ready) {
            l->send(net::Frame::json(proto::make(proto::T::PRESENCE, t.uuid, {{"rec", QJsonObject{{"device", X->deviceId()}}}})));
            waitFor([&] { return l->state() == net::Link::State::Closed; }, 3000);
            answered = l->state() == net::Link::State::Closed;
        }
        check(!ready || answered, QStringLiteral("a device that is not in the project's list gets no direct link (closed at once)"));
        check(t.C->svc->directLinks(t.uuid).size() <= 2 && !t.C->svc->directLinks(t.uuid).contains(X->deviceId()), QStringLiteral("... and is not listed among the direct links"));
        delete l;
    }
    // B (member trust at C) may not ask C for its list of projects, and a forged presence message changes nothing
    {
        TlsLink *l = TlsLink::connectTo("127.0.0.1", t.C->svc->port(), t.B->id.get(), net::TrustPolicy::pinned(t.C->id->pin()));
        QStringList got;
        QObject::connect(l, &net::Link::frameReceived, l, [&](const net::Frame &f) { bool ok = false; got << f.object(&ok).value("code").toString(); });
        waitFor([&] { return l->state() == net::Link::State::Ready; }, 5000);
        // (B has a paired record at H only: at C this link is a member's)
        l->send(net::Frame::json(proto::make(proto::T::PROJECT_LIST)));
        waitFor([&] { return !got.isEmpty() || l->state() == net::Link::State::Closed; }, 3000);
        check(got.contains(QStringLiteral("NOT_PAIRED")) || l->state() == net::Link::State::Closed, QStringLiteral("a member that is not paired with a device cannot ask it for its projects"), got.join(','));
        delete l;
    }
    {
        const auto before = t.C->svc->addressesOf(t.B->deviceId());
        // C's own link to B: an attacker with C's key is not needed, B signs wrongly on purpose
        QJsonObject rec{{"device", t.B->deviceId()}, {"name", "Bee"}, {"port", 9}, {"addrs", QJsonArray{"10.9.9.9"}}, {"at", double(QDateTime::currentSecsSinceEpoch() + 5000)}, {"sig", "AAAA"}};
        TlsLink *l = TlsLink::connectTo("127.0.0.1", t.C->svc->port(), t.B->id.get(), net::TrustPolicy::pinned(t.C->id->pin()));
        waitFor([&] { return l->state() == net::Link::State::Ready; }, 5000);
        l->send(net::Frame::json(proto::make(proto::T::PRESENCE, t.uuid, {{"rec", rec}})));
        QThread::msleep(500);
        waitFor([&] { return false; }, 500);
        const auto after = t.C->svc->addressesOf(t.B->deviceId());
        bool bogus = false;
        for (const auto &a : after)
            bogus = bogus || a.first == QLatin1String("10.9.9.9") || a.second == 9;
        check(!bogus && after.size() >= before.size(), QStringLiteral("a presence message with a wrong signature teaches no address"));
        delete l;
    }
    // the direct links carry what votes need: with the host gone C hears B's chain at once
    t.H->svc->stop();
    check(waitFor([&] { return t.B->svc->directLinks(t.uuid).contains(t.C->deviceId()); }, 3000), QStringLiteral("the direct link between the clients stays when the host goes"));
}

static void testNames(const QString &tmp)
{
    std::printf("== device names reach sessions that exist\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available())
        return;
    Trio t;
    if (!makeTrio(tmp + "/names", t)) {
        check(false, QStringLiteral("three devices share a project"));
        return;
    }
    t.B->svc->clientSession(t.uuid)->lock({"doc:" + t.m.ch1});
    check(waitFor([&] { return t.B->svc->clientSession(t.uuid)->heldLocks().contains("doc:" + t.m.ch1); }), QStringLiteral("B holds a lock"));
    check(waitFor([&] { return t.C->svc->statusModel()->lockInfo(t.uuid, "doc:" + t.m.ch1).holderName == "Bee"; }, 6000), QStringLiteral("C sees the lock held by Bee"));
    t.B->svc->setDeviceName(QStringLiteral("Bumblebee"));
    check(waitFor([&] {
              for (const HostSession::PeerSummary &s : t.H->svc->hostSession(t.uuid)->joinedPeers())
                  if (s.deviceId == t.B->deviceId())
                      return s.name == "Bumblebee";
              return false;
          }, 8000),
          QStringLiteral("the host's open session of B shows the new name"));
    check(waitFor([&] { return t.C->svc->statusModel()->lockInfo(t.uuid, "doc:" + t.m.ch1).holderName == "Bumblebee"; }, 8000),
          QStringLiteral("the lock B holds shows the new name on C without a reconnect"));
    check(waitFor([&] {
              const Member *mb = t.C->svc->authority(t.uuid)->membership().find(t.B->deviceId());
              return mb && mb->name == "Bumblebee";
          }, 8000),
          QStringLiteral("the signed list of devices carries it to C"));
    t.H->svc->setDeviceName(QStringLiteral("Hostess"));
    check(waitFor([&] { return t.C->svc->statusModel()->project(t.uuid).hostName == "Hostess"; }, 8000), QStringLiteral("the host's new name reaches a client's session"));
    check(waitFor([&] { return t.B->svc->statusModel()->project(t.uuid).hostName == "Hostess"; }, 8000), QStringLiteral("... and the other client's"));
}

static void testLocks(const QString &tmp)
{
    std::printf("== locks across a hand-over and a failover\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available())
        return;
    Trio t;
    if (!makeTrio(tmp + "/locks", t)) {
        check(false, QStringLiteral("three devices share a project"));
        return;
    }
    check(waitFor([&] { return t.B->svc->directLinks(t.uuid).contains(t.C->deviceId()) && t.C->svc->directLinks(t.uuid).contains(t.B->deviceId()); }, 20000),
          QStringLiteral("the clients have found each other"));
    ClientSession *cb = t.B->svc->clientSession(t.uuid), *cc = t.C->svc->clientSession(t.uuid), *ch = t.H->svc->clientSession(t.uuid);
    cb->lock({"doc:" + t.m.ch1});
    cc->lock({"doc:" + t.m.ch2});
    ch->lock({"doc:" + t.m.ch3});
    check(waitFor([&] { return cb->holds("doc:" + t.m.ch1) && cc->holds("doc:" + t.m.ch2) && ch->holds("doc:" + t.m.ch3); }), QStringLiteral("the host's device and both clients hold a lock each"));
    const QString lb = cb->token("doc:" + t.m.ch1).lease, lc = cc->token("doc:" + t.m.ch2).lease, lh = ch->token("doc:" + t.m.ch3).lease;
    QString why;
    check(t.H->svc->transferHosting(t.uuid, t.C->deviceId(), &why), QStringLiteral("the host starts a planned hand-over to C"), why);
    check(waitFor([&] { return t.C->svc->isHosting(t.uuid) && !t.H->svc->isHosting(t.uuid) && t.C->svc->authority(t.uuid)->epoch() == 2; }, 40000), QStringLiteral("C hosts in epoch 2"));
    check(waitFor([&] {
              ClientSession *b = t.B->svc->clientSession(t.uuid), *h = t.H->svc->clientSession(t.uuid);
              return b && h && t.C->svc->clientSession(t.uuid) && b->state() == ClientSession::State::Ready && h->state() == ClientSession::State::Ready && b->epoch() == 2 && h->epoch() == 2;
          }, 40000),
          QStringLiteral("the other devices follow the new host"),
          QStringLiteral("B state=%1 epoch=%2 host=%3 problem=%4 direct=%5").arg(int(t.B->svc->clientSession(t.uuid) ? t.B->svc->clientSession(t.uuid)->state() : ClientSession::State::Failed)).arg(t.B->svc->clientSession(t.uuid) ? t.B->svc->clientSession(t.uuid)->epoch() : -1).arg(t.B->svc->statusModel()->project(t.uuid).hostName, t.B->svc->statusModel()->project(t.uuid).problemText, t.B->svc->directLinks(t.uuid).join(',')));
    cb = t.B->svc->clientSession(t.uuid);
    ch = t.H->svc->clientSession(t.uuid);
    cc = t.C->svc->clientSession(t.uuid);
    HostSession *hs = t.C->svc->hostSession(t.uuid);
    const Lock *k1 = hs ? hs->locks().find("doc:" + t.m.ch1) : nullptr;
    const Lock *k2 = hs ? hs->locks().find("doc:" + t.m.ch2) : nullptr;
    const Lock *k3 = hs ? hs->locks().find("doc:" + t.m.ch3) : nullptr;
    check(k1 && k2 && k3, QStringLiteral("the new host's lock table holds all three locks"));
    check(k1 && k1->lease == lb && k1->epoch == 2 && k1->owner.device == t.B->deviceId(), QStringLiteral("B's lock: same lease, epoch 2, still B's"));
    check(k2 && k2->lease == lc && k2->owner.device == t.C->deviceId(), QStringLiteral("the lock of the device that became the host: same lease"));
    check(k3 && k3->lease == lh && k3->owner.device == t.H->deviceId(), QStringLiteral("the old host's own lock: same lease, now its lock as a client"));
    check(waitFor([&] { return cb->holds("doc:" + t.m.ch1) && cb->token("doc:" + t.m.ch1).lease == lb && cb->token("doc:" + t.m.ch1).epoch == 2; }, 8000),
          QStringLiteral("B's session holds the lock with the new epoch's token (no new request was needed)"));
    check(waitFor([&] { return ch->holds("doc:" + t.m.ch3) && ch->token("doc:" + t.m.ch3).lease == lh; }, 8000), QStringLiteral("the old host's session too"));

    // after a failover the table starts empty; the first lock request of a device that was not editing waits for the ones coming back
    HostSession *host = t.C->svc->hostSession(t.uuid);
    host->setLockGrace(4000);
    QStringList denied;
    QObject::connect(cb, &ClientSession::lockDenied, cb, [&](const QString &r, const QString &, const QString &reason) { denied << r + "|" + reason; });
    cb->lock({"doc:" + t.m.ch2 + "x"});
    cb->lock({"settings"});
    check(waitFor([&] { return !denied.isEmpty(); }, 3000) && denied.first().endsWith("HOST_BUSY"), QStringLiteral("in the grace time a new lock request is answered HOST_BUSY"), denied.join(","));
    check(!cb->holds("settings"), QStringLiteral("... and nothing was granted"));
    check(waitFor([&] { cb->lock({"settings"}); return cb->holds("settings"); }, 9000), QStringLiteral("after the grace time it is granted"));
}

// ---------------------------------------------------------------- processes

struct Cluster {
    QString tmp, app;
    std::vector<std::unique_ptr<ProcDev>> dev; // A (host), B, C ...
    Made m;
    QString uuid, replicaB, replicaC;
    ProcDev &operator[](int i) { return *dev[i]; }
    QString replicaOf(int i) { return i == 1 ? replicaB : replicaC; }
};

static QString nameOf(const ProcDev &d) { return LocalLink::serverName(d.lsdir); }

static void identity(ProcDev &d, const QString &app)
{
    startProc(d, app, 300, QString());
    d.proc->waitForFinished(30000);
    QFile f(d.lsdir + "/device.json");
    QJsonObject o;
    if (f.open(QIODevice::ReadOnly))
        o = QJsonDocument::fromJson(f.readAll()).object();
    d.cert = QByteArray::fromBase64(o.value("cert").toString().toLatin1());
    d.instance = o.value("instance").toString();
    d.name = o.value("name").toString().isEmpty() ? d.name : o.value("name").toString();
    d.spki = QSslCertificate(d.cert, QSsl::Der).publicKey().toDer();
    d.id = net::DeviceIdentity::deviceIdOfPin(net::DeviceIdentity::pinOfPublicKey(d.spki));
}

// three programs: A hosts, B and C joined; they paired with A only. No address is given to anyone except A's, which B and C have
static bool makeCluster(const QString &tmp, const QString &app, const QString &title, Cluster &c)
{
    c.tmp = tmp;
    c.app = app;
    const char *names[] = {"ProcA", "ProcB", "ProcC"};
    for (int i = 0; i < 3; ++i) {
        auto d = std::make_unique<ProcDev>();
        d->name = names[i];
        d->xdg = tmp + "/xdg-" + d->name;
        d->lsdir = d->xdg + "/Quillbinder/livesync";
        d->keys = tmp + "/xdg-" + d->name + "-keys.json";
        QDir().mkpath(d->lsdir);
        d->port = freePort();
        identity(*d, app);
        c.dev.push_back(std::move(d));
    }
    ProcDev &A = c[0];
    if (A.id.isEmpty() || c[1].id.isEmpty() || c[2].id.isEmpty())
        return false;
    for (int i = 0; i < 3; ++i) {
        net::PeerStore ps(c[i].lsdir);
        for (int j = 0; j < 3; ++j) {
            if (i == j || (i != 0 && j != 0)) // (the hub: A knows B and C, B and C know A)
                continue;
            net::PeerRecord r;
            r.pin = net::DeviceIdentity::pinOfPublicKey(c[j].spki);
            r.publicKey = c[j].spki;
            r.name = c[j].name;
            r.instanceId = c[j].instance;
            r.pairedAt = 1700000000 + j;
            ps.add(r);
        }
    }
    c.m = makeProject(tmp + "/proj", title);
    c.uuid = Registry::newUuid();
    c.replicaB = tmp + "/replica-b/" + title + ".quillbinder";
    c.replicaC = tmp + "/replica-c/" + title + ".quillbinder";
    QDir().mkpath(c.replicaB);
    QDir().mkpath(c.replicaC);
    {
        Registry ra(A.lsdir);
        RegistryEntry e;
        e.uuid = c.uuid;
        e.folder = c.m.folder;
        e.title = title;
        e.host = true;
        e.hostDeviceId = A.id;
        e.hostName = A.name;
        e.epoch = 1;
        e.access.insert(c[1].id, "editor");
        e.access.insert(c[2].id, "editor");
        ra.upsert(e);
        for (int i = 1; i < 3; ++i) {
            Registry rb(c[i].lsdir);
            RegistryEntry r;
            r.uuid = c.uuid;
            r.folder = c.replicaOf(i);
            r.title = title;
            r.host = false;
            r.hostDeviceId = A.id;
            r.hostName = A.name;
            r.lastAddress = "127.0.0.1";
            r.lastPort = A.port;
            rb.upsert(r);
        }
    }
    for (int i = 0; i < 3; ++i)
        startProc(c[i], app, 600000, QString());
    for (int i = 0; i < 3; ++i)
        if (!waitFor([&, i] { return LocalServer::isServed(nameOf(c[i]), 100); }, 30000))
            return false;
    return true;
}

static void stopCluster(Cluster &c)
{
    for (auto &d : c.dev)
        if (d->proc && d->proc->state() != QProcess::NotRunning) {
            d->proc->kill();
            d->proc->waitForFinished(5000);
        }
}

struct Win10 {
    LocalLink *link = nullptr;
    std::unique_ptr<ServiceClient> c;
    bool open(ProcDev &d, const QString &folder, ServiceClient::Attached *at = nullptr)
    {
        link = LocalLink::connectTo(nameOf(d));
        waitFor([&] { return link->state() == net::Link::State::Ready; }, 3000);
        c = std::make_unique<ServiceClient>(link);
        const ServiceClient::Attached a = c->attach(folder, false);
        if (at)
            *at = a;
        return a.ok;
    }
};

// a window of device `w` types `text` into a document of its folder, under a lock
static bool typeInto(Win10 &w, const QString &folder, const QString &docId, const QString &text)
{
    QString err;
    auto p = Project::open(folder, &err, w.c->follower());
    if (!p)
        return false;
    w.c->lock({"doc:" + docId});
    if (!waitFor([&] { return w.c->holds("doc:" + docId); }, 15000))
        return false;
    w.c->follower()->capture().setListener([c = w.c.get()] { QMetaObject::invokeMethod(c, [c] { c->sendCaptured(); }, Qt::QueuedConnection); });
    p->docs()->text(docId)->setPlainText(text);
    const bool ok = p->save(&err);
    waitFor([&] { return !w.c->busy(); }, 15000);
    return ok;
}

static void testFailover(const QString &tmp, const QString &app)
{
    std::printf("== three programs: the host is killed\n");
    if (app.isEmpty() || !QFileInfo::exists(app) || !net::DeviceIdentity::available()) {
        check(true, QStringLiteral("(the program's file or the TLS pack is not here: the process checks are skipped)"));
        return;
    }
    Cluster c;
    check(makeCluster(tmp + "/fo", app, "Failover", c), QStringLiteral("three service processes run, each with its own data folder"));
    if (c.dev.size() < 3)
        return;
    ProcDev &A = c[0], &B = c[1], &C = c[2];
    Win10 wa, wb, wc;
    ServiceClient::Attached aa, ab, ac;
    check(wa.open(A, c.m.folder, &aa) && wb.open(B, c.replicaB, &ab) && wc.open(C, c.replicaC, &ac) && aa.host && !ab.host && !ac.host,
          QStringLiteral("a window of each: one hosts, two hold copies"), aa.code + ab.code + ac.code);
    check(waitFor([&] { return wb.c->replicaReady() && wc.c->replicaReady() && treeHash(c.replicaB) == treeHash(c.m.folder) && treeHash(c.replicaC) == treeHash(c.m.folder); }, 60000),
          QStringLiteral("both copies are complete"));
    // the clients do not know B's address; they have no discovery
    {
        Registry rc(C.lsdir);
        RegistryEntry e;
        rc.find(c.uuid, &e);
        check(e.lastPort == A.port, QStringLiteral("C knows only the host's address"));
    }
    wc.c->lock({"doc:" + c.m.ch1});
    check(waitFor([&] { return wc.c->holds("doc:" + c.m.ch1); }, 10000), QStringLiteral("C holds a lock when the host dies"));
    const QString lease0 = wc.c->heldLease("doc:" + c.m.ch1);
    waitFor([] { return false; }, 12000); // (the signed addresses pass round)
    wa.c->detach();
    wa.link->close();
    A.proc->kill();
    A.proc->waitForFinished(5000);
    QElapsedTimer since;
    since.start();
    check(waitFor([&] { return wb.c->isHost() && wb.c->epoch() == 2; }, 100000), QStringLiteral("the device next by priority hosts in epoch 2: it got the vote of the third device (no connection through the lost host)"),
          QStringLiteral("B host=%1 epoch=%2; C host=%3 epoch=%4 hostId=%5").arg(wb.c->isHost()).arg(wb.c->epoch()).arg(wc.c->isHost()).arg(wc.c->epoch()).arg(wc.c->hostDeviceId()));
    check(since.elapsed() >= 24000, QStringLiteral("... not before the host was out of contact for about half a minute"), QString::number(since.elapsed()));
    check(waitFor([&] { return wc.c->hostDeviceId() == B.id && wc.c->epoch() == 2 && wc.c->replicaReady() && wc.c->stateCode() == int(ShareState::UpToDate); }, 60000),
          QStringLiteral("the third device follows the new host"), QStringLiteral("host=%1 epoch=%2 state=%3").arg(wc.c->hostDeviceId()).arg(wc.c->epoch()).arg(wc.c->stateCode()));
    {
        Registry rc(C.lsdir);
        RegistryEntry e;
        rc.find(c.uuid, &e);
        check(e.lastAddress == "127.0.0.1" && e.lastPort == B.port, QStringLiteral("it found the new host by the address in the signed presence message (no discovery, no address given)"),
              e.lastAddress + ":" + QString::number(e.lastPort));
    }
    check(waitFor([&] { return wc.c->holds("doc:" + c.m.ch1) && wc.c->heldEpoch("doc:" + c.m.ch1) == 2; }, 40000), QStringLiteral("C asked the new host for its lock again and holds it in epoch 2"));
    check(wc.c->heldLease("doc:" + c.m.ch1) != lease0, QStringLiteral("(a new lock: the table of a failover starts empty)"));
    check(typeInto(wc, c.replicaC, c.m.ch1, QStringLiteral("Written on the third device after the failover.")) &&
              waitFor([&] { return readAll(c.replicaB + "/content/" + c.m.ch1 + "/text.md").contains("after the failover"); }, 20000),
          QStringLiteral("a text typed under that lock reaches the new host's folder"));
    stopCluster(c);
}

static void testPartition(const QString &tmp, const QString &app)
{
    std::printf("== three programs: a partition with the old host alone, then the reunion and the handback\n");
    if (app.isEmpty() || !QFileInfo::exists(app) || !net::DeviceIdentity::available()) {
        check(true, QStringLiteral("(the program's file or the TLS pack is not here: the process checks are skipped)"));
        return;
    }
    Cluster c;
    check(makeCluster(tmp + "/pa", app, "Partition", c), QStringLiteral("three service processes run"));
    if (c.dev.size() < 3)
        return;
    ProcDev &A = c[0], &B = c[1], &C = c[2];
    Win10 wa, wb, wc;
    check(wa.open(A, c.m.folder) && wb.open(B, c.replicaB) && wc.open(C, c.replicaC), QStringLiteral("a window of each"));
    check(waitFor([&] { return wb.c->replicaReady() && wc.c->replicaReady() && treeHash(c.replicaB) == treeHash(c.m.folder) && treeHash(c.replicaC) == treeHash(c.m.folder); }, 60000),
          QStringLiteral("both copies are complete"));
    waitFor([] { return false; }, 12000);
    auto block = [&](bool on) {
        const QStringList forA = {B.id + "\n" + C.id + "\n", A.id + "\n"};
        for (int i = 0; i < 3; ++i) {
            QFile f(c[i].lsdir + "/ls-block.txt");
            if (on && f.open(QIODevice::WriteOnly | QIODevice::Truncate))
                f.write(forA.at(i == 0 ? 0 : 1).toUtf8());
            else if (!on)
                QFile::remove(f.fileName());
        }
    };
    block(true);
    QElapsedTimer since;
    since.start();
    // the old host keeps hosting its side; its window edits after the split
    check(waitFor([&] { return wa.c->isHost() && wb.c->stateCode() != int(ShareState::UpToDate); }, 20000), QStringLiteral("the cut: A still hosts, B is not connected to it"));
    check(typeInto(wa, c.m.folder, c.m.ch2, QStringLiteral("Typed on the old host after the split.")) &&
              waitFor([&] { return readAll(c.m.folder + "/content/" + c.m.ch2 + "/text.md").contains("old host after the split"); }, 20000),
          QStringLiteral("the old host still accepts its own window's edit"));
    check(waitFor([&] { return wb.c->isHost() && wb.c->epoch() == 2; }, 100000), QStringLiteral("the majority (B and C) elects B: epoch 2"));
    check(since.elapsed() >= 24000, QStringLiteral("... after about half a minute"), QString::number(since.elapsed()));
    check(waitFor([&] { return wc.c->hostDeviceId() == B.id && wc.c->epoch() == 2 && wc.c->replicaReady(); }, 60000), QStringLiteral("C follows B"));
    check(wa.c->isHost() && wa.c->epoch() == 1, QStringLiteral("the old host in the minority still hosts epoch 1 (it cannot reach a majority)"));
    check(typeInto(wc, c.replicaC, c.m.ch3, QStringLiteral("Typed on the third device in the new epoch.")) &&
              waitFor([&] { return readAll(c.replicaB + "/content/" + c.m.ch3 + "/text.md").contains("new epoch"); }, 20000),
          QStringLiteral("the new side works"));
    // the network is whole again
    block(false);
    QElapsedTimer healed;
    healed.start();
    check(waitFor([&] { return !wa.c->isHost() && wa.c->epoch() == 2 && wa.c->hostDeviceId() == B.id; }, 90000), QStringLiteral("reunion: the old host meets the newer epoch, stops hosting and follows"),
          QStringLiteral("A host=%1 epoch=%2").arg(wa.c->isHost()).arg(wa.c->epoch()));
    RecoveryDrafts drafts(c.m.folder + "/livesync-local");
    check(waitFor([&] { return !drafts.list().isEmpty(); }, 20000), QStringLiteral("what the old host accepted after the split is kept as a recovery draft on that device"));
    bool forCh2 = false;
    for (const Draft &d : drafts.list())
        forCh2 = forCh2 || d.document == c.m.ch2;
    check(forCh2, QStringLiteral("... one per document (chapter two)"));
    check(waitFor([&] { return treeHash(c.m.folder) == treeHash(c.replicaB) && treeHash(c.replicaC) == treeHash(c.replicaB); }, 60000), QStringLiteral("all three folders are equal again"));
    check(!readAll(c.replicaB + "/content/" + c.m.ch2 + "/text.md").contains("old host after the split") && !readAll(c.m.folder + "/content/" + c.m.ch2 + "/text.md").contains("old host after the split"),
          QStringLiteral("nothing was merged: the newer host's text stays"));
    check(readAll(c.m.folder + "/content/" + c.m.ch3 + "/text.md").contains("new epoch"), QStringLiteral("and the old host has the new side's edit"));
    // the handback to the highest priority device after a minute in step
    check(waitFor([&] { return wa.c->isHost() && wa.c->epoch() == 3; }, 150000), QStringLiteral("the original host takes its hosting back (epoch 3)"));
    check(healed.elapsed() >= 55000, QStringLiteral("... not before it had been in step for a minute"), QString::number(healed.elapsed()));
    check(waitFor([&] { return wb.c->hostDeviceId() == A.id && wc.c->hostDeviceId() == A.id && wb.c->epoch() == 3 && wc.c->epoch() == 3; }, 60000), QStringLiteral("the other two follow it"));
    stopCluster(c);
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_UPDATE_CHECK", "1");
    qputenv("QB_LS_STRICT", "0");
    qputenv("QB_TEST_KEYSTORE", "memory");
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QGuiApplication app(argc, argv);
    QTemporaryDir tmpDir;
    const QString tmp = tmpDir.path();
    const QString program = QStringLiteral(QB_TEST_BUILD_DIR "/quillbinder");
    const QString only = qEnvironmentVariable("QB_LS10_PART");
    auto want = [&](const char *n) { return only.isEmpty() || only == QLatin1String(n); };
    if (want("mesh"))
        testMesh(tmp);
    if (want("names"))
        testNames(tmp);
    if (want("locks"))
        testLocks(tmp);
    if (want("failover"))
        testFailover(tmp, program);
    if (want("partition"))
        testPartition(tmp, program);
    std::printf("\n%d check(s), %d failure(s)\n", total, failures);
    return failures;
}
