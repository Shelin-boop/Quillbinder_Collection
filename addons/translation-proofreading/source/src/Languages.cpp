#include "Languages.h"

#include <QCoreApplication>

namespace tp {

// the names of the languages in English, marked for the translation files (languageName() shows them in the add-on's language)
static const char *const kNames[] = {
    QT_TRANSLATE_NOOP("tp::Languages", "English"),   QT_TRANSLATE_NOOP("tp::Languages", "German"),
    QT_TRANSLATE_NOOP("tp::Languages", "Spanish"),   QT_TRANSLATE_NOOP("tp::Languages", "French"),
    QT_TRANSLATE_NOOP("tp::Languages", "Portuguese (Brazil)"), QT_TRANSLATE_NOOP("tp::Languages", "Italian"),
    QT_TRANSLATE_NOOP("tp::Languages", "Russian"),   QT_TRANSLATE_NOOP("tp::Languages", "Indonesian"),
    QT_TRANSLATE_NOOP("tp::Languages", "Turkish"),   QT_TRANSLATE_NOOP("tp::Languages", "Polish"),
    QT_TRANSLATE_NOOP("tp::Languages", "Dutch")};

const QList<Language> &languages()
{
    static const QList<Language> list{
        {QStringLiteral("en"), QStringLiteral("English"), QStringLiteral("English")},
        {QStringLiteral("de"), QStringLiteral("German"), QStringLiteral("Deutsch")},
        {QStringLiteral("es"), QStringLiteral("Spanish"), QStringLiteral("Español")},
        {QStringLiteral("fr"), QStringLiteral("French"), QStringLiteral("Français")},
        {QStringLiteral("pt"), QStringLiteral("Portuguese (Brazil)"), QStringLiteral("Português (Brasil)")},
        {QStringLiteral("it"), QStringLiteral("Italian"), QStringLiteral("Italiano")},
        {QStringLiteral("ru"), QStringLiteral("Russian"), QStringLiteral("Русский")},
        {QStringLiteral("id"), QStringLiteral("Indonesian"), QStringLiteral("Bahasa Indonesia")},
        {QStringLiteral("tr"), QStringLiteral("Turkish"), QStringLiteral("Türkçe")},
        {QStringLiteral("pl"), QStringLiteral("Polish"), QStringLiteral("Polski")},
        {QStringLiteral("nl"), QStringLiteral("Dutch"), QStringLiteral("Nederlands")},
    };
    return list;
}

bool isLanguage(const QString &code)
{
    for (const Language &l : languages())
        if (l.code == code)
            return true;
    return false;
}

QString languageName(const QString &code)
{
    const QList<Language> &all = languages();
    for (int i = 0; i < all.size(); ++i)
        if (all.at(i).code == code)
            return QCoreApplication::translate("tp::Languages", kNames[i]);
    return code;
}

QString nativeName(const QString &code)
{
    for (const Language &l : languages())
        if (l.code == code)
            return l.native;
    return code;
}

QString languageLabel(const QString &code)
{
    const QString native = nativeName(code), name = languageName(code);
    return native == name ? name : QStringLiteral("%1 (%2)").arg(native, name);
}

QList<Step> stepsFor(const QString &from, const QString &to, bool *ok)
{
    if (ok)
        *ok = isLanguage(from) && isLanguage(to);
    if (!isLanguage(from) || !isLanguage(to) || from == to)
        return {};
    const QString en = QStringLiteral("en");
    if (from == en || to == en)
        return {Step{from, to}};
    return {Step{from, en}, Step{en, to}};
}

QString modelIdFor(const Step &step)
{
    return QStringLiteral("opus-mt-%1-%2").arg(step.from, step.to);
}

QString dictionaryIdFor(const QString &code)
{
    return QStringLiteral("wiktionary-") + code;
}

QString languageFromSettings(const QString &envValue, const QString &settingValue, const QString &systemLocaleName)
{
    if (isLanguage(envValue))
        return envValue;
    if (isLanguage(settingValue))
        return settingValue;
    const QString sys = systemLocaleName.left(2).toLower();
    return isLanguage(sys) ? sys : QStringLiteral("en");
}

} // namespace tp
