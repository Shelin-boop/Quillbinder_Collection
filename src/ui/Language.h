#pragma once
// 0.70.0 (the writer's decision of 2026-11-10, audit A S-06): the program speaks English or German; since 0.95.0 eleven
// languages (item 102).
// The language is a program-wide setting (app/language: "en" or "de"; File > Options > General > Startup) and
// takes effect at the next start; QB_LANGUAGE overrides it (the checks). The German texts are
// translations/quillbinder_de.ts (made with lupdate; compiled with lrelease into quillbinder_de.qm, a resource).
// They also hold the few texts of Qt's own windows and buttons that the program shows (Yes, Cancel, the
// context menu of text fields, the file dialog, key names in menus), translated here.
#include <QLocale>
#include <QString>
#include <QStringList>

namespace qb {
namespace language {

QStringList available();                 // "en", "de", "es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"
QString displayName(const QString &code); // each language's own name: "English", "Deutsch", "Português (Brasil)" ...
// 0.95.0 (item 102): the nine new languages are machine translations until a speaker has checked them. This flag is the
// one place: take the code out of the table in Language.cpp when its translation has been checked.
bool machineTranslated(const QString &code);
// the name for a list: the name, with "(machine translation)" (in the program's language) while it is not checked
QString listLabel(const QString &code);
// the system's language when it is one of the 11, else "en" (QB_SYSTEM_LANGUAGE names it in the checks)
QString fromSystem();
// a true first start: no settings of the program yet (an installation that updates is not one)
bool isFirstStart();
// 0.95.0: the manual, the quick start guide and the welcome project in a language: ":/docs/i18n/<code>/<file>" and
// ":/welcome/i18n/<code>" when the program's language has them, else the English ones (":/docs/<file>", ":/welcome")
QString docResource(const QString &file, const QString &code = QString());
QString welcomeRoot(const QString &code = QString());
// the locale of the numbers, dates and units for a language (German: Germany; English: the system's)
QLocale localeFor(const QString &code);
QString configured();                    // what the settings (or QB_LANGUAGE) ask for
QString active();                        // what install() put in place
// Installs the translator of the configured language (call once, right after the QApplication is made).
void install();
// 0.90.1 (the usability review of 0.90.0, UR-18): large numbers with their digits grouped as the locale writes them
// (the German program: 44.825; else the system's setting, 44,825 under English or "C", 44.825 on a German desktop),
// unless the writer turned that off (File > Options > General > Startup, app/groupDigits). Sets Qt's default locale
// (the system's, or German for the German program, as before) with only its number options changed: every count of
// the program goes through it. Before, a "C" setting (test machines) wrote 44825.
void applyNumbers(bool groupDigits);
// how 44825 looks with the digits grouped
QString numberExample();

} // namespace language
} // namespace qb
