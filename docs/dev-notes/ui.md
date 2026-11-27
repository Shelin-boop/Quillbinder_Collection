# The window: toolbars, themes, editor, menus, options

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## 0.99.0: menu names

- One name per command: the Appearance menus (`LookMenu.cpp`) and the toolbar buttons' menus use the command's own text; an
  alias made by `menuAlias` is only for a short name inside a submenu of its own topic (Open, Split) and starts with a capital.
  "From" and "With" are capitalized (APA title case); a command ends in "…" only when a window opens first and asks.
- `fixMnemonics` (MainWindowBars.cpp) now also gives a letter to commands without one: an action with an object name that has
  no ":" and does not start with "qb", and the menus that carry the property `qbWantsMnemonic` (Label and Status: their
  titles are the project's words). Data lists (styles `style:...`, documents, files, themes) stay without.
- Format > Style (`fillStyleMenu`) lists `stylelist::shown`; with more than ten entries it is `stylelist::addTo`'s scrolling
  list, which shows no shortcuts (the commands `style:...` stay on the window, so Ctrl+Alt+0 to 9 work as before).
- `fillItemMenu` adds Convert and Merge only when the command is enabled (it must run after `updateActions`).
- Find Command (`CommandFinder.cpp`) skips menus by object name (`goToMenu`, `openQuickRefMenu`, `historyBackMenu`,
  `historyForwardMenu`): they are filled with binder titles when they open. A new menu of that kind needs its name in the list.
- The misspelled word's menu: `lookmenu::add(menu, part, parent, before)` puts Appearance in front of the separator that
  precedes "More Commands" (the hook finds the short menu through the parent of the menu named `spellingMore`).
- Mind map Format > Note Style: the styles are the entries; Redefine and Delete ask in a small `QInputDialog` list (a
  submenu with the styles would be the fourth level). Menus go three levels deep at most (suite `v0990g`).
- `Inspector::tabName` is the short name for Navigate > Inspect; `tabToolTip` stays the tab buttons' long text.

## 0.94.0 beta: one place per topic in the menus, Find Command, the first start's page, the application menu entry

- **Menus** (`MainWindowBars.cpp`, `createMenus`): `m_layoutMenu` is now View > Layout (`paneLayoutMenu`): the positions
  (sections "Upright window", "Wide window"), the section "Saved layouts" and Save Current Layout / Manage Layouts;
  `rebuildLayoutActions()` fills all of it (it is also called inside `createMenus`, so that the commands are in the menu
  before `commands::addFromMenuBar`). The toolbar's View and Layouts buttons use the same menu. Window has no Layouts.
  Navigate > Fonts is in Format > Font (`createFontMenu(QMenu *fontMenu)`); the link commands are in Insert; Help has
  the submenus `addonsMenu` (Add-on Manager, Install Update or Add-on, the check for updates) and `troubleMenu`. The
  actions' object names did not change (saved shortcuts, toolbars and the suites use them).
- **Find Command** (`CommandFinder.*`, action `findCommand`, no shortcut: Ctrl+Shift+P is the mind map's Page Setup):
  `collect(menuBar)` walks the menus as they are (lists filled when a menu opens are not in it), `matching()` takes the
  words of the text in any order against the label and the menu path. A disabled command (or one in a disabled menu) is
  listed grayed and `runRow` refuses it.
- **The application menu** (`AppMenuEntry.*`, namespace `appmenu`): `add(moveAppImage, tools)`, `remove()`,
  `moveToApplications()`. A move sets `$APPIMAGE` (and `QB_TEST_APPIMAGE` when it named the old file) to the new path
  with `qputenv`: everything that reads the variable later (`UpdateChecker::ownAppImage`, `Launcher::startProgram`,
  `Host::commandLine`, System Information) follows. `app/integrated` and `app/integrationDeclined` are renamed too.
  Help > Add to / Remove from Application Menu: `updateApplicationMenuAction()` on opening Help.
- **The first start** (`FirstStartDialog`, `main.cpp`): one page; `main.cpp` carries out the answers after `take()`
  (`UpdateChecker::firstStartAnswered`, `MainWindow::applyFirstStartMenuChoice`). `w.show()` waits (`windowLater`) when
  the first-start page or the welcome project is due and is done when the project is open (`QB_NO_DIALOGS` keeps the old order).

## 0.93.0 beta (2026-11-24): the save text in the first editor's footer bar, no status bar row (item 11)

- **`FooterStatus`** (`src/ui/FooterStatus.*`, no Q_OBJECT): one in each `EditorPane` (`footerStatus()`, `MainWindow::editorPane(i)`),
  only the first editor's (`m_panes[0]`) is filled. It holds the label `saveState` (the save text, or a short message
  while one is shown), the label `backupState` (the link "No backups") and the `fadingNote` labels. It lays its labels
  out by hand from its width: the whole text; a message shortened with `elidedText` and the whole message as tooltip;
  below that a "●" mark (green, yellow, red for saved, not saved yet, failed; the highlight color for a message) with the
  text as tooltip; the link becomes "!" when even that is tight. `minimumSizeHint` is 0 wide, so it never widens a footer.
- **Where it sits**: `EditorPane::placeStatus()` moves the one widget into the layout of the footer page shown (text:
  the stretch between the page label and the word count; corkboard and outliner: the left half after their buttons; picture
  and PDF: after the zoom buttons; player: the end of the controls' row; empty: alone); it runs at the end of `applyMode`
  and when playback starts or ends.
- **`MainWindow`**: `showStatusNote(text, ms)` replaces `statusBar()->showMessage` (0 ms = until `clearStatusNote()`; the
  toolbar drag's hint uses it), `statusNote()` reads it, `showFadingNote` goes to `FooterStatus::showFading`. The window
  never calls `statusBar()`, so there is no row (`findChild<QStatusBar *>()` is null); `placeBesideText` no longer
  subtracts the row. Toolbars at the bottom edge are dock areas of the window and are unaffected.
  The status bar's tests read `w.statusNote()`; the crash-safe system test too.
- No project: both labels empty and hidden. Composition mode has its own window: nothing there.

## 0.93.0 beta (2026-11-23): the panes placed by the window's shape

- **Where**: `ui/MainWindowPanes.cpp` (`MainWindow::initPaneSplitters`, `arrangePanes`, `applyPaneSizes`, `capturePaneSizes`,
  `followWindowShape`, `createPaneActions`; the state is `m_upright`, `m_wideOrder`, `m_uprightEditorFirst`,
  `m_uprightBinderLeft`, `m_wideSizes[3]`, `m_uprightEditorShare`, `m_uprightBinderShare`). `m_mainSplit` is the one splitter
  below the toolbars: three columns (horizontal) when wide, two rows (vertical) when upright; `m_sideSplit` is the row of the
  binder and the inspector (upright only; when wide it hangs hidden under the window with no children). The three panes are
  moved with `QSplitter::insertWidget` (a moved pane forgets that it was hidden: `arrangePanes` sets it again from the
  switches `showBinder` / `showInspector`; `updatePaneRow` hides the side row when both are hidden).
- **Shape**: `wantsUpright()` is height > width (a square is wide); with the on-screen keyboard showing, its height is added,
  so a tablet held upright does not turn "wide" when the keyboard takes room. `resizeEvent` queues `followWindowShape` (a
  zero timer: no re-parenting inside a resize). The wide shape's least width is 808 px (binder 100 + editor 389 + inspector
  311 + lines), so a window asked for 736 px is held at 808 until the shape has changed; the upright window's least size is 525
  x 578 (the binder 100 and the inspector 311 fit beside each other from 525 px).
- **Stretch**: the editor's row has stretch 1 and the side row 0 (a shorter window, as with the keyboard, takes the height from
  the editor's row only). Sizes are set in pixels of the splitter's present size (weights that do not add up to it gave
  wrong rows).
- **Kept**: per device, in `QSettings` (never in the project): `window/paneOrder` ("BEI" ...), `window/uprightEditorFirst`,
  `window/uprightBinderLeft`, `window/uprightShares` (editor row of the height, binder of the side row), and `window/mainSplit`
  (now by pane: binder, editor, inspector; before by place, the start order is the same). Sizes are taken over (`capturePaneSizes`)
  when the writer moves a line (`splitterMoved`), when a choice changes and at closing; not at every resize, so that a
  window made narrow does not change what the wide shape remembers. A size set by code does not count unless it calls
  `capturePaneSizes` (tests move a line with the mouse events: `QSplitter::moveSplitter` is protected).
- **Named layouts** (`captureLayout` / `applyLayout`) keep their shares of the wide shape by pane (`paneSizes()`); applied in an
  upright window they are stored for the wide shape. They carry no order.
- **Menu**: View > Layout (`paneLayoutMenu`, actions `layoutEditorMiddle`, `layoutEditorBottom`, `layoutBinderLeft`,
  `layoutBinderRight`, `layoutOrder<BEI...>`, `layoutReset`), radio groups, always enabled. Test: `tests/v0930d_tests.cpp`.
## 0.93.0 beta: documents that are not shown are unloaded (item 39)

- **Where the memory went** (measured with `qb_v0930f_tests`, `QB_MEASURE=1`, `QB_MEASURE_DOCS=<n>`: a generated project of 500
  documents of 1,000 words, shown one after another through the binder; `mallinfo2` for the heap in use, `VmRSS` for the
  process): before, 168 kB of heap per document shown (the 500 documents: 16 MB when opened, 100 MB at the end, RSS 104 to
  156 MB); of it about 61 kB is the `QTextDocument` itself (read with `DocumentStore::text` only, no editor) and about
  107 kB the layout of the editor that showed it (and the spell check's block data). Nothing gave it back: `DocumentStore`
  kept every `Entry` until the project closed. After: 24.7 MB heap in use at the end (16.4 MB at the opening), RSS 106 MB,
  8 to 30 documents held at a time.
- **What is unloaded** (`DocumentStore::unloadUnused`): the whole `QTextDocument` (layout, spell check data, text) of a
  text or notes document that is not shown, has no unsaved change, and has no undo or redo step. The entry is erased
  (`aboutToDeleteDocument` first, as `forgetDocument` does; the editors let go of it), the plain-text cache for its key
  is dropped, and the next `text()` / `notes()` reads the file again (cursor places are in `ViewMemory`, by document id,
  not in the document). The `kKeepLoaded` (8) documents used last stay, so that jumping back and forth costs nothing.
  Kept always: shown ones, one with an undo or redo step, one that is read-only for its file (`m_unreadable`,
  `m_damaged`, `m_shown`, `m_missing`). A document that is only marked as changed by showing it (0.64.1) is compared with
  the bytes it was read from (`sameAsRead`, at most 40 per run, the timer comes again in 300 ms); a really changed one stays
  until it is saved.
- **The undo history** is the document's own (`QTextDocument`'s stack, `UndoNames` hangs on it), so a document with steps is
  not unloaded: its layout stays too (the stack cannot be kept without the document, and the layout cannot be freed from
  outside Qt's own classes). Every document the writer has edited in the session therefore stays until the project
  closes; documents only read or shown are the many ones in a large project, and they go.
- **Who says what is shown**: `DocumentStore::setShownProbe`, set by `MainWindow::adoptProject`: every `QTextEdit` and
  `QPlainTextEdit` of the program (the editors of any pane, Combined Text, quick references, the inspector's notes and
  snapshots), the sources of every `DocMirror` (the inspector's bookmark preview); false while a modal window is open.
  Without a probe (tests without a window, a project the window does not own) nothing is unloaded. **Android does
  unload**: `src/main.cpp` builds `qb::MainWindow` there too and `adoptProject` sets the probe without an `#ifdef` (this note
  said the opposite until the review). The Android run has to check: a project of some hundred documents browsed one
  after another holds a bounded number (`loadedCount`), text typed on the tablet and not yet saved is kept, a document
  shown again is as it was, and the on-screen keyboard's composing text is not lost when the document is shown again.
  The timer (1.5 s, from a document read while more than 8 are held) is in the store; it comes again after 3 s when a
  window is open, a job holds documents, or a nested event loop runs (`loopLevel() > 1`: a menu, a drag, a native
  dialog the probe cannot see), and once after each save (`Project::saved`, `armUnloading`), which clears the marks of
  documents that were only shown.
- **Review of 0.93.0 (the analyst's six points)**: (1) the timer comes again after a refusal (above); (2) a document found
  really changed (`sameAsRead`) is not compared again until it changes (`Entry::changes`, `comparedAt`), the compares of one
  run take about 15 ms (`QElapsedTimer`), and the timer comes again only when some were left (`comparesDone()` counts them
  for the tests); (3) `textEdited` is emitted only when `doc->isModified()` (the spell check's first pass on a document read
  anew fires `contentsChanged` without a change), and `snapshots::sameAsNewest` keeps the snapshot "Saved" from copying the
  newest snapshot; (4) Android (above); (5) on unloading, `{read, rtf, stamp}` are kept in `m_kept` (documents whose file
  is larger than `kKeptMax`, 512 kB, are not unloaded at all); when the file cannot be opened or is cut short at the next
  `readFile` and its stamp is still the kept one, the kept bytes are the text, else the file is read as before (0.64,
  0.92: unreadable, shown empty and read-only). The test switch `QB_TEST_READ_FAILS=<part of a path>` makes such a file
  unopenable (a root user ignores permissions); (6) a nested event loop refuses the run, and the entry is removed by its
  key after `aboutToDeleteDocument`.
- **`DocumentStore::Hold`**: a job that keeps documents in hand and runs the event loop (progress windows with
  `processEvents`: Compile, Replace in Project, Convert Quotes, the style and script updates, the import) creates one;
  nothing is unloaded while one lives. A new such job needs one.
- Also fixed on the way: `m_plainCache` for a notes key was not emptied when the notes were edited (a notes text
  unloaded, read again and changed would have shown the older plain text); the key is emptied at every change now.
- Not unloaded: documents with undo steps (above). The plain-text cache (`m_plainCache`, the text of every document
  asked for by search or counts, about 16 kB per 1,000 words) is bounded since 0.93.1 (to-do item 45):
  `DocumentStore::kPlainCacheBudget` characters (4,000,000, about 8 MB); past it a plain text is read from its file and
  not kept. The first texts asked for stay (no eviction): a search that walks every document of a project larger than
  the bound would otherwise throw out each kept text before it is asked for again. A changed text drops its entry
  (`dropPlain`), which frees its room.

## 0.93.0 beta: notes start as plain text (item 24)

- A notes document (`DocumentStore::load(id, true)`, also `projectNotes()`) is **plain** when it is new or empty, or when
  its file carries the mark: document property `display::kPlainNotesProp` ("qbPlainNotes"). `display::applyPlainNotes` sets the
  interface font (`plainNotesFont`, at the notes' zoom), an empty default paragraph and (Markdown) flattens the normal
  paragraphs. `setNotesZoom` and `applyFormatting` keep plain notes in that font and do not normalize them.
- **The mark in the file**: RTF `{\*\qbplainnotes}` after the header (an ignorable group), Markdown a first line
  `<!--qb-plain-notes-->`; `readFile` and `deserialize` take it off and set the property, `serialize` puts it back, so the
  checks that compare bytes (`unchangedSinceRead`) agree. Notes with text and no mark are read as before (project look).
- `TextEditor::plainNotes()`: Enter on an empty list item gives a plain paragraph; `insertFromMimeData` inserts text only
  (no formatting) unless `m_pasteKeepAll` (Edit > Paste with Formatting, the existing `pasteKeep` command).
- Suite `notesplain`.

## 0.93.0 beta (2026-11-23): the overview of the mind map

- **`ui/MindMapOverview.*`**: a child of the board view's **viewport** (so it takes mouse events and the wheel itself;
  the wheel is ignored so that it travels on to the view as over the board, `zoom::markOwn` of the view applies).
  `MindMapView` owns it (`overview()`), follows it from the scroll bars, `zoomChanged`, `resizeEvent`, the scene's
  `changed` and `boardChanged` (`refresh()`: place, size, shown or not, repaint). Not a scene item, so
  `MindMapScene::render` (PNG/PDF export, print) never holds it.
- **The map**: the area is `contentRect()` united with `visibleRect()` (the frame stays inside when the view is moved
  past the content); scale and origin come from it (`map()`). Painted from the board and `liveRect(id)` (shapes, lines,
  notes; text only when the font would be 5.5 px or more, else the colored frame). While a drag runs the map is frozen
  (`m_frozen`), or the area would change under the pointer. A click centers (`centerOn`), a press inside the frame
  keeps the grab offset. A locked board moves too: only the view changes; the scroll bars feed `setViewMemory`'s timer.
- **Shown or not** (`wantedShown`): "always" (setting) -> shown; else shown when `visibleRect()` does not hold
  `contentRect()`, hidden only when it holds it with a margin of 10 scene units (Zoom to Fit leaves 30), in between the last answer stays (no flicker).
- **Settings** (`overview::` in the same files, plain `QSettings`, per device, not in the project): `mindMap/overviewAlways`
  (bool), `mindMap/overviewSize/<kind>` (QSize; kind = the view memory's "inspector" or "window"). `overview::hub()`
  signals `modeChanged` to every view and to every controller's command. `sizeFor` is the size rule (a fifth of the
  width, at least 80 px wide, at most half the view); the overview's own size is `fittedSize`: the writer's width, the height from the area's proportions (at most half the view, the width shrinks with it); the stored height is not used; the grip is 22 px, 40 px with a touch screen or on Android.
- **The command** `mm_overview` (checkable, checked = always; its tool tip is "Overview: automatic" or "Overview: always
  shown", set in `updateActions`) is in `fillViewMenu` (the large window's View menu and the inspector's Menu > View) and is
  the default action of the bar's button `mmOverviewButton`, right after `mmMenuButton`. Icon `qb-mm-overview` (made by
  `tools/make-item-icons.py`).
- **The "Fixed:" text** (`mmFixedTitle`) is an `ElidedLabel` inside the bar (after the overview button, before the
  stretch), shortened to its room with "..." (full title in the tool tip); no row under the bar any more.
- Suite `mmoverview` (`tests/mmoverview_tests.cpp`).

## 0.92.0 beta (2026-11-20): the marks of a binder row, the fixed mind map, the shown lock

- **Row marks** (`ui/RowMarks.*`): the pin and the padlock are painted (no picture files), left of the label dot:
  `rowmarks::lockRect`, `pinRect` (the same functions for painting, clicks and tool tips; `BinderTree::pinRectOf`,
  `lockRectOf` for the tests). The tree tracks the mouse (`setMouseTracking`, `m_hover`); a press on a pin is
  swallowed (`m_pinPressed`: no selection, no drag) and emits `mindMapPinClicked`. Tool tips in `viewportEvent`.
- **The fixed mind map**: the main window keeps `m_fixedMindMapId` (`fixMindMap`, `releaseMindMap`,
  `applyFixedMindMap`), in the project's view state (`uiState["fixedMindMap"]`, per computer, so the project file
  is not touched). `Inspector::setFixedMindMapId` makes `refreshMindMap` show that item's board and
  `updateTabDots` mark its state; `MindMapPanel::setFixed` shows the line and the pressed button (the bar is built
  anew with every board, `applyFixed` sets it again). `BinderPanel::setFixedMindMap` shows the bar above the
  footer. Released in `applyFixedMindMap` when the item is gone or in the Trash (the model's rows signals queue it).
  Commands `fixMindMap`, `releaseMindMap` (Navigate > Inspect), enabled by `updateMindMapActions`.
- **The shown lock** (`core/WorkMarks.*`, `MainWindowSync.cpp`): one file per device beside each lock file,
  `<lock>-work-<machine key>.json` (`device`, `key`, `opened`, `alive`, `docs`). Written a few seconds after the
  list changed (`noteWorkChange`, `m_workDebounce`; the list: the documents changed since opening and each pane's
  current document), never for the same list; removed in `releaseLocks`; left out of backups (`Zip.cpp`). Its sign
  of life is the lock's while that device holds the lock; a device that lost the lock (`m_lockLost`) renews the file
  itself every `renewIntervalMs`. `workmarks::others` drops lists older than `staleAfterSeconds`. Read every 30 s
  (`m_workPoll`; `QB_WORK_MARKS_MS` in test builds), which writes nothing.

## Toolbars rearranged freely (0.91.1 beta, 2026-11-13)

Reported on 0.91.0 (Kubuntu, Wayland): no order change, no way from one row into the other (nothing
moved), and a row was stuck with Search pulled to the far right. This section replaces what the 0.91.0 section
below says about the drag, the gaps and `window/state`.

- **Causes.** (1) 0.91.0 had no change of order at all. (2) A way into another row was Qt's own toolbar drag, and on
  Wayland Qt runs it as a desktop drag and drop (`QMainWindowLayout::needsPlatformDrag`, `QDrag::exec` in
  `performPlatformWidgetDrag`); on the writer's desktop nothing moved (which step fails there: not known, no Wayland
  in the workspace). (3) Stuck: `moveBarSideways` took room only from the row's last bar (`last->width() -
  sizeHint`), never from the gap after the dragged bar; with Search at the far right no bar before it could move.
- **`ui/BarPlaces.*`** (no widgets, the rules): `Places` = rows of `Slot{name, lead}` at the top and bottom edge, as
  they show from the top down; `lead` = free space in front of a bar (none for a row's first). `toText`/`fromText`
  (one line per row, `top|mainToolbar:0 themesBar:0 searchBar:120`), `normalized` (every bar once, unknown dropped,
  a missing bar behind its standard neighbor, no empty row), `moved` (a bar to a `Target`: row + `before`, or a new
  row; the bars after its old place close up, its lead goes to the next bar that shows, it arrives with lead 0),
  `fitted` (leads shrink in proportion in a row that is too narrow), `shifted` (a bar along its row: left to the end
  of the bar before it, right until the bars after it stand together at the row's end; the others keep their places
  until it reaches them).
- **`ui/BarDock.*`** (a child of the window, `findChild<BarDock *>()`): keeps the places in **`window/barRows`**
  (`load` at the start after `restoreState`; without the key, after 0.91.0: `placesOfWindow()` + `window/barGaps`),
  `arrange()` adds every bar anew in the places' order (`addToolBar` moves a bar that is already there without hiding
  it; a break between rows; the bottom edge's rows from the window's edge upward, as Qt counts them), `refit()` shows
  the leads as gaps (`GripBar::setGap` of the bar on the left) and runs after a resize, a bar's show or hide and a
  layout request. It still writes `window/barGaps` for 0.91.0. `placesOfWindow()` reads the rows from the window's
  layout (`layout()->itemAt`, `toolBarArea`, `toolBarBreak`); the tests compare it with `places()` and with what shows.
- **The drag** (`pressed`/`movedTo`/`released` from `GripBar`'s signals): `GripBar::event` keeps every press, move and
  release on the grip to itself, so **Qt's toolbar drag can never start** (do not hand events on the grip back to
  `QToolBar`). Positions are the bar's own, mapped to the window: no screen coordinates, no window of its own, no
  mouse grab. Two modes, decided per mouse move from the row's geometry as the drag began (`Drag::bands`):
  **Shift** (the mouse in the bar's own row, between the middles of its neighbors, or still pushing the next bar
  ahead): the gaps change at once; **Place** (past a neighbor's middle, another row, the strip of `kZone` px under
  the top rows or over the bottom edge, the status bar): nothing changes while the button is down, the marker
  (`barMarker`, a 3 px line in the highlight color) shows the place, `moved()` + `arrange()` at the release. A place
  that changes nothing is none (forbidden pointer). Esc (application event filter while a drag runs), the window's
  deactivation or the bar's hiding end a drag without a change. The first mode needs the gaps of the row as it shows
  (read from the bars' positions at the drag's start); at its end those are what the places keep.
- `MainWindow`: `m_barDock` is made after `restoreState` (the icon bar exists by then); `resetToolbarPlaces` =
  `BarDock::reset`. `placeToolBars()` is only the first placing and the one-time step for states older than 0.91.0.
  `moveBarSideways`, `saveBarGaps`, `loadBarGaps` are gone. The rows' switches still go by the bar, not by the row
  it stands in.
- Tests: `qb_v0911_tests` (the rules; Qt's drag out of the game; the stuck row; order; rows; a new row; the bottom
  edge; Esc; a drop on nothing; kept and found again; the step from 0.91.0; Reset; a hidden bar; a narrow window; 60
  random drags, then every bar still movable). With the room rule of 0.91.0 put back into `shifted`, the checks named
  "THE REPORT" fail (tried in this job). `qb_v0910_tests`: its drag checks follow the new drag.
- Test drags: tell every mouse step from the window's coordinates (`bar->mapFrom(&w, p)`), as the bar may move. A
  drag far to the left or right of a neighbor's middle is a change of order now, not a shift.
- Not checked here: the drag on Wayland (the workspace has X11 only); a real mouse release outside the window; the
  Windows machine (a new suite goes into `EXPLORE` of `win-tests.sh` first; not done in this job, PR #65 touches
  that file).

## Backups in one folder, eight toolbars, theme buttons (0.91.0 beta, 2026-11-13)

Three requests of the writer in one job (a new AppImage; Linux published as `v0.91.0-beta`, Windows built afterwards
as an experimental test build).

- **Backups straight in the backup folder** (`core/Backup.*`; a backup folder is chosen so that
  all the backups go in there). `Where::BackupFolder` (was `ProjectFolder`) writes into the folder itself; a project's
  backups are the files in the program's own name form whose title part (`ownTitle`: what stands before the last
  " bak <date time>") equals `text::safeFileName(p->title())` (`ownBackupsOf`): only they count for `keep` and can be
  deleted. Compared as text, never as a file pattern (a title may hold `[ ] ? *`). Two projects with the same title
  share backups and `keep` (the writer's choice, as before 0.70.0); after a title change the old title's backups stay.
  `backupsFor(p, dir)` (Restore From Backup) also lists other files that begin "<title> bak " (sync copies).
  The folder per project of 0.70.0 to 0.90.4 is `formerProjectFolder` ("" when there is none): never written, counted
  or cleaned any more, its backups listed by `formerBackupsFor` and shown by Restore with " (in the folder ...)".
  Backups that waited for it (work folder of `<dir>/<title> <key>`) are put into the backup folder by the next backup,
  and that work folder goes. One work folder now serves every project of a backup folder: of the waiting backups only
  those of this title are trimmed to `keep`. Show Backup Folder opens the backup folder. Do not bring the folder per
  project back without the writer's word.
- **Seven toolbars and the icon bar** (`ui/ToolbarConfig.*`, `MainWindowBars.cpp`): `toolbars::Bar` = Main, Format
  (style and font; both keep their numbers and the object names "mainToolbar", "formatBar"), Themes, Search, Tools,
  Character, Paragraph (`allBars()` gives the window's order; `barKey`, `barName`, `barObjectName`, `barCommand`).
  `MainWindow::m_bars[]`; each a `GripBar` (`ui/GripBar.*`), `setMovable(true)`, not floatable, top and bottom edge
  only. **The grip**: `GripBar` paints two columns of dots over the style's handle (`gripRect()` =
  `SE_ToolBarHandle`) and shows the move pointer there. Dragged out of its row, Qt unplugs the bar and puts it into
  another row or at the other edge (rows and edges are in `window/state`, `saveState`). Dragged along its row, Qt would
  move the border between the bar and the one before it: the dragged bar shrinks and its buttons go into the overflow
  menu while the bars after it stay (seen in this job's first screenshots). So `GripBar::event` keeps the sideways
  moves to itself (it returns true for them and hands the events to Qt only once the mouse has left the row), and
  `MainWindow::moveBarSideways` changes the **gap** of the bar before it in the row (`GripBar::setGap`: a spacer after
  the last button, on the bar only while there is a gap; `window/barGaps`, read by `loadBarGaps`). The bars after it
  move along, the row's last bar (which Qt stretches to the edge) gives the room, and a bar goes right only as far as
  that room. This is what the writer called adjustable separators. Qt's own `moveToolBar` must stay unused: it sets
  preferred sizes that would fight the gaps. Tests drag with mouse events told from screen positions (the bar moves
  under the mouse between two events). `placeToolBars()` = the
  standard places (two rows), run once after an older state was restored (`window/barPlaces0910`) and by the command
  `resetToolbarPlaces` (which also closes every gap). **Two levels of switches**: `showToolbar` and `showFormatBar` switch a row's bars as before
  (layouts, Composition Mode and their tests rely on them); each bar has a command `showBar<Key>` kept in
  `AppSettings::toolbarHidden`; `applyBarVisibility()` shows a bar when both are on and disables a bar's switch while
  its row is off. Never call `setVisible` on one of the seven bars elsewhere. `createPopupMenu()` is overridden
  (Qt's own menu would switch bars past the switches); `fillBarsMenu` fills it and the bars' right-click menus.
  Lists: `AppSettings::toolbarList(bar)` (members `toolbarMain`, `toolbarFormat`, `toolbarThemes` ...), settings keys
  `toolbars/bar-<key>`, `toolbars/hidden`. The keys `toolbars/main` and `toolbars/format` of earlier versions are read
  once (`toolbars/split0910`), split by `toolbars::splitFormer` (an item goes to the bar that has it in the standard
  sets, an unknown one with the item before it, spaces dropped; a bar that gets nothing from an arranged list is
  switched off) and never written again, so an older version started afterwards finds its toolbars. The one-time
  additions of 0.60.4 and 0.90.2 run inside that step. An empty list means the standard set, so a bar left empty in
  Customize Toolbars is switched off instead. Controls (`specials`) belong to the main toolbars or to the format
  toolbars and exist once: `ToolbarDialog::addItem` moves one from the bar that has it. Tool buttons of writing tools
  stand on the Tools bar (`placeToolButtonsOnce` on `toolbarTools`). A new bar needs: the enum, `allBars`, the names,
  a standard set, a member in `AppSettings`, the menu line in `createMenus`, a German name.
- **Theme buttons** (`ui/ThemePicker.*`): Bright and Dark show the theme assigned to them (`window/themeBright`,
  `window/themeDark`; "light" and "dark" at first; right-click assigns), U1 and U2 are the own themes `user:User_1` and
  `user:User_2` (`store`: `theme::currentLook()` saved under that id). `press` shows a theme as it is
  (`themeui::useAsItIs`), after `keepOwnLook`: own colors and fonts on top (`themeui::ownChanges() > 0`) go into an
  empty U button without a question, else the writer is asked (Save in U1, Save in U2, Switch Without Saving, Cancel;
  `setAnswerForTests`). The list holds every theme and still runs `themeui::choose`. A fresh program has two own
  colors (the standard paper, `standardOwnColors`), so the first click on Bright or Dark fills U1. The filter of
  0.90.2 (`window/themeFilter`) is not read any more.
- Tests: `qb_v0910_tests` (backups: what waited for a project's folder, titles with special characters; toolbars:
  standard places, contents, a bar dragged along the row by its grip with mouse events (the bars after it make room,
  the row's end is the limit, back to the left), the bottom edge, the window state, Reset, the switches, the popup
  menu, Customize Toolbars); the theme buttons and the split of an arranged main
  toolbar are in `qb_v0902_tests`; changed for the new behavior: v0700 and v0810 (backups by title), v0903, v021, ui,
  v061, v0601, v0604, v017.
- Not checked here: dragging a toolbar on Wayland (the checks run on X11). The Windows machine (run of 1614366, the
  first of the branch `win/0.91.0`, 48 min): no compiler error, program 26, setup 11, layout 8 checks ok, 34 suites
  with 0 failures; v0910 passed at its first run there (32 checks, the drags included) and is counted since.

## The writer's list after 0.90.1 (0.90.2 beta, 2026-11-13)

- **No P/A badges**: `optstore::mark` keeps only the key and the base tool tip (no badge, no "storageHolder");
  `refresh` sets the tool tip and the control's property `qbPlace` ("P"/"A"). The explanation sentences lost their
  "(A)"/"(P)". The holders had spread a fixed-width color button's free space (QPushButton keeps its GrowFlag after
  `setFixedWidth`), so the Mind Map page's color buttons stood at the far right.
- **`theme::Role::Invisibles`** ("invisibles", `Area::Editor`; default `mix(paper, editorText, 0.45)`, what was drawn
  before): `TextColumnView::applyColors` hands it to each editor (`TextEditor::setInvisiblesColor`; invalid for custom
  colors, i.e. composition mode: the text at 45 % as before). Listed in Appearance > Textual Marks and the editor's
  right-click Appearance menu; `v0402`'s digest leaves it out (with `inspectorFields`).
- **Appearance pages**: `LPageView` is gone; `editorPageSurround` ("Around the pages (only in page view)") is in Main
  Editor with the page-view hint. `LookPageId` must match `lookPages()`' order.
- **The theme list** (`ui/ThemePicker.*`, toolbar special `themePicker`, standard set `redo, <>, themePicker, <>,
  searchMenu`; `AppSettings::load` adds it once to an arranged main toolbar, key `toolbars/themePicker0902`):
  `QButtonGroup` of `themeFilterBright/Dark/Both` (setting `window/themeFilter`, program-only by its group), the combo
  `themeCombo` filled again in `showPopup` and on `theme::Notifier::changed`; a choice runs `themeui::choose(window(),
  id)` (as View > Themes). `theme::brightnessOf(id)`: by the resolved Window color; `followDesktop` = Either (in both
  lists). `ThemePicker::giveWay(bar, action, quickSearch)` (a filter on the main toolbar): when the quick search would go
  into the overflow menu, the list's action is hidden; shown again when the bar is `need` wider (and at every
  `rebuildToolBars`).
- **WindowCare** (a 0.90.1 bug, a rare SIGSEGV of the ui suite in `Filter::eventFilter` at a dialog's hide): the owner and
  focus widget at a dialog's Show are kept as `QPointer`s in the filter (`m_before`), no longer as `QObject*` properties
  of the dialog (`qobject_cast` on a deleted one). Guard every widget pointer that outlives an event this way.
- **Download page** (`tools/download-page.py`, `others()`): under each system's full version the program alone, the
  update and the audio and video add-ons (`media-all-formats.qbpkg`; Windows: names with "windows"); only files the
  release has.

## Memory and the usability review (0.90.1 beta, 2026-11-13)

- **Menus filled in `aboutToShow`** are emptied with `qb::clearMenu` (`ui/MenuClear.h`): `QMenu::clear()` keeps the
  submenus made by `addMenu()` (children of the menu, their entries owned by them); heaptrack found them as about 70 % of
  the memory after the system test's writing day. Use `clearMenu` for every menu built anew when it opens.
- **Where an option is kept** (`ui/OptionStorage.*`, `optstore::mark(control, key)` in `OptionsDialogStorage.cpp`,
  `refreshStorage()` from `updateScopeInfo` and the look scope): a badge ("storageBadge", green P / blue A, property
  `qbPlace`) before each control (moved into a "storageHolder"), the tool tip's last paragraph says fixed or depending.
  Keys: the settings key ("group/key"), "group/*" for a group (zoomDefault, tools/<id>), "look" for the look. A **new
  option needs a mark** there: `qb_v0901_tests` fails on any input of File > Options without one (list of non-options
  in `notAnOption`). Tool tips set later go through `optstore::setToolTip` (keeps the place sentence).
- **`ui/WindowCare.*`** (one application event filter, `windowcare::install()` from the MainWindow constructor): dialogs
  fitted to `usableArea` at Show (`fitInto`, title bar room 34 px), the keyboard given back to the owner's former focus
  widget after a dialog hides (at 0 ms and again at 250 ms: without a window manager X takes the focus away later),
  placeholder colors at 4.5:1 (`readablePlaceholder`), `addCloseButton` for Qt's print previews.
- **Undo names** (`ui/UndoNames.*`, `MainWindowUndo.cpp`): a "book" (child object of each QTextDocument) maps Qt's raw
  undo count (`availableUndoSteps` counts single changes, an edit block several; Qt 6.12's `availableRedoSteps` is one
  short) to named steps; names come from `nameStepsOf` (every `act()` helper connects it before the command's slot),
  `TextEditor::keyPressEvent` (Typing, Delete, Paste, Cut) or the change itself. Undo/Redo act on the focus widget
  only and are disabled where nothing can be undone; the toolbar buttons "undoButton"/"redoButton" get the lists
  "undoList"/"redoList" in `placeToolButtons`. Format commands run in one edit block (one step).
- **Numbers**: `language::applyNumbers(groupDigits)` sets `QLocale::setDefault` to the locale as before (German for the
  German program, else `QLocale::system()`) with only the number options changed (`OmitGroupSeparator` when
  `app/groupDigits` is off); `text::plural` uses it. Never set en_US as the default: `units::metric()` ("as in the system
  settings") and `PageSetup::forLocale()` read the default locale's measurement system (inches and US Letter on a German
  desktop: the first release run of 0.90.1 failed in m6 and v0212 for it).
- Also: `MainWindow::showBinderBackgroundMenu` (signal `BinderPanel::backgroundMenuRequested`), `Inspector::focusSynopsis
  / focusNotes` (commands `goSynopsis` Ctrl+Alt+Y, `goNotes` Ctrl+Alt+N), Ctrl+Alt+4..9 for the first six styles of
  `stylelist::all` without a shortcut, `backupNow` Ctrl+Shift+B, `links::fileQuestion` / `breakable`, Compile's
  `fitFormatsColumn` and `window/compileSplit`. New German texts: lupdate on a copy, then the new messages put into
  `translations/quillbinder_de.ts` in its own layout (Qt 6.12's lupdate rewrites the whole file).

## Paragraph buttons of the format bar (0.60.4)

- Specials `spaceBefore`, `spaceAfter`, `indentFirst`, `indentLeft` (`toolbars::specials(Bar::Format)`, standard set after
  `numbers`): `QToolButton`s `para_<id>` with the menus `paraMenu_<id>`, filled in `MainWindow::fillParagraphMenu` (values in pt for
  spacing, `units::` for indents, the value at the cursor ticked, `paraOther` asks with `QInputDialog::getDouble`); they apply a
  `direct::ParaSpec` with one bit through `TextEditor::applyParaSpec` (selected paragraphs, one undo step). Enabled with the
  other format controls in `updateFormatBar`. Icons drawn in code (`icons::kSpaceBefore` ... , `ParagraphEngine` in
  `ItemIcons.cpp`). `AppSettings::load` adds them once to a format bar the writer arranged (key `toolbars/paraButtons0604`).

## Folder totals in binder and footers, Inspector Background (0.60.3)

- **Binder tool tip** (`BinderModel::data`, `Qt::ToolTipRole`): a container shows `totalWordCount()` and, with text of its
  own and children, "(its own text: ...)".
- **Footers**: `folderCount`, `folderCountCorkboard`, `folderCountOutliner` are `QToolButton`s; a click runs
  `EditorPane::showCountDetails(button, folderCountDetails())` (texts counted again with `text::count`; the document count
  uses the same popup, `countLines`). Corkboard and outliner footers use `centeredFooter`: two side widgets with stretch 1
  and the total between them, so it stays in the middle.
- **Colors**: `theme::Role::InspectorFields` (key `inspectorFields`, label "Inspector Background", `Area::General`, defaults
  to `base`; `NotesPaper` defaults to it). `Inspector::applyTheme` puts it into the inspector's Base / AlternateBase with
  `theme::readableOn` as Text; it does not count as a change of the window palette in `theme::apply()`. `base` is no
  longer offered (General Interface lists `inspectorFields` instead); `AppSettings::load` moves an own `base` to
  `inspectorFields`. `InspectorBackground` (the strips) is labelled "Inspector Strips". `theme::panelFields(w)` gives a
  widget Base = Window and Text = WindowText (only those roles; refreshed on `Notifier::changed`): the three main toolbars
  and the option dialog's `optionPages`, sub-page lists and `appearanceAreas`. The ruler draws the column as a frame on
  Window. `lookmenu::fill` adds `lookColor_inspectorFields` for `Part::Inspector` (writes `AppSettings::ownColors`).
  `tests/v0402_tests.cpp` leaves `inspectorFields` out of the theme digest; `tests/v0603_tests.cpp` checks it equals `base`.
- **Double-click beside the text**: `TextColumnView::besideTextDoubleClicked` -> `EditorPane::revealRequested` -> the action
  `reveal` (Reveal in Binder) for that pane; `BinderPanel::flash` blinks the row twice (widget `binderFlash` over the row,
  `flashStep`, 220 ms). A pane has a second, hidden `TextColumnView` (the copyholder): tests pick the visible one.
- **Font names that are not families** (`fontfaces::resolve / substituteUnknown`, `src/text/FontFaces.cpp`): face and
  PostScript names ("SitkaTextItalic", "Georgia-BoldItalic") are matched to an installed family (style words and "MT"/"PS"
  taken off the end, spaces and case ignored, then shorter CamelCase stems) and given to Qt as a font substitution
  (`QFont::insertSubstitution`), so the text keeps the name. Called for RTF font tables (the RTF reader also takes italic
  and bold from the name) and for `font-family` in Markdown spans; results are cached until the fonts change.

## Paste with formatting, invisibles, folder totals, styles over paragraphs (0.60.1)

- **Edit > Paste with Formatting** (`pasteKeep`, Ctrl+Alt+Shift+V): `TextEditor::pasteKeepingFormatting()` sets
  `m_pasteKeepAll` around `paste()`; `insertFromMimeData` then reads RTF with `keepFormatting` and HTML with fonts, and
  `insertSemantic(..., keepParagraphs)` gives each pasted paragraph `direct::withParaSpec(effectiveParaSpec(source))` (the
  first one only when it lands in an empty paragraph). Plain Paste is unchanged. 0.60.2: `TextEditor::contextMenuEvent` puts
  `ctx-pastePlain` (`pasteMatchingStyle()`, also behind Edit > Paste and Match Style) and `ctx-pasteKeep` right after Qt's
  `edit-paste` action, with the shortcuts of `commands::shortcutOf`.
- **Show Invisibles** (`showInvisibles`, `AppSettings::showInvisibles`, toolbar standard set, View button menu):
  `TextEditor::paintInvisibles` draws over the text from the block layouts (not in notes editors); the document's
  text option is never touched, so prints, compile and mirrors stay clean. `blockBoundingRect(b).topLeft()` is the
  layout origin (Qt moves the rect there). The toolbar icon is drawn in code (`icons::kShowInvisibles`, `PilcrowEngine`
  in `ItemIcons.cpp`): neither Breeze nor the bundled set has a pilcrow.
- **Folder totals**: `BinderItem::totalWordCount / totalCharCount`; `EditorPane::folderCountText()` (labels
  `folderCount`, `folderCountCorkboard`, `folderCountOutliner`; refreshed in `updateCounts` and `refresh`), the
  inspector's `inspectorFolderCount` row (`Inspector::refreshTotal`, on `countsChanged` and settings changes). Counts
  follow edits through `DocumentStore`'s 600 ms count timer, not through saving.
- **Styles over paragraphs**: `display::applyParagraphStyle(..., StyleOver::Paragraphs)` (used by
  `TextEditor::setParagraphStyle`, i.e. every style command) clears the char spec set by hand (family, size, weight
  other than bold, color, letter spacing, capitals) before the style; bold, italic, underline, strikethrough,
  highlights and character styles stay; a selection ending at a paragraph's start leaves it out. Paste Formatting and
  script elements keep `StyleOver::KeepOwn`.

## Titles, format bar, style list, alignment (0.40.3)

- **Titles from the first line** (`src/ui/MainWindowTitles.cpp`): a text item titled "" or "Untitled" (`text::isUntitled`)
  gets `text::firstLineTitle` (first line with words, no inline notes or pictures, at most 60 characters) through
  `DocumentStore::textEdited` and a 400 ms timer; `BinderItem::autoTitle` (attribute `autoTitle` in the project file) keeps
  it following; `BinderModel::setData` (every rename by hand) clears it. `queueUntitledDocuments` at `adoptProject` names
  older "Untitled" documents that have text (by the counts kept in the project file).
- **Format bar = what is seen**: `TextEditor::barState()` (toggles, super/subscript, font, size, alignment, line spacing over
  the whole selection; empty when mixed) and `paragraphStylesMixed()`; `toggleState(true)` (by hand) is no longer what the
  bar shows. Not bold / not italic inside a bold / italic style: `props::NotBold` / `NotItalic` (set by `setBold(false)`,
  honored by `styles::applyChars`), in Markdown a span `font-weight:normal` / `font-style:normal`.
- **Alignment of the main text**: `Formatting::align` (was `bool justify`; project file and settings files write `align` and
  still `justify` for older versions; `AppSettings::newAlign`, key `newAlign` plus `newJustify`); `ui/AlignButtons.*`.
  Compile's exporter interface still has `justify` only: centered or right body text comes through the paragraphs.
- **Style list** (`src/ui/StyleList.*`): `Project::styleListOrder / styleListHidden` (element `StyleList` in the project file,
  `list` / `hidden` in the styles section of settings files), `stylelist::addTo` (plain entries up to ten, else a scrolling
  `QListWidget` in a `QWidgetAction`), `StyleCombo` (the format bar's combo box with this popup; its items hold all styles so
  it can name a hidden one), `StyleListDialog`. `MainWindow::fillStyleList` builds the popup and the right-click Style menu.
- **Styles panel**: dialogs started from it get `styleUiParent()` (the panel) as parent; No Style's Redefine From Selection is
  `MainWindow::redefineNoStyleFromSelection` (project formatting + `formatchange::followProject`, own formatting too).
- **Spelling menu**: on a misspelled word `TextEditor::contextMenuEvent` puts everything else into the submenu `spellingMore`.

## The look per project, settings files, style changes, no jump (0.40.2)

- **Look per project** (`src/ui/ProjectLook.*`, `look::`): theme, own colors and fonts of the areas (everything but
  `theme::Area::General` and the interface font), text column width and the border beside the text are stored in
  `settings/look.json` of the project. `MainWindow::adoptProject(p, isNew)` calls `look::settle` (a project without the
  file gets the look for new projects = `AppSettings`; an existing one without a border) and `look::activate`, which
  hands it to `theme::setAreaLook` before `theme::apply()`. `AppSettings::theme / ownColors / ownFonts / editorWidth /
  textBorder` are the look for **new projects** (and the program's menus and panels). Never change a built-in theme's
  colors in an update: `tests/v0402_tests.cpp` keeps a digest of them; a new look goes into a new theme or into the
  look for new projects. File > Options > Appearance has `optLookScope` (this project / new projects); View > Themes
  (`themeui::choose`) sets both.
- **Right-click "Appearance"** (`src/ui/LookMenu.*`, `lookmenu::add / popup`, parts Editor, HeaderBar, Binder,
  Corkboard, Outliner, Inspector, Composition): changes go through `look::change` (the open project's look).
- **Settings files** (`src/core/SettingsFile.*`: sections, capture by names, compare, `applyData`;
  `src/ui/SettingsTransfer.*`: the per-section window; `src/ui/MainWindowSettings.cpp`: Project > Save / Load Project
  Settings). What documents use is never dropped; the settings before loading are kept in
  `settings/settings-before-loading-<date>.qbsettings`.
- **Style changes** reach all text of the style: `direct::takeStyle` (members and paragraph bits that changed, also
  where set by hand), called from `MainWindow::commitStyles` through `formatchange::takeStyle` (snapshot first).
- **No jump on click / right-click**: `TextEditor::cursorMovedByMouse()` makes `TextColumnView::ensureCursorVisible`
  scroll only when the cursor is out of sight; non-left presses are accepted in the editor so they do not reach the
  paper's click handler; `TextColumnView::clickBesideText` puts the cursor on the line beside a click.

## Fonts with several designs, the format buttons (0.40.1)

- `src/text/FontFaces.*`: `fontfaces::install()` (called by `main.cpp` right after the application object; again
  when Qt reads its fonts anew) gives each design of a family that holds several (optical sizes such as Windows 11's
  Sitka, widths) a family of its own in Qt's font list ("Sitka Text" with only the Text faces), so that Bold and
  Italic stay within the design. It works on Qt's private `QFontDatabasePrivate` (families, foundries, styles;
  handles are `FontFile` = file + fontconfig index) and registers faces with `QPlatformFontDatabase::registerFont`.
  The faces of a variable font are read from the file's `fvar`/`name`/`OS/2` tables (read only), because Qt 6.4 files
  the font as a whole under a named face, or folds the faces of one weight into it, depending on fontconfig's order.
  The font files are never changed or copied (Windows fonts may not be modified or converted). `QB_KEEP_FONT_FAMILIES=1`
  switches it off. A new Qt version needs a look at this file (0.80.0, Qt 6.12: `registerFont` takes `colorFont`,
  taken from the family; the separation works as on 6.4, `qb_v0401_tests` passes).
- The Bold / Italic / Underline / Strikethrough buttons and commands work on the whole selection:
  `toggleStateOf` / `TextEditor::toggleState(byHand)` and `setBold` ... in `src/ui/TextEditor.*`; also
  `ObjectTextEdit::isOn` and the fonts window. `TextColumnView` refreshes the format bar on selection changes
  (only for a selection there is); `MainWindow`'s destructor disconnects its panes first.
- `tests/v0401_tests.cpp` uses the free Source Serif 4 variable font in `tests/fonts` (OFL, unchanged; through a
  fontconfig file of the test's own), which is built like Sitka.

## Parts, places, mirrored text (0.21.1)

- Zoom parts: `zoom::kInsp*` and `zoom::kObjectsSheet` in `src/ui/Zoom.*`; a part is a marked widget inside an area
  (`zoom::mark` + `attachFonts`); a root always gets its font set, so it does not take the zoomed font of an area around it;
  widgets made later in a zoomed area are scaled when Qt polishes them (`Filter::adopt`). `QB_ZOOM_DEBUG=1` traces scaling.
- `src/ui/DocMirror.*` keeps a copy of a document in step both ways (the inspector's bookmark preview); it copies changed
  spans piece by piece (not `insertFragment`, which adds paragraphs around lists), ignores Qt's "layout created"
  notifications, and rebuilds the copy only as a last resort.
- `src/ui/ViewMemory.*`: places kept in `Project::uiState` (settings/ui.json): text places per document or combined-text group
  (`TextColumnView::rememberPlace / restorePlace`), board views per board and kind (`MindMapView::setViewMemory`), picture
  and PDF zoom and page (`MediaView`), the inspector (`Inspector::saveState / restoreState`). Nothing here marks the project
  dirty; `MainWindow::saveUiState` writes them at close.
- Mind map backgrounds: `Settings::ownLight / ownDark` (file element `QuillbinderBackground`), `MindMapPrefs::boardBackground`
  / `boardBackgroundDark`, `MindMapDoc::applyBackground` works out the color shown; `BackgroundColor` stays for older readers.
- The format bar's target: `MainWindow::editor()` (TextEditor) and `objectTextTarget()` (`ObjectTextEdit`), both following
  `m_lastTextField` while the keyboard is on the format bar or in a popup.

## Options, zoom, palette (0.21)

File > Options (`src/ui/OptionsDialog.*`) builds every page from `AppSettings` (and `MindMapPrefs`, the zoom
settings) and writes everything back in `apply()`; a new option needs a field in `AppSettings` (load, save,
`resetToDefaults` covers its group), a widget with an object name `opt...`, a line in `apply()`, the code that
acts on it, and a check in `tests/v021_tests.cpp`. Settings the core libraries need are handed over in
`AppSettings::pushToModules()`. Ctrl + wheel: `src/ui/Zoom.*` (mark an area, or give it a handler). Colors offered
to the writer come from `src/ui/ColorPalette.*`. Corrections while typing: `src/text/AutoCorrect.*`.

## Objects (0.20)

The Objects sheet keeps its data in `objects/` of the project folder (one JSON file per object,
`objects/schema.json` for categories, templates, colors and folders), never in the `.qbx`, so older
versions keep working with the project; see `docs/developer-guide.md` section 11. Core: `src/core/Objects.*`
(`Project::objects()`), UI: `src/ui/ObjectsUi.*` (inspector tab `Inspector::Objects`, `ObjectsWindow`,
`ObjectTemplatesDialog`). Object links are `qbobj://<id>` (document links `qb://<id>`).
