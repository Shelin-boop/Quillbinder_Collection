// Scripted run for version 0.95.0, item 102 (Quillbinder in eleven languages), part 1: the language list and its marks, the
// language of a first start, the interface texts of every language (the .ts complete, the .qm of the .ts, the placeholders
// kept), the manuals, quick start guides and welcome projects as files (the manual's headings in the English order), the
// object lists' Choice values, the plural forms, the first-start page's and Options' drop-down lists.
//
//   xvfb-run -a ./qb_v0950f_tests /tmp/shots
#include "AppInfo.h"
#include "core/Objects.h"
#include "text/TextUtil.h"
#include "ui/AppSettings.h"
#include "ui/FirstStartDialog.h"
#include "ui/Language.h"
#include "ui/MainWindow.h"
#include "ui/OptionsDialog.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTranslator>
#include <QXmlStreamReader>

#include <cstdio>

using namespace qb;

static int failures = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 150)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

static void shot(QWidget *w, const QString &name)
{
    settle(300);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

static const QStringList kCodes = {QStringLiteral("en"), QStringLiteral("de"), QStringLiteral("es"), QStringLiteral("fr"),
                                   QStringLiteral("pt"), QStringLiteral("it"), QStringLiteral("ru"), QStringLiteral("id"),
                                   QStringLiteral("tr"), QStringLiteral("pl"), QStringLiteral("nl")};

// ------------------------------------------------------------------ the list

static void listAndMarks()
{
    std::printf("== the language list and its marks\n");
    check(language::available() == kCodes, QStringLiteral("eleven languages: en de es fr pt it ru id tr pl nl"),
          language::available().join(QLatin1Char(' ')));
    const QStringList names = {QStringLiteral("English"), QStringLiteral("Deutsch"), QStringLiteral("Español"), QStringLiteral("Français"),
                               QStringLiteral("Português (Brasil)"), QStringLiteral("Italiano"), QStringLiteral("Русский"),
                               QStringLiteral("Bahasa Indonesia"), QStringLiteral("Türkçe"), QStringLiteral("Polski"),
                               QStringLiteral("Nederlands")};
    QStringList got;
    for (const QString &c : kCodes)
        got << language::displayName(c);
    check(got == names, QStringLiteral("each language is shown by its own name"), got.join(QStringLiteral(" | ")));
    QStringList machine, label;
    for (const QString &c : kCodes) {
        if (language::machineTranslated(c))
            machine << c;
        label << language::listLabel(c);
    }
    check(machine == QStringList({QStringLiteral("es"), QStringLiteral("fr"), QStringLiteral("pt"), QStringLiteral("it"),
                                  QStringLiteral("ru"), QStringLiteral("id"), QStringLiteral("tr"), QStringLiteral("pl"),
                                  QStringLiteral("nl")}),
          QStringLiteral("the nine new languages are marked as machine translations; English and German are not"), machine.join(QLatin1Char(' ')));
    check(label.at(0) == QLatin1String("English") && label.at(1) == QLatin1String("Deutsch")
              && label.at(2) == QStringLiteral("Español (machine translation)") && label.at(4) == QStringLiteral("Português (Brasil) (machine translation)"),
          QStringLiteral("the list shows the mark after the name"), label.join(QStringLiteral(" | ")));
    // in the interface's language: with the German texts the mark is German
    QTranslator de;
    const bool loaded = de.load(QStringLiteral(":/translations/quillbinder_de.qm"));
    QCoreApplication::installTranslator(&de);
    const QString inGerman = language::listLabel(QStringLiteral("fr"));
    QCoreApplication::removeTranslator(&de);
    check(loaded && inGerman == QStringLiteral("Français (maschinelle Übersetzung)"), QStringLiteral("... in the language of the interface"), inGerman);
    // the numbers' locale
    check(language::localeFor(QStringLiteral("de")).language() == QLocale::German && language::localeFor(QStringLiteral("es")).language() == QLocale::Spanish
              && language::localeFor(QStringLiteral("fr")).language() == QLocale::French && language::localeFor(QStringLiteral("pt")).language() == QLocale::Portuguese
              && language::localeFor(QStringLiteral("it")).language() == QLocale::Italian && language::localeFor(QStringLiteral("ru")).language() == QLocale::Russian
              && language::localeFor(QStringLiteral("id")).language() == QLocale::Indonesian && language::localeFor(QStringLiteral("tr")).language() == QLocale::Turkish
              && language::localeFor(QStringLiteral("pl")).language() == QLocale::Polish && language::localeFor(QStringLiteral("nl")).language() == QLocale::Dutch,
          QStringLiteral("each language has its own locale for numbers, dates and units (English: the system's)"));
    check(language::localeFor(QStringLiteral("pt")).territory() == QLocale::Brazil || language::localeFor(QStringLiteral("pt")).territory() == QLocale::Portugal,
          QStringLiteral("... Portuguese: Brazil's (or the system's own)"));
}

// ------------------------------------------------------------------ the first start

static QString runChild(const QString &home, const QStringList &args, const QProcessEnvironment &extra, const QString &key)
{
    QProcess p;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("HOME"), home);
    env.insert(QStringLiteral("XDG_CONFIG_HOME"), home + QStringLiteral("/config"));
    env.insert(QStringLiteral("XDG_DATA_HOME"), home + QStringLiteral("/data"));
    env.remove(QStringLiteral("QB_LANGUAGE"));
    env.remove(QStringLiteral("QB_SYSTEM_LANGUAGE"));
    for (const QString &k : extra.keys())
        env.insert(k, extra.value(k));
    p.setProcessEnvironment(env);
    p.start(QCoreApplication::applicationFilePath(), args);
    if (!p.waitForFinished(60000))
        p.kill();
    const QStringList lines = QString::fromUtf8(p.readAllStandardOutput()).split(QLatin1Char('\n'));
    for (const QString &l : lines)
        if (l.startsWith(key + QLatin1Char('=')))
            return l.mid(key.size() + 1).trimmed();
    return QStringLiteral("(no answer)");
}

