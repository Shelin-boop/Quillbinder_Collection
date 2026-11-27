#pragma once
// The writing tool "Translation and Proofreading" (tool interface 1.1 of Quillbinder): a panel in the inspector
// (input above, controls between, output below) and the same panel in a window of its own (input left, output
// right), a menu entry and a toolbar button, and a section in File > Options where the writer chooses and downloads
// the models.
#include "EngineApi.h"
#include "Languages.h"
#include "Logic.h"
#include "ModelStore.h"
#include "ToolApi.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QTimer>

class QMenu;
class QTranslator;
class QWidget;

namespace tp {

class Panel;

class RegisteredEngine;

class TranslateTool : public QObject, public qb::tools::ToolInterface, public qb::tools::ToolExtension, public qb::tools::ToolModels
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QB_TOOL_IID)
    Q_INTERFACES(qb::tools::ToolInterface qb::tools::ToolExtension qb::tools::ToolModels)
public:
    TranslateTool();
    ~TranslateTool() override;

    // ToolInterface 1.0
    QString id() const override { return QStringLiteral("translation-proofreading"); }
    QString name() const override { return tr("Translation Helper"); }
    QString version() const override;
    QList<qb::tools::Command> commands() const override;
    void setHost(qb::tools::ToolHost *host) override;
    void run(const QString &commandKey) override;
    QString optionsPage() const override;
    qb::tools::OptionsSection *createOptions(QWidget *parent) override;
    void shutdown() override;
    int openWindows() const override;

    // ToolExtension 1.1
    QList<qb::tools::InspectorTab> inspectorTabs() const override;
    QWidget *createInspectorTab(const QString &key, QWidget *parent) override;
    QList<qb::tools::ToolbarButton> toolbarButtons() const override;
    QMenu *commandMenu(const QString &commandKey) override;
    // short: it stands under an icon in the row of File > Options (the full name is its tool tip)
    QString optionsTitle() const override { return tr("Translation Helper"); }
    QString optionsIcon() const override { return iconPath(); }

    // ToolModels 1.3 (program 0.99.0): the program asks to unload a model of the registry; false while a job uses it
    bool unloadModel(const QString &id) override;

    // ---- for the panel and the options
    // 1.6: the program's model registry and shared cores setting are used when the host says 1.3 or later; a program with
    // 1.2 keeps the add-on's own threads and its own unload timer, as 1.5 did
    bool hostHasRegistry() const { return m_host && m_host->apiVersion() >= 130; }
    static QString registryId(const QString &modelId) { return QStringLiteral("translation-proofreading:") + modelId; }
    qb::tools::ToolHost *host() const { return m_host; }
    ModelStore *store() { return &m_store; }
    QVariant setting(const QString &key, const QVariant &def = QVariant()) const;
    void setSetting(const QString &key, const QVariant &value);
    Rules rules() const;
    int threads() const;           // 1.1: one core is left free by default; 1.6 with a 1.3 host: the shared setting (aiThreads)
    // 1.1: why the model cannot be loaded into the slot now (too little free memory), or empty
    QString loadProblem(const QString &slot, const QString &modelId);
    static void lowerPriority();   // the calling thread (and the threads it starts) at a low priority
    QString chosenModel(const QString &kind) const;   // the chosen model of the kind, if it is installed
    // 1.3: the model that translates single words and short texts (never the large one); empty: none is installed
    QString wordModel(const QString &kind) const;
    QString dictionaryFile() const;   // the downloaded German dictionary, or empty
    // ---- 1.5: the Translation Helper's eleven languages
    // The language of the program (QB_LANGUAGE, else its setting "app/language", else the system's): the language of the
    // add-on's own texts; English when it has no translation.
    QString language() const { return m_language; }
    // The installed model that makes a step of a translation between two languages (German to English and English to
    // German: the chosen one, a chat model not counted), or empty.
    QString stepModel(const Step &step) const;
    // The dictionary file of a language ("de", "es" ...), or empty when it is not downloaded (English has none: WordNet)
    QString dictionaryFile(const QString &lang) const;
    // The catalog model of a step (installed or not), or null
    const CatalogModel *catalogModelFor(const Step &step) const;
    // Slots of the engine a translation between two languages uses ("pair-1", "pair-2")
    static QString pairSlot(int index) { return QStringLiteral("pair-") + QString::number(index + 1); }
#ifdef TP_TEST_SWITCHES
    void setEngineForTest(EngineInterface *engine) { m_engine = engine; }   // a stand-in for the checks
#endif
    // The engine (loaded on first use); null with *error when it cannot be loaded.
    EngineInterface *engine(QString *error);
    void touchEngine();            // after work: the models are freed after a while without work (1.4: 15 minutes, a setting;
                                   // 1.6 with a 1.3 host: the program's rule, this does nothing)
    int unloadMinutes() const;     // 1.4: 0 = never
    int unloadIntervalMs() const;
    static QString unloadMessage();   // 1.4: "Model unloaded, RAM usage minimal"
    bool unloadTimerActive() const { return m_unloadTimer.isActive(); }
    QString addonDir() const;      // the add-on's folder "translation-proofreading"
    QString iconPath() const;
    void showOptions();
    void settingsApplied();        // File > Options was applied: panels take the new settings

    void updateCommandState();     // the add-on's icon and the window's on/off state on its menu entries

    static const QString kWindow, kInspector, kTab;

signals:
    void settingsChangedSignal();
    // 1.4: a loaded model was freed after the waiting time: the text and how long the program should show it (ms)
    void modelsUnloaded(const QString &message, int milliseconds);

private:
    void openWindow();
    void installTranslations();
    EngineInterface *wrapped();

    qb::tools::ToolHost *m_host = nullptr;
    ModelStore m_store;
    EngineInterface *m_engine = nullptr;           // the engine itself
    RegisteredEngine *m_registered = nullptr;      // 1.6: with a 1.3 host, what engine() gives: it reports to the registry
    QString m_engineError;
    QTimer m_unloadTimer;
    QList<QPointer<Panel>> m_panels;
    QPointer<QWidget> m_window;
    QPointer<QMenu> m_menu;
    QString m_language = QStringLiteral("en");
    QTranslator *m_translator = nullptr;
};

} // namespace tp
