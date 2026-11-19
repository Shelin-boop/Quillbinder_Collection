#!/usr/bin/env python3
"""The pinned entries of the official add-ons (official-addons/<id>.json), for the workflow "Official add-on"
(.github/workflows/addon-release.yml, 0.91.3). Developer tool.

    addon-entry.py check <entry>                      # checks the entry; prints id=, name=, version=, tag=,
                                                      # repository=, commit= (for $GITHUB_OUTPUT)
    addon-entry.py cache <entry> <source>             # the folders kept between runs and their key (for $GITHUB_OUTPUT)
    addon-entry.py run <entry> setup|steps <source>   # runs the entry's commands in the add-on's source
    addon-entry.py testdata <entry>                   # the test data's key and variable (for $GITHUB_OUTPUT)
    addon-entry.py fetch-testdata <entry> <dir>       # downloads it (gh; GH_TOKEN) from the add-on's releases
    addon-entry.py check-package <entry> <package>    # the package fits the entry and Quillbinder's runtime, unsigned
    addon-entry.py sources <entry> <source> <out>     # the add-on's source, the Ubuntu source of the system libraries
                                                      # it carries, SOURCES.txt
    addon-entry.py notes <entry> <package> <files> <out.md>   # the release text

Each command takes --windows (2026-11-16): the entry's Windows build instead (its "windows" block: the reviewed commit
and the build for Quillbinder for Windows, .github/workflows/addon-release-windows.yml), or --android (2026-11-16, the
"android" block: the build for Quillbinder for Android's tablets, .github/workflows/addon-release-android.yml).
Without either, everything is as before (Linux).

The entries are written by the project (see official-addons/README.md); their commands run in the build job only.
"""
import hashlib
import json
import os
import re
import subprocess
import sys
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SOURCES_RELEASE = "third-party-sources"
REQUIRED_PROGRAM = "0.91.3"   # the first Quillbinder that trusts add-on keys
SYSTEM = "linux"              # "windows" with --windows, "android" with --android (2026-11-16)
ANDROID_PLATFORM = "android-arm64"   # the public Android app is for arm64 tablets (0.91.3)
# what an Android add-on's libraries may need: Android's own libraries, the C++ library and Qt's, all loaded by the app
ANDROID_NEEDED = re.compile(r"^(libc|libm|libdl|liblog|libandroid|libEGL|libGLESv2|libGLESv3|libjnigraphics|libvulkan|"
                            r"libc\+\+_shared)\.so$|^libQt6[A-Za-z]+_arm64-v8a\.so$")


class EntryError(Exception):
    pass


def cmake_var(name):
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r'^set\(%s\s+"([^"]*)"\)' % re.escape(name), f.read(), re.M)
    return m.group(1) if m else ""


def relative(path, what):
    if not isinstance(path, str) or not path or path.startswith("/") or ".." in path.split("/"):
        raise EntryError("%s: %r is not a path inside the add-on's source" % (what, path))
    return path


def strings(value, what, allow_empty=False):
    if not isinstance(value, list) or not all(isinstance(v, str) and v for v in value) or (not value and not allow_empty):
        raise EntryError("%s must be a list of texts" % what)
    return value


