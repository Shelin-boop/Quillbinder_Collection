#include "ui/AiInstallPanel.h"

#include "core/AiCatalog.h"
#include "core/AiEngine.h"
#include "core/ReadAloud.h"
#include "ui/AiDownloads.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace qb {

namespace {
QString megabytes(qint64 bytes)
{
    if (bytes >= 1024LL * 1024 * 1024)
        return QStringLiteral("%1 GB").arg(QLocale().toString(double(bytes) / (1024.0 * 1024 * 1024), 'f', 1));
    return QStringLiteral("%1 MB").arg(QLocale().toString(qMax<qint64>(1, (bytes + 512 * 1024) / (1024 * 1024))));
}

QString languagesText(const QStringList &codes)
{
    if (codes.contains(QStringLiteral("*")))
        return AiInstallPanel::tr("many");
    QStringList names;
    for (const QString &c : codes) {
        const QLocale l(c);
        const QString n = l.language() == QLocale::C ? c : QLocale::languageToString(l.language());
        names << n;
    }
    return names.join(QLatin1String(", "));
}
} // namespace

AiInstallPanel::AiInstallPanel(const QString &engineId, const QStringList &kinds, QWidget *parent)
    : QWidget(parent), m_engine(engineId), m_kinds(kinds)
{
    setObjectName(QStringLiteral("aiInstallPanel"));
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto *engineRow = new QHBoxLayout;
    m_engineState = new QLabel(this);
    m_engineState->setObjectName(QStringLiteral("aiEngineState"));
    m_engineState->setWordWrap(true);
    m_installEngine = new QPushButton(tr("Install the Engine"), this);
    m_installEngine->setObjectName(QStringLiteral("aiInstallEngine"));
    m_installEngine->setToolTip(tr("Downloads the engine of this version from the project's release page and installs it"));
    m_fromFile = new QPushButton(tr("Install from File..."), this);
    m_fromFile->setObjectName(QStringLiteral("aiInstallEngineFile"));
    m_fromFile->setToolTip(tr("Installs the engine from a package file (for a computer without a connection)"));
    engineRow->addWidget(m_engineState, 1);
    engineRow->addWidget(m_installEngine);
    engineRow->addWidget(m_fromFile);
    lay->addLayout(engineRow);

    m_models = new QTreeWidget(this);
    m_models->setObjectName(QStringLiteral("aiModels"));
    m_models->setRootIsDecorated(false);
    m_models->setAllColumnsShowFocus(true);
    m_models->setHeaderLabels({tr("Model"), tr("Languages"), tr("Download"), tr("Memory"), tr("License"), tr("State")});
    m_models->header()->setStretchLastSection(false);
    m_models->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int c = 1; c < 6; ++c)
        m_models->header()->setSectionResizeMode(c, QHeaderView::ResizeToContents);
    m_models->setMinimumHeight(120);
    lay->addWidget(m_models, 1);

    m_detail = new QLabel(this);
    m_detail->setObjectName(QStringLiteral("aiModelDetail"));
    m_detail->setWordWrap(true);
    m_detail->setOpenExternalLinks(true);
    m_detail->setTextFormat(Qt::RichText);
    lay->addWidget(m_detail);

    auto *row = new QHBoxLayout;
    m_download = new QPushButton(tr("Download"), this);
    m_download->setObjectName(QStringLiteral("aiDownloadModel"));
    m_delete = new QPushButton(tr("Delete"), this);
    m_delete->setObjectName(QStringLiteral("aiDeleteModel"));
    m_progress = new QProgressBar(this);
    m_progress->setObjectName(QStringLiteral("aiProgress"));
    m_progress->setRange(0, 1000);
    m_progress->setVisible(false);
    m_cancel = new QPushButton(tr("Cancel"), this);
    m_cancel->setObjectName(QStringLiteral("aiCancel"));
    m_cancel->setVisible(false);
    row->addWidget(m_download);
    row->addWidget(m_delete);
    row->addWidget(m_progress, 1);
    row->addWidget(m_cancel);
    lay->addLayout(row);
    m_message = new QLabel(this);
    m_message->setObjectName(QStringLiteral("aiMessage"));
    m_message->setWordWrap(true);
    lay->addWidget(m_message);

    connect(m_installEngine, &QPushButton::clicked, this, &AiInstallPanel::installEngine);
    connect(m_fromFile, &QPushButton::clicked, this, &AiInstallPanel::chooseFile);
    connect(m_download, &QPushButton::clicked, this, &AiInstallPanel::downloadSelected);
    connect(m_delete, &QPushButton::clicked, this, &AiInstallPanel::deleteSelected);
    connect(m_cancel, &QPushButton::clicked, this, [] { AiDownloads::instance().cancel(); });
    connect(m_models, &QTreeWidget::currentItemChanged, this, &AiInstallPanel::showDetail);
    AiDownloads &d = AiDownloads::instance();
    connect(&d, &AiDownloads::progress, this, &AiInstallPanel::onProgress);
    connect(&d, &AiDownloads::finished, this, &AiInstallPanel::onFinished);
    connect(&ai::Engine::instance(), &ai::Engine::stateChanged, this, &AiInstallPanel::refresh);
    refresh();
    if (d.busy())
        setBusy(true, d.busyWith());
}

