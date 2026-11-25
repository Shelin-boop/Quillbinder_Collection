// 0.94.0: a small message line under the binder. One message at a time, each for 5 seconds (10 seconds when it carries a
// link) or until it is clicked; further messages wait in a queue until the one shown is gone. Used for the backup
// note ("Backup saved." with "Show Folder"), the add-ons' short messages (tool interface 1.2, ToolHost::showMessage)
// and other short notices. The line has no height while it shows nothing.
#pragma once

#include <QQueue>
#include <QString>
#include <QWidget>
#include <functional>

class QLabel;
class QTimer;

namespace qb {

class MessageLine : public QWidget
{
    Q_OBJECT
public:
    explicit MessageLine(QWidget *parent = nullptr);

    // Queues a message. ms <= 0: 5000 (10000 with a link). `tip` is the tooltip of the text.
    void post(const QString &text, int ms = 0, const QString &linkText = QString(), std::function<void()> linkAction = nullptr,
              const QString &tip = QString());
    // The message shown now ("" = none), the number waiting, and the link's label (for the tests).
    QString shownText() const { return m_text; }
    int waiting() const { return int(m_queue.size()); }
    int shownMs() const { return m_shownMs; }
    QLabel *textLabel() const { return m_label; }
    QLabel *linkLabel() const { return m_link; }
    // Ends the message shown now (as a click does); the next one follows.
    void dismiss();
    void clearAll();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    struct Msg {
        QString text, linkText, tip;
        std::function<void()> action;
        int ms = 5000;
    };
    void showNext();
    QLabel *m_label, *m_link;
    QTimer *m_timer;
    QQueue<Msg> m_queue;
    std::function<void()> m_action;
    QString m_text;
    int m_shownMs = 0;
};

} // namespace qb
