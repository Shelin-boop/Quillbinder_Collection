#!/usr/bin/env python3
"""Writes the download page of a release (2026-11-12): one button per system for the full version (the program with
all add-ons), the steps to start it, and links to all files of the release. Since 0.90.2 (the writer's wish) each
system also lists its other downloads: the program alone (the small AppImage), the program update and the audio and
video add-ons. The release workflow publishes it on the
repository's GitHub Pages (job download-page of .github/workflows/release.yml).

    tools/download-page.py <folder with the release's files (their names are enough)> <tag> <out dir>
        [--also <folder> <tag>]...

A file "full-addons-size" in the folder (the add-ons' installed size in bytes) adds the space to the Linux card.
A file of the folder may hold its own size in bytes (the release workflow writes it so); the page names the size of
the add-on packages it lists under each system's other downloads (name, one line, size; 0.94.0).

Since 0.91.2 each system can have a last number of its own (working rules, section 8), so a new Linux release may
come before the Windows files of its number: a system without a file in the release is taken from the "--also"
release that has one (give them newest first), and its card names that version. Since 0.93.0 (a version line per
system in CMakeLists.txt) each system shows its newest release: where several releases have a file of the system,
the one with the highest version wins, whichever order they are given in. A system that none of them
has shows "comes later". The visitor's own system comes first (the page's
script reads the browser's user agent; without script the order is Windows, Linux, Android).
"""
import html
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def cmake_var(name):
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r'^set\(%s\s+"([^"]*)"\)' % name, f.read(), re.M)
    return m.group(1) if m else ""


def pick(files):
    """The full version of each system ("" when the release has none)."""
    win = lambda f: "windows" in f.lower()
    linux = next((f for f in files if f.endswith(".AppImage") and "-full-" in f), "")
    # Windows (0.90.4): the setup program first (it asks for a folder and unpacks into it), else the zip
    windows = next((f for f in files if f.endswith("-setup.exe") and win(f) and "-full-" in f), "") \
        or next((f for f in files if f.endswith(".zip") and win(f) and "-full-" in f), "")
    android = next((f for f in files if f.endswith(".apk")), "")
    return {"linux": linux, "windows": windows, "android": android}


def others(files):
    """The other downloads of each system (0.90.2): [(file, what it is, how to use it)], only those the release has."""
    win = lambda f: "windows" in f.lower()
    first = lambda test: next((f for f in files if test(f)), "")
    linux = [
        (first(lambda f: f.endswith(".AppImage") and "-full-" not in f),
         "The program alone (the small AppImage)", "without the add-ons; they can be installed later, one by one"),
        (first(lambda f: f.endswith("-update.qbpkg") and not win(f)),
         "Program update", "for a Quillbinder you already have (File &gt; Install Update or Add-on)"),
        (first(lambda f: f == "media-all-formats.qbpkg"),
         "Audio and video add-ons", "sound and video in all common formats (File &gt; Install Update or Add-on)"),
    ]
    has_setup = bool(first(lambda f: f.endswith("-setup.exe") and win(f) and "-full-" in f))
    windows = [
        (first(lambda f: has_setup and f.endswith(".zip") and win(f) and "-full-" in f),
         "The full version as a zip file", "the same files as the setup program unpacks; unpack it into a folder of your own"),
        (first(lambda f: f.endswith("-setup.exe") and win(f) and "-full-" not in f),
         "The program alone (setup program)", "without the add-ons; they can be installed later, one by one"),
        (first(lambda f: f.endswith(".zip") and win(f) and "-full-" not in f and "symbols" not in f),
         "The program alone as a zip file" if has_setup else "The program alone",
         "without the add-ons; they can be installed later, one by one"),
        (first(lambda f: f.endswith("-update.qbpkg") and win(f)),
         "Program update", "for a Quillbinder you already have (File &gt; Install Update or Add-on)"),
        (first(lambda f: f.startswith("media-all-formats") and win(f) and f.endswith(".qbpkg")),
         "Audio and video add-ons", "sound and video in all common formats (File &gt; Install Update or Add-on)"),
    ]
    # Android 0.92.0: the app carries Translation and Proofreading; its package on its own, for an app that lacks it
    android = [
        (first(lambda f: f.startswith("translation-proofreading-android") and f.endswith(".qbpkg")),
         "Translation and Proofreading on its own", "the app has it already; for File &gt; Install Update or Add-on"),
    ]
    keep = lambda rows: [r for r in rows if r[0]]
    return {"linux": keep(linux), "windows": keep(windows), "android": keep(android)}