static void firstStart()
{
    std::printf("== the language of a first start\n");
    QSettings s;
    s.clear();
    s.sync();
    QStringList wrong;
    const QList<QPair<QString, QString>> systems = {{QStringLiteral("de_DE.UTF-8"), QStringLiteral("de")}, {QStringLiteral("es-ES"), QStringLiteral("es")},
                                                    {QStringLiteral("fr_CA"), QStringLiteral("fr")}, {QStringLiteral("pt_BR"), QStringLiteral("pt")},
                                                    {QStringLiteral("pt_PT"), QStringLiteral("pt")}, {QStringLiteral("it"), QStringLiteral("it")},
                                                    {QStringLiteral("ru_RU"), QStringLiteral("ru")}, {QStringLiteral("id_ID"), QStringLiteral("id")},
                                                    {QStringLiteral("tr"), QStringLiteral("tr")}, {QStringLiteral("pl_PL"), QStringLiteral("pl")},
                                                    {QStringLiteral("nl-NL"), QStringLiteral("nl")}, {QStringLiteral("en_GB"), QStringLiteral("en")},
                                                    {QStringLiteral("ja_JP"), QStringLiteral("en")}, {QStringLiteral("zh-CN"), QStringLiteral("en")},
                                                    {QStringLiteral("C"), QStringLiteral("en")}, {QStringLiteral("DE"), QStringLiteral("de")}};
    for (const auto &sys : systems) {
        qputenv("QB_SYSTEM_LANGUAGE", sys.first.toUtf8());
        if (language::fromSystem() != sys.second || language::configured() != sys.second)
            wrong << sys.first + QStringLiteral("->") + language::fromSystem() + QLatin1Char('/') + language::configured();
    }
    check(wrong.isEmpty(), QStringLiteral("a true first start takes the system's language when it is one of the eleven, else English (%1 systems)").arg(systems.size()),
          wrong.join(QStringLiteral("; ")));
    qputenv("QB_SYSTEM_LANGUAGE", "de_DE");
    check(language::isFirstStart() && language::configured() == QLatin1String("de"), QStringLiteral("... no settings at all: a first start"));
    // an installation that updates has settings of an earlier version: it keeps English
    QStringList early;
    for (const char *k : {"app/firstStartDone", "app/welcomeShown", "app/recent", "app/forename", "app/authorAsked", "app/language"}) {
        s.clear();
        s.setValue(QLatin1String(k), k == QLatin1String("app/recent") ? QVariant(QStringList({QStringLiteral("/x")})) : QVariant(true));
        s.sync();
        if (language::isFirstStart() || (k != QLatin1String("app/language") && language::configured() != QLatin1String("en")))
            early << QLatin1String(k);
    }
    check(early.isEmpty(), QStringLiteral("an installation that updates (settings from an earlier version) is no first start and keeps English"), early.join(QLatin1Char(' ')));
    s.clear();
    s.setValue(QStringLiteral("app/language"), QStringLiteral("fr"));
    s.sync();
    check(language::configured() == QLatin1String("fr"), QStringLiteral("a stored language wins over the system's"));
    s.setValue(QStringLiteral("app/language"), QStringLiteral("xx"));
    s.sync();
    check(language::configured() == QLatin1String("en"), QStringLiteral("... and an unknown stored one is English"));
    qputenv("QB_LANGUAGE", "tr");
    check(language::configured() == QLatin1String("tr"), QStringLiteral("QB_LANGUAGE (the checks) wins over both"));
    qunsetenv("QB_LANGUAGE");
    qunsetenv("QB_SYSTEM_LANGUAGE");
    s.clear();
    s.sync();
    // AppSettings keeps every one of the eleven
    AppSettings &as = AppSettings::instance();
    QStringList lost;
    for (const QString &c : kCodes) {
        as.language = c;
        as.save();
        as.language = QStringLiteral("en");
        as.load();
        if (as.language != c)
            lost << c;
    }
    check(lost.isEmpty(), QStringLiteral("the settings keep each of the eleven languages"), lost.join(QLatin1Char(' ')));
    s.clear();
    s.sync();

    // whole starts: a child with no settings and the system's language Spanish; one that was used before
    QTemporaryDir root;
    QProcessEnvironment es;
    es.insert(QStringLiteral("QB_SYSTEM_LANGUAGE"), QStringLiteral("es_ES.UTF-8"));
    const QString fresh = runChild(root.path() + QStringLiteral("/a"), {QStringLiteral("--child-first")}, es, QStringLiteral("menu"));
    const QString freshLang = runChild(root.path() + QStringLiteral("/a"), {QStringLiteral("--child-first")}, es, QStringLiteral("language"));
    check(freshLang == QLatin1String("es") && fresh == QLatin1String("&Archivo"),
          QStringLiteral("first start on a Spanish system: the program speaks Spanish (File menu: %1)").arg(fresh), freshLang);
    const QString used = runChild(root.path() + QStringLiteral("/b"), {QStringLiteral("--child-first"), QStringLiteral("used")}, es, QStringLiteral("language"));
    check(used == QLatin1String("en"), QStringLiteral("an installation that was used before keeps English"), used);
    const QString varOnly = runChild(root.path() + QStringLiteral("/c"), {QStringLiteral("--child-first")},
                                     [] { QProcessEnvironment e; e.insert(QStringLiteral("QB_SYSTEM_LANGUAGE"), QStringLiteral("ja_JP")); return e; }(),
                                     QStringLiteral("language"));
    check(varOnly == QLatin1String("en"), QStringLiteral("first start on a system in a language the program does not speak: English"), varOnly);
}

