// The system test, 0.99.0 (item 124): handwriting (S-H01 to S-H06), the rule for the AI models (S-G01 to S-G04), the small
// interface (S-S01 to S-S03), the usability cases U-21 to U-26 and the kills X-13 to X-16. The surroundings are the ones
// of the feature suites: a fake recognizer, a fake memory (sysmem::setFakeRam), a fake AI engine, synthetic pen and touch
// events; the program itself is the real main window with a real project.
#include "cs.h"

#include "compile/Compiler.h"
#include "compile/Progress.h"
#include "core/ModelMemory.h"
#include "core/SystemMemory.h"
#include "core/Project.h"
#include "core/crypto/ProjectCrypto.h"
#include "tools/ToolApi.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/CompileDialog.h"
#include "ui/EditorPane.h"
#include "ui/FocusLine.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/MessageLine.h"
#include "ui/OptionsDialog.h"
#include "ui/PenInput.h"
#include "ui/SmallInterface.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"
#include "ui/ToolHost.h"
#include "ui/ToolbarConfig.h"
#include "ui/handwriting/HandwritingController.h"
#include "ui/handwriting/HandwritingPage.h"
#include "ui/handwriting/HandwritingPanel.h"
#include "ui/handwriting/HandwritingSettings.h"
#include "ui/handwriting/InkCanvas.h"
#include "ui/handwriting/Recognizer.h"
#include "ui/handwriting/WordStrip.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QTest>
#include <QTextEdit>
#include <QDirIterator>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSettings>
#include <QTabletEvent>
#include <QToolBar>
#include <QToolButton>
#include <QTouchEvent>