# the size of each file by its name, in bytes (filled by main() from the folder: an empty file has none)
SIZES = {}


def known_addons(root=ROOT):
    """{id: (name, one line)} of the project's own add-ons (addons/<id>/manifest.json) and the official ones (0.94.0)."""
    out = {}
    folder = os.path.join(root, "addons")
    for name in sorted(os.listdir(folder)) if os.path.isdir(folder) else []:
        try:
            with open(os.path.join(folder, name, "manifest.json"), encoding="utf-8") as f:
                m = json.load(f)
        except (OSError, ValueError):
            continue
        if isinstance(m, dict) and m.get("name") and m.get("description"):
            out[m.get("id", name)] = (m["name"], first_sentence(m["description"]))
    for a in official_addons(root):
        out.setdefault(a["id"], (a["name"], first_sentence(a["summary"])))
    return out


def first_sentence(text):
    """The first sentence of a description (up to 160 characters): the one line of an add-on on the page."""
    text = " ".join(text.split())
    m = re.match(r"(.+?[.!?])(\s|$)", text)
    line = m.group(1) if m else text
    if len(line) > 160:
        colon = line.find(":")
        line = line[:colon] + "." if 20 < colon < 160 else line[:157].rsplit(" ", 1)[0] + "..."
    return line


def addon_rows(files, root=ROOT):
    """The add-on packages of a release, by system (0.94.0): {"linux": [(file, name, line, bytes)], "windows": ..., "android": ...}.
    Only packages of add-ons that are known (addons/ and official-addons/); the program update and the audio and video
    package have their own lines."""
    known = known_addons(root)
    ids = sorted(known, key=len, reverse=True)
    out = {"linux": [], "windows": [], "android": []}
    for f in files:
        low = f.lower()
        if not f.endswith(".qbpkg") or f.endswith("-update.qbpkg") or f.startswith("media-all-formats"):
            continue
        stem = f[:-len(".qbpkg")]
        found = next((i for i in ids if stem == i or stem.startswith(i + "-")), "")
        if not found:
            continue
        key = "windows" if "windows" in low else "android" if "android" in low else "linux"
        name, line = known[found]
        out[key].append((f, name, line, SIZES.get(f, 0)))
    for key in out:
        out[key].sort(key=lambda r: r[1].lower())
    return out


def size_text(n):
    if n <= 0:
        return ""
    return "%d MB" % max(1, int(round(n / 1e6)))


def official_addons(root=ROOT):
    """The official add-ons (0.93.0), read from official-addons/*.json: [{name, summary, systems, builtin, id, version,
    nonCommercial}] sorted by name. "systems" is Linux, plus Windows and Android where the entry has a block for them;
    "builtin" is true for an add-on of the project's own (a folder in addons/): the full version carries it."""
    found = []
    folder = os.path.join(root, "official-addons")
    for name in sorted(os.listdir(folder)) if os.path.isdir(folder) else []:
        if not name.endswith(".json"):
            continue
        try:
            with open(os.path.join(folder, name), encoding="utf-8") as f:
                e = json.load(f)
        except (OSError, ValueError):
            continue
        if not isinstance(e, dict) or not all(isinstance(e.get(k), str) and e[k] for k in ("id", "name", "version", "summary")):
            continue
        systems = ["Linux"] + [label for key, label in (("windows", "Windows"), ("android", "Android")) if isinstance(e.get(key), dict)]
        nc = [d for d in e.get("nonCommercialData", []) if isinstance(d, dict) and d.get("name") and d.get("license")]
        found.append({"id": e["id"], "name": e["name"], "version": e["version"], "summary": e["summary"].strip(),
                      "systems": systems, "nonCommercial": nc,
                      "builtin": os.path.isdir(os.path.join(root, "addons", e["id"]))})
    return sorted(found, key=lambda a: a["name"].lower())


