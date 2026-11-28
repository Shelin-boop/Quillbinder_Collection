#include "ui/LiveSyncDialogs.h"

#include "ui/Ask.h"
#include "ui/HintLabel.h"

#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace qb {
namespace livesync {

DialogHooks &dialogHooks()
{
    static DialogHooks h;
    return h;
}

bool confirmAction(QWidget *parent, const QString &title, const QString &text, const QString &action)
{
    if (dialogHooks().confirm)
        return dialogHooks().confirm(parent, title, text, action);
    return ask::confirm(parent, title, text, action);
}

QString inputText(QWidget *parent, const QString &title, const QString &label)
{
    if (dialogHooks().inputText)
        return dialogHooks().inputText(parent, title, label);
    bool ok = false;
    const QString t = QInputDialog::getText(parent, title, label, QLineEdit::Normal, QString(), &ok);
    return ok ? t.trimmed() : QString();
}

QString pickItem(QWidget *parent, const QString &title, const QString &label, const QStringList &items)
{
    if (dialogHooks().pickItem)
        return dialogHooks().pickItem(parent, title, label, items);
    bool ok = false;
    const QString t = QInputDialog::getItem(parent, title, label, items, 0, false, &ok);
    return ok ? t : QString();
}

bool stopSharingFlow(QWidget *parent, const QString &uuid)
{
    const collab::ProjectStatus p = liveSyncControl().model()->project(uuid);
    if (!p.valid)
        return false;
    int online = 0;
    for (const collab::ParticipantStatus &x : p.participants)
        if (x.online && !x.thisDevice)
            ++online;
    QString text;
    if (p.thisDeviceHosts())
        text = QCoreApplication::translate("LiveSync", "Stop sharing \"%1\"? The other devices keep their copies as projects of their own that "
                                                        "no longer follow this one.")
                   .arg(p.title);
    else
        text = QCoreApplication::translate("LiveSync", "Leave \"%1\"? The copy on this device stays as a project of its own that no longer "
                                                        "follows the host. Changes not sent yet are kept as recovery drafts.")
                   .arg(p.title);
    if (online > 0)
        text += QLatin1Char(' ') + QCoreApplication::translate("LiveSync", "%n other device(s) are connected now.", nullptr, online);
    if (p.pendingChanges > 0)
        text += QLatin1Char(' ') + QCoreApplication::translate("LiveSync", "%n change(s) are not confirmed yet.", nullptr, p.pendingChanges);
    const QString action = p.thisDeviceHosts() ? QCoreApplication::translate("LiveSync", "Stop Sharing")
                                               : QCoreApplication::translate("LiveSync", "Leave");
    if (!confirmAction(parent, action, text, action))
        return false;
    liveSyncControl().actions()->stopSharing(uuid);
    return true;
}

bool transferHostingFlow(QWidget *parent, const QString &uuid)
{
    QStringList names;
    QHash<QString, QString> ids;
    for (const AccessEntry &e : liveSyncControl().access(uuid))
        if (e.role == AccessEntry::Editor && e.mayHost && e.online) {
            names << e.name;
            ids.insert(e.name, e.deviceId);
        }
    if (names.isEmpty())
        return false;
    const QString pick = pickItem(parent, QCoreApplication::translate("LiveSync", "Transfer Hosting"),
                                  QCoreApplication::translate("LiveSync", "Hand hosting of this project to:"), names);
    if (pick.isEmpty() || !ids.contains(pick))
        return false;
    liveSyncControl().transferHosting(uuid, ids.value(pick));
    return true;
}

QString groupedCode(const QString &digits)
{
    QString d = digits;
    d.remove(QLatin1Char(' '));
    if (d.size() == 6)
        return d.left(3) + QLatin1Char(' ') + d.mid(3);
    return d;
}

namespace {

QFont bigFont(const QWidget *w)
{
    QFont f = w->font();
    f.setPointSizeF(f.pointSizeF() * 2.6);
    f.setBold(true);
    return f;
}

void nonModal(QDialog *d)
{
    d->setModal(false);
    d->setAttribute(Qt::WA_DeleteOnClose);
}

QString roleText(AccessEntry::Role r)
{
    switch (r) {
    case AccessEntry::Editor:
        return QCoreApplication::translate("LiveSync", "Editor");
    case AccessEntry::Reader:
        return QCoreApplication::translate("LiveSync", "Reader");
    case AccessEntry::None:
        break;
    }
    return QCoreApplication::translate("LiveSync", "None");
}

} // namespace

QString backupText(const collab::BackupStatus &b)
{
    QString state;
    switch (b.state) {
    case collab::BackupStatus::None:
        state = QCoreApplication::translate("LiveSync", "No backup yet");
        break;
    case collab::BackupStatus::Running:
        state = QCoreApplication::translate("LiveSync", "Backup running");
        break;
    case collab::BackupStatus::Done:
        state = QCoreApplication::translate("LiveSync", "Backup done");
        break;
    case collab::BackupStatus::Waiting:
        state = QCoreApplication::translate("LiveSync", "Backup waiting");
        break;
    case collab::BackupStatus::Incomplete:
        state = QCoreApplication::translate("LiveSync", "Backup incomplete");
        break;
    case collab::BackupStatus::Failed:
        state = QCoreApplication::translate("LiveSync", "Backup failed");
        break;
    }
    if (!b.detail.isEmpty())
        state += QStringLiteral(": ") + b.detail;
    if (b.lastSuccess.isValid())
        state += QStringLiteral(". ") + QCoreApplication::translate("LiveSync", "Last successful backup: %1")
                                            .arg(QLocale().toString(b.lastSuccess, QLocale::ShortFormat));
    return state;
}

// ================================================================== pairing

PairingDialog::PairingDialog(QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("pairingDialog"));
    setWindowTitle(tr("Pair a Device"));
    nonModal(this);
    auto *v = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("To pair two devices, open the pairing window on one of them (it stays open for two minutes), then pick "
                                "that device here on the other. Both show the same six digits: compare them and confirm."),
                             this);
    intro->setWordWrap(true);
    hintlabel::style(intro);
    v->addWidget(intro);
    m_state = new QLabel(this);
    m_state->setObjectName(QStringLiteral("pairState"));
    m_state->setWordWrap(true);
    v->addWidget(m_state);

    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("pairList"));
    m_list->setAccessibleName(tr("Devices with an open pairing window"));
    m_list->setMinimumHeight(90);
    v->addWidget(m_list);
    auto *row = new QHBoxLayout;
    m_openWindow = new QPushButton(tr("Open the Pairing Window on This Device"), this);
    m_openWindow->setObjectName(QStringLiteral("pairOpenWindow"));
    m_pair = new QPushButton(tr("Pair With Selected Device"), this);
    m_pair->setObjectName(QStringLiteral("pairSelected"));
    m_address = new QPushButton(tr("Connect by Address..."), this);
    m_address->setObjectName(QStringLiteral("pairByAddress"));
    row->addWidget(m_openWindow);
    row->addWidget(m_pair);
    row->addWidget(m_address);
    row->addStretch(1);
    v->addLayout(row);

    m_codeBox = new QWidget(this);
    m_codeBox->setObjectName(QStringLiteral("pairCodeBox"));
    auto *cv = new QVBoxLayout(m_codeBox);
    m_other = new QLabel(m_codeBox);
    m_other->setObjectName(QStringLiteral("pairOther"));
    m_other->setAlignment(Qt::AlignHCenter);
    m_code = new QLabel(m_codeBox);
    m_code->setObjectName(QStringLiteral("pairCode"));
    m_code->setAlignment(Qt::AlignHCenter);
    m_code->setFont(bigFont(this));
    m_code->setMinimumHeight(QFontMetrics(m_code->font()).height() + 12);
    m_code->setAccessibleName(tr("Pairing code"));
    cv->addWidget(m_other);
    cv->addWidget(m_code);
    auto *crow = new QHBoxLayout;
    m_match = new QPushButton(tr("Codes Match"), m_codeBox);
    m_match->setObjectName(QStringLiteral("pairMatch"));
    m_differ = new QPushButton(tr("They Differ"), m_codeBox);
    m_differ->setObjectName(QStringLiteral("pairDiffer"));
    crow->addStretch(1);
    crow->addWidget(m_match);
    crow->addWidget(m_differ);
    crow->addStretch(1);
    cv->addLayout(crow);
    v->addWidget(m_codeBox);
    m_codeBox->hide();

    auto *close = new QPushButton(tr("Close"), this);
    close->setObjectName(QStringLiteral("pairClose"));
    auto *crow2 = new QHBoxLayout;
    crow2->addStretch(1);
    crow2->addWidget(close);
    v->addLayout(crow2);

    LiveSyncControl &c = liveSyncControl();
    connect(m_openWindow, &QPushButton::clicked, this, [&c, this] {
        if (c.pairingWindowOpen())
            c.closePairingWindow();
        else
            c.openPairingWindow();
        refreshState();
    });
    connect(m_pair, &QPushButton::clicked, this, [&c, this] {
        QListWidgetItem *it = m_list->currentItem();
        if (!it)
            return;
        c.pairWith(it->data(Qt::UserRole).toString());
        m_state->setText(tr("Waiting for the other device..."));
    });
    connect(m_list, &QListWidget::itemDoubleClicked, m_pair, &QPushButton::click);
    connect(m_list, &QListWidget::currentRowChanged, this, [this] { refreshState(); });
    connect(m_address, &QPushButton::clicked, this, [&c, this] {
        const QString a = inputText(this, tr("Connect by Address"), tr("Address of the other device (name or number, and a port if it is not the usual one):"));
        if (a.isEmpty())
            return;
        c.pairWithAddress(a);
        m_state->setText(tr("Waiting for the other device..."));
    });
    connect(m_match, &QPushButton::clicked, this, [&c, this] {
        c.answerPairing(true);
        m_match->setEnabled(false);
        m_differ->setEnabled(false);
        m_state->setText(tr("Waiting for the other device to confirm..."));
    });
    connect(m_differ, &QPushButton::clicked, this, [&c, this] {
        c.answerPairing(false);
        m_codeBox->hide();
        m_codeShown = false;
        m_state->setText(tr("Pairing stopped: the codes differ. Someone else may be between the devices; do not pair them."));
    });
    connect(close, &QPushButton::clicked, this, &QDialog::close);
    connect(&c, &LiveSyncControl::discoveredChanged, this, &PairingDialog::refreshList);
    connect(&c, &LiveSyncControl::pairingWindowChanged, this, &PairingDialog::refreshState);
    connect(&c, &LiveSyncControl::pairingCode, this, &PairingDialog::showCode);
    connect(&c, &LiveSyncControl::pairingFinished, this, &PairingDialog::finished);
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &PairingDialog::refreshList);
    m_timer->start();
    refreshList();
    refreshState();
}

