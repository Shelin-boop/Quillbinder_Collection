# Quillbinder: notes for Claude Code

Quillbinder is a writing program for Linux (binder, editor, corkboard, outliner,
inspector, compile), for writers who know Scrivener; it reads and writes Scrivener's
file formats but is an independent program (Apache License 2.0 since 0.80.0; 0BSD before). C++17 and Qt 6.12.0,
The Qt Company's own build (since 0.80.0; Ubuntu 24.04's Qt 6.4.2 before), built on Ubuntu 24.04. Never copy text, pictures or code of Scrivener, Scapple or
their manuals and tutorials into the source; describe functions in words of your own.

Before a job, read the project's two working documents, which are kept outside this repository:
`claude/READ-FIRST-working-rules.md` (how to work: questions, tests, costs) and `claude/quillbinder-current.md`
(versions, branches, pull requests, test baseline). Where they differ from this file, they win.
`docs/CHANGES.md` lists what each version brought; the details of a new version go there. `docs/manual.md` is
the user's manual (shown inside the program under Help).

**One repository** (since 0.92.0): the source, the work branches, the pull requests, the builds of the three systems,
the releases, the add-ons and the users' issues are all in `Shelin-boop/Quillbinder_Collection`. Work happens on
branches and pull requests; a version that reaches `main` is built and published by `.github/workflows/release.yml`
(see "Publishing" below), so `main` gets a merge only when the user says so.

## The person you work for

A writer, not a programmer, who wants results, not code. Explain in plain words what
changed and what was checked. Preferences:

- If something is not verified, say "I don't know" first; mark guesses as guesses.
  Say when information may be out of date.
- Dates yyyy-mm-dd, 24 h times, SI units.
- Ask when a request is unclear; do not guess at things that are expensive to redo.
- Longer texts for the user (reports, status) as an .odt file (A4); write "..." and not the
  single ellipsis character.
- American English, Chicago style for texts meant to be read.

## Hard rules

- **One line** (since 0.60.0 alpha, 2026-10-31): there is one source and one version number for everyone.
  Audio and video for everyone: the add-on "Media Playback" (free formats), the add-on "Extra
  Video and Audio Formats" (formats licensed under patents), and the package `media-all-formats.qbpkg` that installs
  both in one step (built by `packaging/linux/build-addons.sh`).
- **Runtime**: the AppImages since 0.80.0 share the runtime id `u2404-qt6.12-r1` (`APP_RUNTIME`, `QB_QT_VERSION`,
  `APP_RUNTIME_SINCE`, `APP_RUNTIME_DATE` in CMakeLists.txt; a new AppImage with other libraries needs a new id).
  The AppImages 0.2.0 to 0.70.0 had `u2404-qt6.4-r1` (Ubuntu's Qt 6.4.2). Program updates (.qbpkg) run on the
  AppImage's libraries: build on Ubuntu 24.04 with Qt 6.12.0 of `tools/get-qt.sh` (`/opt/qt/6.12.0/gcc_64`; CMake
  refuses another Qt version on Linux) and add **no new library dependencies**. Something that needs a library goes
  into an add-on (`addons/`, see the Media Playback add-on with its own FFmpeg). Add-ons with Linux libraries name
  the runtime in their manifest (`tools/make-package.py` adds it, and `minApp` = `APP_RUNTIME_SINCE`); the program
  refuses to install or load one of another runtime (`addons::fitsRuntime`).
- **Version**: `APP_VERSION` in CMakeLists.txt (numbers only: it is compared by the loader and
  the packages; a word such as "alpha" goes into `APP_VERSION_LABEL`, which is only shown);
  raise it for every delivery: a real new function raises the second number, fixes and tiny changes the third,
  which each system (Linux, Windows, Android) counts on its own; the details are the working rules' section 8.
- **Checks**: every change gets checks in a test program under `tests/` (scripted UI
  runs with screenshots: see `tests/fixes_tests.cpp` for a small, current example).
  A reported problem gets a check that fails without the fix. Which suites run is set by the
  working rules (default: the changed parts, their neighbors and a random 10 % sample, X11 only;
  the full suite only when the user asks); every suite that runs must end with "0 failure(s)".
- **Menus**: every menu command must work and be enabled only when it can act; the
  menu audit in `tests/m13_tests.cpp` walks all of them.
- Look at screenshots of what you changed (the tests save them); several bugs were
  found only that way.
- Keep `docs/manual.md` and `docs/CHANGES.md` in step with every change.

