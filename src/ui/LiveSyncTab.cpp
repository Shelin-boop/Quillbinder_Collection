#include "ui/LiveSyncTab.h"

#include "ui/HintLabel.h"
#include "ui/LiveSyncDialogs.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace qb {
namespace livesync {

QString resourceText(const QString &resource, const TitleResolver &titles)
{
    if (resource.startsWith(QLatin1String("doc:"))) {
        const QString t = titles ? titles(resource.mid(4)) : QString();
        return t.isEmpty() ? QCoreApplication::translate("LiveSync", "a document") : t;
    }
    if (resource == QLatin1String("structure"))
        return QCoreApplication::translate("LiveSync", "the binder's structure");
    if (resource == QLatin1String("settings"))
        return QCoreApplication::translate("LiveSync", "the project's settings");
    if (resource.startsWith(QLatin1String("object:")))
        return QCoreApplication::translate("LiveSync", "an object");
    if (resource.startsWith(QLatin1String("mindmap:")))
        return QCoreApplication::translate("LiveSync", "a mind map");
    if (resource == QLatin1String("project"))
        return QCoreApplication::translate("LiveSync", "the whole project");
    return resource;
}

namespace {

QString roleName(collab::ShareRole r)
{
    switch (r) {
    case collab::ShareRole::Host:
        return QCoreApplication::translate("LiveSync", "Host");
    case collab::ShareRole::Editor:
        return QCoreApplication::translate("LiveSync", "Editor");
    case collab::ShareRole::Reader:
        return QCoreApplication::translate("LiveSync", "Reader");
    }
    return QString();
}

QString seenText(const QDateTime &t)
{
    return t.isValid() ? QLocale().toString(t, QLocale::ShortFormat) : QString();
}

} // namespace

// ================================================================== the tab

LiveSyncTab::LiveSyncTab(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("liveSyncTab"));
    auto *v = new QVBoxLayout(this);
    m_summary = new QLabel(this);
    m_summary->setObjectName(QStringLiteral("lsSummary"));
    m_summary->setWordWrap(true);
    m_summary->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(m_summary);
    m_people = new QTreeWidget(this);
    m_people->setObjectName(QStringLiteral("lsPeople"));
    m_people->setAccessibleName(tr("Participants"));
    m_people->setRootIsDecorated(false);
    m_people->setHeaderLabels({tr("Device"), tr("State"), tr("Editing")});
    m_people->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_people->setMinimumHeight(90);
    v->addWidget(m_people, 1);

    m_backup = new QLabel(this);
    m_backup->setObjectName(QStringLiteral("lsBackup"));
    m_backup->setWordWrap(true);
    v->addWidget(m_backup);
    m_requestBackup = new QPushButton(tr("Request Backup"), this);
    m_requestBackup->setObjectName(QStringLiteral("lsRequestBackup"));
    v->addWidget(m_requestBackup);

    auto *drow = new QHBoxLayout;
    m_drafts = new QLabel(this);
    m_drafts->setObjectName(QStringLiteral("lsDrafts"));
    m_openDrafts = new QPushButton(tr("Recovery Drafts..."), this);
    m_openDrafts->setObjectName(QStringLiteral("lsOpenDrafts"));
    drow->addWidget(m_drafts, 1);
    drow->addWidget(m_openDrafts);
    v->addLayout(drow);

    auto *box = new QGroupBox(tr("Selected Document"), this);
    auto *bv = new QVBoxLayout(box);
    m_doc = new QLabel(box);
    m_doc->setObjectName(QStringLiteral("lsDoc"));
    m_doc->setWordWrap(true);
    bv->addWidget(m_doc);
    auto *row = new QHBoxLayout;
    m_ask = new QPushButton(tr("Ask to Edit"), box);
    m_ask->setObjectName(QStringLiteral("lsAsk"));
    m_take = new QPushButton(tr("Take Over"), box);
    m_take->setObjectName(QStringLiteral("lsTake"));
    m_stop = new QPushButton(tr("Stop Editing"), box);
    m_stop->setObjectName(QStringLiteral("lsStop"));
    row->addWidget(m_ask);
    row->addWidget(m_take);
    row->addWidget(m_stop);
    bv->addLayout(row);
    v->addWidget(box);

