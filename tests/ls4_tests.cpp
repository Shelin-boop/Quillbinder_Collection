// Live Sync, work package 4 (0.99.0): the protocol. Messages and their validator, the lock table with leases and fencing, edit
// requests (a decline restarts the waiting time, a takeover only after it), the manifest, blobs, the outbox, and the host and
// client sessions over a fake network (MemLink) with a clock that the test moves by hand; plus one real run of two programs over
// TLS on localhost. No window; the folder given as the first argument is not used.
//
//   QB_SAMPLE=0 QB_SUITES=ls4 tools/run-all-tests.sh build/test-results-x11 x11      (or ./qb_ls4_tests /tmp/shots)
// Exit code = number of failed checks.
#include "core/collab/Blobs.h"
#include "core/collab/ClientSession.h"
#include "core/collab/Clock.h"
#include "core/collab/HostSession.h"
#include "core/collab/Locks.h"
#include "core/collab/Manifest.h"
#include "core/collab/Outbox.h"
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/net/MemLink.h"
#include "core/collab/net/TlsLink.h"
#include "core/collab/proto/Messages.h"
#include "core/SafeSave.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QProcess>
#include <QRandomGenerator>
#include <QTemporaryDir>
#include <QTimer>
#include <cstdio>
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

static bool waitFor(const std::function<bool()> &cond, int ms = 5000)
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
// everything that is under way in memory has happened
static void settle(int rounds = 60)
{
    for (int i = 0; i < rounds; ++i) {
        QEventLoop loop;
        QTimer::singleShot(0, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

static QByteArray readFileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
static bool writeBytes(const QString &path, const QByteArray &b)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    return safesave::writeFile(path, b);
}

// ---------------------------------------------------------------- a small project folder

static const char *kIndex =
    "<?xml version=\"1.0\"?>\n<QuillbinderProject version=\"1\" title=\"Test\"><Binder>"
    "<Item id=\"docA\" type=\"text\" created=\"2026-01-01T00:00:00\" modified=\"2026-01-01T00:00:00\" words=\"2\" chars=\"10\"><Title>Chapter A</Title></Item>"
    "<Item id=\"docB\" type=\"text\" created=\"2026-01-01T00:00:00\" modified=\"2026-01-01T00:00:00\" words=\"2\" chars=\"10\"><Title>Chapter B</Title></Item>"
    "<Item id=\"fold1\" type=\"folder\" created=\"2026-01-01T00:00:00\"><Title>Folder</Title><Children>"
    "<Item id=\"docC\" type=\"text\" created=\"2026-01-01T00:00:00\" modified=\"2026-01-01T00:00:00\" words=\"2\" chars=\"10\"><Title>Chapter C</Title></Item>"
    "</Children></Item>"
    "<Item id=\"TRASH\" type=\"Trash\"><Title>Trash</Title></Item></Binder></QuillbinderProject>\n";

static const char *kUuid = "11111111-2222-3333-4444-555555555555";

static QString makeProject(const QString &folder)
{
    QDir().mkpath(folder);
    writeBytes(folder + "/Test.qbx", kIndex);
    for (const char *id : {"docA", "docB", "docC"})
        writeBytes(folder + QString("/content/%1/text.md").arg(QLatin1String(id)), QByteArray("Text of ") + id + "\n");
    writeBytes(folder + "/settings/look.json", "{\"font\":\"Serif\"}");
    writeBytes(folder + "/objects/schema.json", "{\"v\":1}");
    return folder;
}

static QString treeHash(const QString &folder)
{
    Manifest m;
    Manifest::scan(folder, QString(), {}, &m);
    return m.stateHash();
}

// ---------------------------------------------------------------- the world: a host, clients, a clock

struct Client {
    QString name;
    QByteArray pin;
    QString device;
    QString folder;
    std::unique_ptr<ClientSession> s;
    MemLink *toHost = nullptr; // the client's end
    MemLink *atHost = nullptr; // the host's end
    QList<qint64> events;
    QStringList drafts, rejectedCodes, problems, revoked, denied, granted;
    QList<QString> editAsked;
    struct Status {
        bool declined;
        qint64 waitMs;
        bool mayForce;
    };
    QList<Status> statuses;
    int committed = 0;
    QStringList committedOps;
    bool ready = false;
};

struct World {
    QTemporaryDir dir;
    ManualClock clock;
    QString hostFolder;
    std::unique_ptr<HostSession> host;
    QList<std::shared_ptr<Client>> clients;
    QStringList refusals;
    QStringList snapshotLabels;
    bool snapshotFails = false;

    explicit World(const QString &sub = QStringLiteral("w"))
    {
        hostFolder = makeProject(dir.path() + "/" + sub + "/host");
    }
    bool startHost(Role defaultRole = Role::None)
    {
        HostSession::Options o;
        o.folder = hostFolder;
        o.uuid = QString::fromLatin1(kUuid);
        o.hostName = QStringLiteral("Host");
        o.clock = &clock;
        host = std::make_unique<HostSession>(o);
        Q_UNUSED(defaultRole);
        connect();
        QString err;
        const bool ok = host->start(&err);
        if (!ok)
            std::printf("      host start: %s\n", qPrintable(err));
        return ok;
    }
    void connect()
    {
        QObject::connect(host.get(), &HostSession::refused, host.get(), [this](const QString &, const QString &c) { refusals << c; });
        host->setSnapshotHook([this](const QString &, const QString &label, QString *err) {
            snapshotLabels << label;
            if (snapshotFails) {
                *err = QStringLiteral("snapshot failed");
                return false;
            }
            return true;
        });
    }
    std::shared_ptr<Client> client(const QString &name, Role role = Role::Editor, const QString &folderOverride = QString())
    {
        auto c = std::make_shared<Client>();
        c->name = name;
        c->pin = QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha256);
        c->device = MemLink::deviceIdOfPin(c->pin);
        c->folder = folderOverride.isEmpty() ? dir.path() + "/w/" + name : folderOverride;
        host->setRole(c->device, role);
        ClientSession::Config cfg;
        cfg.folder = c->folder;
        cfg.uuid = QString::fromLatin1(kUuid);
        cfg.deviceId = c->device;
        cfg.deviceName = name;
        cfg.clock = &clock;
        c->s = std::make_unique<ClientSession>(cfg);
        wire(*c);
        c->s->open();
        clients << c;
        return c;
    }
    void wire(Client &c)
    {
        Client *p = &c;
        ClientSession *s = c.s.get();
        QObject::connect(s, &ClientSession::eventApplied, s, [p](qint64 seq, const QString &, const QString &, bool, const QJsonArray &, const QStringList &, const QJsonObject &) { p->events << seq; });
        QObject::connect(s, &ClientSession::draftKept, s, [p](const QString &op, const QString &why) { p->drafts << op + ":" + why; });
        QObject::connect(s, &ClientSession::rejected, s, [p](const QString &, const QJsonArray &r) {
            for (const QJsonValue &v : r)
                p->rejectedCodes << v.toObject().value("code").toString();
        });
        QObject::connect(s, &ClientSession::problem, s, [p](const QString &, const QString &c2) { p->problems << c2; });
        QObject::connect(s, &ClientSession::lockRevoked, s, [p](const QString &r, const QString &why) { p->revoked << r + ":" + why; });
        QObject::connect(s, &ClientSession::lockDenied, s, [p](const QString &r, const QString &, const QString &why) { p->denied << r + ":" + why; });
        QObject::connect(s, &ClientSession::lockGranted, s, [p](const QString &r) { p->granted << r; });
        QObject::connect(s, &ClientSession::editAsked, s, [p](const QString &r, const QString &, qint64) { p->editAsked << r; });
        QObject::connect(s, &ClientSession::editStatus, s, [p](const QString &, bool d, qint64 w, bool m, const QString &) { p->statuses.append({d, w, m}); });
        QObject::connect(s, &ClientSession::committed, s, [p](const QString &op, qint64) { ++p->committed; p->committedOps << op; });
        QObject::connect(s, &ClientSession::readyToUse, s, [p] { p->ready = true; });
    }
    // a client's link pair: the host end sees the client's pin, the client end the host's
    void link(Client &c)
    {
        const QByteArray hostPin = QCryptographicHash::hash("host", QCryptographicHash::Sha256);
        const MemLink::Pair pr = MemLink::createPair(hostPin, c.pin, host.get()); // a = host end (peer: client pin), b = client end
        c.atHost = pr.a;
        c.toHost = pr.b;
        host->addLink(c.atHost);
        c.s->attach(c.toHost);
    }
    void cut(Client &c)
    {
        if (c.toHost)
            c.toHost->close();
        settle();
        c.toHost = c.atHost = nullptr;
    }
    bool join(Client &c, int ms = 8000)
    {
        link(c);
        return waitFor([&] { return c.s->state() == ClientSession::State::Ready; }, ms);
    }
    void advance(qint64 ms)
    {
        for (qint64 done = 0; done < ms; done += 1000) {
            clock.advance(qMin<qint64>(1000, ms - done));
            settle(10);
        }
        settle();
    }
};

static bool lockOn(World &w, Client &c, const QString &res)
{
    c.s->lock({res});
    return waitFor([&] { return c.s->holds(res); }, 3000);
}

// the unit for a document's text
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
static Unit fileUnit(const QString &path, const QByteArray &bytes, QHash<QString, QByteArray> *blobs)
{
    Unit u;
    u.name = path;
    FileOp f;
    f.path = path;
    f.hash = sha256Hex(bytes);
    f.size = bytes.size();
    u.files << f;
    blobs->insert(f.hash, bytes);
    return u;
}
static QString commitText(Client &c, const QString &id, const QByteArray &text)
{
    QHash<QString, QByteArray> blobs;
    return c.s->submit({textUnit(id, text, &blobs)}, blobs);
}
static bool waitCommitted(Client &c, int n, int ms = 5000) { return waitFor([&] { return c.committed >= n; }, ms); }
static QByteArray hostText(World &w, const QString &id) { return readFileBytes(w.hostFolder + QString("/content/%1/text.md").arg(id)); }

// a request sent by hand (a client that is not the program's own)
struct RawPeer {
    MemLink *mine = nullptr, *atHost = nullptr;
    QList<QJsonObject> got;
    QStringList codes;
    QByteArray pin;
    QString device;
    void send(const QJsonObject &o) { mine->sendJson(o); }
};
static std::shared_ptr<RawPeer> rawPeer(World &w, const QString &name, Role role, bool hello = true, bool join = true)
{
    auto r = std::make_shared<RawPeer>();
    r->pin = QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha256);
    r->device = MemLink::deviceIdOfPin(r->pin);
    w.host->setRole(r->device, role);
    const MemLink::Pair pr = MemLink::createPair(QCryptographicHash::hash("host", QCryptographicHash::Sha256), r->pin, w.host.get());
    r->atHost = pr.a;
    r->mine = pr.b;
    RawPeer *p = r.get();
    QObject::connect(r->mine, &net::Link::frameReceived, r->mine, [p](const Frame &f) {
        bool ok = false;
        const QJsonObject o = f.object(&ok);
        if (ok) {
            p->got << o;
            if (o.value("t").toString() == "ERROR")
                p->codes << o.value("code").toString();
        }
    });
    w.host->addLink(r->atHost);
    if (hello)
        r->send(make("HELLO", QString::fromLatin1(kUuid), QJsonObject{{"protoMin", 1}, {"protoMax", 1}, {"name", name}}));
    if (join)
        r->send(make("JOIN", QString::fromLatin1(kUuid), QJsonObject{{"epoch", 1}, {"seq", 0}}));
    settle();
    return r;
}
static QJsonObject lastOf(const RawPeer &r, const QString &type)
{
    for (int i = r.got.size() - 1; i >= 0; --i)
        if (r.got.at(i).value("t").toString() == type)
            return r.got.at(i);
    return QJsonObject();
}
static void upload(RawPeer &r, const QByteArray &bytes)
{
    const QString hash = sha256Hex(bytes);
    const QString tr = newTransferId();
    r.send(make("BLOB_PUT", QString::fromLatin1(kUuid), QJsonObject{{"transfer", tr}, {"hash", hash}, {"size", double(bytes.size())}, {"offset", 0}}));
    if (!bytes.isEmpty())
        r.mine->sendData(transferBytes(tr), 0, bytes);
    r.send(make("BLOB_DONE", QString::fromLatin1(kUuid), QJsonObject{{"transfer", tr}}));
}
static QJsonObject rawCommit(const QString &opId, const QJsonArray &units, qint64 epoch = 1, qint64 baseSeq = 0, const QJsonArray &locks = QJsonArray())
{
    return make("COMMIT_REQUEST", QString::fromLatin1(kUuid),
                QJsonObject{{"opId", opId}, {"epoch", double(epoch)}, {"baseSeq", double(baseSeq)}, {"units", units}, {"locks", locks}});
}
static QJsonObject putUnit(const QString &path, const QByteArray &bytes, qint64 baseRev, const QString &resource)
{
    Unit u;
    u.name = path;
    FileOp f;
    f.path = path;
    f.hash = sha256Hex(bytes);
    f.size = bytes.size();
    u.files << f;
    u.bases.insert(resource, baseRev);
    return toJson(u);
}

