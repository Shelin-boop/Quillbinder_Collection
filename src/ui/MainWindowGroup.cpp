// 0.94.0: commands of the binder that have an undo step of their own: Documents > Group (item 76) and Documents > Convert > Sheet
// to Object (item 67). Edit > Undo and Redo (and the toolbar's lists) work on these steps while the keyboard is in the binder.
#include "ui/MainWindow.h"
#include "core/crypto/ProjectCrypto.h"

#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Objects.h"
#include "core/Project.h"
#include "core/TestSwitches.h"
#include "ui/BinderPanel.h"
#include "ui/Inspector.h"
#include "ui/ObjectsUi.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QInputDialog>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <QTimer>

#include <algorithm>
#include <memory>

namespace qb {

// ------------------------------------------------------------------ the binder's undo steps

void MainWindow::pushBinderStep(const QString &name, std::function<void()> undo, std::function<void()> redo)
{
    while (int(m_binderSteps.size()) > m_binderDone)
        m_binderSteps.removeLast();
    m_binderSteps.append({name, std::move(undo), std::move(redo)});
    if (m_binderSteps.size() > 50)
        m_binderSteps.removeFirst();
    m_binderDone = int(m_binderSteps.size());
    updateUndoNames();
}

void MainWindow::clearBinderSteps()
{
    m_binderSteps.clear();
    m_binderDone = 0;
}

bool MainWindow::binderHasKeyboard() const
{
    QWidget *fw = QApplication::focusWidget();
    return fw && m_binder && m_binder->isAncestorOf(fw) && qobject_cast<QAbstractItemView *>(fw);
}

bool MainWindow::binderUndo()
{
    if (m_binderDone <= 0)
        return false;
    const BinderStep step = m_binderSteps.at(m_binderDone - 1);
    --m_binderDone;
    if (step.undo)
        step.undo();
    updateUndoNames();
    updateActions();
    return true;
}

bool MainWindow::binderRedo()
{
    if (m_binderDone >= int(m_binderSteps.size()))
        return false;
    const BinderStep step = m_binderSteps.at(m_binderDone);
    ++m_binderDone;
    if (step.redo)
        step.redo();
    updateUndoNames();
    updateActions();
    return true;
}

// ------------------------------------------------------------------ Documents > Group

namespace {

// the place of an item in the binder: the rows from the top
QList<int> pathOf(const BinderItem *it)
{
    QList<int> p;
    for (const BinderItem *b = it; b && b->parent; b = b->parent)
        p.prepend(b->row());
    return p;
}

bool binderBefore(const BinderItem *a, const BinderItem *b) { return pathOf(a) < pathOf(b); }

struct Home {
    QString id, parentId;
    int row = 0;
};

} // namespace

BinderItem *MainWindow::groupItems(const QList<BinderItem *> &in)
{
    if (!m_project)
        return nullptr;
    QList<BinderItem *> items;
    for (BinderItem *it : BinderModel::topLevelOnly(in))
        if (it && !it->isRootFolder() && !m_project->isInTrash(it))
            items << it;
    if (items.isEmpty())
        return nullptr;
    std::sort(items.begin(), items.end(), binderBefore);
    BinderItem *first = items.first();
    BinderItem *parent = first->parent;
    const int row0 = first->row();
    QList<Home> homes;
    for (BinderItem *it : items)
        homes.append({it->id, it->parent->id, it->row()});
    const QString parentId = parent->id;
    auto folderId = std::make_shared<QString>();
    auto folderTitle = std::make_shared<QString>(tr("Group"));

    // does the work: the folder at the place of the first item, the items into it (also the redo)
    auto doGroup = [this, homes, parentId, row0, folderId, folderTitle]() -> BinderItem * {
        BinderItem *par = m_project->item(parentId);
        if (!par)
            return nullptr;
        BinderItem *folder = m_project->model()->createItem(par, row0, ItemType::Folder, *folderTitle);
        *folderId = folder->id;
        QList<BinderItem *> list;
        for (const Home &h : homes)
            if (BinderItem *it = m_project->item(h.id))
                list << it;
        m_project->model()->moveItems(list, folder, 0);
        return folder;
    };
    auto undoGroup = [this, homes, folderId, folderTitle]() {
        BinderItem *folder = m_project->item(*folderId);
        if (!folder)
            return;
        *folderTitle = folder->title;
        // back to the old places, the first of each parent first; the folder still sits where the first item was
        QList<Home> sorted = homes;
        std::sort(sorted.begin(), sorted.end(), [](const Home &a, const Home &b) { return a.row < b.row; });
        for (const Home &h : std::as_const(sorted)) {
            BinderItem *it = m_project->item(h.id);
            BinderItem *par = m_project->item(h.parentId);
            if (!it || !par)
                continue;
            const int target = h.row + (par == folder->parent ? 1 : 0);
            m_project->model()->moveItems({it}, par, qMin(target, int(par->children.size())));
        }
        // whatever the writer put into the folder since stays: it goes where the folder was
        BinderItem *outer = folder->parent;
        const int at = folder->row();
        if (!folder->children.isEmpty()) {
            const QList<BinderItem *> rest = folder->children;
            m_project->model()->moveItems(rest, outer, at);
        }
        m_project->model()->deletePermanently(folder);
        m_binder->select({}, false);
    };

    BinderItem *folder = doGroup();
    if (!folder)
        return nullptr;
    pushBinderStep(tr("Group"), undoGroup, [doGroup, this] {
        if (BinderItem *f = doGroup()) {
            m_binder->select({f}, true);
        }
    });
    m_binder->select({folder}, true);
    m_binder->reveal(folder);
    // the title is ready for typing
    QTimer::singleShot(0, this, [this, id = folder->id] {
        if (BinderItem *f = m_project ? m_project->item(id) : nullptr)
            m_binder->editTitle(f);
    });
    return folder;
}

// ------------------------------------------------------------------ Documents > Convert > Sheet to Object

QString MainWindow::sheetObjectOf(const BinderItem *item) const
{
    if (!m_project || !item)
        return QString();
    const objects::Object *o = m_project->objects()->objectForSheet(item->id);
    return o ? o->id : QString();
}

namespace {

QString categoryByFolderName(const objects::Schema &schema, const QString &folderTitle)
{
    const QString t = folderTitle.trimmed().toLower();
    struct Guess {
        const char *id;
        QStringList names;
    };
    const QList<Guess> guesses = {
        {"characters", {QStringLiteral("characters"), QStringLiteral("character"), QObject::tr("Characters").toLower()}},
        {"locations", {QStringLiteral("places"), QStringLiteral("place"), QStringLiteral("locations"), QStringLiteral("location"),
                       QStringLiteral("settings"), QStringLiteral("setting"), QObject::tr("Places").toLower(),
                       QObject::tr("Locations").toLower()}}};
    for (const Guess &g : guesses)
        if (g.names.contains(t) && schema.category(QLatin1String(g.id)))
            return QLatin1String(g.id);
    for (const objects::Category &c : schema.categories)
        if (c.name.toLower() == t || c.id == t)
            return c.id;
    return QString();
}

// the first picture in the text of a document, as an image
QImage firstPicture(QTextDocument *doc)
{
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextImageFormat f = it.fragment().charFormat().toImageFormat();
            if (!f.isValid())
                continue;
            const QVariant r = doc->resource(QTextDocument::ImageResource, QUrl(f.name()));
            QImage img = r.value<QImage>();
            if (img.isNull())
                img = r.value<QPixmap>().toImage();
            if (!img.isNull())
                return img;
        }
    return QImage();
}

} // namespace

