#include "FooterStatus.h"

#include <QApplication>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPalette>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSequentialAnimationGroup>
#include <QTextDocument>
#include <QTimer>

namespace qb {

namespace {
constexpr int kGap = 4;
const QString kMark = QStringLiteral("●"); // a filled circle

QString plainOf(const QString &html)
{
    QTextDocument d;
    d.setHtml(html);
    return d.toPlainText();
}
} // namespace

FooterStatus::FooterStatus(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("footerStatus"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_text = new QLabel(this);
    m_text->setObjectName(QStringLiteral("saveState"));
    m_text->setContentsMargins(6, 0, 6, 0);
    m_text->hide();
    m_link = new QLabel(this);
    m_link->setObjectName(QStringLiteral("backupState"));
    m_link->setContentsMargins(6, 0, 6, 0);
    m_link->setTextFormat(Qt::RichText);
    m_link->hide();
    m_noteTimer = new QTimer(this);
    m_noteTimer->setSingleShot(true);
    connect(m_noteTimer, &QTimer::timeout, this, &FooterStatus::clearNote);
}

void FooterStatus::setSave(const QString &text, const QString &tip, Kind kind)
{
    if (m_saveText == text && m_saveTip == tip && m_kind == kind)
        return;
    m_saveText = text;
    m_saveTip = tip;
    m_kind = kind;
    render();
}

void FooterStatus::clearSave()
{
    setSave(QString(), QString(), Saved);
}

void FooterStatus::showNote(const QString &text, int ms)
{
    m_note = text;
    if (text.isEmpty() || ms <= 0)
        m_noteTimer->stop();
    else
        m_noteTimer->start(ms);
    render();
}

void FooterStatus::clearNote()
{
    m_noteTimer->stop();
    if (!m_note.isEmpty()) {
        m_note.clear();
        render();
    }
}

void FooterStatus::setBackup(const QString &html, const QString &tip, bool visible)
{
    if (m_linkHtml == html && m_linkTip == tip && m_linkVisible == visible)
        return;
    m_linkHtml = html;
    m_linkTip = tip;
    m_linkVisible = visible;
    render();
}

void FooterStatus::showFading(const QString &text)
{
    auto *note = new QLabel(text, this);
    note->setObjectName(QStringLiteral("fadingNote"));
    note->setContentsMargins(6, 0, 6, 0);
    note->setToolTip(text);
    auto *fx = new QGraphicsOpacityEffect(note);
    fx->setOpacity(0.0);
    note->setGraphicsEffect(fx);
    ++m_fading;
    render();
    note->show();
    auto *run = new QSequentialAnimationGroup(note);
    for (int round = 0; round < 2; ++round) {
        auto *in = new QPropertyAnimation(fx, "opacity", run);
        in->setDuration(700);
        in->setStartValue(0.0);
        in->setEndValue(1.0);
        run->addAnimation(in);
        run->addPause(2200);
        auto *out = new QPropertyAnimation(fx, "opacity", run);
        out->setDuration(700);
        out->setStartValue(1.0);
        out->setEndValue(0.0);
        run->addAnimation(out);
        if (round == 0)
            run->addPause(500);
    }
    note->setProperty("fades", 2);
    connect(run, &QAbstractAnimation::finished, note, [this, note] {
        --m_fading;
        note->deleteLater();
        render();
    });
    run->start(QAbstractAnimation::DeleteWhenStopped);
}

QSize FooterStatus::sizeHint() const
{
    const QString shown = m_note.isEmpty() ? m_saveText : m_note;
    int w = shown.isEmpty() ? 0 : m_text->fontMetrics().horizontalAdvance(shown) + 12 + 12;
    if (m_linkVisible)
        w += m_link->fontMetrics().horizontalAdvance(plainOf(m_linkHtml)) + 12 + kGap;
    return QSize(qMin(w, 420), qMax(m_text->fontMetrics().height() + 2, 18));
}

QSize FooterStatus::minimumSizeHint() const
{
    return QSize(0, qMax(m_text->fontMetrics().height() + 2, 18));
}

void FooterStatus::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    render();
}

void FooterStatus::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::PaletteChange || e->type() == QEvent::FontChange)
        render();
}

