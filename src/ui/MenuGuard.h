#pragma once
// A menu run with exec() inside a window that closes itself (Qt::WA_DeleteOnClose) must not see that window deleted
// under it: the menu's event loop runs the deferred delete and the window's destructor frees the menu, which is still
// in use (0.94.0, item 63, a double free). execGuarded() holds a close request of the menu's own window while the menu
// runs and carries it out after exec() has returned, if the window is still there.
#include <QCloseEvent>
#include <QMenu>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QWidget>

namespace qb {

class CloseHold : public QObject
{
public:
    bool held = false;
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() != QEvent::Close)
            return false;
        held = true;
        e->ignore();
        return true;
    }
};

// Runs `menu` at `globalPos` and returns its action as QMenu::exec does. `owner` is any widget of the window to guard.
inline QAction *execGuarded(QMenu &menu, const QPoint &globalPos, QWidget *owner)
{
    QPointer<QWidget> win = owner ? owner->window() : nullptr;
    CloseHold hold;
    if (win)
        win->installEventFilter(&hold);
    QAction *a = menu.exec(globalPos);
    if (win) {
        win->removeEventFilter(&hold);
        if (hold.held)
            win->close();
    }
    return a;
}

} // namespace qb
