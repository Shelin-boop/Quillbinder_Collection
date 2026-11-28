#include "core/collab/net/Mdns.h"

#include <QNetworkAddressEntry>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QVariant>

namespace qb {
namespace collab {
namespace mdns {

namespace {

bool validLabel(const QString &l)
{
    if (l.isEmpty() || l.toUtf8().size() > 63)
        return false;
    return true;
}

// Writes a name in plain labels (no compression). False when a label is empty or over 63 bytes or the name over 255.
bool writeName(QByteArray &out, const QString &name)
{
    if (name.isEmpty()) {
        out.append(char(0));
        return true;
    }
    int total = 1;
    const QStringList labels = name.split(QLatin1Char('.'));
    for (const QString &l : labels) {
        if (!validLabel(l))
            return false;
        const QByteArray b = l.toUtf8();
        total += 1 + b.size();
        if (total > kMaxNameBytes)
            return false;
        out.append(char(b.size()));
        out.append(b);
    }
    out.append(char(0));
    return true;
}

void put16(QByteArray &o, quint16 v)
{
    o.append(char(v >> 8));
    o.append(char(v & 0xff));
}
void put32(QByteArray &o, quint32 v)
{
    put16(o, quint16(v >> 16));
    put16(o, quint16(v & 0xffff));
}

struct Reader {
    const uchar *p;
    int n;
    bool u16(int pos, quint16 *v) const
    {
        if (pos < 0 || pos + 2 > n)
            return false;
        *v = quint16((p[pos] << 8) | p[pos + 1]);
        return true;
    }
    bool u32(int pos, quint32 *v) const
    {
        quint16 a, b;
        if (!u16(pos, &a) || !u16(pos + 2, &b))
            return false;
        *v = (quint32(a) << 16) | b;
        return true;
    }
    // A name at `pos`; `next` is the position after it in the record (not after a followed pointer).
    bool name(int pos, QString *out, int *next) const
    {
        QStringList labels;
        int total = 1;
        int jumps = 0;
        int cur = pos;
        int after = -1;
        int lowest = pos;   // a pointer must go to an earlier place than where the name began or than the last pointer
        for (;;) {
            if (cur < 0 || cur >= n)
                return false;
            const int len = p[cur];
            if (len == 0) {
                if (after < 0)
                    after = cur + 1;
                break;
            }
            if ((len & 0xC0) == 0xC0) {
                if (cur + 2 > n)
                    return false;
                const int target = ((len & 0x3F) << 8) | p[cur + 1];
                if (after < 0)
                    after = cur + 2;
                if (++jumps > kMaxPointerJumps || target >= lowest)
                    return false;
                lowest = target;
                cur = target;
                continue;
            }
            if (len & 0xC0)
                return false;   // the reserved label types
            if (cur + 1 + len > n)
                return false;
            total += 1 + len;
            if (total > kMaxNameBytes)
                return false;
            const QByteArray raw(reinterpret_cast<const char *>(p + cur + 1), len);
            for (char c : raw)
                if (uchar(c) < 0x20 || uchar(c) == 0x7f || c == '.')
                    return false;
            labels << QString::fromLatin1(raw).toLower();
            cur += 1 + len;
        }
        *out = labels.join(QLatin1Char('.'));
        *next = after;
        return true;
    }
};

} // namespace

QString serviceName() { return QStringLiteral("_quillbinder._tcp.local"); }

bool validInstanceId(const QString &id)
{
    if (id.size() != 16)
        return false;
    for (QChar c : id)
        if (!((c >= QLatin1Char('0') && c <= QLatin1Char('9')) || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))))
            return false;
    return true;
}

QString instanceName(const QString &id) { return id + QLatin1Char('.') + serviceName(); }
QString hostName(const QString &id) { return id + QStringLiteral(".local"); }

QByteArray build(const Message &m)
{
    QByteArray o;
    put16(o, 0);   // the id is always 0 in multicast DNS
    put16(o, m.response ? 0x8400 : 0x0000);
    put16(o, quint16(m.questions.size()));
    put16(o, quint16(m.records.size()));
    put16(o, 0);
    put16(o, 0);
    for (const Question &q : m.questions) {
        if (!writeName(o, q.name))
            return QByteArray();
        put16(o, q.type);
        put16(o, kClassIn);
    }
    for (const Record &r : m.records) {
        if (!writeName(o, r.name))
            return QByteArray();
        put16(o, r.type);
        put16(o, kClassIn | (r.type == kTypeA || r.type == kTypeSrv || r.type == kTypeTxt ? 0x8000 : 0));   // cache flush
        put32(o, r.ttl);
        QByteArray d;
        switch (r.type) {
        case kTypePtr:
            if (!writeName(d, r.target))
                return QByteArray();
            break;
        case kTypeSrv:
            put16(d, 0);
            put16(d, 0);
            put16(d, r.port);
            if (!writeName(d, r.target))
                return QByteArray();
            break;
        case kTypeTxt:
            if (r.txt.isEmpty())
                d.append(char(0));
            for (const QString &s : r.txt) {
                const QByteArray b = s.toUtf8();
                if (b.size() > 255)
                    return QByteArray();
                d.append(char(b.size()));
                d.append(b);
            }
            break;
        case kTypeA: {
            bool ok = false;
            const quint32 a = r.address.toIPv4Address(&ok);
            if (!ok)
                return QByteArray();
            put32(d, a);
            break;
        }
        default:
            return QByteArray();
        }
        put16(o, quint16(d.size()));
        o.append(d);
    }
    if (o.size() > kMaxPacket)
        return QByteArray();
    return o;
}

bool parse(const QByteArray &packet, Message *out, QString *error)
{
    auto fail = [&](const char *why) {
        if (error)
            *error = QString::fromLatin1(why);
        return false;
    };
    if (packet.size() < 12)
        return fail("too short");
    if (packet.size() > kMaxPacket)
        return fail("too long");
    const Reader r{reinterpret_cast<const uchar *>(packet.constData()), int(packet.size())};
    quint16 flags, qd, an, ns, ar;
    r.u16(2, &flags);
    r.u16(4, &qd);
    r.u16(6, &an);
    r.u16(8, &ns);
    r.u16(10, &ar);
    if (flags & 0x000F)
        return fail("error response");
    if (((flags >> 11) & 0xF) != 0)
        return fail("not a standard query");
    if (int(qd) + an + ns + ar > kMaxRecords)
        return fail("too many records");
    Message m;
    m.response = (flags & 0x8000) != 0;
    int pos = 12;
    for (int i = 0; i < qd; ++i) {
        Question q;
        int next;
        if (!r.name(pos, &q.name, &next))
            return fail("bad name");
        quint16 cls;
        if (!r.u16(next, &q.type) || !r.u16(next + 2, &cls))
            return fail("truncated question");
        pos = next + 4;
        m.questions << q;
    }
    const int records = int(an) + ns + ar;
    for (int i = 0; i < records; ++i) {
        Record rec;
        int next;
        if (!r.name(pos, &rec.name, &next))
            return fail("bad name");
        quint16 cls, len;
        if (!r.u16(next, &rec.type) || !r.u16(next + 2, &cls) || !r.u32(next + 4, &rec.ttl) || !r.u16(next + 8, &len))
            return fail("truncated record");
        const int data = next + 10;
        if (len > kMaxRdata || data + len > packet.size())
            return fail("record data runs over");
        pos = data + len;
        switch (rec.type) {
        case kTypePtr: {
            int e;
            if (!r.name(data, &rec.target, &e) || e > data + len)
                return fail("bad PTR");
            break;
        }
        case kTypeSrv: {
            quint16 prio, weight;
            int e;
            if (len < 7 || !r.u16(data, &prio) || !r.u16(data + 2, &weight) || !r.u16(data + 4, &rec.port)
                || !r.name(data + 6, &rec.target, &e) || e > data + len)
                return fail("bad SRV");
            break;
        }
        case kTypeA: {
            quint32 a;
            if (len != 4 || !r.u32(data, &a))
                return fail("bad A");
            rec.address = QHostAddress(a);
            break;
        }
        case kTypeTxt: {
            int p2 = data;
            while (p2 < data + len) {
                const int l = r.p[p2];
                if (p2 + 1 + l > data + len)
                    return fail("bad TXT");
                if (l > 0)
                    rec.txt << QString::fromUtf8(reinterpret_cast<const char *>(r.p + p2 + 1), l);
                p2 += 1 + l;
            }
            break;
        }
        default:
            continue;   // a type we do not use: skipped (it still counted against the limit)
        }
        m.records << rec;
    }
    if (pos != packet.size())
        return fail("data after the last record");
    *out = m;
    return true;
}

Message announcement(const QString &id, quint16 port, const QList<QHostAddress> &addresses, bool pairingOpen, quint32 ttl)
{
    Message m;
    m.response = true;
    Record ptr;
    ptr.name = serviceName();
    ptr.type = kTypePtr;
    ptr.ttl = ttl;
    ptr.target = instanceName(id);
    m.records << ptr;
    Record srv;
    srv.name = instanceName(id);
    srv.type = kTypeSrv;
    srv.ttl = ttl;
    srv.port = port;
    srv.target = hostName(id);
    m.records << srv;
    Record txt;
    txt.name = instanceName(id);
    txt.type = kTypeTxt;
    txt.ttl = ttl;
    txt.txt << QStringLiteral("v=1");
    if (pairingOpen)
        txt.txt << QStringLiteral("p=1");
    m.records << txt;
    for (const QHostAddress &a : addresses) {
        if (a.protocol() != QAbstractSocket::IPv4Protocol)
            continue;
        Record rec;
        rec.name = hostName(id);
        rec.type = kTypeA;
        rec.ttl = ttl;
        rec.address = a;
        m.records << rec;
    }
    return m;
}

Message browseQuery()
{
    Message m;
    Question q;
    q.name = serviceName();
    q.type = kTypePtr;
    m.questions << q;
    return m;
}

// ------------------------------------------------------------------ UdpTransport

struct UdpTransport::Impl {
    UdpOptions opt;
    QUdpSocket *socket = nullptr;
    QList<QNetworkInterface> interfaces;
    QList<QHostAddress> addresses;
};

UdpTransport::UdpTransport(const UdpOptions &o, QObject *parent) : Transport(parent), d(new Impl)
{
    d->opt = o;
}

UdpTransport::~UdpTransport()
{
    stop();
    delete d;
}

int UdpTransport::interfaceCount() const { return int(d->interfaces.size()); }
QList<QHostAddress> UdpTransport::localAddresses() const { return d->addresses; }

bool UdpTransport::start(QString *error)
{
    stop();
    const auto all = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &ni : all) {
        const auto f = ni.flags();
        if (!(f & QNetworkInterface::IsUp) || !(f & QNetworkInterface::IsRunning) || (f & QNetworkInterface::IsPointToPoint))
            continue;
        const bool loop = f & QNetworkInterface::IsLoopBack;
        if (d->opt.onlyLoopback && !loop)
            continue;
        if (loop ? !(d->opt.includeLoopback || d->opt.onlyLoopback) : !(f & QNetworkInterface::CanMulticast))
            continue;
        QList<QHostAddress> v4;
        for (const QNetworkAddressEntry &e : ni.addressEntries())
            if (e.ip().protocol() == QAbstractSocket::IPv4Protocol)
                v4 << e.ip();
        if (v4.isEmpty())
            continue;
        d->interfaces << ni;
        d->addresses << v4;
    }
    if (d->interfaces.isEmpty()) {
        if (error)
            *error = QStringLiteral("no network interface can do multicast");
        return false;
    }
    d->socket = new QUdpSocket(this);
    if (!d->socket->bind(QHostAddress::AnyIPv4, d->opt.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        if (error)
            *error = d->socket->errorString();
        delete d->socket;
        d->socket = nullptr;
        d->interfaces.clear();
        d->addresses.clear();
        return false;
    }
    d->socket->setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(255));
    d->socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(1));
    int joined = 0;
    for (const QNetworkInterface &ni : d->interfaces)
        if (d->socket->joinMulticastGroup(d->opt.group, ni))
            ++joined;
    if (joined == 0) {
        if (error)
            *error = d->socket->errorString();
        stop();
        return false;
    }
    connect(d->socket, &QUdpSocket::readyRead, this, [this]() {
        while (d->socket && d->socket->hasPendingDatagrams()) {
            QNetworkDatagram dg = d->socket->receiveDatagram(kMaxPacket + 1);
            if (!dg.isValid())
                continue;
            emit datagram(dg.data(), dg.senderAddress());
        }
    });
    return true;
}

void UdpTransport::stop()
{
    if (d->socket) {
        d->socket->close();
        delete d->socket;
        d->socket = nullptr;
    }
    d->interfaces.clear();
    d->addresses.clear();
}

void UdpTransport::send(const QByteArray &packet)
{
    if (!d->socket || packet.isEmpty())
        return;
    for (const QNetworkInterface &ni : d->interfaces) {
        d->socket->setMulticastInterface(ni);
        d->socket->writeDatagram(packet, d->opt.group, d->opt.port);
    }
}

} // namespace mdns
} // namespace collab
} // namespace qb
