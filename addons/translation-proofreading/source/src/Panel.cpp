#include "Panel.h"

#include "TranslateTool.h"

#include <QAbstractTextDocumentLayout>
#include <algorithm>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QElapsedTimer>
#include <QFrame>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QThread>
#include <QToolButton>
#include <QUrl>
#include <QRegularExpression>

namespace tp {

namespace {

enum Prop {
    UnitProp = QTextFormat::UserProperty + 1,
    WordProp,
    SegmentProp,
};

const QString kCorrect = QStringLiteral("Fix grammatical errors in this sentence: ");
const QString kFormal = QStringLiteral("Make this sentence more formal: ");
const QString kParaphrase = QStringLiteral("Paraphrase this sentence: ");

QList<Alternative> runAltJob(const AltJob &j, QString *error)
{
    TranslateTool::lowerPriority();
    if (!j.engine || !j.engine->load(j.slot, j.files, j.threads, error))
        return {};
    return j.engine->alternatives(j.slot, j.source, j.current, j.word, j.options, 7, error);
}

QIcon themed(const QString &name)
{
    return QIcon::fromTheme(name);
}

QToolButton *toolButton(const QString &icon, const QString &text, const QString &tip, QWidget *parent)
{
    auto *b = new QToolButton(parent);
    b->setIcon(themed(icon));
    b->setText(text);
    b->setToolTip(tip);
    b->setToolButtonStyle(b->icon().isNull() ? Qt::ToolButtonTextOnly : Qt::ToolButtonTextBesideIcon);
    b->setAutoRaise(true);
    return b;
}

// 1.4: the hint in an empty text field. Qt's own placeholder text was cut off at a narrow width; this label wraps its
// words over the whole viewport and goes away with the first text.
class WrappedPlaceholder : public QLabel
{
public:
    WrappedPlaceholder(QAbstractScrollArea *area, QTextDocument *doc, const QString &text)
        : QLabel(text, area->viewport())
        , m_doc(doc)
    {
        setObjectName(QStringLiteral("tpPlaceholder"));
        setWordWrap(true);
        setAlignment(Qt::AlignTop | Qt::AlignLeft);
        setMargin(4);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        QPalette p = palette();
        p.setColor(QPalette::WindowText, p.color(QPalette::PlaceholderText));
        setPalette(p);
        area->viewport()->installEventFilter(this);
        connect(doc, &QTextDocument::contentsChanged, this, [this] { refresh(); });
        refresh();
    }
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Resize)
            refresh();
        return QLabel::eventFilter(o, e);
    }
    void refresh()
    {
        setGeometry(parentWidget()->rect());
        setVisible(m_doc->isEmpty());
    }

private:
    QTextDocument *m_doc;
};

} // namespace

// ---------------------------------------------------------------- InputEdit

InputEdit::InputEdit(tp::Panel *panel, QWidget *parent)
    : QPlainTextEdit(parent)
    , m_panel(panel)
{
    setObjectName(QStringLiteral("tpInput"));
    new WrappedPlaceholder(this, document(),
                           tr("Type or paste text here, or take the selected text from the editor. English text is "
                              "checked for spelling and grammar, German text is translated into English."));
}

void InputEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu(e->pos());
    const QTextCursor c = cursorForPosition(e->pos());
    const tp::Panel::InputPlace place = m_panel->inputPlace(c.position());
    if (!place.wordText.isEmpty()) {
        QAction *first = menu->actions().value(0);
        QAction *ac = new QAction(tr("Other Words for “%1”...").arg(place.wordText), menu);
        const int pos = c.position();
        const QPoint global = e->globalPos();
        connect(ac, &QAction::triggered, m_panel, [this, pos, global] { m_panel->showInputAlternatives(pos, global); });
        menu->insertAction(first, ac);
        menu->insertSeparator(first);
    }
    menu->exec(e->globalPos());
    delete menu;
}

void InputEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseDoubleClickEvent(e);   // the word is selected as usual
    m_panel->showInfoForInput(cursorForPosition(e->pos()).position());
}

void InputEdit::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && (e->modifiers() & Qt::ControlModifier)) {
        m_panel->run();
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
}

// ---------------------------------------------------------------- OutputView

OutputView::OutputView(QWidget *parent)
    : QTextBrowser(parent)
{
    setObjectName(QStringLiteral("tpOutput"));
    setOpenLinks(false);
    new WrappedPlaceholder(this, document(),
                           tr("The result. Marked words were changed: click one to keep the original or to choose "
                              "another word. Every other word can be clicked for other words, too; a double click shows a "
                              "word's suggestions at the bottom."));
    viewport()->setCursor(Qt::PointingHandCursor);
    connect(this, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) { emit linkClicked(url.toString()); });
    m_clickTimer.setSingleShot(true);
    m_clickTimer.setInterval(QApplication::doubleClickInterval());
    connect(&m_clickTimer, &QTimer::timeout, this, [this] {
        if (m_clickPos >= 0)
            emit wordClicked(m_clickPos, m_clickGlobal);
    });
}

int OutputView::charAt(const QPoint &viewportPos) const
{
    const QPoint docPos = viewportPos + QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
    return document()->documentLayout()->hitTest(docPos, Qt::ExactHit);
}

void OutputView::mousePressEvent(QMouseEvent *e)
{
    m_pressPos = e->pos();
    QTextBrowser::mousePressEvent(e);
}

void OutputView::mouseReleaseEvent(QMouseEvent *e)
{
    QTextBrowser::mouseReleaseEvent(e);
    if (m_clickPos == -2) {   // the release of a double click
        m_clickPos = -1;
        return;
    }
    if (!anchorAt(e->pos()).isEmpty())   // a link: anchorClicked (below) takes it
        return;
    if (e->button() != Qt::LeftButton || (e->pos() - m_pressPos).manhattanLength() > 4 || textCursor().hasSelection())
        return;
    const int pos = charAt(e->pos());
    if (pos >= 0) {
        // the menu after the double-click time: a double click is no click
        m_clickPos = pos;
        m_clickGlobal = e->globalPosition().toPoint();
        m_clickTimer.start();
    }
}

void OutputView::mouseDoubleClickEvent(QMouseEvent *e)
{
    m_clickTimer.stop();
    const int pos = charAt(e->pos());
    m_clickPos = -2;
    if (pos >= 0 && anchorAt(e->pos()).isEmpty())
        emit wordDoubleClicked(pos);
}

void OutputView::contextMenuEvent(QContextMenuEvent *e)
{
    emit contextRequested(e->globalPos());
}

// ---------------------------------------------------------------- Panel

Panel::Panel(TranslateTool *tool, Place place, QWidget *parent)
    : QWidget(parent)
    , m_tool(tool)
    , m_place(place)
{
    setObjectName(place == Place::Window ? QStringLiteral("tpWindowPanel") : QStringLiteral("tpInspectorPanel"));
    m_input = new InputEdit(this, this);
    m_output = new OutputView(this);
    connect(m_output, &OutputView::wordClicked, this, &Panel::outputClicked);
    connect(m_output, &OutputView::wordDoubleClicked, this, &Panel::showInfoForOutput);
    connect(m_output, &OutputView::linkClicked, this, &Panel::linkClicked);
    connect(m_output, &OutputView::contextRequested, this, &Panel::outputContextMenu);

    // 1.1: the information area, and live work about a second after the writer stops typing
    m_info = new QTextBrowser(this);
    m_info->setObjectName(QStringLiteral("tpInfo"));
    m_info->setOpenLinks(false);
    new WrappedPlaceholder(m_info, m_info->document(),
                           tr("Double-click a word in the text or in the result: its spelling suggestions, words of "
                              "a similar meaning appear here."));
    m_info->setMinimumHeight(m_info->fontMetrics().lineSpacing() * 3);
    m_live = toolButton(QStringLiteral("view-refresh"), tr("Live"),
                        tr("Live: about a second after you stop typing, the changed sentences are worked on again"), this);
    m_live->setObjectName(QStringLiteral("tpLive"));
    m_live->setCheckable(true);
    m_live->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_live->setChecked(m_tool->setting(QStringLiteral("live"), true).toBool());
    connect(m_live, &QToolButton::toggled, this, [this](bool on) {
        m_tool->setSetting(QStringLiteral("live"), on);
        if (on && !inputText().trimmed().isEmpty())
            m_liveTimer.start();
        else if (!on)
            m_liveTimer.stop();
    });
    m_liveTimer.setSingleShot(true);
    m_liveTimer.setInterval(1000);
    connect(&m_liveTimer, &QTimer::timeout, this, [this] { runNow(true); });
    connect(m_input, &QPlainTextEdit::textChanged, this, &Panel::liveTyped);
    connect(m_input, &QPlainTextEdit::textChanged, this, [this] {
        m_back->clear();   // what was translated back belongs to the text before
        m_back->setVisible(false);
        updateModeUi();
    });

    m_task = new QComboBox(this);
    m_task->setObjectName(QStringLiteral("tpTask"));
    m_task->addItem(tr("Automatic"), QStringLiteral("auto"));
    m_task->addItem(tr("Proofread English"), QStringLiteral("proofread"));
    m_task->addItem(tr("Translate German to English"), QStringLiteral("translate"));
    m_task->addItem(tr("Translate English to German"), QStringLiteral("translateEn"));
    m_task->setToolTip(tr("Automatic: German text is translated into English, English text is proofread."));
    m_style = new QComboBox(this);
    m_style->setObjectName(QStringLiteral("tpStyle"));
    m_style->addItem(tr("Correct"), QStringLiteral("correct"));
    m_style->addItem(tr("More Formal"), QStringLiteral("formal"));
    m_style->setToolTip(tr("Correct: spelling and grammar only. More Formal: the model may also rewrite for a more "
                           "formal tone."));
    m_model = new QComboBox(this);
    m_model->setObjectName(QStringLiteral("tpModel"));
    // 1.3: the second row shows the model of the work: the proofreading model, or in translate mode the translator
    connect(m_model, &QComboBox::activated, this, [this](int i) {
        const QString id = m_model->itemData(i).toString();
        if (!id.isEmpty())
            m_tool->setSetting(m_modeKind, id);   // "corrector", "translator" or "translator-en"; the panels follow
        // a translation on show is made again with the chosen model
        if (!id.isEmpty() && directionNow() != Direction::Proofread && !m_units.isEmpty() && !m_busy
            && !inputText().trimmed().isEmpty())
            run();
    });
    connect(m_task, &QComboBox::currentIndexChanged, this, [this] { updateModeUi(); });
    m_swap = new QPushButton(tr("Swap"), this);
    m_swap->setObjectName(QStringLiteral("tpSwap"));
    m_swap->setToolTip(tr("German to English becomes English to German and back: the result moves into the text and is "
                          "translated the other way"));
    m_swap->setMinimumHeight(36);   // a finger on a tablet
    connect(m_swap, &QPushButton::clicked, this, &Panel::swapDirection);
    m_backButton = new QPushButton(tr("Translate back"), this);
    m_backButton->setObjectName(QStringLiteral("tpBack"));
    m_backButton->setToolTip(tr("Translate the result back into the language of your text, to see what it says"));
    m_backButton->setMinimumHeight(36);
    m_backButton->setEnabled(false);
    connect(m_backButton, &QPushButton::clicked, this, &Panel::translateBack);
    m_back = new QTextBrowser(this);
    m_back->setObjectName(QStringLiteral("tpBackView"));
    m_back->setOpenLinks(false);
    m_back->setVisible(false);
    m_back->setMaximumHeight(m_back->fontMetrics().lineSpacing() * 5);
    connect(m_back, &QTextBrowser::anchorClicked, this, [this](const QUrl &url) {
        if (url.toString().startsWith(QLatin1String("tp-download:")))
            m_tool->store()->download(url.toString().mid(12));
    });
    connect(m_tool->store(), &ModelStore::finished, this, [this](const QString &, bool ok, const QString &) {
        if (ok)
            updateModeUi();
    });
    m_models = toolButton(QStringLiteral("configure"), tr("Models..."), tr("Choose and download models, set the rules"), this);
    connect(m_models, &QToolButton::clicked, m_tool, &TranslateTool::showOptions);

    m_go = new QPushButton(this);
    m_go->setObjectName(QStringLiteral("tpGo"));
    m_go->setDefault(true);
    m_go->setToolTip(tr("Proofread or translate the text (Ctrl+Return in the text)"));
    connect(m_go, &QPushButton::clicked, this, &Panel::run);
    m_stop = toolButton(QStringLiteral("media-playback-stop"), tr("Stop"), tr("Stop the work"), this);
    m_stop->setVisible(false);
    connect(m_stop, &QToolButton::clicked, this, &Panel::stop);
    m_from = toolButton(QStringLiteral("go-up"), tr("From Editor"), tr("Take the text selected in the editor and work on it"), this);
    connect(m_from, &QToolButton::clicked, this, &Panel::takeFromEditor);
    m_into = toolButton(QStringLiteral("go-down"), tr("Into Editor"),
                        tr("Put the result into the editor in place of its selection (one step to undo there)"), this);
    connect(m_into, &QToolButton::clicked, this, &Panel::putIntoEditor);
    m_copy = toolButton(QStringLiteral("edit-copy"), tr("Copy"), tr("Copy the result"), this);
    connect(m_copy, &QToolButton::clicked, this, &Panel::copyResult);

    m_progress = new QProgressBar(this);
    m_progress->setVisible(false);
    m_progress->setTextVisible(false);
    m_progress->setMaximumHeight(8);
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("tpStatus"));
    m_status->setWordWrap(true);
    m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // 1.4: the notice about missing parts
    m_notice = new QFrame(this);
    m_notice->setObjectName(QStringLiteral("tpNotice"));
    static_cast<QFrame *>(m_notice)->setFrameShape(QFrame::StyledPanel);
    auto *nl = new QHBoxLayout(m_notice);
    nl->setContentsMargins(6, 4, 6, 4);
    m_noticeText = new QLabel(m_notice);
    m_noticeText->setObjectName(QStringLiteral("tpNoticeText"));
    m_noticeText->setWordWrap(true);
    m_noticeButton = new QPushButton(tr("Open the Options"), m_notice);
    m_noticeButton->setObjectName(QStringLiteral("tpNoticeButton"));
    m_noticeButton->setToolTip(tr("File > Options > Proofreading: choose and download the models and the German dictionary"));
    connect(m_noticeButton, &QPushButton::clicked, m_tool, &TranslateTool::showOptions);
    nl->addWidget(m_noticeText, 1);
    nl->addWidget(m_noticeButton, 0, Qt::AlignTop);
    m_notice->setVisible(false);
    m_notes = new QLabel(this);
    m_notes->setObjectName(QStringLiteral("tpNotes"));
    m_notes->setWordWrap(true);
    m_notes->setVisible(false);
    QFont small = m_notes->font();
    small.setPointSizeF(small.pointSizeF() * 0.9);
    m_notes->setFont(small);

    if (m_place == Place::Window)
        buildWindowLayout();
    else
        buildInspectorLayout();
    updateModelBox();
    updateModeUi();
    setStatus(tr("Ready."));
}

