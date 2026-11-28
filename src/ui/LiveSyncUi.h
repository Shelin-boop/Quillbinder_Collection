#pragma once
// Live Sync's part of a main window (0.99.0, work package 7): the toolbar button with its menu, the inspector tab, the
// binder's marks and the bars above the editor, all driven by the program's StatusModel (through liveSyncControl()).
// With Live Sync off, none of them shows: the button and the tab are hidden, the bars are empty, no mark is painted.
//
// How the main window uses it (work package 5):
//   auto *ls = new livesync::LiveSyncUi(this);            // once, in the constructor
//   ls->installToolbarButton(m_toolbar);                  // the button (hidden while Live Sync is off)
//   ls->installInspectorTab(m_inspector);                 // the tab (hidden while off)
//   ls->attachBinder(m_binder);                           // the marks of the rows
//   layout->insertWidget(..., ls->bars());                // above the editor area; hidden when it has nothing to say
//   ls->setProject(project);  (null when closed)          // every time the window's project changes
//   ls->setSelectedDocument(id);                          // every time the selected or shown document changes
//   connect(ls, &LiveSyncUi::openProjectRequested, this, <open the folder>);   // after Join a Project
#include "ui/LiveSyncControl.h"

#include <QIcon>
#include <QObject>
#include <QAction>
#include <QPointer>
#include <QToolBar>
#include <QToolButton>

class QAction;
class QMenu;
class QToolBar;
class QToolButton;
class QWidget;

namespace qb {

class BinderPanel;
class Inspector;
class Project;

namespace livesync {

class LiveSyncBars;
class LiveSyncTab;

// the button's icon for a state: a shape and a color (never the color alone)
QIcon stateIcon(collab::ShareState state, const QColor &ink, int side = 24);

class LiveSyncUi : public QObject
{
    Q_OBJECT
public:
    explicit LiveSyncUi(QWidget *window);
    ~LiveSyncUi() override;

    // adds the button at the end of the bar; the returned action shows and hides it
    void syncToolbarButton();
    QAction *installToolbarButton(QToolBar *bar);
    void installInspectorTab(Inspector *inspector);
    void attachBinder(BinderPanel *binder);
    QWidget *bars() const;

    void setProject(Project *project);
    void setSelectedDocument(const QString &docId);
    // the text of a document for the status model's resources
    QString documentTitle(const QString &docId) const;

    // Live Sync is on (the button, the tab and the marks show)
    bool active() const;

    QToolButton *button() const { return m_button; }
    QMenu *menu() const { return m_menu; }
    LiveSyncTab *tab() const;
    LiveSyncBars *barsWidget() const;
    // the project's uuid as the model knows it ("" when not shared)
    QString projectUuid() const;

signals:
    void openProjectRequested(const QString &folder);

public slots:
    void refresh();

private:
    void refreshButton();
    void refreshMenu();
    void refreshMarks();
    void share();
    QWidget *m_window;
    QPointer<Project> m_project;
    QString m_docId;
    bool m_marked = false;   // the binder shows our marks
    QPointer<QToolButton> m_button;
    QPointer<QAction> m_buttonAction;
    QPointer<QToolBar> m_bar;
    QMenu *m_menu = nullptr;
    QAction *m_share = nullptr;
    QAction *m_join = nullptr;
    QAction *m_backup = nullptr;
    QAction *m_transfer = nullptr;
    QAction *m_hosted = nullptr;
    QAction *m_drafts = nullptr;
    QAction *m_stop = nullptr;
    QPointer<LiveSyncTab> m_tab;     // (the window may delete them first)
    QPointer<LiveSyncBars> m_bars;
    QPointer<Inspector> m_inspector;
    QPointer<BinderPanel> m_binder;
};

} // namespace livesync
} // namespace qb
