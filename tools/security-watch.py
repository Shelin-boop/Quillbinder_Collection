#!/usr/bin/env python3
"""The security watch (2026-11-21): a daily look at the third-party parts Quillbinder ships.

It reads only its watch list (tools/security-watch.json), never the rest of the repository, and asks public sources:

- each part's own repository (git ls-remote): its newer releases, and the commit of the version we ship;
- OSV.dev: the known vulnerabilities of those commits and of the Ubuntu packages inside the AppImage;
- Qt's own list of known vulnerabilities (Qt is the CNA for Qt);
- the published security advisories of each part's GitHub repository, and the release notes of a newer release;
- the CVE records of the CVE Program (github.com/CVEProject/cvelistV5): the ratings of the CNA and of CISA.

A finding that concerns a version we ship and that one of them rates High or Critical (or that CISA marks as
actively exploited) becomes one GitHub issue, labeled "security" and assigned to the repository's owner. Everything
else goes into the run's summary only. A main source that cannot be read fails the run, so that GitHub's notice of a
failed run tells that the watch itself needs a look.

  python3 -I tools/security-watch.py [--watchlist tools/security-watch.json] [--dry-run]

Environment: GH_TOKEN (the workflow's token, for GitHub's API), GITHUB_REPOSITORY, GITHUB_REPOSITORY_OWNER,
GITHUB_STEP_SUMMARY. --dry-run writes no issue.
"""
import json
import os
import re
import subprocess
import sys
import time
import urllib.error
import urllib.request

UA = "quillbinder-security-watch/1"
LABEL = "security"
HIGH = {"HIGH", "CRITICAL", "EXPLOITED"}
ORDER = {"EXPLOITED": 5, "CRITICAL": 4, "HIGH": 3, "MEDIUM": 2, "MODERATE": 2, "LOW": 1, "NEGLIGIBLE": 0, "NONE": 0}
CVE_RX = re.compile(r"CVE-\d{4}-\d{4,}")
OSV = "https://api.osv.dev/v1"
CVELIST = "https://raw.githubusercontent.com/CVEProject/cvelistV5/main/cves"
# products in Qt's list that are not the Qt framework we ship
QT_OTHER = re.compile(r"mcu|creator|axivion|design studio|squish|coco|insight|vs ?tools|safe renderer|3d studio|"
                      r"bridge|installer|gammaray|test", re.I)


class SourceError(Exception):
    """A source that could not be read (after retries)."""


# ---------------------------------------------------------------------------------------------------- network
def http(url, data=None, headers=None, tries=3, timeout=40):
    """GET (or POST with data as JSON). Returns (status, body); 404, 410 and 422 are answers, other errors retried."""
    h = {"User-Agent": UA}
    if headers:
        h.update(headers)
    body = None
    if data is not None:
        body = json.dumps(data).encode()
        h["Content-Type"] = "application/json"
    last = ""
    for i in range(tries):
        try:
            with urllib.request.urlopen(urllib.request.Request(url, data=body, headers=h), timeout=timeout) as r:
                return r.status, r.read()
        except urllib.error.HTTPError as e:
            if e.code in (404, 410, 422):
                return e.code, e.read()
            last = f"HTTP {e.code}"
        except (urllib.error.URLError, TimeoutError, OSError) as e:
            last = str(e)
        if i + 1 < tries:
            time.sleep((3, 10, 20)[i])
    raise SourceError(f"{url}: {last}")


def http_json(url, data=None, headers=None):
    st, body = http(url, data, headers)
    if st != 200:
        return st, None
    try:
        return st, json.loads(body)
    except ValueError:
        raise SourceError(f"{url}: not JSON")


def gh_headers():
    h = {"Accept": "application/vnd.github+json", "X-GitHub-Api-Version": "2022-11-28"}
    if os.environ.get("GH_TOKEN"):
        h["Authorization"] = "Bearer " + os.environ["GH_TOKEN"]
    return h


def git_tags(url):
    """{tag: commit} of a repository (an annotated tag gives the commit it points to)."""
    last = ""
    for i in range(3):
        try:
            out = subprocess.run(["git", "ls-remote", "--tags", url], capture_output=True, text=True, timeout=180,
                                 env=dict(os.environ, GIT_TERMINAL_PROMPT="0"))
            if out.returncode == 0:
                break
            last = out.stderr.strip()[-200:]
        except (subprocess.SubprocessError, OSError) as e:
            last = str(e)
        time.sleep((3, 10, 20)[i])
    else:
        raise SourceError(f"git ls-remote {url}: {last}")
    tags, peeled = {}, {}
    for line in out.stdout.splitlines():
        sha, _, ref = line.partition("\t")
        if ref.startswith("refs/tags/"):
            name = ref[len("refs/tags/"):]
            if name.endswith("^{}"):
                peeled[name[:-3]] = sha
            else:
                tags[name] = sha
    tags.update(peeled)
    return tags


