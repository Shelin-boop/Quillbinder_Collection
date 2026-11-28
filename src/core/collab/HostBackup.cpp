#include "core/collab/HostBackup.h"

#include "core/BinderItem.h"
#include "core/DocumentStore.h"
#include "core/PathSafety.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/SaveBatch.h"
#include "core/TestSwitches.h"
#include "core/collab/Gate.h"
#include "core/collab/PathClass.h"
#include "core/crypto/Vault.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QTextDocument>
#include <QThread>
#include <QTimer>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace qb {
namespace collab {

using namespace proto;

namespace {

constexpr qint64 kSpare = qint64(64) << 20;

QString stateName(BackupStatus::State s)
{
    switch (s) {
    case BackupStatus::Running: return QStringLiteral("running");
    case BackupStatus::Done: return QStringLiteral("done");
    case BackupStatus::Waiting: return QStringLiteral("waiting");
    case BackupStatus::Incomplete: return QStringLiteral("incomplete");
    case BackupStatus::Failed: return QStringLiteral("failed");
    case BackupStatus::None: break;
    }
    return QStringLiteral("none");
}
BackupStatus::State stateFrom(const QString &n)
{
    for (BackupStatus::State s : {BackupStatus::Running, BackupStatus::Done, BackupStatus::Waiting, BackupStatus::Incomplete, BackupStatus::Failed})
        if (stateName(s) == n)
            return s;
    return BackupStatus::None;
}

// what a copy of the project leaves out: Live Sync's own state, a save in progress, who has the project open
bool leftOut(const QString &rel, const QString &name)
{
    if (rel == QLatin1String("livesync") || rel == QLatin1String("livesync-local") || rel == QLatin1String("livesync-replica.json"))
        return true;
    if (rel == QLatin1String("settings/journal.json") || savebatch::isStaged(name))
        return true;
    if (rel == QLatin1String("settings/lock") || rel.startsWith(QLatin1String("settings/lock-work-")) || name == QLatin1String("quillbinder.lock")
        || name.startsWith(QLatin1String("quillbinder.lock-work-")))
        return true;
    return false;
}

qint64 treeBytes(const QString &dir, int depth = 0)
{
    qint64 n = 0;
    if (depth > 32)
        return 0;
    for (const QFileInfo &fi : QDir(dir).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System))
        n += fi.isDir() ? treeBytes(fi.absoluteFilePath(), depth + 1) : fi.size();
    return n;
}

struct Walk {
    QString root, dest;
    bool copyOnly = false;
    std::function<qint64()> freeBytes;
    int linked = 0, copied = 0;
    bool noSpace = false, dirty = false; // dirty: a save was in progress
    QString why;
    bool checkedSpace = false;
};

bool copyOne(Walk &w, const QString &from, const QString &to)
{
    // a file that fails on purpose (the tests' switch of the copies)
    const QString refused = QB_TEST_ENV("QB_TEST_FAIL_COPY");
    if (!refused.isEmpty() && (from.contains(refused) || to.contains(refused))) {
        w.why = QStringLiteral("%1: the copy failed (a test's switch)").arg(to);
        return false;
    }
#ifdef Q_OS_UNIX
    if (!w.copyOnly && QB_TEST_ENV("QB_TEST_LS_NO_LINKS") != QLatin1String("1")) {
        if (::link(QFile::encodeName(from).constData(), QFile::encodeName(to).constData()) == 0) {
            ++w.linked;
            return true;
        }
    }
#endif
    if (!w.checkedSpace) { // the first real copy: does the whole project fit?
        w.checkedSpace = true;
        const qint64 need = treeBytes(w.root) + kSpare;
        const qint64 have = w.freeBytes ? w.freeBytes() : QStorageInfo(w.root).bytesAvailable();
        if (have >= 0 && have < need) {
            w.noSpace = true;
            w.why = QStringLiteral("the project needs %1 bytes for its copy and %2 are free").arg(need).arg(have);
            return false;
        }
    }
    QFile in(from);
    if (!in.open(QIODevice::ReadOnly)) {
        w.why = QStringLiteral("%1: %2").arg(from, in.errorString());
        return false;
    }
    QFile out(to);
    if (!out.open(QIODevice::WriteOnly)) {
        w.why = QStringLiteral("%1: %2").arg(to, out.errorString());
        return false;
    }
    qint64 copied = 0;
    for (;;) {
        const QByteArray piece = in.read(4 << 20);
        if (piece.isEmpty())
            break;
        if (out.write(piece) != piece.size()) {
            w.why = QStringLiteral("%1: %2").arg(to, out.errorString());
            return false;
        }
        copied += piece.size();
    }
    out.close();
    if (in.error() != QFileDevice::NoError || copied != in.size() || out.error() != QFileDevice::NoError) {
        w.why = QStringLiteral("%1: not copied whole").arg(from);
        return false;
    }
    ++w.copied;
    return true;
}

bool walk(Walk &w, const QString &srcDir, const QString &dstDir, const QString &relDir, int depth)
{
    if (depth > 32) {
        w.why = QStringLiteral("folders nested too deep");
        return false;
    }
    if (!QDir().mkpath(dstDir)) {
        w.why = QStringLiteral("cannot create %1").arg(dstDir);
        return false;
    }
    for (const QFileInfo &fi : QDir(srcDir).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::Name)) {
        const QString rel = relDir.isEmpty() ? fi.fileName() : relDir + QLatin1Char('/') + fi.fileName();
        if (rel == QLatin1String("settings/journal.json") || savebatch::isStaged(fi.fileName())) {
            w.dirty = true; // a save is under way: this is no state of a commit
            return true;
        }
        if (leftOut(rel, fi.fileName()))
            continue;
        if (fi.isSymLink() && !fi.exists())
            continue; // a link to nothing (a drive that is not attached) has no content
        if (fi.isDir()) {
            if (!walk(w, fi.absoluteFilePath(), dstDir + QLatin1Char('/') + fi.fileName(), rel, depth + 1))
                return false;
            if (w.dirty)
                return true;
        } else if (fi.isFile()) {
            const QString target = fi.isSymLink() ? fi.canonicalFilePath() : fi.absoluteFilePath();
            if (!copyOne(w, target, dstDir + QLatin1Char('/') + fi.fileName()))
                return false;
        }
    }
    return true;
}

