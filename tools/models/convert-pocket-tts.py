#!/usr/bin/env python3
"""Pocket TTS for the release models-5: the ungated Kyutai repository kyutai/pocket-tts-without-voice-cloning (CC BY 4.0)
made into the two GGUF files the AI engine loads (llama.cpp's architecture "pockettts": the text side in libllama, the
flow net and the codec in the mmproj), with the preset voices that carry a license put into the mmproj.

  python3 -I tools/models/convert-pocket-tts.py <language> <kyutai repo folder> <llama.cpp folder> <outdir>

<language>: en fr es pt it (the folder languages/<name> of the repository: english french spanish portuguese italian,
the 16-layer models). <kyutai repo folder> holds languages/<name>/{model.safetensors, tokenizer.model, tokenizer.json,
embeddings/*.safetensors}. <llama.cpp folder> is a checkout of tag b12808 (its convert_hf_to_gguf.py and gguf-py), run
with the Python of its requirements (torch, transformers, sentencepiece, gguf ...).

Only the voices of tools/models/pocket-voices.json are staged (their states are the only files the converter sees), and
juergen is refused by name. Writes pocket-tts-<language>.gguf and mmproj-pocket-tts-<language>.gguf and checks, by reading
them back, that the architecture is pockettts and that the mmproj holds exactly the staged voices.
"""
import json
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
FOLDERS = {"en": "english", "fr": "french", "es": "spanish", "pt": "portuguese", "it": "italian"}


def voices():
    with open(os.path.join(HERE, "pocket-voices.json"), encoding="utf-8") as fh:
        doc = json.load(fh)
    names = sorted(doc["voices"])
    for bad in doc["never"]:
        if bad in names:
            sys.exit(f"{bad} must never be in the list")
    return names


def run(cmd, env=None):
    print("+", " ".join(cmd), flush=True)
    subprocess.run(cmd, check=True, env=env)


def main():
    if len(sys.argv) != 5 or sys.argv[1] not in FOLDERS:
        sys.exit(__doc__)
    lang, repo, llama, out = sys.argv[1:]
    src = os.path.join(repo, "languages", FOLDERS[lang])
    names = voices()
    stage = os.path.join(out, f"stage-{lang}")
    shutil.rmtree(stage, ignore_errors=True)
    os.makedirs(os.path.join(stage, "embeddings"))
    for f in ("model.safetensors", "tokenizer.model", "tokenizer.json"):
        os.symlink(os.path.abspath(os.path.join(src, f)), os.path.join(stage, f))
    for n in names:
        p = os.path.join(src, "embeddings", n + ".safetensors")
        if not os.path.isfile(p):
            sys.exit(f"the voice {n} is not in {src}/embeddings")
        os.symlink(os.path.abspath(p), os.path.join(stage, "embeddings", n + ".safetensors"))
    if os.path.exists(os.path.join(stage, "embeddings", "juergen.safetensors")):
        sys.exit("juergen was staged")

    conv = os.path.join(llama, "convert_hf_to_gguf.py")
    model_out = os.path.abspath(os.path.join(out, f"pocket-tts-{lang}.gguf"))
    mmproj_out = os.path.abspath(os.path.join(out, f"mmproj-pocket-tts-{lang}.gguf"))
    env = dict(os.environ, PYTHONPATH=os.path.join(llama, "gguf-py"))
    run([sys.executable, conv, stage, "--outfile", model_out, "--outtype", "f16"], env)
    run([sys.executable, conv, stage, "--mmproj", "--outfile", mmproj_out, "--outtype", "f16"], env)
    shutil.rmtree(stage)

    sys.path.insert(0, os.path.join(llama, "gguf-py"))
    from gguf import GGUFReader

    r = GGUFReader(model_out)
    arch = bytes(r.fields["general.architecture"].parts[-1]).decode()
    if arch != "pockettts":
        sys.exit(f"{model_out}: architecture {arch}, expected pockettts")
    m = GGUFReader(mmproj_out)
    got = sorted(t.name[len("voice."):] for t in m.tensors if t.name.startswith("voice."))
    print(f"{lang}: {len(got)} voices in the mmproj: {', '.join(got)}")
    if got != names:
        sys.exit(f"voices differ: missing {sorted(set(names) - set(got))}, extra {sorted(set(got) - set(names))}")
    for p in (model_out, mmproj_out):
        print(f"{os.path.basename(p)}: {os.path.getsize(p):,} bytes")


if __name__ == "__main__":
    main()
