// Scripted run for version 0.60: one source and one version for everyone. The
// version; About names FFmpeg with its license; the built-in character and place sheets have
// fields of the program's own choice; the media package with all formats (a bundle of both
// media add-ons) installs both in one step; the release text and the README point to it; the
// release workflow runs in the project's own repository only; the downloads are sorted by system.
// Exit code = number of failed checks.
//
//   QT_QPA_PLATFORM=offscreen ./qb_v060_tests /tmp/shots
//   (the media package checks need tools/get-ffmpeg.sh and packaging/linux/build-addons.sh build build/addons-dist)
#include "core/Addons.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "text/TextUtil.h"
#include "ui/MainWindow.h"
#include "AppInfo.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextDocument>
#include <cstdio>
#include <functional>

using namespace qb;

static int failures = 0;
static const QString kSource = QDir::cleanPath(QStringLiteral(QB_TEST_SOURCE_DIR "/.."));
static const QString kBuild = QStringLiteral(QB_TEST_BUILD_DIR);

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

static void info(const QString &what)
{
    std::printf("      %s\n", qPrintable(what));
    std::fflush(stdout);
}

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

static bool writeFile(const QString &path, const QByteArray &data, bool append = false)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(append ? QIODevice::Append : QIODevice::WriteOnly))
        return false;
    return f.write(data) == data.size();
}

// runs a program; returns stdout + stderr
static QString run(const QString &program, const QStringList &args, int *rc = nullptr, const QString &cwd = QString())
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    if (!cwd.isEmpty())
        p.setWorkingDirectory(cwd);
    p.start(program, args);
    p.waitForFinished(600000);
    if (rc)
        *rc = p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1;
    return QString::fromUtf8(p.readAll());
}

static QString runIn(const QString &program, const QStringList &args, const QProcessEnvironment &env, int *rc)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.setProcessEnvironment(env);
    p.start(program, args);
    p.waitForFinished(600000);
    *rc = p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1;
    return QString::fromUtf8(p.readAll());
}

