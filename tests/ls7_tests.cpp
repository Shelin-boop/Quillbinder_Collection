// Live Sync, work package 7 (0.99.0): the interface. Checks, all against a FakeLiveSyncControl (no network, no service):
// the options page, the toolbar button in its five states and its menu, the inspector tab, the binder's marks, the bars
// and their buttons, the pairing, join, hosted projects and recovery drafts windows (non-modal), the wallet's passphrase
// dialogs, Live Sync off (nothing visible), and the two files routed through the write gate (the inspector's picture
// removal, the project look). Screenshots of what was built are saved into the folder given as first argument.
//
//   xvfb-run -a ./qb_ls7_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/collab/Gate.h"
#include "core/crypto/Wallet.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/EncryptionPage.h"
#include "ui/Inspector.h"
#include "ui/LiveSyncControl.h"
#include "ui/LiveSyncDialogs.h"
#include "ui/LiveSyncPage.h"
#include "ui/LiveSyncTab.h"
#include "ui/LiveSyncUi.h"
#include "ui/LiveSyncWallet.h"
#include "ui/OptionsDialog.h"
#include "ui/ProjectLook.h"
#include "ui/Theme.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <cstdio>

using namespace qb;
using namespace qb::livesync;
namespace col = qb::collab;

static int failures = 0, passes = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (ok)
        ++passes;
    else {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(1500)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 100)
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
    settle(200);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

template <class T> static T *child(QWidget *w, const char *name) { return w->findChild<T *>(QLatin1String(name)); }

static QString textOf(QAbstractButton *b) { return b ? b->text() : QString(); }

// a shared project as the model shows it
static col::ProjectStatus makeStatus(const QString &uuid, const QString &folder, col::ShareRole role, col::ShareState state)
{
    col::ProjectStatus p;
    p.valid = true;
    p.uuid = uuid;
    p.title = QStringLiteral("The Lighthouse");
    p.folder = folder;
    p.role = role;
    p.hostDeviceId = role == col::ShareRole::Host ? QStringLiteral("me") : QStringLiteral("hostdev");
    p.hostName = role == col::ShareRole::Host ? QStringLiteral("This Computer") : QStringLiteral("Studio PC");
    p.state = state;
    col::ParticipantStatus me, other;
    me.deviceId = QStringLiteral("me");
    me.name = QStringLiteral("This Computer");
    me.online = true;
    me.thisDevice = true;
    me.isHost = role == col::ShareRole::Host;
    other.deviceId = QStringLiteral("other");
    other.name = QStringLiteral("Tablet");
    other.online = true;
    other.isHost = role != col::ShareRole::Host;
    p.participants << me << other;
    return p;
}

static QStringList entryTexts(QMenu *m)
{
    QStringList out;
    for (QAction *a : m->actions())
        if (!a->isSeparator())
            out << a->text();
    return out;
}

static QAction *entry(QMenu *m, const QString &object)
{
    for (QAction *a : m->actions())
        if (a->objectName() == object)
            return a;
    return nullptr;
}

// ----------------------------------------------------------------------------------------------- off

static void testOff(const QString &tmp, FakeLiveSyncControl &fake)
{
    fake.on = false;
    QMainWindow w;
    auto *bar = w.addToolBar(QStringLiteral("Main"));
    Inspector insp;
    BinderPanel panel;
    LiveSyncUi ui(&w);
    QAction *a = ui.installToolbarButton(bar);
    ui.installInspectorTab(&insp);
    ui.attachBinder(&panel);
    w.setCentralWidget(&insp);
    w.show();
    settle();
    check(a && !a->isVisible(), QStringLiteral("off: the toolbar button is not visible"));
    check(!insp.extraTabVisible(QStringLiteral("livesync")), QStringLiteral("off: the inspector's Live Sync tab is not visible"));
    check(!ui.bars()->isVisible() && ui.barsWidget()->barCount() == 0, QStringLiteral("off: no bar"));
    check(!ui.active(), QStringLiteral("off: the interface says it is not active"));
    // a shared project in the model does not change that while the switch is off
    auto p = Project::create(tmp, QStringLiteral("Off"), nullptr);
    ui.setProject(p.get());
    fake.status.setProject(makeStatus(QStringLiteral("u1"), p->folderPath(), col::ShareRole::Host, col::ShareState::UpToDate));
    settle();
    check(!a->isVisible() && !insp.extraTabVisible(QStringLiteral("livesync")) && ui.barsWidget()->barCount() == 0,
          QStringLiteral("off: even with a shared project in the model nothing shows"));
    check(panel.tree()->lockTip(QStringLiteral("x")).isEmpty(), QStringLiteral("off: no binder mark"));
    fake.status.removeProject(QStringLiteral("u1"));
    ui.setProject(nullptr);
    // the program's stand-in (no service): off by default
    setLiveSyncControl(nullptr);
    check(!liveSyncControl().enabled() && !prefEnabled(), QStringLiteral("Live Sync is off by default"));
    liveSyncControl().setEnabled(true);
    check(liveSyncControl().enabled() && prefEnabled(), QStringLiteral("the stand-in keeps the switch"));
    liveSyncControl().setEnabled(false);
    check(!prefEnabled(), QStringLiteral("the stand-in switches off again"));
    setLiveSyncControl(&fake);
}

// ----------------------------------------------------------------------------------------------- options page

