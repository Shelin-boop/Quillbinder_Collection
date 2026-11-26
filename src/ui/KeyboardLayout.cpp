#include "ui/KeyboardLayout.h"

#include <QSettings>

namespace qb {
namespace keyboardlayout {

QStringList sets() { return {QStringLiteral("us"), QStringLiteral("de"), QStringLiteral("lt"), QStringLiteral("pl")}; }

QString setForLayout(const QString &layout)
{
    // "de" and "at" are the German-language layouts: the same keys, the same places for { | } [ ] \ =
    if (layout == QLatin1String("de") || layout == QLatin1String("at"))
        return QStringLiteral("de");
    // 0.95.0 (item 102): the layouts of the other languages. French (AZERTY, Belgian), Spanish (also Latin American),
    // Italian, Portuguese (Brazilian ABNT2 and Portugal's), Turkish Q and Dutch need AltGr for [ ] { } and \ | (or lack
    // = and +); their set is the German one (letters only) and moves the four Ctrl+Alt+letter commands that AltGr
    // takes for the letters of those layouts. Polish (programmer) is the US layout with AltGr letters: only those four
    // move. Russian (Latin keys of its layout group) and Indonesian (US) keep the usual keys.
    static const QStringList latin = {QStringLiteral("fr"), QStringLiteral("be"), QStringLiteral("es"), QStringLiteral("latam"),
                                      QStringLiteral("it"), QStringLiteral("br"), QStringLiteral("pt"), QStringLiteral("tr"),
                                      QStringLiteral("nl")};
    if (latin.contains(layout))
        return QStringLiteral("lt");
    if (layout == QLatin1String("pl"))
        return QStringLiteral("pl");
    return QStringLiteral("us");
}

QString chosenSet(bool firstStart)
{
    QSettings s;
    const QString kept = s.value(QStringLiteral("keyboard/layoutSet")).toString();
    if (sets().contains(kept))
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

QMap<QString, QString> awayFromAltGr()
{
    // Ctrl+Alt is AltGr on Windows keyboards: Ctrl+Alt+C, +N (+V, +R) are the letters ć ń (and ₢ and the like) of the
    // Polish and Brazilian layouts. These commands get Ctrl+Shift keys, which no layout takes.
    return {{QStringLiteral("copyFormat"), QStringLiteral("Ctrl+Shift+Y")},
            {QStringLiteral("pasteFormat"), QStringLiteral("Ctrl+Shift+U")},
            {QStringLiteral("goNotes"), QStringLiteral("Ctrl+Shift+X")},
            {QStringLiteral("showRuler"), QStringLiteral("Ctrl+Shift+W")}};
}

QMap<QString, QString> setTable(const QString &set)
{
    if (set == QLatin1String("de"))
        return germanSet();
    QMap<QString, QString> t;
    if (set == QLatin1String("lt"))
        t = germanSet();
    if (set == QLatin1String("lt") || set == QLatin1String("pl")) {
        const QMap<QString, QString> a = awayFromAltGr();
        for (auto it = a.begin(); it != a.end(); ++it)
            if (set == QLatin1String("lt") || it.key() != QLatin1String("showRuler"))
                t.insert(it.key(), it.value());
    }
    return t;
}

QKeySequence standardFor(const QString &commandKey, const QKeySequence &usual, const QString &set)
{
    if (set == QLatin1String("us") || !sets().contains(set))
        return usual;
    const QMap<QString, QString> g = setTable(set);
    const auto it = g.constFind(commandKey);
    return it == g.constEnd() ? usual : QKeySequence(*it, QKeySequence::PortableText);
}

} // namespace keyboardlayout
} // namespace qb
