// The system's key store (0.99.0): see KeyStore.h.
#include "core/crypto/KeyStore.h"

#include "AppInfo.h"
#include "core/TestSwitches.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QHash>

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
        if (QB_TEST_ENV("QB_TEST_KEYSTORE") == QLatin1String("memory"))
            return std::make_unique<MemoryStore>();
        return std::make_unique<SystemStore>();
    }();
    return *store;
}

std::unique_ptr<KeyStore> KeyStore::memory() { return std::make_unique<MemoryStore>(); }

} // namespace crypto
} // namespace qb