// ---------------------------------------------------------------- 1. messages

static void testMessages()
{
    std::printf("== messages\n");
    const QStringList types = messageTypes();
    check(types.size() == 51 && knownType("COMMIT_REQUEST") && knownType("EDIT_FORCE") && !knownType("NOPE"), QStringLiteral("the message types of the design are known (and the cancel of an edit request)"),
          QString::number(types.size()));
    check(errorCodes().size() == 31 && knownError("LOCK_STALE") && knownError("UNKNOWN_OLD") && !knownError("OOPS"), QStringLiteral("the error codes of section 3.2 are known"));
    QString code, why;
    check(validate(make("HB"), &code), QStringLiteral("a plain HB is valid"));
    check(!validate(QJsonObject{{"t", "HB"}}, &code, &why) && code == "BAD_OP", QStringLiteral("a message without an id is refused"), why);
    check(!validate(QJsonObject{{"t", "NOPE"}, {"id", "1"}}, &code) && code == "BAD_OP", QStringLiteral("an unknown type gives BAD_OP"));
    check(!validate(QJsonObject{{"t", "HB"}, {"id", "1"}, {"v", 2}}, &code) && code == "UNSUPPORTED_VERSION", QStringLiteral("another major version gives UNSUPPORTED_VERSION"));
    check(validate(QJsonObject{{"t", "HB"}, {"id", "1"}, {"extra", "x"}}, &code), QStringLiteral("unknown fields are ignored"));
    check(!validate(QJsonObject{{"t", "RESYNC"}, {"id", "1"}, {"fromSeq", "3"}}, &code) && code == "BAD_OP", QStringLiteral("a field of the wrong kind is refused"));
    check(!validate(QJsonObject{{"t", "RESYNC"}, {"id", "1"}, {"fromSeq", 1.5}}, &code), QStringLiteral("a number that is no integer is refused"));
    check(!validate(QJsonObject{{"t", "JOIN"}, {"id", "1"}, {"epoch", 1}, {"seq", 0}}, &code), QStringLiteral("JOIN without a project is refused"));
    QJsonArray big;
    for (int i = 0; i < 70; ++i)
        big.append("doc:a");
    check(!validate(QJsonObject{{"t", "LOCK_REQUEST"}, {"id", "1"}, {"resources", big}}, &code) && code == "TOO_BIG", QStringLiteral("a list that is too long gives TOO_BIG"));
    check(!validate(QJsonObject{{"t", "BLOB_GET"}, {"id", "1"}, {"hash", "zz"}, {"offset", 0}}, &code), QStringLiteral("a blob hash must be a SHA-256"));
    check(!validate(QJsonObject{{"t", "BLOB_PUT"}, {"id", "1"}, {"transfer", "12"}, {"hash", QString(64, 'a')}, {"size", 1}}, &code), QStringLiteral("a transfer id must be 16 bytes"));
    check(!validate(QJsonObject{{"t", "HB"}, {"id", QString(80, 'x')}}, &code), QStringLiteral("a message id is at most 64 characters"));
    // random objects never crash the validator and never pass with an unknown type
    QRandomGenerator rng(7);
    bool oddPassed = false;
    for (int i = 0; i < 3000; ++i) {
        QJsonObject o;
        const int n = rng.bounded(6);
        for (int k = 0; k < n; ++k) {
            const QString key = QStringList{"t", "id", "v", "p", "re", "epoch", "seq", "units", "hash", "opId"}.at(rng.bounded(10));
            switch (rng.bounded(5)) {
            case 0: o.insert(key, rng.bounded(100)); break;
            case 1: o.insert(key, QString::number(rng.generate())); break;
            case 2: o.insert(key, QJsonArray{1, "a"}); break;
            case 3: o.insert(key, QJsonObject{{"x", 1}}); break;
            default: o.insert(key, messageTypes().at(rng.bounded(types.size()))); break;
            }
        }
        QString c;
        if (validate(o, &c) && !knownType(o.value("t").toString()))
            oddPassed = true;
    }
    check(!oddPassed, QStringLiteral("3000 random objects: nothing with an unknown type passes"));
    // units
    Unit u;
    u.name = "u1";
    u.bases.insert("doc:a", 3);
    Op op;
    op.kind = Op::ItemFields;
    op.id = "a";
    op.element = "<Item id=\"a\"><Title>T</Title></Item>";
    u.ops << op;
    FileOp f;
    f.path = "content/a/text.md";
    f.hash = QString(64, 'b');
    f.size = 4;
    u.files << f;
    Unit back;
    QString w2;
    check(fromJson(toJson(u), &back, &w2) && back.name == "u1" && back.bases.value("doc:a") == 3 && back.ops.size() == 1 && back.ops.first() == op && back.files.size() == 1
              && back.files.first().hash == f.hash,
          QStringLiteral("a unit survives its JSON form"), w2);
    QJsonObject bad = toJson(u);
    QJsonArray files = bad.value("files").toArray();
    QJsonObject f0 = files.at(0).toObject();
    f0.insert("hash", "short");
    files[0] = f0;
    bad.insert("files", files);
    check(!fromJson(bad, &back), QStringLiteral("a file entry with a wrong hash is refused"));
    check(resourcesOfOp(op) == QStringList{"doc:a"} && resourcesOfPath("content/a/text.md") == QStringList{"doc:a"} && resourcesOfPath("settings/look.json") == QStringList{"settings"}
              && resourcesOfPath("objects/schema.json") == QStringList{"object:schema"} && resourcesOfPath("content/a/mindmap.scap") == QStringList{"mindmap:a"},
          QStringLiteral("operations and paths map to their lock resources"));
    bool refusedPath = false;
    resourcesOfPath("Test.qbx", &refusedPath);
    bool refused2 = false;
    resourcesOfPath("content/a/snapshots/s1/index.json", &refused2);
    check(refusedPath && refused2, QStringLiteral("the project file and snapshot files are never a client's to write"));
    Token t{"doc:a", 2, 17, "lease"};
    check(tokenFromJson(toJson(t)) == t && t.valid() && !Token().valid(), QStringLiteral("a lock token survives its JSON form"));
}

// ---------------------------------------------------------------- 2. locks

