#pragma once
// The entry in the desktop's application menu (Linux; 0.94.0 made it a part of its own): the .desktop file, the icon
// and the MIME types of projects and packages, made and taken away again, and the AppImage moved to ~/Applications
// when the writer wants that. The running program and its updater know the AppImage through $APPIMAGE, which is set
// to the new place after a move (a program update, the ".before-update" copy and System Information follow it).
#include <QList>
#include <QString>

namespace qb {
namespace appmenu {

// A writing tool that runs on its own becomes an action of the menu entry.
struct ToolAction {
    QString id;   // "name-show"
    QString name; // as shown
    QString key;  // the tool command's key
};

struct Result {
    bool ok = false;           // the entry was made
    bool iconCopied = true;    // false: the entry works, without its icon
    bool moved = false;        // the AppImage was moved
    QString exec;              // what the entry starts
    QString error;             // why the entry could not be made
    QString moveError;         // why the AppImage could not be moved (the entry points to the old place then)
};

// The program runs from an AppImage ($APPIMAGE names a file).
bool runsFromAppImage();
QString appImagePath();
// ~/Applications
QString applicationsFolder();
bool isInApplications(const QString &file);
// A move to ~/Applications is possible and has a point: an AppImage that is not in that folder yet.
bool canMove();
// The entry's file exists.
bool hasEntry();
QString desktopFilePath();

// Makes (or renews) the entry; moveAppImage: first moves the AppImage to ~/Applications.
Result add(bool moveAppImage, const QList<ToolAction> &tools);
// Takes the entry away: the .desktop file, the icon and the MIME file. The AppImage stays where it is.
bool remove(QString *error);
// Moves the AppImage to ~/Applications and sets $APPIMAGE (and the settings that name it) to the new place; on a
// failure nothing is lost and the old file stays.
bool moveToApplications(QString *newPath, QString *error);

} // namespace appmenu
} // namespace qb