static void testOptionsPage(FakeLiveSyncControl &fake)
{
    fake.on = false;
    fake.name = QStringLiteral("Desk");
    fake.calls.clear();
    LiveSyncPage page;
    page.resize(640, 700);
    page.show();
    settle();
    auto *on = child<QCheckBox>(&page, "liveSyncOn");
    auto *name = child<QLineEdit>(&page, "liveSyncName");
    auto *keep = child<QCheckBox>(&page, "liveSyncKeep");
    auto *pair = child<QPushButton>(&page, "liveSyncPair");
    auto *list = child<QTreeWidget>(&page, "liveSyncDevices");
    auto *remove = child<QPushButton>(&page, "liveSyncRemove");
    check(on && name && keep && pair && list && remove, QStringLiteral("page: the switch, the name, the hosting option and the device list exist"));
    check(on && !on->isChecked() && textOf(on) == QLatin1String("Turn on Live Sync"), QStringLiteral("page: Live Sync is off at first"));
    check(!name->isEnabled() && !keep->isEnabled() && !pair->isEnabled(), QStringLiteral("page: off, the other controls are disabled"));
    check(child<QLabel>(&page, "liveSyncKeyPlace") && !child<QLabel>(&page, "liveSyncKeyPlace")->text().isEmpty(),
          QStringLiteral("page: a line says where the keys are kept"));
    // the encryption warning is on the page
    bool warns = false;
    for (QLabel *l : page.findChildren<QLabel *>())
        if (l->text().contains(QLatin1String("not encrypted")) || (l->text().contains(QLatin1String("device's own encryption")) && l->text().contains(QLatin1String("ZIP backups"))))
            warns = true;
    check(warns, QStringLiteral("page: it says what stays readable (plain projects, ZIP backups) and recommends device encryption"));
    shot(&page, QStringLiteral("options-off"));

    on->setChecked(true);
    settle();
    check(fake.calls.contains(QStringLiteral("enabled:1")) && fake.on, QStringLiteral("page: the switch turns Live Sync on at once"));
    check(name->isEnabled() && keep->isEnabled() && pair->isEnabled(), QStringLiteral("page: on, the other controls work"));
    check(name->text() == QLatin1String("Desk"), QStringLiteral("page: the device's name is shown"));
    name->setText(QStringLiteral("Study"));
    emit name->editingFinished();
    check(fake.calls.contains(QStringLiteral("name:Study")), QStringLiteral("page: the name is set"));
    keep->setChecked(true);
    check(fake.calls.contains(QStringLiteral("keep:1")) && fake.keep, QStringLiteral("page: keep hosting is set"));
    // devices
    col::DeviceStatus d1, d2;
    d1.deviceId = QStringLiteral("d1");
    d1.name = QStringLiteral("Tablet");
    d1.online = true;
    d2.deviceId = QStringLiteral("d2");
    d2.name = QStringLiteral("Laptop");
    d2.lastSeen = QDateTime(QDate(2026, 11, 20), QTime(9, 30));
    fake.status.setDevices({d1, d2});
    settle();
    check(list->topLevelItemCount() == 2 && list->topLevelItem(0)->text(0) == QLatin1String("Tablet")
              && list->topLevelItem(0)->text(1) == QLatin1String("Online now") && list->topLevelItem(1)->text(1).contains(QLatin1String("26")),
          QStringLiteral("page: the paired devices with their last seen times"));
    check(!remove->isEnabled(), QStringLiteral("page: Remove waits for a selected device"));
    // Remove asks first
    bool asked = false;
    bool answer = false;
    dialogHooks().confirm = [&](QWidget *, const QString &, const QString &text, const QString &) {
        asked = text.contains(QLatin1String("Laptop"));
        return answer;
    };
    list->setCurrentItem(list->topLevelItem(1));
    remove->click();
    check(asked && !fake.calls.contains(QStringLiteral("remove:d2")), QStringLiteral("page: Remove asks and does nothing when the answer is no"));
    answer = true;
    remove->click();
    check(fake.calls.contains(QStringLiteral("remove:d2")), QStringLiteral("page: Remove removes after a yes"));
    dialogHooks().confirm = nullptr;
    // folded parts
    auto *more = child<QToolButton>(&page, "liveSyncMore");
    auto *addr = child<QLabel>(&page, "liveSyncAddresses");
    check(more && addr && !addr->isVisible(), QStringLiteral("page: More Ways to Connect is folded"));
    fake.addr = {QStringLiteral("192.168.1.20")};
    fake.portNumber = 50573;
    page.refresh();
    more->setChecked(true);
    settle();
    check(addr->isVisible() && addr->text().contains(QLatin1String("192.168.1.20")) && addr->text().contains(QLatin1String("50573")),
          QStringLiteral("page: the addresses and the port show when unfolded"));
    check(child<QPushButton>(&page, "liveSyncByAddress") && child<QPushButton>(&page, "liveSyncByAddress")->isVisible(),
          QStringLiteral("page: Connect by Address... is there"));
    auto *details = child<QToolButton>(&page, "liveSyncDetailsFold");
    details->setChecked(true);
    settle();
    const QString dt = child<QLabel>(&page, "liveSyncDetails")->text();
    check(dt.contains(QLatin1String("AB12")) && dt.contains(QLatin1String("TLS")) && dt.contains(QLatin1String("3 ms")),
          QStringLiteral("page: the details show the fingerprint, the protocol and the round trips"), dt);
    shot(&page, QStringLiteral("options-on"));
    // Pair a Device opens the non-modal pairing window
    pair->click();
    settle();
    auto *pd = qobject_cast<PairingDialog *>(QApplication::activeWindow());
    PairingDialog *found = nullptr;
    for (QWidget *w : QApplication::topLevelWidgets())
        if (auto *x = qobject_cast<PairingDialog *>(w))
            found = x;
    check(found && found->isVisible() && !found->isModal(), QStringLiteral("page: Pair a Device... opens the pairing window, which does not block"));
    Q_UNUSED(pd)
    if (found)
        found->close();
    settle();

    // unavailable
    fake.on = false;
    fake.unavailable = QStringLiteral("There is no place for the keys.");
    page.refresh();
    check(!on->isEnabled() && child<QLabel>(&page, "liveSyncReason")->text().contains(QLatin1String("no place")),
          QStringLiteral("page: where Live Sync cannot be switched on, the page says why"));
    fake.unavailable.clear();
    page.refresh();

    // the Options window has the page between Encryption and Extras
    {
        OptionsDialog od(nullptr, 0);
        od.resize(900, 640);
        od.show();
        settle(200);
        auto *icons = od.findChild<QListWidget *>(QStringLiteral("optionPages"));
        QStringList names;
        for (int i = 0; icons && i < icons->count(); ++i)
            names << icons->item(i)->text();
        const int i = int(names.indexOf(QStringLiteral("Live Sync")));
        check(i == int(names.indexOf(QStringLiteral("Encryption"))) + 1, QStringLiteral("options: the Live Sync section is beside Encryption"), names.join(QLatin1Char(',')));
        od.showSection(OptionsDialog::SLiveSync);
        settle(200);
        check(od.findChild<LiveSyncPage *>() && od.findChild<LiveSyncPage *>()->isVisible(), QStringLiteral("options: the section shows the Live Sync page"));
        shot(&od, QStringLiteral("options-window"));
        od.showSection(OptionsDialog::SEncryption);
        settle(200);
        auto *place = od.findChild<QLabel *>(QStringLiteral("encKeyPlace"));
        check(place && place->isVisible() && place->text().contains(QLatin1String("Keys are kept")), QStringLiteral("options: the Encryption page says where the keys are kept"));
        shot(&od, QStringLiteral("options-encryption"));
    }
    fake.on = false;
}

// ----------------------------------------------------------------------------------------------- toolbar, menu, tab, marks, bars