def addons_section(addons, home):
    """The list of official add-ons for the page (empty when there are none): name and version, one line, the systems
    it is for, where to get it, and the note for data that may be used non-commercially only."""
    if not addons:
        return ""
    items = []
    for a in addons:
        entry = "%s/blob/main/official-addons/%s.json" % (home, a["id"])
        where = ("Included in the full version of each system it is for; also a package of its own on the program's releases."
                 if a["builtin"] else
                 'A package of its own on its release <code>addon-%s-%s</code> (one per system).' % (html.escape(a["id"]), html.escape(a["version"])))
        note = ""
        if a["nonCommercial"]:
            note = ('\n        <p class="small">Free; it must not be sold. It can download %s, whose license allows non-commercial use only.</p>'
                    % html.escape("; ".join("%s (%s)" % (d["name"], d["license"]) for d in a["nonCommercial"])))
        items.append('      <li data-addon="%s"><b>%s</b> <span class="file">version %s &middot; for %s</span>\n'
                     '        <p>%s</p>\n        <p class="small">%s <a href="%s">Its entry</a>, <a href="%s/blob/main/docs/official-addons.md">'
                     'how an add-on becomes official</a>.</p>%s</li>'
                     % (html.escape(a["id"]), html.escape(a["name"]), html.escape(a["version"]),
                        html.escape(", ".join(a["systems"])), html.escape(a["summary"]), where, html.escape(entry), html.escape(home), note))
    return ('  <section class="system official" id="official-addons">\n    <h2>Official add-ons</h2>\n'
            '    <p class="later">Reviewed, built and signed by the project; all are free.</p>\n    <ul class="others">\n%s\n    </ul>\n  </section>\n'
            % "\n".join(items))


def tag_version(tag):
    """[0, 93, 0] for "v0.93.0-beta" and "v0.93.1-windows-experimental" (what the tag's number compares as)."""
    m = re.match(r"v(\d+(?:\.\d+)*)", tag)
    return [int(x) for x in m.group(1).split(".")] if m else []


def shown_of(tag):
    return tag[1:].replace("-", " ") if tag.startswith("v") else tag


def space_note(addon_bytes):
    """The space the full version's add-ons take beside its button (0.94.0); nothing when the release gives no number."""
    if addon_bytes <= 0:
        return ""
    mb = max(1, int(round(addon_bytes / 1e6)))
    return ('\n      <p class="small">The add-ons inside take about %d MB of disk space once they are unpacked (at the first start); '
            'a window shows the progress.</p>' % mb)


