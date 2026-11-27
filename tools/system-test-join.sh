#!/bin/bash
# Joins the shards' summaries of one run of the Linux system test (0.99.0):
#   tools/system-test-join.sh <folder with the <n>-st-summary.txt files> <run id> [planned shards]
# Writes <folder>/<run id>-st-summary.txt: the totals first, then every shard's lines in shard order. A shard that left no
# summary is named as missing.
set -u
DIR=$1
RUN=$2
PLANNED=${3:-0}
OUT=$DIR/$RUN-st-summary.txt
shopt -s nullglob
files=()
for f in "$DIR"/[0-9]*-st-summary.txt; do
    # a shard's number is below the number of shards (a joined file of a run is named with the run's long id)
    n=$(basename "$f" -st-summary.txt)
    if [ "$PLANNED" -gt 0 ]; then [ "$n" -lt "$PLANNED" ] && files+=("$f"); else [ "${#n}" -le 3 ] && files+=("$f"); fi
done
[ ${#files[@]} -gt 0 ] && mapfile -t files < <(printf '%s\n' "${files[@]}" | sort -V)
cases=0 bad=0 failures=0 missing=""
for ((i = 0; i < PLANNED; i++)); do
    [ -f "$DIR/$i-st-summary.txt" ] || missing="$missing $i"
done
for f in "${files[@]}"; do
    c=$(grep -c ' rc=' "$f")
    b=$(grep ' rc=' "$f" | grep -vc ' fail=0 ')
    t=$(grep ' rc=' "$f" | sed -n 's/.* fail=\([0-9]*\) .*/\1/p' | awk '{s += $1} END {print s + 0}')
    cases=$((cases + c))
    bad=$((bad + b))
    failures=$((failures + ${t:-0}))
done
{
    echo "result: run $RUN: ${#files[@]} of ${PLANNED:-?} shard(s) reported, $cases case(s) ended, $bad with failures ($failures failure(s))"
    [ -n "$missing" ] && echo "missing shards:$missing"
    for f in "${files[@]}"; do
        echo
        echo "== shard $(basename "$f" -st-summary.txt)"
        cat "$f"
    done
} > "$OUT"
cat "$OUT" | head -3
