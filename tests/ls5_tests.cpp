// Live Sync, work package 5 (0.99.0): the service and the windows. Checks: the registry; the gate's service scope; change sets
// made into units; the local link; the status model (the fake and the service's); two services on localhost (pairing, sharing,
// joining, two windows editing, derived data recomputed by the host's Project, locks and edit requests, the connection lost and
// found again); a window (MainWindow) of a shared project as a follower: the lock at the first key, a remote commit into open
// documents without overwriting unsaved text, close; and a real service process reached through its local socket.
//
//   xvfb-run -a ./qb_ls5_tests /tmp/shots
// Exit code = number of failed checks.
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/Snapshots.h"
#include "core/collab/ChangeUnits.h"
#include "core/collab/Authority.h"
#include "core/collab/Clock.h"
#include "core/collab/Drafts.h"
#include "core/collab/Membership.h"
#include "core/collab/Gate.h"
#include "core/collab/LocalLink.h"
#include "core/collab/Manifest.h"
#include "core/collab/Registry.h"
#include "core/collab/Service.h"
#include "core/collab/ServiceClient.h"
#include "core/collab/ServiceProtocol.h"
#include "core/collab/StatusModel.h"
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/net/MemLink.h"
#include "core/collab/net/PeerStore.h"
#include "core/collab/proto/Messages.h"
#include "core/crypto/KeyFolder.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/ProjectCrypto.h"
#include "core/crypto/Vault.h"
#include "ui/AppSettings.h"
#include "ui/EditorPane.h"
#include "ui/LiveSyncDialogs.h"
#include "ui/LiveSyncServiceControl.h"
#include "ui/LiveSyncTab.h"
#include "ui/LiveSyncUi.h"
#include "ui/MainWindow.h"
#include "ui/MainWindowLiveSync.h"
#include "ui/TextEditor.h"

#include <QApplication>
#include <QComboBox>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTreeWidget>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QProcess>
#include <QRegularExpression>
#include <QSslCertificate>
#include <QSslKey>
#include <QTcpServer>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

#include <cstdio>
#include <memory>

using namespace qb;
using namespace qb::collab;

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

// ---------------------------------------------------------------- 1. the registry

static void testRegistry(const QString &tmp)
{
    std::printf("== the registry\n");
    const QString dir = tmp + "/reg";
    {
        Registry r(dir);
        check(r.entries().isEmpty(), QStringLiteral("a new registry is empty"));
        RegistryEntry e;
        e.uuid = Registry::newUuid();
        e.folder = tmp + "/p1";
        e.title = "P1";
        e.access.insert("dev1", "editor");
        e.authority.insert("state", "ACTIVE");
        QString err;
        check(r.upsert(e, &err), QStringLiteral("an entry is stored"), err);
        RegistryEntry back;
        check(r.find(e.uuid, &back) && back.title == "P1" && back.host && back.access.value("dev1").toString() == "editor"
                  && back.authority.value("state").toString() == "ACTIVE",
              QStringLiteral("it reads back with its access list and the authority record kept as it is"));
        check(r.findByFolder(tmp + "/p1/") && !r.findByFolder(tmp + "/other"), QStringLiteral("found by folder (also written with a slash)"));
        Registry second(dir);
        RegistryEntry c;
        c.uuid = Registry::newUuid();
        c.host = false;
        c.folder = tmp + "/p2";
        second.upsert(c);
        r.reload();
        check(r.entries().size() == 2, QStringLiteral("two registries of one folder see each other's entries (programs of one device)"));
        check(!r.isCopy(tmp + "/p1", e.uuid) && r.isCopy(tmp + "/elsewhere", e.uuid) && !r.isCopy(tmp + "/x", "unknown-uuid"),
              QStringLiteral("a folder named by the uuid but not the registered place is a copy; an unknown uuid is no copy"));
        check(r.lockHost(e.uuid) && !second.lockHost(e.uuid), QStringLiteral("one host session per project and device: the second claim is refused"));
        r.releaseHost(e.uuid);
        check(second.lockHost(e.uuid), QStringLiteral("a released claim can be taken"));
        second.releaseHost(e.uuid);
        check(r.remove(e.uuid) && !r.find(e.uuid), QStringLiteral("an entry is removed"));
    }
    writeAll(dir + "/registry.json", "{ not json");
    Registry broken(dir);
    check(broken.entries().isEmpty() && QFileInfo::exists(dir + "/registry.json.bad"), QStringLiteral("a damaged registry is kept aside and the list starts empty"));
}

// ---------------------------------------------------------------- 2. the gate's service scope

static void testServiceScope(const QString &tmp)
{
    std::printf("== the gate's service scope\n");
    Made m = makeProject(tmp + "/scope", "Scope");
    Follower f;
    QString err;
    auto p = Project::open(m.folder, &err, &f);
    check(bool(p) && gate().active(), QStringLiteral("a follower is attached"), err);
    const QString file = m.folder + "/content/" + m.ch1 + "/text.md";
    const QByteArray before = readAll(file);
    {
        ServiceScope scope;
        check(ServiceScope::active() && safesave::writeFile(file, "the service writes", nullptr), QStringLiteral("inside a service scope a write of the follower's folder reaches the disk"));
        check(readAll(file) == "the service writes" && f.capture().takeReady().isEmpty(), QStringLiteral("... and is not captured as the window's"));
        {
            WindowScope window;
            safesave::writeFile(file, "the window writes", nullptr);
            check(!ServiceScope::active() && readAll(file) == "the service writes", QStringLiteral("a window's write inside the service's work is captured, not written"));
            f.capture().flush();
            const QList<ChangeSet> sets = f.capture().takeReady();
            check(sets.size() == 1 && sets.first().find("content/" + m.ch1 + "/text.md"), QStringLiteral("the captured set holds that write"));
        }
        check(ServiceScope::active(), QStringLiteral("the service scope is back after the window's"));
    }
    check(!ServiceScope::active(), QStringLiteral("the scope ends with its block"));
    safesave::writeFile(file, "after", nullptr);
    check(readAll(file) == "the service writes", QStringLiteral("outside the scope the follower's write is captured again"));
    (void)before;
    p.reset();
}

// ---------------------------------------------------------------- 3. change sets as units

static void testUnits(const QString &tmp)
{
    std::printf("== change sets as units\n");
    Made m = makeProject(tmp + "/units", "Units");
    Follower f;
    QString err;
    auto p = Project::open(m.folder, &err, &f);
    if (!p) {
        check(false, QStringLiteral("the project opens"), err);
        return;
    }
    p->docs()->text(m.ch1)->setPlainText(QStringLiteral("An edit of chapter one."));
    p->docs()->text(m.ch2)->setPlainText(QStringLiteral("An edit of chapter two."));
    p->save(&err);
    f.capture().flush();
    QList<ChangeSet> sets = f.capture().takeReady();
    check(sets.size() >= 1, QStringLiteral("the saves were captured"));
    UnitsResult ur;
    for (const ChangeSet &s : sets) {
        const UnitsResult one = unitsFromChanges(s, m.folder);
        ur.units << one.units;
        for (auto it = one.blobs.constBegin(); it != one.blobs.constEnd(); ++it)
            ur.blobs.insert(it.key(), it.value());
        ur.skipped << one.skipped;
    }
    QStringList names;
    for (const proto::Unit &u : ur.units)
        names << u.name;
    check(names.contains("doc:" + m.ch1) && names.contains("doc:" + m.ch2) && !names.contains("structure"),
          QStringLiteral("two edited chapters make a unit each, with no structure unit"), names.join(','));
    bool filesOk = true;
    for (const proto::Unit &u : ur.units)
        for (const proto::FileOp &fo : u.files)
            filesOk = filesOk && fo.kind == proto::FileOp::Put && ur.blobs.contains(fo.hash) && fo.size == ur.blobs.value(fo.hash).size();
    check(filesOk && !ur.blobs.isEmpty(), QStringLiteral("each put names a blob by its hash, with the plain bytes"));
    bool noIndexFile = true;
    for (const proto::Unit &u : ur.units)
        for (const proto::FileOp &fo : u.files)
            noIndexFile = noIndexFile && !fo.path.endsWith(".qbx");
    check(noIndexFile, QStringLiteral("the project file itself is never sent"));
    // a new document with text and a structure change
    BinderItem *n = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("New Scene"));
    p->docs()->text(n->id)->setPlainText(QStringLiteral("brand new text"));
    p->item(m.ch3)->title = QStringLiteral("Renamed Three");
    p->notifyItemChanged(p->item(m.ch3));
    p->save(&err);
    f.capture().flush();
    sets = f.capture().takeReady();
    UnitsResult both;
    for (const ChangeSet &s : sets) {
        const UnitsResult one = unitsFromChanges(s, m.folder);
        both.units << one.units;
    }
    bool createWithText = false, rename = false;
    for (const proto::Unit &u : both.units) {
        bool hasCreate = false, hasText = false;
        for (const Op &op : u.ops) {
            hasCreate = hasCreate || (op.kind == Op::ItemCreate && op.id == n->id);
            rename = rename || (op.kind == Op::ItemFields && op.id == m.ch3);
        }
        for (const proto::FileOp &fo : u.files)
            hasText = hasText || fo.path.contains(n->id);
        createWithText = createWithText || (hasCreate && hasText);
    }
    check(createWithText, QStringLiteral("a created document and its text are one unit (never split)"));
    check(rename, QStringLiteral("a rename is an operation on the item"));
    // a host-only path is left out
    ChangeSet manual;
    FileChange c;
    c.kind = FileChange::Put;
    c.path = "contents-by-title.txt";
    c.data = "x";
    c.sha256 = QCryptographicHash::hash(c.data, QCryptographicHash::Sha256).toHex();
    c.info = classify(c.path);
    manual.files << c;
    const UnitsResult derived = unitsFromChanges(manual, m.folder);
    check(derived.isEmpty() && derived.skipped.contains("contents-by-title.txt"), QStringLiteral("a derived file is left out and listed as skipped"));
    p.reset();
}

