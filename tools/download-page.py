#!/usr/bin/env python3
"""Writes the download page of a release (2026-11-12): one button per system for the full version (the program with
all add-ons), the steps to start it, and links to the files. The release workflows publish it on the repository's
GitHub Pages (job download-page of .github/workflows/release.yml, release-windows.yml and release-android.yml).

Since Android 0.94.0 (to-do item 91) the page is two pages:
  - index.html: each system's card has the button for the full version, then the program alone and the program
    update (Android has neither: a new version is a new APK), and one link to the add-ons page;
  - addons/index.html (the folder addons/ of the site): every add-on of each system on its own (name, one line,
    size), the other files (the audio and video package, the Windows zip files) and the official add-ons.
(From 0.90.2 to 0.94.0 the cards listed all of it.)

    tools/download-page.py <folder with the release's files (their names are enough)> <tag> <out dir>
        [--also <folder> <tag>]...

A file "full-addons-size" in the folder (the add-ons' installed size in bytes) adds the space to the Linux card.
A file of the folder may hold its own size in bytes (the release workflow writes it so); the add-ons page names the
size of each add-on package (0.94.0).

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

INSTALL = "Help &gt; Add-ons and Updates &gt; Install Update or Add-on"


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
    """The two other downloads on each system's card (item 91): the program alone, then the program update;
    [(file, what it is, how to use it)], only those the release has. Android has neither."""
    win = lambda f: "windows" in f.lower()
    first = lambda test: next((f for f in files if test(f)), "")
    alone = "without the add-ons; they can be installed later, one by one"
    update = "for a Quillbinder you already have (%s)" % INSTALL
    linux = [
        (first(lambda f: f.endswith(".AppImage") and "-full-" not in f), "The program alone (the small AppImage)", alone),
        (first(lambda f: f.endswith("-update.qbpkg") and not win(f)), "Program update", update),
    ]
    has_setup = bool(first(lambda f: f.endswith("-setup.exe") and win(f) and "-full-" in f))
    # Windows: the setup program of the program alone when the release has setup programs, else its zip
    small = first(lambda f: f.endswith("-setup.exe") and win(f) and "-full-" not in f) if has_setup else \
        first(lambda f: f.endswith(".zip") and win(f) and "-full-" not in f and "symbols" not in f)
    windows = [
        (small, "The program alone (setup program)" if has_setup else "The program alone", alone),
        (first(lambda f: f.endswith("-update.qbpkg") and win(f)), "Program update", update),
    ]
    keep = lambda rows: [r for r in rows if r[0]]
    return {"linux": keep(linux), "windows": keep(windows), "android": []}


def extras(files):
    """The other files of each system for the add-ons page (item 91): [(file, what it is, how to use it)]."""
    win = lambda f: "windows" in f.lower()
    first = lambda test: next((f for f in files if test(f)), "")
    media = "sound and video in all common formats (%s)" % INSTALL
    linux = [(first(lambda f: f == "media-all-formats.qbpkg"), "Audio and video add-ons", media)]
    has_setup = bool(first(lambda f: f.endswith("-setup.exe") and win(f) and "-full-" in f))
    unpack = "the same files as the setup program unpacks; unpack it into a folder of your own"
    windows = [
        (first(lambda f: has_setup and f.endswith(".zip") and win(f) and "-full-" in f), "The full version as a zip file", unpack),
        (first(lambda f: has_setup and f.endswith(".zip") and win(f) and "-full-" not in f and "symbols" not in f),
         "The program alone as a zip file", unpack),
        (first(lambda f: f.startswith("media-all-formats") and win(f) and f.endswith(".qbpkg")), "Audio and video add-ons", media),
    ]
    keep = lambda rows: [r for r in rows if r[0]]
    return {"linux": keep(linux), "windows": keep(windows), "android": []}


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
    """The list of official add-ons for the add-ons page (empty when there are none): name and version, one line, the
    systems it is for, where to get it, and the note for data that may be used non-commercially only."""
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


# the systems in the page's order without script: (key, name, what it runs on)
SYSTEMS = [("windows", "Windows 11", "64-bit"), ("linux", "Linux", "x86_64, an AppImage"),
           ("android", "Android", "tablets, Android 9 or later")]


def gather(files, tag, also=()):
    """Each system's files from its newest release: (full, more, extra, addon_of, tag_of). also: [(files, tag)] of
    older releases, newest first (0.91.2): a system the release has no file for is taken from them."""
    full, more, extra, addon_of = pick(files), others(files), extras(files), addon_rows(files)
    tag_of = {key: tag for key in full}
    # 0.93.0 (a version line per system): each system shows its newest release, the one with the highest version
    # among the release and the older ones (a tie goes to the earlier one in the list: this release, then the
    # newest-first "--also" ones); before, the first release that had a file of the system won
    for older_files, older_tag in also:
        older_full, older_more, older_extra, older_addons = pick(older_files), others(older_files), extras(older_files), addon_rows(older_files)
        for key in full:
            if older_full[key] and (not full[key] or tag_version(older_tag) > tag_version(tag_of[key])):
                full[key], more[key], extra[key], tag_of[key] = older_full[key], older_more[key], older_extra[key], older_tag
                addon_of[key] = older_addons[key]
    return full, more, extra, addon_of, tag_of


STYLE = """<style>
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
  .folder { margin: 14px 0 0; }
  .folder a { color: var(--accent); font-weight: 600; }
  .later { color: var(--muted); margin: 0; }
  .official p { margin: 2px 0 6px; }
  .small { color: var(--muted); font-size: 14px; }
  .more { margin-top: 28px; }
  .more a { color: var(--accent); }
  .note { color: var(--muted); font-size: 14px; margin-top: 28px; }
  code { font-size: 15px; }
