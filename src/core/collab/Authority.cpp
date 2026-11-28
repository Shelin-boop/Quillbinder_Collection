#include "core/collab/Authority.h"

#include "core/SafeSave.h"
#include "core/collab/Election.h"
#include "core/collab/Handover.h"
#include "core/collab/net/DeviceIdentity.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

namespace qb {
namespace collab {

namespace {
constexpr int kMaxChain = 1000;
constexpr int kMaxVotes = 64;

QString kindName(Proof::Kind k)
{
    return k == Proof::Founding ? QStringLiteral("founding") : k == Proof::Handover ? QStringLiteral("handover") : QStringLiteral("failover");
}

bool isShortString(const QJsonValue &v, int max = 200) { return v.isString() && v.toString().toUtf8().size() <= max; }
bool isWhole(const QJsonValue &v) { return v.isDouble() && v.toDouble() >= 0 && v.toDouble() < 1e15; }

QByteArray readFile(const QString &path, bool *exists)
{
    QFile f(path);
    *exists = f.exists();
    if (!f.exists() || !f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}
} // namespace

QString authStateName(AuthState s)
{
    switch (s) {
    case AuthState::Follower: return QStringLiteral("follower");
    case AuthState::Active: return QStringLiteral("active");
    case AuthState::Draining: return QStringLiteral("draining");
    case AuthState::Prepared: return QStringLiteral("prepared");
    case AuthState::Retired: return QStringLiteral("retired");
    }
    return QStringLiteral("follower");
}

static bool authStateFromName(const QString &n, AuthState *s)
{
    for (AuthState v : {AuthState::Follower, AuthState::Active, AuthState::Draining, AuthState::Prepared, AuthState::Retired})
        if (authStateName(v) == n) {
            *s = v;
            return true;
        }
    return false;
}

// ---------------------------------------------------------------- proofs

QJsonObject Proof::toJson() const
{
    QJsonArray v;
    for (const Vote &x : votes)
        v.append(QJsonObject{{QStringLiteral("v"), x.voter}, {QStringLiteral("s"), QString::fromLatin1(x.signature.toBase64())}});
    QJsonObject o{{QStringLiteral("k"), kindName(kind)},
                  {QStringLiteral("p"), project},
                  {QStringLiteral("e"), double(epoch)},
                  {QStringLiteral("prev"), double(previous)},
                  {QStringLiteral("host"), host},
                  {QStringLiteral("signer"), signer},
                  {QStringLiteral("bs"), double(baseSeq)},
                  {QStringLiteral("bh"), baseHash},
                  {QStringLiteral("ld"), locksDigest},
                  {QStringLiteral("votes"), v}};
    if (!signature.isEmpty())
        o.insert(QStringLiteral("sig"), QString::fromLatin1(signature.toBase64()));
    return o;
}

bool Proof::fromJson(const QJsonObject &o, Proof *out, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why)
            *why = w;
        return false;
    };
    Proof p;
    const QString k = o.value(QStringLiteral("k")).toString();
    if (k == QLatin1String("founding"))
        p.kind = Founding;
    else if (k == QLatin1String("handover"))
        p.kind = Handover;
    else if (k == QLatin1String("failover"))
        p.kind = Failover;
    else
        return fail(QStringLiteral("unknown kind"));
    for (const char *f : {"p", "host", "signer", "bh", "ld"})
        if (!isShortString(o.value(QLatin1String(f)), 200))
            return fail(QStringLiteral("a field is no short string"));
    for (const char *f : {"e", "prev", "bs"})
        if (!isWhole(o.value(QLatin1String(f))))
            return fail(QStringLiteral("a number is wrong"));
    p.project = o.value(QStringLiteral("p")).toString();
    p.epoch = qint64(o.value(QStringLiteral("e")).toDouble());
    p.previous = qint64(o.value(QStringLiteral("prev")).toDouble());
    p.host = o.value(QStringLiteral("host")).toString();
    p.signer = o.value(QStringLiteral("signer")).toString();
    p.baseSeq = qint64(o.value(QStringLiteral("bs")).toDouble());
    p.baseHash = o.value(QStringLiteral("bh")).toString();
    p.locksDigest = o.value(QStringLiteral("ld")).toString();
    const QJsonValue vs = o.value(QStringLiteral("votes"));
    if (!vs.isArray() || vs.toArray().size() > kMaxVotes)
        return fail(QStringLiteral("votes are wrong"));
    for (const QJsonValue &v : vs.toArray()) {
        if (!v.isObject() || !isShortString(v.toObject().value(QStringLiteral("v")), 64) || !isShortString(v.toObject().value(QStringLiteral("s")), 400))
            return fail(QStringLiteral("a vote is wrong"));
        p.votes.append({v.toObject().value(QStringLiteral("v")).toString(), QByteArray::fromBase64(v.toObject().value(QStringLiteral("s")).toString().toLatin1())});
    }
    if (!isShortString(o.value(QStringLiteral("sig")), 400))
        return fail(QStringLiteral("no signature"));
    p.signature = QByteArray::fromBase64(o.value(QStringLiteral("sig")).toString().toLatin1());
    *out = p;
    return true;
}

QByteArray Proof::signingBytes() const
{
    QJsonObject o = toJson();
    o.remove(QStringLiteral("sig"));
    return QByteArrayLiteral("QB-LS-PROOF1\n") + QJsonDocument(o).toJson(QJsonDocument::Compact);
}

QByteArray voteBytes(const QString &project, qint64 epoch, const QString &candidate, qint64 baseSeq, const QString &baseHash, const QString &voter)
{
    const QJsonObject o{{QStringLiteral("k"), QStringLiteral("vote")}, {QStringLiteral("p"), project},     {QStringLiteral("e"), double(epoch)},
                        {QStringLiteral("c"), candidate},              {QStringLiteral("bs"), double(baseSeq)}, {QStringLiteral("bh"), baseHash},
                        {QStringLiteral("v"), voter}};
    return QByteArrayLiteral("QB-LS-VOTE1\n") + QJsonDocument(o).toJson(QJsonDocument::Compact);
}

int votesNeeded(int n) { return n <= 2 ? 0 : n / 2 + 1; }

QString locksDigestOf(const QJsonArray &locks)
{
    return QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(locks).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex());
}

