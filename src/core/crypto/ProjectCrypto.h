#pragma once
// The encrypted project and the encrypted backups (0.99.0, to-do item 113, part 2): the two switches of a project, its
// plain marker file, turning the project's encryption on and off, and what an opening needs (unlock, or the message
// with Import Keys). The layer under every read and write of an encrypted project is core/crypto/Vault.h.
//
// The writer's decisions (item 113):
//   1. Encrypted backups: each backup is "<name>.zip.gpg" (OpenPGP, to the writer's key and the recovery key); writing
//      one needs only the public keys; both project formats. Restore From Backup decrypts it (the passphrase is asked
//      when the key is not unlocked yet). Plain backups stay readable.
//   2. Encrypted project: every file of the project folder is encrypted on its own (texts, notes, the project file,
//      snapshots, conflict copies, the plain texts for search and counts, the save journal, the waiting files, settings,
//      objects, mind maps, pictures and media), so that sync programs, the lock and the conflict copies keep working per
//      file. Needs the writer's secret key on each device. Quillbinder's own format only (Scrivener cannot read
//      encrypted files). Two files stay plain: the marker (encryption.json, below) and the lock (settings/lock).
// An encrypted project's backups are always encrypted (switch 1 is on and cannot be switched off while switch 2 is).
// Every file is also encrypted to the recovery key; Quillbinder keeps only its public part.
//
// Where the switches are kept: the marker file "encryption.json" at the top of the project folder (for a project in
// Scrivener format: of its working copy, where the program keeps what only it knows). No marker: both switches off.
#include "core/crypto/OpenPgp.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <functional>
#include <mutex>

class QWidget;

