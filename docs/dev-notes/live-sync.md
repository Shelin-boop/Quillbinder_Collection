# Live Sync (0.99.0): how it works

Live Sync keeps one project the same on several devices of one network while they are connected. It is built into the program
(`src/core/collab/`, compiled into `qbcore`), off by default, and with it off nothing of it runs. This note says how the parts fit and
why; the details of each part are in the header of its files and in `live-sync-authority.md` (who hosts). The suites are `ls1` to `ls8`
(`test-suites.md`).

## The parts

- **Trust** (`collab/net/`): every device has a key pair and a self-signed certificate; the device id and the pin are made from the
  public key. Links are TLS 1.3 (Botan) over `QTcpSocket` with a certificate on both sides, and a peer is trusted only by its pin:
  a paired device (the list in `PeerStore`), a member of the project (the signed membership), or, during the 120-second pairing
  window, any key (the session is then unauthenticated until the two people have compared the six digits). The device key is kept in
  the key store (the system's, else the Quillbinder wallet). Frames are `[length][kind][payload]` with hard limits (1 MiB of JSON, 256 KiB
  of data, 200 control frames a second); a link that breaks a limit is closed with its error code. Discovery is a small multicast DNS
  of our own; it announces an opaque instance id, never a device name.
- **Protocol** (`collab/proto/`, `HostSession`, `ClientSession`): one host per project, any number of clients. A client keeps a replica
  folder, equal to the host's folder except for device-local files, by events (`COMMIT_EVENT`), by replay of the host's log, or by
  comparing manifests. A change is a request of units (operations on the project file by stable item id, and files named by the hash of
  bytes sent beforehand as blobs). The host checks paths, roles, locks and revisions, applies all of it or nothing, writes the files, its
  state and a log record in one `SaveBatch`, and only then answers and tells the others. A request has an operation id; the host remembers
  the answers, so a request sent twice is done once.
- **Locks** (`Locks`): a document, the structure, the settings, an object or the whole project; leases renewed by heartbeats; every
  grant has a generation (a counter that survives restarts by reserving blocks), and a commit with an older token is refused. An edit
  request asks the holder; a decline restarts the 20 seconds; only after them can the asker take over, and a protective snapshot comes
  first.
- **The write gate** (`Gate`, `Capture`): a window of a shared project never writes the shared files. Everything that reaches
  `safesave`, `savebatch`, the vault and the remove helpers asks the gate, which keeps the write (plain bytes, before sealing) as a change
  set of the follower. The source audit in `ls3` lists every place that writes a file and how it gets to the gate.
- **Who hosts** (`Authority`, `Membership`, `Election`, `Handover`, `Drafts`): signed membership and proofs, planned handover, failover
  by priority with a majority, recovery drafts at reunion.
- **The interface** (`src/ui/LiveSync*`) and the **service** (`Service`, one per device, windows are its followers).

## Backups and snapshots as host jobs (`collab/HostBackup.*`, work package 8)

- **Only the host backs up.** `HostBackup` sits beside the `HostSession` of a project and the host's own `Project`. The service gives
  `HostSession::setExtensionHandler` a function that calls `HostBackup::handle` for the types `BACKUP_REQUEST` and `SNAPSHOT_OP`
  (and chains the other packages' types), and `HostSession::setSnapshotHook(jobs->protectiveSnapshotHook())`.
- **The copy at one commit.** `HostBackup::freeze` runs in the host's own thread, so no commit can come in the middle of it (the host
  commits in that thread; a host that does not pauses its queue through `Config::pauseCommits`). It reads the commit number, links or
  copies every file except Live Sync's own folders, the journal, staged and lock files into `livesync/freeze-<seq>/`, and counts the
  copy as good only when no save was under way and the number in memory and in `livesync/state.json` is still the one it began with
  (else it tries again, then says "busy"). Hard links are safe because the program only ever replaces a file by renaming a new
  one over it: a linked file keeps its old content. Where links fail (another drive, a file system without them) files are copied, after
  a check that the project and 64 MiB fit. Symbolic links are followed (as the ZIP code does), links to nothing are left out.
