// Live Sync, work package 1 (0.99.0): the Quillbinder wallet (the key store's fallback), the device identity, the frames,
// the peer store, the in-memory link, the TLS 1.3 link and its listener with pins, and pairing with its window.
// No window; the folder given as the first argument is not used.
//
//   QB_SAMPLE=0 QB_SUITES=ls1 tools/run-all-tests.sh build/test-results-x11 x11      (or ./qb_ls1_tests /tmp/shots)
// Exit code = number of failed checks.
#include "core/collab/net/DeviceIdentity.h"
#include "core/collab/net/Frames.h"
#include "core/collab/net/MemLink.h"
#include "core/collab/net/Pairing.h"
#include "core/collab/net/PeerStore.h"
#include "core/collab/net/TlsLink.h"
#include "core/crypto/KeyStore.h"
#include "core/crypto/Wallet.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTimer>
#include <cstdio>
#include <memory>

using namespace qb;
using namespace qb::collab::net;
using crypto::KeyStore;
using crypto::Wallet;

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

static bool waitFor(const std::function<bool()> &cond, int ms = 8000)
{
    QElapsedTimer t;
    t.start();
    while (!cond() && t.elapsed() < ms) {
        QEventLoop loop;
        QTimer::singleShot(5, &loop, &QEventLoop::quit);
        loop.exec();
    }
    return cond();
}
static void pause(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}
static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
static void writeAll(const QString &path, const QByteArray &d)
{
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(d);
}

// a key store that is not there (a system without KWallet or Secret Service)
class NoStore : public KeyStore {
public:
    QString name() const override { return QStringLiteral("none"); }
    Status write(const QString &, const QByteArray &, QString *) override { return Status::NoBackend; }
    Status read(const QString &, QByteArray *, QString *) override { return Status::NoBackend; }
    Status remove(const QString &, QString *) override { return Status::NoBackend; }
};

