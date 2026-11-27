#pragma once
// 0.99.0 (handwriting, item 112): the page of the inspector's tab "Handwriting": the place of the writing pad (the pad
// itself is the one InkCanvas; in the editor mode and in "both" it lies over the editor and this place is only covered), the
// protected strip with the read text, and the buttons Space, Backspace and Enter, Insert, Pin and Clear.
#include <QWidget>

class QLabel;
class QPushButton;
class QVBoxLayout;

namespace qb {
namespace handwriting {

class WordStrip;

class HandwritingPanel : public QWidget
{
    Q_OBJECT
public:
    explicit HandwritingPanel(QWidget *parent = nullptr);
    QWidget *padHolder() const { return m_holder; }
    QVBoxLayout *padLayout() const { return m_holderLayout; }
    WordStrip *strip() const { return m_strip; }
    QPushButton *insertButton() const { return m_insert; }
    QPushButton *pinButton() const { return m_pin; }
    QPushButton *spaceButton() const { return m_space; }
    QPushButton *backspaceButton() const { return m_back; }
    QPushButton *enterButton() const { return m_enter; }
    QPushButton *clearButton() const { return m_clear; }
    // what the place says while the pad lies elsewhere
    void setPlaceholderText(const QString &text);
    void showPlaceholder(bool on);

private:
    QWidget *m_holder = nullptr;
    QVBoxLayout *m_holderLayout = nullptr;
    QLabel *m_placeholder = nullptr;
    WordStrip *m_strip = nullptr;
    QPushButton *m_insert = nullptr, *m_pin = nullptr, *m_space = nullptr, *m_back = nullptr, *m_enter = nullptr, *m_clear = nullptr;
};

} // namespace handwriting
} // namespace qb