QString Panel::workKey() const
{
    return m_dir == Direction::Proofread ? m_styleId : m_dir == Direction::DeToEn ? QStringLiteral("translate")
                                                                                    : QStringLiteral("translate-de");
}

Rules Panel::rulesForRun() const
{
    Rules r = m_tool->rules();
    if (m_dir == Direction::EnToDe) {   // the American spellings and curly quotes are English rules
        r.american = false;
        r.curlyQuotes = false;
    }
    return r;
}

QString Panel::kindOf(Direction d) const
{
    return d == Direction::DeToEn ? QStringLiteral("translator")
           : d == Direction::EnToDe ? QStringLiteral("translator-en")
                                    : QStringLiteral("corrector");
}

Panel::Direction Panel::directionFor(const QString &task, const QString &text) const
{
    if (task == QLatin1String("translate"))
        return Direction::DeToEn;
    if (task == QLatin1String("translateEn"))
        return Direction::EnToDe;
    if (task == QLatin1String("proofread"))
        return Direction::Proofread;
    if (text.trimmed().isEmpty())
        return Direction::Proofread;
    if (detectLanguage(text) == QLatin1String("de"))
        return Direction::DeToEn;
    // 1.4: a lone word the short word list does not know ("Automatic" ran it as Correct with a proofreading model): a word
    // that is no English word but is one of the German dictionary is German
    QString word;
    if (isSingleWord(text, &word) && !knownWord(word)) {
        const QString file = m_tool->dictionaryFile();
        if (!file.isEmpty() && ensureGermanDictionary(file) && !lookupGerman(word).isEmpty())
            return Direction::DeToEn;
    }
    return Direction::Proofread;
}

Panel::Direction Panel::directionNow() const
{
    return directionFor(currentTask(), inputText());
}

Panel::~Panel()
{
    if (m_cancel)
        m_cancel->store(true);
}

void Panel::buildInspectorLayout()
{
    m_go->setText(tr("Check"));
    if (QFileInfo::exists(m_tool->iconPath()))
        m_go->setIcon(QIcon(m_tool->iconPath()));
    auto *controls = new QWidget(this);
    auto *g = new QGridLayout(controls);
    g->setContentsMargins(0, 4, 0, 4);
    g->setVerticalSpacing(3);
    m_controls = g;
    g->addWidget(m_task, 0, 0, 1, 2);
    g->addWidget(m_style, 1, 0);
    g->addWidget(m_model, 1, 1);
    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(2);
    buttons->addWidget(m_go);
    buttons->addWidget(m_live);
    buttons->addWidget(m_stop);
    buttons->addStretch(1);
    buttons->addWidget(m_from);
    buttons->addWidget(m_into);
    buttons->addWidget(m_copy);
    buttons->addWidget(m_models);
    for (QToolButton *b : {m_from, m_into, m_copy, m_models})
        if (!b->icon().isNull())   // without the program's icons the words stay
            b->setToolButtonStyle(Qt::ToolButtonIconOnly);
    g->addLayout(buttons, 2, 0, 1, 2);
    auto *pair = new QHBoxLayout;
    pair->addWidget(m_swap);
    pair->addWidget(m_backButton);
    g->addLayout(pair, 3, 0, 1, 2);
    g->addWidget(m_progress, 4, 0, 1, 2);
    g->addWidget(m_status, 5, 0, 1, 2);
    // room for two lines of status: the splitter must not cut the second one off
    m_status->setMinimumHeight(2 * m_status->fontMetrics().lineSpacing() + 2);
    m_status->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 1);

    auto *out = new QWidget(this);
    auto *ol = new QVBoxLayout(out);
    ol->setContentsMargins(0, 0, 0, 0);
    ol->addWidget(m_output, 1);
    ol->addWidget(m_back);
    ol->addWidget(m_notes);

    m_split = new QSplitter(Qt::Vertical, this);
    m_split->addWidget(m_input);
    m_split->addWidget(controls);
    m_split->addWidget(out);
    m_split->addWidget(m_info);
    m_split->setCollapsible(1, false);
    m_split->setStretchFactor(0, 2);
    m_split->setStretchFactor(1, 0);
    m_split->setStretchFactor(2, 3);
    m_split->setStretchFactor(3, 1);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(2, 2, 2, 2);
    l->addWidget(m_notice);
    l->addWidget(m_split);
}

void Panel::buildWindowLayout()
{
    m_go->setIcon(themed(QStringLiteral("go-next")));
    m_go->setText(m_go->icon().isNull() ? QStringLiteral("→") : QString());
    m_go->setIconSize(QSize(32, 32));
    m_go->setFixedSize(56, 48);
    auto *top = new QHBoxLayout;
    top->addWidget(m_task);
    top->addWidget(m_style);
    top->addWidget(m_model);
    top->addWidget(m_models);
    top->addWidget(m_swap);
    top->addWidget(m_backButton);
    top->addStretch(1);
    top->addWidget(m_from);
    top->addWidget(m_into);
    top->addWidget(m_copy);

    auto *middle = new QWidget(this);
    auto *ml = new QVBoxLayout(middle);
    ml->setContentsMargins(4, 0, 4, 0);
    ml->addStretch(1);
    ml->addWidget(m_go, 0, Qt::AlignHCenter);
    ml->addWidget(m_live, 0, Qt::AlignHCenter);
    ml->addWidget(m_stop, 0, Qt::AlignHCenter);
    ml->addStretch(1);

    auto *out = new QWidget(this);
    auto *ol = new QVBoxLayout(out);
    ol->setContentsMargins(0, 0, 0, 0);
    ol->addWidget(m_output, 1);
    ol->addWidget(m_back);
    ol->addWidget(m_notes);

    m_split = new QSplitter(Qt::Horizontal, this);
    m_split->addWidget(m_input);
    m_split->addWidget(middle);
    m_split->addWidget(out);
    m_split->setCollapsible(1, false);
    m_split->setStretchFactor(0, 1);
    m_split->setStretchFactor(1, 0);
    m_split->setStretchFactor(2, 1);

    // the information area under both, the width of the window
    auto *outer = new QSplitter(Qt::Vertical, this);
    outer->setObjectName(QStringLiteral("tpInfoSplit"));
    outer->addWidget(m_split);
    outer->addWidget(m_info);
    outer->setStretchFactor(0, 4);
    outer->setStretchFactor(1, 1);
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(6, 6, 6, 6);
    l->addWidget(m_notice);
    l->addLayout(top);
    l->addWidget(outer, 1);
    l->addWidget(m_progress);
    l->addWidget(m_status);
}

void Panel::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    // the window: input and result the same width at first
    if (m_place == Place::Window && !m_sized && m_split && width() > 200) {
        m_sized = true;
        const int middle = 72;
        const int half = qMax(100, (m_split->width() - middle) / 2);
        m_split->setSizes({half, middle, half});
    }
}

void Panel::reloadSettings()
{
    updateModelBox();
    updateNotice();
}

QStringList Panel::missingParts() const
{
    QStringList missing;
    const Direction d = directionNow();
    if (d == Direction::Proofread) {
        // "Automatic" with no text yet: a writer who only translates is not told about the proofreading model
        const bool anyModel = !m_tool->chosenModel(QStringLiteral("corrector")).isEmpty()
                              || !m_tool->chosenModel(QStringLiteral("translator")).isEmpty()
                              || !m_tool->chosenModel(QStringLiteral("translator-en")).isEmpty();
        if (currentTask() == QLatin1String("auto") && inputText().trimmed().isEmpty()) {
            if (!anyModel)
                missing << tr("a proofreading or translation model");
        } else if (m_tool->chosenModel(QStringLiteral("corrector")).isEmpty()) {
            missing << tr("a proofreading model");
        }
        return missing;
    }
    if (m_tool->chosenModel(kindOf(d)).isEmpty())
        missing << (d == Direction::DeToEn ? tr("a translation model (German to English)")
                                           : tr("a translation model (English to German)"));
    if (m_tool->dictionaryFile().isEmpty())
        missing << tr("the German dictionary");
    return missing;
}

void Panel::updateNotice()
{
    if (!m_notice)
        return;
    const QStringList missing = missingParts();
    m_notice->setVisible(!missing.isEmpty());
    if (!missing.isEmpty())
        m_noticeText->setText(tr("Missing: %1. Download %2 in the add-on's options (File > Options > Proofreading).")
                                  .arg(missing.join(tr(" and ")), missing.size() > 1 ? tr("them") : tr("it")));
}

void Panel::updateModelBox()
{
    const Direction d = directionNow();
    m_modeKind = kindOf(d);
    const bool translating = d != Direction::Proofread;
    m_model->clear();
    const QString chosen = m_tool->chosenModel(m_modeKind);
    for (const CatalogModel &m : m_tool->store()->models(m_modeKind))
        if (m_tool->store()->isInstalled(m.id))
            m_model->addItem(m.title, m.id);
    if (m_model->count() == 0) {
        m_model->addItem(translating ? tr("No translation model yet") : tr("No proofreading model yet"), QString());
        m_model->setEnabled(false);
    } else {
        m_model->setEnabled(true);
        m_model->setCurrentIndex(qMax(0, m_model->findData(chosen)));
    }
    m_model->setToolTip(translating ? tr("The translation model (choose and download models in the options)")
                                    : tr("The proofreading model (choose and download models in the options)"));
}

