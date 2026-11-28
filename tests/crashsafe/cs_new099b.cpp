// The system test, 0.99.0 (item 124): the usability cases U-21 to U-26: toolbar sizes, Compile's Cancel, recording to text in
// three steps, the focus line, the contrast of links in every theme, the walk through every menu.
#include "cs.h"

#include "compile/Compiler.h"
#include "compile/PluginManager.h"
#include "compile/Progress.h"
#include "core/Project.h"
#include "tools/ToolApi.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/CompileDialog.h"
#include "ui/EditorPane.h"
#include "ui/FocusLine.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/MessageLine.h"
#include "ui/TextColumnView.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"
#include "ui/ToolHost.h"
#include "ui/ToolbarConfig.h"

#include <QAbstractItemView>
#include <QTextEdit>
#include <QComboBox>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QRegularExpression>
#include <QTest>
#include <QToolBar>
#include <QToolButton>

namespace cs {

using namespace qb;

// ================================================================ U-21 toolbar sizes
void caseU21()
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1500, 900);
    if (!w || !w->project())
        return;
    AppSettings &as = AppSettings::instance();
    struct One {
        const char *name;
        int side;
    };
    for (const One &o : {One{"toolbarSizeSmall", toolbars::kSmall}, One{"toolbarSizeLarge", toolbars::kLarge}, One{"toolbarSizeNormal", toolbars::kNormal}}) {
        QAction *a = action(w, QString::fromLatin1(o.name));
        check(a != nullptr, QStringLiteral("View > Toolbars > Toolbar Size has %1").arg(QString::fromLatin1(o.name)));
        if (!a)
            continue;
        a->trigger();
        settle(500);
        QStringList wrong;
        int bars = 0;
        for (QToolBar *bar : w->findChildren<QToolBar *>()) {
            if (bar->objectName().isEmpty() || bar == w->smallBar())
                continue;
            ++bars;
            if (bar->iconSize() != QSize(o.side, o.side))
                wrong << QStringLiteral("%1 %2").arg(bar->objectName()).arg(bar->iconSize().width());
            for (QToolButton *b : bar->findChildren<QToolButton *>())
                if (b->isVisible() && b->objectName() != QLatin1String("qt_toolbar_ext_button") && b->iconSize() != QSize(o.side, o.side) && !b->icon().isNull())
                    wrong << QStringLiteral("%1/%2 %3").arg(bar->objectName(), b->objectName()).arg(b->iconSize().width());
        }
        check(bars >= 6 && wrong.isEmpty(), QStringLiteral("size %1 px: every bar and every button on it has it (%2 bars)").arg(o.side).arg(bars), wrong.mid(0, 6).join(QLatin1String("; ")));
        check(as.toolbarIconSize == o.side, QStringLiteral("... and the program keeps it (%1)").arg(as.toolbarIconSize));
        shot(w, QStringLiteral("u21-%1").arg(o.side));
    }
    finishProject(w);
}

// ================================================================ U-22 Compile's Cancel
void caseU22()
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1300, 850);
    if (!w || !w->project())
        return;
    Project *p = w->project();
    BinderItem *big = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Big"));
    for (int i = 0; i < 80; ++i) {
        BinderItem *d = p->model()->createItem(big, -1, ItemType::Text, QStringLiteral("Part %1").arg(i + 1));
        QTextCursor(p->docs()->text(d->id)).insertText(QStringLiteral("Paragraph of part %1. The lamp burned all night and the sea was loud.").arg(i + 1));
    }
    const QString dir = ctx().work + QStringLiteral("/u22-out");
    QDir().mkpath(dir);
    const QString docx = dir + QStringLiteral("/book.docx");
    CompileDialog dlg(p, w);
    auto *exporter = dlg.findChild<QComboBox *>(QStringLiteral("compileExporter"));
    check(exporter && exporter->findData(QStringLiteral("docx")) >= 0, QStringLiteral("(the Word format is offered)"));
    if (!exporter)
        return;
    exporter->setCurrentIndex(exporter->findData(QStringLiteral("docx")));
    auto filesIn = [&] { return QDir(dir).entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot); };
    QString err;
    for (int how = 0; how < 2; ++how) {
        const QString which = how == 0 ? QStringLiteral("the Cancel button") : QStringLiteral("Esc");
        writeText(docx, "OLD");
        bool cancelled = false;
        int doneAt = -1, totalAt = -1;
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
                doneAt = m.captured(1).toInt();
                totalAt = m.captured(2).toInt();
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
        const CompileDialog::CompileResult res = dlg.compileTo(docx, &err);
        poll.stop();
        check(cancelled && res == CompileDialog::Cancelled, which + QStringLiteral(" cancels the compile"), QStringLiteral("cancelled %1, result %2").arg(cancelled).arg(int(res)));
        check(doneAt >= 5 && totalAt >= 80 && doneAt < totalAt, which + QStringLiteral(": the window showed \"%1 of %2\"").arg(doneAt).arg(totalAt));
        check(readText(docx) == QLatin1String("OLD"), which + QStringLiteral(": the older file is untouched"));
        check(filesIn() == QStringList{QStringLiteral("book.docx")}, which + QStringLiteral(": no part file is left"), filesIn().join(QLatin1Char(',')));
        MessageLine *ml = w->messageLine();
        check(ml && ml->shownText().contains(QStringLiteral("cancelled"), Qt::CaseInsensitive), which + QStringLiteral(": the message line says so"), ml ? ml->shownText() : QString());
        if (ml)
            ml->clearAll();
    }
    const CompileDialog::CompileResult full = dlg.compileTo(docx, &err);
    check(full == CompileDialog::Compiled && QFileInfo(docx).size() > 1000 && filesIn() == QStringList{QStringLiteral("book.docx")},
          QStringLiteral("a compile after a cancelled one writes the file whole"), err);
    finishProject(w);
}

