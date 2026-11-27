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
# 0.99.0: the label is one setting per system, too (Linux's own, Windows', Android's)
lab, labw, laba = line("APP_VERSION_LABEL"), line("APP_VERSION_LABEL_WINDOWS"), line("APP_VERSION_LABEL_ANDROID")
dash = lambda l: ("-" + l) if l else ""   # the part of a name or a tag that a label makes ("" for a normal release)

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


check("a label per system: Linux's APP_VERSION_LABEL first, then APP_VERSION_LABEL_WINDOWS and APP_VERSION_LABEL_ANDROID",
      None not in (lab, labw, laba) and cm.index('set(APP_VERSION_LABEL "') < cm.index('set(APP_VERSION_LABEL_WINDOWS "') < cm.index('set(APP_VERSION_LABEL_ANDROID "'),
      (lab, labw, laba))
check("the build takes the system's own label as APP_VERSION_LABEL before the shown version is made",
      re.search(r'^if\(WIN32\)\n  set\(APP_VERSION_LABEL "\$\{APP_VERSION_LABEL_WINDOWS\}"\)\nelseif\(ANDROID OR [^\n]*\)\n  set\(APP_VERSION_LABEL "\$\{APP_VERSION_LABEL_ANDROID\}"\)\nendif\(\)', cm, re.M)
      and cm.index('set(APP_VERSION_LABEL "${APP_VERSION_LABEL_ANDROID}")') < cm.index("set(APP_VERSION_SHOWN"))
check("the override lines of the label are indented, so the Linux scripts' sed finds only Linux's label",
      all(not l.startswith('set(APP_VERSION_LABEL "$') for l in cm.splitlines()))


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
                 "Quillbinder-%s%s-x86_64.AppImage" % (lin, dash(lab))):
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
        wl = {"linux-x86_64": lab, "windows-x64": labw, "android-arm64": laba}[system]
        check("... and its own label (%r)" % wl, r.returncode == 0 and prog.get("label") == wl, prog.get("label"))
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
        args = [sys.executable, "-I", os.path.join(copy, "tools", "download-page.py"), os.path.join(work, "main"), "v%s%s" % (lin, dash(lab)), site]
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
    open(os.path.join(work, "main", "Quillbinder-%s-full%s-x86_64.AppImage" % (lin, dash(lab))), "wb").close()
    old_w = ("v%s.0-windows%s" % (level, dash(labw)), ["Quillbinder-%s.0-full%s-windows-x64.zip" % (level, dash(labw))])
    new_w = ("v%s-windows%s" % (wv, dash(labw)), ["Quillbinder-%s-full%s-windows-x64.zip" % (wv, dash(labw))])
    apk = ("v%s-android%s" % (av, dash(laba)), ["Quillbinder-%s%s-android-arm64.apk" % (av, dash(laba))])
    for order in ((new_w, old_w, apk), (old_w, new_w, apk)):
        r, html = page(*order)
        check("download page: Windows shows its newest release (%s), Android its own, Linux the release itself" % wv,
              r.returncode == 0 and "releases/download/v%s-windows%s/Quillbinder-%s-full" % (wv, dash(labw), wv) in html
              and "v%s.0-windows%s" % (level, dash(labw)) not in html and "releases/download/v%s-android%s/" % (av, dash(laba)) in html
              and "v%s%s/Quillbinder-%s-full%s-x86_64.AppImage" % (lin, dash(lab), lin, dash(lab)) in html
              and ("version %s windows %s" % (wv, labw)).strip() in html, r.stderr[-300:])
finally:
    shutil.rmtree(work, ignore_errors=True)

# ---- 0.99.0: a dry run of the first normal release: version 1.0.0 and an empty label on every line (no release is made)
def sed_lines(rel, names, root=ROOT):
    """The shell lines of a script that read the given variables of CMakeLists.txt (VERSION=$(sed ...))."""
    return [l for l in read(rel, root).splitlines() if re.match(r"(%s)=\$\(sed -n " % "|".join(names), l.strip())]


