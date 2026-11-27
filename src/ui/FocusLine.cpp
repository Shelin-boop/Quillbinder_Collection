#include "ui/FocusLine.h"
#include "ui/Copyholder.h"
#include "ui/Corkboard.h"
#include "ui/MediaView.h"
#include "ui/TextColumnView.h"
#include "ui/EditorPane.h"
#include "ui/Outliner.h"
#include "ui/Theme.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>

namespace qb {

FocusLine::FocusLine(QWidget *window, QWidget *binder, QWidget *inspector, std::function<QList<EditorPane *>()> panes)
    : QWidget(window), m_window(window), m_binder(binder), m_inspector(inspector), m_panes(std::move(panes))
{
    setObjectName(QStringLiteral("focusLine"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::NoFocus);
    window->installEventFilter(this);
    hide();
    m_focusLink = connect(qApp, &QApplication::focusChanged, this, [this](QWidget *, QWidget *) { refresh(); });
    connect(&theme::Notifier::instance(), &theme::Notifier::changed, this, [this] { update(); });
}

void FocusLine::stopListening() { disconnect(m_focusLink); }

QColor FocusLine::lineColor() const { return theme::color(theme::Role::Highlight); }

QString FocusLine::partOf(QWidget *focus, QWidget **target) const
{
    auto found = [&](const QString &name, QWidget *w) {
        if (target)
            *target = w;
        return name;
    };
    if (target)
        *target = nullptr;
    if (!focus || !m_window || focus->window() != m_window.data())
        return QString();
    if (m_binder && m_binder->isAncestorOf(focus))
        return found(QStringLiteral("binder"), m_binder);
    if (m_inspector && m_inspector->isAncestorOf(focus))
        return found(QStringLiteral("inspector"), m_inspector);
    for (EditorPane *p : m_panes()) {
        if (!p || !p->isVisible() || !p->isAncestorOf(focus))
            continue;
        if (p->hasCopyholder() && p->copyholder() && p->copyholder()->isAncestorOf(focus))
            return found(QStringLiteral("copyholder"), p->copyholder());
        if (p->corkboard() && p->corkboard()->isAncestorOf(focus))
            return found(QStringLiteral("corkboard"), p->corkboard());
        if (p->outliner() && p->outliner()->isAncestorOf(focus))
            return found(QStringLiteral("outliner"), p->outliner());
        if (p->textView() && p->textView()->isAncestorOf(focus))
            return found(QStringLiteral("editor"), p->textView());
        if (p->media() && p->media()->isAncestorOf(focus))
            return found(QStringLiteral("editor"), p->media());
    }
    return QString();
}

void FocusLine::refresh()
{
    QWidget *target = nullptr;
    const QString part = partOf(QApplication::focusWidget(), &target);
    setTarget(part, target);
}

void FocusLine::setTarget(const QString &part, QWidget *target)
{
    if (m_target != target) {
        if (m_target)
            m_target->removeEventFilter(this);
        m_target = target;
        if (m_target)
            m_target->installEventFilter(this);
    }
    m_part = target ? part : QString();
    place();
}

void FocusLine::place()
{
    if (!m_window)
        return;
    if (!m_target || !m_target->isVisible() || m_part.isEmpty()) {
        m_rect = QRect();
        hide();
        return;
    }
    setGeometry(m_window->rect());
    m_rect = QRect(m_target->mapTo(m_window, QPoint(0, 0)), m_target->size());
    show();
    raise();
    update();
}

bool FocusLine::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
    case QEvent::LayoutRequest:
        if (o == m_window.data() || o == m_target.data())
            QMetaObject::invokeMethod(this, [this] { refresh(); }, Qt::QueuedConnection);
        break;
    default:
        break;
    }
    return false;
}

void FocusLine::paintEvent(QPaintEvent *)
{
    if (m_part.isEmpty() || m_rect.isEmpty())
        return;
    QPainter p(this);
    // inside the part, so that a neighbor's edge does not cover it
    p.fillRect(QRect(m_rect.left(), m_rect.top(), m_rect.width(), kWidth), lineColor());
    p.fillRect(QRect(m_rect.left(), m_rect.bottom() - kWidth + 1, m_rect.width(), kWidth), lineColor());
    p.fillRect(QRect(m_rect.left(), m_rect.top(), kWidth, m_rect.height()), lineColor());
    p.fillRect(QRect(m_rect.right() - kWidth + 1, m_rect.top(), kWidth, m_rect.height()), lineColor());
}

} // namespace qb
