#include "ui/ProfileDialogs.h"

#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "ui/Profiles.h"
#include "ui/Theme.h"
#include "ui/WindowCare.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QLineEdit>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace qb {

namespace profileui {

namespace {
int s_placesAnswer = -1;
}

void setPlacesAnswerForTests(int answer) { s_placesAnswer = answer; }

QString askName(QWidget *parent, const QString &title, const QString &suggestion, const QString &except)
{
    QString name = suggestion;
    while (true) {
        bool ok = false;
        name = QInputDialog::getText(parent, title, QObject::tr("Name of the profile:"), QLineEdit::Normal, name, &ok).trimmed();
        if (!ok)
            return QString();
        if (name.isEmpty())
            continue;
        if (!profiles::nameInUse(name, except))
            return name;
        QMessageBox::information(parent, title, QObject::tr("There is already a profile named “%1”. Choose another name.").arg(name));
    }
}

QString importWithQuestion(QWidget *parent, const QString &file, QString *error)
{
    QStringList refused;
    const QStringList places = profiles::placesIn(file);
    if (!places.isEmpty()) {
        QSettings from(file, QSettings::IniFormat);
        QStringList shown;
        for (const QString &k : places)
            shown << QStringLiteral("%1 = %2").arg(k, from.value(k).toString().left(200));
        bool take = false;
        if (s_placesAnswer >= 0) {
            take = s_placesAnswer == 1;
        } else {
            QMessageBox ask(QMessageBox::Question, QObject::tr("Import Profile"),
                            QObject::tr("The profile also names programs, folders, files or addresses (a folder may not exist on "
                                        "this computer):\n\n%1\n\nTake these too? Take them only if you made the file yourself or "
                                        "trust where it comes from.")
                                .arg(shown.join(QLatin1Char('\n'))),
                            QMessageBox::Yes | QMessageBox::No, parent);
            ask.setObjectName(QStringLiteral("importProfilePlaces"));
            ask.setDefaultButton(QMessageBox::No);
            take = ask.exec() == QMessageBox::Yes;
        }
        if (!take)
            refused = places;
    }
    return profiles::importFrom(file, refused, error);
}

} // namespace profileui

ProfileManagerDialog::ProfileManagerDialog(MainWindow *window) : QDialog(window), m_window(window)
{
    setObjectName(QStringLiteral("profileManager"));
    setWindowTitle(tr("Manage Profiles"));
    resize(520, 380);
    auto *v = new QVBoxLayout(this);
    v->addWidget(new QLabel(tr("A profile holds your options, toolbars, window places and sizes, keyboard set, backup settings, "
                               "standard colors, layouts and the theme. The profile in use is saved when you switch away from it "
                               "and when the program ends."),
                            this));
    static_cast<QLabel *>(v->itemAt(0)->widget())->setWordWrap(true);
    auto *row = new QHBoxLayout;
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("profileList"));
    theme::panelFields(m_list);
    row->addWidget(m_list, 1);
    auto *col = new QVBoxLayout;
    auto button = [&](const QString &text, const char *name) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(QLatin1String(name));
        col->addWidget(b);
        return b;
    };
    m_use = button(tr("Use"), "profileUse");
    m_new = button(tr("Save Settings as New Profile..."), "profileNew");
    m_rename = button(tr("Rename..."), "profileRename");
    m_copy = button(tr("Copy..."), "profileCopy");
    m_delete = button(tr("Delete"), "profileDelete");
    col->addSpacing(8);
    m_export = button(tr("Export..."), "profileExport");
    m_import = button(tr("Import..."), "profileImport");
    col->addStretch(1);
    row->addLayout(col);
    v->addLayout(row, 1);
    m_info = new QLabel(this);
    m_info->setObjectName(QStringLiteral("profileInfo"));
    m_info->setWordWrap(true);
    v->addWidget(m_info);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::accept);
    v->addWidget(box);

    connect(m_list, &QListWidget::currentRowChanged, this, [this] { updateButtons(); });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] { useSelected(); });
    connect(m_use, &QPushButton::clicked, this, [this] { useSelected(); });
    connect(m_new, &QPushButton::clicked, this, [this] {
        const QString name = profileui::askName(this, tr("Save Settings as New Profile"), profiles::uniqueName(tr("My Profile")));
        if (name.isEmpty())
            return;
        QString err;
        if (saveNew(name, &err).isEmpty())
            QMessageBox::warning(this, tr("Save Settings as New Profile"), err);
    });
    connect(m_rename, &QPushButton::clicked, this, [this] {
        const QString old = selectedName();
        const QString name = profileui::askName(this, tr("Rename Profile"), old, old);
        if (name.isEmpty() || name == old)
            return;
        QString err;
        if (!profiles::rename(old, name, &err))
            QMessageBox::warning(this, tr("Rename Profile"), err);
        reload(profiles::exists(name) ? name : old);
        m_window->refreshProfileUi();
    });
    connect(m_copy, &QPushButton::clicked, this, [this] {
        const QString old = selectedName();
        const QString name = profileui::askName(this, tr("Copy Profile"), profiles::uniqueName(tr("%1 (Copy)").arg(old)));
        if (name.isEmpty())
            return;
        QString err;
        if (!profiles::duplicate(old, name, &err))
            QMessageBox::warning(this, tr("Copy Profile"), err);
        reload(name);
        m_window->refreshProfileUi();
    });
    connect(m_delete, &QPushButton::clicked, this, [this] {
        const QString name = selectedName();
        if (QMessageBox::question(this, tr("Delete Profile"), tr("Delete the profile “%1”? This cannot be undone.").arg(name),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes)
            return;
        QString err;
        if (!profiles::remove(name, &err))
            QMessageBox::warning(this, tr("Delete Profile"), err);
        reload();
        m_window->refreshProfileUi();
    });
    connect(m_export, &QPushButton::clicked, this, [this] {
        const QString name = selectedName();
        QString file = QFileDialog::getSaveFileName(this, tr("Export Profile"), QDir::homePath() + QLatin1Char('/') + name + QLatin1String(".qbprofile"),
                                                    tr("Profiles (*.qbprofile)"));
        if (file.isEmpty())
            return;
        if (QFileInfo(file).suffix().isEmpty())
            file += QLatin1String(".qbprofile");
        QString err;
        if (!profiles::exportTo(name, file, &err))
            QMessageBox::warning(this, tr("Export Profile"), err);
    });
    connect(m_import, &QPushButton::clicked, this, [this] {
        const QString file = QFileDialog::getOpenFileName(this, tr("Import Profile"), QDir::homePath(),
                                                          tr("Profiles and options (*.qbprofile *.qboptions)"));
        if (file.isEmpty())
            return;
        QString err;
        if (!importFile(file, &err))
            QMessageBox::warning(this, tr("Import Profile"), err);
    });
    reload(profiles::active());
}

