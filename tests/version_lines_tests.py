#!/usr/bin/env python3
"""Suite verlines (0.93.0): a version line per system in CMakeLists.txt (APP_VERSION for Linux, APP_VERSION_WINDOWS,
APP_VERSION_ANDROID) and everything that reads it, offline: the lines and the build's pick, the shell lines of the
scripts and workflows, tools/update-feed.py, tools/release-notes.py and tools/download-page.py on a copy of the
repository whose Windows and Android lines have other last numbers, and the version jobs of the Windows and Android
release workflows (their own last number, a Linux release of the same first two numbers).

  python3 -I tests/version_lines_tests.py
"""
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FAILS = 0


def check(name, ok, detail=""):
    global FAILS
    print(("PASS " if ok else "FAIL ") + name + ("" if ok else f": {detail}"))
    if not ok:
        FAILS += 1


def read(rel, root=ROOT):
    with open(os.path.join(root, rel), encoding="utf-8") as fh:
        return fh.read()


def run(args, cwd=None):
    return subprocess.run(args, capture_output=True, text=True, cwd=cwd)


cm = read("CMakeLists.txt")
line = lambda name, text=cm: (re.search(r'^set\(%s\s+"([^"]*)"\)' % name, text, re.M) or [None, None])[1]
lin, win, andr = line("APP_VERSION"), line("APP_VERSION_WINDOWS"), line("APP_VERSION_ANDROID")

# ---- CMakeLists.txt
check("three lines, 0.x.y each", all(v and re.fullmatch(r"\d+\.\d+\.\d+", v) for v in (lin, win, andr)), (lin, win, andr))
check("the functional level (the first two numbers) is the same on every system",
      lin.split(".")[:2] == win.split(".")[:2] == andr.split(".")[:2], (lin, win, andr))
check("Linux keeps APP_VERSION as the first line that starts with set(APP_VERSION \"",
      cm.index('set(APP_VERSION "') < cm.index('set(APP_VERSION_WINDOWS "') < cm.index('set(APP_VERSION_ANDROID "'))
check("the build takes the system's own line before the label and the shown version are made",
      re.search(r'^if\(WIN32\)\n  set\(APP_VERSION "\$\{APP_VERSION_WINDOWS\}"\)\nelseif\(ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "Android" OR CMAKE_TOOLCHAIN_FILE MATCHES "/android_\[\^/\]\+/lib/cmake/Qt6/"\)\n'
                r'(  #[^\n]*\n)*  set\(APP_VERSION "\$\{APP_VERSION_ANDROID\}"\)\nendif\(\)', cm, re.M)
      and cm.index("APP_VERSION_ANDROID}") < cm.index("set(APP_VERSION_SHOWN"))
check("a build with another system's number stops (a check after project())",
      "FATAL_ERROR \"Windows build: APP_VERSION must be APP_VERSION_WINDOWS" in cm
      and "FATAL_ERROR \"Android build: APP_VERSION must be APP_VERSION_ANDROID" in cm)
check("the override lines are indented, so the Linux scripts' sed finds only the Linux line",
      all(not l.startswith('set(APP_VERSION "$') for l in cm.splitlines()))


# ---- the shell lines of the scripts and workflows
def sed_of(rel):
    """The sed expressions that read a version line in a file: [(expression, name)]."""
    return re.findall(r"sed -n '(s/\^set\((APP_VERSION[A-Z_]*) [^']*)'", read(rel))


def sed_value(expr):
    r = run(["sed", "-n", expr, os.path.join(ROOT, "CMakeLists.txt")])
    return r.stdout.strip()


expect = {
    "packaging/linux/build-appimage.sh": "APP_VERSION", "packaging/linux/build-update.sh": "APP_VERSION",
    "packaging/linux/build-full-appimage.sh": "APP_VERSION", "tools/publish-release.sh": "APP_VERSION",
    ".github/workflows/release.yml": "APP_VERSION",
    "packaging/windows/win-ci.sh": "APP_VERSION_WINDOWS", ".github/workflows/release-windows.yml": "APP_VERSION_WINDOWS",
    "packaging/android/android-ci.sh": "APP_VERSION_ANDROID", ".github/workflows/release-android.yml": "APP_VERSION_ANDROID",
}
for rel, name in expect.items():
    found = [(e, n) for e, n in sed_of(rel) if n in ("APP_VERSION", "APP_VERSION_WINDOWS", "APP_VERSION_ANDROID")]
    want = {"APP_VERSION": lin, "APP_VERSION_WINDOWS": win, "APP_VERSION_ANDROID": andr}[name]
    check("%s reads %s" % (rel, name), len(found) >= 1 and all(n == name and sed_value(e) == want for e, n in found), found)