</style>"""

SCRIPT = """<script>
  // the visitor's own system first
  (function () {
    var ua = navigator.userAgent || "";
    var own = /Android/i.test(ua) ? "android" : /Windows/i.test(ua) ? "windows" : /Linux|X11/i.test(ua) ? "linux" : "";
    var box = document.getElementById("systems");
    var card = own && box.querySelector('[data-system="' + own + '"]');
    if (card) box.insertBefore(card, box.firstChild);
  })();
</script>"""


def document(title, body):
    return """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>%s</title>
%s
</head>
<body>
<main>
%s</main>
%s
</body>
</html>
""" % (html.escape(title), STYLE, body, SCRIPT)


def note():
    label = cmake_var("APP_VERSION_LABEL")
    article = "an" if (label[:1].lower() in "aeiou" and label) else "a"
    return ('  <p class="note">Quillbinder is an independent program. It is not made by or connected with Literature &amp; Latte,\n'
            '  the maker of Scrivener; it reads and writes Scrivener 3 projects. This is %s %s version: keep backups of\n'
            '  your projects and a copy of your manuscripts outside the program.</p>\n' % (article, html.escape(label or "test")))


def rows_html(key, rows, url):
    return "".join('\n        <li><a href="%s">%s</a>: %s <span class="file">%s</span></li>'
                   % (html.escape(url(key, of)), what, how, html.escape(of)) for of, what, how in rows)


def page(files, tag, also=(), addon_bytes=0):
    """The main page (index.html). also: [(files, tag)] of older releases, newest first (0.91.2)."""
    home = cmake_var("APP_HOMEPAGE") or "https://github.com/Shelin-boop/Quillbinder_Collection"
    shown = shown_of(tag)
    full, more, extra, addon_of, tag_of = gather(files, tag, also)
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
    steps_of = {
        "windows": win_steps,
        "linux": ["Make the file executable: right-click &gt; Properties &gt; Permissions &gt; <i>Is executable</i>.",
                  "Double-click it. Your desktop may ask whether to execute the file (KDE Plasma: <i>Execute</i> or <i>Cancel</i>): "
                  "choose <i>Execute</i>. The add-ons are installed at the first start.",
                  "Help &gt; Add to Application Menu puts it into the menu of the desktop."],
        # Android (0.91.3, 2026-11-16, the first public APK)
        "android": ["Download the APK file on the tablet and open it. Android asks whether the browser or the Files app may "
                    "install apps: allow it for this install, then &ldquo;Install&rdquo;.",
                    "At the first start Quillbinder asks for access to the tablet&rsquo;s files: it keeps projects, backups and "
                    "exports in <b>Documents/Quillbinder</b>. Switch on &ldquo;All files access&rdquo; for it and go back.",
                    "A new version comes as a new APK, which installs over the old one and keeps your settings; "
                    "Help &gt; Add-ons and Updates &gt; Check for Updates Now tells you when there is one."],
    }
    cards = []
    for key, name, detail in SYSTEMS:
        f = full[key]
        if tag_of[key] != tag:
            # this system's newest files are in an older release (0.91.2)
            detail += ", version %s" % shown_of(tag_of[key])
        if f:
            body = ('<a class="button" href="%s">Download for %s</a>\n'
                    '      <p class="file">%s</p>%s\n      <ol>%s</ol>'
                    % (html.escape(url(key, f)), html.escape(name), html.escape(f),
                       space_note(addon_bytes) if key == "linux" and tag_of[key] == tag else "",
                       "".join("<li>%s</li>" % s for s in steps_of[key])))
            if more[key]:
                body += '\n      <h3>Other downloads</h3>\n      <ul class="others">%s\n      </ul>' % rows_html(key, more[key], url)
            # item 91: the add-ons and the other files of the system are on the add-ons page (the site's folder addons/)
            body += ('\n      <p class="folder"><a href="addons/#%s">Add-ons and other files for %s</a></p>'
                     % (key, html.escape(name)))
        else:
            body = '<p class="later">Comes later.</p>'
        cards.append('    <section class="system" data-system="%s">\n      <h2>%s <span>%s</span></h2>\n      %s\n    </section>'
                     % (key, html.escape(name), html.escape(detail), body))
    body = ("""  <h1>Quillbinder %(shown)s</h1>
  <p class="lead">A writing studio for long texts. The full version: the program with all add-ons, one download.</p>
  <div id="systems">