PairingDialog::~PairingDialog()
{
    // a dialog closed while a code waits for an answer counts as "They Differ" (nothing is paired by closing)
    if (m_codeShown)
        liveSyncControl().cancelPairing();
}

void PairingDialog::refreshList()
{
    const QString keep = m_list->currentItem() ? m_list->currentItem()->data(Qt::UserRole).toString() : QString();
    QStringList now;
    for (const DiscoveredDevice &d : liveSyncControl().discovered())
        if (d.pairingOpen && !d.paired)
            now << d.key + QLatin1Char('\n') + (d.name.isEmpty() ? d.address : d.name);
    QStringList shown;
    for (int i = 0; i < m_list->count(); ++i)
        shown << m_list->item(i)->data(Qt::UserRole).toString() + QLatin1Char('\n') + m_list->item(i)->text();
    if (now == shown)
        return;
    m_list->clear();
    for (const QString &n : now) {
        const int nl = int(n.indexOf(QLatin1Char('\n')));
        auto *it = new QListWidgetItem(n.mid(nl + 1), m_list);
        it->setData(Qt::UserRole, n.left(nl));
        if (n.left(nl) == keep)
            m_list->setCurrentItem(it);
    }
    refreshState();
}

void PairingDialog::refreshState()
{
    LiveSyncControl &c = liveSyncControl();
    m_openWindow->setText(c.pairingWindowOpen() ? tr("Close the Pairing Window") : tr("Open the Pairing Window on This Device"));
    m_pair->setEnabled(c.enabled() && m_list->currentItem() != nullptr && !m_codeShown);
    m_address->setEnabled(c.enabled() && !m_codeShown);
    m_openWindow->setEnabled(c.enabled() && !m_codeShown);
    if (m_codeShown)
        return;
    if (!c.enabled())
        m_state->setText(tr("Live Sync is off. Turn it on in File > Options > Live Sync first."));
    else if (c.pairingWindowOpen())
        m_state->setText(tr("The pairing window of this device is open. On the other device, pick this one."));
    else if (m_list->count() == 0)
        m_state->setText(tr("No device with an open pairing window was found. Open the window on the other device, or connect by address."));
    else
        m_state->setText(tr("Pick the device to pair with."));
}

