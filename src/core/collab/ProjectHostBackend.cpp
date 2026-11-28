#include "core/collab/ProjectHostBackend.h"

#include "core/DocumentStore.h"
#include "core/Objects.h"
#include "core/Project.h"
#include "core/SafeSave.h"
#include "core/collab/Gate.h"
#include "core/collab/OpApply.h"
#include "core/collab/PathClass.h"
#include "core/crypto/Vault.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>

namespace qb {
namespace collab {

ProjectHostBackend::ProjectHostBackend(const QString &folder) : m_folder(folder) {}
ProjectHostBackend::~ProjectHostBackend() = default;

bool ProjectHostBackend::open(QString *error)
{
    ServiceScope scope;
    m_project = Project::open(m_folder, error);
    return m_project != nullptr;
}

QStringList ProjectHostBackend::derivedPaths() const { return {QStringLiteral("contents-by-title.txt")}; }

bool ProjectHostBackend::afterCommit(const OpList &ops, const QStringList &putPaths, const QStringList &removedPaths, QString *error)
{
    if (!m_project) {
        if (error)
            *error = QStringLiteral("the host's project is not open");
        return false;
    }
    ServiceScope scope; // the writes below are the service's, not a window's
    QString err;
    bool structural = false;
    for (const Op &op : ops)
        structural = structural || op.kind != Op::ItemFields;
    bool reopen = false;
    if (!ops.isEmpty() && !applyToProject(m_project.get(), ops, &err))
        reopen = true; // the model could not follow: it is made again from the file the host wrote
    QSet<QString> docIds, noteIds;
    bool objects = false;
    for (const QString &p : putPaths + removedPaths) {
        const PathInfo info = classify(p);
        if (info.kind == PathKind::Text || info.kind == PathKind::MediaFile)
            docIds.insert(info.id);
        else if (info.kind == PathKind::Notes)
            noteIds.insert(info.id);
        else if (info.kind == PathKind::Object || info.kind == PathKind::ObjectSchema || info.kind == PathKind::ObjectImage)
            objects = true;
    }
    for (const Op &op : ops)
        if (op.kind == Op::ItemCreate)
            docIds.insert(op.id);
    if (reopen) {
        m_project.reset();
        m_project = Project::open(m_folder, &err);
        if (!m_project) {
            if (error)
                *error = err;
            return false;
        }
        structural = true;
    } else {
        m_project->acknowledgeIndexOnDisk();
        DocumentStore *store = m_project->docs();
        if (store) {
            for (const QString &id : docIds | noteIds)
                if (m_project->item(id))
                    store->reload(id);
            for (const QString &id : docIds)
                if (m_project->item(id))
                    store->recount(id);
        }
        if (objects && m_project->objects())
            m_project->objects()->load();
    }
    QString saveErr;
    bool ok = m_project->save(&saveErr);
    if (!ok && error)
        *error = saveErr;
    // the list of documents follows the binder: it is written whenever the structure changed
    if (structural || !QFileInfo::exists(m_project->titleListFile())) {
        const QByteArray list = m_project->titleListText().toUtf8();
        QByteArray cur;
        if (!(vault::readFile(m_project->titleListFile(), &cur) && cur == list)) {
            QString why;
            if (!safesave::writeFile(m_project->titleListFile(), list, &why) && error && ok)
                *error = why, ok = false;
        }
    }
    return ok;
}

} // namespace collab
} // namespace qb
