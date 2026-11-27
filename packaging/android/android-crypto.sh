#!/usr/bin/env bash
# The encryption libraries for Android (0.99.0 probe): RNP (OpenPGP) with Botan 3 as its crypto backend, json-c, bzip2
# and zlib as STATIC libraries for one ABI, built with the NDK's clang. The same recipe as tools/get-crypto.sh for
# Linux (it reads that script's pins: tags, commits and Botan's module list, so that the two cannot drift apart).
#
#   packaging/android/android-crypto.sh <ndk folder> <abi: arm64-v8a | x86_64> <prefix>
#
# Result: <prefix>/lib/{librnp,libsexpp,libbotan-3,libjson-c,libbz2,libz}.a, <prefix>/include, <prefix>/licenses,
# <prefix>/version.txt. At the end a smoke link: the archives are linked into a shared object with --no-undefined
# (what the program's own library will do), so a missing symbol shows here and not only in the APK build.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
NDK=${1:?ndk folder}
ABI=${2:?abi}
PREFIX=${3:?prefix}
API=28 # Android 9, the program's minimum
JOBS=${QB_JOBS:-$(nproc)}
case "$ABI" in
  arm64-v8a) TRIPLE=aarch64-linux-android; BCPU=arm64 ;;
  x86_64)    TRIPLE=x86_64-linux-android;  BCPU=x86_64 ;;
  *) echo "unknown ABI: $ABI" >&2; exit 2 ;;
esac
BIN="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
[ -x "$BIN/clang" ] || { echo "no compiler: $BIN/clang" >&2; exit 2; }
CC="$BIN/$TRIPLE$API-clang"
CXX="$BIN/$TRIPLE$API-clang++"
AR="$BIN/llvm-ar"
RANLIB="$BIN/llvm-ranlib"

