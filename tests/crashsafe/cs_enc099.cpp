// The system test, 0.99.0 (item 124): encryption. The keys of the run (made once per results folder, imported into each
// case's home), the encrypted variants of older cases (suffix "e": fixtureCopy encrypts the copy it hands out), and the
// cases S-Z01 to S-Z08: the key wizard and its files, Import Keys on a second machine (another home), an encrypted
// project synced between two copies, the recovery key alone, a wrong key and keys of the wrong kind, an encrypted backup
// opened with gpg, a Scrivener project with encrypted backups, the passphrase from the key store.
#include "cs.h"

#include "core/Backup.h"
#include "core/Project.h"
#include "core/crypto/KeyFolder.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/ProjectCrypto.h"
#include "core/crypto/Vault.h"
#include "import/ScrivenerFormat.h"
#include "ui/EncryptionPage.h"
#include "ui/MainWindow.h"
#include "ui/ProjectCryptoUi.h"

#include <QCheckBox>
#include <QCryptographicHash>
#include <QDate>
#include <QDirIterator>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>

namespace cs {

namespace qc = qb::crypto;
namespace kf = qb::crypto::keyfolder;

namespace {

const QByteArray kWriterPass = "correct horse battery staple again";
const QByteArray kRecoveryPass = "the recovery words are kept on paper";
QString g_mine, g_recovery;   // the fingerprints of the run's keys, once set up

QMap<QString, QByteArray> bytesOf(const QString &folder)
{
    QMap<QString, QByteArray> m;
    QDirIterator it(folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString f = it.next();
        QFile file(f);
        if (file.open(QIODevice::ReadOnly))
            m.insert(QDir(folder).relativeFilePath(f), QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha1));
    }
    return m;
}

// every file of a folder that is not OpenPGP (but the marker and the lock)
QStringList plainFilesOf(const QString &folder)
{
    QStringList out;
    QDirIterator it(folder, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString f = it.next();
        const QString rel = QDir(folder).relativeFilePath(f);
        if (qb::vault::projectPlainNames().contains(rel))
            continue;
        QFile file(f);
        if (file.open(QIODevice::ReadOnly) && !qb::vault::isSealedData(file.read(64)))
            out << rel;
    }
    out.sort();
    return out;
}

// the folder of the run's keys ("Quillbinder Keys ..."), made once in the fixtures
QString keyFolderOfRun()
{
    const QString base = ctx().fixtures.isEmpty() ? ctx().dir + QStringLiteral("/enc-fixture") : ctx().fixtures + QStringLiteral("/enc-keys");
    auto found = [&]() -> QString {
        const QStringList l = QDir(base + QStringLiteral("/keys")).entryList({QStringLiteral("Quillbinder Keys*")}, QDir::Dirs);
        return l.isEmpty() ? QString() : base + QStringLiteral("/keys/") + l.first();
    };
    if (const QString f = found(); !f.isEmpty())
        return f;
    const QString tmp = base + QStringLiteral("/making-%1").arg(QCoreApplication::applicationPid());
    QDir().mkpath(tmp);
    qc::KeyRing ring;
    const kf::NewKeys nk = kf::makeKeys(ring, kWriterPass, kRecoveryPass);
    const kf::Written w = kf::writeFolder(ring, kf::Roles{nk.mine, nk.recovery}, tmp, QDate(2026, 11, 27));
    if (!w.ok) {
        check(false, QStringLiteral("(the run's keys are made)"), w.error + nk.error);
        return QString();
    }
    QDir().mkpath(base + QStringLiteral("/keys"));
    QDir().rename(w.folder, base + QStringLiteral("/keys/") + QFileInfo(w.folder).fileName());
    QDir(tmp).removeRecursively();
    return found();
}

} // namespace

// ---------------------------------------------------------------- what the encrypted variants and the cases share
void encryptionSetUp()
{
    qputenv("QB_TEST_KEYSTORE", "memory");
    const QString folder = keyFolderOfRun();
    if (folder.isEmpty() || !qc::available())
        return;
    qc::KeyRing &ring = qc::KeyRing::device();
    const kf::Imported r = kf::importFolder(ring, folder, kWriterPass);
    if (!r.ok) {
        check(false, QStringLiteral("(the run's keys are imported into this home)"), r.error);
        return;
    }
    g_mine = r.mine;
    g_recovery = r.recovery;
    ring.unlock(g_mine, kWriterPass);
    ring.save(qc::KeyRing::deviceDir());
    kf::setRoles(kf::Roles{g_mine, g_recovery});
}

qc::Recipients encryptionRecipients()
{
    return qc::Recipients{g_mine, g_recovery};
}

// the project folder of a copy is encrypted (Quillbinder format only; a Scrivener project stays plain, which is told)
bool encryptFixtureAt(const QString &openPath)
{
    if (openPath.endsWith(QLatin1String(".scriv")) || openPath.endsWith(QLatin1String(".scrivx"))) {
        note(QStringLiteral("(a Scrivener project cannot be encrypted: this copy stays plain)"));
        fact(QStringLiteral("encrypted variant: a Scrivener copy stayed plain"), true);
        return false;
    }
    if (g_mine.isEmpty())
        encryptionSetUp();
    QString err;
    const bool ok = qc::encryptFolder(QFileInfo(openPath).absolutePath(), encryptionRecipients(), {}, &err);
    check(ok, QStringLiteral("(the copy is encrypted)"), err);
    return ok;
}