QString MainWindow::sheetToObject(BinderItem *item, const QString &categoryId)
{
    objects::Store *st = m_project->objects();
    const objects::Category *c = st->category(categoryId);
    if (!c)
        return QString();
    const QString id = st->create(categoryId, QString(), item->title);
    if (id.isEmpty())
        return QString();
    objects::Object o = *st->object(id);
    o.sheet = item->id;
    QTextDocument *doc = m_project->docs()->text(item->id);
    // its text into "Other" (else the first long text of the template)
    const objects::FieldDef *other = c->field(QStringLiteral("other"));
    if (!other || other->type != objects::FieldType::LongText) {
        other = nullptr;
        for (const objects::FieldDef &f : c->fields)
            if (f.type == objects::FieldType::LongText)
                other = &f;
    }
    if (doc && other) {
        QString md = doc->toMarkdown();
        static const QRegularExpression pictures(QStringLiteral(R"(!\[[^\]]*\]\([^)]*\))"));
        md.remove(pictures);
        md = md.trimmed();
        if (!md.isEmpty())
            o.values.insert(other->id, md);
    }
    // its picture
    const objects::FieldDef *pic = nullptr;
    for (const objects::FieldDef &f : c->fields)
        if (f.type == objects::FieldType::Picture && !pic)
            pic = &f;
    if (doc && pic) {
        const QImage img = firstPicture(doc);
        if (!img.isNull()) {
            // 0.99.0 (encryption review M2): the vault's private folder while an encrypted project is open
            const QString tempBase = crypto::tempFolderBase();
            QTemporaryDir tmp(tempBase.isEmpty() ? QString() : tempBase + QStringLiteral("/qb-sheet-XXXXXX"));
            const QString file = tmp.filePath(QStringLiteral("sheet.png"));
            if (tmp.isValid() && img.save(file)) {
                const QString name = st->importPicture(id, pic->id, file);
                if (!name.isEmpty())
                    o.values.insert(pic->id, name);
            }
        }
    }
    st->update(o);
    return id;
}

