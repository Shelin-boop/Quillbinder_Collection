#pragma once
// 0.94.0 (item 79): shortcuts that follow the keyboard layout. Some of the program's shortcuts are keys that need AltGr on a
// German keyboard (Ctrl+{, Ctrl+|, Ctrl+}, Ctrl+[, Ctrl+], Ctrl+\, Ctrl+=). At the first start the program asks the system
// for the keyboard layout and keeps the answer: a German layout gets a set of its own for those commands, every other layout
// the usual set. A shortcut the writer changed (Options > Keyboard) is never replaced.
#include <QKeySequence>
#include <QMap>
#include <QString>
#include <QStringList>

namespace qb {
namespace keyboardlayout {

// The platform part (KeyboardLayoutSystem.cpp): the layout the keyboard has now as a short code ("us", "de", "at", "fr" ...,
// lower case, no variant), or an empty string when the system does not tell. Linux: the environment, the desktop's
// settings, the X server; Windows: the keyboard layout's language; Android: the input method's language.
QString systemLayout();

// 0.95.0 (item 102): the sets: "us" (the usual keys), "de" (German keyboards), "lt" (French, Belgian, Spanish, Italian,
// Portuguese, Turkish and Dutch keyboards: the German set and four more commands off AltGr's letters) and "pl" (Polish
// programmer's keyboard: only those four commands move). sets() lists them.
QStringList sets();
// "us", "de", "lt" or "pl", as chosen at the first start and kept in the settings
// ("keyboard/layoutSet"); the one place that picks a set by layout is setForLayout(). `firstStart`: no shortcut settings and no earlier start; only then the system is asked.
QString chosenSet(bool firstStart);
// What the layout code of the system stands for ("de", "at": "de"; "fr", "es", "it", "br", "pt", "tr", "nl" ...: "lt"; "pl": "pl";
// all others, an unknown one and an empty one: "us").
QString setForLayout(const QString &layout);

// The shortcut a command has out of the box in the given set; `usual` is the one the command was made with.
QKeySequence standardFor(const QString &commandKey, const QKeySequence &usual, const QString &set);
// The commands that have a shortcut of their own in the German set (key -> portable text)
QMap<QString, QString> germanSet();
// The commands that have a shortcut of their own in a set ("us": none), key -> portable text
QMap<QString, QString> setTable(const QString &set);
// The commands that Ctrl+Alt+letter would take from AltGr's letters (key -> the key they have instead in "lt" and "pl")
QMap<QString, QString> awayFromAltGr();

} // namespace keyboardlayout
} // namespace qb
