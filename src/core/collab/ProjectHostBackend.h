#pragma once
// Live Sync, the host's Project (0.99.0): a HostBackend that keeps the authoritative, headless Project of the hosted folder.
// When the host applies a commit it writes the files and the project file as the commit says (HostSession); this class then
// takes the new state into its Project (the operations through the binder model, the changed documents read again), lets the
// Project's own code recompute what depends on it (word and character counts, modified times, the list of documents with
// their titles, written to the project file and contents-by-title.txt), and the host sends the results to every replica as
// files like any others. The Project is never changed by anyone else: the windows of the host's device are followers of the
// service, like every other device's.
#include "core/collab/HostSession.h"

#include <memory>

namespace qb {
class Project;
namespace collab {

class ProjectHostBackend : public HostBackend {
public:
    explicit ProjectHostBackend(const QString &folder);
    ~ProjectHostBackend() override;
    // opens the Project (false with a reason when it cannot be read)
    bool open(QString *error = nullptr);
    Project *project() const { return m_project.get(); }

    QStringList derivedPaths() const override;
    bool afterCommit(const OpList &ops, const QStringList &putPaths, const QStringList &removedPaths, QString *error) override;

private:
    QString m_folder;
    std::unique_ptr<Project> m_project;
};

} // namespace collab
} // namespace qb
