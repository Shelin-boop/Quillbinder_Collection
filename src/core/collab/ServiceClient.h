#pragma once
// Live Sync, the window's end of the link to its service (0.99.0). A window of a shared project opens the folder as a follower
// (collab/Gate.h): it never writes the shared files, its writes are captured. This class takes the captured change sets,
// makes units of them (collab/ChangeUnits.h) and sends them to the service as change requests; it asks for the locks of
// the editor, and it tells the window what the service tells it (a commit is in the folder, a lock was granted or taken, the
// state of the connection). The follower it keeps answers the questions "may this be changed here" from that state.
#include "core/collab/ChangeUnits.h"
#include "core/collab/Gate.h"
#include "core/collab/net/Link.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QTimer>

namespace qb {
namespace collab {

// The follower of a window: what the service said about the project, kept for the questions of the program.
class LiveFollower : public Follower {
public:
    LiveFollower() = default;
    // the explicit lock of the document is held by this device; the editor may change it
    bool holdsLock(const QString &resource) const { return m_mine.contains(resource); }
    QStringList mine() const { return m_mine.values(); }
    void setMine(const QStringList &resources) { m_mine = QSet<QString>(resources.begin(), resources.end()); }
    // without the lock the text is still read-only for the editor (the first key asks for it), but other changes are decided
    // by the host at commit time
    QString whyNotChangeable(const QString &resource) const override { return Follower::whyNotChangeable(resource); }

private:
    QSet<QString> m_mine;
};

class ServiceClient : public QObject {
    Q_OBJECT
public:
    // `link` is Ready (a LocalLink to the service, or one end of a MemLink pair); the client takes it as its child
    explicit ServiceClient(net::Link *link, QObject *parent = nullptr);
    ~ServiceClient() override;

    // the follower to give Project::open; valid for the client's life
    LiveFollower *follower() { return &m_follower; }

    struct Attached {
        bool ok = false;
        QString code; // NOT_SHARED, PROBLEM, TIMEOUT, LINK
        QString uuid, title, hostName;
        bool host = false;
        bool ready = false;
        QString text;
    };
    // Asks the service to take this folder; waits (running the event loop) at most `timeoutMs` for the answer. After it, the state
    // below is current.
    Attached attach(const QString &folder, bool keepHosting, int timeoutMs = 8000);
    bool isAttached() const { return !m_uuid.isEmpty(); }
    QString uuid() const { return m_uuid; }
    QString folder() const { return m_folder; }
    bool linkAlive() const;

    // ---- state as the service last told it
    bool isHost() const { return m_host; }
    int stateCode() const { return m_state; } // ShareState
    bool replicaReady() const { return m_ready; }
    bool readOnly() const { return m_readOnly; }
    QString readOnlyReason() const { return m_readOnlyReason; }
    int pendingCount() const { return m_pending; }
    int draftCount() const { return m_drafts; }
    qint64 seq() const { return m_seq; }
    qint64 epoch() const { return m_epoch; }          // the hosting epoch of the project as the service knows it
    QString hostDeviceId() const { return m_hostId; }
    QString hostName() const { return m_hostName; }
    bool holds(const QString &resource) const { return m_follower.holdsLock(resource); }
    QString heldBy(const QString &resource) const { return m_held.value(resource); }
    QHash<QString, QString> heldElsewhere() const { return m_held; }
    bool unreachable(const QString &resource) const { return m_unreachable.contains(resource); }

    // ---- what the window does
    // Sends what the follower captured (call it when Capture's listener fires). Returns the number of requests made; the reason
    // for a failure is in lastError().
    int sendCaptured();
    QString lastError() const { return m_error; }
    // the opIds of requests sent and not yet answered by a commit, rejection or draft
    QStringList inFlight() const { return m_open; }
    bool busy() const { return !m_open.isEmpty() || !m_submitIds.isEmpty(); }
    void lock(const QStringList &resources);
    void unlock(const QStringList &resources);
    void askToEdit(const QString &resource);
    void cancelAsk(const QString &resource);
    void takeOver(const QString &resource);
    void answerEdit(const QString &resource, bool handOver, const QString &askerDevice = QString());
    void requestBackup();
    // A snapshot job of the shared project (op: take, rename, delete, rollBack) done by its host through the service; waits (running
    // the event loop) for the answer. False with *code (the host's reason code, or LINK / TIMEOUT) and *text.
    bool snapshotJob(const QJsonObject &body, QString *code, QString *text, int timeoutMs = 20000);
    void stopSharing();
    // "Transfer Hosting...": the service starts a planned hand-over to this paired device
    void transferHosting(const QString &targetDeviceId);
    void detach();
    // waits (running the event loop) until every request sent is answered (or the time is up); true when none is open
    bool flush(int timeoutMs = 5000);

signals:
    void stateChanged();
    // a commit (also this device's own: `own`) is in the folder; paths are relative
    void eventApplied(qint64 seq, const QString &opId, const QString &actor, bool own, const QJsonArray &ops, const QStringList &paths,
                      const QJsonObject &revs);
    void replicaReplaced();
    void committed(const QString &opId, qint64 seq);
    void rejected(const QString &opId, const QJsonArray &rejects);
    void draftKept(const QString &opId, const QString &reason);
    void problem(const QString &opId, const QString &code);
    void lockGranted(const QString &resource);
    void lockDenied(const QString &resource, const QString &holder, const QString &reason);
    void lockRevoked(const QString &resource, const QString &reason);
    void editAsked(const QString &resource, const QString &askerName, qint64 waitMs);
    void editStatus(const QString &resource, bool declined, qint64 waitMs, bool mayForce, const QString &holder);
    void serviceGone();
    void quitRequested();

private:
    void onFrame(const net::Frame &f);
    void applyState(const QJsonObject &m);
    void sendMsg(const QJsonObject &m);
    QString nextId();

    net::Link *m_link;
    LiveFollower m_follower;
    QString m_uuid, m_folder, m_hostName, m_readOnlyReason, m_error;
    bool m_host = false, m_ready = false, m_readOnly = false;
    int m_state = 0, m_pending = 0, m_drafts = 0;
    qint64 m_seq = 0;
    qint64 m_epoch = 0;
    QString m_hostId;
    QHash<QString, QString> m_held;
    QSet<QString> m_unreachable;
    QStringList m_open;
    QSet<QString> m_submitIds; // requests sent whose answer (the operation id) has not come
    QHash<QString, QJsonObject> m_answers; // by "re"
    int m_counter = 0;
    bool m_gone = false;
};

} // namespace collab
} // namespace qb
