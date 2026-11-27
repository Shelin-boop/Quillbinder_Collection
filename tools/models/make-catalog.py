#!/usr/bin/env python3
"""The catalog of the release models-5 (format 1, src/core/AiCatalog.h) from tools/models/models-5.json.

  make-catalog.py build <built dir> <texts dir> <out dir> [--defs models-5.json] [--sources models-5-sources.json]
      the job "catalog": copies every file of every model into <out dir> under its asset name (a built file from
      <built dir>/<from>, a license text from <texts dir>; --without-pocket lists the Pocket TTS models unpublished), measures size and SHA-256, checks the pins of copied
      third-party files, writes <out dir>/catalog.json. A shared asset is written once. Any missing file stops it.
  make-catalog.py models-json <resources/ai/models.json> [--catalog catalog.json]
      writes the program's built-in catalog: the same entries; sizes and SHA-256 from the catalog.json of a run, else
      only from the pins (a file without a checksum makes its model "not published yet" in the program).

Texts (in <texts dir>): apache-2.0.txt, cc-by-4.0.txt, cc-by-sa-4.0.txt, cc0-1.0.txt. The text "pocket-voices" is made
here from tools/models/pocket-voices.json; "a+b" joins two texts.
"""
import argparse
import hashlib
import json
import math
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DROP_FILE_KEYS = ("from", "text", "pin")


def load(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def sha256_of(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def voices_text():
    doc = load(os.path.join(HERE, "pocket-voices.json"))
    lines = ["Preset voices of Pocket TTS in this model",
             "=========================================", "",
             "The model is Kyutai's Pocket TTS (Creative Commons Attribution 4.0). Each preset voice was made by Kyutai from the",
             "recording named below; the license is the one Kyutai's voice repository (huggingface.co/kyutai/tts-voices) states",
             "for that recording. CC-BY-4.0: attribution is required (the datasets: VCTK, Alba MacKenna's recordings); CC0-1.0:",
             "no conditions (Voice-Zero, Kyutai's voice donations and own recordings).", ""]
    for name, v in sorted(doc["voices"].items()):
        lines.append(f"{name:16s} {v['license']:10s} {v['source']}")
    lines += ["", "Voices of the original model that are not included: those without a license statement for their recording, and",
              "those whose recording is non-commercial. Do not use a voice to imitate a person without their consent."]
    return "\n".join(lines) + "\n"


def text_for(key, texts_dir):
    if key == "pocket-voices":
        return voices_text().encode("utf-8")
    parts = []
    for k in key.split("+"):
        with open(os.path.join(texts_dir, k + ".txt"), "rb") as fh:
            parts.append(fh.read())
    return (b"\n\n" + b"=" * 78 + b"\n\n").join(parts)


def pin_for(sources, pin):
    group, path = pin
    for f in sources["groups"][group]["files"]:
        if f["path"] == path:
            return f
    sys.exit(f"no pin for {group}/{path}")


def entry(model, files):
    out = {k: v for k, v in model.items() if k != "files"}
    out["files"] = files
    total = sum(f.get("size", 0) for f in files)
    if all("size" in f for f in files):
        out["sizeMB"] = max(1, math.ceil(total / (1024 * 1024)))
    return out


def build(args):
    defs, sources = load(args.defs), load(args.sources)
    os.makedirs(args.out, exist_ok=True)
    seen = {}
    models = []
    for m in defs["models"]:
        files = []
        if args.without_pocket and m["engine"] == "pocket-tts":
            # not made in this run: listed without size and SHA-256, which the program shows as "not published yet"
            models.append(entry(m, [{"name": f["name"], "asset": f["asset"]} for f in m["files"]]))
            continue
        for f in m["files"]:
            asset = f["asset"]
            dest = os.path.join(args.out, asset)
            if asset not in seen:
                if "from" in f:
                    src = os.path.join(args.built, f["from"])
                    if not os.path.isfile(src):
                        sys.exit(f"{m['id']}: the built file {f['from']} is missing")
                    shutil.copyfile(src, dest)
                elif "text" in f:
                    with open(dest, "wb") as fh:
                        fh.write(text_for(f["text"], args.texts))
                else:
                    sys.exit(f"{m['id']}: {f['name']} has neither 'from' nor 'text'")
                size, digest = os.path.getsize(dest), sha256_of(dest)
                if "pin" in f:
                    p = pin_for(sources, f["pin"])
                    if p["size"] != size or p.get("sha256") != digest:
                        sys.exit(f"{asset}: size or SHA-256 differs from the pin of {f['pin']}")
                if size <= 0:
                    sys.exit(f"{asset} is empty")
                if size >= 2 * 1024 ** 3:
                    sys.exit(f"{asset} is {size} bytes: a release asset may have less than 2 GiB")
                seen[asset] = (size, digest)
            size, digest = seen[asset]
            files.append({"name": f["name"], "asset": asset, "size": size, "sha256": digest})
        models.append(entry(m, files))
    catalog = {"format": 1, "release": defs["release"], "models": models}
    with open(os.path.join(args.out, "catalog.json"), "w", encoding="utf-8") as fh:
        json.dump(catalog, fh, ensure_ascii=False, indent=1)
        fh.write("\n")
    print(f"{len(models)} models, {len(seen)} files, {sum(s for s, _ in seen.values()):,} bytes")


def models_json(args):
    defs, sources = load(args.defs), load(args.sources)
    measured = {}
    if args.catalog:
        for m in load(args.catalog)["models"]:
            for f in m["files"]:
                measured[f["asset"]] = f
    models = []
    for m in defs["models"]:
        files = []
        for f in m["files"]:
            item = {"name": f["name"], "asset": f["asset"]}
            if f["asset"] in measured:
                item["size"], item["sha256"] = measured[f["asset"]]["size"], measured[f["asset"]]["sha256"]
            elif "pin" in f:
                p = pin_for(sources, f["pin"])
                item["size"], item["sha256"] = p["size"], p["sha256"]
            files.append(item)
        models.append(entry(m, files))
    with open(args.target, "w", encoding="utf-8") as fh:
        json.dump({"format": 1, "release": defs["release"], "models": models}, fh, ensure_ascii=False, indent=1)
        fh.write("\n")
    print(f"{args.target}: {len(models)} models")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--defs", default=os.path.join(HERE, "models-5.json"))
    ap.add_argument("--sources", default=os.path.join(HERE, "models-5-sources.json"))
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build")
    b.add_argument("built")
    b.add_argument("texts")
    b.add_argument("out")
    b.add_argument("--without-pocket", action="store_true", help="Pocket TTS was not made: its models are listed unpublished")
    j = sub.add_parser("models-json")
    j.add_argument("target")
    j.add_argument("--catalog")
    args = ap.parse_args()
    (build if args.cmd == "build" else models_json)(args)


if __name__ == "__main__":
    main()
