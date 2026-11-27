#!/usr/bin/env python3
"""Suite stshard (0.99.0): the Linux system test on GitHub in shards and the harness made ready for Windows, offline.
Checks tools/system-test-shard.py (the split), tools/system-test-collect.sh and tools/system-test-join.sh (what a shard
leaves, the joined summary), .github/workflows/system-test.yml (triggers, no secret, pinned actions, the run file) and the
rule of the harness' sources: a POSIX header or call outside a branch that Windows does not compile is an error.

  python3 -I tests/system_test_tests.py
"""
import importlib.util
import io
import os
import re
import subprocess
import sys
import tarfile
import tempfile
from contextlib import redirect_stdout

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("stshard_tool", os.path.join(ROOT, "tools", "system-test-shard.py"))
tool = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tool)
FAILS = 0


def check(name, ok, detail=""):
    global FAILS
    print(("PASS " if ok else "FAIL ") + name + ("" if ok else f": {detail}"))
    if not ok:
        FAILS += 1


def read(rel):
    with open(os.path.join(ROOT, rel), encoding="utf-8") as f:
        return f.read()


def run_tool(args, text=""):
    out = io.StringIO()
    err = io.StringIO()
    old = sys.stdin
    sys.stdin = io.StringIO(text)
    try:
        from contextlib import redirect_stderr
        with redirect_stdout(out), redirect_stderr(err):
            rc = tool.main(args)
    finally:
        sys.stdin = old
    return rc, out.getvalue().split(), out.getvalue()


# ------------------------------------------------------------------------------------------------ the split
LIST = """C-01 60 60 - Project round trip
C-02 60 90 - Scrivener write
C-10 210 210 L Compile engine
S-T01 30 30 - First
S-T02 30 30 - Second
S-T10 30 30 - Tenth
S-TX1 30 30 - Not in the group S-T
S-W14 100 100 - Toolbar
U-17 90 90 - Undo and Redo follow the writer
U-06 60 60 - Messages
X-02 600 1200 - Killed while typing
garbage line
"""
rc, ids, _ = run_tool(["--shards", "1", "--index", "0"], LIST)
check("one shard holds every case, in the order of the list", rc == 0 and ids[0] == "C-01" and ids[-1] == "X-02" and len(ids) == 11, ids)
allids = []
for i in range(3):
    rc, part, _ = run_tool(["--shards", "3", "--index", str(i)], LIST)
    allids += part
check("three shards hold every case exactly once", sorted(allids) == sorted(tool.parse(LIST) and [c[0] for c in tool.parse(LIST)]), allids)
rc, a, _ = run_tool(["--shards", "3", "--index", "1"], LIST)
rc, b, _ = run_tool(["--shards", "3", "--index", "1"], LIST)
check("the split is the same every time", a == b)
rc, _, plan = run_tool(["--shards", "3", "--plan", "--profile", "full"], LIST)
check("the plan names a line per shard", len(plan.strip().splitlines()) == 3 and "shard 0:" in plan, plan)
rc, grp, _ = run_tool(["--shards", "1", "--index", "0", "--cases", "S-T, C-01"], LIST)
check("a group takes its ids that go on with a digit, a name takes only itself", grp == ["C-01", "S-T01", "S-T02", "S-T10"], grp)
rc, _, _ = run_tool(["--shards", "2", "--index", "0", "--cases", "Z-99"], LIST)
check("a name that matches nothing stops with 2", rc == 2)
rc, _, _ = run_tool(["--shards", "2", "--index", "5"], LIST)
check("an index outside the shards stops with 2", rc == 2)
where = {}
for i in range(4):
    _, part, _ = run_tool(["--shards", "4", "--index", str(i)], LIST)
    for c in part:
        where[c] = i
check("a case that reads another one's facts goes into its shard", where["U-17"] == where["S-W14"], where)
cases = tool.parse(LIST)
parts, load = tool.split(cases, "full", 3)
check("the load of the shards differs by less than the longest case", max(load) - min(load) <= 1200 + tool.OVERHEAD, load)
check("smoke counts half the quick estimate and 30 s", tool.estimate(("X", 600, 1200), "smoke") == 330)