QString AiInstallPanel::selectedModel() const
{
    const QTreeWidgetItem *it = m_models->currentItem();
    return it ? it->data(0, Qt::UserRole).toString() : QString();
}

void AiInstallPanel::selectModel(const QString &id)
{
    for (int i = 0; i < m_models->topLevelItemCount(); ++i)
        if (m_models->topLevelItem(i)->data(0, Qt::UserRole).toString() == id)
            m_models->setCurrentItem(m_models->topLevelItem(i));
}

void AiInstallPanel::refresh()
{
    const ai::PackageState p = ai::package(m_engine);
    const bool hand = m_engine == ai::handwritingEngineId();
    QString state;
    if (!p.installed)
        state = hand ? tr("The handwriting engine is not installed.") : tr("The AI engine is not installed.");
    else if (!p.fitsRuntime)
        state = tr("The installed engine %1 was made for another version of Quillbinder: install it again.").arg(p.version);
    else if (!p.enabled)
        state = tr("The engine %1 is switched off (Help > Add-ons and Updates > Manage Add-ons).").arg(p.version);
    else
        state = hand ? tr("The handwriting engine %1 is installed.").arg(p.version) : tr("The AI engine %1 is installed.").arg(p.version);
    if (!hand && ai::Engine::instance().restartNeeded())
        state += QLatin1Char(' ') + tr("The new engine is used after Quillbinder is started again.");
    m_engineState->setText(state);
    m_installEngine->setText(p.usable() ? tr("Install Again") : tr("Install the Engine"));

    const QString keep = selectedModel();
    m_models->clear();
    for (const ai::CatalogModel &m : ai::Catalog::builtIn().ofKinds(m_kinds)) {
        auto *it = new QTreeWidgetItem(m_models);
        it->setText(0, m.title);
        it->setData(0, Qt::UserRole, m.id);
        it->setText(1, languagesText(m.languages));
        qint64 bytes = m.downloadBytes();
        if (m.engine == QLatin1String("qwen3-tts")) {   // the voice pack of the Read Aloud language comes with the model
            const ai::CatalogModel *pack = readaloud::voicePackModel(readaloud::effectiveLanguage(readaloud::Config::instance().get()));
            if (pack && pack->published())
                bytes += pack->downloadBytes();
        }
        it->setText(2, megabytes(bytes));
        it->setText(3, m.memoryMB > 0 ? megabytes(qint64(m.memoryMB) * 1024 * 1024) : QStringLiteral("-"));
        it->setText(4, m.license);
        it->setText(5, ai::isInstalled(m) ? tr("Installed") : !m.published() ? tr("Not published yet") : tr("Not installed"));
        if (m.id == keep)
            m_models->setCurrentItem(it);
    }
    if (!m_models->currentItem() && m_models->topLevelItemCount() > 0)
        m_models->setCurrentItem(m_models->topLevelItem(0));
    if (m_models->topLevelItemCount() == 0)
        m_detail->setText(tr("The catalog has no models for this function yet."));
    showDetail();
}

void AiInstallPanel::showDetail()
{
    const ai::CatalogModel *m = ai::Catalog::builtIn().model(selectedModel());
    // busy from the click on (the downloader starts a moment later)
    const bool busy = AiDownloads::instance().busy() || m_progress->isVisible();
    if (!m) {
        m_download->setEnabled(false);
        m_delete->setEnabled(false);
        return;
    }
    const bool installed = ai::isInstalled(*m);
    QStringList lines;
    if (!m->summary.isEmpty())
        lines << m->summary.toHtmlEscaped();
    if (!m->credit.isEmpty())
        lines << m->credit.toHtmlEscaped();
    if (!m->license.isEmpty())
        lines << tr("License: %1").arg(m->licenseUrl.isEmpty() ? m->license.toHtmlEscaped()
                                                              : QStringLiteral("<a href=\"%1\">%2</a>").arg(m->licenseUrl.toHtmlEscaped(), m->license.toHtmlEscaped()));
    const QString problem = installed ? QString() : ai::downloadProblem(*m);
    if (!problem.isEmpty())
        lines << QStringLiteral("<b>") + problem.toHtmlEscaped() + QStringLiteral("</b>");
    m_detail->setText(lines.join(QStringLiteral("<br>")));
    m_download->setEnabled(!busy && !installed && problem.isEmpty());
    m_delete->setEnabled(!busy && (installed || QFileInfo::exists(ai::modelDir(m->id))));
}