def page(files, tag, also=(), addon_bytes=0):
    """also: [(files, tag)] of older releases, newest first (0.91.2): a system the release has no file for is taken
    from the first of them that has one."""
    home = cmake_var("APP_HOMEPAGE") or "https://github.com/Shelin-boop/Quillbinder_Collection"
    shown = shown_of(tag)
    full = pick(files)
    more = others(files)
    addon_of = addon_rows(files)
    tag_of = {key: tag for key in full}
    # 0.93.0 (a version line per system): each system shows its newest release, the one with the highest version
    # among the release and the older ones (a tie goes to the earlier one in the list: this release, then the
    # newest-first "--also" ones); before, the first release that had a file of the system won
    for older_files, older_tag in also:
        older_full, older_more, older_addons = pick(older_files), others(older_files), addon_rows(older_files)
        for key in full:
            if older_full[key] and (not full[key] or tag_version(older_tag) > tag_version(tag_of[key])):
                full[key], more[key], tag_of[key] = older_full[key], older_more[key], older_tag
                addon_of[key] = older_addons[key]
    url = lambda key, f: "%s/releases/download/%s/%s" % (home, tag_of[key], f)
    if full["windows"].endswith("-setup.exe"):
        # 0.90.4: the setup program only unpacks (no admin rights, nothing in the registry, no entry under Apps)
        win_steps = ["Start the downloaded file. Windows may warn because it is not signed: "
                     "&ldquo;More info&rdquo;, then &ldquo;Run anyway&rdquo;.",
                     "Choose a folder of your own, for example "
                     "<code>C:\\Users\\&lt;your name&gt;\\Quillbinder</code> (not Program Files). The setup program only "
                     "unpacks Quillbinder into it: it needs no admin rights and changes nothing else on the computer.",
                     "Start <b>quillbinder.exe</b> in that folder. A guide at the first start explains where the program "
                     "keeps its files and offers a Start menu entry."]
    else:
        win_steps = ["Unpack the zip file completely into a folder of your own, for example "
                     "<code>C:\\Users\\&lt;your name&gt;\\Quillbinder</code> (not into Program Files).",
                     "Start <b>quillbinder.exe</b> in it. Windows may warn because the program is not signed: "
                     "&ldquo;More info&rdquo;, then &ldquo;Run anyway&rdquo;.",
                     "A guide at the first start explains where the program keeps its files."]
    systems = [
        ("windows", "Windows 11", "64-bit", win_steps),
        ("linux", "Linux", "x86_64, an AppImage",
         ["Make the file executable: right-click &gt; Properties &gt; Permissions &gt; <i>Is executable</i>.",
          "Double-click it. Your desktop may ask whether to execute the file (KDE Plasma: <i>Execute</i> or <i>Cancel</i>): "
          "choose <i>Execute</i>. The add-ons are installed at the first start.",
          "Help &gt; Add to Application Menu puts it into the menu of the desktop."]),
        # Android (0.91.3, 2026-11-16, the first public APK)
        ("android", "Android", "tablets, Android 9 or later",
         ["Download the APK file on the tablet and open it. Android asks whether the browser or the Files app may "
          "install apps: allow it for this install, then &ldquo;Install&rdquo;.",
          "At the first start Quillbinder asks for access to the tablet&rsquo;s files: it keeps projects, backups and "
          "exports in <b>Documents/Quillbinder</b>. Switch on &ldquo;All files access&rdquo; for it and go back.",
          "A new version comes as a new APK, which installs over the old one and keeps your settings; "
          "Help &gt; Check for Updates Now tells you when there is one."]),
    ]
    cards = []
    for key, name, detail, steps in systems:
        f = full[key]
        if tag_of[key] != tag:
            # this system's newest files are in an older release (0.91.2)
            detail += ", version %s" % shown_of(tag_of[key])
        if f:
            body = ('<a class="button" href="%s">Download for %s</a>\n'
                    '      <p class="file">%s</p>%s\n      <ol>%s</ol>'
                    % (html.escape(url(key, f)), html.escape(name), html.escape(f),
                       space_note(addon_bytes) if key == "linux" and tag_of[key] == tag else "",
                       "".join("<li>%s</li>" % s for s in steps)))
        else:
            body = '<p class="later">Comes later.</p>'
        if more[key]:
            rows = "".join('\n        <li><a href="%s">%s</a>: %s <span class="file">%s</span></li>'
                           % (html.escape(url(key, of)), what, how, html.escape(of)) for of, what, how in more[key])
            body += ('\n      <h3>Other downloads</h3>\n      <ul class="others">%s\n      </ul>' % rows)
        if addon_of[key]:
            # the add-ons of this system on their own (name, one line, size): for the program alone, or one more later
            arows = "".join('\n        <li data-addon-file="%s"><a href="%s">%s</a>: %s <span class="file">%s%s</span></li>'
                            % (html.escape(af), html.escape(url(key, af)), html.escape(an), html.escape(al), html.escape(af),
                               (" &middot; " + size_text(ab)) if size_text(ab) else "") for af, an, al, ab in addon_of[key])
            body += ('\n      <h3>Add-ons on their own</h3>\n      <p class="small">The full version has them all. With the '
                     'program alone, install one with File &gt; Install Update or Add-on.</p>\n'
                     '      <ul class="others">%s\n      </ul>' % arows)
        cards.append('    <section class="system" data-system="%s">\n      <h2>%s <span>%s</span></h2>\n      %s\n    </section>'
                     % (key, html.escape(name), html.escape(detail), body))
    addons = addons_section(official_addons(), home)
    return """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Quillbinder Download</title>
<style>
  :root { --bg: #fbfaf7; --fg: #1d1d1b; --muted: #5d5b55; --card: #ffffff; --line: #dedbd2; --accent: #2f5d8a; --accent-fg: #ffffff; }
  @media (prefers-color-scheme: dark) {
    :root { --bg: #1b1c1e; --fg: #ecebe7; --muted: #a9a6a0; --card: #242528; --line: #3a3b3f; --accent: #7fb0e0; --accent-fg: #10161c; }
  }
  * { box-sizing: border-box; }
  body { margin: 0; background: var(--bg); color: var(--fg); font: 17px/1.55 system-ui, -apple-system, "Segoe UI", sans-serif; }
  main { max-width: 760px; margin: 0 auto; padding: 32px 16px 48px; }
  h1 { font-size: 30px; margin: 0 0 4px; }
  .lead { color: var(--muted); margin: 0 0 28px; }
  .system { background: var(--card); border: 1px solid var(--line); border-radius: 12px; padding: 20px 22px; margin: 0 0 16px; }
  .system h2 { font-size: 21px; margin: 0 0 12px; }
  .system h2 span { font-size: 15px; font-weight: normal; color: var(--muted); margin-left: 6px; }
  .button { display: inline-block; background: var(--accent); color: var(--accent-fg); text-decoration: none; font-weight: 600;
            padding: 11px 20px; border-radius: 8px; }
  .button:focus-visible { outline: 3px solid var(--fg); outline-offset: 2px; }
  .file { color: var(--muted); font-size: 14px; margin: 8px 0 0; word-break: break-all; }
  ol { margin: 12px 0 0; padding-left: 22px; }
  h3 { font-size: 16px; margin: 18px 0 6px; }
  .others { margin: 0; padding-left: 22px; }
  .others li { margin: 4px 0; }
  .others a { color: var(--accent); font-weight: 600; }
  .others .file { display: block; margin: 0; }
  .later { color: var(--muted); margin: 0; }
  .official p { margin: 2px 0 6px; }
  .small { color: var(--muted); font-size: 14px; }
  .more { margin-top: 28px; }
  .more a { color: var(--accent); }
  .note { color: var(--muted); font-size: 14px; margin-top: 28px; }
  code { font-size: 15px; }
</style>
</head>
<body>
<main>
  <h1>Quillbinder %(shown)s</h1>
  <p class="lead">A writing studio for long texts. The full version: the program with all add-ons, one download.</p>
  <div id="systems">
%(cards)s
  </div>
%(addons)s  <p class="more"><a href="%(release)s">All files of this version</a> (single add-ons, the symbols for crash reports,
  checksums) &middot; <a href="%(home)s/releases">Earlier versions</a> &middot;
  <a href="%(home)s">Source code</a> &middot;
  <a href="%(home)s/issues/new/choose">Report a problem or propose a function</a></p>
  <p class="note">Quillbinder is an independent program. It is not made by or connected with Literature &amp; Latte,
  the maker of Scrivener; it reads and writes Scrivener 3 projects. This is %(article)s %(label)s version: keep backups of
  your projects and a copy of your manuscripts outside the program.</p>
</main>
<script>
  // the visitor's own system first
  (function () {
    var ua = navigator.userAgent || "";
    var own = /Android/i.test(ua) ? "android" : /Windows/i.test(ua) ? "windows" : /Linux|X11/i.test(ua) ? "linux" : "";
    var box = document.getElementById("systems");
    var card = own && box.querySelector('[data-system="' + own + '"]');
    if (card) box.insertBefore(card, box.firstChild);
  })();
</script>
</body>
</html>
""" % {"shown": html.escape(shown), "cards": "\n".join(cards), "addons": addons, "release": html.escape("%s/releases/tag/%s" % (home, tag)),
       "home": html.escape(home), "label": html.escape(cmake_var("APP_VERSION_LABEL") or "test"),
       "article": "an" if (cmake_var("APP_VERSION_LABEL")[:1].lower() in "aeiou" and cmake_var("APP_VERSION_LABEL")) else "a"}


