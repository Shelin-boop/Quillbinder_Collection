#include "ui/SmallInterface.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPropertyAnimation>
#include <QScreen>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

namespace qb {
namespace smallui {

QString choiceKey(Choice c)
{
    return c == Choice::Full ? QStringLiteral("full") : c == Choice::Small ? QStringLiteral("small") : QStringLiteral("auto");
}

Choice choiceFromKey(const QString &key)
{
    if (key == QLatin1String("full"))
        return Choice::Full;
    if (key == QLatin1String("small"))
        return Choice::Small;
    return Choice::Automatic;
}

bool belowFullHd(const QSize &px)
{
    const int longer = std::max(px.width(), px.height()), shorter = std::min(px.width(), px.height());
    return longer < 1920 || shorter < 1080;
}

bool resolve(Choice choice, const QSize &px)
{
    if (choice == Choice::Full)
        return false;
    if (choice == Choice::Small)
        return true;
    return belowFullHd(px);
}

QSize physicalSize(const QScreen *screen)
{
    if (!screen)
        return QSize();
    const double r = screen->devicePixelRatio();
    return QSize(qRound(screen->size().width() * r), qRound(screen->size().height() * r));
}

int touchTargetPx(double dpi, double mm)
{
    return qMax(1, qRound(dpi * mm / 25.4));
}

// ---------------------------------------------------------------- the panel

SlidePanel::SlidePanel(const QString &title, QWidget *parent) : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    auto *head = new QWidget(this);
    auto *hl = new QHBoxLayout(head);
    hl->setContentsMargins(8, 2, 4, 2);
    m_title = new QLabel(title, head);
    QFont f = m_title->font();
    f.setBold(true);
    m_title->setFont(f);
    m_close = new QToolButton(head);
    m_close->setObjectName(QStringLiteral("smallPanelClose"));
    m_close->setText(tr("Close"));
    m_close->setToolTip(tr("Close this panel (Esc)"));
    m_close->setAutoRaise(false);
    hl->addWidget(m_title, 1);
    hl->addWidget(m_close);
    outer->addWidget(head);
    m_holder = new QWidget(this);
    m_holderLayout = new QVBoxLayout(m_holder);
    m_holderLayout->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_holder, 1);
    connect(m_close, &QToolButton::clicked, this, [this] { closePanel(true); });
    hide();
}

void SlidePanel::setContent(QWidget *content)
{
    // (the window may have put the widget back in its splitter since: it is taken again whenever it is not here)
    m_content = content;
    if (content && content->parentWidget() != m_holder)
        m_holderLayout->addWidget(content); // (reparents it; the layout takes it over)
}

void SlidePanel::setTargetSize(int touchPx)
{
    m_close->setMinimumSize(qMax(touchPx * 2, 1), touchPx);
}

QRect SlidePanel::targetRect(const QRect &area, Edge edge) const
{
    if (edge == Edge::Sheet)
        return area;
    // landscape: a good part of the width, never the whole of it (the text stays in sight beside it)
    const int w = std::min(area.width(), std::max(320, area.width() * 45 / 100));
    return edge == Edge::Left ? QRect(area.left(), area.top(), w, area.height())
                              : QRect(area.right() + 1 - w, area.top(), w, area.height());
}

void SlidePanel::openIn(const QRect &area, Edge edge, bool animate)
{
    m_edge = edge;
    const QRect to = targetRect(area, edge);
    if (m_anim)
        m_anim->stop();
    m_open = true;
    setGeometry(to);
    if (m_content)
        m_content->show();
    show();
    raise();
    if (animate) {
        QRect from = to;
        if (edge == Edge::Left)
            from.moveRight(area.left() - 1);
        else if (edge == Edge::Right)
            from.moveLeft(area.right() + 1);
        else
            from.moveTop(area.bottom() + 1);
        if (!m_anim) {
            m_anim = new QPropertyAnimation(this, "geometry", this);
            m_anim->setDuration(120);
            connect(m_anim, &QPropertyAnimation::finished, this, &SlidePanel::settled);
        }
        m_anim->setStartValue(from);
        m_anim->setEndValue(to);
        m_anim->start();
    } else {
        emit settled();
    }
    setFocus();
    emit opened();
}

void SlidePanel::closePanel(bool animate)
{
    if (!m_open)
        return;
    m_open = false;
    if (m_anim)
        m_anim->stop();
    Q_UNUSED(animate)
    if (m_content)
        m_content->hide(); // (so that a later show() of the content is seen by the window)
    hide(); // (a closing slide would keep the panel in the way of the next click; it just goes)
    emit closed();
}

void SlidePanel::refit(const QRect &area, Edge edge)
{
    if (!m_open)
        return;
    m_edge = edge;
    if (m_anim)
        m_anim->stop();
    setGeometry(targetRect(area, edge));
    emit settled();
}

void SlidePanel::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        closePanel(true);
        e->accept();
        return;
    }
    QFrame::keyPressEvent(e);
}

} // namespace smallui
} // namespace qb
