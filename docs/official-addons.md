# Official add-ons

An add-on extends Quillbinder without changing it: a writing tool, an importer, a compile output, or data such as
themes and templates (see `docs/developer-guide.md`, sections 5 and 6). Anyone can write one. This page says how an
add-on written outside the project becomes an **official add-on**: reviewed, built and signed by the project, and
listed on the [download page](https://shelin-boop.github.io/Quillbinder_Collection/).

An add-on never becomes part of the main program this way. Its source stays in its author's repository, and the
author keeps maintaining it.

**Official add-ons are free.** The project charges nothing for an add-on another author wrote, and does not sell it.

## Three kinds of packages, three kinds of trust

The project's decision of 2026-11-15:

| Package | Signed with | What the program does |
|---------|-------------|-----------------------|
| A program update | the project's release key, and only that key | installs it; refuses it without that signature |
| An official add-on (the project's own and those of other authors) | an add-on key of the project (one or more) | installs it as an official add-on |
| Any other add-on | nothing, or a key the program does not know | installs it after a warning; `--install <file> --allow-unsigned` on the command line |

An add-on key can never sign a program update. Unsigned add-ons stay possible for everyone who wants to share one or
try one.

**Since 0.91.3** the program trusts an add-on key when the package carries a certificate in which the release key
vouches for it, for add-ons only; a program update signed with an add-on key is refused. The project's add-on key
was created by the maintainer on 2026-11-15 and is kept as a secret of the workflows; from 0.91.3 on it signs the
project's own add-ons too. Add-ons signed with the release key before stay trusted. A version up to 0.91.2 sees an add-on signed
with an add-on key as an unsigned one and warns, so the release text of such an add-on asks for 0.91.3 or later.

## The way to the list

1. **Submission.** The author opens an issue with the form "Add-on for the official list" in
   [Quillbinder_Collection](https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose): the repository, the exact
   commit, the license, the third-party parts, what the add-on downloads and what it sends. No pull request with the
   add-on's code into Quillbinder's source.
2. **Review.** The project reads that commit. An add-on with program code runs with all the rights of the writer's
   login, so the review looks at what it sends and to where, what it downloads and how it checks it (SHA-256), the
   files it writes, the licenses of everything in the package and of what it downloads (with the rule for data for
   non-commercial use only, below), and that it uses only the public interfaces. The answer comes as a comment on the issue; the author changes the add-on in their repository
   and names the new commit.
3. **Catalog entry.** An accepted add-on gets an entry in Quillbinder's repository, `official-addons/<id>.json`, with
   the reviewed commit, the version, the license, the downloads for non-commercial use only, and the commands that
   build it (`official-addons/README.md`). That entry is all of the add-on that goes into Quillbinder's repository.
4. **Build and checks.** The project's workflow "Official add-on" (`.github/workflows/addon-release.yml`) builds the
   package from that commit for the current Quillbinder (Linux first: Ubuntu 24.04, Qt 6.12.0, runtime
   `u2404-qt6.12-r1`), runs the add-on's own checks, checks the package against the entry and the runtime, installs it
   into the newest released AppImage, and collects the source code of the third-party parts. A run without "Publish"
   ends here, with the unsigned package to look at. An add-on for Windows too (since 2026-11-16) has a Windows block
   in its entry, with its own reviewed commit and commands; the workflow "Official add-on for Windows"
   (`.github/workflows/addon-release-windows.yml`) builds it the same way for Quillbinder for Windows (Windows 11 x64,
   MSVC, Qt 6.12.0 for MSVC) and installs it into the newest Quillbinder for Windows. An add-on for Android's tablets
   (since 2026-11-16) has an Android block in the same way; the workflow "Official add-on for Android"
   (`.github/workflows/addon-release-android.yml`) builds it on Ubuntu with the Android NDK and Qt 6.12.0 for Android
   (arm64), and checks that the package is for Quillbinder for Android (platform `android-arm64`, runtime
   `android-qt6.12-r1`, 0.91.3 or later), that its libraries need nothing the app does not bring, and that they keep
   to 16 KB memory pages. It cannot install the add-on there (Quillbinder for Android runs only on a tablet).
5. **Signing and publishing.** A job that runs no code of the add-on signs the package with the add-on key, and the
   signed package must install into the AppImage (for Windows: into Quillbinder for Windows) without
   `--allow-unsigned`; for Android the signature is checked by the program's own rule. Then it is published on a
   release of its own, `addon-<id>-<version>` (for example "Translation and Proofreading 1.0 (official add-on)"; for Windows
   `addon-<id>-<version>-windows`, for Android `addon-<id>-<version>-android`), never marked as the
   latest release, with a list of checksums and the sources. The release text names the add-on's license, its author,
   and every download for non-commercial use only, with the sentence that the add-on must not be sold.
6. **Listing.** This page (below) and the README list the official add-ons: name, author, what each does, and notes
   such as a model for non-commercial use only. (The download page lists them in a later version.)
7. **New versions.** The author names the new commit in a new issue or in a comment. The review covers what changed
   since the reviewed commit; then the catalog entry moves to the new commit and the add-on is built and signed anew.
   When Quillbinder moves to new libraries (a new runtime), the workflow builds every listed add-on again.
8. **Off the list.** An add-on that is no longer maintained, or that breaks a rule below, is taken off the list. Its
   published packages stay where they are, marked as no longer listed.

## What an official add-on needs

- A public repository and a license that allows the project to build and distribute the package; the source of every
  third-party part that its license asks for (the project publishes it with the package).
- A build that runs on a clean Ubuntu 24.04 next to Quillbinder's source, from the named commit, with every download
  pinned (a version and a checksum or a commit), and checks of its own.
- Only the public interfaces: the package format (`manifest.json`, `payload/`), the plugin interfaces of
  `docs/developer-guide.md` section 6. An add-on that needs more of the program asks for a change of an interface
  first; it never ships a changed program.
- Nothing sent anywhere without the writer's own action, nothing at all to collect usage data, every download named
  and checked by its SHA-256, and the writer asked before a large download or one with license terms of its own.
- Files only in the add-on's own folders and in the places the interfaces give it.
- No text, pictures or code of Scrivener, Scapple or their manuals, templates and tutorials.

## Data for non-commercial use only

Some models and data sets may be used only non-commercially (for example under the license CC BY-NC 4.0, which
allows use and sharing that is not primarily intended for commercial advantage or monetary compensation). The
project's decision of 2026-11-15: an official add-on may offer such data. The review checks that:

- the add-on itself is free of charge (as every official add-on is);
- the data is not inside the package: the add-on downloads it separately, checks it by its SHA-256, and asks the
  writer first, showing the license and where to read it;
- the submission names every such download (the form's field "What it downloads while the writer uses it");
- the add-on's release text says so, in these words or nearly: "This add-on is free and must not be sold. It can
  download <name>, whose license (<license>) allows non-commercial use only."; the download page's list marks it too.

Selling here means asking money for the add-on, for the add-on together with that data, or for the data. Whether a
writer may use the results of such a model in a work they sell is a question of that license, which the writer reads
before the download; the project gives no legal advice on it.

## The list

| Add-on | Author | What it does | Notes | State |
|--------|--------|--------------|-------|-------|
| Translation and Proofreading 1.1 ([source](https://github.com/Shelin-boop/Quillbinder_Translation_Addon)) | The Quillbinder Translation Add-on project | Checks English text for spelling and grammar and translates German text into English, on the writer's computer; works while the writer types, shows suggestions for single words | Can download CoEdIT Large, whose license (CC BY-NC 4.0) allows non-commercial use only; the larger models need 2.8 to 5.5 GB of memory (the add-on refuses one the computer cannot hold) | 1.1 published 2026-11-15: [release addon-translation-proofreading-1.1](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/addon-translation-proofreading-1.1) (needs Quillbinder 0.91.3 or later); **for Windows** 1.1 published 2026-11-16: [release addon-translation-proofreading-1.1-windows](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/addon-translation-proofreading-1.1-windows) (needs Quillbinder for Windows 0.91.3 or later); **for Android** 1.1 published 2026-11-16: [release addon-translation-proofreading-1.1-android](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/addon-translation-proofreading-1.1-android) (needs Quillbinder for Android 0.91.3 or later, a tablet with 64-bit ARM); 1.0: [release addon-translation-proofreading-1.0](https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/addon-translation-proofreading-1.0) |

Since Quillbinder 0.92.0 the add-on is one of the project's own on Linux, and since Windows 0.92.0 on Windows too (version 1.2: its source is `addons/translation-proofreading`, it is built and signed with Quillbinder and is part of the full version, and its models are on this repository's releases `models-1` and `models-2`). The row above still describes 1.1: it stays the version for Android until Android is released from this repository, and its Windows package is the one for Quillbinder for Windows before 0.92.0.

## The add-on key (for the maintainer)

The add-on key is an Ed25519 key of its own, made once on the maintainer's computer and never put into a repository:

    openssl genpkey -algorithm ed25519 -out quillbinder-addon-key.pem

Its whole text (from `-----BEGIN PRIVATE KEY-----` to `-----END PRIVATE KEY-----`) goes into the secret
`QB_ADDON_KEY` of this repository (Settings > Secrets and variables > Actions). The workflows make its
certificate themselves: the signing job signs it with the release key each time ("Quillbinder official add-ons"), so
nothing else has to be kept. Without the secret, "Official add-on" builds and checks but does not publish, and the
program's release signs the project's own add-ons with the release key, as before 0.91.3.
