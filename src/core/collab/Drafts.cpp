#include "core/collab/Drafts.h"

#include "core/SafeSave.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMap>
#include <QRegularExpression>
#include <QXmlStreamReader>

namespace qb {
namespace collab {

namespace {
constexpr int kLedgerKeep = 10000;

QByteArray readAll(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

bool writeBytes(const QString &path, const QByteArray &data, QString *error)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    return safesave::writeFile(path, data, error);
}

// "content/<id>/text.md" -> "<id>"; also tells text or notes
bool documentOfPath(const QString &path, QString *id, QString *kind)
{
    static const QRegularExpression re(QStringLiteral("^content/([^/]+)/(text|notes)\\.[A-Za-z0-9]+$"));
    const QRegularExpressionMatch m = re.match(path);
    if (!m.hasMatch())
        return false;
    *id = m.captured(1);
    *kind = m.captured(2);
    return true;
}

QString titleOfElement(const QString &xml)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd()) {
        if (r.readNext() == QXmlStreamReader::StartElement && r.name() == QLatin1String("Title"))
            return r.readElementText();
    }
    return QString();
}
} // namespace

QJsonObject AuthoredCommit::toJson() const
{
    QJsonArray p;
    for (const QString &s : paths)
        p.append(s);
    return QJsonObject{{QStringLiteral("seq"), double(seq)}, {QStringLiteral("epoch"), double(epoch)}, {QStringLiteral("paths"), p}, {QStringLiteral("ops"), ops}};
}

AuthoredCommit AuthoredCommit::fromJson(const QJsonObject &o)
{
    AuthoredCommit c;
    c.seq = qint64(o.value(QStringLiteral("seq")).toDouble());
    c.epoch = qint64(o.value(QStringLiteral("epoch")).toDouble());
    for (const QJsonValue &v : o.value(QStringLiteral("paths")).toArray())
        if (v.isString())
            c.paths << v.toString();
    c.ops = o.value(QStringLiteral("ops")).toArray();
    return c;
}

QJsonObject Draft::toJson() const
{
    return QJsonObject{{QStringLiteral("v"), 1},
                       {QStringLiteral("id"), id},
                       {QStringLiteral("p"), project},
                       {QStringLiteral("doc"), document},
                       {QStringLiteral("title"), title},
                       {QStringLiteral("reason"), reason},
                       {QStringLiteral("author"), author},
                       {QStringLiteral("forkEpoch"), double(forkEpoch)},
                       {QStringLiteral("forkSeq"), double(forkSeq)},
                       {QStringLiteral("created"), double(createdMs)},
                       {QStringLiteral("hasText"), hasText},
                       {QStringLiteral("hasNotes"), hasNotes},
                       {QStringLiteral("textPath"), textPath},
                       {QStringLiteral("notesPath"), notesPath},
                       {QStringLiteral("fields"), fields}};
}

bool Draft::fromJson(const QJsonObject &o, Draft *out)
{
    if (!o.value(QStringLiteral("id")).isString() || o.value(QStringLiteral("id")).toString().isEmpty())
        return false;
    Draft d;
    d.id = o.value(QStringLiteral("id")).toString();
    d.project = o.value(QStringLiteral("p")).toString();
    d.document = o.value(QStringLiteral("doc")).toString();
    d.title = o.value(QStringLiteral("title")).toString();
    d.reason = o.value(QStringLiteral("reason")).toString();
    d.author = o.value(QStringLiteral("author")).toString();
    d.forkEpoch = qint64(o.value(QStringLiteral("forkEpoch")).toDouble());
    d.forkSeq = qint64(o.value(QStringLiteral("forkSeq")).toDouble());
    d.createdMs = qint64(o.value(QStringLiteral("created")).toDouble());
    d.hasText = o.value(QStringLiteral("hasText")).toBool();
    d.hasNotes = o.value(QStringLiteral("hasNotes")).toBool();
    d.textPath = o.value(QStringLiteral("textPath")).toString();
    d.notesPath = o.value(QStringLiteral("notesPath")).toString();
    d.fields = o.value(QStringLiteral("fields")).toArray();
    *out = d;
    return true;
}