def main(argv):
    rest = argv[4:]
    if len(argv) < 4 or len(rest) % 3 or any(rest[i] != "--also" for i in range(0, len(rest), 3)):
        print(__doc__, file=sys.stderr)
        return 2
    files = sorted(os.listdir(argv[1]))
    # the size of a file: the number it holds (the release workflow writes it so), else its length
    def read_sizes(folder, names):
        for n in names:
            try:
                with open(os.path.join(folder, n), encoding="utf-8") as f:
                    text = f.read(32).strip()
                SIZES[n] = int(text) if text.isdigit() else os.path.getsize(os.path.join(folder, n))
            except (OSError, ValueError):
                pass
    read_sizes(argv[1], [n for n in files if n.endswith(".qbpkg")])
    # 0.94.0: a file "full-addons-size" in the folder (the installed size of the full version's add-ons in bytes, written
    # by the job that builds them) lets the Linux card name the disk space; without it nothing is said
    addon_bytes = 0
    size_file = os.path.join(argv[1], "full-addons-size")
    if "full-addons-size" in files:
        files.remove("full-addons-size")
        try:
            with open(size_file, encoding="utf-8") as f:
                addon_bytes = int(f.read().strip() or 0)
        except (OSError, ValueError):
            addon_bytes = 0
    also = [(sorted(os.listdir(rest[i + 1])), rest[i + 2]) for i in range(0, len(rest), 3)]
    for i, (names, _tag) in enumerate(also):
        for n in names:
            if n.endswith(".qbpkg") and n not in SIZES:
                read_sizes(rest[3 * i + 1], [n])
    os.makedirs(argv[3], exist_ok=True)
    with open(os.path.join(argv[3], "index.html"), "w", encoding="utf-8") as f:
        f.write(page(files, argv[2], also, addon_bytes))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
