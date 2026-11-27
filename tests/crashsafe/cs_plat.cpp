// The system test's platform layer (0.99.0): see cs_plat.h. The Linux half is what the cases did before, moved here
// unchanged; the Windows half (Q_OS_WIN) is compiled by the Windows run only and uses no POSIX header.
#include "cs_plat.h"

#include "cs.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>

#ifdef Q_OS_WIN
#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>
#include <windows.h>
#else
#include <csignal>
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace cs {
namespace plat {

QString programPath(const QString &buildDir)
{
#ifdef Q_OS_WIN
    const QByteArray forced = qgetenv("QB_CS_PROGRAM");
    if (!forced.isEmpty())
        return QString::fromLocal8Bit(forced);
    for (const char *sub : {"", "/bin", "/Release", "/RelWithDebInfo", "/Debug"}) {
        const QString p = buildDir + QString::fromLatin1(sub) + QStringLiteral("/quillbinder.exe");
        if (QFileInfo::exists(p))
            return p;
    }
    return buildDir + QStringLiteral("/quillbinder.exe");
#else
    return buildDir + QStringLiteral("/quillbinder");
#endif
}

QString pythonProgram()
{
#ifdef Q_OS_WIN
    return QStringLiteral("python");
#else
    return QStringLiteral("python3");
#endif
}

QString senderScript(const QString &sourceDir)
{
#ifdef Q_OS_WIN
    return sourceDir + QStringLiteral("/tests/crashsafe/sender_win.py");
#else
    return sourceDir + QStringLiteral("/tests/crashsafe/sender.py");
#endif
}

QStringList pythonIsolation() { return {QStringLiteral("-I")}; }

void prepareChild(QProcess *p)
{
#ifdef Q_OS_WIN
    Q_UNUSED(p);
#else
    p->setChildProcessModifier([] {
        struct rlimit rl = {0, 0};
        ::setrlimit(RLIMIT_CORE, &rl);
    });
#endif
}

#ifdef Q_OS_WIN
namespace {

struct WinFind {
    DWORD pid;
    QList<HWND> list;
};

BOOL CALLBACK collectWindows(HWND h, LPARAM lp)
{
    auto *f = reinterpret_cast<WinFind *>(lp);
    DWORD p = 0;
    GetWindowThreadProcessId(h, &p);
    if (p == f->pid && IsWindowVisible(h))
        f->list.append(h);
    return TRUE;
}

QList<HWND> windowsOf(qint64 pid)
{
    WinFind f{DWORD(pid), {}};
    EnumWindows(collectWindows, reinterpret_cast<LPARAM>(&f));
    return f.list;
}

QString titleOf(HWND h)
{
    wchar_t buf[512] = {};
    const int n = GetWindowTextW(h, buf, 511);
    return QString::fromWCharArray(buf, n);
}

int areaOf(HWND h)
{
    RECT r;
    if (!GetWindowRect(h, &r))
        return 0;
    return int(r.right - r.left) * int(r.bottom - r.top);
}

WORD vkOf(const QString &name, bool *ok)
{
    *ok = true;
    const QString n = name.toLower();
    if (n == QLatin1String("return") || n == QLatin1String("enter")) return VK_RETURN;
    if (n == QLatin1String("escape") || n == QLatin1String("esc")) return VK_ESCAPE;
    if (n == QLatin1String("tab")) return VK_TAB;
    if (n == QLatin1String("space")) return VK_SPACE;
    if (n == QLatin1String("end")) return VK_END;
    if (n == QLatin1String("home")) return VK_HOME;
    if (n == QLatin1String("delete")) return VK_DELETE;
    if (n == QLatin1String("backspace")) return VK_BACK;
    if (n == QLatin1String("up")) return VK_UP;
    if (n == QLatin1String("down")) return VK_DOWN;
    if (n == QLatin1String("left")) return VK_LEFT;
    if (n == QLatin1String("right")) return VK_RIGHT;
    if (n.size() == 1) {
        const QChar c = n.at(0);
        if (c.isLetterOrNumber() && c.unicode() < 128)
            return WORD(c.toUpper().unicode());
    }
    if (n.size() >= 2 && n.at(0) == QLatin1Char('f')) {
        bool num = false;
        const int k = n.mid(1).toInt(&num);
        if (num && k >= 1 && k <= 12)
            return WORD(VK_F1 + k - 1);
    }
    *ok = false;
    return 0;
}

void sendVk(WORD vk, bool up)
{
    INPUT in = {};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = vk;
    in.ki.dwFlags = (up ? KEYEVENTF_KEYUP : 0) | ((vk == VK_END || vk == VK_HOME || vk == VK_DELETE || vk == VK_UP || vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT) ? KEYEVENTF_EXTENDEDKEY : 0);
    SendInput(1, &in, sizeof(INPUT));
}

void sendMouse(DWORD flags)
{
    INPUT in = {};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = flags;
    SendInput(1, &in, sizeof(INPUT));
}

} // namespace
#endif

void killHard(qint64 pid)
{
#ifdef Q_OS_WIN
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, DWORD(pid));
    if (h) {
        TerminateProcess(h, 1);
        CloseHandle(h);
    }
#else
    ::kill(pid_t(pid), SIGKILL);
#endif
}

void terminate(qint64 pid)
{
#ifdef Q_OS_WIN
    for (HWND h : windowsOf(pid))
        PostMessageW(h, WM_CLOSE, 0, 0);
#else
    ::kill(pid_t(pid), SIGTERM);
#endif
}

void crashProcess(qint64 pid)
{
#ifdef Q_OS_WIN
    Q_UNUSED(pid);   // the program crashes itself: QB_TEST_CRASH (crashEnv)
#else
    ::kill(pid_t(pid), SIGSEGV);
#endif
}

QMap<QString, QString> crashEnv()
{
#ifdef Q_OS_WIN
    return {{QStringLiteral("QB_TEST_CRASH"), QStringLiteral("access")}};
#else
    return {};
#endif
}

bool alive(qint64 pid)
{
#ifdef Q_OS_WIN
    if (pid <= 0)
        return false;
    HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, DWORD(pid));
    if (!h)
        return false;
    const bool running = WaitForSingleObject(h, 0) == WAIT_TIMEOUT;
    CloseHandle(h);
    return running;
#else
    return pid > 0 && ::kill(pid_t(pid), 0) == 0;
#endif
}

