// A writing tool built against the tool interface 1.2 (ToolApi12.h, as 0.94.0 had it), for the 0.99.0 tests: a program with
// interface 1.3 must still load it, give it its host and run it; it calls showMessage and never the functions of 1.3.
#include "ToolApi12.h"

#include <QObject>
#include <QVariant>

class Tool12 : public QObject, public qb::tools::ToolInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QB_TOOL_IID)
    Q_INTERFACES(qb::tools::ToolInterface)
public:
    QString id() const override { return QStringLiteral("tool-12"); }
    QString name() const override { return QStringLiteral("Tool 1.2"); }
    QString version() const override { return QStringLiteral("1.2"); }
    QList<qb::tools::Command> commands() const override
    {
        qb::tools::Command c;
        c.key = QStringLiteral("tool-12.run");
        c.text = QStringLiteral("Tool 1.2 Runs");
        c.place = QStringLiteral("edit/writing-tools");
        return {c};
    }
    void setHost(qb::tools::ToolHost *host) override { m_host = host; }
    void run(const QString &key) override
    {
        if (key != QLatin1String("tool-12.run") || !m_host)
            return;
        m_host->setSetting(id(), QStringLiteral("apiVersion"), m_host->apiVersion());
        if (m_host->apiVersion() >= 120)
            m_host->showMessage(QStringLiteral("Tool 1.2 says hello"), 3000);
    }
    QString optionsPage() const override { return QString(); }
    qb::tools::OptionsSection *createOptions(QWidget *) override { return nullptr; }
    void shutdown() override {}
    int openWindows() const override { return 0; }

private:
    qb::tools::ToolHost *m_host = nullptr;
};

#include "Tool12.moc"
