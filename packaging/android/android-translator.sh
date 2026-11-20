#!/usr/bin/env bash
# Android 0.92.0: the add-on Translation and Proofreading (addons/translation-proofreading, one of the project's own
# add-ons since 0.92.0) for one Android ABI, built with The Qt Company's Qt for Android as the APK is, and packed as
# its package: the APK carries it (assets/full-addons, installed at the first start of a version, as the full version
# on Linux and Windows does) and the release publishes it on its own.
#
#   packaging/android/android-translator.sh <abi: arm64-v8a | x86_64> <out dir>
#     -> <out dir>/translation-proofreading-android-<arm64|x86_64>.qbpkg (unsigned: the release workflow signs it)
#
# Needs: QT_ANDROID_BASE, ANDROID_SDK_ROOT, ANDROID_NDK_ROOT (the same as packaging/android/android-ci.sh), ninja, git.
# The engine libraries (CTranslate2 4.9.0 with Ruy, SentencePiece 0.2.1) are built by the add-on's
# packaging/build-deps-android.sh (10 to 20 minutes per ABI) into the add-on's deps-android/<abi>/install, their
# sources into its deps/; when those folders hold them already (the workflows keep both in a cache named with that
# script's hash), they are not built again. The add-on's code: its own CMake project in source/.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
ABI=${1:?abi}
OUT=${2:?out dir}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
SRC=$ROOT/addons/translation-proofreading/source
case "$ABI" in
  arm64-v8a) qtarch=android_arm64_v8a; short=arm64 ;;
  x86_64)    qtarch=android_x86_64;    short=x86_64 ;;
  *) echo "android-translator.sh: unknown ABI $ABI" >&2; exit 2 ;;
esac
qtver=$(sed -n 's/^set(QB_QT_VERSION *"\(.*\)")/\1/p' "$ROOT/CMakeLists.txt")
QT_DIR="${QT_ANDROID_BASE:?}/$qtver/$qtarch"
QT_HOST_DIR="$QT_ANDROID_BASE/$qtver/gcc_64"
NDK=${ANDROID_NDK_ROOT:?}
SDK=${ANDROID_SDK_ROOT:?}
cmp -s "$ROOT/addons/translation-proofreading/manifest.json" "$SRC/packaging/manifest.json" \
  || { echo "android-translator.sh: manifest.json and source/packaging/manifest.json differ" >&2; exit 1; }

DEPS=$SRC/deps-android/$ABI/install
if [ -f "$DEPS/lib/libctranslate2.a" ] && [ -f "$DEPS/lib/libsentencepiece.a" ] && [ -d "$SRC/deps/CTranslate2" ]; then
  echo "engine libraries: from the cache ($(cat "$DEPS/built-for.txt" 2>/dev/null))"
else
  echo "engine libraries: building them ($ABI)"
  bash "$SRC/packaging/build-deps-android.sh" "$NDK" "$ABI"
  rm -rf "$SRC/deps-android/$ABI/b-ct2" "$SRC/deps-android/$ABI/b-spm"
fi

B=$ROOT/build-translator-android-$ABI
cmake -S "$SRC" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$QT_DIR/lib/cmake/Qt6/qt.toolchain.cmake" -DQT_HOST_PATH="$QT_HOST_DIR" \
  -DANDROID_SDK_ROOT="$SDK" -DANDROID_NDK_ROOT="$NDK" -DTP_DEPS_DIR="$DEPS" -DTP_TEST_SWITCHES=OFF
cmake --build "$B"
NM="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-nm" bash "$SRC/tests/check-exports.sh" "$B"
STAGED=$(mktemp -d)
bash "$SRC/packaging/make-addon-android.sh" "$B" "$STAGED" "$ABI" "$NDK"
pkg=$(ls "$STAGED"/translation-proofreading-*-android-"$short".qbpkg)
mv "$pkg" "$OUT/translation-proofreading-android-$short.qbpkg"
rm -rf "$STAGED"
# its libraries: for this processor, their segments at 16 KB borders (Android 15 and later may use 16 KB pages), and
# needing only Android's own libraries, the app's C++ library and Qt's libraries of this ABI
READELF="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf"
LIBS=$(mktemp -d)
unzip -q -o "$OUT/translation-proofreading-android-$short.qbpkg" '*.so' -d "$LIBS"
want=$([ "$ABI" = arm64-v8a ] && echo AArch64 || echo X86-64)
bad=0
while IFS= read -r so; do
  m=$("$READELF" -h "$so" | sed -n 's/^ *Machine: *//p')
  a=$("$READELF" -lW "$so" | awk '$1 == "LOAD" { print $NF }' | sort -u | tr '\n' ' ')
  needs=$("$READELF" -dW "$so" | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p' | tr '\n' ' ')
  echo "  $(basename "$so"): $m, LOAD $a, needs $needs"
  case "$m" in *"$want"*) ;; *) echo "  FAIL: $(basename "$so") is not for $ABI"; bad=1 ;; esac
  case " $a" in *" 0x1000 "*) echo "  FAIL: $(basename "$so") has 4 KB segments"; bad=1 ;; esac
  for n in $needs; do
    case "$n" in
      libc.so | libm.so | libdl.so | liblog.so | libandroid.so | libc++_shared.so | libGLESv2.so | libEGL.so) ;;
      libQt6*_"$ABI".so) ;;
      libqb-tp-engine.so) ;;
      *) echo "  FAIL: $(basename "$so") needs $n"; bad=1 ;;
    esac
  done
done < <(find "$LIBS" -name '*.so' | sort)
rm -rf "$LIBS"
[ $bad -eq 0 ] || { echo "android-translator.sh: the package's libraries do not fit Android" >&2; exit 1; }
echo "package: $OUT/translation-proofreading-android-$short.qbpkg $(stat -c %s "$OUT/translation-proofreading-android-$short.qbpkg") bytes"
