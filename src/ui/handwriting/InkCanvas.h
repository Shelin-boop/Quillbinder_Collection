#pragma once
// 0.99.0 (handwriting, item 112): the one writing area of all three modes. In the inspector's tab it is a pad (own ground,
// with the grid); over the editor it is a see-through layer (the text stays visible under the ink); in the mode "both" it
// is the layer over the editor, the pane line and the pad part of the inspector (the grid only over that part).
//
// What it does with input (the rules of the brief):
//  - With a pen (Qt's tablet events; PenInput tells): the pen writes; a short pen tap places the text cursor; the pen's side
//    button selects; fingers scroll and tap (one finger scrolls, a tap places the cursor).
//  - Without a pen: one finger writes; a tap places the cursor; a long press and drag selects; two fingers scroll. A mouse
//    points (click places the cursor, drag selects, the wheel scrolls) over the text and writes on the pad.
//  - Scratching words out deletes them: ink not read yet, else the text under the scratch.
//  - The hand protection (Ink.h) ignores finger touches in the rest zone while the pen writes and large contacts.
// A line is read right after the pause; the ink stays and fades; as it fades the read text is handed on (textReady).
//
// Time: everything runs on PenInput::instance().now() and advance() (a timer calls it every 30 ms), so the checks can drive
// it with a clock of their own.
#include "ui/handwriting/HandwritingSettings.h"
#include "ui/handwriting/Ink.h"
#include "ui/handwriting/Recognizer.h"

#include <QHash>
#include <QScrollBar>
#include <QImage>
#include <QPointer>
#include <QTextCursor>
#include <QTextEdit>
#include <QTouchEvent>
#include <QTimer>
#include <QWidget>
#include <functional>

namespace qb {
namespace handwriting {

class InkCanvas : public QWidget
{
    Q_OBJECT
public:
    enum class Role { Pad, Overlay };
    enum class Source { Pen, Finger, Mouse };
    enum class Action { Undecided, Writing, Select, Scroll, Ignored };

    explicit InkCanvas(QWidget *parent = nullptr);

    void setRole(Role role);
    Role role() const { return m_role; }
    // The part of the canvas with a grid (canvas coordinates); the pad: the whole canvas, the mode "both": the inspector part,
    // the editor mode: none (an empty rectangle).
    void setGridRect(const QRectF &r);
    QRectF gridRect() const;
    void clearGridRect() { m_gridSet = false; update(); }   // the pad: the whole canvas
    // The text under a point of the canvas that a tap, a selection or a scroll acts on (null: the pad, nothing to point at).
    void setTargetFinder(std::function<QTextEdit *(const QPoint &)> finder) { m_finder = std::move(finder); }
    // A place for a message ("no recognizer")
    void setSettings(const HandwritingSettings &s);
    const HandwritingSettings &settings() const { return m_s; }

    // ---- the state (also for the checks)
    int pendingLines() const;            // lines of ink not read yet
    int visibleLines() const;            // lines on the screen (not read, being read, fading)
    int strokeCount() const;
    double lineAlpha(int index) const;   // 1 while the ink stays, then down to 0
    QString lineState(int index) const;  // "collecting", "reading", "shown"
    Action pointerAction(int id) const { return m_ptr.contains(id) ? m_ptr[id].action : Action::Undecided; }
    QRectF writingArea() const;          // the ink not read yet, and the pen's place
    QRectF restZone() const;             // the zone the hand protection ignores fingers in (empty without a writing area)
    int ignoredTouches() const { return m_ignored; }
    int placedCursors() const { return m_placed; }
    // The ink and the last line read as a picture (screenshots of the checks)
    void advance();                      // the timers' work: the pause, the reading, the stay and the fade
    void clearInk();
    // Numbers of the one grid: the row of a y (ruled, writing lines, squares), -1 for the styles without rows
    int rowOf(double y) const;
    // the scroll bar that moves an editor's text: the editor's own, else the one of the scroll area it lies in (the text column)
    static QScrollBar *scrollBarFor(QTextEdit *ed);
    static void paintGrid(QPainter &p, const QRectF &area, const GridSettings &g, const QColor &themeText);

