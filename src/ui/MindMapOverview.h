#pragma once
// The overview of a mind map board (0.93.0): a small map in the top left corner of the board's view, drawn over the
// board (a child of the view's viewport, so it takes the mouse itself). It shows all of the board small and a
// rectangle for the part shown now. A click moves the view there; dragging the rectangle moves the view with it;
// the zoom stays. It belongs to the view, not to the board: exports, prints and the board's picture never hold it.
// "Automatic" shows it while some of the content lies outside the view, "always" shows it always; both and the
// size are kept per device in the program's settings (not in the project).

#include <QPointer>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QWidget>

namespace qb {

class MindMapView;

class OverviewHub : public QObject
{
    Q_OBJECT
signals:
    void modeChanged();
};

namespace overview {
// the one setting for all boards: false = automatic, true = always shown
bool always();
void setAlways(bool on);
// the size the writer chose for a kind of view ("inspector", "window"); invalid: none chosen yet
QSize storedSize(const QString &kind);
void setStoredSize(const QString &kind, const QSize &size);
// emits modeChanged() when "automatic" / "always" changes (all overviews and the commands follow)
OverviewHub *hub();
// the size the overview has in a view of this size: the stored one, else a fifth of the width (at least 80 px),
// never more than half of the view
QSize sizeFor(const QSize &viewport, const QSize &stored);
} // namespace overview

class MindMapOverview : public QWidget
{
    Q_OBJECT
public:
    explicit MindMapOverview(MindMapView *view);
    void setKind(const QString &kind);
    QString kind() const { return m_kind; }
    // follows the view: the place, the size, whether it is shown, the picture
    void refresh();
    // what the rule says (automatic / always) for the board and the part shown now
    bool wantedShown() const;
    // the board's area the overview shows (the content and the shown part together) and where things are in it
    QRectF area() const;
    QRectF shownRectInOverview() const;       // the rectangle of the part shown, in the overview's coordinates
    QRectF innerRect() const;                 // the part of the overview the map is drawn in
    QPointF sceneAt(const QPointF &pos) const; // a point of the overview as a point of the board
    QRect gripRect() const;                   // the grip in the lower right corner
    bool dragging() const { return m_dragging || m_resizing; }
    static constexpr int kMargin = 6;         // from the view's corner

signals:
    void resized(QSize size);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    struct Map {
        QRectF area;
        double scale = 1;
        QPointF origin;   // where the area's top left lies in the overview
    };
    Map map() const;
    void moveViewTo(const QPointF &scenePos);
    QPointer<MindMapView> m_view;
    QString m_kind = QStringLiteral("inspector");
    bool m_shown = false;      // the last decision of "automatic" (it has a margin, so it must be remembered)
    bool m_dragging = false, m_resizing = false;
    QPointF m_grabOffset;
    bool m_haveFrozen = false;
    Map m_frozen;
};

} // namespace qb
