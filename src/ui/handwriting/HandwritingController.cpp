#include "ui/handwriting/HandwritingController.h"

#include "core/ItemIcons.h"
#include "ui/EditorPane.h"
#include "ui/Inspector.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"
#include "ui/handwriting/HandwritingPanel.h"
#include "ui/handwriting/InkCanvas.h"
#include "ui/handwriting/Recognizer.h"
#include "ui/handwriting/WordStrip.h"

#include <QAction>
#include <QAbstractScrollArea>
#include <QActionGroup>
#include <QApplication>
#include <QContextMenuEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegion>
#include <QSplitter>
#include <QTextDocument>
#include <QToolButton>
#include <QVBoxLayout>

#ifdef Q_OS_ANDROID
#include "android/Mobile.h"
#endif

namespace qb {
namespace handwriting {

// ---------------------------------------------------------------- the button

HandwritingButton::HandwritingButton(QWidget *parent) : QToolButton(parent)
{
    setObjectName(QStringLiteral("handwritingButton"));
    setAutoRaise(true);
    setFocusPolicy(Qt::NoFocus);
    m_hold.setSingleShot(true);
    m_hold.setInterval(kHoldMs);
    connect(&m_hold, &QTimer::timeout, this, [this] {
        if (!m_down)
            return;
        m_held = true;
        setDown(false);
        emit choicesRequested(mapToGlobal(rect().bottomLeft()));
    });
}

void HandwritingButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        e->accept();
        return;   // the context menu event opens the choices
    }
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    m_down = true;
    m_held = false;
    setDown(true);
    m_hold.start();
    e->accept();
}

void HandwritingButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->accept();
        return;
    }
    const bool wasDown = m_down;
    m_down = false;
    m_hold.stop();
    setDown(false);
    if (wasDown && !m_held && rect().contains(e->position().toPoint()))
        emit tapped();
    m_held = false;
    e->accept();
}

void HandwritingButton::contextMenuEvent(QContextMenuEvent *e)
{
    m_hold.stop();
    m_down = false;
    setDown(false);
    emit choicesRequested(e->globalPos());
    e->accept();
}

// ---------------------------------------------------------------- the controller

QString HandwritingController::modeName(Mode m)
{
    switch (m) {
    case Mode::Off: return tr("Off");
    case Mode::Editor: return tr("Editor");
    case Mode::Inspector: return tr("Inspector");
    case Mode::Both: break;
    }
    return tr("Both");
}

QString HandwritingController::iconName(Mode m)
{
    switch (m) {
    case Mode::Off: return QStringLiteral("handwriting-off");
    case Mode::Editor: return QStringLiteral("handwriting-editor");
    case Mode::Inspector: return QStringLiteral("handwriting-inspector");
    case Mode::Both: break;
    }
    return QStringLiteral("handwriting-both");
}

bool HandwritingController::recognizerReady() { return Recognizer::installed(); }

