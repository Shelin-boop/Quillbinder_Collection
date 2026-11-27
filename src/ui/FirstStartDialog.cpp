// The questions of the first start (0.62); see FirstStartDialog.h.
#include "ui/FirstStartDialog.h"
#include "core/TestSwitches.h"
#include "AppInfo.h"
#include "core/Backup.h"
#include "core/Portable.h"
#include "core/SettingsLayer.h"
#include "core/UpdateCheck.h"
#include "ui/AppMenuEntry.h"
#include "ui/AppSettings.h"
#include "ui/HintLabel.h"
#include "ui/Language.h"
#include "ui/PortableSetup.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace qb {

FirstStartDialog::FirstStartDialog(QWidget *parent, bool again) : QDialog(parent)
{
    setObjectName(QStringLiteral("firstStartDialog"));
    // 0.81.0 (audit of 0.80.0, S-07): Qt adds the program's name to every window title itself
    const bool portable = portable::active();
    setWindowTitle(again ? tr("This Computer") : tr("First Start"));
    const AppSettings &s = AppSettings::instance();
    auto *outer = new QVBoxLayout(this);
    auto *body = new QWidget(this);
    auto *v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    if (portable) {
        // W4b: the guide of the portable program is long; it scrolls on a small screen
        auto *scroll = new QScrollArea(this);
        scroll->setObjectName(QStringLiteral("firstStartScroll"));
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setWidget(body);
        outer->addWidget(scroll, 1);
    } else {
        outer->addWidget(body);
    }
    // 0.81.0 (audit of 0.80.0, S-09): only the backups; which settings come first (a project's own or the program's)
    // is no question for someone who has no project yet: it keeps its standard (a project's own settings first) and
    // stays in File > Options > General > Startup
    auto *intro = new QLabel(portable ? tr("Before you start: where Quillbinder keeps its files on this computer and how your "
                                           "projects are backed up. OK keeps what is filled in; you can change all of it "
                                           "later in File > Options (General > This Computer, Backup) or with Help > Troubleshooting "
                                           "and Development > This Computer.")
                                      : tr("Before you start: how your projects are backed up. Everything is filled in already: OK "
                                           "keeps it as it is. You can change all of it later in File > Options."),
                             body);
    intro->setObjectName(QStringLiteral("firstStartIntro"));
    intro->setWordWrap(true);
    v->addWidget(intro);

    // 0.95.0 (item 102): the language, taken from the system when it is one the program speaks
    {
        auto *lang = new QGroupBox(tr("Language"), body);
        auto *lf = new QFormLayout(lang);
        m_language = new QComboBox(lang);
        m_language->setObjectName(QStringLiteral("firstLanguage"));
        for (const QString &code : language::available())
            m_language->addItem(language::listLabel(code), code);
        m_language->setCurrentIndex(qMax(0, m_language->findData(s.language)));
        lf->addRow(tr("Language of the program:"), m_language);
        auto *langHint = new QLabel(tr("A language you choose here is used from the next start of the program."), lang);
        langHint->setObjectName(QStringLiteral("firstLanguageHint"));
        langHint->setWordWrap(true);
        hintlabel::style(langHint);
        lf->addRow(langHint);
        v->addWidget(lang);
    }

    if (portable) {
        // W4b: the portable program, said once in plain words
        const QString folder = portable::programFolder();
        auto *where = new QGroupBox(tr("Everything in One Folder"), body);
        auto *wv = new QVBoxLayout(where);
        auto *text = new QLabel(tr("Quillbinder keeps its settings, add-ons, program updates, fonts and crash reports in the "
                                   "folder \"data\" inside its own folder:\n%1\n\nNothing is written into Windows' own "
                                   "folders or the registry, unless you ask for it below. The whole folder can be moved or "
                                   "copied, to a USB stick or another computer, and deleting it removes the program. Program "
                                   "updates are installed with Help > Add-ons and Updates > Install Update or Add-on; to move to a version you "
                                   "downloaded anew, unpack it and copy the folder \"data\" into it. Your projects stay "
                                   "wherever you keep them.")
                                    .arg(PortableSetup::shownPath(folder)),
                                where);
        text->setObjectName(QStringLiteral("firstStartWhere"));
        text->setWordWrap(true);
        text->setTextInteractionFlags(Qt::TextSelectableByMouse);
        wv->addWidget(text);
        QStringList warnings;
        const QString service = portable::syncService(folder);
        if (!service.isEmpty())
            warnings << tr("The program's folder is inside your %1 folder. That works, but the sync service copies the "
                           "program's own files back and forth and may keep them only online. Better: the program's "
                           "folder outside it (for example C:\\Users\\<your name>\\Quillbinder), your projects inside it.")
                            .arg(service);
        if (portable::longPath(folder))
            warnings << tr("The program's folder has a long path. Windows limits most paths to 260 characters; a short "
                           "place such as C:\\Users\\<your name>\\Quillbinder leaves room for updates and add-ons.");
        warnings << tr("Windows may warn when the program starts for the first time, because it is not signed (SmartScreen: "
                       "\"More info\", then \"Run anyway\"). That happens again for each new download.");
        for (const QString &w : warnings) {
            auto *l = new QLabel(w, where);
            l->setObjectName(QStringLiteral("firstStartWarning"));
            l->setWordWrap(true);
            hintlabel::style(l);
            wv->addWidget(l);
        }
        v->addWidget(where);
    }

    auto *backups = new QGroupBox(tr("Backups"), body);
    auto *bf = new QFormLayout(backups);
    m_backup = new QCheckBox(tr("Make a backup of a project automatically when it is closed"), backups);
    m_backup->setObjectName(QStringLiteral("firstBackup"));
    m_backup->setChecked(s.backupOnClose);
    bf->addRow(m_backup);
    m_keep = new QSpinBox(backups);
    m_keep->setObjectName(QStringLiteral("firstBackupKeep"));
    m_keep->setRange(1, 100);
    m_keep->setValue(s.backupKeep);
    // 0.95.0: the field holds only the number; the words stand before and after it
    auto *keepRow = new QWidget(backups);
    keepRow->setObjectName(QStringLiteral("firstBackupKeepRow"));
    auto *kl = new QHBoxLayout(keepRow);
    kl->setContentsMargins(0, 0, 0, 0);
    auto *keepBefore = new QLabel(tr("Keep the newest"), keepRow);
    keepBefore->setBuddy(m_keep);
    keepBefore->setObjectName(QStringLiteral("firstBackupKeepBefore"));
    auto *keepAfter = new QLabel(tr("backups of each project"), keepRow);
    keepAfter->setObjectName(QStringLiteral("firstBackupKeepAfter"));
    kl->addWidget(keepBefore);
    kl->addWidget(m_keep);
    kl->addWidget(keepAfter);
    kl->addStretch(1);
    bf->addRow(keepRow);
    auto *row = new QHBoxLayout;
    // W4b: the portable program has no standard backup folder (the writer's answer 4b of 2026-11-11)
    m_folder = new QLineEdit(s.backupDir.isEmpty() && !portable ? backup::defaultDirectory() : QDir::toNativeSeparators(s.backupDir),
                             backups);
    m_folder->setObjectName(QStringLiteral("firstBackupFolder"));
    if (portable)
        m_folder->setPlaceholderText(tr("Choose a folder: until then no backups are made"));
    auto *choose = new QPushButton(tr("Choose…"), backups);
    row->addWidget(m_folder, 1);
    row->addWidget(choose);
    bf->addRow(tr("Folder:"), row);
    // 0.70.0 (audit A U-05): a backup on the same drive is lost with it
    auto *driveHint = new QLabel(tr("Best on another drive (a USB stick, a second disk, a folder your sync program "
                                    "copies), so that the backups outlive a failing disk. File > Back Up > Restore From "
                                    "Backup brings one back."),
                                 backups);
    driveHint->setObjectName(QStringLiteral("firstBackupHint"));
    hintlabel::style(driveHint);
    bf->addRow(driveHint);
    connect(choose, &QPushButton::clicked, this, [this] {
        const QString d = QFileDialog::getExistingDirectory(this, tr("Folder for Backups"), m_folder->text());
        if (!d.isEmpty())
            m_folder->setText(QDir::toNativeSeparators(d));
    });
    auto enable = [this](bool on) {
        m_keep->setEnabled(on);
        m_folder->setEnabled(on);
    };
    connect(m_backup, &QCheckBox::toggled, this, enable);
    enable(m_backup->isChecked());
    v->addWidget(backups);

    // 0.94.0: the application menu (the AppImage on Linux) and the check for updates, on this page too, so that the
    // first start has one window and one OK
    if (!again && appmenu::runsFromAppImage()) {
        auto *menu = new QGroupBox(tr("Application Menu"), body);
        auto *mv = new QVBoxLayout(menu);
        m_menuAdd = new QCheckBox(tr("Add %1 to the application menu, so that you can start it like any other program and "
                                     "open projects by double-clicking them")
                                      .arg(QLatin1String(QB_APP_NAME)),
                                  menu);
        m_menuAdd->setObjectName(QStringLiteral("firstMenuAdd"));
        m_menuAdd->setChecked(true);
        mv->addWidget(m_menuAdd);
        if (appmenu::canMove()) {
            m_menuMove = new QCheckBox(tr("Move the AppImage to %1, a fixed place the menu entry starts it from")
                                           .arg(QDir::toNativeSeparators(appmenu::applicationsFolder())),
                                       menu);
            m_menuMove->setObjectName(QStringLiteral("firstMenuMove"));
            m_menuMove->setChecked(true);
            mv->addWidget(m_menuMove);
            connect(m_menuAdd, &QCheckBox::toggled, m_menuMove, &QWidget::setEnabled);
        }
        auto *later = new QLabel(tr("Help > Add to Application Menu or Remove From Application Menu changes this later. If you move the AppImage "
                                "yourself, add it to the menu again."), menu);
        later->setObjectName(QStringLiteral("firstMenuHint"));
        later->setWordWrap(true);
        hintlabel::style(later);
        mv->addWidget(later);
        v->addWidget(menu);
    }
    if (!again && !qEnvironmentVariableIsSet("QB_NO_UPDATE_CHECK") && !updates::State::load().noteShown) {
        auto *upd = new QGroupBox(tr("Check for Updates"), body);
        auto *uv = new QVBoxLayout(upd);
        m_updates = new QCheckBox(tr("Look for updates when the program starts"), upd);
        m_updates->setObjectName(QStringLiteral("firstUpdates"));
        m_updates->setChecked(true);
        uv->addWidget(m_updates);
        auto *note = new QLabel(tr("%1 fetches one small file, at most once a day, and tells you when a newer version of the "
                                   "program or of an installed add-on is there. Nothing about you is sent. You can switch this "
                                   "off or on at any time with Help > Add-ons and Updates > Check for Updates at Startup.")
                                    .arg(QLatin1String(QB_APP_NAME)),
                                upd);
        note->setObjectName(QStringLiteral("firstUpdatesNote"));
        note->setWordWrap(true);
        hintlabel::style(note);
        uv->addWidget(note);
        v->addWidget(upd);
    }

    if (portable) {
        m_setup = new PortableSetup(body);
        v->addWidget(m_setup);
        v->addStretch(1);
    }

    auto *bb = new QDialogButtonBox(again ? QDialogButtonBox::Ok | QDialogButtonBox::Cancel : QDialogButtonBox::Ok, this);
    outer->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    if (portable) {
        // as tall as the content needs, but not taller than the screen (the guide scrolls then)
        const int want = body->sizeHint().height() + bb->sizeHint().height() + 40;
        const QScreen *screen = parent ? parent->screen() : nullptr;
        const int room = screen ? screen->availableGeometry().height() - 80 : 900;
        resize(760, qMin(want, qMax(400, room)));
    } else {
        resize(620, sizeHint().height());
    }
}

