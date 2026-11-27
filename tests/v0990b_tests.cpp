// Scripted run for version 0.99.0: tool interface 1.3, the one rule for the AI models, the system's memory figures, the
// message "Model change in progress", File > Options > General > AI Models (items F3, F4, 115).
//  - the AI's cores (75 %, 6 or fewer cores: minus 1, at least 1) and the shared setting;
//  - the 50 % and 80 % rule with fake figures (RAM and a graphics card), the longest unused model first;
//  - the idle unload (a short time for the test), the low-memory signal (idle models only);
//  - the message line "Model change in progress" and the message that says why a load was refused;
//  - a tool of interface 1.3 (registers, is asked to unload, is refused a load); tools of 1.0 and 1.2 still work;
//  - the page AI Models: the default, a change kept after the settings are read again, the memory line.
//
//   xvfb-run -a ./qb_v0990b_tests /tmp/shots
#include "compile/PluginManager.h"
#include "core/BinderModel.h"
#include "core/KeepAlive.h"
#include "core/ModelMemory.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "core/SystemMemory.h"
#include "tools/ToolApi.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/MainWindow.h"
#include "ui/MessageLine.h"
#include "ui/OptionStorage.h"
#include "ui/OptionsDialog.h"
#include "ui/Theme.h"
#include "ui/ToolHost.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>

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

static constexpr qint64 MB = 1000 * 1000;

// a clock the checks move by hand
static qint64 g_now = 1000000;

// RAM of 1000 MB with `usedMb` in use
static void fakeRam(int usedMb)
{
    sysmem::setFakeRam({1000 * MB, (1000 - usedMb) * MB});
}

// a model list for the rule: ids that were unloaded land in `log`
struct Rig {
    ModelMemory mm;
    QStringList log;
    ModelMemory::Unloader unloader;
    bool busy = false;
    Rig()
    {
        g_now = 1000000;
        mm.setClock([] { return g_now; });
        unloader = [this](const QString &id) {
            if (busy)
                return false;
            log << id;
            return true;
        };
    }
    // a model that is loaded already (asked for, then reported), `age` seconds unused
    void add(const QString &id, int mb, const QString &where, int ageSeconds)
    {
        sysmem::setFakeRam({1000 * MB, 1000 * MB});   // nothing in use: the load is allowed
        const qint64 saved = g_now;
        g_now = saved - qint64(ageSeconds) * 1000;
        mm.requestLoad(id, id, mb * MB, where, unloader);
        mm.modelLoaded(id);
        g_now = saved;
    }
};

static QObject *toolObject(const QString &id)
{
    for (tools::ToolInterface *t : compile::PluginManager::instance().tools())
        if (t->id() == id)
            return dynamic_cast<QObject *>(t);
    return nullptr;
}

static bool callLoad(QObject *tool, const QString &id, int mb, const QString &where)
{
    bool r = false;
    QMetaObject::invokeMethod(tool, "load", Qt::DirectConnection, Q_RETURN_ARG(bool, r), Q_ARG(QString, id), Q_ARG(QString, id),
                              Q_ARG(qint64, qint64(mb) * MB), Q_ARG(QString, where));
    return r;
}

