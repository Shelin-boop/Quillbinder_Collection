#!/bin/bash
# Builds the engine libraries of the AI add-ons (0.99.0) from pinned sources, in one prefix, on ONE ggml:
#   ggml (1.x)        the compute library, compute modules as separate loadable libraries
#   whisper.cpp       speech recognition (against that ggml)
#   llama.cpp         language models and, with its libmtmd, speech synthesis with Pocket TTS and Qwen3-TTS (libraries only)
#   qwentts.cpp       speech synthesis with Qwen3-TTS (its own library, against that ggml)
#   Sonic             changing the speed of speech without changing its pitch
#   tools/get-ai-engine.sh [prefix]          (default build/ai-engine, or QB_AI_ENGINE_DIR)
# Shared libraries with RPATH=$ORIGIN (they find each other, and the compute modules, in their own folder). ggml is built
# with GGML_BACKEND_DL: one CPU module per instruction set (the best one for the processor is chosen at start, so the build
# machine's processor does not matter) and the Vulkan module as a library of its own, loaded only when a graphics card is
# wanted: a computer without Vulkan loads the CPU modules and nothing else. No CUDA, no BLAS.
# Needs git, cmake, ninja, gcc/g++, and for Vulkan libvulkan-dev, glslc and spirv-headers (tools/cloud-setup.sh).
# Takes about 40 to 70 minutes on two cores with Vulkan (the shaders take most of it), 15 to 25 without.
# As get-whisper.sh: first the ready-made pack of this recipe (tools/deps-pack.sh, pinned in tools/deps-packs.txt), built
# from source only without one (QB_DEPS_PACK=0: always). QB_JOBS limits the build processes; QB_AI_ENGINE_VULKAN=OFF
# builds without Vulkan (the workflow always builds with it).
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PREFIX=${1:-${QB_AI_ENGINE_DIR:-$ROOT/build/ai-engine}}
WORK=$ROOT/build/ai-engine-src
JOBS=${QB_JOBS:-$(nproc)}

# the pins: a tag and the commit it must point to (for an annotated tag: the commit inside it)
GGML_TAG=v1.1.1
GGML_COMMIT=bb877c0c51f38c52ee2d5e41d58e3d0c20f3fed4
WHISPER_TAG=v1.9.6
WHISPER_COMMIT=96ec4d9823e9d0211944172da35f9f29134af990
LLAMA_TAG=b12808
LLAMA_COMMIT=ed2153e708de813c35559102aed43b3085179c27
# qwentts.cpp and Sonic have no usable release tags: a commit
QWENTTS_COMMIT=2edb35f87b709a070b2d5379f77e5c52a1daa585
SONIC_COMMIT=b93885dcb70aae50c6f76b0fe4e0868f029a077e
STAMP="ggml $GGML_TAG $GGML_COMMIT; whisper.cpp $WHISPER_TAG $WHISPER_COMMIT; llama.cpp $LLAMA_TAG $LLAMA_COMMIT; qwentts.cpp $QWENTTS_COMMIT; sonic $SONIC_COMMIT"

VULKAN=${QB_AI_ENGINE_VULKAN:-ON}
if [ "$VULKAN" = ON ] && { ! command -v glslc >/dev/null || ! [ -f /usr/include/vulkan/vulkan.h ] || ! [ -d /usr/share/cmake/SPIRV-Headers ]; }; then
    echo "glslc, the Vulkan or the SPIR-V headers are missing (apt-get install libvulkan-dev glslc spirv-headers): building without graphics cards" >&2
    VULKAN=OFF
fi

done_here() { [ "$(cat "$PREFIX/version.txt" 2>/dev/null)" = "$STAMP" ] && [ -f "$PREFIX/lib/libggml.so" ] && [ -f "$PREFIX/lib/libsonic.so" ]; }
if done_here; then
    echo "the AI engine libraries are there: $PREFIX (ggml $GGML_TAG, Vulkan $(cat "$PREFIX/vulkan.txt" 2>/dev/null))"
    exit 0
fi
if "$ROOT/tools/deps-pack.sh" fetch aiengine "$PREFIX" && done_here; then
    echo "the AI engine libraries are there: $PREFIX (ggml $GGML_TAG, Vulkan $(cat "$PREFIX/vulkan.txt" 2>/dev/null))"
    exit 0