// ---------------------------------------------------------------- 4. the local link

static void testLocalLink(const QString &tmp)
{
    std::printf("== the local link\n");
    const QString name = LocalLink::serverName(tmp + "/ll");
    check(name.startsWith("qb-ls-") && name.size() < 40 && name == LocalLink::serverName(tmp + "/ll") && name != LocalLink::serverName(tmp + "/other"),
          QStringLiteral("the socket's name is short and follows the data folder"));
    LocalServer server;
    QString err;
    check(server.listen(name, &err), QStringLiteral("the server listens"), err);
    check(LocalServer::isServed(name), QStringLiteral("it answers"));
    LocalServer second;
    check(!second.listen(name, &err) && err.contains("already"), QStringLiteral("a second server on the same name is refused"), err);
    LocalLink *accepted = nullptr;
    QObject::connect(&server, &LocalServer::newLink, &server, [&](LocalLink *l) { accepted = l; });
    LocalLink *client = LocalLink::connectTo(name);
    check(waitFor([&] { return accepted && client->state() == net::Link::State::Ready; }), QStringLiteral("a client connects"));
    QList<net::Frame> got;
    QObject::connect(accepted, &net::Link::frameReceived, accepted, [&](const net::Frame &f) { got << f; });
    client->send(net::Frame::json(QJsonObject{{"t", "W_PING"}, {"id", "1"}}));
    const QByteArray tid(16, 'x');
    client->send(net::Frame::data(tid, 0, "bytes"));
    check(waitFor([&] { return got.size() == 2; }) && got.first().object().value("t").toString() == "W_PING" && got.last().kind == net::Frame::Data,
          QStringLiteral("a JSON frame and a data frame arrive in order"));
    QStringList closedWith;
    QObject::connect(accepted, &net::Link::closed, accepted, [&](const QString &c) { closedWith << c; });
    client->close();
    check(waitFor([&] { return accepted->state() == net::Link::State::Closed; }), QStringLiteral("closing one end closes the other"));
    LocalLink *none = LocalLink::connectTo(LocalLink::serverName(tmp + "/nobody"));
    check(waitFor([&] { return none->state() == net::Link::State::Closed; }), QStringLiteral("a link to nobody closes"));
    server.close();
    check(!LocalServer::isServed(name, 200), QStringLiteral("a closed server no longer answers"));
}

// ---------------------------------------------------------------- 5. the status model

static void testFakeStatus()
{
    std::printf("== the status model\n");
    FakeStatusModel m;
    QSignalSpy all(&m, &StatusModel::projectsChanged), one(&m, &StatusModel::projectChanged), dev(&m, &StatusModel::devicesChanged),
        lk(&m, &StatusModel::locksChanged);
    check(m.projects().isEmpty() && m.overallState() == ShareState::NotShared && m.serviceRunning(), QStringLiteral("an empty model: nothing shared"));
    ProjectStatus p;
    p.uuid = "u1";
    p.title = "One";
    p.folder = "/x/one";
    p.state = ShareState::UpToDate;
    m.setProject(p);
    check(all.size() == 1 && one.size() == 1 && m.project("u1").valid && m.projectOfFolder("/x/one").uuid == "u1" && !m.project("zz").valid,
          QStringLiteral("a project is added and found by uuid and folder; the signals fire"));
    m.setState("u1", ShareState::NotConnected);
    ProjectStatus q;
    q.uuid = "u2";
    q.title = "Two";
    q.state = ShareState::Sending;
    m.setProject(q);
    check(m.overallState() == ShareState::NotConnected, QStringLiteral("the overall state is the worst of the projects"));
    m.setState("u2", ShareState::Problem, "disk full");
    check(m.overallState() == ShareState::Problem && m.project("u2").problemText == "disk full", QStringLiteral("a problem carries its text"));
    LockInfo l;
    l.resource = "doc:a";
    l.held = true;
    l.holderName = "Laptop";
    m.setLock("u1", l);
    check(m.lockInfo("u1", "doc:a").holderName == "Laptop" && !m.lockInfo("u1", "doc:b").held && lk.size() == 1, QStringLiteral("a lock is read back; an unknown resource is free"));
    m.setDevices({DeviceStatus{"d1", "Laptop", "ab12", true, QDateTime()}});
    check(m.devices().size() == 1 && dev.size() == 1, QStringLiteral("paired devices are listed"));
    m.askToEdit("u1", "doc:a");
    m.stopSharing("u1");
    check(m.calls() == QStringList({"ask:u1:doc:a", "stopsharing:u1"}), QStringLiteral("the actions are noted"));
    check(shareStateText(ShareState::NotConnected) == "Not connected (draft kept)" && shareStateText(ShareState::Sending) == "Sending changes...",
          QStringLiteral("the five states have their words"));
    m.removeProject("u1");
    check(m.projects().size() == 1, QStringLiteral("a project is removed"));
}

// ---------------------------------------------------------------- 6. two services on localhost

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
        cfg.localName = "qb-ls5-" + name + "-" + QString::number(QCoreApplication::applicationPid());
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
    QObject::connect(host.svc.get(), &Service::pairingCodeReady, host.svc.get(), [&](const QString &c, const QString &, bool) { ca = c; });
    QObject::connect(guest.svc.get(), &Service::pairingCodeReady, guest.svc.get(), [&](const QString &c, const QString &, bool) { cb = c; });
    QObject::connect(host.svc.get(), &Service::pairingFinished, host.svc.get(), [&](bool ok, const QString &) { fh = true; okh = ok; });
    QObject::connect(guest.svc.get(), &Service::pairingFinished, guest.svc.get(), [&](bool ok, const QString &) { fg = true; okg = ok; });
    if (!host.svc->openPairing())
        return false;
    guest.svc->pairWith("127.0.0.1", host.svc->port());
    if (!waitFor([&] { return !ca.isEmpty() && !cb.isEmpty(); }))
        return false;
    if (codeA)
        *codeA = ca;
    if (codeB)
        *codeB = cb;
    host.svc->confirmPairing(ca == cb);
    guest.svc->confirmPairing(ca == cb);
    waitFor([&] { return fh && fg; });
    return okh && okg;
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

