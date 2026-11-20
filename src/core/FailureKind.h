#pragma once
// 0.92.0 (audit finding QBA-F06): the kind of a failed write, carried as a value beside its text. Before, the plain
// reason of a failed save ("The disk is full.") was chosen by English words in the error text ("no space left",
// "permission denied"); a system that words its errors in another language got "The files could not be written."
// The kind is noted where the write fails (from the system's error number) and read by the window that tells the
// failure. One slot for the save that runs: Project's save clears it at its start, and the first kind noted stays
// (the first failure is the cause).
#include <cerrno>

namespace qb {
namespace failure {

enum class Kind { None, DiskFull, ReadOnly, NoRights, DriveError, FolderGone };

inline Kind &slot()
{
    static Kind k = Kind::None;
    return k;
}
inline void clear() { slot() = Kind::None; }
inline Kind last() { return slot(); }
inline void note(Kind k)
{
    if (slot() == Kind::None)
        slot() = k;
}
// The kind of a system error number; None for any other (the text then decides, as before).
inline Kind ofErrno(int e)
{
    switch (e) {
    case ENOSPC:
#ifdef EDQUOT
    case EDQUOT:
#endif
        return Kind::DiskFull;
    case EROFS:
        return Kind::ReadOnly;
    case EACCES:
    case EPERM:
        return Kind::NoRights;
    case EIO:
        return Kind::DriveError;
    default:
        return Kind::None;
    }
}
inline void noteErrno(int e) { note(ofErrno(e)); }

} // namespace failure
} // namespace qb
