#pragma once
// 0.94.0 (item 79): shortcuts that follow the keyboard layout. Some of the program's shortcuts are keys that need AltGr on a
// German keyboard (Ctrl+{, Ctrl+|, Ctrl+}, Ctrl+[, Ctrl+], Ctrl+\, Ctrl+=). At the first start the program asks the system
// for the keyboard layout and keeps the answer: a German layout gets a set of its own for those commands, every other layout
// the usual set. A shortcut the writer changed (Options > Keyboard) is never replaced.
#include <QKeySequence>
#include <QMap>
#include <QString>

namespace qb {
namespace keyboardlayout {

// The platform part (KeyboardLayoutSystem.cpp): the layout the keyboard has now as a short code ("us", "de", "at", "fr" ...,
// lower case, no variant), or an empty string when the system does not tell. Linux: the environment, the desktop's
// settings, the X server; Windows: the keyboard layout's language; Android: the input method's language.
QString systemLayout();

// "us" (the usual set) or "de" (the German set), as chosen at the first start and kept in the settings
// ("keyboard/layoutSet"). `firstStart`: no shortcut settings and no earlier start; only then the system is asked.
QString chosenSet(bool firstStart);
// What the layout code of the system stands for: "de" for the German-language layouts, "us" for all others.
QString setForLayout(const QString &layout);

// The shortcut a command has out of the box in the given set; `usual` is the one the command was made with.
QKeySequence standardFor(const QString &commandKey, const QKeySequence &usual, const QString &set);
// The commands that have a shortcut of their own in the German set (key -> portable text)
QMap<QString, QString> germanSet();

} // namespace keyboardlayout
} // namespace qb
