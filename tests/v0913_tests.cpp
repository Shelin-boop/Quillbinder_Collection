// Checks of 0.91.3 beta: add-on keys (the writer's decision of 2026-11-15: one key for program updates, one or more
// for official add-ons, unsigned add-ons still possible). An add-on key is trusted when its certificate, signed by the
// release key, travels in the package's manifest.sig; it vouches for add-ons only, never for a program update.
// The packages, keys and certificates are made the way the release workflow makes them: openssl and
// tools/make-package.py (certify, sign, verify). The install window is looked at once (screenshots into the folder
// given as first argument).
//
//   xvfb-run -a ./qb_v0913_tests /tmp/shots
#include "AppInfo.h"
#include "core/Addons.h"
#include "core/Launcher.h"
#include "core/Signing.h"
#include "ui/AddonDialogs.h"

#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTest>
#include <QTimer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression> // the AppImage's runtime (Windows 0.91.3) and Android's (2026-11-16) from CMakeLists.txt
#include <QTemporaryDir>

#include <cstdio>

using namespace qb;

namespace {
int g_failures = 0;
int g_passes = 0;

void check(bool ok, const QString &what, const QString &detail = QString())
{
    if (ok) {
        ++g_passes;
        std::printf("PASS  %s\n", qPrintable(what));
    } else {
        ++g_failures;
        std::printf("FAIL  %s\n", qPrintable(what));
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(1500)));
    }
    std::fflush(stdout);
}

QString run(const QString &program, const QStringList &args, int *rc = nullptr,
            const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment())
{
    QProcess p;
    p.setProcessEnvironment(env);
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(program, args);
    p.waitForFinished(120000);
    if (rc)
        *rc = p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1;
    return QString::fromLocal8Bit(p.readAll());
}

QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

void writeAll(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
}

QByteArray publicOf(const QString &keyFile)
{
    QProcess p;
    p.start(QStringLiteral("openssl"), {QStringLiteral("pkey"), QStringLiteral("-in"), keyFile, QStringLiteral("-pubout"),
                                        QStringLiteral("-outform"), QStringLiteral("DER")});
    p.waitForFinished();
    return p.readAllStandardOutput().right(32);
}

QByteArray signRaw(const QString &keyFile, const QByteArray &data, const QString &work)
{
    writeAll(work + QStringLiteral("/msg"), data);
    run(QStringLiteral("openssl"), {QStringLiteral("pkeyutl"), QStringLiteral("-sign"), QStringLiteral("-rawin"), QStringLiteral("-inkey"),
                                    keyFile, QStringLiteral("-in"), work + QStringLiteral("/msg"), QStringLiteral("-out"),
                                    work + QStringLiteral("/sig")});
    return readAll(work + QStringLiteral("/sig"));
}

// a package's manifest.sig replaced (python's zipfile: the package's other entries stay as they are)
void replaceSig(const QString &package, const QByteArray &sig, const QString &work)
{
    writeAll(work + QStringLiteral("/new.sig"), sig);
    run(QStringLiteral("python3"),
        {QStringLiteral("-I"), QStringLiteral("-c"),
         QStringLiteral("import zipfile,sys,shutil\n"
                        "src=zipfile.ZipFile(sys.argv[1]);out=zipfile.ZipFile(sys.argv[1]+'.new','w',zipfile.ZIP_DEFLATED)\n"
                        "for i in src.infolist():\n"
                        " if i.filename!='manifest.sig': out.writestr(i,src.read(i.filename))\n"
                        "out.writestr('manifest.sig',open(sys.argv[2],'rb').read());out.close();shutil.move(sys.argv[1]+'.new',sys.argv[1])"),
         package, work + QStringLiteral("/new.sig")});
}

QByteArray manifestOf(const QString &package, const QString &work)
{
    const QString out = work + QStringLiteral("/manifest.json");
    run(QStringLiteral("python3"), {QStringLiteral("-I"), QStringLiteral("-c"),
                                    QStringLiteral("import zipfile,sys;open(sys.argv[2],'wb').write(zipfile.ZipFile(sys.argv[1]).read('manifest.json'))"),
                                    package, out});
    return readAll(out);
}

