#pragma once
// Live Sync's manifest (0.99.0), section 3.5: the list of the files of a project that are replicated, with size, SHA-256 of the
// stored bytes (for an encrypted project: of the OpenPGP files as they are on disk) and the revision of the resource the file
// belongs to. A copy is made and repaired by comparing manifests.
#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace collab {

struct ManifestEntry {
    QString path;
    qint64 size = 0;
    QString sha256; // hex
    qint64 rev = 0;
};

constexpr int kManifestFormat = 1;
constexpr int kManifestMaxEntries = 200000;

struct Manifest {
    qint64 epoch = 0;
    qint64 seq = 0;
    int format = kManifestFormat;
    QList<ManifestEntry> entries;           // sorted by path
    QHash<QString, qint64> revs;            // the revision of every resource that has one (the files' revisions come from here)

    const ManifestEntry *find(const QString &path) const;
    QJsonObject toJson() const;
    // false with a reason when the object is no manifest, a path is not acceptable or not replicated, or two paths differ only
    // by case
    static bool fromJson(const QJsonObject &o, Manifest *out, QString *why = nullptr);
    // SHA-256 (hex) of the sorted lines "path\0size\0sha256\n": what two devices compare to know they hold the same files
    QString stateHash() const;

    // The replicated files of `folder`. `cacheFile` (may be empty) keeps hashes by size and modification time, so that only
    // changed files are read again. Symbolic links are never followed or listed.
    // SHA-256 (hex) of a file's bytes and its size; empty when it cannot be read
    static QString fileSha256(const QString &path, qint64 *size);
    static bool scan(const QString &folder, const QString &cacheFile, const QHash<QString, qint64> &revs, Manifest *out, QString *error = nullptr);
};

struct ManifestDiff {
    QStringList fetch;  // paths to get: missing here, or another hash
    QStringList remove; // replicated paths here that the other does not list
};
// what `local` has to do to become `wanted`
ManifestDiff compare(const Manifest &wanted, const Manifest &local);

} // namespace collab
} // namespace qb