// ------------------------------------------------------------------ the interface texts

struct TsMessage {
    QString context, source, comment, translation;
    bool numerus = false, unfinished = false, vanished = false;
};

static QList<TsMessage> readTs(const QString &path)
{
    QList<TsMessage> out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return out;
    QXmlStreamReader x(&f);
    QString ctx;
    TsMessage cur;
    bool inMsg = false;
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement()) {
            const QString n = x.name().toString();
            if (n == QLatin1String("context")) {
                ctx.clear();
            } else if (n == QLatin1String("name") && !inMsg) {
                ctx = x.readElementText();
            } else if (n == QLatin1String("message")) {
                cur = TsMessage();
                cur.context = ctx;
                cur.numerus = x.attributes().value(QLatin1String("numerus")) == QLatin1String("yes");
                inMsg = true;
            } else if (n == QLatin1String("source") && inMsg) {
                cur.source = x.readElementText(QXmlStreamReader::IncludeChildElements);
            } else if (n == QLatin1String("comment") && inMsg) {
                cur.comment = x.readElementText();
            } else if (n == QLatin1String("translation") && inMsg) {
                const QString type = x.attributes().value(QLatin1String("type")).toString();
                cur.unfinished = type == QLatin1String("unfinished");
                cur.vanished = type == QLatin1String("vanished") || type == QLatin1String("obsolete");
                if (!cur.numerus)
                    cur.translation = x.readElementText(QXmlStreamReader::IncludeChildElements);
                else
                    cur.translation = QStringLiteral("(numerus)");
            }
        } else if (x.isEndElement() && x.name() == QLatin1String("message")) {
            out << cur;
            inMsg = false;
        }
    }
    return out;
}

