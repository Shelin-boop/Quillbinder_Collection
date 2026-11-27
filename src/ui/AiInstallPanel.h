#pragma once
// 0.99.0: the part of a settings page that installs an AI engine and its models (Read Aloud, Handwriting; Speech Transcription
// gets it through tool interface 1.3, ToolHost::aiPanel).
//
//   Engine: <state>                                    [Install the Engine] [Install from File...]
//   +--------------------------------------------------------------------------+
//   | Model            | Languages | Download | Memory | License | State        |
//   +--------------------------------------------------------------------------+
//   <summary, credit and license of the selected model, or why it cannot be downloaded here>
//   [Download] [Delete]                                 [=====progress=====] [Cancel]
//
// The network is used only by the two buttons that download (ui/AiDownloads.h). The panel lists the catalog's models of the
// kinds it was made for ("tts", "asr", "handwriting").
#include <QStringList>
#include <QWidget>

class QLabel;
class QProgressBar;
class QPushButton;
class QTreeWidget;

namespace qb {

class AiInstallPanel : public QWidget
{
    Q_OBJECT
public:
    // engineId: "ai-engine" or "handwriting-engine"; kinds: the catalog's kinds to list
    AiInstallPanel(const QString &engineId, const QStringList &kinds, QWidget *parent = nullptr);

    QString engineId() const { return m_engine; }
    QStringList kinds() const { return m_kinds; }
    QString selectedModel() const;
    void selectModel(const QString &id);

    // the buttons' work, for the tests (and the buttons)
    void installEngine();
    // from a file: asks before an unsigned package (as the add-on window does)
    bool installEngineFromFile(const QString &file);
    void downloadSelected();
    void deleteSelected();

public slots:
    void refresh();

signals:
    void changed();   // the engine or a model was installed or removed

private:
    void chooseFile();
    void showDetail();
    void onProgress(const QString &what, qint64 done, qint64 total);
    void onFinished(const QString &what, bool ok, const QString &message);
    void setBusy(bool on, const QString &what = QString());

    QString m_engine;
    QStringList m_kinds;
    QLabel *m_engineState = nullptr;
    QPushButton *m_installEngine = nullptr, *m_fromFile = nullptr;
    QTreeWidget *m_models = nullptr;
    QLabel *m_detail = nullptr;
    QPushButton *m_download = nullptr, *m_delete = nullptr, *m_cancel = nullptr;
    QProgressBar *m_progress = nullptr;
    QLabel *m_message = nullptr;
};

} // namespace qb
