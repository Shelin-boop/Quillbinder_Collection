#include "ui/handwriting/WordStrip.h"

#include "ui/PenInput.h"
#include "ui/handwriting/HandwritingSettings.h"
#include "ui/handwriting/LearningStore.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>

namespace qb {
namespace handwriting {

namespace {
constexpr int kPadX = 8, kPadY = 4, kGap = 6, kRowGap = 5, kMargin = 6;
}

WordStrip::WordStrip(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("handwritingStrip"));
    setMinimumHeight(48);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_timer.setInterval(30);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        const qint64 now = PenInput::instance().now();
        if (!m_inkStrokes.isEmpty() && m_stroke.isEmpty() && now - m_lastEnd >= HandwritingConfig::instance().get().pauseMs)
            finishInk();
        update();
        if (m_inkStrokes.isEmpty() && m_stroke.isEmpty() && tokenAlpha(int(m_tokens.size()) - 1) >= 1.0)
            m_timer.stop();
    });
}

void WordStrip::addLine(const QImage &picture, const Result &result, qint64 startedMs)
{
    const int line = m_nextLine++;
    m_pictures.insert(line, picture);
    for (const Word &w : result.words) {
        Token t;
        t.kind = Token::Word;
        t.text = w.text;
        t.alternatives = w.alternatives;
        t.lineId = line;
        t.appearedAt = startedMs >= 0 ? startedMs : PenInput::instance().now();
        m_tokens << t;
    }
    relayout();
    m_timer.start();
    emit changed();
}

QString WordStrip::text() const
{
    QString out;
    bool needSpace = false;
    for (const Token &t : m_tokens) {
        switch (t.kind) {
        case Token::Word:
            if (needSpace)
                out += QLatin1Char(' ');
            out += t.text;
            needSpace = true;
            break;
        case Token::Space:
            out += QLatin1Char(' ');
            needSpace = false;
            break;
        case Token::Newline:
            out += QLatin1Char('\n');
            needSpace = false;
            break;
        }
    }
    return out;
}

void WordStrip::clear()
{
    m_tokens.clear();
    m_pictures.clear();
    m_inkStrokes.clear();
    m_stroke.clear();
    relayout();
    emit changed();
}

void WordStrip::setText(const QString &text)
{
    clear();
    const qint64 then = PenInput::instance().now() - m_fadeMs;
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); ++i) {
        for (const QString &w : lines[i].split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
            Token t;
            t.text = w;
            t.appearedAt = then;
            m_tokens << t;
        }
        if (i + 1 < lines.size()) {
            Token t;
            t.kind = Token::Newline;
            t.text = QStringLiteral("\n");
            t.appearedAt = then;
            m_tokens << t;
        }
    }
    relayout();
    emit changed();
}

void WordStrip::appendSpace()
{
    Token t;
    t.kind = Token::Space;
    t.text = QStringLiteral(" ");
    t.appearedAt = PenInput::instance().now() - m_fadeMs;
    m_tokens << t;
    relayout();
    emit changed();
}

void WordStrip::appendNewline()
{
    Token t;
    t.kind = Token::Newline;
    t.text = QStringLiteral("\n");
    t.appearedAt = PenInput::instance().now() - m_fadeMs;
    m_tokens << t;
    relayout();
    emit changed();
}

void WordStrip::backspace()
{
    if (m_tokens.isEmpty())
        return;
    Token &t = m_tokens.last();
    if (t.kind == Token::Word && t.text.size() > 1) {
        t.text.chop(1);
        t.alternatives.clear();
    } else {
        m_tokens.removeLast();
    }
    relayout();
    emit changed();
}

double WordStrip::tokenAlpha(int index) const
{
    if (index < 0 || index >= m_tokens.size())
        return 1.0;
    if (m_fadeMs <= 0)
        return 1.0;
    return qBound(0.0, double(PenInput::instance().now() - m_tokens[index].appearedAt) / m_fadeMs, 1.0);
}