static void testLocks()
{
    std::printf("== locks\n");
    // the matrix of section 5.1
    const QStringList res = {"doc:x", "doc:y", "structure", "settings", "object:o", "project"};
    // rows: wanted; columns: held by another. true = conflict
    const bool matrix[6][6] = {
        {true, false, false, false, false, true},   // DOC(x)
        {false, true, false, false, false, true},   // DOC(y)  (a different document: no conflict with x; with y itself yes)
        {false, false, true, false, false, true},   // STRUCTURE
        {false, false, false, true, false, true},   // SETTINGS
        {false, false, false, false, true, true},   // OBJECT(o)
        {true, true, true, true, true, true}};      // PROJECT
    bool all = true;
    QString first;
    for (int w = 0; w < 6; ++w)
        for (int h = 0; h < 6; ++h) {
            // row DOC(y) wants doc:y; the column list's "doc:y" is the same resource there
            if (conflicts(res.at(w), res.at(h)) != matrix[w][h]) {
                all = false;
                if (first.isEmpty())
                    first = res.at(w) + " vs " + res.at(h);
            }
        }
    check(all, QStringLiteral("the lock matrix of the design: every pair of 6 resources"), first);
    check(!conflicts("doc:x", "doc:y") && conflicts("doc:x", "doc:x") && !conflicts("structure", "doc:x") && conflicts("project", "settings"),
          QStringLiteral("two documents never conflict, a document and the structure never, anything and the whole project always"));
    check(conflicts("bogus", "doc:x") && !validResource("bogus") && !validResource("doc:../x") && validResource("mindmap:a") && validResource("object:schema"),
          QStringLiteral("a resource that is not understood is never let through"));

    ManualClock clock;
    LockTable t(&clock, 3);
    qint64 reserved = 0;
    int persists = 0;
    t.setGenerationReserve(0, [&](qint64 r) {
        reserved = r;
        ++persists;
        return true;
    });
    const Owner a{"devA", "w1"}, b{"devB", "w1"};
    LockTable::Attempt r = t.acquire(a, "A", {"doc:x"});
    check(r.ok && r.locks.size() == 1 && r.locks.first().generation == 1 && r.locks.first().epoch == 3 && !r.locks.first().lease.isEmpty(),
          QStringLiteral("a lock is granted with epoch, generation and lease id"));
    check(reserved == kGenerationBlock && persists == 1, QStringLiteral("generations come from a block that is stored before it is used"));
    r = t.acquire(b, "B", {"doc:x"});
    check(!r.ok && r.holderName == "A" && r.heldResource == "doc:x" && t.find("doc:x")->owner == a, QStringLiteral("a second owner is refused and told who holds it"));
    // all or none, and sorted
    r = t.acquire(b, "B", {"doc:y", "doc:x", "structure"});
    check(!r.ok && !t.find("doc:y") && !t.find("structure"), QStringLiteral("several locks are one step: all or none"));
    r = t.acquire(b, "B", {"structure", "doc:y"});
    check(r.ok && r.locks.size() == 2 && t.find("structure")->owner == b, QStringLiteral("structure beside text: another owner holds the structure while A edits"));
    const proto::Token tok = t.find("doc:x")->token();
    r = t.acquire(a, "A", {"doc:x"});
    check(r.ok && r.locks.first().token() == tok, QStringLiteral("asking again for a lock one holds gives the same token"));
    check(!t.acquire(a, "A", {"project"}).ok && !t.acquire(b, "B", {"project"}).ok, QStringLiteral("the whole project cannot be locked while others hold anything"));
    check(t.valid(tok) && !t.valid(proto::Token{"doc:x", 3, tok.generation + 1, tok.lease}) && !t.valid(proto::Token{"doc:x", 2, tok.generation, tok.lease}),
          QStringLiteral("only the current token is valid (generation and epoch are checked)"));
    // leases
    clock.advance(9000);
    check(t.isReachable(*t.find("doc:x")) && t.newlyUnreachable().isEmpty(), QStringLiteral("9 s without a heartbeat: still reachable"));
    clock.advance(1500);
    QList<Lock> un = t.newlyUnreachable();
    check(un.size() == 3 && !t.isReachable(*t.find("doc:x")) && t.newlyUnreachable().isEmpty(), QStringLiteral("after 10 s the locks are shown as not reachable, once"));
    t.renewDevice("devA");
    check(t.isReachable(*t.find("doc:x")) && !t.isReachable(*t.find("doc:y")), QStringLiteral("a heartbeat renews all of the device's locks, and only those"));
    clock.advance(20000);
    QList<Lock> gone = t.expire();
    check(gone.size() == 2 && !t.find("doc:y") && !t.find("structure") && t.find("doc:x"), QStringLiteral("after 30 s without a heartbeat the lease is gone"));
    clock.advance(15000);
    check(t.expire().size() == 1 && !t.find("doc:x"), QStringLiteral("the other lock expires on its own time"));
    // an unused lock is never given up by itself
    t.acquire(a, "A", {"doc:u"});
    for (int i = 0; i < 400; ++i) { // 20 minutes, with heartbeats every 3 s
        clock.advance(3000);
        t.renewDevice("devA");
        t.expire();
    }
    check(t.find("doc:u") && t.find("doc:u")->owner == a, QStringLiteral("a lock that is only renewed stays for 20 minutes with no work (no idle release)"));
    // generations after a restart
    const qint64 last = t.lastGeneration();
    LockTable t2(&clock, 3);
    t2.setGenerationReserve(reserved, [&](qint64 rr) { reserved = rr; return true; });
    const Lock again = t2.acquire(a, "A", {"doc:u"}).locks.first();
    check(again.generation > last, QStringLiteral("after a restart no generation of the old block is used again (%1 > %2)").arg(again.generation).arg(last));
    // reserve that cannot be stored
    LockTable t3(&clock, 1);
    t3.setGenerationReserve(0, [](qint64) { return false; });
    const LockTable::Attempt fail = t3.acquire(a, "A", {"doc:q"});
    check(!fail.ok && fail.storageFailed && !t3.find("doc:q"), QStringLiteral("nothing is granted when the generation block cannot be stored"));
    // takeover
    Lock prev;
    const Lock fresh = t.takeOver("doc:u", b, "B", &prev);
    check(prev.owner == a && fresh.owner == b && fresh.generation > prev.generation && !t.valid(prev.token()) && t.valid(fresh.token()), QStringLiteral("a takeover gives the next generation and makes the old token stale"));
    // sessions
    t.acquire(a, "A", {"doc:s"});
    const Owner a2{"devA", "w2"};
    const int moved = t.moveSessions("devA", [&](const Owner &o) { return o == a; }, a2);
    check(moved == 1 && t.find("doc:s")->owner == a2, QStringLiteral("a new window of the same device takes over the leases of a window that is gone"));
    check(t.releaseAllOf(a2).size() == 1 && !t.find("doc:s"), QStringLiteral("all locks of an owner are released at once"));

    // edit requests
    ManualClock c2;
    EditRequests q(&c2);
    EditRequest &e = q.open("doc:x", b, "B");
    check(e.deadline == 20000 && !q.mayForce(e) && q.remainingMs(e) == 20000, QStringLiteral("an edit request waits 20 s"));
    c2.advance(15000);
    check(&q.open("doc:x", b, "B") == &e || q.count() == 1, QStringLiteral("asking again does not open a second request"));
    q.decline("doc:x");
    check(q.find("doc:x", b)->deadline == 35000 && q.find("doc:x", b)->declines == 1 && q.remainingMs(*q.find("doc:x", b)) == 20000, QStringLiteral("a decline restarts the waiting time at 20 s"));
    c2.advance(19000);
    check(!q.mayForce(*q.find("doc:x", b)) && q.dueNow().isEmpty(), QStringLiteral("a takeover is not possible while the time runs"));
    c2.advance(1000);
    check(q.mayForce(*q.find("doc:x", b)) && q.dueNow().size() == 1 && q.dueNow().isEmpty(), QStringLiteral("when the time is out the asker is told once"));
    q.decline("doc:x");
    check(!q.mayForce(*q.find("doc:x", b)) && q.find("doc:x", b)->declines == 2, QStringLiteral("a decline after the time out restarts it again"));
    check(q.cancel("doc:x", b) && q.count() == 0 && !q.cancel("doc:x", b), QStringLiteral("an asker can cancel"));
}

// ---------------------------------------------------------------- 3. manifest

static void testManifest(const QString &tmp)
{
    std::printf("== manifest\n");
    const QString f = makeProject(tmp + "/man");
    writeBytes(f + "/settings/ui-abc.json", "{}");
    writeBytes(f + "/settings/journal.json", "{}");
    writeBytes(f + "/content/docA/file.qbstage", "x");
    writeBytes(f + "/livesync/state.json", "{}");
    writeBytes(f + "/livesync-local/outbox/x.json", "{}");
    writeBytes(f + "/livesync-replica.json", "{}");
    Manifest m;
    QString err;
    const bool ok = Manifest::scan(f, f + "/livesync/hashes.json", {{"doc:docA", 4}, {"settings", 2}}, &m, &err);
    QStringList paths;
    for (const ManifestEntry &e : m.entries)
        paths << e.path;
    check(ok && paths == QStringList{"Test.qbx", "content/docA/text.md", "content/docB/text.md", "content/docC/text.md", "objects/schema.json", "settings/look.json"},
          QStringLiteral("the manifest lists the replicated files and none of the device-local or Live Sync's own"), paths.join(","));
    check(m.find("content/docA/text.md")->rev == 4 && m.find("settings/look.json")->rev == 2 && m.find("content/docB/text.md")->rev == 0
              && m.find("content/docA/text.md")->sha256 == sha256Hex(readFileBytes(f + "/content/docA/text.md")),
          QStringLiteral("each entry carries its resource's revision and the SHA-256 of the stored bytes"));
    Manifest back;
    QString why;
    check(Manifest::fromJson(m.toJson(), &back, &why) && back.stateHash() == m.stateHash() && back.entries.size() == m.entries.size(), QStringLiteral("a manifest survives its JSON form"), why);
    Manifest shuffled = m;
    std::reverse(shuffled.entries.begin(), shuffled.entries.end());
    QJsonObject sj = shuffled.toJson();
    Manifest again;
    Manifest::fromJson(sj, &again);
    check(again.stateHash() == m.stateHash(), QStringLiteral("the state hash does not depend on the order of the entries"));
    // the cache
    const QByteArray cache = readFileBytes(f + "/livesync/hashes.json");
    check(!cache.isEmpty(), QStringLiteral("hashes are cached by size and time"));
    // bad manifests
    auto withEntry = [&](const QString &path, const QString &sha = QString(64, 'a')) {
        QJsonObject o = m.toJson();
        QJsonArray list = o.value("entries").toArray();
        list.append(QJsonObject{{"path", path}, {"size", 1}, {"sha256", sha}, {"rev", 0}});
        o.insert("entries", list);
        return o;
    };
    Manifest x;
    check(!Manifest::fromJson(withEntry("../evil.json"), &x), QStringLiteral("a manifest with a path above the folder is refused"));
    check(!Manifest::fromJson(withEntry("content/docZ/CON.md"), &x), QStringLiteral("a manifest with a Windows reserved name is refused"));
    check(!Manifest::fromJson(withEntry("content/DOCA/text.md"), &x), QStringLiteral("a manifest with two paths that differ only by case is refused"));
    check(!Manifest::fromJson(withEntry("random/file.txt"), &x), QStringLiteral("a manifest with a path of no class is refused"));
    check(!Manifest::fromJson(withEntry("livesync/state.json"), &x), QStringLiteral("a manifest cannot name Live Sync's own files"));
    check(!Manifest::fromJson(withEntry("content/docZ/text.md", "short"), &x), QStringLiteral("a manifest with a wrong hash is refused"));
    QJsonObject fmt = m.toJson();
    fmt.insert("format", 9);
    check(!Manifest::fromJson(fmt, &x), QStringLiteral("a manifest of a format this version does not know is refused"));
    QJsonObject dup = m.toJson();
    QJsonArray l2 = dup.value("entries").toArray();
    l2.append(l2.at(0));
    dup.insert("entries", l2);
    check(!Manifest::fromJson(dup, &x), QStringLiteral("a path listed twice is refused"));
    // compare
    Manifest local = m;
    local.entries.removeAt(1);
    local.entries[2].sha256 = QString(64, 'c');
    ManifestEntry extra;
    extra.path = "content/docQ/text.md";
    extra.sha256 = QString(64, 'd');
    local.entries << extra;
    std::sort(local.entries.begin(), local.entries.end(), [](const ManifestEntry &a, const ManifestEntry &b) { return a.path < b.path; });
    const ManifestDiff d = compare(m, local);
    check(d.fetch.size() == 2 && d.remove == QStringList{"content/docQ/text.md"}, QStringLiteral("compare: fetch what is missing or different, remove what the other does not list"), d.fetch.join(","));
}

// ---------------------------------------------------------------- 4. blobs

