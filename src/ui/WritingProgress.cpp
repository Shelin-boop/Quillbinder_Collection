#include "WritingProgress.h"
#include "core/TargetMath.h"
#include "ui/TargetSpinBox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QProgressBar>

namespace qb {

namespace {
QWidget *segment(QWidget *parent, const QString &name, QProgressBar **bar, QLabel **label)
{
    auto *box = new QWidget(parent);
    box->setObjectName(name);
    auto *l = new QHBoxLayout(box);
    l->setContentsMargins(4, 0, 4, 0);
    l->setSpacing(4);
    *bar = new QProgressBar(box);
    (*bar)->setTextVisible(false);
    (*bar)->setFixedSize(44, 7);
    l->addWidget(*bar);
    *label = new QLabel(box);
    l->addWidget(*label);
    return box;
}
} // namespace

WritingProgress::WritingProgress(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("writingProgress"));
    setCursor(Qt::PointingHandCursor);
    auto *l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(2);
    m_sessionBox = segment(this, QStringLiteral("progressSession"), &m_sessionBar, &m_sessionLabel);
    m_draftBox = segment(this, QStringLiteral("progressDraft"), &m_draftBar, &m_draftLabel);
    m_sessionLabel->setObjectName(QStringLiteral("progressSessionText"));
    m_draftLabel->setObjectName(QStringLiteral("progressDraftText"));
    l->addWidget(m_sessionBox);
    l->addWidget(m_draftBox);
    hide();
}

void WritingProgress::setProgress(int session, int sessionTarget, bool sessionChars, int draft, int draftTarget, bool draftChars)
{
    const QLocale loc;
    // 0.95.0: a negative target is a cutting target (its bar fills as the count falls; negative numbers are red)
    const bool s = sessionTarget != 0, d = draftTarget != 0;
    const QColor back = palette().color(QPalette::Window);
    m_sessionBox->setVisible(s);
    m_draftBox->setVisible(d);
    m_sessionLabel->setTextFormat(Qt::RichText);
    m_draftLabel->setTextFormat(Qt::RichText);
    if (s) {
        if (sessionTarget < 0) {
            m_sessionBar->setRange(0, -sessionTarget);
            m_sessionBar->setValue(qBound(0, -session, -sessionTarget));
        } else {
            m_sessionBar->setRange(0, sessionTarget);
            m_sessionBar->setValue(qBound(0, session, sessionTarget));
        }
        m_sessionLabel->setText(tr("%1 / %2 today").arg(targettext::numberHtml(session, back), targettext::numberHtml(sessionTarget, back)));
        m_sessionBox->setToolTip((sessionChars ? tr("Today's writing: %1 of %2 characters (click for Project Targets)")
                                               : tr("Today's writing: %1 of %2 words (click for Project Targets)"))
                                     .arg(loc.toString(session), loc.toString(sessionTarget)));
    }
    if (d) {
        if (draftTarget < 0) {
            m_draftBar->setRange(0, 1000);
            m_draftBar->setValue(int(targetmath::fill(draft, draftTarget) * 1000));
        } else {
            m_draftBar->setRange(0, draftTarget);
            m_draftBar->setValue(qBound(0, draft, draftTarget));
        }
        m_draftLabel->setText(tr("%1 / %2 draft").arg(loc.toString(draft), targettext::numberHtml(draftTarget, back)));
        m_draftBox->setToolTip((draftChars ? tr("The draft: %1 of %2 characters (click for Project Targets)")
                                           : tr("The draft: %1 of %2 words (click for Project Targets)"))
                                   .arg(loc.toString(draft), loc.toString(draftTarget)));
    }
    m_any = s || d;
    setVisible(m_any);
}

QString WritingProgress::sessionText() const { return m_sessionBox->isVisibleTo(this) ? m_sessionLabel->text() : QString(); }
QString WritingProgress::draftText() const { return m_draftBox->isVisibleTo(this) ? m_draftLabel->text() : QString(); }

void WritingProgress::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        emit clicked();
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

} // namespace qb
