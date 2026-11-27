// The window of the full AppImage's add-on installation; see FullInstallWindow.h.
#include "ui/FullInstallWindow.h"
#include "AppInfo.h"
#include "core/Addons.h"
#include "text/TextUtil.h"
#include "ui/Language.h"

#include <QApplication>
#include <QCoreApplication>
#include <QEventLoop>
#include <QIcon>
#include <QLabel>
#include <QMetaObject>
#include <QPixmap>
#include <QProgressBar>
#include <QScreen>
#include <QThread>
#include <QVBoxLayout>

#include <cstdio>

namespace qb {

QString FullInstallWindow::message(int count, qint64 bytes)
{
    const qint64 mb = qMax<qint64>(1, (bytes + 500000) / 1000000);
    return tr("Installing %Ln add-on(s) (%1 MB)...", "", count).arg(mb);
}

FullInstallWindow::FullInstallWindow(int count, qint64 bytes, QWidget *parent) : QWidget(parent, Qt::Window | Qt::WindowTitleHint)
{
    setObjectName(QStringLiteral("fullInstallWindow"));
    setWindowTitle(QStringLiteral(QB_APP_NAME));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(40, 32, 40, 32);
    v->setSpacing(14);
    m_logo = new QLabel(this);
    m_logo->setObjectName(QStringLiteral("fullInstallLogo"));
    m_logo->setAlignment(Qt::AlignCenter);
    m_logo->setPixmap(QIcon(QStringLiteral(":/app/quillbinder.svg")).pixmap(192, 192));
    v->addWidget(m_logo);
    auto *name = new QLabel(QStringLiteral(QB_APP_NAME), this);
    QFont f = name->font();
    f.setPointSizeF(f.pointSizeF() * 1.8);
    f.setBold(true);
    name->setFont(f);
    name->setAlignment(Qt::AlignCenter);
    v->addWidget(name);
    m_message = new QLabel(message(count, bytes), this);
    m_message->setObjectName(QStringLiteral("fullInstallMessage"));
    m_message->setAlignment(Qt::AlignCenter);
    v->addWidget(m_message);
    m_bar = new QProgressBar(this);
    m_bar->setObjectName(QStringLiteral("fullInstallBar"));
    m_bar->setRange(0, int(qMax<qint64>(1, bytes / 1000))); // in kB
    m_bar->setValue(0);
    m_bar->setTextVisible(false);
    v->addWidget(m_bar);
    m_current = new QLabel(QLatin1String(" "), this);
    m_current->setObjectName(QStringLiteral("fullInstallCurrent"));
    m_current->setAlignment(Qt::AlignCenter);
    v->addWidget(m_current);
    setFixedWidth(460);
    adjustSize();
}

void FullInstallWindow::setProgress(int done, int total, const QString &name, qint64 doneBytes)
{
    m_bar->setValue(int(qMin<qint64>(m_bar->maximum(), doneBytes / 1000)));
    QString n = name; // (the add-on's name from its manifest, 0.94.0; before, the package's file name)
    m_current->setText(n.isEmpty() ? tr("Done") : tr("%1 of %2: %3").arg(done + 1).arg(total).arg(n));
}

int FullInstallWindow::progressValue() const { return m_bar->value(); }

int FullInstallWindow::progressMaximum() const { return m_bar->maximum(); }

QString FullInstallWindow::currentText() const { return m_current->text(); }

int runInstallFolder(int &argc, char **argv, const QString &folder)
{
    bool display = true;
#ifdef Q_OS_LINUX
    display = qEnvironmentVariableIsSet("DISPLAY") || qEnvironmentVariableIsSet("WAYLAND_DISPLAY");
#endif
    if (qEnvironmentVariableIsSet("QB_NO_DIALOGS"))
        display = false;
    QCoreApplication *app = display ? new QApplication(argc, argv) : new QCoreApplication(argc, argv);
    QStringList log;
    QString error;
    int count = 0;
    qint64 bytes = 0;
    bool ok = true;
    if (addons::installBundledDue(folder, &count, &bytes)) {
        if (display) {
            language::install();
            FullInstallWindow win(count, bytes);
            win.show();
            QEventLoop loop;
            // the installation runs beside the window, so that it can paint and move
            QThread *worker = QThread::create([&] {
                ok = addons::installBundled(folder, &log, &error,
                                            [&win](int done, int total, const QString &name, qint64 doneBytes) {
                                                QMetaObject::invokeMethod(
                                                    &win, [&win, done, total, name, doneBytes] { win.setProgress(done, total, name, doneBytes); },
                                                    Qt::QueuedConnection);
                                            });
            });
            QObject::connect(worker, &QThread::finished, &loop, &QEventLoop::quit);
            worker->start();
            loop.exec();
            worker->wait();
            delete worker;
            QCoreApplication::processEvents();
        } else {
            ok = addons::installBundled(folder, &log, &error);
        }
    }
    for (const QString &l : log)
        std::printf("%s\n", qPrintable(l));
    if (!ok)
        std::fprintf(stderr, "%s\n", qPrintable(error));
    delete app;
    return ok ? 0 : 1;
}

} // namespace qb