void Panel::updateModeUi()
{
    const Direction d = directionNow();
    const bool translating = d != Direction::Proofread;
    // the style list ("Correct", "More Formal") belongs to proofreading only
    m_style->setVisible(!translating);
    if (m_controls) {
        m_controls->removeWidget(m_model);
        m_controls->addWidget(m_model, 1, translating ? 0 : 1, 1, translating ? 2 : 1);
    }
    if (kindOf(d) != m_modeKind)
        updateModelBox();
    // the buttons: Swap always; Translate back after a translation
    m_swap->setEnabled(!m_busy);
    m_backButton->setEnabled(!m_busy && m_dir != Direction::Proofread && !m_units.isEmpty());
    updateNotice();
}

QString Panel::currentTask() const
{
    return m_task->currentData().toString();
}

QString Panel::stylePrefix() const
{
    return m_styleId == QLatin1String("formal") ? kFormal : kCorrect;
}

void Panel::setInput(const QString &text)
{
    m_input->setPlainText(text);
    m_liveTimer.stop();   // the caller runs when it wants a result
}

QString Panel::inputText() const
{
    return m_input->toPlainText();
}

QString Panel::status() const
{
    return m_status->text();
}

QString Panel::notes() const
{
    return m_notes->text();
}

void Panel::setStatus(const QString &text)
{
    m_status->setText(text);
}

void Panel::setBusy(bool busy)
{
    m_busy = busy;
    m_go->setEnabled(!busy);
    m_stop->setVisible(busy);
    m_progress->setVisible(busy);
    m_task->setEnabled(!busy);
    m_style->setEnabled(!busy);
    m_from->setEnabled(!busy);
    m_swap->setEnabled(!busy);
    m_backButton->setEnabled(!busy && m_dir != Direction::Proofread && !m_units.isEmpty());
}

void Panel::run()
{
    m_liveTimer.stop();
    runNow(false);
}

QString Panel::systemFor(const QString &what)
{
    if (what == QLatin1String("translate"))
        return QStringLiteral(
            "You translate German fiction into natural, idiomatic American English. Translate the user's text "
            "faithfully: keep its meaning, tone, tense and paragraph breaks, and keep dialogue as dialogue with the "
            "same quotation marks. Do not add, explain or leave out anything. Answer with the translation only.");
    if (what == QLatin1String("translate-de"))
        return QStringLiteral(
            "You translate English fiction into natural, idiomatic German. Translate the user's text faithfully: keep "
            "its meaning, tone, tense and paragraph breaks, and keep dialogue as dialogue with the same quotation "
            "marks. Do not add, explain or leave out anything. Answer with the translation only.");
    if (what == QLatin1String("formal"))
        return QStringLiteral(
            "You are an editor. Rewrite the user's English text in a more formal tone. Keep its meaning, its facts and "
            "its dialogue as dialogue. Do not explain. Answer with the rewritten text only.");
    if (what == QLatin1String("paraphrase"))
        return QStringLiteral(
            "You are an editor. Repeat the user's English sentence; you may choose other words, but keep its meaning, "
            "tone and length. Answer with the sentence only.");
    return QStringLiteral(
        "You are a careful proofreader of English fiction. Correct only clear errors of spelling, grammar and "
        "punctuation in the user's text, in American English. Keep the author's words, style, tone and sentence "
        "structure. Keep dialogue as written, including informal speech, slang, contractions and sentence fragments. "
        "Do not add, move or remove quotation marks. Do not rephrase, explain or comment. If there is nothing to "
        "correct, repeat the text unchanged. Answer with the text only.");
}

QStringList Panel::examplesFor(const QString &what)
{
    // One turn each that shows the work: a small model then corrects real errors instead of repeating the text, and
    // leaves dialogue alone (texts of our own; the models get plain quotes)
    if (what == QLatin1String("translate"))
        return {QStringLiteral("\"Kommst du mit?\", fragte sie. Er schüttelte den Kopf und sah weg."),
                QStringLiteral("\"Are you coming?\" she asked. He shook his head and looked away.")};
    if (what == QLatin1String("translate-de"))
        return {QStringLiteral("\"Are you coming?\" she asked. He shook his head and looked away."),
                QStringLiteral("\"Kommst du mit?\", fragte sie. Er schüttelte den Kopf und sah weg.")};
    if (what == QLatin1String("formal"))
        return {QStringLiteral("We gotta leave now, it's getting real late."),
                QStringLiteral("We have to leave now; it is getting very late.")};
    if (what == QLatin1String("paraphrase"))
        return {};
    return {QStringLiteral("Their was no answer, so she knocked again and waited for there reply."),
            QStringLiteral("There was no answer, so she knocked again and waited for their reply."),
            QStringLiteral("\"Ain't nobody home,\" he said. \"Gone. All of 'em.\""),
            QStringLiteral("\"Ain't nobody home,\" he said. \"Gone. All of 'em.\""),
            QStringLiteral("He walked passed the bakery and didnt look back."),
            QStringLiteral("He walked past the bakery and didn't look back.")};
}

bool Panel::chatModel(const QString &modelId) const
{
    const CatalogModel *m = m_tool->store()->model(modelId);
    return m && m->type == QLatin1String("chat");
}

void Panel::liveTyped()
{
    if (isLive())
        m_liveTimer.start();
}

bool Panel::isLive() const
{
    return m_live && m_live->isChecked();
}

void Panel::setLive(bool on)
{
    m_live->setChecked(on);
}

