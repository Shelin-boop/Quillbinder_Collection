// 0.99.0, to-do item 113 (part 1): the OpenPGP layer of the built-in encryption (src/core/crypto): keys, the key ring,
// round trips, wrong passphrases, damaged files, foreign keys, the passphrase generator and its strength check, the key
// store (in memory; QtKeychain without a key store; QtKeychain with a real Secret Service where gnome-keyring is
// installed), what the program links, and the interoperability with GnuPG both ways (each gpg on the PATH, and those
// QB_TEST_GPG_EXTRA names; skipped with a message where there is none).
//
//   xvfb-run -a ./qb_v0990e_tests /tmp/shots        (no window: the folder is not used)
// Exit code = number of failed checks.
#include "core/crypto/KeyStore.h"
#include "core/crypto/OpenPgp.h"
#include "core/crypto/Passphrase.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QThread>
#include <cstdio>
#include <cstring>

using namespace qb;
using namespace qb::crypto;

static int failures = 0;
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

static void info(const QString &s)
{
    std::printf("      %s\n", qPrintable(s));
    std::fflush(stdout);
}

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

static bool writeAll(const QString &path, const QByteArray &data)
{
    QFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(data) == data.size();
}

// the packets of a binary OpenPGP message: (tag, first byte of the body)
static QList<QPair<int, int>> packets(const QByteArray &data)
{
    QList<QPair<int, int>> out;
    const auto *p = reinterpret_cast<const unsigned char *>(data.constData());
    qsizetype i = 0, n = data.size();
    while (i < n) {
        const unsigned char b = p[i++];
        if (!(b & 0x80) || !(b & 0x40) || i >= n)
            break; // (RNP writes the new format only)
        const int tag = b & 0x3f;
        qsizetype len = 0;
        const unsigned char l0 = p[i++];
        bool partial = false;
        if (l0 < 192)
            len = l0;
        else if (l0 < 224 && i < n)
            len = ((l0 - 192) << 8) + p[i++] + 192;
        else if (l0 == 255 && i + 4 <= n) {
            len = (qsizetype(p[i]) << 24) | (p[i + 1] << 16) | (p[i + 2] << 8) | p[i + 3];
            i += 4;
        } else {
            partial = true; // a partial length: the rest is this packet
        }
        out.append({tag, i < n ? p[i] : -1});
        if (partial)
            break;
        i += len;
    }
    return out;
}

// a key packet made by hand (no library writes these here): version, algorithm and some bytes of key material
static QByteArray handMadeKey(int version, int algorithm)
{
    QByteArray body;
    body.append(char(version));
    body.append("\x5f\x00\x00\x00", 4); // creation time
    body.append(char(algorithm));
    if (version >= 5)
        body.append("\x00\x00\x00\x20", 4);
    body.append(QByteArray(32, '\x42'));
    QByteArray packet;
    packet.append(char(0xC0 | 6)); // new format, public key
    packet.append(char(body.size()));
    return packet + body;
}

struct Run {
    int code = -1;
    QByteArray out, err;
};

static Run run(const QString &program, const QStringList &args, const QProcessEnvironment &env,
               const QByteArray &input = QByteArray(), int timeoutMs = 120000)
{
    QProcess p;
    p.setProcessEnvironment(env);
    p.start(program, args);
    Run r;
    if (!p.waitForStarted(20000)) {
        r.err = p.errorString().toUtf8();
        return r;
    }
    if (!input.isEmpty())
        p.write(input);
    p.closeWriteChannel();
    if (!p.waitForFinished(timeoutMs)) {
        p.kill();
        p.waitForFinished(5000);
        r.err = "timed out";
        return r;
    }
    r.code = p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -2;
    r.out = p.readAllStandardOutput();
    r.err = p.readAllStandardError();
    return r;
}

static const QByteArray kWriterPass = "lantern pebble orchard velvet compass ember";
static const QByteArray kRecoveryPass = "harbor tundra marble quiver saddle widow";

// --- the child runs of the key store -------------------------------------------------------------------------------

// QtKeychain with a real Secret Service (gnome-keyring under a D-Bus session of its own, started by the parent)
static int childSecretService()
{
    KeyStore &store = KeyStore::system();
    const QString entry = KeyStore::passphraseEntry(QStringLiteral("0123456789ABCDEF0123456789ABCDEF01234567"));
    const QByteArray secret = QStringLiteral("küsten-lied orbit 42").toUtf8();
    QString err;
    const KeyStore::Status w = store.write(entry, secret, &err);
    QByteArray back;
    const KeyStore::Status r = store.read(entry, &back, &err);
    const KeyStore::Status d = store.remove(entry, &err);
    QByteArray gone;
    const KeyStore::Status r2 = store.read(entry, &gone, &err);
    std::printf("child: write %d read %d same %d remove %d read-again %d (%s)\n", int(w), int(r), back == secret, int(d),
                int(r2), qPrintable(err));
    const bool ok = w == KeyStore::Status::Ok && r == KeyStore::Status::Ok && back == secret && d == KeyStore::Status::Ok
        && r2 == KeyStore::Status::NotFound;
    std::printf(ok ? "CHILD-OK\n" : "CHILD-FAIL\n");
    return ok ? 0 : 1;
}

// QtKeychain where there is no key store (no D-Bus session): an answer, not a hang or a crash
static int childNoStore()
{
    QElapsedTimer t;
    t.start();
    QByteArray back;
    QString err;
    const KeyStore::Status s = KeyStore::system().read(KeyStore::passphraseEntry(QStringLiteral("AB")), &back, &err);
    std::printf("child: status %d after %lld ms (%s) name '%s'\n", int(s), qint64(t.elapsed()), qPrintable(err),
                qPrintable(KeyStore::system().name()));
    std::printf(s != KeyStore::Status::Ok ? "CHILD-OK\n" : "CHILD-FAIL\n");
    return 0;
}

// --- GnuPG ---------------------------------------------------------------------------------------------------------

