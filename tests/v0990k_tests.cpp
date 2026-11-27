// Scripted run for 0.99.0, item 113, part 3 (Linux): the key wizard (Create Keys), the key folder with its README.txt,
// the refused places, the check, the second copy, Import Keys on a second device, Import a Key File (made by gpg:
// v4 Curve25519 taken, RSA refused), Export Keys, File > Options > Encryption (the two switches, a Scrivener project,
// "Forget the passphrase on this device"). Needs the crypto pack (tools/get-crypto.sh); gpg on the PATH for the key files.
//
//   xvfb-run -a ./qb_v0990k_tests /tmp/shots
#include "core/crypto/KeyFolder.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/Passphrase.h"
#include "core/crypto/ProjectSwitches.h"
#include "ui/AppSettings.h"
#include "ui/EncryptionPage.h"
#include "ui/OptionsDialog.h"
#include "ui/Theme.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>
#include <QElapsedTimer>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTreeWidget>

#include <cstdio>

using namespace qb;
namespace qc = qb::crypto;
namespace kf = qb::crypto::keyfolder;

static int failures = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 150)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

static void shot(QWidget *w, const QString &name)
{
    settle(250);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

static bool fileHolds(const QString &path, const QByteArray &needle) { return readAll(path).contains(needle); }

static bool anyFileUnderHolds(const QString &dir, const QByteArray &needle)
{
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        if (fileHolds(it.next(), needle))
            return true;
    return false;
}

static QString button(QWizard &w, QWizard::WizardButton b) { return w.button(b)->isEnabled() ? QStringLiteral("on") : QStringLiteral("off"); }

// the state of the fake device and the answers of the dialogs
static QString g_pick;              // what the folder dialog "returns"
static QString g_file;              // what the file dialog "returns"
static QString g_pass;              // what the passphrase box "returns"
static int g_passAsked = 0;
static QString g_recovery;          // the recovery passphrase the wizard showed
static QStringList g_notes;         // what the message boxes said
static bool g_confirm = true;

static void installHooks()
{
    auto &h = encui::hooks();
    h.pickFolder = [](QWidget *, const QString &) { return g_pick; };
    h.pickFile = [](QWidget *, const QString &) { return g_file; };
    h.askPassphrase = [](QWidget *, const QString &, const QString &) {
        ++g_passAsked;
        encui::Hooks::Answer a;
        a.ok = !g_pass.isEmpty();
        a.passphrase = g_pass;
        return a;
    };
    h.notify = [](QWidget *, const QString &title, const QString &text, bool) { g_notes << title + QStringLiteral(": ") + text; };
    h.confirm = [](QWidget *, const QString &, const QString &) { return g_confirm; };
    h.recoveryShown = [](const QString &s) { g_recovery = s; };
    h.today = [] { return QDate(2026, 11, 27); };
}

static void forgetDevice()
{
    qc::KeyRing &ring = qc::KeyRing::device();
    for (const qc::KeyInfo &k : ring.keys())
        ring.removeKey(k.fingerprint);
    QFile::remove(qc::KeyRing::deviceDir() + QStringLiteral("/roles.json"));
}

static QProcessEnvironment gpgEnv(const QString &home)
{
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    e.insert(QStringLiteral("GNUPGHOME"), home);
    return e;
}

static QByteArray gpgKey(const QString &home, const QString &algo, const QString &pass, QString *log)
{
    QDir().mkpath(home);
    QFile::setPermissions(home, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    auto run = [&](const QStringList &args) {
        QProcess p;
        p.setProcessEnvironment(gpgEnv(home));
        p.start(QStringLiteral("gpg"), args);
        p.waitForFinished(60000);
        if (log)
            *log += QString::fromUtf8(p.readAllStandardError());
        return p.readAllStandardOutput();
    };
    run({QStringLiteral("--batch"), QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"), pass,
         QStringLiteral("--quick-gen-key"), QStringLiteral("Kleo Like <kleo@example.org>"), algo, QStringLiteral("default"),
         QStringLiteral("never")});
    return run({QStringLiteral("--batch"), QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"),
                pass, QStringLiteral("--armor"), QStringLiteral("--export-secret-keys")});
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QB_TEST_KEYSTORE", "memory");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990kTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990kTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    theme::apply();

    if (!qc::available()) {
        std::printf("SKIPPED  this build has no OpenPGP library\n0 failure(s)\n");
        return 0;
    }
    installHooks();

    QTemporaryDir work;
    const QString stick = work.path() + QStringLiteral("/stick");
    const QString stick2 = work.path() + QStringLiteral("/stick2");
    const QString projects = work.path() + QStringLiteral("/Projects/My Novel.quillbinder");
    const QString backups = work.path() + QStringLiteral("/Backups");
    QDir().mkpath(stick);
    QDir().mkpath(stick2);
    QDir().mkpath(projects);
    QDir().mkpath(backups);
    as.recentProjects = {projects};
    as.backupDir = backups;

    // ---------------------------------------------------------------- places
    for (const auto &c : QList<QPair<QString, QString>>{{QStringLiteral("/home/writer/OneDrive/keys"), QStringLiteral("OneDrive")},
                                                        {QStringLiteral("/home/writer/OneDrive - Company/x"), QStringLiteral("OneDrive")},
                                                        {QStringLiteral("/home/writer/Dropbox"), QStringLiteral("Dropbox")},
                                                        {QStringLiteral("/home/writer/Nextcloud/Docs"), QStringLiteral("Nextcloud")},
                                                        {QStringLiteral("/home/writer/Google Drive/x"), QStringLiteral("Google Drive")},
                                                        {QStringLiteral("/Users/writer/iCloud Drive/x"), QStringLiteral("iCloud")},
                                                        {QStringLiteral("/home/writer/Syncthing/x"), QStringLiteral("Syncthing")},
                                                        {QStringLiteral("/home/writer/pCloudDrive/x"), QString()},
                                                        {QStringLiteral("/home/writer/pCloud/x"), QStringLiteral("pCloud")},
                                                        {QStringLiteral("/home/writer/MEGA/x"), QStringLiteral("MEGA")},
                                                        {QStringLiteral("/home/writer/Megapixels/x"), QString()},
                                                        {QStringLiteral("/media/a/STICK/keys"), QString()}})
        check(kf::syncProgramIn(c.first) == c.second, QStringLiteral("sync folder name: %1 -> '%2'").arg(c.first, c.second), kf::syncProgramIn(c.first));
    check(!kf::refusedPlace(projects, as.recentProjects, backups).isEmpty() && !kf::refusedPlace(projects + QStringLiteral("/Files"), as.recentProjects, backups).isEmpty(),
          QStringLiteral("refused: a project's folder and a folder inside it"));
    check(!kf::refusedPlace(backups, as.recentProjects, backups).isEmpty(), QStringLiteral("refused: the backup folder"));
    check(!kf::refusedPlace(work.path() + QStringLiteral("/Dropbox/k"), as.recentProjects, backups).isEmpty(),
          QStringLiteral("refused: a folder of a sync program"));
    check(kf::refusedPlace(stick, as.recentProjects, backups).isEmpty(), QStringLiteral("a USB stick's folder is taken"));

    // ---------------------------------------------------------------- the wizard, end to end
    QString myWords;
    {
        KeyWizard wiz;
        wiz.show();
        settle(200);
        auto *words = wiz.findChild<QLabel *>(QStringLiteral("wizWords"));
        const QString first = words ? words->text() : QString();
        check(first.split(QLatin1Char(' '), Qt::SkipEmptyParts).size() == 6 && qc::passphrase::check(first).ok,
              QStringLiteral("wizard: six random words are proposed"), first);
        shot(&wiz, QStringLiteral("wizard-1-passphrase"));
        wiz.findChild<QPushButton *>(QStringLiteral("wizNewWords"))->click();
        check(words->text() != first, QStringLiteral("wizard: Other words gives other words"));
        // an own passphrase: a weak one is refused, a good one accepted
        wiz.findChild<QRadioButton *>(QStringLiteral("wizUseOwn"))->setChecked(true);
        wiz.findChild<QLineEdit *>(QStringLiteral("wizOwn"))->setText(QStringLiteral("password1234"));
        wiz.findChild<QLineEdit *>(QStringLiteral("wizOwn2"))->setText(QStringLiteral("password1234"));
        settle(50);
        check(button(wiz, QWizard::NextButton) == QLatin1String("off")
                  && !wiz.findChild<QLabel *>(QStringLiteral("wizStrength"))->text().isEmpty(),
              QStringLiteral("wizard: a weak own passphrase is refused, with the reason"),
              wiz.findChild<QLabel *>(QStringLiteral("wizStrength"))->text());
        shot(&wiz, QStringLiteral("wizard-1b-weak"));
        wiz.findChild<QRadioButton *>(QStringLiteral("wizUseProposed"))->setChecked(true);
        settle(50);
        myWords = words->text();
        check(button(wiz, QWizard::NextButton) == QLatin1String("on"), QStringLiteral("wizard: the proposed words are accepted"));
        wiz.button(QWizard::NextButton)->click();
        settle(100);

        // the place
        auto *folder = wiz.findChild<QLineEdit *>(QStringLiteral("wizFolder"));
        check(folder && wiz.currentId() == KeyWizard::PlacePage, QStringLiteral("wizard: second step is the place"));
        folder->setText(projects);
        settle(50);
        check(button(wiz, QWizard::NextButton) == QLatin1String("off") && !wiz.findChild<QLabel *>(QStringLiteral("wizPlaceMessage"))->text().isEmpty(),
              QStringLiteral("wizard: the project's folder is refused"));
        shot(&wiz, QStringLiteral("wizard-2-place-refused"));
        folder->setText(backups);
        settle(50);
        check(button(wiz, QWizard::NextButton) == QLatin1String("off"), QStringLiteral("wizard: the backup folder is refused"));
        folder->setText(work.path() + QStringLiteral("/Dropbox/keys"));
        settle(50);
        check(button(wiz, QWizard::NextButton) == QLatin1String("off")
                  && wiz.findChild<QLabel *>(QStringLiteral("wizPlaceMessage"))->text().contains(QLatin1String("Dropbox")),
              QStringLiteral("wizard: a sync program's folder is refused and named"));
        g_pick = stick;
        wiz.findChild<QPushButton *>(QStringLiteral("wizChoose"))->click();
        check(folder->text() == stick, QStringLiteral("wizard: Choose... takes the folder (test hook)"));
        settle(50);
        shot(&wiz, QStringLiteral("wizard-2-place"));
        QElapsedTimer t;
        t.start();
        wiz.button(QWizard::NextButton)->click();
        settle(100);
        check(wiz.currentId() == KeyWizard::RecoveryPage && !wiz.folder().isEmpty(),
              QStringLiteral("wizard: the keys and the folder are made (%1 ms)").arg(t.elapsed()));

        // the recovery passphrase
        const QStringList rw = g_recovery.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        check(rw.size() == 6 && wiz.findChild<QLabel *>(QStringLiteral("wizRecoveryWords"))->text() == g_recovery,
              QStringLiteral("wizard: the recovery passphrase is shown (six words)"), g_recovery);
        check(button(wiz, QWizard::NextButton) == QLatin1String("off"), QStringLiteral("wizard: Next waits for 'written on paper'"));
        shot(&wiz, QStringLiteral("wizard-3-recovery"));
        wiz.findChild<QCheckBox *>(QStringLiteral("wizWritten"))->setChecked(true);
        wiz.button(QWizard::NextButton)->click();
        settle(100);

        // the check
        check(wiz.currentId() == KeyWizard::CheckPage, QStringLiteral("wizard: fourth step is the check"));
        const QString res = wiz.findChild<QLabel *>(QStringLiteral("wizCheckResult"))->text();
        check(res.count(QLatin1String("[OK]")) == 4 && !res.contains(QLatin1String("FAILED")),
              QStringLiteral("wizard check: files read back, README there, test file opens with both keys"), res);
        auto *w1 = wiz.findChild<QLineEdit *>(QStringLiteral("wizWord1"));
        auto *w2 = wiz.findChild<QLineEdit *>(QStringLiteral("wizWord2"));
        const int a = wiz.findChild<QLabel *>(QStringLiteral("wizWord1Label"))->text().remove(QRegularExpression(QStringLiteral("\\D"))).toInt() - 1;
        const int b = wiz.findChild<QLabel *>(QStringLiteral("wizWord2Label"))->text().remove(QRegularExpression(QStringLiteral("\\D"))).toInt() - 1;
        check(a >= 0 && b >= 0 && a != b && a < 6 && b < 6, QStringLiteral("wizard check: two different words are asked"));
        w1->setText(QStringLiteral("wrong"));
        w2->setText(QStringLiteral("words"));
        settle(50);
        check(button(wiz, QWizard::NextButton) == QLatin1String("off"), QStringLiteral("wizard check: wrong words do not pass"));
        shot(&wiz, QStringLiteral("wizard-4-check"));
        w1->setText(rw.value(a).toUpper());
        w2->setText(QLatin1Char(' ') + rw.value(b) + QLatin1Char(' '));
        settle(50);
        check(button(wiz, QWizard::NextButton) == QLatin1String("on"), QStringLiteral("wizard check: the two right words pass"));
        shot(&wiz, QStringLiteral("wizard-4-check-ok"));
        wiz.button(QWizard::NextButton)->click();
        settle(100);
        check(wiz.committed() && wiz.currentId() == KeyWizard::CopyPage, QStringLiteral("wizard: the keys are on the device, the copy step follows"));

        // the second copy
        auto *copyEdit = wiz.findChild<QLineEdit *>(QStringLiteral("wizCopyFolder"));
        copyEdit->setText(stick);
        wiz.findChild<QPushButton *>(QStringLiteral("wizCopyMake"))->click();
        check(wiz.copyFolder().isEmpty() && wiz.findChild<QLabel *>(QStringLiteral("wizCopyResult"))->text().contains(QLatin1String("first copy")),
              QStringLiteral("wizard copy: the place of the first copy is refused"), wiz.findChild<QLabel *>(QStringLiteral("wizCopyResult"))->text());
        copyEdit->setText(backups);
        wiz.findChild<QPushButton *>(QStringLiteral("wizCopyMake"))->click();
        check(wiz.copyFolder().isEmpty(), QStringLiteral("wizard copy: the backup folder is refused"));
        g_pick = stick2;
        wiz.findChild<QPushButton *>(QStringLiteral("wizCopyChoose"))->click();
        wiz.findChild<QPushButton *>(QStringLiteral("wizCopyMake"))->click();
        settle(100);
        check(!wiz.copyFolder().isEmpty() && wiz.findChild<QLabel *>(QStringLiteral("wizCopyResult"))->text().contains(QLatin1String("checked")),
              QStringLiteral("wizard copy: a second copy is made and checked"), wiz.findChild<QLabel *>(QStringLiteral("wizCopyResult"))->text());
        shot(&wiz, QStringLiteral("wizard-5-copy"));

        // the folder
        const QString dir = wiz.folder();
        check(QFileInfo(dir).fileName() == QLatin1String("Quillbinder Keys 2026-11-27"), QStringLiteral("key folder: named Quillbinder Keys yyyy-mm-dd"), dir);
        QStringList names = QDir(dir).entryList(QDir::Files, QDir::Name);
        check(names == QStringList({QStringLiteral("README.txt"), QStringLiteral("my-key-public.asc"), QStringLiteral("my-key-secret.asc"),
                                    QStringLiteral("recovery-key-public.asc"), QStringLiteral("recovery-key-secret.asc")}),
              QStringLiteral("key folder: the five files"), names.join(QLatin1Char(',')));
        QStringList names2 = QDir(wiz.copyFolder()).entryList(QDir::Files, QDir::Name);
        check(names2 == names && readAll(dir + QStringLiteral("/my-key-secret.asc")) == readAll(wiz.copyFolder() + QStringLiteral("/my-key-secret.asc")),
              QStringLiteral("second copy: the same five files"));
        const QString readme = QString::fromUtf8(readAll(dir + QStringLiteral("/README.txt")));
        check(readme.contains(QLatin1String("Kleopatra")) && readme.contains(QLatin1String("Gpg4win")) && readme.contains(QLatin1String("gpg --import"))
                  && readme.contains(QLatin1String("Import Keys")) && readme.contains(QLatin1String("my-key-secret.asc")),
              QStringLiteral("README.txt: English, tells the files, Kleopatra, Gpg4win, gpg, another device"));
        check(!readme.contains(QLatin1String("PRIVATE KEY")) && !readme.contains(myWords) && !readme.contains(g_recovery),
              QStringLiteral("README.txt: no secret in it"));
        check(!anyFileUnderHolds(dir, g_recovery.toUtf8()) && !anyFileUnderHolds(stick2, g_recovery.toUtf8())
                  && !anyFileUnderHolds(qc::KeyRing::deviceDir(), g_recovery.toUtf8()) && !anyFileUnderHolds(home.path(), g_recovery.toUtf8()),
              QStringLiteral("the recovery passphrase is in no file (folder, copy, key ring, settings, data)"));
        check(!anyFileUnderHolds(home.path(), myWords.toUtf8()) && !anyFileUnderHolds(dir, myWords.toUtf8()),
              QStringLiteral("the passphrase of the key is in no file"));
        check(fileHolds(dir + QStringLiteral("/my-key-secret.asc"), "BEGIN PGP PRIVATE KEY BLOCK")
                  && !fileHolds(dir + QStringLiteral("/my-key-public.asc"), "PRIVATE"),
              QStringLiteral("key files: the secret file holds a private key block, the public one does not"));
        // the files really are locked by their passphrases
        const auto ck = kf::checkFolder(dir, myWords.toUtf8(), g_recovery.toUtf8());
        check(ck.ok(), QStringLiteral("check: the folder reads back and both keys open a test file"), ck.error);
        const auto bad = kf::checkFolder(dir, QByteArray("not the passphrase"), g_recovery.toUtf8());
        check(!bad.ok() && !bad.myKeyWorks, QStringLiteral("check: a wrong passphrase does not open the key"));
        const auto swapped = kf::checkFolder(dir, g_recovery.toUtf8(), myWords.toUtf8());
        check(!swapped.ok(), QStringLiteral("check: each key needs its own passphrase"));
        wiz.button(QWizard::FinishButton)->click();
        settle(50);
    }
    // the device
    const kf::Roles roles = kf::roles();
    qc::KeyRing &dev = qc::KeyRing::device();
    check(roles.complete() && dev.hasSecret(roles.mine) && !dev.hasSecret(roles.recovery) && dev.contains(roles.recovery),
          QStringLiteral("device: my key with its secret part, the recovery key public only (its secret part is on the stick and the paper)"));
    check(dev.isUnlocked(roles.mine), QStringLiteral("device: the key is unlocked for this session"));
    QByteArray remembered;
    check(qc::KeyStore::system().read(qc::KeyStore::passphraseEntry(roles.mine), &remembered) == qc::KeyStore::Status::Ok
              && remembered == myWords.toUtf8(),
          QStringLiteral("device: the passphrase is remembered in the key store"));
    const QString madeFolder = stick + QStringLiteral("/Quillbinder Keys 2026-11-27");

    // ---------------------------------------------------------------- Import Keys on a second device (core)
    {
        qb::crypto::KeyRing ring2;
        QTemporaryDir dir2;
        const auto wrong = kf::importFolder(ring2, madeFolder, QByteArray("wrong words here"));
        check(!wrong.ok && wrong.code == qc::Error::WrongPassphrase && ring2.keys().isEmpty(),
              QStringLiteral("Import Keys: a wrong passphrase imports nothing"), wrong.error);
        const auto r = kf::importFolder(ring2, madeFolder, myWords.toUtf8());
        check(r.ok && r.mine == roles.mine && r.recovery == roles.recovery && ring2.hasSecret(r.mine) && !ring2.hasSecret(r.recovery)
                  && ring2.isUnlocked(r.mine),
              QStringLiteral("Import Keys: the key (secret) and the recovery key (public only) are taken, the key is unlocked"), r.error);
        const QByteArray cipher = dev.encrypt("a secret text", {roles.mine, roles.recovery});
        qc::Error e = qc::Error::None;
        check(ring2.decrypt(cipher, &e) == QByteArray("a secret text"), QStringLiteral("Import Keys: the second device opens what the first one locked"));
        check(kf::setRoles(r.mine == roles.mine ? kf::Roles{r.mine, r.recovery} : kf::Roles(), dir2.path()) && kf::roles(dir2.path()).complete(),
              QStringLiteral("roles are kept in a small file next to the key ring"));
    }
    // the same through the page, on a device that was emptied
    {
        forgetDevice();
        qc::KeyStore::system().remove(qc::KeyStore::passphraseEntry(roles.mine));
        EncryptionPage page;
        page.show();
        shot(&page, QStringLiteral("options-no-keys"));
        check(page.findChild<QLabel *>(QStringLiteral("encNoKeys"))->isVisible() && !page.findChild<QPushButton *>(QStringLiteral("encExport"))->isEnabled()
                  && !page.findChild<QCheckBox *>(QStringLiteral("encBackups"))->isEnabled(),
              QStringLiteral("page: without keys it says so; Export and the switches are off"));
        g_pick = madeFolder;
        g_pass = myWords;
        g_passAsked = 0;
        page.findChild<QPushButton *>(QStringLiteral("encImport"))->click();
        settle(100);
        check(g_passAsked == 1 && kf::roles().complete() && qc::KeyRing::device().hasSecret(kf::roles().mine),
              QStringLiteral("page: Import Keys asks the passphrase once and takes the keys"), QString::number(g_passAsked));
        QByteArray again;
        check(qc::KeyStore::system().read(qc::KeyStore::passphraseEntry(kf::roles().mine), &again) == qc::KeyStore::Status::Ok && again == myWords.toUtf8(),
              QStringLiteral("page: the passphrase is remembered after Import Keys"));
        check(page.findChild<QTreeWidget *>(QStringLiteral("encKeyList"))->topLevelItemCount() == 2, QStringLiteral("page: two keys are listed"));
        shot(&page, QStringLiteral("options-keys"));

        // wrong passphrase through the page: nothing changes, a message
        forgetDevice();
        g_pass = QStringLiteral("some other words");
        g_notes.clear();
        page.findChild<QPushButton *>(QStringLiteral("encImport"))->click();
        check(!kf::roles().complete() && qc::KeyRing::device().keys().isEmpty() && !g_notes.isEmpty(),
              QStringLiteral("page: Import Keys with a wrong passphrase says so and takes nothing"), g_notes.join(QLatin1Char('|')));
        g_pass = myWords;
        page.findChild<QPushButton *>(QStringLiteral("encImport"))->click();
        check(kf::roles().complete(), QStringLiteral("page: the right passphrase then works"));
    }

    // ---------------------------------------------------------------- Import a Key File (made by gpg)
    {
        QString log;
        const QByteArray ed = gpgKey(work.path() + QStringLiteral("/gpg-ed"), QStringLiteral("default"), QStringLiteral("kleo passphrase one"), &log);
        if (ed.isEmpty()) {
            std::printf("SKIPPED  gpg made no key (%s)\n", qPrintable(log.left(200)));
        } else {
            QFile f(work.path() + QStringLiteral("/kleo.asc"));
            f.open(QIODevice::WriteOnly);
            f.write(ed);
            f.close();
            forgetDevice();
            EncryptionPage page;
            page.show();
            g_file = f.fileName();
            g_pass = QStringLiteral("kleo passphrase one");
            g_notes.clear();
            page.findChild<QPushButton *>(QStringLiteral("encImportFile"))->click();
            settle(100);
            const kf::Roles r = kf::roles();
            check(!r.mine.isEmpty() && qc::KeyRing::device().hasSecret(r.mine) && qc::KeyRing::device().isUnlocked(r.mine),
                  QStringLiteral("Import a Key File: a v4 Curve25519 key made by gpg is taken as my key"), g_notes.join(QLatin1Char('|')));
            check(g_notes.join(QLatin1Char(' ')).contains(QLatin1String("no recovery key")),
                  QStringLiteral("Import a Key File: it says that there is no recovery key yet"), g_notes.join(QLatin1Char('|')));
            shot(&page, QStringLiteral("options-keyfile"));
            // a wrong passphrase
            forgetDevice();
            g_pass = QStringLiteral("not it");
            g_notes.clear();
            page.findChild<QPushButton *>(QStringLiteral("encImportFile"))->click();
            check(qc::KeyRing::device().keys().isEmpty() && !g_notes.isEmpty(), QStringLiteral("Import a Key File: a wrong passphrase takes nothing"));
        }
        const QByteArray rsa = gpgKey(work.path() + QStringLiteral("/gpg-rsa"), QStringLiteral("rsa2048"), QStringLiteral("rsa pass one two"), &log);
        if (rsa.isEmpty()) {
            std::printf("SKIPPED  gpg made no RSA key\n");
        } else {
            qc::KeyRing ring3;
            const auto r = kf::importKeyFile(ring3, rsa, QByteArray("rsa pass one two"));
            check(!r.ok && !r.error.isEmpty() && ring3.keys().isEmpty(), QStringLiteral("Import a Key File: an RSA key is refused with a sentence"), r.error);
            const auto n = kf::importKeyFile(ring3, QByteArray("this is not a key"), QByteArray());
            check(!n.ok && !n.error.isEmpty(), QStringLiteral("Import a Key File: a file that is no key is refused"), n.error);
        }
        {
            // L5: the recovery key's secret part is not "your key": refused with a sentence, nothing taken
            qc::KeyRing ring4;
            const auto rs = kf::importKeyFile(ring4, readAll(madeFolder + QStringLiteral("/recovery-key-secret.asc")), g_recovery.toUtf8());
            check(!rs.ok && rs.mine.isEmpty() && !rs.error.isEmpty() && ring4.keys().isEmpty(),
                  QStringLiteral("Import a Key File: the recovery key's secret file is refused as 'your key'"), rs.error);
        }
    }

    // ---------------------------------------------------------------- Export Keys
    {
        forgetDevice();
        qc::KeyRing::device().importKeys(readAll(madeFolder + QStringLiteral("/my-key-secret.asc")));
        qc::KeyRing::device().importKeys(readAll(madeFolder + QStringLiteral("/recovery-key-public.asc")));
        kf::setRoles(roles);
        EncryptionPage page;
        page.show();
        const QString exportTo = work.path() + QStringLiteral("/export");
        QDir().mkpath(exportTo);
        g_pick = work.path() + QStringLiteral("/Nextcloud/out");
        QDir().mkpath(g_pick);
        g_notes.clear();
        page.findChild<QPushButton *>(QStringLiteral("encExport"))->click();
        check(QDir(g_pick).entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty() && !g_notes.isEmpty(),
              QStringLiteral("Export Keys: a sync program's folder is refused"), g_notes.join(QLatin1Char('|')));
        g_pick = exportTo;
        page.findChild<QPushButton *>(QStringLiteral("encExport"))->click();
        const QString out = exportTo + QStringLiteral("/Quillbinder Keys 2026-11-27");
        const QStringList names = QDir(out).entryList(QDir::Files, QDir::Name);
        check(names == QStringList({QStringLiteral("README.txt"), QStringLiteral("my-key-public.asc"), QStringLiteral("my-key-secret.asc"),
                                    QStringLiteral("recovery-key-public.asc")}),
              QStringLiteral("Export Keys: the key, the public parts and a README (no recovery secret: it is not on this device)"), names.join(QLatin1Char(',')));
        qb::crypto::KeyRing ring4;
        const auto r = kf::importFolder(ring4, out, myWords.toUtf8());
        check(r.ok && ring4.hasSecret(roles.mine), QStringLiteral("Export Keys: the exported folder imports with the same passphrase"), r.error);
        check(!QString::fromUtf8(readAll(out + QStringLiteral("/README.txt"))).contains(QLatin1String("PRIVATE")), QStringLiteral("Export Keys: README without a secret"));
    }

    // ---------------------------------------------------------------- File > Options > Encryption, the switches
    {
        forgetDevice();
        qc::KeyRing::device().importKeys(readAll(madeFolder + QStringLiteral("/my-key-secret.asc")));
        qc::KeyRing::device().importKeys(readAll(madeFolder + QStringLiteral("/recovery-key-public.asc")));
        kf::setRoles(roles);
        qc::KeyRing::device().unlock(roles.mine, myWords.toUtf8());
        auto &ps = qc::projectSwitches();
        bool open = false, scriv = false, backupsOn = false, projectOn = false;
        QStringList calls;
        ps.projectOpen = [&] { return open; };
        ps.isScrivenerProject = [&] { return scriv; };
        ps.backupsEncrypted = [&] { return backupsOn; };
        ps.projectEncrypted = [&] { return projectOn; };
        ps.setBackupsEncrypted = [&](bool on, QString *) { backupsOn = on; calls << QStringLiteral("backups=%1").arg(on); return true; };
        ps.setProjectEncrypted = [&](bool on, QString *) { projectOn = on; calls << QStringLiteral("project=%1").arg(on); return true; };
        {
            OptionsDialog od(nullptr, 0);
            od.resize(900, 640);
            od.showSection(OptionsDialog::SEncryption);
            od.show();
            settle(200);
            auto *page = od.findChild<EncryptionPage *>(QStringLiteral("encryptionPage"));
            check(page != nullptr, QStringLiteral("Options: the Encryption section is there"));
            auto *bk = od.findChild<QCheckBox *>(QStringLiteral("encBackups"));
            auto *pj = od.findChild<QCheckBox *>(QStringLiteral("encProject"));
            check(bk && pj && !bk->isEnabled() && !pj->isEnabled(), QStringLiteral("Options: no project open, the switches are off"));
            shot(&od, QStringLiteral("options-encryption-noproject"));
            open = true;
            page->refresh();
            check(bk->isEnabled() && pj->isEnabled(), QStringLiteral("Options: an own-format project, both switches can be set"));
            shot(&od, QStringLiteral("options-encryption-project"));
            bk->setChecked(true);
            pj->setChecked(true);
            od.apply();
            check(calls == QStringList({QStringLiteral("backups=1"), QStringLiteral("project=1")}) && backupsOn && projectOn,
                  QStringLiteral("Options: Apply passes both switches to the project layer"), calls.join(QLatin1Char(',')));
            calls.clear();
            od.apply();
            check(calls.isEmpty(), QStringLiteral("Options: nothing is passed again when nothing changed"));
            scriv = true;
            projectOn = false;
            page->refresh();
            check(bk->isEnabled() && !pj->isEnabled() && od.findChild<QLabel *>(QStringLiteral("encReason"))->text().contains(QLatin1String("Scrivener")),
                  QStringLiteral("Options: a Scrivener project: Encrypted project greyed, with the reason"),
                  od.findChild<QLabel *>(QStringLiteral("encReason"))->text());
            shot(&od, QStringLiteral("options-encryption-scrivener"));
            // Forget the passphrase
            qc::KeyStore::system().write(qc::KeyStore::passphraseEntry(roles.mine), myWords.toUtf8());
            g_confirm = false;
            od.findChild<QPushButton *>(QStringLiteral("encForget"))->click();
            QByteArray left;
            check(qc::KeyStore::system().read(qc::KeyStore::passphraseEntry(roles.mine), &left) == qc::KeyStore::Status::Ok && qc::KeyRing::device().isUnlocked(roles.mine),
                  QStringLiteral("Forget the passphrase: 'no' changes nothing"));
            g_confirm = true;
            od.findChild<QPushButton *>(QStringLiteral("encForget"))->click();
            check(qc::KeyStore::system().read(qc::KeyStore::passphraseEntry(roles.mine), &left) == qc::KeyStore::Status::NotFound
                      && !qc::KeyRing::device().isUnlocked(roles.mine),
                  QStringLiteral("Forget the passphrase: removed from the key store, the key is locked"));
            shot(&od, QStringLiteral("options-encryption-forgotten"));
            od.reject();
        }
        // a switch that cannot be set says why
        ps.setBackupsEncrypted = [&](bool, QString *err) { *err = QStringLiteral("no room"); return false; };
        scriv = false;
        backupsOn = false;
        EncryptionPage page;
        page.show();
        page.findChild<QCheckBox *>(QStringLiteral("encBackups"))->setChecked(true);
        QStringList errors;
        check(!page.apply(&errors) && errors == QStringList(QStringLiteral("no room")), QStringLiteral("Options: a switch that failed is reported"), errors.join(QLatin1Char('|')));
        ps = qc::ProjectSwitches();
    }

    std::printf("%d failure(s)\n", failures);
    return failures;
}
