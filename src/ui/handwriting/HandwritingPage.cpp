#include "ui/handwriting/HandwritingPage.h"

#include "ui/Language.h"
#include "ui/OptionStorage.h"
#include "ui/handwriting/InkCanvas.h"
#include "ui/handwriting/LearningStore.h"
#include "ui/handwriting/Recognizer.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace qb {
namespace handwriting {

GridSample::GridSample(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("hwGridSample"));
    setMinimumSize(120, 90);
}

void GridSample::setGrid(const GridSettings &g)
{
    m_grid = g;
    update();
}

void GridSample::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), palette().color(QPalette::Base));
    InkCanvas::paintGrid(p, QRectF(rect()), m_grid, palette().color(QPalette::Text));
    p.setPen(palette().color(QPalette::Mid));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

QString HandwritingPage::languageLabel(const QString &code)
{
    QString name = language::displayName(code);
    if (!hasModelFor(code))
        name += QStringLiteral(" (") + tr("no model for this language yet") + QLatin1Char(')');
    return name;
}

static QComboBox *languageCombo(QWidget *parent, bool withNone)
{
    auto *c = new QComboBox(parent);
    if (withNone)
        c->addItem(QObject::tr("None"), QString());
    for (const QString &code : language::available()) {
        c->addItem(HandwritingPage::languageLabel(code), code);
        if (!hasModelFor(code)) {
            // Russian: greyed out, it cannot be chosen
            auto *m = qobject_cast<QStandardItemModel *>(c->model());
            if (m)
                m->item(c->count() - 1)->setEnabled(false);
        }
    }
    return c;
}

void HandwritingPage::showColor(QPushButton *button, const QColor &c)
{
    QPixmap pm(16, 16);
    pm.fill(c.isValid() ? c : palette().color(QPalette::Text));
    button->setIcon(QIcon(pm));
    button->setText(c.isValid() ? c.name() : tr("Theme color"));
}

void HandwritingPage::pickColor(QColor *target, QPushButton *button, const QString &title)
{
    QColor c = QColorDialog::getColor(target->isValid() ? *target : palette().color(QPalette::Text), this, title);
    if (c.isValid()) {
        *target = c;
        showColor(button, c);
    }
}

