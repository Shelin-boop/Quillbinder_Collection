#include "ui/ToolDialogs.h"
#include "ui/TargetSpinBox.h"
#include "core/TargetMath.h"
#include "core/TestSwitches.h"
#include "ui/Ask.h"

#include "import/FileImporter.h"
#include "ui/DragDrop.h"
#include "AppInfo.h"
#include "compile/Compiler.h"
#include "compile/DocxWriter.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/ItemIcons.h"
#include "import/ScrivenerImporter.h"
#include "text/MarkdownIO.h"
#include "text/RtfIO.h"
#include "text/TextUtil.h"
#include "ui/AppSettings.h"
#include "ui/Language.h"
#include "ui/ManualWindow.h"

#include <QApplication>
#include <QTimer>
#include <QMimeData>
#include <QDropEvent>
#include <QDrag>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QSaveFile>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace qb {

// ------------------------------------------------------------------ targets

TargetsDialog::TargetsDialog(Project *p, QWidget *parent) : QDialog(parent), m_project(p)
{
    setWindowTitle(tr("Project Targets"));
    setAttribute(Qt::WA_DeleteOnClose);
    auto *lay = new QVBoxLayout(this);

    auto *draft = new QGroupBox(tr("Draft target"), this);
    auto *dl = new QVBoxLayout(draft);
    m_draftBar = new QProgressBar(draft);
    m_draftLabel = new QLabel(draft);
    {
        // the bar keeps its room while hidden: the fields below do not move when a target is set (0.94.0, item 89)
        QSizePolicy sp = m_draftBar->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        m_draftBar->setSizePolicy(sp);
    }
    dl->addWidget(m_draftBar);
    dl->addWidget(m_draftLabel);
    auto *dr = new QHBoxLayout;
    // 0.95.0: a negative number is a cutting target ("bring the draft down to 80,000 words"); shown in red
    auto *draftField = new TargetSpinBox(draft);
    draftField->setObjectName(QStringLiteral("draftTargetField"));
    draftField->setSingleStep(1000);
    draftField->setNoTargetText(tr("no target"));
    m_draftTarget = draftField;
    m_draftType = new QComboBox(draft);
    m_draftType->addItems({tr("words"), tr("characters")});
    dr->addWidget(m_draftTarget, 1);
    dr->addWidget(m_draftType);
    dl->addLayout(dr);
    auto *deadRow = new QHBoxLayout;
    m_useDeadline = new QCheckBox(tr("Deadline:"), draft);
    m_deadline = new QDateEdit(draft);
    m_deadline->setCalendarPopup(true);
    m_deadline->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    deadRow->addWidget(m_useDeadline);
    deadRow->addWidget(m_deadline, 1);
    dl->addLayout(deadRow);
    lay->addWidget(draft);

    auto *session = new QGroupBox(tr("Session target"), this);
    auto *sl = new QVBoxLayout(session);
    m_sessionBar = new QProgressBar(session);
    m_sessionLabel = new QLabel(session);
    {
        QSizePolicy sp = m_sessionBar->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        m_sessionBar->setSizePolicy(sp);
    }
    sl->addWidget(m_sessionBar);
    sl->addWidget(m_sessionLabel);
    auto *sr = new QHBoxLayout;
    auto *sessionField = new TargetSpinBox(session);
    sessionField->setObjectName(QStringLiteral("sessionTargetField"));
    sessionField->setSingleStep(100);
    sessionField->setNoTargetText(tr("no target"));
    m_sessionTarget = sessionField;
    m_sessionType = new QComboBox(session);
    m_sessionType->addItems({tr("words"), tr("characters")});
    sr->addWidget(m_sessionTarget, 1);
    sr->addWidget(m_sessionType);
    sl->addLayout(sr);
    auto *sf = new QFormLayout;
    m_reset = new QComboBox(session);
    m_reset->addItem(tr("at midnight"), QStringLiteral("midnight"));
    m_reset->addItem(tr("every time the project is opened"), QStringLiteral("open"));
    m_reset->addItem(tr("only when I reset it"), QStringLiteral("never"));
    sf->addRow(tr("Reset the session count:"), m_reset);
    sl->addLayout(sf);
    m_anywhere = new QCheckBox(tr("Count text written anywhere in the project (not only in the Draft)"), session);
    sl->addWidget(m_anywhere);
    m_auto = new QCheckBox(tr("Calculate the session target from the draft deadline"), session);
    sl->addWidget(m_auto);
    auto *days = new QHBoxLayout;
    days->addWidget(new QLabel(tr("Writing days:"), session));
    const QStringList names = {tr("Mon"), tr("Tue"), tr("Wed"), tr("Thu"), tr("Fri"), tr("Sat"), tr("Sun")};
    for (const QString &n : names) {
        auto *cb = new QCheckBox(n, session);
        m_days << cb;
        days->addWidget(cb);
    }
    sl->addLayout(days);
    auto *resetNow = new QPushButton(tr("Reset Session Now"), session);
    sl->addWidget(resetNow, 0, Qt::AlignLeft);
    lay->addWidget(session);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, this);
    lay->addWidget(bb);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::close);

    const Targets &t = p->targets;
    m_loading = true;
    m_draftTarget->setValue(t.draftTarget);
    m_draftType->setCurrentIndex(t.draftChars ? 1 : 0);
    m_useDeadline->setChecked(t.deadline.isValid());
    m_deadline->setDate(t.deadline.isValid() ? t.deadline : QDate::currentDate().addMonths(3));
    m_sessionTarget->setValue(t.sessionTarget);
    m_sessionType->setCurrentIndex(t.sessionChars ? 1 : 0);
    m_reset->setCurrentIndex(qMax(0, m_reset->findData(t.sessionReset)));
    m_anywhere->setChecked(t.countAnywhere);
    m_auto->setChecked(t.autoFromDeadline);
    for (int i = 0; i < 7; ++i)
        m_days[i]->setChecked(t.writingDays.size() > i && t.writingDays.at(i) == QLatin1Char('1'));
    m_loading = false;

    for (QSpinBox *s : {m_draftTarget, m_sessionTarget})
        connect(s, qOverload<int>(&QSpinBox::valueChanged), this, &TargetsDialog::store);
    for (QComboBox *c : {m_draftType, m_sessionType, m_reset})
        connect(c, qOverload<int>(&QComboBox::currentIndexChanged), this, &TargetsDialog::store);
    for (QCheckBox *c : {m_useDeadline, m_auto, m_anywhere})
        connect(c, &QCheckBox::toggled, this, &TargetsDialog::store);
    for (QCheckBox *c : m_days)
        connect(c, &QCheckBox::toggled, this, &TargetsDialog::store);
    connect(m_deadline, &QDateEdit::dateChanged, this, &TargetsDialog::store);
    connect(resetNow, &QPushButton::clicked, this, [this] {
        if (m_project) {
            m_project->startSession(true);
            refresh();
        }
    });
    connect(p, &Project::countsChanged, this, &TargetsDialog::refresh);
    refresh();
}

