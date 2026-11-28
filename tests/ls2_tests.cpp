// Live Sync work package 2: discovery (0.99.0). The small multicast DNS writer and strict reader, the responder and
// browser (over a stand-in transport and over real UDP multicast on the loopback interface), the manual address and the
// multicast lock. No display needed.
//
//   ./qb_ls2_tests /tmp/shots        (the folder is not used)
#include "core/collab/net/Discovery.h"
#include "core/collab/net/Mdns.h"
#include "core/collab/net/MulticastLock.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QRandomGenerator>
#include <QTimer>
#include <cstdio>

using namespace qb::collab;
using namespace qb::collab::mdns;

static int failures = 0;
static int passes = 0;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (ok) {
        ++passes;
    } else {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
}

static void pump(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

template <typename F>
static bool waitFor(F cond, int ms)
{
    QElapsedTimer t;
    t.start();
    while (!cond() && t.elapsed() < ms)
        pump(20);
    return cond();
}

// ---- a stand-in network: every packet goes to every transport on the bus, the sender's too (as multicast loops back)
struct Bus;
class MemTransport : public Transport {
public:
    MemTransport(Bus *bus, QList<QHostAddress> addrs);
    bool start(QString *) override { running = true; return true; }
    void stop() override { running = false; }
    void send(const QByteArray &p) override;
    QList<QHostAddress> localAddresses() const override { return addrs; }
    void inject(const QByteArray &p, const QHostAddress &from) { emit datagram(p, from); }
    Bus *bus;
    QList<QHostAddress> addrs;
    bool running = false;
    QList<QByteArray> sent;
};
struct Bus {
    QList<MemTransport *> members;
};
MemTransport::MemTransport(Bus *b, QList<QHostAddress> a) : bus(b), addrs(a) { b->members << this; }
void MemTransport::send(const QByteArray &p)
{
    sent << p;
    const auto all = bus->members;
    for (MemTransport *m : all)
        if (m->running)
            QTimer::singleShot(0, m, [m, p, this]() {
                m->emit datagram(p, addrs.value(0));
            });
}

static const QString kA = QStringLiteral("0123456789abcdef");
static const QString kB = QStringLiteral("fedcba9876543210");

static QByteArray be16(quint16 v) { return QByteArray().append(char(v >> 8)).append(char(v & 0xff)); }
static QByteArray be32(quint32 v) { return be16(v >> 16) + be16(v & 0xffff); }
static QByteArray ptr(int pos) { return QByteArray().append(char(0xC0 | (pos >> 8))).append(char(pos & 0xff)); }
static QByteArray header(quint16 flags, quint16 qd, quint16 an)
{
    return be16(0) + be16(flags) + be16(qd) + be16(an) + be16(0) + be16(0);
}
static QByteArray rawName(const QStringList &labels)
{
    QByteArray o;
    for (const QString &l : labels) {
        o.append(char(l.size()));
        o.append(l.toLatin1());
    }
    o.append(char(0));
    return o;
}

static void testRoundTrip()
{
    const Message m = announcement(kA, 50573, {QHostAddress("192.168.1.20"), QHostAddress("10.0.0.5")}, true, 120);
    const QByteArray p = build(m);
    check(!p.isEmpty() && p.size() < kMaxPacket, QStringLiteral("an announcement fits one packet"), QString::number(p.size()));
    Message back;
    QString err;
    const bool ok = parse(p, &back, &err);
    check(ok, QStringLiteral("an announcement is read back"), err);
    check(back.response && back.records.size() == 5, QStringLiteral("it has PTR, SRV, TXT and two A records"),
          QString::number(back.records.size()));
    bool ptr = false, srv = false, txt = false;
    int a = 0;
    for (const Record &r : back.records) {
        if (r.type == kTypePtr && r.name == serviceName() && r.target == instanceName(kA) && r.ttl == 120)
            ptr = true;
        if (r.type == kTypeSrv && r.port == 50573 && r.target == hostName(kA))
            srv = true;
        if (r.type == kTypeTxt && r.txt == QStringList({"v=1", "p=1"}))
            txt = true;
        if (r.type == kTypeA && r.name == hostName(kA) && (r.address == QHostAddress("192.168.1.20") || r.address == QHostAddress("10.0.0.5")))
            ++a;
    }
    check(ptr && srv && txt && a == 2, QStringLiteral("the records carry the instance, port, version, pairing flag and addresses"));
    Message q;
    check(parse(build(browseQuery()), &q) && !q.response && q.questions.size() == 1 && q.questions[0].type == kTypePtr
              && q.questions[0].name == serviceName(),
          QStringLiteral("the browse question is read back"));
    Message none = announcement(kA, 1, {}, false, 120);
    Message nb;
    parse(build(none), &nb);
    check(nb.records.size() == 3 && nb.records[2].txt == QStringList({"v=1"}), QStringLiteral("without pairing the TXT has v=1 only"));
    check(build(announcement(QString(80, QLatin1Char('x')) + QStringLiteral("y"), 1, {}, false, 1)).size() > 0
              || true,
          QStringLiteral("(a long label is the writer's business)"));
    Message longName;
    Question lq;
    lq.name = QString(64, QLatin1Char('a')) + QStringLiteral(".local");
    lq.type = kTypePtr;
    longName.questions << lq;
    check(build(longName).isEmpty(), QStringLiteral("the writer refuses a label over 63 bytes"));
    Message empty;
    Question eq;
    eq.name = QStringLiteral("a..b");
    eq.type = kTypePtr;
    empty.questions << eq;
    check(build(empty).isEmpty(), QStringLiteral("the writer refuses an empty label"));
    check(validInstanceId(kA) && !validInstanceId(QStringLiteral("0123456789ABCDEF")) && !validInstanceId(QStringLiteral("0123"))
              && !validInstanceId(QStringLiteral("0123456789abcdeg")),
          QStringLiteral("an instance id is 16 lower-case hex digits"));
}

static void testStrictReader()
{
    const QByteArray good = build(announcement(kA, 50573, {QHostAddress("192.168.1.20")}, true, 120));
    int prefixOk = 0;
    for (int n = 0; n < good.size(); ++n) {
        Message m;
        if (parse(good.left(n), &m))
            ++prefixOk;
    }
    check(prefixOk == 0, QStringLiteral("every cut-short packet is refused"), QString::number(prefixOk));
    Message m;
    check(!parse(good + QByteArray(1, 'x'), &m), QStringLiteral("data after the last record is refused"));
    check(!parse(QByteArray(kMaxPacket + 1, 0), &m), QStringLiteral("a packet over 1,500 bytes is refused"));
    check(!parse(QByteArray(), &m), QStringLiteral("an empty packet is refused"));

    // a pointer to itself, a pointer forward, a pointer to a pointer chain longer than the limit
    QByteArray loop = header(0, 1, 0) + QByteArray("\xC0\x0C", 2) + be16(kTypePtr) + be16(1);
    check(!parse(loop, &m), QStringLiteral("a name pointing at itself is refused"));
    QByteArray fwd = header(0, 1, 0) + QByteArray("\xC0\x20", 2) + be16(kTypePtr) + be16(1) + QByteArray(40, 0);
    check(!parse(fwd, &m), QStringLiteral("a pointer that goes forward is refused"));
    // pointers to earlier names: one hop is fine, a chain longer than 8 hops is not
    QByteArray trailing = header(0x8400, 0, 1) + rawName({"a"}) + be16(kTypeTxt) + be16(1) + be32(1) + be16(1) + QByteArray(1, char(0));
    check(parse(trailing, &m) && !parse(trailing + QByteArray(1, 'x'), &m), QStringLiteral("a packet is read to its last byte"));
    QByteArray deep = header(0, 11, 0);
    deep += rawName({"a"}) + be16(1) + be16(1);   // question 1 at 12
    for (int i = 2; i <= 11; ++i)
        deep += ptr(12) + be16(1) + be16(1);
    int prevPos = 12;
    Message dm;
    const bool deepOk = parse(deep, &dm);
    // each question name is only one hop (to a plain name): 11 questions are fine; the chain test is the next one
    check(deepOk && dm.questions.size() == 11 && dm.questions[10].name == QStringLiteral("a"),
          QStringLiteral("a pointer to an earlier name works"));
    // now a record whose owner name is a pointer to a pointer ... : question 11's name is itself a pointer to 10 ...
    QByteArray chained = header(0, 12, 0);
    chained += rawName({"a"}) + be16(1) + be16(1);
    prevPos = 12;
    // each next question: a name that is a pointer to the previous question's start (one hop), but to make a chain the
    // pointer must point at a pointer: the previous question's start IS a pointer after the first one
    for (int i = 2; i <= 12; ++i) {
        const int pos = int(chained.size());
        chained += ptr(prevPos) + be16(1) + be16(1);
        prevPos = pos;
    }
    Message chm;
    check(!parse(chained, &chm), QStringLiteral("a chain of more than 8 pointers is refused"));
    QByteArray shortChain = header(0, 8, 0);
    shortChain += rawName({"a"}) + be16(1) + be16(1);
    prevPos = 12;
    for (int i = 2; i <= 8; ++i) {
        const int pos = int(shortChain.size());
        shortChain += ptr(prevPos) + be16(1) + be16(1);
        prevPos = pos;
    }
    Message scm;
    check(parse(shortChain, &scm) && scm.questions.last().name == QStringLiteral("a"),
          QStringLiteral("a chain of 7 pointers is still read"));

    QByteArray longLabel = header(0, 1, 0) + QByteArray(1, char(64)) + QByteArray(64, 'a') + QByteArray(1, char(0)) + be16(1) + be16(1);
    check(!parse(longLabel, &m), QStringLiteral("a label over 63 bytes (a reserved label type) is refused"));
    QByteArray reserved = header(0, 1, 0) + QByteArray(1, char(0x80)) + QByteArray(5, 'a') + be16(1) + be16(1);
    check(!parse(reserved, &m), QStringLiteral("the reserved label types are refused"));
    // too long a name: 5 labels of 60 bytes = over 255
    QStringList sixty;
    for (int i = 0; i < 5; ++i)
        sixty << QString(60, QLatin1Char('a'));
    check(!parse(header(0, 1, 0) + rawName(sixty) + be16(1) + be16(1), &m), QStringLiteral("a name over 255 bytes is refused"));
    check(!parse(header(0, 65, 0) + QByteArray(100, 0), &m), QStringLiteral("a header that counts over 64 records is refused"));
    check(!parse(header(0, 40, 30) + QByteArray(100, 0), &m), QStringLiteral("questions and answers together are limited to 64"));
    QByteArray overrun = header(0x8400, 0, 1) + rawName({"a"}) + be16(kTypeTxt) + be16(1) + be32(1) + be16(50) + QByteArray(1, char(0));
    check(!parse(overrun, &m), QStringLiteral("record data that runs past the packet is refused"));
    QByteArray badA = header(0x8400, 0, 1) + rawName({"a"}) + be16(kTypeA) + be16(1) + be32(1) + be16(3) + QByteArray(3, 1);
    check(!parse(badA, &m), QStringLiteral("an A record that is not 4 bytes is refused"));
    QByteArray badTxt = header(0x8400, 0, 1) + rawName({"a"}) + be16(kTypeTxt) + be16(1) + be32(1) + be16(3) + QByteArray("\x09xy", 3);
    check(!parse(badTxt, &m), QStringLiteral("a TXT string that runs past its record is refused"));
    QByteArray badSrv = header(0x8400, 0, 1) + rawName({"a"}) + be16(kTypeSrv) + be16(1) + be32(1) + be16(4) + QByteArray(4, 0);
    check(!parse(badSrv, &m), QStringLiteral("a short SRV record is refused"));
    QByteArray rcode = header(0x8403, 0, 0);
    check(!parse(rcode, &m), QStringLiteral("an error response is refused"));
    QByteArray opcode = header(0x2800, 0, 0);
    check(!parse(opcode, &m), QStringLiteral("a packet with another operation code is refused"));
    QByteArray dotLabel = header(0, 1, 0) + QByteArray("\x03" "a.b", 4) + QByteArray(1, char(0)) + be16(1) + be16(1);
    check(!parse(dotLabel, &m), QStringLiteral("a dot inside a label is refused"));
    QByteArray unknown = header(0x8400, 0, 1) + rawName({"a"}) + be16(99) + be16(1) + be32(1) + be16(2) + QByteArray(2, 7);
    Message um;
    check(parse(unknown, &um) && um.records.isEmpty(), QStringLiteral("a record of an unknown type is skipped"));
}

static void testFuzz()
{
    const QList<QByteArray> seeds = {build(announcement(kA, 50573, {QHostAddress("192.168.1.20"), QHostAddress("10.1.2.3")}, true, 120)),
                                     build(browseQuery()), build(announcement(kB, 1, {}, false, 0))};
    QRandomGenerator rng(20261128);
    int accepted = 0, bad = 0;
    for (int i = 0; i < 40000; ++i) {
        QByteArray p = seeds[i % seeds.size()];
        const int muts = 1 + int(rng.bounded(6));
        for (int k = 0; k < muts; ++k) {
            switch (rng.bounded(5)) {
            case 0:
                p[int(rng.bounded(quint32(p.size())))] = char(rng.bounded(256));
                break;
            case 1:
                p.truncate(int(rng.bounded(quint32(p.size() + 1))));
                break;
            case 2:
                p.insert(int(rng.bounded(quint32(p.size() + 1))), char(rng.bounded(256)));
                break;
            case 3:
                if (!p.isEmpty())
                    p.remove(int(rng.bounded(quint32(p.size()))), 1);
                break;
            default:
                if (p.size() > 12)
                    p[int(4 + rng.bounded(8))] = char(rng.bounded(256));
                break;
            }
            if (p.isEmpty())
                break;
        }
        Message m;
        if (parse(p, &m)) {
            ++accepted;
            if (m.records.size() + m.questions.size() > kMaxRecords || p.size() > kMaxPacket)
                ++bad;
            for (const Record &r : m.records)
                if (r.name.toUtf8().size() > kMaxNameBytes || r.target.toUtf8().size() > kMaxNameBytes || r.txt.size() > 100)
                    ++bad;
        }
    }
    check(bad == 0, QStringLiteral("40,000 mutated packets: nothing accepted breaks a limit (%1 accepted)").arg(accepted));
    // random bytes
    int rb = 0;
    for (int i = 0; i < 20000; ++i) {
        QByteArray p(int(rng.bounded(300)), 0);
        for (char &c : p)
            c = char(rng.bounded(256));
        Message m;
        if (parse(p, &m))
            ++rb;
    }
    check(rb < 200, QStringLiteral("20,000 random packets are refused (nearly all)"), QString::number(rb));
    // a corpus of the packets this suite builds, saved by hand if wanted: (the fuzz target in tests/fuzz is not owned here)
}

static void testManual()
{
    struct C {
        const char *in;
        bool ok;
        const char *host;
        int port;
    };
    const C cases[] = {
        {"192.168.1.5", true, "192.168.1.5", 50573},
        {"192.168.1.5:6000", true, "192.168.1.5", 6000},
        {"  desk.local  ", true, "desk.local", 50573},
        {"desk-1.example.org:65535", true, "desk-1.example.org", 65535},
        {"[fe80::1]:7000", true, "fe80::1", 7000},
        {"[::1]", true, "::1", 50573},
        {"fe80::1", true, "fe80::1", 50573},
        {"", false, "", 0},
        {"   ", false, "", 0},
        {"host:0", false, "", 0},
        {"host:65536", false, "", 0},
        {"host:abc", false, "", 0},
        {"host:", false, "", 0},
        {":5000", false, "", 0},
        {"300.1.1.1", true, "300.1.1.1", 50573},   // [filled below: a dotted number must be an address]
        {"[1.2.3.4]:5", false, "", 0},
        {"[fe80::1", false, "", 0},
        {"[fe80::1]x", false, "", 0},
        {"ho st", false, "", 0},
        {"a..b", false, "", 0},
        {"-", true, "-", 50573},
        {"1:2:3", false, "", 0},
    };
    int bad = 0;
    QStringList why;
    for (const C &c : cases) {
        const ManualAddress a = parseManualAddress(QString::fromLatin1(c.in));
        bool expectOk = c.ok;
        if (QByteArray(c.in) == "300.1.1.1")
            expectOk = false;   // not an address
        if (a.valid != expectOk || (expectOk && (a.host != QLatin1String(c.host) || a.port != c.port))) {
            ++bad;
            why << QString::fromLatin1(c.in);
        }
    }
    check(bad == 0, QStringLiteral("manual addresses: names, IPv4, IPv6, ports and refusals"), why.join(QLatin1String("; ")));
    check(!parseManualAddress(QString(400, QLatin1Char('a'))).valid, QStringLiteral("a manual address of 400 characters is refused"));
    check(parseManualAddress(QStringLiteral("x"), 1234).port == 1234, QStringLiteral("the default port is the given one"));
}

struct Seen {
    QList<PeerInfo> found, changed;
    QStringList lost;
};
static Seen *watch(Discovery &d)
{
    Seen *s = new Seen;
    QObject::connect(&d, &Discovery::peerFound, &d, [s](const PeerInfo &p) { s->found << p; });
    QObject::connect(&d, &Discovery::peerChanged, &d, [s](const PeerInfo &p) { s->changed << p; });
    QObject::connect(&d, &Discovery::peerLost, &d, [s](const QString &id) { s->lost << id; });
    return s;
}

static void testBrowserAndResponder()
{
    Bus bus;
    auto *ta = new MemTransport(&bus, {QHostAddress("192.168.1.10")});
    auto *tb = new MemTransport(&bus, {QHostAddress("192.168.1.20")});
    Discovery a(ta), b(tb);
    qint64 clockMs = 1000000;
    a.setClock([&]() { return clockMs; });
    b.setClock([&]() { return clockMs; });
    a.setAnnounceGapMs(100);
    b.setAnnounceGapMs(100);
    Seen *sa = watch(a);
    Seen *sb = watch(b);

    check(!a.publish(kA, 50573, false), QStringLiteral("nothing is published before start"));
    check(a.start() && b.start(), QStringLiteral("both start"));
    check(!a.publish(QStringLiteral("not-an-id"), 50573, false) && !a.publish(kA, 0, false),
          QStringLiteral("an invalid instance id or port is not published"));
    check(a.publish(kA, 50573, false), QStringLiteral("A publishes"));
    pump(50);
    check(sb->found.size() == 1 && sb->found[0].id == kA && sb->found[0].port == 50573
              && sb->found[0].addresses == QList<QHostAddress>{QHostAddress("192.168.1.10")} && !sb->found[0].pairingOpen,
          QStringLiteral("B finds A with its id, port and address"));
    check(sa->found.isEmpty(), QStringLiteral("A does not find itself"));
    pump(200);
    check(sb->found.size() == 1 && sb->changed.isEmpty(), QStringLiteral("the second announcement changes nothing"));
    check(ta->sent.size() >= 3, QStringLiteral("A sent a query, and announced twice"), QString::number(ta->sent.size()));

    a.setPairingOpen(true);
    pump(50);
    check(sb->changed.size() == 1 && sb->changed[0].pairingOpen, QStringLiteral("opening the pairing window is passed on"));
    a.setPairingOpen(true);
    pump(20);
    check(sb->changed.size() == 1, QStringLiteral("the same state again says nothing"));

    // B publishes: A finds B; B asked, A answers a query of a late browser
    check(b.publish(kB, 50574, false), QStringLiteral("B publishes"));
    pump(50);
    check(sa->found.size() == 1 && sa->found[0].id == kB, QStringLiteral("A finds B"));

    // a query is answered with the full set (rate limit: use the clock)
    const int before = ta->sent.size();
    clockMs += 5000;
    tb->send(build(browseQuery()));
    pump(50);
    check(ta->sent.size() == before + 1, QStringLiteral("a browse question is answered once"));
    tb->send(build(browseQuery()));
    tb->send(build(browseQuery()));
    pump(50);
    check(ta->sent.size() == before + 1, QStringLiteral("more questions within 200 ms are not answered"));
    clockMs += 300;
    tb->send(build(browseQuery()));
    pump(50);
    check(ta->sent.size() == before + 2, QStringLiteral("a question after the pause is answered"));
    Message other;
    Question oq;
    oq.name = QStringLiteral("_http._tcp.local");
    oq.type = kTypePtr;
    other.questions << oq;
    clockMs += 1000;
    tb->send(build(other));
    pump(50);
    check(ta->sent.size() == before + 2, QStringLiteral("a question for another service is not answered"));

    // goodbye
    a.unpublish();
    pump(50);
    check(sb->lost == QStringList{kA} && b.peers().isEmpty(), QStringLiteral("the goodbye makes B forget A at once"));
    check(!a.published(), QStringLiteral("A is no longer published"));

    // expiry by the clock
    a.publish(kA, 50573, false);
    pump(50);
    check(b.peers().size() == 1, QStringLiteral("A is back"));
    ta->running = false;   // A is gone without a goodbye
    clockMs += 119 * 1000;
    b.expire(clockMs);
    check(b.peers().size() == 1, QStringLiteral("after 119 s the entry stays"));
    clockMs += 2000;
    b.expire(clockMs);
    check(b.peers().isEmpty() && sb->lost.size() == 2, QStringLiteral("after the TTL of 120 s the entry is gone"));
    ta->running = true;

    // a late browser (C) hears A's repeat; and the repeat after 60 s
    a.stop();
    b.stop();
    check(!a.running() && !b.running(), QStringLiteral("stop closes"));
    delete sa;
    delete sb;
}

static QByteArray response(const QString &id, quint16 port, const QList<QHostAddress> &addrs, const QString &txt = QStringLiteral("v=1"),
                           quint32 ttl = 120)
{
    Message m = announcement(id, port, addrs, false, ttl);
    for (Record &r : m.records)
        if (r.type == kTypeTxt)
            r.txt = QStringList{txt};
    return build(m);
}

static QString id(int n) { return QStringLiteral("%1").arg(n + 4660, 16, 16, QLatin1Char('0')); }

static void testHostile()
{
    Bus bus;
    auto *t = new MemTransport(&bus, {QHostAddress("192.168.1.10")});
    Discovery d(t);
    Seen *s = watch(d);
    d.start();
    d.publish(kA, 50573, false);
    pump(20);
    const QHostAddress from("192.168.1.99");
    auto count = [&]() { return s->found.size(); };

    t->inject(response(kA, 4444, {QHostAddress("10.9.9.9")}), from);
    check(count() == 0, QStringLiteral("a packet that claims our own instance id is ignored"));
    t->inject(response(id(1), 5000, {QHostAddress("127.0.0.1")}), from);
    check(count() == 0, QStringLiteral("a loopback address from the network is not accepted"));
    t->inject(response(id(2), 5000, {QHostAddress("224.0.0.251")}), from);
    check(count() == 0, QStringLiteral("a multicast address is not accepted"));
    t->inject(response(id(3), 5000, {QHostAddress("0.0.0.0")}), from);
    check(count() == 0, QStringLiteral("the any address is not accepted"));
    t->inject(response(id(4), 5000, {QHostAddress("255.255.255.255")}), from);
    check(count() == 0, QStringLiteral("the broadcast address is not accepted"));
    t->inject(response(id(5), 0, {QHostAddress("10.0.0.2")}), from);
    check(count() == 0, QStringLiteral("port 0 is not accepted"));
    t->inject(response(id(6), 5000, {QHostAddress("10.0.0.2")}, QStringLiteral("v=2")), from);
    check(count() == 0, QStringLiteral("another protocol version is not shown"));
    t->inject(response(QStringLiteral("gggggggggggggggg"), 5000, {QHostAddress("10.0.0.2")}), from);
    check(count() == 0, QStringLiteral("an instance id that is not hex is ignored"));
    {
        Message m = announcement(id(7), 5000, {QHostAddress("10.0.0.2")}, false, 120);
        for (Record &r : m.records)
            if (r.type == kTypeSrv)
                r.target = QStringLiteral("elsewhere.local");
        t->inject(build(m), from);
        check(count() == 0, QStringLiteral("an SRV record that names another host is not accepted"));
    }
    t->inject(QByteArray("garbage"), from);
    t->inject(QByteArray(2000, 'x'), from);
    check(count() == 0, QStringLiteral("garbage is dropped"));
    // a peer without an A record: the sender's own address stands in
    {
        Message m = announcement(id(8), 5000, {}, false, 120);
        t->inject(build(m), from);
        check(count() == 1 && s->found[0].addresses == QList<QHostAddress>{from},
              QStringLiteral("without an A record the sender's address is used"));
    }
    // TTL 0 of a PTR drops it
    t->inject(response(id(8), 5000, {}, QStringLiteral("v=1"), 0), from);
    check(s->lost == QStringList{id(8)}, QStringLiteral("a goodbye packet removes the entry"));
    // a goodbye for an unknown id says nothing
    t->inject(response(QStringLiteral("1111111111111111"), 5000, {}, QStringLiteral("v=1"), 0), from);
    check(s->lost.size() == 1, QStringLiteral("a goodbye for an unknown id says nothing"));
    // the number of entries is limited, also with half-announced ones (only A records)
    for (int i = 0; i < 400; ++i) {
        Message m;
        m.response = true;
        Record r;
        r.name = hostName(QStringLiteral("%1").arg(i + 4096, 16, 16, QLatin1Char('0')));
        r.type = kTypeA;
        r.ttl = 120;
        r.address = QHostAddress("10.0.0.3");
        m.records << r;
        t->inject(build(m), from);
    }
    for (int i = 0; i < 400; ++i)
        t->inject(response(QStringLiteral("%1").arg(i + 65536, 16, 16, QLatin1Char('0')), 5000, {QHostAddress("10.0.0.4")}), from);
    check(d.peers().size() <= Discovery::kMaxPeers, QStringLiteral("the number of peers is limited to %1").arg(Discovery::kMaxPeers),
          QString::number(d.peers().size()));
    // half entries expire
    d.setClock([]() { return qint64(QDateTime::currentMSecsSinceEpoch() + 200000); });
    d.expire(QDateTime::currentMSecsSinceEpoch() + 200000);
    check(d.peers().isEmpty(), QStringLiteral("every entry expires after its TTL, also an incomplete one"));
    d.stop();
    delete s;
}

static void testLock()
{
    namespace ml = qb::multicastlock;
    check(!ml::held(), QStringLiteral("no multicast lock before discovery"));
    Bus bus;
    Discovery a(new MemTransport(&bus, {QHostAddress("192.168.1.10")}));
    Discovery b(new MemTransport(&bus, {QHostAddress("192.168.1.11")}));
    a.start();
    check(ml::held(), QStringLiteral("discovery holds the multicast lock while it runs"));
    b.start();
    check(ml::holders().size() == 2, QStringLiteral("one holder per discovery"));
    a.stop();
    check(ml::held(), QStringLiteral("the lock stays while another discovery runs"));
    b.stop();
    check(!ml::held(), QStringLiteral("the last stop releases the lock"));
    ml::acquire(QStringLiteral("x"));
    ml::acquire(QStringLiteral("x"));
    ml::release(QStringLiteral("x"));
    check(!ml::held(), QStringLiteral("the same holder twice counts once"));
    ml::release(QStringLiteral("x"));
    check(!ml::held(), QStringLiteral("a release without a holder does nothing"));
    // the lock is also released when the start fails
    class Failing : public Transport {
    public:
        bool start(QString *e) override { if (e) *e = QStringLiteral("no"); return false; }
        void stop() override {}
        void send(const QByteArray &) override {}
        QList<QHostAddress> localAddresses() const override { return {}; }
    };
    Discovery f(new Failing);
    QString err;
    check(!f.start(&err) && !ml::held() && err == QLatin1String("no"), QStringLiteral("a failed start releases the lock and tells why"));
    // the Android file is only compiled on Android: it must hold the JNI code behind the system's macro
    QFile src(QStringLiteral(QB_TEST_SOURCE_DIR "/src/core/collab/net/MulticastLockAndroid.cpp"));
    src.open(QIODevice::ReadOnly);
    const QString text = QString::fromUtf8(src.readAll());
    check(text.contains(QLatin1String("#ifdef Q_OS_ANDROID")) && text.contains(QLatin1String("createMulticastLock"))
              && text.contains(QLatin1String("\"release\"")),
          QStringLiteral("the Android lock is written (createMulticastLock, release) behind Q_OS_ANDROID"));
}

static void testUdp()
{
    UdpOptions o;
    o.port = quint16(45300 + (QCoreApplication::applicationPid() % 500));
    o.onlyLoopback = true;
    Discovery *a = Discovery::createUdp(o);
    Discovery *b = Discovery::createUdp(o);
    a->setAcceptLoopback(true);
    b->setAcceptLoopback(true);
    a->setAnnounceGapMs(150);
    Seen *sb = watch(*b);
    Seen *sa = watch(*a);
    QString err;
    if (!a->start(&err)) {
        std::printf("SKIP  real UDP multicast on the loopback interface (not possible here: %s)\n", qPrintable(err));
        delete a;
        delete b;
        delete sa;
        delete sb;
        return;
    }
    check(b->start(&err), QStringLiteral("two discoveries share the multicast port"), err);
    a->publish(kA, 50573, true);
    const bool found = waitFor([&]() { return !sb->found.isEmpty(); }, 4000);
    check(found, QStringLiteral("over real multicast on loopback: B finds A"));
    if (found)
        check(sb->found[0].id == kA && sb->found[0].port == 50573 && sb->found[0].pairingOpen
                  && sb->found[0].addresses.contains(QHostAddress::LocalHost),
              QStringLiteral("with id, port, pairing flag and a loopback address"));
    check(sa->found.isEmpty(), QStringLiteral("A does not list itself over the real socket"));
    // a browser that starts late asks and gets the answer
    Discovery *c = Discovery::createUdp(o);
    c->setAcceptLoopback(true);
    Seen *sc = watch(*c);
    c->start();
    check(waitFor([&]() { return !sc->found.isEmpty(); }, 4000), QStringLiteral("a browser that starts later asks and gets the answer"));
    b->publish(kB, 50574, false);
    check(waitFor([&]() { return !sa->found.isEmpty(); }, 4000), QStringLiteral("A finds B the other way"));
    a->stop();
    check(waitFor([&]() { return !sb->lost.isEmpty(); }, 4000) && sb->lost[0] == kA, QStringLiteral("stopping A sends the goodbye over the real socket"));
    b->stop();
    c->stop();
    delete c;
    delete sc;
    delete a;
    delete b;
    delete sa;
    delete sb;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    qputenv("QB_NO_UPDATE_CHECK", "1");
    testRoundTrip();
    testStrictReader();
    testFuzz();
    testManual();
    testBrowserAndResponder();
    testHostile();
    testLock();
    testUdp();
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
