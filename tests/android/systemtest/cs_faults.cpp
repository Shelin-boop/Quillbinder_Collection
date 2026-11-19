// The system test for Android: deliberate conflicts between two synchronized computers (C-39) and helpers other cases
// use. Linux's fault injection (C-34 to C-36, C-38, C-43, S-D06, S-N01: libqbfault.so loaded with LD_PRELOAD) is not in
// Android's copy (2026-11-16): an app cannot load such a library into itself on a tablet.
#include "cs.h"

#include "compile/CompileFormat.h"
#include "compile/Compiler.h"
#include "compile/DocxWriter.h"
#include "compile/PdfExporter.h"
#include "core/Backup.h"
#include "core/MindMap.h"
#include "core/Objects.h"
#include "core/SettingsFile.h"
#include "core/Snapshots.h"
#include "core/SyncSafety.h"
#include "core/Zip.h"
#include "import/ScrivenerFormat.h"
#include "ui/AppSettings.h"
#include "ui/DragDrop.h"
#include "ui/EditorPane.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QLabel>
#include <QAbstractButton>
#include <QDir>
#include <QEvent>
#include <QJsonArray>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QUuid>

#include <dlfcn.h>
#include <random>

namespace cs {

using namespace qb;
using namespace qb::compile;

QString scrivUuidOf(const QString &id)
{
    const QUuid u = QUuid::fromString(id);
    if (!u.isNull())
        return u.toString(QUuid::WithoutBraces).toUpper();
    static const QUuid ns = QUuid::fromString(QStringLiteral("6b1f4a36-3c55-4b55-9a4e-5d9e3c2b7a10"));
    return QUuid::createUuidV5(ns, id).toString(QUuid::WithoutBraces).toUpper();
}

// silent comparison of two models (compareModels reports a failure itself): "" when equal
QString modelDifference(const QJsonObject &a, const QJsonObject &b)
{
    for (auto it = a.begin(); it != a.end(); ++it) {
        if (!b.contains(it.key()))
            return QStringLiteral("missing ") + it.key();
        const QJsonObject x = it.value().toObject(), y = b.value(it.key()).toObject();
        for (auto f = x.begin(); f != x.end(); ++f)
            if (y.value(f.key()) != f.value())
                return QStringLiteral("%1: %2").arg(it.key(), f.key());
    }
    for (auto it = b.begin(); it != b.end(); ++it)
        if (!a.contains(it.key()))
            return QStringLiteral("new ") + it.key();
    return QString();
}

namespace {

QString modelDiff(const QJsonObject &a, const QJsonObject &b) { return modelDifference(a, b); }

// the key of an item without the marks the cases add to titles (" [C34-3]", " [C36]")
QString unmarked(QString key)
{
    static const QRegularExpression mark(QStringLiteral(" \\[C3[46][^\\]]*\\]"));
    return key.remove(mark);
}

// the fields on disk that are neither their old nor their new value (a third state: cut off, mixed up),
// and the items missing or come from nowhere; each field may be old or new on its own
QStringList outsideOldAndNew(const QJsonObject &before, const QJsonObject &after, const QJsonObject &disk)
{
    QHash<QString, QJsonObject> b, a;
    for (auto it = before.begin(); it != before.end(); ++it)
        b.insert(unmarked(it.key()), it.value().toObject());
    for (auto it = after.begin(); it != after.end(); ++it)
        a.insert(unmarked(it.key()), it.value().toObject());
    QStringList out;
    QSet<QString> seen;
    for (auto it = disk.begin(); it != disk.end(); ++it) {
        const QString k = unmarked(it.key());
        seen.insert(k);
        if (!b.contains(k) && !a.contains(k)) {
            out << QStringLiteral("from nowhere: ") + it.key();
            continue;
        }
        const QJsonObject d = it.value().toObject(), o = b.value(k), n = a.value(k);
        for (auto f = d.begin(); f != d.end(); ++f)
            if (f.value() != o.value(f.key()) && f.value() != n.value(f.key()))
                out << QStringLiteral("%1: %2 (old %3, new %4, now %5)")   // 0.64.2: the values too
                           .arg(it.key(), f.key(), QString::fromUtf8(QJsonDocument(QJsonArray{o.value(f.key())}).toJson(QJsonDocument::Compact)).left(60),
                                QString::fromUtf8(QJsonDocument(QJsonArray{n.value(f.key())}).toJson(QJsonDocument::Compact)).left(60),
                                QString::fromUtf8(QJsonDocument(QJsonArray{f.value()}).toJson(QJsonDocument::Compact)).left(60));
    }
    for (auto it = b.begin(); it != b.end(); ++it)
        if (!seen.contains(it.key()) && a.contains(it.key()))
            out << QStringLiteral("missing: ") + it.key();
    return out;
}

// what is on disk, read by a new project object (Scrivener format: the .scriv read afresh)
QJsonObject diskModel(const QString &path, QString *err)
{
    if (path.endsWith(QLatin1String(".scriv"))) {
        const QByteArray keep = qgetenv("XDG_DATA_HOME");
        const QString fresh = ctx().work + QStringLiteral("/fresh-%1").arg(QUuid::createUuid().toString(QUuid::Id128).left(8));
        qputenv("XDG_DATA_HOME", fresh.toUtf8());
        auto r = scrivener::open(path, err);
        QJsonObject m = r.project ? modelOf(r.project.get(), true) : QJsonObject();
        r.project.reset();
        qputenv("XDG_DATA_HOME", keep);
        QDir(fresh).removeRecursively();
        return m;
    }
    auto p = Project::open(path, err);
    return p ? modelOf(p.get(), false) : QJsonObject();
}

std::unique_ptr<Project> openFixture(const QString &fx, QString *path)
{
    QString err;
    *path = fixtureCopy(fx);
    if (fx.endsWith(QLatin1String("scriv")))
        return std::move(scrivener::open(*path, &err).project);
    return Project::open(*path, &err);
}

struct Target {
    QString what;
    std::function<QString(Project *)> pattern;   // a part of the path of the file that gets the fault
};

QList<Target> targetsFor(bool scriv)
{
    QList<Target> t;
    if (!scriv) {
        t << Target{QStringLiteral("the text files"), [](Project *p) { return QFileInfo(p->docs()->textPath(textItems(p).value(1)->id)).absolutePath() + QLatin1Char('/'); }};
        t << Target{QStringLiteral("the project file"), [](Project *p) { return QFileInfo(p->indexPath()).fileName(); }};
        t << Target{QStringLiteral("the notes"), [](Project *p) { return QFileInfo(p->docs()->notesPath(textItems(p).value(1)->id)).fileName(); }};
        t << Target{QStringLiteral("every file"), [](Project *p) { return p->folderPath(); }};
    } else {
        t << Target{QStringLiteral("content.rtf in the .scriv"), [](Project *p) { return scrivUuidOf(textItems(p).value(1)->id) + QStringLiteral("/content"); }};
        t << Target{QStringLiteral("the .scrivx"), [](Project *) { return QStringLiteral(".scrivx"); }};
        t << Target{QStringLiteral("docs.checksum"), [](Project *) { return QStringLiteral("docs.checksum"); }};
        t << Target{QStringLiteral("the working copy"), [](Project *p) { return p->folderPath(); }};
        t << Target{QStringLiteral("every file of the .scriv"), [](Project *p) { return p->scrivenerPath; }};
    }
    return t;
}

const QList<QPair<QString, QString>> &kinds()
{
    static const QList<QPair<QString, QString>> k = {
        {QStringLiteral("disk full at the first byte"), QStringLiteral("write %1 ENOSPC 0")},
        {QStringLiteral("disk full after 700 bytes"), QStringLiteral("write %1 ENOSPC 700")},
        {QStringLiteral("quota exceeded"), QStringLiteral("write %1 EDQUOT 0")},
        {QStringLiteral("no permission to create"), QStringLiteral("open %1 EACCES 0")},
        {QStringLiteral("read-only file system at the rename"), QStringLiteral("rename %1 EROFS 0")},
        {QStringLiteral("an I/O error at fsync"), QStringLiteral("fsync %1 EIO 0")},
        {QStringLiteral("an I/O error at close"), QStringLiteral("close %1 EIO 0")},
        // 0.64.1: a file system that cannot sync; the program counts these as success (SafeSave.h)
        {QStringLiteral("fsync answered EINVAL"), QStringLiteral("fsync %1 EINVAL 0")},
        {QStringLiteral("fsync answered ENOTSUP (full profile)"), QStringLiteral("fsync %1 ENOTSUP 0")},
        {QStringLiteral("fsync answered ENOSYS (full profile)"), QStringLiteral("fsync %1 ENOSYS 0")},
    };
    return k;
}

int falseSuccess = 0;

// the verdict of one operation under a fault: success must be true on disk, failure must say so and leave
// the old or the new state, never a third one
void judge(const QString &what, const QString &rule, bool ok, const QString &err, const QStringList &fired, const QJsonObject &before,
           const QJsonObject &after, const QJsonObject &disk)
{
    if (fired.isEmpty()) {
        note(QStringLiteral("%1: the fault did not hit (no file matched)").arg(what));
        return;
    }
    // 0.64.1: fsync answered EINVAL, ENOTSUP or ENOSYS (a file system that cannot sync) is no error for
    // the program: the save must succeed and the disk hold the saved state
    if (rule.startsWith(QLatin1String("fsync ")) && (rule.contains(QLatin1String(" EINVAL")) || rule.contains(QLatin1String(" ENOTSUP"))
                                                   || rule.contains(QLatin1String(" ENOSYS")))) {
        check(ok, QStringLiteral("%1: the save succeeds (a file system that cannot sync is no error)").arg(what), err);
        if (ok) {
            const QString d = modelDiff(after, disk);
            check(d.isEmpty(), QStringLiteral("%1: the disk holds the saved state").arg(what), d);
        }
        return;
    }
    // an I/O error at fsync or close means the data may never reach the disk: success is false whatever
    // the page cache shows now
    if (ok && (rule.startsWith(QLatin1String("fsync ")) || rule.startsWith(QLatin1String("close ")))) {
        ++falseSuccess;
        check(false, QStringLiteral("%1: no success after an I/O error at %2").arg(what, rule.section(QLatin1Char(' '), 0, 0)),
              QStringLiteral("FALSE SUCCESS: the save reported success (faults: %1)").arg(fired.mid(0, 3).join(QLatin1String("; "))));
        return;
    }
    if (ok) {
        const QString d = modelDiff(after, disk);
        if (!d.isEmpty())
            ++falseSuccess;
        check(d.isEmpty(), QStringLiteral("%1: reported success, and the disk holds the saved state").arg(what),
              QStringLiteral("FALSE SUCCESS: %1 (faults: %2)").arg(d, fired.mid(0, 3).join(QLatin1String("; "))));
    } else {
        check(!err.isEmpty(), QStringLiteral("%1: reported failure with a message").arg(what), err);
        const QStringList third = outsideOldAndNew(before, after, disk);
        check(third.isEmpty(), QStringLiteral("%1: every field on disk is its old or its new value").arg(what), third.mid(0, 5).join(QLatin1String(", ")));
    }
}

} // namespace

// ================================================================ C-39 deliberate conflicts between two synchronized computers
namespace {
QDateTime syncFiles(const QString &from, const QString &to, const QDateTime &since)
{
    const QDateTime now = QDateTime::currentDateTime();
    QDirIterator it(from, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString f = it.next();
        const QString rel = QDir(from).relativeFilePath(f);
        if (rel.endsWith(QLatin1String("lock")) || QFileInfo(f).lastModified() < since)
            continue;
        const QString t = to + QLatin1Char('/') + rel;
        QDir().mkpath(QFileInfo(t).absolutePath());
        QFile::remove(t);
        QFile::copy(f, t);
    }
    // deletions: files of `to` that `from` no longer has, among the texts
    QDirIterator jt(to, QDir::Files, QDirIterator::Subdirectories);
    while (jt.hasNext()) {
        const QString t = jt.next();
        const QString rel = QDir(to).relativeFilePath(t);
        if (rel.startsWith(QLatin1String("content/")) && !QFileInfo::exists(from + QLatin1Char('/') + rel))
            QFile::remove(t);
    }
    return now;
}

QString allText(const QString &folder, Project *p)
{
    QString s;
    if (p)
        for (BinderItem *it : p->allItems()) {
            s += it->title + QLatin1Char('\n') + it->synopsis + QLatin1Char('\n');
            if (it->hasText())
                s += p->docs()->text(it->id)->toPlainText() + QLatin1Char('\n') + p->docs()->notes(it->id)->toPlainText() + QLatin1Char('\n');
        }
    for (const QString &sub : {QStringLiteral("/settings/conflicts"), QStringLiteral("/snapshots")}) {
        QDirIterator di(folder + sub, QDir::Files, QDirIterator::Subdirectories);
        while (di.hasNext())
            s += readText(di.next());
    }
    return s;
}
} // namespace

void caseC39()
{
    struct Scenario {
        QString what;
        std::function<void(Project *, const QString &)> onDesktop, onLaptop;
    };
    auto appendText = [](int n) {
        return [n](Project *p, const QString &mark) {
            QTextCursor c(p->docs()->text(textItems(p).value(n)->id));
            c.movePosition(QTextCursor::End);
            c.insertText(QLatin1Char(' ') + mark);
        };
    };
    const QList<Scenario> scenarios = {
        {QStringLiteral("the same scene typed into on both"), appendText(5), appendText(5)},
        {QStringLiteral("the same title renamed differently"), [](Project *p, const QString &m) { BinderItem *it = textItems(p).value(6); it->title = m; p->notifyItemChanged(it); },
         [](Project *p, const QString &m) { BinderItem *it = textItems(p).value(6); it->title = m; p->notifyItemChanged(it); }},
        {QStringLiteral("a scene deleted on one, edited on the other"), [](Project *p, const QString &) { p->model()->moveToTrash({textItems(p).value(7)}); p->model()->emptyTrash(); },
         appendText(7)},
        // 0.64.2: the move carries the mark in the title (a move alone has no text the check could find)
        {QStringLiteral("a scene moved on one, edited on the other"), [](Project *p, const QString &m) { BinderItem *it = textItems(p).value(8); it->title += QLatin1Char(' ') + m; p->notifyItemChanged(it); p->model()->moveItems({it}, p->draft()->children.value(2), 0); },
         appendText(8)},
        {QStringLiteral("new documents on both"), [](Project *p, const QString &m) { BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, m); fillText(p, it, {m}); },
         [](Project *p, const QString &m) { BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, m); fillText(p, it, {m}); }},
        {QStringLiteral("notes and synopsis on both"), [](Project *p, const QString &m) { BinderItem *it = textItems(p).value(9); it->synopsis += m; QTextCursor(p->docs()->notes(it->id)).insertText(m); p->notifyItemChanged(it); },
         [](Project *p, const QString &m) { BinderItem *it = textItems(p).value(9); it->synopsis += m; QTextCursor(p->docs()->notes(it->id)).insertText(m); p->notifyItemChanged(it); }},
        {QStringLiteral("label and status on both"), [](Project *p, const QString &) { BinderItem *it = textItems(p).value(10); it->label = 1; it->status = 1; p->notifyItemChanged(it); },
         [](Project *p, const QString &) { BinderItem *it = textItems(p).value(10); it->label = 2; it->status = 2; p->notifyItemChanged(it); }},
    };
    int n = 0;
    for (const Scenario &sc : scenarios) {
        ++n;
        step(sc.what);
        const QString src = fixtureCopy(QStringLiteral("fx1-qb"));
        const QString srcFolder = QFileInfo(src).absolutePath();
        const QString rel = QFileInfo(src).fileName();
        const QString desk = ctx().work + QStringLiteral("/c39-%1-desktop").arg(n), lap = ctx().work + QStringLiteral("/c39-%1-laptop").arg(n);
        QDir(desk).removeRecursively();
        QDir(lap).removeRecursively();
        copyTree(srcFolder, desk);
        copyTree(srcFolder, lap);
        QString err;
        // both open (the writer forgot to close one), both edit, the desktop saves first
        qputenv("QB_DEVICE_NAME", "desktop");
        auto a = Project::open(desk + QLatin1Char('/') + rel, &err);
        qputenv("QB_DEVICE_NAME", "laptop");
        auto b = Project::open(lap + QLatin1Char('/') + rel, &err);
        if (!a || !b)
            continue;
        int notices = 0;
        QObject::connect(b.get(), &Project::conflictKept, [&notices](const QString &, const QString &, const QString &, bool) { ++notices; });
        const QString ma = QStringLiteral("DESK%1").arg(n), mb = QStringLiteral("LAP%1").arg(n);
        // the texts are read before the other side's files arrive
        for (BinderItem *it : textItems(b.get()))
            b->docs()->text(it->id);
        sc.onDesktop(a.get(), ma);
        sc.onLaptop(b.get(), mb);
        qputenv("QB_DEVICE_NAME", "desktop");
        const QDateTime t0 = QDateTime::currentDateTime().addSecs(-2);
        a->save(&err);
        settle(1100);
        syncFiles(desk, lap, t0);
        qputenv("QB_DEVICE_NAME", "laptop");
        const bool ok = b->save(&err);
        const QDateTime t1 = QDateTime::currentDateTime().addSecs(-2);
        settle(1100);
        syncFiles(lap, desk, t1);
        b.reset();
        a.reset();
        qunsetenv("QB_DEVICE_NAME");
        auto final = Project::open(desk + QLatin1Char('/') + rel, &err);
        const QString inProject = allText(QString(), final.get());
        const QString all = allText(desk, final.get()) + allText(lap, nullptr);
        auto where = [&](const QString &m) {
            return inProject.contains(m) ? QStringLiteral("in the project") : all.contains(m) ? QStringLiteral("only in a kept version") : QStringLiteral("LOST");
        };
        fact(QStringLiteral("C-39 %1").arg(sc.what), QStringLiteral("laptop's save %1, notices %2; the desktop's change %3, the laptop's change %4")
                                                          .arg(ok ? QStringLiteral("ok") : err.left(80)).arg(notices).arg(where(ma), where(mb)));
        check(final != nullptr, QStringLiteral("%1: the project opens after both computers saved").arg(sc.what), err);
        if (sc.what.contains(QLatin1String("deleted")) || sc.what.contains(QLatin1String("label")))
            continue;   // recorded: which of the two wins
        check(all.contains(ma) && all.contains(mb), QStringLiteral("%1: both versions are in the project or kept").arg(sc.what));
    }
    // the sync program's own conflict copies in the folder
    const QString src = fixtureCopy(QStringLiteral("fx1-qb"));
    const QString folder = QFileInfo(src).absolutePath();
    QString err;
    auto p = Project::open(src, &err);
    const QString text = p ? p->docs()->textPath(textItems(p.get()).value(2)->id) : QString();
    p.reset();
    const QStringList strays = {QFileInfo(text).absolutePath() + QStringLiteral("/text (laptop's conflicted copy 2026-11-09).md"),
                                QFileInfo(src).absolutePath() + QLatin1Char('/') + QFileInfo(src).completeBaseName() + QStringLiteral("-LAPTOP.qbx"),
                                QFileInfo(text).absolutePath() + QStringLiteral("/text-PC.md")};
    for (const QString &s : strays)
        writeText(s, "A copy made by the sync program.\n");
    auto q = Project::open(src, &err);
    check(q != nullptr, QStringLiteral("the project opens with the sync program's conflict copies in its folder"), err);
    if (q) {
        int listed = 0;
        for (BinderItem *it : q->allItems())
            if (it->title.contains(QLatin1String("conflicted")) || it->title.contains(QLatin1String("-PC")))
                ++listed;
        fact(QStringLiteral("C-39 the sync program's copies listed as documents"), listed);
        q->save(&err);
        q.reset();
    }
    int left = 0;
    for (const QString &s : strays)
        left += QFileInfo::exists(s) ? 1 : 0;
    check(left == strays.size(), QStringLiteral("the sync program's copies are not deleted"), QString::number(left));
    Q_UNUSED(folder)
}

// ================================================================ S-D06 faults through the user interface
namespace {
// 0.64.2: every message box shown while a command runs (a box that the timed answer below did not catch, because
// the command's menu took longer than its tries: the Scrivener variant's File > Save told the writer, and the
// case counted it as silent)
struct BoxWatch : QObject {
    QStringList texts;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Show)
            if (auto *b = qobject_cast<QMessageBox *>(o))
                texts << b->text();
        return false;
    }
};
} // namespace
} // namespace cs
