#include "ui/handwriting/HandwritingPanel.h"

#include "ui/handwriting/WordStrip.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace qb {
namespace handwriting {

HandwritingPanel::HandwritingPanel(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("handwritingPanel"));
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_holder = new QWidget(this);
    m_holder->setObjectName(QStringLiteral("handwritingPadHolder"));
    m_holder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_holder->setMinimumHeight(120);
    m_holderLayout = new QVBoxLayout(m_holder);
    m_holderLayout->setContentsMargins(0, 0, 0, 0);
    m_placeholder = new QLabel(m_holder);
    m_placeholder->setObjectName(QStringLiteral("handwritingPlaceholder"));
    m_placeholder->setAlignment(Qt::AlignCenter);
    m_placeholder->setWordWrap(true);
    m_placeholder->setEnabled(false);
    m_holderLayout->addWidget(m_placeholder);
    lay->addWidget(m_holder, 1);

    m_strip = new WordStrip(this);
    lay->addWidget(m_strip);

    // two rows of three buttons: one row of six made the page (and with it the whole inspector) 510 px wide at least,
    // although the Handwriting tab is hidden most of the time
    auto *row = new QHBoxLayout;
    row->setContentsMargins(4, 4, 4, 0);
    row->setSpacing(4);
    auto *row2 = new QHBoxLayout;
    row2->setContentsMargins(4, 4, 4, 4);
    row2->setSpacing(4);
    QHBoxLayout *cur = row;
    auto button = [this, &cur](const QString &name, const QString &text, const QString &tip) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(name);
        b->setToolTip(tip);
        b->setFocusPolicy(Qt::NoFocus);   // a tap on a button leaves the text where the cursor is
        cur->addWidget(b);
        return b;
    };
    m_space = button(QStringLiteral("hwSpace"), tr("Space"), tr("Add a space"));
    m_back = button(QStringLiteral("hwBackspace"), tr("Backspace"), tr("Delete the last character"));
    m_enter = button(QStringLiteral("hwEnter"), tr("Enter"), tr("Add a line break"));
    row->addStretch(1);
    cur = row2;
    m_clear = button(QStringLiteral("hwClear"), tr("Clear"), tr("Throw away the text in the strip"));
    m_pin = button(QStringLiteral("hwPin"), tr("Pin"), tr("Make the text stick to the pen or finger; tap a text field to show the place"));
    m_insert = button(QStringLiteral("hwInsert"), tr("Insert"), tr("Put the text at the cursor"));
    row2->addStretch(1);
    lay->addLayout(row);
    lay->addLayout(row2);
}

void HandwritingPanel::setPlaceholderText(const QString &text) { m_placeholder->setText(text); }
void HandwritingPanel::showPlaceholder(bool on) { m_placeholder->setVisible(on); }

} // namespace handwriting
} // namespace qb