void FooterStatus::render()
{
    const int h = height();
    const bool noteOn = !m_note.isEmpty();
    const QString shown = noteOn ? m_note : m_saveText;
    const bool fading = m_fading > 0;
    int avail = width();
    int x = 0;
    m_mark = false;

    // the link first: its width is kept; narrow: a mark "!" with the same click and tooltip
    const QFontMetrics fm(m_text->font());
    int linkW = 0;
    bool linkMark = false;
    if (m_linkVisible) {
        const QString plain = plainOf(m_linkHtml);
        const int full = m_link->fontMetrics().horizontalAdvance(plain) + 12;
        linkMark = avail < full + (shown.isEmpty() ? 0 : 24) + kGap;
        linkW = linkMark ? m_link->fontMetrics().horizontalAdvance(QStringLiteral("!")) + 12 : full;
    }
    const int textAvail = avail - (linkW ? linkW + kGap : 0);

    if (shown.isEmpty() || fading) {
        m_text->hide();
    } else {
        const int full = fm.horizontalAdvance(shown) + 12;
        const int markW = fm.horizontalAdvance(kMark) + 12;
        QString tip;
        QString style;
        QString text = shown;
        if (noteOn) {
            if (textAvail >= qMin(full, 120)) {
                if (full > textAvail) {
                    text = fm.elidedText(shown, Qt::ElideRight, qMax(0, textAvail - 12));
                    tip = shown;
                } else {
                    tip = QString();
                }
            } else {
                m_mark = true;
                text = kMark;
                tip = shown;
                style = QStringLiteral("QLabel { color: %1; }").arg(QApplication::palette(this).color(QPalette::Highlight).name());
            }
        } else {
            tip = m_saveTip;
            if (m_kind == Failed)
                style = QStringLiteral("QLabel { color: white; background: #b00020; padding: 1px 6px; border-radius: 3px; }");
            if (textAvail < full) {
                m_mark = true;
                text = kMark;
                tip = m_saveText + QLatin1Char('\n') + m_saveTip;
                const char *c = m_kind == Saved ? "#2e9e4f" : m_kind == Pending ? "#e0a100" : "#d0213b";
                style = QStringLiteral("QLabel { color: %1; }").arg(QLatin1String(c));
            }
        }
        m_text->setStyleSheet(style);
        m_text->setText(text);
        m_text->setToolTip(tip);
        m_text->adjustSize();
        int w = qMin(m_text->sizeHint().width(), qMax(0, textAvail));
        if (m_mark)
            w = qMin(markW, qMax(0, textAvail));
        m_text->setGeometry(x, (h - m_text->sizeHint().height()) / 2, w, m_text->sizeHint().height());
        m_text->show();
        x += w + kGap;
    }
    // the fading note sits where the text is
    if (m_fading > 0) {
        const auto notes = findChildren<QLabel *>(QStringLiteral("fadingNote"), Qt::FindDirectChildrenOnly);
        for (QLabel *n : notes) {
            n->adjustSize();
            const int w = qMin(n->sizeHint().width(), qMax(0, textAvail));
            n->setGeometry(0, (h - n->sizeHint().height()) / 2, w, n->sizeHint().height());
        }
        x = 0;
    }
    if (m_linkVisible && !m_linkHtml.isEmpty()) {
        QString html = m_linkHtml;
        if (linkMark)
            html.replace(QRegularExpression(QStringLiteral(">[^<]*</a>")), QStringLiteral(">!</a>"));
        m_link->setText(html);
        m_link->setToolTip(m_linkTip);
        m_link->adjustSize();
        const int lw = qMin(m_link->sizeHint().width(), qMax(0, avail - (m_text->isVisible() ? x : 0)));
        m_link->setGeometry(m_text->isVisible() ? x : 0, (h - m_link->sizeHint().height()) / 2, lw, m_link->sizeHint().height());
        m_link->show();
    } else {
        m_link->hide();
    }
    updateGeometry();
}

} // namespace qb
