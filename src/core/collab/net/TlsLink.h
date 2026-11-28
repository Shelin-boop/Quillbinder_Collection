#pragma once
// The real network link of Live Sync (0.99.0): TLS 1.3 (Botan, linked statically) over a QTcpSocket, with the frames of
// Frames.h inside. One thread, nothing blocks. Both sides show a certificate (mutual authentication); a certificate is
// trusted only through its pin (the SHA-256 of its public key), never through a chain, a name or a date.
//
// Trust is decided by a TrustPolicy at the moment the peer's certificate arrives:
//   - paired:  the pin is in the peer store (or the project's membership); a server meets an unknown key with the frame
//              {"t":"ERROR","code":"NOT_PAIRED"} and closes, saying nothing else; a client refuses a server it does not know
//   - pairing: any key is taken, but the link is marked Pairing: only the pairing exchange may run on it
//   - pinned:  a client that expects exactly one key (a project's host)
// After the handshake both sides send {"t":"HELLO","v":1,"id":<own device id>} first; the link is Ready when the peer's
// HELLO has come within 5 s and its id is that of the key it authenticated with.
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/net/Link.h"
#include "core/collab/net/PeerStore.h"

#include <QHash>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <functional>
#include <memory>

namespace qb {
namespace collab {
namespace net {

struct TrustPolicy {
    // Reject: the handshake is stopped (IDENTITY_MISMATCH). Unknown: handshake done, then NOT_PAIRED (a client treats it
    // as Reject). Pairing, Paired, Member: taken, with that trust.
    using Verdict = PeerInfo::Trust;
    static constexpr Verdict Reject = PeerInfo::Trust::None;
    std::function<Verdict(const QByteArray &pin, bool isServer)> decide;

    static TrustPolicy paired(const PeerStore *peers);
    // every key is taken for a pairing exchange (the host's window is open); `open` says whether it is still
    static TrustPolicy pairing(std::function<bool()> open = nullptr);
    // the peers first; where they do not know the key and the window is open: pairing
    static TrustPolicy pairedOrPairing(const PeerStore *peers, std::function<bool()> windowOpen);
    // only this pin, as Paired
    static TrustPolicy pinned(const QByteArray &pin);
};

class TlsLink : public Link {
    Q_OBJECT
public:
    enum class Role { Client, Server };
    static bool available();
    static constexpr int kHandshakeMs = 10000;
    static constexpr int kHelloMs = 5000;

    // takes the socket (it becomes this link's child). A client socket may still be connecting; the handshake starts when
    // it is connected.
    TlsLink(QTcpSocket *socket, Role role, DeviceIdentity *self, TrustPolicy trust, QObject *parent = nullptr);
    ~TlsLink() override;
    // a client link to a host and port
    static TlsLink *connectTo(const QString &host, quint16 port, DeviceIdentity *self, TrustPolicy trust, QObject *parent = nullptr);

    State state() const override { return st; }
    PeerInfo peer() const override { return info; }
    bool send(const Frame &frame) override;
    qint64 bytesWaiting() const override;
    void close(const QString &code = QString()) override;

    // for the tests
    void setTimeouts(int handshakeMs, int helloMs);
    // the clock of the frame reader's rate limit
    void setClock(std::function<qint64()> nowMs);
    // does not send the HELLO (a test that plays a client which stays silent)
    void setSilent(bool silent) { quiet = silent; }
    // sends these bytes inside the TLS connection as they are (a test that plays a broken peer; not used by the program)
    void sendBytesForTest(const QByteArray &bytes);
    // the TLS version the connection ended up with ("TLS v1.3"), empty before the handshake
    QString protocol() const { return proto; }

    struct Impl;

private:
    friend struct Impl;
    void setState(State s);
    void onConnected();
    void onReadyRead();
    void onSocketClosed();
    void failWith(const QString &code);
    void activated();
    void onFrames(const QList<Frame> &frames);
    void sendRaw(const Frame &f);

    std::unique_ptr<Impl> d;
    QTcpSocket *sock = nullptr;
    Role role;
    State st = State::Connecting;
    PeerInfo info;
    QString proto;
    bool quiet = false;
    bool helloSent = false;
    bool helloSeen = false;
    bool closing = false;
    int handshakeMs = kHandshakeMs, helloMs = kHelloMs;
};

// The listener of a device (0.99.0): accepts connections, hands each one over as a server-side TlsLink after the limits of
// the design: at most 32 links, at most 4 not yet Ready, 10 attempts per minute per address.
class TlsServer : public QObject {
    Q_OBJECT
public:
    static constexpr quint16 kPreferredPort = 50573;
    static constexpr int kMaxLinks = 32;
    static constexpr int kMaxUnauthenticated = 4;
    static constexpr int kAttemptsPerMinute = 10;

    TlsServer(DeviceIdentity *self, TrustPolicy trust, QObject *parent = nullptr);
    ~TlsServer() override;
    // listens on the preferred port, else on a free one; the address is any IPv4 address unless a test says otherwise
    bool listen(const QHostAddress &address = QHostAddress::AnyIPv4, quint16 port = kPreferredPort);
    // listens only on the given port, no other (false when it is taken)
    quint16 port() const;
    void close();
    bool isListening() const;
    // the links that are not closed
    int linkCount() const;
    void setClock(std::function<qint64()> nowMs) { clock = std::move(nowMs); }

signals:
    // a link whose handshake is under way (its state changes tell the rest); the server keeps it as a child
    void newLink(qb::collab::net::TlsLink *link);
    // a connection was refused by a limit (the code is the one a log would show: TOO_MANY, TOO_MANY_UNAUTH, RATE_LIMIT)
    void refused(const QString &address, const QString &code);

private:
    void onNew();
    DeviceIdentity *self;
    TrustPolicy trust;
    QTcpServer server;
    QList<TlsLink *> links;
    QHash<QString, QList<qint64>> attempts;
    std::function<qint64()> clock;
};

} // namespace net
} // namespace collab
} // namespace qb
