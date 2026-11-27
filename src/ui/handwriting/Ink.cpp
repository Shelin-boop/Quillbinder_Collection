#include "ui/handwriting/Ink.h"

#include <QLineF>
#include <QtMath>

namespace qb {
namespace handwriting {

QRectF InkStroke::bounds() const
{
    if (pts.isEmpty())
        return QRectF();
    qreal l = pts[0].p.x(), r = l, t = pts[0].p.y(), b = t;
    for (const InkPoint &q : pts) {
        l = qMin(l, q.p.x());
        r = qMax(r, q.p.x());
        t = qMin(t, q.p.y());
        b = qMax(b, q.p.y());
    }
    return QRectF(QPointF(l, t), QPointF(r, b));
}

QVector<QPointF> InkStroke::points() const
{
    QVector<QPointF> v;
    v.reserve(pts.size());
    for (const InkPoint &q : pts)
        v << q.p;
    return v;
}

double InkStroke::length() const
{
    double len = 0;
    for (int i = 1; i < pts.size(); ++i)
        len += QLineF(pts[i - 1].p, pts[i].p).length();
    return len;
}

bool isScratchOut(const QVector<QPointF> &points, double reference, QRectF *area)
{
    if (points.size() < 8)
        return false;
    qreal l = points[0].x(), r = l, t = points[0].y(), b = t;
    double len = 0;
    for (int i = 0; i < points.size(); ++i) {
        l = qMin(l, points[i].x());
        r = qMax(r, points[i].x());
        t = qMin(t, points[i].y());
        b = qMax(b, points[i].y());
        if (i)
            len += QLineF(points[i - 1], points[i]).length();
    }
    const double w = r - l, h = b - t;
    if (w < qMax(24.0, reference * 0.8) || h > qMax(w, reference * 1.6))
        return false;
    if (len < 3.0 * w)
        return false;
    // the sweeps: x goes one way for at least a quarter of the width, then back
    const double hysteresis = qMax(6.0, w * 0.25);
    int sweeps = 0, dir = 0;
    double lo = points[0].x(), hi = lo, extreme = lo;
    for (const QPointF &q : points) {
        const double x = q.x();
        if (dir == 0) {
            lo = qMin(lo, x);
            hi = qMax(hi, x);
            if (x - lo >= hysteresis) { dir = 1; extreme = x; ++sweeps; }
            else if (hi - x >= hysteresis) { dir = -1; extreme = x; ++sweeps; }
        } else if (dir == 1) {
            if (x > extreme) extreme = x;
            else if (extreme - x >= hysteresis) { dir = -1; extreme = x; ++sweeps; }
        } else {
            if (x < extreme) extreme = x;
            else if (x - extreme >= hysteresis) { dir = 1; extreme = x; ++sweeps; }
        }
    }
    if (sweeps < 4)
        return false;
    if (area)
        *area = QRectF(QPointF(l, t), QPointF(r, b));
    return true;
}

// ---- hand protection

QRectF HandProtection::restZone(const QRectF &writingArea) const
{
    const double f = 0.5 + m_p.strength / 100.0;      // 1.0 at 50
    const double mm = m_p.pxPerMm * f;
    const double side = 30 * mm, hand = 45 * mm, below = 50 * mm;
    const double left = m_p.leftHanded ? hand : side;
    const double right = m_p.leftHanded ? side : hand;
    return writingArea.adjusted(-left, -side, right, below);
}

double HandProtection::palmSizeMm() const { return 14.0 - m_p.strength * 0.08; }
double HandProtection::edgeStripMm() const { return 15.0 * (0.5 + m_p.strength / 100.0); }

bool HandProtection::ignores(const Touch &touch, const Situation &s) const
{
    if (!m_p.on)
        return false;
    // a short, small tap on a button or a menu is always accepted
    if (touch.onButton && touch.shortSmallTap)
        return false;
    if (s.penWriting && s.hasWritingArea && restZone(s.writingArea).contains(touch.pos))
        return true;
    if (s.penInUse && touch.contactMm > 0 && touch.contactMm >= palmSizeMm())
        return true;
    if (!s.penInUse && s.surface.isValid()) {
        const double strip = edgeStripMm() * m_p.pxPerMm;
        if (!s.surface.adjusted(strip, strip, -strip, -strip).contains(touch.pos))
            return true;
    }
    return false;
}

} // namespace handwriting
} // namespace qb
