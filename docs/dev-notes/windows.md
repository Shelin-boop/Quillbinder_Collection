# Windows

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## Windows 0.95.0 (2026-11-26): the dictionary add-ons, the suites of 0.95.0, Translation Helper 1.5

- **The nine dictionary add-ons** (`addons/dictionary-<code>`, Linux 0.95.0) are built by `packaging/windows/win-addons.sh`
  too, without a platform (the same content as Linux's; the release names them `<id>-windows.qbpkg`): each add-on's `stage.sh` runs
  `tools/stage-dictionary.sh`, which fetches LibreOffice's files from raw.githubusercontent.com (GitHub's runner reaches
  it) and checks them against `sources.txt`. The Windows checkout has CRLF line ends: that script reads `sources.txt`
  through `tr -d '\r'` (on Linux nothing changes). The spell checker finds them through `addons::contentDirs` on
  Windows as on Linux (`SpellChecker.cpp` `dictionaryDirs()`); English and German stay in the program folder
  (`win-dictionaries.sh`). They go into the full version and the release; **the update bundle leaves them out**
  (`win-ci.sh` `make_bundle`; `win-check-layout.sh` counts the bundle's add-ons without them), so that installing the
  bundle does not put nine languages into every program. The update file's Windows block names them (an add-on without
  platform or runtime, `tools/update-feed.py`).
- **The suites of Linux 0.95.0** are counted in `win-tests.sh` from their first run: v0950a to v0950d, v0950f, v0950g
  (v0950b leaves out the AppImage's move and menu entry by `Q_OS_LINUX`; v0950g stages, packs and installs the nine
  dictionaries with `bash` and `python3` of the runner).
- **Translation Helper 1.5**: `win-addons.sh` builds it from `addons/translation-proofreading/source` of the commit, as
  before; the official entry's Windows block names the 1.5 source (99439897), checked by a dry run of
  `addon-release-windows.yml` on the work branch.
- **Keyboard sets (to-do item 111)**: Windows' own layouts (kbdlayout.info, read from its tables, not tried on a PC)
  have AltGr letters on Q, W, E, C (Brazilian ABNT2) and A, E, I, Q, S, T (Turkish Q); of the program's Ctrl+Alt+letter
  commands only C (Brazilian) and I (Turkish, Show Inspector) meet them. Since 0.90.4 the AltGr key itself is
  `Qt::GroupSwitchModifier` (`platform::qpaWithAltGr`), so it types and runs no command; a clash is left only when the
  letter is typed with the left Ctrl and Alt. No change. On a Cyrillic layout Qt 6.12.1 takes a shortcut's letter from
  the key's virtual-key code (Windows, `qwindowskeymapper.cpp`, the fall-back key) or from the Latin group (Linux,
  `QXkbCommon::lookupLatinKeysym`): the "us" set fits Russian.

## Windows 0.93.0 (2026-11-24): Qt 6.12.1, zlib 1.4, FFmpeg 9.1 of the project, the mind map's shapes

- **Qt 6.12.1** through `packaging/windows/win-qt.sh` (one place for `windows.yml` and both Qt steps of
  `release-windows.yml`): aqtinstall 3.3.0 with its patch for the folders per compiler, then Qt PDF. Qt 6.12.1 has no
  Qt PDF of its own (a Qt Extensions module since Qt 6.12, listed under 6.12.0 only): the script tries Qt PDF with the
  version, and else installs it from the newest earlier patch release with `--noarchives` (as `tools/get-qt.sh` on
  Linux). The runtime id is `win64-qt6.12-msvc-r2`: a Windows program update carries only `quillbinder.exe`, so a
  0.92.0 folder with Qt 6.12.0's DLLs is offered the setup program (`UpdateCheck.cpp` compares the runtime), and an r1
  program refuses an r2 program update.
- **vcpkg pinned** (`packaging/windows/win-vcpkg.sh`): the runner's vcpkg is moved to one commit (`COMMIT`), its zlib
  and Hunspell ports are checked against `ZLIB` and `HUNSPELL`, and `tool` fetches the vcpkg program that commit names
  before `vcpkg install`. The Hunspell pack's recipe of that commit is the one of 2026-11-21 (`d70c6b0d...`, zlib 1.4).
  The security watch lists "zlib (vcpkg, Windows)" pinned to `ZLIB` there.
- **FFmpeg 9.1 of the project** (`packaging/windows/win-ffmpeg.sh`): Qt 6.12.1 for Windows brings FFmpeg 9.0.2 (its
  `av_version_info`, run 45342399136), which the High findings of issue #7 name ("through 9.0.2"); the writer chose to
  build it. The same archive and SHA-256 as `tools/get-ffmpeg.sh`; Qt's own configuration (its
  `avcodec_configuration()`: no programs, devices or filters, no Vulkan, `--disable-decoder=truemotion1`, zlib, shared,
  `--toolchain=msvc`) with vcpkg's zlib 1.4 copied as `z.lib` (configure asks for `-lz`); configure and make in the
  runner's MSYS2 (`C:\msys64`), about 10 minutes. Pitfalls of the four runs it took: MSYS2's login shell keeps neither
  the PATH it was started with nor MSVC's INCLUDE and LIB; an empty argument is dropped on the way into it; Git Bash's
  `$LIB` was empty although the program's build links in the same step. So the script runs `vcvars64.bat` in cmd
  (`VSDIR`), takes INCLUDE, LIB and MSVC's folders from the environment it prints (`set`), and hands them over in a
  file (`msvc-env.sh`) that the MSYS2 script reads; MSVC's folders go before MSYS2's own `link`. A change of
  `win-ffmpeg.sh`, even of a comment, changes the pack's recipe (its header still says 20 to 30 minutes and that MSYS2
  inherits the environment: left as it is so that the pack of run 45390886814 stays pinned). FFmpeg 9.1 keeps 9.0's major versions, so Qt's `ffmpegmediaplugin.dll` loads these DLLs;
  `win-addons.sh` checks that the names are the ones in Qt's `bin`, and a release (`QB_WIN_STAGE=release`) refuses to
  make the add-on with Qt's FFmpeg. A pack (`win-deps-pack.sh ffmpeg`: source, script, zlib, MSVC) as for whisper.cpp.
  zlib 1.4's own CMake build names its static library `libzs.lib` (the zlib.lib of older ports is gone).