static void testServices(const QString &tmp)
{
    std::printf("== two services on localhost\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the service checks are skipped)"));
        return;
    }
    auto H = makeDev(tmp, "Host");
    auto C = makeDev(tmp, "Client");
    check(H->svc->isRunning() && C->svc->isRunning() && H->svc->port() != 0, QStringLiteral("both services run and the host listens"));
    QString ca, cb;
    check(pair(*H, *C, &ca, &cb) && ca == cb && ca.size() == 7, QStringLiteral("the two devices pair: the same six-digit code on both"), ca + " / " + cb);
    check(H->svc->peers()->peers().size() == 1 && C->svc->peers()->peers().size() == 1 && H->svc->statusModel()->devices().size() == 1,
          QStringLiteral("each stores the other, and the status model lists the paired device"));

    Made m = makeProject(tmp + "/hostproj", "Shared");
    QString err;
    const QString uuid = H->svc->shareProject(m.folder, &err);
    check(!uuid.isEmpty() && H->svc->isHosting(uuid), QStringLiteral("the host shares a project"), err);
    check(H->svc->shareProject(m.folder) == uuid, QStringLiteral("sharing it again gives the same project"));
    check(waitFor([&] { return H->svc->clientSession(uuid) && H->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }),
          QStringLiteral("the host's own loopback session is ready"));
    const ProjectStatus hs0 = H->svc->statusModel()->project(uuid);
    check(hs0.valid && hs0.thisDeviceHosts() && hs0.state == ShareState::UpToDate && hs0.participants.size() == 1 && hs0.participants.first().isHost,
          QStringLiteral("the status model: this device hosts, up to date, one participant"));
    // a device without access is refused
    const QString replica = C->svc->joinProject(H->deviceId(), uuid, "Shared", "127.0.0.1", H->svc->port(), &err);
    check(!replica.isEmpty(), QStringLiteral("the other device joins (the replica's folder is made)"), err);
    QThread::msleep(300);
    waitFor([&] { return C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Failed; }, 1500);
    check(!(C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Ready), QStringLiteral("without access the join does not get through"));
    check(C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Failed
              && C->svc->statusModel()->project(uuid).state == ShareState::Problem,
          QStringLiteral("it says Problem in the status model"), C->svc->statusModel()->project(uuid).problemText);
    H->svc->setAccess(uuid, C->deviceId(), Role::Editor);
    check(waitFor([&] { return C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 25000),
          QStringLiteral("with access the service's next try brings the replica up"));
    const QString replicaProjectFile = Project::resolveProjectFile(replica);
    check(!replicaProjectFile.isEmpty() && treeHash(replica) == treeHash(m.folder), QStringLiteral("the replica equals the host's folder, file for file"));

    // two windows
    Win hw, cw;
    check(hw.open(*H->svc, m.folder), QStringLiteral("a window of the host's device opens its project as a follower"), hw.error);
    check(cw.open(*C->svc, replica), QStringLiteral("a window of the other device opens the replica as a follower"), cw.error);
    if (!hw.project || !cw.project)
        return;
    check(hw.project->isFollower() && cw.project->isFollower() && hw.client->isHost() && !cw.client->isHost(), QStringLiteral("both are followers; one is on the host"));

    // 1. the other device's window edits chapter one
    QStringList results;
    QObject::connect(cw.client.get(), &ServiceClient::committed, cw.client.get(), [&](const QString &, qint64 s) { results << QStringLiteral("committed %1").arg(s); });
    QObject::connect(cw.client.get(), &ServiceClient::rejected, cw.client.get(), [&](const QString &, const QJsonArray &r) { results << QStringLiteral("rejected ") + QJsonDocument(r).toJson(QJsonDocument::Compact); });
    QObject::connect(cw.client.get(), &ServiceClient::draftKept, cw.client.get(), [&](const QString &, const QString &r) { results << "draft " + r; });
    QObject::connect(cw.client.get(), &ServiceClient::problem, cw.client.get(), [&](const QString &o, const QString &c) { results << "problem " + o + " " + c; });
    check(cw.lock(m.ch1), QStringLiteral("the other window takes the lock of chapter one"));
    const QString newText = QStringLiteral("One two three four five six seven eight nine ten.");
    cw.setText(m.ch1, newText);
    check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("nine ten"); }), QStringLiteral("the text reaches the host's folder (and nothing is sent back by the replica's own writes)"),
          QStringLiteral("error=[%1] busy=%2 pending=%3 seq=%4 hostseq=%5 state=%6").arg(cw.client->lastError()).arg(cw.client->busy()).arg(cw.client->pendingCount())
              .arg(cw.client->seq()).arg(H->svc->hostSession(uuid)->seq()).arg(cw.client->stateCode()) + results.join(" | "));
    check(waitFor([&] { return wordsOfItem(m.folder, m.ch1) == 10; }, 4000), QStringLiteral("the host's Project recomputed the word count of the document (derived data)"),
          QString::number(wordsOfItem(m.folder, m.ch1)));
    check(waitFor([&] { return QFileInfo::exists(m.folder + "/contents-by-title.txt"); }), QStringLiteral("the host wrote the list of documents"));
    check(waitFor([&] { return treeHash(replica) == treeHash(m.folder); }, 10000), QStringLiteral("the replica follows: equal to the host's folder again, with the derived data"));
    check(waitFor([&] { return hw.client->seq() >= 1; }), QStringLiteral("the host's own window was told of the commit"));
    check(H->svc->hostSession(uuid)->seq() == 1, QStringLiteral("exactly one commit so far (a window writing back what the service placed would make more)"),
          QString::number(H->svc->hostSession(uuid)->seq()));

    // 2. at the same time the host's window edits chapter two
    check(hw.lock(m.ch2), QStringLiteral("the host's window takes the lock of chapter two"));
    hw.setText(m.ch2, QStringLiteral("Two words."));
    check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch2 + "/text.md").contains("Two words") && wordsOfItem(m.folder, m.ch2) == 2; }),
          QStringLiteral("the host window's change goes through the same checks and is in the folder, counted"));
    check(waitFor([&] { return treeHash(replica) == treeHash(m.folder); }, 10000), QStringLiteral("and reaches the replica"));
    check(readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("nine ten"), QStringLiteral("the first editor's text is still there (two editors on different chapters)"));

    // 3. the status model sees who edits what
    const ProjectStatus hs = H->svc->statusModel()->project(uuid);
    const ProjectStatus cs = C->svc->statusModel()->project(uuid);
    bool hostSeesClient = false, clientEditsCh1 = false;
    for (const ParticipantStatus &p : hs.participants)
        if (!p.thisDevice && p.online) {
            hostSeesClient = true;
            clientEditsCh1 = p.editing.contains("doc:" + m.ch1);
        }
    check(hostSeesClient && clientEditsCh1, QStringLiteral("the host's status model shows the other device online and what it edits"));
    check(cs.role == ShareRole::Editor && cs.hostName == "Host" && cs.state == ShareState::UpToDate && !cs.readOnly, QStringLiteral("the client's status: editor, host named, up to date, writable"),
          QStringLiteral("role=%1 host=%2 state=%3 ro=%4 pending=%5").arg(int(cs.role)).arg(cs.hostName).arg(int(cs.state)).arg(cs.readOnly).arg(cs.pendingChanges));
    const LockInfo li = C->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch2);
    check(li.held && !li.mine && li.holderName == "Host", QStringLiteral("the client sees chapter two held by the host's device"), li.holderName);
    check(C->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch1).mine, QStringLiteral("and its own lock as its own"));

    // 4. the same document: an edit request
    QStringList askedHere;
    QObject::connect(hw.client.get(), &ServiceClient::editAsked, hw.client.get(), [&](const QString &r, const QString &who, qint64) { askedHere << r + "|" + who; });
    C->svc->statusActions()->askToEdit(uuid, "doc:" + m.ch2);
    waitFor([&] { return C->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch2).askOpen; }, 2000);
    const LockInfo asked = C->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch2);
    check(asked.askOpen, QStringLiteral("an open edit request is shown as open"));
    check(waitFor([&] { return C->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch2).askWaitMs > 0; }, 3000), QStringLiteral("... with its waiting time"));
    waitFor([&] { return !askedHere.isEmpty(); }, 3000);
    check(!askedHere.isEmpty() && askedHere.first().startsWith("doc:" + m.ch2), QStringLiteral("the holder's window is told that another device asks"), askedHere.join(","));
    hw.client->answerEdit("doc:" + m.ch2, true, C->deviceId());
    check(waitFor([&] { return cw.client->holds("doc:" + m.ch2); }, 8000), QStringLiteral("Save and Hand Over gives the lock to the asker"));
    C->svc->statusActions()->stopEditing(uuid, "doc:" + m.ch2);
    check(waitFor([&] { return !cw.client->holds("doc:" + m.ch2); }, 5000), QStringLiteral("Stop Editing gives a lock up"));

    // 5. the connection is lost: read-only, the state says so; and found again
    cw.setText(m.ch1, newText + QStringLiteral(" And an unsent change.")); // lock held on chapter one
    H->svc->closePairing();
    // the host's listener goes away: the client link closes
    QPointer<ClientSession> cs5 = C->svc->clientSession(uuid);
    const quint16 oldPort = H->svc->port();
    (void)oldPort;
    H->svc->stop();
    check(waitFor([&] { return cs5 && cs5->state() != ClientSession::State::Ready; }, 8000), QStringLiteral("when the host goes away the client is not ready"));
    const ProjectStatus off = C->svc->statusModel()->project(uuid);
    check(off.state == ShareState::NotConnected && off.readOnly && !off.readOnlyReason.isEmpty(), QStringLiteral("the status model says Not connected and the project is read-only with a reason"), off.readOnlyReason);
    check(waitFor([&] { return cw.client->readOnly(); }, 4000), QStringLiteral("the client's window is told it is read-only"));
    cw.project.reset();
    cw.client.reset();
    hw.project.reset();
    hw.client.reset();
}

// ---------------------------------------------------------------- 6b. who hosts: hand-over, failover, unknown device