fi

clone() {  # <dir> <url> <tag> <commit>
    if [ "$(git -C "$WORK/$1" rev-parse HEAD 2>/dev/null)" != "$4" ]; then
        rm -rf "${WORK:?}/$1"
        git clone -q --depth 1 --branch "$3" "$2" "$WORK/$1"
    fi
    test "$(git -C "$WORK/$1" rev-parse 'HEAD^{commit}')" = "$4" || { echo "get-ai-engine: the tag $3 of $2 is not the commit $4" >&2; exit 1; }
}
fetch_commit() {  # <dir> <url> <commit>
    if [ "$(git -C "$WORK/$1" rev-parse HEAD 2>/dev/null)" != "$3" ]; then
        rm -rf "${WORK:?}/$1"
        git init -q "$WORK/$1"
        git -C "$WORK/$1" fetch -q --depth 1 "$2" "$3"
        git -C "$WORK/$1" checkout -q FETCH_HEAD
    fi
    test "$(git -C "$WORK/$1" rev-parse HEAD)" = "$3" || { echo "get-ai-engine: $2 does not give the commit $3" >&2; exit 1; }
}
mkdir -p "$WORK"
clone ggml https://github.com/ggml-org/ggml "$GGML_TAG" "$GGML_COMMIT"
clone whisper.cpp https://github.com/ggml-org/whisper.cpp "$WHISPER_TAG" "$WHISPER_COMMIT"
clone llama.cpp https://github.com/ggml-org/llama.cpp "$LLAMA_TAG" "$LLAMA_COMMIT"
fetch_commit qwentts.cpp https://github.com/ServeurpersoCom/qwentts.cpp "$QWENTTS_COMMIT"
fetch_commit sonic https://github.com/waywardgeek/sonic "$SONIC_COMMIT"

# QB_AI_ENGINE_RESUME=1 (for working on this script): keep the finished parts of an earlier, unfinished run (markers .done-<part>)
if [ "${QB_AI_ENGINE_RESUME:-0}" != 1 ]; then
    rm -rf "$PREFIX"
fi
part_done() { [ "${QB_AI_ENGINE_RESUME:-0}" = 1 ] && [ -f "$PREFIX/.done-$1" ]; }
rm -rf "$WORK"/b-*
mkdir -p "$PREFIX/lib" "$PREFIX/include" "$PREFIX/licenses"
COMMON=(-G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_PREFIX_PATH="$PREFIX" -DBUILD_SHARED_LIBS=ON
        -DCMAKE_INSTALL_RPATH='$ORIGIN' -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON)

if ! part_done ggml; then
# ggml. One change to its header: the length of a tensor's name is 128 (qwentts.cpp's audio tokenizer has longer names than
# the default 64). The size is part of the tensors' layout, so every part that includes ggml.h must see the same number:
# it is changed in the header itself, which this build installs and every part below includes.
sed -i 's/^#\( *\)define GGML_MAX_NAME        64$/#\1define GGML_MAX_NAME        128/' "$WORK/ggml/include/ggml.h"
grep -q '^# *define GGML_MAX_NAME        128$' "$WORK/ggml/include/ggml.h" || { echo "get-ai-engine: ggml.h has no GGML_MAX_NAME line to change" >&2; exit 1; }
cmake -S "$WORK/ggml" -B "$WORK/b-ggml" "${COMMON[@]}" -DGGML_BACKEND_DL=ON -DGGML_CPU_ALL_VARIANTS=ON -DGGML_NATIVE=OFF \
    -DGGML_VULKAN="$VULKAN" -DGGML_OPENMP=OFF -DGGML_CCACHE=OFF -DGGML_CUDA=OFF -DGGML_BLAS=OFF \
    -DGGML_BUILD_TESTS=OFF -DGGML_BUILD_EXAMPLES=OFF >/dev/null
cmake --build "$WORK/b-ggml" -j"$JOBS"
cmake --install "$WORK/b-ggml" >/dev/null
# the compute modules (ggml installs them into bin/) sit next to the libraries; ggml finds them by the folder it is given
for f in "$PREFIX"/bin/libggml-*.so; do
    [ -f "$f" ] && mv -f "$f" "$PREFIX/lib/"
