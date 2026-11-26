#!/usr/bin/env python3
"""Builds the dictionary extract of one language that the add-on downloads (German since 1.3, the other ten of the
Translation Helper's languages since 1.5), from the English Wiktionary's entries of that language as extracted by
wiktextract (kaikki.org: one JSON object per line).

    python3 tools/make-dictionary.py <code> <kaikki .jsonl> <legal code text> <out dir> [--source-url URL]
    python3 tools/make-dictionary.py --selftest [<file to write the sample extract of German to>]

<code> is the language: de, es, fr, pt, it, ru, id, tr, pl or nl. Writes into <out dir>: <code>-dictionary.qz (the
extract, compressed as Qt's qCompress writes it), DICTIONARY-LICENSE.txt (source and license) and
LICENSE-CC-BY-SA-4.0.txt (the license text as given). The extract's form is described in src/Dictionary.h: a header
line "#qb-<code>-dictionary 1", then lines sorted by their first field, the key (the word in lower case):

    key  L  headword  class  gender  meanings     meanings: "english words separated by ;" + "^" + the definition, separated by "|"
    key  F  headword  class                       an inflected form that leads to the headword of that class

What is kept: word classes, up to twelve meanings per entry (without those marked obsolete, archaic and the like),
the English words of a meaning when its definition is made of short English words ("to curse; to swear"), the
definition itself, the gender of nouns, and the inflected forms. Nothing else of the source is kept.
"""
import hashlib
import json
import os
import re
import struct
import sys
import unicodedata
import zlib

# the languages and the names kaikki.org (and the English Wiktionary) give them
LANGUAGES = {
    "de": "German", "es": "Spanish", "fr": "French", "pt": "Portuguese", "it": "Italian", "ru": "Russian",
    "id": "Indonesian", "tr": "Turkish", "pl": "Polish", "nl": "Dutch",
}

POS = {
    "noun": "noun", "verb": "verb", "adj": "adjective", "adv": "adverb", "prep": "preposition", "pron": "pronoun",
    "conj": "conjunction", "intj": "interjection", "num": "numeral", "article": "article", "particle": "particle",
    "phrase": "phrase", "proverb": "proverb", "det": "determiner", "contraction": "contraction", "name": "name",
    "prefix": "prefix", "suffix": "suffix", "postp": "postposition", "adv_phrase": "phrase", "prep_phrase": "phrase",
}
SKIP_TAGS = {"obsolete", "archaic", "dated", "rare", "historical", "dialectal", "regional", "poetic"}
SKIP_FORM_TAGS = {"table-tags", "inflection-template", "class", "romanization"}
MAX_MEANINGS = 12
PAREN = re.compile(r"\([^)]*\)|\[[^\]]*\]")
WORDS_OK = re.compile(r"[A-Za-z][A-Za-z' -]*")
GENDER = re.compile(r"\b([mfn])\b")


def clean(text):
    return " ".join(re.sub(r"[\t\n\r|^]", " ", text).split())


def english_words(gloss, pos):
    """The English words a definition is made of: only when every part (split at ; and ,) is short."""
    parts = [" ".join(p.split()).strip(" .:") for p in re.split(r"[;,]", PAREN.sub(" ", gloss))]
    parts = [p for p in parts if p]
    if not parts or any(len(p.split()) > 4 or not WORDS_OK.fullmatch(p) for p in parts):
        return []
    out = []
    for p in parts:
        if pos == "noun":
            p = re.sub(r"^(a|an|the) ", "", p)
        if pos == "verb" and not p.startswith("to "):
            p = "to " + p
        if p and p not in out:
            out.append(p)
    return out[:6]


def gender_of(entry):
    for t in entry.get("head_templates") or []:
        m = GENDER.search((t.get("expansion") or "").split("(")[0].split(" ", 1)[-1] if t.get("expansion") else "")
        if m:
            return m.group(1)
    for s in entry.get("senses") or []:
        for tag in s.get("tags") or []:
            if tag in ("masculine", "feminine", "neuter"):
                return tag[0]
    return ""


def plain(word, lang):
    """The word as the add-on looks it up: Russian words are written with stress marks in some forms ("стола́")."""
    if lang == "ru":
        word = unicodedata.normalize("NFC", word.replace("\u0301", "").replace("\u0300", ""))
    return word


