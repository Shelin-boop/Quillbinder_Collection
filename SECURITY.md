# Security

## Reporting a security problem

**Please do not describe a security problem in a public issue.** A public description helps an attacker
before a fix is out.

Report it privately instead:

1. Open the repository's **Security** tab and choose **Report a vulnerability**, or go straight to
   <https://github.com/Shelin-boop/Quillbinder_Collection/security/advisories/new>. Only the project reads
   such a report. You need a GitHub account.
2. If that page does not offer the form, open an ordinary issue that says only "I would like to report
   a security problem" and nothing about the problem itself. The project then opens a private channel
   with you.

What helps in a report:

- the version (**Help > About**) and the system (Linux, Windows or Android);
- what an attacker needs: a prepared project, a file to import, a package (`.qbpkg`), a web page;
- the steps, or a file, that show the problem;
- what the attacker gains (running a program, writing outside Quillbinder's folders, reading files,
  installing something without the signature check).

## What counts as a security problem

- A program update or an official add-on that installs although its signature is missing, wrong or made
  with the wrong key. Every released package is signed: program updates with the release key, official
  add-ons with an add-on key the release key vouches for (`docs/official-addons.md`; how to check a
  download yourself: the user manual, section 17).
- A project, an imported file, a web page or a package that makes Quillbinder run code, or read or write
  files outside the folders it works in.
- Anything that sends data from the computer without the writer starting it. Quillbinder contacts the
  internet for one thing only: to look for updates. It fetches one small file, sends nothing about you, and
  you can switch it off in the Help menu (the user manual, section 1, "Privacy: what leaves your computer",
  and section 17, "Checking for updates").
- A way to make the check for updates offer or install something the project did not release: the list it
  reads counts only with the release key's signature, and every download is checked against that list.
- A fault in the release workflows (`.github/workflows`) that could let someone else's code reach a
  signing key or a release.

Not security problems, and welcome as ordinary issues: a crash on one of your own files, a damaged or lost
text, a wrong display.

An unsigned add-on is not a security problem either: Quillbinder installs one only after a warning, and an
add-on is a program that can do what any program on the computer can do. Install only add-ons whose source
you trust.

## Third-party parts

The third-party parts Quillbinder ships (Qt and the libraries inside it, FFmpeg, OpenSSL, the add-ons' libraries,
the Ubuntu packages inside the AppImage) are checked every day against public vulnerability sources. A finding
rated High or Critical for a version Quillbinder ships becomes an issue with the label `security`.

## Which versions get fixes

Only the newest published version of each system: Linux (beta), Windows (experimental) and Android
(experimental). They are listed on the [download page](https://shelin-boop.github.io/Quillbinder_Collection/).
Quillbinder is beta software.

## Choices that look like faults

Some properties are deliberate, for example that projects are plain files, that a lock warns and does not block, and
where the program does and does not check signatures. Their reasons, costs and what would change them are in
[docs/design-decisions.md](docs/design-decisions.md); please read it before reporting one of them.

## What to expect

Quillbinder is a small project. Reports are read and answered, but there is no fixed response time and no
reward program. A confirmed problem is fixed in the next version, and the fix is named in
`docs/CHANGES.md`, with your name if you wish.