HandwritingController::HandwritingController(MainWindow *window, Inspector *inspector)
    : QObject(window), m_window(window), m_inspector(inspector)
{
    m_panel = new HandwritingPanel;
    m_canvas = new InkCanvas(m_panel->padHolder());
    m_panel->padLayout()->addWidget(m_canvas);
    m_canvas->hide();
    m_canvas->setTargetFinder([this](const QPoint &p) { return targetAt(p); });
    inspector->addExtraTab(QLatin1String(kTabKey), m_panel, icons::action(iconName(Mode::Inspector)), tr("Handwriting"));
    inspector->setExtraTabVisible(QLatin1String(kTabKey), false);

    m_button = new HandwritingButton;
    connect(m_button, &HandwritingButton::tapped, this, [this] { toggle(); });
    connect(m_button, &HandwritingButton::choicesRequested, this, [this](const QPoint &p) { showChoices(p); });

    m_showAction = new QAction(tr("Handwriting Button"), this);
    m_showAction->setObjectName(QStringLiteral("showHandwritingButton"));
    m_showAction->setCheckable(true);
    m_showAction->setChecked(HandwritingConfig::instance().get().buttonShown);
    m_showAction->setToolTip(tr("Show the button for handwriting on the toolbar"));
    connect(m_showAction, &QAction::toggled, this, [this](bool on) { setButtonShown(on); });

    WordStrip *strip = m_panel->strip();
    strip->setFadeMs(HandwritingConfig::instance().get().fadeMs);
    connect(m_canvas, &InkCanvas::textReady, this, [this](const QImage &pic, const Result &r, qint64 started) {
        m_panel->strip()->addLine(pic, r, started);
        showPageIfHidden();
    });
    connect(m_canvas, &InkCanvas::cursorPlaced, this, [this](QTextEdit *ed) { hold(ed); });
    connect(m_canvas, &InkCanvas::readFailed, this, &HandwritingController::message);
    connect(m_panel->spaceButton(), &QPushButton::clicked, this, [this] {
        if (!m_panel->strip()->isEmpty())
            m_panel->strip()->appendSpace();
        else if (m_heldEditor && !m_held.isNull()) {
            QTextCursor c(m_held);
            c.insertText(QStringLiteral(" "));
            m_held = c;
        }
    });
    connect(m_panel->enterButton(), &QPushButton::clicked, this, [this] {
        if (!m_panel->strip()->isEmpty())
            m_panel->strip()->appendNewline();
        else if (!m_held.isNull()) {
            QTextCursor c(m_held);
            c.insertBlock();
            m_held = c;
        }
    });
    connect(m_panel->backspaceButton(), &QPushButton::clicked, this, [this] {
        if (!m_panel->strip()->isEmpty())
            m_panel->strip()->backspace();
        else if (!m_held.isNull()) {
            QTextCursor c(m_held);
            c.deletePreviousChar();
            m_held = c;
        }
    });
    connect(m_panel->clearButton(), &QPushButton::clicked, this, [this] { m_panel->strip()->clear(); });
    connect(m_panel->insertButton(), &QPushButton::clicked, this, [this] { insert(); });
    connect(m_panel->pinButton(), &QPushButton::clicked, this, [this] { startPin(); });

    connect(&HandwritingConfig::instance(), &HandwritingConfig::changed, this, [this] {
        const HandwritingSettings &s = HandwritingConfig::instance().get();
        m_panel->strip()->setFadeMs(s.fadeMs);
        if (m_showAction->isChecked() != s.buttonShown) {
            QSignalBlocker b(m_showAction);
            m_showAction->setChecked(s.buttonShown);
        }
        updateButton();
    });

    // the pin's ghost and the place marker
    m_ghost = new QFrame(window);
    m_ghost->setObjectName(QStringLiteral("hwPinGhost"));
    m_ghost->setFrameShape(QFrame::StyledPanel);
    m_ghost->setAutoFillBackground(true);
    auto *gl = new QHBoxLayout(m_ghost);
    gl->setContentsMargins(6, 3, 6, 3);
    auto *gtext = new QLabel(m_ghost);
    gtext->setObjectName(QStringLiteral("hwPinText"));
    gl->addWidget(gtext);
    auto *ok = new QToolButton(m_ghost);
    ok->setObjectName(QStringLiteral("hwPinDrop"));
    ok->setText(QStringLiteral("✓"));
    ok->setToolTip(tr("Put the text at the place shown"));
    auto *no = new QToolButton(m_ghost);
    no->setObjectName(QStringLiteral("hwPinCancel"));
    no->setText(QStringLiteral("✕"));
    no->setToolTip(tr("Cancel: the text goes back to the strip"));
    gl->addWidget(ok);
    gl->addWidget(no);
    connect(ok, &QToolButton::clicked, this, [this] { dropPin(); });
    connect(no, &QToolButton::clicked, this, [this] { cancelPin(); });
    m_ghost->hide();
    m_marker = new QFrame(window);
    m_marker->setObjectName(QStringLiteral("hwPlaceMarker"));
    m_marker->setAutoFillBackground(true);
    QPalette mp = m_marker->palette();
    mp.setColor(QPalette::Window, QColor(0xd0, 0x30, 0x30));
    m_marker->setPalette(mp);
    m_marker->setFixedWidth(3);
    m_marker->hide();

    // the layout follows the window, the editor area and the page
    window->installEventFilter(this);
    if (QWidget *ea = window->editorAreaWidget())
        ea->installEventFilter(this);
    m_panel->installEventFilter(this);
    m_panel->padHolder()->installEventFilter(this);
    qApp->installEventFilter(this);   // the cursor held when an editor loses the keyboard; the pin's taps
    if (QSplitter *sp = window->paneSplitter())
        connect(sp, &QSplitter::splitterMoved, this, [this] { scheduleLayout(); });
    updateButton();
}