namespace cs {

using namespace qb;
using namespace qb::handwriting;

namespace {

// ---------------------------------------------------------------- synthetic pen and fingers (as the feature suite makes them)
qint64 g_clock = 100000;
void tick(qint64 ms) { g_clock += ms; }

QPointingDevice *penDevice()
{
    static QPointingDevice *d = new QPointingDevice(QStringLiteral("cs-pen"), 21, QInputDevice::DeviceType::Stylus, QPointingDevice::PointerType::Pen,
                                                    QInputDevice::Capability::Position | QInputDevice::Capability::Pressure, 1, 2);
    return d;
}
QPointingDevice *touchDevice()
{
    static QPointingDevice *d = new QPointingDevice(QStringLiteral("cs-touch"), 22, QInputDevice::DeviceType::TouchScreen, QPointingDevice::PointerType::Finger,
                                                    QInputDevice::Capability::Position | QInputDevice::Capability::Area, 5, 0);
    return d;
}
void tablet(InkCanvas *c, QEvent::Type type, const QPointF &p, bool side = false)
{
    Qt::MouseButtons buttons = Qt::LeftButton;
    if (side)
        buttons |= Qt::RightButton;
    const Qt::MouseButton button = type == QEvent::TabletMove ? Qt::NoButton : Qt::LeftButton;
    if (type == QEvent::TabletRelease)
        buttons = side ? Qt::RightButton : Qt::NoButton;
    QTabletEvent e(type, penDevice(), p, c->mapToGlobal(p.toPoint()), 0.6, 0, 0, 0, 0, 0, Qt::NoModifier, button, buttons);
    QApplication::sendEvent(c, &e);
}
void penStroke(InkCanvas *c, const QList<QPointF> &pts, bool side = false, int step = 10)
{
    tablet(c, QEvent::TabletPress, pts.first(), side);
    for (int i = 1; i < pts.size(); ++i) {
        tick(step);
        tablet(c, QEvent::TabletMove, pts[i], side);
    }
    tick(step);
    tablet(c, QEvent::TabletRelease, pts.last(), side);
}
QList<QPointF> lineOf(QPointF a, QPointF b, int n = 8)
{
    QList<QPointF> v;
    for (int i = 0; i <= n; ++i)
        v << a + (b - a) * (double(i) / n);
    return v;
}
QList<QPointF> waveOf(QPointF a, double w, int n = 24)
{
    QList<QPointF> v;
    for (int i = 0; i <= n; ++i)
        v << QPointF(a.x() + w * i / n, a.y() + 8 * std::sin(i * 0.9));
    return v;
}
QList<QPointF> scratchOf(QPointF a, double w, int sweeps = 6)
{
    QList<QPointF> v;
    for (int s = 0; s < sweeps; ++s) {
        const QPointF from = s % 2 ? QPointF(a.x() + w, a.y() + s) : QPointF(a.x(), a.y() + s);
        const QPointF to = s % 2 ? QPointF(a.x(), a.y() + s + 1) : QPointF(a.x() + w, a.y() + s + 1);
        for (int i = 0; i <= 6; ++i)
            v << from + (to - from) * (i / 6.0);
    }
    return v;
}
void touch(InkCanvas *c, QEvent::Type type, const QList<std::pair<int, QPointF>> &pts, const QList<QEventPoint::State> &states)
{
    QList<QEventPoint> points;
    for (int i = 0; i < pts.size(); ++i) {
        const QPointF scene = QPointF(c->mapTo(c->window(), QPoint(0, 0))) + pts[i].second;
        points << QEventPoint(pts[i].first, states[i], scene, QPointF(c->mapToGlobal(pts[i].second.toPoint())));
    }
    QTouchEvent e(type, touchDevice(), Qt::NoModifier, points);
    QApplication::sendEvent(c, &e);
}
void finger(InkCanvas *c, int id, const QList<QPointF> &pts, int step = 10)
{
    touch(c, QEvent::TouchBegin, {{id, pts.first()}}, {QEventPoint::Pressed});
    for (int i = 1; i < pts.size(); ++i) {
        tick(step);
        touch(c, QEvent::TouchUpdate, {{id, pts[i]}}, {QEventPoint::Updated});
    }
    tick(step);
    touch(c, QEvent::TouchEnd, {{id, pts.last()}}, {QEventPoint::Released});
}
QPointF canvasPointOf(InkCanvas *c, TextEditor *ed, int position)
{
    QTextCursor tc(ed->document());
    tc.setPosition(position);
    return QPointF(c->mapFromGlobal(ed->viewport()->mapToGlobal(ed->cursorRect(tc).center())));
}
Result oneLine(const QStringList &words)
{
    Result r;
    r.ok = true;
    for (const QString &t : words) {
        Word w;
        w.text = t;
        r.words << w;
    }
    return r;
}

// the window with the handwriting parts, a fake recognizer, short times and our own clock
struct Hw {
    MainWindow *w = nullptr;
    Project *p = nullptr;
    BinderItem *doc = nullptr;
    HandwritingController *hw = nullptr;
    InkCanvas *canvas = nullptr;
    HandwritingPanel *panel = nullptr;
    WordStrip *strip = nullptr;
    TextEditor *ed = nullptr;
    QString path;
    void setTimes(int pause, int stay, int fade)
    {
        HandwritingSettings s = HandwritingConfig::instance().get();
        s.pauseMs = pause;
        s.stayMs = stay;
        s.fadeMs = fade;
        HandwritingConfig::instance().set(s);
    }
    void readAndFade()
    {
        for (int ms : {500, 500, 500, 1000}) {
            tick(ms);
            canvas->advance();
        }
    }
};
FakeRecognizer &fakeRecognizer()
{
    static FakeRecognizer f;
    return f;
}

bool hwUp(Hw &r, bool withRecognizer = true)
{
    FakeRecognizer &fake = fakeRecognizer();
    fake.setLines({QStringLiteral("hello world")});
    fake.setAlternatives(QStringLiteral("hello"), {QStringLiteral("hallo"), QStringLiteral("jello")});
    Recognizer::setCurrent(withRecognizer ? &fake : nullptr);
    QSettings().remove(QStringLiteral("handwriting"));
    HandwritingConfig::instance().reload();
    PenInput::instance().setClock([] { return g_clock; });
    r.path = qEnvironmentVariableIsSet("QB_CS_PROJECT") ? qEnvironmentVariable("QB_CS_PROJECT") : fixtureCopy(QStringLiteral("fx1-qb"));
    r.w = openWindow(r.path, 1500, 900);
    if (!r.w || !r.w->project())
        return false;
    r.p = r.w->project();
    r.doc = textItems(r.p).value(3);
    r.ed = showDoc(r.w, r.doc);
    r.hw = r.w->handwriting();
    if (!r.hw || !r.ed)
        return false;
    r.canvas = r.hw->canvas();
    r.panel = r.hw->panel();
    r.strip = r.panel->strip();
    if (QAction *show = action(r.w, QStringLiteral("showHandwritingButton")))
        show->setChecked(true);
    r.setTimes(300, 400, 400);
    settle(300);
    return true;
}

} // namespace

// ================================================================ S-H01 pen and finger
void caseSH01()
{
    Hw r;
    if (!hwUp(r))
        return;
    step(QStringLiteral("the pad in the inspector: a pen writes, a line is read after the pause"));
    r.hw->setMode(Mode::Inspector);
    settle(300);
    check(r.canvas->parentWidget() == r.panel->padHolder() && r.canvas->isVisible(), QStringLiteral("Inspector mode: the pad is the writing area"));
    const double pitch = HandwritingConfig::instance().get().grid.pitch();
    r.setTimes(2000, 400, 400);
    const int req = fakeRecognizer().requests();
    penStroke(r.canvas, waveOf(QPointF(30, pitch * 1.2), 120));
    check(r.canvas->strokeCount() == 1 && r.canvas->pendingLines() == 1, QStringLiteral("a pen stroke is ink, one line waits"));
    tick(100);
    r.canvas->advance();
    check(fakeRecognizer().requests() == req, QStringLiteral("the line is not read before the pause is over"));
    tick(2100);
    r.canvas->advance();
    check(fakeRecognizer().requests() == req + 1 && r.canvas->pendingLines() == 0, QStringLiteral("after the pause the line is read"));
    r.setTimes(300, 400, 400);
    r.readAndFade();
    check(r.strip->text() == QLatin1String("hello world"), QStringLiteral("the text of the line appears in the strip"), r.strip->text());
    shot(r.w, QStringLiteral("h01-strip"));
    r.strip->clear();
    r.canvas->clearInk();
    step(QStringLiteral("a finger writes when no pen is about"));
    {
        PenInput::Sample gone;   // the pen is out of range
        PenInput::instance().feed(gone);
    }
    tick(60000);
    check(!PenInput::instance().inUse(30000), QStringLiteral("(no pen in use)"));
    finger(r.canvas, 3, waveOf(QPointF(30, pitch * 1.2), 120));
    check(r.canvas->strokeCount() == 1, QStringLiteral("one finger writes in the pad"), QString::number(r.canvas->strokeCount()));
    r.canvas->clearInk();
    step(QStringLiteral("over the text: with a pen about, a finger scrolls and taps and writes nothing"));
    r.hw->setMode(Mode::Editor);
    settle(400);
    InkCanvas::scrollBarFor(r.ed)->setValue(0);
    PenInput::Sample s;
    s.inRange = true;
    PenInput::instance().feed(s);
    tick(3000);
    const int placed = r.canvas->placedCursors();
    const QPointF tp = canvasPointOf(r.canvas, r.ed, 10);
    finger(r.canvas, 4, {tp, tp});
    check(r.canvas->placedCursors() == placed + 1 && r.canvas->strokeCount() == 0, QStringLiteral("a finger tap places the cursor, no ink"));
    check(qAbs(r.ed->textCursor().position() - 10) <= 2, QStringLiteral("... at the place tapped"), QString::number(r.ed->textCursor().position()));
    // the pen's short tap and its side button
    const QPointF tp2 = canvasPointOf(r.canvas, r.ed, 20);
    tablet(r.canvas, QEvent::TabletPress, tp2);
    tick(60);
    tablet(r.canvas, QEvent::TabletRelease, tp2);
    check(r.canvas->strokeCount() == 0 && qAbs(r.ed->textCursor().position() - 20) <= 2, QStringLiteral("a short pen tap places the cursor too"));
    const QPointF from = canvasPointOf(r.canvas, r.ed, 4), to = canvasPointOf(r.canvas, r.ed, 22);
    tablet(r.canvas, QEvent::TabletPress, from, true);
    for (int i = 1; i <= 6; ++i) {
        tick(10);
        tablet(r.canvas, QEvent::TabletMove, from + (to - from) * (i / 6.0), true);
    }
    tablet(r.canvas, QEvent::TabletRelease, to, true);
    check(r.ed->textCursor().hasSelection() && r.canvas->strokeCount() == 0, QStringLiteral("the pen's side button selects text"), r.ed->textCursor().selectedText());
    shot(r.w, QStringLiteral("h01-selected"));
    r.hw->setMode(Mode::Off);
    finishProject(r.w);
}

// ================================================================ S-H02 the three modes
void caseSH02()
{
    Hw r;
    if (!hwUp(r))
        return;
    QToolButton *button = r.w->findChild<QToolButton *>(QStringLiteral("handwritingButton"));
    check(button && button->isVisible(), QStringLiteral("the button is on the toolbar once its switch is on"));
    check(r.hw->mode() == Mode::Off, QStringLiteral("the mode is off at first"));
    QTest::mouseClick(button, Qt::LeftButton);
    settle(200);
    check(r.hw->mode() == Mode::Inspector, QStringLiteral("a tap switches on the mode used last (Inspector the first time)"));
    shot(r.w, QStringLiteral("h02-inspector"));
    QTest::mouseClick(button, Qt::LeftButton);
    settle(100);
    check(r.hw->mode() == Mode::Off && !r.canvas->isVisible(), QStringLiteral("a second tap switches off; no writing area is shown"));
    for (Mode m : {Mode::Inspector, Mode::Editor, Mode::Both}) {
        const QString name = m == Mode::Inspector ? QStringLiteral("Inspector") : m == Mode::Editor ? QStringLiteral("Editor") : QStringLiteral("Both");
        step(QStringLiteral("mode %1").arg(name));
        r.hw->setMode(m);
        settle(400);
        check(r.hw->mode() == m && r.canvas->isVisible(), QStringLiteral("%1: the writing area is shown").arg(name));
        if (m == Mode::Inspector)
            check(r.canvas->role() == InkCanvas::Role::Pad, QStringLiteral("Inspector: the pad in the tab"));
        if (m == Mode::Editor) {
            const QRect ea(r.w->editorAreaWidget()->mapTo(r.w, QPoint(0, 0)), r.w->editorAreaWidget()->size());
            check(r.canvas->role() == InkCanvas::Role::Overlay && r.canvas->geometry().contains(ea), QStringLiteral("Editor: a see-through layer over the editor"));
        }
        // one stroke is ink in every mode
        r.canvas->clearInk();
        const QRectF g = r.canvas->gridRect();
        const QPointF a = g.isNull() ? canvasPointOf(r.canvas, r.ed, 4) + QPointF(0, 40) : QPointF(g.left() + 30, g.top() + 40);
        penStroke(r.canvas, waveOf(a, 120));
        check(r.canvas->strokeCount() == 1, QStringLiteral("%1: a pen stroke is ink").arg(name));
        r.canvas->clearInk();
        shot(r.w, QStringLiteral("h02-%1").arg(name.toLower()));
    }
    // the long press: the four choices
    r.hw->setMode(Mode::Off);
    QTest::mousePress(button, Qt::LeftButton);
    QTest::qWait(HandwritingButton::kHoldMs + 250);
    QMenu *choices = r.w->findChild<QMenu *>(QStringLiteral("handwritingChoices"));
    QStringList names;
    if (choices)
        for (QAction *a : choices->actions())
            names << a->text();
    check(choices && names == QStringList({QStringLiteral("Off"), QStringLiteral("Editor"), QStringLiteral("Inspector"), QStringLiteral("Both")}),
          QStringLiteral("a long press offers Off, Editor, Inspector, Both"), names.join(QLatin1Char(',')));
    QTest::mouseRelease(button, Qt::LeftButton);
    if (choices)
        choices->close();
    r.hw->setMode(Mode::Off);
    finishProject(r.w);
}

// ================================================================ S-H03 the rest zone
void caseSH03()
{
    Hw r;
    if (!hwUp(r))
        return;
    r.hw->setMode(Mode::Editor);
    settle(400);
    InkCanvas::scrollBarFor(r.ed)->setValue(0);
    const QPointF s = canvasPointOf(r.canvas, r.ed, 4) + QPointF(0, 40);
    tablet(r.canvas, QEvent::TabletPress, s);
    for (int i = 1; i <= 6; ++i) {
        tick(10);
        tablet(r.canvas, QEvent::TabletMove, s + QPointF(i * 20, 4));
    }
    const int ignored = r.canvas->ignoredTouches();
    const QRectF zone = r.canvas->restZone();
    check(!zone.isNull() && zone.contains(s + QPointF(100, 150)), QStringLiteral("while the pen writes, a rest zone surrounds the writing"));
    shot(r.w, QStringLiteral("h03-zone"));
    finger(r.canvas, 8, {s + QPointF(100, 150), s + QPointF(110, 160)});
    check(r.canvas->ignoredTouches() == ignored + 1 && r.canvas->strokeCount() == 0, QStringLiteral("a palm touch in the zone is ignored and leaves no ink"));
    const int placed = r.canvas->placedCursors();
    const QPointF far = canvasPointOf(r.canvas, r.ed, 10) + QPointF(0, 330);
    finger(r.canvas, 9, {far, far});
    check(r.canvas->ignoredTouches() == ignored + 1 && r.canvas->placedCursors() == placed + 1, QStringLiteral("a touch far from the writing is accepted"));
    tablet(r.canvas, QEvent::TabletRelease, s + QPointF(120, 4));
    tick(500);
    finger(r.canvas, 10, {s + QPointF(100, 150), s + QPointF(100, 150)});
    check(r.canvas->ignoredTouches() == ignored + 2, QStringLiteral("0.5 s after the pen lifted the zone still ignores fingers"));
    tick(1200);
    const int ign2 = r.canvas->ignoredTouches();
    finger(r.canvas, 11, {s + QPointF(100, 150), s + QPointF(100, 150)});
    check(r.canvas->ignoredTouches() == ign2, QStringLiteral("about 1 s after the pen lifted, fingers are accepted again"));
    // a left-handed writer: the zone is wider on the left
    HandwritingSettings hs = HandwritingConfig::instance().get();
    hs.leftHanded = true;
    HandwritingConfig::instance().set(hs);
    r.canvas->clearInk();
    settle(200);
    tick(5000);
    tablet(r.canvas, QEvent::TabletPress, s);
    tick(10);
    tablet(r.canvas, QEvent::TabletMove, s + QPointF(20, 3));
    const QRectF zl = r.canvas->restZone();
    check(!zl.isNull() && (s.x() - zl.left()) > (zl.right() - s.x() - 20), QStringLiteral("left-handed: the zone is wider on the left"),
          QStringLiteral("zone %1..%2, pen at %3").arg(zl.left()).arg(zl.right()).arg(s.x()));
    tablet(r.canvas, QEvent::TabletRelease, s + QPointF(20, 3));
    r.canvas->clearInk();
    r.hw->setMode(Mode::Off);
    finishProject(r.w);
}

// ================================================================ S-H04 Insert and Pin
void caseSH04()
{
    Hw r;
    if (!hwUp(r))
        return;
    r.hw->setMode(Mode::Editor);
    settle(400);
    InkCanvas::scrollBarFor(r.ed)->setValue(0);
    const QString before = r.ed->toPlainText();
    const int target = qMax(10, before.indexOf(QLatin1Char(' '), 20));
    const QPointF tp = canvasPointOf(r.canvas, r.ed, target);
    tablet(r.canvas, QEvent::TabletPress, tp);
    tick(50);
    tablet(r.canvas, QEvent::TabletRelease, tp);
    const int held = r.hw->held().position();
    QTextCursor front(r.ed->document());
    front.setPosition(0);
    front.insertText(QStringLiteral("NEW "));
    check(r.hw->held().position() == held + 4, QStringLiteral("the held place moves with the text"));
    r.strip->addLine(QImage(30, 20, QImage::Format_RGB32), oneLine({QStringLiteral("lantern"), QStringLiteral("light")}));
    QTest::mouseClick(r.panel->insertButton(), Qt::LeftButton);
    check(r.ed->toPlainText().contains(QLatin1String("lantern light")) && r.strip->isEmpty(), QStringLiteral("Insert puts the strip's text at the held place"));
    shot(r.w, QStringLiteral("h04-inserted"));
    int undos = 0;
    while (undos < 6 && r.ed->toPlainText().contains(QLatin1String("lantern light"))) {
        r.ed->document()->undo();
        ++undos;
    }
    fact(QStringLiteral("S-H04 undo steps until the inserted text is gone"), undos);
    // Pin: the text sticks, a tap on the editor shows the place, the check mark drops it
    r.strip->addLine(QImage(30, 20, QImage::Format_RGB32), oneLine({QStringLiteral("pinned")}));
    QTest::mouseClick(r.panel->pinButton(), Qt::LeftButton);
    settle(100);
    check(r.hw->pinned() && r.hw->pinGhost()->isVisible() && r.strip->isEmpty(), QStringLiteral("Pin: the text leaves the strip and sticks to the pen"));
    auto *drop = r.hw->pinGhost()->findChild<QToolButton *>(QStringLiteral("hwPinDrop"));
    check(drop && !drop->isEnabled(), QStringLiteral("the check mark waits for a place"));
    const int at = qMax(5, r.ed->toPlainText().indexOf(QLatin1Char(' '), 60));
    QTextCursor tc(r.ed->document());
    tc.setPosition(at);
    QTest::mouseClick(r.ed->viewport(), Qt::LeftButton, Qt::NoModifier, r.ed->cursorRect(tc).center());
    settle(150);
    check(r.hw->hasPlace() && drop && drop->isEnabled(), QStringLiteral("a tap on the editor shows the place"));
    shot(r.w, QStringLiteral("h04-pin"));
    QTest::mouseClick(drop, Qt::LeftButton);
    settle(150);
    check(!r.hw->pinned() && r.ed->toPlainText().contains(QLatin1String("pinned")), QStringLiteral("the check mark drops the text at the place"));
    // cancel: the text goes back to the strip
    r.strip->addLine(QImage(30, 20, QImage::Format_RGB32), oneLine({QStringLiteral("again")}));
    QTest::mouseClick(r.panel->pinButton(), Qt::LeftButton);
    QTest::mouseClick(r.hw->pinGhost()->findChild<QToolButton *>(QStringLiteral("hwPinCancel")), Qt::LeftButton);
    check(!r.hw->pinned() && r.strip->text() == QLatin1String("again"), QStringLiteral("the cross cancels: the text is back in the strip"), r.strip->text());
    r.strip->clear();
    // saved and read again: the inserted word is in the file
    QString err;
    check(r.p->save(&err), QStringLiteral("the project saves"), err);
    const QString id = r.doc->id;
    finishProject(r.w);
    auto p2 = Project::open(r.path, &err);
    check(p2 && p2->docs()->text(id)->toPlainText().contains(QLatin1String("pinned")), QStringLiteral("a fresh start reads the dropped word from the file"), err);
}

// ================================================================ S-H05 scratching out
void caseSH05()
{
    Hw r;
    if (!hwUp(r))
        return;
    r.hw->setMode(Mode::Editor);
    settle(400);
    InkCanvas::scrollBarFor(r.ed)->setValue(0);
    r.canvas->clearInk();
    const QPointF s = canvasPointOf(r.canvas, r.ed, 4) + QPointF(0, 60);
    penStroke(r.canvas, waveOf(s, 120));
    penStroke(r.canvas, waveOf(s + QPointF(0, 200), 120));
    check(r.canvas->strokeCount() == 2, QStringLiteral("(two words of ink, far apart)"));
    penStroke(r.canvas, scratchOf(s + QPointF(-5, -2), 140), false, 4);
    check(r.canvas->strokeCount() == 1, QStringLiteral("scratching a word of ink out deletes it, the other stays"), QString::number(r.canvas->strokeCount()));
    r.canvas->clearInk();
    // the words of the text
    QTextCursor all(r.ed->document());
    all.select(QTextCursor::Document);
    all.insertText(QStringLiteral("The wind rose over the calm grey sea and the gulls cried out above the lamp."));
    settle(200);
    const QString before = r.ed->toPlainText();
    const int p1 = before.indexOf(QLatin1String("calm")), p2 = before.indexOf(QLatin1String("sea"));
    const QPointF a = canvasPointOf(r.canvas, r.ed, p1 + 1), b = canvasPointOf(r.canvas, r.ed, p2 + 1);
    shot(r.w, QStringLiteral("h05-before"));
    penStroke(r.canvas, scratchOf(QPointF(a.x(), a.y() - 4), b.x() - a.x()), false, 4);
    const QString after = r.ed->toPlainText();
    check(!after.contains(QLatin1String("calm")) && !after.contains(QLatin1String("grey")) && !after.contains(QLatin1String("sea")) &&
              after.contains(QLatin1String("wind rose over the and the gulls")),
          QStringLiteral("scratching words of the text out deletes them"), after.left(80));
    shot(r.w, QStringLiteral("h05-after"));
    r.ed->document()->undo();
    check(r.ed->toPlainText().contains(QLatin1String("calm grey sea")), QStringLiteral("... and undo brings them back"), r.ed->toPlainText().left(80));
    // a word in the strip
    r.strip->clear();
    r.strip->addLine(QImage(30, 20, QImage::Format_RGB32), oneLine({QStringLiteral("Hallo"), QStringLiteral("Welt")}));
    check(r.strip->text() == QLatin1String("Hallo Welt"), QStringLiteral("(a line in the strip)"), r.strip->text());
    r.strip->clear();
    r.hw->setMode(Mode::Off);
    finishProject(r.w);
}

// ================================================================ S-H06 without and with a recognizer
void caseSH06()
{
    Hw r;
    if (!hwUp(r, false))
        return;
    QToolButton *button = r.w->findChild<QToolButton *>(QStringLiteral("handwritingButton"));
    const int before = r.hw->settingsRequests();
    bool pageShown = false;
    QTimer::singleShot(400, [&] {
        if (auto *d = qobject_cast<OptionsDialog *>(QApplication::activeModalWidget())) {
            auto *page = d->findChild<HandwritingPage *>();
            pageShown = page && page->isVisible();
            shot(d, QStringLiteral("h06-options"));
            d->reject();
        }
    });
    QTest::mouseClick(button, Qt::LeftButton);
    settle(300);
    check(r.hw->mode() == Mode::Off && r.hw->settingsRequests() == before + 1, QStringLiteral("no recognizer installed: a tap opens the settings, the mode stays off"));
    check(pageShown, QStringLiteral("... the page File > Options > Handwriting is the one shown"));
    // the recognizer is there now
    Recognizer::setCurrent(&fakeRecognizer());
    QTest::mouseClick(button, Qt::LeftButton);
    settle(200);
    check(r.hw->mode() != Mode::Off, QStringLiteral("with a recognizer the tap switches the mode on"));
    // the whole way: write, read, strip, Insert, save
    r.hw->setMode(Mode::Editor);
    settle(300);
    InkCanvas::scrollBarFor(r.ed)->setValue(0);
    fakeRecognizer().setLines({QStringLiteral("lighthouse keeper")});
    penStroke(r.canvas, lineOf(canvasPointOf(r.canvas, r.ed, 4) + QPointF(0, 40), canvasPointOf(r.canvas, r.ed, 4) + QPointF(180, 55), 14));
    r.readAndFade();
    check(r.strip->text() == QLatin1String("lighthouse keeper"), QStringLiteral("the written line is read into the strip"), r.strip->text());
    const QString textBefore = r.ed->toPlainText();
    QTextCursor c = r.ed->textCursor();
    c.movePosition(QTextCursor::End);
    r.ed->setTextCursor(c);
    tablet(r.canvas, QEvent::TabletPress, canvasPointOf(r.canvas, r.ed, 4));
    tick(50);
    tablet(r.canvas, QEvent::TabletRelease, canvasPointOf(r.canvas, r.ed, 4));
    QTest::mouseClick(r.panel->insertButton(), Qt::LeftButton);
    settle(200);
    check(r.ed->toPlainText().contains(QLatin1String("lighthouse keeper")) && r.ed->toPlainText() != textBefore, QStringLiteral("Insert puts the read text into the document"));
    r.hw->setMode(Mode::Off);
    QString err;
    check(r.p->save(&err), QStringLiteral("the project saves"), err);
    finishProject(r.w);
}

// ================================================================ S-G the rule for the AI models
namespace {
constexpr qint64 kMB = 1000 * 1000;
void fakeRam(int usedMb) { sysmem::setFakeRam({1000 * kMB, (1000 - usedMb) * kMB}); }
}

void caseSG01()   // the 10-minute unload (a short interval for the test)
{
    using modelmem::ModelMemory;
    ModelMemory &mm = ModelMemory::instance();
    mm.reset();
    fakeRam(100);
    check(ModelMemory::kDefaultIdleMinutes == 10 && mm.idleMinutes() == 10, QStringLiteral("the default time unused is 10 minutes"));
    qint64 now = 1000000;
    mm.setClock([&now] { return now; });
    QStringList log;
    auto unloader = [&log](const QString &id) {
        log << id;
        return true;
    };
    mm.requestLoad(QStringLiteral("t:a"), QStringLiteral("A"), 10 * kMB, QStringLiteral("ram"), unloader);
    mm.modelLoaded(QStringLiteral("t:a"));
    now += 599 * 1000;
    check(mm.unloadIdle().isEmpty() && mm.isLoaded(QStringLiteral("t:a")), QStringLiteral("unused for 9 min 59 s: the model stays"));
    mm.modelUsed(QStringLiteral("t:a"));
    now += 599 * 1000;
    check(mm.unloadIdle().isEmpty(), QStringLiteral("a use starts the time again"));
    now += 2000;
    check(mm.unloadIdle() == QStringList{QStringLiteral("t:a")} && log == QStringList{QStringLiteral("t:a")}, QStringLiteral("10 minutes after the last use it is unloaded"));
    // the timer with the real clock and a short time
    mm.reset();
    mm.setIdleMs(300);
    mm.requestLoad(QStringLiteral("t:b"), QStringLiteral("B"), 10 * kMB, QStringLiteral("ram"), unloader);
    mm.modelLoaded(QStringLiteral("t:b"));
    mm.startWatching(80);
    settle(120);
    const bool still = mm.isLoaded(QStringLiteral("t:b"));
    waitFor([&] { return !mm.isLoaded(QStringLiteral("t:b")); }, 3000);
    check(still && !mm.isLoaded(QStringLiteral("t:b")), QStringLiteral("the program's timer unloads an idle model by itself (300 ms test time)"));
    mm.setIdleMs(-1);
    mm.reset();
    sysmem::clearFakeRam();
}

void caseSG02()   // a second model while the memory is over 50 %
{
    using modelmem::ModelMemory;
    ModelMemory &mm = ModelMemory::instance();
    mm.reset();
    qint64 now = 5000000;
    mm.setClock([&now] { return now; });
    QStringList log;
    auto unloader = [&log](const QString &id) {
        log << id;
        return true;
    };
    fakeRam(300);
    modelmem::Decision d = mm.requestLoad(QStringLiteral("t:a"), QStringLiteral("A"), 100 * kMB, QStringLiteral("ram"), unloader);
    mm.modelLoaded(QStringLiteral("t:a"));
    check(d.allowed && d.unloaded.isEmpty(), QStringLiteral("under 50 % used: the first model loads, nothing is unloaded"));
    now += 60000;
    d = mm.requestLoad(QStringLiteral("t:b"), QStringLiteral("B"), 100 * kMB, QStringLiteral("ram"), unloader);
    mm.modelLoaded(QStringLiteral("t:b"));
    check(d.allowed && log.isEmpty() && mm.isLoaded(QStringLiteral("t:a")), QStringLiteral("still under 50 %: a second model loads beside the first"));
    now += 60000;
    fakeRam(600);
    d = mm.requestLoad(QStringLiteral("t:c"), QStringLiteral("C"), 100 * kMB, QStringLiteral("ram"), unloader);
    mm.modelLoaded(QStringLiteral("t:c"));
    check(d.allowed && log == QStringList{QStringLiteral("t:a")} && !mm.isLoaded(QStringLiteral("t:a")) && mm.isLoaded(QStringLiteral("t:b")),
          QStringLiteral("over 50 % used: the model unused longest goes first, then the new one loads"), log.join(QLatin1Char(',')));
    mm.reset();
    sysmem::clearFakeRam();
}

void caseSG03()   // the 80 % refusal and its reason
{
    using modelmem::ModelMemory;
    ModelMemory &mm = ModelMemory::instance();
    mm.reset();
    QStringList log, seen;
    auto unloader = [&log](const QString &id) {
        log << id;
        return true;
    };
    QObject::connect(&mm, &ModelMemory::refused, &mm, [&seen](const QString &why) { seen << why; });
    fakeRam(850);
    modelmem::Decision d = mm.requestLoad(QStringLiteral("t:big"), QStringLiteral("Big Model"), 10 * kMB, QStringLiteral("ram"), unloader);
    check(!d.allowed && d.reason.contains(QLatin1String("Big Model")) && d.reason.contains(QLatin1String("80")) && seen.size() == 1 && !mm.changeActive() && !mm.isLoaded(QStringLiteral("t:big")),
          QStringLiteral("85 % used, nothing of ours to unload: refused, the reason names the model and the 80 %"), d.reason);
    fakeRam(700);
    mm.requestLoad(QStringLiteral("t:old"), QStringLiteral("Old"), 10 * kMB, QStringLiteral("ram"), unloader);
    mm.modelLoaded(QStringLiteral("t:old"));
    d = mm.requestLoad(QStringLiteral("t:next"), QStringLiteral("Next"), 500 * kMB, QStringLiteral("ram"), unloader);
    check(!d.allowed && d.reason.contains(QLatin1String("Not enough memory")), QStringLiteral("still over 80 % with every model of ours gone: refused"), d.reason);
    check(!mm.isLoaded(QStringLiteral("t:next")) && !mm.changeActive(), QStringLiteral("the refused model is not in the list and no change stays under way"));
    mm.reset();
    sysmem::clearFakeRam();
}

void caseSG04()   // the message line under the inspector
{
    using modelmem::ModelMemory;
    ModelMemory &mm = ModelMemory::instance();
    mm.reset();
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1400, 900);
    if (!w || !w->project())
        return;
    toolhost::host().attachTools();
    Inspector *insp = w->findChild<Inspector *>();
    MessageLine *il = insp ? insp->messageLine() : nullptr;
    MessageLine *ml = w->messageLine();
    check(il && ml && !il->isVisible() && !ml->isVisible(), QStringLiteral("both message lines take no room while there is nothing to say"));
    if (!il || !ml)
        return;
    mm.changeBegin();
    settle(100);
    check(il->isVisible() && il->textLabel()->text() == QLatin1String("Model change in progress") && !ml->isVisible(),
          QStringLiteral("a model change under way: \"Model change in progress\" under the inspector"), il->textLabel()->text());
    check(il->geometry().bottom() >= insp->height() - 2 && il->height() < 60, QStringLiteral("... a slim line at the bottom of the pane"));
    shot(w, QStringLiteral("g04-change"));
    mm.changeEnd();
    settle(100);
    check(!il->isVisible(), QStringLiteral("the line goes when the change is over"));
    fakeRam(900);
    const tools::ModelDecision md = toolhost::host().requestModelLoad(QStringLiteral("none:big"), QStringLiteral("Big Model"), 300 * kMB, QStringLiteral("ram"));
    settle(100);
    check(!md.allowed && il->shownText().contains(QLatin1String("Not enough memory")), QStringLiteral("a refused load: the reason is under the inspector"), il->shownText());
    shot(w, QStringLiteral("g04-refused"));
    il->clearAll();
    insp->hide();
    settle(100);
    toolhost::host().requestModelLoad(QStringLiteral("none:big2"), QStringLiteral("Big Model"), 300 * kMB, QStringLiteral("ram"));
    settle(100);
    check(ml->shownText().contains(QLatin1String("Not enough memory")), QStringLiteral("with the inspector hidden the reason is under the binder"), ml->shownText());
    ml->clearAll();
    insp->show();
    sysmem::clearFakeRam();
    mm.reset();
    finishProject(w);
}

