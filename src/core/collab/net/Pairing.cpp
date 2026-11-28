// Pairing two devices: see Pairing.h.
#include "core/collab/net/Pairing.h"

#include "core/collab/net/TlsInternal.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QRandomGenerator>
#include <QtEndian>

namespace qb {
namespace collab {
namespace net {

using Trust = PeerInfo::Trust;

QByteArray pairingCommitment(const QByteArray &pinHost, const QByteArray &pinGuest, const QByteArray &nonceHost)
{
    QCryptographicHash h(QCryptographicHash::Sha256);
    h.addData(QByteArrayView("QB-LS-PAIR-C1"));
    h.addData(pinHost);
    h.addData(pinGuest);
    h.addData(nonceHost);
    return h.result();
}

QString pairingCode(const QByteArray &pinHost, const QByteArray &pinGuest, const QByteArray &nonceHost, const QByteArray &nonceGuest)
{
    QCryptographicHash h(QCryptographicHash::Sha256);
    h.addData(QByteArrayView("QB-LS-PAIR-S1"));
    h.addData(pinHost);
    h.addData(pinGuest);
    h.addData(nonceHost);
    h.addData(nonceGuest);
    const QByteArray r = h.result();
    const quint64 v = qFromBigEndian<quint64>(r.constData());
    const quint32 c = quint32(v % 1000000u);
    const QString s = QStringLiteral("%1").arg(c, 6, 10, QLatin1Char('0'));
    return s.left(3) + QLatin1Char(' ') + s.mid(3);
}

#ifndef QB_HAVE_BOTAN_TLS
static QByteArray randomBytes(int n)
{
    QByteArray b(n, '\0');
    QRandomGenerator::system()->fillRange(reinterpret_cast<quint32 *>(b.data()), n / 4); // (n is a multiple of 4)
    return b;
}
#else
static QByteArray randomBytes(int n)
{
    QByteArray b(n, '\0');
    sharedRng()->randomize(reinterpret_cast<uint8_t *>(b.data()), size_t(n));
    return b;
}
#endif

// ---- the window ----

PairingWindow::PairingWindow(std::function<qint64()> nowMs, QObject *parent) : QObject(parent), clock(std::move(nowMs))
{
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [this] { isOpen(); });
}

qint64 PairingWindow::now() const { return clock ? clock() : QDateTime::currentMSecsSinceEpoch(); }

bool PairingWindow::open(QString *why)
{
    if (isOpen())
        return true;
    const qint64 t = now();
    while (!openings.isEmpty() && t - openings.first() >= kWindowsSpanMs)
        openings.removeFirst();
    if (openings.size() >= kMaxWindows) {
        if (why)
            *why = QStringLiteral("RATE");
        return false;
    }
    openings.append(t);
    isOpened = true;
    openedAt = t;
    used = 0;
    busy = false;
    timer.start(int(kWindowMs));
    emit opened();
    return true;
}

void PairingWindow::close()
{
    if (!isOpened)
        return;
    isOpened = false;
    busy = false;
    timer.stop();
    emit closed();
}

bool PairingWindow::isOpen()
{
    if (isOpened && now() - openedAt >= kWindowMs)
        close();
    return isOpened;
}

qint64 PairingWindow::remainingMs() { return isOpen() ? kWindowMs - (now() - openedAt) : 0; }

PairingWindow::Attempt PairingWindow::beginAttempt()
{
    if (!isOpen())
        return Attempt::Closed;
    if (busy)
        return Attempt::Busy;
    if (used >= kMaxAttempts)
        return Attempt::Limit;
    ++used;
    busy = true;
    return Attempt::Ok;
}

void PairingWindow::endAttempt(bool mismatch)
{
    busy = false;
    if (mismatch)
        close();
}

// ---- one exchange ----

PairingSession::PairingSession(Role r, Link *l, DeviceIdentity *s, PeerStore *p, PairingWindow *w, QObject *parent)
    : QObject(parent), role(r), link(l), self(s), peers(p), window(w)
{
    connect(link, &Link::frameReceived, this, &PairingSession::onFrame);
    connect(link, &Link::closed, this, [this] {
        if (!done)
            end(false, QStringLiteral("LINK"));
    });
    if (link->state() == Link::State::Ready)
        start();
    else
        connect(link, &Link::stateChanged, this, [this](Link::State st) {
            if (st == Link::State::Ready && step == 0)
                start();
        });
}

PairingSession::~PairingSession()
{
    if (attempting && window)
        window->endAttempt(false);
}

bool PairingSession::sane(const QString &name)
{
    if (name.isEmpty() || name.size() > 64)
        return false;
    for (const QChar c : name)
        if (c.unicode() < 32 || c.unicode() == 127)
            return false;
    return true;
}

void PairingSession::end(bool ok, const QString &reason)
{
    if (done)
        return;
    done = true;
    if (attempting && window) {
        window->endAttempt(!ok && reason == QLatin1String("MISMATCH"));
        attempting = false;
    }
    emit finished(ok, reason);
}

void PairingSession::abort(const QString &reason, bool mismatch)
{
    if (done)
        return;
    if (mismatch && attempting && window) {
        window->endAttempt(true);
        attempting = false;
    }
    end(false, reason); // (first: the close below would be reported as LINK)
    link->close(reason);
}

void PairingSession::start()
{
    step = 1;
    if (link->peer().trust != Trust::Pairing) {
        abort(QStringLiteral("NOT_PAIRED"));
        return;
    }
    if (role == Role::Host) {
        const PairingWindow::Attempt a = window ? window->beginAttempt() : PairingWindow::Attempt::Closed;
        if (a != PairingWindow::Attempt::Ok) {
            const QString code = a == PairingWindow::Attempt::Busy ? QStringLiteral("PAIRING_BUSY")
                                 : a == PairingWindow::Attempt::Limit ? QStringLiteral("PAIRING_LIMIT")
                                                                      : QStringLiteral("PAIRING_CLOSED");
            link->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_REFUSED")}, {QStringLiteral("code"), code}});
            abort(code);
            return;
        }
        attempting = true;
        nonceMine = randomBytes(32);
        commitment = pairingCommitment(self->pin(), link->peer().pin, nonceMine);
        link->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_COMMIT")},
                        {QStringLiteral("c"), QString::fromLatin1(commitment.toHex())},
                        {QStringLiteral("name"), self->name()},
                        {QStringLiteral("instance"), self->instanceId()}});
    }
}

