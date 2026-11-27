#!/usr/bin/env python3
"""Download one group of the third-party files of the release models-5 from Hugging Face at the pinned commit and check
every pin (size and SHA-256; a file without LFS: its git blob id).

  python3 -I tools/models/fetch-hf.py <sources.json> <group> <destdir>

The files keep their path below <destdir>. A file that is there with the right pin is not fetched again.
"""
import hashlib
import json
import os
import sys
import urllib.parse
import urllib.request


def git_blob_id(path):
    size = os.path.getsize(path)
    h = hashlib.sha1(f"blob {size}\0".encode())
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def matches(path, pin):
    if not os.path.isfile(path) or os.path.getsize(path) != pin["size"]:
        return False
    return sha256(path) == pin["sha256"] if "sha256" in pin else git_blob_id(path) == pin["gitBlob"]


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    sources, group, dest = sys.argv[1:]
    with open(sources, encoding="utf-8") as fh:
        g = json.load(fh)["groups"][group]
    for pin in g["files"]:
        target = os.path.join(dest, pin["path"])
        if matches(target, pin):
            continue
        os.makedirs(os.path.dirname(target), exist_ok=True)
        url = f"https://huggingface.co/{g['repo']}/resolve/{g['revision']}/{urllib.parse.quote(pin['path'])}"
        print("fetch", url, flush=True)
        req = urllib.request.Request(url, headers={"User-Agent": "quillbinder-models-5"})
        with urllib.request.urlopen(req, timeout=120) as resp, open(target + ".part", "wb") as out:
            while True:
                chunk = resp.read(1 << 20)
                if not chunk:
                    break
                out.write(chunk)
        os.replace(target + ".part", target)
        if not matches(target, pin):
            sys.exit(f"{pin['path']}: size or checksum differs from the pin")
    print(f"{group}: {len(g['files'])} files, all pins hold")


if __name__ == "__main__":
    main()