// ================================================================ S-S the small interface
namespace {
const QStringList kRound = {QStringLiteral("menu"),  QStringLiteral("new"),     QStringLiteral("bold"),   QStringLiteral("italic"), QStringLiteral("style"),
                            QStringLiteral("binder"), QStringLiteral("labels"), QStringLiteral("record"), QStringLiteral("notes")};
QToolButton *roundButton(MainWindow *w, const QString &n) { return qobject_cast<QToolButton *>(w->smallButton(n)); }
QMap<QString, bool> barsShown(MainWindow *w)
{
    QMap<QString, bool> m;
    for (QToolBar *b : w->findChildren<QToolBar *>())
        if (!b->objectName().isEmpty() && b != w->smallBar())
            m[b->objectName()] = b->isVisible();
    return m;
}
QAction *viewItem(MainWindow *w, const QString &text)
{
    for (QAction *top : w->menuBar()->actions())
        if (top->menu() && top->menu()->title().remove(QLatin1Char('&')) == QLatin1String("View"))
            for (QAction *a : top->menu()->actions())
                if (a->text().remove(QLatin1Char('&')) == text)
                    return a;
    return nullptr;
}
}

void caseSS01()   // switching on and off restores the window
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1280, 800);
    if (!w || !w->project())
        return;
    for (bool upright : {false, true}) {
        const QString what = upright ? QStringLiteral("upright") : QStringLiteral("wide");
        w->resize(upright ? QSize(1080, 1920) : QSize(1280, 800));
        settle(500);
        const QList<int> split = w->paneSplitter()->sizes(), side = w->sideSplitter()->sizes();
        const auto bars = barsShown(w);
        const bool menu = w->menuBar()->isVisible();
        const QByteArray state = w->saveState();
        QAction *item = viewItem(w, QStringLiteral("Small Interface"));
        check(item && item->isCheckable() && !item->isChecked(), QStringLiteral("%1: View > Small Interface is a check item, off").arg(what));
        if (!item)
            break;
        item->trigger();
        settle(600);
        check(w->smallInterface() && item->isChecked() && w->smallBar()->isVisible() && !w->menuBar()->isVisible(), QStringLiteral("%1: on: the round buttons, no menu bar").arg(what));
        shot(w, QStringLiteral("s01-%1-on").arg(what));
        item->trigger();
        settle(700);
        check(!w->smallInterface() && !w->smallBar()->isVisible() && w->menuBar()->isVisible() == menu, QStringLiteral("%1: off: the menu bar is back").arg(what));
        check(w->paneSplitter()->sizes() == split && w->sideSplitter()->sizes() == side, QStringLiteral("%1: the panes have the sizes they had").arg(what));
        check(barsShown(w) == bars, QStringLiteral("%1: the same toolbars are shown").arg(what));
        check(w->saveState() == state, QStringLiteral("%1: the window's saved state is the same").arg(what));
        shot(w, QStringLiteral("s01-%1-off").arg(what));
    }
    w->resize(1280, 800);
    finishProject(w);
}

