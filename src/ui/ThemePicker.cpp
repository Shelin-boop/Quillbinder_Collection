#include "ui/ThemePicker.h"

#include "core/ItemIcons.h"
#include "text/TextUtil.h"
#include "ui/Ask.h"
#include "ui/MainWindow.h"
#include "ui/ProjectLook.h"
#include "ui/Theme.h"
#include "ui/ThemeDialogs.h"

#include <QActionGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace qb {

namespace {

const char *kBrightKey = "window/themeBright";
const char *kDarkKey = "window/themeDark";
int s_answerForTests = -3;

QString buttonLabel(ThemePicker::Button b) { return ThemePicker::lookName(b); }

QString themeName(const QString &id)
{
    for (const theme::Entry &e : theme::available())
        if (e.id == id)
            return e.name;
    return id;
}

bool themeExists(const QString &id)
{
    theme::Theme t;
    return !id.isEmpty() && theme::load(id, &t);
}

// watches the toolbar: the theme list goes out of the way of the quick search field and comes back when there is room
class GiveWay : public QObject
{
public:
    GiveWay(QToolBar *bar, QAction *own, QWidget *keep) : QObject(bar), m_bar(bar), m_own(own), m_keep(keep)
    {
        bar->installEventFilter(this);
    }

    bool eventFilter(QObject *, QEvent *e) override
    {
        if ((e->type() == QEvent::Resize || e->type() == QEvent::LayoutRequest || e->type() == QEvent::Show) && !m_queued) {
            m_queued = true;
            QTimer::singleShot(0, this, [this] {
                m_queued = false;
                look();
            });
        }
        return false;
    }

private:
    void look()
    {
        if (!m_bar || !m_own || !m_keep || !m_bar->isVisible())
            return;
        // only when both are on the toolbar
        bool keepOnBar = false;
        for (QAction *a : m_bar->actions())
            keepOnBar = keepOnBar || m_bar->widgetForAction(a) == m_keep;
        if (!keepOnBar || !m_bar->actions().contains(m_own))
            return;
        if (m_own->isVisible()) {
            if (!m_keep->isVisible()) { // the search went into the overflow menu
                QWidget *w = m_bar->widgetForAction(m_own);
                m_need = (w ? w->sizeHint().width() : 300) + 24;
                m_hiddenAt = m_bar->width();
                m_own->setVisible(false);
            }
        } else if (m_bar->width() >= m_hiddenAt + m_need) {
            m_own->setVisible(true); // if the search goes again, the next look() hides the list again
        }
    }

    QPointer<QToolBar> m_bar;
    QPointer<QAction> m_own;
    QPointer<QWidget> m_keep;
    int m_hiddenAt = 0, m_need = 0;
    bool m_queued = false;
};

} // namespace

void ThemePicker::giveWay(QToolBar *bar, QAction *own, QWidget *keep)
{
    if (!bar || !own || !keep)
        return;
    for (QObject *o : bar->children())
        if (dynamic_cast<GiveWay *>(o))
            return; // once per toolbar
    new GiveWay(bar, own, keep);
}

void ThemePicker::setAnswerForTests(int answer) { s_answerForTests = answer; }

QString ThemePicker::userThemeName(Button b) { return b == User2 ? QStringLiteral("User_2") : QStringLiteral("User_1"); }

QString ThemePicker::lookName(Button b)
{
    switch (b) {
    case Bright: return tr("Light");
    case Dark: return tr("Dark");
    case User1: return tr("My Look 1");
    case User2: return tr("My Look 2");
    }
    return QString();
}

QString ThemePicker::userThemeId(Button b) { return QLatin1String("user:") + userThemeName(b); }

ThemePicker::ThemePicker(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("themePicker"));
    auto *h = new QHBoxLayout(this);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    m_button = new QToolButton(this);
    m_button->setObjectName(QStringLiteral("themeButton"));
    m_button->setText(tr("Theme"));
    m_button->setIcon(icons::action(QStringLiteral("format-text-color")));
    m_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_button->setPopupMode(QToolButton::InstantPopup);
    m_button->setAccessibleName(tr("Theme"));
    m_menu = new QMenu(m_button);
    m_menu->setObjectName(QStringLiteral("themeButtonMenu"));
    m_menu->setToolTipsVisible(true);
    // filled when opened: add-ons and the theme manager add and remove themes at any time
    connect(m_menu, &QMenu::aboutToShow, this, [this] { fillMenu(m_menu); });
    m_button->setMenu(m_menu);
    h->addWidget(m_button);

    connect(&theme::Notifier::instance(), &theme::Notifier::changed, this, &ThemePicker::refill);
    refill();
}

