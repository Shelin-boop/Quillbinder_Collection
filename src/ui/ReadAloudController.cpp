#include "ui/ReadAloudController.h"

#include "core/AiEngine.h"
#include "ui/GripBar.h"
#include "core/ItemIcons.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QTextDocument>
#include <QToolButton>

namespace qb {
namespace readaloud {

Controller::Controller(MainWindow *window, std::function<TextEditor *()> editor)
    : QObject(window), m_window(window), m_editorOf(std::move(editor))
{
    m_session = &Session::shared();
    connect(m_session, &Session::sentence, this, [this](int s, int l) { markSentence(s, l); });
    connect(m_session, &Session::stateChanged, this, [this] { refresh(); });
    connect(m_session, &Session::finished, this, [this](bool) { clearMark(); refresh(); });
    connect(m_session, &Session::failed, this, [this](const QString &why) { emit message(why); });
    connect(&Config::instance(), &Config::changed, this, [this] { refresh(); });
}

bool Controller::barShown() const { return Config::instance().get().barShown; }

void Controller::setBarShown(bool on)
{
    Settings s = Config::instance().get();
    if (s.barShown == on)
        return;
    s.barShown = on;
    Config::instance().set(s);
    emit barShownChanged(on);
}

GripBar *Controller::createBar()
{
    if (m_bar)
        return m_bar;
    m_bar = new GripBar(QCoreApplication::translate("readaloud", "Read Aloud"));
    m_bar->setObjectName(QStringLiteral("readAloudBar"));
    m_bar->setMovable(true);
    m_bar->setFloatable(false);
    m_bar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    m_play = new QAction(icons::action(QStringLiteral("media-playback-start")), tr("Play"), this);
    m_play->setObjectName(QStringLiteral("readAloudPlay"));
    connect(m_play, &QAction::triggered, this, [this] { playPause(); });
    m_stop = new QAction(icons::action(QStringLiteral("media-playback-stop")), tr("Stop"), this);
    m_stop->setObjectName(QStringLiteral("readAloudStop"));
    connect(m_stop, &QAction::triggered, this, [this] { stopReading(); });
    m_bar->addAction(m_play);
    m_bar->addAction(m_stop);
    m_voice = new QToolButton;
    m_voice->setObjectName(QStringLiteral("readAloudVoice"));
    m_voice->setIcon(icons::action(QStringLiteral("player-volume")));
    m_voice->setPopupMode(QToolButton::InstantPopup);
    m_menu = new QMenu(m_voice);
    m_menu->setObjectName(QStringLiteral("readAloudVoiceMenu"));
    m_menu->setToolTipsVisible(true);
    connect(m_menu, &QMenu::aboutToShow, this, [this] { fillMenu(); });
    m_voice->setMenu(m_menu);
    m_bar->addWidget(m_voice);
    refresh();
    return m_bar;
}

QString Controller::tooltip() const { return Config::instance().get().summary(); }

void Controller::refresh()
{
    if (!m_play)
        return;
    const Session::State st = m_session->state();
    const bool playing = st == Session::Playing;
    m_play->setIcon(icons::action(playing ? QStringLiteral("media-playback-pause") : QStringLiteral("media-playback-start")));
    m_play->setText(playing ? tr("Pause") : st == Session::Paused ? tr("Resume") : tr("Play"));
    m_play->setToolTip(tr("Read Aloud: %1 (the marked text, or from the cursor to the end)").arg(m_play->text()));
    m_stop->setEnabled(st != Session::Idle);
    m_stop->setToolTip(tr("Read Aloud: Stop"));
    m_voice->setToolTip(tooltip());
}

bool Controller::ready() const
{
    ai::Engine &e = ai::Engine::instance();
    if (!e.isInstalled() || !e.ensureLoaded())
        return false;
    return resolveModel(Config::instance().get(), e.devices(), true) != nullptr;
}

void Controller::playPause()
{
    switch (m_session->state()) {
    case Session::Playing:
        m_session->pause();
        return;
    case Session::Paused:
        m_session->resume();
        return;
    case Session::Loading:
        return;
    case Session::Idle:
        break;
    }
    Settings s = Config::instance().get();
    if (!s.settingsOpened) {   // the first click opens the settings
        s.settingsOpened = true;
        Config::instance().set(s);
        emit settingsRequested();
        return;
    }
    if (!ready()) {            // and every click while no engine (or no model) is installed
        emit settingsRequested();
        return;
    }
    start();
}

void Controller::start()
{
    TextEditor *ed = m_editorOf ? m_editorOf() : nullptr;
    if (!ed) {
        emit message(tr("Open a text to read it aloud."));
        return;
    }
    const QString all = ed->document()->toPlainText();
    QTextCursor c = ed->textCursor();
    int from = c.hasSelection() ? c.selectionStart() : c.position();
    int to = c.hasSelection() ? c.selectionEnd() : int(all.size());
    from = qBound(0, from, int(all.size()));
    to = qBound(from, to, int(all.size()));
    QString err;
    clearMark();
    m_target = ed;
    disconnect(m_docConn);
    m_docConn = connect(ed->document(), &QTextDocument::contentsChanged, this, [this] { m_session->stop(); });
    if (!m_session->start(all.mid(from, to - from), from, Config::instance().get(), &err))
        emit message(err);
}

void Controller::stopReading()
{
    m_session->stop();
    clearMark();
}

void Controller::markSentence(int start, int length)
{
    if (m_target)
        m_target->setReadingRange(start, length);
}

void Controller::clearMark()
{
    if (m_target)
        m_target->setReadingRange(-1, 0);
}

void Controller::fillMenu()
{
    m_menu->clear();
    const Settings s = Config::instance().get();
    ai::Engine &e = ai::Engine::instance();
    const QList<ai::Device> devices = e.isLoaded() ? e.devices() : QList<ai::Device>();
    const ai::CatalogModel *model = resolveModel(s, devices, false);
    const QString lang = effectiveLanguage(s);
    auto group = [&](const QString &heading, const QStringList &ids, const QString &current,
                     const std::function<QString(const QString &)> &label,
                     const std::function<Availability(const QString &)> &avail, const std::function<void(Settings &, const QString &)> &set) {
        m_menu->addSection(heading);
        auto *g = new QActionGroup(m_menu);
        g->setExclusive(true);
        for (const QString &id : ids) {
            QAction *ac = m_menu->addAction(label(id));
            ac->setObjectName(QStringLiteral("readAloud:") + id);
            ac->setCheckable(true);
            ac->setActionGroup(g);
            ac->setChecked(id == current);
            const Availability a = avail(id);
            if (!a.ok) {
                ac->setEnabled(false);
                ac->setToolTip(a.reason);
                ac->setStatusTip(a.reason);
            }
            connect(ac, &QAction::triggered, this, [id, set] {
                Settings n = Config::instance().get();
                set(n, id);
                Config::instance().set(n);
            });
        }
    };
    group(tr("Voice"), voiceIds(), s.voice, voiceLabel,
          [&](const QString &id) { return voiceAvailability(model, lang, id, s.style); },
          [](Settings &n, const QString &id) { n.voice = id; });
    group(tr("Style"), styleIds(), s.style, styleLabel,
          [&](const QString &id) { return styleAvailability(model, lang, s.voice, id); },
          [](Settings &n, const QString &id) { n.style = id; });
    group(tr("Speed"), speedIds(), s.speed, speedLabel, [](const QString &) { return Availability(); },
          [](Settings &n, const QString &id) { n.speed = id; });
    m_menu->addSeparator();
    QAction *settings = m_menu->addAction(tr("Read Aloud Settings…"));
    settings->setObjectName(QStringLiteral("readAloudSettings"));
    connect(settings, &QAction::triggered, this, [this] { emit settingsRequested(); });
}

} // namespace readaloud
} // namespace qb
