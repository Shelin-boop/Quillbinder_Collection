#!/usr/bin/env python3
"""Checks a finished engine package (0.99.0): "ai-engine" or "handwriting-engine" (addons/ai-engine, addons/handwriting-engine).

  check-engine-package.py <package.qbpkg> ...

For each package: the manifest names the engine and this system's runtime; payload/engine/lib holds the expected libraries;
payload/engine/licenses holds README.txt, which names every library, and the license files; every library needs nothing
outside the package and the base system (glibc's libraries, libstdc++, libgcc_s; libvulkan.so.1 only for ggml's Vulkan
module), seen with ldd on the unpacked package; every library of the AI engine finds its siblings through RUNPATH $ORIGIN.
Prints PASS/FAIL lines; the exit status is the number of failures.
"""
import fnmatch, json, os, re, subprocess, sys, tempfile, zipfile

EXPECTED = {
    "ai-engine": ["libqbai.so.1", "libggml.so.1", "libggml-base.so.1", "libwhisper.so.1", "libllama.so.1", "libmtmd.so.1",
                  "libqwentts.so.0", "libsonic.so.0", "libggml-cpu-*.so"],
    "handwriting-engine": ["libonnxruntime.so.1"],
}
OPTIONAL = {"ai-engine": ["libggml-vulkan.so"], "handwriting-engine": []}
# the license text of each library's own project
LICENSES = {"ai-engine": ["LICENSE-ggml.txt", "LICENSE-whisper.cpp.txt", "LICENSE-llama.cpp.txt", "LICENSE-qwentts.cpp.txt",
                          "LICENSE-sonic.txt"],
            "handwriting-engine": ["LICENSE-onnxruntime.txt"]}
# the base system: what every Linux with glibc has (the AppImage's own rule)
BASE = {"linux-vdso.so.1", "ld-linux-x86-64.so.2", "libc.so.6", "libm.so.6", "libdl.so.2", "libpthread.so.0", "librt.so.1",
        "libstdc++.so.6", "libgcc_s.so.1"}
ONLY_FOR = {"libvulkan.so.1": "libggml-vulkan.so"}

failures = 0


def check(ok, what, detail=""):
    global failures
    print(("PASS  " if ok else "FAIL  ") + what)
    if not ok:
        failures += 1
        if detail:
            print("      " + detail[:2000])


def runpath(path):
    try:
        out = subprocess.run(["readelf", "-d", path], capture_output=True, text=True, timeout=60).stdout
    except (OSError, subprocess.TimeoutExpired):
        return None
    m = re.search(r"\((?:RUNPATH|RPATH)\)\s+Library (?:runpath|rpath): \[([^\]]*)\]", out)
    return m.group(1) if m else ""


def ldd(path):
    out = subprocess.run(["ldd", path], capture_output=True, text=True, timeout=60)
    needs = []
    for line in out.stdout.splitlines():
        line = line.strip()
        if "=>" in line:
            name, rest = [x.strip() for x in line.split("=>", 1)]
            target = rest.split(" (")[0].strip()
            needs.append((name, target))
        elif line:
            needs.append((line.split(" (")[0].strip(), line.split(" (")[0].strip()))
    return needs


def check_package(pkg):
    name = os.path.basename(pkg)
    with zipfile.ZipFile(pkg) as z:
        m = json.loads(z.read("manifest.json"))
        engine = m.get("id", "")
        check(engine in EXPECTED, "%s: the package of an engine (%s)" % (name, engine))
        if engine not in EXPECTED:
            return
        check(m.get("kind") == "addon" and bool(m.get("runtime")), "%s: an add-on with the runtime of its libraries (%s)" % (name, m.get("runtime")))
        listed = {f["path"] for f in m.get("files", [])}
        libs = sorted(os.path.basename(p) for p in listed if p.startswith("engine/lib/"))
        missing = [w for w in EXPECTED[engine] if not fnmatch.filter(libs, w)]
        check(not missing, "%s: engine/lib holds %s" % (name, ", ".join(EXPECTED[engine])), "missing: %s; there: %s" % (missing, libs))
        allowed = EXPECTED[engine] + OPTIONAL[engine]
        extra = [l for l in libs if not any(fnmatch.fnmatch(l, w) for w in allowed)]
        check(not extra, "%s: nothing else in engine/lib" % name, "more: %s" % extra)
        readme = z.read("payload/engine/licenses/README.txt").decode("utf-8") if "engine/licenses/README.txt" in listed else ""
        check(bool(readme), "%s: engine/licenses/README.txt" % name)
        unnamed = [l for l in libs if l not in readme and not (fnmatch.fnmatch(l, "libggml-cpu-*.so") and "libggml-cpu-*.so" in readme)]
        check(not unnamed, "%s: README.txt names every library" % name, "not named: %s" % unnamed)
        licenses = [p for p in listed if p.startswith("engine/licenses/") and not p.endswith("README.txt")]
        have = {os.path.basename(p) for p in licenses}
        lacking = [l for l in LICENSES[engine] if l not in have]
        check(not lacking and len(licenses) >= (6 if engine == "ai-engine" else 1),
              "%s: the license files (%d), each library's own among them" % (name, len(licenses)), "missing: %s" % lacking)
        with tempfile.TemporaryDirectory() as tmp:
            for p in listed:
                if p.startswith("engine/lib/"):
                    z.extract("payload/" + p, tmp)
            libdir = os.path.join(tmp, "payload", "engine", "lib")
            outside, paths = [], []
            for lib in libs:
                full = os.path.join(libdir, lib)
                os.chmod(full, 0o755)
                for needed, target in ldd(full):
                    if needed in BASE or os.path.basename(needed).startswith("ld-linux"):
                        continue
                    if ONLY_FOR.get(needed) == lib:
                        continue
                    if target.startswith(libdir + os.sep):
                        continue
                    outside.append("%s needs %s (%s)" % (lib, needed, target))
                if engine == "ai-engine":
                    rp = runpath(full)
                    if rp is not None and rp != "$ORIGIN":
                        paths.append("%s: %r" % (lib, rp))
            check(not outside, "%s: every library needs nothing outside the package and the base system" % name, "; ".join(outside))
            if engine == "ai-engine":
                check(not paths, "%s: every library has RUNPATH $ORIGIN" % name, "; ".join(paths))


def main(args):
    if not args:
        print(__doc__)
        return 2
    for pkg in args:
        check_package(pkg)
    print("%d failure(s)" % failures)
    return min(failures, 100)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
