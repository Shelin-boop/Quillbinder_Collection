#!/bin/bash
# The engine libraries of the add-on "Translation and Proofreading": oneDNN, CTranslate2 and SentencePiece (static),
# built from their sources at fixed versions by the add-on's own script into build/translator-deps (or
# QB_TRANSLATOR_DEPS), about 40 minutes on 2 processor cores. Nothing is built when they are there already.
#   tools/get-translator-deps.sh
# The release workflow keeps the folder between runs (its name carries the hash of the recipe, see --recipe).
#   tools/get-translator-deps.sh --recipe     prints that hash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
RECIPE="$ROOT/addons/translation-proofreading/source/packaging/build-deps.sh"
if [ "${1:-}" = "--recipe" ]; then
    sha256sum "$RECIPE" | cut -c1-16
    exit 0
fi
D=${QB_TRANSLATOR_DEPS:-$ROOT/build/translator-deps}
if [ -f "$D/install/lib/libctranslate2.a" ] || [ -f "$D/install/lib64/libctranslate2.a" ]; then
    echo "the translator's engine libraries are in $D"
    exit 0
fi
mkdir -p "$D"
TP_DEPS="$D" bash "$RECIPE"
# the build folders are not needed again (the sources stay: their license texts go into the add-on)
rm -rf "$D/b-onednn" "$D/b-ct2" "$D/b-spm"