static void testHosting(const QString &tmp)
{
    std::printf("== who hosts (package 6 through the service)\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the hosting checks are skipped)"));
        return;
    }
    // ---- a planned hand-over between two services
    auto A = makeDev(tmp, "HoA");
    auto B = makeDev(tmp, "HoB");
    check(pair(*A, *B), QStringLiteral("two devices pair (the keys are kept for the list of devices)"));
    Made m = makeProject(tmp + "/hoproj", "Handover");
    QString err;
    const QString uuid = A->svc->shareProject(m.folder, &err);
    check(!uuid.isEmpty() && A->svc->authority(uuid) && A->svc->authority(uuid)->isHost() && A->svc->authority(uuid)->epoch() == 1,
          QStringLiteral("sharing founds the authority: this device hosts in epoch 1"), err);
    A->svc->setAccess(uuid, B->deviceId(), Role::Editor);
    check(A->svc->authority(uuid)->membership().count() == 2 && A->svc->authority(uuid)->membership().version >= 2,
          QStringLiteral("giving access publishes a signed list with both devices"));
    const QString replica = B->svc->joinProject(A->deviceId(), uuid, "Handover", "127.0.0.1", A->svc->port(), &err);
    check(!replica.isEmpty() && waitFor([&] { return B->svc->clientSession(uuid) && B->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 25000),
          QStringLiteral("the other device joins"), err);
    check(waitFor([&] { return B->svc->authority(uuid) && B->svc->authority(uuid)->membership().count() == 2 && !B->svc->authority(uuid)->chain().isEmpty(); }, 5000),
          QStringLiteral("the joining device took the list and the chain of proofs from its host"));
    // an edit through the host's own window, so that the copy and the state are not empty
    {
        Win hw;
        check(hw.open(*A->svc, m.folder) && hw.lock(m.ch1), QStringLiteral("a window of the first host edits"), hw.error);
        hw.setText(m.ch1, QStringLiteral("Before the hand-over there were exactly seven words."));
        check(waitFor([&] { return B->svc->clientSession(uuid)->seq() >= 1 && treeHash(replica) == treeHash(m.folder); }, 10000), QStringLiteral("and the replica follows"));
        CommitLedger ledger(m.folder + "/livesync-local/ledger.json");
        ledger.load();
        check(ledger.size() >= 1, QStringLiteral("the commit the device wrote was entered in its ledger"), QString::number(ledger.size()));
        hw.project.reset();
        hw.client.reset();
    }
    A->svc->setPeerAddress(B->deviceId(), "127.0.0.1", B->svc->port());
    B->svc->setPeerAddress(A->deviceId(), "127.0.0.1", A->svc->port());
    QString why;
    check(A->svc->transferHosting(uuid, B->deviceId(), &why), QStringLiteral("Transfer Hosting starts"), why);
    check(waitFor([&] { return B->svc->isHosting(uuid) && B->svc->authority(uuid)->isHost(); }, 20000), QStringLiteral("the other device hosts after the hand-over"),
          QString::number(int(B->svc->authority(uuid)->state())) + " " + QString::number(int(A->svc->authority(uuid)->state())));
    check(waitFor([&] { return !A->svc->isHosting(uuid) && A->svc->authority(uuid)->state() == AuthState::Follower && A->svc->authority(uuid)->host() == B->deviceId(); }, 20000),
          QStringLiteral("the first device became a client of the new host"));
    check(B->svc->authority(uuid)->epoch() == 2 && A->svc->authority(uuid)->epoch() == 2, QStringLiteral("both are in epoch 2"));
    check(waitFor([&] { return A->svc->clientSession(uuid) && A->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 25000),
          QStringLiteral("the first device is connected to the new host and ready"));
    {
        Win bw, aw;
        check(bw.open(*B->svc, replica) && bw.client->isHost(), QStringLiteral("a window on the new host attaches to the hosted project"), bw.error);
        check(aw.open(*A->svc, m.folder) && !aw.client->isHost(), QStringLiteral("a window on the old host's device attaches as a client of it"), aw.error);
        check(aw.lock(m.ch2), QStringLiteral("the client window takes a lock from the new host"));
        aw.setText(m.ch2, QStringLiteral("After the hand-over, three words."));
        check(waitFor([&] { return readAll(replica + "/content/" + m.ch2 + "/text.md").contains("three words") && wordsOfItem(replica, m.ch2) == 5; }, 15000),
              QStringLiteral("a change of the old host's device reaches the new host's folder, and the new host counted its words"), QString::number(wordsOfItem(replica, m.ch2)));
        check(waitFor([&] { return treeHash(replica) == treeHash(m.folder); }, 15000), QStringLiteral("and the old host's copy equals the new host's folder"));
        aw.project.reset();
        aw.client.reset();
        bw.project.reset();
        bw.client.reset();
    }
    const ProjectStatus pa = A->svc->statusModel()->project(uuid), pb = B->svc->statusModel()->project(uuid);
    check(pb.thisDeviceHosts() && !pa.thisDeviceHosts() && pa.hostDeviceId == B->deviceId() && pa.epoch == 2,
          QStringLiteral("the status model names the new host on both devices"));

    // ---- an unknown device announces hosting: refused, the warning is in the status model
    {
        auto X = net::DeviceIdentity::createEphemeral("Stranger");
        Membership own;
        own.project = uuid;
        own.version = 1;
        Member sm;
        sm.id = X->deviceId();
        sm.key = X->publicKeyDer();
        sm.name = "S";
        own.members = {sm};
        own.signWith(*X);
        const Proof founding = makeFounding(uuid, 9, *X);
        B->svc->authority(uuid)->handleMessage(X->deviceId(), proto::make(proto::T::AUTHORITY, uuid, {{"chain", QJsonArray{founding.toJson()}}, {"membership", own.toJson()}}));
        const ProjectStatus ps = B->svc->statusModel()->project(uuid);
        check(ps.notices.size() == 1 && ps.notices.first().contains("unknown device") && B->svc->authority(uuid)->epoch() == 2,
              QStringLiteral("an unknown device that announces hosting is refused and the warning shows in the status model"), ps.notices.join("|"));
    }
    A.reset();
    B.reset();

    // ---- a failover: the host goes away, the other device takes over
    auto F = makeDev(tmp, "FoA", false, true);
    auto G = makeDev(tmp, "FoB", false, true);
    check(pair(*F, *G), QStringLiteral("two devices pair"));
    Made m2 = makeProject(tmp + "/foproj", "Failover");
    const QString u2 = F->svc->shareProject(m2.folder, &err);
    F->svc->setAccess(u2, G->deviceId(), Role::Editor);
    const QString rep2 = G->svc->joinProject(F->deviceId(), u2, "Failover", "127.0.0.1", F->svc->port(), &err);
    check(!rep2.isEmpty() && waitFor([&] { return G->svc->clientSession(u2) && G->svc->clientSession(u2)->state() == ClientSession::State::Ready; }, 25000),
          QStringLiteral("the other device joins"), err);
    check(waitFor([&] { return G->svc->authority(u2) && !G->svc->authority(u2)->chain().isEmpty(); }, 5000), QStringLiteral("and holds the chain"));
    // the host's last commit: one document changed
    {
        Win fw;
        fw.open(*F->svc, m2.folder);
        fw.lock(m2.ch1);
        fw.setText(m2.ch1, QStringLiteral("Written before the host went away."));
        check(waitFor([&] { return G->svc->clientSession(u2)->seq() >= 1 && treeHash(rep2) == treeHash(m2.folder); }, 10000), QStringLiteral("the replica holds the last commit"));
        fw.project.reset();
        fw.client.reset();
    }
    F->svc->stop();
    F.reset();
    check(waitFor([&] { return G->svc->clientSession(u2) && G->svc->clientSession(u2)->state() != ClientSession::State::Ready; }, 8000), QStringLiteral("the host is gone"));
    for (int i = 0; i < 20 && !G->svc->isHosting(u2); ++i) {
        G->clock->advance(5000, 250);
        waitFor([&] { return G->svc->isHosting(u2); }, 100);
    }
    check(G->svc->isHosting(u2) && G->svc->authority(u2)->isHost() && G->svc->authority(u2)->epoch() == 2,
          QStringLiteral("after the host was out of contact for the failover time the other device hosts in epoch 2"),
          QString::number(int(G->svc->authority(u2)->state())) + " epoch " + QString::number(G->svc->authority(u2)->epoch()));
    {
        Win gw;
        check(gw.open(*G->svc, rep2) && gw.client->isHost(), QStringLiteral("a window on the new host attaches"), gw.error);
        if (gw.project) {
            check(gw.lock(m2.ch2), QStringLiteral("it takes a lock"));
            gw.setText(m2.ch2, QStringLiteral("Hosted by the replica now."));
            check(waitFor([&] { return readAll(rep2 + "/content/" + m2.ch2 + "/text.md").contains("replica now") && wordsOfItem(rep2, m2.ch2) == 5; }, 10000),
                  QStringLiteral("a commit goes through the new host, and it counted the words"), QString::number(wordsOfItem(rep2, m2.ch2)));
            check(readAll(rep2 + "/content/" + m2.ch1 + "/text.md").contains("before the host went away"), QStringLiteral("what the old host last committed is kept"));
        }
        gw.project.reset();
        gw.client.reset();
    }
}

// ---------------------------------------------------------------- 7. a real window

static void testWindow(const QString &tmp)
{
    std::printf("== a window of a shared project\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the window checks are skipped)"));
        return;
    }
    auto H = makeDev(tmp, "WinHost");
    auto C = makeDev(tmp, "WinClient");
    check(pair(*H, *C), QStringLiteral("the devices pair"));
    Made m = makeProject(tmp + "/winproj", "WinShared");
    QString err;
    const QString uuid = H->svc->shareProject(m.folder, &err);
    H->svc->setAccess(uuid, C->deviceId(), Role::Editor);
    const QString replica = C->svc->joinProject(H->deviceId(), uuid, "WinShared", "127.0.0.1", H->svc->port(), &err);
    check(waitFor([&] { return C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 20000),
          QStringLiteral("a replica is ready"), err);
    // the window of the host's device is a real MainWindow
    ui::setInProcessLiveSyncService(H->svc.get());
    MainWindow w;
    w.resize(1200, 800);
    w.show();
    check(w.openProject(m.folder), QStringLiteral("the main window opens the shared project"));
    Project *p = w.project();
    check(p && p->isFollower(), QStringLiteral("it opened as a follower of the service"));
    if (!p) {
        ui::setInProcessLiveSyncService(nullptr);
        return;
    }
    TextEditor *ed = nullptr;
    if (EditorPane *ep = w.findChildren<EditorPane *>().value(0))
        ep->showItems({p->item(m.ch1)});
    for (QWidget *wd : QApplication::allWidgets())
        if (auto *te = qobject_cast<TextEditor *>(wd))
            if (te->item() && te->item()->id == m.ch1 && te->isVisible() && !te->isNotes())
                ed = te;
    check(ed != nullptr, QStringLiteral("chapter one is shown in an editor"));
    if (ed) {
        ed->setFocus();
        check(TextEditor::hasLockGate(), QStringLiteral("the program has a lock gate for the editors"));
        const QString before = ed->toPlainText();
        QTest::keyClicks(ed, "Z");
        check(ed->toPlainText() == before, QStringLiteral("the first key is held until the lock is granted (the text has not changed yet)"));
        check(waitFor([&] { return ed->toPlainText() != before; }, 5000) && ed->toPlainText().contains("Z"), QStringLiteral("after the grant the held key is typed"));
        check(waitFor([&] { return H->svc->hostSession(uuid)->locks().find("doc:" + m.ch1) != nullptr; }), QStringLiteral("the host holds the lock for this device"));
        // a remote commit from the other device into a document that is open here (a different one)
        Win cw;
        check(cw.open(*C->svc, replica), QStringLiteral("a window of the other device"));
        check(cw.lock(m.ch2), QStringLiteral("it locks chapter two"));
        cw.setText(m.ch2, QStringLiteral("Written on the other device."));
        // chapter two is open in the host's window too (in the second pane or loaded): load it
        p->docs()->text(m.ch2);
        check(waitFor([&] { return p->docs()->text(m.ch2)->toPlainText().contains("other device"); }, 10000),
              QStringLiteral("the remote commit shows in the open document of the window"));
        check(ed->toPlainText().contains("Z"), QStringLiteral("the text typed here is untouched by the remote commit"));
        check(waitFor([&] { return p->item(m.ch2)->wordCount == 5; }, 5000), QStringLiteral("the derived data (the word count) came with it"), QString::number(p->item(m.ch2)->wordCount));
        // unsaved text here is never overwritten by a remote commit (what was typed in chapter one is saved first)
        QString saveErr;
        p->save(&saveErr);
        waitFor([&] { return readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("Z"); }, 8000);
        p->docs()->text(m.ch3)->setPlainText(QStringLiteral("Unsaved local text of chapter three."));
        check(cw.lock(m.ch3), QStringLiteral("the other device locks chapter three (nobody holds it)"));
        cw.setText(m.ch3, QStringLiteral("Remote text for chapter three."));
        check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch3 + "/text.md").contains("Remote text"); }, 10000), QStringLiteral("its text reaches the host"));
        waitFor([&] { return false; }, 700);
        check(p->docs()->text(m.ch3)->toPlainText() == "Unsaved local text of chapter three.", QStringLiteral("the open document with unsaved text was not overwritten"),
              p->docs()->text(m.ch3)->toPlainText());
        p->docs()->text(m.ch3)->setPlainText(QStringLiteral("The remote text.")); // (what the other device wrote)
        p->docs()->reload(m.ch3);
        // (the lock of chapter one is held here: the other device cannot lock it)
        cw.client->lock({"doc:" + m.ch1});
        check(!waitFor([&] { return cw.client->holds("doc:" + m.ch1); }, 1500), QStringLiteral("the other device cannot take the lock of the document being typed in"));
        w.grab().save(QDir::tempPath() + "/ls5-window.png");
        cw.project.reset();
        cw.client.reset();
    }
    // closing: the typed text is handed over and reaches the host's folder
    QString err2;
    p->save(&err2);
    check(w.closeProject(), QStringLiteral("the window closes the project"));
    check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("Z"); }, 10000), QStringLiteral("what was typed is in the host's folder after the close"));
    check(waitFor([&] { return H->svc->hostSession(uuid)->locks().find("doc:" + m.ch1) == nullptr; }, 5000), QStringLiteral("the lock was given up at the close"));
    ui::setInProcessLiveSyncService(nullptr);
}

