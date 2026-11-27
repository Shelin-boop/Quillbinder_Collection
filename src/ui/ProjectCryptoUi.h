#pragma once
// The window's part of the encrypted project (0.99.0, item 113, part 2): the passphrase asked at an opening or a
// restore, the message for a key that is not on this device (with Import Keys), and turning the project's encryption
// on and off with a progress window. The switches themselves: core/crypto/ProjectCrypto.h.
#include "core/crypto/ProjectCrypto.h"

#include <QString>

class QWidget;

namespace qb {
class MainWindow;
namespace ui {

// Makes the project folder openable on this device: nothing for a plain project; else the key store, then the passphrase
// asked (until it is right or the writer cancels), or the message that the key is not here with Import Keys (the
// handler of crypto::setImportKeysHandler). True: Project::open can read the folder. what: the action, for the
// dialogs' title ("Open Project", "Restore From Backup").
bool unlockForFolder(QWidget *parent, const QString &projectFolder, const QString &what);
// The same for a fingerprint (a backup's recipients): true when that key is unlocked now
bool unlockKey(QWidget *parent, const QString &fingerprint, const QString &what, const QString &why);
// For an encrypted file (a backup "<name>.zip.gpg"): a key it is encrypted to is unlocked here now (asked as above), or
// the message that none of its keys is on this device (with Import Keys). True for a file that is not encrypted.
bool unlockForFile(QWidget *parent, const QString &file, const QString &what);
// A conversion of the folder that was cut off (crypto::ProjectSettings::converting) is finished with a progress window,
// when this device began it or the other one began it more than 15 minutes ago. False with *error when it could not be
// finished (the project still opens: both kinds of files are read).
bool finishConversion(QWidget *parent, const QString &projectFolder, QString *error);

// Switch 2 for the window's project: saves it, closes it, encrypts or decrypts its folder with a progress window, and
// opens it again (also when the conversion failed: it then opens in its mixed state, and the next opening finishes it).
// on: recipients must be complete. False with *error (nothing changed when the save or the close failed).
bool changeProjectEncryption(MainWindow *window, bool on, const crypto::Recipients &recipients, QString *error);

// Connects File > Options > Encryption (enc3's core/crypto/ProjectSwitches.h) and Import Keys (crypto::setImportKeysHandler:
// File > Options > Encryption) to this window's project. The recipients are this device's keys
// (crypto::keyfolder::roles()). Switch 2 is changed once no dialog is open any more (the page is applied while File >
// Options is open, and the conversion closes and opens the project); until then the switch shows the state asked for.
void connectWindow(MainWindow *window);

} // namespace ui
} // namespace qb
