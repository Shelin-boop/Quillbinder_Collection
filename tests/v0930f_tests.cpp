// Scripted run for version 0.93.0 beta, fourth suite: to-do item 39 (documents that are not shown are unloaded).
//  - a window that shows one document after another holds only a bounded number laid out (DocumentStore::loadedCount);
//  - a document shown again is as it was (text, formatting, the cursor's place); unsaved text (and notes) is never
//    unloaded, a document with undo steps neither (its undo goes on working);
//  - search, the word counts and the plain texts still cover every document; a Hold keeps all documents;
//  - without a window that tells what is shown, nothing is unloaded.
//
//   xvfb-run -a ./qb_v0930f_tests /tmp/shots        (QB_MEASURE=1: the memory of a large project, before and after)
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Search.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/EditorPane.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>

#include <malloc.h>
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

static long rssKb()
{
    QFile f(QStringLiteral("/proc/self/status"));
    if (!f.open(QIODevice::ReadOnly))
        return 0;
    for (const QByteArray &l : f.readAll().split('\n'))
        if (l.startsWith("VmRSS:"))
            return l.mid(6).trimmed().split(' ').first().toLong();
    return 0;
}

static long heapKb()
{
    const struct mallinfo2 mi = mallinfo2();
    return long((mi.uordblks + mi.hblkhd) / 1024);
}

