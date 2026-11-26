#include "android/Mobile.h"

#include "compile/PluginManager.h"
#include "core/Addons.h"
#include "core/Paths.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/KeyboardLayout.h"
#include "ui/MainWindow.h"
#include "ui/MindMapDoc.h"
#include "ui/SpellChecker.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QInputMethod>
#include <QLocale>
#include <QLoggingCategory>
#include <QSettings>
#ifdef Q_OS_ANDROID
#include <QSslSocket>
#endif

// The start of the program on a tablet (2026-11-14): what the Linux program gets from its system or from its add-on
// packages comes out of the APK here.
//  - Spelling: Linux uses the system's dictionaries, Windows a folder beside the program. An APK is no folder: the
//    dictionary files are in its assets and are copied once into the program's data folder, where the spelling
//    checker looks anyway.
//  - Saving: Android puts an app away without asking and may end it there at any time. The project is saved in full
//    at that moment, as the Linux program does on the way to another program.
//  - Commands that cannot work in an APK are taken out of the menus.
// The shared folders (MobileFiles.cpp) are settled from here too.

namespace qb {
namespace mobile {

namespace {

Q_LOGGING_CATEGORY(lcStart, "qb.tablet", QtInfoMsg)

} // namespace

QString spellLanguageFor(const QString &locale)
{
    static const QStringList have{QStringLiteral("de_DE"), QStringLiteral("de_AT"), QStringLiteral("de_CH"),
                                  QStringLiteral("en_GB"), QStringLiteral("en_US")};
    if (have.contains(locale))
        return locale;
    if (locale.startsWith(QLatin1String("de_")) || locale == QLatin1String("de"))
        return QStringLiteral("de_DE");
    if (locale.startsWith(QLatin1String("en_")) || locale == QLatin1String("en"))
        return QStringLiteral("en_GB");
    return {};
}

int copyDictionaries(const QString &from, const QString &to)
{
    const QFileInfoList files = QDir(from).entryInfoList({QStringLiteral("*.aff"), QStringLiteral("*.dic")}, QDir::Files);
    if (files.isEmpty())
        return 0;
    if (!QDir().mkpath(to))
        return -1;
    int copied = 0;
    bool failed = false;
    for (const QFileInfo &fi : files) {
        const QString target = to + QLatin1Char('/') + fi.fileName();
        const QFileInfo there(target);
        if (there.exists() && there.size() == fi.size())
            continue;
        // through a file of another name: a copy that is cut off never counts as a dictionary
        const QString part = target + QStringLiteral(".part");
        QFile::remove(part);
        if (!QFile::copy(fi.filePath(), part)) {
            failed = true;
            continue;
        }
        QFile::setPermissions(part, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
        QFile::remove(target);
        if (!QFile::rename(part, target)) {
            QFile::remove(part);
            failed = true;
            continue;
        }
        ++copied;
    }
    return failed ? -1 : copied;
}

bool saveAllNow(QMainWindow *window)
{
    auto *main = qobject_cast<MainWindow *>(window);
    Project *project = main ? main->project() : nullptr;
    if (!project)
        return true;
    QString boardError, error;
    const bool boards = mindmaps::saveAll(project, &boardError);
    bool saved = project->save(&error);
    if (!boards) {
        saved = false;
        error = error.isEmpty() ? boardError : error + QLatin1Char('\n') + boardError;
    }
    if (!saved)
        emit project->saveFailed(error); // (the window's own notice and its retries)
    return saved;
}

namespace {

// Dictionaries out of the APK, and at the very first start the tablet's language for the spelling.
void spellingAtStart()
{
#ifdef Q_OS_ANDROID
    const QString to = dataDir() + QStringLiteral("/dictionaries");
    const int copied = copyDictionaries(QStringLiteral("assets:/dictionaries"), to);
    AppSettings &s = AppSettings::instance();
    QSettings st;
    const QStringList have = SpellChecker::instance().availableLanguages();
    if (!have.isEmpty() && !st.value(QStringLiteral("android/spellLanguageChosen")).toBool()) {
        const QString wanted = spellLanguageFor(QLocale::system().name());
        if (!wanted.isEmpty() && have.contains(wanted) && wanted != s.spellLanguage) {
            s.spellLanguage = wanted;
            s.save();
        }
        st.setValue(QStringLiteral("android/spellLanguageChosen"), true);
    }
    if (copied != 0 || SpellChecker::instance().language().isEmpty())
        SpellChecker::instance().setLanguage(s.spellLanguage); // (it looked before the files were there)
    qCInfo(lcStart, "tablet: dictionaries %s (%d copied now), languages %s, spelling in %s",
           copied < 0 ? "COULD NOT BE COPIED" : "in place", qMax(0, copied), qPrintable(have.join(QLatin1Char(' '))),
           qPrintable(SpellChecker::instance().language()));
#endif
}

} // namespace

int installFullAddons(const QString &from)
{
    const QFileInfoList packages = QDir(from).entryInfoList({QStringLiteral("*.qbpkg")}, QDir::Files, QDir::Name);
    if (packages.isEmpty())
        return 0;
    // once per version (addons::installBundled keeps its mark in the data folder): nothing to copy after that
    if (QFile::exists(dataDir() + QStringLiteral("/full-addons-installed-" QB_APP_VERSION)))
        return 0;
    // out of the APK into a folder of the app's own (the installer opens the package as a file of the file system)
    const QString dir = QDir::tempPath() + QStringLiteral("/qb-full-addons");
    QDir(dir).removeRecursively();
    if (!QDir().mkpath(dir))
        return -1;
    bool copied = true;
    for (const QFileInfo &fi : packages)
        copied = QFile::copy(fi.filePath(), dir + QLatin1Char('/') + fi.fileName()) && copied;
    QStringList log;
    QString error;
    const bool ok = copied && addons::installBundled(dir, &log, &error);
    for (const QString &l : log)
        qCInfo(lcStart, "full version: %s", qPrintable(l));
    if (!ok)
        qCWarning(lcStart, "full version: not every add-on could be installed: %s", qPrintable(copied ? error : QStringLiteral("a copy failed")));
    QDir(dir).removeRecursively();
    return ok ? int(packages.size()) : -1;
}

void tabletBeforeWindow()
{
#ifdef Q_OS_ANDROID
    filesBeforeWindow();
#endif
    spellingAtStart();
}

void startTablet(QMainWindow *window)
{
    if (!window)
        return;
    // The program comes as one APK (no program updates); add-ons come as packages since 0.91.3 (the writer, 2026-11-16:
    // signed add-ons allowed); the application menu is the Linux desktop's.
    for (const char *name : {"integrate"}) {
        if (auto *action = window->findChild<QAction *>(QLatin1String(name)))
            action->setVisible(false);
    }
    // Android puts the app away (Home, another app, the screen off) and may end it there without a word
    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, window, [window](Qt::ApplicationState state) {
        static bool away = false;
        if (state == Qt::ApplicationActive) {
            away = false;
            return;
        }
        if (away)
            return;
        away = true;
        const bool saved = saveAllNow(window);
        qCInfo(lcStart, "tablet: put away by Android, the open project %s", saved ? "is saved" : "COULD NOT BE SAVED");
    });
    for (const QString &line : compile::PluginManager::instance().messages())
        qCInfo(lcStart, "tablet: plug-ins: %s", qPrintable(line));
    // Android 0.94.0 (to-do item 79): the shortcuts by the keyboard's layout, read from the input method's language at
    // the first start (src/ui/KeyboardLayoutSystem.cpp); the start check shows what a tablet reports
    qCInfo(lcStart, "tablet: keyboard layout from the input method: %s (language %s); the shortcut set kept: %s",
           qPrintable(keyboardlayout::systemLayout().isEmpty() ? QStringLiteral("none") : keyboardlayout::systemLayout()),
           qPrintable(QGuiApplication::inputMethod() ? QGuiApplication::inputMethod()->locale().name() : QStringLiteral("?")),
           qPrintable(QSettings().value(QStringLiteral("keyboard/layoutSet")).toString()));
#ifdef Q_OS_ANDROID
    // https needs OpenSSL's libraries out of the APK (File > Import > Web Page)
    qCInfo(lcStart, "tablet: secure connections (https) %s (%s)", QSslSocket::supportsSsl() ? "work" : "DO NOT WORK",
           qPrintable(QSslSocket::sslLibraryVersionString()));
    installOpener();
    filesAfterWindow(window);
#endif
    spellingAtStart(); // (again: on Android 9 and 10 the data folder can be written only once the access is given)
}

} // namespace mobile
} // namespace qb
