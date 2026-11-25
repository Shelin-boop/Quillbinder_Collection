#include "WritingProgress.h"

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
    const bool s = sessionTarget > 0, d = draftTarget > 0;
    m_sessionBox->setVisible(s);
    m_draftBox->setVisible(d);
    if (s) {
        m_sessionBar->setRange(0, sessionTarget);
        m_sessionBar->setValue(qBound(0, session, sessionTarget));
        m_sessionLabel->setText(tr("%1 / %2 today").arg(loc.toString(session), loc.toString(sessionTarget)));
        m_sessionBox->setToolTip(tr("Today's writing: %1 of %2 %3 (click for Project Targets)")
                                     .arg(loc.toString(session), loc.toString(sessionTarget),
                                          sessionChars ? tr("characters") : tr("words")));
    }
    if (d) {
        m_draftBar->setRange(0, draftTarget);
        m_draftBar->setValue(qBound(0, draft, draftTarget));
        m_draftLabel->setText(tr("%1 / %2 draft").arg(loc.toString(draft), loc.toString(draftTarget)));
        m_draftBox->setToolTip(tr("The draft: %1 of %2 %3 (click for Project Targets)")
                                   .arg(loc.toString(draft), loc.toString(draftTarget), draftChars ? tr("characters") : tr("words")));
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
