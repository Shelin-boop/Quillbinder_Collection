// The system test, 0.99.0 (item 124): Read Aloud (S-J01 to S-J05). The speech is the fake AI engine's (QB_AI_ENGINE_LIB, set by the
// runner's main for these cases) and its sound output counts frames and plays nothing; the catalog of models is a file of the case.
#include "cs.h"

#include "core/AiCatalog.h"
#include "core/AiEngine.h"
#include "core/ModelMemory.h"
#include "core/ReadAloud.h"
#include "core/SystemMemory.h"
#include "ui/MainWindow.h"
#include "ui/ReadAloudController.h"
#include "ui/TextEditor.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QLibrary>
#include <QMenu>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>

namespace cs {

using namespace qb;
using namespace qb::readaloud;

namespace {

typedef void (*OutStats)(int *, long long *, int *, int *, int *, int *);
typedef void (*OutScale)(double);
OutStats g_outStats = nullptr;
OutScale g_outScale = nullptr;
void setScale(double x) { if (g_outScale) g_outScale(x); }
long long outWritten() { long long v = 0; if (g_outStats) g_outStats(nullptr, &v, nullptr, nullptr, nullptr, nullptr); return v; }
int outOpens() { int v = 0; if (g_outStats) g_outStats(&v, nullptr, nullptr, nullptr, nullptr, nullptr); return v; }
bool outPaused() { int v = 0; if (g_outStats) g_outStats(nullptr, nullptr, &v, nullptr, nullptr, nullptr); return v != 0; }
bool outStopped() { int v = 0; if (g_outStats) g_outStats(nullptr, nullptr, nullptr, &v, nullptr, nullptr); return v != 0; }

QAction *findAct(QMenu *m, const QString &name)
{
    for (QAction *a : m->actions())
        if (a->objectName() == name)
            return a;
    return nullptr;
}

struct Ra {
    MainWindow *w = nullptr;
    Controller *ctl = nullptr;
    TextEditor *ed = nullptr;
    QString body;
};

bool raUp(Ra &r, const QString &body)
{
    const QString catalogFile = qEnvironmentVariable("QB_AI_CATALOG");
    auto modelJson = [](const QString &id, const QString &title, const QString &engine, const QStringList &langs, const QStringList &voices, int memoryMB) {
        QJsonArray ls, vs;
        for (const QString &l : langs)
            ls << l;
        for (const QString &v : voices)
            vs << v;
        return QJsonObject{{"id", id}, {"title", title}, {"kinds", QJsonArray{"tts"}}, {"engine", engine}, {"languages", ls}, {"voices", vs},
                           {"memoryMB", memoryMB}, {"sizeMB", 2}, {"model", "talker.gguf"}, {"extra", "tok.gguf"}, {"license", "MIT"},
                           {"files", QJsonArray{QJsonObject{{"name", "talker.gguf"}, {"asset", id + "--t"}, {"size", 2000}, {"sha256", "00"}},
                                                QJsonObject{{"name", "tok.gguf"}, {"asset", id + "--k"}, {"size", 500}, {"sha256", "00"}}}}};
    };
    const QJsonArray models{
        modelJson(QStringLiteral("pocket-tts-en"), QStringLiteral("Pocket TTS English"), QStringLiteral("pocket-tts"), {QStringLiteral("en")},
                  {QStringLiteral("alba"), QStringLiteral("anna"), QStringLiteral("azelma"), QStringLiteral("charles"), QStringLiteral("michael")}, 50),
        modelJson(QStringLiteral("qwen3-tts-1.7b-q4"), QStringLiteral("Qwen3-TTS 1.7B"), QStringLiteral("qwen3-tts"), {QStringLiteral("en"), QStringLiteral("de")}, {}, 100000000)};
    QDir().mkpath(QFileInfo(catalogFile).absolutePath());
    writeText(catalogFile, QJsonDocument(QJsonObject{{"format", 1}, {"release", "models-5"}, {"models", models}}).toJson());
    for (const QString &id : {QStringLiteral("pocket-tts-en"), QStringLiteral("qwen3-tts-1.7b-q4")}) {
        writeText(ai::modelDir(id) + QStringLiteral("/talker.gguf"), QByteArray(2000, 'm'));
        writeText(ai::modelDir(id) + QStringLiteral("/tok.gguf"), QByteArray(500, 't'));
        writeText(ai::modelDir(id) + QStringLiteral("/.checked"), "ok");
    }
    const QString fakeLib = ctx().buildDir + QStringLiteral("/tests/fake-ai-engine/libqbai.so.1");
    g_outStats = reinterpret_cast<OutStats>(QLibrary(fakeLib).resolve("qbai_fake_out_stats"));
    g_outScale = reinterpret_cast<OutScale>(QLibrary(fakeLib).resolve("qbai_fake_out_set_scale"));
    check(g_outStats && g_outScale, QStringLiteral("(the fake engine's sound functions)"));
    QString why;
    check(ai::Engine::instance().ensureLoaded(&why), QStringLiteral("(the fake engine loads)"), why);
    Config::forceAndroid(0);
    Config::instance().reload();
    Settings s;
    s.barShown = true;
    s.settingsOpened = true;
    Config::instance().set(s);
    modelmem::ModelMemory::instance().reset();
    Session::forgetModels();
    r.w = openWindow(fixtureCopy(QStringLiteral("fx1-qb")), 1400, 800);
    if (!r.w || !r.w->project())
        return false;
    r.ctl = r.w->readAloud();
    r.ed = showDoc(r.w, textItems(r.w->project()).value(3));
    if (!r.ctl || !r.ed)
        return false;
    r.body = body;
    r.ed->setPlainText(body);
    r.ed->setFocus();
    settle(200);
    setScale(8);
    return true;
}

QByteArray wavOf(int rate, int samples)
{
    QByteArray d;
    auto le = [&](quint32 v, int n) { for (int i = 0; i < n; ++i) d.append(char((v >> (8 * i)) & 0xff)); };
    d += "RIFF"; le(36 + samples * 2, 4); d += "WAVEfmt "; le(16, 4); le(1, 2); le(1, 2); le(rate, 4); le(rate * 2, 4); le(2, 2); le(16, 2);
    d += "data"; le(samples * 2, 4);
    for (int i = 0; i < samples; ++i) le(quint16(qint16(3000 * ((i / 20) % 2 ? 1 : -1))), 2);
    return d;
}

const QString kBody = QStringLiteral("Alpha is first. Beta is second. Gamma is third.");

} // namespace

void caseSJ01()   // marked text
{
    Ra r;
    if (!raUp(r, kBody))
        return;
    QList<QPair<int, int>> seen;
    QObject::connect(&r.ctl->session(), &Session::sentence, [&](int a, int b) { if (b > 0) seen << qMakePair(a, b); });
    QTextCursor sel = r.ed->textCursor();
    sel.setPosition(kBody.indexOf(QLatin1String("Gamma")));
    sel.setPosition(kBody.size(), QTextCursor::KeepAnchor);
    r.ed->setTextCursor(sel);
    QSignalSpy fin(&r.ctl->session(), &Session::finished);
    r.ctl->playAction()->trigger();
    check(waitFor([&] { return !fin.isEmpty(); }, 10000), QStringLiteral("the reading of the marked text ends"));
    check(seen.size() == 1 && seen.value(0).first == kBody.indexOf(QLatin1String("Gamma")), QStringLiteral("only the marked sentence is read"), QString::number(seen.size()));
    check(r.ed->readingLength() == 0 && r.ctl->session().state() == Session::Idle, QStringLiteral("the mark is gone at the end"));
    check(outWritten() > 0 && outOpens() > 0, QStringLiteral("sound reached the output"));
    // a mark over two sentences
    seen.clear();
    fin.clear();
    sel.setPosition(kBody.indexOf(QLatin1String("Alpha")));
    sel.setPosition(kBody.indexOf(QLatin1String("Gamma")), QTextCursor::KeepAnchor);
    r.ed->setTextCursor(sel);
    r.ctl->playAction()->trigger();
    check(waitFor([&] { return !fin.isEmpty(); }, 10000) && seen.size() == 2, QStringLiteral("two marked sentences: both are read, the third is not"), QString::number(seen.size()));
    shot(r.w, QStringLiteral("j01"));
    finishProject(r.w);
}

void caseSJ02()   // from the cursor
{
    Ra r;
    if (!raUp(r, kBody))
        return;
    QList<int> starts;
    QObject::connect(&r.ctl->session(), &Session::sentence, [&](int a, int b) { if (b > 0) starts << a; });
    QTextCursor cur = r.ed->textCursor();
    cur.setPosition(kBody.indexOf(QLatin1String("Beta")));
    r.ed->setTextCursor(cur);
    QSignalSpy fin(&r.ctl->session(), &Session::finished);
    r.ctl->playAction()->trigger();
    check(r.ctl->session().state() != Session::Idle, QStringLiteral("Play with nothing marked starts reading"));
    check(waitFor([&] { return !fin.isEmpty(); }, 10000), QStringLiteral("(the reading ends)"));
    check(starts == QList<int>({kBody.indexOf(QLatin1String("Beta")), kBody.indexOf(QLatin1String("Gamma"))}), QStringLiteral("from the cursor to the end: Beta, then Gamma"), QString::number(starts.size()));
    // the mark follows the sentences in the editor
    setScale(2);
    cur.setPosition(0);
    r.ed->setTextCursor(cur);
    r.ctl->playAction()->trigger();
    check(waitFor([&] { return r.ed->readingLength() > 0; }, 8000), QStringLiteral("the first sentence is marked in the editor"));
    bool exact = false;
    for (const QTextEdit::ExtraSelection &x : r.ed->extraSelections())
        exact = exact || (x.cursor.selectionStart() == 0 && x.cursor.selectedText() == QLatin1String("Alpha is first."));
    check(exact, QStringLiteral("the mark is a highlight of exactly that sentence"));
    shot(r.w, QStringLiteral("j02-marked"));
    setScale(40);
    check(waitFor([&] { return r.ctl->session().state() == Session::Idle; }, 10000), QStringLiteral("(the reading ends)"));
    finishProject(r.w);
}

void caseSJ03()   // Pause and Stop
{
    Ra r;
    if (!raUp(r, kBody))
        return;
    setScale(2);
    r.ctl->playAction()->trigger();
    check(waitFor([&] { return r.ctl->session().state() == Session::Playing && r.ed->readingLength() > 0; }, 8000), QStringLiteral("(playing, a sentence marked)"));
    check(r.ctl->playAction()->text() == QLatin1String("Pause") && r.ctl->stopAction()->isEnabled(), QStringLiteral("while it plays the button says Pause, Stop is on"));
    r.ctl->playAction()->trigger();
    check(r.ctl->session().state() == Session::Paused && outPaused() && r.ctl->playAction()->text() == QLatin1String("Resume"), QStringLiteral("Pause pauses the sound"));
    const int at = r.ed->readingStart();
    settle(400);
    check(r.ed->readingStart() == at, QStringLiteral("the mark stands still while paused"));
    shot(r.w, QStringLiteral("j03-paused"));
    r.ctl->playAction()->trigger();
    check(r.ctl->session().state() == Session::Playing && !outPaused(), QStringLiteral("Play again goes on"));
    QSignalSpy fin(&r.ctl->session(), &Session::finished);
    r.ctl->stopAction()->trigger();
    check(r.ctl->session().state() == Session::Idle && r.ed->readingLength() == 0 && outStopped() && !fin.isEmpty(), QStringLiteral("Stop ends the reading, clears the mark and the sound"));
    check(!r.ctl->stopAction()->isEnabled(), QStringLiteral("Stop is off again"));
    // typing stops a reading
    setScale(40);
    r.ctl->playAction()->trigger();
    check(waitFor([&] { return r.ctl->session().state() == Session::Playing; }, 8000), QStringLiteral("(reading, to be changed)"));
    setScale(0);
    r.ed->textCursor().insertText(QStringLiteral("x"));
    check(r.ctl->session().state() == Session::Idle, QStringLiteral("a change in the text stops the reading"));
    finishProject(r.w);
}

void caseSJ04()   // a voice the model lacks is greyed
{
    Ra r;
    if (!raUp(r, kBody))
        return;
    r.ctl->fillMenu();
    QMenu *m = r.ctl->voiceMenu();
    check(findAct(m, QStringLiteral("readAloud:female1")) && findAct(m, QStringLiteral("readAloud:female1"))->isEnabled(), QStringLiteral("Female 1 is on (the model has its voice)"));
    QAction *male2 = findAct(m, QStringLiteral("readAloud:male2"));
    check(male2 && !male2->isEnabled() && male2->toolTip().contains(QLatin1String("does not have this voice")), QStringLiteral("a voice the model lacks is greyed with the reason"),
          male2 ? male2->toolTip() : QString());
    bool styleGrey = true;
    for (const QString &st : styleIds())
        styleGrey = styleGrey && findAct(m, QStringLiteral("readAloud:") + st) && !findAct(m, QStringLiteral("readAloud:") + st)->isEnabled();
    check(styleGrey, QStringLiteral("the model has no style control: the styles are greyed"));
    m->popup(r.ctl->voiceButton()->mapToGlobal(QPoint(0, r.ctl->voiceButton()->height())));
    settle(250);
    shot(m, QStringLiteral("j04-menu"));
    m->hide();
    // chosen anyway (from the settings file): Play tells why and does not start
    Settings s = Config::instance().get();
    s.voice = QStringLiteral("male2");
    Config::instance().set(s);
    QStringList said;
    QObject::connect(r.ctl, &Controller::message, [&](const QString &t) { said << t; });
    r.ctl->playAction()->trigger();
    settle(400);
    check(r.ctl->session().state() == Session::Idle && !said.isEmpty() && said.join(QLatin1Char(' ')).contains(QLatin1String("does not have this voice")),
          QStringLiteral("Play with that voice does not start and says why"), said.join(QLatin1String(" | ")));
    s.voice = QStringLiteral("female1");
    Config::instance().set(s);
    finishProject(r.w);
}

void caseSJ05()   // a model that does not fit is refused with its reason
{
    Ra r;
    if (!raUp(r, kBody))
        return;
    QStringList said;
    QObject::connect(r.ctl, &Controller::message, [&](const QString &t) { said << t; });
    writeText(ai::modelsRoot() + QStringLiteral("/read-aloud-voices/female1-neutral.wav"), wavOf(24000, 24000));   // the voice of the big model exists
    writeText(ai::modelsRoot() + QStringLiteral("/read-aloud-voices/female1-neutral.txt"), "A reference sentence.");
    const int opensBefore = outOpens();
    Settings s = Config::instance().get();
    s.model = QStringLiteral("qwen3-tts-1.7b-q4");
    Config::instance().set(s);
    r.ctl->playAction()->trigger();
    settle(600);
    check(r.ctl->session().state() == Session::Idle && outOpens() == opensBefore, QStringLiteral("a model larger than the memory is not loaded, nothing plays"));
    const QString all = said.join(QLatin1String(" | "));
    check(!said.isEmpty() && (all.contains(QLatin1String("memory"), Qt::CaseInsensitive) || all.contains(QLatin1String("fit"), Qt::CaseInsensitive)), QStringLiteral("the writer is told why"), all);
    shot(r.w, QStringLiteral("j05-refused"));
    // the rule over 80 % with the small model
    s.model = QStringLiteral("pocket-tts-en");
    s.device = QStringLiteral("cpu");   // RAM is the memory of the rule
    Config::instance().set(s);
    said.clear();
    Session::forgetModels();
    modelmem::ModelMemory::instance().reset();
    sysmem::setFakeRam({1000 * 1000 * 1000LL, 100 * 1000 * 1000LL});   // 90 % used
    r.ctl->playAction()->trigger();
    settle(600);
    const QString all2 = said.join(QLatin1String(" | "));
    check(r.ctl->session().state() == Session::Idle && all2.contains(QLatin1String("Not enough memory"), Qt::CaseInsensitive), QStringLiteral("over 80 % of the memory used: refused with the reason"), all2);
    sysmem::clearFakeRam();
    said.clear();
    setScale(40);
    QSignalSpy fin(&r.ctl->session(), &Session::finished);
    r.ctl->playAction()->trigger();
    check(waitFor([&] { return !fin.isEmpty(); }, 10000), QStringLiteral("with the memory free again the same model reads"), said.join(QLatin1String(" | ")));
    finishProject(r.w);
}

} // namespace cs
