// Live Sync, work package 8 (0.99.0): the host's backups and snapshot jobs, the backup state a client sees, a security sweep over the
// network code, and (with --measure <folder>) the measurement of the time from an edit to the confirmed copy on another device.
//
//   QB_SAMPLE=0 QB_SUITES=ls8 tools/run-all-tests.sh build/test-results-x11 x11       (or ./qb_ls8_tests /tmp/shots)
//   ./qb_ls8_tests --measure /tmp/ls8-measure                    three programs over TLS on localhost; writes results.txt
// Exit code = number of failed checks.
#include "core/Backup.h"
#include "core/BinderItem.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/Snapshots.h"
#include "core/Zip.h"
#include "core/collab/Blobs.h"
#include "core/collab/ClientSession.h"
#include "core/collab/Clock.h"
#include "core/collab/Gate.h"
#include "core/collab/HostBackup.h"
#include "core/collab/HostSession.h"
#include "core/collab/Manifest.h"
#include "core/collab/PathClass.h"
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/net/Frames.h"
#include "core/collab/net/MemLink.h"
#include "core/collab/net/TlsLink.h"
#include "core/collab/proto/Messages.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QProcess>
#include <QRandomGenerator>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <memory>

using namespace qb;
using namespace qb::collab;
using namespace qb::collab::proto;
using qb::collab::net::Frame;
using qb::collab::net::MemLink;

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
    while (!cond() && t.elapsed() < ms) {
        QEventLoop loop;
        QTimer::singleShot(2, &loop, &QEventLoop::quit);
        loop.exec();
    }
    return cond();
}
static void settle(int rounds = 60)
{
    for (int i = 0; i < rounds; ++i) {
        QEventLoop loop;
        QTimer::singleShot(0, &loop, &QEventLoop::quit);
        loop.exec();
    }
}
static QByteArray readBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
static bool writeBytes(const QString &path, const QByteArray &b)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(b) == b.size();
}
static qint64 monoNs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return qint64(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}

static Project *g_project = nullptr; // the host's project of the world under test: where its snapshots are
static const char *kUuid = "11111111-2222-3333-4444-555555555555";

// the replicated files of a folder: path -> hash
static QMap<QString, QString> filesOf(const QString &folder)
{
    Manifest m;
    Manifest::scan(folder, QString(), {}, &m);
    QMap<QString, QString> out;
    for (const ManifestEntry &e : m.entries)
        out.insert(e.path, e.sha256);
    return out;
}

// ---------------------------------------------------------------- a shared project with a host, jobs and clients

struct Client {
    QString name, device, folder;
    QByteArray pin;
    std::unique_ptr<ClientSession> s;
    std::unique_ptr<HostJobsClient> jobs;
    MemLink *toHost = nullptr, *atHost = nullptr;
    int committed = 0;
    QStringList errors;
};

struct World {
    QTemporaryDir dir;
    ManualClock clock;
    std::unique_ptr<Project> proj;
    QString hostFolder, backupDir;
    QStringList docs; // the documents' ids
    std::unique_ptr<HostSession> host;
    std::unique_ptr<HostBackup> jobs;
    QList<std::shared_ptr<Client>> clients;
    QMap<qint64, QMap<QString, QString>> states; // the host's replicated files after each commit
    QStringList refusals;

    explicit World(int docCount = 3, qint64 bigBytes = 0)
    {
        QString err;
        proj = Project::create(dir.path() + "/host", QStringLiteral("Shared"), &err);
        for (int i = 0; i < docCount; ++i) {
            BinderItem *it = proj->model()->createItem(proj->draft(), -1, ItemType::Text, QStringLiteral("Chapter %1").arg(i + 1));
            docs << it->id;
            proj->docs()->text(it->id)->setPlainText(QStringLiteral("Text of chapter %1.\n").arg(i + 1));
        }
        proj->save(&err);
        proj->docs()->saveDirty(&err);
        hostFolder = proj->folderPath();
        g_project = proj.get();
        backupDir = dir.path() + "/backups";
        if (bigBytes > 0) {
            QByteArray big(int(bigBytes), 0);
            for (int i = 0; i < big.size(); ++i)
                big[i] = char(QRandomGenerator::global()->bounded(256));
            writeBytes(hostFolder + "/content/" + docs.first() + "/file.bin", big);
        }
    }
    bool startHost()
    {
        HostSession::Options o;
        o.folder = hostFolder;
        o.uuid = QString::fromLatin1(kUuid);
        o.hostName = QStringLiteral("Host");
        o.clock = &clock;
        host = std::make_unique<HostSession>(o);
        QObject::connect(host.get(), &HostSession::refused, host.get(), [this](const QString &, const QString &c) { refusals << c; });
        QString err;
        const bool ok = host->start(&err);
        if (!ok)
            std::printf("      host start: %s\n", qPrintable(err));
        HostBackup::Config cfg;
        cfg.host = host.get();
        cfg.project = proj.get();
        cfg.backupDir = backupDir;
        cfg.keep = 3;
        jobs = std::make_unique<HostBackup>(cfg);
        QObject::connect(jobs.get(), &HostBackup::requestRefused, host.get(), [this](const QString &, const QString &c) { refusals << c; });
        host->setExtensionHandler([this](const QString &d, net::Link *l, const QJsonObject &m) { return jobs->handle(d, l, m); });
        host->setSnapshotHook(jobs->protectiveSnapshotHook());
        QObject::connect(host.get(), &HostSession::committed, host.get(), [this](qint64 seq, const QString &, const QString &) { states.insert(seq, filesOf(hostFolder)); });
        states.insert(host->seq(), filesOf(hostFolder));
        return ok;
    }
    std::shared_ptr<Client> client(const QString &name, Role role = Role::Editor)
    {
        auto c = std::make_shared<Client>();
        c->name = name;
        c->pin = QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha256);
        c->device = MemLink::deviceIdOfPin(c->pin);
        c->folder = dir.path() + "/c-" + name;
        host->setRole(c->device, role);
        ClientSession::Config cfg;
        cfg.folder = c->folder;
        cfg.uuid = QString::fromLatin1(kUuid);
        cfg.deviceId = c->device;
        cfg.deviceName = name;
        cfg.clock = &clock;
        c->s = std::make_unique<ClientSession>(cfg);
        c->jobs = std::make_unique<HostJobsClient>(c->s.get(), QString::fromLatin1(kUuid));
        Client *p = c.get();
        c->s->setExtensionHandler([p](const QJsonObject &m) { return p->jobs->handle(m); });
        QObject::connect(c->s.get(), &ClientSession::committed, c->s.get(), [p](const QString &, qint64) { ++p->committed; });
        QObject::connect(c->s.get(), &ClientSession::problem, c->s.get(), [p](const QString &, const QString &code) { p->errors << code; });
        QObject::connect(c->s.get(), &ClientSession::readyToUse, c->s.get(), [p] { p->jobs->queryStatus(); });
        c->s->open();
        clients << c;
        return c;
    }
    bool join(Client &c)
    {
        const QByteArray hostPin = QCryptographicHash::hash("host", QCryptographicHash::Sha256);
        const MemLink::Pair pr = MemLink::createPair(hostPin, c.pin, host.get());
        c.atHost = pr.a;
        c.toHost = pr.b;
        host->addLink(c.atHost);
        c.s->attach(c.toHost);
        return waitFor([&] { return c.s->state() == ClientSession::State::Ready; });
    }
    void advance(qint64 ms)
    {
        for (qint64 done = 0; done < ms; done += 1000) {
            clock.advance(qMin<qint64>(1000, ms - done));
            settle(10);
        }
        settle();
    }
    QString textPath(const QString &id) const { return hostFolder + "/content/" + id + "/text.md"; }
    QString newestBackup() const
    {
        const QStringList all = backup::backupsFor(proj.get(), backupDir);
        return all.isEmpty() ? QString() : all.last();
    }
};

static bool lockOn(Client &c, const QString &res)
{
    c.s->lock({res});
    return waitFor([&] { return c.s->holds(res); }, 3000);
}
static Unit textUnit(const QString &id, const QByteArray &text, QHash<QString, QByteArray> *blobs)
{
    Unit u;
    u.name = "doc:" + id;
    FileOp f;
    f.path = QString("content/%1/text.md").arg(id);
    f.hash = sha256Hex(text);
    f.size = text.size();
    u.files << f;
    blobs->insert(f.hash, text);
    return u;
}
static QString commitText(Client &c, const QString &id, const QByteArray &text)
{
    QHash<QString, QByteArray> blobs;
    return c.s->submit({textUnit(id, text, &blobs)}, blobs);
}
static bool waitState(HostBackup &j, BackupStatus::State s, int ms = 30000)
{
    return waitFor([&] { return j.status().state == s; }, ms);
}