struct Gpg {
    QString program;
    QString home;
    QProcessEnvironment env;
    Run operator()(const QStringList &args, const QByteArray &input = QByteArray()) const
    {
        return run(program, QStringList{QStringLiteral("--batch"), QStringLiteral("--no-tty"), QStringLiteral("--yes")} + args,
                   env, input);
    }
    QStringList withPass(const QByteArray &pass, const QStringList &args) const
    {
        return QStringList{QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"),
                           QString::fromUtf8(pass)} + args;
    }
    QString fingerprintOf(const QString &uid) const
    {
        const Run r = (*this)({QStringLiteral("--with-colons"), QStringLiteral("--list-keys"), uid});
        for (const QByteArray &line : r.out.split('\n'))
            if (line.startsWith("fpr:"))
                return QString::fromLatin1(line.split(':').value(9));
        return QString();
    }
    void stop() const
    {
        // the agent of this home ends (its folder goes with the test anyway)
        const QString dir = QFileInfo(program).absolutePath();
        QString gpgconf = dir + QStringLiteral("/gpgconf");
        if (!QFileInfo(gpgconf).isExecutable())
            gpgconf = dir + QStringLiteral("/bin/gpgconf");
        if (!QFileInfo(gpgconf).isExecutable())
            gpgconf = QStandardPaths::findExecutable(QStringLiteral("gpgconf"));
        if (!gpgconf.isEmpty())
            run(gpgconf, {QStringLiteral("--kill"), QStringLiteral("all")}, env, QByteArray(), 20000);
    }
};

