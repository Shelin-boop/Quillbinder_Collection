#include "ui/handwriting/InkCanvas.h"

#include "ui/PenInput.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QScrollBar>
#include <QTabletEvent>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QTouchEvent>
#include <QWheelEvent>
#include <QtMath>

namespace qb {
namespace handwriting {

namespace {
constexpr int kPenId = -1;
constexpr int kMouseId = -2;
constexpr qint64 kTapMs = 300;
constexpr qint64 kLongPressMs = 500;
constexpr double kTapSlop = 8.0;
}

InkCanvas::InkCanvas(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("inkCanvas"));
    setAttribute(Qt::WA_AcceptTouchEvents);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    m_s = HandwritingConfig::instance().get();
    connect(&HandwritingConfig::instance(), &HandwritingConfig::changed, this, [this] {
        m_s = HandwritingConfig::instance().get();
        update();
    });
    m_timer.setInterval(30);
    connect(&m_timer, &QTimer::timeout, this, &InkCanvas::advance);
}

void InkCanvas::setRole(Role role)
{
    m_role = role;
    update();
}

void InkCanvas::setGridRect(const QRectF &r)
{
    m_grid = r;
    m_gridSet = true;
    update();
}

QRectF InkCanvas::gridRect() const
{
    if (m_gridSet)
        return m_grid;
    return m_role == Role::Pad ? QRectF(rect()) : QRectF();
}

void InkCanvas::setSettings(const HandwritingSettings &s)
{
    m_s = s;
    update();
}

double InkCanvas::pxPerMm() const { return logicalDpiX() / 25.4; }

double InkCanvas::lineReference() const
{
    return m_s.grid.style == GridSettings::None || m_s.grid.style == GridSettings::Dots ? 36.0 : m_s.grid.pitch();
}

QColor InkCanvas::themeText() const { return palette().color(QPalette::Text); }
QColor InkCanvas::inkColor() const { return m_s.inkColor.isValid() ? m_s.inkColor : themeText(); }

bool InkCanvas::anyWriting() const
{
    for (const Pointer &p : m_ptr)
        if (p.action == Action::Writing)
            return true;
    return false;
}

void InkCanvas::wake()
{
    if (!m_timer.isActive())
        m_timer.start();
}

// ---------------------------------------------------------------- the state

int InkCanvas::pendingLines() const
{
    int n = 0;
    for (const Line &l : m_lines)
        if (l.state == Line::Collecting)
            ++n;
    return n;
}

int InkCanvas::visibleLines() const { return int(m_lines.size()); }

int InkCanvas::strokeCount() const
{
    int n = 0;
    for (const Line &l : m_lines)
        n += int(l.strokes.size());
    return n;
}

double InkCanvas::lineAlpha(int index) const
{
    if (index < 0 || index >= m_lines.size())
        return 0;
    const Line &l = m_lines[index];
    if (l.state != Line::Shown)
        return 1.0;
    const qint64 el = PenInput::instance().now() - l.readAt;
    if (el <= m_s.stayMs)
        return 1.0;
    if (m_s.fadeMs <= 0)
        return 0.0;
    return qBound(0.0, 1.0 - double(el - m_s.stayMs) / m_s.fadeMs, 1.0);
}

QString InkCanvas::lineState(int index) const
{
    if (index < 0 || index >= m_lines.size())
        return QString();
    switch (m_lines[index].state) {
    case Line::Collecting: return QStringLiteral("collecting");
    case Line::Reading: return QStringLiteral("reading");
    case Line::Shown: break;
    }
    return QStringLiteral("shown");
}

QRectF InkCanvas::writingArea() const
{
    QRectF a;
    bool any = false;
    auto add = [&](const QRectF &r) {
        a = any ? a.united(r) : r;
        any = true;
    };
    for (const Line &l : m_lines)
        if (l.state != Line::Shown)
            add(l.bounds);
    for (const Pointer &p : m_ptr)
        if (p.action == Action::Writing && !p.stroke.pts.isEmpty())
            add(p.stroke.bounds());
    if (m_hasPenPos)
        add(QRectF(m_penPos.x() - 40, m_penPos.y() - 40, 80, 80));
    return any ? a : QRectF();
}

HandProtection InkCanvas::protection() const
{
    HandProtection::Params p;
    p.on = m_s.protectHand;
    p.leftHanded = m_s.leftHanded;
    p.strength = m_s.strength;
    p.pxPerMm = pxPerMm();
    return HandProtection(p);
}

QRectF InkCanvas::restZone() const
{
    const QRectF a = writingArea();
    if (a.isNull() && !m_hasPenPos)
        return QRectF();
    return protection().restZone(a);
}

void InkCanvas::clearInk()
{
    m_lines.clear();
    for (auto it = m_ptr.begin(); it != m_ptr.end(); ++it)
        it->stroke = InkStroke();
    update();
    emit inkChanged();
}

// ---------------------------------------------------------------- events

bool InkCanvas::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        return touchEvent(static_cast<QTouchEvent *>(e));
    default:
        break;
    }
    return QWidget::event(e);
}

