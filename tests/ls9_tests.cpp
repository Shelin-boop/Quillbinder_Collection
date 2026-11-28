// Live Sync, work package 9 (0.99.0): the integration finished. Checks: the host's backups and snapshot jobs wired into the service
// (a client's and the host's own request for a backup and its state, a client's and the host's own window's snapshot jobs, a snapshot
// job never writes an older project file over a newer one, a job cut off by a restart is published); a window's answer to the
// service's first message is never lost (the state that follows it); the holder's bar of an edit request stays after a Decline and goes
// when the asker gives up; recovery drafts (their text, Compare, Keep as Copy, Take Over, Delete asks); "Keep hosting when the last
// window is closed" handing the hosted projects to a service process of its own and taking them back.
//
//   xvfb-run -a ./qb_ls9_tests /tmp/shots
// Exit code = number of failed checks.
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/Snapshots.h"
#include "core/collab/Authority.h"
#include "core/collab/Clock.h"
#include "core/collab/Drafts.h"
#include "core/collab/HostBackup.h"
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
#include "core/collab/net/TlsLink.h"
#include "core/crypto/Vault.h"
#include "import/Compare.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/CompareDialog.h"
#include "ui/EditorPane.h"
#include "ui/LiveSyncControl.h"
#include "ui/LiveSyncDialogs.h"
#include "ui/LiveSyncServiceControl.h"
#include "ui/LiveSyncTab.h"
#include "ui/LiveSyncUi.h"
#include "ui/MainWindow.h"
#include "ui/MainWindowLiveSync.h"
#include "ui/TextEditor.h"

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QTreeWidget>

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

static QString treeHash(const QString &folder)
{
    Manifest m;
    Manifest::scan(folder, QString(), {}, &m);
    return m.stateHash();
}

struct Made {
    QString folder, ch1, ch2, ch3;
};

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

// does the project file of `folder` hold an item with this title?
static bool projectFileHasTitle(const QString &folder, const QString &title)
{
    const QString file = Project::resolveProjectFile(folder);
    QByteArray raw;
    if (file.isEmpty() || !vault::readFile(file, &raw))
        return false;
    QDomDocument dom;
    if (!dom.setContent(raw))
        return false;
    const QDomNodeList items = dom.elementsByTagName(QStringLiteral("Item"));
    for (int i = 0; i < items.size(); ++i)
        if (items.at(i).toElement().firstChildElement(QStringLiteral("Title")).text() == title)
            return true;
    return false;
}

static QString idOfTitle(const QString &folder, const QString &title)
{
    const QString file = Project::resolveProjectFile(folder);
    QByteArray raw;
    if (file.isEmpty() || !vault::readFile(file, &raw))
        return QString();
    QDomDocument dom;
    if (!dom.setContent(raw))
        return QString();
    const QDomNodeList items = dom.elementsByTagName(QStringLiteral("Item"));
    for (int i = 0; i < items.size(); ++i)
        if (items.at(i).toElement().firstChildElement(QStringLiteral("Title")).text() == title)
            return items.at(i).toElement().attribute(QStringLiteral("id"));
    return QString();
}

// ---------------------------------------------------------------- devices

struct Dev {
    QString name, dir, backups;
    std::unique_ptr<net::DeviceIdentity> id;
    std::unique_ptr<Service> svc;
    QString deviceId() const { return id->deviceId(); }
};

static std::unique_ptr<Dev> makeDev(const QString &tmp, const QString &name)
{
    auto d = std::make_unique<Dev>();
    d->name = name;
    d->dir = tmp + "/" + name;
    d->backups = tmp + "/backups-" + name;
    d->id = net::DeviceIdentity::createEphemeral(name);
    Service::Config cfg;
    cfg.dir = d->dir;
    cfg.identity = d->id.get();
    cfg.deviceName = name;
    cfg.address = QHostAddress::LocalHost;
    cfg.port = 0;
    cfg.discovery = false;
    cfg.localSocket = false;
    cfg.backupDir = [dir = d->backups] { return dir; };
    cfg.backupKeep = [] { return 3; };
    d->svc = std::make_unique<Service>(cfg);
    QString err;
    if (!d->svc->start(&err))
        std::printf("      service %s: %s\n", qPrintable(name), qPrintable(err));
    return d;
}

static bool pair(Dev &host, Dev &guest)
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
    host.svc->confirmPairing(ca == cb);
    guest.svc->confirmPairing(ca == cb);
    waitFor([&] { return fh && fg; });
    return okh && okg;
}

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