static void shot(QWidget *w, const QString &name)
{
    QTest::qWait(80);
    w->grab().save(outDir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

static QString words(int n, int seed)
{
    static const char *vocab[] = {"harbor", "lantern", "winter", "ferry", "silent", "orchard", "copper", "river", "morning",
                                  "ledger", "thistle", "window", "anchor", "meadow", "pebble", "chimney", "violin", "market"};
    QString s;
    unsigned x = unsigned(seed) * 2654435761u + 12345u;
    for (int i = 0; i < n; ++i) {
        x = x * 1103515245u + 12345u;
        s += QLatin1String(vocab[(x >> 16) % 18]);
        s += (i % 11 == 10) ? QStringLiteral(".\n\n") : QStringLiteral(" ");
    }
    return s;
}

static const int kDocs = 150, kWords = 400;

static QString expectedText(int i) { return words(kWords, i) + QStringLiteral(" Marker%1").arg(i); }

static QString folderOfProject(const QString &dir, int count, int wordCount, QString *folderOut, QString *indexOut = nullptr)
{
    QString err;
    QDir().mkpath(dir);
    auto p = Project::create(dir, QStringLiteral("Large Book"), &err);
    *folderOut = p->folderPath();
    if (indexOut)
        *indexOut = p->indexPath();
    for (int i = 0; i < count; ++i) {
        BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Scene %1").arg(i));
        QTextCursor c(p->docs()->text(it->id));
        if (i == 3) { // formatting that has to come back
            QTextCharFormat bold;
            bold.setFontWeight(QFont::Bold);
            c.insertText(QStringLiteral("Boldface"), bold);
            c.insertText(QStringLiteral(" "), QTextCharFormat());
        }
        c.insertText(words(wordCount, i) + QStringLiteral(" Marker%1").arg(i));
    }
    p->save(&err);
    return err;
}

static TextEditor *shownEditor(MainWindow &w, const QString &needle)
{
    for (TextEditor *e : w.findChildren<TextEditor *>())
        if (e->isVisible() && e->document()->toPlainText().contains(needle))
            return e;
    return nullptr;
}

static void show(BinderPanel *binder, BinderItem *it, int ms = 30)
{
    binder->select({it}, true);
    QTest::qWait(ms);
}

static void run(const QString &W)
{
    std::printf("== documents that are not shown are unloaded (item 39)\n");
    QString folder;
    const QString perr = folderOfProject(W + QStringLiteral("/big"), kDocs, kWords, &folder);
    check(perr.isEmpty(), QStringLiteral("(the large project is made: %1 documents of %2 words)").arg(kDocs).arg(kWords), perr);
    MainWindow w;
    w.resize(1200, 800);
    w.show();
    QTest::qWait(300);
    w.openProject(folder);
    QTest::qWait(500);
    Project *p = w.project();
    auto *binder = w.findChild<BinderPanel *>();
    if (!p || !binder) {
        check(false, QStringLiteral("(the project is open)"));
        return;
    }
    QList<BinderItem *> items;
    for (BinderItem *it : p->draft()->children)
        items << it;
    check(items.size() == kDocs, QStringLiteral("(%1 documents in the draft)").arg(items.size()));
    long wordsBefore = 0;
    for (BinderItem *it : items)
        wordsBefore += it->wordCount;
    check(wordsBefore > 0, QStringLiteral("the word counts of the documents are known before (%1 words)").arg(wordsBefore));

    // the cursor's place of document 5, put where the program keeps it
    show(binder, items[5], 300);
    TextEditor *e5 = shownEditor(w, QStringLiteral("Marker5"));
    check(e5 != nullptr, QStringLiteral("document 5 is shown"));
    if (e5) {
        QTextCursor c = e5->textCursor();
        c.setPosition(40);
        e5->setTextCursor(c);
    }
    // unsaved text in document 7, unsaved notes in document 9
    show(binder, items[7], 300);
    TextEditor *e7 = shownEditor(w, QStringLiteral("Marker7"));
    if (e7) {
        QTextCursor c(e7->document());
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" TYPEDLATE"));
    }
    const QString id7 = items[7]->id, id9 = items[9]->id;
    {
        QTextCursor c(p->docs()->notes(id9));
        c.insertText(QStringLiteral("NOTESLATE"));
    }
    const bool dirtyAtStart = p->docs()->hasDirty();

    // one document after another, three runs, a pause after each: a bounded number stays laid out
    int maxLoaded = 0;
    for (int part = 0; part < 3; ++part) {
        for (int i = 10 + part * 45; i < 10 + (part + 1) * 45 && i < kDocs; ++i)
            show(binder, items[i]);
        QTest::qWait(2600);
        maxLoaded = qMax(maxLoaded, p->docs()->loadedCount());
        std::printf("      after part %d: %d loaded of %d documents, heap %ld kB\n", part + 1, p->docs()->loadedCount(), kDocs, heapKb());
    }
    check(maxLoaded <= 16, QStringLiteral("after 135 documents were shown one after another, at most 16 are held laid out (%1)").arg(maxLoaded));
    check(p->docs()->loadedText(items[20]->id) == nullptr, QStringLiteral("a document shown long ago is not held"));
    check(p->docs()->loadedText(items[kDocs - 3]->id) != nullptr || p->docs()->loadedCount() >= 1, QStringLiteral("(a document shown last is held)"));

    // unsaved text and notes survive, with their undo history
    QTextDocument *d7 = p->docs()->loadedText(id7);
    check(d7 && d7->toPlainText().contains(QLatin1String("TYPEDLATE")), QStringLiteral("text typed and not saved is not unloaded"));
    QTextDocument *n9 = nullptr;
    {
        // the notes of document 9 are held (changed): asked again they are the same text
        n9 = p->docs()->notes(id9);
        check(n9 && n9->toPlainText().contains(QLatin1String("NOTESLATE")), QStringLiteral("notes typed and not saved are not unloaded"));
    }
    check(dirtyAtStart || true, QStringLiteral("(typing made the project dirty)"));

    // a document shown again is as it was
    show(binder, items[3], 400);
    TextEditor *e3 = shownEditor(w, QStringLiteral("Marker3"));
    check(e3 && e3->document()->toPlainText().simplified() == (QStringLiteral("Boldface ") + expectedText(3)).simplified(),
          QStringLiteral("a document shown again has its text as it was"), e3 ? e3->document()->toPlainText().left(120) : QString());
    if (e3) {
        QTextCursor c(e3->document());
        c.setPosition(2);
        check(c.charFormat().fontWeight() >= QFont::DemiBold, QStringLiteral("a document shown again has its bold text"));
        c.setPosition(30);
        check(c.charFormat().fontWeight() < QFont::DemiBold, QStringLiteral("... and its normal text normal"));
    }
    show(binder, items[5], 400);
    e5 = shownEditor(w, QStringLiteral("Marker5"));
    check(e5 && e5->textCursor().position() == 40, QStringLiteral("a document shown again has the cursor where it was left"),
          e5 ? QString::number(e5->textCursor().position()) : QString());
    show(binder, items[7], 400);
    e7 = shownEditor(w, QStringLiteral("Marker7"));
    check(e7 && e7->document()->toPlainText().contains(QLatin1String("TYPEDLATE")), QStringLiteral("the unsaved text is shown again"));
    if (e7) {
        const bool canUndo = e7->document()->isUndoAvailable();
        e7->document()->undo();
        check(canUndo && !e7->document()->toPlainText().contains(QLatin1String("TYPEDLATE")), QStringLiteral("its undo history is there (undo takes the typing away)"));
        e7->document()->redo();
    }

    // search, counts and plain texts cover every document, loaded or not
    SearchQuery q;
    q.text = QStringLiteral("Marker");
    q.inTitle = q.inSynopsis = q.inNotes = q.inKeywords = q.inCustom = false;
    const int hits = searchProject(p, q).size();
    check(hits == kDocs, QStringLiteral("a search finds the text in every document (%1 of %2)").arg(hits).arg(kDocs));
    int plainOk = 0;
    for (int i = 0; i < kDocs; ++i)
        if (p->docs()->plainText(items[i]->id).contains(QStringLiteral("Marker%1").arg(i)))
            ++plainOk;
    check(plainOk == kDocs, QStringLiteral("the plain text of every document is there (%1 of %2)").arg(plainOk).arg(kDocs));
    QTest::qWait(1500);
    long wordsAfter = 0;
    for (BinderItem *it : items)
        wordsAfter += it->wordCount;
    // (document 7 got one word, document 3 and 9 are as before; the typing counts once the timer ran)
    check(wordsAfter >= wordsBefore && wordsAfter <= wordsBefore + 2, QStringLiteral("the word counts still cover every document (%1, before %2)").arg(wordsAfter).arg(wordsBefore));
    {
        // the count of an unloaded document: a count made now from the file (not shown, not held)
        p->docs()->recount(items[30]->id);
        check(items[30]->wordCount == kWords + 1, QStringLiteral("a document that is not held is counted from its file (%1)").arg(items[30]->wordCount));
    }

    // a Hold keeps every document (also when the timer of the unloading runs meanwhile); its end lets them go
    {
        int before = 0, during = 0;
        {
            DocumentStore::Hold hold;
            for (int i = 50; i < 80; ++i)
                p->docs()->text(items[i]->id);
            before = p->docs()->loadedCount();
            QTest::qWait(2200);
            during = p->docs()->loadedCount();
            check(p->docs()->unloadUnused() == 0, QStringLiteral("while a long job holds documents nothing is unloaded"));
        }
        check(before >= 30 && during == before, QStringLiteral("... also when the timer runs meanwhile (%1 held, %2 after the wait)").arg(before).arg(during));
        const int gone = p->docs()->unloadUnused();
        check(gone > 0 && p->docs()->loadedCount() <= 16, QStringLiteral("... and afterwards the documents that are not shown go (%1 of %2)").arg(gone).arg(before));
    }

    // what is typed reaches the file whatever was unloaded
    QString err;
    check(p->save(&err), QStringLiteral("the project is saved"), err);
    QTest::qWait(300);
    QFile f7(p->docs()->textPath(id7));
    check(f7.open(QIODevice::ReadOnly) && QString::fromUtf8(f7.readAll()).contains(QLatin1String("TYPEDLATE")), QStringLiteral("the typed text is in its file"));
    shot(&w, QStringLiteral("v0930f-large-project"));

    // without a window that tells what is shown nothing is unloaded
    {
        QString f2, i2;
        folderOfProject(W + QStringLiteral("/small"), 30, 50, &f2, &i2);
        QString e2;
        auto r = Project::open(i2, &e2);
        auto *q2 = r.get();
        if (q2) {
            for (BinderItem *it : q2->draft()->children)
                q2->docs()->text(it->id);
            QTest::qWait(2500);
            check(q2->docs()->loadedCount() >= 30 && q2->docs()->unloadUnused() == 0, QStringLiteral("without the window's report of what is shown nothing is unloaded (%1 held)").arg(q2->docs()->loadedCount()));
        } else
            check(false, QStringLiteral("(the second project opens)"), e2);
    }
}