void InkCanvas::tabletEvent(QTabletEvent *e)
{
    PenInput::instance().feedTablet(e);
    const QPointF pos = e->position();
    switch (e->type()) {
    case QEvent::TabletPress: {
        const bool side = (e->buttons() & (Qt::RightButton | Qt::MiddleButton)) != 0;
        m_penPos = pos;
        m_hasPenPos = true;
        pointerDown(kPenId, Source::Pen, pos, side, 0, e->pressure());
        break;
    }
    case QEvent::TabletMove:
        m_penPos = pos;
        m_hasPenPos = true;
        if (m_ptr.contains(kPenId))
            pointerMove(kPenId, pos, e->pressure());
        break;
    case QEvent::TabletRelease:
        if (m_ptr.contains(kPenId))
            pointerUp(kPenId, pos);
        break;
    default:
        break;
    }
    e->accept();
}

bool InkCanvas::touchEvent(QTouchEvent *e)
{
    if (e->type() == QEvent::TouchCancel) {
        for (int id : m_ptr.keys())
            if (id >= 0)
                pointerCancel(id);
        e->accept();
        return true;
    }
    for (const QEventPoint &pt : e->points()) {
        const QPointF pos = pt.scenePosition() - QPointF(mapTo(window(), QPoint(0, 0)));
        const int id = pt.id() < 0 ? 0 : pt.id();
        switch (pt.state()) {
        case QEventPoint::Pressed: {
            const QSizeF d = pt.ellipseDiameters();
            const double mm = qMax(d.width(), d.height()) / pxPerMm();
            pointerDown(id, Source::Finger, pos, false, mm, 1.0);
            break;
        }
        case QEventPoint::Updated:
            if (m_ptr.contains(id))
                pointerMove(id, pos, 1.0);
            break;
        case QEventPoint::Released:
            if (m_ptr.contains(id))
                pointerUp(id, pos);
            break;
        default:
            break;
        }
    }
    e->accept();
    return true;
}

void InkCanvas::mousePressEvent(QMouseEvent *e)
{
    if (e->source() != Qt::MouseEventNotSynthesized || e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    pointerDown(kMouseId, Source::Mouse, e->position(), false, 0, 1.0);
    e->accept();
}

void InkCanvas::mouseMoveEvent(QMouseEvent *e)
{
    if (m_ptr.contains(kMouseId))
        pointerMove(kMouseId, e->position(), 1.0);
    e->accept();
}

void InkCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_ptr.contains(kMouseId))
        pointerUp(kMouseId, e->position());
    e->accept();
}

void InkCanvas::wheelEvent(QWheelEvent *e)
{
    if (QTextEdit *ed = m_finder ? m_finder(e->position().toPoint()) : nullptr) {
        scrollBy(ed, e->angleDelta().y() / 2.0);
    }
    e->accept();
}

