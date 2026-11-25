#include "Dictionary.h"

#include "Logic.h"

#include <QFile>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>

#include <algorithm>
#include <tuple>
#include <vector>

namespace tp {

// ---------------------------------------------------------------- English (WordNet)

namespace {

struct Glosses {
    QHash<QString, QString> words;   // word -> "n:id,id|v:id"
    QStringList defs;                // by id
    bool loaded = false;
};
Glosses g_glosses;

QString posOf(const QString &tag)
{
    if (tag == QLatin1String("n"))
        return QStringLiteral("noun");
    if (tag == QLatin1String("v"))
        return QStringLiteral("verb");
    if (tag == QLatin1String("a"))
        return QStringLiteral("adjective");
    return QStringLiteral("adverb");
}

const Glosses &glosses()
{
    if (!g_glosses.loaded)
        loadGlosses(QStringLiteral(":/translation-proofreading/en-glosses.tsv"));
    return g_glosses;
}

} // namespace

bool isWordNetEntry(const QString &word)
{
    return glosses().words.contains(word);
}

void loadGlosses(const QString &file)
{
    g_glosses = Glosses();
    QFile f(file);
    if (f.open(QIODevice::ReadOnly)) {
        std::vector<std::pair<int, QString>> defs;
        for (const QByteArray &line : f.readAll().split('\n')) {
            const int tab = line.indexOf('\t');
            if (line.isEmpty() || line.startsWith('#') || tab <= 0)
                continue;
            if (line.startsWith('=')) {
                bool ok = false;
                const int id = line.mid(1, tab - 1).toInt(&ok, 36);
                if (ok)
                    defs.emplace_back(id, QString::fromUtf8(line.mid(tab + 1)));
            } else {
                g_glosses.words.insert(QString::fromUtf8(line.left(tab)), QString::fromUtf8(line.mid(tab + 1)));
            }
        }
        int top = -1;
        for (const auto &d : defs)
            top = std::max(top, d.first);
        g_glosses.defs = QStringList(top + 1);
        for (const auto &d : defs)
            g_glosses.defs[d.first] = d.second;
    }
    g_glosses.loaded = true;
}

QList<EnglishClass> englishMeanings(const QString &word, QString *base)
{
    const Glosses &g = glosses();
    QList<EnglishClass> out;
    const QStringList bases = baseFormsIn(word, [&g](const QString &b) { return g.words.contains(b); });
    if (base)
        *base = bases.value(0);
    for (const QString &b : bases) {
        for (const QString &group : g.words.value(b).split(QLatin1Char('|'), Qt::SkipEmptyParts)) {
            const int colon = group.indexOf(QLatin1Char(':'));
            if (colon < 0)
                continue;
            const QString pos = posOf(group.left(colon));
            EnglishClass *cls = nullptr;
            for (EnglishClass &c : out)
                if (c.pos == pos)
                    cls = &c;
            if (!cls) {
                out << EnglishClass{pos, {}};
                cls = &out.last();
            }
            for (const QString &id : group.mid(colon + 1).split(QLatin1Char(','), Qt::SkipEmptyParts)) {
                const QString d = g.defs.value(id.toInt(nullptr, 36));
                if (!d.isEmpty() && !cls->meanings.contains(d))
                    cls->meanings << d;
            }
        }
    }
    return out;
}

// ---------------------------------------------------------------- German

namespace {

struct GermanDict {
    QByteArray data;
    std::vector<int> lines;   // start of each line (after the header), sorted by key
    bool loaded = false;
};
GermanDict g_de;
QMutex g_deMutex;

QByteArray keyOf(const QByteArray &line)
{
    return line.left(line.indexOf('\t'));
}

QByteArray lineAt(const GermanDict &d, size_t i)
{
    const int start = d.lines[i];
    const int end = d.data.indexOf('\n', start);
    return d.data.mid(start, end < 0 ? -1 : end - start);
}

QList<QByteArray> linesFor(const GermanDict &d, const QByteArray &key)
{
    QList<QByteArray> out;
    size_t lo = 0, hi = d.lines.size();
    while (lo < hi) {
        const size_t mid = (lo + hi) / 2;
        if (keyOf(lineAt(d, mid)) < key)
            lo = mid + 1;
        else
            hi = mid;
    }
    for (size_t i = lo; i < d.lines.size(); ++i) {
        const QByteArray l = lineAt(d, i);
        if (keyOf(l) != key)
            break;
        out << l;
    }
    return out;
}

QString text(const QByteArray &b)
{
    return QString::fromUtf8(b);
}

} // namespace

bool loadGermanDictionary(const QString &file, QString *error)
{
    QMutexLocker lock(&g_deMutex);
    g_de = GermanDict();
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("cannot read %1").arg(file);
        return false;
    }
    QByteArray raw = f.readAll();
    if (!raw.startsWith('#')) {
        raw = qUncompress(raw);   // qCompress: 4 length bytes and a zlib stream
        if (raw.isEmpty()) {
            if (error)
                *error = QStringLiteral("the dictionary file is damaged");
            return false;
        }
    }
    if (!raw.startsWith("#qb-de-dictionary 1")) {
        if (error)
            *error = QStringLiteral("not a dictionary file of this version");
        return false;
    }
    g_de.data = raw;
    int pos = 0;
    while (pos < raw.size()) {
        const int end = raw.indexOf('\n', pos);
        if (raw.at(pos) != '#' && end != pos)
            g_de.lines.push_back(pos);
        if (end < 0)
            break;
        pos = end + 1;
    }
    // sorted by key (the file is made so; a file that is not is sorted here)
    if (!std::is_sorted(g_de.lines.begin(), g_de.lines.end(), [](int a, int b) {
            return QByteArray::fromRawData(g_de.data.constData() + a, g_de.data.indexOf('\t', a) - a)
                   < QByteArray::fromRawData(g_de.data.constData() + b, g_de.data.indexOf('\t', b) - b);
        }))
        std::stable_sort(g_de.lines.begin(), g_de.lines.end(), [](int a, int b) {
            return QByteArray::fromRawData(g_de.data.constData() + a, g_de.data.indexOf('\t', a) - a)
                   < QByteArray::fromRawData(g_de.data.constData() + b, g_de.data.indexOf('\t', b) - b);
        });
    g_de.loaded = true;
    return true;
}