def load(path):
    try:
        with open(path, encoding="utf-8") as f:
            e = json.load(f)
    except (OSError, ValueError) as ex:
        raise EntryError("%s cannot be read: %s" % (path, ex))
    if not isinstance(e, dict) or e.get("format") != 1:
        raise EntryError("format must be 1")
    checks = {
        "id": r"^[a-z0-9][a-z0-9-]{1,62}$",
        "version": r"^[0-9]+(\.[0-9]+){0,3}$",
        "repository": r"^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$",
        "commit": r"^[0-9a-f]{40}$",
        "reviewed": r"^[0-9]{4}-[0-9]{2}-[0-9]{2}$",
    }
    for key, pattern in checks.items():
        if not isinstance(e.get(key), str) or not re.match(pattern, e[key]):
            raise EntryError("%s is missing or has the wrong form" % key)
    if os.path.splitext(os.path.basename(path))[0] != e["id"]:
        raise EntryError("the file must be named after the id (%s.json)" % e["id"])
    for key in ("name", "author", "license", "summary"):
        if not isinstance(e.get(key), str) or not e[key].strip() or "\n" in e[key]:
            raise EntryError("%s must be one line of text" % key)
    nc = e.get("nonCommercialData", [])
    if not isinstance(nc, list) or not all(isinstance(d, dict) and isinstance(d.get("name"), str) and d["name"]
                                           and isinstance(d.get("license"), str) and d["license"] for d in nc):
        raise EntryError("nonCommercialData must be a list of {name, license}")
    b = e.get("build")
    if not isinstance(b, dict):
        raise EntryError("build is missing")
    strings(b.get("packages", []), "build.packages", True)
    for p in b.get("packages", []):
        if not re.match(r"^[a-z0-9][a-z0-9+.-]*$", p):
            raise EntryError("build.packages: %r is not a package name" % p)
    for p in strings(b.get("cachePaths", []), "build.cachePaths", True):
        relative(p, "build.cachePaths")
    for p in strings(b.get("cacheKeyFiles", []), "build.cacheKeyFiles", True):
        relative(p, "build.cacheKeyFiles")
    strings(b.get("setup", []), "build.setup", True)
    strings(b.get("steps"), "build.steps")
    td = b.get("testData")
    if td is not None:
        if not isinstance(td, dict) or not re.match(r"^[A-Z][A-Z0-9_]*$", str(td.get("variable", ""))):
            raise EntryError("build.testData must name a variable")
        for d in downloads(td):
            if not re.match(r"^[A-Za-z0-9_.-]+$", str(d.get("release", ""))):
                raise EntryError("build.testData: each download names a release")
            for p in d.get("patterns", []):
                if not isinstance(p, str) or not re.match(r"^[A-Za-z0-9_.*?-]+$", p):
                    raise EntryError("build.testData: %r is no file pattern" % p)
    for lib in b.get("systemLibraries", []):
        if not isinstance(lib, dict) or not isinstance(lib.get("from"), str) or not lib["from"].startswith("/usr/lib/"):
            raise EntryError("build.systemLibraries: each needs file and from (a library under /usr/lib)")
        relative(lib.get("file"), "build.systemLibraries")
    for dep in b.get("dependencySources", []):
        if not isinstance(dep, dict) or not isinstance(dep.get("name"), str) or not dep["name"]:
            raise EntryError("build.dependencySources: each needs name and dir")
        relative(dep.get("dir"), "build.dependencySources")
    w = e.get("windows")
    if w is not None:
        check_system_block(w, "windows")
    a = e.get("android")
    if a is not None:
        check_system_block(a, "android")
    if SYSTEM in ("windows", "android"):
        block = w if SYSTEM == "windows" else a
        if block is None:
            raise EntryError("the entry has no %s build (%s)" % ("Windows" if SYSTEM == "windows" else "Android", SYSTEM))
        e = dict(e)
        e["commit"], e["build"], e["system"] = block["commit"], block["build"], SYSTEM
        e["reviewed"] = block.get("reviewed", e["reviewed"])
    return e


def check_windows(w):
    check_system_block(w, "windows")


