#!/usr/bin/env python3
"""Suite secwatch (2026-11-21): the security watch (tools/security-watch.py, tools/security-watch.json,
.github/workflows/security-watch.yml), offline. The network is replaced by answers made up here in the forms of the
real sources; the run's decisions (what is reported, what is not, when the run fails) are checked, and the watch
list's versions are compared with the files the build takes them from.

  python3 -I tests/security_watch_tests.py
"""
import importlib.util
import io
import json
import os
import re
import sys
from contextlib import redirect_stdout

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("secwatch", os.path.join(ROOT, "tools", "security-watch.py"))
sw = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sw)

FAILS = 0


def check(name, ok, detail=""):
    global FAILS
    print(("PASS " if ok else "FAIL ") + name + ("" if ok else f": {detail}"))
    if not ok:
        FAILS += 1


# ---------------------------------------------------------------------------------------------------- pieces
check("versions compare by number", sw.vcmp((6, 12, 0), (6, 9, 5)) > 0 and sw.vcmp((1, 3), (1, 3, 0)) == 0)
check("a version that is not a plain number is not compared", sw.vtuple("1.7.2+really1.7.2-10build3") is None)
comp = {"version": "2.14.3", "tag": "VER-{v}", "sep": "-"}
check("a tag with other separators", sw.tag_of(comp) == "VER-2-14-3"
      and sw.newer_versions(comp, {"VER-2-14-3": "", "VER-2-14-4": "", "VER-2-13-9": ""}) == ["2.14.4"])
comp = {"version": "3.5.9", "tag": "openssl-{v}"}
check("pre-releases and older lines are not newer releases",
      sw.newer_versions(comp, {"openssl-3.5.10": "", "openssl-4.1.0-beta1": "", "openssl-3.4.9": "", "rsaref": ""})
      == ["3.5.10"])
check("ranges as text", sw.text_range("1.9.5", "< 1.9.6") is True and sw.text_range("1.9.6", ">= 1.0, < 1.9.6") is False
      and sw.text_range("1.9.5", "<= main") is None)
check("ranges as bounds", sw.in_range("6.12.0", "6.9.0", below="6.12.1") is True
      and sw.in_range("6.12.0", "4.3.0", below="6.11.2") is False and sw.in_range("6.12.0", "6.12.0") is True)


def rec(cve, sev, ranges, modules=("qtbase",), product="Qt Framework", adp=(), title="A flaw", state="PUBLISHED",
        kind="cvssV4_0"):
    versions = [{"status": "affected", "version": a, "lessThan": b, "versionType": "python"} for a, b in ranges]
    aff = {"vendor": "qt", "product": product, "versions": versions, "defaultStatus": "unaffected"}
    if modules:
        aff["modules"] = list(modules)
    return {"cveMetadata": {"cveId": cve, "state": state},
            "containers": {"cna": {"providerMetadata": {"shortName": "Qt"}, "title": title, "affected": [aff],
                                   "metrics": [{kind: {"baseSeverity": sev}}] if sev else []},
                           "adp": [{"providerMetadata": {"shortName": "CISA-ADP"}, "metrics": list(adp)}]}}


SHIP = {"qtbase", "qtsvg"}
r_yes = rec("CVE-2026-1001", "HIGH", [("6.9.0", "6.12.1")])
check("a record whose range holds our version", sw.record_affects(r_yes, "6.12.0", SHIP) == "yes")
check("a record whose ranges end before our version",
      sw.record_affects(rec("CVE-2026-1002", "HIGH", [("4.3.0", "6.8.9"), ("6.9.0", "6.11.2")]), "6.12.0", SHIP) == "no")
check("a Qt module we do not ship",
      sw.record_affects(rec("CVE-2026-1003", "HIGH", [("6.0.0", "6.13.0")], ("qtquick3d",)), "6.12.0", SHIP) == "other part")
check("another Qt product", sw.record_affects(rec("CVE-2026-1004", "HIGH", [("2.0.0", "9.0.0")], (), "Qt for MCUs"),
                                              "6.12.0", SHIP) == "other part")
