#include "ui/MindMapOverview.h"
#include "ui/MindMapController.h"
#include "ui/MindMapDoc.h"
#include "ui/MindMapScene.h"
#include "ui/MindMapUi.h"
#include "ui/MindMapView.h"

#include <QApplication>
#include <QGuiApplication>
#include <QInputDevice>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSettings>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace qb {

using namespace mindmap;

// ------------------------------------------------------------------ the settings

namespace overview {

namespace {
const char *const kModeKey = "mindMap/overviewAlways";
QString sizeKey(const QString &kind) { return QStringLiteral("mindMap/overviewSize/") + kind; }
} // namespace

bool always()
{
    QSettings s;
    return s.value(QLatin1String(kModeKey), false).toBool();
}

void setAlways(bool on)
{
    if (always() == on)
        return;
    QSettings s;
    s.setValue(QLatin1String(kModeKey), on);
    s.sync();
    emit hub()->modeChanged();
}

QSize storedSize(const QString &kind)
{
    QSettings s;
    const QSize z = s.value(sizeKey(kind)).toSize();
    return z.isValid() ? z : QSize();
}

void setStoredSize(const QString &kind, const QSize &size)
{
    QSettings s;
    s.setValue(sizeKey(kind), size);
    s.sync();
}

OverviewHub *hub()
{
    static OverviewHub *h = new OverviewHub;
    return h;
}

QSize sizeFor(const QSize &vp, const QSize &stored)
{
    const int maxW = std::max(40, vp.width() / 2), maxH = std::max(30, vp.height() / 2);
    int w, h;
    if (stored.isValid()) {
        w = stored.width();
        h = stored.height();
    } else {
        w = std::max(80, vp.width() / 5);
        h = w * 3 / 4;
    }
    w = std::min(std::max(w, std::min(80, maxW)), maxW);
    h = std::min(std::max(h, std::min(60, maxH)), maxH);
    return QSize(w, h);
}

} // namespace overview

// ------------------------------------------------------------------ the widget

namespace {
int gripSize()
{
#ifdef Q_OS_ANDROID
    return 40;
#else
    for (const QInputDevice *d : QInputDevice::devices())
        if (d->type() == QInputDevice::DeviceType::TouchScreen)
            return 40;
    return 22;
#endif
}
} // namespace

MindMapOverview::MindMapOverview(MindMapView *view) : QWidget(view->viewport()), m_view(view)
{
    setObjectName(QStringLiteral("mmOverview"));
    setMouseTracking(true);
    setAttribute(Qt::WA_NoMousePropagation, false);
    setFocusPolicy(Qt::NoFocus);
    setAutoFillBackground(false);
    hide();
}

void MindMapOverview::setKind(const QString &kind)
{
    m_kind = kind;
    refresh();
}

QRectF MindMapOverview::innerRect() const { return QRectF(rect()).adjusted(4, 4, -4, -4); }

QRect MindMapOverview::gripRect() const
{
    const int g = gripSize();
    return QRect(width() - g, height() - g, g, g);
}

QRectF MindMapOverview::area() const
{
    if (!m_view || !m_view->boardScene())
        return QRectF();
    QRectF a = m_view->boardScene()->contentRect();
    return a.united(m_view->visibleRect());
}

MindMapOverview::Map MindMapOverview::map() const
{
    if (m_haveFrozen)
        return m_frozen;
    Map m;
    m.area = area();
    const QRectF in = innerRect();
    if (m.area.isEmpty() || in.isEmpty())
        return m;
    m.scale = std::min(in.width() / m.area.width(), in.height() / m.area.height());
    m.origin = QPointF(in.left() + (in.width() - m.area.width() * m.scale) / 2, in.top() + (in.height() - m.area.height() * m.scale) / 2);
    return m;
}

QRectF MindMapOverview::shownRectInOverview() const
{
    if (!m_view)
        return QRectF();
    const Map m = map();
    const QRectF v = m_view->visibleRect();
    return QRectF(m.origin + (v.topLeft() - m.area.topLeft()) * m.scale, v.size() * m.scale);
}

