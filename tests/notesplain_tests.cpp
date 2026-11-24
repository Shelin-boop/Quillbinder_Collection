// Scripted run for item 24 of 0.93.0: the notes of a document and the project notes start as plain text (no project
// font, indent, spacing or style; the interface's font), formatting set by hand stays, formatted notes of an existing
// project are unchanged, pasting into the notes is plain and Paste with Formatting keeps what the text brings.
// Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_notesplain_tests /tmp/shots
#include "compile/PluginManager.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "text/DisplayFormat.h"
#include "text/RtfIO.h"
#include "text/TextProps.h"
#include "ui/AppSettings.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QMimeData>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
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
static void settle(int ms = 100)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}
static QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// plain: no indent, spacing, style or line height in any paragraph
static bool blocksPlain(QTextDocument *d)
{
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.textIndent() != 0 || bf.leftMargin() != 0 || bf.topMargin() != 0 || bf.bottomMargin() != 0
            || !bf.property(props::Style).toString().isEmpty() || bf.lineHeightType() != QTextBlockFormat::SingleHeight)
            return false;
    }
    return true;
}

// a project of the given storage with a loud text formatting, so that anything taken from it shows
static std::unique_ptr<Project> makeProject(const QString &dir, StorageFormat storage, BinderItem **doc1, BinderItem **doc2)
{
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto p = templates::createProject(novel, dir, QStringLiteral("Notes Test"), &err);
    if (!p)
        return p;
    if (p->storage != storage)
        p->docs()->convertStorage(storage, &err);
    p->formatting.fontFamily = QStringLiteral("Liberation Serif");
    p->formatting.fontSize = 17;
    p->formatting.firstLineIndent = 36;
    p->formatting.paragraphSpacing = 14;
    p->formatting.lineSpacing = 1.6;
    p->docs()->applyFormatting();
    *doc1 = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("One"));
    *doc2 = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Two"));
    return p;
}

