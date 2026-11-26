#pragma once
#include "text/TextProps.h"

#include <QColor>
#include <QList>
#include <QLocale>
#include <QString>
#include <QTextCharFormat>

class QTextDocument;
class QTextCursor;

namespace qb {
namespace text {

// "1 theme" / "3 themes". 0.95.0 (item 102): the form of the word follows the language of the program (set by ui/Language.cpp): French and Portuguese call 0 and 1 singular; Indonesian and Turkish have no plural after a
// number (the singular is used); Russian and Polish have a third form for 2 to 4 (22, 23 ... too): the translation of the
// plural word may then carry two forms, "few|many" (Russian: "документа|документов"); with one form it is used for both.
// the language code of the program ("en", "ru" ...), set by ui/Language.cpp: the rules follow it, not the system's locale
inline QString &pluralLanguage()
{
    static QString code = QStringLiteral("en");
    return code;
}

inline QString pluralForm(int n, const QString &one, const QString &many)
{
    const QString code = pluralLanguage();
    const QLocale::Language lang = code == QLatin1String("id") ? QLocale::Indonesian
        : code == QLatin1String("tr") ? QLocale::Turkish : code == QLatin1String("fr") ? QLocale::French
        : code == QLatin1String("pt") ? QLocale::Portuguese : code == QLatin1String("ru") ? QLocale::Russian
        : code == QLatin1String("pl") ? QLocale::Polish : QLocale::English;
    const int a = n < 0 ? -n : n;
    switch (lang) {
    case QLocale::Indonesian:
    case QLocale::Turkish:
        return one;
    case QLocale::French:
    case QLocale::Portuguese:
        return a <= 1 ? one : many.section(QLatin1Char('|'), -1);
    case QLocale::Russian:
    case QLocale::Polish: {
        if (lang == QLocale::Russian ? (a % 10 == 1 && a % 100 != 11) : a == 1)
            return one;
        const bool few = a % 10 >= 2 && a % 10 <= 4 && !(a % 100 >= 12 && a % 100 <= 14);
        const QStringList forms = many.split(QLatin1Char('|'));
        return few ? forms.constFirst() : forms.constLast();
    }
    default:
        return n == 1 ? one : many.section(QLatin1Char('|'), -1);
    }
}

inline QString plural(int n, const QString &one, const QString &many)
{
    return QLocale().toString(n) + QLatin1Char(' ') + pluralForm(n, one, many); // 0.90.1: digits grouped (UR-18)
}

// Visual + semantic format for a note run.
QTextCharFormat noteFormat(NoteKind kind, const QString &id = QString(), const QString &noteText = QString(),
                           const QColor &color = QColor());
// Update only the display properties of a format that carries a note.
void applyNoteDisplay(QTextCharFormat &cf);
// Imported texts (File > Options > Sharing > Import, 0.21): comments become inline
// annotations (commentsInline) and linked footnotes inline footnotes (footnotesInline),
// or the other way round: an inline footnote (and, when annotationsAreComments, an
// inline annotation) becomes a linked one on the word before it. Returns how many
// notes were changed.
int convertNotes(QTextDocument *doc, bool commentsInline, bool footnotesInline, bool annotationsAreComments);
QTextCharFormat highlightFormat(const QColor &color = QColor());
QTextCharFormat linkFormat(const QString &href);
// 0.64.2: a picture's format on characters other than the object character (U+FFFC) is not a picture;
// such text (left with a picture's format after a paste) is written as text, with this format
QTextCharFormat withoutPicture(const QTextCharFormat &cf);
// a fragment's text split where pictures are: (text, format) pieces, a picture as one U+FFFC each
QList<QPair<QString, QTextCharFormat>> pictureSafePieces(const QString &text, const QTextCharFormat &cf);
QTextCharFormat revisionFormat(int level);
QColor revisionColor(int level);
// the five colors of the revisions: the standard ones, and those of File > Options > Editing (0.21)
QList<QColor> standardRevisionColors();
void setRevisionColors(const QList<QColor> &colors); // not five colors: back to the standard

// Remove every note / highlight / link property from a format.
void clearNoteProps(QTextCharFormat &cf);

struct Counts {
    int words = 0;
    int chars = 0;          // including spaces
    int charsNoSpaces = 0;
    int paragraphs = 0;
};

Counts count(const QString &plain);
int countWords(const QString &plain);

// Plain text of a document. Inline annotations are excluded unless asked.
QString plainText(const QTextDocument *doc, bool includeAnnotations = false);
Counts countDocument(const QTextDocument *doc);

// First few lines of text for index cards / outliner (no titles given).
QString previewText(const QString &plain, int maxChars = 300);

// A title from the first line of a text that has words (0.40.3): without inline annotations,
// inline footnotes and pictures, spaces simplified, at most maxChars, cut at a word
// boundary. Empty when the text has no words.
QString firstLineTitle(const QTextDocument *doc, int maxChars = 60);
// A document title that means "no name": empty or "Untitled".
bool isUntitled(const QString &title);

// Placeholder-safe file name from a title.
QString safeFileName(const QString &title);

// UTF-8 text of an XML file without the characters XML 1.0 does not allow (control
// characters other than tab, line feed and carriage return; U+FFFE, U+FFFF). A vertical
// tab or form feed (a line or page break pasted from a word processor) becomes a space,
// the others go. `removed` counts them. (0.20: such a character in a title made the
// project file unreadable.)
QByteArray xmlSafeBytes(const QByteArray &utf8, int *removed = nullptr);

} // namespace text
} // namespace qb