// ---------------------------------------------------------------- 1. a backup during commits is one commit state

static void testBackupDuringCommits()
{
    std::printf("== a backup during commits\n");
    World w(3, qint64(6) << 20);
    check(w.startHost(), QStringLiteral("the host starts"));
    auto a = w.client("devA");
    check(w.join(*a) && lockOn(*a, "doc:" + w.docs.at(1)), QStringLiteral("a client joins and locks a document"));
    // the copy itself: whole at one commit, made by links
    {
        const FreezeResult f = HostBackup::freeze(w.host.get(), w.hostFolder + "/livesync", false, nullptr);
        check(f.ok && f.seq == w.host->seq() && f.linked > 0 && f.copied == 0, QStringLiteral("the copy is made by hard links and holds the commit the host is at"),
              QString::number(f.linked) + "/" + QString::number(f.copied) + " " + f.why);
        const bool same = [&] {
            QMap<QString, QString> got = filesOf(f.folder), want = w.states.value(f.seq);
            return got == want;
        }();
        check(same, QStringLiteral("the copy equals the host's files at that commit"));
        check(!QFileInfo::exists(f.folder + "/livesync") && !QFileInfo::exists(f.folder + "/livesync-local") && !QFileInfo::exists(f.folder + "/settings/journal.json"),
              QStringLiteral("Live Sync's own folders and the journal are not in the copy"));
        // a linked file never changes when the host replaces the original
        const QString rel = "content/" + w.docs.at(1) + "/text.md";
        const QByteArray before = readBytes(f.folder + "/" + rel);
        commitText(*a, w.docs.at(1), "changed after the copy was made\n");
        waitFor([&] { return a->committed >= 1; });
        check(readBytes(f.folder + "/" + rel) == before && readBytes(w.textPath(w.docs.at(1))) != before, QStringLiteral("a commit after the copy leaves the copy as it was"));
        QDir(f.folder).removeRecursively();
        const FreezeResult c = HostBackup::freeze(w.host.get(), w.hostFolder + "/livesync", true, nullptr);
        check(c.ok && c.copied > 0 && c.linked == 0 && filesOf(c.folder) == filesOf(w.hostFolder), QStringLiteral("where links are not used the copy is made by copying, equal to the folder"));
        QDir(c.folder).removeRecursively();
    }
    // a backup while changes keep coming
    const qint64 seqBefore = w.host->seq();
    QString why;
    check(w.jobs->requestBackup(&why) && w.jobs->running(), QStringLiteral("the host starts a backup"), why);
    int n = 0;
    while (w.jobs->running() && n < 400) {
        commitText(*a, w.docs.at(1), QByteArray("typing ") + QByteArray::number(n) + QByteArray(40000, 'x') + "\n");
        waitFor([&] { return a->committed >= n + 2; }, 3000);
        ++n;
    }
    check(waitState(*w.jobs, BackupStatus::Done), QStringLiteral("the backup ends as done"), QString::number(int(w.jobs->status().state)) + " " + w.jobs->status().detail);
    const qint64 s = w.jobs->statusSeq();
    check(s >= seqBefore && s < w.host->seq() && w.states.contains(s), QStringLiteral("commits went on while the copy was packed (the backup holds commit %1 of %2)").arg(s).arg(w.host->seq()));
    check(w.jobs->status().lastSuccess.isValid() && !QFileInfo::exists(w.hostFolder + QString("/livesync/freeze-%1").arg(s)), QStringLiteral("the time of the last success is kept and the copy is removed"));
    const QString zip = w.newestBackup();
    check(!zip.isEmpty(), QStringLiteral("the backup file is in the backup folder"));
    {
        ZipReader zr(zip);
        bool leak = false;
        for (const QString &nme : zr.fileNames())
            leak = leak || nme.contains("/livesync") || nme.contains("livesync-replica");
        check(zr.isValid() && !leak, QStringLiteral("the ZIP holds none of Live Sync's own files"));
    }
    QString projFile, err;
    const QString restored = backup::restore(zip, w.dir.path() + "/restored", &projFile, &err);
    QDir().mkpath(w.dir.path() + "/restored");
    const QString restored2 = restored.isEmpty() ? backup::restore(zip, w.dir.path() + "/restored", &projFile, &err) : restored;
    check(!restored2.isEmpty() && !projFile.isEmpty(), QStringLiteral("the backup restores"), err);
    QMap<QString, QString> got = filesOf(QFileInfo(projFile).absolutePath());
    got.remove("settings/backup-id.txt"); // the restored project's own id
    check(got == w.states.value(s), QStringLiteral("the restored files are exactly the host's files at that one commit (not a mixture)"),
          QString("restored %1 files, commit %2 %3 files").arg(got.size()).arg(s).arg(w.states.value(s).size()));
    // as a new project
    check(!QFileInfo::exists(QFileInfo(projFile).absolutePath() + "/livesync") && !QFileInfo::exists(QFileInfo(projFile).absolutePath() + "/livesync-replica.json"),
          QStringLiteral("a restored backup carries no Live Sync identity: it is a new project"));
    QString oe;
    auto rp = Project::open(projFile, &oe);
    check(rp && backup::projectId(rp.get()) != backup::projectId(w.proj.get()), QStringLiteral("it has a backup id of its own"), oe);
    check(w.host->seq() >= s && w.host->uuid() == QString::fromLatin1(kUuid), QStringLiteral("restoring touched nothing of the shared project"));
}

// ---------------------------------------------------------------- 2. the client asks and sees the state

static void testBackupState()
{
    std::printf("== the backup state\n");
    World w(2);
    w.startHost();
    auto a = w.client("devA");
    auto r = w.client("devR", Role::Reader);
    w.join(*a);
    w.join(*r);
    waitFor([&] { return a->jobs->status().state == BackupStatus::None; }, 500);
    check(a->jobs->status().state == BackupStatus::None && !a->jobs->status().lastSuccess.isValid(), QStringLiteral("before any backup the state is none and there is no last success"));
    check(a->jobs->requestBackup(), QStringLiteral("a client asks for a backup"));
    check(waitFor([&] { return a->jobs->status().state == BackupStatus::Running; }, 3000) || a->jobs->status().state == BackupStatus::Done, QStringLiteral("it sees the backup running"));
    check(waitFor([&] { return a->jobs->status().state == BackupStatus::Done; }, 30000) && a->jobs->status().lastSuccess.isValid(), QStringLiteral("then done, with the time of the last success"));
    check(!w.newestBackup().isEmpty(), QStringLiteral("the backup was made by the host"));
    check(!QFileInfo::exists(a->folder + "/livesync") && QDir(w.backupDir).exists() && !QDir(a->folder).entryList({"*.zip"}).size(), QStringLiteral("nothing was written on the client"));
    // the other client was told too (it asked for the state when it became ready)
    r->jobs->queryStatus();
    check(waitFor([&] { return r->jobs->status().state == BackupStatus::Done && r->jobs->status().lastSuccess.isValid(); }, 3000), QStringLiteral("a reader sees the state as well"));
    // asking again at once: the host answers HOST_BUSY, nothing starts
    const int backups = backup::backupsFor(w.proj.get(), w.backupDir).size();
    a->jobs->requestBackup();
    settle();
    check(w.refusals.contains("HOST_BUSY") && backup::backupsFor(w.proj.get(), w.backupDir).size() == backups && a->jobs->status().state == BackupStatus::Done,
          QStringLiteral("a second request within a few seconds is refused (HOST_BUSY) and starts nothing"));
    w.advance(11000);
    // a reader may ask too; the host is the one that writes
    check(r->jobs->requestBackup() && waitFor([&] { return backup::backupsFor(w.proj.get(), w.backupDir).size() == backups + 1; }, 30000), QStringLiteral("a reader may ask for a backup"));
    // a device that has no access gets nothing
    auto none = w.client("devN", Role::None);
    w.join(*none);
    const int before = backup::backupsFor(w.proj.get(), w.backupDir).size();
    check(none->s->state() != ClientSession::State::Ready, QStringLiteral("a device with no access does not join"));
    w.refusals.clear();
    const MemLink::Pair pr = MemLink::createPair(QCryptographicHash::hash("host", QCryptographicHash::Sha256), QCryptographicHash::hash("devX", QCryptographicHash::Sha256), w.host.get());
    w.host->addLink(pr.a);
    pr.b->sendJson(make("HELLO", QString::fromLatin1(kUuid), QJsonObject{{"protoMin", 1}, {"protoMax", 1}, {"name", "x"}}));
    pr.b->sendJson(make("BACKUP_REQUEST", QString::fromLatin1(kUuid)));
    settle();
    check(w.refusals.contains("NO_ACCESS") && backup::backupsFor(w.proj.get(), w.backupDir).size() == before, QStringLiteral("a request of a device that has no access is refused and starts nothing"));
}