QStringList MainWindow::sheetsToObjects(const QList<BinderItem *> &in, const QString &category)
{
    QStringList made;
    if (!m_project)
        return made;
    objects::Store *st = m_project->objects();
    const objects::Schema &schema = st->schema();
    if (schema.categories.isEmpty())
        return made;
    QList<BinderItem *> items;
    for (BinderItem *it : in)
        if (it && it->type == ItemType::Text && !it->isRootFolder() && !m_project->isInTrash(it) && sheetObjectOf(it).isEmpty())
            items << it;
    if (items.isEmpty())
        return made;
    std::sort(items.begin(), items.end(), binderBefore);
    // the category of each: the one asked for, else from the name of its folder (Characters, Places), else asked once
    QHash<QString, QString> cat;   // item id -> category
    QList<BinderItem *> unknown;
    for (BinderItem *it : items) {
        QString c = category;
        if (c.isEmpty() && it->parent)
            c = categoryByFolderName(schema, it->parent->title);
        if (c.isEmpty() || !schema.category(c))
            unknown << it;
        else
            cat.insert(it->id, c);
    }
    if (!unknown.isEmpty()) {
        QString chosen = schema.categories.first().id;
        if (!QB_TEST_ENV_SET("QB_NO_DIALOGS")) {
            QStringList names;
            for (const objects::Category &c : schema.categories)
                names << c.name;
            bool ok = false;
            const QString pick = QInputDialog::getItem(this, tr("Sheet to Object"),
                                                       tr("Which kind of object should %n sheet(s) become?", nullptr, int(unknown.size())),
                                                       names, 0, false, &ok);
            if (!ok)
                return made;
            for (const objects::Category &c : schema.categories)
                if (c.name == pick)
                    chosen = c.id;
        }
        for (BinderItem *it : unknown)
            cat.insert(it->id, chosen);
    }
    auto ids = std::make_shared<QStringList>();   // the objects made (the redo makes others)
    auto convert = [this, items, cat, ids]() {
        ids->clear();
        for (BinderItem *it : items) {
            BinderItem *live = m_project->item(it->id);
            if (live && sheetObjectOf(live).isEmpty()) {
                const QString oid = sheetToObject(live, cat.value(live->id));
                if (!oid.isEmpty())
                    ids->append(oid);
            }
        }
    };
    convert();
    made = *ids;
    if (made.isEmpty())
        return made;
    pushBinderStep(tr("Sheet to Object"),
                   [this, ids] {
                       for (const QString &oid : *ids)
                           m_project->objects()->remove(oid);
                       ids->clear();
                   },
                   [this, convert, ids] {
                       convert();
                       if (!ids->isEmpty())
                           showObject(ids->first());
                   });
    showObject(made.first());
    updateActions();
    return made;
}

} // namespace qb
