#!/usr/bin/env bash
# A test release (2026-11-12): the files of a test build on one release page (a pre-release marked as not published),
# sorted by system (tools/release-notes.py --test), so that the writer finds them where the published versions are
# found too: Releases. Run by .github/workflows/test-release.yml on a push to the branch "test-release"; the session
# cannot make releases itself. The list file (test-release.txt at the top of that branch) says what goes in:
#
#   tag: test-0.81.1-w4b
#   title: Test build 0.81.1 with Windows W4b (not published)
#   note: One line of text above the files (may repeat).
#   assets: <file on the branch assets>
#   win-ci: <file of the pre-release win-ci> [> <name in the test release>]
#   android-ci: <file of the pre-release android-ci> [> <name in the test release>]   (Android A0, 2026-11-14)
#
# A test release of the same tag is made anew.
set -euo pipefail
LIST=${1:-test-release.txt}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DIST=$(mktemp -d)
NOTES=$(mktemp)
tag="" title="" note=""
while IFS= read -r line || [ -n "$line" ]; do
    line=${line%$'\r'}
    case "$line" in
        ""|"#"*) continue ;;
        tag:*) tag=$(echo "${line#tag:}" | xargs) ;;
        title:*) title=$(echo "${line#title:}" | sed 's/^ *//') ;;
        note:*) note="$note$(echo "${line#note:}" | sed 's/^ *//') " ;;
        assets:*|win-ci:*|android-ci:*)
            src=${line%%:*}
            spec=$(echo "${line#*:}" | sed 's/^ *//')
            name=$(echo "${spec%%>*}" | xargs)
            as=$name
            case "$spec" in *">"*) as=$(echo "${spec#*>}" | xargs) ;; esac
            if [ "$src" = assets ]; then
                gh api "repos/$GH_REPO/contents/$name?ref=assets" -H 'Accept: application/vnd.github.raw' > "$DIST/$as"
            else
                id=$(gh api "repos/$GH_REPO/releases/tags/$src" --jq ".assets[] | select(.name==\"$name\") | .id")
                [ -n "$id" ] || { echo "not on $src: $name" >&2; exit 1; }
                gh api "repos/$GH_REPO/releases/assets/$id" -H 'Accept: application/octet-stream' > "$DIST/$as"
            fi
            [ -s "$DIST/$as" ] || { echo "empty or missing: $name" >&2; exit 1; }
            echo "file: $as ($(stat -c %s "$DIST/$as") bytes)"
            ;;
        *) echo "unknown line: $line" >&2; exit 1 ;;
    esac
done < "$LIST"
[ -n "$tag" ] && [ -n "$title" ] || { echo "the list needs tag: and title:" >&2; exit 1; }
case "$tag" in test-*) ;; *) echo "a test release's tag starts with test-" >&2; exit 1 ;; esac
( cd "$DIST" && sha256sum -- * > SHA256SUMS.txt )
{
    echo "> **A test build, not published.** For trying it on your own computers before a version is published. ${note}"
    echo ""
    python3 -I "$ROOT/tools/release-notes.py" "$DIST" --test
} > "$NOTES"
gh release delete "$tag" --cleanup-tag -y > /dev/null 2>&1 || true
gh release create "$tag" --prerelease --target "${GITHUB_SHA:-main}" --title "$title" --notes-file "$NOTES" "$DIST"/*
echo "test release $tag: $(ls "$DIST" | wc -l) files"
