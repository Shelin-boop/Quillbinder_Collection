# Design decisions

Some properties of Quillbinder look like faults to a reader who does not know why they are there: a warning that does not
block, a save that counts although the drive did not confirm it, a file that is not encrypted. This document records the
reasons behind such deliberate choices, so that an audit or a new contributor can tell a decision from an oversight.

Each entry says **what** the program does, **why**, what it **costs**, and **what would change** the decision. The reasons
carry a mark:

- **[recorded]**: the reason stands in the code, the manual or the developer notes (the place is named);
- **[reasoning]**: an argument, not measured.

An entry can be wrong or out of date; where the code has moved on, the code wins and this text is to be corrected.
The state described is that of version 0.93.0.

## 1. One writer at a time: the lock warns, the save protects

**What.** While a project is open, a lock file in it names the computer. Opening the project elsewhere shows who has it, with **Cancel** as the default and **Open Anyway** beside it; a lock that was not renewed for 15 minutes counts as left over (`MainWindowSync.cpp`, `confirmLock`). Every save compares the files' content with what was read: a document changed elsewhere and here is kept in both versions (a snapshot and a copy in *settings/conflicts*), and if the other version cannot be kept, nothing is saved over it (manual, "Where your work lives"). Each project has one save journal with a fixed name, and a file that is being written waits beside its target under the target's name with a fixed ending (`SaveBatch.cpp`).

**Why.**
- A lock that travels through a sync folder arrives late, and a computer that crashed or was switched off leaves its lock behind with nobody to remove it. A lock that could not be overridden would shut writers out of their own work. So the lock informs, and the writer decides. [reasoning]
- Because the lock cannot be relied on, the protection sits where it can be enforced: at the moment of writing, by comparing content and keeping the other version. [recorded: manual]
- The journal has one name on purpose. A second computer's save finds the first one's journal under that name and waits (up to 15 minutes) instead of writing over the files of a save in progress. With a name per save, the two saves would not see each other. [recorded: `Project::finishInterruptedSave`, comment of 0.81.1]
- A waiting file that carries its target's name can be told to its document after a crash, also when the save had one file and so no journal. [recorded: `SaveBatch.h`]

**Cost.** Two programs that write the same document of one project at the same moment, after Open Anyway, can still collide before the file is put in place. Since 0.93.0 a waiting file of another save that is in the way is moved aside into *settings/conflicts* and told, never written over, and a journal that cannot be read and cannot be set aside is not written over either (the save waits and says so); the gap that remains is two saves staging the same file in the same instant. This was not reproduced.

**What would change it.** For two programs on one computer, a lock of the operating system held during a save would close that gap cheaply. Across computers no such lock exists.

## 2. A drive that cannot confirm a write still counts as saved

**What.** After writing a file, the program asks the system to put it on the disk and checks the answer. The answers that mean "this file system cannot do that" (EINVAL, ENOTSUP, EOPNOTSUPP, ENOSYS) count as success; every other error fails the save (`SafeSave.h`, `SaveBatch.cpp`, `Backup.cpp`).
Since 0.93.0 the same is asked of the folder's entry after the files of a save are in place (a folder sync, `savebatch::syncFolder`). Here too, "cannot do that" counts as success (the four answers above, and EBADF, EACCES and EPERM), but a real disk error (EIO and the like) fails the save: the journal and the temporary files stay, the changes stay marked as unsaved, the message says that the drive reported an error, and the next save or opening finishes the save from the journal. A drive that answers a folder sync with an error already when a project is opened or made is recognized once (`probeFolderSync`) as one that cannot sync folders, and is remembered for the rest of the session; its saves are then not told as failed.

**Why.** Some network and FUSE folders give that answer to every such request although the write went through. Counting it as a failure would turn every save in such a folder red, for good: the writer could not work there at all, and nothing would be safer for it, because the program cannot make such a folder confirm anything. A real error on a drive that normally syncs is another matter: there the answer is information, so it is not swallowed. [recorded: comment in `SafeSave.h`, `SaveBatch.cpp`, and `docs/dev-notes/saving-and-safety.md`, 0.93.0; these answers are among the faults the system test injects]