static void testWindowParts(const QString &tmp, FakeLiveSyncControl &fake)
{
    fake.on = true;
    fake.status.setRunning(true);
    QMainWindow w;
    auto *bar = w.addToolBar(QStringLiteral("Main"));
    auto *central = new QWidget;
    auto *lay = new QVBoxLayout(central);
    Inspector insp;
    BinderPanel panel;
    LiveSyncUi ui(&w);
    QAction *act = ui.installToolbarButton(bar);
    ui.installInspectorTab(&insp);
    ui.attachBinder(&panel);
    lay->addWidget(ui.bars());
    lay->addWidget(&panel, 1);
    lay->addWidget(&insp, 1);
    w.setCentralWidget(central);
    w.resize(900, 640);
    auto p = Project::create(tmp, QStringLiteral("Parts"), nullptr);
    BinderItem *a = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter One"));
    BinderItem *b = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter Two"));
    panel.setProject(p.get());
    insp.setProject(p.get());
    ui.setProject(p.get());
    w.show();
    settle(200);
    QToolButton *btn = ui.button();
    check(btn && act && act->isVisible(), QStringLiteral("on: the toolbar button shows"));
    check(insp.extraTabVisible(QStringLiteral("livesync")), QStringLiteral("on: the inspector tab shows"));

    // ---- the five states
    const QString uuid = QStringLiteral("u1");
    QHash<col::ShareState, QImage> icons;
    const QList<std::pair<col::ShareState, QString>> states = {{col::ShareState::NotShared, QStringLiteral("Not shared")},
                                                               {col::ShareState::UpToDate, QStringLiteral("Up to date")},
                                                               {col::ShareState::Sending, QStringLiteral("Sending changes...")},
                                                               {col::ShareState::NotConnected, QStringLiteral("Not connected (draft kept)")},
                                                               {col::ShareState::Problem, QStringLiteral("Problem")}};
    // not shared: the button says so
    check(btn->text() == QLatin1String("Not shared") && btn->accessibleName().contains(QLatin1String("Not shared")), QStringLiteral("button: a project that is not shared"));
    icons.insert(col::ShareState::NotShared, btn->icon().pixmap(24, 24).toImage());
    col::ProjectStatus st = makeStatus(uuid, p->folderPath(), col::ShareRole::Host, col::ShareState::UpToDate);
    fake.status.setProject(st);
    for (const auto &s : states) {
        if (s.first == col::ShareState::NotShared)
            continue;
        fake.status.setState(uuid, s.first, s.first == col::ShareState::Problem ? QStringLiteral("The disk is full.") : QString());
        settle(30);
        check(btn->text() == s.second && btn->accessibleName() == QStringLiteral("Live Sync, ") + s.second && btn->toolTip().contains(s.second),
              QStringLiteral("button: state \"%1\" (text, accessible name, tool tip)").arg(s.second), btn->text());
        icons.insert(s.first, btn->icon().pixmap(24, 24).toImage());
        shot(btn->parentWidget(), QStringLiteral("button-state-%1").arg(int(s.first)));
    }
    bool distinct = true;
    for (auto i = icons.begin(); i != icons.end(); ++i)
        for (auto j = icons.begin(); j != icons.end(); ++j)
            if (i.key() != j.key() && i.value() == j.value())
                distinct = false;
    check(distinct && icons.size() == 5, QStringLiteral("button: each of the five states has its own icon (a shape and a color)"));
    check(btn->toolTip().contains(QLatin1String("The disk is full.")), QStringLiteral("button: a problem's words are in the tool tip"));
    check(btn->minimumHeight() >= 0 && btn->popupMode() == QToolButton::InstantPopup, QStringLiteral("button: opens its menu"));
    // a window without a shared project shows the worst state of all
    {
        QMainWindow w2;
        auto *bar2 = w2.addToolBar(QStringLiteral("x"));
        LiveSyncUi ui2(&w2);
        ui2.installToolbarButton(bar2);
        fake.status.setState(uuid, col::ShareState::NotConnected);
        settle(30);
        check(ui2.button()->text() == QLatin1String("Not connected (draft kept)"), QStringLiteral("button: with no shared project open it shows the worst state of all"));
    }
    fake.status.setState(uuid, col::ShareState::UpToDate);

    // ---- the menu, host
    btn->menu()->aboutToShow();
    check(entryTexts(btn->menu())
              == QStringList({QStringLiteral("Share This Project..."), QStringLiteral("Join a Project..."), QStringLiteral("Request Backup"),
                              QStringLiteral("Transfer Hosting..."), QStringLiteral("Hosted Projects..."), QStringLiteral("Recovery Drafts..."),
                              QStringLiteral("Stop Sharing")}),
          QStringLiteral("menu: the seven commands"), entryTexts(btn->menu()).join(QLatin1Char('|')));
    auto en = [&](const char *n) { return entry(btn->menu(), QLatin1String(n)) && entry(btn->menu(), QLatin1String(n))->isEnabled(); };
    check(!en("lsShareProject") && en("lsJoinProject") && en("lsRequestBackup") && en("lsHostedProjects") && !en("lsRecoveryDrafts") && en("lsStopSharing"),
          QStringLiteral("menu: host of a shared project: Share is off; Join, Backup, Hosted, Stop are on; no drafts: off"));
    check(!en("lsTransferHosting"), QStringLiteral("menu: Transfer Hosting waits for an online editor device that may host"));
    AccessEntry ae;
    ae.deviceId = QStringLiteral("other");
    ae.name = QStringLiteral("Tablet");
    ae.role = AccessEntry::Editor;
    ae.online = true;
    fake.accessList.insert(uuid, {ae});
    btn->menu()->aboutToShow();
    check(en("lsTransferHosting"), QStringLiteral("menu: Transfer Hosting is on with such a device"));
    // a client: Leave
    fake.status.setProject(makeStatus(uuid, p->folderPath(), col::ShareRole::Editor, col::ShareState::UpToDate));
    btn->menu()->aboutToShow();
    check(entry(btn->menu(), QStringLiteral("lsStopSharing"))->text() == QLatin1String("Leave") && !en("lsTransferHosting") && !en("lsHostedProjects"),
          QStringLiteral("menu: a client sees Leave; Transfer Hosting and Hosted Projects are off"));
    // not shared: Share
    fake.status.removeProject(uuid);
    btn->menu()->aboutToShow();
    check(en("lsShareProject") && !en("lsRequestBackup") && !en("lsStopSharing") && en("lsJoinProject"), QStringLiteral("menu: a project that is not shared can be shared; backup and stop are off"));
    entry(btn->menu(), QStringLiteral("lsShareProject"))->trigger();
    check(fake.calls.contains(QStringLiteral("share:") + p->folderPath()), QStringLiteral("menu: Share This Project... asks the service to share the open project"));
    // a refused share is told
    fake.shareOk = false;
    entry(btn->menu(), QStringLiteral("lsShareProject"))->trigger();
    settle();
    bool told = false;
    for (QWidget *tl : QApplication::topLevelWidgets())
        if (auto *mb = qobject_cast<QMessageBox *>(tl))
            if (mb->isVisible() && mb->text().contains(QLatin1String("refused"))) {
                told = true;
                mb->close();
            }
    check(told, QStringLiteral("menu: a refused share is told, in a window that does not block"));
    fake.shareOk = true;
    // a closed project: no Share
    ui.setProject(nullptr);
    btn->menu()->aboutToShow();
    check(!en("lsShareProject"), QStringLiteral("menu: without an open project Share is off"));
    ui.setProject(p.get());

    // ---- host project with locks: the tab, the marks, the bars
    st = makeStatus(uuid, p->folderPath(), col::ShareRole::Host, col::ShareState::UpToDate);
    st.participants[1].editing = {QStringLiteral("doc:") + a->id};
    st.participants[0].editing = {QStringLiteral("doc:") + b->id, QStringLiteral("structure")};
    st.backup.state = col::BackupStatus::Done;
    st.backup.lastSuccess = QDateTime(QDate(2026, 11, 27), QTime(20, 15));
    st.draftCount = 2;
    fake.status.setProject(st);
    ui.setSelectedDocument(a->id);
    insp.showExtraTab(QStringLiteral("livesync"));
    settle(200);
    auto *tab = ui.tab();
    auto *people = child<QTreeWidget>(tab, "lsPeople");
    check(tab->isVisible() && people && people->topLevelItemCount() == 2, QStringLiteral("tab: both devices are listed"));
    check(people->topLevelItem(1)->text(2) == QLatin1String("Chapter One") && people->topLevelItem(0)->text(2).contains(QLatin1String("Chapter Two")),
          QStringLiteral("tab: what each edits, with titles"), people->topLevelItem(1)->text(2));
    check(child<QLabel>(tab, "lsSummary")->text().contains(QLatin1String("Host: this device")) && child<QLabel>(tab, "lsSummary")->text().contains(QLatin1String("Up to date")),
          QStringLiteral("tab: host, role and state"));
    check(child<QLabel>(tab, "lsBackup")->text().contains(QLatin1String("Backup done")) && child<QLabel>(tab, "lsBackup")->text().contains(QLatin1String("Last successful")),
          QStringLiteral("tab: the last backup"));
    check(child<QLabel>(tab, "lsDrafts")->text().contains(QLatin1String("2")), QStringLiteral("tab: the number of drafts"));
    // the document held by the other device
    col::LockInfo li;
    li.resource = QStringLiteral("doc:") + a->id;
    li.held = true;
    li.holderDeviceId = QStringLiteral("other");
    li.holderName = QStringLiteral("Tablet");
    fake.status.setLock(uuid, li);
    settle(30);
    auto *ask = child<QPushButton>(tab, "lsAsk");
    auto *take = child<QPushButton>(tab, "lsTake");
    auto *stop = child<QPushButton>(tab, "lsStop");
    check(child<QLabel>(tab, "lsDoc")->text().contains(QLatin1String("Tablet")) && ask->isEnabled() && !take->isEnabled() && !stop->isEnabled(),
          QStringLiteral("tab: a document held elsewhere: Ask to Edit is on, Take Over and Stop Editing are off"));
    ask->click();
    check(fake.status.calls().contains(QStringLiteral("ask:%1:doc:%2").arg(uuid, a->id)), QStringLiteral("tab: Ask to Edit asks"));
    li.askOpen = true;
    li.askWaitMs = 12000;
    fake.status.setLock(uuid, li);
    check(!ask->isEnabled() && !take->isEnabled(), QStringLiteral("tab: while the request waits, neither Ask nor Take Over is on"));
    li.askWaitMs = 0;
    fake.status.setLock(uuid, li);
    check(take->isEnabled(), QStringLiteral("tab: after the waiting time Take Over is on"));
    take->click();
    check(fake.status.calls().contains(QStringLiteral("take:%1:doc:%2").arg(uuid, a->id)), QStringLiteral("tab: Take Over takes over"));
    // my own lock
    ui.setSelectedDocument(b->id);
    li = col::LockInfo();
    li.resource = QStringLiteral("doc:") + b->id;
    li.held = true;
    li.mine = true;
    li.holderName = QStringLiteral("This Computer");
    fake.status.setLock(uuid, li);
    check(stop->isEnabled() && !ask->isEnabled(), QStringLiteral("tab: my own lock: Stop Editing is on"));
    stop->click();
    check(fake.status.calls().contains(QStringLiteral("stop:%1:doc:%2").arg(uuid, b->id)), QStringLiteral("tab: Stop Editing gives the lock up"));
    child<QPushButton>(tab, "lsRequestBackup")->click();
    check(fake.status.calls().contains(QStringLiteral("backup:") + uuid), QStringLiteral("tab: Request Backup"));
    shot(&w, QStringLiteral("window-host"));

    // ---- binder marks
    BinderTree *tree = panel.tree();
    check(tree->lockTip(a->id) == QLatin1String("Being edited on Tablet") && tree->lockKind(a->id) == 0,
          QStringLiteral("marks: a document another device edits has the padlock and its tip"), tree->lockTip(a->id));
    check(tree->lockTip(b->id) == QLatin1String("You are editing this document") && tree->lockKind(b->id) == 1,
          QStringLiteral("marks: your own lock is the pen"), tree->lockTip(b->id));
    st.participants[1].online = false;
    st.participants[1].lastSeen = QDateTime(QDate(2026, 11, 28), QTime(8, 5));
    fake.status.setProject(st);
    check(tree->lockTip(a->id).startsWith(QLatin1String("Not reachable since")) && tree->lockKind(a->id) == 2,
          QStringLiteral("marks: a holder that is not reachable: the dimmed padlock \"Not reachable since <time>\""), tree->lockTip(a->id));
    // the marks are really painted: the row's mark rectangle differs from an unmarked row
    {
        panel.show();
        settle(100);
        const QModelIndex ia = tree->model()->index(0, 0, tree->model()->index(0, 0));
        Q_UNUSED(ia)
        QImage img(tree->viewport()->size(), QImage::Format_ARGB32);
        tree->viewport()->render(&img);
        check(!img.isNull(), QStringLiteral("marks: the binder paints"));
    }
    shot(&panel, QStringLiteral("binder-marks"));
    st.participants[1].online = true;
    fake.status.setProject(st);

    // ---- bars
    auto *bars = ui.barsWidget();
    ui.setSelectedDocument(a->id);
    li = col::LockInfo();
    li.resource = QStringLiteral("doc:") + a->id;
    li.held = true;
    li.holderName = QStringLiteral("Tablet");
    fake.status.setLock(uuid, li);
    settle(30);
    check(bars->isVisible() && bars->barText(QStringLiteral("barReadOnly")).contains(QLatin1String("Read-only: being edited on Tablet")),
          QStringLiteral("bars: a document held elsewhere says so"), bars->barText(QStringLiteral("barReadOnly")));
    auto *askBtn = child<QPushButton>(bars, "barReadOnly:Ask to Edit");
    check(askBtn != nullptr, QStringLiteral("bars: the read-only bar has Ask to Edit"));
    if (askBtn)
        askBtn->click();
    check(fake.status.calls().filter(QStringLiteral("ask:%1:doc:%2").arg(uuid, a->id)).size() >= 2, QStringLiteral("bars: Ask to Edit asks"));
    shot(&w, QStringLiteral("bar-read-only"));
    li.askOpen = true;
    li.askWaitMs = 15000;
    fake.status.setLock(uuid, li);
    settle(30);
    check(bars->barText(QStringLiteral("barWaiting")).contains(QLatin1String("Asked Tablet")) && child<QPushButton>(bars, "barWaiting:Cancel"), QStringLiteral("bars: a waiting request, with Cancel"));
    li.declined = true;
    fake.status.setLock(uuid, li);
    settle(30);
    check(bars->barText(QStringLiteral("barWaiting")).contains(QLatin1String("is being worked on on Tablet")), QStringLiteral("bars: Decline is told to the asker"), bars->barText(QStringLiteral("barWaiting")));
    child<QPushButton>(bars, "barWaiting:Cancel")->click();
    check(fake.status.calls().contains(QStringLiteral("cancel:%1:doc:%2").arg(uuid, a->id)), QStringLiteral("bars: Cancel cancels the request"));
    li.askWaitMs = 0;
    fake.status.setLock(uuid, li);
    settle(30);
    check(bars->barText(QStringLiteral("barWaiting")).contains(QLatin1String("recovery draft")) && child<QPushButton>(bars, "barWaiting:Take Over"),
          QStringLiteral("bars: after the time, Take Over, saying the other's unsaved changes stay as a draft"));
    shot(&w, QStringLiteral("bar-take-over"));
    child<QPushButton>(bars, "barWaiting:Take Over")->click();
    check(fake.status.calls().contains(QStringLiteral("take:%1:doc:%2").arg(uuid, a->id)), QStringLiteral("bars: Take Over takes over"));
    fake.status.clearLocks(uuid);
    // another device asks for a document this one holds
    emit fake.editAsked(uuid, QStringLiteral("doc:") + b->id, QStringLiteral("Tablet"));
    settle(30);
    check(bars->barText(QStringLiteral("barAsked")).contains(QLatin1String("Tablet asks to edit \"Chapter Two\"")) && child<QPushButton>(bars, "barAsked:Save and Hand Over")
              && child<QPushButton>(bars, "barAsked:Decline"),
          QStringLiteral("bars: a request for a document this device holds: Save and Hand Over, Decline"), bars->barText(QStringLiteral("barAsked")));
    shot(&w, QStringLiteral("bar-asked"));
    child<QPushButton>(bars, "barAsked:Decline")->click();
    check(fake.calls.contains(QStringLiteral("decline:%1:doc:%2").arg(uuid, b->id)), QStringLiteral("bars: Decline declines (the bar stays until the request ends)"));
    check(bars->barText(QStringLiteral("barAsked")).contains(QLatin1String("Tablet")), QStringLiteral("bars: after Decline the request is still open"));
    child<QPushButton>(bars, "barAsked:Save and Hand Over")->click();
    check(fake.calls.contains(QStringLiteral("handover:%1:doc:%2").arg(uuid, b->id)), QStringLiteral("bars: Save and Hand Over hands over"));
    emit fake.editAskEnded(uuid, QStringLiteral("doc:") + b->id);
    settle(30);
    check(bars->barText(QStringLiteral("barAsked")).isEmpty(), QStringLiteral("bars: the bar goes when the request ends"));
    // not connected, problem, read-only access
    fake.status.setState(uuid, col::ShareState::NotConnected);
    settle(30);
    check(bars->barText(QStringLiteral("barOffline")) == QLatin1String("Not connected. Your changes are kept on this device."), QStringLiteral("bars: not connected"));
    shot(&w, QStringLiteral("bar-offline"));
    fake.status.setState(uuid, col::ShareState::Problem, QStringLiteral("The host's folder is full."));
    settle(30);
    check(bars->barText(QStringLiteral("barProblem")).contains(QLatin1String("folder is full")), QStringLiteral("bars: a problem"));
    st = makeStatus(uuid, p->folderPath(), col::ShareRole::Reader, col::ShareState::UpToDate);
    st.readOnly = true;
    st.readOnlyReason = QStringLiteral("you have read access only");
    fake.status.setProject(st);
    settle(30);
    check(bars->barText(QStringLiteral("barAccess")).contains(QLatin1String("you have read access only")), QStringLiteral("bars: a reader is told"));
    fake.status.removeProject(uuid);
    settle(30);
    check(!bars->isVisible() && bars->barCount() == 0, QStringLiteral("bars: nothing shared, no bar"));

    // ---- switching off hides everything again
    fake.on = false;
    emit fake.enabledChanged();
    settle(50);
    check(!act->isVisible() && !insp.extraTabVisible(QStringLiteral("livesync")), QStringLiteral("off again: button and tab go"));
    fake.on = true;
}