static QString snapshotDir(const QString &folder, const QString &docId)
{
    QString err;
    auto p = Project::open(folder, &err);
    return p ? p->snapshotsDir(docId) : QString();
}

static QStringList snapshotTitles(const QString &folder, const QString &docId)
{
    QStringList l;
    QString err;
    auto p = Project::open(folder, &err); // (a plain look at the folder's snapshots)
    if (!p)
        return l;
    for (const Snapshot &s : snapshots::list(p.get(), docId))
        l << s.title;
    return l;
}

// ---------------------------------------------------------------- 1. the host's jobs in the service

static void testWiring(const QString &tmp)
{
    std::printf("== the host's backups and snapshot jobs in the service\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the service checks are skipped)"));
        return;
    }
    auto H = makeDev(tmp, "H9");
    auto C = makeDev(tmp, "C9");
    check(pair(*H, *C), QStringLiteral("the devices pair"));
    Made m = makeProject(tmp + "/wireproj", "Wired");
    QString err;
    const QString uuid = H->svc->shareProject(m.folder, &err);
    H->svc->setAccess(uuid, C->deviceId(), Role::Editor);
    const QString replica = C->svc->joinProject(H->deviceId(), uuid, "Wired", "127.0.0.1", H->svc->port(), &err);
    check(waitFor([&] { return C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 25000),
          QStringLiteral("the other device joins and is ready"), err);
    Win hw, cw;
    check(hw.open(*H->svc, m.folder) && cw.open(*C->svc, replica), QStringLiteral("a window on each device"), hw.error + cw.error);
    if (!hw.project || !cw.project)
        return;

    // ---- backups
    check(H->svc->statusModel()->project(uuid).backup.state == BackupStatus::None, QStringLiteral("before any backup the state is none"));
    H->svc->statusActions()->requestBackup(uuid);
    check(waitFor([&] { return H->svc->statusModel()->project(uuid).backup.state == BackupStatus::Done; }, 30000), QStringLiteral("the host's own request makes a backup"),
          QString::number(int(H->svc->statusModel()->project(uuid).backup.state)));
    check(H->svc->statusModel()->project(uuid).backup.lastSuccess.isValid() && !QDir(H->backups).entryList({"*.zip"}).isEmpty(),
          QStringLiteral("... with the time of its success, in the writer's backup folder"));
    check(waitFor([&] { return C->svc->statusModel()->project(uuid).backup.lastSuccess.isValid(); }, 8000),
          QStringLiteral("the other device sees the state of the host's backup (the host tells it when it became ready)"));
    const int before = QDir(H->backups).entryList({"*.zip"}).size();
    C->svc->statusActions()->requestBackup(uuid);
    check(waitFor([&] { return QDir(H->backups).entryList({"*.zip"}).size() == before + 1; }, 30000), QStringLiteral("a request of the other device makes the host back up"));
    check(waitFor([&] { return C->svc->statusModel()->project(uuid).backup.state == BackupStatus::Done; }, 8000), QStringLiteral("... and its state is Done on the other device"));
    check(QDir(C->backups).entryList({"*.zip"}).isEmpty(), QStringLiteral("a client makes no backup of its own"));

    // ---- snapshot jobs of a window, on the other device and on the host's own
    QString code, text;
    check(cw.client->snapshotJob({{"op", "take"}, {"doc", m.ch1}, {"title", "From the other device"}}, &code, &text), QStringLiteral("a window of the other device takes a snapshot (the host does it)"), code + " " + text);
    check(waitFor([&] { return snapshotTitles(replica, m.ch1).contains("From the other device"); }, 8000), QStringLiteral("the snapshot is in the other device's copy"));
    check(snapshotTitles(m.folder, m.ch1).contains("From the other device"), QStringLiteral("and in the host's folder"));
    check(hw.client->snapshotJob({{"op", "take"}, {"doc", m.ch1}, {"title", "From the host's window"}}, &code, &text), QStringLiteral("the window of the host's own device takes one too"), code + " " + text);
    check(snapshotTitles(m.folder, m.ch1).contains("From the host's window"), QStringLiteral("... it is in the folder"));
    check(!cw.client->snapshotJob({{"op", "take"}, {"doc", "does-not-exist"}, {"title", "x"}}, &code, &text) && code == "BAD_OP", QStringLiteral("a job for a document that is not there is refused"), code);
    check(!cw.client->snapshotJob({{"op", "rollBack"}, {"doc", m.ch1}, {"file", "20300101-000000.md"}}, &code, &text) && !code.isEmpty(), QStringLiteral("a roll back to a snapshot that is not there is refused"), code);

    // ---- the host's Project follows every commit: a snapshot job never writes an older project file over a newer one
    BinderItem *added = hw.project->model()->createItem(hw.project->draft(), -1, ItemType::Text, QStringLiteral("Added By The Host Window"));
    const QString addedId = added ? added->id : QString();
    QString saveErr;
    hw.project->save(&saveErr);
    check(waitFor([&] { return projectFileHasTitle(m.folder, "Added By The Host Window"); }, 10000), QStringLiteral("a structure change is committed: the host's project file has the new item"), saveErr);
    check(cw.client->snapshotJob({{"op", "take"}, {"doc", m.ch2}, {"title", "After the structure change"}}, &code, &text), QStringLiteral("a snapshot job right after it works"), code + " " + text);
    check(projectFileHasTitle(m.folder, "Added By The Host Window"), QStringLiteral("the project file still holds the structure change after the snapshot job (it was not overwritten by an older state)"));
    check(waitFor([&] { return projectFileHasTitle(replica, "Added By The Host Window") && treeHash(replica) == treeHash(m.folder); }, 15000),
          QStringLiteral("the other device's copy holds both, equal to the host's folder"));
    // and a rename and a delete of a snapshot
    const QString file = snapshotDir(m.folder, m.ch2).isEmpty() ? QString() : QDir(snapshotDir(m.folder, m.ch2)).entryList({"*.md", "*.rtf"}).value(0);
    check(!file.isEmpty() && cw.client->snapshotJob({{"op", "rename"}, {"doc", m.ch2}, {"file", file}, {"title", "Renamed by a client"}}, &code, &text) && snapshotTitles(m.folder, m.ch2).contains("Renamed by a client"),
          QStringLiteral("a snapshot is renamed by a job"), code + " " + text);
    check(cw.client->snapshotJob({{"op", "delete"}, {"doc", m.ch2}, {"file", file}}, &code, &text) && snapshotTitles(m.folder, m.ch2).isEmpty(), QStringLiteral("and deleted by a job"), code + " " + text);
    (void)addedId;

    // ---- a job cut off by a restart is published when the host starts again
    cw.project.reset();
    cw.client.reset();
    hw.project.reset();
    hw.client.reset();
    const qint64 seqBefore = H->svc->hostSession(uuid)->seq();
    H->svc->stop();
    {
        const QString dir = snapshotDir(m.folder, m.ch3);
        QDir().mkpath(dir);
        QFile f(dir + "/20300102-030405.md");
        f.open(QIODevice::WriteOnly);
        f.write("A snapshot whose commit was never made");
        f.close();
        QDir().mkpath(m.folder + "/livesync");
        QFile mark(m.folder + "/livesync/job-pending.json");
        mark.open(QIODevice::WriteOnly);
        mark.write("{\"opId\":\"cut-off-job\"}");
        mark.close();
    }
    QString startErr;
    check(H->svc->start(&startErr), QStringLiteral("the service starts again"), startErr);
    check(waitFor([&] { return H->svc->hostSession(uuid) && H->svc->hostSession(uuid)->seq() > seqBefore; }, 10000),
          QStringLiteral("the job that was cut off is published as a commit of its own"), QString::number(H->svc->hostSession(uuid) ? H->svc->hostSession(uuid)->seq() : -1));
    check(!QFileInfo::exists(m.folder + "/livesync/job-pending.json"), QStringLiteral("and its mark is gone"));
    const qint64 nowSeq = H->svc->hostSession(uuid) ? H->svc->hostSession(uuid)->seq() : 0;
    check(readAll(m.folder + QStringLiteral("/livesync/log/%1.json").arg(nowSeq, 12, 10, QLatin1Char('0'))).contains("20300102-030405"), QStringLiteral("the commit's log entry names the job's snapshot file"));
}

