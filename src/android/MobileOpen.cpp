#include "android/Mobile.h"


#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QJniEnvironment>
#include <QJniObject>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QUrl>
#include <QtCore/qcoreapplication_platform.h>

// Files and folders handed to other apps (Android only, 2026-11-14).
//
// The program opens a file or a folder "in the system" in many places (Show Project in File Manager, Show Backup
// Folder, a compiled file, a link to a file): on a desktop that is the file manager or the program for that kind of
// file. Qt for Android does it for files (it hands the file to Android through the app's file provider, with the
// kind of file told by its name), but a folder goes nowhere: Android has no "open this folder" for an app's own
// path. Every file address the program opens comes through here first: a folder of the tablet's shared storage is
// shown in the tablet's Files app, a file goes on to Qt, and what cannot be opened is told with its place.

namespace qb {
namespace mobile {

namespace {

Q_LOGGING_CATEGORY(lcOpen, "qb.open", QtInfoMsg)

// the tablet's shared storage ("/storage/emulated/0")
QString sharedRoot()
{
    const QJniObject root = QJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory",
                                                               "()Ljava/io/File;");
    return root.isValid() ? root.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;").toString() : QString();
}

// "/storage/emulated/0/Documents/Quillbinder/Backups" as a writer finds it in the Files app
QString shownPlace(const QString &path)
{
    const QString root = sharedRoot();
    return !root.isEmpty() && path.startsWith(root + QLatin1Char('/')) ? path.mid(root.size() + 1) : path;
}

// Asks Android to show `address` (a content address) as `type`. True when an app took it.
bool startView(const QString &address, const QString &type)
{
    QJniEnvironment env;
    const QJniObject context(QNativeInterface::QAndroidApplication::context());
    if (!context.isValid())
        return false;
    const QJniObject uri = QJniObject::callStaticObjectMethod("android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;",
                                                              QJniObject::fromString(address).object());
    const QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V",
                            QJniObject::fromString(QStringLiteral("android.intent.action.VIEW")).object());
    if (!uri.isValid() || !intent.isValid())
        return false;
    intent.callObjectMethod("setDataAndType", "(Landroid/net/Uri;Ljava/lang/String;)Landroid/content/Intent;", uri.object(),
                            QJniObject::fromString(type).object());
    // FLAG_ACTIVITY_NEW_TASK | FLAG_GRANT_READ_URI_PERMISSION
    intent.callObjectMethod("addFlags", "(I)Landroid/content/Intent;", jint(0x10000000 | 0x00000001));
    // by hand, to see whether Android found an app for it (QJniObject's calls swallow the refusal)
    jclass cls = env->GetObjectClass(context.object());
    const jmethodID start = env.findMethod(cls, "startActivity", "(Landroid/content/Intent;)V");
    env->DeleteLocalRef(cls);
    if (!start)
        return false;
    env->CallVoidMethod(context.object(), start, intent.object());
    return !env.checkAndClearExceptions();
}

// The tablet's Files app at a folder of the shared storage. Not for the app's own folders (Android/data/...):
// the Files app does not show those.
bool showInFilesApp(const QString &folder)
{
    const QString root = sharedRoot();
    if (root.isEmpty() || !folder.startsWith(root + QLatin1Char('/')))
        return false;
    const QString inside = folder.mid(root.size() + 1);
    if (inside.startsWith(QLatin1String("Android/")))
        return false;
    // Android 0.93.2: the Files app of Android 9 takes no folder's address (it ended itself: "Invalid URI" in
    // DocumentsContract.getRootId, the start check's tablet with Android 9); before Android 10 it is shown at the
    // shared storage's top instead
    if (QNativeInterface::QAndroidApplication::sdkVersion() < 29)
        return startView(QStringLiteral("content://com.android.externalstorage.documents/root/primary"),
                         QStringLiteral("vnd.android.document/root"));
    const QString address = QStringLiteral("content://com.android.externalstorage.documents/document/primary%3A")
                            + QString::fromLatin1(QUrl::toPercentEncoding(inside));
    return startView(address, QStringLiteral("vnd.android.document/directory"));
}

class Opener : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

public slots:
    void open(const QUrl &url)
    {
        const QFileInfo info(url.toLocalFile());
        const QString path = info.absoluteFilePath();
        if (info.isDir()) {
            const bool shown = showInFilesApp(path);
            qCInfo(lcOpen, "open: the folder %s %s", qPrintable(path), shown ? "is shown by the tablet's Files app" : "COULD NOT BE SHOWN");
            if (!shown)
                QMessageBox::information(QApplication::activeWindow(), QCoreApplication::translate("Mobile", "Folder"),
                                         QCoreApplication::translate("Mobile", "The tablet's Files app cannot show this folder from here. It is:\n\n%1")
                                             .arg(shownPlace(path)));
            return;
        }
        // a file: Qt's own way (inside this handler the call goes on to Android, not back here)
        const bool opened = QDesktopServices::openUrl(url);
        qCInfo(lcOpen, "open: the file %s %s", qPrintable(path), opened ? "was handed to Android" : "COULD NOT BE OPENED");
        if (!opened)
            QMessageBox::information(QApplication::activeWindow(), QCoreApplication::translate("Mobile", "File"),
                                     QCoreApplication::translate("Mobile", "No app on this tablet opened this file. It is:\n\n%1")
                                         .arg(shownPlace(path)));
    }
};

} // namespace

void installOpener()
{
    static Opener *opener = nullptr;
    if (opener)
        return;
    opener = new Opener(qApp);
    opener->setObjectName(QStringLiteral("mobileOpener"));
    QDesktopServices::setUrlHandler(QStringLiteral("file"), opener, "open");
}

} // namespace mobile
} // namespace qb

#include "MobileOpen.moc"