void TargetsDialog::store()
{
    if (m_loading || !m_project)
        return;
    Targets &t = m_project->targets;
    t.draftTarget = m_draftTarget->value();
    t.draftChars = m_draftType->currentIndex() == 1;
    t.deadline = m_useDeadline->isChecked() ? m_deadline->date() : QDate();
    t.sessionTarget = m_sessionTarget->value();
    t.sessionChars = m_sessionType->currentIndex() == 1;
    t.sessionReset = m_reset->currentData().toString();
    t.countAnywhere = m_anywhere->isChecked();
    t.autoFromDeadline = m_auto->isChecked();
    QString days;
    for (QCheckBox *c : m_days)
        days += c->isChecked() ? QLatin1Char('1') : QLatin1Char('0');
    t.writingDays = days;
    m_project->notifySettingsChanged();
    refresh();
}

void TargetsDialog::refresh()
{
    if (!m_project)
        return;
    const Targets &t = m_project->targets;
    QLocale loc;
    const int draft = t.draftChars ? m_project->draftCharCount() : m_project->draftWordCount();
    const QColor back = palette().color(QPalette::Window);
    m_draftLabel->setTextFormat(Qt::RichText);
    m_sessionLabel->setTextFormat(Qt::RichText);
    if (t.draftTarget < 0) { // a cutting target: the bar fills as the count falls to the number
        m_draftBar->setRange(0, 1000);
        m_draftBar->setValue(int(targetmath::fill(draft, t.draftTarget) * 1000));
    } else {
        m_draftBar->setMaximum(qMax(1, t.draftTarget));
        m_draftBar->setValue(qMin(draft, qMax(1, t.draftTarget)));
    }
    m_draftBar->setVisible(t.draftTarget != 0);
    m_draftLabel->setText(t.draftTarget != 0 ? tr("%1 of %2 %3").arg(loc.toString(draft), targettext::numberHtml(t.draftTarget, back),
                                                                      t.draftChars ? tr("characters") : tr("words"))
                                             : tr("%1 %2 in the Draft").arg(loc.toString(draft), t.draftChars ? tr("characters") : tr("words")));
    const int session = t.sessionChars ? m_project->sessionChars() : m_project->sessionWords();
    const int target = m_project->sessionTargetToday();
    if (target < 0) { // today's count is net: below 0 where more words were cut than written
        m_sessionBar->setRange(0, -target);
        m_sessionBar->setValue(qBound(0, -session, -target));
    } else {
        m_sessionBar->setMaximum(qMax(1, target));
        m_sessionBar->setValue(qMin(session, qMax(1, target)));
    }
    m_sessionBar->setVisible(target != 0);
    m_sessionLabel->setText(target != 0 ? tr("%1 of %2 this session").arg(targettext::numberHtml(session, back), targettext::numberHtml(target, back))
                                        : tr("%1 this session").arg(loc.toString(session)));
    m_sessionTarget->setEnabled(!t.autoFromDeadline);
}

// ------------------------------------------------------------------ keywords

namespace {
// The keywords as a tree that can be dragged from: onto the keyword list of
// the inspector or onto documents in the binder, corkboard and outliner
// (with Alt: together with the groups they belong to), and inside the tree to
// put a keyword into another group.
class KeywordTree : public QTreeWidget
{
public:
    explicit KeywordTree(QWidget *parent) : QTreeWidget(parent)
    {
        setObjectName(QStringLiteral("keywordTree"));
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setDragEnabled(true);
        setAcceptDrops(true);
        setDragDropMode(QAbstractItemView::DragDrop);
        setDefaultDropAction(Qt::CopyAction);
        setDropIndicatorShown(true);
    }
    Project *project = nullptr;
    std::function<void()> changed;

