#pragma once
// Live Sync, from a follower's change set to the units of a change request (0.99.0, section 4.3 of the design). A window of a
// shared project never writes the shared files: what its save would have written was captured (collab/Capture.h). This turns
// one captured set into units: the captured project file is compared with the one in the folder and gives item and section
// operations (the file itself is never sent), the other files become file operations (their bytes travel as blobs by hash).
// Things that belong together share a unit: every operation and file names the resources it needs (a document, the structure,
// the settings ...); resources that one operation or file joins are one unit, so a request never splits a created document from
// its text. A change the host alone makes (derived files, snapshots, the encryption marker) is left out and listed.
#include "core/collab/Capture.h"
#include "core/collab/proto/Messages.h"

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {

struct UnitsResult {
    bool ok = true;
    QString error;                    // when !ok: the project file could not be compared
    QList<proto::Unit> units;
    QHash<QString, QByteArray> blobs; // plain bytes by SHA-256 (hex)
    QStringList skipped;              // paths a client does not send
    bool isEmpty() const { return units.isEmpty(); }
};

// `folder` is the project folder as the service keeps it (its project file is the base of the comparison)
UnitsResult unitsFromChanges(const ChangeSet &set, const QString &folder);

} // namespace collab
} // namespace qb
