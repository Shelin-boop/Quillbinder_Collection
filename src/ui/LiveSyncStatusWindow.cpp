#include "ui/LiveSyncStatusWindow.h"

#include "AppInfo.h"
#include "core/TestSwitches.h"
#include "core/collab/Service.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVBoxLayout>
#include <cstdio>

namespace qb {
namespace ui {

LiveSyncStatusWindow::LiveSyncStatusWindow(collab::StatusModel *model, QWidget *parent) : QWidget(parent), m_model(model)
{
    setObjectName(QStringLiteral("liveSyncStatusWindow"));
    setWindowTitle(tr("Live Sync"));
    auto *lay = new QVBoxLayout(this);
    m_label = new QLabel(this);
    m_label->setObjectName(QStringLiteral("liveSyncStatusLabel"));
    m_label->setAccessibleName(tr("Hosted projects"));
    lay->addWidget(m_label);
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("liveSyncStatusList"));
    m_list->setAccessibleName(tr("Projects hosted for other devices"));
    lay->addWidget(m_list);
    auto *open = new QPushButton(tr("Open Project"), this);
    open->setObjectName(QStringLiteral("liveSyncOpenProject"));
    connect(open, &QPushButton::clicked, this, [this] {
        const int row = m_list->currentRow();
        const QStringList folders = hostedFolders();
        if (row >= 0 && row < folders.size())
            emit openProjectRequested(folders.at(row));
        else if (folders.size() == 1)
            emit openProjectRequested(folders.first());
    });
    lay->addWidget(open);
    auto *quit = new QPushButton(tr("Stop Hosting and Quit"), this);
    quit->setObjectName(QStringLiteral("liveSyncStopAndQuit"));
    connect(quit, &QPushButton::clicked, this, &LiveSyncStatusWindow::stopAndQuitRequested);
    lay->addWidget(quit);
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_tray = new QSystemTrayIcon(windowIcon(), this);
        m_tray->setToolTip(tr("Live Sync"));
        connect(m_tray, &QSystemTrayIcon::activated, this, [this] {
            show();
            raise();
        });
        m_tray->show();
    }
    connect(model, &collab::StatusModel::projectsChanged, this, &LiveSyncStatusWindow::refresh);
    connect(model, &collab::StatusModel::projectChanged, this, &LiveSyncStatusWindow::refresh);
    refresh();
}

int LiveSyncStatusWindow::hostedCount() const { return hostedFolders().size(); }

QStringList LiveSyncStatusWindow::hostedFolders() const
{
    QStringList l;
    for (const collab::ProjectStatus &p : m_model->projects())
        if (p.thisDeviceHosts())
            l << p.folder;
    return l;
}

QString LiveSyncStatusWindow::summary() const { return tr("Hosting %n project(s) for other devices", nullptr, hostedCount()); }

void LiveSyncStatusWindow::refresh()
{
    m_label->setText(summary());
    m_list->clear();
    for (const collab::ProjectStatus &p : m_model->projects())
        if (p.thisDeviceHosts()) {
            int others = 0;
            for (const collab::ParticipantStatus &x : p.participants)
                others += !x.thisDevice && x.online;
            m_list->addItem(tr("%1 (%n device(s) connected)", nullptr, others).arg(p.title));
        }
}

// ---------------------------------------------------------------- the service process

int runLiveSyncService(int argc, char **argv)
{
    QCoreApplication::setOrganizationName(QStringLiteral(QB_APP_NAME));
    QCoreApplication::setApplicationName(QStringLiteral(QB_APP_NAME));
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    collab::Service::Config cfg;
    cfg.quitWhenIdle = true;
    // the suites' switches (test builds only): a port of their own, a short wait before the service ends
    int idleMs = 10000;
    if (QB_TEST_ENV_SET("QB_LS_PORT"))
        cfg.port = quint16(QB_TEST_ENV("QB_LS_PORT").toUInt());
    if (QB_TEST_ENV_SET("QB_LS_IDLE_MS"))
        idleMs = QB_TEST_ENV("QB_LS_IDLE_MS").toInt();
    if (QB_TEST_ENV_SET("QB_LS_NO_DISCOVERY"))
        cfg.discovery = false;
    cfg.idleMs = idleMs;
    collab::Service service(cfg);
    if (QB_TEST_ENV_SET("QB_LS_PEERS")) // "<device id>=<address>:<port>;..." where discovery is off (the suites)
        for (const QString &part : QB_TEST_ENV("QB_LS_PEERS").split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
            const QString id = part.section(QLatin1Char('='), 0, 0), where = part.section(QLatin1Char('='), 1);
            service.setPeerAddress(id, where.section(QLatin1Char(':'), 0, 0), quint16(where.section(QLatin1Char(':'), 1).toUInt()));
        }
    QString error;
    if (!service.start(&error)) {
        // another service of this device is already running: nothing to do; any other reason is told
        if (!error.contains(QLatin1String("already")))
            std::fprintf(stderr, "Live Sync service: %s\n", qPrintable(error));
        return error.contains(QLatin1String("already")) ? 0 : 2;
    }
    QObject::connect(&service, &collab::Service::idle, &app, &QCoreApplication::quit);
    QObject::connect(&service, &collab::Service::quitRequested, &app, &QCoreApplication::quit);
    LiveSyncStatusWindow window(service.statusModel());
    QObject::connect(&window, &LiveSyncStatusWindow::stopAndQuitRequested, &app, &QCoreApplication::quit);
    QObject::connect(&window, &LiveSyncStatusWindow::openProjectRequested, &app, [](const QString &folder) {
        QProcess::startDetached(QCoreApplication::applicationFilePath(), {folder});
    });
    // the window shows only while the service keeps hosting with no window of the program open
    QTimer show;
    show.setInterval(1000);
    QObject::connect(&show, &QTimer::timeout, &window, [&] {
        const bool want = service.keepHosting() && service.windowCount() == 0 && service.hostedCount() > 0;
        if (want != window.isVisible())
            window.setVisible(want);
    });
    show.start();
    return app.exec();
}

} // namespace ui
} // namespace qb
