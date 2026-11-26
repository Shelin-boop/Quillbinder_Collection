#pragma once
#include <QKeySequence>
#include <QMap>
#include <QWidget>

class QKeySequenceEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace qb {

// File > Options > Keyboard: every command with its shortcut. Works on a
// copy of the changes; apply() hands them to commands::setCustom().
class KeyboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardPage(QWidget *parent = nullptr);
    void apply();

    // What the controls do (also used by the tests).
    enum class Result { Done, Unusable, Conflict };
    // takeOver: remove the shortcut from the command that has it now.
    Result assign(const QString &key, const QKeySequence &seq, bool takeOver, QString *other = nullptr);
    void resetOne(const QString &key);
    void resetAll();
    QKeySequence shown(const QString &key) const;
    int changes() const;                 // commands that differ from their default
    void select(const QString &key);
    void setFilter(const QString &text);
    int visibleCommands() const;
    // 0.95.0 (item 103): the set that fits the keyboard layout in use now; asks first what changes (ask = false: no question).
    // Returns true when the set was changed.
    bool useLayoutSet(bool ask = true);
    bool exportTo(const QString &file, QString *error) const;
    bool importFrom(const QString &file, QString *error);

private:
    void rebuild();
    void refreshRow(QTreeWidgetItem *item);
    void refreshAll();
    void selectionChanged();
    QString currentKey() const;

    QMap<QString, QString> m_changes;
    QLineEdit *m_filter;
    QTreeWidget *m_tree;
    QLabel *m_label, *m_hint;
    QKeySequenceEdit *m_edit;
    QPushButton *m_clear, *m_reset, *m_resetAll;
};

} // namespace qb
