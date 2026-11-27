#pragma once
// 0.99.0 (handwriting, item 112): the settings of the writing surface. Kept per device (QSettings, group "handwriting"),
// never in a project. File > Options > Handwriting shows and changes them; the surface reads them through
// HandwritingConfig, which tells when they changed.
#include <QColor>
#include <QObject>
#include <QString>

namespace qb {
namespace handwriting {

enum class Mode { Off = 0, Editor = 1, Inspector = 2, Both = 3 };

struct GridSettings {
    enum Style { Ruled = 0, Writing, Squares, Dots, None };
    Style style = Ruled;
    QColor color;                 // invalid: the theme's text color
    double lineWidth = 1.0;       // pixels, 0.5 .. 4
    int opacity = 35;             // percent, 5 .. 100
    int size = 45;                // 0 (small) .. 100 (large): a slider, no unit; pitch() turns it into pixels
    // the distance between two ruled lines in pixels at a device pixel ratio of 1: 24 .. 84
    double pitch() const { return 24.0 + 60.0 * size / 100.0; }
    static QString styleName(Style s);
    static QString styleKey(Style s);          // "ruled", "writing", "squares", "dots", "none"
    static Style styleFromKey(const QString &key);
};

struct HandwritingSettings {
    // the button and the mode
    bool buttonShown = false;          // Off by default on Linux and Windows, on by default on Android
    Mode mode = Mode::Off;
    Mode lastMode = Mode::Inspector;   // what a tap on the button switches to

    // recognition
    QString mainLanguage = QStringLiteral("en");
    QString fallbackLanguage;          // empty: none

    // writing
    int pauseMs = 1000;                // the pause after the last stroke before a line is read
    int stayMs = 3000;                 // how long the ink stays after the line was read
    int fadeMs = 2000;                 // the fade
    QColor inkColor;                   // invalid: the theme's text color
    double inkWidth = 2.5;             // pixels

    GridSettings grid;

    // hand protection (starting values)
    bool protectHand = true;
    bool leftHanded = false;
    int strength = 50;                 // 0 .. 100: scales the rest zone and the size of a contact that counts as a palm

    // pen and finger
    bool fingerWrites = true;          // without a pen one finger writes (else a finger only points)
    bool sideButtonSelects = true;     // the pen's side button selects
    int penInUseSeconds = 30;          // a pen counts as in use this long after its last touch

    // learning the writer's hand
    bool learning = true;              // keep corrected lines on this device (never sent anywhere)

    // the defaults of a system (Android: the button shown)
    static HandwritingSettings defaults(bool android);
    static HandwritingSettings load();
    void save() const;
};

// The one copy the program works with.
class HandwritingConfig : public QObject
{
    Q_OBJECT
public:
    static HandwritingConfig &instance();
    const HandwritingSettings &get() const { return m_s; }
    void set(const HandwritingSettings &s);      // saves and tells
    void reload();                                // from QSettings (tests change the settings' place)
    static bool isAndroid();                      // the system's default switch (a test may force it)
    static void forceAndroid(int on);             // -1: the real system; 0 / 1: the checks

signals:
    void changed();

private:
    HandwritingConfig();
    HandwritingSettings m_s;
};

} // namespace handwriting
} // namespace qb