// ---------------------------------------------------------------------------------------------------------------------
static void testWallet(const QString &tmp)
{
    check(Wallet::available(), QStringLiteral("the wallet is in this build (Argon2 and AES-GCM of the pack)"));
    const QString file = tmp + QStringLiteral("/w1/wallet.qbw");
    const QByteArray secret = "hunter2-secret-xyz-0123456789";
    {
        Wallet w(file);
        check(!w.exists() && !w.isUnlocked(), QStringLiteral("a wallet that is not made yet does not exist"));
        QByteArray out;
        check(w.read(QStringLiteral("a"), &out) == Wallet::Result::Locked, QStringLiteral("a locked wallet answers Locked"));
        check(w.write(QStringLiteral("a"), secret) == Wallet::Result::Locked, QStringLiteral("a locked wallet takes nothing"));
        check(w.create(QString(), nullptr) == Wallet::Result::Failed, QStringLiteral("an empty master passphrase is refused"));
        check(w.create(QStringLiteral("correct horse"), nullptr) == Wallet::Result::Ok && w.exists() && w.isUnlocked(),
              QStringLiteral("create: the file is there and the wallet is open"));
        check(w.create(QStringLiteral("other"), nullptr) == Wallet::Result::Exists, QStringLiteral("create does not overwrite a wallet"));
        check(w.write(QStringLiteral("Live Sync device key"), secret) == Wallet::Result::Ok &&
                  w.write(QStringLiteral("OpenPGP passphrase ABCD"), "pass phrase two") == Wallet::Result::Ok,
              QStringLiteral("two entries written"));
        const QByteArray raw = readAll(file);
        check(!raw.isEmpty() && !raw.contains(secret) && !raw.contains(secret.toBase64()) && !raw.contains("pass phrase two") &&
                  !raw.contains("Live Sync device key") && !raw.contains("correct horse"),
              QStringLiteral("the file holds no entry, no entry name and no passphrase in plain or base64 form"));
        check(raw.startsWith("QBWALLET1\n"), QStringLiteral("the file starts with its magic"));
#ifdef Q_OS_UNIX
        check((QFile::permissions(file) & (QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::WriteGroup |
                                           QFileDevice::WriteOther)) == 0,
              QStringLiteral("only the owner can read the file"));
#endif
    }
    {
        Wallet w(file);
        QString err;
        check(w.unlock(QStringLiteral("wrong"), &err) == Wallet::Result::WrongPassphrase && !w.isUnlocked(),
              QStringLiteral("a wrong passphrase does not open the wallet"), err);
        QByteArray out;
        check(w.read(QStringLiteral("Live Sync device key"), &out) == Wallet::Result::Locked && out.isEmpty(),
              QStringLiteral("after a wrong passphrase nothing can be read"));
        check(w.unlock(QStringLiteral("correct horse"), &err) == Wallet::Result::Ok, QStringLiteral("the right passphrase opens it"), err);
        check(w.read(QStringLiteral("Live Sync device key"), &out) == Wallet::Result::Ok && out == secret,
              QStringLiteral("an entry survives closing the program"));
        check(w.entries().size() == 2, QStringLiteral("both entries are there"));
        check(w.read(QStringLiteral("none"), &out) == Wallet::Result::NotFound, QStringLiteral("a missing entry is NotFound"));
        w.lock();
        check(!w.isUnlocked() && w.read(QStringLiteral("Live Sync device key"), &out) == Wallet::Result::Locked,
              QStringLiteral("lock forgets the key and the entries"));
    }
    // a changed byte: in the sealed part, and in the header (which is authenticated too)
    {
        const QByteArray good = readAll(file);
        QByteArray bad = good;
        bad[bad.size() - 3] = char(bad[bad.size() - 3] ^ 1);
        const QString f2 = tmp + QStringLiteral("/w1/tampered.qbw");
        writeAll(f2, bad);
        Wallet w(f2);
        check(w.unlock(QStringLiteral("correct horse")) == Wallet::Result::WrongPassphrase && !w.isUnlocked(),
              QStringLiteral("a changed byte in the sealed part is found"));
        const int nl = good.indexOf('\n', 10);
        QByteArray bad2 = good;
        const int at = good.indexOf("\"nonce\"") - 3; // inside the header, in a field of the JSON's text order
        bad2[at] = char(bad2[at] == 'a' ? 'b' : 'a');
        (void)nl;
        writeAll(f2, bad2);
        Wallet w2(f2);
        check(w2.unlock(QStringLiteral("correct horse")) != Wallet::Result::Ok && !w2.isUnlocked(),
              QStringLiteral("a changed byte in the header is not accepted"));
        // a file that names a huge memory cost cannot make the program allocate it
        QByteArray huge = good;
        huge.replace(QByteArray("\"m\":") + QByteArray::number(1024), QByteArray("\"m\":") + QByteArray::number(4000000));
        check(huge != good, QStringLiteral("(the test file names a cost of 4 GiB)"));
        writeAll(f2, huge);
        QElapsedTimer t;
        t.start();
        Wallet w3(f2);
        check(w3.unlock(QStringLiteral("correct horse")) == Wallet::Result::Failed && t.elapsed() < 2000,
              QStringLiteral("a header with a cost over the limit is refused without work"));
        writeAll(f2, QByteArray("QBWALLET1\nnot json\n") + QByteArray(64, 'x'));
        Wallet w4(f2);
        check(w4.unlock(QStringLiteral("x")) == Wallet::Result::Failed, QStringLiteral("a damaged header is refused"));
        writeAll(f2, QByteArray("short"));
        Wallet w5(f2);
        check(w5.unlock(QStringLiteral("x")) == Wallet::Result::Failed, QStringLiteral("a short file is refused"));
    }
    // change of the passphrase
    {
        Wallet w(file);
        w.unlock(QStringLiteral("correct horse"));
        const QByteArray before = readAll(file);
        check(w.changePassphrase(QStringLiteral("not the old one"), QStringLiteral("new one")) == Wallet::Result::WrongPassphrase,
              QStringLiteral("the old passphrase is asked again for a change"));
        check(readAll(file) == before, QStringLiteral("a refused change leaves the file alone"));
        check(w.changePassphrase(QStringLiteral("correct horse"), QString()) == Wallet::Result::Failed,
              QStringLiteral("an empty new passphrase is refused"));
        check(w.changePassphrase(QStringLiteral("correct horse"), QStringLiteral("new one")) == Wallet::Result::Ok,
              QStringLiteral("the passphrase is changed"));
        Wallet a(file), b(file);
        check(a.unlock(QStringLiteral("correct horse")) == Wallet::Result::WrongPassphrase, QStringLiteral("the old passphrase no longer opens it"));
        QByteArray out;
        check(b.unlock(QStringLiteral("new one")) == Wallet::Result::Ok && b.read(QStringLiteral("Live Sync device key"), &out) == Wallet::Result::Ok &&
                  out == secret,
              QStringLiteral("the new one does, with the entries kept"));
        check(w.remove(QStringLiteral("OpenPGP passphrase ABCD")) == Wallet::Result::Ok && w.remove(QStringLiteral("OpenPGP passphrase ABCD")) == Wallet::Result::NotFound,
              QStringLiteral("an entry is removed once"));
        Wallet c(file);
        c.unlock(QStringLiteral("new one"));
        check(c.entries() == QStringList{QStringLiteral("Live Sync device key")}, QStringLiteral("the removal is in the file"));
    }
    // a write that fails leaves the memory as the file is
    {
        writeAll(tmp + QStringLiteral("/plainfile"), "x");
        Wallet w(tmp + QStringLiteral("/plainfile/sub/wallet.qbw")); // (the parent is a file)
        QString err;
        check(w.create(QStringLiteral("pw"), &err) == Wallet::Result::Failed && !w.isUnlocked(), QStringLiteral("a wallet that cannot be written is not made"), err);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void testFallback(const QString &tmp)
{
    const QString file = tmp + QStringLiteral("/w2/wallet.qbw");
    Wallet::setUnlockHandler(nullptr);
    {
        Wallet w(file);
        auto ks = KeyStore::withWallet(std::make_unique<NoStore>(), &w);
        QString err;
        check(ks->write(QStringLiteral("e"), "s", &err) == KeyStore::Status::NoBackend && !w.exists(),
              QStringLiteral("no system store and nobody to ask: NoBackend as before, no wallet is made"), err);
        QByteArray out;
        check(ks->read(QStringLiteral("e"), &out) == KeyStore::Status::NotFound, QStringLiteral("reading with no wallet at all is NotFound, without a question"));
    }
    int asked = 0;
    bool lastCreating = false;
    QString answer = QStringLiteral("master one");
    Wallet::setUnlockHandler([&](bool creating, QString *pw) {
        ++asked;
        lastCreating = creating;
        *pw = answer;
        return true;
    });
    {
        Wallet w(file);
        auto ks = KeyStore::withWallet(std::make_unique<NoStore>(), &w);
        QString err;
        check(ks->write(QStringLiteral("Live Sync device key"), "device-key-bytes", &err) == KeyStore::Status::Ok && asked == 1 && lastCreating &&
                  w.exists(),
              QStringLiteral("with no system store the first write asks for a new passphrase and makes the wallet"), err);
        check(ks->write(QStringLiteral("OpenPGP passphrase X"), "pp", nullptr) == KeyStore::Status::Ok && asked == 1,
              QStringLiteral("the passphrase is asked once per session"));
        QByteArray out;
        check(ks->read(QStringLiteral("Live Sync device key"), &out) == KeyStore::Status::Ok && out == "device-key-bytes",
              QStringLiteral("the entry is read back through the key store"));
        check(ks->name().contains(QStringLiteral("wallet"), Qt::CaseInsensitive), QStringLiteral("the writer is told it is the wallet"), ks->name());
    }
    {
        asked = 0;
        Wallet w(file); // a new program session
        auto ks = KeyStore::withWallet(std::make_unique<NoStore>(), &w);
        QByteArray out;
        check(ks->read(QStringLiteral("OpenPGP passphrase X"), &out) == KeyStore::Status::Ok && out == "pp" && asked == 1 && !lastCreating,
              QStringLiteral("a new session asks for the passphrase and reads the entry"));
    }
    {
        asked = 0;
        answer = QStringLiteral("wrong");
        Wallet w(file);
        auto ks = KeyStore::withWallet(std::make_unique<NoStore>(), &w);
        QByteArray out;
        check(ks->read(QStringLiteral("OpenPGP passphrase X"), &out) == KeyStore::Status::Denied && asked == 3 && out.isEmpty(),
              QStringLiteral("a wrong passphrase three times: Denied, nothing read"));
        Wallet::setUnlockHandler([&](bool, QString *) {
            ++asked;
            return false;
        });
        asked = 0;
        Wallet w2(file);
        auto ks2 = KeyStore::withWallet(std::make_unique<NoStore>(), &w2);
        check(ks2->write(QStringLiteral("e"), "s") == KeyStore::Status::Denied && asked == 1, QStringLiteral("cancelling the question: Denied"));
    }
    {
        asked = 0;
        Wallet::setUnlockHandler([&](bool, QString *pw) {
            ++asked;
            *pw = QStringLiteral("master one");
            return true;
        });
        const QString file2 = tmp + QStringLiteral("/w2/never.qbw");
        Wallet w(file2);
        auto ks = KeyStore::withWallet(KeyStore::memory(), &w);
        check(ks->write(QStringLiteral("e"), "s") == KeyStore::Status::Ok && asked == 0 && !w.exists(),
              QStringLiteral("where a system store works it is used first: no question, no wallet"));
        QByteArray out;
        check(ks->read(QStringLiteral("e"), &out) == KeyStore::Status::Ok && out == "s", QStringLiteral("and read from"));
    }
    Wallet::setUnlockHandler(nullptr);
}

// ---------------------------------------------------------------------------------------------------------------------
static void testIdentity(const QString &tmp)
{
    check(DeviceIdentity::available() && TlsLink::available(), QStringLiteral("Botan TLS is in this build"));
    auto ks = KeyStore::memory();
    const QString dir = tmp + QStringLiteral("/id1");
    QString err;
    KeyStore::Status st = KeyStore::Status::Failed;
    auto id = DeviceIdentity::loadOrCreate(dir, *ks, &err, &st);
    check(id != nullptr, QStringLiteral("a first identity is made"), err);
    if (!id)
        return;
    check(id->pin().size() == 32 && id->deviceId().size() == 32 &&
              id->deviceId() == QString::fromLatin1(QCryptographicHash::hash(id->publicKeyDer(), QCryptographicHash::Sha256).left(16).toHex()) &&
              id->pin() == QCryptographicHash::hash(id->publicKeyDer(), QCryptographicHash::Sha256),
          QStringLiteral("the pin is SHA-256 of the public key and the device id its first 16 bytes"));
    check(QRegularExpression(QStringLiteral("^[0-9a-f]{16}$")).match(id->instanceId()).hasMatch() && !id->name().isEmpty(),
          QStringLiteral("an instance id of 16 hex digits and a name"));
    QByteArray stored;
    check(ks->read(DeviceIdentity::keyEntry(), &stored) == KeyStore::Status::Ok && !stored.isEmpty(), QStringLiteral("the private key is in the key store"));
    const QByteArray json = readAll(dir + QStringLiteral("/device.json"));
    check(!json.isEmpty() && !json.contains(stored) && !json.contains(QByteArray::fromBase64(stored).left(40).toBase64()),
          QStringLiteral("device.json holds the certificate but not the private key"));
    check(DeviceIdentity::keyAlgorithm() == QLatin1String("ECDSA P-256"), QStringLiteral("the key is ECDSA P-256 (Ed25519 probe: see the report)"));
    auto again = DeviceIdentity::loadOrCreate(dir, *ks, &err);
    check(again && again->pin() == id->pin() && again->instanceId() == id->instanceId() && again->name() == id->name() &&
              again->certificateDer() == id->certificateDer(),
          QStringLiteral("a restart gives the same identity"), err);
    // the certificate file lost: made again from the same key
    QFile::remove(dir + QStringLiteral("/device.json"));
    auto third = DeviceIdentity::loadOrCreate(dir, *ks, &err);
    check(third && third->pin() == id->pin(), QStringLiteral("without device.json the same key gives the same device"), err);
    // signatures
    const QByteArray msg = "a vote";
    const QByteArray sig = id->sign(msg);
    check(!sig.isEmpty() && DeviceIdentity::verify(id->publicKeyDer(), msg, sig), QStringLiteral("a signature checks"));
    check(!DeviceIdentity::verify(id->publicKeyDer(), msg + "x", sig) && !DeviceIdentity::verify(id->publicKeyDer(), msg, sig + "x") &&
              !DeviceIdentity::verify(id->publicKeyDer(), msg, QByteArray()),
          QStringLiteral("changed data or signature does not check"));
    auto other = DeviceIdentity::createEphemeral(QStringLiteral("other"));
    check(other && other->pin() != id->pin() && !DeviceIdentity::verify(other->publicKeyDer(), msg, sig),
          QStringLiteral("another device's key does not accept the signature"));
    check(!DeviceIdentity::verify(QByteArray("garbage"), msg, sig), QStringLiteral("a public key that is garbage does not check"));
    // the key entry gone: a new device
    ks->remove(DeviceIdentity::keyEntry());
    auto fresh = DeviceIdentity::loadOrCreate(dir, *ks, &err);
    check(fresh && fresh->pin() != id->pin(), QStringLiteral("a lost key makes a new device (paired again)"), err);
    // a damaged key is never replaced silently
    ks->write(DeviceIdentity::keyEntry(), "not a key");
    auto bad = DeviceIdentity::loadOrCreate(dir, *ks, &err, &st);
    QByteArray left;
    ks->read(DeviceIdentity::keyEntry(), &left);
    check(!bad && !err.isEmpty() && left == "not a key", QStringLiteral("a damaged key entry is reported, not replaced"), err);
    // no key store
    NoStore none;
    const QString dir2 = tmp + QStringLiteral("/id2");
    auto n = DeviceIdentity::loadOrCreate(dir2, none, &err, &st);
    check(!n && st == KeyStore::Status::NoBackend && !QFileInfo::exists(dir2 + QStringLiteral("/device.json")),
          QStringLiteral("no key store: no identity, Live Sync says why, no file written"), err);
    // the program's own (the file key store of the test builds) survives a restart
    QString e2;
    auto p1 = DeviceIdentity::forThisDevice(&e2);
    auto p2 = DeviceIdentity::forThisDevice(&e2);
    check(p1 && p2 && p1->pin() == p2->pin(), QStringLiteral("QB_TEST_KEYSTORE=file: keeps the identity between runs"), e2);
}

// ---------------------------------------------------------------------------------------------------------------------
static QByteArray raw(quint32 len, quint8 kind, const QByteArray &payload)
{
    QByteArray b;
    for (int i = 3; i >= 0; --i)
        b.append(char((len >> (8 * i)) & 0xff));
    b.append(char(kind));
    return b + payload;
}

static void testFrames()
{
    {
        FrameReader r;
        const Frame j = Frame::json({{QStringLiteral("t"), QStringLiteral("X")}, {QStringLiteral("n"), 5}});
        const Frame d = Frame::data(QByteArray(16, 'T'), 0x0102030405060708ULL, "payload");
        const QByteArray bytes = encodeFrame(j) + encodeFrame(d);
        QList<Frame> got;
        for (char c : bytes) // one byte at a time
            got += r.feed(QByteArray(1, c));
        QByteArray t, body;
        quint64 off = 0;
        check(got.size() == 2 && got[0].object().value(QStringLiteral("n")).toInt() == 5 && got[1].dataParts(&t, &off, &body) && t == QByteArray(16, 'T') &&
                  off == 0x0102030405060708ULL && body == "payload" && r.error() == FrameError::None,
              QStringLiteral("frames come out whole, also from single bytes"));
    }
    {
        FrameReader r;
        r.feed(raw(1024 * 1024 + 2, 0, QByteArray())); // only the header: refused before any payload arrives
        check(r.error() == FrameError::TooBig && frameErrorCode(r.error()) == QLatin1String("TOO_BIG"), QStringLiteral("a JSON frame over 1 MiB is refused from its header"));
        check(r.feed(encodeFrame(Frame::json({}))).isEmpty(), QStringLiteral("after an error nothing more is read"));
        FrameReader r2;
        r2.feed(raw(1024 * 1024 + 1, 0, QByteArray()));
        check(r2.error() == FrameError::None && r2.pending() == 5, QStringLiteral("a JSON frame of exactly 1 MiB is allowed (waits for its bytes)"));
        FrameReader r3;
        r3.feed(raw(1 + 24 + 256 * 1024 + 1, 1, QByteArray()));
        check(r3.error() == FrameError::TooBig, QStringLiteral("a data frame with more than 256 KiB is refused"));
        FrameReader r4;
        r4.feed(raw(1 + 24 + 256 * 1024, 1, QByteArray()));
        check(r4.error() == FrameError::None, QStringLiteral("a data frame with exactly 256 KiB is allowed"));
    }
    {
        QByteArray big(limits::kMaxJson + 1, 'x');
        Frame f;
        f.kind = Frame::Json;
        f.payload = big;
        check(encodeFrame(f).isEmpty(), QStringLiteral("a frame that is too big is not even encoded"));
        check(encodeFrame(Frame::data(QByteArray(16, 0), 0, QByteArray(limits::kMaxDataBytes + 1, 'x'))).isEmpty(), QStringLiteral("a data frame over 256 KiB is not encoded"));
        Frame u;
        u.kind = 7;
        check(encodeFrame(u).isEmpty(), QStringLiteral("an unknown kind is not encoded"));
    }
    const auto code = [](const QByteArray &bytes) {
        FrameReader r;
        r.feed(bytes);
        return frameErrorCode(r.error());
    };
    check(code(raw(5, 2, QByteArray(4, 'a'))) == QLatin1String("BAD_FRAME"), QStringLiteral("an unknown frame kind closes with BAD_FRAME"));
    check(code(raw(0, 0, QByteArray())) == QLatin1String("BAD_FRAME"), QStringLiteral("a frame of length 0 closes with BAD_FRAME"));
    check(code(raw(1 + 5, 0, "{bad}")) == QLatin1String("BAD_FRAME"), QStringLiteral("JSON that does not parse closes with BAD_FRAME"));
    check(code(raw(1 + 2, 0, "[]")) == QLatin1String("BAD_FRAME"), QStringLiteral("JSON that is not an object closes with BAD_FRAME"));
    check(code(raw(1 + 10, 1, QByteArray(10, 'a'))) == QLatin1String("BAD_FRAME"), QStringLiteral("a data frame without its header closes with BAD_FRAME"));
    check(code(raw(1 + 2, 0, "{}") + QByteArray()) == QString(), QStringLiteral("an empty object is a good frame"));
    {
        qint64 now = 1000;
        FrameReader r([&] { return now; });
        QByteArray many;
        for (int i = 0; i < 200; ++i)
            many += encodeFrame(Frame::json({}));
        const int n = r.feed(many).size();
        check(n == 200 && r.error() == FrameError::None, QStringLiteral("200 control frames in a second are allowed"));
        r.feed(encodeFrame(Frame::json({})));
        check(r.error() == FrameError::RateLimit && frameErrorCode(r.error()) == QLatin1String("RATE_LIMIT"), QStringLiteral("the 201st closes with RATE_LIMIT"));
        qint64 t2 = 5000;
        FrameReader q([&] { return t2; });
        int total = 0;
        for (int round = 0; round < 5; ++round) {
            QByteArray b;
            for (int i = 0; i < 100; ++i)
                b += encodeFrame(Frame::json({}));
            total += q.feed(b).size();
            t2 += 600;
        }
        check(total == 500 && q.error() == FrameError::None, QStringLiteral("a steady 166 frames per second is no flood (the second slides)"));
    }
    {
        // data frames are not counted as control frames
        FrameReader r;
        QByteArray many;
        for (int i = 0; i < 300; ++i)
            many += encodeFrame(Frame::data(QByteArray(16, 1), quint64(i), "x"));
        check(r.feed(many).size() == 300 && r.error() == FrameError::None, QStringLiteral("data frames do not count against the control limit"));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void testPeerStore(const QString &tmp)
{
    const QString dir = tmp + QStringLiteral("/ps");
    QByteArray pinA(32, 'a'), pinB(32, 'b');
    {
        PeerStore s(dir);
        check(s.peers().isEmpty() && !s.contains(pinA), QStringLiteral("a new peer store is empty"));
        PeerRecord r;
        r.pin = pinA;
        r.name = QStringLiteral("Laptop");
        r.instanceId = QStringLiteral("0123456789abcdef");
        check(s.add(r) && s.contains(pinA), QStringLiteral("a peer is added"));
        PeerRecord bad;
        bad.pin = QByteArray(5, 'x');
        check(!s.add(bad), QStringLiteral("a pin of the wrong size is refused"));
        r.pin = pinB;
        r.name = QStringLiteral("Tablet");
        s.add(r);
    }
    {
        PeerStore s(dir);
        PeerRecord got;
        check(s.peers().size() == 2 && s.find(pinA, &got) && got.name == QLatin1String("Laptop") && got.instanceId == QLatin1String("0123456789abcdef") &&
                  got.pairedAt > 0 && got.deviceId() == QString::fromLatin1(pinA.left(16).toHex()),
              QStringLiteral("peers are kept between runs"));
        check(s.remove(pinA) && !s.contains(pinA) && !s.remove(pinA), QStringLiteral("a peer is removed once"));
    }
    {
        PeerStore s(dir);
        check(s.peers().size() == 1 && s.contains(pinB), QStringLiteral("the removal is kept"));
        writeAll(dir + QStringLiteral("/peers.json"), "{ damaged");
        PeerStore t(dir);
        check(t.peers().isEmpty() && QFileInfo::exists(dir + QStringLiteral("/peers.json.bad")),
              QStringLiteral("a damaged list trusts nobody and is kept aside"));
    }
    {
        writeAll(tmp + QStringLiteral("/psfile"), "x");
        PeerStore s(tmp + QStringLiteral("/psfile/sub"));
        PeerRecord r;
        r.pin = pinA;
        QString err;
        check(!s.add(r, &err) && !s.contains(pinA) && !err.isEmpty(), QStringLiteral("a list that cannot be written is not changed"), err);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void testMemLink()
{
    const QByteArray pa(32, 'A'), pb(32, 'B');
    auto pr = MemLink::createPair(pa, pb);
    QList<Frame> gotA, gotB;
    QObject::connect(pr.a, &Link::frameReceived, [&](const Frame &f) { gotA.append(f); });
    QObject::connect(pr.b, &Link::frameReceived, [&](const Frame &f) { gotB.append(f); });
    check(pr.a->peer().pin == pb && pr.b->peer().pin == pa && pr.a->state() == Link::State::Ready, QStringLiteral("each end knows the other's pin"));
    pr.a->sendJson({{QStringLiteral("t"), QStringLiteral("P")}});
    pr.b->sendData(QByteArray(16, 'x'), 7, "abc");
    check(waitFor([&] { return gotB.size() == 1 && gotA.size() == 1; }) && gotB[0].object().value(QStringLiteral("t")).toString() == QLatin1String("P"),
          QStringLiteral("frames arrive in both directions"));
    pr.a->setHeld(true);
    pr.a->sendJson({});
    pause(30);
    check(gotB.size() == 1 && pr.a->bytesWaiting() > 0, QStringLiteral("a partition keeps the frames back"));
    pr.a->setHeld(false);
    check(waitFor([&] { return gotB.size() == 2; }), QStringLiteral("and releases them in order"));
    pr.a->setDropping(true);
    pr.a->sendJson({});
    pause(30);
    check(gotB.size() == 2, QStringLiteral("dropped frames vanish"));
    pr.a->setDropping(false);
    Frame huge;
    huge.payload = QByteArray(limits::kMaxJson + 1, 'x');
    check(!pr.a->send(huge), QStringLiteral("a frame that is too big is refused at once"));
    // a flood closes both ends with RATE_LIMIT
    QString codeB;
    QObject::connect(pr.b, &Link::closed, [&](const QString &c) { codeB = c; });
    for (int i = 0; i < 260; ++i)
        pr.a->sendJson({});
    check(waitFor([&] { return pr.b->state() == Link::State::Closed; }) && codeB == QLatin1String("RATE_LIMIT") && pr.a->state() == Link::State::Closed,
          QStringLiteral("over 200 control frames a second close the link with RATE_LIMIT"), codeB);
    check(!pr.a->sendJson({}), QStringLiteral("a closed link sends nothing"));
    delete pr.a;
    delete pr.b;
}

// ---------------------------------------------------------------------------------------------------------------------
struct TlsEnd {
    TlsLink *link = nullptr;
    QList<Frame> frames;
    QString closedCode;
    bool closed = false;
    bool everReady = false;
    void watch(TlsLink *l)
    {
        link = l;
        QObject::connect(l, &Link::frameReceived, [this](const Frame &f) { frames.append(f); });
        QObject::connect(l, &Link::closed, [this](const QString &c) {
            closed = true;
            closedCode = c;
        });
        QObject::connect(l, &Link::stateChanged, [this](Link::State s) {
            if (s == Link::State::Ready)
                everReady = true;
        });
        if (l->state() == Link::State::Ready)
            everReady = true;
    }
    bool ready() const { return link && link->state() == Link::State::Ready; }
};

struct Rig {
    std::unique_ptr<DeviceIdentity> srv = DeviceIdentity::createEphemeral(QStringLiteral("Server device"));
    std::unique_ptr<DeviceIdentity> cli = DeviceIdentity::createEphemeral(QStringLiteral("Client device"));
};

// a server link that came in (the first one)
static bool nextServerLink(TlsServer &server, TlsEnd &end, int ms = 5000)
{
    TlsLink *got = nullptr;
    const auto c = QObject::connect(&server, &TlsServer::newLink, [&](TlsLink *l) {
        got = l;
        end.watch(l);
    });
    const bool ok = waitFor([&] { return got != nullptr; }, ms);
    QObject::disconnect(c);
    return ok;
}

static void testTls(const QString &tmp)
{
    Rig rig;
    PeerStore srvPeers(tmp + QStringLiteral("/t-srv")), cliPeers(tmp + QStringLiteral("/t-cli"));
    PeerRecord rc;
    rc.pin = rig.cli->pin();
    rc.name = QStringLiteral("c");
    srvPeers.add(rc);
    PeerRecord rs;
    rs.pin = rig.srv->pin();
    rs.name = QStringLiteral("s");
    cliPeers.add(rs);

    // ---- paired devices, both ways ----
    {
        TlsServer server(rig.srv.get(), TrustPolicy::paired(&srvPeers));
        check(server.listen(QHostAddress::LocalHost, 0) && server.port() != 0, QStringLiteral("the listener listens (a free port when the preferred one is taken)"));
        TlsEnd s, c;
        std::unique_ptr<TlsLink> cl(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::paired(&cliPeers)));
        c.watch(cl.get());
        const bool got = nextServerLink(server, s);
        check(got && waitFor([&] { return s.ready() && c.ready(); }), QStringLiteral("two paired devices reach Ready with TLS 1.3"));
        if (s.ready() && c.ready()) {
            check(s.link->peer().pin == rig.cli->pin() && c.link->peer().pin == rig.srv->pin() && s.link->peer().trust == PeerInfo::Trust::Paired &&
                      c.link->peer().trust == PeerInfo::Trust::Paired && s.link->peer().deviceId == rig.cli->deviceId(),
                  QStringLiteral("each side knows the other's pin and device id, from the key it authenticated with"));
            check(c.link->protocol() == QLatin1String("TLS v1.3") && s.link->protocol() == QLatin1String("TLS v1.3"), QStringLiteral("the connection is TLS 1.3"));
            c.link->sendJson({{QStringLiteral("t"), QStringLiteral("PING")}});
            s.link->sendJson({{QStringLiteral("t"), QStringLiteral("PONG")}});
            const QByteArray big(300 * 1024, 'q');
            c.link->sendData(QByteArray(16, 'T'), 99, big.left(limits::kMaxDataBytes));
            check(waitFor([&] { return s.frames.size() >= 2 && c.frames.size() >= 1; }) && s.frames[0].object().value(QStringLiteral("t")).toString() == QLatin1String("PING") &&
                      c.frames[0].object().value(QStringLiteral("t")).toString() == QLatin1String("PONG"),
                  QStringLiteral("frames go both ways"));
            QByteArray body;
            check(s.frames.size() >= 2 && s.frames[1].dataParts(nullptr, nullptr, &body) && body.size() == limits::kMaxDataBytes, QStringLiteral("a data frame of 256 KiB arrives whole"));
            QJsonObject bigObj;
            bigObj[QStringLiteral("x")] = QString(900 * 1024, QLatin1Char('z'));
            c.link->sendJson(bigObj);
            check(waitFor([&] { return s.frames.size() >= 3; }) && s.frames[2].object().value(QStringLiteral("x")).toString().size() == 900 * 1024, QStringLiteral("a JSON frame of 900 KiB arrives whole"));
            QJsonObject tooBig;
            tooBig[QStringLiteral("x")] = QString(limits::kMaxJson, QLatin1Char('z'));
            check(!c.link->sendJson(tooBig) && c.ready(), QStringLiteral("a frame over the limit is not sent, the link stays"));
        }
        // the writer removes the peer: the next connection is "not paired"
        s.link->close();
        waitFor([&] { return c.closed; });
        srvPeers.remove(rig.cli->pin());
        TlsEnd s2, c2;
        std::unique_ptr<TlsLink> cl2(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::paired(&cliPeers)));
        c2.watch(cl2.get());
        nextServerLink(server, s2);
        check(waitFor([&] { return c2.closed && s2.closed; }) && c2.closedCode == QLatin1String("NOT_PAIRED") && s2.closedCode == QLatin1String("NOT_PAIRED") &&
                  !c2.everReady && !s2.everReady && c2.frames.isEmpty() && s2.frames.isEmpty(),
              QStringLiteral("a removed device meets NOT_PAIRED and learns nothing else"), c2.closedCode + " / " + s2.closedCode);
        srvPeers.add(rc);
    }
    // ---- the wrong pin, a server that is not the one expected ----
    {
        TlsServer server(rig.srv.get(), TrustPolicy::paired(&srvPeers));
        server.listen(QHostAddress::LocalHost, 0);
        auto stranger = DeviceIdentity::createEphemeral(QStringLiteral("x"));
        TlsEnd s, c;
        std::unique_ptr<TlsLink> cl(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::pinned(stranger->pin())));
        c.watch(cl.get());
        nextServerLink(server, s);
        check(waitFor([&] { return c.closed; }) && c.closedCode == QLatin1String("IDENTITY_MISMATCH") && !c.everReady && c.frames.isEmpty(),
              QStringLiteral("a server whose key is not the pinned one is refused: IDENTITY_MISMATCH"), c.closedCode);
        check(waitFor([&] { return s.closed; }) && !s.everReady && s.frames.isEmpty(), QStringLiteral("and the server never reaches Ready"));
        // a client that does not know the server at all
        TlsEnd s3, c3;
        std::unique_ptr<TlsLink> cl3(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::paired(&cliPeers)));
        PeerStore empty(tmp + QStringLiteral("/t-empty"));
        std::unique_ptr<TlsLink> cl4(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::paired(&empty)));
        TlsEnd c4;
        c4.watch(cl4.get());
        check(waitFor([&] { return c4.closed; }) && c4.closedCode == QLatin1String("IDENTITY_MISMATCH") && !c4.everReady,
              QStringLiteral("a client refuses a server it has not paired with"), c4.closedCode);
    }
    // ---- an unknown key, window closed and window open ----
    {
        bool open = false;
        TlsServer server(rig.srv.get(), TrustPolicy::pairedOrPairing(&srvPeers, [&] { return open; }));
        server.listen(QHostAddress::LocalHost, 0);
        auto stranger = DeviceIdentity::createEphemeral(QStringLiteral("Stranger"));
        PeerStore sPeers(tmp + QStringLiteral("/t-str"));
        TlsEnd s, c;
        std::unique_ptr<TlsLink> cl(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), stranger.get(), TrustPolicy::pairing()));
        c.watch(cl.get());
        nextServerLink(server, s);
        check(waitFor([&] { return c.closed && s.closed; }) && c.closedCode == QLatin1String("NOT_PAIRED") && !c.everReady && !s.everReady && c.frames.isEmpty() && s.frames.isEmpty(),
              QStringLiteral("an unknown key with the pairing window closed gets NOT_PAIRED and nothing else"), c.closedCode);
        open = true;
        TlsEnd s2, c2;
        std::unique_ptr<TlsLink> cl2(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), stranger.get(), TrustPolicy::pairing()));
        c2.watch(cl2.get());
        nextServerLink(server, s2);
        check(waitFor([&] { return c2.ready() && s2.ready(); }) && s2.link->peer().trust == PeerInfo::Trust::Pairing && c2.link->peer().trust == PeerInfo::Trust::Pairing,
              QStringLiteral("with the window open the stranger is taken, marked Pairing on both ends"));
    }
    // ---- TLS 1.2 and plain bytes ----
    {
        TlsServer server(rig.srv.get(), TrustPolicy::pairing());
        server.listen(QHostAddress::LocalHost, 0);
        // a ClientHello that offers TLS 1.2 only (an ECDHE-ECDSA suite, no supported_versions extension)
        QByteArray body;
        body.append("\x03\x03", 2);
        body.append(QByteArray(32, 'r'));
        body.append(char(0));
        body.append("\x00\x02\xc0\x2c", 4);
        body.append("\x01\x00", 2);
        QByteArray ext;
        ext.append("\x00\x0a\x00\x04\x00\x02\x00\x17", 8);            // supported_groups: secp256r1
        ext.append("\x00\x0b\x00\x02\x01\x00", 6);                      // ec_point_formats
        ext.append("\x00\x0d\x00\x04\x00\x02\x04\x03", 8);            // signature_algorithms: ecdsa_secp256r1_sha256
        body.append(char(ext.size() >> 8)).append(char(ext.size() & 0xff)).append(ext);
        QByteArray hs;
        hs.append(char(1));
        hs.append(char(0)).append(char(body.size() >> 8)).append(char(body.size() & 0xff));
        hs.append(body);
        QByteArray rec;
        rec.append("\x16\x03\x01", 3);
        rec.append(char(hs.size() >> 8)).append(char(hs.size() & 0xff)).append(hs);
        QTcpSocket sock;
        sock.connectToHost(QStringLiteral("127.0.0.1"), server.port());
        TlsEnd s;
        const bool got = nextServerLink(server, s);
        sock.write(rec);
        check(got && waitFor([&] { return s.closed; }) && !s.everReady, QStringLiteral("a client that offers only TLS 1.2 is refused"), s.closedCode);
        check(s.closedCode == QLatin1String("TLS_ERROR") || s.closedCode == QLatin1String("HANDSHAKE_FAILED"), QStringLiteral("(closed with TLS_ERROR)"), s.closedCode);
        QTcpSocket junk;
        junk.connectToHost(QStringLiteral("127.0.0.1"), server.port());
        TlsEnd s2;
        nextServerLink(server, s2);
        junk.write(QByteArray(200, 'x'));
        check(waitFor([&] { return s2.closed; }) && !s2.everReady, QStringLiteral("bytes that are no TLS close the link"), s2.closedCode);
    }
    // ---- timeouts, a bad HELLO, frame errors over TLS ----
    {
        TlsServer server(rig.srv.get(), TrustPolicy::paired(&srvPeers));
        server.listen(QHostAddress::LocalHost, 0);
        {
            TlsEnd s, c;
            std::unique_ptr<TlsLink> cl(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::paired(&cliPeers)));
            cl->setSilent(true);
            c.watch(cl.get());
            const auto conn = QObject::connect(&server, &TlsServer::newLink, [](TlsLink *l) { l->setTimeouts(5000, 300); });
            nextServerLink(server, s);
            QObject::disconnect(conn);
            check(waitFor([&] { return s.closed; }, 4000) && s.closedCode == QLatin1String("HANDSHAKE_TIMEOUT") && !s.everReady,
                  QStringLiteral("a peer that sends no HELLO is closed (HANDSHAKE_TIMEOUT)"), s.closedCode);
        }
        {
            TlsEnd s, c;
            std::unique_ptr<TlsLink> cl(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::paired(&cliPeers)));
            cl->setSilent(true);
            c.watch(cl.get());
            nextServerLink(server, s);
            waitFor([&] { return cl->protocol() == QLatin1String("TLS v1.3"); });
            cl->sendBytesForTest(encodeFrame(Frame::json({{QStringLiteral("t"), QStringLiteral("HELLO")}, {QStringLiteral("v"), 1}, {QStringLiteral("id"), QStringLiteral("00000000000000000000000000000000")}})));
            check(waitFor([&] { return s.closed; }) && s.closedCode == QLatin1String("IDENTITY_MISMATCH") && !s.everReady,
                  QStringLiteral("a HELLO with another device id than the key's is refused"), s.closedCode);
        }
        const auto frameCase = [&](const QByteArray &bytes, const QString &want, const QString &label) {
            TlsEnd s, c;
            std::unique_ptr<TlsLink> cl(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), rig.cli.get(), TrustPolicy::paired(&cliPeers)));
            c.watch(cl.get());
            nextServerLink(server, s);
            waitFor([&] { return s.ready() && c.ready(); });
            cl->sendBytesForTest(bytes);
            check(waitFor([&] { return s.closed; }) && s.closedCode == want, label, s.closedCode);
        };
        frameCase(raw(3, 9, "ab"), QStringLiteral("BAD_FRAME"), QStringLiteral("over TLS: an unknown kind closes with BAD_FRAME"));
        frameCase(raw(1 + 6, 0, "{nojs}"), QStringLiteral("BAD_FRAME"), QStringLiteral("over TLS: JSON that does not parse closes with BAD_FRAME"));
        frameCase(raw(2 * 1024 * 1024, 0, QByteArray()), QStringLiteral("TOO_BIG"), QStringLiteral("over TLS: a 2 MiB JSON frame is refused (TOO_BIG)"));
        {
            QByteArray flood;
            for (int i = 0; i < 400; ++i)
                flood += encodeFrame(Frame::json({}));
            frameCase(flood, QStringLiteral("RATE_LIMIT"), QStringLiteral("over TLS: 400 control frames at once close with RATE_LIMIT"));
        }
    }
    // ---- the listener's limits ----
    {
        TlsServer server(rig.srv.get(), TrustPolicy::pairing());
        qint64 fake = 1000;
        server.setClock([&] { return fake; });
        server.listen(QHostAddress::LocalHost, 0);
        QStringList refusals;
        QObject::connect(&server, &TlsServer::refused, [&](const QString &, const QString &c) { refusals << c; });
        std::vector<std::unique_ptr<QTcpSocket>> socks;
        for (int i = 0; i < 4; ++i) {
            socks.push_back(std::make_unique<QTcpSocket>());
            socks.back()->connectToHost(QStringLiteral("127.0.0.1"), server.port());
        }
        waitFor([&] { return server.linkCount() == 4; });
        socks.push_back(std::make_unique<QTcpSocket>());
        socks.back()->connectToHost(QStringLiteral("127.0.0.1"), server.port());
        check(waitFor([&] { return !refusals.isEmpty(); }) && refusals.first() == QLatin1String("TOO_MANY_UNAUTH") && server.linkCount() == 4,
              QStringLiteral("a fifth connection that has not shown itself is refused (4 unauthenticated at most)"), refusals.join(','));
        socks.clear();
        waitFor([&] { return server.linkCount() == 0; });
        // ten attempts a minute from one address
        refusals.clear();
        for (int i = 0; i < 12; ++i) {
            QTcpSocket s;
            s.connectToHost(QStringLiteral("127.0.0.1"), server.port());
            // (a refused connection is reset at once: the socket may never show "connected")
            waitFor([&] { return s.state() != QAbstractSocket::ConnectingState && s.state() != QAbstractSocket::HostLookupState; });
            pause(20);
            s.abort();
            waitFor([&] { return server.linkCount() == 0; }, 1000);
        }
        check(refusals.contains(QStringLiteral("RATE_LIMIT")), QStringLiteral("more than 10 attempts a minute from one address are refused (RATE_LIMIT)"), refusals.join(','));
        fake += 61000;
        refusals.clear();
        QTcpSocket s;
        s.connectToHost(QStringLiteral("127.0.0.1"), server.port());
        pause(200);
        check(refusals.isEmpty(), QStringLiteral("after a minute the address may connect again"));
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void testPairing(const QString &tmp)
{
    // the two functions
    {
        const QByteArray pH(32, 'H'), pC(32, 'C'), nH(32, '1'), nC(32, '2');
        const QString c1 = pairingCode(pH, pC, nH, nC);
        check(QRegularExpression(QStringLiteral("^\\d{3} \\d{3}$")).match(c1).hasMatch() && c1 == pairingCode(pH, pC, nH, nC), QStringLiteral("the code is six digits in two groups, the same for the same inputs"), c1);
        check(c1 != pairingCode(pC, pH, nH, nC) && c1 != pairingCode(pH, pC, nC, nH) && c1 != pairingCode(pH, QByteArray(32, 'X'), nH, nC),
              QStringLiteral("the code depends on both pins and both nonces"));
        check(pairingCommitment(pH, pC, nH).size() == 32 && pairingCommitment(pH, pC, nH) != pairingCommitment(pH, pC, nC), QStringLiteral("the commitment is 32 bytes and depends on the nonce"));
        // the spread of the codes: 2000 random draws show no value more than 5 times (a loose check against a constant or tiny range)
        QHash<QString, int> seen;
        for (int i = 0; i < 2000; ++i)
            seen[pairingCode(pH, pC, QByteArray::number(i).rightJustified(32, '0'), nC)]++;
        check(seen.size() > 1800, QStringLiteral("the codes spread over the range"), QString::number(seen.size()));
    }
    // the window
    {
        qint64 now = 100000;
        PairingWindow w([&] { return now; });
        check(!w.isOpen() && w.beginAttempt() == PairingWindow::Attempt::Closed, QStringLiteral("a window that is not open takes no attempt"));
        check(w.open() && w.isOpen() && w.remainingMs() == 120000, QStringLiteral("the window opens for 120 s"));
        now += 119999;
        check(w.isOpen(), QStringLiteral("it is open until the last millisecond"));
        now += 1;
        check(!w.isOpen() && w.remainingMs() == 0, QStringLiteral("and closed after 120 s"));
        w.open();
        check(w.beginAttempt() == PairingWindow::Attempt::Ok && w.beginAttempt() == PairingWindow::Attempt::Busy, QStringLiteral("one attempt at a time"));
        w.endAttempt(false);
        check(w.beginAttempt() == PairingWindow::Attempt::Ok, QStringLiteral("the next may start when the first is over"));
        w.endAttempt(false);
        check(w.beginAttempt() == PairingWindow::Attempt::Ok, QStringLiteral("third attempt"));
        w.endAttempt(false);
        check(w.beginAttempt() == PairingWindow::Attempt::Limit, QStringLiteral("a fourth attempt in one window is refused"));
        w.close();
        w.open();
        w.beginAttempt();
        w.endAttempt(true);
        check(!w.isOpen(), QStringLiteral("a mismatch closes the window"));
        // five windows in ten minutes
        PairingWindow v([&] { return now; });
        int opened = 0;
        for (int i = 0; i < 7; ++i) {
            QString why;
            if (v.open(&why))
                ++opened;
            v.close();
            now += 1000;
        }
        check(opened == 5, QStringLiteral("only five windows can be opened in ten minutes"), QString::number(opened));
        now += 10 * 60 * 1000;
        check(v.open(), QStringLiteral("and the limit passes with the ten minutes"));
    }
    auto hostId = DeviceIdentity::createEphemeral(QStringLiteral("Host machine"));
    auto guestId = DeviceIdentity::createEphemeral(QStringLiteral("Guest machine"));
    // over the fake network
    {
        PeerStore hp(tmp + QStringLiteral("/p-h")), gp(tmp + QStringLiteral("/p-g"));
        PairingWindow win;
        win.open();
        auto pr = MemLink::createPair(hostId->pin(), guestId->pin()); // a = host end, b = guest end
        pr.a->setPeerTrust(PeerInfo::Trust::Pairing);
        pr.b->setPeerTrust(PeerInfo::Trust::Pairing);
        PairingSession host(PairingSession::Role::Host, pr.a, hostId.get(), &hp, &win);
        PairingSession guest(PairingSession::Role::Guest, pr.b, guestId.get(), &gp);
        bool hf = false, gf = false, hok = false, gok = false;
        QObject::connect(&host, &PairingSession::finished, [&](bool ok, const QString &) { hf = true; hok = ok; });
        QObject::connect(&guest, &PairingSession::finished, [&](bool ok, const QString &) { gf = true; gok = ok; });
        check(waitFor([&] { return !host.code().isEmpty() && !guest.code().isEmpty(); }) && host.code() == guest.code(), QStringLiteral("both devices show the same code"), host.code() + " / " + guest.code());
        check(host.peerName() == QLatin1String("Guest machine") && guest.peerName() == QLatin1String("Host machine"), QStringLiteral("the names were told inside the exchange"));
        check(hp.peers().isEmpty() && gp.peers().isEmpty(), QStringLiteral("nothing is pinned before the writers answer"));
        guest.confirm(true);
        pause(50);
        check(hp.peers().isEmpty() && gp.peers().isEmpty() && !hf && !gf, QStringLiteral("one writer's yes pins nothing"));
        host.confirm(true);
        check(waitFor([&] { return hf && gf; }) && hok && gok, QStringLiteral("both yes: the pairing finishes"));
        PeerRecord r1, r2;
        check(hp.find(guestId->pin(), &r1) && gp.find(hostId->pin(), &r2) && r1.name == QLatin1String("Guest machine") && r2.name == QLatin1String("Host machine") &&
                  r1.instanceId == guestId->instanceId() && r2.instanceId == hostId->instanceId(),
              QStringLiteral("each side pinned the other's key with name and instance id"));
        check(waitFor([&] { return pr.a->state() == Link::State::Closed; }), QStringLiteral("the link closes after the pairing"));
        delete pr.a;
        delete pr.b;
    }
    // "They Differ"
    {
        PeerStore hp(tmp + QStringLiteral("/p-h2")), gp(tmp + QStringLiteral("/p-g2"));
        PairingWindow win;
        win.open();
        auto pr = MemLink::createPair(hostId->pin(), guestId->pin());
        pr.a->setPeerTrust(PeerInfo::Trust::Pairing);
        pr.b->setPeerTrust(PeerInfo::Trust::Pairing);
        PairingSession host(PairingSession::Role::Host, pr.a, hostId.get(), &hp, &win);
        PairingSession guest(PairingSession::Role::Guest, pr.b, guestId.get(), &gp);
        QString hreason, greason;
        bool hok = true, gok = true;
        QObject::connect(&host, &PairingSession::finished, [&](bool ok, const QString &r) { hok = ok; hreason = r; });
        QObject::connect(&guest, &PairingSession::finished, [&](bool ok, const QString &r) { gok = ok; greason = r; });
        waitFor([&] { return !host.code().isEmpty() && !guest.code().isEmpty(); });
        guest.confirm(false);
        host.confirm(true);
        check(waitFor([&] { return !hreason.isEmpty() && !greason.isEmpty(); }) && !hok && !gok && hreason == QLatin1String("MISMATCH") && hp.peers().isEmpty() && gp.peers().isEmpty(),
              QStringLiteral("\"They Differ\" on one side: nothing is pinned on either"), hreason + "/" + greason);
        check(!win.isOpen(), QStringLiteral("and the host's window closes"));
        delete pr.a;
        delete pr.b;
    }
    // a man in the middle that only passes frames on: the guest's check of the commitment stops it
    {
        auto mitm1 = DeviceIdentity::createEphemeral(QStringLiteral("M1")); // faces the host
        auto mitm2 = DeviceIdentity::createEphemeral(QStringLiteral("M2")); // faces the guest
        PeerStore hp(tmp + QStringLiteral("/p-h3")), gp(tmp + QStringLiteral("/p-g3"));
        PairingWindow win;
        win.open();
        auto leg1 = MemLink::createPair(hostId->pin(), mitm1->pin()); // a: host, b: man's end
        auto leg2 = MemLink::createPair(mitm2->pin(), guestId->pin()); // a: man's end, b: guest
        for (MemLink *l : {leg1.a, leg1.b, leg2.a, leg2.b})
            l->setPeerTrust(PeerInfo::Trust::Pairing);
        QObject::connect(leg1.b, &Link::frameReceived, [&](const Frame &f) { leg2.a->send(f); });
        QObject::connect(leg2.a, &Link::frameReceived, [&](const Frame &f) { leg1.b->send(f); });
        PairingSession host(PairingSession::Role::Host, leg1.a, hostId.get(), &hp, &win);
        PairingSession guest(PairingSession::Role::Guest, leg2.b, guestId.get(), &gp);
        QString greason;
        QObject::connect(&guest, &PairingSession::finished, [&](bool, const QString &r) { greason = r; });
        check(waitFor([&] { return !greason.isEmpty(); }) && greason == QLatin1String("COMMITMENT") && guest.code().isEmpty() && gp.peers().isEmpty(),
              QStringLiteral("a man in the middle who passes the frames on is stopped: the guest's commitment check fails"), greason);
        delete leg1.a;
        delete leg1.b;
        delete leg2.a;
        delete leg2.b;
    }
    // a man in the middle who plays host to the guest and guest to the host, each with a key of his own: the codes differ
    {
        int equal = 0, shown = 0;
        for (int i = 0; i < 8; ++i) {
            auto mH = DeviceIdentity::createEphemeral(QStringLiteral("MH")); // his face to the guest
            auto mG = DeviceIdentity::createEphemeral(QStringLiteral("MG")); // his face to the host
            PeerStore hp(tmp + QStringLiteral("/p-hx%1").arg(i)), gp(tmp + QStringLiteral("/p-gx%1").arg(i));
            PeerStore m1(tmp + QStringLiteral("/p-m1%1").arg(i)), m2(tmp + QStringLiteral("/p-m2%1").arg(i));
            PairingWindow w1, w2;
            w1.open();
            w2.open();
            auto leg1 = MemLink::createPair(hostId->pin(), mG->pin());
            auto leg2 = MemLink::createPair(mH->pin(), guestId->pin());
            for (MemLink *l : {leg1.a, leg1.b, leg2.a, leg2.b})
                l->setPeerTrust(PeerInfo::Trust::Pairing);
            PairingSession host(PairingSession::Role::Host, leg1.a, hostId.get(), &hp, &w1);
            PairingSession manAsGuest(PairingSession::Role::Guest, leg1.b, mG.get(), &m1);
            PairingSession manAsHost(PairingSession::Role::Host, leg2.a, mH.get(), &m2, &w2);
            PairingSession guest(PairingSession::Role::Guest, leg2.b, guestId.get(), &gp);
            if (waitFor([&] { return !host.code().isEmpty() && !guest.code().isEmpty(); }, 3000)) {
                ++shown;
                if (host.code() == guest.code())
                    ++equal;
            }
            delete leg1.a;
            delete leg1.b;
            delete leg2.a;
            delete leg2.b;
        }
        check(shown == 8 && equal == 0, QStringLiteral("a man in the middle with a key of his own on each side: the two devices show different codes (8 tries)"),
              QStringLiteral("shown %1, equal %2").arg(shown).arg(equal));
    }
    // a host that opens another nonce than it committed to
    {
        PeerStore gp(tmp + QStringLiteral("/p-g4"));
        auto pr = MemLink::createPair(hostId->pin(), guestId->pin());
        pr.a->setPeerTrust(PeerInfo::Trust::Pairing);
        pr.b->setPeerTrust(PeerInfo::Trust::Pairing);
        PairingSession guest(PairingSession::Role::Guest, pr.b, guestId.get(), &gp);
        QString reason;
        bool ok = true;
        QObject::connect(&guest, &PairingSession::finished, [&](bool o, const QString &r) { ok = o; reason = r; });
        bool nonceSeen = false;
        QObject::connect(pr.a, &Link::frameReceived, [&](const Frame &f) {
            if (f.object().value(QStringLiteral("t")).toString() == QLatin1String("PAIR_NONCE")) {
                nonceSeen = true;
                // the reveal does not fit the commitment (the guest's nonce is known to the host now: he picks his to suit)
                pr.a->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_OPEN")}, {QStringLiteral("n"), QString::fromLatin1(QByteArray(32, 'z').toHex())}});
            }
        });
        pr.a->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_COMMIT")}, {QStringLiteral("c"), QString::fromLatin1(QByteArray(32, 'c').toHex())},
                        {QStringLiteral("name"), QStringLiteral("Evil")}, {QStringLiteral("instance"), QStringLiteral("0123456789abcdef")}});
        check(waitFor([&] { return !reason.isEmpty(); }) && nonceSeen && !ok && reason == QLatin1String("COMMITMENT") && guest.code().isEmpty() && gp.peers().isEmpty(),
              QStringLiteral("a reveal that does not fit the commitment ends the pairing without a code"), reason);
        delete pr.a;
        delete pr.b;
    }
    // messages out of order, a closed window, two at once, the attempts
    {
        PeerStore hp(tmp + QStringLiteral("/p-h5"));
        PairingWindow win;
        win.open();
        auto pr = MemLink::createPair(hostId->pin(), guestId->pin());
        pr.a->setPeerTrust(PeerInfo::Trust::Pairing);
        PairingSession host(PairingSession::Role::Host, pr.a, hostId.get(), &hp, &win);
        QString reason;
        QObject::connect(&host, &PairingSession::finished, [&](bool, const QString &r) { reason = r; });
        pr.b->sendJson({{QStringLiteral("t"), QStringLiteral("PAIR_CONFIRM")}, {QStringLiteral("ok"), true}});
        check(waitFor([&] { return !reason.isEmpty(); }) && reason == QLatin1String("BAD_FRAME") && hp.peers().isEmpty(), QStringLiteral("a confirmation before any code was shown ends the pairing"), reason);
        delete pr.a;
        delete pr.b;
    }
    {
        PeerStore hp(tmp + QStringLiteral("/p-h6")), gp(tmp + QStringLiteral("/p-g6"));
        PairingWindow win; // never opened
        auto pr = MemLink::createPair(hostId->pin(), guestId->pin());
        pr.a->setPeerTrust(PeerInfo::Trust::Pairing);
        pr.b->setPeerTrust(PeerInfo::Trust::Pairing);
        PairingSession host(PairingSession::Role::Host, pr.a, hostId.get(), &hp, &win);
        PairingSession guest(PairingSession::Role::Guest, pr.b, guestId.get(), &gp);
        QString reason;
        QObject::connect(&guest, &PairingSession::finished, [&](bool, const QString &r) { reason = r; });
        check(waitFor([&] { return !reason.isEmpty(); }) && reason == QLatin1String("PAIRING_CLOSED") && host.code().isEmpty() && guest.code().isEmpty(),
              QStringLiteral("a pairing request with the window closed is refused and learns nothing"), reason);
        delete pr.a;
        delete pr.b;
    }
    {
        PeerStore hp(tmp + QStringLiteral("/p-h7"));
        PairingWindow win;
        win.open();
        auto p1 = MemLink::createPair(hostId->pin(), guestId->pin());
        auto p2 = MemLink::createPair(hostId->pin(), guestId->pin());
        for (MemLink *l : {p1.a, p1.b, p2.a, p2.b})
            l->setPeerTrust(PeerInfo::Trust::Pairing);
        PairingSession h1(PairingSession::Role::Host, p1.a, hostId.get(), &hp, &win);
        PairingSession h2(PairingSession::Role::Host, p2.a, hostId.get(), &hp, &win);
        PeerStore gp(tmp + QStringLiteral("/p-g7"));
        PairingSession g2(PairingSession::Role::Guest, p2.b, guestId.get(), &gp);
        QString reason2;
        QObject::connect(&g2, &PairingSession::finished, [&](bool, const QString &r) { reason2 = r; });
        check(waitFor([&] { return !reason2.isEmpty(); }) && reason2 == QLatin1String("PAIRING_BUSY"), QStringLiteral("a second request while one runs gets PAIRING_BUSY"), reason2);
        delete p1.a;
        delete p1.b;
        delete p2.a;
        delete p2.b;
    }
    // a pairing over real TLS, then a paired connection
    {
        PeerStore hp(tmp + QStringLiteral("/p-h8")), gp(tmp + QStringLiteral("/p-g8"));
        PairingWindow win;
        win.open();
        TlsServer server(hostId.get(), TrustPolicy::pairedOrPairing(&hp, [&] { return win.isOpen(); }));
        server.listen(QHostAddress::LocalHost, 0);
        std::unique_ptr<PairingSession> host;
        const auto hookConn = QObject::connect(&server, &TlsServer::newLink, [&](TlsLink *l) {
            if (!host)
                host = std::make_unique<PairingSession>(PairingSession::Role::Host, l, hostId.get(), &hp, &win);
        });
        std::unique_ptr<TlsLink> cl(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), guestId.get(), TrustPolicy::pairing()));
        PairingSession guest(PairingSession::Role::Guest, cl.get(), guestId.get(), &gp);
        bool gf = false, gok = false;
        QObject::connect(&guest, &PairingSession::finished, [&](bool ok, const QString &) { gf = true; gok = ok; });
        check(waitFor([&] { return host && !host->code().isEmpty() && !guest.code().isEmpty(); }) && host->code() == guest.code(), QStringLiteral("over TLS: both show the same code"));
        if (host && !host->code().isEmpty()) {
            host->confirm(true);
            guest.confirm(true);
        }
        check(waitFor([&] { return gf; }) && gok && hp.contains(guestId->pin()) && gp.contains(hostId->pin()), QStringLiteral("over TLS: both pinned each other"));
        pause(100);
        QObject::disconnect(hookConn);
        // from now on the pins are enough
        win.close();
        TlsEnd c2, s2;
        std::unique_ptr<TlsLink> cl2(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), guestId.get(), TrustPolicy::paired(&gp)));
        c2.watch(cl2.get());
        nextServerLink(server, s2);
        check(waitFor([&] { return c2.ready() && s2.ready(); }) && s2.link->peer().trust == PeerInfo::Trust::Paired, QStringLiteral("after the pairing a connection with the pins alone is Ready, with the window closed"));
        // the host removes the guest
        hp.remove(guestId->pin());
        TlsEnd c3, s3;
        std::unique_ptr<TlsLink> cl3(TlsLink::connectTo(QStringLiteral("127.0.0.1"), server.port(), guestId.get(), TrustPolicy::paired(&gp)));
        c3.watch(cl3.get());
        nextServerLink(server, s3);
        check(waitFor([&] { return c3.closed; }) && c3.closedCode == QLatin1String("NOT_PAIRED"), QStringLiteral("after the host removed the device it gets NOT_PAIRED"), c3.closedCode);
    }
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    QTemporaryDir keys;
    qputenv("QB_TEST_KEYSTORE", ("file:" + keys.path() + "/keys.json").toUtf8());
    qputenv("QB_TEST_WALLET_FAST", "1");
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication app(argc, argv);
    QTemporaryDir tmpDir;
    const QString tmp = tmpDir.path();

    testWallet(tmp);
    testFallback(tmp);
    testIdentity(tmp);
    testFrames();
    testPeerStore(tmp);
    testMemLink();
    if (DeviceIdentity::available()) {
        testTls(tmp);
        testPairing(tmp);
    }
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