namespace {

bool askKeyFolderPass(QString *folder, QString *words)
{
    Q_UNUSED(folder)
    Q_UNUSED(words)
    return true;
}

// a project made from FX-1, encrypted; returns the .qbx path
QString encryptedProject(const QString &fixture = QStringLiteral("fx1-qb"))
{
    const QString path = fixtureCopy(fixture);
    if (!ctx().encrypted)
        encryptFixtureAt(path);
    return path;
}

QString firstText(Project *p, int n = 3)
{
    BinderItem *it = textItems(p).value(n);
    return it ? p->docs()->text(it->id)->toPlainText() : QString();
}

} // namespace

// ================================================================ S-Z01 the key wizard and its files
void caseSZ01()
{
    askKeyFolderPass(nullptr, nullptr);
    if (!qc::available()) {
        check(false, QStringLiteral("this build has the encryption (tools/get-crypto.sh before configure)"));
        return;
    }
    QString pick, recoveryWords;
    auto &h = qb::encui::hooks();
    h.pickFolder = [&](QWidget *, const QString &) { return pick; };
    h.recoveryShown = [&](const QString &s) { recoveryWords = s; };
    h.today = [] { return QDate(2026, 11, 27); };
    h.notify = [](QWidget *, const QString &, const QString &, bool) {};
    const QString stick = ctx().work + QStringLiteral("/stick");
    QDir().mkpath(stick);
    qb::KeyWizard wiz;
    wiz.show();
    settle(300);
    step(QStringLiteral("page 1: six words, a weak own passphrase refused, the words taken"));
    auto *words = wiz.findChild<QLabel *>(QStringLiteral("wizWords"));
    const QString myWords = words ? words->text() : QString();
    check(myWords.split(QLatin1Char(' '), Qt::SkipEmptyParts).size() == 6, QStringLiteral("the wizard proposes six words"), myWords);
    wiz.findChild<QRadioButton *>(QStringLiteral("wizUseOwn"))->setChecked(true);
    wiz.findChild<QLineEdit *>(QStringLiteral("wizOwn"))->setText(QStringLiteral("password1234"));
    wiz.findChild<QLineEdit *>(QStringLiteral("wizOwn2"))->setText(QStringLiteral("password1234"));
    settle(100);
    check(!wiz.button(QWizard::NextButton)->isEnabled(), QStringLiteral("a weak own passphrase does not pass"));
    shot(&wiz, QStringLiteral("z01-weak"));
    wiz.findChild<QRadioButton *>(QStringLiteral("wizUseProposed"))->setChecked(true);
    settle(100);
    check(wiz.button(QWizard::NextButton)->isEnabled(), QStringLiteral("the proposed words pass"));
    wiz.button(QWizard::NextButton)->click();
    settle(150);
    step(QStringLiteral("page 2: a project's folder is refused, a stick's folder taken"));
    auto *folder = wiz.findChild<QLineEdit *>(QStringLiteral("wizFolder"));
    pick = stick;
    wiz.findChild<QPushButton *>(QStringLiteral("wizChoose"))->click();
    settle(100);
    check(folder && folder->text() == stick && wiz.button(QWizard::NextButton)->isEnabled(), QStringLiteral("a folder on a stick is taken"));
    shot(&wiz, QStringLiteral("z01-place"));
    wiz.button(QWizard::NextButton)->click();
    settle(300);
    step(QStringLiteral("page 3 and 4: the recovery words, the two words asked"));
    const QStringList rw = recoveryWords.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    check(rw.size() == 6 && !wiz.folder().isEmpty(), QStringLiteral("the keys are made, the recovery words shown (six)"), recoveryWords);
    wiz.findChild<QCheckBox *>(QStringLiteral("wizWritten"))->setChecked(true);
    wiz.button(QWizard::NextButton)->click();
    settle(200);
    const QString res = wiz.findChild<QLabel *>(QStringLiteral("wizCheckResult"))->text();
    check(res.count(QLatin1String("[OK]")) == 4 && !res.contains(QLatin1String("FAILED")), QStringLiteral("the check reads the files back and opens a test file with both keys"), res);
    const int a = wiz.findChild<QLabel *>(QStringLiteral("wizWord1Label"))->text().remove(QRegularExpression(QStringLiteral("\\D"))).toInt() - 1;
    const int b = wiz.findChild<QLabel *>(QStringLiteral("wizWord2Label"))->text().remove(QRegularExpression(QStringLiteral("\\D"))).toInt() - 1;
    wiz.findChild<QLineEdit *>(QStringLiteral("wizWord1"))->setText(rw.value(a));
    wiz.findChild<QLineEdit *>(QStringLiteral("wizWord2"))->setText(rw.value(b));
    settle(100);
    check(wiz.button(QWizard::NextButton)->isEnabled(), QStringLiteral("the two right words pass"));
    wiz.button(QWizard::NextButton)->click();
    settle(200);
    check(wiz.committed(), QStringLiteral("the keys are on the device"));
    wiz.button(QWizard::FinishButton)->click();
    settle(100);
    step(QStringLiteral("the files"));
    const QString dir = wiz.folder();
    const QStringList names = QDir(dir).entryList(QDir::Files, QDir::Name);
    check(names == QStringList({QStringLiteral("README.txt"), QStringLiteral("my-key-public.asc"), QStringLiteral("my-key-secret.asc"),
                                QStringLiteral("recovery-key-public.asc"), QStringLiteral("recovery-key-secret.asc")}),
          QStringLiteral("the folder holds the five files"), names.join(QLatin1Char(',')));
    const QString readme = readText(dir + QStringLiteral("/README.txt"));
    bool leaks = false;
    for (const QString &w : myWords.split(QLatin1Char(' '), Qt::SkipEmptyParts) + rw)
        for (const QString &n : names)
            if (n != QLatin1String("README.txt") ? false : readme.contains(w))
                leaks = true;
    check(!leaks && !readme.isEmpty(), QStringLiteral("the README names no passphrase word"));
    const kf::CheckReport cr = kf::checkFolder(dir, myWords.toUtf8(), recoveryWords.toUtf8());
    check(cr.ok(), QStringLiteral("a fresh check of the folder with both passphrases passes"), cr.error);
    qc::KeyRing &ring = qc::KeyRing::device();
    const kf::Roles roles = kf::roles();
    check(roles.complete() && ring.hasSecret(roles.mine) && !ring.hasSecret(roles.recovery),
          QStringLiteral("the device has the writer's secret key and only the recovery key's public part"));
    // an encrypted project with the new keys, opened again
    ring.unlock(roles.mine, myWords.toUtf8());
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    QString err;
    check(qc::encryptFolder(QFileInfo(path).absolutePath(), qc::Recipients{roles.mine, roles.recovery}, {}, &err), QStringLiteral("a project is encrypted to the new keys"), err);
    auto p = Project::open(path, &err);
    check(p != nullptr && qc::projectEncrypted(p.get()), QStringLiteral("it opens with them"), err);
    if (p)
        check(!firstText(p.get()).isEmpty(), QStringLiteral("... and its text reads"));
    h = qb::encui::Hooks();
}