void unloadGermanDictionary()
{
    QMutexLocker lock(&g_deMutex);
    g_de = GermanDict();
}

bool germanDictionaryLoaded()
{
    QMutexLocker lock(&g_deMutex);
    return g_de.loaded;
}

QList<GermanEntry> lookupGerman(const QString &word)
{
    QMutexLocker lock(&g_deMutex);
    QList<GermanEntry> out;
    if (!g_de.loaded)
        return out;
    const QString w = word.trimmed();
    auto addLemma = [&](const QByteArray &line, bool viaForm) {
        const QList<QByteArray> f = line.split('\t');
        if (f.size() < 6 || f.at(1) != "L")
            return;
        const QString head = text(f.at(2)), pos = text(f.at(3));
        for (const GermanEntry &e : out)
            if (e.headword == head && e.pos == pos)
                return;
        GermanEntry e;
        e.headword = head;
        e.pos = pos;
        e.gender = text(f.at(4));
        e.viaForm = viaForm;
        for (const QByteArray &m : f.at(5).split('|')) {
            if (m.isEmpty())
                continue;
            GermanMeaning gm;
            const int caret = m.indexOf('^');
            const QByteArray words = caret < 0 ? m : m.left(caret);
            gm.definition = caret < 0 ? QString() : text(m.mid(caret + 1));
            for (const QByteArray &x : words.split(';'))
                if (!x.trimmed().isEmpty())
                    gm.english << text(x.trimmed());
            e.meanings << gm;
        }
        out << e;
    };
    const QList<QByteArray> hits = linesFor(g_de, w.toLower().toUtf8());
    for (const QByteArray &l : hits)
        addLemma(l, false);
    for (const QByteArray &l : hits) {
        const QList<QByteArray> f = l.split('\t');
        if (f.size() < 4 || f.at(1) != "F")
            continue;
        // the headword's own lines: the one with that headword and class
        for (const QByteArray &hl : linesFor(g_de, text(f.at(2)).toLower().toUtf8())) {
            const QList<QByteArray> hf = hl.split('\t');
            if (hf.size() >= 6 && hf.at(1) == "L" && hf.at(2) == f.at(2) && hf.at(3) == f.at(3))
                addLemma(hl, true);
        }
    }
    // the capital letter is a hint (German nouns), not the rule
    const bool capital = !w.isEmpty() && w.at(0).isUpper();
    std::stable_sort(out.begin(), out.end(), [capital](const GermanEntry &a, const GermanEntry &b) {
        const int ra = (a.pos == QLatin1String("noun")) == capital ? 0 : 1;
        const int rb = (b.pos == QLatin1String("noun")) == capital ? 0 : 1;
        return ra < rb;
    });
    return out;
}

