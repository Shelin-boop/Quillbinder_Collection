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

// Switch 2 for the window's project: saves it, closes it, encrypts or decrypts its folder with a progress window, and
// opens it again (also when the conversion failed: it then opens in its mixed state, and the next opening finishes it).
// on: recipients must be complete. False with *error (nothing changed when the save or the close failed).
bool changeProjectEncryption(MainWindow *window, bool on, const crypto::Recipients &recipients, QString *error);

} // namespace ui
} // namespace qb