void PairingDialog::showCode(const QString &otherName, const QString &code)
{
    m_codeShown = true;
    m_other->setText(tr("Pairing with %1").arg(otherName));
    m_code->setText(groupedCode(code));
    m_match->setEnabled(true);
    m_differ->setEnabled(true);
    m_codeBox->show();
    adjustSize();
    m_state->setText(tr("Compare these digits with those on the other device."));
    refreshState();
    show();
    raise();
}

void PairingDialog::finished(bool ok, const QString &text)
{
    m_codeShown = false;
    m_codeBox->hide();
    m_state->setText(ok ? tr("Paired. %1").arg(text) : tr("Pairing failed. %1").arg(text));
    refreshState();
    if (ok)
        m_state->setText(tr("Paired. %1").arg(text));
}

// ================================================================== join

JoinDialog::JoinDialog(QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("joinDialog"));
    setWindowTitle(tr("Join a Project"));
    nonModal(this);
    resize(520, 360);
    auto *v = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("Projects shared by your paired devices. Joining makes a full copy on this device, kept up to date "
                                "while you are connected."),
                             this);
    intro->setWordWrap(true);
    hintlabel::style(intro);
    v->addWidget(intro);
    m_tree = new QTreeWidget(this);
    m_tree->setObjectName(QStringLiteral("joinTree"));
    m_tree->setHeaderLabels({tr("Project"), tr("Shared by")});
    m_tree->setRootIsDecorated(false);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    v->addWidget(m_tree, 1);
    m_note = new QLabel(this);
    m_note->setObjectName(QStringLiteral("joinNote"));
    m_note->setWordWrap(true);
    v->addWidget(m_note);
    m_progress = new QProgressBar(this);
    m_progress->setObjectName(QStringLiteral("joinProgress"));
    m_progress->setRange(0, 100);
    m_progress->hide();
    v->addWidget(m_progress);
    auto *row = new QHBoxLayout;
    m_pairFirst = new QPushButton(tr("Pair a Device..."), this);
    m_pairFirst->setObjectName(QStringLiteral("joinPairFirst"));
    m_join = new QPushButton(tr("Join"), this);
    m_join->setObjectName(QStringLiteral("joinButton"));
    m_join->setEnabled(false);
    auto *close = new QPushButton(tr("Close"), this);
    close->setObjectName(QStringLiteral("joinClose"));
    row->addWidget(m_pairFirst);
    row->addStretch(1);
    row->addWidget(m_join);
    row->addWidget(close);
    v->addLayout(row);

    LiveSyncControl &c = liveSyncControl();
    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this] {
        QTreeWidgetItem *it = m_tree->currentItem();
        m_join->setEnabled(it && !it->data(0, Qt::UserRole).toString().isEmpty());
    });
    connect(m_join, &QPushButton::clicked, this, [&c, this] {
        QTreeWidgetItem *it = m_tree->currentItem();
        if (!it || it->data(0, Qt::UserRole).toString().isEmpty())
            return;
        m_join->setEnabled(false);
        m_progress->setValue(0);
        m_progress->show();
        m_note->setText(tr("Copying the project..."));
        c.join(it->data(1, Qt::UserRole).toString(), it->data(0, Qt::UserRole).toString());
    });
    connect(m_pairFirst, &QPushButton::clicked, this, [this] { showPairingDialog(parentWidget()); });
    connect(close, &QPushButton::clicked, this, &QDialog::close);
    connect(&c, &LiveSyncControl::joinProgress, this, [this](const QString &, int percent, const QString &text) {
        m_progress->show();
        m_progress->setValue(percent);
        if (!text.isEmpty())
            m_note->setText(text);
    });
    connect(&c, &LiveSyncControl::joinFinished, this, [this](const QString &, bool ok, const QString &folderOrError) {
        m_progress->hide();
        if (ok) {
            m_note->setText(tr("The project is here. Opening it..."));
            emit openProject(folderOrError);
            this->close();
        } else {
            m_note->setText(tr("The project could not be joined: %1").arg(folderOrError));
            refresh();
        }
    });
    connect(&c, &LiveSyncControl::discoveredChanged, this, &JoinDialog::refresh);
    connect(c.model(), &collab::StatusModel::devicesChanged, this, &JoinDialog::refresh);
    refresh();
}