bool ensureGermanDictionary(const QString &file, QString *error)
{
    static QString loadedFrom;
    static QMutex m;
    QMutexLocker lock(&m);
    if (germanDictionaryLoaded() && loadedFrom == file)
        return true;
    if (!loadGermanDictionary(file, error))
        return false;
    loadedFrom = file;
    return true;
}

QList<GermanReverse> reverseLookupGerman(const QStringList &english, const QString &pos, int max)
{
    struct Hit {
        QString headword, pos;
        int rank, sense;
    };
    QList<Hit> hits;
    QMutexLocker lock(&g_deMutex);
    if (!g_de.loaded || english.isEmpty())
        return {};
    QHash<QByteArray, int> rankOf;   // the bare lower-case word, its rank
    for (int i = 0; i < english.size(); ++i) {
        QString w = english.at(i).trimmed().toLower();
        if (w.startsWith(QLatin1String("to ")))
            w = w.mid(3);
        if (!w.isEmpty() && !rankOf.contains(w.toUtf8()))
            rankOf.insert(w.toUtf8(), i);
    }
    const QByteArray wantPos = pos.toUtf8();
    for (size_t i = 0; i < g_de.lines.size(); ++i) {
        const QByteArray line = lineAt(g_de, i);
        const QList<QByteArray> f = line.split('\t');
        if (f.size() < 6 || f.at(1) != "L" || (!wantPos.isEmpty() && f.at(3) != wantPos))
            continue;
        const QList<QByteArray> senses = f.at(5).split('|');
        int bestRank = -1, bestSense = 0;
        for (int s = 0; s < senses.size() && bestRank != 0; ++s) {
            const QByteArray &m = senses.at(s);
            const int caret = m.indexOf('^');
            for (const QByteArray &x : (caret < 0 ? m : m.left(caret)).split(';')) {
                QByteArray w = x.trimmed().toLower();
                if (w.startsWith("to "))
                    w = w.mid(3);
                const auto it = rankOf.constFind(w);
                if (it != rankOf.constEnd() && (bestRank < 0 || it.value() < bestRank || (it.value() == bestRank && s < bestSense))) {
                    bestRank = it.value();
                    bestSense = s;
                }
            }
        }
        if (bestRank >= 0)
            hits.push_back({text(f.at(2)), text(f.at(3)), bestRank, bestSense});
    }
    std::stable_sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) {
        return std::tie(a.rank, a.sense, a.headword) < std::tie(b.rank, b.sense, b.headword);
    });
    QList<GermanReverse> out;
    QSet<QString> seen;
    for (const Hit &h : hits) {
        const QString key = h.headword + QLatin1Char('\x1f') + h.pos;
        if (seen.contains(key))
            continue;
        seen.insert(key);
        out << GermanReverse{h.headword, h.pos, h.rank};
        if (out.size() >= max)
            break;
    }
    return out;
}

QString correctGermanCapitals(const QString &text)
{
    if (!germanDictionaryLoaded())
        return text;
    static const QSet<QString> articles = {QStringLiteral("der"), QStringLiteral("die"), QStringLiteral("das"), QStringLiteral("des"),
        QStringLiteral("dem"), QStringLiteral("den"), QStringLiteral("ein"), QStringLiteral("eine"), QStringLiteral("einer"),
        QStringLiteral("eines"), QStringLiteral("einem"), QStringLiteral("einen"), QStringLiteral("im"), QStringLiteral("am"),
        QStringLiteral("zum"), QStringLiteral("zur"), QStringLiteral("vom"), QStringLiteral("beim"), QStringLiteral("ins"),
        QStringLiteral("ans")};
    struct Tok {
        int start, len;
        bool lower;         // starts with a lower-case letter
        bool nounOnly = false, nounToo = false, hyphen = false;
        bool capital = false;   // a capital letter at the end (typed or given)
    };
    QList<Tok> toks;
    for (int i = 0; i < text.size();) {
        if (!text.at(i).isLetter()) {
            ++i;
            continue;
        }
        int j = i;
        while (j < text.size() && text.at(j).isLetter())
            ++j;
        Tok t;
        t.start = i;
        t.len = j - i;
        t.lower = text.at(i).isLower();
        t.capital = text.at(i).isUpper();
        t.hyphen = (i > 0 && text.at(i - 1) == QLatin1Char('-')) || (j < text.size() && text.at(j) == QLatin1Char('-'));
        if (t.lower && t.len > 1 && !t.hyphen) {
            const QList<GermanEntry> entries = lookupGerman(text.mid(i, t.len));
            bool allNouns = !entries.isEmpty();
            for (const GermanEntry &e : entries) {
                const bool noun = (e.pos == QLatin1String("noun") || e.pos == QLatin1String("name")) && !e.headword.isEmpty()
                                  && e.headword.at(0).isUpper();
                allNouns = allNouns && noun;
                t.nounToo = t.nounToo || noun;
            }
            t.nounOnly = allNouns;
            t.capital = allNouns;
        }
        toks << t;
        i = j;
    }
    auto gapIsSpace = [&](int a, int b) {   // between token a and b only blanks
        const int from = toks.at(a).start + toks.at(a).len, to = toks.at(b).start;
        return text.mid(from, to - from).trimmed().isEmpty() && to > from;
    };
    QString out = text;
    for (int k = 0; k < toks.size(); ++k) {
        const Tok &t = toks.at(k);
        bool up = t.lower && t.nounOnly;
        if (t.lower && !t.nounOnly && t.nounToo && k > 0 && gapIsSpace(k - 1, k)
            && articles.contains(text.mid(toks.at(k - 1).start, toks.at(k - 1).len).toLower())) {
            // "der alte Mann": the next word is a noun, so this one is an adjective
            const bool nounNext = k + 1 < toks.size() && gapIsSpace(k, k + 1) && toks.at(k + 1).capital;
            up = !nounNext;
        }
        if (up)
            out[t.start] = out.at(t.start).toUpper();
    }
    return out;
}

