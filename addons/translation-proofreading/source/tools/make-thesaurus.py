#!/usr/bin/env python3
"""Writes resources/en-thesaurus.tsv: words of a similar meaning for English words (1.1: the information area and a
single word in the input), and the irregular forms that lead back to them; since 1.3 also resources/en-glosses.tsv:
what each word means (WordNet's definitions, the same source and license).

    python3 tools/make-thesaurus.py [WordNet folder]     (default: /usr/share/wordnet, Ubuntu package wordnet-base)

Made from Princeton WordNet 3.0 (license: resources/en-thesaurus-LICENSE.txt). For every word: the other words of
its senses, most frequent sense first (WordNet's index order), by part of speech; for adjectives also the words of
the senses WordNet marks as similar. Lines:

    word<TAB>n:first;second|v:...|a:...|r:...      (n noun, v verb, a adjective, r adverb)
    @inflected<TAB>base[ base2]                     (irregular forms: "went" -> "go"; the regular ones are rules)

en-glosses.tsv (1.3): definitions (without WordNet's example sentences) once each, and for every word the definitions of
its senses by part of speech, the most frequent sense first:

    =<id><TAB>definition                           (id: a number in base 36, in order of first use)
    word<TAB>n:id,id|v:id|a:id|r:id
"""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
POS = [("noun", "n"), ("verb", "v"), ("adj", "a"), ("adv", "r")]
PER_POS = 16   # words kept per part of speech
GLOSSES = {}   # (data file, offset) -> definition


def base36(n):
    digits = "0123456789abcdefghijklmnopqrstuvwxyz"
    s = ""
    while True:
        n, r = divmod(n, 36)
        s = digits[r] + s
        if n == 0:
            return s


def clean(w):
    w = w.replace("_", " ")
    # adjective markers: "big(a)", "galore(ip)"
    if w.endswith(")") and "(" in w:
        w = w[: w.rindex("(")]
    return w


def gloss_of(line):
    """The definition of a data line: the text after " | " up to the first example sentence (a quotation after "; ")."""
    g = line.split(" | ", 1)[1].strip() if " | " in line else ""
    cut = g.find('; "')
    if cut >= 0:
        g = g[:cut]
    return g.replace("\t", " ").strip()


def read_data(path):
    """offset -> (words, similar-to offsets)"""
    out = {}
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.startswith("  "):
                continue
            head = line.split(" | ", 1)[0].split()
            offset, cnt = head[0], int(head[3], 16)
            words = [clean(head[4 + 2 * i]) for i in range(cnt)]
            i = 4 + 2 * cnt
            pcnt = int(head[i])
            similar = []
            for p in range(pcnt):
                sym, off, pos = head[i + 1 + 4 * p], head[i + 2 + 4 * p], head[i + 3 + 4 * p]
                if sym == "&" and pos in ("a", "s"):
                    similar.append(off)
            out[offset] = (words, similar)
            GLOSSES[(path, offset)] = gloss_of(line)
    return out


def main():
    wn = sys.argv[1] if len(sys.argv) > 1 else "/usr/share/wordnet"
    table = {}
    glosses = {}      # lemma -> [tag:id,id ...]
    ids = {}          # (data file, offset) -> id
    id_text = []
    for name, tag in POS:
        data = read_data(os.path.join(wn, f"data.{name}"))
        with open(os.path.join(wn, f"index.{name}"), encoding="utf-8", errors="replace") as f:
            for line in f:
                if line.startswith("  "):
                    continue
                parts = line.split()
                lemma = clean(parts[0])
                synsets = parts[-int(parts[2]):]
                syns = []
                sense_ids = []
                for off in synsets:
                    key = (os.path.join(wn, f"data.{name}"), off)
                    if key in GLOSSES and GLOSSES[key]:
                        if key not in ids:
                            ids[key] = len(id_text)
                            id_text.append(GLOSSES[key])
                        sense_ids.append(base36(ids[key]))
                if sense_ids:
                    glosses.setdefault(lemma, []).append(f"{tag}:" + ",".join(sense_ids))
                for off in synsets:
                    words, similar = data.get(off, ([], []))
                    group = list(words)
                    for s in similar:
                        group += data.get(s, ([], []))[0]
                    for w in group:
                        if w.lower() != lemma.lower() and w not in syns:
                            syns.append(w)
                if syns:
                    table.setdefault(lemma, []).append(f"{tag}:" + ";".join(syns[:PER_POS]))
    exceptions = {}
    for name, _ in POS:
        with open(os.path.join(wn, f"{name}.exc"), encoding="utf-8", errors="replace") as f:
            for line in f:
                p = line.split()
                if len(p) >= 2:
                    exceptions.setdefault(clean(p[0]), [])
                    for b in p[1:]:
                        if clean(b) not in exceptions[clean(p[0])]:
                            exceptions[clean(p[0])].append(clean(b))
    out = os.path.join(ROOT, "resources", "en-thesaurus.tsv")
    with open(out, "w", encoding="utf-8") as f:
        f.write("# Words of a similar meaning, from Princeton WordNet 3.0 (resources/en-thesaurus-LICENSE.txt); "
                "made by tools/make-thesaurus.py\n")
        for lemma in sorted(table):
            f.write(lemma + "\t" + "|".join(table[lemma]) + "\n")
        for w in sorted(exceptions):
            f.write("@" + w + "\t" + " ".join(exceptions[w]) + "\n")
    gout = os.path.join(ROOT, "resources", "en-glosses.tsv")
    with open(gout, "w", encoding="utf-8") as f:
        f.write("# What words mean, from Princeton WordNet 3.0 (resources/en-thesaurus-LICENSE.txt); "
                "made by tools/make-thesaurus.py\n")
        for i, text in enumerate(id_text):
            f.write("=" + base36(i) + "\t" + text + "\n")
        for lemma in sorted(glosses):
            f.write(lemma + "\t" + "|".join(glosses[lemma]) + "\n")
    print(f"{gout}: {len(glosses)} words, {len(id_text)} definitions, {os.path.getsize(gout)} bytes")
    print(f"{out}: {len(table)} words, {len(exceptions)} irregular forms, {os.path.getsize(out)} bytes")


if __name__ == "__main__":
    main()