void JoinDialog::refresh()
{
    LiveSyncControl &c = liveSyncControl();
    const QString keep = m_tree->currentItem() ? m_tree->currentItem()->data(0, Qt::UserRole).toString() : QString();
    m_tree->clear();
    if (!c.enabled()) {
        m_note->setText(tr("Live Sync is off. Turn it on in File > Options > Live Sync first."));
        return;
    }
    QStringList pairedWithProjects;
    for (const RemoteProject &r : c.remoteProjects()) {
        auto *it = new QTreeWidgetItem(m_tree, {r.title, r.hostOnline ? r.hostName : tr("%1 (not reachable)").arg(r.hostName)});
        it->setData(0, Qt::UserRole, r.uuid);
        it->setData(1, Qt::UserRole, r.hostDeviceId);
        if (!r.hostOnline)
            it->setFlags(it->flags() & ~Qt::ItemIsEnabled);
        if (r.uuid == keep)
            m_tree->setCurrentItem(it);
        pairedWithProjects << r.hostDeviceId;
    }
    // hosts found in the network that are not paired: only a hint
    for (const DiscoveredDevice &d : c.discovered())
        if (!d.paired) {
            auto *it = new QTreeWidgetItem(m_tree, {tr("Pair first"), d.name.isEmpty() ? d.address : d.name});
            it->setFlags(it->flags() & ~Qt::ItemIsSelectable);
            it->setToolTip(0, tr("Pair this device first (Pair a Device...); then its projects are listed here."));
        }
    if (m_tree->topLevelItemCount() == 0)
        m_note->setText(tr("No shared projects found. Pair the device that shares the project first."));
    else if (!m_progress->isVisible())
        m_note->clear();
}