static void testBlobs(const QString &tmp)
{
    std::printf("== blobs\n");
    BlobStore storeA(tmp + "/blobA"), storeB(tmp + "/blobB");
    const QByteArray pa = QCryptographicHash::hash("a", QCryptographicHash::Sha256), pb = QCryptographicHash::hash("b", QCryptographicHash::Sha256);
    MemLink::Pair pr = MemLink::createPair(pa, pb, nullptr);
    BlobHub sender(pr.a, &storeA), receiver(pr.b, &storeB);
    QStringList got, failed;
    QObject::connect(&receiver, &BlobHub::received, &receiver, [&](const QString &h) { got << h; });
    QObject::connect(&receiver, &BlobHub::failed, &receiver, [&](const QString &h, const QString &c) { failed << h + ":" + c; });
    QObject::connect(pr.b, &net::Link::frameReceived, &receiver, [&](const Frame &f) {
        if (f.kind == Frame::Data)
            receiver.handleData(f);
        else
            receiver.handleMessage(f.object());
    });
    receiver.setFreeSpaceProbe([] { return qint64(1) << 40; });
    sender.setFreeSpaceProbe([] { return qint64(1) << 40; });
    QByteArray big(700 * 1024, 0);
    for (int i = 0; i < big.size(); ++i)
        big[i] = char((i * 31 + (i >> 8)) & 0xff);
    const QString hb = storeA.put(big);
    check(sender.send(hb, QString()) && waitFor([&] { return got.contains(hb); }), QStringLiteral("a 700 KiB blob goes over in several data frames and is stored under its hash"));
    QByteArray back;
    check(storeB.read(hb, &back) && back == big, QStringLiteral("the received bytes equal the sent ones"));
    check(storeB.partialSize(hb) == 0, QStringLiteral("no partial file is left after a whole transfer"));
    const QString he = storeA.put(QByteArray());
    got.clear();
    check(sender.send(he, QString()) && waitFor([&] { return got.contains(he); }) && storeB.has(he), QStringLiteral("an empty blob goes over"));
    // resume: a partial file of 300000 bytes, the sender starts at that offset
    const QString h2 = sha256Hex(big.left(500000));
    storeA.put(big.left(500000));
    QDir().mkpath(storeB.dir() + "/partial");
    writeBytes(storeB.partialPath(h2), big.left(300000));
    got.clear();
    check(sender.send(h2, QString(), QString(), 300000) && waitFor([&] { return got.contains(h2); }), QStringLiteral("a transfer resumes where the partial file ends"));
    check(storeB.read(h2, &back) && back == big.left(500000), QStringLiteral("a resumed blob equals the original"));
    // a partial file that does not match the offset is refused when the offset is not 0
    const QString h3 = sha256Hex(big.left(400000));
    storeA.put(big.left(400000));
    writeBytes(storeB.partialPath(h3), big.left(100));
    failed.clear();
    sender.send(h3, QString(), QString(), 200);
    waitFor([&] { return !failed.isEmpty(); }, 1000);
    check(!failed.isEmpty() && !storeB.has(h3), QStringLiteral("an offset that is not the end of the partial file is refused"));
    // hash mismatch
    failed.clear();
    const QString lie = sha256Hex("something else");
    const QString tr = newTransferId();
    pr.a->sendJson(make("BLOB_PUT", QString(), QJsonObject{{"transfer", tr}, {"hash", lie}, {"size", 5}, {"offset", 0}}));
    pr.a->sendData(transferBytes(tr), 0, "hello");
    pr.a->sendJson(make("BLOB_DONE", QString(), QJsonObject{{"transfer", tr}}));
    waitFor([&] { return !failed.isEmpty(); }, 1000);
    check(failed.contains(lie + ":HASH_MISMATCH") && !storeB.has(lie) && storeB.partialSize(lie) == 0, QStringLiteral("content that does not match its hash is refused and not kept"), failed.join(","));
    // an incomplete blob
    failed.clear();
    const QString h4 = sha256Hex("hello world");
    const QString tr4 = newTransferId();
    pr.a->sendJson(make("BLOB_PUT", QString(), QJsonObject{{"transfer", tr4}, {"hash", h4}, {"size", 11}, {"offset", 0}}));
    pr.a->sendData(transferBytes(tr4), 0, "hello");
    pr.a->sendJson(make("BLOB_DONE", QString(), QJsonObject{{"transfer", tr4}}));
    waitFor([&] { return !failed.isEmpty(); }, 1000);
    check(!failed.isEmpty() && !storeB.has(h4), QStringLiteral("a blob that ends before its size is refused"), failed.join(","));
    // data out of order
    failed.clear();
    const QString tr5 = newTransferId();
    pr.a->sendJson(make("BLOB_PUT", QString(), QJsonObject{{"transfer", tr5}, {"hash", h4}, {"size", 11}, {"offset", 0}}));
    pr.a->sendData(transferBytes(tr5), 5, "hello");
    waitFor([&] { return !failed.isEmpty(); }, 1000);
    check(failed.contains(h4 + ":BAD_FRAME"), QStringLiteral("a data frame that does not continue the blob is refused"), failed.join(","));
    // too big, no space
    failed.clear();
    pr.a->sendJson(make("BLOB_PUT", QString(), QJsonObject{{"transfer", newTransferId()}, {"hash", sha256Hex("x")}, {"size", double(kMaxBlobBytes + 1)}, {"offset", 0}}));
    waitFor([&] { return !failed.isEmpty(); }, 1000);
    check(failed.contains(sha256Hex("x") + ":TOO_BIG"), QStringLiteral("a blob over 2 GiB is refused"));
    failed.clear();
    receiver.setFreeSpaceProbe([] { return qint64(10) * 1024 * 1024; });
    pr.a->sendJson(make("BLOB_PUT", QString(), QJsonObject{{"transfer", newTransferId()}, {"hash", sha256Hex("y")}, {"size", double(1024 * 1024)}, {"offset", 0}}));
    waitFor([&] { return !failed.isEmpty(); }, 1000);
    check(failed.contains(sha256Hex("y") + ":NO_SPACE"), QStringLiteral("a blob is refused when the free space does not cover its size + 64 MiB"));
    receiver.setFreeSpaceProbe([] { return qint64(1) << 40; });
    // a partition: frames wait, then arrive in order
    got.clear();
    const QString h6 = storeA.put(QByteArray(300000, 'q'));
    pr.a->setHeld(true);
    sender.send(h6, QString());
    settle();
    check(!storeB.has(h6), QStringLiteral("while the link is held back nothing arrives"));
    pr.a->setHeld(false);
    check(waitFor([&] { return got.contains(h6); }), QStringLiteral("after the partition the blob arrives whole"));
    check(!sender.send(sha256Hex("not there"), QString()), QStringLiteral("a blob that is not in the store cannot be sent"));
    check(!storeA.has("../x") && !storeA.has("abc") && storeA.read("zz", &back) == false, QStringLiteral("the store only knows hashes"));
}

// ---------------------------------------------------------------- 5. outbox

static void testOutbox(const QString &tmp)
{
    std::printf("== outbox\n");
    const QString root = tmp + "/ob";
    QString err;
    {
        Outbox o(root);
        o.load();
        QHash<QString, QByteArray> blobs;
        const Unit u1 = textUnit("docA", "one", &blobs);
        const QString id1 = o.add(QJsonArray{toJson(u1)}, blobs, 1000, &err);
        blobs.clear();
        const Unit u2 = textUnit("docB", "two", &blobs);
        const QString id2 = o.add(QJsonArray{toJson(u2)}, blobs, 2000, &err);
        check(!id1.isEmpty() && !id2.isEmpty() && o.pending().size() == 2 && o.blobs().has(sha256Hex("one")), QStringLiteral("entries and their bytes are kept on disk"), err);
        QHash<QString, QByteArray> wrong;
        wrong.insert(QString(64, 'e'), "not that");
        check(o.add(QJsonArray(), wrong, 0, &err).isEmpty(), QStringLiteral("bytes that do not match their hash are not kept"));
    }
    {
        Outbox o(root); // after a restart
        o.load();
        const QList<OutboxEntry> p = o.pending();
        check(p.size() == 2 && p.at(0).createdMs == 1000 && p.at(1).createdMs == 2000, QStringLiteral("after a restart the entries are there, in their order"));
        check(o.toDraft(p.at(0).opId, "LOCK_REVOKED") && o.pending().size() == 1 && o.drafts().size() == 1 && o.blobs().has(sha256Hex("one")),
              QStringLiteral("an entry that cannot be sent moves to the drafts with its bytes"));
        check(o.done(p.at(1).opId) && !o.blobs().has(sha256Hex("two")) && o.pending().isEmpty(), QStringLiteral("a finished entry goes and takes its bytes along when nothing else needs them"));
        check(o.referenced(sha256Hex("one")) && !o.referenced(sha256Hex("two")), QStringLiteral("bytes a draft names are still referenced"));
    }
    {
        Outbox o(root);
        o.load();
        check(o.drafts().size() == 1 && o.drafts().first().reason == "LOCK_REVOKED", QStringLiteral("drafts survive a restart with their reason"));
        check(o.removeDraft(o.drafts().first().opId) && !o.blobs().has(sha256Hex("one")), QStringLiteral("a draft that is deleted by the writer takes its bytes along"));
    }
    // a damaged entry is never dropped
    writeBytes(root + "/outbox/000000000009-bad.json", "{ not json");
    Outbox o(root);
    o.load();
    check(o.pending().isEmpty() && QFileInfo::exists(root + "/drafts/000000000009-bad.json"), QStringLiteral("a damaged entry is set aside, not deleted"));
}

// ---------------------------------------------------------------- 6. sessions: join, commit, events

static void testJoinAndCommit(const QString &)
{
    std::printf("== join and commit\n");
    World w("jc");
    check(w.startHost(), QStringLiteral("the host starts on a project folder"));
    check(QFileInfo::exists(w.hostFolder + "/livesync/state.json") && w.host->seq() == 0 && w.host->epoch() == 1, QStringLiteral("a new host has its state file, seq 0, epoch 1"));
    auto a = w.client("devA");
    check(w.join(*a), QStringLiteral("a client joins and is ready"));
    check(a->s->replicaReady() && treeHash(a->folder) == treeHash(w.hostFolder), QStringLiteral("the full copy equals the host's folder (same manifest hash)"));
    check(a->s->role() == "editor" && a->s->epoch() == 1 && a->s->seq() == 0, QStringLiteral("the join tells role, epoch and head"));
    check(!QFileInfo::exists(a->folder + "/livesync") && QFileInfo::exists(a->folder + "/livesync-replica.json"), QStringLiteral("Live Sync's own state of the host is not copied"));
    auto b = w.client("devB");
    check(w.join(*b), QStringLiteral("a second client joins"));

    // a text commit with the lock
    check(lockOn(w, *a, "doc:docA"), QStringLiteral("A locks the document"));
    const QByteArray text = "A new text of the first chapter.\n";
    const QString op = commitText(*a, "docA", text);
    check(!op.isEmpty() && waitCommitted(*a, 1), QStringLiteral("A's change is committed"));
    check(hostText(w, "docA") == text && w.host->seq() == 1 && w.host->revOf("doc:docA") == 1, QStringLiteral("the host's file, seq and revision are those of the commit"));
    check(waitFor([&] { return readFileBytes(b->folder + "/content/docA/text.md") == text && b->s->seq() == 1; }), QStringLiteral("B's replica follows by the event"));
    check(waitFor([&] { return readFileBytes(a->folder + "/content/docA/text.md") == text && a->s->seq() == 1; }), QStringLiteral("A's own replica is written from the host's event, not by A itself"));
    check(treeHash(a->folder) == treeHash(w.hostFolder) && treeHash(b->folder) == treeHash(w.hostFolder), QStringLiteral("all three folders are equal"));
    check(a->s->revOf("doc:docA") == 1 && b->s->revOf("doc:docA") == 1, QStringLiteral("the revision is known to both clients"));
    const QJsonObject log = w.host->logEntry(1);
    check(log.value("opId").toString() == op && log.value("actor").toString() == a->device && !log.value("files").toArray().isEmpty()
              && !QJsonDocument(log).toJson().contains("A new text"),
          QStringLiteral("the commit record names operation, actor and files, and holds no text"));
    check(a->s->outbox().pending().isEmpty() && !a->s->outbox().blobs().has(sha256Hex(text)), QStringLiteral("the outbox entry and its bytes are gone after the commit"));

    // an operation on the project file: a new title, with the document's lock
    Op f;
    f.kind = Op::ItemFields;
    f.id = "docA";
    f.element = "<Item id=\"docA\" type=\"text\" created=\"2026-01-01T00:00:00\"><Title>Renamed chapter</Title></Item>";
    Unit u;
    u.name = "title";
    u.ops << f;
    QString err;
    const QString op2 = a->s->submit({u}, {}, &err);
    check(!op2.isEmpty() && waitCommitted(*a, 2), QStringLiteral("a change of a field is committed"), err);
    check(readFileBytes(w.hostFolder + "/Test.qbx").contains("Renamed chapter") && waitFor([&] { return readFileBytes(b->folder + "/Test.qbx").contains("Renamed chapter"); }),
          QStringLiteral("the project file is changed on the host and in the replicas (the host applies the operations)"));
    check(w.host->revOf("doc:docA") == 2, QStringLiteral("a field and the text are the same resource: one revision counter"));
    // a client that joins late gets everything
    auto c = w.client("devC");
    check(w.join(*c) && treeHash(c->folder) == treeHash(w.hostFolder) && c->s->seq() == 2, QStringLiteral("a client that joins later gets the current state"));
    // roles and projects
    auto none = w.client("devN", Role::None);
    w.link(*none);
    waitFor([&] { return none->s->state() == ClientSession::State::Failed || w.refusals.contains("NO_ACCESS"); }, 2000);
    check(w.refusals.contains("NO_ACCESS") && none->s->state() != ClientSession::State::Ready, QStringLiteral("a device without access gets NO_ACCESS and no copy"));
    check(!QFileInfo::exists(none->folder + "/content"), QStringLiteral("nothing of the project reaches a device without access"));
    auto raw = rawPeer(w, "devRaw", Role::Editor, true, false);
    raw->send(make("JOIN", "00000000-0000-0000-0000-000000000000", QJsonObject{{"epoch", 1}, {"seq", 0}}));
    settle();
    check(raw->codes.contains("UNKNOWN_PROJECT"), QStringLiteral("a project the host does not host: UNKNOWN_PROJECT"));
    auto old = rawPeer(w, "devOld", Role::Editor, false, false);
    old->send(make("HELLO", QString::fromLatin1(kUuid), QJsonObject{{"protoMin", 2}, {"protoMax", 3}}));
    settle();
    check(old->codes.contains("UNSUPPORTED_VERSION"), QStringLiteral("a device that needs another protocol version: UNSUPPORTED_VERSION"));
    auto early = rawPeer(w, "devEarly", Role::Editor, false, false);
    early->send(make("JOIN", QString::fromLatin1(kUuid), QJsonObject{{"epoch", 1}, {"seq", 0}}));
    settle();
    check(early->codes.contains("BAD_OP") && early->got.size() == 1, QStringLiteral("anything before HELLO is refused"));
    // the project list tells only what the device may see
    auto lister = rawPeer(w, "devList", Role::Reader, true, false);
    lister->send(make("PROJECT_LIST"));
    auto lister2 = rawPeer(w, "devList2", Role::None, true, false);
    lister2->send(make("PROJECT_LIST"));
    settle();
    check(lastOf(*lister, "PROJECTS").value("projects").toArray().size() == 1 && lastOf(*lister2, "PROJECTS").value("projects").toArray().isEmpty(),
          QStringLiteral("PROJECTS lists only what is shared with the asker"));
}