// ---------------------------------------------------------------- 3. failures are told

static void testBackupFailures()
{
    std::printf("== backups that fail are told\n");
    World w(2, qint64(1) << 20);
    w.startHost();
    QString why;
    check(w.jobs->requestBackup(&why) && waitState(*w.jobs, BackupStatus::Done), QStringLiteral("a first backup is done"), why);
    const QDateTime ok1 = w.jobs->status().lastSuccess;
    const QStringList firstSet = backup::backupsFor(w.proj.get(), w.backupDir);
    // the backup folder is not usable
    w.jobs.reset();
    HostBackup::Config cfg;
    cfg.host = w.host.get();
    cfg.project = w.proj.get();
    cfg.backupDir = w.dir.path() + "/afile/inside";
    writeBytes(w.dir.path() + "/afile", "this is a file, not a folder");
    cfg.keep = 3;
    w.jobs = std::make_unique<HostBackup>(cfg);
    check(w.jobs->status().lastSuccess == ok1 && w.jobs->status().state == BackupStatus::Done, QStringLiteral("the state and the last success are kept over a restart of the host"));
    w.jobs->requestBackup();
    waitFor([&] { return !w.jobs->running(); }, 30000);
    const BackupStatus st = w.jobs->status();
    check((st.state == BackupStatus::Waiting || st.state == BackupStatus::Failed) && st.state != BackupStatus::Done && !st.detail.isEmpty(),
          QStringLiteral("a backup folder that cannot be used gives waiting or failed, with a reason, never done"), QString::number(int(st.state)) + " " + st.detail);
    check(st.lastSuccess == ok1, QStringLiteral("the time of the last success is not moved by it"));
    check(backup::backupsFor(w.proj.get(), w.backupDir) == firstSet, QStringLiteral("the older backups stay"));
    // the copy cannot be made: a file fails to copy
    cfg.backupDir = w.backupDir;
    w.jobs = std::make_unique<HostBackup>(cfg);
    qputenv("QB_TEST_LS_NO_LINKS", "1");
    qputenv("QB_TEST_FAIL_COPY", "Shared.qbx");
    w.jobs->requestBackup();
    waitFor([&] { return !w.jobs->running(); }, 30000);
    check(w.jobs->status().state == BackupStatus::Failed && w.jobs->status().lastSuccess == ok1 && backup::backupsFor(w.proj.get(), w.backupDir) == firstSet
              && !QDir(w.hostFolder + "/livesync").entryList({"freeze-*"}, QDir::Dirs).size(),
          QStringLiteral("a copy that cannot be made fails the backup: no file, no false success, no copy left behind"), w.jobs->status().detail);
    qunsetenv("QB_TEST_FAIL_COPY");
    // not enough room for a copy by copying
    HostBackup::Config small = cfg;
    small.freeBytes = [] { return qint64(1000); };
    w.jobs = std::make_unique<HostBackup>(small);
    w.jobs->requestBackup();
    waitFor([&] { return !w.jobs->running(); }, 30000);
    check(w.jobs->status().state == BackupStatus::Failed && w.jobs->status().detail.contains("space"), QStringLiteral("a full disk is told as failed, with the reason"), w.jobs->status().detail);
    qunsetenv("QB_TEST_LS_NO_LINKS");
    // files that cannot be read: incomplete, and the older complete backups stay
    qputenv("QB_TEST_UNREADABLE", "/text.md");
    w.jobs = std::make_unique<HostBackup>(cfg);
    w.jobs->requestBackup();
    waitFor([&] { return !w.jobs->running(); }, 30000);
    check(w.jobs->status().state == BackupStatus::Incomplete && w.jobs->status().lastSuccess == ok1, QStringLiteral("unreadable files give an incomplete backup, which is not a success"), QString::number(int(w.jobs->status().state)) + " " + w.jobs->status().lastSuccess.toString(Qt::ISODateWithMs) + " vs " + ok1.toString(Qt::ISODateWithMs));
    for (const QString &f : firstSet)
        check(QFileInfo::exists(f), QStringLiteral("an older complete backup is still there"));
    qunsetenv("QB_TEST_UNREADABLE");
    // the host closed in the middle of a backup
    writeBytes(w.hostFolder + "/livesync/backup-state.json", R"({"state":"running","lastOk":0,"code":"","detail":"","seq":0})");
    HostBackup again(cfg);
    check(again.status().state == BackupStatus::Failed && !again.status().detail.isEmpty(), QStringLiteral("a backup that was running when the host closed is shown as failed, not as running"));
}

// ---------------------------------------------------------------- 4. snapshots are jobs of the host

static QString snapRel(const QString &doc) { return QDir(g_project->folderPath()).relativeFilePath(g_project->snapshotsDir(doc)); }
static int snapshotCount(const QString &folder, const QString &doc)
{
    return QDir(folder + "/" + snapRel(doc)).entryList({"*.md", "*.rtf"}, QDir::Files).size();
}

