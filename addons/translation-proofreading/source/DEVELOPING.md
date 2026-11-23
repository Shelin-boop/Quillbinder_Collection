# Translation and Proofreading (a Quillbinder add-on): developer notes

A writing-tool add-on for Quillbinder. It began on 2026-11-15 in a repository of its own, written from Quillbinder's
public source alone, and moved into Quillbinder's repository with Quillbinder 0.92.0 (version 1.2 of the add-on). It
is still a project of its own in this folder: its own `CMakeLists.txt`, its own checks, and a copy of the tool
interface it was written against (`third_party/quillbinder`). Quillbinder's build packs it through
`addons/translation-proofreading/stage.sh` (the folder above), with the engine libraries of
`tools/get-translator-deps.sh`; the release workflow builds, signs and publishes it with the other add-ons, and it
is part of the full version. The models are on the releases `models-1` and `models-2` of Quillbinder's repository.
The Windows and Android packages of 1.1 were built from the former repository and are rebuilt here when those
systems are released from this repository.

## Facts the add-on depends on (from the public Quillbinder source)

- Tool interface 1.1: `third_party/quillbinder/ToolApi.h` (copied unchanged; see its README for the commit). Never
  change it; a new Quillbinder interface version would be copied in as a whole.
- Package format and install: Quillbinder's `docs/developer-guide.md`, sections 2, 3, 5, 6 (`packaging/make-package.py`
  writes it). Add-ons with Linux libraries carry `runtime` = `u2404-qt6.12-r1` and `minApp` >= `0.80.0`.
- Quillbinder's AppImage brings Qt 6.12.0 (Core, Gui, Widgets, Network ...) but no TLS plugin: the add-on brings
  `qtplugins/tls/libqopensslbackend.so` of the same Qt (the Speech Transcription add-on does the same).