// ----------------------------------------------------------------------------------------------- dialogs

static void testDialogs(FakeLiveSyncControl &fake)
{
    fake.on = true;
    fake.calls.clear();
    fake.status.setRunning(true);
    // ---- pairing
    {
        DiscoveredDevice d;
        d.key = QStringLiteral("inst1");
        d.name = QStringLiteral("Studio PC");
        d.pairingOpen = true;
        DiscoveredDevice closed;
        closed.key = QStringLiteral("inst2");
        closed.pairingOpen = false;
        fake.found = {d, closed};
        PairingDialog *pd = showPairingDialog(nullptr);
        settle(100);
        check(pd->isVisible() && !pd->isModal(), QStringLiteral("pairing: the window does not block"));
        auto *list = child<QListWidget>(pd, "pairList");
        check(list && list->count() == 1 && list->item(0)->text() == QLatin1String("Studio PC"), QStringLiteral("pairing: only devices with an open pairing window are listed"));
        check(showPairingDialog(nullptr) == pd, QStringLiteral("pairing: a second call raises the same window"));
        auto *open = child<QPushButton>(pd, "pairOpenWindow");
        open->click();
        check(fake.calls.contains(QStringLiteral("openwindow")) && open->text() == QLatin1String("Close the Pairing Window"), QStringLiteral("pairing: the host side opens its window"));
        open->click();
        check(fake.calls.contains(QStringLiteral("closewindow")), QStringLiteral("pairing: and closes it"));
        list->setCurrentRow(0);
        child<QPushButton>(pd, "pairSelected")->click();
        check(fake.calls.contains(QStringLiteral("pair:inst1")), QStringLiteral("pairing: the other side pairs with the picked device"));
        check(!child<QWidget>(pd, "pairCodeBox")->isVisible(), QStringLiteral("pairing: no code before the exchange"));
        emit fake.pairingCode(QStringLiteral("Studio PC"), QStringLiteral("123456"));
        settle(50);
        check(child<QLabel>(pd, "pairCode")->text() == QLatin1String("123 456") && child<QLabel>(pd, "pairOther")->text().contains(QLatin1String("Studio PC"))
                  && child<QPushButton>(pd, "pairMatch")->isVisible() && child<QPushButton>(pd, "pairDiffer")->isVisible(),
              QStringLiteral("pairing: the other's name, the code in large digits, Codes Match and They Differ"));
        check(child<QLabel>(pd, "pairCode")->font().pointSizeF() > pd->font().pointSizeF() * 2, QStringLiteral("pairing: the digits are large"));
        shot(pd, QStringLiteral("pairing-code"));
        child<QPushButton>(pd, "pairMatch")->click();
        check(fake.calls.contains(QStringLiteral("answer:1")), QStringLiteral("pairing: Codes Match confirms"));
        emit fake.pairingFinished(true, QStringLiteral("Studio PC can now share projects with you."));
        settle(30);
        check(child<QLabel>(pd, "pairState")->text().startsWith(QLatin1String("Paired")), QStringLiteral("pairing: the result is told"));
        emit fake.pairingCode(QStringLiteral("Studio PC"), QStringLiteral("654321"));
        settle(30);
        child<QPushButton>(pd, "pairDiffer")->click();
        check(fake.calls.contains(QStringLiteral("answer:0")) && child<QLabel>(pd, "pairState")->text().contains(QLatin1String("differ")), QStringLiteral("pairing: They Differ stops it"));
        pd->close();
        settle(50);
        // a window opened by the other device's request (no dialog open): the interface shows it
        QMainWindow w;
        LiveSyncUi ui(&w);
        emit fake.pairingCode(QStringLiteral("Laptop"), QStringLiteral("000111"));
        settle(50);
        PairingDialog *again = nullptr;
        for (QWidget *tl : QApplication::topLevelWidgets())
            if (auto *x = qobject_cast<PairingDialog *>(tl))
                if (x->isVisible())
                    again = x;
        check(again && child<QLabel>(again, "pairCode")->text() == QLatin1String("000 111"), QStringLiteral("pairing: a pairing started by the other device opens the window with its code"));
        if (again) {
            fake.calls.clear();
            again->close(); // closing while a code waits is no confirmation
            settle(50);
            check(fake.calls.contains(QStringLiteral("cancelpair")) && !fake.calls.contains(QStringLiteral("answer:1")), QStringLiteral("pairing: closing the window pairs nothing"));
        }
    }
    // ---- join
    {
        RemoteProject r;
        r.uuid = QStringLiteral("pr1");
        r.title = QStringLiteral("The Lighthouse");
        r.hostDeviceId = QStringLiteral("h1");
        r.hostName = QStringLiteral("Studio PC");
        r.hostOnline = true;
        fake.remote = {r};
        DiscoveredDevice unpaired;
        unpaired.key = QStringLiteral("x");
        unpaired.name = QStringLiteral("Stranger");
        unpaired.paired = false;
        fake.found = {unpaired};
        JoinDialog *jd = showJoinDialog(nullptr);
        settle(100);
        auto *tree = child<QTreeWidget>(jd, "joinTree");
        check(!jd->isModal() && tree->topLevelItemCount() == 2, QStringLiteral("join: the paired host's project and the unpaired device are listed"));
        check(tree->topLevelItem(0)->text(0) == QLatin1String("The Lighthouse") && tree->topLevelItem(1)->text(0) == QLatin1String("Pair first"),
              QStringLiteral("join: an unpaired host shows only as \"Pair first\""));
        tree->setCurrentItem(nullptr);
        check(!child<QPushButton>(jd, "joinButton")->isEnabled(), QStringLiteral("join: Join waits for a project"));
        tree->setCurrentItem(tree->topLevelItem(1));
        check(!child<QPushButton>(jd, "joinButton")->isEnabled(), QStringLiteral("join: an unpaired entry cannot be joined"));
        tree->setCurrentItem(tree->topLevelItem(0));
        check(child<QPushButton>(jd, "joinButton")->isEnabled(), QStringLiteral("join: a paired host's project can"));
        shot(jd, QStringLiteral("join"));
        QString opened;
        QObject::connect(jd, &JoinDialog::openProject, [&](const QString &f) { opened = f; });
        child<QPushButton>(jd, "joinButton")->click();
        check(fake.calls.contains(QStringLiteral("join:h1:pr1")) && child<QWidget>(jd, "joinProgress")->isVisible(), QStringLiteral("join: Join starts and shows progress"));
        emit fake.joinProgress(QStringLiteral("pr1"), 40, QStringLiteral("Copying 40 of 100 files"));
        settle(30);
        check(child<QProgressBar>(jd, "joinProgress")->value() == 40 && child<QLabel>(jd, "joinNote")->text().contains(QLatin1String("40 of 100")), QStringLiteral("join: the progress of the full copy"));
        shot(jd, QStringLiteral("join-progress"));
        emit fake.joinFinished(QStringLiteral("pr1"), true, QStringLiteral("/tmp/replica/x.quillbinder"));
        settle(30);
        check(opened == QLatin1String("/tmp/replica/x.quillbinder") && !jd->isVisible(), QStringLiteral("join: when the copy is whole the project is opened and the window goes"));
    }
    // ---- hosted projects
    {
        col::ProjectStatus p = makeStatus(QStringLiteral("h1"), QStringLiteral("/tmp/h1"), col::ShareRole::Host, col::ShareState::UpToDate);
        p.pendingChanges = 3;
        p.backup.state = col::BackupStatus::Waiting;
        p.backup.detail = QStringLiteral("the backup folder is missing");
        fake.status.setProject(p);
        AccessEntry e1, e2;
        e1.deviceId = QStringLiteral("other");
        e1.name = QStringLiteral("Tablet");
        e1.role = AccessEntry::Editor;
        e1.online = true;
        e2.deviceId = QStringLiteral("d3");
        e2.name = QStringLiteral("Laptop");
        e2.role = AccessEntry::Reader;
        fake.accessList.insert(QStringLiteral("h1"), {e1, e2});
        fake.calls.clear();
        HostedProjectsDialog *hd = showHostedProjectsDialog(nullptr, QStringLiteral("h1"));
        settle(100);
        auto *devs = child<QTreeWidget>(hd, "hostedDevices");
        check(!hd->isModal() && devs->topLevelItemCount() == 2, QStringLiteral("hosted: the devices of the project"));
        check(child<QLabel>(hd, "hostedBackup")->text().contains(QLatin1String("Backup waiting")) && child<QLabel>(hd, "hostedBackup")->text().contains(QLatin1String("folder is missing")),
              QStringLiteral("hosted: the backup state"));
        auto *combo = child<QComboBox>(hd, "accessCombo:d3");
        check(combo && combo->currentText() == QLatin1String("Reader") && combo->count() == 3, QStringLiteral("hosted: access per device (Editor, Reader, None)"));
        combo->setCurrentIndex(combo->findText(QStringLiteral("None")));
        emit combo->activated(combo->currentIndex());
        check(fake.calls.contains(QStringLiteral("access:h1:d3:%1").arg(int(AccessEntry::None))), QStringLiteral("hosted: changing the access"));
        devs->setCurrentItem(devs->topLevelItem(0));
        child<QPushButton>(hd, "hostedDown")->click();
        check(fake.calls.contains(QStringLiteral("priority:h1:d3,other")), QStringLiteral("hosted: the priority moves"));
        shot(hd, QStringLiteral("hosted"));
        // Stop Sharing warns first
        QString warned;
        bool yes = false;
        dialogHooks().confirm = [&](QWidget *, const QString &, const QString &text, const QString &) {
            warned = text;
            return yes;
        };
        child<QPushButton>(hd, "hostedStop")->click();
        check(warned.contains(QLatin1String("1 other device")) && warned.contains(QLatin1String("3 change")) && !fake.status.calls().contains(QStringLiteral("stopsharing:h1")),
              QStringLiteral("hosted: Stop Sharing names the devices online and the unconfirmed changes, and waits for a yes"), warned);
        yes = true;
        child<QPushButton>(hd, "hostedStop")->click();
        check(fake.status.calls().contains(QStringLiteral("stopsharing:h1")), QStringLiteral("hosted: Stop Sharing after a yes"));
        dialogHooks().confirm = nullptr;
        dialogHooks().pickItem = [](QWidget *, const QString &, const QString &, const QStringList &items) { return items.value(0); };
        child<QPushButton>(hd, "hostedTransfer")->click();
        check(fake.calls.contains(QStringLiteral("transfer:h1:other")), QStringLiteral("hosted: Transfer Hosting hands it to the picked device"));
        dialogHooks().pickItem = nullptr;
        hd->close();
    }
    // ---- recovery drafts
    {
        DraftInfo d1, d2;
        d1.id = QStringLiteral("dr1");
        d1.uuid = QStringLiteral("h1");
        d1.docId = QStringLiteral("a");
        d1.docTitle = QStringLiteral("Chapter One");
        d1.when = QDateTime(QDate(2026, 11, 28), QTime(7, 40));
        d1.reason = QStringLiteral("not sent before the takeover");
        d2 = d1;
        d2.id = QStringLiteral("dr2");
        d2.docTitle = QStringLiteral("Chapter Two");
        d2.reason = QStringLiteral("made after the split");
        fake.draftList = {d1, d2};
        fake.calls.clear();
        DraftsDialog *dd = showDraftsDialog(nullptr);
        settle(100);
        auto *tree = child<QTreeWidget>(dd, "draftsTree");
        check(!dd->isModal() && tree->topLevelItemCount() == 2 && tree->topLevelItem(1)->text(2) == QLatin1String("made after the split"), QStringLiteral("drafts: time, document and reason"));
        tree->setCurrentItem(nullptr);
        check(!child<QPushButton>(dd, "draftDelete")->isEnabled(), QStringLiteral("drafts: the buttons wait for a draft"));
        tree->setCurrentItem(tree->topLevelItem(0));
        child<QPushButton>(dd, "draftCompare")->click();
        child<QPushButton>(dd, "draftKeep")->click();
        check(fake.calls.contains(QStringLiteral("compare:dr1")) && fake.calls.contains(QStringLiteral("keep:dr1")), QStringLiteral("drafts: Compare and Keep as Copy"));
        bool yes = false;
        QString asked;
        dialogHooks().confirm = [&](QWidget *, const QString &, const QString &text, const QString &) {
            asked = text;
            return yes;
        };
        child<QPushButton>(dd, "draftTake")->click();
        check(!fake.calls.contains(QStringLiteral("takedraft:dr1")) && asked.contains(QLatin1String("snapshot")), QStringLiteral("drafts: Take Over asks first and names the protective snapshot"));
        yes = true;
        child<QPushButton>(dd, "draftTake")->click();
        check(fake.calls.contains(QStringLiteral("takedraft:dr1")), QStringLiteral("drafts: Take Over after a yes"));
        yes = false;
        child<QPushButton>(dd, "draftDelete")->click();
        check(!fake.calls.contains(QStringLiteral("deletedraft:dr1")) && tree->topLevelItemCount() == 2, QStringLiteral("drafts: Delete asks and keeps the draft on no"));
        yes = true;
        child<QPushButton>(dd, "draftDelete")->click();
        settle(30);
        check(fake.calls.contains(QStringLiteral("deletedraft:dr1")) && tree->topLevelItemCount() == 1, QStringLiteral("drafts: Delete after a yes"));
        shot(dd, QStringLiteral("drafts"));
        dialogHooks().confirm = nullptr;
        dd->close();
        // closing and opening again at once gives a new window, not the one that is about to be deleted
        DraftsDialog *dd2 = showDraftsDialog(nullptr);
        check(dd2 != dd, QStringLiteral("drafts: a window that was just closed is not raised again"));
        dd2->close();
        settle(50);
        // a project filter
        DraftsDialog *d3 = showDraftsDialog(nullptr, QStringLiteral("other-project"));
        settle(50);
        check(child<QTreeWidget>(d3, "draftsTree")->topLevelItemCount() == 0 && child<QLabel>(d3, "draftsEmpty")->isVisible(), QStringLiteral("drafts: only the drafts of one project, and a line when there are none"));
        d3->close();
    }
    fake.status.removeProject(QStringLiteral("h1"));
    settle(50);
}

