#!/bin/bash
# The compiled files of the last build of main, for a Claude Code session (0.91.2, build optimization P5, a trial).
#
# Every session starts with an empty machine, so it compiled the program and its test programs from nothing (on two
# cores about 14 min for the test build and 9 min for the AppImage's). The workflow
# .github/workflows/ccache.yml builds both on main in a clean Ubuntu 24.04 container, in the session's own folder
# (/home/claude/quillbinder_collection: the compile commands name absolute paths, so the folder must be the same), through
# ccache, and keeps ccache's folder on the pre-release "ccache-linux" ("not for
# downloading"). ccache compares the compiler, every flag and every file a source includes: a hit gives the same
# object file as a compile, anything changed is compiled.
#
#   tools/get-ccache.sh        (once per session, after tools/cloud-setup.sh; before the first build)
#
# It installs ccache (Ubuntu's package) when it is missing, writes ccache's settings (~/.config/ccache/ccache.conf:
# the cache in ~/.cache/ccache, at most 2 GB, the compiler known by "g++ -v", so that the same Ubuntu package on
# another machine gives hits), downloads the folder, checks it against the SHA-256 beside it and unpacks it, and
# leaves the file ~/.config/ccache/quillbinder, by which CMakeLists.txt compiles through ccache. Without gh, without
# a pack or with a damaged one the cache simply starts empty.
# QB_CCACHE_PACK=0: the settings only, no download.
set -euo pipefail
REPO=${QB_DEPS_REPO:-Shelin-boop/Quillbinder_Collection}
TAG=ccache-linux
FILE=ccache-linux.tar
CONF=$HOME/.config/ccache
DIR=$HOME/.cache/ccache

if ! command -v ccache > /dev/null; then
    export DEBIAN_FRONTEND=noninteractive
    apt-get install -y -qq --no-install-recommends ccache > /dev/null 2>&1 \
        || { apt-get update -qq && apt-get install -y -qq --no-install-recommends ccache > /dev/null; }
fi
mkdir -p "$CONF" "$DIR"
cat > "$CONF/ccache.conf" <<EOF
cache_dir = $DIR
max_size = 2G
compiler_check = %compiler% -v
EOF
: > "$CONF/quillbinder"

if [ "${QB_CCACHE_PACK:-1}" != 0 ] && command -v gh > /dev/null; then
    list=$(gh api "repos/$REPO/releases/tags/$TAG" --jq '.assets[] | "\(.name) \(.id)"' 2>/dev/null || true)
    id=$(awk -v f="$FILE" '$1 == f {print $2}' <<< "$list")
    sid=$(awk -v f="$FILE.sha256" '$1 == f {print $2}' <<< "$list")
    if [ -n "$id" ] && [ -n "$sid" ]; then
        work=$(mktemp -d)
        if gh api "repos/$REPO/releases/assets/$sid" -H 'Accept: application/octet-stream' > "$work/$FILE.sha256" \
            && gh api "repos/$REPO/releases/assets/$id" -H 'Accept: application/octet-stream' > "$work/$FILE" \
            && (cd "$work" && sha256sum -c --quiet "$FILE.sha256"); then
            tar -C "$DIR" -xf "$work/$FILE"
            echo "get-ccache: the compiled files of $(gh api "repos/$REPO/releases/tags/$TAG" --jq '.body' 2>/dev/null \
                | sed -n 's/^commit: *//p' | head -1) unpacked ($(du -sh "$DIR" | cut -f1))"
        else
            echo "get-ccache: the download failed or its SHA-256 differs: an empty cache"
        fi
        rm -rf "$work"
    else
        echo "get-ccache: no compiled files on the pre-release $TAG yet: an empty cache"
    fi
else
    echo "get-ccache: no download (QB_CCACHE_PACK=0 or no gh): an empty cache"
fi
ccache -z > /dev/null
echo "get-ccache: builds of this tree now compile through ccache ($DIR)"