static QStringList placeholders(const QString &t)
{
    static const QRegularExpression re(QStringLiteral("%(L?[1-9]|n)"));
    QStringList l;
    auto it = re.globalMatch(t);
    while (it.hasNext())
        l << it.next().captured(0);
    l.sort();
    return l;
}

static void interfaceTexts(const QString &src)
{
    std::printf("== the interface of every language: complete, compiled from its .ts, placeholders kept\n");
    // the reference: every text that one of the nine new languages has (the German file also holds texts the program no longer
    // has, and the texts of 0.95.0 itself are only in German until the others are translated)
    QSet<QString> refKeys;
    for (const QString &code : kCodes) {
        if (code == QLatin1String("en") || code == QLatin1String("de"))
            continue;
        for (const TsMessage &m : readTs(src + QStringLiteral("/translations/quillbinder_%1.ts").arg(code)))
            // (the options note about the language was reworded in 0.95.0: the nine languages have the old text until translated)
            if (!m.vanished && m.context != QLatin1String("QObject") && !m.source.startsWith(QLatin1String("The language of menus, windows and messages")))
                refKeys.insert(m.context + QLatin1Char('\x1f') + m.source + QLatin1Char('\x1f') + m.comment);
    }
    for (const QString &code : kCodes) {
        if (code == QLatin1String("en"))
            continue;
        const QString path = src + QStringLiteral("/translations/quillbinder_%1.ts").arg(code);
        const QList<TsMessage> ts = readTs(path);
        int unfinished = 0, empty = 0;
        QStringList badHolders;
        QSet<QString> keys;
        for (const TsMessage &m : ts) {
            if (m.vanished)
                continue;
            keys.insert(m.context + QLatin1Char('\x1f') + m.source + QLatin1Char('\x1f') + m.comment);
            if (m.unfinished)
                ++unfinished;
            else if (m.translation.trimmed().isEmpty() && !m.numerus)
                ++empty;
            if (!m.numerus && !m.unfinished && !m.translation.isEmpty() && placeholders(m.source) != placeholders(m.translation)
                && badHolders.size() < 8)
                badHolders << m.source.left(60) + QStringLiteral(" => ") + m.translation.left(60);
        }
        int missing = 0;
        QString firstMissing;
        for (const QString &k : refKeys)
            if (!keys.contains(k)) {
                if (missing < 6)
                    firstMissing += QStringLiteral(" [") + k.left(70).replace(QLatin1Char('\x1f'), QLatin1Char('|')).replace(QLatin1Char('\n'), QLatin1Char(' ')) + QLatin1Char(']');
                ++missing;
            }
        check(ts.size() > 4000 && unfinished == 0 && empty == 0 && missing == 0,
              QStringLiteral("%1: %2 texts, all translated, none missing against the other languages").arg(code).arg(ts.size()),
              QStringLiteral("%1 unfinished, %2 empty, %3 missing (first: %4)").arg(unfinished).arg(empty).arg(missing).arg(firstMissing));
        check(badHolders.isEmpty(), QStringLiteral("%1: every placeholder (%%1, %%n ...) of a text is in its translation").arg(code),
              badHolders.join(QStringLiteral("; ")));
        // the compiled file is the one of the .ts
        QTranslator t;
        const bool loaded = t.load(QStringLiteral(":/translations/quillbinder_%1.qm").arg(code));
        int differ = 0, tested = 0;
        QString firstDiff;
        for (const TsMessage &m : ts) {
            if (m.vanished || m.unfinished || m.numerus || m.translation.isEmpty())
                continue;
            ++tested;
            const QString got = t.translate(m.context.toUtf8().constData(), m.source.toUtf8().constData(), m.comment.isEmpty() ? nullptr : m.comment.toUtf8().constData());
            if (got != m.translation) {
                if (!differ)
                    firstDiff = m.source.left(60) + QStringLiteral(" => ") + got.left(60) + QStringLiteral(" | ") + m.translation.left(60);
                ++differ;
            }
        }
        check(loaded && tested > 4000 && differ == 0, QStringLiteral("%1: quillbinder_%1.qm holds the %2 texts of the .ts").arg(code).arg(tested),
              QStringLiteral("%1 differ (first: %2)").arg(differ).arg(firstDiff));
        // Qt's own texts
        const QList<TsMessage> qts = readTs(src + QStringLiteral("/translations/qt_%1.ts").arg(code));
        QTranslator q;
        const bool qloaded = q.load(QStringLiteral(":/translations/qt_%1.qm").arg(code));
        int qdiffer = 0, qunfinished = 0, qtested = 0;
        for (const TsMessage &m : qts) {
            if (m.vanished || m.numerus)
                continue;
            if (m.unfinished || m.translation.isEmpty()) {
                ++qunfinished;
                continue;
            }
            ++qtested;
            if (q.translate(m.context.toUtf8().constData(), m.source.toUtf8().constData(), m.comment.isEmpty() ? nullptr : m.comment.toUtf8().constData())
                != m.translation)
                ++qdiffer;
        }
        check(qloaded && qtested > 10 && qdiffer == 0 && qunfinished == 0, QStringLiteral("%1: qt_%1.qm holds the %2 texts of Qt's own windows").arg(code).arg(qtested),
              QStringLiteral("%1 differ, %2 unfinished").arg(qdiffer).arg(qunfinished));
    }
}

