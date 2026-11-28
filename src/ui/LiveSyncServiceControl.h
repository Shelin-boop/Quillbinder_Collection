#pragma once
// The service behind the screens (0.99.0, work package 5): implements LiveSyncControl (ui/LiveSyncControl.h) on a
// collab::Service that runs inside this program. The program that has Live Sync switched on runs the service itself (its
// socket serves the windows of other programs of this device); the status model it gives out stays the same object whether
// the service runs or not.
#include "core/collab/Service.h"
#include "ui/LiveSyncControl.h"

#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>

namespace qb {
namespace livesync {

class ServiceControl : public LiveSyncControl
{
    Q_OBJECT
public:
    // an own service with the default configuration (this device's identity and data folder), or one made by the caller
    // (suites); `external` is not owned
    explicit ServiceControl(collab::Service *external = nullptr, QObject *parent = nullptr);
    ~ServiceControl() override;

    collab::Service *service() const { return m_svc; }
    // starts the service when Live Sync is switched on in the settings (called once when the program starts)
    void startIfEnabled();

    collab::StatusModel *model() override { return m_svc->statusModel(); }
    collab::StatusActions *actions() override { return m_svc->statusActions(); }
    bool enabled() const override { return m_svc->isRunning(); }
    void setEnabled(bool on) override;
    QString unavailableReason() const override { return m_unavailable; }
    QString deviceName() const override { return prefDeviceName(); }
    void setDeviceName(const QString &name) override;
    bool keepHosting() const override { return prefKeepHosting(); }
    void setKeepHosting(bool on) override;
    QStringList addresses() const override;
    int port() const override { return m_svc->port(); }
    QString fingerprint() const override { return m_svc->isRunning() ? m_svc->fingerprint() : QString(); }
    QString protocolText() const override;
    void removeDevice(const QString &deviceId) override;

    QList<DiscoveredDevice> discovered() const override;
    void openPairingWindow() override;
    void closePairingWindow() override;
    bool pairingWindowOpen() const override { return m_svc->pairingOpen(); }
    void pairWith(const QString &key) override;
    void pairWithAddress(const QString &address) override;
    void answerPairing(bool codesMatch) override { m_svc->confirmPairing(codesMatch); }
    void cancelPairing() override { m_svc->confirmPairing(false); }

    bool shareProject(const QString &folder, QString *error) override;
    QList<RemoteProject> remoteProjects() const override;
    void join(const QString &deviceId, const QString &uuid) override;

    QList<AccessEntry> access(const QString &uuid) const override;
    void setAccess(const QString &uuid, const QString &deviceId, AccessEntry::Role role) override;
    void setPriority(const QString &uuid, const QStringList &deviceIdsInOrder) override { m_svc->setPriority(uuid, deviceIdsInOrder); emit accessChanged(uuid); }
    void transferHosting(const QString &uuid, const QString &deviceId) override;

    QList<DraftInfo> drafts(const QString &uuid) const override;
    // "Keep hosting when the last window is closed": the program is about to end with projects hosted for other devices, so the
    // hosting goes to a Live Sync service process (`quillbinder --live-sync-service`), which shows its small status window. False when
    // there was nothing to hand over. `program` is the program's file (default: this one).
    bool handOffHosting(const QString &program = QString());
    // a program that starts while such a service process runs asks it to end, then hosts in its own process again
    bool takeBackFromServiceProcess(int timeoutMs = 10000);
    void compareDraft(const QString &id) override { draftAction(DraftAction::Compare, id); }
    void keepDraftAsCopy(const QString &id) override { draftAction(DraftAction::KeepAsCopy, id); }
    void takeOverDraft(const QString &id) override { draftAction(DraftAction::TakeOver, id); }
    void deleteDraft(const QString &id) override;

    void handOver(const QString &uuid, const QString &resource) override { m_svc->answerEditRequest(uuid, resource, true); }
    void decline(const QString &uuid, const QString &resource) override { m_svc->answerEditRequest(uuid, resource, false); }

private:
    void draftAction(DraftAction::Kind kind, const QString &id);
    void wire();
    collab::Service *m_svc = nullptr;
    bool m_owned = false;
    QString m_unavailable;
    QHash<QString, QTimer *> m_joins;
    mutable QElapsedTimer m_fetched;
};

} // namespace livesync
} // namespace qb
