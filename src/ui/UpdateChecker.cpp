#include "ui/UpdateChecker.h"
#include "ui/MainWindow.h"

#include "core/Addons.h"
#include "core/TestSwitches.h"

#include "AppInfo.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QPushButton>
#include <QSslSocket>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>

#include <memory>

namespace qb {

namespace {
constexpr int kTimeoutMs = 10000;   // of one try
int retryDelayMs()
{
    bool ok = false;
    const int v = QB_TEST_ENV_INT("QB_TEST_UPDATE_RETRY_MS", &ok);
    return ok && v > 0 ? v : 5000;
}
// what is written while a file comes in
struct Incoming {
    QFile file;
    QCryptographicHash hash{QCryptographicHash::Sha256};
    qint64 size = 0;
    bool failed = false;
};
} // namespace

UpdateChecker::UpdateChecker(MainWindow *window) : QObject(window), m_window(window) {}

bool UpdateChecker::isEnabled() const { return updates::State::load().enabled; }

void UpdateChecker::setEnabled(bool on)
{
    updates::State st = updates::State::load();
    if (st.enabled == on)
        return;
    st.enabled = on;
    st.save();
}

QDialog *UpdateChecker::window() const { return m_offerWindow; }

QString UpdateChecker::ownAppImage()
{
    QString own = QB_TEST_ENV("QB_TEST_APPIMAGE");
    if (own.isEmpty())
        own = qEnvironmentVariable("APPIMAGE");
    return !own.isEmpty() && QFileInfo(own).isFile() ? QFileInfo(own).absoluteFilePath() : QString();
}

bool UpdateChecker::replaceOwnFile(const QString &own, const QString &fresh, QString *backup, QString *error)
{
    const QString aside = own + QLatin1String(".before-update");
    QFile::setPermissions(fresh, QFileInfo(own).permissions() | QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
                                     | QFileDevice::ExeGroup | QFileDevice::ExeOther);
    if (QFileInfo::exists(aside) && !QFile::remove(aside)) {
        if (error)
            *error = tr("%1 (the copy kept at the last update) could not be removed.").arg(QDir::toNativeSeparators(aside));
        return false;
    }
    if (!QFile::rename(own, aside)) {
        if (error)
            *error = tr("%1 could not be moved aside.").arg(QDir::toNativeSeparators(own));
        return false;
    }
    if (!QFile::rename(fresh, own)) {
        const bool back = QFile::rename(aside, own);
        if (error)
            *error = back ? tr("The new file could not be put in the place of %1. Nothing was changed.").arg(QDir::toNativeSeparators(own))
                          : tr("The new file could not be put in the place of %1, and that file could not be put back: it is "
                               "now %2. Rename it to its old name.")
                                .arg(QDir::toNativeSeparators(own), QDir::toNativeSeparators(aside));
        return false;
    }
    if (backup)
        *backup = aside;
    return true;
}

// ---------------------------------------------------------------- looking

void UpdateChecker::startAutomatic()
{
    updates::State st = updates::State::load();
    if (!st.enabled || m_busy)
        return;
    if (!st.noteShown) {
        // the first start: no contact before the writer has seen this
        auto *box = new QMessageBox(QMessageBox::Information, tr("Check for Updates"),
                                    tr("%1 looks for updates when it starts. Nothing about you is sent.\n\nIt fetches one small "
                                       "file, at most once a day, and tells you when a newer version of the program or of an "
                                       "installed add-on is there. You can switch this off or on at any time with Help > Check "
                                       "for Updates at Startup.")
                                        .arg(QStringLiteral(QB_APP_NAME)),
                                    QMessageBox::NoButton, m_window);
        box->setObjectName(QStringLiteral("updateFirstNote"));
        box->setAttribute(Qt::WA_DeleteOnClose);
        QPushButton *ok = box->addButton(QMessageBox::Ok);
        ok->setObjectName(QStringLiteral("updateNoteOk"));
        QPushButton *off = box->addButton(tr("Turn Off"), QMessageBox::RejectRole);
        off->setObjectName(QStringLiteral("updateNoteOff"));
        box->setDefaultButton(ok);
        connect(box, &QMessageBox::buttonClicked, this, [this, off](QAbstractButton *b) {
            updates::State now = updates::State::load();
            now.noteShown = true;
            now.enabled = b != off;
            now.save();
            emit enabledChanged(now.enabled);
            if (now.enabled)
                look(false);
        });
        m_window->showWithoutTakingKeys(box, ok);
        return;
    }
    if (updates::autoCheckDue(st, updates::now()))
        look(false);
}

void UpdateChecker::checkNow()
{
    if (m_offerWindow) {
        m_offerWindow->raise();
        m_offerWindow->activateWindow();
        return;
    }
    // asked for: the first start's note is not needed any more
    updates::State st = updates::State::load();
    if (!st.noteShown) {
        st.noteShown = true;
        st.save();
    }
    look(true);
}

void UpdateChecker::look(bool manual)
{
    if (m_busy)
        return;
    m_busy = true;
    m_manual = manual;
    m_try = 0;
    m_haveJson = false;
    m_json.clear();
    if (manual) {
        // a small window while the look runs; closing it ends the look
        m_progress = new QProgressDialog(tr("Looking for updates..."), tr("Cancel"), 0, 0, m_window);
        m_progress->setObjectName(QStringLiteral("updateLooking"));
        m_progress->setWindowTitle(tr("Check for Updates"));
        m_progress->setMinimumDuration(0);
        m_progress->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_progress, &QProgressDialog::canceled, this, [this] {
            if (!m_busy)
                return;
            m_busy = false;
            if (m_reply)
                m_reply->abort();
            emit finished(false, 0);
        });
        m_progress->show();
    }
    attempt();
}

