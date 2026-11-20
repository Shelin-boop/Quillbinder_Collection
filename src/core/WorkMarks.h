#pragma once
// 0.92.0: the shown lock. Which documents a device has open or has changed since it opened the project, written
// beside the project's lock so that another device (through a sync program) can mark them in its binder. A hint,
// not a guard: nothing is blocked.
//
// One small file per device, "<lock file>-work-<machine key>.json" (settings/lock-work-<key>.json; for a project
// in Scrivener format also quillbinder.lock-work-<key>.json in the .scriv). It is written only when its list changes
// (never per keystroke), so a quiet project gives a sync program nothing to carry; it goes when the project is
// closed. Its sign of life is the lock's while that device holds the lock (the lock is renewed every few minutes
// anyway); a device that lost the lock and keeps working renews the file itself. A list without a sign of life
// for longer than a lock counts as left over is not shown.
#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace workmarks {

struct Entry {
    QString device;     // the device's name as in its lock
    QString key;        // syncsafe::machineKey() of that device
    QDateTime opened;   // UTC: when it opened the project
    QDateTime alive;    // UTC: its last sign of life (the file's own, or its lock's when that is newer)
    QStringList docs;   // ids of the documents it has open or has changed
};

// This device's file beside a lock file.
QString fileFor(const QString &lockFile);
// Writes this device's list (alive = now). False when the file could not be written.
bool write(const QString &lockFile, const QDateTime &opened, const QStringList &docs);
void removeOwn(const QString &lockFile);
// The lists of the other devices beside this lock file that still show a sign of life.
QList<Entry> others(const QString &lockFile, qint64 staleAfterSecs);

} // namespace workmarks
} // namespace qb