QString ThemePicker::themeOf(Button b) const
{
    if (b == User1 || b == User2)
        return themeExists(userThemeId(b)) ? userThemeId(b) : QString();
    const QString assigned = QSettings().value(QLatin1String(b == Bright ? kBrightKey : kDarkKey)).toString();
    // a theme that is gone (an add-on removed, an own theme deleted): the standard again
    return themeExists(assigned) ? assigned : (b == Bright ? QStringLiteral("light") : QStringLiteral("dark"));
}

bool ThemePicker::assign(Button b, const QString &themeId)
{
    if ((b != Bright && b != Dark) || !themeExists(themeId))
        return false;
    QSettings().setValue(QLatin1String(b == Bright ? kBrightKey : kDarkKey), themeId);
    updateButtons();
    return true;
}

void ThemePicker::tell(const QString &text)
{
    if (auto *mw = qobject_cast<MainWindow *>(window()))
        mw->showStatusNote(text, 8000);
}

// The look shown now (theme and the writer's own colors and fonts on top) written as the own theme of a U button.
bool ThemePicker::store(Button b)
{
    theme::Theme t = theme::currentLook();
    t.id = userThemeId(b); // (an own theme's id: saved over the file of that name)
    t.name = userThemeName(b);
    QString error;
    if (theme::saveUserTheme(t, &error).isEmpty()) {
        QMessageBox::warning(window(), tr("Themes"), tr("The look could not be saved in %1: %2").arg(buttonLabel(b), error));
        return false;
    }
    return true;
}

// Before a button shows its theme as it is: colors and fonts of the writer's own on top of the theme shown now are
// saved first, so a click never loses a look. An empty U button takes them without a question; with both taken the
// writer says where they go, or that they may go.
bool ThemePicker::keepOwnLook()
{
    const int own = themeui::ownChanges();
    if (own == 0)
        return true;
    for (Button u : {User1, User2})
        if (themeOf(u).isEmpty()) {
            if (!store(u))
                return false;
            tell(tr("The look you had was saved in %1 first.").arg(buttonLabel(u)));
            return true;
        }
    int answer = s_answerForTests;
    if (answer == -3) {
        QMessageBox box(QMessageBox::Question, tr("Change Theme"),
                        tr("The look you see has %Ln color(s) or font(s) of your own that are not saved in My Look 1 or My Look 2.", "", own),
                        QMessageBox::NoButton, window());
        box.setObjectName(QStringLiteral("themeKeepQuestion"));
        box.setInformativeText(tr("Save the look in My Look 1 or My Look 2 first (it replaces the look saved there), or switch without "
                                  "saving it?"));
        QPushButton *u1 = box.addButton(tr("Save in My Look 1"), QMessageBox::ActionRole);
        QPushButton *u2 = box.addButton(tr("Save in My Look 2"), QMessageBox::ActionRole);
        QPushButton *drop = box.addButton(tr("Switch Without Saving"), QMessageBox::DestructiveRole);
        QPushButton *cancel = box.addButton(QMessageBox::Cancel);
        // the look came from a U button and was changed since: back into that button; else nothing happens on Enter
        const QString now = look::active().theme;
        QPushButton *standard = now == userThemeId(User1) ? u1 : now == userThemeId(User2) ? u2 : cancel;
        box.setDefaultButton(standard);
        box.setEscapeButton(cancel);
        box.exec();
        answer = box.clickedButton() == u1 ? int(User1) : box.clickedButton() == u2 ? int(User2) : box.clickedButton() == drop ? -1 : -2;
    }
    if (answer == -1)
        return true;
    if (answer != int(User1) && answer != int(User2))
        return false;
    if (!store(Button(answer)))
        return false;
    tell(tr("The look you had was saved in %1 first.").arg(buttonLabel(Button(answer))));
    return true;
}

