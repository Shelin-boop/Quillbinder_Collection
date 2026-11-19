#!/usr/bin/env bash
# Hunspell for Android (port step A0, 2026-11-14): the spelling library as a static library for one Android ABI,
# built from Hunspell's source with the NDK's compiler. The same version as the Windows program carries (vcpkg
# builds it there). Only the library's own files are compiled (src/hunspell); they need no configure run: the two
# headers configure would write are made here.
#
#   packaging/android/android-hunspell.sh <ndk folder> <abi: arm64-v8a | x86_64> <prefix>
#
# Result: <prefix>/lib/libhunspell-1.7.a and <prefix>/include/hunspell/*.h(xx); CMakeLists.txt gets them as
# -DQB_HUNSPELL_INCLUDE=<prefix>/include -DQB_HUNSPELL_LIB=<prefix>/lib/libhunspell-1.7.a.
# The source archive must have the SHA-256 below (the archive GitHub made for the tag on 2026-11-14, run 2 of the
# Android workflow); QB_HUNSPELL_SHA256 names another one. The script prints the one it got.
set -euo pipefail
NDK=${1:?ndk folder}
ABI=${2:?abi}
PREFIX=${3:?prefix}
VERSION=1.7.5
WANT=${QB_HUNSPELL_SHA256:-833ee0cb129f7b933def265ea907f9ba4d0cfc5b52d838678eafceeec1adaa53}
API=28 # Android 9, the program's minimum (CMakeLists.txt, QT_ANDROID_MIN_SDK_VERSION)

case "$ABI" in
  arm64-v8a) TRIPLE=aarch64-linux-android ;;
  x86_64)    TRIPLE=x86_64-linux-android ;;
  *) echo "unknown ABI: $ABI" >&2; exit 2 ;;
esac
BIN="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
CXX="$BIN/clang++"
AR="$BIN/llvm-ar"
[ -x "$CXX" ] || { echo "no compiler: $CXX" >&2; exit 2; }

if [ -f "$PREFIX/lib/libhunspell-1.7.a" ] && [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$VERSION $ABI" ]; then
  echo "Hunspell $VERSION for $ABI is there: $PREFIX"
  exit 0
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
FILE="$WORK/hunspell-$VERSION.tar.gz"
curl -fsSL --retry 5 -o "$FILE" "https://github.com/hunspell/hunspell/archive/refs/tags/v$VERSION.tar.gz"
SUM=$(sha256sum "$FILE" | cut -d' ' -f1)
echo "source archive: hunspell-$VERSION.tar.gz sha256=$SUM"
if [ "$SUM" != "$WANT" ]; then
  echo "the archive's SHA-256 is not the expected $WANT" >&2
  exit 3
fi
tar -C "$WORK" -xzf "$FILE"
SRC="$WORK/hunspell-$VERSION/src/hunspell"
[ -f "$SRC/hunspell.cxx" ] || { echo "unexpected archive layout" >&2; exit 3; }

# the two headers configure writes
sed -e 's/@HAVE_VISIBILITY@/1/' "$SRC/hunvisapi.h.in" > "$SRC/hunvisapi.h"
if [ -f "$SRC/hunversion.h.in" ]; then
  IFS=. read -r MAJ MIN PAT <<< "$VERSION"
  sed -e "s/@HUNSPELL_VERSION_MAJOR@/$MAJ/; s/@HUNSPELL_VERSION_MINOR@/$MIN/; s/@HUNSPELL_VERSION_PATCH@/$PAT/; s/@VERSION@/$VERSION/" \
    "$SRC/hunversion.h.in" > "$SRC/hunversion.h"
fi

OBJ="$WORK/obj"
mkdir -p "$OBJ"
for f in "$SRC"/*.cxx; do
  "$CXX" --target="$TRIPLE$API" -std=c++17 -O2 -fPIC -DNDEBUG -DHUNSPELL_STATIC -DBUILDING_LIBHUNSPELL \
    -I"$SRC" -c "$f" -o "$OBJ/$(basename "${f%.cxx}").o"
done

rm -rf "$PREFIX"
mkdir -p "$PREFIX/lib" "$PREFIX/include/hunspell" "$PREFIX/license"
"$AR" rcs "$PREFIX/lib/libhunspell-1.7.a" "$OBJ"/*.o
for h in hunspell.hxx hunspell.h hunvisapi.h hunversion.h w_char.hxx atypes.hxx; do
  [ -f "$SRC/$h" ] && cp "$SRC/$h" "$PREFIX/include/hunspell/"
done
# the license texts, for the APK's license list (a later step)
for l in COPYING COPYING.LESSER COPYING.MPL license.hunspell license.myspell; do
  [ -f "$WORK/hunspell-$VERSION/$l" ] && cp "$WORK/hunspell-$VERSION/$l" "$PREFIX/license/"
done
echo "$SUM" > "$PREFIX/source.sha256"
echo "$VERSION $ABI" > "$PREFIX/version.txt"
echo "Hunspell $VERSION for $ABI built: $(ls -l "$PREFIX/lib/libhunspell-1.7.a" | awk '{print $5}') bytes, $(ls "$OBJ" | wc -l) objects"
