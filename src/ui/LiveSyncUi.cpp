#include "ui/LiveSyncUi.h"

#include "core/Project.h"
#include "ui/BinderPanel.h"
#include "ui/Inspector.h"
#include "ui/LiveSyncDialogs.h"
#include "ui/LiveSyncTab.h"
#include "ui/LiveSyncWallet.h"
#include "ui/WindowCare.h"

#include <QAction>
#include <QApplication>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QToolBar>
#include <QToolButton>

namespace qb {
namespace livesync {

namespace {

QColor stateColor(collab::ShareState s, const QColor &ink)
{
    switch (s) {
    case collab::ShareState::NotShared:
        return ink;
    case collab::ShareState::UpToDate:
        return QColor(40, 150, 70);
    case collab::ShareState::Sending:
        return QColor(40, 120, 210);
    case collab::ShareState::NotConnected:
        return QColor(215, 125, 20);
    case collab::ShareState::Problem:
        return QColor(205, 45, 45);
    }
    return ink;
}

} // namespace

QIcon stateIcon(collab::ShareState state, const QColor &ink, int side)
{
    QIcon icon;
    for (int px : {side, side * 2}) {
        QPixmap pm(px, px);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        const qreal u = px / 24.0;
        auto pt = [&](qreal x, qreal y) { return QPointF(x * u, y * u); };
        const QColor c = stateColor(state, ink);
        QPen pen(c, 2.0 * u);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        switch (state) {
        case collab::ShareState::NotShared: {
            // a dashed circle: nothing is connected
            QPen dashed = pen;
            dashed.setStyle(Qt::DashLine);
            p.setPen(dashed);
            p.drawEllipse(QRectF(pt(3.5, 3.5), pt(20.5, 20.5)));
            break;
        }
        case collab::ShareState::UpToDate:
            p.drawEllipse(QRectF(pt(3, 3), pt(21, 21)));
            p.drawPolyline(QPolygonF({pt(7.5, 12.5), pt(10.8, 16), pt(16.8, 8.5)}));
            break;
        case collab::ShareState::Sending: {
            // two arrows chasing each other
            p.drawArc(QRectF(pt(4, 4), pt(20, 20)), 30 * 16, 120 * 16);
            p.drawArc(QRectF(pt(4, 4), pt(20, 20)), 210 * 16, 120 * 16);
            p.setBrush(c);
            p.drawPolygon(QPolygonF({pt(14.5, 4.2), pt(20.5, 6.5), pt(16.2, 10.8)}));
            p.drawPolygon(QPolygonF({pt(9.5, 19.8), pt(3.5, 17.5), pt(7.8, 13.2)}));
            break;
        }
        case collab::ShareState::NotConnected:
            p.drawEllipse(QRectF(pt(3, 3), pt(21, 21)));
            p.drawLine(pt(6, 18), pt(18, 6));
            break;
        case collab::ShareState::Problem: {
            QPainterPath tri;
            tri.moveTo(pt(12, 3));
            tri.lineTo(pt(21.5, 20));
            tri.lineTo(pt(2.5, 20));
            tri.closeSubpath();
            p.drawPath(tri);
            p.drawLine(pt(12, 9), pt(12, 14));
            p.drawPoint(pt(12, 17));
            break;
        }
        }
        p.end();
        icon.addPixmap(pm);
    }
    return icon;
}

LiveSyncUi::LiveSyncUi(QWidget *window) : QObject(window), m_window(window)
{
    installWalletDialogs();
    LiveSyncControl &c = liveSyncControl();

    m_menu = new QMenu(window);
    m_menu->setObjectName(QStringLiteral("liveSyncMenu"));
    m_share = m_menu->addAction(tr("Share This Project..."));
    m_share->setObjectName(QStringLiteral("lsShareProject"));
    m_join = m_menu->addAction(tr("Join a Project..."));
    m_join->setObjectName(QStringLiteral("lsJoinProject"));
    m_menu->addSeparator();
    m_backup = m_menu->addAction(tr("Request Backup"));
    m_backup->setObjectName(QStringLiteral("lsRequestBackup"));
    m_transfer = m_menu->addAction(tr("Transfer Hosting..."));
    m_transfer->setObjectName(QStringLiteral("lsTransferHosting"));
    m_hosted = m_menu->addAction(tr("Hosted Projects..."));
    m_hosted->setObjectName(QStringLiteral("lsHostedProjects"));
    m_drafts = m_menu->addAction(tr("Recovery Drafts..."));
    m_drafts->setObjectName(QStringLiteral("lsRecoveryDrafts"));
    m_menu->addSeparator();
    m_stop = m_menu->addAction(tr("Stop Sharing"));
    m_stop->setObjectName(QStringLiteral("lsStopSharing"));

    connect(m_share, &QAction::triggered, this, [this] { share(); });
    connect(m_join, &QAction::triggered, this, [this] {
        JoinDialog *d = showJoinDialog(m_window);
        connect(d, &JoinDialog::openProject, this, &LiveSyncUi::openProjectRequested, Qt::UniqueConnection);
    });
    connect(m_backup, &QAction::triggered, this, [this] { liveSyncControl().actions()->requestBackup(projectUuid()); });
    connect(m_transfer, &QAction::triggered, this, [this] { transferHostingFlow(m_window, projectUuid()); });
    connect(m_hosted, &QAction::triggered, this, [this] { showHostedProjectsDialog(m_window, projectUuid()); });
    connect(m_drafts, &QAction::triggered, this, [this] { showDraftsDialog(m_window, projectUuid()); });
    connect(m_stop, &QAction::triggered, this, [this] { stopSharingFlow(m_window, projectUuid()); });
    connect(m_menu, &QMenu::aboutToShow, this, [this] { refreshMenu(); });

    m_tab = new LiveSyncTab;
    m_bars = new LiveSyncBars;
    m_tab->setContext(QString(), QString(), [this](const QString &id) { return documentTitle(id); });

    connect(c.model(), &collab::StatusModel::projectsChanged, this, &LiveSyncUi::refresh);
    connect(c.model(), &collab::StatusModel::projectChanged, this, &LiveSyncUi::refresh);
    connect(c.model(), &collab::StatusModel::locksChanged, this, &LiveSyncUi::refresh);
    connect(&c, &LiveSyncControl::enabledChanged, this, &LiveSyncUi::refresh);
    connect(&c, &LiveSyncControl::draftsChanged, this, &LiveSyncUi::refresh);
    connect(&c, &LiveSyncControl::pairingCode, this, [this](const QString &name, const QString &code) {
        showPairingDialog(m_window)->showCode(name, code);
    });
    connect(&c, &LiveSyncControl::foundUnknownHostRefused, this, [this](const QString &title) {
        auto *box = new QMessageBox(QMessageBox::Warning, tr("Live Sync"),
                                    tr("An unknown device tried to take over hosting of \"%1\". It was refused.").arg(title), QMessageBox::Ok,
                                    m_window);
        box->setAttribute(Qt::WA_DeleteOnClose);
        box->setModal(false);
        box->show();
    });
    refresh();
}

LiveSyncUi::~LiveSyncUi()
{
    // the tab and the bars belong to the inspector and the window once installed; else to nobody
    if (m_tab && !m_tab->parent())
        delete m_tab;
    if (m_bars && !m_bars->parent())
        delete m_bars;
}

QAction *LiveSyncUi::installToolbarButton(QToolBar *bar)
{
    // the button exists only while Live Sync is on: with it off the bar holds exactly what it held before Live Sync
    m_bar = bar;
    refreshButton();
    return m_buttonAction.data();
}

void LiveSyncUi::syncToolbarButton()
{
    if (!m_bar)
        return;
    if (active() && !m_buttonAction) {
        m_button = new QToolButton;
        m_button->setObjectName(QStringLiteral("liveSyncButton"));
        m_button->setMenu(m_menu);
        m_button->setPopupMode(QToolButton::InstantPopup);
        m_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        m_button->setAutoRaise(true);
        if (windowcare::tabletMode())
            m_button->setMinimumSize(48, 48); // a finger's size
        m_buttonAction = m_bar->addWidget(m_button); // the end of the bar; the action owns the button
        m_buttonAction->setObjectName(QStringLiteral("liveSyncButtonAction"));
    } else if (active() && m_buttonAction && !m_bar->actions().contains(m_buttonAction)) {
        m_bar->addAction(m_buttonAction); // the bar was built again (Customize Toolbars, Standard Set)
    } else if (!active() && m_buttonAction) {
        m_bar->removeAction(m_buttonAction);
        delete m_buttonAction.data(); // takes the button with it (the menu belongs to this object)
    }
}

void LiveSyncUi::installInspectorTab(Inspector *inspector)
{
    m_inspector = inspector;
    inspector->addExtraTab(QStringLiteral("livesync"), m_tab, stateIcon(collab::ShareState::UpToDate, inspector->palette().color(QPalette::WindowText), 20),
                           tr("Live Sync"));
    refresh();
}

void LiveSyncUi::attachBinder(BinderPanel *binder)
{
    m_binder = binder;
    refreshMarks();
}

QWidget *LiveSyncUi::bars() const { return m_bars.data(); }
LiveSyncTab *LiveSyncUi::tab() const { return m_tab.data(); }
LiveSyncBars *LiveSyncUi::barsWidget() const { return m_bars.data(); }

void LiveSyncUi::setProject(Project *project)
{
    m_project = project;
    m_docId.clear();
    refresh();
}

void LiveSyncUi::setSelectedDocument(const QString &docId)
{
    m_docId = docId;
    refresh();
}

QString LiveSyncUi::documentTitle(const QString &docId) const
{
    if (!m_project)
        return QString();
    const BinderItem *it = m_project->item(docId);
    return it ? it->title : QString();
}

bool LiveSyncUi::active() const { return liveSyncControl().enabled(); }

QString LiveSyncUi::projectUuid() const
{
    if (!m_project)
        return QString();
    return liveSyncControl().model()->projectOfFolder(m_project->folderPath()).uuid;
}

void LiveSyncUi::refresh()
{
    const QString uuid = projectUuid();
    const TitleResolver titles = [this](const QString &id) { return documentTitle(id); };
    m_tab->setContext(uuid, m_docId, titles);
    m_bars->setContext(uuid, m_docId, titles);
    refreshButton();
    refreshMenu();
    refreshMarks();
    if (m_inspector)
        m_inspector->setExtraTabVisible(QStringLiteral("livesync"), active());
}

void LiveSyncUi::refreshButton()
{
    syncToolbarButton();
    if (!m_button)
        return;
    LiveSyncControl &c = liveSyncControl();
    const collab::ProjectStatus p = c.model()->project(projectUuid());
    collab::ShareState state = p.valid ? p.state : c.model()->overallState();
    const QString text = collab::shareStateText(state);
    m_button->setText(text);
    m_button->setIcon(stateIcon(state, m_button->palette().color(QPalette::ButtonText)));
    m_button->setAccessibleName(tr("Live Sync, %1").arg(text));
    QString tip = tr("Live Sync: %1").arg(text);
    if (p.valid) {
        if (!p.thisDeviceHosts() && !p.hostName.isEmpty())
            tip += QLatin1Char('\n') + tr("Host: %1").arg(p.hostName);
        if (p.pendingChanges > 0)
            tip += QLatin1Char('\n') + tr("%n change(s) waiting to be sent", nullptr, p.pendingChanges);
        if (!p.problemText.isEmpty())
            tip += QLatin1Char('\n') + p.problemText;
        tip += QLatin1Char('\n') + backupText(p.backup);
    }
    m_button->setToolTip(tip);
}

void LiveSyncUi::refreshMenu()
{
    LiveSyncControl &c = liveSyncControl();
    const bool on = active();
    const QString uuid = projectUuid();
    const collab::ProjectStatus p = c.model()->project(uuid);
    const bool shared = p.valid;
    int hosted = 0, drafts = 0;
    for (const collab::ProjectStatus &x : c.model()->projects()) {
        if (x.thisDeviceHosts())
            ++hosted;
        drafts += x.draftCount;
    }
    bool canTransfer = false;
    if (on && shared && p.thisDeviceHosts())
        for (const AccessEntry &e : c.access(uuid))
            if (e.role == AccessEntry::Editor && e.mayHost && e.online)
                canTransfer = true;
    m_share->setEnabled(on && m_project && !shared);
    m_join->setEnabled(on);
    m_backup->setEnabled(on && shared);
    m_transfer->setEnabled(canTransfer);
    m_hosted->setEnabled(on && hosted > 0);
    m_drafts->setEnabled(on && drafts > 0);
    m_stop->setEnabled(on && shared);
    m_stop->setText(shared && !p.thisDeviceHosts() ? tr("Leave") : tr("Stop Sharing"));
}

void LiveSyncUi::refreshMarks()
{
    if (!m_binder)
        return;
    QHash<QString, QString> tips;
    QHash<QString, int> kinds;
    LiveSyncControl &c = liveSyncControl();
    const collab::ProjectStatus p = c.model()->project(projectUuid());
    if (active() && p.valid) {
        for (const collab::ParticipantStatus &x : p.participants)
            for (const QString &r : x.editing) {
                if (!r.startsWith(QLatin1String("doc:")))
                    continue;
                const QString id = r.mid(4);
                if (x.thisDevice) {
                    tips.insert(id, tr("You are editing this document"));
                    kinds.insert(id, 1);
                } else if (!x.online) {
                    tips.insert(id, x.lastSeen.isValid() ? tr("Not reachable since %1").arg(QLocale().toString(x.lastSeen, QLocale::ShortFormat))
                                                          : tr("Not reachable"));
                    kinds.insert(id, 2);
                } else {
                    tips.insert(id, tr("Being edited on %1").arg(x.name));
                }
            }
    } else {
        // nothing of Live Sync to show: leave the marks of the sync-program lock (the main window) alone, unless ours are there
        m_binder->setLockKinds({});
        if (m_marked)
            m_binder->setLockMarks({});
        m_marked = false;
        return;
    }
    m_marked = true;
    m_binder->setLockMarks(tips);
    m_binder->setLockKinds(kinds);
}

void LiveSyncUi::share()
{
    if (!m_project)
        return;
    QString error;
    if (!liveSyncControl().shareProject(m_project->folderPath(), &error)) {
        auto *box = new QMessageBox(QMessageBox::Warning, tr("Share This Project"),
                                    tr("This project cannot be shared: %1").arg(error.isEmpty() ? tr("Live Sync is not running.") : error),
                                    QMessageBox::Ok, m_window);
        box->setAttribute(Qt::WA_DeleteOnClose);
        box->setModal(false);
        box->show();
    }
}

} // namespace livesync
} // namespace qb
