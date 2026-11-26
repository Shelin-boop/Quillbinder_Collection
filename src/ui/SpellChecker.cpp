#include "ui/SpellChecker.h"
#include "core/SafeSave.h"
#include "ui/Theme.h"

#include <QEvent>
#include <QDynamicPropertyChangeEvent>
#include <QTextBlock>
#include <QTextFragment>
#include "core/Addons.h"
#include "core/Launcher.h"
#include "core/Paths.h"
#include "text/TextProps.h"
#include "text/TextUtil.h"

#include "AppInfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextDocument>
#include <QPointer>

#ifdef QB_HAVE_HUNSPELL
#include <hunspell/hunspell.h>
#endif

namespace qb {

static bool s_enabled = true;
static SpellChecker::Display s_display;
static QList<QPointer<SpellHighlighter>> s_highlighters;
// suggestions already looked up (0.60.5): a second right-click on the same word does not
// search the dictionary again; emptied when the language or the personal words change
static QHash<QString, QStringList> s_suggested;

static QStringList dictionaryDirs()
{
#ifdef Q_OS_WIN
    // Windows has no system dictionaries: they come with the program (port, W3: English and German),
    // in the program folder (also when a program update runs) and beside a test program
    QStringList dirs = {launcher::appDir() + QLatin1String("/dictionaries"),
                        QCoreApplication::applicationDirPath() + QLatin1String("/dictionaries")};
    dirs.removeDuplicates();
#else
    QStringList dirs = {QStringLiteral("/usr/share/hunspell"), QStringLiteral("/usr/share/myspell/dicts"),
                        QStringLiteral("/usr/share/myspell"), QStringLiteral("/usr/local/share/hunspell")};
#endif
    dirs << addons::contentDirs(QStringLiteral("dictionaries"));
    dirs << dataDir() + QLatin1String("/dictionaries");
    return dirs;
}

// Hunspell opens its files with the C library: on Windows only a path in the ANSI code page works,
// unless it starts with \\?\ and is UTF-8 (Hunspell then opens it with the wide-character call)
static QByteArray hunspellPath(const QString &file)
{
#ifdef Q_OS_WIN
    return QByteArray("\\\\?\\") + QDir::toNativeSeparators(QFileInfo(file).absoluteFilePath()).toUtf8();
#else
    return QFile::encodeName(file);
#endif
}

// ISO 8859-2 (the Polish dictionary of LibreOffice): the characters 0xA0 to 0xFF (0.95.0)
static const ushort kLatin2[96] = {
    0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7,
    0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
    0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7,
    0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
    0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
    0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
    0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
    0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
};

// Hunspell wants words in the encoding of the loaded dictionary ("SET" in its .aff file): the
// German dictionaries of LibreOffice and Ubuntu are ISO 8859-1, so words with umlauts or ß passed
// as UTF-8 were never found (W3, 2026-11-08). 0 UTF-8, 1 ISO 8859-1, 2 ISO 8859-15, 3 ISO 8859-2 (Polish, 0.95.0); other
// encodings are handed over as UTF-8, as before.
static int s_encoding = 0;

static void takeEncoding(void *handle)
{
#ifdef QB_HAVE_HUNSPELL
    const QByteArray e = QByteArray(Hunspell_get_dic_encoding(static_cast<Hunhandle *>(handle))).toUpper().replace('-', "");
    s_encoding = (e == "ISO88591" || e == "LATIN1") ? 1 : e == "ISO885915" ? 2 : (e == "ISO88592" || e == "LATIN2") ? 3 : 0;
#else
    Q_UNUSED(handle);
#endif
}

// ISO 8859-15 differs from 8859-1 in eight places
static const ushort kLatin9[8][2] = {{0xA4, 0x20AC}, {0xA6, 0x0160}, {0xA8, 0x0161}, {0xB4, 0x017D},
                                     {0xB8, 0x017E}, {0xBC, 0x0152}, {0xBD, 0x0153}, {0xBE, 0x0178}};

static QByteArray toDictionary(const QString &word)
{
    if (s_encoding == 0)
        return word.toUtf8();
    QByteArray out;
    out.reserve(word.size());
    for (const QChar c : word) {
        ushort u = c.unicode();
        if (s_encoding == 3) {
            if (u >= 0xA0) {
                int at = -1;
                for (int k = 0; k < 96; ++k)
                    if (kLatin2[k] == u) {
                        at = k;
                        break;
                    }
                u = at < 0 ? ushort('?') : ushort(0xA0 + at);
            }
            out.append(u < 0x100 ? char(u) : '?');
            continue;
        }
        if (s_encoding == 2) {
            bool mapped = false;
            for (const auto &m : kLatin9) {
                if (u == m[1]) {
                    u = m[0];
                    mapped = true;
                } else if (u == m[0] && !mapped) {
                    u = '?'; // a Latin-1 letter that ISO 8859-15 does not have there
                }
            }
        }
        out.append(u < 0x100 ? char(u) : '?'); // not in the dictionary's letters: cannot be in it
    }
    return out;
}

static QString fromDictionary(const char *text)
{
    if (s_encoding == 0)
        return QString::fromUtf8(text);
    QString out = QString::fromLatin1(text);
    if (s_encoding == 3) {
        for (QChar &c : out)
            if (c.unicode() >= 0xA0 && c.unicode() < 0x100)
                c = QChar(kLatin2[c.unicode() - 0xA0]);
        return out;
    }
    if (s_encoding == 2)
        for (QChar &c : out)
            for (const auto &m : kLatin9)
                if (c.unicode() == m[0]) {
                    c = QChar(m[1]);
                    break;
                }
    return out;
}

SpellChecker &SpellChecker::instance()
{
    static SpellChecker s;
    return s;
}

SpellChecker::SpellChecker()
{
    QFile f(personalFile());
    if (f.open(QIODevice::ReadOnly)) {
        for (const QByteArray &line : f.readAll().split('\n'))
            if (!line.trimmed().isEmpty())
                m_personal.insert(QString::fromUtf8(line.trimmed()));
    }
}

SpellChecker::~SpellChecker()
{
#ifdef QB_HAVE_HUNSPELL
    if (m_handle)
        Hunspell_destroy(static_cast<Hunhandle *>(m_handle));
#endif
}

QString SpellChecker::personalFile() const
{
    return dataDir() + QLatin1String("/personal-dictionary.txt");
}

// The program's name and the words of its own (the manual and the welcome project use them on every
// page; before 0.15.0 they were all underlined as mistakes).
const QStringList &SpellChecker::programWords()
{
    static const QStringList words = {QStringLiteral(QB_APP_NAME), QStringLiteral("outliner"),
                                      QStringLiteral("outliners"), QStringLiteral("subdocument"), QStringLiteral("subdocuments"),
                                      QStringLiteral("copyholder"), QStringLiteral("copyholders"), QStringLiteral("Ctrl"),
                                      QStringLiteral("Esc")};
    return words;
}

QStringList SpellChecker::availableLanguages() const
{
    QStringList out;
    for (const QString &d : dictionaryDirs()) {
        for (const QString &f : QDir(d).entryList({QStringLiteral("*.dic")}, QDir::Files)) {
            const QString lang = f.left(f.size() - 4);
            if (QFile::exists(QDir(d).filePath(lang + QLatin1String(".aff"))) && !out.contains(lang))
                out << lang;
        }
    }
    out.sort();
    return out;
}

bool SpellChecker::setLanguage(const QString &lang)
{
#ifdef QB_HAVE_HUNSPELL
    if (lang == m_language && m_handle)
        return true;
    s_suggested.clear();
    for (const QString &d : dictionaryDirs()) {
        const QString aff = QDir(d).filePath(lang + QLatin1String(".aff"));
        const QString dic = QDir(d).filePath(lang + QLatin1String(".dic"));
        if (QFile::exists(aff) && QFile::exists(dic)) {
            if (m_handle)
                Hunspell_destroy(static_cast<Hunhandle *>(m_handle));
            m_handle = Hunspell_create(hunspellPath(aff).constData(), hunspellPath(dic).constData());
            takeEncoding(m_handle);
            m_language = lang;
            m_cache.clear();
            for (const QString &w : m_personal)
                Hunspell_add(static_cast<Hunhandle *>(m_handle), toDictionary(w).constData());
            for (const QString &w : programWords())
                Hunspell_add(static_cast<Hunhandle *>(m_handle), toDictionary(w).constData());
            return true;
        }
    }
#else
    Q_UNUSED(lang);
#endif
    return false;
}

bool SpellChecker::isCorrect(const QString &word)
{
#ifdef QB_HAVE_HUNSPELL
    if (!m_handle || word.isEmpty())
        return true;
    if (m_ignored.contains(word) || m_personal.contains(word))
        return true;
    auto it = m_cache.constFind(word);
    if (it != m_cache.constEnd())
        return *it;
    QString w = word;
    w.replace(QChar(0x2019), QLatin1Char('\''));
    const bool ok = Hunspell_spell(static_cast<Hunhandle *>(m_handle), toDictionary(w).constData()) != 0;
    if (m_cache.size() > 50000)
        m_cache.clear();
    m_cache.insert(word, ok);
    return ok;
#else
    Q_UNUSED(word);
    return true;
#endif
}

QStringList SpellChecker::suggestions(const QString &word)
{
    QStringList out;
#ifdef QB_HAVE_HUNSPELL
    if (!m_handle)
        return out;
    auto known = s_suggested.constFind(word);
    if (known != s_suggested.constEnd())
        return *known;
    char **list = nullptr;
    const int n = Hunspell_suggest(static_cast<Hunhandle *>(m_handle), &list, toDictionary(word).constData());
    for (int i = 0; i < n && i < 8; ++i)
        out << fromDictionary(list[i]);
    Hunspell_free_list(static_cast<Hunhandle *>(m_handle), &list, n);
    if (s_suggested.size() > 500)
        s_suggested.clear();
    s_suggested.insert(word, out);
#else
    Q_UNUSED(word);
#endif
    return out;
}

QStringList SpellChecker::personalWords() const
{
    QStringList out(m_personal.cbegin(), m_personal.cend());
    out.sort(Qt::CaseInsensitive);
    return out;
}

// Writes the list anew; the dictionary is loaded again so that removed words
// count as mistakes again (Hunspell cannot forget single added words reliably).
void SpellChecker::setPersonalWords(const QStringList &words)
{
    s_suggested.clear();
    m_personal = QSet<QString>(words.cbegin(), words.cend());
    m_cache.clear();
    writePersonalFile();
    const QString lang = m_language;
#ifdef QB_HAVE_HUNSPELL
    if (m_handle) {
        Hunspell_destroy(static_cast<Hunhandle *>(m_handle));
        m_handle = nullptr;
    }
#endif
    m_language.clear();
    if (!lang.isEmpty())
        setLanguage(lang);
    rehighlightAll();
}

void SpellChecker::addToPersonal(const QString &word)
{
    s_suggested.clear();
    m_personal.insert(word);
    m_cache.remove(word);
#ifdef QB_HAVE_HUNSPELL
    if (m_handle)
        Hunspell_add(static_cast<Hunhandle *>(m_handle), toDictionary(word).constData());
#endif
    writePersonalFile();
    rehighlightAll();
}

// 0.70.0 (audit A F-09): the whole list into a new file that takes the old one's place only when it is
// complete (before: the file was emptied first and written word by word, unchecked)
bool SpellChecker::writePersonalFile()
{
    QDir().mkpath(QFileInfo(personalFile()).absolutePath());
    QByteArray data;
    for (const QString &w : personalWords())
        data += w.toUtf8() + '\n';
    QString why;
    if (safesave::writeFile(personalFile(), data, &why))
        return true;
    qWarning("%s: the personal dictionary could not be saved: %s", QB_APP_NAME, qPrintable(why));
    return false;
}

void SpellChecker::ignore(const QString &word)
{
    m_ignored.insert(word);
    rehighlightAll();
}

void SpellChecker::attach(QTextDocument *doc)
{
    if (!doc || doc->findChild<SpellHighlighter *>())
        return;
    auto *h = new SpellHighlighter(doc);
    s_highlighters << h;
}

void SpellChecker::setEnabled(bool on)
{
    if (s_enabled == on)
        return;
    s_enabled = on;
    rehighlightAll();
}

bool SpellChecker::isEnabled() { return s_enabled; }

void SpellChecker::setDisplay(const Display &d)
{
    s_display = d;
}

SpellChecker::Display SpellChecker::display() { return s_display; }

void SpellChecker::rehighlightAll()
{
    for (auto &h : s_highlighters)
        if (h)
            h->rehighlight();
}

SpellHighlighter::SpellHighlighter(QTextDocument *doc) : QSyntaxHighlighter(doc)
{
    doc->installEventFilter(this);
}

// The document store announces the zoom as a dynamic property of the document.
bool SpellHighlighter::eventFilter(QObject *o, QEvent *e)
{
    if (o == document() && e->type() == QEvent::DynamicPropertyChange
        && static_cast<QDynamicPropertyChangeEvent *>(e)->propertyName() == "qbZoom")
        QMetaObject::invokeMethod(this, "rehighlight", Qt::QueuedConnection);
    return QSyntaxHighlighter::eventFilter(o, e);
}

namespace {
// words with digits are one word ("Route66"); numbers alone are never checked
const QRegularExpression &spellWordRe()
{
    static const QRegularExpression re(QStringLiteral("[\\p{L}\\p{N}][\\p{L}\\p{M}\\p{N}'\\x{2019}]*"));
    return re;
}

// the word as it is checked: apostrophes at its end taken off; "" when it is not checked
QString checkable(QString w)
{
    while (w.endsWith(QLatin1Char('\'')) || w.endsWith(QChar(0x2019)))
        w.chop(1);
    if (w.size() < 2)
        return QString();
    bool digits = false, letters = false;
    for (const QChar ch : w)
        (ch.isDigit() ? digits : letters) = true;
    if (!letters || (digits && SpellChecker::display().ignoreNumbers))
        return QString();
    return w;
}
} // namespace

QString SpellChecker::checkedWordAt(const QString &paragraph, int pos, int *start)
{
    auto it = spellWordRe().globalMatch(paragraph);
    while (it.hasNext()) {
        const auto m = it.next();
        const int from = int(m.capturedStart());
        if (from > pos)
            break;
        const QString w = checkable(m.captured());
        if (!w.isEmpty() && pos <= from + w.size()) {
            if (start)
                *start = from;
            return w;
        }
    }
    return QString();
}

void SpellHighlighter::highlightBlock(const QString &text)
{
    const QTextBlock block = currentBlock();
    // Zoom: the document's default font is scaled by the document store.
    // Text with a font size of its own is scaled here, for display only, so
    // that the stored size stays what the writer chose.
    const double zoom = document()->property("qbZoom").toDouble();
    const bool scale = zoom > 0 && !qFuzzyCompare(zoom, 1.0);
    // Links are stored with one blue; on screen they get the theme's link
    // color (a dark paper needs a lighter blue). Display only as well.
    static const QColor storedLink(30, 90, 200);
    const QColor link = theme::color(theme::Role::EditorLink);
    const bool recolor = link.isValid() && link.rgb() != storedLink.rgb();
    const bool plainNotes = s_display.plainAnnotations;
    if (scale || recolor || plainNotes) {
        for (auto fit = block.begin(); !fit.atEnd(); ++fit) {
            const QTextFragment f = fit.fragment();
            if (!f.isValid())
                continue;
            const QTextCharFormat cf = f.charFormat();
            QTextCharFormat shown;
            if (scale && cf.hasProperty(QTextFormat::FontPointSize) && cf.fontPointSize() > 0)
                shown.setFontPointSize(cf.fontPointSize() * zoom);
            if (recolor && cf.isAnchor() && cf.foreground().style() == Qt::SolidPattern
                && cf.foreground().color().rgb() == storedLink.rgb())
                shown.setForeground(link);
            if (plainNotes && NoteKind(cf.intProperty(props::NoteKind)) == NoteKind::InlineAnnotation)
                shown.setForeground(theme::color(theme::Role::EditorText));
            if (!shown.isEmpty())
                setFormat(f.position() - block.position(), f.length(), shown);
        }
    }
    if (!s_enabled || !SpellChecker::instance().isAvailable())
        return;
    QTextCharFormat err;
    err.setUnderlineStyle(s_display.underline == 1   ? QTextCharFormat::DotLine
                          : s_display.underline == 2 ? QTextCharFormat::SingleUnderline
                                                     : QTextCharFormat::SpellCheckUnderline);
    err.setUnderlineColor(theme::color(theme::Role::SpellingError));
    auto it = spellWordRe().globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        const QString w = checkable(m.captured());
        if (w.isEmpty())
            continue;
        // skip words inside links
        bool skip = false;
        for (auto fit = block.begin(); !fit.atEnd(); ++fit) {
            const QTextFragment f = fit.fragment();
            const int rel = f.position() - block.position();
            if (m.capturedStart() >= rel && m.capturedStart() < rel + f.length()) {
                if (f.charFormat().isAnchor())
                    skip = true;
                break;
            }
        }
        if (skip)
            continue;
        if (!SpellChecker::instance().isCorrect(w)) {
            // keep a zoomed font size under the underline
            const int start = int(m.capturedStart());
            for (int k = 0; k < w.size(); ++k) {
                QTextCharFormat f = format(start + k);
                f.merge(err);
                setFormat(start + k, 1, f);
            }
        }
    }
}

} // namespace qb
