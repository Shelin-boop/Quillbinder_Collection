#pragma once
// Paragraph styles and the on-screen formatting of documents.
//
// Documents stored as Markdown carry only *semantic* formatting (bold,
// headings, notes ...). How the text looks on screen (font, indent, line
// spacing) comes from the project's Formatting settings and is applied here.

#include "core/ProjectTypes.h"
#include "text/Styles.h"

#include <QFont>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>

class QTextDocument;

namespace qb {

namespace display {

QString resolvedFontFamily(const Formatting &f);
QFont baseFont(const Formatting &f, double zoom = 1.0);

// Block format of a normal body paragraph.
QTextBlockFormat normalBlockFormat(const Formatting &f);
// What goes between two texts put together (File > Options > General > Separators,
// 0.21): "return" a new paragraph, "empty" an empty line between, "page" a new
// paragraph on a new page, "none" nothing (the texts run on), "custom" the given
// text as a paragraph of its own. The cursor is at the end of the first text and
// is left where the second text goes.
void insertSeparator(QTextCursor &c, const QString &kind, const QString &custom, const Formatting &f);

// Block / char formats for a style (style may be empty = Normal).
QTextBlockFormat blockFormatFor(const Formatting &f, const QString &style, const QTextBlockFormat &base = {});
QTextCharFormat charFormatFor(const Formatting &f, const QString &style);

// How applyParagraphStyle treats the text inside the paragraphs:
//  KeepOwn     fonts, sizes, colors set by hand stay on top of the new style (Paste Formatting, scripts);
//  Paragraphs  the chosen style takes over (0.60.1): those give way to the style; bold, italic, underline,
//              strikethrough, highlights and character styles stay; a selection that ends right at the
//              start of a paragraph leaves that paragraph alone.
enum class StyleOver { KeepOwn, Paragraphs };

// Apply a paragraph style to every block touched by the cursor.
void applyParagraphStyle(QTextCursor cursor, const QString &style, const Formatting &f,
                         StyleOver over = StyleOver::KeepOwn);

// 0.93.0: notes (a document's notes and the project notes) start as plain text: no font, size, indent, spacing or style of
// the project's text formatting; the neutral look is the interface's font at the notes' zoom. A notes document that is
// plain has this property (set when it is new or empty, or when its file carries the mark; written to the file again).
extern const char *const kPlainNotesProp;
QFont plainNotesFont(double zoom);
// the default font, the empty paragraph and (Markdown) the normal paragraphs of a plain notes document
void applyPlainNotes(QTextDocument *doc, double zoom, bool flattenParagraphs);

// Set the default font, margins etc. of a document.
void prepareDocument(QTextDocument *doc, const Formatting &f, double zoom);

// Re-apply the display formatting of all blocks according to their style
// property. Used after loading Markdown and when the project formatting
// changes. Only touches block formats and style char properties.
void normalizeDocument(QTextDocument *doc, const Formatting &f);

// Strip char formats that are not representable (fonts, sizes, colours);
// used for pasted text in Markdown projects.
QTextCharFormat semanticCharFormat(const QTextCharFormat &in);

} // namespace display
} // namespace qb
