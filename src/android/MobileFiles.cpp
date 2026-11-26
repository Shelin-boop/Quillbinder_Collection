#include "android/Mobile.h"

#include "core/Backup.h"
#include "core/Paths.h"
#include "ui/AppSettings.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLoggingCategory>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QJniObject>
#include <QtCore/qcoreapplication_platform.h>
#endif

// Where the writer's work is kept on a tablet (2026-11-14: projects and backups are saved in Downloads or
// Documents).
//
// Qt names the app's own folders as "Documents" on Android 11 and later (.../Android/data/<app id>/files/Documents):
// no other app shows them, and Android deletes them with the app. The folder everybody means is the tablet's shared
// Documents folder. An app reaches it as plain files only with Android's "All files access", which the writer
// switches on once on a page of Android's settings (Android 9 and 10: the storage permission, asked for by a window
// of Android's own).
//
// This file: the rule which of the program's folder settings are moved there (settleFolders: only settings that
// are empty or still name one of the app's own folders; a folder the writer chose stays), the window that explains
// it and sends the writer to Android's settings, file windows that open in the shared folder and list it at their
// side, and the entry File > Files on This Tablet. Nothing of it is in a shared file: the program's own settings
// are set from outside, before the main window reads them.

namespace qb {
namespace mobile {

namespace {

Q_LOGGING_CATEGORY(lcFiles, "qb.files", QtInfoMsg)

QString clean(const QString &path) { return path.isEmpty() ? QString() : QDir::cleanPath(path); }

bool insideOne(const QString &path, const QStringList &roots)
{
    const QString p = clean(path);
    for (const QString &r : roots) {
        const QString root = clean(r);
        if (!root.isEmpty() && (p == root || p.startsWith(root + QLatin1Char('/'))))
            return true;
    }
    return false;
}

} // namespace

Folders foldersIn(const QString &documents)
{
    const QString base = clean(documents) + QLatin1String("/" QB_APP_NAME);
    return {base, base + QStringLiteral("/Backups"), base + QStringLiteral("/Exports"), base + QStringLiteral("/Scratch Pad")};
}

bool settleFolders(const Folders &f, const QStringList &own, bool access)
{
    // with access: a setting that is empty or names one of the app's own folders goes to the shared folder;
    // without: a setting that names the shared folder goes back to the program's standard (the app's own folder)
    const QStringList shared{f.projects};
    bool changed = false;
    const auto settle = [&](QString now, const QString &want, const QString &standard) {
        const QString before = now;
        if (access) {
            if (now.isEmpty() || insideOne(now, own))
                now = want;
        } else if (insideOne(now, shared)) {
            now = standard;
        }
        changed = changed || now != before;
        return now;
    };
    AppSettings &s = AppSettings::instance();
    s.lastProjectDir = settle(s.lastProjectDir, f.projects, QString());
    s.welcomeFolder = settle(s.welcomeFolder, f.projects, QString());
    s.backupDir = settle(s.backupDir, f.backups, backup::defaultDirectory());
    QSettings st;
    const QList<std::pair<QString, QString>> more{{QStringLiteral("export/location"), f.exports},
                                                  {QStringLiteral("tools/scratch-pad/folder"), f.notes}};
    for (const auto &[key, want] : more) {
        const QString now = st.value(key).toString();
        const QString then = settle(now, want, QString());
        if (then != now) {
            if (then.isEmpty())
                st.remove(key);
            else
                st.setValue(key, then);
        }
    }
    if (access) {
        QDir().mkpath(f.projects);
        QDir().mkpath(f.backups);
    }
    if (changed)
        s.save();
    return changed;
}

// ---- the window

namespace {

class FilesDialog : public QDialog
{
public:
    FilesDialog(QWidget *parent, const FilesSystem &system)
        : QDialog(parent)
        , m_system(system)
    {
        setObjectName(QStringLiteral("filesOnTablet"));
        setWindowTitle(QCoreApplication::translate("Mobile", "Where Your Work Is Kept"));
        auto *box = new QVBoxLayout(this);
        box->setContentsMargins(20, 18, 20, 16);
        box->setSpacing(14);
        m_text = new QLabel(this);
        m_text->setObjectName(QStringLiteral("filesText"));
        m_text->setWordWrap(true);
        m_text->setTextFormat(Qt::RichText);
        box->addWidget(m_text);
        m_state = new QLabel(this);
        m_state->setObjectName(QStringLiteral("filesState"));
        m_state->setWordWrap(true);
        box->addWidget(m_state);
        m_allow = new QPushButton(this);
        m_allow->setObjectName(QStringLiteral("filesAllow"));
        m_allow->setDefault(true);
        m_keep = new QPushButton(this);
        m_keep->setObjectName(QStringLiteral("filesKeepInApp"));
        for (QPushButton *b : {m_allow, m_keep}) {
            b->setMinimumHeight(44); // (a finger's size)
            box->addWidget(b);
        }
        connect(m_allow, &QPushButton::clicked, this, [this] {
            if (m_system.allowed && m_system.allowed()) {
                accept();
                return;
            }
            m_asked = true;
            if (m_system.ask)
                m_system.ask();
            // Android's own question (Android 9, 10) lies over the app without sending it away: look again soon
            QTimer::singleShot(600, this, [this] { refresh(); });
        });
        connect(m_keep, &QPushButton::clicked, this, &QDialog::reject);
        // back from Android's settings: is the access there now?
        connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
            if (state == Qt::ApplicationActive)
                QTimer::singleShot(150, this, [this] { refresh(); });
        });
        m_look.setInterval(1000);
        connect(&m_look, &QTimer::timeout, this, [this] {
            refresh();
            // (seen on the emulated tablet, run 19: the window, shown right after the main window, did not get the
            // keys of a keyboard; a finger's tap reaches it either way)
            if (isVisible() && !isActiveWindow() && QGuiApplication::applicationState() == Qt::ApplicationActive
                && !QApplication::activePopupWidget()) {
                const QWindow *has = QGuiApplication::focusWindow();
                if (!m_toldFocus)
                    qCInfo(lcFiles, "files: the question's window did not have the keyboard's keys (they went to %s); asked for them",
                           has ? qPrintable(has->objectName()) : "no window");
                m_toldFocus = true;
                raise();
                activateWindow();
            }
        });
        m_look.start();
        m_allowedAtOpen = m_system.allowed && m_system.allowed();
        const int room = parent && parent->screen() ? parent->screen()->availableGeometry().width() : 800;
        setFixedWidth(qBound(320, room - 40, 600));
        refresh();
    }

private:
    void refresh()
    {
        const bool allowed = m_system.allowed && m_system.allowed();
        if (allowed && (m_asked || !m_allowedAtOpen)) { // the access came while the window was open: done
            accept();
            return;
        }
        const int state = (allowed ? 2 : 0) + (m_asked ? 1 : 0);
        if (state == m_state_shown)
            return;
        m_state_shown = state;
        const Folders f = foldersIn(m_system.documents);
        const QString where = QStringLiteral("<p style=\"margin-left:18px\"><b>%1</b><br><b>%2</b></p>")
                                  .arg(shown(f.projects).toHtmlEscaped(), shown(f.backups).toHtmlEscaped());
        QString text = QCoreApplication::translate("Mobile", "<p>Quillbinder keeps your projects and their backups in this tablet's Documents folder:</p>")
                       + where
                       + QCoreApplication::translate("Mobile", "<p>There they stay when the app is removed, and the tablet's Files app, other apps and a "
                             "computer can reach them.</p>");
        if (allowed) {
            text += QCoreApplication::translate("Mobile", "<p>Android allows Quillbinder to use this folder.</p>");
        } else if (m_system.settingsPage) {
            text += QCoreApplication::translate("Mobile", "<p>For this, Android has to allow Quillbinder access to all files. Tap the first button, switch "
                        "on <b>Allow access to manage all files</b> on the page that opens, and come back.</p>");
        } else {
            text += QCoreApplication::translate("Mobile", "<p>For this, Android has to allow Quillbinder access to files. Tap the first button and "
                        "answer Android's question with <b>Allow</b>.</p>");
        }
        if (!allowed)
            text += QCoreApplication::translate("Mobile", "<p>Without it, everything stays inside the app: no other app shows these files, and Android "
                        "deletes them when the app is removed.</p>");
        m_text->setText(text);
        m_state->setVisible(m_asked && !allowed);
        m_state->setText(QCoreApplication::translate("Mobile", "Android has not allowed the access yet."));
        m_allow->setText(allowed ? QCoreApplication::translate("Mobile", "OK")
                                 : (m_system.settingsPage ? QCoreApplication::translate("Mobile", "Allow in Android's Settings")
                                                          : QCoreApplication::translate("Mobile", "Allow")));
        m_keep->setText(QCoreApplication::translate("Mobile", "Keep Everything Inside the App"));
        m_keep->setVisible(!allowed);
        adjustSize();
    }

