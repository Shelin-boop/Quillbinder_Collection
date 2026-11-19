# Runtime (Qt, FFmpeg), add-ons, tool interface, release files

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## 0.91.2: the download page per system, a compiler cache for the sessions (2026-11-15)

- **The download page takes each system's newest files.** Since the last number is counted per system (working rules,
  section 8), a Linux release can come without the Windows files of its number. `tools/download-page.py` takes
  `--also <folder> <tag>` (repeated, newest first): a system the release has no file for is taken from the first
  older release that has one, and its card names that version ("64-bit, version 0.91.1 beta"). The job
  `download-page` of `release.yml` passes every older `v*` release (`releases?per_page=12`). The old call (folder, tag,
  out) is unchanged; `release-windows.yml` still uses it (its own release always has its Windows files). `qb_v0902_tests`
  checks both. The README names each system's version and links Windows' own release.
- `tools/publish-release.sh` uploads every file of the sources folder but `SOURCES.txt` to the release
  `third-party-sources` (before: only `*.src.tar`, so Qt's `qt-everywhere-src-<version>.tar.xz` was left out).
- **The compiler cache for the sessions (P5, a trial).** `.github/workflows/ccache.yml` compiles the test build
  (`-G Ninja -DQB_TEST_SWITCHES=ON`, as in a session) and the AppImage's build (the flags of `build-appimage.sh`; a
  program update's build shares its objects: `CMAKE_INSTALL_PREFIX` is in no compile flag) through ccache, in a clean
  Ubuntu 24.04 container and in the session's folder `/home/claude/quillbinder_collection` (the test programs carry the absolute
  build and source folders as definitions, so another folder would never hit), with FFmpeg and whisper.cpp from their
  packs, starting from the last pack. It runs when what is compiled changes on `main`, on a `linux/` branch only with
  `[ccache]` in the commit message, and by hand; only its job `publish` writes the pre-release `ccache-linux`
  (`ccache-linux.tar`, its `.sha256`, `ccache-linux.txt` with the commit and ccache's figures) and then deletes the
  run's artifact. `tools/get-ccache.sh` (once per session, after `cloud-setup.sh`, **before the first configure**)
  installs ccache, writes `~/.config/ccache/ccache.conf` (`~/.cache/ccache`, 2 GB, `compiler_check = %compiler% -v`:
  the same Ubuntu compiler package on another machine gives hits), unpacks the pack and leaves
  `~/.config/ccache/quillbinder`, by which `CMakeLists.txt` sets `CMAKE_C/CXX_COMPILER_LAUNCHER` to ccache for every
  build of the tree (also those `build-appimage.sh` and `build-update.sh` configure). A build folder configured before
  that is compiled anew once (the command line changed). Get FFmpeg and whisper.cpp before the first configure, as the
  workflow does, or the media and speech plugins miss. Nothing changes where the file is not there (the public release
  workflow sets its own launcher).

## Build optimization, the Linux part (2026-11-15; no version of its own)

The working rules' section 10 (decisions D1 and D3) and the project document `claude/quillbinder-build-optimization.md`
(proposals P1, P2, P6, P7) say why. Nothing in the program changed.

- **Dependency packs (P1).** `tools/deps-pack.sh` makes a recipe for FFmpeg and for whisper.cpp: the build scripts
  (`get-ffmpeg.sh`, `build-ffmpeg.sh`, the patches; `get-whisper.sh`) by their SHA-256, the Ubuntu release, the
  processor type and the versions of gcc, nasm (FFmpeg), cmake, ninja and the Vulkan tools (whisper.cpp), and a
  format number of the pack's layout (`FORMAT` in the script: raise it when `pack` or `fetch` change). The recipe's
  SHA-256 names the pack. `.github/workflows/deps.yml` builds a pack in a clean Ubuntu 24.04 container
  (`QB_SKIP_QT=1 tools/cloud-setup.sh`, then the get-script with `QB_DEPS_PACK=0`) when a recipe file changes on
  `main` or a `linux/` branch (or by hand), and only its job `publish` (no checkout, no build) creates the pre-release
  `deps-<name>-linux-<first 16 of the hash>` ("not for downloading") with the `.tar.xz`, its `.sha256`, the recipe,
  the SHA-256 of every library and the build log; the release's text carries the line `pin: ...`.
  `tools/deps-packs.txt` pins each pack by its SHA-256, as `get-qt.sh` pins Qt: put the `pin` line there in
  a commit of its own. `get-ffmpeg.sh` and `get-whisper.sh` call `deps-pack.sh fetch` first: no pin for this recipe,
  no `gh`, a missing file, a failed download or another SHA-256 -> they build from source as before.
  `QB_DEPS_PACK=0` never downloads. First packs (2026-11-15, runs 43683385582 and 43685668664): FFmpeg 10.7 MB (recipe 21da73da...),
  whisper.cpp 11.9 MB (c9bcf42a..., with the test models); in the session each was fetched and unpacked in about 2 s (built from source on
  GitHub's runner: 10 and 11 min). whisper.cpp's pack came out byte for byte the same in two runs.
  **Every edit of a recipe file, a comment too, is a new recipe**: deps.yml then builds a new pack (10 to 11 runner
  minutes) and the session builds from source until the new `pin` line is in `tools/deps-packs.txt`; leave those
  files alone unless the build itself has to change. A pack brings only the finished folder, not the source folders
  (`build/ffmpeg-src`, `build/whisper-src`): what the tests need from whisper.cpp's source is copied into the folder
  by `get-whisper.sh` (`samples/jfk.wav`, `test-models/for-tests-ggml-{tiny,small,medium}.bin`, the empty models of
  `qb_v061_tests` and the system test). The pack is unpacked into the prefix asked for; the few `.pc` and `.cmake` files
  that name the build machine's prefix get the new one (the libraries find each other by `$ORIGIN` and sonames).
- **The release workflow (P2, D3).** `release.yml` keeps `build/ffmpeg`, `build/ffmpeg-full` and `build/whisper`
  in GitHub's cache under `deps-linux-<name>-<recipe hash>` (a new recipe is a new key, so it is built anew by itself);
  only the job `build` writes them, only on `main`, only after building them itself; `tests` only reads them; the
  signing jobs touch no cache. The release workflow never downloads packs (`QB_DEPS_PACK=0`: the audits
  want its third-party code built from pinned sources). Each run logs the SHA-256 of the
  FFmpeg and whisper.cpp libraries it used and whether they came from the cache. ccache (`/tmp/ccache`, 500 MB) for the
  AppImage build (`build`) and the test build (`tests`), each its own cache, written only on `main`. The input
  `fresh` of Run workflow (and every main-number release, x.0.0, by itself) uses no cache.
- **Resources once (P6).** `CMakeLists.txt` compiles `QB_RESOURCES_QRC` once into the object library `qb_resources`;
  the program (not the Android one, which keeps its own) and every test program take `${QB_RESOURCES_OBJECTS}`
  (`$<TARGET_OBJECTS:qb_resources>`) instead of their own copy. A new test program writes `${QB_RESOURCES_OBJECTS}`.
- **The download page's rerun (P7).** `upload-pages-artifact` and `deploy-pages` use the artifact name
  `github-pages-<attempt>`, so "Re-run failed jobs" works for `download-page` without deleting artifacts by hand.

## The runtime step: Qt 6.12, FFmpeg 9.0.2, fuzz tests, Apache-2.0 (0.80.0 beta)

- **Qt**: The Qt Company's own build of Qt 6.12.0 for Linux (aqtinstall 3.3.0, modules qtmultimedia qtimageformats
  qtpdf). The cloud session cannot reach download.qt.io: `.github/workflows/qt-linux.yml`
  installs it on GitHub's runner for a pushed branch `qt/<version>` and puts a tarball on
  the pre-release `qt-<version>-linux`; `tools/get-qt.sh` downloads it (SHA-256 pinned in the script) or, where
  download.qt.io answers (GitHub's runners, the release workflow), installs with aqtinstall. A new Qt version:
  push `qt/<version>`, pin the new sum, change `QB_QT_VERSION`, `APP_RUNTIME`, `APP_RUNTIME_SINCE`, `APP_RUNTIME_DATE`.
  Qt 6.12's Wayland plugin is one file (`libqwayland.so`); its FFmpeg backend links Qt Quick/QML/OpenGL (the media
  add-on carries them in `media/lib`, found through the plugin's RUNPATH `$ORIGIN/../../lib`); there is no GStreamer
  backend (the choice is listed as not available, a saved one falls back to FFmpeg).
- **Licenses**: `tools/get-qt-licenses.sh` (sparse GitHub checkouts of qtbase, qtsvg, qtimageformats, qtmultimedia at
  `v<version>`, qtwebengine-chromium at the commit of qtwebengine's tag or branch, ICU 73.2) then
  `tools/collect-licenses.py --qt-licenses build/qt-licenses <AppDir> resources/licenses` writes the list: Ubuntu
  packages as before, plus "Qt" (files of `/opt/qt`, `qtBuild`), "ICU (Qt's build)", every part of Qt's attribution
  files but those in `QT_LEFT_OUT`, and Qt PDF's parts. Source: `qt-everywhere-src-<version>.tar.xz` (the Qt
  Project's archive, `tools/collect-sources.py`, kind "qt").
- **FFmpeg 9.0.2** (sonames avcodec/avformat 63, avutil 61, swresample 7, swscale 10: what Qt 6.12's plugin needs),
  the original archive from Ubuntu's mirror (`tools/get-ffmpeg.sh`); `--disable-postproc` only where configure has it.
- **Fuzz tests**: `qb_fuzz_tests` (11 checks, about 10 s) and the libFuzzer harness `tests/fuzz/` (`-DQB_FUZZ=ON`, clang).
- **PDF metadata**: Qt 6.12's QPdfWriter writes an empty /Author itself; `addPdfNavigationEx` replaces existing
  entries of the keys it sets.
- **Qt 6.12 pitfalls** (found by the full run): QDom's default invalid-data policy is ReturnNullNode (a text node with
  a control character was left out; `TextUtil.cpp` sets AcceptInvalidChars before `main()`, `xmlSafeBytes` cleans the
  output); `QString::arg(char16_t)` writes the character, not a number (cast with `int(...)`); a dialog whose parent
  is a `Qt::Tool` window gets the main window as transient parent (`InputWatch::aboveToolWindow` sets the tool
  window at Show); `ItemsMimeData::retrieveData` must answer `QList<QUrl>`; line heights differ from Qt 6.4. Qt Multimedia 6.12 tries
  PipeWire first and crashes when libpipewire is there without `client.conf` (this machine): `core/AudioBackend.h`
  (`audiobackend::avoidUnconfiguredPipeWire`, called by both add-on engines) sets `QT_AUDIO_BACKEND=pulseaudio` then;
  a file is read in the background (`QtPlayer::m_playWhenRead`). Qt 6.12's PulseAudio input keeps at most 1024 frames
  in the server: v061 feeds its pipe source in 20 ms pieces (100 ms pieces lost two thirds of the sound).

## Tool interface 1.1, Speech Transcription (0.61)

- `src/tools/ToolApi.h` 1.1 adds to 1.0 without changing it: three functions at the end of `ToolHost` (a tool calls them
  only when `apiVersion() >= 110`), and the plugin-side interface `ToolExtension` (own IID; `toolhost::Host::extension()`
  asks the plugin object): inspector tabs (`Inspector::addExtraTab`, after Objects in the stack, buttons at the left of the
  row, kept in the state by key `toolTab`), toolbar buttons (`toolbars::setToolButtons`: part of the standard set; a
  toolbar the writer arranged gets each once, setting `toolbars/toolButtonsPlaced`; the arrow's menu set in
  `MainWindow::placeToolButtons` after every rebuild), a section in File > Options (`optionsPage()` "Section",
  `OptionsDialog::showToolSection`). Never change a 1.0 struct or function: the program reads tools built long ago.
- `plugins/tools/speech-transcription`: `libqb-speech.so` (the tool: commands, `TranscriptPanel`, the options, models and
  downloads with the checks) and `libqb-speech-impl.so` (`impl/SpeechEngine.cpp`: Qt Multimedia's `QAudioSource`,
  `SpeechLogic`'s `Segmenter`, whisper.cpp in a thread; `SpeechApi.h` between the two). `SpeechLogic.*` is Qt Core only
  (the tests compile it). whisper.cpp: `tools/get-whisper.sh` (v1.9.5, shared libraries, `GGML_BACKEND_DL` with all CPU
  variants and Vulkan; about 7 minutes on 2 cores) into `build/whisper`; the engine loads the compute modules from its
  `lib/` with `ggml_backend_load_all_from_path`. Translation: main language English and a fallback: every piece runs
  with `translate` (whisper translates only into English); the language of a piece is found first only for Careful,
  "keep the original" or another main language (an extra encoder run). Models: `SpeechLogic.cpp`'s catalog (SHA-1 of
  whisper.cpp's list; Small's and Medium's checked against the files on Hugging Face on 2026-11-08), saved in
  `<data>/tools/speech-transcription/models`; `QB_SPEECH_SOURCE` points the official source elsewhere (tests).
- The add-on brings Qt Multimedia (only `libQt6Multimedia.so.6`: the microphone needs no Qt Multimedia plugin) and Qt's TLS
  plugin; `addons/speech-transcription/stage.sh` exits with 3 when whisper is not built, and `packaging/linux/build-addons.sh`
  then leaves the add-on out. Icons of the add-on are in the program's icon set (Breeze: microphone, record, list).

## Release, sources, media formats (0.50)

- Project page: `APP_HOMEPAGE` in CMakeLists.txt (`QB_APP_HOMEPAGE`, `QB_SOURCES_URL` in AppInfo.h). The release
  `third-party-sources` there holds one `.src.tar` per third-party part (`tools/collect-sources.py`); every entry of
  `resources/licenses/index.json` names its file (`sourceFile`, `source`), and the add-ons' `stage.sh` write license
  notes that name theirs. `tools/collect-licenses.py --refresh` (run by `build-appimage.sh`) brings the list to the
  package versions of the build system and the runtime's commit; `--check` fails when the AppDir differs.
- Media: `tools/build-ffmpeg.sh <src> <prefix> base|full`; `tools/get-ffmpeg.sh` builds both (`build/ffmpeg`,
  `build/ffmpeg-full`). "Media Playback" 1.1 carries "base" (free formats only, list in the script); the add-on
  `media-extra-formats` ("Extra Video and Audio Formats") carries "full" in `media-extra/lib`, which
  `MediaEngine::preload` loads instead when it is installed and not disabled (`<addons>/state.json`). The player
  (`impl/QtPlayer.cpp`) looks at a file's streams with FFmpeg first and shows which formats need the extra add-on.
  `qb_m11_tests` installs both add-ons; `qb_v050_tests` checks the split (child runs: base, extra, off, removed).
- Releases are built and published by `.github/workflows/release.yml` in this repository only (the job
  `version` has `if: github.repository == 'Shelin-boop/Quillbinder_Collection'`).
