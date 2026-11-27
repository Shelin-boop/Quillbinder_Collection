#include "MessageLine.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

namespace qb {

MessageLine::MessageLine(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("messageLine"));
    setAutoFillBackground(false);
    auto *l = new QHBoxLayout(this);
    l->setContentsMargins(6, 2, 6, 2);
    l->setSpacing(6);
    m_label = new QLabel(this);
    m_label->setObjectName(QStringLiteral("messageLineText"));
    m_label->setTextFormat(Qt::PlainText);
    m_label->setWordWrap(true);
    m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    l->addWidget(m_label, 1);
    m_link = new QLabel(this);
    m_link->setObjectName(QStringLiteral("messageLineLink"));
    m_link->setTextFormat(Qt::RichText);
    m_link->hide();
    l->addWidget(m_link);
    connect(m_link, &QLabel::linkActivated, this, [this] {
        const auto fn = m_action;
        dismiss();
        if (fn)
            fn();
    });
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &MessageLine::dismiss);
    hide();
}

void MessageLine::post(const QString &text, int ms, const QString &linkText, std::function<void()> linkAction, const QString &tip)
{
    if (text.isEmpty())
        return;
    Msg m;
    m.text = text;
    m.linkText = linkText;
    m.action = std::move(linkAction);
    m.tip = tip;
    m.ms = ms > 0 ? ms : (linkText.isEmpty() ? 5000 : 10000);
    m_queue.enqueue(m);
    if (m_text.isEmpty())
        showNext();
}

void MessageLine::showNext()
{
    if (m_queue.isEmpty()) {
        m_text.clear();
        m_action = nullptr;
        m_shownMs = 0;
        m_timer->stop();
        m_link->hide();
        if (!m_status.isEmpty()) {   // 0.99.0: the standing text
            m_label->setText(m_status);
            m_label->setToolTip(QString());
            setToolTip(QString());
            show();
            return;
        }
        m_label->clear();
        hide();
        return;
    }
    const Msg m = m_queue.dequeue();
    m_text = m.text;
    m_action = m.action;
    m_shownMs = m.ms;
    m_label->setText(m.text);
    m_label->setToolTip(m.tip);
    setToolTip(m.tip);
    if (!m.linkText.isEmpty()) {
        m_link->setText(QStringLiteral("<a href=\"#\">%1</a>").arg(m.linkText.toHtmlEscaped()));
        m_link->setToolTip(m.tip);
        m_link->show();
    } else {
        m_link->hide();
    }
    show();
    m_timer->start(m.ms);
}

void MessageLine::setStatus(const QString &text)
{
    if (m_status == text)
        return;
    m_status = text;
    if (m_text.isEmpty())   // no timed message is shown: show or hide the standing text now
        showNext();
}

void MessageLine::dismiss()
{
    m_timer->stop();
    m_text.clear();
    showNext();
}

void MessageLine::clearAll()
{
    m_queue.clear();
    dismiss();
}

void MessageLine::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QColor c = palette().color(QPalette::Highlight);
    c.setAlpha(40);
    p.fillRect(rect(), c);
    p.setPen(palette().color(QPalette::Mid));
    p.drawLine(0, 0, width(), 0);
}

void MessageLine::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && !m_text.isEmpty()) {
        dismiss();
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

} // namespace qb
