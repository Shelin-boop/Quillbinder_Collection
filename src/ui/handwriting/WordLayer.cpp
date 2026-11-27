#include "ui/handwriting/WordLayer.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <limits>

#include <zlib.h>

namespace qb {
namespace handwriting {

namespace {

// The constants of the method (the earlier prototype's, which raised the words read right on a real page from 32 % to 61 %)
constexpr double kSoften = 1.5;          // the frames' score is divided by this
constexpr double kFrequencyWeight = 0.5; // times the word's Zipf value
constexpr double kUnknownZipf = 1.0;     // the raw reading competes as an unknown word
constexpr double kSplitPenalty = 5.0;    // a word split in two is clearly worse than a word
constexpr double kMissingLabel = -30.0;  // a character the codec does not know
constexpr double kNoPath = -1e4;
constexpr int kMaxCandidates = 300;
constexpr int kAlternatives = 4;

bool isMark(const QChar &c) { return c.category() == QChar::Mark_NonSpacing || c.category() == QChar::Mark_SpacingCombining; }

double lse(double a, double b)
{
    if (a < b)
        std::swap(a, b);
    if (b < -1e29)
        return a;
    return a + std::log1p(std::exp(b - a));
}

// A target character of the scoring: the labels it may be read as (the letter in either case; a combining mark as any mark)
struct Token {
    int a = -1, b = -1;   // up to two labels; -1: none
    bool mark = false;
    QChar key;            // equal keys need a blank between them
};

QVector<Token> tokensOf(const QString &word, const Codec &codec)
{
    QVector<Token> out;
    const QString nfd = word.normalized(QString::NormalizationForm_D);
    for (const QChar c : nfd) {
        Token t;
        if (isMark(c)) {
            t.mark = true;
            t.key = QChar(0x0308);
        } else {
            t.key = c.toLower();
            t.a = codec.label.value(QString(c.toLower()), -1);
            t.b = codec.label.value(QString(c.toUpper()), -1);
            if (t.b == t.a)
                t.b = -1;
        }
        out << t;
    }
    return out;
}

// the score of a token at a step
inline double stepScore(const Frames &f, int t, const Token &tok, double markLp)
{
    if (tok.mark)
        return markLp;
    if (tok.a < 0 && tok.b < 0)
        return kMissingLabel;
    const double a = tok.a >= 0 ? f.at(t, tok.a) : -1e30;
    if (tok.b < 0)
        return a;
    return lse(a, f.at(t, tok.b));
}

double ctcForward(const Frames &f, const Codec &codec, const QVector<Token> &tokens, int from, int to, const QVector<double> &markLp)
{
    from = qMax(0, from);
    to = qMin(f.steps - 1, to);
    const int T = to - from + 1;
    const int L = int(tokens.size());
    if (L == 0 || T <= 0)
        return kNoPath;
    const int S = 2 * L + 1;
    const double neg = -1e30;
    QVector<double> prev(S, neg), cur(S, neg);
    auto stepOf = [&](int t, int s) -> double {
        return (s % 2 == 0) ? double(f.at(t, codec.blank)) : stepScore(f, t, tokens[s / 2], markLp[t]);
    };
    prev[0] = stepOf(from, 0);
    if (S > 1)
        prev[1] = stepOf(from, 1);
    for (int i = 1; i < T; ++i) {
        const int t = from + i;
        for (int s = 0; s < S; ++s) {
            double v = prev[s];
            if (s >= 1)
                v = lse(v, prev[s - 1]);
            if (s >= 3 && s % 2 == 1 && tokens[s / 2].key != tokens[s / 2 - 1].key)
                v = lse(v, prev[s - 2]);
            cur[s] = v < -1e29 ? neg : v + stepOf(t, s);
        }
        std::swap(prev, cur);
    }
    double r = prev[S - 1];
    if (S > 1)
        r = lse(r, prev[S - 2]);
    return r < -1e29 ? kNoPath : qMax(kNoPath, r);
}

QVector<double> markScores(const Frames &f, const Codec &codec)
{
    QVector<double> out(f.steps, kMissingLabel);
    for (int t = 0; t < f.steps; ++t) {
        double v = -1e30;
        for (int m : codec.marks)
            v = lse(v, f.at(t, m));
        out[t] = v < -1e29 ? kMissingLabel : v;
    }
    return out;
}

// the case of a raw reading given to a listed word
QString applyCase(const QString &raw, const QString &word)
{
    if (word.isEmpty())
        return word;
    int letters = 0, upper = 0;
    for (const QChar c : raw) {
        if (c.isLetter()) {
            ++letters;
            if (c.isUpper())
                ++upper;
        }
    }
    if (letters > 1 && upper == letters)
        return word.toUpper();
    if (!raw.isEmpty() && raw.at(0).isUpper())
        return word.at(0).toUpper() + word.mid(1);
    return word;
}

// one word as the greedy path read it: the emissions of the word, split into the core (letters) and what is around it
struct RawWord {
    QVector<Emission> em;
    int first = 0, last = -1;   // the core: emissions [first, last]
    QString prefix, suffix;
    QString core;               // NFC
    bool usable = false;        // the layer may change it
    int frameFrom = 0, frameTo = 0;
};

QString joinEmissions(const QVector<Emission> &em, int from, int to)
{
    QString s;
    for (int i = from; i <= to && i < em.size(); ++i)
        s += em[i].text;
    return s;
}

QList<RawWord> splitWords(const QVector<Emission> &all, const Codec &codec)
{
    QList<RawWord> words;
    RawWord cur;
    auto flush = [&] {
        if (cur.em.isEmpty())
            return;
        RawWord w = cur;
        cur = RawWord();
        int first = -1, last = -1;
        for (int i = 0; i < w.em.size(); ++i) {
            const QString &t = w.em[i].text;
            const bool letter = !t.isEmpty() && (t.at(0).isLetter() || isMark(t.at(0)));
            if (letter) {
                if (first < 0)
                    first = i;
                last = i;
            }
        }
        w.frameFrom = w.em.first().frame;
        w.frameTo = w.em.last().frame;
        if (first < 0) {   // no letter: leave it as read
            w.prefix = joinEmissions(w.em, 0, int(w.em.size()) - 1).normalized(QString::NormalizationForm_C);
            words << w;
            return;
        }
        w.first = first;
        w.last = last;
        w.prefix = joinEmissions(w.em, 0, first - 1).normalized(QString::NormalizationForm_C);
        w.suffix = joinEmissions(w.em, last + 1, int(w.em.size()) - 1).normalized(QString::NormalizationForm_C);
        w.core = joinEmissions(w.em, first, last).normalized(QString::NormalizationForm_C);
        bool digit = false;
        int letters = 0;
        for (const QChar c : w.core) {
            digit = digit || c.isDigit();
            letters += c.isLetter() ? 1 : 0;
        }
        w.usable = !digit && letters >= 2;
        words << w;
    };
    for (const Emission &e : all) {
        if (e.text == QLatin1String(" ") || (codec.space >= 0 && codec.label.value(e.text, -2) == codec.space)) {
            flush();
            continue;
        }
        cur.em << e;
    }
    flush();
    return words;
}

struct Candidate {
    QString word;     // as listed (lower case)
    double zipf = 0;
    int distance = 0;
    double score = 0;
};

// the listed words close to `core` (folded compare), the nearest and most frequent first
QVector<Candidate> candidatesFor(const QString &core, const WordList &list)
{
    QVector<Candidate> out;
    const QString folded = foldWord(core);
    const int len = int(folded.size());
    const int maxd = allowedDistance(len);
    for (const ListedWord &w : list.words()) {
        const int dl = int(w.folded.size()) - len;
        if (dl > maxd || -dl > maxd)
            continue;
        const int d = boundedEditDistance(folded, w.folded, maxd);
        if (d > maxd)
            continue;
        out.append({w.word, w.zipf, d, 0});
    }
    std::sort(out.begin(), out.end(), [](const Candidate &a, const Candidate &b) {
        return a.distance != b.distance ? a.distance < b.distance : a.zipf > b.zipf;
    });
    if (out.size() > kMaxCandidates)
        out.resize(kMaxCandidates);
    return out;
}

struct Scored {
    QString text;
    double score = 0;
    bool listed = false;
};

// All readings of one core with their scores, best first; the raw reading is always there. `listedClose` is set when a listed word
// is within the allowed distance.
QVector<Scored> scoreCore(const QString &core, const Frames &f, const Codec &codec, const WordList &list, int from, int to,
                          const QVector<double> &markLp, bool *listedClose)
{
    QVector<Scored> out;
    const QVector<Candidate> cands = candidatesFor(core, list);
    if (listedClose)
        *listedClose = !cands.isEmpty();
    const QString rawLower = core.toLower();
    bool rawListed = false;
    for (const Candidate &c : cands) {
        if (c.word == rawLower)
            rawListed = true;
        const double s = ctcForward(f, codec, tokensOf(c.word, codec), from, to, markLp) / kSoften + kFrequencyWeight * c.zipf;
        out.append({applyCase(core, c.word), s, true});
    }
    if (!rawListed) {
        const double s = ctcForward(f, codec, tokensOf(core, codec), from, to, markLp) / kSoften + kFrequencyWeight * kUnknownZipf;
        out.append({core, s, false});
    }
    std::stable_sort(out.begin(), out.end(), [](const Scored &a, const Scored &b) { return a.score > b.score; });
    return out;
}

double confidenceOf(const QVector<Scored> &s)
{
    if (s.isEmpty())
        return 1.0;
    double sum = 0;
    const int n = qMin(int(s.size()), 8);
    for (int i = 0; i < n; ++i)
        sum += std::exp(s[i].score - s[0].score);
    return 1.0 / sum;
}

LayerWord plainWord(const RawWord &w)
{
    LayerWord lw;
    lw.text = w.prefix + w.core + w.suffix;
    if (!w.usable && w.core.isEmpty())
        lw.text = joinEmissions(w.em, 0, int(w.em.size()) - 1).normalized(QString::NormalizationForm_C);
    lw.frameFrom = w.frameFrom;
    lw.frameTo = w.frameTo;
    return lw;
}

} // namespace

// ---------------------------------------------------------------- codec, frames

Codec Codec::fromJson(const QByteArray &json, QString *error)
{
    Codec c;
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &pe);
    if (!doc.isObject()) {
        if (error)
            *error = pe.errorString();
        return c;
    }
    const QJsonObject root = doc.object();
    const QJsonObject codec = root.value(QLatin1String("codec")).toObject();
    int top = 0;
    for (auto it = codec.begin(); it != codec.end(); ++it) {
        const QJsonArray labels = it.value().toArray();
        if (labels.size() != 1)
            continue;
        const int l = labels.at(0).toInt();
        c.text.insert(l, it.key());
        c.label.insert(it.key(), l);
        top = qMax(top, l);
        const QString k = it.key();
        if (k.size() == 1 && isMark(k.at(0)))
            c.marks << l;
    }
    c.blank = root.value(QLatin1String("output")).toObject().value(QLatin1String("blank")).toInt(0);
    c.classes = root.value(QLatin1String("output")).toObject().value(QLatin1String("classes")).toInt(top + 1);
    c.space = c.label.value(QStringLiteral(" "), -1);
    if (!c.valid() && error)
        *error = QStringLiteral("the codec is empty");
    return c;
}

Frames Frames::fromLogits(const float *data, int classes, int steps)
{
    Frames f;
    f.steps = steps;
    f.classes = classes;
    f.logp.resize(qint64(steps) * classes);
    for (int t = 0; t < steps; ++t) {
        float mx = -std::numeric_limits<float>::infinity();
        for (int c = 0; c < classes; ++c)
            mx = qMax(mx, data[qint64(c) * steps + t]);
        double sum = 0;
        for (int c = 0; c < classes; ++c)
            sum += std::exp(double(data[qint64(c) * steps + t] - mx));
        const float lz = mx + float(std::log(sum));
        for (int c = 0; c < classes; ++c)
            f.logp[qint64(t) * classes + c] = data[qint64(c) * steps + t] - lz;
    }
    return f;
}

QVector<Emission> greedyEmissions(const Frames &frames, const Codec &codec)
{
    QVector<Emission> out;
    int prev = codec.blank;
    for (int t = 0; t < frames.steps; ++t) {
        int best = 0;
        float bv = frames.at(t, 0);
        for (int c = 1; c < frames.classes; ++c) {
            const float v = frames.at(t, c);
            if (v > bv) {
                bv = v;
                best = c;
            }
        }
        if (best != codec.blank && best != prev) {
            const auto it = codec.text.constFind(best);
            if (it != codec.text.constEnd())
                out.append({it.value(), t});
        }
        prev = best;
    }
    return out;
}

// ---------------------------------------------------------------- word list

QString foldWord(const QString &word)
{
    QString out;
    const QString nfd = word.toLower().normalized(QString::NormalizationForm_D);
    for (const QChar c : nfd) {
        if (isMark(c))
            continue;
        if (c == QChar(0x00DF))
            out += QLatin1String("ss");
        else
            out += c;
    }
    return out;
}

int allowedDistance(int length)
{
    if (length <= 3)
        return 1;
    if (length <= 5)
        return 2;
    if (length <= 8)
        return 3;
    return 4;
}

int boundedEditDistance(const QString &a, const QString &b, int max)
{
    const int n = int(a.size()), m = int(b.size());
    if (qAbs(n - m) > max)
        return max + 1;
    QVector<int> prev(m + 1), cur(m + 1);
    for (int j = 0; j <= m; ++j)
        prev[j] = j;
    for (int i = 1; i <= n; ++i) {
        cur[0] = i;
        int rowMin = cur[0];
        for (int j = 1; j <= m; ++j) {
            const int cost = a.at(i - 1) == b.at(j - 1) ? 0 : 1;
            cur[j] = qMin(qMin(prev[j] + 1, cur[j - 1] + 1), prev[j - 1] + cost);
            rowMin = qMin(rowMin, cur[j]);
        }
        if (rowMin > max)
            return max + 1;
        std::swap(prev, cur);
    }
    return qMin(prev[m], max + 1);
}

bool WordList::loadTsv(const QByteArray &tsv)
{
    m_words.clear();
    int pos = 0;
    while (pos < tsv.size()) {
        int end = tsv.indexOf('\n', pos);
        if (end < 0)
            end = int(tsv.size());
        const QByteArray line = tsv.mid(pos, end - pos);
        pos = end + 1;
        const int tab = line.indexOf('\t');
        if (tab <= 0)
            continue;
        bool ok = false;
        const float z = line.mid(tab + 1).trimmed().toFloat(&ok);
        if (!ok)
            continue;
        ListedWord w;
        w.word = QString::fromUtf8(line.left(tab)).normalized(QString::NormalizationForm_C);
        if (w.word.isEmpty())
            continue;
        w.folded = foldWord(w.word);
        w.zipf = z;
        m_words << w;
    }
    return !m_words.isEmpty();
}

bool WordList::loadGzip(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = f.errorString();
        return false;
    }
    const QByteArray gz = f.readAll();
    z_stream zs{};
    if (inflateInit2(&zs, 15 + 32) != Z_OK) {
        if (error)
            *error = QStringLiteral("zlib");
        return false;
    }
    QByteArray out;
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(gz.constData()));
    zs.avail_in = uInt(gz.size());
    char buf[1 << 16];
    int rc = Z_OK;
    while (rc == Z_OK) {
        zs.next_out = reinterpret_cast<Bytef *>(buf);
        zs.avail_out = sizeof buf;
        rc = inflate(&zs, Z_NO_FLUSH);
        out.append(buf, int(sizeof buf - zs.avail_out));
    }
    inflateEnd(&zs);
    if (rc != Z_STREAM_END) {
        if (error)
            *error = QStringLiteral("the word list is not a complete gzip file");
        return false;
    }
    return loadTsv(out);
}

