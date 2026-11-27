#!/usr/bin/env bash
# Ready-made dependency packs for the Android build (Android 0.91.3, 2026-11-16; working rules section 10, decision
# D1): Hunspell (packaging/android/android-hunspell.sh) and OpenSSL (packaging/android/android-openssl.sh) for one
# ABI, each built once per recipe and kept as a .tar.xz on a pre-release "deps-<name>-android-<first 16 characters of
# the hash>", titled "not for downloading" (as packaging/windows/win-deps-pack.sh does for
# Windows). Runs on GitHub's Linux machine (.github/workflows/android.yml, packaging/android/android-ci.sh).
#
#   packaging/android/android-deps-pack.sh recipe <name> <ndk>         the recipe (lines of text)
#   packaging/android/android-deps-pack.sh hash|tag|file <name> <ndk>  its SHA-256, the pre-release's tag, the file name
#   packaging/android/android-deps-pack.sh get <name> <ndk> <dir>      unpack the pinned pack of this recipe into <dir>
#                                                                      (exit 1 when none is pinned: the caller builds)
#   packaging/android/android-deps-pack.sh publish <name> <ndk> <dir>  pack <dir>, put it on its pre-release (if not
#                                                                      there yet) and print the line "pin: ..."
#
# <name>: hunspell-<abi> or openssl-<abi> (abi: arm64-v8a, x86_64). The pins: lines "<name> <hash> <file> <SHA-256 of
# the file>" in the variable QB_ANDROID_PACKS (set in android.yml). A recipe without a pin is
# built as before; its pack is then published and its pin line printed (summary.txt shows it).
#
# A recipe is what decides the pack's content: the build script (it pins the source archive by its SHA-256), the ABI
# and the NDK's version (source.properties).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
REPO=Shelin-boop/Quillbinder_Collection

ndk_version() { sed -n 's/^Pkg.Revision *= *//p' "$1/source.properties" 2>/dev/null | tr -d '\r' | head -1; }

recipe() { # <name> <ndk>
    local name=$1 ndk=$2 what abi
    what=${name%%-*}
    abi=${name#*-}
    case "$abi" in arm64-v8a | x86_64) ;; *) echo "android-deps-pack.sh: unknown ABI in $name" >&2; exit 2 ;; esac
    local v
    v=$(ndk_version "$ndk")
    [ -n "$v" ] || { echo "android-deps-pack.sh: the NDK's version is not known ($ndk/source.properties)" >&2; exit 2; }
    case "$what" in
    hunspell | openssl | crypto)
        echo "pack: $what for Android ($abi)"
        echo "script: packaging/android/android-$what.sh $(sha256sum "$ROOT/packaging/android/android-$what.sh" | cut -d' ' -f1)"
        # (the crypto recipe takes its pins from the Linux recipe's script)
        [ "$what" = crypto ] && echo "pins: $(grep -E '^(BOTAN|JSONC|BZIP2|ZLIB|RNP)_(TAG|COMMIT)=|^SEXPP_COMMIT=|^BOTAN_MODULES=' "$ROOT/tools/get-crypto.sh" | sha256sum | cut -d' ' -f1)"
        echo "ndk: $v"
        ;;
    *) echo "android-deps-pack.sh: unknown pack $name (hunspell-<abi>, openssl-<abi>, crypto-<abi>)" >&2; exit 2 ;;
    esac
}

hash_of() { recipe "$1" "$2" | sha256sum | cut -d' ' -f1; }