void PairingSession::onFrame(const Frame &f)
{
    if (done)
        return;
    bool ok = false;
    const QJsonObject o = f.object(&ok);
    const QString t = o.value(QStringLiteral("t")).toString();
    if (!ok || step == 0) {
        abort(QStringLiteral("BAD_FRAME"));
        return;
    }
    if (t == QLatin1String("PAIR_REFUSED")) {
        const QString c = o.value(QStringLiteral("code")).toString();
        abort(c == QLatin1String("PAIRING_BUSY") || c == QLatin1String("PAIRING_LIMIT") ? c : QStringLiteral("PAIRING_CLOSED"));
        return;
    }
    if (role == Role::Guest && t == QLatin1String("PAIR_COMMIT") && step == 1) {
        const QByteArray c = QByteArray::fromHex(o.value(QStringLiteral("c")).toString().toLatin1());
        const QString name = o.value(QStringLiteral("name")).toString();
        const QString inst = o.value(QStringLiteral("instance")).toString();
        if (c.size() != 32 || !sane(name) || inst.size() != 16) {
            abort(QStringLiteral("BAD_FRAME"));
            return;
        }
        commitment = c;
        otherName = name;
        otherInstance = inst;
        nonceMine = randomBytes(32);
        step = 2;
        link->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_NONCE")},
                        {QStringLiteral("n"), QString::fromLatin1(nonceMine.toHex())},
                        {QStringLiteral("name"), self->name()},
                        {QStringLiteral("instance"), self->instanceId()}});
        return;
    }
    if (role == Role::Host && t == QLatin1String("PAIR_NONCE") && step == 1) {
        const QByteArray n = QByteArray::fromHex(o.value(QStringLiteral("n")).toString().toLatin1());
        const QString name = o.value(QStringLiteral("name")).toString();
        const QString inst = o.value(QStringLiteral("instance")).toString();
        if (n.size() != 32 || !sane(name) || inst.size() != 16) {
            abort(QStringLiteral("BAD_FRAME"));
            return;
        }
        nonceTheirs = n;
        otherName = name;
        otherInstance = inst;
        step = 2;
        link->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_OPEN")}, {QStringLiteral("n"), QString::fromLatin1(nonceMine.toHex())}});
        shownCode = pairingCode(self->pin(), link->peer().pin, nonceMine, nonceTheirs);
        emit codeReady(shownCode);
        return;
    }
    if (role == Role::Guest && t == QLatin1String("PAIR_OPEN") && step == 2) {
        const QByteArray n = QByteArray::fromHex(o.value(QStringLiteral("n")).toString().toLatin1());
        if (n.size() != 32 || pairingCommitment(link->peer().pin, self->pin(), n) != commitment) {
            abort(QStringLiteral("COMMITMENT"), true);
            return;
        }
        nonceTheirs = n; // (the host's)
        step = 3;
        shownCode = pairingCode(link->peer().pin, self->pin(), nonceTheirs, nonceMine);
        emit codeReady(shownCode);
        return;
    }
    if (t == QLatin1String("PAIR_CONFIRM") && step >= 2 && !theirsKnown) {
        theirs = o.value(QStringLiteral("ok")).toBool();
        theirsKnown = true;
        proceed();
        return;
    }
    abort(QStringLiteral("BAD_FRAME"));
}

void PairingSession::confirm(bool match)
{
    if (done || decided || shownCode.isEmpty())
        return;
    decided = true;
    mine = match;
    link->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_CONFIRM")}, {QStringLiteral("ok"), match}});
    proceed();
}

void PairingSession::proceed()
{
    if (done)
        return;
    if ((decided && !mine) || (theirsKnown && !theirs)) {
        // one "no" ends it; nothing is stored
        end(false, QStringLiteral("MISMATCH"));
        link->close(QStringLiteral("MISMATCH"));
        return;
    }
    if (!decided || !theirsKnown)
        return;
    PeerRecord r;
    r.pin = link->peer().pin;
    r.publicKey = link->peer().publicKey;
    r.name = otherName;
    r.instanceId = otherInstance;
    QString err;
    if (!peers || !peers->add(r, &err)) {
        end(false, QStringLiteral("STORE"));
        link->close(QStringLiteral("STORE"));
        return;
    }
    end(true, QString());
    link->close(); // (what was sent before arrives before the close)
}

} // namespace net
} // namespace collab
} // namespace qb
