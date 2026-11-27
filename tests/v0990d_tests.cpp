// Scripted run for 0.99.0, part d (item 118): three usability points.
//   UR-04  the keyboard is not caught in a table (Ctrl+Return and Down in the last row leave it below, Up in the first row
//          above), and a 2 px line in the theme's highlight color shows the part that has the keyboard
//   UR-09  Compile can be cancelled (Cancel and Esc stop after the current document, no file, an older file untouched, an
//          outside converter stopped, "12 of 148" shown)
//   UR-17  link colors of every built-in theme reach 4.5 : 1 on every paper and background; the note for own colors
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0990d_tests /tmp/shots
#include "compile/CompileFormat.h"
#include "compile/Compiler.h"
#include "compile/PluginManager.h"
#include "compile/Progress.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "text/DisplayFormat.h"
#include "text/Tables.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/CompileDialog.h"
#include "ui/Copyholder.h"
#include "ui/Corkboard.h"
#include "ui/EditorPane.h"
#include "ui/FocusLine.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/MessageLine.h"
#include "ui/OptionsDialog.h"
#include "ui/Outliner.h"
#include "ui/TextColumnView.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextTable>
#include <QTimer>
#include <cstdio>
#include <functional>
#ifdef Q_OS_UNIX
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#endif

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
static void info(const QString &s)
{
    std::printf("      %s\n", qPrintable(s));
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

static EditorPane *pane0(MainWindow &w) { return w.editorPane(0); }

// the text editor of the first pane showing the document
static TextEditor *showText(MainWindow &w, BinderItem *item)
{
    EditorPane *pane = pane0(w);
    pane->showItems({item}, false, int(ViewMode::Single));
    settle(300);
    TextEditor *e = pane->currentEditor();
    if (e) {
        e->setFocus();
        w.activateWindow();
    }
    return e;
}

static bool alive(qint64 pid)
{
    QFile f(QStringLiteral("/proc/%1/stat").arg(pid));
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QByteArray s = f.readAll();
    const int close = int(s.lastIndexOf(')'));
    return close >= 0 && close + 2 < s.size() && s.at(close + 2) != 'Z';
}

static QString contentOf(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromLatin1(f.readAll()) : QString();
}

// a QTextTable of `rows` x 2 at the end of the editor's text; cell (r, c) holds "rRcC"
static QTextTable *fillTable(TextEditor *e, int rows)
{
    QTextCursor c(e->document());
    c.movePosition(QTextCursor::End);
    e->setTextCursor(c);
    e->insertTableAtCursor(rows, 2, tables::preset(QStringLiteral("header")));
    QTextTable *t = e->currentTable();
    if (!t)
        return nullptr;
    for (int r = 0; r < rows; ++r)
        for (int col = 0; col < 2; ++col) {
            QTextCursor cc = t->cellAt(r, col).firstCursorPosition();
            cc.insertText(QStringLiteral("r%1c%2").arg(r).arg(col));
        }
    return t;
}

static bool outsideTable(TextEditor *e) { return !e->textCursor().currentTable(); }

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", (home.path() + QLatin1String("/h")).toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_KEYBOARD_LAYOUT", "us");
    qputenv("QB_COLOR_SCHEME", "none");
    qputenv("QB_NO_DIALOGS", "1");

    // a converter that only waits (and starts a helper of its own), for the check that Cancel stops outside programs
    QTemporaryDir plug;
    const QString pidFile = plug.filePath(QStringLiteral("pids.txt"));
    {
        QFile sh(plug.filePath(QStringLiteral("fakeconv.sh")));
        sh.open(QIODevice::WriteOnly);
        sh.write("#!/bin/sh\nsleep 60 &\necho \"$$ $!\" > \"$QB_FAKE_PIDS\"\nwait\n");
        sh.close();
        sh.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        QFile js(plug.filePath(QStringLiteral("fakeconv.json")));
        js.open(QIODevice::WriteOnly);
        QJsonObject o;
        o.insert(QStringLiteral("id"), QStringLiteral("fakeconv"));
        o.insert(QStringLiteral("name"), QStringLiteral("Fake converter"));
        o.insert(QStringLiteral("extension"), QStringLiteral("xyz"));
        o.insert(QStringLiteral("program"), plug.filePath(QStringLiteral("fakeconv.sh")));
        o.insert(QStringLiteral("input"), QStringLiteral("md"));
        o.insert(QStringLiteral("arguments"), QJsonArray{QStringLiteral("{input}"), QStringLiteral("{output}")});
        o.insert(QStringLiteral("timeout"), 120);
        js.write(QJsonDocument(o).toJson());
        js.close();
    }
    qputenv("QB_PLUGIN_PATH", plug.path().toUtf8());
    qputenv("QB_FAKE_PIDS", pidFile.toUtf8());

    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990dTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990dTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.welcomeShown = true;
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
    // documents for the table checks
    BinderItem *folder = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Tables"));
    auto makeDoc = [&](const QString &title, const QString &text) {
        BinderItem *d = p->model()->createItem(folder, -1, ItemType::Text, title);
        QTextCursor c(p->docs()->text(d->id));
        c.insertText(text);
        return d;
    };
    BinderItem *dMid = makeDoc(QStringLiteral("Middle"), QStringLiteral("Text before the table."));
    BinderItem *dEnd = makeDoc(QStringLiteral("End"), QStringLiteral("Text before the table at the end."));
    BinderItem *dStart = makeDoc(QStringLiteral("Start"), QString());
    p->save(&err);
    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.resize(1500, 900);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);

    // ============================================================ UR-04: the table keys
    {
        // a table in the middle of the text: "before", the table, "after"
        TextEditor *e = showText(w, dMid);
        check(e != nullptr, QStringLiteral("(an editor)"));
        if (!e)
            return 1;
        QTextTable *t = fillTable(e, 3);
        check(t && t->rows() == 3, QStringLiteral("(a table of 3 rows in the middle of the text)"));
        {
            QTextCursor c(e->document());
            c.setPosition(t->lastPosition() + 1);
            c.insertText(QStringLiteral("Text after the table."));
        }
        const QString before = e->document()->toPlainText();
        const int blocks = e->document()->blockCount();
        const int undoSteps = e->document()->availableUndoSteps();
        // Down in a row that is not the last stays in the table
        e->setTextCursor(t->cellAt(0, 0).firstCursorPosition());
        QTest::keyClick(e, Qt::Key_Down);
        check(e->textCursor().currentTable() == t, QStringLiteral("Down in the first row stays in the table"));
        // Down in the last row leaves it below, into the text after
        e->setTextCursor(t->cellAt(2, 1).firstCursorPosition());
        QTest::keyClick(e, Qt::Key_Down);
        check(outsideTable(e) && e->textCursor().block().text() == QLatin1String("Text after the table."),
              QStringLiteral("Down in the last row leaves the table below it"), e->textCursor().block().text());
        // Ctrl+Return from a middle row too
        e->setTextCursor(t->cellAt(1, 0).firstCursorPosition());
        QTest::keyClick(e, Qt::Key_Return, Qt::ControlModifier);
        check(outsideTable(e) && e->textCursor().block().text() == QLatin1String("Text after the table."),
              QStringLiteral("Ctrl+Return in a table leaves it below"), e->textCursor().block().text());
        // Up in the first row leaves it above, into the text before
        e->setTextCursor(t->cellAt(0, 1).firstCursorPosition());
        QTest::keyClick(e, Qt::Key_Up);
        check(outsideTable(e) && e->textCursor().block().text() == QLatin1String("Text before the table."),
              QStringLiteral("Up in the first row leaves the table above it"), e->textCursor().block().text());
        // Up in the second row only goes up a row
        e->setTextCursor(t->cellAt(1, 0).firstCursorPosition());
        QTest::keyClick(e, Qt::Key_Up);
        check(e->textCursor().currentTable() == t && t->cellAt(e->textCursor()).row() == 0,
              QStringLiteral("Up in the second row goes to the first row"));
        // the keys changed no text: the same text and paragraphs, nothing for Undo to take back
        check(e->document()->toPlainText() == before && e->document()->blockCount() == blocks && e->document()->availableUndoSteps() == undoSteps,
              QStringLiteral("the keys made no change in the text and no step for Undo"),
              QStringLiteral("%1 blocks, was %2; %3 steps, was %4").arg(e->document()->blockCount()).arg(blocks)
                  .arg(e->document()->availableUndoSteps()).arg(undoSteps));
        // a cell with two lines: Down in its first line moves inside the cell
        e->setTextCursor(t->cellAt(2, 0).lastCursorPosition());
        {
            QTextCursor c = e->textCursor();
            c.insertBlock();
            c.insertText(QStringLiteral("second line"));
        }
        QTextCursor top = t->cellAt(2, 0).firstCursorPosition();
        e->setTextCursor(top);
        QTest::keyClick(e, Qt::Key_Down);
        check(e->textCursor().currentTable() == t && e->textCursor().block().text() == QLatin1String("second line"),
              QStringLiteral("Down in a cell of several lines moves to its next line first"), e->textCursor().block().text());
        QTest::keyClick(e, Qt::Key_Down);
        check(outsideTable(e), QStringLiteral("Down in its last line (last row) then leaves the table"));
        shot(&w, QStringLiteral("table-middle"));
    }
    {
        // the table ends the text (a paragraph follows it: Qt keeps one)
        TextEditor *e = showText(w, dEnd);
        QTextTable *t = e ? fillTable(e, 2) : nullptr;
        check(t != nullptr, QStringLiteral("(a table at the end of the text)"));
        if (t) {
            const int blocks = e->document()->blockCount();
            const int undoBefore = e->document()->availableUndoSteps();
            e->setTextCursor(t->cellAt(1, 1).firstCursorPosition());
            QTest::keyClick(e, Qt::Key_Down);
            check(outsideTable(e) && e->textCursor().position() > t->lastPosition(),
                  QStringLiteral("table at the end of the text: Down in the last row leaves it, below"));
            e->setTextCursor(t->cellAt(0, 0).firstCursorPosition());
            QTest::keyClick(e, Qt::Key_Return, Qt::ControlModifier);
            check(outsideTable(e) && e->textCursor().position() > t->lastPosition(),
                  QStringLiteral("table at the end of the text: Ctrl+Return leaves it, below"));
            check(e->document()->blockCount() == blocks && e->document()->availableUndoSteps() == undoBefore,
                  QStringLiteral("a paragraph that was there needs no new one (no step for Undo)"));
            // typing goes below the table now
            QTest::keyClicks(e, QStringLiteral("Below."));
            check(e->document()->toPlainText().endsWith(QLatin1String("Below.")),
                  QStringLiteral("typing after the keys goes below the table"));
            e->undo();
            check(!e->document()->toPlainText().contains(QLatin1String("Below.")) && e->document()->blockCount() == blocks,
                  QStringLiteral("Undo takes the typed text away and keeps the table and its paragraph"));
        }
    }
    {
        // the table starts the text: a table in an empty document
        TextEditor *e = showText(w, dStart);
        QTextTable *t = e ? fillTable(e, 2) : nullptr;
        check(t != nullptr, QStringLiteral("(a table at the start of the text)"));
        if (t) {
            const int tableFirst = t->firstPosition();
            e->setTextCursor(t->cellAt(0, 1).firstCursorPosition());
            QTest::keyClick(e, Qt::Key_Up);
            check(outsideTable(e) && e->textCursor().position() < tableFirst,
                  QStringLiteral("table at the start of the text: Up in the first row leaves it, above"),
                  QString::number(e->textCursor().position()));
            QTest::keyClicks(e, QStringLiteral("Above."));
            check(e->document()->toPlainText().startsWith(QLatin1String("Above.")),
                  QStringLiteral("typing after the key goes above the table"));
            e->undo();
            check(!e->document()->toPlainText().contains(QLatin1String("Above.")) && e->document()->toPlainText().contains(QLatin1String("r1c1")),
                  QStringLiteral("Undo takes the typed text away; the table stays"));
        }
    }

    // ============================================================ UR-04: the line around the part with the keyboard
    {
        FocusLine *fl = w.focusLine();
        check(fl != nullptr && fl->objectName() == QLatin1String("focusLine"), QStringLiteral("the window has a focus line"));
        BinderPanel *binder = w.findChild<BinderPanel *>();
        EditorPane *pane = pane0(w);
        auto linesShown = [&] {
            int n = 0;
            for (FocusLine *f : w.findChildren<FocusLine *>())
                n += f->isVisible() ? 1 : 0;
            return n;
        };
        auto rectOf = [&](QWidget *part) { return QRect(part->mapTo(&w, QPoint(0, 0)), part->size()); };
        struct One {
            QString name;
            QWidget *widget;
            QWidget *focus;
        };
        // the editor
        TextEditor *e = showText(w, dMid);
        // the corkboard and the outliner of the folder
        pane->showItems({folder}, false, int(ViewMode::Corkboard));
        settle(300);
        QWidget *cork = pane->corkboard();
        pane->showItems({folder}, false, int(ViewMode::Outliner));
        settle(300);
        QWidget *outl = pane->outliner();
        // the binder's list
        QWidget *binderView = binder ? binder->findChild<QAbstractItemView *>() : nullptr;
        QTextEdit *insp = w.findChild<Inspector *>() ? w.findChild<Inspector *>()->findChild<QTextEdit *>() : nullptr;

        QStringList seen;
        auto expect = [&](QWidget *focus, const QString &name, QWidget *part, const QString &what) {
            if (!focus || !part) {
                check(false, what + QStringLiteral(" (the widgets are not there)"));
                return;
            }
            focus->setFocus();
            w.activateWindow();
            settle(250);
            const bool one = fl->part() == name && linesShown() == 1 && fl->isVisible();
            check(one, what + QStringLiteral(": the line is around the %1 and only there").arg(name),
                  QStringLiteral("part '%1', %2 lines").arg(fl->part()).arg(linesShown()));
            check(fl->lineRect() == rectOf(part), what + QStringLiteral(": the line follows the part's edge"),
                  QStringLiteral("%1,%2 %3x%4 vs %5,%6 %7x%8")
                      .arg(fl->lineRect().x()).arg(fl->lineRect().y()).arg(fl->lineRect().width()).arg(fl->lineRect().height())
                      .arg(rectOf(part).x()).arg(rectOf(part).y()).arg(rectOf(part).width()).arg(rectOf(part).height()));
            seen << fl->part();
        };
        expect(binderView, QStringLiteral("binder"), binder, QStringLiteral("binder"));
        expect(outl, QStringLiteral("outliner"), outl, QStringLiteral("outliner"));
        pane->showItems({folder}, false, int(ViewMode::Corkboard));
        settle(300);
        expect(cork, QStringLiteral("corkboard"), cork, QStringLiteral("corkboard"));
        e = showText(w, dMid);
        expect(e, QStringLiteral("editor"), pane->textView(), QStringLiteral("editor"));
        expect(insp, QStringLiteral("inspector"), w.findChild<Inspector *>(), QStringLiteral("inspector"));
        // the copyholder inside the editor
        pane->openCopyholder(dEnd);
        settle(400);
        TextEditor *holderEd = pane->copyholder() ? pane->copyholder()->findChild<TextEditor *>() : nullptr;
        expect(holderEd, QStringLiteral("copyholder"), pane->copyholder(), QStringLiteral("copyholder"));
        // the editor again, with the copyholder open: the line moves to the editor only
        expect(pane->textView()->findChild<TextEditor *>(), QStringLiteral("editor"), pane->textView(),
               QStringLiteral("editor beside a copyholder"));
        pane->closeCopyholder();
        settle(200);
        // a toolbar field: no line (the keyboard is in none of the parts)
        QLineEdit *title = pane->findChild<QLineEdit *>();
        if (title) {
            title->setFocus();
            settle(250);
            check(fl->part().isEmpty() || linesShown() <= 1, QStringLiteral("never two lines at once (title field: '%1')").arg(fl->part()));
        }
        info(QStringLiteral("parts seen: %1").arg(seen.join(QLatin1Char(','))));

        // the color in every built-in theme: the theme's highlight color, drawn 2 px wide
        QStringList bad;
        int themes = 0;
        for (const theme::Entry &en : theme::available()) {
            if (en.kind != theme::Kind::BuiltIn)
                continue;
            ++themes;
            as.theme = en.id;
            as.save();
            theme::apply();
            settle(200);
            e = showText(w, dMid);
            if (e)
                e->setFocus();
            settle(250);
            const QColor hl = theme::color(theme::Role::Highlight);
            const QImage img = w.grab().toImage();
            const QRect r = fl->lineRect();
            bool ok = fl->part() == QLatin1String("editor") && fl->lineColor() == hl && FocusLine::kWidth == 2 && !r.isEmpty();
            if (ok) {
                const int midY = r.top() + r.height() / 2, midX = r.left() + r.width() / 2;
                const QPoint probes[] = {{r.left(), midY}, {r.left() + 1, midY}, {midX, r.top()}, {midX, r.top() + 1},
                                         {r.right(), midY}, {r.right() - 1, midY}, {midX, r.bottom()}, {midX, r.bottom() - 1}};
                for (const QPoint &pt : probes)
                    ok = ok && img.pixelColor(pt).rgb() == hl.rgb();
            }
            if (!ok)
                bad << en.id;
            if (en.id == QLatin1String("light") || en.id == QLatin1String("dark") || en.id == QLatin1String("midnight"))
                shot(&w, QStringLiteral("focusline-") + en.id);
        }
        check(themes >= 8 && bad.isEmpty(), QStringLiteral("the line is 2 px in the highlight color in all %1 built-in themes").arg(themes),
              bad.join(QLatin1Char(' ')));
        as.theme = QStringLiteral("light");
        as.save();
        theme::apply();
        settle(200);
    }

    // ============================================================ UR-17: link colors
    {
        QStringList bad;
        int themes = 0, pairs = 0;
        double worst = 99;
        QString worstWhere;
        for (const theme::Entry &en : theme::available()) {
            if (en.kind != theme::Kind::BuiltIn)
                continue;
            theme::Theme t;
            if (!theme::load(en.id, &t))
                continue;
            ++themes;
            for (int variant = 0; variant < 2; ++variant) {
                // 1: with the program's standard own colors (the olive paper), as a new installation shows
                const QMap<theme::Role, QColor> c = theme::resolvedColors(
                    t, variant ? AppSettings::standardOwnColors() : QMap<QString, QColor>());
                for (const theme::ContrastPair &pr : theme::linkPairs()) {
                    ++pairs;
                    const double r = theme::contrastRatio(c.value(pr.fg), c.value(pr.bg));
                    if (r < worst) {
                        worst = r;
                        worstWhere = QStringLiteral("%1/%2: %3 on %4").arg(en.id).arg(variant).arg(theme::roleLabel(pr.fg), theme::roleLabel(pr.bg));
                    }
                    if (r < 4.5)
                        bad << QStringLiteral("%1/%2: %3 on %4 = %5").arg(en.id).arg(variant).arg(theme::roleLabel(pr.fg),
                                                                                                    theme::roleLabel(pr.bg)).arg(r, 0, 'f', 2);
                }
            }
            check(true, QStringLiteral("(theme %1 looked at)").arg(en.name));
        }
        // the text link on every paper of the text areas (each text area gets the link color that fits its paper):
        // theme x paper x link color, all of them listed in link-contrast.txt
        QStringList listing;
        for (const theme::Entry &en : theme::available()) {
            if (en.kind != theme::Kind::BuiltIn)
                continue;
            theme::Theme t;
            if (!theme::load(en.id, &t))
                continue;
            for (int variant = 0; variant < 2; ++variant) {
                const QMap<theme::Role, QColor> c = theme::resolvedColors(
                    t, variant ? AppSettings::standardOwnColors() : QMap<QString, QColor>());
                QList<theme::ContrastPair> all = theme::linkPairs();
                all << theme::paperLinkPairs();
                for (const theme::ContrastPair &pr : all) {
                    QColor link = c.value(pr.fg);
                    // the text link is worked out for the paper it is shown on; the interface link is one color
                    if (pr.fg == theme::Role::EditorLink)
                        link = theme::linkOn(link, c.value(pr.bg), false);
                    const double r = theme::contrastRatio(link, c.value(pr.bg));
                    ++pairs;
                    listing << QStringLiteral("%1 %2 | %3 %4 on %5 %6 | %7")
                                   .arg(en.id).arg(variant ? QStringLiteral("(own standard colors)") : QStringLiteral("(theme only)"),
                                        theme::roleLabel(pr.fg), link.name(), theme::roleLabel(pr.bg), c.value(pr.bg).name())
                                   .arg(r, 0, 'f', 2);
                    if (r < 4.5)
                        bad << QStringLiteral("%1/%2: %3 on %4 = %5").arg(en.id).arg(variant).arg(theme::roleLabel(pr.fg),
                                                                                                    theme::roleLabel(pr.bg)).arg(r, 0, 'f', 2);
                }
            }
        }
        {
            QFile lf(outDir + QStringLiteral("/link-contrast.txt"));
            if (lf.open(QIODevice::WriteOnly))
                lf.write(listing.join(QLatin1Char('\n')).toUtf8());
        }
        info(QStringLiteral("%1 pairs in %2 themes, the lowest: %3 (%4)").arg(pairs).arg(themes).arg(worst, 0, 'f', 2).arg(worstWhere));
        check(themes >= 8 && bad.isEmpty(),
              QStringLiteral("every built-in theme: links at least 4.5 : 1 on every paper and background (%1 themes)").arg(themes),
              bad.join(QLatin1String("; ")));
        // the highlighter: a link in the notes (a dark paper) beside the editor's light paper, in the theme "Shelin's Dark Theme"
        {
            // the areas take the project's look: its theme is the one that changes the papers
            const theme::AreaLook savedLook = theme::areaLook();
            theme::AreaLook look = savedLook;
            look.set = true;
            look.theme = QStringLiteral("shelins-dark");
            look.colors.clear();
            theme::setAreaLook(look);
            theme::apply();
            settle(400);
            Inspector *insp = w.findChild<Inspector *>();
            TextEditor *notes = nullptr;
            for (TextEditor *te : insp ? insp->findChildren<TextEditor *>() : QList<TextEditor *>())
                if (te->palette().color(QPalette::Base).rgb() == theme::color(theme::Role::NotesPaper).rgb())
                    notes = te;
            TextEditor *main = pane0(w)->currentEditor();
            auto linkShown = [](TextEditor *te, QColor *shownColor) {
                QTextDocument *d = te->document();
                QTextCursor c(d);
                c.movePosition(QTextCursor::End);
                QTextCharFormat lf;
                lf.setAnchor(true);
                lf.setAnchorHref(QStringLiteral("https://example.org"));
                lf.setForeground(QColor(30, 90, 200));
                c.insertBlock();
                c.insertText(QStringLiteral("a link"), lf);
                d->setModified(false);
                QCoreApplication::processEvents();
                QTest::qWait(100);
                const QTextBlock b = d->lastBlock();
                for (const QTextLayout::FormatRange &fr : b.layout()->formats())
                    if (fr.format.hasProperty(QTextFormat::ForegroundBrush)) {
                        *shownColor = fr.format.foreground().color();
                        return true;
                    }
                return false;
            };
            QColor onNotes, onPaper;
            const bool gotNotes = notes && linkShown(notes, &onNotes);
            const bool gotMain = main && linkShown(main, &onPaper);
            check(theme::color(theme::Role::NotesPaper).lightness() < 80 && theme::color(theme::Role::EditorPaper).lightness() > 180,
                  QStringLiteral("(the theme shows a dark notes paper beside a light editor paper)"));
            check(gotNotes && theme::contrastRatio(onNotes, theme::color(theme::Role::NotesPaper)) >= 4.5,
                  QStringLiteral("a link in the notes shows with 4.5 : 1 on the notes' dark paper"),
                  QStringLiteral("%1 on %2").arg(onNotes.name(), theme::color(theme::Role::NotesPaper).name()));
            check(gotMain && theme::contrastRatio(onPaper, theme::color(theme::Role::EditorPaper)) >= 4.5,
                  QStringLiteral("a link in the editor shows with 4.5 : 1 on the editor's light paper"),
                  QStringLiteral("%1 on %2").arg(onPaper.name(), theme::color(theme::Role::EditorPaper).name()));
            check(gotNotes && gotMain && onNotes.rgb() != onPaper.rgb(),
                  QStringLiteral("each text area gets the link color of its own paper (not one for all texts)"),
                  QStringLiteral("notes %1, editor %2 (got %3 %4)").arg(onNotes.name(), onPaper.name()).arg(gotNotes).arg(gotMain));
            theme::setAreaLook(savedLook);
            theme::apply();
            settle(200);
        }
        // the olive paper of the standard look, in particular
        theme::Theme light;
        theme::load(QStringLiteral("light"), &light);
        const QMap<theme::Role, QColor> olive = theme::resolvedColors(light, AppSettings::standardOwnColors());
        check(olive.value(theme::Role::EditorPaper).rgb() == QColor(0xd1, 0xd4, 0x95).rgb()
                  && theme::contrastRatio(olive.value(theme::Role::EditorLink), olive.value(theme::Role::EditorPaper)) >= 4.5,
              QStringLiteral("the link on the standard olive paper reaches 4.5 : 1 (it was 4.05 : 1)"));
        // the writer's own link color is kept as chosen
        QMap<QString, QColor> mine = AppSettings::standardOwnColors();
        mine.insert(QStringLiteral("editorLink"), QColor(0x70, 0x90, 0xd0));
        const QMap<theme::Role, QColor> own = theme::resolvedColors(light, mine);
        check(own.value(theme::Role::EditorLink).rgb() == QColor(0x70, 0x90, 0xd0).rgb(), QStringLiteral("an own link color is not changed"));

        // the note in Appearance
        OptionsDialog dlg(&w);
        dlg.showSection(OptionsDialog::SAppearance);
        dlg.show();
        settle(400);
        QLabel *note = dlg.findChild<QLabel *>(QStringLiteral("contrastNote"));
        check(note && note->isHidden(), QStringLiteral("Appearance: no note while the colors are the theme's"));
        dlg.selectTheme(QStringLiteral("light"));
        dlg.setOwnColor(QStringLiteral("editorPaper"), QColor(0xff, 0xff, 0xff));
        dlg.setOwnColor(QStringLiteral("editorLink"), QColor(0xb0, 0xb8, 0xd0));
        settle(200);
        QMap<QString, QColor> ownNow;
        ownNow.insert(QStringLiteral("editorPaper"), QColor(0xff, 0xff, 0xff));
        ownNow.insert(QStringLiteral("editorLink"), QColor(0xb0, 0xb8, 0xd0));
        theme::Theme lightT;
        theme::load(QStringLiteral("light"), &lightT);
        const QMap<theme::Role, QColor> shownNow = theme::resolvedColors(lightT, ownNow);
        const QList<theme::ContrastIssue> issues = theme::contrastIssues(shownNow, {theme::Role::EditorPaper, theme::Role::EditorLink});
        const double ratio = issues.isEmpty() ? 0 : issues.first().ratio;
        check(note && !note->isHidden() && !issues.isEmpty() && note->text().contains(theme::roleLabel(issues.first().fg))
                  && note->text().contains(theme::roleLabel(issues.first().bg)) && note->text().contains(QLocale().toString(ratio, 'f', 2)),
              QStringLiteral("own colors below 4.5 : 1: the note names the worst pair and its ratio (%1 : 1)").arg(ratio, 0, 'f', 2),
              note ? note->text() : QString());
        bool onEditorPaper = false, onCards = false, onNotesPaper = false, onComposition = false, onSynopsis = false;
        for (const theme::ContrastIssue &is : issues) {
            onEditorPaper |= is.bg == theme::Role::EditorPaper;
            onCards |= is.bg == theme::Role::CardPaper;
            onNotesPaper |= is.bg == theme::Role::NotesPaper;
            onComposition |= is.bg == theme::Role::CompositionPaper;
            onSynopsis |= is.bg == theme::Role::SynopsisPaper;
        }
        check(onEditorPaper && onSynopsis && onCards && onNotesPaper && !onComposition,
              QStringLiteral("the note's pairs include the editor, synopsis, card and notes papers (a light link is fine on the dark composition paper)"), QString::number(issues.size()));
        shot(&dlg, QStringLiteral("contrast-note"));
        QDialogButtonBox *bb = dlg.findChild<QDialogButtonBox *>();
        check(bb && bb->button(QDialogButtonBox::Ok) && bb->button(QDialogButtonBox::Ok)->isEnabled(), QStringLiteral("the note blocks nothing (OK stays on)"));
        dlg.setOwnColor(QStringLiteral("editorLink"), QColor(0x10, 0x20, 0x90));
        settle(200);
        check(note && !note->text().contains(QStringLiteral("Links in the text (Main Editor) on Paper (Main Editor)")),
              QStringLiteral("a link color that reaches 4.5 : 1 on the editor paper is no longer named for it"), note ? note->text() : QString());
        dlg.setOwnColor(QStringLiteral("editorLink"), QColor());
        dlg.setOwnColor(QStringLiteral("editorPaper"), QColor());
        settle(200);
        check(note && note->isHidden(), QStringLiteral("back to the theme's colors: no note"), note ? note->text() : QString());
        dlg.close();
    }

    // ============================================================ UR-09: Compile can be cancelled
    {
        // a large project: 80 documents of some text
        BinderItem *big = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Big"));
        for (int i = 0; i < 80; ++i) {
            BinderItem *d = p->model()->createItem(big, -1, ItemType::Text, QStringLiteral("Part %1").arg(i + 1));
            QTextCursor c(p->docs()->text(d->id));
            c.insertText(QStringLiteral("Paragraph of part %1. The lamp burned all night and the sea was loud.").arg(i + 1));
        }
        QTemporaryDir outTmp;
        const QString outDocx = outTmp.filePath(QStringLiteral("book.docx"));
        const QString outXyz = outTmp.filePath(QStringLiteral("book.xyz"));
        CompileDialog dlg(p, &w);
        auto *exporter = dlg.findChild<QComboBox *>(QStringLiteral("compileExporter"));
        check(exporter && exporter->findData(QStringLiteral("docx")) >= 0 && exporter->findData(QStringLiteral("fakeconv")) >= 0,
              QStringLiteral("(the Word format and the test converter are offered)"));
        auto filesIn = [&] { return QDir(outTmp.path()).entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot); };

        // a compile that is not cancelled: written, "n of N" shown up to the end, no temporary file left
        exporter->setCurrentIndex(exporter->findData(QStringLiteral("docx")));
        {
            QFile f(outDocx);
            f.open(QIODevice::WriteOnly);
            f.write("OLD");
        }
        QString lastLabel;
        QTimer watch;
        watch.setInterval(1);
        QObject::connect(&watch, &QTimer::timeout, [&] {
            for (QWidget *tw : QApplication::topLevelWidgets())
                if (auto *pd = qobject_cast<QProgressDialog *>(tw); pd && pd->objectName() == QLatin1String("compileProgress") && pd->isVisible())
                    lastLabel = pd->labelText();
        });
        watch.start();
        const CompileDialog::CompileResult full = dlg.compileTo(outDocx, &err);
        watch.stop();
        check(full == CompileDialog::Compiled && QFileInfo(outDocx).size() > 1000 && contentOf(outDocx) != QLatin1String("OLD")
                  && filesIn() == QStringList{QStringLiteral("book.docx")},
              QStringLiteral("a compile that runs to the end replaces the file and leaves no temporary file"), filesIn().join(QLatin1Char(',')) + err);
        const QRegularExpressionMatch endMatch = QRegularExpression(QStringLiteral("\n(\\d+) of (\\d+)$")).match(lastLabel);
        check(endMatch.hasMatch() && endMatch.captured(2).toInt() >= 80,
              QStringLiteral("the window shows \"n of N\" while it works"), lastLabel);
        {
            // the count reaches the total at the end of the documents
            int done = -1, total = -1;
            compile::progress::Scope scope([&](double) { done = compile::progress::documentsDone(); total = compile::progress::documentsTotal(); });
            compile::AuthorInfo author{QStringLiteral("A"), QStringLiteral("B")};
            compile::Compiler c(p, compile::builtinFormats().first(), p->compile, author);
            const compile::Manuscript m = c.build();
            check(done == total && total >= 80 && !m.paragraphs.isEmpty(), QStringLiteral("the documents done reach their total (%1 of %2)").arg(done).arg(total));
        }

        // Cancel after a few documents (the button)
        for (int how = 0; how < 2; ++how) {
            const QString which = how == 0 ? QStringLiteral("the Cancel button") : QStringLiteral("Esc");
            {
                QFile f(outDocx);
                f.open(QIODevice::WriteOnly);
                f.write("OLD");
            }
            QString shownBefore;
            int doneAtCancel = -1, totalAtCancel = -1;
            bool cancelled = false;
            QTimer poll;
            poll.setInterval(1);
            QObject::connect(&poll, &QTimer::timeout, [&] {
                if (cancelled)
                    return;
                for (QWidget *tw : QApplication::topLevelWidgets()) {
                    auto *pd = qobject_cast<QProgressDialog *>(tw);
                    if (!pd || pd->objectName() != QLatin1String("compileProgress") || !pd->isVisible())
                        continue;
                    const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("\n(\\d+) of (\\d+)$")).match(pd->labelText());
                    if (!m.hasMatch() || m.captured(1).toInt() < 5)
                        continue;
                    shownBefore = pd->labelText();
                    doneAtCancel = m.captured(1).toInt();
                    totalAtCancel = m.captured(2).toInt();
                    cancelled = true;
                    if (how == 0) {
                        if (QPushButton *b = pd->findChild<QPushButton *>(QStringLiteral("compileCancel")))
                            b->click();
                    } else {
                        QTest::keyClick(pd, Qt::Key_Escape);
                    }
                }
            });
            poll.start();
            const CompileDialog::CompileResult res = dlg.compileTo(outDocx, &err);
            poll.stop();
            check(cancelled && res == CompileDialog::Cancelled, which + QStringLiteral(" in the progress window cancels the compile"),
                  QStringLiteral("cancelled %1, result %2").arg(cancelled).arg(int(res)));
            check(doneAtCancel >= 5 && totalAtCancel >= 80 && doneAtCancel < totalAtCancel,
                  which + QStringLiteral(": the window showed \"%1 of %2\" while it worked").arg(doneAtCancel).arg(totalAtCancel), shownBefore);
            check(contentOf(outDocx) == QLatin1String("OLD"), which + QStringLiteral(": the older file of that name is untouched"));
            check(filesIn() == QStringList{QStringLiteral("book.docx")}, which + QStringLiteral(": no temporary or half-written file is left"),
                  filesIn().join(QLatin1Char(',')));
            MessageLine *ml = w.messageLine();
            check(ml && ml->shownText().contains(QStringLiteral("cancelled"), Qt::CaseInsensitive),
                  which + QStringLiteral(": the message line says the compile was cancelled"), ml ? ml->shownText() : QString());
            if (ml)
                ml->clearAll();
            check(!compile::progress::cancelled(), which + QStringLiteral(": the next compile starts without the flag"));
        }

        // an outside converter: stopped by Cancel (the program and what it started)
