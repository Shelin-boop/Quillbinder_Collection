# Runtime (Qt, FFmpeg), add-ons, tool interface, release files

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## 0.99.0: the packs crypto, onnxruntime and aiengine (Linux)

Three more dependency packs (`tools/deps-pack.sh`, `.github/workflows/deps.yml`, the pattern of the 0.93.0 packs); each recipe
is its `get-*.sh` script (a hash over it and the build machine), each part pinned by tag **and commit** (the script stops when
a tag points elsewhere), none changed except where said here.

- **crypto** (`tools/get-crypto.sh`, prefix `build/crypto`): Botan 3.14.1 (minimized, static, `-fPIC`), json-c 0.19, bzip2
  1.0.8 (the libarchive project's GitHub mirror of it, tag `bzip2-1.0.8`), zlib 1.4, RNP v0.18.1 with its submodule sexpp
  (fetched by its commit), all static, with the license texts in `licenses/`. `tools/check-crypto.sh` builds
  `tests/crypto_pack_check.cpp` against it (a key made in memory, a buffer encrypted with bzip2 compression and decrypted
  again) and links the libraries into a shared object too; `ldd` shows only libstdc++, libm, libgcc_s and libc. Pitfalls found:
  RNP's module list (`ci/botan3-modules`) names `curve25519`, which Botan 3.14 made internal: the public module is `x25519`;
  RNP asks for the `pcurves_*` modules of the curves since Botan 3.7 (here the six NIST/secp ones, Brainpool and SM2);
  RNP 0.18.1's `crypto/mem.cpp` uses `strlen` without `<cstring>` (the build adds `-include cstring`, the source stays as it
  was tagged); RNP has **no switch for bzip2** (`find_package(BZip2 REQUIRED)`), so bzip2 stays; Ubuntu's `libz.a` is **not**
  position-independent (it cannot go into a shared object, only into a PIE program), so zlib is built in the pack;
  RNP 0.18.0 is withdrawn and 0.18.2 was not tagged on 2026-11-27, so 0.18.1.
