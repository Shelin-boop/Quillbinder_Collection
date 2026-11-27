// 0.99.0 (item 114): Read Aloud in the main window: the switch of its toolbar and the toolbar itself (ReadAloudController.h).
#include "ui/MainWindow.h"

#include "ui/GripBar.h"
#include "ui/OptionsDialog.h"
#include "ui/ReadAloudController.h"
#include "ui/Theme.h"

#include <QAction>
#include <QMenu>

namespace qb {

void MainWindow::createReadAloudActions()
{
    auto *ac = new QAction(tr("&Read Aloud"), this);
    ac->setObjectName(QStringLiteral("showBarReadAloud"));
    ac->setIconText(tr("Read Aloud"));
    ac->setCheckable(true);
    ac->setChecked(readaloud::Config::instance().get().barShown);
    a.insert(QStringLiteral("showBarReadAloud"), ac);
    addAction(ac);
    connect(ac, &QAction::toggled, this, [this](bool on) {
        if (m_applyingBars)
            return;
        if (m_readAloud)
            m_readAloud->setBarShown(on);
        else {
            readaloud::Settings s = readaloud::Config::instance().get();
            s.barShown = on;
            readaloud::Config::instance().set(s);
        }
        applyBarVisibility();
    });
}

void MainWindow::createReadAloudBar()
{
    readaloud::useMediaOutput();
    m_readAloud = new readaloud::Controller(this, [this]() -> TextEditor * { return handwritingEditor(); });
    GripBar *bar = m_readAloud->createBar();
    m_readAloudBar = bar;
    addToolBar(Qt::TopToolBarArea, bar);
    bar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bar, &QWidget::customContextMenuRequested, this, [this, bar](const QPoint &pos) {
        QMenu m;
        m.setObjectName(QStringLiteral("barsMenu"));
        fillBarsMenu(&m, -1);
        m.exec(bar->mapToGlobal(pos));
    });
    connect(m_readAloud, &readaloud::Controller::settingsRequested, this,
            [this] { OptionsDialog::run(this, [](OptionsDialog &d) { d.showSection(OptionsDialog::SGeneral, tr("Read Aloud")); }); });
    connect(m_readAloud, &readaloud::Controller::message, this, [this](const QString &text) { showStatusNote(text, 8000); });
    connect(&readaloud::Config::instance(), &readaloud::Config::changed, this, [this] { applyBarVisibility(); });
    bar->setVisible(readaloud::Config::instance().get().barShown);
}

} // namespace qb
