# Scrivener format

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## Scrivener format, part 3: the rest of a project (0.90.0 beta, 2026-11-12)

- **Texts** (`rtf::WriteOptions::scrivener`, used by `scrivRtf`): lists with a `\listtable` and `\ls`/`\ilvl` plus
  `{\listtext\t ... \t}` (`\levelnfc23` bullet, `\levelnfc0` decimal; the reader takes the type from the table:
  `scanListTables`), `\ri` from `props::RightOutdent`, a list item's own indents (`props::ListIndents`), fonts named by a
  face ("Cambria-BoldItalic") keep `\b0\i0` (`props::FaceBold` / `FaceItalic`), links underlined only when they were
  (`props::LinkUnderline`), a picture inside a link, `敓2\'3F`. `DocumentStore::setContent` / `setNotesContent` keep
  the first paragraph's format and list (`takeFirstParagraph`: `insertFragment` dropped them); `\plain` ends the paragraph
  state (an empty last paragraph was dropped).
- **Comments and footnotes**: `content.comments` (`<Comments Version="1.0"><Comment ID Footnote="Yes" ...>`) through
  `commentsXml(notes, f, old)`, the file's own attributes kept.
- **Project parts** (`buildScrivx(p, existing, identifier, data)`, `XmlCtx::data` = the .scriv's `Files/Data` listing):
  bookmarks (`Bookmark BinderUUID` / `Destination`; per item before `Children`), `TextSettings/Target`, `ProjectTargets`
  (deadline kept in the file's text, `IgnoreDeadline`, the session options), the corkboard (`CorkboardSettings/Arrangement`,
  `FreeformCorkboardDocuments` with positions in card widths: `ScrivenerImporter::setCardWidth`, `AppSettings::cardWidth`;
  `BinderItem::corkArrangement` "freeform"/"grid", set per folder by `EditorPane::chooseCorkMode`), card pictures
  (`card-image.<ext>`, `ShowSynopsisImage` / `IndexCardImageFileExtension`; the switch is set to No only where the file has
  a card image: Scrivener also sets it on pictures and media), default labels and statuses and their "No ..." names,
  section types only when the file has them or an item uses one, saved searches (`Collection Type="SavedSearch"` with
  `SearchSettings`: `scrivener::queryFromScrivener`, `writeSearchSettings`; Operator "Any" seen in a real project, the
  others guessed), `Settings/compile.xml` ProjectSettings/MetaData (title, abbreviated title, Forename/Surname, language;
  only values that differ), `Settings/ui-common.xml` (open folders, selection, collections shown), `Files/writing.history`
  (`<Day dwc dcc [dtwc] owc occ [dtcc]>`; only in a full write and only when the .scriv has the file or is new; the
  importer takes it as the history and sets the day's start counts, so an import is no writing day).
- **Re-read** (`mergeFromOld`, `kMetaLevel` 2): a state below 2 joins once what the working copy has of bookmarks,
  targets, the deadline, corkboard positions and arrangement, card pictures and search collections (`matchMeta(...,
  joinSearches)`); the writing history is the union (the .scriv wins a date); compile metadata as an overlay.
- **Markdown storage, list items with a style** (found by the system test): written as `{: style=<percent-encoded name>}`
  before the list mark (`paragraphLine`), read in `parseBlock`; older versions ignore the key.
- **The system test** is on `main` since 0.90.0 (`tests/crashsafe/`, `if(NOT WIN32)`); see the project documents
  `claude/quillbinder-test-procedure-crashsafe.md` and its addendum `...-crashsafe-0.90.md`. `QB_CS_MONKEY_ALL=1` and
  `QB_CS_MONKEY_N=<n>` (with `QB_CS_SEED`) find the step of S-E04 that breaks something.

## Scrivener format, part 2: styles, inline notes, web pages (0.81.3 beta, 2026-11-12)

- **Styles** (`ScrivStyles`, `insertStyleMarks` in `ScrivenerFormat.cpp`): a written text gets Scrivener's marks and its
  `content.styles` (IDs comma-separated, capitals, no line end; removed when no style is used). Placement as Scrivener
  3.1.6.0 for Windows writes it: one pair around paragraphs in a row with the same style; the opening mark at the
  paragraph's start, or right after the previous paragraph's closing mark when that one has a style (before its
  paragraph break); `<$Scr_Cs::n>` around character-style runs. `styles.xml`: a style is found by name ("Centered" =
  "Centered Text", "Block Quote" = "Blockquote"/"Quote"); a style made here is added (new UUID, Type Para / Para+Char /
  Char, FontChange Face/Size, Format = RTF of a one-paragraph sample); the file is written only when it changed. The
  reader (`importStyles`, `stripMarkers`): Scrivener's styles without a style of that name become project styles
  (`styles::capture` from the Format); Cs marks give `props::CharStyle`; a paragraph holding only marks keeps the style
  it is in unless a style opens and closes in it (Scrivener puts openings at the end of the paragraph before).
- **Inline notes** (`scrivRtf`): written in Windows' form (decision of 2026-11-12, replaces "1A"): `{\Scrv_fn=...\end_Scrv_fn}`,
  `{\Scrv_annot \color={\R=..\G=..\B=..} \text=...\end_Scrv_annot}` (6 decimals) as text in the RTF; linked notes stay
  `scrivcmt://` with `content.comments`.
- **Pictures in texts**: Scrivener for Windows writes `{\*\shppict{\pict{\*\nisusfilename ...}...\pngblip hex}}`;
  the RTF reader now skips starred groups inside `\pict` (their text went into the picture's hex: every such picture was
  lost, also when the text was written back). A bare `\u65532` (a placeholder without data) is still dropped by the writer.
- **Web pages** (`core/WebArchive.*`: `pack`, `unpack`, `addressOf`; MHTML, RFC 2557): `BinderItem::sourceUrl` (item
  attribute `url`, Scrivener's `MediaSettings/OriginalURL`); this program's pages (HTML + folders) go into a .scriv as
  `content.mht` (Type "WebArchive", `scrivExt`, `isArchived`), the old `content.html`/`page-files` and the stand-in
  content.rtf removed. `unpack` names parts `part<i>.<ext>` (letters/digits, program endings -> `bin`), rewrites
  src/href/background/poster and CSS `url()` (also inside style sheets), decodes the page's charset through
  `QStringDecoder(name)` (ICU). `MediaView::showWebPage` (cache folder for a .mht), bar `webPageBar` with `webAddress`,
  `webOpenSaved` (`openSavedPage`: a copy in Downloads/"Quillbinder web pages"/"<title> (<id8>)", opened with
  `links::openFile`; Snap Firefox cannot read hidden folders or /tmp), `webOpenOriginal` (`openOriginalAddress`: sourceUrl,
  else an http(s) bookmark, else the archive's address), note `webNote`.
- **Notes of media** are written into the .scriv (the notes block runs for every item). **Repair**: a media or web page
  item without text drops a stand-in `content.rtf` that names `Files/Data/<UUID>` (0.81.1 wrote those); the reader
  already turned such Text items into Media / WebPage by their `content.<ext>`.
- **docs.checksum** in the file's own form: Windows writes "UUID" lines (not for every document: kept, never added) and
  "UUID/<file>=sha1" for content.rtf/.styles/.comments, notes.rtf, synopsis.txt, capitals, sorted; lower-case files keep
  their form; a new .scriv gets Windows' form (before: every write lowered the keys and dropped the UUID lines).
  `Files/binder.autosave` (Windows: a ZIP of the last .scrivx) is removed only when the .scrivx is written.

## Scrivener format: keywords, custom metadata, collections (0.81.2 beta, 2026-11-12)

- **Ids**: the reader keeps Scrivener's ids as this program's (keywords "1", "2" ...; custom fields their FieldID such
  as "thisisacustommetadatatext"; arbitrary collections their UUID), unless one is empty or taken. Settings files match
  by name, never by id, so this is safe. The writer (`prepareMeta` in `ScrivenerFormat.cpp`) maps ours to Scrivener's:
  a number stays; another id (made here, or a working copy of an older version) is matched by title (and parent or
  type) with the file's, else gets the next number (keywords) or a name made of the title's ASCII letters and digits
  (fields; "1", "2" ... when there are none). List options are matched by text with the file's; new ones get the next
  number. Matching by title keeps the ids of an unchanged file, so a save without a change leaves the .scrivx as it was.
- **Values** (`scrivener::valueFromScrivener`, `valueToScrivener`): checkbox "Yes"/"No" <-> "1"/"0"; list: the option's
  ID <-> its text, "-1" <-> ""; date "yyyy-MM-dd HH:mm:ss +zzzz" <-> "yyyy-MM-dd" (written at 00:00 local time); text as
  is. A value that still means the same keeps the file's text (a date's zone). MetaDataItems are written sorted by
  FieldID (Scrivener's order); an unchanged set of values leaves the element alone.
- **Elements**: top-level `Keywords` after `Collections`, `CustomMetaDataSettings` after `StatusSettings` (`kTopOrder`);
  an item's `Keywords` before `Bookmarks`/`CorkboardAndOutliner`/`Children`; `CustomMetaData` at the end of `MetaData`.
  Each top-level element is built anew and put in only when it differs from the file's (`canonicalElement`, the same
  comparison as `sameExceptStamps`). Arbitrary collections are updated in place, or put after the file's other
  collections when added, removed or reordered; `BinderUUIDs` comma-separated; this program's search collections are
  not written (`mergeFromOld` carries them over at every re-read).
- **Re-read** (`mergeFromOld`, `matchMeta`): the .scriv's keywords, fields and collections count (`SyncState::meta` 1).
  A state without `meta` (older versions) makes `open()` read the .scriv again once even when unchanged, and joins:
  what only the working copy has is added (keywords by name and parent, fields by title and type, collections by
  name, document keywords and values without one); `fresh.meta` stays 0 until a write puts them into the .scrivx.
  Documents kept because they never reached the .scriv get their keywords and values mapped.
- **Inline notes kept as text** (`ScrivenerImporter::convertInlineNotes`): `{\Scrv_fn=...\end_Scrv_fn}` and
  `{\Scrv_annot \color={\R=..\G=..\B=..} \text=...\end_Scrv_annot}` (escaped braces in the RTF), in `readRtfFile`
  (texts, notes, snapshots), in `mergeFromOld` for texts taken from the working copy, and in `removeLeftoverMarkers`.
  Writing them back in that form is part 2 (they still become linked notes, decision "1A").
  `ScrivenerImporter::fixOldCustomValues` (called by `MainWindow::adoptProject`, Quillbinder format only) puts checkbox
  and date values an earlier import kept in Scrivener's form into this program's; list choices (option numbers) cannot
  be mapped without the .scriv.
- **Fixed on the way**: audio and video are written as Type "Media" with `FileExtension` and `MediaSettings`
  (`typeFor`; they were text documents with a stand-in RTF, also those read from the .scriv); a web page Scrivener saved
  (`.mht`, Type "WebArchive") keeps its type (`isScrivenerWebArchive`); `isPlaceholder(it)` replaces
  `isPlaceholderType` (other files and this program's HTML pages only). A text's `FileExtension` "rtf"/"txt" stays;
  project bookmarks keep the title text when the file has it. `Project` keeps a text field's `wrap` (attribute
  `wrap="0"`; it was lost at every save).

## Scrivener style marks, standard settings (0.21.2)

- `ScrivenerImporter::stripMarkers` takes Scrivener's `<$Scr_...>` marks out of a text and gives paragraphs their style: a
  heading mark "Heading n", a paragraph style mark the project's style of that name (`<$Scr_Ps::n>` = the n-th id of the
  document's `content.styles`, names in `Files/styles.xml`; `scrivenerStyleName` maps "Centered Text" to "Centered"); marks may
  span paragraphs. `removeLeftoverMarkers` cleans projects imported before 0.14 (called by `MainWindow::adoptProject`; never for
  Scrivener-format working copies).
- The writer's standard (11.0 pt, 1.09, indent 0, 6 pt after; paper #d1d495, beside the column #999999; width 500 px) is the
  members' defaults in `AppSettings` plus `standardOwnColors()`; `load()` puts it once into older settings (key `app/standard0212`,
  removed by Defaults). Scrivener imports take the formatting of new projects through `ScrivenerImporter::setNewProjectFormatting`
  (handed over in `pushToModules`), keeping the Scrivener project's most used font.
