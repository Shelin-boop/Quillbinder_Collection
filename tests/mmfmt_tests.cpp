// Scripted run for version 0.99.0: the format bar's controls act on the notes of a mind map (the NoteFormatTarget
// interface), and the right-click commands act on all selected notes.
//   xvfb-run -a ./qb_mmfmt_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "ui/AppSettings.h"
#include "ui/MindMapController.h"
#include "ui/MindMapDoc.h"
#include "ui/MindMapScene.h"
#include "ui/MindMapUi.h"
#include "ui/MindMapView.h"
#include "ui/NoteFormatTarget.h"
#include "ui/Theme.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QMenu>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QGraphicsTextItem>
#include <QUndoStack>

#include <cmath>
#include <cstdio>

using namespace qb;
using namespace qb::mindmap;

static int failures = 0;
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

static QAction *menuAction(QMenu *m, const QString &plainText)
{
    for (QAction *a : m->actions())
        if (QString(a->text()).remove(QLatin1Char('&')) == plainText)
            return a;
    return nullptr;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    const QString homeDir = home.path() + QLatin1String("/h");
    QDir().mkpath(homeDir);
    qputenv("HOME", homeDir.toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_KEYBOARD_LAYOUT", "us");
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderMmfmtTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderMmfmtTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    const QString outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    AppSettings &as = AppSettings::instance();
    as.load();
    as.welcomeShown = true;
    as.theme = QStringLiteral("light");
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();

    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("Format Test"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    Project *p = proj.get();
    BinderItem *a = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Map"));
    MindMapDoc *doc = mindmaps::docFor(p, a->id);
    doc->change(QStringLiteral("notes"), [doc] {
        for (int i = 0; i < 3; ++i) {
            Note n;
            n.id = i + 1;
            n.text = QStringLiteral("Note %1 with some words").arg(i + 1);
            n.pos = QPointF(40 + i * 220, 60);
            n.width = 160;
            doc->board.notes << n;
        }
    });
    doc->undoStack()->clear();
    MindMapWindow *win = MindMapWindow::open(doc, nullptr);
    win->resize(900, 600);
    win->show();
    win->activateWindow();
    (void)QTest::qWaitForWindowActive(win, 2000);
    settle(300);
    MindMapController *ctl = win->controller();
    MindMapScene *scene = win->view()->boardScene();
    auto note = [&](int id) { return doc->board.note(id); };
    win->view()->setFocus();
    settle(100);

    std::printf("== the interface finds the mind map from the widget that has the keyboard\n");
    check(NoteFormatTarget::forWidget(win->view()) == ctl, QStringLiteral("the view's target is its controller"));
    check(NoteFormatTarget::forWidget(win->view()->viewport()) == ctl, QStringLiteral("the viewport's target is its controller"));
    check(NoteFormatTarget::forWidget(nullptr) == nullptr, QStringLiteral("no widget, no target"));

    std::printf("== one note selected\n");
    check(!ctl->noteFormatActive() && !ctl->noteFormatEnabled(QStringLiteral("bold")), QStringLiteral("nothing selected: the bar's controls are off"));
    scene->selectIds({1});
    settle(100);
    check(ctl->noteFormatActive() && ctl->noteFormatEnabled(QStringLiteral("bold")) && ctl->noteFormatEnabled(QStringLiteral("textColor")),
          QStringLiteral("one note selected: bold and the text color are on"));
    check(!ctl->noteFormatEnabled(QStringLiteral("superscript")) && !ctl->noteFormatEnabled(QStringLiteral("highlight")) &&
              !ctl->noteFormatEnabled(QStringLiteral("bullets")),
          QStringLiteral("controls without a meaning for a note are off"));
    check(ctl->noteFormatApply(QStringLiteral("textColor"), QColor(200, 30, 30)) && note(1)->look.textColor == QColor(200, 30, 30),
          QStringLiteral("the bar's text color changes the note"));
    check(ctl->noteFormatState().textColor == QColor(200, 30, 30), QStringLiteral("the bar shows the note's text color"));
    check(ctl->noteFormatApply(QStringLiteral("bold")) && note(1)->bold && ctl->noteFormatState().bold == 1,
          QStringLiteral("bold: the note changes and the bar shows it"));
    check(ctl->noteFormatApply(QStringLiteral("size"), 20.0) && qFuzzyCompare(note(1)->fontSize, 20.0) &&
              qFuzzyCompare(ctl->noteFormatState().size, 20.0),
          QStringLiteral("size: the note changes and the bar shows it"));
    check(ctl->noteFormatApply(QStringLiteral("alignCenter")) && note(1)->align == Align::Center && ctl->noteFormatState().align == 1,
          QStringLiteral("alignment: the note changes and the bar shows it"));
    check(ctl->noteFormatApply(QStringLiteral("italic")) && note(1)->italic && ctl->noteFormatApply(QStringLiteral("underline")) &&
              note(1)->underline && ctl->noteFormatApply(QStringLiteral("strike")) && note(1)->strike,
          QStringLiteral("italic, underline and strikethrough"));
    check(ctl->noteFormatApply(QStringLiteral("clearFormat")) && !note(1)->bold && !note(1)->italic && !note(1)->look.textColor.isValid() &&
              note(1)->align == Align::Left,
          QStringLiteral("clear formatting"));
    doc->undoStack()->clear();

    std::printf("== three notes selected\n");
    scene->selectIds({1, 2, 3});
    settle(100);
    const int before = doc->undoStack()->index();
    ctl->noteFormatApply(QStringLiteral("textColor"), QColor(10, 120, 40));
    bool all = true;
    for (int i = 1; i <= 3; ++i)
        all = all && note(i)->look.textColor == QColor(10, 120, 40);
    check(all && doc->undoStack()->index() == before + 1, QStringLiteral("text color from the bar: all three change, one undo step"));
    check(doc->undoStack()->undoText() == QLatin1String("Text Color"), QStringLiteral("the step is named after the change"), doc->undoStack()->undoText());
    doc->undoStack()->undo();
    all = true;
    for (int i = 1; i <= 3; ++i)
        all = all && !note(i)->look.textColor.isValid();
    check(all, QStringLiteral("one undo puts all three back"));
    note(2)->bold = true; // a mixed selection shows the neutral state
    check(ctl->noteFormatState().bold == -1, QStringLiteral("mixed bold shows the neutral state"));
    note(2)->bold = false;
    const int before2 = doc->undoStack()->index();
    ctl->noteFormatApply(QStringLiteral("bold"));
    check(note(1)->bold && note(2)->bold && note(3)->bold && doc->undoStack()->index() == before2 + 1, QStringLiteral("bold from the bar: all three, one undo step"));
    doc->undoStack()->undo();
    check(!note(1)->bold && !note(2)->bold && !note(3)->bold, QStringLiteral("one undo: bold is off in all three"));
    ctl->noteFormatApply(QStringLiteral("textColor"), QColor(10, 120, 40));
    ctl->noteFormatApply(QStringLiteral("bold"));
    ctl->noteFormatApply(QStringLiteral("alignRight"));
    win->grab().save(outDir + QStringLiteral("/three-notes-formatted.png"));

    std::printf("== the right-click menu acts on all selected notes\n");
    doc->undoStack()->clear();
    scene->selectIds({1, 2, 3});
    settle(100);
    for (int i = 1; i <= 3; ++i)
        note(i)->look.textColor = QColor();
    QMenu menu;
    ctl->fillContextMenu(&menu);
    QMenu *ink = nullptr;
    for (QAction *ac : menu.actions())
        if (ac->menu() && ac->menu()->objectName() == QLatin1String("mm_menuText"))
            ink = ac->menu();
    check(ink != nullptr, QStringLiteral("the menu has Text Color"));
    if (ink) {
        QAction *first = nullptr;
        for (QAction *ac : ink->actions())
            if (!ac->isSeparator() && !ac->menu() && ac->isEnabled() && ac->data().isValid() && ac->data().canConvert<QColor>()) {
                first = ac;
                break;
            }
        if (!first)
            for (QAction *ac : ink->actions())
                if (!ac->isSeparator() && !ac->menu() && ac->isEnabled() && !ac->text().isEmpty() && ac->text() != QLatin1String("Automatic")) {
                    first = ac;
                    break;
                }
        check(first != nullptr, QStringLiteral("a color entry exists"));
        if (first) {
            first->trigger();
            settle(50);
            check(note(1)->look.textColor.isValid() && note(2)->look.textColor == note(1)->look.textColor &&
                      note(3)->look.textColor == note(1)->look.textColor && doc->undoStack()->index() == 1,
                  QStringLiteral("a color from the right-click menu: all three, one undo step"));
            doc->undoStack()->undo();
            check(!note(1)->look.textColor.isValid() && !note(2)->look.textColor.isValid() && !note(3)->look.textColor.isValid(),
                  QStringLiteral("one undo puts all three back"));
        }
    }
    const int bw1 = note(1)->look.borderWeight, bw3 = note(3)->look.borderWeight;
    QAction *b3 = ctl->action(QStringLiteral("mm_border3"));
    check(b3 && b3->isEnabled(), QStringLiteral("the border command is on"));
    if (b3) {
        b3->trigger();
        check(note(1)->look.borderWeight == 3 && note(2)->look.borderWeight == 3 && note(3)->look.borderWeight == 3 &&
                  doc->undoStack()->index() == 1,
              QStringLiteral("a border from the right-click menu: all three, one undo step"));
        doc->undoStack()->undo();
        check(note(1)->look.borderWeight == bw1 && note(3)->look.borderWeight == bw3, QStringLiteral("one undo puts all three back"));
    }
    scene->selectIds({1, 2, 3});
    win->view()->grab();
    check(scene->selectedNotes().size() == 3, QStringLiteral("(three still selected after the menu was built)"));

    std::printf("== a note in editing: the bar changes only the selected words\n");
    doc->undoStack()->clear();
    for (int i = 1; i <= 3; ++i)
        note(i)->bold = false;
    scene->selectIds({2});
    settle(100);
    scene->startEditing(2, false);
    settle(100);
    NoteItem *ni = scene->editingNote();
    check(ni != nullptr, QStringLiteral("(note 2 is being edited)"));
    if (ni) {
        QTextCursor c = ni->textItem()->textCursor();
        c.setPosition(0);
        c.setPosition(4, QTextCursor::KeepAnchor);
        ni->textItem()->setTextCursor(c);
        check(ctl->noteFormatEnabled(QStringLiteral("bold")), QStringLiteral("editing: bold is on"));
        ctl->noteFormatApply(QStringLiteral("bold"));
        check(ni->textItem()->textCursor().charFormat().fontWeight() >= QFont::Bold && !note(2)->bold && !note(1)->bold,
              QStringLiteral("editing: only the selected words get bold; the note's own flag stays off"),
              QStringLiteral("weight %1 note2 %2 note1 %3").arg(ni->textItem()->textCursor().charFormat().fontWeight()).arg(note(2)->bold).arg(note(1)->bold));
        check(ctl->noteFormatState().bold == 1, QStringLiteral("editing: the bar shows the selected words' state"));
    }
    std::printf("== a pastel fill without a text color is readable on a dark and a light board\n");
    scene->selectIds({});
    const QColor oldBg = doc->board.settings.background;
    for (const QColor &bg : {QColor(30, 30, 36), QColor(250, 250, 245)}) {
        doc->board.settings.background = bg;
        doc->board.settings.textColor = QColor();
        Appearance look;
        look.fill = QColor(255, 220, 120);
        double r1 = 0, r2 = 0;
        const QColor ink = scene->textColor(look);
        auto lum = [](const QColor &c) {
            auto ch = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
            return 0.2126 * ch(c.redF()) + 0.7152 * ch(c.greenF()) + 0.0722 * ch(c.blueF());
        };
        r1 = (qMax(lum(ink), lum(look.fill)) + 0.05) / (qMin(lum(ink), lum(look.fill)) + 0.05);
        const QColor edge = scene->inkOn(look);
        r2 = (qMax(lum(edge), lum(look.fill)) + 0.05) / (qMin(lum(edge), lum(look.fill)) + 0.05);
        check(r1 >= 4.5 && r2 >= 4.5, QStringLiteral("board %1: text %2:1 and border %3:1 on the fill").arg(bg.name()).arg(r1, 0, 'f', 1).arg(r2, 0, 'f', 1));
    }
    doc->board.settings.background = oldBg;
    int rc = failures;
    std::printf("%d failure(s)\n", rc);
    delete win;
    return rc ? 1 : 0;
}