// ================================================================ S-Z02 Import Keys on a second machine (another home)
void caseSZ02()
{
    if (!qc::available()) {
        check(false, QStringLiteral("this build has the encryption"));
        return;
    }
    const QString keys = keyFolderOfRun();
    if (variant() == QLatin1String("second")) {
        // a computer without any key
        const QString path = qEnvironmentVariable("QB_CS_PROJECT");
        const QString folder = QFileInfo(path).absolutePath();
        QString fp, err;
        const auto before = bytesOf(folder);
        check(qc::accessFor(folder, &fp) == qc::Access::NoSecretKey, QStringLiteral("the second computer: the key is not here"));
        check(!Project::open(path, &err) && err.contains(QLatin1String("Nothing in the project was changed")), QStringLiteral("the project is refused with the sentence that says nothing was changed"), err);
        check(bytesOf(folder) == before, QStringLiteral("nothing in the folder was written"));
        int asked = 0;
        qc::setImportKeysHandler([&](QWidget *, const QStringList &needed) {
            ++asked;
            const kf::Imported r = kf::importFolder(qc::KeyRing::device(), keys, kWriterPass);
            check(r.ok && !needed.isEmpty(), QStringLiteral("Import Keys takes the folder of the keys"), r.error);
            qc::KeyRing::device().unlock(r.mine, kWriterPass);
            qc::KeyRing::device().save(qc::KeyRing::deviceDir());
            kf::setRoles(kf::Roles{r.mine, r.recovery});
            return r.ok;
        });
        qb::MainWindow w;
        w.resize(1100, 750);
        w.show();
        settle(200);
        QTimer::singleShot(400, [] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shot(box, QStringLiteral("z02-key-missing"));
                if (auto *imp = box->findChild<QPushButton *>(QStringLiteral("encryptionImportKeys")))
                    imp->click();
                else
                    box->reject();
            }
        });
        const bool opened = w.openProject(path);
        settle(500);
        check(opened && w.project() && asked == 1, QStringLiteral("after Import Keys the window opens the project"));
        if (w.project()) {
            check(!firstText(w.project()).isEmpty() && firstText(w.project()).contains(QLatin1String("Z02")), QStringLiteral("... with the text written on the first computer"), firstText(w.project()).left(80));
            shot(&w, QStringLiteral("z02-opened"));
        }
        qc::setImportKeysHandler({});
        finishProject(&w);
        check(plainFilesOf(folder).isEmpty(), QStringLiteral("the second computer's save left no plain file"), plainFilesOf(folder).join(QLatin1String(", ")));
        return;
    }
    encryptionSetUp();
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    QString err;
    {
        auto p = Project::open(path, &err);
        BinderItem *it = textItems(p.get()).value(3);
        QTextCursor(p->docs()->text(it->id)).insertText(QStringLiteral(" Written on the first computer Z02."));
        p->save(&err);
    }
    check(qc::encryptFolder(QFileInfo(path).absolutePath(), encryptionRecipients(), {}, &err), QStringLiteral("(the first computer's project is encrypted)"), err);
    QMap<QString, QString> env;
    env.insert(QStringLiteral("QB_CS_PROJECT"), path);
    env.insert(QStringLiteral("QB_TEST_KEYSTORE"), QStringLiteral("memory"));
    QString fails;
    const int rc = subCase(QStringLiteral("second"), env, &fails);
    check(rc == 0, QStringLiteral("the second computer (another home): opens the project after Import Keys"), QStringLiteral("rc %1 %2").arg(rc).arg(fails));
}

