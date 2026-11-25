#include "ui/ManualWindow.h"

#include "AppInfo.h"
#include "ui/Inspector.h"

#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QHash>
#include <QSignalBlocker>
#include <QTimer>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QShortcut>
#include <QSplitter>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextDocument>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace qb {

namespace {

QPointer<ManualWindow> s_window;

// "4. Writing" -> "Writing"; "Appendix: x" stays
QString withoutNumber(QString t)
{
    t = t.trimmed();
    int i = 0;
    while (i < t.size() && (t.at(i).isDigit() || t.at(i) == QLatin1Char('.')))
        ++i;
    if (i > 0 && i < t.size() && t.at(i) == QLatin1Char(' '))
        return t.mid(i + 1).trimmed();
    return t;
}

// Qt's Markdown reader takes "<computer>" for an HTML tag and swallows the text up to a closing tag that never comes
// (the manual was cut off after its first section). Words in angle brackets that stand for something the reader fills
// in are written as text, outside code spans and code blocks.
QString escapePlaceholders(const QString &md)
{
    static const QRegularExpression re(QStringLiteral("<((?:computer|ID|id|UUID|your name|version|title|file|date|name|number)[^<>\\n]{0,12})>"));
    QStringList out;
    bool fence = false;
    for (const QString &line : md.split(QLatin1Char('\n'))) {
        if (line.trimmed().startsWith(QLatin1String("```"))) {
            fence = !fence;
            out << line;
            continue;
        }
        if (fence || line.startsWith(QLatin1String("    "))) {
            out << line;
            continue;
        }
        const QStringList parts = line.split(QLatin1Char('`'));
        QStringList fixed;
        for (int i = 0; i < parts.size(); ++i) {
            QString part = parts[i];
            if (i % 2 == 0)
                part.replace(re, QStringLiteral("&lt;\\1&gt;"));
            fixed << part;
        }
        out << fixed.join(QLatin1Char('`'));
    }
    return out.join(QLatin1Char('\n'));
}

} // namespace

ManualWindow::ManualWindow(const QString &markdown, const QString &title, const QString &objectName, QSize size, QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(title);
    setObjectName(objectName + QLatin1String("Window"));
    resize(size);
    auto *lay = new QVBoxLayout(this);

    // the search: field, previous, next, the number of hits
    auto *bar = new QHBoxLayout;
    m_find = new QLineEdit(this);
    m_find->setObjectName(QStringLiteral("manualSearch"));
    m_find->setPlaceholderText(tr("Search the manual (Ctrl+F)"));
    m_find->setClearButtonEnabled(true);
    m_find->setAccessibleName(tr("Search the manual"));
    m_find->installEventFilter(this);
    bar->addWidget(m_find, 1);
    m_prev = new QToolButton(this);
    m_prev->setObjectName(QStringLiteral("manualPrevious"));
    m_prev->setText(tr("Previous"));
    m_prev->setToolTip(tr("The previous hit (Shift+Return)"));
    m_next = new QToolButton(this);
    m_next->setObjectName(QStringLiteral("manualNext"));
    m_next->setText(tr("Next"));
    m_next->setToolTip(tr("The next hit (Return)"));
    m_count = new QLabel(this);
    m_count->setObjectName(QStringLiteral("manualHits"));
    m_count->setMinimumWidth(90);
    bar->addWidget(m_prev);
    bar->addWidget(m_next);
    bar->addWidget(m_count);
    lay->addLayout(bar);

    auto *split = new QSplitter(Qt::Horizontal, this);
    m_contents = new QTreeWidget(split);
    m_contents->setObjectName(QStringLiteral("manualContents"));
    m_contents->setHeaderLabel(tr("Contents"));
    m_contents->setAccessibleName(tr("Contents of the manual"));
    m_contents->setUniformRowHeights(true);
    m_contents->setMinimumWidth(160);
    m_text = new QTextBrowser(split);
    m_text->setObjectName(objectName);
    m_text->setOpenExternalLinks(true);
    m_text->setMarkdown(escapePlaceholders(markdown));
    split->addWidget(m_contents);
    split->addWidget(m_text);
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);
    split->setSizes({230, size.width() - 260});
    lay->addWidget(split, 1);

    buildContents();
    connect(m_contents, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *it) {
        jumpTo(it->data(0, Qt::UserRole).toInt());
        m_text->setFocus();
    });
    connect(m_contents, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem *it) { jumpTo(it->data(0, Qt::UserRole).toInt()); });

    connect(m_find, &QLineEdit::textChanged, this, [this](const QString &t) { search(t); });
    connect(m_next, &QToolButton::clicked, this, [this] { nextHit(true); });
    connect(m_prev, &QToolButton::clicked, this, [this] { nextHit(false); });
    // Ctrl+F (whatever the platform's Find key is) goes to the search field from anywhere in the window
    auto *sc = new QShortcut(QKeySequence::Find, this);
    connect(sc, &QShortcut::activated, this, [this] {
        m_find->setFocus();
        m_find->selectAll();
    });
    // F3 and Shift+F3 as in other programs
    connect(new QShortcut(QKeySequence::FindNext, this), &QShortcut::activated, this, [this] { nextHit(true); });
    connect(new QShortcut(QKeySequence::FindPrevious, this), &QShortcut::activated, this, [this] { nextHit(false); });
}