# the real list: the estimates are in tests/crashsafe/cs_list.cpp
src = read("tests/crashsafe/cs_list.cpp")
real = re.findall(r'\{QStringLiteral\("([A-Z]-[A-Za-z0-9]+)"\),\s*QStringLiteral\("[^"]*"\),\s*(\d+),\s*(\d+),', src)
text = "\n".join(f"{i} {q} {f} - t" for i, q, f in real)
check("the harness' list has more than 100 cases", len(real) > 100, len(real))
parts, load = tool.split(tool.parse(text), "full", 12)
flat = [x for p in parts for x in p]
check("12 shards of the real list hold every case once", sorted(flat) == sorted(i for i, _, _ in real))
longest = max(int(f) for _, _, f in real) + tool.OVERHEAD
check("... and end within the longest case of each other", max(load) - min(load) <= longest, (max(load), min(load), longest))

# ------------------------------------------------------------------------------------------------ collect and join
with tempfile.TemporaryDirectory() as tmp:
    res = os.path.join(tmp, "res")
    for cid, fails in (("C-01", 0), ("C-02", 2)):
        d = os.path.join(res, cid)
        os.makedirs(d)
        for n, t in (("case.log", "log"), ("verify.log", "v"), ("facts.json", "{}"), ("shot.png", "png")):
            open(os.path.join(d, n), "w").write(t)
        line = f"{cid} rc=0 verify=0 pass=5 fail={fails} time=7s\n"
        open(os.path.join(d, "result.line"), "w").write(line)
        open(os.path.join(res, "summary.txt"), "a").write(line)
    out = os.path.join(tmp, "out")
    r = subprocess.run(["bash", os.path.join(ROOT, "tools", "system-test-collect.sh"), res, out, "3"], capture_output=True, text=True)
    summ = open(os.path.join(out, "3-st-summary.txt")).read()
    check("a shard's summary starts with its result line", r.returncode == 0 and summ.startswith("result: shard 3: 2 case(s) ended, 1 with failures"), summ)
    names = tarfile.open(os.path.join(out, "3-st-logs.tar.gz")).getnames()
    check("the logs hold both cases", "C-01/case.log" in names and "C-02/verify.log" in names, names)
    check("a picture is kept only for the case with a failure", "C-02/shot.png" in names and "C-01/shot.png" not in names, names)
    os.makedirs(os.path.join(tmp, "sums"))
    for n in (0, 2, 10):
        open(os.path.join(tmp, "sums", f"{n}-st-summary.txt"), "w").write(
            f"result: shard {n}\nC-0{n % 10} rc=0 verify=0 pass=3 fail={1 if n == 2 else 0} time=1s\n")
    r = subprocess.run(["bash", os.path.join(ROOT, "tools", "system-test-join.sh"), os.path.join(tmp, "sums"), "777", "11"],
                       capture_output=True, text=True)
    j = open(os.path.join(tmp, "sums", "777-st-summary.txt")).read()
    check("the joined summary counts shards, cases and failures", "3 of 11 shard(s) reported, 3 case(s) ended, 1 with failures (1 failure(s))" in j, j)
    check("... names the shard that left nothing", "missing shards: 1 3 4 5 6 7 8 9" in j, j)
    check("... lists the shards in number order (2 before 10)", j.index("== shard 2") < j.index("== shard 10"), j)
    r = subprocess.run(["bash", os.path.join(ROOT, "tools", "system-test-join.sh"), os.path.join(tmp, "sums"), "778", "11"],
                       capture_output=True, text=True)
    j2 = open(os.path.join(tmp, "sums", "778-st-summary.txt")).read()
    check("a run's own joined file is not joined into the next one", "== shard 777" not in j2, j2)

# ------------------------------------------------------------------------------------------------ the workflow
wf = read(".github/workflows/system-test.yml")
on = wf.split("\njobs:")[0]
check("it starts by hand or on a push to linux-st/** only", "workflow_dispatch:" in on and "'linux-st/**'" in on
      and "pull_request" not in on and "issues:" not in on and "issue_comment" not in on and "schedule:" not in on, on)
check("it names no secret and no pull request event anywhere", "secrets." not in wf and "pull_request" not in re.sub(r"#[^\n]*", "", wf))
uses = re.findall(r"uses:\s*(\S+)", wf)
check("every action is pinned to a commit", uses and all(re.match(r"[\w./-]+@[0-9a-f]{40}$", u) for u in uses), uses)
check("it guards the repository and reads the run file", "github.repository == 'Shelin-boop/Quillbinder_Collection'" in wf
      and "tests/crashsafe/system-test-run.txt" in wf)