// ---------------------------------------------------------------- 7b. an encrypted project is hosted through the vault

static void testEncryptedHost(const QString &tmp)
{
    std::printf("== an encrypted project, hosted\n");
    if (!net::DeviceIdentity::available() || !crypto::available()) {
        check(true, QStringLiteral("(this build has no encryption with Botan TLS: the encrypted-host checks are skipped)"));
        return;
    }
    crypto::KeyRing &ring = crypto::KeyRing::device();
    const QByteArray wp = "correct horse battery staple again", rp = "the recovery words are kept on paper";
    const QString writer = ring.createKey(QStringLiteral("Test Writer"), wp);
    const QString recovery = ring.createKey(QStringLiteral("Test Recovery"), rp);
    ring.removeKey(recovery, true);
    ring.unlock(writer, wp);
    ring.save(crypto::KeyRing::deviceDir());
    crypto::keyfolder::setRoles(crypto::keyfolder::Roles{writer, recovery});
    Made m = makeProject(tmp + "/enchost", "EncShared");
    QString err;
    check(crypto::encryptFolder(m.folder, crypto::Recipients{writer, recovery}, {}, &err), QStringLiteral("a project is encrypted"), err);
    auto H = makeDev(tmp, "EncHost");
    const QString uuid = H->svc->shareProject(m.folder, &err);
    check(!uuid.isEmpty() && H->svc->isHosting(uuid), QStringLiteral("the service hosts it (its Project reads the encrypted files)"), err);
    Win hw;
    check(hw.open(*H->svc, m.folder), QStringLiteral("a window opens it as a follower"), hw.error);
    if (!hw.project)
        return;
    check(hw.lock(m.ch1), QStringLiteral("it locks chapter one"));
    hw.setText(m.ch1, QStringLiteral("Secretword was written in a window."));
    const QString textPath = m.folder + "/content/" + m.ch1 + "/text.md";
    check(waitFor([&] {
              QByteArray plain;
              return vault::readFile(textPath, &plain) && plain.contains("Secretword");
          }, 10000),
          QStringLiteral("the commit is in the host's folder (read through the vault)"));
    const QByteArray raw = readAll(textPath);
    check(vault::isSealedData(raw.left(64)) && !raw.contains("Secretword"), QStringLiteral("the file the service wrote is encrypted on disk"));
    QByteArray titles;
    check(waitFor([&] { return QFileInfo::exists(m.folder + "/contents-by-title.txt"); }, 5000) && vault::isSealedData(readAll(m.folder + "/contents-by-title.txt").left(64)),
          QStringLiteral("the list of documents the host's Project wrote is encrypted too"));
    QByteArray idx = readAll(Project::resolveProjectFile(m.folder));
    check(vault::isSealedData(idx.left(64)) && !idx.contains("<Item"), QStringLiteral("the project file is still encrypted"));
}

// ---------------------------------------------------------------- 8. the service as a process

static int serviceProcess(const QString &tmp, const QString &app)
{
    std::printf("== the service process\n");
    if (app.isEmpty() || !QFileInfo::exists(app)) {
        check(true, QStringLiteral("(the program's file is not here: the process checks are skipped)"));
        return 0;
    }
    const QString xdg = tmp + "/proc-xdg";
    QDir().mkpath(xdg);
    const QString lsdir = xdg + "/Quillbinder/livesync";
    QDir().mkpath(lsdir);
    qputenv("XDG_DATA_HOME", xdg.toUtf8());
    Made m = makeProject(tmp + "/procproj", "ProcShared");
    // the registry says this folder is hosted (as if it had been shared earlier)
    {
        Registry reg(lsdir);
        RegistryEntry e;
        e.uuid = Registry::newUuid();
        e.folder = m.folder;
        e.title = "ProcShared";
        e.hostName = "Proc";
        e.epoch = 1;
        reg.upsert(e);
    }
    QProcess proc;
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("XDG_DATA_HOME", xdg);
    env.insert("QB_TEST_KEYSTORE", "file:" + tmp + "/proc-keys.json");
    env.insert("QT_QPA_PLATFORM", "offscreen");
    env.insert("QB_LS_PORT", "0");
    env.insert("QB_LS_NO_DISCOVERY", "1");
    env.insert("QB_LS_IDLE_MS", "1500");
    env.insert("QB_NO_UPDATE_CHECK", "1");
    proc.setProcessEnvironment(env);
    proc.start(app, {"--live-sync-service"});
    check(proc.waitForStarted(5000), QStringLiteral("the service process starts"));
    const QString name = LocalLink::serverName(lsdir);
    check(waitFor([&] { return LocalServer::isServed(name, 100); }, 20000), QStringLiteral("it listens on the socket named after the data folder"));
    LocalLink *link = LocalLink::connectTo(name);
    waitFor([&] { return link->state() == net::Link::State::Ready; }, 3000);
    ServiceClient client(link);
    const ServiceClient::Attached a = client.attach(m.folder, false);
    check(a.ok && a.host && a.title == "ProcShared", QStringLiteral("a window attaches to the hosted folder through the socket"), a.code);
    check(waitFor([&] { client.attach(m.folder, false); return client.replicaReady(); }, 10000), QStringLiteral("the service's loopback session is ready"));
    // a follower window of this test process changes a text: the service process writes it
    QString err;
    auto p = Project::open(m.folder, &err, client.follower());
    check(bool(p), QStringLiteral("the project opens as a follower"), err);
    if (p) {
        client.lock({"doc:" + m.ch1});
        check(waitFor([&] { return client.holds("doc:" + m.ch1); }, 5000), QStringLiteral("the lock comes from the service process"));
        client.follower()->capture().setListener([&] { QMetaObject::invokeMethod(&client, [&] { client.sendCaptured(); }, Qt::QueuedConnection); });
        p->docs()->text(m.ch1)->setPlainText(QStringLiteral("Written by a window, saved by another program."));
        p->save(&err);
        check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("saved by another program"); }, 10000),
              QStringLiteral("the service process wrote the window's text into the project folder"));
        check(waitFor([&] { return wordsOfItem(m.folder, m.ch1) == 8; }, 6000), QStringLiteral("and recomputed the counts with its own Project"), QString::number(wordsOfItem(m.folder, m.ch1)));
        p.reset();
    }
    client.detach();
    link->close();
    check(proc.waitForFinished(15000) && proc.exitStatus() == QProcess::NormalExit, QStringLiteral("with no window left the service process ends by itself"));
    if (proc.state() != QProcess::NotRunning)
        proc.kill();
    return 0;
}

