#pragma once
// 0.99.0 (handwriting, item 112): puts the handwriting together in the main window: the toolbar button with its four looks, the
// mode (kept per device), the inspector's tab "Handwriting", the one writing area in its three places, the strip's Insert and
// Pin, and the first use that opens File > Options > Handwriting while no recognizer is installed.
//
// The button: a tap switches between Off and the mode used last; a long press (or a right-click, or press-and-hold) opens
// the four choices (Off, Editor, Inspector, Both). The mode needs a recognizer: without one the tap and the choices only open
// the settings (settingsRequested), where the install part of the page is.
#include "ui/handwriting/HandwritingSettings.h"

#include <QFrame>
#include <QObject>
#include <QPointer>
#include <QTextCursor>
#include <QTimer>
#include <QToolButton>

class QAction;
class QMenu;
class QTextEdit;

namespace qb {

class Inspector;
class MainWindow;

namespace handwriting {

class HandwritingPanel;
class InkCanvas;

// The button: tap, long press, right-click.
class HandwritingButton : public QToolButton
{
    Q_OBJECT
public:
    explicit HandwritingButton(QWidget *parent = nullptr);
    static constexpr int kHoldMs = 500;

signals:
    void tapped();
    void choicesRequested(const QPoint &globalPos);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QTimer m_hold;
    bool m_held = false;
    bool m_down = false;
};

class HandwritingController : public QObject
{
    Q_OBJECT
public:
    HandwritingController(MainWindow *window, Inspector *inspector);
    ~HandwritingController() override;

    static constexpr const char *kTabKey = "handwriting";

    Mode mode() const { return m_mode; }
    // Switches the mode. Without a recognizer nothing changes and the settings are asked for (returns false).
    bool setMode(Mode mode);
    bool toggle();                       // the tap: Off <-> the mode used last
    static bool recognizerReady();
    // At the start: the stored mode, when a recognizer is there
    void applyStored();
    void setButtonShown(bool shown);
    bool buttonShown() const;

    QToolButton *button() const { return m_button; }
    QAction *showButtonAction() const { return m_showAction; }   // View > Toolbars > Handwriting Button
    void setBarAction(QAction *barAction);                       // the toolbar's widget action that holds the button
    QMenu *showChoices(const QPoint &globalPos = QPoint());      // the four choices (a menu of checkable entries)
    InkCanvas *canvas() const { return m_canvas; }
    HandwritingPanel *panel() const { return m_panel; }
    static QString modeName(Mode m);
    static QString iconName(Mode m);
    int settingsRequests() const { return m_settingsRequests; }

    // ---- text from the strip
    void hold(QTextEdit *editor);                 // the cursor set before (a text cursor that moves with the text)
    QTextCursor held() const { return m_held; }
    bool insert();                                 // Insert: the strip's text at the held cursor; false when there is none
    bool startPin();                               // Pin: the text sticks to the pen or finger
    bool pinned() const { return m_pinned; }
    void tapPlace(QWidget *field, const QPoint &viewportPos);   // a tap on a text field while pinned: shows the place
    bool dropPin();                                // the check mark
    void cancelPin();                              // the cross: the text goes back to the strip
    QWidget *pinGhost() const { return m_ghost; }
    QWidget *placeMarker() const { return m_marker; }
    bool hasPlace() const { return !m_place.isNull() && m_placeWidget; }

    // ---- the places of the one writing area
    void relayoutNow();
    QRegion overlayRegion() const;                 // in the window's coordinates (empty when the pad is in the inspector)
    QTextEdit *targetAt(const QPoint &canvasPos) const;

signals:
    void modeChanged(qb::handwriting::Mode mode);
    void settingsRequested();
    void message(const QString &text);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void applyMode();
    void updateButton();
    void scheduleLayout();
    void showPageIfHidden();
    void onTextReady();
    void setCanvasTransparent(bool on);
    void moveGhostTo(const QPoint &windowPos);
    void updateGhostButtons();

    QPointer<MainWindow> m_window;
    QPointer<Inspector> m_inspector;
    HandwritingPanel *m_panel = nullptr;
    InkCanvas *m_canvas = nullptr;
    HandwritingButton *m_button = nullptr;
    QAction *m_showAction = nullptr;
    QPointer<QAction> m_barAction;
    Mode m_mode = Mode::Off;
    bool m_layoutQueued = false;
    int m_settingsRequests = 0;
    QPointer<QMenu> m_choices;

    QTextCursor m_held;
    QPointer<QTextEdit> m_heldEditor;

    bool m_pinned = false;
    QString m_pinText;
    QFrame *m_ghost = nullptr;
    QFrame *m_marker = nullptr;
    QTextCursor m_place;
    QPointer<QWidget> m_placeWidget;
};

} // namespace handwriting
} // namespace qb
