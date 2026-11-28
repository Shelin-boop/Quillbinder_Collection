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
#include "ui/LiveSyncServiceControl.h"
#include "ui/CompareDialog.h"
#include "ui/LiveSyncControl.h"
#include "ui/LiveSyncDialogs.h"
#include "ui/LiveSyncUi.h"
#include "import/Compare.h"
#include "import/CompareFiles.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QVBoxLayout>

namespace qb {

namespace {
collab::Service *g_inProcess = nullptr;
bool g_keepHosting = false;
}

namespace ui {
void setInProcessLiveSyncService(collab::Service *service) { g_inProcess = service; }
collab::Service *inProcessLiveSyncService() { return g_inProcess; }
void setLiveSyncKeepHosting(bool on) { g_keepHosting = on; }
bool liveSyncKeepHosting() { return g_keepHosting || livesync::prefKeepHosting(); } // (the writer's choice in the options; a window tells it to the service it attaches to)
QString liveSyncDir() { return g_inProcess ? g_inProcess->dir() : collab::net::DeviceIdentity::standardDir(); }
} // namespace ui

// ---------------------------------------------------------------- MainWindow's side

// Whether the project's folder is one this device shares or has joined (the registry says; a program that never shared
// anything has no registry and goes on as it did)
// The service this window is a window of: the in-process service of the program when the window was made (it stays this window's
// when the program's pointer is changed meanwhile: the suites run several services in one process), else the program's.
collab::Service *MainWindow::liveSyncService() const
{
    if (auto *s = qobject_cast<collab::Service *>(m_liveSyncService.data()))
        return s;
    return ui::inProcessLiveSyncService();
}

bool MainWindow::liveSyncShared(const QString &projectFile) const
{
    const QString dir = liveSyncService() ? liveSyncService()->dir() : ui::liveSyncDir();
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
    if (collab::Service *svc = liveSyncService()) {
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

// Without "Keep hosting when the last window is closed", closing the last window of a project this device hosts ends the hosting: with
// other devices online the writer is asked first.
bool MainWindow::liveSyncConfirmClose()
{
    collab::Service *svc = liveSyncService();
    if (!m_liveSync || !m_project || !svc || !svc->isRunning() || svc->keepHosting())
        return true;
    const QString uuid = m_liveSync->client()->uuid();
    if (!svc->isHosting(uuid))
        return true;
    for (QWidget *w : QApplication::topLevelWidgets())
        if (w != this && qobject_cast<MainWindow *>(w) && w->isVisible())
            return true; // another window of this program goes on
    int others = 0;
    for (const collab::ParticipantStatus &p : svc->statusModel()->project(uuid).participants)
        others += !p.thisDevice && p.online;
    if (others == 0)
        return true;
    return livesync::confirmAction(this, tr("Close and Stop Hosting"),
                                   tr("%n other device(s) are working on \"%1\" through this device. Closing the last window stops the hosting: they cannot "
                                      "change the project until it is hosted again. Turn on \"Keep hosting when the last window is closed\" in File > Options > "
                                      "Live Sync to go on hosting.",
                                      nullptr, others)
                                       .arg(m_project->title()),
                                   tr("Close"));
}

void MainWindow::liveSyncAdopted()
{
    if (m_liveSyncUi)
        m_liveSyncUi->setProject(m_project.get());
    if (m_liveSync && m_project)
        m_liveSync->begin(m_project.get());
}

void MainWindow::liveSyncEnd()
{
    if (m_liveSyncUi)
        m_liveSyncUi->setProject(nullptr);
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

LiveSyncGlue::~LiveSyncGlue()
{
    if (!m_jobsFolder.isEmpty() && snapshots::remoteJobsFor(m_jobsFolder) == m_jobs.get())
        snapshots::setRemoteJobs(m_jobsFolder, nullptr);
}

namespace {
// The snapshots of a shared project are changed by its host: the window asks its service, which asks the host (or is the host).
class WindowJobs : public snapshots::RemoteJobs {
public:
    explicit WindowJobs(collab::ServiceClient *client) : m_c(client) {}
    bool take(const QString &docId, const QString &title, QString *error) override
    {
        return run({{"op", "take"}, {"doc", docId}, {"title", title}}, error);
    }
    bool setTitle(const QString &docId, const QString &file, const QString &title, QString *error) override
    {
        return run({{"op", "rename"}, {"doc", docId}, {"file", file}, {"title", title}}, error);
    }
    bool remove(const QString &docId, const QString &file, QString *error) override
    {
        return run({{"op", "delete"}, {"doc", docId}, {"file", file}}, error);
    }
    bool rollBack(const QString &docId, const QString &file, bool allowEmpty, QString *error) override
    {
        return run({{"op", "rollBack"}, {"doc", docId}, {"file", file}, {"allowEmpty", allowEmpty}}, error);
    }

private:
    bool run(const QJsonObject &body, QString *error)
    {
        QString code, text;
        if (m_c && m_c->snapshotJob(body, &code, &text))
            return true;
        if (error)
            *error = !text.isEmpty() ? text
                     : code == QLatin1String("TIMEOUT") || code == QLatin1String("LINK")
                         ? QCoreApplication::translate("qb::LiveSyncGlue", "The Live Sync service did not answer.")
                         : QCoreApplication::translate("qb::LiveSyncGlue", "The host refused the snapshot job (%1).").arg(code);
        return false;
    }
    QPointer<collab::ServiceClient> m_c;
};
} // namespace

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
        if (r.startsWith(QLatin1String("doc:"))) {
            m_window->showStatusNote(tr("\"%1\" is now edited on another device. Your unsaved changes there are kept as a draft.").arg(titleOf(r.mid(4))), 8000);
            // what was typed and not yet saved is handed to the service now: without the lock the host refuses it and it is kept as a
            // recovery draft (Live Sync > Recovery Drafts); it is never written over the other device's text
            QTimer::singleShot(0, this, [this] {
                if (m_project && !m_ended) {
                    QString err;
                    m_project->save(&err);
                }
            });
        }
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
    TextEditor::setLockGate(m_project, [this](const QString &docId, bool) {
        const QString resource = QStringLiteral("doc:") + docId;
        if (m_client->holds(resource))
            return true;
        if (m_wanted.contains(docId))
            return false; // asked already, the answer is on its way
        m_wanted.insert(docId);
        m_client->lock({resource});
        return false;
    });
    m_jobs = std::make_unique<WindowJobs>(m_client);
    m_jobsFolder = project->folderPath();
    snapshots::setRemoteJobs(m_jobsFolder, m_jobs.get());
    m_release.start();
    onState();
}

void LiveSyncGlue::end()
{
    if (m_ended)
        return;
    m_ended = true;
    m_release.stop();
    if (!m_jobsFolder.isEmpty() && snapshots::remoteJobsFor(m_jobsFolder) == m_jobs.get())
        snapshots::setRemoteJobs(m_jobsFolder, nullptr);
    TextEditor::setLockGate(m_project, TextEditor::LockGate());
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

bool LiveSyncGlue::lockAndWait(const QString &docId, QString *why, int timeoutMs)
{
    const QString resource = QStringLiteral("doc:") + docId;
    if (m_client->holds(resource))
        return true;
    bool answered = false, granted = false;
    QString text;
    auto c1 = connect(m_client, &collab::ServiceClient::lockGranted, this, [&](const QString &r) {
        if (r == resource) {
            answered = granted = true;
        }
    });
    auto c2 = connect(m_client, &collab::ServiceClient::lockDenied, this, [&](const QString &r, const QString &holder, const QString &reason) {
        if (r == resource) {
            answered = true;
            text = holder.isEmpty() ? reason : tr("it is being edited on %1").arg(holder);
        }
    });
    m_client->lock({resource});
    QElapsedTimer t;
    t.start();
    while (!answered && t.elapsed() < timeoutMs && m_client->linkAlive()) {
        QEventLoop l;
        QTimer::singleShot(10, &l, &QEventLoop::quit);
        l.exec();
    }
    disconnect(c1);
    disconnect(c2);
    if (!granted && why)
        *why = text.isEmpty() ? tr("the lock was not granted in time") : text;
    return granted;
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
    TextEditor::lockResult(m_project, id, granted);
    if (!granted)
        m_window->showStatusNote(tr("\"%1\" cannot be edited here now: %2").arg(titleOf(id), text.isEmpty() ? tr("the lock was refused") : text), 8000);
}

void LiveSyncGlue::releaseUnused()
{
    if (m_ended || !m_project || m_client->busy())
        return;
    QStringList give;
    for (const QString &r : m_client->follower()->mine()) {
        if (r.startsWith(QLatin1String("doc:")) && !TextEditor::shown(m_project, r.mid(4)))
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

namespace qb {
namespace ui {

// the program's control of Live Sync: made once, with a service of its own that starts when Live Sync is on in the settings
livesync::LiveSyncControl *programLiveSyncControl()
{
    static livesync::ServiceControl *control = nullptr;
    if (!control && !livesync::hasLiveSyncControl()) {
        control = new livesync::ServiceControl(nullptr, qApp);
        livesync::setLiveSyncControl(control);
        control->startIfEnabled();
    }
    return control;
}

} // namespace ui

void MainWindow::liveSyncInstallUi()
{
    if (m_liveSyncUi)
        return;
    ui::programLiveSyncControl();
    m_liveSyncService = ui::inProcessLiveSyncService();
    m_liveSyncUi = new livesync::LiveSyncUi(this);
    m_liveSyncUi->installToolbarButton(m_toolbar);
    m_liveSyncUi->installInspectorTab(m_inspector);
    m_liveSyncUi->attachBinder(m_binder);
    if (auto *lay = qobject_cast<QVBoxLayout *>(m_editorArea->layout()))
        lay->insertWidget(1, m_liveSyncUi->bars());
    connect(m_liveSyncUi, &livesync::LiveSyncUi::openProjectRequested, this, [this](const QString &folder) {
        const QString file = Project::resolveProjectFile(folder);
        openProject(file.isEmpty() ? folder : file);
    });
    // a project shared from this window is opened again as a follower of the service (closing saves it first)
    connect(&livesync::liveSyncControl(), &livesync::LiveSyncControl::accessChanged, this, [this] {
        QTimer::singleShot(0, this, [this] {
            if (!m_project || m_liveSync || m_project->isFollower())
                return;
            const QString file = Project::resolveProjectFile(m_project->folderPath());
            if (m_liveSyncReopening || file.isEmpty() || !liveSyncShared(file))
                return;
            m_liveSyncReopening = true; // (opening spins the event loop: another signal must not start a second reopening)
            if (closeProject())
                openProject(file);
            m_liveSyncReopening = false;
        });
    });
    connect(&livesync::liveSyncControl(), &livesync::LiveSyncControl::draftActionRequested, this,
            [this](livesync::DraftAction *action) { liveSyncDraftAction(action); }, Qt::DirectConnection);
    if (m_project)
        m_liveSyncUi->setProject(m_project.get());
}

// A recovery draft's text goes into the project from this window: compared with the document (the Compare view), kept as a new
// document beside it, or taken over (the document's lock, a snapshot of its present text first, then the draft's text as a change).
// The draft itself stays in the list until the writer deletes it.
void MainWindow::liveSyncDraftAction(livesync::DraftAction *a)
{
    if (!a || a->handled || !m_project || !m_liveSync || !m_project->isFollower() || m_liveSync->client()->uuid() != a->uuid)
        return;
    a->handled = true;
    livesync::LiveSyncControl &control = livesync::liveSyncControl();
    BinderItem *item = m_project->item(a->docId);
    if (!item || !item->hasText()) {
        emit control.draftNote(tr("The document of this draft is not in the project any more."));
        return;
    }
    QTextDocument draft;
    m_project->docs()->deserialize(a->docId, a->text, &draft, a->rtf);
    if (a->kind == livesync::DraftAction::KeepAsCopy) {
        BinderItem *parent = item->parent ? item->parent : m_project->draft();
        BinderItem *copy = m_project->model()->createItem(parent, item->parent ? item->row() + 1 : -1, ItemType::Text, tr("%1 (draft)").arg(item->title));
        if (!copy) {
            emit control.draftNote(tr("The copy could not be made."));
            return;
        }
        QTextCursor c(m_project->docs()->text(copy->id));
        c.insertFragment(QTextDocumentFragment(&draft));
        QString err;
        if (!m_project->save(&err)) {
            emit control.draftNote(tr("The copy could not be saved: %1").arg(err));
            return;
        }
        emit control.draftNote(tr("The draft is kept as the new document \"%1\".").arg(copy->title));
        return;
    }
    if (a->kind == livesync::DraftAction::Compare) {
        QTemporaryFile file(QDir::tempPath() + QStringLiteral("/qb-draft-XXXXXX.txt"));
        if (!file.open()) {
            emit control.draftNote(tr("The draft cannot be shown: no temporary file."));
            return;
        }
        file.write(draft.toPlainText().toUtf8());
        file.flush();
        QString err;
        std::unique_ptr<compare::FileText> text = compare::openFile(file.fileName(), m_project->formatting, &err);
        if (!text) {
            emit control.draftNote(tr("The draft cannot be shown: %1").arg(err));
            return;
        }
        compare::Result r = compare::compareTexts(m_project.get(), {a->docId}, m_project->docs()->text(a->docId)->toPlainText(), text.get());
        r.filePath = tr("Recovery draft of \"%1\"").arg(item->title);
        // the draft's text is taken into the document only with its lock (the compare window changes the document itself)
        QString why;
        if (!m_liveSync->lockAndWait(a->docId, &why))
            emit control.draftNote(tr("The draft is shown for reading only: \"%1\" cannot be changed here now (%2).").arg(item->title, why));
        CompareDialog dlg(m_project.get(), r, this);
        dlg.exec();
        return;
    }
    // Take Over
    QString why;
    if (!m_liveSync->lockAndWait(a->docId, &why)) {
        emit control.draftNote(tr("\"%1\" cannot be taken over now: %2.").arg(item->title, why));
        return;
    }
    QString err;
    if (!snapshots::take(m_project.get(), a->docId, tr("Before taking over a recovery draft"), &err)) {
        emit control.draftNote(tr("Nothing was changed: the snapshot of the present text could not be made (%1).").arg(err));
        return;
    }
    QTextCursor c(m_project->docs()->text(a->docId));
    c.beginEditBlock();
    c.select(QTextCursor::Document);
    c.removeSelectedText();
    c.insertFragment(QTextDocumentFragment(&draft));
    c.endEditBlock();
    if (!m_project->save(&err)) {
        emit control.draftNote(tr("The draft's text could not be saved: %1").arg(err));
        return;
    }
    emit control.draftNote(tr("The draft's text is now the text of \"%1\"; a snapshot of the text before is kept. The draft stays in the list.").arg(item->title));
}

void MainWindow::liveSyncSelected(const QString &docId)
{
    if (m_liveSyncUi)
        m_liveSyncUi->setSelectedDocument(docId);
}

} // namespace qb