static Proof signedProof(Proof p, const net::DeviceIdentity &who)
{
    p.signer = who.deviceId();
    p.signature = who.sign(p.signingBytes());
    return p;
}

Proof makeFounding(const QString &project, qint64 epoch, const net::DeviceIdentity &host)
{
    Proof p;
    p.kind = Proof::Founding;
    p.project = project;
    p.epoch = epoch;
    p.host = host.deviceId();
    return signedProof(p, host);
}

Proof makeHandover(const QString &project, qint64 previous, qint64 epoch, const QString &target, qint64 seq, const QString &hash,
                   const QString &locksDigest, const net::DeviceIdentity &oldHost)
{
    Proof p;
    p.kind = Proof::Handover;
    p.project = project;
    p.previous = previous;
    p.epoch = epoch;
    p.host = target;
    p.baseSeq = seq;
    p.baseHash = hash;
    p.locksDigest = locksDigest;
    return signedProof(p, oldHost);
}

Proof makeFailover(const QString &project, qint64 previous, qint64 epoch, qint64 baseSeq, const QString &baseHash, const QList<Vote> &votes,
                   const net::DeviceIdentity &newHost)
{
    Proof p;
    p.kind = Proof::Failover;
    p.project = project;
    p.previous = previous;
    p.epoch = epoch;
    p.host = newHost.deviceId();
    p.baseSeq = baseSeq;
    p.baseHash = baseHash;
    p.votes = votes;
    return signedProof(p, newHost);
}