WordStrip::Layout WordStrip::layoutFor(int width) const
{
    Layout l;
    const QFontMetrics fm(font());
    const int h = fm.height() + 2 * kPadY;
    int x = kMargin, y = kMargin;
    for (const Token &t : m_tokens) {
        if (t.kind == Token::Newline) {
            l.rects << QRect(x, y, 0, h);
            x = kMargin;
            y += h + kRowGap;
            continue;
        }
        const int w = t.kind == Token::Space ? fm.horizontalAdvance(QLatin1Char(' ')) + 2 * kPadX
                                             : fm.horizontalAdvance(t.text) + 2 * kPadX;
        if (x + w > width - kMargin && x > kMargin) {
            x = kMargin;
            y += h + kRowGap;
        }
        l.rects << QRect(x, y, w, h);
        x += w + kGap;
    }
    l.height = y + h + kMargin;
    return l;
}

void WordStrip::relayout()
{
    const Layout l = layoutFor(qMax(60, width()));
    m_rects = l.rects;
    updateGeometry();
    update();
}

void WordStrip::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    relayout();
}

QSize WordStrip::sizeHint() const
{
    const Layout l = layoutFor(qMax(120, width()));
    return QSize(200, qBound(48, l.height, 130));
}

QSize WordStrip::minimumSizeHint() const { return QSize(80, 48); }

QRect WordStrip::tokenRect(int index) const { return index >= 0 && index < m_rects.size() ? m_rects[index] : QRect(); }

int WordStrip::tokenAt(const QPoint &p) const
{
    for (int i = 0; i < m_rects.size(); ++i)
        if (m_tokens[i].kind == Token::Word && m_rects[i].adjusted(-2, -2, 2, 2).contains(p))
            return i;
    return -1;
}

QString WordStrip::lineText(int lineId) const
{
    QStringList w;
    for (const Token &t : m_tokens)
        if (t.lineId == lineId && t.kind == Token::Word)
            w << t.text;
    return w.join(QLatin1Char(' '));
}

void WordStrip::keepCorrection(int lineId)
{
    if (lineId <= 0)
        return;
    LearningStore::add(m_pictures.value(lineId), lineText(lineId));
    emit corrected(lineId);
}

void WordStrip::replaceWord(int index, const QString &text)
{
    if (index < 0 || index >= m_tokens.size() || m_tokens[index].kind != Token::Word || text.isEmpty())
        return;
    Token &t = m_tokens[index];
    if (t.text == text)
        return;
    // the old reading becomes an alternative
    QStringList alts = t.alternatives;
    alts.removeAll(text);
    alts.prepend(t.text);
    t.text = text;
    t.alternatives = alts;
    relayout();
    keepCorrection(t.lineId);
    emit changed();
}

QMenu *WordStrip::showAlternatives(int index, const QPoint &globalPos)
{
    if (index < 0 || index >= m_tokens.size() || m_tokens[index].kind != Token::Word)
        return nullptr;
    delete findChild<QMenu *>(QStringLiteral("stripAlternatives"));
    auto *menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("stripAlternatives"));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    const QStringList alts = m_tokens[index].alternatives;
    if (alts.isEmpty()) {
        menu->addAction(tr("No other readings"))->setEnabled(false);
    } else {
        for (const QString &a : alts) {
            QAction *ac = menu->addAction(a);
            connect(ac, &QAction::triggered, this, [this, index, a] { replaceWord(index, a); });
        }
    }
    menu->popup(globalPos.isNull() ? mapToGlobal(m_rects.value(index).bottomLeft()) : globalPos);
    return menu;
}

// ---------------------------------------------------------------- input

void WordStrip::mousePressEvent(QMouseEvent *e)
{
    m_pressAt = PenInput::instance().now();
    m_pressPos = e->position();
    m_moved = false;
    m_stroke.clear();
    m_stroke << e->position();
    e->accept();
}

void WordStrip::mouseMoveEvent(QMouseEvent *e)
{
    if (m_stroke.isEmpty())
        return;
    if (QLineF(m_pressPos, e->position()).length() > 8)
        m_moved = true;
    m_stroke << e->position();
    update();
}

