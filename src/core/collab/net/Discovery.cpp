#include "core/collab/net/Discovery.h"

#include "core/collab/net/MulticastLock.h"

#include <QDateTime>
#include <QTimer>

namespace qb {
namespace collab {

ManualAddress parseManualAddress(const QString &textIn, quint16 defaultPort)
{
    ManualAddress r;
    const QString text = textIn.trimmed();
    auto bad = [&](const char *why) {
        r.error = QString::fromLatin1(why);
        return r;
    };
    if (text.isEmpty())
        return bad("empty");
    if (text.size() > 253 + 7)
        return bad("too long");
    QString host = text;
    QString portText;
    if (text.startsWith(QLatin1Char('['))) {
        const int close = text.indexOf(QLatin1Char(']'));
        if (close < 0)
            return bad("missing ]");
        host = text.mid(1, close - 1);
        const QString rest = text.mid(close + 1);
        if (!rest.isEmpty()) {
            if (!rest.startsWith(QLatin1Char(':')))
                return bad("text after ]");
            portText = rest.mid(1);
            if (portText.isEmpty())
                return bad("empty port");
        }
        QHostAddress a;
        if (!a.setAddress(host) || a.protocol() != QAbstractSocket::IPv6Protocol)
            return bad("not an IPv6 address");
    } else {
        const int colons = int(text.count(QLatin1Char(':')));
        if (colons == 1) {
            const int c = text.indexOf(QLatin1Char(':'));
            host = text.left(c);
            portText = text.mid(c + 1);
            if (portText.isEmpty())
                return bad("empty port");
        } else if (colons > 1) {
            QHostAddress a;
            if (!a.setAddress(text) || a.protocol() != QAbstractSocket::IPv6Protocol)
                return bad("not an IPv6 address");
        }
        if (host.isEmpty())
            return bad("empty host");
        if (colons <= 1) {
            QHostAddress a;
            const bool dotted = host.contains(QLatin1Char('.')) && host.at(0).isDigit();
            if (dotted && !a.setAddress(host))
                return bad("not an IPv4 address");
            if (!dotted) {
                for (QChar c : host)
                    if (!(c.isLetterOrNumber() || c == QLatin1Char('-') || c == QLatin1Char('.') || c == QLatin1Char('_')))
                        return bad("not a valid name");
                if (host.startsWith(QLatin1Char('.')) || host.endsWith(QLatin1Char('.')) || host.contains(QStringLiteral("..")))
                    return bad("not a valid name");
            }
        }
    }
    quint16 port = defaultPort;
    if (!portText.isEmpty()) {
        bool ok = false;
        const uint p = portText.toUInt(&ok);
        if (!ok || p < 1 || p > 65535)
            return bad("port out of range");
        port = quint16(p);
    }
    if (port == 0)
        return bad("port out of range");
    r.valid = true;
    r.host = host;
    r.port = port;
    return r;
}

Discovery::Discovery(mdns::Transport *transport, QObject *parent) : QObject(parent), m_t(transport)
{
    m_t->setParent(this);
    m_holder = QStringLiteral("discovery-%1").arg(quintptr(this));
    m_clock = []() { return QDateTime::currentMSecsSinceEpoch(); };
    connect(m_t, &mdns::Transport::datagram, this, &Discovery::onDatagram);
    m_queryTimer = new QTimer(this);
    m_queryTimer->setSingleShot(true);
    connect(m_queryTimer, &QTimer::timeout, this, [this]() {
        sendQuery();
        m_queryGap = qMin(m_queryGap * 2, 60000);
        m_queryTimer->start(m_queryGap);
    });
    m_announceTimer = new QTimer(this);
    m_announceTimer->setSingleShot(true);
    connect(m_announceTimer, &QTimer::timeout, this, [this]() {
        if (m_published)
            sendAnnouncement(120);
    });
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(60000);
    connect(m_refreshTimer, &QTimer::timeout, this, [this]() {
        if (m_published)
            sendAnnouncement(120);
    });
    m_expireTimer = new QTimer(this);
    m_expireTimer->setInterval(5000);
    connect(m_expireTimer, &QTimer::timeout, this, [this]() { expire(now()); });
}

Discovery::~Discovery() { stop(); }

Discovery *Discovery::createUdp(const mdns::UdpOptions &options, QObject *parent)
{
    return new Discovery(new mdns::UdpTransport(options), parent);
}

qint64 Discovery::now() const { return m_clock(); }

bool Discovery::start(QString *error)
{
    if (m_running)
        return true;
    multicastlock::acquire(m_holder);
    if (!m_t->start(error)) {
        multicastlock::release(m_holder);
        return false;
    }
    m_running = true;
    sendQuery();
    m_queryGap = 1000;
    m_queryTimer->start(m_queryGap);
    m_expireTimer->start();
    return true;
}

void Discovery::stop()
{
    if (!m_running)
        return;
    unpublish();
    m_queryTimer->stop();
    m_expireTimer->stop();
    m_t->stop();
    m_running = false;
    const QStringList ids = m_entries.keys();
    for (const QString &id : ids)
        drop(id);
    multicastlock::release(m_holder);
}

bool Discovery::publish(const QString &id, quint16 port, bool pairingOpen)
{
    if (!m_running || !mdns::validInstanceId(id) || port == 0)
        return false;
    m_id = id;
    m_port = port;
    m_pairing = pairingOpen;
    m_published = true;
    sendAnnouncement(120);
    m_announceTimer->start(m_gap);
    m_refreshTimer->start();
    return true;
}

void Discovery::setPairingOpen(bool open)
{
    if (m_pairing == open)
        return;
    m_pairing = open;
    if (m_published)
        sendAnnouncement(120);
}

void Discovery::unpublish()
{
    if (!m_published)
        return;
    sendAnnouncement(0);
    m_published = false;
    m_announceTimer->stop();
    m_refreshTimer->stop();
}

void Discovery::sendAnnouncement(quint32 ttl)
{
    const QByteArray p = mdns::build(mdns::announcement(m_id, m_port, m_t->localAddresses(), m_pairing, ttl));
    if (!p.isEmpty())
        m_t->send(p);
}

void Discovery::sendQuery()
{
    m_t->send(mdns::build(mdns::browseQuery()));
}

QList<mdns::PeerInfo> Discovery::peers() const
{
    QList<mdns::PeerInfo> out;
    for (auto it = m_entries.constBegin(); it != m_entries.constEnd(); ++it)
        if (it->announced)
            out << it->info;
    return out;
}

bool Discovery::acceptable(const QHostAddress &a) const
{
    if (a.protocol() != QAbstractSocket::IPv4Protocol)
        return false;
    const quint32 v = a.toIPv4Address();
    if (v == 0 || v == 0xFFFFFFFFu || (v >> 28) == 0xE)   // any, broadcast, multicast
        return false;
    if ((v >> 24) == 127 && !m_acceptLoopback)
        return false;
    return true;
}

void Discovery::onDatagram(const QByteArray &packet, const QHostAddress &from)
{
    mdns::Message m;
    if (!mdns::parse(packet, &m))
        return;
    if (m.response)
        onResponse(m, from);
    else
        onQuery(m);
}

void Discovery::onQuery(const mdns::Message &m)
{
    if (!m_published)
        return;
    bool ours = false;
    for (const mdns::Question &q : m.questions) {
        if ((q.type == mdns::kTypePtr && q.name == mdns::serviceName()) || q.name == mdns::instanceName(m_id)
            || q.name == mdns::hostName(m_id))
            ours = true;
    }
    if (!ours)
        return;
    const qint64 t = now();
    if (t - m_lastAnswer < 200)   // never answer more often than five times a second
        return;
    m_lastAnswer = t;
    sendAnnouncement(120);
}

void Discovery::onResponse(const mdns::Message &m, const QHostAddress &from)
{
    QStringList touched;
    for (const mdns::Record &r : m.records) {
        QString id;
        if (r.type == mdns::kTypePtr && r.name == mdns::serviceName()) {
            const QString suffix = QLatin1Char('.') + mdns::serviceName();
            if (!r.target.endsWith(suffix))
                continue;
            id = r.target.left(r.target.size() - suffix.size());
        } else if (r.name.endsWith(QStringLiteral(".") + mdns::serviceName())
                   && (r.type == mdns::kTypeSrv || r.type == mdns::kTypeTxt)) {
            id = r.name.left(r.name.size() - mdns::serviceName().size() - 1);
        } else if (r.type == mdns::kTypeA && r.name.endsWith(QStringLiteral(".local"))) {
            id = r.name.left(r.name.size() - 6);
        } else {
            continue;
        }
        if (!mdns::validInstanceId(id) || (m_published && id == m_id))
            continue;
        if (!m_entries.contains(id) && m_entries.size() >= kMaxPeers)
            continue;
        Entry &e = m_entries[id];
        e.info.id = id;
        e.from = from;
        if (e.expires == 0)
            e.expires = now() + 120000;   // a partial entry that never completes goes away too
        const qint64 until = now() + qint64(qMin<quint32>(r.ttl, 4500)) * 1000;
        switch (r.type) {
        case mdns::kTypePtr:
            if (r.ttl == 0) {
                drop(id);
                continue;
            }
            e.havePtr = true;
            e.expires = until;
            break;
        case mdns::kTypeSrv:
            if (r.ttl == 0 || r.port == 0 || r.target != mdns::hostName(id))
                break;
            e.haveSrv = true;
            e.info.port = r.port;
            break;
        case mdns::kTypeTxt: {
            if (r.ttl == 0)
                break;
            int version = 0;
            bool pairing = false;
            for (const QString &s : r.txt) {
                if (s.startsWith(QStringLiteral("v=")))
                    version = s.mid(2).toInt();
                else if (s == QStringLiteral("p=1"))
                    pairing = true;
            }
            e.info.version = version;
            e.info.pairingOpen = pairing;
            break;
        }
        case mdns::kTypeA:
            e.sawA = true;
            if (r.ttl == 0 || !acceptable(r.address))
                break;
            if (!e.info.addresses.contains(r.address) && e.info.addresses.size() < kMaxAddresses)
                e.info.addresses << r.address;
            break;
        }
        if (!touched.contains(id))
            touched << id;
    }
    for (const QString &id : touched)
        update(id);
}

void Discovery::update(const QString &id)
{
    auto it = m_entries.find(id);
    if (it == m_entries.end())
        return;
    Entry &e = *it;
    if (!e.havePtr || !e.haveSrv || e.info.version != 1)
        return;
    if (e.info.addresses.isEmpty() && !e.sawA && acceptable(e.from))   // no A record: the sender's own address
        e.info.addresses << e.from;
    if (e.info.addresses.isEmpty())
        return;
    if (!e.announced) {
        e.announced = true;
        emit peerFound(e.info);
        m_lastEmitted[id] = e.info;
    } else {
        const mdns::PeerInfo &old = m_lastEmitted[id];
        if (old.port != e.info.port || old.pairingOpen != e.info.pairingOpen || old.addresses != e.info.addresses) {
            m_lastEmitted[id] = e.info;
            emit peerChanged(e.info);
        }
    }
}

void Discovery::drop(const QString &id)
{
    auto it = m_entries.find(id);
    if (it == m_entries.end())
        return;
    const bool was = it->announced;
    m_entries.erase(it);
    m_lastEmitted.remove(id);
    if (was)
        emit peerLost(id);
}

void Discovery::expire(qint64 nowMs)
{
    const QStringList ids = m_entries.keys();
    for (const QString &id : ids) {
        const Entry &e = m_entries[id];
        if (e.expires != 0 && e.expires <= nowMs)
            drop(id);
    }
}

} // namespace collab
} // namespace qb