static void testSnapshots()
{
    std::printf("== snapshot jobs\n");
    World w(3);
    w.startHost();
    auto a = w.client("devA");
    auto b = w.client("devB");
    auto r = w.client("devR", Role::Reader);
    w.join(*a);
    w.join(*b);
    w.join(*r);
    const QString doc = w.docs.at(0);
    snapshots::RemoteJobs *ja = a->jobs->remoteJobs();
    QString err;
    const qint64 seq0 = w.host->seq();
    check(ja->take(doc, "First", &err) && snapshotCount(w.hostFolder, doc) == 1, QStringLiteral("a client takes a snapshot: the host makes it"), err);
    check(w.host->seq() == seq0 + 1 && snapshotCount(a->folder, doc) == 1 && snapshotCount(b->folder, doc) == 1, QStringLiteral("it is one commit of the host and reaches both copies"));
    check(waitFor([&] { return filesOf(a->folder) == filesOf(w.hostFolder) && filesOf(b->folder) == filesOf(w.hostFolder); }, 5000), QStringLiteral("the copies equal the host's folder, the list of titles included"));
    const QList<Snapshot> list = snapshots::list(w.proj.get(), doc);
    check(list.size() == 1 && list.first().title == "First", QStringLiteral("the title is kept"));
    const QString file = QFileInfo(list.first().file).fileName();
    // rename
    check(ja->setTitle(doc, file, "Renamed", &err) && snapshots::list(w.proj.get(), doc).first().title == "Renamed", QStringLiteral("renaming is a host job"), err);
    check(waitFor([&] { return readBytes(a->folder + "/" + snapRel(doc) + "/index.json").contains("Renamed") && filesOf(b->folder) == filesOf(w.hostFolder); }, 3000), QStringLiteral("and replicates"));
    // a reader may not
    err.clear();
    check(!r->jobs->remoteJobs()->take(doc, "No", &err) && r->jobs->lastJobCode() == "READ_ONLY" && snapshotCount(w.hostFolder, doc) == 1, QStringLiteral("a reader cannot make a snapshot (READ_ONLY)"), err);
    // another device edits the document: its lock stands in the way
    check(lockOn(*b, "doc:" + doc), QStringLiteral("B locks the document"));
    err.clear();
    check(!ja->take(doc, "Blocked", &err) && a->jobs->lastJobCode() == "LOCK_HELD" && err.contains("devB"), QStringLiteral("a snapshot while another device holds the document is refused (LOCK_HELD)"), err);
    // roll back needs the asker's own lock
    commitText(*b, doc, "B wrote this text.\n");
    waitFor([&] { return b->committed >= 1; });
    err.clear();
    check(!ja->rollBack(doc, file, false, &err) && a->jobs->lastJobCode() == "LOCK_HELD", QStringLiteral("a roll back while B edits is refused"), err);
    b->s->unlock({"doc:" + doc});
    waitFor([&] { return !b->s->holds("doc:" + doc); }, 2000);
    w.advance(1000);
    err.clear();
    check(!ja->rollBack(doc, file, false, &err) && a->jobs->lastJobCode() == "LOCK_NEEDED" && readBytes(w.textPath(doc)) == "B wrote this text.\n", QStringLiteral("a roll back without the asker's own lock is refused (LOCK_NEEDED), the text is not changed"), err);
    check(lockOn(*a, "doc:" + doc), QStringLiteral("A locks the document"));
    err.clear();
    const bool rolled = ja->rollBack(doc, file, false, &err);
    check(rolled && readBytes(w.textPath(doc)).startsWith("Text of chapter 1."), QStringLiteral("a roll back puts the snapshot's text back on the host"), err + " " + readBytes(w.textPath(doc)));
    check(snapshotCount(w.hostFolder, doc) == 2, QStringLiteral("and a protective snapshot of the text as it was comes first"));
    check(waitFor([&] { return readBytes(b->folder + "/content/" + doc + "/text.md") == readBytes(w.textPath(doc)) && filesOf(a->folder) == filesOf(w.hostFolder); }, 5000), QStringLiteral("the rolled back text and the new snapshot replicate"));
    bool hasProtective = false;
    for (const Snapshot &s : snapshots::list(w.proj.get(), doc)) {
        QByteArray bytes;
        QFile f(s.file);
        if (f.open(QIODevice::ReadOnly))
            bytes = f.readAll();
        hasProtective = hasProtective || bytes.contains("B wrote this text.");
    }
    check(hasProtective, QStringLiteral("the protective snapshot holds the text that was replaced"));
    // delete
    const QString newer = QFileInfo(snapshots::list(w.proj.get(), doc).first().file).fileName();
    check(ja->remove(doc, newer, &err) && snapshotCount(w.hostFolder, doc) == 1 && waitFor([&] { return snapshotCount(b->folder, doc) == 1; }, 3000), QStringLiteral("deleting is a host job and replicates"), err);
    // the same job again with the same operation id does it once
    // a protective snapshot before a takeover
    const qint64 seqT = w.host->seq();
    QString tErr;
    check(w.jobs->keepProtectiveSnapshot("doc:" + w.docs.at(1), "Before takeover by devB", &tErr) && snapshotCount(w.hostFolder, w.docs.at(1)) == 1 && w.host->seq() == seqT + 1,
          QStringLiteral("a protective snapshot is one commit of the host"), tErr);
    check(waitFor([&] { return snapshotCount(a->folder, w.docs.at(1)) == 1; }, 3000), QStringLiteral("and replicates"));
    // a real takeover takes it first
    lockOn(*a, "doc:" + w.docs.at(2));
    b->s->askToEdit("doc:" + w.docs.at(2));
    settle();
    w.advance(21000);
    const int shotsBefore = snapshotCount(w.hostFolder, w.docs.at(2));
    b->s->takeOver("doc:" + w.docs.at(2));
    settle();
    check(waitFor([&] { return b->s->holds("doc:" + w.docs.at(2)); }, 3000) && snapshotCount(w.hostFolder, w.docs.at(2)) == shotsBefore + 1 && !a->s->holds("doc:" + w.docs.at(2)),
          QStringLiteral("a takeover makes the protective snapshot first, then the lock changes hands"));
    bool titled = false;
    for (const Snapshot &s : snapshots::list(w.proj.get(), w.docs.at(2)))
        titled = titled || s.title.startsWith("Before takeover by");
    check(titled, QStringLiteral("the protective snapshot says why it was made"));
    // the program's own snapshot calls in a window of a shared project go to the host
    {
        QDir().mkpath(w.dir.path() + "/fcopy");
        const QString fcopy = w.dir.path() + "/fcopy/win";
        QDir().mkpath(fcopy);
        QDirIterator it(a->folder, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString p = it.next();
            const QString rel = QDir(a->folder).relativeFilePath(p);
            if (rel.startsWith("livesync"))
                continue;
            QDir().mkpath(QFileInfo(fcopy + "/" + rel).absolutePath());
            QFile::copy(p, fcopy + "/" + rel);
        }
        Follower fol;
        QString oerr;
        auto fp = Project::open(fcopy, &oerr, &fol);
        check(bool(fp), QStringLiteral("a window opens the shared project as a follower"), oerr);
        if (fp) {
            const int hostBefore = snapshotCount(w.hostFolder, w.docs.at(1));
            const int copyBefore = snapshotCount(fcopy, w.docs.at(1));
            QString te;
            check(!snapshots::take(fp.get(), w.docs.at(1), "No hook", &te) && !te.isEmpty() && snapshotCount(fcopy, w.docs.at(1)) == copyBefore && snapshotCount(w.hostFolder, w.docs.at(1)) == hostBefore,
                  QStringLiteral("without a connection to the host a follower cannot make a snapshot, and writes nothing"), te);
            snapshots::setRemoteJobs(fp->folderPath(), a->jobs->remoteJobs());
            check(snapshots::take(fp.get(), w.docs.at(1), "Through the host", &te) && snapshotCount(w.hostFolder, w.docs.at(1)) == hostBefore + 1 && snapshotCount(fcopy, w.docs.at(1)) == copyBefore,
                  QStringLiteral("with the host's hook the program's own call becomes a host job; the follower's folder is not written"), te);
            QString re2;
            check(snapshots::removeAll(fp.get(), &re2) == 0 && !re2.isEmpty(), QStringLiteral("Delete Old Snapshots is not done by a follower"));
            snapshots::setRemoteJobs(fp->folderPath(), nullptr);
        }
    }
}

// the job's files may be on disk without their commit when the host was stopped: the next start publishes them
static void testJobRecovery()
{
    std::printf("== a snapshot job that was cut off\n");
    World w(2);
    w.startHost();
    auto a = w.client("devA");
    w.join(*a);
    const QString doc = w.docs.at(0);
    // the files of a job are on disk and the mark is there, the commit is not
    QString err;
    snapshots::take(w.proj.get(), doc, "Cut off", &err);
    writeBytes(w.hostFolder + "/livesync/job-pending.json", "{\"opId\":\"cutoff-0001\"}");
    const qint64 seq = w.host->seq();
    check(snapshotCount(a->folder, doc) == 0, QStringLiteral("the copy has not heard of the snapshot"));
    check(w.jobs->recoverJobs(&err) && w.host->seq() == seq + 1 && !QFileInfo::exists(w.hostFolder + "/livesync/job-pending.json"), QStringLiteral("at the next start the job's files become a commit"), err);
    check(waitFor([&] { return snapshotCount(a->folder, doc) == 1 && filesOf(a->folder) == filesOf(w.hostFolder); }, 5000), QStringLiteral("and reach the copy"));
    check(w.jobs->recoverJobs(&err) && w.host->seq() == seq + 1, QStringLiteral("nothing is committed twice"));
}

// ---------------------------------------------------------------- 5. a restore is always a new project

static void testRestoreNewProject(const QString &tmp)
{
    std::printf("== restoring\n");
    const QString src = tmp + "/handmade/Proj.quillbinder";
    QDir().mkpath(src + "/livesync/log");
    QDir().mkpath(src + "/settings");
    writeBytes(src + "/Proj.qbx", "<?xml version=\"1.0\"?><QuillbinderProject version=\"1\" title=\"Proj\"><Binder></Binder></QuillbinderProject>");
    writeBytes(src + "/livesync/state.json", "{\"v\":1,\"p\":\"abc\"}");
    writeBytes(src + "/livesync-replica.json", "{\"p\":\"abc\"}");
    writeBytes(src + "/settings/backup-id.txt", "seed");
    {
        ZipWriter zw(tmp + "/handmade.zip");
        // a hand-made ZIP file may hold anything: the program's own packing leaves Live Sync's files out, a restore removes them
        zw.addFile("Proj.quillbinder/Proj.qbx", readBytes(src + "/Proj.qbx"));
        zw.addFile("Proj.quillbinder/livesync/state.json", "{\"v\":1,\"p\":\"abc\"}");
        zw.addFile("Proj.quillbinder/livesync-replica.json", "{\"p\":\"abc\"}");
        zw.addFile("Proj.quillbinder/settings/backup-id.txt", "seed");
        zw.close();
    }
    QString pf, err;
    const QString dest = backup::restore(tmp + "/handmade.zip", tmp + "/rest", &pf, &err);
    const QString folder = pf.isEmpty() ? QString() : QFileInfo(pf).absolutePath();
    check(!dest.isEmpty() && !folder.isEmpty() && !QFileInfo::exists(folder + "/livesync") && !QFileInfo::exists(folder + "/livesync-replica.json"),
          QStringLiteral("a ZIP file with Live Sync's own files restores without them: the new project has no identity of the shared one"), err);
    check(QString::fromUtf8(readBytes(folder + "/settings/backup-id.txt")) != "seed", QStringLiteral("and it gets a backup id of its own"));
}

// ---------------------------------------------------------------- 6. the security sweep

