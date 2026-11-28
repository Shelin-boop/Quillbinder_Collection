#include "ui/MainWindowLiveSync.h"

#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Objects.h"
#include "core/Project.h"
#include "core/collab/LocalLink.h"
#include "core/collab/OpApply.h"
#include "core/collab/PathClass.h"
#include "core/collab/Service.h"
#include "core/collab/net/DeviceIdentity.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

namespace qb {

namespace {
collab::Service *g_inProcess = nullptr;
bool g_keepHosting = false;
}

namespace ui {
void setInProcessLiveSyncService(collab::Service *service) { g_inProcess = service; }
collab::Service *inProcessLiveSyncService() { return g_inProcess; }
void setLiveSyncKeepHosting(bool on) { g_keepHosting = on; }
bool liveSyncKeepHosting() { return g_keepHosting; }
QString liveSyncDir() { return g_inProcess ? g_inProcess->dir() : collab::net::DeviceIdentity::standardDir(); }
} // namespace ui

// ---------------------------------------------------------------- MainWindow's side

// Whether the project's folder is one this device shares or has joined (the registry says; a program that never shared
// anything has no registry and goes on as it did)
bool MainWindow::liveSyncShared(const QString &projectFile) const
{
    const QString dir = ui::liveSyncDir();
    if (!QFileInfo::exists(dir + QStringLiteral("/registry.json")))
        return false;
    collab::Registry reg(dir);
    return reg.findByFolder(QFileInfo(projectFile).absolutePath());
}

collab::Follower *MainWindow::liveSyncFollower() const { return m_liveSync ? m_liveSync->client()->follower() : nullptr; }

bool MainWindow::liveSyncBegin(const QString &projectFile)
{
    const QString folder = QFileInfo(projectFile).absolutePath();
    collab::ServiceClient *client = nullptr;
    if (collab::Service *svc = ui::inProcessLiveSyncService()) {
        collab::net::MemLink::Pair pair = collab::net::MemLink::createPair(QByteArray(32, 'w'), QByteArray(32, 's'), nullptr);
        svc->attachWindow(pair.a);
        client = new collab::ServiceClient(pair.b);
    } else {
        const QString name = collab::LocalLink::serverName(ui::liveSyncDir());
        if (!collab::LocalServer::isServed(name)) {
            // the first window that needs it starts the service, detached; it ends by itself when nobody needs it
            QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--live-sync-service")});
            QElapsedTimer t;
            t.start();
            while (!collab::LocalServer::isServed(name, 200) && t.elapsed() < 8000) {
                QEventLoop l;
                QTimer::singleShot(100, &l, &QEventLoop::quit);
                l.exec();
            }
        }
        auto *link = collab::LocalLink::connectTo(name);
        QElapsedTimer t;
        t.start();
        while (link->state() == collab::net::Link::State::Connecting && t.elapsed() < 3000) {
            QEventLoop l;
            QTimer::singleShot(10, &l, &QEventLoop::quit);
            l.exec();
        }
        if (link->state() != collab::net::Link::State::Ready) {
            delete link;
            QMessageBox::warning(const_cast<MainWindow *>(this), tr("Open Project"),
                                 tr("This project is shared with Live Sync, but the Live Sync service of this device did not start. "
                                    "The project was not opened, so that nothing is changed outside Live Sync."));
            return false;
        }
        client = new collab::ServiceClient(link);
    }
    const collab::ServiceClient::Attached a = client->attach(folder, ui::liveSyncKeepHosting());
    if (!a.ok) {
        delete client;
        QMessageBox::warning(this, tr("Open Project"),
                             a.code == QLatin1String("NOT_SHARED")
                                 ? tr("This folder is no longer shared by Live Sync on this device.")
                                 : tr("Live Sync cannot take this project now: %1").arg(a.text.isEmpty() ? a.code : a.text));
        return false;
    }
    m_liveSync = new LiveSyncGlue(this, client);
    return true;
}

void MainWindow::liveSyncAdopted()
{
    if (m_liveSync && m_project)
        m_liveSync->begin(m_project.get());
}

void MainWindow::liveSyncEnd()
{
    if (!m_liveSync)
        return;
    m_liveSync->end();
    delete m_liveSync;
    m_liveSync = nullptr;
}

// ---------------------------------------------------------------- the glue

LiveSyncGlue::LiveSyncGlue(MainWindow *window, collab::ServiceClient *client) : QObject(window), m_window(window), m_client(client)
{
    client->setParent(this);
    m_release.setInterval(2000);
    connect(&m_release, &QTimer::timeout, this, &LiveSyncGlue::releaseUnused);
}

LiveSyncGlue::~LiveSyncGlue() = default;

QString LiveSyncGlue::titleOf(const QString &id) const
{
    BinderItem *it = m_project ? m_project->item(id) : nullptr;
    return it ? it->title : id;
}

QString LiveSyncGlue::whyReadOnly() const { return m_client->readOnly() ? m_client->readOnlyReason() : QString(); }

void LiveSyncGlue::begin(Project *project)
{
    m_project = project;
    collab::LiveFollower *f = m_client->follower();
    // what the window's saves would write goes to the service
    f->capture().setListener([this] { QMetaObject::invokeMethod(this, [this] { m_client->sendCaptured(); }, Qt::QueuedConnection); });
    connect(m_client, &collab::ServiceClient::eventApplied, this,
            [this](qint64 seq, const QString &opId, const QString &actor, bool own, const QJsonArray &ops, const QStringList &paths, const QJsonObject &) {
                onEvent(seq, opId, actor, own, ops, paths);
            });
    connect(m_client, &collab::ServiceClient::stateChanged, this, &LiveSyncGlue::onState);
    connect(m_client, &collab::ServiceClient::lockGranted, this, [this](const QString &r) { onLockResult(r, true, QString()); });
    connect(m_client, &collab::ServiceClient::lockDenied, this, [this](const QString &r, const QString &holder, const QString &reason) {
        onLockResult(r, false, holder.isEmpty() ? reason : tr("it is being edited on %1").arg(holder));
    });
    connect(m_client, &collab::ServiceClient::lockRevoked, this, [this](const QString &r, const QString &reason) {
        Q_UNUSED(reason);
        if (r.startsWith(QLatin1String("doc:")))
            m_window->showStatusNote(tr("\"%1\" is now edited on another device. Your unsaved changes there are kept as a draft.").arg(titleOf(r.mid(4))), 8000);
    });
    connect(m_client, &collab::ServiceClient::replicaReplaced, this, [this] {
        // the folder was made again from the host's files: the window opens the project anew
        const QString file = m_project ? m_project->indexPath() : QString();
        if (file.isEmpty())
            return;
        QTimer::singleShot(0, m_window, [w = m_window, file] {
            if (w->closeProject())
                w->openProject(file);
        });
    });
    connect(m_client, &collab::ServiceClient::rejected, this, [this](const QString &, const QJsonArray &rejects) {
        QStringList what;
        for (const QJsonValue &v : rejects)
            what << v.toObject().value(QStringLiteral("unit")).toString();
        m_window->showStatusNote(tr("Some changes were not accepted by the host and were kept as a draft: %1").arg(what.join(QStringLiteral(", "))), 10000);
    });
    connect(m_client, &collab::ServiceClient::serviceGone, this, [this] {
        m_window->showStatusNote(tr("The connection to the Live Sync service was lost. Your changes are kept on this device."), 0);
    });
    TextEditor::setLockGate([this](const QString &docId, bool) {
        const QString resource = QStringLiteral("doc:") + docId;
        if (m_client->holds(resource))
            return true;
        if (m_wanted.contains(docId))
            return false; // asked already, the answer is on its way
        m_wanted.insert(docId);
        m_client->lock({resource});
        return false;
    });
    m_release.start();
    onState();
}

void LiveSyncGlue::end()
{
    if (m_ended)
        return;
    m_ended = true;
    m_release.stop();
    TextEditor::setLockGate(TextEditor::LockGate());
    m_client->follower()->capture().setListener(nullptr);
    // what was captured is handed over, then every lock is given up once the host confirmed
    if (m_project)
        m_client->sendCaptured();
    m_client->flush(5000);
    if (!m_client->follower()->mine().isEmpty()) {
        m_client->unlock(m_client->follower()->mine());
        // the service gives them up once the host confirmed the flush; the window leaves after it told so
        QElapsedTimer t;
        t.start();
        while (!m_client->follower()->mine().isEmpty() && t.elapsed() < 5000 && m_client->linkAlive()) {
            QEventLoop l;
            QTimer::singleShot(10, &l, &QEventLoop::quit);
            l.exec();
        }
    }
    m_client->detach();
}

void LiveSyncGlue::onState()
{
    const int st = m_client->stateCode();
    if (st != m_lastState) {
        m_lastState = st;
        if (st == int(collab::ShareState::NotConnected))
            m_window->showStatusNote(tr("Not connected. Your changes are kept on this device."), 0);
    }
    m_window->updateActions();
}

void LiveSyncGlue::onLockResult(const QString &resource, bool granted, const QString &text)
{
    if (!resource.startsWith(QLatin1String("doc:")))
        return;
    const QString id = resource.mid(4);
    m_wanted.remove(id);
    TextEditor::lockResult(id, granted);
    if (!granted)
        m_window->showStatusNote(tr("\"%1\" cannot be edited here now: %2").arg(titleOf(id), text.isEmpty() ? tr("the lock was refused") : text), 8000);
}

void LiveSyncGlue::releaseUnused()
{
    if (m_ended || !m_project || m_client->busy())
        return;
    QStringList give;
    for (const QString &r : m_client->follower()->mine()) {
        if (r.startsWith(QLatin1String("doc:")) && !TextEditor::shown(r.mid(4)))
            give << r;
    }
    if (!give.isEmpty())
        m_client->unlock(give);
}

// A commit of another device (or the host's derived data of this one) is in the folder: the open project takes it in. Text that
// has unsaved changes here is never overwritten (DocumentStore::reload skips it): the document is locked elsewhere, so there is
// none unless the lock was lost, and then the service keeps the change as a draft.
void LiveSyncGlue::onEvent(qint64, const QString &, const QString &, bool own, const QJsonArray &opsJson, const QStringList &paths)
{
    Project *p = m_project.data();
    if (!p || m_ended)
        return;
    collab::WindowScope window;
    collab::Capture &cap = m_client->follower()->capture();
    cap.setSuspended(true);
    p->beginRemoteApply();
    p->acknowledgeIndexOnDisk();
    if (!own) {
        collab::OpList ops;
        for (const QJsonValue &v : opsJson) {
            collab::Op op;
            if (collab::proto::fromJson(v.toObject(), &op))
                ops << op;
        }
        QString err;
        if (!ops.isEmpty() && !collab::applyToProject(p, ops, &err))
            m_window->showStatusNote(tr("A change from another device could not be taken in; close and open the project again. (%1)").arg(err), 0);
        QSet<QString> docs;
        bool objects = false;
        for (const QString &path : paths) {
            const collab::PathInfo info = collab::classify(path);
            if (info.kind == collab::PathKind::Text || info.kind == collab::PathKind::Notes || info.kind == collab::PathKind::MediaFile)
                docs.insert(info.id);
            else if (info.kind == collab::PathKind::Object || info.kind == collab::PathKind::ObjectSchema)
                objects = true;
        }
        for (const collab::Op &op : ops)
            if (op.kind == collab::Op::ItemCreate)
                docs.insert(op.id);
        for (const QString &id : docs)
            if (p->item(id)) {
                p->docs()->reload(id);
                p->docs()->recount(id);
            }
        if (objects && p->objects())
            p->objects()->load();
    }
    p->endRemoteApply();
    cap.setSuspended(false);
    m_window->updateActions();
}

} // namespace qb
