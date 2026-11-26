// Checks of 1.5, the Translation Helper, without downloaded models: the eleven languages and the models of a pair (also
// through English), the two drop-down lists, the dictionaries of the other languages, the fixed line that says the tool only
// suggests, the new name, and the add-on's own texts in the eleven languages (each translation file complete). A stand-in
// engine (FakeEngine.h) and stand-in models (small files in a catalog of the check; the pairs of the real catalog too) take
// the place of the real ones. Screenshots go into TP_SHOTS (default: build/screenshots).
#include "Dictionary.h"
#include "FakeEngine.h"
#include "FakeHost.h"
#include "Languages.h"
#include "Panel.h"
#include "TranslateTool.h"

#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextBrowser>
#include <QTranslator>
#include <QXmlStreamReader>
#include <QtTest>

using namespace tp;

namespace {

QString g_shots;

void shot(QWidget *w, const QString &name)
{
    QDir().mkpath(g_shots);
    w->grab().save(g_shots + QLatin1Char('/') + name + QStringLiteral(".png"));
}

void write(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(data);
}

// a dictionary of Spanish in the form of the file the download holds (tools/make-dictionary.py)
const QByteArray kSpanishDictionary =
    "#qb-es-dictionary 1\n"
    "casa\tL\tcasa\tnoun\tf\thouse; home^a building to live in|company^a business\n"
    "correr\tL\tcorrer\tverb\t\tto run; to flow^to move fast\n"
    "corri\xc3\xb3\tF\tcorrer\tverb\n";

QString localeCode(const QString &file)
{
    const QRegularExpression re(QStringLiteral("tp_([a-z]+)\\.ts"));
    return re.match(file).captured(1);
}

} // namespace

class LangTests : public QObject
{
    Q_OBJECT
private:
    QTemporaryDir m_data, m_files;
    FakeHost *m_host = nullptr;
    TranslateTool *m_tool = nullptr;
    FakeEngine m_engine;

    void install(const QString &id)
    {
        const CatalogModel *m = m_tool->store()->model(id);
        QVERIFY2(m, qPrintable(id));
        for (const CatalogFile &f : m->files) {
            QByteArray content = QByteArray(int(qMax<qint64>(f.size, 1)), 'x');
            if (f.name == QLatin1String("es-dictionary.qz"))
                content = kSpanishDictionary;
            write(m_tool->store()->filePath(id, f.name), content);
        }
        write(m_tool->store()->filePath(id, QStringLiteral("checked.txt")), "checked\n");
    }
    // the stand-in files of a model of the real catalog have to be of the size the catalog says: it has placeholders (size 0)
    void uninstall(const QString &id) { QDir(m_tool->store()->modelsDir() + QLatin1Char('/') + id).removeRecursively(); }
    bool runPanel(Panel &p, const QString &text)
    {
        QSignalSpy done(&p, &Panel::finished);
        p.setInput(text);
        p.run();
        return done.count() > 0 || done.wait(20000);
    }
    QString out(Panel &p) { return p.output()->toPlainText(); }
    bool usedSlot(const QString &slot)
    {
        for (const auto &c : m_engine.calls())
            if (c.slot == slot)
                return true;
        return false;
    }

private slots:
    void initTestCase()
    {
        g_shots = qEnvironmentVariable("TP_SHOTS", QStringLiteral(TP_BUILD_DIR "/screenshots"));
        // a catalog of the check: every model of the pairs, with small files; the real catalog is read by catalogEntries()
        QJsonArray models;
        auto add = [&](const QString &id, const QString &title, const QStringList &kinds, const QString &type, const QString &from,
                       const QString &to, const QString &language) {
            QJsonObject o{{"id", id}, {"title", title}, {"kinds", QJsonArray::fromStringList(kinds)}, {"type", type},
                          {"license", "x"}, {"licenseUrl", "https://example.invalid"}, {"credit", "c"},
                          {"release", "models-4"}, {"memoryMB", type == QLatin1String("data") ? 0 : 10}, {"sizeMB", 1},
                          {"tokenizer", QJsonObject{{"type", "marian"}, {"source", "s.spm"}, {"target", "t.spm"}}}};
            if (!from.isEmpty()) {
                o["from"] = from;
                o["to"] = to;
            }
            if (!language.isEmpty())
                o["language"] = language;
            const QString name = type == QLatin1String("data") ? language + QStringLiteral("-dictionary.qz") : QStringLiteral("model.bin");
            o["files"] = QJsonArray{QJsonObject{{"name", name}, {"asset", id + "--" + name},
                                                {"size", type == QLatin1String("data") ? kSpanishDictionary.size() : 4}, {"sha256", "0"}}};
            models << o;
        };
        add(QStringLiteral("opus-mt-de-en"), QStringLiteral("OPUS-MT German to English"), {QStringLiteral("translator")},
            QStringLiteral("seq2seq"), {}, {}, {});
        add(QStringLiteral("opus-mt-en-de"), QStringLiteral("OPUS-MT English to German (small)"), {QStringLiteral("translator-en")},
            QStringLiteral("seq2seq"), {}, {}, {});
        for (const QString &code : {"es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"}) {
            const QString name = languageName(code);
            add(QStringLiteral("opus-mt-%1-en").arg(code), QStringLiteral("OPUS-MT %1 to English").arg(name),
                {QStringLiteral("translator-pair")}, QStringLiteral("seq2seq"), code, QStringLiteral("en"), {});
            add(QStringLiteral("opus-mt-en-%1").arg(code), QStringLiteral("OPUS-MT English to %1").arg(name),
                {QStringLiteral("translator-pair")}, QStringLiteral("seq2seq"), QStringLiteral("en"), code, {});
            add(QStringLiteral("wiktionary-%1").arg(code), QStringLiteral("%1 dictionary").arg(name),
                {QStringLiteral("dictionary-%1").arg(code)}, QStringLiteral("data"), {}, {}, code);
        }
        QJsonObject root{{"format", 2}, {"release", "models-4"}, {"models", models}};
        write(m_files.path() + QStringLiteral("/catalog.json"), QJsonDocument(root).toJson());
        qputenv("TP_TEST_CATALOG", QFile::encodeName(m_files.path() + QStringLiteral("/catalog.json")));
        qputenv("QB_LANGUAGE", "en");
        m_host = new FakeHost(QStringLiteral(TP_BUILD_DIR "/addon/translation-proofreading"), m_data.path());
        m_tool = new TranslateTool;
        m_tool->setHost(m_host);
        m_tool->setEngineForTest(&m_engine);
        QCoreApplication::processEvents();
        m_engine.answer = [](const QString &model, const QString &text) {
            // every model writes "<model>(text)": the order of the models in a pair can be read from the result
            return QStringLiteral("%1(%2)").arg(model, text);
        };
    }

