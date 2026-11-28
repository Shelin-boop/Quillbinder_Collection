#include "core/collab/LocalLink.h"

#include <QCryptographicHash>

namespace qb {
namespace collab {

using net::Frame;
using net::Link;

LocalLink::LocalLink(QLocalSocket *socket, QObject *parent) : Link(parent), m_sock(socket)
{
    socket->setParent(this);
    connect(socket, &QLocalSocket::readyRead, this, &LocalLink::onReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, &LocalLink::onGone);
    connect(socket, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError e) {
        if (m_state == State::Connecting || e == QLocalSocket::PeerClosedError)
            onGone();
    });
    connect(socket, &QLocalSocket::connected, this, [this] { setState(State::Ready); });
    if (socket->state() == QLocalSocket::ConnectedState)
        m_state = State::Ready;
}

LocalLink::~LocalLink() = default;

LocalLink *LocalLink::connectTo(const QString &name, QObject *parent)
{
    auto *s = new QLocalSocket;
    auto *l = new LocalLink(s, parent);
    s->connectToServer(name);
    return l;
}

QString LocalLink::serverName(const QString &dataDir)
{
    const QByteArray h = QCryptographicHash::hash(dataDir.toUtf8(), QCryptographicHash::Sha256).toHex().left(20);
    return QStringLiteral("qb-ls-") + QString::fromLatin1(h);
}

net::PeerInfo LocalLink::peer() const
{
    net::PeerInfo p;
    p.deviceId = QStringLiteral("local");
    p.address = QStringLiteral("local");
    p.trust = net::PeerInfo::Trust::Paired;
    return p;
}

void LocalLink::setState(State s)
{
    if (m_state == s)
        return;
    m_state = s;
    emit stateChanged(s);
}

bool LocalLink::send(const Frame &frame)
{
    if (m_state != State::Ready || !m_sock)
        return false;
    const QByteArray bytes = net::encodeFrame(frame);
    if (bytes.isEmpty())
        return false;
    m_sock->write(bytes);
    return true;
}

void LocalLink::onReadyRead()
{
    if (!m_sock)
        return;
    const QList<Frame> frames = m_reader.feed(m_sock->readAll());
    if (m_reader.error() != net::FrameError::None) {
        close(net::frameErrorCode(m_reader.error()));
        return;
    }
    QPointer<LocalLink> self(this);
    for (const Frame &f : frames) {
        if (!self || m_state != State::Ready)
            return;
        emit frameReceived(f);
    }
}

void LocalLink::onGone()
{
    if (m_state == State::Closed)
        return;
    setState(State::Closed);
    emit closed(code);
}

void LocalLink::close(const QString &c)
{
    if (m_state == State::Closed)
        return;
    code = c;
    if (m_sock) {
        m_sock->flush();
        m_sock->disconnectFromServer();
        if (m_sock->state() != QLocalSocket::UnconnectedState)
            m_sock->abort();
    }
    onGone();
}

// ---------------------------------------------------------------- server

LocalServer::LocalServer(QObject *parent) : QObject(parent)
{
    m_server.setSocketOptions(QLocalServer::UserAccessOption);
    connect(&m_server, &QLocalServer::newConnection, this, [this] {
        while (QLocalSocket *s = m_server.nextPendingConnection())
            emit newLink(new LocalLink(s, this));
    });
}

bool LocalServer::isServed(const QString &name, int timeoutMs)
{
    QLocalSocket s;
    s.connectToServer(name);
    const bool ok = s.waitForConnected(timeoutMs);
    s.abort();
    return ok;
}

bool LocalServer::listen(const QString &name, QString *error)
{
    if (m_server.isListening())
        return true;
    if (isServed(name)) {
        if (error)
            *error = QStringLiteral("another service already answers on this socket");
        return false;
    }
    QLocalServer::removeServer(name); // the socket of a service that crashed
    if (!m_server.listen(name)) {
        if (error)
            *error = m_server.errorString();
        return false;
    }
    return true;
}

void LocalServer::close() { m_server.close(); }

} // namespace collab
} // namespace qb
