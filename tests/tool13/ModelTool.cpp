// A writing tool of the tool interface 1.3 for the 0.99.0 tests: it asks the host before loading a model, reports it, is
// asked to unload it (ToolModels), and uses the shared setting of the AI's cores. The tests drive it through its slots.
#include "tools/ToolApi.h"

#include <QObject>
#include <QSet>
#include <QStringList>

class ModelTool : public QObject, public qb::tools::ToolInterface, public qb::tools::ToolModels
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QB_TOOL_IID)
    Q_INTERFACES(qb::tools::ToolInterface qb::tools::ToolModels)
public:
    QString id() const override { return QStringLiteral("model-test"); }
    QString name() const override { return QStringLiteral("Model Test"); }
    QString version() const override { return QStringLiteral("1.3"); }
    QList<qb::tools::Command> commands() const override { return {}; }
    void setHost(qb::tools::ToolHost *host) override { m_host = host; }
    void run(const QString &) override {}
    QString optionsPage() const override { return QString(); }
    qb::tools::OptionsSection *createOptions(QWidget *) override { return nullptr; }
    void shutdown() override {}
    int openWindows() const override { return 0; }

    bool unloadModel(const QString &id) override
    {
        if (m_busy)
            return false;
        m_loaded.remove(id);
        m_unloadLog << id;
        return true;
    }

public slots:
    // asks, loads when allowed (the message line brackets the change), reports; returns whether it was allowed
    bool load(const QString &id, const QString &name, qint64 bytes, const QString &where)
    {
        if (!m_host || m_host->apiVersion() < 130)
            return false;
        m_host->modelMessage(true);
        const qb::tools::ModelDecision d = m_host->requestModelLoad(id, name, bytes, where);
        m_reason = d.reason;
        if (d.allowed) {
            m_loaded.insert(id);
            m_host->modelLoaded(id);
        }
        m_host->modelMessage(false);
        return d.allowed;
    }
    void use(const QString &id) { if (m_host) m_host->modelUsed(id); }
    void dropOwn(const QString &id) { m_loaded.remove(id); if (m_host) m_host->modelUnloaded(id); }
    void report(const QString &where, qint64 total, qint64 available) { if (m_host) m_host->reportDeviceMemory(where, total, available); }
    void setBusy(bool b) { m_busy = b; }
    int threads() const { return m_host ? m_host->aiThreads() : 0; }
    void keep(bool on) { if (m_host) m_host->keepRunning(QStringLiteral("model test job"), on); }
    QString reason() const { return m_reason; }
    QStringList unloadLog() const { return m_unloadLog; }
    bool isLoaded(const QString &id) const { return m_loaded.contains(id); }

private:
    qb::tools::ToolHost *m_host = nullptr;
    QSet<QString> m_loaded;
    QStringList m_unloadLog;
    QString m_reason;
    bool m_busy = false;
};

#include "ModelTool.moc"