// ------------------------------------------------------------------ manuals, quick start guides, welcome projects as files

struct Heading {
    int level;
    QString number;
};

static QList<Heading> headingsOfFile(const QByteArray &md)
{
    QList<Heading> out;
    bool fence = false;
    for (const QByteArray &raw : md.split('\n')) {
        const QString line = QString::fromUtf8(raw);
        if (line.startsWith(QLatin1String("```")) || line.startsWith(QLatin1String("~~~"))) {
            fence = !fence;
            continue;
        }
        if (fence || !line.startsWith(QLatin1Char('#')))
            continue;
        int n = 0;
        while (n < line.size() && line.at(n) == QLatin1Char('#'))
            ++n;
        if (n > 6 || n >= line.size() || line.at(n) != QLatin1Char(' '))
            continue;
        const QRegularExpression num(QStringLiteral("^([0-9]+(?:\\.[0-9]+)*)\\.?\\s"));
        const auto m = num.match(line.mid(n + 1));
        out << Heading{n, m.hasMatch() ? m.captured(1) : QString()};
    }
    return out;
}

static void documents(const QString &src)
{
    std::printf("== the manual, the quick start guide and the welcome project of every language\n");
    const QList<Heading> english = headingsOfFile(readAll(src + QStringLiteral("/docs/manual.md")));
    const QList<Heading> englishQuick = headingsOfFile(readAll(src + QStringLiteral("/docs/quick-start.md")));
    check(english.size() > 100, QStringLiteral("the English manual has %1 headings").arg(english.size()));
    QDir en(src + QStringLiteral("/resources/welcome/docs"));
    const QStringList enDocs = en.entryList({QStringLiteral("*.md")}, QDir::Files, QDir::Name);
    const QJsonObject enOutline = QJsonDocument::fromJson(readAll(src + QStringLiteral("/resources/welcome/outline.json"))).object();
    for (const QString &code : kCodes) {
        if (code == QLatin1String("en"))
            continue;
        const QByteArray manual = readAll(src + QStringLiteral("/docs/i18n/%1/manual.md").arg(code));
        const QList<Heading> h = headingsOfFile(manual);
        QString diff;
        if (h.size() != english.size())
            diff = QStringLiteral("%1 headings, English has %2").arg(h.size()).arg(english.size());
        else
            for (int i = 0; i < h.size(); ++i)
                if (h[i].level != english[i].level || h[i].number != english[i].number) {
                    diff = QStringLiteral("heading %1: level %2 number \"%3\", English level %4 number \"%5\"")
                               .arg(i + 1).arg(h[i].level).arg(h[i].number).arg(english[i].level).arg(english[i].number);
                    break;
                }
        check(manual.size() > 100000 && diff.isEmpty(), QStringLiteral("%1: docs/i18n/%1/manual.md has the English headings (count, level, order, numbers)").arg(code), diff);
        check(QByteArray(readAll(QStringLiteral(":/docs/i18n/%1/manual.md").arg(code))) == manual && !manual.isEmpty(),
              QStringLiteral("%1: the program carries the manual as a resource").arg(code));
        const QByteArray quick = readAll(QStringLiteral(":/docs/i18n/%1/quick-start.md").arg(code));
        const QList<Heading> qh = headingsOfFile(quick);
        check(quick.size() > 2000 && qh.size() == englishQuick.size() && quick.contains("{{APP}}"),
              QStringLiteral("%1: the quick start guide (%2 headings, as in English)").arg(code).arg(qh.size()), QStringLiteral("%1 headings, English %2").arg(qh.size()).arg(englishQuick.size()));
        // the welcome project: the outline and the same documents as the English one
        const QString root = QStringLiteral(":/welcome/i18n/%1").arg(code);
        const QJsonObject o = QJsonDocument::fromJson(readAll(root + QStringLiteral("/outline.json"))).object();
        QStringList docs;
        for (const QString &d : enDocs)
            if (!QFile::exists(root + QStringLiteral("/docs/") + d))
                docs << d;
        check(!o.isEmpty() && docs.isEmpty() && o.value(QStringLiteral("draft")).toArray().size() == enOutline.value(QStringLiteral("draft")).toArray().size()
                  && o.value(QStringLiteral("objects")).toArray().size() == enOutline.value(QStringLiteral("objects")).toArray().size(),
              QStringLiteral("%1: the welcome project has the outline and every document of the English one").arg(code), QStringLiteral("missing: ") + docs.join(QLatin1Char(' ')));
        check(language::docResource(QStringLiteral("manual.md"), code) == QStringLiteral(":/docs/i18n/%1/manual.md").arg(code)
                  && language::welcomeRoot(code) == root,
              QStringLiteral("%1: the program asks for these files").arg(code));
    }
    check(language::docResource(QStringLiteral("manual.md"), QStringLiteral("en")) == QLatin1String(":/docs/manual.md")
              && language::welcomeRoot(QStringLiteral("en")) == QLatin1String(":/welcome")
              && language::docResource(QStringLiteral("manual.md"), QStringLiteral("xx")) == QLatin1String(":/docs/manual.md"),
          QStringLiteral("English (and a language without files) gets the English ones"));
}