r_none = {"containers": {"cna": {"affected": [{"vendor": "n/a", "product": "n/a", "versions": [{"version": "n/a",
                                                                                               "status": "affected"}]}]}}}
check("a record without readable versions counts as not clear", sw.record_affects(r_none, "9.0.2") == "unclear")
r_gh = {"containers": {"cna": {"affected": [{"vendor": "ggml-org", "product": "whisper.cpp",
                                             "versions": [{"version": "< 1.9.6", "status": "affected"}]}]}}}
check("GitHub's form of a range in a record", sw.record_affects(r_gh, "1.9.5") == "yes"
      and sw.record_affects(r_gh, "1.9.6") == "no")
exploited = rec("CVE-2026-1005", "LOW", [("6.0.0", "6.13.0")],
                adp=[{"other": {"type": "ssvc", "content": {"options": [{"Exploitation": "active"}]}}}])
rats = sw.record_ratings(exploited)
check("ratings of the CNA and CISA's 'actively exploited'",
      ("Qt", "LOW", "CVSS 4.0", "maker") in rats and ("CISA-ADP", "EXPLOITED", "SSVC", "other") in rats, rats)
own = {"containers": {"cna": {"providerMetadata": {"shortName": "openssl"},
                              "metrics": [{"other": {"type": "policy", "content": {"text": "Low"}}}]},
                      "adp": [{"providerMetadata": {"shortName": "CISA-ADP"}, "metrics": [{"cvssV3_1": {"baseSeverity": "HIGH"}}]}]}}
check("the maker's own rating decides over CISA's score", [r[1] for r in sw.decisive(sw.record_ratings(own))] == ["LOW"])
check("CISA's score counts where the maker gives none", [r[1] for r in sw.decisive(
    [("CISA-ADP", "HIGH", "CVSS 3.1", "other")])] == ["HIGH"])
check("Ubuntu's priority decides for its packages", [r[1] for r in sw.decisive(
    [("Ubuntu", "LOW", "priority", "maker"), ("NVD", "HIGH", "CVSS 3.1", "maker")])] == ["LOW"])
gdal = {"containers": {"cna": {"affected": [{"vendor": "OSGeo", "product": "gdal",
                                             "versions": [{"version": "0", "lessThan": "3.11.0", "status": "affected"}]}]}}}
ossl = {"containers": {"cna": {"affected": [{"product": "OpenSSL", "versions": [
    {"version": "3.5.0", "lessThan": "3.5.9", "status": "affected"},
    {"version": "1.1.1", "lessThan": "1.1.1zj", "status": "affected"}]}]}}}
check("a bound with letters (OpenSSL's 1.1.1zj) does not make a record unclear",
      sw.record_affects(ossl, "3.5.9", None, "openssl") == "no" and sw.record_affects(ossl, "3.5.8", None, "openssl") == "yes")
old_qt = {"containers": {"cna": {"descriptions": [{"lang": "en", "value": "An issue in HTTP2 in Qt before 5.15.18, "
          "6.x before 6.2.13, 6.3.x through 6.5.x before 6.5.7, and 6.6.x through 6.7.x before 6.7.2."}]}}}
ff = {"containers": {"cna": {"descriptions": [{"lang": "en", "value": "FFmpeg through 9.0.2 has a heap overflow."}]}}}
check("a record without versions whose description names only older fixes is past",
      sw.described_past(old_qt, "6.12.0") and not sw.described_past(old_qt, "6.7.1") and not sw.described_past(ff, "9.0.2"))
check("another project's copy of a part is not the part", sw.record_affects(gdal, "1.3.2", None, "zlib") == "other part")
check("'fixed in' from the ranges", sw.record_fixed(r_yes) == ["6.12.1"])
check("the path of a CVE record", sw.cve_url("CVE-2026-116337").endswith("/cves/2026/116xxx/CVE-2026-116337.json")
      and sw.cve_url("CVE-2021-0007").endswith("/cves/2021/0xxx/CVE-2021-0007.json"))