QString RecoveryDrafts::idFor(const QString &project, qint64 forkEpoch, qint64 forkSeq, const QString &document, const QString &author)
{
    const QByteArray src = (project + QLatin1Char('|') + QString::number(forkEpoch) + QLatin1Char('|') + QString::number(forkSeq) + QLatin1Char('|') +
                            document + QLatin1Char('|') + author)
                               .toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(src, QCryptographicHash::Sha256).toHex().left(24));
}

QList<Draft> RecoveryDrafts::list() const
{
    QList<Draft> out;
    m_damaged = 0;
    if (m_root.isEmpty())
        return out;
    const QDir dir(m_root + QStringLiteral("/recovery"));
    for (const QString &name : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(readAll(dir.filePath(name) + QStringLiteral("/draft.json")), &err);
        Draft d;
        if (err.error != QJsonParseError::NoError || !doc.isObject() || !Draft::fromJson(doc.object(), &d) || d.id != name) {
            ++m_damaged;
            continue;
        }
        out << d;
    }
    return out;
}

bool RecoveryDrafts::exists(const QString &id) const
{
    return !m_root.isEmpty() && QFileInfo::exists(m_root + QStringLiteral("/recovery/") + id + QStringLiteral("/draft.json"));
}

bool RecoveryDrafts::add(const Draft &d, const QByteArray &text, const QByteArray &notes, QString *error)
{
    if (m_root.isEmpty() || d.id.isEmpty() || d.id.contains(QLatin1Char('/')) || d.id.contains(QLatin1Char('.'))) {
        if (error)
            *error = QStringLiteral("no place for drafts");
        return false;
    }
    const QString base = m_root + QStringLiteral("/recovery/") + d.id;
    // the bytes first, the description last: a draft without its description is not listed, one with it is complete
    if (d.hasText && !writeBytes(base + QStringLiteral("/text"), text, error))
        return false;
    if (d.hasNotes && !writeBytes(base + QStringLiteral("/notes"), notes, error))
        return false;
    return writeBytes(base + QStringLiteral("/draft.json"), QJsonDocument(d.toJson()).toJson(QJsonDocument::Indented), error);
}

QByteArray RecoveryDrafts::text(const QString &id) const { return readAll(m_root + QStringLiteral("/recovery/") + id + QStringLiteral("/text")); }
QByteArray RecoveryDrafts::notes(const QString &id) const { return readAll(m_root + QStringLiteral("/recovery/") + id + QStringLiteral("/notes")); }

bool RecoveryDrafts::remove(const QString &id)
{
    if (m_root.isEmpty() || id.isEmpty() || id.contains(QLatin1Char('/')) || id.contains(QLatin1Char('.')))
        return false;
    return QDir(m_root + QStringLiteral("/recovery/") + id).removeRecursively();
}

QList<AuthoredCommit> authoredFromLog(const QList<QJsonObject> &entries, const QString &deviceId, qint64 afterSeq)
{
    QList<AuthoredCommit> out;
    for (const QJsonObject &e : entries) {
        const qint64 seq = qint64(e.value(QStringLiteral("seq")).toDouble());
        if (seq <= afterSeq || e.value(QStringLiteral("actor")).toString() != deviceId)
            continue;
        AuthoredCommit c;
        c.seq = seq;
        c.epoch = qint64(e.value(QStringLiteral("epoch")).toDouble());
        c.ops = e.value(QStringLiteral("ops")).toArray();
        for (const QJsonValue &f : e.value(QStringLiteral("files")).toArray()) {
            const QJsonObject fo = f.toObject();
            if (fo.value(QStringLiteral("kind")).toString() == QLatin1String("put"))
                c.paths << fo.value(QStringLiteral("path")).toString();
        }
        out << c;
    }
    return out;
}

