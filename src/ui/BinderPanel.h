#pragma once
#include "core/Project.h"
#include "core/ProjectTypes.h"

#include <QHash>
#include <QListWidget>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QTabBar>
#include <QTimer>
#include <QTreeView>
#include <QWidget>
#include <functional>

class QLineEdit;
class QStackedWidget;
class QToolButton;
class QLabel;
class QMenu;

namespace qb {


class BinderTree : public QTreeView
{
    Q_OBJECT
public:
    explicit BinderTree(QWidget *parent = nullptr);
    // The document the active editor shows; marked with a grey bar while it
    // is not the selected one (after Back / Forward, a link, a locked editor).
    void setEditorItemId(const QString &id);
    QString editorItemId() const { return m_editorItemId; }
    void setModel(QAbstractItemModel *model) override;
    // The title column is as wide as the widest visible title (at least the
    // width of the view), so long titles are reached with the horizontal
    // scroll bar instead of being cut off (0.17).
    void updateColumnWidth();
    int contentWidth() const;
    // 0.92.0: the item whose mind map is fixed in the inspector ("" = none). Its row carries a pin; the row under
    // the mouse shows an empty one that fixes it with a click (a click on the fixed row's pin releases).
    void setFixedMindMapId(const QString &id);
    QString fixedMindMapId() const { return m_fixedMapId; }
    QModelIndex hoverIndex() const { return m_hover; }
    // whether the pin of the fixed mind map may show on this row: the row under the mouse (Android 0.92.0: on a
    // tablet also the one selected row)
    bool pinShownOn(const QModelIndex &index) const;
    std::function<bool(const QModelIndex &)> canFixMindMap;   // set by the panel (not the Trash and what is in it)
    // 0.92.0: the documents another device works on, with the tool tip of each ("" = none): a padlock on their rows
    void setLockMarks(const QHash<QString, QString> &tips);
    QString lockTip(const QString &id) const { return m_lockTips.value(id); }
    // the places of the two marks in a row, for the tests (viewport coordinates)
    QRect pinRectOf(const QModelIndex &index) const;
    QRect lockRectOf(const QModelIndex &index) const;
signals:
    void mindMapPinClicked(const QString &itemId);
    void enterPressed();
    void openPressed(); // Return with the binder keys of the file manager (0.94.0)
    void deletePressed();
    void spacePressed();
    void selectionChangedByUser();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    bool viewportEvent(QEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void scheduleWidthUpdate();
    void setHover(const QModelIndex &index);
    void updateRow(const QModelIndex &index);
    QString m_fixedMapId;
    QPersistentModelIndex m_hover;
    QHash<QString, QString> m_lockTips;
    bool m_pinPressed = false;
    QPersistentModelIndex m_lockPressed;   // Android 0.92.0: a tap that began on this row's padlock
    QTimer *m_widthTimer = nullptr;
    // A click loads the document when the button is released, so that a
    // document can be dragged elsewhere (an editor's header bar, a copyholder)
    // without being loaded into the editor on the way.
    bool m_mouseDown = false;
    bool m_selectionPending = false;
    bool m_droppedHere = false;
    QString m_editorItemId;
};

// The flat list of documents that takes the binder's place for search
// results and collections. Its rows can be dragged like binder documents; an
// arbitrary collection can be reordered and takes documents dropped into it.
class ItemListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ItemListWidget(QWidget *parent = nullptr);
    void setProject(Project *p) { m_project = p; }
    void setReorderable(bool on) { m_reorderable = on; }
    bool isReorderable() const { return m_reorderable; }
    QList<BinderItem *> selectedBinderItems() const;
    BinderItem *binderItemAt(const QPoint &viewportPos) const;
signals:
    void spacePressed();
    void deletePressed();
    // row: where in the list; internal: the rows were dragged inside this list
    void itemsDropped(const QList<qb::BinderItem *> &items, int row, bool internal);

protected:
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override;
    Qt::DropActions supportedDropActions() const override { return Qt::MoveAction | Qt::CopyAction; }
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    int rowForDrop(const QPoint &pos) const;
    QPointer<Project> m_project;
    bool m_reorderable = false;
    int m_dropRow = -1;
};

// The tabs above the binder: "Binder" and one per collection. Documents can
// be dropped onto the tab of an arbitrary collection; the tabs can be dragged
// into another order.
class CollectionTabs : public QTabBar
{
    Q_OBJECT
public:
    explicit CollectionTabs(QWidget *parent = nullptr);
    std::function<bool(int tab)> takesDocuments; // set by the panel
signals:
    void documentsDropped(int tab, const QMimeData *data);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
};

class BinderPanel : public QWidget
{
    Q_OBJECT
public:
    explicit BinderPanel(QWidget *parent = nullptr);
    void setProject(Project *p);
    BinderTree *tree() const { return m_tree; }

