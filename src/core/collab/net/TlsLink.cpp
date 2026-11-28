// The TLS 1.3 link and the listener of Live Sync: see TlsLink.h.
#include "core/collab/net/TlsLink.h"

#include "core/collab/net/TlsInternal.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QRegularExpression>
#include <QTimer>

#ifdef QB_HAVE_BOTAN_TLS
#include <botan/credentials_manager.h>
#include <botan/tls.h>
#include <botan/tls_exceptn.h>
#include <botan/tls_policy.h>
#include <botan/tls_session_manager_noop.h>
#include <botan/x509cert.h>
#endif

namespace qb {
namespace collab {
namespace net {

using Trust = PeerInfo::Trust;

// ---- trust policies ----

TrustPolicy TrustPolicy::paired(const PeerStore *peers)
{
    TrustPolicy p;
    p.decide = [peers](const QByteArray &pin, bool isServer) {
        if (peers && peers->contains(pin))
            return Trust::Paired;
        return isServer ? Trust::Unknown : Reject;
    };
    return p;
}

TrustPolicy TrustPolicy::pairing(std::function<bool()> open)
{
    TrustPolicy p;
    p.decide = [open](const QByteArray &, bool isServer) {
        if (open && !open())
            return isServer ? Trust::Unknown : Reject;
        return Trust::Pairing;
    };
    return p;
}

TrustPolicy TrustPolicy::pairedOrPairing(const PeerStore *peers, std::function<bool()> windowOpen)
{
    TrustPolicy p;
    p.decide = [peers, windowOpen](const QByteArray &pin, bool isServer) {
        if (peers && peers->contains(pin))
            return Trust::Paired;
        if (windowOpen && windowOpen())
            return Trust::Pairing;
        return isServer ? Trust::Unknown : Reject;
    };
    return p;
}

TrustPolicy TrustPolicy::pinned(const QByteArray &pin)
{
    TrustPolicy p;
    p.decide = [pin](const QByteArray &seen, bool) { return seen == pin ? Trust::Paired : Reject; };
    return p;
}

#ifndef QB_HAVE_BOTAN_TLS

// a build without Botan TLS: a link that fails at once
struct TlsLink::Impl {};
bool TlsLink::available() { return false; }
TlsLink::TlsLink(QTcpSocket *socket, Role r, DeviceIdentity *, TrustPolicy, QObject *parent) : Link(parent), sock(socket), role(r)
{
    sock->setParent(this);
    QTimer::singleShot(0, this, [this] { failWith(QStringLiteral("NO_TLS")); });
}
TlsLink::~TlsLink() = default;
TlsLink *TlsLink::connectTo(const QString &, quint16, DeviceIdentity *self, TrustPolicy trust, QObject *parent)
{
    return new TlsLink(new QTcpSocket, Role::Client, self, std::move(trust), parent);
}
bool TlsLink::send(const Frame &) { return false; }
qint64 TlsLink::bytesWaiting() const { return 0; }
void TlsLink::close(const QString &c) { failWith(c); }
void TlsLink::setTimeouts(int, int) {}
void TlsLink::setClock(std::function<qint64()>) {}
void TlsLink::setState(State s)
{
    st = s;
    emit stateChanged(s);
}
void TlsLink::failWith(const QString &c)
{
    if (st == State::Closed)
        return;
    code = c;
    setState(State::Closed);
    emit closed(c);
}
void TlsLink::sendBytesForTest(const QByteArray &) {}
void TlsLink::onConnected() {}
void TlsLink::onReadyRead() {}
void TlsLink::onSocketClosed() {}
void TlsLink::activated() {}
void TlsLink::onFrames(const QList<Frame> &) {}
void TlsLink::sendRaw(const Frame &) {}

#else

namespace {

// the keys this side shows
class Creds : public Botan::Credentials_Manager {
public:
    explicit Creds(DeviceIdentity::Impl *id) : id(id) {}
    std::vector<Botan::X509_Certificate> cert_chain(const std::vector<std::string> &, const std::vector<Botan::AlgorithmIdentifier> &,
                                                    const std::string &, const std::string &) override
    {
        return {*id->cert};
    }
    std::shared_ptr<Botan::Private_Key> private_key_for(const Botan::X509_Certificate &, const std::string &, const std::string &) override
    {
        return id->key;
    }

private:
    DeviceIdentity::Impl *id;
};

// TLS 1.3 only, ECDSA P-256 certificates, AES-GCM, a client certificate always, no tickets
class Pol : public Botan::TLS::Policy {
public:
    bool allow_tls12() const override { return false; }
    bool allow_tls13() const override { return true; }
    std::vector<std::string> allowed_ciphers() const override { return {"AES-256/GCM", "AES-128/GCM"}; }
    std::vector<std::string> allowed_signature_hashes() const override { return {"SHA-256", "SHA-384"}; }
    std::vector<std::string> allowed_signature_methods() const override { return {"ECDSA"}; }
    std::vector<std::string> allowed_key_exchange_methods() const override { return {"ECDH"}; }
    std::vector<Botan::TLS::Group_Params> key_exchange_groups() const override
    {
        return {Botan::TLS::Group_Params::X25519, Botan::TLS::Group_Params::SECP256R1};
    }
    bool require_client_certificate_authentication() const override { return true; }
    bool request_client_certificate_authentication() const override { return true; }
    size_t new_session_tickets_upon_handshake_success() const override { return 0; }
};

QByteArray toQ(std::span<const uint8_t> d) { return QByteArray(reinterpret_cast<const char *>(d.data()), qsizetype(d.size())); }

} // namespace

struct TlsLink::Impl {
    // the callbacks, as a separate object that outlives a moment of re-entry
    struct CB : Botan::TLS::Callbacks {
        TlsLink *q = nullptr;
        TrustPolicy trust;
        bool server = false;
        FrameReader reader;
        QList<Frame> inbox;
        bool active = false;
        bool rejected = false;
        PeerInfo seen;

