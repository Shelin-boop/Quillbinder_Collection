#!/usr/bin/env bash
# FFmpeg for the Windows add-on "Extra Video and Audio Formats" (Windows 0.93.0). Until 0.93.0 the add-on carried the
# FFmpeg DLLs The Qt Company builds for Qt Multimedia; Qt 6.12.1 for Windows still brings FFmpeg 9.0.2, which the High
# findings of the security issue #7 name ("FFmpeg through 9.0.2"). So the project builds FFmpeg itself: the release
# Linux uses (tools/get-ffmpeg.sh: 9.1, the same archive and SHA-256), with the configuration Qt's own build reports
# (its avcodec_configuration(), seen in the add-on of 2026-11-24), shared DLLs with MSVC. FFmpeg 9.1 keeps the major
# versions of 9.0 (avcodec-63, avformat-63, avutil-61, swresample-7, swscale-10), so Qt's FFmpeg backend
# (ffmpegmediaplugin.dll) loads these DLLs as it loaded Qt's own (packaging/windows/win-addons.sh checks the names).
#
#   packaging/windows/win-ffmpeg.sh <prefix>
#
# Runs in Git Bash inside MSVC's environment (packaging/windows/win-ci.sh); configure and make run in the runner's
# MSYS2 (C:\msys64, with make, nasm and diffutils from its packages), which inherits that environment. zlib: the one
# vcpkg built for the program (1.4, static; packaging/windows/win-vcpkg.sh), in place of Qt's zlib 1.3.1.
# Result: <prefix>/bin (the five DLLs), <prefix>/include, <prefix>/lib, <prefix>/version.txt, <prefix>/configure.txt.
# A finished build of the same version, script and zlib is reused (D1: packaging/windows/win-deps-pack.sh keeps it as
# a pack). Without a pack it takes about 20 to 30 minutes on GitHub's runner.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
PREFIX=$1
VERSION=9.1
SHA256=1a4d394ae63d190a526cd052eb4415c156b5c5e2df37efab41a7de0b8fde2433   # as tools/get-ffmpeg.sh (its signature was checked)
URL=https://ffmpeg.org/releases/ffmpeg-$VERSION.tar.xz
# Qt's configuration for Qt 6.12.1 for Windows, without its own paths (--prefix, zlib's folders): no programs,
# devices or filters, no Vulkan, LGPL (no --enable-gpl, --enable-nonfree or --enable-version3)
OPTS="--disable-programs --disable-doc --disable-debug --enable-network --disable-lzma --enable-pic --disable-vulkan --disable-v4l2-m2m --disable-decoder=truemotion1 --disable-avdevice --disable-avfilter --enable-zlib --toolchain=msvc --enable-shared --disable-static"
TRIPLET=${VCPKG_TRIPLET:-x64-windows-static-md}
ZI="$(cygpath -u "${VCPKG_INSTALLATION_ROOT:?vcpkg is not on this machine}")/installed/$TRIPLET"
# (zlib 1.4's own CMake build names its static library libzs.lib; older ports made zlib.lib)
ZLIB=""
for n in libzs.lib zs.lib zlib.lib zlibstatic.lib; do [ -f "$ZI/lib/$n" ] && { ZLIB="$ZI/lib/$n"; break; }; done
[ -f "$ZI/include/zlib.h" ] && [ -n "$ZLIB" ] || { echo "win-ffmpeg.sh: no zlib in $ZI (the vcpkg step first): $(ls "$ZI/lib" 2>/dev/null | tr '\n' ' ')" >&2; exit 1; }
ZV=$(sed -n 's/^#define ZLIB_VERSION "\(.*\)".*/\1/p' "$ZI/include/zlib.h" | head -1)
STAMP="$VERSION $SHA256 $(sha256sum "$0" | cut -c1-16) zlib $ZV"
LIBS="avcodec avformat avutil swresample swscale"
have_all() { for l in $LIBS; do ls "$PREFIX/bin/" 2>/dev/null | grep -qiE "^$l-[0-9]+\.dll$" || return 1; done; }
if [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$STAMP" ] && have_all; then
    echo "FFmpeg is there: $PREFIX ($VERSION)"
    exit 0
fi
MSYS=/c/msys64/usr/bin/bash.exe
[ -x "$MSYS" ] || { echo "win-ffmpeg.sh: no MSYS2 at C:\\msys64" >&2; exit 1; }
WORK="$PWD/.ffmpeg-work"
rm -rf "$WORK" "$PREFIX"
mkdir -p "$WORK/zlib/include" "$WORK/zlib/lib"
cp "$ZI/include/zlib.h" "$ZI/include/zconf.h" "$WORK/zlib/include/"
cp "$ZLIB" "$WORK/zlib/lib/z.lib"   # FFmpeg's configure asks for -lz, which MSVC's link reads as z.lib
curl -fsSL --retry 5 --retry-delay 10 -o "$WORK/ffmpeg.tar.xz" "$URL"
echo "$SHA256  $WORK/ffmpeg.tar.xz" | sha256sum -c --quiet
cat > "$WORK/build.sh" <<'SH'
set -euo pipefail
work=$(cygpath -u "$1"); prefix=$(cygpath -u "$2"); z=$3; opts=$4   # (zlib: a C:/... path, which cl and link read as it is)
pacman -Sy --noconfirm --needed make nasm diffutils > "$work/pacman.log" 2>&1 || { tail -20 "$work/pacman.log"; exit 1; }
# MSVC's cl and link before MSYS2's own link (coreutils)
cl=$(command -v cl.exe) || { echo "cl.exe is not on the PATH (MSVC's environment)"; exit 1; }
export PATH="$(dirname "$cl"):$PATH"
echo "compiler: $cl; linker: $(command -v link.exe); nasm: $(nasm -v | head -1)"
cd "$work"
tar -xJf ffmpeg.tar.xz
cd ffmpeg-*/
# shellcheck disable=SC2086
./configure --prefix="$prefix" $opts --extra-cflags="-I$z/include" --extra-ldflags="-LIBPATH:$z/lib" > "$work/configure.log" 2>&1 \
    || { tail -40 ffbuild/config.log; exit 1; }
grep -E '^(License|External libraries|Enabled)' -A 2 "$work/configure.log" | head -20 || true
make -j"$(nproc)" > "$work/make.log" 2>&1 || { grep -E 'error|Error' "$work/make.log" | head -40; tail -20 "$work/make.log"; exit 1; }
make install > "$work/install.log" 2>&1 || { tail -20 "$work/install.log"; exit 1; }
SH
MSYS2_PATH_TYPE=inherit CHERE_INVOKING=1 MSYSTEM=MSYS "$MSYS" -l "$(cygpath -m "$WORK/build.sh")" \
    "$(cygpath -m "$WORK")" "$(cygpath -m "$PREFIX")" "$(cygpath -m "$WORK/zlib")" "$OPTS"
have_all || { echo "win-ffmpeg.sh: not every DLL was built: $(ls "$PREFIX/bin" 2>/dev/null | tr '\n' ' ')" >&2; exit 1; }
echo "$OPTS (zlib $ZV of vcpkg)" > "$PREFIX/configure.txt"
echo "$STAMP" > "$PREFIX/version.txt"
rm -rf "$WORK"
echo "FFmpeg built: $PREFIX ($VERSION, zlib $ZV)"
ls "$PREFIX/bin"
