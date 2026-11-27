#pragma once
// Interface between the program and writing tools (version 1.3; 1.0 since 0.11, 1.1 since 0.61, 1.2 since 0.94.0,
// 1.3 since 0.99.0).
//
// A writing tool is a Qt plugin (a MODULE library, *.so) that implements
// qb::tools::ToolInterface. It is delivered as an add-on: the library goes
// into the add-on's plugins/ folder, its data into folders of its own
// (names/, ...), which the program hands out through ToolHost::contentDirs.
//
// A tool needs nothing from the program but this header and Qt (Core, Gui,
// Widgets). Everything it wants from the program goes through ToolHost, a
// class of pure virtual functions the program implements; nothing here has
// to be linked against the program. Plugins built for another interface
// version (QB_TOOL_IID) are not loaded.
//
// Every command of a tool becomes an action of the main window with the key
// "tool:<Command::key>": a menu entry at Command::place, a command in
// File > Options > Keyboard (shortcut can be changed there), a possible
// toolbar button. Commands marked `standalone` also run without a main
// window: `<program> --tool <key>` (the scratch pad uses this for a
// shortcut that works in every program).
//
// Version 1.1 (0.61) adds to 1.0 without changing it, so tools built for 1.0
// keep working and a 1.1 tool still loads in a 1.0 program (both use the IID
// QB_TOOL_IID):
//  - ToolHost has three more functions at its end (showInspectorTab,
//    setCommandState, showOptions). A tool calls them only when
//    host->apiVersion() >= 110: an older program does not have them.
//  - What a tool gives the program in 1.1 is a second interface,
//    ToolExtension (its own IID), which the program asks the plugin object
//    for: tabs in the inspector, buttons of the main toolbar with a menu
//    beside them, a section of its own in File > Options.
// Version 1.2 (0.94.0) adds one more function at the end of ToolHost (showMessage: a short message in the line under
// the binder). A tool calls it only when host->apiVersion() >= 120.
// Version 1.3 (0.99.0) adds functions at the end of ToolHost (the shared setting of the AI's cores, the message "Model
// change in progress", keepRunning, and the program's list of loaded AI models with the rule for their memory) and a
// third interface, ToolModels (its own IID), through which the program asks a tool to unload one of its models. A tool
// calls the new functions only when host->apiVersion() >= 130; a tool without ToolModels is never asked.
// 1.3 also gives tools the program's AI engine (the package "ai-engine", its C interface tools/QbAi.h): aiEngine() returns
// its table, aiPickDevice() applies the one rule of the device choice, aiPanel() is the program's panel that installs the
// engine and the catalog's models, aiModelFile() finds an installed model. A tool never loads libqbai itself: the program
// loads it once for itself and every tool (two copies of its libraries in one process do not work).
// Nothing of 1.0 may change: the program reads the structs and calls the
// functions of tools built years earlier.

#include <QJsonArray>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtPlugin>

class QMenu;
class QObject;
class QWidget;
struct qbai_api;   // tools/QbAi.h (1.3): include it to call the engine

