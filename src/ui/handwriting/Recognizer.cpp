#include "ui/handwriting/Recognizer.h"

#include <QHash>
#include <QPointer>

namespace qb {
namespace handwriting {

QString Result::text() const
{
    QStringList parts;
    for (const Word &w : words)
        parts << w.text;
    return parts.join(QLatin1Char(' '));
}

static QPointer<Recognizer> &currentPointer()
{
    static QPointer<Recognizer> p;
    return p;
}

Recognizer *Recognizer::current() { return currentPointer().data(); }
void Recognizer::setCurrent(Recognizer *recognizer) { currentPointer() = recognizer; }

static Recognizer::Installer &installerRef()
{
    static Recognizer::Installer i;
    return i;
}
void Recognizer::setInstaller(const Installer &installer) { installerRef() = installer; }
const Recognizer::Installer &Recognizer::installer() { return installerRef(); }

QStringList modelLanguages()
{
    // the program's languages; Russian has no model yet
    return {QStringLiteral("en"), QStringLiteral("de"), QStringLiteral("es"), QStringLiteral("fr"), QStringLiteral("pt"),
            QStringLiteral("it"), QStringLiteral("id"), QStringLiteral("tr"), QStringLiteral("pl"), QStringLiteral("nl")};
}

bool hasModelFor(const QString &language) { return modelLanguages().contains(language); }

void FakeRecognizer::recognize(const Request &request, const std::function<void(const Result &)> &done)
{
    ++m_requests;
    m_last = request;
    Result r;
    r.ok = true;
    const QString text = m_texts.isEmpty() ? QString() : m_texts.value(qMin(m_next, int(m_texts.size()) - 1));
    ++m_next;
    const QStringList words = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    const qreal w = words.isEmpty() ? 0 : request.picture.width() / qreal(words.size());
    for (int i = 0; i < words.size(); ++i) {
        Word word;
        word.text = words[i];
        word.alternatives = m_alternatives.value(words[i]);
        word.box = QRectF(i * w, 0, w, request.picture.height());
        r.words << word;
    }
    done(r);
}

} // namespace handwriting
} // namespace qb