void ManualWindow::buildContents()
{
    m_headings.clear();
    m_contents->clear();
    QTreeWidgetItem *top = nullptr, *section = nullptr;
    for (QTextBlock b = m_text->document()->begin(); b.isValid(); b = b.next()) {
        const int level = b.blockFormat().headingLevel();
        if (level <= 0 || level > 3 || b.text().trimmed().isEmpty())
            continue;
        m_headings.append({b.text().trimmed(), level, b.position()});
        auto *item = new QTreeWidgetItem;
        item->setText(0, b.text().trimmed());
        item->setData(0, Qt::UserRole, b.position());
        item->setToolTip(0, b.text().trimmed());
        if (level == 1) {
            m_contents->addTopLevelItem(item);
            top = item;
            section = nullptr;
        } else if (level == 2) {
            if (top)
                top->addChild(item);
            else
                m_contents->addTopLevelItem(item);
            section = item;
        } else {
            if (section)
                section->addChild(item);
            else if (top)
                top->addChild(item);
            else
                m_contents->addTopLevelItem(item);
        }
    }
    if (top)
        top->setExpanded(true);
    // the sections are open, their parts (the third level) closed until needed
    m_contents->resizeColumnToContents(0);
}

QStringList ManualWindow::headings() const
{
    QStringList out;
    for (const Heading &h : m_headings)
        out << h.text;
    return out;
}

void ManualWindow::jumpTo(int position)
{
    QTextCursor c(m_text->document());
    c.setPosition(position);
    const QTextBlock b = c.block();
    auto apply = [this, b] {
        const QRectF r = m_text->document()->documentLayout()->blockBoundingRect(b);
        m_text->verticalScrollBar()->setValue(qMax(0, int(r.top()) - 4));
    };
    apply();
    QTimer::singleShot(0, this, apply); // (the layout of a text that was just set may still grow)
}

QString ManualWindow::topHeading() const
{
    const int y = m_text->verticalScrollBar()->value();
    QString found;
    for (const Heading &h : m_headings) {
        QTextCursor c(m_text->document());
        c.setPosition(h.position);
        if (int(m_text->document()->documentLayout()->blockBoundingRect(c.block()).top()) <= y + 40)
            found = h.text;
        else
            break;
    }
    return found;
}

