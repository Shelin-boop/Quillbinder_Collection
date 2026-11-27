#include "ui/Language.h"
#include "core/TestSwitches.h"
#include "text/TextUtil.h"

#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QRegularExpression>
#include <QSettings>
#include <QTranslator>

namespace qb {
namespace language {

QString englishPlural(const QString &source, int n);

namespace {
// 0.99.0: the source texts with a count are written "%n word(s)" (lupdate makes the plural forms of every language from
// them); English has no translation file, so this translator, installed first (the last one it asks), writes the
// English form: "(s)" and "(es)" are dropped for 1 and become "s" and "es" otherwise.
class EnglishPlurals : public QTranslator
{
public:
    using QTranslator::QTranslator;
    QString translate(const char *, const char *sourceText, const char *, int n) const override
    {
        if (n < 0 || !sourceText)
            return QString();
        const QString src = QString::fromUtf8(sourceText);
        if (!src.contains(QLatin1String("%n")))
            return QString();
        return englishPlural(src, n);
    }
    bool isEmpty() const override { return false; }
};

// installed when the application object is made (every program and every suite), before any language file, so that it is
// the one asked last
void installEnglishPlurals()
{
    QCoreApplication::installTranslator(new EnglishPlurals(QCoreApplication::instance()));
}
Q_COREAPP_STARTUP_FUNCTION(installEnglishPlurals)

QString &activeCode()
{
    static QString c = QStringLiteral("en");
    return c;
}
} // namespace

namespace {
struct Entry
{
    const char *code;
    const char *name; // in the language itself
    bool machine;     // made by a machine translation, not yet checked by a speaker (the one place of this flag)
    QLocale::Language lang;
    QLocale::Country country;
};
const Entry &entry(int i)
{
    static const Entry list[] = {
        {"en", "English", false, QLocale::English, QLocale::AnyCountry},
        {"de", "Deutsch", false, QLocale::German, QLocale::Germany},
        {"es", "Espa\u00f1ol", true, QLocale::Spanish, QLocale::Spain},
        {"fr", "Fran\u00e7ais", true, QLocale::French, QLocale::France},
        {"pt", "Portugu\u00eas (Brasil)", true, QLocale::Portuguese, QLocale::Brazil},
        {"it", "Italiano", true, QLocale::Italian, QLocale::Italy},
        {"ru", "\u0420\u0443\u0441\u0441\u043a\u0438\u0439", true, QLocale::Russian, QLocale::Russia},
        {"id", "Bahasa Indonesia", true, QLocale::Indonesian, QLocale::Indonesia},
        {"tr", "T\u00fcrk\u00e7e", true, QLocale::Turkish, QLocale::Turkey},
        {"pl", "Polski", true, QLocale::Polish, QLocale::Poland},
        {"nl", "Nederlands", true, QLocale::Dutch, QLocale::Netherlands},
    };
    return list[i];
}
constexpr int kCount = 11;
int indexOf(const QString &code)
{
    for (int i = 0; i < kCount; ++i)
        if (code == QLatin1String(entry(i).code))
            return i;
    return -1;
}
} // namespace

QStringList available()
{
    QStringList l;
    for (int i = 0; i < kCount; ++i)
        l << QLatin1String(entry(i).code);
    return l;
}

QString displayName(const QString &code)
{
    const int i = indexOf(code);
    return QString::fromUtf8(entry(i < 0 ? 0 : i).name);
}

bool machineTranslated(const QString &code)
{
    const int i = indexOf(code);
    return i >= 0 && entry(i).machine;
}

QString listLabel(const QString &code)
{
    if (!machineTranslated(code))
        return displayName(code);
    return QCoreApplication::translate("qb::language", "%1 (machine translation)").arg(displayName(code));
}

QString fromSystem()
{
    // a test names the system's language itself (a language code; "C" or "none": none of the 11)
    QString name;
    if (QB_TEST_ENV_SET("QB_SYSTEM_LANGUAGE"))
        name = QB_TEST_ENV("QB_SYSTEM_LANGUAGE");
    else {
        const QStringList ui = QLocale::system().uiLanguages();
        name = ui.isEmpty() ? QLocale::system().name() : ui.constFirst();
    }
    const QString primary = name.section(QRegularExpression(QStringLiteral("[-_.@]")), 0, 0).toLower();
    return indexOf(primary) >= 0 ? primary : QStringLiteral("en");
}

bool isFirstStart()
{
    QSettings s;
    for (const char *k : {"app/language", "app/firstStartDone", "app/welcomeShown", "app/recent", "app/forename", "app/authorAsked"})
        if (s.contains(QLatin1String(k)))
            return false;
    return true;
}

QString configured()
{
    const QString env = qEnvironmentVariable("QB_LANGUAGE");
    if (available().contains(env))
        return env;
    QSettings s;
    const QString v = s.value(QStringLiteral("app/language")).toString();
    if (available().contains(v))
        return v;
    // 0.95.0: only at a true first start the system's language is taken; an installation that updates keeps English
    return !s.contains(QStringLiteral("app/language")) && isFirstStart() ? fromSystem() : QStringLiteral("en");
}

QString active() { return activeCode(); }

QString docResource(const QString &file, const QString &code)
{
    const QString c = code.isEmpty() ? activeCode() : code;
    if (c != QLatin1String("en")) {
        const QString path = QStringLiteral(":/docs/i18n/%1/%2").arg(c, file);
        if (QFile::exists(path))
            return path;
    }
    return QStringLiteral(":/docs/") + file;
}

QString welcomeRoot(const QString &code)
{
    const QString c = code.isEmpty() ? activeCode() : code;
    if (c != QLatin1String("en") && QFile::exists(QStringLiteral(":/welcome/i18n/%1/outline.json").arg(c)))
        return QStringLiteral(":/welcome/i18n/") + c;
    return QStringLiteral(":/welcome");
}

namespace {
bool &grouped()
{
    static bool g = true;
    return g;
}
} // namespace

QLocale localeFor(const QString &code)
{
    const int i = indexOf(code);
    if (i <= 0)
        return QLocale::system(); // English: the system's
    if (code == QLatin1String("de"))
        return QLocale(QLocale::German);
    const Entry &e = entry(i);
    // another language: its own numbers, dates and units; a system that speaks it keeps its territory (pt_PT, es_MX ...)
    const QLocale sys = QLocale::system();
    return sys.language() == e.lang ? sys : QLocale(e.lang, e.country);
}

void applyNumbers(bool groupDigits)
{
    grouped() = groupDigits;
    // the locale as before 0.90.1 (German for the German program, else the system's: units, paper sizes, the decimal
    // mark and dates follow it as they always did); only the grouping of digits is set here. The first release run of
    // 0.90.1 had en_US for English, which turned "as in the system settings" into inches and US Letter on a German
    // desktop (m6, v0212 on GitHub's machine)
    QLocale loc = localeFor(activeCode());
    loc.setNumberOptions(groupDigits ? QLocale::DefaultNumberOptions : QLocale::OmitGroupSeparator);
    QLocale::setDefault(loc);
}

QString numberExample()
{
    QLocale loc = QLocale();
    loc.setNumberOptions(QLocale::DefaultNumberOptions);
    return loc.toString(44825);
}

QString englishPlural(const QString &source, int n)
{
    QString out = source;
    out.replace(QLatin1String("(s)"), n == 1 ? QString() : QStringLiteral("s"));
    out.replace(QLatin1String("(es)"), n == 1 ? QString() : QStringLiteral("es"));
    out.replace(QLatin1String("(y|ies)"), n == 1 ? QStringLiteral("y") : QStringLiteral("ies"));
    return out;
}

void install()
{
    const QString code = configured();
    if (code == QLatin1String("en")) {
        applyNumbers(grouped());
        return;
    }
    auto *t = new QTranslator(QCoreApplication::instance());
    if (t->load(QStringLiteral(":/translations/quillbinder_%1.qm").arg(code))) {
        QCoreApplication::installTranslator(t);
        activeCode() = code;
        text::pluralLanguage() = code;
        applyNumbers(grouped());
        // the texts of Qt's own buttons and dialogs that the program shows (translations/qt_de.ts)
        auto *q = new QTranslator(QCoreApplication::instance());
        if (q->load(QStringLiteral(":/translations/qt_%1.qm").arg(code)))
            QCoreApplication::installTranslator(q);
        else
            delete q;
    } else {
        delete t;
    }
}

} // namespace language
} // namespace qb
