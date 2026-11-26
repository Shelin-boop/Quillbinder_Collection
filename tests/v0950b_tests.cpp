// 0.95.0, second part (xvfb-run -a ./qb_v0950b_tests /tmp/shots):
//  - item 92: the first-start page's texts about moving the AppImage; the menu entry always names the AppImage's
//    current place (a fresh home, an older entry that is replaced and not doubled, a file moved by hand).
#include "AppInfo.h"
#include "compile/PluginManager.h"
#include "ui/AppMenuEntry.h"
#include "ui/AppSettings.h"
#include "ui/FirstStartDialog.h"
#include "ui/Theme.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>

#include <cstdio>

using namespace qb;

static int failures = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 150)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

static void shot(QWidget *w, const QString &name)
{
    settle(300);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static void writeFile(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
}

static QString fileText(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

static QStringList entryFiles(const QString &data)
{
    return QDir(data + QLatin1String("/applications")).entryList({QStringLiteral("*.desktop")}, QDir::Files);
}

// ---- item 92
static void appImageMove(const QString &home)
{
#ifdef Q_OS_LINUX
    const QString data = qEnvironmentVariable("XDG_DATA_HOME");
    const QString dl = home + QLatin1String("/Downloads/Quillbinder-9.9.9-test-x86_64.AppImage");
    writeFile(dl, QByteArray(30000, 'x'));
    QFile::setPermissions(dl, QFile::permissions(dl) | QFileDevice::ExeOwner);
    qputenv("APPIMAGE", dl.toUtf8());
    {
        FirstStartDialog d;
        d.show();
        settle(300);
        auto *move = d.findChild<QCheckBox *>(QStringLiteral("firstMenuMove"));
        auto *hint = d.findChild<QLabel *>(QStringLiteral("firstMenuHint"));
        const QString text = move ? move->text() : QString();
        check(move && text.contains(QStringLiteral("a fixed place the menu entry starts it from")) && !text.contains(QLatin1Char('(')),
              QStringLiteral("the move checkbox names the fixed place and has no bracket"), text);
        check(hint && hint->text().contains(QStringLiteral("If you move the AppImage yourself, add it to the menu again.")),
              QStringLiteral("the grey line tells that a file moved by hand needs the menu entry again"), hint ? hint->text() : QString());
        shot(&d, QStringLiteral("92-first-start-menu"));
    }
    // (a) a fresh home: the entry starts the moved AppImage
    check(entryFiles(data).isEmpty(), QStringLiteral("a fresh home has no menu entry"));
    const QString moved = appmenu::applicationsFolder() + QLatin1String("/") + QFileInfo(dl).fileName();
    appmenu::Result r = appmenu::add(true, {});
    check(r.ok && r.moved && QFile::exists(moved) && !QFile::exists(dl) && entryFiles(data).size() == 1
              && fileText(appmenu::desktopFilePath()).contains(QStringLiteral("Exec=\"%1\"").arg(moved)),
          QStringLiteral("fresh home: the one menu entry starts the AppImage in ~/Applications"), r.error + r.moveError);
    // (b) an older entry (it may start an old AppImage): replaced, not doubled
    writeFile(appmenu::desktopFilePath(),
              "[Desktop Entry]\nType=Application\nName=Quillbinder\nExec=\"/old/place/Quillbinder-0.1.0-x86_64.AppImage\" %f\nIcon=quillbinder\n");
    const QString dl2 = home + QLatin1String("/Downloads/Quillbinder-9.9.10-test-x86_64.AppImage");
    writeFile(dl2, QByteArray(30000, 'y'));
    qputenv("APPIMAGE", dl2.toUtf8());
    const QString moved2 = appmenu::applicationsFolder() + QLatin1String("/") + QFileInfo(dl2).fileName();
    r = appmenu::add(true, {});
    const QString text = fileText(appmenu::desktopFilePath());
    check(r.ok && r.moved && entryFiles(data).size() == 1 && text.contains(QStringLiteral("Exec=\"%1\"").arg(moved2))
              && !text.contains(QStringLiteral("0.1.0")) && text.count(QStringLiteral("[Desktop Entry]")) == 1,
          QStringLiteral("an older entry is replaced, not doubled, and starts the new AppImage in ~/Applications"), text);
    // a file moved by hand: Add to Application Menu writes its current place
    const QString by_hand = home + QLatin1String("/Elsewhere/") + QFileInfo(dl2).fileName();
    QDir().mkpath(home + QLatin1String("/Elsewhere"));
    QFile::rename(moved2, by_hand);
    qputenv("APPIMAGE", by_hand.toUtf8());
    r = appmenu::add(false, {});
    check(r.ok && !r.moved && fileText(appmenu::desktopFilePath()).contains(QStringLiteral("Exec=\"%1\"").arg(by_hand))
              && QSettings().value(QStringLiteral("app/integrated")).toString() == by_hand,
          QStringLiteral("Add to Application Menu after a move by hand writes the AppImage's current place"));
    QString err;
    appmenu::remove(&err);
    qunsetenv("APPIMAGE");
#else
    (void)home;
#endif
}

// @@FUNCTIONS@@

// ---- main
int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0950bTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0950bTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();
    compile::PluginManager::instance().load();

    appImageMove(home.path());
    // @@SECTIONS@@

    std::printf("%d failure(s)\n", failures);
    return failures;
}
