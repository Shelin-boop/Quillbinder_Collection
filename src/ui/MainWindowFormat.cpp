// File > Save As and File > Export (copies): projects in Scrivener 3 format (.scriv) and in the
// program's own format, and converting between them (import/ScrivenerFormat.h).
#include "ui/MainWindow.h"
#include "core/crypto/Vault.h"
#include "ui/Ask.h"

#include "core/Platform.h"
#include "core/Project.h"
#include "import/ScrivenerFormat.h"
#include "text/TextUtil.h"
#include "ui/AppSettings.h"
#include "ui/MindMapDoc.h"

#include "AppInfo.h"

#include <cstring>

#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSysInfo>
#include <QUrl>

namespace qb {

namespace {

struct WaitCursor {
    WaitCursor() { QApplication::setOverrideCursor(Qt::WaitCursor); }
    ~WaitCursor() { QApplication::restoreOverrideCursor(); }
};

// "name.scriv" chosen in a save dialog
QString chooseScrivPath(QWidget *parent, const QString &title, const QString &suggestedName)
{
    QString start = AppSettings::instance().lastProjectDir;
    if (start.isEmpty() || !QDir(start).exists())
        start = QDir::homePath();
    QString f = QFileDialog::getSaveFileName(parent, title, QDir(start).filePath(text::safeFileName(suggestedName) + QLatin1String(".scriv")),
                                             QObject::tr("Scrivener projects (*.scriv)"));
    if (f.isEmpty())
        return QString();
    if (!f.endsWith(QLatin1String(".scriv"), Qt::CaseInsensitive))
        f += QLatin1String(".scriv");
    return f;
}

} // namespace

bool MainWindow::confirmNotOpenElsewhere(const QString &projectFolder)
{
    return confirmLock(projectFolder + QLatin1String("/settings/lock"));
}

bool MainWindow::openScrivenerProject(const QString &path)
{
    const QString scrivDir = scrivener::scrivFolder(path);
    if (scrivener::scrivxIn(scrivDir).isEmpty() || !QFileInfo(scrivener::scrivxIn(scrivDir)).isFile()) {
        QMessageBox::warning(this, tr("Open Project"), tr("No Scrivener project was found at\n%1").arg(path));
        AppSettings::instance().removeRecent(path);
        return false;
    }
    if (m_project && QFileInfo(m_project->scrivenerPath) == QFileInfo(scrivDir))
        return true;
    const QString wc = scrivener::workingCopyFor(scrivDir);
    if (wc.isEmpty()) {
        // the first time: work on it in Scrivener format, or make a copy in our format
        QMessageBox box(QMessageBox::Question, tr("Open Scrivener Project"),
                        tr("\"%1\" is a Scrivener project.\n\n"
                           "Work in Scrivener format: %2 saves your changes into the .scriv, so you can go on in "
                           "Scrivener as well (not both programs at the same time).\n\n"
                           "Convert: %2 makes a project of its own from it; the Scrivener project is not changed.")
                            .arg(QFileInfo(scrivDir).fileName(), QStringLiteral(QB_APP_NAME)),
                        QMessageBox::NoButton, this);
        box.setObjectName(QStringLiteral("openScrivenerQuestion"));
        QPushButton *work = box.addButton(tr("Work in Scrivener Format"), QMessageBox::AcceptRole);
        work->setObjectName(QStringLiteral("workInScrivener"));
        QPushButton *convert = box.addButton(tr("Convert…"), QMessageBox::ActionRole);
        convert->setObjectName(QStringLiteral("convertScrivener"));
        box.addButton(QMessageBox::Cancel);
        box.setDefaultButton(work);
        box.exec();
        if (box.clickedButton() == convert) {
            a[QStringLiteral("importScrivener")]->setData(scrivDir);
            a[QStringLiteral("importScrivener")]->trigger();
            a[QStringLiteral("importScrivener")]->setData(QVariant());
            return m_project != nullptr;
        }
        if (box.clickedButton() != work)
            return false;
    } else if (!confirmNotOpenElsewhere(wc)) {
        return false;
    }
    // 0.64: this program on another computer has the .scriv open (through a sync program)
    if (!confirmLock(scrivDir + QLatin1String("/quillbinder.lock")))
        return false;
    QString who;
    if (scrivener::lockedByScrivener(scrivDir, &who)
        && ask::question(this, tr("Open Scrivener Project"),
                                 tr("Scrivener has this project open (%1), or was not closed properly.\n\n"
                                    "If both programs work on it at the same time, changes are lost. Open it anyway?")
                                     .arg(who), QMessageBox::Yes | QMessageBox::No, QMessageBox::No, tr("Open Anyway"))
               != QMessageBox::Yes)
        return false;
    if (m_project && !closeProject())
        return false;
    QString err;
    scrivener::OpenResult r;
    {
        WaitCursor wait;
        r = scrivener::open(scrivDir, &err);
    }
    if (!r.project) {
        QMessageBox::warning(this, tr("Open Scrivener Project"), err.isEmpty() ? tr("The project could not be opened.") : err);
        return false;
    }
    const bool ok = adoptProject(std::move(r.project));
    QStringList notes;
    if (ok && r.reread && r.changedElsewhere > 0)
        notes << (r.changedElsewhere == 1
                      ? tr("1 document changed outside %1 since the last time; its new text was read.").arg(QStringLiteral(QB_APP_NAME))
                      : tr("%1 documents changed outside %2 since the last time; their new text was read.")
                            .arg(QString::number(r.changedElsewhere), QStringLiteral(QB_APP_NAME)));
    if (ok)
        notes << r.warnings;
    if (!notes.isEmpty())
        showStatusNote(notes.join(QLatin1Char(' ')), 10000);
    return ok;
}

// File > Save As: the name and place in a save dialog whose file types are the two formats; the type decides the format
// (a name that ends in .scriv, .qbx or .quillbinder decides as well). A project is a folder: the dialog asks for a name,
// the program makes the folder.
struct SaveAsChoice {
    QString parent; // the folder the new project goes into
    QString name;   // the project's name (its title)
    bool scriv = false;
};

static bool chooseSaveAs(QWidget *parent, const QString &title, const QString &suggestedName, const QString &startDir,
                         bool currentIsScriv, SaveAsChoice *out)
{
    const QString scrivFilter = QObject::tr("Scrivener 3 Project (*.scriv)");
    const QString ownFilter = QObject::tr("%1 Project (*.qbx)").arg(QStringLiteral(QB_APP_NAME));
    QString start = startDir;
    if (start.isEmpty() || !QDir(start).exists())
        start = AppSettings::instance().lastProjectDir;
    if (start.isEmpty() || !QDir(start).exists())
        start = QDir::homePath();
    QString selected = currentIsScriv ? scrivFilter : ownFilter;
    bool scriv = currentIsScriv;
#ifdef Q_OS_ANDROID
    // the file picker of Android may offer no file types: the format is asked first
    {
        const QStringList kinds{ownFilter, scrivFilter};
        bool ok = false;
        const QString kind = QInputDialog::getItem(parent, title, QObject::tr("Format:"), kinds, currentIsScriv ? 1 : 0, false, &ok);
        if (!ok)
            return false;
        selected = kind;
        scriv = kind == scrivFilter;
    }
#endif
    const QString suggested = text::safeFileName(suggestedName) + (scriv ? QLatin1String(".scriv") : QLatin1String(QB_PROJECT_FILE_SUFFIX));
    const QString filters = (scriv ? scrivFilter + QLatin1String(";;") + ownFilter : ownFilter + QLatin1String(";;") + scrivFilter);
    const QString f = QFileDialog::getSaveFileName(parent, title, QDir(start).filePath(suggested), filters, &selected);
    if (f.isEmpty())
        return false;
    const QFileInfo fi(f);
    QString name = fi.fileName();
    bool isScriv = selected == scrivFilter;
    auto cut = [&name](const char *suffix) {
        if (name.endsWith(QLatin1String(suffix), Qt::CaseInsensitive)) {
            name.chop(int(strlen(suffix)));
            return true;
        }
        return false;
    };
    if (cut(".scriv"))
        isScriv = true;
    else if (cut(QB_PROJECT_FILE_SUFFIX) || cut(QB_PROJECT_SUFFIX))
        isScriv = false;
    name = name.trimmed();
    if (name.isEmpty())
        return false;
    out->parent = fi.absolutePath();
    out->name = name;
    out->scriv = isScriv;
    return true;
}

void MainWindow::saveProjectAs()
{
    if (!m_project)
        return;
    SaveAsChoice c;
    const bool wasScriv = !m_project->scrivenerPath.isEmpty();
    // from the dialog on, nothing more is written into the original (the window's own save when a dialog takes over
    // included): what is not saved yet goes into the new project only
    struct Leaving {
        Project *p;
        ~Leaving()
        {
            if (p)
                p->setLeaving(false);
        }
    } leaving{m_project.get()};
    m_project->setLeaving(true);
    if (!chooseSaveAs(this, tr("Save As"), m_project->title(), QFileInfo(m_project->displayFolder()).absolutePath(), wasScriv, &c))
        return;
    // 0.99.0 (item 113): Scrivener cannot read encrypted files, and a plain copy is not made without the writer's word
    if (c.scriv && m_project->isEncrypted()) {
        QMessageBox box(QMessageBox::Information, tr("Save As"), encryptedScrivenerText(), QMessageBox::Ok, this);
        box.setObjectName(QStringLiteral("encryptedNoScrivener"));
        box.exec();
        return;
    }
    const QString target = c.scriv ? QDir(c.parent).filePath(c.name + QLatin1String(".scriv"))
                                   : QDir(c.parent).filePath(text::safeFileName(c.name) + QLatin1String(QB_PROJECT_SUFFIX));
    if (QFileInfo::exists(target)) {
        QMessageBox::warning(this, tr("Save As"), tr("There is a file or folder with this name already:\n%1").arg(QDir::toNativeSeparators(target)));
        return;
    }
    saveUiState();
    Project *old = m_project.get();
    QString err;
    // what the open project holds unsaved goes into the new folder only (the original is not written)
    auto into = [old](const QString &folder, const QString &indexName, QString *e) {
        vault::Scope encrypted(folder); // 0.99.0: the copy of an encrypted project stays encrypted (its marker came along)
        QString why;
        bool ok = old->writeUnsavedInto(folder, indexName, &why);
        if (!mindmaps::writeDirtyInto(old, folder, &why))
            ok = false;
        if (!ok && e)
            *e = why;
        return ok;
    };
    std::unique_ptr<Project> np;
    {
        WaitCursor wait;
        if (c.scriv)
            np = scrivener::convertToScrivener(old, target, &err, false,
                                               [&into](const QString &folder, QString *e) { return into(folder, QString(), e); }, c.name);
        else
            np = scrivener::saveAsQuillbinder(old, c.parent, c.name, &err, into);
        if (np) {
            np->uiState = old->uiState;
            np->markUiStateDirty();
            np->save(&err);
        }
    }
    if (!np) {
        QMessageBox::warning(this, tr("Save As"), err);
        return;
    }
    const QString where = np->displayFolder();
    const QString from = old->displayFolder();
    // the changes went into the new project: the original is left as it was (nothing is written into it when the window
    // leaves it)
    mindmaps::forgetUnsaved(old);
    if (!adoptProject(std::move(np))) {
        QMessageBox::warning(this, tr("Save As"),
                             tr("The project was saved as\n%1\n\nbut this window could not leave the original. Open the new project from File > Open Recent.").arg(QDir::toNativeSeparators(where)));
        return;
    }
    leaving.p = nullptr; // the original is gone from the window
    showStatusNote(tr("Saved as %1. The original stays as it was: %2").arg(QDir::toNativeSeparators(where), QDir::toNativeSeparators(from)), 15000);
}

QString MainWindow::encryptedScrivenerText()
{
    return tr("This project is encrypted, and Scrivener cannot read encrypted files. To save it in Scrivener format, turn "
              "off the encryption of the project first (File > Options > Encryption); a copy in Quillbinder's format stays "
              "encrypted.");
}

void MainWindow::saveCopyAsScrivener()
{
    if (!m_project)
        return;
    if (m_project->isEncrypted()) { // 0.99.0 (item 113)
        QMessageBox box(QMessageBox::Information, tr("Save a Copy"), encryptedScrivenerText(), QMessageBox::Ok, this);
        box.setObjectName(QStringLiteral("encryptedNoScrivener"));
        box.exec();
        return;
    }
    const QString dir = chooseScrivPath(this, tr("Save a Copy as Scrivener Project"), m_project->title());
    if (dir.isEmpty())
        return;
    if (QFileInfo::exists(dir)) {
        QMessageBox::warning(this, tr("Save a Copy"), tr("There is a file or folder with this name already:\n%1").arg(dir));
        return;
    }
    saveUiState();
    QString err;
    scrivener::WriteReport rep;
    bool ok;
    {
        WaitCursor wait;
        ok = scrivener::saveCopyAsScrivener(m_project.get(), dir, &rep, &err);
    }
    if (!ok) {
        QMessageBox::warning(this, tr("Save a Copy"), err);
        return;
    }
    QMessageBox box(QMessageBox::Information, tr("Save a Copy"),
                    tr("A copy of the project was saved in Scrivener format:\n%1\n\nThe project you are working on stays "
                       "where it is.")
                        .arg(dir),
                    QMessageBox::NoButton, this);
    box.setObjectName(QStringLiteral("copySaved"));
    QPushButton *show = box.addButton(tr("Show Folder"), QMessageBox::ActionRole);
    box.addButton(QMessageBox::Ok);
    box.exec();
    if (box.clickedButton() == show)
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(dir).absolutePath()));
}