HandwritingPage::HandwritingPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("handwritingPage"));
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);
    auto *inner = new QWidget;
    scroll->setWidget(inner);
    auto *lay = new QVBoxLayout(inner);

    // ---- recognition
    auto *rec = new QGroupBox(tr("Recognition"), inner);
    auto *rf = new QFormLayout(rec);
    m_buttonShown = new QCheckBox(tr("Show the handwriting button on the toolbar"), rec);
    m_buttonShown->setObjectName(QStringLiteral("hwButtonShown"));
    rf->addRow(m_buttonShown);
    // the install part: the shared installer for engine packages and models sets Recognizer::setInstaller()
    auto *slot = new QWidget(rec);
    slot->setObjectName(QStringLiteral("hwInstallSlot"));
    auto *sl = new QHBoxLayout(slot);
    sl->setContentsMargins(0, 0, 0, 0);
    m_installStatus = new QLabel(slot);
    m_installStatus->setObjectName(QStringLiteral("hwInstallStatus"));
    m_installStatus->setWordWrap(true);
    m_install = new QPushButton(tr("Install"), slot);
    m_install->setObjectName(QStringLiteral("hwInstall"));
    m_remove = new QPushButton(tr("Remove"), slot);
    m_remove->setObjectName(QStringLiteral("hwRemove"));
    sl->addWidget(m_installStatus, 1);
    sl->addWidget(m_install);
    sl->addWidget(m_remove);
    rf->addRow(tr("Handwriting model:"), slot);
    connect(m_install, &QPushButton::clicked, this, [this] {
        const Recognizer::Installer &i = Recognizer::installer();
        if (i.install)
            i.install(window());
        refreshInstall();
    });
    connect(m_remove, &QPushButton::clicked, this, [this] {
        const Recognizer::Installer &i = Recognizer::installer();
        if (i.remove)
            i.remove();
        refreshInstall();
    });
    m_main = languageCombo(rec, false);
    m_main->setObjectName(QStringLiteral("hwMainLanguage"));
    m_fallback = languageCombo(rec, true);
    m_fallback->setObjectName(QStringLiteral("hwFallbackLanguage"));
    rf->addRow(tr("Main language:"), m_main);
    rf->addRow(tr("Fallback language:"), m_fallback);
    lay->addWidget(rec);

    // ---- writing
    auto *wr = new QGroupBox(tr("Writing"), inner);
    auto *wf = new QFormLayout(wr);
    auto seconds = [wr](const QString &name, double lo, double hi) {
        auto *s = new QDoubleSpinBox(wr);
        s->setObjectName(name);
        s->setRange(lo, hi);
        s->setDecimals(1);
        s->setSingleStep(0.1);
        s->setSuffix(QObject::tr(" s"));
        return s;
    };
    m_pause = seconds(QStringLiteral("hwPause"), 0.2, 10);
    m_stay = seconds(QStringLiteral("hwStay"), 0, 30);
    m_fade = seconds(QStringLiteral("hwFade"), 0, 30);
    wf->addRow(tr("Pause before a line is read:"), m_pause);
    wf->addRow(tr("The ink stays for:"), m_stay);
    wf->addRow(tr("The ink fades over:"), m_fade);
    m_inkColorButton = new QPushButton(wr);
    m_inkColorButton->setObjectName(QStringLiteral("hwInkColor"));
    auto *inkReset = new QPushButton(tr("Theme color"), wr);
    auto *inkRow = new QHBoxLayout;
    inkRow->addWidget(m_inkColorButton);
    inkRow->addWidget(inkReset);
    wf->addRow(tr("Ink color:"), inkRow);
    connect(m_inkColorButton, &QPushButton::clicked, this, [this] { pickColor(&m_inkColor, m_inkColorButton, tr("Ink Color")); });
    connect(inkReset, &QPushButton::clicked, this, [this] { m_inkColor = QColor(); showColor(m_inkColorButton, m_inkColor); });
    m_inkWidth = new QDoubleSpinBox(wr);
    m_inkWidth->setObjectName(QStringLiteral("hwInkWidth"));
    m_inkWidth->setRange(0.5, 12);
    m_inkWidth->setDecimals(1);
    m_inkWidth->setSingleStep(0.5);
    m_inkWidth->setSuffix(tr(" px"));
    wf->addRow(tr("Ink width:"), m_inkWidth);
    lay->addWidget(wr);

    // ---- the grid
    auto *gr = new QGroupBox(tr("Grid (in the inspector's pad)"), inner);
    auto *gl = new QHBoxLayout(gr);
    auto *gf = new QFormLayout;
    gl->addLayout(gf, 1);
    m_gridStyle = new QComboBox(gr);
    m_gridStyle->setObjectName(QStringLiteral("hwGridStyle"));
    for (auto s : {GridSettings::Ruled, GridSettings::Writing, GridSettings::Squares, GridSettings::Dots, GridSettings::None})
        m_gridStyle->addItem(GridSettings::styleName(s), int(s));
    gf->addRow(tr("Style:"), m_gridStyle);
    m_gridColorButton = new QPushButton(gr);
    m_gridColorButton->setObjectName(QStringLiteral("hwGridColor"));
    auto *gridReset = new QPushButton(tr("Theme color"), gr);
    auto *gcRow = new QHBoxLayout;
    gcRow->addWidget(m_gridColorButton);
    gcRow->addWidget(gridReset);
    gf->addRow(tr("Color:"), gcRow);
    connect(m_gridColorButton, &QPushButton::clicked, this, [this] { pickColor(&m_gridColor, m_gridColorButton, tr("Grid Color")); });
    connect(gridReset, &QPushButton::clicked, this, [this] { m_gridColor = QColor(); showColor(m_gridColorButton, m_gridColor); });
    m_gridWidth = new QDoubleSpinBox(gr);
    m_gridWidth->setObjectName(QStringLiteral("hwGridWidth"));
    m_gridWidth->setRange(0.5, 4);
    m_gridWidth->setDecimals(1);
    m_gridWidth->setSingleStep(0.5);
    m_gridWidth->setSuffix(tr(" px"));
    gf->addRow(tr("Line width:"), m_gridWidth);
    m_gridOpacity = new QSpinBox(gr);
    m_gridOpacity->setObjectName(QStringLiteral("hwGridOpacity"));
    m_gridOpacity->setRange(5, 100);
    m_gridOpacity->setSuffix(QStringLiteral(" %"));
    gf->addRow(tr("Opacity:"), m_gridOpacity);
    m_gridSize = new QSlider(Qt::Horizontal, gr);
    m_gridSize->setObjectName(QStringLiteral("hwGridSize"));
    m_gridSize->setRange(0, 100);
    auto *sizeRow = new QHBoxLayout;
    sizeRow->addWidget(new QLabel(tr("small"), gr));
    sizeRow->addWidget(m_gridSize, 1);
    sizeRow->addWidget(new QLabel(tr("large"), gr));
    gf->addRow(tr("Size:"), sizeRow);
    m_sample = new GridSample(gr);
    gl->addWidget(m_sample);
    lay->addWidget(gr);
    auto updateSample = [this] { m_sample->setGrid(shown().grid); };
    connect(m_gridStyle, &QComboBox::currentIndexChanged, this, updateSample);
    connect(m_gridWidth, &QDoubleSpinBox::valueChanged, this, updateSample);
    connect(m_gridOpacity, &QSpinBox::valueChanged, this, updateSample);
    connect(m_gridSize, &QSlider::valueChanged, this, updateSample);
    connect(m_gridColorButton, &QPushButton::clicked, this, updateSample);
    connect(gridReset, &QPushButton::clicked, this, updateSample);

    // ---- hand protection
    auto *hp = new QGroupBox(tr("Hand protection"), inner);
    auto *hf = new QFormLayout(hp);
    m_protect = new QCheckBox(tr("Ignore the resting hand while the pen writes"), hp);
    m_protect->setObjectName(QStringLiteral("hwProtect"));
    hf->addRow(m_protect);
    m_hand = new QComboBox(hp);
    m_hand->setObjectName(QStringLiteral("hwHand"));
    m_hand->addItem(tr("Right-handed"), false);
    m_hand->addItem(tr("Left-handed"), true);
    hf->addRow(tr("Writing hand:"), m_hand);
    m_strength = new QSlider(Qt::Horizontal, hp);
    m_strength->setObjectName(QStringLiteral("hwStrength"));
    m_strength->setRange(0, 100);
    auto *strRow = new QHBoxLayout;
    strRow->addWidget(new QLabel(tr("weak"), hp));
    strRow->addWidget(m_strength, 1);
    strRow->addWidget(new QLabel(tr("strong"), hp));
    hf->addRow(tr("Strength:"), strRow);
    lay->addWidget(hp);

    // ---- pen and finger
    auto *pf = new QGroupBox(tr("Pen and finger"), inner);
    auto *pl = new QFormLayout(pf);
    m_fingerWrites = new QCheckBox(tr("Without a pen, one finger writes (two fingers scroll)"), pf);
    m_fingerWrites->setObjectName(QStringLiteral("hwFingerWrites"));
    m_sideButton = new QCheckBox(tr("The pen's side button selects text"), pf);
    m_sideButton->setObjectName(QStringLiteral("hwSideButton"));
    m_penSeconds = new QSpinBox(pf);
    m_penSeconds->setObjectName(QStringLiteral("hwPenSeconds"));
    m_penSeconds->setRange(1, 3600);
    m_penSeconds->setSuffix(tr(" s"));
    pl->addRow(m_fingerWrites);
    pl->addRow(m_sideButton);
    pl->addRow(tr("A pen counts as in use after its last touch for:"), m_penSeconds);
    lay->addWidget(pf);

    // ---- learning
    auto *lg = new QGroupBox(tr("Learning your hand"), inner);
    auto *ll = new QVBoxLayout(lg);
    m_learning = new QCheckBox(tr("Keep corrected lines on this device"), lg);
    m_learning->setObjectName(QStringLiteral("hwLearning"));
    ll->addWidget(m_learning);
    auto *lh = new QLabel(tr("Only lines you corrected are kept: the picture of the ink and the corrected text. They stay on this "
                             "device and are never sent anywhere. While an encrypted project is open, nothing is kept (the files "
                             "would be plain)."),
                          lg);
    lh->setWordWrap(true);
    ll->addWidget(lh);
    auto *lr = new QHBoxLayout;
    m_learned = new QLabel(lg);
    m_learned->setObjectName(QStringLiteral("hwLearned"));
    auto *del = new QPushButton(tr("Delete the Kept Lines"), lg);
    del->setObjectName(QStringLiteral("hwDeleteLearned"));
    lr->addWidget(m_learned, 1);
    lr->addWidget(del);
    ll->addLayout(lr);
    connect(del, &QPushButton::clicked, this, [this] {
        LearningStore::removeAll();
        refreshLearning();
    });
    lay->addWidget(lg);
    lay->addStretch(1);

    load(HandwritingConfig::instance().get());
    refreshInstall();
    refreshLearning();
}

