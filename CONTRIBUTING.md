# Feedback and contributions

Thank you for trying Quillbinder. It is beta software, and reports from real use are what it needs most.

## Report a problem

Use the form [Report a problem](https://github.com/Shelin-boop/Quillbinder_Collection/issues/new?template=bug-report.yml).
You need a GitHub account. Write in English or German.

- Say which version (**Help > About**) and which system.
- Describe the steps that lead to the problem, as exactly as you can.
- **Help > System Information…** and **Help > Crash Reports…** (**Copy to Clipboard**) give a text you
  can paste into the form.

**Do not attach your manuscript or your project.** Issues are public. A crash report names the open
project and its folder; take those lines out if you do not want them read. On Windows a memory dump is
kept beside each crash report: it can hold text you were writing, so do not upload it.

If text was damaged or lost, say so in the title. Such reports come first.

## Propose a function

Use the form [Propose a function](https://github.com/Shelin-boop/Quillbinder_Collection/issues/new?template=feature-request.yml).
Describe what you want to do and why, in your own words. Please do not post screenshots, texts or files
of other programs.

Much of what one writer needs and another does not fits an add-on better than the program itself: see
"Add-ons" below.

## Report a security problem

Not in a public issue: see [SECURITY.md](SECURITY.md).

## Anything else

Questions, remarks, and ideas you would like to talk over before proposing them:
[Discussions](https://github.com/Shelin-boop/Quillbinder_Collection/discussions). For anything that fits neither a form nor a discussion,
[open an issue without a form](https://github.com/Shelin-boop/Quillbinder_Collection/issues/new).

## Changes to the source

Quillbinder is developed in this repository, on branches and pull requests of the project itself. For
now **it takes pull requests from the project's collaborators only**, so a change from outside comes as
an issue:

1. Describe the change in an issue first (the form "Propose a function", or "Report a problem" for a
   fix). For anything larger, wait for an answer before you write code.
2. A small fix can come as a patch in the issue, or as a link to a branch in a copy of your own. It is
   read and, if it fits, applied by the project. The issue is then closed with a note, and the change is
   credited in `docs/CHANGES.md`.

By offering a change you agree that it is published under the project's license, the Apache License 2.0
(`LICENSE`, section 5).

How the source is laid out and built: `README.md`, `CLAUDE.md` and `docs/dev-notes/`. The automated tests
(`tests/`, run with `tools/run-all-tests.sh`) must pass with a change.

## Add-ons

An add-on stays in its author's own repository; it needs no change to Quillbinder's source.

- How to write one: `docs/developer-guide.md`.
- Anyone can share an unsigned add-on; Quillbinder installs it after a warning.
- To have an add-on reviewed, built, signed and listed as an official add-on: `docs/official-addons.md`
  and the form [Add-on for the official list](https://github.com/Shelin-boop/Quillbinder_Collection/issues/new?template=addon-submission.yml).

## Conduct

Be polite and stay on the subject. Issues that insult people, advertise, or have nothing to do with
Quillbinder are closed.