// ------------------------------------------------------------------ the Choice values of the object lists

static void objectChoices(const QString &src)
{
    std::printf("== the objects' Choice values: the list and the welcome project agree in every language\n");
    const QJsonObject enOutline = QJsonDocument::fromJson(readAll(src + QStringLiteral("/resources/welcome/outline.json"))).object();
    for (const QString &code : kCodes) {
        QTranslator t;
        if (code != QLatin1String("en"))
            t.load(QStringLiteral(":/translations/quillbinder_%1.qm").arg(code));
        QCoreApplication::installTranslator(&t);
        const objects::Schema schema = objects::builtInSchema();
        const QString file = code == QLatin1String("en") ? src + QStringLiteral("/resources/welcome/outline.json")
                                                         : src + QStringLiteral("/resources/welcome/i18n/%1/outline.json").arg(code);
        const QJsonObject o = QJsonDocument::fromJson(readAll(file)).object();
        QStringList bad;
        int n = 0;
        for (const QJsonValue &v : o.value(QStringLiteral("objects")).toArray()) {
            const QJsonObject ob = v.toObject();
            const objects::Category *c = schema.category(ob.value(QStringLiteral("category")).toString());
            const QJsonObject values = ob.value(QStringLiteral("values")).toObject();
            for (auto it = values.begin(); it != values.end(); ++it) {
                const objects::FieldDef *f = c ? c->field(it.key()) : nullptr;
                if (!f || f->type != objects::FieldType::Choice)
                    continue;
                ++n;
                if (!f->choices.contains(it.value().toString()))
                    bad << QStringLiteral("%1.%2 = %3").arg(c->id, it.key(), it.value().toString());
            }
        }
        QCoreApplication::removeTranslator(&t);
        check(n >= 6 && bad.isEmpty(), QStringLiteral("%1: the welcome project's %2 Choice values are in the program's lists").arg(code).arg(n), bad.join(QStringLiteral("; ")));
    }
    (void)enOutline;
}

