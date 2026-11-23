#!/bin/bash
# Ready-made dependency packs for the Linux build (2026-11-15, working rules section 10, decision D1).
#
# FFmpeg (both sets of tools/get-ffmpeg.sh) and whisper.cpp (tools/get-whisper.sh) are built from pinned sources.
# The workflow .github/workflows/deps.yml builds each of them once per recipe and puts the result as a
# .tar.xz on a pre-release titled "not for downloading" (as qt-6.12.0-linux holds Qt).
# tools/deps-packs.txt pins every pack by its SHA-256, as tools/get-qt.sh pins Qt's archive. get-ffmpeg.sh and
# get-whisper.sh call "fetch" first and build from source only when there is no pinned pack of their recipe, or when
# the download fails.
#
# The recipe is what decides the result: the build scripts, the pinned versions inside them, the patches, the build
# machine (Ubuntu release, processor type) and the versions of the tools that compile. Its SHA-256 (the recipe hash)
# names the pack; a changed script, patch, pinned version or compiler gives a new hash and so a new pack, built once.
#
#   tools/deps-pack.sh recipe <ffmpeg|whisper>              the recipe's lines (what the hash is made of)
#   tools/deps-pack.sh hash <ffmpeg|whisper>                its SHA-256
#   tools/deps-pack.sh fetch ffmpeg <base prefix> <full prefix>
#   tools/deps-pack.sh fetch whisper <prefix>               download the pinned pack of this recipe, check its SHA-256,
#                                                           unpack it into the prefixes; exit 1 when there is none
#   tools/deps-pack.sh pack ffmpeg <out dir> <base prefix> <full prefix>
#   tools/deps-pack.sh pack whisper <out dir> <prefix>      make the pack of finished prefixes (deps.yml)
#
# QB_DEPS_PACK=0: never download (the release workflow: it builds from source or uses its own cache, D3).
# QB_DEPS_REPO: the repository with the packs (default Shelin-boop/Quillbinder_Collection); "fetch" needs gh, logged in.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
# the layout of a pack: change this number whenever "pack" or "fetch" put things differently
FORMAT=1
REPO=${QB_DEPS_REPO:-Shelin-boop/Quillbinder_Collection}
PINS=$ROOT/tools/deps-packs.txt

die() { echo "deps-pack: $*" >&2; exit 2; }
filehash() { sha256sum "$ROOT/$1" | cut -d' ' -f1; }
first() { "$@" 2>/dev/null | head -1 || true; }
pkgver() { dpkg-query -W -f='${Version}' "$1" 2>/dev/null || echo none; }

