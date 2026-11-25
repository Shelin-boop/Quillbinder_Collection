#pragma once
// The user manual's window (0.94.0). Before, F1 opened one text of about 36 screens without a contents list, a search or
// jumps. Now the window has
//   - a contents list of the manual's headings on the left (a click jumps to the heading),
//   - a search field above the text (Ctrl+F; Return and Shift+Return, or the buttons, go to the next and the previous hit;
//     a line says which hit of how many is shown; all hits are marked in the text),
//   - and F1 opens the section that belongs to what is on the screen: the main views, the inspector's tabs and the main
//     dialogs have a section (manualhelp::sectionFor...); a dialog without one opens the start of the manual.
#include <QDialog>
#include <QList>
#include <QPointer>
#include <QTextCursor>

class QAction;
class QLabel;
class QLineEdit;
class QTextBrowser;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace qb {

class ManualWindow : public QDialog
{
    Q_OBJECT
public:
    // text: the Markdown of the manual with its placeholders filled in
    ManualWindow(const QString &markdown, const QString &title, const QString &objectName, QSize size, QWidget *parent = nullptr);

    // Scrolls to the first heading whose text (without its number) is `section` (exactly, else starting with it, else
    // containing it; any case); false when there is none (the start is shown then)
    bool showSection(const QString &section);
    // the headings in the contents list, as text (for the checks)
    QStringList headings() const;
    // the search (for the checks): puts the text into the field and marks the hits; returns their number
    int search(const QString &text);
    int hitCount() const { return int(m_hits.size()); }
    int currentHit() const { return m_current; }   // 0-based, -1 when there is none
    void nextHit(bool forward = true);
    // the text of the heading the window shows at its top (for the checks)
    QString topHeading() const;

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void buildContents();
    void jumpTo(int position);
    void markHits();
    void showHit(int index);

    QTextBrowser *m_text = nullptr;
    QTreeWidget *m_contents = nullptr;
    QLineEdit *m_find = nullptr;
    QLabel *m_count = nullptr;
    QToolButton *m_prev = nullptr, *m_next = nullptr;
    struct Heading {
        QString text;
        int level = 0;
        int position = 0;
    };
    QList<Heading> m_headings;
    QList<QTextCursor> m_hits;
    int m_current = -1;
};

namespace manualhelp {

// The section of the manual for a dialog or another window of the program (its class decides); empty: none, the start
QString sectionForWindow(const QWidget *window);
// The section for the inspector's tab (Inspector::Tab number; extraKey: the key of a tool tab of the add-ons)
QString sectionForInspectorTab(int tab, const QString &extraKey);
// F1 in a dialog or a window other than the main window (the main window's action does it there): opens the manual at
// the section of that window. `manual` is the action "User Manual" (its shortcut is the key). Installed once.
void installF1(QAction *manual, QWidget *mainWindow);

} // namespace manualhelp

// Opens (or shows again) the manual's window; section: a heading to show (see ManualWindow::showSection), empty: the
// start. parent: the main window or the dialog the question came from (a modal dialog blocks a window that is not its
// child).
void showManualAt(QWidget *parent, const QString &section);
// The window now open (for the checks), null when none
ManualWindow *manualWindow();

} // namespace qb
