# Changes

What each version of Quillbinder brought, in short. The user manual (`docs/manual.md`, Help > User
Manual in the program) describes every function.

## 0.99.0 beta

New functions and fixes (Linux first).

- **One rule for the memory of every AI function** (Speech Transcription, Translation Helper, Read Aloud and Handwriting; tool
  interface 1.3). The program keeps the list of the loaded AI models. A model unused for 10 minutes is unloaded (the time is
  set in **File > Options > General > AI Models**). Before a model loads, when more than half of the memory it goes into (the
  computer's RAM, or a graphics card's memory as its driver reports it) is in use, the model of Quillbinder unused longest
  goes first; when the new model would still bring that memory over 80 %, it is not loaded, the message line under the inspector says why, and
  the function can offer the next smaller model or the processor. The system's own low-memory signals unload idle models at
  once (Linux: a pressure trigger on `/proc/pressure/memory` when it can be opened; Windows: the low-memory resource
  notification; Android: the memory figures before each load). The AI's threads run at a low priority.
- **Message line "Model change in progress"**: a slim line at the bottom of the inspector (no room while empty; the line under
  the binder when the inspector is hidden), from the start of an unload or a load until the new model is ready, also at a
  first load. The reason a model was not loaded (the 80 % rule) is shown there too. The binder's line keeps its other messages.
- **File > Options > General > AI Models**: the CPU cores for the AI (one setting for every AI function; default three
  quarters of the cores rounded down, with 6 cores or fewer the cores minus 1, at least 1), "Unload a model after 10 minutes
  unused", and a line with the memory in use and the loaded models. Kept per computer, not in a project. The add-ons' own
  fields for threads and unloading are replaced by this page in a later version.
- **Tool interface 1.3** (for add-ons): `aiThreads`, `modelMessage`, `keepRunning` (for Android's foreground service; nothing on
  Linux and Windows), `lowerThreadPriority`, the model registry (`requestModelLoad`, `modelLoaded`, `modelUsed`,
  `modelUnloaded`, `reportDeviceMemory`) and a third interface `ToolModels` through which the program asks a tool to unload a
  model. Tools of 1.0 to 1.2 keep working unchanged.

- **Third-party parts for the built-in encryption and the AI add-ons are prepared** (no function of the program yet): three new
  ready-made packs of libraries for the Linux build (Botan, RNP, json-c, bzip2 and zlib; ONNX Runtime; ggml, whisper.cpp,
  llama.cpp, qwentts.cpp and Sonic), the daily security watch follows each new part, and `THIRD-PARTY.md` and Help > Third-Party
  Licenses (at the next AppImage) name the parts that go into the program.

## 0.95.0 beta (2026-11-26)

New functions and fixes (Linux first; the AppImage stays on the runtime `u2404-qt6.12-r2`).

- **Quillbinder speaks eleven languages**. Besides English and German: Spanish, French, Portuguese (Brazil), Italian,
  Russian, Indonesian, Turkish, Polish and Dutch. Each is shown by its own name in the language list; the nine new ones are
  machine translations and are marked so ("(machine translation)", in the language of the program) until a speaker has
  checked them. The list is on the first-start page and in **File > Options > General > Startup**; a new language is used
  from the next start. At a true first start the program takes the language of the system when it is one of the eleven
  (else English); an installation that is updated keeps what it had. Numbers, dates and units follow the language.
- **The manual, the quick start guide and the welcome project follow the language** (English where a language has none).
  F1 and the links between sections find the right place in a translated manual by its place in the list of headings, so a
  translated manual has the same headings as the English one. The note in the options that they stay in English is gone.
- **Spelling dictionaries for the nine new languages as add-ons**, one per language (Spanish es_ES, French fr_FR, Portuguese
  pt_BR, Italian it_IT, Russian ru_RU, Indonesian id_ID, Turkish tr_TR, Polish pl_PL, Dutch nl_NL), unchanged from
  LibreOffice's dictionaries (tag libreoffice-25.8.7.3), each with its license and source; the download page lists them. The
  spell checker now reads the Polish dictionary's encoding (ISO 8859-2).
- **Speech Transcription**: the main and the fallback language are chosen from the eleven languages (a language stored
  earlier stays selectable).
- **Keyboard sets for more layouts** (0.94.0): French, Belgian, Spanish, Italian, Portuguese, Turkish and Dutch keyboards
  get the keys without AltGr at the first start, as German keyboards do; the Polish programmer's layout and the others that
  use AltGr for letters also move four commands that sat on Ctrl+Alt+letter (copy and paste formatting, go to notes, show
  ruler). Russian and Indonesian keep the usual keys.
- **Plural forms** follow the language: Russian and Polish have a third form for 2 to 4, French and Portuguese call 0
  singular, Indonesian and Turkish have no plural after a number.
- **The inspector's menu entries** (Navigate > Inspect) get their Alt letters from the translated names.
- **The object lists' values** (role, kind, season, precipitation) are marked for translation like every other text, and the
  welcome project of each language stores the values the program's list shows in that language (`tools/align-welcome-choices.py`).

- **Themes and profiles.** A theme is only the look; a profile keeps everything else: the language, all options, the
  toolbars and their places, the window's size and place, the panels, the keyboard set, the backups and the backup
  folder, the checks for updates, the standard colors and the saved layouts, and it names its theme. **View > Profiles**
  switches at once (the language needs a restart; the program says so). **Manage Profiles** saves the settings as a
  new profile, renames, copies, deletes, exports and imports profiles (a file that names folders or programs asks
  first). A program that has been used before takes its settings as the profile **Default** at the update; a new
  installation starts with the standard settings, in the light or dark theme as the system is set. A profile made on a
  large screen fits a smaller screen or a tablet. The old **Save Options to File** and **Load Options from File** are
  replaced by exporting and importing a profile (an old options file can be imported as a profile). A **Profiles**
  toolbar (off at first) shows the profile in use. The welcome project has a page about themes and profiles.
- **Shelin's Dark Theme** is a new built-in theme, at the top of the theme list. It can be deleted and then stays
  away after updates.
- **Shelin's** is a built-in profile, at the top of the profile list. It uses the new theme. It can be deleted and
  then stays away after updates.
- **Profiles, fixes found in a review.** Switching profiles while a project with its own settings is open no longer
  mixes the project's values into the profile that is left or writes the profile's values into the project (the export
  is refused while Options edit This Project). Renaming a profile by case only ("work" to "Work") keeps it. The marks
  of things done once (the first-start page, the placed tool buttons) are no profile settings, so a switch does not
  bring the first-start page back. Every non-empty value of a program, folder, file or address setting in an imported
  profile is asked about, and so are the places of a profile file that another computer wrote (a file copied into
  the profiles folder) at the switch. A switch to a profile that keeps fewer backups asks first; a failed write of
  the settings is told; layouts that cannot all be set leave the old ones; an own theme's name with a backslash or two
  dots is refused.
- **A Labels toolbar** (off at first, **View > Toolbars > Labels**): the label colors of the open project side by
  side, a click sets the label of the selected documents, and a button takes the label away.
- **Standard Colors** in **File > Options > Appearance**: the labels new projects start with (existing projects keep
  theirs), the six colors of the text color and highlight palettes, and the colors the two buttons start with. They
  belong to the profile. **Defaults** gives the program's standard back.
- **File > Options > Keyboard: Use the Set of My Keyboard Layout.** The keyboard set of 0.94.0 was chosen only
  at the first start, so a writer who had updated kept keys that need AltGr. The new button applies the set that fits
  the layout in use now, after listing what changes; shortcuts you set yourself are kept.

- **The Bookmarks tab keeps the sizes of its three parts**. The heights of the two lists and of the text below them
  were lost after a restart or a program update when no bookmark was chosen at that moment (the text part, being
  hidden, was written as size 0) or when the inspector was hidden. The sizes you gave are kept as they were and come
  back, also for the Synopsis and Snapshots tabs.

- **Each part of the Bookmarks tab zooms on its own**. Ctrl + mouse wheel over Document Bookmarks, over Project Bookmarks
  and over the bookmarked text below them changes only that part (before, the two lists zoomed together). With **View >
  Zoom Inspector Tabs Separately** on or off it is the same. The zoom you had for both lists is taken for both at first;
  **File > Options > Editing** lists the three parts, and the zoom list in the inspector names the one you zoomed last.

- **Cutting targets**. The target fields for the Draft, the session and a document take negative numbers (there is no
  option to switch on). A negative target is a cutting target ("cut 2,000 words today", "bring this chapter down to
  4,000 words"): its bar fills as the count falls, and a negative number is shown in red. With a negative session target
  today's count is net and goes below 0, in red, where more words were cut than written; with a positive one it stays as
  it was. In a project in Scrivener format the .scriv holds "no target" for a negative one (Scrivener has no such target)
  and the working copy keeps the number, also when the .scriv is read again.

- **Highlights are drawn exactly as picked**. A highlight was drawn about one third transparent, so a light green over a
  dark field or on a yellowish page showed as another color. It is drawn fully now. Where the text on it would fall below
  a contrast of 4.5 : 1, the text is shown dark or light on the screen only; the stored colors, the saved file and the
  compiled file keep what you picked.

- **The first-start page says plainly what moving the AppImage does**. The checkbox reads "Move the AppImage to
  ~/Applications, a fixed place the menu entry starts it from" (the bracket that sounded as if the move broke the entry is
  gone), and the grey line adds that an AppImage you move yourself has to be added to the menu again. Add to Application
  Menu always writes the file's current place and replaces an older entry.

- **25 backups are kept at first**. A new installation keeps the newest 25 backups of each project (it was 5); a number
  already saved stays. On the first-start page and in **File > Options > Backup** the field holds only the number and the
  words stand around it: "Keep the newest [25] backups of each project".

- **Translation Helper 1.5: eleven languages**. The add-on Translation and Proofreading is now called **Translation
  Helper** (Edit > Writing Tools, the Add-on Manager, the notice of a missing add-on, File > Options > Translation
  Helper) and says in its window, its description and the manual: "This tool gives suggestions only. Its translations
  are not precise: check each one, which needs a working knowledge of both languages." It translates between any two
  of English, German, Spanish, French, Portuguese (Brazil), Italian, Russian, Indonesian, Turkish, Polish and Dutch,
  chosen in two drop-down lists. Each language has two small OPUS-MT models (to and from English, Apache-2.0 or CC BY 4.0,
  pinned by revision and checksum, in the new release *models-4*); a pair without English goes through English, and a
  note under the result says so. A dictionary per language from the Wiktionary (CC BY-SA 4.0, a separate download
  with its license shown first) gives the word classes and meanings of a single word; a language without a usable
  extract gets the model's translations only. A single word keeps its correction, shown as a suggestion. The
  add-on's own texts come in the eleven languages of the program. The 1.4 settings keep working (German to English is
  the first choice, the German dictionary and downloaded models stay). The options page showed "%5</a>%6" for the
  download line of a dictionary (a model without a memory need); fixed.
- **Program**: the command in Edit > Writing Tools and the notice for an add-on that is not installed carry the new
  name (German: Übersetzungshelfer).

For developers: *models/convert.py* pins the revisions of the models of *models-4* and builds its catalog
(`--placeholders`, `--assemble`); the workflow *translator-data-4.yml* builds the release (models and dictionaries) on
GitHub's runner and writes the catalog; *tools/make-dictionary.py* builds the dictionary of any of the nine languages.

## 0.94.0 beta (2026-11-25)

New functions and fixes (a program update for 0.93.x; the AppImage stays on the runtime `u2404-qt6.12-r2`).

- **A Quick Reference window no longer crashes when it is closed while its menu is open**. The menu of the document
  icon in the window's header (Reveal in Binder, Open in the Editor, Project Bookmarks) kept the window from being
  deleted safely: a close request from the window manager while the menu was open ended the program with a "double
  free" error. The close request is now held until the menu has ended and then carried out. The same holds for the
  menus of the mind map windows, the Objects window and the media player of a panel.
- **A new mind map note gets the keyboard**. After **New Note** in the inspector's mind map (button, menu or a
  double-click) the note showed its text selected, but the keys went to the part that had the focus before, often the
  binder. The note's editor now takes the focus.
- **Documents moved to the Trash leave the corkboard at once**. Three documents chosen in the binder and moved to the
  Trash with **Delete** stayed on the corkboard ("3 documents") until another item was chosen. A corkboard or outliner
  that shows several chosen documents lets those go that were moved to the Trash.
- **The inspector shows the document chosen in the binder**. When the editor already showed the chosen document while
  the inspector showed another one, a click in the binder left the inspector where it was (found by a check that
  failed on Windows only). After a choice in the binder the inspector always shows the chosen document.
- **Imported text and pastes are no session words**. After **Import and Split** of a long text the Project Targets
  counted all imported words as written in the session; the first opening of a project counted its words too; and
  after **Reset Session Now** the tooltip of the quick-search field kept the old number until the next change. One
  rule now: imported text, the words a project holds when it is opened the first time, and a paste of more than 200
  words do not count as session words; the tooltip follows **Reset Session Now** at once.
- **Back Up Now tells its success under the binder**. The box "Backup saved" that waited for OK (and on Android blocked
  later steps when it was left open) is gone. The message line under the binder says "Backup saved." with a link **Show Folder**
  (the path is in the tooltip; a message with a link stays 10 seconds); boxes come only for failures and for a backup that is incomplete.
- **Small layout points**. The Project Targets window keeps the room of its progress bars from the start, so its
  fields no longer move under the mouse when a target is set. On a screen narrower than 1600 px the window opens
  maximized the first time (when no saved state exists). The window's size, place and state are saved a moment after
  each change, not only when the program ends (they were lost after a crash).
- **Translation and Proofreading 1.4**. A single word, German to English or English to German, shows the
  translation of each word class (a German word that is a verb in lower case and a noun with a capital gets both),
  then similar translations (from English to German: similar German words from the German dictionary), then the short
  description, instead of repeating the word. **Automatic** translates a lone German word instead of proofreading it
  (with the German dictionary installed). German typed in lower case translates better: the dictionary puts the
  capitals of nouns right in the copy the model gets (the writer's text stays as it is); without the dictionary a
  note says that German nouns need capitals. A notice at the top of the add-on's pane names what is missing (a
  translation model, the German dictionary) with a button to the add-on's options. Models are unloaded after 15
  minutes without use (File > Options > Proofreading > Speed). Fixes: no alternatives that differ only in capitals,
  no "Spelling" line for words of the result, "number" is no longer read as "numb", and the hints in the empty fields
  wrap.
- **One place per topic in the menus, and Find Command**. "Layout" stood in three menus; fonts under Navigate; the
  add-on commands in File, Edit and Help; the link commands in Edit while images, tables and notes were in Insert.
  Now **View > Layout** holds the positions of the binder, editor and inspector, the saved layouts, **Save Current
  Layout...** and **Manage Layouts...** (Window > Layouts is gone); **View > Editor Layout** is **View > Split
  Editor**; the font commands moved from Navigate > Fonts to **Format > Font**; **Link to Document...**, **Link to
  Object...** and **Insert Web Link...** moved from Edit to **Insert**; **Help > Add-ons and Updates** holds the
  Add-on Manager, Install Update or Add-on (out of File), Check for Updates Now and Check for Updates at Startup
  (Edit > Writing Tools holds the tools only). **Help > Find Command...** opens a window with a search field and the
  list of all menu commands with their menu and shortcut: typing narrows the list, Return or a double-click runs the
  chosen command, and a command that is off is shown grayed and does not run. It has no shortcut (Ctrl+Shift+P is the
  mind map's Page Setup). The commands keep their internal names, so saved shortcuts and toolbars are not affected.
- **Help has 13 entries instead of 18**. **Open Plugin Folder**, **Developer Guide**, **Crash Reports**, **This
  Computer** and **System Information** are in the submenu **Help > Troubleshooting and Development**.
- **The first start is one page**. The window with the backup settings, the question about the application menu (Linux
  AppImage), its confirmation box and the note about the check for updates are one page with one **OK**: the backups,
  the application menu (with a line that **Help > Add to Application Menu** or **Remove from Application Menu**
  changes it later) and the tick box for the check for updates. No box confirms the menu entry (a failure is still
  told), and the welcome project opens without an empty main window showing first. On Linux, **Help > Add to
  Application Menu** turns into **Remove from Application Menu** while the entry exists and takes away what adding made
  (the entry file, the icon and the file types).
- **The AppImage can move to ~/Applications with the menu entry** (Linux). Adding the entry, on the first-start page and
  with the Help command, offers to move the AppImage to *~/Applications* (a check box, ticked at first when it is not
  there yet), so that the entry points to a place where the file stays; before, an AppImage in Downloads that was moved
  or deleted could not be started from the menu. After the move, program updates, the *.before-update* copy and System
  Information use the new place at once. A move that fails (another file of that name, no room) loses nothing: the
  file stays, the entry points there, and a message says why.
- **The full AppImage shows its first start**. It unpacked its add-ons (13, about 159 MB) without any sign for 10 s or
  more. Now a window with the program's logo, a progress bar and "Installing 13 add-ons (159 MB)..." shows during the
  installation (one call of the program for the folder of packages, `quillbinder --install-folder <folder>`, instead
  of one call per package; the marker file and its rules stay: once per version, a removed add-on stays removed, no
  marker when a package fails). Without a display it runs without the window. The download page can name the space of
  the add-ons beside the full version's button when the release gives the number (a file `full-addons-size`).

- **One name and one key per command**. Ctrl+Shift+Return, **Insert > Media Timestamp** and **Navigate > Media >
  Insert Timestamp** were one command under two names; it is now **Insert Media Timestamp** in both places (the entry in
  Insert only shows the key of the one command), and the Alt letter of the Insert menu's entries (Alt+T was given to
  Table and to the timestamp) is its own. **Navigate > Inspect** listed 7 of the inspector's 9 tabs and called the first
  one "Notes" while its tooltip says "Synopsis and notes": it now lists every tab, also those of the writing tools
  (Speech Transcription, Translation and Proofreading), under the name of the tab's tooltip. A new check walks all
  menus: no two commands share a shortcut and no menu has two entries with the same Alt letter.
