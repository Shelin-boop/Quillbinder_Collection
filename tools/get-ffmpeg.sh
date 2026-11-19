#!/bin/bash
# Builds the two FFmpeg sets of the media add-ons from the pinned FFmpeg 9.0.2 source
# (tools/build-ffmpeg.sh says what is in each and why):
#   base -> "Media Playback"                      (default prefix build/ffmpeg)
#   full -> "Extra Video and Audio Formats"       (default prefix build/ffmpeg-full)
#   tools/get-ffmpeg.sh [base prefix [full prefix]]
# Takes about 10 to 20 minutes for both; needs nasm. Run it once; finished sets are reused.
# 2026-11-15 (working rules, section 10, D1): first the ready-made pack of this recipe (tools/deps-pack.sh,
# pinned in tools/deps-packs.txt; about a minute); built from source only without one (QB_DEPS_PACK=0: always).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
BASE=${1:-${QB_FFMPEG_DIR:-$ROOT/build/ffmpeg}}
FULL=${2:-${QB_FFMPEG_FULL_DIR:-$ROOT/build/ffmpeg-full}}
# (2026-11-15) full paths: the build below runs in another folder, where a relative prefix would point elsewhere
BASE=$(realpath -m "$BASE")
FULL=$(realpath -m "$FULL")
WORK=$ROOT/build/ffmpeg-src
URL=http://archive.ubuntu.com/ubuntu/pool/universe/f/ffmpeg/ffmpeg_9.0.2.orig.tar.xz
SHA256=8c3850283eb25fa026482078a04051e0be17347b09ef81a0849bec15a96e002e
need() {  # <prefix> <set>: missing, or another set (before 0.50 build/ffmpeg held every decoder)
    [ ! -f "$1/lib/libavcodec.so.63" ] || [ "$(cat "$1/set.txt" 2>/dev/null)" != "$2" ]
}
if ! need "$BASE" base && ! need "$FULL" full; then
    echo "FFmpeg is there: $BASE (base), $FULL (full)"
    exit 0
fi
if "$ROOT/tools/deps-pack.sh" fetch ffmpeg "$BASE" "$FULL" && ! need "$BASE" base && ! need "$FULL" full; then
    echo "FFmpeg is there: $BASE (base), $FULL (full)"
    exit 0
fi
mkdir -p "$WORK"
cd "$WORK"
[ -f ffmpeg_9.0.2.orig.tar.xz ] || curl -fsSLO "$URL"
echo "$SHA256  ffmpeg_9.0.2.orig.tar.xz" | sha256sum -c -
if [ ! -f ffmpeg-9.0.2/configure ]; then
    rm -rf ffmpeg-9.0.2
    tar xf ffmpeg_9.0.2.orig.tar.xz
fi
if need "$BASE" base; then
    rm -rf "$BASE"
    "$ROOT/tools/build-ffmpeg.sh" "$WORK/ffmpeg-9.0.2" "$BASE" base
fi
if need "$FULL" full; then
    rm -rf "$FULL"
    "$ROOT/tools/build-ffmpeg.sh" "$WORK/ffmpeg-9.0.2" "$FULL" full
fi
echo "FFmpeg built: $BASE (base), $FULL (full)"