static BinderItem *findTitle(BinderItem *it, const QString &title)
{
    if (!it)
        return nullptr;
    if (it->title == title)
        return it;
    for (BinderItem *c : it->children)
        if (BinderItem *f = findTitle(c, title))
            return f;
    return nullptr;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QStringLiteral("/data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QStringLiteral("/config")).toUtf8());
    qputenv("QB_NO_WELCOME", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("Quillbinder"));
    QCoreApplication::setApplicationName(QStringLiteral("Quillbinder"));
    QApplication app(argc, argv);
    const QString tools = kSource + QStringLiteral("/tools/");

    // ================================================================ version
    check(Project::compareVersionText(QLatin1String(QB_APP_VERSION), QStringLiteral("0.60.0")) >= 0
              && QLatin1String(QB_APP_VERSION_SHOWN).endsWith(QLatin1String(" " QB_APP_VERSION_LABEL)),
          QStringLiteral("the version is 0.60.0 or newer, shown with its label"), QStringLiteral(QB_APP_VERSION_SHOWN));

    // ================================================================ About names FFmpeg with its license
    {
        const QString about = MainWindow::aboutText();
        check(about.contains(QLatin1String("FFmpeg")) && about.contains(QLatin1String("LGPL 2.1"))
                  && about.contains(QLatin1String("Third-party source code")) && about.contains(QLatin1String(QB_APP_HOMEPAGE)),
              QStringLiteral("About: the audio and video add-ons use FFmpeg under the LGPL 2.1, with where its source is"), about);
        check(about.contains(QLatin1String("not made by or connected with Literature &amp; Latte"))
                  && about.contains(QLatin1String("Main Developer: Claude.AI w. Opus 5.5 Extra")),
              QStringLiteral("About still says it is independent of Literature & Latte, with the credit"));
    }

    // ================================================================ character and place sheets of the novel template
    {
        QTemporaryDir tmp;
        QString err;
        templates::Info novel;
        for (const auto &i : templates::all())
            if (i.id == QLatin1String("novel"))
                novel = i;
        auto p = templates::createProject(novel, tmp.path(), QStringLiteral("Sheets"), &err);
        check(p != nullptr, QStringLiteral("a project from the novel template"), err);
        if (p) {
            BinderItem *ch = findTitle(p->root(), QStringLiteral("Character Sheet"));
            BinderItem *pl = findTitle(p->root(), QStringLiteral("Setting Sheet"));
            const QString c = ch ? text::plainText(p->docs()->text(ch->id)) : QString();
            const QString s = pl ? text::plainText(p->docs()->text(pl->id)) : QString();
            const QStringList newC = {QStringLiteral("In one sentence:"), QStringLiteral("Part in the plot:"),
                                      QStringLiteral("Voice (how they talk):"), QStringLiteral("Ties to other characters:"),
                                      QStringLiteral("From the first scene to the last:"), QStringLiteral("Open questions:")};
            const QStringList oldC = {QStringLiteral("Role in story"), QStringLiteral("Physical description"),
                                      QStringLiteral("Habits and mannerisms"), QStringLiteral("Internal conflicts"),
                                      QStringLiteral("Personality:")};
            bool cOk = ch != nullptr;
            for (const QString &f : newC)
                cOk = cOk && c.contains(f);
            for (const QString &f : oldC)
                cOk = cOk && !c.contains(f);
            check(cOk, QStringLiteral("the character sheet has the fields of 0.60 and none of the list it had before"), c);
            const bool sOk = pl && s.contains(QLatin1String("By day and by night:")) && s.contains(QLatin1String("Who comes here:"))
                             && !s.contains(QLatin1String("Sights:")) && !s.contains(QLatin1String("Smells:"))
                             && !s.contains(QLatin1String("Role in story"));
            check(sOk, QStringLiteral("the place sheet has the fields of 0.60 and none of the list it had before"), s);
            check(p->templatesFolderId.isEmpty() == false && findTitle(p->root(), QStringLiteral("Characters"))
                      && findTitle(p->root(), QStringLiteral("Characters"))->defaultTemplate == (ch ? ch->id : QString()),
                  QStringLiteral("new documents in Characters still start from the character sheet"));
        }
    }

    // ================================================================ the media package with all formats
    {
        const QString all = kBuild + QStringLiteral("/addons-dist/media-all-formats.qbpkg");
        if (!QFile::exists(all)) {
            info(QStringLiteral("(media packages not built: tools/get-ffmpeg.sh, then packaging/linux/build-addons.sh <build> <build>/addons-dist)"));
        } else {
            const addons::PackageInfo pi = addons::inspect(all);
            QStringList nested;
            for (const auto &n : pi.nested)
                nested << n.manifest.id + QLatin1Char(' ') + n.manifest.version;
            check(pi.isValid() && pi.manifest.kind == QLatin1String("bundle") && pi.manifest.id == QLatin1String("media-all-formats")
                      && pi.manifest.name == QLatin1String("Media Playback and Extra Formats")
                      && pi.manifest.packages == QStringList({QStringLiteral("media-playback.qbpkg"), QStringLiteral("media-extra-formats.qbpkg")}),
                  QStringLiteral("media-all-formats.qbpkg is one package holding Media Playback and Extra Video and Audio Formats"),
                  pi.error + QLatin1String(" / ") + nested.join(QLatin1String(", ")));
            check(pi.manifest.description.contains(QLatin1String("H.264")) && pi.manifest.description.contains(QLatin1String("patent")),
                  QStringLiteral("... and says that it brings the formats licensed under patents"), pi.manifest.description);
            QStringList log;
            QString err;
            bool restart = false;
            const bool ok = addons::install(all, &log, &err, &restart);
            QStringList ids;
            bool allOn = true;
            for (const auto &a : addons::installedAddons()) {
                ids << a.manifest.id;
                allOn = allOn && a.enabled;
            }
            check(ok && ids.contains(QLatin1String("media-playback")) && ids.contains(QLatin1String("media-extra-formats")) && allOn,
                  QStringLiteral("installing it in one step installs both media add-ons, switched on"),
                  err + QLatin1String(" / ") + ids.join(QLatin1String(", ")) + QLatin1String(" / ") + log.join(QLatin1String("; ")));
            const QString root = addons::addonsRoot();
            check(QFileInfo::exists(root + QStringLiteral("/media-playback/plugins/libqb-media.so"))
                      && QFileInfo::exists(root + QStringLiteral("/media-extra-formats/media-extra/lib/libavcodec.so.63"))
                      && readAll(root + QStringLiteral("/media-extra-formats/media-extra/licenses/README.txt")).contains("LGPL"),
                  QStringLiteral("... with the player, the full FFmpeg the player then uses, and its license note"), root);
        }
    }

    // ================================================================ the full version: the AppImage with every add-on inside
    {
        const QString shown = QString::fromLatin1(QB_APP_VERSION_SHOWN).replace(QLatin1Char(' '), QLatin1Char('-'));
        const QString appImage = kSource + QStringLiteral("/build-appimage/Quillbinder-%1-x86_64.AppImage").arg(shown);
        const QString dist = kBuild + QStringLiteral("/addons-dist");
        const QStringList ids = QDir(kSource + QStringLiteral("/addons")).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        bool havePackages = !ids.isEmpty();
        for (const QString &id : ids)
            havePackages = havePackages && QFile::exists(dist + QLatin1Char('/') + id + QStringLiteral(".qbpkg"));
        if (!QFile::exists(appImage) || !havePackages
            || !QFile::exists(kSource + QStringLiteral("/packaging/tools/appimagetool-x86_64.AppImage"))) {
            info(QStringLiteral("(full version not checked: needs %1 (packaging/linux/build-appimage.sh) and the add-on packages)").arg(appImage));
        } else {
            QTemporaryDir work;
            const QString bdir = work.path() + QStringLiteral("/b");
            QDir().mkpath(bdir);
            QFile::copy(appImage, bdir + QLatin1Char('/') + QFileInfo(appImage).fileName());
            QFile::setPermissions(bdir + QLatin1Char('/') + QFileInfo(appImage).fileName(), QFile::permissions(appImage));
            int rc = 0;
            const QString out = run(QStringLiteral("bash"), {kSource + QStringLiteral("/packaging/linux/build-full-appimage.sh"), bdir, dist}, &rc);
            const QString full = bdir + QStringLiteral("/Quillbinder-%1-full-%2-x86_64.AppImage")
                                            .arg(QLatin1String(QB_APP_VERSION), shown.mid(int(qstrlen(QB_APP_VERSION)) + 1));
            check(rc == 0 && QFile::exists(full) && out.contains(QStringLiteral("(%1 add-ons inside)").arg(ids.size()))
                      && QFileInfo(full).size() > QFileInfo(appImage).size() + 10 * 1000 * 1000,
                  QStringLiteral("build-full-appimage.sh makes the full version: the AppImage with all %1 add-ons inside").arg(ids.size()),
                  out.right(1500));
            // its first start, with an empty home folder: every add-on installed, then the program runs
            QDir().mkpath(work.path() + QStringLiteral("/home"));
            QProcessEnvironment env;
            env.insert(QStringLiteral("HOME"), work.path() + QStringLiteral("/home"));
            env.insert(QStringLiteral("PATH"), QStringLiteral("/usr/bin:/bin"));
            const QString addonsDir = work.path() + QStringLiteral("/home/.local/share/Quillbinder/addons");
            const QString first = runIn(full, {QStringLiteral("--appimage-extract-and-run"), QStringLiteral("--version")}, env, &rc);
            const QStringList installed = QDir(addonsDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
            check(rc == 0 && first.contains(QStringLiteral("Quillbinder %1").arg(QLatin1String(QB_APP_VERSION))) && installed == ids
                      && QFile::exists(work.path() + QStringLiteral("/home/.local/share/Quillbinder/full-addons-installed-" QB_APP_VERSION)),
                  QStringLiteral("its first start installs every add-on, then the program runs"),
                  first + QLatin1String(" / ") + installed.join(QLatin1String(", ")));
            // an add-on removed afterwards stays removed
            QDir(addonsDir + QStringLiteral("/themes-extra")).removeRecursively();
            runIn(full, {QStringLiteral("--appimage-extract-and-run"), QStringLiteral("--version")}, env, &rc);
            check(rc == 0 && !QFileInfo::exists(addonsDir + QStringLiteral("/themes-extra"))
                      && QFileInfo::exists(addonsDir + QStringLiteral("/media-extra-formats/media-extra/lib/libavcodec.so.63")),
                  QStringLiteral("later starts install nothing again (an add-on removed by the writer stays removed)"));
            // what is inside: the packages and a text that says what the full version is
            const QString ex = work.path() + QStringLiteral("/x");
            QDir().mkpath(ex);
            QProcess xp;
            xp.setWorkingDirectory(ex);
            xp.start(full, {QStringLiteral("--appimage-extract")});
            xp.waitForFinished(300000);
            const QString readme = QString::fromUtf8(readAll(ex + QStringLiteral("/squashfs-root/usr/share/quillbinder/full-addons/README.txt")));
            const QString appRun = QString::fromUtf8(readAll(ex + QStringLiteral("/squashfs-root/AppRun")));
            check(readme.contains(QLatin1String("full version")) && readme.contains(QLatin1String("all its add-ons and the libraries they need"))
                      && readme.contains(QLatin1String("Add-on Manager")) && appRun.contains(QLatin1String("--install"))
                      && appRun.contains(QLatin1String("exec ")),
                  QStringLiteral("inside: a text that explains the full version, and the start script"), readme.left(600));
        }
    }

    // ================================================================ release text, README, workflow
    {
        QTemporaryDir dist;
        const QString v = QStringLiteral(QB_APP_VERSION);
        for (const QString &n : {QStringLiteral("Quillbinder-%1-%2-x86_64.AppImage").arg(v, QStringLiteral(QB_APP_VERSION_LABEL)), QStringLiteral("quillbinder-%1-update.qbpkg").arg(v),
                                 QStringLiteral("SHA256SUMS.txt")})
            writeFile(dist.path() + QLatin1Char('/') + n, QByteArray());
        writeFile(dist.path() + QStringLiteral("/Quillbinder-%1-full-%2-x86_64.AppImage").arg(v, QStringLiteral(QB_APP_VERSION_LABEL)), QByteArray());
        for (const QString &n : {QStringLiteral("media-playback.qbpkg"), QStringLiteral("media-extra-formats.qbpkg"),
                                 QStringLiteral("media-all-formats.qbpkg"), QStringLiteral("themes-extra.qbpkg")}) {
            const QString built = kBuild + QStringLiteral("/addons-dist/") + n;
            if (!QFile::exists(built) || !QFile::copy(built, dist.path() + QLatin1Char('/') + n))
                writeFile(dist.path() + QLatin1Char('/') + n, QByteArray());
        }
        int rc = 0;
        const QString notes = run(QStringLiteral("python3"), {QStringLiteral("-I"), tools + QStringLiteral("release-notes.py"), dist.path()}, &rc);
        const int at = notes.indexOf(QLatin1String("| media-all-formats.qbpkg"));
        check(rc == 0 && notes.contains(QLatin1String("download **media-all-formats.qbpkg**")) && at > 0
                  && at < notes.indexOf(QLatin1String("| media-extra-formats.qbpkg")) && at < notes.indexOf(QLatin1String("| media-playback.qbpkg"))
                  && notes.contains(QLatin1String("one package for the commonly used formats"))
                  && notes.contains(QStringLiteral("| Quillbinder-%1-full-%2-x86_64.AppImage | **The full version**").arg(v, QStringLiteral(QB_APP_VERSION_LABEL)))
                  && notes.contains(QLatin1String("the full version, with all add-ons and the libraries they need included")),
              QStringLiteral("the release text names the one media package in the steps and first among the add-ons"), notes.left(2500));
        const QString news = run(QStringLiteral("python3"), {QStringLiteral("-I"), tools + QStringLiteral("release-notes.py"), QStringLiteral("--whats-new")}, &rc);
        // the newest section of docs/CHANGES.md (0.60.0: "one package"; later versions: their first item)
        const QString changes = QString::fromUtf8(readAll(kSource + QStringLiteral("/docs/CHANGES.md")));
        const QRegularExpressionMatch first = QRegularExpression(QStringLiteral("\\n- \\*\\*([^*]+)\\*\\*")).match(changes);
        check(rc == 0 && news.split(QLatin1Char('\n'), Qt::SkipEmptyParts).size() >= 3 && first.hasMatch()
                  && news.contains(first.captured(1)),
              QStringLiteral("the what's-new lines of this version come from the newest section of docs/CHANGES.md"), news);
        const QString readme = QString::fromUtf8(readAll(kSource + QStringLiteral("/README.md")));
        check(readme.contains(QLatin1String("media-all-formats.qbpkg")) && !readme.contains(QLatin1String("Known issues in 0.50"))
                  && readme.contains(QLatin1String("not made by or connected with Literature & Latte"))
                  && readme.contains(QLatin1String("contains no code, files or pictures of those programs")),
              QStringLiteral("README: the one media package, no old known issues, independent of Literature & Latte"));
        // download links at the top of the README (the public main page): this version's files, the newest release
        const QString tag = QStringLiteral("v%1").arg(QString::fromLatin1(QB_APP_VERSION_SHOWN).replace(QLatin1Char(' '), QLatin1Char('-')));
        const QString dl = QStringLiteral("https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/") + tag + QLatin1Char('/');
        const int download = readme.indexOf(QLatin1String("## Download"));
        check(download > 0 && download < 1500
                  && readme.contains(QStringLiteral("(%1Quillbinder-%2-x86_64.AppImage)").arg(dl, tag.mid(1)))
                  && readme.contains(QStringLiteral("(%1media-all-formats.qbpkg)").arg(dl))
                  && readme.contains(QStringLiteral("(%1Quillbinder-%2-full-%3-x86_64.AppImage)").arg(dl, QLatin1String(QB_APP_VERSION), tag.mid(int(qstrlen(QB_APP_VERSION)) + 2)))
                  && readme.contains(QLatin1String("**the full version**"))
                  && readme.contains(QLatin1String("(https://github.com/Shelin-boop/Quillbinder_Collection/releases/latest)")),
              QStringLiteral("README: near the top, direct links to this version's AppImage and media package, and to the newest release"),
              readme.mid(download, 900));
        const QString pubRel = QString::fromUtf8(readAll(kSource + QStringLiteral("/tools/publish-release.sh")));
        check(pubRel.contains(QLatin1String("PRE=(--prerelease=false --latest)")) && !pubRel.contains(QLatin1String("PRE=(--prerelease)")),
              QStringLiteral("program releases are published as full releases marked Latest, so .../releases/latest leads to the program"));
        const QString wf = QString::fromUtf8(readAll(kSource + QStringLiteral("/.github/workflows/release.yml")));
        check(wf.contains(QLatin1String("if: github.repository == 'Shelin-boop/Quillbinder_Collection'")),
              QStringLiteral("the release workflow runs in the project's own repository only"));
        check(QFileInfo::exists(kSource + QStringLiteral("/tests/media/README.txt")),
              QStringLiteral("the test media say where they come from"));
    }

    // ================================================================ downloads sorted by system (2026-11-12)
    {
        QTemporaryDir dist;
        const QString v = QStringLiteral(QB_APP_VERSION), label = QStringLiteral(QB_APP_VERSION_LABEL);
        for (const QString &n : {QStringLiteral("Quillbinder-%1-%2-x86_64.AppImage").arg(v, label), QStringLiteral("Quillbinder-%1-full-%2-x86_64.AppImage").arg(v, label),
                                 QStringLiteral("quillbinder-%1-update.qbpkg").arg(v), QStringLiteral("Quillbinder-%1-windows-x64.zip").arg(v),
                                 QStringLiteral("Quillbinder-%1-full-windows-x64.zip").arg(v), QStringLiteral("quillbinder-%1-windows-update.qbpkg").arg(v),
                                 QStringLiteral("SHA256SUMS.txt"), QStringLiteral("SHA256SUMS.txt.sig")})
            writeFile(dist.path() + QLatin1Char('/') + n, QByteArray());
        for (const QString &n : {QStringLiteral("media-all-formats.qbpkg"), QStringLiteral("themes-extra.qbpkg")}) {
            const QString built = kBuild + QStringLiteral("/addons-dist/") + n;
            if (!QFile::exists(built) || !QFile::copy(built, dist.path() + QLatin1Char('/') + n))
                writeFile(dist.path() + QLatin1Char('/') + n, QByteArray());
        }
        int rc = 0;
        const QString notes = run(QStringLiteral("python3"), {QStringLiteral("-I"), tools + QStringLiteral("release-notes.py"), dist.path()}, &rc);
        const qsizetype atLinux = notes.indexOf(QLatin1String("\n## Linux\n")), atWindows = notes.indexOf(QLatin1String("\n## Windows\n")),
                        atChecking = notes.indexOf(QLatin1String("\n## Checking the files"));
        check(rc == 0 && atLinux > 0 && atWindows > atLinux && atChecking > atWindows
                  && notes.indexOf(QStringLiteral("| Quillbinder-%1-full-windows-x64.zip | **The full version** for Windows").arg(v)) > atWindows
                  && notes.indexOf(QStringLiteral("| quillbinder-%1-update.qbpkg").arg(v)) > atLinux
                  && notes.indexOf(QStringLiteral("| quillbinder-%1-update.qbpkg").arg(v)) < atWindows
                  && notes.indexOf(QLatin1String("| SHA256SUMS.txt.sig")) > atChecking && notes.contains(QLatin1String("Linux (x86_64) and Windows 11 (64-bit)"))
                  && notes.contains(QLatin1String("(Apache License 2.0)")) && !notes.contains(QLatin1String("Zero-Clause")),
              QStringLiteral("the release text lists the files under a short headline per system, then the files for checking; "
                             "the license is the Apache License 2.0"),
              notes.left(3000));
        const QString test = run(QStringLiteral("python3"), {QStringLiteral("-I"), tools + QStringLiteral("release-notes.py"), dist.path(), QStringLiteral("--test")}, &rc);
        check(rc == 0 && test.contains(QLatin1String("\n## Windows\n")) && !test.contains(QLatin1String("## Source code"))
                  && !test.contains(QLatin1String("## What is new")),
              QStringLiteral("a test build's text has the files only (tools/test-release.sh)"));
        // the download page: one button per system for the full version, "comes later" for the others
        QTemporaryDir site;
        run(QStringLiteral("python3"), {QStringLiteral("-I"), tools + QStringLiteral("download-page.py"), dist.path(), QStringLiteral("v1.2.3-test"), site.path()}, &rc);
        const QString page = QString::fromUtf8(readAll(site.path() + QStringLiteral("/index.html")));
        check(rc == 0 && page.contains(QStringLiteral("releases/download/v1.2.3-test/Quillbinder-%1-full-%2-x86_64.AppImage").arg(v, label))
                  && page.contains(QStringLiteral("releases/download/v1.2.3-test/Quillbinder-%1-full-windows-x64.zip").arg(v))
                  && page.count(QLatin1String("class=\"button\"")) == 2 && page.count(QLatin1String("Comes later.")) == 1
                  && page.contains(QLatin1String("not made by or connected with Literature &amp; Latte"))
                  && page.contains(QLatin1String("data-system=\"android\"")),
              QStringLiteral("the download page: a button for each system's full version in the release, \"Comes later\" for the others"),
              page.left(1500));
        // 0.93.0: the page lists the official add-ons (from official-addons/*.json)
        check(page.contains(QLatin1String("id=\"official-addons\"")) && page.contains(QLatin1String("data-addon=\"translation-proofreading\""))
                  && page.contains(QLatin1String("<b>Translation and Proofreading</b>")) && page.contains(QLatin1String("for Linux, Windows, Android"))
                  && page.contains(QLatin1String("must not be sold")) && page.contains(QLatin1String("CC BY-NC 4.0"))
                  && page.contains(QLatin1String("blob/main/official-addons/translation-proofreading.json")),
              QStringLiteral("the download page lists the official add-ons: name, one line, the systems, the license note for data, a link to the entry"),
              page.mid(page.indexOf(QLatin1String("official-addons")), 800));
        const QString wf = QString::fromUtf8(readAll(kSource + QStringLiteral("/.github/workflows/release.yml")));
        const qsizetype job = wf.indexOf(QLatin1String("\n  download-page:\n"));
        const QString pageJob = job > 0 ? wf.mid(job) : QString();
        check(pageJob.contains(QLatin1String("needs: [version, publish]")) && pageJob.contains(QLatin1String("pages: write"))
                  && pageJob.contains(QLatin1String("tools/download-page.py")) && pageJob.contains(QLatin1String("actions/deploy-pages@"))
                  && !pageJob.contains(QLatin1String("contents: write")) && !pageJob.contains(QLatin1String("QB_SIGNING_KEY")),
              QStringLiteral("the release workflow puts the download page on GitHub Pages after publishing, without write access to the release"));
        check(QFileInfo::exists(kSource + QStringLiteral("/.github/workflows/test-release.yml"))
                  && QFileInfo(kSource + QStringLiteral("/tools/test-release.sh")).isExecutable(),
              QStringLiteral("test releases: the workflow and its script are there"));
        // the build scripts sorted by system (2026-11-12)
        check(QFileInfo::exists(kSource + QStringLiteral("/packaging/linux/build-appimage.sh")) && QFileInfo::exists(kSource + QStringLiteral("/packaging/windows/win-ci.sh"))
                  && QFileInfo::exists(kSource + QStringLiteral("/packaging/android/README.md")) && QFileInfo::exists(kSource + QStringLiteral("/packaging/README.md"))
                  && !QFileInfo::exists(kSource + QStringLiteral("/packaging/build-appimage.sh")) && !QFileInfo::exists(kSource + QStringLiteral("/tools/win-ci.sh")),
              QStringLiteral("the build scripts are in one folder per system (packaging/linux, packaging/windows, packaging/android)"));
    }

    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
