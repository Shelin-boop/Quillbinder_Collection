#!/usr/bin/env bash
# Android build on GitHub Actions (.github/workflows/android.yml; port step A0, the trial build, 2026-11-14).
# Builds the program for one Android ABI with The Qt Company's Qt for Android, packs it with Qt's libraries into
# an APK signed with the key for trial builds (packaging/android/android-trial-key.sh), and looks at the APK.
# As packaging/windows/win-ci.sh does: never stops at the first failure; every step writes its result into the
# results folder, which the workflow puts on the pre-release "android-ci", where a Claude session can read it (the
# session cannot read the Actions API).
#
#   packaging/android/android-ci.sh <results dir> [abi: arm64-v8a (default) | x86_64]
#
# Needs: QT_ANDROID_BASE (the folder with Qt's version folder: <base>/<version>/android_arm64_v8a, .../gcc_64),
# ANDROID_SDK_ROOT, ANDROID_NDK_ROOT, JAVA_HOME (JDK 21), ninja; optional sccache, QB_ANDROID_BUILD (build number).
#
# The public APK (2026-11-16, .github/workflows/release-android.yml): QB_ANDROID_RELEASE=1
# builds it with the permanent app id io.github.shelin_boop.quillbinder.app, the version name QB_ANDROID_VERSION_NAME
# and the file name Quillbinder-<version>-<QB_ANDROID_LABEL>-android-<abi>.apk, and leaves it unsigned: the
# workflow's job "sign-apk" signs it with the project's Android key (the key for trial builds is never used for it).
# QB_ANDROID_PACKS_PUBLISH=0: a Hunspell or OpenSSL built here is not published as a pack (the release workflow).
set -u
OUT=${1:-results}
ABI=${2:-arm64-v8a}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
SUM="$OUT/summary.txt"
: > "$SUM"
: > "$OUT/errors.txt"
say() { echo "$*" | tee -a "$SUM"; }
t0=$(date +%s)
mins() { echo "$(( ($(date +%s) - t0) / 60 )) min"; }

ver=$(sed -n 's/^set(APP_VERSION_ANDROID "\(.*\)")/\1/p' CMakeLists.txt)   # 0.93.0: Android's own version line
RELEASE=${QB_ANDROID_RELEASE:-0}
if [ "$RELEASE" = 1 ]; then
  APPID=io.github.shelin_boop.quillbinder.app
else
  APPID=io.github.shelin_boop.quillbinder.trial
fi
qtver=$(sed -n 's/^set(QB_QT_VERSION *"\(.*\)")/\1/p' CMakeLists.txt)
build=${QB_ANDROID_BUILD:-0}
build=$(( build % 1000 ))
case "$ABI" in
  arm64-v8a) qtarch=android_arm64_v8a; short=arm64 ;;
  x86_64)    qtarch=android_x86_64;    short=x86_64 ;;
  *) say "result: unknown ABI $ABI"; exit 0 ;;