    QStringList selectedIds() const
    {
        QStringList ids;
        for (QTreeWidgetItem *ti : selectedItems())
            ids << ti->data(0, Qt::UserRole).toString();
        return ids;
    }

protected:
    QStringList mimeTypes() const override { return {QString::fromLatin1(kKeywordsMime)}; }
    QMimeData *mimeData(const QList<QTreeWidgetItem *> &items) const override
    {
        QStringList ids;
        for (QTreeWidgetItem *ti : items)
            ids << ti->data(0, Qt::UserRole).toString();
        return project && !ids.isEmpty() ? dnd::mimeForKeywords(project, ids) : nullptr;
    }
    Qt::DropActions supportedDropActions() const override { return Qt::CopyAction | Qt::MoveAction; }
    void startDrag(Qt::DropActions) override
    {
        // a drag of our own: the view's would take the rows out of the tree after a "move"
        const QStringList ids = selectedIds();
        if (!project || ids.isEmpty())
            return;
        auto *drag = new QDrag(this);
        drag->setMimeData(dnd::mimeForKeywords(project, ids));
        drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
    }
    void dropEvent(QDropEvent *e) override
    {
        e->setDropAction(Qt::CopyAction); // (QTreeWidget would shuffle its rows itself on a move)
        QTreeView::dropEvent(e);
    }
    bool dropMimeData(QTreeWidgetItem *parent, int, const QMimeData *data, Qt::DropAction) override
    {
        if (!project)
            return false;
        const QStringList ids = dnd::keywordsFromMime(data, project);
        const QString parentId = parent ? parent->data(0, Qt::UserRole).toString() : QString();
        bool any = false;
        for (Keyword &k : project->keywords) {
            if (!ids.contains(k.id) || k.parentId == parentId || k.id == parentId)
                continue;
            // not into its own group
            bool inside = false;
            const Keyword *up = project->keywordById(parentId);
            for (int guard = 0; up && guard < 20; ++guard) {
                if (up->id == k.id)
                    inside = true;
                up = project->keywordById(up->parentId);
            }
            if (inside)
                continue;
            k.parentId = parentId;
            any = true;
        }
        if (any) {
            project->notifySettingsChanged();
            if (changed)
                QTimer::singleShot(0, this, changed);
        }
        return false; // nothing for the view to move
    }
};
} // namespace

KeywordsDialog::KeywordsDialog(Project *p, QWidget *parent) : QDialog(parent), m_project(p)
{
    setWindowTitle(tr("Project Keywords"));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(420, 520);
    auto *lay = new QVBoxLayout(this);
    auto *tree = new KeywordTree(this);
    tree->project = p;
    tree->changed = [this] { refresh(); };
    m_tree = tree;
    m_tree->setHeaderHidden(true);
    m_tree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    lay->addWidget(m_tree, 1);
    auto *row = new QHBoxLayout;
    auto *add = new QPushButton(tr("Add"), this);
    auto *addChild = new QPushButton(tr("Add Sub-Keyword"), this);
    auto *remove = new QPushButton(tr("Delete"), this);
    auto *color = new QPushButton(tr("Color…"), this);
    row->addWidget(add);
    row->addWidget(addChild);
    row->addWidget(remove);
    row->addWidget(color);
    lay->addLayout(row);
    auto *row2 = new QHBoxLayout;
    auto *assign = new QPushButton(tr("Assign to Selected Documents"), this);
    auto *search = new QPushButton(tr("Find Documents"), this);
    row2->addWidget(assign);
    row2->addWidget(search);
    lay->addLayout(row2);
    auto *hint = new QLabel(tr("Drag keywords onto documents in the binder, the corkboard or the outliner, or into the "
                               "keyword list of the inspector. With Alt held down, the group of a keyword is assigned as well. "
                               "Inside this list, drag a keyword onto another to group them."),
                            this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    lay->addWidget(hint);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, this);
    lay->addWidget(bb);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::close);

    auto addKeyword = [this](const QString &parent) {
        Keyword k;
        k.id = newId();
        k.name = tr("New Keyword");
        k.color = QColor::fromHsv(int(m_project->keywords.size() * 47) % 360, 120, 220);
        k.parentId = parent;
        m_project->keywords << k;
        m_project->notifySettingsChanged();
        refresh();
        for (QTreeWidgetItem *ti : m_tree->findItems(k.name, Qt::MatchRecursive))
            if (ti->data(0, Qt::UserRole).toString() == k.id) {
                m_tree->setCurrentItem(ti);
                m_tree->editItem(ti);
            }
    };
    connect(add, &QPushButton::clicked, this, [=] { addKeyword(QString()); });
    connect(addChild, &QPushButton::clicked, this, [=] {
        QTreeWidgetItem *ti = m_tree->currentItem();
        addKeyword(ti ? ti->data(0, Qt::UserRole).toString() : QString());
    });
    connect(remove, &QPushButton::clicked, this, [this] {
        QTreeWidgetItem *ti = m_tree->currentItem();
        if (!ti)
            return;
        const QString id = ti->data(0, Qt::UserRole).toString();
        if (ask::question(this, tr("Delete Keyword"),
                                  tr("Delete \"%1\" and remove it from all documents?").arg(ti->text(0)), QMessageBox::Yes | QMessageBox::No, QMessageBox::No, tr("Delete Keyword"))
            != QMessageBox::Yes)
            return;
        QStringList ids{id};
        for (const Keyword &k : m_project->keywords)
            if (ids.contains(k.parentId))
                ids << k.id;
        for (int i = m_project->keywords.size() - 1; i >= 0; --i)
            if (ids.contains(m_project->keywords[i].id))
                m_project->keywords.removeAt(i);
        for (BinderItem *it : m_project->allItems()) {
            const int before = it->keywords.size();
            for (const QString &k : ids)
                it->keywords.removeAll(k);
            if (it->keywords.size() != before)
                m_project->notifyItemChanged(it, false);
        }
        m_project->notifySettingsChanged();
        refresh();
    });
    connect(color, &QPushButton::clicked, this, [this] {
        QTreeWidgetItem *ti = m_tree->currentItem();
        if (!ti)
            return;
        for (Keyword &k : m_project->keywords)
            if (k.id == ti->data(0, Qt::UserRole).toString()) {
                const QColor c = QColorDialog::getColor(k.color, this);
                if (c.isValid()) {
                    k.color = c;
                    m_project->notifySettingsChanged();
                    refresh();
                }
            }
    });
    connect(m_tree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *ti, int) {
        for (Keyword &k : m_project->keywords)
            if (k.id == ti->data(0, Qt::UserRole).toString() && k.name != ti->text(0)) {
                k.name = ti->text(0);
                m_project->notifySettingsChanged();
            }
    });
    connect(assign, &QPushButton::clicked, this, [this] {
        if (!m_selection)
            return;
        QStringList ids;
        for (QTreeWidgetItem *ti : m_tree->selectedItems())
            ids << ti->data(0, Qt::UserRole).toString();
        dnd::assignKeywords(m_project, m_selection(), ids, QApplication::keyboardModifiers() & Qt::AltModifier);
    });
    connect(search, &QPushButton::clicked, this, [this] {
        if (QTreeWidgetItem *ti = m_tree->currentItem())
            emit searchRequested(ti->text(0));
    });
    refresh();
}

