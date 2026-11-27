#pragma once
// A stand-in for Quillbinder in the checks: it implements the tool interface's ToolHost (1.2) with a main window
// that holds one text editor, settings in memory, and records of what the tool asked for.
#include "ToolApi.h"

#include <QDir>
#include <QElapsedTimer>
#include <QHash>
#include <QMainWindow>
#include <QMutex>
#include <QMutexLocker>
#include <QPair>
#include <QPlainTextEdit>
#include <QVariant>

class FakeEvents : public QObject
{
    Q_OBJECT
signals:
    void projectChanged();
    void settingsChanged();
};

class FakeHost : public qb::tools::ToolHost
{
public:
    FakeHost(const QString &addonDir, const QString &dataRoot)
        : m_addonDir(addonDir)
        , m_dataRoot(dataRoot)
    {
        main = new QMainWindow;
        main->setObjectName(QStringLiteral("fakeMainWindow"));
        editor = new QPlainTextEdit(main);
        main->setCentralWidget(editor);
        main->resize(900, 600);
    }
    ~FakeHost() override { delete main; }

    QMainWindow *main;
    QPlainTextEdit *editor;
    mutable FakeEvents events_;
    QHash<QString, QVariant> settings;
    QStringList shownTabs, shownOptions;
    QHash<QString, QPair<bool, QString>> states;   // command -> (checked, icon)

    int version = 120;                       // what apiVersion() says (a test sets 110 for a program with interface 1.1)
    QList<QPair<QString, int>> messages;     // showMessage() calls
    int apiVersion() const override { return version; }
    QString appName() const override { return QStringLiteral("Quillbinder"); }
    QString appVersion() const override { return QStringLiteral("0.91.1"); }
    QWidget *window() const override { return main; }
    QStringList contentDirs(const QString &kind) const override
    {
        return kind == QLatin1String("translation-proofreading") ? QStringList{m_addonDir} : QStringList();
    }
    QString dataDir(const QString &toolId) const override
    {
        const QString d = m_dataRoot + QStringLiteral("/tools/") + toolId;
        QDir().mkpath(d);
        return d;
    }
    QVariant setting(const QString &toolId, const QString &key, const QVariant &def) const override
    {
        return settings.value(toolId + QLatin1Char('/') + key, def);
    }
    void setSetting(const QString &toolId, const QString &key, const QVariant &value) override
    {
        settings.insert(toolId + QLatin1Char('/') + key, value);
    }
    bool hasProject() const override { return true; }
    QString projectTitle() const override { return QStringLiteral("Test project"); }
    bool canInsertText() const override { return true; }
    bool insertText(const QString &text) override
    {
        editor->textCursor().insertText(text);
        return true;
    }
    QString selectedText() const override
    {
        return editor->textCursor().selectedText().replace(QChar(0x2029), QLatin1Char('\n'));
    }
    QList<qb::tools::TextDocument> documents(qb::tools::Scope) const override { return {}; }
    int countWords(const QString &text) const override { return text.split(QLatin1Char(' '), Qt::SkipEmptyParts).size(); }
    void showDocument(const QString &) override {}
    void searchProject(const QString &) override {}
    QList<qb::tools::ProjectInfo> openProjects() override { return {}; }
    bool sendToProject(const QString &, const qb::tools::SendRequest &, QString *) override { return false; }
    QString commandLine(const QString &) const override { return QString(); }
    QObject *events() const override { return &events_; }
    void showInspectorTab(const QString &key) override { shownTabs << key; }
    void setCommandState(const QString &commandKey, bool checked, const QString &icon, const QString &) override
    {
        states.insert(commandKey, {checked, icon});
    }
    void showOptions(const QString &toolId) override { shownOptions << toolId; }
    void showMessage(const QString &text, int milliseconds) override { messages.append({text, milliseconds}); }

    // ---- 1.3: a small model registry of the program (set `version` to 130 to use it)
    int cores = 3;                           // what aiThreads() says
    int aiThreads() const override { return cores; }
    int changeCount = 0, changeOn = 0, changeOff = 0;   // modelMessage() calls (on and off counted)
    void modelMessage(bool on) override
    {
        QMutexLocker l(&m_regMutex);
        if (on) {
            ++changeOn;
            ++changeCount;
        } else {
            ++changeOff;
            --changeCount;
        }
    }
    void keepRunning(const QString &, bool) override {}
    void lowerThreadPriority() override {}
    struct Registered {
        QString name, where;
        qint64 bytes = 0;
        int used = 0;
        qint64 lastUsedMs = 0;
    };
    QHash<QString, Registered> registry;     // loaded models by id
    QStringList requests;                    // ids asked for, in order
    QString refuseReason;                    // not empty: requestModelLoad refuses with it
    qb::tools::ModelDecision requestModelLoad(const QString &id, const QString &displayName, qint64 bytes, const QString &where) override
    {
        QMutexLocker l(&m_regMutex);
        requests << id;
        qb::tools::ModelDecision d;
        if (!refuseReason.isEmpty()) {
            d.allowed = false;
            d.reason = refuseReason;
            return d;
        }
        Registered r;
        r.name = displayName;
        r.bytes = bytes;
        r.where = where;
        pending.insert(id, r);
        return d;
    }
    void modelLoaded(const QString &id) override
    {
        QMutexLocker l(&m_regMutex);
        Registered r = pending.take(id);
        r.lastUsedMs = clock.elapsed();
        registry.insert(id, r);
    }
    void modelUsed(const QString &id) override
    {
        QMutexLocker l(&m_regMutex);
        if (registry.contains(id)) {
            ++registry[id].used;
            registry[id].lastUsedMs = clock.elapsed();
        }
    }
    void modelUnloaded(const QString &id) override
    {
        QMutexLocker l(&m_regMutex);
        registry.remove(id);
        pending.remove(id);
    }
    void reportDeviceMemory(const QString &, qint64, qint64) override {}
    // The program's rule in a check: asks the tool to unload a model, as the program's idle timer does, and takes it off its
    // list when the tool says true.
    QElapsedTimer clock = [] { QElapsedTimer t; t.start(); return t; }();
    // The program's idle rule with a short time: every model unused for `idleMs` is unloaded; returns the ids.
    QStringList unloadIdle(qb::tools::ToolModels *tool, qint64 idleMs)
    {
        QStringList ids;
        const QStringList all = registry.keys();
        for (const QString &id : all)
            if (clock.elapsed() - registry.value(id).lastUsedMs >= idleMs && unloadByProgram(tool, id))
                ids << id;
        return ids;
    }
    bool unloadByProgram(qb::tools::ToolModels *tool, const QString &id)
    {
        const bool ok = tool->unloadModel(id);
        if (ok)
            modelUnloaded(id);
        return ok;
    }

private:
    QString m_addonDir, m_dataRoot;
    mutable QMutex m_regMutex;
    QHash<QString, Registered> pending;
};
