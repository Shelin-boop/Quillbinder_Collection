// The small interface (0.99.0, to-do item 122): a general mode for small screens and for use without a keyboard.
//
// Shown: the editor over the whole window, and a bar of round buttons (a row at the bottom of an upright window, a column at
// the right edge of a wide one): the menu button (the full menu bar's menus and a way back to the full interface), a new
// document, bold, italic, a style choice, the binder, labels and icons, record only and the document's notes. The binder
// and the notes are the program's own binder and inspector, moved into slide-in panels (a part of the width beside the
// text when the window is wide, a sheet over the whole editor area when it is upright) and moved back when the mode ends.
//
// Leaving the mode gives back everything: the toolbars were only hidden (the settings that say which are shown were not
// touched), the menu bar was only hidden, the panes' sizes are kept in m_wideSizes and the shares (the pane code stands
// still while the mode runs: arrangePanes, applyPaneSizes, capturePaneSizes), and the switches of the binder and the
// inspector are put back as they were. The mode is a program setting ("window/smallInterface"), so a profile keeps it.
#include "ui/MainWindow.h"

#include "core/ItemIcons.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/BarDock.h"
#include "ui/BinderPanel.h"
#include "ui/FocusLine.h"
#include "ui/Inspector.h"
#include "ui/SmallInterface.h"
#include "ui/TextEditor.h"
#include "ui/ToolHost.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QEvent>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>
#include <QSettings>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <functional>
#include <tuple>

namespace qb {

namespace {
// Commands that show the binder or the inspector by themselves (Project Search, a click on a comment mark) call show() on the
// pane; in the small interface the pane is in a closed panel, so the panel opens instead.
class ShowWatcher : public QObject
{
public:
    ShowWatcher(std::function<void(QObject *)> onShow, QObject *parent) : QObject(parent), m_onShow(std::move(onShow)) {}
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::ShowToParent)
            m_onShow(o);
        return false;
    }

private:
    std::function<void(QObject *)> m_onShow;
};

const char *const kRecordOnlyKey = "tool:speech-transcription.record-only";
const char *const kRecordKey = "tool:speech-transcription.record";
} // namespace

// ---- the choice (the rule and its settings; the menu items of Android are made under Q_OS_ANDROID only)

smallui::Choice MainWindow::smallChoice() const { return m_smallChoice; }

void MainWindow::setSmallChoice(smallui::Choice choice)
{
    m_smallChoice = choice;
    QSettings().setValue(QLatin1String(smallui::kKeyChoice), smallui::choiceKey(choice));
    for (auto [key, c] : {std::pair<const char *, smallui::Choice>{"smallChoiceFull", smallui::Choice::Full},
                          {"smallChoiceSmall", smallui::Choice::Small},
                          {"smallChoiceAuto", smallui::Choice::Automatic}})
        if (QAction *ac = a.value(QLatin1String(key)))
            ac->setChecked(c == choice);
    applySmallRule(smallui::physicalSize(screen() ? screen() : QGuiApplication::primaryScreen()));
}

// the rule: Full never, Small always, Automatic below 1920 x 1080 physical pixels (the size comes in, so that it can be tested)
void MainWindow::applySmallRule(const QSize &physicalPixels)
{
    setSmallInterface(smallui::resolve(m_smallChoice, physicalPixels));
}

// ---- the actions (before the menus are made) and the bar, the panels (after the toolbars)