namespace qb {
namespace tools {

struct Command {
    QString key;        // unique: "<tool id>.<name>", e.g. "name-generator.show"
    QString text;       // menu text with "&" and "…"
    // Where the entry goes: "edit/writing-tools" (Edit > Writing Tools),
    // "window" (Window menu), "project" (Project menu, after Project Statistics).
    QString place;
    QString shortcut;   // standard shortcut as portable text ("Alt+Shift+Return"), may be empty
    QString icon;       // icon theme name, may be empty
    QString toolTip;
    bool needsProject = false;  // disabled while no project is open
    bool standalone = false;    // can run without a main window (--tool <key>)
};

// Which text a tool asks for (ToolHost::documents).
enum class Scope {
    Selection,        // selected text of the active editor
    Document,         // the text documents shown in the active editor (combined text: all of them)
    BinderSelection,  // the documents selected in the binder and the documents inside them
    Draft,            // text documents in the Draft folder marked "Include in Compile"
    Project           // every text document outside the Trash
};

struct TextDocument {
    QString id;         // binder id; empty for a selection
    QString title;
    QString path;       // enclosing folders: "Draft / Part One"
    QString text;       // plain text (inline annotations left out, as for the word count)
    bool inDraft = false;
};

// A running instance of the program (this one included) with its project.
struct ProjectInfo {
    QString key;        // what sendToProject wants
    QString title;      // project title
    QString file;       // project file
    bool thisWindow = false;
    // The binder without the Trash:
    // [ { "id", "title", "folder": bool, "text": bool, "children": [ ... ] } ]
    QJsonArray binder;
};

struct SendRequest {
    QString file;       // file to bring in: .txt .md .rtf .html .docx .odt, pictures, PDF ...
    QString title;      // title of the new document (mode "import")
    QString mode;       // "import": new document inside targetId (empty: the Research folder)
                        // "append": added to the end of the text document targetId
    QString targetId;
};

// What the host answers to ToolHost::requestModelLoad (1.3).
struct ModelDecision {
    bool allowed = true;
    QString reason;     // when refused: why, as shown to the writer in the message line
};

class ToolHost
{
public:
    virtual ~ToolHost() = default;
    virtual int apiVersion() const = 0;                 // 100 = 1.0
    virtual QString appName() const = 0;
    virtual QString appVersion() const = 0;
    // The main window, the parent for the tool's windows; null while the
    // program runs a standalone command (--tool) without a main window.
    virtual QWidget *window() const = 0;
    // Folders called `kind` ("names", ...) of all enabled add-ons.
    virtual QStringList contentDirs(const QString &kind) const = 0;
    // The tool's own folder ~/.local/share/<App>/tools/<toolId> (created).
    virtual QString dataDir(const QString &toolId) const = 0;
    // The program's settings file, group "tools/<toolId>".
    virtual QVariant setting(const QString &toolId, const QString &key, const QVariant &def = QVariant()) const = 0;
    virtual void setSetting(const QString &toolId, const QString &key, const QVariant &value) = 0;

    // ---- the project of this window
    virtual bool hasProject() const = 0;
    virtual QString projectTitle() const = 0;
    // Active text editor: text at the cursor (replaces the selection; one undo step).
    virtual bool canInsertText() const = 0;
    virtual bool insertText(const QString &text) = 0;
    virtual QString selectedText() const = 0;
    // Texts for statistics and the like; reads documents that are not open from their files.
    virtual QList<TextDocument> documents(Scope scope) const = 0;
    // Words counted by the program's own rule (the counts in footer bar and targets).
    virtual int countWords(const QString &text) const = 0;
    virtual void showDocument(const QString &id) = 0;    // in the active editor
    virtual void searchProject(const QString &text) = 0; // project search for a word or phrase

    // ---- every running instance
    virtual QList<ProjectInfo> openProjects() = 0;
    virtual bool sendToProject(const QString &key, const SendRequest &request, QString *error) = 0;
    // Command line that runs a standalone command (for a desktop-wide shortcut).
    virtual QString commandLine(const QString &commandKey) const = 0;

    // An object with the signals (connect with SIGNAL()):
    //   projectChanged()    a project was opened or closed in this window
    //   settingsChanged()   File > Options was applied
    virtual QObject *events() const = 0;

    // ---- 1.1 (program 0.61): only when apiVersion() >= 110
    // Shows the inspector of the main window with the tool's tab (InspectorTab::key).
    virtual void showInspectorTab(const QString &key) = 0;
    // The state of a command's menu entry and toolbar button: checked (on / off), its
    // icon (theme name or file path; empty: the command's own) and its tool tip (empty:
    // the command's own).
    virtual void setCommandState(const QString &commandKey, bool checked, const QString &icon, const QString &toolTip) = 0;
    // File > Options, at the tool's section (ToolInterface::optionsPage() "Section").
    virtual void showOptions(const QString &toolId) = 0;

    // ---- 1.2 (program 0.94.0): only when apiVersion() >= 120
    // A short message in the line under the binder: one at a time, each for `milliseconds` (<= 0: 5 seconds) or until it
    // is clicked; other messages wait until it is gone. No window, no sound.
    virtual void showMessage(const QString &text, int milliseconds) = 0;