HandwritingController::~HandwritingController()
{
    if (qApp)
        qApp->removeEventFilter(this);
}

bool HandwritingController::buttonShown() const { return HandwritingConfig::instance().get().buttonShown; }

void HandwritingController::setButtonShown(bool shown)
{
    HandwritingSettings s = HandwritingConfig::instance().get();
    if (s.buttonShown != shown) {
        s.buttonShown = shown;
        HandwritingConfig::instance().set(s);
    }
    if (m_barAction)
        m_barAction->setVisible(shown);
    if (m_showAction->isChecked() != shown) {
        QSignalBlocker b(m_showAction);
        m_showAction->setChecked(shown);
    }
}

void HandwritingController::setBarAction(QAction *barAction)
{
    m_barAction = barAction;
    if (m_barAction)
        m_barAction->setVisible(buttonShown());
}

void HandwritingController::updateButton()
{
    m_button->setIcon(icons::action(iconName(m_mode)));
    m_button->setToolTip(m_mode == Mode::Off
                             ? tr("Handwriting: off. Tap to switch it on; press and hold for the choices.")
                             : tr("Handwriting: %1. Tap to switch it off; press and hold for the choices.").arg(modeName(m_mode)));
    if (m_barAction)
        m_barAction->setVisible(buttonShown());
}

QMenu *HandwritingController::showChoices(const QPoint &globalPos)
{
    delete m_choices;
    auto *menu = new QMenu(m_button);
    menu->setObjectName(QStringLiteral("handwritingChoices"));
    auto *group = new QActionGroup(menu);
    for (Mode m : {Mode::Off, Mode::Editor, Mode::Inspector, Mode::Both}) {
        QAction *a = menu->addAction(icons::action(iconName(m)), modeName(m));
        a->setObjectName(QStringLiteral("hwChoice") + QString::number(int(m)));
        a->setCheckable(true);
        a->setChecked(m == m_mode);
        group->addAction(a);
        connect(a, &QAction::triggered, this, [this, m] { setMode(m); });
    }
    m_choices = menu;
    if (!globalPos.isNull() || m_button->isVisible())
        menu->popup(globalPos.isNull() ? m_button->mapToGlobal(m_button->rect().bottomLeft()) : globalPos);
    return menu;
}

bool HandwritingController::toggle()
{
    if (m_mode != Mode::Off)
        return setMode(Mode::Off);
    return setMode(HandwritingConfig::instance().get().lastMode);
}

bool HandwritingController::setMode(Mode mode)
{
    if (mode != Mode::Off && !recognizerReady()) {
        // the first use, and every use while nothing is installed: the page where it is installed
        ++m_settingsRequests;
        emit settingsRequested();
        return false;
    }
    if (mode == m_mode)
        return true;
    m_mode = mode;
    HandwritingSettings s = HandwritingConfig::instance().get();
    s.mode = mode;
    if (mode != Mode::Off)
        s.lastMode = mode;
    HandwritingConfig::instance().set(s);
    applyMode();
    emit modeChanged(mode);
    return true;
}