void KeywordsDialog::refresh()
{
    QSignalBlocker b(m_tree);
    m_tree->clear();
    QHash<QString, QTreeWidgetItem *> items;
    QList<Keyword> pending = m_project->keywords;
    int guard = 0;
    while (!pending.isEmpty() && guard++ < 50) {
        for (int i = pending.size() - 1; i >= 0; --i) {
            const Keyword &k = pending[i];
            QTreeWidgetItem *parent = k.parentId.isEmpty() ? nullptr : items.value(k.parentId);
            if (!k.parentId.isEmpty() && !parent && m_project->keywordById(k.parentId))
                continue;
            auto *ti = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(m_tree);
            ti->setText(0, k.name);
            ti->setFlags(ti->flags() | Qt::ItemIsEditable);
            QPixmap pm(12, 12);
            pm.fill(k.color.isValid() ? k.color : Qt::gray);
            ti->setIcon(0, QIcon(pm));
            ti->setData(0, Qt::UserRole, k.id);
            items.insert(k.id, ti);
            pending.removeAt(i);
        }
    }
    m_tree->sortItems(0, Qt::AscendingOrder);
    m_tree->expandAll();
}

// ------------------------------------------------------------------ history / statistics

HistoryDialog::HistoryDialog(Project *p, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Writing History"));
    resize(520, 480);
    auto *lay = new QVBoxLayout(this);
    auto *tree = new QTreeWidget(this);
    tree->setHeaderLabels({tr("Date"), tr("Draft words"), tr("Other words"), tr("Total")});
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QMap<QString, QTreeWidgetItem *> months;
    QLocale loc;
    QList<WritingDay> days = p->history;
    std::sort(days.begin(), days.end(), [](const WritingDay &a, const WritingDay &b) { return a.date > b.date; });
    int total = 0;
    for (const WritingDay &d : days) {
        const QString mkey = d.date.toString(QStringLiteral("yyyy-MM"));
        QTreeWidgetItem *m = months.value(mkey);
        if (!m) {
            m = new QTreeWidgetItem(tree, {mkey, QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("0")});
            QFont f = m->font(0);
            f.setBold(true);
            for (int c = 0; c < 4; ++c)
                m->setFont(c, f);
            months.insert(mkey, m);
        }
        new QTreeWidgetItem(m, {d.date.toString(QStringLiteral("yyyy-MM-dd")), loc.toString(d.draftWords),
                                loc.toString(d.otherWords), loc.toString(d.draftWords + d.otherWords)});
        m->setText(1, loc.toString(loc.toInt(m->text(1)) + d.draftWords));
        m->setText(2, loc.toString(loc.toInt(m->text(2)) + d.otherWords));
        m->setText(3, loc.toString(loc.toInt(m->text(3)) + d.draftWords + d.otherWords));
        total += d.draftWords + d.otherWords;
    }
    if (!months.isEmpty())
        tree->topLevelItem(0)->setExpanded(true);
    lay->addWidget(tree, 1);
    lay->addWidget(new QLabel(tr("Net words written since tracking began: %1").arg(loc.toString(total)), this));
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, this);
    lay->addWidget(bb);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