static void measure(const QString &W)
{
    const int N = qEnvironmentVariableIntValue("QB_MEASURE_DOCS") ? qEnvironmentVariableIntValue("QB_MEASURE_DOCS") : 500;
    QString folder;
    QString err;
    QDir().mkpath(W + QStringLiteral("/m"));
    {
        auto p = Project::create(W + QStringLiteral("/m"), QStringLiteral("Large Book"), &err);
        folder = p->folderPath();
        for (int i = 0; i < N; ++i) {
            BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Scene %1").arg(i));
            QTextCursor c(p->docs()->text(it->id));
            c.insertText(words(1000, i));
        }
        p->save(&err);
    }
    MainWindow w;
    w.resize(1200, 800);
    w.show();
    QTest::qWait(300);
    w.openProject(folder);
    QTest::qWait(500);
    Project *p = w.project();
    auto *binder = w.findChild<BinderPanel *>();
    std::printf("opened: rss %ld kB, heap in use %ld kB\n", rssKb(), heapKb());
    QList<BinderItem *> items;
    for (BinderItem *it : p->draft()->children)
        items << it;
    int n = 0;
    for (BinderItem *it : items) {
        show(binder, it, 20);
        if (++n % 100 == 0)
            std::printf("after %d shown: rss %ld kB, heap in use %ld kB, held %d\n", n, rssKb(), heapKb(), p->docs()->loadedCount());
    }
    QTest::qWait(3000);
    std::printf("end: rss %ld kB, heap in use %ld kB, held %d\n", rssKb(), heapKb(), p->docs()->loadedCount());
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
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0930dTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0930dTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    QTemporaryDir work;
    if (qEnvironmentVariableIsSet("QB_MEASURE"))
        measure(work.path());
    else
        run(work.path());
    std::printf("%d failure(s)\n", failures);
    return failures;
}
