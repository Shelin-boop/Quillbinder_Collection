#include "core/collab/Membership.h"

#include "core/collab/net/DeviceIdentity.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>

namespace qb {
namespace collab {

namespace {
constexpr int kMaxMembers = 64;
constexpr int kMaxNameBytes = 200;
constexpr int kMaxKeyBytes = 512;
} // namespace

QByteArray Member::pin() const { return net::DeviceIdentity::pinOfPublicKey(key); }

const Member *Membership::find(const QString &deviceId) const
{
    for (const Member &m : members)
        if (m.id == deviceId)
            return &m;
    return nullptr;
}

bool Membership::mayHost(const QString &deviceId) const
{
    const Member *m = find(deviceId);
    return m && m->mayHost && m->editor;
}

int Membership::priorityIndex(const QString &deviceId) const
{
    for (int i = 0; i < members.size(); ++i)
        if (members.at(i).id == deviceId)
            return i;
    return -1;
}

QStringList Membership::hostCandidates(const QString &except) const
{
    QStringList out;
    for (const Member &m : members)
        if (m.mayHost && m.editor && m.id != except)
            out << m.id;
    return out;
}

QJsonObject Membership::toJson() const
{
    QJsonArray list;
    for (const Member &m : members)
        list.append(QJsonObject{{QStringLiteral("id"), m.id},
                                {QStringLiteral("key"), QString::fromLatin1(m.key.toBase64())},
                                {QStringLiteral("name"), m.name},
                                {QStringLiteral("role"), m.editor ? QStringLiteral("editor") : QStringLiteral("reader")},
                                {QStringLiteral("mayHost"), m.mayHost}});
    QJsonObject o{{QStringLiteral("p"), project},
                  {QStringLiteral("version"), double(version)},
                  {QStringLiteral("members"), list},
                  {QStringLiteral("signer"), signer}};
    if (!signature.isEmpty())
        o.insert(QStringLiteral("sig"), QString::fromLatin1(signature.toBase64()));
    return o;
}

bool Membership::fromJson(const QJsonObject &o, Membership *out, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why)
            *why = w;
        return false;
    };
    Membership m;
    if (!o.value(QStringLiteral("p")).isString() || o.value(QStringLiteral("p")).toString().isEmpty() || o.value(QStringLiteral("p")).toString().size() > 64)
        return fail(QStringLiteral("no project"));
    m.project = o.value(QStringLiteral("p")).toString();
    const QJsonValue ver = o.value(QStringLiteral("version"));
    if (!ver.isDouble() || ver.toDouble() < 0 || ver.toDouble() > 1e12)
        return fail(QStringLiteral("no version"));
    m.version = qint64(ver.toDouble());
    if (!o.value(QStringLiteral("members")).isArray())
        return fail(QStringLiteral("no members"));
    const QJsonArray list = o.value(QStringLiteral("members")).toArray();
    if (list.isEmpty() || list.size() > kMaxMembers)
        return fail(QStringLiteral("wrong number of members"));
    QSet<QString> seen;
    for (const QJsonValue &v : list) {
        if (!v.isObject())
            return fail(QStringLiteral("a member is no object"));
        const QJsonObject mo = v.toObject();
        Member mem;
        mem.id = mo.value(QStringLiteral("id")).toString();
        mem.key = QByteArray::fromBase64(mo.value(QStringLiteral("key")).toString().toLatin1());
        mem.name = mo.value(QStringLiteral("name")).toString();
        const QString role = mo.value(QStringLiteral("role")).toString();
        if (role != QLatin1String("editor") && role != QLatin1String("reader"))
            return fail(QStringLiteral("unknown role"));
        mem.editor = role == QLatin1String("editor");
        mem.mayHost = mo.value(QStringLiteral("mayHost")).toBool();
        if (mem.key.isEmpty() || mem.key.size() > kMaxKeyBytes || mem.name.toUtf8().size() > kMaxNameBytes)
            return fail(QStringLiteral("a member's key or name is wrong"));
        if (net::DeviceIdentity::deviceIdOfPin(mem.pin()) != mem.id || mem.id.isEmpty())
            return fail(QStringLiteral("a member's id does not fit its key"));
        if (seen.contains(mem.id))
            return fail(QStringLiteral("a member twice"));
        seen.insert(mem.id);
        m.members.append(mem);
    }
    m.signer = o.value(QStringLiteral("signer")).toString();
    if (o.contains(QStringLiteral("sig")))
        m.signature = QByteArray::fromBase64(o.value(QStringLiteral("sig")).toString().toLatin1());
    *out = m;
    return true;
}

QByteArray Membership::signingBytes() const
{
    QJsonObject o = toJson();
    o.remove(QStringLiteral("sig"));
    return QByteArrayLiteral("QB-LS-MEMBERS1\n") + QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void Membership::signWith(const net::DeviceIdentity &signerIdentity)
{
    signer = signerIdentity.deviceId();
    signature = signerIdentity.sign(signingBytes());
}

bool Membership::signatureValid(QString *why) const
{
    const Member *s = find(signer);
    if (!s) {
        if (why)
            *why = QStringLiteral("the signer is no member");
        return false;
    }
    if (signature.isEmpty() || !net::DeviceIdentity::verify(s->key, signingBytes(), signature)) {
        if (why)
            *why = QStringLiteral("the signature does not fit");
        return false;
    }
    return true;
}

bool Membership::acceptsSuccessor(const Membership &next, const QString &requiredSigner, QString *why) const
{
    auto fail = [&](const QString &w) {
        if (why)
            *why = w;
        return false;
    };
    if (next.project != project)
        return fail(QStringLiteral("another project"));
    if (next.version <= version)
        return fail(QStringLiteral("not newer"));
    if (!next.signatureValid(why))
        return false;
    if (!find(next.signer))
        return fail(QStringLiteral("the signer is no member of the record it replaces"));
    if (!requiredSigner.isEmpty() && next.signer != requiredSigner)
        return fail(QStringLiteral("not signed by the host"));
    // the key a member is known by never changes behind our back
    for (const Member &m : members) {
        const Member *n = next.find(m.id);
        if (n && n->key != m.key)
            return fail(QStringLiteral("a member's key changed"));
    }
    return true;
}

} // namespace collab
} // namespace qb