// ================================================================ U-23 recording to text in three steps
void caseU23()
{
    QObject *tool = speechToolObject();
    check(tool != nullptr, QStringLiteral("(the speech tool is loaded from the build's tool plugins)"), compile::PluginManager::instance().messages().join(QLatin1String("; ")));
    if (!tool)
        return;
    auto set = [](const QString &k, const QVariant &v) { toolhost::host().setSetting(QStringLiteral("speech-transcription"), k, v); };
    const QString model = ctx().work + QStringLiteral("/ggml-test.bin");
    writeText(model, QByteArray(20000, 'x'));
    set(QStringLiteral("modelFile"), model);
    set(QStringLiteral("model"), QStringLiteral("small"));
    set(QStringLiteral("mode"), QStringLiteral("live"));
    const QString wav = ctx().buildDir + QStringLiteral("/tool-plugins/speech/samples/jfk.wav");
    check(QFile::exists(wav), QStringLiteral("(the test recording is there)"), wav);
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1500, 900);
    if (!w || !w->project())
        return;
    TextEditor *ed = showDoc(w, textItems(w->project()).value(3));
    toolhost::host().runCommand(QStringLiteral("speech-transcription.show"));
    settle(300);
    auto *panel = w->findChild<QWidget *>(QStringLiteral("speechPanel"));
    check(panel != nullptr, QStringLiteral("the Transcriptions tab is there"));
    step(QStringLiteral("step 1: record"));
    bool started = false;
    QMetaObject::invokeMethod(tool, "startRecording", Qt::DirectConnection, Q_RETURN_ARG(bool, started), Q_ARG(QString, wav));
    waitFor([&] {
        bool busy = true;
        QMetaObject::invokeMethod(tool, "isBusy", Qt::DirectConnection, Q_RETURN_ARG(bool, busy));
        return started && !busy;
    }, 60000);
    settle(300);
    QString file;
    QMetaObject::invokeMethod(tool, "recordingFile", Qt::DirectConnection, Q_RETURN_ARG(QString, file));
    const QString text = readText(file);
    QString lastError;
    QMetaObject::invokeMethod(tool, "lastError", Qt::DirectConnection, Q_RETURN_ARG(QString, lastError));
    check(started && text.contains(QLatin1String("quick brown fox")), QStringLiteral("the recording is written as text (the fake engine's words)"),
          QStringLiteral("started %1, file '%2', text '%3', error '%4'").arg(started).arg(file, text.left(100), lastError));
    step(QStringLiteral("step 2: the text is in the Transcriptions tab"));
    QMetaObject::invokeMethod(panel, "setText", Qt::DirectConnection, Q_ARG(QString, text));
    settle(200);
    auto *edit = panel ? panel->findChild<QPlainTextEdit *>(QStringLiteral("speechText")) : nullptr;
    check(edit && edit->toPlainText().contains(QLatin1String("quick brown fox")), QStringLiteral("the tab shows the text"));
    shot(w, QStringLiteral("u23-tab"));
    step(QStringLiteral("step 3: Insert at Cursor"));
    auto *insert = panel ? panel->findChild<QToolButton *>(QStringLiteral("speechInsert")) : nullptr;
    check(insert != nullptr, QStringLiteral("the button Insert at Cursor is there"));
    if (insert && ed) {
        ed->setFocus();
        QTextCursor c = ed->textCursor();
        c.movePosition(QTextCursor::End);
        ed->setTextCursor(c);
        const QString before = ed->toPlainText();
        insert->click();
        settle(200);
        check(ed->toPlainText() != before && ed->toPlainText().contains(QLatin1String("quick brown fox")), QStringLiteral("the text stands in the scene"));
        shot(w, QStringLiteral("u23-inserted"));
        ed->undo();
        check(ed->toPlainText() == before, QStringLiteral("... as one undo step"));
        insert->click();
        settle(200);
    }
    finishProject(w);
}