namespace qb {
class Project;
namespace crypto {

// The two keys a project is encrypted to: primary fingerprints (40 hex digits, as KeyInfo::fingerprint). Both public
// keys must be in KeyRing::device() to encrypt.
struct Recipients {
    QString writer;   // the writer's key: its secret part opens the project on each device
    QString recovery; // the recovery key: Quillbinder keeps only its public part
    bool isComplete() const;
    QStringList list() const; // {writer, recovery}
    bool operator==(const Recipients &o) const { return writer == o.writer && recovery == o.recovery; }
    bool operator!=(const Recipients &o) const { return !(*this == o); }
};

// "encryption.json"
QString markerName();
QString markerPath(const QString &projectFolder);

// What the marker of a project folder says (no marker, or one that cannot be read: all off; *error says why it could
// not be read)
struct ProjectSettings {
    bool encryptedProject = false; // switch 2 (also while a conversion to encrypted files is under way)
    bool encryptedBackups = false; // switch 1 (always true when encryptedProject is)
    Recipients recipients;
    // a conversion that was cut off (a crash, the power, an error): "encrypt" or "decrypt"; "" = none. The next opening
    // finishes it; meanwhile the folder holds plain and encrypted files, and both are read.
    QString converting;
    QString convertingDevice; // which computer began it (its key, as the lock names it), and when (UTC, ISO 8601)
    QString convertingSince;
    // 0.99.0 (encryption review, M3): the marker is there but cannot be read (damaged, or not Quillbinder's): not "all off"
    bool unreadable = false;
    bool isEncryptedFolder() const { return encryptedProject || !converting.isEmpty(); }
};
ProjectSettings settingsOf(const QString &projectFolder, QString *error = nullptr);
// "" when the folder's marker can be trusted; else the sentence why not: the marker is there but cannot be read, or it is
// missing although the project file is encrypted (0.99.0, encryption review M3). Such a project is neither opened nor
// backed up (a backup would be plain, and the opening would write into files it cannot read).
QString markerProblem(const QString &projectFolder);
// A recipient's secret key is unlocked on this device now (no passphrase is asked for, the key store is not read)
bool unlockedHere(const Recipients &recipients);
// The base folder for a file that must be plain for a moment (a preview, an export, a dropped picture): the private folder
// of the vault while an encrypted project is open ("" when it cannot be made), else the system's temporary folder
QString tempFolderBase();

// ---- switch 1: encrypted backups (both project formats) -------------------------------------------------------------
bool backupsEncrypted(const Project *p);
// false with the reason when switch 1 cannot be changed now (an encrypted project: its backups are always encrypted)
bool canChangeBackupEncryption(const Project *p, QString *reason);
// on: the recipients must be complete and both public keys in KeyRing::device(). Writes the marker (plain); takes effect
// with the next backup. Off: the marker goes (or keeps only what switch 2 needs).
bool setBackupsEncrypted(Project *p, bool on, const Recipients &recipients, QString *error);

// ---- switch 2: encrypted project (Quillbinder's own format only) ----------------------------------------------------
bool projectEncrypted(const Project *p);
// false with the reason for the greyed switch: a project in Scrivener format, a build without encryption, a save of
// another computer that is still arriving (its journal), another computer or window that has the project open (its lock)
bool canChangeProjectEncryption(const Project *p, QString *reason);
// The recipients of a project (from its marker; empty when neither switch is on)
Recipients recipientsOf(const Project *p);

// Turning switch 2 on or off works on the folder of a project that is NOT open (the window saves and closes it, converts,
// and opens it again: core/ui helper ui::changeProjectEncryption in ui/ProjectCryptoUi.h does all of it with a progress
// window). Every file is converted on its own: written whole under a temporary name beside it ("<file>.qbconvert"),
// synced, and renamed over it in one step, so that each file is always either the old or the new version. The marker is
// written (and synced) with the direction before the first file and gets its final state only after the last one; an
// opening finishes a conversion that was cut off. progress(done, total): false stops between two files (the project
// stays openable and the next opening finishes it).
using Progress = std::function<bool(int done, int total)>;
bool encryptFolder(const QString &projectFolder, const Recipients &recipients, const Progress &progress, QString *error);
// needs the secret key of one of the recipients, unlocked
bool decryptFolder(const QString &projectFolder, const Progress &progress, QString *error);
// finishes a conversion that was cut off ("" error and true when there was none)
bool finishConversion(const QString &projectFolder, const Progress &progress, QString *error);

// ---- opening --------------------------------------------------------------------------------------------------------
enum class Access {
    Plain,           // not encrypted: nothing to do
    Ready,           // encrypted, and a recipient's secret key here is unlocked
    NeedsPassphrase, // encrypted; a recipient's secret key is here but locked (*fingerprint: which one); the key store
                     // had no passphrase for it (or not the right one)
    NoSecretKey,     // encrypted; no recipient's secret key is on this device: Import Keys (*fingerprint: the writer's)
    Unavailable,     // encrypted, but this build has no encryption
    // 0.99.0 (encryption review): refused before anything is written
    PublicKeyMissing, // encrypted; a recipient's public key is not on this device, so no save could be encrypted (*fingerprint: it)
    MarkerDamaged,    // the marker cannot be read, or is missing beside an encrypted project file (markerProblem)
};
// What opening the project folder needs. Tries the passphrase kept in the key store (KeyStore::system()) once per key
// and session when the key is locked.
Access accessFor(const QString &projectFolder, QString *fingerprint = nullptr);
// The writer typed the passphrase: unlocks the key and, when that worked, keeps the passphrase in the key store of this
// device (the writer's decision: remembered per device). The KeyStore's status in *kept (Ok, NoBackend ...).
Error unlockAndRemember(const QString &fingerprint, const QByteArray &passphrase, bool *kept = nullptr);
// The sentence for an opening that cannot go on (NoSecretKey, Unavailable ...); "" for Plain and Ready
QString accessText(Access access, const QString &projectFolder);

// ---- the UI's part (set by the window) ------------------------------------------------------------------------------
// Import Keys (enc3's window): called with the parent window and the fingerprints the project needs; true when keys
// were imported (the opening then tries again). Not set: the message has no Import Keys button.
using ImportKeysHandler = std::function<bool(QWidget *parent, const QStringList &needed)>;
void setImportKeysHandler(ImportKeysHandler handler);
const ImportKeysHandler &importKeysHandler();

// KeyRing::device() is one RNP context that is not thread-safe: code that uses it on another thread than the window's
// (the backup's check through decryption) holds this lock, and so do the vault's calls
std::recursive_mutex &deviceRingLock();

} // namespace crypto
} // namespace qb
