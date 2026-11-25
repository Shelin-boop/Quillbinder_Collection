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
# Runs in Git Bash (packaging/windows/win-ci.sh); configure and make run in the runner's MSYS2 (C:\msys64, with make,
# nasm and diffutils from its packages). MSYS2 does not inherit MSVC's environment: this script runs MSVC's own
# vcvars64.bat in cmd (VSDIR, from the workflow's step "Find Visual Studio"), takes MSVC's folders, INCLUDE and LIB
# from what it prints, and hands them to the MSYS2 script in a file (msvc-env.sh in the work folder). zlib: the one
# vcpkg built for the program (1.4, static; packaging/windows/win-vcpkg.sh), in place of Qt's zlib 1.3.1.
# Result: <prefix>/bin (the five DLLs), <prefix>/include, <prefix>/lib, <prefix>/version.txt, <prefix>/configure.txt.
# A finished build of the same version, script and zlib is reused (D1: packaging/windows/win-deps-pack.sh keeps it as
# a pack). Without a pack it takes about 10 minutes on GitHub's runner.
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
# MSVC's folders (compiler, linker, Windows SDK tools) before MSYS2's own tools, and MSVC's INCLUDE and LIB: MSYS2's login
# profile does not keep the PATH it was started with (run 45365674495 of win/0.93.0: "cl.exe is not on the PATH"), INCLUDE
# and LIB did not reach it (run 45370498738: "windows.h: no include path set"), and as arguments the second list was lost
# (run 45380641653): all three come in a file the script reads
. "$work/msvc-env.sh"
export PATH="$(cygpath -u -p "$QB_MSVC_PATH"):$PATH"
export INCLUDE="$QB_MSVC_INCLUDE" LIB="$QB_MSVC_LIB"
[ -n "$INCLUDE" ] && [ -n "$LIB" ] || { echo "MSVC's INCLUDE or LIB is empty"; exit 1; }
echo "INCLUDE: $(echo "$INCLUDE" | tr ';' '\n' | wc -l) folders, LIB: $(echo "$LIB" | tr ';' '\n' | wc -l) folders"
pacman -Sy --noconfirm --needed make nasm diffutils tar xz > "$work/pacman.log" 2>&1 || { tail -20 "$work/pacman.log"; exit 1; }
cl=$(command -v cl.exe) || { echo "cl.exe is not on the PATH (MSVC's environment): $PATH"; exit 1; }
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
# MSVC's environment for the MSYS2 shell: its folders on the PATH, INCLUDE and LIB, in Windows' form. Taken from
# MSVC's own vcvars64.bat run in cmd (VSDIR, from the workflow's step "Find Visual Studio"), which prints the
# environment it makes; Git Bash's own copy is the second choice, its names looked up without regard to case (run
# 45385585996: Git Bash's $LIB was empty although the program's build in the same step links)
winlist() { tr -d '\r' | tr '\\' '/'; }
VCENV=""
if [ -n "${VSDIR:-}" ] && [ -f "$(cygpath -u "$VSDIR")/VC/Auxiliary/Build/vcvars64.bat" ]; then
    printf '@call "%s\\VC\\Auxiliary\\Build\\vcvars64.bat" > nul 2>&1\r\n@set\r\n' "$(cygpath -w "$VSDIR")" > "$WORK/vcenv.bat"
    cmd.exe //d //c "$(cygpath -w "$WORK/vcenv.bat")" > "$WORK/vcenv.txt" 2> /dev/null || true
    VCENV="$WORK/vcenv.txt"
fi
fromvc() { [ -n "$VCENV" ] && grep -ai "^$1=" "$VCENV" | head -1 | cut -d= -f2- | winlist; }
fromsh() { env | grep -ai "^$1=" | head -1 | cut -d= -f2- | winlist; }
INC=$(fromvc INCLUDE); [ -n "$INC" ] || INC=$(fromsh INCLUDE)
LIBV=$(fromvc LIB); [ -n "$LIBV" ] || LIBV=$(fromsh LIB)
VCPATH=$(fromvc PATH)
MSVCPATH=""
if [ -n "$VCPATH" ]; then
    while IFS= read -r d; do
        case "$d" in *"Microsoft Visual Studio"*|*"Windows Kits"*) MSVCPATH="${MSVCPATH:+$MSVCPATH;}$d" ;; esac
    done < <(printf '%s\n' "$VCPATH" | tr ';' '\n')
else
    cl=$(command -v cl.exe) || { echo "win-ffmpeg.sh: cl.exe is not on the PATH (MSVC's environment)" >&2; exit 1; }
    MSVCPATH=$(cygpath -m "$(dirname "$cl")")
    while IFS= read -r d; do
        case "$d" in *"Microsoft Visual Studio"*|*"Windows Kits"*) MSVCPATH="$MSVCPATH;$(cygpath -m "$d")" ;; esac
    done < <(printf '%s\n' "$PATH" | tr ':' '\n')
fi
if [ -z "$INC" ] || [ -z "$LIBV" ] || [ -z "$MSVCPATH" ]; then
    echo "win-ffmpeg.sh: MSVC's environment not found (INCLUDE ${#INC}, LIB ${#LIBV}, folders ${#MSVCPATH} characters; vcvars: ${VCENV:-not run}, $(grep -ciE '^(include|lib|path)=' "${VCENV:-/dev/null}" 2>/dev/null) of its lines; Git Bash's names: $(env | cut -d= -f1 | grep -iE '^(include|lib|libpath|vsdir)$' | tr '\n' ' '))" >&2
    exit 1
fi
echo "MSVC's environment: INCLUDE $(echo "$INC" | tr ';' '\n' | grep -c .) folders, LIB $(echo "$LIBV" | tr ';' '\n' | grep -c .) folders, $(echo "$MSVCPATH" | tr ';' '\n' | grep -c .) folders for the PATH ($([ -n "$VCPATH" ] && echo vcvars || echo "Git Bash"))"
printf 'QB_MSVC_PATH=%q\nQB_MSVC_INCLUDE=%q\nQB_MSVC_LIB=%q\n' "$MSVCPATH" "$INC" "$LIBV" > "$WORK/msvc-env.sh"
MSYS2_PATH_TYPE=inherit CHERE_INVOKING=1 MSYSTEM=MSYS "$MSYS" -l "$(cygpath -m "$WORK/build.sh")" \
    "$(cygpath -m "$WORK")" "$(cygpath -m "$PREFIX")" "$(cygpath -m "$WORK/zlib")" "$OPTS"
have_all || { echo "win-ffmpeg.sh: not every DLL was built: $(ls "$PREFIX/bin" 2>/dev/null | tr '\n' ' ')" >&2; exit 1; }
echo "$OPTS (zlib $ZV of vcpkg)" > "$PREFIX/configure.txt"
echo "$STAMP" > "$PREFIX/version.txt"
rm -rf "$WORK"
echo "FFmpeg built: $PREFIX ($VERSION, zlib $ZV)"
ls "$PREFIX/bin"
