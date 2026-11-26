# Spanish spelling dictionary (es_ES)

The Spanish Hunspell dictionary (`es_ES.aff`, `es_ES.dic`) of LibreOffice's dictionaries
(https://github.com/LibreOffice/dictionaries, tag `libreoffice-25.8.7.3`), **unchanged**. The files are not kept in this repository:
`packaging/linux/build-addons.sh` fetches them with `tools/stage-dictionary.sh`, checks each file against the SHA-256 sum in
`sources.txt` and puts them, with the license and README files named there, into the package (`dictionaries/`).

License: Triple license, free choice: GNU GPL 3 or later, GNU LGPL 3 or later, or MPL 1.1 or later (the dictionary is passed on under the LGPL 3 or the MPL 1.1).

The program reads the files as data (Hunspell); the add-on is a separate download and is not part of the program's
own license. The licenses and READMEs of the dictionary's authors travel in the package beside the dictionary.