// ---------------------------------------------------------------- 2. the edit request's bar, the first message of a window

static void testEditRequests(const QString &tmp)
{
    std::printf("== edit requests and the window's first message\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the service checks are skipped)"));
        return;
    }
    auto H = makeDev(tmp, "H9b");
    auto C = makeDev(tmp, "C9b");
    check(pair(*H, *C), QStringLiteral("the devices pair"));
    Made m = makeProject(tmp + "/askproj", "Asked");
    QString err;
    const QString uuid = H->svc->shareProject(m.folder, &err);
    H->svc->setAccess(uuid, C->deviceId(), Role::Editor);
    const QString replica = C->svc->joinProject(H->deviceId(), uuid, "Asked", "127.0.0.1", H->svc->port(), &err);
    check(waitFor([&] { return C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 25000), QStringLiteral("the other device is ready"), err);
    Win hw;
    check(hw.open(*H->svc, m.folder) && hw.lock(m.ch2), QStringLiteral("the host's window holds chapter two"));
    int asked = 0, ended = 0;
    QObject::connect(H->svc.get(), &Service::editAsked, [&](const QString &, const QString &, const QString &) { ++asked; });
    QObject::connect(H->svc.get(), &Service::editAskEnded, [&](const QString &, const QString &) { ++ended; });
    C->svc->statusActions()->askToEdit(uuid, "doc:" + m.ch2);
    check(waitFor([&] { return asked == 1; }, 8000), QStringLiteral("the holder is told that the other device asks"));
    H->svc->answerEditRequest(uuid, "doc:" + m.ch2, false);
    check(waitFor([&] { return C->svc->statusModel()->lockInfo(uuid, "doc:" + m.ch2).declined; }, 8000), QStringLiteral("Decline reaches the asker"));
    waitFor([&] { return false; }, 400);
    check(ended == 0, QStringLiteral("a Decline leaves the request open: the holder's bar is not taken away"), QString::number(ended));
    C->svc->statusActions()->cancelAsk(uuid, "doc:" + m.ch2);
    check(waitFor([&] { return ended == 1; }, 8000), QStringLiteral("when the asker gives up the holder's bar goes (the host tells the holder)"), QString::number(ended));
    // a request that ends with a takeover: the holder's lock is revoked and its bar goes
    C->svc->statusActions()->askToEdit(uuid, "doc:" + m.ch2);
    check(waitFor([&] { return asked == 2; }, 8000), QStringLiteral("a new request is shown again"));
    hw.client->unlock({"doc:" + m.ch2}); // (the host's window lets the document go: the request is granted)
    check(waitFor([&] { return ended >= 2; }, 8000), QStringLiteral("a request for a lock that is given up is over: the bar goes"), QString::number(ended));
    check(waitFor([&] { return C->svc->clientSession(uuid)->heldLocks().contains("doc:" + m.ch2); }, 10000), QStringLiteral("and the asker gets the lock"));
    hw.client.reset();
    hw.project.reset();

    // ---- the answer to a window's attach and the state that follows it come back to back
    std::printf("-- the first message\n");
    net::MemLink::Pair pr = net::MemLink::createPair(QByteArray(32, 'w'), QByteArray(32, 's'), nullptr);
    QObject::connect(pr.a, &net::Link::frameReceived, pr.a, [&](const net::Frame &f) {
        bool ok = false;
        const QJsonObject q = f.object(&ok);
        if (!ok || local::typeOf(q) != QLatin1String(local::ATTACH))
            return;
        // what a busy service does: the answer, and at once the next state of the project
        QJsonObject ans = local::msg(local::ATTACHED, {{"re", q.value("id")}, {"ok", true}, {"uuid", "u-1"}, {"host", false}, {"state", 3}, {"ready", false},
                                                        {"title", "T"}, {"hostName", "H"}});
        pr.a->send(net::Frame::json(ans));
        pr.a->send(net::Frame::json(local::msg(local::STATE, {{"uuid", "u-1"}, {"host", false}, {"state", 1}, {"ready", true}, {"seq", 7}})));
    });
    ServiceClient fake(pr.b);
    const ServiceClient::Attached a = fake.attach(tmp + "/nowhere", false);
    check(a.ok, QStringLiteral("a window attaches to a service that answers at once"));
    waitFor([&] { return fake.replicaReady(); }, 500);
    check(fake.replicaReady() && fake.seq() == 7 && fake.stateCode() == 1, QStringLiteral("the state that came right after the answer was taken in, not lost"),
          QStringLiteral("ready=%1 seq=%2 state=%3").arg(fake.replicaReady()).arg(fake.seq()).arg(fake.stateCode()));
}