qint64 WordList::bytes() const
{
    qint64 n = 0;
    for (const ListedWord &w : m_words)
        n += 2 * (w.word.size() + w.folded.size()) + 64;
    return n;
}

const ListedWord *WordList::find(const QString &foldedLower, const QString &exactLower) const
{
    for (const ListedWord &w : m_words)
        if (w.folded == foldedLower && (exactLower.isEmpty() || w.word == exactLower))
            return &w;
    return nullptr;
}

// ---------------------------------------------------------------- reading

LayerResult readPlain(const Frames &frames, const Codec &codec)
{
    LayerResult r;
    for (const RawWord &w : splitWords(greedyEmissions(frames, codec), codec))
        r.words << plainWord(w);
    return r;
}

double ctcLogProb(const Frames &frames, const Codec &codec, const QString &word, int from, int to)
{
    return ctcForward(frames, codec, tokensOf(word, codec), from, to, markScores(frames, codec));
}

LayerResult readWithList(const Frames &frames, const Codec &codec, const WordList &list)
{
    if (list.size() == 0)
        return readPlain(frames, codec);
    const QList<RawWord> raws = splitWords(greedyEmissions(frames, codec), codec);
    const QVector<double> markLp = markScores(frames, codec);
    LayerResult result;
    for (int wi = 0; wi < raws.size(); ++wi) {
        const RawWord &w = raws[wi];
        LayerWord base = plainWord(w);
        if (!w.usable) {
            result.words << base;
            continue;
        }
        // the steps of the word: from the middle of the gap before it to the middle of the gap after it
        const int prevEnd = wi > 0 ? raws[wi - 1].frameTo : w.frameFrom - 8;
        const int nextStart = wi + 1 < raws.size() ? raws[wi + 1].frameFrom : w.frameTo + 8;
        const int from = qMax(0, (prevEnd + w.frameFrom) / 2);
        const int to = qMin(frames.steps - 1, (w.frameTo + nextStart) / 2);
        const int coreFrom = w.em[w.first].frame, coreTo = w.em[w.last].frame;
        const int rangeFrom = qMin(coreFrom, from), rangeTo = qMax(coreTo, to);

        bool close = false;
        QVector<Scored> s = scoreCore(w.core, frames, codec, list, rangeFrom, rangeTo, markLp, &close);
        double bestScore = s.first().score;
        // a word split in two: only when no single word fits better than the raw reading, both parts at least two letters
        if (!s.first().listed) {
            int bestSplit = -1;
            double bestSplitScore = bestScore;
            QVector<Scored> bestA, bestB;
            for (int j = w.first + 1; j <= w.last; ++j) {
                if (w.em[j].text.isEmpty() || isMark(w.em[j].text.at(0)))
                    continue;
                const QString a = joinEmissions(w.em, w.first, j - 1).normalized(QString::NormalizationForm_C);
                const QString b = joinEmissions(w.em, j, w.last).normalized(QString::NormalizationForm_C);
                auto letters = [](const QString &x) {
                    int n = 0;
                    for (const QChar c : x)
                        n += c.isLetter() ? 1 : 0;
                    return n;
                };
                if (letters(a) < 2 || letters(b) < 2)
                    continue;
                const int mid = (w.em[j - 1].frame + w.em[j].frame) / 2;
                bool ca = false, cb = false;
                QVector<Scored> sa = scoreCore(a, frames, codec, list, rangeFrom, mid, markLp, &ca);
                QVector<Scored> sb = scoreCore(b, frames, codec, list, mid + 1, rangeTo, markLp, &cb);
                if (!ca || !cb || !sa.first().listed || !sb.first().listed)
                    continue;
                const double total = sa.first().score + sb.first().score - kSplitPenalty;
                if (total > bestSplitScore) {
                    bestSplitScore = total;
                    bestSplit = j;
                    bestA = sa;
                    bestB = sb;
                }
            }
            if (bestSplit >= 0) {
                const int mid = (w.em[bestSplit - 1].frame + w.em[bestSplit].frame) / 2;
                LayerWord a, b;
                a.text = w.prefix + bestA.first().text;
                a.frameFrom = w.frameFrom;
                a.frameTo = mid;
                a.listed = true;
                a.confidence = confidenceOf(bestA);
                b.text = bestB.first().text + w.suffix;
                b.frameFrom = mid + 1;
                b.frameTo = w.frameTo;
                b.listed = true;
                b.confidence = confidenceOf(bestB);
                result.words << a << b;
                result.total += bestSplitScore;
                continue;
            }
        }
        LayerWord lw;
        lw.text = w.prefix + s.first().text + w.suffix;
        lw.frameFrom = w.frameFrom;
        lw.frameTo = w.frameTo;
        lw.listed = s.first().listed;
        lw.confidence = confidenceOf(s);
        for (int i = 1; i < s.size() && lw.alternatives.size() < kAlternatives; ++i) {
            const QString alt = w.prefix + s[i].text + w.suffix;
            if (alt != lw.text && !lw.alternatives.contains(alt) && s[i].listed)
                lw.alternatives << alt;
        }
        const QString raw = w.prefix + w.core + w.suffix;
        if (raw != lw.text && !lw.alternatives.contains(raw))
            lw.alternatives << raw;
        result.words << lw;
        result.total += bestScore;
    }
    return result;
}

LayerResult readBest(const Frames &frames, const Codec &codec, const QList<const WordList *> &lists, int *chosen)
{
    LayerResult best;
    int at = -1;
    for (int i = 0; i < lists.size(); ++i) {
        if (!lists[i] || lists[i]->size() == 0)
            continue;
        const LayerResult lr = readWithList(frames, codec, *lists[i]);
        if (at < 0 || lr.total > best.total) {
            best = lr;
            at = i;
        }
    }
    if (at < 0)
        best = readPlain(frames, codec);
    if (chosen)
        *chosen = at;
    return best;
}

} // namespace handwriting
} // namespace qb
