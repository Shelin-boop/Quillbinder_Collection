# Developer Guide: Updates and Add-ons

This guide is for people who build {{APP}} or write extensions for it. It explains how program updates and add-ons work, how they are made and checked, and which tools in the source tree do the work. Writers do not need it: they install packages with **Help > Add-ons and Updates > Install Update or Add-on** (manual, section 17).

This copy belongs to program version {{VERSION}}, runtime `{{RUNTIME}}`. The source tree has the same text in `docs/developer-guide.md`; what each version brought is in `docs/CHANGES.md`.

---

## 1. The idea in short

{{APP}} is delivered as an **AppImage**: one read-only file that holds the program and the libraries it needs (Qt 6.4.2 of Ubuntu 24.04 and a few more). The user runs the AppImage 0.30.0 alpha (2026-10-25; before it 0.20.0, 0.16.0 and 0.2.0, which have the same libraries). A new AppImage is rare; everything else comes as small **package files** ending in `.qbpkg`:

- a **program update** (kind `core`) is a newer program binary, without libraries. It runs on the libraries inside the user's AppImage;
- an **add-on** (kind `addon`) is content (themes, templates, compile formats, dictionaries, layouts, script formats) and, if needed, plugins and libraries of its own;
- a **bundle** (kind `bundle`) packs several packages that are installed in one step.

Nothing is downloaded by the program: the user installs a package file (offline install). Packages carry SHA-256 sums of their files; they are not signed (decision of 2026-10-03).

---

## 2. The package file