// ---------------------------------------------------------------- frames

QString frameFor(const QString &from, const QString &pos, const QString &word, const QString &gender)
{
    const bool de = from == QLatin1String("de");
    if (pos == QLatin1String("verb"))
        return (de ? QStringLiteral("zu ") : QStringLiteral("to ")) + word;
    if (pos == QLatin1String("noun")) {
        if (!de)
            return QStringLiteral("the ") + word;
        const QString art = gender == QLatin1String("m") ? QStringLiteral("der ")
                            : gender == QLatin1String("f") ? QStringLiteral("die ")
                            : gender == QLatin1String("n") ? QStringLiteral("das ")
                                                           : QString();
        return art + word;
    }
    if (pos == QLatin1String("adjective"))
        return (de ? QStringLiteral("Das ist ") : QStringLiteral("That is ")) + word;
    return word;
}

QString unframe(const QString &to, const QString &pos, const QString &answer)
{
    QString a = answer.trimmed();
    while (!a.isEmpty() && QStringLiteral(".!?,;:").contains(a.at(a.size() - 1)))
        a.chop(1);
    a = a.trimmed();
    auto cutStart = [&a](const QStringList &starts) {
        for (const QString &s : starts)
            if (a.size() > s.size() && a.startsWith(s, Qt::CaseInsensitive)) {
                a = a.mid(s.size());
                return;
            }
    };
    const bool en = to == QLatin1String("en");
    if (pos == QLatin1String("adjective")) {
        cutStart(en ? QStringList{QStringLiteral("that is "), QStringLiteral("this is "), QStringLiteral("it is "),
                                  QStringLiteral("it's "), QStringLiteral("that's "), QStringLiteral("is ")}
                    : QStringList{QStringLiteral("das ist "), QStringLiteral("dies ist "), QStringLiteral("es ist "),
                                  QStringLiteral("ist ")});
    } else if (pos == QLatin1String("noun") && en) {
        cutStart({QStringLiteral("the "), QStringLiteral("a "), QStringLiteral("an ")});
    } else if (pos == QLatin1String("verb")) {
        if (en) {
            if (!a.isEmpty() && !a.contains(QLatin1Char(' ')) && !a.startsWith(QLatin1String("to "), Qt::CaseInsensitive))
                a = QStringLiteral("to ") + a;
        } else {
            cutStart({QStringLiteral("zu ")});
        }
    }
    // a sentence-initial capital of an English answer is only the frame's: the word is lower case unless it was a name
    const bool plain = pos == QLatin1String("verb") || pos == QLatin1String("adjective") || pos == QLatin1String("adverb");
    if (en && plain && a.size() > 1 && a.at(0).isUpper() && a.at(1).isLower())
        a[0] = a.at(0).toLower();
    return a.trimmed();
}

QString bareVerb(const QString &englishVerb)
{
    return englishVerb.startsWith(QLatin1String("to ")) ? englishVerb.mid(3) : englishVerb;
}

} // namespace tp