**Cost.** In such a folder "Saved" means "handed to the file system", not "confirmed on the disk". A power cut can lose more there than on a local disk. The probe judges a drive once per session; a drive whose folder sync only starts to fail later is treated as a real error.

**What would change it.** A note, once per folder, that says so. (Help > System Information already lists a project on a network folder under "Things to look at".)

## 3. The view's state does not turn the save red

**What.** The window's layout and the place in each document are kept per computer in *settings/ui-<computer>.json*. If writing that file fails, the save still counts as saved; the file stays marked and is written with the next save (`Project::writeUiState`).

**Why.** The red sign means one thing: writing is not saved. A failed view file shown as a failed save would tell writers that their text is in danger when it is not, and a sign that gives false alarms is read less carefully on the day it matters. [reasoning]

**Cost.** After a crash the window can come back as it was some saves ago.

**What would change it.** Nothing in the reason. A separate, quiet note would be possible.

## 4. A backup enters the backup folder in one step, under its final name

**What.** A backup is packed and checked in the program's own work folder. Into the backup folder it is moved (same drive) or copied straight to its final name, never over a file that is there, then read again and compared; a copy that fails or differs is removed (`Backup.cpp`, `putInPlace`, `copyInto`).

**Why.**
- On one drive the move is a single step: a program that watches the folder never sees half a backup. [recorded]
- Programs that watch a folder (sync clients) act on file events. One file created, written and closed is what any saved file looks like, and every such program must handle it. A temporary name followed by a rename is two names for one file: more events, and more ways for a watcher to act on the wrong one. [recorded: `docs/dev-notes/saving-and-safety.md`, 0.90.3]
- The folder then holds only names the program gives its backups. That keeps the clean-up's rule simple: it counts and deletes only its own names and leaves every other file alone. The rule exists because copies made by a sync client once counted as backups and pushed real ones out of the folder. [recorded: the same notes]

**Cost.** On another drive, a watcher that reads the file before it is closed gets an incomplete backup under the final name; a watcher that waits for the close does not. The alternative, a temporary name inside the backup folder and a rename, avoids this and brings the two names back. In the one case that was examined (2026-11), the temporary name was not what caused the trouble. So the choice between the two ways is a weighing of two small risks, and the other way is defensible.

**What would change it.** A sync program shown to upload files that are still open and to keep the incomplete upload.

## 5. Projects are plain files; the program does not encrypt them

**What.** A project is a folder of ordinary text files (Markdown or RTF); a backup is an ordinary ZIP file (manual, "Where your work lives").

**Why.**
- The writing must stay readable without the program: with any editor, on any system, years from now, and on the day the program itself is broken. [recorded: manual] Encryption inside the program would make the program and a password the only way to the text. A forgotten password or a damaged key would then be a lost manuscript, and for a writer that is the worse risk than someone reading the disk. [reasoning]
- A project in Scrivener's format must stay a project Scrivener can open. [recorded]
- Safety in a sync folder works by comparing files' content and keeping versions side by side; that needs files other programs can carry and the writer can open. [reasoning]
- Encrypting the drive or the home folder (LUKS, fscrypt, BitLocker) protects everything at once, also backups, snapshots, temporary files and crash reports, without these costs. [reasoning]

**Cost.** Whoever can read the folder can read the text. Snapshots, the trash and backups keep older text after it was deleted in the document.

**What would change it.** An optional encrypted backup (not project) with a recovery key would protect backups on sticks and in the cloud without touching the first reason.

## 6. The lock file names the computer and the user

**What.** The lock holds the computer's name, the user name, the program's version and process number, two times, and a code that tells computers apart. A name of one's own can be set (File > Options > General > Saving); the user name is then left out. The lock is not in backups (manual).

**Why.** The lock exists to tell a person what to do: "open on Laptop since 14:03" sends them to the laptop. A number nobody can read cannot be acted on. The process number lets a computer see that a lock is its own and that its program is gone, so that a crash on the same computer is not shown as "open elsewhere" (`syncsafe::judge`). The code separates two computers with the same name. [reasoning from how the code uses these fields]

**Cost.** Everyone with access to the shared folder sees these names.

**What would change it.** Nothing in the purpose. The default could be a name chosen at the first start instead of the computer's name.

