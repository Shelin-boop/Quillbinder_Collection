// Live Sync (0.99.0), the last writers into a project folder routed through the gate: the removal of a mind map's file
// (Board::save of an emptied board), the removal of an object's file and of replaced pictures (Objects::Store::save), the
// aside copy of a damaged schema, and the folder an import that fails removes again. In a follower each is captured and
// the disk stays as it was; with Live Sync off the same actions change the files as before.
//
//   xvfb-run -a ./qb_lsroute_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/MindMap.h"
#include "core/Objects.h"
#include "core/Project.h"
#include "core/collab/Gate.h"
#include "import/FileImporter.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QTemporaryDir>
#include <QTextDocument>

#include <cstdio>
#include <memory>

using namespace qb;
using namespace qb::collab;

static int failures = 0;
static int passes = 0;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (ok) {
        ++passes;
    } else {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static const FileChange *findIn(const QList<ChangeSet> &sets, const QString &path)
{
    for (const ChangeSet &s : sets)
        if (const FileChange *c = s.find(path))
            return c;
    return nullptr;
}

static QStringList pathsOf(const QList<ChangeSet> &sets)
{
    QStringList out;
    for (const ChangeSet &s : sets)
        out << s.paths();
    return out;
}

static bool fileHas(const QString &path, const char *text)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) && f.readAll().contains(text);
}

static void notesBoard(mindmap::Board &b)
{
    mindmap::Note n;
    n.id = 1;
    n.text = QStringLiteral("An idea");
    b.notes.append(n);
}

struct Made {
    std::unique_ptr<Project> p;
    QString doc, folder, mapFile, objFile, objId, picFile, cat;
};