static void testSecurity(const QString &tmp)
{
    std::printf("== security sweep\n");
    // ---- an unpaired device and a wrong certificate (TLS)
    using namespace qb::collab::net;
    if (!DeviceIdentity::available() || !TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the TLS cases are skipped)"));
    } else {
        auto hostId = DeviceIdentity::createEphemeral("host");
        auto knownId = DeviceIdentity::createEphemeral("known");
        auto strangerId = DeviceIdentity::createEphemeral("stranger");
        auto otherHostId = DeviceIdentity::createEphemeral("other");
        TrustPolicy trust;
        trust.decide = [&](const QByteArray &pin, bool) { return pin == knownId->pin() ? PeerInfo::Trust::Paired : PeerInfo::Trust::Unknown; };
        TlsServer server(hostId.get(), trust);
        TlsServer fake(otherHostId.get(), TrustPolicy::pinned(knownId->pin()));
        check(server.listen(QHostAddress::LocalHost, 0) && fake.listen(QHostAddress::LocalHost, 0), QStringLiteral("two listeners"));
        int served = 0, fakeServed = 0;
        QObject::connect(&server, &TlsServer::newLink, &server, [&](TlsLink *l) {
            QObject::connect(l, &Link::stateChanged, l, [&served, l](Link::State s) { if (s == Link::State::Ready) ++served; });
        });
        QObject::connect(&fake, &TlsServer::newLink, &fake, [&](TlsLink *l) {
            QObject::connect(l, &Link::stateChanged, l, [&fakeServed, l](Link::State s) { if (s == Link::State::Ready) ++fakeServed; });
        });
        // an unpaired device
        TlsLink *stranger = TlsLink::connectTo("127.0.0.1", server.port(), strangerId.get(), TrustPolicy::pinned(hostId->pin()));
        bool strangerReady = false;
        QObject::connect(stranger, &Link::stateChanged, stranger, [&](Link::State s) { if (s == Link::State::Ready) strangerReady = true; });
        waitFor([&] { return stranger->state() == Link::State::Closed; }, 5000);
        check(stranger->state() == Link::State::Closed && served == 0, QStringLiteral("an unpaired device never gets a session: the host's link is not ready for it"));
        check(!stranger->sendJson(make("BACKUP_REQUEST", QString::fromLatin1(kUuid))), QStringLiteral("and nothing it sends is accepted"));
        (void)strangerReady;
        // a paired device meeting a host with another certificate
        TlsLink *known = TlsLink::connectTo("127.0.0.1", fake.port(), knownId.get(), TrustPolicy::pinned(hostId->pin()));
        waitFor([&] { return known->state() == Link::State::Closed; }, 5000);
        check(known->state() == Link::State::Closed && known->closeCode() == "IDENTITY_MISMATCH" && fakeServed == 0, QStringLiteral("a host with another certificate than the pinned one is refused (IDENTITY_MISMATCH)"), known->closeCode());
        // the right one works
        TlsLink *good = TlsLink::connectTo("127.0.0.1", server.port(), knownId.get(), TrustPolicy::pinned(hostId->pin()));
        check(waitFor([&] { return good->state() == Link::State::Ready; }, 5000), QStringLiteral("the paired device with the pinned certificate does get through"));
        good->close();
    }
    // ---- frames
    {
        FrameReader fr;
        QByteArray header(5, 0);
        const quint32 len = 5 * 1024 * 1024; // more than the 1 MiB a control message may have
        header[0] = char(len >> 24);
        header[1] = char(len >> 16);
        header[2] = char(len >> 8);
        header[3] = char(len);
        header[4] = 0;
        fr.feed(header);
        check(fr.error() == FrameError::TooBig, QStringLiteral("an oversize frame is refused as soon as its length is read, before its bytes arrive"));
        FrameReader fr2;
        QByteArray junk(5, 0);
        junk[3] = 2;
        junk[4] = 9; // an unknown kind
        fr2.feed(junk + QByteArray(2, 'x'));
        check(fr2.error() != FrameError::None, QStringLiteral("a frame of an unknown kind is refused"));
        FrameReader fr3([] { return qint64(1000); }); // time stands still: every frame is in the same second
        QByteArray one = encodeFrame(Frame::json(QJsonObject{{"t", "HB"}}));
        for (int i = 0; i < 300 && fr3.error() == FrameError::None; ++i)
            fr3.feed(one);
        check(fr3.error() == FrameError::RateLimit, QStringLiteral("more than 200 control frames in a second are refused"));
    }
    // ---- the host's side with a joined editor
    World w(2);
    w.startHost();
    auto a = w.client("devA");
    w.join(*a);
    const QString doc = w.docs.at(0);
    const MemLink::Pair rp = MemLink::createPair(QCryptographicHash::hash("host", QCryptographicHash::Sha256), a->pin, w.host.get());
    // (a second raw connection of the same device is simplest to hand-write messages with)
    QList<QJsonObject> got;
    QObject::connect(rp.b, &net::Link::frameReceived, rp.b, [&](const Frame &f) {
        bool ok = false;
        const QJsonObject o = f.object(&ok);
        if (ok)
            got << o;
    });
    w.host->addLink(rp.a);
    rp.b->sendJson(make("HELLO", QString::fromLatin1(kUuid), QJsonObject{{"protoMin", 1}, {"protoMax", 1}, {"name", "raw"}}));
    rp.b->sendJson(make("JOIN", QString::fromLatin1(kUuid), QJsonObject{{"epoch", 1}, {"seq", 0}}));
    settle();
    auto lastOf = [&](const char *t) {
        for (int i = got.size() - 1; i >= 0; --i)
            if (got.at(i).value("t").toString() == t)
                return got.at(i);
        return QJsonObject();
    };
    auto errors = [&] {
        QStringList c;
        for (const QJsonObject &o : got)
            if (o.value("t").toString() == "ERROR")
                c << o.value("code").toString();
        return c;
    };
    const QString outside = tmp + "/outside";
    QDir().mkpath(outside);
    // path traversal in a snapshot job
    const qint64 seq0 = w.host->seq();
    for (const QString &bad : {QString("../../outside"), QString("..\\x"), QString("/etc/passwd"), QString("a/b"), QString("")}) {
        rp.b->sendJson(make("SNAPSHOT_OP", QString::fromLatin1(kUuid), QJsonObject{{"opId", proto::newId()}, {"op", "take"}, {"doc", bad}, {"title", "x"}}));
        rp.b->sendJson(make("SNAPSHOT_OP", QString::fromLatin1(kUuid), QJsonObject{{"opId", proto::newId()}, {"op", "delete"}, {"doc", doc}, {"file", bad}}));
    }
    settle();
    int refusedJobs = 0;
    for (const QJsonObject &o : got)
        refusedJobs += (o.value("t").toString() == "SNAPSHOT_RESULT" && o.value("status").toString() == "error" && o.value("code").toString() == "BAD_OP") ? 1 : 0;
    check(refusedJobs == 10 && w.host->seq() == seq0 && snapshotCount(w.hostFolder, doc) == 0 && QDir(outside).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty(),
          QStringLiteral("snapshot jobs that name another place than a document's id and a snapshot's file name are refused, nothing is written"), QString::number(refusedJobs));
    // an operation of a wrong kind, and a title that is too long
    rp.b->sendJson(make("SNAPSHOT_OP", QString::fromLatin1(kUuid), QJsonObject{{"opId", proto::newId()}, {"op", "format"}, {"doc", doc}}));
    rp.b->sendJson(make("SNAPSHOT_OP", QString::fromLatin1(kUuid), QJsonObject{{"opId", proto::newId()}, {"op", "take"}, {"doc", doc}, {"title", QString(5000, 'x')}}));
    rp.b->sendJson(make("SNAPSHOT_OP", QString::fromLatin1(kUuid), QJsonObject{{"opId", "x"}, {"op", "take"}, {"doc", doc}}));
    settle();
    check(snapshotCount(w.hostFolder, doc) == 0 && w.host->seq() == seq0, QStringLiteral("an unknown job, a title of 5,000 characters and a short job id do nothing"));
    // a replayed message: the same message id again, and the same job id in a new message
    QJsonObject job = make("SNAPSHOT_OP", QString::fromLatin1(kUuid), QJsonObject{{"opId", "job-replay-0001"}, {"op", "take"}, {"doc", doc}, {"title", "Once"}});
    rp.b->sendJson(job);
    settle();
    const qint64 seq1 = w.host->seq();
    rp.b->sendJson(job); // the very same message
    settle();
    check(errors().contains("DUPLICATE") && w.host->seq() == seq1 && snapshotCount(w.hostFolder, doc) == 1, QStringLiteral("a message that is sent again (same message id) is refused as a duplicate and does nothing"));
    QJsonObject job2 = make("SNAPSHOT_OP", QString::fromLatin1(kUuid), QJsonObject{{"opId", "job-replay-0001"}, {"op", "take"}, {"doc", doc}, {"title", "Twice"}});
    rp.b->sendJson(job2);
    settle();
    check(w.host->seq() == seq1 && snapshotCount(w.hostFolder, doc) == 1 && lastOf("SNAPSHOT_RESULT").value("status").toString() == "ok", QStringLiteral("the same job in a new message gets the stored answer and is not done twice"));
    // a backup request from a frame with a wrong project
    rp.b->sendJson(make("BACKUP_REQUEST", "99999999-2222-3333-4444-555555555555"));
    settle();
    check(errors().contains("UNKNOWN_PROJECT"), QStringLiteral("a backup request for another project is refused"));
    // a hostile event reaches a client: traversal, case collision, a link on the way
    {
        QDir().mkpath(outside);
        const QString evilName = "evil-event.txt";
        auto sendEvent = [&](const QJsonArray &files) {
            QJsonObject ev = make("COMMIT_EVENT", QString::fromLatin1(kUuid));
            ev.insert("epoch", 1);
            ev.insert("seq", double(w.host->seq() + 1));
            ev.insert("opId", proto::newId());
            ev.insert("actor", "x");
            ev.insert("ops", QJsonArray());
            ev.insert("files", files);
            ev.insert("revs", QJsonObject());
            a->atHost->sendJson(ev);
        };
        const QString h = sha256Hex("evil");
        sendEvent(QJsonArray{QJsonObject{{"path", "../" + evilName}, {"kind", "put"}, {"hash", h}, {"size", 4}, {"rev", 1}}});
        sendEvent(QJsonArray{QJsonObject{{"path", "content/../../" + evilName}, {"kind", "put"}, {"hash", h}, {"size", 4}, {"rev", 1}}});
        sendEvent(QJsonArray{QJsonObject{{"path", "/tmp/" + evilName}, {"kind", "put"}, {"hash", h}, {"size", 4}, {"rev", 1}}});
        sendEvent(QJsonArray{QJsonObject{{"path", "../" + evilName}, {"kind", "removeTree"}}});
        settle();
        waitFor([&] { return false; }, 300);
        check(!QFileInfo::exists(a->folder + "/../" + evilName) && !QFileInfo::exists("/tmp/" + evilName) && !QFileInfo::exists(w.dir.path() + "/" + evilName),
              QStringLiteral("a host's event with a path that leaves the project is not carried out on the client"));
    }
    // a hostile manifest
    {
        Manifest m;
        QString why;
        auto entry = [](const QString &p) { return QJsonObject{{"path", p}, {"size", 1}, {"sha256", QString(64, 'a')}, {"rev", 0}}; };
        auto manifest = [&](const QJsonArray &e) { return QJsonObject{{"epoch", 1}, {"seq", 0}, {"format", 1}, {"entries", e}}; };
        check(!Manifest::fromJson(manifest({entry("../x")}), &m, &why), QStringLiteral("a manifest with a path above the project is refused"));
        check(!Manifest::fromJson(manifest({entry("content/a/text.md"), entry("CONTENT/a/TEXT.md")}), &m, &why), QStringLiteral("a manifest with two paths that differ only by case is refused"));
        check(!Manifest::fromJson(manifest({entry("livesync/state.json")}), &m, &why), QStringLiteral("a manifest that lists Live Sync's own files is refused"));
        QJsonArray many;
        for (int i = 0; i < kManifestMaxEntries + 5; ++i)
            many.append(QJsonObject{{"path", QString("content/d%1/text.md").arg(i)}, {"size", 1}, {"sha256", QString(64, 'a')}, {"rev", 0}});
        check(!Manifest::fromJson(manifest(many), &m, &why), QStringLiteral("a manifest with more entries than the limit is refused"));
        check(!Manifest::fromJson(QJsonObject{{"epoch", 1}, {"entries", "no"}}, &m, &why), QStringLiteral("a manifest that is no list is refused"));
    }
    // a hostile blob: bytes whose hash is not the one announced are not kept
    {
        const QByteArray bytes = "claimed to be something else";
        const QString tr = newTransferId();
        rp.b->sendJson(make("BLOB_PUT", QString::fromLatin1(kUuid), QJsonObject{{"transfer", tr}, {"hash", sha256Hex("something else")}, {"size", double(bytes.size())}, {"offset", 0}}));
        rp.b->sendData(transferBytes(tr), 0, bytes);
        rp.b->sendJson(make("BLOB_DONE", QString::fromLatin1(kUuid), QJsonObject{{"transfer", tr}}));
        settle();
        check(!w.host->incoming().has(sha256Hex("something else")) && !w.host->incoming().has(sha256Hex(bytes)), QStringLiteral("a blob whose bytes do not match its announced hash is not kept"));
    }
}