// ================================================================ U-24 the focus line
void caseU24()
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1500, 900);
    if (!w || !w->project())
        return;
    FocusLine *fl = w->focusLine();
    check(fl != nullptr && fl->objectName() == QLatin1String("focusLine"), QStringLiteral("the window has a focus line"));
    if (!fl)
        return;
    BinderPanel *binder = w->findChild<BinderPanel *>();
    Inspector *insp = w->findChild<Inspector *>();
    QWidget *binderView = binder ? binder->findChild<QAbstractItemView *>() : nullptr;
    QTextEdit *inspEdit = insp ? insp->findChild<QTextEdit *>() : nullptr;
    TextEditor *e = showDoc(w, textItems(w->project()).value(3));
    auto shown = [&] {
        int n = 0;
        for (FocusLine *f : w->findChildren<FocusLine *>())
            n += f->isVisible() ? 1 : 0;
        return n;
    };
    auto expect = [&](QWidget *focus, const QString &name, QWidget *part) {
        if (!focus || !part) {
            check(false, name + QStringLiteral(": the widgets are there"));
            return;
        }
        focus->setFocus();
        w->activateWindow();
        settle(300);
        check(fl->part() == name && shown() == 1, QStringLiteral("%1: the line is around it and only there").arg(name), QStringLiteral("part '%1', %2 lines").arg(fl->part()).arg(shown()));
        check(fl->lineRect() == QRect(part->mapTo(w, QPoint(0, 0)), part->size()), QStringLiteral("%1: it follows the part's edge").arg(name));
        shot(w, QStringLiteral("u24-%1").arg(name));
    };
    expect(binderView, QStringLiteral("binder"), binder);
    expect(e, QStringLiteral("editor"), visiblePane(w)->textView());
    expect(inspEdit, QStringLiteral("inspector"), insp);
    // the line changes with the keyboard: Tab leaves the part
    e->setFocus();
    settle(200);
    const QString first = fl->part();
    QTest::keyClick(e, Qt::Key_Tab, Qt::ControlModifier);
    settle(200);
    check(!first.isEmpty() && shown() <= 1, QStringLiteral("never two lines at once while the keyboard moves"));
    // the colors: the theme's highlight, 2 px
    int themes = 0;
    QStringList bad;
    AppSettings &as = AppSettings::instance();
    for (const theme::Entry &en : theme::available()) {
        if (en.kind != theme::Kind::BuiltIn)
            continue;
        ++themes;
        as.theme = en.id;
        as.save();
        theme::apply();
        settle(200);
        e = showDoc(w, textItems(w->project()).value(3));
        e->setFocus();
        settle(250);
        const QColor hl = theme::color(theme::Role::Highlight);
        const QImage img = w->grab().toImage();
        const QRect r = fl->lineRect();
        bool ok = fl->part() == QLatin1String("editor") && fl->lineColor() == hl && !r.isEmpty();
        if (ok)
            ok = img.pixelColor(QPoint(r.left(), r.top() + r.height() / 2)).rgb() == hl.rgb() && img.pixelColor(QPoint(r.left() + r.width() / 2, r.top())).rgb() == hl.rgb();
        if (!ok)
            bad << en.id;
    }
    check(themes >= 8 && bad.isEmpty(), QStringLiteral("the line is in the highlight color in all %1 built-in themes").arg(themes), bad.join(QLatin1Char(' ')));
    as.theme = QStringLiteral("light");
    as.save();
    theme::apply();
    settle(200);
    finishProject(w);
}

