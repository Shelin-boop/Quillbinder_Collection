#pragma once
// 1.5: the eleven languages the Translation Helper translates between, and which models a pair of them needs.
// Qt Core only (the checks compile it alone).
//
// OPUS-MT models go to and from English; a pair without English in it goes through English (two models). The models
// are named in the catalog (resources/catalog.json: "from" and "to" of each translation model); this file only knows
// the languages and the way through English.
#include <QList>
#include <QString>
#include <QStringList>

namespace tp {

struct Language {
    QString code;      // "en", "de", "es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"
    QString english;   // the name in English ("Spanish")
    QString native;    // the name in the language itself ("Español"); Portuguese is Brazilian
};

// All eleven, English and German first, then in the order of the program's languages.
const QList<Language> &languages();
bool isLanguage(const QString &code);
QString languageName(const QString &code);        // "Spanish"
QString nativeName(const QString &code);          // "Español"
// "Español (Spanish)", for a drop-down list that is read by writers of any language; English: "English"
QString languageLabel(const QString &code);

// One step of a translation: a model from one language to another, one of them English.
struct Step {
    QString from, to;
};
// The steps of a translation from one language to another: none when both are the same, one when English is one of
// them, else two (through English). Empty with *ok false for an unknown language.
QList<Step> stepsFor(const QString &from, const QString &to, bool *ok = nullptr);
// The id of the catalog model that makes a step: "opus-mt-<from>-<to>" for the models of 1.5 and 1.3 (German to English
// is "opus-mt-de-en", English to German "opus-mt-en-de"; the catalog's "opus-mt-big-de-en" is a second choice
// for German to English that the settings may pick).
QString modelIdFor(const Step &step);
// The id of the dictionary of a language: "wiktionary-<code>" (English has none: WordNet is built in).
QString dictionaryIdFor(const QString &code);

// The language the program speaks now: the environment variable QB_LANGUAGE, else the setting "app/language" of the
// program (read from its own settings, the tool interface does not tell it), else the system's language; one of the
// eleven, "en" when it is none of them. `system` is the name of a locale ("de_DE") for the last case.
QString languageFromSettings(const QString &envValue, const QString &settingValue, const QString &systemLocaleName);

} // namespace tp
