#pragma once
// What the format bar needs from a text that is not a TextEditor (0.99.0): a small interface, so that the bar's
// code does not reach into the mind map. The mind map's controller implements it for the notes of a board.
// The keys are the format bar's own action keys ("bold", "italic", "underline", "strike", "textColor",
// "textColorAuto", "alignLeft", "alignCenter", "alignRight", "alignJustify", "fontBigger", "fontSmaller",
// "size", "clearFormat"); any other key has no meaning for a note and is not enabled.

#include <QColor>
#include <QString>
#include <QVariant>

class QWidget;

namespace qb {

struct NoteFormatState {
    // 1 on, 0 off, -1 mixed (the neutral state); for the text of a note being edited: the selected words
    int bold = 0, italic = 0, underline = 0, strike = 0;
    double size = 0;          // 0: mixed
    QColor textColor;         // invalid: automatic
    bool textColorMixed = false;
    int align = -1;           // 0 left, 1 center, 2 right, 3 justified, -1 mixed
};

class NoteFormatTarget
{
public:
    virtual ~NoteFormatTarget() = default;
    // notes are selected or one is being edited
    virtual bool noteFormatActive() const = 0;
    virtual bool noteFormatEnabled(const QString &key) const = 0;
    virtual NoteFormatState noteFormatState() const = 0;
    // one undo step, named after the change; value: a QColor (textColor), a number (size)
    virtual bool noteFormatApply(const QString &key, const QVariant &value = QVariant()) = 0;

    // the target of the widget that has the keyboard (a mind map's view), or nullptr
    static NoteFormatTarget *forWidget(QWidget *focus);
};

} // namespace qb