ChainResult verifyChain(const QList<Proof> &chain, const Membership &members, const QList<Proof> &trusted)
{
    ChainResult r;
    auto fail = [&](const QString &w) {
        r.ok = false;
        r.why = w;
        return r;
    };
    if (chain.isEmpty() || chain.size() > kMaxChain)
        return fail(QStringLiteral("no chain"));
    if (!trusted.isEmpty() && !(chain.first() == trusted.first()))
        return fail(QStringLiteral("another founding"));
    qint64 curEpoch = 0;
    QString curHost;
    qint64 curSeq = 0;
    QString curHash;
    for (int i = 0; i < chain.size(); ++i) {
        const Proof &p = chain.at(i);
        if (p.project != members.project)
            return fail(QStringLiteral("another project"));
        const bool known = i < trusted.size() && trusted.at(i) == p;
        if (i == 0) {
            if (p.kind != Proof::Founding || p.signer != p.host)
                return fail(QStringLiteral("the chain does not start with a founding"));
        } else {
            if (p.kind == Proof::Founding)
                return fail(QStringLiteral("a second founding"));
            if (p.previous != curEpoch || p.epoch <= p.previous)
                return fail(QStringLiteral("the steps do not follow each other"));
        }
        if (!known) {
            const Member *signer = members.find(p.signer);
            const Member *host = members.find(p.host);
            if (!signer || !host)
                return fail(QStringLiteral("a device of the chain is no member"));
            if (!p.host.isEmpty() && !members.mayHost(p.host))
                return fail(QStringLiteral("the host may not host"));
            if (p.signature.isEmpty() || !net::DeviceIdentity::verify(signer->key, p.signingBytes(), p.signature))
                return fail(QStringLiteral("a signature does not fit"));
            if (p.kind == Proof::Handover) {
                if (p.signer != curHost || p.epoch != p.previous + 1 || p.host == curHost)
                    return fail(QStringLiteral("a handover that is not the host's"));
            } else if (p.kind == Proof::Failover) {
                if (p.signer != p.host || p.host == curHost)
                    return fail(QStringLiteral("a failover that is not the candidate's"));
                const int need = votesNeeded(members.count());
                QSet<QString> voters;
                for (const Vote &v : p.votes) {
                    const Member *vm = members.find(v.voter);
                    if (!vm || v.voter == curHost || voters.contains(v.voter))
                        continue;
                    if (!net::DeviceIdentity::verify(vm->key, voteBytes(p.project, p.epoch, p.host, p.baseSeq, p.baseHash, v.voter), v.signature))
                        continue;
                    voters.insert(v.voter);
                }
                if (need > 0 && (voters.size() < need || !voters.contains(p.host)))
                    return fail(QStringLiteral("not enough valid votes"));
            }
        }
        curEpoch = p.epoch;
        curHost = p.host;
        curSeq = p.baseSeq;
        curHash = p.baseHash;
    }
    r.ok = true;
    r.epoch = curEpoch;
    r.host = curHost;
    r.baseSeq = curSeq;
    r.baseHash = curHash;
    return r;
}

// ---------------------------------------------------------------- the record

QJsonObject AuthorityRecord::toJson() const
{
    QJsonArray c;
    for (const Proof &p : chain)
        c.append(p.toJson());
    QJsonObject v;
    for (auto it = votes.constBegin(); it != votes.constEnd(); ++it)
        v.insert(QString::number(it.key()), it.value());
    QJsonObject o{{QStringLiteral("v"), 1},
                  {QStringLiteral("p"), project},
                  {QStringLiteral("epoch"), double(epoch)},
                  {QStringLiteral("host"), host},
                  {QStringLiteral("state"), authStateName(state)},
                  {QStringLiteral("highest"), double(highest)},
                  {QStringLiteral("chain"), c},
                  {QStringLiteral("votes"), v},
                  {QStringLiteral("hoTarget"), hoTarget},
                  {QStringLiteral("hoEpoch"), double(hoEpoch)},
                  {QStringLiteral("hoSeq"), double(hoSeq)},
                  {QStringLiteral("hoHash"), hoHash},
                  {QStringLiteral("pendEpoch"), double(pendEpoch)},
                  {QStringLiteral("pendSeq"), double(pendSeq)},
                  {QStringLiteral("pendFrom"), pendFrom},
                  {QStringLiteral("pendHash"), pendHash}};
    if (hoHasProof)
        o.insert(QStringLiteral("hoProof"), hoProof.toJson());
    if (!hoLocks.isEmpty())
        o.insert(QStringLiteral("hoLocks"), hoLocks);
    return o;
}