// ---------------------------------------------------------------- the pipeline

QPointF InkCanvas::viewportPoint(QTextEdit *ed, const QPointF &p) const
{
    return QPointF(ed->viewport()->mapFromGlobal(mapToGlobal(p.toPoint()))) + (p - p.toPoint());
}

int InkCanvas::positionAt(QTextEdit *ed, const QPointF &p) const
{
    return ed->cursorForPosition(viewportPoint(ed, p).toPoint()).position();
}

void InkCanvas::placeCursor(QTextEdit *ed, const QPointF &p)
{
    QTextCursor c = ed->textCursor();
    c.setPosition(positionAt(ed, p));
    ed->setTextCursor(c);
    ed->setFocus(Qt::OtherFocusReason);
    ++m_placed;
    emit cursorPlaced(ed);
}

void InkCanvas::selectTo(Pointer &p, const QPointF &pos)
{
    if (!p.target || p.anchor < 0)
        return;
    QTextCursor c = p.target->textCursor();
    c.setPosition(p.anchor);
    c.setPosition(positionAt(p.target, pos), QTextCursor::KeepAnchor);
    p.target->setTextCursor(c);
}

QScrollBar *InkCanvas::scrollBarFor(QTextEdit *ed)
{
    for (QWidget *w = ed; w; w = w->parentWidget())
        if (auto *sa = qobject_cast<QAbstractScrollArea *>(w))
            if (sa->verticalScrollBar() && sa->verticalScrollBar()->maximum() > 0)
                return sa->verticalScrollBar();
    return ed->verticalScrollBar();
}

void InkCanvas::scrollBy(QTextEdit *ed, double dy)
{
    if (!ed)
        return;
    QScrollBar *sb = scrollBarFor(ed);
    sb->setValue(sb->value() - int(qRound(dy)));
}

bool InkCanvas::nearUnreadInk(const QPointF &pos) const
{
    for (const Line &l : m_lines)
        if (l.state == Line::Collecting && l.bounds.adjusted(-60, -20, 60, 20).contains(pos))
            return true;
    return false;
}

void InkCanvas::startStroke(Pointer &p, const QPointF &pos, qreal pressure)
{
    p.stroke = InkStroke();
    p.stroke.pts << InkPoint{pos, PenInput::instance().now(), pressure};
}

void InkCanvas::pointerDown(int id, Source src, const QPointF &pos, bool sideButton, double contactMm, qreal pressure)
{
    PenInput &pen = PenInput::instance();
    const qint64 now = pen.now();
    Pointer p;
    p.src = src;
    p.start = p.last = pos;
    p.t0 = now;
    p.target = m_finder ? m_finder(pos.toPoint()) : nullptr;

    if (src == Source::Finger) {
        HandProtection::Touch t;
        t.pos = pos;
        t.contactMm = contactMm;
        HandProtection::Situation s;
        s.penWriting = pen.wroteWithin(1000);
        s.penInUse = pen.inUse(qint64(m_s.penInUseSeconds) * 1000);
        const QRectF wa = writingArea();
        s.hasWritingArea = !wa.isNull() || m_hasPenPos;
        s.writingArea = wa;
        if (QScreen *sc = screen()) {
            const QRect g = sc->geometry();
            s.surface = QRectF(mapFromGlobal(g.topLeft()), QSizeF(g.size()));
        }
        if (protection().ignores(t, s)) {
            p.action = Action::Ignored;
            ++m_ignored;
            m_ptr.insert(id, p);
            return;
        }
        // a second finger without a pen: both scroll, the first one's dot of ink is dropped
        bool others = false;
        for (auto it = m_ptr.begin(); it != m_ptr.end(); ++it)
            if (it.key() >= 0 && it->action != Action::Ignored && it->src == Source::Finger)
                others = true;
        const bool penUsed = pen.inUse(qint64(m_s.penInUseSeconds) * 1000);
        if (others && !penUsed) {
            for (auto it = m_ptr.begin(); it != m_ptr.end(); ++it)
                if (it.key() >= 0 && it->src == Source::Finger && it->action != Action::Ignored) {
                    it->stroke = InkStroke();
                    it->action = Action::Scroll;
                    it->scrollRest = 0;
                }
            p.action = Action::Scroll;
        } else if (penUsed || !m_s.fingerWrites) {
            p.action = Action::Scroll;
        } else {
            p.action = Action::Writing;
        }
    } else if (src == Source::Pen) {
        p.action = (sideButton && m_s.sideButtonSelects && p.target) ? Action::Select : Action::Writing;
    } else {
        p.action = p.target ? Action::Select : Action::Writing;
    }

    if (p.action == Action::Select && p.target) {
        p.anchor = positionAt(p.target, pos);
        QTextCursor c = p.target->textCursor();
        c.setPosition(p.anchor);
        p.target->setTextCursor(c);
        p.target->setFocus(Qt::OtherFocusReason);
    } else if (p.action == Action::Select) {
        p.action = Action::Writing;
    }
    if (p.action == Action::Writing)
        startStroke(p, pos, pressure);
    m_ptr.insert(id, p);
    wake();
    update();
}

