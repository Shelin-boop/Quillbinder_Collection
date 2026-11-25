// Checks of 1.3 without downloaded models: a word on its own (what it means, the German dictionary, the small model in
// a frame), Swap and Translate back, the second row in translate mode, up to three words never to the large model,
// other translations of a short text, the word pane. A stand-in engine (FakeEngine.h) and stand-in models (small
// files in a catalog of the check) take the place of the real ones; the dictionaries are small files of the check.
// Screenshots go into TP_SHOTS (default: build/screenshots).
#include "Dictionary.h"
#include "FakeEngine.h"
#include "FakeHost.h"
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
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextBrowser>
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

const QByteArray kGermanDictionary =
    "#qb-de-dictionary 1\n"
    "eid\tL\tEid\tnoun\tm\toath^a solemn promise\n"
    "fluch\tL\tFluch\tnoun\tm\tcurse; swear word^a profane word|spell^a magical curse\n"
    "fluchen\tL\tfluchen\tverb\t\tto curse; to swear^to use offensive language|to curse^to wish evil on someone\n"
    "fluchte\tF\tfluchen\tverb\n"
    "haus\tL\thausen\tverb\t\tto dwell^to live somewhere\n"
    "haus\tL\tHaus\tnoun\tn\thouse; home^a building to live in|building^a structure|a1; a2^x|b1^x|c1^x|d1^x|e1^x|f1^x\n"
    "kraftausdruck\tL\tKraftausdruck\tnoun\tm\texpletive^a strong word\n"
    "lästern\tL\tlästern\tverb\t\tto blaspheme^to speak against\n"
    "zwirbel\tL\tZwirbel\tnoun\tm\t^something with no English word listed\n";

// WordNet's definitions in the form of resources/en-glosses.tsv: "curse" has a noun and verb meanings, "zork" none
const QByteArray kGlosses =
    "# test\n=0\tan expression of a wish that evil may befall someone\n=1\tto utter a curse\n=2\tto use offensive language\n"
    "curse\tn:0|v:1,2\n";

const QByteArray kWords = "curse\nzork\nwalk\nhouse\nold\nthe\nis\nbeautiful\nhandsome\nhome\ndomicile\nnumber\n";

} // namespace

class WordTests : public QObject
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
        QVERIFY(m);
        for (const CatalogFile &f : m->files) {
            QByteArray content = QByteArray(int(f.size), 'x');
            if (f.name == QLatin1String("de-dictionary.qz"))
                content = kGermanDictionary;
            write(m_tool->store()->filePath(id, f.name), content);
        }
        write(m_tool->store()->filePath(id, QStringLiteral("checked.txt")), "checked\n");
    }
    void uninstall(const QString &id) { QDir(m_tool->store()->modelsDir() + QLatin1Char('/') + id).removeRecursively(); }
    void select(Panel &p, const QString &task)
    {
        p.taskBox()->setCurrentIndex(p.taskBox()->findData(task));
        p.reloadSettings();
    }
    bool runPanel(Panel &p, const QString &text)
    {
        QSignalSpy done(&p, &Panel::finished);
        p.setInput(text);
        p.run();
        return done.count() > 0 || done.wait(20000);
    }
    QString out(Panel &p) { return p.output()->toPlainText(); }
    QStringList modelsCalled() const
    {
        QStringList l;
        for (const FakeEngine::Call &c : m_engine.calls())
            l << c.model;
        return l;
    }