void HandwritingController::applyStored()
{
    const Mode stored = HandwritingConfig::instance().get().mode;
    if (stored != Mode::Off && recognizerReady()) {
        m_mode = stored;
        applyMode();
    } else {
        applyMode();
    }
}

void HandwritingController::applyMode()
{
    const bool on = m_mode != Mode::Off;
    m_inspector->setExtraTabVisible(QLatin1String(kTabKey), on);
    if (on) {
        m_window->toolShowInspectorTab(QLatin1String(kTabKey));   // the inspector opens when a mode needs it
        m_panel->showPlaceholder(m_mode != Mode::Inspector);
        m_panel->setPlaceholderText(m_mode == Mode::Editor ? tr("Writing on the editor.\nThe text you write is collected below.")
                                                          : tr("Writing on the editor and here."));
    }
    relayoutNow();
    updateButton();
#ifdef Q_OS_ANDROID
    // the on-screen keyboard stays down while a mode is on (the switch of Android 0.93.2); off: Android's way again
    if (on)
        qApp->setAutoSipEnabled(false);
    else
        mobile::followKeyboard();
#endif
}

void HandwritingController::showPageIfHidden()
{
    if (m_mode == Mode::Off || !m_inspector)
        return;
    if (!m_panel->isVisible())
        m_window->toolShowInspectorTab(QLatin1String(kTabKey));
}

// ---------------------------------------------------------------- the one writing area

void HandwritingController::scheduleLayout()
{
    if (m_layoutQueued)
        return;
    m_layoutQueued = true;
    QTimer::singleShot(0, this, [this] {
        m_layoutQueued = false;
        relayoutNow();
    });
}

static QRect rectIn(QWidget *window, QWidget *w)
{
    return QRect(w->mapTo(window, QPoint(0, 0)), w->size());
}

QRegion HandwritingController::overlayRegion() const
{
    if (!m_window || (m_mode != Mode::Editor && m_mode != Mode::Both))
        return QRegion();
    QWidget *ea = m_window->editorAreaWidget();
    if (!ea || !ea->isVisible())
        return QRegion();
    const QRect e = rectIn(m_window, ea);
    QRegion r(e);
    if (m_mode == Mode::Both && m_panel->padHolder()->isVisible()) {
        const QRect p = rectIn(m_window, m_panel->padHolder());
        r += p;
        QRect gap;
        if (e.right() < p.left())
            gap = QRect(QPoint(e.right() + 1, qMax(e.top(), p.top())), QPoint(p.left() - 1, qMin(e.bottom(), p.bottom())));
        else if (p.right() < e.left())
            gap = QRect(QPoint(p.right() + 1, qMax(e.top(), p.top())), QPoint(e.left() - 1, qMin(e.bottom(), p.bottom())));
        else if (e.bottom() < p.top())
            gap = QRect(QPoint(qMax(e.left(), p.left()), e.bottom() + 1), QPoint(qMin(e.right(), p.right()), p.top() - 1));
        else if (p.bottom() < e.top())
            gap = QRect(QPoint(qMax(e.left(), p.left()), p.bottom() + 1), QPoint(qMin(e.right(), p.right()), e.top() - 1));
        if (gap.isValid())
            r += gap;
    }
    return r;
}

