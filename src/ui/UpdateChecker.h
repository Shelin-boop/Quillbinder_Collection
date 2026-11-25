#pragma once
// 0.92.0: the check for updates, the part with the network and the windows (the rules: core/UpdateCheck.h).
// Nothing here runs, and no network object is made, unless the check is switched on (Help > Check for Updates at
// Startup, on by default, after the first start's note) or asked for (Help > Add-ons and Updates > Check for Updates Now).
#include "core/UpdateCheck.h"

#include <QObject>
#include <QPointer>

class QDialog;
class QNetworkAccessManager;
class QNetworkReply;
class QProgressDialog;

namespace qb {

class MainWindow;

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(MainWindow *window);
    bool isEnabled() const;
    void setEnabled(bool on);
    // 0.94.0: the first start's page asked (the note of the first look is not shown then)
    void firstStartAnswered(bool on);
    // The program's start: the first start's note, then a look when the last one is a day old or more.
    void startAutomatic();
    // Help > Add-ons and Updates > Check for Updates Now: looks whatever the switch says, and shows what there is.
    void checkNow();
    bool isBusy() const { return m_busy; }
    int requests() const { return m_requests; }   // how many requests left the program (for the tests)
    QDialog *window() const;                      // the window that offers updates, while it is shown

    // Puts a downloaded AppImage into the place of the one that runs: own is moved aside as "<own>.before-update",
    // fresh takes its name (and is made executable); when that fails, own is put back. Both in one folder.
    static bool replaceOwnFile(const QString &own, const QString &fresh, QString *backup, QString *error);
    // The AppImage this program was started from ("" when it is none), and whether its folder can be written.
    static QString ownAppImage();
    // Windows 0.92.0: a new version that needs the whole program anew comes as its setup program. The program
    // downloads it into <data>/update-setup, checks it against the signed list, closes and starts it for its own
    // folder: an update run (/QBUPDATE=1, packaging/windows/quillbinder-setup.iss) shows no pages, waits until this
    // program has ended, unpacks over the folder ("data" stays) and starts the new version.
    // setupArguments: the command line of that run. startSetup: starts it, detached; false with the system's reason.
    static QString setupArguments(const QString &folder);
    static bool startSetup(const QString &setup, const QString &folder, QString *error);

signals:
    void finished(bool reached, int offers);   // a look has ended (reached: the address answered)
    void installed(int done, int problems);    // "Download and Install" has ended
    void enabledChanged(bool on);              // the first start's note switched it

private:
    void look(bool manual);
    void attempt();
    void fetched(QNetworkReply *reply);
    void failed(const QString &why, bool quiet);
    void evaluate(const QByteArray &signature);
    void offer(const updates::Shown &shown);
    void install(const QList<updates::Offer> &offers);
    void installNext();
    void finishInstall();
    void end(bool reached, int offers);
    QNetworkReply *get(const QString &address);

    MainWindow *m_window;
    QNetworkAccessManager *m_nam = nullptr;
    QPointer<QNetworkReply> m_reply;
    QPointer<QDialog> m_offerWindow;
    QPointer<QProgressDialog> m_progress;
    bool m_busy = false;
    bool m_manual = false;
    int m_try = 0;
    int m_requests = 0;
    QByteArray m_json;
    bool m_haveJson = false;
    // installing
    QList<updates::Offer> m_queue;
    updates::Offer m_current;
    QString m_partFile;
    QStringList m_done, m_problems;
    bool m_restart = false;
    QString m_replaced;      // the AppImage that was replaced (then the program starts again)
    QString m_setupFile;     // Windows: the downloaded setup program of the new version (the program closes for it)
};

} // namespace qb
