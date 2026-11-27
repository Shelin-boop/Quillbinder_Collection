#pragma once
// 0.99.0 (handwriting, item 112): the protected strip at the bottom of the inspector. The text read from the ink appears
// here as the ink fades, as chips of words. A tap on a word shows its other readings (a menu); writing over a word corrects
// it (the ink written on the strip is read as one line and replaces the word under it); scratching words out deletes them.
// Insert, Pin and the buttons Space, Backspace and Enter work on this text (HandwritingPanel).
#include "ui/handwriting/Ink.h"
#include "ui/handwriting/Recognizer.h"

#include <QImage>
#include <QList>
#include <QTimer>
#include <QWidget>

class QMenu;

namespace qb {
namespace handwriting {

class WordStrip : public QWidget
{
    Q_OBJECT
public:
    struct Token {
        enum Kind { Word, Space, Newline } kind = Word;
        QString text;
        QStringList alternatives;
        int lineId = 0;            // the line it was read from (0: typed by a button)
        qint64 appearedAt = 0;
    };

    explicit WordStrip(QWidget *parent = nullptr);

    void addLine(const QImage &picture, const Result &result, qint64 startedMs = -1);   // the text of a read line, appearing from startedMs (default: now)
    const QList<Token> &tokens() const { return m_tokens; }
    QString text() const;            // the words with a space between; Space and Enter as typed
    bool isEmpty() const { return m_tokens.isEmpty(); }
    void clear();
    void setText(const QString &text);   // text without readings (a cancelled pin comes back this way)
    void appendSpace();
    void appendNewline();
    void backspace();                // the last character of the last word (a word of one letter goes), else the last token
    double tokenAlpha(int index) const;   // the text appears while the ink fades: 0 .. 1
    QRect tokenRect(int index) const;     // where its chip is (for the checks)
    int tokenAt(const QPoint &p) const;
    // the alternatives of a word (a tap): opens the menu; the chosen reading replaces the word
    QMenu *showAlternatives(int index, const QPoint &globalPos = QPoint());
    void replaceWord(int index, const QString &text);   // a correction (also by writing over it)
    int fadeMs() const { return m_fadeMs; }
    void setFadeMs(int ms) { m_fadeMs = ms; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void changed();
    void corrected(int lineId);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    struct Layout { QList<QRect> rects; int height = 0; };
    Layout layoutFor(int width) const;
    void relayout();
    void finishInk();
    QString lineText(int lineId) const;
    void keepCorrection(int lineId);

    QList<Token> m_tokens;
    QHash<int, QImage> m_pictures;
    int m_nextLine = 1;
    int m_fadeMs = 2000;
    QList<QRect> m_rects;
    // writing over words
    QVector<QPointF> m_stroke;
    QList<QVector<QPointF>> m_inkStrokes;
    qint64 m_pressAt = 0;
    qint64 m_lastEnd = 0;
    bool m_moved = false;
    QPointF m_pressPos;
    QTimer m_timer;
};

} // namespace handwriting
} // namespace qb
