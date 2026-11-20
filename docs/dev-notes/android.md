# Android

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## Android 0.92.0 (2026-11-20): pulled to Linux 0.92.0 in the one repository, a tablet with keyboard and mouse

What 0.91.3 showed on a tablet with an external keyboard and mouse (the findings of 2026-11-16): About could not be closed, some windows had no button that closes them, menus hid and
jittered and did not fit at small sizes, Ctrl + wheel did not zoom, and the binder's width could not be changed
upright. The decisions: Android 0.92.0 at the function level of Linux 0.92.0, on Qt 6.12.0 with a workaround for Qt's
fault (Qt 6.12.1 later as Android 0.92.1), all the program's own points of the report, the pin and the padlock by
touch, the check for updates that only tells and opens the APK in the browser, released without a trial build.

- **`src/android/MobileCare.cpp`, `mobile::installCare()`** (main.cpp right after the style; the system test app's
  `cs_common.cpp` too): switches `windowcare::tabletMode()` on, wraps the style (`TabletStyle`: `SH_Menu_Scrollable`,
  `PM_SplitterWidth` 7 with three dots), and one application filter:
  - **Keys with the mouse**: Qt 6.12.0 for Android (`androidjniinput.cpp`, `mouseDown/Up/Move/Wheel`) sends mouse and
    wheel events with `Qt::NoModifier`, and each clears `QGuiApplicationPrivate::modifier_buttons`. The filter keeps
    Ctrl, Shift, Alt, Meta from the key events (a modifier key's press and release; the other keys carry the state)
    and adds them with `QInputEvent::setModifiers` to mouse and wheel events. It acts on the window's event too, so
    that `QApplication::notify` captures the corrected modifiers again for the widget's event: Qt's item views read
    the global state in `currentChanged` (`selectionCommand(index, nullptr)`), and Shift + click failed in the
    preview until the clicks went through the window as on Android (`QTest::mouseClick(QWindow *, ...)`).
  - **Back** (`Qt::Key_Back`; Qt puts the app away when the press was not accepted, `keyUp` in the same file): a
    popup, a tip, the modal window, the active window or the window shown last (non-modal windows such as Targets
    do not get the keyboard) is closed; with the main window alone the filter does nothing.
  - **Windows** (on `Show`): one of the program's own dialogs (`QDialog` or `qb::...`, not Qt's message boxes and
    file dialogs) or a `QMainWindow` with no button at all gets a Close row (`windowcare::addCloseButton`, a bottom
    toolbar for `QMainWindow`); a dialog that cannot be as small as the usable area gets its layout moved into a
    `QScrollArea` (`setLayout` takes a layout from another widget), its `QDialogButtonBox` rows below it, and its
    least size reset (the old layout had set it); then `windowcare::fitInto`. Again on `availableGeometryChanged`.
  - **Menus of the menu bar**: when `QMenuBar` put one over the bar (it does when the menu fits neither below nor
    above), it is moved below its title and made as high as the room; it scrolls (`SH_Menu_Scrollable`).
  - **Panes**: a `QStackedWidget` of the main window counts only its shown page for its least width (the other pages'
    horizontal policy `Ignored`, restored when shown): text 402 -> 275, inspector 317 -> 253 px, so at 800 and 736 px
    upright the binder, the text and the inspector fit and the line moves.
  - **The line between panes by mouse**: a real mouse (`MouseEventNotSynthesized`, device type `Mouse`) pressed up to
    9 px beside a `QSplitterHandle` drives the handle, as `MobileTouch.cpp` does for fingers.
- **Shared code behind `windowcare::tabletMode()`** (compiled on Linux, never on there): `BinderTree::pinShownOn`
  (the row under the mouse, on a tablet also the one selected row; a click on a pin that is not shown does nothing
  there), a tap on the padlock shows `lockTip` with `QToolTip` after the release, `showItemContextMenu` adds
  `fixMindMap` and `releaseMindMap` for the binder, `showAbout` is a `QDialog` with a `QTextBrowser`.
- **The check for updates**: `Offer::Kind::NewApk` for a system `android-...`: always the block's `"apk"`, never a
  program update; `UpdateChecker::installNext` opens its address with `QDesktopServices::openUrl` (the browser
  downloads, Android installs; no `REQUEST_INSTALL_PACKAGES`). Four texts only Android shows, in the translation
  file. `tools/update-feed.py --system android-arm64`: `"apk"`, the add-ons with `platform: android-arm64`, the
  headlines of `## Android <version>` first. `release-android.yml` writes and signs Android's block, checks that the
  others are unchanged, and publishes it with the page.