- **Return and Space in the binder: a menu option, off at first**. Return made a new empty document and Space opened a
  Quick Reference window (the quick-add keys); a file manager opens the item with Return, and a phrase with a space
  typed to find a title opened a window for each space. The new entry **View > Quick-Add Keys in the Binder** is
  unchecked (also after an update): Return opens the selected document in the editor and puts the keyboard there,
  Space belongs to the binder's type-ahead search (this also holds for the list of a collection or a search). Checked,
  the keys are as before.
- **Shortcuts follow the keyboard layout**. The menus showed Ctrl+{, Ctrl+|, Ctrl+} (align), Ctrl+[ and Ctrl+] (back,
  forward), Ctrl+\\ (list of script elements) and Ctrl+= (subscript); on a German keyboard these need AltGr, and with
  the German layout on X11 Ctrl+AltGr+7, < and 0 triggered the alignment as chords the labels did not show. At the
  very first start (no shortcut settings, no earlier start) the program asks the system for the keyboard layout and keeps
  the answer: a German or Austrian layout gets a set without AltGr (Alt+Shift+L, E, R to align, Alt+Shift+B and F for
  back and forward, Ctrl+Shift+O for the list of script elements, Ctrl+Shift+D for subscript; in the mind map also
  Alt+Shift+J, Ctrl+Shift+PageUp and PageDown), every other layout the set as before. A program that was used before,
  and every shortcut changed in **File > Options > Keyboard**, stays as it is. Linux asks the keyboard settings of the
  desktop (KDE, GNOME), the X server or the system's default; Windows the language of the keyboard layout; Android the
  language of the keyboard (this last way is not tried on a device).
- **Composition mode: the bar is readable in light themes**. After the theme "Light" the buttons Inspector, Exit (Esc),
  the one beside them and the zoom box of the bottom bar were light gray on light gray (1.2:1). The bar is dark in
  every theme now and its buttons, box and texts have their own colors; a check measures the contrast of the bar for
  every built-in theme (at least 4.5:1, as WCAG 2.2 asks for text).
- **Card text and ruler numbers are readable**. The text of an index card that is taken from the document (a card
  without a synopsis) was 3.1:1 on the card and the numbers of the ruler 2.5:1 in the default theme. Both have at
  least 4.5:1 in every built-in theme now (the text of a synopsis too); a check measures them.
- **One theme button instead of four buttons and a list**. Bright, Dark, U1, U2 and the theme list took 333 px of the
  toolbar. Now one button, **Theme**, has a menu: **Light**, **Dark**, **My Look 1**, **My Look 2**, then every theme
  (the one in use checked) and at the end **Save the Current Look as My Look 1** and **...as My Look 2**. Light shows
  the theme Light (it showed the theme that follows the desktop for some), an empty My Look says so in the menu and
  explains itself in its tool tip. U1 and U2 are called My Look 1 and My Look 2 in the menus, tool tips and the manual
  (the saved themes keep their file names). The button is in the Themes toolbar, which is off at first, also for those
  who had it shown (once; **View > Toolbars > Themes** shows it again). The theme choices moved from the Window menu to **View > Themes** (the same commands; Window has no Themes entry).
- **The manual has a contents list and a search**. F1 opened one long text without a contents list, a search or jumps.
  The window now has the headings of the manual on the left (a click jumps to one) and a search field (**Ctrl+F**;
  **Return** and **Shift+Return**, or the buttons Next and Previous, show the next and the previous hit; the line
  beside the field says which hit of how many is shown, and all hits are marked). **F1 opens the part that fits**: the
  document, the corkboard, the outliner, Combined Text, the binder, the tab of the inspector with the keyboard, and the
  open dialog (compile, options, project settings, themes, add-ons, styles and the other main dialogs); a dialog
  without a part of its own opens the start. What is meant for people who build the program (the design decisions, the
  environment variables, plugins and packages, the project folder's files) moved from the manual to the developer guide
  (**Help > Troubleshooting and Development > Developer Guide**); the manual's sections after section 19 are renumbered
  (troubleshooting is section 20).
- **The first compile with a title page asks for the author's name**. The title page of the project templates reads
  "by" and the author's name; a compile to Word gave "by" and nothing after it because the name is asked nowhere. The
  first compile that writes a title page now asks (first and last name, kept with the author information in **File >
  Options > General**), once. If the name is left empty, the "by" line is left out, in every format.
- **The add-on install window names what the add-on does**. The window of Speech Transcription began with whisper.cpp,
  Vulkan, Qt Multimedia 6.12.1, a TLS plugin and the runtime name. It now begins with what the add-on does for a writer;
  the technical lines are under **Details**, which is folded (a new optional field "details" of the add-on's
  manifest). The window of the full AppImage that shows the add-ons being installed names each one by its name from the
  manifest ("Media Playback"), not by its id.
- **The download page lists the add-ons and warns about the question of the desktop**. Under each system's other
  downloads the page lists the add-ons of the release (name, one line and size). The Linux steps say that the desktop
  (KDE Plasma) may ask whether to execute the file after the double-click. The release workflow writes the installed
  size of the full version's add-ons (**tools/full-addons-size.py**) for the line about disk space, which the page
  already read.

- **System test (Linux): S-P03 and S-T03 follow the Android copy**. S-P03 saves before its second Save As (since 0.93.0
  Save As writes nothing more into the original); a right click in S-T03 brings the place into sight in a scroll area
  around the editor. The system test runs only when asked.
- **A message line under the binder, and tool interface 1.2**. Short messages of the program and of add-ons appear in a
  small line under the binder: one at a time, each for 5 seconds (10 seconds with a link) or until it is clicked; the
  others wait. The tool interface has a new host call, `showMessage(text, milliseconds)` (add-ons call it only when the program
  says 1.2 or later; older add-ons keep working). Translation and Proofreading 1.4 and Speech Transcription 1.9 free their models
  after 15 minutes without use (the time can be changed in their options; Speech Transcription has a new field "Free the model
  after") and say "Model unloaded, RAM usage minimal" in this line. **Help > Troubleshooting and Development > System
  Information** names the memory the program holds (resident size).
- **Writing progress in the footer**. With a session or draft target set, the footer shows a small bar and "83 / 1,000 today"
  and "12,400 / 80,000 draft" beside the word count; a click opens **Project Targets**. The two thin lines under the quick search
  field stay as an option (**File > Options > Appearance > General Interface**, on at first).
- **Import and Split at headings**. For .docx, .odt and .md files: Heading 1 becomes a folder, Heading 2 a folder or a document
  (chosen in the window), the text between headings becomes documents, lower headings stay in the text. The window shows the
  resulting binder as a tree before anything is imported. The title of a document that no heading names follows one rule,
  stated in the window and the same in every mode: the first line of its text that is not empty (shortened to 120 characters
  when longer), otherwise "<file name> <number>"; before, most documents got the file name whenever the first line was empty
  or long.
- **Documents > Group**. The selected items move into a new folder at the place of the first one; its title is ready for
  typing; one undo step (**Edit > Undo** with the keyboard in the binder; Ctrl+Shift+J).
- **Objects: small points**. Tab from the name of an object goes to Full name; the dialog for "Appears in" takes several
  documents at once (Ctrl or Shift); the "+" buttons beside the link fields are framed buttons of normal size.
- **Characters and places as Objects**. **Documents > Convert > Sheet to Object** (also in the binder's context menu) makes an
  Object of each selected sheet document: the title is the name, the text goes into Other, a picture becomes the picture; the
  folder's name (Characters, Places) chooses the kind. The binder item stays and from then on shows the Object when it is
  selected and opens it with Return. The document keeps its text, in a Scrivener project too; the link is kept in the project's
  own Objects data. One undo step.
- **A notice for a missing add-on**. **Edit > Writing Tools** keeps "Translate and Proofread" and "Speech Transcription" while
  the add-on is not installed (or switched off); they show a notice with a button to **Help > Add-ons and Updates > Add-on
  Manager**. Before, nothing in the program led to them.
- **"This project" keeps only the changed option**. Options set for "This project" could take the toolbar lists, the hidden
  toolbars and the revision colors along with the one changed option (a fault of earlier versions too, shown by a new key
  count; it depended on the size of the settings files). Only what was changed goes into the project now.

## 0.93.1 beta (2026-11-24)

Fixes and small changes (a program update for 0.93.0; the AppImage stays on the runtime `u2404-qt6.12-r2`).

- **Fonts named by their kind** ("monospace", "serif", "sans-serif") work on systems that do not know these names.
  The style **Code Span** and other text in such a font is shown and written into Word, e-book and PDF files in a
  font of that kind wherever the system does not know the names (on Android tablets it was the system's default font, so a text
  message in Code Span reached a Word file without its fixed-width font). On Linux, which knows the names, nothing
  changes.
- **Search and the counts keep less in memory.** The plain texts kept for search, the word counts and the targets are
  held up to about 8 MB; past that a text is read from its file each time it is needed (large projects use less
  memory, mostly on tablets; nothing changes in what is found or counted).
- **Help > System Information names more for reports from tablets.** It also names the area a window can use on
  each screen, the input devices (keyboard, mouse, touch screen, pen) and whether the on-screen keyboard is shown.
- **Help > Third-Party Licenses names Qt PDF with its own source.** Since Qt 6.12 Qt PDF is released with Qt
  WebEngine and has a version of its own (Qt PDF 6.140.0 for Qt 6.12), and Qt's own source release no longer holds
  it: the list names it as a part of its own with its source, `qtwebengine-everywhere-src-6.140.0.tar.xz`, which the
  release "Third-party source code" gets with this version. The parts Qt carries inside itself are listed in the
  versions of Qt 6.12.1 (0.93.0 still showed those of Qt 6.12.0, such as zlib 1.3.2 instead of 1.4 and HarfBuzz
  14.5.0 instead of 15.1.0); the made-up parts of Qt's own tests are left out.

For developers (nothing changes in the program): the official entry of Translation and Proofreading describes 1.3
and names the add-on's folder in this repository (`folder`, used by the three official add-on workflows through
`tools/addon-entry.py`); the add-on's own packaging script names the program's runtime (it named the runtime before
0.93.0); `tools/get-ffmpeg.sh` builds FFmpeg anew when a folder holds another FFmpeg version; the workflows use the
artifact actions that run on Node.js 24; the security watch follows Qt PDF's own releases; the official add-on
workflows try a cut-off download of test data again.

## 0.93.0 beta (2026-11-24)

A new AppImage for Linux, on Qt 6.12.1, whose built-in libraries bring safety fixes: the check for updates offers it
as a recommended security update (a program update cannot bring those libraries, so 0.93.0 needs the new AppImage
once; later program updates fit it). Four new functions (the panes placed by the
window's shape, File > Save As, an overview for the mind map, the translator for single words and both directions),
and many fixes of the safety nets around saving and backups. The add-ons that bring their own libraries are rebuilt
and need 0.93.0 (the check for updates and **File > Install Update or Add-on** tell you).

- **Qt 6.12.1** (before: 6.12.0). The libraries Qt carries inside itself are newer, with safety fixes: zlib 1.4,
  HarfBuzz 15.1.0, PCRE2 10.49, libpng 1.6.59, libjpeg-turbo 3.2.1, FreeType 2.15.0 and md4c 0.6.0. Because the
  AppImage carries them, a program update alone cannot bring them: the check for updates offers the new AppImage
  (the runtime is called `u2404-qt6.12-r2`).
- **The panes placed by the window's shape.** A window taller than wide now shows two rows: the editor in the full
  width, and the binder and the inspector beside each other in the other row. **View > Layout > Upright window**
  chooses whether the editor's row is the upper one (Editor in the Middle) or the lower one (Editor at the Bottom),
  and whether the binder is left or right. A wide window (a square one counts as wide) keeps three columns; **View >
  Layout > Wide window** chooses any of the six orders. The window follows its shape by itself (also when a tablet is
  turned), and each shape comes back as you left it, places and sizes. **Reset Layout** puts the shown shape back to
  its start values. The choices are kept per computer in the program's settings, never in a project. A hidden binder
  or inspector leaves its room to the other one. With the on-screen keyboard (Android) the row of the binder and the
  inspector keeps its height.
- **File > Save As…** (Ctrl+Shift+S) replaces the menu "Project Format" of the File menu. Type the name and choose the place; the
  file type in the dialog (Scrivener 3 Project or Quillbinder Project) decides the format, so the same command changes
  the format and saves a project under another name or in another place. The window then works on the new project, and
  every later save goes there; the original stays on disk as it was, and what you had not saved yet goes into the new
  project only. The new project starts its own series of backups (the backups so far stay with the original), and
  its title is the name you typed. The copies that leave the window on the original are in **File > Export** now:
  Scrivener Project... and (new there) Quillbinder Project.... A toolbar or shortcut you set for the old entries
  (Quillbinder Project, Scrivener 3 Project, Save a Copy as ...) is left out quietly.
- **An overview for the mind map.** A small map in the top left corner of a board (in the inspector's tab and in the large
  window) shows all of the board and a frame for the part you see; click in it to move the view there, drag the frame to
  move the view with it. It shows by itself when some of the board lies outside the view and hides when all of it fits;
  the new button after **Menu** in the board's bar (or the entry Overview Always Shown in the board's View menu) keeps it shown. Drag the grip in its
  corner to change its size. The text **Fixed: ...** of a fixed board now stands in the bar itself, shortened when the
  bar is narrow, and the row under the bar is gone.
- **Translation and Proofreading 1.3: a word on its own, and both directions.** Type one English word: below it the
  panel says what the word means, in English, with the classes it belongs to (the meanings are from the dictionary
  WordNet 3.0, built into the add-on, and the output says so); a word WordNet does not have gets a meaning written by
  the add-on's chat model (Qwen3 4B, or Qwen3 1.7B) with a warning that it may be inaccurate, and without such a model
  the output says what can be downloaded. Type one German word: it is looked up first in a German dictionary (an
  extract of the Wiktionary, a separate download in the add-on's options, shown with its license, CC BY-SA 4.0, before
  it comes), which gives the base form, the word classes, the meanings and the English words of each meaning
  ("fluchen", verb: to curse, to swear); the first five meanings of a class are shown and the rest is behind "more".
  A class the dictionary has no English word for, and a word it does not have, are translated by the small model, with
  a warning. "Similar" follows the English word you chose, and only its word class. The old lines "The model's other
  words here:" (word pane) and "Other translations:" under a single word are gone: they were beginnings of other
  sentences, which with the large model were often names. For a short text of several words, "Other translations:"
  now lists the model's best complete translations, without names it made up.
- **Swap and Translate back; English to German.** Two new buttons: **Swap** turns German to English into English to
  German and back (the result moves into the text and is translated the other way), **Translate back** shows below
  the result what the translation says in the language of your text, until you change the text. English to German is
  new: **OPUS-MT English to German (small)** is downloaded on request (the reverse model is offered when you press
  Translate back without it). One English word in English to German is translated by its word classes, as one
  German word is the other way round. The second row of the controls shows the translation model while you translate
  (switching it changes the translation), and the list of styles ("Correct", "More Formal") is only there while you
  proofread.
- **Single words and up to three words never go to the large translation model**, which puts names first on single
  words: the dictionary first, else the small model (also when the large one is chosen; when the small one is not
  downloaded, a note offers it). The large model's description now says it is for whole sentences. (The models are
  on the new release *models-3*; the workflow `translator-data.yml` makes it.)
- **Notes start as plain text.** The notes of a document and the project notes no longer take the font, size, indent, spacing
  or style of the project's text formatting: new notes and new text in empty notes are plain, in the interface's font; what you
  format by hand stays, and notes that already have text are unchanged. Pasting into the notes gives plain text; **Edit > Paste with
  Formatting** keeps the formatting.
- **The save text moved into the editor's footer bar**. The window's bottom row is gone. "Saved", "Not saved yet",
  the red "Not saved" sign, the "No backups" link and the short messages of the program (a backup's result, "Saved
  again ...", and the like) show in the empty stretch of the footer bar of the first editor (the left or upper one);
  a message takes the place for its seconds, a message too long is shortened (its tooltip reads it all), and in a very
  narrow editor a small colored mark stands there instead (the text is its tooltip). The status bar row is no more, so
  the window gains that row. Toolbars at the bottom edge stay where they are.
- **A new background shape starts medium gray and not magnetic** (before: pale gray and magnetic). The values you set in Mind
  Map Options stay, except a stored pale gray fill and a stored "Magnetic", which are changed once.
- **Less memory with large projects.** The texts of a session no longer stay laid out until the project closes: a
  document that is not shown, has no change that is not saved and no step to undo is let go a moment after you moved
  on, and read again from its file when you show it again (its text, formatting and the place of the cursor are as
  they were). Showing 500 documents of 1,000 words one after another took about 52 MB more before; now about 2 MB more.
  Search, Combined Text, Compile and the word counts still see every document.
- **The emergency copy is whole, or says it is not.** When texts cannot be saved, the copy of what was not saved now
  counts its files: every message and the notice after an ending by a signal say "n of m files" and name each file that
  is missing with the reason, and an incomplete copy is headed as incomplete (a warning sign), never as done. The copy
  is also made when only the binder or the Objects changed, and it holds the project file as it is in memory and the
  Objects not saved; its README says how to put the project file back. A folder that is there is never used again: a
  second copy in the same second gets a number.
  The project file in the copy is named *.copy, so that the folder can no longer be opened as a project by mistake.
- **Saving tells a drive's error for a folder, and an unreadable journal stops the saves.** When the drive reports an
  error while a folder is synced after a save (not a file system that cannot sync: that is no error, as before), the
  save counts as failed, the journal stays, and the next save or opening finishes it. While an unreadable journal of an
  earlier save could not be moved aside, no save writes a new journal over it: the save waits and its message names the
  file. After a drive error for a folder, the retries no longer report conflicts that are none (a project file or a text
  "changed elsewhere" that was the program's own), and a picture is not written twice.
- **Backups.** The note of a backup that an ending by a signal left undone is removed only when the next opening's
  backup was made (in the folder, or waiting beside it); a backup that fails leaves the note for the next opening. An
  older backup that the clean-up cannot delete is told once with the file and the reason, in a notice that takes no keys
  while you write. **Restore From Backup** says
  "restored", "restored incompletely" (headed so, with the files named) or "not restored", and says when the
  restored project's title could not be saved.
- **Backups that wait are listed, and the old backups can be moved.** **Restore From Backup** now also lists the backups
  of the open project that wait in the program's own work folder (they were made while the backup folder could not be
  reached); they are marked "waiting (not yet in the backup folder)" and are restored like the others. The new command
  **File > Back Up > Move Old Backups Into the Backup Folder** moves the backups that versions 0.70 to 0.90 kept in a
  folder of their own for the project into the backup folder, under their names; it asks first, never overwrites a
  backup of the same name (that one stays where it was and is named in the report), tells what it moved and what it left,
  and removes the old folder when it is empty. The command is on only while the project has such a folder with backups.
- **Temporary files that a crash can leave are removed.** The files Qt writes while it replaces a file (*name.json.AbC123*)
  are removed from a project at the opening, and from a Scrivener project when it is read, when they are ten minutes
  old. A Word export no longer leaves its *.docx.part* file when the file cannot be put in place, the old file is not
  deleted before the new one is in place, and an old *.docx.part* of a cut-off export beside the target goes at the next
  export. The speech add-on removes old *.txt.part* files of a cut-off transcription from its folder. (The list of all
  temporary names is in the developer notes.)
- **Copies that were not checked.** The speech add-on's copy of a model file you choose is checked (a full disk gave a
  part of the file that went on to the model check); the failure is told. When the application menu entry is made and
  the program's icon cannot be copied, the message says so.
- **Saving options and themes to a file never destroys the file that was there.** **Manage > Save Options to File** and
  **Save Theme Options to File** (File > Options) write a temporary file beside the target, check it and only then put
  it in place. When the write fails, the file that was there stays as it was and the message says so (before, a
  failed write could leave no file, or an empty one). A new options file holds only the new options.
- **Web Page Import fetches the pictures of a page from the internet only from public addresses.** A picture that lies
  on this computer or on a private network (also after a redirection, and when a name leads there) is left out, and the
  import's result says how many pictures were left out and why. A page that itself lies on such an address (your own
  computer or network) works as before.