StatisticsDialog::StatisticsDialog(Project *p, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Project Statistics"));
    auto *lay = new QFormLayout(this);
    QLocale loc;
    int compiledWords = 0, docs = 0;
    p->draft()->forEach([&](BinderItem *it) {
        if (it->includeInCompile && it->hasText()) {
            compiledWords += it->wordCount;
            ++docs;
        }
    });
    lay->addRow(tr("Draft words:"), new QLabel(loc.toString(p->draftWordCount())));
    lay->addRow(tr("Draft characters:"), new QLabel(loc.toString(p->draftCharCount())));
    lay->addRow(tr("Words included in compile:"), new QLabel(loc.toString(compiledWords)));
    lay->addRow(tr("Documents in the Draft:"), new QLabel(loc.toString(docs)));
    lay->addRow(tr("Estimated pages (250 words):"), new QLabel(loc.toString(compiledWords / 250.0, 'f', 0)));
    lay->addRow(tr("Words in the whole project:"), new QLabel(loc.toString(p->totalWordCount())));
    lay->addRow(tr("Words this session:"), new QLabel(loc.toString(p->sessionWords())));
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, this);
    lay->addRow(bb);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// ------------------------------------------------------------------ import and split

ImportSplitDialog::ImportSplitDialog(Project *p, QWidget *parent) : QDialog(parent), m_project(p)
{
    setWindowTitle(tr("Import and Split"));
    setObjectName(QStringLiteral("importSplitDialog"));
    auto *outer = new QVBoxLayout(this);
    auto *f = new QFormLayout;
    outer->addLayout(f);
    auto *row = new QHBoxLayout;
    m_file = new QLineEdit(this);
    m_file->setObjectName(QStringLiteral("splitFile"));
    auto *browse = new QPushButton(tr("Browse…"), this);
    row->addWidget(m_file, 1);
    row->addWidget(browse);
    f->addRow(tr("File:"), row);
    m_mode = new QComboBox(this);
    m_mode->addItem(tr("Two empty lines"));
    m_mode->addItem(tr("A line containing only this text:"));
    m_mode->addItem(tr("Scene headings (scripts: .fountain, .fdx)"));
    m_mode->addItem(tr("Headings (.docx, .odt, .md): Heading 1 makes a folder"));
    m_mode->setObjectName(QStringLiteral("splitMode"));
    f->addRow(tr("Sections are separated by:"), m_mode);
    m_sep = new QLineEdit(QStringLiteral("#"), this);
    m_sep->setEnabled(false);
    f->addRow(tr("Separator text:"), m_sep);
    m_h2 = new QComboBox(this);
    m_h2->setObjectName(QStringLiteral("splitHeading2"));
    m_h2->addItem(tr("a folder (its text becomes a document inside)"));
    m_h2->addItem(tr("a document (with the text under it)"));
    f->addRow(tr("Heading 2 makes:"), m_h2);
    m_title = new QCheckBox(tr("Use the first line of each section as its title"), this);
    m_title->setChecked(true);
    m_title->setObjectName(QStringLiteral("splitFirstLine"));
    f->addRow(QString(), m_title);
    m_rule = new QLabel(this);
    m_rule->setObjectName(QStringLiteral("splitTitleRule"));
    m_rule->setWordWrap(true);
    outer->addWidget(m_rule);
    m_previewNote = new QLabel(this);
    m_previewNote->setObjectName(QStringLiteral("splitPreviewNote"));
    outer->addWidget(m_previewNote);
    m_preview = new QTreeWidget(this);
    m_preview->setObjectName(QStringLiteral("splitPreview"));
    m_preview->setHeaderHidden(true);
    m_preview->setMinimumHeight(180);
    outer->addWidget(m_preview, 1);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    outer->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_mode, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { updateState(); });
    connect(m_h2, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { updatePreview(); });
    connect(m_title, &QCheckBox::toggled, this, [this](bool) {
        updateState();
        updatePreview();
    });
    connect(m_file, &QLineEdit::textChanged, this, [this](const QString &) { updatePreview(); });
    connect(browse, &QPushButton::clicked, this, [this] {
        const QString f = QFileDialog::getOpenFileName(this, tr("Import and Split"), QDir::homePath(),
                                                       importer::textFileFilters().join(QStringLiteral(";;")));
        if (f.isEmpty())
            return;
        m_file->setText(f);
        if (importer::isScriptFile(f))
            m_mode->setCurrentIndex(2);
        else if (importer::canSplitAtHeadings(f) && m_mode->currentIndex() == 0)
            m_mode->setCurrentIndex(3);
    });
    resize(560, 560);
    updateState();
}

void ImportSplitDialog::updateState()
{
    const int mode = m_mode->currentIndex();
    m_sep->setEnabled(mode == 1);
    m_h2->setEnabled(mode == 3);
    m_title->setEnabled(mode != 2);
    // the rule for the title of a document, the same for every document that no heading names
    QString rule = mode == 2 ? tr("Title of a document: the scene heading.")
                 : m_title->isChecked()
                     ? tr("Title of a document: the first line of its text that is not empty (a line longer than 120 characters is "
                          "shortened and stays in the text); a section without a line is called “<file name> <number>”.")
                     : tr("Title of a document: “<file name> <number>”.");
    if (mode == 3)
        rule += QLatin1Char(' ') + tr("Headings name the folders and, for Heading 2 as a document, the document. Lower headings stay in the text.");
    m_rule->setText(rule);
    m_preview->setVisible(mode == 3);
    m_previewNote->setVisible(mode == 3);
    updatePreview();
}