QT_LIST = """== Qt ==
=== CVE-2026-1001: ===
Impact: Availability. Affected versions: from Qt 5.0.0 to Qt 6.8.9, from Qt 6.9.0 to Qt 6.12.0. Fixed: Qt 6.12.1
=== CVE-2026-1002: ===
Affected versions: from Qt 4.3.0 to Qt 6.8.8, from Qt 6.9.0 to Qt 6.11.1. Fixed: Qt 6.8.9, Qt 6.11.2, Qt 6.12.0
=== CVE-2026-1006: ===
Affected versions: from Qt 6.10.0 to Qt 6.13.2. Fixed: Qt 6.13.3
=== CVE-2026-1007: ===
A flaw in a tool. No versions named here.
=== CVE-2026-1008: ===
A flaw in the network authorization. Fixed: Qt 5.15.17, Qt 6.7.1
"""
cands, total, fixed = sw.qt_list_candidates(QT_LIST, "6.12.0")
check("Qt's list: the entries that may concern our version", total == 5
      and cands == ["CVE-2026-1001", "CVE-2026-1006", "CVE-2026-1007", "CVE-2026-1008"], (total, cands))
check("Qt's list: the versions an entry names as fixed", fixed["CVE-2026-1008"] == ["5.15.17", "6.7.1"]
      and fixed["CVE-2026-1001"] == ["6.12.1"])
adv = {"vulnerabilities": [{"vulnerable_version_range": "<= 1.9.5", "patched_versions": "1.9.6"}]}
check("a GitHub advisory for our version", sw.advisory_affects(adv, "1.9.5") == "yes"
      and sw.advisory_affects(adv, "1.9.6") == "no" and sw.advisory_affects({"vulnerabilities": []}, "1.0") == "unclear")


# ---------------------------------------------------------------------------------------------------- a whole run
WL = {
    "qt": {"name": "Qt", "version": "6.12.0", "repo": "https://github.com/qt/qtbase", "tag": "v{v}", "uses": "the program",
           "modules": ["qtbase", "qtsvg"], "lists": ["https://qt.example/list?action=raw", "https://qt.example/list"],
           "list_min_entries": 4, "tell_when_released": ["6.12.1"]},
    "components": [
        {"name": "FFmpeg", "version": "9.0.2", "repo": "https://github.com/FFmpeg/FFmpeg", "tag": "n{v}",
         "uses": "the media add-ons"},
        {"name": "whisper.cpp", "version": "1.9.5", "repo": "https://github.com/ggml-org/whisper.cpp", "tag": "v{v}",
         "uses": "speech"}],
    "ubuntu": {"ecosystem": "Ubuntu:24.04:LTS", "packages": {"libpng1.6": "1.6.43-5ubuntu0.7"}},
    "osv_canaries": [{"commit_of": "FFmpeg", "tag": "n4.0"},
                     {"package": {"name": "libpng1.6", "ecosystem": "Ubuntu:24.04:LTS"}, "version": "1.6.0-1"}]}

TAGS = {"https://github.com/qt/qtbase": {"v6.12.0": "q0", "v6.12.1": "q1"},
        "https://github.com/FFmpeg/FFmpeg": {"n9.0.2": "f902", "n4.0": "f400"},
        "https://github.com/ggml-org/whisper.cpp": {"v1.9.5": "w195", "v1.9.6": "w196"}}
