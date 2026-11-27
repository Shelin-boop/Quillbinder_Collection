#include "ui/handwriting/HandwritingSettings.h"

#include <QCoreApplication>
#include <QSettings>

namespace qb {
namespace handwriting {

QString GridSettings::styleKey(Style s)
{
    switch (s) {
    case Ruled: return QStringLiteral("ruled");
    case Writing: return QStringLiteral("writing");
    case Squares: return QStringLiteral("squares");
    case Dots: return QStringLiteral("dots");
    case None: break;
    }
    return QStringLiteral("none");
}

GridSettings::Style GridSettings::styleFromKey(const QString &key)
{
    for (Style s : {Ruled, Writing, Squares, Dots, None})
        if (styleKey(s) == key)
            return s;
    return Ruled;
}

QString GridSettings::styleName(Style s)
{
    switch (s) {
    case Ruled: return QCoreApplication::translate("Handwriting", "Ruled lines");
    case Writing: return QCoreApplication::translate("Handwriting", "Writing lines (baseline and midline)");
    case Squares: return QCoreApplication::translate("Handwriting", "Squares");
    case Dots: return QCoreApplication::translate("Handwriting", "Dots");
    case None: break;
    }
    return QCoreApplication::translate("Handwriting", "No grid");
}

HandwritingSettings HandwritingSettings::defaults(bool android)
{
    HandwritingSettings s;
    s.buttonShown = android;   // on by default on Android, hidden on Linux and Windows
    return s;
}

static int androidForce = -1;

bool HandwritingConfig::isAndroid()
{
    if (androidForce >= 0)
        return androidForce != 0;
#ifdef Q_OS_ANDROID
    return true;
#else
    return false;
#endif
}

void HandwritingConfig::forceAndroid(int on) { androidForce = on; }

static Mode modeFrom(int v, Mode fallback)
{
    return v >= 0 && v <= 3 ? Mode(v) : fallback;
}

static QColor colorFrom(const QString &name)
{
    return name.isEmpty() ? QColor() : QColor(name);
}

HandwritingSettings HandwritingSettings::load()
{
    HandwritingSettings s = defaults(HandwritingConfig::isAndroid());
    QSettings q;
    q.beginGroup(QStringLiteral("handwriting"));
    s.buttonShown = q.value(QStringLiteral("buttonShown"), s.buttonShown).toBool();
    s.mode = modeFrom(q.value(QStringLiteral("mode"), int(s.mode)).toInt(), s.mode);
    s.lastMode = modeFrom(q.value(QStringLiteral("lastMode"), int(s.lastMode)).toInt(), s.lastMode);
    if (s.lastMode == Mode::Off)
        s.lastMode = Mode::Inspector;
    s.mainLanguage = q.value(QStringLiteral("mainLanguage"), s.mainLanguage).toString();
    s.fallbackLanguage = q.value(QStringLiteral("fallbackLanguage"), s.fallbackLanguage).toString();
    s.pauseMs = qBound(200, q.value(QStringLiteral("pauseMs"), s.pauseMs).toInt(), 10000);
    s.stayMs = qBound(0, q.value(QStringLiteral("stayMs"), s.stayMs).toInt(), 30000);
    s.fadeMs = qBound(0, q.value(QStringLiteral("fadeMs"), s.fadeMs).toInt(), 30000);
    s.inkColor = colorFrom(q.value(QStringLiteral("inkColor")).toString());
    s.inkWidth = qBound(0.5, q.value(QStringLiteral("inkWidth"), s.inkWidth).toDouble(), 12.0);
    s.grid.style = GridSettings::styleFromKey(q.value(QStringLiteral("grid/style"), GridSettings::styleKey(s.grid.style)).toString());
    s.grid.color = colorFrom(q.value(QStringLiteral("grid/color")).toString());
    s.grid.lineWidth = qBound(0.5, q.value(QStringLiteral("grid/lineWidth"), s.grid.lineWidth).toDouble(), 4.0);
    s.grid.opacity = qBound(5, q.value(QStringLiteral("grid/opacity"), s.grid.opacity).toInt(), 100);
    s.grid.size = qBound(0, q.value(QStringLiteral("grid/size"), s.grid.size).toInt(), 100);
    s.protectHand = q.value(QStringLiteral("protect/on"), s.protectHand).toBool();
    s.leftHanded = q.value(QStringLiteral("protect/left"), s.leftHanded).toBool();
    s.strength = qBound(0, q.value(QStringLiteral("protect/strength"), s.strength).toInt(), 100);
    s.fingerWrites = q.value(QStringLiteral("pen/fingerWrites"), s.fingerWrites).toBool();
    s.sideButtonSelects = q.value(QStringLiteral("pen/sideButtonSelects"), s.sideButtonSelects).toBool();
    s.penInUseSeconds = qBound(1, q.value(QStringLiteral("pen/inUseSeconds"), s.penInUseSeconds).toInt(), 3600);
    s.learning = q.value(QStringLiteral("learning/on"), s.learning).toBool();
    return s;
}

void HandwritingSettings::save() const
{
    QSettings q;
    q.beginGroup(QStringLiteral("handwriting"));
    q.setValue(QStringLiteral("buttonShown"), buttonShown);
    q.setValue(QStringLiteral("mode"), int(mode));
    q.setValue(QStringLiteral("lastMode"), int(lastMode));
    q.setValue(QStringLiteral("mainLanguage"), mainLanguage);
    q.setValue(QStringLiteral("fallbackLanguage"), fallbackLanguage);
    q.setValue(QStringLiteral("pauseMs"), pauseMs);
    q.setValue(QStringLiteral("stayMs"), stayMs);
    q.setValue(QStringLiteral("fadeMs"), fadeMs);
    q.setValue(QStringLiteral("inkColor"), inkColor.isValid() ? inkColor.name(QColor::HexArgb) : QString());
    q.setValue(QStringLiteral("inkWidth"), inkWidth);
    q.setValue(QStringLiteral("grid/style"), GridSettings::styleKey(grid.style));
    q.setValue(QStringLiteral("grid/color"), grid.color.isValid() ? grid.color.name(QColor::HexArgb) : QString());
    q.setValue(QStringLiteral("grid/lineWidth"), grid.lineWidth);
    q.setValue(QStringLiteral("grid/opacity"), grid.opacity);
    q.setValue(QStringLiteral("grid/size"), grid.size);
    q.setValue(QStringLiteral("protect/on"), protectHand);
    q.setValue(QStringLiteral("protect/left"), leftHanded);
    q.setValue(QStringLiteral("protect/strength"), strength);
    q.setValue(QStringLiteral("pen/fingerWrites"), fingerWrites);
    q.setValue(QStringLiteral("pen/sideButtonSelects"), sideButtonSelects);
    q.setValue(QStringLiteral("pen/inUseSeconds"), penInUseSeconds);
    q.setValue(QStringLiteral("learning/on"), learning);
}

HandwritingConfig::HandwritingConfig() { m_s = HandwritingSettings::load(); }

HandwritingConfig &HandwritingConfig::instance()
{
    static HandwritingConfig c;
    return c;
}

void HandwritingConfig::set(const HandwritingSettings &s)
{
    m_s = s;
    m_s.save();
    emit changed();
}

void HandwritingConfig::reload()
{
    m_s = HandwritingSettings::load();
    emit changed();
}

} // namespace handwriting
} // namespace qb
