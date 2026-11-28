#pragma once
// 0.99.0, Live Sync: finds other Quillbinder devices on the local network and lets them find this one (the design's
// section 2). A device publishes only an opaque instance id (16 hex digits), its port and addresses, and whether its
// pairing window is open; no device or machine name. What discovery finds is never trusted: the pinned key of the
// TLS link decides who a device is. Manual addresses (ManualAddress) work without any of it.

#include "core/collab/net/Mdns.h"

#include <QHash>
#include <QObject>
#include <functional>

class QTimer;

namespace qb {
namespace collab {

// "host", "host:port", "1.2.3.4:50573", "[fe80::1]:50573", a bare IPv6 address; the port is 1 to 65535.
struct ManualAddress {
    bool valid = false;
    QString host;      // a name or the text of an address; resolved by the socket that connects
    quint16 port = 0;
    QString error;     // when not valid: a short reason in English (the caller shows its own translated text)
};
constexpr quint16 kDefaultPort = 50573;
ManualAddress parseManualAddress(const QString &text, quint16 defaultPort = kDefaultPort);

class Discovery : public QObject {
    Q_OBJECT
public:
    // The transport is owned (for tests, a stand-in; createUdp for the real network).
    explicit Discovery(mdns::Transport *transport, QObject *parent = nullptr);
    ~Discovery() override;
    static Discovery *createUdp(const mdns::UdpOptions &options = mdns::UdpOptions(), QObject *parent = nullptr);

    // Opens the transport, takes the multicast lock (Android) and starts looking: a query now, then after 1, 2, 4 ...
    // up to 60 seconds. False (error) when no interface can do multicast; the manual address still works then.
    bool start(QString *error = nullptr);
    // Says goodbye if published, closes everything, releases the lock.
    void stop();
    bool running() const { return m_running; }

    // Publishes this device: announced twice (now and after announceGapMs), repeated every 60 s, answers queries.
    // `id` must be 16 lower-case hex digits. No effect before start().
    bool publish(const QString &id, quint16 port, bool pairingOpen);
    void setPairingOpen(bool open);   // announces again when it changed
    void unpublish();                 // the goodbye
    bool published() const { return m_published; }

    QList<mdns::PeerInfo> peers() const;
    // Drops what has expired at `nowMs` (the clock's time); the timer calls it every 5 s.
    void expire(qint64 nowMs);

    // Tests: a clock in milliseconds, loopback addresses accepted, the gap between the two announcements.
    void setClock(std::function<qint64()> clock) { m_clock = std::move(clock); }
    void setAcceptLoopback(bool on) { m_acceptLoopback = on; }
    void setAnnounceGapMs(int ms) { m_gap = ms; }

    static constexpr int kMaxPeers = 256;
    static constexpr int kMaxAddresses = 8;

signals:
    void peerFound(const qb::collab::mdns::PeerInfo &peer);
    void peerChanged(const qb::collab::mdns::PeerInfo &peer);   // port, addresses or the pairing flag changed
    void peerLost(const QString &id);

private:
    struct Entry {
        mdns::PeerInfo info;
        bool havePtr = false, haveSrv = false;
        bool sawA = false;        // an A record came (even one we refused: then the sender's address is not used)
        bool announced = false;   // peerFound was emitted
        QHostAddress from;
        qint64 expires = 0;
    };
    void onDatagram(const QByteArray &packet, const QHostAddress &from);
    void onQuery(const mdns::Message &m);
    void onResponse(const mdns::Message &m, const QHostAddress &from);
    void update(const QString &id);
    void drop(const QString &id);
    void sendAnnouncement(quint32 ttl);
    void sendQuery();
    bool acceptable(const QHostAddress &a) const;
    qint64 now() const;

    mdns::Transport *m_t = nullptr;
    bool m_running = false;
    bool m_published = false;
    bool m_acceptLoopback = false;
    QString m_id;
    quint16 m_port = 0;
    bool m_pairing = false;
    int m_gap = 1000;
    int m_queryGap = 1000;
    qint64 m_lastAnswer = -100000;
    std::function<qint64()> m_clock;
    QHash<QString, Entry> m_entries;
    QHash<QString, mdns::PeerInfo> m_lastEmitted;
    QTimer *m_queryTimer = nullptr;
    QTimer *m_announceTimer = nullptr;
    QTimer *m_refreshTimer = nullptr;
    QTimer *m_expireTimer = nullptr;
    QString m_holder;
};

} // namespace collab
} // namespace qb