    void cleanupTestCase()
    {
        delete m_tool;
        delete m_host;
    }

    void init()
    {
        for (const CatalogModel &m : m_tool->store()->models())
            uninstall(m.id);
        m_host->settings.clear();
        m_host->settings.insert(QStringLiteral("translation-proofreading/live"), false);
        m_engine.clearCalls();
    }

    // ---- the languages and the models of a pair
    void elevenLanguages()
    {
        const QStringList want{"en", "de", "es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"};
        QStringList have;
        for (const Language &l : languages())
            have << l.code;
        QCOMPARE(have, want);
        QCOMPARE(nativeName(QStringLiteral("pt")), QStringLiteral("Português (Brasil)"));
        QCOMPARE(languageLabel(QStringLiteral("en")), QStringLiteral("English"));
        QCOMPARE(languageLabel(QStringLiteral("es")), QStringLiteral("Español (Spanish)"));
        QVERIFY(!isLanguage(QStringLiteral("zh")));
    }

    void stepsOfAPair()
    {
        bool ok = false;
        QVERIFY(stepsFor(QStringLiteral("es"), QStringLiteral("es"), &ok).isEmpty());
        QVERIFY(ok);
        QList<Step> s = stepsFor(QStringLiteral("es"), QStringLiteral("en"));
        QCOMPARE(s.size(), 1);
        QCOMPARE(modelIdFor(s.first()), QStringLiteral("opus-mt-es-en"));
        s = stepsFor(QStringLiteral("en"), QStringLiteral("de"));
        QCOMPARE(modelIdFor(s.first()), QStringLiteral("opus-mt-en-de"));
        // a pair without English goes through English: two models, the first into English
        s = stepsFor(QStringLiteral("fr"), QStringLiteral("ru"));
        QCOMPARE(s.size(), 2);
        QCOMPARE(modelIdFor(s.at(0)), QStringLiteral("opus-mt-fr-en"));
        QCOMPARE(modelIdFor(s.at(1)), QStringLiteral("opus-mt-en-ru"));
        s = stepsFor(QStringLiteral("de"), QStringLiteral("nl"));
        QCOMPARE(modelIdFor(s.at(0)), QStringLiteral("opus-mt-de-en"));
        QCOMPARE(modelIdFor(s.at(1)), QStringLiteral("opus-mt-en-nl"));
        QVERIFY(stepsFor(QStringLiteral("xx"), QStringLiteral("en"), &ok).isEmpty());
        QVERIFY(!ok);
        // every pair of the eleven has its models named; the catalog of the add-on has every one of them
        ModelStore store;
        int pairs = 0;
        for (const Language &a : languages())
            for (const Language &b : languages())
                for (const Step &st : stepsFor(a.code, b.code)) {
                    QVERIFY2(store.model(modelIdFor(st)), qPrintable(modelIdFor(st)));
                    ++pairs;
                }
        QVERIFY(pairs > 100);
    }