bool ManualWindow::showSection(const QString &section)
{
    const QString want = withoutNumber(section).toLower();
    if (want.isEmpty()) {
        m_text->verticalScrollBar()->setValue(0);
        return false;
    }
    const Heading *hit = nullptr;
    for (int pass = 0; pass < 3 && !hit; ++pass)
        for (const Heading &h : m_headings) {
            const QString t = withoutNumber(h.text).toLower();
            if ((pass == 0 && t == want) || (pass == 1 && t.startsWith(want)) || (pass == 2 && t.contains(want))) {
                hit = &h;
                break;
            }
        }
    if (!hit) {
        m_text->verticalScrollBar()->setValue(0);
        return false;
    }
    jumpTo(hit->position);
    // the contents list shows where we are
    QTreeWidgetItemIterator it(m_contents);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).toInt() == hit->position) {
            m_contents->setCurrentItem(*it);
            m_contents->scrollToItem(*it);
            break;
        }
        ++it;
    }
    return true;
}

int ManualWindow::search(const QString &text)
{
    if (m_find->text() != text) {
        const QSignalBlocker block(m_find);
        m_find->setText(text);
    }
    m_hits.clear();
    m_current = -1;
    const QString what = text.trimmed();
    if (!what.isEmpty()) {
        QTextCursor c(m_text->document());
        for (;;) {
            c = m_text->document()->find(what, c);   // (case-insensitive: no flags)
            if (c.isNull())
                break;
            m_hits.append(c);
        }
    }
    if (!m_hits.isEmpty()) {
        // the first hit from where the text is shown now
        const int y = m_text->verticalScrollBar()->value();
        int first = 0;
        for (int i = 0; i < m_hits.size(); ++i) {
            const QRectF r = m_text->document()->documentLayout()->blockBoundingRect(m_hits[i].block());
            if (int(r.top()) >= y) {
                first = i;
                break;
            }
        }
        showHit(first);
    } else {
        markHits();
    }
    return int(m_hits.size());
}

void ManualWindow::nextHit(bool forward)
{
    if (m_hits.isEmpty()) {
        if (!m_find->text().trimmed().isEmpty())
            search(m_find->text());
        return;
    }
    const int n = int(m_hits.size());
    showHit(m_current < 0 ? 0 : (m_current + (forward ? 1 : n - 1)) % n);
}

void ManualWindow::showHit(int index)
{
    m_current = index;
    markHits();
    const QTextCursor &c = m_hits[index];
    m_text->setTextCursor(c);
    m_text->ensureCursorVisible();
}

void ManualWindow::markHits()
{
    QList<QTextEdit::ExtraSelection> sel;
    for (int i = 0; i < m_hits.size(); ++i) {
        QTextEdit::ExtraSelection s;
        s.cursor = m_hits[i];
        const bool cur = i == m_current;
        s.format.setBackground(cur ? QColor(255, 150, 40) : QColor(255, 236, 120));
        s.format.setForeground(QColor(20, 20, 20));
        sel.append(s);
    }
    m_text->setExtraSelections(sel);
    if (m_find->text().trimmed().isEmpty())
        m_count->clear();
    else if (m_hits.isEmpty())
        m_count->setText(tr("No hits"));
    else
        m_count->setText(tr("%1 of %2").arg(m_current + 1).arg(m_hits.size()));
    m_prev->setEnabled(!m_hits.isEmpty());
    m_next->setEnabled(!m_hits.isEmpty());
}

bool ManualWindow::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_find && e->type() == QEvent::KeyPress) {
        auto *k = static_cast<QKeyEvent *>(e);
        if (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter) {
            nextHit(!(k->modifiers() & Qt::ShiftModifier));
            return true; // (a dialog would take Return as its default button)
        }
    }
    return QDialog::eventFilter(o, e);
}

// ------------------------------------------------------------------ opening

ManualWindow *manualWindow() { return s_window; }