void HandwritingPage::markStorage()
{
    const QList<std::pair<QWidget *, const char *>> keys = {
        {m_buttonShown, "buttonShown"}, {m_main, "mainLanguage"}, {m_fallback, "fallbackLanguage"}, {m_pause, "pauseMs"},
        {m_stay, "stayMs"}, {m_fade, "fadeMs"}, {m_inkColorButton, "inkColor"}, {m_inkWidth, "inkWidth"},
        {m_gridStyle, "grid/style"}, {m_gridColorButton, "grid/color"}, {m_gridWidth, "grid/lineWidth"},
        {m_gridOpacity, "grid/opacity"}, {m_gridSize, "grid/size"}, {m_protect, "protect/on"}, {m_hand, "protect/left"},
        {m_strength, "protect/strength"}, {m_fingerWrites, "pen/fingerWrites"}, {m_sideButton, "pen/sideButtonSelects"},
        {m_penSeconds, "pen/inUseSeconds"}, {m_learning, "learning/on"}};
    for (const auto &k : keys)
        optstore::mark(k.first, QStringLiteral("handwriting/") + QLatin1String(k.second));
}

void HandwritingPage::refreshInstall()
{
    const Recognizer::Installer &i = Recognizer::installer();
    QString status;
    if (i.status)
        status = i.status();
    if (status.isEmpty())
        status = Recognizer::installed() ? tr("Installed.") : tr("Not installed. Install it to write by hand.");
    m_installStatus->setText(status);
    m_install->setEnabled(bool(i.install));
    m_remove->setVisible(bool(i.remove));
    m_install->setToolTip(i.install ? QString() : tr("The installer for handwriting models is not available in this version yet."));
}