dry = tempfile.mkdtemp(prefix="qbdry.")
try:
    dcopy = os.path.join(dry, "repo")
    for d in ("tools", "docs", os.path.join("packaging", "update-feed"), os.path.join("packaging", "linux"), ".github"):
        shutil.copytree(os.path.join(ROOT, d), os.path.join(dcopy, d), ignore=shutil.ignore_patterns("dev-notes", "*.png", "__pycache__", "licenses", "*.desktop"))
    dtext = cm
    for var in ("APP_VERSION", "APP_VERSION_WINDOWS", "APP_VERSION_ANDROID"):
        dtext = re.sub(r'^set\(%s "[^"]*"\)' % var, 'set(%s "1.0.0")' % var, dtext, flags=re.M)
    for var in ("APP_VERSION_LABEL", "APP_VERSION_LABEL_WINDOWS", "APP_VERSION_LABEL_ANDROID"):
        dtext = re.sub(r'^set\(%s "[^"]*"\)' % var, 'set(%s "")' % var, dtext, flags=re.M)
    with open(os.path.join(dcopy, "CMakeLists.txt"), "w", encoding="utf-8") as fh:
        fh.write(dtext)
    with open(os.path.join(dcopy, "docs", "CHANGES.md"), "w", encoding="utf-8") as fh:
        fh.write("# Changes\n\n## 1.0.0 (2026-12-31)\n\n- **The first normal release.** Text.\n")
    dc = os.path.join(dcopy, "CMakeLists.txt")

    def bash(script, **env):
        return run(["bash", "-c", script], cwd=dcopy)

    # the tag (release.yml) and the title and tag check (publish-release.sh), with the very lines of the files
    wf = read(".github/workflows/release.yml")
    tag_line = next(l.strip() for l in wf.splitlines() if l.strip().startswith('if [ "$GITHUB_REF_TYPE" = tag ]; then TAG='))
    ver_lines = [l.strip() for l in wf.splitlines() if re.match(r"\s*(VERSION|LABEL)=\$\(sed -n 's/\^set\(APP_VERSION(_LABEL)? ", l)]
    r = bash("%s\n%s\necho \"[$TAG]\"" % ("\n".join(ver_lines).replace("CMakeLists.txt", dc), tag_line))
    check("empty label, version 1.0.0: release.yml makes the tag v1.0.0 (no dash)", "[v1.0.0]" in r.stdout, r.stdout + r.stderr)
    pub = [l.strip() for l in sed_lines("tools/publish-release.sh", ("VERSION", "LABEL", "NAME"))]
    title = next(l.strip() for l in read("tools/publish-release.sh").splitlines() if l.startswith("TITLE="))
    r = bash("ROOT=%s\n%s\n%s\necho \"[$TITLE]\"" % (dcopy, "\n".join(pub).replace("$ROOT/CMakeLists.txt", dc), title))
    check("... publish-release.sh: the title is \"Quillbinder 1.0.0\"", "[Quillbinder 1.0.0]" in r.stdout, r.stdout + r.stderr)
    # file names of the AppImage, the full AppImage and the update package, with the very lines of the scripts
    def names(rel, wanted):
        lines = sed_lines(rel, ("APP_ID", "APP_NAME", "VERSION", "LABEL"))
        out = [l for l in read(rel).splitlines() if re.match(r"(%s)=" % "|".join(wanted), l)]
        src = "ROOT=%s BUILD=B\n" % dcopy + "\n".join(l.replace("$ROOT/CMakeLists.txt", dc) for l in lines + out)
        return bash(src + "\n" + "\n".join('echo "[$%s]"' % w for w in wanted)).stdout
    one = names("packaging/linux/build-appimage.sh", ("OUT",))
    check("... build-appimage.sh: Quillbinder-1.0.0-x86_64.AppImage", "[B/Quillbinder-1.0.0-x86_64.AppImage]" in one, one)
    full = names("packaging/linux/build-full-appimage.sh", ("IN", "OUT"))
    check("... build-full-appimage.sh: Quillbinder-1.0.0-x86_64.AppImage in, Quillbinder-1.0.0-full-x86_64.AppImage out",
          "[B/Quillbinder-1.0.0-x86_64.AppImage]" in full and "[B/Quillbinder-1.0.0-full-x86_64.AppImage]" in full, full)
    upd = [l for l in read("packaging/linux/build-update.sh").splitlines() if '-update.qbpkg"' in l]
    r = bash("APP_ID=quillbinder VERSION=1.0.0 OUT=O\necho %s" % upd[0].strip().rstrip("\\").strip().replace('"$OUT/$APP_ID-$VERSION-update.qbpkg"', '"$OUT/$APP_ID-$VERSION-update.qbpkg"'))
    check("... build-update.sh: quillbinder-1.0.0-update.qbpkg", "O/quillbinder-1.0.0-update.qbpkg" in r.stdout and "--" not in r.stdout, r.stdout + r.stderr)
    # the shown version: the label part of CMakeLists.txt, run by cmake
    a, b = dtext.index('set(APP_VERSION_LABEL "'), dtext.index("# Identifies the set of libraries")
    for label_value, want in (("", "1.0.0"), ("beta", "1.0.0 beta")):
        script = os.path.join(dry, "shown.cmake")
        seg = dtext[a:b].replace('set(APP_VERSION_LABEL "")', 'set(APP_VERSION_LABEL "%s")' % label_value, 1)
        with open(script, "w", encoding="utf-8") as fh:
            fh.write('set(APP_VERSION "1.0.0")\n' + seg + 'message("[${APP_VERSION_SHOWN}]")\n')
        r = run(["cmake", "-P", script])
        check("... the shown version of CMakeLists.txt with the label %r: %r" % (label_value, want), "[%s]" % want in r.stderr, r.stderr)
    # the release text, the update file and the download page take the names
    dd = os.path.join(dry, "dist")
    os.makedirs(dd)
    files = ["Quillbinder-1.0.0-x86_64.AppImage", "Quillbinder-1.0.0-full-x86_64.AppImage", "quillbinder-1.0.0-update.qbpkg", "SHA256SUMS.txt"]
    for f in files:
        open(os.path.join(dd, f), "wb").write(b"x")
    r = run([sys.executable, "-I", os.path.join(dcopy, "tools", "release-notes.py"), dd])
    check("... release-notes.py: no label in the headline, no \"version\" warning, the files named",
          r.returncode == 0 and r.stdout.startswith("**Quillbinder 1.0.0**") and "This is a" not in r.stdout and "| Quillbinder-1.0.0-full-x86_64.AppImage |" in r.stdout
          and "## What is new in 1.0.0" in r.stdout, r.stdout[:400] + r.stderr[-300:])
    out = os.path.join(dry, "feed.json")
    r = run([sys.executable, "-I", os.path.join(dcopy, "tools", "update-feed.py"), "--dist", dd, "--tag", "v1.0.0", "--repo", "a/b", "--out", out])
    prog = json.load(open(out, encoding="utf-8"))["systems"]["linux-x86_64"]["program"] if r.returncode == 0 else {}
    check("... update-feed.py: version 1.0.0, label \"\", the AppImage and its full version in the release v1.0.0",
          prog.get("version") == "1.0.0" and prog.get("label") == "" and prog.get("appimage", {}).get("url", "").endswith("/releases/download/v1.0.0/Quillbinder-1.0.0-x86_64.AppImage")
          and prog.get("appimageFull", {}).get("url", "").endswith("/v1.0.0/Quillbinder-1.0.0-full-x86_64.AppImage"), r.stderr[-300:] + str(prog)[:300])
    names_dir = os.path.join(dry, "names")
    os.makedirs(names_dir)
    for f in files[:3]:
        open(os.path.join(names_dir, f), "wb").write(b"")
    site = os.path.join(dry, "site")
    r = run([sys.executable, "-I", os.path.join(dcopy, "tools", "download-page.py"), names_dir, "v1.0.0", site])
    html = read("index.html", site) if r.returncode == 0 else ""
    check("... download-page.py: the page takes tag v1.0.0 and the names, shows \"1.0.0\", no \"test version\" sentence",
          r.returncode == 0 and "releases/download/v1.0.0/Quillbinder-1.0.0-full-x86_64.AppImage" in html and "<h1>Quillbinder 1.0.0</h1>" in html
          and "test version" not in html and "This is a" not in html, r.stderr[-300:])