- **CTranslate2 4.9.1** (translator recipe, Linux only, the user's decision; fixes #2159, validation of token ids that disable logits);
  Windows and Android recipes stay at 4.9.0. sentencepiece stays at 0.2.1: 0.2.2 and 0.2.3 need abseil-cpp built separately.
- **onnxruntime** (`tools/get-onnxruntime.sh`, prefix `build/onnxruntime`): Microsoft's prebuilt `onnxruntime-linux-x64-1.32.0.tgz`.
  The session that made the script could not download it (the repository of microsoft/onnxruntime is closed to it; the release
  page answers 403/404), so a pack is needed: the workflow downloads it. The SHA-256 is **not pinned yet**: while `SHA256` in the
  script is empty the download is accepted only with `QB_ONNXRUNTIME_FIRST_RUN=1` (set by deps.yml), and the script prints
  `pin this: SHA256="..."`, which the lead puts into the script (the recipe's hash changes, the pack is made again in seconds).
  The first run does not guess the asset name (the guessed `v1.32.0` URL answered 404): it asks the GitHub API for the releases,
  takes the newest published one with an `onnxruntime-linux-x64-<version>.tgz` (not GPU/CUDA), prints the releases and assets it
  saw, the archive's top-level contents and `pin this: VERSION=... TAG=... ASSET=... SHA256=...`; the pack is made from what is
  really in the archive (the folder with `onnxruntime_c_api.h`, the `libonnxruntime.so.*` files, LICENSE/notices if present).
  The program never links the library (`dlopen` + `OrtGetApiBase`).
- **aiengine** (`tools/get-ai-engine.sh`, prefix `build/ai-engine`, `tools/ai-engine/qwentts/CMakeLists.txt`): ggml 1.1.1,
  whisper.cpp 1.9.6, llama.cpp b12808 (libllama and libmtmd), qwentts.cpp and Sonic at pinned commits, on **one** ggml (installed
  first, the others use it as a system library), shared libraries with `RPATH=$ORIGIN`, ggml with `GGML_BACKEND_DL`,
  `GGML_CPU_ALL_VARIANTS` and the Vulkan module as its own library. The only change to a source: `ggml.h` gets
  `GGML_MAX_NAME 128` instead of 64 (qwentts.cpp's audio tokenizer has longer tensor names; the size is part of the layout of a
  tensor, so the installed header carries it and every consumer must include that header, not another copy).
  qwentts.cpp's own CMake file is not used (it builds a fork of ggml as a submodule, a server and tools): the wrapper builds
  its three source files as `libqwentts.so`; its ggml fork differs from ggml 1.0.0 by one CPU speed-up (a fused snake activation)
  and one Metal kernel, nothing the source calls by name.
  Proven in the session (no Vulkan: its shaders need a runner with more memory): whisper.cpp 1.9.6 vendors ggml 1.0.0 but
  builds and runs on the pack's ggml 1.1.1 (`WHISPER_USE_SYSTEM_GGML`); llama.cpp b12808 vendors the very same ggml commit as the
  1.1.1 tag; a test program linked against libllama, libwhisper, libqwentts and libsonic loaded the CPU module and created a
  Sonic stream; every library has `RUNPATH=$ORIGIN` and needs only libstdc++, libm, libgcc_s, libc and its siblings. ggml's
  installation puts the compute modules into `bin/`; the script moves them next to the libraries. The Vulkan module configures
  with Ubuntu 24.04's packages and all its 349 steps built (one session run was killed for memory: `mul_mm.comp.cpp.o` is a
  huge generated file, build it with `-j1` or on a bigger machine), a 34 MB library that needs `libvulkan.so.1` only when loaded.
  llama.cpp b12808 runs **Pocket TTS** (architecture `pockettts`: the text side in libllama, the flow net inside the mmproj in
  libmtmd, which is why libmtmd is built with `LLAMA_BUILD_MTMD=ON`) and also has a Qwen3-TTS path of its own (libmtmd's
  `qwen3tts-gen`), apart from qwentts.cpp. whisper.cpp 1.9.6 also installs `libparakeet`.

## 0.94.0: the full AppImage's installation window

- `quillbinder --install-folder <folder>` (`FullInstallWindow.*`, `runInstallFolder`, called from the first loop of
  `main()`): `addons::installBundled(dir, log, error, progress)` runs in a thread beside a window (logo, bar in kB, message
  "Installing N add-ons (M MB)..."); without `DISPLAY`/`WAYLAND_DISPLAY` (or with `QB_NO_DIALOGS`) it runs without the window.
  `addons::installBundledDue()` says whether anything is left (marker `full-addons-installed-<version>`). The AppRun that
  `packaging/linux/build-full-appimage.sh` writes makes this one call and keeps the marker rules.
- `tools/download-page.py`: a file `full-addons-size` (bytes) in the folder of the release's names adds "about N MB of disk
  space" to the Linux card; the release workflow does not write that file yet (the script has only file names).

## 0.93.1: Qt PDF's own source, the official entry's folder, the generic font names

- **Qt PDF** is, since Qt 6.12, a Qt Extensions module released with Qt WebEngine under a version of its own
  (6.140.0 for Qt 6.12: `QB_QTPDF_VERSION` in `CMakeLists.txt`); Qt's super-repository no longer has qtwebengine at
  v6.12.1, so `qt-everywhere-src-6.12.1.tar.xz` holds no Qt PDF source. `tools/collect-licenses.py` lists "Qt PDF" (its
  libraries and `libqpdf.so`) as a part of its own and names `qtwebengine-everywhere-src-<QB_QTPDF_VERSION>.tar.xz` for
  it and its parts; `tools/collect-sources.py` fetches that archive from
  `download.qt.io/official_releases/qtwebengine/<major.minor>/<version>/src/` (the Qt Project publishes only an MD5
  list there, `md5sums.txt`: checked, and the SHA-256 printed); `tools/get-qt-licenses.sh` reads Qt PDF's parts from
  qtwebengine's tag `v<QB_QTPDF_VERSION>`. The list written anew for 0.93.1 also has the versions of Qt 6.12.1's own
  parts (the 0.93.0 list still had 6.12.0's), and Qt's test attribution files (`tests/` of a repository, e.g. the SBOM
  test entries of 6.12.1) are left out.
- **Official entries name a folder** (`folder`, and per Windows or Android block): `tools/addon-entry.py`'s
  `source_dir()`; `cache`, `run` and `sources` (`git archive <commit>:<folder>`) use it. The entry of Translation and
  Proofreading names `addons/translation-proofreading/source` and the commit that holds 1.3 with its packaging fix.
- **The generic font names**: `fontfaces::mapGenericFamilies()` (called by `install()`) adds a `QFont` substitution
  for "monospace", "serif" or "sans-serif" only where the name falls to the default font while its style hint gives a
  font of its kind (Qt for Android: `QAndroidPlatformFontDatabase::fallbacksForFamily` answers by the style hint);
  the compiler's `exportFamily()` gives the style hint too. On Linux fontconfig knows the names: nothing is mapped.

## Qt 6.12.1 and the runtime u2404-qt6.12-r2 (0.93.0)

- **Why a new runtime id**: Qt 6.12.1 brings newer libraries inside its own build: zlib 1.4, HarfBuzz 15.1.0, PCRE2 10.49,
  libpng 1.6.59, libjpeg-turbo 3.2.1, FreeType 2.15.0 and md4c 0.6.0 (read from the `qt_attribution.json` files of qtbase
  v6.12.1). The AppImage carries them; a program update (.qbpkg) runs on the libraries of the AppImage that is installed.
  `src/core/UpdateCheck.cpp` offers the program update alone when the runtime is the same, and the new AppImage only when
  it differs, so the runtime id is `u2404-qt6.12-r2`, with `APP_RUNTIME_SINCE` 0.93.0 (`QB_QT_VERSION` 6.12.1).
- **Qt PDF**: Qt 6.12.1 has no Qt PDF of its own. The Qt pack (`tools/get-qt.sh`) takes it from the "Qt Extensions" release
  for Qt 6.12 (Qt WebEngine 6.140.0; Qt's repository folder is still 6.12.0). Quillbinder uses no Qt private headers on
  Linux (only `src/android/MobileWindows.cpp`), so nothing else depends on the exact patch version.
- **Add-ons with libraries** name the runtime and `minApp` 0.93.0: `tools/make-package.py` fills both (`runtime`, and
  `minApp` = `APP_RUNTIME_SINCE` when the manifest has a lower one), the manifests of the add-ons with libraries carry the
  0.93.0 too (Translation and Proofreading has it in `addons/translation-proofreading/manifest.json` and
  `source/packaging/manifest.json`, which must stay equal; suite `v0920` checks it against `QB_RUNTIME_SINCE`). Each
  rebuilt add-on got its last version number raised (Translation and Proofreading stays 1.2 until its own change).
- **Release text** (`tools/addon-entry.py notes`): the warning "Quillbinder X to 0.91.2 does not know add-on keys" appears
  only when the package's `minApp` is below 0.91.3; an add-on built for r2 needs 0.93.0 and has no such sentence (suite
  `v0913`).
- **FFmpeg 9.1** (security watch issue: eleven High findings up to 9.0.2): `tools/get-ffmpeg.sh` takes
  `https://ffmpeg.org/releases/ffmpeg-9.1.tar.xz` (Ubuntu's pool has 9.0.2 only). Sonames stay (avcodec/avformat 63,
  swresample 7, swscale 10, avutil 61), the configure switches and the patch of `tools/build-ffmpeg.sh` were read against
  tag n9.1 (no change needed; libpostproc is gone as before). The SHA-256 pin starts empty: then
  `tools/verify-ffmpeg-source.sh` accepts the archive only with a good release signature (`.asc` and
  `https://ffmpeg.org/ffmpeg-devel.asc`) of the key `KEY_FPR` (stops and prints the fingerprints it found otherwise)
  and prints "pin this". The first run of `deps.yml` prints it (step "Build", group "FFmpeg source archive"); it goes into
  `SHA256` of `get-ffmpeg.sh` in a commit of its own, then the pack line into `tools/deps-packs.txt`. Media Playback and
  Extra Video and Audio Formats are 2.1 (the engine's `version()` too).
- **The translator pack** (`deps-pack.sh`, name `translator`): `build/translator-deps/install` and the license files
  that `make-addon.sh` copies (the list `TP_LICENSES`, checked by the suite `deps`). `get-translator-deps.sh --recipe`
  still prints the hash of `build-deps.sh` for the release's cache. Each pre-release's text comes from
  `deps-pack.sh about <name>`.
- **Baseline on 6.12.1** (before these changes): every suite passed except `secwatch`, `v0920` and `v0913`, which followed
  from the new runtime.

## The security watch (2026-11-21)

- **What**: `.github/workflows/security-watch.yml` runs `tools/security-watch.py` once a day (04:37 UTC) on the
  parts in `tools/security-watch.json` (Qt, FFmpeg, whisper.cpp, the translator's libraries, OpenSSL and Hunspell
  for Android and Windows, zlib, Monocypher, the libraries inside Qt's own build, the Ubuntu packages of the
  AppImage). It checks out only those two files, reads nothing else of the repository and writes nothing to it.
- **Sources**: `git ls-remote` of each part's repository (newer releases, and the commit of our tag); OSV.dev's
  querybatch (those commits; the Ubuntu packages in `Ubuntu:24.04:LTS`); Qt's list of known vulnerabilities on
  wiki.qt.io (`?action=raw`); each GitHub repository's published security advisories and the release notes of
  its newer releases; the CVE records of `CVEProject/cvelistV5` on raw.githubusercontent.com (ratings and versions).
  Only raw.githubusercontent.com and git can be reached from the session; the rest is tried on GitHub's runner
  (a push to `all/security-watch*` is a dry run that writes no issue). 0.93.0: when the repository had no commit for
  50 days (`QUIET_DAYS`), the report has a line "QUIET REPOSITORY" (GitHub stops a scheduled workflow after 60 days
  without activity); the commit's date is read through the API.
- **What is reported**: a finding for a version we ship whose deciding rating is High or Critical, or that CISA's
  SSVC marks as actively exploited. The deciding rating is Ubuntu's priority for its packages, else the maker's
  (the CNA's CVSS or its own scale, as OpenSSL's "Low"; the project's own GitHub advisory), else CISA's. A record
  for another product (GDAL's copy of zlib) is not the part's; a record without versions counts as affected unless
  Qt's list names only older fixes or its description says "before" versions we are past; a bound with letters
  (OpenSSL's 1.1.1zj) lies between its numbers.
- **Issues**: one per upstream project (or Ubuntu package), label `security`, assigned to the repository's owner;
  the keys of the findings are hidden in the issue (`security-watch-key:`, `security-watch-group:`), so a finding
  is reported once. A new finding for a project whose issue is still open becomes a comment on it; after the issue
  is closed, a new finding opens a new issue. The release named in `tell_when_released` (Qt 6.12.1) gets one issue
  of its own when its tag appears.
- **A source that cannot be read** (after three tries), a missing tag, too few entries in Qt's list or an OSV query
  that must find something (`osv_canaries`) and finds nothing fails the run: GitHub's notice of a failed run tells.
  GitHub's advisories and release notes are extras: when they fail, the run warns and goes on.
- **Keeping it in step**: a new version of a part goes into the watch list in the same commit; the suite `secwatch`
  compares each `pin` (a file and pattern, an entry of `resources/licenses/index.json`, or `comes_with` for the
  FFmpeg of Qt's Windows build) with the list, and the AppImage's Ubuntu packages with the license list.
- **Not covered**: the Windows runtime of Microsoft. (0.93.0 added, Linux entries: PDFium inside Qt PDF as the
  revision `1afaa1a3...` in Chromium 140.0.7339.264, which Qt WebEngine 6.140.0 is based on, watched as that Chromium
  version of github.com/chromium/chromium, OSV not asked, its flaws reach the watch only through Qt's list (module
  qtpdf); the ggml of whisper.cpp v1.9.5 as ggml 0.26.0, OSV not asked; the AppImage runtime type2-runtime 20251108. Each
  entry has a `note` that says what the databases know. A `pin` with `comes_with` names the part whose version it
  follows: `Qt 6.12.1` or `whisper.cpp 1.9.5`, and the suite checks it. The Windows and Android entries of Qt's parts,
  and Qt's FFmpeg for Windows, are left for their runs.) A public
  repository's scheduled workflow is switched off after 60 days without activity (GitHub sends a notice first).
- **The first dry run** (2026-11-21, 40 s): 14 findings in 4 projects would have been reported: FFmpeg 9.0.2 and
  Qt's FFmpeg 9.0.1 for Windows (11, up to 9.0.2, newer: 9.1), zlib 1.3.2 (gzwrite, fixed in 1.4), HarfBuzz 14.5.0
  (its WebAssembly shaper), PCRE2 10.48 (JIT with a regular expression from outside, fixed in 10.49).

## 0.92.0 beta (2026-11-20): the check for updates, the translator as one of the project's own add-ons

- **The rules** are `core/UpdateCheck.*` (no network, no windows): `readFeed` (the release key's signature over the
  file's bytes), `offersFor` (a program update when the runtime is the same and the file has one, else the new
  AppImage, the full one for the full version; add-ons only when installed, newer and fitting), `State`
  (`<data>/updates-state.json`), `toShow`, `applyDelay`, `applyIgnore`, `autoCheckDue`. The form of the file is in
  the header.
- **The network and the windows** are `ui/UpdateChecker.*`: no `QNetworkAccessManager` exists until a look runs.
  `startAutomatic` is called from `main.cpp` 4 s after the start, when no start window is open (not with
  `QB_NO_UPDATE_CHECK` set, which the scripted runs set). A plain GET: user agent "Quillbinder", no cookies, no
  cache, redirects only to https. Two tries (`QB_TEST_UPDATE_RETRY_MS`), then `MainWindow::showFadingNote`. HTTP 404
  is "no information" and quiet. A download is hashed while it comes in and must be the size and SHA-256 of the
  signed list; packages go through `addons::install(..., Unsigned::Refuse)`; `replaceOwnFile` swaps the AppImage
  (`$APPIMAGE`) and keeps `<name>.before-update`.
- **Test switches**: `QB_TEST_UPDATE_URL` (also allows `http://127.0.0.1:` downloads), `QB_TEST_UPDATE_NOW` (the
  clock), `QB_TEST_APPIMAGE`, `QB_TEST_NO_RESTART`, `QB_TEST_SIGNING_KEY` (as for packages). v0920 runs a small
  server inside the test.
- **The file** is made by `tools/update-feed.py` in the job `sign-checksums` of `release.yml` (signed with the
  release key, verified with the public one) and published by `download-page`; the Windows and Android workflows
  call `tools/keep-update-feed.sh` because the page is replaced as a whole. `packaging/update-feed/security.json`
  marks security versions. The address is built in: `https://shelin-boop.github.io/Quillbinder_Collection/updates-1.json`.
- **TLS**: the AppImage carries `plugins/tls/libqopensslbackend.so` since 0.92.0 (`build-appimage.sh`); OpenSSL 3 is
  the system's. A program update that runs on an AppImage before 0.92.0 has no TLS part: `QSslSocket::supportsSsl()`
  is false, the automatic look stays quiet and "Check for Updates Now" says why.
- **Translation Helper 1.5** (0.95.0, to-do items 101 and 102; details in the add-on's `source/DEVELOPING.md`, "1.5"): the
  add-on is renamed (id unchanged), translates between any two of eleven languages (two OPUS-MT models per language, a pair
  without English goes through English) and has a dictionary per language from kaikki.org. The data are on the release
  `models-4`, made by `.github/workflows/translator-data-4.yml` on GitHub's runner (push of a `linux/0.95.0*` branch that
  changes it, or by hand); `resources/catalog.json` holds placeholders until the run: the sizes and sums of the artifact's
  `catalog.json` are copied in, and the kaikki SHA-256 sums the run prints are pinned in the workflow. The official entry
  keeps the commit of 1.4 until 1.5 is released (name and summary are the new ones). Its own texts are compiled in
  (`translations/tp_<code>.ts`).
- **Translation and Proofreading 1.3** (0.93.0, to-do item 23; details in the add-on's own `source/DEVELOPING.md`, "1.3"):
  a word on its own (WordNet meanings built in as `resources/en-glosses.tsv`, made with the thesaurus by
  `tools/make-thesaurus.py`; the German dictionary, a data download "wiktionary-de" made by
  `tools/make-de-dictionary.py` from kaikki.org's extract, CC BY-SA 4.0 shown before the download), Swap and Translate
  back, `opus-mt-en-de`, the large model kept away from words and up to three words (catalog flag `forWords`). The data
  is on the release `models-3`, made by `.github/workflows/translator-data.yml` (push of a `linux/0.93.0*` branch that
  changes it; it does nothing when the release has the files; the job `publish` alone writes). Until the run, the
  catalog names the new files with `PLACEHOLDER...` sums (the add-on says "not published yet" and downloads nothing);
  after the run the sizes and sums of the artifact's `catalog.json` go in. `official-addons/translation-proofreading.json`
  still describes 1.2 until the release (`tests/v0920_tests.cpp` compares its version with the manifest).
- **Translation and Proofreading 1.2** is `addons/translation-proofreading`: `source/` is the add-on as a project of
  its own (unchanged from 1.1 but for the version and the model address `kRepo` in `src/ModelStore.cpp`), `stage.sh`
  builds it into `<build>/translator` and stages it with the add-on's `packaging/make-addon.sh`
  (`TP_STAGE_DIR`); exit code 3 without the engine libraries of `tools/get-translator-deps.sh`
  (`build/translator-deps`; the release workflow keeps them under the hash of `build-deps.sh` and fails when the
  add-on is missing). The models: releases `models-1`, `models-2` of this repository, copied in 0.92.0
  from the add-on's former repository; since 0.93.0 `.github/workflows/models-check.yml` (push the branch
  `models-check`) only compares the files of `catalog.json` with the two releases. The add-on's own checks need models
  and are not part of the suites. `official-addons/translation-proofreading.json` describes 1.2 since 0.93.0 (this
  repository, the commits of the 0.92.0 releases; its build lists still name the add-on's source folder
  `addons/translation-proofreading/source` as the root, which `addon-release.yml` does not check out: it builds an
  add-on of another repository). Since Windows 0.92.0 the Windows package is built with Quillbinder too (`docs/dev-notes/windows.md`), and
  the check for updates has its Windows way (the setup program) described there.

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
- **The page's deploy, waited for and tried twice** (2026-11-24; `release.yml`, `release-windows.yml`,
  `release-android.yml`). `actions/deploy-pages` lists the run's artifacts at once and may miss the one
  `upload-pages-artifact` has just uploaded (0.93.0: "No artifacts named github-pages-N were found", three runs in a row,
  the API listed it a little later). Before the deploy a step waits until `github-pages-<attempt>` is listed (every 10 s,
  at most 3 minutes, then it goes on); the first deploy has `continue-on-error`; after a 60 s pause a second deploy
  runs only when the first failed, and when it fails the job fails. The job has `actions: read` for the wait.
- **Page only, Linux** (`release.yml`, same idea as the input in `release-windows.yml`): for a release that is published
  while its page and update file are not. Inputs `page_only` and `from_run` (id of the run whose artifact
  `release-files` holds the signed `updates/updates-1.json` and `.sig`; artifacts are kept 3 days). The job `version`
  then builds nothing (it fails when the release of CMakeLists.txt's version does not exist), `publish` is skipped,
  `download-page` takes `release-files` from `from_run`, checks the file and its signature with
  `packaging/quillbinder-signing-public.pem` and makes and deploys the page as usual. Start it with
  `gh api -X POST repos/Shelin-boop/Quillbinder_Collection/actions/workflows/release.yml/dispatches -f ref=main -F 'inputs[page_only]=true' -f 'inputs[from_run]=<id>'`.
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
