#pragma once
// Help > Find Command (0.94.0): a window with a search field and the list of every menu command with its menu path and
// shortcut. Typing narrows the list; Return or a double-click runs the chosen command. A command that is off at the
// moment is shown greyed and does not run.
#include <QDialog>
#include <QList>
#include <QPointer>

class QAction;
class QLineEdit;
class QListWidget;
class QMenuBar;

namespace qb {

class CommandFinder : public QDialog
{
    Q_OBJECT
public:
    struct Entry {
        QPointer<QAction> action;
        QString label;    // the text without "&" and "..."
        QString path;     // "Edit > Find"
        QString shortcut; // as shown, or empty
        bool enabled = false;
    };

    explicit CommandFinder(QMenuBar *bar, QWidget *parent = nullptr);
    // Every command of the menus (what is in the menus now: lists filled on opening are not in it).
    static QList<Entry> collect(QMenuBar *bar);
    // The rows that match the words of the text (empty: all), in the menus' order.
    static QList<int> matching(const QList<Entry> &entries, const QString &text);

    void setFilter(const QString &text);
    int visibleRows() const;
    // Runs the chosen row (the first one when none is chosen): false when there is none or it is off.
    bool runRow(int row);
    bool runCurrent();

signals:
    void commandRun(const QString &path);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    void refill();
    void step(int by);

    QList<Entry> m_entries;
    QLineEdit *m_search = nullptr;
    QListWidget *m_list = nullptr;
};

} // namespace qb