void MainWindow::createSmallActions()
{
    auto make = [this](const QString &key, const QString &text) {
        auto *ac = new QAction(text, this);
        ac->setObjectName(key);
        ac->setIconText(text);
        a.insert(key, ac);
        addAction(ac);
        return ac;
    };
#ifndef Q_OS_ANDROID
    QAction *check = make(QStringLiteral("smallInterface"), tr("Sm&all Interface"));
    check->setCheckable(true);
    check->setToolTip(tr("A few round buttons and the most room for the text, for a small screen or for use without a keyboard"));
    connect(check, &QAction::toggled, this, [this](bool on) { setSmallInterface(on); });
#else
    QActionGroup *group = new QActionGroup(this);
    for (auto [key, text, c] : {std::tuple<QString, QString, smallui::Choice>{QStringLiteral("smallChoiceFull"), tr("&Full"), smallui::Choice::Full},
                                {QStringLiteral("smallChoiceSmall"), tr("&Small"), smallui::Choice::Small},
                                {QStringLiteral("smallChoiceAuto"), tr("&Automatic"), smallui::Choice::Automatic}}) {
        QAction *ac = make(key, text);
        ac->setCheckable(true);
        group->addAction(ac);
        const smallui::Choice choice = c;
        connect(ac, &QAction::triggered, this, [this, choice] { setSmallChoice(choice); });
    }
    // the check item of the other systems is still there for the code (the menu shows the three choices)
    QAction *check = make(QStringLiteral("smallInterface"), tr("Sm&all Interface"));
    check->setCheckable(true);
    connect(check, &QAction::toggled, this, [this](bool on) { setSmallInterface(on); });
#endif
    // the menu button's way back
    auto *full = new QAction(tr("Switch to the &Full Interface"), this);
    full->setObjectName(QStringLiteral("smallToFull"));
    full->setIcon(icons::action(QStringLiteral("view-split-left-right")));
    connect(full, &QAction::triggered, this, [this] {
#ifdef Q_OS_ANDROID
        setSmallChoice(smallui::Choice::Full);
#else
        setSmallInterface(false);
#endif
    });
    a.insert(QStringLiteral("smallToFull"), full);
    // the round buttons of the binder and the notes (the panels follow them)
    for (auto [key, text, notes] : {std::tuple<QString, QString, bool>{QStringLiteral("smallBinder"), tr("Binder"), false},
                                    {QStringLiteral("smallNotes"), tr("Notes"), true}}) {
        auto *ac = new QAction(text, this);
        ac->setObjectName(key);
        ac->setCheckable(true);
        ac->setIcon(icons::action(notes ? QStringLiteral("view-pim-notes") : QStringLiteral("view-list-tree")));
        ac->setToolTip(notes ? tr("The notes of the document, in a panel") : tr("The binder, in a panel"));
        a.insert(key, ac);
        connect(ac, &QAction::toggled, this, [this, notes](bool on) { showSmallPanel(notes, on); });
    }
    auto *rec = new QAction(tr("Record Only"), this);
    rec->setObjectName(QStringLiteral("smallRecord"));
    rec->setIcon(icons::action(QStringLiteral("media-record")));
    rec->setCheckable(true);
    a.insert(QStringLiteral("smallRecord"), rec);
    m_smallRecordAction = rec;
}

QWidget *MainWindow::smallButton(const QString &name) const { return m_smallButtons.value(name); }

void MainWindow::createSmallInterface()
{
    m_smallBar = new QToolBar(tr("Small Interface"), this);
    m_smallBar->setObjectName(QStringLiteral("smallBar"));
    m_smallBar->setMovable(false);
    m_smallBar->setFloatable(false);
    m_smallBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_smallBar->setAllowedAreas(Qt::BottomToolBarArea | Qt::RightToolBarArea);
    // (the bar is in the window's layout only while the mode runs: the window's saved state does not know it otherwise)
    m_smallBar->hide();

    m_smallBinder = new smallui::SlidePanel(tr("Binder"), this);
    m_smallBinder->setObjectName(QStringLiteral("smallBinderPanel"));
    m_smallNotes = new smallui::SlidePanel(tr("Notes"), this);
    m_smallNotes->setObjectName(QStringLiteral("smallNotesPanel"));
    for (auto [panel, key] : {std::pair<smallui::SlidePanel *, const char *>{m_smallBinder, "smallBinder"}, {m_smallNotes, "smallNotes"}})
        connect(panel, &smallui::SlidePanel::closed, this, [this, key = QLatin1String(key)] {
            if (QAction *ac = a.value(key); ac && ac->isChecked()) {
                QSignalBlocker block(ac);
                ac->setChecked(false);
            }
        });
    for (smallui::SlidePanel *panel : {m_smallBinder, m_smallNotes})
        connect(panel, &smallui::SlidePanel::settled, this, [this] {
            if (m_focusLine)
                m_focusLine->refresh(); // (the line follows a part that moved with its panel)
        });
    auto *watch = new ShowWatcher(
        [this](QObject *o) {
            if (!m_small || m_smallQuiet)
                return;
            const bool notes = o == m_inspector;
            if (smallPanel(notes)->isOpen())
                return;
            QTimer::singleShot(0, this, [this, notes] {
                if (m_small && !m_smallQuiet && !smallPanel(notes)->isOpen())
                    showSmallPanel(notes, true, true);
            });
        },
        this);
    m_binder->installEventFilter(watch);
    m_inspector->installEventFilter(watch);
    // in an upright window the binder is a sheet over the text: a document picked in it shows the text
    connect(m_binder, &BinderPanel::selectionChanged, this, [this] {
        if (m_small && m_smallBinder->isOpen() && m_smallBinder->edge() == smallui::SlidePanel::Edge::Sheet)
            QTimer::singleShot(0, this, [this] { showSmallPanel(false, false); });
    });

    m_smallMenu = new QMenu(this);
    m_smallMenu->setObjectName(QStringLiteral("smallMenu"));
    connect(m_smallMenu, &QMenu::aboutToShow, this, &MainWindow::fillSmallMenu);
    m_smallLabelsMenu = new QMenu(this);
    m_smallLabelsMenu->setObjectName(QStringLiteral("smallLabelsMenu"));
    buildSmallButtons();
    connect(&AppSettings::instance(), &AppSettings::changed, this, [this] {
        if (m_small)
            restyleSmallButtons();
    });
}

