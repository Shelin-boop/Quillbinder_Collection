#!/bin/bash
# Fetches the official prebuilt ONNX Runtime for Linux x64 (MIT license) for the engine packages of the AI add-ons (0.99.0):
#   tools/get-onnxruntime.sh [prefix]          (default build/onnxruntime, or QB_ONNXRUNTIME_DIR)
# The prefix gets the headers (include/), libonnxruntime.so* (lib/), the license files (licenses/) and version.txt.
# The program never links this library: it loads it with dlopen and calls OrtGetApiBase, from an engine package that is
# installed later, so there is no link-time dependency on it.
# The archive is the release asset of microsoft/onnxruntime. The session that made this script cannot download from that
# repository's release page (only the git hosts and the package registries are open to it), so the archive's SHA-256 is
# pinned from the first run of the workflow .github/workflows/deps.yml, which can: while SHA256 is empty the download is
# accepted only with QB_ONNXRUNTIME_FIRST_RUN=1 (the workflow sets it), and the script prints the SHA-256 and "pin this:";
# put that line's value below (then the recipe's hash changes and the workflow builds the pack again, in seconds).
# As get-whisper.sh: first the ready-made pack of this recipe (tools/deps-pack.sh, pinned in tools/deps-packs.txt), the
# download only without one (QB_DEPS_PACK=0: always).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PREFIX=${1:-${QB_ONNXRUNTIME_DIR:-$ROOT/build/onnxruntime}}
WORK=$ROOT/build/onnxruntime-src
VERSION=1.32.0
NAME=onnxruntime-linux-x64-$VERSION
URL=https://github.com/microsoft/onnxruntime/releases/download/v$VERSION/$NAME.tgz
SHA256=""
STAMP="onnxruntime $VERSION linux-x64"

done_here() { [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$STAMP" ] && [ -f "$PREFIX/lib/libonnxruntime.so" ] && [ -f "$PREFIX/include/onnxruntime_c_api.h" ]; }
if done_here; then
    echo "ONNX Runtime is there: $PREFIX ($VERSION)"
    exit 0
fi
if "$ROOT/tools/deps-pack.sh" fetch onnxruntime "$PREFIX" && done_here; then
    echo "ONNX Runtime is there: $PREFIX ($VERSION)"
    exit 0
fi
if [ -z "$SHA256" ] && [ "${QB_ONNXRUNTIME_FIRST_RUN:-0}" != 1 ]; then
    echo "get-onnxruntime: no SHA-256 is pinned yet for $NAME.tgz: run the workflow deps.yml (it sets QB_ONNXRUNTIME_FIRST_RUN=1), then pin the value it prints" >&2
    exit 1
fi
mkdir -p "$WORK"
cd "$WORK"
[ -f "$NAME.tgz" ] || curl -fsSL -o "$NAME.tgz" "$URL"
GOT=$(sha256sum "$NAME.tgz" | cut -d' ' -f1)
if [ -n "$SHA256" ]; then
    [ "$GOT" = "$SHA256" ] || { echo "get-onnxruntime: $NAME.tgz has the SHA-256 $GOT, not the pinned $SHA256" >&2; rm -f "$NAME.tgz"; exit 1; }
else
    echo "get-onnxruntime: no SHA-256 pinned yet (first run): the archive has"
    echo "pin this: SHA256=\"$GOT\""
fi
rm -rf "$NAME"
tar xzf "$NAME.tgz"
# what the archive must hold: the version inside it is the one pinned
[ "$(cat "$NAME/VERSION_NUMBER" 2>/dev/null)" = "$VERSION" ] || { echo "get-onnxruntime: VERSION_NUMBER of the archive is not $VERSION" >&2; exit 1; }
[ -f "$NAME/lib/libonnxruntime.so.$VERSION" ] && [ -f "$NAME/include/onnxruntime_c_api.h" ] || { echo "get-onnxruntime: the archive lacks libonnxruntime.so.$VERSION or its headers" >&2; exit 1; }
rm -rf "$PREFIX"
mkdir -p "$PREFIX/licenses"
cp -a "$NAME/include" "$PREFIX/include"
cp -a "$NAME/lib" "$PREFIX/lib"
cp "$NAME/LICENSE" "$PREFIX/licenses/LICENSE-onnxruntime.txt"
cp "$NAME/ThirdPartyNotices.txt" "$PREFIX/licenses/ThirdPartyNotices-onnxruntime.txt"
echo "$STAMP" > "$PREFIX/version.txt"
echo "ONNX Runtime is in $PREFIX ($VERSION)"
ls "$PREFIX/lib"
