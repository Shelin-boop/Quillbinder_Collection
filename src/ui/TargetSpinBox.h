#pragma once
// A field for a word target (0.95.0): whole numbers from a negative limit to a positive one; 0 reads "no target" (the text
// is the caller's); a negative number (a cutting target) is shown in red. Also the red used for negative numbers in
// labels.
#include <QColor>
#include <QSpinBox>

namespace qb {

class TargetSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit TargetSpinBox(QWidget *parent = nullptr);
    void setNoTargetText(const QString &text);
    QString textFromValue(int value) const override;
    int valueFromText(const QString &text) const override;
    QValidator::State validate(QString &input, int &pos) const override;

private:
    void updateColor();
    QString m_none;
};

namespace targettext {
// the red of a negative number, readable (at least 4.5 : 1) on the background of the widget
QColor negativeColor(const QColor &background);
// "<span style=color>-350</span>" for a negative number, the plain number otherwise (rich text only where it has to be)
QString numberHtml(int value, const QColor &background);
} // namespace targettext

} // namespace qb