RECORDS = {
    "CVE-2026-1001": r_yes,                                                          # High, ours: reported
    "CVE-2026-1006": rec("CVE-2026-1006", "LOW", [("6.10.0", "6.13.3")]),          # ours, but Low: not reported
    "CVE-2026-1007": rec("CVE-2026-1007", "CRITICAL", [("1.0.0", "2.0.0")], (), "Qt Creator"),  # another product
    "CVE-2026-1008": {"cveMetadata": {"state": "PUBLISHED"}, "containers": {"cna": {
        "affected": [{"vendor": "n/a", "product": "n/a", "versions": [{"version": "n/a", "status": "affected"}]}],
        "metrics": [{"cvssV3_1": {"baseSeverity": "CRITICAL"}}]}}},                  # fixed long before ours
    "CVE-2026-2002": {"containers": {"cna": {"providerMetadata": {"shortName": "ffmpeg"}, "affected": [
        {"product": "FFmpeg", "versions": [{"version": "0", "lessThanOrEqual": "9.0.2", "status": "affected"}]}],
        "metrics": [{"other": {"type": "policy", "content": {"text": "Low"}}}]},
        "adp": [{"providerMetadata": {"shortName": "CISA-ADP"}, "metrics": [{"cvssV3_1": {"baseSeverity": "HIGH"}}]}]}},
    "CVE-2026-2003": {"containers": {"cna": {"affected": [
        {"product": "gdal", "versions": [{"version": "0", "lessThan": "3.11.0", "status": "affected"}]}],
        "metrics": [{"cvssV3_1": {"baseSeverity": "HIGH"}}]}}},
    "CVE-2026-2001": {"containers": {"cna": {"title": "FFmpeg overflow", "affected": [], "metrics": []},
                                     "adp": [{"providerMetadata": {"shortName": "CISA-ADP"},
                                              "metrics": [{"cvssV3_1": {"baseSeverity": "HIGH"}}]}]}},
    "CVE-2026-3001": {"containers": {"cna": {"title": "libpng read", "affected": [], "metrics": []}}},
    "CVE-2026-4001": {"containers": {"cna": {"title": "whisper flaw", "affected": [
        {"product": "whisper.cpp", "versions": [{"version": "< 1.9.6", "status": "affected"}]}],
        "metrics": [{"cvssV3_1": {"baseSeverity": "MEDIUM"}}]}}},
}
OSV_VULNS = {
    "CVE-2026-2001": {"id": "CVE-2026-2001", "summary": "FFmpeg overflow"},
    "CVE-2026-2002": {"id": "CVE-2026-2002"},
    "CVE-2026-2003": {"id": "CVE-2026-2003"},
    "UBUNTU-CVE-2026-3001": {"id": "UBUNTU-CVE-2026-3001", "upstream": ["CVE-2026-3001"],
                             "severity": [{"type": "Ubuntu", "score": "high"}],
                             "affected": [{"ranges": [{"type": "ECOSYSTEM", "events": [
                                 {"introduced": "0"}, {"fixed": "1.6.43-5ubuntu0.8"}]}]}]},
    "UBUNTU-CVE-2026-3002": {"id": "UBUNTU-CVE-2026-3002", "severity": [{"type": "Ubuntu", "score": "low"}]},
}


class Net:
    def __init__(self, **over):
        self.posts = []
        self.over = over
        self.existing = over.get("existing", [])

    def git_tags(self, url):
        if url in self.over.get("git_fail", ()):
            raise sw.SourceError("git down")
        return dict(TAGS[url])

    def http(self, url, data=None, headers=None, tries=3, timeout=40):
        if data is not None:
            if url.endswith("/querybatch"):
                out = []
                for q in data["queries"]:
                    if q.get("commit") == "f902":
                        out.append({"vulns": [{"id": "CVE-2026-2001"}, {"id": "CVE-2026-2002"}, {"id": "CVE-2026-2003"}]})
                    elif q.get("commit") == "f400":
                        out.append({"vulns": [] if self.over.get("empty_canary") else [{"id": "CVE-2018-1"}]})
                    elif q.get("version") == "1.6.43-5ubuntu0.7":
                        out.append({"vulns": [{"id": "UBUNTU-CVE-2026-3001"}, {"id": "UBUNTU-CVE-2026-3002"}]})
                    elif q.get("version") == "1.6.0-1":
                        out.append({"vulns": [{"id": "UBUNTU-CVE-2020-1"}]})
                    else:
                        out.append({})
                return 200, json.dumps({"results": out}).encode()
            self.posts.append((url, data))
            return (201 if "/issues" in url or "/labels" in url else 200), b"{}"
        if url.startswith("https://api.osv.dev/v1/vulns/"):
            v = OSV_VULNS.get(url.rsplit("/", 1)[1])
            return (200, json.dumps(v).encode()) if v else (404, b"")
        if url.startswith(sw.CVELIST):
            cve = url.rsplit("/", 1)[1][:-5]
            r = RECORDS.get(cve)
            return (200, json.dumps(r).encode()) if r else (404, b"")
        if url.startswith("https://qt.example/list"):
            text = self.over.get("qt_list", QT_LIST)
            return 200, text.encode()
        if "/security-advisories" in url:
            if "whisper.cpp" in url:
                return 200, json.dumps([{"ghsa_id": "GHSA-aaaa-bbbb-cccc", "cve_id": "CVE-2026-4001", "severity": "high",
                                         "summary": "whisper flaw", "html_url": "https://github.com/advisory",
                                         "vulnerabilities": [{"vulnerable_version_range": "< 1.9.6",
                                                              "patched_versions": "1.9.6"}]}]).encode()
            return 200, b"[]"
        if "/releases/tags/" in url:
            return 404, b""
        if "/issues?labels=" in url:
            return 200, json.dumps(self.existing).encode()
        if "/comments" in url:
            return 200, b"[]"
        raise AssertionError("unexpected request " + url)