bool AuthorityRecord::fromJson(const QJsonObject &o, AuthorityRecord *out, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why)
            *why = w;
        return false;
    };
    AuthorityRecord r;
    r.project = o.value(QStringLiteral("p")).toString();
    r.epoch = qint64(o.value(QStringLiteral("epoch")).toDouble());
    r.host = o.value(QStringLiteral("host")).toString();
    if (!authStateFromName(o.value(QStringLiteral("state")).toString(), &r.state))
        return fail(QStringLiteral("unknown state"));
    r.highest = qint64(o.value(QStringLiteral("highest")).toDouble());
    for (const QJsonValue &v : o.value(QStringLiteral("chain")).toArray()) {
        Proof p;
        if (!Proof::fromJson(v.toObject(), &p, why))
            return false;
        r.chain << p;
    }
    const QJsonObject votes = o.value(QStringLiteral("votes")).toObject();
    for (auto it = votes.constBegin(); it != votes.constEnd(); ++it)
        r.votes.insert(it.key().toLongLong(), it.value().toString());
    r.hoTarget = o.value(QStringLiteral("hoTarget")).toString();
    r.hoEpoch = qint64(o.value(QStringLiteral("hoEpoch")).toDouble());
    r.hoSeq = qint64(o.value(QStringLiteral("hoSeq")).toDouble());
    r.hoHash = o.value(QStringLiteral("hoHash")).toString();
    r.pendEpoch = qint64(o.value(QStringLiteral("pendEpoch")).toDouble());
    r.pendSeq = qint64(o.value(QStringLiteral("pendSeq")).toDouble());
    r.pendFrom = o.value(QStringLiteral("pendFrom")).toString();
    r.pendHash = o.value(QStringLiteral("pendHash")).toString();
    if (o.contains(QStringLiteral("hoProof"))) {
        if (!Proof::fromJson(o.value(QStringLiteral("hoProof")).toObject(), &r.hoProof, why))
            return false;
        r.hoHasProof = true;
    }
    r.hoLocks = o.value(QStringLiteral("hoLocks")).toArray();
    if (r.state == AuthState::Retired && !r.hoHasProof)
        return fail(QStringLiteral("retired without its proof"));
    *out = r;
    return true;
}

// ---------------------------------------------------------------- the controller

Authority::Authority(const Config &config, QObject *parent)
    : QObject(parent), m_cfg(config), m_drafts(config.localRoot), m_election(new Election(*this)), m_handover(new Handover(*this))
{
    m_rec.project = config.project;
    if (m_cfg.clock)
        connect(m_cfg.clock, &Clock::ticked, this, &Authority::tick);
}

Authority::~Authority() = default;

QString Authority::selfId() const { return m_cfg.identity ? m_cfg.identity->deviceId() : QString(); }

bool Authority::handoverRunning() const { return m_handover->running(); }

QString Authority::unknownDeviceWarning() const
{
    return QCoreApplication::translate("qb::collab", "An unknown device tried to take over hosting of '%1'. It was refused.").arg(m_cfg.title);
}

bool Authority::persist()
{
    const QString path = m_cfg.folder + QStringLiteral("/livesync/authority.json");
    QDir().mkpath(QFileInfo(path).absolutePath());
    QString err;
    if (!safesave::writeFile(path, QJsonDocument(m_rec.toJson()).toJson(QJsonDocument::Compact), &err)) {
        emit failed(err);
        return false;
    }
    return true;
}

bool Authority::persistMembership()
{
    const QString path = m_cfg.folder + QStringLiteral("/livesync/membership.json");
    QDir().mkpath(QFileInfo(path).absolutePath());
    QString err;
    if (!safesave::writeFile(path, QJsonDocument(m_members.toJson()).toJson(QJsonDocument::Compact), &err)) {
        emit failed(err);
        return false;
    }
    return true;
}

bool Authority::load(QString *error)
{
    auto fail = [&](const QString &w) {
        if (error)
            *error = w;
        return false;
    };
    bool exists = false;
    QByteArray b = readFile(m_cfg.folder + QStringLiteral("/livesync/membership.json"), &exists);
    if (exists) {
        QJsonParseError pe;
        const QJsonDocument d = QJsonDocument::fromJson(b, &pe);
        QString why;
        if (pe.error != QJsonParseError::NoError || !d.isObject() || !Membership::fromJson(d.object(), &m_members, &why))
            return fail(QStringLiteral("membership.json is damaged"));
    }
    b = readFile(m_cfg.folder + QStringLiteral("/livesync/authority.json"), &exists);
    if (exists) {
        QJsonParseError pe;
        const QJsonDocument d = QJsonDocument::fromJson(b, &pe);
        QString why;
        if (pe.error != QJsonParseError::NoError || !d.isObject() || !AuthorityRecord::fromJson(d.object(), &m_rec, &why))
            return fail(QStringLiteral("authority.json is damaged"));
        if (m_rec.project != m_cfg.project)
            return fail(QStringLiteral("authority.json belongs to another project"));
    }
    m_loaded = true;
    return true;
}

