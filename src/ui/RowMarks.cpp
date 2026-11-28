#include "ui/RowMarks.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace qb {
namespace rowmarks {

QRect lockRect(const QRect &rowRect, int right)
{
    return QRect(right - kLabelRoom - kSide, rowRect.center().y() - kSide / 2, kSide, kSide);
}

QRect pinRect(const QRect &rowRect, int right)
{
    return QRect(right - kLabelRoom - 2 * kSide - 4, rowRect.center().y() - kSide / 2, kSide, kSide);
}

// A push pin seen from the side: head, body, base plate, needle.
void paintPin(QPainter *p, const QRectF &r, const QColor &color, bool filled)
{
    const qreal w = r.width(), h = r.height();
    auto pt = [&](qreal x, qreal y) { return QPointF(r.left() + x * w, r.top() + y * h); };
    QPainterPath body;
    body.moveTo(pt(0.30, 0.06));
    body.lineTo(pt(0.70, 0.06));
    body.lineTo(pt(0.70, 0.20));
    body.lineTo(pt(0.62, 0.20));
    body.lineTo(pt(0.66, 0.50));
    body.lineTo(pt(0.86, 0.62));
    body.lineTo(pt(0.14, 0.62));
    body.lineTo(pt(0.34, 0.50));
    body.lineTo(pt(0.38, 0.20));
    body.lineTo(pt(0.30, 0.20));
    body.closeSubpath();
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    QPen pen(color, qMax<qreal>(1.0, w / 12.0));
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    p->setPen(pen);
    p->setBrush(filled ? QBrush(color) : Qt::NoBrush);
    p->drawPath(body);
    p->drawLine(pt(0.50, 0.62), pt(0.50, 0.97));
    p->restore();
}

void paintLock(QPainter *p, const QRectF &r, const QColor &color)
{
    const qreal w = r.width(), h = r.height();
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    // the shackle
    QPen pen(color, qMax<qreal>(1.2, w / 9.0));
    pen.setCapStyle(Qt::FlatCap);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    QPainterPath shackle;
    shackle.moveTo(r.left() + 0.30 * w, r.top() + 0.48 * h);
    shackle.lineTo(r.left() + 0.30 * w, r.top() + 0.30 * h);
    shackle.arcTo(QRectF(r.left() + 0.30 * w, r.top() + 0.08 * h, 0.40 * w, 0.44 * h), 180, -180);
    shackle.lineTo(r.left() + 0.70 * w, r.top() + 0.48 * h);
    p->drawPath(shackle);
    // the body
    p->setPen(Qt::NoPen);
    p->setBrush(color);
    p->drawRoundedRect(QRectF(r.left() + 0.16 * w, r.top() + 0.46 * h, 0.68 * w, 0.48 * h), w / 9.0, w / 9.0);
    p->restore();
}

void paintPen(QPainter *p, const QRectF &r, const QColor &color)
{
    const qreal w = r.width(), h = r.height();
    auto pt = [&](qreal x, qreal y) { return QPointF(r.left() + x * w, r.top() + y * h); };
    // a slanted pen: body, tip, and a line under it
    QPainterPath body;
    body.moveTo(pt(0.72, 0.08));
    body.lineTo(pt(0.92, 0.28));
    body.lineTo(pt(0.38, 0.82));
    body.lineTo(pt(0.12, 0.88));
    body.lineTo(pt(0.18, 0.62));
    body.closeSubpath();
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    QPen pen(color, qMax<qreal>(1.0, w / 12.0));
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    p->setPen(pen);
    p->setBrush(color);
    p->drawPath(body);
    p->setBrush(Qt::NoBrush);
    p->drawLine(pt(0.10, 0.96), pt(0.60, 0.96));
    p->restore();
}

QIcon pinIcon(const QColor &color, bool filled)
{
    QIcon icon;
    for (int side : {16, 18, 24, 32, 48}) {
        QPixmap pm(side, side);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        paintPin(&p, QRectF(1, 1, side - 2, side - 2), color, filled);
        p.end();
        icon.addPixmap(pm);
    }
    return icon;
}

} // namespace rowmarks
} // namespace qb