// ---------------------------------------------------------------- 3. recovery drafts

static void testDrafts(const QString &tmp)
{
    std::printf("== recovery drafts\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the draft checks are skipped)"));
        return;
    }
    auto H = makeDev(tmp, "H9c");
    Made m = makeProject(tmp + "/draftproj", "Drafted");
    QString err;
    const QString uuid = H->svc->shareProject(m.folder, &err);
    check(!uuid.isEmpty(), QStringLiteral("a project is shared"), err);
    check(waitFor([&] { return H->svc->clientSession(uuid) && H->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 10000), QStringLiteral("it is ready"));
    // two drafts: one of the authority's kind (after a split) and one of the outbox's kind (a refused change)
    Draft d;
    d.project = uuid;
    d.document = m.ch1;
    d.title = "Chapter One";
    d.reason = "split";
    d.author = H->deviceId();
    d.forkEpoch = 1;
    d.forkSeq = 3;
    d.createdMs = 1700000000000;
    d.hasText = true;
    d.textPath = "content/" + m.ch1 + "/text.md";
    d.id = RecoveryDrafts::idFor(uuid, d.forkEpoch, d.forkSeq, d.document, d.author);
    const QByteArray draftText = "The draft's own words, kept after a split.";
    check(H->svc->authority(uuid)->drafts().add(d, draftText, QByteArray(), &err), QStringLiteral("a draft is kept"), err);
    {
        proto::Unit u;
        u.name = "doc:" + m.ch2;
        const QByteArray bytes = "Text of a change the host did not take.";
        proto::FileOp f;
        f.kind = proto::FileOp::Put;
        f.path = "content/" + m.ch2 + "/text.md";
        f.hash = sha256Hex(bytes);
        f.size = bytes.size();
        u.files << f;
        const QString op = H->svc->clientSession(uuid)->outbox().add(QJsonArray{proto::toJson(u)}, {{f.hash, bytes}}, 1700000001000, &err);
        check(!op.isEmpty() && H->svc->clientSession(uuid)->outbox().toDraft(op, "LOCK_REVOKED"), QStringLiteral("a refused change is kept as a draft of the outbox"), err);
    }
    const auto list = H->svc->drafts(uuid);
    check(list.size() == 2, QStringLiteral("the service lists both"), QString::number(list.size()));
    bool titlesOk = true;
    for (const Service::DraftEntry &e : list)
        titlesOk = titlesOk && (e.docTitle == "Chapter One" || e.docTitle == "Chapter Two");
    check(titlesOk, QStringLiteral("each names its document by its title"));
    for (const Service::DraftEntry &e : list) {
        const Service::DraftContent c = H->svc->draftContent(e.id);
        check(c.ok && c.uuid == uuid && c.docId == e.docId && !c.text.isEmpty() && (c.text.startsWith("The draft's own") || c.text.startsWith("Text of a change")),
              QStringLiteral("the text of a draft is read back (%1)").arg(e.docTitle), c.why);
    }
    check(!H->svc->draftContent("no-such-draft").ok && !H->svc->deleteDraft("no-such-draft"), QStringLiteral("an unknown draft is neither read nor deleted"));

    // ---- the actions of the Recovery Drafts window, in a real window of the project
    livesync::ServiceControl control(H->svc.get());
    livesync::setLiveSyncControl(&control);
    ui::setInProcessLiveSyncService(H->svc.get());
    int confirms = 0;
    bool answer = false;
    livesync::dialogHooks().confirm = [&](QWidget *, const QString &, const QString &, const QString &) { ++confirms; return answer; };
    MainWindow w;
    w.resize(1300, 800);
    w.show();
    waitFor([&] { return w.findChild<livesync::LiveSyncUi *>() != nullptr; }, 2000);
    waitFor([&] { return false; }, 100);
    const QString file = Project::resolveProjectFile(m.folder);
    check(w.openProject(file) && w.project() && w.project()->isFollower(), QStringLiteral("the window opens the shared project as a follower"));
    if (!w.project()) {
        livesync::dialogHooks().confirm = nullptr;
        livesync::setLiveSyncControl(nullptr);
        ui::setInProcessLiveSyncService(nullptr);
        return;
    }
    livesync::DraftsDialog *dlg = livesync::showDraftsDialog(&w, uuid);
    auto *tree = dlg->findChild<QTreeWidget *>("draftsTree");
    auto *note = dlg->findChild<QLabel *>("draftNote");
    auto select = [&](const QString &title) {
        for (int i = 0; tree && i < tree->topLevelItemCount(); ++i)
            if (tree->topLevelItem(i)->text(1) == title) {
                tree->setCurrentItem(tree->topLevelItem(i));
                return true;
            }
        return false;
    };
    check(tree && tree->topLevelItemCount() == 2 && select("Chapter One"), QStringLiteral("the window lists the drafts with their documents' titles"));
    auto button = [&](const char *name) { return dlg->findChild<QPushButton *>(name); };
    dlg->grab().save(QDir::tempPath() + "/ls9-drafts.png");

    // Compare: the existing Compare view opens with the draft's text against the document
    bool compareSeen = false;
    int pairs = -1;
    QTimer::singleShot(1500, [&] {
        if (auto *cd = qobject_cast<CompareDialog *>(QApplication::activeModalWidget())) {
            compareSeen = true;
            pairs = int(cd->result().pairs.size());
            cd->grab().save(QDir::tempPath() + "/ls9-compare.png");
            cd->reject();
        }
    });
    button("draftCompare")->click();
    check(compareSeen && pairs >= 1, QStringLiteral("Compare opens the Compare view with the draft's text and the document"), QStringLiteral("seen=%1 pairs=%2").arg(compareSeen).arg(pairs));

    // Keep as Copy: a new document beside the original, the original unchanged
    const QByteArray original = readAll(m.folder + "/content/" + m.ch1 + "/text.md");
    button("draftKeep")->click();
    check(waitFor([&] { return projectFileHasTitle(m.folder, "Chapter One (draft)"); }, 10000), QStringLiteral("Keep as Copy makes a new document beside the original"), note ? note->text() : QString());
    const QString copyId = idOfTitle(m.folder, "Chapter One (draft)");
    check(waitFor([&] { return !copyId.isEmpty() && readAll(m.folder + "/content/" + copyId + "/text.md").contains("The draft's own words"); }, 10000),
          QStringLiteral("... holding the draft's text, committed by the host"));
    check(readAll(m.folder + "/content/" + m.ch1 + "/text.md") == original, QStringLiteral("the original is unchanged"));
    check(note && note->text().contains("Chapter One (draft)"), QStringLiteral("the window says what was done"), note ? note->text() : QString());

    // Take Over: the lock, a snapshot first, then the draft's text as a change
    check(select("Chapter One"), QStringLiteral("the draft is still in the list (drafts are never deleted by themselves)"));
    answer = true;
    confirms = 0;
    button("draftTake")->click();
    check(confirms == 1, QStringLiteral("Take Over asks first"));
    check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("The draft's own words"); }, 15000), QStringLiteral("the draft's text is now the document's text"), note ? note->text() : QString());
    check(snapshotTitles(m.folder, m.ch1).contains("Before taking over a recovery draft"), QStringLiteral("a snapshot of the text before was made first"), snapshotTitles(m.folder, m.ch1).join(" | "));
    check(select("Chapter One"), QStringLiteral("the draft stays in the list after it was taken over"));
    w.grab().save(QDir::tempPath() + "/ls9-window.png");

    // Delete asks
    const int count = tree->topLevelItemCount();
    answer = false;
    confirms = 0;
    select("Chapter One");
    button("draftDelete")->click();
    check(confirms == 1 && tree->topLevelItemCount() == count, QStringLiteral("Delete asks, and a No deletes nothing"));
    answer = true;
    button("draftDelete")->click();
    check(confirms == 2 && tree->topLevelItemCount() == count - 1, QStringLiteral("a Yes deletes that draft"), QString::number(tree->topLevelItemCount()));
    select("Chapter Two");
    button("draftDelete")->click();
    check(tree->topLevelItemCount() == count - 2 && H->svc->drafts(uuid).isEmpty(), QStringLiteral("a draft of the outbox is deleted too"), QString::number(H->svc->drafts(uuid).size()));
    check(H->svc->authority(uuid)->drafts().list().isEmpty(), QStringLiteral("nothing is left of it on disk"));

    // no window has the project open: the window says so
    Draft d2 = d;
    d2.forkSeq = 9;
    d2.id = RecoveryDrafts::idFor(uuid, d2.forkEpoch, d2.forkSeq, d2.document, d2.author);
    H->svc->authority(uuid)->drafts().add(d2, draftText, QByteArray(), &err);
    check(w.closeProject(), QStringLiteral("the window closes the project"));
    waitFor([&] { return tree->topLevelItemCount() == 1; }, 1000);
    dlg->setProject(uuid);
    check(select("Chapter One"), QStringLiteral("a draft is listed with no project open"));
    button("draftKeep")->click();
    check(note && note->text().contains("Open the project"), QStringLiteral("Keep as Copy with no window of the project says to open it first"), note ? note->text() : QString());
    check(!projectFileHasTitle(m.folder, "Chapter One (draft) (draft)"), QStringLiteral("and changes nothing"));
    dlg->close();
    livesync::dialogHooks().confirm = nullptr;
    livesync::setLiveSyncControl(nullptr);
    ui::setInProcessLiveSyncService(nullptr);
}