def build(entries, lang="de"):
    """entries: iterable of wiktextract objects. Returns the text of the extract."""
    lemmas = {}    # (headword, pos) -> [gender, [meaning strings]]
    forms = set()  # (key, headword, pos)
    for e in entries:
        word = plain((e.get("word") or "").strip(), lang)
        pos = POS.get(e.get("pos") or "")
        if not word or not pos or " " in word and pos not in ("phrase", "proverb"):
            continue
        senses = e.get("senses") or []
        form_senses = [s for s in senses if s.get("form_of") or s.get("alt_of")]
        for s in form_senses:
            for fo in (s.get("form_of") or s.get("alt_of") or [])[:1]:
                lemma = plain((fo.get("word") or "").strip(), lang)
                if lemma and lemma != word:
                    forms.add((word.lower(), lemma, pos))
        meanings = []
        for s in senses:
            if s in form_senses or (set(s.get("tags") or []) & SKIP_TAGS):
                continue
            glosses = s.get("glosses") or []
            gloss = clean(glosses[-1]) if glosses else ""
            if not gloss or len(gloss) > 160:
                continue
            meanings.append(";".join(clean(w) for w in english_words(gloss, pos)) + "^" + gloss)
        if meanings:
            slot = lemmas.setdefault((word, pos), ["", []])
            slot[0] = slot[0] or (gender_of(e) if pos == "noun" else "")
            for m in meanings:
                if m not in slot[1] and len(slot[1]) < MAX_MEANINGS:
                    slot[1].append(m)
            for f in e.get("forms") or []:
                form = plain((f.get("form") or "").strip(), lang)
                if (not form or form == word or " " in form or not re.search(r"[^\W\d_]", form)
                        or set(f.get("tags") or []) & SKIP_FORM_TAGS):
                    continue
                forms.add((form.lower(), word, pos))
    lines = []
    for (word, pos), (gender, meanings) in lemmas.items():
        lines.append((word.lower(), f"{word.lower()}\tL\t{word}\t{pos}\t{gender}\t" + "|".join(meanings)))
    for key, head, pos in forms:
        if (head, pos) in lemmas:   # a form of a headword the extract does not have leads nowhere
            lines.append((key, f"{key}\tF\t{head}\t{pos}"))
    lines.sort(key=lambda t: (t[0].encode("utf-8"), t[1]))
    return f"#qb-{lang}-dictionary 1\n" + "".join(t[1] + "\n" for t in lines)


def compress(text):
    data = text.encode("utf-8")
    return struct.pack(">I", len(data)) + zlib.compress(data, 9)   # as Qt's qCompress writes it


def selftest(path=None):
    sample = [
        {"word": "fluchen", "pos": "verb", "senses": [
            {"glosses": ["to curse; to swear"]}, {"glosses": ["(of a clock) to strike the hour in a loud and clumsy way"]},
            {"glosses": ["to use obscenities"], "tags": ["obsolete"]}],
         "forms": [{"form": "fluchte", "tags": ["past"]}, {"form": "fluchen", "tags": ["infinitive"]},
                   {"form": "-", "tags": ["x"]}, {"form": "hat geflucht", "tags": ["perfect"]}]},
        {"word": "Fluch", "pos": "noun", "head_templates": [{"expansion": "Fluch m (strong, genitive Fluches, plural Flüche)"}],
         "senses": [{"glosses": ["curse", "a profane word"]}], "forms": [{"form": "Flüche", "tags": ["plural"]}]},
        {"word": "fluchte", "pos": "verb", "senses": [{"glosses": ["first-person singular preterite of fluchen"],
                                                      "form_of": [{"word": "fluchen"}]}]},
        {"word": "Haus", "pos": "noun", "senses": [{"glosses": ["a house, a home"], "tags": ["neuter"]}]},
        {"word": "nowhere", "pos": "character", "senses": [{"glosses": ["x"]}]},
    ]
    text = build(sample, "de")
    lines = text.split("\n")
    assert lines[0] == "#qb-de-dictionary 1", lines[0]
    assert ("fluchen\tL\tfluchen\tverb\t\tto curse;to swear^to curse; to swear|"
            "^(of a clock) to strike the hour in a loud and clumsy way\n") in text, text
    assert "obscenities" not in text, "obsolete meaning kept"
    assert "fluchte\tF\tfluchen\tverb" in text and "flüche\tF\tFluch\tnoun" in text
    assert "hat geflucht" not in text and "\t-\t" not in text
    assert "fluch\tL\tFluch\tnoun\tm\tprofane word^a profane word\n" in text, text
    assert "haus\tL\tHaus\tnoun\tn\thouse;home^a house, a home" in text, text
    assert "nowhere" not in text
    keys = [l.split("\t")[0].encode() for l in lines[1:] if l]
    assert keys == sorted(keys), "not sorted"
    assert struct.unpack(">I", compress(text)[:4])[0] == len(text.encode())
    # another language: the header names it, a Russian form with a stress mark leads to its word
    ru = build([{"word": "стол", "pos": "noun", "head_templates": [{"expansion": "стол m inan"}],
                 "senses": [{"glosses": ["table"]}], "forms": [{"form": "стола\u0301", "tags": ["genitive"]}]}], "ru")
    assert ru.startswith("#qb-ru-dictionary 1\n") and "\u0441\u0442\u043e\u043b\u0430\tF\t\u0441\u0442\u043e\u043b\tnoun" in ru, ru
    es = build([{"word": "casa", "pos": "noun", "senses": [{"glosses": ["house"]}]}], "es")
    assert es.startswith("#qb-es-dictionary 1\n") and "casa\tL\tcasa\tnoun\t\thouse^house" in es, es
    if path:   # the form the download has, for the add-on's own check of its reader (tests/logic_tests.cpp)
        with open(path, "wb") as f:
            f.write(compress(text))
    print("selftest ok")
    return text