        void tls_emit_data(std::span<const uint8_t> data) override
        {
            if (q && q->sock && q->sock->state() == QAbstractSocket::ConnectedState)
                q->sock->write(reinterpret_cast<const char *>(data.data()), qint64(data.size()));
        }
        void tls_record_received(uint64_t, std::span<const uint8_t> data) override
        {
            const QList<Frame> f = reader.feed(toQ(data));
            inbox.append(f);
        }
        void tls_alert(Botan::TLS::Alert) override {}
        void tls_session_activated() override { active = true; }
        void tls_verify_cert_chain(const std::vector<Botan::X509_Certificate> &chain,
                                   const std::vector<std::optional<Botan::OCSP::Response>> &,
                                   const std::vector<Botan::Certificate_Store *> &, Botan::Usage_Type, std::string_view,
                                   const Botan::TLS::Policy &) override
        {
            const auto reject = [&] {
                rejected = true;
                throw Botan::TLS::TLS_Exception(Botan::TLS::Alert::BadCertificate, "the peer's key is not accepted");
            };
            if (chain.size() != 1 || !chain[0].is_self_signed())
                reject();
            const auto key = chain[0].subject_public_key();
            if (!key || key->algo_name() != "ECDSA" || !chain[0].check_signature(*key))
                reject();
            const std::vector<uint8_t> &spki = chain[0].subject_public_key_info();
            const QByteArray der(reinterpret_cast<const char *>(spki.data()), qsizetype(spki.size()));
            seen.pin = DeviceIdentity::pinOfPublicKey(der);
            seen.publicKey = der;
            seen.deviceId = DeviceIdentity::deviceIdOfPin(seen.pin);
            const Trust v = trust.decide ? trust.decide(seen.pin, server) : TrustPolicy::Reject;
            if (v == TrustPolicy::Reject || (!server && v == Trust::Unknown))
                reject();
            seen.trust = v;
        }
    };

