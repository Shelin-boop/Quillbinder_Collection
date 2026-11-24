#!/usr/bin/env python3
"""Suite addonentry (0.93.1): the official entry of Translation and Proofreading 1.3 and the add-on's own packaging.

  - to-do item 23: an entry may name the add-on's folder in its repository ("folder"; the project's own add-on lives in
    addons/translation-proofreading/source of Quillbinder's repository): tools/addon-entry.py runs the commands there,
    keys the cache by files there, archives only that folder, takes a Windows or Android block's own folder, and
    refuses a folder outside the repository; the real entry describes 1.3 and its folder has what its commands use;
  - to-do item 52: the add-on's own make-package.py names Quillbinder's runtime (read from CMakeLists.txt), not an
    older one of its own.

Offline, Python only (a made-up repository in a temporary folder).

  python3 -I tests/addon_entry_tests.py
"""
import json
import os
import re
import subprocess
import sys
import tarfile
import tempfile
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOL = os.path.join(ROOT, "tools", "addon-entry.py")
FAILS = 0


def check(name, ok, detail=""):
    global FAILS
    print(("PASS " if ok else "FAIL ") + name + ("" if ok else f": {detail}"))
    if not ok:
        FAILS += 1


def cmake(name, android=False):
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        text = f.read()
    if android:  # Android's line, inside if(ANDROID)
        m = re.search(r'^\s+set\(%s "(android-[^"]+)"\)' % re.escape(name), text, re.M)
    else:
        m = re.search(r'^set\(%s "([^"]+)"\)' % re.escape(name), text, re.M)
    return m.group(1) if m else ""


def tool(*args, cwd=None):
    r = subprocess.run([sys.executable, "-I", TOOL] + list(args), capture_output=True, text=True, cwd=cwd)
    return r.returncode, r.stdout + r.stderr


def git(repo, *args):
    return subprocess.run(["git", "-C", repo, "-c", "user.name=c", "-c", "user.email=c@c"] + list(args), check=True,
                          capture_output=True, text=True).stdout.strip()


# ---------------------------------------------------------------- item 23: the folder field
with tempfile.TemporaryDirectory() as tmp:
    repo = os.path.join(tmp, "repo")
    folder = "addons/sample/source"
    src = os.path.join(repo, folder)
    os.makedirs(os.path.join(src, "packaging"))
    with open(os.path.join(repo, "TOP-LEVEL.txt"), "w") as f:
        f.write("not part of the add-on\n")
    with open(os.path.join(src, "packaging", "deps.sh"), "w") as f:
        f.write("echo deps\n")
    with open(os.path.join(src, "README.md"), "w") as f:
        f.write("the add-on\n")
    subprocess.run(["git", "init", "-q", repo], check=True)
    git(repo, "add", "-A")
    git(repo, "commit", "-qm", "sample")
    commit = git(repo, "rev-parse", "HEAD")
    entry = {
        "format": 1, "id": "sample-addon", "name": "Sample Add-on", "version": "1.0", "repository": "someone/sample",
        "commit": commit, "folder": folder, "author": "Someone", "license": "MIT", "reviewed": "2026-11-24",
        "summary": "For the checks.",
        "build": {"cachePaths": ["deps/install"], "cacheKeyFiles": ["packaging/deps.sh"],
                  "setup": ["pwd > setup-ran.txt"], "steps": ["test -f README.md && echo steps > steps-ran.txt"]},
        "windows": {"commit": commit, "folder": "addons/sample/source/packaging",
                    "build": {"steps": ["pwd > windows-ran.txt"]}},
    }
    path = os.path.join(tmp, "sample-addon.json")
    with open(path, "w") as f:
        json.dump(entry, f)

    rc, out = tool("check", path)
    check("addon-entry.py check: the entry with a folder is accepted and names it", rc == 0 and ("folder=%s" % folder) in out, out)
    rc, out = tool("cache", path, repo)
    check("cache: the folders kept are inside the add-on's folder",
          rc == 0 and os.path.join(src, "deps/install") in out and os.path.join(repo, "deps/install") + "\n" not in out, out)
    rc, out = tool("run", path, "setup", repo)
    ran = open(os.path.join(src, "setup-ran.txt")).read().strip() if os.path.exists(os.path.join(src, "setup-ran.txt")) else ""
    check("run setup: the commands run in the add-on's folder", rc == 0 and os.path.realpath(ran) == os.path.realpath(src),
          "%s / %s" % (ran, out))
    rc, out = tool("run", path, "steps", repo)
    check("run steps: a path of the entry is a path inside the folder", rc == 0 and os.path.exists(os.path.join(src, "steps-ran.txt")), out)
    files = os.path.join(tmp, "files")
    rc, out = tool("sources", path, repo, files)
    names = []
    archive = os.path.join(files, "sample-addon-1.0-source.tar.gz")
    if os.path.exists(archive):
        with tarfile.open(archive) as t:
            names = t.getnames()
    check("sources: the archive holds the add-on's folder of the reviewed commit, nothing of the repository's top",
          rc == 0 and "sample-addon-1.0/README.md" in names and "sample-addon-1.0/packaging/deps.sh" in names
          and not any("TOP-LEVEL" in n or "addons/" in n for n in names), "%s %s" % (names, out))
    sources_txt = open(os.path.join(files, "SOURCES.txt")).read() if os.path.exists(os.path.join(files, "SOURCES.txt")) else ""
    check("sources: SOURCES.txt names the folder beside the commit", ("its folder %s" % folder) in sources_txt, sources_txt[:400])
    rc, out = tool("run", path, "steps", repo, "--windows")
    wran = os.path.join(src, "packaging", "windows-ran.txt")
    check("a Windows block's own folder wins over the entry's", rc == 0 and os.path.exists(wran), out)
    for bad in ("../outside", "/abs/path", "addons/../../x"):
        e2 = dict(entry, folder=bad)
        with open(path, "w") as f:
            json.dump(e2, f)
        rc, out = tool("check", path)
        check("a folder outside the repository is refused (%s)" % bad, rc != 0 and "folder" in out, out)
    e3 = dict(entry, build=dict(entry["build"], cacheKeyFiles=[]))
    del e3["folder"]
    with open(path, "w") as f:
        json.dump(e3, f)
    rc, out = tool("cache", path, repo)
    check("without a folder the repository's top is the add-on's folder (as before)",
          rc == 0 and os.path.join(repo, "deps/install") in out, out)

