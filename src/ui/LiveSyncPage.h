#pragma once
// File > Options > Live Sync (0.99.0): the switch, this device's name, the hosting option, the paired devices, and two
// folded parts (More Ways to Connect, Details). Everything it shows comes from the program's LiveSyncControl; a change
// is carried out at once (switching on starts the service), so Pair a Device works without closing the window.
#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QTreeWidget;

namespace qb {
namespace livesync {

class LiveSyncPage : public QWidget
{
    Q_OBJECT
public:
    explicit LiveSyncPage(QWidget *parent = nullptr);
    void refresh();

private:
    QWidget *folded(const QString &title, const char *name, QWidget *body);
    void refreshDevices();
    QCheckBox *m_on = nullptr;
    QLabel *m_reason = nullptr;
    QLineEdit *m_name = nullptr;
    QCheckBox *m_keep = nullptr;
    QLabel *m_keys = nullptr;
    QTreeWidget *m_devices = nullptr;
    QPushButton *m_pair = nullptr;
    QPushButton *m_remove = nullptr;
    QLabel *m_addresses = nullptr;
    QPushButton *m_byAddress = nullptr;
    QLabel *m_details = nullptr;
    bool m_loading = false;
};

} // namespace livesync
} // namespace qb