QNetworkReply *UpdateChecker::get(const QString &address)
{
    if (!m_nam)
        m_nam = new QNetworkAccessManager(this);
    QNetworkRequest req{QUrl(address)};
    // a plain request: the program's name as the sender's name, nothing else (no version, no system, no cookies)
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral(QB_APP_NAME));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    req.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    req.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, QNetworkRequest::Manual);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    req.setTransferTimeout(kTimeoutMs);
    ++m_requests;
    return m_nam->get(req);
}

void UpdateChecker::attempt()
{
    if (!m_busy)
        return;
    const QString address = updates::feedAddress();
    if (address.startsWith(QLatin1String("https://")) && !QSslSocket::supportsSsl()) {
        // an AppImage of a version before 0.92.0 (a program update runs on its libraries) has no TLS part
        failed(tr("This copy of %1 cannot make a secure connection, so it cannot look for updates. Download the current "
                  "AppImage once from the project's page; from then on the check works.")
                   .arg(QStringLiteral(QB_APP_NAME)),
               true);
        return;
    }
    if (!m_haveJson)
        ++m_try;
    QNetworkReply *reply = get(m_haveJson ? address + QLatin1String(".sig") : address);
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply] { fetched(reply); });
}

void UpdateChecker::fetched(QNetworkReply *reply)
{
    reply->deleteLater();
    if (!m_busy || reply != m_reply)
        return;
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() == QNetworkReply::NoError && status == 200) {
        const QByteArray body = reply->read(4 * 1024 * 1024 + 1);
        if (!m_haveJson) {
            m_json = body;
            m_haveJson = true;
            attempt();   // its signature
            return;
        }
        evaluate(body);
        return;
    }
    if (status == 404 || status == 410) {
        // the address answers, the file is not there: no update information (not "no connection")
        failed(tr("No update information was found at the project's address."), true);
        return;
    }
    const QString why = reply->errorString();
    if (m_try < 2) {
        m_haveJson = false;
        QTimer::singleShot(retryDelayMs(), this, [this] { attempt(); });   // the second and last try
        return;
    }
    failed(tr("The update address could not be reached (%1).").arg(why), false);
}