finally:
    shutil.rmtree(dry, ignore_errors=True)

# the download page reads the number of every tag a release can have (the system's word and the label after it)
import importlib.util
spec = importlib.util.spec_from_file_location("download_page", os.path.join(ROOT, "tools", "download-page.py"))
dp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(dp)
tags = {"v1.0.0": [1, 0, 0], "v1.0.0-windows": [1, 0, 0], "v1.0.0-android": [1, 0, 0], "v0.99.0-beta": [0, 99, 0],
        "v0.99.0-windows-experimental": [0, 99, 0], "v0.99.0-android-experimental": [0, 99, 0]}
check("download-page.py reads the number of v1.0.0, v1.0.0-windows, v1.0.0-android and the three 0.99.0 tags", all(dp.tag_version(t) == v for t, v in tags.items()),
      {t: dp.tag_version(t) for t in tags})
check("... and 1.0.0 is newer than 0.99.0 and 0.99.1 on every system, whatever the label",
      dp.tag_version("v1.0.0-windows") > dp.tag_version("v0.99.1-windows-experimental") and dp.tag_version("v1.0.0") > dp.tag_version("v0.99.0-beta")
      and dp.shown_of("v1.0.0") == "1.0.0" and dp.shown_of("v0.99.0-windows-experimental") == "0.99.0 windows experimental")

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