// a project with a document, a mind map file on disk, an object with a picture on disk
static Made make(const QString &parent, const QString &name)
{
    Made m;
    QString err;
    m.p = Project::create(parent, name, &err);
    if (!m.p)
        return m;
    BinderItem *a = m.p->model()->createItem(m.p->draft(), -1, ItemType::Text, QStringLiteral("One"));
    m.doc = a->id;
    m.p->docs()->text(a->id)->setPlainText(QStringLiteral("Words."));
    m.folder = m.p->folderPath();
    m.mapFile = mindmap::pathFor(m.p->contentDir(a->id));
    mindmap::Board b;
    notesBoard(b);
    b.save(m.mapFile, &err);
    // an object of a category with a picture field, and its picture on disk
    QString picField;
    for (const objects::Category &c : m.p->objects()->schema().categories)
        for (const objects::FieldDef &fd : c.fields)
            if (picField.isEmpty() && fd.type == objects::FieldType::Picture) {
                m.cat = c.id;
                picField = fd.id;
            }
    m.objId = m.p->objects()->create(m.cat, QString(), QStringLiteral("Someone"));
    QImage img(8, 8, QImage::Format_RGB32);
    img.fill(Qt::red);
    const QString src = parent + QLatin1String("/pic.png");
    img.save(src);
    const QString pn = m.p->objects()->importPicture(m.objId, picField, src);
    objects::Object o = *m.p->objects()->object(m.objId);
    o.values.insert(picField, pn);
    m.p->objects()->update(o, nullptr);
    m.picFile = m.p->objects()->picturePath(pn);
    m.p->save(&err);
    return m;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_UPDATE_CHECK", "1");
    qputenv("QB_TEST_KEYSTORE", "memory");
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    QGuiApplication app(argc, argv);
    QTemporaryDir tmp;

    // ---- Live Sync off: the actions change the files as before
    {
        Made m = make(tmp.path() + QLatin1String("/off"), QStringLiteral("Off"));
        check(bool(m.p) && QFileInfo::exists(m.mapFile) && QFileInfo::exists(m.picFile), QStringLiteral("the project is made with a map file and a picture"));
        QString err;
        mindmap::Board empty;
        check(empty.save(m.mapFile, &err) && !QFileInfo::exists(m.mapFile), QStringLiteral("off: an emptied board removes its file"), err);
        const QString objFile = m.p->folderPath() + QLatin1String("/objects/") + m.cat + QLatin1Char('/') + m.objId + QLatin1String(".json");
        check(QFileInfo::exists(objFile), QStringLiteral("off: the object's file exists"), objFile);
        m.p->objects()->remove(m.objId);
        m.p->save(&err);
        check(!QFileInfo::exists(objFile), QStringLiteral("off: a removed object's file goes at the save"));
        check(!QFileInfo::exists(m.picFile), QStringLiteral("off: the removed object's picture goes at the save"));
    }

    // ---- a follower
    Made m = make(tmp.path() + QLatin1String("/on"), QStringLiteral("On"));
    const QString folder = m.folder;
    const QString doc = m.doc;
    const QString objRel = QLatin1String("objects/") + m.cat + QLatin1Char('/') + m.objId + QLatin1String(".json");
    const QString mapRel = QLatin1String("content/") + doc + QLatin1String("/mindmap.scap");
    const QString picRel = QDir(folder).relativeFilePath(m.picFile);
    check(QFileInfo::exists(folder + QLatin1Char('/') + objRel) && QFileInfo::exists(m.mapFile) && QFileInfo::exists(m.picFile),
          QStringLiteral("the files to remove exist on disk"), objRel + QLatin1Char(' ') + picRel);
    m.p.reset();

    Follower f;
    QString err;
    auto p = Project::open(folder, &err, &f);
    check(bool(p) && p->isFollower(), QStringLiteral("the project opens as a follower"), err);
    auto take = [&]() { f.capture().flush(); return f.capture().takeReady(); };
    take();

    // the mind map: an emptied board
    {
        mindmap::Board empty;
        const QString mf = mindmap::pathFor(p->contentDir(doc));
        check(empty.save(mf, &err), QStringLiteral("follower: saving an emptied board succeeds"), err);
        const auto sets = take();
        const FileChange *c = findIn(sets, mapRel);
        check(c && c->kind == FileChange::Remove && QFileInfo::exists(mf), QStringLiteral("follower: the removal of the mind map's file is captured, the file stays"), pathsOf(sets).join(QLatin1Char(',')));
        mindmap::Board b;
        notesBoard(b);
        b.notes[0].text = QStringLiteral("Changed");
        check(b.save(mf, &err), QStringLiteral("follower: saving a board succeeds"), err);
        const auto sets2 = take();
        const FileChange *c2 = findIn(sets2, mapRel);
        check(c2 && c2->kind == FileChange::Put && c2->data.contains("Changed") && !fileHas(mf, "Changed"), QStringLiteral("follower: a changed board is captured, not written"));
    }

    // the object: its file and its picture
    {
        p->objects()->remove(m.objId);
        check(p->save(&err), QStringLiteral("follower: an object is removed and saved"), err);
        const auto sets = take();
        const FileChange *c = findIn(sets, objRel);
        check(c && c->kind == FileChange::Remove && QFileInfo::exists(folder + QLatin1Char('/') + objRel),
              QStringLiteral("follower: the removal of an object's file is captured, the file stays"), pathsOf(sets).join(QLatin1Char(',')));
        // the picture the removed object named goes with it
        const QString pr = QDir(folder).relativeFilePath(m.picFile);
        const FileChange *pc = findIn(sets, pr);
        check(pc && pc->kind == FileChange::Remove && QFileInfo::exists(m.picFile),
              QStringLiteral("follower: the removal of the removed object's picture is captured, the file stays"), pr + QLatin1Char(' ') + pathsOf(sets).join(QLatin1Char(',')));
    }

    // an import whose copy fails: the folder it made is removed through the gate
    {
        qputenv("QB_TEST_FAIL_COPY", "failme");
        const QString src = tmp.path() + QLatin1String("/failme.png");
        QImage img(4, 4, QImage::Format_RGB32);
        img.fill(Qt::blue);
        img.save(src);
        QStringList warnings;
        const QStringList dirsBefore = QDir(folder + QLatin1String("/content")).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        BinderItem *it = importer::importMedia(p.get(), src, p->research(), -1, &warnings);
        qunsetenv("QB_TEST_FAIL_COPY");
        const auto sets = take();
        bool treeRemoved = false;
        for (const ChangeSet &cs : sets)
            for (const QString &pa : cs.paths())
                if (pa.startsWith(QLatin1String("content/")) && cs.find(pa) && cs.find(pa)->kind == FileChange::RemoveTree)
                    treeRemoved = true;
        // the folder the import made stays on disk (its removal is the service's); before the routing it was removed here
        const QStringList dirsAfter = QDir(folder + QLatin1String("/content")).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        check(!it && !warnings.isEmpty() && treeRemoved && dirsAfter.size() == dirsBefore.size() + 1, QStringLiteral("follower: a failed import's folder removal is captured"),
              warnings.join(QLatin1Char('|')) + QLatin1Char(' ') + pathsOf(sets).join(QLatin1Char(',')));
    }

    std::printf("%d pass\n%d failure(s)\n", passes, failures);
    return failures ? 1 : 0;
}
