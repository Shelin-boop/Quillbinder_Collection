// The entry in the application menu; see AppMenuEntry.h.
#include "ui/AppMenuEntry.h"
#include "AppInfo.h"
#include "core/TestSwitches.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

namespace qb {
namespace appmenu {

static QString dataRoot() { return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation); }

QString appImagePath()
{
    const QString p = qEnvironmentVariable("APPIMAGE");
    return p.isEmpty() ? QString() : QFileInfo(p).absoluteFilePath();
}

bool runsFromAppImage()
{
#ifdef Q_OS_LINUX
    const QString p = appImagePath();
    return !p.isEmpty() && QFileInfo(p).isFile();
#else
    return false;
#endif
}

QString applicationsFolder() { return QDir::cleanPath(QDir::homePath() + QStringLiteral("/Applications")); }

bool isInApplications(const QString &file)
{
    return QFileInfo(file).absoluteDir().canonicalPath() == QFileInfo(applicationsFolder()).canonicalFilePath()
           && QFileInfo(applicationsFolder()).isDir();
}

bool canMove() { return runsFromAppImage() && !isInApplications(appImagePath()); }

QString desktopFilePath() { return dataRoot() + QLatin1String("/applications/" QB_APP_ID ".desktop"); }

bool hasEntry() { return QFileInfo::exists(desktopFilePath()); }

// the settings that name the AppImage's place
static void renameInSettings(const QString &from, const QString &to)
{
    QSettings s;
    for (const char *key : {"app/integrated", "app/integrationDeclined"})
        if (s.value(QLatin1String(key)).toString() == from)
            s.setValue(QLatin1String(key), to);
}

bool moveToApplications(QString *newPath, QString *error)
{
    auto fail = [error](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    const QString from = appImagePath();
    if (from.isEmpty() || !QFileInfo(from).isFile())
        return fail(QCoreApplication::translate("qb::appmenu", "The program does not run from an AppImage file."));
    const QString dir = applicationsFolder();
    const QString to = dir + QLatin1Char('/') + QFileInfo(from).fileName();
    if (QFileInfo(from).absoluteFilePath() == to) {
        if (newPath)
            *newPath = to;
        return true;
    }
    if (!QDir().mkpath(dir))
        return fail(QCoreApplication::translate("qb::appmenu", "The folder %1 could not be made.").arg(QDir::toNativeSeparators(dir)));
    if (QFileInfo::exists(to))
        return fail(QCoreApplication::translate("qb::appmenu", "There is a file with this name in %1 already.")
                        .arg(QDir::toNativeSeparators(dir)));
    bool moved = QFile::rename(from, to); // the same file system: one step
    if (!moved) {
        // another file system: a copy, checked, and then the old file goes
        if (!QFile::copy(from, to)) {
            QFile::remove(to);
            return fail(QCoreApplication::translate("qb::appmenu", "The file could not be copied to %1.").arg(QDir::toNativeSeparators(dir)));
        }
        if (QFileInfo(to).size() != QFileInfo(from).size()) {
            QFile::remove(to);
            return fail(QCoreApplication::translate("qb::appmenu", "The copy in %1 is not complete.").arg(QDir::toNativeSeparators(dir)));
        }
        if (!QFile::remove(from)) {
            QFile::remove(to); // the old file stays the one in use
            return fail(QCoreApplication::translate("qb::appmenu", "The old file could not be removed after the copy."));
        }
    }
    // everything that reads $APPIMAGE later in this session (program updates, the ".before-update" copy, System
    // Information) now finds the file in its new place
    qputenv("APPIMAGE", to.toUtf8());
    if (QB_TEST_ENV("QB_TEST_APPIMAGE") == from)
        qputenv("QB_TEST_APPIMAGE", to.toUtf8());
    renameInSettings(from, to);
    if (newPath)
        *newPath = to;
    return true;
}

Result add(bool moveAppImage, const QList<ToolAction> &tools)
{
    Result r;
    if (moveAppImage && canMove()) {
        QString np;
        if (moveToApplications(&np, &r.moveError))
            r.moved = true;
    }
    const QString appimage = appImagePath();
    const QString exec = appimage.isEmpty() ? QCoreApplication::applicationFilePath() : appimage;
    r.exec = exec;
    const QString data = dataRoot();
    QDir().mkpath(data + QLatin1String("/applications"));
    QDir().mkpath(data + QLatin1String("/icons/hicolor/scalable/apps"));
    QDir().mkpath(data + QLatin1String("/mime/packages"));
    const QString icon = data + QLatin1String("/icons/hicolor/scalable/apps/" QB_APP_ID ".svg");
    QFile::remove(icon);
    // a failed copy is told by the caller (the menu entry works without the icon)
    r.iconCopied = QFile::copy(QStringLiteral(":/app/quillbinder.svg"), icon);
    QFile::setPermissions(icon, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadOther | QFileDevice::ReadGroup);
    QStringList actionIds;
    QString actionSections;
    for (const ToolAction &t : tools) {
        actionIds << t.id;
        actionSections += QStringLiteral("\n[Desktop Action %1]\nName=%2\nExec=\"%3\" --tool %4\n").arg(t.id, t.name, exec, t.key);
    }
    QFile desktop(desktopFilePath());
    if (!desktop.open(QIODevice::WriteOnly)) {
        r.error = QCoreApplication::translate("qb::appmenu", "The file %1 could not be written.")
                      .arg(QDir::toNativeSeparators(desktop.fileName()));
        return r;
    }
    desktop.write(QStringLiteral("[Desktop Entry]\nType=Application\nName=%1\nGenericName=Writing Studio\n"
                                 "Comment=Write, organize and compile long texts\nExec=\"%2\" %f\nIcon=%3\n"
                                 "Categories=Office;WordProcessor;\nMimeType=application/x-%3-project;application/x-%3-package;\nTerminal=false\n"
                                 "StartupWMClass=%3\n")
                      .arg(QLatin1String(QB_APP_NAME), exec, QLatin1String(QB_APP_ID))
                      .toUtf8());
    if (!actionIds.isEmpty())
        desktop.write((QStringLiteral("Actions=") + actionIds.join(QLatin1Char(';')) + QStringLiteral(";\n") + actionSections).toUtf8());
    desktop.close();
    QFile mime(data + QLatin1String("/mime/packages/" QB_APP_ID ".xml"));
    if (mime.open(QIODevice::WriteOnly)) {
        mime.write(QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                  "<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n"
                                  "  <mime-type type=\"application/x-%1-project\">\n"
                                  "    <comment>%2 project</comment>\n"
                                  "    <icon name=\"%1\"/>\n"
                                  "    <glob pattern=\"*%3\"/>\n"
                                  "  </mime-type>\n"
                                  "  <mime-type type=\"application/x-%1-package\">\n"
                                  "    <comment>%2 update or add-on</comment>\n"
                                  "    <icon name=\"%1\"/>\n"
                                  "    <glob pattern=\"*.qbpkg\"/>\n"
                                  "  </mime-type>\n</mime-info>\n")
                       .arg(QLatin1String(QB_APP_ID), QLatin1String(QB_APP_NAME), QLatin1String(QB_PROJECT_FILE_SUFFIX))
                       .toUtf8());
        mime.close();
    }
    QProcess::startDetached(QStringLiteral("update-mime-database"), {data + QLatin1String("/mime")});
    QProcess::startDetached(QStringLiteral("update-desktop-database"), {data + QLatin1String("/applications")});
    QProcess::startDetached(QStringLiteral("kbuildsycoca5"), {});
    QProcess::startDetached(QStringLiteral("kbuildsycoca6"), {});
    QSettings().setValue(QStringLiteral("app/integrated"), exec);
    r.ok = true;
    return r;
}

bool remove(QString *error)
{
    const QString data = dataRoot();
    QStringList failed;
    for (const QString &f : {desktopFilePath(), data + QLatin1String("/icons/hicolor/scalable/apps/" QB_APP_ID ".svg"),
                             data + QLatin1String("/mime/packages/" QB_APP_ID ".xml")})
        if (QFileInfo::exists(f) && !QFile::remove(f))
            failed << QDir::toNativeSeparators(f);
    if (!failed.isEmpty()) {
        if (error)
            *error = QCoreApplication::translate("qb::appmenu", "These files could not be removed:\n%1").arg(failed.join(QLatin1Char('\n')));
        return false;
    }
    QProcess::startDetached(QStringLiteral("update-mime-database"), {data + QLatin1String("/mime")});
    QProcess::startDetached(QStringLiteral("update-desktop-database"), {data + QLatin1String("/applications")});
    QProcess::startDetached(QStringLiteral("kbuildsycoca5"), {});
    QProcess::startDetached(QStringLiteral("kbuildsycoca6"), {});
    QSettings s;
    s.remove(QStringLiteral("app/integrated"));
    // taken away on purpose: the program does not ask again at its start
    const QString appimage = appImagePath();
    if (!appimage.isEmpty())
        s.setValue(QStringLiteral("app/integrationDeclined"), appimage);
    return true;
}

} // namespace appmenu
} // namespace qb