// ================================================================ U-25 link contrast in every theme
void caseU25()
{
    QStringList bad;
    int themes = 0, pairs = 0;
    double worst = 99;
    QString listing;
    for (const theme::Entry &en : theme::available()) {
        if (en.kind != theme::Kind::BuiltIn)
            continue;
        theme::Theme t;
        if (!theme::load(en.id, &t))
            continue;
        ++themes;
        for (int variant = 0; variant < 2; ++variant) {
            const QMap<theme::Role, QColor> c = theme::resolvedColors(t, variant ? AppSettings::standardOwnColors() : QMap<QString, QColor>());
            QList<theme::ContrastPair> all = theme::linkPairs();
            all << theme::paperLinkPairs();
            for (const theme::ContrastPair &pr : all) {
                QColor link = c.value(pr.fg);
                if (pr.fg == theme::Role::EditorLink)
                    link = theme::linkOn(link, c.value(pr.bg), false);
                const double r = theme::contrastRatio(link, c.value(pr.bg));
                ++pairs;
                worst = qMin(worst, r);
                listing += QStringLiteral("%1 %2 | %3 on %4 | %5\n").arg(en.id).arg(variant).arg(theme::roleLabel(pr.fg), theme::roleLabel(pr.bg)).arg(r, 0, 'f', 2);
                if (r < 4.5)
                    bad << QStringLiteral("%1/%2: %3 on %4 = %5").arg(en.id).arg(variant).arg(theme::roleLabel(pr.fg), theme::roleLabel(pr.bg)).arg(r, 0, 'f', 2);
            }
        }
    }
    writeText(ctx().dir + QStringLiteral("/link-contrast.txt"), listing.toUtf8());
    check(themes >= 8 && pairs > 50, QStringLiteral("(%1 built-in themes, %2 pairs of link and background)").arg(themes).arg(pairs));
    check(bad.isEmpty(), QStringLiteral("every link has a contrast of at least 4.5 : 1 (worst %1)").arg(worst, 0, 'f', 2), bad.mid(0, 6).join(QLatin1String("; ")));
    fact(QStringLiteral("U-25 worst link contrast"), worst);
    // and looked at: a link in the text of the window in three themes
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1300, 850);
    if (!w || !w->project())
        return;
    AppSettings &as = AppSettings::instance();
    TextEditor *e = showDoc(w, textItems(w->project()).value(3));
    if (e) {
        QTextCursor c(e->document());
        c.movePosition(QTextCursor::End);
        QTextCharFormat f;
        f.setAnchor(true);
        f.setAnchorHref(QStringLiteral("https://example.org/"));
        c.insertText(QStringLiteral(" a link in the text"), f);
    }
    const auto undoLink = [&] {
        if (e)
            e->undo();
    };
    for (const QString &th : {QStringLiteral("light"), QStringLiteral("dark"), QStringLiteral("midnight")}) {
        as.theme = th;
        as.save();
        theme::apply();
        shot(w, QStringLiteral("u25-%1").arg(th));
    }
    as.theme = QStringLiteral("light");
    as.save();
    theme::apply();
    undoLink();
    finishProject(w);
}

// ================================================================ U-26 the walk through every menu
namespace {
QString plainOf(QString t)
{
    t.remove(QLatin1Char('&'));
    return t.trimmed();
}
void walk(QMenu *m, int depth, int *deepest, QStringList *dups, QStringList *paths, const QString &path)
{
    emit m->aboutToShow();
    settle(15);
    *deepest = qMax(*deepest, depth);
    QMap<QString, int> names;
    for (QAction *a : m->actions()) {
        if (a->isSeparator() || a->text().isEmpty())
            continue;
        const QString n = plainOf(a->text());
        names[n]++;
        const QString here = path + QStringLiteral(" > ") + n;
        paths->append(here);
        const QString on = a->menu() ? a->menu()->objectName() : QString();
        // lists of the writer's own documents and files are data, not menus of commands
        if (a->menu() && on != QLatin1String("goToMenu") && on != QLatin1String("openQuickRefMenu") && on != QLatin1String("historyBackMenu") &&
            on != QLatin1String("historyForwardMenu"))
            walk(a->menu(), depth + 1, deepest, dups, paths, here);
    }
    for (auto it = names.cbegin(); it != names.cend(); ++it)
        if (it.value() > 1 && !path.contains(QLatin1String("Recent")) && !path.contains(QLatin1String("Go To")))
            dups->append(QStringLiteral("%1: %2 x%3").arg(path, it.key()).arg(it.value()));
}
}

void caseU26()
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1500, 900);
    if (!w || !w->project())
        return;
    showDoc(w, textItems(w->project()).value(3));
    int deepest = 0;
    QStringList dups, paths;
    for (QAction *top : w->menuBar()->actions())
        if (top->menu())
            walk(top->menu(), 1, &deepest, &dups, &paths, plainOf(top->text()));
    settle(300);
    fact(QStringLiteral("U-26 entries walked / deepest level"), QStringLiteral("%1 / %2").arg(paths.size()).arg(deepest));
    check(paths.size() > 150, QStringLiteral("(%1 entries were walked)").arg(paths.size()));
    check(deepest <= 3, QStringLiteral("no entry lies deeper than 3 levels (%1)").arg(deepest));
    check(dups.isEmpty(), QStringLiteral("no menu holds two entries of the same name"), dups.mid(0, 6).join(QLatin1String("; ")));
    // every entry that acts can be told apart by its path: no two commands share one path
    QSet<QString> seen;
    QStringList same;
    for (const QString &p : paths)
        if (seen.contains(p) && !p.contains(QLatin1String("Recent")))
            same << p;
        else
            seen.insert(p);
    check(same.isEmpty(), QStringLiteral("no two entries share one path"), same.mid(0, 6).join(QLatin1String("; ")));
    finishProject(w);
}

} // namespace cs
