#include "TranslateTool.h"

#include "OptionsPage.h"
#include "Panel.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>

#ifdef Q_OS_WIN
// (the add-on's TP_VERSION, a definition of the build, is also the name of a type in Windows' winnt.h)
#pragma push_macro("TP_VERSION")
#undef TP_VERSION
#include <windows.h>
#pragma pop_macro("TP_VERSION")
#else
#include <dlfcn.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

using namespace qb::tools;

namespace tp {

const QString TranslateTool::kWindow = QStringLiteral("translation-proofreading.window");
const QString TranslateTool::kInspector = QStringLiteral("translation-proofreading.inspector");
const QString TranslateTool::kTab = QStringLiteral("translation-proofreading.panel");

namespace {

const char *kVersion = TP_VERSION;

// The add-on's own window: the panel side by side. It remembers its size and place.
class ToolWindow : public QWidget
{
public:
    ToolWindow(TranslateTool *tool, QWidget *parent)
        : QWidget(parent, Qt::Window)
        , m_tool(tool)
    {
        setObjectName(QStringLiteral("tpWindow"));
        setWindowTitle(tool->name());
        if (QFileInfo::exists(tool->iconPath()))
            setWindowIcon(QIcon(tool->iconPath()));
        m_panel = new Panel(tool, Panel::Place::Window, this);
        auto *l = new QVBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(m_panel);
        if (!restoreGeometry(tool->setting(QStringLiteral("windowGeometry")).toByteArray()))
            resize(1000, 560);
    }
    Panel *panel() const { return m_panel; }

protected:
    void closeEvent(QCloseEvent *e) override
    {
        m_tool->setSetting(QStringLiteral("windowGeometry"), saveGeometry());
        QWidget::closeEvent(e);
    }
    void hideEvent(QHideEvent *e) override
    {
        QWidget::hideEvent(e);
        QMetaObject::invokeMethod(m_tool, [t = m_tool] { t->updateCommandState(); }, Qt::QueuedConnection);
    }

private:
    TranslateTool *m_tool;
    Panel *m_panel;
};

} // namespace

TranslateTool::TranslateTool()
{
    // the models are freed after a while without work (1.4: 15 minutes at first, the writer's setting "unloadMinutes";
    // before 10 minutes), because they take memory
    m_unloadTimer.setSingleShot(true);
    connect(&m_unloadTimer, &QTimer::timeout, this, [this] {
        if (!m_engine)
            return;
        for (const QPointer<Panel> &p : m_panels)
            if (p && p->isBusy()) {
                m_unloadTimer.start();
                return;
            }
        bool any = false;
        for (const QString &slot : {QStringLiteral("corrector"), QStringLiteral("translator"), QStringLiteral("translator-en")})
            if (!m_engine->loaded(slot).isEmpty()) {
                m_engine->unload(slot);
                any = true;
            }
        // the program is to show this under the binder for 5 seconds; the tool interface 1.1 has no call for a message
        // (see DEVELOPING.md, 1.4), so far the signal only says it
        if (any)
            emit modelsUnloaded(unloadMessage(), 5000);
    });
    connect(this, &TranslateTool::settingsChangedSignal, this, &TranslateTool::updateCommandState);
}

TranslateTool::~TranslateTool() = default;

QString TranslateTool::version() const
{
    return QString::fromLatin1(kVersion);
}

QList<Command> TranslateTool::commands() const
{
    Command window;
    window.key = kWindow;
    window.text = tr("&Translate and Proofread...");
    window.place = QStringLiteral("edit/writing-tools");
    window.icon = QStringLiteral("applications-education-language");
    window.toolTip = tr("Proofread English text or translate German text into English, in a window of its own");
    Command inspector;
    inspector.key = kInspector;
    inspector.text = tr("Translate and Proofread in the &Inspector");
    inspector.place = QStringLiteral("edit/writing-tools");
    inspector.icon = QStringLiteral("applications-education-language");
    inspector.toolTip = tr("The same in the inspector, beside the editor");
    return {window, inspector};
}

void TranslateTool::setHost(ToolHost *host)
{
    m_host = host;
    if (!m_host)
        return;
    m_store.setDataDir(m_host->dataDir(id()));
    m_store.setPluginDir(addonDir() + QStringLiteral("/qtplugins"));
    connect(&m_store, &ModelStore::changed, this, [this] {
        for (const QPointer<Panel> &p : m_panels)
            if (p)
                p->reloadSettings();
    });
    if (QObject *ev = m_host->events())
        connect(ev, SIGNAL(settingsChanged()), this, SIGNAL(settingsChangedSignal()));
    // the add-on's own icon on the menu entries and the toolbar button (tool interface 1.1); the actions exist
    // once the main window is built
    QTimer::singleShot(0, this, &TranslateTool::updateCommandState);
}

void TranslateTool::updateCommandState()
{
    if (!m_host || m_host->apiVersion() < 110)
        return;
    const QString icon = QFileInfo::exists(iconPath()) ? iconPath() : QString();
    m_host->setCommandState(kWindow, m_window && m_window->isVisible(), icon, QString());
    m_host->setCommandState(kInspector, false, icon, QString());
}

QString TranslateTool::addonDir() const
{
    const QString env = qEnvironmentVariable("QB_TP_DIR");
    if (!env.isEmpty())
        return env;
    return m_host ? m_host->contentDirs(QStringLiteral("translation-proofreading")).value(0) : QString();
}

QString TranslateTool::iconPath() const
{
    const QString dir = addonDir();
    return dir.isEmpty() ? QStringLiteral("applications-education-language") : dir + QStringLiteral("/icons/translation-proofreading.svg");
}

void TranslateTool::run(const QString &commandKey)
{
    if (commandKey == kWindow) {
        openWindow();
    } else if (commandKey == kInspector && m_host && m_host->apiVersion() >= 110) {
        m_host->showInspectorTab(kTab);
        updateCommandState();
    } else if (commandKey == kInspector) {
        openWindow(); // a program without inspector tabs for tools
    }
}

void TranslateTool::openWindow()
{
    if (!m_window) {
        auto *w = new ToolWindow(this, m_host ? m_host->window() : nullptr);
        w->setAttribute(Qt::WA_DeleteOnClose, false);
        m_panels << w->panel();
        m_window = w;
    }
    m_window->show();
    m_window->raise();
    m_window->activateWindow();
    updateCommandState();
}

QString TranslateTool::optionsPage() const
{
    return m_host && m_host->apiVersion() >= 110 ? QStringLiteral("Section") : QStringLiteral("General");
}

OptionsSection *TranslateTool::createOptions(QWidget *parent)
{
    return m_host ? new OptionsPage(this, parent) : nullptr;
}

void TranslateTool::shutdown()
{
    if (m_window) {
        setSetting(QStringLiteral("windowGeometry"), m_window->saveGeometry());
        m_window->close();
    }
}

int TranslateTool::openWindows() const
{
    return m_window && m_window->isVisible() ? 1 : 0;
}

QList<InspectorTab> TranslateTool::inspectorTabs() const
{
    return {InspectorTab{kTab, iconPath(), tr("Translate and proofread")}};
}

QWidget *TranslateTool::createInspectorTab(const QString &key, QWidget *parent)
{
    if (key != kTab)
        return nullptr;
    auto *p = new Panel(const_cast<TranslateTool *>(this), Panel::Place::Inspector, parent);
    m_panels << p;
    return p;
}

QList<ToolbarButton> TranslateTool::toolbarButtons() const
{
    return {ToolbarButton{kWindow, QString()}};
}

QMenu *TranslateTool::commandMenu(const QString &commandKey)
{
    if (commandKey != kWindow)
        return nullptr;
    if (!m_menu) {
        m_menu = new QMenu;
        m_menu->setObjectName(QStringLiteral("tpMenu"));
        m_menu->addAction(tr("Open the Window"), this, [this] { run(kWindow); });
        m_menu->addAction(tr("Open in the Inspector"), this, [this] { run(kInspector); });
        m_menu->addSeparator();
        m_menu->addAction(tr("Models and Rules..."), this, &TranslateTool::showOptions);
    }
    return m_menu;
}

QVariant TranslateTool::setting(const QString &key, const QVariant &def) const
{
    return m_host ? m_host->setting(id(), key, def) : def;
}

void TranslateTool::setSetting(const QString &key, const QVariant &value)
{
    if (m_host)
        m_host->setSetting(id(), key, value);
    if (key == QLatin1String("corrector") || key == QLatin1String("translator") || key == QLatin1String("translator-en"))
        for (const QPointer<Panel> &p : m_panels)
            if (p)
                p->reloadSettings();
}

Rules TranslateTool::rules() const
{
    Rules r;
    r.noEmDash = setting(QStringLiteral("noEmDash"), true).toBool();
    r.american = setting(QStringLiteral("american"), true).toBool();
    r.curlyQuotes = setting(QStringLiteral("curlyQuotes"), true).toBool();
    r.plainEllipsis = setting(QStringLiteral("plainEllipsis"), true).toBool();
    r.replacements = parseReplacements(setting(QStringLiteral("replacements")).toString());
    return r;
}

int TranslateTool::threads() const
{
    const int ideal = qMax(1, QThread::idealThreadCount());
    return qBound(1, setting(QStringLiteral("threads"), qMax(1, ideal - 1)).toInt(), ideal);
}

QString TranslateTool::loadProblem(const QString &slot, const QString &modelId)
{
    const CatalogModel *m = m_store.model(modelId);
    if (!m)
        return QString();
    const QStringList allSlots{QStringLiteral("corrector"), QStringLiteral("translator"), QStringLiteral("translator-en")};
    if (m_engine)
        for (const QString &s : allSlots)
            if (m_engine->loaded(s) == modelId)
                return QString();   // loaded already (or shared with another slot)
    // the model in this slot is freed first, unless another slot keeps it
    qint64 freed = 0;
    if (m_engine) {
        const QString current = m_engine->loaded(slot);
        bool shared = false;
        for (const QString &s : allSlots)
            shared = shared || (s != slot && !current.isEmpty() && m_engine->loaded(s) == current);
        if (!current.isEmpty() && !shared)
            if (const CatalogModel *c = m_store.model(current))
                freed = c->memoryMB;
    }
    return memoryProblem(*m, Machine::now(), true, freed);
}

void TranslateTool::lowerPriority()
{
#ifdef Q_OS_WIN
    // Windows (2026-11-16): the thread below normal; the threads the engine and OpenMP start begin at normal priority
    // (Windows gives a new thread no priority of the one that starts it), so on Windows only one core left free keeps
    // the program responsive
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
#else
    // Linux: a thread is a task of its own; the threads it starts (the engine's and OpenMP's) take its priority
    setpriority(PRIO_PROCESS, id_t(syscall(SYS_gettid)), 10);
#endif
}

QString TranslateTool::chosenModel(const QString &kind) const
{
    const QString chosen = setting(kind).toString();
    if (!chosen.isEmpty() && m_store.isInstalled(chosen))
        return chosen;
    // nothing chosen (or the chosen one is gone): the first installed model of the kind
    for (const CatalogModel &m : m_store.models(kind))
        if (m_store.isInstalled(m.id))
            return m.id;
    return QString();
}

QString TranslateTool::wordModel(const QString &kind) const
{
    // the model for single words and short texts: the chosen one when it may translate them, else the first installed
    // one that may (never the large model, 1.3)
    const QString chosen = chosenModel(kind);
    if (const CatalogModel *c = m_store.model(chosen); c && c->forWords)
        return chosen;
    for (const CatalogModel &m : m_store.models(kind))
        if (m.forWords && m_store.isInstalled(m.id))
            return m.id;
    return QString();
}

QString TranslateTool::dictionaryFile() const
{
    const QString id = chosenModel(QStringLiteral("dictionary"));
    return id.isEmpty() ? QString() : m_store.filePath(id, QStringLiteral("de-dictionary.qz"));
}

EngineInterface *TranslateTool::engine(QString *error)
{
    if (m_engine)
        return m_engine;
    auto fail = [&](const QString &e) -> EngineInterface * {
        m_engineError = e;
        if (error)
            *error = e;
        return nullptr;
    };
    if (!m_engineError.isEmpty())
        return fail(m_engineError);
    const QString dir = addonDir();
#ifdef Q_OS_WIN
    // Windows (2026-11-16): qb-tp-engine.dll with MSVC's OpenMP (vcomp140.dll) beside it; the altered search path
    // finds the DLLs an engine needs in its own folder first
    const QString lib = dir + QStringLiteral("/qb-tp-engine.dll");
    if (dir.isEmpty() || !QFileInfo::exists(lib))
        return fail(tr("The add-on's engine is missing (install the add-on again)."));
    HMODULE h = LoadLibraryExW(reinterpret_cast<const wchar_t *>(QDir::toNativeSeparators(lib).utf16()), nullptr,
                               LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!h)
        return fail(tr("Cannot load the engine: %1").arg(QStringLiteral("Windows error %1").arg(GetLastError())));
    auto create = reinterpret_cast<TpEngineCreateFn>(reinterpret_cast<void *>(GetProcAddress(h, "qb_tp_engine_create")));
#else
    const QString lib = dir + QStringLiteral("/libqb-tp-engine.so");
    if (dir.isEmpty() || !QFileInfo::exists(lib))
        return fail(tr("The add-on's engine is missing (install the add-on again)."));
    void *h = dlopen(QFile::encodeName(lib).constData(), RTLD_NOW | RTLD_LOCAL);
    if (!h)
        return fail(tr("Cannot load the engine: %1").arg(QString::fromLocal8Bit(dlerror())));
    auto create = reinterpret_cast<TpEngineCreateFn>(dlsym(h, "qb_tp_engine_create"));
#endif
    if (!create)
        return fail(tr("The engine is damaged (no entry point)."));
    m_engine = create();
    return m_engine;
}

QString TranslateTool::unloadMessage()
{
    return tr("Model unloaded, RAM usage minimal");
}

int TranslateTool::unloadMinutes() const
{
    return qBound(0, setting(QStringLiteral("unloadMinutes"), 15).toInt(), 24 * 60);
}

int TranslateTool::unloadIntervalMs() const
{
#ifdef TP_TEST_SWITCHES
    if (qEnvironmentVariableIsSet("TP_TEST_UNLOAD_MS"))
        return qEnvironmentVariableIntValue("TP_TEST_UNLOAD_MS");
#endif
    return unloadMinutes() * 60 * 1000;
}

void TranslateTool::touchEngine()
{
    // 0 minutes: the models stay loaded
    if (unloadIntervalMs() > 0)
        m_unloadTimer.start(unloadIntervalMs());
    else
        m_unloadTimer.stop();
}

void TranslateTool::showOptions()
{
    if (m_host && m_host->apiVersion() >= 110)
        m_host->showOptions(id());
}

void TranslateTool::settingsApplied()
{
    if (m_unloadTimer.isActive())
        touchEngine();   // the new waiting time counts from now
    for (const QPointer<Panel> &p : m_panels)
        if (p)
            p->reloadSettings();
}

} // namespace tp