def check_system_block(w, name):
    """The Windows block (2026-11-16): {"commit", "reviewed"?, "build"} with build as for Linux, but without Ubuntu
    packages and system libraries; Microsoft's redistributable DLLs it carries are named in "redistributables".
    The Android block (2026-11-16): the same form; its build runs on Ubuntu with the Android NDK and Qt for Android, and
    the libraries of the NDK's toolchain linked into the add-on are named in "toolchainLibraries" ({name, source})."""
    if not isinstance(w, dict):
        raise EntryError("%s must be an object" % name)
    if not isinstance(w.get("commit"), str) or not re.match(r"^[0-9a-f]{40}$", w["commit"]):
        raise EntryError("%s.commit is missing or has the wrong form" % name)
    if "reviewed" in w and not re.match(r"^[0-9]{4}-[0-9]{2}-[0-9]{2}$", str(w["reviewed"])):
        raise EntryError("%s.reviewed has the wrong form" % name)
    b = w.get("build")
    if not isinstance(b, dict):
        raise EntryError("%s.build is missing" % name)
    if b.get("packages") or b.get("systemLibraries"):
        raise EntryError("%s.build: no Ubuntu packages or system libraries for %s" % (name, name.capitalize()))
    for p in strings(b.get("cachePaths", []), "%s.build.cachePaths" % name, True):
        relative(p, "%s.build.cachePaths" % name)
    for p in strings(b.get("cacheKeyFiles", []), "%s.build.cacheKeyFiles" % name, True):
        relative(p, "%s.build.cacheKeyFiles" % name)
    strings(b.get("setup", []), "%s.build.setup" % name, True)
    strings(b.get("steps"), "%s.build.steps" % name)
    td = b.get("testData")
    if td is not None:
        if not isinstance(td, dict) or not re.match(r"^[A-Z][A-Z0-9_]*$", str(td.get("variable", ""))):
            raise EntryError("%s.build.testData must name a variable" % name)
        for d in downloads(td):
            if not re.match(r"^[A-Za-z0-9_.-]+$", str(d.get("release", ""))):
                raise EntryError("%s.build.testData: each download names a release" % name)
            for p in d.get("patterns", []):
                if not isinstance(p, str) or not re.match(r"^[A-Za-z0-9_.*?-]+$", p):
                    raise EntryError("%s.build.testData: %r is no file pattern" % (name, p))
    if name != "windows" and b.get("redistributables"):
        raise EntryError("%s.build: redistributables are Windows' (Microsoft's DLLs)" % name)
    for r in b.get("redistributables", []):
        if not isinstance(r, dict) or not isinstance(r.get("name"), str) or not r["name"]:
            raise EntryError("%s.build.redistributables: each needs file and name" % name)
        relative(r.get("file"), "%s.build.redistributables" % name)
    if name != "android" and b.get("toolchainLibraries"):
        raise EntryError("%s.build: toolchainLibraries are Android's (the NDK's)" % name)
    for t in b.get("toolchainLibraries", []):
        if not isinstance(t, dict) or not all(isinstance(t.get(k), str) and t[k].strip() for k in ("name", "source")):
            raise EntryError("%s.build.toolchainLibraries: each needs name and source" % name)
    for dep in b.get("dependencySources", []):
        if not isinstance(dep, dict) or not isinstance(dep.get("name"), str) or not dep["name"]:
            raise EntryError("%s.build.dependencySources: each needs name and dir" % name)
        relative(dep.get("dir"), "%s.build.dependencySources" % name)


def downloads(td):
    """The test data's downloads: {"release": ...} (one whole release, 1.0's form) or "downloads": [{"release": ...,
    "patterns": [...]}] (1.1: the files of several releases, or some of them)."""
    if td.get("downloads"):
        return td["downloads"]
    return [{"release": td["release"]}] if td.get("release") else []


def cmd_testdata(path):
    e = load(path)
    td = e["build"].get("testData") or {}
    items = downloads(td)
    h = hashlib.sha256(json.dumps(items, sort_keys=True).encode()).hexdigest()[:16]
    print("any=%s" % ("true" if items else "false"))
    print("key=addon-%s-testdata-%s" % (e["id"], h))
    print("variable=%s" % td.get("variable", ""))
    return 0


def cmd_fetch_testdata(path, out):
    e = load(path)
    os.makedirs(out, exist_ok=True)
    for d in downloads(e["build"].get("testData") or {}):
        cmd = ["gh", "release", "download", d["release"], "-R", e["repository"], "-D", out, "--clobber"]
        for p in d.get("patterns", []):
            cmd += ["-p", p]
        print(" ".join(cmd), flush=True)
        subprocess.run(cmd, check=True)
    return 0


def tag(e):
    # never "v...": the program's releases and the download page take the tags that start with v
    if e.get("system") in ("windows", "android"):
        return "addon-%s-%s-%s" % (e["id"], e["version"], e["system"])
    return "addon-%s-%s" % (e["id"], e["version"])


def cmd_check(path):
    e = load(path)
    for k in ("id", "name", "version", "repository", "commit"):
        print("%s=%s" % (k, e[k]))
    print("tag=%s" % tag(e))
    return 0


def cmd_cache(path, src):
    e = load(path)
    b = e["build"]
    h = hashlib.sha256()
    h.update(json.dumps({"setup": b.get("setup", []), "packages": b.get("packages", [])}, sort_keys=True).encode())
    for f in b.get("cacheKeyFiles", []):
        h.update(f.encode() + b"\0")
        with open(os.path.join(src, f), "rb") as fh:
            h.update(fh.read())
    print("key=addon-%s-deps-%s-%s" % (e["id"], h.hexdigest()[:16], {"windows": "windows-2025", "android": "ubuntu-24.04-android"}
                                        .get(e.get("system"), "ubuntu-24.04")))
    print("paths<<PATHS_END")
    for p in b.get("cachePaths", []):
        print(os.path.join(src, p))
    print("PATHS_END")
    print("any=%s" % ("true" if b.get("cachePaths") else "false"))
    return 0


