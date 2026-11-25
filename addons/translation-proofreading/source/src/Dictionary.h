#pragma once
// What a word means (1.3), from two dictionaries the add-on reads; Qt Core only (the checks compile it alone):
//  - English words: Princeton WordNet 3.0's definitions (resources/en-glosses.tsv, built into the add-on, the same
//    source and license as the thesaurus);
//  - German words: an extract of the Wiktionary (word classes, meanings, English words per meaning), a separate
//    download (File > Options), read from one file (format below).
// And the frames the small translation models translate a lone word in ("zu fluchen" for a verb, "das Fluchen" for a
// noun), cut off afterwards.
#include <QList>
#include <QString>
#include <QStringList>

namespace tp {

// ---------------------------------------------------------------- English (WordNet)

struct EnglishClass {
    QString pos;            // "noun", "verb", "adjective", "adverb"
    QStringList meanings;   // the definitions, the most frequent sense first
};
void loadGlosses(const QString &file);   // tests load another file
// The meanings of an English word (its base form: "walked" gives "walk"), by part of speech; empty: WordNet has none.
// 1.4: the word (as it is, lower case) is a lemma of WordNet itself: "number" is one, "numb" too, "river" too (the thesaurus
// lists only words with synonyms, this lists every word WordNet has)
bool isWordNetEntry(const QString &word);
QList<EnglishClass> englishMeanings(const QString &word, QString *base = nullptr);

// ---------------------------------------------------------------- German (Wiktionary extract)

// The file (made by tools/make-de-dictionary.py, kept by the download "wiktionary-de"): UTF-8 text, sorted by the first
// field (bytes), optionally compressed with qCompress (then it starts with the 4 length bytes, as qCompress writes
// them). Its first line is "#qb-de-dictionary 1". Lines (fields separated by tabs):
//     key  L  headword  class  gender  meanings      a word with its meanings; the key is the headword in lower case;
//                                                    gender "m", "f", "n" or empty (nouns); meanings are separated by
//                                                    "|", each "english words separated by ;" then "^" and the definition
//     key  F  headword  class                        an inflected form ("fluchte") that leads to a headword and class
struct GermanMeaning {
    QStringList english;    // the English words of the meaning ("to curse", "to swear"); empty: none known
    QString definition;     // the dictionary's own words for it (English)
};
struct GermanEntry {
    QString headword;       // the base form ("fluchen")
    QString pos;            // the word class: "noun", "verb", "adjective", ...
    QString gender;         // "m", "f", "n" or empty
    QList<GermanMeaning> meanings;
    bool viaForm = false;   // the word asked for is an inflected form of the headword
};
bool loadGermanDictionary(const QString &file, QString *error = nullptr);
void unloadGermanDictionary();
bool germanDictionaryLoaded();
// Every entry for a word, any case ("Fluchen" is also the verb); a capital letter is a hint: nouns first, a lower-case
// word has them last. Entries are not repeated.
QList<GermanEntry> lookupGerman(const QString &word);

// 1.4: reads the file when no dictionary or another file is loaded (the work of the panel's threads and of the main thread
// share it); false with *error when it cannot be read.
bool ensureGermanDictionary(const QString &file, QString *error = nullptr);

// 1.4: the German words whose meanings name one of the English words (a reverse lookup; the German similar words of an
// English word: the word itself, then its WordNet synonyms). `english` is ordered: rank 0 is the first (the word itself).
// "to " before a verb's meaning is ignored, the case too. `pos` keeps one class ("noun" ...; empty: all). Sorted by rank, then
// by the position of the meaning in the entry, then alphabetically; at most `max` headwords. Needs the dictionary loaded.
struct GermanReverse {
    QString headword, pos;
    int rank = 0;
};
QList<GermanReverse> reverseLookupGerman(const QStringList &english, const QString &pos, int max = 20);

// 1.4: the capital letters of German nouns put right in a copy of the text that goes to the translation model (OPUS-MT
// learned from German with capitals; a text in lower case is translated in pieces). A word the dictionary knows only as a
// noun gets its capital; a word known both ways ("essen") keeps its form as typed, except right after an article or a
// preposition with article (der, die, das, des, dem, den, ein, eine, einer, eines, einem, einen, im, am, zum, zur, vom,
// beim, ins, ans), where it gets the capital unless the next word is a noun itself (then it is an adjective: "der alte
// Mann"). Needs the dictionary loaded; without it the text is returned as it is.
QString correctGermanCapitals(const QString &text);

// ---------------------------------------------------------------- frames for a lone word

// The frame a word is translated in, by word class; `from` is "de" or "en" (the language of the word).
//   de: verb "zu fluchen", noun "das Fluchen" (with the article its gender gives; none when unknown), adjective
//       "Das ist schön", other classes the word itself
//   en: verb "to curse", noun "the curse", adjective "That is big", other classes the word itself
QString frameFor(const QString &from, const QString &pos, const QString &word, const QString &gender = QString());
// The word out of the model's answer to a frame: the frame's words and the final punctuation cut off. `to` is the
// language of the answer. Verbs keep "to " in English ("to curse") and lose "zu " in German ("fluchen"); a German noun
// keeps its article (it tells the gender), an English one loses it.
QString unframe(const QString &to, const QString &pos, const QString &answer);

// Strips a verb's "to " ("to curse" gives "curse"), for the similar words and the word lists.
QString bareVerb(const QString &englishVerb);

} // namespace tp
