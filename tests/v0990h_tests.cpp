// 0.99.0, the AI engine packages (decision 3; items 114, 115, 117, 119, 110): the C interface (tools/QbAi.h), the program's
// side (core/AiEngine.*), the device choice, the registration with ModelMemory, cancelling, the catalog of the models
// (core/AiCatalog.*), installing the engine and the models (ui/AiDownloads.*, ui/AiInstallPanel.*) from a local file and
// from a local "release", tool interface 1.3's engine functions, and the package recipes (addons/ai-engine,
// addons/handwriting-engine, tools/check-engine-package.py).
//
// Most of it runs against a fake libqbai built by the tests (tests/fakeqbai: a sine for speech, a fixed text for
// recognition, a fake graphics card). One real smoke test runs in a child process against the session-built engine when
// it and whisper.cpp's tiny English model are at hand:
//   QB_AI_REAL_ENGINE=<build>/qbai/libqbai.so.1 (the default when built)  QB_AI_WHISPER_MODEL=<ggml-tiny.en.bin>
//   QB_AI_WHISPER_SAMPLE=<jfk.wav> (default: build/whisper/samples/jfk.wav)
//   QB_AI_QWEN_TALKER=<talker .gguf> QB_AI_QWEN_CODEC=<tokenizer .gguf>: also a sentence with Qwen3-TTS (slow, about 3 GB)
// The package recipe check needs the pack (QB_AI_ENGINE_DIR or build/ai-engine) and libqbai built.
//
//   xvfb-run -a ./qb_v0990h_tests /tmp/shots
#include "AppInfo.h"
#include "core/Addons.h"
#include "core/AiCatalog.h"
#include "core/AiEngine.h"
#include "core/ModelMemory.h"
#include "core/ReadAloud.h"
#include "core/SystemMemory.h"
#include "core/Zip.h"
#include "ui/AiDownloads.h"
#include "ui/AiInstallPanel.h"
#include "ui/AppSettings.h"
#include "ui/Theme.h"
#include "ui/ToolHost.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLibrary>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <QTimer>
#include <QTreeWidget>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <functional>

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
static void info(const QString &text)
{
    std::printf("      %s\n", qPrintable(text));
    std::fflush(stdout);
}

static const QString kSource = QStringLiteral(QB_TEST_SOURCE_DIR);
static const QString kBuild = QStringLiteral(QB_TEST_BUILD_DIR);
static const qint64 MB = 1024 * 1024;

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
static bool writeFile(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(data) == data.size();
}
static QString sha(const QByteArray &d) { return QString::fromLatin1(QCryptographicHash::hash(d, QCryptographicHash::Sha256).toHex()); }

static QString run(const QString &program, const QStringList &args, int *rc, const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment(),
                   int timeoutMs = 600000)
{
    QProcess p;
    p.setProcessEnvironment(env);
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.setWorkingDirectory(kSource);
    p.start(program, args);
    if (!p.waitForFinished(timeoutMs)) {
        p.kill();
        p.waitForFinished();
        if (rc)
            *rc = -1;
        return QString::fromUtf8(p.readAll()) + QStringLiteral("\n(timed out)");
    }
    if (rc)
        *rc = p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1;
    return QString::fromUtf8(p.readAll());
}

// makes an add-on package as tools/make-package.py does: manifest + payload files
static bool makePackage(const QString &dir, const QJsonObject &manifest, const QList<QPair<QString, QString>> &files, const QString &out,
                        QString *log)
{
    QDir(dir).removeRecursively();
    writeFile(dir + QStringLiteral("/manifest.json"), QJsonDocument(manifest).toJson());
    for (const auto &f : files) {
        QDir().mkpath(QFileInfo(dir + QStringLiteral("/payload/") + f.first).absolutePath());
        QFile::copy(f.second, dir + QStringLiteral("/payload/") + f.first);
    }
    int rc = 0;
    *log = run(QStringLiteral("python3"), {QStringLiteral("-I"), kSource + QStringLiteral("/tools/make-package.py"), QStringLiteral("addon"), dir, out}, &rc);
    return rc == 0 && QFileInfo::exists(out);
}

struct Done {
    bool got = false, ok = false;
    QString what, message;
};
static Done waitFinished(AiDownloads &d, const std::function<void()> &start, int ms = 30000)
{
    Done r;
    const QMetaObject::Connection c = QObject::connect(&d, &AiDownloads::finished, [&r](const QString &w, bool ok, const QString &m) {
        r.got = true;
        r.ok = ok;
        r.what = w;
        r.message = m;
    });
    start();
    QElapsedTimer t;
    t.start();
    while (!r.got && t.elapsed() < ms)
        QTest::qWait(10);
    QObject::disconnect(c);
    return r;
}