#ifdef Q_OS_UNIX
        exporter->setCurrentIndex(exporter->findData(QStringLiteral("fakeconv")));
        {
            QFile f(outXyz);
            f.open(QIODevice::WriteOnly);
            f.write("OLD");
        }
        QFile::remove(pidFile);
        bool clicked = false;
        QTimer poll;
        poll.setInterval(20);
        QObject::connect(&poll, &QTimer::timeout, [&] {
            if (clicked || !QFile::exists(pidFile) || contentOf(pidFile).trimmed().split(QLatin1Char(' ')).size() < 2)
                return;
            for (QWidget *tw : QApplication::topLevelWidgets())
                if (auto *pd = qobject_cast<QProgressDialog *>(tw); pd && pd->objectName() == QLatin1String("compileProgress") && pd->isVisible())
                    if (QPushButton *b = pd->findChild<QPushButton *>(QStringLiteral("compileCancel"))) {
                        clicked = true;
                        b->click();
                    }
        });
        poll.start();
        QElapsedTimer took;
        took.start();
        const CompileDialog::CompileResult res = dlg.compileTo(outXyz, &err);
        poll.stop();
        const QStringList pids = contentOf(pidFile).trimmed().split(QLatin1Char(' '));
        const qint64 shell = pids.value(0).toLongLong(), helper = pids.value(1).toLongLong();
        QTest::qWait(300);
        check(clicked && res == CompileDialog::Cancelled && took.elapsed() < 20000,
              QStringLiteral("Cancel while the converter works ends the compile at once (%1 ms)").arg(took.elapsed()),
              QStringLiteral("clicked %1 result %2 error '%3' pids '%4'").arg(clicked).arg(int(res)).arg(err, contentOf(pidFile)));
        check(shell > 0 && helper > 0 && !alive(shell) && !alive(helper),
              QStringLiteral("the converter and the program it started are stopped"), QStringLiteral("pids %1 %2").arg(shell).arg(helper));
        check(contentOf(outXyz) == QLatin1String("OLD") && filesIn().size() == 2, QStringLiteral("converter: the older file is untouched and no file is left beside it"),
              filesIn().join(QLatin1Char(',')));
        if (shell > 0 && alive(shell))
            ::kill(pid_t(shell), SIGKILL);
        if (helper > 0 && alive(helper))
            ::kill(pid_t(helper), SIGKILL);
#endif
        // the same without Cancel for the built-in exporter once more: the old file is replaced (the flag does not stay)
        exporter->setCurrentIndex(exporter->findData(QStringLiteral("docx")));
        const CompileDialog::CompileResult again = dlg.compileTo(outDocx, &err);
        check(again == CompileDialog::Compiled && QFileInfo(outDocx).size() > 1000, QStringLiteral("a compile after a cancelled one works"), err);
    }

    delete wp;
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
