#!/bin/bash
# 0.92.0: carries the update file along when the project's page is published anew.
#
#   tools/keep-update-feed.sh <site folder> [<folder with a new updates-1.json and updates-1.json.sig>]
#
# The page (GitHub Pages) is replaced as a whole by every workflow that publishes it, and the program's check for
# updates reads updates-1.json there (packaging/update-feed/README.md). With a second folder that holds a new file
# and its signature, those are put into the site; else the two files published so far are fetched and kept. Either
# way they are taken only when the signature is the release key's (packaging/quillbinder-signing-public.pem); a
# site without the file is no error (the program then finds no update information and stays quiet).
set -u
SITE="$1"
NEW="${2:-}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
KEY="$ROOT/packaging/quillbinder-signing-public.pem"
PAGE="https://shelin-boop.github.io/Quillbinder_Collection"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

if [ -n "$NEW" ] && [ -f "$NEW/updates-1.json" ] && [ -f "$NEW/updates-1.json.sig" ]; then
    cp "$NEW/updates-1.json" "$NEW/updates-1.json.sig" "$TMP/"
    from="this release"
elif curl -fsSL --max-time 30 -o "$TMP/updates-1.json" "$PAGE/updates-1.json" \
     && curl -fsSL --max-time 30 -o "$TMP/updates-1.json.sig" "$PAGE/updates-1.json.sig"; then
    from="the page as it was"
else
    echo "update file: none to carry along"
    exit 0
fi
if openssl pkeyutl -verify -pubin -inkey "$KEY" -rawin -in "$TMP/updates-1.json" -sigfile "$TMP/updates-1.json.sig" >/dev/null 2>&1; then
    cp "$TMP/updates-1.json" "$TMP/updates-1.json.sig" "$SITE/"
    echo "update file: from $from, signature good"
else
    echo "update file: from $from, but its signature is not the release key's: left out" >&2
    [ -n "$NEW" ] && exit 1
fi
exit 0
