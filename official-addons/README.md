# Official add-ons: the pinned entries

One file per add-on on the official list (`docs/official-addons.md`): which source (repository and commit) the
project's workflow builds, and how. The workflow "Official add-on" (`.github/workflows/addon-release.yml`) reads it:

- `id`, `name`, `version`: must match the add-on's own `manifest.json`;
- `repository`, `commit`: the reviewed source, a full commit id (a new version is a new review and a new commit here);
- `folder` (0.93.1, optional): the add-on's folder inside that repository when it is not the repository's top (the
  project's own Translation and Proofreading: `addons/translation-proofreading/source` of Quillbinder's repository).
  Every path of the entry is then inside that folder, its commands run there, and the source archive on the release
  holds that folder only. A `windows` or `android` block may name a folder of its own;
- `author`, `license`, `reviewed` (the date of the review), `summary`: for the release text;
- `nonCommercialData`: data the add-on can download whose license allows non-commercial use only (the release text
  names each, with the sentence the rules ask for);
- `build`: the Ubuntu packages, the folders kept between runs (`cachePaths`, renewed when a file of `cacheKeyFiles`
  changes) and the commands that fill them (`setup`), the test data (a release of the add-on's repository, or since
  2026-11-15 `downloads`: several releases, each with `patterns` for some of its files; their files in one folder, given
  to the checks in `variable`), the commands that build, check and package (`steps`, run in the add-on's source
  with `QT_DIR`, `OUT` and `SHOTS` set; the package goes to `$OUT`), the system libraries the package carries
  (`systemLibraries`: their Ubuntu source goes onto the release) and the libraries built from source whose exact
  commits the release names (`dependencySources`).
- `windows` (2026-11-16, optional): the add-on for Quillbinder for Windows, built by the workflow "Official add-on for
  Windows" (`.github/workflows/addon-release-windows.yml`): its own reviewed `commit` (and `reviewed`), and a `build`
  as above but without Ubuntu packages and system libraries; its commands run in Git Bash inside MSVC's environment,
  with `QT_DIR` (Qt for MSVC), `OUT` and `SHOTS` as Windows paths; Microsoft's redistributable DLLs the package carries
  are named in `redistributables` (they have no source; the release's SOURCES.txt names them). The package must name
  `"platform": "windows-x64"`; its release is `addon-<id>-<version>-windows`. `tools/addon-entry.py ... --windows` reads
  this block.
- `android` (2026-11-16, optional): the add-on for Quillbinder for Android's tablets (arm64), built by the workflow
  "Official add-on for Android" (`.github/workflows/addon-release-android.yml`) on Ubuntu: its own reviewed `commit`
  (and `reviewed`), and a `build` as for Windows; its commands run with `QT_DIR` (Qt for Android, arm64), `QT_HOST_DIR`
  (Qt for the build machine), `ANDROID_SDK_ROOT`, `ANDROID_NDK_ROOT` (the NDK of `QB_NDK` in the workflow), `OUT` and `SHOTS`. Libraries
  of the NDK's toolchain linked into the add-on are named in `toolchainLibraries` (`name`, `source`). The package must
  name `"platform": "android-arm64"` and the runtime of Quillbinder for Android (`APP_RUNTIME` of its build in
  `CMakeLists.txt`, `android-qt6.12-r2` since Android 0.93.0), need 0.91.3 or
  later, and its libraries may need only Android's own, the C++ library and Qt's; its release is
  `addon-<id>-<version>-android`. `tools/addon-entry.py ... --android` reads this block (its package check needs
  `READELF`, the NDK's `llvm-readelf`).

These files are written by the project, not by the add-on's author: the commands in them run in the build job, which
has no signing key.