void AiInstallPanel::setBusy(bool on, const QString &what)
{
    m_progress->setVisible(on);
    m_cancel->setVisible(on);
    m_installEngine->setEnabled(!on);
    m_fromFile->setEnabled(!on);
    if (on) {
        m_progress->setValue(0);
        m_message->setText(what == m_engine ? tr("Downloading the engine...") : tr("Downloading %1...").arg(what));
    }
    showDetail();
}

void AiInstallPanel::installEngine()
{
    setBusy(true, m_engine);
    AiDownloads::instance().installEngine(m_engine);
}

void AiInstallPanel::chooseFile()
{
    const QString f = QFileDialog::getOpenFileName(this, tr("Install the Engine from a File"), QDir::homePath(),
                                                   tr("Quillbinder packages (*.qbpkg)"));
    if (!f.isEmpty())
        installEngineFromFile(f);
}

bool AiInstallPanel::installEngineFromFile(const QString &file)
{
    QString why;
    bool unsignedPackage = false;
    bool ok = AiDownloads::installEngineFile(m_engine, file, false, &why, &unsignedPackage);
    if (!ok && unsignedPackage) {
        QMessageBox box(QMessageBox::Warning, tr("Install the Engine"),
                        tr("This package is not signed by the Quillbinder project, so it cannot be told whether it is "
                           "the project's own. Install it only when you trust where it comes from."),
                        QMessageBox::NoButton, this);
        box.setObjectName(QStringLiteral("aiUnsignedEngine"));
        QPushButton *yes = box.addButton(tr("Install Anyway"), QMessageBox::AcceptRole);
        box.addButton(QMessageBox::Cancel);
        box.setDefaultButton(QMessageBox::Cancel);
        box.exec();
        if (box.clickedButton() != yes) {
            m_message->setText(tr("The engine was not installed."));
            return false;
        }
        ok = AiDownloads::installEngineFile(m_engine, file, true, &why);
    }
    m_message->setText(ok ? tr("The engine is installed.") : why);
    refresh();
    if (ok)
        emit changed();
    return ok;
}

void AiInstallPanel::downloadSelected()
{
    const ai::CatalogModel *m = ai::Catalog::builtIn().model(selectedModel());
    if (!m || AiDownloads::instance().busy())
        return;
    if (m->nonCommercial) {
        QMessageBox box(QMessageBox::Question, tr("Download"),
                        tr("%1 may be used for non-commercial purposes only (%2). Download it?").arg(m->title, m->license),
                        QMessageBox::Yes | QMessageBox::No, this);
        box.setObjectName(QStringLiteral("aiNonCommercial"));
        if (box.exec() != QMessageBox::Yes)
            return;
    }
    setBusy(true, m->title);
    AiDownloads::instance().download(m->id);
}

void AiInstallPanel::deleteSelected()
{
    const ai::CatalogModel *m = ai::Catalog::builtIn().model(selectedModel());
    if (!m)
        return;
    QString why;
    if (ai::removeModel(*m, &why)) {
        readaloud::removeVoicePacksIfNoQwen();   // the last Qwen model takes the voice packs with it
        m_message->setText(tr("%1 is deleted.").arg(m->title));
        emit changed();
    } else {
        m_message->setText(why);
    }
    refresh();
}

void AiInstallPanel::onProgress(const QString &, qint64 done, qint64 total)
{
    if (!m_progress->isVisible())
        setBusy(true, AiDownloads::instance().busyWith());
    if (total > 0) {
        m_progress->setRange(0, 1000);
        m_progress->setValue(int(qBound<qint64>(0, done * 1000 / total, qint64(1000))));
    } else {
        m_progress->setRange(0, 0);   // the size is not known
    }
}

void AiInstallPanel::onFinished(const QString &, bool ok, const QString &message)
{
    setBusy(false);
    m_progress->setRange(0, 1000);
    m_message->setText(message);
    refresh();
    if (ok)
        emit changed();
}

} // namespace qb