// the install window of a package: its signature note and default button, then Cancel
QString installWindow(const QString &package, const QString &shotFile)
{
    QString seen;
    QTimer::singleShot(600, qApp, [&seen, shotFile] {
        for (QWidget *tl : QApplication::topLevelWidgets())
            if (auto *d = qobject_cast<QDialog *>(tl); d && d->isVisible() && !qobject_cast<QMessageBox *>(d)) {
                if (auto *l = d->findChild<QLabel *>(QStringLiteral("addonSignedNote")))
                    seen += QStringLiteral("signed: ") + l->text() + QLatin1Char(' ');
                if (d->findChild<QLabel *>(QStringLiteral("addonUnsignedWarning")))
                    seen += QStringLiteral("unsigned ");
                for (QPushButton *b : d->findChildren<QPushButton *>())
                    if (b->isDefault())
                        seen += QStringLiteral("default=") + b->text().remove(QLatin1Char('&'));
                d->grab().save(shotFile);
                d->reject();
                return;
            }
    });
    qb::runInstallPackage(nullptr, package);
    QTest::qWait(200);
    return seen;
}
} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QString shots = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("/tmp/shots-v0913");
    QDir().mkpath(shots);
    QTemporaryDir tmp;
    const QString W = tmp.path(), src = QStringLiteral(QB_TEST_SOURCE_DIR) + QStringLiteral("/..");
    const QString py = QStringLiteral("python3"), tool = src + QStringLiteral("/tools/make-package.py");

    // the keys: a release key for this run (the test builds trust QB_TEST_SIGNING_KEY as a release key), an add-on
    // key, and a key the program does not know
    const QString rel = W + QStringLiteral("/release.pem"), addonKey = W + QStringLiteral("/addon.pem"), other = W + QStringLiteral("/other.pem");
    for (const QString &k : {rel, addonKey, other})
        run(QStringLiteral("openssl"), {QStringLiteral("genpkey"), QStringLiteral("-algorithm"), QStringLiteral("ed25519"), QStringLiteral("-out"), k});
    const QByteArray relPub = publicOf(rel), addonPub = publicOf(addonKey);
    qputenv("QB_TEST_SIGNING_KEY", relPub.toHex());
    run(QStringLiteral("openssl"), {QStringLiteral("pkey"), QStringLiteral("-in"), rel, QStringLiteral("-pubout"), QStringLiteral("-out"),
                                    W + QStringLiteral("/release.pub.pem")});

    // the certificate the release workflow makes, and one made by a key the program does not know
    const QString cert = W + QStringLiteral("/cert.json"), fakeCert = W + QStringLiteral("/fake-cert.json");
    QProcessEnvironment withRel = QProcessEnvironment::systemEnvironment();
    withRel.insert(QStringLiteral("QB_SIGNING_KEY_FILE"), rel);
    QProcessEnvironment withOther = QProcessEnvironment::systemEnvironment();
    withOther.insert(QStringLiteral("QB_SIGNING_KEY_FILE"), other);
    int rc = 0;
    QString out = run(py, {QStringLiteral("-I"), tool, QStringLiteral("certify"), QStringLiteral("--name"), QStringLiteral("Quillbinder official add-ons"),
                           QStringLiteral("--issued"), QStringLiteral("2026-11-15"), addonKey, cert}, &rc, withRel);
    run(py, {QStringLiteral("-I"), tool, QStringLiteral("certify"), QStringLiteral("--name"), QStringLiteral("Someone else"), addonKey, fakeCert},
        nullptr, withOther);
    const QJsonObject c = QJsonDocument::fromJson(readAll(cert)).object();
    check(rc == 0 && c.value(QStringLiteral("role")).toString() == QLatin1String("addons")
              && c.value(QStringLiteral("key")).toString() == QString::fromLatin1(addonPub.toHex())
              && c.value(QStringLiteral("by")).toString() == signing::keyId(relPub)
              && signing::verify(signing::certificateText(QStringLiteral("addons"), addonPub, QStringLiteral("Quillbinder official add-ons"),
                                                          QStringLiteral("2026-11-15")),
                                 QByteArray::fromBase64(c.value(QStringLiteral("signature")).toString().toLatin1()), relPub),
          QStringLiteral("make-package.py certify: the release key vouches for the add-on key, for add-ons only, in the program's own form"), out);

    // the packages: an add-on, a program update whose program is a script, and bundles
    const QString addonDir = W + QStringLiteral("/addon");
    writeAll(addonDir + QStringLiteral("/manifest.json"),
             R"({"id": "check-addon", "name": "Check Add-on", "version": "1.0", "description": "An add-on for the checks."})");
    writeAll(addonDir + QStringLiteral("/payload/themes/check.json"), R"({"name": "Check", "base": "light", "colors": {}})");
    const QString prog = W + QStringLiteral("/bin/quillbinder");
    writeAll(prog, "#!/bin/sh\nexit 0\n");
    QFile::setPermissions(prog, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    auto make = [&](const QStringList &args) {
        int r = 0;
        const QString o = run(py, QStringList{QStringLiteral("-I"), tool} + args, &r);
        return r == 0 ? QString() : o;
    };
    const QString viaAddon = W + QStringLiteral("/via-addon-key.qbpkg"), viaRelease = W + QStringLiteral("/via-release-key.qbpkg"),
                  viaFake = W + QStringLiteral("/via-fake-cert.qbpkg"), core = W + QStringLiteral("/core.qbpkg"),
                  coreViaAddon = W + QStringLiteral("/core-via-addon-key.qbpkg"), addonsBundle = W + QStringLiteral("/addons-bundle.qbpkg"),
                  mixedBundle = W + QStringLiteral("/mixed-bundle.qbpkg"), plain = W + QStringLiteral("/plain.qbpkg");
    QString problems = make({QStringLiteral("addon"), addonDir, plain})
                       + make({QStringLiteral("core"), QStringLiteral("--binary"), prog, QStringLiteral("--version"), QStringLiteral("9.9.9"),
                               QStringLiteral("--runtime"), QStringLiteral(QB_RUNTIME_ID), core});
    for (const QString &copy : {viaAddon, viaRelease, viaFake})
        QFile::copy(plain, copy);
    QFile::copy(core, coreViaAddon);
    problems += make({QStringLiteral("bundle"), QStringLiteral("--id"), QStringLiteral("check-addons"), QStringLiteral("--name"), QStringLiteral("Add-ons"),
                      QStringLiteral("--version"), QStringLiteral("1"), plain, addonsBundle})
                + make({QStringLiteral("bundle"), QStringLiteral("--id"), QStringLiteral("check-mixed"), QStringLiteral("--name"), QStringLiteral("Mixed"),
                        QStringLiteral("--version"), QStringLiteral("1"), plain, core, mixedBundle});
    // signed as the release workflow signs with both keys: add-ons with the add-on key, program updates with the release key
    QProcessEnvironment both = withRel;
    both.insert(QStringLiteral("QB_ADDON_KEY_FILE"), addonKey);
    both.insert(QStringLiteral("QB_ADDON_CERT"), cert);
    int r1 = 0, r2 = 0, r3 = 0;
    out = run(py, {QStringLiteral("-I"), tool, QStringLiteral("sign"), viaAddon, core, addonsBundle, mixedBundle}, &r1, both);
    out += run(py, {QStringLiteral("-I"), tool, QStringLiteral("sign"), viaRelease}, &r2, withRel);
    QProcessEnvironment fake = withRel;
    fake.insert(QStringLiteral("QB_ADDON_KEY_FILE"), addonKey);
    fake.insert(QStringLiteral("QB_ADDON_CERT"), fakeCert);
    out += run(py, {QStringLiteral("-I"), tool, QStringLiteral("sign"), viaFake}, &r3, fake);
    check(problems.isEmpty() && r1 == 0 && r2 == 0 && r3 == 0, QStringLiteral("(packages made and signed)"), problems + out);

    // a program update signed with the add-on key and its certificate (the workflow never does this; someone could)
    {
        const QByteArray m = manifestOf(coreViaAddon, W);
        QJsonObject s;
        s.insert(QStringLiteral("algorithm"), QStringLiteral("ed25519"));
        s.insert(QStringLiteral("key"), signing::keyId(addonPub));
        s.insert(QStringLiteral("signature"), QString::fromLatin1(signRaw(addonKey, m, W).toBase64()));
        s.insert(QStringLiteral("signs"), QStringLiteral("manifest.json"));
        s.insert(QStringLiteral("certificate"), c);
        replaceSig(coreViaAddon, QJsonDocument(s).toJson(), W);
    }

    qputenv("QB_REQUIRE_SIGNED", "1");
    const addons::PackageInfo a = addons::inspect(viaAddon), rl = addons::inspect(viaRelease), fk = addons::inspect(viaFake),
                              cr = addons::inspect(core), cra = addons::inspect(coreViaAddon), ab = addons::inspect(addonsBundle),
                              mb = addons::inspect(mixedBundle);
    check(a.isValid() && a.signature == signing::State::Valid && a.trusted() && a.signer == QLatin1String("Quillbinder official add-ons")
              && addons::trustProblem(a, addons::Unsigned::Refuse).isEmpty(),
          QStringLiteral("an add-on signed with an add-on key the release key vouches for is trusted, also on the command line without "
                         "--allow-unsigned"),
          a.error + addons::trustProblem(a, addons::Unsigned::Refuse));
    check(rl.isValid() && rl.signature == signing::State::Valid && rl.trusted() && rl.signer.isEmpty(),
          QStringLiteral("an add-on signed with the release key (all add-ons published before 0.91.3) stays trusted"), rl.error);
    check(fk.isValid() && fk.signature == signing::State::UnknownKey && !fk.trusted()
              && addons::trustProblem(fk, addons::Unsigned::AllowAddons).isEmpty() && !addons::trustProblem(fk, addons::Unsigned::Refuse).isEmpty(),
          QStringLiteral("an add-on key vouched for by a key the program does not know counts as not signed: after the warning yes, on "
                         "the command line only with --allow-unsigned"),
          addons::trustProblem(fk, addons::Unsigned::Refuse));
    check(cr.isValid() && cr.trusted() && addons::trustProblem(cr, addons::Unsigned::Refuse).isEmpty(),
          QStringLiteral("make-package.py sign keeps the release key for a program update while an add-on key is set"));
    check(cra.isValid() && cra.signature == signing::State::UnknownKey && !cra.trusted()
              && addons::trustProblem(cra, addons::Unsigned::AllowAddons).contains(QLatin1String("program update")),
          QStringLiteral("a program update signed with an add-on key and its certificate is refused like an unsigned one"),
          addons::trustProblem(cra, addons::Unsigned::AllowAddons));
    {
        QString e;
        QStringList log;
        bool restart = false;
        const bool refused = !addons::install(coreViaAddon, &log, &e, &restart, addons::Unsigned::AllowAddons);
        check(refused && e.contains(QLatin1String("not signed")), QStringLiteral("... and installing it is refused"), e);
    }
    check(ab.isValid() && ab.trusted() && addons::trustProblem(ab, addons::Unsigned::Refuse).isEmpty() && mb.isValid() && mb.trusted()
              && addons::trustProblem(mb, addons::Unsigned::Refuse).isEmpty(),
          QStringLiteral("bundles: add-ons only, signed with the add-on key; add-ons and a program update, each part with its own key"),
          addons::trustProblem(ab, addons::Unsigned::Refuse) + addons::trustProblem(mb, addons::Unsigned::Refuse));
    qunsetenv("QB_REQUIRE_SIGNED");
    {
        const QString viaKey = installWindow(viaAddon, shots + QStringLiteral("/v0913-install-addon-key.png"));
        const QString viaRel = installWindow(viaRelease, shots + QStringLiteral("/v0913-install-release-key.png"));
        const QString viaUnknown = installWindow(viaFake, shots + QStringLiteral("/v0913-install-unknown-key.png"));
        check(viaKey.contains(QLatin1String("\"Quillbinder official add-ons\", which the Quillbinder project vouches for"))
                  && viaKey.endsWith(QLatin1String("default=Install")) && viaRel.contains(QLatin1String("Signed by the Quillbinder project"))
                  && viaUnknown.startsWith(QLatin1String("unsigned default=Cancel")),
              QStringLiteral("the install window names the add-on key the project vouches for, says Signed by the Quillbinder project "
                             "for the release key, and warns of an add-on key nobody vouches for, with Cancel as the default"),
              viaKey + QLatin1String(" / ") + viaRel + QLatin1String(" / ") + viaUnknown);
    }

    // certificates changed or of the wrong kind
    {
        const QByteArray m = manifestOf(viaAddon, W);
        auto sigWith = [&](const QJsonObject &certificate, const QString &key) {
            QJsonObject s;
            s.insert(QStringLiteral("algorithm"), QStringLiteral("ed25519"));
            s.insert(QStringLiteral("key"), signing::keyId(addonPub));
            s.insert(QStringLiteral("signature"), QString::fromLatin1(signRaw(key, m, W).toBase64()));
            s.insert(QStringLiteral("signs"), QStringLiteral("manifest.json"));
            s.insert(QStringLiteral("certificate"), certificate);
            return QJsonDocument(s).toJson();
        };
        QJsonObject renamed = c;
        renamed.insert(QStringLiteral("name"), QStringLiteral("Another name"));
        QJsonObject updates = c;
        updates.insert(QStringLiteral("role"), QStringLiteral("updates"));
        updates.insert(QStringLiteral("signature"), QString::fromLatin1(
                           signRaw(rel, signing::certificateText(QStringLiteral("updates"), addonPub, QStringLiteral("Quillbinder official add-ons"),
                                                                 QStringLiteral("2026-11-15")), W).toBase64()));
        QJsonObject twoLines = c;
        const QString badName = QStringLiteral("Official\nadd-ons");
        twoLines.insert(QStringLiteral("name"), badName);
        twoLines.insert(QStringLiteral("signature"), QString::fromLatin1(
                            signRaw(rel, signing::certificateText(QStringLiteral("addons"), addonPub, badName, QStringLiteral("2026-11-15")), W).toBase64()));
        const signing::Check good = signing::checkManifest(m, sigWith(c, addonKey));
        const signing::Check changedName = signing::checkManifest(m, sigWith(renamed, addonKey));
        const signing::Check wrongRole = signing::checkManifest(m, sigWith(updates, addonKey));
        const signing::Check newline = signing::checkManifest(m, sigWith(twoLines, addonKey));
        const signing::Check otherSigner = signing::checkManifest(m, sigWith(c, other));
        QByteArray changedManifest = m;
        changedManifest.replace("An add-on for the checks.", "An add-on, changed later.");
        const signing::Check changedPackage = signing::checkManifest(changedManifest, sigWith(c, addonKey));
        check(good.state == signing::State::Valid && !good.releaseKey && good.signer == QLatin1String("Quillbinder official add-ons"),
              QStringLiteral("(the certificate as the workflow makes it is accepted)"));
        check(changedName.state == signing::State::Invalid,
              QStringLiteral("a certificate changed after the release key signed it (another name) is refused as changed"));
        check(wrongRole.state == signing::State::UnknownKey && newline.state == signing::State::UnknownKey,
              QStringLiteral("a certificate for another role, or with a name of two lines, vouches for nothing"));
        check(otherSigner.state == signing::State::Invalid && changedPackage.state == signing::State::Invalid,
              QStringLiteral("a package signed with another key than its certificate names, or changed after signing, is refused as changed"));
    }

    // the launcher: an installed program update only with the release key itself
    {
        const QString dir = W + QStringLiteral("/core/9.9.9");
        const QByteArray program = "#!/bin/sh\necho program\n";
        const QByteArray sha = QCryptographicHash::hash(program, QCryptographicHash::Sha256).toHex();
        writeAll(dir + QStringLiteral("/quillbinder"), program);
        const QByteArray manifest = QStringLiteral(R"({"format": 1, "id": "core", "kind": "core", "version": "9.9.9", "files": [{"path": "quillbinder", "sha256": "%1", "size": %2}]})")
                                        .arg(QString::fromLatin1(sha))
                                        .arg(program.size())
                                        .toUtf8();
        writeAll(dir + QStringLiteral("/package-manifest.json"), manifest);
        auto sigBy = [&](const QString &key, const QByteArray &pub, bool withCert) {
            QJsonObject s;
            s.insert(QStringLiteral("algorithm"), QStringLiteral("ed25519"));
            s.insert(QStringLiteral("key"), signing::keyId(pub));
            s.insert(QStringLiteral("signature"), QString::fromLatin1(signRaw(key, manifest, W).toBase64()));
            s.insert(QStringLiteral("signs"), QStringLiteral("manifest.json"));
            if (withCert)
                s.insert(QStringLiteral("certificate"), c);
            return QJsonDocument(s).toJson();
        };
        QString why;
        writeAll(dir + QStringLiteral("/package-manifest.sig"), sigBy(rel, relPub, false));
        const bool byRelease = launcher::coreOrigin(dir, QStringLiteral("quillbinder"), sha, &why);
        writeAll(dir + QStringLiteral("/package-manifest.sig"), sigBy(addonKey, addonPub, true));
        const bool byAddonKey = launcher::coreOrigin(dir, QStringLiteral("quillbinder"), sha, &why);
        check(byRelease && !byAddonKey,
              QStringLiteral("the launcher starts an installed program update signed with the release key, not one signed with an add-on key"),
              why);
    }

    // make-package.py verify, the check of the release workflow's signing job
    {
        int good = 0, bad = 0;
        const QString o1 = run(py, {QStringLiteral("-I"), tool, QStringLiteral("verify"), QStringLiteral("--release-key"),
                                    W + QStringLiteral("/release.pub.pem"), viaAddon, viaRelease, core, addonsBundle, mixedBundle},
                               &good);
        const QString o2 = run(py, {QStringLiteral("-I"), tool, QStringLiteral("verify"), QStringLiteral("--release-key"),
                                    W + QStringLiteral("/release.pub.pem"), viaFake, coreViaAddon},
                               &bad);
        check(good == 0 && o1.count(QLatin1String("signed:")) == 8 && o1.contains(QLatin1String("the add-on key 'Quillbinder official add-ons'"))
                  && bad == 1 && o2.count(QLatin1String("NOT TRUSTED")) == 2,
              QStringLiteral("make-package.py verify follows the program's rule (add-on keys only for add-ons, only with the release key's certificate)"),
              o1 + o2);
    }
    qunsetenv("QB_TEST_SIGNING_KEY");

    // the release workflow: the add-on key reaches only the signing job, which signs program updates with the release
    // key and checks every signature by the program's rule before the packages are handed on
    {
        const QString wf = QString::fromUtf8(readAll(src + QStringLiteral("/.github/workflows/release.yml"))).remove(QLatin1Char('\r'));
        const qsizetype a = wf.indexOf(QLatin1String("\n  sign-packages:\n")), b = wf.indexOf(QLatin1String("\n  assemble:\n"));
        const QString job = a >= 0 && b > a ? wf.mid(a, b - a) : QString();
        const QString rest = wf.left(qMax<qsizetype>(a, 0)) + wf.mid(qMax<qsizetype>(b, 0));
        check(!job.isEmpty() && job.contains(QLatin1String("secrets.QB_ADDON_KEY")) && !rest.contains(QLatin1String("QB_ADDON_KEY"))
                  && job.contains(QLatin1String("make-package.py certify")) && job.contains(QLatin1String("make-package.py verify --release-key"))
                  && job.indexOf(QLatin1String("make-package.py verify")) > job.indexOf(QLatin1String("make-package.py sign ")),
              QStringLiteral("release workflow: the add-on key (secret QB_ADDON_KEY) reaches only the signing job, which certifies it, "
                             "signs and then checks every package by the program's rule"));
    }

    // the official add-ons' workflow and its tool (tools/addon-entry.py, .github/workflows/addon-release.yml)
    {
        const QString entryTool = src + QStringLiteral("/tools/addon-entry.py");
        int rc1 = 0;
        const QString real = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"),
                                      src + QStringLiteral("/official-addons/translation-proofreading.json")},
                                 &rc1);
        const QJsonObject realEntry =
            QJsonDocument::fromJson(readAll(src + QStringLiteral("/official-addons/translation-proofreading.json"))).object();
        check(rc1 == 0
                  && real.contains(QStringLiteral("tag=addon-translation-proofreading-%1").arg(realEntry.value(QStringLiteral("version")).toString()))
                  && real.contains(QStringLiteral("commit=") + realEntry.value(QStringLiteral("commit")).toString())
                  && realEntry.value(QStringLiteral("commit")).toString().size() == 40,
              QStringLiteral("the pinned entry of Translation and Proofreading is complete; its release tag does not start with v (the "
                             "program's releases and the download page keep to those)"),
              real);

        // an add-on of another author, in a Git repository of its own, with a library and a template
        const QString repo = W + QStringLiteral("/author");
        writeAll(repo + QStringLiteral("/stage/manifest.json"),
                 R"({"id": "check-official", "name": "Check Official", "version": "2.1", "description": "For the checks.", "minApp": "0.80.0", "whatsNew": ["First version"]})");
        writeAll(repo + QStringLiteral("/stage/payload/plugins/libcheck.so"), "not really a library");
        auto git = [&](const QStringList &args) { return run(QStringLiteral("git"), QStringList{QStringLiteral("-C"), repo} + args).trimmed(); };
        git({QStringLiteral("init"), QStringLiteral("-q")});
        git({QStringLiteral("-c"), QStringLiteral("user.name=c"), QStringLiteral("-c"), QStringLiteral("user.email=c@c"), QStringLiteral("add"), QStringLiteral(".")});
        git({QStringLiteral("-c"), QStringLiteral("user.name=c"), QStringLiteral("-c"), QStringLiteral("user.email=c@c"), QStringLiteral("commit"),
             QStringLiteral("-qm"), QStringLiteral("one")});
        const QString commit = git({QStringLiteral("rev-parse"), QStringLiteral("HEAD")});
        QJsonObject entry{{QStringLiteral("format"), 1},
                          {QStringLiteral("id"), QStringLiteral("check-official")},
                          {QStringLiteral("name"), QStringLiteral("Check Official")},
                          {QStringLiteral("version"), QStringLiteral("2.1")},
                          {QStringLiteral("repository"), QStringLiteral("someone/check-official")},
                          {QStringLiteral("commit"), commit},
                          {QStringLiteral("author"), QStringLiteral("Someone")},
                          {QStringLiteral("license"), QStringLiteral("MIT License")},
                          {QStringLiteral("reviewed"), QStringLiteral("2026-11-15")},
                          {QStringLiteral("summary"), QStringLiteral("Checks things.")},
                          {QStringLiteral("nonCommercialData"),
                           QJsonArray{QJsonObject{{QStringLiteral("name"), QStringLiteral("Model X")}, {QStringLiteral("license"), QStringLiteral("CC BY-NC 4.0")}}}},
                          {QStringLiteral("build"),
                           QJsonObject{{QStringLiteral("steps"), QJsonArray{QStringLiteral("echo built")}},
                                       {QStringLiteral("dependencySources"),
                                        QJsonArray{QJsonObject{{QStringLiteral("name"), QStringLiteral("Itself")}, {QStringLiteral("dir"), QStringLiteral(".")}}}}}}};
        const QString entries = W + QStringLiteral("/entries");
        auto writeEntry = [&](const QString &file, const QJsonObject &e) {
            writeAll(entries + QLatin1Char('/') + file, QJsonDocument(e).toJson());
            return entries + QLatin1Char('/') + file;
        };
        const QString good = writeEntry(QStringLiteral("check-official.json"), entry);
        QStringList refused;
        QJsonObject bad = entry;
        bad.insert(QStringLiteral("commit"), commit.left(12));
        refused << writeEntry(QStringLiteral("check-official.json.short"), bad);
        bad = entry;
        QJsonObject b = bad.value(QStringLiteral("build")).toObject();
        b.insert(QStringLiteral("cachePaths"), QJsonArray{QStringLiteral("../outside")});
        bad.insert(QStringLiteral("build"), b);
        refused << writeEntry(QStringLiteral("dotdot/check-official.json"), bad);
        refused << writeEntry(QStringLiteral("another-name.json"), entry);
        int okRc = 0;
        run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), good}, &okRc);
        QString why;
        bool allRefused = true;
        for (const QString &f : refused) {
            int r = 0;
            why += run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), f}, &r);
            allRefused = allRefused && r == 2;
        }
        check(okRc == 0 && allRefused,
              QStringLiteral("an entry is refused with a short commit id, a folder outside the source, or a file not named after its id"), why);

        // test data from several releases (1.1 of the translation add-on): its own cache key; a bad pattern is refused
        {
            QJsonObject e2 = entry;
            QJsonObject b2 = e2.value(QStringLiteral("build")).toObject();
            b2.insert(QStringLiteral("testData"),
                      QJsonObject{{QStringLiteral("variable"), QStringLiteral("TP_MODELS_DIR")},
                                  {QStringLiteral("downloads"),
                                   QJsonArray{QJsonObject{{QStringLiteral("release"), QStringLiteral("models-1")}},
                                              QJsonObject{{QStringLiteral("release"), QStringLiteral("models-2")},
                                                          {QStringLiteral("patterns"), QJsonArray{QStringLiteral("qwen3-1.7b--*")}}}}}});
            e2.insert(QStringLiteral("build"), b2);
            int r1 = 0, r2 = 0;
            const QString td = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("testdata"),
                                        writeEntry(QStringLiteral("td/check-official.json"), e2)},
                                   &r1);
            QJsonObject b3 = b2;
            QJsonObject t3 = b3.value(QStringLiteral("testData")).toObject();
            t3.insert(QStringLiteral("downloads"), QJsonArray{QJsonObject{{QStringLiteral("release"), QStringLiteral("models-2")},
                                                                          {QStringLiteral("patterns"), QJsonArray{QStringLiteral("../x")}}}});
            b3.insert(QStringLiteral("testData"), t3);
            QJsonObject e3 = e2;
            e3.insert(QStringLiteral("build"), b3);
            run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), writeEntry(QStringLiteral("td2/check-official.json"), e3)}, &r2);
            check(r1 == 0 && td.contains(QLatin1String("any=true")) && td.contains(QLatin1String("key=addon-check-official-testdata-"))
                      && td.contains(QLatin1String("variable=TP_MODELS_DIR")) && r2 == 2,
                  QStringLiteral("addon-entry.py testdata: test data from several releases, some files of one; a pattern with a path is refused"),
                  td);
        }

        // the package as the add-on's commands leave it: checked against the entry and the program's runtime
        const QString pkg = W + QStringLiteral("/check-official-2.1.qbpkg");
        make({QStringLiteral("addon"), repo + QStringLiteral("/stage"), pkg});
        int fits = 0, wrong = 0, signedRc = 0;
        const QString o1 = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check-package"), good, pkg}, &fits);
        QJsonObject other = entry;
        other.insert(QStringLiteral("version"), QStringLiteral("2.2"));
        const QString o2 = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check-package"),
                                    writeEntry(QStringLiteral("v22/check-official.json"), other), pkg},
                               &wrong);
        const QString signedPkg = W + QStringLiteral("/check-official-signed.qbpkg");
        QFile::copy(pkg, signedPkg);
        run(py, {QStringLiteral("-I"), tool, QStringLiteral("sign"), signedPkg}, nullptr, withRel);
        const QString o3 = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check-package"), good, signedPkg}, &signedRc);