namespace {
QToolButton *roundButton(const QString &objectName)
{
    auto *b = new QToolButton;
    b->setObjectName(objectName);
    b->setToolButtonStyle(Qt::ToolButtonIconOnly);
    b->setFocusPolicy(Qt::StrongFocus);
    b->setAutoRaise(false);
    return b;
}

// a button follows the enabled state of an action (menus of the menu bar have one too)
void mirrorEnabled(QToolButton *b, QAction *ac)
{
    b->setEnabled(ac->isEnabled());
    QObject::connect(ac, &QAction::changed, b, [b, ac] { b->setEnabled(ac->isEnabled()); });
}
} // namespace

void MainWindow::buildSmallButtons()
{
    auto add = [this](const QString &name, QToolButton *b, const QString &tip) {
        b->setToolTip(tip);
        b->setAccessibleName(tip);
        m_smallButtons[name] = b;
        m_smallBar->addWidget(b);
    };
    auto withAction = [&](const QString &name, const QString &actionKey) {
        QToolButton *b = roundButton(QStringLiteral("small_") + name);
        b->setDefaultAction(a[actionKey]); // (icon, state, checked and tip are the action's)
        b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_smallButtons[name] = b;
        m_smallBar->addWidget(b);
        return b;
    };
    // the menu: every menu of the menu bar and the way back
    {
        QToolButton *b = roundButton(QStringLiteral("small_menu"));
        b->setIcon(icons::action(QStringLiteral("view-list-text")));
        b->setPopupMode(QToolButton::InstantPopup);
        b->setMenu(m_smallMenu);
        add(QStringLiteral("menu"), b, tr("Menu: all the menus of the program"));
    }
    withAction(QStringLiteral("new"), QStringLiteral("newText"));
    withAction(QStringLiteral("bold"), QStringLiteral("bold"));
    withAction(QStringLiteral("italic"), QStringLiteral("italic"));
    {
        QToolButton *b = roundButton(QStringLiteral("small_style"));
        b->setIcon(icons::action(QStringLiteral("font")));
        b->setPopupMode(QToolButton::InstantPopup);
        if (m_styleMenu) {
            b->setMenu(m_styleMenu);
            mirrorEnabled(b, m_styleMenu->menuAction());
        }
        add(QStringLiteral("style"), b, tr("Style: choose a style for the paragraph"));
    }
    withAction(QStringLiteral("binder"), QStringLiteral("smallBinder"));
    {
        QToolButton *b = roundButton(QStringLiteral("small_labels"));
        b->setIcon(icons::action(QStringLiteral("tag")));
        b->setPopupMode(QToolButton::InstantPopup);
        b->setMenu(m_smallLabelsMenu);
        if (m_labelMenu)
            mirrorEnabled(b, m_labelMenu->menuAction());
        add(QStringLiteral("labels"), b, tr("Labels and icons of the selected documents"));
        if (m_labelMenu)
            m_smallLabelsMenu->addMenu(m_labelMenu);
        if (m_iconMenu)
            m_smallLabelsMenu->addMenu(m_iconMenu);
    }
    withAction(QStringLiteral("record"), QStringLiteral("smallRecord"));
    withAction(QStringLiteral("notes"), QStringLiteral("smallNotes"));
    connect(m_smallRecordAction, &QAction::triggered, this, [this] {
        if (QAction *ro = a.value(QLatin1String(kRecordOnlyKey)))
            ro->trigger();
        updateSmallRecord();
    });
    if (QAction *rec = a.value(QLatin1String(kRecordKey)))
        connect(rec, &QAction::toggled, this, [this] { updateSmallRecord(); });
    updateSmallRecord();
}