// ----------------------------------------------------------------------------------------------- wallet

static void testWallet()
{
    // the dialog
    {
        WalletPassphraseDialog c(true);
        c.show();
        auto *one = child<QLineEdit>(&c, "walletPass");
        auto *two = child<QLineEdit>(&c, "walletPass2");
        auto *ok = child<QPushButton>(&c, "walletOk");
        check(one && two && ok && !ok->isEnabled(), QStringLiteral("wallet: creating asks for the passphrase twice; Ok waits"));
        one->setText(QStringLiteral("short"));
        two->setText(QStringLiteral("short"));
        check(!ok->isEnabled() && !child<QLabel>(&c, "walletStrength")->text().isEmpty(), QStringLiteral("wallet: a weak passphrase is refused, with the reason"));
        one->setText(QStringLiteral("harbor lantern velvet orchard meadow quartz"));
        two->setText(QStringLiteral("harbor lantern velvet orchard meadow quarz"));
        check(!ok->isEnabled() && child<QLabel>(&c, "walletStrength")->text().contains(QLatin1String("not the same")), QStringLiteral("wallet: two different entries are refused"));
        two->setText(one->text());
        check(ok->isEnabled() && c.passphrase() == one->text(), QStringLiteral("wallet: a strong passphrase typed twice is accepted"));
        shot(&c, QStringLiteral("wallet-create"));
        WalletPassphraseDialog u(false);
        u.show();
        check(!child<QLineEdit>(&u, "walletPass2") && !child<QPushButton>(&u, "walletOk")->isEnabled(), QStringLiteral("wallet: unlocking asks once"));
        child<QLineEdit>(&u, "walletPass")->setText(QStringLiteral("x"));
        check(child<QPushButton>(&u, "walletOk")->isEnabled(), QStringLiteral("wallet: any typed passphrase may be tried"));
        shot(&u, QStringLiteral("wallet-unlock"));
    }
    // through the handler and the real wallet (a file in this test's data folder)
    crypto::Wallet &w = crypto::Wallet::standard();
    check(crypto::Wallet::available(), QStringLiteral("wallet: this build has the wallet"));
    check(!w.exists(), QStringLiteral("wallet: none yet in the test's data folder"));
    installWalletDialogs();
    check(crypto::Wallet::hasUnlockHandler(), QStringLiteral("wallet: the handler is set"));
    const QString good = QStringLiteral("harbor lantern velvet orchard meadow quartz");
    // answers every wallet window that opens while it runs (a wrong passphrase is asked for again, up to three times)
    QTimer responder;
    QString first, second;
    bool accept = true;
    responder.setInterval(60);
    QObject::connect(&responder, &QTimer::timeout, [&] {
        auto *d = qobject_cast<WalletPassphraseDialog *>(QApplication::activeModalWidget());
        if (!d)
            return;
        d->findChild<QLineEdit *>(QStringLiteral("walletPass"))->setText(first);
        if (auto *t = d->findChild<QLineEdit *>(QStringLiteral("walletPass2")))
            t->setText(second);
        if (accept)
            d->findChild<QPushButton *>(QStringLiteral("walletOk"))->click();
        else
            d->reject();
    });
    responder.start();
    auto answer = [&](const QString &f, const QString &sec, bool acc) {
        first = f;
        second = sec;
        accept = acc;
    };
    answer(good, good, false);
    QString err;
    check(w.ensureUnlocked(&err) == crypto::Wallet::Result::Locked && !w.exists(), QStringLiteral("wallet: cancelling the creation makes no wallet"));
    answer(good, good, true);
    check(w.ensureUnlocked(&err) == crypto::Wallet::Result::Ok && w.exists() && w.isUnlocked(), QStringLiteral("wallet: created through the dialog, unlocked"), err);
    w.write(QStringLiteral("entry"), QByteArray("secret"));
    w.lock();
    check(!w.isUnlocked(), QStringLiteral("wallet: locked again"));
    answer(QStringLiteral("wrong wrong wrong"), QString(), true);
    check(w.ensureUnlocked(&err) != crypto::Wallet::Result::Ok && !w.isUnlocked(), QStringLiteral("wallet: a wrong passphrase does not open it"));
    answer(good, QString(), true);
    check(w.ensureUnlocked(&err) == crypto::Wallet::Result::Ok, QStringLiteral("wallet: the right passphrase opens it, once"), err);
    QByteArray got;
    w.read(QStringLiteral("entry"), &got);
    check(got == "secret", QStringLiteral("wallet: the entries are there"));
    // the line about where the keys are kept
    const QString walletLine = keyPlaceTextFor(QStringLiteral("no key store"), false, true);
    const QString systemLine = keyPlaceTextFor(QStringLiteral("KWallet or the Secret Service"), true, true);
    check(walletLine.contains(QLatin1String("Quillbinder wallet")) && systemLine.contains(QLatin1String("KWallet or the Secret Service")) && systemLine.contains(QLatin1String("wallet already")),
          QStringLiteral("wallet: the line names the wallet or the system's key store"), walletLine + QStringLiteral(" | ") + systemLine);
    check(keyPlaceTextFor(QStringLiteral("no key store"), false, false).contains(QLatin1String("no place to keep keys")), QStringLiteral("wallet: a build without a wallet and a system without a key store says so"));
}

