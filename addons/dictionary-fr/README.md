# French spelling dictionary (fr_FR)

The French Hunspell dictionary (`fr_FR.aff`, `fr_FR.dic`) of LibreOffice's dictionaries
(https://github.com/LibreOffice/dictionaries, tag `libreoffice-25.8.7.3`), **unchanged**. The files are not kept in this repository:
`packaging/linux/build-addons.sh` fetches them with `tools/stage-dictionary.sh`, checks each file against the SHA-256 sum in
`sources.txt` and puts them, with the license and README files named there, into the package (`dictionaries/`).

License: Mozilla Public License 2.0 (the dictionary of the Grammalecte project; the file is named fr.dic in LibreOffice and fr_FR.dic here).

The program reads the files as data (Hunspell); the add-on is a separate download and is not part of the program's
own license. The licenses and READMEs of the dictionary's authors travel in the package beside the dictionary.