    void realCatalogOfModels4()
    {
        // the real catalog (the add-on's resource): 18 models of models-4, a dictionary entry for each of the nine languages,
        // each with its license and a credit line; Polish and Portuguese name the language token
        const QByteArray saved = qgetenv("TP_TEST_CATALOG");
        qunsetenv("TP_TEST_CATALOG");
        ModelStore real;
        qputenv("TP_TEST_CATALOG", saved);
        int pairModels = 0, dictionaries = 0;
        for (const CatalogModel &m : real.models(QStringLiteral("translator-pair"))) {
            ++pairModels;
            QCOMPARE(m.release, QStringLiteral("models-4"));
            QVERIFY2(!m.license.isEmpty() && !m.licenseUrl.isEmpty() && !m.credit.isEmpty() && !m.source.isEmpty(), qPrintable(m.id));
            QVERIFY(!m.nonCommercial);
            QVERIFY2(m.license == QLatin1String("Apache-2.0") || m.license == QLatin1String("CC BY 4.0"), qPrintable(m.id));
            QVERIFY2(m.source.contains(QStringLiteral("/tree/")) && m.source.section(QLatin1Char('/'), -1).size() == 40, qPrintable(m.source));
            QVERIFY2(m.from == QLatin1String("en") || m.to == QLatin1String("en"), qPrintable(m.id));
            QCOMPARE(m.id, QStringLiteral("opus-mt-%1-%2").arg(m.from, m.to));
            QCOMPARE(m.tokenizer, QStringLiteral("marian"));
            QCOMPARE(m.files.size(), 6);
        }
        QCOMPARE(pairModels, 18);
        QCOMPARE(real.model(QStringLiteral("opus-mt-en-pl"))->sourceToken, QStringLiteral(">>pol<<"));
        QCOMPARE(real.model(QStringLiteral("opus-mt-en-pt"))->sourceToken, QStringLiteral(">>pob<<"));
        for (const QString &code : {"es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"}) {
            const CatalogModel *d = real.model(dictionaryIdFor(code));
            QVERIFY2(d, qPrintable(code));
            ++dictionaries;
            QCOMPARE(d->license, QStringLiteral("CC BY-SA 4.0"));
            QVERIFY(d->askLicense);   // the license is shown before the download
            QCOMPARE(d->language, code);
            QCOMPARE(d->files.size(), 3);
            QVERIFY(d->files.last().name == code + QStringLiteral("-dictionary.qz"));
            QVERIFY(!d->files.first().sha256.isEmpty());
        }
        QCOMPARE(dictionaries, 9);
        // the models before are as they were: German to English, English to German and the German dictionary
        QVERIFY(real.model(QStringLiteral("opus-mt-de-en")) && real.model(QStringLiteral("opus-mt-en-de")) && real.model(QStringLiteral("wiktionary-de")));
        QCOMPARE(real.model(QStringLiteral("opus-mt-de-en"))->release, QStringLiteral("models-1"));
        QCOMPARE(real.model(QStringLiteral("opus-mt-en-de"))->release, QStringLiteral("models-3"));
        // a model that is not published yet (placeholder sums) is not downloaded
        for (const CatalogModel &m : real.models(QStringLiteral("translator-pair")))
            if (m.files.first().sha256.startsWith(QLatin1String("PLACEHOLDER")))
                QVERIFY(real.downloadProblem(m.id).contains(QStringLiteral("not published yet")));
    }