// ---------------------------------------------------------------- 7. the time from a change to the copy (a short run in one program)

static qint64 percentile(QList<qint64> v, double p)
{
    if (v.isEmpty())
        return -1;
    std::sort(v.begin(), v.end());
    return v.at(qMin<int>(v.size() - 1, int(std::ceil(p * v.size())) - 1));
}

static void testLatencySmoke()
{
    std::printf("== a change reaches the other copy (in-memory smoke run)\n");
    World w(3);
    w.startHost();
    auto a = w.client("devA");
    auto b = w.client("devB");
    w.join(*a);
    w.join(*b);
    const QString doc = w.docs.at(0);
    lockOn(*a, "doc:" + doc);
    QList<qint64> times;
    bool allThere = true;
    QByteArray text;
    for (int i = 0; i < 30; ++i) {
        text += "typed " + QByteArray::number(i) + " ";
        const QByteArray want = text + "\n";
        QElapsedTimer t;
        t.start();
        commitText(*a, doc, want);
        const bool ok = waitFor([&] { return readBytes(b->folder + "/content/" + doc + "/text.md") == want; }, 5000);
        times << t.elapsed();
        allThere = allThere && ok;
    }
    check(allThere, QStringLiteral("30 changes of one editor each reach the other device's copy"));
    check(percentile({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 0.95) == 10 && percentile({5}, 0.95) == 5 && percentile({}, 0.95) == -1, QStringLiteral("the percentile used for the measurement is the nearest-rank one"));
    std::printf("INFO  in-memory smoke run: p95 %lld ms (no network, no TLS: not the measurement)\n", static_cast<long long>(percentile(times, 0.95)));
}

// ---------------------------------------------------------------- the measurement: three programs over TLS on localhost

static const int kProjects = 2;
static const int kDocsPerProject = 100;

static void writeLine(const QString &path, const QString &line)
{
    QFile f(path);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        f.write(line.toUtf8() + "\n");
        f.flush();
    }
}

struct MeasureClient {
    std::unique_ptr<ClientSession> s;
    std::unique_ptr<HostJobsClient> jobs;
    qb::collab::net::TlsLink *link = nullptr;
    QString folder, uuid;
    QStringList docs;
    qint64 joinNs = 0;
};

static QString measureUuid(int p) { return QStringLiteral("00000000-0000-4000-8000-00000000000%1").arg(p + 1); }

static std::unique_ptr<MeasureClient> measureConnect(const QString &dir, const QString &who, int p, qb::collab::net::DeviceIdentity *id, SystemClock *clock)
{
    using namespace qb::collab::net;
    auto mc = std::make_unique<MeasureClient>();
    mc->uuid = measureUuid(p);
    mc->folder = dir + "/" + who + "-replica" + QString::number(p);
    ClientSession::Config cfg;
    cfg.folder = mc->folder;
    cfg.uuid = mc->uuid;
    cfg.deviceId = id->deviceId();
    cfg.deviceName = who;
    cfg.clock = clock;
    mc->s = std::make_unique<ClientSession>(cfg);
    mc->jobs = std::make_unique<HostJobsClient>(mc->s.get(), mc->uuid);
    HostJobsClient *jj = mc->jobs.get();
    mc->s->setExtensionHandler([jj](const QJsonObject &m) { return jj->handle(m); });
    mc->s->open();
    const quint16 port = quint16(readBytes(dir + "/port" + QString::number(p)).toInt());
    const QByteArray hostPin = QByteArray::fromHex(readBytes(dir + "/host.pin"));
    mc->link = TlsLink::connectTo("127.0.0.1", port, id, TrustPolicy::pinned(hostPin));
    ClientSession *sp = mc->s.get();
    QObject::connect(mc->link, &Link::stateChanged, sp, [sp, l = mc->link](Link::State st) {
        if (st == Link::State::Ready)
            sp->attach(l);
    });
    return mc;
}