    // ---- the input pipeline the events feed (public: Android's code and the checks may feed it directly)
    void pointerDown(int id, Source src, const QPointF &pos, bool sideButton, double contactMm, qreal pressure = 1.0);
    void pointerMove(int id, const QPointF &pos, qreal pressure = 1.0);
    void pointerUp(int id, const QPointF &pos);
    void pointerCancel(int id);

signals:
    // a line was read (the picture is the line, the result its words); the text goes on at the fade
    void lineRead(const QImage &picture, const qb::handwriting::Result &result);
    // the ink of a read line starts to fade: its text appears in the strip now
    // (startedMs: the clock's time the fade started at, which the text's appearing follows)
    void textReady(const QImage &picture, const qb::handwriting::Result &result, qint64 startedMs);
    void readFailed(const QString &reason);
    // a pen tap or a finger tap placed the cursor of this editor (the cursor is held by the receiver)
    void cursorPlaced(QTextEdit *editor);
    // scratching deleted text of an editor
    void textScratched(QTextEdit *editor, const QString &removed);
    void inkChanged();

protected:
    void paintEvent(QPaintEvent *e) override;
    bool event(QEvent *e) override;
    void tabletEvent(QTabletEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    struct Line {
        int id = 0;
        int row = -1;
        QVector<InkStroke> strokes;
        QRectF bounds;
        qint64 lastEnd = 0;
        enum State { Collecting, Reading, Shown } state = Collecting;
        qint64 readAt = 0;
        bool revealed = false;
        Result result;
        QImage picture;
    };
    struct Pointer {
        Source src = Source::Pen;
        Action action = Action::Undecided;
        QPointF start, last;
        qint64 t0 = 0;
        InkStroke stroke;
        bool moved = false;
        int anchor = -1;
        QPointer<QTextEdit> target;
        double scrollRest = 0;
    };

    bool touchEvent(QTouchEvent *e);
    void startStroke(Pointer &p, const QPointF &pos, qreal pressure);
    void finishStroke(int id, Pointer &p, const QPointF &pos);
    void addStroke(const InkStroke &s);
    void removeStrokesIn(const QRectF &area, bool *removedAny);
    void readLine(Line &line);
    void onResult(int lineId, const Result &r);
    QImage pictureOf(const Line &line, Request *request) const;
    QPointF viewportPoint(QTextEdit *ed, const QPointF &p) const;
    int positionAt(QTextEdit *ed, const QPointF &p) const;
    void placeCursor(QTextEdit *ed, const QPointF &p);
    void selectTo(Pointer &p, const QPointF &pos);
    void scrollBy(QTextEdit *ed, double dy);
    void convertToSelect(Pointer &p);
    bool deleteWordsUnder(QTextEdit *ed, const QVector<QPointF> &pts);
    bool nearUnreadInk(const QPointF &pos) const;
    double pxPerMm() const;
    double lineReference() const;
    QColor inkColor() const;
    QColor themeText() const;
    bool anyWriting() const;
    HandProtection protection() const;
    void wake();

    Role m_role = Role::Pad;
    HandwritingSettings m_s;
    QRectF m_grid;
    bool m_gridSet = false;
    std::function<QTextEdit *(const QPoint &)> m_finder;
    QHash<int, Pointer> m_ptr;       // pen -1, mouse -2, touches by their id
    QList<Line> m_lines;
    int m_nextLine = 1;
    QTimer m_timer;
    QPointF m_penPos;
    bool m_hasPenPos = false;
    int m_ignored = 0;
    int m_placed = 0;
    QPointer<QObject> m_self;
};

} // namespace handwriting
} // namespace qb