# ---- the Python tools, on a copy whose Windows and Android lines have other last numbers
work = tempfile.mkdtemp(prefix="qbverlines.")
try:
    copy = os.path.join(work, "repo")
    for d in ("tools", "docs", os.path.join("packaging", "update-feed")):
        shutil.copytree(os.path.join(ROOT, d), os.path.join(copy, d), ignore=shutil.ignore_patterns("dev-notes", "*.png", "__pycache__"))
    level = ".".join(lin.split(".")[:2])
    wv, av = level + ".7", level + ".9"
    text = cm.replace('set(APP_VERSION_WINDOWS "%s")' % win, 'set(APP_VERSION_WINDOWS "%s")' % wv) \
              .replace('set(APP_VERSION_ANDROID "%s")' % andr, 'set(APP_VERSION_ANDROID "%s")' % av)
    with open(os.path.join(copy, "CMakeLists.txt"), "w", encoding="utf-8") as fh:
        fh.write(text)
    changes = read("docs/CHANGES.md")
    with open(os.path.join(copy, "docs", "CHANGES.md"), "w", encoding="utf-8") as fh:
        fh.write(changes.replace("## %s " % lin, ("## Windows %s experimental (2026-12-01)\n\n- **A Windows fix of its own.** Text.\n\n"
                                  "## Android %s experimental (2026-12-02)\n\n- **An Android fix of its own.** Text.\n\n## %s ") % (wv, av, lin), 1))
    dist = os.path.join(work, "dist")
    os.makedirs(dist)
    for name in ("Quillbinder-%s-windows-x64-setup.exe" % wv, "Quillbinder-%s-android-arm64.apk" % av,
                 "Quillbinder-%s-beta-x86_64.AppImage" % lin):
        open(os.path.join(dist, name), "wb").write(b"x")

    def feed(system):
        out = os.path.join(work, "feed-%s.json" % system)
        r = run([sys.executable, "-I", os.path.join(copy, "tools", "update-feed.py"), "--dist", dist, "--tag", "v1.2.3-test", "--repo", "a/b",
                 "--out", out, "--system", system])
        return r, (json.load(open(out, encoding="utf-8"))["systems"][system]["program"] if r.returncode == 0 else {})

    for system, want, own in (("linux-x86_64", lin, None), ("windows-x64", wv, "A Windows fix of its own."),
                              ("android-arm64", av, "An Android fix of its own.")):
        r, prog = feed(system)
        check("update-feed.py --system %s names version %s" % (system, want), r.returncode == 0 and prog.get("version") == want, r.stderr[-300:] + str(prog)[:200])
        if own:
            check("... and the headline of that system's own section first", prog.get("whatsNew", [None])[0] == own, prog.get("whatsNew"))
    # the real repository: nothing changes for Linux
    r = run([sys.executable, "-I", os.path.join(ROOT, "tools", "release-notes.py"), "--whats-new"])
    r2 = run([sys.executable, "-I", os.path.join(copy, "tools", "release-notes.py"), "--whats-new"])
    check("release-notes.py --whats-new (Linux) is the same on the copy with other Windows and Android lines", r.returncode == 0 and r.stdout == r2.stdout and r.stdout.strip(), r.stderr)
    w = run([sys.executable, "-I", os.path.join(copy, "tools", "release-notes.py"), "--whats-new", "--system", "windows"])
    a = run([sys.executable, "-I", os.path.join(copy, "tools", "release-notes.py"), "--whats-new", "--system", "android"])
    check("release-notes.py --whats-new --system windows|android: the system's own version and its own section first",
          w.stdout.splitlines()[:1] == ["A Windows fix of its own"] and a.stdout.splitlines()[:1] == ["An Android fix of its own"], (w.stdout[:200], a.stdout[:200]))

    # the download page: each system shows its newest release, whatever the order of the older ones
    def page(*also):
        names = {}
        site = os.path.join(work, "site")
        shutil.rmtree(site, ignore_errors=True)
        args = [sys.executable, "-I", os.path.join(copy, "tools", "download-page.py"), os.path.join(work, "main"), "v%s-beta" % lin, site]
        for i, (tag, files) in enumerate(also):
            d = os.path.join(work, "also%d" % i)
            shutil.rmtree(d, ignore_errors=True)
            os.makedirs(d)
            for f in files:
                open(os.path.join(d, f), "wb").write(b"")
            args += ["--also", d, tag]
        r = run(args)
        return r, (read("index.html", site) if r.returncode == 0 else "")

    os.makedirs(os.path.join(work, "main"))
    open(os.path.join(work, "main", "Quillbinder-%s-full-beta-x86_64.AppImage" % lin), "wb").close()
    old_w = ("v%s.0-windows-experimental" % level, ["Quillbinder-%s.0-full-experimental-windows-x64.zip" % level])
    new_w = ("v%s-windows-experimental" % wv, ["Quillbinder-%s-full-experimental-windows-x64.zip" % wv])
    apk = ("v%s-android-experimental" % av, ["Quillbinder-%s-experimental-android-arm64.apk" % av])
    for order in ((new_w, old_w, apk), (old_w, new_w, apk)):
        r, html = page(*order)
        check("download page: Windows shows its newest release (%s), Android its own, Linux the release itself" % wv,
              r.returncode == 0 and "releases/download/v%s-windows-experimental/Quillbinder-%s-full" % (wv, wv) in html
              and "v%s.0-windows-experimental" % level not in html and "releases/download/v%s-android-experimental/" % av in html
              and "v%s-beta/Quillbinder-%s-full-beta-x86_64.AppImage" % (lin, lin) in html
              and "version %s windows experimental" % wv in html, r.stderr[-300:])
finally:
    shutil.rmtree(work, ignore_errors=True)

# ---- the Windows and Android release workflows: their own last number, a Linux release of the same first two numbers
for name, label, var in (("release-windows.yml", "Windows", "APP_VERSION_WINDOWS"), ("release-android.yml", "Android", "APP_VERSION_ANDROID")):
    wf = read(".github/workflows/" + name)
    check("%s: the version comes from %s, the tag from it" % (name, var),
          ("VERSION=$(sed -n 's/^set(%s " % var) in wf and 'TAG="v$VERSION-%s-experimental"' % label.lower() in wf)
    check("%s: a Linux release with the same first two numbers and its full version is enough (not the same last number)" % name,
          'LEVEL=$(echo "$VERSION" | cut -d. -f1,2)' in wf and 'startswith(\\"v$LEVEL.\\")' in wf)
    check("%s: no test of the Linux release against $VERSION as a whole" % name, "v$VERSION\"" not in re.sub(r'TAG="v\$VERSION-[a-z]+-experimental"', "", wf))

print(f"{FAILS} failure(s)")
sys.exit(1 if FAILS else 0)