qint64 stateSeqOnDisk(const QString &folder)
{
    QByteArray raw;
    if (!vault::readFile(folder + QStringLiteral("/livesync/state.json"), &raw))
        return -1;
    return qint64(QJsonDocument::fromJson(raw).object().value(QStringLiteral("seq")).toDouble(-1));
}

const QRegularExpression &opIdRx()
{
    static const QRegularExpression rx(QStringLiteral("^[0-9A-Za-z-]{8,64}$"));
    return rx;
}

} // namespace

QString backupReasonText(const QString &code, const QString &hostWords)
{
    if (code == QLatin1String("no-space"))
        return QCoreApplication::translate("qb::collab::HostBackup", "There is not enough free space on the host for the backup.");
    if (code == QLatin1String("busy"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The project was being saved all the time; no whole copy could be made.");
    if (code == QLatin1String("freeze-failed"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The host could not make a copy of the project to back up.");
    if (code == QLatin1String("waiting"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The backup was made, but the backup folder cannot take it now. It waits on the host.");
    if (code == QLatin1String("skipped-files"))
        return QCoreApplication::translate("qb::collab::HostBackup", "Some files could not be read, so the backup is incomplete.");
    if (code == QLatin1String("pack-failed"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The host could not write the backup.");
    if (code == QLatin1String("interrupted"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The host was closed while the backup ran.");
    if (code == QLatin1String("no-project"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The host cannot back up this project now.");
    return hostWords;
}

// ---------------------------------------------------------------- the copy

FreezeResult HostBackup::freeze(HostSession *host, const QString &destinationParent, bool copyOnly, const std::function<qint64()> &freeBytes)
{
    FreezeResult r;
    const QString root = host->m_folder;
    for (int attempt = 0; attempt < 4; ++attempt) {
        const qint64 seq = host->m_seq;
        r = FreezeResult();
        r.seq = seq;
        r.folder = destinationParent + QStringLiteral("/freeze-%1").arg(seq);
        QDir(r.folder).removeRecursively();
        if (host->m_broken) {
            r.code = QStringLiteral("busy");
            r.why = QStringLiteral("an earlier save is not finished");
            return r;
        }
        Walk w;
        w.root = root;
        w.dest = r.folder;
        w.copyOnly = copyOnly;
        w.freeBytes = freeBytes;
        const bool ok = walk(w, root, r.folder, QString(), 0);
        r.linked = w.linked;
        r.copied = w.copied;
        // whole at one commit: no save was under way, and the commit number on disk and in memory is still the one we began with
        const bool whole = ok && !w.dirty && host->m_seq == seq && stateSeqOnDisk(root) == seq
                           && !QFileInfo::exists(root + QStringLiteral("/settings/journal.json"));
        if (whole) {
            r.ok = true;
            return r;
        }
        QDir(r.folder).removeRecursively();
        if (!ok) {
            r.code = w.noSpace ? QStringLiteral("no-space") : QStringLiteral("freeze-failed");
            r.why = w.why;
            return r;
        }
        QThread::msleep(40); // a save of another program was under way (or a commit came): once more
    }
    r.code = QStringLiteral("busy");
    r.why = QStringLiteral("the project changed all the time");
    return r;
}

void HostBackup::removeOldFreezes()
{
    const QString base = m_cfg.host->m_folder + QStringLiteral("/livesync");
    for (const QString &n : QDir(base).entryList({QStringLiteral("freeze-*")}, QDir::Dirs))
        pathsafety::removeTreeInside(base + QLatin1Char('/') + n, m_cfg.host->m_folder);
}

// ---------------------------------------------------------------- the host's state and backups

HostBackup::HostBackup(const Config &config, QObject *parent) : QObject(parent), m_cfg(config)
{
    loadState();
    removeOldFreezes();
}

HostBackup::~HostBackup() = default;

QString HostBackup::statePath() const { return m_cfg.host->m_folder + QStringLiteral("/livesync/backup-state.json"); }
QString HostBackup::pendingPath() const { return m_cfg.host->m_folder + QStringLiteral("/livesync/job-pending.json"); }

void HostBackup::loadState()
{
    QByteArray raw;
    if (!vault::readFile(statePath(), &raw))
        return;
    const QJsonObject o = QJsonDocument::fromJson(raw).object();
    const qint64 ok = qint64(o.value(QStringLiteral("lastOk")).toDouble());
    if (ok > 0)
        m_status.lastSuccess = QDateTime::fromMSecsSinceEpoch(ok);
    m_status.state = stateFrom(o.value(QStringLiteral("state")).toString());
    m_code = o.value(QStringLiteral("code")).toString();
    m_rawDetail = o.value(QStringLiteral("detail")).toString();
    m_statusSeq = qint64(o.value(QStringLiteral("seq")).toDouble());
    if (m_status.state == BackupStatus::Running) { // the host was closed in the middle of it: that is no success
        m_status.state = BackupStatus::Failed;
        m_code = QStringLiteral("interrupted");
        m_rawDetail.clear();
    }
    m_status.detail = backupReasonText(m_code, m_rawDetail);
}

void HostBackup::saveState()
{
    QJsonObject o{{QStringLiteral("state"), stateName(m_status.state)},
                  {QStringLiteral("lastOk"), m_status.lastSuccess.isValid() ? double(m_status.lastSuccess.toMSecsSinceEpoch()) : 0.0},
                  {QStringLiteral("code"), m_code},
                  {QStringLiteral("detail"), m_rawDetail},
                  {QStringLiteral("seq"), double(m_statusSeq)}};
    QString err;
    safesave::writeFile(statePath(), QJsonDocument(o).toJson(QJsonDocument::Compact), &err); // a state that cannot be kept is not fatal
}

QJsonObject HostBackup::statusJson(const QString &re) const
{
    QJsonObject body{{QStringLiteral("state"), stateName(m_status.state)},
                     {QStringLiteral("lastOk"), m_status.lastSuccess.isValid() ? double(m_status.lastSuccess.toMSecsSinceEpoch()) : 0.0},
                     {QStringLiteral("seq"), double(m_statusSeq)},
                     {QStringLiteral("code"), m_code},
                     {QStringLiteral("detail"), m_rawDetail.left(500)}};
    QJsonObject m = make(QLatin1String(T::BACKUP_STATUS), m_cfg.host->m_uuid, body);
    if (!re.isEmpty())
        m.insert(QStringLiteral("re"), re);
    return m;
}

void HostBackup::pushStatus()
{
    for (auto it = m_watchers.begin(); it != m_watchers.end();) {
        if (it->isNull() || (*it)->state() != net::Link::State::Ready) {
            it = m_watchers.erase(it);
            continue;
        }
        (*it)->sendJson(statusJson());
        ++it;
    }
    emit statusChanged();
}

void HostBackup::setState(BackupStatus::State state, const QString &code, const QString &detail)
{
    m_status.state = state;
    m_code = code;
    m_rawDetail = detail;
    m_status.detail = backupReasonText(code, detail);
    saveState();
    pushStatus();
}

bool HostBackup::requestBackup(QString *why)
{
    ServiceScope serviceScope; // the host's own writes are never a window's (Gate.h)
    if (running()) {
        if (why)
            *why = tr("A backup is running already.");
        return false;
    }
    if (!project() || !m_cfg.host || !m_cfg.host->isStarted()) {
        if (why)
            *why = backupReasonText(QStringLiteral("no-project"), QString());
        return false;
    }
    m_statusSeq = m_cfg.host->m_seq;
    setState(BackupStatus::Running, QString(), QString());
    QTimer::singleShot(0, this, [this] { begin(QString()); });
    return true;
}

void HostBackup::begin(const QString &)
{
    ServiceScope serviceScope; // the host's own writes are never a window's (Gate.h)
    HostSession *host = m_cfg.host;
    if (m_cfg.pauseCommits)
        m_cfg.pauseCommits();
    const FreezeResult view = freeze(host, host->m_folder + QStringLiteral("/livesync"), false, m_cfg.freeBytes);
    if (m_cfg.resumeCommits)
        m_cfg.resumeCommits();
    if (!view.ok) {
        finish(BackupStatus::Failed, view.code, view.why);
        return;
    }
    m_statusSeq = view.seq;
    pack(view);
}

void HostBackup::pack(const FreezeResult &view)
{
    ServiceScope serviceScope; // the host's own writes are never a window's (Gate.h)
    QPointer<HostBackup> self(this);
    backup::FrozenView fv;
    fv.folder = view.folder;
    fv.keepAlive = [self] {
        if (self && !self->m_inPump) { // the host goes on answering its devices while the copy is packed
            self->m_inPump = true;
            WindowScope outside; // what the windows of this process do meanwhile is theirs, not the host's
            QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            if (self)
                self->m_inPump = false;
        }
    };
    QString out, err;
    bool waiting = false;
    QStringList notes;
    const bool ok = backup::backupProject(project(), m_cfg.backupDirOf ? m_cfg.backupDirOf() : m_cfg.backupDir, m_cfg.keepOf ? m_cfg.keepOf() : m_cfg.keep, &out, &err, backup::Where::BackupFolder, &waiting, &notes, &fv);
    if (!self)
        return;
    pathsafety::removeTreeInside(view.folder, m_cfg.host->m_folder);
    if (ok && err.isEmpty()) {
        m_status.lastSuccess = QDateTime::currentDateTime();
        finish(BackupStatus::Done, QString(), QString());
    } else if (ok) {
        finish(BackupStatus::Incomplete, QStringLiteral("skipped-files"), err);
    } else if (waiting) {
        finish(BackupStatus::Waiting, QStringLiteral("waiting"), err);
    } else {
        finish(BackupStatus::Failed, QStringLiteral("pack-failed"), err);
    }
}

void HostBackup::finish(BackupStatus::State state, const QString &code, const QString &detail) { setState(state, code, detail); }

void HostBackup::answer(net::Link *link, const QJsonObject &m)
{
    if (link && link->state() == net::Link::State::Ready)
        link->sendJson(m);
}

void HostBackup::refuse(const QString &deviceId, net::Link *link, const QString &code, const QString &re, const QString &text)
{
    emit requestRefused(deviceId, code);
    answer(link, errorMessage(code, re, text, m_cfg.host->m_uuid));
}

bool HostBackup::handle(const QString &deviceId, net::Link *link, const QJsonObject &m)
{
    ServiceScope serviceScope; // the host's own writes are never a window's (Gate.h)
    const QString t = m.value(QStringLiteral("t")).toString();
    if (is(t, T::BACKUP_REQUEST)) {
        onBackupRequest(deviceId, link, m);
        return true;
    }
    if (is(t, T::SNAPSHOT_OP)) {
        onSnapshotOp(deviceId, link, m);
        return true;
    }
    return false;
}

void HostBackup::onBackupRequest(const QString &deviceId, net::Link *link, const QJsonObject &m)
{
    const QString id = m.value(QStringLiteral("id")).toString();
    HostSession *host = m_cfg.host;
    if (m.contains(QStringLiteral("p")) && m.value(QStringLiteral("p")).toString() != host->m_uuid) {
        refuse(deviceId, link, QStringLiteral("UNKNOWN_PROJECT"), id);
        return;
    }
    if (host->roleOf(deviceId) == Role::None) {
        refuse(deviceId, link, QStringLiteral("NO_ACCESS"), id);
        return;
    }
    if (!m_watchers.contains(QPointer<net::Link>(link)))
        m_watchers << QPointer<net::Link>(link);
    const bool queryOnly = m.value(QStringLiteral("query")).toBool(false);
    if (queryOnly || running()) {
        answer(link, statusJson(id));
        return;
    }
    const qint64 now = host->m_clock->now();
    if (m_lastAsk.contains(deviceId) && now - m_lastAsk.value(deviceId) < m_cfg.minIntervalMs) {
        refuse(deviceId, link, QStringLiteral("HOST_BUSY"), id, QStringLiteral("a backup was asked for a moment ago"));
        return;
    }
    m_lastAsk.insert(deviceId, now);
    QString why;
    if (!requestBackup(&why)) {
        refuse(deviceId, link, QStringLiteral("HOST_BUSY"), id, why);
        return;
    }
    // (pushStatus already told every watcher, this link included, that it runs)
}

// ---------------------------------------------------------------- snapshot jobs

void HostBackup::onSnapshotOp(const QString &deviceId, net::Link *link, const QJsonObject &m)
{
    const QString id = m.value(QStringLiteral("id")).toString();
    HostSession *host = m_cfg.host;
    auto reply = [&](const JobResult &r, qint64 seq) {
        QJsonObject body{{QStringLiteral("opId"), m.value(QStringLiteral("opId")).toString()},
                         {QStringLiteral("status"), r.ok ? QStringLiteral("ok") : QStringLiteral("error")},
                         {QStringLiteral("code"), r.code},
                         {QStringLiteral("text"), r.text.left(500)},
                         {QStringLiteral("file"), r.file},
                         {QStringLiteral("seq"), double(seq)}};
        QJsonObject out = replyTo(m, QLatin1String(T::SNAPSHOT_RESULT), body);
        answer(link, out);
    };
    const QString opId = m.value(QStringLiteral("opId")).toString();
    if (m.contains(QStringLiteral("p")) && m.value(QStringLiteral("p")).toString() != host->m_uuid) {
        refuse(deviceId, link, QStringLiteral("UNKNOWN_PROJECT"), id);
        return;
    }
    if (!opIdRx().match(opId).hasMatch()) {
        refuse(deviceId, link, QStringLiteral("BAD_OP"), id, QStringLiteral("opId"));
        return;
    }
    if (m_doneJobs.contains(opId)) { // the same job again: the stored answer, never done twice
        reply(m_doneJobs.value(opId), host->m_seq);
        return;
    }
    const JobResult r = runSnapshotJob(deviceId, m);
    m_doneJobs.insert(opId, r);
    m_doneOrder << opId;
    while (m_doneOrder.size() > 1000)
        m_doneJobs.remove(m_doneOrder.takeFirst());
    reply(r, host->m_seq);
}

static QString jobCodeText(const QString &code, const QString &host);

HostBackup::JobAnswer HostBackup::runJobFor(const QString &deviceId, const QJsonObject &body)
{
    ServiceScope serviceScope;
    JobAnswer a;
    QJsonObject m = body;
    QString opId = m.value(QStringLiteral("opId")).toString();
    if (!opIdRx().match(opId).hasMatch()) {
        opId = newId();
        m.insert(QStringLiteral("opId"), opId);
    }
    JobResult r;
    if (m_doneJobs.contains(opId)) {
        r = m_doneJobs.value(opId);
    } else {
        r = runSnapshotJob(deviceId, m);
        m_doneJobs.insert(opId, r);
        m_doneOrder << opId;
        while (m_doneOrder.size() > 1000)
            m_doneJobs.remove(m_doneOrder.takeFirst());
    }
    a.ok = r.ok;
    a.code = r.code;
    a.text = r.ok ? QString() : jobCodeText(r.code, r.text);
    a.file = r.file;
    return a;
}

HostBackup::JobResult HostBackup::runSnapshotJob(const QString &deviceId, const QJsonObject &m)
{
    static const QRegularExpression docRx(QStringLiteral("^[0-9A-Za-z_.-]{1,64}$"));
    static const QRegularExpression fileRx(QStringLiteral("^\\d{8}-\\d{6}\\.(md|rtf)$"));
    JobResult bad;
    auto fail = [&](const char *code, const QString &text = QString()) {
        JobResult r;
        r.code = QLatin1String(code);
        r.text = text;
        return r;
    };
    HostSession *host = m_cfg.host;
    Project *p = project();
    if (!p || !host->isStarted() || host->m_broken)
        return fail("STORAGE_FAILED", QStringLiteral("the host cannot change the project now"));
    if (host->roleOf(deviceId) != Role::Editor)
        return fail("READ_ONLY");
    const QString op = m.value(QStringLiteral("op")).toString();
    const QString doc = m.value(QStringLiteral("doc")).toString();
    const QString file = m.value(QStringLiteral("file")).toString();
    const QString title = m.value(QStringLiteral("title")).toString();
    if (!(op == QLatin1String("take") || op == QLatin1String("rename") || op == QLatin1String("delete") || op == QLatin1String("rollBack"))
        || !docRx.match(doc).hasMatch() || title.size() > 500)
        return fail("BAD_OP");
    if (op != QLatin1String("take") && !fileRx.match(file).hasMatch())
        return fail("BAD_OP");
    BinderItem *item = p->item(doc);
    if (!item || !item->hasText())
        return fail("BAD_OP", QStringLiteral("no such document"));
    // locks: other devices' locks stand in the way of a change of the document's snapshots; a roll back needs the asker's own
    const QString resource = QStringLiteral("doc:") + doc;
    Owner asker;
    asker.device = deviceId;
    asker.session = QStringLiteral("-");
    const QList<Lock> mine = host->m_locks.locksOfDevice(deviceId);
    if (!mine.isEmpty())
        asker = mine.first().owner;
    const QList<Lock> blockers = host->m_locks.blockers(resource, asker);
    if (!blockers.isEmpty())
        return fail("LOCK_HELD", blockers.first().ownerName);
    if (op == QLatin1String("rollBack")) {
        const Lock *l = host->m_locks.find(resource);
        if (!l || l->owner.device != deviceId)
            return fail("LOCK_NEEDED");
    }
    // the snapshot the job names is looked up in the host's own list (a name from the wire never becomes a path)
    Snapshot snap;
    if (op != QLatin1String("take")) {
        bool found = false;
        for (const Snapshot &s : snapshots::list(p, doc))
            if (QFileInfo(s.file).fileName() == file) {
                snap = s;
                found = true;
                break;
            }
        if (!found)
            return fail("BAD_OP", QStringLiteral("no such snapshot"));
    }
    const QString opId = m.value(QStringLiteral("opId")).toString();
    QString err;
    p->docs()->reload(doc); // the host's Project follows what the commits wrote on disk
    // a mark that tells the next start that a job's files may lack their commit
    safesave::writeFile(pendingPath(), QJsonDocument(QJsonObject{{QStringLiteral("opId"), opId}}).toJson(QJsonDocument::Compact), nullptr);
    bool ok = false;
    JobResult r;
    if (op == QLatin1String("take")) {
        QTextDocument committed; // the text as the last commit left it on disk
        QByteArray bytes;
        const QString tp = p->docs()->textPath(doc);
        if (vault::readFile(tp, &bytes))
            p->docs()->deserialize(doc, bytes, &committed, tp.endsWith(QLatin1String(".rtf")));
        ok = snapshots::take(p, doc, title, &err, &committed);
        if (ok) {
            QString saveErr;
            p->save(&saveErr); // the item's "has snapshots" mark; a save that fails shows at the next commit
        }
    } else if (op == QLatin1String("rename")) {
        ok = snapshots::setTitle(p, snap, title, &err);
    } else if (op == QLatin1String("delete")) {
        ok = snapshots::remove(p, snap);
        if (!ok)
            err = QStringLiteral("the snapshot could not be removed");
    } else {
        ok = snapshots::take(p, doc, QStringLiteral("Before rolling back"), &err, nullptr);
        if (ok) {
            ok = snapshots::rollBack(p, snap, &err, m.value(QStringLiteral("allowEmpty")).toBool(false), nullptr);
            if (ok)
                ok = p->save(&err);
        }
    }
    QString pubErr;
    const bool published = publishChanges(opId, deviceId, &pubErr);
    QFile::remove(pendingPath());
    if (!ok) {
        r.code = QStringLiteral("STORAGE_FAILED");
        r.text = err;
        return r;
    }
    if (!published) {
        r.code = QStringLiteral("STORAGE_FAILED");
        r.text = pubErr;
        return r;
    }
    r.ok = true;
    if (op == QLatin1String("take")) {
        const QList<Snapshot> all = snapshots::list(p, doc);
        if (!all.isEmpty())
            r.file = QFileInfo(all.first().file).fileName();
    }
    return r;
}

bool HostBackup::keepProtectiveSnapshot(const QString &resource, const QString &label, QString *error)
{
    ServiceScope serviceScope; // the host's own writes are never a window's (Gate.h)
    Project *p = project();
    if (!p || !resource.startsWith(QLatin1String("doc:"))) {
        if (error)
            *error = QStringLiteral("no document to keep");
        return false;
    }
    const QString doc = resource.mid(4);
    BinderItem *item = p->item(doc);
    if (!item || !item->hasText())
        return true; // nothing of text to keep
    QTextDocument committed;
    QByteArray bytes;
    const QString tp = p->docs()->textPath(doc);
    if (!vault::readFile(tp, &bytes)) {
        // no text file yet: nothing to lose
        return true;
    }
    p->docs()->deserialize(doc, bytes, &committed, tp.endsWith(QLatin1String(".rtf")));
    safesave::writeFile(pendingPath(), QJsonDocument(QJsonObject{{QStringLiteral("opId"), QStringLiteral("protective")}}).toJson(QJsonDocument::Compact), nullptr);
    QString err;
    if (!snapshots::take(p, doc, label, &err, &committed)) {
        QFile::remove(pendingPath());
        if (error)
            *error = err;
        return false;
    }
    QString pubErr;
    const bool published = publishChanges(newId(), QString(), &pubErr);
    QFile::remove(pendingPath());
    if (!published && error)
        *error = pubErr;
    return published;
}

HostSession::SnapshotHook HostBackup::protectiveSnapshotHook()
{
    QPointer<HostBackup> self(this);
    return [self](const QString &resource, const QString &label, QString *error) {
        if (!self) {
            if (error)
                *error = QStringLiteral("no snapshot maker");
            return false;
        }
        return self->keepProtectiveSnapshot(resource, label, error);
    };
}

bool HostBackup::recoverJobs(QString *error)
{
    ServiceScope serviceScope; // the host's own writes are never a window's (Gate.h)
    if (!QFileInfo::exists(pendingPath()))
        return true;
    const bool ok = publishChanges(newId(), QString(), error, nullptr, true);
    if (ok)
        QFile::remove(pendingPath());
    return ok;
}

// What the host's own code changed in the project folder becomes one commit: the files are on disk already, so the batch holds
// the state and the log record. The host's list of files is compared with the folder; every changed file raises the revision of its
// resource, and the others are told by an event like for any commit.
bool HostBackup::publishChanges(const QString &opId, const QString &actorDevice, QString *error, qint64 *seqOut, bool afterRestart)
{
    HostSession *h = m_cfg.host;
    auto fail = [&](const QString &w) {
        if (error)
            *error = w;
        return false;
    };
    Manifest now;
    QString err;
    if (!Manifest::scan(h->m_folder, h->m_folder + QStringLiteral("/livesync/hashes.json"), h->m_revs, &now, &err))
        return fail(err);
    QHash<QString, const ManifestEntry *> before;
    for (const ManifestEntry &e : h->m_manifest.entries)
        before.insert(e.path, &e);
    QStringList put, removed;
    for (const ManifestEntry &e : now.entries) {
        const ManifestEntry *old = before.value(e.path, nullptr);
        if (!old || old->sha256 != e.sha256 || old->size != e.size || (afterRestart && (classify(e.path).kind == PathKind::Snapshot || classify(e.path).kind == PathKind::Index)))
            put << e.path;
    }
    QSet<QString> nowPaths;
    for (const ManifestEntry &e : now.entries)
        nowPaths.insert(e.path);
    for (const ManifestEntry &e : h->m_manifest.entries)
        if (!nowPaths.contains(e.path))
            removed << e.path;
    if (put.isEmpty() && removed.isEmpty()) {
        if (seqOut)
            *seqOut = h->m_seq;
        return true;
    }
    const qint64 newSeq = h->m_seq + 1;
    QHash<QString, qint64> newRevs = h->m_revs;
    QJsonObject changedRevs;
    QSet<QString> bumped;
    for (const QString &path : put + removed) {
        const PathInfo info = classify(path);
        if (!info.resource.isEmpty() && !bumped.contains(info.resource)) {
            bumped.insert(info.resource);
            newRevs[info.resource] = h->m_revs.value(info.resource) + 1;
            changedRevs.insert(info.resource, double(newRevs.value(info.resource)));
        }
    }
    const qint64 at = h->m_clock->wallMs();
    QJsonArray logFiles, eventFiles;
    for (const QString &path : put) {
        logFiles.append(QJsonObject{{QStringLiteral("path"), path}, {QStringLiteral("kind"), QStringLiteral("put")}});
        const ManifestEntry *e = now.find(path);
        const PathInfo info = classify(path);
        eventFiles.append(QJsonObject{{QStringLiteral("path"), path},
                                      {QStringLiteral("kind"), QStringLiteral("put")},
                                      {QStringLiteral("hash"), e->sha256},
                                      {QStringLiteral("size"), double(e->size)},
                                      {QStringLiteral("rev"), double(info.resource.isEmpty() ? 0 : newRevs.value(info.resource))}});
    }
    for (const QString &path : removed) {
        logFiles.append(QJsonObject{{QStringLiteral("path"), path}, {QStringLiteral("kind"), QStringLiteral("remove")}});
        eventFiles.append(QJsonObject{{QStringLiteral("path"), path}, {QStringLiteral("kind"), QStringLiteral("remove")}});
    }
    const QJsonObject logEntry{{QStringLiteral("seq"), double(newSeq)},
                               {QStringLiteral("opId"), opId},
                               {QStringLiteral("actor"), actorDevice.isEmpty() ? QStringLiteral("host") : actorDevice},
                               {QStringLiteral("at"), double(at)},
                               {QStringLiteral("ops"), QJsonArray()},
                               {QStringLiteral("files"), logFiles},
                               {QStringLiteral("revs"), changedRevs},
                               {QStringLiteral("job"), true}};
    savebatch::Batch batch(h->m_folder, h->m_folder + QStringLiteral("/settings/journal.json"));
    bool staged = batch.stage(h->m_folder + QStringLiteral("/livesync/state.json"), h->stateJson(newSeq, newRevs, h->m_genReserve),
                              QStringLiteral("state"), QStringLiteral("Live Sync"), &err);
    if (staged)
        staged = batch.stage(h->logPath(newSeq), QJsonDocument(logEntry).toJson(QJsonDocument::Compact), QStringLiteral("log"), QStringLiteral("Live Sync"), &err);
    if (!staged || !batch.commit(&err)) {
        if (!staged)
            batch.drop();
        else
            h->m_broken = true;
        return fail(err);
    }
    h->m_seq = newSeq;
    h->m_revs = newRevs;
    h->m_manifest = now;
    HostSession::StoredResult sr;
    sr.seq = newSeq;
    sr.revs = changedRevs;
    sr.wallMs = at;
    h->m_ops.insert(opId, sr);
    h->m_opOrder << std::make_pair(newSeq, opId);
    h->m_logSeqs << std::make_pair(newSeq, at);
    QJsonObject ev = make(QLatin1String(T::COMMIT_EVENT), h->m_uuid);
    ev.insert(QStringLiteral("epoch"), double(h->m_epoch));
    ev.insert(QStringLiteral("seq"), double(newSeq));
    ev.insert(QStringLiteral("opId"), opId);
    ev.insert(QStringLiteral("actor"), logEntry.value(QStringLiteral("actor")));
    ev.insert(QStringLiteral("actorName"), h->m_hostName);
    ev.insert(QStringLiteral("ops"), QJsonArray());
    ev.insert(QStringLiteral("files"), eventFiles);
    ev.insert(QStringLiteral("revs"), changedRevs);
    ev.insert(QStringLiteral("derived"), QJsonObject());
    for (HostSession::Peer *q : h->m_peers)
        h->sendEvent(q, ev);
    if (seqOut)
        *seqOut = newSeq;
    emit h->committed(newSeq, opId, ev.value(QStringLiteral("actor")).toString());
    return true;
}

// ---------------------------------------------------------------- the client's side

HostJobsClient::HostJobsClient(ClientSession *session, const QString &projectUuid, QObject *parent)
    : QObject(parent), m_session(session), m_uuid(projectUuid), m_remote(this)
{
}

HostJobsClient::~HostJobsClient()
{
    for (Waiting *w : std::as_const(m_waiting))
        w->done = true; // a call that still waits gives up
}

bool HostJobsClient::handle(const QJsonObject &m)
{
    const QString t = m.value(QStringLiteral("t")).toString();
    if (is(t, T::BACKUP_STATUS)) {
        BackupStatus s;
        s.state = stateFrom(m.value(QStringLiteral("state")).toString());
        const qint64 ok = qint64(m.value(QStringLiteral("lastOk")).toDouble());
        if (ok > 0)
            s.lastSuccess = QDateTime::fromMSecsSinceEpoch(ok);
        s.detail = backupReasonText(m.value(QStringLiteral("code")).toString(), m.value(QStringLiteral("detail")).toString().left(500));
        if (s.state == BackupStatus::Done || s.state == BackupStatus::Running)
            s.detail.clear();
        m_status = s;
        emit statusChanged();
        return true;
    }
    if (is(t, T::SNAPSHOT_RESULT)) {
        const QString opId = m.value(QStringLiteral("opId")).toString();
        const bool ok = m.value(QStringLiteral("status")).toString() == QLatin1String("ok");
        const QString code = m.value(QStringLiteral("code")).toString();
        const QString text = m.value(QStringLiteral("text")).toString();
        if (Waiting *w = m_waiting.value(opId, nullptr)) {
            w->done = true;
            w->ok = ok;
            w->code = code;
            w->text = text;
            w->seq = qint64(m.value(QStringLiteral("seq")).toDouble());
        }
        emit jobDone(opId, ok, code, text);
        return true;
    }
    if (is(t, T::ERROR)) {
        const QString re = m.value(QStringLiteral("re")).toString();
        for (auto it = m_waiting.begin(); it != m_waiting.end(); ++it)
            if (it.value()->messageId == re) {
                it.value()->done = true;
                it.value()->code = m.value(QStringLiteral("code")).toString();
                it.value()->text = m.value(QStringLiteral("msg")).toString();
                return true;
            }
    }
    return false;
}

bool HostJobsClient::requestBackup()
{
    if (!m_session || !m_session->isAttached())
        return false;
    return m_session->sendMessage(make(QLatin1String(T::BACKUP_REQUEST), m_uuid));
}

bool HostJobsClient::queryStatus()
{
    if (!m_session || !m_session->isAttached())
        return false;
    return m_session->sendMessage(make(QLatin1String(T::BACKUP_REQUEST), m_uuid, QJsonObject{{QStringLiteral("query"), true}}));
}

static QString jobCodeText(const QString &code, const QString &host)
{
    if (code == QLatin1String("LOCK_HELD"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The document is being edited on %1.").arg(host);
    if (code == QLatin1String("LOCK_NEEDED"))
        return QCoreApplication::translate("qb::collab::HostBackup", "Rolling back needs the document's edit lock.");
    if (code == QLatin1String("READ_ONLY"))
        return QCoreApplication::translate("qb::collab::HostBackup", "This device may only read the project.");
    if (code == QLatin1String("STORAGE_FAILED"))
        return QCoreApplication::translate("qb::collab::HostBackup", "The host could not do it: %1").arg(host);
    return QCoreApplication::translate("qb::collab::HostBackup", "The host refused the snapshot job (%1).").arg(code);
}

bool HostJobsClient::job(const QJsonObject &body, QString *error)
{
    m_lastCode.clear();
    if (!m_session || !m_session->isAttached()) {
        if (error)
            *error = QCoreApplication::translate("qb::collab::HostBackup", "The device that hosts the project cannot be reached now.");
        return false;
    }
    const QString opId = newId();
    QJsonObject b = body;
    b.insert(QStringLiteral("opId"), opId);
    QJsonObject msg = make(QLatin1String(T::SNAPSHOT_OP), m_uuid, b);
    Waiting w;
    w.messageId = msg.value(QStringLiteral("id")).toString();
    m_waiting.insert(opId, &w);
    const bool sent = m_session->sendMessage(msg);
    QElapsedTimer t;
    t.start();
    while (sent && !w.done && t.elapsed() < m_timeoutMs) {
        QEventLoop loop;
        QTimer::singleShot(5, &loop, &QEventLoop::quit);
        loop.exec();
    }
    // the files of the host's commit follow its answer: wait until they are in the copy
    while (sent && w.done && w.ok && m_session && m_session->seq() < w.seq && t.elapsed() < m_timeoutMs) {
        QEventLoop loop;
        QTimer::singleShot(5, &loop, &QEventLoop::quit);
        loop.exec();
    }
    m_waiting.remove(opId);
    if (!sent || !w.done) {
        if (error)
            *error = QCoreApplication::translate("qb::collab::HostBackup", "The host did not answer in time.");
        return false;
    }
    if (!w.ok) {
        m_lastCode = w.code;
        if (error)
            *error = jobCodeText(w.code, w.text);
        return false;
    }
    return true;
}

bool HostJobsClient::Remote::take(const QString &docId, const QString &title, QString *error)
{
    return m_o->job(QJsonObject{{QStringLiteral("op"), QStringLiteral("take")}, {QStringLiteral("doc"), docId}, {QStringLiteral("title"), title}}, error);
}
bool HostJobsClient::Remote::setTitle(const QString &docId, const QString &file, const QString &title, QString *error)
{
    return m_o->job(QJsonObject{{QStringLiteral("op"), QStringLiteral("rename")}, {QStringLiteral("doc"), docId}, {QStringLiteral("file"), file}, {QStringLiteral("title"), title}}, error);
}
bool HostJobsClient::Remote::remove(const QString &docId, const QString &file, QString *error)
{
    return m_o->job(QJsonObject{{QStringLiteral("op"), QStringLiteral("delete")}, {QStringLiteral("doc"), docId}, {QStringLiteral("file"), file}}, error);
}
bool HostJobsClient::Remote::rollBack(const QString &docId, const QString &file, bool allowEmpty, QString *error)
{
    return m_o->job(QJsonObject{{QStringLiteral("op"), QStringLiteral("rollBack")},
                                {QStringLiteral("doc"), docId},
                                {QStringLiteral("file"), file},
                                {QStringLiteral("allowEmpty"), allowEmpty}},
                    error);
}

} // namespace collab
} // namespace qb