bool ThemePicker::saveLookIn(Button b, bool askFirst)
{
    if (b != User1 && b != User2)
        return false;
    const bool filled = !themeOf(b).isEmpty();
    if (filled && look::active().theme == userThemeId(b) && themeui::ownChanges() == 0) {
        tell(tr("%1 already holds the look you see.").arg(buttonLabel(b)));
        return false;
    }
    if (filled && askFirst
        && !ask::confirm(window(), tr("Themes"),
                         tr("%1 already holds a look. Replace it with the look you see now? The look saved there cannot be "
                            "brought back.")
                             .arg(buttonLabel(b)),
                         tr("Replace the Look in %1").arg(buttonLabel(b))))
        return false;
    if (!store(b))
        return false;
    // the own colors and fonts are part of the saved look now: it is shown as it is
    const bool ok = themeui::useAsItIs(window(), userThemeId(b));
    refill();
    tell(tr("The look you see is saved in %1. A click on %1 brings it back.").arg(buttonLabel(b)));
    return ok;
}

bool ThemePicker::press(Button b)
{
    if ((b == User1 || b == User2) && themeOf(b).isEmpty())
        return saveLookIn(b, false); // an empty button takes the look shown now
    const QString id = themeOf(b);
    if (id.isEmpty())
        return false;
    if (id == look::active().theme && themeui::ownChanges() == 0)
        return false; // it is shown already
    if (!keepOwnLook())
        return false;
    const bool ok = themeui::useAsItIs(window(), id);
    refill();
    return ok;
}

void ThemePicker::fillMenu(QMenu *menu)
{
    menu->clear();
    const QString now = look::active().theme;
    auto *group = new QActionGroup(menu);
    for (Button b : {Bright, Dark, User1, User2}) {
        const QString id = themeOf(b);
        const bool mine = b == User1 || b == User2;
        QAction *one = menu->addAction(mine && id.isEmpty() ? tr("%1 (empty)").arg(lookName(b)) : lookName(b));
        one->setObjectName(QLatin1String(b == Bright ? "themeBright" : b == Dark ? "themeDark" : b == User1 ? "themeUser1" : "themeUser2"));
        one->setCheckable(true);
        one->setChecked(!id.isEmpty() && id == now);
        one->setData(id);
        group->addAction(one);
        if (mine) {
            const QString tip = id.isEmpty() ? tr("Empty: choosing it saves the look you see now as %1").arg(lookName(b))
                                             : tr("Your own look saved as %1; it brings back the colors and fonts saved there").arg(lookName(b));
            one->setToolTip(tip);
            one->setStatusTip(tip);
        } else {
            one->setToolTip(tr("Show the theme “%1”").arg(themeName(id)));
        }
        connect(one, &QAction::triggered, this, [this, b] {
            press(b);
            updateButtons();
        });
    }
    menu->addSeparator();
    theme::Kind last = theme::Kind::BuiltIn;
    for (const theme::Entry &e : theme::available()) {
        if (e.kind != last) {
            menu->addSeparator();
            last = e.kind;
        }
        QAction *one = menu->addAction(e.name);
        one->setObjectName(QStringLiteral("themeEntry"));
        one->setCheckable(true);
        one->setChecked(e.id == now);
        one->setData(e.id);
        group->addAction(one);
        connect(one, &QAction::triggered, this, [this, id = e.id] {
            if (id != look::active().theme)
                themeui::choose(window(), id);
            refill();
        });
    }
    menu->addSeparator();
    for (Button b : {User1, User2}) {
        QAction *save = menu->addAction(tr("Save the Current Look as %1").arg(lookName(b)));
        save->setObjectName(b == User1 ? QStringLiteral("themeSaveHere1") : QStringLiteral("themeSaveHere2"));
        connect(save, &QAction::triggered, this, [this, b] { saveLookIn(b); });
    }
}

void ThemePicker::updateButtons()
{
    const QString now = look::active().theme;
    QString what = themeName(now);
    for (Button b : {Bright, Dark, User1, User2})
        if (!themeOf(b).isEmpty() && themeOf(b) == now)
            what = lookName(b);
    m_button->setToolTip(tr("Theme: %1 (click for the list of themes, Light, Dark and your own looks)").arg(what));
}

QStringList ThemePicker::shownIds() const
{
    QMenu m;
    const_cast<ThemePicker *>(this)->fillMenu(&m);
    QStringList out;
    for (QAction *a : m.actions())
        if (a->objectName() == QLatin1String("themeEntry"))
            out << a->data().toString();
    return out;
}

void ThemePicker::refill()
{
    updateButtons();
}

} // namespace qb