def cmd_run(path, which, src):
    e = load(path)
    if which not in ("setup", "steps"):
        raise EntryError("run: setup or steps")
    for c in e["build"].get(which, []):
        print("::group::%s" % c, flush=True)
        # (QB_BASH: Git Bash on Windows; there "bash" alone could be found in System32 first)
        r = subprocess.run([os.environ.get("QB_BASH") or "bash", "-euo", "pipefail", "-c", c], cwd=src)
        print("::endgroup::", flush=True)
        if r.returncode != 0:
            print("::error title=The add-on's build failed::%s (exit status %d)" % (c, r.returncode), flush=True)
            return 1
    return 0


def cmd_check_package(path, package):
    e = load(path)
    problems = []
    with zipfile.ZipFile(package) as z:
        names = set(z.namelist())
        m = json.loads(z.read("manifest.json"))
        if m.get("id") != e["id"]:
            problems.append("the manifest's id is %r, the entry's %r" % (m.get("id"), e["id"]))
        if m.get("version") != e["version"]:
            problems.append("the manifest's version is %r, the entry's %r" % (m.get("version"), e["version"]))
        if m.get("kind", "addon") != "addon":
            problems.append("the package is no add-on (kind %r)" % m.get("kind"))
        has_libraries = any(n.endswith(".so") or ".so." in n for n in names)
        if e.get("system") == "windows":
            # Quillbinder for Windows takes an add-on of its own system only ("platform"; no runtime id, no .so files)
            if m.get("platform") != "windows-x64":
                problems.append("the manifest's platform is %r, Quillbinder for Windows needs 'windows-x64'" % m.get("platform"))
            if has_libraries:
                problems.append("the package carries Linux libraries (.so)")
        elif e.get("system") == "android":
            # Quillbinder for Android (0.91.3): its own platform and runtime, and 0.91.3 or later (the first that installs
            # add-ons); its libraries need only what the app loads and keep to 16 KB pages
            problems += android_problems(z, m, names)
        elif has_libraries and m.get("runtime") != cmake_var("APP_RUNTIME"):
            problems.append("the package names the runtime %r, Quillbinder's AppImage has %r"
                            % (m.get("runtime"), cmake_var("APP_RUNTIME")))
        if "manifest.sig" in names:
            problems.append("the package is signed already (only the workflow's signing job signs)")
        listed = set()
        for fe in m.get("files", []):
            p = "payload/" + fe.get("path", "")
            listed.add(p)
            if p not in names:
                problems.append("%s is listed but missing" % p)
                continue
            data = z.read(p)
            if hashlib.sha256(data).hexdigest() != fe.get("sha256") or len(data) != fe.get("size"):
                problems.append("%s does not fit its SHA-256 or size" % p)
        extra = sorted(n for n in names if n.startswith("payload/") and not n.endswith("/") and n not in listed)
        if extra:
            problems.append("files not in the manifest: %s" % ", ".join(extra[:5]))
    for p in problems:
        print("::error title=The package does not fit::%s" % p)
    if not problems:
        if e.get("system") in ("windows", "android"):
            print("%s %s: fits (%d files, platform %s)" % (m["id"], m["version"], len(listed), m.get("platform")))
        else:
            print("%s %s: fits (%d files, runtime %s)" % (m["id"], m["version"], len(listed), m.get("runtime", "none")))
    return 1 if problems else 0


def version_tuple(v):
    return tuple(int(x) for x in re.findall(r"[0-9]+", str(v))[:4])


def android_runtime():
    """Quillbinder for Android's runtime id (its own line of APP_RUNTIME, inside if(ANDROID) in CMakeLists.txt)."""
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r'set\(APP_RUNTIME\s+"(android-[^"]*)"\)', f.read())
    return m.group(1) if m else ""


