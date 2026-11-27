# Test suites: what each one checks

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## v0990m (0.99.0)

`qb_v0990m_tests`: Speech Transcription 1.10 with the fake libqbai of `v0990h` (`QB_AI_ENGINE_LIB`, `QBAI_FAKE_CARDS` with two
cards). The add-on is loaded from `<build>/tool-plugins` (`QB_PLUGIN_PATH`, `QB_SPEECH_DIR`: build the targets `qb-speech` and
`qb-speech-impl`; no package needed). Checks: version 1.10 and `ToolModels`; the manifest's `minApp` and `offersFor` (a 0.95.0
installation gets 1.10 only with the program update); the options page (the program's AI panel, no fields for threads and
unloading, the device list Automatic / processor / the two cards, the new option); a live recording from the sample through the
fake engine (its text, the card chosen by Automatic, the registry entry, `keepRunning`, the model kept between recordings,
the program's unload request, a chosen card and the processor); Insert at Cursor (whole text, marked part, one undo step
each); Write into the text as I speak (off, on); Android's default through `QB_SPEECH_ANDROID`. Screenshots `01-options`,
`02-insert`, `03-write-as-I-speak`. A real transcription with the tiny model through the real engine is run by hand (see the
report of the job; the model is `ggml-tiny.en.bin`, never committed).

## v0990h (0.99.0)

