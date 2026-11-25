#include "core/TestSwitches.h"
#include "ui/KeyboardLayout.h"

#include <QFile>
#include <QGuiApplication>
#include <QInputMethod>
#include <QLocale>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace qb {
namespace keyboardlayout {

namespace {

// "de(nodeadkeys)", "DE", "de-DE", "de,us" -> "de"
QString firstCode(QString text)
{
    text = text.trimmed().toLower();
    const QRegularExpression re(QStringLiteral("^[\"']?([a-z]{2,3})"));
    const auto m = re.match(text);
    return m.hasMatch() ? m.captured(1) : QString();
}

#ifdef Q_OS_LINUX
QString fromCommand(const QString &program, const QStringList &args)
{
    if (QStandardPaths::findExecutable(program).isEmpty())
        return {};
    QProcess p;
    p.start(program, args);
    if (!p.waitForFinished(1500)) {
        p.kill();
        p.waitForFinished(300);
        return {};
    }
    return p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0 ? QString::fromLocal8Bit(p.readAllStandardOutput()) : QString();
}

QString linuxLayout()
{
    // 1. the variable the compositors and many sessions set
    const QString env = QString::fromLocal8Bit(qgetenv("XKB_DEFAULT_LAYOUT"));
    if (!env.isEmpty())
        return firstCode(env);
    // 2. KDE's own keyboard settings
    {
        QSettings kde(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kxkbrc"),
                      QSettings::IniFormat);
        if (kde.value(QStringLiteral("Layout/Use")).toString() == QLatin1String("true")) {
            const QString list = kde.value(QStringLiteral("Layout/LayoutList")).toStringList().join(QLatin1Char(','));
            if (!list.isEmpty())
                return firstCode(list);
        }
    }
    // 3. the X server (also through XWayland)
    const QString x = fromCommand(QStringLiteral("setxkbmap"), {QStringLiteral("-query")});
    {
        const auto m = QRegularExpression(QStringLiteral("^layout:\\s*(\\S+)"), QRegularExpression::MultilineOption).match(x);
        if (m.hasMatch())
            return firstCode(m.captured(1));
    }
    // 4. GNOME's input sources, e.g. [('xkb', 'de')]
    const QString g = fromCommand(QStringLiteral("gsettings"), {QStringLiteral("get"), QStringLiteral("org.gnome.desktop.input-sources"),
                                                               QStringLiteral("sources")});
    {
        const auto m = QRegularExpression(QStringLiteral("'xkb',\\s*'([^']+)'")).match(g);
        if (m.hasMatch())
            return firstCode(m.captured(1));
    }
    // 5. the system's default
    QFile f(QStringLiteral("/etc/default/keyboard"));
    if (f.open(QIODevice::ReadOnly)) {
        const auto m = QRegularExpression(QStringLiteral("^XKBLAYOUT=(.*)$"), QRegularExpression::MultilineOption)
                           .match(QString::fromLocal8Bit(f.readAll()));
        if (m.hasMatch())
            return firstCode(m.captured(1));
    }
    return {};
}
#endif

#ifdef Q_OS_WIN
QString windowsLayout()
{
    // the language of the keyboard layout of the thread that has the keyboard: 0x07 = German (sub-language 1 Germany,
    // 3 Austria); the other languages are not told apart
    const HKL hkl = GetKeyboardLayout(0);
    const unsigned lang = unsigned(reinterpret_cast<quintptr>(hkl)) & 0xFFFFu;
    const unsigned primary = lang & 0x3FFu;
    const unsigned sub = lang >> 10;
    if (primary == 0x07)
        return sub == 3 ? QStringLiteral("at") : sub == 1 ? QStringLiteral("de") : QStringLiteral("de-other");
    if (primary == 0x09)
        return QStringLiteral("en");
    return QStringLiteral("other");
}
#endif

} // namespace

QString systemLayout()
{
    // a test names the layout itself ("de", "us", "fr", ...; "none": the system does not tell)
    if (QB_TEST_ENV_SET("QB_KEYBOARD_LAYOUT")) {
        const QString v = QB_TEST_ENV("QB_KEYBOARD_LAYOUT").trimmed().toLower();
        return v == QLatin1String("none") ? QString() : v;
    }
#if defined(Q_OS_WIN)
    return windowsLayout();
#elif defined(Q_OS_ANDROID)
    // the language of the input method; the language says "German", the layout is the keyboard's German one
    if (QGuiApplication::instance())
        if (QInputMethod *im = QGuiApplication::inputMethod()) {
            const QLocale l = im->locale();
            if (l.language() == QLocale::German)
                return l.territory() == QLocale::Austria ? QStringLiteral("at") : QStringLiteral("de");
            return QStringLiteral("other");
        }
    return {};
#elif defined(Q_OS_LINUX)
    return linuxLayout();
#else
    return {};
#endif
}

} // namespace keyboardlayout
} // namespace qb