bool Authority::found(const Membership &signedMembership, qint64 epoch, QString *error)
{
    auto fail = [&](const QString &w) {
        if (error)
            *error = w;
        return false;
    };
    if (m_dead || !m_cfg.identity)
        return fail(QStringLiteral("no identity"));
    if (!m_rec.chain.isEmpty())
        return fail(QStringLiteral("the project is shared already"));
    QString why;
    if (signedMembership.project != m_cfg.project || !signedMembership.signatureValid(&why) || !signedMembership.isMember(selfId()))
        return fail(QStringLiteral("the membership does not fit: ") + why);
    m_members = signedMembership;
    const Proof p = makeFounding(m_cfg.project, epoch, *m_cfg.identity);
    m_rec.epoch = epoch;
    m_rec.host = selfId();
    m_rec.highest = epoch;
    m_rec.chain = {p};
    m_rec.state = AuthState::Active;
    if (!persistMembership() || !persist())
        return fail(QStringLiteral("the files of Live Sync cannot be written"));
    m_started = true;
    resetHostContact();
    QString e;
    if (!m_cfg.backend->startHosting(epoch, QJsonArray(), &e))
        return fail(e);
    emit hostChanged(m_rec.host, m_rec.epoch);
    return true;
}

bool Authority::joinWith(const QList<Proof> &chain, const Membership &signedMembership, QString *error)
{
    auto fail = [&](const QString &w) {
        if (error)
            *error = w;
        return false;
    };
    QString why;
    if (signedMembership.project != m_cfg.project || !signedMembership.signatureValid(&why) || !signedMembership.isMember(selfId()))
        return fail(QStringLiteral("the membership does not fit: ") + why);
    const ChainResult r = verifyChain(chain, signedMembership);
    if (!r.ok)
        return fail(r.why);
    m_members = signedMembership;
    m_rec.epoch = r.epoch;
    m_rec.host = r.host;
    m_rec.highest = r.epoch;
    m_rec.chain = chain;
    m_rec.state = AuthState::Follower;
    if (!persistMembership() || !persist())
        return fail(QStringLiteral("the files of Live Sync cannot be written"));
    return true;
}

void Authority::resetHostContact() { m_lastHostContact = m_cfg.clock ? m_cfg.clock->now() : 0; }

void Authority::start()
{
    if (m_dead || m_rec.chain.isEmpty() || m_started) // (found() has started already)
        return;
    m_started = true;
    resetHostContact();
    const bool self = m_rec.host == selfId();
    switch (m_rec.state) {
    case AuthState::Active:
    case AuthState::Draining:
    case AuthState::Prepared:
        if (self) {
            // a host that was handing over comes back as the host (the handover is aborted); the target answers for itself
            m_rec.state = AuthState::Active;
            m_rec.hoTarget.clear();
            m_rec.hoEpoch = m_rec.hoSeq = 0;
            m_rec.hoHash.clear();
            m_rec.hoLocks = QJsonArray();
            if (!persist())
                return;
            QString e;
            if (!m_cfg.backend->startHosting(m_rec.epoch, QJsonArray(), &e))
                emit failed(e);
            return;
        }
        // PREPARED as a target without its proof: FOLLOWER (the old host resends the proof if it retired)
        m_rec.state = AuthState::Follower;
        m_rec.pendEpoch = m_rec.pendSeq = 0;
        m_rec.pendFrom.clear();
        m_rec.pendHash.clear();
        persist();
        m_cfg.backend->followHost(m_rec.host, m_rec.epoch);
        return;
    case AuthState::Retired:
        // never hosts this epoch again; the tick resends the proof until the target says it is done
        m_cfg.backend->followHost(m_rec.hoProof.host, m_rec.hoProof.epoch);
        return;
    case AuthState::Follower:
        if (m_rec.pendEpoch) {
            m_rec.pendEpoch = m_rec.pendSeq = 0;
            m_rec.pendFrom.clear();
            m_rec.pendHash.clear();
            persist();
        }
        m_cfg.backend->followHost(m_rec.host, m_rec.epoch);
        return;
    }
}

