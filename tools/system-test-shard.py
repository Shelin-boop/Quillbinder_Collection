#!/usr/bin/env python3
"""Splits the system test's cases into shards (0.99.0; .github/workflows/system-test.yml).

  qb_crashsafe_tests --list | python3 -I tools/system-test-shard.py --profile full --shards 12 --index 3
  python3 -I tools/system-test-shard.py --list FILE --profile quick --shards 4 --plan

The input is the harness's own list: "<id> <quick s> <full s> <needs> <title>" per line (a "-" in `needs` means none).
Prints the ids of shard <index> (0 to shards-1), one per line, in the order of the list; with --plan one line per
shard: its number of cases and its estimated minutes. The split is deterministic: the longest case goes first to the
shard with the least load (ties: the lower number), so that every shard of a run ends at about the same time.

--cases names only some cases, as tools/run-crashsafe.sh does: an id, or a group such as "S-T" (every id that starts
with it and goes on with a digit). Several names are separated by spaces or commas; empty means every case.

A case that reads what another one left goes into the shard of that one (AFTER below). Every case also counts a fixed
overhead (the display, the verify step, the copies): 40 s.
"""
import argparse
import re
import sys

OVERHEAD = 40
# U-17 reads the facts S-W14 wrote into the run's folder
AFTER = {"U-17": "S-W14"}


def parse(text):
    cases = []
    for line in text.splitlines():
        f = line.split(None, 4)
        if len(f) < 4 or not f[1].isdigit() or not f[2].isdigit():
            continue
        cases.append((f[0], int(f[1]), int(f[2])))
    return cases


def wanted(cid, names):
    if not names:
        return True
    return any(cid == n or (cid.startswith(n) and cid[len(n):len(n) + 1].isdigit()) for n in names)


def estimate(case, profile):
    _, quick, full = case
    if profile == "full":
        return full
    if profile == "smoke":
        return quick // 2 + 30
    return quick


def split(cases, profile, shards):
    """-> a list of `shards` lists of ids (list order inside each)."""
    order = {c[0]: i for i, c in enumerate(cases)}
    groups = {}   # leader id -> [ids]
    leader = {}
    for c in cases:
        cid = c[0]
        after = AFTER.get(cid)
        if after and after in order:
            leader[cid] = leader.get(after, after)
        else:
            leader[cid] = cid
        groups.setdefault(leader[cid], []).append(cid)
    est = {c[0]: estimate(c, profile) + OVERHEAD for c in cases}
    weight = {g: sum(est[i] for i in ids) for g, ids in groups.items()}
    load = [0] * shards
    out = [[] for _ in range(shards)]
    for g in sorted(groups, key=lambda g: (-weight[g], order[g])):
        k = min(range(shards), key=lambda s: (load[s], s))
        load[k] += weight[g]
        out[k].extend(groups[g])
    for ids in out:
        ids.sort(key=lambda i: order[i])
    return out, load


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--list", default="-", help="the list file (default: standard input)")
    ap.add_argument("--profile", default="quick", choices=["quick", "full", "smoke"])
    ap.add_argument("--shards", type=int, default=1)
    ap.add_argument("--index", type=int, default=0)
    ap.add_argument("--cases", default="")
    ap.add_argument("--plan", action="store_true")
    a = ap.parse_args(argv)
    text = sys.stdin.read() if a.list == "-" else open(a.list, encoding="utf-8").read()
    names = [n for n in re.split(r"[\s,]+", a.cases) if n]
    cases = [c for c in parse(text) if wanted(c[0], names)]
    shards = max(1, a.shards)
    if not cases:
        print("no case matches: " + a.cases, file=sys.stderr)
        return 2
    parts, load = split(cases, a.profile, shards)
    if a.plan:
        for i, ids in enumerate(parts):
            print(f"shard {i}: {len(ids)} case(s), about {load[i] // 60} min")
        return 0
    if not 0 <= a.index < shards:
        print("index out of range", file=sys.stderr)
        return 2
    for cid in parts[a.index]:
        print(cid)
    return 0


if __name__ == "__main__":
    sys.exit(main())
