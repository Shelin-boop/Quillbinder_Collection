#!/bin/bash
# Puts ONNX Runtime into a staged copy of the package "Handwriting Engine" (0.99.0).
#   stage.sh <build dir> <stage dir>      (called by packaging/linux/build-addons.sh)
# Needs the pack of tools/get-onnxruntime.sh (build/onnxruntime, or QB_ONNXRUNTIME_DIR: lib/libonnxruntime.so.<version>,
# version.txt, licenses/, README.txt); without it: exit code 3 (left out).
# The program opens engine/lib/libonnxruntime.so.1 itself (dlopen, OrtGetApiBase): no wrapper of ours is needed.
# ONNX Runtime's own MIT text is in this recipe (payload/engine/licenses/LICENSE-onnxruntime.txt): the Maven Central package
# the pack takes the library from carries only Microsoft's third-party notices; those come from the pack's licenses/.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
STAGE=$2
O=${QB_ONNXRUNTIME_DIR:-$ROOT/build/onnxruntime}
LIB=$(ls "$O"/lib/libonnxruntime.so.1.* 2>/dev/null | grep -v '\.debug$' | head -1 || true)
if [ -z "$LIB" ] || [ ! -f "$O/version.txt" ]; then
    echo "handwriting-engine: ONNX Runtime is missing (tools/get-onnxruntime.sh, then build again)" >&2
    exit 3
fi
L=$STAGE/payload/engine/lib
LIC=$STAGE/payload/engine/licenses
mkdir -p "$L" "$LIC"
# under its SONAME, the name the program opens
cp -L "$LIB" "$L/libonnxruntime.so.1"
strip --strip-unneeded "$L/libonnxruntime.so.1" 2>/dev/null || true
for f in "$O"/licenses/*; do
    [ -f "$f" ] && cp "$f" "$LIC/"
done
[ -f "$O/README.txt" ] && cp "$O/README.txt" "$LIC/SOURCE-onnxruntime.txt"
PARTS=$(cat "$O/version.txt")
VERSION=$(python3 -I -c "import json,sys; print(json.load(open(sys.argv[1]))['version'])" "$STAGE/manifest.json")
cat > "$LIC/README.txt" <<TEXT
Libraries in the package "Handwriting Engine" $VERSION
====================================================

engine/lib/libonnxruntime.so.1
    ONNX Runtime ($PARTS) by Microsoft (github.com/microsoft/onnxruntime), Microsoft's own build
    for Linux x64, unchanged: the native library of the Maven Central package
    com.microsoft.onnxruntime:onnxruntime (SOURCE-onnxruntime.txt names the file and its SHA-256),
    fetched by tools/get-onnxruntime.sh of the Quillbinder source.
    Licence: MIT (LICENSE-onnxruntime.txt); the notices of the parts it holds are in the other
    files of this folder that come with it (ThirdPartyNotices).

The models are not part of the package: the settings page of Handwriting downloads them.
TEXT
