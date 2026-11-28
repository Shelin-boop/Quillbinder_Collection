#pragma once
// Live Sync's interface to the program (0.99.0, work package 7): what the windows and dialogs of Live Sync ask of the
// service beyond the read-only StatusModel and its StatusActions (core/collab/StatusModel.h): the switch, the device's
// name, pairing, joining, access of the hosted projects, recovery drafts and the answers to edit requests.
//
// The service of a program (work package 5) implements LiveSyncControl and hands it to the interface with
// setLiveSyncControl(); until then the built-in stand-in answers: it keeps only the switch, the device's name and the
// hosting option (settings of this program, Live Sync off), and every other call does nothing. A call returns at once;
// the result shows in the model's signals or in the signals below.
#include "core/collab/StatusModel.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

namespace qb {
namespace livesync {

// A device found in the network (a paired one or one with an open pairing window).
struct DiscoveredDevice {
    QString key;       // what pairWith() takes (an instance id or an address)
    QString name;      // "" while its pairing window is closed (names are shown only then)
    QString address;
    bool pairingOpen = false;
    bool paired = false;
};

// A project shared by a paired device, as the join dialog lists it.
struct RemoteProject {
    QString uuid;
    QString title;
    QString hostDeviceId;
    QString hostName;
    bool hostOnline = false;
};

// One device's access to a hosted project.
struct AccessEntry {
    enum Role { None, Editor, Reader };
    QString deviceId;
    QString name;
    Role role = None;
    bool online = false;
    bool mayHost = true;  // may become host (desktops by default; Android and readers only when allowed)
};

// A recovery draft: changes kept on this device that were not accepted by the host.
struct DraftInfo {
    QString id;
    QString uuid;
    QString docId;
    QString docTitle;
    QDateTime when;
    QString reason;   // "not sent before the takeover", "made after the split" (translated)
};

class LiveSyncControl : public QObject
{
    Q_OBJECT
public:
    explicit LiveSyncControl(QObject *parent = nullptr) : QObject(parent) {}

    virtual collab::StatusModel *model() = 0;
    virtual collab::StatusActions *actions() = 0;

    // ---- the options page
    virtual bool enabled() const = 0;
    virtual void setEnabled(bool on) = 0;
    // why it cannot be switched on here ("" when it can)
    virtual QString unavailableReason() const { return QString(); }
    virtual QString deviceName() const = 0;
    virtual void setDeviceName(const QString &name) = 0;
    virtual bool keepHosting() const = 0;
    virtual void setKeepHosting(bool on) = 0;
    virtual QStringList addresses() const { return {}; }
    virtual int port() const { return 0; }
    virtual QString fingerprint() const { return QString(); }
    virtual QString protocolText() const { return QString(); }
    virtual QString roundTripText() const { return QString(); }
    virtual void removeDevice(const QString &deviceId) { Q_UNUSED(deviceId) }

    // ---- pairing
    virtual QList<DiscoveredDevice> discovered() const { return {}; }
    virtual void openPairingWindow() {}                       // the host's side: 120 s
    virtual void closePairingWindow() {}
    virtual bool pairingWindowOpen() const { return false; }
    virtual void pairWith(const QString &key) { Q_UNUSED(key) }
    virtual void pairWithAddress(const QString &address) { Q_UNUSED(address) }
    virtual void answerPairing(bool codesMatch) { Q_UNUSED(codesMatch) }
    virtual void cancelPairing() {}

    // ---- sharing and joining
    virtual bool shareProject(const QString &folder, QString *error)
    {
        Q_UNUSED(folder)
        if (error)
            *error = tr("Live Sync is not running.");
        return false;
    }
    virtual QList<RemoteProject> remoteProjects() const { return {}; }
    virtual void join(const QString &deviceId, const QString &uuid) { Q_UNUSED(deviceId) Q_UNUSED(uuid) }

    // ---- hosted projects
    virtual QList<AccessEntry> access(const QString &uuid) const { Q_UNUSED(uuid) return {}; }
    virtual void setAccess(const QString &uuid, const QString &deviceId, AccessEntry::Role role)
    {
        Q_UNUSED(uuid) Q_UNUSED(deviceId) Q_UNUSED(role)
    }
    virtual void setPriority(const QString &uuid, const QStringList &deviceIdsInOrder) { Q_UNUSED(uuid) Q_UNUSED(deviceIdsInOrder) }
    virtual void transferHosting(const QString &uuid, const QString &deviceId) { Q_UNUSED(uuid) Q_UNUSED(deviceId) }

    // ---- recovery drafts (uuid "" = of all projects)
    virtual QList<DraftInfo> drafts(const QString &uuid) const { Q_UNUSED(uuid) return {}; }
    virtual void compareDraft(const QString &id) { Q_UNUSED(id) }
    virtual void keepDraftAsCopy(const QString &id) { Q_UNUSED(id) }
    virtual void takeOverDraft(const QString &id) { Q_UNUSED(id) }
    virtual void deleteDraft(const QString &id) { Q_UNUSED(id) }