int makeSplitDrafts(RecoveryDrafts &store, const QString &folder, const QString &project, const QString &author,
                    const QList<AuthoredCommit> &commits, qint64 forkEpoch, qint64 forkSeq, const QString &reason, qint64 nowMs, QString *error)
{
    struct Acc {
        QString title, textPath, notesPath;
        QJsonArray fields;
    };
    QMap<QString, Acc> byDoc; // "" = the project itself
    for (const AuthoredCommit &c : commits) {
        for (const QString &path : c.paths) {
            QString id, kind;
            if (!documentOfPath(path, &id, &kind))
                continue;
            Acc &a = byDoc[id];
            (kind == QLatin1String("text") ? a.textPath : a.notesPath) = path;
        }
        for (const QJsonValue &v : c.ops) {
            const QJsonObject op = v.toObject();
            const QString id = op.value(QStringLiteral("id")).toString();
            Acc &a = byDoc[id];
            a.fields.append(op);
            if (op.value(QStringLiteral("k")).toString() != QLatin1String("item.delete")) {
                const QString t = titleOfElement(op.value(QStringLiteral("element")).toString());
                if (!t.isEmpty())
                    a.title = t;
            }
        }
    }
    int made = 0;
    for (auto it = byDoc.constBegin(); it != byDoc.constEnd(); ++it) {
        Draft d;
        d.project = project;
        d.document = it.key();
        d.title = it->title;
        d.reason = reason;
        d.author = author;
        d.forkEpoch = forkEpoch;
        d.forkSeq = forkSeq;
        d.createdMs = nowMs;
        d.fields = it->fields;
        d.id = RecoveryDrafts::idFor(project, forkEpoch, forkSeq, it.key(), author);
        QByteArray text, notes;
        if (!it->textPath.isEmpty()) {
            QFile f(folder + QLatin1Char('/') + it->textPath);
            if (f.open(QIODevice::ReadOnly)) {
                text = f.readAll();
                d.hasText = true;
                d.textPath = it->textPath;
            }
        }
        if (!it->notesPath.isEmpty()) {
            QFile f(folder + QLatin1Char('/') + it->notesPath);
            if (f.open(QIODevice::ReadOnly)) {
                notes = f.readAll();
                d.hasNotes = true;
                d.notesPath = it->notesPath;
            }
        }
        if (!d.hasText && !d.hasNotes && d.fields.isEmpty())
            continue;
        if (!store.add(d, text, notes, error))
            return -1;
        ++made;
    }
    return made;
}

bool CommitLedger::load()
{
    m_list.clear();
    QFile f(m_path);
    if (!f.exists())
        return true;
    if (!f.open(QIODevice::ReadOnly))
        return false;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;
    for (const QJsonValue &v : doc.object().value(QStringLiteral("commits")).toArray())
        m_list << AuthoredCommit::fromJson(v.toObject());
    return true;
}

bool CommitLedger::save(QString *error)
{
    QJsonArray a;
    for (const AuthoredCommit &c : m_list)
        a.append(c.toJson());
    return writeBytes(m_path, QJsonDocument(QJsonObject{{QStringLiteral("v"), 1}, {QStringLiteral("commits"), a}}).toJson(QJsonDocument::Compact), error);
}

bool CommitLedger::add(const AuthoredCommit &c, QString *error)
{
    m_list << c;
    while (m_list.size() > kLedgerKeep)
        m_list.removeFirst();
    return save(error);
}

QList<AuthoredCommit> CommitLedger::after(qint64 afterSeq, qint64 beforeEpoch) const
{
    QList<AuthoredCommit> out;
    for (const AuthoredCommit &c : m_list)
        if (c.seq > afterSeq && c.epoch < beforeEpoch)
            out << c;
    return out;
}

bool CommitLedger::forget(qint64 afterSeq, qint64 beforeEpoch, QString *error)
{
    QList<AuthoredCommit> keep;
    for (const AuthoredCommit &c : m_list)
        if (!(c.seq > afterSeq && c.epoch < beforeEpoch))
            keep << c;
    m_list = keep;
    return save(error);
}

} // namespace collab
} // namespace qb
