#!/bin/bash
# Fetches the official prebuilt ONNX Runtime for Linux x64 (MIT license) for the engine packages of the AI add-ons (0.99.0):
#   tools/get-onnxruntime.sh [prefix]          (default build/onnxruntime, or QB_ONNXRUNTIME_DIR)
# The prefix gets the headers (include/), libonnxruntime.so* (lib/), the license files (licenses/) and version.txt.
# The program never links this library: it loads it with dlopen and calls OrtGetApiBase, from an engine package that is
# installed later, so there is no link-time dependency on it.
# The archive is the release asset of microsoft/onnxruntime. The session that made this script cannot reach that
# repository's release page, so the exact asset is found by the first run of the workflow .github/workflows/deps.yml:
# while SHA256 is empty the download is accepted only with QB_ONNXRUNTIME_FIRST_RUN=1 (the workflow sets it). That run asks
# the GitHub API (GH_TOKEN) for the releases of microsoft/onnxruntime, takes the newest one that has a Linux x64 CPU archive
# (onnxruntime-linux-x64-<version>.tgz, not the GPU, CUDA or other variants), prints the tag, the asset names it saw and the
# chosen one, downloads it, lists the archive's top-level contents and prints the line "pin this: VERSION=... TAG=... ASSET=...
# SHA256=...": put those four values below (then the recipe's hash changes and the workflow builds the pack again, in seconds).
# With a pinned SHA256 the download is by TAG and ASSET and the SHA-256 decides.
# As get-whisper.sh: first the ready-made pack of this recipe (tools/deps-pack.sh, pinned in tools/deps-packs.txt), the
# download only without one (QB_DEPS_PACK=0: always).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PREFIX=${1:-${QB_ONNXRUNTIME_DIR:-$ROOT/build/onnxruntime}}
WORK=$ROOT/build/onnxruntime-src
VERSION=1.32.0
TAG=v1.32.0
ASSET=onnxruntime-linux-x64-1.32.0.tgz
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
FIRST=0
if [ -z "$SHA256" ]; then
    if [ "${QB_ONNXRUNTIME_FIRST_RUN:-0}" != 1 ]; then
        echo "get-onnxruntime: no SHA-256 is pinned yet for $ASSET: run the workflow deps.yml (it sets QB_ONNXRUNTIME_FIRST_RUN=1), then pin the line it prints" >&2
        exit 1
    fi
    FIRST=1
fi
mkdir -p "$WORK"
cd "$WORK"
if [ "$FIRST" = 1 ]; then
    # find the asset: the newest published release with a Linux x64 CPU archive
    # (a workflow's token is scoped to this repository: another repository's API may answer 404 to it, so the
    #  public API is asked without the token first)
    api='https://api.github.com/repos/microsoft/onnxruntime/releases?per_page=30'
    curl -fsSL -H 'Accept: application/vnd.github+json' "$api" -o releases.json \
        || curl -fsSL -H "Authorization: Bearer ${GH_TOKEN:-}" -H 'Accept: application/vnd.github+json' "$api" -o releases.json
    python3 -I - releases.json > chosen.txt <<'PY'
import json, re, sys
rels = json.load(open(sys.argv[1]))
pick = None
for r in rels:
    names = [a["name"] for a in r.get("assets", [])]
    print("release %s%s: %d assets; linux archives: %s" % (r["tag_name"], " (draft)" if r.get("draft") else " (pre-release)" if r.get("prerelease") else "",
          len(names), ", ".join(n for n in names if "linux" in n) or "none"), file=sys.stderr)
    if r.get("draft") or r.get("prerelease") or pick:
        continue
    for a in r["assets"]:
        if re.fullmatch(r"onnxruntime-linux-x64-[0-9]+(\.[0-9]+)*\.tgz", a["name"]):
            pick = (r["tag_name"], a["name"], a["url"])
if not pick:
    sys.exit("no published release has an onnxruntime-linux-x64-<version>.tgz asset")
print(*pick)
PY
    read -r TAG ASSET ASSET_URL < chosen.txt
    VERSION=${ASSET#onnxruntime-linux-x64-}
    VERSION=${VERSION%.tgz}
    echo "get-onnxruntime: chosen release $TAG, asset $ASSET"
    rm -f "$ASSET"
    curl -fsSL -H 'Accept: application/octet-stream' -o "$ASSET" "$ASSET_URL" \
        || curl -fsSL -o "$ASSET" "https://github.com/microsoft/onnxruntime/releases/download/$TAG/$ASSET"
else
    [ -f "$ASSET" ] || curl -fsSL -o "$ASSET" "https://github.com/microsoft/onnxruntime/releases/download/$TAG/$ASSET"
fi
GOT=$(sha256sum "$ASSET" | cut -d' ' -f1)
if [ "$FIRST" = 0 ]; then
    [ "$GOT" = "$SHA256" ] || { echo "get-onnxruntime: $ASSET has the SHA-256 $GOT, not the pinned $SHA256" >&2; rm -f "$ASSET"; exit 1; }
else
    echo "get-onnxruntime: no SHA-256 pinned yet (first run). The archive's top-level contents:"
    tar tzf "$ASSET" | awk -F/ 'NF<=3' | head -60
    echo "pin this: VERSION=$VERSION TAG=$TAG ASSET=$ASSET SHA256=\"$GOT\""
fi
# what is really in the archive decides: one top folder with the headers and the libraries inside
rm -rf unpacked && mkdir unpacked
tar xzf "$ASSET" -C unpacked
TOP=$(find unpacked -name onnxruntime_c_api.h -printf '%h\n' | head -1)
LIBF=$(find unpacked -name 'libonnxruntime.so.*' ! -type l | head -1)
[ -n "$TOP" ] && [ -n "$LIBF" ] || { echo "get-onnxruntime: the archive has no onnxruntime_c_api.h or no libonnxruntime.so.<version>" >&2; exit 1; }
ROOTD=$(dirname "$TOP")
LIBD=$(dirname "$LIBF")
LIBVER=${LIBF##*/libonnxruntime.so.}
echo "get-onnxruntime: headers in ${TOP#unpacked/}, libraries in ${LIBD#unpacked/} (libonnxruntime.so.$LIBVER)"
[ "$LIBVER" = "$VERSION" ] || echo "get-onnxruntime: WARNING: the library's version $LIBVER is not the version $VERSION named by the asset" >&2
[ "$FIRST" = 1 ] || [ "$VERSION" = "$LIBVER" ] || { echo "get-onnxruntime: the pinned VERSION $VERSION is not the archive's $LIBVER" >&2; exit 1; }
rm -rf "$PREFIX"
mkdir -p "$PREFIX/licenses"
cp -a "$TOP" "$PREFIX/include"
mkdir -p "$PREFIX/lib"
cp -a "$LIBD"/libonnxruntime.so* "$PREFIX/lib/"
for f in "$ROOTD"/LICENSE* "$ROOTD"/ThirdPartyNotices* "$ROOTD"/VERSION_NUMBER "$ROOTD"/Privacy.md; do
    [ -f "$f" ] && cp "$f" "$PREFIX/licenses/$(basename "$f")-onnxruntime"
done
ls "$PREFIX/licenses" | grep -q '^LICENSE' || { echo "get-onnxruntime: the archive has no LICENSE file" >&2; exit 1; }
echo "onnxruntime $VERSION linux-x64" > "$PREFIX/version.txt"
echo "ONNX Runtime is in $PREFIX ($VERSION)"
ls "$PREFIX/lib" "$PREFIX/licenses"
