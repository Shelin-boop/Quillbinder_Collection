#include "ui/KeyboardLayout.h"

#include <QSettings>

namespace qb {
namespace keyboardlayout {

QString setForLayout(const QString &layout)
{
    // "de" and "at" are the German-language layouts: the same keys, the same places for { | } [ ] \ =
    return layout == QLatin1String("de") || layout == QLatin1String("at") ? QStringLiteral("de") : QStringLiteral("us");
}

QString chosenSet(bool firstStart)
{
    QSettings s;
    const QString kept = s.value(QStringLiteral("keyboard/layoutSet")).toString();
    if (kept == QLatin1String("us") || kept == QLatin1String("de"))
        return kept;
    if (!firstStart)
        return QStringLiteral("us"); // a program that was used before keeps its shortcuts (nothing is stored)
    const QString set = setForLayout(systemLayout());
    s.setValue(QStringLiteral("keyboard/layoutSet"), set);
    return set;
}

QMap<QString, QString> germanSet()
{
    // keys that need no AltGr on the German keyboard (and no Ctrl+Alt, which Windows reads as AltGr): Alt+Shift+letter for the
    // alignments, Alt+Shift+B and Alt+Shift+F for the history, Ctrl+Shift+letter for the rest. The commands not named here keep
    // their usual keys. The tests check the set against every other shortcut.
    return {{QStringLiteral("alignLeft"), QStringLiteral("Alt+Shift+L")},
            {QStringLiteral("alignCenter"), QStringLiteral("Alt+Shift+E")},
            {QStringLiteral("alignRight"), QStringLiteral("Alt+Shift+R")},
            {QStringLiteral("back"), QStringLiteral("Alt+Shift+B")},
            {QStringLiteral("forward"), QStringLiteral("Alt+Shift+F")},
            {QStringLiteral("scriptElements"), QStringLiteral("Ctrl+Shift+O")},
            {QStringLiteral("subscript"), QStringLiteral("Ctrl+Shift+D")},
            // the mind map's own commands (they are not in the list of Options > Keyboard)
            {QStringLiteral("mm_textAlign0"), QStringLiteral("Alt+Shift+L")},
            {QStringLiteral("mm_textAlign1"), QStringLiteral("Alt+Shift+E")},
            {QStringLiteral("mm_textAlign2"), QStringLiteral("Alt+Shift+J")},
            {QStringLiteral("mm_textAlign3"), QStringLiteral("Alt+Shift+R")},
            {QStringLiteral("mm_front"), QStringLiteral("Ctrl+Shift+PgUp")},
            {QStringLiteral("mm_back"), QStringLiteral("Ctrl+Shift+PgDown")}};
}

QKeySequence standardFor(const QString &commandKey, const QKeySequence &usual, const QString &set)
{
    if (set != QLatin1String("de"))
        return usual;
    const QMap<QString, QString> g = germanSet();
    const auto it = g.constFind(commandKey);
    return it == g.constEnd() ? usual : QKeySequence(*it, QKeySequence::PortableText);
}

} // namespace keyboardlayout
} // namespace qb
