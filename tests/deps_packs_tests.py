#!/usr/bin/env python3
"""Suite deps (0.93.0): the dependency packs of the Linux build (tools/deps-pack.sh, tools/get-ffmpeg.sh,
tools/get-whisper.sh, tools/get-translator-deps.sh, tools/deps-packs.txt, .github/workflows/deps.yml), offline: the
recipes, the packs' texts (what is inside, its license, its source), the workflow's matrix and push filter, and a pack
of the translator's libraries made and put back from a made-up folder.

  python3 -I tests/deps_packs_tests.py
"""
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


def read(rel):
    with open(os.path.join(ROOT, rel), encoding="utf-8") as fh:
        return fh.read()


def pack(*args, env=None):
    return subprocess.run([os.path.join(ROOT, "tools", "deps-pack.sh")] + list(args), capture_output=True, text=True,
                          env=dict(os.environ, **(env or {})))


NAMES = ("ffmpeg", "whisper", "translator", "crypto", "onnxruntime", "aiengine")
dp = read("tools/deps-pack.sh")
wf = read(".github/workflows/deps.yml")

# ---------------------------------------------------------------------------------------------------- recipes and texts
hashes = {}
for n in NAMES:
    r = pack("recipe", n)
    h = pack("hash", n)
    hashes[n] = h.stdout.strip()
    check(f"the recipe of {n} names its pack, the machine and the files its hash comes from",
          r.returncode == 0 and f"pack: {n} linux format" in r.stdout and "machine:" in r.stdout and "file tools/get-" in r.stdout
          and re.fullmatch(r"[0-9a-f]{64}", hashes[n]) is not None, r.stdout + r.stderr)
check("the six recipes differ", len(set(hashes.values())) == 6)
check("the translator's recipe holds its two scripts",
      "file tools/get-translator-deps.sh " in pack("recipe", "translator").stdout
      and "file addons/translation-proofreading/source/packaging/build-deps.sh " in pack("recipe", "translator").stdout)
check("the new recipes hold their scripts",
      "file tools/get-crypto.sh " in pack("recipe", "crypto").stdout
      and "file tools/get-onnxruntime.sh " in pack("recipe", "onnxruntime").stdout
      and "file tools/get-ai-engine.sh " in pack("recipe", "aiengine").stdout
      and "file tools/ai-engine/qwentts/CMakeLists.txt " in pack("recipe", "aiengine").stdout)
check("the FFmpeg recipe holds the signature script", "file tools/verify-ffmpeg-source.sh " in pack("recipe", "ffmpeg").stdout)
for n, words in (("ffmpeg", ("FFmpeg 9.1", "LGPL-2.1-or-later", "https://ffmpeg.org/releases/ffmpeg-9.1.tar.xz")),
                 ("whisper", ("whisper.cpp", "MIT", "https://github.com/ggml-org/whisper.cpp")),
                 ("translator", ("oneDNN", "CTranslate2", "SentencePiece", "Apache-2.0", "MIT", "BSD-3-Clause",
                                 "https://github.com/OpenNMT/CTranslate2")),
                 ("crypto", ("Botan 3.14.1", "RNP v0.18.1", "json-c", "bzip2", "BSD-2-Clause", "https://github.com/rnpgp/rnp")),
                 ("onnxruntime", ("ONNX Runtime 1.32.0", "MIT", "Maven Central", "com.microsoft.onnxruntime:onnxruntime")),
                 ("aiengine", ("ggml v1.1.1", "whisper.cpp v1.9.6", "llama.cpp b12808", "qwentts.cpp", "Sonic", "MIT", "Apache-2.0",
                               "https://github.com/ggml-org/llama.cpp"))):
    a = pack("about", n)
    check(f"the text of the {n} pack says what is inside, its license and its source",
          a.returncode == 0 and a.stdout.count("\n") == 2 and a.stdout.startswith("Inside:") and "\nLicense" in a.stdout
          and "Source" in a.stdout and all(w in a.stdout for w in words), a.stdout + a.stderr)
check("the pack of a name that does not exist is refused", pack("about", "nothing").returncode != 0)
check("the repository with the packs defaults to this one",
      re.search(r"^REPO=\$\{QB_DEPS_REPO:-Shelin-boop/Quillbinder_Collection\}$", dp, re.M) is not None)

