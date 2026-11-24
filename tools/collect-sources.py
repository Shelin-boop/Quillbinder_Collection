#!/usr/bin/env python3
"""Collects the source code of every third-party part the AppImage and the add-ons carry (0.50).

    tools/collect-sources.py <out dir>                 # download and pack everything
    tools/collect-sources.py --list                    # the file names only (nothing downloaded)
    tools/collect-sources.py --name ffmpeg             # the source file of the add-ons' FFmpeg
    tools/collect-sources.py --name ubuntu <file>      # "<source package> <version> <source file>"
                                                       # of a library file of this system
    tools/collect-sources.py --of-file <file> <out dir>  # the Ubuntu source package of one library file,
                                                       # packed as <out dir>/<source file> (0.91.3: for the
                                                       # official add-ons' workflow, addon-release.yml)

Each part becomes one file <name>.src.tar in <out dir>; the release workflow puts them into the
release "Third-party source code" of the project page (APP_HOMEPAGE in CMakeLists.txt), where they
stay for every version that used them. Help > Third-Party Licenses and the license notes of the
add-ons name these files.

  - every Ubuntu 24.04 source package of resources/licenses/index.json (the AppImage's libraries),
    in the version this system has (tools/collect-licenses.py --refresh keeps the list in step):
    the .dsc, the original upstream archive and Ubuntu's changes, as "apt-get source" fetches them
    (needs deb-src lines in the apt sources);
  - the Ubuntu source packages of the libraries the add-ons bring (ADDON_LIBRARIES below);
  - FFmpeg 9.1 of the media add-ons (0.93.0; 9.0.2 from 0.80.0, 6.1.1 before): the original archive plus tools/build-ffmpeg.sh,
    tools/get-ffmpeg.sh and tools/ffmpeg-patches (the file name carries a hash of these, so a
    changed build gets a new file);
  - the AppImage runtime (type2-runtime) at the commit named in index.json, with the archives of
    the libraries its build links in (as its scripts/common/install-dependencies.sh downloads them);
  - Qt's source release, and (0.93.1) Qt PDF's: since Qt 6.12 the source release of Qt WebEngine and Qt PDF
    (qtwebengine-everywhere-src-<QB_QTPDF_VERSION>.tar.xz), which Qt's own no longer holds.

A SOURCES.txt in <out dir> lists the files and what each holds.
"""
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MULTIARCH = "/usr/lib/x86_64-linux-gnu"
# 0.93.0: FFmpeg 9.1 from ffmpeg.org; the version, the pin and the signing key are the ones of tools/get-ffmpeg.sh
def _get_ffmpeg_setting(name):
    with open(os.path.join(ROOT, "tools", "get-ffmpeg.sh")) as f:
        m = re.search(r'^%s="?([^"\n]*)"?$' % name, f.read(), re.M)
    return m.group(1) if m else ""


FFMPEG_VERSION = _get_ffmpeg_setting("VERSION")
FFMPEG_ORIG = "ffmpeg-%s.tar.xz" % FFMPEG_VERSION
FFMPEG_URL = "https://ffmpeg.org/releases/" + FFMPEG_ORIG
FFMPEG_SHA256 = _get_ffmpeg_setting("SHA256")   # empty until pinned: then the release signature decides
FFMPEG_KEY = _get_ffmpeg_setting("KEY_FPR")
RUNTIME_REPO = "https://github.com/AppImage/type2-runtime"

# library files of this system that add-ons carry (addons/*/stage.sh); since 0.80.0 the add-ons take Qt's parts
# (Qt Multimedia, the TLS plugin) from Qt's own build, whose source is the Qt entry below
# 0.92.0: the GNU OpenMP runtime the add-on "Translation and Proofreading" brings (Ubuntu's libgomp1, unchanged)
ADDON_LIBRARIES = ["/usr/lib/x86_64-linux-gnu/libgomp.so.1"]
QT_URL = "https://download.qt.io/official_releases/qt/%s/%s/single/%s"


def qt_version():
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r'^set\(QB_QT_VERSION\s+"([^"]*)"\)', f.read(), re.M)
    return m.group(1) if m else ""


def qt_source_file():
    return "qt-everywhere-src-%s.tar.xz" % qt_version()


