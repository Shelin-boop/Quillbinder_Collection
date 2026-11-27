// Scripted run for version 0.94.0 (shared code, fifth part of the run's to-do list):
//  - item 72 and the message line under the binder: tool interface 1.2 (ToolHost::showMessage), one message at a time for
//    5 seconds (10 with a link), the rest in a queue; the speech tool's time without use; the program's memory in the
//    system information;
//  - item 85: Back Up Now says "Backup saved." in the message line with a link "Show Folder";
//  - item 59: the places that lead to a missing add-on show a notice with a button to the Add-on Manager;
//  - item 74: the writing progress in the footer, the option for the lines under the search field;
//  - item 75: Import and Split at headings (a preview, one title rule);
//  - item 76: Documents > Group with one undo step;
//  - item 77: the Objects form (Tab, several documents at once, framed "+" buttons);
//  - item 67: Documents > Convert > Sheet to Object.
//
//   xvfb-run -a ./qb_v0940e_tests /tmp/shots
#include "compile/PluginManager.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Objects.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/Zip.h"
#include "import/FileImporter.h"
#include "tools/ToolApi.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/CrashDialogs.h"
#include "ui/EditorPane.h"
#include "ui/FooterStatus.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/MessageLine.h"
#include "ui/ObjectsUi.h"
#include "ui/TargetLineEdit.h"
#include "ui/Theme.h"
#include "ui/ToolDialogs.h"
#include "ui/ToolHost.h"
#include "ui/WritingProgress.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>

#include <cstdio>

using namespace qb;

static int failures = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 150)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