check("it publishes to the pre-release linux-st and joins the summaries", "linux-st" in wf and "system-test-join.sh" in wf
      and "system-test-collect.sh" in wf and "system-test-shard.py" in wf)
check("it builds with the test switches and the compiler cache as release.yml does",
      "-DQB_TEST_SWITCHES=ON" in wf and "ccache-linux-tests-" in wf and "tools/get-whisper.sh" in wf)
check("an input is never put into a script text", not re.search(r"run:[^\n]*\$\{\{\s*(github\.event\.inputs|inputs)", wf)
      and "${{ github.event.inputs" not in re.sub(r"\n\s+IN_\w+: \$\{\{ github\.event\.inputs\.\w+ \}\}", "", wf))
runfile = {k: v for k, v in (l.split(":", 1) for l in read("tests/crashsafe/system-test-run.txt").splitlines()
                              if l and not l.startswith("#") and ":" in l)}
check("the run file has shards, profile and minutes", runfile["shards"].strip().isdigit() and runfile["profile"].strip() in ("quick", "full", "smoke")
      and runfile["minutes"].strip().isdigit() and "cases" in runfile, runfile)

# ------------------------------------------------------------------------------------------------ the harness for Windows
POSIX_INC = re.compile(r'#\s*include\s*<(unistd\.h|sys/[a-z]+\.h|signal\.h|csignal|fcntl\.h|pwd\.h|dlfcn\.h)>')
POSIX_CALL = re.compile(r'((?<![A-Za-z])::(kill|geteuid|chmod|chown|setrlimit|dup2|rename|open)\(|SIGKILL|SIGTERM|SIGSEGV|O_WRONLY)')


def unguarded(path):
    """lines with a POSIX header or call that a Windows compile would see."""
    stack = []   # 'win' (only Windows), 'posix' (not Windows), 'other'
    bad = []
    with open(path, encoding="utf-8") as f:
        for n, line in enumerate(f, 1):
            s = line.strip()
            m = re.match(r"#\s*(ifdef|ifndef|if|elif|else|endif)\b(.*)", s)
            if m:
                kw, rest = m.group(1), m.group(2).strip()
                if kw in ("ifdef", "if", "ifndef"):
                    win = "Q_OS_WIN" in rest or "_WIN32" in rest
                    neg = kw == "ifndef" or "!defined(Q_OS_WIN)" in rest or "!defined(_WIN32)" in rest
                    stack.append(("posix" if neg else "win") if win else "other")
                elif kw == "else" and stack:
                    stack[-1] = {"win": "posix", "posix": "win"}.get(stack[-1], "other")
                elif kw == "elif" and stack:
                    stack[-1] = "other"
                elif kw == "endif" and stack:
                    stack.pop()
                continue
            if "posix" in stack:
                continue
            code = re.sub(r'"(\\.|[^"\\])*"', '""', line)
            code = re.sub(r"//.*", "", code)
            if POSIX_INC.search(code) or POSIX_CALL.search(code):
                bad.append(n)
    return bad


folder = os.path.join(ROOT, "tests", "crashsafe")
problems = {}
for name in sorted(os.listdir(folder)):
    if name.endswith((".cpp", ".h")) and name not in ("faultinject.cpp", "cs_plat.cpp"):
        b = unguarded(os.path.join(folder, name))
        if b:
            problems[name] = b[:5]
check("no POSIX header or call in the harness outside a branch that Windows skips", not problems, problems)
plat = read("tests/crashsafe/cs_plat.cpp")
check("the platform layer has both halves and no POSIX header on the Windows side",
      "TerminateProcess" in plat and "SendInput" in plat and "#ifdef Q_OS_WIN\n#include <QGuiApplication>" in plat
      and unguarded(os.path.join(folder, "cs_plat.cpp")) == [])
check("the Windows sender has the log of the Linux one", all(t in read("tests/crashsafe/sender_win.py") for t in ('"k %d %d\\n"', '"b %d %d\\n"', '"end %d\\n"', "SendInput")))
cm = read("tests/CMakeLists.txt")
check("the harness is built on Windows only when asked (QB_SYSTEM_TEST_WINDOWS)", "QB_SYSTEM_TEST_WINDOWS" in cm and "crashsafe/cs_plat.cpp" in cm)

print(f"{FAILS} failure(s)")
sys.exit(1 if FAILS else 0)