- **Four limits for prepared input.** **Install Update or Add-on** refuses a package of more than 400 MB without
  reading it (as the check of the package already did); a backup archive whose sizes cannot be added up is refused
  by **Restore From Backup**, with nothing made; and the link between {{APP}}'s windows and the writing tools reads a
  request of 64 KiB at most.
- **No crash when a window ends.** A window that was closed while the keyboard moved to another window or panel could
  crash at its very end (found by the system test on Android: the program answered the focus change with a window
  already half deleted). The window now cuts its links to the application's signals before anything of it is deleted.
- **FFmpeg 9.1 in the media add-ons** (before: 9.0.2). Media Playback and Extra Video and Audio Formats 2.1 play the
  same formats, with the newer FFmpeg's safety fixes. The source is taken from FFmpeg's own site and checked by its
  release signature.
- **The other add-ons with libraries** (Import Web Page, Import Word 97, Name Generator, Scratch Pad, Text Statistics,
  Speech Transcription) are rebuilt for the new runtime, with a new last number each. Translation and Proofreading
  is 1.3 (next item).
- **Help > Report a Problem…** opens the project's issue forms in the browser; the manual's troubleshooting section says
  where to report a problem and that a security problem goes privately (`SECURITY.md`). The README has a picture of the
  main window.
- **The download page lists the official add-ons** (Translation and Proofreading so far): name, one line of what it
  does, the systems it is for, whether it is inside the full version, and the note for a model that may be used
  non-commercially only. Each system's card shows that system's newest version.
- **The security watch** (the daily check of the parts the project ships) now also looks at PDFium inside Qt PDF, the
  ggml library inside whisper.cpp and the AppImage's own starter, and says in its report when the repository has had
  no change for 50 days (GitHub stops such a check after 60). For the project's builds: a third ready-made pack, for
  the translator's libraries, and the packs' pages name what is inside, the license and the source.
