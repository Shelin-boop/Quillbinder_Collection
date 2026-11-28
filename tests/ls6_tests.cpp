// Live Sync, work package 6 (0.99.0): who hosts a shared project. Membership records and proofs signed with the device key, the
// chain of authority, the planned handover (old host ACTIVE -> DRAINING -> PREPARED -> RETIRED, target FOLLOWER -> PREPARED ->
// ACTIVE) killed at every step, failover after about 30 seconds (two devices alone, three or more only with a majority), the
// refusal of unknown devices, reunion with recovery drafts per document and author, and the handback to the highest-priority
// device after 60 seconds. The devices are in one process; the network is a fake that can be cut, and the clock is moved by hand.
//
//   QB_SAMPLE=0 QB_SUITES=ls6 tools/run-all-tests.sh build/test-results-x11 x11      (or ./qb_ls6_tests /tmp/shots)
// Exit code = number of failed checks.
#include "core/collab/Authority.h"
#include "core/collab/Clock.h"
#include "core/collab/Drafts.h"
#include "core/collab/Membership.h"
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/proto/Messages.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QMap>
#include <QSet>
#include <QTemporaryDir>
#include <cstdio>
#include <memory>

using namespace qb;
using namespace qb::collab;
using qb::collab::net::DeviceIdentity;

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