bool isRoot()
{
#ifdef Q_OS_WIN
    return false;
#else
    return ::geteuid() == 0;
#endif
}

QStringList windowNames(qint64 pid)
{
#ifdef Q_OS_WIN
    QStringList out;
    for (HWND h : windowsOf(pid)) {
        const QString t = titleOf(h);
        if (!t.isEmpty())
            out << t;
    }
    return out;
#else
    return runTool(QStringLiteral("sh"), {QStringLiteral("-c"),
                                          QStringLiteral("for w in $(xdotool search --onlyvisible --pid %1); do xdotool getwindowname $w; done").arg(pid)})
        .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
#endif
}

QString findWindow(qint64 pid, const QString &titleRegex)
{
#ifdef Q_OS_WIN
    const QRegularExpression re(titleRegex);
    for (HWND h : windowsOf(pid))
        if (re.match(titleOf(h)).hasMatch())
            return QString::number(reinterpret_cast<quintptr>(h));
    return QString();
#else
    return runTool(QStringLiteral("xdotool"), {QStringLiteral("search"), QStringLiteral("--onlyvisible"), QStringLiteral("--pid"),
                                               QString::number(pid), QStringLiteral("--name"), titleRegex})
        .section(QLatin1Char('\n'), 0, 0)
        .trimmed();
#endif
}

void focusWindow(const QString &id)
{
#ifdef Q_OS_WIN
    HWND h = reinterpret_cast<HWND>(quintptr(id.toULongLong()));
    if (!h)
        return;
    // Windows lets a program take the foreground only after a key press of its own: a tap on Alt allows it
    keybd_event(VK_MENU, 0, 0, 0);
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
    if (IsIconic(h))
        ShowWindow(h, SW_RESTORE);
    BringWindowToTop(h);
    SetForegroundWindow(h);
#else
    runTool(QStringLiteral("xdotool"), {QStringLiteral("windowfocus"), QStringLiteral("--sync"), id});
#endif
}

bool focusMainWindow(qint64 processId)
{
#ifdef Q_OS_WIN
    HWND best = nullptr;
    int area = 0;
    for (HWND h : windowsOf(processId))
        if (areaOf(h) > area) {
            area = areaOf(h);
            best = h;
        }
    if (!best)
        return false;
    focusWindow(QString::number(reinterpret_cast<quintptr>(best)));
    settle(300);
    RECT r;
    GetWindowRect(best, &r);
    SetCursorPos(int((r.left + r.right) / 2), int((r.top + r.bottom) / 2));
    settle(100);
    sendMouse(MOUSEEVENTF_LEFTDOWN);
    sendMouse(MOUSEEVENTF_LEFTUP);
    settle(400);
    pressKey(QStringLiteral("ctrl+End"));
    settle(300);
    return true;
#else
    const QString pid = QString::number(processId);
    QString wid;
    int area = 0;
    for (const QString &id : runTool(QStringLiteral("xdotool"), {QStringLiteral("search"), QStringLiteral("--onlyvisible"), QStringLiteral("--pid"), pid})
                                 .split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QString g = runTool(QStringLiteral("xdotool"), {QStringLiteral("getwindowgeometry"), id});
        const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("Geometry: (\\d+)x(\\d+)")).match(g);
        if (m.hasMatch() && m.captured(1).toInt() * m.captured(2).toInt() > area) {
            area = m.captured(1).toInt() * m.captured(2).toInt();
            wid = id;
        }
    }
    if (wid.isEmpty())
        return false;
    // 0.64.2: raised first: the two "computers" of X-08 / X-10 share one screen without a window manager, and the
    // click below landed on the other program's notice lying above (X-10 B: it pressed "Close the Project Here"
    // there and the keys went to the other program)
    runTool(QStringLiteral("xdotool"), {QStringLiteral("windowraise"), wid});
    runTool(QStringLiteral("xdotool"), {QStringLiteral("windowfocus"), QStringLiteral("--sync"), wid});
    const QString g = runTool(QStringLiteral("xdotool"), {QStringLiteral("getwindowgeometry"), wid});
    const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("Geometry: (\\d+)x(\\d+)")).match(g);
    const int w = m.hasMatch() ? m.captured(1).toInt() : 800, h = m.hasMatch() ? m.captured(2).toInt() : 600;
    runTool(QStringLiteral("xdotool"), {QStringLiteral("mousemove"), QStringLiteral("--window"), wid, QString::number(w / 2), QString::number(h / 2),
                                        QStringLiteral("click"), QStringLiteral("1")});
    settle(400);
    runTool(QStringLiteral("xdotool"), {QStringLiteral("key"), QStringLiteral("ctrl+End")});
    settle(300);
    return true;