// ---------------------------------------------------------------- 8. the real screens, two programs

// the screens of Live Sync of two programs (two devices, two main windows, two services in this process; the control the screens
// talk to is switched to the device whose window is being used)
static void testScreens(const QString &tmp)
{
    std::printf("== the screens of two programs\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the screen checks are skipped)"));
        return;
    }
    auto A = makeDev(tmp, "ScreenA", false, true);
    auto B = makeDev(tmp, "ScreenB", false, true);
    livesync::ServiceControl ca(A->svc.get()), cb(B->svc.get());
    auto useA = [&] {
        livesync::setLiveSyncControl(&ca);
        ui::setInProcessLiveSyncService(A->svc.get());
    };
    auto useB = [&] {
        livesync::setLiveSyncControl(&cb);
        ui::setInProcessLiveSyncService(B->svc.get());
    };
    auto advance = [&](int ms) {
        for (int i = 0; i < ms; i += 250) {
            A->clock->advance(250, 250);
            B->clock->advance(250, 250);
            waitFor([] { return false; }, 25);
        }
    };
    A->svc->setPeerAddress(B->deviceId(), "127.0.0.1", B->svc->port());
    B->svc->setPeerAddress(A->deviceId(), "127.0.0.1", A->svc->port());
    Made m = makeProject(tmp + "/screenproj", "Screens");
    const QString file = Project::resolveProjectFile(m.folder);

    useA();
    MainWindow wa;
    wa.resize(1300, 800);
    wa.show();
    waitFor([&] { return wa.findChild<livesync::LiveSyncUi *>() != nullptr; }, 2000);
    useB();
    MainWindow wb;
    wb.resize(1300, 800);
    wb.show();
    waitFor([&] { return wb.findChild<livesync::LiveSyncUi *>() != nullptr; }, 2000);
    auto *uiA = wa.findChild<livesync::LiveSyncUi *>();
    auto *uiB = wb.findChild<livesync::LiveSyncUi *>();
    check(uiA && uiB && uiA->button() && uiA->active() && uiB->active(), QStringLiteral("each program has Live Sync's button, tab and bars, switched on"));

    // ---- pairing through the pairing window
    QString codeB;
    QObject::connect(&cb, &livesync::LiveSyncControl::pairingCode, [&](const QString &, const QString &c) { codeB = c; });
    bool pairedA = false, pairedB = false;
    QObject::connect(&ca, &livesync::LiveSyncControl::pairingFinished, [&](bool ok, const QString &) { pairedA = ok; });
    QObject::connect(&cb, &livesync::LiveSyncControl::pairingFinished, [&](bool ok, const QString &) { pairedB = ok; });
    useA();
    livesync::PairingDialog *pd = livesync::showPairingDialog(&wa);
    auto *open = pd->findChild<QPushButton *>("pairOpenWindow");
    check(open != nullptr, QStringLiteral("the pairing window of the first program has its button"));
    if (!open)
        return;
    open->click();
    check(A->svc->pairingOpen(), QStringLiteral("Open the Pairing Window opens it on the first device"));
    cb.pairWithAddress("127.0.0.1:" + QString::number(A->svc->port()));
    auto *codeLabel = pd->findChild<QLabel *>("pairCode");
    check(waitFor([&] { return !codeB.isEmpty() && codeLabel && !codeLabel->text().trimmed().isEmpty(); }, 15000), QStringLiteral("both programs show a code"));
    QString shown = codeLabel ? codeLabel->text() : QString();
    shown.remove(QRegularExpression(QStringLiteral("[^0-9]")));
    codeB.remove(QRegularExpression(QStringLiteral("[^0-9]")));
    check(!codeB.isEmpty() && shown == codeB, QStringLiteral("the codes match"), shown + " / " + codeB);
    pd->grab().save(QDir::tempPath() + "/ls5-pairing.png");
    useA();
    if (auto *match = pd->findChild<QPushButton *>("pairMatch"))
        match->click();
    cb.answerPairing(true);
    check(waitFor([&] { return pairedA && pairedB && A->svc->peers()->peers().size() == 1 && B->svc->peers()->peers().size() == 1; }, 15000),
          QStringLiteral("after both confirm the devices are paired"));
    pd->close();

    // ---- sharing from the first program's window
    useA();
    check(wa.openProject(file) && wa.project() && !wa.project()->isFollower(), QStringLiteral("the first program opens its project (not shared yet)"));
    QAction *share = uiA->menu() ? uiA->menu()->findChild<QAction *>("lsShareProject") : nullptr;
    if (!share && uiA->menu())
        for (QAction *a : uiA->menu()->actions())
            if (a->objectName() == QLatin1String("lsShareProject"))
                share = a;
    check(share && share->isEnabled(), QStringLiteral("Share This Project is in the button's menu and can be used"));
    if (!share)
        return;
    share->trigger();
    check(waitFor([&] { return wa.project() && wa.project()->isFollower(); }, 15000), QStringLiteral("the project is shared and its window opened it again as a follower of the service"),
          QStringLiteral("project=%1 registry=%2 dir=%3").arg(wa.project() != nullptr).arg(QFileInfo::exists(A->dir + "/registry.json")).arg(ui::liveSyncDir()));
    const QStringList uuids = A->svc->projectUuids();
    const QString uuid = uuids.value(0);
    check(uuids.size() == 1 && A->svc->isHosting(uuid), QStringLiteral("this device hosts it"));
    // access for the other device, from the Hosted Projects window
    auto *hd = livesync::showHostedProjectsDialog(&wa, uuid);
    waitFor([&] { return hd->findChild<QComboBox *>("accessCombo:" + B->deviceId()) != nullptr; }, 3000);
    if (auto *combo = hd->findChild<QComboBox *>("accessCombo:" + B->deviceId())) {
        const int idx = combo->findText(QStringLiteral("Editor"));
        combo->setCurrentIndex(idx < 0 ? 0 : idx);
        useA();
        emit combo->activated(combo->currentIndex()); // (what choosing an entry with the mouse does)
    }
    check(waitFor([&] { return A->svc->authority(uuid)->membership().isMember(B->deviceId()); }, 8000), QStringLiteral("giving the other device the Editor role in Hosted Projects puts it in the signed list"));
    hd->grab().save(QDir::tempPath() + "/ls5-hosted.png");
    hd->close();

    // ---- joining from the second program's Join window
    useB();
    auto *jd = livesync::showJoinDialog(&wb);
    auto *tree = jd->findChild<QTreeWidget *>("joinTree");
    check(waitFor([&] { return tree && tree->topLevelItemCount() >= 1 && tree->topLevelItem(0)->text(0) == QLatin1String("Screens"); }, 15000),
          QStringLiteral("the Join window lists the project the paired device shares"));
    QString opened;
    QObject::connect(jd, &livesync::JoinDialog::openProject, [&](const QString &f) { opened = f; });
    if (tree && tree->topLevelItemCount() >= 1) {
        tree->setCurrentItem(tree->topLevelItem(0));
        jd->grab().save(QDir::tempPath() + "/ls5-join.png");
        if (auto *join = jd->findChild<QPushButton *>("joinButton"))
            join->click();
    }
    check(waitFor([&] { return !opened.isEmpty(); }, 40000), QStringLiteral("when the copy is whole the Join window hands its folder over"), opened);
    useB();
    check(!opened.isEmpty() && wb.openProject(Project::resolveProjectFile(opened)) && wb.project() && wb.project()->isFollower(), QStringLiteral("the second program opens the copy as a follower"));
    if (!wa.project() || !wb.project())
        return;
    check(waitFor([&] { return treeHash(opened) == treeHash(m.folder); }, 10000), QStringLiteral("the copy equals the host's folder"));

    // ---- two documents in parallel, one in each program
    auto editorOf = [&](MainWindow &w, const QString &id) -> TextEditor * {
        for (EditorPane *ep : w.findChildren<EditorPane *>())
            if (ep->isVisible()) {
                ep->showItems({w.project()->item(id)});
                break;
            }
        TextEditor *found = nullptr;
        waitFor([&] {
            for (QWidget *wd : QApplication::allWidgets())
                if (auto *te = qobject_cast<TextEditor *>(wd))
                    if (te->item() && te->item()->id == id && te->isVisible() && !te->isNotes() && te->project() == w.project())
                        found = te;
            return found != nullptr;
        }, 3000);
        return found;
    };
    TextEditor *ea = editorOf(wa, m.ch1);
    useB();
    TextEditor *eb = editorOf(wb, m.ch2);
    check(ea && eb, QStringLiteral("each program shows another chapter in its editor"), [&] {
              QStringList d;
              for (QWidget *wd : QApplication::allWidgets())
                  if (auto *te = qobject_cast<TextEditor *>(wd))
                      d << QStringLiteral("%1:%2:%3:%4").arg(te->item() ? te->item()->id.left(4) : "-").arg(te->isVisible()).arg(te->project() == wa.project() ? "A" : te->project() == wb.project() ? "B" : "?").arg(te->isNotes());
              return QStringLiteral("a=%1 b=%2 ").arg(ea != nullptr).arg(eb != nullptr) + d.join(' ') + " ch2=" + m.ch2.left(4);
          }());
    if (!ea || !eb)
        return;
    ea->setFocus();
    QTest::keyClicks(ea, "Alpha");
    eb->setFocus();
    QTest::keyClicks(eb, "Beta");
    check(waitFor([&] { return ea->toPlainText().contains("Alpha") && eb->toPlainText().contains("Beta"); }, 8000), QStringLiteral("both type at the same time: each gets its lock at the first key"));
    QString e1, e2;
    wa.project()->save(&e1);
    wb.project()->save(&e2);
    check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("Alpha") && readAll(m.folder + "/content/" + m.ch2 + "/text.md").contains("Beta"); }, 15000),
          QStringLiteral("both texts are in the host's folder"));
    waitFor([&] { return false; }, 3000); // (the commits of both programs settle)
    check(waitFor([&] { return treeHash(opened) == treeHash(m.folder); }, 20000), QStringLiteral("and the copy follows"),
          treeDiff(m.folder, opened) + QStringLiteral(" Bstate=%1 Bseq=%2 Bhead=%3 hostseq=%4 Apending=%5 Bpending=%6")
              .arg(int(B->svc->clientSession(uuid)->state())).arg(B->svc->clientSession(uuid)->seq()).arg(B->svc->clientSession(uuid)->headSeq())
              .arg(A->svc->hostSession(uuid)->seq()).arg(A->svc->clientSession(uuid)->outbox().pending().size()).arg(B->svc->clientSession(uuid)->outbox().pending().size())
              + QStringLiteral(" problem=[%1] Afail=[%2] Bfail=[%3] drafts=%4").arg(B->svc->statusModel()->project(uuid).problemText, A->svc->clientSession(uuid)->failure(), B->svc->clientSession(uuid)->failure())
                    .arg(B->svc->clientSession(uuid)->outbox().drafts().size())
              + QStringLiteral(" Afollower=%1 Bfollower=%2 gateA=%3 gateB=%4 hostCh1=[%5] hostCh2=[%6]")
                    .arg(wa.project()->isFollower()).arg(wb.project()->isFollower()).arg(gate().followerOfFolder(wa.project()->folderPath()) != nullptr)
                    .arg(gate().followerOfFolder(wb.project()->folderPath()) != nullptr).arg(QString::fromUtf8(readAll(m.folder + "/content/" + m.ch1 + "/text.md")).left(30),
                                                                                                    QString::fromUtf8(readAll(m.folder + "/content/" + m.ch2 + "/text.md")).left(30)));
    check(waitFor([&] { return wa.project()->docs()->text(m.ch2)->toPlainText().contains("Beta") && wb.project()->docs()->text(m.ch1)->toPlainText().contains("Alpha"); }, 10000),
          QStringLiteral("each program's open documents received the other's text"));
    wa.grab().save(QDir::tempPath() + "/ls5-screen-a.png");

    // ---- an edit request: declined, then taken over after the waiting time
    useB();
    uiB->setSelectedDocument(m.ch1);
    waitFor([&] { return false; }, 300);
    auto *ask = uiB->tab() ? uiB->tab()->findChild<QPushButton *>("lsAsk") : nullptr;
    check(ask && waitFor([&] { return ask->isEnabled(); }, 4000), QStringLiteral("the second program's tab offers Ask to Edit for the document the first one holds"));
    if (!ask)
        return;
    int askedSignals = 0;
    QObject::connect(&ca, &livesync::LiveSyncControl::editAsked, [&](const QString &, const QString &, const QString &) { ++askedSignals; });
    // (the first program types once more: it holds its lock again if it had let it go while the other program's window changed)
    useA();
    ea->setFocus();
    QTest::keyClicks(ea, "!");
    waitFor([&] { return A->svc->clientSession(uuid)->heldLocks().contains("doc:" + m.ch1); }, 5000);
    useB();
    waitFor([&] { return ask->isEnabled(); }, 4000);
    ask->click();
    useA();
    auto *barsA = uiA->barsWidget();
    check(waitFor([&] { return barsA && barsA->findChild<QPushButton *>("barAsked:Decline") != nullptr; }, 8000), QStringLiteral("the first program shows the request with Save and Hand Over and Decline"),
          QStringLiteral("signals=%1 requests=%2 bars=%3 buttons=%4").arg(askedSignals).arg(A->svc->hostSession(uuid)->editRequests().count())
              .arg(barsA ? barsA->isVisible() : false).arg([&] { QStringList n; for (auto *b : barsA->findChildren<QPushButton *>()) n << b->objectName(); return n.join(','); }()));
    wa.grab().save(QDir::tempPath() + "/ls5-asked.png");
    if (auto *dec = barsA->findChild<QPushButton *>("barAsked:Decline"))
        dec->click();
    useB();
    check(waitFor([&] { return B->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch1).declined; }, 8000), QStringLiteral("Decline reaches the asker: the status shows the request declined"));
    // the request stays open after a Decline: the holder's bar stays, the asker's tab shows the request as open (Ask to Edit cannot be
    // used twice), and a second Decline starts the waiting time again
    useA();
    check(barsA->findChild<QPushButton *>("barAsked:Decline") != nullptr, QStringLiteral("a declined request stays open: the first program's bar stays"));
    useB();
    check(ask && !ask->isEnabled() && B->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch1).askOpen, QStringLiteral("the asker's tab shows the request as open"));
    advance(15000);
    const qint64 leftBefore = B->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch1).askWaitMs;
    useA();
    if (auto *dec2 = barsA->findChild<QPushButton *>("barAsked:Decline"))
        dec2->click();
    useB();
    check(waitFor([&] { return B->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch1).askWaitMs > leftBefore + 5000; }, 8000),
          QStringLiteral("a second Decline starts the asker's waiting time again"),
          QStringLiteral("before=%1 now=%2").arg(leftBefore).arg(B->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch1).askWaitMs));
    advance(21000);
    useB();
    auto *take = uiB->tab()->findChild<QPushButton *>("lsTake");
    check(take && waitFor([&] { return take->isEnabled(); }, 4000), QStringLiteral("after the waiting time Take Over can be used"));
    // the first program types once more, not yet saved, just before the other one takes over
    useA();
    ea->setFocus();
    QTest::keyClicks(ea, "?");
    const bool unsavedBefore = ea->toPlainText().contains('?');
    useB();
    check(unsavedBefore, QStringLiteral("the first program has text it has not sent when the other takes over"));
    if (take)
        take->click();
    check(waitFor([&] { return B->svc->clientSession(uuid) && B->svc->clientSession(uuid)->heldLocks().contains("doc:" + m.ch1); }, 10000),
          QStringLiteral("taking over gives the second program the lock of the document"));
    check(waitFor([&] { return !A->svc->clientSession(uuid)->heldLocks().contains("doc:" + m.ch1); }, 8000), QStringLiteral("and the first program lost it"));
    // the protective snapshot of the committed text came first, and the first program's unsent text became a draft on its device
    check(waitFor([&] {
              for (const Snapshot &sn : snapshots::list(wa.project(), m.ch1))
                  if (sn.title.startsWith(QLatin1String("Before takeover by")))
                      return true;
              return false;
          }, 8000), QStringLiteral("a protective snapshot \"Before takeover by ...\" of the document was made first"));
    check(waitFor([&] { return A->svc->clientSession(uuid)->outbox().drafts().size() >= 1; }, 10000), QStringLiteral("the text the first program had not sent is kept as a recovery draft on its device"),
          QStringLiteral("drafts=%1 pending=%2").arg(A->svc->clientSession(uuid)->outbox().drafts().size()).arg(A->svc->clientSession(uuid)->outbox().pending().size()));
    wb.grab().save(QDir::tempPath() + "/ls5-screen-b.png");

    useA();
    wa.closeProject();
    useB();
    wb.closeProject();
    ui::setInProcessLiveSyncService(nullptr);
    livesync::setLiveSyncControl(nullptr);
}