void UpdateChecker::failed(const QString &why, bool quiet)
{
    qInfo("update check: %s", qPrintable(why));
    if (m_manual) {
        end(false, 0);
        QMessageBox box(QMessageBox::Information, tr("Check for Updates"), why, QMessageBox::Ok, m_window);
        box.setObjectName(QStringLiteral("updateNotReached"));
        box.exec();
        return;
    }
    if (!quiet) {
        // two tries were made: a small note below the editor, at most once a day, no window
        updates::State st = updates::State::load();
        const QDate today = updates::now().toLocalTime().date();
        if (st.unreachableTold != today) {
            st.unreachableTold = today;
            st.save();
            m_window->showFadingNote(tr("Could not look for updates (no connection to the update address)."));
        }
    }
    end(false, 0);
}

void UpdateChecker::end(bool reached, int offers)
{
    m_busy = false;
    if (m_progress) {
        QSignalBlocker b(m_progress);
        m_progress->close();
    }
    emit finished(reached, offers);
}

void UpdateChecker::evaluate(const QByteArray &signature)
{
    updates::State st = updates::State::load();
    const QDateTime now = updates::now();
    updates::Feed feed;
    QString err;
    auto tell = [this](const QString &text) {
        if (!m_manual)
            return;
        QMessageBox box(QMessageBox::Information, tr("Check for Updates"), text, QMessageBox::Ok, m_window);
        box.setObjectName(QStringLiteral("updateNothing"));
        box.exec();
    };
    if (!updates::readFeed(m_json, signature, &feed, &err)) {
        qInfo("update check: ignored (%s)", qPrintable(err));
        end(true, 0);
        tell(tr("The update information could not be trusted and was ignored: %1.").arg(err));
        return;
    }
    if (st.feedDate.isValid() && feed.date < st.feedDate) {
        // older than the newest one accepted: someone may be handing out an old list
        qInfo("update check: an older file was ignored");
        end(true, 0);
        tell(tr("The update information was older than the one %1 has already seen, and was ignored.").arg(QStringLiteral(QB_APP_NAME)));
        return;
    }
    st.feedDate = feed.date;
    st.lastCheck = now;
    st.save();
    const QList<updates::Offer> offers = updates::offersFor(feed, updates::systemName(), updates::installedHere());
    const updates::Shown shown = updates::toShow(offers, st, now, m_manual);
    end(true, int(shown.offers.size()));
    if (shown.offers.isEmpty()) {
        tell(tr("%1 and its add-ons are up to date.").arg(QStringLiteral(QB_APP_NAME)));
        return;
    }
    offer(shown);
}

// ---------------------------------------------------------------- the window

static QString delayWords(int days)
{
    if (days == 1)
        return UpdateChecker::tr("1 day");
    if (days == 7)
        return UpdateChecker::tr("1 week");
    return UpdateChecker::tr("%1 weeks").arg(days / 7);
}