# Android 0.92.0 (the move to the one public repository): the pre-release's text names what is inside, its license,
# and where its source is, as packaging/windows/win-deps-pack.sh does for the Windows packs; the license texts are in
# the pack.
about() { # <name> <dir>
    local what=${1%%-*} abi=${1#*-} v
    case "$what" in
    hunspell)
        v=$(sed -n 's/^VERSION=//p' "$ROOT/packaging/android/android-hunspell.sh")
        echo "Inside: Hunspell $v as a static library for Android ($abi), unchanged, built with the Android NDK by packaging/android/android-hunspell.sh (the source archive is pinned there by its SHA-256)."
        echo "License: MPL-1.1, GPL-2.0-or-later or LGPL-2.1-or-later (Hunspell's three licenses). Source: https://github.com/hunspell/hunspell at the tag v$v."
        ;;
    openssl)
        v=$(sed -n 's/^VERSION=//p' "$ROOT/packaging/android/android-openssl.sh")
        echo "Inside: OpenSSL $v as the shared libraries libcrypto_3.so and libssl_3.so for Android ($abi), unchanged, built with the Android NDK by packaging/android/android-openssl.sh (the source archive is pinned there by its SHA-256)."
        echo "License: Apache-2.0. Source: https://github.com/openssl/openssl/releases/tag/openssl-$v (also on this repository's release third-party-sources)."
        ;;
    crypto)
        echo "Inside: RNP and Botan 3 (the OpenPGP engine of the encryption), json-c, bzip2 and zlib as static libraries for Android ($abi), unchanged, built with the Android NDK by packaging/android/android-crypto.sh (sources pinned by tag and commit in tools/get-crypto.sh)."
        echo "License: BSD-2-Clause (RNP, Botan), MIT (json-c), bzip2's BSD-style license, zlib license. Sources: https://github.com/rnpgp/rnp, https://github.com/randombit/botan."
        ;;
    esac
    local texts
    texts=$(cd "$2" 2>/dev/null && find . -maxdepth 3 \( -iname 'COPYING*' -o -iname 'LICENSE*' -o -iname 'license.*' \) -type f | sed 's|^\./||' | sort | tr '\n' ' ')
    [ -n "$texts" ] && echo "The license texts are in the pack: $texts"
    return 0
}
tag_of() { echo "deps-$1-android-$(hash_of "$1" "$2" | cut -c1-16)"; }
file_of() { echo "$1-android-$(hash_of "$1" "$2" | cut -c1-16).tar.xz"; }

cmd=${1:-}
name=${2:-}
ndk=${3:-}
case "$cmd" in
recipe) recipe "$name" "$ndk" ;;
hash) hash_of "$name" "$ndk" ;;
tag) tag_of "$name" "$ndk" ;;
file) file_of "$name" "$ndk" ;;
get)
    dir=${4:?dir}
    h=$(hash_of "$name" "$ndk")
    pin=$(printf '%s\n' "${QB_ANDROID_PACKS:-}" | awk -v n="$name" -v h="$h" '$1 == n && $2 == h { print $3, $4; exit }')
    if [ -z "$pin" ]; then
        echo "no pack pinned for $name (recipe $(echo "$h" | cut -c1-16))"
        exit 1
    fi
    file=${pin% *}
    sum=${pin#* }
    tmp=$(mktemp -d)
    if ! gh release download "$(tag_of "$name" "$ndk")" --repo "$REPO" -p "$file" -D "$tmp" > /dev/null 2>&1; then
        echo "the pack $file could not be downloaded: building instead"
        rm -rf "$tmp"
        exit 1
    fi
    if ! echo "$sum  $tmp/$file" | sha256sum -c --quiet > /dev/null 2>&1; then
        echo "the pack $file does not match its pinned SHA-256: building instead"
        rm -rf "$tmp"
        exit 1
    fi
    rm -rf "$dir"
    mkdir -p "$dir"
    tar -xJf "$tmp/$file" -C "$dir"
    rm -rf "$tmp"
    echo "from the pack $file (pre-release $(tag_of "$name" "$ndk"))"
    ;;
publish)
    dir=${4:?dir}
    tag=$(tag_of "$name" "$ndk")
    file=$(file_of "$name" "$ndk")
    if gh api "repos/$REPO/releases/tags/$tag" --jq '.assets[].name' 2> /dev/null | grep -qxF "$file"; then
        echo "pack there already: $file"
        exit 0
    fi
    out=$(mktemp -d)
    recipe "$name" "$ndk" > "$out/recipe-$name.txt"
    (cd "$dir" && tar --owner=0 --group=0 --sort=name -cJf "$out/$file" .)
    sum=$(sha256sum "$out/$file" | cut -d' ' -f1)
    echo "$sum  $file" > "$out/$file.sha256"
    pin="$name $(hash_of "$name" "$ndk") $file $sum"
    {
        echo "Built by .github/workflows/android.yml (run ${GITHUB_RUN_ID:-local}, commit ${GITHUB_SHA:-local}) for packaging/android/android-deps-pack.sh."
        echo "Not for downloading: working storage of the Android build."
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
               --title "$name for the Android build, not for downloading" --notes-file "$out/notes.txt" > /dev/null
    gh release edit "$tag" --repo "$REPO" --draft=false > /dev/null 2>&1 || true
    gh release upload "$tag" --repo "$REPO" "$out/$file" "$out/$file.sha256" "$out/recipe-$name.txt" --clobber > /dev/null
    echo "pin: $pin"
    echo "pack published: $file ($(du -k "$out/$file" | cut -f1) KiB, pre-release $tag)"
    rm -rf "$out"
    ;;
*)
    sed -n '2,20p' "$0"
    exit 2
    ;;
esac
