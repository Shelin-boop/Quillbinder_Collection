#!/bin/bash
# What one shard of the Linux system test leaves for the pre-release "linux-st" (0.99.0):
#   tools/system-test-collect.sh <results folder of tools/run-crashsafe.sh> <output folder> <shard number>
# Writes into <output folder>: <n>-st-summary.txt (the shard's line per case; "result: ..." first) and <n>-st-logs.tar.gz
# (every case's case.log, verify.log, result.line, facts.json, crash reports; pictures only of cases with a failure).
set -u
RES=$1
OUT=$2
N=$3
mkdir -p "$OUT"
S=$OUT/$N-st-summary.txt
{
    if [ -f "$RES/summary.txt" ]; then
        done=$(grep -c ' rc=' "$RES/summary.txt")
        bad=$(grep ' rc=' "$RES/summary.txt" | grep -vc ' fail=0 ')
        echo "result: shard $N: $done case(s) ended, $bad with failures"
        grep -v '^$' "$RES/summary.txt"
    else
        echo "result: shard $N: no case ended (see its logs)"
    fi
} > "$S"
LIST=$(mktemp)
for d in "$RES"/*/; do
    id=$(basename "$d")
    [ -f "$d/result.line" ] || continue
    for f in case.log verify.log result.line facts.json; do [ -f "$d/$f" ] && echo "$id/$f"; done
    [ -d "$d/reports" ] && find "$d/reports" -type f | sed "s|^$RES/||"
    if ! head -1 "$d/result.line" | grep -q ' fail=0 '; then
        find "$d" -maxdepth 1 -name '*.png' | sed "s|^$RES/||"
    fi
done > "$LIST"
for f in fixtures.log addons.log; do [ -f "$RES/$f" ] && echo "$f" >> "$LIST"; done
tar -C "$RES" -czf "$OUT/$N-st-logs.tar.gz" -T "$LIST" 2>/dev/null
rm -f "$LIST"