QString ProfileManagerDialog::selectedName() const
{
    const QListWidgetItem *it = m_list->currentItem();
    return it ? it->data(Qt::UserRole).toString() : QString();
}

void ProfileManagerDialog::select(const QString &name)
{
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->data(Qt::UserRole).toString() == name)
            m_list->setCurrentRow(i);
}

void ProfileManagerDialog::reload(const QString &selectName)
{
    const QString keep = selectName.isEmpty() ? selectedName() : selectName;
    m_list->clear();
    const QString inUse = profiles::active();
    for (const QString &n : profiles::names()) {
        auto *it = new QListWidgetItem(n.compare(inUse, Qt::CaseInsensitive) == 0 ? tr("%1 (in use)").arg(n) : n, m_list);
        it->setData(Qt::UserRole, n);
    }
    select(keep);
    if (m_list->currentRow() < 0 && m_list->count())
        m_list->setCurrentRow(0);
    updateButtons();
}

void ProfileManagerDialog::updateButtons()
{
    const QString name = selectedName();
    const bool any = !name.isEmpty();
    const bool inUse = any && name.compare(profiles::active(), Qt::CaseInsensitive) == 0;
    m_use->setEnabled(any && !inUse);
    m_rename->setEnabled(any && !profiles::isBuiltIn(name) && profiles::canDelete(name));
    m_copy->setEnabled(any);
    m_delete->setEnabled(any && !inUse && profiles::canDelete(name));
    m_export->setEnabled(any);
    QString text;
    if (any) {
        profiles::Content c;
        if (profiles::read(name, &c) || inUse) {
            const QString themeId = inUse ? AppSettings::instance().theme : c.themeId;
            QString themeName;
            for (const theme::Entry &e : theme::available())
                if (e.id == themeId)
                    themeName = e.name;
            text = themeName.isEmpty() ? QString() : tr("Theme: %1.").arg(themeName);
        }
        if (profiles::isBuiltIn(name))
            text += (text.isEmpty() ? QString() : QStringLiteral(" ")) + tr("Built-in profile; it can be deleted.");
        else if (!profiles::canDelete(name))
            text += (text.isEmpty() ? QString() : QStringLiteral(" ")) + tr("The profile every program starts with.");
    }
    m_info->setText(text);
}

bool ProfileManagerDialog::useSelected()
{
    const QString name = selectedName();
    if (name.isEmpty() || !m_window->switchProfile(name))
        return false;
    reload(name);
    return true;
}

QString ProfileManagerDialog::saveNew(const QString &name, QString *error)
{
    if (!profiles::createFromCurrent(name, error))
        return QString();
    reload(name);
    m_window->refreshProfileUi();
    return name;
}

bool ProfileManagerDialog::importFile(const QString &file, QString *error)
{
    const QString name = profileui::importWithQuestion(this, file, error);
    if (name.isEmpty())
        return false;
    reload(name);
    m_window->refreshProfileUi();
    return true;
}

} // namespace qb