static void testIdempotency()
{
    std::printf("== idempotency\n");
    World w("idem");
    w.startHost();
    auto a = w.client("devA");
    w.join(*a);
    lockOn(w, *a, "doc:docA");
    // the same operation id twice: one commit
    auto raw = rawPeer(w, "devRaw", Role::Editor);
    const QByteArray t1 = "first";
    upload(*raw, t1);
    const QString opId = newId();
    const QJsonArray units{putUnit("content/docB/text.md", t1, 0, "doc:docB")};
    raw->send(rawCommit(opId, units));
    settle();
    const QJsonObject r1 = lastOf(*raw, "COMMIT_RESULT");
    check(r1.value("status").toString() == "ok" && r1.value("seq").toInt() == 1 && w.host->seq() == 1, QStringLiteral("a request is committed"));
    raw->send(rawCommit(opId, units));
    settle();
    const QJsonObject r2 = lastOf(*raw, "COMMIT_RESULT");
    check(r2.value("status").toString() == "ok" && r2.value("duplicate").toBool() && r2.value("seq").toInt() == 1 && w.host->seq() == 1 && r2.value("revs") == r1.value("revs"),
          QStringLiteral("a repeated operation id gets the stored result and is not applied again"));
    check(readFileBytes(w.hostFolder + "/content/docB/text.md") == t1 && w.host->revOf("doc:docB") == 1, QStringLiteral("the file and the revision were changed once"));
    // a repeat of the very same message (same message id): not done twice
    const QJsonObject again = rawCommit(newId(), QJsonArray{putUnit("content/docB/text.md", "second", 1, "doc:docB")});
    upload(*raw, "second");
    raw->send(again);
    raw->send(again);
    settle();
    check(raw->codes.contains("DUPLICATE") && w.host->seq() == 2, QStringLiteral("a replayed message is answered with DUPLICATE and not applied twice"));
    // a lost result: the client sends again after reconnecting, and the host answers from its memory
    w.advance(100);
    a->toHost->setDropping(false);
    a->atHost->setDropping(true); // the host's answers (result and event) vanish
    const QString op = commitText(*a, "docA", "text written while the answers are lost");
    waitFor([&] { return w.host->seq() == 3; });
    settle();
    check(w.host->seq() == 3 && a->committed == 0 && a->s->outbox().pending().size() == 1, QStringLiteral("the host committed, the client never heard: its outbox entry stays"));
    w.cut(*a);
    w.link(*a);
    check(waitCommitted(*a, 1) && w.host->seq() == 3 && a->s->outbox().pending().isEmpty(), QStringLiteral("after reconnecting the same request gets the stored result: one commit, outbox empty"));
    check(a->committedOps.contains(op), QStringLiteral("the client's committed signal names the operation"));
    check(waitFor([&] { return a->s->seq() == 3 && treeHash(a->folder) == treeHash(w.hostFolder); }), QStringLiteral("the client then catches up by replay of the commits it missed"));
    // the host remembers operations across a restart
    w.host->stop();
    w.cut(*a);
    w.host.reset();
    w.startHost();
    check(w.host->seq() == 3 && w.host->knowsOp(op) && w.host->knowsOp(opId), QStringLiteral("a restarted host knows the operations of its log"));
    auto raw2 = rawPeer(w, "devRaw", Role::Editor);
    raw2->send(rawCommit(opId, units, 1, 0));
    settle();
    const QJsonObject r3 = lastOf(*raw2, "COMMIT_RESULT");
    check(r3.value("duplicate").toBool() && w.host->seq() == 3, QStringLiteral("a repeat after the host's restart is still not applied again"));
    // an operation older than the host remembers
    const QString old = newId();
    QDir(w.hostFolder + "/livesync/log").remove("000000000001.json");
    QDir(w.hostFolder + "/livesync/log").remove("000000000002.json");
    w.host->stop();
    w.host.reset();
    w.startHost();
    check(w.host->opHorizon() == 2, QStringLiteral("the host knows up to where its log reaches"), QString::number(w.host->opHorizon()));
    auto raw3 = rawPeer(w, "devRaw", Role::Editor);
    upload(*raw3, "x");
    raw3->send(rawCommit(old, QJsonArray{putUnit("content/docC/text.md", "x", 0, "doc:docC")}, 1, 0));
    settle();
    const QJsonObject r4 = lastOf(*raw3, "COMMIT_RESULT");
    check(r4.value("code").toString() == "UNKNOWN_OLD" && w.host->seq() == 3 && readFileBytes(w.hostFolder + "/content/docC/text.md") != "x", QStringLiteral("an unknown, older operation gets UNKNOWN_OLD and is never applied blindly"));
}

static void testGapsAndReplay()
{
    std::printf("== gaps and replay\n");
    World w("gap");
    w.startHost();
    auto a = w.client("devA");
    auto b = w.client("devB");
    w.join(*a);
    w.join(*b);
    lockOn(w, *a, "doc:docA");
    // B misses three commits (its link is cut), then returns and replays
    w.cut(*b);
    for (int i = 1; i <= 3; ++i) {
        commitText(*a, "docA", QByteArray("version ") + QByteArray::number(i));
        waitCommitted(*a, i);
    }
    check(w.host->seq() == 3 && b->s->seq() == 0, QStringLiteral("B missed 3 commits"));
    w.link(*b);
    check(waitFor([&] { return b->s->state() == ClientSession::State::Ready && b->s->seq() == 3; }), QStringLiteral("B returns and is brought up by replay from the host's log"));
    check(treeHash(b->folder) == treeHash(w.hostFolder) && b->s->fullSyncCount() == 1, QStringLiteral("the replayed replica equals the host's, without a second full copy"), QString::number(b->s->fullSyncCount()));
    // an event that arrives out of order or with a hole: the client asks for the missing ones
    b->atHost->setDropping(true);
    commitText(*a, "docA", "version 4");
    waitCommitted(*a, 4);
    b->atHost->setDropping(false);
    commitText(*a, "docA", "version 5");
    waitCommitted(*a, 5);
    check(waitFor([&] { return b->s->seq() == 5; }) && treeHash(b->folder) == treeHash(w.hostFolder), QStringLiteral("an event with a hole before it leads to RESYNC and the copy is whole again"));
    // the log does not cover the range: GAP and a copy by manifest
    w.cut(*b);
    for (int i = 6; i <= 8; ++i) {
        commitText(*a, "docA", QByteArray("version ") + QByteArray::number(i));
        waitCommitted(*a, i);
    }
    for (int i = 5; i <= 7; ++i)
        QFile::remove(w.hostFolder + QString("/livesync/log/%1.json").arg(i, 12, 10, QLatin1Char('0')));
    w.link(*b);
    check(waitFor([&] { return b->s->state() == ClientSession::State::Ready && b->s->seq() == 8; }) && b->s->fullSyncCount() == 2 && treeHash(b->folder) == treeHash(w.hostFolder),
          QStringLiteral("when the log is missing a commit the host says GAP and the replica is made again by manifest"), QString::number(b->s->fullSyncCount()));
    // repairing a replica with files that differ
    writeBytes(b->folder + "/content/docB/text.md", "damaged");
    QFile::remove(b->folder + "/content/docC/text.md");
    writeBytes(b->folder + "/content/docX/text.md", "alien");
    b->s->resync();
    check(waitFor([&] { return b->s->fullSyncCount() == 3 && b->s->state() == ClientSession::State::Ready && treeHash(b->folder) == treeHash(w.hostFolder); }),
          QStringLiteral("a copy by manifest fetches what differs and removes what the host does not have"));
    // a client that cannot reach the host keeps its request and sends it when it can
    w.cut(*a);
    const QString op = commitText(*a, "docB", "written offline");
    settle();
    check(!op.isEmpty() && a->s->outbox().pending().size() == 1 && w.host->seq() == 8, QStringLiteral("offline: the request waits in the outbox, nothing is sent"));
    w.link(*a);
    check(waitCommitted(*a, 9) && w.host->seq() == 9 && hostText(w, "docB") == "written offline", QStringLiteral("when the link is back the outbox goes through by itself"));
}

// ---------------------------------------------------------------- 7. locks over the network