def license_text(lang, url, src_sha):
    name = LANGUAGES[lang]
    return (
        f"{name} dictionary (extract of the Wiktionary), a data download of the add-on Translation Helper "
        "(Translation and Proofreading before version 1.5).\n\n"
        f"Source: the English Wiktionary (en.wiktionary.org), its {name} entries, by the contributors of the\n"
        "Wiktionary, as extracted by wiktextract and published by kaikki.org\n"
        f"({url}); SHA-256 of the file used: {src_sha}.\n\n"
        "License: Wiktionary's text is available under the Creative Commons Attribution-ShareAlike 4.0 License\n"
        "(CC BY-SA 4.0, https://creativecommons.org/licenses/by-sa/4.0/; the text is in LICENSE-CC-BY-SA-4.0.txt)\n"
        "and the GNU Free Documentation License. This extract is shared under CC BY-SA 4.0.\n\n"
        "Changed: reduced to the word classes, up to twelve meanings per entry (the English words of a meaning and its\n"
        "definition), the gender of nouns and the inflected forms; made by tools/make-dictionary.py of Quillbinder's\n"
        "repository (https://github.com/Shelin-boop/Quillbinder_Collection).\n"
        "The contributors of an entry are named in its page history at en.wiktionary.org.\n")


def main():
    if "--selftest" in sys.argv:
        rest = [a for a in sys.argv[1:] if not a.startswith("--")]
        selftest(rest[0] if rest else None)
        return
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    url = None
    if "--source-url" in sys.argv:
        url = sys.argv[sys.argv.index("--source-url") + 1]
        args.remove(url)
    if len(args) != 4 or args[0] not in LANGUAGES:
        sys.exit(__doc__)
    lang, src, legal, out = args
    if url is None:
        url = f"https://kaikki.org/dictionary/{LANGUAGES[lang]}/"
    os.makedirs(out, exist_ok=True)

    def entries():
        with open(src, encoding="utf-8") as f:
            for line in f:
                if line.strip():
                    yield json.loads(line)

    text = build(entries(), lang)
    n_lemmas = sum(1 for l in text.split("\n") if "\tL\t" in l)
    n_forms = sum(1 for l in text.split("\n") if "\tF\t" in l)
    if n_lemmas < 1000:
        sys.exit(f"only {n_lemmas} headwords: the source is not what was expected")
    name = f"{lang}-dictionary.qz"
    with open(os.path.join(out, name), "wb") as f:
        f.write(compress(text))
    with open(legal, "rb") as f:
        legal_bytes = f.read()
    with open(os.path.join(out, "LICENSE-CC-BY-SA-4.0.txt"), "wb") as f:
        f.write(legal_bytes)
    src_sha = hashlib.sha256(open(src, "rb").read()).hexdigest()
    with open(os.path.join(out, "DICTIONARY-LICENSE.txt"), "w", encoding="utf-8") as f:
        f.write(license_text(lang, url, src_sha))
    print(f"{name}: {n_lemmas} headwords, {n_forms} forms, {os.path.getsize(os.path.join(out, name))} bytes")


if __name__ == "__main__":
    main()
