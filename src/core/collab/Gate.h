#pragma once
// Live Sync, the write gate (0.99.0). While a project of a shared folder is open in a window, that window is a
// follower: it never writes the shared files. Every write that reaches safesave, savebatch or the vault, and every
// remove helper, asks gate() first. For a follower's folder the write is captured (plain bytes, collab/Capture.h) and
// nothing reaches the disk; a path of no class is refused and counted (a program started with QB_LS_STRICT=1 in a test
// build aborts instead, so that a write path nobody knew of fails the suites). Files that belong to this device only
// (settings/ui-*.json, the lock and its marks, the journal, Live Sync's own folders) pass. With no follower attached
// every call returns at once: Live Sync off changes nothing.
#include "core/collab/Capture.h"
#include "core/collab/PathClass.h"

#include <QHash>
#include <QMutex>
#include <QSet>
#include <QString>
#include <QStringList>

#include <atomic>

class QSaveFile;

namespace qb {
namespace collab {

// What a window of a shared project knows about the project's state. The service's session classes derive from it
// (locks, roles); this base answers from what it is told, which also serves the tests.
class Follower
{
public:
    Follower();
    virtual ~Follower();
    Follower(const Follower &) = delete;
    Follower &operator=(const Follower &) = delete;

    QString folder() const { return m_folder; } // set by Gate::attach (Project::open)
    Capture &capture() { return m_capture; }

    // writes the gate refused (a path of no class) and which
    int refusedCount() const;
    QStringList refusedPaths() const;

    // This device has the Reader role, or the project is not writable here for another reason (text of the reason: the
    // part of a sentence that follows "cannot be changed here:").
    void setReadOnly(bool on, const QString &reason = QString());
    bool isReadOnly() const { return m_readOnly; }
    // a resource ("doc:<id>", "structure", "settings", "object:<oid>", "mindmap:<id>", "project") locked by another device
    void setHeldElsewhere(const QString &resource, const QString &deviceName);
    void clearHeld(const QString &resource);
    // why the text or notes of the document may not be changed here ("" = it may): part of a sentence
    virtual QString whyNotEditable(const QString &documentId, bool notes) const;
    // whether this device may change the resource now (drives the fields and menus that would change it)
    virtual bool canChange(const QString &resource) const;
    // the reason canChange gave false ("" when it is true)
    virtual QString whyNotChangeable(const QString &resource) const;

private:
    friend class Gate;
    QString m_folder;
    Capture m_capture;
    mutable QMutex m_lock;
    QStringList m_refused;
    bool m_readOnly = false;
    QString m_readOnlyReason;
    QHash<QString, QString> m_held; // resource -> device name
};

// The service of a program (collab/Service.h) writes the shared files itself: the host's commits and a replica's events.
// When a window of the same folder lives in the same process (Android, the tests), those writes must not be captured as
// the window's own. The service marks the stretch of its own work with a ServiceScope: inside it Gate::decide() passes
// everything. The mark belongs to the thread and nests. A window that is called from inside the service's work (a slot
// of one of its signals) steps out with WindowScope for the time of its own writes, so that what it writes is captured.
class ServiceScope
{
public:
    ServiceScope();
    ~ServiceScope();
    ServiceScope(const ServiceScope &) = delete;
    ServiceScope &operator=(const ServiceScope &) = delete;
    static bool active();
};
class WindowScope
{
public:
    WindowScope();
    ~WindowScope();
    WindowScope(const WindowScope &) = delete;
    WindowScope &operator=(const WindowScope &) = delete;

private:
    int m_saved;
};

enum class Verdict {
    Pass,     // not a follower's folder, or a file of this device only: the caller goes on
    Captured, // kept in the follower's change set: the caller treats the write as done
    Refused   // a path of no class: the caller treats the write as failed
};

class Gate
{
public:
    // some follower is attached
    bool active() const noexcept { return m_count.load(std::memory_order_relaxed) > 0; }

    void attach(Follower *follower, const QString &projectFolder);
    void detach(Follower *follower);

    struct Decision {
        Verdict verdict = Verdict::Pass;
        Follower *follower = nullptr;
        QString relative;
        PathInfo info;
    };
    // the verdict for a write of `path`, without keeping anything
    // tree: `path` is a folder (classified as a file inside it)
    Decision decide(const QString &path, bool tree = false) const;

    Verdict write(const QString &path, const QByteArray &plain);
    Verdict remove(const QString &path);
    Verdict removeTree(const QString &path);
    // a copy: the source is read (decrypted when it must be) and captured as a write of `to`; *error says why a
    // source that cannot be read stopped it (the verdict is then Refused)
    Verdict copy(const QString &from, const QString &to, QString *error = nullptr);
    // A write that went on to the vault's seal() although its place is a follower's capturable path: the gate's
    // backstop. False with a reason (and counted, aborting in strict mode) for such a path.
    bool sealAllowed(const QString &path, QString *error = nullptr);

    // the QSaveFile of safesave::write(QSaveFile &) that was captured (its commit then counts as done)
    void noteCapturedSave(const QSaveFile *f);
    bool takeCapturedSave(const QSaveFile *f);

    // a follower for the folder `projectFolder` (nullptr when none)
    Follower *followerOfFolder(const QString &projectFolder) const;

private:
    void refuse(const Decision &d, const QString &path);
    mutable QMutex m_lock;
    QList<Follower *> m_followers;
    QSet<const QSaveFile *> m_captured;
    std::atomic<int> m_count{0};
};

Gate &gate();

} // namespace collab
} // namespace qb