- **Documents:** the manual and the Quick Start take the label of the version ("beta") from the program (a placeholder, as for its name), so
  they cannot say "alpha" again; `docs/design-decisions.md` gives the reasons behind choices that can look like faults
  (README, SECURITY.md and the manual's introduction link to it).

## Windows 0.95.0 experimental (2026-11-26)

The functions of 0.95.0 for Windows (a program update for Windows 0.94.x; the libraries stay those of 0.93.0).
Everything in 0.95.0 above is in it: eleven interface languages with their manuals and welcome projects, themes and
user profiles (View > Profiles), "Shelin's Dark Theme" and the profile "Shelin's", the Labels toolbar, File > Options >
Appearance > Standard Colors, File > Options > Keyboard > Use the Set of My Keyboard Layout, cutting targets in red,
highlights drawn as picked, the Bookmarks tab's zoom per part and its sizes kept, 25 backups at first, and Translation
Helper 1.5. What is Windows' own:

- **The spelling dictionaries of the nine new languages are add-ons, as on Linux**: `dictionary-<code>-windows.qbpkg` in the
  release and inside the full version (English and German still come with the program). Install the one you need
  with Help > Add-ons and Updates > Install Update or Add-on, restart, then choose it in File > Options > Editing. The
  update with all add-ons leaves the dictionaries out, so that it does not install nine languages at once.
- **"Default" follows Windows' own light or dark setting**, and the profiles are kept in the portable folder's data
  folder (`data\profiles`), with everything else of the program.

For developers (nothing changes in the program): the suites of 0.95.0 run on Windows (v0950b leaves out the AppImage's
move and its menu entry); the dictionary add-ons' build script ignores the carriage returns of the Windows checkout;
the official add-on entry's Windows block names the source of Translation Helper 1.5.

## Android 0.95.0 experimental (2026-11-26)

The functions of 0.95.0 for Android tablets (a new APK, as always on Android; it installs over Android 0.94.x and keeps
the settings). Everything in 0.95.0 above that a tablet has is in it: eleven interface languages with their manuals
and welcome projects, themes and user profiles (View > Profiles), "Shelin's Dark Theme" and the profile "Shelin's", the
Labels toolbar, File > Options > Appearance > Standard Colors, File > Options > Keyboard > Use the Set of My Keyboard
Layout, cutting targets in red, highlights drawn as picked, the Bookmarks tab's zoom per part and its sizes kept, 25
backups at first, and Translation Helper 1.5. What is Android's own:

- **The tablet's own texts in every language**: the window "Where Your Work Is Kept", the messages when a file or
  folder cannot be opened, and Help > About were in English or German only; they now follow the interface language.
- **The spelling dictionaries of the nine new languages are add-ons, as on Linux and Windows**:
  `dictionary-<code>-android.qbpkg` in the release (English and German still come with the app). Install the one you
  need with Help > Add-ons and Updates > Install Update or Add-on, restart, then choose it in File > Options > Editing.
- **"Default" follows Android's own light or dark setting**, and a profile's sizes are fitted to the tablet's screen
  when it is applied.

For developers (nothing changes in the program): the preview tests check the Profiles and Labels toolbars at the
tablet widths, the built-in profile's columns on a 1280 px tablet and a dictionary add-on installed on a tablet; the
start check waits for the program's line "menu open" and chooses a File menu row by the name the program logs for it, and on the
Android 14 tablet tries a virtual mouse and keyboard (Ctrl + wheel over the text, Alt + F); the official add-on entry's
Android block names the source of Translation Helper 1.5.

## Android 0.94.0 experimental (2026-11-26)

The functions of 0.94.0 for Android tablets (a new APK, as always on Android; it installs over Android 0.93.x and keeps
the settings). Everything in 0.94.0 above that a tablet has is in it: Help > Find Command, the menus with one place per
topic (Help > Add-ons and Updates, View > Layout, View > Themes and its one theme button, Format > Font), the message
line under the binder (Back Up Now says there where the backup went; the window with OK that, left open, took the
keys and touches is gone), Documents > Group, the writing progress in the footer, and Translation and Proofreading 1.4,
which frees its model after 15 minutes without use. What is Android's own:

- **A new window comes up in the middle of the screen and gets the keys**: Enter and Esc reach it (they went to the
  main window), and with a keyboard attached it no longer stands at the top left under the menu bar.
- **A long menu of the menu bar scrolls on a small screen**: the View menu, longer in 0.94.0, fits a screen of 960 x
  540 but not the room below its title; it was cut there and its last entries could not be reached. It now scrolls,
  as the menus taller than the screen do.
- **The shortcuts that follow the keyboard layout** take the language of the tablet's input method at the first start
  (German: the set without AltGr). Help has no "Add to Application Menu" on a tablet.

For developers (nothing changes in the program): the start check follows the menus of 0.94.0 (File no longer has
Install Update or Add-on; Back Up Now's message line needs no closing), takes a picture after Esc in each window it
opens, measures whether an open menu stands still, and lists the layout the input method reports, which window each
new window and each Enter and Esc reached, and the emulator's input devices with its uinput tool; the preview test
switches the Themes toolbar on before it checks how the toolbars fit (it is off at first since 0.94.0); the official
add-on entry's Android block names the 1.4 source. The download page has, per system, the full version, the program
alone and the program update, and links to a page of its own (`addons/`) with every add-on on its own, the audio and
video add-ons, the Windows zip files and the official add-ons.

## Windows 0.94.0 experimental (2026-11-25)

The functions of 0.94.0 for Windows (a program update for Windows 0.93.x; the libraries stay those of 0.93.0).
Everything in 0.94.0 above is in it: Help > Find Command, the menus with one place per topic (Help > Add-ons and
Updates, View > Layout, View > Themes and its one theme button, Format > Font), the message line under the binder, the
manual's section of the current view with F1, Documents > Group, the writing progress in the footer, Translation and
Proofreading 1.4, Speech Transcription 1.9, and the inspector that always shows the document chosen in the binder (its
check had failed on Windows only). What is Windows' own:

- **The first-start page has no part about the application menu**: Windows has none of the Linux desktop's, and
  Help shows no "Add to Application Menu" there; the setup program and the portable program's guide make the Start
  menu entry, as before. Its first line names **Help > Troubleshooting and Development > This Computer** (it named
  the place of 0.93.x).
- **The shortcuts that follow the keyboard layout** read Windows' own layout at the first start (German: the set
  without Ctrl+Alt, which is AltGr on a German keyboard).

For developers (nothing changes in the program): the suites of 0.94.0 run on Windows (v0940b leaves out the AppImage's
move and the Linux application menu) and v017 counts again; on Qt's offscreen platform, where the Windows build's
program checks start the program on a screen 800 px wide, the first window does not open maximized (showing it
maximized there ended the program inside Qt's toolbar layout); the official add-on entry's Windows block names the 1.4
source and fetches the models of models-3 for its tests, as the Linux block does; the download page names Help >
Add-ons and Updates > Install Update or Add-on.

## Windows 0.93.1 experimental (2026-11-25)

Fixes and small changes of Windows 0.93.0 (a program update for it; the libraries stay those of 0.93.0). Everything in
0.93.1 above is in it: fonts named by their kind, search and the counts keeping less in memory, the new lines of
Help > System Information, and Qt PDF named with its own source in Help > Third-Party Licenses. What is Windows' own:

- **The list of the Windows files' third-party sources names Qt PDF** (`THIRD-PARTY-SOURCES-windows.txt`), with its
  source `qtwebengine-everywhere-src-6.140.0.tar.xz` in the release "Third-party source code", as Help > Third-Party
  Licenses now does. The parts Qt carries inside itself were already listed in the versions of Qt 6.12.1 on Windows.

For developers (nothing changes in the program): the description of the script that builds FFmpeg for "Extra Video and
Audio Formats" says how MSVC's environment reaches MSYS2 (the same FFmpeg 9.1 and configuration; its pack was built
anew); the official add-on workflow for Windows lets Git take long paths, which the add-on's folder in this repository
needs, and the entry's Windows block names a commit of its own (one of the add-on's own checks now waits for the
other translations it asks for in the background, which GitHub's Windows machine gave late); the Windows build's early pick of its own version line is checked (a build with another Linux line names the
Windows line); the generic font names are checked on Windows too.

## Android 0.93.2 experimental (2026-11-25)

Fixes of Android 0.93.1 (a new APK, as always on Android). Everything in 0.93.1 above is in it: fonts named by their
kind (on a tablet the style Code Span now reaches Word, e-book and PDF files in a fixed-width font), search and the
counts keeping less in memory, the new lines of Help > System Information, and Qt PDF named with its own source in
Help > Third-Party Licenses. What is Android's own:

- **No on-screen keyboard while a keyboard is attached**: Qt's text fields asked for the on-screen keyboard after every
  key typed and every click, so with a keyboard attached to the tablet it kept coming up while you typed. The program
  now asks for it only while Android reports no physical keyboard in use (a keyboard folded away counts as none).
  Help > System Information names the keyboard Android reports and Android's own setting for the on-screen keyboard
  while a physical keyboard is in use.
- **Built with the Android NDK that Qt 6.12 is built with** (r30 instead of r27c); the C++ runtime inside the APK is
  r30's. The app still runs from Android 9 on.
- **A picture or figure cut and pasted inside the program comes back with it.** After a cut the program's own copy
  (with the picture) was replaced by an empty one, so the paste took Android's clipboard, which keeps only text and
  HTML, and the figure was gone.
- **File > Show Project in File Manager on Android 9**: the Files app of Android 9 could not open a folder's address
  and closed itself; before Android 10 it now opens at the top of the tablet's shared storage.
- **Help > Third-Party Licenses** names Qt PDF 6.140.0 as a part of its own with its source (the Qt Project's archive
  of Qt WebEngine and Qt PDF) and the C++ runtime of NDK r30; `THIRD-PARTY-SOURCES-android.txt` names Qt PDF's source.

For developers (nothing changes in the program): the Android system test app starts again (its Qt Test library needs
NDK r30's C++ runtime); a system test run without cases fails, and lines of the emulated graphics driver are no longer
read as cases; its checks of a Word file's fixed-width font and of an e-book's tables work on a tablet, it answers the format question
of File > Save As, translates in the direction of the model it gives, leaves out Help > Report a Problem (Android's
browser puts the app away) and keeps the program's own log lines of each case; a paste on Android writes to the log
what the clipboard gave and whether the program's own copy was used. The start check runs a
second tablet with Android 9 and no keyboard attached in the last run before a merge, checks that the on-screen
keyboard stays down while a keyboard is attached, closes the text's popup before its File menu steps and reaches the
File menu's last command with the keys. The translator's Android libraries are built with NDK r30 too (the official
entry's Android block names the add-on's commit with that note).

## Android 0.93.1 experimental (2026-11-24)

A fix of Android 0.93.0 alone; everything of Android 0.93.0 below is in it.

- **Translation and Proofreading works again on the tablet.** The package of it inside the APK of 0.93.0 (and the one
  published beside it) still named the libraries of Android 0.92.0 (Qt 6.12.0), so Quillbinder 0.93.0 refused to
  install it, at the first start and from File > Install Update or Add-on. Its package now names the app's own, and
  the build checks that the two are the same.

## Android 0.93.0 experimental (2026-11-24)

Quillbinder for Android tablets at the level of Linux 0.93.0. Everything in 0.93.0 above is in it: the panes placed by
the window's shape (also when you turn the tablet), File > Save As, the mind map's overview, the translator for single
words and both directions, and the fixes around saving and backups. What is Android's own:

- **Qt 6.12.1 for Android**, with the safety fixes of its built-in HarfBuzz and PCRE2 (and newer libpng, libjpeg-turbo
  and FreeType). This is a security update; it comes, as always on Android, as a new APK.
- **New windows and the keys with the mouse now work through Qt itself.** Qt 6.12.1 repairs both faults Android
  0.92.0 worked around: a window that opened while the on-screen keyboard was busy could end the program, and mouse
  clicks and wheel turns came without the keys held down. Quillbinder's own workarounds are gone. On an emulated
  tablet, windows opened from the File menu with the keyboard up and the app put away right after typing ended
  nothing, and Ctrl + click, Shift + click and Ctrl + wheel work as before.
- **The mind map's overview by finger.** A tap in the overview moves the view there, a finger drags the rectangle of
  the part shown, and the grip in its corner changes its size. Before, a finger on the overview scrolled the board
  under it, the other way.
- **File > Save As asks for the format first** (Scrivener 3 or Quillbinder), then opens the file window.
- **Newer add-ons without libraries are offered on the tablet too.** The extra themes, the templates and the formats
  come with the Linux release; the check for updates now names them for Android as well.
- Translation and Proofreading 1.3 comes with the app, as on Linux and Windows.
- The tests: the preview test checks the overview by finger in an upright tablet's window and in a phone-sized one,
  the unloading of documents in a project of 300 documents (a word the on-screen keyboard is still composing is
  kept), and the update file's Android block; the start check on the emulated tablet also opens File > Save As with
  the keyboard up.

## Windows 0.93.0 experimental (2026-11-24)

Quillbinder for Windows 11 at the level of Linux 0.93.0. Everything above is in it. What is Windows' own:

- Qt 6.12.1 for Windows, with the safety fixes of its built-in zlib, HarfBuzz and PCRE2, and zlib 1.4 inside
  `quillbinder.exe` (1.3.2 before). This is a security update. Because the program's libraries changed, a
  Quillbinder 0.92.0 for Windows gets the new setup program from its check for updates, not the program update.
- **Extra Video and Audio Formats** carries FFmpeg 9.1 with its safety fixes. Qt 6.12.1 for Windows still brings
  FFmpeg 9.0.2, so the project now builds FFmpeg itself, with the same configuration Qt uses.
- Mind maps: background shapes keep their size when a board is opened. In 0.92.0 a shape drawn elsewhere showed as
  a tiny circle at its corner, and a board saved on Windows kept the shrunk sizes. A board saved with 0.92.0 on
  Windows keeps those sizes; draw such shapes again where needed.
- Installing an add-on or a program update tries again for a few seconds when Windows still holds a file of the
  installed version (a virus scanner reading it, for example), as it already did for the new version.
- Translation and Proofreading 1.3, as on Linux.

## 0.92.0 beta (2026-11-20)

A new AppImage and a program update for Linux, and the first version from the project's one repository,
`Shelin-boop/Quillbinder_Collection`. Three new functions, an add-on that moved in, and four groups of fixes. The add-ons of 0.80.0 and later
fit it, and 0.80.0 and later can take the program update (the check for updates needs the new AppImage once, for
its secure connection).

- **A fixed mind map**. One document or folder of a project can be fixed: the inspector's Mind Map tab then keeps
  showing that item's mind map, whatever is selected. A pin on the binder's rows (on the fixed row, and on the row
  under the mouse) fixes and releases, and so do the pin button of the tab and two commands in **Navigate > Inspect**.
  While an item is fixed, a slim bar at the binder's bottom names it and has **Release**. Kept with the project,
  released when the item goes into the Trash.
- **A padlock on what another device works on**. Each device notes beside the project's lock which documents it has
  open or has changed; a second device that opens the project shows a padlock on those rows and on the folders
  around them, with the device's name in the tool tip. A hint: nothing is blocked. The note is written only when it
  changes and goes when the project is closed.
- **Check for updates**. **Help > Check for Updates at Startup** (on at first, after a note at the first start) and
  **Help > Check for Updates Now**. At most once a day the program fetches one small signed file from the project's
  page, sends nothing about you, and compares on its own. A window offers a newer program version or add-on:
  **Download and Install**, **Ignore This Version**, **Delay** (1 day, 1 week, 2 weeks, 4 weeks, then one last
  reminder). A security update is marked as recommended, cannot be ignored and keeps reminding. A version that needs
  a new AppImage is put in the place of the AppImage you started, wherever it is, and the one before is kept beside
  it. It is the only thing for which Quillbinder contacts the internet on its own; switched off, it contacts nothing.
- **Translation and Proofreading is one of Quillbinder's own add-ons now**. Version 1.2 comes inside the full
  version, its source is in Quillbinder's repository, and it downloads its language models from there (before: an
  official add-on with a repository of its own). English text is checked, German text is translated into English,
  on your own computer. Nothing else changed since 1.1; models you already have stay. Quillbinder itself still has
  no AI functions: the add-on can be switched off or removed.
- **No more false "Other Versions Kept" in a synced folder**. Some sync programs set a file's time again a few
  seconds after Quillbinder wrote it. That alone counted as a change from elsewhere, so the notice came at every save
  and left a copy and a snapshot each time. Now the content decides: the same bytes are no other version, nothing is
  kept, told or written, and a project file that already holds what a save would write is left alone. A real change
  elsewhere is kept and told as before. The notice has a button **Don't Show Again Until Quillbinder Is Closed**.
- **Backups carry the project's ID**. A backup is now named *Title #K7T2M bak 2026-11-20 15-40-00.zip*. Two projects
  with the same title keep their own backups and their own number kept; before, each could push the other's older
  backups out. The ID is the same on every computer the project is opened on. Backups of earlier versions (no ID in
  the name) are no longer deleted by the clean-up and are still offered by **Restore From Backup**: delete them
  yourself when you no longer need them.
- **A copy that fails is told**. At **File > Import > Scrivener Project**, at **File > Import > Files** and when a
  Scrivener project is read again after a change elsewhere, a file that could not be copied was left out without a
  word. Now the report names it, a file import leaves no empty item, and a re-read that cannot carry a file over
  stops and leaves everything as it was. An import whose final save fails is told as failed.
- **Failures that were told too quietly**. The reason of a failed save ("The disk is full.") no longer depends on
  the system wording its errors in English. A manual save whose backup cannot be made shows a notice, not only the
  status bar. A choice in the Add-on Manager, or an install, that could not be written is reported as failed; and
  when a failed install cannot put the earlier version back, the message says where it is.

## Android 0.92.0 experimental (2026-11-20)

Quillbinder for Android tablets at the level of Linux 0.92.0, and the first Android version released from the
project's one repository. Everything in 0.92.0 above is in it. What is Android's own, most of it for a tablet with an
external keyboard and mouse:

- **Ctrl and Shift work with the mouse.** Ctrl + mouse wheel zooms every part, and Ctrl + click or Shift + click
  chooses several documents in the binder. Qt 6.12.0 for Android hands mouse clicks and wheel turns to the program
  without the keys held down; Quillbinder now remembers them from the keyboard (until a Qt version with the repair
  is used).
- **Every window can be closed.** About is a window whose text scrolls above its buttons (before, its buttons could
  lie below the screen). The manual, the Quick Start Guide, the Developer Guide and the Objects window have a Close
  button. A dialog that is taller or wider than the screen (or than the room the on-screen keyboard leaves) scrolls,
  its buttons stay in view.
- **Back closes the menu, the tip or the window in front** instead of putting the app away. With nothing open in
  front of the main window, Back does what Android does.
- **The line between two panes is easier to find and to move.** It is wider and shows a grip, and a mouse pressed a
  few pixels beside it takes it along. On a tablet held upright the binder can be made wider or narrower: only the
  page a pane shows counts for its least width, so the binder, the text and the inspector fit side by side.
- **Menus stay below the menu bar.** A menu that is longer than the room below the bar scrolls there, instead of
  wrapping into columns or covering the other menus.
- **The fixed mind map and the shown lock by touch.** The pin shows on the selected binder row (a tablet has no
  place "under the mouse"); a click on it fixes the mind map, and the row's menu has Fix and Release too. A tap on a
  row's padlock shows who works on it and since when.
- **Translation and Proofreading 1.2 comes with the app**, built from the same source as on Linux and Windows and
  installed at the first start of the version; its models are downloaded only when you choose one. The release also
  carries its package on its own. Before, it was the official add-on 1.1 from a repository of its own.
- **Help > Check for Updates knows Android.** A new version is always a new APK: Quillbinder tells you when there is
  one and opens its download in the browser (Quillbinder asks for no right to install apps); newer add-ons are
  installed inside Quillbinder, as on Linux.
- The tests: a preview test on the desktop opens every menu and every window a command opens at nine tablet sizes
  (upright, and with the on-screen keyboard up) and checks that each fits, shows its buttons, and closes with a
  button and with Back; it also checks the keys with the mouse, the line between panes, the pin and the padlock.

## Windows 0.92.0 experimental (2026-11-20)

Quillbinder for Windows 11 at the level of Linux 0.92.0, and the first Windows version released from the project's
one repository. Everything above is in it. What is Windows' own:

- Translation and Proofreading 1.2 comes inside the full version (setup program and zip) and as a package of its
  own, built from the same source as on Linux. Before, it was an official add-on from a repository of its own.
- The check for updates knows Windows' files. The usual update is installed inside Quillbinder, as on Linux. A
  version that needs the whole program anew (new libraries) is downloaded as its setup program into Quillbinder's
  own folder, checked against the project's signed list, and started when you choose **Close and Update Now**:
  Quillbinder closes, the setup program unpacks the new version into the folder you use, without its pages, and
  starts it. Your settings, add-ons and everything else in `data` stay. Where Windows blocks the setup program
  (it carries no code-signing certificate), Quillbinder says so and names the file.
- The Windows release now also carries the program update alone and every add-on as a package of its own (their
  names end with `-windows`), which is what the check for updates offers.
- A setup program can be run as an update (`/QBUPDATE=1`): it waits until the Quillbinder in the folder has ended.
- Installing a program update no longer fails with "Cannot create ...\data\core\<version>" when Windows still
  holds a file of the new version for a moment (the new version is started once before it is put in place, and a
  virus scanner may be reading it): Quillbinder tries for up to five seconds before it gives up.

## 0.91.3 beta (2026-11-15)

A new AppImage and a program update for Linux. It prepares the list of official add-ons, which other authors can
also write: their add-ons are signed with a key of their own that the project vouches for. The add-ons of 0.80.0 and
later fit it, and 0.80.0 and later can take the program update.

- **Official add-ons can carry a key of their own**. Besides the project's own key, which signs the program and its
  updates, there can now be keys for official add-ons. Such a key is trusted when the package carries a certificate
  in which the project's own key vouches for it, for add-ons only. The install window then names the key, for
  example: *Signed with the add-on key "Quillbinder official add-ons", which the Quillbinder project vouches for*.
  From this version on, the project's own add-ons are signed with that key, and the translation and proofreading
  add-on will be the first official add-on of another author.
- **A program update still needs the project's own key**. An add-on key can never stand in for it: a program update
  signed with an add-on key is refused like an unsigned one, and an installed one is not started.
- **Everything you installed stays as it is**. Add-ons signed with the project's own key, as all of them were up to
  now, stay trusted, and an add-on without a signature can still be installed after the warning. Versions up to
  0.91.2 do not know add-on keys: they treat an add-on signed with one as unsigned and warn before installing it, so
  install the program update to 0.91.3 first (it is still signed with the project's own key), then the add-ons.
- **The release checks every signature by the program's rule**. Before the packages of a release are handed on,
  each signature is checked against the public key the program carries, the same way the program checks it.
- **A workflow for official add-ons**. It builds an add-on of another author from the reviewed commit, runs the
  add-on's own checks, installs it into the newest AppImage, and only on request signs it with the add-on key and
  publishes it on a release of its own, with its sources and the note on data for non-commercial use only
  (`docs/official-addons.md`).

## 0.91.2 beta (2026-11-15)

A new AppImage for Linux with internal changes only: the program works as 0.91.1 did. The add-ons of 0.80.0 and later
fit it, and 0.80.0 and later can take the program update.

- **Nothing new in the program, a faster way to build it**. The parts that do not change between versions (the
  audio and video libraries, the speech recognition) are no longer built again for every version, and the compiled
  files of one build are kept for the next. The files you download are made from the same sources as before.
- **The download page shows each system's newest version**. Linux, Windows and Android now count their own fixes,
  so the newest Windows version can have another last number than the newest Linux version: the page keeps the
  Windows download of 0.91.1 next to Linux 0.91.2 instead of saying that it comes later.
- **The manual calls the Windows version published**. Its part about Windows said that the Windows version was in
  preparation and not published yet; it is experimental and published since 0.91.1.

## 0.91.1 beta (2026-11-13)

A program update for Linux after the writer's first hours with the toolbars of 0.91.0: they could not change their
order, a toolbar could not go from one row into the other, and a row could get stuck.

- **Toolbars can be put in any order and into either row**. Drag a toolbar by its grip onto another toolbar or into
  another row: a line in the highlight color shows where it will stand, and it goes there when you release the mouse
  button. A main toolbar can stand in the format row and a format toolbar in the first row. Dropped just under the
  toolbar rows, a toolbar gets a row of its own; dropped on the bottom edge of the window, it stands there. Esc leaves
  everything as it was. In 0.91.0 the order could not be changed, and on KDE with Wayland a toolbar could not leave
  its row: the program now moves the toolbars itself instead of leaving that to the desktop.
- **A toolbar row no longer gets stuck**. With a toolbar pulled to the far right of its row, the toolbars before it
  could not be moved anymore. A toolbar dragged along its row now takes the free space after it first and pushes the
  toolbars after it only when it reaches them, as far as they can stand together at the row's end.
- **Free space stays with the toolbar it stands in front of**. The space you leave in front of a toolbar is kept when
  other toolbars change their places or are switched off. In a window too narrow for it the space shrinks before a
  toolbar loses buttons, and it is back when the window is wide again. The rows, the order and the spaces of 0.91.0
  are taken over.

## 0.91.0 beta (2026-11-13)

A new AppImage for Linux with three requests of the writer: all backups straight in the backup folder, toolbars that
can be moved and switched off one by one, and theme buttons that switch the look with one click.

- **All backups are straight in the backup folder again**. The automatic backups of every project go into the folder
  chosen under File > Options > Backup itself, named *<title> bak <date time>.zip* as always; versions 0.70 to 0.90
  put them into a folder per project inside it. A project's backups are the files with its title, and only they count
  toward the number to keep. Two projects with the same title share their backups (give a copy a title of its own);
  after a title change the backups made under the old title stay until you delete them. An older backup with the
  project's title that lies in the backup folder counts again. The folders per project of 0.70 to 0.90 are left as
  they are, and Restore From Backup still lists their backups. Show Backup Folder opens the backup folder.
- **Eight toolbars that can be moved**. The two toolbars became seven: Main, Themes, Search and Tools in the first
  row; Style and Font, Character and Alignment, Paragraph and Insert in the second, followed by the icon bar. Each has
  a grip at its front (two columns of dots): drag it sideways to put the bar further right or left in its row, and the
  bars after it make room; drag it up or down to put the bar into another row or at the bottom edge of the window. The
  program remembers the places and the spaces between the bars.
- **A switch for every toolbar**. View > Toolbars, and a right-click on a toolbar, switch each bar on and off. Show
  Toolbar and Show Format Bar switch a whole row as before. Reset Toolbar Places puts every bar back.
- **Customize Toolbars works with every toolbar**. Buttons can be added to each of the seven bars; a control such as
  the quick search field moves from one bar to another. A toolbar arranged in an earlier version is split over the new
  bars, and what was taken off it stays off.
- **Theme buttons that switch: Bright, Dark, U1 and U2**. Bright and Dark show a bright and a dark theme with one
  click; a right-click chooses which theme each of them shows. U1 and U2 keep two looks of your own (the themes User_1
  and User_2): a click on an empty button saves the look you see, a click on a filled one brings it back, a right-click
  saves the current look there. The button "Both" is gone, and the list shows every theme.
- **A look of your own is not lost by a click on a theme button**. A button shows its theme as it is. Colors and fonts
  of your own on top of the theme shown before are saved first: into an empty U button without a question, otherwise
  the program asks whether to save them in U1 or U2 or to switch without saving.

## 0.90.4 beta (2026-11-13)

The first version built for Linux and Windows from one source. For Linux it is a program update for 0.90.3 (File >
Install Update or Add-on; it fits 0.80.0 and later) that changes nothing in the writing work. For Windows 11 it is an
experimental build, not published yet, with what Linux 0.90.3 has.

- **One source for Linux and Windows**. On Linux this update changes nothing in the writing work. It carries the
  number of the first Windows build made from the same source.
- **Windows: what Linux 0.90.3 has**. The Scrivener format in full (0.81.2 to 0.90.0), the usability changes of
  0.90.1, the theme list of 0.90.2 and the backups of 0.90.3, which are packed outside the backup folder and arrive
  there as one finished file. Only Speech Transcription on the graphics card is missing on Windows (it uses the
  processor there).
- **Windows: a setup program**. It asks for a folder and unpacks Quillbinder into it. It needs no admin rights and
  changes nothing else: no entry under Apps, nothing in the registry, no Start menu entry (the guide at the first start
  offers those). The zip files hold the same folders.
- **Windows: AltGr types its character**. Windows reports the AltGr key as Ctrl+Alt, so on a German keyboard AltGr+7,
  8, 9, 0 and < would have run commands instead of typing `{ [ ] }` and `|`. The commands keep their keys (left Ctrl
  and left Alt).
- **Windows: nothing outside the program folder**. A saved web page is unpacked for viewing, and copied for the
  browser, in a temporary folder that is removed when the program closes. Linux keeps the copy in Downloads, for
  browsers that run in a sandbox.
- **Windows: saved web pages in older character sets**. A page saved as windows-1252 or in a similar character set
  keeps its curved quotes, dashes and the euro sign.
- **Also**. File > Options offers Windows' program folder for the bibliography program; a Windows build without test
  switches refuses an unsigned program update, as Linux does; the download page and the release text know the setup
  programs.

## 0.90.3 beta (2026-11-13)

A program update for 0.90.2 (File > Install Update or Add-on; it fits 0.80.0 and later). Backups and sync programs:
found when a sync program filled a backup folder with copies of one backup after its upload was refused.

- **Copies of a backup no longer count as backups**. The clean-up counts and deletes only files named as the program
  names its backups. Copies that a sync program or you put into a project's backup folder ("...-MyComputer-safeBackup-0001.zip",
  "... (conflicted copy).zip", "... - Copy.zip") are left alone; before, they counted as backups of their date, and
  six of them would have pushed every real older backup out of a folder that keeps five. Restore From Backup still
  lists them.
- **A backup is put together outside the backup folder**. It is packed and checked in the program's own folder
  (`~/.local/share/Quillbinder/backup-work`) and only the finished file goes into the backup folder, in one step and
  under its final name (moved on one drive, copied and compared on two). A sync program that watches the backup folder
  sees one finished file arrive, and no ".zip.part" file any more.
- **A backup folder that is out of reach loses no backup**. When the folder cannot be made or written (a drive that
  is not plugged in, no room), the backup is made all the same, waits in the program's folder, a message names both
  folders, and the next backup puts it in place. A copy that was cut off is replaced by the whole backup.

## 0.90.2 beta (2026-11-13)

A program update for 0.90.1 (File > Install Update or Add-on; it fits 0.80.0 and later). The writer's list from testing
0.90.1.

- **A theme list in the main toolbar**. In the middle of the main toolbar, Bright, Dark and Both narrow the list of
  themes beside them; a theme chosen there is used as with Window > Themes (the open project and new projects). The
  choice of Bright, Dark or Both is kept; a main toolbar arranged by hand gets the list once, after Redo. On a crowded
  toolbar (a narrow window, large icons with text) the list gives way to the quick search and comes back with room.
- **A color of its own for Show Invisibles**. File > Options > Appearance > Textual Marks (and right-click >
  Appearance on the text) sets the color of the dots, arrows and pilcrows; it belongs to the project's look. Without
  one they look as before (the text faded into the paper).
- **No lone "Page View" entry in Appearance**. Its one color, the desk around the pages, is now in Main Editor as
  "Around the pages (only in page view)"; it had seemed to do nothing, since it only shows in page view.
- **No P and A circles in File > Options**. Where an option is kept stays in its tool tip; the circles drew too much
  attention, and the room they took pushed the color buttons of the Mind Map page to the far right.
- **A rare crash when a dialog closed** (from 0.90.1). Giving the keyboard back after a dialog read a window or field
  that had been deleted while the dialog was open; it is now guarded. Found by the regression run (the ui suite crashed
  in about one run of three).
- **The download page lists more**. Under the full version of each system: the program alone (the small AppImage),
  the program update and the audio and video add-ons.

## 0.90.1 beta (2026-11-13)

A program update for 0.90.0 (File > Install Update or Add-on; it fits 0.80.0 and later). Memory during long writing
sessions, and the findings of the usability review of 0.90.0 the writer chose (UR-01, 02, 05, 07, 08, 10, 12, 13, 14,
16, 18, 19).

- **Menus that grew at every opening**. Navigate > Go To and Open Quick Reference (the binder as nested menus),
  Documents > Change Icon, Format > Style (Edit Style, Delete Style), the mind map's menus and the script settings' menu
  are filled anew each time they open; the old submenus stayed in memory with all their entries. In the system test's
  writing day the program held 296 MB after 10 minutes, about 70 % of it these menus; now 83 MB (measured with
  heaptrack). With a large project each opening of Go To kept a copy of the whole binder.
- Measured, not changed: left open with a project, the program's memory stays the same (30 minutes, 110 MB); no memory
  is lost in the sense of a leak (under 0.4 MB at the end of the writing day). What remains is the texts shown in a
  session and their layout, which stay in memory until the project closes (documents are not unloaded yet).
- **Where an option is kept** (UR-10, UR-03). In File > Options a small circle stands before every option: green "P",
  kept in the open project; blue "A", kept by the program for all projects. Its tool tip says whether that place is
  fixed (the language, the backup folder ...) or depends on "Settings for" (and, for colors, fonts and widths of the
  look, on "Show and change the look of").
- **Dialogs on small screens** (UR-02). No window of the program is larger than the screen without the desktop's
  panels: Options scroll, Compile and the license list get smaller; OK, Cancel and Apply stay in view (before, a
  1366 x 768 laptop could hide them below the edge).
- **The keyboard comes back** (UR-01). When a dialog closes and no window of the program has the keyboard (seen on a
  desktop without a window manager; KDE usually gives it back itself), the window the dialog came from gets it again,
  in the control that had it: words typed after closing Options with Esc reach the text.
- **Undo and Redo name their step** (UR-12): "Undo Typing", "Undo Paste", "Undo Bold", "Redo Delete" (the step names
  of every menu command, of typing, deleting and pasting). They act in the field that has the keyboard and are off
  elsewhere, so after Empty Trash an enabled Undo no longer suggests it could bring the documents back. The arrow beside
  the toolbar's Undo and Redo lists the steps; choosing one takes back that step and every later one. Each Format
  command is one step now (Bold made two, so Undo had to be pressed twice).
- **Shortcuts** (UR-07): Navigate > Inspect > Go to Synopsis (Ctrl+Alt+Y) and Go to Document Notes (Ctrl+Alt+N) put
  the keyboard there; Back Up Now is Ctrl+Shift+B; Ctrl+Alt+4 to 9 apply the next six styles of the style list (after
  No Style and Heading 1 to 3, which keep Ctrl+Alt+0 to 3). [Not checked: whether a desktop takes one of these for
  itself; File > Options > Keyboard changes them.]
- **Right-click on the binder's empty space** (UR-14) offers only New Text, New Folder, New From Template and
  Appearance (before: every command of the selected document, such as Move to Trash and Merge).
- **Close buttons** (UR-16) in Find by Formatting and in Print Preview (Qt's preview had none).
- **Project Statistics** (UR-08) shows the busy pointer while it counts (2.7 s on the system test's large project).
- **The question before a link to a file opens** (UR-05, self-descriptiveness as ISO 9241-110 asks) names the kind of
  file ("plain text document (.txt)"), the whole name and the folder. A long name wraps anywhere, so its ending can be
  read, and a file that is not there is said to be missing.
- **Large numbers** (UR-18): always with a separator between the thousands, as the system writes numbers (44,825 in
  English, 44.825 on a German desktop or in the German program); before, a system set to plain "C" (such as the test
  machines) wrote 44825. File > Options > General > Startup, "Group the digits of large numbers", switches it off.
  Units, paper sizes and the decimal mark follow the system as before.
- **Hint texts readable** (UR-13): the grey text in empty fields (the synopsis' "Synopsis: a short summary ...", Quick
  search) has a contrast of 4.5:1 or more, in light and dark looks (before about 3:1).
- **Compile window** (UR-19): the Formats column is at least as wide as its longest name at any scale (at scale 2
  "Default (Editor Formatting)" touched the edge); the widths of the three columns set by hand are kept; the arrow of
  "All categories" no longer covers its text.
- The German texts of all of this.

## 0.90.0 beta (2026-11-12)

A new AppImage on the runtime of 0.80.0 (`u2404-qt6.12-r1`, the same libraries: the add-ons of 0.80.0 fit, and 0.80.0
and later take the program update). The Linux version that completes the work on Scrivener compatibility: everything
found in two projects of Scrivener 3 for Windows is read, shown where Quillbinder has it, and written back in
Scrivener's own form.

- **Texts written back as Scrivener wrote them**. Checked with every text of two real projects written anew and
  measured by a reader of the checks' own: the same characters, bold and italic, colors, fonts, sizes, indents, tab
  stops, lists, pictures and links. Before, a text changed in Quillbinder lost in Scrivener: right indents below zero
  (lines that run past the page for a tab stop out there), the link of a picture linked to a file, the underlining of
  links to web addresses, and the list table of bullet and numbered lists (numbered lists became bullets); and it gained
  bold and italic switches on fonts that are named by their bold or italic face (as Scrivener for Windows names fonts).
  The place of a picture whose data is missing stays in the text.
- **Comments and footnotes keep what Scrivener keeps with them**: their IDs, whether the box is folded, the comment's
  color and the formatting inside an unchanged note.
- **Bookmarks**: the bookmarks of each document are read and written (they stayed in Quillbinder only), and bookmarks
  of the project to web addresses or files stay in the Scrivener project (every write took them out).
- **Targets**: a document's target, the deadline of the draft target (it was not read, and every write switched
  Scrivener's deadline off), and whether only the draft counts and whether the session target follows the deadline.
- **The writing history** (the words written each day) is read from the Scrivener project and written into it. Opening a project no longer counts its whole text as written that day.
- **Compile**: the title, the author and the language of the compiled file are shared with Scrivener's compile settings
  (Scrivener's compile formats stay its own).
- **The binder as Scrivener left it**: the first opening shows the folders that were open in Scrivener and the documents
  selected there.
- **The corkboard**: cards placed freely in Scrivener keep their places in Quillbinder and back (Scrivener keeps them in
  card widths: Quillbinder turns them into its own card size); each folder keeps its corkboard arrangement, freely
  placed or in a grid, as it does in Scrivener.
- **Labels and statuses**: the default label and status of new documents stay as set in Scrivener (every write set them
  to none), and the names of "No Label" and "No Status" stay as the project has them (with the German interface they
  were written in German).
- **Saved searches**: Scrivener's saved searches are search collections in Quillbinder, and Quillbinder's search
  collections go into the Scrivener project as saved searches (they stayed in Quillbinder only). Checked with a search
  for any of the words; the other kinds were not seen in a real Scrivener project yet (see the manual).
- **Index card pictures**: a card that shows a picture instead of its synopsis keeps it, in both directions.
- **Found on the way, fixed**: a list that began with the first paragraph of a text lost its first item (at every
  import of a Scrivener project, Roll Back and the like); an empty last paragraph of a text was dropped; a project file
  without section types got Quillbinder's standard ones at the first write; keywords were put in another place among a
  document's parts than Scrivener puts them.
- **Found by the system test, fixed**: a list item with a paragraph style (a heading, a caption ...) lost its style at
  the next opening in projects that keep their texts as Markdown (the list's mark took the place of the style's mark;
  now written as `{: style=...}` before the mark, which earlier versions ignore).

## 0.81.3 beta (2026-11-12)

A new AppImage on the runtime of 0.80.0 (`u2404-qt6.12-r1`, the same libraries: the add-ons of 0.80.0 fit, and 0.80.0
and later take the program update). Projects in Scrivener format, part 2 of complete Scrivener compatibility: styles,
inline notes and web pages go into the Scrivener project in the form Scrivener 3 for Windows writes them.

- **Styles in Scrivener format**. A text changed in Quillbinder lost its paragraph and character styles in Scrivener
  (they stayed only as the look of the text). They are written as Scrivener keeps them now (marks in the text and the
  list of styles each text uses), so Scrivener shows the same styles; a style made in Quillbinder is added to the
  Scrivener project's styles. Scrivener's character styles and its styles that Quillbinder had no style of that name for
  are read too (they became plain formatting before).
- **Inline footnotes and annotations in Scrivener's own form**. A text changed in Quillbinder gave them to Scrivener as
  linked notes on the word before them. They stay inline footnotes and annotations now, the annotations in their color.
- **Pictures in the texts of Scrivener for Windows** (found while checking). Quillbinder did not show them, and a text
  written by Quillbinder lost them in the Scrivener project. They show and stay now. A picture placeholder that
  Scrivener keeps without the picture's data is not written back.
- **Web pages**. A web page Scrivener saved showed as codes (it is one archive file, `.mht`). It shows as a page now,
  with its address and two buttons above it: **Open Saved Page in Browser** shows the page as it was saved (a copy goes
  into the Downloads folder, "Quillbinder web pages": a browser installed as a sandboxed package, such as Firefox on
  Kubuntu, cannot read the program's own folders) and **Open Original Address** opens the page as it is today. Web pages
  saved by Quillbinder keep their address and go into a Scrivener project as Scrivener's own saved web pages.
- **Notes of pictures, PDFs, audio and video** go into the Scrivener project too (only the notes of texts did).
- **Audio, video and web pages that earlier versions turned into text documents** in a Scrivener project (a text that
  named the file) are put right at the next save: Scrivener's own audio, video or web page document again.
- **The checksum file of Scrivener for Windows** (found while checking). Every write rebuilt it in another form: lines
  Scrivener wrote were lost, and a text written by Quillbinder got a second line beside its old one. It keeps
  Scrivener's form now. Scrivener's backup copy of the binder is removed only when Quillbinder writes the binder (it was
  removed at every save, also without a change).

Not yet written into a .scriv: Quillbinder's search collections (Scrivener's form of a saved search is not known yet).

## 0.81.2 beta (2026-11-12)

A new AppImage on the runtime of 0.80.0 (`u2404-qt6.12-r1`, the same libraries: the add-ons of 0.80.0 fit, and 0.80.0
and later take the program update). Projects in Scrivener format: keywords, custom metadata and collections go into the
Scrivener project now, in the form Scrivener 3 for Windows writes them (part 1 of complete Scrivener compatibility).

- **Keywords, custom metadata and collections in Scrivener format**. They stayed in Quillbinder's working copy and
  never reached the .scriv, so Scrivener did not see keywords, values or collections made in Quillbinder. They are
  written into the .scriv now, under Scrivener's own numbers and names, so a keyword is the same in both programs. A
  project opened and saved without a change stays as Scrivener wrote it.
- **Nothing lost when the project was changed in Scrivener**. When Quillbinder read a project again after work in
  Scrivener, its keywords, fields and collections could replace the ones made in Quillbinder, and things deleted in
  Scrivener could come back. Everything comes from the .scriv now. The first time a working copy of an older version
  opens, Quillbinder joins its keywords, fields, values and collections with the Scrivener project's once, so nothing
  made in Quillbinder is lost.
- **Custom metadata values from Scrivener**. Scrivener keeps a ticked box as "Yes", a list's choice as a number and a
  date with its time; Quillbinder showed such boxes unticked, lists empty and dates as "none". They show as in
  Scrivener now; in projects imported earlier, boxes and dates are put right when the project opens (a list's choice
  cannot be: import the project again for it). A text field's wrap setting is kept with the project (it was lost at
  the next save).
- **Collections from Scrivener for Windows** opened empty (it lists their documents in one line). They show their
  documents now.
- **Inline notes from Scrivener for Windows** showed as codes in the text (`{\Scrv_fn=` ...). They are inline footnotes
  and annotations now, the annotations in their color; projects imported earlier and working copies are put right when
  they open.
- **Audio, video and web pages became text documents** (found while checking). Every save into a .scriv turned
  Scrivener's audio, video and saved web pages into text documents in Scrivener's binder. They keep their kind now,
  and audio and video added in Quillbinder go in as Scrivener's own audio and video documents (before: a text that
  named the file).
- **A save without a change rewrote the binder** of a project of Scrivener for Windows (found while checking):
  Quillbinder left out the file type of texts and the titles of project bookmarks. Both stay now.

Not yet written into a .scriv (part 2): styles, inline notes in Scrivener's own form (a text changed in Quillbinder
gives them to Scrivener as linked notes), Quillbinder's search collections, and web pages saved by Quillbinder.

## 0.81.1 beta (2026-11-11)

A new AppImage on the runtime of 0.80.0 (`u2404-qt6.12-r1`, the same libraries: the add-ons of 0.80.0 fit, and 0.80.0
and 0.81.0 take the program update). It brings the fixes of the four findings at function level of the audit of
0.81.0 (part 1).

- **Documents whose file cannot be read** (F-01). Such a document (damaged, saved by another program in another
  character encoding, or not arrived yet through a sync folder) is shown read-only and never written over. Commands
  could still change it in memory, and the save then counted it as saved: a roll back seemed to repair it until the
  project was opened again, and a merged text existed only in memory while the other documents went to the Trash. Now
  Roll Back repairs a damaged document (its damaged file is kept as a copy in `settings/conflicts` first, then the
  snapshot's text is saved in its place); Merge Documents and Split refuse with the reason; Project Replace, Convert
  Quotes and style changes leave it as it is and name it. A change a command makes all the same is kept as an RTF file
  in `settings/conflicts`, and a message says where.
- **Pictures in notes** (F-02). A picture pasted into notes from a word processor was written as a new file with every
  save of those notes; the project folder, the backups and the sync folder grew. It is written once now.
- **An interrupted save whose journal cannot be read** (F-03). The journal and the files of that save were deleted, and
  the notice said the project was as at the save before, though some of its documents could be from the newer save.
  Now nothing is put in place or deleted: the journal and those files are kept in `settings/conflicts`, and the notice
  says what was found. In a sync folder, the journal of the other computer's save is left alone for 15 minutes (that
  save may still be running there or its files still arriving); this computer does not save the project meanwhile
  and tries again by itself.
- **A freeze on a damaged RTF file** (F-04). A damaged RTF file could give a table that a figure ran across; writing such
  a text (a save in RTF storage, the emergency copy, Copy, Scrivener format) never ended, and the program had to be
  stopped. The reader no longer lets a figure and a table row take the same paragraphs, and the writer stops at the end
  of every table cell. The auditor's file is a fuzzing input of the tests now, and the fuzz tests write RTF as well.
- **Roll Back showed an empty page** (found while checking F-01). After a roll back, the editor kept the old layout of
  the text and showed an empty page until something made it lay the text out again; the text is shown at once now.

## 0.81.0 beta (2026-11-11)

A new AppImage on the runtime of 0.80.0 (`u2404-qt6.12-r1`, the same libraries: the add-ons of 0.80.0 fit, and 0.80.0
takes the program update). It brings the fixes of the audit of 0.80.0 (part 1 of 2: usability, performance,
trustworthiness, stability, data protection, data security, robustness; 24 points).

- **Backups after a new title** (F-01, critical). The age of a backup is read from the date in its name. Before, the
  whole name was compared, title first: after the title changed to one that sorts earlier, every new backup counted
  as the oldest and was deleted the moment it was made, while the program reported success. The backup just made is
  never deleted now, and the program checks that it is still there after the clean-up.
- **Restore of any size** (U-01, F-03). The ZIP reader knows ZIP64 and unpacks each file in pieces to the disk:
  backups of 2 GB and more, files of more than 1 GB (a video, a recording) and more than 65,535 files come back.
  Before, such a backup was called "not a complete ZIP file", and large files were left out.
- **Restore onto a full disk** (F-02). Every restored file is written under a temporary name, synced and checked to
  the end; a file that cannot be written is never left cut short or empty, and the message names each file that is
  missing with the reason. A restore that needs more room than the drive has is not started; a restore shows its
  progress and can be cancelled.
- **Folders that are links** (F-05) are backed up with their files; a link that points nowhere or back into the
  project makes the backup incomplete and is named. Of incomplete backups, the newest ones up to the number to keep
  stay (before, they were never deleted).
- **A backup missed at a shutdown** (S-03). When the program is ended by a signal (a shutdown that does not wait, a
  kill command) with the backup at closing switched on, the next opening of the project makes the backup and says so.
- **Snapshot titles** (F-04). A damaged list of titles is no longer taken for an empty one: the titles that can still
  be read stay, the damaged file is kept beside it, and a list that cannot be written is reported.
- **Saves nobody looked at** (F-06). A failed save started by File > Compile, Project Replace, Empty Trash, the boards
  and other places is told like a failed autosave (the sign in the status bar, the notice, new attempts).
- **Empty Trash** (F-08) asks with the number of snapshots that go with the documents, saves the project first and
  removes the files only after a save that works.
- **Safer copies**: the kept copy of a version changed elsewhere is synced to the disk (F-07); the project file of the
  last three openings is kept (`settings/spare-copy`, `-2`, `-3`; U-02).
- **Memory** (U-04): converting the storage format holds only the documents that are open; the comparison of
  snapshots gives up on two long, very different paragraphs at 2,500 steps (about 25 MB, before up to 144 MB).
- **Autosave at 10 seconds at the latest** (S-04; 30 s before) when typing goes on without any pause.
- **Readable explanations** (S-01). The lines that explain options (File > Options, First Start, Compile, Format,
  Crash Reports, Bookmarks) were drawn in the gray of unavailable controls (contrast 1.62:1); they are drawn in the
  text color now, a little lighter, with a contrast of at least 4.5:1 in every look.
- **First Start asks only about backups** (S-09); which settings come first stays in File > Options > General >
  Startup. **Window titles** no longer repeat the program's name (S-07).
- **The failed-save notice** (S-08) leads with the reason in plain words ("The disk is full.") and the documents,
  keeps the paths in its details, and opens in the window's lower right corner instead of over the text.
- **No computer name in the project** (S-06): the window-state file is named by the computer's anonymous key
  (`settings/ui-<key>.json`); the file of earlier versions is read once and replaced.
- **Test switches are out of the released program** (S-10): the 14 variables only the test suites use are not read
  by a release build any more; the ones a release still reads are listed in the manual (appendix "Environment
  variables").
- **Release** (S-02, S-11): nothing is published before the full test suite has passed (from 2026-11-11 to 0.80.0 the
  release did not wait for it), and a new job runs cppcheck over the saving, backup and update code
  (`tools/code-check.sh`), which must find nothing.
- Checks: `qb_v0810_tests` (new).

## 0.80.0 beta (2026-11-11)

A new AppImage with a new runtime (`u2404-qt6.12-r1`): program updates of 0.70.0 and earlier do not fit it, and the
add-ons with program code come in new versions for it (Media Playback 2.0, Extra Video and Audio Formats 2.0, Speech
Transcription 1.6, Scratch Pad 1.3, Text Statistics 1.1, Name Generator 1.1, Web Page Import 1.1, Word 97-2003
Import 1.3). It does the "runtime step" the external audit of 2026-11-09 asked for (X-6, X-7, ST-4, ST-5) and changes
the license.

- **Qt 6.12 instead of Qt 6.4.2**. The AppImage carries The Qt Company's own build of Qt 6.12.0 (the current
  long-term version, released 2026-09-30) instead of Ubuntu 24.04's Qt 6.4.2, which gets no fixes from Qt any
  more. The program still runs on Ubuntu 24.04 and newer (the same minimum Linux). The separation of fonts with
  several designs (Sitka) works on the new Qt's font list; Qt 6.12's PDF writer fills in the author itself, and
  the program's author, subject and keywords now replace its empty entries.
- **The runtime's age in Help > System Information**: the runtime line names its Qt, the day that Qt was released
  and how many months ago that was, with a note when it is older than 18 months.
- **FFmpeg 9.0.2 in the media add-ons** (before: 6.1.1, a release line FFmpeg no longer maintains), the version Qt
  6.12's FFmpeg backend is made for. Qt 6.12's own build has no GStreamer backend, so the choice "GStreamer of the
  system" is listed as no longer available; a writer who chose it before gets the built-in decoding.
- **Add-ons are tied to the runtime**. Add-ons with program code name the runtime they were built for, and need
  0.80.0 or newer. Old ones (built for Qt 6.4) stay installed but their code is not loaded, and the Add-on Manager
  says to install the new version; the install window refuses an add-on of another runtime with that message.
- **The readers are tested against damaged files** (`qb_fuzz_tests`, 11 checks): RTF, Markdown, XML, ZIP, DOCX,
  ODT, Word 97 and .scrivx files damaged in many fixed ways must all come back without a crash, an exception or a
  hang. `tests/fuzz/` has a harness for long runs with libFuzzer.
- **License: Apache License 2.0** instead of the Zero-Clause BSD license. Quillbinder stays free; whoever passes it
  on keeps the license and the new `NOTICE` file, which names the project and its source (no person). Versions up
  to 0.70.0 stay under 0BSD for those who have them. About, Help > Third-Party Licenses, README, THIRD-PARTY.md and
  the AppStream file say so.
- **Help > Third-Party Licenses** lists Qt's own build with the parts Qt builds in (from Qt's attribution files),
  Qt PDF's parts and the ICU that Qt ships; their source is the Qt Project's source release of Qt 6.12.0 in the
  release "Third-party source code".