recipe() {
    local os
    os=$(. /etc/os-release 2>/dev/null && echo "$ID $VERSION_ID")
    echo "pack: $1 linux format $FORMAT"
    echo "machine: ${os:-unknown} $(uname -m)"
    echo "gcc: $(first gcc -dumpfullversion)"
    case "$1" in
    ffmpeg)
        echo "nasm: $(first nasm -v)"
        echo "file tools/get-ffmpeg.sh $(filehash tools/get-ffmpeg.sh)"
        echo "file tools/build-ffmpeg.sh $(filehash tools/build-ffmpeg.sh)"
        echo "file tools/verify-ffmpeg-source.sh $(filehash tools/verify-ffmpeg-source.sh)"
        local p
        for p in "$ROOT"/tools/ffmpeg-patches/*.patch; do
            [ -f "$p" ] && echo "file tools/ffmpeg-patches/$(basename "$p") $(filehash "tools/ffmpeg-patches/$(basename "$p")")"
        done
        ;;
    whisper)
        echo "cmake: $(first cmake --version)"
        echo "ninja: $(first ninja --version)"
        # the graphics-card module: built only where glslc and the headers are there (get-whisper.sh)
        local vulkan=OFF
        if command -v glslc >/dev/null && [ -f /usr/include/vulkan/vulkan.h ] && [ -d /usr/share/cmake/SPIRV-Headers ]; then
            vulkan=ON
        fi
        echo "vulkan: $vulkan glslc $(pkgver glslc) libvulkan-dev $(pkgver libvulkan-dev) spirv-headers $(pkgver spirv-headers)"
        echo "file tools/get-whisper.sh $(filehash tools/get-whisper.sh)"
        ;;
    *) die "unknown pack: $1 (ffmpeg or whisper)" ;;
    esac
}
recipehash() { recipe "$1" | sha256sum | cut -d' ' -f1; }
short() { echo "${1:0:16}"; }
tagof() { echo "deps-$1-linux-$(short "$2")"; }
fileof() { echo "$1-linux-$(short "$2").tar.xz"; }

# put the pack's folders into place: a folder of the pack -> a prefix (the old prefix goes)
place() {  # <unpacked folder> <prefix>
    local from=$1 to=$2 old
    old=$(cat "$from/prefix.txt" 2>/dev/null || true)
    rm -f "$from/prefix.txt"
    # the libraries find each other by their names ($ORIGIN, sonames), not by the prefix; the few text files that name
    # the prefix of the build machine (pkg-config, CMake) get the new one, so that nothing points to a missing folder
    if [ -n "$old" ]; then
        grep -rlF --include='*.pc' --include='*.cmake' "$old" "$from" 2>/dev/null | while IFS= read -r f; do
            sed -i "s|$old|$to|g" "$f"
        done
    fi
    rm -rf "$to"
    mkdir -p "$(dirname "$to")"
    mv "$from" "$to"
}

fetch() {  # <name> <prefix...>
    local name=$1; shift
    if [ "${QB_DEPS_PACK:-1}" = 0 ]; then
        echo "deps-pack: QB_DEPS_PACK=0, no download: building $name from source"
        return 1
    fi
    local h file tag sum
    h=$(recipehash "$name")
    file=$(fileof "$name" "$h")
    if [ ! -f "$PINS" ]; then
        echo "deps-pack: no list of packs (tools/deps-packs.txt): building $name from source"
        return 1
    fi
    sum=$(awk -v n="$name" -v h="$h" '$1==n && $2==h {print $4}' "$PINS" | head -1)
    if [ -z "$sum" ]; then
        echo "deps-pack: no pack of this $name recipe ($(short "$h")) in tools/deps-packs.txt: building from source"
        echo "  (push the branch: .github/workflows/deps.yml builds the pack; then pin it)"
        return 1
    fi
    if ! command -v gh >/dev/null; then
        echo "deps-pack: no gh to download $file: building $name from source"
        return 1
    fi
    tag=$(tagof "$name" "$h")
    local work=$ROOT/build/deps-download id
    mkdir -p "$work"
    if ! echo "$sum  $work/$file" | sha256sum -c --status 2>/dev/null; then
        id=$(gh api "repos/$REPO/releases/tags/$tag" --jq ".assets[] | select(.name==\"$file\") | .id" 2>/dev/null || true)
        if [ -z "$id" ]; then
            echo "deps-pack: $file is not on the pre-release $tag of $REPO: building $name from source"
            return 1
        fi
        if ! gh api "repos/$REPO/releases/assets/$id" -H 'Accept: application/octet-stream' > "$work/$file.part" 2>/dev/null; then
            rm -f "$work/$file.part"
            echo "deps-pack: the download of $file failed: building $name from source"
            return 1
        fi
        mv "$work/$file.part" "$work/$file"
    fi
    if ! echo "$sum  $work/$file" | sha256sum -c --status; then
        rm -f "$work/$file"
        echo "deps-pack: $file does not have the SHA-256 pinned in tools/deps-packs.txt: not used, building $name from source" >&2
        return 1
    fi
    local x=$work/unpack-$name
    rm -rf "$x" && mkdir -p "$x"
    tar -C "$x" -xJf "$work/$file"
    case "$name" in
    ffmpeg)
        [ $# = 2 ] || die "fetch ffmpeg <base prefix> <full prefix>"
        [ -f "$x/base/set.txt" ] && [ -f "$x/full/set.txt" ] || die "$file has no base and full set"
        place "$x/base" "$1"
        place "$x/full" "$2"
        ;;
    whisper)
        [ $# = 1 ] || die "fetch whisper <prefix>"
        [ -f "$x/whisper/version.txt" ] || die "$file has no whisper folder"
        place "$x/whisper" "$1"
        ;;
    esac
    rm -rf "$x" "$work/$file"
    echo "deps-pack: $name from $file (recipe $(short "$h"), SHA-256 checked)"
}

pack() {  # <name> <out dir> <prefix...>
    local name=$1 out=$2; shift 2
    local h file s
    h=$(recipehash "$name")
    file=$(fileof "$name" "$h")
    mkdir -p "$out"
    out=$(cd "$out" && pwd)
    s=$(mktemp -d)
    case "$name" in
    ffmpeg)
        [ $# = 2 ] || die "pack ffmpeg <out dir> <base prefix> <full prefix>"
        # only finished sets: each with its marker and its libraries
        [ "$(cat "$1/set.txt" 2>/dev/null)" = base ] && [ -f "$1/lib/libavcodec.so.63" ] || die "no finished base set in $1"
        [ "$(cat "$2/set.txt" 2>/dev/null)" = full ] && [ -f "$2/lib/libavcodec.so.63" ] || die "no finished full set in $2"
        cp -a "$1" "$s/base"
        cp -a "$2" "$s/full"
        (cd "$1" && pwd) > "$s/base/prefix.txt"
        (cd "$2" && pwd) > "$s/full/prefix.txt"
        ;;
    whisper)
        [ $# = 1 ] || die "pack whisper <out dir> <prefix>"
        [ -f "$1/version.txt" ] && [ -f "$1/lib/libwhisper.so.1" ] || die "no finished whisper.cpp in $1"
        cp -a "$1" "$s/whisper"
        (cd "$1" && pwd) > "$s/whisper/prefix.txt"
        ;;
    esac
    # the same files give the same archive: names sorted, no owners, no times of the build
    XZ_OPT='-T0 -6' tar -C "$s" --sort=name --owner=0 --group=0 --numeric-owner --mtime='@0' -cJf "$out/$file" .
    rm -rf "$s"
    recipe "$name" > "$out/recipe-$name.txt"
    echo "$h" > "$out/hash-$name.txt"
    (cd "$out" && sha256sum "$file" > "$file.sha256")
    echo "pin: $name $h $file $(cut -d' ' -f1 "$out/$file.sha256")" | tee "$out/pin-$name.txt"
}

cmd=${1:-}; [ $# -ge 2 ] || die "usage: deps-pack.sh recipe|hash|fetch|pack <ffmpeg|whisper> ..."
shift
case "$cmd" in
recipe) recipe "$1" ;;
hash) recipehash "$1" ;;
tag) tagof "$1" "$(recipehash "$1")" ;;
file) fileof "$1" "$(recipehash "$1")" ;;
fetch) fetch "$@" ;;
pack) pack "$@" ;;
*) die "unknown command: $cmd" ;;
esac