// ---------------------------------------------------------------- 9. two service processes: hand-over and failover

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
    d.proc->setProcessEnvironment(env);
    d.proc->start(app, {"--live-sync-service"});
}
} // namespace

static void testHostingProcesses(const QString &tmp, const QString &app)
{
    std::printf("== two service processes: hand-over and failover\n");
    if (app.isEmpty() || !QFileInfo::exists(app) || !net::DeviceIdentity::available()) {
        check(true, QStringLiteral("(the program's file or the TLS pack is not here: the process checks are skipped)"));
        return;
    }
    ProcDev A, B;
    for (ProcDev *d : {&A, &B}) {
        d->name = d == &A ? "ProcA" : "ProcB";
        d->xdg = tmp + "/hp-" + d->name;
        d->lsdir = d->xdg + "/Quillbinder/livesync";
        d->keys = tmp + "/hp-" + d->name + "-keys.json";
        QDir().mkpath(d->lsdir);
        d->port = freePort();
    }
    // 1. each process makes its identity once and ends again
    for (ProcDev *d : {&A, &B}) {
        startProc(*d, app, 300, QString());
        d->proc->waitForFinished(30000);
        QFile f(d->lsdir + "/device.json");
        QJsonObject o;
        if (f.open(QIODevice::ReadOnly))
            o = QJsonDocument::fromJson(f.readAll()).object();
        d->cert = QByteArray::fromBase64(o.value("cert").toString().toLatin1());
        d->instance = o.value("instance").toString();
        d->name = o.value("name").toString().isEmpty() ? d->name : o.value("name").toString();
        d->spki = QSslCertificate(d->cert, QSsl::Der).publicKey().toDer();
        d->id = net::DeviceIdentity::deviceIdOfPin(net::DeviceIdentity::pinOfPublicKey(d->spki));
    }
    check(!A.id.isEmpty() && !B.id.isEmpty() && A.id != B.id, QStringLiteral("both processes made an identity"));
    if (A.id.isEmpty() || B.id.isEmpty())
        return;
    // 2. the two know each other (as if paired) and share a project
    for (ProcDev *d : {&A, &B}) {
        const ProcDev *o = d == &A ? &B : &A;
        net::PeerStore ps(d->lsdir);
        net::PeerRecord r;
        r.pin = net::DeviceIdentity::pinOfPublicKey(o->spki);
        r.publicKey = o->spki;
        r.name = o->name;
        r.instanceId = o->instance;
        r.pairedAt = 1700000000;
        ps.add(r);
    }
    Made m = makeProject(tmp + "/hpproj", "ProcHandover");
    const QString uuid = Registry::newUuid();
    const QString replica = tmp + "/hp-replica/ProcHandover.quillbinder";
    QDir().mkpath(replica);
    {
        Registry ra(A.lsdir);
        RegistryEntry e;
        e.uuid = uuid;
        e.folder = m.folder;
        e.title = "ProcHandover";
        e.host = true;
        e.hostDeviceId = A.id;
        e.hostName = A.name;
        e.epoch = 1;
        e.access.insert(B.id, "editor");
        ra.upsert(e);
        Registry rb(B.lsdir);
        RegistryEntry r;
        r.uuid = uuid;
        r.folder = replica;
        r.title = "ProcHandover";
        r.host = false;
        r.hostDeviceId = A.id;
        r.hostName = A.name;
        r.lastAddress = "127.0.0.1";
        r.lastPort = A.port;
        rb.upsert(r);
    }
    startProc(A, app, 600000, B.id + "=127.0.0.1:" + QString::number(B.port));
    startProc(B, app, 600000, A.id + "=127.0.0.1:" + QString::number(A.port));
    const QString nameA = LocalLink::serverName(A.lsdir), nameB = LocalLink::serverName(B.lsdir);
    check(waitFor([&] { return LocalServer::isServed(nameA, 100) && LocalServer::isServed(nameB, 100); }, 30000), QStringLiteral("both service processes listen on their sockets"));
    LocalLink *la = LocalLink::connectTo(nameA);
    LocalLink *lb = LocalLink::connectTo(nameB);
    waitFor([&] { return la->state() == net::Link::State::Ready && lb->state() == net::Link::State::Ready; }, 3000);
    ServiceClient ca(la), cb(lb);
    const ServiceClient::Attached aa = ca.attach(m.folder, false);
    const ServiceClient::Attached ab = cb.attach(replica, false);
    check(aa.ok && aa.host && ab.ok && !ab.host, QStringLiteral("a window of each process attaches: one hosts, one holds a copy"), aa.code + "/" + ab.code);
    check(waitFor([&] { return cb.replicaReady() && treeHash(replica) == treeHash(m.folder); }, 40000), QStringLiteral("the second process's copy is complete and equals the host's folder"),
          QStringLiteral("ready=%1 state=%2 seq=%3 pending=%4 %5\nhost state: %6\nreplica state: %7")
              .arg(cb.replicaReady()).arg(cb.stateCode()).arg(cb.seq()).arg(cb.pendingCount()).arg(treeDiff(replica, m.folder))
              .arg(QString::fromUtf8(readAll(m.folder + "/livesync/state.json")).left(300), QString::fromUtf8(readAll(replica + "/livesync-replica.json")).left(300)));
    check(waitFor([&] { return ca.epoch() == 1; }, 3000), QStringLiteral("the host's epoch is 1"));
    // 3. a planned hand-over
    ca.transferHosting(B.id);
    check(waitFor([&] { return cb.isHost() && cb.epoch() == 2 && !ca.isHost() && ca.epoch() == 2; }, 30000), QStringLiteral("the second process hosts in epoch 2 and the first became its client"),
          QStringLiteral("A host=%1 epoch=%2, B host=%3 epoch=%4").arg(ca.isHost()).arg(ca.epoch()).arg(cb.isHost()).arg(cb.epoch()));
    check(waitFor([&] { return ca.replicaReady() && ca.stateCode() == int(ShareState::UpToDate); }, 30000), QStringLiteral("the first process is connected to the new host and up to date"), QString::number(ca.stateCode()));
    QString err;
    auto p = Project::open(m.folder, &err, ca.follower());
    check(bool(p), QStringLiteral("a window of the first process opens its copy as a follower"), err);
    if (p) {
        ca.lock({"doc:" + m.ch1});
        check(waitFor([&] { return ca.holds("doc:" + m.ch1); }, 8000), QStringLiteral("the lock comes from the new host's process"));
        ca.follower()->capture().setListener([&] { QMetaObject::invokeMethod(&ca, [&] { ca.sendCaptured(); }, Qt::QueuedConnection); });
        p->docs()->text(m.ch1)->setPlainText(QStringLiteral("Typed after the hand-over by the old host's device."));
        p->save(&err);
        check(waitFor([&] { return readAll(replica + "/content/" + m.ch1 + "/text.md").contains("old host's device"); }, 15000),
              QStringLiteral("the text reached the new host's folder"));
        check(waitFor([&] { return wordsOfItem(replica, m.ch1) == 9; }, 6000), QStringLiteral("and the new host's process counted its words"), QString::number(wordsOfItem(replica, m.ch1)));
        check(waitFor([&] { return treeHash(replica) == treeHash(m.folder); }, 15000), QStringLiteral("the first process's copy follows"));
        p.reset();
    }
    // 4. the new host's process dies: after the failover time the first device hosts (epoch 3)
    cb.detach();
    lb->close();
    B.proc->kill();
    B.proc->waitForFinished(5000);
    QElapsedTimer since;
    since.start();
    check(waitFor([&] { return ca.isHost() && ca.epoch() == 3; }, 70000), QStringLiteral("after the failover time the first process hosts in epoch 3"),
          QStringLiteral("host=%1 epoch=%2 state=%3").arg(ca.isHost()).arg(ca.epoch()).arg(ca.stateCode()));
    check(since.elapsed() >= 25000, QStringLiteral("... not before the host was out of contact for about half a minute"), QString::number(since.elapsed()));
    check(readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("old host's device"), QStringLiteral("the text of the dead host's last commit is in the new host's folder"));
    ca.detach();
    la->close();
    A.proc->kill();
    A.proc->waitForFinished(5000);
}