- **Packing.** `backup::backupProject` got an optional `FrozenView`: it packs that folder under the project's name and identity and does
  not save the project first. While it packs, `keepAlive` runs every 50 ms: the host's event loop goes on, so commits are not held up
  by a large backup. The copy is removed afterwards (also after a crash: `removeOldFreezes`). The result becomes the state: done;
  incomplete (files could not be read: named so, no older backup is removed); waiting (made, but the backup folder cannot take it: it
  stays in the work folder, as always); failed. The last success only moves for a complete backup. The state is kept in
  `livesync/backup-state.json`; a state "running" found at the start is "failed: interrupted".
- **What a client sees.** `BACKUP_REQUEST {query?}` is answered with `BACKUP_STATUS {state, lastOk, seq, code, detail}`; the host
  tells every connection that asked when the state changes. The client turns the reason code into its own language
  (`backupReasonText`); the host's own words are only a fallback. A device may ask once in ten seconds (HOST_BUSY), a device without a
  role is refused. Readers may ask: a backup changes nothing.
- **Snapshots.** In a window of a shared project `snapshots::take/setTitle/remove/rollBack` call a hook (`snapshots::RemoteJobs`, set
  per project folder by the service from `HostJobsClient::remoteJobs()`) which sends `SNAPSHOT_OP` and waits for the answer and for the
  copy to reach the commit. Without a hook a follower cannot change snapshots. On the host `HostBackup::runSnapshotJob` checks the
  role (editors), names (a document id and a snapshot file name by their pattern; the file is looked up in the host's own list, never
  built into a path), and locks (take, rename and delete: no other device's conflicting lock; roll back: the asker's own lock), runs the
  usual `snapshots::` function on the host's `Project` (a roll back first keeps the text as a snapshot of its own), saves, and then
  `publishChanges`: it scans the folder against the host's manifest, raises the revision of every resource that changed, writes state and
  log in one batch and sends one `COMMIT_EVENT`. `HostBackup` is a friend of `HostSession` for this one purpose (its private state:
  seq, revisions, manifest, log, peers); if `HostSession`'s commit tail changes, `publishChanges` must follow.
- **A job cut off.** Before a job writes, `livesync/job-pending.json` is written; it goes after the commit. At the next start
  `recoverJobs` publishes what is on disk. The answers of the last 1,000 jobs are kept in memory (a job sent twice is done once; after a
  restart a repeated "take" would make a second snapshot, which is harmless).
- **Restore** is only a new project: `backup::restore` gives it a new backup id and now also removes any `livesync*` a hand-made ZIP
  held. The ZIP code (`Zip.cpp`) leaves `livesync/`, `livesync-local/` and `livesync-replica.json` out of every archive.
- **Pitfall.** The host's `Project` is headless and must follow the folder: the service's `ProjectHostBackend` takes each commit into
  it. A job reloads the document it touches (`DocumentStore::reload`) and saves the project; a Project that has not seen a change of the
  project file would write its older version over it.

## The security sweep (suite `ls8`)

The checks name what a hostile peer can try and what must happen: an unpaired device gets no session; a host with another certificate
than the pinned one is refused (`IDENTITY_MISMATCH`); an oversize frame is refused as soon as its length is read; an unknown frame kind,
and more than 200 control frames a second, close the link; a snapshot job that names another place than a document id and a snapshot's
file name does nothing; the same message twice is `DUPLICATE`; the same job in a new message gets the stored answer; a host's event with
a path that leaves the project is not carried out; a manifest with such a path, two paths that differ only by case, Live Sync's own files,
too many entries or no list is refused; a blob whose bytes do not match its hash is not kept. Paths in change requests, case collisions,
reserved names and links on the way are in `ls4`.

