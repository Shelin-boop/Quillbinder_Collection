#pragma once
// Live Sync, the link between a program's windows and its service (0.99.0, section 4.1 of the design): the frames of
// net/Frames.h over a local socket (a Unix socket, a named pipe on Windows) named after the program's data folder. Only the
// user's own programs can reach it (user access only). Its peers are this device's own programs, so the trust is Paired and
// the peer has no key.
#include "core/collab/net/Frames.h"
#include "core/collab/net/Link.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>

namespace qb {
namespace collab {

class LocalLink : public net::Link {
    Q_OBJECT
public:
    // takes the socket (it becomes this link's child)
    explicit LocalLink(QLocalSocket *socket, QObject *parent = nullptr);
    ~LocalLink() override;
    // connects to the named server; the link is Ready when connected (Closed at once when nobody listens)
    static LocalLink *connectTo(const QString &name, QObject *parent = nullptr);

    State state() const override { return m_state; }
    net::PeerInfo peer() const override;
    bool send(const net::Frame &frame) override;
    qint64 bytesWaiting() const override { return m_sock ? m_sock->bytesToWrite() : 0; }
    void close(const QString &code = QString()) override;

    // the name of the service's socket for a data folder: short, the same for every program that uses that folder
    static QString serverName(const QString &dataDir);

private:
    void onReadyRead();
    void onGone();
    void setState(State s);
    QPointer<QLocalSocket> m_sock;
    State m_state = State::Connecting;
    net::FrameReader m_reader;
};

class LocalServer : public QObject {
    Q_OBJECT
public:
    explicit LocalServer(QObject *parent = nullptr);
    // false with a reason when the name is taken by a live server; a stale socket of a crashed one is removed
    bool listen(const QString &name, QString *error = nullptr);
    void close();
    bool isListening() const { return m_server.isListening(); }
    QString name() const { return m_server.serverName(); }
    // whether some server answers on the name (a connection that is closed again at once)
    static bool isServed(const QString &name, int timeoutMs = 300);

signals:
    void newLink(qb::collab::LocalLink *link);

private:
    QLocalServer m_server;
};

} // namespace collab
} // namespace qb