%(cards)s
  </div>
  <p class="more"><a href="addons/">Add-ons and other files</a> (each add-on on its own, the official add-ons) &middot;
  <a href="%(release)s">All files of this version</a> (the symbols for crash reports, checksums) &middot;
  <a href="%(home)s/releases">Earlier versions</a> &middot; <a href="%(home)s">Source code</a> &middot;
  <a href="%(home)s/issues/new/choose">Report a problem or propose a function</a></p>
""" % {"shown": html.escape(shown), "cards": "\n".join(cards), "release": html.escape("%s/releases/tag/%s" % (home, tag)),
       "home": html.escape(home)}) + note()
    return document("Quillbinder Download", body)


def addons_page(files, tag, also=()):
    """The add-ons page (addons/index.html, item 91): per system each add-on on its own and the other files, then the
    official add-ons."""
    home = cmake_var("APP_HOMEPAGE") or "https://github.com/Shelin-boop/Quillbinder_Collection"
    shown = shown_of(tag)
    full, more, extra, addon_of, tag_of = gather(files, tag, also)
    url = lambda key, f: "%s/releases/download/%s/%s" % (home, tag_of[key], f)
    how = {"linux": "The full version has them all. With the program alone, install one with %s." % INSTALL,
           "windows": "The full version has them all. With the program alone, install one with %s." % INSTALL,
           "android": "The app has them already; a package on its own is for an app that lacks it (%s)." % INSTALL}
    sections = []
    for key, name, detail in SYSTEMS:
        if tag_of[key] != tag:
            detail += ", version %s" % shown_of(tag_of[key])
        parts = []
        if addon_of[key]:
            # the add-ons of this system on their own (name, one line, size)
            arows = "".join('\n        <li data-addon-file="%s"><a href="%s">%s</a>: %s <span class="file">%s%s</span></li>'
                            % (html.escape(af), html.escape(url(key, af)), html.escape(an), html.escape(al), html.escape(af),
                               (" &middot; " + size_text(ab)) if size_text(ab) else "") for af, an, al, ab in addon_of[key])
            parts.append('<h3>Add-ons on their own</h3>\n      <p class="small">%s</p>\n      <ul class="others">%s\n      </ul>'
                         % (how[key], arows))
        if extra[key]:
            parts.append('<h3>Other files</h3>\n      <ul class="others">%s\n      </ul>' % rows_html(key, extra[key], url))
        if not parts:
            parts.append('<p class="later">%s</p>' % ("No add-on packages of their own in this version." if full[key] else "Comes later."))
        sections.append('    <section class="system" data-system="%s" id="%s">\n      <h2>%s <span>%s</span></h2>\n      %s\n    </section>'
                        % (key, key, html.escape(name), html.escape(detail), "\n      ".join(parts)))
    body = ("""  <h1>Quillbinder %(shown)s: add-ons and other files</h1>
  <p class="lead">The full version of each system already has its add-ons: <a href="../">the downloads</a>.</p>
  <div id="systems">
%(sections)s
  </div>
%(official)s  <p class="more"><a href="../">Back to the downloads</a> &middot;
  <a href="%(release)s">All files of this version</a> (the symbols for crash reports, checksums) &middot;
  <a href="%(home)s/releases">Earlier versions</a></p>
""" % {"shown": html.escape(shown), "sections": "\n".join(sections), "official": addons_section(official_addons(), home),
       "release": html.escape("%s/releases/tag/%s" % (home, tag)), "home": html.escape(home)}) + note()
    return document("Quillbinder Add-ons", body)


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
    os.makedirs(os.path.join(argv[3], "addons"), exist_ok=True)
    with open(os.path.join(argv[3], "index.html"), "w", encoding="utf-8") as f:
        f.write(page(files, argv[2], also, addon_bytes))
    with open(os.path.join(argv[3], "addons", "index.html"), "w", encoding="utf-8") as f:
        f.write(addons_page(files, argv[2], also))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
