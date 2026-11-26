#pragma once
// 0.95.0 (item 95): user profiles. A theme is only the look (colors, fonts); a profile holds everything else of the
// program's settings: the language, all options, the toolbars and their places, the window's size and place, the panels
// shown, the dividers, the keyboard set and shortcuts, the backups and the backup folder, the update checks, the
// standard colors, the saved layouts of View > Layout, and the theme it names. The program's settings file is the
// working copy of the active profile; a profile is also kept as a file of its own, <data folder>/profiles/<name>.qbprofile
// (the settings file's INI form: the keys as the program writes them), written when the profile is left, when the program
// ends, and by "Save" in Manage Profiles.
//
// Not part of a profile (they belong to the person or the computer): the author's name and address, the name of this
// computer, the recent projects and the last folders, the places of the welcome project, what was done once at an
// update (migration marks), the licenses a writer accepted, the number of threads and the graphics card of the writing
// tools. A profile file that lacks a key that is part of profiles gives the program's standard for it, except
// keyboard/layoutSet (the keyboard set of item 79 stays as it is).
//
// "Default" is the profile every program starts with (a program of an earlier version: its settings at the update; a new
// installation: the standard, light or dark as the system is set). It cannot be deleted. "Shelin's" is built in (item
// 104), stands at the top of the list and can be deleted; it then stays away after updates.
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace qb {
namespace profiles {

QString defaultName();    // "Default"
QString builtInName();    // "Shelin's"
QString userDir();        // <data folder>/profiles

// ---- the system's color scheme (the platform part: Qt's style hints on Linux, Windows and Android)
// "light" or "dark" (ids of the built-in themes) as the system is set, or an empty string where it does not tell. The
// environment variable QB_COLOR_SCHEME ("light", "dark", "none") replaces the answer (tests).
QString systemThemeId();

// ---- the list
QStringList names();                   // "Shelin's" (unless deleted), "Default", then the others by name
bool exists(const QString &name);
bool isBuiltIn(const QString &name);   // the built-in profile (also after it was changed and saved: it keeps its place)
bool canDelete(const QString &name);   // everything but "Default"
bool nameInUse(const QString &name, const QString &except = QString());
QString uniqueName(const QString &wanted);
QString active();                      // the profile the settings belong to ("Default" at first)
void setActive(const QString &name);   // only the mark (profile/active)

// ---- what a profile holds
struct Content
{
    QVariantMap values;                    // settings keys ("appearance/theme") and their values
    QMap<QString, QByteArray> layouts;     // the writer's own layouts (file name -> JSON)
    bool hasLayouts = false;               // false: the layouts on this computer stay as they are
    QString themeId;                       // appearance/theme, for the list
    QString themeJson;                     // an own theme the profile names ("user:..."), so that an export carries it
};
// Is the settings key part of a profile?
bool ownsKey(const QString &key);
// The program's settings as a profile now (the layouts and the own theme included).
Content captureCurrent();
// Reads a profile (a built-in one that was never saved: its built-in values); false when there is none or it is damaged.
bool read(const QString &name, Content *out, QString *error = nullptr);
bool write(const QString &name, const Content &content, QString *error = nullptr);

// ---- doing
// Writes the settings into the file of the active profile.
bool saveActive(QString *error = nullptr);
struct SwitchReport
{
    bool ok = false;
    QString error;
    QString from, to;
    bool languageChanged = false;   // takes effect at the next start
    QStringList places;             // keys the profile gave that were not taken (a place that was refused)
};
// Saves the active profile, then makes the settings those of `name`: every key that belongs to profiles is replaced (a
// key the profile lacks goes back to the standard), the layouts are replaced when the profile has layouts, and the
// profile is marked active. The window is not touched: MainWindow::switchProfile reads everything anew afterwards.
// `refusedPlaces`: keys of the profile whose value names a program, folder or address that the writer did not agree to.
SwitchReport switchTo(const QString &name, const QStringList &refusedPlaces = QStringList());
// A new profile with the settings as they are now; it becomes the active one (the profile left keeps its own state).
bool createFromCurrent(const QString &name, QString *error = nullptr);
bool remove(const QString &name, QString *error = nullptr);   // not the active profile, not "Default"
bool rename(const QString &name, const QString &newName, QString *error = nullptr);
bool duplicate(const QString &name, const QString &newName, QString *error = nullptr);
// Export: a file (.qbprofile) with the profile's keys, layouts and own theme. Import: a profile file or an options file of
// File > Options (.qboptions, an older kind of file); the profile gets the name from the file (or a free one).
bool exportTo(const QString &name, const QString &file, QString *error = nullptr);
// The keys of a file that name a program, folder, file or address (the writer is asked about them first).
QStringList placesIn(const QString &file);
QString importFrom(const QString &file, const QStringList &refusedPlaces, QString *error = nullptr);
// Whether a file is a profile or an options file
bool isProfileFile(const QString &file);

// ---- sizes fitted to the screen (item 95: a smaller screen, a tablet)
// The widths of the three columns scaled to the width there is now (proportionally); the side columns at least `minSide`
// and at most 40 % of the width, the editor column at least `minCenter` (when the width allows). A hidden column (size 0)
// stays 0.
QList<int> fitColumns(const QList<int> &sizes, int total, int minSide = 140, int minCenter = 320);

// ---- the built-in profile
QString builtInText();   // its values in the INI form of the settings file

} // namespace profiles
} // namespace qb
