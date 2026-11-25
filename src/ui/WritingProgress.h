// 0.94.0 (to-do item 74): the writing progress in the footer of an editor, beside the word count: a small bar and a text for
// the session ("83 / 1,000 today") and for the draft ("12,400 / 80,000 draft"), each shown only while its target is set.
// A click on either opens Project Targets.
#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;

namespace qb {

class WritingProgress : public QWidget
{
    Q_OBJECT
public:
    explicit WritingProgress(QWidget *parent = nullptr);
    // target <= 0: that part is hidden. `chars`: the numbers count characters, not words (for the tooltip).
    void setProgress(int session, int sessionTarget, bool sessionChars, int draft, int draftTarget, bool draftChars);
    QString sessionText() const;
    QString draftText() const;
    QProgressBar *sessionBar() const { return m_sessionBar; }
    QProgressBar *draftBar() const { return m_draftBar; }
    bool hasAnything() const { return m_any; }

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *e) override;

private:
    QWidget *m_sessionBox, *m_draftBox;
    QProgressBar *m_sessionBar, *m_draftBar;
    QLabel *m_sessionLabel, *m_draftLabel;
    bool m_any = false;
};

} // namespace qb
