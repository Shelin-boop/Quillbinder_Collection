#!/usr/bin/env python3
"""Outside key sender of the system test on Windows (0.99.0): the same arguments and the same log as sender.py, with
Windows' own SendInput instead of the X server's XTest.

  sender_win.py --text FILE --log FILE --profile slow|fast|wpmN --seconds N [--seed N] [--burst N] [--start-delay S]

Each character is sent as a Unicode key event (KEYEVENTF_UNICODE), so the keyboard layout of the runner does not matter
and no character is skipped; the keys go to the window that has the keyboard focus. The time of every press is
time.perf_counter_ns(): the clock behind std::chrono::steady_clock of the Microsoft compiler (QueryPerformanceCounter),
so the program side can measure key-to-screen as it does on Linux. Log lines: "k <index> <ns>" per press of the profile
run, "b <index> <ns>" per press of the burst, "end <sent>" at the end. Windows only: not started on Linux.
"""
import argparse
import ctypes
import random
import sys
import time

PROFILES = {
    # mean gap, SD (seconds), rollover share, overlap mean and SD (seconds): as sender.py
    "slow": (0.481, 0.123, 0.08, 0.030, 0.015),
    "fast": (0.122, 0.012, 0.50, 0.030, 0.015),
}

INPUT_KEYBOARD = 1
KEYEVENTF_KEYUP = 0x0002
KEYEVENTF_UNICODE = 0x0004
VK_RETURN = 0x0D


def make_api():
    from ctypes import wintypes
    ulong_p = ctypes.POINTER(ctypes.c_ulong)

    class KEYBDINPUT(ctypes.Structure):
        _fields_ = [("wVk", wintypes.WORD), ("wScan", wintypes.WORD), ("dwFlags", wintypes.DWORD),
                    ("time", wintypes.DWORD), ("dwExtraInfo", ulong_p)]

    class MOUSEINPUT(ctypes.Structure):   # only to give the union its true size
        _fields_ = [("dx", wintypes.LONG), ("dy", wintypes.LONG), ("mouseData", wintypes.DWORD),
                    ("dwFlags", wintypes.DWORD), ("time", wintypes.DWORD), ("dwExtraInfo", ulong_p)]

    class HARDWAREINPUT(ctypes.Structure):
        _fields_ = [("uMsg", wintypes.DWORD), ("wParamL", wintypes.WORD), ("wParamH", wintypes.WORD)]

    class UNION(ctypes.Union):
        _fields_ = [("ki", KEYBDINPUT), ("mi", MOUSEINPUT), ("hi", HARDWAREINPUT)]

    class INPUT(ctypes.Structure):
        _fields_ = [("type", wintypes.DWORD), ("u", UNION)]

    send = ctypes.windll.user32.SendInput
    send.argtypes = [wintypes.UINT, ctypes.POINTER(INPUT), ctypes.c_int]
    send.restype = wintypes.UINT
    return INPUT, send


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--text", required=True)
    ap.add_argument("--log", required=True)
    ap.add_argument("--profile", required=True, help="slow, fast, or wpmN")
    ap.add_argument("--seconds", type=float, default=60)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--burst", type=int, default=200)
    ap.add_argument("--start-delay", type=float, default=1.0)
    a = ap.parse_args()
    if sys.platform != "win32":
        print("sender_win.py runs on Windows only", file=sys.stderr)
        return 2
    with open(a.text, encoding="utf-8") as f:
        text = f.read()
    INPUT, send = make_api()

    def key(ch, up):
        i = INPUT()
        i.type = INPUT_KEYBOARD
        if ch == "\n":
            i.u.ki.wVk = VK_RETURN
            i.u.ki.dwFlags = KEYEVENTF_KEYUP if up else 0
        else:
            code = ord(ch)
            if code > 0xFFFF:   # outside the basic plane: two surrogates (the caller sends each)
                code = 0xFFFD
            i.u.ki.wScan = code
            i.u.ki.dwFlags = KEYEVENTF_UNICODE | (KEYEVENTF_KEYUP if up else 0)
        return i

    def press(ch, up):
        i = key(ch, up)
        send(1, ctypes.byref(i), ctypes.sizeof(INPUT))

    rnd = random.Random(a.seed)
    min_gap = 0.015 if a.profile.startswith("wpm") else 0.060
    if a.profile.startswith("wpm"):
        gap = 60.0 / (float(a.profile[3:]) * 5.0)
        mean, sd, roll, ov_mean, ov_sd = (gap, gap * 0.1, 0.50, min(0.030, gap * 0.4), 0.010)
    else:
        mean, sd, roll, ov_mean, ov_sd = PROFILES[a.profile]
    log = open(a.log, "w", encoding="utf-8", buffering=1)
    log.write("m 0\n")

    def wait_until(t):
        while True:
            d = t - time.perf_counter()
            if d <= 0:
                return
            time.sleep(d - 0.002 if d > 0.004 else 0)

    time.sleep(a.start_delay)
    t0 = time.perf_counter()
    end = t0 + a.seconds
    t = t0
    held = None   # (character, release time) of a key held into the next one (rollover)
    n = 0
    k = 0
    while k < len(text) and t < end:
        ch = text[k]
        nxt = text[k + 1] if k + 1 < len(text) else None
        gap = max(min_gap, rnd.gauss(mean, sd))
        wait_until(t)
        press(ch, False)
        log.write("k %d %d\n" % (k, time.perf_counter_ns()))
        n += 1
        if held is not None:
            wait_until(held[1])
            press(held[0], True)
            held = None
        rollover = nxt is not None and nxt != ch and rnd.random() < roll
        if rollover:
            overlap = min(0.100, max(0.005, rnd.gauss(ov_mean, ov_sd)))
            held = (ch, t + gap + overlap)
        else:
            wait_until(t + min(0.080, gap * 0.5))
            press(ch, True)
        t += gap
        k += 1
    if held is not None:
        wait_until(held[1])
        press(held[0], True)
    time.sleep(1.5)
    b = 0
    while k < len(text) and b < a.burst:
        press(text[k], False)
        log.write("b %d %d\n" % (k, time.perf_counter_ns()))
        press(text[k], True)
        b += 1
        k += 1
    log.write("end %d\n" % (n + b))
    log.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