# ---------------------------------------------------------------------------------------------------- versions
def vtuple(s):
    """'6.12.0' -> (6, 12, 0); None for anything that is not a plain dotted number."""
    s = (s or "").strip()
    return tuple(int(x) for x in s.split(".")) if re.fullmatch(r"\d+(?:\.\d+)*", s) else None


def vcmp(a, b):
    """-1, 0, 1 for two version tuples of any length (missing parts count as 0)."""
    n = max(len(a), len(b))
    a, b = a + (0,) * (n - len(a)), b + (0,) * (n - len(b))
    return (a > b) - (a < b)


def tag_of(comp, version=None):
    return comp["tag"].replace("{v}", (version or comp["version"]).replace(".", comp.get("sep", ".")))


def newer_versions(comp, tags):
    """The versions of tags newer than the one we ship, oldest first (pre-releases do not fit the template)."""
    sep = comp.get("sep", ".")
    pre, _, post = comp["tag"].partition("{v}")
    rx = re.compile("^" + re.escape(pre) + r"(\d+(?:" + re.escape(sep) + r"\d+)*)" + re.escape(post) + "$")
    ours = vtuple(comp["version"])
    out = set()
    for t in tags:
        m = rx.match(t)
        if m:
            v = m.group(1).replace(sep, ".")
            if vcmp(vtuple(v), ours) > 0:
                out.add(v)
    return sorted(out, key=lambda v: vtuple(v) + (0,) * (8 - len(vtuple(v))))


def loose(s):
    """'1.1.1zj' -> ((1, 1, 1), 'zj'), '6.12.0' -> ((6, 12, 0), ''); (None, '') for anything else."""
    m = re.fullmatch(r"v?(\d+(?:\.\d+)*)([a-z]{0,3})", (s or "").strip())
    return (tuple(int(x) for x in m.group(1).split(".")), m.group(2)) if m else (None, "")


def bound_cmp(vt, s):
    """vt against a bound that may carry letters ('1.1.1zj' lies between 1.1.1 and 1.1.2): -1, 1, or None."""
    b, suffix = loose(s)
    if b is None:
        return None
    c = vcmp(vt, b)
    if not suffix:
        return c
    return None if c == 0 else c


def in_range(v, start=None, below=None, upto=None):
    """v in [start, below) or [start, upto]; None when a bound cannot be compared."""
    vt = vtuple((v or "").lstrip("v"))
    if vt is None:
        return None
    if start not in (None, "", "0", "*"):
        c = bound_cmp(vt, start)
        if c is None:
            return None
        if c < 0:
            return False
    elif below is None and upto is None:
        return None
    if below is not None:
        c = bound_cmp(vt, below)
        return None if c is None else c < 0
    if upto is not None:
        c = bound_cmp(vt, upto)
        return None if c is None else c <= 0
    return bound_cmp(vt, start) == 0


def text_range(version, rng):
    """True / False / None (not readable) for a range written as text: '< 1.9.6', '>= 1.0, <= 1.2', '= 2.0'."""
    ok = True
    for part in rng.split(","):
        m = re.fullmatch(r"\s*(<=|<|>=|>|=)?\s*v?(\d+(?:\.\d+)*)\s*", part)
        if not m:
            return None
        op, c = m.group(1) or "=", vcmp(vtuple(version), vtuple(m.group(2)))
        ok = ok and {"<": c < 0, "<=": c <= 0, ">": c > 0, ">=": c >= 0, "=": c == 0}[op]
    return ok


# ---------------------------------------------------------------------------------------------------- CVE records
def cve_url(cve):
    year, num = cve[4:8], cve[9:]
    return f"{CVELIST}/{year}/{num[:-3]}xxx/{cve}.json"


def record_ratings(rec):
    """[(who, rating, kind, role)] of a CVE record: the CNA's ratings (role 'maker': CVSS or its own scale, as
    OpenSSL's "Low"), the ADPs' (role 'other': CISA), and CISA's 'actively exploited'."""
    out = []
    cont = rec.get("containers") or {}
    parts = [(cont.get("cna") or {}, "maker")] + [(a, "other") for a in cont.get("adp") or []]
    for c, role in parts:
        who = ((c.get("providerMetadata") or {}).get("shortName") or "CNA").strip()
        for m in c.get("metrics") or []:
            for k in ("cvssV4_0", "cvssV3_1", "cvssV3_0"):
                s = (m.get(k) or {}).get("baseSeverity")
                if s:
                    out.append((who, s.upper(), "CVSS " + k[5:].replace("_", "."), role))
            other = m.get("other") or {}
            content = other.get("content") or {}
            if other.get("type") == "ssvc":
                for o in content.get("options") or []:
                    if str(o.get("Exploitation", "")).lower() == "active":
                        out.append((who, "EXPLOITED", "SSVC", role))
            elif isinstance(content, dict) and str(content.get("text", "")).upper() in ORDER:
                out.append((who, content["text"].upper(), "its own scale", role))
    return out