#ifdef Q_OS_WIN
        // (Windows 0.91.3: official add-ons are built on Ubuntu for the AppImage, so the tool names the AppImage's runtime,
        // the first APP_RUNTIME of CMakeLists.txt, not this program's)
        QFile cm(src + QStringLiteral("/CMakeLists.txt"));
        const QString cmText = cm.open(QIODevice::ReadOnly) ? QString::fromUtf8(cm.readAll()) : QString();
        const QRegularExpressionMatch rm = QRegularExpression(QStringLiteral("^set\\(APP_RUNTIME\\s+\"([^\"]+)\"\\)"),
                                                              QRegularExpression::MultilineOption).match(cmText);
        check(fits == 0 && rm.hasMatch() && o1.contains(rm.captured(1)) && wrong == 1 && o2.contains(QLatin1String("version"))
                  && signedRc == 1 && o3.contains(QLatin1String("signed already")),
#else
        check(fits == 0 && o1.contains(QLatin1String(QB_RUNTIME_ID)) && wrong == 1 && o2.contains(QLatin1String("version"))
                  && signedRc == 1 && o3.contains(QLatin1String("signed already")),
#endif
              QStringLiteral("addon-entry.py check-package: the built package fits its entry and the AppImage's runtime; another version, "
                             "or a package signed before the signing job, is refused"),
              o1 + o2 + o3);

        // sources and the release text
        const QString files = W + QStringLiteral("/files");
        QDir().mkpath(files);
        QFile::copy(pkg, files + QStringLiteral("/check-official-2.1.qbpkg"));
        int srcRc = 0, notesRc = 0;
        const QString o4 = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("sources"), good, repo, files}, &srcRc);
        const QString sourcesTxt = QString::fromUtf8(readAll(files + QStringLiteral("/SOURCES.txt")));
        run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("notes"), good, pkg, files, W + QStringLiteral("/notes.md")}, &notesRc);
        const QString notes = QString::fromUtf8(readAll(W + QStringLiteral("/notes.md")));
        check(srcRc == 0 && QFileInfo::exists(files + QStringLiteral("/check-official-2.1-source.tar.gz")) && sourcesTxt.contains(commit)
                  && sourcesTxt.contains(QLatin1String("qt-everywhere-src-")),
              QStringLiteral("addon-entry.py sources: the add-on's source at the reviewed commit, the commits of what was built from source, "
                             "where Qt's source is"),
              o4 + sourcesTxt);
        check(notesRc == 0
                  && notes.contains(QLatin1String("This add-on is free and must not be sold. It can download Model X, whose license "
                                                  "(CC BY-NC 4.0) allows non-commercial use only."))
                  && notes.contains(QLatin1String("Linux 0.91.3 or later")) && notes.contains(QLatin1String("0.80.0 to 0.91.2"))
                  && notes.contains(QLatin1String("`SHA256SUMS.txt`")) && notes.contains(QLatin1String("not made by or connected with Literature & Latte"))
                  && notes.contains(commit.left(12)),
              QStringLiteral("the release text: free and not to be sold, the data for non-commercial use only by name and license, which "
                             "Quillbinder it needs, the reviewed commit, not connected with Literature & Latte"),
              notes);

        // the workflow: the keys only in the signing job, write permission only in the publishing job, both only on request
        const QString wf = QString::fromUtf8(readAll(src + QStringLiteral("/.github/workflows/addon-release.yml"))).remove(QLatin1Char('\r'));
        const QStringList jobs = {QStringLiteral("prepare"), QStringLiteral("build"), QStringLiteral("sign"), QStringLiteral("check-signed"),
                                  QStringLiteral("publish")};
        QMap<QString, QString> body;
        for (int i = 0; i < jobs.size(); ++i) {
            const qsizetype a = wf.indexOf(QStringLiteral("\n  ") + jobs.at(i) + QStringLiteral(":\n"));
            const qsizetype e = i + 1 < jobs.size() ? wf.indexOf(QStringLiteral("\n  ") + jobs.at(i + 1) + QStringLiteral(":\n")) : wf.size();
            body.insert(jobs.at(i), a >= 0 && e > a ? wf.mid(a, e - a) : QString());
        }
        bool shape = true;
        for (const QString &j : jobs) {
            shape = shape && !body.value(j).isEmpty();
            shape = shape && body.value(j).contains(QLatin1String("secrets.")) == (j == QLatin1String("sign"));
            shape = shape && body.value(j).contains(QLatin1String("contents: write")) == (j == QLatin1String("publish"));
        }
        const QString sign = body.value(QStringLiteral("sign"));
        check(shape && sign.contains(QLatin1String("if: inputs.publish")) && !sign.contains(QLatin1String("container:"))
                  && !sign.contains(QLatin1String("cloud-setup")) && !sign.contains(QLatin1String("addon-entry.py run"))
                  && body.value(QStringLiteral("build")).contains(QLatin1String("addon-entry.py run"))
                  && body.value(QStringLiteral("publish")).contains(QLatin1String("--latest=false"))
                  && wf.contains(QLatin1String("github.repository == 'Shelin-boop/Quillbinder_Collection'")),
              QStringLiteral("workflow Official add-on: the author's commands run only in the build job, which has no secret; the keys only "
                             "in the signing job (on request, no container); published only on request, never as Latest"));

        // Windows (2026-11-16): the entry's "windows" block (its own reviewed commit and build) and the workflow "Official
        // add-on for Windows" with the same rules
        {
            QJsonObject we = entry;
            we.insert(QStringLiteral("windows"),
                      QJsonObject{{QStringLiteral("commit"), commit},
                                  {QStringLiteral("build"),
                                   QJsonObject{{QStringLiteral("steps"), QJsonArray{QStringLiteral("echo built")}},
                                               {QStringLiteral("redistributables"),
                                                QJsonArray{QJsonObject{{QStringLiteral("file"), QStringLiteral("stage/manifest.json")},
                                                                       {QStringLiteral("name"), QStringLiteral("A file")}}}}}}});
            const QString wEntry = writeEntry(QStringLiteral("win/check-official.json"), we);
            int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
            const QString linuxView = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), wEntry}, &r1);
            const QString winView = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), wEntry, QStringLiteral("--windows")}, &r2);
            const QString noWin = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), good, QStringLiteral("--windows")}, &r3);
            const QString winStage = W + QStringLiteral("/winstage");
            writeAll(winStage + QStringLiteral("/manifest.json"),
                     R"({"id": "check-official", "name": "Check Official", "version": "2.1", "description": "For the checks."})");
            writeAll(winStage + QStringLiteral("/payload/plugins/check.dll"), "not really a library");
            const QString winPkg = W + QStringLiteral("/check-official-2.1-windows.qbpkg");
            const QString made = make({QStringLiteral("addon"), QStringLiteral("--platform"), QStringLiteral("windows-x64"), winStage, winPkg});
            const QString o1 = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check-package"), wEntry, winPkg, QStringLiteral("--windows")}, &r4);
            const QString o2 = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check-package"), wEntry, pkg, QStringLiteral("--windows")}, &r5);
            check(r1 == 0 && linuxView.contains(QLatin1String("tag=addon-check-official-2.1")) && !linuxView.contains(QLatin1String("-windows"))
                      && r2 == 0 && winView.contains(QLatin1String("tag=addon-check-official-2.1-windows"))
                      && winView.contains(QStringLiteral("commit=") + commit) && r3 == 2 && noWin.contains(QLatin1String("no Windows build"))
                      && made.isEmpty() && r4 == 0 && o1.contains(QLatin1String("platform windows-x64")) && r5 == 1
                      && o2.contains(QLatin1String("platform")),
                  QStringLiteral("addon-entry.py --windows: the entry's Windows block (its tag ends in -windows), refused without one; a "
                                 "Windows package must name windows-x64, a Linux one is refused for Windows"),
                  linuxView + winView + noWin + made + o1 + o2);

            const QString wfw = QString::fromUtf8(readAll(src + QStringLiteral("/.github/workflows/addon-release-windows.yml"))).remove(QLatin1Char('\r'));
            QMap<QString, QString> wb;
            for (int i = 0; i < jobs.size(); ++i) {
                const qsizetype a = wfw.indexOf(QStringLiteral("\n  ") + jobs.at(i) + QStringLiteral(":\n"));
                const qsizetype e = i + 1 < jobs.size() ? wfw.indexOf(QStringLiteral("\n  ") + jobs.at(i + 1) + QStringLiteral(":\n")) : wfw.size();
                wb.insert(jobs.at(i), a >= 0 && e > a ? wfw.mid(a, e - a) : QString());
            }
            bool wshape = true;
            for (const QString &j : jobs) {
                wshape = wshape && !wb.value(j).isEmpty();
                wshape = wshape && wb.value(j).contains(QLatin1String("secrets.")) == (j == QLatin1String("sign"));
                wshape = wshape && wb.value(j).contains(QLatin1String("contents: write")) == (j == QLatin1String("publish"));
            }
            const QString wsign = wb.value(QStringLiteral("sign"));
            check(wshape && wsign.contains(QLatin1String("if: inputs.publish")) && !wsign.contains(QLatin1String("container:"))
                      && !wsign.contains(QLatin1String("addon-entry.py run"))
                      && wb.value(QStringLiteral("build")).contains(QLatin1String("addon-entry.py run"))
                      && wb.value(QStringLiteral("build")).contains(QLatin1String("--windows"))
                      && wb.value(QStringLiteral("build")).contains(QLatin1String("runs-on: windows-"))
                      && wb.value(QStringLiteral("publish")).contains(QLatin1String("--latest=false"))
                      && wfw.contains(QLatin1String("github.repository == 'Shelin-boop/Quillbinder_Collection'")),
                  QStringLiteral("workflow Official add-on for Windows: the same rules (the author's commands only in the Windows build job, "
                                 "without secrets; the keys only in the signing job, on request; never Latest)"));
        }

        // Android (2026-11-16): the entry's "android" block and the workflow "Official add-on for Android": a package for
        // the tablets (android-arm64, the Android runtime, 0.91.3 or later) whose libraries need only what the app brings
        {
            QJsonObject ae = entry;
            ae.insert(QStringLiteral("android"),
                      QJsonObject{{QStringLiteral("commit"), commit},
                                  {QStringLiteral("reviewed"), QStringLiteral("2026-11-16")},
                                  {QStringLiteral("build"),
                                   QJsonObject{{QStringLiteral("steps"), QJsonArray{QStringLiteral("echo built")}},
                                               {QStringLiteral("dependencySources"),
                                                QJsonArray{QJsonObject{{QStringLiteral("name"), QStringLiteral("Itself")},
                                                                       {QStringLiteral("dir"), QStringLiteral(".")}}}},
                                               {QStringLiteral("toolchainLibraries"),
                                                QJsonArray{QJsonObject{{QStringLiteral("name"), QStringLiteral("A runtime of the toolchain")},
                                                                       {QStringLiteral("source"), QStringLiteral("https://example.org/toolchain")}}}}}}});
            const QString aEntry = writeEntry(QStringLiteral("android/check-official.json"), ae);
            QJsonObject wrong = ae;
            QJsonObject wa = wrong.value(QStringLiteral("android")).toObject();
            QJsonObject wab = wa.value(QStringLiteral("build")).toObject();
            wab.insert(QStringLiteral("redistributables"),
                       QJsonArray{QJsonObject{{QStringLiteral("file"), QStringLiteral("stage/manifest.json")}, {QStringLiteral("name"), QStringLiteral("A DLL")}}});
            wa.insert(QStringLiteral("build"), wab);
            wrong.insert(QStringLiteral("android"), wa);
            const QString wrongEntry = writeEntry(QStringLiteral("android-wrong/check-official.json"), wrong);
            int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
            const QString aView = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), aEntry, QStringLiteral("--android")}, &r1);
            const QString noAndroid = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), good, QStringLiteral("--android")}, &r2);
            const QString wrongView = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"), wrongEntry, QStringLiteral("--android")}, &r3);
            const QString realAndroid = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check"),
                                                 src + QStringLiteral("/official-addons/translation-proofreading.json"), QStringLiteral("--android")}, &r4);
            check(r1 == 0 && aView.contains(QLatin1String("tag=addon-check-official-2.1-android")) && r2 == 2
                      && noAndroid.contains(QLatin1String("no Android build")) && r3 == 2 && wrongView.contains(QLatin1String("redistributables"))
                      && r4 == 0 && realAndroid.contains(QLatin1String("tag=addon-translation-proofreading-1.1-android")),
                  QStringLiteral("addon-entry.py --android: the entry's Android block (its tag ends in -android), refused without one or with "
                                 "Windows' redistributables; the pinned translation add-on has one"),
                  aView + noAndroid + wrongView + realAndroid);

            // the packages: made like the add-on's own script makes them (platform android-arm64, the Android runtime)
            const QString aStage = W + QStringLiteral("/androidstage");
            writeAll(aStage + QStringLiteral("/manifest.json"),
                     R"({"id": "check-official", "name": "Check Official", "version": "2.1", "description": "For the checks.", "minApp": "0.91.3"})");
            writeAll(aStage + QStringLiteral("/payload/plugins/libcheck.so"), "not really a library");
            const QString linuxRuntime = W + QStringLiteral("/check-official-2.1-android-linuxruntime.qbpkg");
            const QString aPkg = W + QStringLiteral("/check-official-2.1-android-arm64.qbpkg");
            const QString made = make({QStringLiteral("addon"), QStringLiteral("--platform"), QStringLiteral("android-arm64"), aStage, linuxRuntime});
            const QString androidRuntime = [&] {
                const QString c = QString::fromUtf8(readAll(src + QStringLiteral("/CMakeLists.txt")));
                const QRegularExpressionMatch m = QRegularExpression(QStringLiteral(R"re(set\(APP_RUNTIME\s+"(android-[^"]*)"\))re")).match(c);
                return m.hasMatch() ? m.captured(1) : QString();
            }();
            int rz = 0;
            const QString rewritten = run(py, {QStringLiteral("-I"), QStringLiteral("-c"),
                                               QStringLiteral("import sys, zipfile, json\n"
                                                              "src, dst, rt = sys.argv[1:4]\n"
                                                              "with zipfile.ZipFile(src) as z, zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as o:\n"
                                                              "    for i in z.infolist():\n"
                                                              "        d = z.read(i.filename)\n"
                                                              "        if i.filename == 'manifest.json':\n"
                                                              "            m = json.loads(d); m['runtime'] = rt; d = json.dumps(m, indent=2).encode()\n"
                                                              "        o.writestr(i, d)\n"),
                                               linuxRuntime, aPkg, androidRuntime}, &rz);
            // a stand-in for the NDK's llvm-readelf: an arm64 library that needs Android's and Qt's libraries, 16 KB pages
            // (FAKE_ELF=bad: one that needs a Linux library and has 4 KB pages)
            const QString fakeReadelf = W + QStringLiteral("/fake-readelf");
            writeAll(fakeReadelf,
                     "#!/bin/sh\n"
                     "case \"$1\" in\n"
                     "  -d) [ \"$FAKE_ELF\" = bad ] && echo ' 0x0000000000000001 (NEEDED)             Shared library: [libstdc++.so.6]'\n"
                     "      echo ' 0x0000000000000001 (NEEDED)             Shared library: [libc.so]'\n"
                     "      echo ' 0x0000000000000001 (NEEDED)             Shared library: [libQt6Core_arm64-v8a.so]' ;;\n"
                     "  -h) echo '  Machine:                           AArch64' ;;\n"
                     "  -lW) a=0x4000; [ \"$FAKE_ELF\" = bad ] && a=0x1000\n"
                     "      echo \"  LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x001000 0x001000 R   $a\" ;;\n"
                     "esac\n"
                     "exit 0\n");
            QFile::setPermissions(fakeReadelf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
            QProcessEnvironment withReadelf = QProcessEnvironment::systemEnvironment();
            withReadelf.insert(QStringLiteral("READELF"), fakeReadelf);
            QProcessEnvironment badElf = withReadelf;
            badElf.insert(QStringLiteral("FAKE_ELF"), QStringLiteral("bad"));
            QProcessEnvironment noReadelf = QProcessEnvironment::systemEnvironment();
            noReadelf.remove(QStringLiteral("READELF"));
            int p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0;
            auto checkPkg = [&](const QString &package, int *rc, const QProcessEnvironment &env) {
                return run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("check-package"), aEntry, package, QStringLiteral("--android")}, rc, env);
            };
            const QString fits = checkPkg(aPkg, &p1, withReadelf);
            const QString badLibs = checkPkg(aPkg, &p2, badElf);
            const QString unchecked = checkPkg(aPkg, &p3, noReadelf);
            const QString wrongRuntime = checkPkg(linuxRuntime, &p4, withReadelf);
            const QString windowsPkg = checkPkg(W + QStringLiteral("/check-official-2.1-windows.qbpkg"), &p5, withReadelf);
            const QString linuxPkg = checkPkg(pkg, &p6, withReadelf);
            check(made.isEmpty() && rz == 0 && androidRuntime.startsWith(QLatin1String("android-")) && p1 == 0
                      && fits.contains(QLatin1String("platform android-arm64")) && p2 == 1 && badLibs.contains(QLatin1String("libstdc++.so.6"))
                      && badLibs.contains(QLatin1String("16 KB")) && p3 == 1 && unchecked.contains(QLatin1String("READELF"))
                      && p4 == 1 && wrongRuntime.contains(QLatin1String("runtime")) && p5 == 1 && windowsPkg.contains(QLatin1String("platform"))
                      && p6 == 1 && linuxPkg.contains(QLatin1String("platform")),
                  QStringLiteral("addon-entry.py check-package --android: a tablet package fits (android-arm64, the Android runtime, its "
                                 "libraries' needs and 16 KB pages); refused: a library that needs a Linux library or has 4 KB pages, no "
                                 "readelf to look with, the Linux runtime, a Windows or a Linux package"),
                  made + rewritten + fits + badLibs + unchecked + wrongRuntime + windowsPkg + linuxPkg);

            // sources and release text for Android
            const QString aFiles = W + QStringLiteral("/android-files");
            int s1 = 0, s2 = 0;
            const QString srcOut = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("sources"), aEntry, repo, aFiles, QStringLiteral("--android")}, &s1);
            const QString aNotes = W + QStringLiteral("/android-notes.md");
            const QString notesOut = run(py, {QStringLiteral("-I"), entryTool, QStringLiteral("notes"), aEntry, aPkg, aFiles, aNotes,
                                              QStringLiteral("--android")}, &s2);
            const QString sourcesTxt = QString::fromUtf8(readAll(aFiles + QStringLiteral("/SOURCES.txt")));
            const QString aText = QString::fromUtf8(readAll(aNotes));
            check(s1 == 0 && QFile::exists(aFiles + QStringLiteral("/check-official-2.1-android-source.tar.gz"))
                      && sourcesTxt.contains(QLatin1String("A runtime of the toolchain")) && sourcesTxt.contains(QLatin1String("Quillbinder for Android brings Qt"))
                      && s2 == 0 && aText.contains(QLatin1String("Check Official 2.1 for Android")) && aText.contains(QLatin1String("File > Install Update or Add-on"))
                      && aText.contains(QLatin1String("Quillbinder for Android 0.91.3")),
                  QStringLiteral("addon-entry.py sources/notes --android: the Android source archive, the toolchain's libraries and where Qt "
                                 "comes from in SOURCES.txt; the release text tells how to install on the tablet"),
                  srcOut + notesOut + sourcesTxt.left(600) + aText.left(600));

            // the workflow: the same rules (no install check: Quillbinder for Android runs only on a tablet)
            const QString wfa = QString::fromUtf8(readAll(src + QStringLiteral("/.github/workflows/addon-release-android.yml"))).remove(QLatin1Char('\r'));
            const QStringList ajobs = {QStringLiteral("prepare"), QStringLiteral("build"), QStringLiteral("sign"), QStringLiteral("publish")};
            QMap<QString, QString> ab;
            for (int i = 0; i < ajobs.size(); ++i) {
                const qsizetype a = wfa.indexOf(QStringLiteral("\n  ") + ajobs.at(i) + QStringLiteral(":\n"));
                const qsizetype e = i + 1 < ajobs.size() ? wfa.indexOf(QStringLiteral("\n  ") + ajobs.at(i + 1) + QStringLiteral(":\n")) : wfa.size();
                ab.insert(ajobs.at(i), a >= 0 && e > a ? wfa.mid(a, e - a) : QString());
            }
            bool ashape = true;
            for (const QString &j : ajobs) {
                ashape = ashape && !ab.value(j).isEmpty();
                ashape = ashape && ab.value(j).contains(QLatin1String("secrets.")) == (j == QLatin1String("sign"));
                ashape = ashape && ab.value(j).contains(QLatin1String("contents: write")) == (j == QLatin1String("publish"));
            }
            const QString asign = ab.value(QStringLiteral("sign"));
            const QString abuild = ab.value(QStringLiteral("build"));
            check(ashape && asign.contains(QLatin1String("if: inputs.publish")) && !asign.contains(QLatin1String("container:"))
                      && !asign.contains(QLatin1String("addon-entry.py run")) && abuild.contains(QLatin1String("addon-entry.py run"))
                      && abuild.contains(QLatin1String("--android")) && abuild.contains(QLatin1String("READELF="))
                      && abuild.contains(QLatin1String("android_arm64_v8a"))
                      && abuild.contains(QLatin1String("set +o pipefail"))
                      && ab.value(QStringLiteral("publish")).contains(QLatin1String("--latest=false"))
                      && ab.value(QStringLiteral("publish")).contains(QLatin1String("for Android (official add-on)"))
                      && ab.value(QStringLiteral("prepare")).contains(QLatin1String("-android-"))
                      && wfa.contains(QLatin1String("github.repository == 'Shelin-boop/Quillbinder_Collection'")),
                  QStringLiteral("workflow Official add-on for Android: the same rules (the author's commands only in the build job, without "
                                 "secrets; NDK and Qt for Android arm64; sdkmanager without pipefail; the keys only in the signing job, on "
                                 "request; never Latest)"));
        }
    }

    std::printf("\n%d failure(s)\n", g_failures);
    return g_failures == 0 ? 0 : 1;
}
