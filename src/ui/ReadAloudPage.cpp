#include "ui/ReadAloudPage.h"

#include "core/AiEngine.h"
#include "ui/AiInstallPanel.h"
#include "ui/Language.h"
#include "ui/OptionStorage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLocale>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace qb {
namespace readaloud {

QString ReadAloudPage::languageLabel(const QString &code)
{
    QString name = language::displayName(code);
    if (!languageHasModel(code))
        name += QStringLiteral(" (") + tr("no model for this language yet") + QLatin1Char(')');
    return name;
}

static void enableItem(QComboBox *c, int index, bool on, const QString &tip = QString())
{
    if (auto *m = qobject_cast<QStandardItemModel *>(c->model())) {
        m->item(index)->setEnabled(on);
        m->item(index)->setToolTip(tip);
    }
}

ReadAloudPage::ReadAloudPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("readAloudPage"));
    m_keep = Config::instance().get();
    auto *lay = new QVBoxLayout(this);

    auto *intro = new QLabel(tr("Read Aloud reads the marked text (or from the cursor to the end) with a speech model on this "
                                "computer. Nothing is installed until you install it here. The sound is only played, never saved."),
                             this);
    intro->setWordWrap(true);
    lay->addWidget(intro);

    auto *box = new QGroupBox(tr("Voice model"), this);
    auto *f = new QFormLayout(box);
    m_language = new QComboBox(box);
    m_language->setObjectName(QStringLiteral("raLanguage"));
    f->addRow(tr("Language:"), m_language);
    m_model = new QComboBox(box);
    m_model->setObjectName(QStringLiteral("raModel"));
    f->addRow(tr("Model:"), m_model);
    m_device = new QComboBox(box);
    m_device->setObjectName(QStringLiteral("raDevice"));
    m_device->setToolTip(tr("Automatic: the graphics card when the model fits into its free memory, else the computer's memory "
                            "and processor."));
    f->addRow(tr("Device:"), m_device);
    m_speed = new QLabel(box);
    m_speed->setObjectName(QStringLiteral("raSpeed"));
    m_speed->setWordWrap(true);
    f->addRow(tr("Speed:"), m_speed);
    m_state = new QLabel(box);
    m_state->setObjectName(QStringLiteral("raState"));
    m_state->setWordWrap(true);
    f->addRow(m_state);
    lay->addWidget(box);

    // 0.99.0: where each value is kept (File > Options tells it in the tool tip, as on the other pages): the program's,
    // for this computer's models
    qb::optstore::mark(m_language, QStringLiteral("readaloud/language"));
    qb::optstore::mark(m_model, QStringLiteral("readaloud/model"));
    qb::optstore::mark(m_device, QStringLiteral("readaloud/device"));

    m_panel = new AiInstallPanel(ai::aiEngineId(), {QStringLiteral("tts")}, this);
    lay->addWidget(m_panel, 1);
    auto *shared = new QLabel(tr("The processor cores and the minutes a model stays loaded are set for all AI functions in "
                                 "File > Options > General > AI Models."),
                              this);
    shared->setObjectName(QStringLiteral("raSharedHint"));
    shared->setWordWrap(true);
    lay->addWidget(shared);

    connect(m_panel, &AiInstallPanel::changed, this, [this] {
        fill();
        measure();
    });
    connect(m_language, &QComboBox::currentIndexChanged, this, [this] {
        if (!m_filling) {
            m_keep.language = m_language->currentData().toString();
            m_keep.model.clear();
            fill();
            measure();
        }
    });
    connect(m_model, &QComboBox::currentIndexChanged, this, [this] {
        if (!m_filling)
            measure();
    });
    connect(m_device, &QComboBox::currentIndexChanged, this, [this] {
        if (!m_filling) {
            m_keep.device = m_device->currentData().toString();
            fill();
            measure();
        }
    });
    connect(&Session::shared(), &Session::speedMeasured, this, &ReadAloudPage::onMeasured);
    fill();
}

