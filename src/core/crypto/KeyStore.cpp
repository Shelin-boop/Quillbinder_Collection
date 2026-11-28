// The system's key store (0.99.0): see KeyStore.h.
#include "core/crypto/KeyStore.h"

#include "AppInfo.h"
#include "core/TestSwitches.h"
#include "core/crypto/Wallet.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#ifdef QB_HAVE_QTKEYCHAIN
#include <qtkeychain/keychain.h>
#endif

namespace qb {
namespace crypto {

namespace {

QString say(const char *text) { return QCoreApplication::translate("qb::crypto", text); }

class MemoryStore : public KeyStore {
public:
    QString name() const override { return QStringLiteral("memory"); }
    Status write(const QString &entry, const QByteArray &secret, QString *) override
    {
        entries.insert(entry, secret);
        return Status::Ok;
    }
    Status read(const QString &entry, QByteArray *secret, QString *) override
    {
        const auto it = entries.constFind(entry);
        if (it == entries.constEnd())
            return Status::NotFound;
        if (secret)
            *secret = it.value();
        return Status::Ok;
    }
    Status remove(const QString &entry, QString *) override
    {
        return entries.remove(entry) ? Status::Ok : Status::NotFound;
    }

private:
    QHash<QString, QByteArray> entries;
};

// test builds: the entries in one JSON file (base64), re-read at each call, so that several processes share them
class FileStore : public KeyStore {
public:
    explicit FileStore(const QString &path) : path(path) {}
    QString name() const override { return QStringLiteral("file"); }
    Status write(const QString &entry, const QByteArray &secret, QString *error) override
    {
        QJsonObject o = load();
        o.insert(entry, QString::fromLatin1(secret.toBase64()));
        return save(o, error);
    }
    Status read(const QString &entry, QByteArray *secret, QString *) override
    {
        const QJsonObject o = load();
        if (!o.contains(entry))
            return Status::NotFound;
        if (secret)
            *secret = QByteArray::fromBase64(o.value(entry).toString().toLatin1());
        return Status::Ok;
    }
    Status remove(const QString &entry, QString *error) override
    {
        QJsonObject o = load();
        if (!o.contains(entry))
            return Status::NotFound;
        o.remove(entry);
        return save(o, error);
    }

private:
    QJsonObject load() const
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(f.readAll()).object() : QJsonObject();
    }
    Status save(const QJsonObject &o, QString *error) const
    {
        QSaveFile f(path);
        if (!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(o).toJson()) < 0 || !f.commit()) {
            if (error)
                *error = f.errorString();
            return Status::Failed;
        }
        return Status::Ok;
    }
    QString path;
};

// the system's store first; where the system has none, the Quillbinder wallet (Wallet.h)
class FallbackStore : public KeyStore {
public:
    explicit FallbackStore(std::unique_ptr<KeyStore> primary, Wallet *wallet) : primary(std::move(primary)), wallet(wallet) {}
    QString name() const override { return viaWallet ? say("The Quillbinder wallet") : primary->name(); }
    Status write(const QString &entry, const QByteArray &secret, QString *error) override
    {
        const Status s = primary->write(entry, secret, error);
        if (s != Status::NoBackend)
            return s;
        const Status w = unlocked(error);
        return w != Status::Ok ? w : map(wallet->write(entry, secret, error));
    }
    Status read(const QString &entry, QByteArray *secret, QString *error) override
    {
        const Status s = primary->read(entry, secret, error);
        if (s != Status::NoBackend)
            return s;
        // a wallet that does not exist has nothing to read: no question to the writer for that
        if (!wallet->exists())
            return Status::NotFound;
        const Status w = unlocked(error);
        return w != Status::Ok ? w : map(wallet->read(entry, secret, error));
    }
    Status remove(const QString &entry, QString *error) override
    {
        const Status s = primary->remove(entry, error);
        if (s != Status::NoBackend)
            return s;
        if (!wallet->exists())
            return Status::NotFound;
        const Status w = unlocked(error);
        return w != Status::Ok ? w : map(wallet->remove(entry, error));
    }

private:
    Status unlocked(QString *error)
    {
        if (!Wallet::available()) {
            if (error)
                *error = say("This build of Quillbinder has no access to the system's key store.");
            return Status::NoBackend;
        }
        const Wallet::Result r = wallet->ensureUnlocked(error);
        if (r == Wallet::Result::Ok) {
            viaWallet = true;
            return Status::Ok;
        }
        if (r == Wallet::Result::Locked && !Wallet::hasUnlockHandler())
            return Status::NoBackend; // nobody to ask: as before the wallet existed
        return r == Wallet::Result::Failed ? Status::Failed : Status::Denied;
    }
    static Status map(Wallet::Result r)
    {
        switch (r) {
        case Wallet::Result::Ok: return Status::Ok;
        case Wallet::Result::NotFound: return Status::NotFound;
        case Wallet::Result::Locked: return Status::Denied;
        case Wallet::Result::WrongPassphrase: return Status::Denied;
        default: return Status::Failed;
        }
    }
    std::unique_ptr<KeyStore> primary;
    Wallet *wallet;
    bool viaWallet = false;
};

#ifdef QB_HAVE_QTKEYCHAIN

class SystemStore : public KeyStore {
public:
    QString name() const override
    {
#if defined(Q_OS_WIN)
        return say("Windows Credential Manager");
#elif defined(Q_OS_ANDROID)
        return say("Android Keystore");
#else
        return say("KWallet or the Secret Service");
#endif
    }
    Status write(const QString &entry, const QByteArray &secret, QString *error) override
    {
        QKeychain::WritePasswordJob job(service());
        job.setKey(entry);
        // as text: the writer can find the passphrase again in the key store's own program (KWallet Manager, Seahorse)
        job.setTextData(QString::fromUtf8(secret));
        return run(job, error);
    }
    Status read(const QString &entry, QByteArray *secret, QString *error) override
    {
        QKeychain::ReadPasswordJob job(service());
        job.setKey(entry);
        const Status s = run(job, error);
        if (s == Status::Ok && secret)
            *secret = job.textData().toUtf8();
        return s;
    }
    Status remove(const QString &entry, QString *error) override
    {
        QKeychain::DeletePasswordJob job(service());
        job.setKey(entry);
        return run(job, error);
    }

private:
    // starts the job and waits for its answer
    static Status run(QKeychain::Job &job, QString *error)
    {
        job.setAutoDelete(false);
        job.setInsecureFallback(false); // never a plain file instead of the key store
        QEventLoop loop;
        bool done = false;
        QObject::connect(&job, &QKeychain::Job::finished, &loop, [&] {
            done = true;
            loop.quit();
        });
        job.start();
        if (!done)
            loop.exec(QEventLoop::ExcludeUserInputEvents);
        if (error)
            *error = job.error() == QKeychain::NoError ? QString() : job.errorString();
        switch (job.error()) {
        case QKeychain::NoError:
            return Status::Ok;
        case QKeychain::EntryNotFound:
            return Status::NotFound;
        case QKeychain::NoBackendAvailable:
        case QKeychain::NotImplemented:
            return Status::NoBackend;
        case QKeychain::AccessDenied:
        case QKeychain::AccessDeniedByUser:
            return Status::Denied;
        default:
            break;
        }
        return Status::Failed;
    }
};

#else

class SystemStore : public KeyStore {
public:
    QString name() const override { return say("no key store"); }
    Status write(const QString &, const QByteArray &, QString *error) override { return none(error); }
    Status read(const QString &, QByteArray *, QString *error) override { return none(error); }
    Status remove(const QString &, QString *error) override { return none(error); }

private:
    static Status none(QString *error)
    {
        if (error)
            *error = say("This build of Quillbinder has no access to the system's key store.");
        return Status::NoBackend;
    }
};

#endif

} // namespace

QString KeyStore::service() { return QStringLiteral(QB_APP_NAME); }

QString KeyStore::passphraseEntry(const QString &fingerprint)
{
    return QStringLiteral("OpenPGP passphrase %1").arg(fingerprint.toUpper());
}

KeyStore &KeyStore::system()
{
    static const std::unique_ptr<KeyStore> store = []() -> std::unique_ptr<KeyStore> {
        const QString test = QB_TEST_ENV("QB_TEST_KEYSTORE");
        if (test == QLatin1String("memory"))
            return std::make_unique<MemoryStore>();
        if (test.startsWith(QLatin1String("file:")))
            return std::make_unique<FileStore>(test.mid(5));
        return std::make_unique<FallbackStore>(std::make_unique<SystemStore>(), &Wallet::standard());
    }();
    return *store;
}

std::unique_ptr<KeyStore> KeyStore::withWallet(std::unique_ptr<KeyStore> primary, Wallet *wallet)
{
    return std::make_unique<FallbackStore>(std::move(primary), wallet);
}

std::unique_ptr<KeyStore> KeyStore::memory() { return std::make_unique<MemoryStore>(); }

} // namespace crypto
} // namespace qb
