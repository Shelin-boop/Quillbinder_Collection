#include "ui/LiveSyncPage.h"

#include "ui/HintLabel.h"
#include "ui/LiveSyncControl.h"
#include "ui/LiveSyncDialogs.h"
#include "ui/LiveSyncWallet.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace qb {
namespace livesync {

LiveSyncPage::LiveSyncPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("liveSyncPage"));
    installWalletDialogs();
    auto *v = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("Live Sync keeps one project the same on several of your devices in the same network, while they are "
                                "connected. It is off until you turn it on. Content travels encrypted between your paired devices; "
                                "the files of a project that is not encrypted stay readable on each device, and so do ZIP backups "
                                "of such a project: use your device's own encryption and an encrypted backup drive."),
                             this);
    intro->setWordWrap(true);
    hintlabel::style(intro);
    v->addWidget(intro);

    m_on = new QCheckBox(tr("Turn on Live Sync"), this);
    m_on->setObjectName(QStringLiteral("liveSyncOn"));
    v->addWidget(m_on);
    m_reason = new QLabel(this);
    m_reason->setObjectName(QStringLiteral("liveSyncReason"));
    m_reason->setWordWrap(true);
    v->addWidget(m_reason);

    auto *form = new QFormLayout;
    m_name = new QLineEdit(this);
    m_name->setObjectName(QStringLiteral("liveSyncName"));
    m_name->setMaxLength(60);
    form->addRow(tr("This device's name:"), m_name);
    v->addLayout(form);
    m_keep = new QCheckBox(tr("Keep hosting when the last window is closed"), this);
    m_keep->setObjectName(QStringLiteral("liveSyncKeep"));
    v->addWidget(m_keep);
    m_keys = new QLabel(keyPlaceText(), this);
    m_keys->setObjectName(QStringLiteral("liveSyncKeyPlace"));
    m_keys->setWordWrap(true);
    hintlabel::style(m_keys);
    v->addWidget(m_keys);

    auto *dev = new QGroupBox(tr("Paired Devices"), this);
    auto *dv = new QVBoxLayout(dev);
    m_devices = new QTreeWidget(dev);
    m_devices->setObjectName(QStringLiteral("liveSyncDevices"));
    m_devices->setAccessibleName(tr("Paired Devices"));
    m_devices->setRootIsDecorated(false);
    m_devices->setHeaderLabels({tr("Name"), tr("Last seen")});
    m_devices->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_devices->setMinimumHeight(100);
    dv->addWidget(m_devices);
    auto *row = new QHBoxLayout;
    m_pair = new QPushButton(tr("Pair a Device..."), dev);
    m_pair->setObjectName(QStringLiteral("liveSyncPair"));
    m_remove = new QPushButton(tr("Remove"), dev);
    m_remove->setObjectName(QStringLiteral("liveSyncRemove"));
    row->addWidget(m_pair);
    row->addWidget(m_remove);
    row->addStretch(1);
    dv->addLayout(row);
    v->addWidget(dev);

    auto *more = new QWidget(this);
    auto *mv = new QVBoxLayout(more);
    mv->setContentsMargins(20, 0, 0, 0);
    m_addresses = new QLabel(more);
    m_addresses->setObjectName(QStringLiteral("liveSyncAddresses"));
    m_addresses->setWordWrap(true);
    m_addresses->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mv->addWidget(m_addresses);
    m_byAddress = new QPushButton(tr("Connect by Address..."), more);
    m_byAddress->setObjectName(QStringLiteral("liveSyncByAddress"));
    auto *brow = new QHBoxLayout;
    brow->addWidget(m_byAddress);
    brow->addStretch(1);
    mv->addLayout(brow);
    v->addWidget(folded(tr("More Ways to Connect"), "liveSyncMore", more));

    auto *details = new QWidget(this);
    auto *tv = new QVBoxLayout(details);
    tv->setContentsMargins(20, 0, 0, 0);
    m_details = new QLabel(details);
    m_details->setObjectName(QStringLiteral("liveSyncDetails"));
    m_details->setWordWrap(true);
    m_details->setTextInteractionFlags(Qt::TextSelectableByMouse);
    tv->addWidget(m_details);
    v->addWidget(folded(tr("Details"), "liveSyncDetailsFold", details));
    v->addStretch(1);

    LiveSyncControl &c = liveSyncControl();
    connect(m_on, &QCheckBox::toggled, this, [this](bool on) {
        if (m_loading)
            return;
        liveSyncControl().setEnabled(on);
        refresh();
    });
    connect(m_name, &QLineEdit::editingFinished, this, [this] {
        if (!m_loading && !m_name->text().trimmed().isEmpty())
            liveSyncControl().setDeviceName(m_name->text());
    });
    connect(m_keep, &QCheckBox::toggled, this, [this](bool on) {
        if (!m_loading)
            liveSyncControl().setKeepHosting(on);
    });
    connect(m_pair, &QPushButton::clicked, this, [this] { showPairingDialog(window()); });
    connect(m_byAddress, &QPushButton::clicked, this, [this] {
        const QString a = inputText(this, tr("Connect by Address"), tr("Address of the other device (name or number, and a port if it is not the usual one):"));
        if (a.isEmpty())
            return;
        showPairingDialog(window());
        liveSyncControl().pairWithAddress(a);
    });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        QTreeWidgetItem *it = m_devices->currentItem();
        if (!it)
            return;
        if (!confirmAction(this, tr("Remove Device"),
                           tr("Remove \"%1\"? It loses access to every project of this device at once, and has to be paired again to "
                              "take part.")
                               .arg(it->text(0)),
                           tr("Remove")))
            return;
        liveSyncControl().removeDevice(it->data(0, Qt::UserRole).toString());
    });
    connect(m_devices, &QTreeWidget::currentItemChanged, this, [this] { m_remove->setEnabled(m_devices->currentItem() != nullptr); });
    connect(&c, &LiveSyncControl::enabledChanged, this, &LiveSyncPage::refresh);
    connect(c.model(), &collab::StatusModel::devicesChanged, this, &LiveSyncPage::refreshDevices);
    refresh();
}

