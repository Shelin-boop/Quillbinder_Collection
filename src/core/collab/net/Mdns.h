#pragma once
// 0.99.0, Live Sync: a small multicast DNS (RFC 6762) writer and a strict reader, and the transport under them.
// Only what finding another Quillbinder on the local network needs: PTR, SRV, TXT and A records, one question or
// answer section at a time. The reader treats every packet as hostile (see the limits below).

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {
namespace mdns {

constexpr quint16 kTypeA = 1;
constexpr quint16 kTypePtr = 12;
constexpr quint16 kTypeTxt = 16;
constexpr quint16 kTypeSrv = 33;
constexpr quint16 kClassIn = 1;

// Limits of the reader (a packet over these is refused whole, never cut).
constexpr int kMaxPacket = 1500;
constexpr int kMaxRecords = 64;       // questions + answers + authority + additional
constexpr int kMaxNameBytes = 255;
constexpr int kMaxPointerJumps = 8;   // compression pointers followed for one name
constexpr int kMaxRdata = 512;

struct Question {
    QString name;   // dotted, lower case
    quint16 type = 0;
};

struct Record {
    QString name;   // dotted, lower case
    quint16 type = 0;
    quint32 ttl = 0;
    // the parsed content, by type
    QString target;          // PTR: the instance name; SRV: the host name
    quint16 port = 0;        // SRV
    QHostAddress address;    // A
    QStringList txt;         // TXT: the strings "key=value"
};

struct Message {
    bool response = false;
    QList<Question> questions;
    QList<Record> records;   // answers, authority and additional together
};

// The packet of a message. Empty when it does not fit into kMaxPacket or a name is not valid.
QByteArray build(const Message &m);
// The strict reader: false (and `error`) for anything that is not exactly a well-formed packet within the limits:
// too short or long, a pointer that does not go back, a loop, too many records, a label over 63 bytes, a record
// whose data does not end where its length says, a type we do not know inside an answer is skipped but counted.
bool parse(const QByteArray &packet, Message *out, QString *error = nullptr);

// The names of the service (the instance's id is 16 lower-case hex digits).
QString serviceName();                       // "_quillbinder._tcp.local"
bool validInstanceId(const QString &id);
QString instanceName(const QString &id);     // "<id>._quillbinder._tcp.local"
QString hostName(const QString &id);         // "<id>.local"

// A packet carrying what a device publishes: PTR, SRV, TXT, A (ttl 0 = goodbye).
Message announcement(const QString &id, quint16 port, const QList<QHostAddress> &addresses, bool pairingOpen, quint32 ttl);
Message browseQuery();

// What a packet says about one instance, collected by the browser.
struct PeerInfo {
    QString id;
    quint16 port = 0;
    QList<QHostAddress> addresses;
    int version = 0;
    bool pairingOpen = false;
};

// ---- the transport (a socket for the real network, a stand-in for tests) ----
class Transport : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual bool start(QString *error = nullptr) = 0;
    virtual void stop() = 0;
    virtual void send(const QByteArray &packet) = 0;
    // The IPv4 addresses of this device on the interfaces in use (what an announcement carries).
    virtual QList<QHostAddress> localAddresses() const = 0;
signals:
    void datagram(const QByteArray &packet, const QHostAddress &from);
};

struct UdpOptions {
    quint16 port = 5353;
    QHostAddress group = QHostAddress(QStringLiteral("224.0.0.251"));
    bool includeLoopback = false;   // tests: the loopback interface counts as multicast-capable
    bool onlyLoopback = false;      // tests: use the loopback interface and no other
};

// UDP on the multicast group, one receiving socket (shared address) and one send per interface that is up, not
// point-to-point, multicast-capable and has an IPv4 address.
class UdpTransport : public Transport {
    Q_OBJECT
public:
    explicit UdpTransport(const UdpOptions &o = UdpOptions(), QObject *parent = nullptr);
    ~UdpTransport() override;
    bool start(QString *error = nullptr) override;
    void stop() override;
    void send(const QByteArray &packet) override;
    QList<QHostAddress> localAddresses() const override;
    int interfaceCount() const;

private:
    struct Impl;
    Impl *d;
};

} // namespace mdns
} // namespace collab
} // namespace qb