# 0.93.1 (to-do item 48): Qt PDF's source. Since Qt 6.12 Qt PDF is released with Qt WebEngine as a Qt Extensions module
# of its own version (QB_QTPDF_VERSION), and qt-everywhere-src no longer holds it. The Qt Project publishes an MD5 list
# beside the archive (no SHA-256): the archive is checked against it, and its SHA-256 is printed.
QTPDF_URL = "https://download.qt.io/official_releases/qtwebengine/%s/%s/src/%s"


def qtpdf_version():
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r'^set\(QB_QTPDF_VERSION\s+"([^"]*)"\)', f.read(), re.M)
    return m.group(1) if m else ""


def qtpdf_source_file():
    return "qtwebengine-everywhere-src-%s.tar.xz" % qtpdf_version()


def sh(*args, cwd=None, check=False):
    r = subprocess.run(args, capture_output=True, text=True, cwd=cwd)
    if check and r.returncode != 0:
        raise RuntimeError("%s failed:\n%s%s" % (" ".join(args), r.stdout, r.stderr))
    return r.stdout


def safe_name(s):
    return re.sub(r"[^A-Za-z0-9._-]", "_", s)


def ubuntu_source_file(src, version):
    return "%s_%s.src.tar" % (safe_name(src), safe_name(version))


def ffmpeg_source_file():
    h = hashlib.sha256()
    files = [os.path.join(ROOT, "tools", "build-ffmpeg.sh")]
    pdir = os.path.join(ROOT, "tools", "ffmpeg-patches")
    files += [os.path.join(pdir, n) for n in sorted(os.listdir(pdir))]
    for f in files:
        h.update(os.path.basename(f).encode() + b"\0")
        with open(f, "rb") as fh:
            h.update(fh.read())
    return "ffmpeg-%s-quillbinder-%%s.src.tar" % FFMPEG_VERSION % h.hexdigest()[:8], files


def package_of(path):
    for p in (path, os.path.realpath(path)):
        for line in sh("dpkg", "-S", p).splitlines():
            pkg, _, f = line.partition(": ")
            if f.strip() == p:
                return pkg.split(":")[0]
    return None


def field(pkg, f):
    return sh("dpkg-query", "-W", "-f", "${%s}" % f, pkg).strip()


def ubuntu_of_file(path):
    pkg = package_of(path)
    if not pkg:
        return None
    src = field(pkg, "source:Package") or pkg
    ver = field(pkg, "source:Version") or field(pkg, "Version")
    return src, ver


def wanted():
    """[(file name, kind, details)] of everything to collect."""
    index = json.load(open(os.path.join(ROOT, "resources", "licenses", "index.json")))
    out, seen = [], set()
    for c in index["components"]:
        if c.get("package"):
            key = (c["package"], c["version"])
            if key not in seen:
                seen.add(key)
                out.append((c["sourceFile"], "ubuntu", {"package": c["package"], "version": c["version"],
                                                       "for": "the AppImage: " + c["where"]}))
    for lib in ADDON_LIBRARIES:
        if not os.path.exists(lib):
            print("note: %s is not installed; its source is left out" % lib, file=sys.stderr)
            continue
        src, ver = ubuntu_of_file(lib)
        if (src, ver) not in seen:
            seen.add((src, ver))
            out.append((ubuntu_source_file(src, ver), "ubuntu", {"package": src, "version": ver,
                                                                 "for": "add-ons: " + os.path.basename(lib)}))
        else:
            for f in out:
                if f[2].get("package") == src and f[2].get("version") == ver:
                    f[2]["for"] += ", add-ons: " + os.path.basename(lib)
    if any(c.get("qtBuild") for c in index["components"]):
        out.append((qt_source_file(), "qt", {"version": qt_version(),
                                             "for": "the AppImage (Qt) and the add-ons Media Playback and Web Page Import"}))
    if any(c.get("sourceFile") == qtpdf_source_file() for c in index["components"]):
        out.append((qtpdf_source_file(), "qtpdf", {"version": qtpdf_version(),
                                                   "for": "the AppImage (Qt PDF: the PDF preview and the PDF files of Help)"}))
    name, _files = ffmpeg_source_file()
    out.append((name, "ffmpeg", {"for": "the add-ons Media Playback and Extra Video and Audio Formats"}))
    rt = index.get("runtime")
    if rt:
        out.append((rt["sourceFile"], "runtime", {"commit": rt["commit"], "for": "the AppImage runtime"}))
    else:
        print("note: index.json names no runtime (tools/collect-licenses.py --refresh <dir> <runtime>)",
              file=sys.stderr)
    return out