void InkCanvas::pointerMove(int id, const QPointF &pos, qreal pressure)
{
    auto it = m_ptr.find(id);
    if (it == m_ptr.end())
        return;
    Pointer &p = *it;
    if (p.action == Action::Ignored)
        return;
    if (QLineF(p.start, pos).length() > kTapSlop)
        p.moved = true;
    switch (p.action) {
    case Action::Writing:
        p.stroke.pts << InkPoint{pos, PenInput::instance().now(), pressure};
        update();
        break;
    case Action::Select:
        if (p.moved)
            selectTo(p, pos);
        break;
    case Action::Scroll: {
        int fingers = 0;
        for (const Pointer &q : m_ptr)
            if (q.src == Source::Finger && q.action == Action::Scroll)
                ++fingers;
        const double dy = (pos.y() - p.last.y()) / qMax(1, fingers);
        if (p.moved && p.target)
            scrollBy(p.target, dy);
        break;
    }
    default:
        break;
    }
    p.last = pos;
}

void InkCanvas::pointerCancel(int id)
{
    m_ptr.remove(id);
    update();
}

void InkCanvas::pointerUp(int id, const QPointF &pos)
{
    auto it = m_ptr.find(id);
    if (it == m_ptr.end())
        return;
    Pointer p = *it;
    m_ptr.erase(it);
    const qint64 now = PenInput::instance().now();
    const bool tap = !p.moved && now - p.t0 < kTapMs;
    switch (p.action) {
    case Action::Writing:
        p.stroke.pts << InkPoint{pos, now, 1.0};
        finishStroke(id, p, pos);
        break;
    case Action::Select:
        if (p.target) {
            if (!p.moved)
                ++m_placed;
            emit cursorPlaced(p.target);
        }
        break;
    case Action::Scroll:
        if (tap && p.target)
            placeCursor(p.target, pos);
        break;
    default:
        break;
    }
    update();
}

void InkCanvas::finishStroke(int id, Pointer &p, const QPointF &pos)
{
    Q_UNUSED(id)
    const qint64 now = PenInput::instance().now();
    const bool shortTap = !p.moved && now - p.t0 < kTapMs;
    // a short tap on the text places the cursor (the dot of an "i" beside unread ink stays ink)
    if (shortTap && p.target && !nearUnreadInk(pos)) {
        placeCursor(p.target, pos);
        return;
    }
    QRectF area;
    if (isScratchOut(p.stroke.points(), lineReference(), &area)) {
        bool removed = false;
        removeStrokesIn(area, &removed);
        if (!removed && p.target)
            deleteWordsUnder(p.target, p.stroke.points());
        emit inkChanged();
        return;
    }
    addStroke(p.stroke);
}