def run(dry, **over):
    net = Net(**over)
    sw.http, sw.git_tags = net.http, net.git_tags
    path = os.path.join(os.environ.get("TMPDIR", "/tmp"), "secwatch-test-list.json")
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(WL, fh)
    os.environ.update(GITHUB_REPOSITORY="owner/repo", GITHUB_REPOSITORY_OWNER="owner", GH_TOKEN="t")
    os.environ.pop("GITHUB_STEP_SUMMARY", None)
    buf = io.StringIO()
    with redirect_stdout(buf):
        rc = sw.main(["--watchlist", path] + (["--dry-run"] if dry else []))
    return rc, buf.getvalue(), net


rc, out, net = run(True)
lines = re.findall(r"^  ((?:Security|Release) watch: .*)", out, re.M)
keys = sorted(k for l in lines for k in l.rsplit(": ", 1)[1].split(", "))
check("dry run: the run ends well", rc == 0, out[-600:])
check("dry run: High findings for versions we ship are reported, and only those",
      keys == ["CVE-2026-1001", "CVE-2026-2001", "CVE-2026-3001", "CVE-2026-4001", "Qt 6.12.1"], keys)
check("one issue per project, named by its title",
      "  Security watch: FFmpeg: 1 finding rated High or Critical: CVE-2026-2001" in out
      and "  Release watch: Qt 6.12.1 is out: Qt 6.12.1" in out, lines)
check("a GitHub advisory rated High counts, though the CNA says Medium", "CVE-2026-4001 High" in out)
check("the release asked for is reported", "Release watch: Qt 6.12.1 is out" in out)
check("dry run: no issue written", not net.posts and "(dry run: no issue written)" in out, net.posts)
check("the report names every part", all(p in out for p in ("Qt 6.12.0:", "FFmpeg 9.0.2:", "whisper.cpp 1.9.5:",
                                                             "Ubuntu packages in the AppImage")))
check("the report names newer releases", "newer releases 6.12.1" in out and "newer releases 1.9.6" in out)

rc, out, net = run(False, existing=[
    {"number": 3, "state": "closed", "comments": 0, "body": "<!-- security-watch-key: CVE-2026-2001 -->\n"
     "<!-- security-watch-group: https://github.com/FFmpeg/FFmpeg -->"},
    {"number": 7, "state": "open", "comments": 1, "body": "<!-- security-watch-group: https://github.com/ggml-org/whisper.cpp -->"}])
issues = [d for u, d in net.posts if u.endswith("/issues")]
comments = [(u, d) for u, d in net.posts if u.endswith("/comments")]
check("a real run writes issues for new findings only", rc == 0 and len(issues) == 3
      and not any("FFmpeg" in d["title"] for d in issues), [d["title"] for d in issues])
check("a new finding for a project with an open issue becomes a comment on it", len(comments) == 1
      and comments[0][0].endswith("/issues/7/comments") and "CVE-2026-4001" in comments[0][1]["body"]
      and not any("whisper" in d["title"] for d in issues), comments)
check("issues carry the label and are assigned to the owner",
      all(d["labels"] == ["security"] and d["assignees"] == ["owner"] for d in issues))
