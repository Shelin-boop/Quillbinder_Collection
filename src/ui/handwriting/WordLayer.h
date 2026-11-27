#pragma once
// 0.99.0 (handwriting, item 112, part 2): what is done with the line model's output. No ONNX Runtime and no widgets here:
// frames in, words out, so the checks can feed it frames of their own.
//
//   frames (T steps of 8 px, log-probabilities over the codec's labels)
//     -> greedy CTC: the characters as read (blank 0, repeats collapsed), NFC after decoding
//     -> the word layer: for each word, the listed words close to it are scored by the frames (CTC forward over the word's
//        frames, softened by 1.5) plus 0.5 x the word's Zipf frequency; the raw reading competes as an unknown word (Zipf 1)
//
// The details are in docs/dev-notes/ui.md, "Handwriting recognizer (0.99.0)".
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

namespace qb {
namespace handwriting {

// The labels of the line model: the codec JSON of the model's folder ({"codec": {"<text>": [label]}, ...}).
struct Codec {
    int blank = 0;
    int classes = 0;                          // the model's number of outputs (1623)
    QHash<int, QString> text;                 // label -> text (a letter, a digit, a combining mark, a space ...)
    QHash<QString, int> label;                // text -> label
    QVector<int> marks;                       // the labels of the combining marks (all of them count as one group in the scoring)
    int space = -1;
    bool valid() const { return classes > 1 && !text.isEmpty(); }
    static Codec fromJson(const QByteArray &json, QString *error = nullptr);
};

// log-probabilities per step: logp[t * classes + c]
struct Frames {
    int steps = 0;
    int classes = 0;
    QVector<float> logp;
    float at(int t, int c) const { return logp[t * classes + c]; }
    // the model's output as it comes, (1, classes, 1, T): data[c * T + t]; log-softmax over the classes per step
    static Frames fromLogits(const float *data, int classes, int steps);
};

// one character the greedy path emitted (a letter, a space, a combining mark ...) and the step it came at
struct Emission {
    QString text;
    int frame = 0;
};
QVector<Emission> greedyEmissions(const Frames &frames, const Codec &codec);

// A word list: a gzip'd TSV, "word<TAB>Zipf" per line (models-5's wordlist-<language>.tsv.gz)
struct ListedWord {
    QString word;
    QString folded;     // lower case, without diacritics (the compare form)
    float zipf = 0;
};
class WordList
{
public:
    bool loadGzip(const QString &path, QString *error = nullptr);
    bool loadTsv(const QByteArray &tsv);               // for the checks
    int size() const { return int(m_words.size()); }
    const QVector<ListedWord> &words() const { return m_words; }
    qint64 bytes() const;                              // an estimate of the memory
    const ListedWord *find(const QString &foldedLower, const QString &exactLower) const;
private:
    QVector<ListedWord> m_words;
};

// lower case without diacritics (NFD without the combining marks; ss for the sharp s)
QString foldWord(const QString &word);
// the number of edits between two folded words, or max + 1 when it is more than max
int boundedEditDistance(const QString &a, const QString &b, int max);
// how far a reading of this length may be from a listed word: 1 to 4 edits by length
int allowedDistance(int length);

struct LayerWord {
    QString text;
    QStringList alternatives;     // the next candidates, the raw reading last (when it differs); the chosen text is not repeated
    int frameFrom = 0;            // the steps the word covers
    int frameTo = 0;
    double confidence = 1.0;
    bool listed = false;          // a word of the list (not the raw reading)
};
struct LayerResult {
    QList<LayerWord> words;
    double total = 0;             // the sum of the words' scores: what two languages are compared by
};

// The characters as read, without the word layer.
LayerResult readPlain(const Frames &frames, const Codec &codec);
// The word layer: `list` may be empty (then the same as readPlain). Constants of the method (kSoften ...) in WordLayer.cpp.
LayerResult readWithList(const Frames &frames, const Codec &codec, const WordList &list);
// Each line read with every list given (the main language first, then the fallback); the better total is kept, the first wins a
// tie. *chosen: the index of that list, -1 when there is none (the characters as read).
LayerResult readBest(const Frames &frames, const Codec &codec, const QList<const WordList *> &lists, int *chosen = nullptr);
// The natural log of the probability of a word over the steps [from, to] (CTC forward, all combining marks one group, upper and
// lower case one group); -1e4 when the steps are too few. For the checks.
double ctcLogProb(const Frames &frames, const Codec &codec, const QString &word, int from, int to);

} // namespace handwriting
} // namespace qb