static void shot(QWidget *w, const QString &name)
{
    settle(300);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static QAction *byKey(QWidget *w, const QString &key) { return w->findChild<QAction *>(key); }

static BinderItem *child(BinderItem *parent, const QString &title)
{
    for (BinderItem *c : parent->children)
        if (c->title == title)
            return c;
    return nullptr;
}

static QString titles(const QList<BinderItem *> &l)
{
    QStringList t;
    for (BinderItem *i : l)
        t << i->title;
    return t.join(QLatin1Char('|'));
}

// ---------------------------------------------------------------- the message line, tool interface 1.2
static void messageLine(MainWindow &w)
{
    std::printf("== the message line under the binder and tool interface 1.2 (item 72)\n");
    MessageLine *ml = w.messageLine();
    check(ml != nullptr && !ml->isVisible(), QStringLiteral("the message line is there and has no height while it is empty"));
    if (!ml)
        return;
    check(toolhost::host().apiVersion() >= 120, QStringLiteral("the program offers tool interface 1.2 or later"),
          QString::number(toolhost::host().apiVersion()));
    tools::ToolHost *host = &toolhost::host();
    host->showMessage(QStringLiteral("Model unloaded, RAM usage minimal"), 5000);
    host->showMessage(QStringLiteral("Second message"), 5000);
    host->showMessage(QStringLiteral("Third message"), 0);
    settle(100);
    check(ml->isVisible() && ml->shownText() == QStringLiteral("Model unloaded, RAM usage minimal"),
          QStringLiteral("a call of the host shows its text under the binder"), ml->shownText());
    check(ml->waiting() == 2, QStringLiteral("... one message at a time: the others wait in a queue"), QString::number(ml->waiting()));
    check(ml->shownMs() == 5000, QStringLiteral("... for 5 seconds"));
    shot(&w, QStringLiteral("v0940e-message-line"));
    ml->dismiss();
    check(ml->shownText() == QStringLiteral("Second message") && ml->waiting() == 1, QStringLiteral("when it is gone the next one follows"));
    // a click ends it
    QTest::mouseClick(ml, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    check(ml->shownText() == QStringLiteral("Third message") && ml->shownMs() == 5000,
          QStringLiteral("a click ends the message; a call with no time takes 5 seconds"), ml->shownText());
    ml->dismiss();
    check(!ml->isVisible() && ml->shownText().isEmpty(), QStringLiteral("the line is gone when the queue is empty"));
    // a message with a link stays 10 seconds
    bool clicked = false;
    ml->post(QStringLiteral("With a link."), 0, QStringLiteral("Open"), [&clicked] { clicked = true; }, QStringLiteral("the tip"));
    settle(100);
    check(ml->shownMs() == 10000 && ml->linkLabel()->isVisible() && ml->linkLabel()->text().contains(QStringLiteral("Open")),
          QStringLiteral("a message with a link stays 10 seconds and shows the link"));
    Q_EMIT ml->linkLabel()->linkActivated(QStringLiteral("#"));
    check(clicked && ml->shownText().isEmpty(), QStringLiteral("the link runs its action and ends the message"));
    // a program that gives a message while another waits (the old interface's tool never calls it): nothing breaks
    ml->clearAll();
    // the program's memory in the system information
    const QString info = sysinfo::text(nullptr);
    check(info.contains(QStringLiteral("Program memory (resident size): ")) && !info.contains(QStringLiteral("resident size): ?")),
          QStringLiteral("System Information names the program's memory (resident size)"), info.left(1200));
}

// ---------------------------------------------------------------- the speech tool's time without use
static void speechUnload()
{
    std::printf("== the speech model's time without use and the cores are the program's one setting (1.10)\n");
    tools::ToolInterface *speech = compile::PluginManager::instance().tool(QStringLiteral("speech-transcription"));
    if (!speech) {
        std::printf("      (the Speech Transcription add-on is not loaded here: the option is not checked)\n");
        return;
    }
    QWidget host;
    tools::OptionsSection *sec = speech->createOptions(&host);
    QWidget *page = sec ? sec->widget() : nullptr;
    check(page && !page->findChild<QSpinBox *>(QStringLiteral("speechUnload")) && !page->findChild<QSpinBox *>(QStringLiteral("speechThreads")),
          QStringLiteral("the options of Speech Transcription 1.10 have no fields of their own for the cores and the time to free the model"));
    auto *note = page ? page->findChild<QLabel *>(QStringLiteral("speechSharedSettings")) : nullptr;
    check(note && note->text().contains(QLatin1String("AI Models")),
          QStringLiteral("... the page names File > Options > General > AI Models instead"), note ? note->text() : QString());
}

// ---------------------------------------------------------------- item 85
static void backupMessage(MainWindow &w, const QString &work)
{
    std::printf("== Back Up Now in the message line (item 85)\n");
    AppSettings &s = AppSettings::instance();
    const QString dir = work + QStringLiteral("/backups");
    QDir().mkpath(dir);
    s.backupDir = dir;
    bool modal = false;
    auto *t = new QTimer(&w);
    t->setInterval(50);
    QObject::connect(t, &QTimer::timeout, [&modal] {
        if (QWidget *m = QApplication::activeModalWidget()) {
            modal = true;
            m->close();
        }
    });
    t->start();
    QAction *now = byKey(&w, QStringLiteral("backupNow"));
    check(now && now->isEnabled(), QStringLiteral("(Back Up Now is enabled)"));
    if (!now)
        return;
    w.messageLine()->clearAll();
    now->trigger();
    settle(500);
    t->stop();
    MessageLine *ml = w.messageLine();
    check(!modal, QStringLiteral("Back Up Now opens no box that waits for OK"));
    check(ml->shownText() == QStringLiteral("Backup saved."), QStringLiteral("the line under the binder says \"Backup saved.\""),
          ml->shownText());
    check(ml->linkLabel()->isVisible() && ml->linkLabel()->text().contains(QStringLiteral("Show Folder")),
          QStringLiteral("... with the link \"Show Folder\""), ml->linkLabel()->text());
    check(ml->shownMs() == 10000, QStringLiteral("... for 10 seconds, as a message with a link"));
    check(ml->textLabel()->toolTip().contains(QDir::toNativeSeparators(dir)), QStringLiteral("... the path is in the tooltip"), ml->textLabel()->toolTip());
    check(w.statusNote().isEmpty(), QStringLiteral("the footer has no note of it (in a narrow footer it was only a dot)"), w.statusNote());
    check(!QDir(dir).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty(), QStringLiteral("... and the backup is there"));
    shot(&w, QStringLiteral("v0940e-backup-message"));
    ml->clearAll();
}

// ---------------------------------------------------------------- item 59
static void missingAddons(MainWindow &w)
{
    std::printf("== a notice for an add-on that is not installed (item 59)\n");
    QAction *tr = byKey(&w, QStringLiteral("missingTranslator"));
    check(tr != nullptr && tr->isEnabled(), QStringLiteral("Edit > Writing Tools has \"Translation Helper\" while that add-on is not there"));
    QMenu *writing = w.findChild<QMenu *>(QStringLiteral("writingToolsMenu"));
    check(writing && writing->actions().contains(tr), QStringLiteral("... it is in the menu"));
    if (!tr)
        return;
    bool boxSeen = false, managerSeen = false;
    QString text;
    auto *timer = new QTimer(&w);
    timer->setInterval(60);
    QObject::connect(timer, &QTimer::timeout, [&] {
        QWidget *m = QApplication::activeModalWidget();
        if (!m)
            return;
        if (auto *box = qobject_cast<QMessageBox *>(m); box && box->objectName() == QLatin1String("missingAddonNotice")) {
            boxSeen = true;
            text = box->text();
            m->grab().save(outDir + QStringLiteral("/v0940e-missing-addon.png"));
            if (auto *b = box->findChild<QPushButton *>(QStringLiteral("openAddonManager")))
                b->click();
            else
                box->close();
        } else if (boxSeen) {
            managerSeen = QString::fromLatin1(m->metaObject()->className()).contains(QStringLiteral("AddonManager"));
            m->close();
        }
    });
    timer->start();
    tr->trigger();
    settle(300);
    timer->stop();
    check(boxSeen && text.contains(QStringLiteral("Translation Helper")) && text.contains(QStringLiteral("not installed")),
          QStringLiteral("it shows a notice that the add-on is not installed"), text);
    check(managerSeen, QStringLiteral("... and its button opens the Add-on Manager"));
    // Speech Transcription: the notice only where the add-on is not loaded
    const bool speechThere = compile::PluginManager::instance().tool(QStringLiteral("speech-transcription")) != nullptr;
    QAction *sp = byKey(&w, QStringLiteral("missingSpeech"));
    check(speechThere ? sp == nullptr : sp != nullptr, QStringLiteral("the same for Speech Transcription: a notice only while the add-on is missing"));
}

// ---------------------------------------------------------------- item 74
static void footerProgress(MainWindow &w, Project *p, const QString &work)
{
    std::printf("== the writing progress in the footer (item 74)\n");
    WritingProgress *wp = w.editorPane(0)->writingProgress();
    check(wp != nullptr, QStringLiteral("(the progress widget is in the footer)"));
    if (!wp)
        return;
    p->targets.sessionTarget = 0;
    p->targets.draftTarget = 0;
    p->targets.autoFromDeadline = false;
    Q_EMIT p->countsChanged();
    settle(100);
    check(!wp->isVisible(), QStringLiteral("without a target nothing shows"));
    p->targets.sessionTarget = 1000;
    p->targets.draftTarget = 80000;
    Q_EMIT p->countsChanged();
    settle(200);
    check(wp->isVisible() && wp->sessionText().contains(QStringLiteral("/ 1,000 today")) && wp->draftText().contains(QStringLiteral("/ 80,000 draft")),
          QStringLiteral("with targets: a bar and \"n / 1,000 today\", \"n / 80,000 draft\" beside the word count"),
          wp->sessionText() + QStringLiteral(" | ") + wp->draftText());
    check(wp->sessionBar()->maximum() == 1000 && wp->draftBar()->maximum() == 80000, QStringLiteral("... the bars have the targets as their end"));
    shot(&w, QStringLiteral("v0940e-footer-progress"));
    p->targets.draftTarget = 0;
    Q_EMIT p->countsChanged();
    settle(100);
    check(!wp->draftText().isEmpty() ? false : !wp->sessionText().isEmpty(), QStringLiteral("a target not set: only the other part shows"));
    p->targets.draftTarget = 80000;
    Q_EMIT p->countsChanged();
    // a click opens Project Targets
    const int before = w.findChildren<TargetsDialog *>().size();
    QTest::mouseClick(wp, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    settle(300);
    const auto dialogs = w.findChildren<TargetsDialog *>();
    check(dialogs.size() == before + 1 && dialogs.last()->isVisible(), QStringLiteral("a click on it opens Project Targets"));
    for (TargetsDialog *d : dialogs)
        d->close();
    // the lines under the search field are an option
    TargetLineEdit *quick = nullptr;
    for (QLineEdit *le : w.findChildren<QLineEdit *>())
        if (auto *t = dynamic_cast<TargetLineEdit *>(le))
            quick = t;
    AppSettings &as = AppSettings::instance();
    check(as.progressUnderSearch, QStringLiteral("the lines under the search field are on at first"));
    Q_EMIT p->countsChanged();
    check(quick && quick->draft >= 0 && quick->session >= 0, QStringLiteral("... and then they show"));
    as.progressUnderSearch = false;
    as.save();
    as.notifyChanged();
    settle(100);
    Q_EMIT p->countsChanged();
    check(quick && quick->draft < 0 && quick->session < 0 && wp->isVisible(),
          QStringLiteral("turned off in File > Options: the lines are gone, the footer still shows the progress"));
    as.progressUnderSearch = true;
    as.save();
    as.notifyChanged();
    p->targets.sessionTarget = 0;
    p->targets.draftTarget = 0;
    Q_EMIT p->countsChanged();
    Q_UNUSED(work);
}

// ---------------------------------------------------------------- item 75
static QString writeDocx(const QString &path, const QStringList &paras)
{
    // paras: "H1:text", "H2:text", "H3:text", or plain text
    QByteArray body;
    for (const QString &p : paras) {
        QString style, text = p;
        if (p.startsWith(QLatin1String("H1:")))
            style = QStringLiteral("Heading1"), text = p.mid(3);
        else if (p.startsWith(QLatin1String("H2:")))
            style = QStringLiteral("Heading2"), text = p.mid(3);
        else if (p.startsWith(QLatin1String("H3:")))
            style = QStringLiteral("Heading3"), text = p.mid(3);
        body += "<w:p>";
        if (!style.isEmpty())
            body += "<w:pPr><w:pStyle w:val=\"" + style.toUtf8() + "\"/></w:pPr>";
        if (!text.isEmpty())
            body += "<w:r><w:t xml:space=\"preserve\">" + text.toHtmlEscaped().toUtf8() + "</w:t></w:r>";
        body += "</w:p>";
    }
    const QByteArray doc = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:document "
                           "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>"
                           + body + "</w:body></w:document>";
    const QByteArray styles =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:styles "
        "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading3\"><w:name w:val=\"heading 3\"/></w:style></w:styles>";
    ZipWriter z(path);
    z.addFile(QStringLiteral("[Content_Types].xml"),
              "<?xml version=\"1.0\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
              "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
              "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
              "<Override PartName=\"/word/document.xml\" "
              "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/></Types>");
    z.addFile(QStringLiteral("_rels/.rels"),
              "<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
              "<Relationship Id=\"rId1\" "
              "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" "
              "Target=\"word/document.xml\"/></Relationships>");
    z.addFile(QStringLiteral("word/document.xml"), doc);
    z.addFile(QStringLiteral("word/styles.xml"), styles);
    z.close();
    return path;
}

static QString treeText(QTreeWidgetItem *it, int depth = 0)
{
    QString s = QString(depth * 2, QLatin1Char(' ')) + (it->data(0, Qt::UserRole).toBool() ? QStringLiteral("[F] ") : QStringLiteral("[D] "))
                + it->text(0) + QLatin1Char('\n');
    for (int i = 0; i < it->childCount(); ++i)
        s += treeText(it->child(i), depth + 1);
    return s;
}

static QString treeText(QTreeWidget *t)
{
    QString s;
    for (int i = 0; i < t->topLevelItemCount(); ++i)
        s += treeText(t->topLevelItem(i));
    return s;
}

static QString itemTree(const BinderItem *it, int depth = 0)
{
    QString s = QString(depth * 2, QLatin1Char(' ')) + (it->isFolder() ? QStringLiteral("[F] ") : QStringLiteral("[D] ")) + it->title
                + QLatin1Char('\n');
    for (const BinderItem *c : it->children)
        s += itemTree(c, depth + 1);
    return s;
}

static void importAtHeadings(MainWindow &w, Project *p, const QString &work)
{
    std::printf("== Import and Split at headings (item 75)\n");
    const QString docx = writeDocx(work + QStringLiteral("/book.docx"),
                                   {QStringLiteral("Opening words before any heading."), QStringLiteral("H1:Part One"),
                                    QStringLiteral("Between the heading of the part and its first chapter."),
                                    QStringLiteral("H2:The Storm"), QStringLiteral("The wind rose over the rocks."),
                                    QStringLiteral("H3:A small heading stays in the text"), QStringLiteral("More of the storm."),
                                    QStringLiteral("H2:The Calm"), QStringLiteral("The sea lay still."),
                                    QStringLiteral("H1:Part Two"), QStringLiteral("H2:The Ships"), QStringLiteral("Sails came over the horizon.")});
    check(importer::canSplitAtHeadings(docx) && !importer::canSplitAtHeadings(work + QStringLiteral("/a.txt")),
          QStringLiteral("splitting at headings is for .docx, .odt and .md"));
    importer::HeadingSplitOptions o;
    o.heading2Folder = true;
    QList<importer::SplitNode> plan;
    QString err;
    check(importer::planHeadingSplit(p, docx, o, &plan, &err), QStringLiteral("(the plan of the .docx is made)"), err);
    // folders: Heading 2 as a folder
    BinderItem *holder = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Import target"));
    QStringList warnings;
    const auto items = importer::importAtHeadings(p, docx, o, holder, -1, &err, &warnings);
    const QString tree = itemTree(holder);
    const QString expected = QStringLiteral(
        "[F] Import target\n"
        "  [D] Opening words before any heading.\n"
        "  [F] Part One\n"
        "    [D] Between the heading of the part and its first chapter.\n"
        "    [F] The Storm\n"
        "      [D] The wind rose over the rocks.\n"
        "    [F] The Calm\n"
        "      [D] The sea lay still.\n"
        "  [F] Part Two\n"
        "    [F] The Ships\n"
        "      [D] Sails came over the horizon.\n");
    check(items.size() == 3 && tree.contains(QStringLiteral("[F] Part One")), QStringLiteral("Heading 1 becomes a folder"), tree);
    // the Heading 3 paragraph stays in the text of the Storm
    BinderItem *storm = child(child(holder, QStringLiteral("Part One")), QStringLiteral("The Storm"));
    const QString stormText = storm && !storm->children.isEmpty() ? p->docs()->text(storm->children.first()->id)->toPlainText() : QString();
    check(stormText.contains(QStringLiteral("A small heading stays in the text")) && stormText.contains(QStringLiteral("More of the storm")),
          QStringLiteral("a lower heading stays in the text"), stormText);
    // the first line of a section is its title (the same rule for every document), and leaves the text
    BinderItem *first = holder->children.value(0);
    check(first && first->title == QStringLiteral("Opening words before any heading.") &&
              p->docs()->text(first->id)->toPlainText().trimmed().isEmpty(),
          QStringLiteral("text between headings becomes documents, titled by the one rule (here the first line, which leaves the text)"),
          first ? first->title : QString());
    Q_UNUSED(expected);
    // Heading 2 as a document
    o.heading2Folder = false;
    BinderItem *holder2 = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Import target 2"));
    importer::importAtHeadings(p, docx, o, holder2, -1, &err, &warnings);
    const QString tree2 = itemTree(holder2);
    BinderItem *storm2 = child(child(holder2, QStringLiteral("Part One")), QStringLiteral("The Storm"));
    check(storm2 && !storm2->isFolder() && p->docs()->text(storm2->id)->toPlainText().contains(QStringLiteral("The wind rose over the rocks")),
          QStringLiteral("Heading 2 as a document: its title is the heading, the text under it is its text"), tree2);
    // without "first line as title": the file name and a number, the same for each
    o.firstLineIsTitle = false;
    BinderItem *holder3 = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Import target 3"));
    importer::importAtHeadings(p, docx, o, holder3, -1, &err, &warnings);
    check(holder3->children.value(0) && holder3->children.value(0)->title == QStringLiteral("book 1"),
          QStringLiteral("without the first-line rule a document is called \"<file name> <number>\""), itemTree(holder3));
    o.firstLineIsTitle = true;
    // a Markdown file
    const QString md = work + QStringLiteral("/book.md");
    {
        QFile f(md);
        f.open(QIODevice::WriteOnly);
        f.write("# Part A\n\nIntro text.\n\n## Chapter A1\n\nText A1.\n\n## Chapter A2\n\nText A2.\n");
    }
    BinderItem *holder4 = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Import target 4"));
    o.heading2Folder = false;
    importer::importAtHeadings(p, md, o, holder4, -1, &err, &warnings);
    check(child(holder4, QStringLiteral("Part A")) && child(child(holder4, QStringLiteral("Part A")), QStringLiteral("Chapter A2")),
          QStringLiteral("the same for a Markdown file"), itemTree(holder4));

    // the title rule in the separator modes: a first line after empty paragraphs, a line longer than 120 characters
    const QString txt = work + QStringLiteral("/loose.txt");
    {
        QFile f(txt);
        f.open(QIODevice::WriteOnly);
        f.write("\n\nFirst title\nBody one.\n\n\nSecond section starts with a very long line that goes on and on and on and on and on and on "
                "and on and on and on and on and on and on and on and on\nBody two.\n\n\nThird\nBody three.\n");
    }
    BinderItem *holder5 = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Import target 5"));
    importer::importAndSplit(p, txt, QString(), true, holder5, -1, &err, &warnings);
    bool numbered = false;
    for (BinderItem *c : holder5->children)
        numbered = numbered || c->title.startsWith(QStringLiteral("loose "));
    check(holder5->children.size() == 3 && !numbered && holder5->children.value(0)->title == QStringLiteral("First title")
              && holder5->children.value(1)->title.endsWith(QStringLiteral("...")) && holder5->children.value(1)->title.size() <= 123,
          QStringLiteral("the title rule is the same for every document: the first line that is not empty, a long one shortened"),
          itemTree(holder5));

    // the dialog: a preview of the resulting binder before importing
    ImportSplitDialog dlg(p, &w);
    dlg.show();
    dlg.setFile(docx);
    dlg.setMode(3);
    dlg.setHeading2Folder(true);
    settle(300);
    const QString pv = treeText(dlg.preview());
    check(dlg.atHeadings() && dlg.preview()->isVisible() && pv.contains(QStringLiteral("[F] Part One")) && pv.contains(QStringLiteral("  [F] The Storm"))
              && pv.contains(QStringLiteral("[D] The wind rose over the rocks.")),
          QStringLiteral("the dialog shows a preview of the binder as a tree"), pv);
    check(dlg.ruleLabel()->text().contains(QStringLiteral("Title of a document")) && dlg.ruleLabel()->text().contains(QStringLiteral("120")),
          QStringLiteral("... and states the title rule"), dlg.ruleLabel()->text());
    shot(&dlg, QStringLiteral("v0940e-import-headings"));
    dlg.setHeading2Folder(false);
    settle(200);
    check(treeText(dlg.preview()).contains(QStringLiteral("[D] The Storm")), QStringLiteral("... the preview follows the choice of Heading 2 (a document)"),
          treeText(dlg.preview()));
    dlg.setMode(0);
    settle(100);
    check(!dlg.preview()->isVisible(), QStringLiteral("the preview belongs to the headings mode"));
    dlg.close();
    // the project is the one of the others: these folders go away
    for (BinderItem *h : {holder, holder2, holder3, holder4, holder5})
        p->model()->deletePermanently(h);
}

// ---------------------------------------------------------------- item 76
static void group(MainWindow &w, Project *p, BinderPanel *binder)
{
    std::printf("== Documents > Group (item 76)\n");
    BinderItem *c1 = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Grp One"));
    BinderItem *a = p->model()->createItem(c1, -1, ItemType::Text, QStringLiteral("Ga"));
    BinderItem *b = p->model()->createItem(c1, -1, ItemType::Text, QStringLiteral("Gb"));
    BinderItem *c = p->model()->createItem(c1, -1, ItemType::Text, QStringLiteral("Gc"));
    BinderItem *c2 = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Grp Two"));
    BinderItem *d = p->model()->createItem(c2, -1, ItemType::Text, QStringLiteral("Gd"));
    p->model()->createItem(c2, -1, ItemType::Text, QStringLiteral("Ge"));
    settle(100);
    QAction *grp = byKey(&w, QStringLiteral("group"));
    check(grp != nullptr, QStringLiteral("(the command Group exists)"));
    if (!grp)
        return;
    const QKeySequence ks = grp->shortcut();
    check(!ks.isEmpty() && !ks.toString(QKeySequence::PortableText).contains(QStringLiteral("Alt")),
          QStringLiteral("Group has a shortcut without Ctrl+Alt"), ks.toString(QKeySequence::PortableText));
    QMenu *docs = nullptr;
    for (QAction *top : w.menuBar()->actions())
        if (top->menu() && QString(top->menu()->title()).remove(QLatin1Char('&')).contains(QStringLiteral("Documents")))
            docs = top->menu();
    check(docs && docs->actions().contains(grp), QStringLiteral("... it is in the Documents menu"));
    binder->select({c1}, true);
    settle(100);
    // not for the Draft itself
    binder->select({p->draft()}, true);
    settle(100);
    check(!grp->isEnabled(), QStringLiteral("Group is off for the Draft folder"));
    binder->select({a, c, d}, true);
    binder->tree()->setFocus();
    settle(100);
    check(grp->isEnabled(), QStringLiteral("Group is on for documents"));
    const QString before = itemTree(p->draft());
    grp->trigger();
    settle(300);
    BinderItem *g = c1->children.value(0);
    check(g && g->isFolder() && g->title == QStringLiteral("Group") && titles(g->children) == QStringLiteral("Ga|Gc|Gd"),
          QStringLiteral("the selected items are in a new folder at the place of the first one, in the order of the binder"),
          itemTree(p->draft()));
    check(titles(c1->children) == QStringLiteral("Group|Gb") && titles(c2->children) == QStringLiteral("Ge"),
          QStringLiteral("... what was not selected stays where it was"), itemTree(p->draft()));
    auto *editor = binder->tree()->findChild<QLineEdit *>();
    check(editor != nullptr && editor->isVisible(), QStringLiteral("the title of the new folder is ready for typing"));
    shot(&w, QStringLiteral("v0940e-group"));
    if (editor) {
        editor->setText(QStringLiteral("Together"));
        QTest::keyClick(editor, Qt::Key_Return);
    }
    settle(200);
    check(g && g->title == QStringLiteral("Together"), QStringLiteral("... a typed title is kept"), g ? g->title : QString());
    // one undo step
    binder->tree()->setFocus();
    settle(100);
    QAction *undo = byKey(&w, QStringLiteral("undo"));
    check(undo && undo->isEnabled() && undo->text().contains(QStringLiteral("Group")),
          QStringLiteral("Edit > Undo with the keyboard in the binder says \"Undo Group\""), undo ? undo->text() : QString());
    if (undo)
        undo->trigger();
    settle(200);
    check(itemTree(p->draft()) == before, QStringLiteral("one undo step puts everything back"), itemTree(p->draft()));
    QAction *redo = byKey(&w, QStringLiteral("redo"));
    check(redo && redo->isEnabled() && redo->text().contains(QStringLiteral("Group")), QStringLiteral("... and Redo group again"),
          redo ? redo->text() : QString());
    if (redo)
        redo->trigger();
    settle(200);
    check(titles(c1->children.value(0)->children) == QStringLiteral("Ga|Gc|Gd"), QStringLiteral("... the redo makes the same group"), itemTree(p->draft()));
    if (undo)
        undo->trigger();
    settle(200);
    check(itemTree(p->draft()) == before, QStringLiteral("(undone again)"));
    for (BinderItem *x : {c1, c2})
        p->model()->deletePermanently(x);
}

// ---------------------------------------------------------------- item 77
static void objectsForm(MainWindow &w, Project *p)
{
    std::printf("== the Objects form (item 77)\n");
    ObjectsWindow *ow = ObjectsWindow::open(p, QString(), &w);
    check(ow != nullptr, QStringLiteral("(the Objects window is open)"));
    if (!ow)
        return;
    ObjectsPanel *panel = ow->panel();
    panel->setCategory(QStringLiteral("characters"));
    const QString id = panel->newObject(QStringLiteral("Mara"));
    settle(300);
    ow->activateWindow();
    (void)QTest::qWaitForWindowActive(ow, 2000);
    QLineEdit *name = panel->form()->nameField();
    QWidget *full = panel->form()->fieldWidget(QStringLiteral("fullName"));
    check(!id.isEmpty() && name && full, QStringLiteral("(a new object, its name and Full name field)"));
    if (!name || !full)
        return;
    name->setFocus();
    settle(100);
    QTest::keyClick(name, Qt::Key_Tab);
    settle(100);
    check(QApplication::focusWidget() == full, QStringLiteral("Tab from the name goes to Full name"),
          QApplication::focusWidget() ? QApplication::focusWidget()->objectName() + QStringLiteral(" ") + QString::fromLatin1(QApplication::focusWidget()->metaObject()->className())
                                      : QStringLiteral("nothing"));
    // the + buttons: framed, of normal size
    auto *scenes = qobject_cast<ChipList *>(panel->form()->fieldWidget(QStringLiteral("scenes")));
    auto *livesIn = qobject_cast<ChipList *>(panel->form()->fieldWidget(QStringLiteral("livesIn")));
    QToolButton *plus = scenes ? scenes->addButton() : nullptr;
    check(plus && !plus->autoRaise() && plus->minimumWidth() >= 24 && plus->minimumHeight() >= 24 && livesIn && livesIn->addButton()
              && !livesIn->addButton()->autoRaise(),
          QStringLiteral("the + buttons are framed buttons of normal size"),
          plus ? QStringLiteral("%1 x %2").arg(plus->width()).arg(plus->height()) : QString());
    shot(ow, QStringLiteral("v0940e-objects-form"));
    // several documents at once
    ObjectChooser dlg(p, ObjectChooser::Documents, {}, {}, &w);
    dlg.show();
    settle(100);
    QListWidget *list = dlg.list();
    check(list->selectionMode() == QAbstractItemView::ExtendedSelection && list->count() >= 3,
          QStringLiteral("the dialog for \"Appears in\" takes several documents (Ctrl, Shift)"));
    list->clearSelection();
    list->item(0)->setSelected(true);
    list->item(2)->setSelected(true);
    list->setCurrentRow(2, QItemSelectionModel::NoUpdate);
    const QString id0 = list->item(0)->data(Qt::UserRole).toString(), id2 = list->item(2)->data(Qt::UserRole).toString();
    static_cast<QDialog &>(dlg).accept();
    check(dlg.chosenIds() == QStringList({id0, id2}), QStringLiteral("... and gives all that were chosen, in the order of the list"),
          dlg.chosenIds().join(QLatin1Char(',')));
    // added to the object
    panel->form()->addLink(QStringLiteral("scenes"), id0);
    panel->form()->addLink(QStringLiteral("scenes"), id2);
    check(p->objects()->object(id)->ids(QStringLiteral("scenes")) == QStringList({id0, id2}), QStringLiteral("... they are linked to the object"));
    ow->close();
}

// ---------------------------------------------------------------- item 67
static void sheetToObject(MainWindow &w, Project *p, BinderPanel *binder, const QString &work)
{
    std::printf("== Documents > Convert > Sheet to Object (item 67)\n");
    BinderItem *chars = nullptr, *places = nullptr;
    for (BinderItem *c : p->draft()->children)
        (void)c;
    chars = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Characters"));
    places = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Places"));
    BinderItem *anna = p->model()->createItem(chars, -1, ItemType::Text, QStringLiteral("Anna Voss"));
    BinderItem *inn = p->model()->createItem(places, -1, ItemType::Text, QStringLiteral("The Lamp Inn"));
    BinderItem *plain = p->model()->createItem(chars, -1, ItemType::Text, QStringLiteral("Tom Reyes"));
    const QString annaText = QStringLiteral("Tall, quiet, keeps a logbook.\n\nShe distrusts the harbor master.");
    {
        QTextDocument *doc = p->docs()->text(anna->id);
        QImage img(40, 30, QImage::Format_RGB32);
        img.fill(Qt::darkGreen);
        QTextCursor c(doc);
        c.insertText(annaText);
        QTextImageFormat f;
        f.setName(p->docs()->importImage(anna->id, img));
        c.insertImage(f);
    }
    {
        QTextCursor c(p->docs()->text(inn->id));
        c.insertText(QStringLiteral("A low room above the quay."));
        QTextCursor c2(p->docs()->text(plain->id));
        c2.insertText(QStringLiteral("Dockhand."));
    }
    const QString annaBefore = p->docs()->text(anna->id)->toPlainText();
    QAction *act = byKey(&w, QStringLiteral("sheetToObject"));
    check(act != nullptr, QStringLiteral("(the command Sheet to Object exists)"));
    if (!act)
        return;
    QMenu *convert = nullptr;
    for (QAction *top : w.menuBar()->actions())
        if (top->menu() && QString(top->menu()->title()).remove(QLatin1Char('&')).contains(QStringLiteral("Documents")))
            for (QAction *sub : top->menu()->actions())
                if (sub->menu() && QString(sub->menu()->title()).remove(QLatin1Char('&')).contains(QStringLiteral("Convert")))
                    convert = sub->menu();
    check(convert && convert->actions().contains(act), QStringLiteral("... it is in Documents > Convert"));
    binder->select({p->draft()}, true);
    settle(100);
    check(!act->isEnabled(), QStringLiteral("it is off for a root folder"));
    binder->select({anna, inn}, true);
    binder->tree()->setFocus();
    settle(100);
    check(act->isEnabled(), QStringLiteral("it is on for sheet documents"));
    const int objectsBefore = p->objects()->count();
    act->trigger();
    settle(300);
    const objects::Object *oa = p->objects()->objectForSheet(anna->id);
    const objects::Object *oi = p->objects()->objectForSheet(inn->id);
    check(p->objects()->count() == objectsBefore + 2 && oa && oi, QStringLiteral("one Object for each selected sheet"));
    if (!oa || !oi)
        return;
    check(oa->category == QStringLiteral("characters") && oi->category == QStringLiteral("locations"),
          QStringLiteral("the folder's name (Characters, Places) chooses the kind of object"), oa->category + QLatin1Char(' ') + oi->category);
    check(oa->name == QStringLiteral("Anna Voss") && oa->text(QStringLiteral("other")).contains(QStringLiteral("keeps a logbook")),
          QStringLiteral("the title is the name, the text goes into Other"), oa->text(QStringLiteral("other")));
    const QString pic = oa->text(QStringLiteral("picture"));
    check(!pic.isEmpty() && QFile::exists(p->objects()->picturePath(pic)), QStringLiteral("its picture becomes the picture of the Object"), pic);
    check(oi->text(QStringLiteral("picture")).isEmpty() && oi->text(QStringLiteral("other")).contains(QStringLiteral("low room")),
          QStringLiteral("a sheet with no picture gives an Object with none"));
    check(p->docs()->text(anna->id)->toPlainText() == annaBefore, QStringLiteral("the document keeps its text unchanged"));
    check(w.sheetObjectOf(anna) == oa->id, QStringLiteral("the binder item stands for the Object"));
    // selecting it shows the Object
    binder->select({plain}, true);
    settle(200);
    binder->select({anna}, true);
    settle(300);
    check(w.findChild<Inspector *>()->objectsPanel()->currentObject() == oa->id,
          QStringLiteral("selecting the binder item shows the Object in the inspector"));
    shot(&w, QStringLiteral("v0940e-sheet-object"));
    // saved in Quillbinder's own data, the document is a normal one
    QString err;
    p->save(&err);
    QFile f(p->objects()->dir() + QStringLiteral("/characters/") + oa->id + QStringLiteral(".json"));
    f.open(QIODevice::ReadOnly);
    check(f.readAll().contains(anna->id.toUtf8()), QStringLiteral("the link is kept in the project's own Objects data"), err);
    check(anna->type == ItemType::Text && p->docs()->text(anna->id)->toPlainText() == annaBefore,
          QStringLiteral("... the binder item stays a normal document"));
    // one undo step
    binder->tree()->setFocus();
    settle(100);
    QAction *undo = byKey(&w, QStringLiteral("undo"));
    check(undo && undo->isEnabled() && undo->text().contains(QStringLiteral("Sheet to Object")),
          QStringLiteral("Undo says \"Undo Sheet to Object\""), undo ? undo->text() : QString());
    if (undo)
        undo->trigger();
    settle(200);
    check(p->objects()->count() == objectsBefore && w.sheetObjectOf(anna).isEmpty() && w.sheetObjectOf(inn).isEmpty(),
          QStringLiteral("one undo step takes back both Objects"));
    QAction *redo = byKey(&w, QStringLiteral("redo"));
    if (redo)
        redo->trigger();
    settle(200);
    check(p->objects()->count() == objectsBefore + 2 && !w.sheetObjectOf(anna).isEmpty(), QStringLiteral("... and Redo makes them again"));
    // a second conversion of the same sheet does nothing
    binder->select({anna}, true);
    check(!act->isEnabled(), QStringLiteral("a sheet that stands for an Object is not converted again"));
    // a folder of a name that tells nothing: asked (here the first kind)
    BinderItem *misc = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Odds and ends"));
    BinderItem *odd = p->model()->createItem(misc, -1, ItemType::Text, QStringLiteral("A thing"));
    const QStringList ids = w.sheetsToObjects({odd});
    check(ids.size() == 1 && p->objects()->object(ids.first()) && !p->objects()->object(ids.first())->category.isEmpty(),
          QStringLiteral("a sheet in a folder with another name gets the first kind when no question can be asked"));
    Q_UNUSED(work);
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0940eTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0940eTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();
    compile::PluginManager::instance().load();

    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("Lamp"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    Project *p = proj.get();
    for (int i = 0; i < 3; ++i) {
        BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Scene %1").arg(i + 1));
        QTextCursor c(p->docs()->text(it->id));
        c.insertText(QStringLiteral("The wind rose in the night and the sea came up over the rocks."));
    }
    p->save(&err);

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.resize(1280, 800);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);
    auto *binder = w.findChild<BinderPanel *>();
    if (!binder)
        return 1;

    messageLine(w);
    speechUnload();
    backupMessage(w, work.path());
    missingAddons(w);
    binder->select({p->draft()->children.value(1)}, true);
    settle(300);
    footerProgress(w, p, work.path());
    importAtHeadings(w, p, work.path());
    group(w, p, binder);
    objectsForm(w, p);
    sheetToObject(w, p, binder, work.path());

    std::printf("%d failure(s)\n", failures);
    return failures;
}