- **What Qt 6.12 does differently, made right**. A title, synopsis or note with a control character in it (pasted
  from another program) was left out of the project file by Qt 6.12's XML classes without a word; it is kept
  again, with the character taken out as before. Characters such as `*` or `#` in a colored text of an object field
  are written as numbered references again (Qt 6.12 wrote the character itself). A dialog started from the Styles
  panel stays above the panel. A click or right-click on a line that the bottom edge of the view cuts through no
  longer moves the text by a few pixels. Dragging documents from the binder hands other programs their links as
  before. Qt 6.12 plays sound through PipeWire when PipeWire's library is installed; where the library is there
  without its settings (a system that runs PulseAudio only), the first sound ended the program: Media Playback and
  Speech Transcription then use PulseAudio. A video plays its picture also without a sound output again (Qt 6.12
  reads a file in the background, and Play came before it knew the file had a picture).

## 0.70.0 beta (2026-11-10)

A new AppImage and a program update for the AppImages of the same runtime (0.60.0 alpha and later); Scratch Pad 1.2,
Speech Transcription 1.5 and Word 97-2003 Import 1.2. It fixes what two external audits of 2026-11-10 found, speaks
German if you want it, and is the first beta.

- **The save state is always on screen**. The status bar says Saved, Not saved yet, or, in red, that the last save
  failed and when; the title is marked while saves fail. The message of a failed save no longer takes the keyboard
  (before, the next typed key closed it unread), names the documents, offers Save Texts Elsewhere, and comes again
  after five minutes of failing; the program tries again by itself until a save works.
