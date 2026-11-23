# Saving, backups, sync safety, the audits' fixes, crash reports

Moved out of `CLAUDE.md` on 2026-11-15 (the index there lists every file); sections newest first. Search with grep and read only the section a job needs. A job adds its notes here as a new section at the top.

## 0.93.0 beta (2026-11-23): the emergency copy counted, a folder's sync, the journal that cannot be set aside, the backup note

- **Item 38: backups and saving, what was not done** (suite `v0930c`).
  **(a) Waiting backups in Restore From Backup**: `backup::waitingBackupsFor(p, dir)` = the backups of this project in
  the work folder of the backup folder (and those of the project's own folder of 0.70 to 0.90, as `backupProject` puts
  them in place), picked by the same rule as `backupsFor` (`mineOf`); `MainWindow::restoreBackup` merges them with the
  folder's by time and adds ", waiting (not yet in the backup folder)" to the state. A restore never moves them.
  **(c) Move Old Backups** (`File > Back Up`, action `moveOldBackups`, `MainWindow::moveOldBackups`,
  `backup::moveFormerBackups` -> `MoveReport{from, moved, left, folderRemoved}`): asks (box `moveOldBackupsAsk`), then
  per file a hard link into the backup folder where the drive allows (never over a name: EEXIST), else `copyInto`
  (`NewOnly`, synced, compared), then the original is removed; a name that is there stays in the old folder and is told
  ("<name>: <reason>"); the old folder is removed with `rmdir` (only when empty). Report box `moveOldBackupsReport`.
  The action is switched on when the Back Up menu opens (`aboutToShow`: project, a backup folder, and
  `formerBackupsFor` not empty), off without a project (`updateActions`). The moved files have no ID in their names, so
  the clean-up still never counts or deletes them. The former folders can still exist: the 0.91.0 notes say they were
  left as they were. The work folder of the old folder (backups that waited for it) is not touched by this command; the
  next backup puts them in place.
  **(d) Copies whose result was not checked**: the speech add-on's `useLocalModel` (a part of a model went to the check
  after a failed copy: now the copy must have the original's size, else the part is removed and the failure told; the
  string is in the add-on's context in the .ts); the application icon of `integrateDesktop` (told in the message when
  it fails; the quiet start-up call stays quiet); `DragDrop::copyTreeTo` (the pictures of a web page copied for another
  program) and `MediaView` (the pictures beside a web page shown in the browser) stay unchecked on purpose, with a note
  in the code: originals are never touched and a failure is a gap in a copy that has no place to say so.
  **(b) The temporary names that Quillbinder writes where a sync program may see them** (Linux; the same code on the
  other systems):

  | Name | Where | Lives | A crash can leave it | Removed by |
  |---|---|---|---|---|
  | `<file>.qbstage` | beside each target in the project folder (batch save) | from staging to the rename, with `settings/journal.json` | yes | `savebatch::recover` at the opening (finished with the journal, else dropped); `Batch::stage` sets a foreign one aside |
  | `<file>.qbstage` of an earlier save, in the way of a new save | `settings/conflicts/<file> (temporary file of an earlier save <time>)` | for good | made on purpose by `Batch::stage` (before 0.93.0 beside the target as `.qbstage.kept-<time>`) | nothing, on purpose: it may hold the only copy of unsaved text; the save tells it once (`conflictKept`, like the other kept versions) |
  | QSaveFile `<file>.XXXXXX` (also `safesave::copyFile`, settings, index, snapshots, `.scriv` files) | beside the target | milliseconds | yes | **new**: `savebatch::removeSaveLeftovers` at `Project::open` and `ScrivenerFormat::open` (a known file type, six characters, 10 minutes old); before, only the reader of a .scriv ignored `content.*.XXXXXX` |
  | `<name>.zip.part` of a backup | the program's work folder, not the backup folder | the packing | yes | `sweepWork` (an hour old) at the next backup into that folder (0.90.3) |
  | `<export>.docx.part` | beside the export's target | the export | yes | **new**: replaced in one step (`savebatch::replace`; before, the old file was deleted first and the `.part` stayed when the rename failed), removed when it fails, an hour-old one beside the target goes at the next export |
  | `<name>.txt.part` | the speech add-on's recordings folder (a folder of the writer's choice) | the transcription | yes | **new**: `speech::removeLeftoverParts` (an hour old, not while it transcribes) when the panel refreshes; before, only a new run of the same recording overwrote it |
  | `<file>.qbtmp` | beside a file of Save Options to File | the write | yes | the next save to the same file only (left: rare, a folder the writer chose) |
  | `<name>.old` (a Scrivener working copy moved aside) | the program's data folder, not the project | a re-read | yes | the next read of that project (`ScrivenerFormat::open`) |
  | `.update-part`, `.before-update` | beside the AppImage | the update | yes | the update's next run; not a project folder |
  | model `<file>.part` | the add-on's data folder | the download | yes | the next download; not a project folder |

- **The emergency copy** (`Project::makeEmergencyCopy`, returns an `EmergencyReport` of `ProjectTypes.h`: `dir`, `reason`,
  `tally` = `CopyTally` with `wanted`, `written`, `missing` "name: reason"). `writeEmergencyCopy(parent, &error)` stays for
  the old callers (the folder; `error` = the reason, or the incomplete text). `DocumentStore::writeUnsaved`,
  `mindmaps::writeUnsaved` (`Project::emergencyExtra` now takes the tally) and the new `objects::Store::writeUnsaved`
  (changed objects and the schema as `objects/...`) all count into the tally. The copy is made when
  `Project::needsEmergencyCopy()` (texts, boards, **binder** `m_indexDirty`, **objects**); it always holds the project file
  as `Project::indexBytes()` makes it (split out of `saveIndex`; the same bytes a save would write). The folder is made
  with `mkdir` (atomic): "<title> <time>", then " (2)", " (3)" ...; nothing is ever reused. README: the way back in two
  sentences, headed "THIS COPY IS INCOMPLETE: n of m" when files are missing. `Project::emergencyText(report)` is the
  text of all places (`MainWindow::tellEmergencyCopy` for Save Texts Elsewhere and Close Project; the notice of
  `saveForTermination` through `writeEmergencyNotice(what, report)`). An incomplete copy shows a warning box with
  "Copy Incomplete" in its title. Android's own save (`src/android/MobileStart.cpp`) was not changed.
  Review fixes: the project file in the copy is `<name>.<suffix>.copy` (never opens as a project, takes no backup ID);
  `EmergencyReport::nothingToWrite` (no failure text); the README of a Scrivener project gives no way back;
  `tellBackupCleanup(notes, modal)` is non-modal except at closing.
  Test switch: `QB_TEST_WRITE_FAILS` ("number|text|part of the path") now also works in `safesave::writeFile`.
  Boards (and texts) with the same title get " (2)", " (3)" in their file names.
- **A folder's sync** (`savebatch::syncFolder` now returns false with errno): EINVAL, ENOTSUP, EOPNOTSUPP, ENOSYS and a
  folder that cannot be opened are no error (as for files); any other error (EIO ...) after the files are in place:
  `Batch::commit` keeps the journal (`m_journalWritten` stays true, the temporary files stay for `recover`), does not run
  the `after` steps (the changes stay marked as unsaved), and returns false with "The drive reported an error ...".
  Without a journal (one file) the save is told as failed the same way. The syncs before the journal is written and
  the one after it is removed are still not checked.
  **Review fixes (0.93.0):** `put(..., done, placed)`: `placed` steps (stamps, `e.read`, index mark and sha,
  `savedImages`) run once every rename is done, also when the sync then fails (`Batch::afterPlaced`, `placed()`); `done`
  steps (dirty flags, old-format files) only after a good sync. `savebatch` keeps the set of folders whose checked sync
  failed (`g_unsynced`): every commit (also an empty one) and `recover()` sync it again; success clears it, `recover()`
  keeps the journal while it fails (`Recovery::syncFailed`). The syncs before anything is replaced are checked (clean
  failure, `drop()`). `probeFolderSync` (at `Project::open`) remembers a device whose folder sync errs as "cannot sync";
  EBADF, EACCES, EPERM also count as "cannot sync". Storage conversion keeps the new format when `placed()`.
  `keepUnreadable` puts the journal back when a temporary file could not be moved; `Batch::stage` moves a foreign
  `.qbstage` into `settings/conflicts` (`takeKeptAside`, told once); `failure::Kind::JournalInTheWay`. `recover()` reports no "lost" file whose
  temporary file is gone and whose target was replaced later. Known: an opening still deletes temporary files when no
  journal is there (a save that never began to land). Shim: `dirsync` acts only on folders without temporary files or
  journal (after the renames), `dirsync-early` on the others, `QB_SHIM_ARM=<file>` switches the fault on once the file
  exists. `savebatch::syncDirChecked` is the public form. The fault library
  has the modes `dirsync` (EIO) and `dirsync-notsup` (ENOTSUP), for folders only.
