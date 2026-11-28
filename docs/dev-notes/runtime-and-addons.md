# Runtime (Qt, FFmpeg), add-ons, tool interface, release files

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## 0.99.0: the label is one setting per system

- `CMakeLists.txt` holds `APP_VERSION_LABEL` (Linux's, in the first column: every Linux script, `release.yml`,
  `tools/publish-release.sh`, `tools/release-notes.py` and the shared tools read this line), `APP_VERSION_LABEL_WINDOWS`
  and `APP_VERSION_LABEL_ANDROID`. A Windows or an Android build takes its own as `APP_VERSION_LABEL` (a second
  `if(WIN32)` / `elseif(ANDROID ...)` block after the version one), so that `AppInfo.h`, `APP_VERSION_SHOWN` and the
  program see one label. `tools/update-feed.py` takes the label of `--system` (`--label` still wins).
- A normal release has an empty label and gets plain names: tag `v1.0.0`, `Quillbinder-1.0.0-x86_64.AppImage`,
  `Quillbinder-1.0.0-full-x86_64.AppImage`, `quillbinder-1.0.0-update.qbpkg` (every name is made with
  `${LABEL:+-$LABEL}`; never write `-$LABEL` alone). The download page's note leaves out "This is a ... version" without a
  label. `verlines` runs this as a dry run (a copy with 1.0.0 and empty labels).
- Still hard-coded for Windows and Android (their parts follow later): the tags `v<version>-windows-experimental` /
  `-android-experimental` in `release-windows.yml` and `release-android.yml`, and the file names in `packaging/windows/`
  and `packaging/android/`.
- A security version is found by number only: the update check offers "recommended (security)" when a listed version lies
  after the installed one and not after the newest (`securityBetween`, `src/core/UpdateCheck.cpp`), so a program on 0.95.0
  sees 0.99.0's mark even when 1.0.0 is the newest, as long as 0.99.0 stays in `packaging/update-feed/security.json` (the
  feed also keeps what the previous feed listed).
## 0.99.0: tool interface 1.3, the rule for the AI models' memory

- **Interface** (`src/tools/ToolApi.h`; `apiVersion() == 130`): new functions at the end of `ToolHost` (`aiThreads`, `modelMessage`,
  `keepRunning`, `lowerThreadPriority`, `requestModelLoad`, `modelLoaded`, `modelUsed`, `modelUnloaded`, `reportDeviceMemory`)
  and a third interface `ToolModels` (IID `org.quillbinder.ToolModels/1.3`, `bool unloadModel(id)`), declared with
  `Q_INTERFACES(qb::tools::ToolInterface qb::tools::ToolModels)` and found by `ToolHost::models()` like the 1.1 part. A tool calls
  the new functions only when `apiVersion() >= 130`. `tests/tool12/` holds a tool built with the header of 1.2 (it still loads
  and runs), `tests/tool13/` a tool of 1.3. The add-on project `addons/translation-proofreading/source/third_party/quillbinder/ToolApi.h`
  is a copy of 1.2 and stays so until that add-on adopts 1.3.
- **The model's id** is `<tool id>:<name>`; the part before the colon tells the program whom to ask to unload it. Calls
  from a worker thread are passed to the main thread (blocking), so a tool may call from where it loads; do not call while the
  main thread waits for that thread.