void showManualAt(QWidget *parent, const QString &section)
{
    if (!s_window) {
        QFile f(QStringLiteral(":/docs/manual.md"));
        f.open(QIODevice::ReadOnly);
        QString text = QString::fromUtf8(f.readAll());
        text.replace(QLatin1String("{{APP}}"), QLatin1String(QB_APP_NAME))
            .replace(QLatin1String("{{VERSION}}"), QLatin1String(QB_APP_VERSION))
            .replace(QLatin1String("{{LABEL}}"), QLatin1String(QB_APP_VERSION_LABEL))
            .replace(QLatin1String("{{SUFFIX}}"), QLatin1String(QB_PROJECT_SUFFIX))
            .replace(QLatin1String("{{FILE}}"), QLatin1String(QB_PROJECT_FILE_SUFFIX));
        s_window = new ManualWindow(text, QObject::tr("User Manual"), QStringLiteral("manual"), QSize(980, 760), parent);
    } else if (parent && s_window->parentWidget() != parent) {
        // a question from a dialog: the window is a child of that dialog, so that the dialog's modality does not block it
        s_window->setParent(parent, s_window->windowFlags());
    }
    s_window->show(); // (shown first: a window that was hidden has no scroll range yet)
    s_window->raise();
    s_window->activateWindow();
    s_window->showSection(section);
}

// ------------------------------------------------------------------ F1 in other windows