done
rmdir "$PREFIX/bin" 2>/dev/null || true
git -C "$WORK/ggml" checkout -q -- include/ggml.h   # (the source stays as it was tagged)
touch "$PREFIX/.done-ggml"
fi

# whisper.cpp, on that ggml
if ! part_done whisper; then
cmake -S "$WORK/whisper.cpp" -B "$WORK/b-whisper" "${COMMON[@]}" -DWHISPER_USE_SYSTEM_GGML=ON \
    -DWHISPER_BUILD_EXAMPLES=OFF -DWHISPER_BUILD_TESTS=OFF -DWHISPER_BUILD_SERVER=OFF -DWHISPER_SDL2=OFF >/dev/null
cmake --build "$WORK/b-whisper" -j"$JOBS"
cmake --install "$WORK/b-whisper" >/dev/null
touch "$PREFIX/.done-whisper"
fi

if ! part_done llama; then
# llama.cpp, on that ggml: the libraries only (libllama and, for the speech models, libmtmd); no tools, examples, server,
# tests, RPC or web access (no OpenSSL, no curl)
cmake -S "$WORK/llama.cpp" -B "$WORK/b-llama" "${COMMON[@]}" -DLLAMA_USE_SYSTEM_GGML=ON -DLLAMA_BUILD_MTMD=ON \
    -DLLAMA_BUILD_COMMON=OFF -DLLAMA_BUILD_TOOLS=OFF -DLLAMA_BUILD_EXAMPLES=OFF -DLLAMA_BUILD_SERVER=OFF \
    -DLLAMA_BUILD_TESTS=OFF -DLLAMA_OPENSSL=OFF -DLLAMA_BUILD_APP=OFF >/dev/null
cmake --build "$WORK/b-llama" -j"$JOBS"
cmake --install "$WORK/b-llama" >/dev/null
# libmtmd's header is not installed by llama.cpp's own install rules of this build: copy it if missing
for h in mtmd.h mtmd-helper.h; do
    [ -f "$PREFIX/include/$h" ] || cp "$WORK/llama.cpp/tools/mtmd/$h" "$PREFIX/include/"
done
touch "$PREFIX/.done-llama"
fi

# qwentts.cpp: its own build file (tools/ai-engine/qwentts/CMakeLists.txt) instead of its CMakeLists.txt
if ! part_done qwentts; then
cmake -S "$ROOT/tools/ai-engine/qwentts" -B "$WORK/b-qwentts" "${COMMON[@]}" -DQWENTTS_SRC="$WORK/qwentts.cpp" \
    -DQWENTTS_COMMIT="${QWENTTS_COMMIT:0:7}" -DQWENTTS_DATE=pinned >/dev/null
cmake --build "$WORK/b-qwentts" -j"$JOBS"
cmake --install "$WORK/b-qwentts" >/dev/null
touch "$PREFIX/.done-qwentts"
fi

# Sonic: one source file, a small shared library
(cd "$WORK/sonic" && gcc -O2 -fPIC -shared -Wl,-soname,libsonic.so.0 -Wl,-rpath,'$ORIGIN' -o "$PREFIX/lib/libsonic.so.0.3.0" sonic.c -lm)
ln -sf libsonic.so.0.3.0 "$PREFIX/lib/libsonic.so.0"
ln -sf libsonic.so.0 "$PREFIX/lib/libsonic.so"
cp "$WORK/sonic/sonic.h" "$PREFIX/include/"

# the license texts of every part (the pack carries them)
cp "$WORK/ggml/LICENSE" "$PREFIX/licenses/LICENSE-ggml.txt"
cp "$WORK/whisper.cpp/LICENSE" "$PREFIX/licenses/LICENSE-whisper.cpp.txt"
cp "$WORK/llama.cpp/LICENSE" "$PREFIX/licenses/LICENSE-llama.cpp.txt"
cp "$WORK/qwentts.cpp/LICENSE" "$PREFIX/licenses/LICENSE-qwentts.cpp.txt"
cp "$WORK/sonic/LICENSE" "$PREFIX/licenses/LICENSE-sonic.txt"
rm -f "$PREFIX"/.done-*
echo "$VULKAN" > "$PREFIX/vulkan.txt"
echo "$STAMP" > "$PREFIX/version.txt"
echo "the AI engine libraries are built: $PREFIX (ggml $GGML_TAG, Vulkan $VULKAN)"
ls -la "$PREFIX/lib"
