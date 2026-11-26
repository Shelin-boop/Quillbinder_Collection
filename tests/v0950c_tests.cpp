// Scripted run for version 0.95.0 (shared code, third part of the run's to-do list): the rules of user profiles
// (item 95), the built-in theme and profile of the writer's look (items 96 and 104), the standard colors (item 98) and the
// set of shortcuts of the keyboard layout (item 103). No window here: ui/Profiles.*, the theme list, AppSettings and
// ui/Commands.* are checked on their own; the window's side is qb_v0950d_tests.
//
//   xvfb-run -a ./qb_v0950c_tests /tmp/shots
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "ui/AppSettings.h"
#include "ui/ColorPickButton.h"
#include "ui/Commands.h"
#include "ui/KeyboardLayout.h"
#include "ui/Layouts.h"
#include "ui/Profiles.h"
#include "ui/Theme.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>

#include <cstdio>

using namespace qb;

static int failures = 0;
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

static QString writeText(const QString &path, const QByteArray &bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(bytes);
    return path;
}

static void clearSettings()
{
    QSettings s;
    s.clear();
    s.sync();
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", (home.path() + QLatin1String("/h")).toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_KEYBOARD_LAYOUT", "us");
    qputenv("QB_COLOR_SCHEME", "none");
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0950cTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0950cTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    QTemporaryDir work;
    AppSettings &as = AppSettings::instance();

    // ============================================================ item 96: Shelin's Dark Theme
    {
        const QList<theme::Entry> list = theme::available();
        check(!list.isEmpty() && list.first().id == QLatin1String("shelins-dark") && list.first().name == QLatin1String("Shelin's Dark Theme"),
              QStringLiteral("the theme list starts with Shelin's Dark Theme"), list.isEmpty() ? QString() : list.first().name);
        theme::Theme t;
        check(theme::load(QStringLiteral("shelins-dark"), &t), QStringLiteral("the theme loads"));
        auto col = [&t](theme::Role r) { return t.colors.value(r).name(); };
        check(col(theme::Role::BinderBackground) == QLatin1String("#2a2a2a") && col(theme::Role::BinderText) == QLatin1String("#ffffff")
                  && col(theme::Role::EditorLink) == QLatin1String("#1f64c8") && col(theme::Role::EditorMargin) == QLatin1String("#6f6f6f")
                  && col(theme::Role::EditorPaper) == QLatin1String("#fee4a6") && col(theme::Role::EditorText) == QLatin1String("#000000")
                  && col(theme::Role::InspectorBackground) == QLatin1String("#4c4431") && col(theme::Role::InspectorFields) == QLatin1String("#1c1f22")
                  && col(theme::Role::NotesPaper) == QLatin1String("#222222") && col(theme::Role::SynopsisPaper) == QLatin1String("#576e78"),
              QStringLiteral("its ten colors are the writer's"));
        check(t.fonts.value(theme::FontRole::CombinedTitle).family == QLatin1String("Cinzel")
                  && qFuzzyCompare(t.fonts.value(theme::FontRole::CombinedTitle).size, 11.0),
              QStringLiteral("its font is Cinzel 11 for the titles of the combined text"));
        theme::Theme dark;
        theme::load(QStringLiteral("dark"), &dark);
        check(!t.followDesktop && t.colors.value(theme::Role::Window) == dark.colors.value(theme::Role::Window)
                  && t.colors.value(theme::Role::Highlight) == dark.colors.value(theme::Role::Highlight)
                  && t.colors.value(theme::Role::Button) == dark.colors.value(theme::Role::Button),
              QStringLiteral("it does not follow the desktop; the window's other colors are those of Dark Mode"));
        check(theme::isRemovableBuiltIn(QStringLiteral("shelins-dark")) && !theme::isRemovableBuiltIn(QStringLiteral("dark")),
              QStringLiteral("only this built-in theme can be deleted"));
        QString err;
        check(theme::removeUserTheme(QStringLiteral("shelins-dark"), &err), QStringLiteral("Delete works for it"), err);
        bool found = false;
        for (const theme::Entry &e : theme::available())
            found = found || e.id == QLatin1String("shelins-dark");
        check(!found && theme::available().first().id == QLatin1String("system"), QStringLiteral("it is gone from the list afterwards"));
        // "stays away after updates": the choice is in the settings, which an update keeps
        check(QSettings().value(QStringLiteral("themes/removed")).toStringList().contains(QStringLiteral("shelins-dark")),
              QStringLiteral("the deletion is kept in the program's settings"));
        QSettings().remove(QStringLiteral("themes/removed")); // for the profile checks below
    }

    // ============================================================ item 95: the system's color scheme and the first start
    {
        qputenv("QB_COLOR_SCHEME", "dark");
        check(profiles::systemThemeId() == QLatin1String("dark"), QStringLiteral("the system's scheme (dark) gives the theme id dark"));
        qputenv("QB_COLOR_SCHEME", "light");
        check(profiles::systemThemeId() == QLatin1String("light"), QStringLiteral("... light gives light"));
        qputenv("QB_COLOR_SCHEME", "none");
        check(profiles::systemThemeId().isEmpty(), QStringLiteral("... a system that does not tell gives no id"));

        clearSettings();
        qputenv("QB_COLOR_SCHEME", "dark");
        as.theme = QStringLiteral("system");
        as.load();
        check(as.theme == QLatin1String("dark"), QStringLiteral("a new installation starts dark when the system is dark"), as.theme);
        clearSettings();
        qputenv("QB_COLOR_SCHEME", "light");
        as.theme = QStringLiteral("system");
        as.load();
        check(as.theme == QLatin1String("light"), QStringLiteral("... light when the system is light"), as.theme);
        clearSettings();
        qputenv("QB_COLOR_SCHEME", "none");
        as.theme = QStringLiteral("system");
        as.load();
        check(as.theme == QLatin1String("system"), QStringLiteral("... today's default where the system does not tell"), as.theme);
        // a program of an earlier version (it wrote the mark of 0.21.2) keeps its theme whatever the system says
        clearSettings();
        qputenv("QB_COLOR_SCHEME", "dark");
        QSettings().setValue(QStringLiteral("app/standard0212"), true);
        QSettings().setValue(QStringLiteral("appearance/theme"), QStringLiteral("sepia"));
        as.theme = QStringLiteral("system");
        as.load();
        check(as.theme == QLatin1String("sepia"), QStringLiteral("a program that was used before keeps its theme"), as.theme);
        qputenv("QB_COLOR_SCHEME", "none");
        clearSettings();
        as.load();
        check(profiles::active() == QLatin1String("Default"), QStringLiteral("a program without a mark starts with the profile Default"), profiles::active());
    }

    // ============================================================ the list
    {
        const QStringList names = profiles::names();
        check(names.size() >= 2 && names.at(0) == QLatin1String("Shelin's") && names.at(1) == QLatin1String("Default"),
              QStringLiteral("the list: Shelin's at the top, then Default"), names.join(QLatin1Char(',')));
        check(profiles::isBuiltIn(QStringLiteral("Shelin's")) && !profiles::isBuiltIn(QStringLiteral("Default")),
              QStringLiteral("Shelin's is the built-in one"));
        check(!profiles::canDelete(QStringLiteral("Default")) && profiles::canDelete(QStringLiteral("Shelin's")),
              QStringLiteral("Default cannot be deleted, Shelin's can"));
        check(profiles::nameInUse(QStringLiteral("shelin's")) && profiles::nameInUse(QStringLiteral("DEFAULT")) && !profiles::nameInUse(QStringLiteral("Mine")),
              QStringLiteral("names are compared without regard to case"));
        check(profiles::uniqueName(QStringLiteral("Default")) == QLatin1String("Default 2"), QStringLiteral("a free name is made from a taken one"));
    }

    // ============================================================ what a profile holds
    {
        check(profiles::ownsKey(QStringLiteral("editing/smartQuotes")) && profiles::ownsKey(QStringLiteral("app/language"))
                  && profiles::ownsKey(QStringLiteral("window/geometry")) && profiles::ownsKey(QStringLiteral("window/mainSplit"))
                  && profiles::ownsKey(QStringLiteral("toolbars/bar-main")) && profiles::ownsKey(QStringLiteral("backup/dir"))
                  && profiles::ownsKey(QStringLiteral("keyboard/shortcuts")) && profiles::ownsKey(QStringLiteral("keyboard/layoutSet"))
                  && profiles::ownsKey(QStringLiteral("standardColors/labels")) && profiles::ownsKey(QStringLiteral("appearance/theme"))
                  && profiles::ownsKey(QStringLiteral("tools/speech-transcription/model")) && profiles::ownsKey(QStringLiteral("zoomDefault/text")),
              QStringLiteral("options, language, window, toolbars, backups, keyboard, standard colors, theme and tools are in a profile"));
        check(!profiles::ownsKey(QStringLiteral("app/forename")) && !profiles::ownsKey(QStringLiteral("app/email"))
                  && !profiles::ownsKey(QStringLiteral("app/deviceName")) && !profiles::ownsKey(QStringLiteral("app/recent"))
                  && !profiles::ownsKey(QStringLiteral("app/lastDir")) && !profiles::ownsKey(QStringLiteral("app/welcomeShown"))
                  && !profiles::ownsKey(QStringLiteral("app/standard0212")) && !profiles::ownsKey(QStringLiteral("toolbars/split0910"))
                  && !profiles::ownsKey(QStringLiteral("profile/active")) && !profiles::ownsKey(QStringLiteral("themes/removed"))
                  && !profiles::ownsKey(QStringLiteral("tools/translation-proofreading/licenseAccepted"))
                  && !profiles::ownsKey(QStringLiteral("tools/speech-transcription/threads")),
              QStringLiteral("the person's and the computer's keys, the marks of updates and the accepted licenses are not"));
    }

    // ============================================================ switching: a round trip
    {
        clearSettings();
        as.load();
        QSettings s;
        s.setValue(QStringLiteral("app/standard0212"), true);
        s.setValue(QStringLiteral("app/forename"), QStringLiteral("Test"));
        s.setValue(QStringLiteral("app/autosave"), 7);
        s.setValue(QStringLiteral("editing/smartQuotes"), false);
        s.setValue(QStringLiteral("appearance/theme"), QStringLiteral("sepia"));
        s.setValue(QStringLiteral("backup/dir"), work.path() + QLatin1String("/backups"));
        s.setValue(QStringLiteral("window/mainSplit"), QVariantList{300, 900, 400});
        s.setValue(QStringLiteral("app/language"), QStringLiteral("de"));
        s.setValue(QStringLiteral("keyboard/layoutSet"), QStringLiteral("de"));
        s.sync();
        // the writer's own layouts: two files in the layouts folder
        const QString lay = layouts::userDir();
        writeText(lay + QLatin1String("/Mine.json"), R"({"name":"Mine","state":{"binder":true}})");
        writeText(lay + QLatin1String("/Other.json"), R"({"name":"Other","state":{"binder":false}})");
        // an update: the settings of the earlier version are the profile Default
        check(profiles::active() == QLatin1String("Default"), QStringLiteral("after an update the settings belong to Default"));
        QString err;
        check(profiles::createFromCurrent(QStringLiteral("Travel"), &err), QStringLiteral("Save Settings as New Profile"), err);
        check(profiles::active() == QLatin1String("Travel") && profiles::exists(QStringLiteral("Travel")), QStringLiteral("the new profile is the one in use"));
        check(QFile::exists(profiles::userDir() + QLatin1String("/Default.qbprofile")), QStringLiteral("Default was written as a file when it was left"));
        check(!profiles::createFromCurrent(QStringLiteral("travel"), &err) && !err.isEmpty(), QStringLiteral("a name that is taken is refused (case ignored)"), err);

        // change the settings under "Travel"
        s.setValue(QStringLiteral("app/autosave"), 3);
        s.setValue(QStringLiteral("editing/ellipsis"), true);
        s.setValue(QStringLiteral("editing/zzOnlyHere"), 5);
        s.sync();
        QFile::remove(lay + QLatin1String("/Other.json")); // a layout deleted under this profile
        // switch to Default: its values come back, a key it lacks is gone, the person's keys stay
        profiles::SwitchReport r = profiles::switchTo(QStringLiteral("Default"));
        QSettings t;
        check(r.ok && profiles::active() == QLatin1String("Default"), QStringLiteral("the switch to Default works"), r.error);
        check(t.value(QStringLiteral("app/autosave")).toInt() == 7 && !t.value(QStringLiteral("editing/smartQuotes")).toBool()
                  && t.value(QStringLiteral("appearance/theme")).toString() == QLatin1String("sepia"),
              QStringLiteral("... with its options and theme"));
        check(!t.contains(QStringLiteral("editing/zzOnlyHere")) && !t.value(QStringLiteral("editing/ellipsis")).toBool(),
              QStringLiteral("... a key the profile does not have is removed, one it has takes its value"));
        check(t.value(QStringLiteral("app/forename")).toString() == QLatin1String("Test") && t.value(QStringLiteral("app/standard0212")).toBool(),
              QStringLiteral("... the person's name and the marks of updates are untouched"));
        check(QFile::exists(lay + QLatin1String("/Other.json")) && QFile::exists(lay + QLatin1String("/Mine.json")),
              QStringLiteral("... the layouts are those of the profile (the one deleted under Travel is back)"));
        check(!r.languageChanged, QStringLiteral("same language: no restart is asked"));
        // and back
        r = profiles::switchTo(QStringLiteral("Travel"));
        QSettings u;
        check(r.ok && u.value(QStringLiteral("app/autosave")).toInt() == 3 && u.value(QStringLiteral("editing/ellipsis")).toBool(),
              QStringLiteral("the switch back brings Travel's own changes"), r.error);
        check(!QFile::exists(lay + QLatin1String("/Other.json")) && QFile::exists(lay + QLatin1String("/Mine.json")),
              QStringLiteral("... and its layouts"));
        // language
        QSettings().setValue(QStringLiteral("app/language"), QStringLiteral("en"));
        QSettings().sync();
        r = profiles::switchTo(QStringLiteral("Default"));
        check(r.ok && r.languageChanged, QStringLiteral("a profile with another language says that the language needs a restart"));
        check(!profiles::switchTo(QStringLiteral("Nowhere")).ok, QStringLiteral("a profile that does not exist is refused"));
        profiles::switchTo(QStringLiteral("Travel"));

        // rename, copy, delete
        check(profiles::rename(QStringLiteral("Travel"), QStringLiteral("Journey"), &err) && profiles::active() == QLatin1String("Journey"),
              QStringLiteral("Rename keeps the profile in use"), err);
        check(!profiles::rename(QStringLiteral("Default"), QStringLiteral("Home"), &err) && !profiles::rename(QStringLiteral("Shelin's"), QStringLiteral("S"), &err),
              QStringLiteral("Default and the built-in profile cannot be renamed"));
        check(profiles::duplicate(QStringLiteral("Journey"), QStringLiteral("Copy"), &err) && profiles::exists(QStringLiteral("Copy")),
              QStringLiteral("Copy"), err);
        check(!profiles::remove(QStringLiteral("Journey"), &err), QStringLiteral("the profile in use cannot be deleted"));
        check(!profiles::remove(QStringLiteral("Default"), &err), QStringLiteral("Default cannot be deleted"));
        check(profiles::remove(QStringLiteral("Copy"), &err) && !profiles::exists(QStringLiteral("Copy")), QStringLiteral("Delete"), err);
    }

    // ============================================================ the built-in profile (item 104)
    {
        QString err;
        profiles::Content c;
        check(profiles::read(QStringLiteral("Shelin's"), &c, &err) && !c.values.isEmpty(), QStringLiteral("the built-in profile can be read"), err);
        auto v = [&c](const char *k) { return c.values.value(QLatin1String(k)); };
        check(v("appearance/theme").toString() == QLatin1String("shelins-dark"), QStringLiteral("its theme is Shelin's Dark Theme"), v("appearance/theme").toString());
        check(v("editing/newFontFamily").toString() == QLatin1String("Sitka") && v("editing/newAlign").toString() == QLatin1String("justify")
                  && v("playback/backend").toString() == QLatin1String("ffmpeg") && v("toolbars/iconSize").toInt() == 22
                  && v("backup/keep").toInt() == 50 && v("app/autosave").toInt() == 2,
              QStringLiteral("its values are those of the file"));
        check(!c.values.contains(QStringLiteral("window/geometry")) && !c.values.contains(QStringLiteral("window/state"))
                  && !c.values.contains(QStringLiteral("keyboard/shortcuts")) && !c.values.contains(QStringLiteral("app/forename"))
                  && !c.values.contains(QStringLiteral("backup/dir")) && !c.values.contains(QStringLiteral("app/deviceName")),
              QStringLiteral("what was left out on purpose is not there (window place, shortcuts, name, paths)"));
        check(c.values.value(QStringLiteral("tools/speech-transcription/model")).toString() == QLatin1String("medium")
                  && c.values.value(QStringLiteral("tools/translation-proofreading/corrector")).toString() == QLatin1String("coedit-large"),
              QStringLiteral("the models of the two add-ons are in it"));
        const QVariantList split = c.values.value(QStringLiteral("window/mainSplit")).toList();
        check(split.size() == 3 && split[0].toInt() == 325 && split[1].toInt() == 1030 && split[2].toInt() == 557,
              QStringLiteral("mainSplit is 325, 1030, 557"));
        check(!c.hasLayouts, QStringLiteral("it brings no layouts: the writer's layouts stay"));
        // switching to it
        QSettings s;
        s.setValue(QStringLiteral("keyboard/layoutSet"), QStringLiteral("de"));
        s.setValue(QStringLiteral("keyboard/shortcuts"), QByteArray("{\"bold\":\"Ctrl+Alt+B\"}"));
        s.setValue(QStringLiteral("editing/ellipsis"), true);
        s.setValue(QStringLiteral("editing/zzOnlyHere"), 5);
        s.sync();
        const profiles::SwitchReport r = profiles::switchTo(QStringLiteral("Shelin's"));
        QSettings t;
        check(r.ok && profiles::active() == QLatin1String("Shelin's"), QStringLiteral("the switch to Shelin's works"), r.error);
        check(t.value(QStringLiteral("appearance/theme")).toString() == QLatin1String("shelins-dark")
                  && t.value(QStringLiteral("editing/newFontFamily")).toString() == QLatin1String("Sitka")
                  && !t.value(QStringLiteral("editing/ellipsis")).toBool() && !t.contains(QStringLiteral("editing/zzOnlyHere")),
              QStringLiteral("its values are in the settings, what it lacks went back to the standard"));
        check(t.value(QStringLiteral("keyboard/layoutSet")).toString() == QLatin1String("de"),
              QStringLiteral("the keyboard set of the layout is kept (the profile leaves it out on purpose)"));
        check(!t.contains(QStringLiteral("keyboard/shortcuts")), QStringLiteral("shortcuts the profile leaves out are the standard ones"));
        // delete it: it stays away
        profiles::switchTo(QStringLiteral("Default"));
        check(profiles::remove(QStringLiteral("Shelin's"), &err), QStringLiteral("the built-in profile can be deleted"), err);
        check(!profiles::names().contains(QStringLiteral("Shelin's")) && profiles::names().first() == QLatin1String("Default"),
              QStringLiteral("it is gone from the list"));
        check(!profiles::exists(QStringLiteral("Shelin's")) && !profiles::switchTo(QStringLiteral("Shelin's")).ok,
              QStringLiteral("it stays away (a later start does not bring it back)"));
        check(QSettings().value(QStringLiteral("profile/removed")).toStringList().contains(QStringLiteral("Shelin's")),
              QStringLiteral("the deletion is in the program's settings"));
        QSettings().remove(QStringLiteral("profile/removed"));
        check(profiles::names().first() == QLatin1String("Shelin's"), QStringLiteral("(without that entry it is listed again)"));
    }

    // ============================================================ export and import
    {
        QString err;
        QSettings s;
        profiles::switchTo(QStringLiteral("Journey"));
        s.setValue(QStringLiteral("app/autosave"), 9);
        s.setValue(QStringLiteral("backup/dir"), QStringLiteral("/this/folder/does/not/exist"));
        s.setValue(QStringLiteral("app/citationsApp"), QStringLiteral("/usr/bin/some-program"));
        s.setValue(QStringLiteral("app/forename"), QStringLiteral("Test"));
        s.sync();
        // an own theme goes with the export
        theme::Theme own;
        own.name = QStringLiteral("My Test Theme");
        own.colors.insert(theme::Role::Window, QColor(10, 20, 30));
        own.colors.insert(theme::Role::Text, QColor(240, 240, 240));
        const QString themeId = theme::saveUserTheme(own, &err);
        s.setValue(QStringLiteral("appearance/theme"), themeId);
        s.sync();
        const QString file = work.path() + QLatin1String("/out/Journey.qbprofile");
        QDir().mkpath(work.path() + QLatin1String("/out"));
        check(profiles::exportTo(QStringLiteral("Journey"), file, &err) && QFile::exists(file), QStringLiteral("Export writes a file"), err);
        check(profiles::isProfileFile(file), QStringLiteral("... that is a profile file"));
        const QStringList places = profiles::placesIn(file);
        check(places.contains(QStringLiteral("backup/dir")) && places.contains(QStringLiteral("app/citationsApp")),
              QStringLiteral("the folders and programs it names are found (they are asked about)"), places.join(QLatin1Char(',')));
        check(!places.contains(QStringLiteral("sharing/docConverter")) && !places.contains(QStringLiteral("app/forename")),
              QStringLiteral("a plain word and the person's name are no places"));
        QSettings text(file, QSettings::IniFormat);
        check(!text.contains(QStringLiteral("app/forename")), QStringLiteral("the person's name is not in the file"));
        // import without the places
        QFile::remove(theme::userThemeDir() + QLatin1String("/") + themeId.mid(5) + QLatin1String(".json"));
        const QString name = profiles::importFrom(file, places, &err);
        check(!name.isEmpty() && name != QLatin1String("Journey") && profiles::exists(name), QStringLiteral("Import adds a profile under a free name"), err + name);
        profiles::Content c;
        profiles::read(name, &c);
        check(!c.values.contains(QStringLiteral("backup/dir")) && !c.values.contains(QStringLiteral("app/citationsApp"))
                  && c.values.value(QStringLiteral("app/autosave")).toInt() == 9,
              QStringLiteral("the places the writer refused are not taken, the rest is"));
        check(QFile::exists(theme::userThemeDir() + QLatin1String("/") + themeId.mid(5) + QLatin1String(".json")),
              QStringLiteral("the own theme of the file is made again when it is missing"));
        const QString name2 = profiles::importFrom(file, {}, &err);
        profiles::Content c2;
        profiles::read(name2, &c2);
        check(!name2.isEmpty() && c2.values.value(QStringLiteral("backup/dir")).toString() == QLatin1String("/this/folder/does/not/exist"),
              QStringLiteral("places the writer agreed to are taken"));
        check(profiles::active() == QLatin1String("Journey"), QStringLiteral("an import does not switch"));
        // an old options file becomes a profile
        const QString opt = work.path() + QLatin1String("/old.qboptions");
        {
            QSettings o(opt, QSettings::IniFormat);
            o.setValue(QStringLiteral("quillbinderOptions/version"), 1);
            o.setValue(QStringLiteral("app/autosave"), 4);
            o.setValue(QStringLiteral("window/geometry"), QByteArray("x"));
            o.sync();
        }
        const QString fromOptions = profiles::importFrom(opt, {}, &err);
        profiles::Content c3;
        profiles::read(fromOptions, &c3);
        check(fromOptions == QLatin1String("old") && c3.values.value(QStringLiteral("app/autosave")).toInt() == 4,
              QStringLiteral("an options file can be imported as a profile (named after the file)"), fromOptions + err);
        const QString junk = writeText(work.path() + QLatin1String("/junk.qbprofile"), "hello");
        check(profiles::importFrom(junk, {}, &err).isEmpty() && !err.isEmpty(), QStringLiteral("a file that is no profile is refused"), err);
        // a write that fails leaves the old file: the target is a folder
        QDir().mkpath(work.path() + QLatin1String("/folderfile.qbprofile"));
        check(!profiles::exportTo(QStringLiteral("Journey"), work.path() + QLatin1String("/folderfile.qbprofile"), &err) && !err.isEmpty(),
              QStringLiteral("an export that cannot be written says so"));
    }

    // ============================================================ sizes fitted to the screen
    {
        const QList<int> shelins = {325, 1030, 557}; // pixels of a 4K screen at scale 2
        QList<int> a = profiles::fitColumns(shelins, 1912);
        check(a[0] == 325 && a[1] + a[0] + a[2] == 1912, QStringLiteral("a width as large as the profile's keeps the sizes"), QStringLiteral("%1 %2 %3").arg(a[0]).arg(a[1]).arg(a[2]));
        QList<int> b = profiles::fitColumns(shelins, 1280);
        check(b[0] + b[1] + b[2] == 1280 && b[1] >= 320 && b[0] >= 140 && b[2] >= 140 && b[0] < 325 && b[2] < 557,
              QStringLiteral("a smaller screen gets smaller columns that fit"), QStringLiteral("%1 %2 %3").arg(b[0]).arg(b[1]).arg(b[2]));
        QList<int> c = profiles::fitColumns(shelins, 800);
        check(c[0] + c[1] + c[2] == 800 && c[1] >= 320 && c[0] >= 100 && c[2] >= 100, QStringLiteral("a tablet's width still leaves the editor its room"),
              QStringLiteral("%1 %2 %3").arg(c[0]).arg(c[1]).arg(c[2]));
        QList<int> d = profiles::fitColumns({300, 900, 0}, 600);
        check(d[2] == 0 && d[0] + d[1] == 600, QStringLiteral("a hidden column stays hidden"), QStringLiteral("%1 %2 %3").arg(d[0]).arg(d[1]).arg(d[2]));
        QList<int> e = profiles::fitColumns({260, 800, 300}, 4000);
        check(e[0] + e[1] + e[2] == 4000 && e[0] <= 1600 && e[2] <= 1600, QStringLiteral("a very wide screen: the side columns stay below 40 %"),
              QStringLiteral("%1 %2 %3").arg(e[0]).arg(e[1]).arg(e[2]));
    }

    // ============================================================ item 98: standard colors
    {
        check(as.textSwatchColors() == AppSettings::standardSwatches() && as.textSwatchColors().size() == 6,
              QStringLiteral("the swatches are the six standard colors at first"));
        check(ColorPickButton::palette().at(0).first == Project::tr("Red") || ColorPickButton::palette().at(0).first == QObject::tr("Red"),
              QStringLiteral("the text color popup's first swatch is red at first"));
        as.standardLabels = Project::builtInLabels();
        as.standardLabels[0].name = QStringLiteral("Hero");
        as.standardLabels[0].color = QColor(1, 2, 3);
        as.textSwatches = AppSettings::standardSwatches();
        as.textSwatches[1] = QColor(10, 200, 30);
        as.highlightSwatches = AppSettings::standardSwatches();
        as.highlightSwatches[3] = QColor(250, 0, 250);
        as.barTextColor = QColor(11, 22, 33);
        as.save();
        QSettings s;
        check(s.contains(QStringLiteral("standardColors/labels")) && s.contains(QStringLiteral("standardColors/textSwatches"))
                  && s.contains(QStringLiteral("standardColors/highlightSwatches")),
              QStringLiteral("what differs from the standard is saved"));
        as.standardLabels.clear();
        as.textSwatches.clear();
        as.highlightSwatches.clear();
        as.load();
        check(as.standardLabels.size() == 6 && as.standardLabels[0].name == QLatin1String("Hero") && as.standardLabels[0].color == QColor(1, 2, 3)
                  && as.textSwatches.size() == 6 && as.textSwatches[1] == QColor(10, 200, 30)
                  && as.highlightSwatches[3] == QColor(250, 0, 250) && as.barTextColor == QColor(11, 22, 33),
              QStringLiteral("they are read again"));
        check(ColorPickButton::palette(ColorPickButton::TextColor).at(1).second == QColor(10, 200, 30)
                  && ColorPickButton::palette(ColorPickButton::TextColor).at(1).first == QLatin1String("#0ac81e")
                  && ColorPickButton::palette(ColorPickButton::Highlight).at(3).second == QColor(250, 0, 250)
                  && ColorPickButton::palette(ColorPickButton::Highlight).at(1).second == AppSettings::standardSwatches().at(1),
              QStringLiteral("the two popups use their own lists (a changed color is named by its code)"));
        // a new project starts with them, an existing one keeps its labels
        auto p1 = Project::create(work.path(), QStringLiteral("Before"), nullptr);
        check(p1 && p1->labels.size() == 6 && p1->labels[0].name == QLatin1String("Hero") && p1->labels[0].color == QColor(1, 2, 3),
              QStringLiteral("a new project starts with the standard labels"));
        const QString path = p1 ? p1->folderPath() : QString();
        as.standardLabels = Project::builtInLabels();
        as.standardLabels[0].name = QStringLiteral("Villain");
        as.save();
        check(p1 && p1->labels[0].name == QLatin1String("Hero"), QStringLiteral("a project that exists keeps its labels"));
        auto p2 = Project::create(work.path(), QStringLiteral("After"), nullptr);
        check(p2 && p2->labels[0].name == QLatin1String("Villain") && p2->labels.size() == 6, QStringLiteral("the next new project has the new ones"));
        as.resetStandardColors();
        as.save();
        QSettings s2;
        check(!s2.contains(QStringLiteral("standardColors/labels")) && !s2.contains(QStringLiteral("standardColors/textSwatches")),
              QStringLiteral("Defaults: nothing but the standard is kept"));
        auto p3 = Project::create(work.path(), QStringLiteral("Third"), nullptr);
        check(p3 && p3->labels[0].name == Project::builtInLabels()[0].name && p3->labels[0].color == QColor(214, 69, 65)
                  && as.barTextColor == QColor(200, 30, 30) && as.barHighlightColor == QColor(255, 230, 0),
              QStringLiteral("... the built-in six labels and the buttons' colors are back"));
        // the colors belong to the profile
        as.textSwatches = AppSettings::standardSwatches();
        as.textSwatches[0] = QColor(5, 6, 7);
        as.save();
        profiles::Content now = profiles::captureCurrent();
        check(now.values.contains(QStringLiteral("standardColors/textSwatches")), QStringLiteral("the standard colors are part of a profile"));
        as.resetStandardColors();
        as.save();
    }

    // ============================================================ item 103: the set of the keyboard layout
    {
        QAction *align = new QAction(QStringLiteral("Align Left"));
        align->setObjectName(QStringLiteral("alignLeft"));
        align->setShortcut(QKeySequence(QStringLiteral("Ctrl+{"), QKeySequence::PortableText));
        QAction *plain = new QAction(QStringLiteral("Bold"));
        plain->setObjectName(QStringLiteral("bold"));
        plain->setShortcut(QKeySequence(QStringLiteral("Ctrl+B"), QKeySequence::PortableText));
        QSettings().setValue(QStringLiteral("keyboard/layoutSet"), QStringLiteral("us"));
        QSettings().remove(QStringLiteral("keyboard/shortcuts"));
        commands::clear();
        commands::add(align, QStringLiteral("Test"));
        commands::add(plain, QStringLiteral("Test"));
        commands::reload();
        const QString before = commands::find(QStringLiteral("alignLeft"))->standard.toString(QKeySequence::PortableText);
        const auto diff = commands::layoutSetChanges(QStringLiteral("de"));
        check(diff.contains(QStringLiteral("alignLeft")) && !diff.contains(QStringLiteral("bold")) && diff.size() == 1,
              QStringLiteral("only the commands of the German set would change"), before);
        // an own shortcut is kept
        commands::setCustom({{QStringLiteral("bold"), QStringLiteral("Ctrl+Shift+B")}});
        commands::setLayoutSet(QStringLiteral("de"));
        check(commands::layoutSet() == QLatin1String("de") && QSettings().value(QStringLiteral("keyboard/layoutSet")).toString() == QLatin1String("de"),
              QStringLiteral("the set is chosen and kept in the settings"));
        check(commands::find(QStringLiteral("alignLeft"))->standard.toString(QKeySequence::PortableText) == QLatin1String("Alt+Shift+L")
                  && align->shortcut().toString(QKeySequence::PortableText) == QLatin1String("Alt+Shift+L"),
              QStringLiteral("the German set gives Align Left a key without AltGr, on the command and on the action"));
        check(plain->shortcut().toString(QKeySequence::PortableText) == QLatin1String("Ctrl+Shift+B"),
              QStringLiteral("a shortcut the writer set stays"));
        commands::setLayoutSet(QStringLiteral("us"));
        check(align->shortcut().toString(QKeySequence::PortableText) == QLatin1String("Ctrl+{")
                  && commands::find(QStringLiteral("alignLeft"))->usual.toString(QKeySequence::PortableText) == QLatin1String("Ctrl+{"),
              QStringLiteral("and back to the usual set"));
        // a profile with the other set: reload() takes the standard from the settings
        QSettings().setValue(QStringLiteral("keyboard/layoutSet"), QStringLiteral("de"));
        commands::reload();
        check(commands::layoutSet() == QLatin1String("de") && align->shortcut().toString(QKeySequence::PortableText) == QLatin1String("Alt+Shift+L"),
              QStringLiteral("reload() follows the set of the settings (a switch of profile)"));
        commands::setLayoutSet(QStringLiteral("us"));
        commands::setCustom({});
        commands::clear();
        delete align;
        delete plain;
    }

    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