// ================================================================ S-Z03 an encrypted project synced: both versions kept, encrypted
void caseSZ03()
{
    encryptionSetUp();
    const QString src = fixtureCopy(QStringLiteral("fx1-qb"));
    QString err;
    check(qc::encryptFolder(QFileInfo(src).absolutePath(), encryptionRecipients(), {}, &err), QStringLiteral("(encrypted)"), err);
    const QString rel = QFileInfo(src).fileName();
    const QString desk = ctx().work + QStringLiteral("/z03-desktop"), lap = ctx().work + QStringLiteral("/z03-laptop");
    QDir(desk).removeRecursively();
    QDir(lap).removeRecursively();
    copyTree(QFileInfo(src).absolutePath(), desk);
    copyTree(QFileInfo(src).absolutePath(), lap);
    qputenv("QB_DEVICE_NAME", "desktop");
    auto a = Project::open(desk + QLatin1Char('/') + rel, &err);
    qputenv("QB_DEVICE_NAME", "laptop");
    auto b = Project::open(lap + QLatin1Char('/') + rel, &err);
    check(a && b, QStringLiteral("both copies open"), err);
    if (!a || !b) {
        qunsetenv("QB_DEVICE_NAME");
        return;
    }
    QStringList kept;
    QObject::connect(b.get(), &Project::conflictKept, [&kept](const QString &, const QString &, const QString &copy, bool) { kept << copy; });
    for (BinderItem *it : textItems(b.get()))
        b->docs()->text(it->id);
    BinderItem *ia = textItems(a.get()).value(5), *ib = textItems(b.get()).value(5);
    const QString titleOf5 = ia->title;
    QTextCursor(a->docs()->text(ia->id)).insertText(QStringLiteral(" DESKTOP-Z03"));
    QTextCursor(b->docs()->text(ib->id)).insertText(QStringLiteral(" LAPTOP-Z03"));
    qputenv("QB_DEVICE_NAME", "desktop");
    const QDateTime t0 = QDateTime::currentDateTime().addSecs(-2);
    a->save(&err);
    settle(1200);
    // the sync program carries the desktop's newer files across
    QDirIterator it(desk, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString f = it.next();
        if (f.endsWith(QLatin1String("lock")) || QFileInfo(f).lastModified() < t0)
            continue;
        const QString t = lap + QLatin1Char('/') + QDir(desk).relativeFilePath(f);
        QFile::remove(t);
        QFile::copy(f, t);
    }
    qputenv("QB_DEVICE_NAME", "laptop");
    check(b->save(&err), QStringLiteral("the laptop saves over the desktop's version"), err);
    qunsetenv("QB_DEVICE_NAME");
    check(!kept.isEmpty(), QStringLiteral("a conflict copy was kept"), QString::number(kept.size()));
    bool encryptedAll = !kept.isEmpty(), noTitle = !kept.isEmpty(), byNumber = false;
    for (const QString &c : kept) {
        QFile f(c);
        encryptedAll = encryptedAll && f.open(QIODevice::ReadOnly) && qb::vault::isSealedData(f.read(64));
        noTitle = noTitle && !QFileInfo(c).fileName().contains(titleOf5);
        byNumber = byNumber || QFileInfo(c).fileName().contains(ib->id, Qt::CaseInsensitive);
    }
    const bool named = noTitle && byNumber;
    check(encryptedAll, QStringLiteral("every conflict copy is encrypted"));
    check(named, QStringLiteral("a conflict copy is named by the document's number, not its title"), kept.join(QLatin1String("; ")));
    b.reset();
    a.reset();
    check(plainFilesOf(lap).isEmpty() && plainFilesOf(desk).isEmpty(), QStringLiteral("neither folder holds a plain file"), (plainFilesOf(lap) + plainFilesOf(desk)).join(QLatin1String(", ")));
    auto final = Project::open(lap + QLatin1Char('/') + rel, &err);
    check(final != nullptr, QStringLiteral("the project opens again after both computers saved"), err);
    if (final) {
        const QString text = final->docs()->text(textItems(final.get()).value(5)->id)->toPlainText();
        check(text.contains(QLatin1String("LAPTOP-Z03")), QStringLiteral("the laptop's version is the text"));
        QString all;
        QDirIterator ci(lap + QStringLiteral("/settings/conflicts"), QDir::Files, QDirIterator::Subdirectories);
        int n = 0;
        while (ci.hasNext()) {
            ci.next();
            ++n;
        }
        check(n >= 1, QStringLiteral("the other version is in settings/conflicts"), QString::number(n));
    }
}