# ---------------------------------------------------------------------------------------------------- the workflow
check("the workflow builds the six packs",
      re.search(r"pack: \[ffmpeg, whisper, translator, crypto, onnxruntime, aiengine\]", wf) is not None)
for f in ("tools/get-ffmpeg.sh", "tools/build-ffmpeg.sh", "tools/verify-ffmpeg-source.sh", "tools/get-whisper.sh",
          "tools/get-translator-deps.sh", "addons/translation-proofreading/source/packaging/build-deps.sh",
          "tools/get-crypto.sh", "tools/get-onnxruntime.sh", "tools/get-ai-engine.sh", "tools/ai-engine/**",
          "tools/deps-pack.sh"):
    check(f"a change of {f} starts the workflow", f"      - '{f}'\n" in wf)
check("the workflow runs the six scripts and packs the six",
      all(s in wf for s in ("ffmpeg) script=tools/get-ffmpeg.sh", "whisper) script=tools/get-whisper.sh",
                            "translator) script=tools/get-translator-deps.sh",
                            "translator) tools/deps-pack.sh pack translator out build/translator-deps",
                            "crypto) script=tools/get-crypto.sh", "onnxruntime) script=tools/get-onnxruntime.sh",
                            "aiengine) script=tools/get-ai-engine.sh",
                            "crypto) tools/deps-pack.sh pack crypto out build/crypto",
                            "onnxruntime) tools/deps-pack.sh pack onnxruntime out build/onnxruntime",
                            "aiengine) tools/deps-pack.sh pack aiengine out build/ai-engine")))
check("the ONNX Runtime step prints the SHA-256 for the pin and the workflow allows the first run",
      "QB_ONNXRUNTIME_FIRST_RUN: '1'" in wf and "pin this" in wf and 'PACK" = onnxruntime' in wf)
check("the pre-release's text names what is inside, the license and the source",
      'tools/deps-pack.sh about "$PACK" > "out/about-$PACK.txt"' in wf and 'cat "$d/about-$p.txt"' in wf)
check("the FFmpeg step prints the SHA-256 of the signed source for the pin",
      "pin this" in wf and "SHA-256 of" in wf and "verify-ffmpeg-source:" in wf)
check("only the job publish may write", len(re.findall(r"contents: write", wf)) == 1
      and re.search(r"^permissions:\n  contents: read\n", wf, re.M) is not None)
for s in ("get-ffmpeg.sh", "get-whisper.sh", "get-translator-deps.sh", "get-crypto.sh", "get-onnxruntime.sh", "get-ai-engine.sh"):
    check(f"{s} asks for a pack first and builds from source without one",
          '"$ROOT/tools/deps-pack.sh" fetch' in read(f"tools/{s}") and "QB_DEPS_PACK=0" in read(f"tools/{s}"))

# ---------------------------------------------------------------------------------------------------- the license files
mk = read("addons/translation-proofreading/source/packaging/make-addon.sh")
used = sorted(re.findall(r'cp "\$DEPS/([^"]+)" "\$L/', mk))
listed = sorted(re.search(r'^TP_LICENSES="(.*?)"$', dp, re.M | re.S).group(1).split())
check("the license files in the translator's pack are those the add-on's make-addon.sh copies", used == listed,
      set(used) ^ set(listed))

