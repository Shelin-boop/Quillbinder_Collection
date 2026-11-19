#!/bin/bash
# Puts files on the branch `assets` of Shelin-boop/Quillbinder_Collection without downloading that branch's
# files: fetches only its newest commit and file list (--filter=blob:none, a few
# hundred KiB), adds or replaces the given files in that list, and pushes one new commit.
# (Until 2026-10-29 this used GitHub's contents API; the session's proxy refuses writes there.)
#   tools/put-asset.sh dist/quillbinder-<version>-update.qbpkg dist/quillbinder-<version>-symbols.xz
#   QB_DRY_RUN=1 tools/put-asset.sh ...   builds the commit and shows what changes, pushes nothing
# Files up to about 50 MiB (GitHub warns above 50 MiB and refuses above 100 MiB).
set -euo pipefail
URL=${QB_ASSETS_URL:-https://github.com/Shelin-boop/Quillbinder_Collection}
[ $# -gt 0 ] || { echo "usage: $0 <file> ..."; exit 1; }
files=()
for f in "$@"; do
    [ -f "$f" ] || { echo "no file: $f"; exit 1; }
    files+=("$(cd "$(dirname "$f")" && pwd)/$(basename "$f")")
done
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"
git init -q
git remote add origin "$URL"
git fetch -q --depth 1 --filter=blob:none origin assets
git read-tree FETCH_HEAD
names=()
for f in "${files[@]}"; do
    name=$(basename "$f")
    names+=("$name")
    git update-index --add --cacheinfo "100644,$(git hash-object -w "$f"),$name"
done
tree=$(git write-tree)
# only the given files may change; everything else on the branch stays as it is
git ls-tree FETCH_HEAD > old.txt
git ls-tree "$tree" > new.txt
# (diff ends with 1 when the lists differ, which they always do here: not an error under pipefail)
changed=$({ diff old.txt new.txt || true; } | sed -n 's/^[<>] [0-9]* blob [0-9a-f]*\t//p' | sort -u)
for c in $changed; do
    printf '%s\n' "${names[@]}" | grep -qxF "$c" || { echo "unexpected change: $c"; exit 1; }
done
commit=$(git -c user.name="${GIT_AUTHOR_NAME:-Claude}" -c user.email="${GIT_AUTHOR_EMAIL:-noreply@anthropic.com}" \
    commit-tree "$tree" -p FETCH_HEAD -m "assets: ${names[*]}")
for n in "${names[@]}"; do
    if grep -qP "\t\Q$n\E$" old.txt; then echo "$n: replaced"; else echo "$n: new file"; fi
done
if [ "${QB_DRY_RUN:-0}" = 1 ]; then
    echo "dry run: commit $commit not pushed"
    exit 0
fi
git push -q origin "$commit:refs/heads/assets"
echo "pushed $commit to assets"