namespace manualhelp {

QString sectionForInspectorTab(int tab, const QString &extraKey)
{
    if (!extraKey.isEmpty()) {
        if (extraKey.contains(QLatin1String("transcri"), Qt::CaseInsensitive) || extraKey.contains(QLatin1String("speech"), Qt::CaseInsensitive))
            return QStringLiteral("Speech Transcription");
        return QStringLiteral("The inspector");
    }
    switch (tab) {
    case Inspector::Bookmarks: return QStringLiteral("Bookmarks");
    case Inspector::Metadata: return QStringLiteral("Custom metadata");
    case Inspector::Snapshots: return QStringLiteral("Snapshots");
    case Inspector::Comments: return QStringLiteral("Comments and footnotes");
    case Inspector::MindMap: return QStringLiteral("Mind maps");
    case Inspector::Objects: return QStringLiteral("Objects: characters, locations, and more");
    default: return QStringLiteral("The inspector");
    }
}

QString sectionForWindow(const QWidget *window)
{
    if (!window)
        return {};
    static const QHash<QString, QString> map = {
        {QStringLiteral("qb::OptionsDialog"), QStringLiteral("Options")},
        {QStringLiteral("qb::ProjectSettingsDialog"), QStringLiteral("Settings for one project")},
        {QStringLiteral("qb::SettingsImportDialog"), QStringLiteral("Settings for one project")},
        {QStringLiteral("qb::CompileDialog"), QStringLiteral("Compiling: making the manuscript")},
        {QStringLiteral("qb::FormatEditorDialog"), QStringLiteral("Editing a compile format")},
        {QStringLiteral("qb::CompareDialog"), QStringLiteral("Compare and transfer")},
        {QStringLiteral("qb::TargetsDialog"), QStringLiteral("Targets, statistics, and writing tools")},
        {QStringLiteral("qb::StatisticsDialog"), QStringLiteral("Text Statistics")},
        {QStringLiteral("qb::KeywordsDialog"), QStringLiteral("Keywords")},
        {QStringLiteral("qb::ProjectReplaceDialog"), QStringLiteral("Project Replace")},
        {QStringLiteral("qb::FormatFindWindow"), QStringLiteral("Find by Formatting")},
        {QStringLiteral("qb::ThemeManagerDialog"), QStringLiteral("Themes, colors, and fonts")},
        {QStringLiteral("qb::AddonManagerDialog"), QStringLiteral("Updates and add-ons")},
        {QStringLiteral("qb::SystemInfoDialog"), QStringLiteral("Crash reports and system information")},
        {QStringLiteral("qb::CrashReportsDialog"), QStringLiteral("Crash reports and system information")},
        {QStringLiteral("qb::ToolbarDialog"), QStringLiteral("Toolbars of your own")},
        {QStringLiteral("qb::IconBarDialog"), QStringLiteral("The icon bar")},
        {QStringLiteral("qb::LayoutsDialog"), QStringLiteral("Layouts")},
        {QStringLiteral("qb::StyleListDialog"), QStringLiteral("Styles")},
        {QStringLiteral("qb::StyleEditDialog"), QStringLiteral("Styles")},
        {QStringLiteral("qb::StylesPanel"), QStringLiteral("Styles")},
        {QStringLiteral("qb::StyleFromSelectionDialog"), QStringLiteral("Styles")},
        {QStringLiteral("qb::FontDialog"), QStringLiteral("Fonts")},
        {QStringLiteral("qb::ParagraphDialog"), QStringLiteral("Paragraph formatting")},
        {QStringLiteral("qb::DefaultFormattingDialog"), QStringLiteral("Default formatting")},
        {QStringLiteral("qb::TableDialog"), QStringLiteral("Tables")},
        {QStringLiteral("qb::PictureDialog"), QStringLiteral("Pictures beside the text, captions, and descriptions")},
        {QStringLiteral("qb::PageSetupDialog"), QStringLiteral("Printing")},
        {QStringLiteral("qb::PrintSettingsDialog"), QStringLiteral("Printing")},
        {QStringLiteral("qb::ImportSplitDialog"), QStringLiteral("Importing files")},
        {QStringLiteral("qb::ExportFilesDialog"), QStringLiteral("Exporting documents")},
        {QStringLiteral("qb::ObjectChooser"), QStringLiteral("Objects: characters, locations, and more")},
        {QStringLiteral("qb::ObjectTemplatesDialog"), QStringLiteral("Objects: characters, locations, and more")},
        {QStringLiteral("qb::MindMapOptionsDialog"), QStringLiteral("Mind maps")},
        {QStringLiteral("qb::ConvertQuotesDialog"), QStringLiteral("Corrections while typing")},
        {QStringLiteral("qb::ScriptSettingsDialog"), QStringLiteral("Scriptwriting")},
        {QStringLiteral("qb::ManageIconsDialog"), QStringLiteral("Icons")},
        {QStringLiteral("qb::IconFromTextDialog"), QStringLiteral("Icons")},
        {QStringLiteral("qb::CommandFinder"), QStringLiteral("The menus")},
        {QStringLiteral("qb::CompositionWindow"), QStringLiteral("Composition mode")},
    };
    return map.value(QString::fromLatin1(window->metaObject()->className()));
}

namespace {

// F1 where the main window's own action does not reach: in dialogs and other windows of the program
class F1Filter : public QObject
{
public:
    F1Filter(QAction *manual, QWidget *main) : QObject(main), m_manual(manual), m_main(main) {}

    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() != QEvent::KeyPress || !m_manual)
            return false;
        auto *k = static_cast<QKeyEvent *>(e);
        if (k->isAutoRepeat())
            return false;
        const QKeySequence pressed(k->keyCombination());
        bool mine = false;
        for (const QKeySequence &s : m_manual->shortcuts())
            mine = mine || s == pressed;
        if (!mine)
            return false;
        // the window of the object that gets the key (the active window is unknown on some systems)
        QWidget *w = o && o->isWidgetType() ? static_cast<QWidget *>(o)->window() : QApplication::activeWindow();
        if (!w || w == m_main || qobject_cast<ManualWindow *>(w) || w->window() == m_main)
            return false; // the main window's action opens the manual there
        // a window of another application part: its section, or the start
        QWidget *top = w;
        for (QWidget *p = w; p; p = p->parentWidget())
            if (!sectionForWindow(p).isEmpty()) {
                top = p;
                break;
            }
        showManualAt(w, sectionForWindow(top));
        return true;
    }

private:
    QPointer<QAction> m_manual;
    QPointer<QWidget> m_main;
};

} // namespace

void installF1(QAction *manual, QWidget *mainWindow)
{
    qApp->installEventFilter(new F1Filter(manual, mainWindow));
}

} // namespace manualhelp
} // namespace qb
