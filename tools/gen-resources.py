#!/usr/bin/env python3
"""Regenerates resources/resources.qrc and resources-windows.qrc from the files in resources/ and docs/ (also docs/i18n/) and
translations/translations.qrc from the .qm files in translations/.

Run after adding or removing icons, license texts or files of the welcome project:  python3 tools/gen-resources.py
"""
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(ROOT, "resources")

entries = []  # (alias, path relative to resources/)
for theme in ("qbicons", "qbicons-dark"):
    base = os.path.join(RES, "icons", theme)
    for dirpath, _, files in sorted(os.walk(base)):
        for f in sorted(files):
            full = os.path.join(dirpath, f)
            rel = os.path.relpath(full, RES)
            entries.append(("icons/" + os.path.relpath(full, os.path.join(RES, "icons")), rel))
entries.append(("app/quillbinder.svg", "app/quillbinder.svg"))
entries.append(("docs/manual.md", "../docs/manual.md"))
entries.append(("docs/developer-guide.md", "../docs/developer-guide.md"))
entries.append(("docs/mindmap-help.md", "../docs/mindmap-help.md"))
entries.append(("docs/quick-start.md", "../docs/quick-start.md"))
# 0.95.0 (item 102): the manual and the quick start guide of each language (docs/i18n/<code>/), found by themselves
i18n = os.path.join(ROOT, "docs", "i18n")
for dirpath, _, files in sorted(os.walk(i18n)):
    for f in sorted(files):
        if f.endswith(".md"):
            rel = os.path.relpath(os.path.join(dirpath, f), os.path.join(ROOT, "docs"))
            entries.append(("docs/" + rel, "../docs/" + rel))
# the license texts of the third-party parts (tools/collect-licenses.py) and
# the welcome project (an outline with its Markdown files, src/core/OutlineProject.*)
for sub in ("licenses", "welcome"):
    base = os.path.join(RES, sub)
    for dirpath, _, files in sorted(os.walk(base)):
        for f in sorted(files):
            rel = os.path.relpath(os.path.join(dirpath, f), RES)
            entries.append((rel, rel))

def write_qrc(name, items):
    with open(os.path.join(RES, name), "w") as out:
        out.write("<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n  <qresource prefix=\"/\">\n")
        for alias, path in items:
            out.write(f"    <file alias=\"{alias}\">{path}</file>\n")
        out.write("  </qresource>\n</RCC>\n")


write_qrc("resources.qrc", entries)

# the compiled translations (translations/translations.qrc, prefix /translations): every quillbinder_<code>.qm and
# qt_<code>.qm that is there (0.95.0)
tdir = os.path.join(ROOT, "translations")
with open(os.path.join(tdir, "translations.qrc"), "w") as out:
    out.write("<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n<qresource prefix=\"/translations\">\n")
    for f in sorted(os.listdir(tdir)):
        if f.endswith(".qm") and (f.startswith("quillbinder_") or f.startswith("qt_")):
            out.write(f"    <file>{f}</file>\n")
    out.write("</qresource>\n</RCC>\n")
# Windows (port, W3): the same files, but the license list of what the Windows program ships
# (resources/licenses-windows, packaging/windows/collect-licenses-windows.py) under the same names :/licenses/...
win = [e for e in entries if not e[0].startswith("licenses/")]
base = os.path.join(RES, "licenses-windows")
for dirpath, _, files in sorted(os.walk(base)):
    for f in sorted(files):
        rel = os.path.relpath(os.path.join(dirpath, f), RES)
        win.append(("licenses/" + os.path.relpath(os.path.join(dirpath, f), base), rel))
write_qrc("resources-windows.qrc", win)
print(f"{len(entries)} files ({len(win)} for Windows)")
