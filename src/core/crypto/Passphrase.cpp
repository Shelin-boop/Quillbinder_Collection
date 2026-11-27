// Passphrases of the built-in encryption (0.99.0): see Passphrase.h.
#include "core/crypto/Passphrase.h"

#include <QCoreApplication>
#include <QFile>
#include <QRandomGenerator>
#include <QSet>
#include <cmath>
#include <vector>

namespace qb {
namespace crypto {
namespace passphrase {

namespace {

QString say(const char *text) { return QCoreApplication::translate("qb::crypto", text); }

struct List {
    QStringList words;
    QSet<QString> set;
    int longest = 0;
};

const List &list()
{
    static const List l = [] {
        List r;
        // (qt_add_resources in CMakeLists.txt; lines "<five dice digits><tab><word>")
        QFile f(QStringLiteral(":/qb-crypto/eff_large_wordlist.txt"));
        if (f.open(QIODevice::ReadOnly)) {
            const QList<QByteArray> lines = f.readAll().split('\n');
            for (const QByteArray &line : lines) {
                const qsizetype tab = line.indexOf('\t');
                const QString w = QString::fromUtf8(tab >= 0 ? line.mid(tab + 1) : line).trimmed();
                if (!w.isEmpty()) {
                    r.words.append(w);
                    r.set.insert(w);
                    r.longest = qMax(r.longest, int(w.size()));
                }
            }
        }
        return r;
    }();
    return l;
}

// words and passwords that are tried first by anyone guessing (a short list; each counts as about 5 bits)
const QSet<QString> &common()
{
    static const QSet<QString> s = {
        QStringLiteral("password"), QStringLiteral("passwort"), QStringLiteral("passphrase"), QStringLiteral("passwd"),
        QStringLiteral("quillbinder"), QStringLiteral("qwerty"), QStringLiteral("qwertz"), QStringLiteral("azerty"),
        QStringLiteral("letmein"), QStringLiteral("iloveyou"), QStringLiteral("welcome"), QStringLiteral("admin"),
        QStringLiteral("monkey"), QStringLiteral("dragon"), QStringLiteral("sunshine"), QStringLiteral("princess"),
        QStringLiteral("football"), QStringLiteral("baseball"), QStringLiteral("master"), QStringLiteral("shadow"),
        QStringLiteral("secret"), QStringLiteral("geheim"), QStringLiteral("hallo"), QStringLiteral("hello"),
        QStringLiteral("login"), QStringLiteral("default"), QStringLiteral("changeme"), QStringLiteral("trustno"),
        QStringLiteral("summer"), QStringLiteral("sommer"), QStringLiteral("winter"), QStringLiteral("spring"),
        QStringLiteral("autumn"), QStringLiteral("freedom"), QStringLiteral("whatever"), QStringLiteral("superman"),
        QStringLiteral("batman"), QStringLiteral("starwars"), QStringLiteral("pokemon"), QStringLiteral("computer"),
        QStringLiteral("internet"), QStringLiteral("love"), QStringLiteral("liebe"), QStringLiteral("test"),
        QStringLiteral("writer"), QStringLiteral("novel"), QStringLiteral("book"), QStringLiteral("chapter"),
        QStringLiteral("manuscript"), QStringLiteral("story")};
    return s;
}

// rows of common keyboards (QWERTY, QWERTZ, AZERTY) and the digit row
const QStringList &rows()
{
    static const QStringList r = {QStringLiteral("1234567890"), QStringLiteral("qwertyuiop"), QStringLiteral("qwertzuiop"),
                                  QStringLiteral("azertyuiop"), QStringLiteral("asdfghjkl"), QStringLiteral("qsdfghjklm"),
                                  QStringLiteral("zxcvbnm"), QStringLiteral("yxcvbnm"), QStringLiteral("wxcvbn")};
    return r;
}

bool keyboardNeighbors(QChar a, QChar b)
{
    for (const QString &row : rows()) {
        const qsizetype i = row.indexOf(a);
        if (i >= 0 && ((i + 1 < row.size() && row.at(i + 1) == b) || (i > 0 && row.at(i - 1) == b)))
            return true;
    }
    return false;
}

bool isSeparator(QChar c)
{
    return c == QLatin1Char(' ') || c == QLatin1Char('-') || c == QLatin1Char('_') || c == QLatin1Char('.')
        || c == QLatin1Char(',') || c == QLatin1Char('/') || c == QLatin1Char('+');
}

} // namespace

const QStringList &words() { return list().words; }

QString generate(int count)
{
    const QStringList &w = words();
    if (w.isEmpty() || count <= 0)
        return QString();
    // an unbiased choice: numbers above the last whole multiple of the list's length are drawn again
    const quint32 n = quint32(w.size());
    const quint32 limit = quint32((quint64(1) << 32) / n * n);
    QStringList chosen;
    for (int i = 0; i < count; ++i) {
        quint32 r;
        do
            r = QRandomGenerator::system()->generate();
        while (limit != 0 && r >= limit);
        chosen.append(w.at(int(r % n)));
    }
    return chosen.join(QLatin1Char(' '));
}

Strength check(const QString &passphrase)
{
    Strength s;
    const QString text = passphrase.normalized(QString::NormalizationForm_C).left(256);
    const QList<uint> code = text.toUcs4();
    const QList<uint> low = text.toLower().toUcs4();
    const int n = int(qMin(low.size(), code.size()));
    // one QChar per character (a character outside the Basic Multilingual Plane, an emoji, stands as U+FFFD), so that
    // the places below are the same in both strings
    QString lower, original;
    for (int i = 0; i < n; ++i) {
        lower += QChar(char16_t(low.at(i) <= 0xffff ? low.at(i) : 0xfffd));
        original += QChar(char16_t(code.at(i) <= 0xffff ? code.at(i) : 0xfffd));
    }
    // the kinds of characters that occur
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSign = false, hasOther = false;
    for (uint c : code) {
        if (c >= 'a' && c <= 'z')
            hasLower = true;
        else if (c >= 'A' && c <= 'Z')
            hasUpper = true;
        else if (c >= '0' && c <= '9')
            hasDigit = true;
        else if (c < 128)
            hasSign = true;
        else
            hasOther = true;
    }
    const double pool = (hasLower ? 26 : 0) + (hasUpper ? 26 : 0) + (hasDigit ? 10 : 0) + (hasSign ? 33 : 0) + (hasOther ? 100 : 0);
    const double perChar = pool > 1 ? std::log2(pool) : 1;
    const double perWord = std::log2(7776.0);
    auto at = [&](int i) { return lower.at(i); };
    // the cost of one character in its place
    auto charCost = [&](int i) -> double {
        const uint c = low.at(i);
        if (i > 0 && c == low.at(i - 1))
            return 1; // repeated
        const QChar qc = at(i);
        if (isSeparator(qc)) {
            for (int j = 0; j < i; ++j)
                if (low.at(j) == c)
                    return 0.5; // the same separator again
            return 2.5;
        }
        if (i > 0 && qc.isLetterOrNumber() && at(i - 1).isLetterOrNumber()) {
            const int step = int(c) - int(low.at(i - 1));
            if ((step == 1 || step == -1) && (i < 2 || int(low.at(i - 1)) - int(low.at(i - 2)) == step))
                return 1.5; // a run: abc, 321
            if (keyboardNeighbors(at(i - 1), qc))
                return 2; // along a keyboard row
        }
        return perChar;
    };
    // the cheapest reading of the whole text: character by character, or in pieces that are words of the list, common
    // words, years or pieces that came before
    const int longest = qMax(list().longest, 12);
    std::vector<double> best(size_t(n) + 1, 0);
    for (int i = 1; i <= n; ++i) {
        double b = best[size_t(i) - 1] + charCost(i - 1);
        for (int len = 3; len <= qMin(i, longest); ++len) {
            const QString piece = lower.mid(i - len, len);
            const double caseBit = original.mid(i - len, len) != piece ? 1 : 0;
            if (list().set.contains(piece))
                b = qMin(b, best[size_t(i - len)] + perWord + caseBit);
            if (common().contains(piece))
                b = qMin(b, best[size_t(i - len)] + 5 + caseBit);
            if (len == 4) {
                bool digits = true;
                for (QChar c : piece)
                    digits = digits && c.isDigit();
                if (digits && piece.toInt() >= 1900 && piece.toInt() <= 2099)
                    b = qMin(b, best[size_t(i - len)] + 7);
            }
            if (lower.left(i - len).contains(piece))
                b = qMin(b, best[size_t(i - len)] + 3); // the same piece once more
        }
        best[size_t(i)] = b;
    }
    s.bits = best[size_t(n)];
    if (n < kMinimumLength) {
        s.reason = say("This passphrase is too short: it needs at least %1 characters (or take the six words Quillbinder suggests).")
                       .arg(kMinimumLength);
    } else if (s.bits < kMinimumBits) {
        s.reason = say("This passphrase is too easy to guess: it is made of few or common words, repeats, runs or keyboard "
                       "rows. Add more words, or take the six words Quillbinder suggests.");
    } else {
        s.ok = true;
    }
    return s;
}

} // namespace passphrase
} // namespace crypto
} // namespace qb
