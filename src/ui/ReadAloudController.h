#pragma once
// 0.99.0 (item 114): Read Aloud in the main window: the toolbar "Read Aloud" (Play/Pause, Stop and a Voice button with its menu),
// the sentence marked in the editor and the first click that opens File > Options > Read Aloud (core/ReadAloud.h has the reading).
//
// The Voice menu has three groups with headings (Voice, Style, Speed) and "Read Aloud Settings..." at the bottom; a choice the
// model in use cannot do is greyed out and its tool tip says why. The buttons follow the toolbar size setting (MainWindow).
#include "core/ReadAloud.h"

#include <QObject>
#include <QPointer>

class QAction;
class QMenu;
class QToolBar;
class QToolButton;

namespace qb {

class GripBar;
class MainWindow;
class TextEditor;

namespace readaloud {

class Controller : public QObject
{
    Q_OBJECT
public:
    // `editor` gives the text editor the writer works in (null when none)
    Controller(MainWindow *window, std::function<TextEditor *()> editor);

    GripBar *createBar();                   // the toolbar (made once)
    QAction *showAction() const { return m_show; }   // View > Toolbars > Read Aloud
    QAction *playAction() const { return m_play; }
    QAction *stopAction() const { return m_stop; }
    QToolButton *voiceButton() const { return m_voice; }
    QMenu *voiceMenu() const { return m_menu; }
    Session &session() const { return *m_session; }
    bool barShown() const;
    void setBarShown(bool on);

    // The Play/Pause button's work
    void playPause();
    void stopReading();
    // Can Play read now: the engine and a model for the language are installed
    bool ready() const;
    void fillMenu();                        // the Voice menu (also when it opens)
    QString tooltip() const;                // "Female 2 · Calm · Medium"

signals:
    void settingsRequested();               // File > Options > Read Aloud
    void barShownChanged(bool on);
    void message(const QString &text);

private:
    void refresh();
    void start();
    void markSentence(int start, int length);
    void clearMark();

    MainWindow *m_window = nullptr;
    std::function<TextEditor *()> m_editorOf;
    Session *m_session = nullptr;
    GripBar *m_bar = nullptr;
    QAction *m_play = nullptr, *m_stop = nullptr, *m_show = nullptr;
    QToolButton *m_voice = nullptr;
    QMenu *m_menu = nullptr;
    QPointer<TextEditor> m_target;
    QMetaObject::Connection m_docConn;
};

// Plays through the media add-on's player (a sink of the add-on): set as the output of the program's reading
void useMediaOutput();

} // namespace readaloud
} // namespace qb