- **`src/core/ModelMemory.{h,cpp}`** keeps the list (`requestLoad`, `modelLoaded`, `modelUsed`, `modelUnloaded`) and applies the rule
  (a memory `where` is `"ram"` or `"gpu:<name>"`; RAM from `SystemMemory`, a card from the engine's report; unknown figures
  never refuse). Order of `requestLoad`: above 50 % used, the models of that memory unused longest go one by one until it is at
  or under 50 %; then, if the new model would bring it over 80 %, further models go (longest unused first) only when that
  can make it fit, else nothing more is unloaded and the load is refused (the 50 % step has run already: [helper's choice]).
  A model whose owner answers `false` (in use) is skipped. Freed bytes are counted from the model's registered size, because
  the system's figures lag behind an unload.
- **Idle**: a timer every 30 s unloads models unused for the set time. **Low memory** (`LowMemoryMonitor`): models used within the
  last 30 s are not idle for it. The count of changes under way (`changeBegin`/`changeEnd`) drives the standing text of the
  message line (`MessageLine::setStatus`) under the inspector (`Inspector::messageLine`; `MainWindow::modelLine()` takes the binder's line while the inspector is hidden); a load that is allowed counts as under way until `modelLoaded` (or `modelUnloaded`
  when it failed).
- **`SystemMemory.h`**: `SystemMemoryLinux.cpp` (`/proc/meminfo`; PSI trigger `some 150000 2000000`, a window that is a multiple of
  2 s as an unprivileged trigger needs, watched by a `QSocketNotifier` of type Exception), `SystemMemoryWin.cpp` and
  `SystemMemoryAndroid.cpp` (compile only on their systems; not built or run in the Linux part). `KeepAlive.h` keeps the reasons;
  `KeepAliveNone.cpp` (Linux, Windows) does nothing, `KeepAliveAndroid.cpp` is a stub for the Android part. CMake picks the
  files per system (`CORE_SOURCES` filter).
- **Settings**: `app/aiThreads` (0 = the default, so it follows the computer) and `app/aiUnloadMinutes` (10); program-only
  (`settingslayer::programOnly`), not in profiles; handed over in `AppSettings::pushToModules`.
- Suite `v0990b`.
## 0.99.0: the encryption block of CMakeLists.txt on three systems, and the packs in the Linux workflows

- The RNP block no longer excludes Windows and Android: it takes the libraries from `QB_CRYPTO_DIR` (Linux and Android
  `lib<name>.a`; Windows MSVC `.lib` files, with or without the `lib` prefix, found by name). Windows adds `RNP_STATIC` (the
  headers say dllimport without it), vcpkg's zlib (`ZLIB::ZLIB`) and `bcrypt advapi32 crypt32 user32 ws2_32`; Android
  links the NDK's zlib with `-lz`, one prefix per ABI. Not tried on Windows or Android here (no such build in this tree).
  The Linux link lines of the program and of `qb_v0990e_tests` and the program's `ldd` are byte for byte the same as before.
- `release.yml` (job `build`: the cache under `deps-linux-crypto-<hash>` then `tools/get-crypto.sh`, written to the cache on
  main; job `tests`: the three caches read only, then the three scripts), `ccache.yml` and `system-test.yml` fetch the
  crypto, AI engine and ONNX Runtime packs before they configure; suite `deps` checks it.

## 0.99.0: the packs crypto, onnxruntime and aiengine (Linux)

## 0.99.0: the AI engine packages

- **Two packages, add-ons of their own** (decision 3): `addons/ai-engine` (ggml 1.x with its CPU modules and, from the
  GitHub pack, the Vulkan module; whisper.cpp, libllama and libmtmd (Pocket TTS), libqwentts (Qwen3-TTS), libsonic and our
  libqbai) and `addons/handwriting-engine` (ONNX Runtime's `libonnxruntime.so.1`, no wrapper: Handwriting opens it itself and
  calls `OrtGetApiBase`; `ai::onnxRuntimeLibrary()` finds it). Payload `engine/lib` (every library with RUNPATH `$ORIGIN`, the
  compute modules beside them) and `engine/licenses` (the libraries' own license files from the pack, the texts of the code
  libmtmd and the CPU modules hold besides their own (stb_image, miniaudio, subprocess.h, xxHash, SHA-1/SHA-256, llamafile's
  sgemm; taken from the pinned sources into `addons/ai-engine/payload/engine/licenses`), and a `README.txt` written by
  `stage.sh` that names every library). `minApp` 0.99.0; `make-package.py` adds the runtime as for every add-on with libraries;
  signed with the add-on key at release like the other add-ons. `stage.sh` exits 3 (left out) without the pack or without
  libqbai; the file `may-be-missing` in their folders lets `build-full-appimage.sh` leave a package out that was not built
  (with a message) instead of failing, so a release without the packs still builds. `tools/check-engine-package.py` checks a
  finished package (the expected libraries, nothing else, README names each, the license files, `ldd` of each needs only the
  package and glibc's libraries, libstdc++ and libgcc_s, `libvulkan.so.1` only for `libggml-vulkan.so`, RUNPATH `$ORIGIN`).
  Sizes from the published packs: "ai-engine" 21.4 MB with the Vulkan module (10.5 MB without), "handwriting-engine"
  13.1 MB. ONNX Runtime comes from Maven Central (the JAR's `linux-x64` library, `tools/get-onnxruntime.sh`), whose package
  holds Microsoft's `ThirdPartyNotices` and `Privacy.md` but not ONNX Runtime's own MIT text: that text is in the recipe
  (`addons/handwriting-engine/payload/engine/licenses/LICENSE-onnxruntime.txt`); `stage.sh` adds the pack's license files and
  its `README.txt` (the Maven coordinates and SHA-256) as `SOURCE-onnxruntime.txt`. Seen in the session with the published
  Vulkan pack: the Vulkan module loads from the package's folder with the system's `libvulkan.so.1`, and Mesa's software
  renderer (lavapipe) is not listed as a device by ggml, so Automatic never takes it.
- **libqbai** (`engines/qbai/qbai.cpp`, built by `engines/qbai/CMakeLists.txt` only when the pack is there at configure time
  (`QB_AI_ENGINE_DIR` or `build/ai-engine`), into `<build>/qbai/libqbai.so.1` with links to the pack's libraries beside it,
  as in the package; not `<build>/ai-engine`, which is the pack's own default folder). The program links none of it.
- **The C interface** `src/tools/QbAi.h` (the program and add-ons include it): one exported entry `qbai_get_api(int version)`
  returning `const struct qbai_api *`, a table of function pointers, `QBAI_API_VERSION` 2 since Speech Transcription 1.10 (1 before). A newer library keeps every function
  of an older version at its place and adds new ones at the end; `qbai_get_api(v)` answers for every `1 <= v <=` its own
  version (NULL above), and `size` says how much of the table there is. Every parameter struct begins with its own `size`.
  Functions: `engine_version`, `last_error` (thread-local, English), `device_count(refresh)`/`device_get` (index 0 the CPU, then
  ggml's GPU and IGPU devices in ggml's order with `ggml_backend_dev_memory`, for Vulkan the driver's budget),
  `set_threads`/`threads`, `model_memory` (an estimate from the files: whisper file + 25 % + 200 MB, Pocket TTS + 20 % + 256 MB,
  Qwen + 25 % + 1 GB; the catalog's `memoryMB` wins), `load`/`unload`/`model_info`/`voice_name`/`language_name`, `transcribe`
  (16 kHz mono float; segment, progress and cancel callbacks on the calling thread), `synthesize` (one sentence; `chunk`
  streams it, `cancel` stops it; both may run on an engine thread: qwentts.cpp computes on a worker of its own), `free_text`,
  `free_pcm`, `stretch` and `stretch_begin/feed/end` (Sonic, speed 0.5 to 4, pitch kept). Threads: one model per handle,
  calls on a handle from one thread; load, transcribe and synthesize from a worker that lowered its priority first (ggml's
  and qwentts.cpp's threads are started from it and inherit the nice value).
- **How the models are driven**: whisper.cpp with `use_gpu`/`gpu_device` (its device number counts ggml's GPU and IGPU devices,
  so it is our index minus 1). Pocket TTS: `llama_model_load_from_file` with `devices = {card}` (an empty list for the CPU,
  `n_gpu_layers` 0), a context with `embeddings = true` and `n_ctx` 4096, `mtmd_init_from_file` for the mmproj, then
  `mtmd_helper_gen_audio_*` as llama.cpp's own `tools/tts/tts.cpp` drives it (prompt steps, then one frame per `step_gen` from
  the hidden state; the sampled token is not used by Pocket TTS but the loop takes one: a plain `llama_sampler` chain, since
  libcommon is not in the pack); the voice is `mtmd_gen_audio_get_voice(name)` or a clip through `mtmd_bitmap_init_from_audio`
  when the model has its voice encoder. Qwen3-TTS: `qt_init` with `device` = ggml's name ("Vulkan0", "CPU"), `max_batch` 1,
  `codec_chunk_sec` 4 (bounds the decoder's memory), language codes mapped to qwentts.cpp's names ("en" -> "english", else
  "auto"), Base with a reference clip (resampled to 24 kHz, linearly) and its text, CustomVoice with a speaker and an
  instruction; VoiceDesign is not offered. **Limit**: qwentts.cpp takes half the logical cores itself (no switch in its API),
  so the shared setting of the cores does not reach Qwen3-TTS.
- **The program's side** `src/core/AiEngine.{h,cpp}`: `Engine` (one per process, kept to the end) finds the installed
  "ai-engine" (`addons::installedAddons`, switched on, of this runtime) and its `engine/lib/libqbai.so.1`, opens it with
  `QLibrary` on first use (`ensureLoaded`, from any thread), checks the version and `size`, hands the shared setting of the
  cores over, lists the devices (and reports the cards to `ModelMemory`). A package installed while one is loaded takes effect
  at the next start (`restartNeeded`; the libraries are in use). **Pitfall**: before loading it looks with `dl_iterate_phdr`
  for libraries of the engine's names (`libwhisper.so.1`, `libggml.so.1` ...) already loaded from another folder (Speech
  Transcription up to 1.9 brings its own whisper.cpp): the system's loader goes by the name and would give libqbai that copy,
  so the engine is refused with a message (restart, or update that add-on). The other way round (the engine loaded first,
  then Speech Transcription 1.9) is that add-on's matter; 1.10 uses the engine. `pickDevice` is the one rule of the device
  choice: "auto" takes the card whose free memory holds the model (a card with memory of its own before a graphics part that
  shares the RAM, then the one with more free memory; size unknown: the CPU), "gpu:<name>" that card while it is there,
  "cpu". `Model` (one per function and model, id "<function>:<model>") asks `ModelMemory::requestLoad` before every load (from
  a worker the call waits for the main thread), loads, then `modelLoaded`; when Automatic's card is refused by the 80 % rule,
  or the card fails to load it, it tries the CPU once; its unloader answers false while a job runs (an atomic count and a
  `tryLock`), so `ModelMemory` never waits. Device names: "gpu:<the card's name>", " #<index>" added for a second card of
  the same name.
- **Tool interface 1.3** (no new version number: 1.3 was not published before these functions): `ToolHost::aiEngine(version)`
  returns the program's own table (`const qbai_api *`; ToolApi.h forward-declares the struct), so an add-on never opens
  libqbai itself; `aiPickDevice(choice, bytes, &where)` applies the rule and gives the index and ModelMemory's `where`;
  `aiDeviceChoices()`; `aiPanel(kinds, parent)` the program's install panel; `aiModelFile(id, role)`.
- **The catalog** (`src/core/AiCatalog.{h,cpp}`, `resources/ai/models.json`, built into the program; format 1, the pattern of
  models-4's `catalog.json`): `{"format": 1, "release": "models-5", "models": [{id, title, summary, kinds ["tts" | "asr" |
  "handwriting"], engine ("whisper" | "pocket-tts" | "qwen3-tts" | "onnx"), languages (ISO codes, "*" for many), voices,
  license, licenseUrl, credit, source, nonCommercial, sizeMB, memoryMB, release, model, extra, files [{name, asset, size,
  sha256, parts [{asset, size, sha256}]}]}]}`. An id or file name that would leave its folder is left out; a file without its
  SHA-256 is "not published yet". The models go into `<data>/ai-models/<id>` (`qb::dataDir()`), `.checked` marks a model whose
  files were all checked, `.<name>.ok` a file checked in a download that stopped later (it is not fetched again). Until the
  `models-5` workflow writes it, the catalog lists no models.
- **Downloads** (`src/ui/AiDownloads.{h,cpp}`, Qt Network is in qbui, not qbcore): only by a button (no network object before);
  the addresses are `<APP_HOMEPAGE>/releases/download/<release>/<asset>` for models and
  `.../releases/download/v<version>[-<label>]/ai-engine.qbpkg` (Windows `ai-engine-windows.qbpkg`, Android
  `ai-engine-android.qbpkg`: names of a later part) for the engine, never asked from a server; the request carries the
  program's name only, as the check for updates. Each piece is checked against its size and SHA-256 while it comes in, a file
  in parts again as a whole. An engine download must be signed by the project (`Unsigned::Refuse`); a file the writer chose
  may be unsigned after the warning (`AllowAddons`). Test builds accept unsigned packages unless `QB_REQUIRE_SIGNED` is set
  (the installer's own rule). Test switches: `QB_AI_ENGINE_LIB` (a libqbai to load instead), `QB_AI_CATALOG`,
  `QB_AI_MODEL_SOURCE`, `QB_AI_ENGINE_SOURCE` (a folder or an address ending in "/").
- **The panel** (`src/ui/AiInstallPanel.{h,cpp}`): the engine's state with Install the Engine / Install Again and Install from
  File..., the catalog's models of the given kinds (languages, download, memory, license, state), a detail line (summary,
  credit, license link, why a model cannot be downloaded), Download / Delete, a progress bar with Cancel. The pages of Read
  Aloud and Handwriting place it; Speech Transcription gets it through `aiPanel`.
- Suite `v0990h` (a fake libqbai in `tests/fakeqbai`); the release workflow builds the two packs from source into its cache
  (D3) and checks the packages with `tools/check-engine-package.py`.




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
- **onnxruntime** (`tools/get-onnxruntime.sh`, prefix `build/onnxruntime`): taken from **Maven Central**, not from GitHub: the
  repository microsoft/onnxruntime is not public under that name any more (its release list answers 404, `git ls-remote` asks for a
  login; PyPI and Maven Central have 1.32.0). The Linux library is `ai/onnxruntime/native/linux-x64/libonnxruntime.so` of the JAR
  `com.microsoft.onnxruntime:onnxruntime:<v>` (the same JAR has `win-x64/onnxruntime.dll` for the Windows part); the C headers are
  `headers/*` of the AAR `com.microsoft.onnxruntime:onnxruntime-android:<v>` (its `jni/<abi>/libonnxruntime.so` are for Android
  later). Pack: `include/`, `lib/libonnxruntime.so.<v>` with the symlinks `libonnxruntime.so.1` and `libonnxruntime.so` (the program
  `dlopen`s the full path of `libonnxruntime.so`, then `OrtGetApiBase`), `licenses/`, `README.txt` (coordinates, version, SHA-256).
  The SHA-256 of both files is **not pinned yet**: while `SHA256_JAR` or `SHA256_AAR` is empty the download is accepted only with
  `QB_ONNXRUNTIME_FIRST_RUN=1` (set by deps.yml). That run reads `maven-metadata.xml` of both packages, takes the newest plain release
  both have (a version with a word in it is a pre-release), prints both versions, the file lists of both files (native libraries,
  headers, licenses), the `NEEDED` list of the library (`objdump -p`; it must name only base-system libraries) and
  `pin this: VERSION=... SHA256_JAR=... SHA256_AAR=...`, which the lead puts into the script (the recipe's hash changes, the pack is
  made again in seconds). License files that the JAR or AAR carry go into `licenses/`; where they carry none the MIT text named by the
  POM's license entry is written there and the README says so. `QB_MAVEN_URL` changes the repository (tests: a `file://` folder).
  The program never links the library. The security watch reads this part from Maven Central (`"maven"` in `tools/security-watch.json`:
  `maven-metadata.xml` for newer releases, OSV for the Maven package and version; no GitHub advisories).
- **onnxruntime** (`tools/get-onnxruntime.sh`, prefix `build/onnxruntime`): Microsoft's prebuilt `onnxruntime-linux-x64-1.32.0.tgz`.
  The session that made the script could not download it (the repository of microsoft/onnxruntime is closed to it; the release
  page answers 403/404), so a pack is needed: the workflow downloads it. The SHA-256 is **not pinned yet**: while `SHA256` in the
  script is empty the download is accepted only with `QB_ONNXRUNTIME_FIRST_RUN=1` (set by deps.yml), and the script prints
  `pin this: SHA256="..."`, which the lead puts into the script (the recipe's hash changes, the pack is made again in seconds).
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

## 0.99.0: the designed voices (job `voices` of `models-5.yml`)

- **What**: 7 languages x 6 voices x 3 styles = 126 clips, designed from the descriptions in `design-voices.py` with the
  VoiceDesign 1.7B Q8_0 GGUF (pins: group `qwen-voicedesign` of `models-5-sources.json`, same repo and commit as `qwen`) and
  qwentts.cpp at its pinned commit. `design-voices.cpp` is a small tool on the engine's public interface (`qwen.h`: it loads the model
  once and writes mono 24 kHz 16 bit WAV); it is built in the job next to the engine pack's libraries (the compute modules are found
  beside the program) and runs on the processor. `design-voices.py make` checks each clip (5 to 20 s, RMS above 0.01, no run of 10
  samples at full scale), designs a failed clip once more with another seed, then fails the job; it writes `read-aloud-voices-<lang>.zip`
  (36 files + `voices.json`: model, license Apache-2.0, descriptions, "no real person's voice").
- **How to run** (the release must exist): dispatch `models-5.yml` with `voices=true`. The `state` job then switches every other job
  off; `voices` (matrix of the seven languages, read-only) -> `voices-catalog` (read-only: `make-catalog.py voices` adds the seven
  `voicepack` entries to the release's own catalog.json) -> `voices-publish` (write; no checkout; uploads the zips and the catalog with
  `--clobber`: a run again replaces a language's zip). Nothing else of the release changes. The run time of a language is not known
  (CPU, 18 clips of 5 to 20 s; the job's limit is 300 minutes).
- **After a run** (by hand): download the release's `catalog.json`, run `python3 tools/models/make-catalog.py models-json
  resources/ai/models.json --catalog catalog.json` (fills `size` and `sha256` of the seven `voices-<lang>` entries; until then they are
  `0` and `""`, and `tests/models5_tests.py` fails with the list of the unfilled ones), replace `tools/models/models-5-catalog.json` with
  that catalog.json, run `python3 tools/gen-resources.py` if the build asks, commit.
- **In the program**: `readaloud::installVoicePack` (core/ReadAloud.cpp) unpacks with `ZipReader` only names of `voicePackNames()`
  (no folder, no path, no name twice) into `read-aloud-voices/<lang>.new`, then swaps it for `<lang>/`; `AiDownloads` calls it after the
  zip's SHA-256 was checked and a Qwen model's download is followed by `ensureVoicePack(effectiveLanguage)`; the Read Aloud page calls it when
  the language is chosen; the panel calls `removeVoicePacksIfNoQwen()` after a delete. `languageHasVoice` decides the default language.
  A pack entry with `size` 0 / no SHA-256 is "not published yet" (no download, `ensureVoicePack` does nothing).

## 0.99.0: the release models-5 (`models-5.yml`, `tools/models/`)

- Files: `models-5.json` (every catalog field except size and SHA-256), `models-5-sources.json` (pins of the Hugging Face
  files: commit, size, SHA-256), `pocket-voices.json` (the licensed voices), `fetch-hf.py`, `convert-ppocr.py`,
  `make-wordlists.py`, `convert-pocket-tts.py`, `make-catalog.py` (`build` for the catalog job, `models-json` writes
  `resources/ai/models.json`). Suite `models5` keeps them, the workflow and the program's catalog in step.
- **Handwriting model**: kraken's PP-OCRv6 small (Apache-2.0; the file has 13,065,380 bytes = about 3.24 M parameters, which is what
  the model card says; the paper's 5.2 M is not this file; the script prints the count of the network it builds). The network's
  source is taken from `kraken==7.2` installed with `--no-deps`: kraken's package initializer imports Lightning and more, so the
  script loads `lib/ppocr` as a package of its own. The file's metadata says line height 96 (the card's 128 px is the paper's);
  the backbone's last `avg_pool2d(kernel_size=(h, 2))` reads h from the tensor, so the export keeps the height fixed and the width
  free (the export patches `F.avg_pool2d` so that h is a plain number; input widths are multiples of 8, because the network pads each stage by the width's remainder and a trace fixes that for one width; `dynamo=False`; kraken needs torch 2.9 or newer, where the new exporter is the default). The first runner run (2026-11-27) failed in the export (the pooling's Gather node, fixed as said) and the Pocket TTS conversions failed at the tokenizer (sentencepiece 0.2.3's model file was made with protobuf 7.36 and refuses an older runtime, the converter's requirements pin 4.x: the workflow installs `protobuf>=7.36.1`; the second run still failed on 6.33); both failures were hidden by a pipe to `tee` (no pipefail), so the artifacts lacked the files and only the catalog job noticed. The measured parameter count is 3,216,543 (3.22 M). the script stops on a shape or text
  difference between ONNX Runtime and PyTorch and on a largest difference of 0.001 or more.
- **Word lists** are Latin-script words only (wordfreq's lists hold some other scripts, e.g. Chinese words in the Indonesian one).
- **Pocket TTS**: the engine finds a preset voice by name in the mmproj (`voice.<name>` tensors); `kyutai/pocket-tts-without-voice-cloning`'s
  language folders carry 27 voice states each, and llama.cpp's converter turns the states of the folder `embeddings/` into those
  tensors, so only the licensed ones are staged. Licenses per voice follow the README of `kyutai/tts-voices` (VCTK and Alba
  MacKenna: CC BY 4.0; Voice-Zero, voice donations and Kyutai's own recordings: CC0; Expresso and EARS: non-commercial, left out;
  voices whose recording that README does not name: left out; juergen: never). The model is the 16-layer one of each language.
- The dispatch input `pocket` (default on) turns Pocket TTS off: the catalog job then runs without it (`make-catalog.py build --without-pocket`) and the five models are listed without checksums ("not published yet"). A published release is never changed, so Pocket TTS would then come with a later release.
- Published 2026-11-27 (run 46013112875, 73 assets, every size and SHA-256 equal to the release's own digests): `tools/models/models-5-catalog.json` is its catalog.json; `resources/ai/models.json` is made from it. Each Pocket TTS mmproj holds 20 voices (the staged ones; the mmproj has 50.8 MB because it has no voice encoder).
- A release asset may have less than 2 GiB: the largest file (Qwen 1.7B Q8_0) has 2,079,448,256 bytes, so no file is split.
  `catalog.json` is uploaded last: a stopped upload is finished by the next run, which skips what is there.
## 0.99.0: Speech Transcription 1.10 (the AI engine instead of its own whisper.cpp)

- **Layout.** `libqb-speech-impl.so` no longer links whisper.cpp or ggml: it gets the engine from the host (`ToolHost::aiEngine`,
  `QBAI_API_VERSION`) and calls the C table of `tools/QbAi.h`. `SpeechApi.h`: `EngineSettings::device` ("auto", "cpu",
  "gpu:<name>") replaces `gpuIndex`; `threads`, `careful` and `unloadMinutes` are gone; `computeDevices()` is gone (the tool's
  list is `ToolHost::aiDeviceChoices()`); `setHost()` is new, called by the tool right after the library is created. The stage
  script and CMake need whisper.cpp's folder only for the test recording `samples/jfk.wav`.
- **One `Holder` for the model.** The live pieces, the model check and the speed test share one loaded model (mutex in `Holder`).
  The registry id is `speech-transcription:<model id>` (`ToolHost::toolOfModel` takes the part before the colon). Every load
  asks `requestModelLoad` (with `modelMessage` around it), then `modelLoaded` (a failed load: `modelUnloaded`); each piece calls
  `modelUsed`. **Pitfall**: `requestModelLoad` waits for the main thread, so the program's request to unload
  (`ToolModels::unloadModel`, main thread) must never wait for the worker: `Engine::unloadModelOfId` only tries the mutex and
  answers `false` ("in use") when it cannot take it. A blocking wait here would be a deadlock (the same one ai1 met in the
  engine's job lock).
- **Engine interface 2** (`tools/QbAi.h`, `QBAI_API_VERSION` 2): added at the end, so version 1 is a prefix. `qbai_get_api(1)` returns a
  table with `size == QBAI_API_V1_SIZE`; `qbai_get_api(2)` the whole one with `detect_language`. `AiEngine::ensureLoaded` accepts
  version 1 (an older engine package still works for Read Aloud); the add-on asks `aiEngine(2)` and falls back to `aiEngine(1)`
  (then "Careful" and the language choice do nothing and an unlimited language is "auto"). Transcribe params of version 2 are read
  only when `size` reaches them (`QBAI_TRANSCRIBE_V1_SIZE`). The add-on passes `beam_size` 5 for Careful, `no_speech_max` 0.6 and
  `suppress_nst` for the silence guard (whisper's no-speech probability per segment), `no_timestamps` and `no_context` as 1.9
  did; the two-language choice is `speech::chooseLanguage` on `detect_language`'s probabilities, as in 1.9 (for translation into
  English, for "keep the original" and for Careful). The fake engine (`QBAI_FAKE_LANG`, `qbai_fake_detects`) follows.
- **Model files.** `installedModelFile()` is the add-on's own download first, else the program's catalog entry `whisper-<id>`
  (`ToolHost::aiModelFile`, `<id>` from `speech::catalog()`); `ownModelFile()` is what Remove and the replace step touch. The
  catalog's ASR entries (models-5) must use these ids.
- **minApp and the update file.** `addons::compareVersions(minApp, programAfter)` in `UpdateCheck.cpp`: an add-on offer needs the program
  version the writer has after the same check (a program update offered as `ProgramUpdate` counts; a new AppImage does not,
  as the writer has to install it first). So 1.10 reaches a 0.95.0 installation only with the update to 0.99.0, and
  `addons::inspect` refuses a package with a higher `minApp`. Checked in `v0990m`.
- **Microphone permission** (`QMicrophonePermission`, in `startAudio`): granted at once on Linux and Windows; on Android the
  first recording asks and does not start (the writer presses Record again). `keepRunning("recording")` is set from the start of
  a recording (live or Record only) until it stops. `recordOnly()` is Android's default (`Q_OS_ANDROID`, `QSysInfo`, or the test
  switch `QB_SPEECH_ANDROID`).

## 0.99.0: Translation Helper 1.6 (tool interface 1.3, CTranslate2 4.9.1)

- The add-on's `third_party/quillbinder/ToolApi.h` is the program's 1.3 header again (copied whole). `TranslateTool` also
  implements `ToolModels` (`Q_INTERFACES(... ToolModels)`). With `host->apiVersion() >= 130` (`hostHasRegistry()`, decided at
  each call: the checks switch the fake host's version) `engine()` returns a `RegisteredEngine` that wraps the real engine:
  `load` asks `requestModelLoad("translation-proofreading:<catalog id>", title, memoryMB in bytes, "ram")` (one entry per model,
  however many slots hold it), calls `modelLoaded` or `modelUnloaded`, and puts `modelMessage(true/false)` around the change;
  `run`, `variants` and `alternatives` call `modelUsed` (`run` at each batch). The host's calls come from the worker thread and
  block until the main thread has answered; `unloadModel` runs on the main thread and answers false while any panel is busy.
- The add-on's own memory check (`loadProblem`) and its unload timer are for 1.2 only; with 1.3 the program's rule decides and
  its reason reaches the writer through the load's error text. `threads()` is `aiThreads()` with 1.3.
- The options page shows the two fields only for a 1.2 program; with 1.3 a label (object name `tpSharedSettings`) names the
  program's page. No call opens it (`showOptions` opens a tool's own section only).
- Pitfall: `slots` is a Qt keyword (macro) in a class with `Q_OBJECT` headers included: do not use it as a variable name.
- CTranslate2 4.9.1 is the Linux engine only (`Engine.cpp` `version()` says 4.9.0 on Windows and Android, whose recipes did not
  change). The pack line of the new recipe hash is in `tools/deps-packs.txt`.

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
