#pragma once
// Android's own interface code (port, 2026-11-14). Compiled into the program on Android only (CMakeLists.txt, inside
// if(ANDROID)); the Linux and Windows programs do not contain it. On a desktop it is built only into the test program
// qb_mobile_tests (CMake option QB_MOBILE_PREVIEW, off unless asked for), which checks it in windows of a tablet's size.

#include <QString>
#include <QStringList>
#include <functional>

class QMainWindow;
class QWidget;

namespace qb {
namespace mobile {

// Fingers (android/MobileTouch.cpp). Call it once the application object exists, and again after the main window
// was made: the program's other filters then come after this one and never see a press that turns out to be a swipe.
//  - A finger dragged over a text, a list or a tree scrolls it (with a throw at the end), instead of selecting. A
//    tap still puts the cursor or chooses a line; a finger that rests for a moment and then moves drags as a mouse
//    does (it selects text); a mouse, a touch pad and a pen stay as they are.
//  - Two fingers moved apart or together zoom the part under them, as Ctrl + mouse wheel does (steps of 10 %; twice
//    as far apart: seven steps).
//  - A finger put on or close beside the line between two panes (binder and text, text and inspector, the parts of
//    the inspector, any other splitter) and moved across it takes the line along: the panes change their sizes.
// It also sets up the two parts below (once).
void installTouch();

// The handles of a text (android/MobileHandles.cpp; set up by installTouch()): the drop under the cursor shows for
// a few seconds after a tap into the text and goes at once with typing, a swipe or two fingers; no handle shows
// while its place in the text is out of sight (scrolled out of its pane); the handles follow a text that is moved
// as a whole inside what it stands in.
void installHandles();
void handlesAway(); // a finger began to swipe or two fingers came down: the drop under the cursor goes

// New windows while Android's keyboard program calls into the program (android/MobileWindows.cpp, which exists on
// Android only: the preview test on a desktop does not have it; set up by installTouch()). Qt 6.12.0 ends the program
// when a window is first drawn during such a call. Each window's Android surface is made right before its first
// picture instead, at a moment when no such call is on its way.
void installWindowGuard();

// Every window on a tablet with an external keyboard and mouse (android/MobileCare.cpp, Android 0.92.0). Call once,
// right after the program's style is set (main.cpp): it switches the shared windowcare::tabletMode() on and
//  - wraps the style: menus scroll instead of wrapping into columns over the menu bar; the line between two panes is
//    wider and shows a grip;
//  - gives a window that has no button at all a Close row (on a computer its title bar closes it), and puts the
//    contents of one of the program's own dialogs that cannot be as small as the screen into a part that scrolls,
//    its buttons below it in view; again when the usable area changes (the on-screen keyboard, a turned tablet);
//  - Android's Back closes the menu, the tip or the window in front instead of putting the app away;
//  - keeps the keys Ctrl, Shift, Alt and Meta for mouse and wheel events, which Qt 6.12.0 for Android sends without
//    them (Ctrl + wheel zooms, Ctrl + click and Shift + click choose several documents);
//  - a mouse pressed a few pixels beside the line between two panes takes the line along.
void installCare();

// Toolbars that fit the window's width (android/MobileBars.cpp). Call once for the main window, after it is shown.
// A row whose toolbars need more room than the window is wide hands its last toolbars to the next row (a third row
// and more when needed), at the start and whenever the window's width changes (a turned tablet). When that would
// take more than three rows, the Themes toolbar is switched off, and on again when there is room; a writer who
// switches it on by hand keeps it. The order the writer gave the toolbars stays.
void fitToolbars(QMainWindow *window);

// ---- Where the writer's work is kept (android/MobileFiles.cpp)

// The folders the program proposes on a tablet, inside the tablet's shared Documents folder.
struct Folders
{
    QString projects; // <Documents>/Quillbinder: new projects, the welcome project, where Open Project starts
    QString backups;  // <Documents>/Quillbinder/Backups
    QString exports;  // <Documents>/Quillbinder/Exports: File > Export > Files
    QString notes;    // <Documents>/Quillbinder/Scratch Pad: the Scratch Pad's notes
};
Folders foldersIn(const QString &documents);

// Moves the program's folder settings into `folders` where they are empty or name a folder inside one of `own` (the
// app's own folders, which no other app shows and Android deletes with the app); a folder the writer chose stays.
// With `access` false (the writer did not allow the access) it takes a setting inside the shared folder back to the
// program's standard. True when a setting changed (they are saved then).
bool settleFolders(const Folders &folders, const QStringList &own, bool access);

// What the window below needs from the system (Android's calls on a tablet, stand-ins in the preview test).
struct FilesSystem
{
    std::function<bool()> allowed; // may the program use the shared folders?
    std::function<void()> ask;     // opens Android's page (or question) where the writer allows it
    bool settingsPage = true;      // Android 11 and later: a page of Android's settings; before: a question
    QString documents;             // the tablet's shared Documents folder
};
// The window that says where projects and backups are kept and sends the writer to Android to allow it. It closes
// by itself when the writer comes back with the access given. True when the access is there afterwards.
bool askForFiles(QWidget *parent, const FilesSystem &system);

// Qt's file windows: `side` at their side (the folders that exist), and a window that would open in one of
// `standards` (the app's own folders, taken for want of a better one) opens in the projects folder instead.
void watchFileWindows(const Folders &folders, const QStringList &side, const QStringList &standards);

// ---- The start on a tablet (android/MobileStart.cpp)

// The spelling language of the first start for a tablet set to `locale` ("de_AT"), as the Windows program chooses
// it: the tablet's language when its dictionary comes with the program, else "" (the program's standard).
QString spellLanguageFor(const QString &locale);

// Copies the dictionary files of `from` (inside the APK) into `to` (where the spelling checker looks) when they are
// missing there or differ in size. Returns how many files it copied, -1 when one could not be copied.
int copyDictionaries(const QString &from, const QString &to);

// Android 0.92.0: installs the add-on packages of `from` (inside the APK: "assets:/full-addons", the translator) once
// per version, as the full version on Linux and Windows does (addons::installBundled), before the program loads its
// plug-ins. Returns how many packages it installed, 0 when there was nothing to do, -1 when one failed.
int installFullAddons(const QString &from);

// Saves the open project in full (texts, mind maps), as the program does on the way to another program. False when
// something could not be saved (the program's own notice tells the writer).
bool saveAllNow(QMainWindow *window);

// Call once the settings are read and before the main window is made (main.cpp): the folder settings (when the
// access to the shared folders was given at an earlier start) and the dictionaries out of the APK.
void tabletBeforeWindow();

// Call once after the main window is shown (main.cpp): the question about the shared folders, the save when
// Android puts the app away, the commands a tablet cannot have taken out of the menus.
void startTablet(QMainWindow *window);

// (Android only, MobileFiles.cpp; called by the two functions above)
void filesBeforeWindow();
void filesAfterWindow(QMainWindow *window);

// Files and folders handed to other apps (android/MobileOpen.cpp, Android only; set up by startTablet()): a folder
// the program "shows in the file manager" opens in the tablet's Files app, a file in the app Android has for it.
void installOpener();

} // namespace mobile
} // namespace qb