static QByteArray readFile(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
static void writeFileTo(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
}

// ---------------------------------------------------------------- the fake device

static QString hashOf(qint64 seq, const QString &tag) { return QStringLiteral("h%1%2").arg(seq).arg(tag); }

struct FakeBackend : HostingBackend {
    qint64 s = 0;
    QString tag = QStringLiteral("a");
    bool ready = true;
    bool hosting = false;
    qint64 hostedEpoch = 0;
    int startCount = 0, stopCount = 0, backups = 0, outboxDrafts = 0;
    QString following;
    bool acceptLocks = true, acceptCommits = true;
    bool drainedNow = true;
    QJsonArray locks;
    CommitLedger *ledger = nullptr;
    QString myId;
    bool failBackup = false;
    bool busy_ = false;
    bool busy() const override { return busy_; }

    qint64 seq() const override { return s; }
    QString stateHash() const override { return hashOf(s, tag); }
    bool replicaReady() const override { return ready; }
    bool startHosting(qint64 epoch, const QJsonArray &l, QString *) override
    {
        hosting = true;
        hostedEpoch = epoch;
        ++startCount;
        locks = l;
        acceptLocks = acceptCommits = true;
        return true;
    }
    void stopHosting() override
    {
        hosting = false;
        ++stopCount;
    }
    void followHost(const QString &host, qint64) override { following = host; }
    void setAcceptingLocks(bool on) override { acceptLocks = on; }
    void setAcceptingCommits(bool on) override { acceptCommits = on; }
    bool drained() const override { return drainedNow; }
    QJsonArray lockTable() const override { return locks; }
    bool backupBeforeRejoin(QString *e) override
    {
        ++backups;
        if (failBackup && e)
            *e = QStringLiteral("no space");
        return !failBackup;
    }
    QList<AuthoredCommit> authoredCommits(qint64 afterSeq, qint64 beforeEpoch) override
    {
        return ledger ? ledger->after(afterSeq, beforeEpoch) : QList<AuthoredCommit>();
    }
    int outboxToDrafts(const QString &) override { return outboxDrafts; }
    void reunionDone(qint64 afterSeq, qint64 newEpoch) override
    {
        if (ledger)
            ledger->forget(afterSeq, newEpoch);
    }
    // a commit of this device that the host (itself or another) accepted: seq + 1 in the current history
    void commit(const QString &newTag, qint64 epoch, const QStringList &paths, const QJsonArray &ops = QJsonArray())
    {
        ++s;
        tag = newTag;
        if (ledger) {
            AuthoredCommit c;
            c.seq = s;
            c.epoch = epoch;
            c.paths = paths;
            c.ops = ops;
            ledger->add(c);
        }
    }
};

struct Net;

struct Node {
    Net *net = nullptr;
    QString name;
    std::unique_ptr<DeviceIdentity> id;
    QString dir;
    std::unique_ptr<FakeBackend> be;
    std::unique_ptr<CommitLedger> ledger;
    std::unique_ptr<Authority> auth;
    bool up = true;
    QStringList unknown, claims, warnings;
    int drafts = 0;
    QList<QString> hostChanges;
    QString title = QStringLiteral("Test Project");
    std::function<bool(const QString &)> kill;

    QString deviceId() const { return id->deviceId(); }
    void make();
};

struct Msg {
    QString from, to;
    QJsonObject m;
};

struct Net {
    ManualClock clock;
    QMap<QString, Node *> nodes; // by device id
    QSet<QString> cut;           // "a|b" (both orders) = no connection
    QList<Msg> queue;
    QList<Msg> log;
    QSet<QString> dropTypes;     // message types that vanish
    QMap<QString, Node *> byName;
    bool violated = false;
    QString violation;
    bool strict = false; // no partition: two devices hosting at one moment is a violation, whatever their epochs

    static QString key(const QString &a, const QString &b) { return a + QLatin1Char('|') + b; }
    bool connected(const QString &a, const QString &b) const
    {
        return nodes.contains(a) && nodes.contains(b) && nodes.value(a)->up && nodes.value(b)->up && !cut.contains(key(a, b));
    }
    bool send(const QString &from, const QString &to, const QJsonObject &m)
    {
        if (!connected(from, to))
            return false;
        queue.append({from, to, m});
        return true;
    }
    void pump()
    {
        int guard = 0;
        while (!queue.isEmpty() && ++guard < 5000) {
            const Msg msg = queue.takeFirst();
            log.append(msg);
            if (dropTypes.contains(msg.m.value(QStringLiteral("t")).toString()))
                continue;
            if (!connected(msg.from, msg.to))
                continue;
            Node *n = nodes.value(msg.to);
            if (n->auth && !n->auth->isDead())
                n->auth->handleMessage(msg.from, msg.m);
            checkInvariants();
        }
    }
    // the connection between two devices comes up or goes down
    void link(Node *a, Node *b, bool on)
    {
        const bool was = connected(a->deviceId(), b->deviceId());
        if (on)
            cut.remove(key(a->deviceId(), b->deviceId())), cut.remove(key(b->deviceId(), a->deviceId()));
        else
            cut.insert(key(a->deviceId(), b->deviceId())), cut.insert(key(b->deviceId(), a->deviceId()));
        const bool now = connected(a->deviceId(), b->deviceId());
        if (was == now)
            return;
        notify(a, b, now);
    }
    void notify(Node *a, Node *b, bool now)
    {
        if (a->auth && !a->auth->isDead())
            now ? a->auth->peerConnected(b->deviceId()) : a->auth->peerDisconnected(b->deviceId());
        if (b->auth && !b->auth->isDead())
            now ? b->auth->peerConnected(a->deviceId()) : b->auth->peerDisconnected(a->deviceId());
        pump();
    }
    void partition(const QList<QList<Node *>> &groups)
    {
        // devices in different groups are cut, devices in one group are joined
        for (int i = 0; i < groups.size(); ++i)
            for (int j = 0; j < groups.size(); ++j)
                for (Node *a : groups.at(i))
                    for (Node *b : groups.at(j))
                        if (a != b && a->deviceId() < b->deviceId())
                            link(a, b, i == j);
    }
    void joinAll()
    {
        const QList<Node *> all = nodes.values();
        for (Node *a : all)
            for (Node *b : all)
                if (a != b && a->deviceId() < b->deviceId()) {
                    cut.remove(key(a->deviceId(), b->deviceId())), cut.remove(key(b->deviceId(), a->deviceId()));
                    notify(a, b, true);
                }
    }
    // who hosts what, now: an epoch with two hosts is a violation of "never two accepted authorities"
    void checkInvariants()
    {
        QMap<qint64, QStringList> hosts;
        for (Node *n : nodes) {
            if (!n->up || !n->auth || n->auth->isDead())
                continue;
            if (n->auth->isHost() || n->auth->state() == AuthState::Retired)
                if (n->auth->isHost())
                    hosts[n->auth->epoch()] << n->name;
        }
        for (auto it = hosts.constBegin(); it != hosts.constEnd(); ++it)
            if (it.value().size() > 1 && !violated) {
                violated = true;
                violation = QStringLiteral("two hosts in epoch %1: %2").arg(it.key()).arg(it.value().join(QLatin1Char(',')));
            }
        if (strict && !violated) {
            QStringList hosting;
            for (Node *n : nodes)
                if (n->up && n->be && n->be->hosting)
                    hosting << n->name;
            if (hosting.size() > 1) {
                violated = true;
                violation = QStringLiteral("hosting at one moment: %1").arg(hosting.join(QLatin1Char(',')));
            }
        }
        // one proof per epoch across all chains
        QMap<qint64, QByteArray> proofs;
        for (Node *n : nodes) {
            if (!n->auth)
                continue;
            for (const Proof &p : n->auth->chain()) {
                const QByteArray b = QJsonDocument(p.toJson()).toJson(QJsonDocument::Compact);
                if (proofs.contains(p.epoch) && proofs.value(p.epoch) != b && !violated) {
                    violated = true;
                    violation = QStringLiteral("two different proofs for epoch %1").arg(p.epoch);
                }
                proofs.insert(p.epoch, b);
            }
        }
    }
    // time passes in steps of one second; heartbeats of every connected pair arrive in each step
    void step(qint64 ms)
    {
        for (qint64 t = 0; t < ms; t += 1000) {
            clock.advance(qMin<qint64>(1000, ms - t), 250);
            for (Node *a : nodes)
                for (Node *b : nodes)
                    if (a != b && connected(a->deviceId(), b->deviceId()) && a->auth && !a->auth->isDead())
                        a->auth->heardFrom(b->deviceId());
            pump();
            checkInvariants();
        }
    }
    int liveHosts() const
    {
        int n = 0;
        for (Node *x : nodes)
            if (x->up && x->auth && !x->auth->isDead() && x->auth->isHost())
                ++n;
        return n;
    }
    Node *hostNode() const
    {
        for (Node *x : nodes)
            if (x->up && x->auth && !x->auth->isDead() && x->auth->isHost())
                return x;
        return nullptr;
    }
};

void Node::make()
{
    Authority::Config c;
    c.project = QStringLiteral("proj-1");
    c.title = title;
    c.folder = dir;
    c.localRoot = dir + QStringLiteral("/livesync-local");
    c.identity = id.get();
    c.clock = &net->clock;
    c.backend = be.get();
    const QString me = deviceId();
    c.send = [this, me](const QString &to, const QJsonObject &m) { return net->send(me, to, m); };
    auth.reset(new Authority(c));
    if (kill)
        auth->setKillHook(kill);
    QObject::connect(auth.get(), &Authority::unknownDeviceRefused, [this](const QString &d, const QString &) {
        unknown << d;
        warnings << auth->unknownDeviceWarning();
    });
    QObject::connect(auth.get(), &Authority::hostingClaimRefused, [this](const QString &d, const QString &) { claims << d; });
    QObject::connect(auth.get(), &Authority::reunited, [this](int n, const QString &) { drafts += n; });
    QObject::connect(auth.get(), &Authority::hostChanged, [this](const QString &h, qint64 e) { hostChanges << QStringLiteral("%1@%2").arg(h.left(4)).arg(e); });
}

// a group of devices that share one project; the first is the highest priority
struct World {
    QTemporaryDir tmp;
    Net net;
    QList<std::shared_ptr<Node>> list;
    Membership members;

    // names in priority order; mayHost false for the names in `noHost`; the device `founder` hosts at epoch 1
    World(const QStringList &names, const QString &founder, const QStringList &noHost = {}, const QStringList &readers = {})
    {
        for (const QString &n : names) {
            auto node = std::make_shared<Node>();
            node->net = &net;
            node->name = n;
            node->id = DeviceIdentity::createEphemeral(n);
            node->dir = tmp.path() + QLatin1Char('/') + n;
            QDir().mkpath(node->dir);
            node->be.reset(new FakeBackend);
            node->be->myId = node->deviceId();
            node->ledger.reset(new CommitLedger(node->dir + QStringLiteral("/livesync-local/ledger.json")));
            node->be->ledger = node->ledger.get();
            list << node;
            net.nodes.insert(node->deviceId(), node.get());
            net.byName.insert(n, node.get());
        }
        members.project = QStringLiteral("proj-1");
        members.version = 1;
        for (const std::shared_ptr<Node> &n : list) {
            Member m;
            m.id = n->deviceId();
            m.key = n->id->publicKeyDer();
            m.name = n->name;
            m.editor = !readers.contains(n->name);
            m.mayHost = m.editor && !noHost.contains(n->name);
            members.members << m;
        }
        members.signWith(*net.byName.value(founder)->id);
        for (const std::shared_ptr<Node> &n : list) {
            n->make();
            n->auth->load();
        }
        QString err;
        Node *f = net.byName.value(founder);
        if (!f->auth->found(members, 1, &err))
            std::printf("found failed: %s\n", qPrintable(err));
        for (const std::shared_ptr<Node> &n : list) {
            if (n.get() == f)
                continue;
            if (!n->auth->joinWith(f->auth->chain(), members, &err))
                std::printf("join failed: %s\n", qPrintable(err));
            n->auth->start();
        }
        f->auth->start();
        net.joinAll();
    }
    Node *n(const QString &name) const { return net.byName.value(name); }
    // the process of a device ends and starts again: the same files, a new object, a backend that knows only the disk
    void crashAndRestart(Node *x)
    {
        const qint64 s = x->be->s;
        const QString tag = x->be->tag;
        x->up = false;
        for (Node *o : net.nodes)
            if (o != x && o->auth && !o->auth->isDead())
                o->auth->peerDisconnected(x->deviceId());
        x->auth.reset();
        x->be.reset(new FakeBackend);
        x->be->s = s;
        x->be->tag = tag;
        x->be->ledger = x->ledger.get();
        x->kill = nullptr;
        x->make();
        QString err;
        x->auth->load(&err);
        x->auth->start();
        x->up = true;
        for (Node *o : net.nodes)
            if (o != x && net.connected(x->deviceId(), o->deviceId())) {
                x->auth->peerConnected(o->deviceId());
                if (o->auth && !o->auth->isDead())
                    o->auth->peerConnected(x->deviceId());
            }
        net.pump();
    }
};

static bool identityOk() { return DeviceIdentity::available(); }

// ---------------------------------------------------------------- tests

static void testVotesNeeded()
{
    check(votesNeeded(2) == 0 && votesNeeded(1) == 0, "two devices need no votes: the other takes over alone");
    check(votesNeeded(3) == 2 && votesNeeded(4) == 3 && votesNeeded(5) == 3 && votesNeeded(6) == 4,
          "three or more need more than half of all devices (3: 2, 4: 3, 5: 3, 6: 4)");
}

static void testMembership()
{
    auto a = DeviceIdentity::createEphemeral(QStringLiteral("A"));
    auto b = DeviceIdentity::createEphemeral(QStringLiteral("B"));
    auto x = DeviceIdentity::createEphemeral(QStringLiteral("X"));
    auto mk = [](DeviceIdentity &d, const QString &name, bool editor = true, bool mayHost = true) {
        Member m;
        m.id = d.deviceId();
        m.key = d.publicKeyDer();
        m.name = name;
        m.editor = editor;
        m.mayHost = mayHost;
        return m;
    };
    Membership m;
    m.project = QStringLiteral("p");
    m.version = 1;
    m.members = {mk(*a, QStringLiteral("A")), mk(*b, QStringLiteral("B"), false, false)};
    m.signWith(*a);
    QString why;
    check(m.signatureValid(&why), "a signed membership is valid", why);
    Membership back;
    check(Membership::fromJson(m.toJson(), &back, &why) && back.signatureValid() && back.count() == 2, "it survives JSON", why);
    check(back.priorityIndex(a->deviceId()) == 0 && back.priorityIndex(b->deviceId()) == 1 && back.priorityIndex(x->deviceId()) == -1, "priority is the order");
    check(back.mayHost(a->deviceId()) && !back.mayHost(b->deviceId()) && !back.mayHost(x->deviceId()), "a reader and a stranger may not host");
    check(back.hostCandidates() == QStringList{a->deviceId()}, "the candidates are the devices that may host");
    check(Membership::defaultMayHost(true, false) && !Membership::defaultMayHost(true, true) && !Membership::defaultMayHost(false, false),
          "desktops host by default, Android and readers only when allowed");
    Membership t = back;
    t.members[1].name = QStringLiteral("Changed");
    check(!t.signatureValid(), "a changed record fails its signature");
    t = back;
    t.signer = x->deviceId();
    check(!t.signatureValid(), "a signer that is no member is refused");
    Membership wrongId = m;
    wrongId.members[0].id = QStringLiteral("00000000000000000000000000000000");
    check(!Membership::fromJson(wrongId.toJson(), &back, &why), "an id that does not fit its key is refused", why);
    Membership next = m;
    next.version = 2;
    next.members.append(mk(*x, QStringLiteral("X")));
    next.signWith(*a);
    check(m.acceptsSuccessor(next, a->deviceId(), &why), "a newer record signed by the host replaces the old one", why);
    check(!m.acceptsSuccessor(next, b->deviceId(), &why), "... but not when the host is somebody else");
    Membership same = next;
    same.version = 1;
    same.signWith(*a);
    check(!m.acceptsSuccessor(same, QString(), &why), "the same version does not replace it");
    Membership byStranger = next;
    byStranger.signWith(*x);
    check(!m.acceptsSuccessor(byStranger, QString(), &why), "a record signed by a device that is no member is refused");
    Membership rekey = next;
    rekey.members[1].key = x->publicKeyDer();
    rekey.members[1].id = x->deviceId();
    rekey.signWith(*a);
    check(rekey.signatureValid() && m.acceptsSuccessor(rekey, QString(), &why), "another member under another key is a new member");
    Membership swapped = next;
    swapped.members[0].key = b->publicKeyDer(); // the id no longer fits: refused when read
    check(!Membership::fromJson(swapped.toJson(), &back, &why), "a changed key behind an id is refused when read");
}

static void testProofsAndChains()
{
    World w({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}, QStringLiteral("A"));
    Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C"));
    const QString P = QStringLiteral("proj-1");
    const Proof f = A->auth->chain().first();
    check(verifyChain({f}, w.members).ok, "a founding proof verifies");
    Proof bad = f;
    bad.epoch = 7;
    check(!verifyChain({bad}, w.members).ok, "a changed founding fails");
    // handover A -> B
    const Proof h = makeHandover(P, 1, 2, B->deviceId(), 5, QStringLiteral("hx"), locksDigestOf(QJsonArray()), *A->id);
    ChainResult r = verifyChain({f, h}, w.members);
    check(r.ok && r.epoch == 2 && r.host == B->deviceId() && r.baseSeq == 5, "a handover signed by the host moves hosting", r.why);
    const Proof hc = makeHandover(P, 1, 2, B->deviceId(), 5, QStringLiteral("hx"), locksDigestOf(QJsonArray()), *C->id);
    check(!verifyChain({f, hc}, w.members).ok, "a handover signed by a device that is not the host fails");
    Proof tampered = h;
    tampered.baseSeq = 6;
    check(!verifyChain({f, tampered}, w.members).ok, "a handover with a changed state fails its signature");
    check(!verifyChain({h}, w.members).ok, "a chain must begin with the founding");
    const Proof skip = makeHandover(P, 1, 3, B->deviceId(), 5, QStringLiteral("hx"), QString(), *A->id);
    check(!verifyChain({f, skip}, w.members).ok, "a handover must open the next epoch");
    // failover B with votes of B and C (3 devices: 2 needed)
    auto vote = [&](Node *voter, const QString &cand, qint64 e, qint64 s, const QString &hash) {
        return Vote{voter->deviceId(), voter->id->sign(voteBytes(P, e, cand, s, hash, voter->deviceId()))};
    };
    const QString bid = B->deviceId();
    const Proof fo = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {vote(B, bid, 2, 4, QStringLiteral("hh")), vote(C, bid, 2, 4, QStringLiteral("hh"))}, *B->id);
    r = verifyChain({f, fo}, w.members);
    check(r.ok && r.host == bid && r.epoch == 2, "a failover with a majority of votes verifies", r.why);
    const Proof alone = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {}, *B->id);
    check(!verifyChain({f, alone}, w.members).ok, "three devices: a failover without votes fails");
    const Proof one = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {vote(B, bid, 2, 4, QStringLiteral("hh"))}, *B->id);
    check(!verifyChain({f, one}, w.members).ok, "three devices: its own vote alone is no majority");
    const Proof dup = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {vote(B, bid, 2, 4, QStringLiteral("hh")), vote(B, bid, 2, 4, QStringLiteral("hh"))}, *B->id);
    check(!verifyChain({f, dup}, w.members).ok, "one voter counted twice is not a majority");
    const Proof fromHost = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {vote(B, bid, 2, 4, QStringLiteral("hh")), vote(A, bid, 2, 4, QStringLiteral("hh"))}, *B->id);
    check(!verifyChain({f, fromHost}, w.members).ok, "the lost host's own vote does not count");
    const Proof otherBase = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {vote(B, bid, 2, 4, QStringLiteral("hh")), vote(C, bid, 2, 3, QStringLiteral("hh"))}, *B->id);
    check(!verifyChain({f, otherBase}, w.members).ok, "a vote for another state is not counted");
    const Proof noSelf = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {vote(C, bid, 2, 4, QStringLiteral("hh")), vote(A, bid, 2, 4, QStringLiteral("hh"))}, *B->id);
    check(!verifyChain({f, noSelf}, w.members).ok, "the candidate's own vote is part of the majority");
    // a proof survives JSON and a hostile one is refused
    Proof back;
    check(Proof::fromJson(fo.toJson(), &back) && back == fo, "a proof survives JSON");
    QJsonObject hostile = fo.toJson();
    hostile.insert(QStringLiteral("votes"), QJsonArray{1, 2});
    check(!Proof::fromJson(hostile, &back), "a proof with wrong votes is refused");
    hostile = fo.toJson();
    hostile.remove(QStringLiteral("sig"));
    check(!Proof::fromJson(hostile, &back), "a proof without a signature is refused");
    // four devices: three votes
    World w4({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D")}, QStringLiteral("A"));
    const QString b4 = w4.n(QStringLiteral("B"))->deviceId();
    auto v4 = [&](const QString &name) {
        Node *n = w4.n(name);
        return Vote{n->deviceId(), n->id->sign(voteBytes(P, 2, b4, 4, QStringLiteral("hh"), n->deviceId()))};
    };
    const Proof f4 = w4.n(QStringLiteral("A"))->auth->chain().first();
    const Proof two = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {v4(QStringLiteral("B")), v4(QStringLiteral("C"))}, *w4.n(QStringLiteral("B"))->id);
    const Proof three = makeFailover(P, 1, 2, 4, QStringLiteral("hh"), {v4(QStringLiteral("B")), v4(QStringLiteral("C")), v4(QStringLiteral("D"))}, *w4.n(QStringLiteral("B"))->id);
    check(!verifyChain({f4, two}, w4.members).ok && verifyChain({f4, three}, w4.members).ok, "four devices: two votes are not enough, three are (2:2 elects nobody)");
    // a reader or a device that is marked not to host cannot be a host in a chain
    World wr({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"), {QStringLiteral("B")});
    const Proof toB = makeHandover(P, 1, 2, wr.n(QStringLiteral("B"))->deviceId(), 1, QStringLiteral("x"), QString(), *wr.n(QStringLiteral("A"))->id);
    check(!verifyChain({wr.n(QStringLiteral("A"))->auth->chain().first(), toB}, wr.members).ok, "a device that may not host cannot receive hosting");
}

static void testRecordAndRestartRules()
{
    World w({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"));
    Node *A = w.n(QStringLiteral("A"));
    check(A->be->hosting && A->auth->isHost() && A->auth->epoch() == 1, "the founder hosts at epoch 1");
    check(QFileInfo::exists(A->dir + QStringLiteral("/livesync/authority.json")) && QFileInfo::exists(A->dir + QStringLiteral("/livesync/membership.json")),
          "authority.json and membership.json are written");
    AuthorityRecord rec = A->auth->record();
    rec.state = AuthState::Draining;
    rec.hoTarget = w.n(QStringLiteral("B"))->deviceId();
    rec.hoEpoch = 2;
    AuthorityRecord back;
    QString why;
    check(AuthorityRecord::fromJson(rec.toJson(), &back, &why) && back.state == AuthState::Draining && back.hoEpoch == 2, "the record survives JSON", why);
    rec.state = AuthState::Retired;
    check(!AuthorityRecord::fromJson(rec.toJson(), &back, &why), "RETIRED without its proof is refused");
    // a damaged file is told, never replaced silently
    writeFileTo(A->dir + QStringLiteral("/livesync/authority.json"), "{ nonsense");
    Authority probe([&] {
        Authority::Config c;
        c.project = QStringLiteral("proj-1");
        c.folder = A->dir;
        c.identity = A->id.get();
        c.clock = &w.net.clock;
        c.backend = A->be.get();
        return c;
    }());
    check(!probe.load(&why) && why.contains(QStringLiteral("damaged")), "a damaged authority.json is reported", why);
    check(readFile(A->dir + QStringLiteral("/livesync/authority.json")) == "{ nonsense", "... and left as it is");
}

// -------------------------------------------------------------- planned handover

struct HandoverResult {
    QString finalHost;
    qint64 finalEpoch = 0;
    bool consistent = false;
    bool violated = false;
    QString violation;
    int targetStarts = 0;
};

// A hosts at epoch 1 and hands over to B; `who` is the device that dies at `point` and starts again at once
static HandoverResult runHandover(const QString &point, const QString &who, qint64 downMs = 0, const QString &dropType = QString())
{
    World w({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}, QStringLiteral("A"));
    Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C"));
    for (Node *n : {A, B, C}) { // everybody holds the same state
        n->be->s = 7;
        n->be->tag = QStringLiteral("a");
    }
    A->be->locks = QJsonArray{QJsonObject{{QStringLiteral("resource"), QStringLiteral("doc:1")}}};
    w.net.strict = downMs == 0; // (a host that was down while the others failed over comes back as the host of its epoch, like a cut-off one)
    Node *victim = who == QLatin1String("A") ? A : B;
    if (!point.isEmpty())
        victim->auth->setKillHook([point](const QString &p) { return p == point; });
    if (!dropType.isEmpty())
        w.net.dropTypes.insert(dropType);
    QString why;
    const bool started = A->auth->transferHosting(B->deviceId(), &why);
    w.net.step(1000);
    if (!started)
        std::printf("      handover did not start: %s\n", qPrintable(why));
    w.net.step(3000);
    if (!point.isEmpty()) {
        if (downMs > 0) { // the victim stays down for a while
            victim->up = false;
            for (Node *o : w.net.nodes)
                if (o != victim && o->auth && !o->auth->isDead())
                    o->auth->peerDisconnected(victim->deviceId());
            w.net.step(downMs);
            victim->up = true;
        }
        w.crashAndRestart(victim);
    }
    if (!dropType.isEmpty()) {
        w.net.step(10000);
        w.net.dropTypes.clear();
    }
    w.net.step(150000);
    HandoverResult res;
    res.violated = w.net.violated;
    res.violation = w.net.violation;
    Node *h = w.net.hostNode();
    res.finalHost = h ? h->name : QString();
    res.finalEpoch = h ? h->auth->epoch() : 0;
    res.targetStarts = B->be->startCount;
    // every device agrees on who hosts, and the hosting device's backend is hosting exactly once at the end
    res.consistent = h && w.net.liveHosts() == 1;
    for (Node *n : {A, B, C})
        if (!h || n->auth->host() != h->deviceId() || n->auth->epoch() != h->auth->epoch() || (n != h && n->be->hosting))
            res.consistent = false;
    return res;
}

static void testHandoverClean()
{
    World w({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}, QStringLiteral("A"));
    Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C"));
    for (Node *n : {A, B, C})
        n->be->s = 12;
    A->be->locks = QJsonArray{QJsonObject{{QStringLiteral("resource"), QStringLiteral("doc:1")}, {QStringLiteral("device"), QStringLiteral("C")}}};
    QString why;
    check(!A->auth->transferHosting(A->deviceId(), &why), "handing over to itself is refused");
    check(!B->auth->transferHosting(A->deviceId(), &why), "a device that does not host cannot hand over");
    w.net.cut.insert(Net::key(A->deviceId(), B->deviceId()));
    w.net.cut.insert(Net::key(B->deviceId(), A->deviceId()));
    A->auth->peerDisconnected(B->deviceId());
    check(!A->auth->transferHosting(B->deviceId(), &why), "a target that is not reachable is refused");
    w.net.link(A, B, true);
    QStringList states;
    QObject::connect(A->auth.get(), &Authority::stateChanged, [&](AuthState s) { states << authStateName(s); });
    QStringList bStates;
    QObject::connect(B->auth.get(), &Authority::stateChanged, [&](AuthState s) { bStates << authStateName(s); });
    A->be->drainedNow = false;
    check(A->auth->transferHosting(B->deviceId(), &why), "the handover starts", why);
    check(A->auth->state() == AuthState::Draining && !A->be->acceptLocks && A->be->acceptCommits, "DRAINING: no new locks, commits still accepted");
    w.net.step(5000);
    check(A->auth->state() == AuthState::Draining, "it waits for the clients to flush");
    w.net.step(6000);
    check(A->auth->state() == AuthState::Follower && B->auth->isHost(), "after at most 10 s it goes on and completes");
    check(states == QStringList({QStringLiteral("draining"), QStringLiteral("prepared"), QStringLiteral("retired"), QStringLiteral("follower")}),
          "old host: ACTIVE -> DRAINING -> PREPARED -> RETIRED -> client", states.join(QLatin1Char(',')));
    check(bStates.first() == QStringLiteral("prepared") && bStates.contains(QStringLiteral("active")), "target: FOLLOWER -> PREPARED -> ACTIVE", bStates.join(QLatin1Char(',')));
    check(B->be->hosting && B->be->hostedEpoch == 2 && B->be->startCount == 1 && !A->be->hosting && A->be->stopCount == 1, "one start and one stop");
    check(B->be->locks == A->be->locks && !B->be->locks.isEmpty(), "the locks move along");
    check(A->auth->host() == B->deviceId() && C->auth->host() == B->deviceId() && C->auth->epoch() == 2, "the clients learn the new host");
    check(C->be->following == B->deviceId() && A->be->following == B->deviceId(), "they reconnect to it");
    check(!w.net.violated, "never two hosts in one epoch", w.net.violation);
    const AuthorityRecord &ra = A->auth->record();
    check(ra.chain.size() == 2 && ra.chain.last().kind == Proof::Handover && ra.chain.last().signer == A->deviceId(), "the proof is signed by the old host");
    check(verifyChain(C->auth->chain(), w.members).ok, "a client can verify the chain it got");
    check(A->be->acceptCommits == false, "the old host accepts no commit after the handover");
    // abort before the proof
    World w2({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"));
    Node *A2 = w2.n(QStringLiteral("A")), *B2 = w2.n(QStringLiteral("B"));
    A2->be->drainedNow = false;
    check(A2->auth->transferHosting(B2->deviceId()), "second handover starts");
    A2->auth->abortHandover();
    check(A2->auth->state() == AuthState::Active && A2->be->acceptLocks && A2->be->acceptCommits && A2->auth->isHost(), "a handover can be aborted while it drains");
    w2.net.step(5000);
    check(!B2->auth->isHost() && w2.net.liveHosts() == 1, "nothing changed on the target");
    // the target is not at the same state: it refuses
    World w3({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"));
    Node *A3 = w3.n(QStringLiteral("A")), *B3 = w3.n(QStringLiteral("B"));
    A3->be->s = 9;
    A3->be->tag = QStringLiteral("a");
    B3->be->s = 9;
    B3->be->tag = QStringLiteral("other");
    bool finished = false, okFlag = true;
    QObject::connect(A3->auth.get(), &Authority::handoverFinished, [&](bool ok, const QString &) { finished = true; okFlag = ok; });
    check(A3->auth->transferHosting(B3->deviceId()), "a handover to a target with another state starts");
    w3.net.step(8000);
    check(finished && !okFlag && A3->auth->isHost() && !B3->auth->isHost() && A3->be->acceptCommits, "the target compares its state hash and says no: aborted");
    // the target is not ready
    World w4({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"));
    w4.n(QStringLiteral("B"))->be->ready = false;
    w4.n(QStringLiteral("A"))->auth->transferHosting(w4.n(QStringLiteral("B"))->deviceId());
    w4.net.step(8000);
    check(w4.n(QStringLiteral("A"))->auth->isHost() && !w4.n(QStringLiteral("B"))->auth->isHost(), "a target whose copy is not ready is refused");
}

static void testHandoverKills()
{
    const QList<std::pair<QString, QString>> points = {
        {QStringLiteral("draining-persisted"), QStringLiteral("A")}, {QStringLiteral("prepared-persisted"), QStringLiteral("A")},
        {QStringLiteral("prepare-sent"), QStringLiteral("A")},       {QStringLiteral("ready-received"), QStringLiteral("A")},
        {QStringLiteral("retired-persisted"), QStringLiteral("A")},  {QStringLiteral("commit-sent"), QStringLiteral("A")},
        {QStringLiteral("target-prepared-persisted"), QStringLiteral("B")}, {QStringLiteral("ready-sent"), QStringLiteral("B")},
        {QStringLiteral("target-active-persisted"), QStringLiteral("B")},   {QStringLiteral("done-sent"), QStringLiteral("B")}};
    const QSet<QString> movesOn = {QStringLiteral("retired-persisted"), QStringLiteral("commit-sent"), QStringLiteral("ready-sent"),
                                   QStringLiteral("target-active-persisted"), QStringLiteral("done-sent")};
    const HandoverResult clean = runHandover(QString(), QStringLiteral("A"));
    check(clean.consistent && clean.finalHost == QStringLiteral("B") && clean.finalEpoch == 2 && !clean.violated, "without a crash B hosts at epoch 2", clean.violation);
    for (const auto &pt : points) {
        for (qint64 down : {qint64(0), qint64(45000)}) {
            const HandoverResult r = runHandover(pt.first, pt.second, down);
            const QString label = QStringLiteral("%1 killed %2 (down %3 s)").arg(pt.second, pt.first).arg(down / 1000);
            check(!r.violated, label + QStringLiteral(": never two hosts in one epoch, one proof per epoch"), r.violation);
            check(r.consistent && !r.finalHost.isEmpty(), label + QStringLiteral(": one host in the end, all agree"));
            if (down == 0) {
                const QString expect = movesOn.contains(pt.first) ? QStringLiteral("B") : QStringLiteral("A");
                check(r.finalHost == expect && r.finalEpoch == (expect == QLatin1String("B") ? 2 : 1),
                      label + QStringLiteral(": the old state or the new one, as the proof says (host %1, epoch %2)").arg(r.finalHost).arg(r.finalEpoch));
                check(r.targetStarts <= 1, label + QStringLiteral(": the target started hosting at most once"));
            }
        }
    }
    // a lost DONE: the old host resends the same proof, the target answers again and never starts twice
    const HandoverResult lost = runHandover(QString(), QStringLiteral("A"), 0, QStringLiteral("HANDOVER_DONE"));
    check(lost.consistent && lost.finalHost == QStringLiteral("B") && lost.finalEpoch == 2 && lost.targetStarts == 1 && !lost.violated,
          "a lost DONE: the proof is resent, B hosts once, epoch 2", lost.violation);
    // the old host is RETIRED and the target never comes: nobody hosts that epoch again on the old host
    {
        World w({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"));
        Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B"));
        w.net.dropTypes.insert(QStringLiteral("HANDOVER_COMMIT"));
        A->auth->transferHosting(B->deviceId());
        w.net.step(20000);
        check(A->auth->state() == AuthState::Retired && !A->auth->isHost() && !A->be->hosting, "after RETIRED the old host does not host that epoch again");
        w.crashAndRestart(A);
        check(A->auth->state() == AuthState::Retired && !A->be->hosting && A->auth->record().hoHasProof, "... also not after a restart: it still holds the proof");
        w.net.dropTypes.clear();
        w.net.step(10000);
        check(B->auth->isHost() && A->auth->state() == AuthState::Follower && A->auth->host() == B->deviceId(), "the restarted old host resends the proof and becomes a client");
    }
    // a restart in DRAINING and in PREPARED: the old host is the host again (aborted); the target goes back to FOLLOWER
    {
        World w({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"));
        Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B"));
        A->be->drainedNow = false;
        A->auth->transferHosting(B->deviceId());
        w.crashAndRestart(A);
        check(A->auth->state() == AuthState::Active && A->be->hosting && A->auth->isHost(), "a restart in DRAINING: the host again");
    }
}

// -------------------------------------------------------------- failover

static void testFailoverTwoDevices()
{
    World w({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"));
    Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B"));
    for (Node *n : {A, B})
        n->be->s = 20;
    w.net.step(10000);
    check(!B->auth->isHost(), "the host is there: nothing happens");
    w.net.partition({{A}, {B}});
    w.net.step(29000);
    check(!B->auth->isHost(), "the other device waits: 29 s without the host are not enough");
    w.net.step(2500);
    check(B->auth->isHost() && B->auth->epoch() == 2 && B->be->hosting && B->be->hostedEpoch == 2, "after about 30 s the other device takes over alone, epoch 2");
    check(B->auth->chain().last().kind == Proof::Failover && B->auth->chain().last().votes.isEmpty() && B->auth->chain().last().baseSeq == 20,
          "its proof is a failover with the state it starts from");
    check(verifyChain(B->auth->chain(), w.members).ok, "the chain verifies against the membership");
    check(A->auth->isHost() && A->be->hosting, "the old host, cut off, keeps hosting what it has (those edits become drafts later)");
    check(!w.net.violated, "two epochs, never two hosts in one epoch", w.net.violation);
}

static void testFailoverMajority()
{
    const QStringList names = {QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")};
    {
        World w(names, QStringLiteral("A"));
        Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C"));
        for (Node *n : {A, B, C})
            n->be->s = 30;
        w.net.step(5000);
        w.net.partition({{A}, {B, C}}); // one against two
        w.net.step(29000);
        check(!B->auth->isHost() && !C->auth->isHost(), "three devices: not before 30 s");
        w.net.step(3000);
        check(B->auth->isHost() && B->auth->epoch() == 2, "one against two: the two elect the next device by priority (B)");
        const Proof &p = B->auth->chain().last();
        check(p.kind == Proof::Failover && p.votes.size() == 2 && verifyChain(B->auth->chain(), w.members).ok, "its proof holds the two signed votes");
        check(C->auth->host() == B->deviceId() && C->auth->epoch() == 2 && !C->auth->isHost(), "C follows B");
        check(A->auth->isHost() && A->auth->epoch() == 1, "the one device in the minority keeps hosting epoch 1");
        w.net.step(60000);
        check(!w.net.violated, "never two hosts in one epoch while split", w.net.violation);
    }
    {
        World w(names, QStringLiteral("A"));
        Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C"));
        int lost = 0;
        QObject::connect(C->auth.get(), &Authority::electionFinished, [&](bool won, qint64) { lost += won ? 0 : 1; });
        w.net.partition({{A, B}, {C}});
        w.net.step(150000);
        check(!C->auth->isHost() && C->auth->epoch() == 1 && lost >= 2, "two against one: the single device is never elected (rounds fail, they repeat)");
        check(A->auth->isHost() && A->auth->epoch() == 1 && !B->auth->isHost(), "the host and its partner are not disturbed");
    }
    {
        World w({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D")}, QStringLiteral("A"));
        w.net.partition({{w.n(QStringLiteral("A")), w.n(QStringLiteral("B"))}, {w.n(QStringLiteral("C")), w.n(QStringLiteral("D"))}});
        w.net.step(150000);
        check(!w.n(QStringLiteral("C"))->auth->isHost() && !w.n(QStringLiteral("D"))->auth->isHost() && w.n(QStringLiteral("C"))->auth->epoch() == 1,
              "four devices split 2:2: nobody is elected (3 votes needed)");
    }
    {
        // the second device by priority cannot be reached: the third one starts 5 s later
        World w(names, QStringLiteral("A"));
        Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C"));
        w.net.partition({{A}, {B}, {C}});
        w.net.link(B, C, true);
        w.net.link(A, B, false);
        w.net.link(A, C, false);
        w.net.step(60000);
        check(B->auth->isHost() && !C->auth->isHost(), "the candidate with the best priority wins; the next one does not start a second election");
        check(C->auth->epoch() == 2 && C->auth->host() == B->deviceId(), "the other device follows");
    }
    {
        // a device that is not allowed to host does not take over, however long the host is away
        World w({QStringLiteral("A"), QStringLiteral("B")}, QStringLiteral("A"), {QStringLiteral("B")});
        w.net.partition({{w.n(QStringLiteral("A"))}, {w.n(QStringLiteral("B"))}});
        w.net.step(100000);
        check(!w.n(QStringLiteral("B"))->auth->isHost(), "a device that may not host (Android not marked, reader) does not take over");
    }
    {
        // a vote is never given twice for one epoch, also not after a crash between the vote and the answer
        World w({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D")}, QStringLiteral("A"));
        Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C")), *D = w.n(QStringLiteral("D"));
        w.net.partition({{A}, {B, C, D}});
        w.net.step(31000); // B has asked at 30 s
        C->auth->peerDisconnected(B->deviceId());
        w.net.step(1);
        // C voted for B at epoch 2 in the round above; D now asks for the same epoch
        const AuthorityRecord &rc = C->auth->record();
        check(rc.votes.value(2) == B->deviceId() || B->auth->isHost(), "C's vote is on its disk");
        QJsonObject req = proto::make(proto::T::ELECTION_REQUEST, QStringLiteral("proj-1"),
                                      {{QStringLiteral("epoch"), 2}, {QStringLiteral("seq"), 0}, {QStringLiteral("stateHash"), hashOf(0, QStringLiteral("a"))},
                                       {QStringLiteral("candidate"), D->deviceId()}});
        w.net.cut.remove(Net::key(C->deviceId(), D->deviceId()));
        C->auth->handleMessage(D->deviceId(), req);
        QString answer;
        for (const Msg &m : w.net.queue)
            if (m.from == C->deviceId() && m.to == D->deviceId())
                answer = m.m.value(QStringLiteral("code")).toString() + m.m.value(QStringLiteral("t")).toString();
        check(answer.contains(QStringLiteral("ERROR")) && (answer.contains(QStringLiteral("ALREADY_VOTED")) || answer.contains(QStringLiteral("HOST_ALIVE")) || answer.contains(QStringLiteral("WRONG_EPOCH"))),
              "a second candidate for the same epoch gets no vote", answer);
    }
}

static void testVoteSurvivesCrash()
{
    World w({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D")}, QStringLiteral("A"));
    Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C")), *D = w.n(QStringLiteral("D"));
    Q_UNUSED(A);
    // C dies right after it wrote its vote for B and before it answered
    w.net.partition({{A}, {B, C, D}});
    C->auth->setKillHook([](const QString &p) { return p == QStringLiteral("vote-persisted"); });
    w.net.step(31000);
    check(C->auth->isDead(), "C died after writing its vote");
    w.crashAndRestart(C);
    check(C->auth->record().votes.value(2) == B->deviceId(), "the vote for B in epoch 2 is still there after the restart");
    QJsonObject req = proto::make(proto::T::ELECTION_REQUEST, QStringLiteral("proj-1"),
                                  {{QStringLiteral("epoch"), 2}, {QStringLiteral("seq"), 0}, {QStringLiteral("stateHash"), hashOf(0, QStringLiteral("a"))},
                                   {QStringLiteral("candidate"), D->deviceId()}});
    w.net.cut.remove(Net::key(C->deviceId(), D->deviceId()));
    w.net.queue.clear();
    w.net.step(31000); // C has lost contact with the host long enough
    w.net.queue.clear();
    C->auth->handleMessage(D->deviceId(), req);
    QString code;
    bool granted = false;
    for (const Msg &m : w.net.queue)
        if (m.from == C->deviceId() && m.to == D->deviceId()) {
            code = m.m.value(QStringLiteral("code")).toString();
            granted = m.m.value(QStringLiteral("granted")).toBool();
        }
    check(!granted && code == QStringLiteral("ALREADY_VOTED"), "D asks for the same epoch after the crash: ALREADY_VOTED", code);
}

// -------------------------------------------------------------- unknown devices and forged proofs

static void testUnknownDevice()
{
    World w({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}, QStringLiteral("A"), {}, {});
    Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C"));
    auto stranger = DeviceIdentity::createEphemeral(QStringLiteral("Stranger"));
    const QString sid = stranger->deviceId();
    const QString P = QStringLiteral("proj-1");
    // the stranger founds a chain of its own and announces hosting
    Membership own;
    own.project = P;
    own.version = 1;
    Member sm;
    sm.id = sid;
    sm.key = stranger->publicKeyDer();
    sm.name = QStringLiteral("S");
    own.members = {sm};
    own.signWith(*stranger);
    const Proof founding = makeFounding(P, 5, *stranger);
    QJsonObject auth = proto::make(proto::T::AUTHORITY, P, {{QStringLiteral("chain"), QJsonArray{founding.toJson()}}, {QStringLiteral("membership"), own.toJson()}});
    const QString epochBefore = QString::number(B->auth->epoch());
    B->auth->handleMessage(sid, auth);
    check(B->unknown.contains(sid) && B->auth->epoch() == 1 && B->auth->host() == A->deviceId(), "an unknown device announcing hosting is refused, nothing changes");
    check(!B->warnings.isEmpty() && B->warnings.first().contains(QStringLiteral("unknown device")) && B->warnings.first().contains(QStringLiteral("Test Project")),
          "and the warning names the project", B->warnings.join(QLatin1Char('|')));
    B->auth->handleMessage(sid, proto::make(proto::T::ELECTION_REQUEST, P, {{QStringLiteral("epoch"), 9}, {QStringLiteral("seq"), 99}, {QStringLiteral("stateHash"), QStringLiteral("x")}, {QStringLiteral("candidate"), sid}}));
    B->auth->handleMessage(sid, proto::make(proto::T::HANDOVER_PREPARE, P, {{QStringLiteral("epoch"), 2}, {QStringLiteral("from"), 1}, {QStringLiteral("seq"), 0}, {QStringLiteral("hash"), QStringLiteral("x")},
                                                                           {QStringLiteral("locks"), QJsonArray()}, {QStringLiteral("target"), B->deviceId()}}));
    B->auth->handleMessage(sid, proto::make(proto::T::HANDOVER_COMMIT, P, {{QStringLiteral("proof"), founding.toJson()}, {QStringLiteral("locks"), QJsonArray()}}));
    check(B->unknown.size() == 4 && B->auth->state() == AuthState::Follower && B->auth->epoch() == 1, "an election request, a handover offer and a commit of an unknown device are refused too");
    bool told = false;
    for (const Msg &m : w.net.queue)
        if (m.to == sid && m.m.value(QStringLiteral("code")).toString() == QStringLiteral("UNKNOWN_DEVICE"))
            told = true;
    (void)told; // (the stranger has no connection in this fake; the refusal is the signal)
    // a member forges a failover without the votes
    const Proof forged = makeFailover(P, 1, 2, 0, hashOf(0, QStringLiteral("a")), {}, *C->id);
    QJsonObject forgedMsg = proto::make(proto::T::AUTHORITY, P, {{QStringLiteral("chain"), QJsonArray{A->auth->chain().first().toJson(), forged.toJson()}}});
    B->auth->handleMessage(C->deviceId(), forgedMsg);
    check(B->unknown.contains(C->deviceId()) && B->auth->epoch() == 1 && B->auth->host() == A->deviceId(), "a member's failover without the votes of a majority is refused");
    // a chain with a changed signature
    Proof broken = forged;
    broken.signature[0] = char(broken.signature.at(0) ^ 0x55);
    B->auth->handleMessage(C->deviceId(), proto::make(proto::T::AUTHORITY, P, {{QStringLiteral("chain"), QJsonArray{A->auth->chain().first().toJson(), broken.toJson()}}}));
    check(B->auth->epoch() == 1 && B->unknown.size() == 6, "a proof with a bad signature is refused");
    // a member that may not host
    World r({QStringLiteral("A"), QStringLiteral("R"), QStringLiteral("B")}, QStringLiteral("A"), {}, {QStringLiteral("R")});
    Node *R = r.n(QStringLiteral("R")), *RB = r.n(QStringLiteral("B"));
    RB->auth->handleMessage(R->deviceId(), proto::make(proto::T::ELECTION_REQUEST, P, {{QStringLiteral("epoch"), 2}, {QStringLiteral("seq"), 0}, {QStringLiteral("stateHash"), hashOf(0, QStringLiteral("a"))}, {QStringLiteral("candidate"), R->deviceId()}}));
    check(RB->claims.contains(R->deviceId()) && RB->auth->epoch() == 1, "a reader asking for votes is refused (a member without the right to host)");
    // hostile content does not crash
    B->auth->handleMessage(C->deviceId(), proto::make(proto::T::AUTHORITY, P, {{QStringLiteral("chain"), QJsonArray{1, QStringLiteral("x"), QJsonObject()}}}));
    B->auth->handleMessage(C->deviceId(), proto::make(proto::T::HANDOVER_PREPARE, P, {{QStringLiteral("epoch"), QStringLiteral("two")}}));
    B->auth->handleMessage(C->deviceId(), proto::make(proto::T::VOTE, P, {{QStringLiteral("epoch"), -4}}));
    check(B->auth->epoch() == 1, "hostile messages from a member change nothing");
    // a membership from a device that is not the host
    Membership next = w.members;
    next.version = 2;
    next.signWith(*C->id);
    B->auth->handleMessage(C->deviceId(), proto::make(proto::T::MEMBERSHIP, P, {{QStringLiteral("membership"), next.toJson()}}));
    check(B->auth->membership().version == 1, "a new membership that the host did not sign is not taken");
    Membership good = w.members;
    good.version = 2;
    good.signWith(*A->id);
    B->auth->handleMessage(A->deviceId(), proto::make(proto::T::MEMBERSHIP, P, {{QStringLiteral("membership"), good.toJson()}}));
    check(B->auth->membership().version == 2, "the host's is");
    // the host writes a membership and tells the others
    QList<Member> ms = A->auth->membership().members;
    QString err;
    check(A->auth->publishMembership(ms, &err) && A->auth->membership().version == 1 + 0 + (A->auth->membership().version - 1), "the host publishes a membership", err);
    check(!B->auth->publishMembership(ms, &err), "a device that does not host cannot");
    (void)epochBefore;
}

// -------------------------------------------------------------- reunion, drafts, handback

static void testReunionAndHandback()
{
    const QStringList names = {QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D"), QStringLiteral("E")};
    World w(names, QStringLiteral("A"));
    Node *A = w.n(QStringLiteral("A")), *B = w.n(QStringLiteral("B")), *C = w.n(QStringLiteral("C")), *D = w.n(QStringLiteral("D")), *E = w.n(QStringLiteral("E"));
    // the project folders hold documents
    for (Node *n : {A, B, C, D, E}) {
        n->be->s = 40;
        writeFileTo(n->dir + QStringLiteral("/content/d1/text.md"), "base one");
        writeFileTo(n->dir + QStringLiteral("/content/d2/text.md"), "base two");
        writeFileTo(n->dir + QStringLiteral("/content/d3/text.md"), "base three");
    }
    w.net.step(5000);
    // the split: A and E on one side (minority), B C D on the other (three of five)
    w.net.partition({{A, E}, {B, C, D}});
    w.net.step(32000);
    check(B->auth->isHost() && B->auth->epoch() == 2, "five devices, two against three: the three elect B");
    check(B->auth->chain().last().votes.size() >= 3, "with at least three signed votes", QString::number(B->auth->chain().last().votes.size()));
    const qint64 fork = B->auth->chain().last().baseSeq;
    check(fork == 40, "the fork is at the state B had", QString::number(fork));
    // during the split the old host accepts edits: its own on d1 and d3, a client's (E) on d2 and d3
    writeFileTo(A->dir + QStringLiteral("/content/d1/text.md"), "A wrote this during the split");
    A->be->commit(QStringLiteral("splitA"), 1, {QStringLiteral("content/d1/text.md")},
                  QJsonArray{QJsonObject{{QStringLiteral("k"), QStringLiteral("item.fields")}, {QStringLiteral("id"), QStringLiteral("d1")},
                                         {QStringLiteral("element"), QStringLiteral("<BinderItem ID=\"d1\"><Title>New Title One</Title></BinderItem>")}}});
    writeFileTo(A->dir + QStringLiteral("/content/d3/text.md"), "A on three");
    A->be->commit(QStringLiteral("splitA"), 1, {QStringLiteral("content/d3/text.md")});
    A->be->commit(QStringLiteral("splitA"), 1, {QStringLiteral("content/d3/text.md")}); // a second commit on d3: still one draft
    writeFileTo(E->dir + QStringLiteral("/content/d2/text.md"), "E wrote this during the split");
    E->be->commit(QStringLiteral("splitA"), 1, {QStringLiteral("content/d2/text.md")});
    writeFileTo(E->dir + QStringLiteral("/content/d3/text.md"), "E on three");
    E->be->commit(QStringLiteral("splitA"), 1, {QStringLiteral("content/d3/text.md")});
    // the majority side goes on
    writeFileTo(B->dir + QStringLiteral("/content/d1/text.md"), "B wrote this");
    B->be->commit(QStringLiteral("newB"), 2, {QStringLiteral("content/d1/text.md")});
    w.net.step(20000);
    check(A->auth->isHost() && A->auth->epoch() == 1 && A->be->hosting, "while split the old host still hosts");
    // reunion: the newer host stays
    w.net.joinAll();
    w.net.step(1000);
    check(!A->auth->isHost() && A->auth->host() == B->deviceId() && A->auth->epoch() == 2, "reunion: the newer host stays, the old one is a client");
    check(!A->be->hosting && A->be->stopCount == 1 && A->be->following == B->deviceId(), "the old host stops hosting at once and follows B");
    check(A->be->backups == 1, "the old host made one backup before rejoining");
    check(E->auth->host() == B->deviceId() && E->auth->epoch() == 2, "its client learns the new host too");
    const QList<Draft> da = A->auth->drafts().list(), de = E->auth->drafts().list();
    QSet<QString> docsA, docsE;
    for (const Draft &d : da)
        docsA << d.document;
    for (const Draft &d : de)
        docsE << d.document;
    check(da.size() == 2 && docsA == QSet<QString>({QStringLiteral("d1"), QStringLiteral("d3")}), "the old host keeps one draft per document it changed (d1, d3)", QString::number(da.size()));
    check(de.size() == 2 && docsE == QSet<QString>({QStringLiteral("d2"), QStringLiteral("d3")}), "its client keeps one draft per document it authored (d2, d3)", QString::number(de.size()));
    bool textOk = true, titleOk = false;
    for (const Draft &d : da) {
        if (d.document == QLatin1String("d1")) {
            textOk = textOk && A->auth->drafts().text(d.id) == "A wrote this during the split";
            titleOk = d.title == QStringLiteral("New Title One") && d.fields.size() == 1;
        }
        if (d.document == QLatin1String("d3"))
            textOk = textOk && A->auth->drafts().text(d.id) == "A on three" && d.reason == QStringLiteral("split") && d.forkSeq == 40 && d.forkEpoch == 1;
    }
    for (const Draft &d : de)
        if (d.document == QLatin1String("d3"))
            textOk = textOk && E->auth->drafts().text(d.id) == "E on three";
    check(textOk, "the drafts hold each author's own text, not a merge");
    check(titleOk, "a draft holds the title and the list of other changes");
    check(A->ledger->after(0, 99).isEmpty() && E->ledger->after(0, 99).isEmpty(), "what became a draft is not turned into one again");
    check(B->auth->drafts().list().isEmpty() && C->auth->drafts().list().isEmpty(), "the devices of the winning side make no drafts");
    // the same step run again (a crash in the middle repeats it) makes no second draft
    const QList<AuthoredCommit> again = {[&] {
        AuthoredCommit c;
        c.seq = 41;
        c.epoch = 1;
        c.paths = {QStringLiteral("content/d1/text.md")};
        return c;
    }()};
    QString err;
    makeSplitDrafts(A->auth->drafts(), A->dir, QStringLiteral("proj-1"), A->deviceId(), again, 1, 40, QStringLiteral("split"), 0, &err);
    check(A->auth->drafts().list().size() == 2, "the same draft is found again by its id");
    check(!w.net.violated, "never two hosts in one epoch", w.net.violation);
    // handback: A has the highest priority and is in sync; after 60 s B hands hosting back
    for (Node *n : {A, B, C, D, E}) { // (A and E made their copies again from B's manifest)
        n->be->s = 50;
        n->be->tag = QStringLiteral("newB");
    }
    B->auth->setPeerInSync(A->deviceId(), true);
    w.net.step(59000);
    check(B->auth->isHost() && B->auth->epoch() == 2, "no handback before the device has been in sync for 60 s");
    w.net.step(4000);
    w.net.step(30000);
    check(A->auth->isHost() && A->auth->epoch() == 3 && !B->auth->isHost() && w.net.liveHosts() == 1, "after 60 s hosting goes back to the highest-priority device by a handover");
    check(A->be->hosting && A->be->hostedEpoch == 3 && C->auth->host() == A->deviceId(), "A hosts again at epoch 3; the others follow");
    check(A->auth->chain().last().kind == Proof::Handover && verifyChain(A->auth->chain(), w.members).ok, "the handback is a signed handover");
    check(!w.net.violated, "never two hosts in one epoch, also not over the handback", w.net.violation);
    // a handback that has no sync does not start
    World w2({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}, QStringLiteral("B"));
    w2.net.step(200000);
    check(w2.n(QStringLiteral("B"))->auth->isHost(), "the top-priority device is not told to be in sync: no handback");
    w2.n(QStringLiteral("B"))->auth->setPeerInSync(w2.n(QStringLiteral("A"))->deviceId(), true);
    w2.net.step(30000);
    w2.n(QStringLiteral("B"))->auth->setPeerInSync(w2.n(QStringLiteral("A"))->deviceId(), false);
    w2.net.step(40000);
    check(w2.n(QStringLiteral("B"))->auth->isHost(), "a device that falls out of sync starts the minute again");
    w2.n(QStringLiteral("B"))->auth->setPeerInSync(w2.n(QStringLiteral("A"))->deviceId(), true);
    w2.n(QStringLiteral("B"))->be->busy_ = true;
    w2.net.step(70000);
    check(w2.n(QStringLiteral("B"))->auth->isHost(), "no handback while a backup or handover runs");
}

static void testDraftsStore(const QString &dir)
{
    RecoveryDrafts store(dir + QStringLiteral("/dr"));
    Draft d;
    d.id = RecoveryDrafts::idFor(QStringLiteral("p"), 1, 5, QStringLiteral("d1"), QStringLiteral("dev"));
    d.project = QStringLiteral("p");
    d.document = QStringLiteral("d1");
    d.title = QStringLiteral("Chapter");
    d.hasText = true;
    d.reason = QStringLiteral("split");
    QString err;
    check(store.add(d, "text bytes", QByteArray(), &err), "a draft is kept", err);
    check(store.list().size() == 1 && store.text(d.id) == "text bytes" && store.exists(d.id), "it is listed and its text is read back");
    check(RecoveryDrafts::idFor(QStringLiteral("p"), 1, 5, QStringLiteral("d1"), QStringLiteral("dev")) == d.id &&
              RecoveryDrafts::idFor(QStringLiteral("p"), 1, 5, QStringLiteral("d2"), QStringLiteral("dev")) != d.id &&
              RecoveryDrafts::idFor(QStringLiteral("p"), 1, 5, QStringLiteral("d1"), QStringLiteral("other")) != d.id,
          "the id comes from project, fork, document and author");
    writeFileTo(dir + QStringLiteral("/dr/recovery/broken/draft.json"), "{ not json");
    check(store.list().size() == 1 && store.damaged() == 1 && QFileInfo::exists(dir + QStringLiteral("/dr/recovery/broken/draft.json")), "a damaged draft is skipped and left on disk");
    check(!store.remove(QStringLiteral("../x")) && !store.remove(QString()), "a draft id with a path in it is refused");
    check(store.remove(d.id) && store.list().isEmpty(), "a draft goes only when asked to");
    // the ledger
    CommitLedger led(dir + QStringLiteral("/led.json"));
    AuthoredCommit c1;
    c1.seq = 5;
    c1.epoch = 1;
    c1.paths = {QStringLiteral("content/x/text.md")};
    AuthoredCommit c2 = c1;
    c2.seq = 9;
    c2.epoch = 2;
    check(led.add(c1) && led.add(c2), "the ledger keeps commits");
    CommitLedger led2(dir + QStringLiteral("/led.json"));
    check(led2.load() && led2.size() == 2 && led2.after(4, 2).size() == 1 && led2.after(9, 9).isEmpty(), "it is read back; the fork and the epoch select");
    check(led2.forget(4, 2) && led2.after(0, 99).size() == 1, "forgotten commits are gone");
    const QList<QJsonObject> log = {QJsonObject{{QStringLiteral("seq"), 3}, {QStringLiteral("actor"), QStringLiteral("me")}, {QStringLiteral("files"), QJsonArray{QJsonObject{{QStringLiteral("path"), QStringLiteral("content/a/text.md")}, {QStringLiteral("kind"), QStringLiteral("put")}}}}},
                                    QJsonObject{{QStringLiteral("seq"), 4}, {QStringLiteral("actor"), QStringLiteral("you")}},
                                    QJsonObject{{QStringLiteral("seq"), 6}, {QStringLiteral("actor"), QStringLiteral("me")}, {QStringLiteral("files"), QJsonArray{QJsonObject{{QStringLiteral("path"), QStringLiteral("content/a/notes.md")}, {QStringLiteral("kind"), QStringLiteral("remove")}}}}}};
    const QList<AuthoredCommit> mine = authoredFromLog(log, QStringLiteral("me"), 2);
    check(mine.size() == 2 && mine.first().paths == QStringList{QStringLiteral("content/a/text.md")} && mine.last().paths.isEmpty() && authoredFromLog(log, QStringLiteral("me"), 3).size() == 1,
          "a host reads its own commits from its log (only puts, only its own, only after the fork)");
}

int main(int argc, char **argv)
{
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication app(argc, argv);
    QTemporaryDir tmp;
    testVotesNeeded();
    testDraftsStore(tmp.path());
    if (!identityOk()) {
        std::printf("SKIP  this build has no Botan TLS: membership, proofs and hosting are not checked\n");
        std::printf("\n%d checks\n%d failure(s)\n", total, failures);
        return failures;
    }
    testMembership();
    testProofsAndChains();
    testRecordAndRestartRules();
    testHandoverClean();
    testHandoverKills();
    testFailoverTwoDevices();
    testFailoverMajority();
    testVoteSurvivesCrash();
    testUnknownDevice();
    testReunionAndHandback();
    std::printf("\n%d checks\n%d failure(s)\n", total, failures);
    return failures;
}