void ReadAloudPage::fill()
{
    m_filling = true;
    ai::Engine &engine = ai::Engine::instance();
    if (engine.isInstalled())
        engine.ensureLoaded();
    const QList<ai::Device> devices = engine.isLoaded() ? engine.devices() : QList<ai::Device>();

    const QString chosenLanguage = m_keep.language;
    m_language->clear();
    for (const QString &code : language::available()) {
        m_language->addItem(languageLabel(code), code);
        if (!languageHasModel(code))
            enableItem(m_language, m_language->count() - 1, false);
    }
    m_language->setCurrentIndex(qMax(0, m_language->findData(effectiveLanguage(m_keep))));
    Q_UNUSED(chosenLanguage);

    m_device->clear();
    for (const auto &p : ai::deviceChoices(devices))
        m_device->addItem(p.second, p.first);
    m_device->setCurrentIndex(qMax(0, m_device->findData(m_keep.device)));

    const QString lang = effectiveLanguage(m_keep);
    Settings now = m_keep;
    now.device = m_device->currentData().toString();
    const ai::CatalogModel *def = resolveModel(now, devices, false);
    m_model->clear();
    m_model->addItem(def ? tr("Automatic (%1)").arg(def->title) : tr("Automatic"), QString());
    auto size = [](qint64 b) { return QLocale().formattedDataSize(b, 1, QLocale::DataSizeSIFormat); };
    for (const ai::CatalogModel *m : modelsFor(lang)) {
        const Fit fit = fitOf(*m, now.device, devices);
        const QString text = tr("%1: %2, %3, %4").arg(m->title, size(qint64(m->sizeMB) * 1000 * 1000), m->languages.join(QLatin1Char(' ')), fit.text);
        m_model->addItem(text, m->id);
        if (!fit.fits)
            enableItem(m_model, m_model->count() - 1, false, fit.text);
    }
    m_model->setCurrentIndex(qMax(0, m_model->findData(m_keep.model)));

    if (!engine.isInstalled())
        m_state->setText(tr("The engine is not installed yet: install it below, then a model."));
    else if (!resolveModel(now, devices, true))
        m_state->setText(tr("No model is installed for this language yet: download one below."));
    else
        m_state->setText(tr("Ready: press Play on the Read Aloud toolbar."));
    m_filling = false;
    showSpeed();
}

Settings ReadAloudPage::shown() const
{
    Settings s = m_keep;
    s.language = m_language->currentData().toString();
    s.model = m_model->currentData().toString();
    s.device = m_device->currentData().toString();
    return s;
}

void ReadAloudPage::showSpeed()
{
    const Settings s = shown();
    ai::Engine &e = ai::Engine::instance();
    const ai::CatalogModel *m = resolveModel(s, e.isLoaded() ? e.devices() : QList<ai::Device>(), true);
    if (!m) {
        m_speed->setText(tr("Not measured: no model is installed."));
        return;
    }
    if (!m_measuring.isEmpty()) {
        m_speed->setText(tr("Measuring the speed with a short test sentence..."));
        return;
    }
    const double r = measuredSpeed(m->id, s.device);
    if (r <= 0)
        m_speed->setText(tr("Not measured yet."));
    else if (r > 1.0)
        m_speed->setText(tr("Slower than real time (about %1 times slower on this device): Read Aloud makes a few sentences "
                            "ahead before it plays.")
                             .arg(QString::number(r, 'f', 1)));
    else
        m_speed->setText(tr("Faster than real time (%1 seconds of work for one second of sound).").arg(QString::number(r, 'f', 2)));
}

void ReadAloudPage::measure()
{
    showSpeed();
    const Settings s = shown();
    QString err;
    if (!Session::shared().measureSpeed(s, &err))
        return;   // no model, no engine or busy: showSpeed() says what it knows
    ai::Engine &e = ai::Engine::instance();
    const ai::CatalogModel *m = resolveModel(s, e.devices(), true);
    m_measuring = m ? m->id : QString();
    showSpeed();
}

void ReadAloudPage::onMeasured(const QString &, double, const QString &error)
{
    m_measuring.clear();
    showSpeed();
    if (!error.isEmpty())
        m_speed->setText(tr("The speed test failed: %1").arg(error));
}

void ReadAloudPage::apply()
{
    Settings s = shown();
    const Settings &now = Config::instance().get();
    s.voice = now.voice;
    s.style = now.style;
    s.speed = now.speed;
    s.barShown = now.barShown;
    s.settingsOpened = now.settingsOpened;
    Config::instance().set(s);
    m_keep = s;
}

} // namespace readaloud
} // namespace qb