static void testLockRules()
{
    std::printf("== lock rules\n");
    World w("lk");
    w.startHost();
    auto a = w.client("devA");
    auto b = w.client("devB");
    w.join(*a);
    w.join(*b);
    // two requests at once: exactly one wins
    a->s->lock({"doc:docA"});
    b->s->lock({"doc:docA"});
    settle();
    const bool aHas = a->s->holds("doc:docA"), bHas = b->s->holds("doc:docA");
    check(aHas != bHas && (aHas ? b->denied : a->denied).contains("doc:docA:LOCK_HELD"), QStringLiteral("two requests for one document at once: exactly one wins, the other is told it is held"));
    Client &holder = aHas ? *a : *b;
    Client &other = aHas ? *b : *a;
    check(other.s->heldElsewhere("doc:docA") == holder.name, QStringLiteral("the others see who holds the document"));
    // structure beside text
    check(lockOn(w, other, "structure"), QStringLiteral("structure beside text: the structure lock is free while someone writes a chapter"));
    // a change of the held document by the other device is rejected, the structure change goes through
    commitText(other, "docA", "text by the wrong device");
    waitFor([&] { return !other.drafts.isEmpty(); });
    check(!other.drafts.isEmpty() && other.drafts.first().endsWith(":LOCK_HELD") && other.rejectedCodes.contains("LOCK_HELD") && hostText(w, "docA") == "Text of docA\n",
          QStringLiteral("a text change of a document someone else holds is rejected, nothing is written, the content is kept as a draft"));
    check(other.s->outbox().drafts().size() == 1 && other.s->outbox().blobs().has(sha256Hex("text by the wrong device")), QStringLiteral("the draft keeps the typed text"));
    Op mv;
    mv.kind = Op::ItemMove;
    mv.id = "docB";
    mv.parent = "fold1";
    mv.after = "docC";
    Unit mu;
    mu.name = "move";
    mu.ops << mv;
    other.s->submit({mu}, {});
    check(waitCommitted(other, 1) && readFileBytes(w.hostFolder + "/Test.qbx").indexOf("docB") > readFileBytes(w.hostFolder + "/Test.qbx").indexOf("fold1"), QStringLiteral("a move goes through while another device edits a chapter"));
    // trashing a document needs the document lock too (and the ones below)
    Op tr;
    tr.kind = Op::ItemTrash;
    tr.id = "fold1";
    tr.parent = "TRASH";
    Unit tu;
    tu.name = "trash";
    tu.ops << tr;
    // fold1 contains docC (and docB now)
    lockOn(w, holder, "doc:docC");
    other.s->submit({tu}, {});
    waitFor([&] { return other.drafts.size() >= 2; });
    check(other.drafts.size() >= 2 && other.rejectedCodes.count("LOCK_HELD") >= 2 && readFileBytes(w.hostFolder + "/Test.qbx").indexOf("id=\"fold1\"") < readFileBytes(w.hostFolder + "/Test.qbx").indexOf("id=\"TRASH\""),
          QStringLiteral("trashing a folder is refused while someone holds a document below it"));
    // unrelated documents do not conflict
    check(lockOn(w, other, "doc:docB") && holder.s->holds("doc:docA"), QStringLiteral("locks on different documents are independent"));
    // the host's table
    check(w.host->locks().all().size() >= 4, QStringLiteral("the host keeps the table"));
    // the project lock excludes everyone
    other.s->lock({"project"});
    settle();
    check(!other.s->holds("project") && other.denied.contains("project:LOCK_HELD"), QStringLiteral("the whole-project lock is refused while others hold locks"));
    // reader
    auto rd = w.client("devR", Role::Reader);
    w.join(*rd);
    rd->s->lock({"doc:docC"});
    settle();
    check(rd->denied.contains("doc:docC:READ_ONLY") && !rd->s->holds("doc:docC"), QStringLiteral("a reader cannot lock"));
    commitText(*rd, "docC", "reader text");
    waitFor([&] { return !rd->drafts.isEmpty(); });
    check(!rd->drafts.isEmpty() && rd->drafts.first().endsWith(":READ_ONLY") && hostText(w, "docC") == "Text of docC\n", QStringLiteral("a reader's change is refused with READ_ONLY and kept as a draft"));
    // releasing after the flush
    holder.s->unlock({"doc:docA"});
    check(waitFor([&] { return !holder.s->holds("doc:docA") && other.s->heldElsewhere("doc:docA").isEmpty(); }), QStringLiteral("a lock is released when asked, and the others see it go"));
    check(lockOn(w, other, "doc:docA"), QStringLiteral("then another device can take it"));
}

static void testLeases()
{
    std::printf("== leases\n");
    World w("lease");
    w.startHost();
    auto a = w.client("devA");
    auto b = w.client("devB");
    w.join(*a);
    w.join(*b);
    lockOn(w, *a, "doc:docA");
    // with heartbeats a lock stays for ever, also unused
    w.advance(100 * 1000);
    check(a->s->holds("doc:docA") && w.host->locks().find("doc:docA") && b->s->heldElsewhere("doc:docA") == "devA", QStringLiteral("an unused lock stays for 100 s (over three lease times) with only heartbeats: no idle release"));
    // A is cut off
    a->toHost->setDropping(true);
    w.advance(11000);
    check(b->s->isUnreachable("doc:docA") && w.host->locks().find("doc:docA"), QStringLiteral("after 10 s without a heartbeat the others see the lock as not reachable"));
    w.advance(10000);
    check(w.host->locks().find("doc:docA") != nullptr && b->s->heldElsewhere("doc:docA") == "devA", QStringLiteral("at 21 s the lease is still there"));
    w.advance(10000);
    check(!w.host->locks().find("doc:docA") && b->s->heldElsewhere("doc:docA").isEmpty(), QStringLiteral("at 31 s without a heartbeat the lease is gone"));
    check(lockOn(w, *b, "doc:docA"), QStringLiteral("and another device can lock the document"));
    a->toHost->setDropping(false);
    w.advance(3500);
    check(a->revoked.contains("doc:docA:expired") && !a->s->holds("doc:docA"), QStringLiteral("the old holder is told that its lease is gone"), a->revoked.join(","));
    // the old holder's changes are refused with its old token
    QHash<QString, QByteArray> blobs;
    const QString op = a->s->submit({textUnit("docA", "late text", &blobs)}, blobs);
    waitFor([&] { return !a->drafts.isEmpty(); });
    check(!a->drafts.isEmpty() && hostText(w, "docA") == "Text of docA\n" && !op.isEmpty(), QStringLiteral("changes of the old holder are not taken (they become a draft)"), a->drafts.join(","));
}

static void testEditRequests()
{
    std::printf("== edit requests\n");
    World w("edit");
    w.startHost();
    auto a = w.client("devA");
    auto b = w.client("devB");
    w.join(*a);
    w.join(*b);
    lockOn(w, *a, "doc:docA");
    QString note;
    // B asks
    b->s->askToEdit("doc:docA");
    settle();
    check(a->editAsked.contains("doc:docA") && !b->statuses.isEmpty() && b->statuses.last().waitMs == 20000 && !b->statuses.last().declined && !b->statuses.last().mayForce,
          QStringLiteral("A is asked, B is told to wait 20 s"));
    // taking over at once is refused
    b->s->takeOver("doc:docA");
    settle();
    check(w.refusals.contains("LOCK_HELD") && !b->s->holds("doc:docA") && a->s->holds("doc:docA") && w.snapshotLabels.isEmpty(), QStringLiteral("a takeover before the time is out is refused and changes nothing"));
    w.advance(15000);
    // A declines: the time starts again
    a->s->answerEdit("doc:docA", false);
    settle();
    check(b->statuses.last().declined && b->statuses.last().waitMs == 20000 && !b->statuses.last().mayForce, QStringLiteral("a decline restarts B's waiting time and tells B that the document is being worked on"));
    check(b->s->editWaitMs("doc:docA") == 20000 && a->s->holds("doc:docA"), QStringLiteral("B's time left is 20 s again; A still holds the lock"));
    w.advance(19000);
    b->s->takeOver("doc:docA");
    settle();
    check(!b->s->holds("doc:docA") && a->s->holds("doc:docA"), QStringLiteral("19 s after the decline a takeover is still refused"));
    // A declines again, later
    a->s->answerEdit("doc:docA", false);
    settle();
    check(b->s->editWaitMs("doc:docA") == 20000, QStringLiteral("every decline restarts the time"));
    // no answer for 20 s: B may take over
    w.advance(20500);
    check(!b->statuses.isEmpty() && b->statuses.last().mayForce && b->s->editWaitMs("doc:docA") == 0, QStringLiteral("with no answer for 20 s B is told that it may take over"));
    // the snapshot failing stops the takeover
    w.snapshotFails = true;
    b->s->takeOver("doc:docA");
    settle();
    check(!b->s->holds("doc:docA") && a->s->holds("doc:docA") && w.snapshotLabels.size() == 1 && w.snapshotLabels.first() == "Before takeover by devB",
          QStringLiteral("when the protective snapshot fails the takeover is refused and nothing changes"), w.snapshotLabels.join(","));
    w.snapshotFails = false;
    // A has unsaved changes pending at the time
    a->toHost->setDropping(true); // A's messages do not arrive: its change stays unconfirmed
    QHash<QString, QByteArray> blobs;
    a->s->submit({textUnit("docA", "A's unconfirmed text", &blobs)}, blobs);
    settle();
    const qint64 oldGen = a->s->token("doc:docA").generation;
    a->toHost->setDropping(false);
    b->s->takeOver("doc:docA");
    settle();
    check(b->s->holds("doc:docA") && b->s->token("doc:docA").generation > oldGen && w.snapshotLabels.size() == 2, QStringLiteral("after the snapshot B holds the lock with a higher generation"));
    check(waitFor([&] { return a->revoked.contains("doc:docA:takeover") && !a->s->holds("doc:docA"); }), QStringLiteral("A is told the lock was taken over"));
    check(a->s->outbox().pending().isEmpty() && a->s->outbox().drafts().size() >= 1 && a->s->outbox().blobs().has(sha256Hex("A's unconfirmed text")),
          QStringLiteral("A's unconfirmed text stays on A as a draft"));
    check(hostText(w, "docA") == "Text of docA\n", QStringLiteral("the host never took that text"));
    // the old token is refused
    auto raw = rawPeer(w, "devA", Role::Editor); // same device as A, another connection
    upload(*raw, "sneaky");
    const Token old{"doc:docA", 1, oldGen, "old"};
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docA/text.md", "sneaky", 0, "doc:docA")}, 1, 0, QJsonArray{toJson(old)}));
    settle();
    const QJsonObject res = lastOf(*raw, "COMMIT_RESULT");
    check(res.value("status").toString() == "rejected" && hostText(w, "docA") == "Text of docA\n", QStringLiteral("a commit with the old token is refused, whoever sends it"), QJsonDocument(res).toJson());
    // B commits
    commitText(*b, "docA", "B's text");
    check(waitCommitted(*b, 1) && hostText(w, "docA") == "B's text", QStringLiteral("B can write now"));
    // hand over
    b->events.clear();
    a->s->askToEdit("doc:docA");
    settle();
    check(b->editAsked.contains("doc:docA"), QStringLiteral("A asks B in its turn"));
    commitText(*b, "docA", "B's last text before the hand-over");
    b->s->answerEdit("doc:docA", true);
    check(waitFor([&] { return a->s->holds("doc:docA") && !b->s->holds("doc:docA"); }), QStringLiteral("Save and Hand Over: B's change is committed first, then A holds the lock"));
    check(hostText(w, "docA") == "B's last text before the hand-over", QStringLiteral("the held-over text is on the host"));
    // a request that the asker cancels
    b->s->askToEdit("doc:docA");
    settle();
    b->s->cancelAsk("doc:docA");
    settle();
    check(w.host->editRequests().count() == 0, QStringLiteral("an asker can cancel"));
    // the asker disappears: the request goes
    b->s->askToEdit("doc:docA");
    settle();
    check(w.host->editRequests().count() == 1, QStringLiteral("a request is open"));
    w.cut(*b);
    check(w.host->editRequests().count() == 0, QStringLiteral("a request ends with its asker's connection"));
    // the holder unreachable: the same 20 s, then a takeover
    w.link(*b);
    waitFor([&] { return b->s->state() == ClientSession::State::Ready; });
    a->toHost->setDropping(true);
    a->atHost->setDropping(true);
    b->s->askToEdit("doc:docA");
    settle();
    w.advance(5000);
    b->s->takeOver("doc:docA");
    settle();
    check(!b->s->holds("doc:docA"), QStringLiteral("an unreachable holder is waited for the same 20 s"));
    w.advance(16000);
    b->s->takeOver("doc:docA");
    settle();
    check(b->s->holds("doc:docA"), QStringLiteral("after the 20 s the lock of an unreachable holder can be taken over"));
}

static void testLockHandOverRelease()
{
    std::printf("== release grants the waiting asker\n");
    World w("rel");
    w.startHost();
    auto a = w.client("devA");
    auto b = w.client("devB");
    w.join(*a);
    w.join(*b);
    lockOn(w, *a, "doc:docB");
    b->s->askToEdit("doc:docB");
    settle();
    a->s->unlock({"doc:docB"});
    check(waitFor([&] { return b->s->holds("doc:docB"); }), QStringLiteral("when the holder lets go, the asker who waits is given the lock"));
    check(w.host->editRequests().count() == 0, QStringLiteral("and the request is closed"));
}

