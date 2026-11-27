#pragma once
// The system test's own platform layer (0.99.0): what differs between Linux and Windows, in one place.
// Linux: signals, xdotool, ImageMagick's import, python3, the virtual display (the commands are the ones the cases
// always used). Windows (Q_OS_WIN; compiled by the Windows run, never on Linux): TerminateProcess, WM_CLOSE, SendInput,
// Qt's screen grab, python, the runner's own desktop (no Xvfb), the program as quillbinder.exe in its folder.
// Nothing here is called for a new reason on Linux: each function does what the code at its call site did before.
#include <QMap>
#include <QPoint>
#include <QString>
#include <QStringList>

class QProcess;

namespace cs {
namespace plat {

constexpr bool isWindows =
#ifdef Q_OS_WIN
    true;
#else
    false;
#endif

// the program under test: <build dir>/quillbinder (Windows: quillbinder.exe in the build dir, or in bin/ or Release/ of it;
// QB_CS_PROGRAM names it outright)
QString programPath(const QString &buildDir);
// the Python that runs the outside senders ("python3"; Windows: "python")
QString pythonProgram();
// the key sender's script in the source: sender.py (Windows: sender_win.py)
QString senderScript(const QString &sourceDir);
// arguments in front of the script ("-I" on Linux; the same on Windows)
QStringList pythonIsolation();

// a program started by a case: no core files (Linux); nothing on Windows
void prepareChild(QProcess *p);

// ending a program: the hard end (SIGKILL; TerminateProcess), the ordinary request to end (SIGTERM; WM_CLOSE to its
// windows), a real crash (SIGSEGV on Linux; on Windows the program's own QB_TEST_CRASH, see crashEnv)
void killHard(qint64 pid);
void terminate(qint64 pid);
void crashProcess(qint64 pid);
// the environment that makes the program crash by itself (Windows: QB_TEST_CRASH=access); empty on Linux
QMap<QString, QString> crashEnv();
bool alive(qint64 pid);
bool isRoot();   // Linux: the user is root; Windows: false

// the titles of the visible windows of a program (xdotool search / getwindowname; EnumWindows)
QStringList windowNames(qint64 pid);
// the id of the program's visible window whose title matches the regular expression (the first), "" for none
QString findWindow(qint64 pid, const QString &titleRegex);
void focusWindow(const QString &id);
// the program's main window (the largest visible one): raised, focused, the pointer in its middle, a click, then ctrl+End
bool focusMainWindow(qint64 pid);
// keys to the focused window, one chord after the other: "ctrl+s", "Return", "Escape", "Tab", "space", "ctrl+End"
void pressKeys(const QStringList &chords);
inline void pressKey(const QString &chord) { pressKeys({chord}); }
// the whole screen as a picture file (import -window root; Windows: Qt's grab of the primary screen)
void screenshot(const QString &file);
// a mouse drag from a to b in global coordinates, with pauses (xdotool; SendInput); true when it ran
bool dragMouse(const QPoint &a, const QPoint &b);

// cases that make no sense on the platform: the reason, or "" (Windows: LD_PRELOAD faults, root and mounts, the
// layout tool of the X server ...; Linux: none)
QString notOnThisPlatform(const QString &caseId);

} // namespace plat
} // namespace cs