void InkCanvas::removeStrokesIn(const QRectF &area, bool *removedAny)
{
    const QRectF zone = area.adjusted(-8, -lineReference() * 0.5, 8, lineReference() * 0.5);
    for (int i = int(m_lines.size()) - 1; i >= 0; --i) {
        Line &l = m_lines[i];
        if (l.state != Line::Collecting)
            continue;
        for (int k = int(l.strokes.size()) - 1; k >= 0; --k)
            if (zone.contains(l.strokes[k].bounds().center())) {
                l.strokes.removeAt(k);
                *removedAny = true;
            }
        if (l.strokes.isEmpty())
            m_lines.removeAt(i);
        else {
            QRectF b = l.strokes[0].bounds();
            for (const InkStroke &s : l.strokes)
                b = b.united(s.bounds());
            l.bounds = b;
        }
    }
}

bool InkCanvas::deleteWordsUnder(QTextEdit *ed, const QVector<QPointF> &pts)
{
    if (!ed || ed->isReadOnly() || pts.isEmpty())
        return false;
    QPointF left = pts[0], right = pts[0];
    double sumY = 0;
    for (const QPointF &q : pts) {
        if (q.x() < left.x())
            left = q;
        if (q.x() > right.x())
            right = q;
        sumY += q.y();
    }
    const double y = sumY / pts.size();
    QTextCursor a = ed->cursorForPosition(viewportPoint(ed, QPointF(left.x(), y)).toPoint());
    QTextCursor b = ed->cursorForPosition(viewportPoint(ed, QPointF(right.x(), y)).toPoint());
    a.movePosition(QTextCursor::StartOfWord);
    b.movePosition(QTextCursor::EndOfWord);
    int from = a.position(), to = b.position();
    if (to <= from)
        return false;
    QTextDocument *doc = ed->document();
    if (to < doc->characterCount() - 1 && doc->characterAt(to) == QLatin1Char(' '))
        ++to;   // the space after the last word goes with it
    QTextCursor sel(doc);
    sel.setPosition(from);
    sel.setPosition(to, QTextCursor::KeepAnchor);
    const QString removed = sel.selectedText();
    sel.beginEditBlock();
    sel.removeSelectedText();
    sel.endEditBlock();
    emit textScratched(ed, removed);
    return true;
}

// ---------------------------------------------------------------- lines of ink

int InkCanvas::rowOf(double y) const
{
    const GridSettings &g = m_s.grid;
    if (g.style == GridSettings::None || g.style == GridSettings::Dots)
        return -1;
    const QRectF r = gridRect();
    if (r.isNull() || y < r.top() || y > r.bottom())
        return -1;
    return int((y - r.top()) / g.pitch());
}

void InkCanvas::addStroke(const InkStroke &s)
{
    if (s.pts.isEmpty())
        return;
    const QRectF b = s.bounds();
    const int row = rowOf(b.center().y());
    const qint64 now = PenInput::instance().now();
    Line *target = nullptr;
    for (Line &l : m_lines) {
        if (l.state != Line::Collecting)
            continue;
        if (row >= 0) {
            if (l.row == row) { target = &l; break; }
        } else if (l.row < 0) {
            const double tol = qMax(12.0, lineReference() * 0.4);
            if (b.center().y() >= l.bounds.top() - tol && b.center().y() <= l.bounds.bottom() + tol) { target = &l; break; }
        }
    }
    if (!target) {
        Line l;
        l.id = m_nextLine++;
        l.row = row;
        l.bounds = b;
        m_lines << l;
        target = &m_lines.last();
    }
    target->strokes << s;
    target->bounds = target->bounds.united(b);
    target->lastEnd = now;
    wake();
    update();
    emit inkChanged();
}

