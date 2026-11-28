#pragma once
// Live Sync, applying operations (0.99.0): the one code that the host uses to apply a commit to its project and that a
// window uses to apply a remote commit, so that the models stay equal. Two layers: the project file as XML (what the
// replica's file becomes, and what the tests compare), and a Project (the binder model and the settings of a window).
#include "core/collab/IndexDiff.h"

#include <QByteArray>
#include <QString>

namespace qb {

class Project;

namespace collab {

// Applies `ops` in order to the project file `index` (its XML); all or nothing. An operation that names an item that is not
// there, a parent that is not there, or an item moved into itself fails the whole call with a reason. Attributes the host
// computes (modified, words, chars) stay as they are in `index`; a created item starts with them at their defaults.
bool applyToIndex(QByteArray *index, const OpList &ops, QString *error = nullptr);

// Applies `ops` to the project in memory: items through its binder model (views update through the model's signals),
// sections through Project::applySectionsFromIndex. Marks nothing as changed (Project::beginRemoteApply). A document
// deleted by an operation has its files removed by the model as usual. False with a reason at the first operation that
// cannot be applied (the ones before it stay applied: the caller then loads the replica's file again).
bool applyToProject(Project *project, const OpList &ops, QString *error = nullptr);

// The binder ids that `ops` touch (for the window: which documents to reload, which rows to look at)
QStringList touchedIds(const OpList &ops);

} // namespace collab
} // namespace qb