## Measurement

`qb_ls8_tests --measure <folder>`: the test program is the host of two projects (100 documents each, 1 KB to 1 MB, about 60 MB each)
over two TLS listeners on localhost, and starts itself twice: a typer (edits spaced 350 ms apart in 14 documents per project, then 50
changes at 10 a second into one document per project, then 20 fresh locks) and a watcher (each change carries the time of the monotonic
clock, which is the same in all programs of a machine; the watcher reads it from its replica when the event is applied and notes the
difference). It writes `results.txt` with p50, p95, p99 and the maximum. The numbers depend on the machine; they are recorded, not promised.

## Finishing the integration (work package 9)

- **Per hosted project** the service keeps a `HostBackup` (extension handler chained after the authority's, the snapshot hook, `recoverJobs` at
  start) and per replica a `HostJobsClient` (`Service::Proj::jobs` / `jobsClient`). `HostBackup` gets the host's Project through
  `Config::projectOf`: `ProjectHostBackend` makes the Project again when its model could not follow a commit, so a pointer kept from the
  start would be stale and a snapshot job's `Project::save()` would write an older project file over a newer one (check in `ls9`).
  Every entry point of `HostBackup` runs inside a `ServiceScope` (the gate would capture the host's writes when a window of the same
  folder lives in the process); the event pump while a backup is packed steps out with a `WindowScope`.
- **Windows reach the snapshot jobs** through the service: `W_JOB` / `W_JOB_RESULT` (`ServiceClient::snapshotJob`); `LiveSyncGlue` installs
  a `snapshots::RemoteJobs` per project folder. On the host's own device the service calls `HostBackup::runJobFor`.
- **A restart and a cut-off job**: `HostSession::start` reads the folder, so the job's files are already in the host's list; `recoverJobs`
  therefore publishes every snapshot file and the project file again (`publishChanges(..., afterRestart)`).
- **The first message of a window**: the service's answer to `W_ATTACH` and a `W_STATE` can arrive in one read; `ServiceClient::onFrame`
  takes the answer in at once (before: the state was dropped as "not my project" and the window never saw the host ready; it made
  `ls5`'s process check fail in about one run of five).
- **One window, one service**: `MainWindow::liveSyncService()` keeps the in-process service the window was made with. Suites run several
  services in one process and switch `ui::setInProcessLiveSyncService`; a reopen (after a replaced copy) that read the global opened the
  window without its follower, so its writes went straight into the host's folder.
- **Edit requests**: `EDIT_ENDED` (host to holder) when the asker cancels; the service ends the holder's bar on a hand-over, an unlock of
  the resource and a revoked lock, not on a Decline. After a takeover the window saves the document at once: without the lock the host
  refuses the text and it is kept as an outbox draft.
- **Drafts**: `Service::draftContent` reads the text of an authority draft or of an outbox draft (the first text file of its units, bytes
  from the blob store); `LiveSyncControl::draftActionRequested(DraftAction *)` goes to the window that has the project open
  (`MainWindow::liveSyncDraftAction`); none: the dialog says to open the project. Take Over keeps the draft in the list.
  Compare writes the draft's text to a temporary `.txt` and uses `compare::compareTexts` for that one document; text that shares no
  four-word run with the document is shown as a section of its own (nothing is changed until the writer ticks it).
- **Keep hosting**: `ServiceControl::handOffHosting` (on `aboutToQuit` of a program that owns its service) stops the in-process service and
  starts `--live-sync-service`; the process reads the choice from the settings (`QB_LS_KEEP_HOSTING=1` in the suites). A program that
  starts finds the socket taken and sends `W_STOP`; only a service process (`quitWhenIdle`) answers "stopping" and ends, then the program
  hosts itself. A failed `Service::start` closes its listener and discovery again.
- Pitfall: `QB_LS5_SCREENS_FULL` is gone, the second half of the screens test always runs.