QWidget *LiveSyncPage::folded(const QString &title, const char *name, QWidget *body)
{
    auto *w = new QWidget(this);
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    auto *b = new QToolButton(w);
    b->setObjectName(QLatin1String(name));
    b->setText(title);
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    b->setArrowType(Qt::RightArrow);
    b->setCheckable(true);
    b->setAutoRaise(true);
    l->addWidget(b);
    l->addWidget(body);
    body->setVisible(false);
    connect(b, &QToolButton::toggled, this, [b, body](bool open) {
        b->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
        body->setVisible(open);
    });
    return w;
}

void LiveSyncPage::refresh()
{
    LiveSyncControl &c = liveSyncControl();
    m_loading = true;
    const QString why = c.unavailableReason();
    m_on->setChecked(c.enabled());
    m_on->setEnabled(c.enabled() || why.isEmpty());
    m_reason->setText(why);
    m_reason->setVisible(!why.isEmpty());
    if (!m_name->hasFocus())
        m_name->setText(c.deviceName());
    m_keep->setChecked(c.keepHosting());
    m_keys->setText(keyPlaceText());
    const bool on = c.enabled();
    m_name->setEnabled(on);
    m_keep->setEnabled(on);
    m_pair->setEnabled(on);
    m_byAddress->setEnabled(on);
    const QStringList addr = c.addresses();
    m_addresses->setText(on ? (addr.isEmpty() ? tr("No address yet.") : tr("Addresses of this device: %1. Port: %2.").arg(addr.join(QStringLiteral(", ")), QString::number(c.port())))
                            : tr("Turn on Live Sync to see them."));
    // only what is no content: a short fingerprint, the protocol and round-trip times
    QStringList d;
    if (!c.fingerprint().isEmpty())
        d << tr("Fingerprint: %1").arg(c.fingerprint());
    if (!c.protocolText().isEmpty())
        d << tr("Protocol: %1").arg(c.protocolText());
    if (!c.roundTripText().isEmpty())
        d << tr("Round trips: %1").arg(c.roundTripText());
    m_details->setText(on && !d.isEmpty() ? d.join(QLatin1Char('\n')) : tr("Nothing to show."));
    m_loading = false;
    refreshDevices();
}

void LiveSyncPage::refreshDevices()
{
    const QString keep = m_devices->currentItem() ? m_devices->currentItem()->data(0, Qt::UserRole).toString() : QString();
    m_devices->clear();
    for (const collab::DeviceStatus &d : liveSyncControl().model()->devices()) {
        const QString seen = d.online ? tr("Online now") : (d.lastSeen.isValid() ? QLocale().toString(d.lastSeen, QLocale::ShortFormat) : tr("Never"));
        auto *it = new QTreeWidgetItem(m_devices, {d.name, seen});
        it->setData(0, Qt::UserRole, d.deviceId);
        if (d.deviceId == keep)
            m_devices->setCurrentItem(it);
    }
    m_remove->setEnabled(m_devices->currentItem() != nullptr);
}

} // namespace livesync
} // namespace qb
