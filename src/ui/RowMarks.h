#pragma once
// 0.92.0: the two small marks at the right end of a binder row, painted by the program itself (no picture files):
// the pin of the fixed mind map and the padlock of a document another device works on.
#include <QColor>
#include <QIcon>
#include <QRect>

class QPainter;

namespace qb {
namespace rowmarks {

constexpr int kSide = 14;      // a mark's side
constexpr int kLabelRoom = 22; // the label dot's place at the row's right end

// The places of the marks in a row (rowRect: the row in viewport coordinates; right: the right edge that can be seen).
QRect lockRect(const QRect &rowRect, int right);
QRect pinRect(const QRect &rowRect, int right);

void paintPin(QPainter *p, const QRectF &r, const QColor &color, bool filled);
void paintLock(QPainter *p, const QRectF &r, const QColor &color);
// 0.99.0 (Live Sync): a small pen, the mark of a document this device edits itself
void paintPen(QPainter *p, const QRectF &r, const QColor &color);
QIcon pinIcon(const QColor &color, bool filled);

} // namespace rowmarks
} // namespace qb