## Build

    tools/cloud-setup.sh                  # once on a fresh Ubuntu 24.04 (as root); fetches Qt 6.12.0 (tools/get-qt.sh)
    tools/get-ccache.sh                   # the compiled files of main (0.91.2); before the first configure
    tools/get-ffmpeg.sh; tools/get-whisper.sh   # ready-made packs, seconds (only when the media or speech suites run)
    cmake -S . -B build -G Ninja -DQB_TEST_SWITCHES=ON   # the switches the suites need (off by default since 0.70.0)
    cmake --build build -j"$(nproc)"

## Tests

    QB_SUITES="v040 m13" QB_SAMPLE=10 tools/run-all-tests.sh build/test-results-x11 x11   # the default run
    tools/run-all-tests.sh build/test-results-x11 x11       # the full suite, only when the user asks
    xvfb-run -a build/tests/qb_fixes_tests /tmp/shots       # one suite by hand

- Which suites a job runs is set by the working rules (`claude/READ-FIRST-working-rules.md`). `QB_SUITES` names suites
  as the runner prints them (`qb_tests`, `ui`, `m6` ... `m14`, `fixes`, `v017` ...); `QB_SAMPLE=10` adds whole suites
  drawn at random until about 10 % of the checks are covered and prints the seed (`QB_SEED=<n>` repeats it). A full
  X11 run writes `tests/check-counts.txt` anew: commit it. Read the runner's one line per suite and the FAIL lines,
  not whole logs.
- What each suite checks, its switches and its needs (FFmpeg, whisper.cpp, LibreOffice): `docs/dev-notes/test-suites.md`.
- The suites and scripted starts set `QB_NO_UPDATE_CHECK=1` (0.92.0): a program they start never looks for updates.
- Offscreen runs have no window manager: a widget has focus only after `activateWindow()`; modal dialogs are answered
  with `QTimer` + `activeModalWidget()`.
- Never run `pkill -f` / `pgrep -f` with a pattern that also appears in your own command line: it kills the shell
  that runs it.

## Delivery

1. Raise `APP_VERSION`, update `docs/manual.md` and `docs/CHANGES.md`.
2. Tests as the working rules say (see Tests above).
3. Update package: `packaging/linux/build-update.sh "What is new, line 1" "line 2"`
   gives `dist/quillbinder-<version>-update.qbpkg` (separate build dir `build-update`).