static void fillPreview(QTreeWidget *tree, QTreeWidgetItem *parent, const QList<importer::SplitNode> &nodes)
{
    for (const importer::SplitNode &n : nodes) {
        auto *it = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(tree);
        it->setText(0, n.title);
        it->setIcon(0, QIcon::fromTheme(n.folder ? QStringLiteral("folder") : QStringLiteral("text-x-generic")));
        it->setData(0, Qt::UserRole, n.folder);
        fillPreview(tree, it, n.children);
    }
}

void ImportSplitDialog::updatePreview()
{
    m_preview->clear();
    if (m_mode->currentIndex() != 3)
        return;
    const QString path = m_file->text().trimmed();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        m_previewNote->setText(tr("Choose a file to see the binder it makes."));
        return;
    }
    if (!importer::canSplitAtHeadings(path)) {
        m_previewNote->setText(tr("Splitting at headings works for .docx, .odt and .md files."));
        return;
    }
    importer::HeadingSplitOptions o;
    o.heading2Folder = heading2Folder();
    o.firstLineIsTitle = firstLineIsTitle();
    QList<importer::SplitNode> plan;
    QString err;
    if (!importer::planHeadingSplit(m_project, path, o, &plan, &err)) {
        m_previewNote->setText(err.isEmpty() ? tr("The file could not be read.") : err);
        return;
    }
    fillPreview(m_preview, nullptr, plan);
    m_preview->expandAll();
    const int n = importer::splitNodeCount(plan);
    m_previewNote->setText(n == 0 ? tr("Nothing to import: the file has no text.")
                                  : tr("The binder after the import (%n item(s)):", nullptr, n));
}

QString ImportSplitDialog::file() const { return m_file->text(); }
QString ImportSplitDialog::separator() const
{
    if (m_mode->currentIndex() == 2)
        return importer::splitAtScenes();
    return m_mode->currentIndex() == 0 ? QString() : m_sep->text();
}
bool ImportSplitDialog::firstLineIsTitle() const { return m_title->isChecked(); }
bool ImportSplitDialog::atHeadings() const { return m_mode->currentIndex() == 3; }
bool ImportSplitDialog::heading2Folder() const { return m_h2->currentIndex() == 0; }
void ImportSplitDialog::setFile(const QString &path) { m_file->setText(path); }
void ImportSplitDialog::setMode(int index) { m_mode->setCurrentIndex(index); }
void ImportSplitDialog::setHeading2Folder(bool folder) { m_h2->setCurrentIndex(folder ? 0 : 1); }

// ------------------------------------------------------------------ export files