// ---------------------------------------------------------------- main

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_UPDATE_CHECK", "1");
    qputenv("QB_LS_STRICT", "0");
    qputenv("QB_TEST_KEYSTORE", "memory");
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QB_NO_WELCOME", "1");
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QApplication app(argc, argv);
    QTemporaryDir tmpDir;
    const QString tmp = tmpDir.path();
    QString program = QStringLiteral(QB_TEST_BUILD_DIR "/quillbinder");

    // (a suite's own switch: QB_LS5_PART=screens runs one part only, while a part is being worked on)
    const QString only = qEnvironmentVariable("QB_LS5_PART");
    auto want = [&](const char *n) { return only.isEmpty() || only == QLatin1String(n); };
    if (want("small")) {
        testRegistry(tmp);
        testServiceScope(tmp);
        testUnits(tmp);
        testLocalLink(tmp);
        testFakeStatus();
    }
    if (want("services"))
        testServices(tmp);
    if (want("hosting"))
        testHosting(tmp);
    if (want("window"))
        testWindow(tmp);
    if (want("screens"))
        testScreens(tmp);
    if (want("encrypted"))
        testEncryptedHost(tmp);
    if (want("process"))
        serviceProcess(tmp, program);
    if (want("processes"))
        testHostingProcesses(tmp, program);

    std::printf("\n%d check(s), %d failure(s)\n", total, failures);
    return failures;
}