// ================================================================== hosted projects

HostedProjectsDialog::HostedProjectsDialog(QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("hostedDialog"));
    setWindowTitle(tr("Hosted Projects"));
    nonModal(this);
    resize(560, 420);
    auto *v = new QVBoxLayout(this);
    m_projects = new QComboBox(this);
    m_projects->setObjectName(QStringLiteral("hostedProjects"));
    m_projects->setAccessibleName(tr("Project"));
    v->addWidget(m_projects);
    m_devices = new QTreeWidget(this);
    m_devices->setObjectName(QStringLiteral("hostedDevices"));
    m_devices->setHeaderLabels({tr("Device"), tr("Access"), tr("State")});
    m_devices->setRootIsDecorated(false);
    m_devices->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    v->addWidget(m_devices, 1);
    auto *hint = new QLabel(tr("The order of the list is the order in which devices take over hosting if this one is lost."), this);
    hint->setWordWrap(true);
    hintlabel::style(hint);
    v->addWidget(hint);
    m_backup = new QLabel(this);
    m_backup->setObjectName(QStringLiteral("hostedBackup"));
    m_backup->setWordWrap(true);
    v->addWidget(m_backup);
    auto *row = new QHBoxLayout;
    m_up = new QPushButton(tr("Move Up"), this);
    m_up->setObjectName(QStringLiteral("hostedUp"));
    m_down = new QPushButton(tr("Move Down"), this);
    m_down->setObjectName(QStringLiteral("hostedDown"));
    m_transfer = new QPushButton(tr("Transfer Hosting..."), this);
    m_transfer->setObjectName(QStringLiteral("hostedTransfer"));
    m_stop = new QPushButton(tr("Stop Sharing"), this);
    m_stop->setObjectName(QStringLiteral("hostedStop"));
    auto *close = new QPushButton(tr("Close"), this);
    close->setObjectName(QStringLiteral("hostedClose"));
    row->addWidget(m_up);
    row->addWidget(m_down);
    row->addStretch(1);
    row->addWidget(m_transfer);
    row->addWidget(m_stop);
    row->addWidget(close);
    v->addLayout(row);

    LiveSyncControl &c = liveSyncControl();
    connect(m_projects, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { refreshAccess(); });
    auto move = [this](int by) {
        QTreeWidgetItem *it = m_devices->currentItem();
        if (!it)
            return;
        const int i = m_devices->indexOfTopLevelItem(it);
        const int j = i + by;
        if (j < 0 || j >= m_devices->topLevelItemCount())
            return;
        QStringList order;
        for (int k = 0; k < m_devices->topLevelItemCount(); ++k)
            order << m_devices->topLevelItem(k)->data(0, Qt::UserRole).toString();
        order.move(i, j);
        liveSyncControl().setPriority(currentUuid(), order);
        refreshAccess();
        m_devices->setCurrentItem(m_devices->topLevelItem(j));
    };
    connect(m_up, &QPushButton::clicked, this, [move] { move(-1); });
    connect(m_down, &QPushButton::clicked, this, [move] { move(1); });
    connect(m_transfer, &QPushButton::clicked, this, [this] { transferHostingFlow(this, currentUuid()); });
    connect(m_stop, &QPushButton::clicked, this, [this] { stopSharingFlow(this, currentUuid()); });
    connect(&c, &LiveSyncControl::accessChanged, this, [this](const QString &uuid) {
        if (uuid == currentUuid())
            refreshAccess();
    });
    connect(c.model(), &collab::StatusModel::projectsChanged, this, &HostedProjectsDialog::refreshProjects);
    connect(c.model(), &collab::StatusModel::projectChanged, this, [this](const QString &uuid) {
        if (uuid == currentUuid())
            refreshAccess();
    });
    connect(close, &QPushButton::clicked, this, &QDialog::close);
    refreshProjects();
}

