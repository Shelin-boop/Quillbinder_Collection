# The update file

`security.json` names the versions that were security updates (see its comment). `tools/update-feed.py` writes
`updates-1.json`, the small file the program's check for updates reads (its form: `src/core/UpdateCheck.h`); the
release workflow signs it with the release key and puts it on the project's page beside the download page:

    https://shelin-boop.github.io/Quillbinder_Collection/updates-1.json
    https://shelin-boop.github.io/Quillbinder_Collection/updates-1.json.sig

That address is built into every version since 0.92.0, so it must never change, and neither must the repository's
name. Every workflow that publishes the page must carry the two files along (the page is replaced as a whole).