def android_problems(z, m, names):
    problems = []
    if m.get("platform") != ANDROID_PLATFORM:
        problems.append("the manifest's platform is %r, Quillbinder for Android's tablets need %r" % (m.get("platform"), ANDROID_PLATFORM))
    if m.get("runtime") != android_runtime():
        problems.append("the package names the runtime %r, Quillbinder for Android has %r" % (m.get("runtime"), android_runtime()))
    if version_tuple(m.get("minApp", "0")) < version_tuple(REQUIRED_PROGRAM):
        problems.append("minApp is %r; Quillbinder for Android installs add-ons since %s" % (m.get("minApp"), REQUIRED_PROGRAM))
    if any(n.lower().endswith(".dll") or n.lower().endswith(".exe") for n in names):
        problems.append("the package carries Windows files")
    libs = sorted(n for n in names if n.startswith("payload/") and (n.endswith(".so") or ".so." in n))
    if not libs:
        problems.append("the package carries no library for Android")
    readelf = os.environ.get("READELF", "")
    if not readelf or not os.path.isfile(readelf):
        problems.append("READELF (the NDK's llvm-readelf) is not set: the libraries' needs and pages were not checked")
        return problems
    import tempfile
    with tempfile.TemporaryDirectory() as tmp:
        for n in libs:
            f = os.path.join(tmp, os.path.basename(n))
            with open(f, "wb") as out:
                out.write(z.read(n))
            dyn = subprocess.run([readelf, "-d", f], capture_output=True, text=True).stdout
            needed = re.findall(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]", dyn)
            other = [x for x in needed if not ANDROID_NEEDED.match(x)]
            if other:
                problems.append("%s needs %s, which Quillbinder for Android does not bring" % (n, ", ".join(other)))
            head = subprocess.run([readelf, "-h", f], capture_output=True, text=True).stdout
            if "AArch64" not in head:
                problems.append("%s is not built for arm64 (AArch64)" % n)
            prog = subprocess.run([readelf, "-lW", f], capture_output=True, text=True).stdout
            aligns = [int(line.split()[-1], 16) for line in prog.splitlines() if line.split()[:1] == ["LOAD"]]
            if not aligns or min(aligns) < 16384:
                problems.append("%s has LOAD segments below 16 KB alignment (%s): Android 15 and later may use 16 KB pages"
                                % (n, ", ".join(hex(a) for a in aligns) or "none"))
            print("%s: needs %s; LOAD alignment %s" % (n, " ".join(needed) or "nothing", ", ".join(hex(a) for a in aligns)))
    return problems


def sh(*args, cwd=None):
    return subprocess.run(list(args), cwd=cwd, check=True, capture_output=True, text=True).stdout.strip()