4. End-to-end with the real AppImage: `packaging/linux/e2e-appimage.sh <AppImage> <update.qbpkg>` (it also checks the first
   start with the welcome project and prints its first document; `-` instead of the package checks a new AppImage
   alone; give the work dir a short path: the instance socket's path may have at most 107 bytes). Only the current
   base AppImage, the update and the way back; older AppImages (`packaging/linux/e2e-old-version.sh`) only when asked.
   Add-ons are packed with `packaging/linux/build-addons.sh`; an update and add-ons go to the user as one bundle
   (`tools/make-package.py bundle`).
   The base AppImage is the last published one: download only that file (its place, name and SHA-256 are in
   `claude/quillbinder-current.md`).
5. Commit on a branch and open a pull request. The user installs an update with **File > Install Update or Add-on**.
   A session cannot create releases: files that have to be kept between sessions go on the branch `assets` with
   `tools/put-asset.sh <files>` (it fetches only the newest commit and file list with `--filter=blob:none`, adds the
   files and pushes one commit), and a test build for the writer goes on a test release (push the branch
   `test-release`, see "Downloads and the repository's layout").
6. A release, only when the user says so: merge the version's pull request. A version on `main` that has no release
   yet starts `.github/workflows/release.yml`, which builds, tests and publishes everything and creates the tag
   `v<version>-<label>`; the session follows it through GitHub's API (runs, jobs and job logs; not the caches): see
   `claude/quillbinder-current.md`, "Publishing".

## Downloads and the repository's layout (2026-11-12, the writer's choices 1, 2, 4, 5, 6)

- **Build scripts per system**: `packaging/linux/` (build-appimage, build-update, build-addons, build-full-appimage,
  e2e-appimage, e2e-old-version, desktop/AppStream/MIME files, `licenses/` of the AppImage runtime),
  `packaging/windows/` (win-ci, win-addons, win-check, win-check-layout, win-tests, win-dictionaries,
  collect-licenses-windows.py), `packaging/android/` (android-ci, android-emulator, android-qt, android-hunspell, package files); shared tools stay in `tools/`. Moved scripts find the
  repository two folders up (`$(dirname "$0")/../..`). `packaging/README.md` and the developer guide (section 8) map it.
- **Release text** (`tools/release-notes.py`): the files as before, sorted by system under a short headline each
  (`## Linux`, `## Windows`, `## Android`, `## Add-ons for every system`, `## Checking the files`); add-ons are sorted by
  their manifest (`platform`, `runtime`, `.so` files, or the packages inside a bundle). `--test` leaves out the source
  and the news. The source note says Apache License 2.0 (it said Zero-Clause BSD until 2026-11-12).
- **Download page** (`tools/download-page.py`, job `download-page` of `release.yml` after `publish`; GitHub Pages of
  this repository, https://shelin-boop.github.io/Quillbinder_Collection/): a button per system for the full version, the
  visitor's system first. Needs, once, Settings > Pages > Source "GitHub Actions" (the writer).
  The README's download part links it and is sorted by system too.
- **Test releases** (`tools/test-release.sh`, `.github/workflows/test-release.yml`): push the branch `test-release`
  with `test-release.txt` (`tag: test-...`, `title:`, `note:`, `assets: <file>`, `win-ci: <file> [> <new name>]`); the
  workflow copies the files into a pre-release with the sorted text, marked as a test build that is not published.
  This is where the writer downloads test builds; `assets` and `win-ci` are working storage. The pre-releases
  `win-ci`, `android-ci`, `android-st`, `qt-<version>-linux`, `deps-...` and `ccache-linux` are titled "not for
  downloading".
- **Branches**: work branches `linux/...`, `win/...`, `android/...`, `all/...`. The session cannot delete branches (the
  proxy refuses `git push --delete` and the API): the writer deletes merged ones on GitHub; the repository setting
  "Automatically delete head branches" does it at each merge.

## Where things are

- `src/core`: project, binder model, document store (Markdown or RTF files), snapshots,
  search, backups, package installer. `src/text`: formatting (`DirectFormat` = what
  was set by hand, `Styles`, `DisplayFormat`), Markdown and RTF reading/writing.
  `src/ui`: main window (split over `MainWindow*.cpp`), editor, inspector, dialogs.
  `src/compile`: compile, built-in PDF/EPUB/DOCX. `src/import`: file import, Scrivener,
  Compare and Transfer (`Compare.*` engine, `CompareFiles.*` Word/LibreOffice writers, `MiniXml.*`).
- `src/android`: Android's own code (the Android program only).
- `addons/`: add-on packages; `packaging/`: AppImage, update and add-on build scripts per system;
  `tools/`: helper scripts; `resources/`: icons (`qbicons` themes), templates, manual.

## Developer notes by topic (`docs/dev-notes/`)

The notes each version added (how a part works, why, its pitfalls) are kept by topic, newest section first. Search
them with grep (`grep -n '<word>' docs/dev-notes/*.md`) and read only the section a job needs; this file stays a
short index (under about 20 KB), so that it costs little when it is loaded.

- `test-suites.md`: what each test suite checks, its switches and what it needs.
- `ui.md`: the marks of a binder row, the fixed mind map and the shown lock (0.92.0), toolbars and their places (0.91.x), theme buttons, menus filled on opening, undo names, the format bar,
  the look per project, fonts with several designs, zoom parts, objects.
- `saving-and-safety.md`: 0.92.0 (the content decides, the project ID in backup names, checked copies, failures that are told), backups and sync programs, the save journal, safe copies, signatures, the audits' fixes
  (0.64.1 to 0.81.1), the system test's fixes, settings per project, speed with large projects, crash reports.
- `scrivener-format.md`: reading and writing .scriv (styles, inline notes, web pages, keywords, metadata, collections,
  the rest of a project).
- `runtime-and-addons.md`: the check for updates and the translator add-on (0.92.0), Qt 6.12 and FFmpeg 9.0.2 (the runtime), tool interface 1.1 and Speech Transcription,
  release files and third-party sources.
- `windows.md`: Windows 0.92.0 (the translator's build, the check for updates with the setup program's update run, the
  update file's Windows block, what a Windows job may add to shared files), the Windows port (W1 to W4c), what 0.90.4
  brought, the public Windows files, Windows 0.91.3 (its own release, `[build-only]` / `[suites: ...]` in a commit
  message, the Windows dependency packs, the run on main).
- `android.md`: the Android trial builds, the public APK, add-ons on Android.

## Publishing

- `README.md` starts with direct download links of the current version (`releases/download/v<version>-<label>/...`
  for the AppImage and `media-all-formats.qbpkg`): change them with every new version (`qb_v060_tests` checks them).
  Releases are full releases marked "Latest" (`tools/publish-release.sh`).
- **A version on `main` releases by itself**: `release.yml` runs on a push to `main` that changes `CMakeLists.txt` or
  the workflow, and publishes when that version has no release yet. Until a version is finished, its work stays on
  branches; a commit that must reach `main` without a release carries `[skip ci]`.
- **Full version** (0.60): `packaging/linux/build-full-appimage.sh [AppImage build dir] [add-on packages dir]` makes
  `<Name>-<version>-full-<label>-x86_64.AppImage`: the AppImage extracted, every `addons/*` package in
  `usr/share/quillbinder/full-addons/`, and a start script (`AppRun`) that runs `quillbinder --install` for each at the
  first start of that version (marker `~/.local/share/Quillbinder/full-addons-installed-<version>`), then the program.
  No program change; the release workflow builds and checks it, and builds a release again when it lacks one.
- **The update file** (0.92.0): `tools/update-feed.py` writes `updates-1.json` for the program's check for updates, the
  job `sign-checksums` signs it and `download-page` publishes it; every workflow that publishes the page carries it
  along (`tools/keep-update-feed.sh`); a security update is marked in `packaging/update-feed/security.json` in the
  commit that raises the version. The address is built into the program and must never change.
- **Translation and Proofreading** (0.92.0) is one of the project's own add-ons (`addons/translation-proofreading`, a
  project of its own in `source/`); its engine libraries come from `tools/get-translator-deps.sh`, its models are on
  the releases `models-1` and `models-2`.
- **The security watch** (2026-11-21): `.github/workflows/security-watch.yml` checks the parts in
  `tools/security-watch.json` daily and opens an issue (label `security`) only for High or Critical findings; a
  new version of a part goes into that list in the same commit (suite `secwatch`). `docs/dev-notes/runtime-and-addons.md`.
- **Official add-ons** (0.91.3): `official-addons/<id>.json`, workflow `addon-release.yml`, `tools/addon-entry.py`;
  `docs/official-addons.md` (the way to the list, the add-on key) and `docs/dev-notes/saving-and-safety.md` ("Add-on keys").
- Every picture, sound, video, font and document file says where it comes from (a README, LICENSE or COPYING file in
  its folder or above), and every font has its license beside it. FFmpeg is built without GPL or non-free parts and is
  named with its LGPL in About and the media add-ons.
- Texts and pictures of Scrivener, Scapple and their manuals, templates and tutorials are never copied, not even as
  a list of fields or a layout taken from a screenshot; describe and design in words and choices of your own. The
  built-in character and place sheets were written anew in 0.60 for this reason.

## Welcome project, licenses, names (0.40)

- The welcome project is an outline (`resources/welcome/outline.json` with Markdown files in `docs/`), made into a
  project by `src/core/OutlineProject.*` (also used by `tools/qb-template.cpp`; the outline can carry a mind map per
  item and objects). `MainWindow::openWelcomeOnFirstStart` (called by `main.cpp` when no project file was given)
  opens it once (`AppSettings::welcomeShown`); **Help > Welcome Project** opens it again or makes a fresh copy.
  Keep its texts short; `tests/v040_tests.cpp` checks every menu place they name. After changing files in
  `resources/welcome` or `resources/licenses`, run `python3 tools/gen-resources.py`.
- `docs/quick-start.md` is **Help > Quick Start Guide**; keep it in step with the manual.
- **Help > Third-Party Licenses** (`src/ui/LicensesDialog.*`) shows `resources/licenses` (made by
  `tools/collect-licenses.py` from the Ubuntu packages of the AppImage's libraries; `tools/licenses-extra.json` for
  the parts that are no Ubuntu package). `packaging/linux/build-appimage.sh` fails when a library has no license text.
- The license is the Apache License 2.0 since 0.80.0 (`LICENSE` unchanged from Apache, `NOTICE` with "Copyright 2026
  The Quillbinder project" and the link to the public source; no person named; 0BSD up to 0.70.0). Credits:
  "Main Developer: Claude.AI w. Opus 5.5 Extra" only.
- The view of several documents as one text is called **Combined Text** (`ViewMode::Combined`); the settings keys
  of older versions (`scriveningsTitles`, font `scriveningsTitle`) are still read. Do not use the word
  "Scrivenings" in the program or its texts; "Scrivener" only where the file format or the import is meant.
