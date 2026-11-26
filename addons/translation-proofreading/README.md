# Translation Helper (called Translation and Proofreading up to 1.4)

One of Quillbinder's own add-ons since Quillbinder 0.92.0 (the add-on's version 1.2; 1.3 with Quillbinder 0.93.0); before, it was an official
add-on with a repository of its own. Text is translated between any two of eleven languages (1.5; before: German and English) and English text is checked for spelling and
grammar, on the writer's own computer, with language models the writer chooses and downloads. This tool gives suggestions only. Its translations are not precise: check each one, which needs a working knowledge of both languages.

- `manifest.json`: the add-on's description (the same file as `source/packaging/manifest.json`).
- `stage.sh`: builds the add-on from `source/` and stages it for `packaging/linux/build-addons.sh`. It needs the
  engine libraries (oneDNN, CTranslate2, SentencePiece) of `tools/get-translator-deps.sh`; without them the add-on is
  left out of a local build. The release workflow builds them and fails when the add-on is missing.
- `source/`: the add-on as a project of its own (`source/README.md` for writers, `source/DEVELOPING.md` for
  developers, `source/tests` for its own checks, which need downloaded models).
- The models: releases `models-1`, `models-2`, `models-3` (1.3: OPUS-MT English to German and the German dictionary, made by
  `.github/workflows/translator-data.yml`) and `models-4` (1.5: the models and dictionaries of the eleven languages, made by
  `.github/workflows/translator-data-4.yml`) of this repository (`.github/workflows/models-check.yml` compares them with
  the add-on's catalog). Each model carries its license; CoEdIT Large (Grammarly) is CC BY-NC 4.0,
  for non-commercial use only, and the add-on says so before it is downloaded.

Quillbinder itself has no AI functions; this add-on can be switched off or removed in the Add-on Manager.
