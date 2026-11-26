# Portuguese (Brazil) spelling dictionary (pt_BR)

The Portuguese (Brazil) Hunspell dictionary (`pt_BR.aff`, `pt_BR.dic`) of LibreOffice's dictionaries
(https://github.com/LibreOffice/dictionaries, tag `libreoffice-25.8.7.3`), **unchanged**. The files are not kept in this repository:
`packaging/linux/build-addons.sh` fetches them with `tools/stage-dictionary.sh`, checks each file against the SHA-256 sum in
`sources.txt` and puts them, with the license and README files named there, into the package (`dictionaries/`).

License: GNU Lesser General Public License 3 and Mozilla Public License (both, as published by the dictionary's authors).

The program reads the files as data (Hunspell); the add-on is a separate download and is not part of the program's
own license. The licenses and READMEs of the dictionary's authors travel in the package beside the dictionary.