- **Translation and Proofreading 1.2**: `packaging/android/android-translator.sh <abi> <out>` (the add-on's own
  `build-deps-android.sh`, cached by the workflows under its hash and the NDK; its CMake project with Qt's Android
  toolchain; `check-exports.sh`; `make-addon-android.sh`; then the libraries' machine, 16 KB segments and needs) ->
  `translation-proofreading-android-<arm64|x86_64>.qbpkg`. `android-ci.sh` puts it into `assets/full-addons`
  (`QB_ANDROID_FULL_ADDONS`: the release's signed one). `mobile::installFullAddons("assets:/full-addons")` (main.cpp,
  before the plug-ins load) copies the packages out of the APK and calls `addons::installBundled` (once per version).
  The release: `build` -> `sign-packages` (add-on key, certificate of the release key) -> `sign-apk` (the signed
  package replaces the unsigned one in the APK with `zip -0`, checked by SHA-256, then zipalign and apksigner with
  `--v4-signing-enabled false`). The system test's build job makes the x86_64 package (`QB_ST_ADDON_FILE`); the
  model comes from this repository's `models-1`.
- **Packs**: their pre-releases name content, license and source (`about` in `android-deps-pack.sh`); made anew in this
  repository by the first run, pins in `android.yml` and `android-system-test.yml`.
- **Tests**: `tests/mobile_fit_tests.cpp` (`qb_mobile_fit_tests`, preview only), run by
  `packaging/android/run-tablet-fit.sh <build>` at nine sizes (1472x920, 1472x836, 1920x1080, 1280x720, 1178x660,
  960x540, 800x1200, 736x1100, 1472x420): 24 checks each, 62 windows opened by 73 commands (every command with "..."
  and the Help menu), each inside the screen, its buttons in view, a button that closes it, Back closes it; every menu
  below the bar, one column, its last entry in reach; About; Back; Ctrl/Shift + click and Ctrl + wheel through the
  window; the line by mouse; pin and padlock; `offersFor` on Android; the APK's add-ons. `QB_FIT_ONLY` runs parts,
  `QB_FIT_WIDTHS=1` prints which widgets decide the panes' least widths. `qb_mobile_tests`' check that the install
  command is hidden on a tablet was stale since 0.91.3 and now expects it shown.
- **Not here**: the Android trial key stays per run; `official-addons/translation-proofreading.json` keeps its Android
  block (a Linux suite, v0913, reads the file); the menus' jitter on the writer's tablets is not explained (each menu
  is a new Android surface under `WindowGuard`, and Qt asks for the on-screen keyboard when a menu closes).

## The translation add-on as an official add-on for Android (2026-11-16)

The second step after the public APK (the APK first, the add-on next): Translation and Proofreading 1.1 for Quillbinder for
Android, built, signed and published by the project like the Linux and Windows add-ons (`docs/official-addons.md`).

- **The entry's `android` block** (`official-addons/translation-proofreading.json`): the add-on's commit
  `d0316b5ae568` (main after its Android work), its own commands (the add-on's `packaging/build-deps-android.sh` for
  arm64, cmake with Qt's Android toolchain, `tests/check-exports.sh` with the NDK's `llvm-nm`,
  `packaging/make-addon-android.sh`), and `toolchainLibraries`: LLVM's OpenMP runtime of the NDK, linked into the
  engine (no source folder of its own; the release's SOURCES.txt names where it comes from).
- **`tools/addon-entry.py --android`**: the block checked like Windows' (`check_system_block`), the tag
  `addon-<id>-<version>-android`, the cache key `...-ubuntu-24.04-android`; `check-package` wants platform
  `android-arm64`, the Android runtime of `CMakeLists.txt` (its second `APP_RUNTIME`, inside `if(ANDROID)`), minApp
  0.91.3 or later, no Windows files, and with `READELF` (the NDK's `llvm-readelf`; without it the check fails) every
  library for AArch64, needing only Android's own libraries, `libc++_shared.so` and Qt's `_arm64-v8a` libraries, and
  LOAD segments aligned to 16 KB. Sources: `<id>-<version>-android-source.tar.gz`; the release text tells how to install
  on the tablet.
- **`.github/workflows/addon-release-android.yml`** (public only, started by hand, input `publish`): prepare (the entry,
  "published already?", the newest `v*-android-*` release at 0.91.3 or later) -> build (Ubuntu 24.04, NDK
  27.2.12479018 by sdkmanager without pipefail, Qt 6.12.0 for Android arm64 and for the build machine by aqtinstall
  3.3.0, kept in a cache; the add-on's setup and steps; the package check; sources; release text) -> sign (the add-on
  key, vouched for by the release key) -> publish (`... for Android (official add-on)`, never "Latest"). No install
  check: Quillbinder for Android runs only on a tablet; the same source's x86_64 build ran in the Android system
  test (trial builds 25 and 26).

## The first public Android release (0.91.3 experimental, 2026-11-16)

Decided on 2026-11-16: merge and release; the APK now, the translation add-on for Android
as an official add-on in a second step; app id **`io.github.shelin_boop.quillbinder.app`**; the key made by Claude and
kept by the writer; the name carries "experimental", like Windows.

- **The key**: PKCS #12, RSA 4096, alias `quillbinder`, certificate "CN=Quillbinder, O=The Quillbinder project", valid
  until 2126; made with keytool in the session, handed to the writer with its password, then deleted there. Secrets of
  this repository: `QB_ANDROID_KEYSTORE` (base64 of the file), `QB_ANDROID_KEYSTORE_PASS`. Its certificate's
  SHA-256 is in `packaging/android/release-key.txt`; the workflow refuses another key. **Every later public APK must be
  signed with it** (Android installs an update only from the same signer).
- **`.github/workflows/release-android.yml`** (started by hand): version (tag
  `v<version>-android-experimental`; a Linux release of the same functional level must exist) -> build
  (`android-ci.sh` with `QB_ANDROID_RELEASE=1`: the permanent id, `QB_ANDROID_VERSION_NAME`, the file name
  `Quillbinder-<version>-experimental-android-arm64.apk`, unsigned, the packs not published) -> sign-apk (zipalign
  `-P 16`, apksigner with the key, then apksigner's signer and v2 scheme, the 16 KB borders, aapt2's id and version
  name checked) -> sources (what `resources/licenses-android/index.json` names and `third-party-sources` lacks: OpenSSL,
  checked against the SHA-256 pinned in `android-openssl.sh`) -> sign-checksums (`SHA256SUMS-android.txt`, the release
  key) -> publish (never "Latest") -> download-page. No test suite runs there: the work branches' Android build and
  system test checked the same source. Started with
  `gh api -X POST repos/Shelin-boop/Quillbinder_Collection/actions/workflows/release-android.yml/dispatches -f ref=main`
  (`-F 'inputs[publish]=false'` builds, signs and checks only; `-F 'inputs[page_only]=true'` only the page).
- **Trial and public side by side**: trial builds keep `...quillbinder.trial` and the trial key; the public app is
  another app on the tablet (its own data). `CMakeLists.txt`: `QB_ANDROID_PACKAGE`, `QB_ANDROID_VERSION_NAME`.
- **The page and the other workflows**: `tools/download-page.py`'s Android card got its first steps (install from the
  browser or Files, "All files access"); release-windows.yml and release-android.yml take the newest Linux release
  without `-windows-` and `-android-` tags. The README's Android part and the manual's "On Android tablets" (section
  1) name the release. `docs/CHANGES.md` gets no entry of its own (as for Windows 0.91.3: its first section is Linux's
  and feeds the program update's news).

## Android 0.91.3: add-ons, the translation add-on, Android's own system test (2026-11-16, trial builds 25 and 26)

Job 4 continued after the writer's pause for the new working rules (sections 8, 10, 11). The writer's answers of
2026-11-16: Android takes the number 0.91.3, like Windows; **signed add-ons are allowed on Android** (before: one full
APK, no add-ons); the translation add-on is built for Android in its own public repository; everything, including the
system test's run. `main` 6c23dfd (Linux 0.91.3) was merged into `android/full`. Everything is Android's own switch:
`src/android`, `packaging/android`, `tests/android`, `if(ANDROID)`, and blocks inside `Q_OS_ANDROID` in shared files
(`core/Addons.cpp`, `core/Platform.cpp`, `compile/PluginManager.*`, `ui/ToolHost.cpp`, `ui/AddonDialogs.cpp`,
`ui/TextEditor.cpp`, `ui/LinkOpen.cpp`). Linux proof each time: the standard build, `qb_tests`, `v060`, `fixes`,
`v0611` and a 10 % sample, 0 failures.

- **Add-ons on Android**: `addons::addonsRoot()` is `AppLocalDataLocation/addons` (the app's private folder: Android
  runs no library from the shared storage, where `dataDir()` is); an add-on's data (models, settings) stays in
  `dataDir()`. The package system is `android-arm64` or `android-x86_64` (the emulated tablets; `Platform.cpp`). A
  package with libraries for another system or processor is refused with its reason, a program update (kind `core`)
  too: a new Quillbinder for Android comes as a new APK. A package without libraries names no system and fits every
  one (trial 26; trial 25 refused themes and templates: S-A07). The texts that only Android shows are English and
  German side by side (`androidSay`: German when the program's German translation answers for one of the
  installer's own texts), never through the shared translation file. After an install the program starts again
  through Android's launch intent (`NEW_TASK | CLEAR_TASK`, then `System.exit(0)` after 400 ms;
  `relaunchApplication`). The Add-on Manager hides the program-version part and the folder button; File > Install
  Update or Add-on is shown again (`MobileStart.cpp` hides only `integrate`), its file window opens in Download.
- **Type information across libraries** (trial 26): `Host::extension()` reached a tool's interface 1.1 by a
  `dynamic_cast<QObject *>` from `tools::ToolInterface *`. On Android that finds nothing: the interface has no key
  function, so the program and each plug-in keep their own copy of its type information (libraries loaded side by
  side, not merged), and Android's C++ library compares type information by address. The tools' sections in
  File > Options, their inspector tabs and toolbar buttons were missing (A-A01 on the tablet). Now
  `PluginManager::toolObject()` keeps the loader's object of each tool, and `Host::extension()` uses it on Android.
  Any other `dynamic_cast` across a plug-in's border would fail the same way (`qobject_cast` works: it compares the
  interface's name).
- **Clipboard** (`TextEditor.cpp`): Android's clipboard carries plain text and HTML only (through Qt). The program
  keeps its own last copy (`createMimeDataFromSelection`) and pastes it while the clipboard's text equals the copy's
  (object characters U+FFFC taken out, white space simplified: a figure alone, S-T03).
- **Links** (`LinkOpen.cpp`): app packages (`apk`, `apks`, `apkm`, `xapk`, `aab`) are refused as programs. Files on
  the shared storage never carry the right to run (chmod is refused there), so a file without an ending is asked
  about, not refused.
- **Packs (D1)**: `packaging/android/android-deps-pack.sh` (see `packaging/android/README.md`). Pinned on 2026-11-16
  from run 43851207847: `hunspell-arm64-v8a`, `hunspell-x86_64`, `openssl-arm64-v8a`, `openssl-x86_64`. A new NDK or
  a changed build script makes a new recipe: the next run builds it, publishes it and prints the `pin:` line.
  `android.yml` also runs on `main` when the merge message names `/android/` (build only; `android-ci` untouched).
- **The translation add-on for Android** (repository `Quillbinder_Translation_Addon`, branch `addon/android`, not
  merged yet): CTranslate2 4.9.0 with Ruy (oneDNN is for x86 only), SentencePiece 0.2.1, LLVM's OpenMP linked in
  (`-static-openmp`), 16 KB pages, plain `.so` names (`SUFFIX ".so"`), `CMAKE_FIND_ROOT_PATH_MODE_* BOTH` for the
  engine's own libraries. Its workflow builds both ABIs on the public repository's free runners, checks exports, the
  libraries each needs and the 16 KB pages, and puts `translation-proofreading-1.1-android-<abi>.qbpkg` and
  `SHA256SUMS-android.txt` on its pre-release `test-android` (nothing on its `main` yet). Models come from its release
  `models-1` (Models... in the add-on) or from a folder.
- **Android's own system test** (`tests/android/systemtest`, its README): a copy of Linux's system test (6c23dfd),
  changed only there; Linux's cases are untouched. 189 cases: 21 taken out (fault library, a second real program,
  Linux's key sender), 4 new (A-F01 folders, A-A01 the translation add-on, A-K01 killed while typing, A-H01 Home
  then killed). The script (`packaging/android/android-system-test.sh`): sub-runs `<case>@<variant>`
  (`subcases.txt`, `QB_ST_KEEP_DATA=1` keeps the app's data), needs K and H (the script kills the app after
  `kill-now` / Home and 6 s after `home-now`, then `--verify`), T (the add-on's Android package, checked against its
  SHA256SUMS, and the model opus-mt-de-en fetched and put under `QB_CS_SOURCE`), named cases shared among the tablets.
  Waiters of one kind of window answer in the order they were armed; a waiter that already looked in vain ends at
  the next step or at the next waiter of its kind (U-04, U-05, S-C04, S-T01). On the desktop:
  `-DQB_MOBILE_PREVIEW=ON`, target `qb_android_st_preview`.
- **First full run** (run 43856407155, quick profile, 8 tablets, Android 14, 18 to 41 min each): 180 cases and 6
  sub-runs; 172 clean, 12 with failed checks, 1 stopped at its time limit (S-E04: the tablet is about 3.5 times slower
  than the Linux machine, now factor 3), 1 ended early (U-04, a crash in the old waiters), 9 skipped (Speech
  Transcription is not built for Android). Program findings fixed in trial 26: add-ons without libraries, the type
  information, a figure through the clipboard, `.apk` links. Harness findings fixed: the waiters (U-04, U-05, S-C04,
  S-T01), A-A01's packages, S-I12's expectation. **Lost**: the logs of the fifth tablet (S-K01, S-T04, S-D01 with a
  failed check each): the next run deleted them before they were read (read each tablet's files as soon as it ends;
  its summary lines are also in the job's log).
- **Check runs**: run 43870845331 (15 cases on 2 tablets, trial 26's code): A-A01 and A-A01@loaded clean (the
  translation "The house is old and the weather is nice." in 41 s, the model's first load included; the tool's
  Options section there), S-A07, S-I12, S-C04, S-T01, S-A02, S-A03, C-12, S-W16, U-05 and its two sub-runs clean;
  S-T03, S-I01, S-D05 failed checks; S-E04 stopped at its time; U-04 crashed again. Run 43879216552 (6 cases):
  S-T03, S-D01, S-K01, S-T04 failed checks, S-E04 stopped at its time after its last step (436 menu entries chosen,
  2.5 s a step: its estimate is now 500 s), U-04 crashed again (SIGSEGV inside a `withModalTry<QDialog>` answer, 23 s
  after the start; now guarded with `QPointer` and its window named in the log: not yet run on a tablet).
- **Open after the check runs** (each needs a look on the tablet or in the next run): S-T03, a figure cut and pasted
  does not come back (the clipboard's text equals the cut text; the program's own copy works in S-W16 and, with a
  clipboard reduced to text and HTML, on the desktop; cause not known); S-K01, the Word file loses the monospace font
  of a text message (probably the font name Android gives, not known); S-T04, the e-book's tables, figures with
  captions and alt text (no detail logged); S-D01, a typing gate with a p99 of 183 ms and a backlog of 3 keys on the
  emulator.
- **Found for a Linux job** (Linux code, not changed): the main window's undo watcher (`MainWindowUndo.cpp`) is
  reached during `~MainWindow`. U-05 (2026-11-14) crashed in it; on 2026-11-16 valgrind showed the second form: when
  the keyboard was last in an add-on's panel (a child of the main window made after `m_undoNamesTimer`),
  `~QWidget` deletes the timer first and then the panel, whose `destroyed` starts the freed timer (writes of 4 and
  8 bytes, "corrupted double-linked list" at the end). Likely when a writer quits with the keyboard in the
  translation add-on's panel (`main.cpp` keeps the window on the stack); not tried with the real program. Fix:
  disconnect `m_undoWatch` and the `focusChanged` connection in `~MainWindow`, or make the timer a `QPointer`.
- **Open**: S-I01 types into the inspector's notes with 120,000 characters: a median of 154 ms per key on the
  emulated tablet (the desktop preview: 1 ms; the main editor on the tablet with a 60,000-word text: 9 ms); the cause
  is not known. S-D05: a backlog of 3 keys at 200 WPM on the emulator (the limit is 2), every delay within its
  limits; whether a real tablet passes is not known. Speech Transcription for Android, audio and video, recording,
  crash reports: not done. Neither trial build nor the add-on has run on a real tablet yet (the writer's test).

## Android: the full version (2026-11-14, trial builds 19 and later; branch `android/full`)

Decided after build 18: close the gap to the Linux version, merge, make it a full version with the missing functions,
keep projects and backups in Downloads or Documents, then adapt the system test and make a full run, and after that
try the first release. #69 is merged (main b7a46e8). All of this is Android's own switch: `src/android`,
`packaging/android`, `tests/android`, CMake inside `if(ANDROID)`, and blocks inside `Q_OS_ANDROID` in
`main.cpp`, `compile/PluginManager.cpp`, `ui/MainWindow.cpp` (the About text), `tests/crashsafe_tests.cpp`,
`tests/crashsafe/cs_common.cpp`.

- **Shared folders** (`src/android/MobileFiles.cpp`): Qt's "Documents" on Android 11+ is the app's own folder
  (`/storage/emulated/0/Android/data/<id>/files/Documents`: no other app shows it, deleted with the app). Projects go
  to the tablet's `Documents/Quillbinder`, backups to `Documents/Quillbinder/Backups`, exports to `.../Exports`,
  Scratch Pad notes to `.../Scratch Pad`. That needs Android's "All files access" (`MANAGE_EXTERNAL_STORAGE`; the
  writer switches it on on a page of Android's settings, opened with the intent
  `android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION` and `package:<id>`; Android 9 and 10: the storage
  permission through `Activity.requestPermissions`, with `requestLegacyExternalStorage`; that path was never run).
  `settleFolders(folders, own, access)` moves only settings that are empty or name one of the app's own folders
  (`AppSettings::lastProjectDir`, `welcomeFolder`, `backupDir`, QSettings `export/location`,
  `tools/scratch-pad/folder`); without access it takes settings inside the shared folder back. `tabletBeforeWindow()`
  (main.cpp, after the settings are read) settles when the access is there; `startTablet(&w)` (after `w.show()`,
  before the first start's window and the welcome project) shows the question `filesOnTablet` once
  (`android/filesKeptInApp` when declined) and adds File > Files on This Tablet. File windows (Qt's own): the shared
  folders at the side, and a window that would open in one of the app's own standard folders opens in the projects
  folder (`watchFileWindows`). Texts of Android-only windows are English and German side by side (`say()`), never
  through the shared translation file. **Seen on the emulated tablet (run 19): the question, shown right after the
  main window, did not get a keyboard's Enter, and Android's Back key then put the whole app away; a finger's tap
  works (run 20).** The window closes by itself when the access appears.
- **One full APK**: the dictionaries (the Windows program's: `packaging/windows/win-dictionaries.sh` run by
  `android-ci.sh` into `.apk-assets`, `-DQB_ANDROID_ASSETS`) are assets (`assets:/dictionaries`), copied by
  `copyDictionaries` into `dataDir()/dictionaries`, where `SpellChecker` looks anyway; the first start takes the
  tablet's language for spelling (`android/spellLanguageChosen`). The CMake block copies
  `packaging/android/package` into `<build>/android-package` and adds the assets there. The writing tools (Name
  Generator, Scratch Pad, Text Statistics), both importers (Word 97-2003, web pages) and the Markdown exporter are
  libraries in the APK: their targets get `LIBRARY_OUTPUT_DIRECTORY <build>/apk-plugins`, which is the app target's
  `QT_ANDROID_EXTRA_PLUGINS` (androiddeployqt copies `.so` files of that folder into `lib/<abi>`); `QB_APK_PLUGINS`
  lists them. **An APK keeps its libraries uncompressed inside itself** (not unpacked): Qt names the folder
  `<apk>!/lib/<abi>/` in `QT_PLUGIN_PATH`, lists it through its own file layer and loads from it;
  `PluginManager::searchDirs()` adds that folder and `load()` takes only `libqb-*.so` and `libmarkdown-exporter*.so`
  from it (seen working, runs 19 and 20: five plug-ins loaded). No plug-in needs symbols of the program (checked
  with `nm -u`). `installPackage` and `integrate` are hidden (`startTablet`).
- **https** (`packaging/android/android-openssl.sh`): Qt's only TLS on Android is its OpenSSL plug-in, which loads
  `libcrypto_3.so` and `libssl_3.so` from the APK. OpenSSL 3.5.9 from its own archive (SHA-256 pinned), built with
  the NDK through two targets written into its `Configurations` (`qbandroid-arm64`, `qbandroid-x86_64`:
  OpenSSL's Android targets plus `shlib_variant => "_3"`; make variables do not rename the libraries, run 21),
  `-Wl,-z,max-page-size=16384`; `-DQB_ANDROID_OPENSSL=<prefix>` -> `QT_ANDROID_EXTRA_LIBS`. The manifest asks for
  `INTERNET` (the import of a web page; nothing else connects). `startTablet` logs whether https works.
- **Saving when Android puts the app away** (`saveAllNow`, on `applicationStateChanged` away from Active): mind maps
  and `Project::save`, `saveFailed` on failure. Seen in the log of runs 19 and 20. Qt draws once more while Android
  takes the surfaces ("Failed to make context current" right after; harmless there).
- **Fingers for the right mouse button** (`MobileTouch.cpp`): a finger that rests `kMenuMs` (500 ms) without moving
  and goes up sends a `QContextMenuEvent` to what is under it (0 ms later, not while a popup is open); not on
  buttons, combo boxes, sliders, spin boxes, menus, splitter handles; not in a text that can be typed in
  (`WA_InputMethodEnabled`, but lists and trees also set that while their current line can be renamed: they do get
  the menu). In such a text Android's long press selects the word, and a tap on selected text opens the text's
  menu (`onSelectionOf`). Qt's own switch for a right click on a long press
  (`QT_ANDROID_ENABLE_RIGHT_MOUSE_FROM_LONG_PRESS`) is not used.
- **Files and folders for other apps** (`MobileOpen.cpp`, Android only, a `Q_OBJECT` in a `.cpp` with its `.moc`):
  `QDesktopServices::setUrlHandler("file", ...)`; a folder of the shared storage opens in the tablet's Files app
  (`content://com.android.externalstorage.documents/document/primary%3A<path>`, type
  `vnd.android.document/directory`), a file goes on to Qt (inside the handler `QDesktopServices::openUrl` reaches
  the platform: Qt's file provider covers the whole shared storage). Not seen working yet.
- **License list** `resources/licenses-android` (`packaging/android/collect-licenses-android.py <extra>`: made from
  the Windows list and checked against Qt's own list of the parts of its Android build, the SBOM
  `<Qt>/sbom/qtbase-6.12.0.spdx`, kept by the workflow as `qt-sbom.txt`; `gradle-dependencies.txt` names the Java
  libraries). `packaging/android/android-resources.py` writes `<build>/resources-android.qrc` at configure time
  (`QB_RESOURCES_QRC`). `MainWindow::aboutText()` has an Android text inside `Q_OS_ANDROID`. For a release:
  `openssl-3.5.9.tar.gz` must go to `third-party-sources`.
- **APK checks** (`android-ci.sh`): the permissions are exactly INTERNET, MANAGE_EXTERNAL_STORAGE and (up to
  Android 10) READ/WRITE_EXTERNAL_STORAGE; the plug-ins, OpenSSL and the dictionaries are in the APK.
- **Emulator steps**: A (the question: tap its first button at 725 x 510, `appops set --uid <pkg>
  MANAGE_EXTERNAL_STORAGE allow`, Back), M (menus by finger), K (`input keycombination 113 59 30` = Ctrl+Shift+B, a
  backup in Documents/Quillbinder/Backups: seen), P (a typed word, Home at once: the word on the shared storage;
  run 20: not found, cause open), O (Show Project in File Manager; its place in the menu was a guess, run 20 missed).
- **The system test on Android** (`tests/android`, `packaging/android/android-system-test.sh`,
  `.github/workflows/android-system-test.yml`, results on the pre-release `android-st`): the Linux system test's
  program built as an app (`qb_system_test`, id `...quillbinder.systemtest`, with `-DQB_TEST_SWITCHES=ON
  -DQB_ANDROID_SYSTEM_TEST=ON`), started once per case by the script: `pm clear`, the access given by `appops`,
  the arguments written to `/sdcard/Download/qbst/next-args.txt` (**Qt takes arguments from the start request only
  in an app built for debugging**), the output into `QB_CS_LOG`, the case's folder on the shared storage, a time
  limit of four times the Linux estimate, then `--verify`. `system-test-run.txt` says what a push to
  `android-st/...` runs (shards, cases, profile); `system-test-skip.txt` names cases a tablet cannot run.
- **Pitfalls of this job**: a new run of `android.yml` deletes the old run's files at its start (run 20's pictures
  and APK were lost by pushing first); `grep -c` ends with 1 when it counts 0 (it broke an `&&` chain); `m12` cannot
  run in the session (it needs the add-on packages); Qt's `QJniObject` calls swallow Java exceptions (use
  `QJniEnvironment` by hand to see a refused `startActivity`); the JNI parts are syntax-checked on Linux with
  stand-ins (`/home/claude/stub/android-jni-stub.h` in the session: not in the repository).
- **Not done yet**: audio and video, recording (record only), crash reports, the runs' open points above, the
  system test's first results, the release (the app's permanent id, the signing key, the name: the writer's).

## Android trial build (port, A0, 2026-11-14)

The first Android build: the program as it is (the Widgets window, no minimal interface yet), as an APK for tablets.
It runs parallel to Linux, as Windows does (working rules, section 9): its own workflow, scripts and results;
nothing the Linux or Windows build compiles, packs or tests changed.

- **Workflow** `.github/workflows/android.yml` (a push to `android/**`; `[skip ci]` starts no run,
  `[no-emu]` leaves the start check out). Runner `ubuntu-24.04`. Results on the pre-release **`android-ci`**
  (`summary.txt` first; the notes say `status=running`, `stage=emulator`, then `status=done result=... emulator=...`).
  Times: Qt install 2 min the first time (then cached), the APK 3 min with warm caches (11 min the first compile),
  the x86_64 build 1 to 4 min, the emulator stage about 10 min.
- **Tools** (Qt 6.12's documentation, checked 2026-11-14): NDK r27c (27.2.12479018, installed with sdkmanager; the
  runner carries 27.3, 29, 30), JDK 21 (`JAVA_HOME_21_X64`), SDK platform 36, build tools 36.0.0; Gradle 9.5.1 and
  the Android Gradle plugin 9.2.1 come from Qt's own templates.
- **Qt** (`packaging/android/android-qt.sh`): aqtinstall 3.3.0, `install-qt linux desktop` (the tools) and
  `install-qt all_os android 6.12.0 android_arm64_v8a|android_x86_64 -m qtpdf qtimageformats` into one folder.
  aqtinstall is patched first: Qt PDF for Android is in `all_os/extensions/qtpdf/6120/qt6_6120_<abi>/`, aqtinstall
  looks in `.../6120/android_<abi>/`.
- **Build** (`packaging/android/android-ci.sh <results> [abi]`): Hunspell 1.7.5 from source with the NDK's compiler
  (`android-hunspell.sh`, archive SHA-256 pinned), then CMake with Qt's toolchain file
  (`-DCMAKE_TOOLCHAIN_FILE=<qt android>/lib/cmake/Qt6/qt.toolchain.cmake -DQT_HOST_PATH=<qt gcc_64>
  -DANDROID_SDK_ROOT -DANDROID_NDK_ROOT -DQT_ANDROID_SIGN_APK=ON`), 16 KB pages
  (`-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON` and `-Wl,-z,max-page-size=16384`), no tests, tools or add-on plugins;
  targets `quillbinder` and `quillbinder_make_apk`; the APK is `build-android-<abi>/android-build/quillbinder.apk`.
  Signing: Qt's androiddeployqt reads `QT_ANDROID_KEYSTORE_PATH/_ALIAS/_STORE_PASS/_KEY_PASS`; the key for trial
  builds comes from `packaging/android/android-trial-key.sh` (a kept key named by `QB_ANDROID_TRIAL_KEYSTORE`, or a key
  made for the run alone; never for a public APK).
- **CMakeLists.txt**: everything for Android is inside `if(ANDROID)`, and **after `project()`**: before it CMake does
  not know yet that it builds for Android (`ANDROID` is set by the toolchain; run 3 had Qt's default app id for
  that). `qt_add_executable` instead of `add_executable` (the program becomes a library the app's Java part
  loads); app id `io.github.shelin_boop.quillbinder.trial` (`QB_ANDROID_PACKAGE`; the public id is not decided),
  version code `major*100000000 + minor*100000 + patch*1000 + build` (`-DQB_ANDROID_BUILD`, the workflow's run
  number), Android 9 (API 28) to API 36, runtime id `android-qt6.12-r1`. No `install(TARGETS)` on Android (CMake
  refuses it for a module library), no pkg-config there (it finds the build machine's Hunspell), no
  `/opt/qt` prefix.
- **Package files** `packaging/android/package`: `AndroidManifest.xml` is Qt 6.12.0's template without the
  `INSERT_PERMISSIONS` line (the app asks for no permission; Qt would add the internet permission for Qt Network)
  and with `android:allowBackup="false"`; compare it with Qt's template when the Qt version changes. Launcher icons
  from `resources/app/quillbinder.svg`.
- **"Not on Android" guards** (Qt defines `Q_OS_LINUX` on Android too): `CrashGuard.cpp` is off and
  `CrashGuardOther.cpp` on (no `<execinfo.h>` before Android 13); no glibc version in System Information
  (`CrashDialogs.cpp`); `fontfaces::separate()` does nothing (`FontFaces.cpp`); `platform::packagePlatform()` is
  `android-arm64` (a package made for Linux is refused). `main.cpp` on Android: `Qt::AA_DontUseNativeDialogs`
  (Android's file picker gives `content://` addresses, no folders) and `Qt::AA_DontUseNativeMenuBar` (Qt hands a
  native menu bar to Android's options menu, which Android shows nowhere).
- **Checks of the APK** (in `android-ci.sh`, 11): app id, version code, API levels, the app's name, one ABI, no
  permission but androidx's own, the program's library, Qt's libraries and platform plugin, the signature
  (certificate of the trial key), `zipalign -c -P 16`, every library's LOAD segments at 16 KB.
- **Start check** (`android-emulator.sh`): the x86_64 APK on an emulated tablet (system image
  `android-34;default;x86_64`, 1472 x 920 at 160 dpi: a 12.7-inch tablet's 2944 x 1840 in its own units), KVM, the
  runner's disk freed first (the emulator refuses to start with little room). Steps with `adb shell input`, each
  followed by "does the program still run"; pictures `emu-*.png`, the log `emu-logcat.txt`. A menu is closed with a
  tap beside it, not with Esc (Esc leaves the menu bar waiting for letters: run 7 typed into the menus).
- **Where the program keeps things on Android** (Qt 6.12, Android 11 and later): `QStandardPaths::DocumentsLocation`
  and `GenericDataLocation` are the app's own folders on shared storage
  (`/storage/emulated/0/Android/data/<app id>/files/Documents` and `.../files`), which need no permission and are
  deleted with the app; `HomeLocation` is the app's private `files`, settings are in `files/settings`. On Android
  9 and 10 the same calls name the public folders, which need a storage permission the APK does not ask for.
- **Found on the emulated tablet** (Android 14): the program starts, shows the first start's window and the welcome
  project, takes typed text; the window shrinks to the room above the on-screen keyboard; a long press selects a
  word with Android's handles and its Cut / Copy / Select all bubble; menus open by finger; Qt's own file window
  works.
- **(Guarded since trial build 18: see "Android: new windows with the keyboard at work ..." below; what follows is
  the fault as it was found.) Qt 6.12.0 ends the program when a window is first drawn while Android's input method is
  in a call into the program** (`qFatal("Failed to acquire deadlock protector for QAndroidPlatformOpenGLWindow::eglSurface()")`,
  qtbase `src/plugins/platforms/android/qandroidplatformopenglwindow.cpp`; the same line is in 6.10 and 6.11, 6.8
  returns without it). Every Qt window on Android is drawn through OpenGL, and its Android surface is made at the
  first drawing, which needs Qt's "deadlock protector"; a call of the input method (QAndroidInputContext's
  `runOnQtThread`) holds the same protector while it waits for the program's thread. Showing a window changes the
  focus, Qt tells Android (hide or restart the keyboard), the input method answers with calls, and when one of
  them is on its way at the first drawing, Qt aborts. Seen on the emulated tablet, always after a tap into the
  text (the input method active): File > Open Project (run 8), the File menu itself (runs 6, 10, 11). How often
  differs from run to run: in run 8 the File menu opened 9 of 9 times with the keyboard up, in run 11 it ended
  the program 3 of 3 times (also the first time, though the menu had been open before the tap into the text).
  Before the first tap into the text the menu opened in every run. Not known: a real tablet, a hardware keyboard.
  Two guards in the program were tried and taken out: answering the waiting calls and waiting a moment before the
  window is shown made it worse (run 9, 16 of 22 steps); keeping the protector from the show event to the first
  paint event helped only in part (run 10: menus 6 of 6, then 9 of 12 steps ended the program).
  `QtAndroidPrivate::acquireAndroidDeadlockProtector()` / `releaseAndroidDeadlockProtector()` are exported from Qt
  Core (qjnihelpers_p.h). Qt's branches `6.12` and `6.12.1` have this part rewritten (no protector in
  `eglSurface()`); 6.12.1 was not released on 2026-11-14.
- **Not done in A0** (the plan's later steps): the minimal interface, projects on shared storage with "All files
  access", saving when Android ends the app, crash reports, dictionaries in the APK, fonts, printing and PDF,
  licenses for the APK, tests on a device, the public APK with its permanent app id and key.

## Android: toolbar rows that fit, fingers (2026-11-14, the build after A0)

Reported from the trial build on tablets: the toolbars tend to run over the screen's edges; a third row that
arranges them could help, on small screens the Themes toolbar could be left out, and swiping and zooming with
fingers were asked for.

- **Where the code is.** `src/android/` (`Mobile.h`, `MobileBars.cpp`, `MobileTouch.cpp`): in none of the libraries;
  `target_sources` adds it to the program inside `if(ANDROID)`. `main.cpp` (inside `Q_OS_ANDROID`) calls
  `mobile::installTouch()` after `zoom::install()` and again after `w.show()` (the filter installed last is asked
  first: the second call puts it before the filters the window made), then `mobile::fitToolbars(&w)`. Both use only
  what the shared code offers from outside (`BarDock`'s public functions, the commands' actions by object name,
  Ctrl + wheel events): no shared file changed for them.
- **Toolbar rows** (`MobileBars.cpp`, class `BarFit`, a child of the window named "mobileBarFit"). A row of the top
  or bottom edge keeps its toolbars while they fit the window's width (`GripBar::naturalWidth()` + 2 each; a toolbar
  that is switched off needs nothing), the rest goes before the toolbars of the next row, and so on (a third row
  and more). More than three rows at the top: the Themes toolbar is switched off through its own switch
  (`toolbars::barCommand(Bar::Themes)`; `window/mobileThemesOffForRoom`) and on again when three rows are enough
  with it; a writer who switches it on while it is off for room keeps it at every width
  (`window/mobileThemesKept`); one the writer switched off stays off. It runs at the start, on the window's
  resize, when a toolbar is shown, hidden or changes its contents, and after `BarDock::placesChanged`. The rows the
  writer has (the standard ones, a drag, Reset Toolbar Places) are kept in `window/mobileBarRows` and are what a wider
  window gets back; `window/barRows` (the place keeper's) holds the rows as last fitted. A row that changed starts
  without free space between its toolbars. What the toolbars need on Linux with the Fusion style (px): Main 245,
  Themes 336, Search 393, Tools 259, Style and font 484, Character 327, Paragraph 300, icons 410: the second row
  alone needs 1,521, so a 1,472 px tablet has three rows.
- **How a touch reaches the program** (Qt 6.12 Widgets, read in Qt's source): Qt sends the touch to the window
  (application event filters see the `QTouchEvent` with the `QWidgetWindow` as receiver; a widget gets it only with
  `WA_AcceptTouchEvents`, which none of the program's has). Unaccepted, Qt makes mouse events of the first finger
  (`QGuiApplicationPrivate::processTouchEvent`); they name the touch screen as their device
  (`pointingDevice()->type() == TouchScreen`), which is how `MobileTouch.cpp` tells a finger from a mouse, a touch
  pad or a pen. Before a press at a new place Qt sends a move without buttons. Qt gives the keyboard focus in
  `QApplication::notify` before any filter sees the press (only for spontaneous events: the press this code sends
  later gives none, and needs none). Qt takes every press of a finger within 400 ms of the one before for the
  second half of a double click, wherever it is (it compares the place with the new touch's own).
- **Fingers** (`MobileTouch.cpp`, class `Fingers`, a child of the application named "mobileFingers"). A press of a
  finger inside something that scrolls (the viewport of a `QAbstractScrollArea` with a range, or any text, list or
  tree) waits (`Pending`). Moved 12 px within 300 ms: the nearest view that can scroll that way follows the finger
  (`Scroll`; a list one way at a time, other views both ways; a throw when the finger went up moving: the speed
  of its last 100 ms, none after 40 ms of rest, slowing by `exp(-t / 0.35 s)`); the widgets get neither press nor
  release, so no cursor is set and no keyboard comes. Nothing scrolls that way: in a text, a list or a tree
  nothing happens (`Swallow`), elsewhere it is a mouse drag (`Pass`: a slider on a page of controls). Gone up
  where it came down: the press that waited, then the release (a tap). Rested 300 ms: the press is sent and what
  follows is a mouse drag. A finger put on a running view stops it and clicks nothing. A double click is passed
  on only within 30 px of a tap that reached a widget. Lists and trees are set to scroll by pixels when they are
  shown; a `QPlainTextEdit` scrolls by lines. Two fingers (seen in the touches themselves): the ratio of their
  distances becomes whole notches of Ctrl + wheel for the widget that was between them (seven for twice as far
  apart, the same back), which `ui/Zoom.cpp` and the mind map's own zoom take; nothing of it is a click. A second
  finger beside one that already drags as a mouse starts no zoom. One log line per touch, category `qb.fingers`
  ("fingers: swipe, scrolled ...", "tap after ...", "rested ..., pressed", "two fingers, n steps of zoom"); the
  emulator script lists them in `emu-summary.txt`.
- **Times are the events' own** (`QMouseEvent::timestamp()`, when the touch happened), not when the program got to
  them; and when the 300 ms are over, the rest is only taken for one after 20 ms more, in which input that
  already waits is handled (timers come before input in Qt's loop). Without this a swipe was taken for a rest
  when the program was busy.
- **What Qt does not do on Android** (`qwidgettextcontrol.cpp`: the text control starts without
  `Qt::TextSelectableByMouse` there): a drag over a `QTextEdit` selects nothing and a double tap selects no word,
  mouse or finger. Selecting is Android's own: a long press selects the word and shows the handles
  (`QAndroidInputContext::longPress`, started by Java's gesture detector; it does not come through the filter and
  works as before, seen in run 14). The on-screen keyboard is asked for only by `QTextEdit::mouseReleaseEvent` and
  `keyReleaseEvent`.
- **Preview test** `tests/mobile_tests.cpp` (`qb_mobile_tests`, 44 checks; in `tests/CMakeLists.txt` inside
  `if(QB_MOBILE_PREVIEW)`, a variable no `option()` declares, so it is off unless cmake gets
  `-DQB_MOBILE_PREVIEW=ON`; in no test run of Linux or Windows and not in `tools/run-all-tests.sh`): the main
  window at 1472, 1280, 1178, 960, 920 and 808 px with the rows worked out a second time in the test, the Themes
  toolbar's part, the writer's own rows, Reset, a toolbar switched off; fingers with Qt's touch test events on
  the editor, the binder, a list, a plain text, a page of controls, a combo box. Pitfalls of such a test: every
  touch event must name every finger that is down with its place (`stationary()` of a new sequence object has
  the place 0,0); touches less than 470 ms apart become double clicks; a drag and drop started by a simulated
  finger blocks the test (`QDrag::exec`); `QB_MOBILE_TRACE=1` prints the mouse events the editor gets. The
  window's smallest size is 808 x 491 px.
- **Emulator steps** (`android-emulator.sh`). B: `adb shell wm size 1280x800`, `800x1280`, `reset` (the running
  app's window gets the new size; one unit per pixel on that tablet). F: raw touch events as root (`adb root`,
  `sendevent` on the device named "virtio_input_multi_touch_1", protocol with slots, places scaled to its maximum
  32767): two fingers apart on the text, swipes up and down on the then longer text, sideways, over the binder, two
  fingers together. `adb shell input swipe` is of no use: its first move came 428 ms after the finger came down
  (a resting finger, and the program said so), and after it the keyboard is up and later swipes land on it.
- **Run 14 on the emulated tablet** (Android 14): three rows at 1280 x 800, four rows without the Themes toolbar at
  800 x 1280, the two rows back at 1472 x 920; two fingers 8 steps in and 8 out (the picture as before); the
  swipes scrolled 193 px up and back with no keyboard; sideways and over the short binder nothing; taps, the File
  menu (also with the keyboard up, four times) and the long press as before; File > Open Project, Compile and
  Print Settings ended the program 3 of 3 times (Qt 6.12.0's abort, see above; in runs 12 and 13 it also hit the
  File menu, 7 of 7 steps, after the keyboard had come up early).
- **The result release as a draft** (run 14): `gh release create` left `android-ci` as a draft, which nothing reads
  by its tag (`releases/tags/android-ci`: 404). The workflow now runs `gh release edit android-ci --draft=false`
  after making it and at the end. A session cannot edit releases; it reads a draft through its id from the
  releases list (`gh api 'repos/.../releases?per_page=8'`).
- **Not done, not checked**: a real tablet (the writer's); the throw on the emulator (raw events are too slow for
  one; in the preview test it works); a drag and drop by a finger that rested; two fingers on the corkboard and
  the mind map; an 11-inch tablet held upright has 800 units and the window needs 808 (the right edge is cut by
  8); toolbar buttons are as small as on a desktop; the on-screen keyboard still covers a third to a half of the
  window (the plan's minimal interface).

## Android: new windows with the keyboard at work, a text's handles, lines between panes (2026-11-14, trial build 18)

Reported from build 15 on tablets: menus close as described; the text's handle is drawn in front of everything
(in the bookmarks it scrolls over the windows above, and it covers the text it may select); and the sizes of
the panes need a way to be changed with a finger (the widths of the binder, the editor and the inspector, the
inspector's parts and so on).

- **Where the code is.** `src/android/MobileWindows.cpp` (Android only: it needs `QNativeInterface::
  QAndroidApplication` and Qt's private headers, so `Qt6::GuiPrivate` is linked to the program inside
  `if(ANDROID)`; not in the preview test), `MobileHandles.cpp`, and the part "the line between two panes" of
  `MobileTouch.cpp`. `installTouch()` sets all of it up (`installHandles()`, on Android `installWindowGuard()`):
  `main.cpp` did not change. `Mobile.h` declares `installWindowGuard()` for every system (the header includes no Qt
  header, so `Q_OS_ANDROID` is not known in it: run 16 failed to compile for an `#ifdef` there).
- **The fault of Qt 6.12.0, in full** (read in Qt's source and seen in the logs: "locked by
  'QAndroidInputContext::runOnQtThread()'"). A window's Android surface is made at its first drawing:
  `QRhiBackingStore::flush` makes the window's drawing engine (`QPlatformBackingStore::createRhi`), and the engine's
  making (`QRhiGles2::create`) makes its context current **on the window** -> `QAndroidPlatformOpenGLWindow::
  eglSurface()` -> `createSurface()` and a wait for Android's own thread, for which it takes Qt's one lock
  (`QtAndroidPrivate::AndroidDeadlockProtector`, not reentrant) and `qFatal`s when the lock is taken. Every call of
  the keyboard program (`QtInputConnection` -> `runOnQtThread`) takes the same lock on Android's own thread and
  waits for the program's thread (BlockingQueuedConnection to the input context's `safeCall`). A new window changes
  the focus, Qt hides or restarts the keyboard, the keyboard program answers with calls, and the program's thread is
  busy painting the new window (about 200 ms on the emulator): the call waits, `eglSurface()` finds the lock taken.
- **The guard** (`WindowGuard`, a child of the application named "mobileWindowGuard", an application event filter).
  At a window's `QEvent::Expose` or `Paint` (receiver a `QWindow`, exposed, OpenGL surface, property
  `qbSurfaceMade` not set; Qt draws a window only after its expose event): (1) a context of the window's format
  is taken (`contextOf`: one `QOpenGLContext` per `QSurfaceFormat` is kept for all windows; a window that has a
  drawing engine already, after the app came back from out of sight, gives its engine's context:
  `QRhiGles2NativeHandles`); (2) a piece of code is run on Android's own thread
  (`QNativeInterface::QAndroidApplication::runOnAndroidMainThread`) that reports "standing" and waits for "go";
  until it stands, the program's thread answers the keyboard program's waiting calls
  (`QCoreApplication::sendPostedEvents(<input context>, QEvent::MetaCall)`, up to `kStandMs` = 1000 ms; seen: 4 to
  433 ms); (3) "go", and at once `context->makeCurrent(window)`: `eglSurface()` takes the free lock and makes the
  surface; Android's thread stands still `kParkMs` = 12 ms longer, then makes the surface, and calls of the
  keyboard program meanwhile find the lock taken and return at once (as Qt means them to). Afterwards
  `m_androidSurfaceCreated` is true and `eglSurface()` never takes the lock for that window again. The property is
  cleared when the platform surface is destroyed and for all windows when the app goes out of sight
  (`applicationState <= Hidden`: Qt destroys every surface then). The guard holds no lock: when Android's thread
  does not stand in time, the window is drawn as without the guard (the log says "DID NOT STAND STILL").
  **Not through the window's own engine from the start**: `createRhi` itself is the call that makes the surface
  (see above) and takes milliseconds (a context is made); it must happen after the surface exists. The kind of the
  surface (EGL configuration) comes from the first context's format, and Qt chooses a context's kind by its format
  alone (`QAndroidPlatformIntegration::createPlatformOpenGLContext` sets 8 bits per color for all): the kept
  context of `window->format()` gives the kind the engine's would have given, and the engine's later context fits
  it (the script counts `eglMakeCurrent failed` and the like: 0).
  **Take the file out with the Qt version that has the fault repaired** (Qt's branch 6.12: `eglSurface()` without
  the lock; 6.12.1 was not released on 2026-11-14).
- **Handles** (`Handles`, "mobileHandles"; Qt's public interface only). Qt's Android part shows the drop under the
  cursor and the two handles of a selection as windows of their own above everything
  (`QAndroidInputContext::updateSelectionHandles`, at every `cursorRectangleChanged`); it never asks whether the
  cursor can be seen, it shows the drop at every touch of the focused text (`touchDown`), and every cursor movement
  gives it 4 more seconds. What the filter does: it answers Qt's `QInputMethodQueryEvent` for `Qt::ImHints` (sent
  to the focus widget; the widget answers first, re-sent under a guard) with `Qt::ImhNoTextHandles` added while
  the handles should stay away, worked out at that moment from the widget's own answers (cursor and anchor
  rectangle, `Qt::ImInputItemClipRectangle` = the part of the widget that can be seen): no selection: the cursor
  out of sight, or no tap lately; a selection: both ends out of sight. Qt then hides the handles but keeps its own
  state, so they come back when the hint is gone. The drop under the cursor is allowed by a finger's press that
  reached the focused text (a tap, or a finger that rests), for `kCursorHandleMs` = 3000 ms (started again by
  every cursor movement, so the drop can be dragged), and sent away by typing, a key, a swipe or two fingers
  (`handlesAway()` from `MobileTouch.cpp`), a change of focus. **Typing** is told from the keyboard program's
  housekeeping by the event's shape (`m_marked` = the last preedit): Android's keyboards mark the word at the
  cursor about a second after a tap (preedit = the word, replacing itself) and hand it back unchanged when the
  cursor leaves it (commit = the preedit): neither is typing (run 17 took the mark for typing). After a change of
  the allowance Qt is nudged with `QInputMethod::update(Qt::ImCursorRectangle)`. **The text's place**: when a
  widget the focus widget stands in moves or changes size (the main text is moved as a whole inside
  `TextColumnView`), `QInputMethod::setInputItemTransform` is called as `QWidgetPrivate::updateWidgetTransform`
  does for the widget's own moves (a 0 ms timer, once per turn of the loop): the handles follow the text.
- **Lines between panes** (`MobileTouch.cpp`). A press of a finger on or within `kDividerReach` = 18 px beside a
  visible `QSplitterHandle` of the touched window (`dividerNear`: the nearest; `findChildren` at each press)
  waits like a press in a list (also one on a widget that otherwise gets its press at once: `m_plain`). The first
  move decides: across the line (`across`: sideways for a standing line) -> mode `Divider`: the handle gets a
  press at the finger's first place, the moves and the release as mouse events (`toDivider`; the place inside the
  handle may lie beside it, `QSplitterHandle` goes by the screen place, so the line does not jump); along the
  line: as without the line (a swipe scrolls; a widget that would have got the press at once gets it now and the
  finger drags as a mouse, e.g. a scroll bar); a tap goes to what is under the finger; a finger that rested moves
  the line too. No handle is made wider (a splitter's saved state holds the handle width).
- **Preview test** `qb_mobile_tests`: 66 checks (22 new: the hints after a tap, a marked word, a typed letter, a
  key, a swipe, three seconds, a selection scrolled out and back, the place told to Qt; lines moved from both
  sides, along the line, a tap beside it, after a rest, 60 px away, a lying line, the binder's edge in the
  program's window). A `QInputMethodEvent` sent to a `QTextEdit` in a test changes its text: mark exactly the
  word at the cursor and hand it back.
- **Emulator steps** (`android-emulator.sh`): D1 to D6 (the binder's edge, the inspector's edge, the line under
  the synopsis, each moved and back, by raw touch events; the log line "the line between two panes moved by n
  px"), S7 (every menu of the menu bar once with the keyboard up), H1 to H5 (pictures: the drop after a tap, gone
  after a letter, a selection's handles, none while the word is scrolled out of the pane, back with the word),
  S6 as before. The summary lists "new windows: surfaces made ahead ..." and the handle's log lines. A menu is
  closed with a tap at 1050 x 300, right of where the menus reach (run 17: a tap at 700 x 330 chose Help > System
  Information, and that window covered all later steps).
- **Run 18 on the emulated tablet** (Android 14, commit 18c71d5): the program ended 0 times (runs 12 to 14: at
  every menu or window opened with the keyboard up); 16 windows opened with the keyboard at work (the File menu
  three times, nine other menus, a long press and the menu, Open Project, Compile, Print Settings), 21 surfaces
  made ahead, Android's thread stood still every time (after 4 to 433 ms), no context that did not fit; six lines
  moved; the pictures of the handles as described.
- **Not done, not checked**: a real tablet (the writer's); how the handles look at a tablet's pixel density (the
  emulator has one unit per pixel); the size and look of Android's handles (they come from the app's Android
  theme, `textSelectHandle...`; not changed); a selection of which only one end is in sight keeps both handles;
  the edges of table columns (outliner, header views) are as thin as on a desktop; no sign shows where a line
  between panes can be taken; the guard with a hardware keyboard or another keyboard app; windows that are no
  widget's (none in the program).