// the record button: the add-on's record-only command when it is installed, else off with the reason
void MainWindow::updateSmallRecord()
{
    QAction *ro = a.value(QLatin1String(kRecordOnlyKey));
    QAction *rec = a.value(QLatin1String(kRecordKey));
    m_smallRecordAction->setEnabled(ro != nullptr);
    if (ro) {
        QSignalBlocker block(m_smallRecordAction);
        m_smallRecordAction->setChecked(rec && rec->isChecked());
        m_smallRecordAction->setToolTip(m_smallRecordAction->isChecked()
                                            ? tr("Stop recording")
                                            : tr("Record only: the sound is kept and its text can be written later"));
    } else if (rec) {
        m_smallRecordAction->setToolTip(tr("Record Only is off: the installed Speech Transcription add-on is too old. "
                                           "Update it in the Add-on Manager."));
    } else {
        m_smallRecordAction->setToolTip(tr("Record Only is off: it needs the add-on Speech Transcription, which is not installed. "
                                           "The Add-on Manager can add it."));
    }
}

void MainWindow::restyleSmallButtons()
{
    if (!m_smallBar)
        return;
    QScreen *sc = screen() ? screen() : QGuiApplication::primaryScreen();
    const double dpr = sc ? qMax(1.0, sc->devicePixelRatio()) : 1.0;
    const double dpi = sc ? sc->physicalDotsPerInch() / dpr : 96.0;
    const int iconSide = qBound(16, AppSettings::instance().toolbarIconSize, 48);
    int d = qMax(smallui::touchTargetPx(dpi), iconSide + 14);
    // never more than a sixth of the window's short side: a column of nine buttons must fit a small window
    d = qMin(d, qMax(34, qMin(width(), height()) / 6));
    d = qMax(d, qMin(iconSide + 6, 34));
    const QString radius = QString::number(d / 2);
    const QString css = QStringLiteral(
                            "QToolButton { border: 1px solid palette(mid); border-radius: %1px; background: palette(button); "
                            "color: palette(button-text); }"
                            "QToolButton:checked { background: palette(highlight); color: palette(highlighted-text); }"
                            "QToolButton:disabled { background: palette(window); color: palette(mid); }"
                            "QToolButton:focus { border: 3px solid palette(highlight); }"
                            "QToolButton::menu-indicator { image: none; }")
                            .arg(radius);
    m_smallBar->setIconSize(QSize(iconSide, iconSide));
    for (QWidget *w : std::as_const(m_smallButtons)) {
        if (!w)
            continue;
        w->setFixedSize(d, d);
        w->setStyleSheet(css);
        if (auto *b = qobject_cast<QToolButton *>(w))
            b->setIconSize(QSize(iconSide, iconSide));
    }
    m_smallTargetPx = d;
    m_smallBinder->setTargetSize(d);
    m_smallNotes->setTargetSize(d);
}

void MainWindow::fillSmallMenu()
{
    m_smallMenu->clear();
    for (QAction *top : menuBar()->actions())
        if (top->menu())
            m_smallMenu->addMenu(top->menu());
    m_smallMenu->addSeparator();
    m_smallMenu->addAction(a[QStringLiteral("smallToFull")]);
}

// ---- where things stand

QRect MainWindow::smallArea() const
{
    return centralWidget() ? centralWidget()->geometry() : rect();
}

smallui::SlidePanel::Edge MainWindow::smallEdge(bool notes) const
{
    if (wantsUpright())
        return smallui::SlidePanel::Edge::Sheet;
    return notes ? smallui::SlidePanel::Edge::Right : smallui::SlidePanel::Edge::Left;
}

// the bar: a row at the bottom of an upright window, a column at the right edge of a wide one
void MainWindow::placeSmallBar()
{
    if (!m_smallBar)
        return;
    const bool up = wantsUpright();
    const Qt::ToolBarArea want = up ? Qt::BottomToolBarArea : Qt::RightToolBarArea;
    if (!m_small) {
        removeToolBar(m_smallBar); // (hides it)
        return;
    }
    if (toolBarArea(m_smallBar) != want) {
        setUpdatesEnabled(false);
        addToolBar(want, m_smallBar);
        setUpdatesEnabled(true);
    }
    m_smallBar->setOrientation(up ? Qt::Horizontal : Qt::Vertical);
    m_smallBar->show();
}

void MainWindow::refitSmallPanels()
{
    if (!m_small)
        return;
    placeSmallBar();
    restyleSmallButtons();
    // the geometry of the central area is settled after the layout; the panels are placed then
    QTimer::singleShot(0, this, [this] {
        if (!m_small)
            return;
        for (bool notes : {false, true})
            if (smallui::SlidePanel *p = smallPanel(notes); p->isOpen())
                p->refit(smallArea(), smallEdge(notes));
    });
}