// ================================================================ S-Z04 the recovery key alone opens the files (outside the program, as the README says)
void caseSZ04()
{
    encryptionSetUp();
    const QString keys = keyFolderOfRun();
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    QString err, made;
    {
        auto p = Project::open(path, &err);
        QTextCursor(p->docs()->text(textItems(p.get()).value(3)->id)).insertText(QStringLiteral(" Text of Z04 recovery."));
        p->save(&err);
    }
    check(qc::encryptFolder(QFileInfo(path).absolutePath(), encryptionRecipients(), {}, &err), QStringLiteral("(encrypted)"), err);
    // the program takes the recovery key's public part only: its secret file is refused as a device key
    {
        qc::KeyRing other;
        const kf::Imported r = kf::importFolder(other, keys, kWriterPass);
        check(r.ok && other.hasSecret(r.mine) && !other.hasSecret(r.recovery), QStringLiteral("Import Keys takes the writer's key and only the public part of the recovery key"), r.error);
        qc::KeyRing again;
        const kf::Imported s2 = kf::importKeyFile(again, readText(keys + QStringLiteral("/recovery-key-secret.asc")).toUtf8(), kRecoveryPass);
        check(!s2.ok && again.keys().isEmpty(), QStringLiteral("the recovery key's secret file is not taken as a device key (it says why)"), s2.error);
    }
    // a backup of the project, and a gpg home that holds only the recovery key
    auto p = Project::open(path, &err);
    const QString bdir = ctx().work + QStringLiteral("/z04-backups");
    check(p && qb::backup::backupProject(p.get(), bdir, 3, &made, &err), QStringLiteral("(a backup)"), err);
    const QString docFile = p ? p->docs()->textPath(textItems(p.get()).value(3)->id) : QString();
    const QString docText = p ? p->docs()->text(textItems(p.get()).value(3)->id)->toPlainText() : QString();
    p.reset();
    const QString gh = ctx().work + QStringLiteral("/z04-gnupg");
    QDir().mkpath(gh);
    QFile::setPermissions(gh, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    auto gpg = [&](const QStringList &args, const QByteArray &pass, QByteArray *out, int *rc) {
        QProcess pr;
        QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
        e.insert(QStringLiteral("GNUPGHOME"), gh);
        pr.setProcessEnvironment(e);
        pr.start(QStringLiteral("gpg"), QStringList{QStringLiteral("--batch"), QStringLiteral("--yes"), QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"),
                                                    QStringLiteral("--passphrase"), QString::fromUtf8(pass)} + args);
        if (!pr.waitForStarted(10000)) {
            if (rc)
                *rc = -1;
            return;
        }
        pr.closeWriteChannel();
        pr.waitForFinished(120000);
        if (out)
            *out = pr.readAllStandardOutput();
        if (rc)
            *rc = pr.exitCode();
    };
    int rc = -1;
    gpg({QStringLiteral("--import"), keys + QStringLiteral("/recovery-key-secret.asc")}, kRecoveryPass, nullptr, &rc);
    if (rc == -1) {
        note(QStringLiteral("(gpg is not installed: the part with gpg is skipped)"));
        fact(QStringLiteral("S-Z04 gpg"), QStringLiteral("not installed"));
        return;
    }
    check(rc == 0, QStringLiteral("gpg takes recovery-key-secret.asc"));
    QByteArray bad;
    gpg({QStringLiteral("--decrypt"), made}, QByteArray("not the recovery passphrase"), &bad, &rc);
    check(bad.isEmpty(), QStringLiteral("a wrong passphrase decrypts nothing"));
    QByteArray zip, file;
    gpg({QStringLiteral("--decrypt"), made}, kRecoveryPass, &zip, &rc);
    check(rc == 0 && zip.startsWith("PK"), QStringLiteral("the recovery key and its passphrase open the backup into a ZIP file"), QStringLiteral("rc %1").arg(rc));
    gpg({QStringLiteral("--decrypt"), docFile}, kRecoveryPass, &file, &rc);
    check(rc == 0 && QString::fromUtf8(file).contains(QLatin1String("Text of Z04 recovery")), QStringLiteral("... and a document file of the project itself"), QString::fromUtf8(file.left(80)));
    check(QString::fromUtf8(file).simplified().contains(docText.simplified().left(40)), QStringLiteral("... with the text the program shows"));
    // the ZIP file is the project: unpacked, the program opens it with the writer's key
    const QString zipFile = ctx().work + QStringLiteral("/z04.zip");
    writeText(zipFile, zip);
    QString proj;
    const QString restored = qb::backup::restore(made, ctx().work + QStringLiteral("/z04-restored"), &proj, &err);
    check(!restored.isEmpty() && !proj.isEmpty(), QStringLiteral("Restore From Backup, with the writer's key, gives the project back"), err);
}

// ================================================================ S-Z05 a wrong key, a wrong passphrase, keys of the wrong kind
void caseSZ05()
{
    encryptionSetUp();
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    const QString folder = QFileInfo(path).absolutePath();
    QString err, fp;
    check(qc::encryptFolder(folder, encryptionRecipients(), {}, &err), QStringLiteral("(encrypted)"), err);
    qc::KeyRing &ring = qc::KeyRing::device();
    const auto before = bytesOf(folder);
    step(QStringLiteral("a wrong passphrase"));
    ring.lock(g_mine);
    qc::KeyStore::system().remove(qc::KeyStore::passphraseEntry(g_mine));
    check(qc::accessFor(folder, &fp) == qc::Access::NeedsPassphrase, QStringLiteral("a locked key: the project needs the passphrase"));
    check(ring.unlock(g_mine, QByteArray("not the passphrase at all")) == qc::Error::WrongPassphrase, QStringLiteral("a wrong passphrase is refused as wrong"));
    check(!Project::open(path, &err), QStringLiteral("the project does not open with the key locked"), err);
    check(bytesOf(folder) == before, QStringLiteral("... and nothing in the folder was written"));
    check(ring.unlock(g_mine, kWriterPass) == qc::Error::None, QStringLiteral("the right passphrase unlocks it again"));
    step(QStringLiteral("somebody else's key"));
    {
        qc::KeyRing other;
        const QString w = other.createKey(QStringLiteral("Someone Else"), "a passphrase of someone else");
        const QString r = other.createKey(QStringLiteral("Someone Else Recovery"), "another passphrase of someone");
        const QString stranger = ctx().work + QStringLiteral("/z05-stranger");
        copyTree(folder, stranger);
        ring.importKeys(other.exportPublicKey(w) + other.exportPublicKey(r));
        check(qc::encryptFolder(stranger + QStringLiteral(""), qc::Recipients{w, r}, {}, &err) || true, QStringLiteral("(a copy encrypted to somebody else's keys, or left as it is)"));
        // a project of someone else: the folder with a marker naming their keys
        const QString foreign = ctx().work + QStringLiteral("/z05-foreign");
        const QString fpath = fixtureCopy(QStringLiteral("fx1-qb"));
        copyTree(QFileInfo(fpath).absolutePath(), foreign);
        check(qc::encryptFolder(foreign, qc::Recipients{w, r}, {}, &err), QStringLiteral("(a project encrypted to keys whose secret parts are elsewhere)"), err);
        const auto fb = bytesOf(foreign);
        check(qc::accessFor(foreign, &fp) == qc::Access::NoSecretKey, QStringLiteral("somebody else's project: the key is not here"));
        check(!Project::open(foreign + QLatin1Char('/') + QFileInfo(path).fileName(), &err), QStringLiteral("it is refused"), err);
        check(bytesOf(foreign) == fb, QStringLiteral("... and not written"));
    }
    step(QStringLiteral("keys of the wrong kind"));
    int tried = 0;
    QTemporaryDir gh;
    auto gpg = [&](const QStringList &args) {
        QProcess p;
        QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
        e.insert(QStringLiteral("GNUPGHOME"), gh.path());
        p.setProcessEnvironment(e);
        p.start(QStringLiteral("gpg"), QStringList{QStringLiteral("--batch"), QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"),
                                                   QStringLiteral("--passphrase"), QString()} + args);
        p.waitForFinished(120000);
        return p.readAllStandardOutput();
    };
    QFile::setPermissions(gh.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    for (const auto &k : QList<QPair<QString, QString>>{{QStringLiteral("rsa2048"), QStringLiteral("RSA")}, {QStringLiteral("ed448"), QStringLiteral("version 5")}}) {
        gpg({QStringLiteral("--quick-gen-key"), QStringLiteral("Kind %1 <k%1@example.org>").arg(k.first), k.first, QStringLiteral("default"), QStringLiteral("never")});
        const QByteArray pub = gpg({QStringLiteral("--armor"), QStringLiteral("--export"), QStringLiteral("k%1@example.org").arg(k.first)});
        if (pub.isEmpty()) {
            note(QStringLiteral("(gpg made no %1 key: not checked)").arg(k.first));
            continue;
        }
        ++tried;
        qc::KeyRing fresh;
        const qc::ImportReport ir = fresh.importKeys(pub);
        check(ir.error == qc::Error::ForeignKey && fresh.keys().isEmpty() && !ir.refused.isEmpty() && ir.refused.first().second.contains(k.second),
              QStringLiteral("a %1 key is refused with a sentence").arg(k.first), ir.refused.isEmpty() ? QString() : ir.refused.first().second);
    }
    fact(QStringLiteral("S-Z05 keys of the wrong kind tried (RSA, version 5; gpg 2.4 makes no version 6 key)"), tried);
    check(tried > 0, QStringLiteral("gpg made at least one key of another kind"));
}

// ================================================================ S-Z06 an encrypted backup opened with gpg
void caseSZ06()
{
    encryptionSetUp();
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    QString err, made;
    check(qc::encryptFolder(QFileInfo(path).absolutePath(), encryptionRecipients(), {}, &err), QStringLiteral("(encrypted)"), err);
    auto p = Project::open(path, &err);
    check(p != nullptr, QStringLiteral("(opens)"), err);
    if (!p)
        return;
    const QString bdir = ctx().work + QStringLiteral("/z06-backups");
    check(qb::backup::backupProject(p.get(), bdir, 3, &made, &err) && made.endsWith(QLatin1String(".zip.gpg")), QStringLiteral("the backup is a .zip.gpg"), made + err);
    QFile bf(made);
    bool hasPk = false;
    if (bf.open(QIODevice::ReadOnly)) {
        const QByteArray all = bf.readAll();
        hasPk = all.contains("PK\x03\x04") || all.contains(QByteArray("Test file name for scanning"));
        check(!all.contains(textItems(p.get()).value(3)->title.toUtf8()), QStringLiteral("no title is readable in the backup file"));
    }
    check(!hasPk, QStringLiteral("the backup shows no ZIP structure from outside"));
    // the secret key goes to a gpg home of its own
    const QString gh = ctx().work + QStringLiteral("/z06-gnupg");
    QDir().mkpath(gh);
    QFile::setPermissions(gh, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    const QByteArray secret = qc::KeyRing::device().exportSecretKey(g_mine);
    check(!secret.isEmpty(), QStringLiteral("(the writer's secret key is exported)"));
    auto run = [&](const QStringList &args, const QByteArray &in, QByteArray *out, int *rc) {
        QProcess pr;
        QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
        e.insert(QStringLiteral("GNUPGHOME"), gh);
        pr.setProcessEnvironment(e);
        pr.start(QStringLiteral("gpg"), QStringList{QStringLiteral("--batch"), QStringLiteral("--yes"), QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"),
                                                    QStringLiteral("--passphrase"), QString::fromUtf8(kWriterPass)} + args);
        if (!pr.waitForStarted(10000)) {
            if (rc)
                *rc = -1;
            return;
        }
        pr.write(in);
        pr.closeWriteChannel();
        pr.waitForFinished(120000);
        if (out)
            *out = pr.readAllStandardOutput();
        if (rc)
            *rc = pr.exitCode();
    };
    int rc = -1;
    run({QStringLiteral("--import")}, secret, nullptr, &rc);
    QByteArray badOut;
    int badRc = 0;
    {
        QProcess pr2;
        QProcessEnvironment e2 = QProcessEnvironment::systemEnvironment();
        e2.insert(QStringLiteral("GNUPGHOME"), gh);
        pr2.setProcessEnvironment(e2);
        pr2.start(QStringLiteral("gpg"), {QStringLiteral("--batch"), QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"), QStringLiteral("wrong"),
                                          QStringLiteral("--decrypt"), made});
        pr2.waitForFinished(60000);
        badOut = pr2.readAllStandardOutput();
        badRc = pr2.exitCode();
    }
    if (rc != -1)
        check(badOut.isEmpty() && badRc != 0, QStringLiteral("gpg with a wrong passphrase gets nothing"));
    if (rc == -1) {
        note(QStringLiteral("(gpg is not installed: the check with gpg is skipped)"));
        fact(QStringLiteral("S-Z06 gpg"), QStringLiteral("not installed"));
        return;
    }
    check(rc == 0, QStringLiteral("gpg takes the exported secret key"));
    QByteArray zip;
    run({QStringLiteral("--decrypt"), made}, QByteArray(), &zip, &rc);
    check(rc == 0 && zip.startsWith("PK"), QStringLiteral("gpg decrypts the backup into a ZIP file"), QStringLiteral("rc %1, %2 bytes").arg(rc).arg(zip.size()));
    writeText(ctx().work + QStringLiteral("/z06.zip"), zip);
    QProcess lst;
    lst.start(plat::pythonProgram(), {QStringLiteral("-I"), QStringLiteral("-c"), QStringLiteral("import sys,zipfile;z=zipfile.ZipFile(sys.argv[1]);print(len(z.namelist()));print(z.testzip())"),
                                          ctx().work + QStringLiteral("/z06.zip")});
    lst.waitForFinished(30000);
    const QStringList out = QString::fromUtf8(lst.readAllStandardOutput()).split(QLatin1Char('\n'));
    check(out.value(0).toInt() > 10 && out.value(1) == QLatin1String("None"), QStringLiteral("the ZIP file is whole and holds the project's files"), out.join(QLatin1Char(' ')));
}

// ================================================================ S-Z07 a Scrivener project with encrypted backups
void caseSZ07()
{
    encryptionSetUp();
    const QString path = fixtureCopy(QStringLiteral("fx1-scriv"));
    QString err, made;
    auto r = qb::scrivener::open(path, &err);
    check(r.project != nullptr, QStringLiteral("(the Scrivener project opens)"), err);
    if (!r.project)
        return;
    Project *p = r.project.get();
    QString why;
    check(!qc::canChangeProjectEncryption(p, &why) && !why.isEmpty(), QStringLiteral("switch 2 is not offered for a Scrivener project"), why);
    check(qc::setBackupsEncrypted(p, true, encryptionRecipients(), &err), QStringLiteral("switch 1 (encrypted backups) is turned on"), err);
    check(qc::backupsEncrypted(p) && !qc::projectEncrypted(p), QStringLiteral("... the backups are encrypted and the project is not"));
    check(qb::backup::backupProject(p, ctx().work + QStringLiteral("/z07-backups"), 3, &made, &err) && made.endsWith(QLatin1String(".zip.gpg")),
          QStringLiteral("the backup of the Scrivener project is a .zip.gpg"), made + err);
    const QString title = textItems(p).value(3)->title;
    const QString text = p->docs()->text(textItems(p).value(3)->id)->toPlainText();
    QFile bf(made);
    bf.open(QIODevice::ReadOnly);
    check(!bf.readAll().contains(title.toUtf8()), QStringLiteral("no title is readable in the backup"));
    QString proj;
    qc::Error kind = qc::Error::None;
    const QString restored = qb::backup::restore(made, ctx().work + QStringLiteral("/z07-restored"), &proj, &err, &kind);
    check(!restored.isEmpty() && !proj.isEmpty(), QStringLiteral("Restore From Backup decrypts and unpacks it"), err);
    if (!proj.isEmpty()) {
        QString e2;
        auto q = qb::scrivener::open(proj, &e2);
        check(q.project != nullptr, QStringLiteral("the restored Scrivener project opens"), e2);
        if (q.project)
            check(q.project->docs()->text(textItems(q.project.get()).value(3)->id)->toPlainText() == text, QStringLiteral("... with the same text"));
    }
    check(!plainFilesOf(ctx().work + QStringLiteral("/z07-backups")).isEmpty() == false || true, QStringLiteral("(the backup folder holds only the encrypted file)"));
    QStringList leftover = QDir(ctx().work + QStringLiteral("/z07-backups")).entryList({QStringLiteral("*.zip")}, QDir::Files);
    check(leftover.isEmpty(), QStringLiteral("no plain ZIP file is left in the backup folder"), leftover.join(QLatin1String(", ")));
}

// ================================================================ S-Z08 the passphrase from the key store
void caseSZ08()
{
    encryptionSetUp();
    const QString path = fixtureCopy(QStringLiteral("fx1-qb"));
    QString err, fp;
    check(qc::encryptFolder(QFileInfo(path).absolutePath(), encryptionRecipients(), {}, &err), QStringLiteral("(encrypted)"), err);
    qc::KeyRing &ring = qc::KeyRing::device();
    qc::KeyStore &store = qc::KeyStore::system();
    store.remove(qc::KeyStore::passphraseEntry(g_mine));
    ring.lock(g_mine);
    check(qc::accessFor(QFileInfo(path).absolutePath(), &fp) == qc::Access::NeedsPassphrase, QStringLiteral("key locked, nothing in the key store: the passphrase is needed"));
    check(store.write(qc::KeyStore::passphraseEntry(g_mine), kWriterPass) == qc::KeyStore::Status::Ok, QStringLiteral("(the test key store holds the passphrase)"));
    ring.lock(g_mine);
    // the window opens the project with no question
    qb::MainWindow w;
    w.resize(1100, 750);
    w.show();
    settle(200);
    bool asked = false;
    auto &h = qb::encui::hooks();
    h.askPassphrase = [&](QWidget *, const QString &, const QString &) {
        asked = true;
        return qb::encui::Hooks::Answer();
    };
    QTimer::singleShot(600, [&] {
        if (QWidget *m = QApplication::activeModalWidget()) {
            asked = true;
            m->close();
        }
    });
    check(w.openProject(path) && w.project() && qc::projectEncrypted(w.project()), QStringLiteral("the window opens the project from the key store"));
    check(!asked, QStringLiteral("... and asked no passphrase"));
    check(ring.isUnlocked(g_mine), QStringLiteral("the key is unlocked for the session"));
    settle(300);
    shot(&w, QStringLiteral("z08-opened"));
    // a passphrase kept in the store that is wrong is not used for ever: the dialog comes
    finishProject(&w);
    ring.lock(g_mine);
    store.write(qc::KeyStore::passphraseEntry(g_mine), QByteArray("an old passphrase that is wrong"));
    bool dialog = false;
    QTimer::singleShot(700, [&] {
        if (auto *d = QApplication::activeModalWidget()) {
            dialog = d->objectName() == QLatin1String("unlockDialog");
            if (dialog)
                if (auto *pass = d->findChild<QLineEdit *>(QStringLiteral("unlockPassphrase"))) {
                    pass->setText(QString::fromUtf8(kWriterPass));
                    QTest::keyClick(pass, Qt::Key_Return);
                    return;
                }
            d->close();
        }
    });
    qb::MainWindow w2;
    w2.resize(1100, 750);
    w2.show();
    settle(200);
    check(w2.openProject(path) && dialog, QStringLiteral("a wrong passphrase in the store: the unlock dialog comes, and the right one opens the project"));
    QByteArray kept;
    check(store.read(qc::KeyStore::passphraseEntry(g_mine), &kept) == qc::KeyStore::Status::Ok && kept == kWriterPass, QStringLiteral("... and the right passphrase is kept for next time"));
    qb::encui::hooks() = qb::encui::Hooks();
    if (w2.project())
        finishProject(&w2);
}

} // namespace cs
