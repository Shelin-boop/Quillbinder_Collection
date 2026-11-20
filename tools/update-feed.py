#!/usr/bin/env python3
"""Writes updates-1.json, the file the program's check for updates reads (0.92.0; its form: src/core/UpdateCheck.h).

    tools/update-feed.py --dist <folder with the release's files> --tag <tag> --repo <owner/name> --out <file>
                         [--system linux-x86_64] [--runtime <id>] [--previous <the file published so far>]
                         [--date <UTC time, ISO>] [--label <the version's label>]

It names, for one system, the program's newest version (its program update and its AppImages, each with address,
size and SHA-256) and the newest version of every add-on package in the folder. The blocks of the other systems and
the add-ons of this system that the folder does not hold (add-ons released on their own) are taken over from
--previous. The versions that were security updates come from packaging/update-feed/security.json and from
--previous. The release workflow signs the result with the release key (job sign-checksums); this script signs
nothing and needs no key.

Windows 0.92.0: with --system windows-x64 (and --runtime, --label of the Windows files) the whole program anew is
named by its setup programs ("setup", "setupFull": the files "...-setup.exe" of the version) instead of AppImages.

Android 0.92.0: with --system android-arm64 the program is named by its APK only ("apk": the file "...-android-arm64.apk"
of the version; Android takes no program update), and the headlines of the section "## Android <version> ..." of
docs/CHANGES.md come before those of the version itself.
"""
import argparse
import datetime
import hashlib
import json
import os
import re
import sys
import urllib.parse
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# the files a program block can name: the program update, Linux's AppImages, Windows' setup programs
FILE_KINDS = ("update", "appimage", "appimageFull", "setup", "setupFull", "apk")


def cmake_var(name):
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r'^set\(%s\s+"([^"]*)"\)' % name, f.read(), re.M)
    return m.group(1) if m else ""


def version_key(v):
    return [int(x) if x.isdigit() else 0 for x in re.split(r"[.\-]", v)]


def file_entry(path, repo, tag):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    name = os.path.basename(path)
    return {
        "url": "https://github.com/%s/releases/download/%s/%s" % (repo, urllib.parse.quote(tag), urllib.parse.quote(name)),
        "name": name,
        "size": os.path.getsize(path),
        "sha256": h.hexdigest(),
    }


def whats_new(version, own=""):
    """The headlines of this version's entry in docs/CHANGES.md (the bold starts of its points); with `own` ("Android")
    those of the section "## <own> <version> ..." first."""
    try:
        with open(os.path.join(ROOT, "docs", "CHANGES.md"), encoding="utf-8") as f:
            text = f.read()
    except OSError:
        return []
    out = []
    for head in ([re.escape(own) + r"\s+" + re.escape(version)] if own else []) + [re.escape(version)]:
        m = re.search(r"^## %s\b.*?$(.*?)(?=^## |\Z)" % head, text, re.M | re.S)
        if m:
            lines = re.findall(r"^- \*\*(.+?)\*\*", m.group(1), re.M)
            out += [re.sub(r"\s+", " ", x).strip().rstrip(".") + "." for x in lines]
    return out[:8]


def manifest_of(path):
    try:
        with zipfile.ZipFile(path) as z:
            return json.loads(z.read("manifest.json").decode("utf-8"))
    except (OSError, KeyError, ValueError, zipfile.BadZipFile):
        return None


def merged(a, b):
    out = sorted(set(a or []) | set(b or []), key=version_key)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dist", required=True)
    ap.add_argument("--tag", required=True)
    ap.add_argument("--repo", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--system", default="linux-x86_64")
    ap.add_argument("--runtime", default="")
    ap.add_argument("--previous", default="")
    ap.add_argument("--date", default="")
    ap.add_argument("--label", default="")
    args = ap.parse_args()

    version = cmake_var("APP_VERSION")
    label = args.label or cmake_var("APP_VERSION_LABEL")
    runtime = args.runtime or cmake_var("APP_RUNTIME")
    previous = {}
    if args.previous and os.path.isfile(args.previous):
        with open(args.previous, encoding="utf-8") as f:
            previous = json.load(f)
        if previous.get("format") != 1:
            previous = {}
    with open(os.path.join(ROOT, "packaging", "update-feed", "security.json"), encoding="utf-8") as f:
        security = json.load(f)
    old_sys = (previous.get("systems") or {}).get(args.system) or {}

    names = sorted(os.listdir(args.dist))
    program = {"version": version, "label": label, "runtime": runtime,
               "security": merged((security.get("program") or {}).get(args.system),
                                  (old_sys.get("program") or {}).get("security")),
               "whatsNew": whats_new(version, "Android" if args.system.startswith("android") else "")}
    addons = {}
    for name in names:
        path = os.path.join(args.dist, name)
        if name.endswith(".AppImage") and version in name:
            program["appimageFull" if "-full-" in name else "appimage"] = file_entry(path, args.repo, args.tag)
        elif args.system.startswith("windows") and name.endswith("-setup.exe") and version in name:
            program["setupFull" if "-full-" in name else "setup"] = file_entry(path, args.repo, args.tag)
        elif args.system.startswith("android") and name.endswith(".apk") and version in name:
            program["apk"] = file_entry(path, args.repo, args.tag)
        elif name.endswith(".qbpkg"):
            m = manifest_of(path)
            if not m:
                continue
            kind = m.get("kind")
            if kind == "core" and m.get("version") == version and not args.system.startswith("android"):
                program["update"] = file_entry(path, args.repo, args.tag)
            elif kind == "addon":
                platform = m.get("platform") or ""
                if platform and not args.system.startswith(platform.split("-")[0]):
                    continue   # an add-on built for another system
                if not platform and args.system != "linux-x86_64" and m.get("runtime"):
                    continue   # Linux libraries
                entry = {"id": m.get("id"), "name": m.get("name") or m.get("id"), "version": str(m.get("version")),
                         "minApp": m.get("minApp") or "", "runtime": m.get("runtime") or "",
                         "security": merged((security.get("addons") or {}).get(m.get("id")), []),
                         "whatsNew": [str(x) for x in (m.get("whatsNew") or [])][:8] if isinstance(m.get("whatsNew"), list)
                         else ([str(m.get("whatsNew"))] if m.get("whatsNew") else []),
                         "file": file_entry(path, args.repo, args.tag)}
                if entry["id"]:
                    addons[entry["id"]] = entry
    # add-ons of this system that this release does not carry (released on their own): kept as they were
    for old in old_sys.get("addons") or []:
        if old.get("id") and old["id"] not in addons:
            addons[old["id"]] = old
        elif old.get("id"):
            addons[old["id"]]["security"] = merged(addons[old["id"]]["security"], old.get("security"))

    systems = dict(previous.get("systems") or {})
    block = {"addons": [addons[k] for k in sorted(addons)]}
    if any(k in program for k in FILE_KINDS):
        block["program"] = program
    elif old_sys.get("program"):
        block["program"] = old_sys["program"]
    systems[args.system] = block
    date = args.date or datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    feed = {"format": 1, "date": date, "systems": systems}
    with open(args.out, "w", encoding="utf-8", newline="\n") as f:
        json.dump(feed, f, ensure_ascii=False, indent=1, sort_keys=True)
        f.write("\n")
    prog = block.get("program") or {}
    print("updates-1.json: %s, program %s (%s), %d add-on(s)" % (args.system, prog.get("version", "-"),
          ", ".join(k for k in FILE_KINDS if k in prog) or "no files", len(block["addons"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
