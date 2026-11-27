#!/usr/bin/env python3
"""The word lists of the release models-5 (for the word layer of Handwriting): wordfreq's most frequent words of a
language with their Zipf frequency, one gzip'd TSV per language, and the license and attribution texts beside them.

  python3 -I tools/models/make-wordlists.py <outdir> [--top 100000] [--lang en --lang de ...]

Needs wordfreq 3.1.1 (code Apache License 2.0, data Creative Commons Attribution-ShareAlike 4.0) with its dependencies.
Languages: en de es fr pt it pl nl (wordfreq has a large list for each) and id tr (a small list only: as many words
as it has). Not ru: Handwriting has no line model for Cyrillic.

Files (all names start with "wordlist-"):
  wordlist-<lang>.tsv.gz   lines "<word><TAB><zipf>", most frequent first, UTF-8, Zipf with two decimals;
                           made with a fixed time stamp, so that the same input gives the same bytes
  wordlist-ATTRIBUTION.txt what wordfreq asks to be named (its README's license and citation parts), and what this is
  wordlist-LICENSE-CC-BY-SA-4.0.txt   (only when --legal gives the text; the catalog job adds the license text itself)
"""
import argparse
import gzip
import importlib.metadata
import os
import re
import sys
import unicodedata

LANGUAGES = ["en", "de", "es", "fr", "pt", "it", "pl", "nl", "id", "tr"]
SELFTEST_WORDS = {"en": "the", "de": "und", "es": "que", "fr": "les", "pt": "que", "it": "che", "pl": "nie", "nl": "het",
                  "id": "yang", "tr": "bir"}
WORD = re.compile(r"^[^\W\d_]+(?:['’-][^\W\d_]+)*$")   # letters, with an inner apostrophe or hyphen


def good(word):
    """A word the word layer can match against a line's reading: letters only (no digits, no punctuation, no URLs)."""
    if not WORD.match(word) or len(word) > 32:
        return False
    # all ten languages are written in the Latin alphabet: no other script (wordfreq's lists hold some)
    return all(c in "'\u2019-" or unicodedata.name(c, "").startswith(("LATIN", "COMBINING")) for c in word)


def make_list(lang, top):
    from wordfreq import top_n_list, zipf_frequency

    rows, seen = [], set()
    # ask for more than needed: tokens with digits and the like are dropped
    for word in top_n_list(lang, top * 2, wordlist="best"):
        word = unicodedata.normalize("NFC", word)
        if word in seen or not good(word):
            continue
        seen.add(word)
        rows.append((word, zipf_frequency(word, lang, wordlist="best")))
        if len(rows) >= top:
            break
    return rows


def write_gz(path, rows):
    data = "".join(f"{w}\t{z:.2f}\n" for w, z in rows).encode("utf-8")
    with open(path, "wb") as raw:
        with gzip.GzipFile(filename="", mode="wb", fileobj=raw, mtime=0, compresslevel=9) as gz:
            gz.write(data)


def readme_part(text, start, stop):
    a = text.index(start)
    b = text.index(stop, a) if stop else len(text)
    return text[a:b].rstrip() + "\n"


def attribution(version, counts):
    meta = importlib.metadata.metadata("wordfreq")
    body = meta.get_payload() if hasattr(meta, "get_payload") else ""
    lic = readme_part(body, "## License", "## Citing wordfreq")
    cite = readme_part(body, "## Citing wordfreq", "## Citations to work that wordfreq is built on")
    refs = readme_part(body, "## Citations to work that wordfreq is built on", None)
    lines = "\n".join(f"  {lang}: {n} words" for lang, n in counts)
    return (
        "Word lists for Quillbinder's Handwriting (release models-5)\n"
        "=============================================================\n\n"
        f"Made from wordfreq {version} by Robyn Speer (https://github.com/rspeer/wordfreq/). wordfreq's code is under the\n"
        "Apache License 2.0; its data files are under the Creative Commons Attribution-ShareAlike 4.0 license\n"
        "(https://creativecommons.org/licenses/by-sa/4.0/; the text is in LICENSE.txt beside this file). These lists\n"
        "are derived from that data: each is the most frequent words of one language with their Zipf frequency (the\n"
        "logarithm of the frequency per billion words), words with digits or punctuation left out, words changed only to\n"
        "Unicode form NFC. They are shared under the same license, Creative Commons Attribution-ShareAlike 4.0.\n"
        "Among wordfreq's sources are Google Books Ngrams and the SUBTLEX word lists; the terms of its sources follow, as\n"
        "its README states them.\n\n"
        f"Words per list:\n{lines}\n\n"
        "---- from wordfreq's README ----\n\n" + lic + "\n" + cite + "\n" + refs
    )


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("outdir")
    ap.add_argument("--top", type=int, default=100000)
    ap.add_argument("--lang", action="append")
    ap.add_argument("--legal", help="the text of the CC BY-SA 4.0 legal code to copy beside the lists")
    args = ap.parse_args()
    version = importlib.metadata.version("wordfreq")
    if version != "3.1.1":
        print(f"::warning::wordfreq {version} (the workflow pins 3.1.1)")
    os.makedirs(args.outdir, exist_ok=True)
    counts = []
    for lang in args.lang or LANGUAGES:
        rows = make_list(lang, args.top)
        probe = SELFTEST_WORDS.get(lang)
        if probe and probe not in {w for w, _ in rows[:50]}:
            sys.exit(f"{lang}: '{probe}' is not among the 50 most frequent words: the list looks wrong")
        if len(rows) < 5000:
            sys.exit(f"{lang}: only {len(rows)} words")
        write_gz(os.path.join(args.outdir, f"wordlist-{lang}.tsv.gz"), rows)
        counts.append((lang, len(rows)))
        print(f"{lang}: {len(rows)} words, most frequent '{rows[0][0]}' {rows[0][1]:.2f}, last '{rows[-1][0]}' {rows[-1][1]:.2f}")
    with open(os.path.join(args.outdir, "wordlist-ATTRIBUTION.txt"), "w", encoding="utf-8") as fh:
        fh.write(attribution(version, counts))
    if args.legal:
        with open(args.legal, "rb") as src, open(os.path.join(args.outdir, "wordlist-LICENSE-CC-BY-SA-4.0.txt"), "wb") as dst:
            dst.write(src.read())


if __name__ == "__main__":
    main()
