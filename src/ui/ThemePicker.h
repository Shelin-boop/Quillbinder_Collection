#pragma once
// The theme control of the Themes toolbar (0.90.2; its own toolbar "Themes" since 0.91.0; one button with a menu since
// 0.94.0, the toolbar being off at first).
//
// Before 0.94.0 four buttons (Bright, Dark, U1, U2) and a list took 333 px of the toolbar; U1 and U2 were explained only
// by a message after the first switch. Now there is ONE button "Theme" with a menu: Light, Dark, My Look 1, My Look 2,
// then every theme (the one in use checked), then "Save the Current Look as My Look 1/2".
//   Light, Dark:        the themes assigned to the two entries ("Light" and "Dark Mode" until another is assigned with
//                       assign(); kept in window/themeBright, window/themeDark).
//   My Look 1, My Look 2: the writer's own looks, kept as the own themes "User_1" and "User_2" (the file names stay).
//                       An entry that is still empty is marked "(empty)" and saves the look shown now; a filled one shows
//                       its look.
// A theme is shown as it is, so the choice changes what is seen. Colors and fonts of the writer's own that lie on top of
// the theme shown before would be lost by that: they are saved first, into an empty My Look without a question, else the
// writer says where (keepOwnLook). The list of all themes does what Window > Themes does (themeui::choose).
#include <QStringList>
#include <QWidget>

class QAction;
class QMenu;
class QToolBar;
class QToolButton;

namespace qb {

class ThemePicker : public QWidget
{
    Q_OBJECT
public:
    enum Button { Bright = 0, Dark = 1, User1 = 2, User2 = 3 };

    explicit ThemePicker(QWidget *parent = nullptr);

    // the theme an entry shows: "" for an empty U button
    QString themeOf(Button b) const;
    static QString userThemeId(Button b);   // "user:User_1", "user:User_2"
    static QString userThemeName(Button b); // "User_1", "User_2"
    // Light, Dark (Bright, Dark): the theme of the entry from now on (false: no such theme, or a My Look)
    bool assign(Button b, const QString &themeId);
    // what a click on the button does; false when nothing changed (cancelled, or a failure that was told)
    bool press(Button b);
    // My Look 1, My Look 2: the look shown now becomes the button's look and the theme in use (askFirst: a filled button asks
    // before its look is replaced)
    bool saveLookIn(Button b, bool askFirst = true);
    // the menu of the button, filled again each time it opens (add-ons and the theme manager add and remove themes)
    void fillMenu(QMenu *menu);
    static QString lookName(Button b); // "My Look 1" for User1 (for menus, tool tips and messages), "Light", "Dark"

    // the ids in the menu's list of all themes now (for the checks)
    QStringList shownIds() const;
    void refill();

    // For the checks: the answer to the question that comes when the look has changes of the writer's own, both My Looks
    // are taken and an entry is chosen: User1 or User2 = save there first, -1 = switch without saving,
    // -2 = cancel, -3 (the standard) = ask.
    static void setAnswerForTests(int answer);

    // On a toolbar too narrow for both, the list gives way to `keep` (the quick search field): its action is hidden
    // while `keep` would otherwise go into the toolbar's overflow menu, and shown again once the toolbar is wide enough.
    // (Only where the writer put both on one toolbar: since 0.91.0 each stands on a toolbar of its own.)
    static void giveWay(QToolBar *bar, QAction *own, QWidget *keep);

private:
    void updateButtons();
    bool store(Button b);
    bool keepOwnLook();
    void tell(const QString &text);

    QToolButton *m_button = nullptr;
    QMenu *m_menu = nullptr;
};

} // namespace qb