// the watcher: writes "<project> <k> <latency ns>" for every marked change it receives
static int runWatcher(const QString &dir)
{
    using namespace qb::collab::net;
    auto id = DeviceIdentity::createEphemeral("watcher");
    if (!id)
        return 3;
    writeBytes(dir + "/watcher.pin", id->pin().toHex());
    SystemClock clock;
    for (int i = 0; i < 400 && !QFileInfo::exists(dir + "/port1"); ++i)
        waitFor([] { return false; }, 25);
    QList<std::shared_ptr<MeasureClient>> mcs;
    int nready = 0;
    for (int p = 0; p < kProjects; ++p) {
        std::shared_ptr<MeasureClient> mc = measureConnect(dir, "watcher", p, id.get(), &clock);
        const QString res = dir + "/watcher.results";
        MeasureClient *raw = mc.get();
        QObject::connect(mc->s.get(), &ClientSession::eventApplied, mc->s.get(),
                         [=](qint64, const QString &, const QString &, bool own, const QJsonArray &, const QStringList &paths, const QJsonObject &) {
                             if (own)
                                 return;
                             const qint64 now = monoNs();
                             for (const QString &path : paths) {
                                 if (!path.endsWith("/text.md"))
                                     continue;
                                 QFile f(raw->folder + "/" + path);
                                 if (!f.open(QIODevice::ReadOnly))
                                     continue;
                                 const qint64 size = f.size();
                                 f.seek(qMax<qint64>(0, size - 200));
                                 const QByteArray tail = f.readAll();
                                 const int at = tail.lastIndexOf("@@");
                                 if (at < 0)
                                     continue;
                                 const QList<QByteArray> parts = tail.mid(at + 2).trimmed().split(':');
                                 if (parts.size() < 3)
                                     continue;
                                 writeLine(res, QString("%1 %2 %3 %4").arg(QString::fromLatin1(parts.at(1))).arg(QString::fromLatin1(parts.at(2))).arg(now - parts.at(0).toLongLong()).arg(size));
                             }
                         });
        QObject::connect(mc->s.get(), &ClientSession::readyToUse, mc->s.get(), [&nready, dir, p] {
            ++nready;
            writeBytes(dir + QString("/watcher.ready%1").arg(p), "1");
        });
        mcs << mc;
    }
    QElapsedTimer t;
    t.start();
    while (!QFileInfo::exists(dir + "/stop") && t.elapsed() < 900000)
        waitFor([&] { return QFileInfo::exists(dir + "/stop"); }, 50);
    waitFor([] { return false; }, 1500);
    return 0;
}

// the typer: edits documents of both projects
static int runTyper(const QString &dir)
{
    using namespace qb::collab::net;
    auto id = DeviceIdentity::createEphemeral("typer");
    if (!id)
        return 3;
    writeBytes(dir + "/typer.pin", id->pin().toHex());
    SystemClock clock;
    for (int i = 0; i < 400 && !QFileInfo::exists(dir + "/port1"); ++i)
        waitFor([] { return false; }, 25);
    QList<std::shared_ptr<MeasureClient>> mcs;
    for (int p = 0; p < kProjects; ++p) {
        std::shared_ptr<MeasureClient> mc = measureConnect(dir, "typer", p, id.get(), &clock);
        MeasureClient *raw = mc.get();
        QObject::connect(mc->s.get(), &ClientSession::readyToUse, mc->s.get(), [raw, dir, p] {
            writeBytes(dir + QString("/typer.ready%1").arg(p), QByteArray::number(raw->joinNs));
        });
        mcs << mc;
    }
    const qint64 joinStart = monoNs();
    if (!waitFor([&] { return QFileInfo::exists(dir + "/typer.ready0") && QFileInfo::exists(dir + "/typer.ready1"); }, 600000))
        return 4;
    writeBytes(dir + "/typer.joinms", QByteArray::number((monoNs() - joinStart) / 1000000));
    // wait for the watcher and the go from the host
    if (!waitFor([&] { return QFileInfo::exists(dir + "/go"); }, 600000))
        return 5;
    struct Doc {
        QString id;
        qint64 size = 0;
        QByteArray base;
        QByteArray typed;
        bool locked = false;
        qint64 lockAsk = 0;
    };
    // documents to edit: spread over the sizes; every project: 14 of its documents (spaced edits), one for the typing
    int sent = 0, confirmed = 0;
    QList<qint64> confirmMs, lockMs;
    QList<qint64> statusMs;
    QMap<QString, qint64> sentAt; // opId -> ns
    for (int p = 0; p < kProjects; ++p) {
        MeasureClient *mc = mcs.at(p).get();
        QObject::connect(mc->s.get(), &ClientSession::committed, mc->s.get(), [&, p](const QString &op, qint64) {
            ++confirmed;
            if (sentAt.contains(op))
                confirmMs << (monoNs() - sentAt.take(op)) / 1000000;
        });
    }
    // the documents of a project are the folders under content/ of the replica
    QList<QStringList> docIds;
    for (int p = 0; p < kProjects; ++p)
        docIds << QDir(mcs.at(p)->folder + "/content").entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QList<QHash<QString, Doc>> docState(kProjects);
    auto prepare = [&](int p, const QString &docId) -> Doc & {
        Doc &d = docState[p][docId];
        if (d.id.isEmpty()) {
            d.id = docId;
            d.base = readBytes(mcs.at(p)->folder + "/content/" + docId + "/text.md");
            while (d.base.endsWith('\n') || d.base.endsWith(' '))
                d.base.chop(1);
            const int mark = d.base.lastIndexOf("@@");
            if (mark >= 0)
                d.base = d.base.left(mark);
        }
        return d;
    };
    auto submit = [&](int p, Doc &d, int k, const QByteArray &tail) {
        MeasureClient *mc = mcs.at(p).get();
        const qint64 ns = monoNs();
        const QByteArray text = d.base + tail + "\n@@" + QByteArray::number(ns) + ":" + QByteArray::number(p) + ":" + QByteArray::number(k) + "\n";
        QHash<QString, QByteArray> blobs;
        Unit u;
        u.name = "doc:" + d.id;
        FileOp f;
        f.path = QString("content/%1/text.md").arg(d.id);
        f.hash = sha256Hex(text);
        f.size = text.size();
        u.files << f;
        blobs.insert(f.hash, text);
        const QString op = mc->s->submit({u}, blobs);
        sentAt.insert(op, ns);
        ++sent;
    };
    auto lockDoc = [&](int p, Doc &d) {
        if (d.locked)
            return true;
        const QString res = "doc:" + d.id;
        QElapsedTimer t;
        t.start();
        mcs.at(p)->s->lock({res});
        if (!waitFor([&] { return mcs.at(p)->s->holds(res); }, 10000))
            return false;
        lockMs << t.elapsed();
        d.locked = true;
        return true;
    };
    QRandomGenerator rng(1234);
    // phase 1: spaced edits of 14 different documents per project, 350 ms apart, both projects in turn
    for (int k = 0; k < 14; ++k)
        for (int p = 0; p < kProjects; ++p) {
            Doc &d = prepare(p, docIds.at(p).at(int(rng.bounded(docIds.at(p).size()))));
            if (!lockDoc(p, d))
                continue;
            d.typed += " edit" + QByteArray::number(k);
            submit(p, d, k, d.typed);
            waitFor([] { return false; }, 350);
        }
    // phase 2: continuous typing, 10 changes a second (each a few characters more), in one document of each project, 45 s... 50 changes each
    Doc *typing[kProjects];
    for (int p = 0; p < kProjects; ++p) {
        typing[p] = &prepare(p, docIds.at(p).at(0));
        lockDoc(p, *typing[p]);
    }
    for (int k = 0; k < 50; ++k) {
        for (int p = 0; p < kProjects; ++p) {
            typing[p]->typed += " word" + QByteArray::number(k);
            submit(p, *typing[p], 100 + k, typing[p]->typed);
        }
        waitFor([] { return false; }, 100);
    }
    // control messages: the state of the host's backup (a small question and answer) and fresh locks of other documents
    for (int k = 0; k < 20; ++k) {
        const int p = k % kProjects;
        Doc &d = prepare(p, docIds.at(p).at(1 + k % 20));
        QElapsedTimer t;
        t.start();
        const QString res = "doc:" + d.id;
        mcs.at(p)->s->lock({res});
        if (waitFor([&] { return mcs.at(p)->s->holds(res); }, 10000))
            lockMs << t.elapsed();
        mcs.at(p)->s->unlock({res});
        waitFor([] { return false; }, 100);
    }
    waitFor([&] { return confirmed >= sent; }, 60000);
    QString lockLine, confLine;
    for (qint64 v : lockMs)
        lockLine += QString::number(v) + " ";
    for (qint64 v : confirmMs)
        confLine += QString::number(v) + " ";
    writeBytes(dir + "/typer.lock", lockLine.toUtf8());
    writeBytes(dir + "/typer.confirm", confLine.toUtf8());
    writeBytes(dir + "/typer.sent", QByteArray::number(sent));
    writeBytes(dir + "/typer.done", "1");
    waitFor([&] { return QFileInfo::exists(dir + "/stop"); }, 120000);
    return confirmed >= sent ? 0 : 6;
}

static QString describe(QList<qint64> v, const QString &unit)
{
    if (v.isEmpty())
        return QStringLiteral("no values");
    std::sort(v.begin(), v.end());
    return QStringLiteral("n=%1  p50 %2 %5  p95 %3 %5  p99 %4 %5  max %6 %5")
        .arg(v.size()).arg(percentile(v, 0.5)).arg(percentile(v, 0.95)).arg(percentile(v, 0.99)).arg(unit).arg(v.last());
}