`qb_v0990h_tests`: the AI engine packages against a fake libqbai (`tests/fakeqbai/FakeQbAi.cpp`, built as
`<build>/tests/fake-ai-engine/libqbai.so.1`: a sine for speech, a fixed text for recognition, a fake card from
`QBAI_FAKE_CARDS`, `QBAI_FAKE_SLOW_MS` to slow it down, files beginning with `FAIL` or `NOGPU` that do not load, and
`qbai_fake_stats`): no network and no engine until a button is pressed, the interface's versions, no engine installed, the
device choice (Automatic, fixed cards, the CPU), the engine loaded (`QB_AI_ENGINE_LIB`), its devices reported to `ModelMemory`,
the shared cores, tool interface 1.3's `aiEngine`/`aiPickDevice`/`aiDeviceChoices`/`aiPanel`/`aiModelFile`, a model registered
and unloaded, speaking at once and streamed, cancelling, a busy model kept, the 80 % rule, Automatic's fall back to the CPU,
recognition, the speed of speech; the catalog (`QB_AI_CATALOG`; the program's own), the downloads from a local "release"
(`QB_AI_MODEL_SOURCE`: a file in parts, a wrong checksum, going on after a stop, cancelling, a model too large), the engine
package from a file and from a "release" (`QB_AI_ENGINE_SOURCE`; `QB_REQUIRE_SIGNED` for the refusal of unsigned ones), the
panel (screenshots `v0990h-panel-*`), the package recipes (`addons/*-engine`, with the pack and `<build>/qbai/libqbai.so.1`:
stage, package, `tools/check-engine-package.py`; the handwriting engine's clear message without its pack) and, in a child
process, the real engine when built (Sonic's speed with the pitch kept; with `QB_AI_WHISPER_MODEL` (whisper.cpp's tiny English
model) and `QB_AI_WHISPER_SAMPLE` (jfk.wav) a real transcription; `QB_AI_QWEN_TALKER`/`QB_AI_QWEN_CODEC` a word of Qwen3-TTS).
The recipe checks take the packs from `QB_AI_ENGINE_DIR` and `QB_ONNXRUNTIME_DIR` (default `build/ai-engine`,
`build/onnxruntime`). Without the pack and libqbai those parts print a line and are not counted.
## v0990j (0.99.0: handwriting, part 1)

`qb_v0990j_tests`: one window with a project and a `FakeRecognizer` (a fixed text per line; `PenInput`'s clock is the test's own,
`InkCanvas::advance()` is called by hand, so nothing waits for real time except the button's long press). Synthetic
`QTabletEvent` and `QTouchEvent` sequences sent to the canvas. The defaults per system (`forceAndroid`), the plain parts (scratch
detection, `HandProtection`, `PenInput`), the button and its four looks (tap, long press, right-click, hidden by default, View >
Toolbars switch), the first use with no recognizer (File > Options > Handwriting opens, the Install slot, Russian greyed), the
three modes (the tab, the layer's mask, the pane line covered, the grid only over the pad part), writing a line in each mode, a
pen tap, the side button, finger scroll and tap, one finger writes / long press selects / two fingers scroll, the rest zone and a
tap on a button, scratching out (ink, text, strip), the fade timing (stay and fade of 400 ms), the strip (alternatives, writing
over a word, learning store, off switch, delete), Space / Backspace / Enter, Insert at the held cursor after the text moved, Pin
with check and cross (editor and synopsis), the grid styles and the size slider with its sample, the Options page, the mode
kept per device and the ModelMemory id. Screenshots `hw-*.png` of the icons, the three modes, the strip, the pin and each grid
style (`hw-grid-*.png`).

## v0990b (0.99.0)

`qb_v0990b_tests`: the AI's default cores (1->1 ... 16->12), the 50 % and 80 % rule with fake RAM and card figures
(`sysmem::setFakeRam`, `ModelMemory::reportDeviceMemory`; the clock is set by hand), the idle unload (by hand and by the timer),
the low-memory signal (injected; idle models only), the message line "Model change in progress", a tool of interface 1.3
(`tests/tool13`: registers, is asked to unload, is refused, busy models stay) and one of 1.2 (`tests/tool12`, built with the
header of 1.2), and the page File > Options > General > AI Models (default, change kept after the settings are read again,
memory line). The two test tools are loaded from `<build>/tests/tool12-plugins` and `tool13-plugins`.
## v0990l (0.99.0, item 122: the small interface)

`qb_v0990l_tests`: the rule (`smallui::belowFullHd/resolve/touchTargetPx`, both sides of 1920 x 1080, the three choices), the
View > Small Interface check item (off at first, no Android choices on Linux), the mode on and off in a wide (1280 x 800) and an
upright (1080 x 1920) window, with the inspector shown and hidden (pane sizes, toolbars and their places, menu bar, the saved
state, binder and inspector back in their splitters), the nine round buttons (size at least 9 mm, one row or column, inside
the window, theme colors, keyboard focus), each button's action (new document, bold, italic, style menu, labels and icons,
menu with all the menus and the way back), the binder and notes panels (a part of the width from the left or right, a sheet when
upright, one at a time, Close, Esc, the focus line after the slide), record only (grayed with the reason without the add-on; the
add-on is loaded in a second run of the same program, `--addon`, because the tools are loaded once per program), a profile that
keeps the mode (profile files, switching both ways) and the Android rule through `setSmallChoice` / `applySmallRule` with sizes.
The window is resized by `resize()`; the offscreen screen keeps its size. Run: `xvfb-run -a -s "-screen 0 1920x1080x24"
./qb_v0990l_tests /tmp/shots`.
## v0990g (0.99.0: the menu walk)

`qb_v0990g_tests`: one window with a project. Navigate > Inspect names (25 characters at most), the renamed commands, the
Appearance and View names alike, the Search button's menu, no lower-case start, Format > Style with 3 more styles (at most 25
entries), the item menu without grayed Convert/Merge entries, Edit > Select gone, View > Toolbars and the bars' right-click
menu in the same groups, menu depth, the mind map's Note Style menu, an Alt letter on every command, "…" rules, Find Command
without the binder-title menus, F1 in Project Statistics. Run: `xvfb-run -a ./qb_v0990g_tests /tmp/shots`.
## v0990d (0.99.0, item 118: tables, focus line, Cancel in Compile, link contrast)

`qb_v0990d_tests`: the table keys (Down and Ctrl+Return in the last row, Up in the first row, in the middle, at the end and at
the start of a text; a cell of several lines; Undo), the focus line on each part (binder, outliner, corkboard, editor,
inspector, copyholder, never two at once; its place and its 2 px color in every built-in theme, with screenshots of a light
and a dark one), the link contrast of every built-in theme x paper/background x link color (504 pairs, listed in `link-contrast.txt` in the shots folder; also with the standard olive paper), a link shown in the notes' dark paper beside the editor's light paper in "Shelin's Dark Theme" (the highlighter's per-paper color),
the note for own colors in Options > Appearance, and Compile: a project of 80 documents compiled to Word and cancelled by the
button and by Esc after five documents (the older file untouched, no temporary file, "n of N" shown, the message line), a fake
converter (`QB_PLUGIN_PATH` with a JSON definition, a script that waits and starts a helper) stopped with its helper (the
check reads `/proc/<pid>/stat`; Linux only).

## v0950f and v0950g (0.95.0, item 102: eleven languages)

`qb_v0950f_tests`: the language list (codes, own names, the machine-translation mark in the interface's language, the
locales), the language of a first start (`QB_SYSTEM_LANGUAGE` names the system's language; a child process `--child-first`
starts with no settings, and with the settings of an earlier version), the settings keeping each language, every
`translations/quillbinder_<code>.ts` (nothing unfinished, empty or missing against the German file, placeholders kept) and
its `.qm` and Qt's `qt_<code>.qm` (the compiled texts equal the .ts), the manuals (same headings as `docs/manual.md`: count,
level, order, numbers), quick start guides and welcome projects as files and resources, the objects' Choice values (the
welcome project's values are in the program's lists in that language), plural forms, the first-start page's and Options'
drop-down lists. `qb_v0950g_tests`: each language in a child process (`--child <code>`, `QB_LANGUAGE`): the manual opens at
the section asked for by its English name, the welcome project is made, the menu places its texts and the quick start guide
name are in that language's menus, F1 in the corkboard view, one Alt letter per inspector tab entry; the nine dictionary
packages (`tools/stage-dictionary.sh` fetches them from raw.githubusercontent.com into `~/.cache/quillbinder-dictionaries`,
so the suite needs the network once; license, install, found, words checked); the speech tool's language lists; the keyboard
sets "lt" and "pl".

