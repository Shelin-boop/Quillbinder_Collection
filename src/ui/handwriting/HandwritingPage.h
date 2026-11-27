#pragma once
// 0.99.0 (handwriting, item 112): File > Options > Handwriting. Recognition (the install part, the languages),
// writing, the grid, hand protection, pen and finger, learning the hand. Kept per device (HandwritingConfig); OK and Apply of
// the Options window call apply().
#include "ui/handwriting/HandwritingSettings.h"

#include <QColor>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QSlider;
class QSpinBox;

namespace qb {
namespace handwriting {

// The sample beside the grid size slider: the grid as it will be drawn
class GridSample : public QWidget
{
    Q_OBJECT
public:
    explicit GridSample(QWidget *parent = nullptr);
    void setGrid(const GridSettings &g);
    const GridSettings &grid() const { return m_grid; }
    QSize sizeHint() const override { return QSize(150, 110); }

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    GridSettings m_grid;
};

class HandwritingPage : public QWidget
{
    Q_OBJECT
public:
    explicit HandwritingPage(QWidget *parent = nullptr);
    void markStorage();                             // the tool tips of where each option is kept (optstore)
    void apply();                                   // the widgets -> the settings
    HandwritingSettings shown() const;              // what the widgets show now
    void load(const HandwritingSettings &s);        // the settings -> the widgets
    // the language list: every program language, Russian greyed out with "no model for this language yet"
    static QString languageLabel(const QString &code);

private:
    void refreshInstall();
    void refreshLearning();
    void pickColor(QColor *target, QPushButton *button, const QString &title);
    void showColor(QPushButton *button, const QColor &c);

    QLabel *m_installStatus = nullptr;
    QPushButton *m_install = nullptr, *m_remove = nullptr;
    QCheckBox *m_buttonShown = nullptr;
    QComboBox *m_main = nullptr, *m_fallback = nullptr;
    QDoubleSpinBox *m_pause = nullptr, *m_stay = nullptr, *m_fade = nullptr, *m_inkWidth = nullptr;
    QPushButton *m_inkColorButton = nullptr;
    QColor m_inkColor;
    QComboBox *m_gridStyle = nullptr;
    QPushButton *m_gridColorButton = nullptr;
    QColor m_gridColor;
    QDoubleSpinBox *m_gridWidth = nullptr;
    QSpinBox *m_gridOpacity = nullptr;
    QSlider *m_gridSize = nullptr;
    GridSample *m_sample = nullptr;
    QCheckBox *m_protect = nullptr;
    QComboBox *m_hand = nullptr;
    QSlider *m_strength = nullptr;
    QCheckBox *m_fingerWrites = nullptr, *m_sideButton = nullptr;
    QSpinBox *m_penSeconds = nullptr;
    QCheckBox *m_learning = nullptr;
    QLabel *m_learned = nullptr;
    HandwritingSettings m_keep;   // the values the page does not show (the mode, the last mode)
};

} // namespace handwriting
} // namespace qb