def decisive(ratings):
    """The ratings that decide: a distribution's priority (Ubuntu) where there is one, else the maker's (the CNA,
    the project's own advisory), else all; CISA's 'actively exploited' always counts."""
    for pick in (lambda r: r[0] == "Ubuntu", lambda r: r[3] == "maker", lambda r: True):
        chosen = [r for r in ratings if pick(r) and r[1] != "EXPLOITED"]
        if chosen:
            break
    return chosen + [r for r in ratings if r[1] == "EXPLOITED"]


def product_word(name):
    return re.split(r"[^a-z0-9]+", name.lower())[0]


def record_affects(rec, version, modules=None, product=None):
    """Whether a CVE record names `version` as affected: 'yes', 'no', 'unclear', or 'other part' (another product, or
    a Qt module we do not ship). modules: the set of shipped Qt modules (Qt), product: the part's word (the others)."""
    cna = (rec.get("containers") or {}).get("cna") or {}
    entries = cna.get("affected") or []
    considered = 0
    unclear = False
    for a in entries:
        prod = f"{a.get('vendor', '')} {a.get('product', '')}"
        if modules is not None and QT_OTHER.search(prod):
            continue
        squeezed = re.sub(r"[^a-z0-9]", "", prod.lower())
        if product and squeezed not in ("", "na", "nana") and product not in squeezed:
            continue
        if modules is not None and a.get("modules"):
            if not {str(m).lower() for m in a["modules"]} & modules:
                continue
        considered += 1
        vs = a.get("versions") or []
        if not vs:
            unclear = unclear or a.get("defaultStatus") != "unaffected"
        for e in vs:
            if e.get("status") != "affected":
                continue
            v0 = str(e.get("version", ""))
            if re.match(r"\s*[<>=]", v0) and not e.get("lessThan") and not e.get("lessThanOrEqual"):
                r = text_range(version, v0)      # GitHub's records: "< 1.9.6"
            else:
                r = in_range(version, v0, e.get("lessThan"), e.get("lessThanOrEqual"))
            if r is True:
                return "yes"
            if r is None:
                unclear = True
    if entries and not considered:
        return "other part"
    return "unclear" if unclear or not entries else "no"


def described_past(rec, version):
    """For a record without versions: True when its description names only fixes our version already has
    ("... before 6.2.13, 6.3.x through 6.5.x before 6.5.7 ..."; "through 9.0.2" is not past for 9.0.2)."""
    cna = (rec.get("containers") or {}).get("cna") or {}
    desc = " ".join(d.get("value", "") for d in cna.get("descriptions") or [] if d.get("lang", "en").startswith("en"))
    ours = vtuple(version)
    before = [vtuple(x) for x in re.findall(r"\bbefore (?:version )?v?(\d+(?:\.\d+)+)", desc)]
    through = [vtuple(x) for x in re.findall(r"\bthrough (?:version )?v?(\d+(?:\.\d+)+)", desc)]
    if not before and not through:
        return False
    return all(vcmp(b, ours) <= 0 for b in before) and all(vcmp(t, ours) < 0 for t in through)


def record_fixed(rec):
    """The versions a CVE record names as the end of an affected range ('fixed in')."""
    out = []
    for a in ((rec.get("containers") or {}).get("cna") or {}).get("affected") or []:
        for e in a.get("versions") or []:
            if e.get("status") == "affected" and e.get("lessThan") and vtuple(e["lessThan"]):
                out.append(e["lessThan"])
    return sorted(set(out), key=lambda v: vtuple(v))


def record_title(rec):
    cna = (rec.get("containers") or {}).get("cna") or {}
    if cna.get("title"):
        return cna["title"].strip()
    for d in cna.get("descriptions") or []:
        if d.get("lang", "en").startswith("en"):
            t = " ".join(d.get("value", "").split())
            return t if len(t) <= 200 else t[:197].rsplit(" ", 1)[0] + " ..."
    return ""