// ---------------------------------------------------------------- 8. tokens after a restart, the re-acquire rule

static void testRestartAndReacquire()
{
    std::printf("== restart and re-acquire\n");
    World w("re");
    w.startHost();
    auto a = w.client("devA");
    w.join(*a);
    lockOn(w, *a, "doc:docA");
    const qint64 oldGen = a->s->token("doc:docA").generation;
    commitText(*a, "docA", "before the restart");
    waitCommitted(*a, 1);
    // the host restarts: its table is empty, its generations go on
    w.cut(*a);
    w.host->stop();
    w.host.reset();
    w.startHost();
    w.host->setRole(a->device, Role::Editor);
    check(w.host->locks().all().isEmpty() && w.host->seq() == 1 && w.host->revOf("doc:docA") == 1, QStringLiteral("a restarted host keeps seq and revisions, and has no locks"));
    w.link(*a);
    check(waitFor([&] { return a->s->state() == ClientSession::State::Ready && a->s->holds("doc:docA"); }), QStringLiteral("the client asks again for the lock it held, and nothing changed under it: it gets it"));
    check(a->s->token("doc:docA").generation > oldGen, QStringLiteral("the new lock has a higher generation than any of the old host's (%1 > %2)").arg(a->s->token("doc:docA").generation).arg(oldGen));
    // pending changes go through by themselves with the new lock
    w.cut(*a);
    commitText(*a, "docA", "written while the host was away");
    w.host->stop();
    w.host.reset();
    w.startHost();
    w.host->setRole(a->device, Role::Editor);
    w.link(*a);
    check(waitCommitted(*a, 2) && hostText(w, "docA") == "written while the host was away", QStringLiteral("after a host restart the outbox goes through by itself (re-acquire rule)"));
    // someone else changed the document meanwhile: the old lock is not given back
    auto b = w.client("devB");
    w.join(*b);
    w.cut(*a);
    w.cut(*b);
    w.host->stop();
    w.host.reset();
    w.startHost();
    w.host->setRole(a->device, Role::Editor);
    w.host->setRole(b->device, Role::Editor);
    w.link(*b);
    waitFor([&] { return b->s->state() == ClientSession::State::Ready; });
    lockOn(w, *b, "doc:docA");
    commitText(*b, "docA", "B was first");
    waitCommitted(*b, 1);
    commitText(*a, "docA", "A's older change");
    w.link(*a);
    waitFor([&] { return a->s->state() == ClientSession::State::Ready; });
    waitFor([&] { return !a->drafts.isEmpty(); });
    check(!a->drafts.isEmpty() && hostText(w, "docA") == "B was first" && !a->s->holds("doc:docA"), QStringLiteral("when someone else has the document and changed it, A's old change becomes a draft and the lock is not given back"), a->drafts.join(","));
    check(a->revoked.contains("doc:docA:lost") || a->denied.contains("doc:docA:LOCK_HELD") || a->denied.contains("doc:docA:BASE_MISMATCH"), QStringLiteral("A is told that it no longer holds the lock"), a->revoked.join(",") + "|" + a->denied.join(","));
    // the rule at the host: a stale token whose resource is free and unchanged gets a new lock; a changed one does not
    auto raw = rawPeer(w, "devRaw", Role::Editor);
    upload(*raw, "free text");
    const Token stale{"doc:docC", 1, 12345, "gone"};
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docC/text.md", "free text", 0, "doc:docC")}, 1, 0, QJsonArray{toJson(stale)}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("status").toString() == "ok" && !lastOf(*raw, "LOCK_GRANTED").isEmpty() && w.host->locks().find("doc:docC"),
          QStringLiteral("a stale token whose resource is free and unchanged: the commit goes through and the holder gets a new lock"));
    upload(*raw, "second free text");
    const Token stale2{"doc:docB", 1, 777, "gone"};
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docB/text.md", "second free text", 5, "doc:docB")}, 1, 0, QJsonArray{toJson(stale2)}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("status").toString() == "rejected" && hostText(w, "docB") == "Text of docB\n", QStringLiteral("a stale token whose resource changed under it: refused"));
}

// ---------------------------------------------------------------- 9. what a client may not do

