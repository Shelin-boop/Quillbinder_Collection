// 0.95.0 (items 95 and 97): profiles in the main window (the Profiles toolbar, View > Profiles, the switch that applies a
// profile at once) and the Labels toolbar. The rules and the files are in ui/Profiles.*.
#include "ui/MainWindow.h"

#include "core/ItemIcons.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/BarDock.h"
#include "ui/BinderPanel.h"
#include "ui/ColorPalette.h"
#include "ui/Commands.h"
#include "ui/GripBar.h"
#include "ui/MenuClear.h"
#include "ui/MindMapDoc.h"
#include "ui/ProfileDialogs.h"
#include "ui/Profiles.h"
#include "ui/ProjectLook.h"
#include "ui/ToolHost.h"
#include "ui/UndoNames.h"

#include <QAction>
#include <QGuiApplication>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>
#include <tuple>

namespace qb {

void MainWindow::createProfileActions()
{
    auto act = [this](const QString &key, const QString &text, const QString &icon = QString()) {
        auto *ac = new QAction(text, this);
        ac->setObjectName(key);
        QString plain = text;
        plain.remove(QLatin1Char('&'));
        if (plain.endsWith(QChar(0x2026)))
            plain.chop(1);
        ac->setIconText(plain);
        if (!icon.isEmpty())
            ac->setIcon(icons::action(icon));
        undonames::nameStepsOf(ac);
        a.insert(key, ac);
        addAction(ac);
        return ac;
    };
    connect(act(QStringLiteral("profileManage"), tr("&Manage Profiles…"), QStringLiteral("configure")), &QAction::triggered, this,
            [this] { manageProfiles(); });
    a[QStringLiteral("profileManage")]->setToolTip(tr("Use, save, rename, copy, delete, export and import profiles"));
    connect(act(QStringLiteral("profileSaveNew"), tr("&Save Settings as New Profile…")), &QAction::triggered, this, [this] {
        const QString name = profileui::askName(this, tr("Save Settings as New Profile"), profiles::uniqueName(tr("My Profile")));
        if (name.isEmpty())
            return;
        QString err;
        if (!profiles::createFromCurrent(name, &err))
            QMessageBox::warning(this, tr("Save Settings as New Profile"), err);
        refreshProfileUi();
    });
    a[QStringLiteral("profileSaveNew")]->setToolTip(tr("Keep the options, toolbars, window and theme as they are now in a new profile"));
    // the switches of the two new toolbars (they are off at first)
    for (auto [key, command, name] : {std::tuple<QString, QString, QString>{QStringLiteral("profiles"), QStringLiteral("showBarProfiles"), tr("Profiles")},
                                      {QStringLiteral("labels"), QStringLiteral("showBarLabels"), tr("Labels")}}) {
        QAction *one = act(command, name);
        one->setCheckable(true);
        one->setChecked(AppSettings::instance().toolbarExtraShown.contains(key));
        connect(one, &QAction::toggled, this, [this, key = key](bool on) {
            if (m_applyingBars)
                return;
            AppSettings &s = AppSettings::instance();
            if (!on)
                s.toolbarExtraShown.removeAll(key);
            else if (!s.toolbarExtraShown.contains(key))
                s.toolbarExtraShown << key;
            s.save();
            applyBarVisibility();
        });
    }
}

void MainWindow::createProfileBars()
{
    // ---- Profiles
    auto *pb = new GripBar(tr("Profiles"), this);
    m_profileBar = pb;
    pb->setObjectName(QStringLiteral("profilesBar"));
    pb->setMovable(true);
    pb->setFloatable(false);
    pb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBar(Qt::TopToolBarArea, pb);
    auto *title = new QLabel(tr("Profile:"));
    title->setContentsMargins(6, 0, 4, 0);
    auto *titleAction = new QWidgetAction(pb);
    titleAction->setDefaultWidget(title);
    pb->addAction(titleAction);
    m_profileButton = new QToolButton;
    m_profileButton->setObjectName(QStringLiteral("profileButton"));
    m_profileButton->setPopupMode(QToolButton::InstantPopup);
    m_profileButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    auto *menu = new QMenu(m_profileButton);
    menu->setObjectName(QStringLiteral("profileButtonMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] { fillProfilesMenu(menu); });
    m_profileButton->setMenu(menu);
    auto *buttonAction = new QWidgetAction(pb);
    buttonAction->setDefaultWidget(m_profileButton);
    pb->addAction(buttonAction);
    pb->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(pb, &QWidget::customContextMenuRequested, this, [this, pb](const QPoint &pos) {
        QMenu m;
        m.setObjectName(QStringLiteral("barsMenu"));
        fillBarsMenu(&m, -1);
        m.exec(pb->mapToGlobal(pos));
    });
    refreshProfileUi();

    // ---- Labels
    auto *lb = new GripBar(tr("Labels"), this);
    m_labelBar = lb;
    lb->setObjectName(QStringLiteral("labelsBar"));
    lb->setMovable(true);
    lb->setFloatable(false);
    lb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    lb->setIconSize(QSize(20, 20));
    addToolBar(Qt::TopToolBarArea, lb);
    lb->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(lb, &QWidget::customContextMenuRequested, this, [this, lb](const QPoint &pos) {
        QMenu m;
        m.setObjectName(QStringLiteral("barsMenu"));
        fillBarsMenu(&m, -1);
        m.exec(lb->mapToGlobal(pos));
    });
    rebuildLabelBar();
    // off at first (the switches say so; applyBarVisibility() shows them when the writer turns them on)
    pb->setVisible(false);
    lb->setVisible(false);
}

void MainWindow::refreshProfileUi()
{
    if (m_profileButton) {
        m_profileButton->setText(profiles::active());
        m_profileButton->setToolTip(tr("The profile in use: %1. A click lists the profiles.").arg(profiles::active()));
    }
}

void MainWindow::fillProfilesMenu(QMenu *menu)
{
    clearMenu(menu);
    const QString inUse = profiles::active();
    for (const QString &name : profiles::names()) {
        QAction *ac = menu->addAction(name);
        ac->setCheckable(true);
        ac->setChecked(name.compare(inUse, Qt::CaseInsensitive) == 0);
        connect(ac, &QAction::triggered, this, [this, name] { switchProfile(name); });
    }
    menu->addSeparator();
    menu->addAction(a[QStringLiteral("profileSaveNew")]);
    menu->addAction(a[QStringLiteral("profileManage")]);
}

void MainWindow::manageProfiles()
{
    ProfileManagerDialog dlg(this);
    dlg.exec();
    refreshProfileUi();
}

// ---- the Labels toolbar (item 97)

void MainWindow::setLabelFor(int id)
{
    if (!m_project)
        return;
    for (BinderItem *it : actionItems()) {
        it->label = id;
        m_project->notifyItemChanged(it, false);
    }
}

void MainWindow::rebuildLabelBar()
{
    if (!m_labelBar)
        return;
    m_labelBar->clear();
    const Project *p = m_project.get();
    auto *label = new QWidgetAction(m_labelBar);
    auto *l = new QLabel((p && !p->labelTitle.isEmpty() ? p->labelTitle : tr("Label")) + QLatin1Char(':'));
    l->setContentsMargins(6, 0, 4, 0);
    label->setDefaultWidget(l);
    m_labelBar->addAction(label);
    QAction *none = m_labelBar->addAction(palette::swatch(QColor(), 16), tr("No Label"));
    none->setObjectName(QStringLiteral("labelBar_none"));
    none->setToolTip(p ? tr("No %1").arg(p->labelTitle) : tr("No Label"));
    none->setEnabled(p != nullptr);
    connect(none, &QAction::triggered, this, [this] { setLabelFor(-1); });
    if (p)
        for (const Label &lab : p->labels) {
            if (!lab.color.isValid())
                continue;
            QAction *ac = m_labelBar->addAction(palette::swatch(lab.color, 16), lab.name);
            ac->setObjectName(QStringLiteral("labelBar_%1").arg(lab.id));
            ac->setToolTip(lab.name);
            ac->setData(lab.id);
            const int id = lab.id;
            connect(ac, &QAction::triggered, this, [this, id] { setLabelFor(id); });
        }
}

// ---- switching

void MainWindow::saveWindowSettings()
{
    QSettings s;
    s.setValue(QStringLiteral("window/geometry"), saveGeometry());
    s.setValue(QStringLiteral("window/state"), saveState());
    savePaneSettings(); // 0.93.0: the places and sizes (both shapes, per device)
    s.setValue(QStringLiteral("window/inspector"), a[QStringLiteral("showInspector")]->isChecked());
    s.setValue(QStringLiteral("window/binder"), a[QStringLiteral("showBinder")]->isChecked());
    s.setValue(QStringLiteral("window/toolbar"), a[QStringLiteral("showToolbar")]->isChecked());
    s.setValue(QStringLiteral("window/formatBar"), a[QStringLiteral("showFormatBar")]->isChecked());
    s.sync();
}

bool MainWindow::switchProfile(const QString &name)
{
    if (name.compare(profiles::active(), Qt::CaseInsensitive) == 0)
        return true;
    if (!profileui::confirmLowerBackupKeep(this, name))
        return false;
    const QStringList refused = profileui::askPlacesAtSwitch(this, name); // a profile another computer wrote
    saveWindowSettings(); // what the window is now goes into the profile that is left
    const profiles::SwitchReport r = profiles::switchTo(name, refused);
    if (!r.ok) {
        QMessageBox::warning(this, tr("Switch Profile"), r.error);
        return false;
    }
    reloadFromProfile(r.languageChanged);
    if (!r.warning.isEmpty())
        QMessageBox::warning(this, tr("Switch Profile"), r.warning);
    return true;
}

// the program takes everything of the settings anew (the profile just put there)
void MainWindow::reloadFromProfile(bool languageChanged)
{
    AppSettings &s = AppSettings::instance();
    s.reloadAll();
    MindMapPrefs::instance() = MindMapPrefs();
    MindMapPrefs::instance().load();
    commands::reload(); // the keyboard set and the shortcuts
    QSettings st;
    // the window: size and place (kept within the screen), the toolbars' places, the panes
    if (st.contains(QStringLiteral("window/geometry")) && !isMaximized() && !isFullScreen()) {
        restoreGeometry(st.value(QStringLiteral("window/geometry")).toByteArray());
        fitWindowToScreen();
    }
    if (st.contains(QStringLiteral("window/state")))
        restoreState(st.value(QStringLiteral("window/state")).toByteArray());
    if (m_barDock)
        m_barDock->load();
    loadPaneSettings(st);
    a[QStringLiteral("showInspector")]->setChecked(st.value(QStringLiteral("window/inspector"), true).toBool());
    a[QStringLiteral("showBinder")]->setChecked(st.value(QStringLiteral("window/binder"), true).toBool());
    a[QStringLiteral("showToolbar")]->setChecked(st.value(QStringLiteral("window/toolbar"), true).toBool());
    a[QStringLiteral("showFormatBar")]->setChecked(st.value(QStringLiteral("window/formatBar"), true).toBool());
    applyBarVisibility();
    // the theme, colors, toolbars and everything applySettings() reads from the settings
    s.notifyChanged();
    // the open project shows the profile's look (as when a theme is chosen in View > Themes)
    if (m_project) {
        const look::Values want = look::forNewProjects();
        if (look::active() != want) {
            QString err;
            if (!look::change(want, &err))
                QMessageBox::warning(this, tr("Switch Profile"), err);
        }
    }
    icons::clearCache();
    rebuildIconBar();
    rebuildLabelBar();
    refreshProfileUi();
    QTimer::singleShot(0, this, [this] {
        fitPanesToScreen();
        if (m_barDock)
            m_barDock->refit();
    });
    QTimer::singleShot(0, &toolhost::host(), [] { toolhost::host().notifySettingsChanged(); });
    if (languageChanged)
        showStatusNote(tr("The profile sets another language; it takes effect when the program is started again."), 8000);
}

// the window within the screen's usable area (a profile made on a larger screen)
void MainWindow::fitWindowToScreen()
{
    if (isMaximized() || isFullScreen())
        return;
    QScreen *sc = screen() ? screen() : QGuiApplication::primaryScreen();
    if (!sc)
        return;
    const QRect avail = sc->availableGeometry();
    const QSize frame = frameGeometry().size() - size();
    const QSize want = size().boundedTo(avail.size() - frame).expandedTo(QSize(minimumSizeHint().width(), 200).boundedTo(avail.size() - frame));
    if (want != size())
        resize(want);
    const QRect fg = frameGeometry();
    const int x = qBound(avail.left(), fg.left(), qMax(avail.left(), avail.right() - fg.width() + 1));
    const int y = qBound(avail.top(), fg.top(), qMax(avail.top(), avail.bottom() - fg.height() + 1));
    if (x != fg.left() || y != fg.top())
        move(x, y);
}

void MainWindow::fitPanesToScreen()
{
    if (m_upright || !m_mainSplit)
        return;
    int total = m_mainSplit->width() - 2 * m_mainSplit->handleWidth();
    if (total < 200)
        total = width() - 2 * m_mainSplit->handleWidth();
    QList<int> in;
    for (int p = 0; p < 3; ++p)
        in << (paneWidget(p)->isHidden() ? 0 : m_wideSizes[p]);
    // a hidden column takes no room: the others share the width
    const QList<int> out = profiles::fitColumns(in, total);
    for (int p = 0; p < 3; ++p)
        if (!paneWidget(p)->isHidden() && out[p] > 0)
            m_wideSizes[p] = out[p];
    applyPaneSizes();
    capturePaneSizes();
}

} // namespace qb
