#!/bin/bash
# Puts the libraries of the package "AI Engine" (0.99.0) into a staged copy of it.
#   stage.sh <build dir> <stage dir>      (called by packaging/linux/build-addons.sh)
# Needs the pack of tools/get-ai-engine.sh (build/ai-engine, or QB_AI_ENGINE_DIR) and libqbai, built with the program when the
# pack was there at configure time (<build dir>/qbai/libqbai.so.1, engines/qbai). Without them: exit code 3 (left out).
# The package: payload/engine/lib (libqbai and the libraries it needs, every one with RUNPATH $ORIGIN; ggml's compute modules
# beside them) and payload/engine/licenses. tools/check-engine-package.py checks the finished package.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=$1
STAGE=$2
E=${QB_AI_ENGINE_DIR:-$ROOT/build/ai-engine}
QBAI=$BUILD/qbai/libqbai.so.1
if [ ! -f "$E/lib/libggml.so.1" ] || [ ! -f "$E/version.txt" ]; then
    echo "ai-engine: the engine's libraries are missing (tools/get-ai-engine.sh, then configure and build again)" >&2
    exit 3
fi
if [ ! -f "$QBAI" ]; then
    echo "ai-engine: libqbai is not built in $BUILD (configure again after tools/get-ai-engine.sh, then build)" >&2
    exit 3
fi
L=$STAGE/payload/engine/lib
mkdir -p "$L" "$STAGE/payload/engine/licenses"
cp -L "$QBAI" "$L/"
for l in libggml.so.1 libggml-base.so.1 libwhisper.so.1 libllama.so.1 libmtmd.so.1 libqwentts.so.0 libsonic.so.0; do
    cp -L "$E/lib/$l" "$L/"
done
cp "$E"/lib/libggml-cpu-*.so "$L/"
VULKAN="without the Vulkan module (built without graphics cards)"
if [ -f "$E/lib/libggml-vulkan.so" ]; then
    cp "$E/lib/libggml-vulkan.so" "$L/"
    VULKAN="with the Vulkan module libggml-vulkan.so (it needs the system's libvulkan.so.1, and is loaded only where that is there)"
fi
strip --strip-unneeded "$L"/*.so* 2>/dev/null || true
LIC=$STAGE/payload/engine/licenses
for f in "$E"/licenses/LICENSE-*.txt; do
    cp "$f" "$LIC/"
done
PARTS=$(cat "$E/version.txt")
VERSION=$(python3 -I -c "import json,sys; print(json.load(open(sys.argv[1]))['version'])" "$STAGE/manifest.json")
HOME_URL=$(sed -n 's/^set(APP_HOMEPAGE *"\(.*\)")/\1/p' "$ROOT/CMakeLists.txt")
cat > "$LIC/README.txt" <<TEXT
Libraries in the package "AI Engine" $VERSION
=============================================

Built from their sources at fixed versions by tools/get-ai-engine.sh of the Quillbinder source
($HOME_URL), on one ggml:
    $PARTS
This package is built $VULKAN.

engine/lib/libqbai.so.1
    Quillbinder's wrapper (engines/qbai of the Quillbinder source, Apache License 2.0): the one
    library the program opens; the others are separate shared libraries under their own licences,
    each replaceable by another build of the same version (same file name).

engine/lib/libggml.so.1, libggml-base.so.1, libggml-cpu-*.so, libggml-vulkan.so
    ggml (github.com/ggml-org/ggml): the computations. Licence: MIT (LICENSE-ggml.txt). Its
    header ggml.h differs from the tag in one line (the length of a tensor's name, 128).
    The CPU modules hold llamafile's sgemm (Mozilla Foundation, MIT: llamafile-sgemm-MIT.txt).

engine/lib/libwhisper.so.1
    whisper.cpp (github.com/ggml-org/whisper.cpp): speech recognition. Licence: MIT
    (LICENSE-whisper.cpp.txt).

engine/lib/libllama.so.1, libmtmd.so.1
    llama.cpp (github.com/ggml-org/llama.cpp), its libraries only: Pocket TTS. Licence: MIT
    (LICENSE-llama.cpp.txt). libmtmd holds stb_image (MIT or public domain: stb_image.txt),
    miniaudio (public domain or MIT No Attribution: miniaudio.txt), subprocess.h (Unlicense:
    subprocess-Unlicense.txt), xxHash (BSD 2-Clause: xxhash-BSD-2-Clause.txt) and SHA-1 and
    SHA-256 code in the public domain (sha1-sha256-public-domain.txt).

engine/lib/libqwentts.so.0
    qwentts.cpp (github.com/ServeurpersoCom/qwentts.cpp): Qwen3-TTS. Licence: MIT
    (LICENSE-qwentts.cpp.txt).

engine/lib/libsonic.so.0
    Sonic (github.com/waywardgeek/sonic): the speed of speech, the pitch kept. Licence: Apache
    License 2.0 (LICENSE-sonic.txt).

The sources: the repositories named above, at the tags and commits of the line at the top.

The models are not part of the package: the settings pages of the functions download them.
TEXT
