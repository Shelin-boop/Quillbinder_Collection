// 0.99.0 (UR-04): a clear line of 2 px in the theme's highlight color around the part of the window that has the keyboard
// (binder, editor, corkboard, outliner, inspector, copyholder), in every theme. One line at a time: the window has one
// FocusLine, a transparent layer over it that draws the frame of the part the keyboard is in and nothing else.
#pragma once

#include <QColor>
#include <QPointer>
#include <QRect>
#include <QString>
#include <QWidget>
#include <functional>

namespace qb {

class EditorPane;

class FocusLine : public QWidget
{
    Q_OBJECT
public:
    FocusLine(QWidget *window, QWidget *binder, QWidget *inspector, std::function<QList<EditorPane *>()> panes);

    static constexpr int kWidth = 2;

    // The part that has the keyboard: "binder", "editor", "corkboard", "outliner", "inspector", "copyholder", or "" (none:
    // the keyboard is elsewhere, in a toolbar or another window)
    QString part() const { return m_part; }
    QWidget *partWidget() const { return m_target; }
    // The frame, in the window's coordinates; empty when no line is shown
    QRect lineRect() const { return m_part.isEmpty() ? QRect() : m_rect; }
    QColor lineColor() const; // the theme's highlight color

    // The part a widget with the keyboard belongs to (also for the checks); the widget of the part in *target
    QString partOf(QWidget *focus, QWidget **target = nullptr) const;
    void refresh(); // looks at where the keyboard is now

protected:
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void setTarget(const QString &part, QWidget *target);
    void place();

    QPointer<QWidget> m_window;
    QPointer<QWidget> m_binder, m_inspector;
    std::function<QList<EditorPane *>()> m_panes;
    QString m_part;
    QPointer<QWidget> m_target;
    QRect m_rect;
};

} // namespace qb