static void testSecurity(const QString &tmp)
{
    std::printf("== what a client may not do\n");
    World w("sec");
    w.startHost();
    auto raw = rawPeer(w, "devRaw", Role::Editor);
    auto tryPath = [&](const QString &path, const QString &label, const QByteArray &bytes = "evil") {
        const int before = int(w.host->seq());
        upload(*raw, bytes);
        raw->send(rawCommit(newId(), QJsonArray{putUnit(path, bytes, 0, "doc:docA")}));
        settle();
        const QJsonObject r = lastOf(*raw, "COMMIT_RESULT");
        const bool refused = r.value("status").toString() == "rejected" && r.value("code").toString() == "BAD_PATH" && w.host->seq() == before;
        check(refused, label, QJsonDocument(r).toJson(QJsonDocument::Compact));
    };
    tryPath("../evil.txt", QStringLiteral("a path above the project folder is refused (BAD_PATH), nothing is written"));
    check(!QFileInfo::exists(w.dir.path() + "/w/evil.txt"), QStringLiteral("and the file is not there"));
    tryPath("content/docA/../../evil.txt", QStringLiteral("a path with .. inside is refused"));
    tryPath("/etc/evil", QStringLiteral("an absolute path is refused"));
    tryPath("content\\docA\\text.md", QStringLiteral("a path with a backslash is refused"));
    tryPath("C:/evil.txt", QStringLiteral("a path with a drive letter is refused"));
    tryPath("content/docA/CON.md", QStringLiteral("a Windows reserved name is refused"));
    tryPath("content/docA/aux", QStringLiteral("a reserved name without extension is refused"));
    tryPath("content/docA/name ", QStringLiteral("a name that ends in a space is refused"));
    tryPath("content/docA/" + QString(300, 'x') + ".md", QStringLiteral("a path of more than 240 bytes is refused"));
    tryPath(QString("content/docA/a") + QChar(7) + "b.md", QStringLiteral("a control character in a path is refused"));
    tryPath("content/DOCA/text.md", QStringLiteral("a path that differs from an existing one only by case is refused"));
    tryPath("random/place.txt", QStringLiteral("a path of no class is refused"));
    tryPath("Test.qbx", QStringLiteral("the project file is never written as a file (only by operations)"));
    tryPath("content/docA/snapshots/s1/index.json", QStringLiteral("snapshot files are host jobs, not a client's"));
    tryPath("livesync/state.json", QStringLiteral("Live Sync's own state cannot be written by a client"));
    tryPath("livesync-local/outbox/x.json", QStringLiteral("nor can its local folder"));
    tryPath("settings/lock", QStringLiteral("device-local files are not a client's"));
    tryPath("encryption.json", QStringLiteral("the encryption marker is the host's"));
    // case collision inside one request
    upload(*raw, "a");
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docN/Text.md", "a", 0, "doc:docN"), putUnit("content/docN/TEXT.md", "a", 0, "doc:docN")}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "BAD_PATH" && !QFileInfo::exists(w.hostFolder + "/content/docN"), QStringLiteral("two paths of one request that differ only by case are refused"));
    // a symbolic link on the way
    const QString outside = tmp + "/outside";
    QDir().mkpath(outside);
    const bool linked = QFile::link(outside, w.hostFolder + "/content/docL");
    if (linked) {
        tryPath("content/docL/text.md", QStringLiteral("a symbolic link on the way is refused, nothing is written outside"));
        check(!QFileInfo::exists(outside + "/text.md"), QStringLiteral("and nothing reached the place the link leads to"));
    } else
        check(true, QStringLiteral("(no symbolic links on this file system: skipped)"));
    // wrong content
    const QString mis = newId();
    upload(*raw, "real bytes");
    QJsonObject unit = putUnit("content/docA/text.md", "real bytes", 0, "doc:docA");
    QJsonArray fl = unit.value("files").toArray();
    QJsonObject f0 = fl.at(0).toObject();
    f0.insert("size", 3);
    fl[0] = f0;
    unit.insert("files", fl);
    raw->send(rawCommit(mis, QJsonArray{unit}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "HASH_MISMATCH" && w.host->seq() == 0, QStringLiteral("a size that does not match the uploaded bytes is refused"));
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docA/text.md", "never uploaded", 0, "doc:docA")}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "BLOB_MISSING" && w.host->seq() == 0, QStringLiteral("a request whose bytes were not uploaded: BLOB_MISSING"));
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docA/text.md", "real bytes", 0, "doc:docA")}, 2));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "WRONG_EPOCH" && w.host->seq() == 0, QStringLiteral("a request of another epoch: WRONG_EPOCH"));
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docA/text.md", "real bytes", 9, "doc:docA")}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "BASE_MISMATCH" && w.host->seq() == 0, QStringLiteral("a request that builds on an old revision: BASE_MISMATCH"));
    Unit nobase;
    nobase.name = "nb";
    FileOp nf;
    nf.path = "content/docA/text.md";
    nf.hash = sha256Hex("real bytes");
    nf.size = 10;
    nobase.files << nf;
    raw->send(rawCommit(newId(), QJsonArray{toJson(nobase)}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "BASE_MISMATCH" && w.host->seq() == 0, QStringLiteral("a unit without a base revision for what it changes is refused"));
    // an operation that does not apply
    Op bad;
    bad.kind = Op::ItemFields;
    bad.id = "noSuchItem";
    bad.element = "<Item id=\"noSuchItem\"/>";
    Unit bu;
    bu.name = "bad";
    bu.ops << bad;
    bu.bases.insert("doc:noSuchItem", 0);
    raw->send(rawCommit(newId(), QJsonArray{toJson(bu)}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "BAD_OP" && w.host->seq() == 0, QStringLiteral("an operation that names an item that is not there: BAD_OP, nothing applied"));
    // all or nothing: one good unit and one bad
    upload(*raw, "good one");
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docB/text.md", "good one", 0, "doc:docB"), putUnit("../bad", "good one", 0, "doc:docB")}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("status").toString() == "rejected" && hostText(w, "docB") == "Text of docB\n" && w.host->seq() == 0, QStringLiteral("one bad unit rejects the whole request: nothing of it is applied"));
    check(lastOf(*raw, "COMMIT_RESULT").value("rejected").toArray().size() == 1 && lastOf(*raw, "COMMIT_RESULT").value("rejected").toArray().at(0).toObject().value("unit").toString() == "../bad",
          QStringLiteral("the rejection names the failed unit"));
    // too big
    Unit many;
    many.name = "many";
    many.bases.insert("doc:docA", 0);
    for (int i = 0; i < 9000; ++i) {
        Op o;
        o.kind = Op::ItemFields;
        o.id = "docA";
        o.element = "<Item id=\"docA\" note=\"" + QString::number(i) + "\"/>";
        many.ops << o;
    }
    raw->send(rawCommit(newId(), QJsonArray{toJson(many)}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "TOO_BIG" && w.host->seq() == 0, QStringLiteral("a request of more than 512 KiB: TOO_BIG"));
    // no space
    upload(*raw, "space text");
    settle();
    w.host->setFreeSpaceProbe([] { return qint64(1000); });
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docB/text.md", "space text", 0, "doc:docB")}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "NO_SPACE" && w.host->seq() == 0 && hostText(w, "docB") == "Text of docB\n", QStringLiteral("a full disk: NO_SPACE, nothing applied"), QJsonDocument(lastOf(*raw, "COMMIT_RESULT")).toJson(QJsonDocument::Compact));
    w.host->setFreeSpaceProbe([] { return qint64(1) << 40; });
    // a failing write: STORAGE_FAILED, nothing changed, no event
    auto watcher = w.client("devW");
    w.join(*watcher);
    const int ev = watcher->events.size();
    qputenv("QB_TEST_WRITE_FAILS", "28|No space left on device|livesync/log/");
    const QString failOp = newId();
    raw->send(rawCommit(failOp, QJsonArray{putUnit("content/docB/text.md", "space text", 0, "doc:docB")}));
    settle();
    qunsetenv("QB_TEST_WRITE_FAILS");
    check(lastOf(*raw, "COMMIT_RESULT").value("code").toString() == "STORAGE_FAILED" && w.host->seq() == 0 && hostText(w, "docB") == "Text of docB\n" && !w.host->knowsOp(failOp) && watcher->events.size() == ev,
          QStringLiteral("when the commit record cannot be written nothing changes and nobody is told"), QJsonDocument(lastOf(*raw, "COMMIT_RESULT")).toJson(QJsonDocument::Compact) + " seq=" + QString::number(w.host->seq()));
    raw->send(rawCommit(newId(), QJsonArray{putUnit("content/docB/text.md", "space text", 0, "doc:docB")}));
    settle();
    check(lastOf(*raw, "COMMIT_RESULT").value("status").toString() == "ok" && w.host->seq() == 1, QStringLiteral("the next request works (the failed one left nothing behind)"), QJsonDocument(lastOf(*raw, "COMMIT_RESULT")).toJson(QJsonDocument::Compact));
    check(!QFileInfo::exists(w.hostFolder + "/content/docB/text.md.qbstage") && !QFileInfo::exists(w.hostFolder + "/settings/journal.json"), QStringLiteral("no temporary file or journal is left"));
    // a message of a type the host does not take
    raw->send(make("WELCOME", QString::fromLatin1(kUuid), QJsonObject{{"proto", 1}}));
    raw->send(make("SNAPSHOT_OP", QString::fromLatin1(kUuid)));
    raw->send(make("VOTE", QString::fromLatin1(kUuid)));
    settle();
    check(raw->codes.contains("NOT_HOST") && raw->codes.count("BAD_OP") >= 2, QStringLiteral("a host's message sent to a host: NOT_HOST; types of other packages without a handler: BAD_OP"));
    int handled = 0;
    w.host->setExtensionHandler([&](const QString &, net::Link *, const QJsonObject &m) {
        if (m.value("t").toString() == "VOTE") {
            ++handled;
            return true;
        }
        return false;
    });
    raw->send(make("VOTE", QString::fromLatin1(kUuid)));
    settle();
    check(handled == 1, QStringLiteral("messages of hosting and host jobs go to the extension handler"));
    // a frame that is no JSON closes the connection
    const int nlinks = w.host->linkCount();
    raw->mine->send(Frame{Frame::Json, "not json"});
    settle();
    check(w.host->linkCount() == nlinks - 1, QStringLiteral("a frame that is no JSON object closes the connection"));
    // unauthorised: a device with no role cannot upload
    auto nobody = rawPeer(w, "devNobody", Role::None, true, false);
    upload(*nobody, "sneaky");
    settle();
    check(!w.host->incoming().has(sha256Hex("sneaky")), QStringLiteral("a device that did not join cannot upload"));
    auto reader = rawPeer(w, "devRead", Role::Reader);
    upload(*reader, "reader's bytes");
    settle();
    check(!w.host->incoming().has(sha256Hex("reader's bytes")) && reader->codes.contains("READ_ONLY"), QStringLiteral("a reader cannot upload"));
    // a device that has no access cannot fetch blobs
    nobody->send(make("BLOB_GET", QString::fromLatin1(kUuid), QJsonObject{{"hash", sha256Hex("Text of docA\n")}, {"offset", 0}}));
    settle();
    check(nobody->codes.contains("BAD_OP") && lastOf(*nobody, "BLOB_PUT").isEmpty(), QStringLiteral("a device that did not join gets no file contents"));
    // a blob nobody has
    reader->send(make("BLOB_GET", QString::fromLatin1(kUuid), QJsonObject{{"hash", QString(64, 'a')}, {"offset", 0}}));
    settle();
    check(reader->codes.contains("BLOB_MISSING"), QStringLiteral("a blob the host does not have: BLOB_MISSING"));
}

// ---------------------------------------------------------------- 10. two programs over TLS

static QByteArray hostText2(const QString &folder, const QString &id) { return readFileBytes(folder + QString("/content/%1/text.md").arg(id)); }

static int runChild(const QString &dir)
{
    using namespace qb::collab::net;
    auto id = DeviceIdentity::createEphemeral("child");
    if (!id)
        return 3;
    writeBytes(dir + "/client.pin", id->pin().toHex());
    auto readWait = [&](const QString &name) {
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < 15000) {
            const QByteArray b = readFileBytes(dir + "/" + name);
            if (!b.isEmpty())
                return b;
            QEventLoop l;
            QTimer::singleShot(20, &l, &QEventLoop::quit);
            l.exec();
        }
        return QByteArray();
    };
    const quint16 port = quint16(readWait("port").toInt());
    const QByteArray hostPin = QByteArray::fromHex(readWait("host.pin"));
    if (!port || hostPin.isEmpty())
        return 4;
    SystemClock clock;
    ClientSession::Config cfg;
    cfg.folder = dir + "/childreplica";
    cfg.uuid = QString::fromLatin1(kUuid);
    cfg.deviceId = id->deviceId();
    cfg.deviceName = "child";
    cfg.clock = &clock;
    ClientSession s(cfg);
    s.open();
    TlsLink *link = TlsLink::connectTo("127.0.0.1", port, id.get(), TrustPolicy::pinned(hostPin));
    bool locked = false, sent = false;
    QObject::connect(link, &Link::stateChanged, &s, [&](Link::State st) {
        if (st == Link::State::Ready)
            s.attach(link);
    });
    QObject::connect(&s, &ClientSession::readyToUse, &s, [&] { s.lock({"doc:docA"}); });
    QObject::connect(&s, &ClientSession::lockGranted, &s, [&](const QString &) {
        if (locked)
            return;
        locked = true;
        QHash<QString, QByteArray> blobs;
        s.submit({textUnit("docA", "written by another program\n", &blobs)}, blobs);
        sent = true;
    });
    QObject::connect(&s, &ClientSession::committed, &s, [&](const QString &, qint64) { writeBytes(dir + "/child.committed", "1"); });
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 30000) {
        QEventLoop l;
        QTimer::singleShot(20, &l, &QEventLoop::quit);
        l.exec();
        if (s.seq() >= 2 && s.state() == ClientSession::State::Ready) {
            writeBytes(dir + "/child.done", treeHash(cfg.folder).toLatin1());
            return sent ? 0 : 5;
        }
    }
    return 2;
}

static void testTls(const QString &tmp)
{
    std::printf("== two programs over TLS\n");
    using namespace qb::collab::net;
    if (!DeviceIdentity::available() || !TlsLink::available()) {
        check(true, QStringLiteral("(this build has no Botan TLS: the two-program run is skipped)"));
        return;
    }
    const QString dir = tmp + "/tls";
    QDir().mkpath(dir);
    const QString hostFolder = makeProject(dir + "/host");
    auto hostId = DeviceIdentity::createEphemeral("host");
    SystemClock clock;
    HostSession::Options o;
    o.folder = hostFolder;
    o.uuid = QString::fromLatin1(kUuid);
    o.hostName = "Host";
    o.clock = &clock;
    HostSession host(o);
    QString err;
    check(host.start(&err), QStringLiteral("the host starts"), err);
    TrustPolicy trust;
    trust.decide = [&](const QByteArray &pin, bool) {
        const QByteArray known = QByteArray::fromHex(readFileBytes(dir + "/client.pin"));
        return !known.isEmpty() && pin == known ? PeerInfo::Trust::Paired : PeerInfo::Trust::Unknown;
    };
    TlsServer server(hostId.get(), trust);
    check(server.listen(QHostAddress::LocalHost, 0), QStringLiteral("the host listens on localhost"));
    QObject::connect(&server, &TlsServer::newLink, &server, [&](TlsLink *l) {
        QObject::connect(l, &Link::stateChanged, l, [&host, l](Link::State st) {
            if (st == Link::State::Ready) {
                host.setRole(l->peer().deviceId, Role::Editor);
                host.addLink(l);
            }
        });
    });
    writeBytes(dir + "/host.pin", hostId->pin().toHex());
    writeBytes(dir + "/port", QByteArray::number(server.port()));
    QProcess child;
    child.setProcessChannelMode(QProcess::ForwardedChannels);
    child.start(QCoreApplication::applicationFilePath(), {"--ls4-child", dir});
    check(child.waitForStarted(5000), QStringLiteral("the second program starts"));
    check(waitFor([&] { return QFileInfo::exists(dir + "/child.committed"); }, 25000) && host.seq() == 1 && hostText2(hostFolder, "docA") == "written by another program\n",
          QStringLiteral("the other program joins over TLS 1.3, takes a lock and commits a change: the host's file is that text"));
    // a second device, in this program, changes another document: the other program gets the event
    const QByteArray pinB = QCryptographicHash::hash("devB", QCryptographicHash::Sha256);
    const QByteArray hostPinMem = QCryptographicHash::hash("host", QCryptographicHash::Sha256);
    MemLink::Pair pr = MemLink::createPair(hostPinMem, pinB, &host);
    host.setRole(MemLink::deviceIdOfPin(pinB), Role::Editor);
    host.addLink(pr.a);
    ClientSession::Config cb;
    cb.folder = dir + "/b";
    cb.uuid = QString::fromLatin1(kUuid);
    cb.deviceId = MemLink::deviceIdOfPin(pinB);
    cb.deviceName = "devB";
    cb.clock = &clock;
    ClientSession b(cb);
    b.open();
    b.attach(pr.b);
    check(waitFor([&] { return b.state() == ClientSession::State::Ready && b.seq() == 1; }, 10000), QStringLiteral("a second device (in memory) joins the same host and gets the first commit"));
    QHash<QString, QByteArray> blobs;
    b.submit({textUnit("docB", "written by the second device\n", &blobs)}, blobs);
    check(waitFor([&] { return QFileInfo::exists(dir + "/child.done"); }, 25000), QStringLiteral("the other program receives the second commit by event"));
    check((child.state() == QProcess::NotRunning || child.waitForFinished(15000)) && child.exitCode() == 0, QStringLiteral("the other program ends well"), QString::number(child.exitCode()));
    check(readFileBytes(dir + "/child.done") == treeHash(hostFolder).toLatin1() && host.seq() == 2, QStringLiteral("its replica equals the host's folder, file for file"));
    check(readFileBytes(dir + "/childreplica/content/docB/text.md") == "written by the second device\n", QStringLiteral("the second device's text reached the other program's replica"));
    if (child.state() != QProcess::NotRunning)
        child.kill();
}

// ---------------------------------------------------------------- main

int main(int argc, char **argv)
{
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication app(argc, argv);
    if (argc >= 3 && QString::fromLocal8Bit(argv[1]) == QLatin1String("--ls4-child"))
        return runChild(QString::fromLocal8Bit(argv[2]));
    QTemporaryDir tmp;
    testMessages();
    testLocks();
    testManifest(tmp.path());
    testBlobs(tmp.path());
    testOutbox(tmp.path());
    testJoinAndCommit(tmp.path());
    testIdempotency();
    testGapsAndReplay();
    testLockRules();
    testLeases();
    testEditRequests();
    testLockHandOverRelease();
    testRestartAndReacquire();
    testSecurity(tmp.path());
    testTls(tmp.path());
    std::printf("\n%d checks\n%d failure(s)\n", total, failures);
    return failures;
}
