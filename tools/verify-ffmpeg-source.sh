#!/bin/bash
# Checks FFmpeg's release signature (0.93.0): a source archive is trusted when the signature file made for it is good
# and was made by the key with the expected fingerprint (FFmpeg's release signing key, from ffmpeg-devel.asc).
#   tools/verify-ffmpeg-source.sh <archive> <archive.asc> <ffmpeg-devel.asc> <fingerprint>
# Prints the SHA-256 of the archive and the line "pin this" (tools/get-ffmpeg.sh keeps it as its pin). Exit 0: good;
# 1: not good (the reason is printed, and the fingerprints that the key file does hold when the expected one is not
# among them). Needs gpg and gpgv; uses a folder of its own for the keys, never the user's keyring.
set -euo pipefail
[ $# = 4 ] || { echo "usage: $0 <archive> <archive.asc> <ffmpeg-devel.asc> <fingerprint>" >&2; exit 2; }
FILE=$1; SIG=$2; KEYS=$3; WANT=$(echo "$4" | tr -d ' ' | tr a-f A-F)
command -v gpg >/dev/null && command -v gpgv >/dev/null || { echo "verify-ffmpeg-source: gpg and gpgv are needed (apt-get install gnupg gpgv)" >&2; exit 1; }
H=$(mktemp -d)
trap 'rm -rf "$H"' EXIT
chmod 700 "$H"
if ! gpg --homedir "$H" --batch --quiet --import "$KEYS" 2>"$H/import.err"; then
    echo "verify-ffmpeg-source: the key file $KEYS could not be read:" >&2; cat "$H/import.err" >&2; exit 1
fi
FOUND=$(gpg --homedir "$H" --batch --with-colons --fingerprint 2>/dev/null | awk -F: '$1=="fpr" {print $10}')
if ! echo "$FOUND" | grep -qx "$WANT"; then
    echo "verify-ffmpeg-source: STOP. The key file does not hold the key with the fingerprint $WANT." >&2
    echo "Fingerprints found in it:" >&2
    echo "$FOUND" | sed 's/^/  /' >&2
    exit 1
fi
gpg --homedir "$H" --batch --export "$WANT" > "$H/trusted.gpg" 2>/dev/null
STATUS=$(gpgv --homedir "$H" --keyring "$H/trusted.gpg" --status-fd 1 "$SIG" "$FILE" 2>"$H/gpgv.err" || true)
# VALIDSIG <key> ... <primary key fingerprint>: a good signature of the expected key, as primary key or subkey
if ! echo "$STATUS" | grep -q '^\[GNUPG:\] GOODSIG' || ! echo "$STATUS" | awk '$2=="VALIDSIG" {print $3; print $NF}' | grep -qx "$WANT"; then
    echo "verify-ffmpeg-source: STOP. The signature $SIG is not a good signature of $FILE by the key $WANT:" >&2
    cat "$H/gpgv.err" >&2
    exit 1
fi
SUM=$(sha256sum "$FILE" | cut -d' ' -f1)
echo "verify-ffmpeg-source: good signature of $(basename "$FILE") by $WANT"
echo "SHA-256 of $(basename "$FILE"): $SUM"
echo "pin this: SHA256=\"$SUM\""
