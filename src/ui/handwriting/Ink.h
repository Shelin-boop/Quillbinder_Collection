#pragma once
// 0.99.0 (handwriting, item 112): the plain parts of the writing surface that need no widget: strokes, the gesture of
// scratching words out, and the hand protection (the rest zone and the rules for contacts). They are separate so that
// the checks can test them with numbers alone.
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QVector>

namespace qb {
namespace handwriting {

struct InkPoint {
    QPointF p;
    qint64 t = 0;
    qreal pressure = 1.0;
};

struct InkStroke {
    QVector<InkPoint> pts;
    QRectF bounds() const;
    QVector<QPointF> points() const;
    double length() const;
};

// Scratching words out: a stroke that goes back and forth (at least four sweeps, each of a good part of the stroke's
// width), several times longer than it is wide and not tall. `reference` is the height of a line of writing in pixels.
// On success `area` is the stroke's box.
bool isScratchOut(const QVector<QPointF> &points, double reference, QRectF *area = nullptr);

// ---- hand protection
// The starting values of the brief: a rest zone (the writing area plus about 30 mm, wider on the writing hand's side and
// below) that ignores finger touches while the pen writes and about 1 s after; large contacts ignored while the pen is
// used; a short, small tap on a button or a menu always accepted; without a pen, touches in a strip of about 15 mm at the
// screen's edge ignored.
class HandProtection
{
public:
    struct Params {
        bool on = true;
        bool leftHanded = false;
        int strength = 50;          // 0 .. 100; 50 gives the values above
        double pxPerMm = 4.0;       // the screen's density
        qint64 afterPenMs = 1000;   // the rest zone stays this long after the pen lifted
    };
    struct Touch {
        QPointF pos;                // in the coordinates of `surface`
        double contactMm = 0;       // the size of the contact; 0: not known
        bool shortSmallTap = false; // a tap of less than about 0.3 s that did not grow or move
        bool onButton = false;      // the touch is on a button or a menu
    };
    struct Situation {
        bool penWriting = false;    // the tip is down, or lifted less than afterPenMs ago
        bool penInUse = false;      // a pen is in use (near, or used lately)
        bool hasWritingArea = false;
        QRectF writingArea;         // the ink not read yet (and the pen's place)
        QRectF surface;             // the window's or the screen's rectangle for the edge strip
    };

    explicit HandProtection(const Params &p) : m_p(p) {}
    // the rest zone around a writing area
    QRectF restZone(const QRectF &writingArea) const;
    // the size at which a contact counts as a palm, in mm
    double palmSizeMm() const;
    double edgeStripMm() const;
    // true: the touch is ignored
    bool ignores(const Touch &touch, const Situation &situation) const;

private:
    Params m_p;
};

} // namespace handwriting
} // namespace qb