    LiveSyncControl &c = liveSyncControl();
    connect(m_requestBackup, &QPushButton::clicked, this, [this] { liveSyncControl().actions()->requestBackup(m_uuid); });
    connect(m_openDrafts, &QPushButton::clicked, this, [this] { showDraftsDialog(window(), m_uuid); });
    connect(m_ask, &QPushButton::clicked, this, [this] { liveSyncControl().actions()->askToEdit(m_uuid, docResource(m_docId)); });
    connect(m_take, &QPushButton::clicked, this, [this] { liveSyncControl().actions()->takeOver(m_uuid, docResource(m_docId)); });
    connect(m_stop, &QPushButton::clicked, this, [this] { liveSyncControl().actions()->stopEditing(m_uuid, docResource(m_docId)); });
    connect(c.model(), &collab::StatusModel::projectChanged, this, [this](const QString &u) {
        if (u == m_uuid)
            refresh();
    });
    connect(c.model(), &collab::StatusModel::projectsChanged, this, &LiveSyncTab::refresh);
    connect(c.model(), &collab::StatusModel::locksChanged, this, [this](const QString &u) {
        if (u == m_uuid)
            refresh();
    });
    connect(&c, &LiveSyncControl::draftsChanged, this, &LiveSyncTab::refresh);
    refresh();
}

void LiveSyncTab::setContext(const QString &uuid, const QString &docId, const TitleResolver &titles)
{
    m_uuid = uuid;
    m_docId = docId;
    m_titles = titles;
    refresh();
}

void LiveSyncTab::refresh()
{
    LiveSyncControl &c = liveSyncControl();
    const collab::ProjectStatus p = c.model()->project(m_uuid);
    m_people->clear();
    const bool shared = p.valid;
    m_requestBackup->setEnabled(shared);
    m_openDrafts->setEnabled(shared && p.draftCount > 0);
    if (!shared) {
        m_summary->setText(tr("This project is not shared."));
        m_backup->clear();
        m_drafts->clear();
        m_doc->clear();
        m_ask->setEnabled(false);
        m_take->setEnabled(false);
        m_stop->setEnabled(false);
        return;
    }
    QString s = tr("Host: %1\nYour role: %2\nState: %3")
                    .arg(p.thisDeviceHosts() ? tr("this device") : p.hostName, roleName(p.role), collab::shareStateText(p.state));
    if (!p.problemText.isEmpty())
        s += QStringLiteral("\n") + p.problemText;
    if (p.readOnly && !p.readOnlyReason.isEmpty())
        s += QStringLiteral("\n") + tr("Read-only: %1").arg(p.readOnlyReason);
    m_summary->setText(s);
    for (const collab::ParticipantStatus &x : p.participants) {
        QStringList editing;
        for (const QString &r : x.editing)
            editing << resourceText(r, m_titles);
        QString name = x.name;
        if (x.thisDevice)
            name += QLatin1Char(' ') + tr("(this device)");
        if (x.isHost)
            name += QLatin1Char(' ') + tr("(host)");
        const QString state = x.online ? tr("Online") : (x.lastSeen.isValid() ? tr("Not connected since %1").arg(seenText(x.lastSeen)) : tr("Not connected"));
        new QTreeWidgetItem(m_people, {name, state, editing.join(QStringLiteral(", "))});
    }
    const QString b = backupText(p.backup);
    m_backup->setText(b);
    m_drafts->setText(p.draftCount > 0 ? tr("%n recovery draft(s) on this device", nullptr, p.draftCount) : tr("No recovery drafts."));

    // the selected document
    m_ask->setEnabled(false);
    m_take->setEnabled(false);
    m_stop->setEnabled(false);
    if (m_docId.isEmpty()) {
        m_doc->setText(tr("No document is selected."));
        return;
    }
    const QString title = resourceText(docResource(m_docId), m_titles);
    const collab::LockInfo li = c.model()->lockInfo(m_uuid, docResource(m_docId));
    if (!li.held) {
        m_doc->setText(tr("Nobody is editing \"%1\".").arg(title));
    } else if (li.mine) {
        m_doc->setText(tr("You are editing \"%1\".").arg(title));
        m_stop->setEnabled(true);
    } else {
        QString t = li.unreachable ? tr("\"%1\" is held by %2, who has not been reachable since %3.").arg(title, li.holderName, seenText(li.since))
                                   : tr("\"%1\" is being edited on %2.").arg(title, li.holderName);
        if (li.askOpen) {
            if (li.askWaitMs <= 0)
                t += QLatin1Char(' ') + tr("There was no answer.");
            else if (li.declined)
                t += QLatin1Char(' ') + tr("The request was declined.");
            else
                t += QLatin1Char(' ') + tr("You asked; waiting for the answer.");
        }
        m_doc->setText(t);
        m_ask->setEnabled(!li.askOpen && p.role != collab::ShareRole::Reader && !p.readOnly);
        m_take->setEnabled(li.askOpen && li.askWaitMs <= 0 && p.role != collab::ShareRole::Reader);
    }
}