void UpdateChecker::offer(const updates::Shown &shown)
{
    if (m_offerWindow)
        m_offerWindow->close();
    auto *dlg = new QDialog(m_window);
    m_offerWindow = dlg;
    dlg->setObjectName(QStringLiteral("updateWindow"));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Updates"));
    auto *lay = new QVBoxLayout(dlg);
    const bool several = shown.offers.size() > 1;
    auto *head = new QLabel(shown.security ? (several ? tr("Updates are available. One of them is recommended (security).")
                                                      : tr("A recommended update (security) is available."))
                                           : (several ? tr("Updates are available.") : tr("An update is available.")),
                            dlg);
    head->setObjectName(QStringLiteral("updateHeadline"));
    QFont bold = head->font();
    bold.setBold(true);
    head->setFont(bold);
    head->setWordWrap(true);
    lay->addWidget(head);

    QString html;
    bool newImage = false;
    for (const updates::Offer &o : shown.offers) {
        html += QStringLiteral("<p><b>%1</b> %2 → %3 &nbsp; <i>%4</i>")
                    .arg(o.name.toHtmlEscaped(), o.installed.toHtmlEscaped(), o.version.toHtmlEscaped(),
                         o.security ? tr("recommended (security)") : tr("functional"));
        for (const QString &line : o.whatsNew)
            html += QStringLiteral("<br>• ") + line.toHtmlEscaped();
        html += QStringLiteral("</p>");
        newImage = newImage || o.kind == updates::Offer::Kind::NewAppImage;
    }
    auto *list = new QLabel(html, dlg);
    list->setObjectName(QStringLiteral("updateList"));
    list->setTextFormat(Qt::RichText);
    list->setWordWrap(true);
    list->setMinimumWidth(460);
    lay->addWidget(list);

    QString notes;
    if (newImage) {
        const QString own = ownAppImage();
        if (!own.isEmpty() && QFileInfo(QFileInfo(own).absolutePath()).isWritable())
            notes = tr("The new program version needs a new AppImage. %1 downloads it, puts it in the place of the AppImage you "
                       "started (%2), keeps the one you have now beside it as \"%3\", and starts again.")
                        .arg(QStringLiteral(QB_APP_NAME), QDir::toNativeSeparators(own), QFileInfo(own).fileName() + QLatin1String(".before-update"));
        else
            notes = tr("The new program version needs a new AppImage. It is saved in your Downloads folder; close %1, put the "
                       "new file in the place of the old one and start it.")
                        .arg(QStringLiteral(QB_APP_NAME));
        notes += QStringLiteral("\n\n");
    }
    const QString wait = delayWords(shown.delayDays);
    if (shown.security)
        notes += tr("Delay: you are reminded again in %1. Each further delay waits longer (1 day, 1 week, 2 weeks, then every "
                    "4 weeks). A security update cannot be ignored, and its reminders do not end.")
                     .arg(wait);
    else if (shown.lastReminder)
        notes += tr("You have delayed this four times. This is the last reminder: Delay now ends the reminders for this "
                    "version. Help > Check for Updates Now still shows it.");
    else
        notes += tr("Delay: you are reminded again in %1. Each further delay waits longer (1 day, 1 week, 2 weeks, 4 weeks); "
                    "after that there is one last reminder, and then the reminders for this version end.")
                     .arg(wait);
    auto *note = new QLabel(notes, dlg);
    note->setObjectName(QStringLiteral("updateNotes"));
    note->setWordWrap(true);
    lay->addWidget(note);

    auto *buttons = new QDialogButtonBox(dlg);
    QPushButton *installBtn = buttons->addButton(shown.security ? tr("Download and Install (recommended: security)") : tr("Download and Install"),
                                                 QDialogButtonBox::AcceptRole);
    installBtn->setObjectName(QStringLiteral("updateInstall"));
    QPushButton *ignoreBtn = nullptr;
    if (!shown.security) {
        ignoreBtn = buttons->addButton(several ? tr("Ignore These Versions") : tr("Ignore This Version"), QDialogButtonBox::DestructiveRole);
        ignoreBtn->setObjectName(QStringLiteral("updateIgnore"));
    }
    QPushButton *delayBtn = buttons->addButton(shown.lastReminder ? tr("Delay (ends the reminders)") : tr("Delay (%1)").arg(wait),
                                               QDialogButtonBox::RejectRole);
    delayBtn->setObjectName(QStringLiteral("updateDelay"));
    installBtn->setDefault(true);
    lay->addWidget(buttons);

    connect(installBtn, &QPushButton::clicked, this, [this, dlg, shown] {
        dlg->close();
        install(shown.offers);
    });
    if (ignoreBtn)
        connect(ignoreBtn, &QPushButton::clicked, this, [dlg, shown] {
            updates::State st = updates::State::load();
            updates::applyIgnore(shown, &st);
            st.save();
            dlg->close();
        });
    connect(delayBtn, &QPushButton::clicked, this, [dlg, shown] {
        updates::State st = updates::State::load();
        updates::applyDelay(shown, &st, updates::now());
        st.save();
        dlg->close();
    });
    // beside the work, without taking the keyboard: the writer may be typing when it comes
    dlg->setAttribute(Qt::WA_ShowWithoutActivating, !m_manual);
    dlg->show();
}