    std::shared_ptr<CB> cb = std::make_shared<CB>();
    std::shared_ptr<Creds> creds;
    std::shared_ptr<Pol> policy = std::make_shared<Pol>();
    std::unique_ptr<Botan::TLS::Channel> chan;
    QTimer timer;
    DeviceIdentity *self = nullptr;
};

bool TlsLink::available() { return true; }

TlsLink::TlsLink(QTcpSocket *socket, Role r, DeviceIdentity *self, TrustPolicy trust, QObject *parent)
    : Link(parent), d(new Impl), sock(socket), role(r)
{
    qRegisterMetaType<qb::collab::net::Frame>();
    qRegisterMetaType<qb::collab::net::Link::State>();
    sock->setParent(this);
    d->self = self;
    d->creds = std::make_shared<Creds>(self->impl());
    d->cb->q = this;
    d->cb->trust = std::move(trust);
    d->cb->server = r == Role::Server;
    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, this, [this] { failWith(QStringLiteral("HANDSHAKE_TIMEOUT")); });
    connect(sock, &QTcpSocket::readyRead, this, &TlsLink::onReadyRead);
    connect(sock, &QTcpSocket::disconnected, this, &TlsLink::onSocketClosed);
    connect(sock, &QAbstractSocket::errorOccurred, this, [this] {
        if (st == State::Connecting)
            failWith(QStringLiteral("CONNECT_FAILED"));
    });
    info.address = sock->peerAddress().toString();
    d->timer.start(handshakeMs);
    if (sock->state() == QAbstractSocket::ConnectedState) {
        QTimer::singleShot(0, this, &TlsLink::onConnected);
    } else {
        connect(sock, &QTcpSocket::connected, this, &TlsLink::onConnected);
    }
}

TlsLink::~TlsLink()
{
    if (d && d->cb)
        d->cb->q = nullptr;
    d->chan.reset();
}

TlsLink *TlsLink::connectTo(const QString &host, quint16 port, DeviceIdentity *self, TrustPolicy trust, QObject *parent)
{
    auto *s = new QTcpSocket;
    auto *link = new TlsLink(s, Role::Client, self, std::move(trust), parent);
    s->connectToHost(host, port);
    return link;
}

void TlsLink::setTimeouts(int h, int hello)
{
    handshakeMs = h;
    helloMs = hello;
    if (st == State::Connecting || st == State::Handshaking)
        d->timer.start(helloSent ? helloMs : handshakeMs);
}

void TlsLink::setClock(std::function<qint64()> nowMs) { d->cb->reader = FrameReader(std::move(nowMs)); }

void TlsLink::setState(State s)
{
    if (st == s)
        return;
    st = s;
    emit stateChanged(s);
}

void TlsLink::onConnected()
{
    if (st == State::Closed || d->chan)
        return;
    info.address = sock->peerAddress().toString();
    setState(State::Handshaking);
    try {
        if (role == Role::Client) {
            d->chan = std::make_unique<Botan::TLS::Client>(d->cb, std::make_shared<Botan::TLS::Session_Manager_Noop>(), d->creds,
                                                           d->policy, sharedRng(), Botan::TLS::Server_Information());
        } else {
            d->chan = std::make_unique<Botan::TLS::Server>(d->cb, std::make_shared<Botan::TLS::Session_Manager_Noop>(), d->creds,
                                                           d->policy, sharedRng());
        }
    } catch (const std::exception &) {
        failWith(QStringLiteral("TLS_ERROR"));
        return;
    }
    if (sock->bytesAvailable() > 0)
        onReadyRead();
}

void TlsLink::onReadyRead()
{
    const QByteArray bytes = sock->readAll();
    if (st == State::Closed || !d->chan || bytes.isEmpty())
        return;
    try {
        d->chan->received_data(reinterpret_cast<const uint8_t *>(bytes.constData()), size_t(bytes.size()));
    } catch (const std::exception &) {
        failWith(d->cb->rejected ? QStringLiteral("IDENTITY_MISMATCH") : QStringLiteral("TLS_ERROR"));
        return;
    }
    if (d->cb->reader.error() != FrameError::None) {
        failWith(frameErrorCode(d->cb->reader.error()));
        return;
    }
    if (d->cb->active && !helloSent && st == State::Handshaking)
        activated();
    if (st == State::Closed)
        return;
    if (!d->cb->inbox.isEmpty()) {
        const QList<Frame> frames = d->cb->inbox;
        d->cb->inbox.clear();
        onFrames(frames);
    }
}

void TlsLink::sendRaw(const Frame &f)
{
    const QByteArray bytes = encodeFrame(f);
    if (bytes.isEmpty() || !d->chan || !d->chan->is_active())
        return;
    try {
        d->chan->send(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(bytes.constData()), size_t(bytes.size())));
    } catch (const std::exception &) {
        failWith(QStringLiteral("TLS_ERROR"));
    }
}

void TlsLink::sendBytesForTest(const QByteArray &bytes)
{
    if (!d->chan || !d->chan->is_active())
        return;
    try {
        d->chan->send(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(bytes.constData()), size_t(bytes.size())));
    } catch (const std::exception &) {
    }
}

void TlsLink::activated()
{
    helloSent = true;
    proto = QStringLiteral("TLS v1.3"); // (the policy allows nothing else)
    info.pin = d->cb->seen.pin;
    info.publicKey = d->cb->seen.publicKey;
    info.deviceId = d->cb->seen.deviceId;
    info.trust = d->cb->seen.trust;
    d->timer.start(helloMs);
    if (role == Role::Server && info.trust == Trust::Unknown) {
        // a key we do not know: one word, then nothing more (no names, no counts)
        sendRaw(Frame::json(QJsonObject{{QStringLiteral("t"), QStringLiteral("ERROR")}, {QStringLiteral("code"), QStringLiteral("NOT_PAIRED")}}));
        failWith(QStringLiteral("NOT_PAIRED"));
        return;
    }
    if (!quiet)
        sendRaw(Frame::json(QJsonObject{{QStringLiteral("t"), QStringLiteral("HELLO")},
                                        {QStringLiteral("v"), 1},
                                        {QStringLiteral("id"), d->self->deviceId()}}));
}