// ================================================================== the bars

LiveSyncBars::LiveSyncBars(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("liveSyncBars"));
    m_lay = new QVBoxLayout(this);
    m_lay->setContentsMargins(0, 0, 0, 0);
    m_lay->setSpacing(1);
    hide();
    LiveSyncControl &c = liveSyncControl();
    connect(&c, &LiveSyncControl::editAsked, this, [this](const QString &uuid, const QString &res, const QString &who) {
        m_asked.insert(uuid + QLatin1Char('|') + res, who);
        refresh();
    });
    connect(&c, &LiveSyncControl::editAskEnded, this, [this](const QString &uuid, const QString &res) {
        m_asked.remove(uuid + QLatin1Char('|') + res);
        refresh();
    });
    connect(c.model(), &collab::StatusModel::projectChanged, this, [this](const QString &u) {
        if (u == m_uuid)
            refresh();
    });
    connect(c.model(), &collab::StatusModel::projectsChanged, this, &LiveSyncBars::refresh);
    connect(c.model(), &collab::StatusModel::locksChanged, this, [this](const QString &u) {
        if (u == m_uuid)
            refresh();
    });
}

void LiveSyncBars::setContext(const QString &uuid, const QString &docId, const TitleResolver &titles)
{
    m_uuid = uuid;
    m_docId = docId;
    m_titles = titles;
    refresh();
}

QFrame *LiveSyncBars::makeBar(const QString &name, const QString &text, const QList<std::pair<QString, std::function<void()>>> &buttons)
{
    auto *f = new QFrame(this);
    f->setObjectName(name);
    f->setFrameShape(QFrame::StyledPanel);
    f->setAutoFillBackground(true);
    QPalette pal = f->palette();
    pal.setColor(QPalette::Window, pal.color(QPalette::AlternateBase));
    f->setPalette(pal);
    auto *l = new QHBoxLayout(f);
    l->setContentsMargins(8, 4, 8, 4);
    auto *label = new QLabel(text, f);
    label->setObjectName(name + QStringLiteral("Text"));
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    l->addWidget(label, 1);
    for (const auto &b : buttons) {
        auto *btn = new QPushButton(b.first, f);
        btn->setObjectName(name + QStringLiteral(":") + b.first);
        const auto fn = b.second;
        connect(btn, &QPushButton::clicked, this, [fn] { fn(); });
        l->addWidget(btn);
    }
    m_lay->addWidget(f);
    return f;
}

int LiveSyncBars::barCount() const
{
    int n = 0;
    for (int i = 0; i < m_lay->count(); ++i)
        if (m_lay->itemAt(i)->widget())
            ++n;
    return n;
}