ExportFilesDialog::ExportFilesDialog(Project *p, const QList<BinderItem *> &items, QWidget *parent)
    : QDialog(parent), m_project(p), m_items(items)
{
    setWindowTitle(tr("Export Files"));
    QSettings st;
    auto *l = new QVBoxLayout(this);
    auto *top = new QFormLayout;
    m_format = new QComboBox(this);
    m_format->setObjectName(QStringLiteral("exportFormat"));
    m_format->addItem(tr("Rich Text (.rtf)"), QStringLiteral("rtf"));
    m_format->addItem(tr("Word (.docx)"), QStringLiteral("docx"));
    m_format->addItem(tr("Markdown (.md)"), QStringLiteral("md"));
    m_format->addItem(tr("Plain text (.txt)"), QStringLiteral("txt"));
    m_format->addItem(tr("Fountain screenplay (.fountain)"), QStringLiteral("fountain"));
    m_format->addItem(tr("Final Draft (.fdx)"), QStringLiteral("fdx"));
    m_format->setCurrentIndex(qMax(0, m_format->findData(st.value(QStringLiteral("export/format"), QStringLiteral("rtf")).toString())));
    // scripts go out as scripts unless the writer chooses otherwise
    bool allScripts = !items.isEmpty();
    bool anyText = false;
    for (BinderItem *it : items) {
        if (!it->scriptMode)
            allScripts = false;
        anyText = anyText || it->hasText();
    }
    if (allScripts)
        m_format->setCurrentIndex(m_format->findData(QStringLiteral("fountain")));
    top->addRow(tr("Export text files as:"), m_format);
    auto *hint = new QLabel(anyText ? tr("Pictures, PDF files, web pages, audio and video are exported as they are.")
                                    : tr("Only notes (and other text files of the options below) use this format; "
                                         "the selected files are exported as they are."),
                            this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    top->addRow(QString(), hint);
    l->addLayout(top);

    auto box = [this, l](const QString &title) {
        auto *g = new QGroupBox(title, this);
        auto *v = new QVBoxLayout(g);
        l->addWidget(g);
        return v;
    };
    auto check = [this, &st](QVBoxLayout *v, const char *key, const QString &text, bool def) {
        auto *c = new QCheckBox(text, this);
        c->setObjectName(QStringLiteral("export_") + QLatin1String(key));
        c->setChecked(st.value(QStringLiteral("export/") + QLatin1String(key), def).toBool());
        v->addWidget(c);
        return c;
    };
    QVBoxLayout *meta = box(tr("Metadata"));
    m_metadata = check(meta, "metadata", tr("Export metadata: label, status, keywords, dates, custom metadata …"), false);
    m_metadata->setToolTip(tr("A text file beside each document, “name (metadata).txt”, with its title, kind, label, status, "
                              "keywords, include in compile, section type, words, target, dates, custom metadata and bookmarks."));
    m_synopsis = check(meta, "synopsis", tr("Export synopses (in the metadata file, or a text file of their own)"), false);
    QVBoxLayout *opts = box(tr("Options"));
    m_children = check(opts, "children", tr("Include subdocuments (folders become folders)"), true);
    m_notes = check(opts, "notes", tr("Export notes (in the text format chosen above)"), false);
    m_comments = check(opts, "comments", tr("Export comments and inline annotations"), true);
    m_snapshots = check(opts, "snapshots", tr("Export snapshots (a folder of them beside each document)"), false);
    for (QCheckBox *c : {m_metadata, m_synopsis, m_children, m_notes, m_comments, m_snapshots})
        c->setMinimumWidth(10);

    QVBoxLayout *where = box(tr("Location"));
    auto *row = new QHBoxLayout;
    QString loc = st.value(QStringLiteral("export/location")).toString();
    if (loc.isEmpty() || !QFileInfo(loc).isDir())
        loc = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_location = new QLineEdit(loc, this);
    m_location->setObjectName(QStringLiteral("exportLocation"));
    auto *browse = new QPushButton(tr("Browse…"), this);
    row->addWidget(new QLabel(tr("Save in:"), this));
    row->addWidget(m_location, 1);
    row->addWidget(browse);
    where->addLayout(row);
    connect(browse, &QPushButton::clicked, this, [this] {
        const QString d = QFileDialog::getExistingDirectory(this, tr("Save in"), m_location->text());
        if (!d.isEmpty())
            m_location->setText(d);
    });
    m_folderName = new QLineEdit(this);
    m_folderName->setObjectName(QStringLiteral("exportFolderName"));
    if (items.size() > 1) {
        auto *fr = new QHBoxLayout;
        fr->addWidget(new QLabel(tr("New folder for the files:"), this));
        m_folderName->setText(text::safeFileName(p->title()));
        fr->addWidget(m_folderName, 1);
        where->addLayout(fr);
    } else {
        m_folderName->hide();
    }
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    l->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, this, [this] {
        if (!QFileInfo(m_location->text()).isDir()) {
            QMessageBox::warning(this, tr("Export Files"), tr("The folder “%1” does not exist.").arg(m_location->text()));
            return;
        }
        if (m_items.size() > 1 && text::safeFileName(m_folderName->text().trimmed()).isEmpty()) {
            QMessageBox::warning(this, tr("Export Files"), tr("Give the new folder a name."));
            return;
        }
        accept();
    });
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

dnd::ExportOptions ExportFilesDialog::options() const
{
    dnd::ExportOptions o;
    o.format = m_format->currentData().toString();
    o.metadata = m_metadata->isChecked();
    o.synopsis = m_synopsis->isChecked();
    o.children = m_children->isChecked();
    o.notes = m_notes->isChecked();
    o.comments = m_comments->isChecked();
    o.snapshots = m_snapshots->isChecked();
    return o;
}

QString ExportFilesDialog::run(QStringList *made)
{
    const dnd::ExportOptions o = options();
    QSettings st;
    st.setValue(QStringLiteral("export/format"), o.format);
    st.setValue(QStringLiteral("export/metadata"), o.metadata);
    st.setValue(QStringLiteral("export/synopsis"), o.synopsis);
    st.setValue(QStringLiteral("export/children"), o.children);
    st.setValue(QStringLiteral("export/notes"), o.notes);
    st.setValue(QStringLiteral("export/comments"), o.comments);
    st.setValue(QStringLiteral("export/snapshots"), o.snapshots);
    st.setValue(QStringLiteral("export/location"), m_location->text());
    QString dir = m_location->text();
    if (m_items.size() > 1) {
        // a new folder (another name if it exists already)
        const QString wanted = text::safeFileName(m_folderName->text().trimmed());
        QString name = wanted;
        for (int n = 2; QFileInfo::exists(QDir(dir).filePath(name)); ++n)
            name = QStringLiteral("%1 %2").arg(wanted).arg(n);
        dir = QDir(dir).filePath(name);
        QDir().mkpath(dir);
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (BinderItem *it : BinderModel::topLevelOnly(m_items)) {
        const QString path = dnd::exportItem(m_project, it, dir, o);
        if (made && !path.isEmpty())
            made->append(path);
    }
    QApplication::restoreOverrideCursor();
    return dir;
}

bool exportItems(Project *p, const QList<BinderItem *> &items, QWidget *parent)
{
    if (items.isEmpty())
        return false;
    ExportFilesDialog dlg(p, items, parent);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    const QString dir = dlg.run();
    if (!QB_TEST_ENV_SET("QB_NO_DIALOGS"))
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    return true;
}

// ------------------------------------------------------------------ scrivener import

std::unique_ptr<Project> runScrivenerImport(QWidget *parent, const QString &given)
{
    const QString scrivx = !given.isEmpty() ? ScrivenerImporter::findScrivx(given)
                                            : QFileDialog::getOpenFileName(
                                                  parent, QObject::tr("Import Scrivener Project — choose the .scrivx file inside the .scriv folder"),
                                                  QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                  QObject::tr("Scrivener projects (*.scrivx)"));
    if (scrivx.isEmpty())
        return nullptr;

    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("Import Scrivener Project"));
    auto *f = new QFormLayout(&dlg);
    QString baseName = QFileInfo(scrivx).completeBaseName();
    auto *name = new QLineEdit(baseName, &dlg);
    f->addRow(QObject::tr("New project name:"), name);
    auto *row = new QHBoxLayout;
    QString loc = AppSettings::instance().lastProjectDir;
    if (loc.isEmpty())
        loc = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    auto *where = new QLineEdit(loc, &dlg);
    auto *browse = new QPushButton(QObject::tr("Browse…"), &dlg);
    row->addWidget(where, 1);
    row->addWidget(browse);
    f->addRow(QObject::tr("Save in:"), row);
    auto *md = new QRadioButton(QObject::tr("Markdown (recommended: plain text, future-proof)"), &dlg);
    auto *rtfBtn = new QRadioButton(QObject::tr("RTF (keeps fonts, sizes and colors exactly)"), &dlg);
    md->setChecked(true);
    f->addRow(QObject::tr("Store documents as:"), md);
    f->addRow(QString(), rtfBtn);
    auto *note = new QLabel(QObject::tr("The Scrivener project is only read, never changed."), &dlg);
    note->setEnabled(false);
    f->addRow(QString(), note);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    bb->button(QDialogButtonBox::Ok)->setText(QObject::tr("Import"));
    f->addRow(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(browse, &QPushButton::clicked, &dlg, [&] {
        const QString d = QFileDialog::getExistingDirectory(&dlg, QObject::tr("Save in"), where->text());
        if (!d.isEmpty())
            where->setText(d);
    });
    if (dlg.exec() != QDialog::Accepted)
        return nullptr;

    DocumentStore::Hold unloadHold; // 0.93.0: no document goes while this job runs events
    QProgressDialog progress(QObject::tr("Importing…"), QString(), 0, 100, parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(200);
    ScrivenerImportReport report;
    QString err;
    auto p = ScrivenerImporter::importProject(
        scrivx, where->text(), name->text().trimmed().isEmpty() ? baseName : name->text().trimmed(),
        rtfBtn->isChecked() ? StorageFormat::Rtf : StorageFormat::Markdown, &report, &err,
        [&](int done, int total, const QString &what) {
            progress.setMaximum(qMax(1, total));
            progress.setValue(done);
            progress.setLabelText(QObject::tr("Importing “%1”…").arg(what));
            QApplication::processEvents();
        });
    progress.close();
    if (!p) {
        QMessageBox::warning(parent, QObject::tr("Import Scrivener Project"), QObject::tr("The import failed:\n%1").arg(err));
        return nullptr;
    }
    QString msg = QObject::tr("Imported %1 text documents, %2 research files, %3 snapshots and %4 comments/footnotes.")
                      .arg(report.textDocuments)
                      .arg(report.mediaFiles)
                      .arg(report.snapshots)
                      .arg(report.comments);
    if (!report.warnings.isEmpty())
        msg += QStringLiteral("\n\n") + QObject::tr("Notes:") + QStringLiteral("\n• ")
               + report.warnings.mid(0, 12).join(QStringLiteral("\n• "));
    QMessageBox::information(parent, QObject::tr("Import Scrivener Project"), msg);
    return p;
}

// ------------------------------------------------------------------ manual

// A Markdown text of the resources in a window of its own (one window per text).
static QDialog *helpWindow(QPointer<QDialog> &win, QWidget *parent, const QString &resource, const QString &title,
                           const QString &objectName, QSize size)
{
    if (!win) {
        win = new QDialog(parent);
        win->setAttribute(Qt::WA_DeleteOnClose);
        win->setWindowTitle(title);
        win->setObjectName(objectName + QLatin1String("Window"));
        win->resize(size);
        auto *l = new QVBoxLayout(win);
        auto *tb = new QTextBrowser(win);
        tb->setObjectName(objectName);
        tb->setOpenExternalLinks(true);
        QFile f(resource);
        f.open(QIODevice::ReadOnly);
        QString text = QString::fromUtf8(f.readAll());
        text.replace(QLatin1String("{{APP}}"), QLatin1String(QB_APP_NAME))
            .replace(QLatin1String("{{VERSION}}"), QLatin1String(QB_APP_VERSION))
            .replace(QLatin1String("{{SUFFIX}}"), QLatin1String(QB_PROJECT_SUFFIX))
            .replace(QLatin1String("{{FILE}}"), QLatin1String(QB_PROJECT_FILE_SUFFIX));
        text = applyLabel(text); // 0.93.0: "beta"; the documents never go stale (0.99.0: also without a label)
        tb->setMarkdown(escapePlaceholders(text));
        l->addWidget(tb);
    }
    win->show();
    win->raise();
    return win;
}

void showManual(QWidget *parent, const QString &anchor)
{
    showManualAt(parent, anchor); // (0.94.0: the window with its contents list and search, ui/ManualWindow.h)
}

void showQuickStartGuide(QWidget *parent)
{
    static QPointer<QDialog> win;
    helpWindow(win, parent, language::docResource(QStringLiteral("quick-start.md")), QObject::tr("Quick Start Guide"),
               QStringLiteral("quickStart"), QSize(760, 720));
}

} // namespace qb
