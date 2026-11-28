#!/usr/bin/env python3
"""Suite models5 (0.99.0): the release models-5 (item 112 and 114), offline: the workflow (.github/workflows/models-5.yml),
the scripts and lists in tools/models/ and the program's built-in catalog (resources/ai/models.json) agree: ids, languages
(Read Aloud has none of id, tr, pl, nl; Handwriting none of ru), the licensed voices (never juergen), the pins, the rights
of the jobs, and a made-up run of the catalog step.

  python3 -I tests/models5_tests.py
"""
import gzip
import math
import struct
import wave
import zipfile
import importlib.util
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
M = os.path.join(ROOT, "tools", "models")
FAILS = 0
COUNT = 0


def check(name, ok, detail=""):
    global FAILS, COUNT
    COUNT += 1
    print(("PASS " if ok else "FAIL ") + name + ("" if ok else f": {detail}"))
    if not ok:
        FAILS += 1


def read(path):
    with open(path, encoding="utf-8") as fh:
        return fh.read()


def jload(path):
    return json.loads(read(path))


def module(name, file):
    spec = importlib.util.spec_from_file_location(name, os.path.join(M, file))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


defs = jload(os.path.join(M, "models-5.json"))
sources = jload(os.path.join(M, "models-5-sources.json"))
voices_doc = jload(os.path.join(M, "pocket-voices.json"))
built_in = jload(os.path.join(ROOT, "resources", "ai", "models.json"))
wf_text = read(os.path.join(ROOT, ".github", "workflows", "models-5.yml"))
models = defs["models"]
by_id = {m["id"]: m for m in models}