QString HostedProjectsDialog::currentUuid() const { return m_projects->currentData().toString(); }

void HostedProjectsDialog::selectProject(const QString &uuid)
{
    const int i = m_projects->findData(uuid);
    if (i >= 0)
        m_projects->setCurrentIndex(i);
}

void HostedProjectsDialog::refreshProjects()
{
    const QString keep = currentUuid();
    QSignalBlocker b(m_projects);
    m_projects->clear();
    for (const collab::ProjectStatus &p : liveSyncControl().model()->projects())
        if (p.thisDeviceHosts())
            m_projects->addItem(p.title, p.uuid);
    const int i = m_projects->findData(keep);
    if (i >= 0)
        m_projects->setCurrentIndex(i);
    b.unblock();
    refreshAccess();
}

void HostedProjectsDialog::refreshAccess()
{
    const QString uuid = currentUuid();
    const QString keep = m_devices->currentItem() ? m_devices->currentItem()->data(0, Qt::UserRole).toString() : QString();
    m_devices->clear();
    const collab::ProjectStatus p = liveSyncControl().model()->project(uuid);
    const bool have = p.valid && p.thisDeviceHosts();
    m_stop->setEnabled(have);
    m_up->setEnabled(have);
    m_down->setEnabled(have);
    m_transfer->setEnabled(have);
    m_backup->setText(have ? backupText(p.backup) : tr("This device hosts no shared project."));
    if (!have)
        return;
    for (const AccessEntry &e : liveSyncControl().access(uuid)) {
        auto *it = new QTreeWidgetItem(m_devices);
        it->setText(0, e.name);
        it->setData(0, Qt::UserRole, e.deviceId);
        it->setText(2, e.online ? tr("Online") : tr("Not connected"));
        auto *combo = new QComboBox(m_devices);
        combo->setObjectName(QStringLiteral("accessCombo:") + e.deviceId);
        combo->setAccessibleName(tr("Access of %1").arg(e.name));
        for (AccessEntry::Role r : {AccessEntry::Editor, AccessEntry::Reader, AccessEntry::None})
            combo->addItem(roleText(r), int(r));
        combo->setCurrentIndex(combo->findData(int(e.role)));
        const QString dev = e.deviceId;
        connect(combo, qOverload<int>(&QComboBox::activated), this, [this, combo, dev](int i) {
            liveSyncControl().setAccess(currentUuid(), dev, AccessEntry::Role(combo->itemData(i).toInt()));
        });
        m_devices->setItemWidget(it, 1, combo);
        if (e.deviceId == keep)
            m_devices->setCurrentItem(it);
    }
}