- **Mind maps are saved like texts**. A board that cannot be saved is told, kept in the window, counted when the
  project closes, and written into the emergency copy; before, its changes were dropped without a word. A board file
  that cannot be read opens read-only instead of being replaced by the first change.
- **Backups per project, and restored by the program**. Each project's backups are in a folder of their own, so a
  copy of a project, a restored backup or another project with the same title can no longer delete them (before,
  backups were told apart by the title). File > Back Up > Restore From Backup lists them and opens one in a new
  folder beside the project. A backup shows its progress and can be cancelled, is on the disk before older ones are
  deleted, and no longer carries the lock file with the computer's and the login's name.
- **Fixed: bold inside italics came back with asterisks** in Markdown storage (the standard). Files written so far
  are read right again, and touching formats are written so that they cannot be misread.
- **Fixed: Merge Documents and Split at Selection lost pictures**; they go along with the text now.
- **Fixed: a damaged RTF file could crash the program, or open empty and be replaced by an empty file**; a text file
  saved in another character encoding lost its umlauts. Such files are shown as far as they can be read, read-only,
  with a message, and never written over.
- **Questions before deleting have Cancel as the default**: Enter or a space typed too early no longer empties the
  Trash or deletes a snapshot, a keyword, a collection, a layout, a style or a note; the button names the step.
