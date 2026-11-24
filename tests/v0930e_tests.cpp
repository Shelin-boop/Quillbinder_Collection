// Scripted run for version 0.93.0 beta, fifth suite: to-do items 11, 17, 18 and 30.
//  - (a) item 11: the save text, the warning link and the short messages live in the empty stretch of the first editor's
//    footer bar; the window has no status bar row (it gains the row); two editors; a narrow editor; the corkboard's footer;
//    no project: nothing;
//  - (b) item 17: the manual and the Quick Start take the version's label from the program; the README names it too;
//  - (c) item 18: docs/design-decisions.md (ten entries, entry 2 of today's code), its links, the code's comments;
//  - (d) item 30: Help > Report a Problem..., the manual's words, the README's picture (docs/images/) and its note on
//    where the picture comes from. With QB_README_SHOT=<file.png> the run makes that picture of the welcome project.
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0930e_tests /tmp/shots
#include "AppInfo.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/EditorPane.h"
#include "ui/FooterStatus.h"
#include "ui/MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPointer>
#include <QRegularExpression>
#include <QStatusBar>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTimer>

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
            std::printf("      %s\n", qPrintable(detail.left(3000)));
    }
    std::fflush(stdout);
}

static void settle(int ms) { QTest::qWait(ms); }

static void shot(QWidget *w, const QString &name)
{
    settle(120);
    w->grab().save(outDir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

static QString readText(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

static const QString kRoot = QStringLiteral(QT_TESTCASE_SOURCEDIR "/..");

static BinderItem *textDoc(Project *p, BinderItem *parent, const QString &title, const QString &text)
{
    BinderItem *it = p->model()->createItem(parent, -1, ItemType::Text, title);
    QTextCursor c(p->docs()->text(it->id));
    c.insertText(text);
    return it;
}

static QAction *act(MainWindow &w, const char *name) { return w.findChild<QAction *>(QLatin1String(name)); }

// the footer status of the editor n
static FooterStatus *statusOf(MainWindow &w, int n)
{
    return w.editorPane(n)->footerStatus();
}

// ------------------------------------------------------------------ (a) item 11

static void footerChecks(const QString &W)
{
    std::printf("== the save text in the first editor's footer bar (item 11)\n");
    QString err;
    auto made = Project::create(W, QStringLiteral("Footer"), &err);
    BinderItem *chap = textDoc(made.get(), made->draft(), QStringLiteral("Chapter"), QStringLiteral("Some typed text for the footer."));
    textDoc(made.get(), made->draft(), QStringLiteral("Second"), QStringLiteral("More text."));
    made->save(&err);
    const QString folder = made->folderPath();
    const QString chapId = chap->id;
    made.reset();

    MainWindow w;
    w.resize(1300, 820);
    w.show();
    w.activateWindow();
    settle(300);
    FooterStatus *none = statusOf(w, 0);
    check(none && none->saveText().isEmpty() && !none->textLabel()->isVisible(), QStringLiteral("no project open: nothing is shown"));
    check(w.findChild<QStatusBar *>() == nullptr, QStringLiteral("the window has no status bar (its row is gone)"));
    w.openProject(folder);
    settle(500);
    Project *p = w.project();
    BinderItem *item = p ? p->item(chapId) : nullptr;
    check(item != nullptr, QStringLiteral("(project open)"));
    if (!item)
        return;
    auto *binder = w.findChild<BinderPanel *>();
    binder->select({item}, true);
    settle(300);
    const QList<EditorPane *> panes = {w.editorPane(0), w.editorPane(1)};
    FooterStatus *fs = statusOf(w, 0);
    QLabel *text = fs ? fs->textLabel() : nullptr;
    QTest::qWaitFor([&] { return fs && fs->saveText() == QLatin1String("Saved"); }, 12000);
    check(fs && text && text->isVisible() && text->text() == QLatin1String("Saved") && fs->saveText() == QLatin1String("Saved"),
          QStringLiteral("the footer bar of the first editor says Saved"), text ? text->text() : QString());
    check(!panes.isEmpty() && text && panes.first()->isAncestorOf(text) && !w.findChild<QStatusBar *>(),
          QStringLiteral("... inside the editor, not in a row of the window"));
    const QRect cen = w.centralWidget()->geometry();
    check(cen.bottom() == w.height() - 1 || (cen.bottom() + 1 == w.height()),
          QStringLiteral("the window's contents reach its bottom edge: the row is gained"),
          QStringLiteral("central %1..%2 in a window %3 high").arg(cen.top()).arg(cen.bottom()).arg(w.height()));
    // the save text moves with the text
    QTextCursor c(p->docs()->text(chapId));
    c.movePosition(QTextCursor::End);
    c.insertText(QStringLiteral(" Typed more."));
    settle(300);
    check(fs->saveText() == QLatin1String("Not saved yet") || fs->saveText() == QLatin1String("Saved"), QStringLiteral("a change shows in the same place"));
    check(QTest::qWaitFor([&] { return fs->saveText() == QLatin1String("Saved"); }, 12000), QStringLiteral("... and Saved comes back after the autosave"));
    shot(&w, QStringLiteral("footer-one-editor"));
    // where the text sits: between the left end and the word count, in the empty stretch
    QWidget *footerBar = fs->parentWidget();
    check(footerBar && fs->geometry().left() > 0 && text->mapTo(&w, QPoint(0, 0)).y() > w.height() / 2,
          QStringLiteral("it stands in the footer bar at the bottom of the editor"));

    // a short message takes the place, then the save text comes back
    w.showStatusNote(QStringLiteral("A short message."), 700);
    settle(100);
    check(text->text() == QLatin1String("A short message.") && w.statusNote() == QLatin1String("A short message."),
          QStringLiteral("a short message shows in the same place"), text->text());
    shot(&w, QStringLiteral("footer-message"));
    settle(900);
    check(text->text() == fs->saveText() && w.statusNote().isEmpty(), QStringLiteral("... and after its seconds the save text is back"), text->text());
    // a message too long: shortened, the tooltip has it all
    const QString longMsg = QStringLiteral("The automatic backup failed because the folder for backups cannot be written and the drive says that it is full, "
                                           "so nothing was copied and the old backups stay as they are until the next try.");
    fs->setFixedWidth(260);
    w.showStatusNote(longMsg, 0);
    settle(100);
    check(text->text() != longMsg && text->text().endsWith(QChar(0x2026)) && text->toolTip() == longMsg && text->width() <= 260,
          QStringLiteral("a message too long for the place is shortened, its tooltip is the whole text"), text->text());
    shot(&w, QStringLiteral("footer-long-message"));
    w.clearStatusNote();
    // a narrow editor: a small colored mark, the text as its tooltip
    fs->setFixedWidth(24);
    settle(100);
    check(fs->isMark() && text->text() != QLatin1String("Saved") && text->toolTip().contains(fs->saveText()) && text->width() <= 24,
          QStringLiteral("in a place too narrow for the text: a small mark with the text as its tooltip"), text->text() + QLatin1String(" / ") + text->toolTip());
    shot(&w, QStringLiteral("footer-narrow-mark"));
    fs->setMinimumWidth(0);
    fs->setMaximumWidth(QWIDGETSIZE_MAX);
    settle(100);
    check(!fs->isMark() && text->text() == fs->saveText(), QStringLiteral("... and the text again when the place is wide"));
    // a really narrow window
    w.resize(560, 820);
    settle(300);
    shot(&w, QStringLiteral("footer-narrow-window"));
    w.resize(1300, 820);
    settle(200);

    // the warning link beside the save text
    qputenv("QB_TEST_PORTABLE", "1");
    AppSettings &as = AppSettings::instance();
    as.backupDir.clear();
    as.backupOnClose = true;
    as.notifyChanged();
    settle(200);
    QLabel *link = fs->backupLabel();
    check(link->isVisible() && link->text().contains(QLatin1String("No backups")) && panes.first()->isAncestorOf(link)
              && link->geometry().left() >= text->geometry().right() && text->text() == fs->saveText(),
          QStringLiteral("the link \"No backups\" stands beside the save text"), link->text());
    shot(&w, QStringLiteral("footer-link"));
    bool opened = false;
    QPointer<QTimer> t = new QTimer;
    t->setInterval(50);
    QObject::connect(t.data(), &QTimer::timeout, [&opened] {
        if (QWidget *m = QApplication::activeModalWidget()) {
            opened = true;
            m->close();
        }
    });
    t->start();
    QTest::mouseClick(link, Qt::LeftButton, Qt::NoModifier, QPoint(12, link->height() / 2));
    settle(300);
    check(opened, QStringLiteral("a click on the link opens File > Options > Backup"));
    delete t.data();
    qunsetenv("QB_TEST_PORTABLE");
    as.notifyChanged();
    settle(200);
    check(!link->isVisible(), QStringLiteral("the link is gone when backups are no concern"));

    // the update check's note appears there and fades twice
    w.showFadingNote(QStringLiteral("Could not look for updates."));
    settle(300);
    QLabel *fading = w.findChild<QLabel *>(QStringLiteral("fadingNote"));
    check(fading && fading->isVisible() && panes.first()->isAncestorOf(fading) && fading->property("fades").toInt() == 2,
          QStringLiteral("the fading note shows in the footer bar of the first editor"));
    shot(&w, QStringLiteral("footer-fading-note"));
    QPointer<QLabel> fp = fading;
    check(QTest::qWaitFor([&] { return fp.isNull(); }, 12000) && text->isVisible(), QStringLiteral("... and is gone after its rounds, the save text is there"));

    // the corkboard's footer holds it too
    binder->select({p->draft()}, true);
    settle(200);
    w.editorPane(0)->setViewMode(ViewMode::Corkboard);
    settle(300);
    check(fs->isVisible() && text->isVisible() && text->text() == fs->saveText() && !fs->saveText().isEmpty(),
          QStringLiteral("the corkboard's footer bar holds the save text as well"));
    shot(&w, QStringLiteral("footer-corkboard"));
    binder->select({item}, true);
    settle(300);

    // two editors: the first one's footer bar shows it, the second one's none
    act(w, "splitV")->trigger();
    settle(500);
    FooterStatus *a = statusOf(w, 0), *b = statusOf(w, 1);
    check(a && b && a != b && a->textLabel()->isVisible() && !b->textLabel()->isVisible() && b->saveText().isEmpty(),
          QStringLiteral("with two editors only the first one's footer bar shows the save text"));
    shot(&w, QStringLiteral("footer-two-editors"));
    w.showStatusNote(QStringLiteral("Done."), 600);
    settle(100);
    check(a && b && a->textLabel()->text() == QLatin1String("Done.")  && !b->textLabel()->isVisible(),
          QStringLiteral("... and so does a message"));
    settle(800);
    act(w, "noSplit")->trigger();

    // closing the project: nothing
    const bool closed = w.closeProject();
    settle(1300);
    check(closed && statusOf(w, 0)->saveText().isEmpty() && !statusOf(w, 0)->textLabel()->isVisible(), QStringLiteral("closing the project leaves nothing in the footer"));
    w.close();
    settle(100);
}

// ------------------------------------------------------------------ (b) item 17

static void labelChecks(MainWindow &w)
{
    std::printf("== the label in the documents (item 17)\n");
    const QString label = QStringLiteral(QB_APP_VERSION_LABEL);
    const QString manual = readText(kRoot + QStringLiteral("/docs/manual.md"));
    const QString quick = readText(kRoot + QStringLiteral("/docs/quick-start.md"));
    const QString readme = readText(kRoot + QStringLiteral("/README.md"));
    check(!label.isEmpty() && manual.contains(QLatin1String("{{APP}} is {{LABEL}} software")) && quick.contains(QLatin1String("-full-{{LABEL}}-x86_64")),
          QStringLiteral("the manual and the Quick Start take the label through a placeholder"));
    const QRegularExpression stale(QStringLiteral("\\b(alpha|beta)\\b(?= software)|-full-(alpha|beta)-"));
    check(!stale.match(manual).hasMatch() && !stale.match(quick).hasMatch(), QStringLiteral("... no label is written out in them"));
    check(readme.contains(QStringLiteral("Quillbinder is %1 software").arg(label))
              && readme.contains(QStringLiteral("-%1-full-%2-x86_64.AppImage").arg(QLatin1String(QB_APP_VERSION), label)),
          QStringLiteral("the README names the same label (it cannot use the placeholder)"));
    for (const char *name : {"manual", "quickStart"}) {
        QAction *a = act(w, name);
        if (a)
            a->trigger();
        settle(400);
        QTextBrowser *tb = nullptr;
        for (QTextBrowser *b : QApplication::activeWindow() ? QApplication::activeWindow()->findChildren<QTextBrowser *>() : QList<QTextBrowser *>())
            if (b->objectName() == QLatin1String(name))
                tb = b;
        if (!tb)
            for (QWidget *top : QApplication::topLevelWidgets())
                if (auto *b = top->findChild<QTextBrowser *>(QLatin1String(name)))
                    tb = b;
        const QString shown = tb ? tb->toPlainText() : QString();
        check(tb && !shown.contains(QLatin1String("{{")) && (QLatin1String(name) == QLatin1String("manual") ? shown.contains(label + QStringLiteral(" software"))
                                                                                                             : shown.contains(QStringLiteral("-full-%1-x86_64").arg(label))),
              QStringLiteral("Help > %1 shows the label \"%2\"").arg(QLatin1String(name), label),
              QString::number(tb != nullptr) + QStringLiteral(" braces %1 ").arg(shown.indexOf(QLatin1String("{{"))) + shown.mid(shown.indexOf(QLatin1String("Getting the program")), 900));
        if (tb)
            tb->window()->close();
    }
}

// ------------------------------------------------------------------ (c) item 18

static void designChecks()
{
    std::printf("== docs/design-decisions.md (item 18)\n");
    const QString doc = readText(kRoot + QStringLiteral("/docs/design-decisions.md"));
    int entries = 0;
    for (int i = 1; i <= 10; ++i)
        if (doc.contains(QRegularExpression(QStringLiteral("\n## %1\\. ").arg(i))))
            ++entries;
    check(entries == 10, QStringLiteral("the document has its ten entries"), QString::number(entries));
    check(doc.contains(QLatin1String("[recorded")) && doc.contains(QLatin1String("[reasoning")) && doc.contains(QLatin1String("**What would change it.**"))
              && doc.startsWith(QLatin1String("# Design decisions")),
          QStringLiteral("... with its head and the marks"));
    const int e2 = int(doc.indexOf(QLatin1String("## 2.")));
    const QString entry2 = doc.mid(e2, doc.indexOf(QLatin1String("## 3.")) - e2);
    check(entry2.contains(QLatin1String("EBADF")) && entry2.contains(QLatin1String("EACCES")) && entry2.contains(QLatin1String("EPERM"))
              && entry2.contains(QLatin1String("folder sync")) && entry2.contains(QLatin1String("probeFolderSync")) && entry2.contains(QLatin1String("journal")),
          QStringLiteral("entry 2 says what 0.93.0 does with folder syncs"));
    const QString sb = readText(kRoot + QStringLiteral("/src/core/SaveBatch.cpp"));
    check(sb.contains(QLatin1String("e == EBADF || e == EACCES || e == EPERM")), QStringLiteral("(the code entry 2 describes)"));
    QString who = doc.toLower();
    check(!who.contains(QLatin1String("claude")) && !who.contains(QRegularExpression(QStringLiteral("\\bchats?\\b"))),
          QStringLiteral("no names of chats or people in it"));
    for (const char *f : {"/README.md", "/SECURITY.md", "/docs/manual.md"})
        check(readText(kRoot + QLatin1String(f)).contains(QLatin1String("design-decisions.md")), QStringLiteral("%1 links the document").arg(QLatin1String(f)));
    // the code's comments at the places of entries 1 to 4 and 10
    QSet<int> found;
    QDirIterator it(kRoot + QStringLiteral("/src"), {QStringLiteral("*.cpp"), QStringLiteral("*.h")}, QDir::Files, QDirIterator::Subdirectories);
    static const QRegularExpression re(QStringLiteral("docs/design-decisions\\.md, entry (\\d+)"));
    while (it.hasNext()) {
        const QString t = readText(it.next());
        auto m = re.globalMatch(t);
        while (m.hasNext())
            found.insert(m.next().captured(1).toInt());
    }
    check(found.contains(1) && found.contains(2) && found.contains(3) && found.contains(4) && found.contains(10),
          QStringLiteral("the code names the entries at the places of entries 1, 2, 3, 4 and 10"));
}

// ------------------------------------------------------------------ (d) item 30

static void reportChecks(MainWindow &w, bool withProject)
{
    QAction *a = act(w, "reportProblem");
    bool inHelp = false;
    for (QAction *m : w.menuBar()->actions())
        if (m->menu() && m->menu()->title().contains(QLatin1String("Help")))
            inHelp = m->menu()->actions().contains(a);
    check(a && inHelp && a->isEnabled() && a->text().startsWith(QLatin1String("Report a Problem")),
          QStringLiteral("Help > Report a Problem... is there and enabled %1").arg(withProject ? QStringLiteral("(project open)") : QStringLiteral("(no project)")));
}

static void docChecks()
{
    std::printf("== where to report a problem; the README's picture (item 30)\n");
    const QString manual = readText(kRoot + QStringLiteral("/docs/manual.md"));
    check(manual.contains(QLatin1String("Help > Report a Problem")) && manual.contains(QLatin1String("issues/new/choose")) && manual.contains(QLatin1String("SECURITY.md")),
          QStringLiteral("the manual's troubleshooting section names the issue forms and the private way for security reports"));
    const QString readme = readText(kRoot + QStringLiteral("/README.md"));
    const QRegularExpression img(QStringLiteral("!\\[[^\\]]*\\]\\((docs/images/[^)]+\\.png)\\)"));
    const auto m = img.match(readme);
    check(m.hasMatch() && QFileInfo(kRoot + QLatin1Char('/') + m.captured(1)).size() > 20000, QStringLiteral("the README shows a picture of the main window"),
          m.hasMatch() ? m.captured(1) : QString());
    const QString note = readText(kRoot + QStringLiteral("/docs/images/README.md"));
    check(note.contains(QLatin1String("welcome")) && note.contains(QLatin1String("QB_README_SHOT")),
          QStringLiteral("docs/images/README.md says where the picture comes from and how it is made"));
}

static void makeReadmePicture(const QString &target)
{
    std::printf("== making the README picture: %s\n", qPrintable(target));
    AppSettings &as = AppSettings::instance();
    QTemporaryDir docs;
    as.welcomeFolder = docs.path();
    as.welcomeShown = false;
    MainWindow w;
    w.resize(1300, 820);
    w.show();
    w.activateWindow();
    settle(400);
    const bool ok = w.openWelcomeOnFirstStart();
    settle(1500);
    w.activateWindow();
    w.clearStatusNote(); // (the message names the temporary folder)
    settle(2500);
    QDir().mkpath(QFileInfo(target).absolutePath());
    check(ok && w.grab().save(target), QStringLiteral("the picture is saved"));
    w.close();
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    qputenv("LANG", "C");
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0930eTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0930eTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    const QString shotTarget = qEnvironmentVariable("QB_README_SHOT");
    if (!shotTarget.isEmpty()) {
        makeReadmePicture(shotTarget);
        std::printf("%d failure(s)\n", failures);
        return failures;
    }
    QTemporaryDir work;
    footerChecks(work.path());
    {
        MainWindow w;
        w.resize(1300, 820);
        w.show();
        settle(300);
        reportChecks(w, false);
        labelChecks(w);
        w.close();
        settle(100);
    }
    designChecks();
    docChecks();
    std::printf("%d failure(s)\n", failures);
    return failures;
}