    QList<BinderItem *> selectedItems() const;
    BinderItem *currentItem() const;
    void select(const QList<BinderItem *> &items, bool notify);
    void reveal(BinderItem *item);
    // The item's row blinks twice in a color that stands out (0.60.3: Reveal in Binder, and a
    // double-click beside the text). The blink is a widget named "binderFlash" over the row.
    void flash(BinderItem *item);
    void editTitle(BinderItem *item);
    void setFooterMenus(QMenu *addMenu, QMenu *gearMenu);
    // 0.92.0: the fixed mind map (the pin on its row, and a slim bar above the footer with the item's title and
    // "Release" while one is fixed); the panel only shows it, the main window keeps it
    void setFixedMindMap(BinderItem *item);
    // 0.92.0: the padlocks (documents another device works on; id -> tool tip)
    void setLockMarks(const QHash<QString, QString> &tips);

    // project search
    void showSearch(bool focus);
    void hideSearch();
    bool isSearching() const;
    void runSearch(const SearchQuery &q);
    SearchQuery searchQuery() const { return m_query; }
    // the list shows the results of a project search (not the binder, not a collection)
    bool showsSearchResults() const;
    // Options menu of the search field. "Search In": one place (or all with a
    // null member); multi (Alt held) switches that place on or off beside the others.
    void chooseSearchIn(bool SearchQuery::*member, bool multi);
    void fillSearchMenu(QMenu *menu);
    void setSearchQuery(const SearchQuery &q);   // options without running a search (empty text)
    QLineEdit *searchField() const { return m_searchEdit; }

    // collections
    void setCollectionsVisible(bool on);
    bool collectionsVisible() const;
    void refreshCollections();
    void showCollection(const QString &id);
    QString currentCollectionId() const;
    void addToCollection(const QString &collectionId, const QList<BinderItem *> &items, int row = -1);
    // true while the list shows an arbitrary collection (not the binder, not a search)
    bool showsArbitraryCollection() const;
    void removeFromCollection(const QList<BinderItem *> &items);
    ItemListWidget *resultsList() const { return m_results; }
    CollectionTabs *collectionTabs() const { return m_tabs; }
    void setEditorItem(BinderItem *item);

    QStringList expandedIds() const;
    void restoreExpanded(const QStringList &ids);

signals:
    void selectionChanged(const QList<qb::BinderItem *> &items);
    void contextMenuRequested(const QPoint &globalPos);
    void backgroundMenuRequested(const QPoint &globalPos); // empty space of the binder (0.90.1)
    void fixMindMapRequested(qb::BinderItem *item);   // 0.92.0: the pin of a row
    void releaseMindMapRequested();                   // 0.92.0: the pin of the fixed row, or the bar's button
    void newDocumentRequested();
    void openRequested();   // Return: show the selected document in the editor and put the keyboard there (0.94.0)
    void newFolderRequested();
    void trashRequested();
    void quickReferenceRequested();
    // a project search was run, changed or closed (the editors show its matches)
    void searchChanged();

private:
    void onSearchTextChanged();
    void showResults(const QList<BinderItem *> &items, const QString &title, const QColor &color);
    void onResultSelection();
    void onTabChanged(int index);
    void buildSearchMenu();
    void selectSearchTab(bool search);
    void flashStep();

    QPointer<Project> m_project;
    BinderTree *m_tree = nullptr;
    QWidget *m_flash = nullptr;
    QTimer m_flashTimer;
    QPersistentModelIndex m_flashIndex;
    int m_flashStep = 0;
    QWidget *m_searchBar = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QToolButton *m_searchOptions = nullptr;
    QStackedWidget *m_stack = nullptr;
    QWidget *m_resultsPage = nullptr;
    QLabel *m_resultsTitle = nullptr;
    ItemListWidget *m_results = nullptr;
    CollectionTabs *m_tabs = nullptr;
    QWidget *m_collectionsBar = nullptr;
    QToolButton *m_addButton = nullptr;
    QToolButton *m_folderButton = nullptr;
    QToolButton *m_gearButton = nullptr;
    QWidget *m_fixedBar = nullptr;      // 0.92.0
    QLabel *m_fixedLabel = nullptr;
    QString m_fixedTitle;
    SearchQuery m_query;
    QString m_shownCollection; // id or "__search"
    bool m_updatingTabs = false;
};

} // namespace qb