void HandwritingPage::refreshLearning()
{
    m_learned->setText(tr("%n line(s) kept", nullptr, LearningStore::count()));
}

void HandwritingPage::load(const HandwritingSettings &s)
{
    m_keep = s;
    m_buttonShown->setChecked(s.buttonShown);
    m_main->setCurrentIndex(qMax(0, m_main->findData(s.mainLanguage)));
    m_fallback->setCurrentIndex(qMax(0, m_fallback->findData(s.fallbackLanguage)));
    m_pause->setValue(s.pauseMs / 1000.0);
    m_stay->setValue(s.stayMs / 1000.0);
    m_fade->setValue(s.fadeMs / 1000.0);
    m_inkColor = s.inkColor;
    showColor(m_inkColorButton, m_inkColor);
    m_inkWidth->setValue(s.inkWidth);
    m_gridStyle->setCurrentIndex(qMax(0, m_gridStyle->findData(int(s.grid.style))));
    m_gridColor = s.grid.color;
    showColor(m_gridColorButton, m_gridColor);
    m_gridWidth->setValue(s.grid.lineWidth);
    m_gridOpacity->setValue(s.grid.opacity);
    m_gridSize->setValue(s.grid.size);
    m_protect->setChecked(s.protectHand);
    m_hand->setCurrentIndex(s.leftHanded ? 1 : 0);
    m_strength->setValue(s.strength);
    m_fingerWrites->setChecked(s.fingerWrites);
    m_sideButton->setChecked(s.sideButtonSelects);
    m_penSeconds->setValue(s.penInUseSeconds);
    m_learning->setChecked(s.learning);
    m_sample->setGrid(shown().grid);
}

HandwritingSettings HandwritingPage::shown() const
{
    HandwritingSettings s = m_keep;
    s.buttonShown = m_buttonShown->isChecked();
    s.mainLanguage = m_main->currentData().toString();
    s.fallbackLanguage = m_fallback->currentData().toString();
    if (s.fallbackLanguage == s.mainLanguage)
        s.fallbackLanguage.clear();
    s.pauseMs = int(qRound(m_pause->value() * 1000));
    s.stayMs = int(qRound(m_stay->value() * 1000));
    s.fadeMs = int(qRound(m_fade->value() * 1000));
    s.inkColor = m_inkColor;
    s.inkWidth = m_inkWidth->value();
    s.grid.style = GridSettings::Style(m_gridStyle->currentData().toInt());
    s.grid.color = m_gridColor;
    s.grid.lineWidth = m_gridWidth->value();
    s.grid.opacity = m_gridOpacity->value();
    s.grid.size = m_gridSize->value();
    s.protectHand = m_protect->isChecked();
    s.leftHanded = m_hand->currentData().toBool();
    s.strength = m_strength->value();
    s.fingerWrites = m_fingerWrites->isChecked();
    s.sideButtonSelects = m_sideButton->isChecked();
    s.penInUseSeconds = m_penSeconds->value();
    s.learning = m_learning->isChecked();
    return s;
}

void HandwritingPage::apply()
{
    HandwritingSettings s = shown();
    // the mode is the button's, not the page's: keep what the program holds now
    const HandwritingSettings &now = HandwritingConfig::instance().get();
    s.mode = now.mode;
    s.lastMode = now.lastMode;
    HandwritingConfig::instance().set(s);
    m_keep = s;
}

} // namespace handwriting
} // namespace qb