void Authority::setState(AuthState s)
{
    if (m_rec.state == s)
        return;
    m_rec.state = s;
    emit stateChanged(s);
}

qint64 Authority::floorEpoch() const { return qMax(qMax(m_rec.epoch, m_rec.pendEpoch), m_rec.hoEpoch); }

bool Authority::killed(const char *point)
{
    if (m_dead)
        return true;
    if (m_kill && m_kill(QLatin1String(point))) {
        m_dead = true;
        return true;
    }
    return false;
}

bool Authority::sendTo(const QString &deviceId, const QJsonObject &m)
{
    if (m_dead || !m_cfg.send)
        return false;
    return m_cfg.send(deviceId, m);
}

void Authority::sendAuthorityTo(const QString &deviceId)
{
    if (m_rec.chain.isEmpty())
        return;
    QJsonArray c;
    for (const Proof &p : m_rec.chain)
        c.append(p.toJson());
    sendTo(deviceId, proto::make(proto::T::AUTHORITY, m_cfg.project, {{QStringLiteral("chain"), c}, {QStringLiteral("membership"), m_members.toJson()}}));
}

void Authority::broadcastAuthority()
{
    for (const QString &d : m_online)
        sendAuthorityTo(d);
}

bool Authority::refuse(const QString &from, const QJsonObject &m, const QString &code, const QString &reason, bool unknown)
{
    sendTo(from, proto::errorMessage(code, m.value(QStringLiteral("id")).toString(), reason, m_cfg.project));
    if (unknown)
        emit unknownDeviceRefused(from, reason);
    else
        emit hostingClaimRefused(from, code);
    return true;
}

void Authority::peerConnected(const QString &deviceId)
{
    if (m_dead)
        return;
    m_online.insert(deviceId);
    if (deviceId == m_rec.host)
        resetHostContact();
    if (m_inSync.contains(deviceId) && !m_syncSince.contains(deviceId) && m_cfg.clock)
        m_syncSince.insert(deviceId, m_cfg.clock->now());
    if (m_members.isMember(deviceId))
        sendAuthorityTo(deviceId);
}

void Authority::peerDisconnected(const QString &deviceId)
{
    m_online.remove(deviceId);
    m_syncSince.remove(deviceId);
}

void Authority::heardFrom(const QString &deviceId)
{
    if (m_dead)
        return;
    if (deviceId == m_rec.host && m_cfg.clock)
        m_lastHostContact = m_cfg.clock->now();
}

void Authority::setPeerInSync(const QString &deviceId, bool inSync)
{
    if (inSync) {
        m_inSync.insert(deviceId);
        if (m_online.contains(deviceId) && !m_syncSince.contains(deviceId) && m_cfg.clock)
            m_syncSince.insert(deviceId, m_cfg.clock->now());
    } else {
        m_inSync.remove(deviceId);
        m_syncSince.remove(deviceId);
    }
}

bool Authority::handleMessage(const QString &from, const QJsonObject &m)
{
    const QString type = m.value(QStringLiteral("t")).toString();
    const bool ours = proto::is(type, proto::T::AUTHORITY) || proto::is(type, proto::T::MEMBERSHIP) || proto::is(type, proto::T::ELECTION_REQUEST) ||
                      proto::is(type, proto::T::VOTE) || proto::is(type, proto::T::HANDOVER_PREPARE) || proto::is(type, proto::T::HANDOVER_READY) ||
                      proto::is(type, proto::T::HANDOVER_COMMIT) || proto::is(type, proto::T::HANDOVER_DONE) || proto::is(type, proto::T::HANDOVER_ABORT);
    if (!ours)
        return false;
    if (m_dead)
        return true;
    if (m.value(QStringLiteral("p")).toString() != m_cfg.project)
        return false;
    heardFrom(from);
    // only members take part in the hosting of a project; a device that is none claims something it has no right to
    if (!m_members.isMember(from))
        return refuse(from, m, QStringLiteral("UNKNOWN_DEVICE"), QStringLiteral("not a member"), true);
    if (proto::is(type, proto::T::AUTHORITY))
        onAuthority(from, m);
    else if (proto::is(type, proto::T::MEMBERSHIP))
        onMembership(from, m);
    else if (proto::is(type, proto::T::ELECTION_REQUEST))
        m_election->onRequest(from, m);
    else if (proto::is(type, proto::T::VOTE))
        m_election->onVote(from, m);
    else if (proto::is(type, proto::T::HANDOVER_PREPARE))
        m_handover->onPrepare(from, m);
    else if (proto::is(type, proto::T::HANDOVER_READY))
        m_handover->onReady(from, m);
    else if (proto::is(type, proto::T::HANDOVER_COMMIT))
        m_handover->onCommit(from, m);
    else if (proto::is(type, proto::T::HANDOVER_DONE))
        m_handover->onDone(from, m);
    else
        m_handover->onAbort(from, m);
    return true;
}

