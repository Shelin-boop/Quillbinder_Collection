// The system test, stage 2: helpers shared by the system, usability and whole-program cases.
#include "cs.h"
#include <QPdfDocument>
#include <QPdfSelection>

#include "core/Zip.h"
#include "ui/BinderPanel.h"
#include "ui/EditorPane.h"
#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <utility>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QTest>
#include <QTreeView>

namespace cs {

using namespace qb;

bool menuPath(MainWindow *w, const QString &path)
{
    return chooseMenu(w, path.split(QStringLiteral(" > ")));
}

static QString plainLabel(QString t)
{
    return t.remove(QLatin1Char('&')).trimmed();
}

void clickLater(int ms, const QString &buttonText, int tries)
{
    QTimer::singleShot(ms, qApp, [buttonText, tries] {
        for (QWidget *tl : QApplication::topLevelWidgets()) {
            auto *d = qobject_cast<QDialog *>(tl);
            if (!d || !d->isVisible())
                continue;
            for (QAbstractButton *b : d->findChildren<QAbstractButton *>())
                if (b->isVisible() && b->isEnabled() && plainLabel(b->text()).contains(buttonText, Qt::CaseInsensitive)) {
                    note(QStringLiteral("(%1: %2) %3").arg(d->objectName().isEmpty() ? d->windowTitle() : d->objectName(), plainLabel(b->text()),
                                                         qobject_cast<QMessageBox *>(d) ? qobject_cast<QMessageBox *>(d)->text().left(120) : QString()));
                    b->click();
                    return;
                }
        }
        if (tries > 0)
            clickLater(150, buttonText, tries - 1);
        else
            note(QStringLiteral("(no window with a button \"%1\" came)").arg(buttonText));
    });
}

void answerBoxLater(int ms, QMessageBox::StandardButton button, BoxText *text)
{
    std::shared_ptr<bool> alive = text ? text->alive : nullptr;
    withModal<QMessageBox>(ms, [button, text, alive](QMessageBox *b) {
        if (text)
            *text = b->text();
        if (QAbstractButton *x = b->button(button))
            x->click();
        else if (button == QMessageBox::Cancel || button == QMessageBox::No)
            b->reject();
        else
            b->accept();
    }, 100, alive);
}

void fileDialogLater(int ms, const QString &path, bool *asked)
{
    withModal<QFileDialog>(ms, [path, asked](QFileDialog *fd) {
        if (asked)
            *asked = true;
        const QFileInfo fi(path);
        if (fi.isDir()) {
            fd->setDirectory(path);
            fd->selectFile(path);
            // 0.64.2: the same for a folder: the name field (focused) keeps what it had otherwise
            if (auto *name = fd->findChild<QLineEdit *>(QStringLiteral("fileNameEdit")); name && fd->fileMode() == QFileDialog::Directory)
                name->setText(path);
        } else {
            fd->setDirectory(fi.absolutePath());
            fd->selectFile(path);
            // 0.64.2: Qt does not change the name field while it has the focus (a save dialog gives it the
            // focus), so the suggested name stayed and S-C01 waited for a file under another name
            if (auto *name = fd->findChild<QLineEdit *>(QStringLiteral("fileNameEdit")))
                name->setText(fd->fileMode() == QFileDialog::ExistingFiles || fd->acceptMode() == QFileDialog::AcceptOpen
                                  ? path : fi.fileName());
        }
        static_cast<QDialog *>(fd)->accept();
    }, 150);   // up to about 22 s: the menus are opened by mouse first (shakedown 2026-11-09)
}

void closeDialogsLater(int ms)
{
    QTimer::singleShot(ms, qApp, [] {
        for (QWidget *tl : QApplication::topLevelWidgets())
            if (auto *d = qobject_cast<QDialog *>(tl); d && d->isVisible())
                d->reject();
    });
}

BinderPanel *binderOf(MainWindow *w)
{
    return w ? w->findChild<BinderPanel *>() : nullptr;
}

void selectItems(MainWindow *w, const QList<BinderItem *> &items)
{
    if (BinderPanel *bp = binderOf(w)) {
        bp->select(items, true);
        bp->tree()->setFocus(Qt::OtherFocusReason);
        settle(150);
    }
}

QString projectPathOf(Project *p)
{
    if (!p)
        return QString();
    return p->scrivenerPath.isEmpty() ? p->indexPath() : p->scrivenerPath;
}

MainWindow *reopenWindow(MainWindow *w)
{
    if (!w || !w->project())
        return w;
    const QString path = projectPathOf(w->project());
    QString err;
    w->project()->save(&err);
    w->closeProject();
    settle(400);
    dropWindow(w);
    return openWindow(path);
}

QString docText(Project *p, BinderItem *it)
{
    return p && it ? rawText(p->docs()->text(it->id)) : QString();
}

QString pdfToText(const QString &pdf, bool layout)
{
    // Android copy (2026-11-16): a tablet has no pdftotext (poppler-utils): Qt PDF (the APK has it) reads the text,
    // page by page with a form feed between pages as pdftotext writes it; "layout" (pdftotext's columns) has no
    // counterpart there and gives the same text
    Q_UNUSED(layout)
    QPdfDocument doc;
    if (doc.load(pdf) != QPdfDocument::Error::None)
        return QString();
    QStringList pages;
    for (int i = 0; i < doc.pageCount(); ++i)
        pages << doc.getAllText(i).text().replace(QLatin1String("\r\n"), QLatin1String("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));
    const QString t = pages.join(QChar(0x0C));
    // 0.64.2: typographic ligatures ("ﬁ", "ﬂ") as their letters, so that words are found
    QString out = t;
    for (const auto &pr : {std::pair<ushort, const char *>{0xFB00, "ff"}, {0xFB01, "fi"}, {0xFB02, "fl"}, {0xFB03, "ffi"}, {0xFB04, "ffl"}, {0xFB05, "st"}, {0xFB06, "st"}})
        out.replace(QChar(pr.first), QLatin1String(pr.second));
    return out;
}

QByteArray zipFile(const QString &zip, const QString &name)
{
    ZipReader z(zip);
    return z.fileData(name);
}

QString variant()
{
    return qEnvironmentVariable("QB_CS_VARIANT");
}

int subCase(const QString &variantName, const QMap<QString, QString> &env, QString *summary)
{
    // Android copy (2026-11-16): an app on a tablet cannot start a second copy of itself. The sub-run is asked of the
    // script that starts the cases (packaging/android/android-system-test.sh): one line in subcases.txt of the case's
    // folder ("<variant>" and NAME=VALUE pairs, tab-separated); the script starts the case again with that variant in
    // a fresh home (pm clear) and counts its result as a case of its own, "<case>@<variant>". The case here goes on
    // as if the sub-run had passed.
    QStringList fields = {variantName};
    for (auto i = env.begin(); i != env.end(); ++i)
        fields << (i.key() + QLatin1Char('=') + (i.value().isNull() ? QString() : i.value()));
    QFile f(ctx().dir + QStringLiteral("/subcases.txt"));
    if (f.open(QIODevice::Append | QIODevice::Text))
        f.write((fields.join(QLatin1Char('\t')) + QLatin1Char('\n')).toUtf8());
    step(QStringLiteral("variant %1: asked of the script, as the case %2@%1").arg(variantName, ctx().id));
    if (summary)
        summary->clear();
    return 0;
}

bool realDrag(QWidget *from, const QPoint &a, QWidget *to, const QPoint &b)
{
    // Android copy (2026-11-16): Linux makes a real drag and drop through the X server (xdotool); a tablet has none,
    // and a drag started inside the test would wait for the platform's own touches. The cases record it as not done
    // ("drag ...: xdotool failed" in Linux's words).
    Q_UNUSED(from)
    Q_UNUSED(a)
    Q_UNUSED(to)
    Q_UNUSED(b)
    return false;
}

QPoint binderPoint(MainWindow *w, BinderItem *it)
{
    BinderPanel *bp = binderOf(w);
    if (!bp || !it)
        return QPoint(-1, -1);
    bp->reveal(it);
    const QModelIndex ix = w->project()->model()->indexFor(it);
    bp->tree()->scrollTo(ix);
    settle(80);
    return bp->tree()->visualRect(ix).center();
}

void typeInto(QWidget *w, const QString &text)
{
    keysInto(w, text);
    settle(100);
}

} // namespace cs