static void shot(QWidget *w, const QString &name)
{
    w->grab().save(outDir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

// a 16-bit PCM WAV file's samples as floats (the data chunk found by its name)
static QVector<float> readWav(const QString &path, int *rate)
{
    QVector<float> out;
    const QByteArray d = readAll(path);
    if (d.size() < 44 || !d.startsWith("RIFF"))
        return out;
    int pos = 12, channels = 1;
    while (pos + 8 <= d.size()) {
        const QByteArray id = d.mid(pos, 4);
        const quint32 size = quint32(uchar(d[pos + 4])) | quint32(uchar(d[pos + 5])) << 8 | quint32(uchar(d[pos + 6])) << 16 | quint32(uchar(d[pos + 7])) << 24;
        if (id == "fmt ") {
            channels = uchar(d[pos + 10]) | uchar(d[pos + 11]) << 8;
            *rate = int(quint32(uchar(d[pos + 12])) | quint32(uchar(d[pos + 13])) << 8 | quint32(uchar(d[pos + 14])) << 16);
        } else if (id == "data") {
            const int n = int(qMin<qint64>(size, d.size() - pos - 8)) / 2;
            for (int i = 0; i < n; i += qMax(1, channels)) {
                const qint16 s = qint16(uchar(d[pos + 8 + 2 * i]) | uchar(d[pos + 9 + 2 * i]) << 8);
                out << float(s) / 32768.0f;
            }
            break;
        }
        pos += 8 + int(size) + int(size & 1);
    }
    return out;
}

static int zeroCrossings(const QVector<float> &s)
{
    int n = 0;
    for (int i = 1; i < s.size(); ++i)
        if ((s[i - 1] < 0) != (s[i] < 0))
            ++n;
    return n;
}

// ================================================================ the real engine, in a child process
static int childReal(const QString &model, const QString &wav)
{
    // the memory rule is checked with the fake engine; here the computer's real figures (other programs) must not decide
    sysmem::setFakeRam(sysmem::Figures{16 * 1024 * MB, 12 * 1024 * MB});
    ai::Engine &e = ai::Engine::instance();
    QString why;
    const bool loaded = e.ensureLoaded(&why);
    check(loaded && e.version().contains(QLatin1String("ggml")) && e.version().contains(QLatin1String("whisper.cpp")),
          QStringLiteral("real engine: libqbai loads with QLibrary and names its parts"), why + QLatin1String(" / ") + e.version());
    if (!loaded)
        return failures;
    info(e.version());
    const QList<ai::Device> devices = e.devices(true);
    check(!devices.isEmpty() && !devices.first().gpu && devices.first().index == 0,
          QStringLiteral("real engine: the devices begin with the processor (%1 device(s))").arg(devices.size()));
    for (const ai::Device &d : devices)
        info(QStringLiteral("device %1: %2 (%3), %4 of %5 MB free").arg(d.index).arg(d.name, d.id).arg(d.free / MB).arg(d.total / MB));
    ModelMemory::instance().setThreadSetting(2);
    e.applyThreads();
    check(e.api()->threads() == 2, QStringLiteral("real engine: the shared setting of the AI's cores reaches it"));

    // Sonic: twice as fast, the pitch kept (as many zero crossings per sample)
    QVector<float> tone(24000);
    for (int i = 0; i < tone.size(); ++i)
        tone[i] = 0.5f * float(std::sin(2.0 * M_PI * 220.0 * i / 24000.0));
    QVector<float> fast;
    const bool st = ai::stretch(tone, 24000, 2.0, &fast, &why);
    const double ratio = tone.isEmpty() ? 0 : double(fast.size()) / tone.size();
    const double pitch = fast.isEmpty() ? 0 : (double(zeroCrossings(fast)) / fast.size()) / (double(zeroCrossings(tone)) / tone.size());
    check(st && ratio > 0.45 && ratio < 0.55 && pitch > 0.9 && pitch < 1.1,
          QStringLiteral("real engine: Sonic makes 1 s of a tone 0.5 s long at speed 2, its pitch kept"),
          QStringLiteral("%1 samples (%2), pitch %3 %4").arg(fast.size()).arg(ratio).arg(pitch).arg(why));

    {
        // the sound output (miniaudio inside libqbai): a computer or container without a sound device gets miniaudio's silent
        // backend or a clear error; never a crash
        const qbai_api *a2 = e.apiFor(2);
        qbai_output *o = a2 && a2->output_open ? a2->output_open(24000, 1) : nullptr;
        if (!o) {
            check(a2 && a2->output_open && QString::fromUtf8(a2->last_error()).contains(QLatin1String("sound output")),
                  QStringLiteral("real engine: no sound output here, and the reason is told"), QString::fromUtf8(a2 ? a2->last_error() : ""));
        } else {
            const std::vector<float> beep(12000, 0.0f);   // silence: nothing audible in a check
            check(a2->output_write(o, beep.data(), 12000) == QBAI_OK, QStringLiteral("real engine: a sound output opens and takes PCM"));
            QElapsedTimer t;
            t.start();
            while (a2->output_played(o) < 3000 && t.elapsed() < 3000)
                QTest::qWait(20);
            check(a2->output_played(o) >= 3000, QStringLiteral("real engine: its played position moves with the clock"), QString::number(a2->output_played(o)));
            a2->output_stop(o);
            check(a2->output_played(o) == 0, QStringLiteral("real engine: stop puts the position back to 0"));
            a2->output_close(o);
        }
    }

    if (model.isEmpty() || !QFileInfo::exists(model) || !QFileInfo::exists(wav)) {
        info(QStringLiteral("(speech recognition not checked: QB_AI_WHISPER_MODEL and QB_AI_WHISPER_SAMPLE name no files)"));
    } else {
        int rate = 0;
        const QVector<float> pcm = readWav(wav, &rate);
        ai::Model m(QStringLiteral("smoke:whisper"));
        ai::ModelFiles f;
        f.kind = QBAI_ASR_WHISPER;
        f.model = model;
        f.name = QStringLiteral("whisper tiny.en");
        const bool ok = m.load(f, QStringLiteral("auto"), &why);
        check(ok && m.isLoaded() && ModelMemory::instance().isLoaded(QStringLiteral("smoke:whisper")) && m.bytes() > 75 * MB,
              QStringLiteral("real engine: whisper.cpp's tiny model loads (Automatic, %1), registered with %2 MB")
                  .arg(m.device().name).arg(m.bytes() / MB), why);
        QStringList segments;
        QString text;
        const bool tr = ok && rate == 16000
                        && m.transcribe(pcm, QStringLiteral("en"), false, [&segments](const QString &s, qint64, qint64) { segments << s; }, nullptr,
                                        &text, &why);
        check(tr && text.toLower().contains(QLatin1String("ask not what your country can do for you")) && !segments.isEmpty(),
              QStringLiteral("real engine: whisper.cpp writes the test recording as text, in segments"), text + QLatin1String(" / ") + why);
        info(text);
        m.unload();
        check(!m.isLoaded() && !ModelMemory::instance().isLoaded(QStringLiteral("smoke:whisper")),
              QStringLiteral("real engine: unloaded, and off the list"));
    }
    const QString talker = qEnvironmentVariable("QB_AI_QWEN_TALKER"), codec = qEnvironmentVariable("QB_AI_QWEN_CODEC");
    if (talker.isEmpty() || codec.isEmpty() || !QFileInfo::exists(talker) || !QFileInfo::exists(codec)) {
        info(QStringLiteral("(speech synthesis not checked: QB_AI_QWEN_TALKER and QB_AI_QWEN_CODEC name no files)"));
    } else {
        ai::Model m(QStringLiteral("smoke:qwen"));
        ai::ModelFiles f;
        f.kind = QBAI_TTS_QWEN;
        f.model = talker;
        f.extra = codec;
        const bool ok = m.load(f, QStringLiteral("cpu"), &why);
        QVector<float> pcm;
        int pieces = 0;
        const bool spoke = ok && m.synthesize(QStringLiteral("Hello."), QStringLiteral("en"), ai::Voice(),
                                              [&pieces](const float *, qint64) { ++pieces; return true; }, nullptr, &pcm, &why);
        check(spoke && pieces > 0 && m.sampleRate() == 24000, QStringLiteral("real engine: Qwen3-TTS speaks a word, in pieces (%1)").arg(pieces), why);
        m.unload();
    }
    return failures;
}

int main(int argc, char **argv)
{
    if (argc > 1 && QByteArray(argv[1]) == "--child-real") {
        QCoreApplication app(argc, argv);
        QTemporaryDir home;
        qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
        return childReal(argc > 2 ? QString::fromLocal8Bit(argv[2]) : QString(), argc > 3 ? QString::fromLocal8Bit(argv[3]) : QString());
    }
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    const QString work = home.path() + QStringLiteral("/work");
    QDir().mkpath(work);
    // the catalog of these checks (read once, before anything asks for it)
    const QString catalogFile = work + QStringLiteral("/catalog.json");
    qputenv("QB_AI_CATALOG", catalogFile.toUtf8());
    const QString source = work + QStringLiteral("/release");   // the "release" the models come from
    qputenv("QB_AI_MODEL_SOURCE", source.toUtf8());
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990hTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990hTest"));
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
    as.save();
    theme::apply();
    const QString fakeLib = kBuild + QStringLiteral("/tests/fake-ai-engine/libqbai.so.1");

    // the models of the catalog and their files on the "release"
    const QByteArray voice = QByteArray(1 * MB, 'v'), mm1 = QByteArray(300 * 1024, 'm'), mm2 = QByteArray(200 * 1024, 'n'),
                     asr = QByteArray(512 * 1024, 'a'), good = QByteArray(64 * 1024, 'g'), wrong = QByteArray(64 * 1024, 'w');
    writeFile(source + QStringLiteral("/fake-tts--voice.gguf"), voice);
    writeFile(source + QStringLiteral("/fake-tts--mmproj.gguf.part1"), mm1);
    writeFile(source + QStringLiteral("/fake-tts--mmproj.gguf.part2"), mm2);
    writeFile(source + QStringLiteral("/fake-asr--asr.bin"), asr);
    writeFile(source + QStringLiteral("/fake-bad--bad.bin"), wrong);           // not what the catalog says
    writeFile(source + QStringLiteral("/fake-two--a.bin"), good);
    writeFile(source + QStringLiteral("/fake-two--b.bin"), wrong);             // first wrong, later right
    // voice packs of Read Aloud (0.99.0): a zip that is a pack, one that is not, and a Qwen model whose download brings the pack of the language
    QTemporaryDir zipDir;
    auto makeZip = [&](const QString &name, const QStringList &entries) {
        const QString path = zipDir.filePath(name);
        {
            ZipWriter z(path);
            for (const QString &e : entries)
                z.addFile(e, e.endsWith(QLatin1String(".txt")) ? QByteArray("Words of the clip.") : QByteArray(48044, 'w'));
            z.close();
        }
        return readAll(path);
    };
    const QByteArray packEn = makeZip(QStringLiteral("en.zip"), {QStringLiteral("female1-neutral.wav"), QStringLiteral("female1-neutral.txt"), QStringLiteral("voices.json")});
    const QByteArray packDe = makeZip(QStringLiteral("de.zip"), {QStringLiteral("male1-calm.wav"), QStringLiteral("male1-calm.txt")});
    const QByteArray packBad = makeZip(QStringLiteral("bad.zip"), {QStringLiteral("female1-neutral.wav"), QStringLiteral("../outside.wav")});
    writeFile(source + QStringLiteral("/read-aloud-voices-en.zip"), packEn);
    writeFile(source + QStringLiteral("/read-aloud-voices-de.zip"), packDe);
    writeFile(source + QStringLiteral("/read-aloud-voices-fr.zip"), packBad);
    writeFile(source + QStringLiteral("/fake-qwen--q.gguf"), good);
    auto file = [](const QString &name, const QString &asset, const QByteArray &d) {
        return QJsonObject{{"name", name}, {"asset", asset}, {"size", double(d.size())}, {"sha256", sha(d)}};
    };
    QJsonObject mmproj{{"name", "mmproj.gguf"}, {"size", double(mm1.size() + mm2.size())}, {"sha256", sha(mm1 + mm2)},
                       {"parts", QJsonArray{QJsonObject{{"asset", "fake-tts--mmproj.gguf.part1"}, {"size", double(mm1.size())}, {"sha256", sha(mm1)}},
                                            QJsonObject{{"asset", "fake-tts--mmproj.gguf.part2"}, {"size", double(mm2.size())}, {"sha256", sha(mm2)}}}}};
    const QJsonArray models{
        QJsonObject{{"id", "fake-tts"}, {"title", "Fake Voice"}, {"summary", "A voice of the checks."}, {"kinds", QJsonArray{"tts"}},
                    {"engine", "pocket-tts"}, {"languages", QJsonArray{"en", "de"}}, {"voices", QJsonArray{"alba", "marius"}},
                    {"license", "CC-BY-4.0"}, {"licenseUrl", "https://creativecommons.org/licenses/by/4.0/"}, {"credit", "Made for the checks."},
                    {"memoryMB", 50}, {"sizeMB", 2}, {"model", "voice.gguf"}, {"extra", "mmproj.gguf"},
                    {"files", QJsonArray{file("voice.gguf", "fake-tts--voice.gguf", voice), mmproj}}},
        QJsonObject{{"id", "fake-asr"}, {"title", "Fake Ears"}, {"kinds", QJsonArray{"asr"}}, {"engine", "whisper"}, {"languages", QJsonArray{"*"}},
                    {"license", "MIT"}, {"files", QJsonArray{file("asr.bin", "fake-asr--asr.bin", asr)}}},
        QJsonObject{{"id", "fake-unpublished"}, {"title", "Not Yet"}, {"kinds", QJsonArray{"tts"}}, {"engine", "qwen3-tts"},
                    {"files", QJsonArray{QJsonObject{{"name", "x.gguf"}, {"asset", "x"}, {"size", 10}, {"sha256", "PLACEHOLDER"}}}}},
        QJsonObject{{"id", "fake-huge"}, {"title", "Too Big"}, {"kinds", QJsonArray{"tts"}}, {"engine", "qwen3-tts"}, {"memoryMB", 100000000},
                    {"files", QJsonArray{file("big.gguf", "fake-huge--big.gguf", good)}}},
        QJsonObject{{"id", "fake-bad"}, {"title", "Damaged"}, {"kinds", QJsonArray{"asr"}}, {"engine", "whisper"},
                    {"files", QJsonArray{file("bad.bin", "fake-bad--bad.bin", good)}}},
        QJsonObject{{"id", "fake-two"}, {"title", "Two Files"}, {"kinds", QJsonArray{"asr"}}, {"engine", "whisper"},
                    {"files", QJsonArray{file("a.bin", "fake-two--a.bin", good), file("b.bin", "fake-two--b.bin", good)}}},
        QJsonObject{{"id", "../evil"}, {"title", "Out of its folder"}, {"kinds", QJsonArray{"tts"}}, {"files", QJsonArray{file("e", "e", good)}}},
        QJsonObject{{"id", "fake-evil-file"}, {"title", "A file out of its folder"}, {"kinds", QJsonArray{"tts"}},
                    {"files", QJsonArray{file("../e", "e", good)}}},
        QJsonObject{{"id", "fake-qwen"}, {"title", "Fake Qwen"}, {"kinds", QJsonArray{"tts"}}, {"engine", "qwen3-tts"}, {"languages", QJsonArray{"en", "de"}},
                    {"memoryMB", 50}, {"model", "q.gguf"}, {"files", QJsonArray{file("q.gguf", "fake-qwen--q.gguf", good)}}},
        QJsonObject{{"id", "voices-en"}, {"title", "Voices, English"}, {"kinds", QJsonArray{"voicepack"}}, {"languages", QJsonArray{"en"}},
                    {"files", QJsonArray{file("read-aloud-voices-en.zip", "read-aloud-voices-en.zip", packEn)}}},
        QJsonObject{{"id", "voices-de"}, {"title", "Voices, German"}, {"kinds", QJsonArray{"voicepack"}}, {"languages", QJsonArray{"de"}},
                    {"files", QJsonArray{file("read-aloud-voices-de.zip", "read-aloud-voices-de.zip", packDe)}}},
        QJsonObject{{"id", "voices-fr"}, {"title", "Voices, French"}, {"kinds", QJsonArray{"voicepack"}}, {"languages", QJsonArray{"fr"}},
                    {"files", QJsonArray{file("read-aloud-voices-fr.zip", "read-aloud-voices-fr.zip", packBad)}}},
        QJsonObject{{"id", "voices-ru"}, {"title", "Voices, Russian"}, {"kinds", QJsonArray{"voicepack"}}, {"languages", QJsonArray{"ru"}},
                    {"files", QJsonArray{QJsonObject{{"name", "read-aloud-voices-ru.zip"}, {"asset", "read-aloud-voices-ru.zip"}, {"size", 0}, {"sha256", ""}}}}},
    };
    writeFile(catalogFile, QJsonDocument(QJsonObject{{"format", 1}, {"release", "models-5"}, {"models", models}}).toJson());

    // ================================================================ silence: nothing on the network unless the writer asks
    std::printf("== silence\n");
    {
        auto *panel = new AiInstallPanel(ai::aiEngineId(), {QStringLiteral("tts")});
        panel->resize(720, 400);
        panel->show();
        QTest::qWait(50);
        check(!AiDownloads::instance().hasNetwork() && AiDownloads::instance().requests() == 0,
              QStringLiteral("the panel of a settings page makes no network object and no request until a button is pressed"));
        check(!ai::Engine::instance().isLoaded(), QStringLiteral("... and loads no engine (the AI is loaded only when used)"));
        delete panel;
    }

    // ================================================================ the interface header and the fake library
    std::printf("== the C interface\n");
    {
        check(QBAI_API_VERSION == 2 && QBAI_API_V1_SIZE < sizeof(qbai_api) && QBAI_TRANSCRIBE_V1_SIZE < sizeof(qbai_transcribe_params) && offsetof(qbai_api, version) == 0 && offsetof(qbai_api, size) == sizeof(int)
                  && offsetof(qbai_load_params, size) == 0 && offsetof(qbai_synth_params, size) == 0 && offsetof(qbai_voice, size) == 0
                  && offsetof(qbai_transcribe_params, size) == 0 && offsetof(qbai_model_info, size) == 0,
              QStringLiteral("tools/QbAi.h: version 2 (version 1 is its prefix); the table and every parameter struct begin with their version or size"));
        QLibrary lib(fakeLib);
        const bool loaded = lib.load();
        auto get = loaded ? reinterpret_cast<qbai_get_api_fn>(lib.resolve("qbai_get_api")) : nullptr;
        check(get && get(1) && get(1)->version == 1 && get(1)->size == int(QBAI_API_V1_SIZE) && get(2) && get(2)->version == 2
                  && get(2)->size == int(sizeof(qbai_api)) && get(2)->detect_language && !get(0) && !get(3),
              QStringLiteral("the fake libqbai: qbai_get_api(1) gives the version 1 table, (2) the whole one with detect_language, 0 and 3 give none"), lib.errorString());
    }

    // ================================================================ no engine installed
    std::printf("== no engine\n");
    {
        ai::Engine &e = ai::Engine::instance();
        QString why;
        check(!e.isInstalled() && e.libraryPath().isEmpty() && !e.ensureLoaded(&why) && why.contains(QLatin1String("not installed")),
              QStringLiteral("without the package: no library, and the engine says it is not installed"), why);
        check(toolhost::host().aiEngine(1) == nullptr && toolhost::host().aiPickDevice(QStringLiteral("auto"), MB, nullptr) == -1
                  && toolhost::host().aiDeviceChoices().size() == 2,
              QStringLiteral("tool interface 1.3: aiEngine() is null, aiPickDevice() -1, the choices Automatic and the processor"));
        check(!ai::package(ai::aiEngineId()).installed && ai::onnxRuntimeLibrary().isEmpty() && e.devices().isEmpty(),
              QStringLiteral("no package, no ONNX Runtime, no devices"));
    }

    // ================================================================ the device choice
    std::printf("== the device choice\n");
    {
        ai::Device cpu;
        cpu.name = QStringLiteral("Processor (CPU)");
        auto card = [](int index, const QString &name, qint64 freeMB, qint64 totalMB, bool integrated = false) {
            ai::Device d;
            d.index = index;
            d.gpu = true;
            d.integrated = integrated;
            d.name = name;
            d.id = QStringLiteral("Vulkan%1").arg(index - 1);
            d.free = freeMB * MB;
            d.total = totalMB * MB;
            return d;
        };
        QString why;
        check(!ai::pickDevice({cpu}, QStringLiteral("auto"), 100 * MB, &why).gpu && why.isEmpty(),
              QStringLiteral("Automatic without a card: the processor, nothing to say"));
        const ai::Device c3 = card(1, QStringLiteral("Card A"), 3072, 4096);
        check(ai::pickDevice({cpu, c3}, QStringLiteral("auto"), 1024 * MB, &why).index == 1,
              QStringLiteral("Automatic: the card, when the model fits into its free memory (1 GB into 3 GB)"));
        const ai::Device big = ai::pickDevice({cpu, c3}, QStringLiteral("auto"), 3500 * MB, &why);
        check(!big.gpu && why.contains(QLatin1String("does not fit")),
              QStringLiteral("Automatic: the processor, when it does not fit (3.5 GB into 3 GB), and it says why"), why);
        check(!ai::pickDevice({cpu, c3}, QStringLiteral("auto"), 0).gpu, QStringLiteral("Automatic, the size not known: the processor"));
        const ai::Device igpu = card(1, QStringLiteral("Shared"), 6000, 8000, true), d2 = card(2, QStringLiteral("Own"), 2000, 4000);
        check(ai::pickDevice({cpu, igpu, d2}, QStringLiteral("auto"), 1500 * MB).name == QLatin1String("Own")
                  && ai::pickDevice({cpu, igpu, d2}, QStringLiteral("auto"), 3000 * MB).name == QLatin1String("Shared"),
              QStringLiteral("Automatic: a card with memory of its own before a graphics part that shares the RAM, else the one that fits"));
        const ai::Device s = card(1, QStringLiteral("Small"), 2000, 4000), l = card(2, QStringLiteral("Large"), 5000, 8000);
        check(ai::pickDevice({cpu, s, l}, QStringLiteral("auto"), 1000 * MB).name == QLatin1String("Large"),
              QStringLiteral("Automatic, two cards that fit: the one with more free memory"));
        check(ai::pickDevice({cpu, c3}, QStringLiteral("gpu:Card A"), 9000 * MB).index == 1,
              QStringLiteral("a fixed card is taken as chosen (the memory rule decides later)"));
        const ai::Device gone = ai::pickDevice({cpu, c3}, QStringLiteral("gpu:Card B"), MB, &why);
        check(!gone.gpu && why.contains(QLatin1String("Card B")), QStringLiteral("a fixed card that is not there: the processor, and it says so"), why);
        check(!ai::pickDevice({cpu, c3}, QStringLiteral("cpu"), MB).gpu, QStringLiteral("the processor when chosen, though the card would fit"));
        const auto choices = ai::deviceChoices({cpu, c3, l});
        check(choices.size() == 4 && choices[0].first == QLatin1String("auto") && choices[1].first == QLatin1String("cpu")
                  && choices[2].first == QLatin1String("gpu:Card A") && choices[3].second == QLatin1String("Large"),
              QStringLiteral("the entries of a device setting: Automatic, the processor, then each card"));
    }

    // ================================================================ the fake engine
    std::printf("== the engine loaded (the fake one)\n");
    qputenv("QB_AI_ENGINE_LIB", fakeLib.toUtf8());
    ModelMemory &mm = ModelMemory::instance();
    mm.reset();
    sysmem::setFakeRam(sysmem::Figures{16 * 1024 * MB, 12 * 1024 * MB});
    using Stats = void (*)(int *, int *, int *, int *);
    Stats stats = nullptr;
    {
        ai::Engine &e = ai::Engine::instance();
        QString why;
        check(e.ensureLoaded(&why) && e.version() == QLatin1String("fake qbai 1 (tests)") && e.loadedFrom() == QFileInfo(fakeLib).absoluteFilePath(),
              QStringLiteral("QB_AI_ENGINE_LIB: the fake libqbai is loaded with QLibrary, its version checked"), why);
        stats = reinterpret_cast<Stats>(QLibrary(fakeLib).resolve("qbai_fake_stats"));
        const QList<ai::Device> devices = e.devices(true);
        check(devices.size() == 2 && !devices[0].gpu && devices[1].gpu && devices[1].name == QLatin1String("Fake Card")
                  && devices[1].total == 4096 * MB && devices[1].free == 3072 * MB,
              QStringLiteral("the devices: the processor, then the fake card with its memory"));
        const modelmem::DeviceFigures fig = mm.device(QStringLiteral("gpu:Fake Card"));
        check(fig.total == 4096 * MB && fig.available == 3072 * MB, QStringLiteral("... and the card's figures went to ModelMemory"));
        mm.setThreadSetting(3);
        e.applyThreads();
        check(e.api()->threads() == 3, QStringLiteral("the shared setting of the AI's cores reaches the engine"));
        check(toolhost::host().aiEngine(1) == e.api() && toolhost::host().aiEngine(2) && toolhost::host().aiEngine(2)->version == 2 && toolhost::host().aiEngine(3) == nullptr,
              QStringLiteral("tool interface 1.3: aiEngine(1) is the program's own table (one engine in the process), aiEngine(2) the version 2 table, aiEngine(3) none"));
        {
            // the sound output of the engine (Read Aloud): open, write, played, pause, resume, stop, close
            const qbai_api *a2 = e.apiFor(2);
            check(a2 && a2->size == int(sizeof(qbai_api)) && a2->output_open && a2->output_write && a2->output_pause && a2->output_resume && a2->output_stop
                      && a2->output_close && a2->output_played && !(e.api()->size > int(QBAI_API_V1_SIZE)),
                  QStringLiteral("the version 2 table ends with the sound output functions; the version 1 table (what the program holds) has none"));
            typedef void (*OutScale)(double);
            auto setScale = reinterpret_cast<OutScale>(QLibrary(fakeLib).resolve("qbai_fake_out_set_scale"));
            qbai_output *o = a2 ? a2->output_open(24000, 1) : nullptr;
            const std::vector<float> pcm(24000, 0.1f);
            if (setScale)
                setScale(1000);
            check(o && a2->output_write(o, pcm.data(), 24000) == QBAI_OK, QStringLiteral("an output opens and takes PCM (24000 frames at 24 kHz)"));
            QTest::qWait(60);
            const qint64 p1 = o ? a2->output_played(o) : -1;
            check(p1 > 0 && p1 <= 24000, QStringLiteral("the played position moves while it plays"), QString::number(p1));
            if (o) {
                a2->output_pause(o);
                const qint64 at = a2->output_played(o);
                QTest::qWait(60);
                check(a2->output_played(o) == at, QStringLiteral("paused: the position stands still"));
                a2->output_resume(o);
                QTest::qWait(60);
                check(a2->output_played(o) >= at, QStringLiteral("resumed: it goes on"));
                a2->output_stop(o);
                check(a2->output_played(o) == 0, QStringLiteral("stop drops the queue; the position goes back to 0"));
                a2->output_close(o);
                a2->output_close(nullptr);
            }
            qputenv("QBAI_FAKE_NO_SOUND", "1");
            check(a2 && a2->output_open(24000, 1) == nullptr && QString::fromUtf8(a2->last_error()).contains(QLatin1String("sound output")),
                  QStringLiteral("no sound output: open fails with a reason"));
            qunsetenv("QBAI_FAKE_NO_SOUND");
            check(a2 && a2->output_write(nullptr, pcm.data(), 10) != QBAI_OK && a2->output_open(0, 1) == nullptr, QStringLiteral("bad arguments are refused"));
        }
        QString where;
        check(toolhost::host().aiPickDevice(QStringLiteral("auto"), 1024 * MB, &where) == 1 && where == QLatin1String("gpu:Fake Card")
                  && toolhost::host().aiPickDevice(QStringLiteral("auto"), 3500 * MB, &where) == 0 && where == QLatin1String("ram"),
              QStringLiteral("aiPickDevice: the card for 1 GB, the processor (\"ram\") for 3.5 GB"));
        check(toolhost::host().aiDeviceChoices().size() == 3, QStringLiteral("aiDeviceChoices: Automatic, the processor and the card"));
    }

    // ================================================================ a model of the program
    std::printf("== a model and the memory rule\n");
    {
        const QString d = work + QStringLiteral("/models");
        writeFile(d + QStringLiteral("/voice.gguf"), QByteArray(MB, 'x'));
        writeFile(d + QStringLiteral("/mmproj.gguf"), QByteArray(MB, 'y'));
        writeFile(d + QStringLiteral("/fail.gguf"), QByteArray("FAIL") + QByteArray(1000, 'z'));
        writeFile(d + QStringLiteral("/nogpu.gguf"), QByteArray("NOGPU") + QByteArray(1000, 'z'));
        writeFile(d + QStringLiteral("/asr.bin"), QByteArray(MB, 'w'));
        ai::ModelFiles f;
        f.kind = QBAI_TTS_POCKET;
        f.model = d + QStringLiteral("/voice.gguf");
        f.extra = d + QStringLiteral("/mmproj.gguf");
        f.name = QStringLiteral("Test Voice");
        ai::Model m(QStringLiteral("read-aloud:test"));
        QString why;
        int loaded = 0, loads = 0, lastDevice = -1, lastThreads = 0;
        const bool ok = m.load(f, QStringLiteral("auto"), &why);
        stats(&loaded, &loads, &lastDevice, &lastThreads);
        const QList<modelmem::LoadedModel> list = mm.models();
        check(ok && m.isLoaded() && m.device().gpu && lastDevice == 1 && loaded == 1 && lastThreads == 3,
              QStringLiteral("Automatic: a small model loads on the card (with the AI's cores)"), why);
        check(list.size() == 1 && list[0].id == QLatin1String("read-aloud:test") && list[0].bytes == 2 * MB
                  && list[0].where == QLatin1String("gpu:Fake Card") && list[0].name == QLatin1String("Test Voice") && !mm.changeActive(),
              QStringLiteral("... registered with ModelMemory: its id, name, size (the engine's estimate) and memory; no change under way"));
        check(m.voices() == QStringList({QStringLiteral("alba"), QStringLiteral("marius")}) && m.sampleRate() == 24000,
              QStringLiteral("its voices and its sample rate"));
        QVector<float> pcm;
        const bool spoke = m.synthesize(QStringLiteral("Hello there."), QStringLiteral("en"), ai::Voice(), nullptr, nullptr, &pcm, &why);
        float peak = 0;
        for (float s : pcm)
            peak = qMax(peak, std::fabs(s));
        check(spoke && pcm.size() == 12 * 1200 && peak > 0.29f && peak < 0.31f,
              QStringLiteral("a sentence: all its PCM at once (the fake's sine, 50 ms per character)"), QString::number(pcm.size()) + QLatin1Char(' ') + why);
        int pieces = 0;
        qint64 samples = 0;
        const bool streamed = m.synthesize(QStringLiteral("Hello there."), QString(), ai::Voice(),
                                           [&](const float *, qint64 n) { ++pieces; samples += n; return true; }, nullptr, &pcm, &why);
        check(streamed && pieces == 12 && samples == 12 * 1200, QStringLiteral("streaming: the same sentence in 12 pieces as they are made"));
        ai::Voice wrongVoice;
        wrongVoice.preset = QStringLiteral("nobody");
        check(!m.synthesize(QStringLiteral("Hi."), QString(), wrongVoice, nullptr, nullptr, &pcm, &why) && why.contains(QLatin1String("no such voice")),
              QStringLiteral("a voice the model does not have: refused with the engine's error"), why);

        // cancel, and ModelMemory's request to unload while it speaks
        qputenv("QBAI_FAKE_SLOW_MS", "20");
        std::atomic_bool cancel{false};
        bool result = true;
        QString cancelWhy;
        QElapsedTimer t;
        t.start();
        QThread *worker = QThread::create([&] {
            ModelMemory::lowerThreadPriority();
            result = m.synthesize(QString(200, QLatin1Char('x')), QString(), ai::Voice(), [](const float *, qint64) { return true; }, &cancel, nullptr,
                                  &cancelWhy);
        });
        worker->start();
        QTest::qWait(150);
        mm.setIdleMs(0);
        const QStringList idleWhileBusy = mm.unloadIdle();
        check(idleWhileBusy.isEmpty() && m.isLoaded() && mm.isLoaded(QStringLiteral("read-aloud:test")),
              QStringLiteral("while it speaks, ModelMemory's request to unload is refused (busy), the model stays"));
        cancel = true;
        while (!worker->isFinished() && t.elapsed() < 10000)
            QTest::qWait(10);
        delete worker;
        check(!result && cancelWhy == QLatin1String("Stopped.") && t.elapsed() < 3000,
              QStringLiteral("cancel: the sentence (10 s of sound) stops at once"), QStringLiteral("%1 ms: %2").arg(t.elapsed()).arg(cancelWhy));
        qunsetenv("QBAI_FAKE_SLOW_MS");
        const QStringList idle = mm.unloadIdle();
        stats(&loaded, nullptr, nullptr, nullptr);
        check(idle == QStringList({QStringLiteral("read-aloud:test")}) && !m.isLoaded() && loaded == 0 && !mm.isLoaded(QStringLiteral("read-aloud:test")),
              QStringLiteral("idle: ModelMemory unloads it, the engine frees it"));
        mm.setIdleMs(-1);

        // the memory rule refuses: the computer's memory nearly full
        sysmem::setFakeRam(sysmem::Figures{8 * 1024 * MB, 1024 * MB});
        ai::ModelFiles large = f;
        large.memoryBytes = 2048 * MB;
        stats(nullptr, &loads, nullptr, nullptr);
        const int before = loads;
        const bool refused = !m.load(large, QStringLiteral("cpu"), &why);
        stats(nullptr, &loads, nullptr, nullptr);
        check(refused && !why.isEmpty() && loads == before && !mm.changeActive() && !mm.isLoaded(QStringLiteral("read-aloud:test")),
              QStringLiteral("the memory rule refuses a model that would bring the memory over 80 %: not loaded, the reason given"), why);
        sysmem::setFakeRam(sysmem::Figures{16 * 1024 * MB, 12 * 1024 * MB});

        // Automatic: the card would go over 80 % (1 GB used + 2.5 GB of 4 GB): the processor instead
        large.memoryBytes = 2560 * MB;
        const bool fell = m.load(large, QStringLiteral("auto"), &why);
        stats(nullptr, nullptr, &lastDevice, nullptr);
        check(fell && !m.device().gpu && lastDevice == 0 && mm.models().value(0).where == QLatin1String("ram"),
              QStringLiteral("Automatic: the card is refused by the memory rule, so the model loads into the computer's memory"), why);
        m.unload();
        check(!mm.isLoaded(QStringLiteral("read-aloud:test")), QStringLiteral("unloaded by its owner: off the list"));

        // Automatic: the card does not take it after all (its driver): once more on the processor
        ai::ModelFiles nogpu = f;
        nogpu.model = d + QStringLiteral("/nogpu.gguf");
        const bool again = m.load(nogpu, QStringLiteral("auto"), &why);
        stats(nullptr, nullptr, &lastDevice, nullptr);
        check(again && !m.device().gpu && lastDevice == 0 && mm.models().value(0).where == QLatin1String("ram") && !mm.changeActive(),
              QStringLiteral("Automatic: a card that fails to load it: the processor, registered there"), why);
        m.unload();
        check(!m.load(nogpu, QStringLiteral("gpu:Fake Card"), &why) && why.contains(QLatin1String("no room")) && !mm.changeActive()
                  && mm.models().isEmpty(),
              QStringLiteral("a fixed card that fails: an error (no other device), nothing registered"), why);

        ai::ModelFiles broken = f;
        broken.model = d + QStringLiteral("/fail.gguf");
        check(!m.load(broken, QStringLiteral("auto"), &why) && why.contains(QLatin1String("Test Voice")) && why.contains(QLatin1String("cannot load"))
                  && mm.models().isEmpty() && !mm.changeActive(),
              QStringLiteral("a model that does not load: the engine's error with the model's name; nothing registered, no change under way"), why);
        check(m.load(f, QStringLiteral("gpu:Fake Card"), &why) && m.device().gpu, QStringLiteral("the card when chosen"));
        m.unload();
        check(m.load(f, QStringLiteral("gpu:Missing"), &why) && !m.device().gpu, QStringLiteral("a chosen card that is not there: the processor"));
        m.unload();

        ai::Model ears(QStringLiteral("test:ears"));
        ai::ModelFiles af;
        af.kind = QBAI_ASR_WHISPER;
        af.model = d + QStringLiteral("/asr.bin");
        QStringList segments;
        QString text;
        const bool heard = ears.load(af, QStringLiteral("cpu"), &why)
                           && ears.transcribe(QVector<float>(16000, 0.0f), QStringLiteral("de"), false,
                                              [&](const QString &s, qint64, qint64) { segments << s; }, nullptr, &text, &why);
        check(heard && text == QLatin1String("[de] The quick brown fox jumps over the lazy dog.") && segments.size() == 2,
              QStringLiteral("speech recognition: the text, and its segments as they come"), text + QLatin1Char(' ') + why);
        std::atomic_bool stop{true};
        check(!ears.transcribe(QVector<float>(16000, 0.0f), QString(), false, nullptr, &stop, &text, &why) && why == QLatin1String("Stopped."),
              QStringLiteral("speech recognition can be cancelled"));
        ears.unload();

        QVector<float> fast;
        check(ai::stretch(QVector<float>(24000, 0.1f), 24000, 2.0, &fast, &why) && fast.size() == 12000
                  && !ai::stretch(QVector<float>(100, 0.1f), 24000, 9.0, &fast, &why),
              QStringLiteral("the speed of speech: 2.0 halves the length; 9.0 is refused"), why);
        stats(&loaded, nullptr, nullptr, nullptr);
        check(loaded == 0 && mm.models().isEmpty(), QStringLiteral("every model is freed in the engine and off the list"));
    }

    // ================================================================ the catalog
    std::printf("== the catalog\n");
    {
        QString err;
        const ai::Catalog own = ai::Catalog::fromJson(readAll(QStringLiteral(":/ai/models.json")), &err);
        check(own.format == 1 && own.release == QLatin1String("models-5"),
              QStringLiteral("the program's own catalog (resources/ai/models.json): format 1, the release models-5"), err);
        const ai::Catalog &c = ai::Catalog::builtIn();
        QStringList ids;
        for (const ai::CatalogModel &m : c.models)
            ids << m.id;
        check(c.format == 1 && ids == QStringList({QStringLiteral("fake-tts"), QStringLiteral("fake-asr"), QStringLiteral("fake-unpublished"),
                                                   QStringLiteral("fake-huge"), QStringLiteral("fake-bad"), QStringLiteral("fake-two"), QStringLiteral("fake-qwen"),
                                                   QStringLiteral("voices-en"), QStringLiteral("voices-de"), QStringLiteral("voices-fr"), QStringLiteral("voices-ru")}),
              QStringLiteral("QB_AI_CATALOG: the checks' catalog; a model whose id or file name leaves its folder is left out"), ids.join(QLatin1String(", ")));
        const ai::CatalogModel *t = c.model(QStringLiteral("fake-tts"));
        check(t && t->engineKind() == QBAI_TTS_POCKET && t->languages == QStringList({QStringLiteral("en"), QStringLiteral("de")})
                  && t->memoryMB == 50 && t->files.size() == 2 && t->files[1].parts.size() == 2 && t->downloadBytes() == voice.size() + mm1.size() + mm2.size()
                  && t->release == QLatin1String("models-5") && t->published(),
              QStringLiteral("a model: its engine, languages, memory, files (one in two parts), release"));
        check(c.ofKinds({QStringLiteral("tts")}).size() == 4 && c.ofKinds({QStringLiteral("asr")}).size() == 3 && c.ofKinds({QStringLiteral("voicepack")}).size() == 4 && c.ofKinds({}).size() == 11,
              QStringLiteral("the models of a kind (\"tts\", \"asr\")"));
        check(!c.model(QStringLiteral("fake-unpublished"))->published()
                  && ai::downloadProblem(*c.model(QStringLiteral("fake-unpublished"))).contains(QLatin1String("not published")),
              QStringLiteral("a file without its SHA-256: not published yet"));
        check(ai::downloadProblem(*c.model(QStringLiteral("fake-huge"))).contains(QLatin1String("memory")),
              QStringLiteral("a model that needs more than 80 % of the computer's memory is not downloaded"), ai::downloadProblem(*c.model(QStringLiteral("fake-huge"))));
        check(ai::Catalog::fromJson("{\"format\": 2, \"models\": []}", &err).format == 0 && err.contains(QLatin1String("format")),
              QStringLiteral("a catalog of another format is not read"));
        qunsetenv("QB_AI_MODEL_SOURCE");
        const QString address = ai::fileAddress(*t, t->files[0]);
        qputenv("QB_AI_MODEL_SOURCE", source.toUtf8());
        check(address == QStringLiteral(QB_APP_HOMEPAGE "/releases/download/models-5/fake-tts--voice.gguf"),
              QStringLiteral("a file's address: built from the release's tag and the asset's name"), address);
        check(ai::releaseTag() == QStringLiteral("v" QB_APP_VERSION) + (QStringLiteral(QB_APP_VERSION_LABEL).isEmpty() ? QString() : QStringLiteral("-" QB_APP_VERSION_LABEL))
                  && ai::engineAddress(ai::aiEngineId()) == QStringLiteral(QB_APP_HOMEPAGE "/releases/download/") + ai::releaseTag() + QStringLiteral("/ai-engine.qbpkg")
                  && ai::engineAsset(ai::handwritingEngineId()) == QLatin1String("handwriting-engine.qbpkg"),
              QStringLiteral("the engine's address: this version's release, ai-engine.qbpkg"), ai::engineAddress(ai::aiEngineId()));
    }

    // ================================================================ downloading models
    std::printf("== downloading models\n");
    {
        const ai::Catalog &c = ai::Catalog::builtIn();
        const ai::CatalogModel &t = *c.model(QStringLiteral("fake-tts"));
        AiDownloads d;
        QList<qint64> seen;
        qint64 total = 0;
        QObject::connect(&d, &AiDownloads::progress, [&](const QString &, qint64 done, qint64 all) { seen << done; total = all; });
        const Done r = waitFinished(d, [&] { d.download(QStringLiteral("fake-tts")); });
        bool rising = !seen.isEmpty();
        for (int i = 1; i < seen.size(); ++i)
            rising = rising && seen[i] >= seen[i - 1];
        check(r.ok && ai::isInstalled(t) && d.requests() == 3, QStringLiteral("a model downloads: its files, one in two parts (3 requests)"), r.message);
        check(rising && !seen.isEmpty() && seen.last() == t.downloadBytes() && total == t.downloadBytes(),
              QStringLiteral("... with its progress, up to its size"));
        const QString extra = ai::modelFile(t, QStringLiteral("extra"));
        check(ai::modelFile(t, QStringLiteral("model")) == ai::modelDir(QStringLiteral("fake-tts")) + QStringLiteral("/voice.gguf")
                  && sha(readAll(extra)) == sha(mm1 + mm2) && QFileInfo::exists(ai::modelDir(QStringLiteral("fake-tts")) + QStringLiteral("/.checked"))
                  && ai::modelDir(QStringLiteral("fake-tts")).startsWith(home.path() + QStringLiteral("/data/")),
              QStringLiteral("... kept in the data folder (ai-models/<id>), the two parts joined and checked, marked as checked"), extra);
        const Done bad = waitFinished(d, [&] { d.download(QStringLiteral("fake-bad")); });
        check(!bad.ok && bad.message.contains(QLatin1String("SHA-256")) && !ai::isInstalled(*c.model(QStringLiteral("fake-bad")))
                  && QDir(ai::modelDir(QStringLiteral("fake-bad"))).entryList(QDir::Files | QDir::Hidden).isEmpty(),
              QStringLiteral("a file that is not the one the catalog names is not kept"), bad.message);
        const Done two = waitFinished(d, [&] { d.download(QStringLiteral("fake-two")); });
        const int requestsBefore = d.requests();
        writeFile(source + QStringLiteral("/fake-two--b.bin"), good);
        const Done two2 = waitFinished(d, [&] { d.download(QStringLiteral("fake-two")); });
        check(!two.ok && two2.ok && d.requests() == requestsBefore + 1 && ai::isInstalled(*c.model(QStringLiteral("fake-two"))),
              QStringLiteral("a download that stopped at the second file goes on there (the first is not fetched again)"), two2.message);
        const Done cancelled = waitFinished(d, [&] { d.download(QStringLiteral("fake-asr")); d.cancel(); });
        check(!cancelled.ok && cancelled.message.contains(QLatin1String("cancelled")) && !ai::isInstalled(*c.model(QStringLiteral("fake-asr")))
                  && !QFileInfo::exists(ai::modelDir(QStringLiteral("fake-asr")) + QStringLiteral("/asr.bin.part")),
              QStringLiteral("a cancelled download leaves nothing behind"), cancelled.message);
        const Done huge = waitFinished(d, [&] { d.download(QStringLiteral("fake-huge")); });
        check(!huge.ok && huge.message.contains(QLatin1String("memory")), QStringLiteral("a model too large for the computer is not downloaded"));

        // ---- 0.99.0: the voice packs of Read Aloud
        {
            using namespace qb::readaloud;
            const QString root = ai::modelsRoot() + QStringLiteral("/read-aloud-voices");
            check(!d.ensureVoicePack(QStringLiteral("de")) && !d.busy() && !ai::isInstalled(*c.model(QStringLiteral("voices-de"))),
                  QStringLiteral("a pack is not fetched while no Qwen model is installed"));
            const Done q = waitFinished(d, [&] { d.download(QStringLiteral("fake-qwen")); });
            check(q.ok && ai::isInstalled(*c.model(QStringLiteral("fake-qwen"))), QStringLiteral("a Qwen model downloads"), q.message);
            QElapsedTimer waited;
            waited.start();
            while (!voicePackInstalled(QStringLiteral("en")) && waited.elapsed() < 20000)
                QTest::qWait(20);
            check(voicePackInstalled(QStringLiteral("en")) && QFileInfo::exists(root + QStringLiteral("/en/female1-neutral.wav"))
                      && QFileInfo::exists(root + QStringLiteral("/en/voices.json")) && ai::isInstalled(*c.model(QStringLiteral("voices-en"))),
                  QStringLiteral("... and its download is followed by the voice pack of the Read Aloud language (checked, unpacked into read-aloud-voices/en/)"));
            const Done de = waitFinished(d, [&] { check(d.ensureVoicePack(QStringLiteral("de")), QStringLiteral("choosing German starts its pack's download")); });
            check(de.ok && QFileInfo::exists(root + QStringLiteral("/de/male1-calm.wav")) && !d.ensureVoicePack(QStringLiteral("de")),
                  QStringLiteral("a language chosen later: its pack is fetched; once installed it is not fetched again"), de.message);
            const Done badPack = waitFinished(d, [&] { d.download(QStringLiteral("voices-fr")); });
            check(!badPack.ok && !voicePackInstalled(QStringLiteral("fr")) && !QFileInfo::exists(root + QStringLiteral("/outside.wav"))
                      && !QFileInfo::exists(ai::modelDir(QStringLiteral("voices-fr")) + QStringLiteral("/.checked")),
                  QStringLiteral("a pack with a path outside its folder is refused and does not count as installed"), badPack.message);
            const Done ru = waitFinished(d, [&] { d.download(QStringLiteral("voices-ru")); });
            check(!ru.ok && ru.message.contains(QLatin1String("not published")) && !d.ensureVoicePack(QStringLiteral("ru")),
                  QStringLiteral("a pack with a placeholder for its size and SHA-256 is \"not published yet\": no download"), ru.message);
            // removing the last Qwen model removes the packs
            ai::removeModel(*c.model(QStringLiteral("fake-qwen")));
            check(removeVoicePacksIfNoQwen() && !voicePackInstalled(QStringLiteral("en")) && !voicePackInstalled(QStringLiteral("de"))
                      && !ai::isInstalled(*c.model(QStringLiteral("voices-en"))),
                  QStringLiteral("the last Qwen model is removed: the packs go with it (folders and zips)"));
        }
        check(toolhost::host().aiModelFile(QStringLiteral("fake-tts"), QStringLiteral("model")) == ai::modelFile(t, QStringLiteral("model"))
                  && toolhost::host().aiModelFile(QStringLiteral("fake-asr"), QStringLiteral("model")).isEmpty(),
              QStringLiteral("tool interface 1.3: aiModelFile() finds an installed model, \"\" for one that is not"));

        // a downloaded model with the engine: the catalog's memory figure goes to ModelMemory
        ai::Model m(QStringLiteral("read-aloud:fake-tts"));
        QString why;
        const bool ok = m.load(ai::modelFiles(t), QStringLiteral("cpu"), &why);
        check(ok && mm.models().value(0).bytes == 50 * MB && mm.models().value(0).name == QLatin1String("Fake Voice"),
              QStringLiteral("a catalog model loads; ModelMemory gets the catalog's memory figure (50 MB) and its title"), why);
        m.unload();
        QString err;
        check(ai::removeModel(t, &err) && !ai::isInstalled(t) && !QFileInfo::exists(ai::modelDir(QStringLiteral("fake-tts"))),
              QStringLiteral("a model is deleted with its folder"), err);
        check(!AiDownloads::instance().hasNetwork(), QStringLiteral("(the program's own download object is still unused)"));
    }

    // ================================================================ installing the engine
    std::printf("== installing the engine package\n");
    {
        ai::Engine &e = ai::Engine::instance();
        e.unloadForTests();
        qunsetenv("QB_AI_ENGINE_LIB");
        QString log;
        const QString pkg = work + QStringLiteral("/ai-engine.qbpkg");
        const QJsonObject manifest{{"id", "ai-engine"}, {"name", "AI Engine"}, {"version", "1.0"}, {"description", "The fake engine of the checks."},
                                   {"author", "Quillbinder"}, {"minApp", "0.93.0"}};
        check(makePackage(work + QStringLiteral("/stage-engine"), manifest, {{QStringLiteral("engine/lib/libqbai.so.1"), fakeLib}}, pkg, &log),
              QStringLiteral("a package of the fake engine (tools/make-package.py)"), log);
        const QString other = work + QStringLiteral("/handwriting-engine.qbpkg");
        writeFile(work + QStringLiteral("/libonnxruntime.so.1"), QByteArray("not really ONNX Runtime"));
        QJsonObject hm = manifest;
        hm[QStringLiteral("id")] = QStringLiteral("handwriting-engine");
        hm[QStringLiteral("name")] = QStringLiteral("Handwriting Engine");
        check(makePackage(work + QStringLiteral("/stage-hw"), hm, {{QStringLiteral("engine/lib/libonnxruntime.so.1"), work + QStringLiteral("/libonnxruntime.so.1")}}, other, &log),
              QStringLiteral("a package of a fake handwriting engine"), log);
        QString why;
        check(!AiDownloads::installEngineFile(ai::aiEngineId(), other, true, &why) && why.contains(QLatin1String("not a package of the AI Engine")),
              QStringLiteral("another package is not taken for the engine"), why);
        qputenv("QB_REQUIRE_SIGNED", "1");
        bool isUnsigned = false;
        check(!AiDownloads::installEngineFile(ai::aiEngineId(), pkg, false, &why, &isUnsigned) && isUnsigned && !ai::package(ai::aiEngineId()).installed,
              QStringLiteral("an unsigned engine package is refused without the writer's yes"), why);
        check(AiDownloads::installEngineFile(ai::aiEngineId(), pkg, true, &why) && ai::package(ai::aiEngineId()).usable()
                  && ai::package(ai::aiEngineId()).version == QLatin1String("1.0"),
              QStringLiteral("... and installed with it, as an add-on (from a file: a computer without a connection)"), why);
        const QString lib = e.libraryPath();
        check(lib == addons::addonsRoot() + QStringLiteral("/ai-engine/engine/lib/libqbai.so.1") && e.ensureLoaded(&why)
                  && e.loadedFrom() == lib && !e.restartNeeded(),
              QStringLiteral("the program finds the installed engine's libqbai and loads it"), lib + QLatin1Char(' ') + why);
        check(AiDownloads::installEngineFile(ai::aiEngineId(), pkg, true, &why) && e.restartNeeded(),
              QStringLiteral("installed again while it is loaded: the new one is used after a restart"), why);
        check(AiDownloads::installEngineFile(ai::handwritingEngineId(), other, true, &why)
                  && ai::onnxRuntimeLibrary() == addons::addonsRoot() + QStringLiteral("/handwriting-engine/engine/lib/libonnxruntime.so.1"),
              QStringLiteral("the handwriting engine: onnxRuntimeLibrary() names its ONNX Runtime"), ai::onnxRuntimeLibrary());

        auto *panel = new AiInstallPanel(ai::aiEngineId(), {QStringLiteral("tts")});
        panel->resize(760, 380);
        panel->show();
        QTest::qWait(50);
        const QString state = panel->findChild<QLabel *>(QStringLiteral("aiEngineState"))->text();
        const QTreeWidget *list = panel->findChild<QTreeWidget *>(QStringLiteral("aiModels"));
        check(state.contains(QLatin1String("1.0 is installed")) && state.contains(QLatin1String("started again"))
                  && panel->findChild<QPushButton *>(QStringLiteral("aiInstallEngine"))->text() == QLatin1String("Install Again")
                  && list->topLevelItemCount() == 4 && list->topLevelItem(0)->text(5) == QLatin1String("Not installed")
                  && list->topLevelItem(1)->text(5) == QLatin1String("Not published yet"),
              QStringLiteral("the panel: the installed engine (a restart pending), the catalog's voices with their state"), state);
        shot(panel, QStringLiteral("v0990h-panel-installed"));
        panel->selectModel(QStringLiteral("fake-huge"));
        check(!panel->findChild<QPushButton *>(QStringLiteral("aiDownloadModel"))->isEnabled()
                  && panel->findChild<QLabel *>(QStringLiteral("aiModelDetail"))->text().contains(QLatin1String("memory")),
              QStringLiteral("... a model too large for the computer: Download is off, and the panel says why"));
        shot(panel, QStringLiteral("v0990h-panel-too-large"));
        panel->selectModel(QStringLiteral("fake-tts"));
        QStringList progress;
        bool downloadOffWhileBusy = false;
        QObject::connect(&AiDownloads::instance(), &AiDownloads::progress, panel, [&](const QString &, qint64, qint64) {
            if (progress.isEmpty() && panel->findChild<QProgressBar *>(QStringLiteral("aiProgress"))->isVisible()) {
                shot(panel, QStringLiteral("v0990h-panel-downloading"));
                downloadOffWhileBusy = !panel->findChild<QPushButton *>(QStringLiteral("aiDownloadModel"))->isEnabled()
                                       && !panel->findChild<QPushButton *>(QStringLiteral("aiInstallEngine"))->isEnabled();
            }
            progress << QStringLiteral("x");
        });
        const Done r = waitFinished(AiDownloads::instance(), [&] { panel->downloadSelected(); });
        QTest::qWait(20);
        check(r.ok && list->currentItem() && list->currentItem()->text(5) == QLatin1String("Installed") && !progress.isEmpty()
                  && !panel->findChild<QProgressBar *>(QStringLiteral("aiProgress"))->isVisible()
                  && panel->findChild<QPushButton *>(QStringLiteral("aiDeleteModel"))->isEnabled(),
              QStringLiteral("the panel's Download: progress shown, then the model is installed and can be deleted"), r.message);
        check(downloadOffWhileBusy, QStringLiteral("... while it downloads, Download and Install the Engine are off"));
        shot(panel, QStringLiteral("v0990h-panel-model-installed"));
        panel->deleteSelected();
        check(list->currentItem() && list->currentItem()->text(5) == QLatin1String("Not installed"), QStringLiteral("the panel's Delete"));
        delete panel;

        // from a "release" (a folder with the engine's package; the address otherwise is this version's release)
        QString err;
        addons::removeAddon(ai::aiEngineId(), &err);
        const QString rel = work + QStringLiteral("/engine-release");
        QDir().mkpath(rel);
        QFile::copy(pkg, rel + QStringLiteral("/ai-engine.qbpkg"));
        qputenv("QB_AI_ENGINE_SOURCE", rel.toUtf8());
        AiDownloads d;
        const Done refused = waitFinished(d, [&] { d.installEngine(ai::aiEngineId()); });
        check(!refused.ok && !ai::package(ai::aiEngineId()).installed && d.requests() == 1,
              QStringLiteral("Install the Engine: a downloaded package must be signed by the project"), refused.message);
        qunsetenv("QB_REQUIRE_SIGNED");
        const Done got = waitFinished(d, [&] { d.installEngine(ai::aiEngineId()); });
        check(got.ok && ai::package(ai::aiEngineId()).usable() && d.requests() == 2
                  && !QFileInfo::exists(QDir::tempPath() + QStringLiteral("/quillbinder-engine-%1").arg(QCoreApplication::applicationPid())),
              QStringLiteral("... (test builds take an unsigned one): downloaded, checked and installed; the download is removed"), got.message);
        QFile::remove(rel + QStringLiteral("/ai-engine.qbpkg"));
        const Done missing = waitFinished(d, [&] { d.installEngine(ai::aiEngineId()); });
        check(!missing.ok && missing.message.contains(QLatin1String("failed")), QStringLiteral("no package at the address: the download fails and says so"),
              missing.message);
        qunsetenv("QB_AI_ENGINE_SOURCE");
        QWidget parent;
        auto *asrPanel = qobject_cast<AiInstallPanel *>(toolhost::host().aiPanel({QStringLiteral("asr")}, &parent));
        check(asrPanel && asrPanel->parent() == &parent && asrPanel->kinds() == QStringList({QStringLiteral("asr")})
                  && asrPanel->findChild<QTreeWidget *>(QStringLiteral("aiModels"))->topLevelItemCount() == 3,
              QStringLiteral("tool interface 1.3: aiPanel() gives the program's panel for the kinds asked for, owned by the tool's widget"));
    }

    // ================================================================ the package recipes
    std::printf("== the engine packages' recipes\n");
    {
        // audit AU-04: the program fetches only its own version's engine, so a release without one must not go out
        const QString wf = QString::fromUtf8(readAll(kSource + QStringLiteral("/.github/workflows/release.yml")));
        check(wf.contains(QLatin1String("::error::the package $e was not built")) && !wf.contains(QLatin1String("it is left out of this release")),
              QStringLiteral("release.yml: a missing engine package (ai-engine, handwriting-engine) stops the release instead of a warning"));
    }
    {
        const QString pack =qEnvironmentVariableIsSet("QB_AI_ENGINE_DIR") ? qEnvironmentVariable("QB_AI_ENGINE_DIR") : kSource + QStringLiteral("/build/ai-engine");
        const QString qbai = kBuild + QStringLiteral("/qbai/libqbai.so.1");
        for (const QString id : {QStringLiteral("ai-engine"), QStringLiteral("handwriting-engine")}) {
            const QJsonObject m = QJsonDocument::fromJson(readAll(kSource + QStringLiteral("/addons/") + id + QStringLiteral("/manifest.json"))).object();
            check(m.value(QStringLiteral("id")).toString() == id && m.value(QStringLiteral("minApp")).toString() == QLatin1String("0.99.0")
                      && QFileInfo(kSource + QStringLiteral("/addons/") + id + QStringLiteral("/stage.sh")).isExecutable()
                      && QFileInfo::exists(kSource + QStringLiteral("/addons/") + id + QStringLiteral("/may-be-missing")),
                  QStringLiteral("addons/%1: its manifest (for 0.99.0 and later), its stage script, left out of a build without its libraries").arg(id));
        }
        if (!QFileInfo::exists(pack + QStringLiteral("/lib/libggml.so.1")) || !QFileInfo::exists(qbai)) {
            info(QStringLiteral("(the AI engine's package not built: needs the pack in %1 (tools/get-ai-engine.sh) and %2)").arg(pack, qbai));
        } else {
            const QString stage = work + QStringLiteral("/stage-real");
            QDir().mkpath(stage);
            QFile::copy(kSource + QStringLiteral("/addons/ai-engine/manifest.json"), stage + QStringLiteral("/manifest.json"));
            int rc = 0;
            QString out = run(QStringLiteral("cp"), {QStringLiteral("-r"), kSource + QStringLiteral("/addons/ai-engine/payload"), stage + QStringLiteral("/")}, &rc);
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("QB_AI_ENGINE_DIR"), pack);
            out += run(QStringLiteral("bash"), {kSource + QStringLiteral("/addons/ai-engine/stage.sh"), kBuild, stage}, &rc, env);
            const QString pkg = work + QStringLiteral("/real-ai-engine.qbpkg");
            if (rc == 0)
                out += run(QStringLiteral("python3"), {QStringLiteral("-I"), kSource + QStringLiteral("/tools/make-package.py"), QStringLiteral("addon"), stage, pkg}, &rc);
            check(rc == 0 && QFileInfo::exists(pkg), QStringLiteral("addons/ai-engine/stage.sh and make-package.py make the engine package"), out.right(1500));
            const addons::PackageInfo infoPkg = addons::inspect(pkg);
            QStringList libs;
            for (const addons::FileEntry &f : infoPkg.manifest.files)
                if (f.path.startsWith(QLatin1String("engine/lib/")))
                    libs << f.path.mid(11);
            check(infoPkg.manifest.id == QLatin1String("ai-engine") && infoPkg.manifest.runtime == QLatin1String(QB_RUNTIME_ID)
                      && libs.contains(QLatin1String("libqbai.so.1")) && libs.contains(QLatin1String("libwhisper.so.1")),
                  QStringLiteral("... an add-on for this runtime, with libqbai and the engine's libraries (%1 files)").arg(libs.size()), libs.join(QLatin1String(", ")));
            const QString checkOut = run(QStringLiteral("python3"), {QStringLiteral("-I"), kSource + QStringLiteral("/tools/check-engine-package.py"), pkg}, &rc);
            check(rc == 0 && checkOut.contains(QLatin1String("0 failure(s)")) && checkOut.contains(QLatin1String("needs nothing outside")),
                  QStringLiteral("tools/check-engine-package.py: the expected libraries, the licenses, ldd needs only the package and the base system, RUNPATH $ORIGIN"),
                  checkOut.right(2000));
        }
        const QString onnx = qEnvironmentVariableIsSet("QB_ONNXRUNTIME_DIR") ? qEnvironmentVariable("QB_ONNXRUNTIME_DIR") : kSource + QStringLiteral("/build/onnxruntime");
        const QString stage = work + QStringLiteral("/stage-hw-real");
        QDir().mkpath(stage);
        QFile::copy(kSource + QStringLiteral("/addons/handwriting-engine/manifest.json"), stage + QStringLiteral("/manifest.json"));
        int cprc = 0;
        run(QStringLiteral("cp"), {QStringLiteral("-r"), kSource + QStringLiteral("/addons/handwriting-engine/payload"), stage + QStringLiteral("/")}, &cprc);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("QB_ONNXRUNTIME_DIR"), QFileInfo::exists(onnx + QStringLiteral("/version.txt")) ? onnx : work + QStringLiteral("/no-onnxruntime"));
        int rc = 0;
        const QString out = run(QStringLiteral("bash"), {kSource + QStringLiteral("/addons/handwriting-engine/stage.sh"), kBuild, stage}, &rc, env);
        if (!QFileInfo::exists(onnx + QStringLiteral("/version.txt"))) {
            check(rc == 3 && out.contains(QLatin1String("ONNX Runtime is missing")),
                  QStringLiteral("addons/handwriting-engine/stage.sh without the ONNX Runtime pack: left out (exit 3) with a clear message"), out);
        } else {
            const QString pkg = work + QStringLiteral("/real-handwriting-engine.qbpkg");
            QString o2 = run(QStringLiteral("python3"), {QStringLiteral("-I"), kSource + QStringLiteral("/tools/make-package.py"), QStringLiteral("addon"), stage, pkg}, &rc);
            o2 += run(QStringLiteral("python3"), {QStringLiteral("-I"), kSource + QStringLiteral("/tools/check-engine-package.py"), pkg}, &rc);
            check(rc == 0 && o2.contains(QLatin1String("0 failure(s)")) && o2.contains(QLatin1String("needs nothing outside")),
                  QStringLiteral("addons/handwriting-engine: the package of ONNX Runtime, checked by tools/check-engine-package.py"), o2.right(1500));
        }
    }

    // ================================================================ the real engine (a child process)
    std::printf("== the real engine\n");
    {
        const QString real = qEnvironmentVariableIsSet("QB_AI_REAL_ENGINE") ? qEnvironmentVariable("QB_AI_REAL_ENGINE") : kBuild + QStringLiteral("/qbai/libqbai.so.1");
        QString model = qEnvironmentVariable("QB_AI_WHISPER_MODEL");
        if (model.isEmpty())
            model = qEnvironmentVariable("QB_SPEECH_MODEL_FILE");
        QString wav = qEnvironmentVariable("QB_AI_WHISPER_SAMPLE");
        if (wav.isEmpty())
            wav = (qEnvironmentVariableIsSet("QB_WHISPER_DIR") ? qEnvironmentVariable("QB_WHISPER_DIR") : kSource + QStringLiteral("/build/whisper"))
                  + QStringLiteral("/samples/jfk.wav");
        if (!QFileInfo::exists(real)) {
            info(QStringLiteral("(the real engine not checked: %1 is not built (tools/get-ai-engine.sh, then configure and build))").arg(real));
        } else {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("QB_AI_ENGINE_LIB"), real);
            env.remove(QStringLiteral("QB_AI_CATALOG"));
            int rc = 0;
            const QString out = run(QCoreApplication::applicationFilePath(), {QStringLiteral("--child-real"), model, wav}, &rc, env, 900000);
            int passes = 0;
            for (const QString &line : out.split(QLatin1Char('\n'))) {
                if (line.startsWith(QLatin1String("PASS  ")))
                    ++passes;
                if (line.startsWith(QLatin1String("PASS  ")) || line.startsWith(QLatin1String("FAIL  ")))
                    check(line.startsWith(QLatin1String("PASS")), line.mid(6));
                else if (line.startsWith(QLatin1String("      ")))
                    info(line.mid(6));
            }
            check(rc == 0 && passes >= 3, QStringLiteral("the real engine's child process ends without failures"), out.right(1500));
        }
    }

    sysmem::clearFakeRam();
    std::printf("%d failure(s)\n", failures);
    return failures;
}