void HandwritingController::relayoutNow()
{
    if (!m_window)
        return;
    switch (m_mode) {
    case Mode::Off:
        m_canvas->hide();
        m_canvas->clearMask();
        if (m_canvas->parentWidget() != m_panel->padHolder()) {
            m_canvas->setParent(m_panel->padHolder());
            m_panel->padLayout()->addWidget(m_canvas);
            m_canvas->hide();
        }
        break;
    case Mode::Inspector:
        if (m_canvas->parentWidget() != m_panel->padHolder()) {
            m_canvas->clearMask();
            m_canvas->setParent(m_panel->padHolder());
            m_panel->padLayout()->addWidget(m_canvas);
        }
        m_canvas->setRole(InkCanvas::Role::Pad);
        m_canvas->clearGridRect();
        m_canvas->show();
        break;
    case Mode::Editor:
    case Mode::Both: {
        if (m_canvas->parentWidget() != m_window) {
            m_panel->padLayout()->removeWidget(m_canvas);
            m_canvas->setParent(m_window);
        }
        const QRegion region = overlayRegion();
        if (region.isEmpty()) {
            m_canvas->hide();
            break;
        }
        const QRect box = region.boundingRect();
        m_canvas->setRole(InkCanvas::Role::Overlay);
        m_canvas->setGeometry(box);
        m_canvas->setMask(region.translated(-box.topLeft()));
        if (m_mode == Mode::Both && m_panel->padHolder()->isVisible()) {
            const QRect pad = rectIn(m_window, m_panel->padHolder());
            m_canvas->setGridRect(QRectF(pad.translated(-box.topLeft())));
        } else {
            m_canvas->setGridRect(QRectF());
        }
        m_canvas->show();
        m_canvas->raise();
        if (m_ghost->isVisible())
            m_ghost->raise();
        break;
    }
    }
}

QTextEdit *HandwritingController::targetAt(const QPoint &p) const
{
    if (m_mode == Mode::Inspector || m_mode == Mode::Off || !m_window)
        return nullptr;
    if (m_canvas->gridRect().contains(p))
        return nullptr;
    QWidget *ea = m_window->editorAreaWidget();
    if (!ea)
        return nullptr;
    const QPoint g = m_canvas->mapToGlobal(p);
    const QList<TextEditor *> eds = ea->findChildren<TextEditor *>();
    for (TextEditor *ed : eds)
        if (ed->isVisible() && ed->viewport()->rect().contains(ed->viewport()->mapFromGlobal(g)))
            return ed;
    return nullptr;
}

// ---------------------------------------------------------------- text from the strip

void HandwritingController::hold(QTextEdit *editor)
{
    if (!editor)
        return;
    m_heldEditor = editor;
    m_held = editor->textCursor();
}

bool HandwritingController::insert()
{
    WordStrip *strip = m_panel->strip();
    if (strip->isEmpty())
        return false;
    if (m_held.isNull() || !m_heldEditor) {
        if (m_window)
            if (TextEditor *ed = m_window->handwritingEditor())
                hold(ed);
    }
    if (m_held.isNull() || !m_heldEditor || m_heldEditor->isReadOnly())
        return false;
    QTextCursor c(m_held);
    c.insertText(strip->text());
    m_held = c;   // the place behind the text
    if (m_heldEditor->document() == c.document())
        m_heldEditor->setTextCursor(c);
    strip->clear();
    return true;
}

bool HandwritingController::startPin()
{
    WordStrip *strip = m_panel->strip();
    if (strip->isEmpty() || m_pinned)
        return false;
    m_pinned = true;
    m_pinText = strip->text();
    strip->clear();
    m_place = QTextCursor();
    m_placeWidget = nullptr;
    auto *label = m_ghost->findChild<QLabel *>(QStringLiteral("hwPinText"));
    const QString oneLine = QString(m_pinText).replace(QLatin1Char('\n'), QStringLiteral(" / "));
    label->setText(QFontMetrics(label->font()).elidedText(oneLine, Qt::ElideRight, 260));
    m_ghost->adjustSize();
    m_ghost->move(m_window->mapFromGlobal(QCursor::pos()) + QPoint(14, 14));
    m_ghost->show();
    m_ghost->raise();
    updateGhostButtons();
    setCanvasTransparent(true);   // taps reach the text fields under the writing area
    return true;
}

void HandwritingController::setCanvasTransparent(bool on)
{
    m_canvas->setAttribute(Qt::WA_TransparentForMouseEvents, on);
}

void HandwritingController::updateGhostButtons()
{
    if (auto *ok = m_ghost->findChild<QToolButton *>(QStringLiteral("hwPinDrop")))
        ok->setEnabled(hasPlace());
}