    // "/storage/emulated/0/Documents/Quillbinder" as a writer knows it: "Documents/Quillbinder"
    QString shown(const QString &path) const
    {
        const QString docs = clean(m_system.documents);
        const int cut = docs.lastIndexOf(QLatin1Char('/'));
        return cut >= 0 && path.startsWith(docs) ? path.mid(cut + 1) : path;
    }

    FilesSystem m_system;
    QLabel *m_text = nullptr;
    QLabel *m_state = nullptr;
    QPushButton *m_allow = nullptr;
    QPushButton *m_keep = nullptr;
    QTimer m_look;
    bool m_asked = false;
    bool m_allowedAtOpen = false;
    bool m_toldFocus = false;
    int m_state_shown = -1;
};

// ---- file windows

// Qt's own file window (Android's file picker gives addresses the program cannot use as folders): the shared
// folders at its side, and when it would open in one of the app's own folders only because nothing else was
// known, it opens in the writer's projects folder.
class FileWindows : public QObject
{
public:
    FileWindows(QObject *parent, const Folders &folders, const QStringList &side, const QStringList &standards)
        : QObject(parent)
        , m_folders(folders)
        , m_side(side)
        , m_standards(standards)
    {
    }

    void setFolders(const Folders &folders, const QStringList &side, const QStringList &standards)
    {
        m_folders = folders;
        m_side = side;
        m_standards = standards;
    }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() != QEvent::Show)
            return false;
        auto *dialog = qobject_cast<QFileDialog *>(o);
        if (!dialog || dialog->property("qbTabletPlaces").toBool())
            return false;
        dialog->setProperty("qbTabletPlaces", true);
        QList<QUrl> urls;
        for (const QString &dir : m_side) {
            if (QFileInfo(dir).isDir())
                urls << QUrl::fromLocalFile(dir);
        }
        if (!urls.isEmpty())
            dialog->setSidebarUrls(urls);
        // packages (Help > Add-ons and Updates > Install Update or Add-on, 0.91.3) come from the browser: the Download folder
        if (dialog->nameFilters().join(QLatin1Char(' ')).contains(QLatin1String(".qbpkg"))) {
            for (const QString &dir : m_side) {
                if (QFileInfo(dir).fileName() == QLatin1String("Download") && QFileInfo(dir).isDir()) {
                    dialog->setDirectory(dir);
                    return false;
                }
            }
        }
        const QString now = clean(dialog->directory().absolutePath());
        bool standard = false;
        for (const QString &s : m_standards)
            standard = standard || now == clean(s);
        if (standard && QFileInfo(m_folders.projects).isDir()) {
            const QStringList chosen = dialog->selectedFiles();
            dialog->setDirectory(m_folders.projects);
            // (a proposed file name stays: Compile, Export)
            if (dialog->acceptMode() == QFileDialog::AcceptSave && !chosen.isEmpty() && !QFileInfo(chosen.constFirst()).isDir())
                dialog->selectFile(QFileInfo(chosen.constFirst()).fileName());
            qCInfo(lcFiles, "files: a file window opened in the projects folder instead of the app's own");
        }
        return false;
    }

