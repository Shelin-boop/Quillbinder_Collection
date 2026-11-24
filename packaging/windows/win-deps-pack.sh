#!/usr/bin/env bash
# Ready-made dependency packs for the Windows build (Windows 0.91.3, 2026-11-15; working rules section 10, decision D1):
# whisper.cpp built with MSVC (packaging/windows/win-whisper.sh) and Hunspell with zlib from vcpkg, each built once per
# recipe and kept as a .tar.xz on a pre-release "deps-<name>-windows-<first 16 characters of the hash>" of this
# repository, titled "not for downloading" (as tools/deps-pack.sh does for Linux). Runs in Git Bash on GitHub's
# Windows machine (.github/workflows/windows.yml, packaging/windows/win-ci.sh).
# Windows 0.92.0 (2026-11-20): the repository is public, so a pack's pre-release says what is inside, under which
# license, and where its source is (about, below); the license texts are in the pack.
#
#   packaging/windows/win-deps-pack.sh recipe <name>         the recipe (lines of text)
#   packaging/windows/win-deps-pack.sh hash|tag|file <name>  its SHA-256, the pre-release's tag, the pack's file name
#   packaging/windows/win-deps-pack.sh get <name> <dir>      unpack the pinned pack of this recipe into <dir>
#                                                            (exit 1 when none is pinned: the caller builds as before)
#   packaging/windows/win-deps-pack.sh publish <name> <dir>  pack <dir>, put it on its pre-release (if not there yet)
#                                                            and print the line "pin: ..." for QB_WIN_PACKS
#
# The pins: lines "<name> <hash> <file> <SHA-256 of the file>" in the variable QB_WIN_PACKS (set in
# windows.yml). A recipe without a pin is built as before; its pack is then published, and its pin line printed.
#
# A recipe is what decides the pack's content:
#   whisper   win-whisper.sh (it pins whisper.cpp's tag and commit) and the compiler's version (MSVC major.minor)
#   hunspell  the vcpkg ports of hunspell and zlib as the machine's vcpkg has them (their folders' files), the triplet,
#             the compiler's version and this script (which holds the install command)
# The tree of hunspell is vcpkg's installed/<triplet> folder; CMake finds it through vcpkg's toolchain file (no
# manifest: classic mode, which only reads that folder).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
TRIPLET=${VCPKG_TRIPLET:-x64-windows-static-md}
REPO=Shelin-boop/Quillbinder_Collection

msvc() {
    # major.minor of the compiler: binaries of one minor version stay usable for that version (inside MSVC's
    # environment VCToolsVersion is set; before it, Visual Studio's default file, VSDIR from the workflow)
    local v=${VCToolsVersion:-}
    if [ -z "$v" ] && [ -n "${VSDIR:-}" ]; then
        v=$(tr -d '\r\n ' < "$(cygpath -u "$VSDIR")/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt" 2>/dev/null || true)
    fi
    [ -n "$v" ] || { echo "win-deps-pack.sh: the compiler's version is not known (VCToolsVersion, VSDIR)" >&2; exit 2; }
    echo "$v" | cut -d. -f1,2
}

files_sum() { # <dir>: one SHA-256 over every file's path and content (sorted, line ends as they are)
    ( cd "$1" && find . -type f | LC_ALL=C sort | while IFS= read -r f; do printf '%s %s\n' "$(sha256sum "$f" | cut -d' ' -f1)" "$f"; done ) \
        | sha256sum | cut -d' ' -f1
}

