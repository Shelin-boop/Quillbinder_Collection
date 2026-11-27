#pragma once
// The two switches of the open project that File > Options > Encryption shows (0.99.0, item 113, part 3): "Encrypted
// backups" and "Encrypted project". The page does not know the encrypted project (part 2): it calls these functions,
// which the main window connects to the project layer. Not connected (all empty): no project is open for the page.
#include <QString>
#include <QStringList>

#include <functional>

namespace qb {
namespace crypto {

struct ProjectSwitches {
    std::function<bool()> projectOpen;                   // a project is open
    std::function<bool()> isScrivenerProject;            // ... in Scrivener's format (it cannot be encrypted)
    std::function<bool()> backupsEncrypted;              // the switch's state now
    std::function<bool()> projectEncrypted;
    std::function<bool(bool, QString *)> setBackupsEncrypted;   // false + the reason in *error when it failed
    std::function<bool(bool, QString *)> setProjectEncrypted;
    std::function<QStringList()> projectFolders;         // the open project's folder (keys must not be written there)
};

inline ProjectSwitches &projectSwitches()
{
    static ProjectSwitches s;
    return s;
}

} // namespace crypto
} // namespace qb
