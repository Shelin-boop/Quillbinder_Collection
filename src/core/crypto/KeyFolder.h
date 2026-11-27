#pragma once
// The key folder of the built-in encryption (0.99.0, to-do item 113, part 3): "Quillbinder Keys yyyy-mm-dd" with the
// writer's key and the recovery key as .asc files and a README.txt without any secret, the check that reads it back,
// the import on another device, the import of a key file (e.g. from Kleopatra) and the export. Qt Core only; the
// windows (the wizard, File > Options > Encryption) are in src/ui/EncryptionPage.*.
//
// The recovery passphrase is never written anywhere by this code: the caller shows it once, to be written on paper.
// The writer's passphrase is kept only in the system's key store (KeyStore), and only when the writer lets it.
#include "core/crypto/OpenPgp.h"

#include <QDate>
#include <QString>
#include <QStringList>

namespace qb {
namespace crypto {
namespace keyfolder {

// the file names of the folder
inline QString myKeySecretName() { return QStringLiteral("my-key-secret.asc"); }
inline QString myKeyPublicName() { return QStringLiteral("my-key-public.asc"); }
inline QString recoverySecretName() { return QStringLiteral("recovery-key-secret.asc"); }
inline QString recoveryPublicName() { return QStringLiteral("recovery-key-public.asc"); }
inline QString readmeName() { return QStringLiteral("README.txt"); }
// "Quillbinder Keys 2026-11-27"
QString folderName(const QDate &date);

// which keys of this device are the writer's key and the recovery key (a small file next to the key ring)
struct Roles {
    QString mine;     // fingerprint of the writer's key
    QString recovery; // fingerprint of the recovery key (its public part)
    bool complete() const { return !mine.isEmpty() && !recovery.isEmpty(); }
};
Roles roles(const QString &ringDir = QString()); // default: KeyRing::deviceDir()
bool setRoles(const Roles &r, const QString &ringDir = QString());

// the name of the sync program a path belongs to ("OneDrive", "Dropbox", "Nextcloud", "Google Drive", "iCloud",
// "Syncthing", "pCloud", "MEGA"), or "" (a folder name of the path starts with the program's name)
QString syncProgramIn(const QString &path);
// "" when keys may be written into this folder; else the sentence why not (a project's folder, the backup folder, a
// folder a sync program keeps). projectFolders: the open project's and the recent projects' folders.
QString refusedPlace(const QString &folder, const QStringList &projectFolders, const QString &backupFolder);

// README.txt: the program's language (one translatable text per paragraph); no secret, only the fingerprints
QString readmeText(const QString &myFingerprint, const QString &recoveryFingerprint, const QDate &date,
                   bool withRecoverySecret = true);

// makes the writer's key and the recovery key in ring, each locked by its own passphrase
struct NewKeys {
    QString mine, recovery; // fingerprints; empty on failure
    QString error;
};
NewKeys makeKeys(KeyRing &ring, const QByteArray &myPassphrase, const QByteArray &recoveryPassphrase);

// writes the folder "Quillbinder Keys yyyy-mm-dd" into parent (a name taken: " (2)", ...): my-key-secret.asc,
// my-key-public.asc, recovery-key-public.asc, README.txt and, when withRecoverySecret, recovery-key-secret.asc (an
// export from a device that has no recovery key's secret part leaves it out). Secret files: owner only.
struct Written {
    bool ok = false;
    QString folder; // the path of the new folder
    QString error;
};
Written writeFolder(const KeyRing &ring, const Roles &keys, const QString &parent, const QDate &date,
                    bool withRecoverySecret = true);

// reads the folder back into fresh key rings and decrypts a test file with each key
struct CheckReport {
    bool filesRead = false;     // every file is there and holds the key it should
    bool myKeyWorks = false;    // a test file encrypted to both keys opens with the writer's key and its passphrase
    bool recoveryWorks = false; // ... and with the recovery key and its passphrase
    bool readmeOk = false;      // README.txt is there
    QString error;
    bool ok() const { return filesRead && myKeyWorks && recoveryWorks && readmeOk; }
};
CheckReport checkFolder(const QString &folder, const QByteArray &myPassphrase, const QByteArray &recoveryPassphrase);

// a second copy of the whole folder in another place (refused places as above are the caller's to check)
Written copyFolder(const QString &folder, const QString &parent);

// Import Keys on another device: the folder's writer's key (secret and public) and the recovery key's public part go
// into ring when the passphrase opens the writer's key. The recovery key's secret part is never taken.
struct Imported {
    bool ok = false;
    Error code = Error::None;
    QString error;
    QString mine, recovery; // fingerprints taken
    QStringList notes;      // what was left out and why
};
Imported importFolder(KeyRing &ring, const QString &folder, const QByteArray &passphrase);
// Import a Key File: one .asc (or binary) file with a key. A key with a secret part is the writer's key (`mine`; its
// passphrase must open it); a key without is `recovery` (the caller decides what a public key is for). Others than v4
// Curve25519 are refused with the layer's sentence.
Imported importKeyFile(KeyRing &ring, const QByteArray &data, const QByteArray &passphrase);

// Export Keys: the folder as above from this device's keys (no recovery secret: it is not kept here)
Written exportKeys(const KeyRing &ring, const Roles &keys, const QString &parent, const QDate &date);

} // namespace keyfolder
} // namespace crypto
} // namespace qb
