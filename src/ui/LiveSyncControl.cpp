#include "ui/LiveSyncControl.h"

#include <QSettings>
#include <QSysInfo>

namespace qb {
namespace livesync {

namespace {

QSettings &prefs()
{
    thread_local QSettings s;
    s.sync();
    return s;
}

const QString kGroup = QStringLiteral("livesync/");

// the stand-in: the three settings, nothing else (no service in the program)
class StandIn : public LiveSyncControl
{
public:
    StandIn() = default;
    collab::StatusModel *model() override { return &m_status; }
    collab::StatusActions *actions() override { return &m_status; }
    bool enabled() const override { return prefEnabled(); }
    void setEnabled(bool on) override
    {
        setPrefEnabled(on);
        m_status.setRunning(on);
        emit enabledChanged();
    }
    QString unavailableReason() const override { return QString(); }
    QString deviceName() const override { return prefDeviceName(); }
    void setDeviceName(const QString &name) override { setPrefDeviceName(name); }
    bool keepHosting() const override { return prefKeepHosting(); }
    void setKeepHosting(bool on) override { setPrefKeepHosting(on); }

private:
    collab::FakeStatusModel m_status; // empty: nothing is shared without a service
};

LiveSyncControl *g_control = nullptr;

} // namespace

bool prefEnabled() { return prefs().value(kGroup + QStringLiteral("enabled"), false).toBool(); }
void setPrefEnabled(bool on)
{
    QSettings &s = prefs();
    s.setValue(kGroup + QStringLiteral("enabled"), on);
    s.sync();
}
QString prefDeviceName()
{
    const QString n = prefs().value(kGroup + QStringLiteral("deviceName")).toString().trimmed();
    return n.isEmpty() ? QSysInfo::machineHostName() : n;
}
void setPrefDeviceName(const QString &name)
{
    QSettings &s = prefs();
    s.setValue(kGroup + QStringLiteral("deviceName"), name.trimmed());
    s.sync();
}
bool prefKeepHosting() { return prefs().value(kGroup + QStringLiteral("keepHosting"), false).toBool(); }
void setPrefKeepHosting(bool on)
{
    QSettings &s = prefs();
    s.setValue(kGroup + QStringLiteral("keepHosting"), on);
    s.sync();
}

LiveSyncControl &liveSyncControl()
{
    if (g_control)
        return *g_control;
    static StandIn standIn;
    return standIn;
}

bool hasLiveSyncControl() { return g_control != nullptr; }

void setLiveSyncControl(LiveSyncControl *control) { g_control = control; }

} // namespace livesync
} // namespace qb