static QStringList callList(QObject *tool, const char *slot)
{
    QStringList r;
    QMetaObject::invokeMethod(tool, slot, Qt::DirectConnection, Q_RETURN_ARG(QStringList, r));
    return r;
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    const QString buildDir = QStringLiteral(QB_TEST_BUILD_DIR);
    qputenv("QB_PLUGIN_PATH", (buildDir + QStringLiteral("/tests/tool12-plugins:") + buildDir + QStringLiteral("/tests/tool13-plugins")).toUtf8());
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990bTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990bTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();

    // ================================================================ the AI's cores
    std::printf("== the AI's cores\n");
    {
        const QList<QPair<int, int>> cases = {{1, 1}, {2, 1}, {3, 2}, {4, 3}, {5, 4}, {6, 5}, {7, 5}, {8, 6}, {12, 9}, {16, 12}, {24, 18}, {0, 1}};
        QStringList bad;
        for (const auto &c : cases)
            if (ModelMemory::defaultThreads(c.first) != c.second)
                bad << QStringLiteral("%1 cores -> %2 (expected %3)").arg(c.first).arg(ModelMemory::defaultThreads(c.first)).arg(c.second);
        check(bad.isEmpty(), QStringLiteral("default cores: 1->1, 2->1, 4->3, 6->5, 7->5, 8->6, 12->9, 16->12 (6 or fewer: minus 1; at least 1)"),
              bad.join(QLatin1String("; ")));
        check(ModelMemory::defaultThreads() == ModelMemory::defaultThreads(QThread::idealThreadCount()),
              QStringLiteral("the cores are the ones Qt counts (QThread::idealThreadCount)"));
        ModelMemory mm;
        check(mm.aiThreads() == ModelMemory::defaultThreads(), QStringLiteral("nothing set: the default"));
        mm.setThreadSetting(2);
        check(mm.aiThreads() == 2, QStringLiteral("a set number is used"));
    }

    // ================================================================ the rule: 50 % and 80 %
    std::printf("== the rule for the memory (fake figures)\n");
    {
        Rig r;
        fakeRam(400);
        modelmem::Decision d = r.mm.requestLoad(QStringLiteral("a"), QStringLiteral("A"), 300 * MB, QStringLiteral("ram"), r.unloader);
        check(d.allowed && d.unloaded.isEmpty() && r.mm.changeActive(), QStringLiteral("RAM 40 % used, 300 MB more (70 %): allowed, nothing unloaded, the change is under way"));
        r.mm.modelLoaded(QStringLiteral("a"));
        check(!r.mm.changeActive() && r.mm.isLoaded(QStringLiteral("a")), QStringLiteral("... until the model is ready"));
    }
    {
        Rig r;
        r.add(QStringLiteral("old"), 200, QStringLiteral("ram"), 600);
        r.add(QStringLiteral("new1"), 100, QStringLiteral("ram"), 10);
        fakeRam(600);
        const modelmem::Decision d = r.mm.requestLoad(QStringLiteral("c"), QStringLiteral("C"), 100 * MB, QStringLiteral("ram"), r.unloader);
        check(d.allowed && r.log == QStringList{QStringLiteral("old")} && !r.mm.isLoaded(QStringLiteral("old")) && r.mm.isLoaded(QStringLiteral("new1")),
              QStringLiteral("RAM 60 % used: the model unused longest goes first (40 %, then 50 % with the new one: allowed)"), r.log.join(QLatin1Char(',')));
    }
    {
        Rig r;
        fakeRam(850);
        QStringList seen;
        QObject::connect(&r.mm, &ModelMemory::refused, &r.mm, [&seen](const QString &why) { seen << why; });
        const modelmem::Decision d = r.mm.requestLoad(QStringLiteral("big"), QStringLiteral("Big Model"), 10 * MB, QStringLiteral("ram"), r.unloader);
        check(!d.allowed && d.reason.contains(QLatin1String("Big Model")) && d.reason.contains(QLatin1String("85")) && d.reason.contains(QLatin1String("80"))
                  && seen.size() == 1 && !r.mm.changeActive() && !r.mm.isLoaded(QStringLiteral("big")),
              QStringLiteral("RAM 85 % used and nothing of ours to unload: refused, the reason names the model and the figures, no change under way"), d.reason);
    }
    {
        Rig r;
        r.add(QStringLiteral("a"), 100, QStringLiteral("ram"), 300);
        r.add(QStringLiteral("b"), 200, QStringLiteral("ram"), 100);
        fakeRam(550);
        const modelmem::Decision d = r.mm.requestLoad(QStringLiteral("c"), QStringLiteral("C"), 400 * MB, QStringLiteral("ram"), r.unloader);
        check(d.allowed && r.log == QStringList({QStringLiteral("a"), QStringLiteral("b")}),
              QStringLiteral("55 % used, the new model would bring it to 85 %: the idle models of ours go, longest unused first, then it fits"), r.log.join(QLatin1Char(',')));
    }
    {
        Rig r;
        r.add(QStringLiteral("a"), 100, QStringLiteral("ram"), 300);
        fakeRam(700);
        const modelmem::Decision d = r.mm.requestLoad(QStringLiteral("c"), QStringLiteral("C"), 500 * MB, QStringLiteral("ram"), r.unloader);
        check(!d.allowed && d.reason.contains(QLatin1String("Not enough memory")),
              QStringLiteral("still over 80 % with every model of ours gone: refused (the caller offers a smaller model or the CPU)"), d.reason);
    }
    {
        Rig r;
        r.busy = true;
        r.add(QStringLiteral("a"), 300, QStringLiteral("ram"), 300);
        fakeRam(600);
        const modelmem::Decision d = r.mm.requestLoad(QStringLiteral("c"), QStringLiteral("C"), 300 * MB, QStringLiteral("ram"), r.unloader);
        check(!d.allowed && r.mm.isLoaded(QStringLiteral("a")) && r.log.isEmpty(),
              QStringLiteral("a model that cannot be unloaded now (in use) stays, and the load is refused when it does not fit"));
    }
    {
        Rig r;
        r.mm.reportDeviceMemory(QStringLiteral("gpu:Test Card"), 8000 * MB, 5000 * MB);   // 37.5 % used
        r.add(QStringLiteral("ram-model"), 100, QStringLiteral("ram"), 900);
        fakeRam(300);
        modelmem::Decision d = r.mm.requestLoad(QStringLiteral("g1"), QStringLiteral("G1"), 3000 * MB, QStringLiteral("gpu:Test Card"), r.unloader);
        check(d.allowed && d.unloaded.isEmpty(), QStringLiteral("card 37.5 % used, a model of 3000 MB (75 %): allowed"));
        r.mm.modelLoaded(QStringLiteral("g1"));
        r.mm.reportDeviceMemory(QStringLiteral("gpu:Test Card"), 8000 * MB, 2000 * MB);   // 75 % used now
        d = r.mm.requestLoad(QStringLiteral("g2"), QStringLiteral("G2"), 3000 * MB, QStringLiteral("gpu:Test Card"), r.unloader);
        check(d.allowed && d.unloaded == QStringList{QStringLiteral("g1")} && r.mm.isLoaded(QStringLiteral("ram-model")),
              QStringLiteral("card 75 % used: the card's own model goes, the RAM model stays (each memory by itself), the new one fits"), d.unloaded.join(QLatin1Char(',')));
        r.mm.modelLoaded(QStringLiteral("g2"));
        const QStringList devs = r.mm.devices();
        check(devs == QStringList({QStringLiteral("ram"), QStringLiteral("gpu:Test Card")}) && r.mm.device(QStringLiteral("gpu:Test Card")).valid(),
              QStringLiteral("the memories with figures: RAM, then each card reported"), devs.join(QLatin1Char(',')));
        d = r.mm.requestLoad(QStringLiteral("x"), QStringLiteral("X"), 1 * MB, QStringLiteral("gpu:Unknown"), r.unloader);
        check(d.allowed, QStringLiteral("a card without figures never refuses a load"));
    }

    // ================================================================ the idle unload
    std::printf("== the idle unload\n");
    {
        check(ModelMemory::kDefaultIdleMinutes == 10, QStringLiteral("the default time unused is 10 minutes"));
        Rig r;
        check(r.mm.idleMinutes() == 10, QStringLiteral("... also in a new list"));
        r.mm.setIdleMinutes(5);
        r.add(QStringLiteral("a"), 10, QStringLiteral("ram"), 290);
        r.add(QStringLiteral("b"), 10, QStringLiteral("ram"), 310);
        const QStringList gone = r.mm.unloadIdle();
        check(gone == QStringList{QStringLiteral("b")} && r.mm.isLoaded(QStringLiteral("a")),
              QStringLiteral("a model unused for the set time (5 minutes here) is unloaded, one used 290 s ago is not"), gone.join(QLatin1Char(',')));
        r.mm.modelUsed(QStringLiteral("a"));
        g_now += 200 * 1000;
        check(r.mm.unloadIdle().isEmpty(), QStringLiteral("a use starts the time again"));
        g_now += 150 * 1000;
        check(r.mm.unloadIdle() == QStringList{QStringLiteral("a")} && r.mm.models().isEmpty(), QStringLiteral("... and 5 minutes after the last use it goes"));
    }
    {
        // the timer, with a short time and the real clock
        ModelMemory mm;
        QStringList log;
        fakeRam(100);
        mm.setIdleMs(250);
        mm.requestLoad(QStringLiteral("t"), QStringLiteral("T"), 1 * MB, QStringLiteral("ram"), [&log](const QString &id) { log << id; return true; });
        mm.modelLoaded(QStringLiteral("t"));
        mm.startWatching(80);
        settle(120);
        const bool stillThere = mm.isLoaded(QStringLiteral("t"));
        settle(500);
        check(stillThere && !mm.isLoaded(QStringLiteral("t")) && log == QStringList{QStringLiteral("t")},
              QStringLiteral("the timer unloads an idle model by itself (250 ms test time)"), log.join(QLatin1Char(',')));
    }

    // ================================================================ the low-memory signal
    std::printf("== the system's low-memory signal\n");
    {
        Rig r;
        r.add(QStringLiteral("idle"), 10, QStringLiteral("ram"), 120);
        r.add(QStringLiteral("busy"), 10, QStringLiteral("ram"), 5);
        r.mm.startWatching(60000);
        check(r.mm.monitor() != nullptr, QStringLiteral("the program watches the system's signal (%1)").arg(
                  r.mm.monitor() && r.mm.monitor()->active() ? r.mm.monitor()->method() : QStringLiteral("no system signal on this computer: injected here")));
        r.mm.monitor()->injectLowMemory();
        check(r.log == QStringList{QStringLiteral("idle")} && r.mm.isLoaded(QStringLiteral("busy")),
              QStringLiteral("the signal unloads the idle models at once (long before the 10 minutes) and not the one used 5 s ago"), r.log.join(QLatin1Char(',')));
    }
    {
        const sysmem::Figures real = sysmem::ram();
        check(real.valid() && real.total > 100 * MB, QStringLiteral("Linux: /proc/meminfo gives total and available memory"),
              QStringLiteral("%1 / %2").arg(real.total).arg(real.available));
        fakeRam(250);
        check(qAbs(sysmem::ram().usedShare() - 0.25) < 1e-9, QStringLiteral("the test hook injects figures"));
        sysmem::clearFakeRam();
    }

    // ================================================================ the program: message line and tools
    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("Lamp"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    compile::PluginManager::instance().load();
    ModelMemory &mm = ModelMemory::instance();
    mm.reset();
    MainWindow w;
    w.resize(1280, 800);
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);
    toolhost::host().attachTools();

    std::printf("== the message line\n");
    MessageLine *ml = w.messageLine();
    check(ml && !ml->isVisible(), QStringLiteral("the message line has no height while there is nothing to say"));
    if (ml) {
        mm.changeBegin();
        settle(50);
        check(ml->isVisible() && ml->textLabel()->text() == QLatin1String("Model change in progress") && ml->shownText().isEmpty(),
              QStringLiteral("a model change under way: the line says \"Model change in progress\""), ml->textLabel()->text());
        shot(&w, QStringLiteral("v0990b-model-change"));
        toolhost::host().showMessage(QStringLiteral("A short message"), 3000);
        settle(50);
        check(ml->shownText() == QLatin1String("A short message"), QStringLiteral("a short message of a tool goes before it"));
        ml->dismiss();
        check(ml->isVisible() && ml->textLabel()->text() == QLatin1String("Model change in progress"), QStringLiteral("... and when it is gone the standing text is back"));
        mm.changeBegin();
        mm.changeEnd();
        check(ml->isVisible(), QStringLiteral("two changes at once: the line stays until both are over"));
        mm.changeEnd();
        settle(50);
        check(!ml->isVisible() && !mm.changeActive(), QStringLiteral("the change is over: the line is gone"));
    }

    std::printf("== tool interface 1.3\n");
    check(toolhost::host().apiVersion() == 130, QStringLiteral("the program offers tool interface 1.3"), QString::number(toolhost::host().apiVersion()));
    QObject *t13 = toolObject(QStringLiteral("model-test"));
    QObject *t12 = toolObject(QStringLiteral("tool-12"));
    QObject *t10 = toolObject(QStringLiteral("old-tool"));
    check(t13 != nullptr, QStringLiteral("the test tool of interface 1.3 is loaded"), compile::PluginManager::instance().messages().join(QLatin1String("; ")));
    check(t12 != nullptr && toolhost::Host::models(dynamic_cast<tools::ToolInterface *>(t12)) == nullptr,
          QStringLiteral("a tool built for 1.2 is loaded and has no 1.3 part (ToolModels)"));
    check(t13 && toolhost::Host::models(dynamic_cast<tools::ToolInterface *>(t13)) != nullptr,
          QStringLiteral("the 1.3 tool has the ToolModels part"));
    (void)t10;
    if (t12) {
        toolhost::host().runCommand(QStringLiteral("tool-12.run"));
        settle(100);
        check(ml && ml->shownText() == QLatin1String("Tool 1.2 says hello")
                  && toolhost::host().setting(QStringLiteral("tool-12"), QStringLiteral("apiVersion")).toInt() == 130,
              QStringLiteral("a 1.2 tool still runs in a 1.3 program: it asks the host and shows its message"), ml ? ml->shownText() : QString());
        if (ml)
            ml->clearAll();
    }
    if (t13) {
        mm.reset();
        as.aiThreads = 0;
        as.save();
        int thr = 0;
        QMetaObject::invokeMethod(t13, "threads", Qt::DirectConnection, Q_RETURN_ARG(int, thr));
        check(thr == ModelMemory::defaultThreads(), QStringLiteral("the tool asks for the AI's cores: the default"), QString::number(thr));
        as.aiThreads = 2;
        as.save();
        QMetaObject::invokeMethod(t13, "threads", Qt::DirectConnection, Q_RETURN_ARG(int, thr));
        check(thr == 2, QStringLiteral("... and gets the number set in the options"), QString::number(thr));
        as.aiThreads = 0;
        as.save();

        // the registry through the interface
        fakeRam(600);   // 60 % used
        g_now = 5000000;
        mm.setClock([] { return g_now; });
        const bool a = callLoad(t13, QStringLiteral("model-test:a"), 100, QStringLiteral("ram"));
        g_now += 1000;
        check(a && mm.isLoaded(QStringLiteral("model-test:a")) && !mm.changeActive(),
              QStringLiteral("the tool asks, is allowed, loads and reports: the program lists the model; the change is over"));
        g_now += 1000;
        const bool b = callLoad(t13, QStringLiteral("model-test:b"), 100, QStringLiteral("ram"));
        const QStringList log = callList(t13, "unloadLog");
        check(b && log == QStringList{QStringLiteral("model-test:a")} && !mm.isLoaded(QStringLiteral("model-test:a")) && mm.isLoaded(QStringLiteral("model-test:b")),
              QStringLiteral("RAM 60 % used: the program asks the tool to unload its model unused longest (ToolModels), then the new one loads"), log.join(QLatin1Char(',')));
        QMetaObject::invokeMethod(t13, "use", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("model-test:b")));
        const QList<modelmem::LoadedModel> list = mm.models();
        check(list.size() == 1 && list.first().bytes == 100 * MB && list.first().name == QLatin1String("model-test:b") && list.first().where == QLatin1String("ram"),
              QStringLiteral("the list holds the model with its size and place"));

        fakeRam(850);
        const bool c = callLoad(t13, QStringLiteral("model-test:c"), 40, QStringLiteral("ram"));
        QString why;
        QMetaObject::invokeMethod(t13, "reason", Qt::DirectConnection, Q_RETURN_ARG(QString, why));
        check(c, QStringLiteral("85 % used: the tool's model b goes (75 %), 4 % more: allowed"), why);
        fakeRam(900);
        const bool d2 = callLoad(t13, QStringLiteral("model-test:d"), 300, QStringLiteral("ram"));
        QMetaObject::invokeMethod(t13, "reason", Qt::DirectConnection, Q_RETURN_ARG(QString, why));
        check(!d2 && why.contains(QLatin1String("Not enough memory")) && !mm.isLoaded(QStringLiteral("model-test:d")),
              QStringLiteral("a load that would bring the memory over 80 % is refused with a reason"), why);
        settle(50);
        check(ml && ml->shownText().contains(QLatin1String("Not enough memory")) && !mm.changeActive(),
              QStringLiteral("... the message line says why, and no change stays under way"), ml ? ml->shownText() : QString());
        shot(&w, QStringLiteral("v0990b-refused"));
        if (ml)
            ml->clearAll();
        QMetaObject::invokeMethod(t13, "dropOwn", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("model-test:c")));
        check(!mm.isLoaded(QStringLiteral("model-test:c")), QStringLiteral("a model the tool unloaded itself leaves the list"));

        // a model that is busy refuses to go
        fakeRam(400);
        callLoad(t13, QStringLiteral("model-test:e"), 100, QStringLiteral("ram"));
        QMetaObject::invokeMethod(t13, "setBusy", Qt::DirectConnection, Q_ARG(bool, true));
        const int before = callList(t13, "unloadLog").size();
        fakeRam(600);
        g_now += 1000;
        callLoad(t13, QStringLiteral("model-test:f"), 100, QStringLiteral("ram"));
        check(mm.isLoaded(QStringLiteral("model-test:e")) && callList(t13, "unloadLog").size() == before,
              QStringLiteral("a tool that says its model is in use keeps it"));
        QMetaObject::invokeMethod(t13, "setBusy", Qt::DirectConnection, Q_ARG(bool, false));
        // the idle unload reaches the tool, too
        mm.setIdleMs(5000);
        g_now += 60000;
        const QStringList gone = mm.unloadIdle();
        check(gone.contains(QStringLiteral("model-test:e")) && mm.models().isEmpty(), QStringLiteral("the idle unload asks the tool as well"), gone.join(QLatin1Char(',')));

        // keepRunning does nothing on Linux, but the reasons are kept
        QMetaObject::invokeMethod(t13, "keep", Qt::DirectConnection, Q_ARG(bool, true));
        const bool on = keepalive::active() && keepalive::reasons() == QStringList{QStringLiteral("model test job")};
        QMetaObject::invokeMethod(t13, "keep", Qt::DirectConnection, Q_ARG(bool, false));
        check(on && !keepalive::active(), QStringLiteral("keepRunning: the reason is kept while the job runs, nothing else happens on Linux"));
        // a call from a worker thread waits for the main thread
        bool fromThread = false;
        QThread *th = QThread::create([&fromThread] {
            tools::ToolHost *h = &toolhost::host();
            h->reportDeviceMemory(QStringLiteral("gpu:Thread Card"), 1000 * MB, 900 * MB);
            h->lowerThreadPriority();
            fromThread = true;
        });
        th->start();
        QElapsedTimer tm;
        tm.start();
        while (!th->isFinished() && tm.elapsed() < 5000)
            settle(20);
        th->wait();
        check(fromThread && mm.device(QStringLiteral("gpu:Thread Card")).valid(), QStringLiteral("a call from a worker thread reaches the program's list"));
        mm.reset();
        sysmem::clearFakeRam();
    }

    // ================================================================ File > Options > General > AI Models
    std::printf("== File > Options > General > AI Models\n");
    {
        as.aiThreads = 0;
        as.aiUnloadMinutes = 10;
        as.save();
        const int defThreads = ModelMemory::defaultThreads();
        mm.setClock(nullptr);
        mm.requestLoad(QStringLiteral("page:m"), QStringLiteral("Page Model"), 150 * MB, QStringLiteral("ram"), [](const QString &) { return true; });
        mm.modelLoaded(QStringLiteral("page:m"));
        mm.reportDeviceMemory(QStringLiteral("gpu:Test Card"), 8000 * MB, 6000 * MB);
        OptionsDialog d(&w, 0);
        d.showSection(OptionsDialog::SGeneral, QStringLiteral("AI Models"));
        d.show();
        settle(300);
        auto *thr = d.findChild<QSpinBox *>(QStringLiteral("optAiThreads"));
        auto *unl = d.findChild<QSpinBox *>(QStringLiteral("optAiUnload"));
        auto *mem = d.findChild<QLabel *>(QStringLiteral("optAiMemory"));
        check(thr && unl && mem && thr->value() == defThreads && unl->value() == 10 && thr->isVisible(),
              QStringLiteral("the page shows the default cores and \"Unload a model after 10 minutes unused\""), thr ? QString::number(thr->value()) : QString());
        check(optstore::keyOf(thr) == QLatin1String("app/aiThreads") && optstore::keyOf(unl) == QLatin1String("app/aiUnloadMinutes")
                  && thr->toolTip().contains(QLatin1String("Fixed:")),
              QStringLiteral("both are kept by the program for the device, not by a project"));
        check(mem && mem->text().contains(QLatin1String("RAM:")) && !mem->text().contains(QLatin1String("not known")) && mem->text().contains(QLatin1String("Test Card")) && mem->text().contains(QLatin1String("Page Model")),
              QStringLiteral("the memory line shows RAM, each card reported and the loaded models with their sizes"), mem ? mem->text() : QString());
        shot(&d, QStringLiteral("v0990b-options-ai-models"));
        if (thr && unl) {
            const int other = defThreads > 1 ? defThreads - 1 : qMin(2, QThread::idealThreadCount());
            thr->setValue(other);
            unl->setValue(25);
            d.apply();
            check(as.aiThreads == (other == defThreads ? 0 : other) && as.aiUnloadMinutes == 25 && mm.idleMinutes() == 25 && mm.aiThreads() == other,
                  QStringLiteral("a change is applied to the program and its AI functions"), QStringLiteral("%1 %2").arg(as.aiThreads).arg(as.aiUnloadMinutes));
            // "a restart of the settings": read them from the file again
            as.aiThreads = -1;
            as.aiUnloadMinutes = -1;
            as.load();
            OptionsDialog d2(&w, 0);
            auto *thr2 = d2.findChild<QSpinBox *>(QStringLiteral("optAiThreads"));
            auto *unl2 = d2.findChild<QSpinBox *>(QStringLiteral("optAiUnload"));
            check(thr2 && unl2 && thr2->value() == other && unl2->value() == 25,
                  QStringLiteral("the change is still there after the settings are read again"), thr2 ? QString::number(thr2->value()) : QString());
            if (auto *btn = d2.findChild<QPushButton *>(QStringLiteral("optAiThreadsDefault"))) {
                btn->click();
                d2.apply();
                check(as.aiThreads == 0 && mm.aiThreads() == defThreads, QStringLiteral("the Default button gives the default back (stored as \"follow the computer\")"));
            }
            d2.reject();
        }
        d.reject();
        as.aiThreads = 0;
        as.aiUnloadMinutes = 10;
        as.save();
        mm.reset();
    }

    std::printf("\n%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
