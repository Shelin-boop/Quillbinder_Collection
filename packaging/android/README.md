# Android

The Android version is being built (APK only; trial builds since 2026-11-14, at Linux 0.91.3 since 2026-11-16). It runs parallel to the
Linux version, as the Windows version does: the same source, its own build, its own files here. Nothing in this
folder is used by the Linux or the Windows build.

| File | What it is |
|------|------------|
| `android-ci.sh` | The build on GitHub's Linux machine, started by `.github/workflows/android.yml` (a push to a branch `android/...`): the program for one Android ABI with The Qt Company's Qt for Android, the APK, and the checks of the APK (app id, version code, Android versions, permissions, libraries, signature, 16 KB alignment). |
| `android-qt.sh` | Qt for the build: The Qt Company's binaries for the build machine and for Android (two ABIs), installed with aqtinstall; it tells aqtinstall where Qt's repository keeps Qt PDF for Android. |
| `android-hunspell.sh` | Hunspell, the spelling library, built from its source with the NDK's compiler. |
| `android-openssl.sh` | OpenSSL 3.5.9 (archive pinned by SHA-256), built with the NDK as `libcrypto_3.so` and `libssl_3.so`: Qt's only way to https on Android (File > Import > Web Page). |
| `android-deps-pack.sh` | Hunspell and OpenSSL as ready-made packs per ABI (D1, 2026-11-16): `get`, `publish`, `recipe`. A pack lies on a pre-release `deps-<name>-android-<recipe>`; the recipe is the hash of its script, the ABI and the NDK's version; the pins are in `QB_ANDROID_PACKS` of `android.yml` and `android-system-test.yml`. Without a pin a run builds the library, publishes its pack and prints the `pin:` line in `summary.txt` (x86_64: `emu-build-summary.txt`). |
| `android-system-test.sh` | The system test on emulated tablets (`.github/workflows/android-system-test.yml`, a push to `android-st/...`): builds Android's own copy of the system test (`tests/android/systemtest`) as an app and starts it once per case; sub-runs (`<case>@<variant>`), the app killed or sent away from outside (needs K, H), the translation add-on and a model fetched for A-A01 (needs T). What a run does: `system-test-run.txt`; cases a tablet cannot run yet: `system-test-skip.txt`. Results on the pre-release `android-st`. |
| `android-resources.py`, `collect-licenses-android.py` | The program's resources for Android (`resources-android.qrc`) and its list of licenses (`resources/licenses-android`). |
| `android-emulator.sh` | The start check: the program built for x86_64, installed and started on an emulated tablet with Android 14; the toolbar rows at other screen sizes, swipes, two fingers and the lines between panes as raw touch events, every menu and three windows opened while the on-screen keyboard is up, the handles of a text; pictures of the screen and the system's log. |
| `package/` | The package's own files, which Qt's `androiddeployqt` puts into the APK: `AndroidManifest.xml` (Qt 6.12.0's own manifest with the changes named at its top) and the launcher icon in five sizes (`res/mipmap-*/ic_launcher.png`). |
| `android-trial-key.sh` | The key that signs trial builds and the system test's app: a kept key named by `QB_ANDROID_TRIAL_KEYSTORE` and `QB_ANDROID_TRIAL_KEYSTORE_PASS`, or without them a key made for the run alone (such a trial build cannot be installed over the one before). Never for a public APK. |
| `release-key.txt` | The public app's key (2026-11-16): its app id `io.github.shelin_boop.quillbinder.app` and the SHA-256 of its certificate. The key itself is the writer's and a secret of this repository (`QB_ANDROID_KEYSTORE`, `QB_ANDROID_KEYSTORE_PASS`); `.github/workflows/release-android.yml` checks it against this file. |

What the APK is made of is in `CMakeLists.txt`, inside `if(ANDROID)`: the app's id, name, icon, version name and
version code, and the Android versions (Android 9 and later). The results of a run are on the pre-release
`android-ci` (`summary.txt` first).

Android's own interface code is in `src/android` and is compiled into the Android program only (`CMakeLists.txt`,
inside `if(ANDROID)`): `MobileBars.cpp` (toolbar rows that follow the screen's width), `MobileTouch.cpp` (a finger
scrolls and taps, two fingers zoom, a finger moves the line between two panes), `MobileHandles.cpp` (the drop under
the cursor and a selection's handles: only where their place in the text can be seen, the drop only for a moment
after a tap) and `MobileWindows.cpp` (Qt 6.12.0 ends the program when a window is first drawn while Android's
keyboard program calls into it: each window's surface is made ahead of its first picture; to be taken out with the
Qt version that has this repaired). On a desktop the same code, but for `MobileWindows.cpp`, is checked by the test
program `tests/mobile_tests.cpp`, in windows of a tablet's size and with simulated fingers; it is built only when
asked for:

    cmake -S . -B build -G Ninja -DQB_TEST_SWITCHES=ON -DQB_MOBILE_PREVIEW=ON
    cmake --build build --target qb_mobile_tests
    xvfb-run -a -s "-screen 0 1920x1200x24" build/tests/qb_mobile_tests /tmp/shots

The launcher icons are the program icon of Quillbinder (`resources/app/quillbinder.svg`, drawn for Quillbinder,
under the same license as the rest of Quillbinder's own files), made with
`rsvg-convert -w <size> -h <size>` at 48, 72, 96, 144 and 192 px.

To build by hand (Linux, with Qt 6.12.0 for Android and for the build machine side by side in one folder, the
Android SDK with NDK 27.2.12479018, and JDK 21):

    QT_ANDROID_BASE=~/Qt ANDROID_SDK_ROOT=<sdk> ANDROID_NDK_ROOT=<sdk>/ndk/27.2.12479018 JAVA_HOME=<jdk 21> \
      packaging/android/android-ci.sh results arm64-v8a

**The public APK** (since 0.91.3, 2026-11-16): `.github/workflows/release-android.yml`, started
by hand when the writer calls for an Android version (Linux leads: a Linux release at the same functional level must
exist). It builds the APK with `android-ci.sh` and `QB_ANDROID_RELEASE=1` (the permanent app id, the version name
"<version> experimental", unsigned), aligns it to 16 KB and signs it with the project's Android key in a job of its own
(only the runner's Android build tools and Java run there), checks the signer, the id and the version, fetches the
third-party sources the Android license list names that the release "third-party-sources" lacks (OpenSSL), signs the
checksum list `SHA256SUMS-android.txt` with the project's release key, and publishes the release
`v<version>-android-experimental` (never "Latest") and the download page's Android card. File name:
`Quillbinder-<version>-experimental-android-arm64.apk`; version code as for trial builds, with the workflow's run number
as the build number.