A `.qbpkg` is a ZIP archive:

    manifest.json          what the package is
    payload/...            the files (add-ons and program updates)
    packages/*.qbpkg       only in bundles: the packages inside

`manifest.json` (see `src/core/Addons.h`, struct `Manifest`):

| Key | Meaning |
|-----|---------|
| `format` | 1 |
| `id` | lower case, e.g. `themes-extra`; program updates use `core` |
| `name`, `version`, `description`, `author` | shown in the install window and the Add-on Manager; versions are `1.2` or `1.2.3` |
| `kind` | `addon`, `core` or `bundle` |
| `whatsNew` | list of lines, shown before installing |
| `details` | optional (0.94.0): list of technical lines (libraries, engines, the runtime), shown under **Details** (folded) in the install window; `description` is for writers |
| `minApp` | add-on: the lowest program version that can use it |
| `runtime` | program update: the runtime (library set) it was built for |
| `binary` | program update: the name of the program file inside `payload/` |
| `files` | `path` (relative to `payload/`), `sha256`, `size`, `executable` |
| `packages` | bundle: the file names inside `packages/` |

---

## 3. What happens when a package is installed

`addons::install()` in `src/core/Addons.cpp` does this (also from the command line: `quillbinder --install file.qbpkg`):

1. **Inspect.** The manifest is read; every file of `payload/` must be listed, with the right size and SHA-256 sum. A cut-off or changed package is refused.
2. **Fit.** An add-on with a `minApp` above the running version is refused. A program update whose `runtime` differs from the AppImage's (`APP_RUNTIME` in `CMakeLists.txt`, now `{{RUNTIME}}`) is refused: it would not find the libraries it was built against.
3. **Unpack** into a temporary folder `<data>/addons/.new-<id>` or `<data>/core/.new-<version>` (`<data>` is `~/.local/share/Quillbinder`).
4. **Self-test** (program updates only): the new binary is started once, invisibly (`--selftest`, platform `offscreen`), on the AppImage's libraries. If it does not answer "selftest ok", nothing is installed.
5. **Move into place:** `<data>/addons/<id>/` or `<data>/core/<version>/`. A bundle installs its add-ons first and the program update last.
6. **Activate** (program updates): `<data>/core/state.json` gets `active` = the new version, `previous` = the version used before, `attempts` = 0. The user is asked to restart; the open project is saved and reopened.

---

## 4. Starting an installed update (the loader)

The AppImage cannot be changed, so the program built into it acts as a **loader** (`src/core/Launcher.*`, called first in `main()`):

1. At start it reads `state.json`. If a newer program version is active and was built for the same runtime, it starts that binary in its place (`exec`), with the AppImage's libraries and plugins.
2. Every such start is counted in `attempts`. The new version calls `launcher::confirmStart()` once its window is up, which sets the counter back to 0.
3. After **two starts in a row** that did not get that far, the loader switches the update off and the built-in version runs again, with a message for the user. Nothing in the projects changes.
4. **Help > Add-ons and Updates > Add-on Manager** lists every program version (`addons::coreVersions()`); the user can go back to an earlier one or remove one. `quillbinder --versions` prints the same list.

---

## 5. Add-ons

Installed add-ons live in `<data>/addons/<id>/`. They can be switched off (they stay installed) or removed in the Add-on Manager. The program looks for content in folders of a fixed name inside every enabled add-on (`addons::contentDirs(kind)`):

| Folder | Content | Used by |
|--------|---------|---------|
| `themes/*.json` | color themes | View > Themes |
| `layouts/*.json` | window layouts | View > Layout |
| `scripts/*.json` | script formats | Format > Scriptwriting |
| `templates/*.zip` | project templates | New Project |
| `compile-formats/*.json` | compile formats | Compile |
| `dictionaries/*.aff, *.dic` | spelling dictionaries | Options > Editing |
| `plugins/*.so, *.json` | plugins (below) | at program start |

An add-on folder in the source tree (`addons/<id>/`) holds `manifest.json` and `payload/`. Extras:

- `outline.json` or `outlines/*.json`: project templates written as outlines; `packaging/linux/build-addons.sh` turns them into template files with `build/qb_template` (`tools/qb-template.cpp`, which uses `src/core/OutlineProject.*` like the welcome project of the program);
- `plugins.txt`: the plugin libraries (built into `build/tool-plugins/`) that are copied into the package;
- `stage.sh`: a script that copies further files in (libraries of the Media Playback add-on, Qt's TLS plugin of the Web Page Import add-on).

---

## 6. Plugins

Plugins are Qt plugins (`QPluginLoader`) found in `plugins/` folders: of the AppImage, of every enabled add-on, of `<data>/plugins` (**Help > Troubleshooting and Development > Open Plugin Folder**) and of `QB_PLUGIN_PATH` (colon-separated, for developers; since 0.64.1 only in builds with the tests, `-DQB_TEST_SWITCHES=ON`). They talk to the program only through **interface headers of pure virtual classes**; nothing is linked against the program. A plugin built for another interface version is not loaded.

| Interface | Header | IID | Examples |
|-----------|--------|-----|----------|
| compile output | `src/compile/ExporterApi.h` | `org.quillbinder.ExporterInterface/1.4` | Markdown exporter (part of every program update) |
| writing tools | `src/tools/ToolApi.h` | `org.quillbinder.ToolInterface/1.0`, since 0.61 also `org.quillbinder.ToolExtension/1.1` | Name Generator, Scratch Pad, Text Statistics; Speech Transcription (1.1) |
| importers | `src/import/ImporterApi.h` | `org.quillbinder.ImporterInterface/1.0` | Word 97–2003, Web Page |
| audio and video | `src/media/MediaApi.h` | `org.quillbinder.MediaEngine/1.0` | Media Playback |

Command plugins (`plugins/command/*.json`) run another program (LibreOffice, Pandoc) for a compile output; they need no code.

Plugins are loaded once at program start, so an add-on with plugins needs a restart after it is installed.

Compile output 1.4 (0.62): `Manuscript::tables` holds the tables of the text (rows, columns, column shares, lines,
padding, header rows, width, alignment, the place beside the text, and each cell once with its spans and background);
the paragraphs of a table cell carry `table` and `cell` and follow each other cell by cell. A picture with its caption:
the picture's paragraph has `figure` (1 left and 2 right with the text flowing around it, 3 centered), the caption's
paragraphs after it `figureCaption`. `Run::imageDescription` is the alt text of a picture. An exporter that knows none
of this still gets every paragraph in order. The IID changed, so exporters built for 1.3 must be rebuilt.

Writing tools 1.1 (0.61): a tool may also implement `ToolExtension` (list both interfaces in `Q_INTERFACES`, keep the
IID of `Q_PLUGIN_METADATA` at 1.0) for tabs in the inspector, a button with a menu on the main toolbar and a section of
its own in File > Options, and call the three newer `ToolHost` functions when `apiVersion()` is 110 or more. The 1.0
parts never change, so tools built for 1.0 keep working and a 1.1 tool still loads in an older program (without the
1.1 parts). A function that needs more of the program than the interface offers is the only reason for a program
update; everything else of such a tool comes with an update of its add-on.

---

## 7. The rules for a program update

- Build on **Ubuntu 24.04 with Ubuntu's Qt 6.4.2**: the update runs on the AppImage's Qt. Use no Qt API newer than 6.4.
- Add **no new library dependency**. Something that needs a library goes into an add-on that brings the library (see `addons/media-playback`, which loads its own FFmpeg and Qt Multimedia by full path).
- `src/text/PageLayout.cpp` calls a private Qt class; the self-test exercises it, so an update built against another Qt is refused at installation.
- Raise `APP_VERSION` in `CMakeLists.txt` for every delivery (corrections: third number; a milestone: second number). The version must be above the version of the user's AppImage (now 0.20.0): an update that is older than the program built into the AppImage is not used. `APP_RUNTIME` changes only when a new AppImage brings other libraries (the AppImages 0.16.0 and 0.20.0 kept `u2404-qt6.4-r1`).
- Projects must stay readable by older versions where possible; a project that needs a newer version names it in the attribute `needs` of the project file, and older versions ask before opening it (from 0.10 on).

---

## 8. Tools in the source tree

**Where is what** (2026-11-12). One source for every system:

| Folder | What is in it |
|--------|---------------|
| `src/` | the program: `core` (projects, saving, backups, packages), `text` (formatting, Markdown and RTF), `ui` (the windows), `compile`, `import`, `tools` (the interface of the writing tools) |
| `plugins/`, `addons/` | the writing tools, importers and media playback as plugins, and the add-on packages made of them |
| `resources/`, `translations/` | icons, templates, the welcome project, the license lists (Linux and Windows); the German texts |
| `tests/` | the test programs (`tools/run-all-tests.sh` runs them on Linux, `packaging/windows/win-tests.sh` on Windows) |
| `packaging/linux/` | building and checking the Linux downloads: AppImage, program update, add-ons, full version, end-to-end checks |
| `packaging/windows/` | building and checking the Windows downloads on GitHub's Windows machine (`.github/workflows/windows.yml`) |
| `packaging/android/` | planned |
| `tools/` | shared tools: packages, release text, download page, publishing, test runner, the build machine's set-up |
| `docs/` | the user manual, the quick start guide, this guide, the list of changes |

Downloads: published versions on this repository's Releases page and its download page (GitHub Pages, one
button per system for the full version, `tools/download-page.py`).
Test builds before publishing go on a test release (`tools/test-release.sh`, started by pushing the branch
`test-release` with its list `test-release.txt`; `.github/workflows/test-release.yml`).
Work branches are named after the system they are for: `linux/...`, `win/...`, `android/...`, or `all/...`.

| Tool | What it does |
|------|--------------|
| `packaging/linux/build-update.sh "line" ...` | builds a program update from the source: `dist/quillbinder-<version>-update.qbpkg` (own build folder `build-update`); the lines become `whatsNew` |
| `packaging/linux/build-addons.sh [build] [out]` | builds every add-on in `addons/` into `<out>/<id>.qbpkg` |
| `packaging/linux/build-appimage.sh` | builds a new AppImage (rarely; keep `APP_RUNTIME` when the libraries stay the same) |
| `tools/make-package.py addon / core / bundle` | writes `.qbpkg` files with the manifest and the SHA-256 sums; `bundle` packs several packages |
| `packaging/linux/e2e-appimage.sh <AppImage> <update.qbpkg>` | end-to-end check with the real AppImage: install, `--versions`, `--selftest`, a cut-off package, the first start with the welcome project and printing its first document, the second start, a Scrivener sample (`QB_SCRIV_SAMPLE`), the way back (`QB_PREV_UPDATE`); `-` instead of the package checks a new AppImage alone |
| `tools/run-all-tests.sh <out> offscreen\|x11` | every test suite, one line each |
| `tools/cloud-setup.sh` | the packages for a fresh Ubuntu 24.04 (build and tests) |
| `tools/get-ffmpeg.sh`, `tools/build-ffmpeg.sh` | the two FFmpeg sets of the media add-ons: "base" (free formats, Media Playback) and "full" (every decoder, Extra Video and Audio Formats) |
| `tools/deps-pack.sh recipe\|hash\|fetch\|pack` | ready-made packs of FFmpeg and whisper.cpp, named by a hash of their recipe; `get-ffmpeg.sh` and `get-whisper.sh` use a pinned pack and build from source without one (`QB_DEPS_PACK=0`: always build) |
| `tools/qb-template.cpp` | project template files from outlines (the builder is `src/core/OutlineProject.*`, shared with the welcome project) |
| `tools/collect-licenses.py` | the license texts of every library the AppImage carries (`resources/licenses`, Help > Third-Party Licenses) with the name of each library's source file; `--refresh` (versions of the build system and the runtime's commit) and `--check` (same versions as the AppDir) are run by `packaging/linux/build-appimage.sh` |
| `tools/collect-sources.py <out>` | the source code of every third-party part of the AppImage and the add-ons, one `.src.tar` per part, for the release "Third-party source code" (needs `tools/enable-deb-src.sh`) |
| `tools/release-notes.py`, `tools/publish-release.sh` | the text of a release (the files sorted by system under a short headline each) and its publication on the project page (run by `.github/workflows/release.yml`) |
| `tools/download-page.py` | the download page of a release (GitHub Pages; job `download-page` of the release workflow); lists the add-on packages of each system (name and one line from `addons/<id>/manifest.json`, size from the release) and reads the file `full-addons-size` |
| `tools/full-addons-size.py` | the installed size of the full version's add-ons in bytes (0.94.0; the step of `release.yml` that builds the full AppImage writes it, the page job reads it) |
| `tools/make-item-icons.py` | draws the document icons of Documents > Change Icon (0.17) |
| `tools/gen-resources.py`, `tools/update-icon-index.sh` | the resource list after adding icons or documents |

Example: a bundle of a program update and an add-on

    packaging/linux/build-update.sh "Mind maps" "Icon bar"
    packaging/linux/build-addons.sh build dist
    tools/make-package.py bundle --id quillbinder-bundle --name "Quillbinder 0.17" --version 0.17.0 \
        dist/quillbinder-0.17.0-update.qbpkg dist/themes-extra.qbpkg dist/bundle.qbpkg

---

## 9. Delivering a version

1. Raise `APP_VERSION`; write what is new in `docs/CHANGES.md`; bring `docs/manual.md` up to date.
2. Run every test suite, offscreen and under X11 (`tools/run-all-tests.sh`); all must end with "0 failure(s)". New functions get checks of their own in a test program under `tests/`; look at the screenshots the tests save.
3. Build the update package (`packaging/linux/build-update.sh`).
4. Run the end-to-end check with the real AppImage (`packaging/linux/e2e-appimage.sh`; the AppImage 0.40.0 alpha and earlier packages are on the branch `assets` of the repository; `packaging/linux/e2e-old-version.sh` checks that an older AppImage keeps the objects of a project).
5. Commit on a branch, open a pull request, merge it; put the package on the branch `assets` as well.
6. The user installs the package with **Help > Add-ons and Updates > Install Update or Add-on**.
7. A release (0.50): merge the version's pull request into `main`; a version on `main` that has no release yet (or a tag `v<version>-<label>` pushed from a computer, for example `v0.50.0-alpha`) starts `.github/workflows/release.yml`, which builds everything on GitHub in a clean Ubuntu 24.04, checks the AppImage, collects the third-party source code and publishes the release with the AppImage, the program update, the add-ons, the checksums and the list of source files; the source files go into the release "Third-party source code" (added once, never removed). Releases and tags cannot be made from a Claude Code session; the workflow makes both with GitHub's own token.

---

## 10. Reading a crash or freeze report

Since 0.19 the program writes a report into `~/.local/share/Quillbinder/CrashReports` when it ends with a fatal signal (`crash-<unix time>-<pid>.txt`) or when its main thread does not answer for 30 s (`freeze-...`). The user copies it from **Help > Troubleshooting and Development > Crash Reports**. The code is `src/core/CrashGuard.*` (signal handlers, watchdog, session markers) and `src/ui/CrashDialogs.*` (snapshot, dialogs, recorder of recent actions).

A report has these parts: the header (signal, version, process, time), **Program and system** (the text of Help > Troubleshooting and Development > System Information, prepared before the crash, with "Things to look at" first), **Process** (memory, threads), **Recent actions** (commands, shortcuts, buttons, windows, keys; typing is only counted), **Recent warnings** (Qt messages), the **Backtrace** (crash) or **Where the main thread is** (freeze), and **Program files in memory**.

Backtrace lines look like `quillbinder(_ZN2qb14TextColumnView4pageEPNS_10TextEditorEb+0x1e)[0x5621a16d853d]`. The program exports its symbols (`ENABLE_EXPORTS`), so most functions are named even in the stripped program of an update; Help > Troubleshooting and Development > Crash Reports shows the names readable (`qb::TextColumnView::page(qb::TextEditor*, bool)`). A line without a name, such as `quillbinder(+0x55253d)`, is a static function: take the program file of that version with symbols (`dist/quillbinder-<version>-symbols.xz`, kept on the branch `assets` since 0.19; its build ID is the one in the report) and run

    xz -dc quillbinder-0.19.0-symbols.xz > quillbinder-symbols
    addr2line -f -C -e quillbinder-symbols 0x55253d

The offsets always refer to the program file named under **Program** in the report (with its build ID). When an update runs on an AppImage, the backtrace can show the start name `.../AppRun` in front of the parentheses, although the code is that of the update in `~/.local/share/Quillbinder/core/<version>/`; **Program files in memory** shows the real paths. For the program built into an AppImage, extract that AppImage's program file with `--appimage-extract`.

---

## 11. The objects of a project (0.20)

The Objects sheet (manual, section 7) keeps its data in the project folder, beside the project file, never in it: versions before 0.20 read and write the project file as before and leave the folder `objects/` alone. The code is `src/core/Objects.*` (model, templates, storage; `Project::objects()` loads it on first use) and `src/ui/ObjectsUi.*` (inspector tab, window, form, templates dialog).

    objects/schema.json             {"format": 1, "categories": [...], "folders": [...]}
    objects/<category id>/<id>.json one object
    objects/images/                 pictures of objects (<object>-<field>-<random>.png / .jpg)

A category is `{"id", "name", "color": "#rrggbb", "fields": [...]}`; a field is `{"id", "label", "type", "unit"?, "choices"?, "targets"?}` with the types `text`, `longtext`, `number`, `choice`, `yesno`, `date`, `picture`, `objects` (links to objects; `targets` = the category ids it may link to, empty = all) and `documents` (links to binder items). The standard categories have the ids `characters`, `locations`, `environments`, `pois`, `weather`; categories and fields made by the writer get random ids (`cat-...`, `f-...`), so renaming never touches the objects.

An object is `{"format": 1, "id", "category", "name", "folder"?, "color"?, "created", "modified", "fields": {<field id>: value}}`: a string for text, long text (Markdown), number, choice and date, `true` for yes, a list of ids for links, the file name in `objects/images` for a picture. Values of fields that the template no longer has stay in the file. Links inside long texts are Markdown links to `qbobj://<id>` (document links: `qb://<id>`, as in the texts of documents); a text of a document links to an object the same way. Since 0.21.1 a long text can hold text colors, highlights and underlines: such pieces (and pieces with more than one of bold, italic and strikethrough) are written as HTML spans inside the Markdown, for example `<span style="font-weight:700; color:#d62728;">north</span>`, with characters that Markdown or HTML would read as markup written as character references; Qt's Markdown reader shows them (also in 0.20 and 0.21).

Nothing is written until the first object is made or a template is changed. Files are written with `QSaveFile` (a temporary file, then renamed). A file that cannot be read is reported when the project opens and never overwritten; an object of a category that the templates do not have gets a category of that id; a damaged `schema.json` is renamed to `schema.json.damaged-<time>` before a new one is written. The templates for new projects (**Use for New Projects**) are in `~/.local/share/Quillbinder/object-templates.json` (same layout, without folders).

---

## 12. Working with Claude Code

The repository holds a `CLAUDE.md` with the rules above and the user's preferences (a short index; the notes per topic are in `docs/dev-notes/`), so that a Claude Code session (or a person) can take over: set up a fresh Ubuntu 24.04 with `tools/cloud-setup.sh`, build, run the tests, build the package, check it with the real AppImage, and hand it over as described in section 9.

---

## 13. Plugins, packages, and the project folder (moved here from the user manual)

This part is for the technically minded: it was section 20 of the user manual until 0.94.0.

### Media engines

Audio and video are played by a plugin of the add-on *Media Playback* that implements `src/media/MediaApi.h` of the source (Qt Multimedia 6.12 with a decoding-only FFmpeg 9.1, see `addons/media-playback` and `tools/build-ffmpeg.sh`). The add-on keeps its libraries in its own folder (`media/lib`), with the parts of Qt its decoding needs that the AppImage does not carry (Qt Quick and Qt QML). Its FFmpeg decodes free formats only; the add-on *Extra Video and Audio Formats* brings a full FFmpeg of the same version in `media-extra/lib`, which *Media Playback* loads instead while that add-on is installed and switched on. **File > Options > Extras > Add-ons and Plugins** lists the engine.

### Adding compile formats

**File > Options > Extras > Add-ons and Plugins > Open My Plugin Folder** opens *~/.local/share/{{APP}}/plugins*. Two kinds of plugins go there (or into the `plugins` folder of an add-on package):

- **Command plugins**: a small `.json` file that tells {{APP}} to run a converter program (such as LibreOffice or Pandoc) on the Word file it has just written. Copy one of the bundled examples and change the program and arguments. The keys are `id`, `name`, `extension`, `input` (`docx` or `md`), `program` or `programs`, `arguments` (with `{input}`, `{inputbase}`, `{outdir}`, `{output}`, `{tmpdir}`, `{title}`, `{author}`), `output`, and `timeout`.
- **Qt plugins**: a shared library (`.so`) built against Qt 6 that implements the `ExporterInterface` from the header *ExporterApi.h* in the source code. It receives the compiled manuscript as a simple model of paragraphs, formatted runs, footnotes, and comments. The Markdown exporter in the source code is a complete example. Version 0.3 extended the model with fonts and paragraph formatting (interface 1.1); a plugin built for an earlier version is not loaded until it is built again.

Restart {{APP}} after adding a plugin.

### Package files (.qbpkg)

A package is a ZIP file with a `manifest.json` (id, name, version, kind, description, what's new, required versions, and a list of files with SHA-256 checksums) and a `payload` folder. In an add-on, the payload uses these folders: `themes` (color themes as JSON), `layouts` (window layouts as JSON), `scripts` (script formats as JSON), `templates` (project templates as ZIP), `compile-formats` (JSON), `plugins` (command plugins and Qt plugins), and `dictionaries` (Hunspell files). The source code contains the tool *tools/make-package.py*, which writes the manifest and builds the file, and example add-ons in the *addons* folder.

Installed add-ons are stored in *~/.local/share/{{APP}}/addons*, program updates in *~/.local/share/{{APP}}/core*.

### Theme files

A theme is a small JSON file: in an add-on as *themes/name.json*, your own themes in *~/.local/share/{{APP}}/themes*, exported themes with the ending *.qbtheme*.

```
{ "name": "Northern Night",
  "colors": { "window": "#2b303b", "text": "#e3e7ee", "highlight": "#5e81ac", "cardPaper": "#434c5e" },
  "fonts":  { "binder": { "family": "Noto Sans", "size": 10.5, "bold": false, "italic": false } } }
```

Only `window` and `text` are required; every other color is derived when it is missing. The color names, in the order of the options: `window`, `windowText`, `base`, `inspectorFields` (Inspector Background), `alternateBase`, `text`, `button`, `buttonText`, `highlight`, `highlightedText`, `link`, `toolTipBase`, `toolTipText`, `mid`, `dark`, `light`, `disabledText`, `targetSession`, `targetDraft`, `targetReached`; `binderBackground`, `binderText`; `editorPaper`, `editorText`, `editorMargin`, `editorLink`, `searchHit`, `spellingError`, `headerBar`, `headerBarText`, `headerBarActive`, `headerBarLocked`; `corkboardBackground`, `cardPaper`, `cardTitle`, `cardText`, `cardTitleLine`, `cardLines`, `statusStamp`; `outlinerBackground`, `outlinerAlternate`, `outlinerText`; `inspectorBackground`, `inspectorText`, `synopsisPaper`, `synopsisText`, `notesPaper`, `notesText`; `compositionBackground`, `compositionPaper`, `compositionText`. Font names: `interface`, `binder`, `headerTitle`, `combinedTitle` (written `scriveningsTitle` before 0.40; still read), `cardTitle`, `cardText`, `outliner`, `synopsis`; in a font, leave out what should stay as it is. `"followDesktop": true` takes the window colors from the desktop. Theme files written for versions before 0.5 work unchanged, and versions before 0.5 read the window colors of newer files and ignore the rest.

### Layout and shortcut files

A layout is a JSON file: in an add-on as *layouts/name.json*, your own layouts in *~/.local/share/{{APP}}/layouts*, exported layouts with the ending *.qblayout*.

```
{ "format": "qblayout", "version": 1, "name": "Research Desk",
  "state": { "binder": true, "inspector": false, "toolbar": true, "formatBar": true,
             "binderShare": 0.17, "split": "vertical", "splitShare": 0.42,
             "panes": [ { "view": "outliner", "affectsOther": true }, { "view": "text" } ],
             "binderAffects": "first", "active": 1 } }
```

`split` is `none`, `horizontal` (one editor above the other), or `vertical` (side by side). `view` is `text`, `corkboard`, `outliner`, or `keep`; a corkboard can add `"cork": "grid"`, `"freeform"`, or `"labels"`. `binderShare`, `inspectorShare`, and `splitShare` are shares of the width (or height), so a layout fits small and large screens. `binderAffects` is `current`, `first`, `second`, or `other`. Optional: `collections`, `ruler`, `inspectorTab` (0 to 4), and `"window": { "width": 1400, "height": 900, "maximized": false }`.

A shortcut file (*.qbkeys*) lists only the changes: `{ "format": "qbkeys", "version": 1, "shortcuts": { "merge": "Ctrl+Alt+M", "split": "" } }`. The names are those of the commands in the program (an empty text means "no shortcut"); styles are written as `style:Name`, layouts as `layout:id`. The contents of the toolbars and your shortcuts are stored with the other options in *~/.config/{{APP}}/{{APP}}.conf*.

### Script format files

A script format is a JSON file: in an add-on as *scripts/name.json*, your own formats in *~/.local/share/{{APP}}/scripts*, saved formats with the ending *.qbscript*. The quickest way to make one is **Format > Scriptwriting > Script Settings… > Manage > Save to File…**.

```
{ "name": "Radio Play", "description": "...", "font": "", "size": 12, "unit": "in", "first": "Scene Heading",
  "elements": [
    { "name": "Character", "role": "character", "key": "C", "caps": true, "align": "left",
      "left": 55.9, "right": 0, "firstLine": 0, "linesBefore": 1, "lineSpacing": 1, "keepWithNext": true,
      "enter": "Dialogue", "tabEmpty": "Transition", "tabEnd": "Parenthetical", "completeFromScript": true },
    { "name": "Parenthetical", "role": "parenthetical", "key": "P", "left": 40.6, "right": 50.8,
      "linesBefore": 0, "prefix": "(", "suffix": ")", "enter": "Dialogue", "tabEmpty": "Dialogue" } ] }
```

Lengths are millimeters from the text margins (`unit` only says what the settings window shows). `role` is `scene`, `action`, `character`, `parenthetical`, `dialogue`, `transition`, `shot`, `lyrics`, `centered`, or `section`. `enter`, `tabEmpty`, and `tabEnd` name the element that follows; `tabInserts` is a text that Tab types instead. `completions` are offered at the start of the paragraph, `tabCompletions` after the text typed by Tab; with `completeFromScript` the texts of the same element elsewhere in the document are offered too. Also possible: `bold`, `italic`, `underline`, `color`. The name of an element may not contain `:` or `>`.

### Inside a project folder

```
MyNovel{{SUFFIX}}/
  MyNovel{{FILE}}          the binder and all settings (XML)
  content/<ID>/text.md     the text of a document (or text.rtf)
  content/<ID>/notes.md    its notes
  content/<ID>/images/     pictures used in the text
  content/<ID>/file.pdf    an imported file (pictures, PDFs …)
  content/<ID>/mindmap.scap  the mind map of a document or folder
  icons/                   the project's own icons
  objects/schema.json      categories, templates, colors, and folders of the objects
  objects/<category>/<ID>.json  one object (a character, a location ...)
  objects/images/          pictures of objects
  snapshots/<ID>/          snapshots of a document
  settings/                window layout and other preferences
  settings/spare-copy{{FILE}}  the project file as it was when the project was last opened
  settings/spare-copy-2{{FILE}}, spare-copy-3{{FILE}}  the project file as it was at the two openings before (0.81)
  settings/look.json       the project's look: theme, colors, fonts, column width, border (0.40.2)
  settings/options.ini     the project's own options (File > Options, "This project"; 0.62)
  settings/settings-before-loading-<date>.qbsettings  the settings before Load Project Settings
```

A project in Scrivener format has a working copy of this kind in a folder of its own under *~/.local/share/{{APP}}/ScrivenerProjects/*; its *settings/scrivener-sync.json* records what was written into the *.scriv* the last time. Deleting the working copy is safe: the next opening asks the first-time question again and reads the *.scriv* anew (what only {{APP}} knew, such as styles and compile settings, is then lost).

---

## 14. Environment variables

{{APP}} reads a few environment variables at its start. You never need them; they are listed so that nothing changes its behavior without being written down (0.81). Since 0.81 the switches that only the test suites use (for example to answer every warning window by itself, or to make two "computers" on one machine) are not part of the released program at all.

- `QB_LANGUAGE=de` or `en`: the language of the interface for this start (instead of the one chosen in **File > Options**).
- `QB_KEEP_FONT_FAMILIES=1`: font families with several designs (such as Sitka) are left as the system lists them.
- `QB_NO_UPDATES=1`: the AppImage starts its own version, without an installed program update (a way back when an update does not start).
- `QB_ALLOW_UNSIGNED_CORE=1`: an installed program update without a valid signature is started anyway. Only for checks of builds that were never signed; do not set it otherwise.
- `QB_TIMING=1`, `QB_DEBUG_ENV=1`, `QB_DEBUG_LINK=1`, `QB_DEBUG_SHORTCUTS=1`, `QB_ZOOM_DEBUG=1`, `QB_MIRROR_DEBUG=1`, `QB_SPEECH_DEBUG=1`: messages for finding a fault, on the error output.
- Set by the program itself for its own parts (never set them by hand): `QB_CORE_ACTIVE`, `QB_CORE_BASE`, `QB_CORE_APPDIR`, `QB_ORIG_PATH`, `QB_ORIG_LD_LIBRARY_PATH`, `QB_MEDIA_DIR`, `QB_MEDIA_EXTRA_DIR`, `QB_MEDIA_FFMPEG_SET`, `QB_SPEECH_DIR`, `QB_SPEECH_GGML_DIR`, `QB_TOOL_DATA`.

---

## 15. Why some choices look like faults

The reasons behind some deliberate choices that can look like faults (a lock that warns and does not block, a drive that cannot confirm a write, projects that are not encrypted) are written down in [design-decisions.md](https://github.com/Shelin-boop/Quillbinder_Collection/blob/main/docs/design-decisions.md).