void HandwritingController::moveGhostTo(const QPoint &windowPos)
{
    m_ghost->move(windowPos + QPoint(14, 14));
    m_ghost->raise();
}

void HandwritingController::tapPlace(QWidget *field, const QPoint &viewportPos)
{
    if (!m_pinned || !field)
        return;
    QTextCursor c;
    QRect cr;
    QWidget *vp = nullptr;
    if (auto *te = qobject_cast<QTextEdit *>(field)) {
        c = te->cursorForPosition(viewportPos);
        cr = te->cursorRect(c);
        vp = te->viewport();
    } else if (auto *pe = qobject_cast<QPlainTextEdit *>(field)) {
        c = pe->cursorForPosition(viewportPos);
        cr = pe->cursorRect(c);
        vp = pe->viewport();
    } else {
        return;
    }
    m_place = c;
    m_placeWidget = field;
    const QPoint top = vp->mapTo(m_window, cr.topLeft());
    m_marker->setGeometry(top.x(), top.y(), 3, qMax(14, cr.height()));
    m_marker->show();
    m_marker->raise();
    moveGhostTo(top + QPoint(0, cr.height()));
    updateGhostButtons();
}

bool HandwritingController::dropPin()
{
    if (!m_pinned || !hasPlace())
        return false;
    QTextCursor c(m_place);
    c.insertText(m_pinText);
    if (auto *te = qobject_cast<QTextEdit *>(m_placeWidget.data())) {
        te->setTextCursor(c);
        if (m_window->editorAreaWidget() && m_window->editorAreaWidget()->isAncestorOf(te))
            hold(te);
    }
    m_pinned = false;
    m_pinText.clear();
    m_ghost->hide();
    m_marker->hide();
    m_place = QTextCursor();
    m_placeWidget = nullptr;
    setCanvasTransparent(false);
    return true;
}

void HandwritingController::cancelPin()
{
    if (!m_pinned)
        return;
    m_panel->strip()->setText(m_pinText);
    m_pinned = false;
    m_pinText.clear();
    m_ghost->hide();
    m_marker->hide();
    m_place = QTextCursor();
    m_placeWidget = nullptr;
    setCanvasTransparent(false);
}

bool HandwritingController::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
    case QEvent::LayoutRequest:
        if (o == m_window || o == m_panel || o == m_panel->padHolder() || (m_window && o == m_window->editorAreaWidget()))
            if (m_mode == Mode::Editor || m_mode == Mode::Both)
                scheduleLayout();
        break;
    case QEvent::FocusOut:
        // the keyboard leaves an editor (a tap on a button): its cursor is the place the text goes to
        if (auto *te = qobject_cast<TextEditor *>(o))
            if (m_window && m_window->editorAreaWidget() && m_window->editorAreaWidget()->isAncestorOf(te))
                hold(te);
        break;
    case QEvent::MouseMove:
        if (m_pinned) {
            if (auto *w = qobject_cast<QWidget *>(o); w && w->window() == m_window && !m_ghost->isAncestorOf(w) && w != m_ghost)
                if (!hasPlace())
                    moveGhostTo(m_window->mapFromGlobal(static_cast<QMouseEvent *>(e)->globalPosition().toPoint()));
        }
        break;
    case QEvent::MouseButtonRelease:
        if (m_pinned) {
            auto *w = qobject_cast<QWidget *>(o);
            auto *me = static_cast<QMouseEvent *>(e);
            if (w && w->parentWidget() && w->window() == m_window && !m_ghost->isAncestorOf(w)) {
                QWidget *field = w->parentWidget();
                if (qobject_cast<QTextEdit *>(field) || qobject_cast<QPlainTextEdit *>(field))
                    if (static_cast<QAbstractScrollArea *>(field)->viewport() == w)
                        tapPlace(field, me->position().toPoint());
            }
        }
        break;
    default:
        break;
    }
    return QObject::eventFilter(o, e);
}

} // namespace handwriting
} // namespace qb