// ---------------------------------------------------------------- 3b. closing the last window of a hosted project

static void testClose(const QString &tmp)
{
    std::printf("== closing the last window of a hosted project\n");
    if (!net::DeviceIdentity::available() || !net::TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: skipped)"));
        return;
    }
    auto H = makeDev(tmp, "H9d");
    auto C = makeDev(tmp, "C9d");
    check(pair(*H, *C), QStringLiteral("the devices pair"));
    Made m = makeProject(tmp + "/closeproj", "Closing");
    QString err;
    const QString uuid = H->svc->shareProject(m.folder, &err);
    H->svc->setAccess(uuid, C->deviceId(), Role::Editor);
    C->svc->joinProject(H->deviceId(), uuid, "Closing", "127.0.0.1", H->svc->port(), &err);
    check(waitFor([&] { return C->svc->clientSession(uuid) && C->svc->clientSession(uuid)->state() == ClientSession::State::Ready; }, 25000), QStringLiteral("the other device is online"), err);
    check(waitFor([&] { return H->svc->hostSession(uuid) && !H->svc->hostSession(uuid)->joinedPeers().isEmpty(); }, 5000), QStringLiteral("the host sees it"));
    livesync::ServiceControl control(H->svc.get());
    livesync::setLiveSyncControl(&control);
    ui::setInProcessLiveSyncService(H->svc.get());
    int asked = 0;
    bool answer = false;
    livesync::dialogHooks().confirm = [&](QWidget *, const QString &, const QString &, const QString &) { ++asked; return answer; };
    auto *w = new MainWindow;
    w->resize(1200, 800);
    w->show();
    waitFor([&] { return w->findChild<livesync::LiveSyncUi *>() != nullptr; }, 2000);
    check(w->openProject(Project::resolveProjectFile(m.folder)) && w->project() && w->project()->isFollower(), QStringLiteral("the window opens the hosted project"));
    check(!w->close() && asked == 1 && w->project() != nullptr, QStringLiteral("closing it asks first (another device is working on the project), and a No keeps it open"), QString::number(asked));
    answer = true;
    asked = 0;
    check(w->close() && asked == 1, QStringLiteral("a Yes closes it"), QString::number(asked));
    delete w;
    livesync::setPrefKeepHosting(true); // (the window tells the service the writer's choice when it attaches)
    H->svc->setKeepHosting(true);
    asked = 0;
    auto *w2 = new MainWindow;
    w2->show();
    waitFor([&] { return w2->findChild<livesync::LiveSyncUi *>() != nullptr; }, 2000);
    w2->openProject(Project::resolveProjectFile(m.folder));
    const bool closed2 = w2->close();
    check(closed2 && asked == 0, QStringLiteral("with \"Keep hosting\" on nothing is asked"), QStringLiteral("closed=%1 asked=%2").arg(closed2).arg(asked));
    delete w2;
    livesync::setPrefKeepHosting(false);
    livesync::dialogHooks().confirm = nullptr;
    livesync::setLiveSyncControl(nullptr);
    ui::setInProcessLiveSyncService(nullptr);
}