// ------------------------------------------------------------------ plural forms

static void pluralForms()
{
    std::printf("== plural forms by language\n");
    auto word = [](int n, const QString &lang, const QString &one, const QString &many) {
        text::pluralLanguage() = lang;
        const QString r = text::pluralForm(n, one, many);
        text::pluralLanguage() = QStringLiteral("en");
        return r;
    };
    const QString one = QStringLiteral("документ"), many = QStringLiteral("документа|документов");
    QStringList ru;
    for (int n : {0, 1, 2, 3, 4, 5, 11, 12, 14, 21, 22, 25, 101, 111})
        ru << word(n, QStringLiteral("ru"), one, many);
    check(ru == QStringList({QStringLiteral("документов"), one, QStringLiteral("документа"), QStringLiteral("документа"), QStringLiteral("документа"),
                             QStringLiteral("документов"), QStringLiteral("документов"), QStringLiteral("документов"), QStringLiteral("документов"), one,
                             QStringLiteral("документа"), QStringLiteral("документов"), one, QStringLiteral("документов")}),
          QStringLiteral("Russian: one, few (2 to 4, 22 ...) and many"), ru.join(QLatin1Char(' ')));
    QStringList pl;
    for (int n : {0, 1, 2, 4, 5, 12, 22, 25})
        pl << word(n, QStringLiteral("pl"), QStringLiteral("dokument"), QStringLiteral("dokumenty|dokumentów"));
    check(pl == QStringList({QStringLiteral("dokumentów"), QStringLiteral("dokument"), QStringLiteral("dokumenty"), QStringLiteral("dokumenty"),
                             QStringLiteral("dokumentów"), QStringLiteral("dokumentów"), QStringLiteral("dokumenty"), QStringLiteral("dokumentów")}),
          QStringLiteral("Polish: one, few and many"), pl.join(QLatin1Char(' ')));
    check(word(0, QStringLiteral("fr"), QStringLiteral("page"), QStringLiteral("pages")) == QLatin1String("page")
              && word(2, QStringLiteral("fr"), QStringLiteral("page"), QStringLiteral("pages")) == QLatin1String("pages")
              && word(1, QStringLiteral("pt"), QStringLiteral("página"), QStringLiteral("páginas")) == QStringLiteral("página")
              && word(0, QStringLiteral("pt"), QStringLiteral("página"), QStringLiteral("páginas")) == QStringLiteral("página"),
          QStringLiteral("French and Portuguese: 0 and 1 are singular"));
    check(word(5, QStringLiteral("id"), QStringLiteral("halaman"), QStringLiteral("halaman")) == QLatin1String("halaman")
              && word(5, QStringLiteral("tr"), QStringLiteral("sayfa"), QStringLiteral("sayfalar")) == QLatin1String("sayfa"),
          QStringLiteral("Indonesian and Turkish: no plural after a number"));
    check(word(0, QStringLiteral("en"), QStringLiteral("page"), QStringLiteral("pages")) == QLatin1String("pages")
              && word(1, QStringLiteral("de"), QStringLiteral("Seite"), QStringLiteral("Seiten")) == QLatin1String("Seite")
              && word(2, QStringLiteral("es"), QStringLiteral("página"), QStringLiteral("páginas")) == QStringLiteral("páginas")
              && word(2, QStringLiteral("en"), QStringLiteral("page"), QStringLiteral("pages|many")) == QLatin1String("many"),
          QStringLiteral("English, German, Spanish: as before"));
    text::pluralLanguage() = QStringLiteral("ru");
    const QString full = text::plural(3, one, many);
    text::pluralLanguage() = QStringLiteral("en");
    check(full.endsWith(QStringLiteral(" документа")) && full.startsWith(QLocale().toString(3)), QStringLiteral("plural() writes the number and the form"), full);
}

// ------------------------------------------------------------------ the drop-down lists

