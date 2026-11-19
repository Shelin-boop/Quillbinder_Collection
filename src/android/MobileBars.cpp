#include "android/Mobile.h"

#include "ui/BarDock.h"
#include "ui/BarPlaces.h"
#include "ui/GripBar.h"
#include "ui/ToolbarConfig.h"

#include <QAction>
#include <QEvent>
#include <QMainWindow>
#include <QSettings>
#include <QTimer>

namespace qb {
namespace mobile {

namespace {

using namespace barplaces;

constexpr int kMaxRows = 3; // more rows than this: the Themes toolbar makes room

class BarFit : public QObject
{
public:
    BarFit(QMainWindow *window, BarDock *dock)
        : QObject(window), m_window(window), m_dock(dock)
    {
        // The rows to start from. The place keeper keeps the rows as they were fitted to the last width; the rows
        // the writer has are kept here, so that a wider window gets them back.
        QSettings s;
        m_wanted = dock->places();
        if (const QStringList kept = s.value(QStringLiteral("window/mobileBarRows")).toStringList(); !kept.isEmpty())
            m_wanted = normalized(fromText(kept), dock->standardPlaces());
        else
            s.setValue(QStringLiteral("window/mobileBarRows"), toText(m_wanted));
        m_themes = window->findChild<GripBar *>(toolbars::barObjectName(toolbars::Bar::Themes));
        m_themesSwitch = window->findChild<QAction *>(toolbars::barCommand(toolbars::Bar::Themes));
        m_hidThemes = s.value(QStringLiteral("window/mobileThemesOffForRoom"), false).toBool();
        m_keepThemes = s.value(QStringLiteral("window/mobileThemesKept"), false).toBool();
        m_timer.setSingleShot(true);
        m_timer.setInterval(0);
        connect(&m_timer, &QTimer::timeout, this, [this] { fit(); });
        // the writer moved a toolbar: that is the order and the rows to start from
        connect(dock, &BarDock::placesChanged, this, [this] {
            if (!m_applying) {
                m_wanted = m_dock->places();
                QSettings().setValue(QStringLiteral("window/mobileBarRows"), toText(m_wanted));
                m_timer.start();
            }
        });
        // the writer switched the Themes toolbar on after it was switched off for room: it stays
        if (m_themesSwitch)
            connect(m_themesSwitch, &QAction::toggled, this, [this](bool on) {
                if (m_applying)
                    return;
                if (on && m_hidThemes) {
                    m_keepThemes = true;
                    QSettings().setValue(QStringLiteral("window/mobileThemesKept"), true);
                }
                setHidThemes(false);
                m_timer.start();
            });
        window->installEventFilter(this);
        for (GripBar *bar : window->findChildren<GripBar *>()) {
            bar->installEventFilter(this);
            connect(bar, &GripBar::contentsChanged, &m_timer, qOverload<>(&QTimer::start));
        }
        m_timer.start();
    }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (o == m_window ? e->type() == QEvent::Resize || e->type() == QEvent::Show
                          : (e->type() == QEvent::Show || e->type() == QEvent::Hide) && !m_applying)
            m_timer.start();
        return false;
    }

private:
    void setHidThemes(bool on)
    {
        if (m_hidThemes == on)
            return;
        m_hidThemes = on;
        QSettings().setValue(QStringLiteral("window/mobileThemesOffForRoom"), on);
    }

    // what a toolbar needs in its row (0 for one that is switched off)
    int need(const QString &name, bool themesOn) const
    {
        GripBar *bar = m_window->findChild<GripBar *>(name);
        if (!bar)
            return 0;
        if (bar == m_themes) {
            if (!themesOn)
                return 0;
        } else if (bar->isHidden()) {
            return 0;
        }
        return bar->naturalWidth() + 2;
    }

    // The rows of one edge: a row that is too wide hands its last toolbars to the row after it.
    QList<Row> flowed(const QList<Row> &rows, int width, bool themesOn, int *shownRows) const
    {
        QList<Row> out;
        Row carry;
        int count = 0;
        for (int r = 0; r < rows.size() || !carry.isEmpty(); ++r) {
            Row row = carry;
            const int carried = carry.size();
            carry.clear();
            if (r < rows.size())
                row += rows[r];
            Row keep;
            int used = 0, shown = 0;
            bool full = false, moved = carried > 0;
            for (int i = 0; i < row.size(); ++i) {
                const int w = need(row[i].name, themesOn);
                if (!full && w > 0 && shown > 0 && used + w > width)
                    full = true;
                if (full) {
                    carry << Slot{row[i].name, 0};
                    moved = true;
                    continue;
                }
                keep << row[i];
                used += w;
                shown += w > 0 ? 1 : 0;
            }
            if (moved)
                for (Slot &s : keep)
                    s.lead = 0; // (a row that changed starts without free space between its toolbars)
            if (!keep.isEmpty())
                out << keep;
            count += shown > 0 ? 1 : 0;
        }
        if (shownRows)
            *shownRows = count;
        return out;
    }

    void fit()
    {
        const int width = m_window->width();
        if (!m_window->isVisible() || width < 200 || m_dock->dragging())
            return;
        // would the writer have the Themes toolbar on? (its own switch, or switched off here for room)
        const bool themesWanted = m_themes && m_themesSwitch && (m_themesSwitch->isChecked() || m_hidThemes);
        int rows = 0;
        Places p = m_wanted;
        p.top = flowed(m_wanted.top, width, themesWanted, &rows);
        bool themesOn = themesWanted;
        if (themesWanted && rows > kMaxRows && !m_keepThemes) {
            themesOn = false;
            p.top = flowed(m_wanted.top, width, false, &rows);
        }
        p.bottom = flowed(m_wanted.bottom, width, themesOn, nullptr);

        m_applying = true;
        if (m_themesSwitch && themesWanted && m_themesSwitch->isChecked() != themesOn) {
            setHidThemes(!themesOn);
            m_themesSwitch->setChecked(themesOn); // (the window's own switch: it shows or hides the toolbar)
        }
        if (p != m_dock->places())
            m_dock->setPlaces(p);
        m_applying = false;
    }

    QMainWindow *m_window;
    BarDock *m_dock;
    GripBar *m_themes = nullptr;
    QAction *m_themesSwitch = nullptr;
    Places m_wanted; // the rows the writer has (or the standard ones), before they are fitted to a width
    QTimer m_timer;
    bool m_applying = false;
    bool m_hidThemes = false;  // the Themes toolbar is off because of the room, not by the writer
    bool m_keepThemes = false; // the writer switched it on again: never off for room any more
};

} // namespace

void fitToolbars(QMainWindow *window)
{
    if (!window || window->findChild<QObject *>(QStringLiteral("mobileBarFit"), Qt::FindDirectChildrenOnly))
        return;
    if (BarDock *dock = window->findChild<BarDock *>())
        (new BarFit(window, dock))->setObjectName(QStringLiteral("mobileBarFit"));
}

} // namespace mobile
} // namespace qb
