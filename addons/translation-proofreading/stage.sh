#!/bin/bash
# Builds and stages the add-on "Translation and Proofreading" (one of the project's own add-ons since 0.92.0).
#   stage.sh <build dir> <stage dir>      (called by packaging/linux/build-addons.sh)
# The add-on is a project of its own in source/ (its CMakeLists.txt, tests and notes: source/DEVELOPING.md). It needs
# its engine libraries, oneDNN, CTranslate2 and SentencePiece, built by tools/get-translator-deps.sh into
# <repository>/build/translator-deps (or QB_TRANSLATOR_DEPS), and the Qt the AppImage carries (QB_QT_DIR).
# Without the engine libraries it is left out (exit code 3), like "Speech Transcription" without whisper.cpp.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
BUILD=$(cd "$1" && pwd)
STAGE=$2
DEPS=${QB_TRANSLATOR_DEPS:-$ROOT/build/translator-deps}
QT_VERSION=$(sed -n 's/^set(QB_QT_VERSION *"\(.*\)")/\1/p' "$ROOT/CMakeLists.txt")
QT=${QB_QT_DIR:-/opt/qt/$QT_VERSION/gcc_64}
if [ ! -f "$DEPS/install/lib/libctranslate2.a" ] && [ ! -f "$DEPS/install/lib64/libctranslate2.a" ]; then
    echo "translation-proofreading: its engine libraries are missing (tools/get-translator-deps.sh, about 40 minutes)" >&2
    exit 3
fi
cmp -s "$HERE/manifest.json" "$HERE/source/packaging/manifest.json" \
    || { echo "translation-proofreading: manifest.json and source/packaging/manifest.json differ" >&2; exit 1; }
B=$BUILD/translator
cmake -S "$HERE/source" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QT" \
    -DTP_DEPS_DIR="$DEPS/install" -DTP_TEST_SWITCHES=OFF > "$B.configure.log" 2>&1 \
    || { tail -30 "$B.configure.log" >&2; exit 1; }
cmake --build "$B" > "$B.build.log" 2>&1 || { tail -40 "$B.build.log" >&2; exit 1; }
"$HERE/source/tests/check-exports.sh" "$B"
# staged as the add-on's own script does it (payload, libraries, license texts)
TP_DEPS="$DEPS" QB_QT_DIR="$QT" TP_STAGE_DIR="$STAGE" "$HERE/source/packaging/make-addon.sh" "$B" "$STAGE"
