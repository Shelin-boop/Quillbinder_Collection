// Read Aloud (0.99.0, item 114): the toolbar with Play/Pause, Stop and the Voice menu, the first click that opens File > Options >
// Read Aloud, the reading (marked text, from the cursor, the sentence marked in the editor), the voices and styles with the reasons
// of the greyed ones, the settings page (languages, models, fit, device, speed test), the memory rule, cancel, and that no sound is
// written to disk. A fake libqbai makes the "speech" and a fake output stands in for the media add-on (nothing is played).
//
//   xvfb-run -a ./qb_v0990n_tests /tmp/shots
#include "core/AiCatalog.h"
#include "core/AiEngine.h"
#include "core/ModelMemory.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/ReadAloud.h"
#include "core/BinderModel.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/MainWindow.h"
#include <QSplitter>
#include "ui/ReadAloudController.h"
#include "ui/ReadAloudPage.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLibrary>
#include <QMenu>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QDialog>
#include <QStyleFactory>
#include <QTimer>
#include <QTemporaryDir>
#include <QTest>
#include <QToolBar>
#include <QToolButton>
#include <cstdio>

using namespace qb;
using modelmem::ModelMemory;

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
static void settle(int ms = 200)
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
    settle(250);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}
static bool waitFor(const std::function<bool()> &cond, int ms = 8000)
{
    QElapsedTimer t;
    t.start();
    while (!cond() && t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
    return cond();
}
static const qint64 MB = 1024 * 1024;

static void writeFile(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
}

// ---- the sound output is the fake engine's (nothing is played; its clock runs `scale` times faster than real time)
typedef void (*OutStats)(int *, long long *, int *, int *, int *, int *);
typedef void (*OutScale)(double);
static OutStats g_outStats = nullptr;
static OutScale g_outScale = nullptr;
static void setScale(double x) { if (g_outScale) g_outScale(x); }
static int outOpens() { int v = 0; g_outStats(&v, nullptr, nullptr, nullptr, nullptr, nullptr); return v; }
static long long outWritten() { long long v = 0; g_outStats(nullptr, &v, nullptr, nullptr, nullptr, nullptr); return v; }
static bool outPaused() { int v = 0; g_outStats(nullptr, nullptr, &v, nullptr, nullptr, nullptr); return v != 0; }
static bool outStopped() { int v = 0; g_outStats(nullptr, nullptr, nullptr, &v, nullptr, nullptr); return v != 0; }
static int outRate() { int v = 0; g_outStats(nullptr, nullptr, nullptr, nullptr, &v, nullptr); return v; }
static int outLive() { int v = 0; g_outStats(nullptr, nullptr, nullptr, nullptr, nullptr, &v); return v; }

static QByteArray wavFile(int rate, int samples)
{
    QByteArray d;
    auto le = [&](quint32 v, int n) { for (int i = 0; i < n; ++i) d.append(char((v >> (8 * i)) & 0xff)); };
    d += "RIFF"; le(36 + samples * 2, 4); d += "WAVEfmt "; le(16, 4); le(1, 2); le(1, 2); le(rate, 4); le(rate * 2, 4); le(2, 2); le(16, 2);
    d += "data"; le(samples * 2, 4);
    for (int i = 0; i < samples; ++i) le(quint16(qint16(3000 * ((i / 20) % 2 ? 1 : -1))), 2);
    return d;
}

// File > Options opens modal: look at it and close it
static int g_dialogs = 0;
static bool g_pageShown = false;
static void armDialog(const QString &shotName)
{
    QTimer::singleShot(500, [shotName] {
        QWidget *m = QApplication::activeModalWidget();
        if (!m) {
            QTimer::singleShot(500, [shotName] { armDialog(shotName); });
            return;
        }
        ++g_dialogs;
        for (QWidget *pg : m->findChildren<QWidget *>(QStringLiteral("readAloudPage")))
            g_pageShown = g_pageShown || pg->isVisible();
        m->grab().save(outDir + QLatin1Char('/') + shotName + QLatin1String(".png"));
        static_cast<QDialog *>(m)->reject();
    });
}

static BinderItem *firstDocument(BinderItem *it)
{
    for (BinderItem *c : it->children) {
        if (!c->isFolder())
            return c;
        if (BinderItem *d = firstDocument(c))
            return d;
    }
    return nullptr;
}

static QAction *findAction(QMenu *m, const QString &name)
{
    for (QAction *a : m->actions())
        if (a->objectName() == name)
            return a;
    return nullptr;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QBAI_FAKE_CARDS", "Big Card,8192,8000;Small Card,1024,40");
    const QString work = home.path() + QStringLiteral("/work");
    QDir().mkpath(work);
    const QString catalogFile = work + QStringLiteral("/catalog.json");
    qputenv("QB_AI_CATALOG", catalogFile.toUtf8());
    const QString fakeLib = QStringLiteral(QB_TEST_BUILD_DIR) + QStringLiteral("/tests/fake-ai-engine/libqbai.so.1");
    g_outStats = reinterpret_cast<OutStats>(QLibrary(fakeLib).resolve("qbai_fake_out_stats"));
    g_outScale = reinterpret_cast<OutScale>(QLibrary(fakeLib).resolve("qbai_fake_out_set_scale"));
    if (!g_outStats || !g_outScale) {
        std::printf("FAIL  (the fake engine's sound functions)\n");
        return 1;
    }
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990nTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990nTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.welcomeShown = true;
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();
    using namespace readaloud;
    ModelMemory &mm = ModelMemory::instance();

    // ======================================================================== the plain parts
    std::printf("== the choices, sentences, models\n");
    {
        check(Settings::defaults(false).barShown && !Settings::defaults(true).barShown,
              QStringLiteral("the toolbar is shown by default on Linux and Windows, hidden on Android"));
        Settings s;
        s.voice = QStringLiteral("female2");
        s.style = QStringLiteral("calm");
        s.speed = QStringLiteral("medium");
        check(s.summary() == QString::fromUtf8("Female 2 · Calm · Medium"), QStringLiteral("the summary of the choices: Female 2 · Calm · Medium"), s.summary());
        check(voiceIds().size() == 6 && styleIds().size() == 3 && speedIds().size() == 3, QStringLiteral("six voices, three styles, three speeds"));
        check(speedFactor(QStringLiteral("medium")) == 1.0 && speedFactor(QStringLiteral("slow")) < 0.9 && speedFactor(QStringLiteral("slow")) > 0.8
                  && speedFactor(QStringLiteral("fast")) > 1.1 && speedFactor(QStringLiteral("fast")) < 1.2,
              QStringLiteral("speed: Medium is the model's own pace, Slow and Fast about 15 % slower and faster"));
        QStringList used;
        bool never = true;
        for (const QString &v : voiceIds()) {
            used << pocketVoice(QStringLiteral("en"), v);
            never = never && pocketVoice(QStringLiteral("en"), v) != QLatin1String("juergen") && !pocketVoice(QStringLiteral("fr"), v).isEmpty();
        }
        used.removeDuplicates();
        check(used.size() == 6 && never, QStringLiteral("Pocket TTS: six different licensed preset voices (never the one without a license)"), used.join(QLatin1Char(' ')));
        check(modelOrder().first() == QLatin1String("pocket-tts-") && modelOrder().size() == 4, QStringLiteral("the order of the models is one table"));
        check(languageHasModel(QStringLiteral("de")) && languageHasModel(QStringLiteral("ru")) && !languageHasModel(QStringLiteral("id"))
                  && !languageHasModel(QStringLiteral("tr")) && !languageHasModel(QStringLiteral("pl")) && !languageHasModel(QStringLiteral("nl")),
              QStringLiteral("id, tr, pl and nl have no model yet"));

        const QString text = QStringLiteral("First one. Second one here! Is the third? \"Yes,\" he said. Dr. Smith agreed.\nNew line");
        const QList<Span> sp = splitSentences(text);
        QStringList parts;
        for (const Span &x : sp)
            parts << text.mid(x.start, x.length);
        check(parts.size() == 6 && parts[0] == QLatin1String("First one.") && parts[2] == QLatin1String("Is the third?")
                  && parts[3] == QLatin1String("\"Yes,\" he said.") && parts[4] == QLatin1String("Dr. Smith agreed.") && parts[5] == QLatin1String("New line"),
              QStringLiteral("sentences: full stops, questions, quotes, an abbreviation, a new line"), parts.join(QLatin1Char('|')));
        const QString longText = QString(QStringLiteral("word, ")).repeated(120);
        const QList<Span> cut = splitSentences(longText, 100);
        bool short100 = !cut.isEmpty();
        for (const Span &x : cut)
            short100 = short100 && x.length <= 100;
        check(cut.size() > 5 && short100, QStringLiteral("a very long sentence is cut at commas"));
        check(lookAheadFor(0.5).ahead == 2 && lookAheadFor(0.5).prebuffer == 1 && lookAheadFor(3).ahead == 4 && lookAheadFor(3).prebuffer == 3,
              QStringLiteral("a few sentences ahead: 2 (4 and 3 before the start when slower than real time)"));
    }

    // ======================================================================== the fixtures
    // the catalog: Pocket TTS English (a voice list without "george"), Pocket French (not installed), Qwen 0.6B, Qwen 1.7B (too big)
    auto modelJson = [](const QString &id, const QString &title, const QString &engine, const QStringList &langs, const QStringList &voices,
                        int memoryMB) {
        QJsonArray ls, vs;
        for (const QString &l : langs) ls << l;
        for (const QString &v : voices) vs << v;
        return QJsonObject{{"id", id}, {"title", title}, {"kinds", QJsonArray{"tts"}}, {"engine", engine}, {"languages", ls}, {"voices", vs},
                           {"memoryMB", memoryMB}, {"sizeMB", 2}, {"model", "talker.gguf"}, {"extra", "tok.gguf"}, {"license", "MIT"},
                           {"files", QJsonArray{QJsonObject{{"name", "talker.gguf"}, {"asset", id + "--t"}, {"size", 2000}, {"sha256", "00"}},
                                                QJsonObject{{"name", "tok.gguf"}, {"asset", id + "--k"}, {"size", 500}, {"sha256", "00"}}}}};
    };
    const QJsonArray models{
        modelJson(QStringLiteral("pocket-tts-en"), QStringLiteral("Pocket TTS English"), QStringLiteral("pocket-tts"), {QStringLiteral("en")},
                  {QStringLiteral("alba"), QStringLiteral("anna"), QStringLiteral("azelma"), QStringLiteral("charles"), QStringLiteral("michael")}, 50),
        modelJson(QStringLiteral("pocket-tts-fr"), QStringLiteral("Pocket TTS French"), QStringLiteral("pocket-tts"), {QStringLiteral("fr")}, {}, 50),
        modelJson(QStringLiteral("qwen3-tts-0.6b-q8"), QStringLiteral("Qwen3-TTS 0.6B"), QStringLiteral("qwen3-tts"),
                  {QStringLiteral("en"), QStringLiteral("de")}, {}, 80),
        modelJson(QStringLiteral("qwen3-tts-1.7b-q4"), QStringLiteral("Qwen3-TTS 1.7B"), QStringLiteral("qwen3-tts"),
                  {QStringLiteral("en"), QStringLiteral("de")}, {}, 100000000)};
    writeFile(catalogFile, QJsonDocument(QJsonObject{{"format", 1}, {"release", "models-5"}, {"models", models}}).toJson());
    const ai::Catalog &cat = ai::Catalog::builtIn();
    check(cat.models.size() == 4, QStringLiteral("(the catalog of the checks is read)"), QString::number(cat.models.size()));
    auto install = [&](const QString &id) {
        writeFile(ai::modelDir(id) + QStringLiteral("/talker.gguf"), QByteArray(2000, 'm'));
        writeFile(ai::modelDir(id) + QStringLiteral("/tok.gguf"), QByteArray(500, 't'));
        writeFile(ai::modelDir(id) + QStringLiteral("/.checked"), "ok");
    };
    auto uninstall = [&](const QString &id) { QDir(ai::modelDir(id)).removeRecursively(); };

    Config::forceAndroid(0);
    Config::instance().reload();
    std::printf("== no engine: the first click, every click\n");
    ai::Engine &engine = ai::Engine::instance();
    {
        check(!engine.isInstalled(), QStringLiteral("(no engine is installed)"));
        const ai::CatalogModel *def = resolveModel(Settings(), {}, false);
        check(def && def->id == QLatin1String("pocket-tts-en") && resolveModel(Settings(), {}, true) == nullptr,
              QStringLiteral("the best candidate is named (Pocket TTS first for English), no model counts while none is installed"));
    }

    // ======================================================================== the window
    QTemporaryDir projDir;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, projDir.path(), QStringLiteral("Lamp"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    proj->save(&err);
    Project *projectPtr = proj.get();

    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.setWindowState(Qt::WindowNoState);
    w.move(0, 0);
    w.resize(1400, 800);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    w.adoptProject(std::move(proj));
    settle(400);
    readaloud::Controller *ctl = w.readAloud();
    check(ctl && w.readAloudBar(), QStringLiteral("the window has the Read Aloud toolbar"));
    if (!ctl)
        return 1;
    // the install of the engine is simulated by the fake library (QB_AI_ENGINE_LIB) later; first: no engine
    std::printf("== the toolbar\n");
    {
        check(w.readAloudBar()->isVisible() && w.findChild<QAction *>(QStringLiteral("showBarReadAloud"))->isChecked(),
              QStringLiteral("Linux and Windows: the toolbar is shown by default (View > Toolbars > Read Aloud is checked)"));
        QToolButton *voice = ctl->voiceButton();
        check(voice && voice->popupMode() == QToolButton::InstantPopup && voice->menu() == ctl->voiceMenu(),
              QStringLiteral("a small Voice button with a menu"));
        check(ctl->playAction() && ctl->stopAction() && !ctl->stopAction()->isEnabled(), QStringLiteral("Play/Pause and Stop; Stop is off while nothing is read"));
        check(ctl->tooltip() == QString::fromUtf8("Female 1 · Neutral · Medium") && voice->toolTip() == ctl->tooltip(),
              QStringLiteral("the Voice button's tool tip shows the choice"), voice->toolTip());
        shot(&w, QStringLiteral("toolbar-window"));
        QPixmap p = w.readAloudBar()->grab();
        p.save(outDir + QStringLiteral("/toolbar.png"));

        QSignalSpy spy(ctl, &Controller::settingsRequested);
        armDialog(QStringLiteral("options-read-aloud"));
        ctl->playAction()->trigger();
        check(spy.size() == 1 && Config::instance().get().settingsOpened && g_dialogs == 1 && g_pageShown,
              QStringLiteral("the first click opens File > Options > Read Aloud (the page is shown)"));
        armDialog(QStringLiteral("options-read-aloud-2"));
        ctl->playAction()->trigger();
        check(spy.size() == 2 && g_dialogs == 2, QStringLiteral("every click while no engine is installed opens it again"));
        check(ctl->session().state() == Session::Idle && outOpens() == 0, QStringLiteral("nothing was loaded, no sound opened"));
    }

    // the engine and the models
    qputenv("QB_AI_ENGINE_LIB", fakeLib.toUtf8());
    install(QStringLiteral("pocket-tts-en"));
    install(QStringLiteral("qwen3-tts-0.6b-q8"));
    install(QStringLiteral("qwen3-tts-1.7b-q4"));
    typedef void (*Stats)(int *, int *, int *, int *);
    QString why;
    check(engine.ensureLoaded(&why), QStringLiteral("(the fake engine loads)"), why);
    Stats stats = reinterpret_cast<Stats>(QLibrary(fakeLib).resolve("qbai_fake_stats"));
    const QList<ai::Device> devices = engine.devices(true);

    std::printf("== models and devices\n");
    {
        Settings s;
        const ai::CatalogModel *def = resolveModel(s, devices, true);
        check(def && def->id == QLatin1String("pocket-tts-en"), QStringLiteral("default for English: Pocket TTS (first in the order, installed, fits)"));
        s.language = QStringLiteral("de");
        def = resolveModel(s, devices, true);
        check(def && def->id == QLatin1String("qwen3-tts-0.6b-q8"),
              QStringLiteral("German has no Pocket TTS: Qwen3-TTS 0.6B; the 1.7B that does not fit is skipped"), def ? def->id : QString());
        s.language = QStringLiteral("en");
        rememberSpeed(QStringLiteral("pocket-tts-en"), QStringLiteral("auto"), 3.0);
        def = resolveModel(s, devices, true);
        check(def && def->id == QLatin1String("qwen3-tts-0.6b-q8"), QStringLiteral("a model that is measured slower than real time gives way to the next that fits"));
        rememberSpeed(QStringLiteral("pocket-tts-en"), QStringLiteral("auto"), 0.0);
        s.model = QStringLiteral("qwen3-tts-0.6b-q8");
        def = resolveModel(s, devices, true);
        check(def && def->id == QLatin1String("qwen3-tts-0.6b-q8"), QStringLiteral("a model the writer chose is used"));

        const ai::CatalogModel &pocket = *cat.model(QStringLiteral("pocket-tts-en"));
        const ai::CatalogModel &big = *cat.model(QStringLiteral("qwen3-tts-1.7b-q4"));
        Fit f1 = fitOf(pocket, QStringLiteral("auto"), devices);
        check(f1.fits && f1.text == QLatin1String("fits on Big Card"), QStringLiteral("Automatic: the card whose free memory holds the model"), f1.text);
        Fit f2 = fitOf(pocket, QStringLiteral("cpu"), devices);
        check(f2.fits && f2.text == QLatin1String("fits in RAM"), QStringLiteral("the processor: fits in RAM"), f2.text);
        Fit f3 = fitOf(pocket, QStringLiteral("gpu:Small Card"), devices);
        check(!f3.fits && f3.text.startsWith(QLatin1String("does not fit (needs")), QStringLiteral("a fixed card without room: does not fit, with the size"), f3.text);
        Fit f4 = fitOf(big, QStringLiteral("auto"), devices);
        check(!f4.fits && f4.text.contains(QLatin1String("needs")), QStringLiteral("a model larger than the memory: does not fit"), f4.text);
        QStringList choices;
        for (const auto &p : ai::deviceChoices(devices))
            choices << p.first;
        check(choices.contains(QStringLiteral("auto")) && choices.contains(QStringLiteral("cpu")) && choices.contains(QStringLiteral("gpu:Big Card")),
              QStringLiteral("the device choices: Automatic, the processor, each card"), choices.join(QLatin1Char(' ')));
    }

    std::printf("== the voice menu\n");
    {
        Settings s = Config::instance().get();
        s.voice = QStringLiteral("female2");
        s.style = QStringLiteral("calm");
        Config::instance().set(s);
        check(ctl->voiceButton()->toolTip() == QString::fromUtf8("Female 2 · Calm · Medium"), QStringLiteral("the tool tip follows the choice"), ctl->voiceButton()->toolTip());
        ctl->fillMenu();
        QMenu *m = ctl->voiceMenu();
        QStringList heads;
        bool noSub = true;
        for (QAction *a : m->actions()) {
            if (a->isSeparator() && !a->text().isEmpty())
                heads << a->text();
            noSub = noSub && !a->menu();
        }
        check(heads == QStringList({QStringLiteral("Voice"), QStringLiteral("Style"), QStringLiteral("Speed")}) && noSub,
              QStringLiteral("three groups with headings, no submenus"), heads.join(QLatin1Char(' ')));
        check(m->actions().last()->text() == QString::fromUtf8("Read Aloud Settings…"), QStringLiteral("\"Read Aloud Settings...\" at the bottom"));
        check(findAction(m, QStringLiteral("readAloud:female2"))->isChecked() && findAction(m, QStringLiteral("readAloud:calm"))->isChecked()
                  && findAction(m, QStringLiteral("readAloud:medium"))->isChecked(),
              QStringLiteral("the current choices are checked"));
        bool styleGrey = true;
        for (const QString &st : styleIds())
            styleGrey = styleGrey && !findAction(m, QStringLiteral("readAloud:") + st)->isEnabled()
                        && findAction(m, QStringLiteral("readAloud:") + st)->toolTip().contains(QLatin1String("no style control"));
        check(styleGrey, QStringLiteral("Pocket TTS has no style control: Style is greyed with the reason"));
        check(findAction(m, QStringLiteral("readAloud:female1"))->isEnabled() && !findAction(m, QStringLiteral("readAloud:male2"))->isEnabled()
                  && findAction(m, QStringLiteral("readAloud:male2"))->toolTip().contains(QLatin1String("does not have this voice")),
              QStringLiteral("a voice the model lacks is greyed with the reason"));
        bool speedOn = true;
        for (const QString &sp : speedIds())
            speedOn = speedOn && findAction(m, QStringLiteral("readAloud:") + sp)->isEnabled();
        check(speedOn, QStringLiteral("the speeds are always available"));
        m->setActiveAction(nullptr);
        m->popup(ctl->voiceButton()->mapToGlobal(QPoint(0, ctl->voiceButton()->height())));
        settle(200);
        m->grab().save(outDir + QStringLiteral("/voice-menu-pocket.png"));
        m->hide();

        findAction(m, QStringLiteral("readAloud:male1"))->trigger();
        check(Config::instance().get().voice == QLatin1String("male1") && QSettings().value(QStringLiteral("readaloud/voice")).toString() == QLatin1String("male1"),
              QStringLiteral("a choice is kept per device (the program's settings)"));
        ctl->fillMenu();
        findAction(ctl->voiceMenu(), QStringLiteral("readAloud:fast"))->trigger();
        check(Config::instance().get().speed == QLatin1String("fast"), QStringLiteral("the speed choice"));

        // Qwen3-TTS: no clips yet
        s = Config::instance().get();
        s.model = QStringLiteral("qwen3-tts-0.6b-q8");
        s.voice = QStringLiteral("female1");
        s.style = QStringLiteral("neutral");
        Config::instance().set(s);
        ctl->fillMenu();
        bool allGrey = true;
        for (const QString &v : voiceIds())
            allGrey = allGrey && !findAction(ctl->voiceMenu(), QStringLiteral("readAloud:") + v)->isEnabled()
                      && findAction(ctl->voiceMenu(), QStringLiteral("readAloud:") + v)->toolTip() == QLatin1String("Voice not installed yet.");
        check(allGrey, QStringLiteral("Qwen3-TTS before its voices exist: every voice greyed \"Voice not installed yet.\""));
        writeFile(ai::modelsRoot() + QStringLiteral("/read-aloud-voices/female1-neutral.wav"), wavFile(24000, 24000));
        writeFile(ai::modelsRoot() + QStringLiteral("/read-aloud-voices/female1-neutral.txt"), "A reference sentence.");
        ctl->fillMenu();
        check(findAction(ctl->voiceMenu(), QStringLiteral("readAloud:female1"))->isEnabled() && !findAction(ctl->voiceMenu(), QStringLiteral("readAloud:female2"))->isEnabled()
                  && findAction(ctl->voiceMenu(), QStringLiteral("readAloud:neutral"))->isEnabled() && !findAction(ctl->voiceMenu(), QStringLiteral("readAloud:calm"))->isEnabled(),
              QStringLiteral("with the clip of Female 1 (Neutral): that voice and style are on, the others greyed"));
        ctl->voiceMenu()->popup(ctl->voiceButton()->mapToGlobal(QPoint(0, ctl->voiceButton()->height())));
        settle(200);
        ctl->voiceMenu()->grab().save(outDir + QStringLiteral("/voice-menu-qwen.png"));
        ctl->voiceMenu()->hide();
        QVector<float> clip;
        int rate = 0;
        check(readWav(ai::modelsRoot() + QStringLiteral("/read-aloud-voices/female1-neutral.wav"), &clip, &rate) && rate == 24000 && clip.size() == 24000,
              QStringLiteral("a reference clip (WAV) is read"));
        s.model.clear();
        s.voice = QStringLiteral("female1");
        s.style = QStringLiteral("neutral");
        s.speed = QStringLiteral("medium");
        Config::instance().set(s);
    }

    // ======================================================================== the reading in the window
    std::printf("== reading: marked text, from the cursor, the marked sentence\n");
    {
        BinderPanel *binder = nullptr;
        for (BinderPanel *b : w.findChildren<BinderPanel *>()) {
            QWidget *par = b->parentWidget();
            if (par == static_cast<QWidget *>(w.paneSplitter()) || par == static_cast<QWidget *>(w.sideSplitter()))
                binder = b;
        }
        BinderItem *doc = firstDocument(projectPtr->draft());
        if (binder && doc)
            binder->select({doc}, true);
        settle(400);
    }
    TextEditor *ed = w.handwritingEditor();
    check(ed != nullptr, QStringLiteral("(an editor is there)"));
    if (!ed)
        return 1;
    const QString body = QStringLiteral("Alpha is first. Beta is second. Gamma is third.");
    ed->setPlainText(body);
    ed->setFocus();
    settle(100);
    // the data folder before reading, to see that nothing is written (no sound files)
    auto listing = [&] {
        QStringList l;
        QDirIterator it(home.path(), QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString f = it.next();
            if (!f.contains(QLatin1String("/config/")))
                l << f;
        }
        l.sort();
        return l;
    };
    const QStringList before = listing();
    QObject::connect(ctl, &Controller::message, [](const QString &t) { std::printf("      (message: %s)\n", qPrintable(t)); });
    struct Seen { QList<QPair<int, int>> sentences; };
    Seen seen;
    QMetaObject::Connection c = QObject::connect(&ctl->session(), &Session::sentence, [&](int a, int b) { if (b > 0) seen.sentences << qMakePair(a, b); });
    {
        QTextCursor cur = ed->textCursor();
        cur.setPosition(body.indexOf(QLatin1String("Beta")));
        ed->setTextCursor(cur);
        QSignalSpy fin(&ctl->session(), &Session::finished);
        setScale(8);
        ctl->playAction()->trigger();
        check(ctl->session().state() != Session::Idle, QStringLiteral("Play with nothing marked starts reading"));
        check(waitFor([&] { return !fin.isEmpty(); }), QStringLiteral("(the reading ends)"));
        check(seen.sentences.size() == 2 && seen.sentences[0].first == body.indexOf(QLatin1String("Beta")) && seen.sentences[1].first == body.indexOf(QLatin1String("Gamma"))
                  && fin.value(0).value(0).toBool(),
              QStringLiteral("with nothing marked: from the cursor to the end (Beta, Gamma), the sentences' places are the document's"),
              QString::number(seen.sentences.size()));
        check(ed->readingLength() == 0 && ctl->session().state() == Session::Idle && !ctl->stopAction()->isEnabled(),
              QStringLiteral("the mark is gone at the end, the buttons are back"));
        check(outWritten() > 0 && outRate() == 24000, QStringLiteral("sound reached the output (24 kHz)"));
        seen.sentences.clear();
        QTextCursor sel = ed->textCursor();
        sel.setPosition(body.indexOf(QLatin1String("Gamma")));
        sel.setPosition(body.size(), QTextCursor::KeepAnchor);
        ed->setTextCursor(sel);
        fin.clear();
        ctl->playAction()->trigger();
        check(waitFor([&] { return !fin.isEmpty(); }) && seen.sentences.size() == 1 && seen.sentences[0].first == body.indexOf(QLatin1String("Gamma")),
              QStringLiteral("marked text: only the marked sentence is read"));
        seen.sentences.clear();
    }
    QObject::disconnect(c);

    std::printf("== the marked sentence moves; pause; stop\n");
    {
        QTextCursor cur = ed->textCursor();
        cur.setPosition(0);
        ed->setTextCursor(cur);
        setScale(2);   // slow: a sentence takes a moment
        QList<int> marks;
        QMetaObject::Connection c2 = QObject::connect(&ctl->session(), &Session::sentence, [&](int a, int b) { if (b > 0) marks << ed->readingStart(); (void)a; });
        ctl->playAction()->trigger();
        check(waitFor([&] { return ctl->session().state() == Session::Playing; }), QStringLiteral("(playing)"));
        check(waitFor([&] { return ed->readingLength() > 0; }) && ed->readingStart() == 0, QStringLiteral("the first sentence is marked in the editor"));
        bool sel = false;
        for (const QTextEdit::ExtraSelection &x : ed->extraSelections())
            sel = sel || (x.cursor.selectionStart() == 0 && x.cursor.selectedText() == QLatin1String("Alpha is first."));
        check(sel, QStringLiteral("the mark is a highlight of exactly that sentence"));
        shot(&w, QStringLiteral("reading-marked-sentence"));
        check(ctl->playAction()->text() == QLatin1String("Pause"), QStringLiteral("while it plays the button is Pause"));
        ctl->playAction()->trigger();
        check(ctl->session().state() == Session::Paused && outPaused() && ctl->playAction()->text() == QLatin1String("Resume"),
              QStringLiteral("Pause pauses the sound"));
        const int at = ed->readingStart();
        settle(300);
        check(ed->readingStart() == at, QStringLiteral("the mark stands still while paused"));
        ctl->playAction()->trigger();
        check(ctl->session().state() == Session::Playing && !outPaused(), QStringLiteral("Play again goes on"));
        setScale(8);
        check(waitFor([&] { return ctl->session().state() == Session::Idle; }) && marks.size() >= 3 && marks[0] == 0 && marks[1] == body.indexOf(QLatin1String("Beta"))
                  && marks[2] == body.indexOf(QLatin1String("Gamma")),
              QStringLiteral("the mark moves sentence by sentence"), QString::number(marks.size()));
        QObject::disconnect(c2);
        // stop
        setScale(1);
        QSignalSpy fin(&ctl->session(), &Session::finished);
        ctl->playAction()->trigger();
        check(waitFor([&] { return ed->readingLength() > 0; }), QStringLiteral("(reading again)"));
        ctl->stopAction()->trigger();
        check(ctl->session().state() == Session::Idle && ed->readingLength() == 0 && outStopped() && !fin.isEmpty() && !fin.last().value(0).toBool(),
              QStringLiteral("Stop ends the reading, clears the mark and the sound"));
        setScale(40);
        // typing in the text stops a reading
        ctl->playAction()->trigger();
        check(waitFor([&] { return ctl->session().state() == Session::Playing; }), QStringLiteral("(reading, to be changed)"));
        setScale(0);
        ed->textCursor().insertText(QStringLiteral("x"));
        check(ctl->session().state() == Session::Idle, QStringLiteral("a change in the text stops the reading (the places would be wrong)"));
        setScale(40);
    }
    check(listing() == before, QStringLiteral("nothing is written to disk while reading (no sound file, no cache)"));
    {
        QStringList sounds;
        QDirIterator it(QDir::tempPath(), {QStringLiteral("*.wav"), QStringLiteral("*.ogg"), QStringLiteral("*.flac")}, QDir::Files);
        while (it.hasNext())
            sounds << it.next();
        check(sounds.isEmpty(), QStringLiteral("... and no sound file in the temporary folder"), sounds.join(QLatin1Char(' ')));
    }

    check(outLive() == 0 && outOpens() > 0, QStringLiteral("the sound goes through the AI engine's output; it is closed when the reading ends"),
          QStringLiteral("%1 open, %2 opened").arg(outLive()).arg(outOpens()));

    // ======================================================================== the model registry and the unload
    std::printf("== the memory rule\n");
    {
        int loaded = 0, loads = 0, dev = 0, thr = 0;
        stats(&loaded, &loads, &dev, &thr);
        const int loadsBefore = loads;
        check(mm.isLoaded(QStringLiteral("read-aloud:pocket-tts-en")) && loaded == 1, QStringLiteral("the model stays loaded after reading and is registered as \"read-aloud:pocket-tts-en\""));
        check(mm.models().value(0).name == QLatin1String("Pocket TTS English") && mm.models().value(0).where.startsWith(QLatin1String("gpu:")),
              QStringLiteral("ModelMemory knows it by its title, on the card (Automatic)"), mm.models().value(0).where);
        qint64 clockMs = 0;
        mm.setClock([&] { return clockMs; });
        mm.setIdleMs(1000);
        mm.modelUsed(QStringLiteral("read-aloud:pocket-tts-en"));
        clockMs += 5000;
        const QStringList gone = mm.unloadIdle();
        stats(&loaded, &loads, &dev, &thr);
        check(gone.contains(QStringLiteral("read-aloud:pocket-tts-en")) && loaded == 0 && !mm.isLoaded(QStringLiteral("read-aloud:pocket-tts-en")),
              QStringLiteral("unused for the set time (here 1 s instead of 10 minutes): the model is unloaded"));
        mm.setIdleMs(-1);
        mm.setClock(nullptr);
        QSignalSpy fin(&ctl->session(), &Session::finished);
        ed->setPlainText(QStringLiteral("One short sentence."));
        ctl->playAction()->trigger();
        check(waitFor([&] { return !fin.isEmpty(); }), QStringLiteral("(read again)"));
        stats(&loaded, &loads, &dev, &thr);
        check(loads == loadsBefore + 1 && dev > 0, QStringLiteral("the next Play loads it again, on the card"));
        ModelMemory::lowerThreadPriority();   // (no effect on the main thread's work below; only that the call exists)
    }

    // ======================================================================== look-ahead and cancel (the session by itself)
    std::printf("== a few sentences ahead; cancel\n");
    {
        Session::forgetModels();
        mm.reset();
        Session session;
        const QString text = QStringLiteral("One. Two. Three. Four. Five. Six. Seven. Eight. Nine. Ten.");
        Settings s;
        setScale(0);
        QString e1;
        check(session.start(text, 0, s, &e1), QStringLiteral("(a long text starts)"), e1);
        check(waitFor([&] { return session.generated() >= 2; }), QStringLiteral("(two sentences are made)"));
        settle(600);
        check(session.generated() == 2 && session.maxAhead() <= 2 && session.sentenceCount() == 10,
              QStringLiteral("with the sound standing still only 2 of 10 sentences are made ahead (never the whole text)"),
              QString::number(session.generated()));
        setScale(200);
        QSignalSpy fin(&session, &Session::finished);
        check(waitFor([&] { return !fin.isEmpty(); }) && session.generated() == 10 && session.maxAhead() <= 2,
              QStringLiteral("then the rest follows as the sound plays, never more than 2 ahead"),
              QStringLiteral("generated %1 maxAhead %2 state %3").arg(session.generated()).arg(session.maxAhead()).arg(int(session.state())));

        rememberSpeed(QStringLiteral("pocket-tts-en"), QStringLiteral("auto"), 2.5);
        s.model = QStringLiteral("pocket-tts-en");
        setScale(0);
        check(session.start(text, 0, s, &e1), QStringLiteral("(starts again, the model is slower than real time)"), e1);
        check(session.lookAhead().ahead == 4 && session.lookAhead().prebuffer == 3, QStringLiteral("a model slower than real time: 4 ahead, 3 ready before the sound"));
        check(waitFor([&] { return session.generated() >= 4; }), QStringLiteral("(four are made)"));
        settle(600);
        check(session.generated() == 4 && session.maxAhead() <= 4 && outWritten() >= 3 * 4800, QStringLiteral("it makes 4 ahead and no more; the sound started with 3 ready"),
              QStringLiteral("%1 %2").arg(session.generated()).arg(outWritten()));
        rememberSpeed(QStringLiteral("pocket-tts-en"), QStringLiteral("auto"), 0.0);
        // cancel while the worker is busy
        session.stop();
        check(session.state() == Session::Idle, QStringLiteral("stop() ends it"));
        qputenv("QBAI_FAKE_SLOW_MS", "150");
        setScale(40);
        QSignalSpy marks(&session, &Session::sentence);
        s.model.clear();
        check(session.start(text, 0, s, &e1), QStringLiteral("(starts with a slow engine)"), e1);
        settle(250);
        QElapsedTimer t;
        t.start();
        session.stop();
        const int n = int(marks.size());
        const qint64 took = t.elapsed();
        settle(1500);
        check(session.state() == Session::Idle && int(marks.size()) == n && took < 1000,
              QStringLiteral("cancel while a sentence is being made: stops at once, no further sentence is announced"),
              QStringLiteral("state %1, signals %2 then %3, took %4 ms").arg(int(session.state())).arg(n).arg(marks.size()).arg(took));
        qunsetenv("QBAI_FAKE_SLOW_MS");
        int loaded = 0, loads = 0, dev = 0, thr = 0;
        stats(&loaded, &loads, &dev, &thr);
        check(loaded <= 1, QStringLiteral("(no model is loaded twice)"), QString::number(loaded));
        // no sound device: told, nothing keeps running
        qputenv("QBAI_FAKE_NO_SOUND", "1");
        QSignalSpy failed(&session, &Session::failed);
        setScale(200);
        check(session.start(QStringLiteral("Short one."), 0, s, &e1) && waitFor([&] { return !failed.isEmpty(); }) && session.state() == Session::Idle
                  && failed.value(0).value(0).toString().contains(QLatin1String("no sound output")),
              QStringLiteral("no sound output: the reading ends and the reason is told"), failed.value(0).value(0).toString());
        qunsetenv("QBAI_FAKE_NO_SOUND");
        // an unknown voice of the model: told
        s.voice = QStringLiteral("male2");
        QString e2;
        check(!session.start(text, 0, s, &e2) && e2.contains(QLatin1String("does not have this voice")), QStringLiteral("a voice the model lacks cannot start; the reason is told"), e2);
        s.voice = QStringLiteral("female1");
        // the device choice reaches the model
        s.device = QStringLiteral("cpu");
        Session::forgetModels();
        mm.reset();
        QSignalSpy fin2(&session, &Session::finished);
        setScale(200);
        check(session.start(QStringLiteral("Short one."), 0, s, &e2) && waitFor([&] { return !fin2.isEmpty(); }), QStringLiteral("(reads on the processor)"), e2);
        stats(&loaded, &loads, &dev, &thr);
        check(dev == 0, QStringLiteral("the device choice \"cpu\" is used"), QString::number(dev));
        s.device = QStringLiteral("gpu:Big Card");
        Session::forgetModels();
        mm.reset();
        fin2.clear();
        check(session.start(QStringLiteral("Short one."), 0, s, &e2) && waitFor([&] { return !fin2.isEmpty(); }), QStringLiteral("(reads on a chosen card)"), e2);
        stats(&loaded, &loads, &dev, &thr);
        check(dev > 0, QStringLiteral("a fixed card is used"), QString::number(dev));
        Session::forgetModels();
        mm.reset();
    }

    // ======================================================================== the page
    std::printf("== the settings page\n");
    {
        Config::instance().reload();
        Settings s = Config::instance().get();
        s.device = QStringLiteral("auto");
        s.model.clear();
        s.language.clear();
        Config::instance().set(s);
        auto *page = new ReadAloudPage;
        page->resize(900, 760);
        page->show();
        settle(400);
        QComboBox *lang = page->findChild<QComboBox *>(QStringLiteral("raLanguage"));
        QComboBox *model = page->findChild<QComboBox *>(QStringLiteral("raModel"));
        QComboBox *device = page->findChild<QComboBox *>(QStringLiteral("raDevice"));
        QLabel *speed = page->findChild<QLabel *>(QStringLiteral("raSpeed"));
        check(lang && model && device && speed && page->findChild<QWidget *>(QStringLiteral("aiInstallPanel")),
              QStringLiteral("the page has language, model, device, the speed and the install panel (engine and models)"));
        QStringList greyed;
        auto *lm = qobject_cast<QStandardItemModel *>(lang->model());
        for (int i = 0; i < lang->count(); ++i)
            if (!lm->item(i)->isEnabled())
                greyed << lang->itemData(i).toString();
        greyed.sort();
        bool says = true;
        for (int i = 0; i < lang->count(); ++i)
            if (greyed.contains(lang->itemData(i).toString()))
                says = says && lang->itemText(i).contains(QLatin1String("no model for this language yet"));
        check(greyed == QStringList({QStringLiteral("id"), QStringLiteral("nl"), QStringLiteral("pl"), QStringLiteral("tr")}) && says,
              QStringLiteral("languages: id, tr, pl and nl are greyed with \"no model for this language yet\""), greyed.join(QLatin1Char(' ')));
        auto *mmod = qobject_cast<QStandardItemModel *>(model->model());
        QStringList lines;
        for (int i = 0; i < model->count(); ++i)
            lines << model->itemText(i) + (mmod->item(i)->isEnabled() ? QString() : QStringLiteral(" [grey]"));
        check(model->itemText(0).startsWith(QLatin1String("Automatic (Pocket TTS English")) && lines.join(QLatin1Char('\n')).contains(QLatin1String("fits on Big Card"))
                  && lines.join(QLatin1Char('\n')).contains(QLatin1String("fits in RAM")) == false,
              QStringLiteral("models: Automatic with the default named; each shows its fit on this computer"), lines.join(QLatin1Char('|')));
        // German: the big model does not fit: greyed
        lang->setCurrentIndex(lang->findData(QStringLiteral("de")));
        settle(300);
        bool bigGrey = false;
        for (int i = 0; i < model->count(); ++i)
            if (model->itemText(i).contains(QLatin1String("1.7B")))
                bigGrey = !mmod->item(i)->isEnabled() && model->itemText(i).contains(QLatin1String("does not fit (needs"));
        check(bigGrey, QStringLiteral("a model that does not fit is greyed with \"does not fit\" and the size it needs"));
        lang->setCurrentIndex(lang->findData(QStringLiteral("en")));
        settle(300);
        // the speed test
        qputenv("QBAI_FAKE_SLOW_MS", "120");
        model->setCurrentIndex(model->findData(QStringLiteral("qwen3-tts-0.6b-q8")));
        check(waitFor([&] { return speed->text().contains(QLatin1String("Slower than real time")); }),
              QStringLiteral("a model is chosen: the test sentence is measured; slower than real time is said, with the consequence"), speed->text());
        check(speed->text().contains(QLatin1String("a few sentences")) && measuredSpeed(QStringLiteral("qwen3-tts-0.6b-q8"), QStringLiteral("auto")) > 1.0,
              QStringLiteral("... \"generates a few sentences ahead before it plays\", and the figure is kept"));
        shot(page, QStringLiteral("settings-page-slow"));
        qunsetenv("QBAI_FAKE_SLOW_MS");
        model->setCurrentIndex(model->findData(QStringLiteral("pocket-tts-en")));
        check(waitFor([&] { return speed->text().startsWith(QLatin1String("Faster than real time")); }), QStringLiteral("a fast model: faster than real time"), speed->text());
        // device
        QStringList dv;
        for (int i = 0; i < device->count(); ++i)
            dv << device->itemData(i).toString();
        check(dv.contains(QStringLiteral("auto")) && dv.contains(QStringLiteral("cpu")) && dv.contains(QStringLiteral("gpu:Small Card")), QStringLiteral("the device list: Automatic, processor, the fake cards"), dv.join(QLatin1Char(' ')));
        device->setCurrentIndex(device->findData(QStringLiteral("gpu:Small Card")));
        settle(300);
        bool smallGrey = false;
        for (int i = 0; i < model->count(); ++i)
            if (model->itemData(i).toString() == QLatin1String("pocket-tts-en"))
                smallGrey = !mmod->item(i)->isEnabled();
        check(smallGrey, QStringLiteral("a fixed card with too little free memory greys the model"));
        device->setCurrentIndex(device->findData(QStringLiteral("auto")));
        settle(200);
        bool hint = false;
        for (QLabel *l : page->findChildren<QLabel *>())
            hint = hint || l->text().contains(QLatin1String("General > AI Models"));
        check(hint, QStringLiteral("a pointer to the shared AI settings (cores, unload)"));
        shot(page, QStringLiteral("settings-page"));
        model->setCurrentIndex(model->findData(QStringLiteral("qwen3-tts-0.6b-q8")));
        lang->setCurrentIndex(lang->findData(QStringLiteral("de")));
        settle(200);
        page->apply();
        check(Config::instance().get().language == QLatin1String("de") && Config::instance().get().voice == QLatin1String("female1")
                  && QSettings().value(QStringLiteral("readaloud/language")).toString() == QLatin1String("de"),
              QStringLiteral("OK keeps the page's choices per device and leaves the voice choices alone"));
        delete page;
        s = Config::instance().get();
        s.language.clear();
        s.model.clear();
        Config::instance().set(s);
    }

    // ======================================================================== the toolbar by system and size
    std::printf("== the toolbar by system and size\n");
    {
        QAction *large = w.findChild<QAction *>(QStringLiteral("toolbarSizeLarge"));
        if (large) {
            large->trigger();
            settle(300);
            bool all = w.readAloudBar()->iconSize() == QSize(32, 32) && ctl->voiceButton()->iconSize() == QSize(32, 32);
            check(all, QStringLiteral("the buttons follow the toolbar size setting"), QStringLiteral("%1").arg(w.readAloudBar()->iconSize().width()));
            shot(&w, QStringLiteral("toolbar-large"));
            w.findChild<QAction *>(QStringLiteral("toolbarSizeNormal"))->trigger();
            settle(200);
        }
        QAction *show = w.findChild<QAction *>(QStringLiteral("showBarReadAloud"));
        show->trigger();
        settle(200);
        check(!w.readAloudBar()->isVisible() && !Config::instance().get().barShown, QStringLiteral("View > Toolbars > Read Aloud hides the toolbar"));
        show->trigger();
        settle(200);
        check(w.readAloudBar()->isVisible(), QStringLiteral("... and shows it again"));
    }
    delete wp;
    settle(200);
    {
        QSettings().remove(QStringLiteral("readaloud"));
        Config::forceAndroid(1);
        Config::instance().reload();
        auto *wa = new MainWindow;
        wa->resize(1400, 800);
        wa->show();
        settle(500);
        check(wa->readAloudBar() && !wa->readAloudBar()->isVisible() && !wa->findChild<QAction *>(QStringLiteral("showBarReadAloud"))->isChecked(),
              QStringLiteral("Android (forced by the test switch): the toolbar is hidden by default; the switch is unchecked"));
        wa->findChild<QAction *>(QStringLiteral("showBarReadAloud"))->trigger();
        settle(200);
        check(wa->readAloudBar()->isVisible(), QStringLiteral("... and can be shown"));
        delete wa;
        Config::forceAndroid(-1);
    }

    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
