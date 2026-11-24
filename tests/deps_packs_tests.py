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


NAMES = ("ffmpeg", "whisper", "translator")
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
check("the three recipes differ", len(set(hashes.values())) == 3)
check("the translator's recipe holds its two scripts",
      "file tools/get-translator-deps.sh " in pack("recipe", "translator").stdout
      and "file addons/translation-proofreading/source/packaging/build-deps.sh " in pack("recipe", "translator").stdout)
check("the FFmpeg recipe holds the signature script", "file tools/verify-ffmpeg-source.sh " in pack("recipe", "ffmpeg").stdout)
for n, words in (("ffmpeg", ("FFmpeg 9.1", "LGPL-2.1-or-later", "https://ffmpeg.org/releases/ffmpeg-9.1.tar.xz")),
                 ("whisper", ("whisper.cpp", "MIT", "https://github.com/ggml-org/whisper.cpp")),
                 ("translator", ("oneDNN", "CTranslate2", "SentencePiece", "Apache-2.0", "MIT", "BSD-3-Clause",
                                 "https://github.com/OpenNMT/CTranslate2"))):
    a = pack("about", n)
    check(f"the text of the {n} pack says what is inside, its license and its source",
          a.returncode == 0 and a.stdout.count("\n") == 2 and a.stdout.startswith("Inside:") and "\nLicense" in a.stdout
          and "Source" in a.stdout and all(w in a.stdout for w in words), a.stdout + a.stderr)
check("the pack of a name that does not exist is refused", pack("about", "nothing").returncode != 0)
check("the repository with the packs defaults to this one",
      re.search(r"^REPO=\$\{QB_DEPS_REPO:-Shelin-boop/Quillbinder_Collection\}$", dp, re.M) is not None)

# ---------------------------------------------------------------------------------------------------- the workflow
check("the workflow builds the three packs", re.search(r"pack: \[ffmpeg, whisper, translator\]", wf) is not None)
for f in ("tools/get-ffmpeg.sh", "tools/build-ffmpeg.sh", "tools/verify-ffmpeg-source.sh", "tools/get-whisper.sh",
          "tools/get-translator-deps.sh", "addons/translation-proofreading/source/packaging/build-deps.sh",
          "tools/deps-pack.sh"):
    check(f"a change of {f} starts the workflow", f"      - '{f}'\n" in wf)
check("the workflow runs the three scripts and packs the three",
      all(s in wf for s in ("ffmpeg) script=tools/get-ffmpeg.sh", "whisper) script=tools/get-whisper.sh",
                            "translator) script=tools/get-translator-deps.sh",
                            "translator) tools/deps-pack.sh pack translator out build/translator-deps")))
check("the pre-release's text names what is inside, the license and the source",
      'tools/deps-pack.sh about "$PACK" > "out/about-$PACK.txt"' in wf and 'cat "$d/about-$p.txt"' in wf)
check("the FFmpeg step prints the SHA-256 of the signed source for the pin",
      "pin this" in wf and "SHA-256 of" in wf and "verify-ffmpeg-source:" in wf)
check("only the job publish may write", len(re.findall(r"contents: write", wf)) == 1
      and re.search(r"^permissions:\n  contents: read\n", wf, re.M) is not None)
for s in ("get-ffmpeg.sh", "get-whisper.sh", "get-translator-deps.sh"):
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
finally:
    shutil.rmtree(work, ignore_errors=True)

print(f"{FAILS} failure(s)")
sys.exit(1 if FAILS else 0)