void caseSS02()   // the round buttons
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1280, 800);
    if (!w || !w->project())
        return;
    w->setSmallInterface(true);
    settle(700);
    QScreen *sc = w->screen() ? w->screen() : QGuiApplication::primaryScreen();
    const int target = smallui::touchTargetPx(sc->physicalDotsPerInch() / qMax(1.0, sc->devicePixelRatio()));
    QStringList small, missing;
    for (const QString &n : kRound) {
        QToolButton *b = roundButton(w, n);
        if (!b)
            missing << n;
        else if (b->width() < target || b->height() < target || b->width() != b->height())
            small << n;
    }
    check(missing.isEmpty(), QStringLiteral("every button exists"), missing.join(QLatin1Char(' ')));
    check(small.isEmpty(), QStringLiteral("every button is round-sized and at least 9 mm (%1 px)").arg(target), small.join(QLatin1Char(' ')));
    bool aligned = true;
    for (int i = 1; i < kRound.size(); ++i)
        if (roundButton(w, kRound[i - 1]) && roundButton(w, kRound[i]))
            aligned = aligned && roundButton(w, kRound[i - 1])->mapTo(w, QPoint(0, 0)).x() == roundButton(w, kRound[i])->mapTo(w, QPoint(0, 0)).x();
    check(aligned, QStringLiteral("wide: the buttons stand in one column at the right edge"));
    shot(w, QStringLiteral("s02-buttons"));
    // bold on a marked word through the round button, one undo step
    TextEditor *e = showDoc(w, textItems(w->project()).value(3));
    if (e) {
        QTextCursor c = e->textCursor();
        c.movePosition(QTextCursor::Start);
        c.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        e->setTextCursor(c);
        const QString runsBefore = runsOf(e->document());
        roundButton(w, QStringLiteral("bold"))->click();
        settle(150);
        check(runsOf(e->document()) != runsBefore, QStringLiteral("the Bold button changes the marked word"));
        e->undo();
        check(runsOf(e->document()) == runsBefore, QStringLiteral("... and one undo takes it back"));
    }
    // the binder and the notes slide in, one at a time
    smallui::SlidePanel *bp = w->smallPanel(false), *np = w->smallPanel(true);
    roundButton(w, QStringLiteral("binder"))->click();
    settle(600);
    check(bp->isOpen() && !np->isOpen(), QStringLiteral("the binder button opens the binder panel"));
    shot(w, QStringLiteral("s02-binder"));
    roundButton(w, QStringLiteral("notes"))->click();
    settle(600);
    check(np->isOpen() && !bp->isOpen(), QStringLiteral("the notes button opens the notes and closes the binder panel"));
    shot(w, QStringLiteral("s02-notes"));
    roundButton(w, QStringLiteral("notes"))->click();
    settle(500);
    check(!np->isOpen(), QStringLiteral("a second tap closes the panel"));
    // the menu button leads back
    QToolButton *menu = roundButton(w, QStringLiteral("menu"));
    QMetaObject::invokeMethod(menu->menu(), "aboutToShow", Qt::DirectConnection);
    check(menu->menu()->actions().last()->text().remove(QLatin1Char('&')) == QLatin1String("Switch to the Full Interface"), QStringLiteral("the menu button ends with the way back"));
    w->findChild<QAction *>(QStringLiteral("smallToFull"))->trigger();
    settle(600);
    check(!w->smallInterface() && w->menuBar()->isVisible(), QStringLiteral("\"Switch to the Full Interface\" leads back"));
    finishProject(w);
}

