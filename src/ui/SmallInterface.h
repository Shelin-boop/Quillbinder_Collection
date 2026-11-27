#pragma once
// 0.99.0 (item 122): the small interface, a general mode for small screens and for use without a keyboard. The window keeps
// the most room for the text; a few round buttons make a document, format (bold, italic, a style), open the binder and the
// document's notes as slide-in panels, offer labels and icons, record only (the speech add-on) and lead to the full menus.
// The mode itself is MainWindow::setSmallInterface (MainWindowSmall.cpp); this file holds the rules that need no window
// and the slide-in panel.
#include <QFrame>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QString>

class QLabel;
class QScreen;
class QToolButton;
class QVBoxLayout;
class QPropertyAnimation;

namespace qb {
namespace smallui {

// What the writer chose on Android (View > Small Interface: Full, Small, Automatic). On Linux and Windows the mode is a
// check item that is off at first, and only the writer's hand switches it.
enum class Choice { Full, Small, Automatic };
QString choiceKey(Choice c);                 // "full", "small", "auto"
Choice choiceFromKey(const QString &key);    // anything else: Automatic

// Is a screen below 1920 x 1080 physical pixels? The longer side is compared with 1920 and the shorter with 1080, so that a
// tablet held upright counts as it does held across.
bool belowFullHd(const QSize &physicalPixels);
// Whether the small interface is shown: Full never, Small always, Automatic below 1920 x 1080 physical pixels.
bool resolve(Choice choice, const QSize &physicalPixels);
// The size of a screen in physical pixels (the logical size times the device pixel ratio).
QSize physicalSize(const QScreen *screen);

// The side of a finger-sized target in pixels: `mm` millimetres (9 mm is Claude's starting value) at the system's DPI.
int touchTargetPx(double dpi, double mm = 9.0);
constexpr double kTouchTargetMm = 9.0;

// The settings keys (program settings, so a profile keeps them): the check item, and the Android choice
const char *const kKeyOn = "window/smallInterface";
const char *const kKeyChoice = "window/smallInterfaceChoice";

// A panel that slides in over the editor from the left or the right edge (landscape), or covers the whole area as a sheet
// (portrait). It holds one widget (the binder, the inspector) under a line with the title and a Close button.
class SlidePanel : public QFrame
{
    Q_OBJECT
public:
    enum class Edge { Left, Right, Sheet };
    explicit SlidePanel(const QString &title, QWidget *parent);
    void setContent(QWidget *content);
    QWidget *content() const { return m_content; }
    void setTargetSize(int touchPx);                   // the height of the title line and the Close button
    bool isOpen() const { return m_open; }
    // where the panel stands (or will stand) in the parent's coordinates, for the area it may cover
    QRect targetRect(const QRect &area, Edge edge) const;
    void openIn(const QRect &area, Edge edge, bool animate);
    void closePanel(bool animate);
    void refit(const QRect &area, Edge edge);          // the window changed its size while the panel is open
    Edge edge() const { return m_edge; }
    QToolButton *closeButton() const { return m_close; }
    QWidget *contentArea() const { return m_holder; }

signals:
    void closed();
    void opened();
    void settled();   // the panel stands where it will stay (after the slide, a refit): the focus line looks again

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    bool m_open = false;
    Edge m_edge = Edge::Left;
    QLabel *m_title = nullptr;
    QToolButton *m_close = nullptr;
    QWidget *m_holder = nullptr;
    QVBoxLayout *m_holderLayout = nullptr;
    QPointer<QWidget> m_content;
    QPropertyAnimation *m_anim = nullptr;
};

} // namespace smallui
} // namespace qb
