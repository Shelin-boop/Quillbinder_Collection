# Translation and Proofreading

One of Quillbinder's own add-ons since Quillbinder 0.92.0 (the add-on's version 1.2); before, it was an official
add-on with a repository of its own. English text is checked for spelling and grammar, German text is translated
into English, on the writer's own computer, with language models the writer chooses and downloads.

- `manifest.json`: the add-on's description (the same file as `source/packaging/manifest.json`).
- `stage.sh`: builds the add-on from `source/` and stages it for `packaging/linux/build-addons.sh`. It needs the
  engine libraries (oneDNN, CTranslate2, SentencePiece) of `tools/get-translator-deps.sh`; without them the add-on is
  left out of a local build. The release workflow builds them and fails when the add-on is missing.
- `source/`: the add-on as a project of its own (`source/README.md` for writers, `source/DEVELOPING.md` for
  developers, `source/tests` for its own checks, which need downloaded models).
- The models: releases `models-1` and `models-2` of this repository (`.github/workflows/models-check.yml` compares
  them with the add-on's catalog). Each model carries its license; CoEdIT Large (Grammarly) is CC BY-NC 4.0,
  for non-commercial use only, and the add-on says so before it is downloaded.

Quillbinder itself has no AI functions; this add-on can be switched off or removed in the Add-on Manager.