def pack(out_file, folder):
    tmp = out_file + ".part"
    with tarfile.open(tmp, "w") as t:
        for n in sorted(os.listdir(folder)):
            t.add(os.path.join(folder, n), arcname=os.path.join(os.path.basename(out_file)[:-len(".src.tar")], n))
    os.replace(tmp, out_file)


def get_ubuntu(d, work):
    sh("apt-get", "source", "--download-only", "-qq", "%s=%s" % (d["package"], d["version"]), cwd=work, check=True)
    if not any(n.endswith(".dsc") for n in os.listdir(work)):
        raise RuntimeError("apt-get source gave no .dsc for %s %s" % (d["package"], d["version"]))


def get_qt(d, target):
    ver = d["version"]
    url = QT_URL % (ver.rsplit(".", 1)[0], ver, os.path.basename(target))
    sh("curl", "-fsSL", "--retry", "10", "--retry-delay", "15", "--retry-all-errors", "-o", target + ".part", url, check=True)
    # the Qt Project publishes a SHA-256 beside its archives
    r = subprocess.run(["curl", "-fsSL", "--retry", "5", url + ".sha256"], capture_output=True, text=True)
    if r.returncode == 0 and r.stdout.split():
        h = hashlib.sha256()
        with open(target + ".part", "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
        if h.hexdigest() != r.stdout.split()[0].lower():
            os.remove(target + ".part")
            raise RuntimeError("%s does not have the SHA-256 the Qt Project publishes" % url)
    os.replace(target + ".part", target)


def get_qtpdf(d, target):
    ver = d["version"]
    name = os.path.basename(target)
    url = QTPDF_URL % (ver.rsplit(".", 1)[0], ver, name)
    sh("curl", "-fsSL", "--retry", "10", "--retry-delay", "15", "--retry-all-errors", "-o", target + ".part", url, check=True)
    r = subprocess.run(["curl", "-fsSL", "--retry", "5", url.rsplit("/", 1)[0] + "/md5sums.txt"], capture_output=True,
                       text=True)
    want = [l.split()[0].lower() for l in r.stdout.splitlines() if l.split() and l.split()[-1].lstrip("*") == name]
    md5, sha = hashlib.md5(), hashlib.sha256()
    with open(target + ".part", "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            md5.update(chunk)
            sha.update(chunk)
    if r.returncode != 0 or not want:
        os.remove(target + ".part")
        raise RuntimeError("%s: the Qt Project's md5sums.txt beside it names no sum for it" % url)
    if md5.hexdigest() != want[0]:
        os.remove(target + ".part")
        raise RuntimeError("%s does not have the MD5 sum the Qt Project publishes" % url)
    print("%s: MD5 as published; SHA-256 %s" % (name, sha.hexdigest()), flush=True)
    os.replace(target + ".part", target)


def get_ffmpeg(work):
    cached = os.path.join(ROOT, "build", "ffmpeg-src", FFMPEG_ORIG)
    dst = os.path.join(work, FFMPEG_ORIG)
    if os.path.exists(cached):
        shutil.copy(cached, dst)
    else:
        sh("curl", "-fsSL", "-o", dst, FFMPEG_URL, check=True)
    if FFMPEG_SHA256:
        with open(dst, "rb") as f:
            if hashlib.sha256(f.read()).hexdigest() != FFMPEG_SHA256:
                raise RuntimeError("FFmpeg archive: wrong checksum")
    else:
        # no pin yet: only a good release signature of FFmpeg's key is accepted (tools/verify-ffmpeg-source.sh)
        sh("curl", "-fsSL", "-o", dst + ".asc", FFMPEG_URL + ".asc", check=True)
        sh("curl", "-fsSL", "-o", dst + ".keys.asc", "https://ffmpeg.org/ffmpeg-devel.asc", check=True)
        sh(os.path.join(ROOT, "tools", "verify-ffmpeg-source.sh"), dst, dst + ".asc", dst + ".keys.asc", FFMPEG_KEY, check=True)
    _name, files = ffmpeg_source_file()
    os.makedirs(os.path.join(work, "quillbinder-build", "ffmpeg-patches"))
    for f in files + [os.path.join(ROOT, "tools", "get-ffmpeg.sh"), os.path.join(ROOT, "tools", "verify-ffmpeg-source.sh")]:
        sub = "ffmpeg-patches" if os.path.basename(os.path.dirname(f)) == "ffmpeg-patches" else ""
        shutil.copy(f, os.path.join(work, "quillbinder-build", sub, os.path.basename(f)))
    with open(os.path.join(work, "README.txt"), "w") as f:
        f.write("FFmpeg %s as built for the Quillbinder add-ons \"Media Playback\" (set \"base\") and\n"
                "\"Extra Video and Audio Formats\" (set \"full\").\n\n"
                "%s  the unchanged FFmpeg source of ffmpeg.org (its release signature checked)\n"
                "quillbinder-build/                 how it was built: get-ffmpeg.sh unpacks the archive and calls\n"
                "                                   build-ffmpeg.sh, which applies ffmpeg-patches/ and configures\n"
                "                                   each set (the options are in that script)\n" % (FFMPEG_VERSION, FFMPEG_ORIG))


def get_runtime(d, work):
    repo = os.path.join(work, "..", "type2-runtime-git")
    if not os.path.exists(repo):
        sh("git", "clone", "-q", "--filter=blob:none", RUNTIME_REPO, repo, check=True)
    full = sh("git", "rev-parse", d["commit"] + "^{commit}", cwd=repo, check=True).strip()
    sh("git", "archive", "--format=tar.gz", "--prefix=type2-runtime-%s/" % d["commit"],
       "-o", os.path.join(work, "type2-runtime-%s.tar.gz" % d["commit"]), full, cwd=repo, check=True)
    script = sh("git", "show", full + ":scripts/common/install-dependencies.sh", cwd=repo, check=True)
    urls = re.findall(r"wget\s+\"?(https://[^\s\"]+)", script)
    sums = dict((n, h) for h, n in re.findall(r"echo\s+\"([0-9a-f]{64})\s+([^\"]+)\"\s*\|\s*sha256sum", script))
    os.makedirs(os.path.join(work, "deps"))
    notes = []
    for u in urls:
        name = u.rsplit("/", 1)[1]
        dst = os.path.join(work, "deps", name)
        r = subprocess.run(["curl", "-fsSL", "-o", dst, u], capture_output=True, text=True)
        m = re.match(r"https://github\.com/([^/]+)/([^/]+)/archive/(.+)\.tar\.gz$", u)
        if r.returncode != 0 and m:
            # GitHub's archive service unreachable: the same tag through git
            owner, repo_name, ref = m.groups()
            clone = os.path.join(work, "..", "git-" + repo_name)
            sh("git", "clone", "-q", "--depth", "1", "--branch", ref,
               "https://github.com/%s/%s" % (owner, repo_name), clone, check=True)
            sh("git", "archive", "--format=tar.gz", "--prefix=%s-%s/" % (repo_name, ref), "-o", dst, "HEAD",
               cwd=clone, check=True)
            notes.append("%s was made with git archive from the tag %s (%s could not be fetched)" % (name, ref, u))
            continue
        if r.returncode != 0:
            raise RuntimeError("curl %s failed: %s" % (u, r.stderr))
        if name in sums:
            with open(dst, "rb") as f:
                if hashlib.sha256(f.read()).hexdigest() != sums[name]:
                    raise RuntimeError("%s: wrong checksum" % name)
    with open(os.path.join(work, "README.txt"), "w") as f:
        f.write("The AppImage runtime (the start code at the beginning of the Quillbinder AppImage file):\n"
                "type2-runtime at commit %s (%s/commit/%s), MIT license.\n\n"
                "deps/ holds the archives its build downloads and links in statically (see\n"
                "scripts/common/install-dependencies.sh inside the type2-runtime archive), among them\n"
                "libfuse (LGPL 2.1) and squashfuse (BSD). The other parts it links in (musl, zstd, zlib,\n"
                "mimalloc) come from Alpine Linux packages of the release named in its build scripts.\n"
                "Build: scripts/chroot/build.sh or scripts/docker/build-with-docker.sh of type2-runtime.\n"
                % (full, RUNTIME_REPO, full))
        f.write("\nFiles in deps/:\n" + "".join("  %s  (%s)\n" % (u.rsplit("/", 1)[1], u) for u in urls))
        for n in notes:
            f.write("Note: %s.\n" % n)


def main(argv):
    if len(argv) == 2 and argv[1] == "--list":
        for name, kind, d in wanted():
            print(name)
        return 0
    if len(argv) == 3 and argv[1] == "--name" and argv[2] == "ffmpeg":
        print(ffmpeg_source_file()[0])
        return 0
    if len(argv) == 3 and argv[1] == "--name" and argv[2] == "qt":
        print(qt_source_file())
        return 0
    if len(argv) == 3 and argv[1] == "--name" and argv[2] == "qtpdf":
        print(qtpdf_source_file())
        return 0
    if len(argv) == 4 and argv[1] == "--name" and argv[2] == "ubuntu":
        r = ubuntu_of_file(argv[3])
        if not r:
            print("no package owns %s" % argv[3], file=sys.stderr)
            return 1
        print(r[0], r[1], ubuntu_source_file(*r))
        return 0
    if len(argv) == 4 and argv[1] == "--of-file":
        r = ubuntu_of_file(argv[2])
        if not r:
            print("no package owns %s" % argv[2], file=sys.stderr)
            return 1
        out = os.path.abspath(argv[3])
        os.makedirs(out, exist_ok=True)
        target = os.path.join(out, ubuntu_source_file(*r))
        work = tempfile.mkdtemp(prefix="qb-source-")
        try:
            get_ubuntu({"package": r[0], "version": r[1]}, work)
            pack(target, work)
        finally:
            shutil.rmtree(work, ignore_errors=True)
        print(os.path.basename(target))
        return 0
    if len(argv) != 2 or argv[1].startswith("-"):
        print(__doc__, file=sys.stderr)
        return 2
    out = os.path.abspath(argv[1])
    os.makedirs(out, exist_ok=True)
    items = wanted()
    lines = []
    tmp_root = tempfile.mkdtemp(prefix="qb-sources-")
    try:
        for name, kind, d in items:
            target = os.path.join(out, name)
            if kind == "ubuntu":
                what = "Ubuntu 24.04 source package %s %s (for %s)" % (d["package"], d["version"], d["for"])
            elif kind == "ffmpeg":
                what = "FFmpeg %s and the Quillbinder build of it (for %s)" % (FFMPEG_VERSION, d["for"])
            elif kind == "qt":
                what = "Qt %s, the Qt Project's source release, unchanged (for %s)" % (d["version"], d["for"])
            elif kind == "qtpdf":
                what = ("Qt WebEngine and Qt PDF %s, the Qt Project's source release, unchanged (for %s)"
                        % (d["version"], d["for"]))
            else:
                what = "AppImage runtime type2-runtime %s with its libraries (for %s)" % (d["commit"], d["for"])
            lines.append("%s\n    %s\n" % (name, what))
            if os.path.exists(target):
                print("have %s" % name)
                continue
            if kind == "qt":
                # the Qt Project's own archive, as it is (not packed again)
                print("collecting %s" % name, flush=True)
                get_qt(d, target)
                continue
            if kind == "qtpdf":
                print("collecting %s" % name, flush=True)
                get_qtpdf(d, target)
                continue
            work = os.path.join(tmp_root, name)
            os.makedirs(work)
            print("collecting %s" % name, flush=True)
            if kind == "ubuntu":
                get_ubuntu(d, work)
            elif kind == "ffmpeg":
                get_ffmpeg(work)
            else:
                get_runtime(d, work)
            pack(target, work)
            shutil.rmtree(work)
    finally:
        shutil.rmtree(tmp_root, ignore_errors=True)
    with open(os.path.join(out, "SOURCES.txt"), "w") as f:
        f.write("Source code of the third-party parts of Quillbinder's AppImage and add-ons\n"
                "=========================================================================\n\n"
                "Each file is a tar archive (tar xf <file>); the license texts are in the program under\n"
                "Help > Third-Party Licenses and in the add-ons' licenses folders.\n\n")
        f.write("\n".join(lines))
    print("%d source files in %s" % (len(items), out))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