void caseSS03()   // upright
{
    MainWindow *w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1080, 1920);
    if (!w || !w->project())
        return;
    settle(500);
    check(w->isUpright(), QStringLiteral("(the window is upright)"));
    w->setSmallInterface(true);
    settle(700);
    check(w->toolBarArea(w->smallBar()) == Qt::BottomToolBarArea, QStringLiteral("upright: the buttons are in a row at the bottom"));
    bool row = true;
    for (int i = 1; i < kRound.size(); ++i)
        row = row && roundButton(w, kRound[i - 1])->mapTo(w, QPoint(0, 0)).y() == roundButton(w, kRound[i])->mapTo(w, QPoint(0, 0)).y();
    check(row, QStringLiteral("... all on one line"));
    QWidget *area = w->findChild<QWidget *>(QStringLiteral("editorArea"));
    check(area && area->width() >= w->width() - 8, QStringLiteral("the text has the whole width"));
    shot(w, QStringLiteral("s03-upright"));
    smallui::SlidePanel *bp = w->smallPanel(false);
    roundButton(w, QStringLiteral("binder"))->click();
    settle(600);
    const QRect full = w->centralWidget()->geometry();
    check(bp->isOpen() && bp->edge() == smallui::SlidePanel::Edge::Sheet && bp->geometry() == full, QStringLiteral("upright: the binder is a sheet over the whole text area"));
    shot(w, QStringLiteral("s03-sheet"));
    // turned to wide: the buttons move to the edge
    roundButton(w, QStringLiteral("binder"))->click();
    settle(400);
    w->resize(1280, 800);
    settle(700);
    check(!w->isUpright() && w->toolBarArea(w->smallBar()) == Qt::RightToolBarArea, QStringLiteral("turned wide: the buttons stand in a column at the right"));
    w->setSmallInterface(false);
    settle(500);
    finishProject(w);
}