# ---------------------------------------------------------------------------------------------------- Qt's list
def qt_list_candidates(text, version):
    """CVE ids of Qt's list whose entry may concern `version` (a cheap filter before the records are read):
    the entry names the version's series, a range that holds it, or no Qt version at all. Also returns the number
    of entries and, per entry, the versions its "Fixed" part names."""
    series = ".".join(version.split(".")[:2])
    found = list(CVE_RX.finditer(text))
    seen, out, fixed = set(), [], {}
    for i, m in enumerate(found):
        cve = m.group(0)
        if cve in seen:
            continue
        seen.add(cve)
        end = len(text)
        for n in found[i + 1:]:
            if n.group(0) != cve:
                end = n.start()
                break
        block, _, fixes = text[m.end():end].partition("Fixed")    # the versions that fix it do not count here
        fixed[cve] = re.findall(r"\b(\d+\.\d+\.\d+)\b", fixes)
        if re.search(r"\bQt\s+" + re.escape(series) + r"(?:\.\d+)?\b", block):
            out.append(cve)
            continue
        hit = False
        for a, b in re.findall(r"from Qt\s+(\d+(?:\.\d+)+)\s+(?:to|through|until)\s+Qt\s+(\d+(?:\.\d+)+)", block):
            if in_range(version, a, upto=b):
                hit = True
        for a in re.findall(r"Qt\s+(\d+(?:\.\d+)+)\s+(?:and|or)\s+(?:later|newer)", block):
            if in_range(version, a, below="999"):
                hit = True
        if hit or not re.search(r"Qt\s+\d+\.\d+", block):
            out.append(cve)
    return out, len(seen), fixed


# ---------------------------------------------------------------------------------------------------- GitHub advisories
def advisory_affects(adv, version):
    """'yes', 'no' or 'unclear' for a GitHub repository advisory and the version we ship."""
    vulns = adv.get("vulnerabilities") or []
    if not vulns:
        return "unclear"
    res = "no"
    for v in vulns:
        patched = (v.get("patched_versions") or "").strip()
        rng = (v.get("vulnerable_version_range") or "").strip()
        if patched and vtuple(patched.lstrip("v")) and vcmp(vtuple(version), vtuple(patched.lstrip("v"))) >= 0:
            continue
        if not rng:
            res = "unclear"
            continue
        ok = text_range(version, rng)
        if ok is None:
            res = "unclear"
        elif ok:
            return "yes"
    return res


# ---------------------------------------------------------------------------------------------------- OSV
def osv_ratings(v):
    out = []
    for s in v.get("severity") or []:
        if s.get("type") == "Ubuntu" and s.get("score"):
            out.append(("Ubuntu", str(s["score"]).upper(), "priority", "maker"))
    ds = v.get("database_specific") or {}
    if isinstance(ds.get("severity"), str) and ds["severity"]:
        out.append((v.get("id", "OSV").split("-")[0], ds["severity"].upper(), "advisory", "maker"))
    return out


def osv_cves(v):
    ids = [v.get("id", "")] + list(v.get("aliases") or []) + list(v.get("upstream") or [])
    out = []
    for i in ids:
        m = CVE_RX.search(i or "")
        if m and m.group(0) not in out:
            out.append(m.group(0))
    return out


def osv_fixed(v):
    out = []
    for a in v.get("affected") or []:
        for r in a.get("ranges") or []:
            if r.get("type") == "ECOSYSTEM":
                out += [e["fixed"] for e in r.get("events") or [] if e.get("fixed")]
    return sorted(set(out))