- **The pins of the packs** are in `packaging/windows/win-packs.txt` (before: `QB_WIN_PACKS` in `windows.yml`), read by
  `win-ci.sh` and the Hunspell step of `windows.yml`, so that `release-windows.yml` takes the same packs (its build
  steps have `GH_TOKEN` for the download). A change of `win-deps-pack.sh` changes the Hunspell recipe (the script is
  part of it): its next run builds the pack anew and prints the new pin.
- **Item 19, the mind map's background shapes**: the MSVC 14.52 build (19.52.37029) read a shape's width and height in
  `Board::fromXml` as garbage, also with Qt 6.12.1 (the probe of v017: the place right, the same lines compiled in the
  test file right, with and without the optimizer, a shape appended in the test right). `Board::fromXml` is compiled
  without the optimizer under `_MSC_VER` (`#pragma optimize("", off)`): the probe then read 240.5 x 130.25, and the
  counted check in `mmoverview` (`#ifdef Q_OS_WIN`: read, write, read again) passes. The cause inside the optimized
  function is not known; GCC and the MSVC 14.51 build were right. Take the pragma out only with a probe on a newer MSVC.
- **Item 33**: the installed version's folder moves aside with `moveIntoPlace`'s tries (on Linux the one rename);
  examined in v0920 (an INFO line, run 45350224680): a folder whose add-on DLL is loaded can be moved aside on Windows.
- **Tests**: m10 and the suites of Linux 0.93.0 are counted (`win-tests.sh`); v017 stays "explore" for its one old
  failure (another document's board). The new suites' Linux assumptions, each under `Q_OS_WIN` or with no effect on
  Linux: v0930f's heap measure is glibc's (`__GLIBC__`); v0930's notice is compared with the system's separators and
  its backup folder is saved to the settings file (opening a project reads the settings again, and Windows, always
  portable, has no standard backup folder); v0930e sets a backup folder on Windows instead of leaving the portable
  mode; saveas uses Qt's own file dialog on Windows (`AA_DontUseNativeDialogs`: the system's dialog is no widget the
  test can fill in). `win-ci.sh`'s result line names explore suites with failures (item 32).

## Windows 0.92.0 (2026-11-20): pulled to Linux 0.92.0 in the one repository, the translator inside, the check for updates

- **What it brings**: the source of Linux 0.92.0, built, tested and released from `Shelin-boop/Quillbinder_Collection` for
  the first time (version 0.92.0, release `v0.92.0-windows-experimental`). Windows' own parts: the translator as one of
  the project's add-ons, the check for updates with Windows' files, and what the workflows needed in this repository.
- **Shared files** (the project's rule since 2026-11-20): a Windows or Android job may add parts for its own system
  to files Linux shares when Linux shows and does nothing new; the job proves it with the Linux suites around those
  files. Here: five German texts (`translations/quillbinder_de.ts`; the compiled file differs by exactly these
  entries), a Windows branch in `tools/update-feed.py`, `Offer::Kind::NewSetup` and the setup keys in
  `core/UpdateCheck.*`, the hand-over in `ui/UpdateChecker.*`, `MainWindow::closeThenExit`. The code that only
  Windows can run sits in `#ifdef Q_OS_WIN`; the rest compiles on Linux too, where no update file names a setup program.
- **The translator** (`addons/translation-proofreading`, version 1.2 as on Linux): `win-ci.sh` builds the engine
  libraries (oneDNN, CTranslate2, SentencePiece) with the add-on's `packaging/build-deps-windows.sh` into
  `.translator-deps` when that folder is empty (18 min on the build machine; the workflows keep it in a cache named
  with the script's hash, the same key in `windows.yml` and `release-windows.yml`, so the run on `main` after a merge
  fills it for the release). `win-addons.sh` builds the add-on's own project against them (a release build, no test
  switches), checks its exports and packs it with the add-on's `packaging/make-addon-windows.sh`:
  `translation-proofreading-windows.qbpkg`, about 8 MiB. Without the libraries it is left out with a line in the
  log; a build that fails fails the add-on step after everything else was made; a release build
  (`QB_WIN_STAGE=release`) is incomplete without it. `addons/translation-proofreading/.gitattributes` keeps
  `manifest.json` at LF: it is compared byte by byte with `source/packaging/manifest.json` (the scripts, v0920), and
  Windows checks text files out with CRLF (the first run left the add-on out for that).