- **Safer settings**: File > Options opens for all projects; a project's own settings can make saving safer, never
  less safe; the latest automatic save is 30 seconds after a change whatever the delay; a folder with several
  project files (a sync program's conflict copy) opens the one named like the folder and says so.
- **Damaged and hostile files**: a Word 97 file with wrong lengths, a binder nested thousands of levels deep, two
  items with one id, other XML named .qbx, huge counts in a project file or a board are handled without a crash or
  a loss; long formatted paragraphs and long screenplay dialogue are read much faster.
- **Releases are tested and signed apart**: the release workflow runs the full test suite on every release (beside
  the publishing, which does not wait for it), and the signing key is used only in jobs that run no third-party code. An installed program update is
  checked for the project's signature before every start, and a program built as the README says now behaves like
  a release (the test switches are off unless asked for).
- **Deutsch**: File > Options > General > Startup > Language switches the menus, windows and messages to German
  (from the next start). The manual and the welcome project stay in English. Each Alt letter is used once per menu,
  also with entries that are hidden at first or change their name (View > Document or Combined Text).
- **Dark themes get a dark page**: a very dark gray page with white text instead of the light paper (a page color
  you chose yourself stays).

## 0.65.0 alpha (2026-11-10)

A new AppImage and a program update for the AppImages of the same runtime (0.60.0 alpha and later); Scratch Pad 1.1
and Speech Transcription 1.4. It fixes what the external audit of 2026-11-10 found, and signs every package.

- **A save lands as a whole**. When one save writes several files (texts, their pictures, the Objects sheet, the
  project file), they are written beside their places first, listed in a journal, and then put in place together. A
  save cut off in the middle is finished at the next opening, or, if it had not got that far, the project stays as
  it was at the save before; a notice says which. One file that cannot be written stops the whole save, and your
  changes stay in the window for the next try.
- **A list of the documents by title**: *contents-by-title.txt* in the project folder names each document, its place
  in the binder and its files, so that a file can be found outside the program.
- **Signed packages**. The release page's packages and checksum list are signed with the project's key; the program
  installs program updates only with that signature, warns before an add-on without it (Cancel is the default), and
  refuses a package whose signature does not match. On the command line, `--allow-unsigned` installs a package
  without the signature. The manual (section 17) shows how to check a download yourself.
- **Fixed: documents made in Scrivener on another computer were deleted**. A project in Scrivener format that was
  open here while Scrivener added documents to it elsewhere (through a sync folder) lost those documents' folders at
  the next full save. They are kept now, and the binder that names them is not replaced; open the project again to
  see both. Pictures, PDFs, sound and video files and mind maps changed in the *.scriv* elsewhere are no longer
  overwritten with the old version, and they are replaced in one step (before, the file was deleted first, so a
  failed copy left none).
- **Fixed: pictures could be lost without a notice** in projects stored as Markdown: a picture that could not be
  written left the text pointing at a file that was not there. Now the save fails and says so.
- **Fixed: Roll Back could empty a text** when the snapshot could not be read. Now the snapshot is read first, one
  that holds no text replaces the text only after a question, and when the snapshot of the current text was asked
  for but cannot be taken, nothing is rolled back.
- **Fixed: the scratch pad could throw a note away** after a save that failed (switching notes, a new note, another
  notes folder, the end of the program). The note stays open with its text now; at the end its text goes into the
  emergency folder. The same for an edited transcription of Speech Transcription.
- **Backups**: a backup with files missing is named *(incomplete)* and never counts among the backups to keep, so no
  complete backup is deleted for it; the automatic backups when a project opens or closes tell such a backup (and
  the one at opening tells a failure, which it did not).
- **The end by logging out or shutting down**: when the save then fails, the texts not saved go into
  *~/.local/share/Quillbinder/Emergency*, the next start says so, and the program ends with status 1.
- **Closing a project that cannot be saved**: *Keep Open* is the default; *Save Texts Elsewhere…* writes the texts
  not saved into a folder you choose.
- **Also fixed**: Duplicate takes out a copy that is not complete (and says so); an object's old picture is deleted
  only after the object's new file is written; a text read only in part is not saved over; a file changed elsewhere
  with the same size and time is noticed; changing the storage format is all or nothing; a lock that cannot be
  written is told; command exporters check the program's exit status, keep the earlier export when they fail, and
  the window keeps being drawn while they run; a bundle installs all its parts or none; large archives are mapped
  instead of read whole; the view state is written again after a failed write; the test runner ends with status 1
  when a suite failed or was not built.
- **The manual** says that the program does not encrypt projects or backups, and what to use instead.

## 0.64.2 alpha (2026-11-10)

A new AppImage and a program update for the AppImages of the same runtime (0.60.0 alpha and later). It fixes what the
full run of the system test of 0.64.1 found.

- **Fixed: crashes**. Adding a comment or a footnote over text of several formats (seen after a long run of edits and
  undos); choosing an entry of Navigate > Go To, a Quick Reference or a bookmark menu whose document had been deleted
  meanwhile; a speech model download that was cut off and resumed (Speech Transcription 1.3). The same mistake (changing text while walking
  through it) was also taken out of the Scrivener writer, the import options, the objects' texts and the inspector's
  snapshot view.
- **Fixed: a picture cut and pasted was lost when the text was saved**, and letters typed right after a picture
  could take the picture's format and were then saved as pictures, one per letter.
- **Scrivener projects**: a picture in the notes is kept; a synopsis that starts with a space keeps it; a text
  changed in Scrivener (or on another computer) while the project was open here is shown at the next opening (before,
  the old text stayed until the project changed again); a project opened and closed without a change stays as it was
  (the .scrivx was written anew in this program's layout, the label colors, the top folders, the name of the computer
  that wrote it last, and the search index were changed); the leftover of a save that Scrivener did not finish (`content.rtf.XXXXXX`) is no longer imported as a
  document; Scrivener's lock names who has the project open in either form of the file.
- **A project folder or a Scrivener project renamed or moved while open is no longer made anew**. Saving made a new
  folder under the old name that held only the documents changed since; the real project could then not be put back
  under its name. Now saving fails with a message, your changes stay in the program, and the next save after the
  folder is back writes them.
- **The spare copy of a project file opens the project**. Opening `settings/spare-copy.qbx` showed every text empty,
  and a backup made then held only the settings folder.
- **Word counts**: an accent written as a character of its own, letters outside the common Unicode range and numbers
  such as 3,500 or 1.000.000 now count as one word each.
- **Superscript and subscript at the end of a word** act on what you type next (H₂O); before, they changed the word.
- **German quotes while typing**: an apostrophe typed after a word closes an open ‚ with ‘; an apostrophe inside a
  word (geht’s) stays one.
- **Markdown storage**: a paragraph of the style Centered keeps its style (it came back as a paragraph centered by
  hand); a bold or italic run that ends with a space of a format of its own no longer comes back with its asterisks as
  text; a centered paragraph keeps no-break spaces at its ends.
- **Notices that come while you type no longer take the keys**: "Project Opened Elsewhere" and "Other Versions Kept"
  appear beside the window without taking the keyboard, so what you type still goes into the text. Before, the keys
  went into the notice, and a Space could press "Close the Project Here"; the typing after it was lost.
- **A save that fails when you switch to another program is told** (the window saves when it loses the focus); on a
  full disk it failed silently. A Scrivener project's search index that cannot be written fails the save as well.
- **Backspace at the start of a list item** takes the paragraph out of the list with the spacing and indent of its
  style; it kept the list's until the project was opened again.
- **RTF files whose font is named after its italic or bold face** ("SitkaTextItalic") give italic or bold text also
  in projects that take the project's own fonts for imported text.
- **A second start with a project that is open** brings its window to the front (before: the question whether to open
  it anyway, and then a second window on the same project). Another project still opens in a window of its own.
- **The Documents menu** names the label and status lists as the project calls them (Project > Project Settings).
- **File > Options in "This project"**: settings that belong to the program alone (the computer's name and the like)
  changed there are kept (they were dropped).
- **File > Save while Scrivener has the project open** shows the notice once, as the automatic save does, instead of a
  second message.
- **Documents > Snapshots > Take Snapshot** says so when the snapshot cannot be written.
- **Compile through Pandoc (EPUB) without an author** no longer writes an empty author (e-book checkers refuse it).

## 0.64.1 alpha (2026-11-09)

A new AppImage (also as the full version with all add-ons) and a program update for the AppImages of the same runtime
(0.60.0 alpha and later). It fixes what the first run of the system test (crash safety and usability) found, and the
points of an external review of the program's safety (2026-11-09) that are changes of the program.

- **Fixed: a text only looked at could be saved over a newer version from another computer**. Showing a document
  marked it as changed, so a version a sync program had brought in was replaced by this computer's older text (kept
  only as a copy and a snapshot). A document that was not edited here is no longer written over.
- **Fixed: the other version of a changed file could be lost when it could not be kept**. When a file was changed
  elsewhere and here, and its other version could not be kept (a full disk, a folder that cannot be written), this
  computer's version replaced it anyway, and the notice still said it was kept. Now nothing is replaced until the
  other version is kept: saving fails with a message naming the file, and your changes stay unsaved. This holds for
  texts, notes, the project file, and Scrivener projects.
- **Fixed: empty paragraphs in table cells came back as "<p/>" (Markdown storage)**. A cell that began or ended with an
  empty paragraph was read back with the text "<p/>", and the text of a table inside a cell was not saved (in RTF
  storage neither: such a table comes only by pasting from a word processor; its text now stays in the cell). Projects
  saved by 0.62 to 0.64 are read right.
- **Fixed: Split at Selection could lose a table**. Inside a table cell, or at the paragraph right after a table, the
  split took the whole table out of both documents. Inside a table or a picture's caption the split is refused with a
  note now; next to a table it keeps the table.
- **Fixed: a figure cut and pasted lost its picture**. The picture now comes along on the clipboard.
- **Compiling shows its progress**. A long manuscript (with long tables) held the program for half a minute without
  drawing the window, and the program's own watchdog wrote a freeze report. Compile and its preview now show a progress
  window and keep the program responsive while they work.
- **Table cells are no longer justified with the body text**. In the editor, PDF, Word and e-book a narrow cell spread
  its words over its width. An alignment you give a paragraph in a cell stays.
- **Merge also merges the notes**. Before, the notes of the documents after the first one stayed behind in the Trash.
- **Fixed: a crash after Merge Cells, Undo, Redo and Delete Rows**. It happened in a table whose merged cells were empty.
- **Fixed: a save on a full disk could report success and leave a cut-off file**. The last part of a file (and its
  sync to the disk) was written without looking at the result. Now a file replaces the old one only when it was
  written whole; otherwise saving fails with a message and the old file stays. A backup is made only after a
  successful save (before, it went on without the latest changes and reported success).

From the safety review:

- **A project, archive or package from someone else can no longer reach outside its folder**. Names of documents in a
  project file, of files in ZIP archives (templates, backups, add-ons) and of files in Scrivener projects are checked;
  before, a made-up name could make Empty Trash delete a folder beside the project, or an archive write a file
  anywhere. Damaged archive entries (a wrong checksum, cut off) are no longer read as text or written as empty files.
- **Links that lead out of the program are checked**. Web and mail links open at once; a link to a file, a folder, a
  network share or another program is shown in full and opened only when you agree; links to programs, scripts and
  shortcuts are never opened. This holds for texts, mind maps, bookmarks and imported web pages.
- **Scrivener's lock is looked at before every save into a Scrivener project**. When Scrivener opens the project while
  it is open here, nothing more is written into it (both programs used to write); a notice says so, and the changes
  stay in the working copy until Scrivener has closed it.
- **Settings a project may bring are limited**. Programs to start, folders, converters and your contact data are never
  taken from a project, and from an options file only after you have seen them and agreed.
- **A name of your own for this computer in shared folders**. Set it in File > Options > General > Saving; your user
  name is then left out of lock files. Locks tell two computers of the same name apart.
- **Speech models are checked before they are read**. The checksum (now SHA-256) comes first; a model from the
  official source must be the known one, and other addresses must be https.
- **A warning before packages with program code**. Also versioned libraries, plugins and templates count. Damaged Word 97 files and very deeply nested Word or XML files are refused instead of filling the memory or
  crashing. The folder for the links between running programs in /tmp is checked.
- **Safer builds**. The switches of the tests (plugins from other folders, skipped checks) are only in test builds;
  the AppImage tools are pinned to tagged releases with checksums; the release workflow keeps its write permission in
  a publishing job of its own.
- **A privacy section in the manual**. "Privacy: what leaves your computer" (chapter 1).

## 0.64.0 alpha (2026-11-08)

A new AppImage (also as the full version with all add-ons) and a program update for the AppImages of the same runtime
(0.60.0 alpha and later).

- **Safer with a project in a synced folder on several computers**. It works on the level of files, whatever the sync
  program is (OneDrive, Dropbox, Nextcloud, Syncthing ...). The lock in the project names the computer and is renewed
  every five minutes, so another computer shows who has the project open, since when, and its last sign of life (a lock
  not renewed for 15 minutes counts as left over). A notice comes when another computer opens the project while it is
  open here. A file changed elsewhere and here is no longer overwritten silently: the other version is kept as a
  snapshot and as a copy in settings/conflicts (also the project file, and in Scrivener format). Opening and closing
  without a change changes no file (the window layout is kept per computer in settings/ui-<computer>.json; a .scriv is
  no longer rewritten at every close). Texts whose files have not arrived yet are shown read-only and never written.
- **Speech Transcription can record now and write the text later**. With Record only (the add-on Speech Transcription 1.1; File > Options > Speech;
  the standard on Android later) the microphone only records into a sound file beside the transcriptions, which uses little
  power; a Transcribe button in the recording's line (and Transcribe All) writes its text later, on this computer or
  another one. Optionally the recording is deleted once its text is there.
- **Saved more often while you type on**. The first short break once 5 seconds have passed saves (before: only a pause
  of the autosave delay, else every 30 seconds), and 30 seconds stay the latest.
- **Fixed: the spelling menu on words with an apostrophe**. A right-click on "Kayel's" (straight apostrophe) near its
  end gave the long menu without suggestions, and on "D'Nersun" the menu offered to add "Nersun". The menu now takes
  the word the underline marks, at any place of it.
- **Fixed: the corkboard's card size could get lost**. Only a drag with the mouse was saved; a change with the keys,
  the wheel or a click beside the handle went when the settings were read again (for example when a project opened).

## 0.63.0 alpha (2026-11-08)

A new AppImage (also as the full version with all add-ons) and a program update for the AppImages of the same runtime
(0.60.0 alpha and later). It brings together 0.60.5, 0.61.0, 0.61.1 and 0.62.0, which were not published on their
own (their sections in this file say more), and a few fixes:

- **Speech Transcription** (a new add-on, part of the full version): speech becomes text on this computer, with a
  model downloaded once under File > Options > Speech (0.61.0).
- **Tables and pictures**: Insert > Table and Format > Table with ready-made designs or your own, pictures beside the
  text with captions and descriptions, also in compile to PDF, Word, e-book and Markdown (0.62.0).
- **Settings per project**: File > Options > Settings for: This project, and a window at the first start for backups
  and settings (0.62.0).
- **Quote marks of a whole project converted** (File > Options > Corrections) and **Project > Delete Old Snapshots**
  (0.61.1).
- **Undo and Redo** on the main toolbar, and the crash of Split at Selection from the text's right-click menu fixed
  (0.61.0).
- **Large projects in Scrivener format** save much faster while you write (0.60.5).

- **German spelling knows words with umlauts and ß**: the German dictionaries (of LibreOffice and Ubuntu) are
  stored in ISO 8859-1, and the program handed every word to them in UTF-8, so "Straße", "Jänner" or "Wörterbuch"
  were always underlined. Words now go to each dictionary in its own encoding.
- **An update package with add-ons installs on a program that already has its version**: inside a bundle, a
  program update the program already contains is left out (and the bundle says so); the add-ons go in. Before,
  the whole bundle was refused.
- **Add-ons for another system are refused**: a package names the system its libraries were built for; a Windows
  add-on cannot be installed on Linux (and the other way round).

## 0.62.0 alpha (2026-11-08)

A program update for the AppImage 0.61.0 alpha.

- **Settings per project**: File > Options has **Settings for:** at the top, **All projects (the program)** or
  **This project**. What you change for "This project" is kept with the project (in *settings/options.ini* of its
  folder) and applies while it is open; everything it does not set comes from the program's settings. **Use Program
  Settings** removes a project's own settings again; the line below the choice says how many it has and lists them.
  A few settings belong to the program only: the recent projects, reopening the last project, the desktop's file
  dialogs, window places and layouts, crash report times, and the look for new projects (a project's look has been
  its own since 0.40.2).
- **Which settings come first** (File > Options > General > Startup, for all projects): a project's own settings
  first (the standard), or the program's settings always; then projects' own settings are not used (they stay in the
  projects for later), and every project also shows the look for new projects.
- **Project > Save Project Settings to File** has a new section, **Options of This Project**.
- **First start**: a small window asks two things before the first project: backups (on closing, how many, the
  folder) and which settings come first. Everything is filled in; OK keeps it.
- **Tables**: **Insert > Table…** (rows, columns and the design at once), and **Format > Table** (also in the
  right-click menu inside a table): insert rows above or below and columns left or right, delete rows, columns or
  the table, merge and split cells, a cell's own color, and **Table Properties…**. Tab goes to the next cell (in
  the last cell it adds a row), Shift+Tab back.
- **Table design**: five ready-made designs (Grid, Grid with a header row, Striped rows, Lines between the rows, No
  lines) or your own: lines (width, color, only between the rows), space inside and between cells, header rows
  (with a color; printed again on every page), a color for every other row, the width (a share of the text or as
  wide as the contents), column widths, the alignment, and the place: on its own lines, or left or right with the
  text flowing beside it.
- **Pictures** (**Format > Picture…** or right-click a picture): the width, the position (in the line, left or
  right with the text flowing around it, or centered on its own lines), a **caption** under the picture, and a
  **description** of what it shows. The description is not shown in the text; it goes along as the alt text of
  e-books, Word files and Markdown, and screen readers read it.
- Tables, pictures beside the text, captions and descriptions are kept in both storage formats (Markdown and RTF)
  and in Scrivener format (as RTF tables; Scrivener shows pictures beside the text as ordinary paragraphs). Tables
  from RTF files of other programs become tables.
- **Compile**: tables and figures in PDF (a long table goes on over pages with its header rows on each), Word (real
  tables; pictures beside the text in a borderless table with their caption, so Word and LibreOffice let the text
  flow around them), e-books (tables, figures with captions and alt text), and Markdown (pipe tables, alt text).
  Plugins for other output formats see tables and figures through the exporter interface 1.4 (plugins built for 1.3
  must be rebuilt; the Markdown exporter that comes with the program is).

## 0.61.1 alpha (2026-11-08)

A program update for the AppImage 0.61.0 alpha.

- **Convert quote marks** (File > Options > Corrections, under Smart quotes): **Convert to Typographic Quotes…**
  turns the straight " and ' already in the project into the marks of the chosen style (“…” ‘…’, „…“ ‚…‘, and the
  others), opening or closing by their neighbors, an apostrophe inside a word or before a number as ’;
  **Convert to Straight Quotes…** goes back. A window like Project Replace chooses where (titles, text, notes,
  synopses, custom metadata, comments and footnotes) and the scope (the whole project or the selected documents),
  counts first, and takes a snapshot of every text it changes. Formatting stays.
- **Project > Delete Old Snapshots…** (below Empty Trash): deletes every snapshot of the project, also of
  documents in the Trash, so the project gets smaller; the texts stay as they are. It asks first, with the number
  of snapshots and their size.
- **Fixed, Scrivener format**: a snapshot deleted in the program (in the inspector) stayed in the *.scriv*: Scrivener
  still showed it, and it came back whenever the program read the *.scriv* again. Snapshots deleted here now leave the *.scriv* with the next
  full save (switching to another program, File > Save, closing); a snapshot only Scrivener has stays.
- Speech Transcription, tested with real models for the first time (Small and Medium from Hugging Face): German
  speech is written in English; German words inside English sentences come out right when they are listed under
  Hint words (the manual says so now). The checksums of Small and Medium match the files on Hugging Face.

## 0.61.0 alpha (2026-11-08)