- `ToolHost::contentDirs("translation-proofreading")` is the add-on's folder (engine, libgomp, icon, TLS plugin);
  `ToolHost::dataDir` keeps the models (`models/<id>/`, with `checked.txt` once every file's SHA-256 matched).
- A command's icon is a theme name only; the add-on's own icon goes on via `setCommandState` (1.1), which also makes
  the action checkable (the window command is "on" while the window is open).
- The plugin must not show symbols the program also has (2026-11-15: both had `qInitResources_resources`, and the
  add-on's catalog was not found in the real program): hidden visibility, `-Bsymbolic`, a resource file with a unique
  name, and `tests/check-exports.sh`.

## Build and checks

    packaging/build-deps.sh                     # once: oneDNN 3.9.1, CTranslate2 4.9.0, SentencePiece 0.2.1
    cmake -S . -B build -G Ninja -DTP_TEST_SWITCHES=ON [-DCMAKE_PREFIX_PATH=/opt/qt/6.12.1/gcc_64]
    cmake --build build
    TP_MODELS_DIR=<files of the release models-1> QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure

Screenshots of the checks go to `build/screenshots` (`TP_SHOTS`): look at them after UI changes. The cloud session has
no Qt 6.12 (download.qt.io is not reachable); Ubuntu's Qt 6.4 is fine for development (a plugin built with an older Qt
loads in a newer one), but releases come only from the workflow "Build" (Qt 6.12.0 via aqtinstall).

## Releases

(Since 1.2 the add-on is released by Quillbinder's release workflow, with Quillbinder. What follows describes 1.0 and
1.1, which were released from the add-on's former repository.)

- `packaging/manifest.json` and `TP_VERSION` in CMakeLists.txt carry the version (make-addon.sh refuses a mismatch).
- A push to a work branch puts the package on the pre-release `test`; a push to `main` publishes `v<version>` once.
  Work on branches and merge to `main` when the writer agrees.
- Models: `models/convert.py` and the workflow "Models" make a model release (`models-1` for 1.0, `models-2` for 1.1)
  and the catalog of all of them (artifact `catalog`, copied into `resources/catalog.json`; each model names its
  release; the add-on checks downloads against it). A published model release is never changed: new or changed models
  get a new release name. A file over 1.9 GB goes up in parts (`...--model.bin.part1`), joined by the add-on.
- The official add-on (signed with the Quillbinder project's add-on key) is built by the workflow "Official add-on" of
  Quillbinder_Collection from the commit named in its `official-addons/translation-proofreading.json`; a new version needs
  a new commit there.

## Windows (2026-11-16, 1.1 for Windows)

- From the public source: Quillbinder for Windows takes add-ons with `"platform": "windows-x64"` (file name
  `<id>-windows.qbpkg`), DLLs without "lib", and brings Qt's TLS plugin itself (Schannel); minApp 0.91.1 (the first
  public Windows version; 0.91.3 knows the add-on key). `packaging/make-addon-windows.sh` writes such a package.
- Every Windows change sits in `#ifdef Q_OS_WIN` / `if(WIN32)`, so the Linux build stays as it was (the Linux 1.1
  package came from 59537a5). `packaging/build-deps.sh` is the Linux recipe and the key of the Linux caches: leave it.
- `packaging/build-deps-windows.sh`: MSVC, static libraries with the DLL runtime (/MD). CTranslate2 4.9.0 forces /MT for
  a static build (one line changed by the script), CMake 4 needs `CMAKE_POLICY_VERSION_MINIMUM=3.5` (cpu_features),
  and CMake on Windows starts the cache with Debug: always `-DCMAKE_BUILD_TYPE=Release`.
- The engine is `qb-tp-engine.dll` with MSVC's OpenMP (`vcomp140.dll`) beside it, loaded with
  `LOAD_WITH_ALTERED_SEARCH_PATH`. The memory guard reads `GlobalMemoryStatusEx`. Windows gives a new thread no
  priority of the one that starts it: only the tool's own thread runs below normal there.
- Paths with letters outside ASCII (C:\Users\Jürgen): CTranslate2 4.9 opens UTF-8 paths itself; SentencePiece takes the
  system's code page, so on Windows the engine hands it the file's bytes (`LoadFromSerializedProto`). The checks run
  with TEMP in "Prüfung Jürgen ß", and set paths in environment variables with `_wputenv_s` (qputenv's bytes are
  taken in the code page there).
- `.gitattributes` keeps LF on Windows: the word lists and the catalog are read line by line. `TP_VERSION` is also a type
  of winnt.h: `windows.h` is included with the macro pushed away. Windows deletes no open file (close before removing).
  The check programs' output did not reach CTest on Windows: they write `build/test-*.txt`, which the workflow prints.
- The Windows job of "Build" puts the package on the pre-release `test-windows`; on main it publishes `v<version>-windows`
  once (never Latest). The official Windows add-on is built by Quillbinder_Collection's workflow "Official add-on for
  Windows" from the `windows` block of the entry.

## Windows 1.2: one of Quillbinder's own add-ons (2026-11-20)

- Since Quillbinder for Windows 0.92.0 the Windows package is built with Quillbinder, as the Linux one since 0.92.0:
  `packaging/windows/win-ci.sh` builds the engine libraries with `packaging/build-deps-windows.sh` into
  `.translator-deps` (`TP_DEPS`; the workflows keep the folder in a cache under that script's hash, without the build
  folders `b-*`), and `packaging/windows/win-addons.sh` configures this project as a release build
  (`-DTP_DEPS_DIR=<deps>/install -DTP_TEST_SWITCHES=OFF`), runs `tests/check-exports-windows.sh` and
  `packaging/make-addon-windows.sh`. The package is `translation-proofreading-windows.qbpkg`; it goes into the full
  version's `full-addons\` and into the Windows release, signed with the add-on key like the project's other add-ons.
- The add-on's own checks (`ctest`, which needs the language models) are not part of Quillbinder's Windows runs, as
  on Linux. They ran for 1.1 on Windows on 2026-11-16; the code is unchanged since but for the version and the
  models' address.
- The entry `official-addons/translation-proofreading.json` and the workflow "Official add-on for Windows" describe
  1.1 from the add-on's former repository; they are no longer the way the Windows package is made.

## Android (2026-11-16, branch `addon/android`, not merged)

- From Quillbinder's Android build (private trial builds, 0.91.3): add-ons with `"platform": "android-arm64"` (tablets)
  or `"android-x86_64"` (emulated tablets), runtime `android-qt6.12-r1`, minApp 0.91.3; Quillbinder for Android keeps
  them in the app's private folder (Android runs no library from the shared storage) and refuses another system's or
  processor's package with its reason. `packaging/make-addon-android.sh <build> <out> <abi> <ndk>` writes the package.
- Every Android change sits in `if(ANDROID)` / `#ifdef Q_OS_ANDROID`; the Linux and Windows builds stay as they were.
- `packaging/build-deps-android.sh`: CTranslate2 4.9.0 with Ruy (oneDNN is for x86 only), SentencePiece 0.2.1, with the
  NDK (r27c, API 28), 16 KB pages. The engine links LLVM's OpenMP statically (`-static-openmp`; CMake's OpenMP target
  named `libomp.so`, which Android does not have: its link libraries are cleared). Android's toolchain looks only in
  the NDK's system folder: `CMAKE_FIND_ROOT_PATH_MODE_INCLUDE/LIBRARY BOTH`. The plugin keeps a plain `.so` name
  (`SUFFIX ".so"`) and may need Qt's graphics libraries (libGLESv2.so is the system's).