recipe() {
    case "$1" in
    whisper)
        echo "pack: whisper (Windows x64)"
        echo "script: packaging/windows/win-whisper.sh $(sha256sum "$ROOT/packaging/windows/win-whisper.sh" | cut -d' ' -f1)"
        echo "msvc: $(msvc)"
        ;;
    hunspell)
        local vr
        vr=$(cygpath -u "${VCPKG_INSTALLATION_ROOT:?vcpkg is not on this machine}")
        echo "pack: hunspell and zlib (vcpkg, Windows x64)"
        for p in hunspell zlib; do
            [ -d "$vr/ports/$p" ] || { echo "win-deps-pack.sh: no vcpkg port $p" >&2; exit 2; }
            echo "port: $p $(sed -n 's/.*"version[a-z-]*": *"\([^"]*\)".*/\1/p' "$vr/ports/$p/vcpkg.json" | head -1)#$(sed -n 's/.*"port-version": *\([0-9]*\).*/\1/p' "$vr/ports/$p/vcpkg.json" | head -1) $(files_sum "$vr/ports/$p")"
        done
        local tf="" c
        for c in "$vr/triplets/$TRIPLET.cmake" "$vr/triplets/community/$TRIPLET.cmake"; do
            [ -f "$c" ] && { tf=$c; break; }
        done
        [ -n "$tf" ] || { echo "win-deps-pack.sh: no vcpkg triplet $TRIPLET" >&2; exit 2; }
        echo "triplet: $TRIPLET $(sha256sum "$tf" | cut -d' ' -f1)"
        echo "script: packaging/windows/win-deps-pack.sh $(sha256sum "$ROOT/packaging/windows/win-deps-pack.sh" | cut -d' ' -f1)"
        echo "msvc: $(msvc)"
        ;;
    ffmpeg)
        # (Windows 0.93.0) the source, the script, the compiler and the zlib it links
        local zi
        zi="$(cygpath -u "${VCPKG_INSTALLATION_ROOT:?vcpkg is not on this machine}")/installed/$TRIPLET/include/zlib.h"
        echo "pack: ffmpeg (Windows x64)"
        echo "source: ffmpeg-$(sed -n 's/^VERSION=//p' "$ROOT/packaging/windows/win-ffmpeg.sh").tar.xz $(sed -n 's/^SHA256=\([0-9a-f]*\).*/\1/p' "$ROOT/packaging/windows/win-ffmpeg.sh")"
        echo "script: packaging/windows/win-ffmpeg.sh $(sha256sum "$ROOT/packaging/windows/win-ffmpeg.sh" | cut -d' ' -f1)"
        echo "zlib: $(sed -n 's/^#define ZLIB_VERSION "\(.*\)".*/\1/p' "$zi" 2>/dev/null | head -1)"
        echo "msvc: $(msvc)"
        ;;
    *) echo "win-deps-pack.sh: unknown pack $1 (whisper, hunspell, ffmpeg)" >&2; exit 2 ;;
    esac
}

# .tar.xz with GNU tar and xz when Git Bash has xz, otherwise with Python's tarfile (its lzma module is built in)
PY=$(command -v python3 || command -v python || true)
tar_create() { # <dir> <file>
    if command -v xz > /dev/null; then
        ( cd "$1" && tar --owner=0 --group=0 -cJf "$2" . )
    else
        "$PY" -c 'import sys, tarfile
with tarfile.open(sys.argv[2], "w:xz") as t:
    t.add(sys.argv[1], arcname=".")' "$(cygpath -w "$1")" "$(cygpath -w "$2")"
    fi
}
tar_extract() { # <file> <dir>
    if command -v xz > /dev/null; then
        tar -xJf "$1" -C "$2"
    else
        "$PY" -c 'import sys, tarfile
with tarfile.open(sys.argv[1], "r:xz") as t:
    t.extractall(sys.argv[2], **({"filter": "data"} if hasattr(tarfile, "data_filter") else {}))' "$(cygpath -w "$1")" "$(cygpath -w "$2")"
    fi
}

# What a pack holds, its license and its source (the text of its public pre-release). <name> <dir of the pack>
about() {
    case "$1" in
    whisper)
        echo "Inside: whisper.cpp with ggml, $(sed -n 's/^TAG=//p' "$ROOT/packaging/windows/win-whisper.sh") (commit $(sed -n 's/^COMMIT=//p' "$ROOT/packaging/windows/win-whisper.sh")), unchanged, built with Microsoft Visual C++ by packaging/windows/win-whisper.sh, and the test recording whisper.cpp ships (public domain)."
        echo "License: MIT; its text is in the pack (LICENSE-whisper.cpp.txt). Source: https://github.com/ggml-org/whisper.cpp at that tag."
        ;;
    hunspell)
        echo "Inside: Hunspell and zlib as static libraries, unchanged, built with Microsoft Visual C++ by vcpkg from its ports (their versions: the recipe below)."
        echo "Licenses: Hunspell under MPL-1.1, GPL-2.0-or-later or LGPL-2.1-or-later (its three licenses), zlib under the zlib license. Sources: https://github.com/hunspell/hunspell and https://github.com/madler/zlib at the ports' versions."
        local texts=""
        for c in share/hunspell/copyright share/zlib/copyright; do [ -f "$2/$c" ] && texts="$texts $c"; done
        if [ -n "$texts" ]; then echo "The license texts are in the pack:$texts."; else echo "The license texts: the sources above (COPYING* and README of Hunspell, zlib.h of zlib)."; fi
        ;;
    ffmpeg)
        echo "Inside: FFmpeg $(sed -n 's/^VERSION=//p' "$ROOT/packaging/windows/win-ffmpeg.sh") as DLLs (avcodec, avformat, avutil, swresample, swscale) with their headers and import libraries, unchanged, built with Microsoft Visual C++ by packaging/windows/win-ffmpeg.sh with the configuration of The Qt Company's FFmpeg for Qt Multimedia (configure.txt in the pack), zlib linked in statically."
        echo "Licenses: FFmpeg under the GNU LGPL 2.1 or later (no GPL or non-free parts); zlib under the zlib license. Sources: https://ffmpeg.org/releases/ (the archive named in the recipe below) and https://github.com/madler/zlib at the version below."
        ;;
    esac
}
hash_of() { recipe "$1" | tr -d '\r' | sha256sum | cut -d' ' -f1; }
tag_of() { echo "deps-$1-windows-$(hash_of "$1" | cut -c1-16)"; }
file_of() { echo "$1-windows-$(hash_of "$1" | cut -c1-16).tar.xz"; }