def cmd_sources(path, src, out):
    e = load(path)
    os.makedirs(out, exist_ok=True)
    lines = []
    windows = e.get("system") == "windows"
    android = e.get("system") == "android"
    archive = "%s-%s-%ssource.tar.gz" % (e["id"], e["version"], "windows-" if windows else "android-" if android else "")
    with open(os.path.join(out, archive), "wb") as f:
        subprocess.run(["git", "-C", src, "archive", "--format=tar.gz", "--prefix=%s-%s/" % (e["id"], e["version"]),
                        e["commit"]], stdout=f, check=True)
    lines.append("%s\n    the add-on's own source, the reviewed commit %s of %s\n" % (archive, e["commit"], e["repository"]))
    for lib in e["build"].get("systemLibraries", []):
        built = os.path.join(src, lib["file"])
        with open(built, "rb") as a, open(os.path.realpath(lib["from"]), "rb") as b:
            if a.read() != b.read():
                raise EntryError("%s is not the system's %s" % (lib["file"], lib["from"]))
        name = sh(sys.executable, "-I", os.path.join(ROOT, "tools", "collect-sources.py"), "--of-file", lib["from"], out)
        pkg = sh(sys.executable, "-I", os.path.join(ROOT, "tools", "collect-sources.py"), "--name", "ubuntu", lib["from"])
        lines.append("%s\n    the Ubuntu 24.04 source package %s %s, of %s inside the add-on\n"
                     % (name.splitlines()[-1], pkg.split()[0], pkg.split()[1], os.path.basename(lib["file"])))
    for r in e["build"].get("redistributables", []):
        if not os.path.isfile(os.path.join(src, r["file"])):
            raise EntryError("%s is not in the build" % r["file"])
        lines.append("%s (no source: %s, redistributable code of Microsoft's Visual Studio, unchanged)\n"
                     % (os.path.basename(r["file"]), r["name"]))
    for t in e["build"].get("toolchainLibraries", []):
        lines.append("%s (linked into the add-on from the build's toolchain)\n    source: %s\n" % (t["name"], t["source"]))
    for dep in e["build"].get("dependencySources", []):
        d = os.path.join(src, dep["dir"])
        commit = sh("git", "-C", d, "rev-parse", "HEAD")
        url = subprocess.run(["git", "-C", d, "remote", "get-url", "origin"], capture_output=True, text=True).stdout.strip() \
            or "(a folder of the add-on's source)"
        described = subprocess.run(["git", "-C", d, "describe", "--tags", "--always"], capture_output=True, text=True).stdout.strip()
        subs = subprocess.run(["git", "-C", d, "submodule", "status", "--recursive"], capture_output=True, text=True).stdout
        lines.append("%s (built from source, linked into the add-on)\n    %s %s, commit %s\n" % (dep["name"], url, described, commit)
                     + "".join("    submodule %s\n" % s.strip() for s in subs.splitlines() if s.strip()))
    with open(os.path.join(out, "SOURCES.txt"), "w", encoding="utf-8") as f:
        f.write("Source code of the add-on \"%s\" %s\n%s\n\n" % (e["name"], e["version"], "=" * (30 + len(e["name"]))))
        f.write("\n".join(lines))
        if android:
            f.write("\nThe Android add-on carries no part of Qt (Quillbinder for Android brings Qt %s, its TLS plugin and\n"
                    "OpenSSL): the Qt Project's source release and OpenSSL's are on the release \"%s\" of the Quillbinder\n"
                    "project (qt-everywhere-src-%s.tar.xz, openssl-*.tar.gz).\n"
                    % (cmake_var("QB_QT_VERSION"), SOURCES_RELEASE, cmake_var("QB_QT_VERSION")))
        elif windows:
            f.write("\nThe Windows add-on carries no part of Qt (Quillbinder for Windows brings Qt %s and its TLS plugin):\n"
                    "the Qt Project's source release is on the release \"%s\" of the Quillbinder project\n"
                    "(qt-everywhere-src-%s.tar.xz).\n" % (cmake_var("QB_QT_VERSION"), SOURCES_RELEASE, cmake_var("QB_QT_VERSION")))
        else:
            f.write("\nQt's TLS plugin, if the add-on carries it, is part of Qt %s: the Qt Project's source release is on the\n"
                    "release \"%s\" of the Quillbinder project (qt-everywhere-src-%s.tar.xz).\n"
                    % (cmake_var("QB_QT_VERSION"), SOURCES_RELEASE, cmake_var("QB_QT_VERSION")))
    print("\n".join(sorted(os.listdir(out))))
    return 0


def non_commercial_sentence(e):
    nc = e.get("nonCommercialData", [])
    if not nc:
        return "This add-on is free and must not be sold."
    parts = ["%s, whose license (%s) allows non-commercial use only" % (d["name"], d["license"]) for d in nc]
    return "This add-on is free and must not be sold. It can download %s." % "; and ".join(parts)