- **An unreadable journal that cannot be set aside** (`Recovery::keptFailed` with the journal still there):
  `Project::finishInterruptedSave` returns false with a message that names `settings/journal.json`; no journal of a new
  save is written over it. The notice of the recovery is told once (`m_keptFailedTold`); every save tries to set the
  journal aside again.
- **The backup note** (`backup::isDue`, `clearDue`; `takeDue` = both; `markDue` returns its write's result). The
  opening (`MainWindow::openProject`) looks with `isDue` and clears the note only when its backup was made
  (`ok`) or waits in the work folder (`waiting`); with "back up at closing" switched off the note is cleared at once.
  **Clean-up**: `backupProject(..., &waiting, &cleanupNotes)`; `removeBackupFile` tells a backup that cannot be deleted,
  once per file and session ("<file>: <reason>"); `MainWindow::tellBackupCleanup`. Test switch
  `QB_TEST_FAIL_REMOVE` (a part of the file's name). **Restore** (`MainWindow::restoreBackupFile`): the forms are
  restored / "restored INCOMPLETELY" (warning icon, `backup::restore`'s text with the files) / not restored; a title that
  could not be saved ("could not be marked") is added; `lastRestoreText()` for the checks. Suite `v0930` (child runs
  with the fault library: `--child-save`).

- **Second review (0.93.0):** `writeEntry` and `saveIndex` clear the dirty marks of their "holds exactly this already" path in
  `Batch::after` (only after a good commit and its sync; at once without a batch). `Project::create` probes the folder sync as
  `open` does; the probe's device set has a mutex (a backup's worker thread reads it). `keepUnreadable` with a file that
  stayed: the journal goes back, the new folder in `settings/conflicts` is removed when empty, otherwise named in the notice
  with what stayed (test switch `QB_TEST_FAIL_MOVE`, a part of the source path). `EmergencyReport::scrivenerWaits`; the
  "nothing to write" text no longer says everything is saved. Shim mode `dirsync-err` with `QB_SHIM_ERRNO`.

## 0.92.0 beta (2026-11-20): the content decides, the project ID in backup names, checked copies, failures that are told

- **A file whose time alone changed is no other version** (the sync program of the report sets a file's time a few
  seconds after the program's write). `DocumentStore::writeEntry`: when the stamp differs, the file's bytes are
  compared with `e.read` (what was read or last written here; `it->read = data` after each write); equal: `elsewhere`
  is false and only `e.stamp` is renewed. `Project::saveIndex`: `shaOnDisk` against `m_indexSha` both ways (other
  bytes with the same stamp: changed, as since 0.65.0; the same bytes with another stamp: not changed), and a file
  that already holds the bytes to write is not written (the marks are cleared, `writeUiState` still runs). Scrivener
  format: `SyncState::scrivxSha` (key `scrivxSha` of `settings/scrivener-sync.json`) for the `.scrivx`; texts there
  were compared by SHA-1 already; media files are still judged by size and time (`statHash`), which only matters
  when a media file was changed on both sides.
- **No round of save, sync, changed time, save**: nothing in the program watches file times; a time-only change
  writes no file, no copy, no snapshot. v0920 touches every file of a project and saves three times: no file changes.
- **The notice's button**: `MainWindow::conflictNoticeOff` / `setConflictNoticeOff` (a static, not saved); versions
  are still kept while it is off.
- **Backup names** `"<title> #<ID> bak <date>.zip"`: `backup::projectId` = `idFromSeed` (SHA-256, five characters of
  `23456789ABCDEFGHJKMNPQRSTUVWXYZ`) of, in this order: the `.scrivx` Identifier (a project in Scrivener format: its
  working copy is local, the identifier travels), the seed in `settings/backup-id.txt`, the Draft folder's id, the
  title. No new id is written for an existing project, so two computers agree without talking. `giveNewId` writes
  the seed file for copies the program makes (`copyAsQuillbinder`, `backup::restore`, a project made from a
  template file), which all carry their original's Draft id. `nameStart` is what the clean-up counts
  (`ownBackupsOf(dir, mine)`); `backupsFor` offers every own-form name whose title without the ID is this title, and
  the others that begin `"<title> bak "` or `"<title> #"`. A version before 0.92.0 takes `"<title> #<ID>"` for another
  title: it neither counts nor deletes the new backups (read in its code, not run).
- **Checked copies**: `safesave::copyChecked` (SafeSave.h; the tests make it fail with `QB_TEST_FAIL_COPY=<part of a
  path>`). The Scrivener import adds a warning per failed file and does not count it; `importMedia` / `addWebPage`
  leave no item behind; `mergeFromOld(..., QString *copyFailed)` stops `scrivener::open` (the new copy in `.reading`
  is removed, the old working copy stays, the project does not open); `importProject` returns nullptr when its
  final save fails. Left as they were, since no file of the project is at stake: `DragDrop.cpp` (the export of a
  drag), `MediaView.cpp` (a page's side files into a temporary folder), the app icon of the menu entry,
  `SpeechTool.cpp` (it checks that the copy exists).
- **The kind of a failed write**: `core/FailureKind.h` (`failure::note`, `noteErrno`, `last`; one slot, cleared at
  the start of `Project::saveImpl`, the first kind stays), noted in `savebatch::writeSynced` from `errno`;
  `MainWindow::plainSaveReason(err, kind)`. Test switch `QB_TEST_WRITE_FAILS="<errno>|<text>|<part of path>"`.
- **States that report**: `addons::writeAddonState` / `launcher::writeState` return bool with a reason
  (`QB_TEST_FAIL_STATE=addons|core`); `setEnabled`, `setActiveCore`, `removeCore`, `finish` hand it on. The way back
  of `installData` (`backOne`, `tellLeft`; `QB_TEST_FAIL_RENAME="<part>|<moves that still work>"`).
- **The backup of a manual save**: `MainWindow::tellManualBackupProblem` (once per cause, the date taken out of it).

## Add-on keys (0.91.3 beta, 2026-11-15)

- **Official add-ons for Windows (2026-11-16)**: an entry may carry a `windows` block (its own reviewed commit and
  build; `official-addons/README.md`); `tools/addon-entry.py ... --windows` reads it (tag `addon-<id>-<version>-windows`,
  cache key `...-windows-2025`, `check-package` wants `"platform": "windows-x64"` and no `.so`, `sources` names
  Microsoft's redistributable DLLs instead of Ubuntu source packages; `QB_BASH` names Git Bash, since a plain `bash`
  could be WSL's in System32). The workflow `.github/workflows/addon-release-windows.yml` has the same five jobs as
  `addon-release.yml` (the build on `windows-2025` without secrets, inside MSVC's environment; the install checks with
  the newest public Quillbinder for Windows, unzipped, `quillbinder.exe --install`; the signing on Ubuntu with both
  keys; publish `--latest=false`); v0913 checks both workflows' shape. Without `--windows` everything is as before.

- The writer's decision of 2026-11-15: one key for the program and its updates (the release key, `kReleaseKey`, secret
  `QB_SIGNING_KEY`), one or more keys for official add-ons, and unsigned add-ons still possible after the warning.
- An add-on key is trusted through a certificate in `manifest.sig`: `"certificate": {"format": 1, "role": "addons",
  "key": "<64 hex>", "name", "issued": "yyyy-mm-dd", "by": "<release key id>", "signature": "<base64>"}`; the release
  key signs `signing::certificateText` ("Quillbinder key certificate 1", then role, key, name, issued, one per line).
  `signing::checkManifest` tries the release keys first (`Check::releaseKey`), then the certificate: wrong format or
  role, a name with a line break, a key that is not the signing key, an issuer the program does not know: UnknownKey
  (counts as unsigned); a certificate whose signature does not fit, or a manifest that does not fit the add-on key:
  Invalid (refused). `Check::signer` is the certificate's name; the install window shows it.
- A program update is never trusted through an add-on key: `addons::inspect` turns Valid without `releaseKey` into
  UnknownKey for `kind == core`, and `launcher::coreOrigin` wants `releaseKey`. A bundle with add-ons and a program
  update: `make-package.py sign` signs it with the release key (`holds_program_update`).
- `tools/make-package.py`: `certify --name <name> [--issued <date>] <add-on key> <certificate>` (with
  `QB_SIGNING_KEY_FILE`); `sign` uses `QB_ADDON_KEY_FILE` and `QB_ADDON_CERT` for add-ons when both are set (it checks
  that the certificate names that key); `verify --release-key <public pem> <packages>` checks by the program's rule
  (exit status 1 for any package not trusted).
- Release workflow, job `sign-packages`: with the secret `QB_ADDON_KEY` it certifies that key ("Quillbinder official
  add-ons") and signs the add-ons with it; without it, everything as before with the release key. Then `verify` over
  every package. `release-windows.yml` stays with the release key: the Windows program (0.91.1) does not know add-on
  keys yet.
- Older programs (up to 0.91.2) see an add-on key as UnknownKey: a warning, no refusal; on the command line only
  with `--allow-unsigned`. So the project's own add-ons move to the add-on key only when it exists, and the release
  text says so.
- Official add-ons: `official-addons/<id>.json` (the reviewed commit and the commands that build it,
  `official-addons/README.md`), `tools/addon-entry.py` (check, cache, run, check-package, sources, notes; testdata and fetch-testdata since
  Translation and Proofreading 1.1: test data from several releases, some files of one by `patterns`, cached under a
  key made from the list) and the
  workflow `addon-release.yml` (prepare, build without secrets, sign on request with both keys and no container,
  check-signed: installs into the newest AppImage without `--allow-unsigned`, publish: release `addon-<id>-<version>`,
  `--latest=false`; tags not starting with `v`, which the download page and `release.yml` keep to).
  `collect-sources.py --of-file <library> <dir>` packs the Ubuntu source of one library (the translation add-on's
  `libgomp.so.1` comes from `gcc-14`, not `gcc-13`).
- Checks: `tests/v0913_tests.cpp` (23).

## Backups and sync programs (0.90.3 beta, 2026-11-13)

Reported: a backup folder inside a synced folder filled with "<backup>-<computer>-safeBackup-000n.zip"
copies. The sync program's log showed the cause: its upload of the new backup was refused (HTTP 401), it then took the
leftover of its own failed upload for a newer version online, set the local file aside under that name and tried again,
every pass. The backup's ".zip.part" file was not the cause (the client handled the rename as one new file). Two things
were this program's:

- **The clean-up knows its own files** (`backup::isOwnBackupName`, `ownForm()` in `core/Backup.cpp`): only
  "<title> bak <yyyy-MM-dd HH-mm-ss>[ (n)][ (incomplete[ n])].zip" (n from 2) counts for `keep` and can be deleted. Any
  other file in a project's backup folder is left alone (sync copies, "(1)", "- Copy"). `backupsFor` (Restore From
  Backup) still lists every "* bak *.zip". Never widen the clean-up's pattern again.
- **Packed outside, put in place as one file**: `backupProject` packs "<name>.zip.part" in
  `backup::workFolderFor(<backup folder>)` (`<data>/backup-work/<12 hex of the folder's place>`, with `target.txt`),
  checks it, renames it there to its final name ("(incomplete)" included), then `putInPlace`: `savebatch::replace`
  (rename; one drive), else `copyInto` (straight to the final name with `NewOnly`, fsync, read again and compared by
  SHA-1; never `QFile::copy`, `QFile::rename` or `safesave::copyFile` here: they write a temporary file beside the
  target or copy in ways a watcher sees as several files). The backup folder sees one `IN_MOVED_TO`, or `IN_CREATE` +
  one `IN_CLOSE_WRITE`. All of it runs in the worker thread.
- (0.91.0: the backups are straight in the backup folder and told apart by title; see "Backups in one folder ...".)
- **A backup that waits**: when the folder cannot be made or written, the whole backup stays in the work folder
  (`*waiting` true, `*outPath` there, the error text names both folders; return false); the next backup into that
  folder puts the waiting ones in place first, oldest first. Of those that wait, the newest `keep` stay. `Where::AsChosen`
  keeps nothing. A same-named file in the folder: the same bytes -> the waiting file goes; not a whole ZIP (a copy that
  was cut off) -> replaced; another whole ZIP -> it stays and ours gets the next "(n)". `sweepWork` removes `.part`
  files older than an hour.
- Test switch `QB_TEST_BACKUP_COPY` (test builds): the copy also on one drive. `MainWindow::manualSave` shows a waiting
  backup's own text in the status bar; `tellBackupProblem` shows it under "failed".
- (The review of the temporary names written inside a project folder was done in 0.93.0: see its table at the top.)

## The audit of 0.81.0 (0.81.1 beta, 2026-11-11, part 1, the four findings at function level)

- **Unreadable documents** (F-01, `core/DocumentStore.*`): `whyNotEditable(id, notes)` (the reason as part of a
  sentence, "" when editable; only documents read so far are known: load them first), `releaseDamaged` (Roll Back: the
  damaged file kept with `syncsafe::keepCopy`, the key out of `m_unreadable`/`m_damaged`, signal `editableAgain`, which
  `TextEditor` follows; a file that cannot be read or has not arrived is refused), `snapshots::rollBack(..., keptCopy)`
  (the inspector tells with `qbLastRollBackNote` under `QB_NO_DIALOGS`). `editStored` and `restyleAll` leave an
  unreadable document alone and note its title (`takeSkippedUnreadable`: `ReplaceResult::skipped`, the status bar after
  `commitStyles`); `convertStorage` and `applyFormatting` leave it as read. Merge (`mergeDocuments`) and Split
  (`splitDocument`) refuse. Safety net in `writeEntry`: `m_shown` holds an unreadable entry as read (`shownBytes`: the
  store's own writing, pictures by name, with the formatting and storage of that moment); a different text at the save
  is written as RTF into `settings/conflicts` ("<title> (changed, not saved) <time>.rtf"), signal `changeKeptAside` ->
  notice `changeKeptAsideNotice` (`MainWindow::lastChangeKeptAside`); the save is never failed for it (one failing
  `writeEntry` drops the whole batch). `setContent`/`setNotesContent` call `markContentsDirty` over the whole text: under
  Qt 6.12 an editor kept the old layout after a whole replacement and showed an empty page.
- **Pictures in notes** (F-02): `serialize(id, doc, notes)` looks `savedImages` up under the entry's key (`n:<id>`).
- **Save journal** (F-03, `core/SaveBatch.*`): `recover` keeps a journal that cannot be read (open, JSON, no `files`) with
  every staged file under its target's name in `settings/conflicts/unreadable save journal <time>/` (`Recovery::unreadable`,
  `keptIn`, `keptFiles`, `keptFailed`); a journal whose `key` is another computer's and whose `time` is younger than
  `kOtherSaveWait` (15 min) is left alone with all staged files (`Recovery::waiting()`, `otherDevice`, `otherSince`), and
  `Project::finishInterruptedSave` (`saveImpl`, `convertStorage`) then refuses to save with the reason (retried by the
  window). Tests fake another computer's key with `QB_DEVICE_NAME`.
- **RTF** (F-04, `text/RtfIO.cpp`): a `qbfigure` mark inside a table (rows pending or `intbl`) is ignored, and a cell that
  ends inside an open figure drops the figure: the two never take the same block numbers. `Writer::table` walks a cell
  only forward and inside `cell.lastPosition()`. `qb_fuzz_tests` and the libFuzzer RTF harness write what they read;
  `tests/fuzz/inputs/` holds the auditor's file.

## The audit of 0.80.0 (0.81.0 beta, 2026-11-11, part 1 of 2)

- **Backups** (`core/Backup.cpp`): `ageKey` sorts by the date and number in the name, then the file time (F-01: the
  whole name sorted a renamed project's new backups first, and the clean-up deleted them); the backup just made is
  never deleted and must exist after the clean-up; incomplete backups are trimmed to `keep` too. `markDue`/`takeDue`
  (`<data>/backups-due.txt`): `saveForTermination` notes the closing backup it has no time for, `adoptProject` makes
  it (S-03, `MainWindow::lastDueBackup`). `restore` checks the free space first (`QStorageInfo`), unpacks in a
  `QThread::create` worker with the progress handler and Cancel, and names the failed files (20, then "and n more").
- **ZIP** (`core/Zip.*`): the reader knows ZIP64 (end record, extra field 0x0001; no 2 GiB limit), reads small archives
  whole and large ones in pieces from the open file (no mapping any more), `fileData` stays in memory (1 GiB limit),
  `extractFile`/`extractAll(dir, &failed)` stream each entry through a `QSaveFile` (flush checked before
  `safesave::commit`, so a full disk gives its own reason), with progress and Cancel (`setProgress`). The writer's
  `addDirectory` walks itself: folder links are followed; a loop or a dangling link is `skipped()` (F-05).
- **Saving**: `Project::save()` without an error pointer emits `saveFailed` (F-06); `saveImpl` sets
  `lastFailureWithoutChanges()` (only new counts failed: its own notice text, S-12) and retries the counts in the next
  autosave after a failure; `Project::open` starts `m_lastIndexWrite` (counts found at the opening wait a minute);
  `m_maxSaveWait` 10 s (S-04). `BinderModel::emptyTrash(&err)`: out of the binder (`DocumentStore::forgetDocument`),
  saved, then `removeDocumentFiles` (after the next save that works, `removePendingFiles` on `Project::saved`).
  Spare copies rotate to `spare-copy-2/-3` (U-02). `convertStorage` loads, writes and forgets one document at a time
  (U-04). `uiStateFile()` = `ui-<machineKey>.json`, `oldUiStateFile()` read once and removed (S-06).
  `syncsafe::keepCopy` uses `safesave::copyFile` (F-07). Snapshots: `updateIndex` keeps a damaged `index.json` as
  `index.damaged-<time>.json` and rescues the readable pairs; `take` true with *error when only the title failed;
  `setTitle(..., &error)` (F-04); the comparison stops at 2,500 steps.
- **Window**: `ui/HintLabel.*` (`hintlabel::style`: explanation lines in the text color moved toward the background,
  contrast >= 4.5:1, worked out again on palette changes; never `setEnabled(false)` for a label of explanation, S-01);
  `noteSaveFailed`: `plainSaveReason`, details hold the system's text, `placeBesideText` (S-08); First Start without
  the settings question (S-09); titles without the program name (Qt adds it, S-07).
- **Test switches** (`core/TestSwitches.h`, S-10): `QB_TEST_ENV_SET` / `QB_TEST_ENV` / `QB_TEST_ENV_INT` /
  `QB_TEST_ENV_BYTES` read a variable only in `QB_TEST_SWITCHES` builds (the name is not in a release binary;
  audit1's X-10 list has them). Use them for every new test-only variable; the manual's appendix lists what a release
  still reads.
- **Release**: `publish` needs `tests` and `code-check` (S-02, S-11); `tools/code-check.sh` runs cppcheck 2.13 over the
  saving, backup and update files and fails on error/warning/portability/performance findings not listed in
  `tools/code-check-suppressions.txt`.

## The two external audits' fixes (0.70.0 beta, reports A and B of 2026-11-10)

- **Markdown emphasis** (A F-01, `MarkdownIO.cpp`): the reader takes runs of up to six marks, closes before it opens,
  follows CommonMark's rule of three, and lets the two kinds of one "***" run close in either order; the writer
  opens the longer-lasting format first and writes `<b>`/`<i>` for a bold or italic run whose marks would be
  ambiguous (a run that closes and opens, marks next to a space on the wrong side). `startsAt` uses a view (it copied
  the rest of the paragraph per character). Property check in v0700 (4,000 random sentences).
- **Readers**: `QPointer<QTextList>` for cached lists in the RTF and Markdown readers (Qt deletes empty lists; B F-1),
  list levels 0..8; `rtf::wellFormed` / `rtf::mended`; `DocumentStore::decodeStored` (UTF-8 strictly, UTF-16, zero
  bytes); a damaged or other-encoding file -> `m_damaged` + `m_unreadable` (shown as read, read-only, never
  written), signal `damagedFile` -> notice `damagedDocumentNotice`; `charactersLeftOut` once per document.
- **Failed saves** (A S-01): `MainWindow::noteSaveFailed` / `noteSaved` / `retrySave` / `updateSaveState`: status-bar
  label `saveState` (Saved / Not saved yet / red failure), `setWindowModified` with "[*]" in the title, retries every
  15 s for 2 min then 60 s, notice `saveFailedNotice` through `showWithoutTakingKeys` (again after 5 min), button
  `saveFailedElsewhere`. `Project::setUnsavedExtra` / `unsavedTitles` / `emergencyExtra`; `m_saveDelay` and the 30 s
  latest save whatever the delay (A S-09).
- **Mind maps** (A U-01, U-02): `MindMapDoc::saveOrTell` (emits the project's `saveFailed`, retries in 15 s),
  `mindmaps::saveAll(p, &err)` checked by File > Save, close and the end; `closeAll(p, discard)`;
  `mindmaps::writeUnsaved` for the emergency copy; a board that cannot be read is `isUnreadable()`, `locked()`, never
  written (`mindMapUnreadableNotice`).
- **Backups** (A F-02, U-05, U-06, U-09, F-11, U-10): `backup::projectFolder` ("<title> <key>", key = SHA-1 of the
  project folder's canonical place), `Where::ProjectFolder|AsChosen`, `backupsFor(Project*)`; packing and checking in a
  `QThread::create` worker with `setProgressHandler` (the window's QProgressDialog `backupProgress`, Cancel); fsync of
  the archive and `savebatch::syncDir` before the clean-up and of staged folders before the journal; ZipWriter leaves
  out `settings/lock`; `backup::restore` + `MainWindow::restoreBackup` / `restoreBackupFile` (File > Back Up > Restore
  From Backup).
- **Pictures** (A U-03): `DocumentStore::carryPictures(namesIn, from, to)` adds a picture under the target's resolved
  name too (QTextDocument looks resources up by the resolved URL); Merge, Split and every `setContent`.
- **Questions** (A F-03, B S-3): `ui/Ask.h` (`ask::question` with a default and an action name, `ask::confirm`); every
  `QMessageBox::question` names its default (v0700's source check).
- **Settings**: `AppSettings::keepSavingSafe` (a project may only make saving safer; ranges), Options opens for all
  projects (A S-05), `app/language` program-only.
- **Project file** (A F-06, F-07, F-08, B F-2, L-1): root element and version checked; nesting refused above 5,000
  (checked with QXmlStreamReader before QDom), flattened above 200 (`readItem` without recursion); duplicate ids get
  new ids with a copy of the shared text folder; counts saturate; `resolveProjectFile` prefers the .qbx named like the
  folder, else the newest, and `open()` tells about the others. Boards: ids, places and sizes bounded.
- **Word 97** (A U-11): pieces bounded by the stream, character counts by the pieces (16 million at most).
- **Build and release** (A S-04, S-03, S-02, U-07): `QB_TEST_SWITCHES` off by default (the runner refuses a build
  without it); the release workflow's jobs build (no key), tests (full suite but v060, one retry; publish waits for it again since 0.81.0, audit of 0.80.0 S-02), code-check (cppcheck, 0.81.0), sign-packages
  (`make-package.py sign`), assemble (the full version), sign-checksums, publish; the installer keeps
  `package-manifest.json`/`.sig`, and `launcher::coreOrigin` checks them before every start (Linux; unsigned only with
  `QB_ALLOW_UNSIGNED_CORE`, which `packaging/linux/e2e-appimage.sh` sets). The first run on GitHub's container failed for the
  machine, not the program: no `unzip` (now in `tools/cloud-setup.sh`) and no sound server (PulseAudio refuses the job's
  home folder, owned by another user; the runner now starts it with folders of its own, `-n` with a null sink only, log
  `pulseaudio.log`, and prints "(sound server: ...)"). On a second failure the workflow prints the failed checks with
  their details into the job's log, which the session can read (`gh api .../actions/jobs/<id>/logs`; the artifacts
  are on a host the session cannot reach). `ZipWriter` counts progress and sees Cancel per megabyte of a file.
- **German** (S-06): `ui/Language.*` (QTranslator from `:/translations/quillbinder_de.qm` and `qt_de.qm`, setting
  `app/language`, `QB_LANGUAGE`), `translations/README.md` for lupdate/lrelease. **Dark page** (S-07): in
  `theme::resolvedColors`, a dark theme with the standard own paper shows #1e1e1e with white text (or the theme's
  own page). **Self-test** (S-08): the font resolver. **Crash reports** (Info-1): a note about personal names.

## The external audit's fixes (0.65.0, report of 2026-11-10)

- **Save journal** (`src/core/SaveBatch.*`, audit F19): `Project::saveImpl` runs a `savebatch::Batch` (`Scope` makes it
  `current()`); `savebatch::put(path, data, error, title, place, done)` stages `<file>.qbstage` (synced) in a batch, else
  writes at once; `done` runs once the file is in place (marks, stamps, `savedImages`). `commit`: with 2 files or more the
  journal `settings/journal.json` (target, staged, sha256, replaces, title, place, device, key), then POSIX `rename`
  (`savebatch::replace`, MoveFileEx on Windows), then the journal goes. A staging failure stops the whole save
  (`batch.drop()`, entries stay dirty). `savebatch::recover` at `Project::open` and before each save: a committed journal
  is finished (target unchanged since: renamed; changed: the staged file kept in `settings/conflicts`; missing: listed as
  lost), stray `.qbstage` files without a journal are removed; only files inside the project, and only their own staged
  name. `Project::saveRecoveryText` -> `MainWindow::m_lastSaveRecovery` (notice `saveRecoveryNotice`). Backups skip
  `.qbstage` files and the journal. Writers on the batch: `DocumentStore::writeEntry` and its pictures, `Objects::writeJson`,
  `Project::saveIndex`, `contents-by-title.txt` (`Project::titleListText`, Quillbinder format only), `convertStorage`
  (its own batch). Not on it: the ui state, mind maps, snapshots, the Scrivener write.
- **Pictures** (F04): `DocumentStore::serialize` encodes PNG into memory and writes it through `put`; a failure sets
  `serializeError()`, and `writeEntry` fails. `importImage` writes checked (the resource stays for the save).
- **Reads** (F23): `readFile` treats a read error or fewer bytes than the file's size as unreadable (`m_unreadable`).
- **Same size and time** (F14): `writeEntry` compares the file with `Entry::read`, `saveIndex` with `m_indexSha`.
- **Snapshots** (F02): `snapshots::load(..., &error)` and `rollBack(p, s, error, allowEmpty)`; the inspector reads first,
  asks for an empty snapshot, stops when the "before" snapshot fails (`snapshotRollBack`, `snapshotList`,
  property `qbLastRollBackError` with `QB_NO_DIALOGS`).
- **Backups** (F03): skipped files -> `... (incomplete).zip`, true with *error; retention counts complete backups only and
  removes incomplete ones older than the oldest kept. `MainWindow::tellBackupProblem` (open, close). Test switch
  `QB_TEST_UNREADABLE=<path suffix>` in `ZipWriter::addDirectory`.
- **Safe copies** (`safesave::copyFile`, `safesave::writeFile` in `SafeSave.h`, header-only, also for plugins): used by the
  Scrivener media/icons/web page files, `copyDocument` (F06, `duplicateItems` removes a partial duplicate), the command
  exporter (F11, also exit status; F24: waits in 100 ms steps with `progress::tick`).
- **Scrivener format** (F10, F05, F25): the removed-document sweep knows only `state.theirs`; documents of the .scrivx on
  disk this program never knew (`madeElsewhere`) keep their folders, and the .scrivx is then not written (`binderKept`,
  `keptTheirs`, one notice per stamp: `Project::scrivenerBinderTold`). `SyncState::binder`/`binderKnown` ("binder" in
  scrivener-sync.json): the UUIDs of the .scrivx at the last read or write; `mergeFromOld` keeps working-copy documents
  not among them (states of older versions: `theirs` as before). Media and mind maps: changed only in the .scriv ->
  kept; both -> `keepTheirs` first; copies through `safesave::copyFile`; a mind map or icon failure fails the sync.
- **Objects** (F09): removed files and orphan pictures are deleted in the batch's `after` step, only when all writes
  worked; pictures that cannot be removed stay listed.
- **The end** (F07): `saveForTermination` on failure: `Project::writeEmergencyCopy` (RTF of every dirty entry,
  `DocumentStore::writeUnsaved`, `<data>/Emergency/<title> <time>/` with README.txt), `MainWindow::writeEmergencyNotice`
  (`<data>/Emergency/notice.txt`), `crash::setTerminationSaveFailed()` -> exit status 1; `showEmergencyNotice` at the
  next start (main.cpp).
- **Close question** (F26): `closeUnsavedQuestion` with Keep Open (default), `closeSaveElsewhere`, `closeLoseChanges`.
- **Scratch Pad 1.1** (F01): `save()` returns bool (safesave), switching notes / New Note / folder / rename / send refuse
  after a failed save (`tellUnsaved`), `saveEmergencyCopy` at the end. **Speech Transcription 1.4**: `TranscriptPanel::save`
  bool, `saveOrKeep` (emergency copy) before a recording and at the end.
- **Lock** (F13): `MainWindow::lockNotWritten` (notice `lockWriteNotice`, once per opening).
- **Bundles** (F18): `Addons.cpp` `prepare` (every part into `.new-<name>`, the program's self-test), then all renamed,
  the placed ones put back when one fails, then `finish`.
- **ZIP reader** (F17): archives above 64 MiB are mapped (`ZipReader::m_file`), 2 GiB and more refused.
- **Signatures** (F12, `src/core/Signing.*`, Monocypher 4.0.3 unchanged in `src/core/third_party/monocypher`, BSD-2-Clause,
  listed in resources/licenses and licenses-windows): `manifest.sig` = Ed25519 signature of manifest.json
  (`{"algorithm","key","signature","signs"}`); `signing::checkManifest`, `trustedKeys()` (the release key `kReleaseKey`,
  `packaging/quillbinder-signing-public.pem`; test builds also `QB_TEST_SIGNING_KEY`). `PackageInfo::signature`,
  `trusted()` (a bundle: every installed part), `addons::trustProblem(info, Unsigned::Refuse|AllowAddons|AllowAll)`:
  a mismatching signature never; a program update only signed; an add-on after the window's warning
  (`addonSignedNote` / `addonCodeWarning`, Cancel the default). `--install <file> --allow-unsigned` on the command line
  (e2e-appimage.sh, the full AppImage's AppRun, win-check*.sh pass it). Test builds allow everything unless
  `QB_REQUIRE_SIGNED` is set. `tools/make-package.py` signs with `QB_SIGNING_KEY_FILE` (openssl). The release workflow:
  step "The signing key" (secret `QB_SIGNING_KEY`, checked against `kReleaseKey`, else the run stops), the packages
  signed while they are made, "Checksums and their signature" (SHA256SUMS.txt + .sig + the public key, after the
  third-party sources), `publish-release.sh` checks the signed list instead of making it again.
- **Runner** (F08): `tools/run-all-tests.sh` ends with 1 and `failed: ...` / `not built: ...` (`runx` for suites that may
  be missing; the line of the long-gone `menu_audit` was removed).

## Fixes of the system test's full run (0.64.2, 2026-11-10)

- **Formats changed while walking fragments**: never change a char format inside a `QTextBlock::iterator` loop (Qt
  invalidates the iterator: SIGSEGV in S-W14); collect (start, end, format) first, then apply. Done in
  `TextEditor::addLinkedNote`, `scrivRtf` (links, note anchors), `FileImporter`'s `applyImportOptions`, `ObjectsUi`,
  `Inspector` (snapshot zoom).
- **Menus filled in `aboutToShow`** (Go To, Quick Reference, bookmarks): the lambdas capture the item's id and look it
  up (`m_project->item(id)`); a pointer captured there crashed after the item was deleted (S-E04).
- **Pictures**: `text::withoutPicture` / `text::pictureSafePieces` (a picture's format on letters is not a picture; one
  picture per U+FFFC) in `insertSemantic`, `RtfIO`'s runs, `MarkdownIO`'s segments; `TextEditor` drops the picture
  format from the current char format after a picture. `insertSemantic` copies the source document's picture resources
  (RTF on the clipboard names them `rtfimage://...`; the pasted picture was lost at the save).
  `DocumentStore::setNotesContent` copies resources as `setContent` does (pictures in notes of a .scriv).
- **Markdown**: the style Centered is written `::Centered::` (`->text<-` stays for centering by hand; `toStandardMarkdown`
  turns the style back into `->text<-`); `trimmedAscii` for centered paragraphs (no-break spaces at the ends stay);
  white space at the end of a run moves out of its markers also when it is a text item of its own (`**ca **` was read
  back as asterisks).
- **Notices during typing**: `MainWindow::showWithoutTakingKeys` (not modal, `WA_ShowWithoutActivating`, the safe
  button focused) for `lockTakenNotice` (Keep Working the default and escape button) and `conflictNotice`.
  `changeEvent`'s full save on the way to another program emits `saveFailed` on failure (not the quick save when a window of the program takes over: m9 hung on a second modal box over Close Project's question). `TextEditor::leaveList` (Backspace at a list
  item's start, a list switched off): the style's block format with the paragraph's own direct spec.
- **Scrivener format**: `sameExceptStamps` compares the .scrivx as XML (`canonicalScrivx`: elements, sorted attributes
  without the root's Modified/ModID/Device, text, white space only in leaf elements), else bytes; a .scriv opened and
  closed without a change stays byte for byte (label colors kept as written
  when the 8-bit color is the same, `IncludeInCompile` and `MetaData` not added to elements that had none, `TextSettings`
  only for new items, `Device` ignored by `sameExceptStamps`, `search.indexes` made only with a change or for a new
  .scriv); a failed `search.indexes` write fails the sync; `sync` saves the state also when `write` fails (what was
  written is known); `write` refuses a .scriv that is gone (`state.theirs` known, same path); files kept as theirs
  (`keptTheirs`) clear the fingerprint so the next opening reads the .scriv (S-D02); the importer skips QSaveFile
  leftovers (`isSaveLeftover`) and keeps a synopsis' leading spaces; `lockedByScrivener` reads key=value and XML.
- **Projects**: `Project::resolveProjectFile` maps `settings/spare-copy.qbx` to the project's own file; `saveImpl`
  refuses when the project folder is gone (no folder made anew with only the changed documents).
- **Second start**: `main.cpp` asks the running programs (`showProject` through the tool host's link,
  `MainWindow::showIfOpen`) before making a window; the program that has the project open raises it.
- Also: `SettingsLayer::takeProjectValues` writes program-only keys changed in "This project" scope to the program
  file; `manualSave` with Scrivener's lock emits `saveFailed` (one notice); Take Snapshot reports failures; command
  exporters drop an argument whose `{author}` / `{title}` is empty with its option; word counts
  (`belongsToWord`); AutoCorrect's `smartQuote(typed, textBefore, style)` and `apostropheBeforeLetter`; the RTF reader
  takes italic and bold from a face name also without kept fonts; `mergeFormat` changes the whole word only inside it;
  `SpeechTool`'s download handlers hold their own reply (a resumed download's old reply no longer reaches the new one).
- **Test switch**: `PluginManager::setMediaOffForTests` (QB_TEST_SWITCHES builds) acts as if Media Playback were not
  installed (a plugin loaded in the process stays loaded when its add-on is removed).
- **System test (harness)**: S-T01's typing gate in the 1,000-row table is gone (the writer's decision of 2026-11-10:
  such a table is a spreadsheet's job); Scrivener format keeps inline notes as linked notes (decision "1A": the model
  `scrivenerText`); `showDoc`, `cursorTo`, `toEnd` activate the window (focus, Edit commands and shortcuts need it
  without a window manager); `formatRuns` ignores bold/italic/underline/strike on white space and the formatting of an
  empty text; `fileDialogLater` sets the focused name field; S-Y04 syncs through `SyncSim` (conflicted copies).

## The external audit's fixes (0.64.1, report of 2026-11-09)

- **Names from outside** (`src/core/PathSafety.*`): `isSafeId` for every id that becomes a folder name (`Project::readItem`
  gives an unsafe id a new one; `contentDir` / `snapshotsDir` map one anyway to `_invalid-<hash>`), `isSafeExtension`
  (`mediaFilePath`: else `file.bin`), `isSafeRelative` + `isInside` for archive and package paths (`ZipReader::extractAll`,
  `Addons`), `removeTreeInside` for every recursive delete of a project's folder (a link is removed, not followed).
  `ZipReader::fileData(name, &ok)`: CRC, exact size, bounds; a damaged entry is never written. `Cfb.cpp`: DIFAT loop and
  sectors outside the file. `MiniXml`: depth limit 1000.
- **Settings a project may hold**: `settingslayer::programOnly` is an allow list (see `SettingsLayer.cpp`);
  `namesProgramOrPlace` keys are never a project's and Load Options asks (`loadOptionsPlaces`).
- **Links** (`src/ui/LinkOpen.*`): `links::open` (http/https/mailto at once, `linkQuestion` for other targets,
  `linkRefused` for program files: `isProgramFile`, last ending or an executable with "#!", ELF, "MZ"), `links::openFile`
  for a project's file in another program; `setOpenerForTests`. Used by `MainWindow::openLink`, mind maps, bookmarks
  (both lists and the menu), `MediaView` (the web page's `QTextBrowser` has `openLinks` off and goes through it).
- **Scrivener's lock** (`scrivener::scrivenerLockStamp`): recorded in `Project::scrivenerLockAtOpen` by `scrivener::open`;
  `sync` refuses to write while another one is there (`scrivenerLockRefused`, `scrivenerLockWho`; the window's
  `saveFailed` handler shows `scrivenerLockNotice` once per lock, `closeProject` asks with its own text).
- **The computer's name** (`AppSettings::deviceName`, `app/deviceName`, `optDeviceName` on General > Saving,
  `syncsafe::setDeviceName`): replaces the host name and leaves `user=` empty; locks carry `key=` (`syncsafe::machineKey`,
  a hash of the machine id; `judge` compares it when both have one). `QB_DEVICE_NAME` (tests) still wins.
- **Test switches** (`QB_TEST_SWITCHES`, CMake option, default = `QB_BUILD_TESTS`): `QB_PLUGIN_PATH`, `QB_SPEECH_SOURCE`,
  `QB_SPEECH_ACCEPT_CHANGED`, `QB_SPEECH_SKIP_TEST` work only in builds with the tests; packages are built with
  `-DQB_BUILD_TESTS=OFF` (build-appimage, build-update). The Windows runner builds with the tests (switches on there).
- **Speech models**: `CatalogEntry::sha256` (Hugging Face's Git LFS ids, 2026-11-09), `sumsMatch`, `catalogEntryBySums`,
  `addressAllowed` (https, http only on loopback); `SpeechTool::checkAndInstall` hashes first (official source: must
  match; otherwise the question before reading), then `checkModelFile` (description, test transcription).
- **Socket folder**: `toolhost::Host::linkDirSafe` (lstat: a folder of this user, closed to others), else
  `~/.cache/quillbinder/link`.
- **Pipeline**: `packaging/linux/build-appimage.sh` pins linuxdeploy 1-alpha-20251107-1, linuxdeploy-plugin-qt 1-alpha-20250213-1,
  appimagetool 1.9.1 and type2-runtime 20251108 with SHA-256 (the 0.64.0 AppImage had the "continuous" runtime). The
  release workflow: `contents: read` for all, `publish` alone writes; actions pinned to commits (tags in comments).
- Not changed (and why): the comments that name Scrivener's windows (L-3) describe where a layout came from; they stay
  until the layouts were compared and reviewed by a lawyer (a comment changed alone would hide it, not change it).

## Fixes of the system test's first run (0.64.1)

- **Saving** (`src/core/SafeSave.h`, `safesave::commit(QSaveFile&)`): every QSaveFile of project data, settings files,
  compile output and the like is written with `safesave::write` (a short write, which Qt reports as no error, cancels
  the file) and committed through it (flush and `error()` checked, `fsync` checked, a `dup` of the descriptor closed
  and checked, then `commit()`): Qt 6.4's `commit()` ignores a failed last write and a failed sync (its own sync runs
  after the close), and its temporary file engine closes the file only after the rename, unchecked.
  `backup::backupProject` stops when its save fails. A failed docs.checksum write fails the Scrivener sync.
  Checked with `tests/faultshim.cpp` (LD_PRELOAD library `qbfaultshim`: `QB_SHIM_DIR`, `QB_SHIM_MODE=write|fsync|close`) in
  `qb_audit1_tests`' child runs (`--child <folder> save|backup:<dir>`).

- **Shown, not edited** (`DocumentStore::Entry::read` / `readRtf`: the bytes as read or last written): when the file
  changed elsewhere, `writeEntry` asks `unchangedSinceRead` (the read bytes read again and serialized the same way equal
  what would be written) and then writes nothing (showing a text emits `contentsChanged`, which marks it dirty).
- **Keep or do not write**: `DocumentStore::keepOtherVersion` returns false when neither a copy nor a snapshot was kept;
  `writeEntry` then fails with an error and leaves the entry dirty. The same for the project file (`Project::saveIndex`),
  and in Scrivener format (`keepTheirs` -> `mustWrite` false, `.scrivx` not written, `WriteReport::keepFailed`, `sync`
  returns false with the error after saving the state). `syncsafe::keepCopy` returns "" when `mkpath` fails too. Tests
  block `settings/conflicts` and `snapshots/<id>` with a plain file (the session's root ignores permissions).
- **Table cells**: `props::InTableCell` (block, never saved) is set on every cell paragraph by `tables::apply`;
  `display::blockFormatFor` keeps it and gives a cell paragraph left instead of the justified body alignment (unless set
  by hand or by its style). Compile: `PdfExporter`'s `cellParagraph`, `DocxWriter` (`w:jc left` in cells), the e-book CSS.
  Markdown: `cellParagraphs()` splits at " <p/> " also at a trimmed edge; the writer takes every block of a cell (also of
  a table or figure inside it). RTF (`RtfIO.cpp`, `table()`): a frame inside a cell is written as its paragraphs too.
- **Deleted for good**: `BinderModel::itemsAboutToBeDeleted` (from `deletePermanently`); the window clears every pane
  and copyholder showing one (S-T06 emptied the Trash past the window; a pane kept a deleted document and crashed at
  close).
- **Split at Selection** (`MainWindow::splitDocument`) refuses inside a frame (table, figure) and never lets its two
  clean-up steps select into one (Qt widens such a selection to the whole frame).
- **Compile progress**: `compile/Progress.*` (`progress::tick`, `progress::Scope`); `Compiler::build` and the built-in
  exporters tick in their loops; `CompileDialog`'s `CompileProgress` shows a modal `QProgressDialog` and runs
  `processEvents(ExcludeUserInputEvents)` at most every 100 ms, so `CrashGuard`'s heartbeat timer keeps beating.
  `PdfExporter`'s `Builder::build` makes and fills each table inside one edit block (`tableOpen`; laid out once, not per
  cell: 27 s to 0.4 s for the v0641 table); one edit block over the whole build left the last rows of v062's long table
  out of the pages (cause not found).
- **Copy**: `TextEditor::createMimeDataFromSelection` copies the pictures' resources into the temporary document, so the
  RTF on the clipboard carries them. **Merge** joins the notes too (same separator as the text).

## Synced folders, Record only, autosave in typing gaps (0.64)

- **Sync safety** (`src/core/SyncSafety.*`, Qt Core; `src/ui/MainWindowSync.cpp`): works on files only, whatever the
  sync program. The lock (`settings/lock`; for Scrivener format also `<x>.scriv/quillbinder.lock`): first line
  "<host> <pid>" as before (older versions read it), then `device=`, `user=`, `instance=` (one per running program),
  `program=`, `opened=`, `alive=` (UTC); `MainWindow::takeLocks` at adoption, renewed every 5 min (`QB_LOCK_RENEW_MS`),
  checked after every save (`Project::saved` -> `renewLocks(false)`): another computer's fresh lock -> notice
  `lockTakenNotice`, no more renewals; `releaseLocks` removes only our own lock. `confirmLock` before opening:
  `judge()` -> other window / other device (renewed within 15 min, `QB_LOCK_STALE_S`) / left over (Open is the
  default). `QB_DEVICE_NAME` fakes the computer's name (tests). No read-only mode for a whole project.
- **Stamps** (size + time): `DocumentStore::Entry::stamp` (set in `load()`, after each `writeEntry`), `Project::m_indexMark`
  (the project file), `SyncState::theirs[".scrivx"]` ("size:time"). A changed stamp before a write = someone else wrote
  the file: `DocumentStore::keepOtherVersion` / `syncsafe::keepCopy` keep it (copy in `settings/conflicts`, for a text
  also a snapshot "Other version (changed elsewhere, ...)") and `Project::conflictKept` -> `conflictNotice`. Scrivener
  format: `mustWrite` keeps theirs when both sides changed (before: ours won silently).
- **Nothing written without a change**: the UI state goes to `settings/ui-<computer>.json` (`Project::uiStateFile`,
  `markUiStateDirty`; `ui.json` of older versions is still read), `saveUiState` no longer marks the project file dirty;
  the spare copy only when it differs; the .scrivx only when more than its `Modified`/`ModID` differs
  (`sameExceptStamps`), docs.checksum only when it differs.
- **Texts not arrived**: a text with `charCount > 0` in the project file and no file -> `DocumentStore::m_missing` (+
  `m_unreadable`: read-only, never written, not recounted), `missingFile` signal; `textsNotArrived()` -> notice
  `notArrivedNotice` after opening. Not for Scrivener format (the working copy is local).
- **Autosave** (`Project::scheduleSave`): the 2 s pause timer as before; once the changes are 5 s old
  (`m_gapSaveAfter`, or the delay when longer) every change restarts `m_gapTimer` (300 ms): the first gap saves;
  `m_maxSaveWait` 30 s stays the latest. `autosaveCount()` for the tests. Replaces PR #35 (flat 5 s).
- **Speech, Record only** (setting `mode` = `record` / `live`; standard `record` only on Android): `EngineSettings::recordOnly`
  + `recordFile` -> `Engine::startRecordOnly` (no model, no Segmenter; `speech::WavWriter` 16 kHz mono 16 bit, header
  updated every second). `speech::transcriptsIn` lists `.wav` without `.txt` as pending (`Transcript::audio`,
  `pending()`). `SpeechTool::transcribeRecording` runs the WAV with `fileAsFast` (fed as fast as the worker takes it:
  at most 2 pieces waiting, the feed timer stops meanwhile) into `<name>.txt.part`, renamed at the end
  (`finishRecordingText`; stopped: the part file goes, the recording stays pending); a queue for Transcribe All;
  `deleteRecording` moves the WAV to the trash afterwards. Panel: item widget with `speechTranscribeNow`,
  `speechTranscribeAll`; rename and delete take the WAV along. States "recording", "reading <percent>".
- **Spelling menu**: `SpellChecker::checkedWordAt` (the highlighter's regex and rules) gives the menu its word at the
  click (Qt's WordUnderCursor ends at a straight apostrophe).
- **Card size slider**: saved on every change unless the handle is held, follows `AppSettings::changed`.

## Settings per project, first start, tables and pictures (0.62)

- **Settings layer** (`src/core/SettingsLayer.*`): every `QSettings` of the program reads a merged copy (the program's
  INI plus the project's `settings/options.ini`; QSettings' default path is pointed at it with `setPath`), and
  `settingslayer::sync()` (called by `AppSettings::save` and at exit) sends each change to the project file when the
  project holds that key, else to the program file. `programOnly()` lists the keys that never go to a project (recent
  projects, window/, layouts/, the look for new projects, crash watchdog, the switch itself ...): add a key there when
  a new setting must stay program-wide. `activate()` from `MainWindow::adoptProject` / `closeProject`; the switch
  `projectFirst` (General > Startup, `optProjectFirst`) off means the layer never merges a project, and
  `look::setProgramSettingsAlways` makes every project show the look for new projects (`look::setFollowSettings` stays the hook of the tests written before 0.40.2). File > Options in "This project"
  scope (`OptionsDialog::run`, `optScope`): `beginProjectEdit` keeps the values the dialog started from,
  `takeProjectValues` stores in the project what was changed (and what the project held); in "All projects" scope
  with a project open the layer is suspended while the dialog runs. Defaults and Load Options are off in project scope.
  Settings files: section `Options` (`sf::Options`).
- **First start** (`src/ui/FirstStartDialog.*`, from `main.cpp` before the desktop question): backups and the switch;
  `app/firstStartDone` in the program file. Skipped with `QB_NO_DIALOGS` or `QB_SKIP_FIRST_START` (the test runner sets it);
  `packaging/linux/e2e-appimage.sh` answers it and checks it came.
- **Tables and figures in the text** (`src/text/Tables.*`, see the comment at its top): `tables::Spec` as JSON in
  `props::TableSpec`, `tables::apply` after every change; figures are `QTextFrame`s with `props::Figure`; the description
  is `props::ImageDescription`. Qt keeps a block before and after every frame; Qt 6.4 crashes when rows or columns under a
  merged cell are removed, so `TextEditorTables.cpp` splits those cells first. Storage: Markdown `:::table {json}` rows
  `:::` and `:::figure {json}` ... `:::` (`MarkdownIO.cpp`), RTF `\trowd ... \row` with `{\*\qbtable json}`,
  `{\*\qbfigure json}`, `{\*\qbfigureend}`, `{\*\qbalt text}` (`RtfIO.cpp`). `DocumentStore::writeEntry` sets
  `Project::needsVersion` to 0.62.0 when a document has a frame. Dialogs: `src/ui/TableDialogs.*`; commands `insertTable`,
  `table*`, `picture` (Format > Table, right-click menu).
- **Compile** (exporter interface 1.4, `ExporterApi.h`): `Compiler::appendDocument` fills `Manuscript::tables` and the
  paragraphs' `table`, `cell`, `figure`, `figureCaption`, `Run::imageDescription`. PDF: tables and figures are rebuilt as
  frames of the layout document and placed as regions (`Region`, painted with `drawContents` clipped), a table in row bands
  with its header rows again on a new page; floats do not move `cur`, and something on its own lines after a float starts
  below it. Frame measures (borders, padding, margins, fixed widths) are given in 96-dpi pixels: Qt scales them to the
  1200-dpi layout device. Word: `w:tbl` (floating with `w:tblpPr`), a figure beside the text as a borderless floating table
  of one cell with its caption, `descr` on `wp:docPr`. EPUB: `<table class="qbtable">` and `<figure class="qbfigure">`.
  Markdown (the plugin and `writeMarkdown` for Pandoc): pipe tables.

## Quote conversion, Delete Old Snapshots (0.61.1)

- `autocorrect::convertQuotes(text, typographic, style)` (pure, per paragraph) gives the positions and new characters;
  `replace::convertQuotes` runs it through the project with Project Replace's engine: `replace::run(p, where, finder,
  mayChange, ...)` takes any `ChangeFinder` (the search's finder is one); the old regex functions are wrappers.
  `ConvertQuotesDialog` (`SearchTools.*`) is opened by `MainWindow::convertQuotes` from the buttons
  `optConvertQuotesSmart` / `optConvertQuotesStraight` on File > Options > Corrections (the Options dialog's parent
  must be the main window); afterwards `MainWindow::afterProjectWideChange` (shared with Project Replace) refreshes and saves.
- `snapshots::totals / removeAll` work on the whole `snapshots/` folder (also folders of documents no longer there);
  the action `deleteSnapshots` (`MainWindow::deleteOldSnapshots`, `updateSnapshotAction` on the Project menu's
  `aboutToShow` and in `updateActions`) asks with `deleteSnapshotsQuestion` and saves in full at once.
- Scrivener format: the full write (`scrivener::write`, not quick) takes snapshots out of the .scriv whose keys are in
  `SyncState::snapshots` but no longer among the document's snapshots; files and index.xml entries are matched by
  their moment (`snapshotMoment`, with the time zone of the name; a key's name is ours, the .scriv's file may carry
  another offset). Snapshots the state does not know (Scrivener's own, never read) stay.

## Speed with large projects (0.60.5)

- **Autosave vs. full save**: `Project::autosave()` (the timers of `scheduleSave`, and `MainWindow::changeEvent` when
  another window of the program becomes active) runs `saveImpl(quick = true)`; `save()` (switching to another program,
  File > Save, closing, every other caller) is the full save as before.
- **Quick Scrivener write** (`scrivener::sync(p, err, quick)` / `write(..., quick)`): texts, notes, synopses that changed
  (our file's SHA-1 comes from `knownSha1File`, read again only when size or time changed; Scrivener's file is read only
  when ours changed), docs.checksum, and the .scrivx only when `Project::binderChangedSinceSync` (set by `markIndexDirty`,
  cleared by a write of the project's own .scriv). Snapshots, icons, media, mind maps, removed documents and
  `Files/search.indexes` wait for the next full write; `SyncState::indexStale` (in scrivener-sync.json) makes the full write
  write the index even when it finds nothing new. The fingerprint after a quick write comes from `FingerprintCache` (filled
  by every `fingerprint()` call, lines of the files written updated); files changed elsewhere are not in it, so the next
  opening reads the .scriv again (the safe way round). A quick write without a .scrivx is a full one.
- **Counts**: `DocumentStore::recount` calls `Project::notifyItemCounted` (modified date, `m_countsDirty`, `itemChanged`),
  not `notifyItemChanged`: the project file is written with the next full save or a minute after its last write
  (`m_lastIndexWrite`); `hasUnsavedChanges()` does not count it.
- **Timing**: `QB_TIMING=1` prints `timing: ...` lines (`src/core/Timing.h`, `timing::Laps`) for the save steps, the
  Scrivener sync and write, and the editor's right-click menu.
- **Menus**: `noUiAnimations()` in `MainWindow.cpp` (constructor, and on `QEvent::ThemeChange` in `InputWatch`) switches
  off Qt's menu, combo, tooltip and tool box effects. `SpellChecker::suggestions` keeps the words looked up
  (`s_suggested`, emptied by `setLanguage`, `addToPersonal`, `setPersonalWords`, and above 500 words).
- Measured on the cloud machine (2 cores) with the v0605 project: autosave 250 to 350 ms before, 30 to 40 ms after; the
  right-click menu about 10 ms (60 ms the first time on a misspelled word: Hunspell's suggestions). Not done: the
  Combined Text view of a large folder (one editor per document, full layout and spell check of every document) and
  unloading documents that are no longer shown.

## Crash and freeze reports (0.19)

The program writes reports into `~/.local/share/Quillbinder/CrashReports` (fatal signal, or the
main thread silent for 30 s); the user copies them from **Help > Crash Reports**. When the user
sends one, read it as described in `docs/developer-guide.md` section 10 ("Reading a crash or freeze
report"); `packaging/linux/build-update.sh` keeps `dist/quillbinder-<version>-symbols.xz` (unstripped
program, same build ID) for `addr2line`: keep it with the package (the release workflow publishes it with each release; a session's own build goes on the branch `assets`).
`src/core/CrashGuard.*` (handlers, watchdog, sessions) must stay async-signal-safe in the
handlers: no allocation, no locks, no Qt there. Windows (W2): `CrashGuardWin.cpp`, the same rule for its writer thread and
watchdog (fixed buffers, Windows calls only); reports in `%LOCALAPPDATA%\Quillbinder\CrashReports` with a `.dmp` beside.