#endif
}

void pressKeys(const QStringList &chords)
{
#ifdef Q_OS_WIN
    for (const QString &chord : chords) {
        QList<WORD> mods;
        WORD key = 0;
        bool ok = true;
        for (const QString &part : chord.split(QLatin1Char('+'))) {
            const QString p = part.toLower();
            if (p == QLatin1String("ctrl") || p == QLatin1String("control"))
                mods << VK_CONTROL;
            else if (p == QLatin1String("shift"))
                mods << VK_SHIFT;
            else if (p == QLatin1String("alt"))
                mods << VK_MENU;
            else
                key = vkOf(part, &ok);
        }
        if (!ok || !key)
            continue;
        for (WORD m : mods)
            sendVk(m, false);
        sendVk(key, false);
        sendVk(key, true);
        for (int i = int(mods.size()) - 1; i >= 0; --i)
            sendVk(mods.at(i), true);
        settle(60);
    }
#else
    runTool(QStringLiteral("xdotool"), QStringList{QStringLiteral("key")} + chords);
#endif
}

void screenshot(const QString &file)
{
#ifdef Q_OS_WIN
    if (QScreen *s = QGuiApplication::primaryScreen())
        s->grabWindow(0).save(file);
#else
    runTool(QStringLiteral("import"), {QStringLiteral("-window"), QStringLiteral("root"), file});
#endif
}

bool dragMouse(const QPoint &a, const QPoint &b)
{
#ifdef Q_OS_WIN
    // the same movements as the xdotool call of the Linux harness, with the event loop running between them (the
    // program that is dragged in is this process)
    const QPoint mid = (a + b) / 2;
    SetCursorPos(a.x(), a.y());
    settle(200);
    sendMouse(MOUSEEVENTF_LEFTDOWN);
    settle(200);
    SetCursorPos(a.x() + 12, a.y() + 12);
    settle(300);
    SetCursorPos(mid.x(), mid.y());
    settle(300);
    SetCursorPos(b.x(), b.y());
    settle(400);
    sendMouse(MOUSEEVENTF_LEFTUP);
    settle(400);
    return true;
#else
    Q_UNUSED(a);
    Q_UNUSED(b);
    return false;   // Linux keeps its own code (xdotool in realDrag)
#endif
}

QString notOnThisPlatform(const QString &caseId)
{
#ifdef Q_OS_WIN
    // the first list [helper's choice]: what rests on LD_PRELOAD, root, mounts, POSIX permissions and links, shell
    // scripts as helper programs, or the X server's key table; the Windows run refines it
    static const QMap<QString, QString> list = {
        {QStringLiteral("C-23"), QStringLiteral("mounts and permissions of folders")},
        {QStringLiteral("C-34"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("C-35"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("C-36"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("C-38"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("C-39"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("C-40"), QStringLiteral("symbolic links and permissions")},
        {QStringLiteral("S-D06"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("S-I12"), QStringLiteral("POSIX permissions and shell scripts")},
        {QStringLiteral("S-A04"), QStringLiteral("shell scripts as helper programs")},
        {QStringLiteral("S-A05"), QStringLiteral("shell scripts as helper programs")},
        {QStringLiteral("S-W17"), QStringLiteral("the X server's key table")},
        {QStringLiteral("U-20"), QStringLiteral("shell scripts as helper programs")},
        {QStringLiteral("X-04"), QStringLiteral("folder owners and POSIX permissions")},
        {QStringLiteral("X-10"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("X-11"), QStringLiteral("fault injection by LD_PRELOAD")},
        {QStringLiteral("X-12"), QStringLiteral("fault injection by LD_PRELOAD")},
    };
    return list.value(caseId);
#else
    Q_UNUSED(caseId);
    return QString();
#endif
}

} // namespace plat
} // namespace cs