private slots:
    void initTestCase()
    {
        g_shots = qEnvironmentVariable("TP_SHOTS", QStringLiteral(TP_BUILD_DIR "/screenshots"));
        write(m_files.path() + QStringLiteral("/glosses.tsv"), kGlosses);
        write(m_files.path() + QStringLiteral("/words.txt"), kWords);
        loadGlosses(m_files.path() + QStringLiteral("/glosses.tsv"));
        loadWordList(m_files.path() + QStringLiteral("/words.txt"));
        // the stand-in models: a catalog of the check, files of the declared size
        auto model = [](const QString &id, const QString &title, const QStringList &kinds, const QString &type, bool forWords) {
            QJsonObject o{{"id", id}, {"title", title}, {"kinds", QJsonArray::fromStringList(kinds)}, {"type", type},
                          {"license", "x"}, {"licenseUrl", "https://example.invalid"}, {"credit", "c"},
                          {"release", "models-3"}, {"memoryMB", type == QLatin1String("data") ? 0 : 10},
                          {"forWords", forWords}, {"sizeMB", 1},
                          {"tokenizer", QJsonObject{{"type", "marian"}, {"source", "s.spm"}, {"target", "t.spm"}}}};
            const QString name = type == QLatin1String("data") ? QStringLiteral("de-dictionary.qz") : QStringLiteral("model.bin");
            o["files"] = QJsonArray{QJsonObject{{"name", name}, {"asset", id + "--" + name},
                                                {"size", type == QLatin1String("data") ? kGermanDictionary.size() : 4},
                                                {"sha256", "0"}}};
            return o;
        };
        QJsonObject root{{"format", 2}, {"release", "models-3"},
                         {"models", QJsonArray{model("opus-mt-de-en", "OPUS-MT German to English", {"translator"}, "seq2seq", true),
                                               model("opus-mt-big-de-en", "OPUS-MT Big", {"translator"}, "seq2seq", false),
                                               model("opus-mt-en-de", "OPUS-MT English to German (small)", {"translator-en"},
                                                     "seq2seq", true),
                                               model("qwen3-1.7b", "Qwen3 1.7B", {"corrector", "translator"}, "chat", true),
                                               model("qwen3-4b", "Qwen3 4B", {"corrector", "translator"}, "chat", true),
                                               model("wiktionary-de", "German dictionary", {"dictionary"}, "data", true)}}};
        write(m_files.path() + QStringLiteral("/catalog.json"), QJsonDocument(root).toJson());
        qputenv("TP_TEST_CATALOG", QFile::encodeName(m_files.path() + QStringLiteral("/catalog.json")));
        m_host = new FakeHost(QStringLiteral(TP_BUILD_DIR "/addon/translation-proofreading"), m_data.path());
        m_host->settings.insert(QStringLiteral("translation-proofreading/live"), false);
        m_tool = new TranslateTool;
        m_tool->setHost(m_host);
        m_tool->setEngineForTest(&m_engine);
        QCoreApplication::processEvents();
        m_engine.answer = [](const QString &model, const QString &text) {
            static const QHash<QString, QString> table = {
                {"zu fluchen", "to curse"},        {"der Zwirbel", "the twirl"},   {"Quatschkopf", "Blabbermouth"},
                {"Das Haus ist alt", "The house is old"}, {"The house is old", "Das Haus ist alt"},
                {"to curse", "zu fluchen"},         {"the curse", "der Fluch"},    {"Das ist schön", "That is beautiful"},
                {"Das ist ein Haus", "That is a house"}};
            if (text.startsWith(QStringLiteral("What does the English word")))
                return QStringLiteral("A made-up word [%1].").arg(model);
            return table.value(text, QStringLiteral("<%1>").arg(text));
        };
    }

    void cleanupTestCase()
    {
        delete m_tool;
        delete m_host;
    }

    void init()
    {
        for (const QString &id : {"opus-mt-de-en", "opus-mt-big-de-en", "opus-mt-en-de", "qwen3-1.7b", "qwen3-4b", "wiktionary-de"})
            uninstall(id);
        m_host->settings.clear();
        m_host->settings.insert(QStringLiteral("translation-proofreading/live"), false);
        m_engine.clearCalls();
        m_engine.variantList.clear();
    }

    // a: one English word says what it means (WordNet), also with nothing installed
    void englishWordMeaning()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        QVERIFY(runPanel(p, QStringLiteral("curse")));
        const QString o = out(p);
        QVERIFY2(o.contains(QStringLiteral("Meaning (noun):")) && o.contains(QStringLiteral("Meaning (verb):")), qPrintable(o));
        QVERIFY(o.contains(QStringLiteral("an expression of a wish that evil may befall someone")));
        QVERIFY(o.contains(QStringLiteral("WordNet")));
        QVERIFY(o.contains(QStringLiteral("Similar (")));
        QVERIFY(m_engine.calls().isEmpty());
        shot(&p, QStringLiteral("word-english-meaning"));
    }

    // a: no WordNet entry: nothing installed says what can be downloaded; with a chat model it writes the meaning, with a warning
    void englishWordWithoutEntry()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        QVERIFY(runPanel(p, QStringLiteral("zork")));
        QString o = out(p);
        QVERIFY2(o.contains(QStringLiteral("no meaning")) && o.contains(QStringLiteral("Qwen3 1.7B or Qwen3 4B")), qPrintable(o));
        QVERIFY(!o.contains(QStringLiteral("Meaning (")));
        QVERIFY(m_engine.calls().isEmpty());
        // the small chat model writes it
        install(QStringLiteral("qwen3-1.7b"));
        QVERIFY(runPanel(p, QStringLiteral("zork")));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("A made-up word [qwen3-1.7b].")), qPrintable(out(p)));
        QVERIFY(out(p).contains(QStringLiteral("may be inaccurate")));
        // the larger one is taken when it is there
        install(QStringLiteral("qwen3-4b"));
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("zork")));
        QTRY_VERIFY2(out(p).contains(QStringLiteral("[qwen3-4b]")), qPrintable(out(p)));
        shot(&p, QStringLiteral("word-english-no-entry"));
    }

    // d: "fluchen": verb, "to curse, to swear", the note, Similar only with verb senses; the engine is not asked
    void germanWordFromDictionary()
    {
        install(QStringLiteral("wiktionary-de"));
        install(QStringLiteral("opus-mt-de-en"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translate"));
        QVERIFY(runPanel(p, QStringLiteral("fluchen")));
        QTRY_VERIFY(!p.isBusy());
        const QString o = out(p);
        QVERIFY2(o.contains(QStringLiteral("fluchen (verb)")), qPrintable(o));
        QVERIFY2(o.contains(QStringLiteral("1. to curse, to swear")), qPrintable(o));
        QVERIFY(o.contains(QStringLiteral("From the Wiktionary (CC BY-SA 4.0)")));
        QCOMPARE(p.resultText(), QStringLiteral("to curse"));
        QVERIFY2(o.contains(QStringLiteral("Similar translations (verb")) && !o.contains(QStringLiteral("Similar translations (noun")), qPrintable(o));
        QVERIFY(!o.contains(QStringLiteral("Other translations")));
        QVERIFY(!o.contains(QStringLiteral("Warning")));
        QVERIFY(m_engine.calls().isEmpty());
        // an inflected form leads to the base form
        QVERIFY(runPanel(p, QStringLiteral("fluchte")));
        QVERIFY(out(p).contains(QStringLiteral("fluchen (verb)")));
        // several classes, the first five meanings, the rest behind "more"
        QVERIFY(runPanel(p, QStringLiteral("Haus")));
        QVERIFY2(out(p).contains(QStringLiteral("more (3)")), qPrintable(out(p)));
        QVERIFY(!out(p).contains(QStringLiteral("6. ")));
        QVERIFY(out(p).contains(QStringLiteral("hausen (verb)")));
        shot(&p, QStringLiteral("word-german-dictionary"));
        // a click on "more" shows the rest; a click on an English word makes it the result
        p.output()->anchorClicked(QUrl(QStringLiteral("tp-more:c0")));
        QTRY_VERIFY(out(p).contains(QStringLiteral("8. ")));
        p.output()->anchorClicked(QUrl(QStringLiteral("tp-pick:noun:home")));
        QTRY_COMPARE(p.resultText(), QStringLiteral("home"));
        // a word with no English word: its class is known, the small model translates it in its frame, with the warning
        QVERIFY(runPanel(p, QStringLiteral("Zwirbel")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(m_engine.calls().size(), 1);
        QCOMPARE(m_engine.calls().first().texts, QStringList{QStringLiteral("der Zwirbel")});
        QCOMPARE(m_engine.calls().first().model, QStringLiteral("opus-mt-de-en"));
        QCOMPARE(p.resultText(), QStringLiteral("the twirl").mid(4));   // the article of the frame is cut off
        QVERIFY(out(p).contains(QStringLiteral("Warning")));
    }

    // d: a word that is not in the dictionary: the small model alone, with the warning
    void germanWordNotInDictionary()
    {
        install(QStringLiteral("wiktionary-de"));
        install(QStringLiteral("opus-mt-de-en"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translate"));
        QVERIFY(runPanel(p, QStringLiteral("Quatschkopf")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(m_engine.calls().size(), 1);
        // 1.4: a word the dictionary lacks is translated in both forms (the form as typed first)
        QCOMPARE(m_engine.calls().first().texts, (QStringList{QStringLiteral("Quatschkopf"), QStringLiteral("quatschkopf")}));
        QVERIFY2(out(p).contains(QStringLiteral("blabbermouth")), qPrintable(p.status() + "|" + out(p)));
        QCOMPARE(p.resultText(), QStringLiteral("blabbermouth"));
        QVERIFY2(out(p).contains(QStringLiteral("may be inaccurate")), qPrintable(out(p)));
        QVERIFY(!out(p).contains(QStringLiteral("Other translations")));
        // without the dictionary the writer is told where to get it
        uninstall(QStringLiteral("wiktionary-de"));
        QVERIFY(runPanel(p, QStringLiteral("fluchen")));
        QTRY_VERIFY(!p.isBusy());
        QVERIFY2(out(p).contains(QStringLiteral("dictionary is not downloaded")), qPrintable(out(p)));
        shot(&p, QStringLiteral("word-german-not-in-dictionary"));
    }

    // d: English to German for one English word: WordNet's classes as frames, the reverse small model
    void englishToGermanWord()
    {
        install(QStringLiteral("opus-mt-en-de"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translateEn"));
        QVERIFY(runPanel(p, QStringLiteral("curse")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(m_engine.calls().size(), 1);
        QCOMPARE(m_engine.calls().first().texts, (QStringList{QStringLiteral("the curse"), QStringLiteral("to curse")}));
        QCOMPARE(m_engine.calls().first().model, QStringLiteral("opus-mt-en-de"));
        QCOMPARE(p.resultText(), QStringLiteral("der Fluch"));
        QVERIFY(out(p).contains(QStringLiteral("fluchen")));   // "zu fluchen" lost its "zu"
        QVERIFY(out(p).contains(QStringLiteral("Meaning (verb):")));
        shot(&p, QStringLiteral("word-english-to-german"));
    }

    // b: the word pane has no line of the model's other words; Spelling and Similar stay
    void wordPane()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 900);
        p.show();
        p.setInput(QStringLiteral("She walked to the window and looked at the streets bellow."));
        const int pos = p.inputText().indexOf(QStringLiteral("bellow")) + 2;
        QTextCursor c(p.input()->document());
        c.setPosition(pos);
        QTest::mouseDClick(p.input()->viewport(), Qt::LeftButton, {}, p.input()->cursorRect(c).center());
        QTRY_VERIFY(p.info()->toPlainText().contains(QStringLiteral("bellow")));
        const QString info = p.info()->toPlainText();
        QVERIFY(info.contains(QStringLiteral("Spelling:")));
        QVERIFY(info.contains(QStringLiteral("Similar (verb")));
        QVERIFY(!info.contains(QStringLiteral("other words here")));
        QVERIFY(!info.contains(QStringLiteral("Asking the model")));
        QVERIFY(m_engine.calls().isEmpty());
    }

    // c: Swap and Translate back with the stand-in pair
    void swapAndBack()
    {
        install(QStringLiteral("opus-mt-de-en"));
        install(QStringLiteral("opus-mt-en-de"));
        Panel p(m_tool, Panel::Place::Window, nullptr);
        p.resize(1000, 600);
        p.show();
        select(p, QStringLiteral("translate"));
        // four words: not a short text (one sentence: the variants are asked too)
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt")));
        QCOMPARE(p.resultText(), QStringLiteral("The house is old"));
        QVERIFY(p.backButton()->isEnabled());
        // Translate back: shown read-only under the result, cleared when the input changes
        p.backButton()->click();
        QTRY_VERIFY(p.backView()->isVisible());
        QTRY_VERIFY2(p.backView()->toPlainText().contains(QStringLiteral("Das Haus ist alt")), qPrintable(p.backView()->toPlainText()));
        QVERIFY(p.backView()->isReadOnly());
        QTRY_VERIFY(!p.isBusy());
        p.input()->appendPlainText(QStringLiteral(" "));
        QVERIFY(!p.backView()->isVisible());
        QVERIFY(p.backView()->toPlainText().isEmpty());
        p.setInput(QStringLiteral("Das Haus ist alt"));
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt")));
        // Swap: the result moves into the input and is translated the other way
        QSignalSpy done(&p, &Panel::finished);
        p.swapButton()->click();
        QTRY_VERIFY(done.count() > 0);
        QCOMPARE(p.taskBox()->currentData().toString(), QStringLiteral("translateEn"));
        QCOMPARE(p.inputText(), QStringLiteral("The house is old"));
        QTRY_COMPARE(p.resultText(), QStringLiteral("Das Haus ist alt"));
        QVERIFY(modelsCalled().contains(QStringLiteral("opus-mt-en-de")));
        // and back
        QSignalSpy done2(&p, &Panel::finished);
        p.swapButton()->click();
        QTRY_VERIFY(done2.count() > 0);
        QCOMPARE(p.taskBox()->currentData().toString(), QStringLiteral("translate"));
        QCOMPARE(p.inputText(), QStringLiteral("Das Haus ist alt"));
        // the way back without its model: it is offered for download
        QTRY_VERIFY(!p.isBusy());
        uninstall(QStringLiteral("opus-mt-en-de"));
        p.backButton()->click();
        QTRY_VERIFY2(p.backView()->toPlainText().contains(QStringLiteral("Download it")), qPrintable(p.backView()->toPlainText()));
        shot(&p, QStringLiteral("window-back-offer"));
    }

    // e: the second row in translate mode is the translator's model, and the style list is hidden
    void secondRow()
    {
        install(QStringLiteral("opus-mt-de-en"));
        install(QStringLiteral("opus-mt-big-de-en"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        QVERIFY(!p.styleBox()->isHidden());
        QCOMPARE(p.modelBox()->toolTip().contains(QStringLiteral("proofreading")), true);
        select(p, QStringLiteral("translate"));
        QVERIFY(p.styleBox()->isHidden());
        QStringList titles;
        for (int i = 0; i < p.modelBox()->count(); ++i)
            titles << p.modelBox()->itemData(i).toString();
        QCOMPARE(titles, (QStringList{QStringLiteral("opus-mt-de-en"), QStringLiteral("opus-mt-big-de-en")}));
        // choosing the large one changes the setting `translator`, and the next translation uses it
        const int big = p.modelBox()->findData(QStringLiteral("opus-mt-big-de-en"));
        p.modelBox()->setCurrentIndex(big);
        QMetaObject::invokeMethod(p.modelBox(), "activated", Q_ARG(int, big));
        QCOMPARE(m_tool->setting(QStringLiteral("translator")).toString(), QStringLiteral("opus-mt-big-de-en"));
        QVERIFY(runPanel(p, QStringLiteral("Das ist ein Haus")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(modelsCalled().last(), QStringLiteral("opus-mt-big-de-en"));
        // switching the model with a translation on show translates again with it. (The other translations of a short
        // text are asked for in the background after it, at a lower priority: wait for them, so that their call is not
        // taken for the new translation's; on GitHub's Windows machine it came after the calls were cleared.)
        QTRY_VERIFY(!p.variantsPending());
        m_engine.clearCalls();
        const int small = p.modelBox()->findData(QStringLiteral("opus-mt-de-en"));
        p.modelBox()->setCurrentIndex(small);
        QMetaObject::invokeMethod(p.modelBox(), "activated", Q_ARG(int, small));
        QCOMPARE(m_tool->setting(QStringLiteral("translator")).toString(), QStringLiteral("opus-mt-de-en"));
        QTRY_VERIFY(!m_engine.calls().isEmpty());
        QCOMPARE(m_engine.calls().first().model, QStringLiteral("opus-mt-de-en"));
        QTRY_VERIFY(!p.isBusy());
        shot(&p, QStringLiteral("inspector-second-row-translate"));
        select(p, QStringLiteral("proofread"));
        QVERIFY(!p.styleBox()->isHidden());
    }

    // e: a word or up to three words never reach the large model
    void shortTextsSmallModel()
    {
        install(QStringLiteral("opus-mt-de-en"));
        install(QStringLiteral("opus-mt-big-de-en"));
        m_host->settings.insert(QStringLiteral("translation-proofreading/translator"), QStringLiteral("opus-mt-big-de-en"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translate"));
        QVERIFY(runPanel(p, QStringLiteral("Das ist schön")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(modelsCalled().last(), QStringLiteral("opus-mt-de-en"));
        QVERIFY(runPanel(p, QStringLiteral("Quatschkopf")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(modelsCalled().last(), QStringLiteral("opus-mt-de-en"));
        QVERIFY(!modelsCalled().contains(QStringLiteral("opus-mt-big-de-en")));
        // four words: the chosen (large) one
        QVERIFY(runPanel(p, QStringLiteral("Das ist ein Haus")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(modelsCalled().last(), QStringLiteral("opus-mt-big-de-en"));
        QTRY_VERIFY(!p.variantsPending());
        // the small model is not there: a note offers it, and nothing is translated by the large one
        uninstall(QStringLiteral("opus-mt-de-en"));
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("Das ist schön")));
        QVERIFY2(p.status().contains(QStringLiteral("small model")) && p.status().contains(QStringLiteral("OPUS-MT German to English")),
                 qPrintable(p.status()));
        QVERIFY(runPanel(p, QStringLiteral("Quatschkopf")));
        QVERIFY(p.status().contains(QStringLiteral("small model")));
        QVERIFY(m_engine.calls().isEmpty());
    }

    // b: other translations of a short text: complete translations, names dropped; none for a single word
    void otherTranslations()
    {
        install(QStringLiteral("opus-mt-de-en"));
        m_engine.variantList = {QStringLiteral("The house is old"), QStringLiteral("The home is old"),
                                QStringLiteral("The house is old, said Eriksson"), QStringLiteral("That house is old")};
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translate"));
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt")));
        QTRY_VERIFY(!p.variantsPending());
        QStringList v = p.otherTranslations();
        QCOMPARE(v, (QStringList{QStringLiteral("The home is old"), QStringLiteral("That house is old")}));
        QVERIFY(out(p).contains(QStringLiteral("Other translations:")));
        QVERIFY(!out(p).contains(QStringLiteral("Eriksson")));
        // a click on one uses it
        p.output()->anchorClicked(QUrl(QStringLiteral("tp-variant:0")));
        QCOMPARE(p.resultText(), QStringLiteral("The home is old"));
        // a single word asks for none
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("Quatschkopf")));
        QTRY_VERIFY(!p.isBusy());
        for (const FakeEngine::Call &c : m_engine.calls())
            QCOMPARE(c.what, QStringLiteral("run"));
        QVERIFY(p.otherTranslations().isEmpty());
        QVERIFY(hasStrangeName(QStringLiteral("He met Eriksson there."), QStringLiteral("Er traf ihn dort.")));
        QVERIFY(!hasStrangeName(QStringLiteral("He met Mara there."), QStringLiteral("Er traf Mara dort.")));
        QVERIFY(!hasStrangeName(QStringLiteral("Water, he said."), QStringLiteral("Wasser, sagte er.")));
    }

    // ---- 1.4

    // 57: a single lower-case German word with "Automatic" is translated (it ran as Correct before); the typed form gives
    // both of its classes
    void automaticSingleGermanWord()
    {
        install(QStringLiteral("wiktionary-de"));
        install(QStringLiteral("opus-mt-de-en"));
        install(QStringLiteral("qwen3-1.7b"));   // a proofreading model is there, too: it must not be taken
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        QCOMPARE(p.taskBox()->currentData().toString(), QStringLiteral("auto"));
        QVERIFY(runPanel(p, QStringLiteral("zwirbel")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(p.direction(), Panel::Direction::DeToEn);
        QVERIFY2(out(p).contains(QStringLiteral("Zwirbel (noun)")), qPrintable(out(p)));
        QVERIFY(!out(p).contains(QStringLiteral("Spelling:")));
        for (const QString &m : modelsCalled())
            QVERIFY2(m != QStringLiteral("qwen3-1.7b"), "the proofreading model was used");
        // a lower-case word that is a verb and a noun with a capital gives both classes
        QVERIFY(runPanel(p, QStringLiteral("haus")));
        QTRY_VERIFY(!p.isBusy());
        QVERIFY2(out(p).contains(QStringLiteral("hausen (verb)")) && out(p).contains(QStringLiteral("Haus (noun)")), qPrintable(out(p)));
        // an English word of the word list stays English, also when the dictionary is there
        QVERIFY(runPanel(p, QStringLiteral("house")));
        QCOMPARE(p.direction(), Panel::Direction::Proofread);
        // without the dictionary nothing says it is German: proofreading as before
        uninstall(QStringLiteral("wiktionary-de"));
        QVERIFY(runPanel(p, QStringLiteral("zwirbel")));
        QCOMPARE(p.direction(), Panel::Direction::Proofread);
        shot(&p, QStringLiteral("word-automatic-german"));
    }

    // 57: English to German: the similar German words come from the dictionary's meanings (a reverse lookup of the word and
    // its WordNet synonyms); without the dictionary one line says where to get it
    void similarGermanWords()
    {
        install(QStringLiteral("opus-mt-en-de"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 900);
        p.show();
        select(p, QStringLiteral("translateEn"));
        QVERIFY(runPanel(p, QStringLiteral("curse")));
        QTRY_VERIFY(!p.isBusy());
        QVERIFY2(out(p).contains(QStringLiteral("German dictionary")) && out(p).contains(QStringLiteral("File > Options > Proofreading")),
                 qPrintable(out(p)));
        QVERIFY(!out(p).contains(QStringLiteral("Similar German words (")));
        install(QStringLiteral("wiktionary-de"));
        QVERIFY(runPanel(p, QStringLiteral("curse")));
        QTRY_VERIFY(!p.isBusy());
        const QString o = out(p);
        QVERIFY2(o.contains(QStringLiteral("Similar German words (noun):")), qPrintable(o));
        QVERIFY(o.contains(QStringLiteral("Eid")) && o.contains(QStringLiteral("Kraftausdruck")));   // "oath", "expletive": synonyms
        QVERIFY2(o.contains(QStringLiteral("Similar German words (verb):")) && o.contains(QStringLiteral("lästern")), qPrintable(o));
        // the translation itself ("der Fluch") is not a similar word of itself
        const int at = o.indexOf(QStringLiteral("Similar German words (noun):"));
        QVERIFY(!o.mid(at, o.indexOf(QLatin1Char('\n'), at) - at).contains(QStringLiteral("Fluch")));
        shot(&p, QStringLiteral("word-similar-german"));
    }

    // 58: German capitals are put right in the copy the model gets, not in the writer's text
    void germanCapitals()
    {
        install(QStringLiteral("wiktionary-de"));
        install(QStringLiteral("opus-mt-de-en"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translate"));
        const QString text = QStringLiteral("das haus ist alt und ich sage fluch");
        QVERIFY(runPanel(p, text));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(m_engine.calls().first().texts, QStringList{QStringLiteral("das Haus ist alt und ich sage Fluch")});
        QCOMPARE(p.inputText(), text);
        QVERIFY(!p.notes().contains(QStringLiteral("capital")));
        QTRY_VERIFY(!p.variantsPending());
        // without the dictionary: the text goes as it is, and a note says that nouns need capitals
        uninstall(QStringLiteral("wiktionary-de"));
        m_engine.clearCalls();
        QVERIFY(runPanel(p, QStringLiteral("das haus ist alt und ich sage fluch")));
        QTRY_VERIFY(!p.isBusy());
        QCOMPARE(m_engine.calls().first().texts, QStringList{text});
        QVERIFY2(p.notes().contains(QStringLiteral("German nouns need capital letters")), qPrintable(p.notes()));
        shot(&p, QStringLiteral("word-capitals-note"));
    }

    // 78: the notice names what is missing and its button opens the options
    void missingPartsNotice()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        // nothing yet, "Automatic" and no text: a model of some kind
        QVERIFY(!p.notice()->isHidden());
        QVERIFY2(p.noticeLabel()->text().contains(QStringLiteral("proofreading or translation model")), qPrintable(p.noticeLabel()->text()));
        shot(&p, QStringLiteral("notice-nothing"));
        select(p, QStringLiteral("translate"));
        QVERIFY2(p.noticeLabel()->text().contains(QStringLiteral("translation model (German to English)"))
                     && p.noticeLabel()->text().contains(QStringLiteral("German dictionary")),
                 qPrintable(p.noticeLabel()->text()));
        m_host->shownOptions.clear();
        p.noticeButton()->click();
        QVERIFY(!m_host->shownOptions.isEmpty());
        // the model is there, the dictionary not
        install(QStringLiteral("opus-mt-de-en"));
        p.reloadSettings();
        QVERIFY2(!p.notice()->isHidden() && !p.noticeLabel()->text().contains(QStringLiteral("translation model"))
                     && p.noticeLabel()->text().contains(QStringLiteral("German dictionary")),
                 qPrintable(p.noticeLabel()->text()));
        install(QStringLiteral("wiktionary-de"));
        p.reloadSettings();
        QVERIFY(p.notice()->isHidden());
        // English to German needs its own model
        select(p, QStringLiteral("translateEn"));
        QVERIFY(p.noticeLabel()->text().contains(QStringLiteral("(English to German)")));
        // proofreading
        select(p, QStringLiteral("proofread"));
        QVERIFY(p.noticeLabel()->text().contains(QStringLiteral("a proofreading model")));
        install(QStringLiteral("qwen3-1.7b"));
        p.reloadSettings();
        QVERIFY(p.notice()->isHidden());
        // the plugin's own window too
        Panel w(m_tool, Panel::Place::Window, nullptr);
        w.resize(900, 500);
        w.show();
        select(w, QStringLiteral("translateEn"));
        QVERIFY(!w.notice()->isHidden());
        shot(&w, QStringLiteral("notice-window"));
    }

    // 57: no alternative that differs from another only in capitals (also not from the main translation); the higher one stays
    void noDuplicateAlternatives()
    {
        install(QStringLiteral("opus-mt-de-en"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translate"));
        m_engine.variantList = {QStringLiteral("The house is old"), QStringLiteral("The House is old"),
                                QStringLiteral("The home is old"), QStringLiteral("The Home is old"), QStringLiteral("The domicile is old")};
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt")));
        QTRY_VERIFY(!p.variantsPending());
        QCOMPARE(p.otherTranslations(), (QStringList{QStringLiteral("The home is old"), QStringLiteral("The domicile is old")}));
        // the words of the model for one place
        m_engine.alternativeList = {{QStringLiteral("House"), QStringLiteral("The House is old")}, {QStringLiteral("home"), QStringLiteral("The home is old")},
                                    {QStringLiteral("Home"), QStringLiteral("The Home is old")}, {QStringLiteral("house"), QStringLiteral("x")},
                                    {QStringLiteral("Domicile"), QStringLiteral("The Domicile is old")}};
        QString error;
        QStringList words;
        for (const Alternative &a : p.outputAlternatives(0, 1, &error))
            words << a.word;
        QCOMPARE(words, (QStringList{QStringLiteral("home"), QStringLiteral("Domicile")}));
    }

    // 57: the word pane has no Spelling line for a word of the output; a word clicked in the input keeps it
    void wordPaneOutputNoSpelling()
    {
        install(QStringLiteral("opus-mt-de-en"));
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 900);
        p.show();
        select(p, QStringLiteral("translate"));
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt")));
        QTRY_VERIFY(!p.isBusy());
        p.showInfoForOutput(out(p).indexOf(QStringLiteral("house")) + 2);
        QVERIFY(p.info()->toPlainText().contains(QStringLiteral("house")));
        QVERIFY2(!p.info()->toPlainText().contains(QStringLiteral("Spelling")), qPrintable(p.info()->toPlainText()));
        // an unknown word of the output, too
        select(p, QStringLiteral("proofread"));
        QVERIFY(runPanel(p, QStringLiteral("She sees a zork there.")));
        QTRY_VERIFY(!p.isBusy());
        p.showInfoForOutput(out(p).indexOf(QStringLiteral("zork")) + 1);
        QVERIFY2(!p.info()->toPlainText().contains(QStringLiteral("Spelling")), qPrintable(p.info()->toPlainText()));
        p.showInfoForInput(p.inputText().indexOf(QStringLiteral("zork")) + 1);
        QVERIFY(p.info()->toPlainText().contains(QStringLiteral("Spelling")));
    }

    // 72: a model is unloaded after the waiting time (15 minutes at first, a setting), and the tool says so
    void modelUnloading()
    {
        QCOMPARE(m_tool->unloadMinutes(), 15);
        QCOMPARE(m_tool->unloadIntervalMs(), 15 * 60 * 1000);
        m_host->settings.insert(QStringLiteral("translation-proofreading/unloadMinutes"), 30);
        QCOMPARE(m_tool->unloadIntervalMs(), 30 * 60 * 1000);
        m_host->settings.insert(QStringLiteral("translation-proofreading/unloadMinutes"), 0);   // never
        m_tool->touchEngine();
        QVERIFY(!m_tool->unloadTimerActive());
        m_host->settings.remove(QStringLiteral("translation-proofreading/unloadMinutes"));
        QCOMPARE(QString::fromLatin1("Model unloaded, RAM usage minimal"), TranslateTool::unloadMessage());

        install(QStringLiteral("opus-mt-de-en"));
        qputenv("TP_TEST_UNLOAD_MS", "80");
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(380, 800);
        p.show();
        select(p, QStringLiteral("translate"));
        QSignalSpy spy(m_tool, &TranslateTool::modelsUnloaded);
        QVERIFY(runPanel(p, QStringLiteral("Das Haus ist alt")));
        QTRY_VERIFY(!p.isBusy());
        QVERIFY(!m_engine.loaded(QStringLiteral("translator")).isEmpty());
        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spy.first().at(0).toString(), QStringLiteral("Model unloaded, RAM usage minimal"));
        QCOMPARE(spy.first().at(1).toInt(), 5000);
        QVERIFY(m_engine.loaded(QStringLiteral("translator")).isEmpty());
        // nothing loaded: nothing to say
        QTest::qWait(300);
        QCOMPARE(spy.count(), 1);
        qunsetenv("TP_TEST_UNLOAD_MS");
    }

    // 89: the hints in the empty fields wrap at a narrow width (they were cut off at 300 px)
    void placeholdersWrap()
    {
        Panel p(m_tool, Panel::Place::Inspector, nullptr);
        p.resize(300, 800);
        p.show();
        QTest::qWait(50);
        const QList<QAbstractScrollArea *> areas = {p.input(), p.output(), p.info()};
        for (QAbstractScrollArea *a : areas) {
            auto *label = a->viewport()->findChild<QLabel *>(QStringLiteral("tpPlaceholder"));
            QVERIFY(label);
            QVERIFY(!label->isHidden());
            QVERIFY(label->wordWrap());
            QVERIFY2(label->width() <= a->viewport()->width() && label->width() > 100, qPrintable(QString::number(label->width())));
            // several lines: more than the one line of an unwrapped text
            QVERIFY2(label->heightForWidth(label->width()) > 2 * label->fontMetrics().lineSpacing(),
                     qPrintable(QString::number(label->heightForWidth(label->width()))));
        }
        shot(&p, QStringLiteral("placeholders-300"));
        p.setInput(QStringLiteral("x"));
        QVERIFY(p.input()->viewport()->findChild<QLabel *>(QStringLiteral("tpPlaceholder"))->isHidden());
    }
};

QTEST_MAIN(WordTests)
#include "word_tests.moc"