    // ---- 1.3 (program 0.99.0): only when apiVersion() >= 130
    // The AI's CPU cores: one setting (File > Options > General > AI Models) for every AI function. Default: 75 % of the
    // cores rounded down, with 6 cores or fewer the cores minus 1, at least 1. A tool uses it for its engine's threads.
    virtual int aiThreads() const = 0;
    // The message line under the binder says "Model change in progress" while `on` (true before an unload or a load of a
    // model starts, false when the new model is ready, also at a first load). Calls are counted: every true needs a false.
    virtual void modelMessage(bool on) = 0;
    // A long job runs and must go on while the program's window is out of sight (`reason`: a short text for the
    // notification, the same text for on and off). Android starts and stops its foreground service; Linux and Windows do
    // nothing.
    virtual void keepRunning(const QString &reason, bool on) = 0;
    // Lowers the priority of the calling thread: the AI's worker threads call it first thing.
    virtual void lowerThreadPriority() = 0;

    // The model registry. The program keeps the list of loaded models (a tool cannot unload another tool's model) and applies
    // the one rule to all of them: a model unused for the set time (default 10 minutes) is unloaded; before a load, when
    // the memory it goes into is more than 50 % used (the whole computer's use), the model unused longest is unloaded
    // first; when the new model would still bring that memory over 80 %, it is not loaded.
    // `id` is unique, "<tool id>:<name>" ("speech-transcription:small"): the part before the colon tells the program
    // which tool to ask to unload it (ToolModels). `where`: "ram" or "gpu:<name>". `bytes`: what the model takes there.
    // Ask before every load. When the answer is `allowed`, load the model and then call modelLoaded(id); when the load
    // fails, call modelUnloaded(id). When refused, the program has shown `reason` in the message line; offer the next
    // smaller model or the CPU. The program may call ToolModels::unloadModel while this runs (in the tool's thread).
    virtual ModelDecision requestModelLoad(const QString &id, const QString &displayName, qint64 bytes, const QString &where) = 0;
    virtual void modelLoaded(const QString &id) = 0;
    // The model is used now (its idle time starts again): call it at each use, a long job at each step.
    virtual void modelUsed(const QString &id) = 0;
    // The tool unloaded the model itself (or its load failed).
    virtual void modelUnloaded(const QString &id) = 0;
    // The figures of a device's memory, as the engine's driver reports its budget: "gpu:<name>", total and available
    // bytes. Report them before requestModelLoad and whenever they change. (The program reads the RAM itself.)
    virtual void reportDeviceMemory(const QString &where, qint64 total, qint64 available) = 0;