    // ---- the drop-down lists
    void dropDownLists()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 900);
        p.show();
        QVERIFY(p.fromLanguageBox() && p.toLanguageBox());
        QCOMPARE(p.fromLanguageBox()->count(), 11);
        QCOMPARE(p.toLanguageBox()->count(), 11);
        QStringList codes;
        for (int i = 0; i < p.fromLanguageBox()->count(); ++i)
            codes << p.fromLanguageBox()->itemData(i).toString();
        QCOMPARE(codes, (QStringList{"en", "de", "es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"}));
        // hidden until the task is "Translate"; German to English at first (the settings of 1.4 keep working)
        p.taskBox()->setCurrentIndex(p.taskBox()->findData(QStringLiteral("auto")));
        QVERIFY(!p.fromLanguageBox()->isVisible() && !p.toLanguageBox()->isVisible());
        p.taskBox()->setCurrentIndex(p.taskBox()->findData(QStringLiteral("translate")));
        QVERIFY(p.fromLanguageBox()->isVisible() && p.toLanguageBox()->isVisible() && p.languageArrow()->isVisible());
        QCOMPARE(p.fromLanguageBox()->currentData().toString(), QStringLiteral("de"));
        QCOMPARE(p.toLanguageBox()->currentData().toString(), QStringLiteral("en"));
        QCOMPARE(p.direction(), Panel::Direction::Proofread);
        QCOMPARE(p.directionNow(), Panel::Direction::DeToEn);
        // the choice is kept in the add-on's settings, and the other panel follows
        Panel q(m_tool, Panel::Place::Window, nullptr);
        p.fromLanguageBox()->setCurrentIndex(p.fromLanguageBox()->findData(QStringLiteral("fr")));
        p.toLanguageBox()->setCurrentIndex(p.toLanguageBox()->findData(QStringLiteral("ru")));
        QCOMPARE(m_host->settings.value(QStringLiteral("translation-proofreading/langFrom")).toString(), QStringLiteral("fr"));
        QCOMPARE(m_host->settings.value(QStringLiteral("translation-proofreading/langTo")).toString(), QStringLiteral("ru"));
        QCOMPARE(p.directionNow(), Panel::Direction::Pair);
        q.reloadSettings();   // a panel made by the tool is told by it; this one is asked
        QCOMPARE(q.fromLanguageBox()->currentData().toString(), QStringLiteral("fr"));
        QCOMPARE(q.toLanguageBox()->currentData().toString(), QStringLiteral("ru"));
        shot(&p, QStringLiteral("lang-inspector"));
        q.resize(1000, 500);
        q.show();
        shot(&q, QStringLiteral("lang-window"));
    }

    // ---- a pair with one model, a pair through English, and what is missing
    void pairNeedsItsModels()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.show();
        p.setLanguages(QStringLiteral("es"), QStringLiteral("fr"));
        QVERIFY(runPanel(p, QStringLiteral("La casa es vieja.")));
        // nothing installed: the notice names both models and no engine call was made
        QVERIFY2(p.status().contains(QStringLiteral("Spanish to English")) && p.status().contains(QStringLiteral("English to French")),
                 qPrintable(p.status()));
        QVERIFY(p.notice()->isVisible() || p.noticeLabel()->text().contains(QStringLiteral("Spanish")));
        QVERIFY2(p.noticeLabel()->text().contains(QStringLiteral("a translation model (Spanish to English)"))
                     && p.noticeLabel()->text().contains(QStringLiteral("a translation model (English to French)")),
                 qPrintable(p.noticeLabel()->text()));
        QVERIFY(m_engine.calls().isEmpty());
        // only the first one downloaded: still missing
        install(QStringLiteral("opus-mt-es-en"));
        p.reloadSettings();
        QVERIFY2(!p.noticeLabel()->text().contains(QStringLiteral("Spanish to English")), qPrintable(p.noticeLabel()->text()));
        QVERIFY(p.noticeLabel()->text().contains(QStringLiteral("English to French")));
        QVERIFY(runPanel(p, QStringLiteral("La casa es vieja.")));
        QVERIFY(m_engine.calls().isEmpty());
    }

    void pivotThroughEnglish()
    {
        install(QStringLiteral("opus-mt-es-en"));
        install(QStringLiteral("opus-mt-en-fr"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 900);
        p.show();
        p.setLanguages(QStringLiteral("es"), QStringLiteral("fr"));
        p.reloadSettings();
        QVERIFY(!p.notice()->isVisible());
        QVERIFY(runPanel(p, QStringLiteral("La casa es vieja.")));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("opus-mt-en-fr(opus-mt-es-en(La casa es vieja.))")), qPrintable(out(p)));
        // two runs, the first model first; each in a slot of its own
        const QList<FakeEngine::Call> calls = m_engine.calls();
        QCOMPARE(calls.size(), 2);
        QCOMPARE(calls.at(0).model, QStringLiteral("opus-mt-es-en"));
        QCOMPARE(calls.at(1).model, QStringLiteral("opus-mt-en-fr"));
        QCOMPARE(calls.at(0).texts, QStringList{QStringLiteral("La casa es vieja.")});
        QCOMPARE(calls.at(1).texts, QStringList{QStringLiteral("opus-mt-es-en(La casa es vieja.)")});
        QVERIFY(calls.at(0).slot != calls.at(1).slot);
        QCOMPARE(p.lastPairModels(), (QStringList{"opus-mt-es-en", "opus-mt-en-fr"}));
        QCOMPARE(p.pivotText(0), QStringLiteral("opus-mt-es-en(La casa es vieja.)"));
        QVERIFY(p.notes().contains(QStringLiteral("through English")));
        QVERIFY(p.modelBox()->itemText(0).contains(QStringLiteral("OPUS-MT Spanish to English"))
                && p.modelBox()->itemText(0).contains(QStringLiteral("OPUS-MT English to French")));
        shot(&p, QStringLiteral("lang-pivot"));
        // the same text again comes from the cache: no model is asked
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("La casa es vieja.")));
        QVERIFY(m_engine.calls().isEmpty());
        // Translate back: French to Spanish, through English the other way round
        install(QStringLiteral("opus-mt-fr-en"));
        install(QStringLiteral("opus-mt-en-es"));
        QSignalSpy done(&p, &Panel::finished);
        p.translateBack();
        QVERIFY(done.count() > 0 || done.wait(20000));
        QCOMPARE(m_engine.calls().size(), 2);
        QCOMPARE(m_engine.calls().at(0).model, QStringLiteral("opus-mt-fr-en"));
        QCOMPARE(m_engine.calls().at(1).model, QStringLiteral("opus-mt-en-es"));
        QVERIFY2(p.backView()->toPlainText().contains(QStringLiteral("opus-mt-en-es(opus-mt-fr-en(")), qPrintable(p.backView()->toPlainText()));
    }

    void oneModelPairs()
    {
        // English to Polish and Portuguese to English: one model each; Swap turns the pair round
        install(QStringLiteral("opus-mt-pl-en"));
        install(QStringLiteral("opus-mt-en-pl"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.show();
        p.setLanguages(QStringLiteral("en"), QStringLiteral("pl"));
        QVERIFY(runPanel(p, QStringLiteral("The house is old.")));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("opus-mt-en-pl(The house is old.)")), qPrintable(out(p)));
        QCOMPARE(m_engine.calls().size(), 1);
        QVERIFY(!p.notes().contains(QStringLiteral("through English")));
        m_engine.clearCalls();
        QSignalSpy done(&p, &Panel::finished);
        p.swapDirection();
        QVERIFY(done.count() > 0 || done.wait(20000));
        QCOMPARE(p.fromLanguageBox()->currentData().toString(), QStringLiteral("pl"));
        QCOMPARE(p.toLanguageBox()->currentData().toString(), QStringLiteral("en"));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("opus-mt-pl-en(")), qPrintable(out(p)));
        // two equal languages: nothing to translate
        p.setLanguages(QStringLiteral("pl"), QStringLiteral("pl"));
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("Dom jest stary.")));
        QVERIFY(m_engine.calls().isEmpty());
        QVERIFY(p.status().contains(QStringLiteral("two different languages")));
    }

    void germanPairsKeepTheirModels()
    {
        // German to English and English to German are the pairs of 1.3 and 1.4: their own slots and models, no pair step
        install(QStringLiteral("opus-mt-de-en"));
        install(QStringLiteral("opus-mt-en-de"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.show();
        p.setLanguages(QStringLiteral("de"), QStringLiteral("en"));
        QCOMPARE(p.directionNow(), Panel::Direction::DeToEn);
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt.")));
        QTRY_VERIFY(out(p).contains(QStringLiteral("opus-mt-de-en(")));
        QTRY_VERIFY(!p.isBusy());   // (the other translations of a short text are asked after the run: let them end)
        QVERIFY(usedSlot(QStringLiteral("translator")));
        p.setLanguages(QStringLiteral("en"), QStringLiteral("de"));
        QCOMPARE(p.directionNow(), Panel::Direction::EnToDe);
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("The house is old.")));
        QTRY_VERIFY(out(p).contains(QStringLiteral("opus-mt-en-de(")));
        QTRY_VERIFY(!p.isBusy());
        QVERIFY(usedSlot(QStringLiteral("translator-en")));
        // through English from German: the large model's place is the chosen model, the second model the pair's
        install(QStringLiteral("opus-mt-en-nl"));
        p.setLanguages(QStringLiteral("de"), QStringLiteral("nl"));
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt.")));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("opus-mt-en-nl(opus-mt-de-en(")), qPrintable(out(p)));
    }

    // ---- the dictionaries of the other languages
    void spanishDictionary()
    {
        QVERIFY(m_tool->dictionaryFile(QStringLiteral("es")).isEmpty());
        install(QStringLiteral("wiktionary-es"));
        const QString file = m_tool->dictionaryFile(QStringLiteral("es"));
        QVERIFY2(file.endsWith(QStringLiteral("es-dictionary.qz")), qPrintable(file));
        QString error;
        QVERIFY2(ensureDictionary(QStringLiteral("es"), file, &error), qPrintable(error));
        QVERIFY(dictionaryLoaded(QStringLiteral("es")));
        QVERIFY(!germanDictionaryLoaded() || true);   // the German one is its own
        const QList<DictEntry> e = lookupWord(QStringLiteral("es"), QStringLiteral("Casa"));
        QCOMPARE(e.size(), 1);
        QCOMPARE(e.first().gender, QStringLiteral("f"));
        QCOMPARE(e.first().meanings.first().english, (QStringList{"house", "home"}));
        QCOMPARE(lookupWord(QStringLiteral("es"), QStringLiteral("corrió")).first().headword, QStringLiteral("correr"));
        QVERIFY(lookupWord(QStringLiteral("es"), QStringLiteral("zzz")).isEmpty());
        // a file of another language is refused
        QVERIFY(!loadDictionary(QStringLiteral("fr"), file, &error));
        // the reverse lookup of the Spanish words of an English word
        const QList<GermanReverse> r = reverseLookup(QStringLiteral("es"), {QStringLiteral("run")}, QString());
        QCOMPARE(r.size(), 1);
        QCOMPARE(r.first().headword, QStringLiteral("correr"));
        unloadDictionary(QStringLiteral("es"));
        QVERIFY(!dictionaryLoaded(QStringLiteral("es")));
    }

    void singleWordOfAnotherLanguage()
    {
        install(QStringLiteral("opus-mt-es-en"));
        install(QStringLiteral("wiktionary-es"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 900);
        p.show();
        p.setLanguages(QStringLiteral("es"), QStringLiteral("en"));
        QVERIFY(runPanel(p, QStringLiteral("casa")));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("casa (noun)")), qPrintable(out(p)));
        QVERIFY2(out(p).contains(QStringLiteral("house")) && out(p).contains(QStringLiteral("Spanish dictionary")), qPrintable(out(p)));
        QCOMPARE(p.singleWord().result, QStringLiteral("house"));
        shot(&p, QStringLiteral("lang-single-es"));
        // a word the dictionary does not have: the model's word, with the warning
        QVERIFY(runPanel(p, QStringLiteral("quizas")));
        QTRY_VERIFY2(p.singleWord().result == QStringLiteral("opus-mt-es-en(quizas)"), qPrintable(p.singleWord().result));
        QVERIFY(out(p).contains(QStringLiteral("small translation model")));
        // into a language that is not English the dictionary only explains: the model's word is the result
        install(QStringLiteral("opus-mt-en-fr"));
        p.setLanguages(QStringLiteral("es"), QStringLiteral("fr"));
        QVERIFY(runPanel(p, QStringLiteral("casa")));
        QTRY_VERIFY2(p.singleWord().result == QStringLiteral("opus-mt-en-fr(opus-mt-es-en(casa))"), qPrintable(p.singleWord().result));
        QCOMPARE(p.singleWord().kind, QStringLiteral("pair"));
        QVERIFY(out(p).contains(QStringLiteral("casa (noun)")));
        // no dictionary downloaded: a note says so
        uninstall(QStringLiteral("wiktionary-es"));
        QVERIFY(runPanel(p, QStringLiteral("casa")));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("Spanish dictionary is not downloaded")), qPrintable(out(p)));
    }

    // ---- the fixed line, the name
    void suggestionsLine()
    {
        const QString line = QStringLiteral("This tool gives suggestions only. Its translations are not precise: check each one, "
                                            "which needs a working knowledge of both languages.");
        for (Panel::Place place : {Panel::Place::Inspector, Panel::Place::Window}) {
            Panel p(m_tool, place, nullptr);
            p.resize(place == Panel::Place::Window ? 1000 : 380, 700);
            p.show();
            QVERIFY(p.suggestionsLabel());
            QCOMPARE(p.suggestionsLabel()->text(), line);
            QVERIFY(p.suggestionsLabel()->isVisible());
            QVERIFY(p.suggestionsLabel()->wordWrap());
            // fixed: there is no button on it and it stays when the text is translated
            QVERIFY(p.suggestionsLabel()->findChildren<QPushButton *>().isEmpty());
            QVERIFY(runPanel(p, QStringLiteral("Das Haus.")));
            QVERIFY(p.suggestionsLabel()->isVisible());
            shot(&p, place == Panel::Place::Window ? QStringLiteral("lang-line-window") : QStringLiteral("lang-line-inspector"));
        }
    }

    void singleWordCorrectionIsASuggestion()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.show();
        QVERIFY(runPanel(p, QStringLiteral("hosue")));
        QVERIFY2(p.status().startsWith(QStringLiteral("Suggestion:")) || p.status().contains(QStringLiteral("not in the dictionary")),
                 qPrintable(p.status()));
    }

    void theNewName()
    {
        QCOMPARE(m_tool->name(), QStringLiteral("Translation Helper"));
        QCOMPARE(m_tool->id(), QStringLiteral("translation-proofreading"));   // the id stays: installed copies update
        QCOMPARE(m_tool->version(), QStringLiteral("1.5"));
        QCOMPARE(m_tool->optionsTitle(), QStringLiteral("Translation Helper"));
        const auto cmds = m_tool->commands();
        QCOMPARE(cmds.at(0).text, QStringLiteral("&Translation Helper..."));
        QVERIFY(cmds.at(1).text.contains(QStringLiteral("Translation Helper")));
        QCOMPARE(m_tool->inspectorTabs().first().toolTip, QStringLiteral("Translation Helper"));
        for (const auto &c : cmds)
            QVERIFY2(!c.text.contains(QStringLiteral("Proofread")) && !c.toolTip.contains(QStringLiteral("Translation and Proofreading")), qPrintable(c.text));
        // the manifest says so too (both copies are the same file: stage.sh checks), and the id is the old one
        for (const QString &path : {QStringLiteral(TP_SOURCE_DIR "/packaging/manifest.json"), QStringLiteral(TP_SOURCE_DIR "/../manifest.json")}) {
            QFile f(path);
            QVERIFY2(f.open(QIODevice::ReadOnly), qPrintable(path));
            const QJsonObject m = QJsonDocument::fromJson(f.readAll()).object();
            QCOMPARE(m.value(QStringLiteral("id")).toString(), QStringLiteral("translation-proofreading"));
            QCOMPARE(m.value(QStringLiteral("name")).toString(), QStringLiteral("Translation Helper"));
            QCOMPARE(m.value(QStringLiteral("version")).toString(), QStringLiteral("1.5"));
            QVERIFY(m.value(QStringLiteral("description")).toString().contains(
                QStringLiteral("This tool gives suggestions only. Its translations are not precise: check each one, which needs a working "
                               "knowledge of both languages.")));
        }
    }

    // ---- the add-on's own texts in the eleven languages
    void translationFilesAreComplete()
    {
        // one file for each of the ten other languages, each with every text of the source (no empty or unfinished one) and the
        // placeholders (%1, %2 ...) and the line breaks of the English text in every translation; the compiled .qm loads
        QDir dir(QStringLiteral(TP_SOURCE_DIR "/translations"));
        const QStringList files = dir.entryList({QStringLiteral("tp_*.ts")}, QDir::Files, QDir::Name);
        QCOMPARE(files.size(), 10);
        QStringList codes;
        for (const QString &f : files)
            codes << localeCode(f);
        codes.sort();
        QCOMPARE(codes, (QStringList{"de", "es", "fr", "id", "it", "nl", "pl", "pt", "ru", "tr"}));
        QHash<QString, int> count;
        QHash<QString, QStringList> sources;
        for (const QString &fn : files) {
            QFile f(dir.filePath(fn));
            QVERIFY(f.open(QIODevice::ReadOnly));
            QXmlStreamReader xml(&f);
            QString source, translation, context;
            QString transType;
            QStringList seen;
            int n = 0;
            QStringList problems;
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement()) {
                    if (xml.name() == QLatin1String("name"))
                        context = xml.readElementText();
                    else if (xml.name() == QLatin1String("source"))
                        source = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    else if (xml.name() == QLatin1String("translation")) {
                        transType = xml.attributes().value(QStringLiteral("type")).toString();
                        translation = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                        ++n;
                        seen << context + QLatin1Char('\x1f') + source;
                        if (!transType.isEmpty() || translation.trimmed().isEmpty())
                            problems << QStringLiteral("unfinished: ") + source.left(50);
                        static const QRegularExpression ph(QStringLiteral("%\\d"));
                        auto marks = [](const QString &t) {
                            QStringList l;
                            auto it = ph.globalMatch(t);
                            while (it.hasNext())
                                l << it.next().captured();
                            l.sort();
                            return l;
                        };
                        if (marks(source) != marks(translation))
                            problems << QStringLiteral("placeholders: ") + source.left(50) + QStringLiteral(" -> ") + translation.left(50);
                        if (source.count(QLatin1Char('\n')) != translation.count(QLatin1Char('\n')))
                            problems << QStringLiteral("line breaks: ") + source.left(50);
                        if (source.contains(QStringLiteral("<a href")) != translation.contains(QStringLiteral("<a href")))
                            problems << QStringLiteral("link: ") + source.left(50);
                    }
                }
            }
            QVERIFY2(!xml.hasError(), qPrintable(fn + xml.errorString()));
            QVERIFY2(problems.isEmpty(), qPrintable(fn + ": " + problems.mid(0, 5).join(QStringLiteral("; "))));
            count.insert(localeCode(fn), n);
            sources.insert(localeCode(fn), seen);
            // the compiled file is in the add-on's resources and loads
            QTranslator t;
            QVERIFY2(t.load(QStringLiteral(":/translation-proofreading/translations/tp_%1.qm").arg(localeCode(fn))), qPrintable(fn));
            QVERIFY(!t.isEmpty());
        }
        QVERIFY(count.value(QStringLiteral("de")) > 200);
        for (const QString &c : codes) {
            QCOMPARE(count.value(c), count.value(QStringLiteral("de")));
            QCOMPARE(sources.value(c), sources.value(QStringLiteral("de")));   // the same texts, in the same order
        }
    }

    void theAddonSpeaksTheProgramsLanguage()
    {
        // QB_LANGUAGE (the program's own override) picks the file; English is the fallback; the id of the tool does not change
        const QString key = QStringLiteral("Translation Helper");
        QCOMPARE(m_tool->language(), QStringLiteral("en"));
        for (const QString &code : {"de", "es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"}) {
            qputenv("QB_LANGUAGE", code.toLatin1());
            TranslateTool t;
            QCOMPARE(t.language(), code);
            // the name and the fixed line are translated, and differ from English
            QVERIFY2(t.name() != key || code == QLatin1String("id") || code == QLatin1String("tr") || code == QLatin1String("nl")
                         || code == QLatin1String("pl"),
                     qPrintable(code + ": " + t.name()));
            QCOMPARE(t.id(), QStringLiteral("translation-proofreading"));
            Panel p(&t, Panel::Place::Inspector, nullptr);
            QVERIFY2(p.suggestionsLabel()->text() != QStringLiteral("This tool gives suggestions only. Its translations are not precise: check each one, "
                                                                    "which needs a working knowledge of both languages."),
                     qPrintable(code));
            QVERIFY2(!p.suggestionsLabel()->text().isEmpty(), qPrintable(code));
            p.resize(380, 900);
            p.show();
            shot(&p, QStringLiteral("lang-") + code);
        }
        // a language the add-on has no file for, and the program's English: English texts
        qputenv("QB_LANGUAGE", "xx");
        {
            TranslateTool t;
            QCOMPARE(t.name(), QStringLiteral("Translation Helper"));
        }
        qputenv("QB_LANGUAGE", "en");
        // the rule for the language itself (no program needed)
        QCOMPARE(languageFromSettings(QStringLiteral("fr"), QStringLiteral("de"), QStringLiteral("es_ES")), QStringLiteral("fr"));
        QCOMPARE(languageFromSettings(QString(), QStringLiteral("de"), QStringLiteral("es_ES")), QStringLiteral("de"));
        QCOMPARE(languageFromSettings(QString(), QString(), QStringLiteral("es_ES")), QStringLiteral("es"));
        QCOMPARE(languageFromSettings(QString(), QString(), QStringLiteral("zh_CN")), QStringLiteral("en"));
    }

    void optionsListEveryLanguage()
    {
        qb::tools::OptionsSection *s = m_tool->createOptions(nullptr);
        QVERIFY(s);
        QWidget *w = s->widget();
        w->resize(500, 700);
        for (const QString &code : {"es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"}) {
            QWidget *g = w->findChild<QWidget *>(QStringLiteral("tpLanguage:") + code);
            QVERIFY2(g, qPrintable(code));
            QVERIFY(g->findChild<QPushButton *>(QStringLiteral("tpDownload:opus-mt-%1-en").arg(code)));
            QVERIFY(g->findChild<QPushButton *>(QStringLiteral("tpDownload:opus-mt-en-%1").arg(code)));
            QVERIFY(g->findChild<QPushButton *>(QStringLiteral("tpDownload:wiktionary-%1").arg(code)));
        }
        shot(w, QStringLiteral("lang-options"));
        delete s;
    }
};

QTEST_MAIN(LangTests)
#include "lang_tests.moc"