// ================================================================ the kills X-14 to X-16
namespace {
// a sub-run that ends the program by itself at the chosen moment; then a fresh start looks at what is left
void killSelf() { plat::killHard(QCoreApplication::applicationPid()); }

void makeBig(Project *p)
{
    BinderItem *big = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Big"));
    for (int i = 0; i < 80; ++i) {
        BinderItem *d = p->model()->createItem(big, -1, ItemType::Text, QStringLiteral("Part %1").arg(i + 1));
        QTextCursor(p->docs()->text(d->id)).insertText(QStringLiteral("Paragraph of part %1. The lamp burned all night and the sea was loud.").arg(i + 1));
    }
}
}

void caseX14()   // killed during a model load: the next start is clean
{
    using modelmem::ModelMemory;
    const QString v = variant();
    if (v == QLatin1String("load")) {
        const QString path = qEnvironmentVariable("QB_CS_PROJECT");
        MainWindow *w = openWindow(path, 1300, 850);
        if (!w || !w->project())
            return;
        BinderItem *it = textItems(w->project()).value(3);
        QTextCursor(w->project()->docs()->text(it->id)).insertText(QStringLiteral(" Saved before the load X14."));
        QString err;
        w->project()->save(&err);
        QTextCursor(w->project()->docs()->text(it->id)).insertText(QStringLiteral(" Typed during the load X14b."));
        sysmem::setFakeRam({1000 * kMB, 800 * kMB});
        ModelMemory &mm = ModelMemory::instance();
        mm.reset();
        mm.requestLoad(QStringLiteral("t:model"), QStringLiteral("Model"), 50 * kMB, QStringLiteral("ram"), [](const QString &) { return true; });   // the change is under way
        settle(300);
        writeText(qEnvironmentVariable("QB_CS_MARK"), "loading");
        killSelf();
        return;
    }
    if (v == QLatin1String("next")) {
        const QString path = qEnvironmentVariable("QB_CS_PROJECT");
        MainWindow *w = openWindow(path, 1300, 850);
        if (!w || !w->project())
            return;
        const QString text = w->project()->docs()->text(textItems(w->project()).value(3)->id)->toPlainText();
        check(text.contains(QLatin1String("Saved before the load X14")), QStringLiteral("the next start: the text saved before the load is there"), text.right(80));
        check(!ModelMemory::instance().changeActive() && ModelMemory::instance().models().isEmpty(), QStringLiteral("no model change is under way, no model is listed"));
        MessageLine *ml = w->messageLine();
        check(!ml || ml->shownText().isEmpty() || !ml->shownText().contains(QLatin1String("Model change")), QStringLiteral("no \"Model change in progress\" is left on the screen"));
        shot(w, QStringLiteral("x14-next"));
        finishProject(w);
        return;
    }
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    QMap<QString, QString> env;
    env.insert(QStringLiteral("QB_CS_PROJECT"), path);
    env.insert(QStringLiteral("QB_CS_MARK"), ctx().dir + QStringLiteral("/x14-mark"));
    if (ctx().encrypted) {
        env.insert(QStringLiteral("QB_TEST_KEYSTORE"), QStringLiteral("memory"));
    }
    subCase(QStringLiteral("load"), env, nullptr);
    check(readText(ctx().dir + QStringLiteral("/x14-mark")) == QLatin1String("loading"), QStringLiteral("(the program was killed with the model change under way)"));
    QString fails;
    const int rc = subCase(QStringLiteral("next"), env, &fails);
    check(rc == 0, QStringLiteral("the next start (a fresh home) is clean: the project opens, no model change left"), QStringLiteral("rc %1 %2").arg(rc).arg(fails));
    QString err;
    auto p = Project::open(path, &err);
    check(p != nullptr, QStringLiteral("the project opens again here"), err);
}

