#!/usr/bin/env python3
"""The installed size of the full version's add-ons (0.94.0): the sum of the unpacked sizes of the files of every add-on
package that the full AppImage carries (one <id>.qbpkg per folder of addons/), in bytes, written as one number into a
file. tools/download-page.py reads it ("full-addons-size") and names the disk space on the Linux card.

    tools/full-addons-size.py <folder with the <id>.qbpkg files> <out file> [<addons folder of the source>]
"""
import os
import sys
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def unpacked_size(package):
    """The bytes of the files inside payload/ of a package (what the installation writes; the manifest is small)."""
    total = 0
    with zipfile.ZipFile(package) as z:
        for info in z.infolist():
            if info.filename.startswith("payload/") and not info.is_dir():
                total += info.file_size
    return total


def main(argv):
    if len(argv) < 3:
        print(__doc__, file=sys.stderr)
        return 2
    packages, out = argv[1], argv[2]
    addons = argv[3] if len(argv) > 3 else os.path.join(ROOT, "addons")
    ids = sorted(d for d in os.listdir(addons) if os.path.isdir(os.path.join(addons, d)))
    total = 0
    for i in ids:
        path = os.path.join(packages, i + ".qbpkg")
        if not os.path.isfile(path):
            print("full-addons-size.py: missing %s" % path, file=sys.stderr)
            return 1
        total += unpacked_size(path)
    with open(out, "w", encoding="utf-8") as f:
        f.write("%d\n" % total)
    print("the add-ons of the full version take %d bytes unpacked (%d packages)" % (total, len(ids)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
