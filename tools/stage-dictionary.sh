#!/bin/bash
# Puts the files of a dictionary add-on into the staging folder of packaging/linux/build-addons.sh (0.95.0, item 102):
#   tools/stage-dictionary.sh <addons/dictionary-xx folder> <stage folder>
# The folder's sources.txt names LibreOffice's dictionaries at a pinned tag (https://github.com/LibreOffice/dictionaries) and,
# per file, its path there, its SHA-256 sum and its name in the package. Each file is fetched from raw.githubusercontent.com
# (kept in $QB_DICT_CACHE, default ~/.cache/quillbinder-dictionaries, so a second run needs no network), checked against the
# sum (a different file stops the build) and copied to <stage>/payload/dictionaries/.
set -euo pipefail
DIR=$(cd "$1" && pwd)
STAGE=$2
CACHE=${QB_DICT_CACHE:-$HOME/.cache/quillbinder-dictionaries}
OUT=$STAGE/payload/dictionaries
mkdir -p "$OUT" "$CACHE"
TAG=$(awk '$1=="tag"{print $2}' "$DIR/sources.txt")
[ -n "$TAG" ] || { echo "$DIR/sources.txt names no tag" >&2; exit 1; }
while read -r path sum name; do
    case "$path" in ''|'#'*|tag) continue ;; esac
    cached=$CACHE/$sum
    if [ ! -f "$cached" ] || [ "$(sha256sum "$cached" | cut -d' ' -f1)" != "$sum" ]; then
        curl -fsSL --retry 3 -o "$cached.part" "https://raw.githubusercontent.com/LibreOffice/dictionaries/$TAG/$path"
        got=$(sha256sum "$cached.part" | cut -d' ' -f1)
        if [ "$got" != "$sum" ]; then
            rm -f "$cached.part"
            echo "$path at $TAG has the SHA-256 $got, not $sum: the dictionary changed, check it before pinning the new sum" >&2
            exit 1
        fi
        mv "$cached.part" "$cached"
    fi
    cp "$cached" "$OUT/$name"
done < "$DIR/sources.txt"
