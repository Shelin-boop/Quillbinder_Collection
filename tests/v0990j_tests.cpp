// Checks of 0.99.0 (item 112, handwriting, part 1: everything but the recognizer): the button with its four looks, the modes and
// the one writing area (inspector pad, see-through layer over the editor, both), writing versus pointing (pen, finger, mouse),
// the hand protection, scratching out, the ink's stay and fade, the strip (alternatives, writing over a word, Insert, Pin), the
// grid and its slider, File > Options > Handwriting, the defaults per system. A fake recognizer returns a fixed text per line.
//
//   xvfb-run -a ./qb_v0990j_tests /tmp/shots

#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/ItemIcons.h"
#include "core/ModelMemory.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/crypto/Vault.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/EditorPane.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/OptionsDialog.h"
#include "ui/PenInput.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"
#include "ui/handwriting/HandwritingController.h"
#include "ui/handwriting/HandwritingPage.h"
#include "ui/handwriting/HandwritingPanel.h"
#include "ui/handwriting/HandwritingSettings.h"
#include "ui/handwriting/InkCanvas.h"
#include "ui/handwriting/LearningStore.h"
#include "ui/handwriting/Recognizer.h"
#include "ui/handwriting/WordStrip.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDir>
#include <QElapsedTimer>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QStyleFactory>
#include <QTabletEvent>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTouchEvent>
#include <cstdio>

using namespace qb;
using namespace qb::handwriting;