    // The AI engine (also 1.3). Its table: what qbai_get_api(version) of the installed package "ai-engine" returns (tools/QbAi.h,
    // QBAI_API_VERSION); null when no engine is installed, it could not be loaded, or it is older than `version`. The first call
    // loads it (do that when the writer uses the function, not at the tool's start: the AI is loaded only when used). Before a
    // model loads, ask requestModelLoad as above; the engine's devices are those of qbai_api::device_get.
    virtual const qbai_api *aiEngine(int version) = 0;
    // The device for a model of `bytes` under the writer's choice ("auto", "cpu" or "gpu:<name>", as the tool keeps it):
    // Automatic takes the graphics card whose free memory (its driver's budget) holds the model, else the CPU. Returns the
    // device's index for qbai_load_params::device (0 = the CPU) and in *where its memory for requestModelLoad ("ram" or
    // "gpu:<name>"); the cards' figures are reported to the registry. It loads the engine as aiEngine() does; -1 when there is
    // no engine.
    virtual int aiPickDevice(const QString &choice, qint64 bytes, QString *where) = 0;
    // The entries of a device setting: the value and its text, ("auto", "Automatic"), ("cpu", "Processor (CPU)"), then the
    // cards of a loaded engine ("gpu:<name>", "<name>").
    virtual QList<QPair<QString, QString>> aiDeviceChoices() = 0;
    // The program's panel for a tool's options: the engine's state with "Install the Engine" and "Install from File...", and
    // the models of the program's catalog of the given kinds ("asr", "tts") with download, progress, check and removal. Owned
    // by `parent` (main thread only). Its signal changed() (connect with SIGNAL()) says that the engine or a model was
    // installed or removed.
    virtual QWidget *aiPanel(const QStringList &kinds, QWidget *parent) = 0;
    // The file of an installed model of the catalog ("" when it is not installed): `role` "model" (the file the engine loads)
    // or "extra" (its mmproj or tokenizer).
    virtual QString aiModelFile(const QString &modelId, const QString &role) const = 0;
};

// A section a tool adds to File > Options (ToolInterface::optionsPage).
class OptionsSection
{
public:
    virtual ~OptionsSection() = default;
    virtual QWidget *widget() = 0;   // shown in a group box; owned by the options window
    virtual void apply() = 0;        // OK or Apply
};

class ToolInterface
{
public:
    virtual ~ToolInterface() = default;
    virtual QString id() const = 0;            // "name-generator"
    virtual QString name() const = 0;          // "Name Generator"
    virtual QString version() const = 0;
    virtual QList<Command> commands() const = 0;
    virtual void setHost(ToolHost *host) = 0;  // once, before any command runs
    virtual void run(const QString &commandKey) = 0;
    // Options: "" = none, "General" = a section on the General page,
    // "Section" (1.1) = a section of its own in the icon row of File > Options
    // (ToolExtension::optionsTitle / optionsIcon; a 1.0 program shows none).
    virtual QString optionsPage() const = 0;
    virtual OptionsSection *createOptions(QWidget *parent) = 0;  // null if none
    // The program ends (or the main window closes): save open work.
    virtual void shutdown() = 0;
    // Number of top-level windows of the tool that are open (a standalone
    // run ends when the last one closes).
    virtual int openWindows() const = 0;
};

// ---- 1.1 (0.61)

// A tab of the tool in the inspector; its button sits at the left end of the
// row of tab buttons.
struct InspectorTab {
    QString key;        // unique: "<tool id>.<name>", e.g. "speech-transcription.list"
    QString icon;       // icon theme name or file path
    QString toolTip;
};

// A button of the main toolbar for one of the tool's commands. It is part of
// the toolbar's standard set (before the item `before`, an item id of that set
// such as "showInspector"; empty: at the end); a toolbar the writer arranged
// gets it once.
struct ToolbarButton {
    QString commandKey;
    QString before;
};

class ToolExtension
{
public:
    virtual ~ToolExtension() = default;
    virtual QList<InspectorTab> inspectorTabs() const = 0;
    // The page of a tab; the inspector owns it. Asked once per main window.
    virtual QWidget *createInspectorTab(const QString &key, QWidget *parent) = 0;
    virtual QList<ToolbarButton> toolbarButtons() const = 0;
    // The menu opened by the arrow beside the command's toolbar button (the tool
    // owns it and may fill it when it opens); null: a button without an arrow.
    virtual QMenu *commandMenu(const QString &commandKey) = 0;
    // The section of File > Options when optionsPage() is "Section".
    virtual QString optionsTitle() const = 0;
    virtual QString optionsIcon() const = 0;   // icon theme name or file path
};

// ---- 1.3 (0.99.0)

// Implemented by a tool that loads AI models (next to ToolInterface; declared with Q_INTERFACES). The program calls it
// when the rule asks for a model to go (idle too long, memory short, the system's low-memory signal).
class ToolModels
{
public:
    virtual ~ToolModels() = default;
    // Unloads the model `id` (as given to requestModelLoad) and frees its memory; true when it is unloaded (also when it
    // was not loaded). false: it is in use now and stays. The tool does not call modelUnloaded for this: the program
    // takes the model off its list when the answer is true.
    virtual bool unloadModel(const QString &id) = 0;
};

} // namespace tools
} // namespace qb

#define QB_TOOL_IID "org.quillbinder.ToolInterface/1.0"
Q_DECLARE_INTERFACE(qb::tools::ToolInterface, QB_TOOL_IID)
#define QB_TOOL_EXTENSION_IID "org.quillbinder.ToolExtension/1.1"
Q_DECLARE_INTERFACE(qb::tools::ToolExtension, QB_TOOL_EXTENSION_IID)
#define QB_TOOL_MODELS_IID "org.quillbinder.ToolModels/1.3"
Q_DECLARE_INTERFACE(qb::tools::ToolModels, QB_TOOL_MODELS_IID)
