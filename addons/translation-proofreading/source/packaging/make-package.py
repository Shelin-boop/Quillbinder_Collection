#!/usr/bin/env python3
"""Writes a Quillbinder add-on package (.qbpkg) from a staged folder.

    make-package.py <folder> <out.qbpkg>
      <folder>/manifest.json   id, name, version, description, author, whatsNew, minApp
      <folder>/payload/...     the add-on's files

The package format is the one of Quillbinder's developer guide (docs/developer-guide.md of the public source, sections
2 and 5): a ZIP file with manifest.json and payload/; the manifest lists every file of payload/ with its SHA-256 and
size. An add-on with Linux libraries names the runtime it was built for (the Qt inside Quillbinder's AppImage) and
needs at least the first program version with that runtime. Packages of this add-on are not signed (only Quillbinder's
own packages are); the program says so when it installs one.
"""
import hashlib
import json
import os
import re
import sys
import zipfile

# Quillbinder's runtime: Ubuntu 24.04 with The Qt Company's Qt (APP_RUNTIME and APP_RUNTIME_SINCE in Quillbinder's
# CMakeLists.txt). 0.93.1 (to-do item 52): read from that file when the add-on's source is inside Quillbinder's
# (addons/translation-proofreading/source), so that the add-on's own build cannot name an older runtime than the
# program's (it said "-r1" after Quillbinder 0.93.0 had moved to "-r2"); the values below only for a source alone.
RUNTIME = "u2404-qt6.12-r2"
RUNTIME_SINCE = "0.93.0"
# Quillbinder for Android: The Qt Company's Qt for Android in the APK; add-ons since 0.91.3. Qt 6.12.1 since Android
# 0.93.0 ("android-qt6.12-r2"; "-r1" was Qt 6.12.0, Android 0.91.3 to 0.92.x). It must be the runtime of the APK the
# package goes with: Quillbinder refuses an add-on with libraries of another runtime (Quillbinder's
# packaging/android/android-translator.sh checks it against APP_RUNTIME)
ANDROID_RUNTIME = "android-qt6.12-r2"
ANDROID_RUNTIME_SINCE = "0.93.0"


QUILLBINDER_CMAKE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "..", "CMakeLists.txt")


def quillbinder_runtimes(path=QUILLBINDER_CMAKE):
    """(linux runtime, its first version, android runtime) of Quillbinder's CMakeLists.txt, or None without one."""
    try:
        text = open(path, encoding="utf-8").read()
    except OSError:
        return None
    linux = re.search(r'^set\(APP_RUNTIME "([^"]+)"\)', text, re.M)
    since = re.search(r'^set\(APP_RUNTIME_SINCE "([^"]+)"\)', text, re.M)
    android = re.search(r'^\s+set\(APP_RUNTIME "(android-[^"]+)"\)', text, re.M)
    if not (linux and since and android):
        raise SystemExit("make-package.py: %s names no runtime (APP_RUNTIME, APP_RUNTIME_SINCE)" % path)
    return linux.group(1), since.group(1), android.group(1)


def version_key(v):
    return [int(x) if x.isdigit() else 0 for x in v.split(".")]


def main():
    folder, out = sys.argv[1], sys.argv[2]
    manifest = json.load(open(os.path.join(folder, "manifest.json"), encoding="utf-8"))
    manifest["format"] = 1
    manifest["kind"] = "addon"
    files, payload = [], []
    root = os.path.join(folder, "payload")
    for d, _, names in sorted(os.walk(root)):
        for n in sorted(names):
            p = os.path.join(d, n)
            rel = os.path.relpath(p, root).replace(os.sep, "/")
            data = open(p, "rb").read()
            e = {"path": rel, "sha256": hashlib.sha256(data).hexdigest(), "size": len(data)}
            if n.endswith(".so") or ".so." in n:
                e["executable"] = True
            files.append(e)
            payload.append((rel, data))
    manifest["files"] = files
    if any(f["path"].endswith(".so") or ".so." in f["path"] for f in files):
        linux_runtime, linux_since, android_runtime = RUNTIME, RUNTIME_SINCE, ANDROID_RUNTIME
        found = quillbinder_runtimes()
        if found:
            linux_runtime, linux_since, android_runtime = found
            if android_runtime != ANDROID_RUNTIME:
                raise SystemExit("make-package.py: Quillbinder's Android runtime is %s, this script's %s: change "
                                 "ANDROID_RUNTIME and ANDROID_RUNTIME_SINCE together" % (android_runtime, ANDROID_RUNTIME))
        runtime, since = linux_runtime, linux_since
        if manifest.get("platform", "").startswith("android"):
            # Android (2026-11-16): Quillbinder for Android's runtime (Qt 6.12.0 for Android, the NDK's C++ library)
            runtime, since = android_runtime, ANDROID_RUNTIME_SINCE
        manifest["runtime"] = runtime
        if version_key(manifest.get("minApp", "0")) < version_key(since):
            manifest["minApp"] = since
    if os.path.exists(out):
        os.remove(out)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        z.writestr("manifest.json", json.dumps(manifest, indent=2, ensure_ascii=False).encode("utf-8"))
        for rel, data in payload:
            z.writestr("payload/" + rel, data)
    print(f"{out}  ({os.path.getsize(out) / 1e6:.2f} MB, {len(files)} files)")


if __name__ == "__main__":
    main()