void MainWindow::saveCopyAsQuillbinder()
{
    if (!m_project)
        return;
    const QString parent = QFileDialog::getExistingDirectory(
        this, tr("Save a Copy as %1 Project").arg(QStringLiteral(QB_APP_NAME)),
        QFileInfo(m_project->displayFolder()).absolutePath());
    if (parent.isEmpty())
        return;
    saveUiState();
    QString err;
    std::unique_ptr<Project> copy;
    {
        WaitCursor wait;
        copy = scrivener::copyAsQuillbinder(m_project.get(), parent, m_project->title(), &err);
    }
    if (!copy) {
        QMessageBox::warning(this, tr("Save a Copy"), err);
        return;
    }
    const QString where = copy->folderPath();
    copy.reset();
    QMessageBox box(QMessageBox::Information, tr("Save a Copy"),
                    tr("A copy of the project was saved in %1's own format:\n%2\n\nThe project you are working on stays "
                       "where it is.")
                        .arg(QStringLiteral(QB_APP_NAME), where),
                    QMessageBox::NoButton, this);
    box.setObjectName(QStringLiteral("copySaved"));
    QPushButton *show = box.addButton(tr("Show Folder"), QMessageBox::ActionRole);
    box.addButton(QMessageBox::Ok);
    box.exec();
    if (box.clickedButton() == show)
        QDesktopServices::openUrl(QUrl::fromLocalFile(parent));
}

} // namespace qb