// ----------------------------------------------------------------------------------------------- the gate

static void testGateFiles(const QString &tmp)
{
    // the inspector's picture removal goes through the gate: in a follower's folder the card picture stays on disk
    QString err;
    auto p = Project::create(tmp + QStringLiteral("/g"), QStringLiteral("Gate"), &err);
    BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Scene"));
    const QString dir = p->contentDir(it->id);
    QDir().mkpath(dir);
    QFile card(dir + QStringLiteral("/card.png"));
    card.open(QIODevice::WriteOnly);
    card.write("png");
    card.close();
    it->synopsisImage = QStringLiteral("card.png");
    p->save(&err);
    const QString folder = p->folderPath();
    const QString id = it->id;
    p.reset();
    collab::Follower f;
    auto fp = Project::open(folder, &err, &f);
    check(bool(fp) && collab::gate().active(), QStringLiteral("gate: the project opens as a follower"), err);
    if (fp) {
        Inspector insp;
        insp.setProject(fp.get());
        insp.setItem(fp->item(id));
        insp.setSynopsisImage(QString());
        f.capture().flush();
        check(QFile::exists(folder + QStringLiteral("/content/") + id + QStringLiteral("/card.png")),
              QStringLiteral("gate: removing the synopsis picture in a follower is captured; the file stays on disk"));
        // the look file
        look::Values v = look::forNewProjects();
        const QByteArray before = [&] {
            QFile x(fp->settingsDir() + QStringLiteral("/look.json"));
            x.open(QIODevice::ReadOnly);
            return x.readAll();
        }();
        QString e;
        look::write(fp.get(), v, &e);
        f.capture().flush();
        const QByteArray after = [&] {
            QFile x(fp->settingsDir() + QStringLiteral("/look.json"));
            x.open(QIODevice::ReadOnly);
            return x.readAll();
        }();
        check(before == after, QStringLiteral("gate: the project's look is captured in a follower; look.json on disk is unchanged"));
        check(!f.capture().takeReady().isEmpty(), QStringLiteral("gate: the look arrives in the change set"));
    }
    fp.reset();
    // not a follower: the removal works as before
    auto q = Project::open(folder, &err);
    if (q) {
        Inspector insp;
        insp.setProject(q.get());
        insp.setItem(q->item(id));
        insp.setSynopsisImage(QString());
        check(!QFile::exists(folder + QStringLiteral("/content/") + id + QStringLiteral("/card.png")), QStringLiteral("gate: without a follower the picture is removed as before"));
    }
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_UPDATE_CHECK", "1");
    qputenv("QB_TEST_KEYSTORE", "memory");
    qputenv("HOME", home.path().toUtf8());
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderLs7Test"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderLs7Test"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.save();
    theme::apply();

    QTemporaryDir work;
    FakeLiveSyncControl fake;
    setLiveSyncControl(&fake);
    testOff(work.path() + QStringLiteral("/off"), fake);
    testOptionsPage(fake);
    testWindowParts(work.path() + QStringLiteral("/parts"), fake);
    testDialogs(fake);
    testWallet();
    testGateFiles(work.path());
    setLiveSyncControl(nullptr);

    std::printf("%d pass\n%d failure(s)\n", passes, failures);
    return failures ? 1 : 0;
}
