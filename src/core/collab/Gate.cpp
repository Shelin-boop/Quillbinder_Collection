#include "core/collab/Gate.h"

#include "core/TestSwitches.h"
#include "core/crypto/Vault.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSaveFile>

#include <cstdio>
#include <cstdlib>

namespace qb {
namespace collab {

// ---------------------------------------------------------------- Follower

Follower::Follower() = default;
Follower::~Follower() { gate().detach(this); }

int Follower::refusedCount() const
{
    QMutexLocker l(&m_lock);
    return int(m_refused.size());
}

QStringList Follower::refusedPaths() const
{
    QMutexLocker l(&m_lock);
    return m_refused;
}

void Follower::setReadOnly(bool on, const QString &reason)
{
    QMutexLocker l(&m_lock);
    m_readOnly = on;
    m_readOnlyReason = on ? reason : QString();
}

void Follower::setHeldElsewhere(const QString &resource, const QString &device)
{
    QMutexLocker l(&m_lock);
    m_held.insert(resource, device);
}

void Follower::clearHeld(const QString &resource)
{
    QMutexLocker l(&m_lock);
    m_held.remove(resource);
}

QString Follower::whyNotChangeable(const QString &resource) const
{
    QMutexLocker l(&m_lock);
    if (m_readOnly)
        return m_readOnlyReason.isEmpty()
                   ? QCoreApplication::translate("qb::collab::Follower", "this device has read-only access to the project")
                   : m_readOnlyReason;
    // a document's lock also covers what is in it; the project-wide lock covers everything
    for (const QString &r : {resource, QStringLiteral("project")}) {
        auto it = m_held.constFind(r);
        if (it != m_held.constEnd())
            return QCoreApplication::translate("qb::collab::Follower", "it is being edited on %1").arg(*it);
    }
    return QString();
}

bool Follower::canChange(const QString &resource) const { return whyNotChangeable(resource).isEmpty(); }

QString Follower::whyNotEditable(const QString &id, bool notes) const
{
    Q_UNUSED(notes);
    return whyNotChangeable(QStringLiteral("doc:") + id);
}

// ---------------------------------------------------------------- Gate

// ---------------------------------------------------------------- service scope

static thread_local int t_serviceDepth = 0;

ServiceScope::ServiceScope() { ++t_serviceDepth; }
ServiceScope::~ServiceScope() { --t_serviceDepth; }
bool ServiceScope::active() { return t_serviceDepth > 0; }

WindowScope::WindowScope() : m_saved(t_serviceDepth) { t_serviceDepth = 0; }
WindowScope::~WindowScope() { t_serviceDepth = m_saved; }

Gate &gate()
{
    static Gate g;
    return g;
}

static QString cleanAbsolute(const QString &p) { return QDir::cleanPath(QFileInfo(p).absoluteFilePath()); }

void Gate::attach(Follower *f, const QString &folder)
{
    if (!f)
        return;
    {
        QMutexLocker l(&m_lock);
        f->m_folder = cleanAbsolute(folder);
        if (!m_followers.contains(f))
            m_followers.append(f);
        m_count.store(int(m_followers.size()));
    }
}

void Gate::detach(Follower *f)
{
    QMutexLocker l(&m_lock);
    m_followers.removeAll(f);
    m_count.store(int(m_followers.size()));
}

Follower *Gate::followerOfFolder(const QString &projectFolder) const
{
    if (!active())
        return nullptr;
    const QString c = cleanAbsolute(projectFolder);
    QMutexLocker l(&m_lock);
    for (Follower *f : m_followers)
        if (f->m_folder == c)
            return f;
    return nullptr;
}

Gate::Decision Gate::decide(const QString &path, bool tree) const
{
    Decision d;
    if (!active() || path.isEmpty() || t_serviceDepth > 0)
        return d;
    const QString c = cleanAbsolute(path);
    QMutexLocker l(&m_lock);
    for (Follower *f : m_followers) {
        if (!c.startsWith(f->m_folder + QLatin1Char('/')))
            continue;
        // the innermost folder wins
        if (d.follower && d.follower->m_folder.size() >= f->m_folder.size())
            continue;
        d.follower = f;
        d.relative = c.mid(f->m_folder.size() + 1);
    }
    if (!d.follower)
        return d;
    d.info = classify(tree ? d.relative + QStringLiteral("/_") : d.relative);
    d.verdict = d.info.kind == PathKind::DeviceLocal || d.info.kind == PathKind::Internal ? Verdict::Pass
                : d.info.kind == PathKind::Unknown                                          ? Verdict::Refused
                                                                                            : Verdict::Captured;
    return d;
}

void Gate::refuse(const Decision &d, const QString &path)
{
    const bool classless = d.verdict == Verdict::Refused;
    if (d.follower) {
        QMutexLocker l(&d.follower->m_lock);
        d.follower->m_refused << d.relative;
    }
    std::fprintf(stderr, "Quillbinder: a write into a shared project's folder was refused (%s): %s\n",
                 classless ? "a path of no class" : "it did not go through the gate", qPrintable(d.relative.isEmpty() ? path : d.relative));
    if (QB_TEST_ENV("QB_LS_STRICT") == QLatin1String("1"))
        std::abort();
}

Verdict Gate::write(const QString &path, const QByteArray &plain)
{
    const Decision d = decide(path);
    if (d.verdict == Verdict::Captured)
        d.follower->capture().put(d.relative, plain, d.info);
    else if (d.verdict == Verdict::Refused)
        refuse(d, path);
    return d.verdict;
}

Verdict Gate::remove(const QString &path)
{
    const Decision d = decide(path);
    if (d.verdict == Verdict::Captured)
        d.follower->capture().remove(d.relative, d.info);
    else if (d.verdict == Verdict::Refused)
        refuse(d, path);
    return d.verdict;
}

Verdict Gate::removeTree(const QString &path)
{
    const Decision d = decide(path, true);
    if (d.verdict == Verdict::Captured)
        d.follower->capture().removeTree(d.relative, d.info);
    else if (d.verdict == Verdict::Refused)
        refuse(d, path);
    return d.verdict;
}

Verdict Gate::copy(const QString &from, const QString &to, QString *error)
{
    const Decision d = decide(to);
    if (d.verdict != Verdict::Captured) {
        if (d.verdict == Verdict::Refused)
            refuse(d, to);
        return d.verdict;
    }
    QByteArray data;
    QString why;
    if (!vault::readFile(from, &data, &why)) {
        if (error)
            *error = why;
        return Verdict::Refused;
    }
    d.follower->capture().put(d.relative, data, d.info);
    return Verdict::Captured;
}

bool Gate::sealAllowed(const QString &path, QString *error)
{
    const Decision d = decide(path);
    if (d.verdict == Verdict::Pass)
        return true;
    refuse(d, path);
    if (error)
        *error = QCoreApplication::translate("qb::collab::Gate", "%1 belongs to a shared project and cannot be written on this device.")
                     .arg(QDir::toNativeSeparators(path));
    return false;
}

void Gate::noteCapturedSave(const QSaveFile *f)
{
    QMutexLocker l(&m_lock);
    m_captured.insert(f);
}

bool Gate::takeCapturedSave(const QSaveFile *f)
{
    if (!active())
        return false;
    QMutexLocker l(&m_lock);
    return m_captured.remove(f);
}

} // namespace collab
} // namespace qb