QImage InkCanvas::pictureOf(const Line &line, Request *request) const
{
    const int margin = 8;
    const QRectF b = line.bounds;
    QImage img(qMax(8, int(qCeil(b.width())) + 2 * margin), qMax(8, int(qCeil(b.height())) + 2 * margin), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(Qt::black, qMax(1.5, m_s.inkWidth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    const QPointF shift(margin - b.left(), margin - b.top());
    for (const InkStroke &s : line.strokes) {
        QVector<QPointF> pts;
        for (const InkPoint &q : s.pts)
            pts << q.p + shift;
        if (request)
            request->strokes << pts;
        if (pts.size() == 1)
            p.drawPoint(pts[0]);
        else
            p.drawPolyline(pts.constData(), int(pts.size()));
    }
    return img;
}

void InkCanvas::readLine(Line &line)
{
    line.state = Line::Reading;
    Request req;
    line.picture = pictureOf(line, &req);
    req.picture = line.picture;
    req.languages << m_s.mainLanguage;
    if (!m_s.fallbackLanguage.isEmpty() && m_s.fallbackLanguage != m_s.mainLanguage)
        req.languages << m_s.fallbackLanguage;
    Recognizer *r = Recognizer::current();
    if (!r || !r->isInstalled()) {
        line.state = Line::Shown;
        line.readAt = PenInput::instance().now();
        line.revealed = true;   // nothing to show
        emit readFailed(tr("Handwriting is not installed."));
        return;
    }
    const int id = line.id;
    QPointer<InkCanvas> self(this);
    r->recognize(req, [self, id](const Result &res) {
        if (self)
            self->onResult(id, res);
    });
}

void InkCanvas::onResult(int lineId, const Result &r)
{
    for (Line &l : m_lines) {
        if (l.id != lineId)
            continue;
        l.state = Line::Shown;
        l.readAt = PenInput::instance().now();
        l.result = r;
        if (!r.ok) {
            l.revealed = true;
            emit readFailed(r.error.isEmpty() ? tr("The line could not be read.") : r.error);
        } else if (r.words.isEmpty()) {
            l.revealed = true;
        } else {
            emit lineRead(l.picture, r);
        }
        update();
        return;
    }
}

void InkCanvas::advance()
{
    PenInput &pen = PenInput::instance();
    const qint64 now = pen.now();

    // a finger held still on the text selects (without a pen)
    for (auto it = m_ptr.begin(); it != m_ptr.end(); ++it) {
        Pointer &p = *it;
        if (p.src == Source::Finger && p.action == Action::Writing && !p.moved && p.target && now - p.t0 >= kLongPressMs) {
            p.stroke = InkStroke();
            p.action = Action::Select;
            p.anchor = positionAt(p.target, p.start);
            QTextCursor c = p.target->textCursor();
            c.setPosition(p.anchor);
            p.target->setTextCursor(c);
        }
    }

    // the pause ends: the lines are read, top to bottom
    if (!anyWriting()) {
        QList<int> ready;
        for (const Line &l : m_lines)
            if (l.state == Line::Collecting && now - l.lastEnd >= m_s.pauseMs)
                ready << l.id;
        std::sort(ready.begin(), ready.end(), [this](int a, int b) {
            double ya = 0, yb = 0;
            for (const Line &l : m_lines) {
                if (l.id == a) ya = l.bounds.top();
                if (l.id == b) yb = l.bounds.top();
            }
            return ya < yb;
        });
        for (int id : ready)
            for (int i = 0; i < m_lines.size(); ++i)
                if (m_lines[i].id == id) {
                    readLine(m_lines[i]);
                    break;
                }
    }

    // the ink stays, then fades; the text goes on as it fades
    for (int i = int(m_lines.size()) - 1; i >= 0; --i) {
        Line &l = m_lines[i];
        if (l.state != Line::Shown)
            continue;
        const qint64 el = now - l.readAt;
        if (!l.revealed && el >= m_s.stayMs) {
            l.revealed = true;
            emit textReady(l.picture, l.result, l.readAt + m_s.stayMs);
        }
        if (el >= m_s.stayMs + m_s.fadeMs) {
            if (!l.revealed)
                emit textReady(l.picture, l.result, l.readAt + m_s.stayMs);
            m_lines.removeAt(i);
        }
    }
    update();
    if (m_lines.isEmpty() && m_ptr.isEmpty())
        m_timer.stop();
}

// ---------------------------------------------------------------- painting

void InkCanvas::paintGrid(QPainter &p, const QRectF &area, const GridSettings &g, const QColor &themeText)
{
    if (g.style == GridSettings::None || area.isEmpty())
        return;
    QColor c = g.color.isValid() ? g.color : themeText;
    c.setAlphaF(qBound(0.05, g.opacity / 100.0, 1.0));
    const double pitch = g.pitch();
    p.save();
    p.setClipRect(area);
    QPen pen(c, g.lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    switch (g.style) {
    case GridSettings::Ruled:
        for (double y = area.top() + pitch; y <= area.bottom() + 0.5; y += pitch)
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
        break;
    case GridSettings::Writing: {
        for (double top = area.top(); top < area.bottom(); top += pitch) {
            const double base = top + pitch * 0.78, mid = top + pitch * 0.40;
            p.setPen(pen);
            p.drawLine(QPointF(area.left(), base), QPointF(area.right(), base));
            QPen dashed(c, qMax(0.5, g.lineWidth * 0.8), Qt::DashLine);
            p.setPen(dashed);
            p.drawLine(QPointF(area.left(), mid), QPointF(area.right(), mid));
        }
        break;
    }
    case GridSettings::Squares:
        for (double y = area.top(); y <= area.bottom() + 0.5; y += pitch)
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
        for (double x = area.left(); x <= area.right() + 0.5; x += pitch)
            p.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
        break;
    case GridSettings::Dots: {
        p.setPen(Qt::NoPen);
        p.setBrush(c);
        const double step = pitch * 0.6;
        for (double y = area.top() + step; y <= area.bottom(); y += step)
            for (double x = area.left() + step; x <= area.right(); x += step)
                p.drawEllipse(QPointF(x, y), g.lineWidth * 0.9 + 0.4, g.lineWidth * 0.9 + 0.4);
        break;
    }
    case GridSettings::None:
        break;
    }
    p.restore();
}

void InkCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (m_role == Role::Pad) {
        p.fillRect(rect(), palette().color(QPalette::Base));
    } else {
        // the see-through layer: a faint tint and a border show that the surface is on; the text stays visible
        QColor tint = palette().color(QPalette::Highlight);
        tint.setAlpha(14);
        p.fillRect(rect(), tint);
        tint.setAlpha(120);
        p.setPen(QPen(tint, 2));
        p.drawRect(rect().adjusted(1, 1, -1, -1));
    }
    const QRectF g = gridRect();
    if (!g.isNull())
        paintGrid(p, g, m_s.grid, themeText());

    auto drawStroke = [&](const InkStroke &s, double alpha) {
        if (s.pts.isEmpty())
            return;
        QColor c = inkColor();
        c.setAlphaF(c.alphaF() * alpha);
        QPen pen(c, m_s.inkWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        if (s.pts.size() == 1) {
            p.drawPoint(s.pts[0].p);
            return;
        }
        QPainterPath path(s.pts[0].p);
        for (int i = 1; i < s.pts.size(); ++i)
            path.lineTo(s.pts[i].p);
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    };
    for (int i = 0; i < m_lines.size(); ++i) {
        const double a = lineAlpha(i);
        for (const InkStroke &s : m_lines[i].strokes)
            drawStroke(s, a);
    }
    for (const Pointer &q : m_ptr)
        if (q.action == Action::Writing)
            drawStroke(q.stroke, 1.0);
}

} // namespace handwriting
} // namespace qb