void caseX15()   // killed while a handwriting insert is saved: the text has the insert or not, never half
{
    const QString v = variant();
    if (v.startsWith(QLatin1String("kill:"))) {
        Hw r;
        if (!hwUp(r))
            return;
        r.hw->setMode(Mode::Editor);
        settle(300);
        QTextCursor c = r.ed->textCursor();
        c.movePosition(QTextCursor::End);
        r.ed->setTextCursor(c);
        r.hw->held();
        tablet(r.canvas, QEvent::TabletPress, canvasPointOf(r.canvas, r.ed, 6));
        tick(50);
        tablet(r.canvas, QEvent::TabletRelease, canvasPointOf(r.canvas, r.ed, 6));
        r.strip->addLine(QImage(30, 20, QImage::Format_RGB32), oneLine({QStringLiteral("HANDWRITTEN"), QStringLiteral("INSERT")}));
        QTest::mouseClick(r.panel->insertButton(), Qt::LeftButton);
        settle(200);
        writeText(qEnvironmentVariable("QB_CS_AFTER"), r.p->docs()->text(r.doc->id)->toPlainText().toUtf8());
        const QStringList a = v.split(QLatin1Char(':'));
        setFaults({QStringLiteral("kill %1 0 %2").arg(QFileInfo(r.p->docs()->textPath(r.doc->id)).absolutePath() + QLatin1Char('/'), a.value(1))});
        QString err;
        r.p->save(&err);
        clearFaults();
        note(QStringLiteral("the save ended without the kill"));
        return;
    }
    if (!QFileInfo::exists(faultLibrary())) {
        check(false, QStringLiteral("(the fault library is built)"));
        return;
    }
    if (ctx().encrypted)
        encryptionSetUp();
    int n = 0, whole = 0, withInsert = 0;
    for (int bytes : {0, 1, 40, 4000}) {
        ++n;
        const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
        QString err;
        QString idOfDoc, before;
        {
            auto p = Project::open(path, &err);
            if (!p)
                continue;
            BinderItem *it = textItems(p.get()).value(3);
            idOfDoc = it->id;
            before = p->docs()->text(it->id)->toPlainText();
        }
        const QString afterFile = ctx().dir + QStringLiteral("/x15-after-%1.txt").arg(n);
        QMap<QString, QString> env = faultEnv(ctx().dir + QStringLiteral("/variant-kill:%1").arg(bytes));
        env.insert(QStringLiteral("QB_CS_PROJECT"), path);
        env.insert(QStringLiteral("QB_CS_AFTER"), afterFile);
        env.insert(QStringLiteral("QB_TEST_KEYSTORE"), QStringLiteral("memory"));
        subCase(QStringLiteral("kill:%1").arg(bytes), env, nullptr);
        const QString after = readText(afterFile);
        auto p2 = Project::open(path, &err);
        check(p2 != nullptr, QStringLiteral("killed after %1 bytes: the project opens").arg(bytes), err);
        if (!p2)
            continue;
        const QString disk = p2->docs()->text(idOfDoc)->toPlainText();
        const bool ok = disk == before || disk == after;
        check(ok && !after.isEmpty(), QStringLiteral("killed after %1 bytes: the text is the old one or the one with the insert, not a mixture").arg(bytes),
              QStringLiteral("disk %1 chars, old %2, new %3").arg(disk.size()).arg(before.size()).arg(after.size()));
        whole += ok ? 1 : 0;
        withInsert += disk.contains(QLatin1String("HANDWRITTEN INSERT")) ? 1 : 0;
    }
    fact(QStringLiteral("X-15 kills / whole texts / with the insert"), QStringLiteral("%1 / %2 / %3").arg(n).arg(whole).arg(withInsert));
}

