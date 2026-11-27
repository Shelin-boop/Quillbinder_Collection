#!/bin/bash
# Ready-made dependency packs for the Linux build (2026-11-15, working rules section 10, decision D1).
#
# FFmpeg (both sets of tools/get-ffmpeg.sh), whisper.cpp (tools/get-whisper.sh), the translator's engine libraries
# (tools/get-translator-deps.sh; 0.93.0), the encryption libraries (tools/get-crypto.sh; 0.99.0), ONNX Runtime
# (tools/get-onnxruntime.sh; 0.99.0) and the AI engine libraries (tools/get-ai-engine.sh; 0.99.0) are built from pinned
# sources (ONNX Runtime: the official archive, pinned by its SHA-256).
# The workflow .github/workflows/deps.yml builds each of them once per recipe and puts the result as a
# .tar.xz on a pre-release titled "not for downloading" (as qt-6.12.0-linux holds Qt).
# tools/deps-packs.txt pins every pack by its SHA-256, as tools/get-qt.sh pins Qt's archive. get-ffmpeg.sh and
# get-whisper.sh and the other get-*.sh call "fetch" first and build from source only when there is no pinned pack of their recipe, or when
# the download fails.
#
# The recipe is what decides the result: the build scripts, the pinned versions inside them, the patches, the build
# machine (Ubuntu release, processor type) and the versions of the tools that compile. Its SHA-256 (the recipe hash)
# names the pack; a changed script, patch, pinned version or compiler gives a new hash and so a new pack, built once.
#
#   tools/deps-pack.sh recipe <name>                        the recipe's lines (what the hash is made of)
#   tools/deps-pack.sh hash <name>                          its SHA-256
#                         (the names: ffmpeg, whisper, translator, crypto, onnxruntime, aiengine)
#   tools/deps-pack.sh about <name>                         what the pack holds, its licenses and the source (the text
#                                                           of its pre-release, made by deps.yml)
#   tools/deps-pack.sh fetch ffmpeg <base prefix> <full prefix>
#   tools/deps-pack.sh fetch whisper <prefix>               download the pinned pack of this recipe, check its SHA-256,
#                                                           unpack it into the prefixes; exit 1 when there is none
#   tools/deps-pack.sh fetch translator <folder>            (build/translator-deps: install/ and the license texts)
#   tools/deps-pack.sh fetch crypto <prefix>                (static libraries, headers, licenses/)
#   tools/deps-pack.sh fetch onnxruntime <prefix>           (headers, libonnxruntime.so*, licenses/)
#   tools/deps-pack.sh fetch aiengine <prefix>              (shared libraries and compute modules, headers, licenses/)
#   tools/deps-pack.sh pack ffmpeg <out dir> <base prefix> <full prefix>
#   tools/deps-pack.sh pack whisper <out dir> <prefix>      make the pack of finished prefixes (deps.yml)
#   tools/deps-pack.sh pack translator <out dir> <folder>
#   tools/deps-pack.sh pack crypto|onnxruntime|aiengine <out dir> <prefix>
#
# QB_DEPS_PACK=0: never download (the release workflow: it builds from source or uses its own cache, D3).
# QB_DEPS_REPO: the repository with the packs (default Shelin-boop/Quillbinder_Collection); "fetch" needs gh, logged in.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
# the layout of a pack: change this number whenever "pack" or "fetch" put things differently
FORMAT=1
REPO=${QB_DEPS_REPO:-Shelin-boop/Quillbinder_Collection}
PINS=$ROOT/tools/deps-packs.txt
TP_BUILD=addons/translation-proofreading/source/packaging/build-deps.sh
# the license files of the translator's sources that the add-on's make-addon.sh copies (the pack holds them, so that
# the add-on can be made from a pack; tests/deps_packs_tests.py compares this list with make-addon.sh)
TP_LICENSES="CTranslate2/LICENSE CTranslate2/third_party/cpu_features/LICENSE CTranslate2/third_party/spdlog/LICENSE
oneDNN/LICENSE oneDNN/THIRD-PARTY-PROGRAMS sentencepiece/LICENSE sentencepiece/third_party/absl/LICENSE
sentencepiece/third_party/protobuf-lite/LICENSE sentencepiece/third_party/darts_clone/LICENSE sentencepiece/third_party/esaxx/LICENSE"

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
    translator)
        echo "cmake: $(first cmake --version)"
        echo "ninja: $(first ninja --version)"
        echo "file tools/get-translator-deps.sh $(filehash tools/get-translator-deps.sh)"
        echo "file $TP_BUILD $(filehash "$TP_BUILD")"
        ;;
    crypto)
        echo "cmake: $(first cmake --version)"
        echo "ninja: $(first ninja --version)"
        echo "python3: $(first python3 --version)"
        echo "make: $(first make --version)"
        echo "file tools/get-crypto.sh $(filehash tools/get-crypto.sh)"
        ;;
    onnxruntime)
        echo "file tools/get-onnxruntime.sh $(filehash tools/get-onnxruntime.sh)"
        ;;
    aiengine)
        echo "cmake: $(first cmake --version)"
        echo "ninja: $(first ninja --version)"
        # the graphics-card module: built only where glslc and the headers are there (get-ai-engine.sh)
        local vk=OFF
        if command -v glslc >/dev/null && [ -f /usr/include/vulkan/vulkan.h ] && [ -d /usr/share/cmake/SPIRV-Headers ]; then
            vk=ON
        fi
        echo "vulkan: $vk glslc $(pkgver glslc) libvulkan-dev $(pkgver libvulkan-dev) spirv-headers $(pkgver spirv-headers)"
        echo "file tools/get-ai-engine.sh $(filehash tools/get-ai-engine.sh)"
        echo "file tools/ai-engine/qwentts/CMakeLists.txt $(filehash tools/ai-engine/qwentts/CMakeLists.txt)"
        ;;
    *) die "unknown pack: $1 (ffmpeg, whisper, translator, crypto, onnxruntime or aiengine)" ;;
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
        # (grep finds nothing in a pack without such files: that is no failure, as pipefail would make it)
        { grep -rlF --include='*.pc' --include='*.cmake' "$old" "$from" 2>/dev/null || true; } | while IFS= read -r f; do
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
    translator)
        [ $# = 1 ] || die "fetch translator <folder>"
        [ -f "$x/translator/install/lib/libctranslate2.a" ] || [ -f "$x/translator/install/lib64/libctranslate2.a" ] \
            || die "$file has no translator libraries"
        place "$x/translator" "$1"
        ;;
    crypto)
        [ $# = 1 ] || die "fetch crypto <prefix>"
        [ -f "$x/crypto/version.txt" ] && [ -f "$x/crypto/lib/librnp.a" ] || die "$file has no encryption libraries"
        place "$x/crypto" "$1"
        ;;
    onnxruntime)
        [ $# = 1 ] || die "fetch onnxruntime <prefix>"
        [ -f "$x/onnxruntime/version.txt" ] && [ -f "$x/onnxruntime/include/onnxruntime_c_api.h" ] || die "$file has no ONNX Runtime"
        place "$x/onnxruntime" "$1"
        ;;
    aiengine)
        [ $# = 1 ] || die "fetch aiengine <prefix>"
        [ -f "$x/aiengine/version.txt" ] && [ -f "$x/aiengine/lib/libggml.so" ] || die "$file has no AI engine libraries"
        place "$x/aiengine" "$1"
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
    translator)
        [ $# = 1 ] || die "pack translator <out dir> <folder>"
        [ -f "$1/install/lib/libctranslate2.a" ] || [ -f "$1/install/lib64/libctranslate2.a" ] || die "no finished translator libraries in $1"
        mkdir -p "$s/translator"
        cp -a "$1/install" "$s/translator/install"
        local f
        for f in $TP_LICENSES; do
            [ -f "$1/$f" ] || die "the license file $f is missing in $1"
            mkdir -p "$s/translator/$(dirname "$f")"
            cp "$1/$f" "$s/translator/$f"
        done
        (cd "$1" && pwd) > "$s/translator/prefix.txt"
        ;;
    crypto)
        [ $# = 1 ] || die "pack crypto <out dir> <prefix>"
        [ -f "$1/version.txt" ] && [ -f "$1/lib/librnp.a" ] && [ -f "$1/lib/libbotan-3.a" ] || die "no finished encryption libraries in $1"
        cp -a "$1" "$s/crypto"
        (cd "$1" && pwd) > "$s/crypto/prefix.txt"
        ;;
    onnxruntime)
        [ $# = 1 ] || die "pack onnxruntime <out dir> <prefix>"
        [ -f "$1/version.txt" ] && [ -f "$1/include/onnxruntime_c_api.h" ] && [ -f "$1/lib/libonnxruntime.so" ] || die "no finished ONNX Runtime in $1"
        cp -a "$1" "$s/onnxruntime"
        (cd "$1" && pwd) > "$s/onnxruntime/prefix.txt"
        ;;
    aiengine)
        [ $# = 1 ] || die "pack aiengine <out dir> <prefix>"
        [ -f "$1/version.txt" ] && [ -f "$1/lib/libggml.so" ] && [ -f "$1/lib/libsonic.so" ] || die "no finished AI engine libraries in $1"
        cp -a "$1" "$s/aiengine"
        (cd "$1" && pwd) > "$s/aiengine/prefix.txt"
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

# What a pack holds, its licenses and its source (the text of its pre-release; deps.yml writes it into the pack's files)
setting() { sed -n "s/^$2=\"\{0,1\}\([^\" ]*\).*/\1/p" "$ROOT/$1" | head -1; }
about() {
    case "$1" in
    ffmpeg)
        echo "Inside: FFmpeg $(setting tools/get-ffmpeg.sh VERSION), decoding only, in two sets (base: free formats; full: every decoder but WMA Pro and WMV 9), shared libraries, built from the source by tools/get-ffmpeg.sh and tools/build-ffmpeg.sh (with tools/ffmpeg-patches/)."
        echo "License: LGPL-2.1-or-later (built without GPL and non-free parts); the license text is COPYING.LGPLv2.1 of the source, and the media add-ons carry it. Source: https://ffmpeg.org/releases/ffmpeg-$(setting tools/get-ffmpeg.sh VERSION).tar.xz (release signature checked by tools/verify-ffmpeg-source.sh); the build scripts are in this repository."
        ;;
    whisper)
        echo "Inside: whisper.cpp with ggml, $(setting tools/get-whisper.sh TAG) (commit $(setting tools/get-whisper.sh COMMIT)), unchanged, built by tools/get-whisper.sh (shared libraries, compute modules per instruction set, the Vulkan module where the build machine has it), and the test models and the test recording that whisper.cpp ships (the recording is public domain)."
        echo "License: MIT; its text is the file LICENSE of the source. Source: https://github.com/ggml-org/whisper.cpp at that tag."
        ;;
    translator)
        echo "Inside: oneDNN, CTranslate2 and SentencePiece as static libraries (with the headers), unchanged, built from the sources at fixed versions by $TP_BUILD (oneDNN $(sed -n 's|.*oneDNN.git \(v[0-9.]*\).*|\1|p' "$ROOT/$TP_BUILD"), CTranslate2 $(sed -n 's|.*CTranslate2.git \(v[0-9.]*\).*|\1|p' "$ROOT/$TP_BUILD"), SentencePiece $(sed -n 's|.*sentencepiece.git \(v[0-9.]*\).*|\1|p' "$ROOT/$TP_BUILD")), and the license files of their sources."
        echo "Licenses: oneDNN under Apache-2.0; CTranslate2 under MIT (with cpu_features under Apache-2.0 and spdlog under MIT); SentencePiece under Apache-2.0 (with abseil under Apache-2.0, protobuf-lite under BSD-3-Clause, darts_clone under BSD-2-Clause and esaxx under MIT). The license texts are in the pack (the files LICENSE and THIRD-PARTY-PROGRAMS of the sources). Sources: https://github.com/uxlfoundation/oneDNN, https://github.com/OpenNMT/CTranslate2 and https://github.com/google/sentencepiece at those tags."
        ;;
    crypto)
        echo "Inside: the encryption libraries built into the program, as static libraries (with the headers), unchanged, built from the sources at fixed versions by tools/get-crypto.sh: Botan $(setting tools/get-crypto.sh BOTAN_TAG) (a minimized build), json-c $(setting tools/get-crypto.sh JSONC_TAG), bzip2 $(setting tools/get-crypto.sh BZIP2_TAG), zlib $(setting tools/get-crypto.sh ZLIB_TAG), RNP $(setting tools/get-crypto.sh RNP_TAG) (with its sexpp), and the license files of the sources."
        echo "Licenses: Botan under BSD-2-Clause; json-c under MIT; bzip2 under its BSD-style license; zlib under the zlib license; RNP under BSD-2-Clause, BSD-3-Clause, Apache-2.0 and MIT parts (and the OCB license of LICENSE-OCB.md); sexpp under MIT. The license texts are in the pack (the folder licenses). Sources: https://github.com/randombit/botan, https://github.com/json-c/json-c, https://github.com/libarchive/bzip2 (a mirror of bzip2), https://github.com/madler/zlib and https://github.com/rnpgp/rnp at those tags and commits."
        ;;
    onnxruntime)
        echo "Inside: ONNX Runtime $(setting tools/get-onnxruntime.sh VERSION) for Linux x64 as Microsoft publishes it (the headers and libonnxruntime.so*), unchanged, fetched by tools/get-onnxruntime.sh and checked by its SHA-256."
        echo "License: MIT; the license text and the third-party notices of the archive are in the pack (the folder licenses). Source: https://github.com/microsoft/onnxruntime at the tag v$(setting tools/get-onnxruntime.sh VERSION) (the prebuilt archive is the release asset of that tag)."
        ;;
    aiengine)
        echo "Inside: ggml $(setting tools/get-ai-engine.sh GGML_TAG), whisper.cpp $(setting tools/get-ai-engine.sh WHISPER_TAG), llama.cpp $(setting tools/get-ai-engine.sh LLAMA_TAG) (libllama and libmtmd only), qwentts.cpp (commit $(setting tools/get-ai-engine.sh QWENTTS_COMMIT)) and Sonic (commit $(setting tools/get-ai-engine.sh SONIC_COMMIT)), as shared libraries with the headers, built from those sources by tools/get-ai-engine.sh (compute modules per instruction set and the Vulkan module as libraries of their own, the Vulkan module where the build machine has it), and the license files of the sources. ggml.h is changed in one line: the length of a tensor's name is 128 (qwentts.cpp needs it)."
        echo "Licenses: MIT for ggml, whisper.cpp, llama.cpp and qwentts.cpp; Apache-2.0 for Sonic. The license texts are in the pack (the folder licenses). Sources: https://github.com/ggml-org/ggml, https://github.com/ggml-org/whisper.cpp, https://github.com/ggml-org/llama.cpp, https://github.com/ServeurpersoCom/qwentts.cpp and https://github.com/waywardgeek/sonic at those tags and commits."
        ;;
    *) die "unknown pack: $1 (ffmpeg, whisper, translator, crypto, onnxruntime or aiengine)" ;;
    esac
}

cmd=${1:-}; [ $# -ge 2 ] || die "usage: deps-pack.sh recipe|hash|about|fetch|pack <ffmpeg|whisper|translator|crypto|onnxruntime|aiengine> ..."
shift
case "$cmd" in
recipe) recipe "$1" ;;
hash) recipehash "$1" ;;
about) about "$1" ;;
tag) tagof "$1" "$(recipehash "$1")" ;;
file) fileof "$1" "$(recipehash "$1")" ;;
fetch) fetch "$@" ;;
pack) pack "$@" ;;
*) die "unknown command: $cmd" ;;
esac
