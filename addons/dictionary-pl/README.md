# Polish spelling dictionary (pl_PL)

The Polish Hunspell dictionary (`pl_PL.aff`, `pl_PL.dic`) of LibreOffice's dictionaries
(https://github.com/LibreOffice/dictionaries, tag `libreoffice-25.8.7.3`), **unchanged**. The files are not kept in this repository:
`packaging/linux/build-addons.sh` fetches them with `tools/stage-dictionary.sh`, checks each file against the SHA-256 sum in
`sources.txt` and puts them, with the license and README files named there, into the package (`dictionaries/`).

License: Several licenses at the user's choice: GPL, LGPL, MPL, Apache 2.0 and Creative Commons ShareAlike (the dictionary is passed on under the Apache License 2.0 or the LGPL; the texts are in the READMEs); the file is encoded in ISO 8859-2, which the program reads.

The program reads the files as data (Hunspell); the add-on is a separate download and is not part of the program's
own license. The licenses and READMEs of the dictionary's authors travel in the package beside the dictionary.