# ---------------------------------------------------------------------------------------------------- a pack made and used
work = tempfile.mkdtemp(prefix="qb-deps-")
try:
    src = os.path.join(work, "translator-deps")
    os.makedirs(os.path.join(src, "install", "lib"))
    os.makedirs(os.path.join(src, "install", "lib", "cmake"))
    for f in ("install/lib/libctranslate2.a", "install/lib/libsentencepiece.a"):
        open(os.path.join(src, f), "w").write("x")
    open(os.path.join(src, "install/lib/cmake/c.cmake"), "w").write(f"set(P {src}/install)\n")
    for f in listed:
        os.makedirs(os.path.dirname(os.path.join(src, f)), exist_ok=True)
        open(os.path.join(src, f), "w").write("license " + f)
    os.makedirs(os.path.join(src, "oneDNN", "src"), exist_ok=True)
    open(os.path.join(src, "oneDNN", "src", "big.cpp"), "w").write("not in the pack")
    out = os.path.join(work, "out")
    p = pack("pack", "translator", out, src)
    files = os.listdir(out) if os.path.isdir(out) else []
    tarname = next((f for f in files if f.endswith(".tar.xz")), "")
    check("the translator's pack is made, with its recipe, hash and pin line",
          p.returncode == 0 and tarname.startswith("translator-linux-") and p.stdout.startswith("pin: translator ")
          and "recipe-translator.txt" in files and "hash-translator.txt" in files, p.stdout + p.stderr)
    listing = subprocess.run(["tar", "-tJf", os.path.join(out, tarname)], capture_output=True, text=True).stdout if tarname else ""
    check("the pack holds the libraries and the license files, not the rest of the sources",
          "install/lib/libctranslate2.a" in listing and "translator/oneDNN/LICENSE" in listing
          and "big.cpp" not in listing, listing)
    # fetch: pinned in a list of packs of a copy of the scripts, downloaded by a gh that is made up here
    fake = os.path.join(work, "repo")
    os.makedirs(os.path.join(fake, "tools"))
    for f in ("deps-pack.sh", "get-translator-deps.sh", "get-ffmpeg.sh", "get-whisper.sh", "verify-ffmpeg-source.sh"):
        shutil.copy(os.path.join(ROOT, "tools", f), os.path.join(fake, "tools", f))
    os.makedirs(os.path.join(fake, "addons/translation-proofreading/source/packaging"))
    shutil.copy(os.path.join(ROOT, "addons/translation-proofreading/source/packaging/build-deps.sh"),
                os.path.join(fake, "addons/translation-proofreading/source/packaging/build-deps.sh"))
    h = subprocess.run([os.path.join(fake, "tools", "deps-pack.sh"), "hash", "translator"], capture_output=True, text=True).stdout.strip()
    pin = open(os.path.join(out, "pin-translator.txt")).read().split()
    # (the made-up recipe differs from the real one only in the machine's tools: the pack is named by the fake's hash)
    shutil.copy(os.path.join(out, tarname), os.path.join(work, "asset.tar.xz"))
    sha = subprocess.run(["sha256sum", os.path.join(work, "asset.tar.xz")], capture_output=True, text=True).stdout.split()[0]
    fname = f"translator-linux-{h[:16]}.tar.xz"
    open(os.path.join(fake, "tools", "deps-packs.txt"), "w").write(f"translator {h} {fname} {sha}\n")
    bindir = os.path.join(work, "bin")
    os.makedirs(bindir)
    gh = os.path.join(bindir, "gh")
    open(gh, "w").write(f"""#!/bin/bash
case "$*" in
  *"--jq"*) echo 4242 ;;
  *releases/assets/4242*) cat "{work}/asset.tar.xz" ;;
  *) exit 1 ;;
esac
""")
    os.chmod(gh, 0o755)
    env = dict(os.environ, PATH=bindir + os.pathsep + os.environ["PATH"], QB_DEPS_PACK="1")
    dest = os.path.join(work, "restored")
    os.makedirs(dest)
    open(os.path.join(dest, "old.txt"), "w").write("the old folder goes")
    r = subprocess.run([os.path.join(fake, "tools", "deps-pack.sh"), "fetch", "translator", dest], capture_output=True, text=True, env=env)
    check("fetch: the pinned pack is downloaded, its SHA-256 checked and put in place of the folder",
          r.returncode == 0 and "SHA-256 checked" in r.stdout and os.path.isfile(os.path.join(dest, "install/lib/libctranslate2.a"))
          and os.path.isfile(os.path.join(dest, "sentencepiece/third_party/absl/LICENSE"))
          and not os.path.exists(os.path.join(dest, "old.txt")) and not os.path.exists(os.path.join(dest, "prefix.txt")),
          r.stdout + r.stderr)
    cm = open(os.path.join(dest, "install/lib/cmake/c.cmake")).read()
    check("fetch: a path of the build machine inside a CMake file is made the new folder's", dest in cm and src not in cm, cm)
    open(os.path.join(fake, "tools", "deps-packs.txt"), "w").write(f"translator {h} {fname} {'0' * 64}\n")
    shutil.rmtree(dest)
    r = subprocess.run([os.path.join(fake, "tools", "deps-pack.sh"), "fetch", "translator", dest], capture_output=True, text=True, env=env)
    check("fetch: a pack with another SHA-256 than the pinned one is not used", r.returncode == 1 and not os.path.exists(dest)
          and "does not have the SHA-256" in r.stderr + r.stdout, r.stdout + r.stderr)
    r = subprocess.run([os.path.join(fake, "tools", "deps-pack.sh"), "fetch", "translator", dest], capture_output=True, text=True,
                       env=dict(env, QB_DEPS_PACK="0"))
    check("QB_DEPS_PACK=0: nothing is downloaded", r.returncode == 1 and "building translator from source" in r.stdout)
    # the three packs of 0.99.0: made from made-up prefixes, and the ONNX Runtime one put back through fetch
    fakes = {"crypto": ["lib/librnp.a", "lib/libbotan-3.a", "include/rnp/rnp.h", "licenses/LICENSE-botan.txt"],
             "onnxruntime": ["lib/libonnxruntime.so", "lib/libonnxruntime.so.1.32.0", "include/onnxruntime_c_api.h",
                             "licenses/LICENSE-onnxruntime.txt"],
             "aiengine": ["lib/libggml.so", "lib/libsonic.so", "lib/libggml-cpu-haswell.so", "include/ggml.h",
                          "licenses/LICENSE-ggml.txt"]}
    for name, files in fakes.items():
        pre = os.path.join(work, "prefix-" + name)
        for f in files:
            os.makedirs(os.path.dirname(os.path.join(pre, f)), exist_ok=True)
            open(os.path.join(pre, f), "w").write("x")
        open(os.path.join(pre, "version.txt"), "w").write("made up")
        open(os.path.join(pre, "lib", "libx.pc"), "w").write(f"prefix={pre}\n") if False else None
        o = os.path.join(work, "out-" + name)
        p = pack("pack", name, o, pre)
        tn = next((f for f in os.listdir(o) if f.endswith(".tar.xz")), "") if os.path.isdir(o) else ""
        lst = subprocess.run(["tar", "-tJf", os.path.join(o, tn)], capture_output=True, text=True).stdout if tn else ""
        check(f"the {name} pack is made with its files and its pin line",
              p.returncode == 0 and tn.startswith(f"{name}-linux-") and p.stdout.startswith(f"pin: {name} ")
              and all(f"{name}/{f}" in lst for f in files), p.stdout + p.stderr + lst)
    pre = os.path.join(work, "prefix-empty")
    os.makedirs(pre)
    check("a pack of an unfinished prefix is refused", all(pack("pack", n, os.path.join(work, "out-x"), pre).returncode != 0
                                                          for n in ("crypto", "onnxruntime", "aiengine")))
    # fetch of the ONNX Runtime pack with a made-up gh
    fake2 = os.path.join(work, "repo2")
    os.makedirs(os.path.join(fake2, "tools"))
    for f in ("deps-pack.sh", "get-onnxruntime.sh"):
        shutil.copy(os.path.join(ROOT, "tools", f), os.path.join(fake2, "tools", f))
    # (the copy runs as an unpinned recipe: the real script's pins are blanked, so the refusal and the first run can be checked)
    ort = os.path.join(fake2, "tools", "get-onnxruntime.sh")
    text = re.sub(r'^(SHA256_(JAR|AAR))="[0-9a-f]*"$', r'\1=""', open(ort).read(), flags=re.M)
    open(ort, "w").write(text)
    os.chmod(ort, 0o755)
    h2 = subprocess.run([os.path.join(fake2, "tools", "deps-pack.sh"), "hash", "onnxruntime"], capture_output=True, text=True).stdout.strip()
    shutil.copy(os.path.join(work, "out-onnxruntime", next(f for f in os.listdir(os.path.join(work, "out-onnxruntime")) if f.endswith(".tar.xz"))),
                os.path.join(work, "asset2.tar.xz"))
    sha2 = subprocess.run(["sha256sum", os.path.join(work, "asset2.tar.xz")], capture_output=True, text=True).stdout.split()[0]
    open(os.path.join(fake2, "tools", "deps-packs.txt"), "w").write(f"onnxruntime {h2} onnxruntime-linux-{h2[:16]}.tar.xz {sha2}\n")
    open(gh, "w").write(f"""#!/bin/bash
case "$*" in
  *"--jq"*) echo 4243 ;;
  *releases/assets/4243*) cat "{work}/asset2.tar.xz" ;;
  *) exit 1 ;;
esac
""")
    dest2 = os.path.join(work, "restored-onnx")
    r = subprocess.run([os.path.join(fake2, "tools", "deps-pack.sh"), "fetch", "onnxruntime", dest2], capture_output=True, text=True, env=env)
    check("fetch: the ONNX Runtime pack is downloaded, checked and put in place",
          r.returncode == 0 and "SHA-256 checked" in r.stdout and os.path.isfile(os.path.join(dest2, "include/onnxruntime_c_api.h"))
          and os.path.isfile(os.path.join(dest2, "lib/libonnxruntime.so")) and not os.path.exists(os.path.join(dest2, "prefix.txt")),
          r.stdout + r.stderr)
    # get-onnxruntime.sh without a pinned SHA-256 and without the first-run switch refuses to download
    r = subprocess.run([os.path.join(fake2, "tools", "get-onnxruntime.sh"), os.path.join(work, "no-pin")], capture_output=True, text=True,
                       env=dict(env, QB_DEPS_PACK="0", QB_ONNXRUNTIME_FIRST_RUN="0"))
    check("get-onnxruntime.sh: no SHA-256 pinned, no first-run switch: nothing is downloaded",
          r.returncode == 1 and "no SHA-256 is pinned" in r.stderr and not os.path.exists(os.path.join(work, "no-pin")), r.stdout + r.stderr)
    # the first run against a made-up Maven repository: the newest plain release of both packages, both files' file lists,
    # the pin line, the pack's files and the license text from the POM (neither file carries one)
    import zipfile
    mv = os.path.join(work, "maven")
    base = "com/microsoft/onnxruntime"
    for art, ext, entries in (("onnxruntime", "jar", {"ai/onnxruntime/native/linux-x64/libonnxruntime.so": open("/bin/true", "rb").read(),
                                                      "ai/onnxruntime/native/win-x64/onnxruntime.dll": b"dll"}),
                              ("onnxruntime-android", "aar", {"headers/onnxruntime_c_api.h": b"/* h */",
                                                              "jni/arm64-v8a/libonnxruntime.so": b"so"})):
        d = os.path.join(mv, base, art)
        os.makedirs(os.path.join(d, "1.32.0"))
        with open(os.path.join(d, "maven-metadata.xml"), "w") as fh:
            fh.write("<metadata><versioning><versions><version>1.31.0</version><version>1.32.0</version>"
                     "<version>1.33.0-rc1</version></versions></versioning></metadata>")
        with zipfile.ZipFile(os.path.join(d, "1.32.0", f"{art}-1.32.0.{ext}"), "w") as z:
            for n, data in entries.items():
                z.writestr(n, data)
    open(os.path.join(mv, base, "onnxruntime", "1.32.0", "onnxruntime-1.32.0.pom"), "w").write(
        "<project><licenses><license><name>MIT License</name></license></licenses></project>")
    pre = os.path.join(work, "ort-first")
    r = subprocess.run([os.path.join(fake2, "tools", "get-onnxruntime.sh"), pre], capture_output=True, text=True,
                       env=dict(env, QB_DEPS_PACK="0", QB_ONNXRUNTIME_FIRST_RUN="1", QB_MAVEN_URL="file://" + mv))
    check("get-onnxruntime.sh, first run: both versions, the file lists, the NEEDED list and the pin line are printed",
          r.returncode == 0 and "chosen version 1.32.0" in r.stdout and "jar: ai/onnxruntime/native/linux-x64/libonnxruntime.so" in r.stdout
          and "aar: headers/onnxruntime_c_api.h" in r.stdout and "NEEDED libc.so.6" in r.stdout
          and re.search(r'^pin this: VERSION=1\.32\.0 SHA256_JAR="[0-9a-f]{64}" SHA256_AAR="[0-9a-f]{64}"$', r.stdout, re.M), r.stdout + r.stderr)
    check("get-onnxruntime.sh: the pack has the headers, the library with its symlinks, the license and the README",
          os.path.isfile(os.path.join(pre, "include/onnxruntime_c_api.h")) and os.path.islink(os.path.join(pre, "lib/libonnxruntime.so"))
          and os.path.isfile(os.path.join(pre, "lib/libonnxruntime.so.1.32.0")) and os.path.islink(os.path.join(pre, "lib/libonnxruntime.so.1"))
          and "MIT" in open(os.path.join(pre, "licenses/LICENSE-onnxruntime.txt")).read()
          and "Maven" in open(os.path.join(pre, "README.txt")).read()
          and open(os.path.join(pre, "version.txt")).read().strip() == "onnxruntime 1.32.0 linux-x64")
finally:
    shutil.rmtree(work, ignore_errors=True)

print(f"{FAILS} failure(s)")
sys.exit(1 if FAILS else 0)