void Panel::runNow(bool live)
{
    if (m_busy) {
        if (live) {   // typed on while a run was busy: it stops after the sentences being worked on, then runs again
            m_rerun = true;
            if (m_cancel)
                m_cancel->store(true);
        }
        return;
    }
    m_liveRun = live;
    const QString text = inputText();
    if (text.trimmed().isEmpty()) {
        if (live) {
            m_units.clear();
            m_single = SingleWord();
            m_output->clear();
            m_notes->setVisible(false);
            setStatus(tr("Ready."));
        } else {
            setStatus(tr("There is no text to work on yet."));
        }
        emit finished();
        return;
    }
    m_dir = directionFor(currentTask(), text);
    m_styleId = m_style->currentData().toString();
    const QString kind = slot();
    const Rules rules = rulesForRun();
    m_variants.clear();
    m_variantsPending = false;
    ++m_variantsRequest;
    updateModeUi();
    // the writer's decisions on the sentences that stay as they were
    QHash<QString, QList<Segment>> previous;
    for (const Unit &u : std::as_const(m_units))
        previous.insert(u.original, u.segments);

    // a single English word: corrected by the word list, with every suggestion (no model needed)
    QString single;
    if (proofreading() && isSingleWord(text, &single)) {
        m_plan = makePlan(text.trimmed(), false);
        m_units.clear();
        m_noteList.clear();
        Unit u;
        u.original = text.trimmed();
        u.modelInput = toPlainQuotes(u.original);
        m_units << u;
        finishSingle();
        return;
    }

    // 1.3: a single German or English word: the dictionary first, then the small model
    if (!proofreading() && isSingleWord(text, &single)) {
        runSingleTranslation(single);
        return;
    }
    // 1.3: up to three words never go to the large model
    const bool shortText = !proofreading() && text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size() <= 3;
    const QString modelId = shortText ? m_tool->wordModel(kind) : m_tool->chosenModel(kind);
    if (modelId.isEmpty()) {
        if (shortText && !m_tool->chosenModel(kind).isEmpty())
            setStatus(tr("A word or a short text is translated by the small model, never by the large one, and the small "
                         "model is not downloaded: choose %1 with Models... (File > Options > Proofreading).")
                          .arg(kind == QLatin1String("translator") ? tr("OPUS-MT German to English")
                                                                   : tr("OPUS-MT English to German (small)")));
        else
            setStatus(!proofreading() ? tr("No translation model is downloaded yet. Choose one with Models... (File > "
                                           "Options > Proofreading).")
                                      : tr("No proofreading model is downloaded yet. Choose one with Models... (File > "
                                           "Options > Proofreading)."));
        emit finished();
        return;
    }
    QString error;
    EngineInterface *engine = m_tool->engine(&error);
    if (!engine) {
        setStatus(tr("The engine cannot be used: %1").arg(error));
        emit finished();
        return;
    }
    // 1.1: never a model the computer has no room for (it would make the system swap or stop programs)
    const QString memory = m_tool->loadProblem(kind, modelId);
    if (!memory.isEmpty()) {
        setStatus(memory);
        emit finished();
        return;
    }
    m_plan = makePlan(text, !proofreading());
    m_units.clear();
    m_noteList.clear();
    m_single = SingleWord();
    int dashes = 0;
    // 1.4: the translation model learned German with capitals: with the dictionary the nouns of the copy it gets are
    // capitalized (the writer's text stays as it is); without it a note says so
    bool capitals = false;
    if (m_dir == Direction::DeToEn) {
        const QString dict = m_tool->dictionaryFile();
        capitals = !dict.isEmpty() && ensureGermanDictionary(dict);
        if (!capitals)
            m_noteList << tr("German nouns need capital letters for a good translation. The German dictionary (File > Options > "
                             "Proofreading, Models...) sets them in the copy the model gets; it is not available.");
    }
    for (const QString &u : std::as_const(m_plan.units)) {
        Unit unit;
        unit.original = u;
        int n = 0;
        unit.modelInput = toPlainQuotes(beforeModel(u, rules, &n));
        if (capitals)
            unit.modelInput = correctGermanCapitals(unit.modelInput);
        dashes += n;
        m_units << unit;
    }
    // keep the decisions for later (finishRun compares)
    for (Unit &u : m_units)
        if (previous.contains(u.original))
            u.segments = previous.value(u.original);
    if (dashes)
        m_noteList << (dashes == 1 ? tr("1 dash replaced by a comma (rule: no em-dashes): click it to keep the dash.")
                                   : tr("%1 dashes replaced (rule: no em-dashes): click one to keep the dash.").arg(dashes));

    RunOptions o;
    const bool chat = chatModel(modelId);
    if (chat) {
        o.system = systemFor(workKey());
        o.examples = examplesFor(workKey());
        o.batch = 4;
    } else {
        o.prefix = proofreading() ? stylePrefix() : QString();
        o.batch = 8;
    }
    // beam 4: with 2 the large corrector turned "bellow" into "downstairs" (test of 2026-11-15)
    o.beam = m_tool->setting(QStringLiteral("careful"), false).toBool() ? 6 : 4;
    o.banDashes = rules.noEmDash;
    // the sentences worked on before (same model, same instruction) come from the cache
    m_runKey = modelId + QLatin1Char('\x1f') + o.prefix + o.system + o.examples.join(QLatin1Char('\x1e')) + QLatin1Char('\x1f') + (o.banDashes ? u'1' : u'0')
               + QString::number(o.beam);
    m_sent.clear();
    QStringList modelTexts;
    for (int i = 0; i < m_units.size(); ++i)
        if (!m_cache.contains(m_runKey + QLatin1Char('\x1f') + m_units.at(i).modelInput)) {
            m_sent << i;
            modelTexts << m_units.at(i).modelInput;
        }
    if (modelTexts.isEmpty()) {
        finishRun({}, QString(), 0.0, false);
        return;
    }
    const ModelFiles files = m_tool->store()->files(modelId);
    const int threads = m_tool->threads();
    const QString slotName = kind;
    m_cancel = std::make_shared<std::atomic_bool>(false);
    auto cancel = m_cancel;
    setBusy(true);
    m_progress->setRange(0, 0);
    const QString title = m_tool->store()->model(modelId) ? m_tool->store()->model(modelId)->title : modelId;
    setStatus(engine->loaded(slotName) == modelId ? tr("Working...") : tr("Loading %1...").arg(title));
    QPointer<Panel> self(this);
    QThread *t = QThread::create([self, engine, files, threads, slotName, modelTexts, o, cancel] {
        TranslateTool::lowerPriority();
        QElapsedTimer clock;
        clock.start();
        QString error;
        QStringList results;
        RunOptions opts = o;
        opts.cancel = cancel.get();
        if (engine->load(slotName, files, threads, &error)) {
            results = engine->run(slotName, modelTexts, opts,
                                  [self, cancel](int done, int total) {
                                      if (self)
                                          QMetaObject::invokeMethod(self.data(), [self, done, total] {
                                              if (!self)
                                                  return;
                                              self->m_progress->setRange(0, total);
                                              self->m_progress->setValue(done);
                                              self->setStatus(tr("Working: %1 of %2 sentences...").arg(done).arg(total));
                                          }, Qt::QueuedConnection);
                                      return !cancel->load();
                                  },
                                  &error);
        }
        const double seconds = clock.elapsed() / 1000.0;
        const bool stopped = cancel->load();
        if (self)
            QMetaObject::invokeMethod(self.data(), [self, results, error, seconds, stopped] {
                if (self)
                    self->finishRun(results, error, seconds, stopped);
            }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void Panel::stop()
{
    m_rerun = false;
    m_liveTimer.stop();
    if (m_cancel)
        m_cancel->store(true);
    setStatus(tr("Stopping after the sentences being worked on..."));
}

void Panel::finishRun(const QStringList &sentResults, const QString &error, double seconds, bool stopped)
{
    setBusy(false);
    m_tool->touchEngine();
    if (!error.isEmpty() || sentResults.size() != m_sent.size()) {
        setStatus(tr("The work failed: %1").arg(error.isEmpty() ? tr("no result") : error));
        m_rerun = false;
        emit finished();
        return;
    }
    // the results of the model and those from the cache
    QStringList results;
    for (int i = 0; i < m_units.size(); ++i)
        results << m_cache.value(m_runKey + QLatin1Char('\x1f') + m_units.at(i).modelInput);
    for (int k = 0; k < m_sent.size(); ++k) {
        const QString &r = sentResults.at(k);
        results[m_sent.at(k)] = r;
        if (!r.isEmpty())
            m_cache.insert(m_runKey + QLatin1Char('\x1f') + m_units.at(m_sent.at(k)).modelInput, r);
    }
    if (m_cache.size() > 5000)
        m_cache.clear();
    const Rules rules = rulesForRun();
    int kept = 0;
    for (int i = 0; i < m_units.size(); ++i) {
        Unit &u = m_units[i];
        const QList<Segment> before = u.segments;   // the writer's decisions of the run before, if any
        QString raw = results.at(i);
        if (raw.isEmpty()) // stopped before this sentence
            raw = u.modelInput;
        u.raw = raw;
        u.flag.clear();
        if (proofreading() && m_styleId == QLatin1String("correct")) {
            if (changedTooMuch(u.modelInput, raw)) {
                u.flag = tr("The model rewrote this sentence too much; your sentence was kept. (It wrote: %1)").arg(raw);
                u.raw = u.modelInput;
                ++kept;
            } else if (newDoubledWord(u.modelInput, raw)) {
                u.flag = tr("The model repeated a word; your sentence was kept. (It wrote: %1)").arg(raw);
                u.raw = u.modelInput;
                ++kept;
            }
        }
        QString shown = afterModel(u.raw, rules, &m_noteList);
        if (proofreading()) {
            // 1.1: the writer's quotation marks and dialogue punctuation where the writer had them (and curly, if
            // the rule says so: from their places, the direction is clear)
            shown = keepDialogue(u.original, shown);
            if (rules.curlyQuotes)
                shown = toCurlyQuotes(toPlainQuotes(shown));
            u.segments = compareWords(u.original, shown);
            // the same suggestions as in the run before: the writer's choices stay
            if (before.size() == u.segments.size()) {
                bool same = true;
                for (int s = 0; s < before.size() && same; ++s)
                    same = before.at(s).before == u.segments.at(s).before && before.at(s).after == u.segments.at(s).after;
                if (same)
                    u.segments = before;
            }
        } else {
            Segment s;
            s.before = s.after = words(tidyPunctuation(shown));
            u.segments = {s};
        }
    }
    if (kept)
        m_noteList << (kept == 1 ? tr("1 sentence kept as you wrote it: the model's version did not look right "
                                      "(point at the sentence to see what it wrote).")
                                 : tr("%1 sentences kept as you wrote them: the model's versions did not look right "
                                      "(point at a sentence to see what it wrote).").arg(kept));
    render();
    const QString done = m_sent.isEmpty() ? tr("Done") : tr("Done in %1 s").arg(seconds, 0, 'f', 1);
    if (stopped)
        setStatus(m_rerun ? tr("Working on your changes...")
                          : tr("Stopped. The sentences done so far are shown; the rest is as you wrote it."));
    else if (proofreading())
        setStatus(changeCount() == 0 ? tr("%1: no changes.").arg(done)
                  : changeCount() == 1 ? tr("%1: 1 change. Click it for the original and other words.").arg(done)
                                       : tr("%1: %2 changes. Click one for the original and other words.")
                                             .arg(done)
                                             .arg(changeCount()));
    else
        setStatus(tr("Translated (%1). Click a word for other words.").arg(m_sent.isEmpty() ? tr("at once")
                                                                                            : tr("%1 s").arg(seconds, 0, 'f', 1)));
    updateModeUi();
    emit finished();
    if (m_rerun) {
        m_rerun = false;
        QTimer::singleShot(0, this, [this] { runNow(true); });
    } else if (!proofreading() && !stopped && m_units.size() == 1 && words(m_units.at(0).original).size() >= 2) {
        requestVariants();
    }
}

void Panel::finishSingle()
{
    // the word as it should be: as typed when the dictionary knows it, else the nearest word of the dictionary
    Unit &u = m_units[0];
    QString word;
    isSingleWord(u.original, &word);
    m_single = SingleWord();
    m_single.on = true;
    m_single.kind = QStringLiteral("proofread");
    m_single.word = word;
    const bool known = knownWord(word);
    m_single.spelling = known ? QStringList() : spellingSuggestions(word, 12);
    m_single.result = known || m_single.spelling.isEmpty() ? word : m_single.spelling.first();
    m_single.similar = similarWords(m_single.result, &m_single.base);
    m_single.meanings = englishMeanings(m_single.result);   // 1.3: what the word means
    QString shown = u.original;
    shown.replace(word, m_single.result);
    u.raw = toPlainQuotes(shown);
    u.segments = compareWords(u.original, shown);
    render();
    if (known)
        setStatus(tr("“%1” is spelled correctly. Below: words of a similar meaning.").arg(word));
    else if (m_single.spelling.isEmpty())
        setStatus(tr("“%1” is not in the dictionary, and no word near it is.").arg(word));
    else
        setStatus(tr("“%1” is not in the dictionary: “%2” is the nearest word. Below: the other suggestions.")
                      .arg(word, m_single.result));
    requestChatMeaning();
    render();
    requestSingleModelWords();
    emit finished();
}

void Panel::requestSingleModelWords()
{
    // the model's other words for the word (proofreading: the corrector's; translation: other translations)
    AltJob job;
    QString jobError;
    if (!m_single.on || m_units.isEmpty() || !outputJob(0, 0, &job, &jobError))
        return;
    m_single.modelPending = true;
    const int request = ++m_singleRequest;
    const QString current = words(job.current).value(0);
    QPointer<Panel> self(this);
    QThread *t = QThread::create([self, job, request, current] {
        TranslateTool::lowerPriority();
        QString error;
        const QList<Alternative> found = runAltJob(job, &error);
        QMetaObject::invokeMethod(qApp, [self, found, request, current] {
            if (!self || request != self->m_singleRequest)
                return;
            self->m_tool->touchEngine();
            self->m_single.modelPending = false;
            QStringList words;
            for (const Alternative &a : self->filterAlternatives(found, current))
                words << a.word;
            if (self->proofreading()) {
                // proofreading a word alone: only the model's words close to it in spelling or meaning
                QStringList near = self->m_single.spelling;
                near << self->m_single.word;
                for (const Similar &sim : std::as_const(self->m_single.similar))
                    near << sim.words;
                words = usefulModelWords(words, self->m_single.result, false, &near);
            } else {
                words = usefulModelWords(words, self->m_single.result);
            }
            for (const QString &w : std::as_const(words))
                if (w.compare(self->m_single.word, Qt::CaseInsensitive) != 0 && !self->m_single.modelWords.contains(w))
                    self->m_single.modelWords << w;
            self->render();
        }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void Panel::linkClicked(const QString &href)
{
    if (m_units.isEmpty())
        return;
    if (href.startsWith(QLatin1String("tp-pick:"))) {   // 1.3: a word of the dictionary
        const QString rest = href.mid(8);
        const int colon = rest.indexOf(QLatin1Char(':'));
        pickWord(QUrl::fromPercentEncoding(rest.mid(colon + 1).toUtf8()), rest.left(colon));
        return;
    }
    if (href.startsWith(QLatin1String("tp-more:"))) {
        m_single.expanded << href.mid(8);
        render();
        return;
    }
    if (href.startsWith(QLatin1String("tp-variant:"))) {   // 1.3: another complete translation
        const int i = href.mid(11).toInt();
        if (i >= 0 && i < m_variants.size()) {
            const QString v = m_variants.at(i);
            m_variants[i] = unitText(0);
            chooseOutputAlternative(0, Alternative{QString(), toPlainQuotes(v)});
        }
        return;
    }
    // a word of the suggestions under a single word: it becomes the result
    if (!href.startsWith(QLatin1String("tp-word:")))
        return;
    const QString w = QUrl::fromPercentEncoding(href.mid(8).toUtf8());
    QString shown = m_units.at(0).original;
    shown.replace(m_single.word, w);
    chooseOutputAlternative(0, Alternative{w, toPlainQuotes(shown)});
    m_single.result = w;
    m_single.meanings = englishMeanings(w);
    requestChatMeaning();
    render();
}

int Panel::changeCount() const
{
    int n = 0;
    for (const Unit &u : m_units)
        for (const Segment &s : u.segments)
            n += s.changed ? 1 : 0;
    return n;
}

QString Panel::unitText(int unit) const
{
    return segmentsText(m_units.value(unit).segments);
}

QString Panel::resultText() const
{
    QStringList texts;
    for (int i = 0; i < m_units.size(); ++i)
        texts << unitText(i);
    return m_units.isEmpty() ? QString() : joinPlan(m_plan, texts);
}

void Panel::render()
{
    const int scroll = m_output->verticalScrollBar()->value();
    QTextDocument *doc = m_output->document();
    doc->clear();
    QTextCursor c(doc);
    const QPalette pal = m_output->palette();
    QTextCharFormat plain;
    plain.setForeground(pal.color(QPalette::Text));
    QTextCharFormat changed = plain;
    QColor mark(255, 196, 0, 110);
    changed.setBackground(mark);
    QTextCharFormat kept = plain;
    kept.setUnderlineStyle(QTextCharFormat::DotLine);
    kept.setUnderlineColor(QColor(230, 150, 0));
    QTextCharFormat flagged = plain;
    flagged.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    flagged.setUnderlineColor(QColor(150, 150, 150));

    for (int p = 0; p < m_plan.paragraphs.size(); ++p) {
        if (p > 0)
            c.insertBlock();
        const QList<int> &idx = m_plan.paragraphs.at(p);
        if (idx.isEmpty()) {
            c.insertText(m_plan.emptyParagraphs.value(p), plain);
            continue;
        }
        for (int k = 0; k < idx.size(); ++k) {
            const int u = idx.at(k);
            const Unit &unit = m_units.at(u);
            if (k > 0)
                c.insertText(QStringLiteral(" "), plain);
            int word = 0;
            bool firstWord = true;
            for (int s = 0; s < unit.segments.size(); ++s) {
                const Segment &seg = unit.segments.at(s);
                const QStringList shown = seg.changed && !seg.accepted ? seg.before : seg.after;
                QTextCharFormat f = !seg.changed ? (unit.flag.isEmpty() ? plain : flagged) : seg.accepted ? changed : kept;
                f.setProperty(UnitProp, u);
                f.setProperty(SegmentProp, seg.changed ? s : -1);
                if (seg.changed) {
                    f.setToolTip(seg.accepted ? (seg.before.isEmpty() ? tr("Added. Click to leave it out.")
                                                                      : tr("Was: %1").arg(seg.before.join(QLatin1Char(' '))))
                                              : tr("Suggested: %1").arg(seg.after.isEmpty() ? tr("(leave out)")
                                                                                             : seg.after.join(QLatin1Char(' '))));
                } else if (!unit.flag.isEmpty()) {
                    f.setToolTip(unit.flag);
                }
                if (shown.isEmpty()) {
                    // a word left out (or an added one not taken): a small mark to click
                    if (!firstWord)
                        c.insertText(QStringLiteral(" "), plain);
                    QTextCharFormat m = f;
                    m.setProperty(WordProp, -1);
                    c.insertText(QStringLiteral("‸"), m);
                    firstWord = false;
                    continue;
                }
                for (const QString &w : shown) {
                    if (!firstWord) {
                        QTextCharFormat space = plain;
                        space.setProperty(UnitProp, u);
                        c.insertText(QStringLiteral(" "), space);
                    }
                    QTextCharFormat wf = f;
                    wf.setProperty(WordProp, word++);
                    c.insertText(w, wf);
                    firstWord = false;
                }
            }
        }
    }
    if (m_single.on)
        renderSingle(c, plain);
    if (!m_single.on && !m_variants.isEmpty()) {
        // 1.3: other complete translations of a short text
        QTextCharFormat head = plain;
        head.setForeground(pal.color(QPalette::PlaceholderText));
        c.insertBlock();
        c.insertBlock();
        c.insertText(tr("Other translations:"), head);
        for (int i = 0; i < m_variants.size(); ++i) {
            c.insertBlock();
            QTextCharFormat l = plain;
            l.setAnchor(true);
            l.setAnchorHref(QStringLiteral("tp-variant:%1").arg(i));
            l.setForeground(pal.color(QPalette::Link));
            l.setToolTip(tr("Use this translation"));
            c.insertText(tr("Use: %1").arg(m_variants.at(i)), l);
        }
    } else if (!m_single.on && m_variantsPending) {
        QTextCharFormat head = plain;
        head.setForeground(pal.color(QPalette::PlaceholderText));
        c.insertBlock();
        c.insertBlock();
        c.insertText(tr("Looking for other translations..."), head);
    }
    m_output->verticalScrollBar()->setValue(scroll);
    m_notes->setText(m_noteList.join(QLatin1Char('\n')));
    m_notes->setVisible(!m_noteList.isEmpty());
}

Panel::WordInfo Panel::wordAt(int position) const
{
    WordInfo info;
    QTextCursor c(m_output->document());
    c.setPosition(position);
    if (!c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor))
        return info;
    const QTextCharFormat f = c.charFormat();
    if (!f.hasProperty(UnitProp))
        return info;
    info.unit = f.property(UnitProp).toInt();
    info.word = f.hasProperty(WordProp) ? f.property(WordProp).toInt() : -1;
    info.segment = f.hasProperty(SegmentProp) ? f.property(SegmentProp).toInt() : -1;
    if (info.word >= 0)
        info.text = words(unitText(info.unit)).value(info.word);
    return info;
}

void Panel::setSegmentAccepted(int unit, int segment, bool accepted)
{
    if (unit < 0 || unit >= m_units.size() || segment < 0 || segment >= m_units[unit].segments.size())
        return;
    m_units[unit].segments[segment].accepted = accepted;
    render();
}

void Panel::setAllAccepted(bool accepted)
{
    for (Unit &u : m_units)
        for (Segment &s : u.segments)
            s.accepted = accepted;
    render();
}

bool Panel::realWord(const QString &word) const
{
    QString letters;
    for (QChar ch : word)
        if (ch.isLetter() || ch == QLatin1Char('\'') || ch == QChar(0x2019))
            letters += ch == QChar(0x2019) ? QLatin1Char('\'') : ch;
    while (letters.endsWith(QLatin1Char('\'')))
        letters.chop(1);
    if (letters.isEmpty())
        return false;
    // a single letter is a piece of a word, except "a" and "I"
    if (letters.size() == 1 && letters != QLatin1String("a") && letters != QLatin1String("A") && letters != QLatin1String("I"))
        return false;
    const QSet<QString> &list = wordList();
    if (list.isEmpty())
        return true; // no list: nothing can be judged
    if (list.contains(letters) || list.contains(letters.toLower()))
        return true;
    // "don't", "she'll": the part before the apostrophe
    const int apo = letters.indexOf(QLatin1Char('\''));
    return apo > 0 && list.contains(letters.left(apo).toLower());
}

bool Panel::outputJob(int unit, int word, AltJob *job, QString *error)
{
    if (unit < 0 || unit >= m_units.size() || word < 0)
        return false;
    const QString modelId = m_tool->chosenModel(slot());
    if (modelId.isEmpty()) {
        if (error)
            *error = tr("no model downloaded");
        return false;
    }
    job->engine = m_tool->engine(error);
    if (!job->engine)
        return false;
    if (const QString memory = m_tool->loadProblem(slot(), modelId); !memory.isEmpty()) {
        if (error)
            *error = memory;
        return false;
    }
    job->slot = slot();
    job->files = m_tool->store()->files(modelId);
    job->threads = m_tool->threads();
    job->source = m_units.at(unit).modelInput;
    job->current = toPlainQuotes(unitText(unit));
    job->word = word;
    if (chatModel(modelId)) {
        job->options.system = systemFor(workKey());
        job->options.examples = examplesFor(workKey());
    } else
        job->options.prefix = proofreading() ? stylePrefix() : QString();
    job->options.banDashes = m_tool->rules().noEmDash;
    return true;
}

QList<Alternative> Panel::filterAlternatives(const QList<Alternative> &alts, const QString &currentWord) const
{
    QList<Alternative> out;
    QSet<QString> seen;
    seen.insert(currentWord.toLower());   // 1.4: also the main translation; capitals do not make another word
    for (const Alternative &a : alts) {
        if (a.word == currentWord || !realWord(a.word) || seen.contains(a.word.toLower()))
            continue;
        seen.insert(a.word.toLower());
        out << a;
        if (out.size() >= 6)
            break;
    }
    return out;
}

QList<Alternative> Panel::outputAlternatives(int unit, int word, QString *error)
{
    AltJob job;
    if (!outputJob(unit, word, &job, error))
        return {};
    const QList<Alternative> out = filterAlternatives(runAltJob(job, error), words(job.current).value(word));
    m_tool->touchEngine();
    return out;
}

void Panel::chooseOutputAlternative(int unit, const Alternative &a)
{
    if (unit < 0 || unit >= m_units.size())
        return;
    Unit &u = m_units[unit];
    u.raw = a.sentence;
    u.flag.clear();
    QStringList notes;
    QString shown = afterModel(a.sentence, rulesForRun(), &notes);
    if (proofreading()) {
        shown = keepDialogue(u.original, shown);
        if (rulesForRun().curlyQuotes)
            shown = toCurlyQuotes(toPlainQuotes(shown));
        u.segments = compareWords(u.original, shown);
    } else {
        Segment s;
        s.before = s.after = words(shown);
        u.segments = {s};
    }
    render();
}

void Panel::outputClicked(int position, const QPoint &globalPos)
{
    if (m_busy)
        return;
    const WordInfo info = wordAt(position);
    if (info.unit < 0)
        return;
    if (m_wordMenu)
        m_wordMenu->close();
    auto *menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("tpWordMenu"));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    m_wordMenu = menu;
    const Unit &unit = m_units.at(info.unit);
    if (info.segment >= 0) {
        const Segment &seg = unit.segments.at(info.segment);
        const QString before = seg.before.join(QLatin1Char(' ')), after = seg.after.join(QLatin1Char(' '));
        QAction *use = menu->addAction(after.isEmpty() ? tr("Leave Out “%1”").arg(before)
                                                       : tr("Use “%1”").arg(after));
        use->setCheckable(true);
        use->setChecked(seg.accepted);
        QAction *keep = menu->addAction(before.isEmpty() ? tr("Do Not Add “%1”").arg(after)
                                                         : tr("Keep “%1”").arg(before));
        keep->setCheckable(true);
        keep->setChecked(!seg.accepted);
        const int u = info.unit, s = info.segment;
        connect(use, &QAction::triggered, this, [this, u, s] { setSegmentAccepted(u, s, true); });
        connect(keep, &QAction::triggered, this, [this, u, s] { setSegmentAccepted(u, s, false); });
    }
    if (!unit.flag.isEmpty() && info.segment < 0) {
        QAction *why = menu->addAction(tr("Your Sentence Was Kept"));
        why->setEnabled(false);
        why->setToolTip(unit.flag);
    }
    if (info.word >= 0) {
        if (!menu->isEmpty())
            menu->addSeparator();
        QAction *wait = menu->addAction(tr("Looking for other words..."));
        wait->setEnabled(false);
        const int request = ++m_request;
        const int u = info.unit;
        AltJob job;
        QString jobError;
        const bool ok = outputJob(info.unit, info.word, &job, &jobError);
        const QString currentWord = words(job.current).value(info.word);
        // the original words of a change are offered by "Keep" already
        const QString original = info.segment >= 0 ? unit.segments.at(info.segment).before.join(QLatin1Char(' ')) : QString();
        QPointer<Panel> self(this);
        QPointer<QMenu> menuGuard(menu);
        // the model's answer takes up to a few seconds: the menu is open meanwhile and gets the words when they come
        QThread *t = QThread::create([self, job, ok, jobError, request, menuGuard, u, currentWord, original] {
            QString error = jobError;
            const QList<Alternative> found = ok ? runAltJob(job, &error) : QList<Alternative>();
            QMetaObject::invokeMethod(qApp, [self, found, error, request, menuGuard, u, currentWord, original] {
                if (!self || !menuGuard || request != self->m_request)
                    return;
                self->m_tool->touchEngine();
                QList<Alternative> alts = self->filterAlternatives(found, currentWord);
                alts.erase(std::remove_if(alts.begin(), alts.end(), [&](const Alternative &a) { return a.word == original; }),
                           alts.end());
                QMenu *m = menuGuard.data();
                if (QAction *last = m->actions().isEmpty() ? nullptr : m->actions().last(); last && !last->isEnabled())
                    m->removeAction(last);
                if (alts.isEmpty()) {
                    QAction *none = m->addAction(error.isEmpty() ? tr("No other words found") : tr("No other words: %1").arg(error));
                    none->setEnabled(false);
                }
                for (const Alternative &a : alts) {
                    QAction *ac = m->addAction(a.word);
                    ac->setToolTip(a.sentence);
                    QObject::connect(ac, &QAction::triggered, self.data(), [self, u, a] {
                        if (self)
                            self->chooseOutputAlternative(u, a);
                    });
                }
                m->adjustSize();
            }, Qt::QueuedConnection);
        });
        connect(t, &QThread::finished, t, &QObject::deleteLater);
        t->start();
    }
    if (menu->isEmpty()) {
        delete menu;
        return;
    }
    menu->setToolTipsVisible(true);
    menu->popup(globalPos);
}

void Panel::outputContextMenu(const QPoint &globalPos)
{
    QMenu menu(this);
    QAction *copySel = menu.addAction(tr("Copy Selection"), m_output, &QTextBrowser::copy);
    copySel->setEnabled(m_output->textCursor().hasSelection());
    menu.addAction(tr("Copy the Result"), this, &Panel::copyResult)->setEnabled(!m_units.isEmpty());
    if (proofreading() && changeCount() > 0) {
        menu.addSeparator();
        menu.addAction(tr("Use All Suggestions"), this, [this] { setAllAccepted(true); });
        menu.addAction(tr("Keep All Originals"), this, [this] { setAllAccepted(false); });
    }
    menu.exec(globalPos);
}

Panel::InputPlace Panel::inputPlace(int position) const
{
    InputPlace place;
    const QTextBlock block = m_input->document()->findBlock(position);
    if (!block.isValid())
        return place;
    const QString text = block.text();
    const int offset = position - block.position();
    int from = 0;
    for (const QString &s : splitSentences(text)) {
        const int start = text.indexOf(s, from);
        if (start < 0)
            continue;
        from = start + s.size();
        if (offset < start || offset > start + s.size())
            continue;
        place.block = block.blockNumber();
        place.start = start;
        place.length = s.size();
        place.sentence = s;
        // the word under the position: count the words before it
        const QString before = s.left(offset - start);
        const QStringList ws = words(s);
        int index = words(before).size();
        if (!before.isEmpty() && !before.at(before.size() - 1).isSpace() && index > 0)
            --index; // inside a word
        if (index >= 0 && index < ws.size()) {
            place.word = index;
            place.wordText = ws.at(index);
        }
        break;
    }
    // other words only for English text
    if (detectLanguage(text) == QLatin1String("de"))
        place.wordText.clear();
    return place;
}

QList<Alternative> Panel::inputAlternatives(const InputPlace &place, QString *error)
{
    if (place.word < 0)
        return {};
    const QString modelId = m_tool->chosenModel(QStringLiteral("corrector"));
    if (modelId.isEmpty()) {
        if (error)
            *error = tr("no proofreading model downloaded");
        return {};
    }
    AltJob job;
    job.engine = m_tool->engine(error);
    if (!job.engine)
        return {};
    if (const QString memory = m_tool->loadProblem(QStringLiteral("corrector"), modelId); !memory.isEmpty()) {
        if (error)
            *error = memory;
        return {};
    }
    job.slot = QStringLiteral("corrector");
    job.files = m_tool->store()->files(modelId);
    job.threads = m_tool->threads();
    job.source = job.current = toPlainQuotes(place.sentence);
    job.word = place.word;
    if (chatModel(modelId))
        job.options.system = systemFor(QStringLiteral("paraphrase"));
    else
        job.options.prefix = kParaphrase;
    job.options.banDashes = m_tool->rules().noEmDash;
    const QList<Alternative> out = filterAlternatives(runAltJob(job, error), words(job.current).value(place.word));
    m_tool->touchEngine();
    return out;
}

void Panel::chooseInputAlternative(const InputPlace &place, const Alternative &a)
{
    const QTextBlock block = m_input->document()->findBlockByNumber(place.block);
    if (!block.isValid() || block.text().mid(place.start, place.length) != place.sentence)
        return;
    QTextCursor c(block);
    c.setPosition(block.position() + place.start);
    c.setPosition(block.position() + place.start + place.length, QTextCursor::KeepAnchor);
    c.insertText(afterModel(a.sentence, m_tool->rules()));
}

void Panel::showInputAlternatives(int position, const QPoint &globalPos)
{
    const InputPlace place = inputPlace(position);
    if (place.word < 0)
        return;
    QApplication::setOverrideCursor(Qt::BusyCursor);
    setStatus(tr("Looking for other words for “%1”...").arg(place.wordText));
    QString error;
    const QList<Alternative> alts = inputAlternatives(place, &error);
    QApplication::restoreOverrideCursor();
    setStatus(alts.isEmpty() ? (error.isEmpty() ? tr("No other words found.") : tr("No other words: %1").arg(error))
                             : tr("Choose another word; the rest of the sentence is fitted to it."));
    if (alts.isEmpty())
        return;
    QMenu menu(this);
    menu.setObjectName(QStringLiteral("tpInputWordMenu"));
    menu.setToolTipsVisible(true);
    for (const Alternative &a : alts) {
        QAction *ac = menu.addAction(a.word);
        ac->setToolTip(a.sentence);
        connect(ac, &QAction::triggered, this, [this, place, a] { chooseInputAlternative(place, a); });
    }
    menu.exec(globalPos);
}

// ---------------------------------------------------------------- 1.3: a word on its own

namespace {

const int kFirstMeanings = 5;   // the meanings of a class shown at first; the rest is behind "more"

struct SingleJob {
    EngineInterface *engine = nullptr;
    Panel::Direction dir = Panel::Direction::DeToEn;
    QString word;
    QString dictFile;
    QString slot;
    ModelFiles files;
    int threads = 0;
    RunOptions options;
    bool modelOk = false;
    QString modelNote;      // why the small model cannot be used
};

// The work behind a single word, away from the window: the dictionary, then the small model for what the dictionary
// does not say. (Order of the writer's settlement: 1. base form, classes, meanings; 2. English words of a meaning;
// 3. class and meaning but no English word: the small model, in a frame for the class, with the warning; 4. not in the
// dictionary: the small model alone, with the warning.)
Panel::SingleWord computeSingle(const SingleJob &j, QString *error)
{
    using DictClass = Panel::DictClass;
    Panel::SingleWord w;
    w.on = true;
    w.word = j.word;
    const bool toEn = j.dir == Panel::Direction::DeToEn;
    w.kind = toEn ? QStringLiteral("de-en") : QStringLiteral("en-de");
    const QString from = toEn ? QStringLiteral("de") : QStringLiteral("en"), to = toEn ? QStringLiteral("en") : QStringLiteral("de");
    QStringList frames;
    QList<int> targets;   // the class a frame is for; -1: the word alone
    if (toEn) {
        if (!j.dictFile.isEmpty()) {
            QString e;
            if (!ensureGermanDictionary(j.dictFile, &e))
                w.note = QObject::tr("The German dictionary cannot be read (%1): download it again in File > Options > Proofreading.").arg(e);
            for (const GermanEntry &e : lookupGerman(j.word)) {
                DictClass dc;
                dc.headword = e.headword;
                dc.pos = e.pos;
                dc.gender = e.gender;
                for (const GermanMeaning &m : e.meanings)
                    dc.senses << Panel::DictSense{m.english, m.definition};
                w.classes << dc;
            }
            w.fromDictionary = !w.classes.isEmpty();
        } else {
            w.note = QObject::tr("The German dictionary is not downloaded (File > Options > Proofreading): it gives the word classes "
                                 "and meanings of a German word.");
        }
        for (int i = 0; i < w.classes.size(); ++i) {
            bool any = false;
            for (const Panel::DictSense &s : std::as_const(w.classes.at(i).senses))
                any = any || !s.english.isEmpty();
            if (!any) {
                frames << frameFor(from, w.classes.at(i).pos, w.classes.at(i).headword, w.classes.at(i).gender);
                targets << i;
            }
        }
        if (w.classes.isEmpty()) {
            // 1.4: the model learned German with capitals: a word it knows nothing about is translated in both forms
            // (a verb or adjective in lower case, a noun with a capital)
            QString otherForm = j.word;
            otherForm[0] = j.word.at(0).isUpper() ? j.word.at(0).toLower() : j.word.at(0).toUpper();
            frames << j.word;   // the form as typed first
            targets << -1;
            if (otherForm != j.word) {
                frames << otherForm;
                targets << -2;
            }
        }
    } else {
        QString base;
        w.meanings = englishMeanings(j.word, &base);
        for (const EnglishClass &ec : std::as_const(w.meanings)) {
            DictClass dc;
            dc.headword = base.isEmpty() ? j.word : base;
            dc.pos = ec.pos;
            w.classes << dc;
            frames << frameFor(from, ec.pos, dc.headword);
            targets << int(w.classes.size()) - 1;
        }
        if (w.classes.isEmpty()) {
            frames << j.word;
            targets << -1;
        }
    }
    QString bare, bareUpper;
    if (!frames.isEmpty()) {
        if (!j.modelOk) {
            w.note = w.note.isEmpty() ? j.modelNote : w.note + QLatin1Char(' ') + j.modelNote;
        } else if (!j.engine) {
            if (error)
                *error = QObject::tr("The engine cannot be used.");
        } else {
            QString e;
            QStringList answers;
            if (j.engine->load(j.slot, j.files, j.threads, &e))
                answers = j.engine->run(j.slot, frames, j.options, nullptr, &e);
            if (answers.size() != frames.size()) {
                if (error)
                    *error = e.isEmpty() ? QObject::tr("no result") : e;
            } else {
                w.fromModel = true;
                for (int i = 0; i < answers.size(); ++i) {
                    if (targets.at(i) >= 0)
                        w.classes[targets.at(i)].modelWord = unframe(to, w.classes.at(targets.at(i)).pos, answers.at(i));
                    else if (targets.at(i) == -2)
                        bareUpper = unframe(to, QString(), answers.at(i)).trimmed();
                    else
                    {
                        bare = unframe(to, QString(), answers.at(i));
                        // a German noun's capital letter is not the English word's: a name stays as it is
                        if (toEn && !j.word.isEmpty() && j.word.at(0).isUpper() && bare != j.word && bare.size() > 1
                            && bare.at(0).isUpper() && bare.at(1).isLower())
                            bare[0] = bare.at(0).toLower();
                    }
                }
            }
        }
    }
    // the word that stands as the result: the first English word of the first class that has one
    for (const DictClass &dc : std::as_const(w.classes)) {
        for (const Panel::DictSense &s : dc.senses)
            if (w.result.isEmpty() && !s.english.isEmpty()) {
                w.result = s.english.first();
                w.resultPos = dc.pos;
            }
        if (w.result.isEmpty() && !dc.modelWord.isEmpty()) {
            w.result = dc.modelWord;
            w.resultPos = dc.pos;
        }
        if (!w.result.isEmpty())
            break;
    }
    if (w.result.isEmpty())
        w.result = bare;
    // 1.4: the word in the other form: a second translation, as a class of its own (no dictionary entry told the class)
    if (!bareUpper.isEmpty() && bareUpper.compare(bare, Qt::CaseInsensitive) != 0) {
        DictClass dc;
        dc.headword = j.word.at(0).isUpper() ? j.word.at(0).toLower() + j.word.mid(1) : j.word.at(0).toUpper() + j.word.mid(1);
        dc.modelWord = bareUpper;
        w.classes << dc;
        if (w.result.isEmpty())
            w.result = bareUpper;
    }
    if (toEn && !w.result.isEmpty()) {
        w.similar = similarWords(bareVerb(w.result), &w.base, w.resultPos);
        // 1.4: similar translations of every class, not only of the first
        QSet<QString> shownPos;
        for (const Similar &s : std::as_const(w.similar))
            shownPos.insert(s.pos);
        for (const DictClass &dc : std::as_const(w.classes)) {
            if (dc.pos.isEmpty() || dc.pos == w.resultPos)
                continue;
            QString first = dc.modelWord;
            for (const Panel::DictSense &sn : dc.senses)
                if (first.isEmpty() && !sn.english.isEmpty())
                    first = sn.english.first();
            if (first.isEmpty())
                continue;
            for (const Similar &s : similarWords(bareVerb(first), nullptr, dc.pos))
                if (!shownPos.contains(s.pos)) {
                    shownPos.insert(s.pos);
                    w.similar << s;
                }
        }
    }
    if (!toEn) {
        // 1.4: similar German words: the German words whose meanings name the English word or one of its synonyms
        if (j.dictFile.isEmpty()) {
            w.note = w.note.isEmpty() ? QObject::tr("Similar German words need the German dictionary: download it in File > Options > Proofreading (Models...).")
                                      : w.note;
        } else if (ensureGermanDictionary(j.dictFile)) {
            QSet<QString> taken;
            auto bareGerman = [](QString g) {   // "der Fluch" and "zu fluchen" are the words Fluch and fluchen
                g = g.toLower();
                for (const QString &a : {QStringLiteral("der "), QStringLiteral("die "), QStringLiteral("das "), QStringLiteral("zu ")})
                    if (g.startsWith(a))
                        return g.mid(a.size());
                return g;
            };
            taken.insert(bareGerman(w.result));
            for (const DictClass &dc : std::as_const(w.classes))
                taken.insert(bareGerman(dc.modelWord));
            QSet<QString> doneClass;
            for (const DictClass &dc : std::as_const(w.classes)) {
                if (doneClass.contains(dc.pos))
                    continue;
                doneClass.insert(dc.pos);
                QStringList english{dc.headword};
                for (const Similar &s : similarWords(dc.headword, nullptr, dc.pos))
                    english << s.words;
                english.removeDuplicates();
                while (english.size() > 25)
                    english.removeLast();
                Similar sim;
                sim.pos = dc.pos;
                for (const GermanReverse &r : reverseLookupGerman(english, dc.pos, 20))
                    if (!taken.contains(r.headword.toLower())) {
                        taken.insert(r.headword.toLower());
                        sim.words << r.headword;
                    }
                if (!sim.words.isEmpty())
                    w.similar << sim;
            }
        }
    }
    return w;
}

} // namespace

void Panel::runSingleTranslation(const QString &word)
{
    SingleJob job;
    job.dir = m_dir;
    job.word = word;
    job.slot = slot();
    job.dictFile = m_tool->dictionaryFile();   // 1.4: also for English to German (the similar German words)
    const QString modelId = m_tool->wordModel(slot());
    job.modelOk = false;
    if (modelId.isEmpty()) {
        job.modelNote = m_tool->chosenModel(slot()).isEmpty()
                            ? tr("No translation model is downloaded yet. Choose one with Models... (File > Options > Proofreading).")
                            : tr("A single word is translated by the small model, never by the large one, and the small model is not "
                                 "downloaded: choose %1 with Models... (File > Options > Proofreading).")
                                  .arg(m_dir == Direction::DeToEn ? tr("OPUS-MT German to English") : tr("OPUS-MT English to German (small)"));
    } else {
        QString error;
        job.engine = m_tool->engine(&error);
        const QString memory = job.engine ? m_tool->loadProblem(slot(), modelId) : error;
        if (!job.engine || !memory.isEmpty()) {
            job.modelNote = job.engine ? memory : tr("The engine cannot be used: %1").arg(error);
        } else {
            job.modelOk = true;
            job.files = m_tool->store()->files(modelId);
            job.threads = m_tool->threads();
            job.options.beam = 4;
            job.options.banDashes = m_tool->rules().noEmDash;
            job.options.batch = 4;
            if (chatModel(modelId)) {
                job.options.system = systemFor(workKey());
                job.options.examples = examplesFor(workKey());
            }
        }
    }
    m_plan = makePlan(inputText().trimmed(), false);
    m_units.clear();
    m_noteList.clear();
    m_single = SingleWord();
    Unit u;
    u.original = inputText().trimmed();
    u.modelInput = toPlainQuotes(u.original);
    m_units << u;
    const int request = ++m_singleTranslationRequest;
    const bool needsModel = job.modelOk;
    if (needsModel) {
        setBusy(true);
        m_progress->setRange(0, 0);
        setStatus(tr("Looking the word up..."));
    }
    QPointer<Panel> self(this);
    QThread *t = QThread::create([self, job, request] {
        TranslateTool::lowerPriority();
        QString error;
        const SingleWord w = computeSingle(job, &error);
        QMetaObject::invokeMethod(qApp, [self, w, error, request] {
            if (self && request == self->m_singleTranslationRequest)
                self->finishSingleTranslation(w, error);
        }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void Panel::finishSingleTranslation(const SingleWord &w, const QString &error)
{
    setBusy(false);
    m_tool->touchEngine();
    if (!error.isEmpty() && w.result.isEmpty()) {
        setStatus(tr("The work failed: %1").arg(error));
        emit finished();
        return;
    }
    m_single = w;
    Unit &u = m_units[0];
    u.raw = toPlainQuotes(w.result);
    Segment s;
    s.before = s.after = words(w.result);
    u.segments = {s};
    if (m_single.kind == QLatin1String("en-de")) {
        requestChatMeaning();   // when WordNet has nothing
    }
    render();
    if (w.result.isEmpty())
        setStatus(w.note.isEmpty() ? tr("No translation found.") : w.note);
    else if (w.fromDictionary && !w.fromModel)
        setStatus(tr("From the dictionary. Click a word to use it."));
    else if (w.fromModel)
        setStatus(tr("Translated by the small model: it may be inaccurate."));
    else
        setStatus(tr("Done."));
    updateModeUi();
    emit finished();
}

void Panel::pickWord(const QString &word, const QString &pos)
{
    if (!m_single.on || m_units.isEmpty())
        return;
    m_single.result = word;
    m_single.resultPos = pos;
    if (m_single.kind == QLatin1String("de-en"))
        m_single.similar = similarWords(bareVerb(word), &m_single.base, pos);
    Unit &u = m_units[0];
    u.raw = toPlainQuotes(word);
    Segment s;
    s.before = s.after = words(word);
    u.segments = {s};
    render();
}

static QString chatMeaningModel(TranslateTool *tool)
{
    for (const char *id : {"qwen3-4b", "qwen3-1.7b"})
        if (tool->store()->isInstalled(QString::fromLatin1(id)))
            return QString::fromLatin1(id);
    return QString();
}

void Panel::requestChatMeaning()
{
    // what an English word means, when WordNet has no entry: a chat model writes it (with a warning), or the output
    // says what can be downloaded
    const bool proof = m_single.kind == QLatin1String("proofread");
    const QString word = proof ? m_single.result : m_single.word;
    m_single.meaningText.clear();
    m_single.meaningNote.clear();
    m_single.meaningPending = false;
    if (!m_single.on || !m_single.meanings.isEmpty() || word.isEmpty())
        return;
    const QString modelId = chatMeaningModel(m_tool);
    if (modelId.isEmpty()) {
        m_single.meaningNote = tr("WordNet has no entry for “%1”, so there is no meaning to show. A model can write one: download "
                                  "Qwen3 1.7B or Qwen3 4B in File > Options > Proofreading.")
                                   .arg(word);
        return;
    }
    QString error;
    EngineInterface *engine = m_tool->engine(&error);
    const QString memory = engine ? m_tool->loadProblem(QStringLiteral("corrector"), modelId) : error;
    if (!engine || !memory.isEmpty()) {
        m_single.meaningNote = engine ? memory : error;
        return;
    }
    m_single.meaningPending = true;
    m_single.meaningModel = m_tool->store()->model(modelId) ? m_tool->store()->model(modelId)->title : modelId;
    const int request = ++m_meaningRequest;
    const ModelFiles files = m_tool->store()->files(modelId);
    const int threads = m_tool->threads();
    RunOptions o;
    o.system = QStringLiteral("You explain English words. Answer with one short sentence in plain English that says what the "
                              "word means. Do not repeat the word in a list and do not add anything else.");
    o.batch = 1;
    const QString ask = QStringLiteral("What does the English word \"%1\" mean?").arg(word);
    QPointer<Panel> self(this);
    QThread *t = QThread::create([self, engine, files, threads, o, ask, request] {
        TranslateTool::lowerPriority();
        QString e;
        QString text;
        if (engine->load(QStringLiteral("corrector"), files, threads, &e)) {
            const QStringList r = engine->run(QStringLiteral("corrector"), {ask}, o, nullptr, &e);
            text = r.value(0);
        }
        QMetaObject::invokeMethod(qApp, [self, text, e, request] {
            if (!self || request != self->m_meaningRequest)
                return;
            self->m_tool->touchEngine();
            self->m_single.meaningPending = false;
            self->m_single.meaningText = text.trimmed();
            if (text.trimmed().isEmpty())
                self->m_single.meaningNote = e.isEmpty() ? tr("The model wrote no meaning.") : e;
            self->render();
            emit self->finished();
        }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void Panel::requestVariants()
{
    // other complete translations of a short text: one run of the model with several hypotheses, names dropped
    if (m_units.size() != 1)
        return;
    const QString modelId = words(m_units.at(0).original).size() <= 3 ? m_tool->wordModel(slot()) : m_tool->chosenModel(slot());
    if (modelId.isEmpty() || chatModel(modelId))
        return;
    QString error;
    EngineInterface *engine = m_tool->engine(&error);
    if (!engine || !m_tool->loadProblem(slot(), modelId).isEmpty())
        return;
    const ModelFiles files = m_tool->store()->files(modelId);
    const int threads = m_tool->threads();
    const QString kind = slot();
    const QString source = m_units.at(0).modelInput;
    const QString current = unitText(0);
    RunOptions o;
    o.beam = 6;
    o.banDashes = m_tool->rules().noEmDash;
    const int request = ++m_variantsRequest;
    m_variantsPending = true;
    const Rules rules = rulesForRun();
    QPointer<Panel> self(this);
    QThread *t = QThread::create([self, engine, files, threads, kind, source, current, o, request, rules] {
        TranslateTool::lowerPriority();
        QString e;
        QStringList found;
        if (engine->load(kind, files, threads, &e))
            found = engine->variants(kind, source, o, 6, &e);
        QMetaObject::invokeMethod(qApp, [self, found, request, source, current, rules] {
            if (!self || request != self->m_variantsRequest)
                return;
            self->m_tool->touchEngine();
            self->m_variantsPending = false;
            self->m_variants.clear();
            QStringList seen{current.toLower()};   // 1.4: capitals do not make another translation; the first (higher ranked) stays
            for (const QString &f : found) {
                const QString shown = tidyPunctuation(afterModel(f, rules));
                if (seen.contains(shown.toLower()) || (self->m_dir == Direction::DeToEn && hasStrangeName(shown, source)))
                    continue;
                seen << shown.toLower();
                self->m_variants << shown;
            }
            self->render();
        }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void Panel::swapDirection()
{
    if (m_busy)
        return;
    const Direction now = m_units.isEmpty() ? directionNow() : m_dir;
    const QString result = m_units.isEmpty() ? QString() : resultText();
    const QString task = now == Direction::DeToEn ? QStringLiteral("translateEn") : QStringLiteral("translate");
    m_task->setCurrentIndex(qMax(0, m_task->findData(task)));
    if (result.trimmed().isEmpty()) {
        updateModeUi();
        return;
    }
    setInput(result);   // clears what was translated back
    run();
}

void Panel::showBack(const QString &html)
{
    m_back->setHtml(html);
    m_back->setVisible(true);
}

void Panel::translateBack()
{
    if (m_busy || m_units.isEmpty() || m_dir == Direction::Proofread)
        return;
    const Direction back = m_dir == Direction::DeToEn ? Direction::EnToDe : Direction::DeToEn;
    const QString kind = kindOf(back);
    const QString text = resultText();
    if (text.trimmed().isEmpty())
        return;
    const bool shortText = text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size() <= 3;
    const QString modelId = shortText ? m_tool->wordModel(kind) : m_tool->chosenModel(kind);
    if (modelId.isEmpty()) {
        // the reverse model is offered for download
        QString offer;
        for (const CatalogModel &m : m_tool->store()->models(kind))
            if ((m.forWords || !shortText) && offer.isEmpty())
                offer = m.id;
        const CatalogModel *m = m_tool->store()->model(offer);
        showBack(m ? tr("The model for the way back is not downloaded: %1 (%2 MB). <a href=\"tp-download:%3\">Download it</a>")
                         .arg(m->title.toHtmlEscaped()).arg(m->sizeMB).arg(m->id)
                   : tr("There is no model for the way back."));
        return;
    }
    QString error;
    EngineInterface *engine = m_tool->engine(&error);
    const QString memory = engine ? m_tool->loadProblem(kind, modelId) : error;
    if (!engine || !memory.isEmpty()) {
        showBack(memory.toHtmlEscaped());
        return;
    }
    RunOptions o;
    o.beam = 4;
    o.banDashes = m_tool->rules().noEmDash;
    if (chatModel(modelId)) {
        o.system = systemFor(back == Direction::DeToEn ? QStringLiteral("translate") : QStringLiteral("translate-de"));
        o.examples = examplesFor(back == Direction::DeToEn ? QStringLiteral("translate") : QStringLiteral("translate-de"));
    }
    const ModelFiles files = m_tool->store()->files(modelId);
    const int threads = m_tool->threads();
    Rules rules = m_tool->rules();
    if (back == Direction::EnToDe) {
        rules.american = false;
        rules.curlyQuotes = false;
    }
    setBusy(true);
    setStatus(tr("Translating back..."));
    const QString sent = toPlainQuotes(text);
    const QString input = inputText();
    QPointer<Panel> self(this);
    QThread *t = QThread::create([self, engine, files, threads, kind, sent, o, rules, input] {
        TranslateTool::lowerPriority();
        QString e;
        QString out;
        if (engine->load(kind, files, threads, &e))
            out = engine->run(kind, {sent}, o, nullptr, &e).value(0);
        QMetaObject::invokeMethod(qApp, [self, out, e, rules, input] {
            if (!self)
                return;
            self->setBusy(false);
            self->m_tool->touchEngine();
            if (self->inputText() == input) {   // the input changed meanwhile: it belongs to a text that is gone
                self->showBack(out.isEmpty() ? tr("The way back failed: %1").arg(e.toHtmlEscaped())
                                             : tr("<i>Back:</i> %1").arg(tidyPunctuation(afterModel(out, rules)).toHtmlEscaped()));
            }
            self->setStatus(tr("Translated back."));
            emit self->finished();
        }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void Panel::renderSingle(QTextCursor &c, const QTextCharFormat &plain)
{
    const QPalette pal = m_output->palette();
    // 1.1: under a single word, every suggestion for it; a click on one makes it the result
    QTextCharFormat head = plain;
    head.setForeground(pal.color(QPalette::PlaceholderText));
    QFont f = m_output->font();
    f.setPointSizeF(f.pointSizeF() * 0.9);
    head.setFont(f);
    QTextCharFormat link = plain;
    link.setAnchor(true);
    link.setForeground(pal.color(QPalette::Link));
    const bool proof = m_single.kind == QLatin1String("proofread");
    auto linkFor = [&](const QString &word, const QString &href) {
        QTextCharFormat l = link;
        l.setAnchorHref(href);
        l.setToolTip(tr("Use “%1”").arg(word));
        return l;
    };
    auto list = [&](const QString &title, const QStringList &items) {
        if (items.isEmpty())
            return;
        c.insertBlock();
        c.insertText(title + QStringLiteral(" "), head);
        for (int i = 0; i < items.size(); ++i) {
            if (i > 0)
                c.insertText(QStringLiteral(", "), plain);
            c.insertText(items.at(i),
                         linkFor(items.at(i), QStringLiteral("tp-word:") + QString::fromUtf8(QUrl::toPercentEncoding(items.at(i)))));
        }
    };
    auto small = [&](const QString &text, bool italic = false) {
        QTextCharFormat h = head;
        h.setFontItalic(italic);
        c.insertBlock();
        c.insertText(text, h);
    };
    auto more = [&](const QString &key, int total) {
        if (m_single.expanded.contains(key) || total <= kFirstMeanings)
            return;
        QTextCharFormat l = link;
        l.setAnchorHref(QStringLiteral("tp-more:") + key);
        c.insertBlock();
        c.insertText(tr("more (%1)").arg(total - kFirstMeanings), l);
    };
    c.insertBlock();
    if (proof)
        list(tr("Spelling:"), m_single.spelling);

    // the German dictionary's classes (German to English), or the German words of an English word's classes
    for (int k = 0; k < m_single.classes.size(); ++k) {
        const DictClass &dc = m_single.classes.at(k);
        c.insertBlock();
        c.insertText(dc.pos.isEmpty() ? dc.headword : QStringLiteral("%1 (%2)").arg(dc.headword, dc.pos), head);
        const QString key = QStringLiteral("c%1").arg(k);
        int shownCount = 0;
        bool anyEnglish = false;
        for (const DictSense &s : dc.senses)
            anyEnglish = anyEnglish || !s.english.isEmpty();
        for (int i = 0; i < dc.senses.size(); ++i) {
            if (!m_single.expanded.contains(key) && i >= kFirstMeanings)
                break;
            const DictSense &s = dc.senses.at(i);
            c.insertBlock();
            c.insertText(QStringLiteral("%1. ").arg(i + 1), plain);
            if (s.english.isEmpty()) {
                c.insertText(s.definition.isEmpty() ? tr("(no English word)") : s.definition, head);
            } else {
                for (int w = 0; w < s.english.size(); ++w) {
                    if (w > 0)
                        c.insertText(QStringLiteral(", "), plain);
                    c.insertText(s.english.at(w),
                                 linkFor(s.english.at(w), QStringLiteral("tp-pick:%1:%2").arg(dc.pos, QString::fromUtf8(QUrl::toPercentEncoding(s.english.at(w))))));
                }
            }
            ++shownCount;
        }
        if (!dc.modelWord.isEmpty() && !anyEnglish) {
            c.insertBlock();
            c.insertText(dc.modelWord, linkFor(dc.modelWord, QStringLiteral("tp-pick:%1:%2").arg(dc.pos, QString::fromUtf8(QUrl::toPercentEncoding(dc.modelWord)))));
        }
        more(key, int(dc.senses.size()));
    }
    if (m_single.fromDictionary)
        small(tr("From the Wiktionary (CC BY-SA 4.0), the German dictionary you downloaded."));
    if (m_single.fromModel)
        small(tr("Warning: some of these words come from the small translation model. It may be inaccurate."), true);
    if (!m_single.note.isEmpty())
        small(m_single.note, true);

    // what an English word means (WordNet), or a model's words, or why there is none
    if (proof || m_single.kind == QLatin1String("en-de")) {
        for (const EnglishClass &ec : std::as_const(m_single.meanings)) {
            const QString key = QStringLiteral("m:") + ec.pos;
            c.insertBlock();
            c.insertText(tr("Meaning (%1):").arg(ec.pos), head);
            for (int i = 0; i < ec.meanings.size(); ++i) {
                if (!m_single.expanded.contains(key) && i >= kFirstMeanings)
                    break;
                c.insertBlock();
                c.insertText(QStringLiteral("%1. %2").arg(i + 1).arg(ec.meanings.at(i)), plain);
            }
            more(key, int(ec.meanings.size()));
        }
        if (!m_single.meanings.isEmpty())
            small(tr("The meanings are from WordNet 3.0 (Princeton University), a dictionary."));
        else if (m_single.meaningPending)
            small(tr("Asking %1 what the word means...").arg(m_single.meaningModel), true);
        else if (!m_single.meaningText.isEmpty()) {
            c.insertBlock();
            c.insertText(tr("Meaning:"), head);
            c.insertBlock();
            c.insertText(m_single.meaningText, plain);
            small(tr("Warning: WordNet has no entry for this word. %1 wrote this meaning, and it may be inaccurate.")
                      .arg(m_single.meaningModel),
                  true);
        } else if (!m_single.meaningNote.isEmpty()) {
            small(m_single.meaningNote, true);
        }
    }
    for (const Similar &sim : std::as_const(m_single.similar)) {
        const QString forBase = m_single.base.isEmpty() || m_single.base == m_single.result.toLower()
                                    ? QString()
                                    : tr(", for “%1”").arg(m_single.base);
        // 1.4: a translated word has similar translations (English: the thesaurus of the meaning; German: the dictionary's words)
        if (m_single.kind == QLatin1String("de-en"))
            list(tr("Similar translations (%1%2):").arg(sim.pos, forBase), sim.words);
        else if (m_single.kind == QLatin1String("en-de"))
            list(tr("Similar German words (%1):").arg(sim.pos), sim.words);
        else
            list(tr("Similar (%1%2):").arg(sim.pos, forBase), sim.words);
    }
    if (proof) {
        if (!m_single.modelWords.isEmpty())
            list(tr("The model’s other words:"), m_single.modelWords);
        else if (m_single.modelPending) {
            c.insertBlock();
            c.insertText(tr("Asking the model for other words..."), head);
        }
    }
}

// ---------------------------------------------------------------- the information area (1.1)

void Panel::showInfoForOutput(int position)
{
    const WordInfo w = wordAt(position);
    if (w.unit < 0 || w.word < 0)
        return;
    QString core;
    if (!isSingleWord(w.text, &core))
        core = w.text;
    m_infoWord = core;
    m_infoContext = QStringLiteral("output");
    m_infoEnglish = m_dir != Direction::EnToDe;   // the result is English, unless it was translated into German
    fillInfo();
}

void Panel::showInfoForInput(int position)
{
    const InputPlace place = inputPlace(position);
    const QTextBlock block = m_input->document()->findBlock(position);
    // the word under the position, also in German text (inputPlace gives English words only)
    QString word = place.wordText;
    if (word.isEmpty() && block.isValid()) {
        const QString text = block.text();
        int a = position - block.position(), b = a;
        auto inWord = [](QChar c) { return c.isLetter() || c == QLatin1Char('\'') || c == QChar(0x2019) || c == QLatin1Char('-'); };
        while (a > 0 && inWord(text.at(a - 1)))
            --a;
        while (b < text.size() && inWord(text.at(b)))
            ++b;
        word = text.mid(a, b - a);
    }
    QString core;
    if (!isSingleWord(word, &core))
        return;
    m_infoWord = core;
    m_infoContext = QStringLiteral("input");
    m_infoEnglish = block.isValid() && detectLanguage(block.text()) != QLatin1String("de");
    fillInfo();
}

void Panel::fillInfo()
{
    const QString w = m_infoWord;
    QString html = QStringLiteral("<p><b>%1</b>").arg(w.toHtmlEscaped());
    if (!m_infoEnglish) {
        html += tr(" (German): spelling suggestions and similar words are given for English words.") + QStringLiteral("</p>");
        m_info->setHtml(html);
        return;
    }
    QString base;
    const QList<Similar> similar = similarWords(w, &base);
    if (!base.isEmpty() && base != w.toLower())
        html += tr(" (from “%1”)").arg(base.toHtmlEscaped());
    html += QStringLiteral("</p>");
    if (m_infoContext == QLatin1String("output")) {
        // 1.4: the words of the result are the program's own: no line about their spelling
    } else if (knownWord(w)) {
        html += QStringLiteral("<p>") + tr("Spelling: correct.") + QStringLiteral("</p>");
    } else {
        const QStringList sp = spellingSuggestions(w, 10);
        html += QStringLiteral("<p>") + (sp.isEmpty() ? tr("Spelling: not in the dictionary, and no word near it is.")
                                                      : tr("Spelling: not in the dictionary. Did you mean %1?")
                                                            .arg(sp.join(QStringLiteral(", ")).toHtmlEscaped()))
                + QStringLiteral("</p>");
    }
    if (similar.isEmpty())
        html += QStringLiteral("<p>") + tr("Similar words: none known.") + QStringLiteral("</p>");
    for (const Similar &s : similar)
        html += QStringLiteral("<p>") + tr("Similar (%1): %2").arg(s.pos, s.words.join(QStringLiteral(", ")).toHtmlEscaped())
                + QStringLiteral("</p>");
    m_info->setHtml(html);
}

void Panel::takeFromEditor()
{
    qb::tools::ToolHost *host = m_tool->host();
    const QString text = host ? host->selectedText() : QString();
    if (text.trimmed().isEmpty()) {
        setStatus(tr("Select some text in the editor first."));
        return;
    }
    setInput(text);
    run();
}

void Panel::putIntoEditor()
{
    qb::tools::ToolHost *host = m_tool->host();
    if (m_units.isEmpty()) {
        setStatus(tr("There is no result yet."));
        return;
    }
    if (!host || !host->canInsertText()) {
        setStatus(tr("Click into a text in the editor first (or select the text to be replaced)."));
        return;
    }
    if (host->insertText(resultText()))
        setStatus(tr("Put into the editor. Edit > Undo there takes it back."));
}

void Panel::copyResult()
{
    if (m_units.isEmpty()) {
        setStatus(tr("There is no result yet."));
        return;
    }
    QGuiApplication::clipboard()->setText(resultText());
    setStatus(tr("The result is copied."));
}

} // namespace tp
