# Russian spelling dictionary (ru_RU)

The Russian Hunspell dictionary (`ru_RU.aff`, `ru_RU.dic`) of LibreOffice's dictionaries
(https://github.com/LibreOffice/dictionaries, tag `libreoffice-25.8.7.3`), **unchanged**. The files are not kept in this repository:
`packaging/linux/build-addons.sh` fetches them with `tools/stage-dictionary.sh`, checks each file against the SHA-256 sum in
`sources.txt` and puts them, with the license and README files named there, into the package (`dictionaries/`).

License: BSD-style license (three clauses, with the marking of modified versions; its text is in the README).

The program reads the files as data (Hunspell); the add-on is a separate download and is not part of the program's
own license. The licenses and READMEs of the dictionary's authors travel in the package beside the dictionary.