static void interop(const QString &program, const QString &tmp, KeyRing &ring, const QString &writer, const QString &recovery)
{
    QTemporaryDir home(QStringLiteral("/tmp/qbgpg-XXXXXX"));
    QFile::setPermissions(home.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    Gpg gpg;
    gpg.program = program;
    gpg.home = home.path();
    gpg.env = QProcessEnvironment::systemEnvironment();
    gpg.env.insert(QStringLiteral("GNUPGHOME"), home.path());
    gpg.env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    const Run version = gpg({QStringLiteral("--version")});
    const QString name = QString::fromUtf8(version.out.split('\n').value(0)).trimmed();
    check(version.code == 0 && name.contains(QLatin1String("GnuPG")), QStringLiteral("[%1] %2 runs").arg(program, name),
          QString::fromUtf8(version.err));
    if (version.code != 0)
        return;
    const QString tag = QStringLiteral("[%1]").arg(name);

    // 1. RNP's keys and file opened by GnuPG
    const QString pubFile = tmp + QStringLiteral("/writer-public.asc"), secFile = tmp + QStringLiteral("/writer-secret.asc");
    writeAll(pubFile, ring.exportPublicKey(writer));
    writeAll(secFile, ring.exportSecretKey(writer));
    Run r = gpg({QStringLiteral("--import"), pubFile});
    check(r.code == 0, QStringLiteral("%1 imports Quillbinder's public key").arg(tag), QString::fromUtf8(r.err));
    r = gpg(gpg.withPass(kWriterPass, {QStringLiteral("--import"), secFile}));
    check(r.code == 0 && r.err.contains("secret keys imported: 1"),
          QStringLiteral("%1 imports Quillbinder's secret key with its passphrase").arg(tag), QString::fromUtf8(r.err));
    const QByteArray text = QStringLiteral("Kapitel 1. Die Küste lag still; « guillemets », em dash — and more.\n").toUtf8().repeated(50);
    Error e;
    const QByteArray cipher = ring.encrypt(text, {writer, recovery}, &e);
    const QString cipherFile = tmp + QStringLiteral("/by-quillbinder.gpg");
    writeAll(cipherFile, cipher);
    r = gpg(gpg.withPass(kWriterPass, {QStringLiteral("-v"), QStringLiteral("--decrypt"), cipherFile}));
    check(r.code == 0 && r.out == text, QStringLiteral("%1 decrypts a file Quillbinder encrypted").arg(tag),
          QString::fromUtf8(r.err.right(800)));
    check(r.err.contains("AES256.CFB") && !r.err.contains("OCB") && !r.err.contains("AEAD"),
          QStringLiteral("%1 names it AES-256 with CFB (SEIPD v1 with MDC), no AEAD").arg(tag), QString::fromUtf8(r.err.right(800)));
    r = gpg({QStringLiteral("--list-packets"), cipherFile});
    check(r.out.count(":pubkey enc packet: version 3, algo 18") == 2 && (r.out.contains("mdc_method: 2") || r.out.contains("mdc: 2")),
          QStringLiteral("%1 sees two Cv25519 recipients and the MDC").arg(tag), QString::fromUtf8(r.out.left(800)));
    r = gpg({QStringLiteral("--list-packets"), secFile});
    check(r.out.contains("version 4, algo 22") && r.out.contains("version 4, algo 18") && !r.out.contains("version 5")
              && r.out.count("iter+salt S2K, algo: 9") == 2 && r.out.count("hash: 8") == 2,
          QStringLiteral("%1 sees a v4 EdDSA key with a v4 ECDH subkey, both protected by iterated and salted S2K "
                         "(AES-256, SHA-256)").arg(tag), QString::fromUtf8(r.out.left(1200)));

    // 2. GnuPG's file to Quillbinder's key opened by Quillbinder
    const QString plainFile = tmp + QStringLiteral("/plain.txt"), gpgFile = tmp + QStringLiteral("/by-gnupg.gpg");
    writeAll(plainFile, text);
    r = gpg({QStringLiteral("--trust-model"), QStringLiteral("always"), QStringLiteral("-r"), writer, QStringLiteral("-o"), gpgFile,
             QStringLiteral("--encrypt"), plainFile});
    check(r.code == 0, QStringLiteral("%1 encrypts to Quillbinder's public key").arg(tag), QString::fromUtf8(r.err));
    ring.unlock(writer, kWriterPass);
    const QByteArray back = ring.decrypt(readAll(gpgFile), &e);
    check(e == Error::None && back == text, QStringLiteral("%1: Quillbinder decrypts GnuPG's file").arg(tag),
          errorText(e) + QLatin1Char(' ') + ring.errorDetail());

    // 3. a key made by GnuPG (v4, Ed25519 and Cv25519) taken into Quillbinder
    const QByteArray gpgPass = "gnupg made this key for the check";
    r = gpg(gpg.withPass(gpgPass, {QStringLiteral("--quick-gen-key"), QStringLiteral("GnuPG Writer"), QStringLiteral("ed25519"),
                                   QStringLiteral("sign"), QStringLiteral("never")}));
    const QString gf = gpg.fingerprintOf(QStringLiteral("GnuPG Writer"));
    r = gpg(gpg.withPass(gpgPass, {QStringLiteral("--quick-add-key"), gf, QStringLiteral("cv25519"), QStringLiteral("encr"),
                                   QStringLiteral("never")}));
    check(!gf.isEmpty() && r.code == 0, QStringLiteral("%1 makes a v4 Ed25519/Cv25519 key").arg(tag), QString::fromUtf8(r.err));
    const QByteArray gpgSecret = gpg(gpg.withPass(gpgPass, {QStringLiteral("--armor"), QStringLiteral("--export-secret-keys"), gf})).out;
    KeyRing other;
    const ImportReport rep = other.importKeys(gpgSecret);
    check(rep.error == Error::None && rep.imported == QStringList{gf} && rep.withSecret == QStringList{gf},
          QStringLiteral("%1: Quillbinder takes GnuPG's key with its secret part").arg(tag),
          rep.refused.isEmpty() ? QString() : rep.refused.first().second);
    check(other.unlock(gf, "not the passphrase") == Error::WrongPassphrase && other.unlock(gf, gpgPass) == Error::None,
          QStringLiteral("%1: GnuPG's key opens with its passphrase only").arg(tag));
    const QByteArray toGpgKey = other.encrypt(text, {gf}, &e);
    writeAll(tmp + QStringLiteral("/to-gnupg-key.gpg"), toGpgKey);
    r = gpg(gpg.withPass(gpgPass, {QStringLiteral("--decrypt"), tmp + QStringLiteral("/to-gnupg-key.gpg")}));
    check(e == Error::None && r.code == 0 && r.out == text,
          QStringLiteral("%1 decrypts what Quillbinder encrypted to GnuPG's key").arg(tag), QString::fromUtf8(r.err.right(600)));
    r = gpg({QStringLiteral("--trust-model"), QStringLiteral("always"), QStringLiteral("-r"), gf, QStringLiteral("-o"),
             tmp + QStringLiteral("/gnupg-to-own.gpg"), QStringLiteral("--encrypt"), plainFile});
    check(r.code == 0 && other.decrypt(readAll(tmp + QStringLiteral("/gnupg-to-own.gpg")), &e) == text && e == Error::None,
          QStringLiteral("%1: Quillbinder decrypts GnuPG's file to GnuPG's own key").arg(tag), errorText(e));
    // the same key without a passphrase: its public part is taken, its secret part is not
    r = gpg({QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"), QString(),
             QStringLiteral("--quick-gen-key"), QStringLiteral("Open Writer"), QStringLiteral("ed25519"), QStringLiteral("sign"),
             QStringLiteral("never")});
    const QString of = gpg.fingerprintOf(QStringLiteral("Open Writer"));
    gpg({QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"), QString(),
         QStringLiteral("--quick-add-key"), of, QStringLiteral("cv25519"), QStringLiteral("encr"), QStringLiteral("never")});
    const QByteArray openSecret = gpg({QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"),
                                       QString(), QStringLiteral("--armor"), QStringLiteral("--export-secret-keys"), of}).out;
    KeyRing openRing;
    const ImportReport openRep = openRing.importKeys(openSecret);
    check(!of.isEmpty() && openRep.imported == QStringList{of} && openRep.withSecret.isEmpty() && !openRing.hasSecret(of)
              && openRep.refused.size() == 1,
          QStringLiteral("%1: a secret key without a passphrase: only its public part is taken").arg(tag),
          openRep.refused.isEmpty() ? QString() : openRep.refused.first().second);

    // 4. keys of another kind are refused with a message
    auto refusedWith = [&](const QString &what, const QStringList &genArgs, const QString &word) {
        const Run g = gpg(QStringList{QStringLiteral("--pinentry-mode"), QStringLiteral("loopback"), QStringLiteral("--passphrase"),
                                      QString(), QStringLiteral("--quick-gen-key")} + genArgs);
        const QString f = gpg.fingerprintOf(genArgs.first());
        if (g.code != 0 || f.isEmpty()) {
            info(QStringLiteral("%1 cannot make %2 (%3): not checked").arg(tag, what, QString::fromUtf8(g.err.trimmed().right(200))));
            return;
        }
        const QByteArray pub = gpg({QStringLiteral("--armor"), QStringLiteral("--export"), f}).out;
        KeyRing fresh;
        const ImportReport ir = fresh.importKeys(pub);
        const QString why = ir.refused.isEmpty() ? QString() : ir.refused.first().second;
        check(ir.error == Error::ForeignKey && ir.imported.isEmpty() && fresh.keys().isEmpty() && why.contains(word),
              QStringLiteral("%1: %2 is refused (\"%3\")").arg(tag, what, why), errorText(ir.error));
    };
    refusedWith(QStringLiteral("an RSA key"), {QStringLiteral("RSA Writer"), QStringLiteral("rsa2048"), QStringLiteral("default"),
                                              QStringLiteral("never")}, QStringLiteral("RSA"));
    refusedWith(QStringLiteral("a version 5 key (Ed448)"), {QStringLiteral("Five Writer"), QStringLiteral("ed448"),
                                                            QStringLiteral("default"), QStringLiteral("never")},
                QStringLiteral("version 5"));
    refusedWith(QStringLiteral("a NIST P-256 key"), {QStringLiteral("Nist Writer"), QStringLiteral("nistp256"),
                                                     QStringLiteral("default"), QStringLiteral("never")}, QStringLiteral("ECDSA"));
    if (name.contains(QLatin1String(" 2.5")))
        refusedWith(QStringLiteral("a post-quantum key (pqc)"), {QStringLiteral("Pqc Writer"), QStringLiteral("pqc"),
                                                                 QStringLiteral("default"), QStringLiteral("never")},
                    QStringLiteral("version 5"));
    ring.lock(writer);
    gpg.stop();
}

int main(int argc, char **argv)
{
    if (argc > 1 && QByteArray(argv[1]) == "--child-secret-service") {
        QCoreApplication app(argc, argv);
        return childSecretService();
    }
    if (argc > 1 && QByteArray(argv[1]) == "--child-no-store") {
        QCoreApplication app(argc, argv);
        return childNoStore();
    }
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_TEST_KEYSTORE", "memory");
    QCoreApplication app(argc, argv);
    QTemporaryDir tmpDir;
    const QString tmp = tmpDir.path();
    QElapsedTimer timer;

    // --- the library ---
    check(available(), QStringLiteral("this build has the OpenPGP library (%1)").arg(libraryVersion()),
          QStringLiteral("no RNP in this build: tools/get-crypto.sh, then configure again"));
    if (!available()) {
        std::printf("\n%d failure(s)\n", failures);
        return failures;
    }
    check(libraryVersion().startsWith(QLatin1String("RNP 0.18.1")) && libraryVersion().contains(QLatin1String("Botan")),
          QStringLiteral("RNP 0.18.1 with Botan"), libraryVersion());
    check(groupedFingerprint(QStringLiteral("6925bc66304795e7dab8feba364a438c375bb5e0"))
              == QLatin1String("6925 BC66 3047 95E7 DAB8  FEBA 364A 438C 375B B5E0")
              && groupedFingerprint(QStringLiteral("xyz")) == QLatin1String("xyz"),
          QStringLiteral("fingerprints are shown in ten groups of four, as GnuPG shows them"));

    // --- keys ---
    KeyRing ring;
    Error e = Error::Failed;
    timer.start();
    const QString writer = ring.createKey(QStringLiteral("Quillbinder test writer"), kWriterPass, &e);
    info(QStringLiteral("the writer's key made in %1 ms").arg(timer.elapsed()));
    const QString recovery = ring.createKey(QStringLiteral("Quillbinder test recovery key"), kRecoveryPass, &e);
    check(writer.size() == 40 && recovery.size() == 40 && writer != recovery && e == Error::None,
          QStringLiteral("two keys made (%1)").arg(groupedFingerprint(writer)), errorText(e) + QLatin1Char(' ') + ring.errorDetail());
    const KeyInfo wk = ring.key(writer);
    check(wk.secret && !wk.unlocked && wk.encryptionFingerprint.size() == 40 && wk.userId == QLatin1String("Quillbinder test writer")
              && qAbs(wk.created.secsTo(QDateTime::currentDateTimeUtc())) < 600,
          QStringLiteral("the key's facts: secret part here, locked, an encryption subkey, its user id and date"));
    check(ring.keys().size() == 2, QStringLiteral("the ring lists two keys"));
    const QByteArray pub = ring.exportPublicKey(writer), sec = ring.exportSecretKey(writer);
    check(pub.startsWith("-----BEGIN PGP PUBLIC KEY BLOCK-----") && sec.startsWith("-----BEGIN PGP PRIVATE KEY BLOCK-----"),
          QStringLiteral("keys are exported armored (.asc)"));
    {
        KeyRing pubOnly;
        const ImportReport r = pubOnly.importKeys(pub);
        const KeyInfo k = pubOnly.key(writer);
        check(r.error == Error::None && r.imported == QStringList{writer} && r.withSecret.isEmpty() && !k.secret
                  && k.encryptionFingerprint == wk.encryptionFingerprint,
              QStringLiteral("the public key is taken by another ring (without a secret part)"));
        const QByteArray c = pubOnly.encrypt("only the public keys", {writer}, &e);
        check(e == Error::None && looksEncrypted(c), QStringLiteral("encrypting needs only the public key"));
        pubOnly.decrypt(c, &e);
        check(e == Error::NoSecretKey, QStringLiteral("a ring without the secret part cannot decrypt (%1)").arg(errorText(e)));
    }
    // --- unlock, wrong passphrase ---
    check(ring.unlock(writer, "lantern pebble orchard velvet compass") == Error::WrongPassphrase && !ring.isUnlocked(writer),
          QStringLiteral("a wrong passphrase does not unlock the key"));
    timer.restart();
    check(ring.unlock(writer, kWriterPass) == Error::None && ring.isUnlocked(writer), QStringLiteral("the right one does"));
    const qint64 unlockMs = timer.elapsed();
    info(QStringLiteral("unlocking took %1 ms (RNP's S2K, tuned to about 150 ms)").arg(unlockMs));
    check(unlockMs >= 20 && unlockMs < 5000, QStringLiteral("unlocking costs a noticeable time (iterated S2K), %1 ms").arg(unlockMs));
    check(ring.unlock(writer, "wrong") == Error::None && ring.isUnlocked(writer),
          QStringLiteral("a key unlocked in this session stays unlocked (once per session)"));
    // the same passphrase in another Unicode form (decomposed) opens a key made with the composed form
    {
        KeyRing u;
        const QByteArray composed = QStringLiteral("Grüße aus der Bücherei, zweiundvierzig").toUtf8();
        const QByteArray decomposed = QStringLiteral("Grüße aus der Bücherei, zweiundvierzig").normalized(QString::NormalizationForm_D).toUtf8();
        const QString f = u.createKey(QStringLiteral("Unicode"), composed);
        check(composed != decomposed && u.unlock(f, decomposed) == Error::None,
              QStringLiteral("passphrases are taken in Unicode's composed form (NFC)"));
    }

    // --- round trips ---
    const QByteArray text = QStringLiteral("Der erste Satz. ¶ Zweiter Absatz mit Ümlauten, 日本語, emoji \U0001F58B.\n").toUtf8().repeated(200);
    const QByteArray cipher = ring.encrypt(text, {writer, recovery}, &e);
    check(e == Error::None && looksEncrypted(cipher) && !cipher.contains("Der erste Satz"), QStringLiteral("a text is encrypted"));
    const QList<QPair<int, int>> pk = packets(cipher);
    QList<int> tags;
    for (const auto &p : pk)
        tags.append(p.first);
    check(tags == QList<int>{1, 1, 18} && pk.at(0).second == 3 && pk.at(2).second == 1,
          QStringLiteral("its packets: two session keys (one per recipient) and SEIPD version 1, no AEAD packet"),
          QStringLiteral("tags %1").arg([&] { QStringList s; for (int t : tags) s << QString::number(t); return s.join(QLatin1Char(' ')); }()));
    QStringList named;
    QByteArray plain = ring.decrypt(cipher, &e, qint64(1) << 30, &named);
    check(e == Error::None && plain == text, QStringLiteral("and decrypted with the writer's key"), errorText(e) + ring.errorDetail());
    check(named.size() == 2 && named.contains(wk.encryptionFingerprint.right(16)) && named.contains(ring.key(recovery).encryptionFingerprint.right(16)),
          QStringLiteral("the message names both keys: the writer's and the recovery key"), named.join(QLatin1Char(' ')));
    {
        // the recovery key alone (in a ring of its own, as after "both keys lost but the recovery key")
        KeyRing rec;
        rec.importKeys(ring.exportSecretKey(recovery));
        check(rec.unlock(recovery, kRecoveryPass) == Error::None && rec.decrypt(cipher, &e) == text && e == Error::None,
              QStringLiteral("the recovery key alone decrypts it too"));
    }
    const QByteArray armored = ring.encrypt(text, {writer, recovery}, &e, true);
    check(armored.startsWith("-----BEGIN PGP MESSAGE-----") && looksEncrypted(armored) && ring.decrypt(armored, &e) == text,
          QStringLiteral("armored messages too"));
    check(!looksEncrypted("plain text") && !looksEncrypted(QByteArray()) && !looksEncrypted(pub),
          QStringLiteral("plain text and keys do not look encrypted"));
    const QByteArray empty = ring.encrypt(QByteArray(), {writer, recovery}, &e);
    check(e == Error::None && ring.decrypt(empty, &e).isEmpty() && e == Error::None, QStringLiteral("an empty text goes and comes back"));
    ring.encrypt(text, {}, &e);
    check(e == Error::NoRecipient, QStringLiteral("no recipient: refused (%1)").arg(errorText(e)));
    ring.encrypt(text, {writer, QStringLiteral("0000000000000000000000000000000000000000")}, &e);
    check(e == Error::UnknownKey, QStringLiteral("an unknown recipient: refused (%1)").arg(errorText(e)));
    // files: 3 MB of random bytes, names with letters beyond ASCII
    {
        QByteArray big(3 << 20, Qt::Uninitialized);
        for (qsizetype i = 0; i < big.size(); i += 4) {
            const quint32 r = QRandomGenerator::global()->generate();
            std::memcpy(big.data() + i, &r, 4);
        }
        const QString src = tmp + QStringLiteral("/Kapitel ü 1.bin"), enc = tmp + QStringLiteral("/Kapitel ü 1.bin.gpg"),
                      dec = tmp + QStringLiteral("/Kapitel ü 1 zurück.bin");
        writeAll(src, big);
        check(ring.encryptFile(src, enc, {writer, recovery}) == Error::None && QFileInfo(enc).size() > big.size(),
              QStringLiteral("a file is encrypted into another"));
        check(ring.decryptFile(enc, dec) == Error::None && readAll(dec) == big, QStringLiteral("and decrypted into a third, unchanged"));
        QByteArray cut = readAll(enc);
        cut.truncate(cut.size() / 2);
        writeAll(enc + QStringLiteral(".cut"), cut);
        QFile::remove(dec);
        check(ring.decryptFile(enc + QStringLiteral(".cut"), dec) == Error::Damaged && !QFileInfo::exists(dec),
              QStringLiteral("a file cut in half: refused as damaged, nothing written"));
        check(ring.encryptFile(tmp + QStringLiteral("/missing"), enc + QStringLiteral(".x"), {writer}) == Error::Io,
              QStringLiteral("a missing source file: an error"));
    }
    // the limit on the size of a text
    {
        const QByteArray big = QByteArray(2 << 20, 'x');
        const QByteArray c = ring.encrypt(big, {writer}, &e);
        const QByteArray d = ring.decrypt(c, &e, 1 << 20);
        check(e == Error::TooLarge && d.isEmpty(), QStringLiteral("a text above the limit is refused (%1)").arg(errorText(e)));
    }

    // --- damaged and strange input: no crash, a clear error, no text ---
    {
        QByteArray flipped = cipher;
        flipped[flipped.size() / 2] = char(flipped.at(flipped.size() / 2) ^ 0x20);
        plain = ring.decrypt(flipped, &e);
        check(e == Error::Damaged && plain.isEmpty(), QStringLiteral("one changed byte inside: damaged, no text (%1)").arg(errorText(e)),
              ring.errorDetail());
        QByteArray last = cipher;
        last[last.size() - 3] = char(last.at(last.size() - 3) ^ 0x01);
        plain = ring.decrypt(last, &e);
        check(e == Error::Damaged && plain.isEmpty(), QStringLiteral("a changed byte in the integrity check: damaged"), ring.errorDetail());
        plain = ring.decrypt(cipher.left(cipher.size() - 10), &e);
        check(e == Error::Damaged && plain.isEmpty(), QStringLiteral("a message cut short: damaged"));
        QByteArray noise(1000, Qt::Uninitialized);
        for (char &c : noise)
            c = char(QRandomGenerator::global()->bounded(256));
        ring.decrypt(noise, &e);
        const Error noiseError = e;
        ring.decrypt(QByteArray(), &e);
        const Error emptyError = e;
        ring.decrypt("Hello, plain text.", &e);
        check(noiseError == Error::Damaged && emptyError == Error::Damaged && e == Error::Damaged,
              QStringLiteral("noise, nothing and plain text: damaged"));
        // L3: a message without the integrity check (a symmetrically encrypted data packet, tag 9, after the same session
        // keys) is refused, and no text comes back
        {
            qsizetype pos = 0;
            while (pos + 2 < cipher.size()) {
                const unsigned char b = static_cast<unsigned char>(cipher.at(pos));
                const unsigned char o1 = static_cast<unsigned char>(cipher.at(pos + 1));
                qsizetype hdr = 0, len = 0;
                int tag = 0;
                if (b & 0x40) {
                    tag = b & 0x3f;
                    if (o1 < 192) {
                        hdr = 2;
                        len = o1;
                    } else if (o1 < 224) {
                        hdr = 3;
                        len = ((o1 - 192) << 8) + static_cast<unsigned char>(cipher.at(pos + 2)) + 192;
                    } else {
                        break;
                    }
                } else {
                    tag = (b >> 2) & 0x0f;
                    const int type = b & 3;
                    if (type == 0) {
                        hdr = 2;
                        len = o1;
                    } else if (type == 1) {
                        hdr = 3;
                        len = (o1 << 8) + static_cast<unsigned char>(cipher.at(pos + 2));
                    } else {
                        break;
                    }
                }
                if (tag != 1)
                    break;
                pos += hdr + len;
            }
            QByteArray sed = cipher.left(pos);
            sed.append(char(0xA7)); // old format, tag 9, the body runs to the end
            sed.append(cipher.mid(pos + 6, 300));
            plain = ring.decrypt(sed, &e);
            check(pos > 0 && e != Error::None && plain.isEmpty(), QStringLiteral("a message with a tag 9 data packet (no integrity check) is refused: %1").arg(errorText(e)),
                  ring.errorDetail());
        }
        QByteArray header = cipher.left(200);
        ring.decrypt(header + QByteArray(5000, '\0'), &e);
        check(e == Error::Damaged, QStringLiteral("a good start and then zeros: damaged"), errorText(e) + QLatin1Char(' ') + ring.errorDetail());
    }
    ring.lock(writer);
    check(!ring.isUnlocked(writer), QStringLiteral("the key is locked again"));
    ring.decrypt(cipher, &e);
    check(e == Error::Locked, QStringLiteral("a locked key: the message says to unlock it (%1)").arg(errorText(e)));

    // --- foreign keys (made by hand: no GnuPG needed) and things that are no keys ---
    {
        KeyRing f;
        ImportReport r = f.importKeys(handMadeKey(5, 22));
        check(r.error == Error::ForeignKey && r.refused.size() == 1 && r.refused.first().second.contains(QLatin1String("version 5")),
              QStringLiteral("a key of version 5 is refused: \"%1\"").arg(r.refused.value(0).second));
        r = f.importKeys(handMadeKey(6, 27));
        check(r.error == Error::ForeignKey && r.refused.value(0).second.contains(QLatin1String("version 6")),
              QStringLiteral("a key of version 6 is refused: \"%1\"").arg(r.refused.value(0).second));
        r = f.importKeys(handMadeKey(4, 1));
        check(r.error == Error::ForeignKey && r.refused.value(0).second.contains(QLatin1String("RSA")),
              QStringLiteral("an RSA key is refused: \"%1\"").arg(r.refused.value(0).second));
        r = f.importKeys(handMadeKey(4, 105));
        check(r.error == Error::ForeignKey && r.refused.value(0).second.contains(QLatin1String("post-quantum")),
              QStringLiteral("an unknown (post-quantum) algorithm is refused: \"%1\"").arg(r.refused.value(0).second));
        r = f.importKeys("no key at all");
        check(r.error == Error::NotAKey && f.keys().isEmpty(), QStringLiteral("text that is no key: \"%1\"").arg(errorText(r.error)));
        r = f.importKeys(armored);
        check(r.error == Error::NotAKey, QStringLiteral("a message is no key"));
        r = f.importKeys(pub.left(pub.size() / 2));
        check(r.error == Error::NotAKey && f.keys().isEmpty(), QStringLiteral("a key file cut in half: not taken"));
        // a good key and a foreign one in one file: the good one is taken
        r = f.importKeys(ring.exportPublicKey(recovery) + "\n" + ring.exportPublicKey(writer));
        check(r.imported.size() == 2 && r.error == Error::None, QStringLiteral("two keys in one file are both taken"));
        f.encrypt("x", {writer}, &e);
        check(e == Error::None, QStringLiteral("an imported public key encrypts"));
    }

    // --- the key ring on disk ---
    {
        const QString dir = tmp + QStringLiteral("/keyring");
        QString err;
        check(ring.save(dir, &err), QStringLiteral("the ring is saved"), err);
        const QFileDevice::Permissions own = QFileDevice::ReadOwner | QFileDevice::WriteOwner;
        check((QFile::permissions(dir + QStringLiteral("/secring.gpg")) & ~(own | QFileDevice::ReadUser | QFileDevice::WriteUser)) == 0
                  && (QFile::permissions(dir) & (QFileDevice::ReadOther | QFileDevice::ReadGroup | QFileDevice::ExeOther)) == 0,
              QStringLiteral("its folder and the secret ring are for the owner only"));
        KeyRing loaded;
        check(loaded.load(dir, &err) && loaded.keys().size() == 2 && loaded.hasSecret(writer) && !loaded.isUnlocked(writer),
              QStringLiteral("and read again: both keys, secret parts, locked"), err);
        check(loaded.unlock(writer, kWriterPass) == Error::None && loaded.decrypt(cipher, &e) == text,
              QStringLiteral("the key read again decrypts"));
        KeyRing none;
        check(none.load(tmp + QStringLiteral("/no-such-folder"), &err) && none.keys().isEmpty(), QStringLiteral("no folder: an empty ring"));
        writeAll(dir + QStringLiteral("/pubring.gpg"), "garbage");
        KeyRing bad;
        err.clear();
        const bool badLoaded = bad.load(dir, &err);
        check(!badLoaded && !err.isEmpty(), QStringLiteral("a damaged ring file: an error (%1)").arg(err));
        check(KeyRing::deviceDir().endsWith(QLatin1String("/keyring")) && KeyRing::deviceDir().startsWith(home.path()),
              QStringLiteral("the device's ring is in the data folder: %1").arg(KeyRing::deviceDir()));
        check(KeyRing::device().keys().isEmpty(), QStringLiteral("the device's ring starts empty"));
    }

    // --- passphrase change, removing the secret part ---
    {
        KeyRing c;
        c.importKeys(ring.exportSecretKey(writer));
        const QByteArray newPass = "a brand new passphrase for this key here";
        check(c.changePassphrase(writer, "wrong", newPass) == Error::WrongPassphrase && c.unlock(writer, kWriterPass) == Error::None,
              QStringLiteral("a passphrase change with a wrong old one changes nothing"));
        c.lock(writer);
        check(c.changePassphrase(writer, kWriterPass, newPass) == Error::None && c.unlock(writer, kWriterPass) == Error::WrongPassphrase
                  && c.unlock(writer, newPass) == Error::None,
              QStringLiteral("after a change only the new passphrase opens the key"));
        check(c.removeKey(writer, true) && c.contains(writer) && !c.hasSecret(writer) && c.exportSecretKey(writer, &e).isEmpty()
                  && e == Error::NoSecretKey,
              QStringLiteral("the secret part alone can be removed (the public key stays)"));
        check(c.removeKey(writer) && !c.contains(writer), QStringLiteral("and the whole key"));
    }

    // --- error texts ---
    {
        bool all = true;
        for (Error x : {Error::Unavailable, Error::WrongPassphrase, Error::Locked, Error::NoSecretKey, Error::Damaged, Error::Unprotected,
                        Error::ForeignKey, Error::NotAKey, Error::UnknownKey, Error::NoRecipient, Error::TooLarge, Error::Io, Error::Failed})
            all = all && errorText(x).endsWith(QLatin1Char('.'));
        check(all && errorText(Error::None).isEmpty(), QStringLiteral("every error has a sentence for the writer"));
    }

    // --- the passphrase generator and the strength check ---
    {
        const QStringList &w = passphrase::words();
        static const QRegularExpression wordRe(QStringLiteral("^[a-z-]+$"));
        bool shape = w.size() == 7776 && w.first() == QLatin1String("abacus") && w.last() == QLatin1String("zoom");
        for (const QString &x : w)
            shape = shape && wordRe.match(x).hasMatch();
        check(shape && QSet<QString>(w.begin(), w.end()).size() == 7776, QStringLiteral("the word list: the EFF's 7,776 words"));
        const QString p = passphrase::generate();
        const QStringList parts = p.split(QLatin1Char(' '));
        bool inList = parts.size() == 6;
        for (const QString &x : parts)
            inList = inList && w.contains(x);
        check(inList, QStringLiteral("a passphrase of six words of the list: \"%1\"").arg(p));
        QSet<QString> seen, phrases;
        for (int i = 0; i < 1000; ++i) {
            const QString g = passphrase::generate();
            phrases.insert(g);
            for (const QString &x : g.split(QLatin1Char(' ')))
                seen.insert(x);
        }
        check(phrases.size() == 1000 && seen.size() > 3000, QStringLiteral("1,000 passphrases, all different, %1 different words").arg(seen.size()));
        const passphrase::Strength gs = passphrase::check(p);
        check(gs.ok && gs.bits >= 77, QStringLiteral("a generated passphrase passes the check (%1 bits)").arg(gs.bits, 0, 'f', 1));
        const QStringList weak = {QStringLiteral("password1234"), QStringLiteral("aaaaaaaaaaaaaaaa"), QStringLiteral("qwertyuiop123"),
                                  QStringLiteral("abcdefghijklmnop"), QStringLiteral("Sommer2024Sommer2024"), QStringLiteral("quillbinder2026"),
                                  QStringLiteral("1234567890123456"), w.at(10) + QLatin1Char(' ') + w.at(20) + QLatin1Char(' ') + w.at(30)};
        for (const QString &x : weak) {
            const passphrase::Strength s = passphrase::check(x);
            check(!s.ok && !s.reason.isEmpty(), QStringLiteral("refused: \"%1\" (%2 bits)").arg(x).arg(s.bits, 0, 'f', 1), s.reason);
        }
        const QStringList shortOnes = {QStringLiteral("Tr0ub4dor&3"), QStringLiteral("x7#Kq9!vB2"), QString()};
        for (const QString &x : shortOnes) {
            const passphrase::Strength s = passphrase::check(x);
            check(!s.ok && s.reason.contains(QLatin1String("too short")), QStringLiteral("too short: \"%1\"").arg(x));
        }
        const QStringList strong = {QStringLiteral("x7#Kq9!vB2mZ@p4L"), w.at(100) + QLatin1Char(' ') + w.at(2000) + QLatin1Char(' ') + w.at(4000)
                                        + QLatin1Char(' ') + w.at(7000),
                                    QStringLiteral("Mein Kater frisst um 7 Uhr 3 Sardinen!"), QString::fromUtf8(kWriterPass)};
        for (const QString &x : strong) {
            const passphrase::Strength s = passphrase::check(x);
            check(s.ok && s.reason.isEmpty(), QStringLiteral("taken: \"%1\" (%2 bits)").arg(x).arg(s.bits, 0, 'f', 1), s.reason);
        }
    }

    // --- the key store ---
    {
        auto m = KeyStore::memory();
        const QString entry = KeyStore::passphraseEntry(writer.toLower());
        QByteArray back;
        check(entry == QLatin1String("OpenPGP passphrase ") + writer && KeyStore::service() == QLatin1String("Quillbinder"),
              QStringLiteral("entries are named \"%1\" under the service \"Quillbinder\"").arg(entry));
        check(m->read(entry, &back) == KeyStore::Status::NotFound && m->write(entry, kWriterPass) == KeyStore::Status::Ok
                  && m->read(entry, &back) == KeyStore::Status::Ok && back == kWriterPass && m->remove(entry) == KeyStore::Status::Ok
                  && m->read(entry, &back) == KeyStore::Status::NotFound,
              QStringLiteral("the test store keeps, gives back and forgets a passphrase"));
        check(KeyStore::system().name() == QLatin1String("memory"), QStringLiteral("QB_TEST_KEYSTORE=memory makes the system's store a test store"));
        // the passphrase remembered, the key unlocked from it in a new session
        KeyStore::system().write(entry, kWriterPass);
        KeyRing session;
        session.importKeys(ring.exportSecretKey(writer));
        QByteArray remembered;
        check(KeyStore::system().read(entry, &remembered) == KeyStore::Status::Ok && session.unlock(writer, remembered) == Error::None,
              QStringLiteral("a remembered passphrase unlocks the key in a new session"));
        wipe(remembered);
        check(remembered.isEmpty(), QStringLiteral("a passphrase buffer is wiped"));

        const QString self = QCoreApplication::applicationFilePath();
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        for (const char *v : {"QB_TEST_KEYSTORE", "DBUS_SESSION_BUS_ADDRESS", "DISPLAY", "WAYLAND_DISPLAY", "XDG_CURRENT_DESKTOP",
                              "KDE_SESSION_VERSION", "KDE_FULL_SESSION", "DESKTOP_SESSION", "GNOME_DESKTOP_SESSION_ID"})
            env.remove(QString::fromLatin1(v));
        QTemporaryDir childHome(QStringLiteral("/tmp/qbks-XXXXXX"));
        childHome.setAutoRemove(false); // (removed below, once the key store's daemon is gone)
        env.insert(QStringLiteral("HOME"), childHome.path());
        env.insert(QStringLiteral("XDG_RUNTIME_DIR"), childHome.path());
        env.insert(QStringLiteral("XDG_DATA_HOME"), childHome.path() + QStringLiteral("/data"));
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
        timer.restart();
        Run r = run(self, {QStringLiteral("--child-no-store")}, env, QByteArray(), 60000);
        info(QString::fromUtf8(r.out.trimmed()));
        check(r.code == 0 && r.out.contains("CHILD-OK") && timer.elapsed() < 45000,
              QStringLiteral("QtKeychain without a key store answers (no passphrase, no hang, no crash)"), QString::fromUtf8(r.err.right(500)));
        const QString dbus = QStandardPaths::findExecutable(QStringLiteral("dbus-run-session"));
        const QString keyring = QStandardPaths::findExecutable(QStringLiteral("gnome-keyring-daemon"));
        if (dbus.isEmpty() || keyring.isEmpty()) {
            info(QStringLiteral("no dbus-run-session or gnome-keyring-daemon here: the Secret Service is not checked"));
        } else {
            r = run(dbus, {QStringLiteral("--"), QStringLiteral("sh"), QStringLiteral("-c"),
                           QStringLiteral("printf 'unlock-for-the-check' | gnome-keyring-daemon --unlock --components=secrets >/dev/null 2>&1; "
                                          "sleep 0.5; exec \"$0\" --child-secret-service"),
                           self},
                    env, QByteArray(), 90000);
            info(QString::fromUtf8(r.out.trimmed()));
            check(r.code == 0 && r.out.contains("CHILD-OK"),
                  QStringLiteral("QtKeychain keeps, gives back and forgets a passphrase in the Secret Service (libsecret loaded at run time)"),
                  QString::fromUtf8(r.err.right(800)));
        }
        QElapsedTimer wait;
        wait.start();
        while (QFileInfo::exists(childHome.path()) && !QDir(childHome.path()).removeRecursively() && wait.elapsed() < 5000)
            QThread::msleep(200);
    }

    // --- what the program links: nothing new (the program update runs on the AppImage's libraries) ---
    {
        const QString self = QCoreApplication::applicationFilePath();
        const QString readelf = QStandardPaths::findExecutable(QStringLiteral("readelf"));
        const QString nm = QStandardPaths::findExecutable(QStringLiteral("nm"));
        if (readelf.isEmpty() || nm.isEmpty()) {
            info(QStringLiteral("no readelf or nm here: the libraries are not checked"));
        } else {
            const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            const Run d = run(readelf, {QStringLiteral("-d"), self}, env);
            QStringList needed;
            static const QRegularExpression lib(QStringLiteral("Shared library: \\[([^\\]]+)\\]"));
            for (auto it = lib.globalMatch(QString::fromUtf8(d.out)); it.hasNext();)
                needed << it.next().captured(1);
            static const QRegularExpression allowed(
                QStringLiteral("^(libQt6(Core|Gui|Xml|DBus|Network)\\.so\\.6|libz\\.so\\.1|libhunspell-1\\.7\\.so\\.0|libstdc\\+\\+\\.so\\.6|"
                               "libm\\.so\\.6|libgcc_s\\.so\\.1|libc\\.so\\.6|ld-linux-x86-64\\.so\\.2)$"));
            QStringList other;
            for (const QString &n : needed)
                if (!allowed.match(n).hasMatch())
                    other << n;
            check(!needed.isEmpty() && other.isEmpty(),
                  QStringLiteral("the encryption adds no shared library (%1)").arg(needed.join(QStringLiteral(", "))), other.join(QLatin1Char(' ')));
            const Run all = run(nm, {QStringLiteral("--defined-only"), self}, env);
            const Run dyn = run(nm, {QStringLiteral("-D"), QStringLiteral("--defined-only"), self}, env);
            static const QRegularExpression foreign(QStringLiteral("\\b(rnp_ffi_create|json_object_new_object|BZ2_bzCompress|_ZN5Botan\\w*)\\b"));
            check(all.out.contains("rnp_ffi_create") && all.out.contains("_ZN5Botan") && !foreign.match(QString::fromUtf8(dyn.out)).hasMatch()
                      && dyn.code == 0,
                  QStringLiteral("RNP and Botan are inside, and none of their names is exported (the program exports its own for plugins)"),
                  foreign.match(QString::fromUtf8(dyn.out)).captured(0));
        }
    }

    // --- GnuPG, both ways ---
    QStringList gpgs;
    const QString onPath = QStandardPaths::findExecutable(QStringLiteral("gpg"));
    if (!onPath.isEmpty())
        gpgs << onPath;
    for (const QString &x : qEnvironmentVariable("QB_TEST_GPG_EXTRA").split(QLatin1Char(' '), Qt::SkipEmptyParts))
        gpgs << x;
    if (gpgs.isEmpty())
        info(QStringLiteral("SKIPPED: no gpg here (Ubuntu: apt install gnupg): the interoperability with GnuPG is not checked"));
    for (const QString &g : std::as_const(gpgs))
        interop(g, tmp, ring, writer, recovery);

    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