## 7. New project folders get the system's ordinary permissions

**What.** Folders and files are made the way every program makes them: the system's settings (the umask, the parent folder's rules) decide who may read them. A file that is replaced keeps the permissions it had (`Project::create`, `Batch::stage`).

**Why.**
- Who may read a user's files is decided by the system's owner, through the home folder's permissions and the umask. A program that forces a stricter mode of its own overrides that choice where it was made on purpose (a folder shared with a co-author, an editor's account) and breaks it without a word. [reasoning]
- On drives that keep no such permissions (FAT, exFAT, many network and sync folders) a forced mode does nothing, and promising privacy there would be false. [reasoning]
- Keeping a replaced file's permissions follows the same rule: what the owner set stays. [recorded: code]

**Cost.** On a computer with several accounts whose home folders are open to each other, another account can read a project.

**What would change it.** A choice "private project" when a project is made, which gives the folder to its owner alone where the drive can do that.

## 8. Signatures are checked when something is installed; a plug-in folder of one's own stays open

**What.** A program update is installed only with the project's signature, and an installed update is checked against its signed list of files when the program starts (`Launcher.cpp`). An official add-on is checked when it is installed. An unsigned add-on installs only from the command line with `--allow-unsigned`, and never as a program update (`Addons.cpp`, `SECURITY.md`). Libraries in the user's own plug-in folder and in installed add-ons are loaded at the start without a new check (`PluginManager.cpp`).

**Why.**
- A signature answers one question: does this come from the project? It is asked where that is decided, at installation, and for the program itself at the start, because the program is what everything else relies on. [reasoning]
- The plug-in folder exists so that people can extend the program (compile formats, importers, tools) without the project's consent. The program is open source, and an add-on is a program like any other on the computer. A check at loading would have to refuse exactly these. [recorded: `SECURITY.md`; manual, section 20]
- Whoever can put a file into that folder already acts as the writer and can read every manuscript directly. A check at loading does not hold such an attacker off (entry 9). [reasoning]

**Cost.** A library changed or planted in the data folder runs with the next start. This matters where someone else can reach the data folder: it must not lie in a shared or synced place (on Windows the data is inside the program folder).

**What would change it.** Add-ons from people a writer has no reason to trust. Those would have to run outside the program with limited rights, which is a different design.

## 9. The line that is defended: other users and outside input, not programs of the same user

**What.** The link to the writing tools is a socket only the user can reach (`ToolHost.cpp`). Archives, packages, imported files and web pages are checked: names that leave the target folder are refused, sizes are limited, packages need signatures (`Zip.cpp`, `PathSafety.cpp`, `Addons.cpp`). Nothing guards against a program that already runs as the writer, for example one that swaps a folder for a link while a backup is unpacked.

**Why.** Linux and Windows do not separate the programs of one user from each other. Such a program can read the project files, change the program's settings or attach to the running program; no check inside Quillbinder stops it, and each attempt adds complexity to the code that saves the writing. The effort goes to the lines that can be held. [reasoning; the security audit of 2026-11-20 names the same limit as its own non-goal]

**Cost.** Malware that runs under the writer's account is not stopped by the program.

**What would change it.** A packaging that the system itself confines (Flatpak with portals, for example).

## 10. A save runs in the window's thread

**What.** The automatic save turns the changed documents into files, writes them and puts them in place before the window takes the next key (`Project::saveImpl`). It writes only what changed. Backups are packed in a thread of their own.

**Why.**
- During a save nothing can change, so what lands on the disk is one consistent state, without a copy of every document and without locks around every edit. The texts are Qt documents that belong to the window's thread; a save in another thread would first need a copy of each changed document, made in the window's thread. [reasoning; not measured against the alternative]
- The automatic save of the test project took 30 to 40 ms on a two-core machine after the work of 0.60.5. [recorded: `docs/dev-notes/saving-and-safety.md`]

**Cost.** On a slow drive (a network or FUSE folder, a nearly full disk) a save can hold up typing. No figures exist for a very large project on such a drive. Help > System Information lists a long automatic-save time under "Things to look at".

**What would change it.** Measured stalls in real use. Then the writing of the files, not the turning of documents into files, would move to a thread.