def cmd_notes(path, package, files_dir, out):
    e = load(path)
    with zipfile.ZipFile(package) as z:
        m = json.loads(z.read("manifest.json"))
    # SHA256SUMS.txt is written by the signing job, after this text
    files = sorted(set(n for n in os.listdir(files_dir) if os.path.isfile(os.path.join(files_dir, n))) | {"SHA256SUMS.txt"})
    pkg = os.path.basename(package)
    src_url = "https://github.com/%s/tree/%s" % (e["repository"], e["commit"])
    windows = e.get("system") == "windows"
    android = e.get("system") == "android"
    text = ["# %s %s%s, an official add-on for Quillbinder" % (e["name"], e["version"],
                                                              " for Windows" if windows else " for Android" if android else ""), "",
            e["summary"], "",
            "Written by %s under the %s; reviewed for the official list of Quillbinder add-ons on %s and built by the "
            "Quillbinder project from the reviewed source ([%s at %s](%s))."
            % (e["author"], e["license"], e["reviewed"], e["repository"], e["commit"][:12], src_url), "",
            non_commercial_sentence(e), "",
            "## Installing", ""]
    if android:
        text += ["In Quillbinder for Android %s or later (tablets, 64-bit ARM): download `%s` on the tablet, then "
                 "**File > Install Update or Add-on** in Quillbinder (the file window opens in the Download folder) and "
                 "choose it. The install window says that the add-on is signed with the add-on key \"Quillbinder official "
                 "add-ons\", which the Quillbinder project vouches for; Quillbinder offers to start again, which loads it. The add-on is kept "
                 "in the app's own folder; its models are downloaded from the add-on's window (**Models...**). (For Linux and "
                 "Windows, the add-on has releases of its own.)" % (REQUIRED_PROGRAM, pkg), ""]
    elif windows:
        text += ["In Quillbinder for Windows %s or later: **File > Install Update or Add-on**, choose `%s`. The install "
                 "window says that the add-on is signed with the add-on key \"Quillbinder official add-ons\", which the "
                 "Quillbinder project vouches for. Quillbinder for Windows %s does not know add-on keys yet and warns that "
                 "the add-on is not signed: update Quillbinder first. (For Linux, the add-on has a release of its own.)"
                 % (REQUIRED_PROGRAM, pkg, m.get("minApp", "0.91.1")), ""]
    else:
        text += [
            "In Quillbinder for Linux %s or later: **File > Install Update or Add-on**, choose `%s`. The install window "
            "says that the add-on is signed with the add-on key \"Quillbinder official add-ons\", which the Quillbinder "
            "project vouches for. Quillbinder %s to %s does not know add-on keys yet and warns that the add-on is not "
            "signed: update Quillbinder first." % (REQUIRED_PROGRAM, pkg, m.get("minApp", "0.80.0"), "0.91.2"), ""]
    if m.get("whatsNew"):
        text += ["## What is new", ""] + ["- %s" % w for w in m["whatsNew"]] + [""]
    text += ["## Files", ""]
    for f in files:
        if f == pkg:
            what = "the add-on"
        elif f == "SHA256SUMS.txt":
            what = "the SHA-256 checksum of every file (`sha256sum -c --ignore-missing SHA256SUMS.txt`)"
        elif f == "SOURCES.txt":
            what = "what each source file holds, and the exact commits of the libraries built from source"
        elif f.endswith("-source.tar.gz"):
            what = "the add-on's source at the reviewed commit"
        elif f.endswith(".src.tar"):
            what = "the Ubuntu source package of a system library inside the add-on"
        else:
            what = ""
        text.append("- `%s`%s" % (f, ": " + what if what else ""))
    text += ["", "Quillbinder is an independent program, not made by or connected with Literature & Latte. The add-on "
             "is not made by or connected with the makers of the data it can download.", ""]
    with open(out, "w", encoding="utf-8") as f:
        f.write("\n".join(text))
    return 0


def main(argv):
    global SYSTEM
    for flag in ("--windows", "--android"):
        if flag in argv[2:]:
            SYSTEM = flag[2:]
            argv = [a for i, a in enumerate(argv) if not (i >= 2 and a == flag)]
    try:
        if len(argv) == 3 and argv[1] == "check":
            return cmd_check(argv[2])
        if len(argv) == 4 and argv[1] == "cache":
            return cmd_cache(argv[2], argv[3])
        if len(argv) == 3 and argv[1] == "testdata":
            return cmd_testdata(argv[2])
        if len(argv) == 4 and argv[1] == "fetch-testdata":
            return cmd_fetch_testdata(argv[2], argv[3])
        if len(argv) == 5 and argv[1] == "run":
            return cmd_run(argv[2], argv[3], argv[4])
        if len(argv) == 4 and argv[1] == "check-package":
            return cmd_check_package(argv[2], argv[3])
        if len(argv) == 5 and argv[1] == "sources":
            return cmd_sources(argv[2], argv[3], argv[4])
        if len(argv) == 6 and argv[1] == "notes":
            return cmd_notes(argv[2], argv[3], argv[4], argv[5])
    except EntryError as ex:
        print("::error title=The add-on's entry::%s" % ex)
        return 2
    except (OSError, ValueError, KeyError, zipfile.BadZipFile, subprocess.CalledProcessError) as ex:
        print("::error title=%s failed::%s" % (argv[1], ex))
        return 1
    print(__doc__, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv))
