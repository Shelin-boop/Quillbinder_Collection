#!/bin/bash
# Builds the two FFmpeg sets of the media add-ons from the pinned FFmpeg 9.1 source
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
# 0.93.0: FFmpeg 9.1 from ffmpeg.org (Ubuntu's pool has 9.0.2 only). The pin is the SHA-256 of the archive; while it is
# empty, the archive is accepted only with a good release signature of the key below (tools/verify-ffmpeg-source.sh),
# which prints the SHA-256 and "pin this": put it here. With a pin the SHA-256 decides, as before.
VERSION=9.1
URL=https://ffmpeg.org/releases/ffmpeg-$VERSION.tar.xz
SIG_URL=$URL.asc
KEYS_URL=https://ffmpeg.org/ffmpeg-devel.asc
# FFmpeg's release signing key (primary key fingerprint); the script stops and prints what it found when the key file differs
KEY_FPR=FCF986EA15E6E293A5644F10B4322F04D67658D8
SHA256="1a4d394ae63d190a526cd052eb4415c156b5c5e2df37efab41a7de0b8fde2433"
TARBALL=ffmpeg-$VERSION.tar.xz
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
[ -f "$TARBALL" ] || curl -fsSLO "$URL"
if [ -n "$SHA256" ]; then
    echo "$SHA256  $TARBALL" | sha256sum -c -
else
    echo "get-ffmpeg: no SHA-256 pinned yet: only a good release signature is accepted"
    curl -fsSL -o "$TARBALL.asc" "$SIG_URL"
    curl -fsSL -o ffmpeg-devel.asc "$KEYS_URL"
    "$ROOT/tools/verify-ffmpeg-source.sh" "$TARBALL" "$TARBALL.asc" ffmpeg-devel.asc "$KEY_FPR"
fi
if [ ! -f ffmpeg-$VERSION/configure ]; then
    rm -rf ffmpeg-$VERSION
    tar xf "$TARBALL"
fi
if need "$BASE" base; then
    rm -rf "$BASE"
    "$ROOT/tools/build-ffmpeg.sh" "$WORK/ffmpeg-$VERSION" "$BASE" base
fi
if need "$FULL" full; then
    rm -rf "$FULL"
    "$ROOT/tools/build-ffmpeg.sh" "$WORK/ffmpeg-$VERSION" "$FULL" full
fi
echo "FFmpeg built: $BASE (base), $FULL (full)"