static void runStorage(const QString &name, StorageFormat storage)
{
    QTemporaryDir dir;
    BinderItem *d1 = nullptr, *d2 = nullptr;
    auto p = makeProject(dir.path(), storage, &d1, &d2);
    check(p != nullptr && p->storage == storage, QStringLiteral("(%1 project made)").arg(name));
    if (!p)
        return;
    const QString ext = storage == StorageFormat::Rtf ? QStringLiteral("rtf") : QStringLiteral("md");
    const QString id1 = d1->id, id2 = d2->id;
    const QFont ui = QGuiApplication::font();

    // ---- new notes start plain
    QTextDocument *n1 = p->docs()->notes(id1);
    check(n1->property(display::kPlainNotesProp).toBool() && n1->defaultFont().family() == ui.family()
              && blocksPlain(n1),
          QStringLiteral("%1: the notes of a new document are plain: the interface's font, no indent, spacing or style").arg(name));
    QTextDocument *pn = p->docs()->projectNotes();
    check(pn->property(display::kPlainNotesProp).toBool() && pn->defaultFont().family() == ui.family() && blocksPlain(pn),
          QStringLiteral("%1: the project notes are plain too").arg(name));
    // typed text, one word bold by hand
    {
        QTextCursor c(n1);
        c.insertText(QStringLiteral("First plain line"));
        c.insertBlock();
        c.insertText(QStringLiteral("Second line with bold here"));
        QTextCursor b(n1);
        b.setPosition(n1->toPlainText().indexOf(QStringLiteral("bold")));
        b.setPosition(b.position() + 4, QTextCursor::KeepAnchor);
        QTextCharFormat bf;
        bf.setFontWeight(QFont::Bold);
        b.mergeCharFormat(bf);
        QTextCursor c2(pn);
        c2.insertText(QStringLiteral("A project note"));
        QTextCursor pb(pn);
        pb.setPosition(2);
        pb.setPosition(5, QTextCursor::KeepAnchor);
        pb.mergeCharFormat(bf);
    }
    QString err;
    check(p->save(&err), QStringLiteral("(saved)"), err);
    const QString notesFile = p->contentDir(id1) + QStringLiteral("/notes.") + ext;
    const QByteArray raw = fileBytes(notesFile);
    if (storage == StorageFormat::Rtf) {
        const QByteArray body = raw.mid(qMax(0, raw.indexOf("\\pard"))); // (the style table before it names styles)
        const bool noMetrics = !QRegularExpression(QStringLiteral("\\\\(fi|li|sb|sa)-?[1-9]")).match(QString::fromLatin1(body)).hasMatch()
                               && !body.contains("\\sl") && !body.contains("\\s1");
        check(noMetrics && raw.contains("qbplainnotes"),
              QStringLiteral("%1: the saved notes carry no indent, spacing, line height or style (and the mark that they are plain)").arg(name),
              QString::fromLatin1(raw.left(600)));
    } else {
        check(raw.startsWith("<!--qb-plain-notes-->"), QStringLiteral("%1: the saved notes carry the mark that they are plain").arg(name));
    }

    // ---- reopened: still plain, bold stays
    const QString file = p->indexPath();
    p.reset();
    auto q = Project::open(file, &err);
    check(q != nullptr, QStringLiteral("(reopened)"), err);
    if (!q)
        return;
    {
        QTextDocument *r = q->docs()->notes(id1);
        const int at = r->toPlainText().indexOf(QStringLiteral("bold"));
        QTextCursor c(r);
        c.setPosition(at + 1);
        const bool bold = c.charFormat().fontWeight() >= QFont::DemiBold;
        c.setPosition(2);
        const bool restPlain = c.charFormat().fontWeight() < QFont::DemiBold;
        QTextDocument *rp = q->docs()->projectNotes();
        QTextCursor pc(rp);
        pc.setPosition(3);
        check(r->property(display::kPlainNotesProp).toBool() && r->defaultFont().family() == ui.family() && blocksPlain(r) && bold && restPlain
                  && r->toPlainText().contains(QStringLiteral("Second line with bold here")),
              QStringLiteral("%1: reopened, the notes are still plain and the bold applied by hand stays").arg(name));
        check(rp->property(display::kPlainNotesProp).toBool() && blocksPlain(rp) && pc.charFormat().fontWeight() >= QFont::DemiBold,
              QStringLiteral("%1: the same for the project notes").arg(name));
    }

    // ---- notes that were formatted before: unchanged
    {
        // a notes file as older versions wrote it: the project's formatting on every paragraph
        QTextDocument old;
        display::prepareDocument(&old, q->formatting, 1.0);
        QTextCursor c(&old);
        c.setBlockFormat(display::normalBlockFormat(q->formatting));
        c.insertText(QStringLiteral("Old notes text"));
        const QByteArray data = storage == StorageFormat::Rtf ? rtf::write(&old, q->formatting) : QByteArray("Old notes text\n");
        QDir().mkpath(q->contentDir(id2));
        QFile f(q->contentDir(id2) + QStringLiteral("/notes.") + ext);
        f.open(QIODevice::WriteOnly);
        f.write(data);
        f.close();
        QTextDocument *o = q->docs()->notes(id2);
        const bool formatted = !o->property(display::kPlainNotesProp).toBool() && o->toPlainText().contains(QStringLiteral("Old notes"))
                               && o->begin().blockFormat().textIndent() > 20 && o->defaultFont().family() != ui.family();
        check(formatted, QStringLiteral("%1: notes that already had text keep the project's formatting").arg(name),
              QStringLiteral("indent %1, font %2").arg(o->begin().blockFormat().textIndent()).arg(o->defaultFont().family()));
    }
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_FONT_MANAGER", "none");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderNotesPlainTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderNotesPlainTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
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

    runStorage(QStringLiteral("RTF"), StorageFormat::Rtf);
    runStorage(QStringLiteral("Markdown"), StorageFormat::Markdown);

    // ---- pasting
    {
        QTemporaryDir dir;
        BinderItem *d1 = nullptr, *d2 = nullptr;
        auto p = makeProject(dir.path(), StorageFormat::Rtf, &d1, &d2);
        TextEditor ed;
        ed.resize(400, 300);
        ed.show();
        ed.setProjectDocument(p.get(), d1, p->docs()->notes(d1->id), true);
        settle(150);
        auto *md = new QMimeData;
        md->setHtml(QStringLiteral("<p style=\"margin-left:40px; text-indent:20px\"><b>Bold</b> and <i>slanted</i> words</p>"));
        md->setText(QStringLiteral("Bold and slanted words"));
        QApplication::clipboard()->setMimeData(md);
        ed.setFocus();
        ed.paste();
        settle(100);
        QTextDocument *n = ed.document();
        bool anyBold = false;
        for (QTextBlock b = n->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().fontWeight() >= QFont::DemiBold || it.fragment().charFormat().fontItalic())
                    anyBold = true;
        check(n->toPlainText() == QLatin1String("Bold and slanted words") && !anyBold && blocksPlain(n),
              QStringLiteral("Pasting formatted text into the notes gives plain text"), n->toPlainText());
        ed.pasteKeepingFormatting();
        settle(100);
        bool bold2 = false;
        for (QTextBlock b = n->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().fontWeight() >= QFont::DemiBold)
                    bold2 = true;
        check(bold2, QStringLiteral("... Paste with Formatting keeps the formatting"), n->toPlainText());
        // the project notes
        ed.setProjectDocument(p.get(), nullptr, p->docs()->projectNotes(), true);
        QApplication::clipboard()->setMimeData([] {
            auto *m = new QMimeData;
            m->setHtml(QStringLiteral("<p><b>Strong</b> text</p>"));
            m->setText(QStringLiteral("Strong text"));
            return m;
        }());
        ed.setFocus();
        ed.paste();
        settle(100);
        bool pb = false;
        for (auto it = ed.document()->begin().begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().fontWeight() >= QFont::DemiBold)
                pb = true;
        check(!pb && ed.document()->toPlainText() == QLatin1String("Strong text"), QStringLiteral("... and into the project notes"));
        ed.pasteKeepingFormatting();
        settle(100);
        pb = false;
        for (auto it = ed.document()->begin().begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().fontWeight() >= QFont::DemiBold)
                pb = true;
        check(pb, QStringLiteral("... Paste with Formatting keeps it in the project notes too"));
        ed.grab().save(outDir + QStringLiteral("/notesplain-paste.png"));
    }
    std::printf("%d failure(s)\n", failures);
    return failures;
}
