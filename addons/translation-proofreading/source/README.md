# Translation Helper (a Quillbinder add-on; called Translation and Proofreading up to 1.4)

**Since version 1.2 this add-on is one of Quillbinder's own.** It comes inside the full version of Quillbinder 0.92.0
and later, its source is this folder of Quillbinder's repository, and its models are on that repository's releases.
What follows was written for version 1.1, when it had a repository of its own; where it speaks of installing the
package by hand, the full version has done that for you.

An add-on for [Quillbinder](https://github.com/Shelin-boop/Quillbinder_Collection) (Linux, version 0.80.0 or later, and
since 2026-11-16 Windows 11, Quillbinder for Windows 0.91.1 or later) that
translates between any two of eleven languages (1.5) and checks English text for spelling and grammar, on your own computer. Once a
model is downloaded, it works without any internet connection.

**This tool gives suggestions only. Its translations are not precise: check each one, which needs a working knowledge of both languages.**

## What it does

- **Two places to work.** A tab of its own in the inspector (its icon is the first in the row: the text goes in at
  the top, the result comes out at the bottom, the controls are between them) and a window of its own (the text left,
  the result right, the controls above and between). Open the window with the add-on's button on the toolbar or with
  **Edit > Writing Tools > Translation Helper...**.
- **Eleven languages** (1.5). With **Translate**, two drop-down lists name the language of your text and the language of the
  translation, any two of English, German, Spanish, French, Portuguese (Brazil), Italian, Russian, Indonesian, Turkish,
  Polish and Dutch. Every language has two OPUS-MT models (to and from English); a pair without English goes through
  English (two steps, so a mistake of the first is carried into the second, which the output says). A dictionary per
  language except English (an extract of the Wiktionary, CC BY-SA 4.0, a separate download whose license is shown first)
  gives the word classes and meanings of a single word; a language without a dictionary gets the model's translations
  only. A single word keeps its correction, shown as a suggestion. The add-on's own texts are in the eleven languages
  of the program (the add-on reads the program's language; English where there is no translation). The line above
  stands in the window under the controls.
- **One rule for the memory** (1.6, with Quillbinder 0.99.0 or later). The add-on reports its models to the program's model list: the program decides before a load whether it fits, unloads a model that was not used for the set time, and shows "Model change in progress". The processor cores and the unload time are the program's one setting for all AI functions (File > Options > General > AI Models); with an older Quillbinder the add-on keeps its own two fields.
- **English text is proofread, German text is translated.** With **Automatic** the add-on tells the two apart by
  itself; you can also choose **Proofread English** or **Translate German to English**. **More Formal** lets the model
  also rewrite for a more formal tone.
- **Every change is marked.** Click a marked word: keep your original, or take the suggestion. Click any word of the
  result for other words: the rest of the sentence is fitted to the word you choose. In the text you put in
  (English), the right mouse button offers **Other Words for ...** as well.
- **Live** (1.1). About a second after you stop typing, the result follows; only the sentences you changed are worked
  on again. The **Live** button switches it off.
- **What a word means** (1.3). One English word: below the result, what it means in English, by word class (the
  meanings are from WordNet 3.0, built into the add-on, and the output says so). A word WordNet does not have gets a
  meaning written by Qwen3 4B (or Qwen3 1.7B, when that is the one downloaded), with a warning that it may be
  inaccurate; with neither, the output says what can be downloaded.
- **One German word** (1.3). It is looked up first in the **German dictionary** (an extract of the English
  Wiktionary's German entries, about the size shown in the options; a separate download, with its license, CC BY-SA
  4.0, shown first): base form, word classes, meanings and the English words of each meaning, the first five meanings
  of a class and the rest behind "more". A class without an English word, and a word that is not in the dictionary,
  are translated by the small model (in a frame for the class: "zu fluchen", "das Fluchen"), with a warning. A click
  on an English word makes it the result; "Similar" follows it and its word class. English to German does the same
  for one English word, with WordNet's classes and OPUS-MT English to German (small).
- **A word on its own, in both directions** (1.4). The translations of each word class (a lower-case German word that is a verb and a noun gets both; a word the dictionary lacks is translated in both forms), "Similar translations" (English: the thesaurus of the result; English to German: "Similar German words", a reverse lookup in the German dictionary of the word and its WordNet synonyms), then the short description. "Automatic" translates a single German word the English word list does not know when the German dictionary is installed.
- **German capitals** (1.4). Before German is translated, the German dictionary puts the capital letters of nouns right in the copy the model gets (the text stays as written); without the dictionary a note says that nouns need capitals.
- **A notice for missing parts** (1.4): a translation model or the German dictionary that is missing is named at the top, with a button to the options. Models are unloaded after 15 minutes without work (a setting).
- **Swap and Translate back** (1.3). Swap turns German to English into English to German and back; Translate back
  shows what the result says in the language of your text, until you change the text. OPUS-MT English to German
  (small) (CC BY 4.0, about 79 MB) is the model for the way back and for English to German; it is offered for download
  when it is missing. In translate mode the second row shows the translation model, and the list of styles is hidden.
- **Short texts** (1.3). A single word or up to three words never go to the large translation model: the dictionary
  first, else the small model. For a short text, "Other translations:" lists the model's best complete translations
  (without made-up names). The information area no longer lists the model's other words.
- **A single word** (1.1). Type one word: the result is the word as it should be, and below it its spelling
  suggestions, words of a similar meaning and the model's other words; click one to take it.
- **The information area** (1.1). Double-click a word in the text or in the result: its spelling suggestions, words of
  a similar meaning (from WordNet) and the model's other words for that place appear in an area at the bottom.
- **Dialogue as written** (1.1). Quotation marks stay where you put them, also when a model moves, turns, doubles or
  drops one; a punctuation mark the model sets apart joins the word before it.
- **Your rules.** No em-dashes (the models may not write one; a dash you wrote becomes a comma, which you can click to
  keep the dash), American spelling (colour to color, realise to realize, towards to toward, about 2,800 words),
  curly quotes and apostrophes, three periods instead of the ellipsis character, and your own replacements
  (`OK => okay`). All in **File > Options > Translation Helper**.
- **From and into the editor.** **From Editor** takes the text selected in Quillbinder's editor and works on it;
  **Into Editor** puts the result in its place (one step to undo there); **Copy** copies it.
- **Safety checks.** When the model rewrites a sentence too much or doubles a word, your sentence is kept, with a note.

## Installing

1. Download `translation-proofreading-<version>.qbpkg` (Linux) or `translation-proofreading-<version>-windows.qbpkg`
   (Windows): the official add-on, built and signed by the Quillbinder project, from its release in
   [the Quillbinder releases](https://github.com/Shelin-boop/Quillbinder_Collection/releases) ("Translation and Proofreading ...
   (official add-on)", for Windows "... for Windows (official add-on)"); or a test build from the pre-release **test**
   (Linux) or **test-windows** (Windows) of this repository.
2. In Quillbinder (0.91.3 or later): **File > Install Update or Add-on**, choose the file. The official add-on is shown
   as signed with the project's add-on key; a test build is not signed, and Quillbinder warns before installing it.
   Restart Quillbinder.
3. **File > Options > Translation Helper**: download a proofreading model and the translation model (see below). They are
   checked (size and checksum of every file) and then used without any connection.

## Models and licenses

The models are not part of the add-on. You choose which ones to download:

| Model | For | Download | Memory while working | License |
|---|---|---|---|---|
| CoEdIT Large (Grammarly) | proofreading: finds many errors, but rewrites more | 788 MB | about 1.6 GB | CC BY-NC 4.0: **non-commercial use only** |
| CoEdIT Base (J. Bochi) | proofreading: smaller, misses many errors | 250 MB | about 0.6 GB | Apache-2.0 |
| Qwen3 1.7B (Qwen team, Alibaba Cloud) | proofreading and translation (1.1) | 1,739 MB | about 2.8 GB | Apache-2.0 |
| Qwen3 4B (Qwen team, Alibaba Cloud) | proofreading and translation (1.1): better, slower | 4,043 MB | about 5.5 GB | Apache-2.0 |
| OPUS-MT German to English (University of Helsinki) | translation | 79 MB | about 0.3 GB | Apache-2.0 |
| OPUS-MT Big, German to English (University of Helsinki) | translation (1.1): better for whole sentences, unreliable for single words (those always go to the small model) | 218 MB | about 0.5 GB | CC BY 4.0 |
| OPUS-MT English to German, small (University of Helsinki) | translation (1.3): English to German, the way back | about 79 MB (set from the release models-3) | about 0.3 GB | CC BY 4.0 |
| German dictionary (extract of the Wiktionary) | a single German word (1.3): classes, meanings, English words | set from the release models-3 | small | CC BY-SA 4.0 (shown before the download) |
| OPUS-MT Spanish, French, Italian, Indonesian, Dutch, Turkish and Polish to English, and English to Spanish, French, Italian, Russian, Indonesian and Dutch (University of Helsinki) | translation (1.5), release models-4 | set from the release | about 0.3 GB | Apache-2.0 |
| OPUS-MT Russian to English (University of Helsinki) | translation (1.5) | set from the release | about 0.3 GB | CC BY 4.0 |
| OPUS-MT English to Polish: the West Slavic model (University of Helsinki), told to write Polish | translation (1.5) | set from the release | about 0.3 GB | Apache-2.0 |
| OPUS-MT Portuguese (Brazil) to English: the Romance model (University of Helsinki) | translation (1.5) | set from the release | about 0.3 GB | Apache-2.0 |
| OPUS-MT English to Turkish and English to Portuguese (Brazil): the larger tc-big models (University of Helsinki) | translation (1.5) | set from the release | about 0.35 GB | CC BY 4.0 |
| Dictionaries of Spanish, French, Portuguese, Italian, Russian, Indonesian, Turkish, Polish and Dutch (extracts of the Wiktionary, by kaikki.org) | a single word of that language (1.5) | set from the release models-4 | small | CC BY-SA 4.0 (shown before the download) |

The Qwen models are general models told what to do: fix only real errors and leave your words, style and dialogue
alone, or translate. One downloaded Qwen model does both and is loaded once.

**Your computer is protected** (1.1). Every model in the list shows how much memory it needs. A model that needs more
than three quarters of the computer's memory is not offered for download; a model is not loaded when less memory is
free than it needs and 1 GB besides (close other programs, or choose a smaller model). The work leaves one processor
core free by default and runs at a low priority, so the desktop stays usable.

Before CoEdIT Large is downloaded, the add-on asks you to accept its license. Whether a use counts as non-commercial is
your decision; this project gives no legal advice. The models are the originals from Hugging Face (fixed revisions),
converted for the engine with 8-bit weights; the converted files are on the pre-releases
[models-1](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/models-1) and
[models-2](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/models-2) (1.1; the release [models-4](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/models-4) holds the 1.5 downloads, made by `.github/workflows/translator-data-4.yml`; the release [models-3](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/models-3) holds the 1.3 downloads; the 4B model's
large file in three parts, joined after the download), made by the workflow "Models" (`models/convert.py`). On a computer without internet, **From Folder...** takes the files of that release from
a folder.

**Speed** (test machine with 2 processor cores): proofreading with CoEdIT Large about 40 words a second (5,000 words in
about 2 minutes); translating is much faster (the 74 words of the test text in under a second). Other words for a word
take one to three seconds. A computer with more cores is probably faster (not measured).

## Building (for developers)

Everything the add-on needs from Quillbinder is its public tool interface, `third_party/quillbinder/ToolApi.h`
(version 1.1, copied unchanged from the public source), and Qt. On Ubuntu 24.04:

    packaging/build-deps.sh        # oneDNN, CTranslate2 and SentencePiece into deps/install (once, about 40 minutes)
    cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/opt/qt/6.12.1/gcc_64 -DTP_TEST_SWITCHES=ON
    cmake --build build
    TP_MODELS_DIR=<folder with the files of models-1> QT_QPA_PLATFORM=offscreen ctest --test-dir build
    cmake -S . -B build-release -G Ninja -DCMAKE_PREFIX_PATH=/opt/qt/6.12.1/gcc_64
    cmake --build build-release && packaging/make-addon.sh build-release dist

On Windows 11 (since 2026-11-16), in Git Bash inside MSVC's environment (`vcvars64.bat`), with The Qt Company's Qt
6.12.0 for MSVC 2022:

    packaging/build-deps-windows.sh   # the same three libraries with MSVC (about 22 minutes on GitHub's machine)
    cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64 -DTP_TEST_SWITCHES=ON
    cmake --build build && ctest --test-dir build      # (PATH with Qt's bin folder; the results go to build/test-*.txt)
    packaging/make-addon-windows.sh build-release dist  # after a release build: the -windows.qbpkg

A release is built against The Qt Company's Qt 6.12.0, the Qt inside Quillbinder's AppImage since 0.80.0 and inside
Quillbinder for Windows (the workflow "Build" installs it with aqtinstall). The parts:

- `src/TranslateTool.*`: the writing tool (commands, inspector tab, toolbar button, options section).
- `src/Panel.*`: the panel in both layouts; `src/OptionsPage.*`: the options; `src/ModelStore.*`: catalog and download.
- `src/Logic.*`: language, sentences, rules, safety checks, the word comparison (Qt Core only).
- `src/engine/Engine.cpp`: the engine library (CTranslate2 with oneDNN, SentencePiece), loaded by the tool when it is
  first needed (`src/EngineApi.h` between the two); `src/engine/Bpe.cpp`: the Qwen models' tokenizer (checked against
  the model's own tokenizer: `tokenizer-check.json` of each Qwen model).
- `tests/`: the checks: the text logic, the panel and options with the engine and the real models in a stand-in for
  Quillbinder (`FakeHost.h`), the plugin as Quillbinder loads it, and the libraries' visible symbols.
- `src/Dictionary.*`: what a word means (WordNet's definitions, the German dictionary's reader, the frames a lone word is translated in).
- `tools/make-de-dictionary.py`: makes the German dictionary extract from kaikki.org's file (run by the workflow `translator-data.yml`).
- `tools/make-us-spelling.py`: makes the American spelling list from SCOWL's Hunspell dictionaries;
  `tools/make-thesaurus.py`: the list of similar words and (1.3) the list of meanings from Princeton WordNet 3.0 (Ubuntu package wordnet-base).

The add-on's own code is under the Apache License 2.0 (LICENSE, NOTICE); the libraries inside it come under their own
licenses, listed in the package's `translation-proofreading/licenses/README.txt`.

Quillbinder is an independent program; this add-on is not made by or connected with Grammarly, the University of
Helsinki, Alibaba Cloud, Princeton University or the makers of the libraries it uses.