QPointF MindMapOverview::sceneAt(const QPointF &pos) const
{
    const Map m = map();
    if (m.scale <= 0)
        return QPointF();
    return m.area.topLeft() + (pos - m.origin) / m.scale;
}

bool MindMapOverview::wantedShown() const
{
    if (overview::always())
        return true;
    if (!m_view || !m_view->boardScene())
        return false;
    const QRectF content = m_view->boardScene()->contentRect();
    if (content.isNull())
        return false;
    const QRectF vis = m_view->visibleRect();
    if (!vis.contains(content))
        return true;                     // some of it lies outside the view
    const double margin = 10.0; // scene units: less than the 30 that Zoom to Fit leaves, so that it hides after Zoom to Fit
    if (vis.contains(content.adjusted(-margin, -margin, margin, margin)))
        return false;                    // all of it fits, with a small margin
    return m_shown;                      // in between: as it is (no flicker at the edge)
}

void MindMapOverview::refresh()
{
    if (!m_view)
        return;
    const QSize vp = m_view->viewport()->size();
    const QSize sz = overview::sizeFor(vp, overview::storedSize(m_kind));
    if (!m_resizing && size() != sz)
        resize(sz);
    move(kMargin, kMargin);
    const bool want = wantedShown();
    m_shown = want;
    if (want == isHidden() && !(want && vp.isEmpty())) {
        setVisible(want);
        if (want)
            raise();
    }
    if (want)
        update();
}

void MindMapOverview::wheelEvent(QWheelEvent *e)
{
    // the wheel works as over the board: handed to the view's viewport at the same place
    if (!m_view)
        return;
    QWheelEvent moved(mapToParent(e->position()), e->globalPosition(), e->pixelDelta(), e->angleDelta(), e->buttons(), e->modifiers(),
                      e->phase(), e->inverted(), e->source());
    QApplication::sendEvent(m_view->viewport(), &moved);
    e->accept();
}

void MindMapOverview::moveViewTo(const QPointF &scenePos)
{
    if (m_view)
        m_view->centerOn(scenePos);
    update();
}

void MindMapOverview::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !m_view) {
        e->accept();
        return;
    }
    e->accept();
    if (gripRect().contains(e->position().toPoint())) {
        m_resizing = true;
        return;
    }
    // the map keeps its scale while the view moves
    m_frozen = map();
    m_haveFrozen = true;
    m_dragging = true;
    const QPointF p = sceneAt(e->position());
    if (shownRectInOverview().contains(e->position())) {
        m_grabOffset = p - m_view->visibleCenter();
    } else {
        m_grabOffset = QPointF();
        moveViewTo(p);
    }
}

void MindMapOverview::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();
    if (m_resizing) {
        const QSize want(int(e->position().x()) + 2, int(e->position().y()) + 2);
        const QSize sz = overview::sizeFor(m_view->viewport()->size(), want);
        if (sz != size()) {
            resize(sz);
            update();
        }
        return;
    }
    if (m_dragging) {
        // the pointer may leave the overview, but never the board's area
        QPointF pos = e->position();
        const QRectF in = innerRect();
        pos.setX(std::clamp(pos.x(), in.left(), in.right()));
        pos.setY(std::clamp(pos.y(), in.top(), in.bottom()));
        moveViewTo(sceneAt(pos) - m_grabOffset);
        return;
    }
    setCursor(gripRect().contains(e->position().toPoint()) ? Qt::SizeFDiagCursor
              : shownRectInOverview().contains(e->position()) ? Qt::OpenHandCursor
                                                              : Qt::ArrowCursor);
}

void MindMapOverview::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
    if (m_resizing) {
        m_resizing = false;
        overview::setStoredSize(m_kind, size());
        emit resized(size());
    }
    if (m_dragging) {
        m_dragging = false;
        m_haveFrozen = false;
        update();
    }
}