void TlsLink::onFrames(const QList<Frame> &frames)
{
    for (const Frame &f : frames) {
        if (st == State::Closed)
            return;
        if (!helloSeen) {
            // the first frame is the peer's HELLO (or, for a client, the server's ERROR)
            bool ok = false;
            const QJsonObject o = f.object(&ok);
            const QString t = o.value(QStringLiteral("t")).toString();
            if (!ok) {
                failWith(QStringLiteral("BAD_FRAME"));
                return;
            }
            if (t == QLatin1String("ERROR")) {
                static const QRegularExpression valid(QStringLiteral("^[A-Z_]{1,32}$"));
                const QString c = o.value(QStringLiteral("code")).toString();
                failWith(valid.match(c).hasMatch() ? c : QStringLiteral("BAD_FRAME"));
                return;
            }
            if (t != QLatin1String("HELLO") || o.value(QStringLiteral("id")).toString() != info.deviceId) {
                failWith(t == QLatin1String("HELLO") ? QStringLiteral("IDENTITY_MISMATCH") : QStringLiteral("BAD_FRAME"));
                return;
            }
            helloSeen = true;
            d->timer.stop();
            setState(State::Ready);
            continue;
        }
        emit frameReceived(f);
    }
}

bool TlsLink::send(const Frame &frame)
{
    if (st != State::Ready)
        return false;
    if (encodeFrame(frame).isEmpty())
        return false;
    sendRaw(frame);
    return st == State::Ready;
}

qint64 TlsLink::bytesWaiting() const { return sock ? sock->bytesToWrite() : 0; }

void TlsLink::failWith(const QString &c)
{
    if (st == State::Closed)
        return;
    code = c;
    closing = true;
    d->timer.stop();
    try {
        if (d->chan && d->chan->is_active())
            d->chan->close();
    } catch (const std::exception &) {
    }
    setState(State::Closed);
    if (sock) {
        if (sock->state() == QAbstractSocket::ConnectedState)
            sock->disconnectFromHost(); // (after what is still to be written)
        else if (sock->state() != QAbstractSocket::UnconnectedState)
            sock->abort();
    }
    emit closed(c);
}

void TlsLink::close(const QString &c) { failWith(c); }

void TlsLink::onSocketClosed()
{
    if (st == State::Closed)
        return;
    failWith(helloSeen ? QStringLiteral("CONNECTION_LOST") : QStringLiteral("HANDSHAKE_FAILED"));
}

#endif // QB_HAVE_BOTAN_TLS

// ---- the listener ----

TlsServer::TlsServer(DeviceIdentity *s, TrustPolicy t, QObject *parent) : QObject(parent), self(s), trust(std::move(t))
{
    server.setParent(this);
    clock = [] { return QDateTime::currentMSecsSinceEpoch(); };
    connect(&server, &QTcpServer::newConnection, this, &TlsServer::onNew);
}

TlsServer::~TlsServer() = default;

bool TlsServer::listen(const QHostAddress &address, quint16 port)
{
    if (server.isListening())
        return true;
    if (port != 0 && server.listen(address, port))
        return true;
    return server.listen(address, 0);
}

quint16 TlsServer::port() const { return server.serverPort(); }
bool TlsServer::isListening() const { return server.isListening(); }
void TlsServer::close() { server.close(); }

int TlsServer::linkCount() const
{
    int n = 0;
    for (const TlsLink *l : links)
        if (l->state() != Link::State::Closed)
            ++n;
    return n;
}

void TlsServer::onNew()
{
    while (QTcpSocket *s = server.nextPendingConnection()) {
        const QString addr = s->peerAddress().toString();
        const qint64 now = clock();
        QList<qint64> &list = attempts[addr];
        while (!list.isEmpty() && now - list.first() >= 60000)
            list.removeFirst();
        list.append(now);
        links.removeIf([](TlsLink *l) { return l->state() == Link::State::Closed; });
        int unauth = 0;
        for (TlsLink *l : std::as_const(links))
            if (l->state() != Link::State::Ready)
                ++unauth;
        QString refusal;
        if (list.size() > kAttemptsPerMinute)
            refusal = QStringLiteral("RATE_LIMIT");
        else if (links.size() >= kMaxLinks)
            refusal = QStringLiteral("TOO_MANY");
        else if (unauth >= kMaxUnauthenticated)
            refusal = QStringLiteral("TOO_MANY_UNAUTH");
        if (!refusal.isEmpty()) {
            s->abort();
            s->deleteLater();
            emit refused(addr, refusal);
            continue;
        }
        auto *link = new TlsLink(s, TlsLink::Role::Server, self, trust, this);
        links.append(link);
        connect(link, &QObject::destroyed, this, [this, link] { links.removeAll(link); });
        emit newLink(link);
    }
}

} // namespace net
} // namespace collab
} // namespace qb