- **Speech Transcription** (a new add-on, "Speech Transcription" 1.0): speech becomes text on this computer only.
  A microphone button on the main toolbar starts and stops a recording; the arrow beside it chooses the
  microphone and opens the options. What you say is written into a new transcription file (named by date and
  time, in *Documents/Quillbinder Transcriptions*), shown in the new inspector tab **Transcriptions** (the first
  button of the inspector's row), where it can be edited, renamed, copied and deleted. A pause ends a piece; its
  text follows a few seconds later; a long pause starts a new paragraph. Recognition by whisper.cpp 1.9.5.
- **File > Options > Speech** (a section of its own): the model (Small, Medium (recommended), Large v3, Large v3
  Turbo, each with a note on download size, memory, speed and translation), downloaded once when you choose it
  from whisper.cpp's model files on Hugging Face, or from another address, or a file you downloaded yourself. Before
  a model is used it is checked (its checksum, the model's description in the file, a test transcription of a
  short recording); only then does it replace the model in use, which is deleted. An interrupted download goes on
  where it stopped. Further: the processor or a graphics card (Vulkan; offered when one with enough memory is
  found), processor threads, fast or careful, a speed test, the main language (the text) and a fallback language
  (also spoken: with English as the main language it is written in English), hint words, the microphone,
  sensitivity, the pauses for a piece and a paragraph, the folder, two advanced switches.
- **Undo and Redo** on the main toolbar: in the standard set after Move to Trash (a toolbar you arranged
  yourself keeps what it has).
- **Crash fixed: Split at Selection from the text's right-click menu** (reported on 2026-11-08 with 0.60.5): the
  split loads the new document into the editor, which replaced the editor while its right-click menu was still
  handling the click; the program then used memory that was already given back and crashed. The menu now stays
  until its click is handled. The same could happen with Split with Selection as Title and Open Link of that menu.
  Ctrl+K and the Documents menu were not affected.
- For add-ons: the **tool interface 1.1** lets a writing tool add a tab to the inspector, a button with a menu
  to the main toolbar (a toolbar arranged by the writer gets it once) and a section of its own to File > Options.
  It adds to 1.0 without changing it: the tools built for 1.0 keep working, and an update of such an add-on no
  longer needs a program update.

## 0.60.5 alpha (2026-11-07)

- **No more short stops while writing in a large project in Scrivener format**: the automatic save now writes
  only the texts, notes and synopses you changed into the *.scriv* (and the binder when it changed). Before, every
  automatic save went through the whole project: it read and checked every file on both sides, rewrote Scrivener's
  search index with every word and rewrote the binder file. With a test project of 500,000 words (1,000
  documents) an automatic save took about 250 to 350 ms on the test machine and now takes about 30 to 40 ms.
  Scrivener's search index, snapshots, pictures and the rest follow when you switch to another program, use
  File > Save, or close the project, as before.
- **Counts no longer rewrite the project file every few seconds**: new word counts while you type are written
  with the next full save, or a minute later at the latest (the texts themselves are saved as before, a few
  seconds after you stop). This helps large projects in either format.
- **Opening a dialog or panel saves quickly**: when another window of Quillbinder takes over (a dialog, a panel,
  the composition window), the quick save of the automatic save runs; the full save is kept for switching to
  another program and for closing.
- **Menus without animation**: Qt's own animations of menus, lists and tooltips stay off, whatever the desktop
  theme asks for. (A check with Qt 6.4 and the KDE settings found them off already; now they are off for
  certain.)
- **Spelling suggestions** are remembered for the session: a second right-click on the same misspelled word
  opens the menu at once (the first search of the dictionary took about 50 ms on the test machine).
- For checking speed: started with `QB_TIMING=1`, the program writes the times of each save step and of the
  right-click menu to its error output.

## 0.60.4 alpha (2026-11-07)

- **Spacing and indents in the format bar**: four new buttons after the list buttons set the space before and
  the space after the paragraph (0, 3, 6, 9, 12 or 18 pt, or any value), the indent of the first line, and the
  indent of the whole paragraph (in cm or inches, as the ruler). They act on the paragraph at the cursor or all
  selected paragraphs, and Edit > Undo takes the change back. A format bar you arranged yourself gets them once.
- Brings the changes of 0.60.3 (below) to everyone: it is the first public version with them.
- The README says that Quillbinder works under Kubuntu 26.04 LTS with Wayland (the sentence added on GitHub on
  2026-11-06 is now part of the source, so publishing keeps it).

## 0.60.3 alpha (2026-11-07)

- **The binder counts a folder in full**: resting the mouse on a folder (or a text with subdocuments) shows the
  words of the folder and everything inside it, as the editor's footer does, and the folder's own words when it
  has text of its own. Before, it showed only the folder's own text (often "0 words").
- **The folder total in the middle of the footer, with details**: on the corkboard and in the outliner the total
  sits in the middle of the footer; a click shows words, characters, characters without spaces, paragraphs and
  pages for the whole folder, as the count of a text does. The same for the total beside a folder shown as text.
- **Inspector Background**: the color "Lists and text fields" (File > Options > Appearance > General Interface)
  is now **Inspector Background** and colors only the inspector's lists and text fields (bookmarks, notes, the
  title, the metadata fields ...), the same in every project; the text there turns light or dark to stay
  readable. The inspector's right-click Appearance menu offers it as "Inspector Background (All Projects)". A
  color you chose for "Lists and text fields" is kept as the Inspector Background. The inspector's own color
  "Background" (Inspector & Notes) is now called **Inspector Strips**: the tab row, the headings and the
  Label/Status row.
- **Find the document you are writing in**: a double-click on the space beside the text (the paper beside the
  column or the area around it) selects that document in the binder, opens the folders above it, and lets its row
  blink twice in orange; the editor keeps what it shows. Navigate > Reveal in Binder (Ctrl+Shift+R) blinks too.
- **Italics from Scrivener in the right font**: Scrivener for Windows names the font of italic text by its face
  (for example "SitkaTextItalic"), which is no font family on Linux, so the text was shown in the desktop's
  fallback font (Noto Sans). Such names (also "Georgia-BoldItalic", "TimesNewRomanPS-ItalicMT", "Sitka Text
  Italic") are now shown in the family they stand for, in italic or bold as the name says; the text keeps the
  name, so Scrivener gets back what it wrote.
- **Toolbars and options in the color of the menus**: the fields on the toolbars (quick search, style, font,
  size, line spacing), the ruler, and the row of pages and the lists on the left of File > Options have the color
  of the window and the menus, no longer the color of lists.

## 0.60.2 alpha (2026-11-06)

- **Three ways to paste in the right-click menu** of the text: Paste, Paste and Match Style, and Paste with
  Formatting, one below the other, with their shortcuts (on a misspelled word under More Commands).

## 0.60.1 alpha (2026-11-06)

- **Paste with Formatting** (Edit menu, **Ctrl+Alt+Shift+V**): text from a word processor or a web page keeps
  its fonts, sizes, colors and the alignment, indents and spacing of its paragraphs, also in Markdown projects.
  Paste itself is unchanged.
- **Show Invisibles** (View > Text Editing, and a ¶ button on the toolbar): dots for spaces, arrows for tabs, a
  circle for a no-break space, a bent arrow for a line break and ¶ at the end of each paragraph, drawn on the
  screen only. If you changed the toolbar yourself, add the button under View > Toolbars > Customize Toolbars.
- **The total of a folder**: the footer of a folder (text, corkboard or outliner) and the inspector's metadata
  show the words of the folder and everything inside it (characters when the footer counts characters).
- **Styles take over the paragraphs**: a paragraph style chosen for a selection over several paragraphs styles
  all of them in full, and fonts, sizes, colors, letter spacing and capitals set by hand in them give way to the
  style; bold, italics, underlining, strikethrough, highlights and character styles stay. A selection that ends
  right at the start of a paragraph leaves that paragraph alone.

## 0.60.0 alpha (2026-10-31)

- **Audio and video in one package**: *media-all-formats.qbpkg* installs the add-ons Media Playback and Extra
  Video and Audio Formats in one step, with all the formats the two add-ons play (H.264, H.265, AAC and the other
  formats licensed under patents included). *media-playback.qbpkg* alone still plays the free formats, and the
  extra formats can still be switched off or removed on their own in Help > Add-on Manager.
- **The full version**: *Quillbinder-0.60.0-full-alpha-x86_64.AppImage* is the program with all add-ons and the
  libraries they need inside (the commonly used audio and video formats, themes, templates, the scriptwriting pack, the
  writing tools, the importers). They are installed at its first start; one download, nothing else to install.
- **One version for everyone**: 0.60.0 alpha brings the public 0.50.0 alpha and the changes of 0.40.1, 0.40.2
  and 0.40.3 together (listed below): fonts with several designs, the format buttons of a selection, a look per
  project, project settings files, titles from the first line, the style list, the alignment of the main text
  and more.
- **About** names FFmpeg, which the audio and video add-ons use, with its license (GNU LGPL 2.1 or later) and
  where its source code is.
- **Character and place sheets** of the fiction templates have new fields of their own (In one sentence, Part
  in the plot, Voice, Goal, Fear, Secret, Ties to other characters ...; for places: Where it is, By day and by
  night, Who comes here ...). Existing projects keep their sheets.

## 0.50.0 alpha (2026-10-25)

- **Download from the project page**: the AppImage, the program update and every add-on are
  published on the Releases page of the project's repository, built by GitHub
  from the public source code. Each release lists SHA-256 checksums of its files.
- **Source code of every library**: the release "Third-party source code" holds the source of each
  library the AppImage and the add-ons carry, exactly in the version used (Ubuntu's source packages,
  FFmpeg with the build scripts, the AppImage runtime). Help > Third-Party Licenses names the file for
  each library, and the add-ons' license notes do the same.
- **Media Playback 1.1** decodes free formats only (MP3, Ogg Vorbis, Opus, FLAC, WAV, WebM with VP8 or
  VP9, Theora ...). H.264, H.265, AAC, AC-3 and the other formats licensed under patents are in the
  new, separate add-on **Extra Video and Audio Formats**; while it is installed and switched on, Media
  Playback uses it. Without it, a file in such a format says which formats it uses and what to do.
- An AppStream description for software centers; the Add-on Manager names the contents of the new
  add-on.
## 0.40.3 alpha (2026-10-29)

- **Documents without a name are named by their first line**: a document called "Untitled" (or with no
  title) takes the first line of its text that has words as its title (at most 60 characters, cut at a
  word; inline annotations, inline footnotes and pictures left out). The title follows the first line
  until you rename the document by hand; a document without text is "Untitled" again. Existing
  "Untitled" documents with text get their name when the project opens. It is a real title: saved,
  compiled, written to Scrivener projects.
- **The format bar shows what you see**: Bold, Italic, Underline, Strikethrough, superscript and
  subscript are on when all of the selection looks so, also when a style makes it so (a heading's bold);
  Bold on a bold heading makes it not bold, and it stays so when saved, also in Markdown projects. Font,
  size, alignment, line spacing and style show the whole selection; when they differ within it, the
  field stays empty and no alignment button is on.
- **Styles panel**: **No Style** has **Redefine From Selection** (right-click, or the Redefine button):
  the formatting at the cursor becomes the project's main text style (font, size, line spacing, first
  line indent, space after paragraphs, alignment), and all text without a style in the project follows.
  The dialogs and questions the panel starts now open above the panel.
- **Alignment of the main text**: File > Options > Editing > Formatting and Project > Project Settings >
  Formatting have four buttons (left, centered, right, justified) instead of "Justify text".
- **Style list** of the format bar and of the right-click menus: **Customize Style List** (in the list,
  in Format > Style) chooses which entries show and in what order; with more than ten the list scrolls;
  New Style From Selection, Redefine Style From Selection, Styles Panel and Customize Style List stay
  below it. The choice belongs to the project and goes into project settings files.
- **Spelling**: on a misspelled word the right-click menu is short: the suggestions, Add, Ignore and
  **More Commands** with everything else, so the suggestions open next to the mouse.

## 0.40.2 alpha (2026-10-26)

- **Every project keeps its own look**: the theme, colors and fonts of the binder, editor, corkboard,
  outliner, inspector and composition mode, the text column width and the new border beside the text
  belong to the project. A change in one project, a new standard for new projects, or an update of the
  program no longer changes how an existing project looks. **File > Options > Appearance** shows and
  changes the look of **This project** or of **New projects**; menus and panels stay one look for the
  whole program. A project opened first by 0.40.2 keeps the look it had.
- **Border beside the text**: paper in the paper's color left and right of the text, 10 px for new
  projects (0 to 100 px, File > Options > Appearance > Main Editor), outside the text column, so lines
  break as before. Older projects keep theirs without a border until it is set.
- **Right-click menus** offer the colors, fonts and sizes of what is under the mouse (Appearance): the
  text and the paper beside it, the header bar, the binder, the corkboard, the outliner, the inspector
  and its notes, composition mode.
- **Project > Save Project Settings to File / Load Project Settings from File** (*.qbsettings*): labels,
  status, section types, custom metadata, keywords, formatting, styles, script format, targets, compile
  settings, page and print setup, the look, the object templates. When the project's settings differ
  from the file, a window asks per section: take the file's, keep the project's, or the Quillbinder
  default. What documents use is never lost; the settings as they were are kept to go back.
- **Style changes reach all text of the style**: font, size, bold, italics, color, letter spacing,
  capitals, alignment, indents, spacing, line spacing and tab stops of a changed style are taken over
  also by text of the style that had its own values (set by hand or from an import). A snapshot of each
  changed document is made first.
- Corrected: a click or a right-click in the editor made the text jump (near the top or bottom edge of
  the view; a right-click beside the text even jumped to the end of the document and lost the
  selection). The view now stays where it is; a click beside the text puts the cursor on the line beside
  the click.

## 0.40.1 alpha (2026-10-25)

- **Fonts with several designs** (the optical sizes of Sitka on Windows 11 and similar fonts) are listed
  as one font per design ("Sitka Small", "Sitka Text" ...). Bold and italic now stay within the chosen
  design; before, bold text could come out in another design and look smaller and narrower. The font
  files are not changed.
- Corrected: the Bold, Italic, Underline and Strikethrough buttons did not show the state of the
  selected text when it was selected from right to left (they showed the character in front of the
  selection). They are now on when all of the selection has the format; Bold on a partly bold selection
  makes all of it bold. The same in the text fields of objects and in the fonts window.

## 0.40.0 alpha (2026-10-25)

- **Welcome project**: the first start opens a short project that shows, one document each, what
  Quillbinder has beyond the usual: the icon bar, a zoom for every part of the window, mind maps (with an
  example board), object sheets (with linked example objects), the comments of a whole folder, Compare
  and Transfer, Scrivener projects, themes, updates and add-ons, and what protects your text.
  **Help > Welcome Project** opens it again or makes a fresh copy.
- **Help > Quick Start Guide**: where the important settings are, and what the program can do.
- **Help > Third-Party Licenses** (also a button of Help > About): every part of other projects the
  AppImage carries, with its full license text and where its source code is; the texts are also in the
  AppImage (`usr/share/doc/quillbinder/licenses`).
- **License**: Quillbinder is now under the Zero-Clause BSD license (0BSD).
- The view of several documents as one text is called **Combined Text**. Settings and theme files of
  older versions keep working.
- The interactive tutorial of 0.15 to 0.30 was removed.
- Corrected: Page Down and Page Up in the combined text skipped the documents between the first and
  the last one when the cursor landed between two documents (seen in 0.30.0 alpha).
- Corrected: a crash, now and then, when a project was closed ("corrupted double-linked list"), if a
  picture or PDF file of a project closed earlier had been shown in the same window.

## 0.30.0 alpha (2026-10-25)

- The AppImage with the program of 0.21.2; a version label ("alpha") in Help > About and the file name.

## 0.21.x (2026-10-22 to 2026-10-24)

- File > Options and the compile window reorganized, with options for the program's own features
  (Extras); Ctrl + mouse wheel zooms the part under the mouse everywhere; a zoom per part of the
  inspector; mind map colors, shapes, icons on notes and lines; places kept with the project; Scrivener
  headings, paragraph styles and project bookmarks on import; the standard formatting of new projects.

## 0.20.0 (2026-10-19)

- The Objects sheet: characters, locations, environments, points of interest, weather and categories of
  your own, with links; data-safety fixes (control characters, unreadable files, backups over 4 GiB,
  autosave while typing, the spare copy of the project file).

## 0.17.0 to 0.19.0 (2026-10-19)

- Document icons, the icon bar, Icon from Text, installing fonts, color buttons, the comments of a
  folder, mind maps; Compare and Transfer with Word and LibreOffice files; Mind Map Help; crash and
  freeze reports, Help > System Information.

## 0.14.0 to 0.16.0 (2026-10-17 to 2026-10-18)

- Projects in Scrivener 3 format; printing; the AppImage 0.16.0.

## 0.2.0 to 0.13.0 (2026-10-03 to 2026-10-16)

- Updates and add-ons as packages, fonts and paragraphs, styles, themes, menus, toolbars, shortcuts and
  layouts, page view and copyholders, drag and drop and Quick Reference, search and replace,
  scriptwriting, writing tools (Name Generator, Scratch Pad, Text Statistics), audio and video, import of
  old Word files and web pages.

## 0.1.0 (2026-09-26)

- Projects and templates, binder, editor, corkboard, outliner, inspector, snapshots, targets, composition
  mode, Markdown or RTF storage, compile to DOCX (PDF, ODT and EPUB through plugins), Scrivener import,
  backups.