cmd=${1:-}
name=${2:-}
case "$cmd" in
recipe) recipe "$name" ;;
hash) hash_of "$name" ;;
tag) tag_of "$name" ;;
file) file_of "$name" ;;
get)
    dir=$3
    h=$(hash_of "$name")
    pin=$(printf '%s\n' "${QB_WIN_PACKS:-}" | tr -d '\r' | awk -v n="$name" -v h="$h" '$1 == n && $2 == h { print $3, $4; exit }')
    if [ -z "$pin" ]; then
        echo "no pack pinned for $name (recipe $(echo "$h" | cut -c1-16))"
        exit 1
    fi
    file=${pin% *}
    sum=${pin#* }
    tmp=$(mktemp -d)
    if ! gh release download "$(tag_of "$name")" --repo "$REPO" -p "$file" -D "$tmp" > /dev/null 2>&1; then
        echo "the pack $file could not be downloaded: building instead"
        rm -rf "$tmp"
        exit 1
    fi
    if ! echo "$sum  $tmp/$file" | sha256sum -c --quiet > /dev/null 2>&1; then
        echo "the pack $file does not match its pinned SHA-256: building instead"
        rm -rf "$tmp"
        exit 1
    fi
    mkdir -p "$dir"
    tar_extract "$tmp/$file" "$dir"
    rm -rf "$tmp"
    echo "from the pack $file (pre-release $(tag_of "$name"))"
    ;;
publish)
    dir=$3
    tag=$(tag_of "$name")
    file=$(file_of "$name")
    if gh api "repos/$REPO/releases/tags/$tag" --jq '.assets[].name' 2> /dev/null | grep -qxF "$file"; then
        echo "pack there already: $file"
        exit 0
    fi
    out=$(mktemp -d)
    recipe "$name" | tr -d '\r' > "$out/recipe-$name.txt"
    tar_create "$dir" "$out/$file"
    sum=$(sha256sum "$out/$file" | cut -d' ' -f1)
    echo "$sum  $file" > "$out/$file.sha256"
    pin="$name $(hash_of "$name") $file $sum"
    {
        echo "Built by .github/workflows/windows.yml (run ${GITHUB_RUN_ID:-local}, commit ${GITHUB_SHA:-local}) for packaging/windows/win-deps-pack.sh."
        echo "Not for downloading: working storage of the Windows build."
        echo
        about "$name" "$dir"
        echo
        echo '```'
        echo "pin: $pin"
        echo
        cat "$out/recipe-$name.txt"
        echo '```'
    } > "$out/notes.txt"
    gh release view "$tag" --repo "$REPO" > /dev/null 2>&1 \
        || gh release create "$tag" --repo "$REPO" --prerelease --latest=false --target "${GITHUB_SHA:-main}" \
               --title "$name for the Windows build, not for downloading" --notes-file "$out/notes.txt" > /dev/null
    gh release upload "$tag" --repo "$REPO" "$out/$file" "$out/$file.sha256" "$out/recipe-$name.txt" --clobber > /dev/null
    echo "pin: $pin"
    echo "pack published: $file ($(du -k "$out/$file" | cut -f1) KiB, pre-release $tag)"
    rm -rf "$out"
    ;;
*)
    sed -n '2,25p' "$0"
    exit 2
    ;;
esac
