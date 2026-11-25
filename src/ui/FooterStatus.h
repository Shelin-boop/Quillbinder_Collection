// 0.93.0: the writer's "Saved / Not saved yet" text, the warning link "No backups" and the short messages of the
// program, shown in the empty stretch of the first editor's footer bar (the window has no status bar row any more).
// One of these sits in each editor; only the first editor's is filled. It chooses by its width what to show: the
// whole text, a message shortened (its full text is the tooltip), or a small colored mark with the text as tooltip.
#pragma once

#include <QString>
#include <functional>
#include <QWidget>

class QLabel;
class QTimer;

namespace qb {

class FooterStatus : public QWidget
{
public:
    enum Kind { Saved = 0, Pending = 1, Failed = 2 };
    explicit FooterStatus(QWidget *parent = nullptr);

    void setSave(const QString &text, const QString &tip, Kind kind);
    void clearSave();
    QString saveText() const { return m_saveText; }

    // a short message for its seconds (0 = until clearNote), then the save text returns
    void showNote(const QString &text, int ms);
    // the same with a link after the text (0.94.0: "Show Folder" after a backup); `action` runs when it is clicked
    void showNote(const QString &text, int ms, const QString &actionText, std::function<void()> action);
    QLabel *noteActionLabel() const { return m_noteAction; }
    void clearNote();
    QString noteText() const { return m_note; }

    // the warning link (rich text with a link), beside the save text
    void setBackup(const QString &html, const QString &tip, bool visible);
    QLabel *backupLabel() const { return m_link; }
    QLabel *textLabel() const { return m_text; }

    // the update check's note: fades in and out twice, in this spot
    void showFading(const QString &text);

    bool isMark() const { return m_mark; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override; // the mark of a note with a link runs the link
    void changeEvent(QEvent *e) override;

private:
    void render();
    QLabel *m_text, *m_link, *m_noteAction;
    QString m_noteActionText;
    std::function<void()> m_noteFn;
    QTimer *m_noteTimer;
    QString m_saveText, m_saveTip, m_note, m_linkHtml, m_linkTip;
    Kind m_kind = Saved;
    bool m_linkVisible = false;
    bool m_mark = false;
    int m_fading = 0;
};

} // namespace qb