// ---------------------------------------------------------------- download and install

void UpdateChecker::install(const QList<updates::Offer> &offers)
{
    if (m_busy)
        return;
    m_busy = true;
    m_queue = offers;
    // the program first: an add-on may need the newer program
    std::stable_sort(m_queue.begin(), m_queue.end(), [](const updates::Offer &a, const updates::Offer &b) {
        return a.kind != updates::Offer::Kind::Addon && b.kind == updates::Offer::Kind::Addon;
    });
    m_done.clear();
    m_problems.clear();
    m_restart = false;
    m_replaced.clear();
    installNext();
}

void UpdateChecker::installNext()
{
    if (m_queue.isEmpty()) {
        finishInstall();
        return;
    }
    m_current = m_queue.takeFirst();
    const QString own = ownAppImage();
    const bool inPlace = m_current.kind == updates::Offer::Kind::NewAppImage && !own.isEmpty()
                         && QFileInfo(QFileInfo(own).absolutePath()).isWritable();
    if (inPlace) {
        m_partFile = QFileInfo(own).absolutePath() + QLatin1String("/.") + QFileInfo(own).fileName() + QLatin1String(".update-part");
    } else if (m_current.kind == updates::Offer::Kind::NewAppImage) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        QDir().mkpath(dir);
        m_partFile = dir + QLatin1Char('/') + m_current.file.name;
        for (int n = 2; QFileInfo::exists(m_partFile) && n < 100; ++n)
            m_partFile = dir + QLatin1Char('/') + QFileInfo(m_current.file.name).completeBaseName() + QStringLiteral(" (%1).").arg(n)
                         + QFileInfo(m_current.file.name).suffix();
    } else {
        const QString dir = QDir::tempPath() + QStringLiteral("/quillbinder-update-%1").arg(QCoreApplication::applicationPid());
        QDir().mkpath(dir);
        m_partFile = dir + QLatin1Char('/') + m_current.file.name;
    }
    QFile::remove(m_partFile);
    auto in = std::make_shared<Incoming>();
    in->file.setFileName(m_partFile);
    if (!in->file.open(QIODevice::WriteOnly)) {
        m_problems << tr("%1: %2 could not be written.").arg(m_current.name, QDir::toNativeSeparators(m_partFile));
        installNext();
        return;
    }
    if (!m_progress) {
        m_progress = new QProgressDialog(m_window);
        m_progress->setObjectName(QStringLiteral("updateDownload"));
        m_progress->setWindowTitle(tr("Updates"));
        m_progress->setWindowModality(Qt::WindowModal);
        m_progress->setMinimumDuration(0);
        m_progress->setAutoClose(false);
        m_progress->setAutoReset(false);
        connect(m_progress, &QProgressDialog::canceled, this, [this] {
            m_queue.clear();
            if (m_reply)
                m_reply->abort();
        });
    }
    m_progress->setLabelText(tr("Downloading %1 %2...").arg(m_current.name, m_current.version));
    m_progress->setRange(0, 1000);
    m_progress->setValue(0);
    m_progress->show();
    QNetworkReply *reply = get(m_current.file.url);
    m_reply = reply;
    const qint64 expected = m_current.file.size;
    connect(reply, &QNetworkReply::readyRead, this, [this, reply, in, expected] {
        const QByteArray chunk = reply->readAll();
        in->size += chunk.size();
        if (in->size > expected || in->file.write(chunk) != chunk.size()) {
            in->failed = true;   // more than the signed list says, or the disk does not take it
            reply->abort();
            return;
        }
        in->hash.addData(chunk);
        if (m_progress && expected > 0)
            m_progress->setValue(int(in->size * 1000 / expected));
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, in] {
        reply->deleteLater();
        const bool written = in->file.flush();
        in->file.close();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString problem;
        if (reply->error() == QNetworkReply::OperationCanceledError && !in->failed)
            problem = tr("%1: the download was cancelled.").arg(m_current.name);
        else if (reply->error() != QNetworkReply::NoError || status != 200 || in->failed || !written)
            problem = tr("%1: the download failed (%2).").arg(m_current.name, in->failed ? tr("the file could not be written, or is larger than announced")
                                                                                       : reply->errorString());
        else if (in->size != m_current.file.size || QString::fromLatin1(in->hash.result().toHex()) != m_current.file.sha256)
            problem = tr("%1: the downloaded file is not the one the signed list names, so it was not installed.").arg(m_current.name);
        if (!problem.isEmpty()) {
            QFile::remove(m_partFile);
            m_problems << problem;
            installNext();
            return;
        }
        // the file is the one the release key vouches for
        if (m_current.kind == updates::Offer::Kind::NewAppImage) {
            const QString own = ownAppImage();
            if (m_partFile.endsWith(QLatin1String(".update-part")) && !own.isEmpty()) {
                QString backup, err;
                if (replaceOwnFile(own, m_partFile, &backup, &err)) {
                    m_replaced = own;
                    m_restart = true;
                    m_done << tr("%1 %2 is in the place of your AppImage. The one you had is kept as %3; delete it when the "
                                 "new version works for you.")
                                  .arg(m_current.name, m_current.version, QDir::toNativeSeparators(backup));
                } else {
                    QFile::remove(m_partFile);
                    m_problems << tr("%1 %2: %3").arg(m_current.name, m_current.version, err);
                }
            } else {
                QFile::setPermissions(m_partFile, QFileInfo(m_partFile).permissions() | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
                m_done << tr("%1 %2 was saved as %3. Close %1, put that file in the place of the AppImage you use, and start it.")
                              .arg(m_current.name, m_current.version, QDir::toNativeSeparators(m_partFile));
            }
        } else {
            QStringList log;
            QString err;
            bool restart = false;
            // only what the project signed is installed this way
            if (addons::install(m_partFile, &log, &err, &restart, addons::Unsigned::Refuse)) {
                m_done << log;
                m_restart = m_restart || restart;
            } else {
                m_problems << tr("%1 %2: %3").arg(m_current.name, m_current.version, err);
            }
            QFile::remove(m_partFile);
            QDir().rmdir(QFileInfo(m_partFile).absolutePath());
        }
        installNext();
    });
}

