#include "ui/TargetSpinBox.h"
#include "ui/Theme.h"

#include <QLineEdit>
#include <QLocale>
#include <QPalette>

namespace qb {

TargetSpinBox::TargetSpinBox(QWidget *parent) : QSpinBox(parent)
{
    setRange(-10000000, 10000000);
    setSingleStep(100);
    connect(this, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updateColor(); });
}

void TargetSpinBox::setNoTargetText(const QString &text)
{
    m_none = text;
    updateColor();
    lineEdit()->setText(textFromValue(value()));
}

QString TargetSpinBox::textFromValue(int value) const
{
    if (value == 0 && !m_none.isEmpty())
        return m_none;
    return QSpinBox::textFromValue(value);
}

int TargetSpinBox::valueFromText(const QString &text) const
{
    if (!m_none.isEmpty() && text.trimmed().compare(m_none, Qt::CaseInsensitive) == 0)
        return 0;
    return QSpinBox::valueFromText(text);
}

QValidator::State TargetSpinBox::validate(QString &input, int &pos) const
{
    if (!m_none.isEmpty() && m_none.startsWith(input.trimmed(), Qt::CaseInsensitive) && !input.trimmed().isEmpty())
        return input.trimmed().compare(m_none, Qt::CaseInsensitive) == 0 ? QValidator::Acceptable : QValidator::Intermediate;
    return QSpinBox::validate(input, pos);
}

void TargetSpinBox::updateColor()
{
    if (value() < 0) {
        QPalette pal = palette();
        pal.setColor(QPalette::Text, targettext::negativeColor(pal.color(QPalette::Base)));
        setPalette(pal);
    } else {
        setPalette(QPalette()); // the look of the other fields
    }
}

namespace targettext {

QColor negativeColor(const QColor &background)
{
    return theme::withContrast(QColor(205, 40, 40), background, 4.5);
}

QString numberHtml(int value, const QColor &background)
{
    const QString n = QLocale().toString(value);
    if (value >= 0)
        return n;
    return QStringLiteral("<span style=\"color:%1\">%2</span>").arg(negativeColor(background).name(), n);
}

} // namespace targettext

} // namespace qb