# ---------------------------------------------------------------------------------------------------- the run
class Watch:
    def __init__(self, wl, dry=False):
        self.wl = wl
        self.dry = dry
        self.lines = []          # the report
        self.findings = {}       # key -> finding
        self.records = {}        # CVE id -> record (or None)
        self.warnings = []
        self.osv_cache = {}
        self.tag_cache = {}

    def say(self, s):
        self.lines.append(s)
        print(s, flush=True)

    def tags(self, url):
        if url not in self.tag_cache:
            self.tag_cache[url] = git_tags(url)
        return self.tag_cache[url]

    def record(self, cve):
        if cve not in self.records:
            st, rec = http_json(cve_url(cve))
            if st not in (200, 404):
                raise SourceError(f"the CVE record {cve}: HTTP {st}")
            self.records[cve] = rec if rec and (rec.get("cveMetadata") or {}).get("state") != "REJECTED" else None
        return self.records[cve]

    def add(self, key, part, group, how, ratings, affects, title="", fixed=(), links=()):
        f = self.findings.setdefault(key, {"key": key, "group": group, "parts": [], "how": [], "ratings": [],
                                           "affects": affects, "title": title, "fixed": [], "links": []})
        if part not in f["parts"]:
            f["parts"].append(part)
        for lst, new in (("how", [how]), ("ratings", ratings), ("fixed", list(fixed)), ("links", list(links))):
            for x in new:
                if x and x not in f[lst]:
                    f[lst].append(x)
        if affects == "yes" or (f["affects"] != "yes" and affects == "unclear"):
            f["affects"] = affects
        f["title"] = f["title"] or title

    def add_cve(self, cve, comp, how, modules=None, extra_ratings=(), extra_links=(), fixed=(), past_fix=False):
        """A CVE named by a source for a part: kept unless its record says the version or the product is not ours."""
        part, group = self.part(comp), self.group(comp)
        rec = self.record(cve)
        link = f"https://www.cve.org/CVERecord?id={cve}"
        if rec is None:
            if extra_ratings:
                self.add(cve, part, group, how, list(extra_ratings), "unclear", "", fixed, [link] + list(extra_links))
            return
        aff = record_affects(rec, comp["version"], modules, None if modules else product_word(comp["name"]))
        if aff in ("no", "other part") or (aff == "unclear" and (past_fix or described_past(rec, comp["version"]))):
            return
        self.add(cve, part, group, how, record_ratings(rec) + list(extra_ratings), aff, record_title(rec),
                 list(fixed) + record_fixed(rec), [link] + list(extra_links))

    @staticmethod
    def part(comp):
        return f"{comp['name']} {comp['version']}"

    @staticmethod
    def group(comp):
        return comp["repo"]

    # ---- Qt
    def check_qt(self, tags):
        qt = self.wl["qt"]
        part = self.part(qt)
        newer = newer_versions(qt, tags)
        text = None
        for url in qt["lists"]:
            st, body = http(url)
            if st == 200 and CVE_RX.search(body.decode("utf-8", "replace")):
                text = body.decode("utf-8", "replace")
                break
        if text is None:
            raise SourceError("Qt's list of known vulnerabilities: no list could be read")
        cands, total, fixed = qt_list_candidates(text, qt["version"])
        if total < qt.get("list_min_entries", 20):
            raise SourceError(f"Qt's list of known vulnerabilities holds only {total} entries: its form may have changed")
        ours = vtuple(qt["version"])
        before = set(self.findings)
        for cve in cands:
            fx = [vtuple(v) for v in fixed.get(cve, [])]
            # a record without versions, for an entry whose newest fix is not newer than ours: fixed in our version
            past = bool(fx) and vcmp(max(fx), ours) <= 0
            self.add_cve(cve, qt, "Qt's list of known vulnerabilities", set(qt["modules"]),
                         extra_links=[qt["lists"][-1]], fixed=fixed.get(cve, []), past_fix=past)
        mine = [k for k in self.findings if k not in before]
        top = [k for k in mine if self.is_high(self.findings[k])]
        self.say(f"{part}: newer releases {', '.join(newer) or 'none'}; Qt's list: {total} entries, "
                 f"{len(cands)} looked at, {len(mine)} for a version and module we ship, High or Critical {len(top)}"
                 + (": " + ", ".join(f"{k} {self.rating(self.findings[k])}" for k in top) if top else ""))
        for v in qt.get("tell_when_released") or []:
            if tag_of(qt, v) in tags:
                self.findings[f"Qt {v}"] = {"key": f"Qt {v}", "release": True, "group": f"release: Qt {v}",
                                            "parts": [part], "version": v, "tag": tag_of(qt, v), "repo": qt["repo"]}

    # ---- GitHub: advisories and release notes
    def github_extras(self, comp, newer):
        m = re.match(r"https://github\.com/([^/]+/[^/]+?)(?:\.git)?$", comp["repo"])
        if not m:
            return 0
        repo = m.group(1)
        n = 0
        try:
            st, advs = http_json(f"https://api.github.com/repos/{repo}/security-advisories?state=published&per_page=100",
                                 headers=gh_headers())
            for adv in advs or []:
                aff = advisory_affects(adv, comp["version"])
                if aff == "no":
                    continue
                n += 1
                rat = ([(repo, str(adv["severity"]).upper(), "GitHub advisory", "maker")]
                       if adv.get("severity") else [])
                links = [adv.get("html_url", "")]
                if adv.get("cve_id"):
                    self.add_cve(adv["cve_id"], comp, f"the security advisories of {repo}", extra_ratings=rat,
                                 extra_links=links)
                else:
                    self.add(adv.get("ghsa_id"), self.part(comp), self.group(comp),
                             f"the security advisories of {repo}", rat, aff, adv.get("summary", ""), [], links)
        except SourceError as e:
            self.warnings.append(f"{self.part(comp)}: advisories not read ({e})")
        for v in newer[-3:]:
            try:
                st, rel = http_json(f"https://api.github.com/repos/{repo}/releases/tags/{tag_of(comp, v)}",
                                    headers=gh_headers())
            except SourceError as e:
                self.warnings.append(f"{self.part(comp)}: release notes of {v} not read ({e})")
                continue
            for cve in dict.fromkeys(CVE_RX.findall((rel or {}).get("body") or "")):
                n += 1
                self.add_cve(cve, comp, f"the release notes of {comp['name']} {v}", fixed=[v],
                             extra_links=[(rel or {}).get("html_url", "")])
        return n

    # ---- OSV
    def osv_batch(self, queries):
        """[[vuln ids]] for each query (pages followed)."""
        res = [[] for _ in queries]
        pending = list(range(len(queries)))
        tokens = {}
        for _ in range(6):
            if not pending:
                break
            batch = [dict(queries[i], **({"page_token": tokens[i]} if i in tokens else {})) for i in pending]
            st, ans = http_json(f"{OSV}/querybatch", {"queries": batch})
            if st != 200 or not ans or len(ans.get("results", [])) != len(batch):
                raise SourceError(f"OSV's querybatch answered HTTP {st}")
            nxt = []
            for i, r in zip(pending, ans["results"]):
                res[i] += [v["id"] for v in r.get("vulns") or []]
                if r.get("next_page_token"):
                    tokens[i] = r["next_page_token"]
                    nxt.append(i)
            pending = nxt
        return res

    def osv_vuln(self, vid):
        if vid not in self.osv_cache:
            st, v = http_json(f"{OSV}/vulns/{vid}")
            if st != 200 or not v:
                raise SourceError(f"OSV's record {vid}: HTTP {st}")
            self.osv_cache[vid] = v
        return self.osv_cache[vid]

    def osv_finding(self, vid, owner):
        """owner: (part, group, the component or None for an Ubuntu package)."""
        part, group, comp = owner
        v = self.osv_vuln(vid)
        if v.get("withdrawn"):
            return
        cves = osv_cves(v)
        rat = osv_ratings(v)
        links = [f"https://osv.dev/vulnerability/{vid}"]
        fixed = osv_fixed(v)
        rec = self.record(cves[0]) if cves else None
        if rec and comp is not None:
            # OSV matched a commit we ship; the record can still say that it is another product or version
            if record_affects(rec, comp["version"], None, product_word(comp["name"])) in ("no", "other part"):
                return
        title = (record_title(rec) if rec else "") or v.get("summary", "")
        key = cves[0] if cves else vid
        self.add(key, part, group, "OSV.dev", rat + (record_ratings(rec) if rec else []), "yes", title, fixed,
                 ([f"https://www.cve.org/CVERecord?id={key}"] if cves else []) + links)

    # ---- decisions
    @staticmethod
    def rating(f):
        best = max((r[1] for r in decisive(f["ratings"])), key=lambda r: ORDER.get(r, -1), default="")
        return {"EXPLOITED": "actively exploited", "": "not rated yet"}.get(best, best.capitalize())

    @staticmethod
    def is_high(f):
        return bool(f.get("release")) or any(r[1] in HIGH for r in decisive(f["ratings"]))

    def run(self):
        wl = self.wl
        qt = wl["qt"]
        qt_tags = self.tags(qt["repo"])
        if tag_of(qt) not in qt_tags:
            raise SourceError(f"Qt's tag {tag_of(qt)} is not in {qt['repo']}")
        comps = []
        for c in wl["components"]:
            tags = self.tags(c["repo"])
            if tag_of(c) not in tags:
                raise SourceError(f"{c['name']}: the tag {tag_of(c)} is not in {c['repo']}")
            comps.append((c, tags))
        # OSV: the commits we ship, the Ubuntu packages, and the queries that must find something
        ub = wl["ubuntu"]
        queries, owners = [], []
        for c, tags in comps:
            if c.get("osv", True):
                queries.append({"commit": tags[tag_of(c)]})
                owners.append((self.part(c), self.group(c), c))
        for pkg, ver in sorted(ub["packages"].items()):
            queries.append({"package": {"name": pkg, "ecosystem": ub["ecosystem"]}, "version": ver})
            owners.append((f"{pkg} {ver} (Ubuntu, in the AppImage)", f"ubuntu: {pkg}", None))
        canaries = []
        for cq in wl.get("osv_canaries") or []:
            q = dict(cq)
            if "commit_of" in q:
                tags = next((t for c, t in comps if c["name"] == q["commit_of"]), {})
                if q["tag"] not in tags:
                    continue
                q = {"commit": tags[q["tag"]]}
            canaries.append(len(queries))
            queries.append(q)
            owners.append(None)
        results = self.osv_batch(queries)
        for i in canaries:
            if not results[i]:
                raise SourceError(f"OSV found nothing for a query that must find something ({queries[i]}): "
                                  "the query form may have changed")
        osv_count = {}
        for owner, res in zip(owners, results):
            if owner is None:
                continue
            osv_count[owner[0]] = len(res)
            for vid in res:
                self.osv_finding(vid, owner)
        # Qt, then each part's newer releases with their advisories and notes
        self.check_qt(qt_tags)
        for c, tags in comps:
            part = self.part(c)
            newer = newer_versions(c, tags)
            nx = self.github_extras(c, newer)
            mine = [k for k, f in self.findings.items() if part in f.get("parts", [])]
            top = [k for k in mine if self.is_high(self.findings[k])]
            self.say(f"{part}: newer releases {', '.join(newer[-4:]) if newer else 'none'}"
                     + (f" ({len(newer)} in all)" if len(newer) > 4 else "")
                     + (f"; OSV {osv_count[part]}" if part in osv_count else "")
                     + (f"; advisories and notes {nx}" if nx else "")
                     + (f"; findings {len(mine)}, High or Critical {len(top)}" if mine else "")
                     + (": " + ", ".join(f"{k} {self.rating(self.findings[k])}" for k in top) if top else ""))
        ub_parts = [o[0] for o in owners if o and o[2] is None]
        ub_keys = [k for k, f in self.findings.items() if set(f.get("parts", [])) & set(ub_parts)]
        self.say(f"Ubuntu packages in the AppImage ({ub['ecosystem']}): {len(ub_parts)} asked, "
                 f"{sum(osv_count.get(p, 0) for p in ub_parts)} known vulnerabilities, "
                 f"{sum(1 for k in ub_keys if self.is_high(self.findings[k]))} rated High or Critical")

    # ---- issues: one per upstream project (or Ubuntu package); later findings for it as comments while it is open
    def group_label(self, group):
        if group.startswith("ubuntu: "):
            return f"{group[8:]} (the Ubuntu package in the AppImage)"
        if group.startswith("release: "):
            return group[9:]
        for c in [self.wl["qt"]] + self.wl["components"]:
            if c["repo"] == group:
                return re.sub(r"\s*\(.*\)$", "", c["name"])
        return group

    def describe(self, part):
        for c in [self.wl["qt"]] + self.wl["components"]:
            if part == self.part(c):
                return f"{part} ({c['uses']})"
        return part

    def section(self, f):
        rat = "; ".join(f"{who}: {r.lower()} ({kind})" for who, r, kind, _ in f["ratings"]) or "none yet"
        aff = {"yes": "yes", "unclear": "not clear from the record (counted as affected)"}.get(f["affects"], f["affects"])
        return "\n".join([
            f"### {f['key']}: {self.rating(f)}",
            f"- **What:** {f['title'] or 'see the links'}",
            f"- **Parts:** {'; '.join(self.describe(p) for p in f['parts'])}",
            f"- **Ratings:** {rat}",
            f"- **The version we ship is affected:** {aff}",
            f"- **Fixed in:** {', '.join(f['fixed']) or 'not named'}",
            f"- **Found through:** {', '.join(f['how'])}",
            "- **Links:** " + " ".join(l for l in f["links"] if l),
            f"<!-- security-watch-key: {f['key']} -->",
            ""])

    FOOT = ("Made by the daily security watch (`.github/workflows/security-watch.yml`). It changes nothing by itself; "
            "whether and how to act is decided by hand. Rated by the maker of the part where it gives a rating "
            "(the CNA, the project's own advisory, Ubuntu for its packages), otherwise by CISA.")

    def issue_body(self, group, fs):
        if fs[0].get("release"):
            f = fs[0]
            return (f"Qt {f['version']} is out: the tag `{f['tag']}` is in Qt's source repository ({f['repo']}).\n\n"
                    "Watched because the next round of builds waits for this release.\n\n"
                    "Made by the daily security watch (`.github/workflows/security-watch.yml`); it changes nothing "
                    f"by itself.\n\n<!-- security-watch-key: {f['key']} -->\n<!-- security-watch-group: {group} -->\n")
        text = "\n".join(self.section(f) for f in fs)
        if len(text) > 60000:
            text = text[:60000] + "\n\n(cut: more in the run's summary)\n"
        return f"{text}\n{self.FOOT}\n\n<!-- security-watch-group: {group} -->\n"

    def issue_title(self, group, fs):
        if fs[0].get("release"):
            return f"Release watch: Qt {fs[0]['version']} is out"
        n = len(fs)
        return f"Security watch: {self.group_label(group)}: {n} finding{'s' if n != 1 else ''} rated High or Critical"

    def existing(self, repo):
        """(the keys already reported, {group: number of its open issue})."""
        keys, open_groups = set(), {}
        for page in range(1, 21):
            st, items = http_json(f"https://api.github.com/repos/{repo}/issues?labels={LABEL}&state=all&per_page=100"
                                  f"&page={page}", headers=gh_headers())
            if st != 200 or items is None:
                raise SourceError(f"the issues of {repo}: HTTP {st}")
            for it in items:
                body = it.get("body") or ""
                keys |= set(re.findall(r"security-watch-key: (.+?) -->", body))
                g = re.search(r"security-watch-group: (.+?) -->", body)
                if g and it.get("state") == "open":
                    open_groups[g.group(1)] = it["number"]
                    if it.get("comments"):
                        st, cs = http_json(f"https://api.github.com/repos/{repo}/issues/{it['number']}/comments"
                                           "?per_page=100", headers=gh_headers())
                        for c in cs or []:
                            keys |= set(re.findall(r"security-watch-key: (.+?) -->", c.get("body") or ""))
            if len(items) < 100:
                break
        return keys, open_groups

    def report_issues(self):
        repo = os.environ.get("GITHUB_REPOSITORY", "")
        high = [f for f in self.findings.values() if self.is_high(f)]
        groups = {}
        for f in high:
            groups.setdefault(f["group"], []).append(f)
        self.say(f"Findings rated High or Critical for a version we ship: {sum(1 for f in high if not f.get('release'))}"
                 + "".join(f"\n  {self.issue_title(g, fs)}: " + ", ".join(f["key"] for f in fs)
                           for g, fs in groups.items()))
        if not high:
            return
        if self.dry or not repo:
            self.say("(dry run: no issue written)")
            return
        known, open_groups = self.existing(repo)
        todo = {g: [f for f in fs if f["key"] not in known] for g, fs in groups.items()}
        todo = {g: fs for g, fs in todo.items() if fs}
        self.say(f"New: {sum(len(fs) for fs in todo.values())} in {len(todo)} group(s)")
        if not todo:
            return
        http(f"https://api.github.com/repos/{repo}/labels",
             {"name": LABEL, "color": "b60205", "description": "Found by the security watch (High or Critical)"},
             gh_headers())
        owner = os.environ.get("GITHUB_REPOSITORY_OWNER", "")
        for g, fs in todo.items():
            if g in open_groups:
                body = ("New findings rated High or Critical:\n\n" + "\n".join(self.section(f) for f in fs)
                        + f"\n{self.FOOT}\n")
                st, _ = http(f"https://api.github.com/repos/{repo}/issues/{open_groups[g]}/comments",
                             {"body": body}, gh_headers())
                if st != 201:
                    raise SourceError(f"the comment on #{open_groups[g]} was not written: HTTP {st}")
                self.say(f"  commented on #{open_groups[g]}: {', '.join(f['key'] for f in fs)}")
                continue
            issue = {"title": self.issue_title(g, fs), "body": self.issue_body(g, fs), "labels": [LABEL]}
            st, _ = http(f"https://api.github.com/repos/{repo}/issues", dict(issue, assignees=[owner] if owner else []),
                         gh_headers())
            if st == 422:
                st, _ = http(f"https://api.github.com/repos/{repo}/issues", issue, gh_headers())
            if st != 201:
                raise SourceError(f"the issue for {g} was not written: HTTP {st}")
            self.say(f"  written: {issue['title']}")

    def summary(self):
        p = os.environ.get("GITHUB_STEP_SUMMARY")
        if p:
            with open(p, "a", encoding="utf-8") as fh:
                fh.write("## Security watch\n\n```\n" + "\n".join(self.lines) + "\n```\n")


def main(argv):
    wl_path = "tools/security-watch.json"
    dry = "--dry-run" in argv
    if "--watchlist" in argv:
        wl_path = argv[argv.index("--watchlist") + 1]
    with open(wl_path, encoding="utf-8") as fh:
        wl = json.load(fh)
    w = Watch(wl, dry)
    w.say(f"Security watch {time.strftime('%Y-%m-%d %H:%M', time.gmtime())} UTC" + (" (dry run)" if dry else ""))
    try:
        w.run()
        w.report_issues()
    except SourceError as e:
        w.say(f"SOURCE FAILED: {e}")
        w.summary()
        return 1
    for x in w.warnings:
        w.say(f"warning: {x}")
    w.summary()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
