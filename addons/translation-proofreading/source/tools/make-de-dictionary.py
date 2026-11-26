#!/usr/bin/env python3
"""The German dictionary of version 1.3, made by tools/make-dictionary.py since 1.5 (one script for every language):

    python3 tools/make-de-dictionary.py <kaikki .jsonl> <legal code text> <out dir> [--source-url URL]
    python3 tools/make-de-dictionary.py --selftest [<file to write the sample extract to>]

is the same as `make-dictionary.py de ...`.
"""
import os
import runpy
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
args = sys.argv[1:]
if "--selftest" not in args:
    pos = [a for a in args if not a.startswith("--")]
    # insert the language before the first positional argument
    i = args.index(pos[0]) if pos else len(args)
    args = args[:i] + ["de"] + args[i:]
sys.argv = [os.path.join(HERE, "make-dictionary.py")] + args
runpy.run_path(os.path.join(HERE, "make-dictionary.py"), run_name="__main__")