body = next((d["body"] for d in issues if "CVE-2026-3001" in d["body"]), "")
check("an issue says what, where, how rated and how found",
      all(s in body for s in ("**What:** libpng read", "libpng1.6 1.6.43-5ubuntu0.7", "Ubuntu: high (priority)",
                              "1.6.43-5ubuntu0.8", "OSV.dev", "security-watch-key: CVE-2026-3001",
                              "security-watch-group: ubuntu: libpng1.6")), body)
check("the label is made before the first issue", net.posts and net.posts[0][0].endswith("/labels"))

rc, out, net = run(False, empty_canary=True)
check("a source that answers nothing where it must fails the run", rc == 1 and "SOURCE FAILED" in out and not net.posts)
rc, out, net = run(True, git_fail=("https://github.com/ggml-org/whisper.cpp",))
check("a repository that cannot be read fails the run", rc == 1 and "git down" in out)
rc, out, net = run(True, qt_list="nothing here")
check("Qt's list in another form fails the run", rc == 1 and "Qt's list" in out)
saved = TAGS["https://github.com/FFmpeg/FFmpeg"].pop("n9.0.2")
rc, out, net = run(True)
TAGS["https://github.com/FFmpeg/FFmpeg"]["n9.0.2"] = saved
check("a version whose tag is not there fails the run", rc == 1 and "n9.0.2" in out)

# ---------------------------------------------------------------------------------------------------- the real list
with open(os.path.join(ROOT, "tools", "security-watch.json"), encoding="utf-8") as fh:
    real = json.load(fh)
with open(os.path.join(ROOT, "resources", "licenses", "index.json"), encoding="utf-8") as fh:
    lic = {c["name"]: c for c in json.load(fh)["components"]}


def pinned(pin):
    if "licenses" in pin:
        return (lic.get(pin["licenses"]) or {}).get("version")
    if "comes_with" in pin:
        return None
    with open(os.path.join(ROOT, pin["file"]), encoding="utf-8") as fh:
        m = re.search(pin["pattern"], fh.read())
    return m.group(1) if m else None


bad = []
for c in [real["qt"]] + real["components"]:
    if not all(c.get(k) for k in ("name", "version", "repo", "tag", "uses", "pin")) or "{v}" not in c["tag"]:
        bad.append(c.get("name"))
        continue
    pin = c["pin"]
    if "comes_with" in pin:
        if pin["comes_with"] != f"Qt {real['qt']['version']}":
            bad.append(f"{c['name']}: comes with {pin['comes_with']}, the list has Qt {real['qt']['version']}")
    elif pinned(pin) != c["version"]:
        bad.append(f"{c['name']}: list {c['version']}, build {pinned(pin)}")
check("every part of the watch list has the version the build uses", not bad, bad)
ub = {c["package"]: c["version"] for c in lic.values() if c.get("package")}
check("the watch list holds the AppImage's Ubuntu packages at their versions", ub == real["ubuntu"]["packages"],
      set(ub.items()) ^ set(real["ubuntu"]["packages"].items()))
check("the release asked for is newer than the Qt we ship",
      all(sw.vcmp(sw.vtuple(v), sw.vtuple(real["qt"]["version"])) > 0 for v in real["qt"].get("tell_when_released", [])))

# ---------------------------------------------------------------------------------------------------- the workflow
with open(os.path.join(ROOT, ".github", "workflows", "security-watch.yml"), encoding="utf-8") as fh:
    wf = fh.read()
check("the workflow runs once a day and by hand", re.search(r"^\s*- cron: '\d+ \d+ \* \* \*'", wf, re.M) is not None
      and "workflow_dispatch:" in wf)
check("the workflow may read the code and write issues, nothing else",
      re.search(r"^permissions:\n  contents: read\n  issues: write\n(?!  )", wf, re.M) is not None)
check("the workflow checks out only the watch's two files",
      re.search(r"sparse-checkout: \|\n\s+tools/security-watch\.py\n\s+tools/security-watch\.json\n", wf) is not None
      and "sparse-checkout-cone-mode: false" in wf and "persist-credentials: false" in wf)
check("a push to the work branch is a dry run", "all/security-watch" in wf
      and "github.event_name == 'push' && '--dry-run'" in wf)

print(f"{FAILS} failure(s)")
sys.exit(1 if FAILS else 0)
