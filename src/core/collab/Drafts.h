#pragma once
// Recovery drafts of Live Sync (0.99.0), section 6.4 item 6 of the design. When two hosts of one project meet again, the newer host
// stays and everything the other side accepted after the split is kept as a draft on the device that authored it: one draft per
// document (its text, its notes, its title and the list of its other changes), never merged, never deleted by itself. A draft is
// found again by the same id however often the step is repeated (the id comes from project, fork and document), so a crash in
// the middle of the step is repaired by running it again.
//   <root>/recovery/<id>/draft.json   <root>/recovery/<id>/text   <root>/recovery/<id>/notes
// The ledger remembers, per device, the commits it authored that the host confirmed (a client has no log of its own); a host reads
// its own commit log instead (authoredFromLog).
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {

// one commit this device authored: its place in the history, the files it wrote and the operations on the project file
struct AuthoredCommit {
    qint64 seq = 0;
    qint64 epoch = 0;
    QStringList paths;  // relative to the project folder (puts only)
    QJsonArray ops;     // proto::toJson(Op)
    QJsonObject toJson() const;
    static AuthoredCommit fromJson(const QJsonObject &o);
};

struct Draft {
    QString id;
    QString project;
    QString document;   // the binder id; "" = changes of the project itself
    QString title;
    QString reason;     // "split" (made after the split), "outbox" ...
    QString author;     // the device id
    qint64 forkEpoch = 0; // the epoch the changes were accepted in
    qint64 forkSeq = 0;   // the state the other side continued from
    qint64 createdMs = 0;
    bool hasText = false, hasNotes = false;
    QString textPath, notesPath; // where the text and notes were in the project
    QJsonArray fields;           // the other changes (operations)
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &o, Draft *out);
};

class RecoveryDrafts {
public:
    explicit RecoveryDrafts(const QString &root = QString()) : m_root(root) {}
    void setRoot(const QString &root) { m_root = root; }
    QString root() const { return m_root; }
    // reads the folder; a damaged draft is skipped and counted in damaged(), never removed
    QList<Draft> list() const;
    int damaged() const { return m_damaged; }
    bool exists(const QString &id) const;
    bool add(const Draft &d, const QByteArray &text, const QByteArray &notes, QString *error = nullptr);
    QByteArray text(const QString &id) const;
    QByteArray notes(const QString &id) const;
    bool remove(const QString &id); // the writer's decision (asks first in the interface)
    static QString idFor(const QString &project, qint64 forkEpoch, qint64 forkSeq, const QString &document, const QString &author);

private:
    QString m_root;
    mutable int m_damaged = 0;
};

// The commits `deviceId` authored, from a host's log entries (the objects of `livesync/log/<seq>.json`) after `afterSeq`.
QList<AuthoredCommit> authoredFromLog(const QList<QJsonObject> &entries, const QString &deviceId, qint64 afterSeq);

// Makes one draft per document from the commits, reading the document's text and notes as the project folder holds them now
// (the state the device built during the split). Returns the number of drafts made, -1 with *error when one could not be kept.
int makeSplitDrafts(RecoveryDrafts &store, const QString &projectFolder, const QString &project, const QString &author,
                    const QList<AuthoredCommit> &commits, qint64 forkEpoch, qint64 forkSeq, const QString &reason, qint64 nowMs,
                    QString *error = nullptr);

// The ledger: the commits this device authored and the host confirmed, in `<path>` (kept for 10,000 commits).
class CommitLedger {
public:
    explicit CommitLedger(const QString &path) : m_path(path) {}
    bool load();
    bool add(const AuthoredCommit &c, QString *error = nullptr);
    QList<AuthoredCommit> after(qint64 afterSeq, qint64 beforeEpoch) const;
    // the commits after `afterSeq` of earlier epochs were handled (turned into drafts)
    bool forget(qint64 afterSeq, qint64 beforeEpoch, QString *error = nullptr);
    int size() const { return int(m_list.size()); }

private:
    bool save(QString *error);
    QString m_path;
    QList<AuthoredCommit> m_list;
};

} // namespace collab
} // namespace qb
