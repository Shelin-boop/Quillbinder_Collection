// Scripted run for version 0.93.0 beta, second suite: fixes in code that several systems share (to-do items 15, 16, 26).
//  - item 15: exporting options and themes never destroys the file that was there (a write that fails leaves the old
//    file byte for byte, and says so);
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0930b_tests /tmp/shots
#include "core/Paths.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "ui/OptionsDialog.h"
#include "ui/Theme.h"

#include "AppInfo.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
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
            std::printf("      %s\n", qPrintable(detail.left(3000)));
    }
    std::fflush(stdout);
}

static QByteArray readAll(const QString &f)
{
    QFile file(f);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

static void writeAll(const QString &f, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(f).absolutePath());
    QFile file(f);
    if (file.open(QIODevice::WriteOnly))
        file.write(data);
}

// ------------------------------------------------------------------ item 15: exports that keep the old file

static void exports(const QString &W)
{
    MainWindow w;
    OptionsDialog d(&w, 0);
    const QString opt = W + QStringLiteral("/export/my.qboptions");
    const QString thm = W + QStringLiteral("/export/my.qbtheme");
    QDir().mkpath(W + QStringLiteral("/export"));
    QString e;
    check(d.saveOptionsToFile(opt, &e) && QFile::exists(opt), QStringLiteral("an options file is written"), e);
    theme::Theme look = theme::Theme();
    check(OptionsDialog::saveThemeToFile(thm, look, &e) && QFile::exists(thm), QStringLiteral("a theme file is written"), e);
    // an older export holds an entry that the new one must not inherit
    {
        QSettings s(opt, QSettings::IniFormat);
        s.setValue(QStringLiteral("zzold/entry"), 7);
        s.sync();
    }
    d.saveOptionsToFile(opt, &e);
    check(!QSettings(opt, QSettings::IniFormat).contains(QStringLiteral("zzold/entry")),
          QStringLiteral("a second export is the new options only (no entry of the old file is mixed in)"));
    check(QDir(W + QStringLiteral("/export")).entryList(QDir::Files).size() == 2, QStringLiteral("no temporary file stays beside the exports"),
          QDir(W + QStringLiteral("/export")).entryList(QDir::Files).join(QLatin1Char(',')));

    const QByteArray oldOpt = readAll(opt), oldThm = readAll(thm);
    qputenv("QB_TEST_WRITE_FAILS", "5|Input/output error|/export/my.");
    QString eo, et;
    const bool okO = d.saveOptionsToFile(opt, &eo);
    const bool okT = OptionsDialog::saveThemeToFile(thm, look, &et);
    qunsetenv("QB_TEST_WRITE_FAILS");
    check(!okO && readAll(opt) == oldOpt && !oldOpt.isEmpty(), QStringLiteral("options: a write that fails leaves the old file byte for byte"));
    check(!okT && readAll(thm) == oldThm && !oldThm.isEmpty(), QStringLiteral("theme: a write that fails leaves the old file byte for byte"));
    check(eo.contains(QLatin1String("unchanged")) && et.contains(QLatin1String("unchanged")) && et.contains(QLatin1String("Input/output error")),
          QStringLiteral("both messages come, name the reason and say the old file is unchanged"), eo + QLatin1Char('|') + et);
    check(QDir(W + QStringLiteral("/export")).entryList(QDir::Files).size() == 2, QStringLiteral("... and no temporary file is left"),
          QDir(W + QStringLiteral("/export")).entryList(QDir::Files).join(QLatin1Char(',')));
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0930bTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0930bTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    QTemporaryDir work;
    const QString W = work.path();
    exports(W);
    std::printf("%d failure(s)\n", failures);
    return failures;
}