void WordStrip::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_stroke.isEmpty())
        return;
    const qint64 now = PenInput::instance().now();
    m_stroke << e->position();
    if (!m_moved && now - m_pressAt < 400) {
        m_stroke.clear();
        const int i = tokenAt(e->position().toPoint());
        if (i >= 0)
            showAlternatives(i, e->globalPosition().toPoint());
        return;
    }
    QRectF area;
    if (isScratchOut(m_stroke, 24.0, &area)) {
        // scratching words out: the words under the scratch go
        for (int i = int(m_tokens.size()) - 1; i >= 0; --i)
            if (m_tokens[i].kind == Token::Word && QRectF(m_rects[i]).intersects(area.adjusted(-2, -2, 2, 2))
                && QRectF(m_rects[i]).intersected(area).width() >= m_rects[i].width() * 0.3)
                m_tokens.removeAt(i);
        m_stroke.clear();
        relayout();
        emit changed();
        return;
    }
    m_inkStrokes << m_stroke;
    m_stroke.clear();
    m_lastEnd = now;
    m_timer.start();
}

void WordStrip::finishInk()
{
    // the ink written over the words is read as one line; it replaces the word it lies on most
    QRectF box;
    for (const QVector<QPointF> &s : m_inkStrokes)
        for (const QPointF &p : s)
            box = box.isNull() ? QRectF(p, QSizeF(0, 0)) : box.united(QRectF(p, QSizeF(0, 0)));
    int best = -1;
    double bestArea = 0;
    for (int i = 0; i < m_rects.size(); ++i) {
        if (m_tokens[i].kind != Token::Word)
            continue;
        const QRectF in = QRectF(m_rects[i]).intersected(box.adjusted(-6, -6, 6, 6));
        const double a = in.width() * in.height();
        if (a > bestArea) {
            bestArea = a;
            best = i;
        }
    }
    Recognizer *r = Recognizer::current();
    const QList<QVector<QPointF>> strokes = m_inkStrokes;
    m_inkStrokes.clear();
    if (best < 0 || !r || !r->isInstalled())
        return;
    Request req;
    const int margin = 6;
    req.picture = QImage(qMax(8, int(box.width()) + 2 * margin), qMax(8, int(box.height()) + 2 * margin), QImage::Format_RGB32);
    req.picture.fill(Qt::white);
    QPainter p(&req.picture);
    p.setPen(QPen(Qt::black, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (const QVector<QPointF> &s : strokes) {
        QVector<QPointF> pts;
        for (const QPointF &q : s)
            pts << q - box.topLeft() + QPointF(margin, margin);
        req.strokes << pts;
        p.drawPolyline(pts.constData(), int(pts.size()));
    }
    p.end();
    const HandwritingSettings &hs = HandwritingConfig::instance().get();
    req.languages << hs.mainLanguage;
    if (!hs.fallbackLanguage.isEmpty())
        req.languages << hs.fallbackLanguage;
    QPointer<WordStrip> self(this);
    const int index = best;
    r->recognize(req, [self, index](const Result &res) {
        if (!self || !res.ok || res.words.isEmpty())
            return;
        QStringList parts;
        for (const Word &w : res.words)
            parts << w.text;
        self->replaceWord(index, parts.join(QLatin1Char(' ')));
    });
}

void WordStrip::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::AlternateBase));
    p.setPen(pal.color(QPalette::Mid));
    p.drawLine(0, 0, width(), 0);
    if (m_tokens.isEmpty()) {
        QColor c = pal.color(QPalette::PlaceholderText);
        p.setPen(c);
        p.drawText(rect().adjusted(kMargin, 0, -kMargin, 0), Qt::AlignVCenter | Qt::TextWordWrap,
                   tr("The text you write appears here."));
    }
    for (int i = 0; i < m_tokens.size() && i < m_rects.size(); ++i) {
        const Token &t = m_tokens[i];
        if (t.kind != Token::Word)
            continue;
        const double a = tokenAlpha(i);
        p.setOpacity(a);
        p.setBrush(pal.color(QPalette::Base));
        p.setPen(pal.color(QPalette::Mid));
        p.drawRoundedRect(m_rects[i], 5, 5);
        p.setPen(pal.color(QPalette::Text));
        p.drawText(m_rects[i], Qt::AlignCenter, t.text);
    }
    p.setOpacity(1.0);
    p.setPen(QPen(pal.color(QPalette::Highlight), 2, Qt::SolidLine, Qt::RoundCap));
    for (const QVector<QPointF> &s : m_inkStrokes)
        p.drawPolyline(s.constData(), int(s.size()));
    if (!m_stroke.isEmpty())
        p.drawPolyline(m_stroke.constData(), int(m_stroke.size()));
}

} // namespace handwriting
} // namespace qb
