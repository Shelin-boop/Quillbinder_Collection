// Scripted run for 0.99.0, Speech Transcription 1.10: the recognition through the program's AI engine (a fake libqbai stands in
// for it: tests/fakeqbai), the device list with the graphics cards, the one place for the AI settings, the model registry
// (ToolModels::unloadModel), "Insert at Cursor" (one undo step), "Write into the text as I speak", keepRunning around a
// recording, Android's default (a test switch), and what the add-on's minApp means for the update file.
// Needs <build>/tool-plugins (the add-on's two libraries) and <build>/tests/fake-ai-engine/libqbai.so.1.
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0990m_tests /tmp/shots
#include "AppInfo.h"
#include "compile/PluginManager.h"
#include "core/Addons.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/KeepAlive.h"
#include "core/ModelMemory.h"
#include "core/Paths.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/UpdateCheck.h"
#include "tools/ToolApi.h"
#include "ui/AppSettings.h"
#include "ui/EditorPane.h"
#include "ui/MainWindow.h"
#include "ui/OptionsDialog.h"
#include "ui/TextEditor.h"
#include "ui/Theme.h"
#include "ui/ToolHost.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLibrary>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QToolButton>
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
            std::printf("      %s\n", qPrintable(detail.left(3000)));
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
template <typename F>
static bool waitFor(F cond, int ms)
{
    QElapsedTimer t;
    t.start();
    while (!cond() && t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(20);
    }
    return cond();
}

static QObject *speechTool()
{
    for (tools::ToolInterface *t : compile::PluginManager::instance().tools())
        if (t->id() == QLatin1String("speech-transcription"))
            return dynamic_cast<QObject *>(t);
    return nullptr;
}
static bool callBool(QObject *o, const char *slot)
{
    bool r = false;
    QMetaObject::invokeMethod(o, slot, Qt::DirectConnection, Q_RETURN_ARG(bool, r));
    return r;
}
static QString callString(QObject *o, const char *slot)
{
    QString r;
    QMetaObject::invokeMethod(o, slot, Qt::DirectConnection, Q_RETURN_ARG(QString, r));
    return r;
}
static void setSpeech(const QString &key, const QVariant &v)
{
    toolhost::host().setSetting(QStringLiteral("speech-transcription"), key, v);
}
static QString readFile(const QString &f)
{
    QFile file(f);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
}

using StatsFn = void (*)(int *, int *, int *, int *);
static StatsFn statsFn;
struct Stats {
    int loaded = 0, loads = 0, device = -1, threads = 0;
};
static Stats stats()
{
    Stats s;
    if (statsFn)
        statsFn(&s.loaded, &s.loads, &s.device, &s.threads);
    return s;
}