void UpdateChecker::finishInstall()
{
    m_busy = false;
    if (m_progress) {
        QSignalBlocker b(m_progress);
        m_progress->close();
        m_progress->deleteLater();
        m_progress = nullptr;
    }
    QString text = m_done.join(QLatin1Char('\n'));
    if (!m_problems.isEmpty())
        text += (text.isEmpty() ? QString() : QStringLiteral("\n\n")) + tr("Not installed:") + QLatin1Char('\n') + m_problems.join(QLatin1Char('\n'));
    if (text.isEmpty())
        text = tr("Nothing was installed.");
    QMessageBox box(m_problems.isEmpty() ? QMessageBox::Information : QMessageBox::Warning, tr("Updates"), text, QMessageBox::NoButton, m_window);
    box.setObjectName(QStringLiteral("updateResult"));
    QPushButton *restart = nullptr;
    if (m_restart) {
        box.setInformativeText(tr("The new version is used after a restart."));
        restart = box.addButton(tr("Restart Now"), QMessageBox::AcceptRole);
        restart->setObjectName(QStringLiteral("updateRestart"));
        box.addButton(tr("Later"), QMessageBox::RejectRole);
    } else {
        box.addButton(QMessageBox::Ok);
    }
    box.exec();
    emit installed(m_done.size(), m_problems.size());
    if (restart && box.clickedButton() == restart && !QB_TEST_ENV_SET("QB_TEST_NO_RESTART"))
        m_window->restartApplication();
}

} // namespace qb
