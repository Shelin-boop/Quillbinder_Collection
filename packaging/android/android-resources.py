#!/usr/bin/env python3
"""Writes the Android program's resource file (2026-11-14).

  packaging/android/android-resources.py <resources/resources.qrc> <resources/licenses-android> <the new .qrc>

The same files as the Linux program's (icons, manual, welcome project), but the license list of what the APK ships
(resources/licenses-android, packaging/android/collect-licenses-android.py) under the same names :/licenses/... .
Written at build time (CMakeLists.txt, inside if(ANDROID)), so that a change of the Linux list of resources reaches
Android without a second file to keep; the paths in it are absolute (the file stands in the build folder).
"""
import os
import re
import sys

src, licenses, out = sys.argv[1], sys.argv[2], sys.argv[3]
base = os.path.dirname(os.path.abspath(src))
entries = []
for alias, path in re.findall(r'<file alias="([^"]+)">([^<]+)</file>', open(src, encoding="utf-8").read()):
    if alias.startswith("licenses/"):
        continue
    entries.append((alias, os.path.normpath(os.path.join(base, path))))
count = 0
for dirpath, _, files in sorted(os.walk(licenses)):
    for f in sorted(files):
        full = os.path.join(dirpath, f)
        entries.append(("licenses/" + os.path.relpath(full, licenses).replace(os.sep, "/"), os.path.abspath(full)))
        count += 1
if count == 0 or not os.path.exists(os.path.join(licenses, "index.json")):
    sys.exit("no license list in " + licenses)
missing = [p for _, p in entries if not os.path.exists(p)]
if missing:
    sys.exit("files named in the resource list do not exist: " + ", ".join(missing[:5]))
with open(out, "w", encoding="utf-8") as f:
    f.write('<!DOCTYPE RCC>\n<RCC version="1.0">\n  <qresource prefix="/">\n')
    for alias, path in entries:
        f.write(f'    <file alias="{alias}">{path}</file>\n')
    f.write("  </qresource>\n</RCC>\n")
print(f"{len(entries)} files, {count} of the Android license list")
