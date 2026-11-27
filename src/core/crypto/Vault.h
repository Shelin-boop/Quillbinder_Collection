#pragma once
// The layer under every read and write of an encrypted project (0.99.0, to-do item 113, part 2).
//
// A folder is in the vault while an encrypted project in it is open (Project::open and create add it, the project's end
// removes it), while a conversion or a copy writes into one (Scope), and for an emergency copy of an encrypted project.
// Every write of the program into such a folder goes through seal() (safesave::write, safesave::writeFile,
// safesave::copyFile, savebatch's waiting files and journal), every read of the program's own files through unseal() or
// readFile(), and a viewer that opens a file by its path (pictures, PDF, audio, video) gets plainPath(): a decrypted
// copy in a private folder that lives until the project closes. So no plain text is written into an encrypted project
// folder, and the files' names, sizes and times stay what sync programs and the conflict copies go by.
//
// A folder's plain files: the marker (encryption.json) and the lock (settings/lock), and what Scope or add() names.
// In a vault folder a plain file is still read (a conversion under way, a file another program wrote); a file is taken
// as encrypted when it starts like an OpenPGP message of Quillbinder's kind (isSealedData).
//
// Plain projects pay nothing: with no folder in the vault, every call returns at once.
#include "core/crypto/OpenPgp.h"

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QStringList>

namespace qb {
namespace vault {

// the relative paths that stay plain in a project folder: {"encryption.json", "settings/lock"}
QStringList projectPlainNames();

// sealNew false: files are read (and decrypted) as in the vault, but written plain (a conversion to plain files)
void add(const QString &folder, const QStringList &recipients, const QStringList &plainNames = projectPlainNames(),
         bool sealNew = true);
// one add() is undone (a folder added twice stays until both are removed); the decrypted copies of plainPath go with
// the last one
void remove(const QString &folder);
bool active(); // some folder is in the vault
// 0.99.0 (audit AU-01): locks the key at once when no encrypted project is open, else when the last one is removed
void lockWhenIdle(const QString &fingerprint);
bool contains(const QString &folder);

struct Place {
    bool found = false;     // the path is inside a vault folder
    bool plainName = false; // ... and one of its plain files
    bool sealNew = true;
    QString folder;
    QStringList recipients;
    bool reads() const { return found && !plainName; }            // what is read there may be encrypted
    bool seals() const { return found && !plainName && sealNew; } // what is written there is encrypted
};
Place placeOf(const QString &path);
bool seals(const QString &path);

// whether data starts like an OpenPGP message as Quillbinder writes them (binary; a session-key packet of version 3)
bool isSealedData(const QByteArray &head);

// What goes on disk for `path`: data as it is, or (in a vault folder) OpenPGP to its recipients, in place. False with
// a reason (data unchanged): the public keys are not there, or this build has no encryption.
bool seal(const QString &path, QByteArray &data, QString *error = nullptr);
// What was read from `path`: an encrypted file of a vault folder is decrypted in place (nothing comes back unless its
// integrity check passed); anything else stays as it is. False with the reason and its kind (Locked, NoSecretKey,
// Damaged ...): data is then empty.
bool unseal(const QString &path, QByteArray &data, QString *error = nullptr, crypto::Error *kind = nullptr);
// A whole file read and unsealed. False: not there, not readable, read short, or not decryptable (*error, *kind)
bool readFile(const QString &path, QByteArray *data, QString *error = nullptr, crypto::Error *kind = nullptr);
// The file's text for a comparison: readFile's data, or an empty array when it fails
QByteArray readOrEmpty(const QString &path);
// A picture (QImage(path) for a plain file; decrypted in memory for an encrypted one); a null image when it fails
QImage readImage(const QString &path);
// A picture written (QImage::save for a plain place; encrypted through safesave::writeFile in a vault folder). format:
// as QImage::save (nullptr: by the file's suffix)
bool saveImage(const QImage &image, const QString &path, const char *format = nullptr);

// A copy from `from` to `to` that encrypts or decrypts as the two places need (both plain, or both encrypted to the same
// keys: the bytes as they are). The target is written whole or not at all. *handled false: neither place is in the
// vault, the caller copies as before.
bool copy(const QString &from, const QString &to, QString *error, bool *handled);

// For a viewer that opens files by path: the path itself when it is not encrypted; else a decrypted copy (under the
// same file name) in a private folder of this program run (in memory where the system keeps one: $XDG_RUNTIME_DIR on
// Linux; access for the owner only), made again when the original changes, removed when its project's folder leaves the
// vault and at the program's end. "" when it cannot be decrypted (*error).
QString plainPath(const QString &path, QString *error = nullptr);
// The same for a file with the files of its folder (a saved web page with its pictures beside it): the folder copied
// decrypted into the private folder (removed with the project's folder); the copy of `path` in it
QString plainTree(const QString &path, QString *error = nullptr);
// removes every decrypted copy (the program's end; folders of program runs that ended without it too)
void removePlainCopies();
// that private folder of this program run ("" when it cannot be made), for a file that must be plain for a moment
QString privateFolder();

// Adds the folder while it lives when its marker says it is encrypted (a copy of a project, a restored backup, a folder
// that is converted): for code that writes into a project folder that is not open
class Scope
{
public:
    explicit Scope(const QString &projectFolder);
    ~Scope();
    Scope(const Scope &) = delete;
    Scope &operator=(const Scope &) = delete;
    bool encrypted() const { return m_added; }

private:
    QString m_folder;
    bool m_added = false;
};

} // namespace vault
} // namespace qb