# ---- the definitions and the program's catalog
check("definitions: release models-5, 19 models and 7 voice packs, unique ids", defs["release"] == "models-5" and len(models) == 26 and len(by_id) == 26)
check("models.json: format 1, release models-5", built_in["format"] == 1 and built_in["release"] == "models-5")
check("models.json has the same ids in the same order as the definitions", [m["id"] for m in built_in["models"]] == [m["id"] for m in models])
kinds = {k for m in models for k in m["kinds"]}
check("kinds are handwriting, wordlist, tts and voicepack", kinds == {"handwriting", "wordlist", "tts", "voicepack"}, kinds)
check("engines are onnx, pocket-tts, qwen3-tts or none (word lists)", {m["engine"] for m in models} == {"onnx", "pocket-tts", "qwen3-tts", ""})
for m in models:
    names = [f["name"] for f in m["files"]]
    ok = (m["model"] in names and (not m.get("extra") or m["extra"] in names) and len(set(names)) == len(names)
          and all(re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._+-]*", n) for n in names + [f["asset"] for f in m["files"]]))
    check(f"{m['id']}: model/extra are files, names are safe", ok)
    check(f"{m['id']}: every file has a source (from, text, or external: made by the voices run)", all(("from" in f) + ("text" in f) + ("external" in f) == 1 for f in m["files"]))
    check(f"{m['id']}: license fields", bool(m["license"]) and m["licenseUrl"].startswith("https://") and bool(m["credit"]) and bool(m["source"]))

# ---- languages (decision 6)
ELEVEN = {"en", "de", "es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"}
check("no entry lists a language outside the program's eleven (no zh, ja, ko)",
      all(set(m["languages"]) <= ELEVEN for m in models + built_in["models"]), {l for m in models for l in m["languages"]} - ELEVEN)
tts = {l for m in models if "tts" in m["kinds"] for l in m["languages"]}
hw = {l for m in models if "handwriting" in m["kinds"] for l in m["languages"]}
wl = sorted(l for m in models if "wordlist" in m["kinds"] for l in m["languages"])
check("Read Aloud: no id, tr, pl, nl", not ({"id", "tr", "pl", "nl"} & tts), tts)
check("Read Aloud: en de es fr pt it ru are there", {"en", "de", "es", "fr", "pt", "it", "ru"} <= tts)
check("Qwen3-TTS lists exactly the seven languages it serves", all(set(m["languages"]) == {"en", "de", "es", "fr", "pt", "it", "ru"} for m in models if m["engine"] == "qwen3-tts"))
check("Handwriting: no ru, the other nine and en", "ru" not in hw and hw == {"en", "de", "es", "fr", "pt", "it", "id", "tr", "pl", "nl"}, hw)
check("word lists: en de es fr pt it pl nl id tr, not ru", wl == sorted("en de es fr pt it pl nl id tr".split()), wl)
check("Pocket TTS: en fr es pt it, one model each", sorted(m["languages"][0] for m in models if m["engine"] == "pocket-tts") == sorted("en fr es pt it".split()))
rec = module("convert_ppocr", "convert-ppocr.py")
check("the program's Handwriting languages (Recognizer.cpp) are the model's",
      set(re.findall(r'QStringLiteral\("([a-z]{2})"\)', read(os.path.join(ROOT, "src/ui/handwriting/Recognizer.cpp")).split("modelLanguages()")[1].split("}")[0])) == hw)

# ---- the voices
v = voices_doc["voices"]
check("voices: never juergen, none of the unlicensed or non-commercial ones",
      "juergen" in voices_doc["never"] and not ({"juergen", "cosette", "jean", "giovanni", "lola", "rafael", "daan"} & set(v)))
check("voices: every voice has CC-BY-4.0 or CC0-1.0 and a source", all(x["license"] in ("CC-BY-4.0", "CC0-1.0") and x["source"] for x in v.values()))
check("voices: alba and michael CC-BY-4.0, estelle CC0-1.0", v["alba"]["license"] == v["michael"]["license"] == "CC-BY-4.0" and v["estelle"]["license"] == "CC0-1.0")
check("every Pocket TTS entry lists exactly these voices", all(m["voices"] == sorted(v) for m in models if m["engine"] == "pocket-tts"))
check("juergen is in no definition, pin or catalog file", not any("juergen" in read(p).lower() for p in (os.path.join(M, "models-5.json"), os.path.join(M, "models-5-sources.json"), os.path.join(ROOT, "resources/ai/models.json"), os.path.join(ROOT, ".github/workflows/models-5.yml"))))

# ---- the pins
groups = sources["groups"]
check("pins: groups ppocr, qwen, qwen-voicedesign and pocket-<lang> for the five languages", set(groups) == {"ppocr", "qwen", "qwen-voicedesign", "pocket-en", "pocket-fr", "pocket-es", "pocket-pt", "pocket-it"})
check("pins: a 40-hex commit for every group", all(re.fullmatch(r"[0-9a-f]{40}", g["revision"]) for g in groups.values()))
check("pins: every file has a size and a SHA-256 (or a git blob id for a file without LFS)",
      all(f["size"] > 0 and (re.fullmatch(r"[0-9a-f]{64}", f.get("sha256", "")) or re.fullmatch(r"[0-9a-f]{40}", f.get("gitBlob", ""))) for g in groups.values() for f in g["files"]))
for l in ("en", "fr", "es", "pt", "it"):
    paths = {f["path"].rsplit("/", 1)[-1] for f in groups["pocket-" + l]["files"]}
    check(f"pins pocket-{l}: model, tokenizer and exactly the licensed voices",
          paths == {"model.safetensors", "tokenizer.model", "tokenizer.json"} | {n + ".safetensors" for n in v})
check("pins: the four Qwen files (0.6B Q8_0, 1.7B Q4_K_M, 1.7B Q8_0, tokenizer Q8_0)",
      sorted(f["path"] for f in groups["qwen"]["files"]) == sorted(["qwen-talker-0.6b-base-Q8_0.gguf", "qwen-talker-1.7b-base-Q4_K_M.gguf", "qwen-talker-1.7b-base-Q8_0.gguf", "qwen-tokenizer-12hz-Q8_0.gguf"]))
check("pins: the largest asset is under 2 GiB", max(f["size"] for f in groups["qwen"]["files"]) < 2 * 1024 ** 3)
qwen_pins = {f["path"]: f for f in groups["qwen"]["files"]}
for m in built_in["models"]:
    if m["engine"] == "qwen3-tts":
        mine = {f["name"]: f for f in m["files"]}
        check(f"models.json {m['id']}: talker and tokenizer carry the pinned size and SHA-256",
              all(any(p["size"] == mine[n]["size"] and p["sha256"] == mine[n]["sha256"] for p in qwen_pins.values()) for n in ("talker.gguf", "tokenizer-12hz-Q8_0.gguf")))
published = jload(os.path.join(M, "models-5-catalog.json"))
check("the published release's catalog (models-5-catalog.json) lists the same ids and files as the definitions",
      [(m["id"], [f["asset"] for f in m["files"]]) for m in published["models"] if "voicepack" not in m["kinds"]] == [(m["id"], [f["asset"] for f in m["files"]]) for m in models if "voicepack" not in m["kinds"]])
check("models.json: every file has its size and a 64-hex SHA-256 from the published release",
      all(len(f.get("sha256", "")) == 64 and f.get("size", 0) > 0 for m in built_in["models"] if "voicepack" not in m["kinds"] for f in m["files"]))
# THE PLACEHOLDER CHECK: it fails until the lead has run the job "voices" and put the measured size and SHA-256 of the seven
# packs into resources/ai/models.json (python3 tools/models/make-catalog.py models-json resources/ai/models.json --catalog <catalog.json of the release>)
packs = [m for m in built_in["models"] if "voicepack" in m["kinds"]]
unfilled = [m["id"] for m in packs if not all(len(f.get("sha256", "")) == 64 and f.get("size", 0) > 0 for f in m["files"])]
check("voice packs: seven of them, and none with a placeholder for its size or SHA-256 (fill them after the run of the job voices: " + ", ".join(unfilled) + ")",
      len(packs) == 7 and sorted(m["languages"][0] for m in packs) == ["de", "en", "es", "fr", "it", "pt", "ru"] and not unfilled)

# ---- the workflow
check("workflow: release models-5, llama.cpp b12808 with its commit", "RELEASE_TAG: models-5" in wf_text and "LLAMA_TAG: b12808" in wf_text and "ed2153e708de813c35559102aed43b3085179c27" in wf_text)
check("workflow: pushes of linux/** that change it or tools/models, and a dispatch", "'linux/**'" in wf_text and "'tools/models/**'" in wf_text and "workflow_dispatch:" in wf_text)
jobs = re.findall(r"^  ([a-z-]+):\n", wf_text.split("\njobs:\n", 1)[1], re.M)
check("workflow: the jobs", jobs == ["state", "handwriting", "wordlists", "pocket-tts", "qwen", "voices", "voices-catalog", "catalog", "publish", "voices-publish"], jobs)
check("workflow: contents: write only in the jobs publish and voices-publish", wf_text.count("contents: write") == 2 and wf_text.index("contents: write") > wf_text.index("\n  publish:\n"))
check("workflow: a published release is never changed (no --clobber), catalog.json is uploaded last",
      "--clobber" not in wf_text.split("\n  voices-publish:\n")[0].replace("# no --clobber", "") and wf_text.rindex("out/catalog.json") > wf_text.rindex('gh release upload "$RELEASE_TAG" "out/$f"'))
check("workflow: does nothing when catalog.json is on the release", "grep -qxF catalog.json" in wf_text and "needs.state.outputs.there != 'true'" in wf_text)
check("workflow: the voice jobs run only by the input voices; every other job is switched off by it", "inputs.voices" in wf_text and "default: false" in wf_text and 'VOICES: ${{ github.event_name' in wf_text)
vp = wf_text.split("\n  voices-publish:\n")[1]
vj = wf_text.split("\n  voices:\n")[1].split("\n  voices-catalog:\n")[0]
check("workflow voices: seven languages in a matrix, the repository guard, no write permission, pinned sources",
      "lang: [en, de, es, fr, pt, it, ru]" in vj and "github.repository == 'Shelin-boop/Quillbinder_Collection'" in vj and "contents: write" not in vj
      and "fetch-hf.py tools/models/models-5-sources.json qwen-voicedesign" in vj and "get-ai-engine.sh" in vj)
check("workflow voices-publish: only adds zip files and replaces catalog.json (nothing is deleted), no checkout",
      "gh release upload" in vp and "gh release delete" not in vp and "gh release edit" not in vp and "actions/checkout" not in vp and "out/catalog.json" in vp)
check("workflow: every action is pinned by a commit", all(re.fullmatch(r"[0-9a-f]{40}", m) for m in re.findall(r"uses: [^@\s]+@(\S+)", wf_text)))
check("workflow: marked a pre-release 'not for downloading'", "--prerelease" in wf_text and "not for downloading" in wf_text)
translator = read(os.path.join(ROOT, ".github/workflows/translator-data-4.yml"))
pinned_actions = set(re.findall(r"uses: (actions/[a-z-]+@[0-9a-f]{40})", wf_text))
check("workflow: every action is pinned to a commit that translator-data-4.yml uses too", pinned_actions and all(a in translator for a in pinned_actions), pinned_actions)
check("workflow: the publish job runs no repository script (no checkout)", "actions/checkout" not in wf_text.split("\n  publish:\n", 1)[1])
check("workflow: wordfreq 3.1.1, kraken 7.2 without dependencies, the converter's requirements", 'wordfreq==3.1.1' in wf_text and 'kraken==7.2' in wf_text and "--no-deps" in wf_text and "requirements-convert_hf_to_gguf.txt" in wf_text)
arts = set(re.findall(r"name: (m5-[a-z${}. -]+)", wf_text))
used = {f["from"].split("/")[0] for m in models for f in m["files"] if "from" in f}
expanded = set()
for a in arts:
    if "matrix.lang" in a:
        expanded |= {"m5-pocket-" + l for l in ("en", "fr", "es", "pt", "it")}
    else:
        expanded.add(a)
check("workflow: every artifact the definitions read from is uploaded by a job", used <= expanded, used - expanded)
# the first run failed here: a conversion that failed inside a pipe to tee did not stop its step, and the artifact lacked the files
for step in ("convert-ppocr.py", "convert-pocket-tts.py"):
    i = wf_text.index("python -I tools/models/" + step)
    block = wf_text[wf_text.rindex("run: |", 0, i):i]
    check(f"workflow: the step with {step} stops when the script fails (pipefail before the pipe to tee)", "set -o pipefail" in block)
for m in models:
    for f in m["files"]:
        if f.get("from", "").startswith(("m5-handwriting/", "m5-pocket-")):
            art, name = f["from"].split("/")
            lang = art.rsplit("-", 1)[-1]
            expect = f"test -s out/{name}" if art == "m5-handwriting" else f"test -s out/{name.replace('-' + lang + '.', '-${{ matrix.lang }}.')}"
            check(f"workflow: the job that makes {f['from']} tests that out/{name} exists before it uploads", expect in wf_text, expect)
check("workflow: the converter's protobuf is raised for sentencepiece (7.36.1 or newer: sentencepiece's model file was made with it)", 'protobuf>=7.36.1' in wf_text)
check("convert-ppocr: widths are padded to a multiple of 8 and the pooling kernel is made a plain number",
      "WIDTH_MULTIPLE = 8" in read(os.path.join(M, "convert-ppocr.py")) and "fixed_pool" in read(os.path.join(M, "convert-ppocr.py")))
check("workflow: the sources file names the same Pocket TTS languages as the matrix", "lang: [en, fr, es, pt, it]" in wf_text)

# ---- the scripts' small parts
wlm = module("make_wordlists", "make-wordlists.py")
check("word list filter: letters, inner apostrophe and hyphen pass", all(wlm.good(w) for w in ("house", "don't", "l’homme", "peut-être", "straße", "şüphe")))
check("word list filter: digits, URLs, other scripts, empty fail", not any(wlm.good(w) for w in ("abc123", "www.x.com", "月", "мир", "", "a_b", "-x")))
check("word list languages: the ten, no ru", wlm.LANGUAGES == ["en", "de", "es", "fr", "pt", "it", "pl", "nl", "id", "tr"])
tmp = tempfile.mkdtemp(prefix="models5-")
try:
    a, b = os.path.join(tmp, "a.gz"), os.path.join(tmp, "b.gz")
    rows = [("the", 7.73), ("und", 7.5)]
    wlm.write_gz(a, rows)
    wlm.write_gz(b, rows)
    check("word list file: same input, same bytes; lines word TAB zipf", open(a, "rb").read() == open(b, "rb").read()
          and gzip.open(a, "rt", encoding="utf-8").read() == "the\t7.73\nund\t7.50\n")
    codec = rec.Codec({" ": [1], "a": [2], "b": [3], "x́": [4, 5], "́": [6]})
    check("codec: single labels, a multi-label grapheme, a combining mark", codec.decode([2, 1, 3, 4, 5, 6]) == "a bx́́")
    check("codec: an unknown label is skipped", codec.decode([2, 99, 3]) == "ab")
    check("convert-ppocr: input 96 px, padding 16, 1623 classes, marks kept", rec.EXPECTED_CLASSES == 1623 and rec.KEEP_MARKS == [0x300, 0x301, 0x302, 0x303, 0x308, 0x327]
          and 'default=96' in read(os.path.join(M, "convert-ppocr.py")) and 'default=16' in read(os.path.join(M, "convert-ppocr.py")))
    check("convert-ppocr: fixes the height, checks 0.001, NFC", "dynamo=False" in read(os.path.join(M, "convert-ppocr.py")) and "max-difference\", type=float, default=0.001" in read(os.path.join(M, "convert-ppocr.py")) and "NFC" in read(os.path.join(M, "convert-ppocr.py")))
    pk = read(os.path.join(M, "convert-pocket-tts.py"))
    check("convert-pocket-tts: stages only the listed voices and refuses juergen", "pocket-voices.json" in pk and "juergen was staged" in pk and 'doc["never"]' in pk)

    # ---- a made-up run of the catalog step
    mc = module("make_catalog", "make-catalog.py")
    built, texts, out = (os.path.join(tmp, d) for d in ("built", "texts", "out"))
    os.makedirs(texts)
    for t in ("apache-2.0", "cc-by-4.0", "cc-by-sa-4.0", "cc0-1.0"):
        open(os.path.join(texts, t + ".txt"), "w").write(t + " text\n")
    fake_sources = json.loads(json.dumps(sources))
    for m in models:
        for f in m["files"]:
            if "from" in f:
                p = os.path.join(built, f["from"])
                os.makedirs(os.path.dirname(p), exist_ok=True)
                if not os.path.exists(p):
                    open(p, "wb").write(f["from"].encode() * 3)
    for pin in fake_sources["groups"]["qwen"]["files"]:
        p = os.path.join(built, "m5-qwen", "qwen", pin["path"])
        pin["size"], pin["sha256"] = os.path.getsize(p), mc.sha256_of(p)
    fs = os.path.join(tmp, "sources.json")
    json.dump(fake_sources, open(fs, "w"))
    r = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "--sources", fs, "build", built, texts, out], capture_output=True, text=True)
    check("make-catalog build: runs on made-up files", r.returncode == 0, r.stdout + r.stderr)
    cat = jload(os.path.join(out, "catalog.json")) if r.returncode == 0 else {"models": []}
    check("catalog: 19 models, every file with size and a 64-hex SHA-256 and its asset on disk",
          len(cat["models"]) == 19 and all("voicepack" not in x["kinds"] for x in cat["models"]) and all(len(f["sha256"]) == 64 and os.path.getsize(os.path.join(out, f["asset"])) == f["size"] for x in cat["models"] for f in x["files"]))
    check("catalog: the shared tokenizer is one asset", len({f["asset"] for x in cat["models"] for f in x["files"] if f["name"].startswith("tokenizer")}) == 1)
    voices_txt = open(os.path.join(out, "pocket-tts-en--VOICES.txt"), encoding="utf-8").read() if r.returncode == 0 else ""
    check("VOICES.txt names every voice with its license, and no juergen", all(n in voices_txt for n in v) and "juergen" not in voices_txt.lower())
    check("licenses: Pocket TTS gets CC BY and CC0 texts joined", open(os.path.join(out, "pocket-tts-fr--LICENSES.txt")).read().count("text") >= 2 if r.returncode == 0 else False)
    bad = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "--sources", os.path.join(M, "models-5-sources.json"), "build", built, texts, os.path.join(tmp, "out2")], capture_output=True, text=True)
    check("make-catalog build: a file that differs from its pin stops it", bad.returncode != 0 and "differs from the pin" in bad.stderr + bad.stdout)
    # the release without Pocket TTS: its five models are listed unpublished, the rest is complete
    shutil.rmtree(os.path.join(built, "m5-pocket-en"))
    r5 = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "--sources", fs, "build", built, texts, os.path.join(tmp, "out3"), "--without-pocket"], capture_output=True, text=True)
    c3 = jload(os.path.join(tmp, "out3", "catalog.json")) if r5.returncode == 0 else {"models": []}
    pk = [x for x in c3["models"] if x["engine"] == "pocket-tts"]
    check("make-catalog --without-pocket: five Pocket TTS models without checksums, every other file measured",
          len(pk) == 5 and all("sha256" not in f for x in pk for f in x["files"]) and all("sha256" in f for x in c3["models"] if x["engine"] != "pocket-tts" for f in x["files"]), r5.stderr)
    check("workflow: the dispatch input pocket skips Pocket TTS and the catalog still runs (handwriting, word lists, Qwen needed)",
          "inputs.pocket" in wf_text and "pocket-tts.result == 'skipped'" in wf_text and "--without-pocket" in wf_text)
    # ---- the voices run: the packs are added to the release's catalog
    zd = os.path.join(tmp, "zips")
    os.makedirs(zd)
    for l in ("en", "de"):
        open(os.path.join(zd, f"read-aloud-voices-{l}.zip"), "wb").write(b"PK" + l.encode() * 50)
    rv = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "voices", os.path.join(out, "catalog.json"), zd, os.path.join(tmp, "withvoices.json")], capture_output=True, text=True)
    cv = jload(os.path.join(tmp, "withvoices.json")) if rv.returncode == 0 else {"models": []}
    vp_models = [x for x in cv["models"] if "voicepack" in x["kinds"]]
    check("make-catalog voices: adds the packs whose zip is there with size and SHA-256, every other entry stays as it was",
          rv.returncode == 0 and [x["id"] for x in vp_models] == ["voices-en", "voices-de"] and all(len(x["files"][0]["sha256"]) == 64 and x["files"][0]["size"] > 0 for x in vp_models)
          and cv["models"][:len(cat["models"])] == cat["models"], rv.stderr)
    rv2 = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "voices", os.path.join(tmp, "withvoices.json"), os.path.join(tmp, "nozips"), os.path.join(tmp, "x.json")], capture_output=True, text=True)
    check("make-catalog voices: without any zip it stops", rv2.returncode != 0)
    target = os.path.join(tmp, "models.json")
    r2 = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "models-json", target, "--catalog", os.path.join(out, "catalog.json")], capture_output=True, text=True)
    merged = jload(target) if r2.returncode == 0 else {"models": []}
    check("models-json with a catalog: every file has its measured checksum (a voice pack not measured yet: size 0 and an empty SHA-256)",
          r2.returncode == 0 and all("sha256" in f for x in merged["models"] for f in x["files"]) and all(f["size"] == 0 and f["sha256"] == "" for x in merged["models"] if "voicepack" in x["kinds"] for f in x["files"]))
    r2b = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "models-json", os.path.join(tmp, "m4.json"), "--catalog", os.path.join(tmp, "withvoices.json")], capture_output=True, text=True)
    m4 = {x["id"]: x for x in jload(os.path.join(tmp, "m4.json"))["models"]} if r2b.returncode == 0 else {}
    check("models-json with the voices run's catalog: the measured packs are filled in, the others stay placeholders",
          m4.get("voices-de", {}).get("files", [{}])[0].get("size", 0) > 0 and len(m4["voices-de"]["files"][0]["sha256"]) == 64 and m4["voices-fr"]["files"][0]["sha256"] == "")

    # ---- the designing script: texts, descriptions, and the clip check on made-up WAV files
    dv = module("design_voices", "design-voices.py")
    check("voices: six voices with the written descriptions and three styles, 18 clips", len(dv.names()) == 18 and dv.VOICES["male1"] == "Male, adult, deep warm baritone voice"
          and dv.STYLES["action"] == "energetic, urgent, fast pace, tense and intense" and dv.description("female2-calm") == "Female, middle-aged, lower pitch, slightly husky voice, calm and soothing, soft, slow relaxed pace")
    check("voices: one reference text of 25 to 35 words for each of the seven languages", sorted(dv.TEXTS) == ["de", "en", "es", "fr", "it", "pt", "ru"] and all(25 <= len(t.split()) <= 35 for t in dv.TEXTS.values()))
    def wavfile(name, seconds, amp, rate=24000, clip=False, width=2, channels=1):
        n = int(seconds * rate)
        samples = [int(amp * 32767 * math.sin(i * 0.3)) for i in range(n)]
        if clip:
            samples[1000:1020] = [32767] * 20
        path = os.path.join(tmp, name)
        with wave.open(path, "wb") as w:
            w.setnchannels(channels)
            w.setsampwidth(width)
            w.setframerate(rate)
            w.writeframes(b"".join(struct.pack("<h", x) for x in samples) if channels == 1 else b"\0" * (n * 2 * channels))
        return path
    check("clip check: a good clip passes", dv.clip_problem(wavfile("good.wav", 8, 0.3)) is None)
    check("clip check: too short, too long, silent, clipped, wrong rate are refused",
          all(dv.clip_problem(wavfile(n, sec, a, rate=r, clip=c)) for n, sec, a, r, c in
              (("short.wav", 2, 0.3, 24000, False), ("long.wav", 25, 0.3, 24000, False), ("silent.wav", 8, 0.0005, 24000, False), ("clipped.wav", 8, 0.3, 24000, True), ("rate.wav", 8, 0.3, 16000, False))))
    check("clip check: the command line says OK or the reason and its exit code follows",
          subprocess.run([sys.executable, "-I", os.path.join(M, "design-voices.py"), "check", os.path.join(tmp, "good.wav")], capture_output=True, text=True).returncode == 0
          and subprocess.run([sys.executable, "-I", os.path.join(M, "design-voices.py"), "check", os.path.join(tmp, "silent.wav")], capture_output=True, text=True).returncode == 1)
    # the whole make step with a fake tool that writes the clips (one bad clip the first time, good the second time)
    fake = os.path.join(tmp, "faketool.py")
    open(fake, "w").write("import sys, wave, struct, math, os\n"
                          "lst, out, seed = sys.argv[4], sys.argv[5], int(sys.argv[6])\n"
                          "for line in open(lst, encoding='utf-8'):\n"
                          "    name = line.split('\\t')[0]\n"
                          "    amp = 0.0005 if (name == 'male3-calm' and seed == 1000) else 0.3\n"
                          "    w = wave.open(os.path.join(out, name + '.wav'), 'wb'); w.setnchannels(1); w.setsampwidth(2); w.setframerate(24000)\n"
                          "    w.writeframes(b''.join(struct.pack('<h', int(amp * 32767 * math.sin(i * 0.3))) for i in range(24000 * 6))); w.close()\n")
    wrapper = os.path.join(tmp, "tool.sh")
    open(wrapper, "w").write(f'#!/bin/sh\nexec {sys.executable} {fake} "$@"\n')
    os.chmod(wrapper, 0o755)
    mo = os.path.join(tmp, "made")
    rm = subprocess.run([sys.executable, "-I", os.path.join(M, "design-voices.py"), "make", "de", wrapper, "t.gguf", "c.gguf", mo], capture_output=True, text=True)
    zp = os.path.join(mo, "read-aloud-voices-de.zip")
    zn = zipfile.ZipFile(zp).namelist() if rm.returncode == 0 else []
    check("make: a failed clip is designed again once; the zip holds the 36 files and voices.json; the clip texts are the language's",
          rm.returncode == 0 and "male3-calm: nearly silent" in rm.stdout and len(zn) == 37 and "voices.json" in zn and "female1-neutral.txt" in zn
          and open(os.path.join(mo, "de", "female1-neutral.txt"), encoding="utf-8").read().strip() == dv.TEXTS["de"], rm.stdout[-400:] + rm.stderr[-400:])
    vj_meta = json.loads(zipfile.ZipFile(zp).read("voices.json")) if rm.returncode == 0 else {}
    check("make: voices.json names the model, its license, the descriptions and says no real person's voice",
          vj_meta.get("modelLicense") == "Apache-2.0" and "voicedesign" in vj_meta.get("model", "") and "no real person's voice" in vj_meta.get("note", "") and vj_meta["voices"]["female3"]["description"] == dv.VOICES["female3"])
    open(fake, "a").write("    if name == 'male3-calm': os.remove(os.path.join(out, name + '.wav'))\n")
    rm2 = subprocess.run([sys.executable, "-I", os.path.join(M, "design-voices.py"), "make", "en", wrapper, "t.gguf", "c.gguf", os.path.join(tmp, "made2")], capture_output=True, text=True)
    check("make: a clip that fails twice fails the run", rm2.returncode != 0 and "failed twice" in rm2.stdout + rm2.stderr)
    target = os.path.join(tmp, "models.json")
    r3 = subprocess.run([sys.executable, "-I", os.path.join(M, "make-catalog.py"), "models-json", os.path.join(tmp, "m3.json"), "--catalog", os.path.join(M, "models-5-catalog.json")], capture_output=True, text=True)
    check("models-json with the published release's catalog equals the committed built-in catalog", r3.returncode == 0 and jload(os.path.join(tmp, "m3.json")) == built_in, r3.stderr)
finally:
    shutil.rmtree(tmp, ignore_errors=True)

print(f"{FAILS} failure(s)")
sys.exit(1 if FAILS else 0)