# ---------------------------------------------------------------- item 23: the real entry describes 1.3
real_path = os.path.join(ROOT, "official-addons", "translation-proofreading.json")
real = json.load(open(real_path, encoding="utf-8"))
tp = os.path.join(ROOT, real.get("folder", "-"))
manifest = json.load(open(os.path.join(ROOT, "addons", "translation-proofreading", "source", "packaging", "manifest.json"), encoding="utf-8"))
check("the official entry describes the version the add-on's manifest has (%s)" % manifest["version"],
      real["version"] == manifest["version"] == "1.3", "%s / %s" % (real["version"], manifest["version"]))
check("the official entry names the add-on's folder in this repository", real.get("folder") == "addons/translation-proofreading/source"
      and os.path.isdir(tp), real.get("folder", ""))
rc, out = tool("check", real_path)
check("addon-entry.py accepts the real entry (Linux)", rc == 0 and "version=1.3" in out, out)
for flag in ("--windows", "--android"):
    rc, out = tool("check", real_path, flag)
    check("addon-entry.py accepts the real entry (%s)" % flag[2:], rc == 0 and "folder=addons/translation-proofreading/source" in out, out)
missing = []
for system, block in (("linux", real), ("windows", real.get("windows", {})), ("android", real.get("android", {}))):
    b = block.get("build", {})
    for p in b.get("cacheKeyFiles", []):
        if not os.path.isfile(os.path.join(tp, p)):
            missing.append("%s: %s" % (system, p))
    for c in b.get("setup", []) + b.get("steps", []):
        for word in re.findall(r"(?:packaging|tools|tests)/[A-Za-z0-9_.-]+\.(?:sh|py)", c):
            if not os.path.isfile(os.path.join(tp, word)):
                missing.append("%s: %s" % (system, word))
check("every script the entry's commands and cache keys name is in the add-on's folder", not missing, ", ".join(missing))
downloads = real["build"].get("testData", {}).get("downloads", [])
check("the test data come from models-1, models-2 and (1.3) models-3, models-3 last (its catalog names every model)",
      [d["release"] for d in downloads] == ["models-1", "models-2", "models-3"]
      and "catalog.json" in downloads[-1].get("patterns", []), json.dumps(downloads))
commits = {real["commit"], real.get("windows", {}).get("commit"), real.get("android", {}).get("commit")}
check("each system's block names a full commit id", all(c and re.match(r"^[0-9a-f]{40}$", c) for c in commits), str(commits))

# ---------------------------------------------------------------- item 52: the add-on's own packaging names the runtime
mp = os.path.join(ROOT, "addons", "translation-proofreading", "source", "packaging", "make-package.py")
want_linux, want_since, want_android = cmake("APP_RUNTIME"), cmake("APP_RUNTIME_SINCE"), cmake("APP_RUNTIME", True)
with tempfile.TemporaryDirectory() as tmp:
    for platform, want in (("", want_linux), ("android-arm64", "android-")):
        stage = os.path.join(tmp, "stage" + platform)
        os.makedirs(os.path.join(stage, "payload", "lib"))
        m = {"id": "translation-proofreading", "name": "T", "version": "1.3", "description": "d", "author": "a",
             "minApp": "0.80.0", "whatsNew": ["x"]}
        if platform:
            m["platform"] = platform
        with open(os.path.join(stage, "manifest.json"), "w") as f:
            json.dump(m, f)
        with open(os.path.join(stage, "payload", "lib", "libengine.so"), "wb") as f:
            f.write(b"\x7fELF")
        out = os.path.join(tmp, "out%s.qbpkg" % platform)
        r = subprocess.run([sys.executable, "-I", mp, stage, out], capture_output=True, text=True)
        got = {}
        if r.returncode == 0:
            with zipfile.ZipFile(out) as z:
                got = json.loads(z.read("manifest.json"))
        if platform:
            check("make-package.py: an Android package names Quillbinder for Android's runtime (%s)" % want_android,
                  got.get("runtime") == want_android and want_android.startswith("android-"), "%s %s" % (got.get("runtime"), r.stderr))
        else:
            check("make-package.py: a Linux package names the program's runtime (%s), not an older one" % want_linux,
                  got.get("runtime") == want_linux and want_linux.endswith("-r2"), "%s %s" % (got.get("runtime"), r.stderr))
            check("make-package.py: and needs the first version of that runtime (%s)" % want_since,
                  got.get("minApp") == want_since, "%s" % got.get("minApp"))

print("%d failure(s)" % FAILS)
sys.exit(1 if FAILS else 0)