static void dropDowns()
{
    std::printf("== the first-start page and File > Options offer the eleven languages\n");
    AppSettings &as = AppSettings::instance();
    as.language = QStringLiteral("it");
    {
        FirstStartDialog d;
        d.show();
        settle(200);
        auto *box = d.findChild<QComboBox *>(QStringLiteral("firstLanguage"));
        QStringList codes, texts;
        for (int i = 0; box && i < box->count(); ++i) {
            codes << box->itemData(i).toString();
            texts << box->itemText(i);
        }
        int marked = 0;
        for (const QString &t : texts)
            marked += t.contains(QLatin1String("(machine translation)")) ? 1 : 0;
        check(box && codes == kCodes && marked == 9 && box->currentData().toString() == QLatin1String("it"),
              QStringLiteral("the first-start page has the list: eleven languages, nine marked, the language now in use chosen"), texts.join(QStringLiteral(" | ")));
        auto *hint = d.findChild<QLabel *>(QStringLiteral("firstLanguageHint"));
        check(hint && hint->text().contains(QLatin1String("next start")), QStringLiteral("... and says that a new language is used from the next start"));
        shot(&d, QStringLiteral("v0950f-first-start-language"));
        if (box)
            box->setCurrentIndex(box->findData(QStringLiteral("pl")));
        d.take();
        QSettings s;
        check(as.language == QLatin1String("pl") && s.value(QStringLiteral("app/language")).toString() == QLatin1String("pl"),
              QStringLiteral("OK keeps the choice (Polish) in the settings"));
    }
    {
        MainWindow w;
        w.show();
        settle(300);
        OptionsDialog dlg(&w);
        dlg.show();
        settle(200);
        auto *box = dlg.findChild<QComboBox *>(QStringLiteral("optLanguage"));
        QStringList codes;
        int marked = 0;
        for (int i = 0; box && i < box->count(); ++i) {
            codes << box->itemData(i).toString();
            marked += box->itemText(i).contains(QLatin1String("(machine translation)")) ? 1 : 0;
        }
        check(box && codes == kCodes && marked == 9 && box->currentData().toString() == QLatin1String("pl"),
              QStringLiteral("File > Options > General > Startup: the same list, with the language now chosen"), codes.join(QLatin1Char(' ')));
        dlg.showSection(OptionsDialog::SGeneral);
        shot(&dlg, QStringLiteral("v0950f-options-language"));
        QString hintText;
        for (QLabel *l : dlg.findChildren<QLabel *>())
            if (l->text().startsWith(QLatin1String("The language of menus")))
                hintText = l->text();
        check(hintText.contains(QLatin1String("manual")) && hintText.contains(QLatin1String("next start")) && !hintText.contains(QLatin1String("stay in English")),
              QStringLiteral("... its note no longer says that the manual and the welcome project stay in English"), hintText);
        if (box)
            box->setCurrentIndex(box->findData(QStringLiteral("nl")));
        dlg.accept();
        settle(100);
        check(as.language == QLatin1String("nl"), QStringLiteral("OK takes the choice (Dutch), for the next start"));
    }
    as.language = QStringLiteral("en");
    as.save();
}

// ------------------------------------------------------------------ main

int main(int argc, char **argv)
{
    const bool childFirst = argc > 1 && QByteArray(argv[1]) == "--child-first";
    QTemporaryDir home;
    if (!childFirst) {
        qputenv("HOME", (home.path() + QLatin1String("/h")).toUtf8());
        qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
        qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
        QDir().mkpath(home.path() + QLatin1String("/h"));
        qunsetenv("QB_LANGUAGE");
        qunsetenv("QB_SYSTEM_LANGUAGE");
    }
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0950fTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0950fTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    if (childFirst) {
        // a start of the program with the settings of this folder: the language it takes
        if (argc > 2 && QByteArray(argv[2]) == "used")
            QSettings().setValue(QStringLiteral("app/firstStartDone"), true);
        language::install();
        AppSettings::instance().load();
        std::printf("language=%s\nsetting=%s\nmenu=%s\n", qPrintable(language::active()), qPrintable(AppSettings::instance().language),
                    qPrintable(QCoreApplication::translate("qb::MainWindow", "&File")));
        return 0;
    }
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    const QString src = QStringLiteral(QB_TEST_SOURCE_DIR);
    AppSettings::instance().load();
    AppSettings::instance().welcomeShown = true;
    AppSettings::instance().spellCheck = false;
    AppSettings::instance().backupOnClose = false;
    AppSettings::instance().backupOnOpen = false;

    listAndMarks();
    firstStart();
    interfaceTexts(src);
    documents(src);
    objectChoices(src);
    pluralForms();
    dropDowns();

    std::printf("\n%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