static int failures = 0;
static QString shotDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(1500)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 200)
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
    w->grab().save(shotDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

// ---- the clock of the checks (PenInput's), in milliseconds
static qint64 clockMs = 1000000;
static void tick(qint64 ms)
{
    clockMs += ms;
}

// ---- synthetic input
static QPointingDevice *penDevice()
{
    static QPointingDevice *d = new QPointingDevice(QStringLiteral("test-pen"), 11, QInputDevice::DeviceType::Stylus,
                                                    QPointingDevice::PointerType::Pen,
                                                    QInputDevice::Capability::Position | QInputDevice::Capability::Pressure, 1, 2);
    return d;
}
static QPointingDevice *touchDevice()
{
    static QPointingDevice *d = new QPointingDevice(QStringLiteral("test-touch"), 12, QInputDevice::DeviceType::TouchScreen,
                                                    QPointingDevice::PointerType::Finger,
                                                    QInputDevice::Capability::Position | QInputDevice::Capability::Area, 5, 0);
    return d;
}

static void tablet(InkCanvas *c, QEvent::Type type, const QPointF &p, bool side = false, bool tipDown = true)
{
    Qt::MouseButtons buttons = tipDown ? Qt::LeftButton : Qt::NoButton;
    if (side)
        buttons |= Qt::RightButton;
    const Qt::MouseButton button = type == QEvent::TabletMove ? Qt::NoButton : Qt::LeftButton;
    if (type == QEvent::TabletRelease)
        buttons = side ? Qt::RightButton : Qt::NoButton;
    QTabletEvent e(type, penDevice(), p, c->mapToGlobal(p.toPoint()), tipDown ? 0.6 : 0.0, 0, 0, 0, 0, 0, Qt::NoModifier, button, buttons);
    QApplication::sendEvent(c, &e);
}

// a pen stroke through the points (a move between them); `step` ms pass between the events
static void penStroke(InkCanvas *c, const QList<QPointF> &pts, bool side = false, int step = 10)
{
    tablet(c, QEvent::TabletPress, pts.first(), side);
    for (int i = 1; i < pts.size(); ++i) {
        tick(step);
        tablet(c, QEvent::TabletMove, pts[i], side);
    }
    tick(step);
    tablet(c, QEvent::TabletRelease, pts.last(), side);
}

static QList<QPointF> line(QPointF a, QPointF b, int n = 8)
{
    QList<QPointF> v;
    for (int i = 0; i <= n; ++i)
        v << a + (b - a) * (double(i) / n);
    return v;
}

// a wavy line that goes forward (handwriting, not a scratch)
static QList<QPointF> wave(QPointF a, double w, int n = 24)
{
    QList<QPointF> v;
    for (int i = 0; i <= n; ++i)
        v << QPointF(a.x() + w * i / n, a.y() + 8 * std::sin(i * 0.9));
    return v;
}

// a scratch: back and forth over `w` pixels, `sweeps` times
static QList<QPointF> scratch(QPointF a, double w, int sweeps = 6)
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

static void touch(InkCanvas *c, QEvent::Type type, const QList<std::pair<int, QPointF>> &pts, const QList<QEventPoint::State> &states)
{
    QList<QEventPoint> points;
    for (int i = 0; i < pts.size(); ++i) {
        const QPointF scene = QPointF(c->mapTo(c->window(), QPoint(0, 0))) + pts[i].second;
        points << QEventPoint(pts[i].first, states[i], scene, QPointF(c->mapToGlobal(pts[i].second.toPoint())));
    }
    QTouchEvent e(type, touchDevice(), Qt::NoModifier, points);
    QApplication::sendEvent(c, &e);
}

static void finger(InkCanvas *c, int id, const QList<QPointF> &pts, int step = 10, bool release = true)
{
    touch(c, QEvent::TouchBegin, {{id, pts.first()}}, {QEventPoint::Pressed});
    for (int i = 1; i < pts.size(); ++i) {
        tick(step);
        touch(c, QEvent::TouchUpdate, {{id, pts[i]}}, {QEventPoint::Updated});
    }
    tick(step);
    if (release)
        touch(c, QEvent::TouchEnd, {{id, pts.last()}}, {QEventPoint::Released});
}

static QPointF canvasPointOf(InkCanvas *c, TextEditor *ed, int position)
{
    QTextCursor tc(ed->document());
    tc.setPosition(position);
    const QRect r = ed->cursorRect(tc);
    return QPointF(c->mapFromGlobal(ed->viewport()->mapToGlobal(r.center())));
}

static int penUpFor(PenInput &pen)
{
    PenInput::Sample s;
    pen.feed(s);
    return 0;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990jTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990jTest"));
    QApplication app(argc, argv);
    shotDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(shotDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.welcomeShown = true;
    as.spellCheck = false;
    as.backupOnClose = false;
    as.save();
    theme::apply();

    PenInput &pen = PenInput::instance();
    pen.setClock([] { return clockMs; });

    // ================================================================ defaults per system
    std::printf("== the defaults per system\n");
    {
        const HandwritingSettings desk = HandwritingSettings::defaults(false);
        const HandwritingSettings android = HandwritingSettings::defaults(true);
        check(!desk.buttonShown && desk.mode == Mode::Off, QStringLiteral("Linux and Windows: the button is hidden and the mode is off"));
        check(android.buttonShown, QStringLiteral("Android: the button is shown by default"));
        HandwritingConfig::forceAndroid(1);
        QSettings().remove(QStringLiteral("handwriting"));
        HandwritingConfig::instance().reload();
        check(HandwritingConfig::instance().get().buttonShown, QStringLiteral("a device with no settings of its own, forced to Android, shows the button"));
        HandwritingConfig::forceAndroid(0);
        QSettings().remove(QStringLiteral("handwriting"));
        HandwritingConfig::instance().reload();
        check(!HandwritingConfig::instance().get().buttonShown, QStringLiteral("forced to a desktop: hidden"));
        HandwritingConfig::forceAndroid(-1);
    }

    // ================================================================ the plain parts
    std::printf("== scratching out, hand protection and the pen (plain parts)\n");
    {
        QVector<QPointF> sc, wv;
        for (const QPointF &p : scratch(QPointF(10, 50), 90))
            sc << p;
        for (const QPointF &p : wave(QPointF(10, 50), 200))
            wv << p;
        QRectF area;
        check(isScratchOut(sc, 30.0, &area) && area.width() > 80, QStringLiteral("a back-and-forth stroke is a scratch-out"));
        check(!isScratchOut(wv, 30.0), QStringLiteral("a stroke that goes forward is not"));
        QVector<QPointF> dot{QPointF(5, 5), QPointF(5, 5)};
        check(!isScratchOut(dot, 30.0), QStringLiteral("a dot is not"));

        HandProtection::Params pr;
        pr.pxPerMm = 4.0;
        HandProtection right(pr);
        pr.leftHanded = true;
        HandProtection left(pr);
        const QRectF wa(200, 200, 100, 40);
        const QRectF zr = right.restZone(wa), zl = left.restZone(wa);
        check(qAbs((wa.left() - zr.left()) - 30 * 4.0) < 1 && qAbs((wa.top() - zr.top()) - 30 * 4.0) < 1,
              QStringLiteral("the rest zone is the writing area plus about 30 mm"));
        check(zr.right() - wa.right() > wa.left() - zr.left() && zr.bottom() - wa.bottom() > wa.top() - zr.top(),
              QStringLiteral("it is wider on the writing hand's side (right) and below"));
        check(wa.left() - zl.left() > zl.right() - wa.right() - 1 && zl.bottom() - wa.bottom() > wa.top() - zl.top(),
              QStringLiteral("left-handed: wider on the left and below"));
        HandProtection::Situation sit;
        sit.penWriting = true;
        sit.penInUse = true;
        sit.hasWritingArea = true;
        sit.writingArea = wa;
        HandProtection::Touch palm;
        palm.pos = QPointF(260, 330);
        check(right.ignores(palm, sit), QStringLiteral("a finger below the writing is ignored while the pen writes"));
        palm.pos = QPointF(1500, 900);
        check(!right.ignores(palm, sit), QStringLiteral("a finger far away is accepted"));
        HandProtection::Touch tapButton;
        tapButton.pos = QPointF(260, 330);
        tapButton.onButton = true;
        tapButton.shortSmallTap = true;
        check(!right.ignores(tapButton, sit), QStringLiteral("a short, small tap on a button is accepted inside the zone"));
        tapButton.shortSmallTap = false;
        check(right.ignores(tapButton, sit), QStringLiteral("a long touch on a button inside the zone is not"));
        HandProtection::Touch big;
        big.pos = QPointF(1500, 900);
        big.contactMm = 20;
        check(right.ignores(big, sit), QStringLiteral("a large contact is ignored while the pen is used"));
        sit.penWriting = false;
        check(right.ignores(big, sit), QStringLiteral("... also a little after the pen lifted (the pen is still in use)"));
        sit.penInUse = false;
        sit.surface = QRectF(0, 0, 1000, 800);
        HandProtection::Touch edge;
        edge.pos = QPointF(10, 400);
        check(right.ignores(edge, sit), QStringLiteral("without a pen, a touch in the strip of about 15 mm at the edge is ignored"));
        edge.pos = QPointF(500, 400);
        check(!right.ignores(edge, sit), QStringLiteral("... one in the middle is not"));

        PenInput own;
        qint64 t = 5000;
        own.setClock([&] { return t; });
        PenInput::Sample s;
        s.inRange = true;
        s.tipDown = true;
        own.feed(s);
        check(own.tipDown() && own.wroteWithin(1000) && own.inUse(1000), QStringLiteral("PenInput: the tip is down: writing and in use"));
        s.tipDown = false;
        t += 400;
        own.feed(s);
        t += 500;
        check(own.wroteWithin(1000) && !own.wroteWithin(300), QStringLiteral("PenInput: it wrote within the last second, not within 0.3 s"));
        s.inRange = false;
        own.feed(s);
        t += 5000;
        check(!own.inUse(1000) && own.inUse(10000), QStringLiteral("PenInput: out of range, it counts as in use only for a while"));
    }

    // ================================================================ the window
    TextEditor *ed = nullptr;
    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("The Lighthouse"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    Project *p = proj.get();
    BinderItem *ch = p->model()->createItem(p->draft(), -1, ItemType::Folder, QStringLiteral("Chapter One"));
    BinderItem *d1 = p->model()->createItem(ch, -1, ItemType::Text, QStringLiteral("The Storm"));
    {
        QTextCursor c(p->docs()->text(d1->id));
        c.insertText(QStringLiteral("The wind rose over the calm grey sea and the gulls cried out above the lamp."));
        for (int i = 0; i < 70; ++i) {
            c.insertBlock();
            c.insertText(QStringLiteral("Line %1 of the long night watch, the keeper counted the waves again.").arg(i));
        }
    }

    FakeRecognizer fake;
    fake.setLines({QStringLiteral("hello world")});
    fake.setAlternatives(QStringLiteral("hello"), {QStringLiteral("hallo"), QStringLiteral("jello")});
    Recognizer::setCurrent(&fake);
    QSettings().remove(QStringLiteral("handwriting"));
    HandwritingConfig::instance().reload();

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.move(0, 0);
    w.resize(1500, 900);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);
    auto *binder = w.findChild<BinderPanel *>();
    auto *inspector = w.findChild<Inspector *>();
    HandwritingController *hw = w.handwriting();
    check(binder && inspector && hw, QStringLiteral("(window parts found)"));
    if (!binder || !inspector || !hw)
        return 1;
    binder->select({d1}, true);
    settle(300);
    ed = w.handwritingEditor();
    check(ed != nullptr, QStringLiteral("(the editor shows the document)"));
    if (!ed)
        return 1;
    InkCanvas *canvas = hw->canvas();
    HandwritingPanel *panel = hw->panel();
    WordStrip *strip = panel->strip();

    // short times, so that the checks run fast; the clock is ours
    auto setTimes = [](int pause, int stay, int fade) {
        HandwritingSettings s = HandwritingConfig::instance().get();
        s.pauseMs = pause;
        s.stayMs = stay;
        s.fadeMs = fade;
        HandwritingConfig::instance().set(s);
    };
    setTimes(300, 400, 400);
    auto readAndFade = [&] {   // the pause passes, the line is read, the ink stays and fades away
        tick(500);
        canvas->advance();
        tick(500);
        canvas->advance();
        tick(500);
        canvas->advance();
        tick(1000);
        canvas->advance();
    };

    // ================================================================ the button
    std::printf("== the button\n");
    QToolButton *button = w.findChild<QToolButton *>(QStringLiteral("handwritingButton"));
    check(button != nullptr, QStringLiteral("the button exists on the toolbar"));
    check(button && !button->isVisible() && !hw->buttonShown(), QStringLiteral("off by default on Linux: the button is hidden"));
    QAction *show = w.findChild<QAction *>(QStringLiteral("showHandwritingButton"));
    check(show && !show->isChecked(), QStringLiteral("View > Toolbars > Handwriting Button is there and unchecked"));
    {
        QMenu *barsMenu = nullptr;
        for (QAction *top : w.menuBar()->actions())
            if (top->text().remove(QLatin1Char('&')) == QLatin1String("View") && top->menu())
                for (QAction *sub : top->menu()->actions())
                    if (sub->menu() && sub->text().remove(QLatin1Char('&')) == QLatin1String("Toolbars"))
                        barsMenu = sub->menu();
        check(barsMenu && barsMenu->actions().contains(show), QStringLiteral("the switch is in View > Toolbars"));
    }
    if (show)
        show->setChecked(true);
    settle(300);
    check(button && button->isVisible() && hw->buttonShown(), QStringLiteral("switched on in View > Toolbars: the button shows"));
    check(button && !button->icon().pixmap(22, 22).isNull(), QStringLiteral("the button has an icon"));
    {
        QSet<QByteArray> looks;
        bool all = true;
        for (const char *n : {"handwriting-off", "handwriting-editor", "handwriting-inspector", "handwriting-both"}) {
            const QImage img = icons::action(QString::fromLatin1(n)).pixmap(22, 22).toImage();
            bool ink = false;
            for (int y = 0; y < img.height() && !ink; ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qAlpha(img.pixel(x, y)) > 40) {
                        ink = true;
                        break;
                    }
            all = all && ink;
            looks << QByteArray(reinterpret_cast<const char *>(img.constBits()), qsizetype(img.sizeInBytes()));
        }
        check(all && looks.size() == 4, QStringLiteral("the four looks (off, editor, inspector, both) are drawn and differ"));
        QImage sheet(4 * 66, 66, QImage::Format_ARGB32);
        sheet.fill(Qt::white);
        QPainter sp(&sheet);
        int x = 0;
        for (const char *n : {"handwriting-off", "handwriting-editor", "handwriting-inspector", "handwriting-both"}) {
            sp.drawPixmap(x + 1, 1, icons::action(QString::fromLatin1(n)).pixmap(64, 64));
            x += 66;
        }
        sp.end();
        sheet.save(shotDir + QLatin1String("/hw-icons.png"));
    }

    // a tap switches between off and the last mode; the first use (no recognizer) opens the settings
    Recognizer::setCurrent(nullptr);
    {
        const int before = hw->settingsRequests();
        QTimer::singleShot(300, [] {
            if (auto *d = qobject_cast<OptionsDialog *>(QApplication::activeModalWidget()))
                d->reject();
        });
        QTest::mouseClick(button, Qt::LeftButton);
        settle(100);
        check(hw->mode() == Mode::Off && hw->settingsRequests() == before + 1,
              QStringLiteral("no recognizer installed: a tap opens the settings and the mode stays off"));
    }
    // the real window: File > Options > Handwriting, with the install slot
    {
        bool shownPage = false, slot = false, installDisabled = false, ruGrey = false;
        QString ruText;
        QTimer::singleShot(400, [&] {
            auto *d = qobject_cast<OptionsDialog *>(QApplication::activeModalWidget());
            if (!d)
                return;
            auto *page = d->findChild<HandwritingPage *>();
            shownPage = page && page->isVisible();
            if (page) {
                slot = page->findChild<QWidget *>(QStringLiteral("hwInstallSlot")) != nullptr;
                auto *inst = page->findChild<QPushButton *>(QStringLiteral("hwInstall"));
                installDisabled = inst && inst->text() == QLatin1String("Install") && !inst->isEnabled();
                auto *main = page->findChild<QComboBox *>(QStringLiteral("hwMainLanguage"));
                const int ru = main ? main->findData(QStringLiteral("ru")) : -1;
                if (ru >= 0) {
                    ruText = main->itemText(ru);
                    ruGrey = !(main->model()->flags(main->model()->index(ru, 0)) & Qt::ItemIsEnabled);
                }
                shot(d, QStringLiteral("hw-options-page"));
            }
            d->reject();
        });
        hw->setMode(Mode::Both);   // still no recognizer: opens File > Options > Handwriting
        check(shownPage, QStringLiteral("the first use opens File > Options > Handwriting"));
        check(slot && installDisabled, QStringLiteral("the install slot with its Install button is there (no installer connected yet)"));
        check(ruGrey && ruText.contains(QLatin1String("no model for this language yet")),
              QStringLiteral("Russian is greyed out with \"no model for this language yet\""), ruText);
        check(hw->mode() == Mode::Off, QStringLiteral("... and the mode is still off"));
    }
    // an installer that is connected: the button calls it
    {
        bool called = false;
        Recognizer::Installer inst;
        inst.install = [&](QWidget *) { called = true; };
        inst.status = [] { return QStringLiteral("Not installed (test)"); };
        Recognizer::setInstaller(inst);
        OptionsDialog dlg(&w);
        dlg.showSection(OptionsDialog::SGeneral, QStringLiteral("Handwriting"));
        dlg.show();
        settle(200);
        auto *page = dlg.findChild<HandwritingPage *>();
        auto *ib = page ? page->findChild<QPushButton *>(QStringLiteral("hwInstall")) : nullptr;
        check(ib && ib->isEnabled(), QStringLiteral("with an installer set, the Install button is enabled"));
        if (ib)
            ib->click();
        check(called, QStringLiteral("... and it calls the installer's function"));
        Recognizer::setInstaller({});
    }
    Recognizer::setCurrent(&fake);

    QTest::mouseClick(button, Qt::LeftButton);
    settle(200);
    check(hw->mode() == Mode::Inspector, QStringLiteral("a tap switches the mode on (the mode used last: Inspector the first time)"));
    check(button->icon().cacheKey() != 0 && hw->button()->toolTip().contains(QLatin1String("Inspector")), QStringLiteral("the button shows the mode"));
    QTest::mouseClick(button, Qt::LeftButton);
    settle(100);
    check(hw->mode() == Mode::Off, QStringLiteral("a second tap switches it off"));
    hw->setMode(Mode::Editor);
    hw->setMode(Mode::Off);
    QTest::mouseClick(button, Qt::LeftButton);
    settle(100);
    check(hw->mode() == Mode::Editor, QStringLiteral("a tap goes back to the last mode used (Editor)"));
    QTest::mouseClick(button, Qt::LeftButton);
    settle(100);

    // a long press opens the four choices
    QTest::mousePress(button, Qt::LeftButton);
    QTest::qWait(HandwritingButton::kHoldMs + 250);
    QMenu *choices = w.findChild<QMenu *>(QStringLiteral("handwritingChoices"));
    QStringList names;
    if (choices)
        for (QAction *a : choices->actions())
            names << a->text();
    check(choices && choices->isVisible() && names == QStringList({QStringLiteral("Off"), QStringLiteral("Editor"), QStringLiteral("Inspector"),
                                                                    QStringLiteral("Both")}),
          QStringLiteral("a long press opens the four choices: Off, Editor, Inspector, Both"), names.join(QLatin1Char(',')));
    QTest::mouseRelease(button, Qt::LeftButton);
    check(hw->mode() == Mode::Off, QStringLiteral("... and the release of a long press is no tap"));
    if (choices)
        choices->close();
    settle(100);
    {
        QContextMenuEvent ce(QContextMenuEvent::Mouse, QPoint(5, 5), button->mapToGlobal(QPoint(5, 5)));
        QApplication::sendEvent(button, &ce);
        settle(100);
        choices = w.findChild<QMenu *>(QStringLiteral("handwritingChoices"));
        check(choices && choices->isVisible(), QStringLiteral("a right-click (press-and-hold) opens the choices too"));
        QAction *both = choices ? choices->findChild<QAction *>(QStringLiteral("hwChoice3")) : nullptr;
        if (both)
            both->trigger();
        settle(200);
        check(hw->mode() == Mode::Both, QStringLiteral("choosing Both from the menu switches to it"));
        hw->setMode(Mode::Off);
    }

    // ================================================================ the modes and the one writing area
    std::printf("== the modes\n");
    // the inspector is closed: a mode that needs it opens it
    QAction *showInspector = w.findChild<QAction *>(QStringLiteral("showInspector"));
    if (showInspector)
        showInspector->setChecked(false);
    settle(200);
    hw->setMode(Mode::Inspector);
    settle(300);
    check(inspector->isVisible() && inspector->currentExtraTab() == QLatin1String("handwriting"),
          QStringLiteral("Inspector mode: the inspector opens with the new tab \"Handwriting\""));
    check(inspector->extraTabToolTip(QStringLiteral("handwriting")) == QLatin1String("Handwriting"), QStringLiteral("the tab is named Handwriting"));
    check(canvas->parentWidget() == panel->padHolder() && canvas->role() == InkCanvas::Role::Pad && canvas->isVisible(),
          QStringLiteral("the one writing area is the pad in the tab"));
    check(canvas->gridRect().width() > 100 && canvas->gridRect().height() > 100, QStringLiteral("the pad has the grid over all of it"));
    shot(&w, QStringLiteral("hw-mode-inspector"));

    // writing a line in the pad: ruled rows are the lines of the recognizer
    {
        fake.setLines({QStringLiteral("hello world")});
        setTimes(2000, 400, 400);
        const int req = fake.requests();
        const double pitch = HandwritingConfig::instance().get().grid.pitch();
        const QPointF a(30, pitch * 1.2), b(30, pitch * 1.2 + 6);
        penStroke(canvas, wave(a, 120));
        check(canvas->pendingLines() == 1 && canvas->strokeCount() == 1, QStringLiteral("a pen stroke in the pad is ink, one line waiting"));
        penStroke(canvas, wave(QPointF(170, pitch * 1.2), 100));
        check(canvas->pendingLines() == 1 && canvas->strokeCount() == 2, QStringLiteral("a second stroke on the same ruled row joins its line"));
        penStroke(canvas, wave(QPointF(30, pitch * 3.3), 100));
        check(canvas->pendingLines() == 2, QStringLiteral("a stroke on another ruled row is another line"));
        Q_UNUSED(b)
        tick(100);
        canvas->advance();
        check(fake.requests() == req, QStringLiteral("the line is not read before the pause is over"));
        tick(2000);
        canvas->advance();
        setTimes(300, 400, 400);
        check(fake.requests() == req + 2 && canvas->pendingLines() == 0, QStringLiteral("after the pause each ruled line is read (2 requests)"));
        check(!fake.lastRequest().picture.isNull() && !fake.lastRequest().strokes.isEmpty() && fake.lastRequest().languages.first() == QLatin1String("en"),
              QStringLiteral("the request holds the strokes, the picture and the languages"));
        canvas->clearInk();
        strip->clear();
    }

    // the fade timing with short test values: stay 400 ms, fade 400 ms
    {
        fake.setLines({QStringLiteral("hello world")});
        penStroke(canvas, wave(QPointF(30, 70), 160));
        tick(350);
        canvas->advance();   // read now
        const qint64 readAt = clockMs;
        check(canvas->lineState(0) == QLatin1String("shown") && canvas->lineAlpha(0) == 1.0 && strip->isEmpty(),
              QStringLiteral("read right after the pause: the ink is full, the strip still empty"));
        tick(200);
        canvas->advance();
        check(canvas->lineAlpha(0) == 1.0 && strip->isEmpty(), QStringLiteral("the ink stays for the set time (200 of 400 ms)"));
        tick(400);   // 600 ms after the reading: 200 ms into the fade
        canvas->advance();
        const double a = canvas->lineAlpha(0);
        check(a > 0.3 && a < 0.7, QStringLiteral("then it fades (halfway through the fade the ink is about half)"), QString::number(a));
        check(strip->text() == QLatin1String("hello world"), QStringLiteral("as it fades the read text appears in the strip"), strip->text());
        check(strip->tokenAlpha(0) > 0.3 && strip->tokenAlpha(0) < 0.7, QStringLiteral("the text appears with the fade"), QString::number(strip->tokenAlpha(0)));
        tick(300);
        canvas->advance();
        check(canvas->visibleLines() == 0 && strip->tokenAlpha(0) == 1.0, QStringLiteral("the ink is gone after the fade and the text is full"));
        Q_UNUSED(readAt)
        check(w.handwritingEditor()->toPlainText().contains(QLatin1String("hello world")) == false,
              QStringLiteral("no direct insertion: the document did not get the text"));
        shot(&w, QStringLiteral("hw-strip-after-fade"));
    }

    // Space, Backspace and Enter as buttons
    {
        strip->clear();
        strip->addLine(QImage(20, 20, QImage::Format_RGB32), [] {
            Result r;
            r.ok = true;
            Word a;
            a.text = QStringLiteral("hello");
            r.words << a;
            return r;
        }());
        QTest::mouseClick(panel->spaceButton(), Qt::LeftButton);
        check(strip->text() == QLatin1String("hello "), QStringLiteral("the Space button adds a space"), strip->text());
        QTest::mouseClick(panel->enterButton(), Qt::LeftButton);
        check(strip->text() == QLatin1String("hello \n"), QStringLiteral("the Enter button adds a line break"), strip->text());
        QTest::mouseClick(panel->backspaceButton(), Qt::LeftButton);
        QTest::mouseClick(panel->backspaceButton(), Qt::LeftButton);
        check(strip->text() == QLatin1String("hello"), QStringLiteral("the Backspace button takes the last of them back"), strip->text());
        QTest::mouseClick(panel->backspaceButton(), Qt::LeftButton);
        check(strip->text() == QLatin1String("hell"), QStringLiteral("... and then the last character of the word"), strip->text());
        QTest::mouseClick(panel->clearButton(), Qt::LeftButton);
        check(strip->isEmpty(), QStringLiteral("Clear empties the strip"));
    }

    // ---- editor mode
    hw->setMode(Mode::Editor);
    settle(400);
    check(canvas->parentWidget() == &w && canvas->role() == InkCanvas::Role::Overlay && canvas->isVisible(),
          QStringLiteral("Editor mode: the writing area is a see-through layer over the editor"));
    {
        const QRect ea(w.editorAreaWidget()->mapTo(&w, QPoint(0, 0)), w.editorAreaWidget()->size());
        check(canvas->geometry().contains(ea) && canvas->geometry().size() == ea.size(), QStringLiteral("the layer covers the editor area exactly"),
              QStringLiteral("%1 vs %2").arg(canvas->geometry().width()).arg(ea.width()));
        check(canvas->gridRect().isNull(), QStringLiteral("no grid over the editor's text"));
        check(inspector->isVisible() && inspector->currentExtraTab() == QLatin1String("handwriting"),
              QStringLiteral("the inspector is open with the strip (also in editor mode)"));
        check(!canvas->mask().isEmpty() && canvas->mask().contains(QPoint(10, 10)), QStringLiteral("the layer is clipped to the editor"));
    }
    shot(&w, QStringLiteral("hw-mode-editor"));
    InkCanvas::scrollBarFor(ed)->setValue(0);
    settle(100);
    {
        // a stroke over the text is ink; the text under it stays
        const QString textBefore = ed->toPlainText();
        const QPointF s = canvasPointOf(canvas, ed, 4);
        fake.setLines({QStringLiteral("hello world")});
        penStroke(canvas, line(s + QPointF(0, 40), s + QPointF(180, 55), 14));
        check(canvas->strokeCount() == 1 && ed->toPlainText() == textBefore, QStringLiteral("a pen stroke over the editor's text is ink; the text is untouched"));
        readAndFade();
        check(strip->text() == QLatin1String("hello world"), QStringLiteral("the line was read; its text is in the strip"), strip->text());
        strip->clear();
    }

    // a short pen tap places the cursor, the side button selects
    {
        const int target = 20;
        const QPointF tp = canvasPointOf(canvas, ed, target);
        const int placedBefore = canvas->placedCursors();
        tablet(canvas, QEvent::TabletPress, tp);
        tick(60);
        tablet(canvas, QEvent::TabletRelease, tp);
        check(canvas->strokeCount() == 0 && canvas->placedCursors() == placedBefore + 1, QStringLiteral("a short pen tap leaves no ink and places the cursor"));
        check(qAbs(ed->textCursor().position() - target) <= 1 && !ed->textCursor().hasSelection(), QStringLiteral("... at the place tapped"),
              QString::number(ed->textCursor().position()));
        check(hw->held().position() == ed->textCursor().position(), QStringLiteral("the place is held as a text cursor"));

        const QPointF from = canvasPointOf(canvas, ed, 4), to = canvasPointOf(canvas, ed, 22);
        tablet(canvas, QEvent::TabletPress, from, true);
        for (int i = 1; i <= 6; ++i) {
            tick(10);
            tablet(canvas, QEvent::TabletMove, from + (to - from) * (i / 6.0), true);
        }
        tablet(canvas, QEvent::TabletRelease, to, true);
        const QString sel = ed->textCursor().selectedText();
        check(ed->textCursor().hasSelection() && sel.size() >= 14 && canvas->strokeCount() == 0,
              QStringLiteral("the pen's side button selects text and writes nothing"), sel);
        shot(&w, QStringLiteral("hw-editor-selected"));
        QTextCursor c = ed->textCursor();
        c.clearSelection();
        ed->setTextCursor(c);
    }

    // fingers with a pen: they scroll and tap
    {
        PenInput::Sample s;
        s.inRange = true;
        pen.feed(s);   // the pen hovers: it is in use
        tick(3000);
        InkCanvas::scrollBarFor(ed)->setValue(0);
        const int before = InkCanvas::scrollBarFor(ed)->value();
        finger(canvas, 1, line(QPointF(300, 500), QPointF(300, 300), 8));
        check(InkCanvas::scrollBarFor(ed)->value() > before && canvas->strokeCount() == 0,
              QStringLiteral("with a pen, a finger scrolls (and writes nothing)"), QString::number(InkCanvas::scrollBarFor(ed)->value()));
        const int placed = canvas->placedCursors();
        const QPointF tp = canvasPointOf(canvas, ed, 10);
        finger(canvas, 2, {tp, tp});
        check(canvas->placedCursors() == placed + 1 && canvas->strokeCount() == 0, QStringLiteral("with a pen, a finger tap places the cursor"));
        InkCanvas::scrollBarFor(ed)->setValue(0);
        penUpFor(pen);
        tick(40000);   // the pen is long gone
    }

    // without a pen: one finger writes, a tap places the cursor, a long press and drag selects, two fingers scroll
    {
        check(!pen.inUse(30000), QStringLiteral("(no pen in use any more)"));
        InkCanvas::scrollBarFor(ed)->setValue(0);
        settle(60);
        const QPointF s = canvasPointOf(canvas, ed, 4);
        finger(canvas, 3, line(s + QPointF(0, 40), s + QPointF(150, 52), 12));
        check(canvas->strokeCount() == 1, QStringLiteral("without a pen one finger writes"));
        canvas->clearInk();
        const int placed = canvas->placedCursors();
        const QPointF tp = canvasPointOf(canvas, ed, 12);
        finger(canvas, 4, {tp, tp});
        check(canvas->placedCursors() == placed + 1 && canvas->strokeCount() == 0, QStringLiteral("a finger tap places the cursor"));

        // long press, then drag: selects
        const QPointF a = canvasPointOf(canvas, ed, 4), b = canvasPointOf(canvas, ed, 24);
        touch(canvas, QEvent::TouchBegin, {{5, a}}, {QEventPoint::Pressed});
        tick(600);
        canvas->advance();
        check(canvas->pointerAction(5) == InkCanvas::Action::Select, QStringLiteral("a long press turns into selecting"));
        for (int i = 1; i <= 6; ++i) {
            tick(10);
            touch(canvas, QEvent::TouchUpdate, {{5, a + (b - a) * (i / 6.0)}}, {QEventPoint::Updated});
        }
        touch(canvas, QEvent::TouchEnd, {{5, b}}, {QEventPoint::Released});
        check(ed->textCursor().hasSelection() && canvas->strokeCount() == 0, QStringLiteral("... and the drag selects text"), ed->textCursor().selectedText());
        QTextCursor c = ed->textCursor();
        c.clearSelection();
        ed->setTextCursor(c);

        // two fingers scroll
        const int before = InkCanvas::scrollBarFor(ed)->value();
        touch(canvas, QEvent::TouchBegin, {{6, QPointF(300, 500)}}, {QEventPoint::Pressed});
        tick(10);
        touch(canvas, QEvent::TouchUpdate, {{6, QPointF(300, 500)}, {7, QPointF(340, 500)}}, {QEventPoint::Stationary, QEventPoint::Pressed});
        for (int i = 1; i <= 8; ++i) {
            tick(10);
            touch(canvas, QEvent::TouchUpdate, {{6, QPointF(300, 500 - 25 * i)}, {7, QPointF(340, 500 - 25 * i)}},
                  {QEventPoint::Updated, QEventPoint::Updated});
        }
        touch(canvas, QEvent::TouchEnd, {{6, QPointF(300, 300)}, {7, QPointF(340, 300)}}, {QEventPoint::Released, QEventPoint::Released});
        check(InkCanvas::scrollBarFor(ed)->value() > before && canvas->strokeCount() == 0,
              QStringLiteral("two fingers scroll (and the first finger's dot of ink is dropped)"), QString::number(InkCanvas::scrollBarFor(ed)->value()));
        InkCanvas::scrollBarFor(ed)->setValue(0);
    }

    // the rest zone while the pen writes
    {
        canvas->clearInk();
        const QPointF s = canvasPointOf(canvas, ed, 4) + QPointF(0, 40);
        tablet(canvas, QEvent::TabletPress, s);
        for (int i = 1; i <= 6; ++i) {
            tick(10);
            tablet(canvas, QEvent::TabletMove, s + QPointF(i * 20, 4));
        }
        const int ignored = canvas->ignoredTouches();
        const QRectF zone = canvas->restZone();
        check(!zone.isNull() && zone.contains(s + QPointF(100, 150)), QStringLiteral("the pen writes: the rest zone surrounds the writing"));
        finger(canvas, 8, {s + QPointF(100, 150), s + QPointF(110, 160)});
        check(canvas->ignoredTouches() == ignored + 1, QStringLiteral("a palm touch in the rest zone is ignored while the pen writes"));
        check(canvas->strokeCount() == 0, QStringLiteral("... and leaves no ink"));
        const int placed = canvas->placedCursors();
        const QPointF far = canvasPointOf(canvas, ed, 10) + QPointF(0, 330);
        finger(canvas, 9, {far, far});
        check(canvas->ignoredTouches() == ignored + 1 && canvas->placedCursors() == placed + 1,
              QStringLiteral("a touch far from the writing is accepted (a tap there places the cursor)"));
        tablet(canvas, QEvent::TabletRelease, s + QPointF(120, 4));
        tick(500);
        finger(canvas, 10, {s + QPointF(100, 150), s + QPointF(100, 150)});
        check(canvas->ignoredTouches() == ignored + 2, QStringLiteral("0.5 s after the pen lifted the zone still ignores fingers"));
        tick(1200);
        const int ign2 = canvas->ignoredTouches();
        finger(canvas, 11, {s + QPointF(100, 150), s + QPointF(100, 150)});
        check(canvas->ignoredTouches() == ign2, QStringLiteral("about 1 s after the pen lifted, fingers are accepted again"));
        // a short, small tap on a button is accepted (the buttons lie outside the writing layer)
        bool clicked = false;
        QObject::connect(panel->spaceButton(), &QPushButton::clicked, [&] { clicked = true; });
        tablet(canvas, QEvent::TabletPress, s);
        QTest::mouseClick(panel->spaceButton(), Qt::LeftButton);
        check(clicked, QStringLiteral("a short, small tap on a button still works while the pen writes"));
        tablet(canvas, QEvent::TabletRelease, s);
        canvas->clearInk();
        strip->clear();
        penUpFor(pen);
        tick(40000);
    }

    // scratching out: ink not read yet, then text
    {
        canvas->clearInk();
        const QPointF s = canvasPointOf(canvas, ed, 4) + QPointF(0, 60);
        penStroke(canvas, wave(s, 120));
        penStroke(canvas, wave(s + QPointF(0, 200), 120));
        check(canvas->strokeCount() == 2, QStringLiteral("(two words of ink, far apart)"));
        penStroke(canvas, scratch(s + QPointF(-5, -2), 140), false, 4);
        check(canvas->strokeCount() == 1, QStringLiteral("scratching a word of ink out deletes it (the other stays)"), QString::number(canvas->strokeCount()));
        canvas->clearInk();
        // the text: scratch over the words "calm grey sea"
        const QString before = ed->toPlainText();
        const int p1 = before.indexOf(QLatin1String("calm")), p2 = before.indexOf(QLatin1String("sea"));
        const QPointF a = canvasPointOf(canvas, ed, p1 + 1), b = canvasPointOf(canvas, ed, p2 + 1);
        const double w2 = b.x() - a.x();
        penStroke(canvas, scratch(QPointF(a.x(), a.y() - 4), w2), false, 4);
        const QString after = ed->toPlainText();
        check(!after.contains(QLatin1String("calm")) && !after.contains(QLatin1String("grey")) && !after.contains(QLatin1String("sea"))
                  && after.contains(QLatin1String("wind rose over the and the gulls")),
              QStringLiteral("scratching words of the text out deletes them"), after.left(80));
        QTextCursor c(ed->document());
        c.select(QTextCursor::Document);
        c.removeSelectedText();   // (the next checks need the long text back)
        c.insertText(before);
        ed->document()->clearUndoRedoStacks();
        canvas->clearInk();
    }

    // ---- both
    hw->setMode(Mode::Both);
    settle(400);
    {
        const QRect ea(w.editorAreaWidget()->mapTo(&w, QPoint(0, 0)), w.editorAreaWidget()->size());
        const QRect pad(panel->padHolder()->mapTo(&w, QPoint(0, 0)), panel->padHolder()->size());
        const QRegion region = hw->overlayRegion();
        check(canvas->parentWidget() == &w && canvas->isVisible(), QStringLiteral("Both: the one layer lies over the editor and the inspector"));
        check(region.contains(ea.center()) && region.contains(pad.center()), QStringLiteral("it covers the editor's text and the inspector's content"));
        // the pane line (the splitter's handle between the editor and the inspector)
        QSplitter *sp = w.paneSplitter();
        bool handleCovered = false;
        for (int i = 1; i < sp->count(); ++i) {
            QWidget *h = sp->handle(i);
            const QPoint hc = h->mapTo(&w, h->rect().center());
            if (ea.right() < hc.x() && hc.x() < pad.left() + 2)
                handleCovered = region.contains(hc);
        }
        check(handleCovered, QStringLiteral("it covers the pane line, so the pane line cannot be dragged meanwhile"));
        const QPoint tabRow = inspector->findChild<QToolButton *>(QStringLiteral("inspectorTab:handwriting"))->mapTo(&w, QPoint(5, 5));
        check(!region.contains(tabRow) && !region.contains(panel->strip()->mapTo(&w, QPoint(10, 10))),
              QStringLiteral("the tabs and the strip stay free"));
        check(canvas->gridRect().width() > 100 && canvas->gridRect().height() > 100 && canvas->gridRect().width() < canvas->width() - 100,
              QStringLiteral("the grid lies only over the inspector part"));
        shot(&w, QStringLiteral("hw-mode-both"));
        // write in the editor part and in the pad part
        fake.setLines({QStringLiteral("hello world"), QStringLiteral("second line")});
        const QPointF inEditor = canvasPointOf(canvas, ed, 6) + QPointF(0, 30);
        const QPointF inPad = canvas->gridRect().topLeft() + QPointF(40, 60);
        penStroke(canvas, wave(inEditor, 120));
        penStroke(canvas, wave(inPad, 100));
        check(canvas->strokeCount() == 2, QStringLiteral("Both: writing works over the editor and over the pad part"));
        readAndFade();
        check(strip->text().contains(QLatin1String("hello world")) && strip->text().contains(QLatin1String("second line")),
              QStringLiteral("... both lines were read"), strip->text());
        strip->clear();
        canvas->clearInk();
        // a tap in the pad part puts no cursor (nothing to point at); over the text it does
        const int placed = canvas->placedCursors();
        tablet(canvas, QEvent::TabletPress, inPad);
        tick(40);
        tablet(canvas, QEvent::TabletRelease, inPad);
        check(canvas->placedCursors() == placed && canvas->strokeCount() == 1, QStringLiteral("in the pad part a tap is a dot of ink, not a cursor"));
        canvas->clearInk();
    }

    // ---- the strip: alternatives, writing over a word, learning
    std::printf("== the strip\n");
    hw->setMode(Mode::Inspector);
    settle(300);
    {
        QSettings().remove(QStringLiteral("handwriting/learning"));
        LearningStore::removeAll();
        fake.setLines({QStringLiteral("hello world")});
        penStroke(canvas, wave(QPointF(30, 70), 160));
        readAndFade();
        check(strip->text() == QLatin1String("hello world") && strip->tokens().size() == 2, QStringLiteral("(the strip holds two words)"));
        const QRect r0 = strip->tokenRect(0);
        QTest::mouseClick(strip, Qt::LeftButton, Qt::NoModifier, r0.center());
        settle(100);
        QMenu *menu = strip->findChild<QMenu *>(QStringLiteral("stripAlternatives"));
        QStringList alts;
        if (menu)
            for (QAction *a : menu->actions())
                alts << a->text();
        check(menu && menu->isVisible() && alts == QStringList({QStringLiteral("hallo"), QStringLiteral("jello")}),
              QStringLiteral("a tap on a word shows its alternatives (from the recognizer)"), alts.join(QLatin1Char(',')));
        shot(&w, QStringLiteral("hw-alternatives"));
        if (menu)
            menu->actions().first()->trigger();
        settle(100);
        check(strip->text() == QLatin1String("hallo world"), QStringLiteral("choosing one replaces the word"), strip->text());
        check(LearningStore::count() == 1, QStringLiteral("the corrected line (picture and text) was kept on the device"), QString::number(LearningStore::count()));
        QDir d(LearningStore::folder());
        const QStringList files = d.entryList({QStringLiteral("*.txt")});
        QFile f(d.filePath(files.value(0)));
        check(f.open(QIODevice::ReadOnly) && QString::fromUtf8(f.readAll()) == QLatin1String("hallo world"), QStringLiteral("... with the corrected text"));

        // writing over a word corrects it
        fake.setLines({QStringLiteral("Welt")});
        const QRect r1 = strip->tokenRect(1);
        QList<QPointF> over = line(QPointF(r1.left() + 4, r1.center().y()), QPointF(r1.right() - 4, r1.center().y() + 3), 8);
        QTest::mousePress(strip, Qt::LeftButton, Qt::NoModifier, over.first().toPoint());
        for (const QPointF &q : over) {
            tick(10);
            QTest::mouseMove(strip, q.toPoint());
        }
        QTest::mouseRelease(strip, Qt::LeftButton, Qt::NoModifier, over.last().toPoint());
        tick(1500);
        settle(150);
        check(strip->text() == QLatin1String("hallo Welt"), QStringLiteral("writing over a word corrects it"), strip->text());
        check(LearningStore::count() == 2, QStringLiteral("(a second corrected line was kept)"));

        // scratching a word out in the strip
        const QRect r2 = strip->tokenRect(0);
        QList<QPointF> sc = scratch(QPointF(r2.left() + 2, r2.center().y() - 2), r2.width() - 4);
        QTest::mousePress(strip, Qt::LeftButton, Qt::NoModifier, sc.first().toPoint());
        for (const QPointF &q : sc) {
            tick(4);
            QTest::mouseMove(strip, q.toPoint());
        }
        QTest::mouseRelease(strip, Qt::LeftButton, Qt::NoModifier, sc.last().toPoint());
        settle(100);
        check(strip->text() == QLatin1String("Welt"), QStringLiteral("scratching a word out in the strip deletes it"), strip->text());

        // the learning switch and the delete
        HandwritingSettings s = HandwritingConfig::instance().get();
        s.learning = false;
        HandwritingConfig::instance().set(s);
        strip->clear();
        fake.setLines({QStringLiteral("hello world")});
        penStroke(canvas, wave(QPointF(30, 70), 160));
        readAndFade();
        strip->replaceWord(0, QStringLiteral("hallo"));
        check(LearningStore::count() == 2, QStringLiteral("with learning switched off nothing more is kept"), QString::number(LearningStore::count()));
        s.learning = true;
        HandwritingConfig::instance().set(s);
        // H2 (encryption review): while an encrypted project is open nothing is kept (the files would be plain text and pictures)
        {
            qb::vault::add(QDir::tempPath() + QStringLiteral("/qb-j-encrypted-folder"), {QStringLiteral("A"), QStringLiteral("B")});
            QImage pic(40, 20, QImage::Format_ARGB32);
            pic.fill(Qt::black);
            check(!LearningStore::add(pic, QStringLiteral("secret words")) && LearningStore::count() == 2,
                  QStringLiteral("H2: with an encrypted project open no corrected line is kept"), QString::number(LearningStore::count()));
            strip->clear();
            fake.setLines({QStringLiteral("hello world")});
            penStroke(canvas, wave(QPointF(30, 70), 160));
            readAndFade();
            strip->replaceWord(0, QStringLiteral("hallo"));
            check(LearningStore::count() == 2, QStringLiteral("... also not by a correction in the strip"), QString::number(LearningStore::count()));
            qb::vault::remove(QDir::tempPath() + QStringLiteral("/qb-j-encrypted-folder"));
            check(!qb::vault::active(), QStringLiteral("(the folder left the vault)"));
        }
        check(LearningStore::removeAll() == 2 && LearningStore::count() == 0, QStringLiteral("the kept lines can be deleted"));
        strip->clear();
    }

    // ---- Insert at the held cursor, after the text moved; Pin and drop
    std::printf("== Insert and Pin\n");
    hw->setMode(Mode::Editor);
    settle(300);
    InkCanvas::scrollBarFor(ed)->setValue(0);
    {
        const int target = ed->toPlainText().indexOf(QLatin1String("lamp"));
        const QPointF tp = canvasPointOf(canvas, ed, target);
        tablet(canvas, QEvent::TabletPress, tp);
        tick(50);
        tablet(canvas, QEvent::TabletRelease, tp);
        const int held = hw->held().position();
        // the text moves: something is put in front of it
        QTextCursor front(ed->document());
        front.setPosition(0);
        front.insertText(QStringLiteral("NEW "));
        check(hw->held().position() == held + 4, QStringLiteral("the held cursor moves with the text"));
        strip->addLine(QImage(30, 20, QImage::Format_RGB32), [] {
            Result r;
            r.ok = true;
            for (const QString &t : {QStringLiteral("lantern"), QStringLiteral("light")}) {
                Word wd;
                wd.text = t;
                r.words << wd;
            }
            return r;
        }());
        QTest::mouseClick(panel->insertButton(), Qt::LeftButton);
        const QString doc = ed->toPlainText();
        check(doc.contains(QLatin1String("lantern lightlamp")) && strip->isEmpty(),
              QStringLiteral("Insert puts the strip's text at the cursor set before (after the text moved)"), doc.left(100));
        check(hw->held().position() == held + 4 + 13 && ed->textCursor().position() == hw->held().position(),
              QStringLiteral("... and moves the place behind it"), QString::number(hw->held().position()));
        ed->document()->undo();
        ed->document()->undo();

        // Pin: the text sticks; a tap on a text field shows the place; the check mark drops it
        strip->addLine(QImage(30, 20, QImage::Format_RGB32), [] {
            Result r;
            r.ok = true;
            Word wd;
            wd.text = QStringLiteral("pinned");
            r.words << wd;
            return r;
        }());
        QTest::mouseClick(panel->pinButton(), Qt::LeftButton);
        settle(100);
        check(hw->pinned() && hw->pinGhost()->isVisible() && strip->isEmpty(), QStringLiteral("Pin: the text leaves the strip and sticks to the pen"));
        check(canvas->testAttribute(Qt::WA_TransparentForMouseEvents), QStringLiteral("... the writing layer lets taps through to the text fields"));
        auto *drop = hw->pinGhost()->findChild<QToolButton *>(QStringLiteral("hwPinDrop"));
        check(drop && !drop->isEnabled() && !hw->hasPlace(), QStringLiteral("the check mark waits for a place"));
        const int at = ed->toPlainText().indexOf(QLatin1String("keeper"));
        QTextCursor tc(ed->document());
        tc.setPosition(at);
        const QRect cr = ed->cursorRect(tc);
        QTest::mouseClick(ed->viewport(), Qt::LeftButton, Qt::NoModifier, cr.center());
        settle(100);
        check(hw->hasPlace() && hw->placeMarker()->isVisible() && drop->isEnabled(), QStringLiteral("a tap on the editor shows the place"));
        shot(&w, QStringLiteral("hw-pin-place"));
        QTest::mouseClick(drop, Qt::LeftButton);
        settle(100);
        check(!hw->pinned() && !hw->pinGhost()->isVisible() && ed->toPlainText().contains(QLatin1String("pinned")),
              QStringLiteral("the check mark drops the text at the place"));
        check(!canvas->testAttribute(Qt::WA_TransparentForMouseEvents), QStringLiteral("... and the writing layer takes input again"));
        ed->document()->undo();

        // cancel with the cross: the text goes back; a tap on the synopsis (a text field of the inspector) sets a place
        strip->addLine(QImage(30, 20, QImage::Format_RGB32), [] {
            Result r;
            r.ok = true;
            Word wd;
            wd.text = QStringLiteral("again");
            r.words << wd;
            return r;
        }());
        QTest::mouseClick(panel->pinButton(), Qt::LeftButton);
        auto *cancel = hw->pinGhost()->findChild<QToolButton *>(QStringLiteral("hwPinCancel"));
        QTest::mouseClick(cancel, Qt::LeftButton);
        check(!hw->pinned() && strip->text() == QLatin1String("again"), QStringLiteral("the cross cancels: the text is back in the strip"), strip->text());
        QTest::mouseClick(panel->pinButton(), Qt::LeftButton);
        inspector->showTab(Inspector::Notes);
        settle(200);
        QPlainTextEdit *syn = nullptr;
        for (QPlainTextEdit *e : inspector->findChildren<QPlainTextEdit *>())
            if (e->isVisible()) {
                syn = e;
                break;
            }
        check(syn != nullptr, QStringLiteral("(the synopsis is shown)"));
        if (syn) {
            QTest::mouseClick(syn->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
            settle(100);
            check(hw->hasPlace(), QStringLiteral("a tap on the synopsis shows the place there too"));
            QTest::mouseClick(drop, Qt::LeftButton);
            settle(100);
            check(syn->toPlainText().contains(QLatin1String("again")), QStringLiteral("the text goes into the synopsis"), syn->toPlainText());
            syn->clear();
        }
        inspector->showExtraTab(QStringLiteral("handwriting"));
    }

    // ---- the grid
    std::printf("== the grid\n");
    hw->setMode(Mode::Inspector);
    settle(300);
    {
        canvas->clearInk();
        QList<QImage> shots;
        QStringList names;
        const GridSettings::Style styles[] = {GridSettings::Ruled, GridSettings::Writing, GridSettings::Squares, GridSettings::Dots, GridSettings::None};
        for (auto st : styles) {
            HandwritingSettings s = HandwritingConfig::instance().get();
            s.grid.style = st;
            HandwritingConfig::instance().set(s);
            settle(100);
            const QImage img = canvas->grab().toImage();
            shots << img;
            img.save(shotDir + QStringLiteral("/hw-grid-%1.png").arg(GridSettings::styleKey(st)));
        }
        bool distinct = true;
        for (int i = 0; i < 4; ++i)
            for (int j = i + 1; j < 5; ++j)
                if (shots[i] == shots[j])
                    distinct = false;
        check(distinct, QStringLiteral("ruled lines, writing lines, squares, dots and none look different"));
        // none: only the ground
        QImage blank(shots[4].size(), shots[4].format());
        blank.fill(canvas->palette().color(QPalette::Base));
        check(shots[4] == blank, QStringLiteral("\"none\" draws nothing but the ground"));
        HandwritingSettings s = HandwritingConfig::instance().get();
        s.grid.style = GridSettings::Ruled;
        s.grid.size = 0;
        HandwritingConfig::instance().set(s);
        const double small = canvas->rowOf(100.0), smallPitch = s.grid.pitch();
        s.grid.size = 100;
        HandwritingConfig::instance().set(s);
        const double large = canvas->rowOf(100.0), largePitch = s.grid.pitch();
        check(smallPitch < largePitch && small > large, QStringLiteral("the size slider goes from small to large (the pitch grows, fewer rows fit)"),
              QStringLiteral("%1 %2 %3 %4").arg(smallPitch).arg(largePitch).arg(small).arg(large));
        s.grid.style = GridSettings::Dots;
        HandwritingConfig::instance().set(s);
        check(canvas->rowOf(100.0) == -1, QStringLiteral("dots and no grid have no rows (the ink is grouped by its position)"));
        // strokes apart without rows: two lines
        penStroke(canvas, wave(QPointF(30, 60), 100));
        penStroke(canvas, wave(QPointF(30, 260), 100));
        check(canvas->pendingLines() == 2, QStringLiteral("without rows, strokes far apart are two lines"));
        canvas->clearInk();
        s.grid = GridSettings();
        HandwritingConfig::instance().set(s);
    }

    // ---- File > Options > Handwriting
    std::printf("== the Options page\n");
    {
        OptionsDialog dlg(&w);
        dlg.showSection(OptionsDialog::SGeneral, QStringLiteral("Handwriting"));
        dlg.show();
        settle(300);
        auto *page = dlg.findChild<HandwritingPage *>();
        check(page && page->isVisible(), QStringLiteral("File > Options > Handwriting opens"));
        check(dlg.pageNames(OptionsDialog::SGeneral).contains(QStringLiteral("Handwriting")), QStringLiteral("it is listed among the pages"));
        if (page) {
            auto cb = [&](const char *n) { return page->findChild<QComboBox *>(QLatin1String(n)); };
            auto *main = cb("hwMainLanguage");
            auto *fallback = cb("hwFallbackLanguage");
            QStringList langs;
            for (int i = 0; i < main->count(); ++i)
                langs << main->itemData(i).toString();
            check(langs.contains(QStringLiteral("de")) && langs.contains(QStringLiteral("en")) && langs.size() == 11, QStringLiteral("the main language lists the program's eleven languages"),
                  langs.join(QLatin1Char(',')));
            bool onlyRuGrey = true;
            for (int i = 0; i < main->count(); ++i) {
                const bool enabled = main->model()->flags(main->model()->index(i, 0)) & Qt::ItemIsEnabled;
                if (enabled == (main->itemData(i).toString() == QLatin1String("ru")))
                    onlyRuGrey = false;
            }
            check(onlyRuGrey, QStringLiteral("only Russian is greyed out"));
            check(fallback->count() == main->count() + 1 && fallback->itemData(0).toString().isEmpty(), QStringLiteral("a fallback language can be chosen, or none"));
            check(page->findChild<QSlider *>(QStringLiteral("hwGridSize")) && page->findChild<GridSample *>(QStringLiteral("hwGridSample")),
                  QStringLiteral("the grid size is a slider with a sample beside it"));
            // the sample follows the slider
            auto *slider = page->findChild<QSlider *>(QStringLiteral("hwGridSize"));
            auto *sample = page->findChild<GridSample *>(QStringLiteral("hwGridSample"));
            slider->setValue(0);
            settle(50);
            const QImage smallImg = sample->grab().toImage();
            const double smallPitch = sample->grid().pitch();
            slider->setValue(100);
            settle(50);
            check(sample->grab().toImage() != smallImg && sample->grid().pitch() > smallPitch, QStringLiteral("the sample beside the slider changes live"));
            // change things and apply
            main->setCurrentIndex(main->findData(QStringLiteral("de")));
            fallback->setCurrentIndex(fallback->findData(QStringLiteral("en")));
            page->findChild<QComboBox *>(QStringLiteral("hwHand"))->setCurrentIndex(1);
            page->findChild<QSlider *>(QStringLiteral("hwStrength"))->setValue(80);
            page->findChild<QCheckBox *>(QStringLiteral("hwProtect"))->setChecked(false);
            page->findChild<QCheckBox *>(QStringLiteral("hwFingerWrites"))->setChecked(false);
            page->findChild<QCheckBox *>(QStringLiteral("hwLearning"))->setChecked(false);
            page->findChild<QComboBox *>(QStringLiteral("hwGridStyle"))->setCurrentIndex(2);
            shot(&dlg, QStringLiteral("hw-options-changed"));
            dlg.apply();
            const HandwritingSettings &now = HandwritingConfig::instance().get();
            check(now.mainLanguage == QLatin1String("de") && now.fallbackLanguage == QLatin1String("en") && now.leftHanded && now.strength == 80
                      && !now.protectHand && !now.fingerWrites && !now.learning && now.grid.style == GridSettings::Squares && now.grid.size == 100,
                  QStringLiteral("Apply keeps the choices"));
            const HandwritingSettings loaded = HandwritingSettings::load();
            check(loaded.mainLanguage == QLatin1String("de") && loaded.leftHanded && loaded.grid.style == GridSettings::Squares,
                  QStringLiteral("they are kept per device (QSettings), not in a project"));
            // the writing hand and the strength change the rest zone
            check(canvas->settings().leftHanded && canvas->settings().strength == 80, QStringLiteral("the writing area follows the settings at once"));
            // the button switch of the page
            auto *shownBox = page->findChild<QCheckBox *>(QStringLiteral("hwButtonShown"));
            shownBox->setChecked(false);
            dlg.apply();
            settle(200);
            check(!button->isVisible() && !show->isChecked(), QStringLiteral("the page's switch hides the toolbar button"));
            shownBox->setChecked(true);
            dlg.apply();
            settle(200);
            check(button->isVisible() && show->isChecked(), QStringLiteral("... and shows it again"));
            // learning: the delete button
            QImage pic(20, 20, QImage::Format_RGB32);
            pic.fill(Qt::white);
            HandwritingSettings ls = HandwritingConfig::instance().get();
            ls.learning = true;
            HandwritingConfig::instance().set(ls);
            LearningStore::add(pic, QStringLiteral("kept"));
            check(LearningStore::count() == 1, QStringLiteral("(a kept line)"));
            page->findChild<QPushButton *>(QStringLiteral("hwDeleteLearned"))->click();
            check(LearningStore::count() == 0, QStringLiteral("the page deletes the kept lines"));
        }
        dlg.reject();
        // back to the starting values
        HandwritingSettings s = HandwritingSettings::defaults(false);
        s.buttonShown = true;
        s.pauseMs = 300;
        s.stayMs = 400;
        s.fadeMs = 400;
        HandwritingConfig::instance().set(s);
    }

    // ---- the mode is kept per device; the model-memory hook
    std::printf("== kept per device, the hook\n");
    {
        hw->setMode(Mode::Both);
        HandwritingConfig::instance().reload();
        check(HandwritingConfig::instance().get().mode == Mode::Both && HandwritingConfig::instance().get().lastMode == Mode::Both,
              QStringLiteral("the mode and the last mode are kept per device"));
        QDir pdir(p->folderPath());
        bool inProject = false;
        const QStringList all = pdir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &f : all) {
            QFile pf(pdir.filePath(f));
            if (pf.open(QIODevice::ReadOnly) && pf.readAll().contains("handwriting"))
                inProject = true;
        }
        check(!inProject, QStringLiteral("nothing about it is written into the project"));
        hw->setMode(Mode::Off);
        check(!inspector->extraTabVisible(QStringLiteral("handwriting")) && !canvas->isVisible(), QStringLiteral("off: the tab and the writing area are gone"));
        check(fake.memoryId().startsWith(QLatin1String("handwriting:")), QStringLiteral("the recognizer's model id for ModelMemory starts with handwriting:"));
        modelmem::ModelMemory mm;
        const auto d = mm.requestLoad(fake.memoryId(), QStringLiteral("Handwriting"), 1000, QStringLiteral("ram"), [](const QString &) { return true; });
        mm.modelLoaded(fake.memoryId());
        check(d.allowed && mm.isLoaded(fake.memoryId()), QStringLiteral("a recognizer can register its model with ModelMemory through that id"));
    }

    shot(&w, QStringLiteral("hw-final"));
    delete wp;
    Recognizer::setCurrent(nullptr);
    std::printf("\n%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