# the pins of the Linux recipe
eval "$(grep -E '^(BOTAN|JSONC|BZIP2|ZLIB|RNP)_(TAG|COMMIT)=|^SEXPP_COMMIT=|^BOTAN_MODULES=' "$ROOT/tools/get-crypto.sh")"
STAMP="botan $BOTAN_TAG $BOTAN_COMMIT; json-c $JSONC_TAG $JSONC_COMMIT; bzip2 $BZIP2_TAG $BZIP2_COMMIT; zlib $ZLIB_TAG $ZLIB_COMMIT; rnp $RNP_TAG $RNP_COMMIT; sexpp $SEXPP_COMMIT; ndk $ABI"
if [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$STAMP" ] && [ -f "$PREFIX/lib/librnp.a" ] && [ -f "$PREFIX/lib/libbotan-3.a" ]; then
  echo "the encryption libraries for $ABI are there: $PREFIX"
  exit 0
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
clone() {  # <dir> <url> <tag> <commit>
  git clone -q --depth 1 --branch "$3" "$2" "$WORK/$1"
  test "$(git -C "$WORK/$1" rev-parse 'HEAD^{commit}')" = "$4" || { echo "android-crypto: the tag $3 of $2 is not the commit $4" >&2; exit 1; }
}
clone botan https://github.com/randombit/botan "$BOTAN_TAG" "$BOTAN_COMMIT"
clone json-c https://github.com/json-c/json-c "$JSONC_TAG" "$JSONC_COMMIT"
clone bzip2 https://github.com/libarchive/bzip2 "$BZIP2_TAG" "$BZIP2_COMMIT"
clone zlib https://github.com/madler/zlib "$ZLIB_TAG" "$ZLIB_COMMIT"
clone rnp https://github.com/rnpgp/rnp "$RNP_TAG" "$RNP_COMMIT"
git init -q "$WORK/sexpp"
git -C "$WORK/sexpp" fetch -q --depth 1 https://github.com/rnpgp/sexpp.git "$SEXPP_COMMIT"
git -C "$WORK/sexpp" checkout -q FETCH_HEAD
test "$(git -C "$WORK/sexpp" rev-parse HEAD)" = "$SEXPP_COMMIT"
test "$(git -C "$WORK/rnp" ls-tree HEAD src/libsexpp | awk '{print $3}')" = "$SEXPP_COMMIT"
rm -rf "$WORK/rnp/src/libsexpp"
cp -a "$WORK/sexpp" "$WORK/rnp/src/libsexpp"
rm -rf "$WORK/rnp/src/libsexpp/.git"

rm -rf "$PREFIX"
mkdir -p "$PREFIX/lib" "$PREFIX/include" "$PREFIX/licenses"
step() { echo "=== $1 ($(date +%H:%M:%S))"; }

step "botan"
(cd "$WORK/botan" && python3 ./configure.py --prefix="$PREFIX" --build-targets=static --disable-shared-library \
    --minimized-build --enable-modules="$(echo $BOTAN_MODULES | tr ' ' ',')" --without-documentation \
    --os=android --cpu=$BCPU --cc=clang --cc-bin="$CXX" --ar-command="$AR" --extra-cxxflags=-fPIC > "$WORK/botan-configure.log" 2>&1 \
    || { tail -30 "$WORK/botan-configure.log" >&2; exit 4; }
 make -j"$JOBS" libs
 make install > /dev/null)

TC=(-DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" -DANDROID_ABI="$ABI" -DANDROID_PLATFORM=android-$API
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_FIND_ROOT_PATH="$PREFIX"
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON)

step "json-c"
cmake -S "$WORK/json-c" -B "$WORK/b-json-c" -G Ninja "${TC[@]}" -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_TESTING=OFF \
    -DBUILD_APPS=OFF -DDISABLE_EXTRA_LIBS=ON > /dev/null
cmake --build "$WORK/b-json-c" -j"$JOBS"
cmake --install "$WORK/b-json-c" > /dev/null

step "bzip2"
(cd "$WORK/bzip2" && make -j"$JOBS" libbz2.a CC="$CC" AR="$AR" RANLIB="$RANLIB" CFLAGS="-O2 -fPIC -D_FILE_OFFSET_BITS=64 -Wall"
 cp libbz2.a "$PREFIX/lib/" && cp bzlib.h "$PREFIX/include/")

step "zlib"
(cd "$WORK/zlib" && CC="$CC" AR="$AR" RANLIB="$RANLIB" CFLAGS="-O2 -fPIC" ./configure --static --prefix="$PREFIX" > /dev/null
 make -j"$JOBS" libz.a
 make install > /dev/null)

step "rnp"
cmake -S "$WORK/rnp" -B "$WORK/b-rnp" -G Ninja "${TC[@]}" -DCMAKE_PREFIX_PATH="$PREFIX" -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_CXX_FLAGS="-include cstring" -DCRYPTO_BACKEND=botan3 -DBOTAN_ROOT_DIR="$PREFIX" -DENABLE_CRYPTO_REFRESH=OFF \
    -DENABLE_PQC=OFF -DBUILD_TESTING=OFF -DENABLE_DOC=OFF -DDOWNLOAD_GTEST=OFF -DENABLE_COVERAGE=OFF -DENABLE_SANITIZERS=OFF \
    -DENABLE_FUZZERS=OFF -DBZIP2_INCLUDE_DIR="$PREFIX/include" -DBZIP2_LIBRARY_RELEASE="$PREFIX/lib/libbz2.a" \
    -DZLIB_INCLUDE_DIR="$PREFIX/include" -DZLIB_LIBRARY="$PREFIX/lib/libz.a" \
    -DJSON-C_INCLUDE_DIR="$PREFIX/include" -DJSON-C_LIBRARY="$PREFIX/lib/libjson-c.a" > "$WORK/rnp-configure.log" 2>&1 \
    || { tail -40 "$WORK/rnp-configure.log" >&2; exit 4; }
cmake --build "$WORK/b-rnp" --target librnp -j"$JOBS"
cmake --install "$WORK/b-rnp" --component development > /dev/null

step "smoke link"
cat > "$WORK/smoke.c" <<'C'
#include <rnp/rnp.h>
const char *qb_smoke(void) { return rnp_version_string(); }
C
"$CC" -fPIC -I"$PREFIX/include" -c "$WORK/smoke.c" -o "$WORK/smoke.o"
"$CXX" -shared -o "$WORK/libsmoke.so" "$WORK/smoke.o" -Wl,--whole-archive "$PREFIX/lib/librnp.a" -Wl,--no-whole-archive \
    "$PREFIX/lib/libsexpp.a" "$PREFIX/lib/libbotan-3.a" "$PREFIX/lib/libjson-c.a" "$PREFIX/lib/libbz2.a" "$PREFIX/lib/libz.a" \
    -Wl,--no-undefined -Wl,-z,max-page-size=16384
echo "smoke link ok: $(stat -c %s "$WORK/libsmoke.so") bytes"

cp "$WORK/botan/license.txt" "$PREFIX/licenses/LICENSE-botan.txt"
cp "$WORK/json-c/COPYING" "$PREFIX/licenses/LICENSE-json-c.txt"
cp "$WORK/bzip2/LICENSE" "$PREFIX/licenses/LICENSE-bzip2.txt"
cp "$WORK/rnp/LICENSE.md" "$PREFIX/licenses/LICENSE-rnp.txt"
cp "$WORK/rnp/LICENSE-OCB.md" "$PREFIX/licenses/LICENSE-rnp-OCB.txt"
cp "$WORK/sexpp/LICENSE.md" "$PREFIX/licenses/LICENSE-sexpp.txt"
cp "$WORK/zlib/LICENSE" "$PREFIX/licenses/LICENSE-zlib.txt"
echo "$STAMP" > "$PREFIX/version.txt"
echo "the encryption libraries for $ABI are built: $PREFIX"
ls -l "$PREFIX/lib"
