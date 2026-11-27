#pragma once
// Passphrases of the built-in encryption (0.99.0, to-do item 113): a generator of six random words, and a check that
// refuses a weak passphrase the writer makes up.
//
// The words: the EFF's large word list (7,776 English words, Electronic Frontier Foundation 2016, Creative Commons
// Attribution 3.0; src/core/crypto/words/README.txt). It exists in English only, so the generator gives English words
// in every program language. Six words of it are 77.5 bits (log2 of 7,776 to the sixth power).
#include <QString>
#include <QStringList>

namespace qb {
namespace crypto {
namespace passphrase {

// the word list (empty only when the program's resources are missing)
const QStringList &words();
// count words from the list, chosen with the system's cryptographic random generator, separated by single spaces
QString generate(int count = 6);

struct Strength {
    bool ok = false;
    double bits = 0;  // an estimate of how hard the passphrase is to guess, in bits
    QString reason;   // why it is refused (a sentence for the writer; empty when ok)
};
// a passphrase is taken when it has at least kMinimumLength characters and an estimate of at least kMinimumBits.
// The estimate counts each word of the list (and each common password or word of a short list) as one choice, and
// repeated characters, runs (abc, 321) and keyboard rows (qwert, asdf) as nearly nothing; other characters count by the
// kinds that occur (lower case, upper case, digits, other signs).
constexpr int kMinimumLength = 12;
constexpr double kMinimumBits = 50.0;
Strength check(const QString &passphrase);

} // namespace passphrase
} // namespace crypto
} // namespace qb