// ================================================================== recovery drafts

DraftsDialog::DraftsDialog(QWidget *parent) : QDialog(parent)
{
    setObjectName(QStringLiteral("draftsDialog"));
    setWindowTitle(tr("Recovery Drafts"));
    nonModal(this);
    resize(560, 360);
    auto *v = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("Changes that were kept on this device because they could not be sent. They are never deleted by "
                                "themselves."),
                             this);
    intro->setWordWrap(true);
    hintlabel::style(intro);
    v->addWidget(intro);
    m_tree = new QTreeWidget(this);
    m_tree->setObjectName(QStringLiteral("draftsTree"));
    m_tree->setHeaderLabels({tr("Time"), tr("Document"), tr("Why")});
    m_tree->setRootIsDecorated(false);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    v->addWidget(m_tree, 1);
    m_empty = new QLabel(tr("There are no recovery drafts."), this);
    m_empty->setObjectName(QStringLiteral("draftsEmpty"));
    v->addWidget(m_empty);
    m_note = new QLabel(this);
    m_note->setObjectName(QStringLiteral("draftNote"));
    m_note->setWordWrap(true);
    m_note->setAccessibleName(tr("What happened to the draft"));
    v->addWidget(m_note);
    auto *row = new QHBoxLayout;
    m_compare = new QPushButton(tr("Compare"), this);
    m_compare->setObjectName(QStringLiteral("draftCompare"));
    m_keep = new QPushButton(tr("Keep as Copy"), this);
    m_keep->setObjectName(QStringLiteral("draftKeep"));
    m_take = new QPushButton(tr("Take Over"), this);
    m_take->setObjectName(QStringLiteral("draftTake"));
    m_delete = new QPushButton(tr("Delete"), this);
    m_delete->setObjectName(QStringLiteral("draftDelete"));
    auto *close = new QPushButton(tr("Close"), this);
    close->setObjectName(QStringLiteral("draftClose"));
    for (QPushButton *b : {m_compare, m_keep, m_take, m_delete})
        row->addWidget(b);
    row->addStretch(1);
    row->addWidget(close);
    v->addLayout(row);

    LiveSyncControl &c = liveSyncControl();
    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this] {
        const bool on = !currentId().isEmpty();
        for (QPushButton *b : {m_compare, m_keep, m_take, m_delete})
            b->setEnabled(on);
    });
    connect(m_compare, &QPushButton::clicked, this, [this] { liveSyncControl().compareDraft(currentId()); });
    connect(m_keep, &QPushButton::clicked, this, [this] { liveSyncControl().keepDraftAsCopy(currentId()); });
    connect(m_take, &QPushButton::clicked, this, [this] {
        if (confirmAction(this, tr("Take Over"),
                          tr("Take over this draft? The document is locked for you, a snapshot of its present text is made first, and "
                             "the draft's text replaces it."),
                          tr("Take Over")))
            liveSyncControl().takeOverDraft(currentId());
    });
    connect(m_delete, &QPushButton::clicked, this, [this] {
        if (confirmAction(this, tr("Delete Draft"), tr("Delete this draft? Its text cannot be brought back."), tr("Delete Draft")))
            liveSyncControl().deleteDraft(currentId());
    });
    connect(close, &QPushButton::clicked, this, &QDialog::close);
    connect(&c, &LiveSyncControl::draftsChanged, this, &DraftsDialog::refresh);
    connect(&c, &LiveSyncControl::draftNote, this, [this](const QString &t) { m_note->setText(t); });
    refresh();
}