esac
QT_TARGET="${QT_ANDROID_BASE:?}/$qtver/$qtarch"
QT_HOST="$QT_ANDROID_BASE/$qtver/gcc_64"
SDK=${ANDROID_SDK_ROOT:?}
NDK=${ANDROID_NDK_ROOT:?}
# the SDK build tools Qt's documentation names (the workflow's QB_BUILD_TOOLS), else the newest there
BT="$SDK/build-tools/${QB_BUILD_TOOLS:-none}"
[ -d "$BT" ] || BT=$(ls -d "$SDK"/build-tools/* 2>/dev/null | sort -V | tail -1)

say "commit: ${GITHUB_SHA:-local}"
say "version: $ver, build $build, abi $ABI"
say "qt: $qtver ($QT_TARGET)"
say "ndk: $(sed -n 's/^Pkg.Revision *= *//p' "$NDK/source.properties" 2>/dev/null)"
say "build tools: $(basename "${BT:-none}")"
say "jdk: $("${JAVA_HOME:-/usr}/bin/java" -version 2>&1 | head -1)"
say "cmake: $(cmake --version | head -1), ninja $(ninja --version 2>/dev/null)"

LAUNCH=""
if command -v sccache > /dev/null 2>&1; then
  LAUNCH="-DCMAKE_C_COMPILER_LAUNCHER=sccache -DCMAKE_CXX_COMPILER_LAUNCHER=sccache"
fi

# ---- Hunspell for this ABI (cached by the workflow); without it the program is built without spell checking
# (D1, 0.91.3: from its ready-made pack when android.yml pins one for this recipe; else built and its pack published)
HUN="$PWD/.hunspell/$ABI"
if bash packaging/android/android-deps-pack.sh get "hunspell-$ABI" "$NDK" "$HUN" > "$OUT/hunspell-pack.log" 2>&1; then
  rc=0
else
  bash packaging/android/android-hunspell.sh "$NDK" "$ABI" "$HUN" > "$OUT/hunspell.log" 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    if [ "${QB_ANDROID_PACKS_PUBLISH:-1}" != 0 ]; then
      bash packaging/android/android-deps-pack.sh publish "hunspell-$ABI" "$NDK" "$HUN" >> "$OUT/hunspell-pack.log" 2>&1 || echo "publish failed" >> "$OUT/hunspell-pack.log"
    fi
  fi
fi
say "hunspell pack: $(grep -E '^(from the pack|no pack|pin:|pack there|publish failed|the pack)' "$OUT/hunspell-pack.log" | tr '\n' ' ')"
say "hunspell: rc=$rc, $(cat "$HUN/version.txt" 2>/dev/null), source sha256 $(cat "$HUN/source.sha256" 2>/dev/null) ($(mins))"
HUNARGS=""
if [ $rc -eq 0 ] && [ -f "$HUN/lib/libhunspell-1.7.a" ]; then
  HUNARGS="-DQB_HUNSPELL_INCLUDE=$HUN/include -DQB_HUNSPELL_LIB=$HUN/lib/libhunspell-1.7.a"
else
  tail -15 "$OUT/hunspell.log" >> "$SUM"
fi

# ---- OpenSSL for this ABI (cached by the workflow): Qt's only way to https on Android (File > Import > Web Page).
# Without it the program is built all the same, and the import of a web page tells that it cannot connect.
SSL="$PWD/.openssl/$ABI"
if bash packaging/android/android-deps-pack.sh get "openssl-$ABI" "$NDK" "$SSL" > "$OUT/openssl-pack.log" 2>&1; then
  rc=0
else
  bash packaging/android/android-openssl.sh "$NDK" "$ABI" "$SSL" > "$OUT/openssl.log" 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    if [ "${QB_ANDROID_PACKS_PUBLISH:-1}" != 0 ]; then
      bash packaging/android/android-deps-pack.sh publish "openssl-$ABI" "$NDK" "$SSL" >> "$OUT/openssl-pack.log" 2>&1 || echo "publish failed" >> "$OUT/openssl-pack.log"
    fi
  fi
fi
say "openssl pack: $(grep -E '^(from the pack|no pack|pin:|pack there|publish failed|the pack)' "$OUT/openssl-pack.log" | tr '\n' ' ')"
say "openssl: rc=$rc, $(cat "$SSL/version.txt" 2>/dev/null), source sha256 $(cat "$SSL/source.sha256" 2>/dev/null) ($(mins))"
SSLARGS=""
if [ $rc -eq 0 ] && [ -f "$SSL/lib/libssl_3.so" ]; then
  SSLARGS="-DQB_ANDROID_OPENSSL=$SSL"
else
  tail -15 "$OUT/openssl.log" >> "$SUM"
fi

# ---- the spelling dictionaries (English and German, the ones of the Windows program: LibreOffice's, at a fixed tag
# and checked against their sums) become assets of the APK; the program copies them into its data folder
ASSETS="$PWD/.apk-assets"
rm -rf "$ASSETS"
mkdir -p "$ASSETS"
bash packaging/windows/win-dictionaries.sh "$ASSETS" > "$OUT/dictionaries.log" 2>&1
rc=$?
# (the note beside the license texts is written for the Windows program)
[ -f "$ASSETS/dictionaries/licenses/README.txt" ] && sed -i -e '1s/.*/Spelling dictionaries of Quillbinder for Android/' -e '2s/.*/================================================/' "$ASSETS/dictionaries/licenses/README.txt"
say "dictionaries: rc=$rc, $(ls "$ASSETS/dictionaries" 2>/dev/null | grep -c '\.dic$') languages, $(du -sk "$ASSETS" | cut -f1) KB ($(mins))"
[ $rc -ne 0 ] && tail -8 "$OUT/dictionaries.log" >> "$SUM"

# ---- Translation and Proofreading (Android 0.92.0): the APK carries its package in assets/full-addons, installed at
# the first start of a version, as the full version on Linux and Windows does. The release workflow hands over the
# package its job sign-translator signed (QB_ANDROID_FULL_ADDONS); a trial build makes it here (unsigned).
mkdir -p "$ASSETS/full-addons"
if [ -n "${QB_ANDROID_FULL_ADDONS:-}" ]; then
  cp "$QB_ANDROID_FULL_ADDONS"/translation-proofreading-android-$short.qbpkg "$ASSETS/full-addons/" 2>/dev/null
  rc=$?
  say "translator: rc=$rc, the package of the release ($(ls "$ASSETS/full-addons" | tr '\n' ' '))"
else
  bash packaging/android/android-translator.sh "$ABI" "$OUT/addons" > "$OUT/translator.log" 2>&1
  rc=$?
  [ $rc -eq 0 ] && cp "$OUT/addons/translation-proofreading-android-$short.qbpkg" "$ASSETS/full-addons/"
  say "translator: rc=$rc, $(grep '^package:' "$OUT/translator.log" | sed 's|.*/||') ($(grep -c 'FAIL' "$OUT/translator.log") FAIL lines) ($(mins))"
  [ $rc -ne 0 ] && { echo "---- translator.log (end)"; tail -30 "$OUT/translator.log"; } >> "$OUT/errors.txt"
fi

# ---- the spelling dictionaries of the nine other languages (Android 0.95.0, to-do item 102): the add-ons of Linux and
# Windows (addons/dictionary-*, LibreOffice's files at a pinned tag, checked against sources.txt by their stage.sh),
# without a platform and without libraries, so the same content installs on the tablet. Not in the APK (English and
# German are its assets): a writer installs the one language. The release names them <id>-android.qbpkg.
mkdir -p "$OUT/addons"
dmade=0; dfailed=""
for dir in addons/dictionary-*/; do
  id=$(basename "$dir"); st="$OUT/stage/$id"
  rm -rf "$st"; mkdir -p "$st/payload"; cp "$dir/manifest.json" "$st/"
  if bash "$dir/stage.sh" "$OUT" "$st" >> "$OUT/dictionary-addons.log" 2>&1 \
     && python3 -I tools/make-package.py addon "$st" "$OUT/addons/$id-android.qbpkg" >> "$OUT/dictionary-addons.log" 2>&1; then
    dmade=$((dmade + 1))
  else
    dfailed="$dfailed $id"
  fi
done
say "dictionary add-ons: $dmade made${dfailed:+, failed:$dfailed} ($(mins))"
[ -n "$dfailed" ] && { echo "---- dictionary-addons.log (end)"; tail -20 "$OUT/dictionary-addons.log"; } >> "$OUT/errors.txt"

# ---- configure: the program, and what the Linux program gets as add-on packages or beside it: the writing tools
# (Name Generator, Scratch Pad, Text Statistics), the Word 97-2003 importer and the Markdown exporter go into the
# APK as libraries. No tests, no developer tools. The APK is signed with the key for trial builds.
# Memory pages of 16 KB (Android 15 and later may use them): the NDK's switch and the linker's option itself, so
# that the program's library is aligned for them as Qt's libraries are.
SIGNARGS="-DQT_ANDROID_SIGN_APK=OFF"
VNAMEARGS=()
if [ "$RELEASE" = 1 ]; then
  [ -n "${QB_ANDROID_VERSION_NAME:-}" ] && VNAMEARGS=("-DQB_ANDROID_VERSION_NAME=$QB_ANDROID_VERSION_NAME")
else
  . packaging/android/android-trial-key.sh
  SIGNARGS="-DQT_ANDROID_SIGN_APK=ON"
fi
B="build-android-$ABI"
cmake -S . -B "$B" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$QT_TARGET/lib/cmake/Qt6/qt.toolchain.cmake" \
  -DQT_HOST_PATH="$QT_HOST" \
  -DANDROID_SDK_ROOT="$SDK" -DANDROID_NDK_ROOT="$NDK" \
  $SIGNARGS -DQB_ANDROID_PACKAGE="$APPID" "${VNAMEARGS[@]}" \
  -DQT_ANDROID_SDK_BUILD_TOOLS_REVISION="$(basename "$BT")" \
  -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON \
  -DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384 -DCMAKE_MODULE_LINKER_FLAGS=-Wl,-z,max-page-size=16384 \
  -DQB_ANDROID_BUILD="$build" \
  $LAUNCH $HUNARGS $SSLARGS \
  -DQB_ANDROID_ASSETS="$ASSETS" \
  -DQB_BUILD_TESTS=OFF -DQB_TEST_SWITCHES=OFF -DQB_BUILD_TOOLS=OFF -DQB_BUILD_TOOL_PLUGINS=ON \
  -DQB_BUILD_IMPORTER_PLUGINS=ON -DQB_BUILD_MEDIA_PLUGIN=OFF -DQB_BUILD_EXAMPLE_PLUGIN=ON \
  > "$OUT/configure.log" 2>&1
rc=$?
say "configure: rc=$rc ($(mins))"
grep -iE 'hunspell|CMake (Error|Warning)|error:' "$OUT/configure.log" | head -20 >> "$SUM"
if [ $rc -ne 0 ]; then
  tail -60 "$OUT/configure.log" > "$OUT/errors.txt"
  say "result: configure failed"
  exit 0
fi

# ---- build the program (-k 0: go on after errors, so one run shows all of them)
cmake --build "$B" --target quillbinder -- -k 0 > "$OUT/build.log" 2>&1
rc=$?
grep -E ': (fatal )?error: |^ld(\.lld)?: error|: undefined (reference|symbol)|^FAILED:|ninja: (error|build stopped)' "$OUT/build.log" \
  | sort -u > "$OUT/errors.txt"
lib=$(find "$B" -maxdepth 1 -name "libquillbinder_$ABI.so" | head -1)
say "build: rc=$rc, error lines $(grep -c 'error' "$OUT/errors.txt"), failed files $(grep -c '^FAILED:' "$OUT/errors.txt"), compiler warnings $(grep -c ': warning: ' "$OUT/build.log") ($(mins))"
command -v sccache > /dev/null 2>&1 && sccache --show-stats > "$OUT/sccache.txt" 2>&1
if [ $rc -ne 0 ] || [ -z "$lib" ]; then
  # the first error of each failed file with a few lines around it, for reading without the whole log
  grep -nE -m 60 -A4 ': (fatal )?error: ' "$OUT/build.log" | cut -c1-400 > "$OUT/errors-context.txt"
  say "result: build failed"
  exit 0
fi
say "program library: $(basename "$lib") $(stat -c %s "$lib") bytes"

# ---- the APK (Qt's androiddeployqt and Gradle; Gradle and the Android build plugins are downloaded here)
cmake --build "$B" --target quillbinder_make_apk > "$OUT/apk.log" 2>&1
rc=$?
apk="$B/android-build/quillbinder.apk"
# (unsigned, the public APK: where Gradle leaves it when androiddeployqt did not copy it)
[ -f "$apk" ] || apk=$(find "$B/android-build/build/outputs/apk" -name '*.apk' 2>/dev/null | sort | head -1)
say "apk: rc=$rc ($(mins))"
if [ $rc -ne 0 ] || [ ! -f "$apk" ]; then
  { grep -nE -B2 -A12 'FAILURE:|What went wrong|error:|Error:|Exception|Cannot|not found|failed' "$OUT/apk.log" | cut -c1-400 | head -150
    echo "---- the end of apk.log"; tail -40 "$OUT/apk.log"; } > "$OUT/errors.txt"
  ls -R "$B/android-build/build/outputs" > "$OUT/apk-outputs.txt" 2>&1
  say "result: apk failed"
  exit 0
fi
# for the license list: what Qt's build names as its parts, and the Java libraries Gradle put into the APK
ls "$QT_TARGET/sbom" > "$OUT/qt-sbom-files.txt" 2>&1
grep -hE '^(PackageName|PackageVersion|PackageLicenseConcluded|PackageLicenseDeclared|PackageDownloadLocation):' "$QT_TARGET"/sbom/*.spdx 2>/dev/null \
  | cut -c1-200 > "$OUT/qt-sbom.txt"
( cd "$B/android-build" && ./gradlew -q dependencies --configuration releaseRuntimeClasspath ) > "$OUT/gradle-dependencies.txt" 2>&1
name="Quillbinder-$ver-trial$build-android-$short.apk"
[ "$RELEASE" = 1 ] && name="Quillbinder-$ver-${QB_ANDROID_LABEL:-experimental}-android-$short.apk"
cp "$apk" "$OUT/$name"
say "apk file: $name $(stat -c %s "$OUT/$name") bytes, sha256 $(sha256sum "$OUT/$name" | cut -d' ' -f1)"

# ---- look at the APK: what Android will read from it, the signature, the libraries' alignment
fails=0
ok()  { say "  ok: $*"; }
bad() { say "  FAIL: $*"; fails=$((fails + 1)); }
"$BT/aapt2" dump badging "$OUT/$name" > "$OUT/apk-badging.txt" 2>&1
unzip -l "$OUT/$name" > "$OUT/apk-files.txt" 2>&1
say "checks:"
pkg=$(sed -n "s/^package: name='\([^']*\)'.*/\1/p" "$OUT/apk-badging.txt")
code=$(sed -n "s/^package: .*versionCode='\([^']*\)'.*/\1/p" "$OUT/apk-badging.txt")
vname=$(sed -n "s/^package: .*versionName='\([^']*\)'.*/\1/p" "$OUT/apk-badging.txt")
minsdk=$(sed -n "s/^\(sdkVersion\|minSdkVersion\):'\([^']*\)'.*/\2/p" "$OUT/apk-badging.txt" | head -1)
target=$(sed -n "s/^targetSdkVersion:'\([^']*\)'.*/\1/p" "$OUT/apk-badging.txt")
label=$(sed -n "s/^application-label:'\([^']*\)'.*/\1/p" "$OUT/apk-badging.txt")
abis=$(sed -n "s/^native-code: //p" "$OUT/apk-badging.txt")
perms=$(sed -n "s/^uses-permission: name='\([^']*\)'.*/\1/p" "$OUT/apk-badging.txt" | tr '\n' ' ')
say "  package $pkg, version code $code, version name '$vname', label '$label', min SDK $minsdk, target SDK $target, native code $abis"
say "  permissions: ${perms:-none}"
if [ "$RELEASE" = 1 ]; then
  [ "$pkg" = "$APPID" ] && ok "the public app's permanent id ($pkg)" || bad "app id: $pkg, expected $APPID"
  [ -z "${QB_ANDROID_VERSION_NAME:-}" ] || [ "$vname" = "$QB_ANDROID_VERSION_NAME" ] && ok "version name '$vname'" \
    || bad "version name '$vname', expected '$QB_ANDROID_VERSION_NAME'"
else
  [ "$pkg" = "$APPID" ] && ok "the trial build's own app id" || bad "app id: $pkg"
fi
IFS=. read -r vmaj vmin vpat <<< "$ver"
want=$(( vmaj * 100000000 + vmin * 100000 + vpat * 1000 + build ))
[ "$code" = "$want" ] && ok "version code $code" || bad "version code $code, expected $want"
[ "$minsdk" = 28 ] && [ "$target" = 36 ] && ok "Android 9 (API 28) and later, built for API 36" || bad "min SDK $minsdk, target SDK $target"
[ "$label" = "Quillbinder" ] && ok "the app's name is Quillbinder" || bad "the app's name: $label"
echo "$abis" | grep -q "'$ABI'" && [ "$(echo "$abis" | wc -w)" = 1 ] && ok "libraries for $ABI only" || bad "native code: $abis"
# The permissions: the access to the tablet's shared folders (projects and backups in Documents/Quillbinder) and
# nothing else; the one androidx adds for the app's own use (its name starts with the app's id) does not count.
other=$(echo "$perms" | tr ' ' '\n' | grep -v "^$pkg\." | grep -v '^$' | sort -u | tr '\n' ' ')
wantp="android.permission.INTERNET android.permission.MANAGE_EXTERNAL_STORAGE android.permission.READ_EXTERNAL_STORAGE android.permission.WRITE_EXTERNAL_STORAGE "
[ "$other" = "$wantp" ] && ok "the app asks only for the access to the shared folders (all files; Android 9 and 10: storage) and for the internet (to import a web page)" \
  || bad "permissions asked for: $other"
grep -q "maxSdkVersion='29'" "$OUT/apk-badging.txt" && ok "the storage permission is asked for on Android 9 and 10 only" \
  || bad "the storage permission has no upper Android version"
grep -q "lib/$ABI/libquillbinder_$ABI.so" "$OUT/apk-files.txt" && ok "the program's library is in the APK" || bad "the program's library is missing"
for l in Core Gui Widgets Svg Xml PrintSupport Pdf PdfWidgets Network; do
  grep -q "lib/$ABI/libQt6${l}_$ABI.so" "$OUT/apk-files.txt" || bad "Qt library missing in the APK: Qt6$l"
done
grep -q "lib/$ABI/libplugins_platforms_qtforandroid_$ABI.so" "$OUT/apk-files.txt" && ok "Qt's libraries and its Android platform plugin are in the APK" \
  || bad "Qt's Android platform plugin is missing"
# what the Linux program gets as add-ons: the plug-ins as libraries, the dictionaries as assets
missing=""
for l in libmarkdown-exporter libqb-name-generator libqb-scratch-pad libqb-text-statistics libqb-import-word97 libqb-import-web; do
  grep -q "lib/$ABI/$l.so" "$OUT/apk-files.txt" || missing="$missing $l"
done
[ -z "$missing" ] && ok "the writing tools, the two importers (Word 97-2003, web pages) and the Markdown exporter are in the APK" \
  || bad "plug-ins missing in the APK:$missing"
grep -q "lib/$ABI/libssl_3.so" "$OUT/apk-files.txt" && grep -q "lib/$ABI/libcrypto_3.so" "$OUT/apk-files.txt" \
  && ok "OpenSSL's two libraries are in the APK (https for the import of web pages)" || bad "OpenSSL's libraries are missing in the APK"
missing=""
for l in en_US en_GB de_DE de_AT de_CH; do
  grep -q "assets/dictionaries/$l.dic" "$OUT/apk-files.txt" && grep -q "assets/dictionaries/$l.aff" "$OUT/apk-files.txt" || missing="$missing $l"
done
[ -z "$missing" ] && ok "the spelling dictionaries are in the APK (English and German)" || bad "dictionaries missing in the APK:$missing"
grep -q "assets/full-addons/translation-proofreading-android-$short.qbpkg" "$OUT/apk-files.txt" \
  && ok "Translation and Proofreading is in the APK (assets/full-addons)" || bad "the translator's package is missing in the APK"
# (Android 0.95.0) the nine dictionary add-ons: no platform, no runtime, no libraries, a .dic and an .aff each, and a
# program version they need that this APK has
dcheck=$(python3 -I - "$OUT/addons" "$ver" <<'PY'
import glob, json, os, sys, zipfile
def key(v): return [int(x) for x in str(v).split(".") if x.isdigit()]
bad = []
pkgs = sorted(glob.glob(os.path.join(sys.argv[1], "dictionary-*-android.qbpkg")))
for p in pkgs:
    try:
        with zipfile.ZipFile(p) as z:
            m = json.loads(z.read("manifest.json"))
            names = z.namelist()
    except Exception as e:
        bad.append("%s: %s" % (os.path.basename(p), e)); continue
    paths = [f.get("path", "") for f in m.get("files") or []]
    if m.get("platform") or m.get("runtime") or any(x.endswith(".so") for x in paths):
        bad.append("%s: platform/runtime/libraries" % m.get("id"))
    if not any(x.endswith(".dic") for x in paths) or not any(x.endswith(".aff") for x in paths):
        bad.append("%s: no .dic/.aff" % m.get("id"))
    if key(m.get("minApp") or "0") > key(sys.argv[2]):
        bad.append("%s: needs %s" % (m.get("id"), m.get("minApp")))
print("%d %s" % (len(pkgs), "; ".join(bad)))
PY
)
dn=${dcheck%% *}; dbad=${dcheck#* }; [ "$dbad" = "$dcheck" ] && dbad=""
[ "$dn" = 9 ] && [ -z "$dbad" ] && ok "the nine spelling dictionaries as add-ons (no platform, no libraries, for this version)" \
  || bad "dictionary add-ons: $dn packages${dbad:+; $dbad}"
if [ "$RELEASE" = 1 ]; then
  # unsigned here: the job "sign-apk" of the release workflow aligns and signs it with the project's Android key and
  # checks the result (signer, schemes, 16 KB borders)
  "$BT/apksigner" verify "$OUT/$name" > "$OUT/apk-signature.txt" 2>&1 && bad "the APK is signed already (it should be unsigned here)" \
    || ok "unsigned, for the job sign-apk"
else
"$BT/apksigner" verify --print-certs "$OUT/$name" > "$OUT/apk-signature.txt" 2>&1
rc=$?
cert=$(sed -n 's/^Signer #1 certificate SHA-256 digest: //p' "$OUT/apk-signature.txt")
KT=keytool
[ -x "${JAVA_HOME:-}/bin/keytool" ] && KT="$JAVA_HOME/bin/keytool"
keycert=$("$KT" -list -keystore "$QT_ANDROID_KEYSTORE_PATH" -storepass "$QT_ANDROID_KEYSTORE_STORE_PASS" 2>/dev/null \
  | sed -n 's/^Certificate fingerprint (SHA-256): //p' | tr -d ':' | tr 'A-F' 'a-f')
[ $rc -eq 0 ] && [ -n "$cert" ] && [ "$cert" = "$keycert" ] && ok "signed with the key for trial builds ($cert)" \
  || bad "signature: rc=$rc, certificate '$cert', the key's '$keycert'"
fi
# Android 15 and later may run with 16 KB memory pages: uncompressed libraries must sit at 16 KB borders in the APK
# (the unsigned public APK is aligned in the job sign-apk, before it is signed)
"$BT/zipalign" -c -P 16 -v 4 "$OUT/$name" > "$OUT/apk-zipalign.txt" 2>&1
rc=$?
if [ $rc -eq 0 ]; then ok "zipalign: 16 KB borders"
elif [ "$RELEASE" = 1 ]; then say "  note: not at 16 KB borders yet (aligned in the job sign-apk)"
else bad "zipalign -c -P 16: rc=$rc ($(tail -1 "$OUT/apk-zipalign.txt"))"; fi
# ... and the libraries' own segments must be aligned to 16 KB
READELF="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf"
mkdir -p "$OUT/tmp-libs"
unzip -q -o "$OUT/$name" "lib/$ABI/*.so" -d "$OUT/tmp-libs" 2> /dev/null
small=""
for so in "$OUT/tmp-libs/lib/$ABI"/*.so; do
  a=$("$READELF" -lW "$so" 2>/dev/null | awk '$1 == "LOAD" { print $NF }' | sort -u | tr '\n' ' ')
  echo "$(basename "$so") $(stat -c %s "$so") LOAD-align: $a" >> "$OUT/apk-libs.txt"
  case " $a" in *" 0x1000 "*) small="$small $(basename "$so")" ;; esac
done
rm -rf "$OUT/tmp-libs"
[ -z "$small" ] && ok "every library's segments are aligned to 16 KB or more ($(wc -l < "$OUT/apk-libs.txt") libraries)" \
  || bad "libraries with 4 KB segments:$small"

if [ $fails -eq 0 ]; then
  say "result: ok"
else
  say "result: apk built, $fails checks failed"
fi
exit 0