    // ---- an edit request of another device for a document this device holds
    virtual void handOver(const QString &uuid, const QString &resource) { Q_UNUSED(uuid) Q_UNUSED(resource) }
    virtual void decline(const QString &uuid, const QString &resource) { Q_UNUSED(uuid) Q_UNUSED(resource) }

signals:
    void enabledChanged();
    void discoveredChanged();
    void pairingWindowChanged();
    void pairingCode(const QString &otherName, const QString &code);   // show the code, ask "Codes Match / They Differ"
    void pairingFinished(bool ok, const QString &text);
    void joinProgress(const QString &uuid, int percent, const QString &text);
    void joinFinished(const QString &uuid, bool ok, const QString &folderOrError);
    void accessChanged(const QString &uuid);
    void draftsChanged();
    void editAsked(const QString &uuid, const QString &resource, const QString &askerName);
    void editAskEnded(const QString &uuid, const QString &resource);
    void foundUnknownHostRefused(const QString &projectTitle);   // "An unknown device tried to take over hosting ..."
};

// The control the interface uses: the service's once it is set, else the stand-in. Never null. The service owns its control.
LiveSyncControl &liveSyncControl();
void setLiveSyncControl(LiveSyncControl *control);   // nullptr: the stand-in again

// This program's three settings of Live Sync (File > Options > Live Sync), for the stand-in and for the service.
bool prefEnabled();
void setPrefEnabled(bool on);
QString prefDeviceName();   // the host's name when none was set
void setPrefDeviceName(const QString &name);
bool prefKeepHosting();
void setPrefKeepHosting(bool on);

// A control filled by hand, for the suites of the interface: the calls are noted, the signals are fired by the suite.
class FakeLiveSyncControl : public LiveSyncControl
{
    Q_OBJECT
public:
    explicit FakeLiveSyncControl(QObject *parent = nullptr) : LiveSyncControl(parent) {}
    collab::StatusModel *model() override { return &status; }
    collab::StatusActions *actions() override { return &status; }
    bool enabled() const override { return on; }
    void setEnabled(bool v) override
    {
        on = v;
        calls << QStringLiteral("enabled:%1").arg(v);
        emit enabledChanged();
    }
    QString unavailableReason() const override { return unavailable; }
    QString deviceName() const override { return name; }
    void setDeviceName(const QString &n) override { name = n; calls << QStringLiteral("name:") + n; }
    bool keepHosting() const override { return keep; }
    void setKeepHosting(bool v) override { keep = v; calls << QStringLiteral("keep:%1").arg(v); }
    QStringList addresses() const override { return addr; }
    int port() const override { return portNumber; }
    QString fingerprint() const override { return QStringLiteral("AB12 CD34"); }
    QString protocolText() const override { return QStringLiteral("Live Sync 1, TLS 1.3"); }
    QString roundTripText() const override { return QStringLiteral("3 ms"); }
    void removeDevice(const QString &id) override { calls << QStringLiteral("remove:") + id; }
    QList<DiscoveredDevice> discovered() const override { return found; }
    void openPairingWindow() override { calls << QStringLiteral("openwindow"); windowOpen = true; emit pairingWindowChanged(); }
    void closePairingWindow() override { calls << QStringLiteral("closewindow"); windowOpen = false; emit pairingWindowChanged(); }
    bool pairingWindowOpen() const override { return windowOpen; }
    void pairWith(const QString &key) override { calls << QStringLiteral("pair:") + key; }
    void pairWithAddress(const QString &a) override { calls << QStringLiteral("pairaddr:") + a; }
    void answerPairing(bool m) override { calls << QStringLiteral("answer:%1").arg(m); }
    void cancelPairing() override { calls << QStringLiteral("cancelpair"); }
    bool shareProject(const QString &folder, QString *error) override
    {
        calls << QStringLiteral("share:") + folder;
        if (!shareOk && error)
            *error = QStringLiteral("refused");
        return shareOk;
    }
    QList<RemoteProject> remoteProjects() const override { return remote; }
    void join(const QString &d, const QString &u) override { calls << QStringLiteral("join:%1:%2").arg(d, u); }
    QList<AccessEntry> access(const QString &uuid) const override { return accessList.value(uuid); }
    void setAccess(const QString &uuid, const QString &d, AccessEntry::Role r) override
    {
        calls << QStringLiteral("access:%1:%2:%3").arg(uuid, d).arg(int(r));
    }
    void setPriority(const QString &uuid, const QStringList &o) override { calls << QStringLiteral("priority:%1:%2").arg(uuid, o.join(QLatin1Char(','))); }
    void transferHosting(const QString &uuid, const QString &d) override { calls << QStringLiteral("transfer:%1:%2").arg(uuid, d); }
    QList<DraftInfo> drafts(const QString &uuid) const override
    {
        QList<DraftInfo> out;
        for (const DraftInfo &d : draftList)
            if (uuid.isEmpty() || d.uuid == uuid)
                out << d;
        return out;
    }
    void compareDraft(const QString &id) override { calls << QStringLiteral("compare:") + id; }
    void keepDraftAsCopy(const QString &id) override { calls << QStringLiteral("keep:") + id; }
    void takeOverDraft(const QString &id) override { calls << QStringLiteral("takedraft:") + id; }
    void deleteDraft(const QString &id) override
    {
        calls << QStringLiteral("deletedraft:") + id;
        for (int i = 0; i < draftList.size(); ++i)
            if (draftList.at(i).id == id) {
                draftList.removeAt(i);
                break;
            }
        emit draftsChanged();
    }
    void handOver(const QString &u, const QString &r) override { calls << QStringLiteral("handover:%1:%2").arg(u, r); }
    void decline(const QString &u, const QString &r) override { calls << QStringLiteral("decline:%1:%2").arg(u, r); }

    collab::FakeStatusModel status;
    bool on = false;
    QString unavailable;
    QString name = QStringLiteral("This Computer");
    bool keep = false;
    QStringList addr;
    int portNumber = 0;
    QList<DiscoveredDevice> found;
    QList<RemoteProject> remote;
    QHash<QString, QList<AccessEntry>> accessList;
    QList<DraftInfo> draftList;
    bool windowOpen = false;
    bool shareOk = true;
    QStringList calls;
};

} // namespace livesync
} // namespace qb
