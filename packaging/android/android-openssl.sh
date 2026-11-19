#!/usr/bin/env bash
# OpenSSL for Android (2026-11-14): libcrypto_3.so and libssl_3.so for one Android ABI, built from OpenSSL's own
# source archive with the NDK's compiler. Qt for Android has one way to make secure connections (https): its TLS
# plug-in for OpenSSL, which loads exactly these two library names from the APK. The program needs them for File >
# Import > Web Page (the Linux program gets its TLS from the system's OpenSSL, the Windows program from Windows).
#
#   packaging/android/android-openssl.sh <ndk folder> <abi: arm64-v8a | x86_64> <prefix>
#
# Result: <prefix>/lib/libcrypto_3.so, <prefix>/lib/libssl_3.so, <prefix>/license/LICENSE.txt; CMakeLists.txt gets
# the folder as -DQB_ANDROID_OPENSSL=<prefix> and puts the two libraries into the APK.
# The source archive must have the SHA-256 below (OpenSSL's own openssl-3.5.9.tar.gz.sha256 of its GitHub release,
# read on 2026-11-14); QB_OPENSSL_SHA256 names another one. 3.5 is OpenSSL's long-term branch (until 2030-04-08).
# The names: Android itself has a libcrypto.so and a libssl.so of its own that an app must not meet, so the
# libraries carry "_3" (as Qt's documentation and KDAB's android_openssl name them): no version after ".so".
# OpenSSL's own way to such names is a build target with "shlib_variant" (Configurations/README.md): two targets
# that take everything from OpenSSL's Android targets and add the variant are written into the source tree here.
# (Run 21 of the Android workflow tried make variables instead: the libraries came out as libcrypto.so, libssl.so.)
set -euo pipefail
NDK=${1:?ndk folder}
ABI=${2:?abi}
PREFIX=${3:?prefix}
VERSION=3.5.9
WANT=${QB_OPENSSL_SHA256:-603f5602e2eef00d77fbd429d34dcd5822bb301757a1bc9cdb24c670f1eb859a}
API=28 # Android 9, the program's minimum (CMakeLists.txt, QT_ANDROID_MIN_SDK_VERSION)

case "$ABI" in
  arm64-v8a) TARGET=qbandroid-arm64 ;;
  x86_64)    TARGET=qbandroid-x86_64 ;;
  *) echo "unknown ABI: $ABI" >&2; exit 2 ;;
esac
BIN="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
[ -x "$BIN/clang" ] || { echo "no compiler: $BIN/clang" >&2; exit 2; }

if [ -f "$PREFIX/lib/libssl_3.so" ] && [ -f "$PREFIX/lib/libcrypto_3.so" ] && [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$VERSION $ABI" ]; then
  echo "OpenSSL $VERSION for $ABI is there: $PREFIX"
  exit 0
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
FILE="$WORK/openssl-$VERSION.tar.gz"
curl -fsSL --retry 5 -o "$FILE" "https://github.com/openssl/openssl/releases/download/openssl-$VERSION/openssl-$VERSION.tar.gz"
SUM=$(sha256sum "$FILE" | cut -d' ' -f1)
echo "source archive: openssl-$VERSION.tar.gz sha256=$SUM"
if [ "$SUM" != "$WANT" ]; then
  echo "the archive's SHA-256 is not the expected $WANT" >&2
  exit 3
fi
tar -C "$WORK" -xzf "$FILE"
SRC="$WORK/openssl-$VERSION"
[ -f "$SRC/Configure" ] || { echo "unexpected archive layout" >&2; exit 3; }

cd "$SRC"
# (OpenSSL's Android rules take the processor from what follows the last "-" of the target's name)
cat > Configurations/99-quillbinder.conf <<'CONF'
my %targets = (
    "qbandroid-arm64" => { inherit_from => [ "android-arm64" ], shlib_variant => "_3" },
    "qbandroid-x86_64" => { inherit_from => [ "android-x86_64" ], shlib_variant => "_3" },
);
CONF
export ANDROID_NDK_ROOT="$NDK"
export PATH="$BIN:$PATH"
# shared libraries only; no programs, tests or documentation. 16 KB memory pages (Android 15 and later may use
# them): the linker's option, as for the program's own libraries.
./Configure "$TARGET" -D__ANDROID_API__=$API shared no-tests no-docs no-apps no-ssl3 no-comp \
  -Wl,-z,max-page-size=16384 > "$WORK/configure.log" 2>&1 || { tail -30 "$WORK/configure.log" >&2; exit 4; }
make -j"$(nproc)" build_libs > "$WORK/make.log" 2>&1 || { tail -40 "$WORK/make.log" >&2; exit 4; }
[ -f libcrypto_3.so ] && [ -f libssl_3.so ] || { echo "the libraries were not built under their names:" >&2; ls lib* >&2; exit 4; }

rm -rf "$PREFIX"
mkdir -p "$PREFIX/lib" "$PREFIX/license"
cp libcrypto_3.so libssl_3.so "$PREFIX/lib/"
"$BIN/llvm-strip" --strip-unneeded "$PREFIX/lib/libcrypto_3.so" "$PREFIX/lib/libssl_3.so"
cp LICENSE.txt "$PREFIX/license/"
echo "$SUM" > "$PREFIX/source.sha256"
echo "$VERSION $ABI" > "$PREFIX/version.txt"
echo "OpenSSL $VERSION for $ABI built: libcrypto_3.so $(stat -c %s "$PREFIX/lib/libcrypto_3.so") bytes, libssl_3.so $(stat -c %s "$PREFIX/lib/libssl_3.so") bytes"
"$BIN/llvm-readelf" -d "$PREFIX/lib/libssl_3.so" | grep -E 'SONAME|NEEDED' | sed 's/^/  /'
