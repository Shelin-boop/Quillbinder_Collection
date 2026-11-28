#pragma once
#include <QDateTime>
#include <QList>
#include <QString>

class QTextDocument;

namespace qb {

class Project;

struct Snapshot {
    QString id;       // owning document
    QString file;     // absolute path
    QDateTime date;
    QString title;
};

namespace snapshots {
// 0.99.0 (Live Sync): in a window of a shared project (a follower, which never writes the shared files) snapshots are jobs of
// the host: taking, renaming, deleting and rolling back are asked of it, and the snapshot files come back as ordinary files of
// the copy. The service's client side (collab/HostBackup.h) installs one of these per shared project, by the project's folder;
// the calls wait for the host's answer (at most a few seconds) and return false with the reason when it does not come. A
// follower with no such hook cannot change snapshots (false with a reason). Listing and loading read the copy as always.
class RemoteJobs
{
public:
    virtual ~RemoteJobs() = default;
    virtual bool take(const QString &docId, const QString &title, QString *error) = 0;
    virtual bool setTitle(const QString &docId, const QString &file, const QString &title, QString *error) = 0;
    virtual bool remove(const QString &docId, const QString &file, QString *error) = 0;
    // the host first keeps the document's text as a snapshot of its own ("before rolling back"), then replaces the text; the
    // asker holds the document's lock
    virtual bool rollBack(const QString &docId, const QString &file, bool allowEmpty, QString *error) = 0;
};
void setRemoteJobs(const QString &projectFolder, RemoteJobs *jobs); // nullptr removes
RemoteJobs *remoteJobsFor(const QString &projectFolder);

QList<Snapshot> list(Project *p, const QString &docId);   // newest first
// from: the text to store if it is not the document's own (a copy read from its file)
// 0.93.0: the text of the document is what its newest snapshot holds (a snapshot of it would be a copy of that one)
bool sameAsNewest(Project *p, const QString &docId);
bool take(Project *p, const QString &docId, const QString &title, QString *error = nullptr, QTextDocument *from = nullptr);
void load(Project *p, const Snapshot &s, QTextDocument *into);
// 0.65.0 (audit F02): false with a reason when the snapshot's file cannot be read whole
bool load(Project *p, const Snapshot &s, QTextDocument *into, QString *error);
bool remove(Project *p, const Snapshot &s);
// false with *error when the list of titles could not be written (0.81.0)
bool setTitle(Project *p, const Snapshot &s, const QString &title, QString *error = nullptr);
// The text replaced with the snapshot; false with a reason (nothing replaced) when it cannot be read whole,
// or when it holds no text while its file is not empty (0.65.0, audit F02)
// 0.81.1 (audit of 0.81.0, F-01): a text whose file is damaged (shown read-only, never written) is repaired this way:
// its damaged file is kept as a copy in settings/conflicts first (*keptCopy), then the snapshot's text replaces it and
// is saved. A text whose file cannot be read at all, or has not arrived yet, is refused with a reason.
bool rollBack(Project *p, const Snapshot &s, QString *error = nullptr, bool allowEmpty = false, QString *keptCopy = nullptr);

// All snapshots of a project (also of documents in the Trash and of documents no longer there).
struct Totals {
    int snapshots = 0;
    int documents = 0;   // documents with snapshots
    qint64 bytes = 0;    // the project's snapshot folder
};
Totals totals(Project *p);
// Project > Delete Old Snapshots (0.61.1): every snapshot of the project; the texts stay as they
// are. A Scrivener-format project loses them in the .scriv with its next full save.
// Returns the snapshots deleted.
int removeAll(Project *p, QString *error = nullptr);
} // namespace snapshots

// ---------------------------------------------------------------- diff
struct DiffOp {
    enum Type { Equal, Insert, Delete } type;
    QString text;
};

enum class DiffGranularity { Paragraph, Sentence, Word, Character };

QList<DiffOp> diffTexts(const QString &oldText, const QString &newText, DiffGranularity g = DiffGranularity::Word);

} // namespace qb
