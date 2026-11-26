#!/usr/bin/env python3
"""Makes the Choice values of every translated welcome project the ones the program's lists hold in that language.

The built-in object templates (src/core/Objects.cpp) fill their Choice lists (role, kind, season, precipitation) through
tr(); a welcome project's object stores one of those values as text. One rule for all languages: the value an outline
stores is what the program shows for the English value in that language (translations/quillbinder_<code>.ts, context
"QObject"); where the .ts has no translation the list shows the English text, so the outline keeps the English value.
A value a translator put into an outline but not into the .ts goes into the .ts (so that the list and the outline agree).

Run after changing the choice values of a language:  python3 tools/align-welcome-choices.py [--check]
--check changes nothing and exits 1 when a language does not follow the rule.
"""
import json
import os
import re
import sys
from xml.sax.saxutils import escape, unescape

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CHOICE_FIELDS = {"role", "kind", "season", "precipitation"}
LANGS = ["de", "es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"]


def ts_lookup(text):
    ctx = re.search(r"<context>\s*<name>QObject</name>(.*?)</context>", text, re.S)
    out = {}
    if ctx:
        for m in re.finditer(r"<message>\s*<source>(.*?)</source>.*?<translation([^>]*)>(.*?)</translation>", ctx.group(1), re.S):
            if "unfinished" not in m.group(2) and m.group(3):
                out[unescape(m.group(1))] = unescape(m.group(3))
    return out


def ts_add(text, source, translation):
    entry = f"    <message>\n        <source>{escape(source)}</source>\n        <translation>{escape(translation)}</translation>\n    </message>\n"
    m = re.search(r"(<context>\s*<name>QObject</name>.*?)(</context>)", text, re.S)
    if m:
        return text[: m.start(2)] + entry + text[m.start(2):]
    return text.replace("</TS>", f"<context>\n    <name>QObject</name>\n{entry}</context>\n</TS>")


def main():
    check = "--check" in sys.argv
    en = json.load(open(os.path.join(ROOT, "resources/welcome/outline.json"), encoding="utf-8"))
    bad = 0
    for code in LANGS:
        op = os.path.join(ROOT, f"resources/welcome/i18n/{code}/outline.json")
        tp = os.path.join(ROOT, f"translations/quillbinder_{code}.ts")
        o = json.load(open(op, encoding="utf-8"))
        ts = open(tp, encoding="utf-8").read()
        known = ts_lookup(ts)
        changed_o = changed_t = False
        for a, b in zip(en["objects"], o["objects"]):
            for k, v in a["values"].items():
                if k not in CHOICE_FIELDS or not isinstance(v, str):
                    continue
                have = b["values"].get(k)
                if v in known:
                    want = known[v]
                elif have and have != v:
                    # a translator's value that the .ts lacks: the .ts takes it over
                    if not check:
                        ts = ts_add(ts, v, have)
                    known[v] = have
                    changed_t = True
                    want = have
                else:
                    want = v
                if have != want:
                    bad += 1
                    if not check:
                        b["values"][k] = want
                    changed_o = True
        if changed_t and not check:
            open(tp, "w", encoding="utf-8").write(ts)
        if changed_o and not check:
            open(op, "w", encoding="utf-8").write(json.dumps(o, ensure_ascii=False, indent=2) + "\n")
        print(f"{code}: {'changed' if (changed_o or changed_t) else 'ok'}")
    if check and bad:
        sys.exit(1)


main()