void MindMapOverview::paintEvent(QPaintEvent *)
{
    if (!m_view || !m_view->boardScene())
        return;
    MindMapScene *sc = m_view->boardScene();
    const Board &b = sc->board();
    const Map m = map();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor ink = b.settings.effectiveTextColor();
    QColor bg = b.settings.background;
    bg.setAlpha(205);
    auto softInk = [&](int alpha) {
        QColor c = ink;
        c.setAlpha(alpha);
        return c;
    };
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
    p.setClipRect(QRectF(rect()).adjusted(1, 1, -1, -1));
    if (m.scale > 0) {
        auto toOv = [&](const QRectF &r) {
            QRectF o(m.origin + (r.topLeft() - m.area.topLeft()) * m.scale, r.size() * m.scale);
            if (o.width() < 2)
                o.setWidth(2);
            if (o.height() < 2)
                o.setHeight(2);
            return o;
        };
        // shapes behind everything
        for (const Shape &s : b.shapes) {
            const QRectF r = toOv(sc->liveRect(s.id));
            p.setOpacity(s.look.faded ? sc->fadedOpacity() : 1.0);
            p.setBrush(s.look.fill.isValid() ? s.look.fill : QColor(0, 0, 0, 0));
            p.setPen(s.look.borderWeight > 0 || !s.look.fill.isValid() ? QPen(s.look.borderColor.isValid() ? s.look.borderColor : softInk(150), 1) : Qt::NoPen);
            p.drawRect(r);
        }
        p.setOpacity(1.0);
        // lines
        QSet<QString> done;
        auto lines = [&](int a, const QSet<int> &conn) {
            for (int other : conn) {
                const QString key = Board::lineKey(a, other);
                if (done.contains(key))
                    continue;
                done.insert(key);
                const QRectF ra = sc->liveRect(a), rb = sc->liveRect(other);
                if (ra.isNull() || rb.isNull())
                    continue;
                const LineLook look = b.lineLook(a, other);
                p.setPen(QPen(look.color.isValid() ? look.color : softInk(190), 1));
                const QRectF oa = toOv(ra), ob = toOv(rb);
                p.drawLine(oa.center(), ob.center());
            }
        };
        for (const Note &n : b.notes)
            lines(n.id, n.connected);
        for (const Shape &s : b.shapes)
            lines(s.id, s.connected);
        // notes: the colored frame, and the text where it can still be read
        for (const Note &n : b.notes) {
            const QRectF r = toOv(sc->liveRect(n.id));
            p.setOpacity(sc->isFaded(n.id) ? sc->fadedOpacity() : 1.0);
            const QColor fill = n.look.fill.isValid() ? n.look.fill : QColor(0, 0, 0, 0);
            p.setBrush(fill);
            p.setPen(QPen(n.look.borderColor.isValid() ? n.look.borderColor : softInk(n.look.fill.isValid() ? 120 : 170), 1));
            p.drawRect(r);
            const double px = n.fontSize * 96.0 / 72.0 * m.scale;
            if (px >= 5.5 && !n.isImage() && !n.text.isEmpty()) {
                QFont f(b.fontFamily());
                f.setPixelSize(std::max(1, int(px)));
                p.setFont(f);
                p.setPen(sc->textColor(n.look));
                p.save();
                p.setClipRect(r.adjusted(1, 1, -1, -1), Qt::IntersectClip);
                p.drawText(r.adjusted(2, 1, -2, -1), Qt::TextWordWrap | Qt::AlignTop | Qt::AlignLeft, n.text);
                p.restore();
            } else if (n.isImage()) {
                p.setBrush(QColor(128, 128, 128, 120));
                p.drawRect(r);
            }
        }
        p.setOpacity(1.0);
        // the part shown now
        const QColor hl = palette().color(QPalette::Highlight);
        QRectF v = shownRectInOverview();
        p.setBrush(QColor(hl.red(), hl.green(), hl.blue(), 45));
        p.setPen(QPen(hl, 1.5));
        p.drawRect(v.adjusted(0.5, 0.5, -0.5, -0.5));
    }
    p.setClipping(false);
    // the grip
    const QRect g = gripRect();
    p.setPen(QPen(softInk(200), 1.2));
    const int e = g.right() - 3, f = g.bottom() - 3;
    for (int i = 0; i < 3; ++i) {
        const int d = 4 + i * 4;
        p.drawLine(e - d, f, e, f - d);
    }
    // the thin frame
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(softInk(200), 1));
    p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
}

} // namespace qb