static int runMeasure(const QString &dir)
{
    using namespace qb::collab::net;
    std::printf("== measurement: three programs over TLS on localhost\n");
    if (!DeviceIdentity::available() || !TlsLink::available()) {
        std::printf("this build has no Botan TLS: nothing measured\n");
        return 1;
    }
    QDir(dir).removeRecursively();
    QDir().mkpath(dir);
    struct Hosted {
        std::unique_ptr<Project> proj;
        std::unique_ptr<HostSession> host;
        std::unique_ptr<TlsServer> server;
    };
    SystemClock clock;
    auto hostId = DeviceIdentity::createEphemeral("host");
    writeBytes(dir + "/host.pin", hostId->pin().toHex());
    QList<std::shared_ptr<Hosted>> hosted;
    QRandomGenerator rng(99);
    qint64 totalBytes = 0;
    QElapsedTimer mk;
    mk.start();
    TrustPolicy trust;
    trust.decide = [&](const QByteArray &pin, bool) {
        for (const char *who : {"typer", "watcher"}) {
            const QByteArray known = QByteArray::fromHex(readBytes(dir + "/" + who + ".pin"));
            if (!known.isEmpty() && pin == known)
                return PeerInfo::Trust::Paired;
        }
        return PeerInfo::Trust::Unknown;
    };
    for (int p = 0; p < kProjects; ++p) {
        auto h = std::make_shared<Hosted>();
        QString err;
        h->proj = Project::create(dir + "/host" + QString::number(p), "Measured", &err);
        static const int sizes[] = {1000, 5000, 20000, 100000, 400000, 1000000};
        for (int i = 0; i < kDocsPerProject; ++i) {
            BinderItem *it = h->proj->model()->createItem(h->proj->draft(), -1, ItemType::Text, QStringLiteral("Doc %1").arg(i));
            const int size = sizes[rng.bounded(6)];
            QByteArray words;
            while (words.size() < size)
                words += "word" + QByteArray::number(rng.bounded(100000)) + " ";
            words += "\n";
            totalBytes += words.size();
            h->proj->docs()->text(it->id)->setPlainText(QString::fromLatin1(words));
        }
        h->proj->save(&err);
        h->proj->docs()->saveDirty(&err);
        HostSession::Options o;
        o.folder = h->proj->folderPath();
        o.uuid = measureUuid(p);
        o.hostName = "Host";
        o.clock = &clock;
        h->host = std::make_unique<HostSession>(o);
        if (!h->host->start(&err)) {
            std::printf("host start: %s\n", qPrintable(err));
            return 2;
        }
        h->server = std::make_unique<TlsServer>(hostId.get(), trust);
        h->server->listen(QHostAddress::LocalHost, 0);
        HostSession *hs = h->host.get();
        QObject::connect(h->server.get(), &TlsServer::newLink, h->server.get(), [hs](TlsLink *l) {
            QObject::connect(l, &Link::stateChanged, l, [hs, l](Link::State st) {
                if (st == Link::State::Ready) {
                    hs->setRole(l->peer().deviceId, Role::Editor);
                    hs->addLink(l);
                }
            });
        });
        writeBytes(dir + "/port" + QString::number(p), QByteArray::number(h->server->port()));
        hosted << h;
    }
    std::printf("two projects of %d documents made (%.1f MiB of text in all, %lld ms)\n", kDocsPerProject, totalBytes / 1048576.0, static_cast<long long>(mk.elapsed()));
    QProcess typer, watcher;
    typer.setProcessChannelMode(QProcess::ForwardedChannels);
    watcher.setProcessChannelMode(QProcess::ForwardedChannels);
    watcher.start(QCoreApplication::applicationFilePath(), {"--ls8-watcher", dir});
    typer.start(QCoreApplication::applicationFilePath(), {"--ls8-typer", dir});
    const bool started = typer.waitForStarted(5000) && watcher.waitForStarted(5000);
    std::printf("the typer and the watcher started: %s\n", started ? "yes" : "no");
    const bool ready = waitFor([&] {
        return QFileInfo::exists(dir + "/typer.ready0") && QFileInfo::exists(dir + "/typer.ready1") && QFileInfo::exists(dir + "/watcher.ready0")
               && QFileInfo::exists(dir + "/watcher.ready1");
    }, 600000);
    if (!ready) {
        std::printf("the copies were not ready in time\n");
        typer.kill();
        watcher.kill();
        return 3;
    }
    std::printf("both programs hold copies of both projects (the typer needed %s ms for them)\n", readBytes(dir + "/typer.joinms").constData());
    waitFor([] { return false; }, 1000);
    writeBytes(dir + "/go", "1");
    const bool done = waitFor([&] { return QFileInfo::exists(dir + "/typer.done"); }, 600000);
    waitFor([] { return false; }, 6000); // the last events arrive
    writeBytes(dir + "/stop", "1");
    typer.waitForFinished(20000);
    watcher.waitForFinished(20000);
    if (!done) {
        std::printf("the typer did not finish\n");
        return 4;
    }
    QList<qint64> latencies, latContinuous, latSpaced, locks, confirms;
    for (const QString &line : QString::fromUtf8(readBytes(dir + "/watcher.results")).split('\n', Qt::SkipEmptyParts)) {
        const QStringList f = line.split(' ');
        if (f.size() < 4)
            continue;
        const qint64 ms = f.at(2).toLongLong() / 1000000;
        latencies << ms;
        (f.at(1).toInt() >= 100 ? latContinuous : latSpaced) << ms;
    }
    for (const QString &v : QString::fromUtf8(readBytes(dir + "/typer.lock")).split(' ', Qt::SkipEmptyParts))
        locks << v.toLongLong();
    for (const QString &v : QString::fromUtf8(readBytes(dir + "/typer.confirm")).split(' ', Qt::SkipEmptyParts))
        confirms << v.toLongLong();
    const int sent = readBytes(dir + "/typer.sent").toInt();
    QString out;
    out += QString("changes sent: %1, seen by the other device: %2\n").arg(sent).arg(latencies.size());
    out += "edit to the confirmed copy on the other device, all   : " + describe(latencies, "ms") + "\n";
    out += "   spaced edits (one change at a time)                : " + describe(latSpaced, "ms") + "\n";
    out += "   continuous typing (10 changes a second, 2 projects): " + describe(latContinuous, "ms") + "\n";
    out += "edit to the host's confirmation on the typing device  : " + describe(confirms, "ms") + "\n";
    out += "control message: lock asked to lock granted           : " + describe(locks, "ms") + "\n";
    out += QString("projects: %1 of %2 documents (%3 MiB of text), 3 programs, TLS 1.3, localhost, %4 cores\n")
               .arg(kProjects).arg(kDocsPerProject).arg(totalBytes / 1048576.0, 0, 'f', 1).arg(QThread::idealThreadCount());
    out += QString("target: p95 of edit to copy <= 10000 ms, control p95 < 1000 ms: %1, %2\n")
               .arg(percentile(latencies, 0.95) >= 0 && percentile(latencies, 0.95) <= 10000 ? "met" : "NOT met")
               .arg(percentile(locks, 0.95) >= 0 && percentile(locks, 0.95) < 1000 ? "met" : "NOT met");
    std::printf("%s", qPrintable(out));
    writeBytes(dir + "/results.txt", out.toUtf8());
    return 0;
}

// ---------------------------------------------------------------- main

int main(int argc, char **argv)
{
    qputenv("QB_NO_UPDATE_CHECK", "1");
    qputenv("QT_QPA_PLATFORM", qgetenv("QT_QPA_PLATFORM").isEmpty() ? QByteArray("offscreen") : qgetenv("QT_QPA_PLATFORM"));
    QGuiApplication app(argc, argv);
    if (argc >= 3 && QString::fromLocal8Bit(argv[1]) == QLatin1String("--ls8-watcher"))
        return runWatcher(QString::fromLocal8Bit(argv[2]));
    if (argc >= 3 && QString::fromLocal8Bit(argv[1]) == QLatin1String("--ls8-typer"))
        return runTyper(QString::fromLocal8Bit(argv[2]));
    if (argc >= 3 && QString::fromLocal8Bit(argv[1]) == QLatin1String("--measure"))
        return runMeasure(QString::fromLocal8Bit(argv[2]));
    QTemporaryDir tmp;
    qputenv("QB_DATA_DIR", tmp.path().toUtf8() + "/data");
    testBackupDuringCommits();
    testBackupState();
    testBackupFailures();
    testSnapshots();
    testJobRecovery();
    testRestoreNewProject(tmp.path());
    testSecurity(tmp.path());
    testLatencySmoke();
    std::printf("\n%d checks\n%d failure(s)\n", total, failures);
    return failures;
}