void caseX16()   // killed during a cancelled compile: no part file left after the next start
{
    const QString v = variant();
    const QString out = qEnvironmentVariable("QB_CS_OUT_DIR");
    if (v == QLatin1String("compile") || v == QLatin1String("compile-next")) {
        const QString path = qEnvironmentVariable("QB_CS_PROJECT");
        MainWindow *w = openWindow(path, 1300, 850);
        if (!w || !w->project())
            return;
        makeBig(w->project());
        const QString docx = out + QStringLiteral("/book.docx");
        CompileDialog dlg(w->project(), w);
        auto *exporter = dlg.findChild<QComboBox *>(QStringLiteral("compileExporter"));
        if (exporter)
            exporter->setCurrentIndex(exporter->findData(QStringLiteral("docx")));
        QString err;
        if (v == QLatin1String("compile-next")) {
            const CompileDialog::CompileResult res = dlg.compileTo(docx, &err);
            check(res == CompileDialog::Compiled && QFileInfo(docx).size() > 1000, QStringLiteral("the next start: a compile into the same folder works"), err);
            return;
        }
        QTimer poll;
        poll.setInterval(1);
        bool done = false;
        QObject::connect(&poll, &QTimer::timeout, [&] {
            if (done)
                return;
            for (QWidget *tw : QApplication::topLevelWidgets()) {
                auto *pd = qobject_cast<QProgressDialog *>(tw);
                if (!pd || pd->objectName() != QLatin1String("compileProgress") || !pd->isVisible())
                    continue;
                const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("\n(\\d+) of (\\d+)$")).match(pd->labelText());
                if (!m.hasMatch() || m.captured(1).toInt() < 5)
                    continue;
                done = true;
                if (QPushButton *b = pd->findChild<QPushButton *>(QStringLiteral("compileCancel")))
                    b->click();
                writeText(qEnvironmentVariable("QB_CS_MARK"), "cancelled");
                killSelf();
            }
        });
        poll.start();
        dlg.compileTo(docx, &err);
        poll.stop();
        note(QStringLiteral("the compile ended without the kill"));
        return;
    }
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    const QString outDir = ctx().work + QStringLiteral("/x16-out");
    QDir(outDir).removeRecursively();
    QDir().mkpath(outDir);
    writeText(outDir + QStringLiteral("/book.docx"), "OLD");
    QMap<QString, QString> env;
    env.insert(QStringLiteral("QB_CS_PROJECT"), path);
    env.insert(QStringLiteral("QB_CS_OUT_DIR"), outDir);
    env.insert(QStringLiteral("QB_CS_MARK"), ctx().dir + QStringLiteral("/x16-mark"));
    env.insert(QStringLiteral("QB_TEST_KEYSTORE"), QStringLiteral("memory"));
    subCase(QStringLiteral("compile"), env, nullptr);
    check(readText(ctx().dir + QStringLiteral("/x16-mark")) == QLatin1String("cancelled"), QStringLiteral("(the program was killed right after Cancel)"));
    check(readText(outDir + QStringLiteral("/book.docx")) == QLatin1String("OLD"), QStringLiteral("the older file of that name is untouched"));
    QStringList left = QDir(outDir).entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
    left.removeAll(QStringLiteral("book.docx"));
    fact(QStringLiteral("X-16 files the kill left in the folder"), left.join(QLatin1String(", ")));
    // a part file as an earlier export that was killed in the middle of writing leaves it
    writeText(outDir + QStringLiteral("/book.docx.part"), "half");
    {
        QFile part(outDir + QStringLiteral("/book.docx.part"));   // the program removes a part file once it is an hour old
        if (part.open(QIODevice::ReadWrite))
            part.setFileTime(QDateTime::currentDateTime().addSecs(-7200), QFileDevice::FileModificationTime);
    }
    QString fails;
    const int rc = subCase(QStringLiteral("compile-next"), env, &fails);
    check(rc == 0, QStringLiteral("the next start compiles into the folder"), QStringLiteral("rc %1 %2").arg(rc).arg(fails));
    QStringList after = QDir(outDir).entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
    check(after == QStringList{QStringLiteral("book.docx")}, QStringLiteral("no part file is left after the next start"), after.join(QLatin1String(", ")));
    check(QFileInfo(outDir + QStringLiteral("/book.docx")).size() > 1000, QStringLiteral("... and the file is the new, whole one"));
}

} // namespace cs