void Authority::onMembership(const QString &from, const QJsonObject &m)
{
    Membership next;
    QString why;
    if (!Membership::fromJson(m.value(QStringLiteral("membership")).toObject(), &next, &why)) {
        refuse(from, m, QStringLiteral("BAD_OP"), why, false);
        return;
    }
    if (next.version <= m_members.version)
        return;
    if (!m_members.acceptsSuccessor(next, m_rec.host, &why)) {
        refuse(from, m, QStringLiteral("UNKNOWN_DEVICE"), why, true);
        return;
    }
    m_members = next;
    if (persistMembership())
        emit membershipChanged();
}

void Authority::onAuthority(const QString &from, const QJsonObject &m)
{
    const QJsonArray arr = m.value(QStringLiteral("chain")).toArray();
    if (arr.isEmpty() || arr.size() > kMaxChain) {
        refuse(from, m, QStringLiteral("BAD_OP"), QStringLiteral("no chain"), false);
        return;
    }
    QList<Proof> chain;
    for (const QJsonValue &v : arr) {
        Proof p;
        QString why;
        if (!Proof::fromJson(v.toObject(), &p, &why)) {
            refuse(from, m, QStringLiteral("BAD_OP"), why, false);
            return;
        }
        chain << p;
    }
    // a newer membership that comes with the announcement: only when a member of ours signed it and it is the host's
    if (m.contains(QStringLiteral("membership"))) {
        Membership next;
        QString why;
        if (Membership::fromJson(m.value(QStringLiteral("membership")).toObject(), &next, &why) && next.version > m_members.version &&
            m_members.acceptsSuccessor(next, QString(), &why) && (next.signer == chain.last().host || next.signer == m_rec.host)) {
            m_members = next;
            if (persistMembership())
                emit membershipChanged();
        }
    }
    const ChainResult r = verifyChain(chain, m_members, m_rec.chain);
    if (!r.ok) {
        refuse(from, m, QStringLiteral("UNKNOWN_DEVICE"), r.why, true);
        return;
    }
    if (r.epoch > m_rec.epoch) {
        adopt(chain, r, from);
        return;
    }
    if (r.epoch == m_rec.epoch && r.host != m_rec.host) {
        // two hosts in one epoch cannot come from the rules; if it happens the priority decides and it is told
        emit epochConflict(m_rec.host, r.host);
        if (m_members.priorityIndex(r.host) < m_members.priorityIndex(m_rec.host)) {
            adopt(chain, r, from);
            return;
        }
    }
    if (r.epoch < m_rec.epoch || (r.epoch == m_rec.epoch && r.host != m_rec.host)) {
        const QString key = from + QLatin1Char('|') + QString::number(m_rec.epoch);
        if (!m_answeredAt.contains(key)) {
            m_answeredAt.insert(key, 1);
            sendAuthorityTo(from);
        }
    }
}