- **The check for updates on Windows**: the usual update is the program update, installed inside the program as on
  Linux. A version on another runtime is offered as its setup program (`Offer::Kind::NewSetup`; the update file's
  Windows block names `setup` and `setupFull` where Linux names `appimage` and `appimageFull`; the full version is
  known by `full-addons\` beside the program). `UpdateChecker` downloads it into `<data>/update-setup` (inside the
  program folder: the portable program keeps nothing elsewhere, and the setup leaves `data` alone), checks size and
  SHA-256 against the signed list, and offers **Close and Update Now**: `MainWindow::closeThenExit` closes the
  project as a restart does, `startSetup` starts the setup detached with `setupArguments` (`/SILENT /NOCANCEL
  /QBUPDATE=1 /DIR="<folder>"`, handed over with `QProcess::setNativeArguments` so that the quotes stay where Inno
  Setup wants them), and the program ends. When the start fails (Windows blocks unsigned programs where Smart App
  Control is on), the project is opened again and a message names the file. The folder is emptied at the next start
  (`UpdateChecker`'s constructor) and before a new download. Test switch: `QB_TEST_SETUP_LOG` writes the command into
  a file instead of running it.
- **The setup's update run** (`quillbinder-setup.iss`): with `/QBUPDATE=1`, `PrepareToInstall` waits up to two minutes
  until the program in the folder has ended (`ProgramEnded`: the file of a running program cannot be opened for
  writing; `SaveStringToFile(exe, '', True)` appends nothing and tells), then unpacks as always and starts the new
  version (a `[Run]` entry with `Check: UpdateRun`, without `postinstall`). `win-setup.sh` tries both with the real
  setup program: over a folder whose program is not running, and while it runs (the setup waits, the check ends the
  program, the setup goes on and starts it).
- **The update file**: `tools/update-feed.py --system windows-x64 --runtime <win64-...> --label experimental` writes
  Windows' block from the release's files and takes the other systems' blocks from the published file;
  `release-windows.yml` does this in `sign-checksums`, signs the file with the release key, checks that Windows'
  block is complete and Linux's unchanged, and `download-page` publishes it with the page, unless the published file
  is newer than this run's (a job started again after a later release). For this the Windows release carries the
  program update alone (`quillbinder-<v>-experimental-windows-program.qbpkg`) and every add-on as a package of its
  own (all named `...-windows.qbpkg`). **The program update's name must not end in `-update.qbpkg`**: the download
  page takes the first such Windows file for its "update" link, which is the bundle.
- **The workflows in this repository**: the test fonts come from a job `test-fonts` in `windows.yml` too (no archive
  on a branch any more); the packs were made anew by the first run and pinned (`QB_WIN_PACKS`: whisper.cpp v1.9.5,
  Hunspell 1.7.5 with zlib 1.3.2, MSVC 14.51), and their public pre-releases name content, license and source
  (`win-deps-pack.sh`, `about`); the Windows jobs' time limit is 180 minutes. Times on the build machine: the first
  run with empty caches 47 min (engine libraries 18, build 6, checks and 39 suites 20); a `[suites: ...]` run with
  warm caches and packs 6 min.
- **The move of a new version into its place** (`addons::moveIntoPlace`, used by the installer for the folder
  `.new-<name>` -> its place): Windows refuses to rename a folder while a file in it is open. A program update is
  started once from that folder for its self-test (`launcher::selfTest`) right before the move, and a virus scanner
  may be reading the new files; with one try, the first release run of 0.92.0 stopped in its program checks
  ("Cannot create ...\data\core\0.92.1": `update-install` and the three checks after it; the four work runs
  before had passed). On Windows the move is now tried again every 250 ms for up to five seconds; on the other
  systems it is one rename as before, and a move the tests make fail (`QB_TEST_FAIL_RENAME`) is made once. The move
  of an installed version aside (`.old-<process>`) is still one try.
- **Tests**: `qb_v0920_tests` is counted (110 checks on Windows: Linux's 105 without the two of the swapped AppImage,
  plus seven of Windows' own: the hand-over, a wrong setup file, the emptied folder, the tool for Windows, the offer
  as a setup program, the command line, and a folder that is held for a moment and then moved into place). What the first run showed, all in the tests and corrected inside
  `#ifdef Q_OS_WIN`:
  - `QTest::mouseMove` also sets the real pointer; on the build machine's desktop the binder then gets a leave event
    and no row counts as under the mouse. The move is sent to the viewport once more and looked at at once.
  - Windows needs about two seconds to refuse a connection: a look at an address that cannot be reached takes about
    8 s with its second try (Linux: under a second). Waits and "the first note is still there" follow that.
  - `qputenv` takes bytes in the system's code page while the program reads the variable as text: a German text
    goes in with `toLocal8Bit()`.
  - `stage.sh` is the Linux staging of the translator; started as `bash` from a test program it ended with exit
    code 1 (why was not looked for). Windows checks its own scripts instead.
  - `qb_v0913_tests` leaves out its check of an Android package (its stand-in for readelf is a shell script).
  - `qb_m10_tests` (still "explore") takes the Windows packages of the writing tools (`QB_TEST_ADDONS_DIR`,
    `tool-*-windows.qbpkg`); before, it looked for the Linux packages, loaded no tool and ended with a crash of its
    own on a null action. Windows add-ons with program code name a platform and no runtime id (the Linux ones name
    their runtime): the check for updates and the installer therefore do not hold a Windows add-on back when the
    Windows runtime changes one day. Not changed here.
  - `qb_v017_tests` (still "explore"): its one failing check says now which part fails and what the panel shows.
- **Pictures**: the pin and the padlock of `ui/RowMarks.*` look right on Windows at 100 % (the suite's pictures in
  `test-output.zip`, `shots-v0920`); other scale factors were not looked at on the build machine.

## Windows with what Linux 0.90.3 has, the setup programs (0.90.4 beta, 2026-11-13)

The writer's requirements: project document `claude/quillbinder-windows-requirements-0.90.4.md` (W-01 to W-12, S-01 to
S-09, with the result of each). Linux 0.90.4 is a program update without a change in the writing work; Windows 0.90.4 is
an experimental build from the same source (test switches on, packages unsigned).

