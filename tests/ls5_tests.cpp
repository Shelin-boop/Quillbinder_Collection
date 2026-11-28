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
#include "core/collab/ChangeUnits.h"
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
#include "core/crypto/KeyFolder.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/ProjectCrypto.h"
#include "core/crypto/Vault.h"
#include "ui/AppSettings.h"
#include "ui/EditorPane.h"
#include "ui/MainWindow.h"
#include "ui/MainWindowLiveSync.h"
#include "ui/TextEditor.h"

#include <QApplication>
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
    QString deviceId() const { return id->deviceId(); }
};

static std::unique_ptr<Dev> makeDev(const QString &tmp, const QString &name, bool localSocket = false)
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

    testRegistry(tmp);
    testServiceScope(tmp);
    testUnits(tmp);
    testLocalLink(tmp);
    testFakeStatus();
    testServices(tmp);
    testWindow(tmp);
    testEncryptedHost(tmp);
    serviceProcess(tmp, program);

    std::printf("\n%d check(s), %d failure(s)\n", total, failures);
    return failures;
}
