#pragma once
// The inspector's Live Sync tab and the bars above the editor (0.99.0, work package 7). Both read the StatusModel and act
// through StatusActions / LiveSyncControl; neither looks into a session.
#include "ui/LiveSyncControl.h"

#include <QFrame>
#include <QWidget>

#include <functional>

class QLabel;
class QPushButton;
class QTreeWidget;
class QVBoxLayout;

namespace qb {
namespace livesync {

// what the interface needs to turn a document id into a title (the window's project); returns "" for an unknown id
using TitleResolver = std::function<QString(const QString &docId)>;

// "<title>" for "doc:<id>", else a short description of the resource ("the binder's structure", ...)
QString resourceText(const QString &resource, const TitleResolver &titles);
// the lock resource of a document
inline QString docResource(const QString &docId) { return QStringLiteral("doc:") + docId; }

// ---------------------------------------------------------------- the inspector tab
class LiveSyncTab : public QWidget
{
    Q_OBJECT
public:
    explicit LiveSyncTab(QWidget *parent = nullptr);
    void setContext(const QString &uuid, const QString &docId, const TitleResolver &titles);
    void refresh();

private:
    QString m_uuid, m_docId;
    TitleResolver m_titles;
    QLabel *m_summary = nullptr;
    QTreeWidget *m_people = nullptr;
    QLabel *m_backup = nullptr;
    QPushButton *m_requestBackup = nullptr;
    QLabel *m_drafts = nullptr;
    QPushButton *m_openDrafts = nullptr;
    QLabel *m_doc = nullptr;
    QPushButton *m_ask = nullptr;
    QPushButton *m_take = nullptr;
    QPushButton *m_stop = nullptr;
};

// ---------------------------------------------------------------- the bars
// A column of bars that do not block anything. Empty: hidden.
class LiveSyncBars : public QWidget
{
    Q_OBJECT
public:
    explicit LiveSyncBars(QWidget *parent = nullptr);
    void setContext(const QString &uuid, const QString &docId, const TitleResolver &titles);
    void refresh();
    int barCount() const;
    // a bar's text by its object name ("barAsked", "barWaiting", "barReadOnly", "barOffline", "barProblem", "barAccess")
    QString barText(const QString &name) const;

private:
    QFrame *makeBar(const QString &name, const QString &text, const QList<std::pair<QString, std::function<void()>>> &buttons);
    QString m_uuid, m_docId;
    TitleResolver m_titles;
    QVBoxLayout *m_lay = nullptr;
    // requests of other devices for documents this device holds: key "<uuid>|<resource>" -> asker
    QHash<QString, QString> m_asked;
};

} // namespace livesync
} // namespace qb
