# Quillbinder

A writing studio for long texts (binder, corkboard, outliner, combined text,
snapshots, targets, composition mode, mind maps, object sheets, compile to DOCX,
PDF and EPUB, projects in its own format or in Scrivener 3 format), written in
C++17 with Qt 6.12 for Ubuntu 24.04 LTS and KDE Plasma. Works under Kubuntu 26.04 LTS
Wayland. "Quillbinder" is a working name; it is set once in `CMakeLists.txt` (`APP_NAME`, `APP_ID`).

Quillbinder is an independent program. It is not made by or connected with Literature & Latte,
the maker of Scrivener and Scapple; it reads and writes their file formats so that projects can move
between the programs. Scrivener and Scapple are names of products of Literature & Latte.

Quillbinder contacts the internet for one thing only: to look for updates. It fetches one small file, sends nothing
about you, and you can switch it off in the Help menu.

Found a problem, or have an idea? See [Feedback](#feedback) below.

## Download

**[The download page](https://shelin-boop.github.io/Quillbinder_Collection/)** has one button per system for the full
version (the program with all add-ons), and below it the program alone, the program update and the audio and video
add-ons. The files, sorted by system:

### Linux

**Quillbinder 0.93.0 beta for Linux (x86_64):**

- **[Quillbinder-0.93.0-beta-x86_64.AppImage](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.93.0-beta/Quillbinder-0.93.0-beta-x86_64.AppImage)**: the program
- **[quillbinder-0.93.0-update.qbpkg](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.93.0-beta/quillbinder-0.93.0-update.qbpkg)**: the program update, for a Quillbinder you already have (0.80.0 or later; **File > Install Update or Add-on**)
- **[media-all-formats.qbpkg](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.93.0-beta/media-all-formats.qbpkg)**: the commonly used audio and video formats (an add-on; optional)
- **[Quillbinder-0.93.0-full-beta-x86_64.AppImage](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.93.0-beta/Quillbinder-0.93.0-full-beta-x86_64.AppImage)**:
  **the full version**: the program with all add-ons and all the libraries they need included (the commonly
  used audio and video formats, extra themes, project templates, the scriptwriting pack, the writing tools with offline speech recognition, the
  importers of web pages and old Word files). They are installed at the first start: one download, nothing
  else to install.
- **Translation and Proofreading** (English proofreading and German-to-English translation on your computer) is one
  of Quillbinder's own add-ons since 0.92.0: version 1.2 is inside the full version, and
  [translation-proofreading.qbpkg](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.93.0-beta/translation-proofreading.qbpkg)
  is beside it for a Quillbinder you already have. It downloads the language models you choose; one of them (CoEdIT
  Large) may be used for non-commercial purposes only, and the add-on says so. For Windows it is inside the full
  version too since Windows 0.92.0, and for Android tablets inside the app since Android 0.92.0
- [All files of the newest version](https://github.com/Shelin-boop/Quillbinder_Collection/releases/latest): the other
  add-ons and SHA-256 checksums. Earlier versions: [Releases](https://github.com/Shelin-boop/Quillbinder_Collection/releases).

0.91.3 prepares the list of official add-ons: an add-on can carry a key of its own that the project vouches for,
while program updates still need the project's own key. Its add-ons are signed with that key: with 0.91.2 or
earlier, install the program update first, then the add-ons. 0.91.2 changes nothing in the program: it is built and
released in a faster way, and the download page shows each system's newest version. 0.91.1 lets you arrange the toolbars freely: any order, any toolbar in either row, in a row of its own or at the
bottom edge. The program moves them itself now (in 0.91.0 a toolbar could not leave its row on KDE with Wayland),
and a row no longer gets stuck when a toolbar stands at its far right. 0.91.0 puts all backups straight into the backup folder again, splits the toolbars into eight that can be moved by
a grip and switched off one by one, and gives the theme buttons a click that switches: Bright, Dark, and U1 and U2
for two looks of your own. 0.90.4 changes nothing in the writing work on Linux: it is the first version whose source also builds the Windows
version, which is still being tested and not published. 0.90.3 made backups safe in a folder that a sync program
watches. 0.90.2 added a theme list with Bright, Dark and Both to the main toolbar and a color of its own for Show
Invisibles, and leaves where an option is kept to its tool tip. 0.90.1 keeps long writing sessions light on memory and makes everyday work smoother: every option shows whether
the open project or the program keeps it, Undo names its step, dialogs fit small screens, and a few shortcuts were
added. 0.90.0 completed the work on Scrivener compatibility: texts, comments, footnotes, bookmarks, targets, the
writing history, the corkboard, saved searches, index card pictures and the compile title and author are read from
a Scrivener 3 project and written back in Scrivener's own form; 0.81.0 to 0.81.3 brought the fixes of two audits
(backups, restore, damaged documents, interrupted saves) and keywords, custom metadata, collections, styles, inline
notes and web pages in Scrivener format. It is a new AppImage on the same libraries as 0.80.0: the add-ons of 0.80.0
fit it, and 0.80.0 and later can also take the program update (on the Releases page). Since 0.80.0 (a new Qt, 6.12), add-ons with program code installed for earlier versions must be installed
again in their new versions (the bundle on the Releases page, `media-all-formats.qbpkg`, or simply the full
version); program updates of 0.70.0 and earlier do not fit.

To start, make the AppImage executable (right-click > Properties > Permissions > *Is executable*, or
`chmod +x`) and double-click it; **Help > Add to Application Menu** puts it into the menu of the desktop.
Add-ons are installed with **File > Install Update or Add-on**.

### Windows

**Quillbinder 0.92.0 for Windows 11 (64-bit), experimental:** the same functions as Linux 0.92.0 (each system counts
its own fixes in the last number), with Translation and Proofreading inside the full version and the check for
updates. All Windows files: [the release 0.92.0 for Windows](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/v0.92.0-windows-experimental).

- **[Quillbinder-0.92.0-full-experimental-windows-x64-setup.exe](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.92.0-windows-experimental/Quillbinder-0.92.0-full-experimental-windows-x64-setup.exe)**:
  **the full version** (the program with all add-ons) as a setup program. It asks for a folder (for example
  `C:\Users\<your name>\Quillbinder`, not Program Files) and only unpacks into it: no admin rights, no installation,
  nothing in the registry. The same files as a zip:
  [Quillbinder-0.92.0-full-experimental-windows-x64.zip](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.92.0-windows-experimental/Quillbinder-0.92.0-full-experimental-windows-x64.zip).
- The program alone: [setup program](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.92.0-windows-experimental/Quillbinder-0.92.0-experimental-windows-x64-setup.exe),
  [zip](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.92.0-windows-experimental/Quillbinder-0.92.0-experimental-windows-x64.zip). For a Quillbinder you already have:
  [quillbinder-0.92.0-experimental-windows-update.qbpkg](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.92.0-windows-experimental/quillbinder-0.92.0-experimental-windows-update.qbpkg)
  (**File > Install Update or Add-on**; the program update and every add-on in one package), or run the new setup
  program into the same folder: it keeps your settings. The release also has the program update alone and each
  add-on as a package of its own.
- From 0.92.0 on Quillbinder for Windows looks for updates itself, as on Linux. A later version that needs the whole
  program anew is downloaded as its setup program, checked against the project's signed list, and unpacked into the
  folder you use; your settings and add-ons stay.

The files carry no code-signing certificate: Windows SmartScreen warns (**More info > Run anyway**), and a PC with
Smart App Control switched on blocks them. The packages are signed as on Linux (the program and its update with the
project's key, the add-ons with the add-on key it vouches for); the checksums of the Windows files are in `SHA256SUMS-windows.txt` (signed, `SHA256SUMS-windows.txt.sig`). Everything the program keeps
stays in its own folder. Not in the Windows version yet: Speech Transcription on graphics cards (it uses the
processor). A later version for Linux can be out before its Windows files are.

### Android

**Quillbinder 0.92.0 for Android tablets, experimental:** the same functions as Linux 0.92.0, made for a tablet with
an external keyboard and mouse and for fingers (Android 9 or later, 64-bit ARM). All Android files:
[the release 0.92.0 for Android](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/v0.92.0-android-experimental).

- **[Quillbinder-0.92.0-experimental-android-arm64.apk](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.92.0-android-experimental/Quillbinder-0.92.0-experimental-android-arm64.apk)**:
  the app, with the spelling dictionaries, the writing tools, the importers and Translation and Proofreading. Open it
  on the tablet; Android asks whether the browser or the Files app may install apps. At the first start Quillbinder
  asks for "All files access": with it, projects, backups and exports are kept in *Documents/Quillbinder*, where other
  apps and your computer can reach them. A new version comes as a new APK, which installs over the old one and keeps
  your settings; Help > Check for Updates Now tells you when there is one.
- [translation-proofreading-android-arm64.qbpkg](https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/v0.92.0-android-experimental/translation-proofreading-android-arm64.qbpkg):
  Translation and Proofreading on its own (the app has it already), for **File > Install Update or Add-on**.

The APK is signed with the project's Android key (its certificate's SHA-256 is in `packaging/android/release-key.txt`);
the checksums of the Android files are in `SHA256SUMS-android.txt` (signed, `SHA256SUMS-android.txt.sig`). Not in the
Android version yet: Speech Transcription, audio and video in the editor, crash reports.

### How the downloads are made

Everything on the Releases page is built by GitHub from this source code (`.github/workflows/release.yml`). Since 0.65.0 every package and the checksum list
(`SHA256SUMS.txt`, with its signature `SHA256SUMS.txt.sig`) are signed with the project's key
(`packaging/quillbinder-signing-public.pem`); the program installs program updates only with that signature.
Since 0.70.0 the release workflow runs the full test suite on every release, and since 0.81.0 nothing is published
before it has passed; the signing key is used only in jobs of their own that run no third-party code.
How to check a download yourself: the user manual, section 17.

**Quillbinder is beta software.** Keep backups of your projects (**File > Back Up > Back Up Now**; the
automatic backups are set under **File > Options > Backup**), and keep a copy of your manuscripts outside
the program.

Audio and video: download **`media-all-formats.qbpkg`** and install it with **File > Install Update or
Add-on**: one package for the commonly used formats. It installs the add-on "Media Playback" (free formats: MP3, Ogg Vorbis,
Opus, FLAC, WAV, WebM with VP8 or VP9 ...) together with "Extra Video and Audio Formats" (H.264, H.265, AAC and
the other formats licensed under patents). Who wants the free formats only installs `media-playback.qbpkg`
alone. Whether a patent license is needed to use the other formats depends on the country.

The source code of every third-party library the AppImage and the add-ons carry, in exactly the versions
used, is in the release
[Third-party source code](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources)
(`tools/collect-sources.py` makes those files).

## Feedback

Quillbinder is beta software, and reports from real use are what it needs most. You need a GitHub account;
write in English or German.

- **Something does not work, or text was damaged or lost:**
  [report a problem](https://github.com/Shelin-boop/Quillbinder_Collection/issues/new?template=bug-report.yml).
  Do not attach your manuscript or your project: issues are public.
- **Something Quillbinder should be able to do:**
  [propose a function](https://github.com/Shelin-boop/Quillbinder_Collection/issues/new?template=feature-request.yml).
- **A question, or an idea to talk over first:** [Discussions](https://github.com/Shelin-boop/Quillbinder_Collection/discussions).
- **A security problem:** not in a public issue; see [SECURITY.md](SECURITY.md).
- **A change to the source, or an add-on of your own:** see [CONTRIBUTING.md](CONTRIBUTING.md).

## Documentation

The user manual is `docs/manual.md` (shown in the program with F1); `docs/quick-start.md` is the
Quick Start Guide (Help > Quick Start Guide); the welcome project that opens at the first start is
written in `resources/welcome`. `docs/CHANGES.md` lists what each version brought. Working with Claude Code: `CLAUDE.md`.

## Build (Ubuntu 24.04)

Since 0.80.0 Quillbinder is built with The Qt Company's own build of Qt 6.12.0 (Ubuntu 24.04's Qt 6.4.2 is too old);
`tools/get-qt.sh` installs it into `/opt/qt/6.12.1/gcc_64` with aqtinstall (`-DQB_QT_DIR=<folder>` names another
place):

```
sudo apt install build-essential cmake python3-pip libhunspell-dev zlib1g-dev libgl-dev libegl-dev \
     libxkbcommon-dev libfontconfig1-dev libxcb-cursor0
sudo tools/get-qt.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/quillbinder
```

A build like this behaves like a release (since 0.70.0 the test switches are off unless asked for). To run the
test suites, configure with `-DQB_TEST_SWITCHES=ON`:

```
cmake -S . -B build -DQB_TEST_SWITCHES=ON
```

## Tests

```
tools/cloud-setup.sh                         # (as root) everything to build and test on a fresh Ubuntu 24.04
tools/run-all-tests.sh build/test-results offscreen   # every suite, one line each; "x11" instead of offscreen for a virtual display
tools/get-ffmpeg.sh                          # once: FFmpeg for the Media Playback add-on (the runner builds the add-ons then)
packaging/linux/e2e-appimage.sh <AppImage> dist/quillbinder-<version>-update.qbpkg   # the update in the real AppImage
QT_QPA_PLATFORM=offscreen ./build/tests/qb_tests               # formats, compile, import
QT_QPA_PLATFORM=offscreen ./build/tests/qb_ui_tests /tmp/shots  # scripted UI run + screenshots
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m6_tests /tmp/shots  # page view and copyholders
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m7_tests /tmp/shots  # drag and drop, navigation, Quick Reference panels
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m8_tests /tmp/shots  # search and replace, filters, find by formatting
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m9_tests /tmp/shots  # scriptwriting (the add-on part needs packaging/linux/build-addons.sh build build/addons-dist)
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m10_tests /tmp/shots # writing tools (installs build/addons-dist/tool-*.qbpkg; build them first as above)
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m11_tests /tmp/shots # audio and video (installs build/addons-dist/media-playback.qbpkg; needs a sound server, see below)
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m12_tests /tmp/shots # .doc and web page import, export options (installs build/addons-dist/import-*.qbpkg; serves its test pages itself)
QT_QPA_PLATFORM=offscreen ./build/tests/qb_m13_tests /tmp/shots # Scrivener project format, compile window and format editor, PDF / EPUB, menu audit (installs build/addons-dist/*.qbpkg when built)
QT_QPA_PLATFORM=offscreen ./build/tests/qb_fixes_tests /tmp/shots # corrections of 0.14.1: fonts project-wide, label tint, bookmarks, icons, search marks
QT_QPA_PLATFORM=offscreen ./build/tests/qb_v020_tests /tmp/shots # 0.20: the Objects sheet and the data-safety fixes (QB_SKIP_BIG_BACKUP=1: without the 4.3 GB backup)
QT_QPA_PLATFORM=offscreen ./build/tests/qb_v040_tests /tmp/shots # 0.40: welcome project, Quick Start Guide, third-party licenses, About
packaging/linux/e2e-old-version.sh <older AppImage>    # 0.20: a project with objects opened and saved by an older version keeps them
QB_SCRIV_SAMPLE=/path/to/sample.scriv QT_QPA_PLATFORM=offscreen ./build/tests/qb_tests   # also round-trips a real Scrivener 3 project
# under X11 the M6 and M7 runs also drag with the mouse: xvfb-run -a -s "-screen 0 1920x1080x24" ./build/tests/qb_m7_tests /tmp/shots
```

## AppImage

```
packaging/linux/build-appimage.sh      # downloads linuxdeploy/appimagetool on first run
```

The result is `build-appimage/Quillbinder-<version>-x86_64.AppImage`. It uses
the static AppImage runtime, so it does not need libfuse2. The script checks with
`tools/collect-licenses.py --check` that every library it carries has its license text, and puts the
texts into the AppImage (`usr/share/doc/quillbinder/licenses`).

## Updates and add-ons

Updates and extensions are delivered as `.qbpkg` packages (see `src/core/Addons.h` and section 17 of the manual).
Wrote an add-on of your own and want it listed as an official add-on (reviewed, built and signed by the project, its
source staying in your repository)? See [docs/official-addons.md](docs/official-addons.md) and the issue form "Add-on
for the official list".

```
packaging/linux/build-update.sh "What is new ..."   # program update -> dist/quillbinder-<version>-update.qbpkg
packaging/linux/build-addons.sh build dist          # add-ons in addons/ -> dist/<id>.qbpkg
tools/make-package.py bundle --id ... --name ... --version ... a.qbpkg b.qbpkg out.qbpkg
```

## Releases

Quillbinder is developed in this repository, on branches and pull requests.
A new version on `main` (a change of `CMakeLists.txt` whose version has no release yet), or a
tag `v<version>` (for example `v0.50.0-alpha`, the version and label of `CMakeLists.txt`) pushed from a computer, starts
`.github/workflows/release.yml` on GitHub, which also creates the tag: in a clean Ubuntu 24.04 it builds both FFmpeg sets, the
AppImage, the add-ons and the program update, checks the AppImage (`packaging/linux/e2e-appimage.sh`), collects the source
code of every third-party library (`tools/collect-sources.py`, needs `tools/enable-deb-src.sh`) and publishes the
release (`tools/publish-release.sh`; the text comes from `tools/release-notes.py` and `docs/CHANGES.md`).
`packaging/linux/build-addons.sh` also makes `media-all-formats.qbpkg`, which installs both media add-ons in one step.
`tools/collect-licenses.py --refresh` keeps Help > Third-Party Licenses at the library versions of the build system.

`src/text/PageLayout.cpp` calls a private Qt class (page margins without an entry in the undo history). The program must
therefore run on the Qt it was built with, which is the case for the AppImage and its program updates (same `APP_RUNTIME`);
`--selftest` tries the call, so an update that does not fit is refused when it is installed.

A program update is the program binary only; it runs on the libraries inside the AppImage. `APP_RUNTIME` in
`CMakeLists.txt` names that library set: change it (and ship a new AppImage) whenever the bundled libraries change.
Build updates on the same system as the AppImage (Ubuntu 24.04).

## Writing tools (add-ons with a plugin)

The name generator, the scratch pad and text statistics are Qt plugins that implement `src/tools/ToolApi.h`
(a header of pure virtual classes: the program gives a tool a `ToolHost`; nothing is linked against the
program). They are built into `build/tool-plugins/` and delivered as add-ons (`addons/tool-*`, whose
`plugins.txt` names the library that `packaging/linux/build-addons.sh` copies in). To try them from the build folder:

```
QB_PLUGIN_PATH=build/tool-plugins QB_TOOL_DATA=addons/tool-name-generator/payload ./build/quillbinder
./build/quillbinder --scratch-pad      # the scratch pad without a main window (--tool <command key>)
```

The name lists are made from the Python package Faker (MIT licence) by `tools/make-name-lists.py`.
Running instances find each other through local sockets in `$XDG_RUNTIME_DIR/quillbinder/` (`src/ui/ToolHost.h`).

## Audio and video (add-ons with libraries of their own)

Playback is the add-on "Media Playback" (`addons/media-playback`). Since 1.1 (program 0.50) its FFmpeg decodes
free formats only (set "base" of `tools/build-ffmpeg.sh`); the add-on "Extra Video and Audio Formats"
(`addons/media-extra-formats`) brings the full FFmpeg (set "full"), which Media Playback loads instead while it is
installed and switched on (`plugins/media/MediaEngine.*`). Without it, a file in a format the base set cannot
decode gets a message naming its formats and the add-on (`plugins/media/impl/QtPlayer.cpp`). The program only knows the interface
`src/media/MediaApi.h` and draws the controls (`src/ui/MediaPlayback.*`, `MediaView`, `MainWindowMedia.cpp`);
the add-on brings two libraries from `plugins/media` (a small plugin the program loads at start, and the
player built on Qt Multimedia, loaded with the first audio or video file) plus Qt Multimedia 6.12.0 (with the
Qt Quick and Qt QML libraries its FFmpeg backend needs) and an FFmpeg 9.1 that decodes only. To build the add-on:

```
sudo apt install nasm                              # build time only; Qt Multimedia comes with tools/get-qt.sh
tools/get-ffmpeg.sh                                # FFmpeg 9.1 source (ffmpeg.org, signature checked) -> tools/build-ffmpeg.sh -> build/ffmpeg
                                                   # (set "base") and build/ffmpeg-full (set "full"); applies
                                                   # tools/ffmpeg-patches, about 10 minutes for both on 2 cores
packaging/linux/build-addons.sh build build/addons-dist  # addons/media-*/stage.sh copy the libraries in
```

Why the parts are as they are (details in `plugins/media/MediaEngine.h` and `tools/build-ffmpeg.sh`):
the add-on's libraries are loaded by their full path before Qt Multimedia, so the system's FFmpeg or Qt do
not matter; Qt 6.12's own build has no GStreamer backend, so since 0.80.0 the built-in FFmpeg is the only way;
two FFmpeg decoders are left out because Qt 6.4.2's backend crashed on them (kept so in 0.80.0); one FFmpeg patch gives
Matroska streams a duration (else no seeking in .mkv / .webm); without a sound device the player gets no
audio output (Qt 6.4.2's FFmpeg backend crashes otherwise). The test run needs a sound server; a null
sink is enough: `pulseaudio -D --exit-idle-time=-1`, then `PULSE_SERVER=unix:/tmp/pulse-*/native`.
From the build folder: `QB_PLUGIN_PATH=build/tool-plugins ./build/quillbinder` (uses the system's Qt
Multimedia and FFmpeg).

## Importers: old Word files and web pages (add-ons with a plugin)

`src/import/ImporterApi.h` (interface 1.0) lets an add-on read files by suffix and import web pages. An importer
hands back plain structs (`Content`: paragraphs with runs, notes, pictures); the program makes the document
(`src/import/ImportedContent.*`), so styles, lists, notes and pictures follow the project. Both importers are
built into `build/tool-plugins/` and delivered as add-ons:

- `plugins/importers/word97` → add-on "Word 97–2003 Import" (`addons/import-word97`): a reader of the Word binary
  format (compound file, piece table, character / paragraph / style properties, lists, tables, fields, footnotes,
  endnotes, comments, inline pictures) without another program; LibreOffice (if installed) for Word 6.0 / 95.
  `build/plugins/importers/qb_doc_dump file.doc` prints what the reader makes of a file.
- `plugins/importers/web` → add-on "Web Page Import" (`addons/import-web`): fetching with Qt Network, a forgiving
  HTML parser (`Html.*`), the article finder (`Article.*`, the approach of "readability" tools) and the window.
  The add-on brings Qt's TLS plugin of Qt 6.12.0 (`addons/import-web/stage.sh`), because the AppImage has none;
  OpenSSL 3 comes from the system. `build/plugins/importers/qb_web_extract page.html [address]` prints what the
  article finder makes of a saved page.

To try them from the build folder: `QB_PLUGIN_PATH=build/tool-plugins ./build/quillbinder` (HTTPS then uses the
system's Qt TLS plugin).

## Projects in Scrivener 3 format

`src/import/ScrivenerFormat.*` reads and writes `.scriv` folders (File > Project Format, New Project > Format, opening a
`.scrivx`). The window works on a working copy (an ordinary project in `~/.local/share/Quillbinder/ScrivenerProjects/`)
whose `Project::scrivenerPath` names the `.scriv`; `Project::save` then calls `scrivener::sync`, which writes only what
changed (state in `settings/scrivener-sync.json`: hashes of both sides, so files Scrivener changed meanwhile are not
overwritten). What is written follows a real Scrivener 3.4 project (the sample of github.com/writerslogic/scrivener-mcp,
`tests/sample-project.scriv`); the header of `ScrivenerFormat.h` lists what is and is not written. `tests/scrivener_checks.cpp`
checks it (with `QB_SCRIV_SAMPLE` also against the real sample: an unchanged save leaves every file byte for byte as it was).

## Compile part 2: format editor, PDF, EPUB

`src/compile/PdfExporter.*` (own paginator on top of Qt's text layout: footnotes at the page bottom, headers and footers,
right-hand chapter pages, widows and orphans, a contents page with links, PDF bookmarks written into the file by an
incremental update), `src/compile/EpubExporter.*` (EPUB 3 with NCX, checked with epubcheck), `src/compile/Look.*` (the
look of a paragraph, shared by both and matching the DOCX writer), `src/ui/FormatEditor.cpp` (the nine pages of the
format editor). Style overrides, replacements, transformations and the contents settings are part of
`compile::Format` (`CompileFormat.h`); the compiler applies them (`Compiler::finish`). The exporter interface is 1.3
(`ExporterApi.h`): plugins built for 1.2 must be rebuilt (the bundled Markdown plugin is part of the update).

## Layout

| Folder | Contents |
|--------|----------|
| `src/core` | project model, binder, document store, snapshots, backups, search, templates |
| `src/text` | script formats and their elements (`Script`), Fountain and Final Draft files (`ScriptIO`), Markdown dialect and RTF reader/writer, display formats, formatting set by hand (`DirectFormat`), paragraph and character styles (`Styles`), pages of the page view (`PageLayout`, the one file that uses private Qt headers) |
| `src/compile` | compile model, formats, DOCX writer, PDF and EPUB writers (`PdfExporter`, `EpubExporter`, `Look`), Fountain and Final Draft exporters (`ScriptExporters`), plugin manager, exporter API |
| `src/import` | file import (txt, md, rtf, docx, odt, fountain, fdx, images, PDF), Scrivener import, projects in Scrivener 3 format (`ScrivenerFormat`), the importer interface for add-ons (`ImporterApi.h`) and what the program makes of their content (`ImportedContent`) |
| `src/ui` | all windows and dialogs; the program's side of the writing tools and the link between running instances (`ToolHost`, `MainWindowTools`); themes (`Theme`: colors and fonts per area, theme files; `ThemeDialogs`: preview, theme list); commands and shortcuts (`Commands`, `KeyboardPage`); menus, toolbars and layouts of the main window (`MainWindowBars`, `ToolbarConfig`, `Layouts`); copyholders (`Copyholder`); page size dialog (`PageDialogs`); scriptwriting (`ScriptEditing`: Tab, Enter and auto-completion in the text editor; `MainWindowScript`: commands; `ScriptDialogs`: Script Settings) |
| `src/tools` | `ToolApi.h`: the interface between the program and writing tools (plugins) |
| `src/media` | `MediaApi.h`: the interface between the program and a media engine (add-on "Media Playback") |
| `plugins/` | example C++ exporter (Markdown), JSON command plugins (LibreOffice, Pandoc), writing tools (`plugins/tools/`: name generator, scratch pad, text statistics), media engine (`plugins/media/`), importers (`plugins/importers/`: Word 97–2003, web pages) |
| `resources/` | app icon, bundled Breeze icon subset (LGPL-3.0-or-later, see `resources/icons/COPYING-BREEZE`) |
| `addons/` | sources of the add-ons (themes, project templates, compile format, scriptwriting pack, the three writing tools, media playback, the two importers) |
| `tools/` | `make-package.py` (builds .qbpkg), `qb-template.cpp` (project template from an outline; `--dump-script` prints a script format), `make-name-lists.py` (name lists from Faker), `build-ffmpeg.sh` and `ffmpeg-patches/` (the add-on's FFmpeg), `gen-resources.py`, `update-icon-index.sh` |

## Credits

- Main Developer: Claude.AI w. Opus 5.5 Extra

## License

Quillbinder is under the Apache License 2.0 (`LICENSE`), since 0.80.0: anyone may use, copy, change and
pass it on, with or without fee, keeping the license and the `NOTICE` file, which names the project and its
source. Versions up to 0.70.0 were published under the Zero-Clause BSD license (0BSD); copies of those
versions stay under it. The parts of other projects it uses, and their
licenses, are listed in `THIRD-PARTY.md`; their full license texts are in `resources/licenses` and in
the program under Help > Third-Party Licenses (made by `tools/collect-licenses.py`).

Scrivener and Scapple are names of products of Literature & Latte; they are used here only to say which file
formats Quillbinder reads and writes. Quillbinder contains no code, files or pictures of those programs.
