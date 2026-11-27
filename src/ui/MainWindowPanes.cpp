// The panes placed by the window's shape (0.93.0, to-do item 22).
//
// A wide window (width not less than height; a square window counts as wide) shows three columns in an order the
// writer chooses (six orders). An upright window shows two rows below the toolbars: the editor's row and a row with
// the binder and the inspector beside each other; the writer chooses whether the editor's row is the upper or the lower
// one and whether the binder is on the left or the right. The window follows its shape by itself; each shape keeps its
// own places and sizes. Everything is kept per device in the program's settings (never in a project).
//
// Parts: m_mainSplit is the one splitter below the toolbars (three columns when wide, two rows when upright);
// m_sideSplit is the row of the binder and the inspector (upright only; when the window is wide it hangs hidden under the
// window without panes).
#include "ui/MainWindow.h"

#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/Inspector.h"

#include <QAction>
#include <QActionGroup>
#include <QGuiApplication>
#include <QInputMethod>
#include <QResizeEvent>
#include <QSettings>
#include <QSplitter>
#include <QTimer>

namespace qb {

namespace {
const char *const kOrders[] = {"BEI", "BIE", "EBI", "EIB", "IBE", "IEB"};
constexpr int kStartWide[3] = {260, 800, 300};
constexpr double kStartEditorShare = 0.62;
constexpr double kStartBinderShare = 0.46;

int paneIndex(QChar c)
{
    return c == QLatin1Char('B') ? 0 : c == QLatin1Char('E') ? 1 : 2;
}

bool validOrder(const QString &o)
{
    for (const char *k : kOrders)
        if (o == QLatin1String(k))
            return true;
    return false;
}
} // namespace

QWidget *MainWindow::paneWidget(int pane) const
{
    return pane == 0 ? static_cast<QWidget *>(m_binder) : pane == 1 ? m_editorArea : static_cast<QWidget *>(m_inspector);
}

void MainWindow::initPaneSplitters()
{
    m_sideSplit = new QSplitter(Qt::Horizontal, this);
    m_sideSplit->setObjectName(QStringLiteral("sideSplit"));
    m_sideSplit->setChildrenCollapsible(false);
    m_sideSplit->hide();
    m_mainSplit->setObjectName(QStringLiteral("mainSplit"));
    connect(m_mainSplit, &QSplitter::splitterMoved, this, [this] { capturePaneSizes(); });
    connect(m_sideSplit, &QSplitter::splitterMoved, this, [this] { capturePaneSizes(); });
}

bool MainWindow::wantsUpright() const
{
    int h = height();
    // the on-screen keyboard takes height from the window; it must not turn a tablet held upright into a wide window
    if (QInputMethod *im = QGuiApplication::inputMethod(); im && im->isVisible())
        h += im->keyboardRectangle().toRect().height();
    return h > width();
}

void MainWindow::loadPaneSettings(QSettings &s)
{
    const QString order = s.value(QStringLiteral("window/paneOrder"), QStringLiteral("BEI")).toString();
    m_wideOrder = validOrder(order) ? order : QStringLiteral("BEI");
    m_uprightEditorFirst = s.value(QStringLiteral("window/uprightEditorFirst"), true).toBool();
    m_uprightBinderLeft = s.value(QStringLiteral("window/uprightBinderLeft"), true).toBool();
    const QVariantList shares = s.value(QStringLiteral("window/uprightShares")).toList();
    if (shares.size() == 2) {
        m_uprightEditorShare = qBound(0.1, shares[0].toDouble(), 0.9);
        m_uprightBinderShare = qBound(0.1, shares[1].toDouble(), 0.9);
    }
    // the wide shape's widths by pane (as before, they were by place; the start order is the same)
    const QVariantList sizes = s.value(QStringLiteral("window/mainSplit")).toList();
    if (sizes.size() == 3)
        for (int i = 0; i < 3; ++i)
            if (sizes[i].toInt() > 0)
                m_wideSizes[i] = sizes[i].toInt();
    m_upright = wantsUpright();
    arrangePanes();
    updatePaneActions();
}

QList<int> MainWindow::paneSizes() const
{
    QList<int> r{m_wideSizes[0], m_wideSizes[1], m_wideSizes[2]};
    if (!m_upright && !m_small) {
        const QList<int> now = m_mainSplit->sizes();
        for (int i = 0; i < 3 && i < now.size(); ++i)
            if (now[i] > 0 && !paneWidget(paneIndex(m_wideOrder.at(i)))->isHidden())
                r[paneIndex(m_wideOrder.at(i))] = now[i];
    }
    for (int p = 0; p < 3; ++p)
        if (paneWidget(p)->isHidden())
            r[p] = 0;
    return r;
}

void MainWindow::capturePaneSizes()
{
    if (!m_mainSplit || !m_sideSplit || m_small) // (the small interface keeps the full interface's sizes as they were)
        return;
    const QList<int> main = m_mainSplit->sizes();
    if (!m_upright) {
        for (int i = 0; i < 3 && i < main.size(); ++i) {
            const int p = paneIndex(m_wideOrder.at(i));
            if (!paneWidget(p)->isHidden() && main[i] > 0)
                m_wideSizes[p] = main[i];
        }
        return;
    }
    if (main.size() == 2 && !m_sideSplit->isHidden()) {
        const int ei = m_uprightEditorFirst ? 0 : 1, si = 1 - ei;
        if (main[ei] > 0 && main[si] > 0)
            m_uprightEditorShare = qBound(0.1, double(main[ei]) / (main[ei] + main[si]), 0.9);
    }
    const QList<int> side = m_sideSplit->sizes();
    if (side.size() == 2 && !m_binder->isHidden() && !m_inspector->isHidden() && side[0] > 0 && side[1] > 0) {
        const int bi = m_uprightBinderLeft ? 0 : 1;
        m_uprightBinderShare = qBound(0.1, double(side[bi]) / (side[0] + side[1]), 0.9);
    }
}

void MainWindow::applyPaneSizes()
{
    if (m_small)
        return;
    if (!m_upright) {
        QList<int> sizes;
        for (int i = 0; i < 3; ++i) {
            const int p = paneIndex(m_wideOrder.at(i));
            sizes << (paneWidget(p)->isHidden() ? 0 : m_wideSizes[p]);
        }
        m_mainSplit->setSizes(sizes);
        return;
    }
    // pixels of the splitters' present size (a size that does not add up to the splitter's is not taken as shares)
    const int h = m_mainSplit->height() > 200 ? m_mainSplit->height() - m_mainSplit->handleWidth() : 1000;
    const int ew = int(m_uprightEditorShare * h), sw = h - ew;
    m_mainSplit->setSizes(m_uprightEditorFirst ? QList<int>{ew, sw} : QList<int>{sw, ew});
    const int total = m_sideSplit->width() > 200 ? m_sideSplit->width() - m_sideSplit->handleWidth() : 1000;
    const int bw = int(m_uprightBinderShare * total), iw = total - bw;
    m_sideSplit->setSizes(m_uprightBinderLeft ? QList<int>{bw, iw} : QList<int>{iw, bw});
}

void MainWindow::arrangePanes()
{
    if (!m_mainSplit || !m_sideSplit || m_small)
        return;
    setUpdatesEnabled(false);
    // the side row leaves the main splitter first (it has no place there while the window is wide)
    if (m_sideSplit->parentWidget() != this) {
        m_sideSplit->hide();
        m_sideSplit->setParent(this);
        m_sideSplit->hide();
    }
    if (!m_upright) {
        m_mainSplit->setOrientation(Qt::Horizontal);
        for (int i = 0; i < 3; ++i)
            m_mainSplit->insertWidget(i, paneWidget(paneIndex(m_wideOrder.at(i))));
        for (int i = 0; i < 3; ++i)
            m_mainSplit->setStretchFactor(i, m_wideOrder.at(i) == QLatin1Char('E') ? 1 : 0);
    } else {
        m_sideSplit->insertWidget(0, m_uprightBinderLeft ? static_cast<QWidget *>(m_binder) : static_cast<QWidget *>(m_inspector));
        m_sideSplit->insertWidget(1, m_uprightBinderLeft ? static_cast<QWidget *>(m_inspector) : static_cast<QWidget *>(m_binder));
        m_sideSplit->setStretchFactor(0, 1);
        m_sideSplit->setStretchFactor(1, 1);
        m_mainSplit->setOrientation(Qt::Vertical);
        const int ei = m_uprightEditorFirst ? 0 : 1;
        m_mainSplit->insertWidget(ei, m_editorArea);
        m_mainSplit->insertWidget(1 - ei, m_sideSplit);
        // with the on-screen keyboard the window loses height: the editor's row takes it, the side row keeps its height
        m_mainSplit->setStretchFactor(ei, 1);
        m_mainSplit->setStretchFactor(1 - ei, 0);
    }
    // a pane moved to another parent forgets whether it was hidden: set it from the switches
    if (a.contains(QStringLiteral("showBinder")))
        m_binder->setVisible(a[QStringLiteral("showBinder")]->isChecked());
    if (a.contains(QStringLiteral("showInspector")))
        m_inspector->setVisible(a[QStringLiteral("showInspector")]->isChecked());
    m_editorArea->show();
    if (m_upright)
        m_sideSplit->setVisible(!m_binder->isHidden() || !m_inspector->isHidden());
    applyPaneSizes();
    setUpdatesEnabled(true);
    QTimer::singleShot(0, this, [this] { applyPaneSizes(); }); // after the layout of the moved panes
}

void MainWindow::updatePaneRow()
{
    if (!m_upright || m_small)
        return;
    const bool any = !m_binder->isHidden() || !m_inspector->isHidden();
    if (any != !m_sideSplit->isHidden()) {
        m_sideSplit->setVisible(any);
        if (any)
            applyPaneSizes();
    }
}

void MainWindow::followWindowShape()
{
    const bool up = wantsUpright();
    if (up == m_upright)
        return;
    m_upright = up;
    arrangePanes();
    updatePaneActions();
    if (m_small)
        refitSmallPanels(); // the bar goes to the bottom or the right edge, the open panel changes its shape
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    windowPlaceChanged();
    if (m_small)
        QTimer::singleShot(0, this, [this] { refitSmallPanels(); });
    if (wantsUpright() != m_upright && !m_shapeCheckQueued) {
        m_shapeCheckQueued = true;
        QTimer::singleShot(0, this, [this] {
            m_shapeCheckQueued = false;
            followWindowShape();
        });
    }
}

void MainWindow::setWideOrder(const QString &order)
{
    if (!validOrder(order))
        return;
    capturePaneSizes();
    m_wideOrder = order;
    if (!m_upright)
        arrangePanes();
    updatePaneActions();
    savePaneSettings();
}

void MainWindow::setUprightEditorFirst(bool first)
{
    capturePaneSizes();
    m_uprightEditorFirst = first;
    if (m_upright)
        arrangePanes();
    updatePaneActions();
    savePaneSettings();
}

void MainWindow::setUprightBinderLeft(bool left)
{
    capturePaneSizes();
    m_uprightBinderLeft = left;
    if (m_upright)
        arrangePanes();
    updatePaneActions();
    savePaneSettings();
}

void MainWindow::resetPaneLayout()
{
    if (m_upright) {
        m_uprightEditorFirst = true;
        m_uprightBinderLeft = true;
        m_uprightEditorShare = kStartEditorShare;
        m_uprightBinderShare = kStartBinderShare;
    } else {
        m_wideOrder = QStringLiteral("BEI");
        for (int i = 0; i < 3; ++i)
            m_wideSizes[i] = kStartWide[i];
    }
    arrangePanes();
    updatePaneActions();
    savePaneSettings();
}

void MainWindow::savePaneSettings()
{
    capturePaneSizes();
    QSettings s;
    const QList<int> sizes = paneSizes();
    // the width of a hidden pane is not lost: its last width stays
    s.setValue(QStringLiteral("window/mainSplit"),
               QVariantList{sizes[0] > 0 ? sizes[0] : m_wideSizes[0], sizes[1] > 0 ? sizes[1] : m_wideSizes[1],
                            sizes[2] > 0 ? sizes[2] : m_wideSizes[2]});
    s.setValue(QStringLiteral("window/paneOrder"), m_wideOrder);
    s.setValue(QStringLiteral("window/uprightEditorFirst"), m_uprightEditorFirst);
    s.setValue(QStringLiteral("window/uprightBinderLeft"), m_uprightBinderLeft);
    s.setValue(QStringLiteral("window/uprightShares"), QVariantList{m_uprightEditorShare, m_uprightBinderShare});
}

void MainWindow::updatePaneActions()
{
    auto check = [this](const QString &key, bool on) {
        if (QAction *ac = a.value(key))
            ac->setChecked(on);
    };
    check(QStringLiteral("layoutEditorMiddle"), m_uprightEditorFirst);
    check(QStringLiteral("layoutEditorBottom"), !m_uprightEditorFirst);
    check(QStringLiteral("layoutBinderLeft"), m_uprightBinderLeft);
    check(QStringLiteral("layoutBinderRight"), !m_uprightBinderLeft);
    for (const char *k : kOrders)
        check(QStringLiteral("layoutOrder") + QLatin1String(k), m_wideOrder == QLatin1String(k));
}

void MainWindow::createPaneActions()
{
    auto make = [this](const QString &key, const QString &text, QActionGroup *group) {
        auto *ac = new QAction(text, this);
        ac->setObjectName(key);
        ac->setIconText(text);
        if (group) {
            ac->setCheckable(true);
            group->addAction(ac);
        }
        a.insert(key, ac);
        addAction(ac);
        return ac;
    };
    m_paneGroups[0] = new QActionGroup(this);
    m_paneGroups[1] = new QActionGroup(this);
    m_paneGroups[2] = new QActionGroup(this);
    connect(make(QStringLiteral("layoutEditorMiddle"), tr("Editor in the &Middle"), m_paneGroups[0]), &QAction::triggered, this,
            [this] { setUprightEditorFirst(true); });
    connect(make(QStringLiteral("layoutEditorBottom"), tr("Editor at the &Bottom"), m_paneGroups[0]), &QAction::triggered, this,
            [this] { setUprightEditorFirst(false); });
    connect(make(QStringLiteral("layoutBinderLeft"), tr("Binder &Left"), m_paneGroups[1]), &QAction::triggered, this,
            [this] { setUprightBinderLeft(true); });
    connect(make(QStringLiteral("layoutBinderRight"), tr("Binder &Right"), m_paneGroups[1]), &QAction::triggered, this,
            [this] { setUprightBinderLeft(false); });
    const QString texts[6] = {tr("Binder, Editor, Inspector"), tr("Binder, Inspector, Editor"), tr("Editor, Binder, Inspector"),
                              tr("Editor, Inspector, Binder"), tr("Inspector, Binder, Editor"), tr("Inspector, Editor, Binder")};
    int n = 0;
    for (const char *k : kOrders) {
        const QString order = QLatin1String(k);
        connect(make(QStringLiteral("layoutOrder") + order, texts[n++], m_paneGroups[2]), &QAction::triggered, this,
                [this, order] { setWideOrder(order); });
    }
    connect(make(QStringLiteral("layoutReset"), tr("&Reset Layout"), nullptr), &QAction::triggered, this,
            [this] { resetPaneLayout(); });
    // a binder or an inspector shown or hidden: the row of the two in an upright window follows
    connect(a[QStringLiteral("showBinder")], &QAction::toggled, this, [this] { updatePaneRow(); });
    connect(a[QStringLiteral("showInspector")], &QAction::toggled, this, [this] { updatePaneRow(); });
    updatePaneActions();
}

} // namespace qb