## v0940b (0.94.0)

`qb_v0940b_tests`: the menus' places (View > Layout, Split Editor, Format > Font, Insert links, Help > Add-ons and Updates,
Troubleshooting and Development), Find Command (filter, Return, double-click, grayed commands), the first start's page,
Help > Add to / Remove from Application Menu with a fake AppImage in a temporary HOME (move, failure, `$APPIMAGE`, the
updater's `.before-update` copy), the full AppImage's window and `quillbinder --install-folder` (with and without display,
needs `build/quillbinder`), the start script and the download page's size note.

## The suites

    QB_SUITES="v040 m13" QB_SAMPLE=10 tools/run-all-tests.sh build/test-results-x11 x11   # the default run
    tools/run-all-tests.sh build/test-results-x11 x11       # the full suite, only when the user asks
    xvfb-run -a build/tests/qb_fixes_tests /tmp/shots       # one suite by hand

- `QB_SUITES` names suites as the runner prints them (`qb_tests`, `ui`, `m6` ... `m14`, `fixes`, `v017` ...).
  `QB_SAMPLE=10` adds whole suites drawn at random until about 10 % of the checks are covered (counts from
  `tests/check-counts.txt`); it prints the seed and the suites, and `QB_SEED=<n>` repeats a sample. A full X11 run
  writes `tests/check-counts.txt` anew: commit it with the job. Keep logs out of the conversation: read the runner's
  one line per suite and the FAIL lines, not whole logs.

- `qb_saveas_tests` (0.93.0, item 25: File > Save As in four ways with changes not saved, the original unchanged byte by byte, IDs, locks,
  Open Recent, later saves, the backups' fresh start, refused names, File > Export > Quillbinder Project..., the menus and the dialog).
- `qb_tests` (core, formats, compile), `qb_ui_tests`, `qb_m6_tests` ... `qb_m15_tests`
  (one per milestone; m14 printing; m15, the tutorial of 0.15 to 0.30, was removed in 0.40),
  `qb_fixes_tests` (corrections of 0.14.1), `qb_v017_tests` (0.17: icons, icon bar, fonts,
  color buttons, folder comments, mind maps, developer guide), `qb_v018_tests` (0.18: Compare and
  Transfer with Word / LibreOffice files, Mind Map Help and example board; uses LibreOffice and
  pdftoppm when installed), `qb_v019_tests` (0.19: Page Down / Page Up, crash and freeze reports,
  sessions that ended unexpectedly, saving on SIGTERM, Help > System Information and Crash Reports;
  starts `<build>/quillbinder` and ends it with signals), `qb_v020_tests` (0.20: the Objects sheet with
  templates, colors, folders and links; the data-safety fixes: control characters, unreadable text files,
  backups over 4 GiB (a sparse 4.3 GB file, about 30 s; `QB_SKIP_BIG_BACKUP=1` leaves it out), autosave
  while typing, Scrivener-format working copies, the spare copy of the project file, mind map ranges), `qb_v021_tests` (0.21:
  File > Options in Scrivener's layout and what each option does, Ctrl + wheel zoom, the inspector's zoom, mind map
  colors / shapes / icons / line menus, the Compile Overview window), `qb_v0211_tests` (0.21.1: the format bar's target,
  the editable bookmark preview and its mirror (a stress run of 400 edits; `QB_MIRROR_SEED` repeats one, `QB_MIRROR_DEBUG=1` traces),
  a zoom per part, mind map backgrounds per theme kind, places kept with the project, formatted object texts, credits), `qb_v0212_tests`
  (0.21.2: Scrivener style marks and paragraph styles, project bookmarks as Scrivener for Windows writes them, marks left in projects
  imported before 0.14 taken out when they open, the writer's standard settings put once; `QB_USER_SAMPLE=<folder.scriv>` also imports
  a real project), `qb_v040_tests` (0.40: the welcome project, the Quick Start Guide, Third-Party Licenses and About,
  "Combined Text", menu places of the guide and the welcome texts), `qb_v0401_tests`
  (0.40.1: a family per design of a font with several designs, the format buttons of a selection), `qb_v0402_tests`
  (0.40.2: no jump on a click or right-click, the look per project and the built-in themes unchanged, the border,
  the Appearance submenus of the right-click menus, project settings saved and loaded with the per-section choice,
  style changes reaching all text of the style), `qb_v0403_tests` (0.40.3: titles from the first line, No Style redefined
  from the panel, the short spelling menu, the format bar showing what is seen, dialogs above the Styles panel, the alignment
  of the main text, the chosen style list), `qb_v0601_tests` (0.60.1: Paste with Formatting, Show Invisibles, the total of a
  folder, paragraph styles over several paragraphs), `qb_v0602_tests` (0.60.2: the three ways to paste in the right-click menu),
  menu audit.
- `qb_v0604_tests` (0.60.4: the paragraph buttons of the format bar, the Wayland line of the README).
- `qb_v0605_tests` (0.60.5: a generated 500,000-word project in Scrivener format: autosave times and what it writes,
  the full save, the fingerprint, files changed in Scrivener; the right-click menu's time and size, Qt's animations off,
  the quick save when a dialog takes over). Times are printed; the limits are generous for a two-core machine.
- `qb_v061_tests` (0.61: tool interface 1.1 with a tool built for 1.0 (`tests/tool10`, the old header), Undo/Redo in the
  standard toolbar, the add-on Speech Transcription: models, headers, languages, pieces, files, the package, the button and
  its menu, the inspector tab, the options, downloads from a local test server, Vulkan through Mesa's lavapipe
  (`GGML_VK_VISIBLE_DEVICES=0`), recordings from a WAV file and a PulseAudio pipe source). Needs `tools/get-whisper.sh` and
  the add-on package; `QB_SPEECH_FAKE=1` writes "Piece n" instead of running a model; `QB_SPEECH_MODEL_FILE=<ggml model>`
  adds checks with real recognition; `QB_SPEECH_OFFICIAL=small` downloads that model from Hugging Face through the add-on
  (allowed in the cloud session since 2026-11-08: `huggingface.co` and its download host `us.aws.cdn.hf.co`);
  `QB_SPEECH_EXTRA="<wav>=<word>|<word>;..."` (with `QB_SPEECH_HINT`) adds recordings that must hold one of the words.
- `qb_v064_tests` (0.64: the spelling menu at every place of words with apostrophes, the autosave while typing on
  (a gap after 5 s, at most 30 s; about 45 s of typing), the card size kept when settings are read again, the lock with
  device and renewal, a lock taken over while open, conflicts kept as snapshot and copy (texts, the project file,
  Scrivener format), opening and closing without a change writes nothing (also a .scriv), texts not arrived yet).
  The speech add-on's Record only and Transcribe / Transcribe All are checked in `qb_v061_tests`.
- `qb_v0641_tests` (0.64.1, the fixes of the system test's first run: Markdown table cells with empty paragraphs and
  tables inside cells, Split at Selection next to and inside tables, a text only shown not written over a newer one,
  nothing replaced when the other version cannot be kept (text, notes, project file), a figure cut and pasted, compile
  progress with the event loop running, cells not justified, Merge with notes, Merge Cells / Undo / Redo / Delete Rows,
  typing in a 1,000-row table against a plain `QTextEdit`; `QB_V0641_ONLY_TYPING=1` runs only that, `QB_V0641_KEYS=<n>`).
- `qb_v0642_tests` (0.64.2, the fixes of the system test's full run of 0.64.1: word counts, a picture's format on
  letters and pasted pictures, italic from RTF face names, German quotes, Markdown paragraphs (leading spaces, the style
  Centered, a bold run ending in a space), the spare copy, project and .scriv folders that are gone, a .scriv as
  Scrivener writes it opened and closed unchanged, pictures in notes, outside changes while open, Scrivener's XML lock,
  linked notes over several formats, H2O by subscript, stale menu entries, the Documents menu, `showIfOpen`).
- `qb_v0650_tests` (0.65.0, the fixes of the external audit of 2026-11-10: safe copies, the save journal and its recovery
  (also a journal naming files outside the project), contents-by-title.txt, pictures that cannot be written, partial reads,
  Roll Back (core and inspector), incomplete backups, Duplicate, object pictures, storage conversion, same size and time,
  a 70 MB archive, command exporters (exit status, event loop), bundles and signatures (RFC 8032 vector, the release key,
  test keys made with openssl, `QB_TEST_SIGNING_KEY`, `QB_REQUIRE_SIGNED`), the scratch pad, the close question, the end by a
  signal, the lock, Scrivener format with a document and a picture changed elsewhere, the manual). Needs
  `<build>/addons-dist/tool-scratch-pad.qbpkg`, python3 and openssl.
- `qb_v0700_tests` (0.70.0 beta, the fixes of the two external audits of 2026-11-10 (report A: 31 faults, report B: 14
  items) and the writer's decisions: Markdown round trips of touching formats (4,000 random sentences), the RTF
  reader's lists, damaged RTF and other encodings read-only, control characters told, the save state and the
  failed-save notice, mind maps that cannot be saved or read, backups per project, Restore, backup progress and
  Cancel, pictures in Merge and Split, questions with a default (a source check), project settings only safer,
  the 30 s latest save, the project file of a folder with several, binder depth, duplicate ids, foreign XML,
  counts, boards, Word 97 damaged copies (needs `plugins/importers/qb_doc_dump`), reading speed, the release
  workflow's jobs, `make-package.py sign`, `launcher::coreOrigin`, the self-test, German, the dark page).
  `QB_V0700_ONLY=markdown,rtf,damaged,saves,backups,pictures,dialogs,hostile,release,german` runs parts.
- `qb_v0810_tests` (0.81.0 beta, the fixes of the audit of 0.80.0: backups after a title change, restore of any size
  (a sparse 2.3 GB file, `QB_SKIP_BIG_BACKUP=1` leaves it out; 70,000 entries), onto a full disk (child runs with
  `qbfaultshim`), linked folders, snapshot titles, unchecked saves, Empty Trash, spare copies, storage conversion, the
  ui file's name, opening with old counts on a full disk, readable explanations, the failed-save notice, First Start,
  the missed backup of a signal, the release gate; `QB_V0810_ONLY=backups,saving,window,release` runs parts).
- `qb_v0811_tests` (0.81.1 beta, the fixes of the audit of 0.81.0, part 1, the four findings at function level: a damaged
  text and Roll Back (core and inspector, the damaged file kept), Merge and Split refused, Project Replace and a style
  change leaving it out, a change kept aside, a text not arrived; pictures in notes written once; a save journal that
  cannot be read and another computer's journal (young, then old); the auditor's RTF file in a child run with a time
  limit; Roll Back shows the text at once).
- `qb_v0812_tests` (0.81.2 beta, keywords, custom metadata and collections in Scrivener format: a hand-made project in
  Scrivener 3 for Windows' form read (ids, values, collections, inline notes kept as text), opened and saved unchanged,
  changed and written (forms, ids, order), saved again unchanged, read again after changes elsewhere (made here kept,
  deleted there not brought back), a working copy of an older version joined once; a Quillbinder project switched to
  Scrivener format; the inspector's values. `QB_SCRIV_REAL=<folder.scriv>` adds checks on a copy of a real project
  (never named in the repository); `QB_V0812_KEEP=<file>` keeps the project file written in that run).
- `qb_v0813_tests` (0.81.3 beta, Scrivener compatibility part 2: a hand-made Windows-form project with styles.xml, Ps/Cs
  marks, an inline footnote, a picture in Scrivener's `\shppict` form, a picture with notes, a saved .mht; read, saved
  unchanged, the marks written in Scrivener's placement, a style made here added once, content.styles removed with the
  last style; web archives packed, unpacked (quoted-printable, windows-1252, Content-ID, a hostile archive); the page view
  and its two buttons (`links::setOpenerForTests`, the copy in the Downloads folder); a Quillbinder project switched to
  Scrivener format (inline notes, notes of media, its web page as .mht) and read back; the repair of 0.81.1's stand-ins.
  `QB_SCRIV_REAL=<folder.scriv>` opens and saves a copy unchanged (every file), writes every text anew and compares what
  reads back; `QB_V0813_DUMP=<dir>` writes the texts that differ and the written copy).
- `qb_v0900_tests` (0.90.0 beta, Scrivener compatibility part 3: list items with a style in Markdown and RTF; a hand-made
  Windows-form project `Quay.scriv` (lists, links, face-named fonts, comments, footnotes, bookmarks, targets, the deadline,
  the writing history, compile.xml, ui-common.xml, a saved search, a card picture): read, saved unchanged, every text
  written anew; changes made here (freeform corkboard, bookmarks, targets, history, saved searches, card picture, compile
  author) written and read back; the project file in English and German. `QB_SCRIV_REAL=<folder.scriv>` adds a real
  project (counts only of what it holds; never named); `QB_V0900_KEEP=<file>`).
- `qb_v0901_tests` (0.90.1 beta: menus opened 30 times keep no old submenus; File > Options: every option control
  marked with its place (P/A badge, tool tip; also in "This project" scope), the pages scroll; the keyboard back in the
  text after Options, Project Targets, Take Snapshot, Project Settings, Compile; dialogs fitted to a 1366 x 768 screen;
  placeholder contrast (light and dark); Compile's Formats column also at scale 2 (a child run with `QT_SCALE_FACTOR=2`);
  Undo names and lists; the binder's empty space; Close in Find by Formatting and Print Preview; the busy pointer of
  Project Statistics; the new shortcuts and no shortcut given twice; digit grouping; the link question of a long name).
- `qb_v0902_tests` (0.90.2 beta: an arranged main toolbar gets the theme list once (and not again when taken away); the
  download page's other downloads (Linux and Windows names); Appearance without "Page View", the color around the pages
  in Main Editor, the invisibles' color in Textual Marks; the Mind Map page's color buttons beside their labels; the
  invisibles' default and an own color in the project's look (red pixels); the theme list: placed between Redo and the
  search button, Bright / Dark / Both, the kept filter, a theme chosen there and one chosen in Window > Themes).
- `qb_v0903_tests` (0.90.3 beta: backups and sync programs: `backup::isOwnBackupName`; nine copies of a backup in the
  folder push no real backup out; what a watcher of the backup folder sees during a backup (inotify: one `movedTo`, or
  `create` + `written` for the copy of two drives, `QB_TEST_BACKUP_COPY=1`; a real second drive is `/dev/shm`); a backup
  that waits for its folder and is put in place later, a cut-off copy replaced, a same-named whole backup kept; Cancel;
  old `.part` files of the work folder; a full drive under the backup folder and under the work folder (child runs with
  `qbfaultshim`); the window's messages).
- `qb_v0913_tests` (0.91.3 beta: add-on keys; keys and certificates made with openssl and `make-package.py certify`,
  packages signed with both keys as the release workflow does; add-on key trusted for add-ons, refused for a program
  update (inspect, install, launcher), changed certificates, `make-package.py verify`, the install window's note, the
  workflow's signing job; `tools/addon-entry.py` and the workflow "Official add-on"; see "Add-on keys" in
  `saving-and-safety.md`).
- `qb_v0930e_tests` (0.93.0 beta, fifth suite, items 11, 17, 18, 30: the save text, the short messages, the link, the
  fading note in the first editor's footer bar (also the corkboard's), two editors, a narrow place (mark and tooltip), a
  long message shortened, no status bar row and the window's height; the label of `APP_VERSION_LABEL` in the manual,
  the Quick Start (as shown) and the README; `docs/design-decisions.md` (ten entries, entry 2, links, comments in the
  code); Help > Report a Problem; the README's picture. `QB_README_SHOT=<file.png>` makes that picture of the welcome
  project instead of running the checks.)
- `qb_v0930f_tests` (0.93.0 beta, fourth suite, item 39: a project of 150 documents shown one after another holds at most
  16 laid out (`DocumentStore::loadedCount`); a document shown again has its text, bold text and cursor place; unsaved text
  and unsaved notes are not unloaded and the undo history of the typing works; search, plain texts and word counts cover
  every document; a `DocumentStore::Hold` keeps all, also while the timer runs; without the window's probe nothing is
  unloaded. `QB_MEASURE=1` (`QB_MEASURE_DOCS`) prints the memory of 500 documents instead of checking; one screenshot).
- `qb_v0930c_tests` (0.93.0 beta, third suite, item 38: the restore list with backups that wait in the work folder (a
  screenshot of the list), the command Move Old Backups Into the Backup Folder (its question and report as screenshots,
  never overwriting, the folder removed when empty, the action off without a project and when nothing is left), the
  temporary names (QSaveFile leftovers at the opening, the Word writer's `.part`, the speech `.txt.part`); the sources
  of the speech add-on's logic are compiled into the suite).
- `qb_v0930b_tests` (0.93.0 beta, second suite: fixes in shared code. Exports of options and themes that keep the old file
  when a write fails (`QB_TEST_WRITE_FAILS`; the options go through a temporary file, since QSettings would mix an old
  file's entries with the new). Item 16: the picture rule of the Web Page Import (the plugin's sources are compiled into the
  suite; `QB_TEST_WEB_PUBLIC` names "host:port" entries that count as public, as every test server is on this computer),
  the 400 MB limit of install, a crafted archive whose sizes overflow, the 64 KiB request line. Item 26: the main window
  listens to no application signal while its own widgets are deleted (a probe child counts the focus signal's receivers
  then; without the fix the suite counts two and ends in a segmentation fault).)
- `qb_v0930_tests` (0.93.0 beta: the emergency copy whole or said to be incomplete ("n of m", names, README), made for a
  change of the binder or the objects alone, never into an existing folder, the end by a signal and its notice, the
  messages (screenshots);
  a folder's sync answered with EIO or ENOTSUP in a child run (`--child-save`, the fault library's `dirsync` modes), an
  unreadable journal that cannot be set aside;
  the backup note after a failed opening backup, a backup that cannot be deleted (`QB_TEST_FAIL_REMOVE`), the three forms
  of a restore. Uses `QB_TEST_WRITE_FAILS`, which also works for `safesave::writeFile` since 0.93.0.)
- `qb_v0920_tests` (0.92.0 beta: files whose time alone changed and the pattern of a sync program that touches
  every file, the notice's button, the project ID in backup names and the IDs of copies, copies that fail
  (`QB_TEST_FAIL_COPY`), states that cannot be written (`QB_TEST_FAIL_STATE`, `QB_TEST_FAIL_RENAME`), the kind of a
  failed save in the German interface (`QB_TEST_WRITE_FAILS`; the suite links `translations.qrc`), the fixed mind
  map, the shown lock (`QB_WORK_MARKS_MS`, a second device through `QB_DEVICE_NAME`), the check for updates with
  a small server inside the test (`QB_TEST_UPDATE_URL`, `QB_TEST_UPDATE_NOW`, `QB_TEST_UPDATE_RETRY_MS`,
  `QB_TEST_APPIMAGE`, `QB_TEST_NO_RESTART`; a key made with openssl in `QB_TEST_SIGNING_KEY`), `tools/update-feed.py`,
  and the translator add-on's place. Needs python3 and openssl; links Qt Network. The unreachable case waits for
  two tries and a fading note, so the suite takes about 50 s.)
- `secwatch` (2026-11-21, `tests/security_watch_tests.py`, Python only, no build): the security watch
  (`tools/security-watch.py`) offline: versions and ranges, the CVE records' ratings and versions, Qt's list,
  GitHub's advisories, whole runs against made-up sources (what is reported, one issue per project, comments,
  failed sources, a part pinned by a commit), the watch list's versions against the build's files, and the workflow's rights.
- `deps` (0.93.0, `tests/deps_packs_tests.py`, Python only, no build): the dependency packs of the Linux build
  (`tools/deps-pack.sh`): the six recipes (0.99.0: crypto, onnxruntime, aiengine) and the texts of their pre-releases (what is inside, license, source), the
  workflow's matrix, push filter and rights, the translator pack's license files against the add-on's
  `make-addon.sh`, and a pack made and fetched through a made-up `gh` (SHA-256 checked, paths made new).
  The three packs of 0.99.0 are made from made-up prefixes; the ONNX Runtime one is fetched back (a pack without any `.pc` or
  `.cmake` file, which `place` once failed on silently), and `get-onnxruntime.sh` refuses an unpinned download.
  `tools/check-crypto.sh` (not a suite: it needs the built pack) is the real check of the crypto pack.
  `secwatch` also checks `tools/verify-ffmpeg-source.sh` with keys made for the check (needs gpg).
- `qb_v0931_tests` (0.93.1: the rule that maps the generic font names "monospace", "serif", "sans-serif" on a platform
  that does not know them, with Android-like and Linux-like inputs, and that Linux maps none; the plain texts kept for
  search and counts stay within `DocumentStore::setPlainCacheBudget`, every text is right kept or not, a changed text
  frees its room; Help > System Information names each screen's usable area, the input devices by kind with printable
  names and the on-screen keyboard's state; screenshot `system-information.png`).
- `addonentry` (0.93.1, `tests/addon_entry_tests.py`, Python only, no build): the official entries' `folder` (a
  made-up repository with the add-on in a subfolder: commands, cache paths and keys, the source archive of that folder
  only, a Windows block's own folder, folders outside refused, no folder as before); the real entry of Translation and
  Proofreading against the add-on's manifest and folder (1.3, every script its commands name, models-3 last); the
  add-on's own `make-package.py` naming the program's runtime for Linux and Android.
- `v0990c` (0.99.0, item 116: toolbar sizes): the default (Normal; Large for Android through the function), the names and
  font factors; the choices in View > Toolbars (place, one group), the bars' right-click menu and Customize Toolbars; at each of
  Small, Normal and Large every bar, every button on a bar and every box has the size and font, and the bars of a row have one
  height (screenshots `size-*.png`); Reset in Customize Toolbars; the size kept across a restart; a profile switch applies it.
- `verlines` (0.93.0, `tests/version_lines_tests.py`, Python only, no build): the version line per system in
  `CMakeLists.txt` (`APP_VERSION` for Linux, `APP_VERSION_WINDOWS`, `APP_VERSION_ANDROID`: the same first two numbers,
  the build's pick and its stop on a wrong number), the shell lines of the scripts and workflows that read them,
  `tools/update-feed.py`, `tools/release-notes.py --whats-new [--system]` and `tools/download-page.py` on a copy whose
  Windows and Android lines have other last numbers, and the version jobs of the two release workflows. 0.99.0: the label
  per system (`APP_VERSION_LABEL`, `_WINDOWS`, `_ANDROID`) and a dry run with version 1.0.0 and empty labels: the tag,
  the title, the AppImage, full AppImage and update names, the shown version, the release text, the update file and the
  download page (also the number of every tag form).
- `qb_v0911_tests` (0.91.1 beta: toolbars rearranged freely by their grips, the row that got stuck; see "Toolbars
  rearranged freely").
- `qb_v0910_tests` (0.91.0 beta: backups straight in the backup folder, the seven toolbars and the icon bar; see
  "Backups in one folder, eight toolbars, theme buttons").
- `qb_audit1_tests` (0.64.1, the fixes of the external audit of 2026-11-09: ids of a project file and names in archives
  (`core/PathSafety.*`), damaged ZIP entries, a Word 97 file with a looping sector chain (compiles the plugin's `Cfb.cpp`),
  XML nested 20,000 deep, the keys a project may hold and the question of Load Options, the code warning of packages,
  links (`ui/LinkOpen.*`, also in imported web pages), the socket folder, Scrivener's lock before every write and a .scriv
  file whose other version cannot be kept, the computer's name; `QB_RELEASE_BINARY=<files>` (':'-separated) searches a
  release build for the test switches' names).
- `qb_v0611_tests` (0.61.1: the quote rules, Convert Quotes from File > Options > Corrections with every place and the
  scope, Project > Delete Old Snapshots with its question, Scrivener format: deleted snapshots leave the .scriv).
- `qb_v062_tests` (0.62: settings per project: the first start window, File > Options with "Settings for", what goes
  where (also changes made outside the window), Use Program Settings, the switch "which settings come first" and the look
  following it, the Options section of project settings files; tables and pictures: designs, the Markdown and RTF forms
  and RTF of other programs, the editor's table and picture commands in both storage formats, Scrivener format, the
  needed version 0.62.0, compile to PDF, Word, e-book and Markdown; LibreOffice opens the Word file when installed,
  `QB_SKIP_SOFFICE=1` leaves that out). The runner sets `QB_SKIP_FIRST_START=1` for all other suites.
- `qb_w2_tests` (Windows port W2, on Linux: the packages' platform field, the process check, pruning reports with
  their memory dumps; no window).
- `qb_v0603_tests` (0.60.3: folder totals in the binder and the footers, Inspector Background, the color of the toolbars,
  font names that are not families, double-click beside the text reveals the document).
- Printing checks print into PDF files (Print to File) and read them with poppler-utils.
- The add-on Media Playback needs FFmpeg: `tools/get-ffmpeg.sh` once (a few seconds from the pinned pack, 5 to 10
  minutes from source without one), then the test runner builds the add-ons.
- Scrivener format: `tests/scrivener_checks.cpp` reads a real Scrivener 3 project when
  `QB_SCRIV_SAMPLE=<folder.scriv>` is set (a public sample: GitHub
  writerslogic/scrivener-mcp, tests/sample-project.scriv, pinned to commit d4d42c3 of 2026-11-08 by a hash over its
  files, `kTreeSha256`; since 2026-10-31 it has no docs.checksum, which the program writes only when a .scriv has one).
- Offscreen runs have no window manager: a widget has focus only after
  `activateWindow()`; modal dialogs are answered with `QTimer` + `activeModalWidget()`.
- Never run `pkill -f` / `pgrep -f` with a pattern that also appears in your own
  command line: it kills the shell that runs it.

## The Translation Helper's own suites (add-on 1.6, 0.99.0)

The add-on is a project of its own (`addons/translation-proofreading/source`), so its suites are not in `run-all-tests.sh`: the
release workflow (`official-addons/translation-proofreading.json`) runs `ctest` there (logic, ui, words, languages, plugin,
exports and the dictionary tool). Build it by hand against the pack of `tools/get-translator-deps.sh`:
`cmake -S addons/translation-proofreading/source -B <dir> -G Ninja -DTP_TEST_SWITCHES=ON -DTP_DEPS_DIR=build/translator-deps/install`,
then `QT_QPA_PLATFORM=offscreen ctest` in `<dir>`. `memoryGuard` of `tp_ui_tests` needs `TP_MODELS_DIR` (the models of the releases):
without it that one check fails and 15 are skipped. 1.6 added `registryWithAnInterface13Host`, `settingsPageByInterface` and
`interface12KeepsTheOldWay` to `tp_lang_tests` (a fake program with a model registry: `tests/FakeHost.h`, `version` 130).