QString LiveSyncBars::barText(const QString &name) const
{
    for (int i = 0; i < m_lay->count(); ++i)
        if (QWidget *w = m_lay->itemAt(i)->widget())
            if (w->objectName() == name)
                if (auto *l = w->findChild<QLabel *>(name + QStringLiteral("Text")))
                    return l->text();
    return QString();
}

void LiveSyncBars::refresh()
{
    // remove the old bars at once (deleteLater would leave them in the layout until the next event round)
    while (QLayoutItem *it = m_lay->takeAt(0)) {
        if (QWidget *w = it->widget()) {
            w->hide();
            w->setParent(nullptr);
            w->deleteLater();
        }
        delete it;
    }
    LiveSyncControl &c = liveSyncControl();
    const collab::ProjectStatus p = c.model()->project(m_uuid);
    if (p.valid && c.enabled()) {
        const QString uuid = m_uuid;
        collab::StatusActions *act = c.actions();
        if (p.state == collab::ShareState::Problem && !p.problemText.isEmpty())
            makeBar(QStringLiteral("barProblem"), p.problemText, {});
        if (p.state == collab::ShareState::NotConnected)
            makeBar(QStringLiteral("barOffline"), tr("Not connected. Your changes are kept on this device."), {});
        else if (p.readOnly && !p.readOnlyReason.isEmpty())
            makeBar(QStringLiteral("barAccess"), tr("Read-only: %1").arg(p.readOnlyReason), {});
        // another device asks to edit a document this device holds
        const QString prefix = uuid + QLatin1Char('|');
        const QStringList keys = m_asked.keys();
        for (const QString &k : keys) {
            if (!k.startsWith(prefix))
                continue;
            const QString res = k.mid(prefix.size());
            const QString who = m_asked.value(k);
            makeBar(QStringLiteral("barAsked"),
                    tr("%1 asks to edit \"%2\".").arg(who, resourceText(res, m_titles)),
                    {{tr("Save and Hand Over"), [uuid, res] { liveSyncControl().handOver(uuid, res); }},
                     {tr("Decline"), [uuid, res] { liveSyncControl().decline(uuid, res); }}});
        }
        // the selected document, held by another device
        if (!m_docId.isEmpty()) {
            const QString res = docResource(m_docId);
            const QString title = resourceText(res, m_titles);
            const collab::LockInfo li = c.model()->lockInfo(uuid, res);
            if (li.held && !li.mine && !p.readOnly) {
                const QString holder = li.holderName.isEmpty() ? tr("another device") : li.holderName;
                if (li.askOpen && li.askWaitMs <= 0) {
                    makeBar(QStringLiteral("barWaiting"),
                            tr("%1 has not answered. You can take over \"%2\"; the changes %1 has not saved stay on %1 as a recovery draft.")
                                .arg(holder, title),
                            {{tr("Take Over"), [act, uuid, res] { act->takeOver(uuid, res); }},
                             {tr("Cancel"), [act, uuid, res] { act->cancelAsk(uuid, res); }}});
                } else if (li.askOpen) {
                    makeBar(QStringLiteral("barWaiting"),
                            li.declined ? tr("\"%1\" is being worked on on %2.").arg(title, holder)
                                        : tr("Asked %1 to let you edit \"%2\"...").arg(holder, title),
                            {{tr("Cancel"), [act, uuid, res] { act->cancelAsk(uuid, res); }}});
                } else {
                    makeBar(QStringLiteral("barReadOnly"),
                            li.unreachable ? tr("Read-only: held by %1, who has not been reachable since %2.")
                                                   .arg(holder, li.since.isValid() ? QLocale().toString(li.since, QLocale::ShortFormat) : tr("a while"))
                                           : tr("Read-only: being edited on %1.").arg(holder),
                            {{tr("Ask to Edit"), [act, uuid, res] { act->askToEdit(uuid, res); }}});
                }
            }
        }
    }
    setVisible(barCount() > 0);
}

} // namespace livesync
} // namespace qb