// a live recording from the sample, run to its end; true when it started
static bool runLive(QObject *tool, const QString &wav, bool *keepSeen = nullptr)
{
    bool started = false;
    QMetaObject::invokeMethod(tool, "startRecording", Qt::DirectConnection, Q_RETURN_ARG(bool, started), Q_ARG(QString, wav));
    waitFor([&] {
        if (keepSeen && keepalive::reasons().contains(QStringLiteral("recording")))
            *keepSeen = true;
        return !callBool(tool, "isBusy") && started;
    }, 60000);
    settle(200);
    return started;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QB_SPEECH_FAST", "1");
    const QString buildDir = QStringLiteral(QB_TEST_BUILD_DIR);
    const QString fakeLib = buildDir + QStringLiteral("/tests/fake-ai-engine/libqbai.so.1");
    // (the model check of the add-on refuses English-only models, so the tiny English model is used for a live recording only)
    // QB_V0990M_REAL=<whisper ggml model>: one real transcription through the real engine (<build>/qbai/libqbai.so.1) instead
    // of the fake one (the tiny English model is enough; never committed)
    const QString realModel = qEnvironmentVariable("QB_V0990M_REAL");
    const QString realLib = buildDir + QStringLiteral("/qbai/libqbai.so.1");
    qputenv("QB_AI_ENGINE_LIB", (realModel.isEmpty() ? fakeLib : realLib).toUtf8());
    qputenv("QBAI_FAKE_CARDS", "Card A,8192,6000;Card B,2048,1500");
    qputenv("QB_PLUGIN_PATH", (buildDir + QStringLiteral("/tool-plugins")).toUtf8());
    qputenv("QB_SPEECH_DIR", (buildDir + QStringLiteral("/tool-plugins/speech")).toUtf8());
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990mTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990mTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    QLibrary fake(fakeLib);
    statsFn = reinterpret_cast<StatsFn>(fake.resolve("qbai_fake_stats"));
    check(QFile::exists(fakeLib) && statsFn, QStringLiteral("(the fake AI engine is built)"));

    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.save();
    theme::apply();
    compile::PluginManager::instance().load();
    QObject *tool = speechTool();
    check(tool != nullptr, QStringLiteral("the writing tool \"Speech Transcription\" is loaded"),
          compile::PluginManager::instance().messages().join(QLatin1String("; ")));
    if (!tool)
        return failures ? failures : 1;
    auto *ti = dynamic_cast<tools::ToolInterface *>(tool);
    auto *tm = qobject_cast<tools::ToolModels *>(tool);
    check(ti && ti->version() == QLatin1String("1.10") && tm && toolhost::host().apiVersion() >= 130,
          QStringLiteral("version 1.10; it answers the program's model registry (ToolModels); the program offers interface 1.3"),
          ti ? ti->version() : QString());

    if (!realModel.isEmpty()) {
        toolhost::host().attachTools();   // as the window does at its start
        setSpeech(QStringLiteral("modelFile"), realModel);
        setSpeech(QStringLiteral("model"), QStringLiteral("small"));
        setSpeech(QStringLiteral("mode"), QStringLiteral("live"));
        setSpeech(QStringLiteral("device"), QStringLiteral("cpu"));
        setSpeech(QStringLiteral("main"), QStringLiteral("en"));
        setSpeech(QStringLiteral("fallback"), qEnvironmentVariable("QB_V0990M_FALLBACK"));
        setSpeech(QStringLiteral("hint"), qEnvironmentVariable("QB_V0990M_HINT"));
        const QString sample = buildDir + QStringLiteral("/tool-plugins/speech/samples/jfk.wav");
        const bool started = runLive(tool, sample);
        const QString text = readFile(callString(tool, "recordingFile"));
        check(started && text.contains(QLatin1String("country"), Qt::CaseInsensitive),
              QStringLiteral("real engine, real tiny model: the test recording is written as text by a live recording"),
              text + callString(tool, "lastError"));
        std::printf("NOTE  %s\n", qPrintable(text.simplified()));
        std::printf("\n%d failure(s)\n", failures);
        return failures;
    }

    // ---- minApp: the package, and what the update file does with it
    {
        QFile mf(QStringLiteral(QB_TEST_SOURCE_DIR "/addons/speech-transcription/manifest.json"));
        check(mf.open(QIODevice::ReadOnly) && mf.readAll().contains("\"minApp\": \"0.99.0\""),
              QStringLiteral("the add-on's manifest needs Quillbinder 0.99.0 (its whisper comes from the AI Engine package)"));
        using namespace updates;
        auto fileEntry = [](const QString &name) {
            return QJsonObject{{"url", QStringLiteral("https://example.org/") + name}, {"name", name}, {"sha256", QString(64, QLatin1Char('a'))},
                               {"size", 10}};
        };
        auto feedFor = [&](const QString &programVersion) {
            QJsonObject program{{"version", programVersion}, {"runtime", QStringLiteral(QB_RUNTIME_ID)}, {"update", fileEntry(QStringLiteral("u.qbpkg"))},
                                {"appimage", fileEntry(QStringLiteral("a.AppImage"))}};
            QJsonObject addon{{"id", QStringLiteral("speech-transcription")}, {"name", QStringLiteral("Speech Transcription")},
                              {"version", QStringLiteral("1.10")}, {"minApp", QStringLiteral("0.99.0")}, {"file", fileEntry(QStringLiteral("s.qbpkg"))}};
            Feed f;
            f.format = 1;
            f.systems.insert(systemName(), QJsonObject{{"program", program}, {"addons", QJsonArray{addon}}});
            return f;
        };
        auto speechOffered = [](const QList<Offer> &o) {
            for (const Offer &x : o)
                if (x.id == QLatin1String("speech-transcription"))
                    return true;
            return false;
        };
        Installed here;
        here.runtime = QStringLiteral(QB_RUNTIME_ID);
        here.addons = {{QStringLiteral("speech-transcription"), QStringLiteral("1.9")}};
        here.appVersion = QStringLiteral("0.95.0");
        const bool oldKeeps = !speechOffered(offersFor(feedFor(QStringLiteral("0.95.0")), systemName(), here));
        const bool oldWithProgram = speechOffered(offersFor(feedFor(QStringLiteral("0.99.0")), systemName(), here));
        Installed other = here;
        other.runtime = QStringLiteral("another-runtime");
        const bool newImage = !speechOffered(offersFor(feedFor(QStringLiteral("0.99.0")), systemName(), other));
        here.appVersion = QStringLiteral("0.99.0");
        const bool current = speechOffered(offersFor(feedFor(QStringLiteral("0.99.0")), systemName(), here));
        check(oldKeeps && oldWithProgram && newImage && current,
              QStringLiteral("update file: 1.10 (minApp 0.99.0) is offered to 0.99.0, and to 0.95.0 only together with the program "
                             "update to 0.99.0; a 0.95.0 user without it, or whose program needs a new AppImage, keeps 1.9"),
              QStringLiteral("%1 %2 %3 %4").arg(oldKeeps).arg(oldWithProgram).arg(newImage).arg(current));
    }

    // ---- a project and its editor
    QTemporaryDir work;
    templates::Info blank;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("blank"))
            blank = t;
    QString err;
    auto proj = templates::createProject(blank, work.path(), QStringLiteral("Dictation"), &err);
    if (!proj) {
        check(false, QStringLiteral("(project made)"), err);
        return failures;
    }
    Project *p = proj.get();
    BinderItem *doc = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Chapter"));
    QTextCursor(p->docs()->text(doc->id)).insertText(QStringLiteral("The lamp. "));
    p->save(&err);
    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.move(0, 0);
    w.resize(1500, 900);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(600);
    TextEditor *editor = nullptr;
    for (EditorPane *ep : w.findChildren<EditorPane *>())
        if (ep->isVisible()) {
            ep->showItems({doc}, false, int(ViewMode::Single));
            settle(300);
            editor = ep->currentEditor();
        }
    check(editor != nullptr, QStringLiteral("(an editor is open)"));
    toolhost::host().runCommand(QStringLiteral("speech-transcription.show"));
    settle(300);
    auto *panel = w.findChild<QWidget *>(QStringLiteral("speechPanel"));
    check(panel != nullptr, QStringLiteral("(the Transcriptions tab is there)"));

    // ---- the options: the program's panel, the one place for the AI settings, the device list
    {
        QWidget host;
        tools::OptionsSection *sec = ti->createOptions(&host);
        QWidget *page = sec ? sec->widget() : nullptr;
        check(page != nullptr, QStringLiteral("(the options page is made)"));
        if (page) {
            host.resize(900, 700);
            auto *aiPanel = page->findChild<QWidget *>(QStringLiteral("speechAiPanel"));
            check(aiPanel != nullptr, QStringLiteral("the options show the program's AI panel (engine and models of the catalog)"));
            auto *shared = page->findChild<QLabel *>(QStringLiteral("speechSharedSettings"));
            check(!page->findChild<QSpinBox *>(QStringLiteral("speechThreads")) && !page->findChild<QSpinBox *>(QStringLiteral("speechUnload"))
                      && !page->findChild<QComboBox *>(QStringLiteral("speechAccuracy")) && shared
                      && shared->text().contains(QLatin1String("File > Options > General > AI Models")),
                  QStringLiteral("no fields of its own for the cores and the time to free the model: the page points to General > AI Models"));
            auto *device = page->findChild<QComboBox *>(QStringLiteral("speechDevice"));
            QStringList data, names;
            for (int i = 0; device && i < device->count(); ++i) {
                data << device->itemData(i).toString();
                names << device->itemText(i);
            }
            check(device && data == QStringList({QStringLiteral("auto"), QStringLiteral("cpu"), QStringLiteral("gpu:Card A"), QStringLiteral("gpu:Card B")})
                      && names.value(0) == QLatin1String("Automatic") && names.value(2) == QLatin1String("Card A"),
                  QStringLiteral("compute on: Automatic, the processor and the graphics cards of the engine's device list"),
                  names.join(QLatin1String(" | ")) + QLatin1String(" / ") + data.join(QLatin1String(" | ")));
            check(device && device->currentData().toString() == QLatin1String("auto"), QStringLiteral("... Automatic at first"));
            if (device)
                device->setCurrentIndex(device->findData(QStringLiteral("gpu:Card B")));
            auto *writeInto = page->findChild<QCheckBox *>(QStringLiteral("speechWriteIntoText"));
            check(writeInto && !writeInto->isChecked(), QStringLiteral("\"Write into the text as I speak\" is off at first"));
            if (writeInto)
                writeInto->setChecked(true);
            sec->apply();
            check(toolhost::host().setting(QStringLiteral("speech-transcription"), QStringLiteral("device")).toString() == QLatin1String("gpu:Card B")
                      && toolhost::host().setting(QStringLiteral("speech-transcription"), QStringLiteral("writeIntoText")).toBool(),
                  QStringLiteral("Apply keeps the chosen card and the writing into the text"));
            host.show();
            settle(300);
            host.grab().save(outDir + QStringLiteral("/01-options.png"));
            if (device)
                device->setCurrentIndex(device->findData(QStringLiteral("auto")));
            if (writeInto)
                writeInto->setChecked(false);
            sec->apply();
        }
        delete sec;
    }

    // ---- a model file (the engine is the fake: any file that does not begin with FAIL loads)
    const QString modelFile = home.path() + QStringLiteral("/ggml-test.bin");
    {
        QFile f(modelFile);
        f.open(QIODevice::WriteOnly);
        f.write(QByteArray(20000, 'x'));
    }
    setSpeech(QStringLiteral("modelFile"), modelFile);
    setSpeech(QStringLiteral("model"), QStringLiteral("small"));
    setSpeech(QStringLiteral("mode"), QStringLiteral("live"));
    const QString wav = buildDir + QStringLiteral("/tool-plugins/speech/samples/jfk.wav");
    check(QFile::exists(wav), QStringLiteral("(the test recording is there)"), wav);

    // ---- a live recording through the engine, Automatic: the card with the most free memory; keepRunning; the registry
    {
        bool keep = false;
        const bool started = runLive(tool, wav, &keep);
        const QString text = readFile(callString(tool, "recordingFile"));
        const Stats s = stats();
        check(started && text.contains(QLatin1String("quick brown fox")),
              QStringLiteral("a live recording is written through the AI engine (the fake's text)"), text.left(200) + callString(tool, "lastError"));
        check(s.loads == 1 && s.loaded == 1 && s.device == 1,
              QStringLiteral("Automatic: the model is on the graphics card with the most free memory (device 1)"),
              QStringLiteral("loads %1 loaded %2 device %3").arg(s.loads).arg(s.loaded).arg(s.device));
        check(modelmem::ModelMemory::instance().isLoaded(QStringLiteral("speech-transcription:small")),
              QStringLiteral("the model is in the program's list of loaded models, under the add-on's name"));
        check(keep && !keepalive::reasons().contains(QStringLiteral("recording")),
              QStringLiteral("keepRunning(\"recording\") is on while recording and off after it"));
        check(s.threads >= 1, QStringLiteral("the engine runs with the program's shared number of cores"), QString::number(s.threads));
        // a second recording: the model stays loaded (not loaded again)
        runLive(tool, wav);
        check(stats().loads == 1, QStringLiteral("the next recording uses the loaded model"), QString::number(stats().loads));
        // the program's rule asks to unload it
        check(tm->unloadModel(QStringLiteral("speech-transcription:small")) && stats().loaded == 0,
              QStringLiteral("the program's request to unload the model frees it (ToolModels::unloadModel)"));
        modelmem::ModelMemory::instance().modelUnloaded(QStringLiteral("speech-transcription:small"));
        check(tm->unloadModel(QStringLiteral("speech-transcription:other")),
              QStringLiteral("... and a request for a model it does not hold is answered \"gone\""));
    }
    // ---- the chosen card, and the processor
    {
        setSpeech(QStringLiteral("device"), QStringLiteral("gpu:Card B"));
        runLive(tool, wav);
        check(stats().device == 2 && stats().loaded == 1, QStringLiteral("a chosen card is used (device 2)"), QString::number(stats().device));
        tm->unloadModel(QStringLiteral("speech-transcription:small"));
        modelmem::ModelMemory::instance().modelUnloaded(QStringLiteral("speech-transcription:small"));
        setSpeech(QStringLiteral("device"), QStringLiteral("cpu"));
        runLive(tool, wav);
        check(stats().device == 0, QStringLiteral("the processor when chosen (device 0)"), QString::number(stats().device));
        tm->unloadModel(QStringLiteral("speech-transcription:small"));
        modelmem::ModelMemory::instance().modelUnloaded(QStringLiteral("speech-transcription:small"));
        setSpeech(QStringLiteral("device"), QStringLiteral("auto"));
    }
    // ---- Insert at Cursor
    {
        auto *edit = panel ? panel->findChild<QPlainTextEdit *>(QStringLiteral("speechText")) : nullptr;
        auto *insert = panel ? panel->findChild<QToolButton *>(QStringLiteral("speechInsert")) : nullptr;
        check(insert != nullptr && edit != nullptr, QStringLiteral("the Transcriptions tab has the button Insert at Cursor"));
        if (insert && edit && editor) {
            QMetaObject::invokeMethod(panel, "setText", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("First part. Second part.\n\nThird part.")));
            settle(200);
            editor->setFocus();
            QTextCursor c = editor->textCursor();
            c.movePosition(QTextCursor::End);
            editor->setTextCursor(c);
            const QString before = editor->toPlainText();
            // the whole text (nothing marked)
            QMetaObject::invokeMethod(panel, "insertAtCursor", Qt::DirectConnection);
            settle(150);
            check(editor->toPlainText() == before + QStringLiteral("First part. Second part.\n\nThird part."),
                  QStringLiteral("Insert at Cursor puts the whole text at the cursor when nothing is marked"), editor->toPlainText());
            editor->undo();
            check(editor->toPlainText() == before, QStringLiteral("... as one undo step (a paragraph break inside included)"), editor->toPlainText());
            // a marked part
            QTextCursor sel = edit->textCursor();
            sel.setPosition(12);
            sel.setPosition(24, QTextCursor::KeepAnchor);
            edit->setTextCursor(sel);
            const QString marked = sel.selectedText();
            c = editor->textCursor();
            c.movePosition(QTextCursor::End);
            editor->setTextCursor(c);
            insert->click();
            settle(150);
            check(!marked.isEmpty() && editor->toPlainText().startsWith(before.trimmed()) && editor->toPlainText().endsWith(marked.trimmed()) && !editor->toPlainText().contains(QLatin1String("First part")), QStringLiteral("... only the marked part when something is marked"),
                  editor->toPlainText().toUtf8().toHex() + QLatin1String(" / ") + marked.toUtf8().toHex() + QLatin1String(" / ") + before.toUtf8().toHex());
            editor->undo();
            check(editor->toPlainText() == before, QStringLiteral("... also one undo step"), editor->toPlainText());
            w.grab().save(outDir + QStringLiteral("/02-insert.png"));
        }
    }

    // ---- Write into the text as I speak
    if (editor) {
        const QString before = editor->toPlainText();
        runLive(tool, wav);
        check(editor->toPlainText() == before, QStringLiteral("\"Write into the text as I speak\" off: the text in the editor stays as it was"),
              editor->toPlainText());
        editor->setFocus();
        QTextCursor c = editor->textCursor();
        c.movePosition(QTextCursor::End);
        editor->setTextCursor(c);
        setSpeech(QStringLiteral("writeIntoText"), true);
        runLive(tool, wav);
        const QString kept = readFile(callString(tool, "recordingFile"));
        check(editor->toPlainText().contains(QLatin1String("quick brown fox")) && kept.contains(QLatin1String("quick brown fox")),
              QStringLiteral("on: the pieces go into the text at the cursor, and the transcription is still kept in the tab"),
              editor->toPlainText());
        w.grab().save(outDir + QStringLiteral("/03-write-as-I-speak.png"));
        setSpeech(QStringLiteral("writeIntoText"), false);
    }

    // ---- Android's standard: Record only (the test switch stands in for the system)
    {
        setSpeech(QStringLiteral("mode"), QVariant());
        check(!callBool(tool, "recordOnly"), QStringLiteral("Linux: Transcribe live is the standard"));
        qputenv("QB_SPEECH_ANDROID", "1");
        check(callBool(tool, "recordOnly"), QStringLiteral("Android: Record only is the standard"));
        setSpeech(QStringLiteral("mode"), QStringLiteral("live"));
        check(!callBool(tool, "recordOnly"), QStringLiteral("... and what the writer chose counts"));
        qunsetenv("QB_SPEECH_ANDROID");
    }

    delete wp;
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