- **A folder for one run** (`portable::sessionTemp()`, `core/Portable.*`): `<temp>/quillbinder/run-<pid>`, made at the
  first call, removed by a post routine when the program ends; folders of runs that are gone
  (`platform::processAlive`) are removed at the first call of a later run. The portable program (Windows) puts there
  what it needs only for the moment: the unpacked .mht of a web page shown (`MediaView::showWebPage`; Linux: the
  system's cache folder) and the copy for the browser (`openSavedPage`; Linux: Downloads, for browsers in a sandbox).
  New code that needs a place for short-lived files on Windows takes this folder, never `CacheLocation`, AppData or a
  folder of the writer: `win-check.sh`'s `portable-nothing-outside` also looks at Downloads and at `run-*` folders left
  in `%TEMP%`.
- **AltGr** (`platform::qpaWithAltGr`, called in `main.cpp` before the application object, Windows only): Windows
  reports AltGr as Ctrl+Alt, and Qt then ran the command on Ctrl+Alt+<key> instead of typing (German keyboard:
  AltGr+7, 8, 9, 0, < for `{ [ ] } |`; about 30 commands use Ctrl+Alt). `QT_QPA_PLATFORM=windows:altgr` makes Qt's
  Windows platform report AltGr as `Qt::GroupSwitchModifier` (the switch is in Qt 6.12's `qwindowsintegration.cpp`). A
  platform named on the command line (`-platform offscreen`, the self-test) or another platform in the variable is
  left alone. A test program that checks keys on Windows sets the variable the same way before its application object
  (`tests/v0904_tests.cpp`, which presses real keys with `SendInput` on the German layout and leaves the check out with
  a note when the test window does not get the keyboard).
- **Character sets of saved web pages** (`core/WebArchive.cpp`, Windows only): when `QStringDecoder` does not know the
  page's character set (Qt for Windows has no ICU), `windowsCodePage` + `MultiByteToWideChar` convert it
  (windows-125x, ISO 8859-x, a few more); Latin-1 or UTF-8 only as the last resort.
- **Unsigned updates**: `launcher::coreOrigin` refuses an unsigned program update in a build without test switches on
  Windows too (it was left out there). The experimental Windows builds are test builds; a Windows release needs signed
  packages.
- **Setup programs** (step W4d): `packaging/windows/quillbinder-setup.iss` (Inno Setup: `PrivilegesRequired=lowest`,
  `Uninstallable=no`, `CreateUninstallRegKey=no`, the folder page with `{%USERPROFILE}\Quillbinder`, English and
  German, `data\` never part of it, `PrepareToInstall` refuses a folder that cannot be written, a question before
  unpacking into a folder that is not empty and holds no Quillbinder) and `packaging/windows/win-setup.sh` (finds
  `ISCC.exe` or installs Inno Setup with Chocolatey; gives the script a byte order mark, without which the compiler
  reads the German texts in the machine's code page; `MSYS_NO_PATHCONV=1` for the compiler's and the setup's `/`
  switches; then the checks: built, unpacks into a folder with letters outside ASCII, the same files as the zip's
  folder by SHA-256, no uninstaller, the unpacked program's self-test, `data` kept by a second run, the full version
  with its add-ons, a folder without write rights refused, nothing new in the registry, the Start menus or on the
  desktops, the manifest "asInvoker"). `win-ci.sh` runs it after the full zip; its result counts for "result: ok".
  Files: `Quillbinder-<v>-windows-x64-setup.exe`, `Quillbinder-<v>-full-windows-x64-setup.exe`.
  `tools/download-page.py` puts the full setup on the Windows button when a release has it (the zips move to "Other
  downloads"), `tools/release-notes.py` explains and lists the setups.
- **Tests**: `qb_v0904_tests` (Linux with `QB_TEST_PORTABLE`, and Windows; 29 checks on Linux). `qb_v0903_tests` has a
  folder watcher for Windows (`ReadDirectoryChangesW`; Windows tells a file moved in from another folder as "create",
  perhaps with "written" after it: `arrivedOnce`); its full-drive checks need the Linux fault library and stay Linux
  only (`QB_FAULT_SHIM` is not defined on Windows). `win-tests.sh` counts v0812, v0813, v0900, v0901, v0902, v0903 and
  v0904 besides the 17 suites of W4c.
- **The runs on the Windows machine**: run 1 (f1934fe, 66 minutes: a new branch starts without the caches of other
  branches) built without one error of MSVC and passed the program, layout and setup checks; of the tests it failed
  v0901 (the four This Computer controls without `optstore::mark`: fixed in `OptionsDialog::markStorage`; and the
  screen-fit checks, which took 34 px for a frame where Windows' is 31 px: the test now takes the window's real frame)
  and v0902 (the test held `settings/look.json` open while the look was saved again: Windows refuses to replace an
  open file). Of the 27 older suites tried, nine passed unchanged and are counted since (m7, m8, m9, v018, v0212, v0402,
  v0403, v0605, v0642); the others stay Linux only, each with its reason in `win-tests.sh`. Run 2 (b82f7a6, 21 minutes):
  result ok, 33 suites, 1,972 checks, 0 failures; program checks 26, setup checks 11, layout checks 8. v0904 on the
  machine, with real key presses on the German layout: with the switch AltGr+7 types "{"; in a child run without it
  nothing is typed and the command on Ctrl+Alt+7 runs (Qt 6.12.0). Sitka's designs are families of their own there, each
  with its bold and italic: `fontfaces::separate()` is not needed on Windows.
- **Windows syntax check before a push** (saves runs on the Windows machine): MinGW-w64 (`g++-mingw-w64-x86-64-posix`)
  with `-fsyntax-only` and the Linux build's flags from `ninja -C build -t compdb`, Qt's `qwindowdefs_win.h` (into a
  folder `QtGui/`) and `qt_windows.h` from qtbase's source at the Qt version, zlib's and Hunspell's headers. In this
  job it found a variable that shadowed a parameter in Windows-only code before the first run.

## Windows build (port, W1, 2026-11-07)

Plan and steps: project doc `claude/quillbinder-porting-plan.md`. `.github/workflows/windows.yml` (on push to
`win/**`) runs `packaging/windows/win-ci.sh` on `windows-2025`: MSVC from Visual Studio 2026 (toolset 14.51),
Qt 6.12.0 since W4a (2026-11-11; 6.8.3 LTS in W1 to W3) with Qt Multimedia, Image Formats and PDF, installed by the
workflow itself with aqtinstall 3.3.0 told the folder of Qt 6.11 and later (Qt's repository keeps Windows packages in
`windows_x86/desktop/qt6_6120/qt6_6120_msvc2022_64/`; aqtinstall looks in `qt6_6120/qt6_6120/`; the step patches its
`extension_for_arch` and caches `C:\Qt\6.12.0`; `jurplel/install-qt-action` could not do it), Hunspell and zlib from vcpkg
(`x64-windows-static-md`, no extra DLLs), sccache; writing tools, importers and media are built since W2, the example plugin (Markdown exporter)
and the test suites named in `packaging/windows/win-tests.sh` since W3. Result: the pre-release `win-ci`, made anew each run (notes `sha=... status=running`,
then `status=done result=ok|build failed|...`; files `summary.txt`, `errors.txt` (all compile errors, `ninja -k 0`),
`build.log`, `selftest.txt`, `Quillbinder-<version>-windows-x64.zip`; since W2 also `checks.log`, the reports of the
crash checks, `check-files.zip`, the symbols zip, `quillbinder-<version>-windows-update.qbpkg` and the add-ons
`<id>-windows.qbpkg`). Read it with
`gh api repos/Shelin-boop/Quillbinder_Collection/releases/tags/win-ci` and download a file by its asset id
(`.../releases/assets/<id> -H 'Accept: application/octet-stream'`); since 2026-11 the session can also read the runs, jobs and
job logs through GitHub's API (`gh api repos/Shelin-boop/Quillbinder_Collection/actions/...`), not the caches.
Times (measured 2026-11-14): with warm caches 21 to 27 minutes (build and pack 2.5, checks 3, the 35 suites 16); the
first run of a new branch 48 to 82 minutes (GitHub's caches belong to a branch: Qt 1.7, vcpkg 11, whisper.cpp 5, a
cold compile 18). The plan to shorten both: project document `claude/quillbinder-build-optimization.md`. A push whose message
contains `[skip ci]` starts no run (the results on `win-ci` stay).

- Linux-only code sits behind `Q_OS_LINUX` (or `Q_OS_UNIX`) with the Linux path unchanged: `CrashGuard.cpp` whole
  (Windows: `CrashGuardWin.cpp` since W2; other systems `CrashGuardOther.cpp`), `Launcher.cpp` (exec), `AddonDialogs.cpp`
  (restart: `QProcess::startDetached`), `ToolHost.cpp`, `CrashDialogs.cpp` (glibc line), `FontFaces.cpp`
  (`separate()` returns 0). CMake: `/utf-8 /bigobj`, `_USE_MATH_DEFINES`, `NOMINMAX` with MSVC; Hunspell found
  without pkg-config; `WIN32_EXECUTABLE`; option `QB_BUILD_EXAMPLE_PLUGIN`.
- Qt 6.8 refuses `findChild<T *>` for a class without `Q_OBJECT` at compile time (Qt 6.4 accepts it): look the
  child up without `findChild` (as `Zoom.cpp`'s `handlerOf()` does), or give the class `Q_OBJECT`.
- Pitfall: exporting MSVC's environment (vcvars) to later steps through `GITHUB_ENV` broke Git Bash there (exit code
  127); vcvars runs inside the build step only (`shell: cmd`). GitHub refuses empty release files: leave them out.

**W4a (2026-11-11): Qt 6.12.0 on Windows.** Runtime id `win64-qt6.12-msvc-r1` (CMake now refuses another Qt version on
Windows too); `resources/licenses-windows` for Qt 6.12.0 (`packaging/windows/collect-licenses-windows.py` with the checkouts of
`tools/get-qt-licenses.sh`: emoji-segmenter, tl::expected, libwebp new, IAccessible2 gone); `packaging/windows/win-ci.sh` keeps the
image plugins of the AppImage (gif, ico, jpeg, pdf, svg, webp); `packaging/windows/win-addons.sh` writes `media-dependents.txt` (what
Qt's media plugins need); `packaging/windows/win-tests.sh` runs suites written for Linux as `EXPLORE` (reported, not counted);
`SpeechTool.cpp` loads its engine with `LoadLibraryExW` on Windows (`openLibrary`, `librarySymbol`, `libraryError`; the
engine itself, whisper.cpp, is W4c). The offscreen platform on Windows (the update's self-test, `launcher::selfTest`):
windeployqt leaves `qoffscreen.dll` out (jurplel's action had hidden it with `QT_PLUGIN_PATH`), so `win-ci.sh` copies it
into `platforms\`; it reads fonts with FreeType only from `QT_QPA_FONTDIR` (else "fonts" beside Qt's libraries), so
`main.cpp` points that at `%WINDIR%\Fonts` for an offscreen start (without it: "selftest failed: no fonts", every
update refused). In Git Bash, MSYS turns arguments such as `/nologo` into paths: give MSVC tools `-nologo`. Explore
results of the first good run (6e6778e): fuzz, v0811, v064, v0611, v0603 pass; v0650 (9 failures: safe copy, read size,
command exporters, the scratch pad's add-on, Duplicate), v0604 (the README's Wayland line) and v0700 (over 300 s) not.

**W4b (2026-11-12): the portable program.** `qb::dataDir()` (`core/Paths.h`) on Windows = `launcher::appDir()` +
`/data` (test builds: `QB_DATA_DIR`, and Qt's test mode keeps the old place); `portable::useProgramFolder()` (first in
`main()`) puts the settings INI into `data\settings`; `portable::folderProblem()` refuses a program folder that cannot
be written (exit 2, message); fonts in `data\fonts`, loaded by `fontinstall::reload()` at each start, no registry;
the plugins' Emergency folder is `host->dataDir(id)/../../Emergency` (Scratch Pad 1.4, Speech Transcription 1.7).
`core/Portable.*` (`active()`: Windows, or `QB_TEST_PORTABLE=1` on Linux for the checks; `programFolder()`, test
`QB_TEST_PORTABLE_FOLDER`; the place of notes and transcriptions: `app/notesPlace`, `app/notesOwn`, the tools' keys
`tools/<id>/folder`, `app/notesGiven/<id>` so `followProgramFolder()` moves only what it gave; `syncService`,
`longPath`). `ui/WinShell.*` (Start menu and desktop `.lnk` through IShellLinkW, file types under
`HKCU\Software\Classes`: `Quillbinder.Project` for .qbx, `Quillbinder.ScrivenerProject` only in .scrivx's
OpenWithProgids; tests `QB_TEST_SHELL_DIR`, `QB_TEST_SHELL_CLASSES`). `ui/PortableSetup.*` (the shared widget;
`shownPath` puts zero-width spaces after separators so long paths wrap), `FirstStartDialog` in portable mode (the guide,
scrolls; `again` from Help > This Computer, action `thisComputer`), Options > General > This Computer. No standard
backup folder in portable mode: `MainWindow::backupFolderReady(AskFolder)` (question `backupFolderQuestion`, tests
`QB_TEST_ASK_BACKUP_FOLDER`, `QB_TEST_BACKUP_FOLDER`), status-bar link `backupState`. Style `windows11` on Windows.
`resources/app/quillbinder.ico` + `.rc` (the exe's icon). Runner: the self-test's `data` is removed before the zips;
`win-check.sh` and `win-check-layout.sh` run on copies (`check-program`, `check-main`), with checks `portable-settings`,
`portable-nothing-outside` (AppData, registry, shortcuts), `portable-readonly-folder` (icacls deny). Suite `qb_w4_tests`.
Pitfalls found by the runs: Qt's `windows11` style keeps its light fields under a dark palette unless
`QStyleHints::setColorScheme` is set first (`theme::apply`); a widget's own `palette()` may still be the old one when
`theme::Notifier::changed` arrives, so colors written into rich text take `QApplication::palette(widget)` (the backup
link was black on the dark status bar, on Linux too); a theme in a test is changed as Window > Themes does it (the
program's theme and `look::change` for the open project), else the project's areas keep their look; `QB_DATA_DIR` must be
absolute with "/" (paths are compared); icacls `(WD,AD)`, not `(W)` (that also denies SYNCHRONIZE, and Git Bash cannot
start in the folder).

**W4c (2026-11-12): the add-ons with program code.** Speech Transcription on Windows, the processor only (the writer's
choice): `packaging/windows/win-whisper.sh <prefix>` builds whisper.cpp v1.9.5 with MSVC (shared, `GGML_BACKEND_DL`,
`GGML_CPU_ALL_VARIANTS`: the variants MSVC can build, no Vulkan, no OpenMP) into `.whisper` (cached by the workflow,
key = the script's hash; the first build takes 10 to 20 minutes); `win-ci.sh` sets `QB_WHISPER_DIR`; the plugin's
CMake links `whisper.lib`/`ggml.lib`/`ggml-base.lib` with `WHISPER_SHARED`/`GGML_SHARED`/`GGML_BACKEND_SHARED` and copies
`bin/` into `tool-plugins/speech/lib`; `win-addons.sh` packs `speech-transcription-windows.qbpkg` (plugins/qb-speech.dll,
speech/qb-speech-impl.dll, speech/Qt6Multimedia.dll, speech/lib/whisper.dll, ggml.dll, ggml-base.dll, ggml-cpu-*.dll;
no TLS plugin: the program's Schannel plugin downloads the models). `SpeechTool::engine` loads ggml-base, ggml and
whisper from `speech\lib` by full path before the engine (Windows uses a loaded DLL of the same name);
`SpeechEngine.cpp` finds its folder with `GetModuleHandleExW` and hands whisper.cpp and ggml UTF-8 paths on Windows
(`pathBytes`: both read `char` paths as UTF-8 there; a writer named Jürgen). Suite `qb_w4c_tests` (Linux and Windows):
the package, its files, the tool, and with `QB_SPEECH_MODEL_FILE` the model check and a recording from a file, with the
data folder, the model and the transcriptions in folders named with "ü" (on the runner `win-tests.sh` names the data
folder so; the model is whisper.cpp's ggml-tiny.bin, SHA-256 pinned there, cached in `.models`). The media add-ons
were already Windows add-ons (W2, W3); `media-dependents.txt` showed that Qt 6.12's Windows FFmpeg plugin needs no Qt
Quick, QML or OpenGL (unlike Linux). The explore failures of v0650 and v0604 were all Linux assumptions of the tests: files of
`/proc` (safe copy's short read, F23, Duplicate: Linux only now), a shell script as command exporter (a `.cmd` on
Windows), the scratch pad's package (`QB_TEST_ADDONS_DIR`, `<id>-windows.qbpkg`), the emergency folder taken from
`GenericDataLocation` (now `qb::dataDir()`), the README and the release workflow read with CRLF (Git on Windows);
v0700 hung in a file walk written `found = found || readAll(it.next())...` (once found, `next()` was never called again;
Linux's file order hid it; also in v0650 and v040: write the call first), and it read the runner's real AppData settings
file (on Windows HOME and XDG_* mean nothing: `portable::useProgramFolder()` with `QSettings::IniFormat` keeps a suite's
settings in its data folder). Since W4c every suite of `win-tests.sh` is counted (17); `EXPLORE` is empty.

**W2 (2026-11-07): the Linux-only parts on Windows.** Linux paths unchanged; `src/core/Platform.*` holds the small
differences (`processAlive`, plugin patterns, program file name, package platform).
- Crash and freeze reports: `src/core/CrashGuardWin.cpp` (same report format as Linux; the list and headers are in
  `CrashGuardCommon.cpp` for both). Unhandled-exception filter, SIGABRT, the C library's invalid-parameter and
  pure-call handlers; the failing thread only copies its state and wakes a writer thread made at `install()`, which
  writes without heap memory, then a minidump `crash-<time>-<pid>.dmp` (it can hold document text: the report says
  so). Backtraces are `file+offset` (RtlVirtualUnwind); the symbols file `Quillbinder-<version>-windows-x64-symbols.zip`
  (.pdb, built with `/Z7` + `/DEBUG`) goes with every runner result. Freezes: SuspendThread / GetThreadContext /
  ResumeThread, unwound after the main thread runs again. Sessions: a running instance is known by program file name
  and start time. WM_ENDSESSION (log off, shutdown) saves through `onTermination` and ends the process.
- Program updates: `Launcher.cpp` starts the update as a child process (`startChild`: PATH and `QB_CORE_APPDIR` to the
  program folder, `qt.conf` with `Plugins = .`), waits and ends with its exit code; a child that cannot load its DLLs
  (0xC0000135 ...) switches the update off at once. Runtime id `win64-qt6.8.3-msvc-r1` (the indented `set(APP_RUNTIME`
  in `CMakeLists.txt`; the workflow pins Qt 6.8.3). `launcher::startProgram()` is what a restart or a writing tool's
  command line starts (the built-in program, also when an update runs).
- Packages: `"platform"` in the manifest (`make-package.py addon --platform windows-x64`); `addons::inspect` refuses an
  add-on with libraries of another system (a Linux package without the field counts as Linux when it has .so files).
  Plugins are `*.dll` on Windows (`platform::libraryPatterns()`). `packaging/windows/win-addons.sh` builds the Windows add-ons
  `<id>-windows.qbpkg` (writing tools, importers, Media Playback); add-ons without libraries are the same on every system.
- Media Playback on Windows: `plugins/media/MediaEngineWin.cpp` (LoadLibraryEx), backend `windows` (Qt's Media
  Foundation backend: Windows decodes, no FFmpeg in the add-on); Qt's `ffmpeg` backend only if the add-on brings it.
- Writing-tool link: named pipes leave no file, so each instance writes a marker `<pid>.sock` in `%TEMP%\quillbinder`.
- Also: the project lock check (`platform::processAlive`), System Information (memory, processor, desktop line),
  fonts (`%LOCALAPPDATA%\Microsoft\Windows\Fonts` + HKCU registration), Help > Add to Application Menu hidden.
- Runner checks: `packaging/windows/win-check.sh` (update install and loader, damaged update, crash / abort / freeze reports,
  sessions, add-on installs, a Linux add-on refused, a tool plugin loaded, the media add-on started, the instance
  link). Developer switches of the Windows program for them: `QB_TEST_CRASH=access|abort|freeze|media|quit`,
  `QB_DEBUG_ENV=1 --versions` prints the loader's state. Linux checks of W2: `qb_w2_tests` (suite `w2`).

**W3 (2026-11-08): dictionaries, extra formats, licenses, three downloads, tests on the runner.**
- Spelling: `packaging/windows/win-dictionaries.sh <program folder>` puts en_US, en_GB, de_DE, de_AT, de_CH (LibreOffice's
  dictionaries at tag `libreoffice-25.8.7.3`, checked against `packaging/windows/win-dictionaries.sha256`; de_XX = de_XX_frami)
  into `dictionaries\` beside the program; `SpellChecker` looks there on Windows (`launcher::appDir()` and the
  program's own folder) and hands Hunspell `\\?\` + UTF-8 paths (any letter in the path). The first start takes
  Windows' language when a dictionary fits (`AppSettings::load`).
- Media: the Windows add-on "Extra Video and Audio Formats" (`packaging/windows/win-addons.sh`) carries Qt's
  `ffmpegmediaplugin.dll` and Qt's FFmpeg DLLs in `media-extra\qtplugins\multimedia` and `media-extra\lib`; the
  script asks FFmpeg (Python ctypes: `avcodec_license`, `avcodec_configuration`) and refuses a GPL, version3 or
  non-free build; `ffmpeg-build.txt` goes into the add-on. `MediaEngineWin.cpp`: `extraDir()` as on Linux
  (state.json "disabled"), backend `ffmpeg` first while the add-on is there, else `windows`.
- Licenses: `resources/licenses-windows/` (format of resources/licenses) made by `packaging/windows/collect-licenses-windows.py`
  from Qt 6.8.3's `qt_attribution.json` files (qtbase, qtsvg), Qt PDF's parts (qtwebengine-chromium at the
  submodule commit of v6.8.3, Qt's list "Qt PDF Licensing" plus PDFium's own parts), Hunspell 1.7.5, zlib 1.3.2 and
  the dictionaries; checkouts and tags in the script's header. `tools/gen-resources.py` writes
  `resources/resources-windows.qrc` (same files, the Windows list under `:/licenses/`); CMake uses it on Windows
  (`QB_RESOURCES_QRC`, also in tests). The About text has a Windows variant (`MainWindow::aboutText`).
- Three downloads (`packaging/windows/win-ci.sh`): main `Quillbinder-<v>-windows-x64.zip` (program, Qt, dictionaries; without
  `vc_redist.x64.exe`, `dxcompiler.dll`, `dxil.dll`), update `quillbinder-<v>-windows-update.qbpkg` (a bundle:
  program update + every add-on; a program update the program already is, is skipped inside a bundle,
  `PackageInfo::skip`), full `Quillbinder-<v>-full-windows-x64.zip` (the program with `full-addons\`, installed at
  the first start of the version by `addons::installBundled`, marker `full-addons-installed-<v>` as on Linux).
  Content add-ons are built on the runner too (`qb_template` is built on Windows). Checks:
  `packaging/windows/win-check-layout.sh`.
- Tests on the runner: `packaging/windows/win-tests.sh` builds the suites it names (`tests` is `EXCLUDE_FROM_ALL` on Windows)
  and runs them with `QT_QPA_PLATFORM=windows`, 15 minutes each; output in `test-output.zip` on `win-ci`.
  `qb_w3_tests` (suite `w3`, Linux and Windows): bundles with a program update of the running version,
  `installBundled`, the license list in the program, the dictionaries (`QB_TEST_DICTIONARIES=<folder>`).
  The runner gets the Linux test fonts (`test-fonts-linux-1.zip` on the branch `assets`, SHA-256 in the script;
  DejaVu, Carlito, Caladea, FreeSerif), installed for the whole machine; it has no Noto Serif or Liberation Serif,
  so a project's automatic font is DejaVu Serif there (checks that paste or apply a font take one that differs
  from `display::resolvedFontFamily`). The runner's screen is 1024 x 768; `win-tests.sh` sets 1920 x 1080 first (ChangeDisplaySettings, works). The core
  checks skip the update's self-start with `QB_TEST_NO_SELFTEST=1`.
- Also (W3): Windows keeps the settings in an INI file (`QSettings::IniFormat`, `main.cpp` and `AppSettings.cpp`),
  not in the registry. The Markdown exporter (example plugin) is built and put into `plugins\` beside the program
  (`PluginManager` looks there). RTF from Windows programs comes as `application/x-qt-windows-mime;value="Rich Text
  Format"` (`rtfFormatOf` in `TextEditor.cpp`). Hunspell gets words in the dictionary's encoding
  (`SpellChecker.cpp`: `takeEncoding`, `toDictionary`; the German frami dictionaries are ISO8859-1, also on Linux).
  Windows refuses to replace a file that is still open: tests close files they read before the next save.

## Windows 0.91.3 (2026-11-15): pulled to Linux 0.91.3, a release of its own, shorter runs, packs

- **What it brings**: the source of Linux 0.91.3 (the add-on keys: an add-on signed with an add-on key that the
  release key vouches for installs as signed). No Windows code changed for it. Version 0.91.3 like Linux (the
  writer's choice: the same number, though the last number may differ since 2026-11-15); so `APP_VERSION` serves both
  and no version line per system was needed yet.
- **Its own release** (the writer's choice, the first time the Windows number could differ): `release-windows.yml`
  publishes into `v<version>-windows-experimental`, made by its job `publish` when it is not there (`--latest=false`:
  GitHub's "Latest", `.../releases/latest`, the official add-ons' workflow and the README keep leading to Linux).
  `version` no longer needs the Linux release of the same number, but a Linux release `v<major>.<middle>.*` with its
  full AppImage (Linux leads). The release text is written by the job itself (`tools/release-notes.py` is shared and
  says "beta" and the Linux news). The download page: the newest Linux release is the page's version, each other
  system comes from its newest release (`download-page.py --also`, as `release.yml` does since 0.91.2); the page
  artifact has a name per attempt, so `download-page` can be rerun; the input `page_only` (start it with
  `-F 'inputs[page_only]=true'`) runs only `version` and `download-page`, for a page job that failed after the files
  were published (the first public run of 0.91.3: `gh ... | head -1` under `pipefail` ended with exit code 141, a
  broken pipe; jq now picks the first tag itself). The tag starts with `v`, so the Linux
  workflow's page finds the Windows files too.
- **The add-ons are signed with the add-on key**, as in `release.yml` (decided 2026-11-15: as for Linux):
  `sign-packages` certifies `QB_ADDON_KEY` with the release key, signs with both (the program update and the bundle
  with the release key, each add-on with the add-on key, in the bundle part by part) and checks every package with
  `make-package.py verify --release-key` (it replaced the job's own check, which knew the release key only). Windows
  0.91.1 does not know add-on keys and warns before it installs those add-ons, also from the update bundle; the
  release text says so and points to the setup program, which keeps `data\`.
- **Shorter in-between runs** (D2): `[build-only]` or `[suites: v0913 w4]` on a line of its own in the commit message
  (the workflow hands the message over as `QB_WIN_SWITCHES`; `win-ci.sh` parses it, `win-tests.sh` takes
  `QB_WIN_SUITES`). Only a whole line counts: the first run of `win/0.91.3` became build-only because its message named
  `[build-only]` inside a sentence. A suites run
  makes the program checks and the named suites; `setup` and `layout` in the list add those checks. Its result line
  says `partial ok`; the last run before a merge must say `ok` (no switch).
- **Dependency packs** (D1): `packaging/windows/win-deps-pack.sh` (recipe, get, publish). whisper.cpp's recipe is
  `win-whisper.sh` and MSVC's major.minor; Hunspell's is the vcpkg ports of hunspell and zlib (their files), the
  triplet file, MSVC's major.minor and the pack script. A pack is a `.tar.xz` on the pre-release
  `deps-<name>-windows-<16 hex>`; the pins are `QB_WIN_PACKS` in `windows.yml`; the release workflow builds
  as before (from source, with its caches). A recipe without a pin is built (or taken from
  the branch's cache) and then published; its `pin:` line is in `summary.txt`. A pinned pack is taken before the
  caches. Hunspell's pack is vcpkg's `installed/<triplet>` folder (classic mode: CMake only reads that folder). The
  first packs (run 43746715076, MSVC 14.51): `deps-whisper-windows-4105b2802919f7eb` (1.8 MiB) and
  `deps-hunspell-windows-ac5c39a435047f77` (5.4 MiB); building them took 5 and 10 minutes. Changing
  `win-deps-pack.sh` changes Hunspell's recipe (the script is part of it), so a new pack is made once.
- **Warm main** (P3): a push to `main` whose message names a `win/` branch (the merge of a Windows pull request; not
  with `[skip ci]`) runs `windows.yml` build-only on `main` (`QB_WIN_ONLY=build`), so that the next branch finds
  `main`'s caches. It leaves `win-ci` as it is.
- **Tests**: `qb_v0913_tests` is counted from its first run (no `EXPLORE` round: one full run less); it runs `python3` and `openssl` like the release workflow:
  `win-tests.sh` puts a copy `python3.exe` beside the runner's `python.exe` when there is none (a Windows program
  looks a name up with `.exe` only).
- **Found by the first full run (4357154)**: the Windows scripts read the runtime id as every indented
  `set(APP_RUNTIME ...)` of `CMakeLists.txt`; since Android A0 there are two (Windows', Android's), so the program update
  named both and every update check failed (`win-ci.sh`, `win-check.sh`: now the `win64-` line only). It would have
  broken the public Windows update package too. Two Linux checks assumed Linux and got a branch inside
  `#ifdef Q_OS_WIN` (the Linux code is unchanged): v0902 reads `release.yml`, which Windows checks out with CRLF; v0913
  expects the AppImage's runtime for an official add-on, which on Linux is `QB_RUNTIME_ID`.

## Public Windows files (0.91.1, 2026-11-14)

- Working rules, section 9: Linux leads; the Windows files are added to the Linux release of the same number by a
  workflow of their own, `.github/workflows/release-windows.yml` (started by hand:
  `gh api -X POST repos/Shelin-boop/Quillbinder_Collection/actions/workflows/release-windows.yml/dispatches -f ref=main`).
  It never changes `release.yml`, the Linux files or `SHA256SUMS.txt`.
- Jobs: `version` (the Linux release of this version must exist) -> `build` (Windows, `QB_WIN_STAGE=release` of
  `packaging/windows/win-ci.sh`: no test switches, no tests, everything unsigned in `results/stage`) ->
  `sign-packages` (Ubuntu, the key) -> `assemble` (Windows, `packaging/windows/win-release.sh`: zips, setup programs,
  checks with the program as released) and `sources` (Ubuntu) -> `sign-checksums` (Ubuntu, the key:
  `SHA256SUMS-windows.txt`) -> `publish` (uploads with `--clobber`, one paragraph in the release text) ->
  `download-page`.
- File names carry "experimental" (`QB_WIN_NAME` in the job `assemble`; the writer's choice of 2026-11-14). The .exe
  files have no code-signing certificate (the writer's choice): SmartScreen warns, Smart App Control blocks.
- Tests as for Linux (decided 2026-11-14: done as for Linux): the job `tests` is a second
  build with the test switches and runs `win-ci.sh` as the work branches' Windows workflow does (program, setup and layout
  checks, the suites of `win-tests.sh`); `publish` waits for it. Its fonts come from the job `test-fonts` (Ubuntu
  24.04's packages, handed over as `QB_TEST_FONTS_ZIP`; `windows.yml` still takes its archive from `assets`).
  Run `windows.yml` on the same source first all the same.
- Sources only the Windows build needs go to `third-party-sources` under the names of
  `resources/licenses-windows/index.json`: `hunspell-<v>.tar.gz`, `libreoffice-dictionaries-<v>-de.tar`, and
  `ffmpeg-<v>.tar.xz` (the version of the add-on's `ffmpeg-build.txt`).
- Left for a Linux job (shared files a Windows job must not change): the manual still calls the Windows version
  "in preparation, not published yet" (`docs/manual.md`). And `tools/publish-release.sh` uploads only `*.src.tar` to
  `third-party-sources`, so Qt's source archive `qt-everywhere-src-6.12.0.tar.xz`, which the Linux releases 0.80.0 to
  0.91.1 name there, was never uploaded (found by the first public Windows run, 2026-11-14). The Windows workflow's
  job `sources` fetches the archive from the Qt Project when it is missing and `publish` adds it; the Linux script
  still needs its fix, or a later Qt version will be missing again.