void MainWindow::showSmallPanel(bool notes, bool on, bool keepTab)
{
    if (!m_small)
        return;
    smallui::SlidePanel *p = smallPanel(notes), *other = smallPanel(!notes);
    QAction *ac = a[notes ? QStringLiteral("smallNotes") : QStringLiteral("smallBinder")];
    if (on) {
        other->closePanel(false); // one panel at a time
        if (notes && !keepTab)
            m_inspector->showTab(Inspector::Notes);
        p->openIn(smallArea(), smallEdge(notes), true);
        if (notes) {
            if (!m_inspector->focusNotes())
                p->setFocus();
        } else {
            m_binder->setFocus();
        }
    } else {
        p->closePanel(false);
        if (TextEditor *e = editor())
            e->setFocus();
    }
    if (ac->isChecked() != on) {
        QSignalBlocker block(ac);
        ac->setChecked(on);
    }
}

// ---- the mode

void MainWindow::setSmallInterface(bool on)
{
    QAction *check = a.value(QStringLiteral("smallInterface"));
    if (on == m_small) {
        if (check && check->isChecked() != on) {
            QSignalBlocker block(check);
            check->setChecked(on);
        }
        return;
    }
    if (!m_smallBar)
        return; // (the window is still being made)
    QSettings st;
#ifndef Q_OS_ANDROID
    st.setValue(QLatin1String(smallui::kKeyOn), on); // (on Android the choice is what is kept)
#endif
    if (on) {
        capturePaneSizes(); // the sizes of the full interface are kept in m_wideSizes and the shares
        m_smallBinderChecked = a[QStringLiteral("showBinder")]->isChecked();
        m_smallInspectorChecked = a[QStringLiteral("showInspector")]->isChecked();
        m_smallIconBarShown = a[QStringLiteral("showIconBar")] && a[QStringLiteral("showIconBar")]->isChecked();
        m_smallMenuBarShown = !menuBar()->isHidden();
        m_small = true;
        if (check && !check->isChecked()) {
            QSignalBlocker block(check);
            check->setChecked(true);
        }
        setUpdatesEnabled(false);
        // the binder and the inspector leave the splitter for their panels
        m_smallBinder->setContent(m_binder);
        m_smallNotes->setContent(m_inspector);
        m_binder->hide();
        m_inspector->hide();
        m_sideSplit->hide();
        applyBarVisibility(); // (hides the bars while the mode runs)
        if (m_iconBar)
            m_iconBar->hide();
        menuBar()->hide();
        a[QStringLiteral("showBinder")]->setEnabled(false);
        a[QStringLiteral("showInspector")]->setEnabled(false);
        if (m_layoutMenu)
            m_layoutMenu->menuAction()->setEnabled(false);
        if (m_barsMenuBar)
            m_barsMenuBar->menuAction()->setEnabled(false);
        placeSmallBar();
        restyleSmallButtons();
        updateSmallRecord();
        setUpdatesEnabled(true);
    } else {
        for (bool notes : {false, true})
            if (smallui::SlidePanel *p = smallPanel(notes); p->isOpen())
                p->closePanel(false);
        m_small = false;
        if (check && check->isChecked()) {
            QSignalBlocker block(check);
            check->setChecked(false);
        }
        setUpdatesEnabled(false);
        placeSmallBar(); // (takes the bar out of the layout)
        menuBar()->setVisible(m_smallMenuBarShown);
        a[QStringLiteral("showBinder")]->setEnabled(true);
        a[QStringLiteral("showInspector")]->setEnabled(true);
        if (m_layoutMenu)
            m_layoutMenu->menuAction()->setEnabled(true);
        if (m_barsMenuBar)
            m_barsMenuBar->menuAction()->setEnabled(true);
        // what the switches were; the panes come back into the splitter (arrangePanes sets their visibility from them)
        a[QStringLiteral("showBinder")]->setChecked(m_smallBinderChecked);
        a[QStringLiteral("showInspector")]->setChecked(m_smallInspectorChecked);
        m_upright = wantsUpright();
        arrangePanes();
        updatePaneActions();
        applyBarVisibility();
        if (m_iconBar)
            m_iconBar->setVisible(m_smallIconBarShown);
        setUpdatesEnabled(true);
        QTimer::singleShot(0, this, [this] {
            if (!m_small && m_barDock)
                m_barDock->refit();
        });
    }
    for (QAction *ac : {a.value(QStringLiteral("smallBinder")), a.value(QStringLiteral("smallNotes"))})
        if (ac && ac->isChecked()) {
            QSignalBlocker block(ac);
            ac->setChecked(false);
        }
}

} // namespace qb