// ---------------------------------------------------------------- 4. keep hosting after the last window

static void testKeepHosting(const QString &tmp, const QString &app)
{
    std::printf("== keep hosting when the last window is closed\n");
    if (!net::DeviceIdentity::available() || app.isEmpty() || !QFileInfo::exists(app)) {
        check(true, QStringLiteral("(no Botan TLS or the program's file is not here: the process checks are skipped)"));
        return;
    }
    // this program and the service process it starts are one device: one data folder, one key file
    const QString xdg = tmp + "/kh-xdg";
    QDir().mkpath(xdg);
    qputenv("XDG_DATA_HOME", xdg.toUtf8());
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QB_LS_PORT", "0");
    qputenv("QB_LS_NO_DISCOVERY", "1");
    qputenv("QB_LS_KEEP_HOSTING", "1");
    Made m = makeProject(tmp + "/khproj", "KeepHosting");
    Service::Config cfg;
    cfg.port = 0;
    cfg.discovery = false;
    Service svc(cfg);
    QString err;
    check(svc.start(&err), QStringLiteral("this program's service starts"), err);
    svc.setKeepHosting(true);
    const QString uuid = svc.shareProject(m.folder, &err);
    check(!uuid.isEmpty() && svc.isHosting(uuid), QStringLiteral("it hosts a project"), err);
    livesync::ServiceControl control(&svc);
    const QString name = svc.localName();
    check(control.handOffHosting(app), QStringLiteral("with the last window closed and the choice on, the hosting is handed to a service process"));
    check(!svc.isRunning() && !svc.isHosting(uuid), QStringLiteral("this program's service no longer hosts"));
    check(waitFor([&] { return LocalServer::isServed(name, 200); }, 30000), QStringLiteral("a service process answers on the device's socket"));
    LocalLink *link = LocalLink::connectTo(name);
    waitFor([&] { return link->state() == net::Link::State::Ready; }, 3000);
    ServiceClient client(link);
    ServiceClient::Attached a;
    waitFor([&] {
        a = client.attach(m.folder, false, 3000);
        return a.ok;
    }, 20000);
    check(a.ok && a.host, QStringLiteral("a window attaches: the process hosts the project"), a.code);
    check(waitFor([&] { client.attach(m.folder, false); return client.replicaReady(); }, 15000), QStringLiteral("its session is ready"));
    auto p = Project::open(m.folder, &err, client.follower());
    check(bool(p), QStringLiteral("the project opens as a follower"), err);
    if (p) {
        client.lock({"doc:" + m.ch1});
        check(waitFor([&] { return client.holds("doc:" + m.ch1); }, 8000), QStringLiteral("the lock comes from the service process"));
        client.follower()->capture().setListener([&] { QMetaObject::invokeMethod(&client, [&] { client.sendCaptured(); }, Qt::QueuedConnection); });
        p->docs()->text(m.ch1)->setPlainText(QStringLiteral("Hosted on after the last window closed."));
        p->save(&err);
        check(waitFor([&] { return readAll(m.folder + "/content/" + m.ch1 + "/text.md").contains("Hosted on after"); }, 15000),
              QStringLiteral("the service process commits the change: hosting goes on"));
        client.unlock({"doc:" + m.ch1});
        p.reset();
    }
    client.detach();
    link->close();
    // with no window the process stays (the choice is on): it is still there after its idle time
    waitFor([&] { return false; }, 2500);
    check(LocalServer::isServed(name, 300), QStringLiteral("with no window the process keeps running while it hosts"));
    // a program that starts now takes the hosting back: it asks the process to end, then hosts itself
    check(control.takeBackFromServiceProcess(), QStringLiteral("a program that starts asks the process to end"));
    check(!LocalServer::isServed(name, 300), QStringLiteral("the process is gone"));
    QString err2;
    check(svc.start(&err2), QStringLiteral("this program's service starts again"), err2);
    check(svc.isHosting(uuid), QStringLiteral("and hosts the project again"));
    // a service of a program (no process) is not asked to end
    LocalLink *l2 = LocalLink::connectTo(name);
    waitFor([&] { return l2->state() == net::Link::State::Ready; }, 3000);
    bool answered = false, stopping = true;
    QObject::connect(l2, &net::Link::frameReceived, l2, [&](const net::Frame &f) {
        bool ok = false;
        const QJsonObject q = f.object(&ok);
        if (ok && local::typeOf(q) == QLatin1String(local::PONG) && q.value("re").toString() == "stop") {
            answered = true;
            stopping = q.value("stopping").toBool();
        }
    });
    l2->send(net::Frame::json(local::msg(local::STOP, {{"id", "stop"}})));
    waitFor([&] { return answered; }, 3000);
    check(answered && !stopping && svc.isRunning() && svc.isHosting(uuid), QStringLiteral("a service that lives inside a program ignores the request to end"));
    delete l2;
    svc.stop();
}

// ---------------------------------------------------------------- main

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_UPDATE_CHECK", "1");
    qputenv("QB_LS_STRICT", "0");
    qputenv("QB_TEST_KEYSTORE", ("file:" + home.path() + "/keys.json").toUtf8());
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
    const QString program = QStringLiteral(QB_TEST_BUILD_DIR "/quillbinder");
    const QString only = qEnvironmentVariable("QB_LS9_PART");
    auto want = [&](const char *n) { return only.isEmpty() || only == QLatin1String(n); };
    if (want("wiring"))
        testWiring(tmp);
    if (want("asks"))
        testEditRequests(tmp);
    if (want("drafts"))
        testDrafts(tmp);
    if (want("close"))
        testClose(tmp);
    if (want("hosting"))
        testKeepHosting(tmp, program);
    std::printf("\n%d check(s)\n%d failure(s)\n", total, failures); // (the runner reads a line that starts with "<n> failure(s)")
    return failures;
}
