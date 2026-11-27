#pragma once
// 0.99.0 (item 114): File > Options > Read Aloud. The language (the program's languages; those without a speech model greyed),
// the model with its size, languages and whether it fits this computer, the device, installing the engine and the models
// (AiInstallPanel), the speed test of the model chosen, and a pointer to the shared AI settings. Kept per device
// (readaloud::Config); OK and Apply of the Options window call apply().
#include "core/ReadAloud.h"

#include <QWidget>

class QComboBox;
class QLabel;

namespace qb {

class AiInstallPanel;

namespace readaloud {

class ReadAloudPage : public QWidget
{
    Q_OBJECT
public:
    explicit ReadAloudPage(QWidget *parent = nullptr);
    void apply();                    // the widgets -> the settings
    Settings shown() const;          // what the widgets show now
    // the speed test of the model shown (also started when the choice changes)
    void measure();
    static QString languageLabel(const QString &code);

private:
    void fill();                     // the lists, after an install or a change of language
    void showSpeed();
    void onMeasured(const QString &id, double ratio, const QString &error);

    QComboBox *m_language = nullptr, *m_model = nullptr, *m_device = nullptr;
    QLabel *m_speed = nullptr, *m_state = nullptr;
    AiInstallPanel *m_panel = nullptr;
    Settings m_keep;
    bool m_filling = false;
    QString m_measuring;
};

} // namespace readaloud
} // namespace qb