- The Android job of "Build" builds both ABIs, checks the exports, the libraries each needs and the 16 KB pages, and puts
  `translation-proofreading-<version>-android-<abi>.qbpkg` and `SHA256SUMS-android.txt` on the pre-release
  `test-android`; on `main` it publishes nothing yet (no release for Android before Quillbinder for Android is public).
- Seen working (2026-11-16, Quillbinder's system test on an emulated tablet, Android 14, x86_64): installed, loaded at
  the next start, its Options section, a model taken from a folder, "Das Haus ist alt und das Wetter ist schön."
  translated to "The house is old and the weather is nice." in 41 s (the first run, loading included). Not tried: a
  real tablet (arm64), the speed there, the proofreading models.

## Things learned (2026-11-15, 1.1)

- Chat models (Qwen3): CTranslate2's `Generator`, the prompt in ChatML (`<|im_start|>system ...`), the system message as
  `static_prompt` (its state is cached, so live work pays for it once); Qwen3 1.7B needs an empty `<think>\n\n</think>`
  after the assistant's start (thinkTags), Qwen3-4B-Instruct-2507 none. `src/engine/Bpe.cpp` is the byte-level BPE of
  their tokenizer.json; the logic checks compare it with the model's own tokenizer (`tokenizer-check.json`, made by
  `models/convert.py`): the same pieces for every sample, the whole chat prompt included.
- `max_length` of a generation with `return_alternatives` counts from the start (static prompt and prompt included);
  without alternatives it counts the new pieces only.
- Both slots share one loaded model (a Qwen model chosen for proofreading and translation is loaded once).
- The memory guard: `memoryProblem` (catalog `memoryMB` against /proc/meminfo); the checks fake the computer with
  `TP_TEST_MEMINFO_MB=total,available` and a catalog with `TP_TEST_CATALOG` (test builds only).
- `keepDialogue` (Logic.cpp) puts the writer's quotation marks back after any model; then the curly-quote rule turns
  them from their places. The UI checks switch live work off (setting `live`) except in `liveTyping`.
- A click in the result waits for the double-click time (a double click shows the word in the information area, not
  the menu); links (the suggestions under a single word) come through `QTextBrowser::anchorClicked`.

## Things learned (2026-11-15)

- Beam 2 let CoEdIT Large turn "bellow" into "downstairs"; the default is beam 4 ("Careful": 6). With oneDNN's
  numbers it still says "downstairs" there (Python's build with Intel MKL said "below"); the checks accept either.
- CTranslate2 goes wrong when it continues straight from an alternative (Marian models): other words are found in two
  steps (one decoding step for the candidates, then each sentence finished on its own).
- Curly quotes in the input made CoEdIT double a word; the models get plain quotes, and the rules put curly ones back.
- oneDNN's assembler stub has no stack note: link with `-z noexecstack`, else newer systems refuse the engine.