private:
    Folders m_folders;
    QStringList m_side;
    QStringList m_standards;
};

QPointer<FileWindows> s_fileWindows;

} // namespace

bool askForFiles(QWidget *parent, const FilesSystem &system)
{
    FilesDialog dialog(parent, system);
    dialog.exec();
    return system.allowed && system.allowed();
}

void watchFileWindows(const Folders &folders, const QStringList &side, const QStringList &standards)
{
    if (s_fileWindows) {
        s_fileWindows->setFolders(folders, side, standards);
        return;
    }
    s_fileWindows = new FileWindows(qApp, folders, side, standards);
    s_fileWindows->setObjectName(QStringLiteral("mobileFileWindows"));
    qApp->installEventFilter(s_fileWindows);
}

// ---- Android itself

#ifdef Q_OS_ANDROID

namespace {

const char kKeptInApp[] = "android/filesKeptInApp"; // the writer chose "Keep Everything Inside the App"

int sdk() { return QNativeInterface::QAndroidApplication::sdkVersion(); }

// Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_...): the tablet's shared folders
QString sharedFolder(const char *field)
{
    const QJniObject name = QJniObject::getStaticObjectField("android/os/Environment", field, "Ljava/lang/String;");
    if (!name.isValid())
        return {};
    const QJniObject file = QJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStoragePublicDirectory",
                                                               "(Ljava/lang/String;)Ljava/io/File;", name.object());
    return file.isValid() ? file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;").toString() : QString();
}