void FirstStartDialog::take()
{
    AppSettings &s = AppSettings::instance();
    if (m_language && language::available().contains(m_language->currentData().toString()))
        s.language = m_language->currentData().toString();
    s.backupOnClose = m_backup->isChecked();
    s.backupKeep = m_keep->value();
    const QString folder = QDir::fromNativeSeparators(m_folder->text().trimmed());
    if (m_setup) {
        // W4b: an empty folder means no backups until one is chosen
        s.backupDir = folder;
        if (!folder.isEmpty())
            QDir().mkpath(folder);
        m_setup->apply();
    } else {
        s.backupDir = folder.isEmpty() ? backup::defaultDirectory() : folder;
        QDir().mkpath(s.backupDir);
    }
    s.save();
    markDone();
}

bool FirstStartDialog::addToMenu() const { return m_menuAdd && m_menuAdd->isChecked(); }

bool FirstStartDialog::moveAppImage() const { return addToMenu() && m_menuMove && m_menuMove->isChecked(); }

bool FirstStartDialog::checkForUpdates() const { return m_updates && m_updates->isChecked(); }

bool FirstStartDialog::due()
{
    if (QB_TEST_ENV_SET("QB_NO_DIALOGS") || QB_TEST_ENV_SET("QB_SKIP_FIRST_START"))
        return false;
    QSettings p(settingslayer::programFile(), QSettings::IniFormat);
    return !p.value(QStringLiteral("app/firstStartDone")).toBool();
}

void FirstStartDialog::markDone()
{
    QSettings p(settingslayer::programFile(), QSettings::IniFormat);
    p.setValue(QStringLiteral("app/firstStartDone"), true);
    p.sync();
}

} // namespace qb