bool Authority::adopt(const QList<Proof> &chain, const ChainResult &r, const QString &from)
{
    Q_UNUSED(from);
    const bool wasHost = isHost();
    const qint64 oldEpoch = m_rec.epoch;
    const qint64 forkSeq = r.baseSeq;
    HostingBackend *b = m_cfg.backend;
    if (wasHost) {
        b->setAcceptingCommits(false);
        b->setAcceptingLocks(false);
        QString e;
        if (!b->backupBeforeRejoin(&e)) // the drafts below protect the edits; a missing backup is told
            emit failed(e.isEmpty() ? QStringLiteral("the backup before rejoining failed") : e);
    }
    // everything this device authored after the fork becomes a draft, one per document, before anything is replaced
    QString err;
    const qint64 now = m_cfg.clock ? m_cfg.clock->wallMs() : 0;
    int made = makeSplitDrafts(m_drafts, m_cfg.folder, m_cfg.project, selfId(), b->authoredCommits(forkSeq, r.epoch), oldEpoch, forkSeq,
                               QStringLiteral("split"), now, &err);
    if (made < 0) {
        emit failed(err);
        if (wasHost) { // the drafts could not be kept: keep hosting what we have, nothing was lost
            b->setAcceptingCommits(true);
            b->setAcceptingLocks(true);
        }
        return false;
    }
    if (wasHost)
        made += b->outboxToDrafts(QStringLiteral("split"));
    if (killed("adopt-drafted"))
        return false;
    AuthorityRecord next = m_rec;
    next.epoch = r.epoch;
    next.host = r.host;
    next.highest = qMax(next.highest, r.epoch);
    next.chain = chain;
    next.state = AuthState::Follower;
    next.hoTarget.clear();
    next.hoEpoch = next.hoSeq = 0;
    next.hoHash.clear();
    next.hoHasProof = false;
    next.hoLocks = QJsonArray();
    next.pendEpoch = next.pendSeq = 0;
    next.pendFrom.clear();
    next.pendHash.clear();
    const AuthorityRecord before = m_rec;
    m_rec = next;
    if (!persist()) {
        m_rec = before;
        if (wasHost) {
            b->setAcceptingCommits(true);
            b->setAcceptingLocks(true);
        }
        return false;
    }
    if (killed("adopt-persisted"))
        return false;
    if (wasHost)
        b->stopHosting();
    m_election->cancel();
    m_handover->reset();
    resetHostContact();
    m_handbackBlockedUntil = 0;
    b->followHost(r.host, r.epoch);
    b->resyncFromHost();
    b->reunionDone(forkSeq, r.epoch);
    emit stateChanged(m_rec.state);
    emit hostChanged(r.host, r.epoch);
    if (made > 0 || wasHost)
        emit reunited(made, r.host);
    broadcastAuthority();
    return true;
}

bool Authority::publishMembership(QList<Member> members, QString *error)
{
    auto fail = [&](const QString &w) {
        if (error)
            *error = w;
        return false;
    };
    if (!isHost() || !m_cfg.identity)
        return fail(QStringLiteral("only the host writes the membership"));
    Membership next;
    next.project = m_cfg.project;
    next.version = m_members.version + 1;
    next.members = members;
    next.signWith(*m_cfg.identity);
    QString why;
    if (!next.signatureValid(&why) || !m_members.acceptsSuccessor(next, selfId(), &why))
        return fail(why);
    const Membership before = m_members;
    m_members = next;
    if (!persistMembership()) {
        m_members = before;
        return fail(QStringLiteral("the membership cannot be written"));
    }
    for (const QString &d : m_online)
        if (m_members.isMember(d))
            sendTo(d, proto::make(proto::T::MEMBERSHIP, m_cfg.project, {{QStringLiteral("membership"), m_members.toJson()}}));
    emit membershipChanged();
    return true;
}

bool Authority::transferHosting(const QString &target, QString *why) { return m_handover->start(target, why); }
void Authority::abortHandover(const QString &reason) { m_handover->abort(reason, true); }

void Authority::tick()
{
    if (m_dead || !m_started || !m_cfg.clock)
        return;
    m_handover->tick();
    if (m_dead)
        return;
    m_election->tick();
    if (m_dead)
        return;
    checkHandback();
}

void Authority::checkHandback()
{
    if (m_rec.state != AuthState::Active || m_rec.host != selfId() || m_handover->running())
        return;
    const qint64 now = m_cfg.clock->now();
    if (now < m_handbackBlockedUntil)
        return;
    const QStringList cands = m_members.hostCandidates();
    if (cands.isEmpty() || cands.first() == selfId())
        return;
    const QString top = cands.first();
    if (!m_online.contains(top) || !m_syncSince.contains(top) || now - m_syncSince.value(top) < kHandbackMs)
        return;
    if (m_cfg.backend->busy())
        return;
    QString why;
    if (!m_handover->start(top, &why))
        m_handbackBlockedUntil = now + kHandbackMs;
}

} // namespace collab
} // namespace qb