bool accessAllowed()
{
    if (sdk() >= 30) // Android 11 and later: "All files access"
        return QJniObject::callStaticMethod<jboolean>("android/os/Environment", "isExternalStorageManager", "()Z");
    const QJniObject context(QNativeInterface::QAndroidApplication::context());
    const QJniObject permission = QJniObject::fromString(QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"));
    return context.isValid()
           && context.callMethod<jint>("checkSelfPermission", "(Ljava/lang/String;)I", permission.object()) == 0;
}

// true when Android took the request (a page or a window of Android's own opens)
bool startSettings(const QJniObject &context, const QString &action, bool withPackage)
{
    QJniEnvironment env;
    const QJniObject jAction = QJniObject::fromString(action);
    QJniObject intent;
    if (withPackage) {
        const QString package = context.callObjectMethod("getPackageName", "()Ljava/lang/String;").toString();
        const QJniObject address = QJniObject::fromString(QStringLiteral("package:") + package);
        const QJniObject uri = QJniObject::callStaticObjectMethod("android/net/Uri", "parse",
                                                                  "(Ljava/lang/String;)Landroid/net/Uri;", address.object());
        intent = QJniObject("android/content/Intent", "(Ljava/lang/String;Landroid/net/Uri;)V", jAction.object(), uri.object());
    } else {
        intent = QJniObject("android/content/Intent", "(Ljava/lang/String;)V", jAction.object());
    }
    if (!intent.isValid())
        return false;
    // by hand, to see whether Android refused (a tablet without that page): QJniObject's calls swallow that
    jclass cls = env->GetObjectClass(context.object());
    const jmethodID start = env.findMethod(cls, "startActivity", "(Landroid/content/Intent;)V");
    env->DeleteLocalRef(cls);
    if (!start)
        return false;
    env->CallVoidMethod(context.object(), start, intent.object());
    return !env.checkAndClearExceptions();
}

void askAndroid()
{
    // (on Android's own thread, where an app's windows are asked for)
    (void)QNativeInterface::QAndroidApplication::runOnAndroidMainThread([]() -> QVariant {
        const QJniObject context(QNativeInterface::QAndroidApplication::context());
        if (!context.isValid())
            return {};
        if (sdk() >= 30) {
            // the page of this app; a tablet that does not have it shows the list of all apps
            if (!startSettings(context, QStringLiteral("android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION"), true))
                startSettings(context, QStringLiteral("android.settings.MANAGE_ALL_FILES_ACCESS_PERMISSION"), false);
            return {};
        }
        QJniEnvironment env;
        jclass strings = env.findClass("java/lang/String");
        if (!strings)
            return {};
        jobjectArray wanted = env->NewObjectArray(2, strings, nullptr);
        env->SetObjectArrayElement(wanted, 0,
                                   QJniObject::fromString(QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE")).object());
        env->SetObjectArrayElement(wanted, 1,
                                   QJniObject::fromString(QStringLiteral("android.permission.READ_EXTERNAL_STORAGE")).object());
        context.callMethod<void>("requestPermissions", "([Ljava/lang/String;I)V", wanted, jint(7411));
        env->DeleteLocalRef(wanted);
        return {};
    });
}

FilesSystem androidSystem()
{
    FilesSystem system;
    system.allowed = [] { return accessAllowed(); };
    system.ask = [] { askAndroid(); };
    system.settingsPage = sdk() >= 30;
    system.documents = sharedFolder("DIRECTORY_DOCUMENTS");
    return system;
}

// the app's own folders (what Qt names as the standard places): a folder setting inside them was never chosen
QStringList ownRoots()
{
    QStringList roots;
    const auto up = [](const QString &path) { return path.isEmpty() ? QString() : QFileInfo(path).absolutePath(); };
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation); // /data/user/0/<id>/files
    roots << up(home);
    if (sdk() >= 30) {
        // /storage/emulated/0/Android/data/<id>/files
        roots << up(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    } else {
        // Android 9, 10: Qt's "generic data" is the shared storage itself; the program's part of it is its data folder
        roots << dataDir();
    }
    roots.removeAll(QString());
    return roots;
}

void settle(bool access, const char *when)
{
    const FilesSystem system = androidSystem();
    const Folders f = foldersIn(system.documents);
    if (system.documents.isEmpty()) {
        qCInfo(lcFiles, "files: Android names no Documents folder; everything stays in the app's own folders");
        return;
    }
    const bool changed = settleFolders(f, ownRoots(), access);
    qCInfo(lcFiles, "files (%s): access to the shared folders %s; projects in %s, backups in %s%s", when,
           access ? "allowed" : "NOT allowed", qPrintable(AppSettings::instance().lastProjectDir),
           qPrintable(AppSettings::instance().backupDir), changed ? " (settings changed)" : "");
    // file windows: the shared folders at the side; "standard" folders a window may open in for want of a better one
    QStringList side, standards;
    if (access)
        side << f.projects << system.documents << sharedFolder("DIRECTORY_DOWNLOADS");
    else
        side << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (access && sdk() >= 30)
        standards << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                  << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                  << QStandardPaths::writableLocation(QStandardPaths::HomeLocation) << QDir::homePath();
    watchFileWindows(f, side, standards);
}

void filesWindow(QMainWindow *window, bool atStart)
{
    const FilesSystem system = androidSystem();
    if (system.documents.isEmpty())
        return;
    QSettings st;
    if (atStart) {
        if (system.allowed()) {
            st.remove(QLatin1String(kKeptInApp));
            return;
        }
        if (st.value(QLatin1String(kKeptInApp)).toBool()) {
            settle(false, "start, kept in the app");
            return;
        }
    }
    const bool allowed = askForFiles(window, system);
    if (allowed)
        st.remove(QLatin1String(kKeptInApp));
    else
        st.setValue(QLatin1String(kKeptInApp), true);
    settle(allowed, atStart ? "the first question" : "File > Files on This Tablet");
}

} // namespace

void filesBeforeWindow()
{
    // The folder settings must be right before the main window, the first start's window and the welcome project
    // read them. With the access given at an earlier start that is all there is to do.
    const bool allowed = accessAllowed();
    qCInfo(lcFiles, "files: Android %d, shared Documents folder %s", sdk(), qPrintable(sharedFolder("DIRECTORY_DOCUMENTS")));
    if (allowed)
        settle(true, "start");
}

void filesAfterWindow(QMainWindow *window)
{
    // the entry in the File menu, before Options
    if (auto *options = window->findChild<QAction *>(QStringLiteral("options"))) {
        const QList<QAction *> menus = window->menuBar()->actions();
        for (QAction *top : menus) {
            QMenu *menu = top->menu();
            if (!menu || !menu->actions().contains(options))
                continue;
            auto *entry = new QAction(QCoreApplication::translate("Mobile", "Files on This &Tablet…"), window);
            entry->setObjectName(QStringLiteral("filesOnTablet"));
            QObject::connect(entry, &QAction::triggered, window, [window] { filesWindow(window, false); });
            menu->insertAction(options, entry);
            break;
        }
    }
    filesWindow(window, true); // asks once, when the access is not there
}

#endif // Q_OS_ANDROID

} // namespace mobile
} // namespace qb