QString DraftsDialog::currentId() const
{
    QTreeWidgetItem *it = m_tree->currentItem();
    return it ? it->data(0, Qt::UserRole).toString() : QString();
}

void DraftsDialog::setProject(const QString &uuid)
{
    m_uuid = uuid;
    refresh();
}

void DraftsDialog::refresh()
{
    const QString keep = currentId();
    m_tree->clear();
    for (const DraftInfo &d : liveSyncControl().drafts(m_uuid)) {
        auto *it = new QTreeWidgetItem(m_tree, {QLocale().toString(d.when, QLocale::ShortFormat), d.docTitle, d.reason});
        it->setData(0, Qt::UserRole, d.id);
        if (d.id == keep)
            m_tree->setCurrentItem(it);
    }
    m_empty->setVisible(m_tree->topLevelItemCount() == 0);
    for (QPushButton *b : {m_compare, m_keep, m_take, m_delete})
        b->setEnabled(!currentId().isEmpty());
}

// ================================================================== opening

namespace {
template <class D> D *openOne(QPointer<D> &slot, QWidget *parent)
{
    // a window that was closed is deleted at the next event round: never raise one that is about to go
    if (!slot || !slot->isVisible())
        slot = new D(parent);
    slot->show();
    slot->raise();
    slot->activateWindow();
    return slot;
}
} // namespace

PairingDialog *showPairingDialog(QWidget *parent)
{
    static QPointer<PairingDialog> d;
    return openOne(d, parent);
}
JoinDialog *showJoinDialog(QWidget *parent)
{
    static QPointer<JoinDialog> d;
    return openOne(d, parent);
}
HostedProjectsDialog *showHostedProjectsDialog(QWidget *parent, const QString &uuid)
{
    static QPointer<HostedProjectsDialog> d;
    HostedProjectsDialog *h = openOne(d, parent);
    if (!uuid.isEmpty())
        h->selectProject(uuid);
    return h;
}
DraftsDialog *showDraftsDialog(QWidget *parent, const QString &uuid)
{
    static QPointer<DraftsDialog> d;
    DraftsDialog *h = openOne(d, parent);
    h->setProject(uuid);
    return h;
}

} // namespace livesync
} // namespace qb
