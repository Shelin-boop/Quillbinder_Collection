#include "core/UpdateCheck.h"
#include "core/Addons.h"
#include "core/Paths.h"
#include "core/SafeSave.h"
#include "core/Signing.h"
#include "core/TestSwitches.h"

#include "AppInfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

namespace qb {
namespace updates {

QString feedAddress()
{
    const QString test = QB_TEST_ENV("QB_TEST_UPDATE_URL");   // the tests' own server
    if (!test.isEmpty())
        return test;
    return QStringLiteral("https://shelin-boop.github.io/Quillbinder_Collection/updates-1.json");
}

QString systemName()
{
#if defined(Q_OS_ANDROID)
    return QStringLiteral("android-arm64");
#elif defined(Q_OS_WIN)
    return QStringLiteral("windows-x64");
#else
    return QStringLiteral("linux-x86_64");
#endif
}

QDateTime now()
{
    const QDateTime test = QDateTime::fromString(QB_TEST_ENV("QB_TEST_UPDATE_NOW"), Qt::ISODate);
    return test.isValid() ? test.toUTC() : QDateTime::currentDateTimeUtc();
}

bool readFeed(const QByteArray &json, const QByteArray &signature, Feed *feed, QString *error)
{
    auto fail = [error](const QString &why) {
        if (error)
            *error = why;
        return false;
    };
    if (json.size() > 4 * 1024 * 1024)
        return fail(QStringLiteral("the update file is larger than such a file can be"));
    // the signature: 64 raw bytes, or their Base64 (with or without a line end)
    QByteArray sig = signature;
    if (sig.size() != 64)
        sig = QByteArray::fromBase64(signature.trimmed());
    bool trusted = false;
    if (sig.size() == 64)
        for (const QByteArray &key : signing::trustedKeys())
            trusted = trusted || signing::verify(json, sig, key);
    if (!trusted)
        return fail(QStringLiteral("the update file is not signed with the project's release key"));
    QJsonParseError pe;
    const QJsonObject o = QJsonDocument::fromJson(json, &pe).object();
    if (pe.error != QJsonParseError::NoError || o.value(QStringLiteral("format")).toInt() != 1)
        return fail(QStringLiteral("the update file is of a form this version does not know"));
    Feed f;
    f.format = 1;
    f.date = QDateTime::fromString(o.value(QStringLiteral("date")).toString(), Qt::ISODate).toUTC();
    f.systems = o.value(QStringLiteral("systems")).toObject();
    if (!f.date.isValid())
        return fail(QStringLiteral("the update file carries no date"));
    if (feed)
        *feed = f;
    return true;
}

Installed installedHere()
{
    Installed here;
    here.appVersion = QStringLiteral(QB_APP_VERSION);
    here.runtime = QStringLiteral(QB_RUNTIME_ID);
    const QString appDir = qEnvironmentVariable("APPDIR");
    here.fullAppImage = !appDir.isEmpty() && QDir(appDir + QLatin1String("/usr/share/quillbinder/full-addons")).exists();
    for (const addons::Installed &a : addons::installedAddons())
        here.addons.insert(a.manifest.id, a.manifest.version);
    return here;
}

static Download downloadOf(const QJsonValue &v)
{
    const QJsonObject o = v.toObject();
    Download d;
    d.url = o.value(QStringLiteral("url")).toString();
    d.name = o.value(QStringLiteral("name")).toString();
    d.sha256 = o.value(QStringLiteral("sha256")).toString().toLower();
    d.size = qint64(o.value(QStringLiteral("size")).toDouble());
    // only https addresses (the tests' server on this computer excepted), and a plain file name
    const bool local = d.url.startsWith(QLatin1String("http://127.0.0.1:")) && !QB_TEST_ENV("QB_TEST_UPDATE_URL").isEmpty();
    if (!d.url.startsWith(QLatin1String("https://")) && !local)
        d.url.clear();
    d.name = QFileInfo(d.name).fileName();
    if (d.name.isEmpty())
        d.name = QFileInfo(d.url.section(QLatin1Char('?'), 0, 0)).fileName();
    return d;
}

// whether a version after `installed`, up to `newest`, is in the list of security updates
static bool securityBetween(const QJsonValue &list, const QString &installed, const QString &newest)
{
    for (const QJsonValue &v : list.toArray()) {
        const QString s = v.toString();
        if (!s.isEmpty() && addons::compareVersions(s, installed) > 0 && addons::compareVersions(s, newest) <= 0)
            return true;
    }
    return false;
}

static QStringList linesOf(const QJsonValue &v)
{
    QStringList out;
    for (const QJsonValue &line : v.toArray())
        if (!line.toString().trimmed().isEmpty() && out.size() < 12)
            out << line.toString().trimmed().left(400);
    return out;
}

QList<Offer> offersFor(const Feed &feed, const QString &system, const Installed &here)
{
    QList<Offer> out;
    const QJsonObject sys = feed.systems.value(system).toObject();
    const QJsonObject prog = sys.value(QStringLiteral("program")).toObject();
    const QString newest = prog.value(QStringLiteral("version")).toString();
    QString programAfter = here.appVersion;   // the program version the add-ons are judged against
    if (!newest.isEmpty() && addons::compareVersions(newest, here.appVersion) > 0) {
        Offer o;
        o.id = QStringLiteral("program");
        o.name = QStringLiteral(QB_APP_NAME);
        o.installed = here.appVersion;
        o.version = newest;
        o.security = securityBetween(prog.value(QStringLiteral("security")), here.appVersion, newest);
        o.whatsNew = linesOf(prog.value(QStringLiteral("whatsNew")));
        const Download update = downloadOf(prog.value(QStringLiteral("update")));
        const Download image = downloadOf(prog.value(here.fullAppImage ? QStringLiteral("appimageFull") : QStringLiteral("appimage")));
        const Download other = downloadOf(prog.value(here.fullAppImage ? QStringLiteral("appimage") : QStringLiteral("appimageFull")));
        if (prog.value(QStringLiteral("runtime")).toString() == here.runtime && update.isValid()) {
            o.kind = Offer::Kind::ProgramUpdate;
            o.file = update;
        } else {
            o.kind = Offer::Kind::NewAppImage;
            o.file = image.isValid() ? image : other;
        }
        if (o.file.isValid()) {
            out << o;
            if (o.kind == Offer::Kind::ProgramUpdate)
                programAfter = newest;
        }
    }
    for (const QJsonValue &v : sys.value(QStringLiteral("addons")).toArray()) {
        const QJsonObject a = v.toObject();
        const QString id = a.value(QStringLiteral("id")).toString();
        const QString version = a.value(QStringLiteral("version")).toString();
        if (id.isEmpty() || !here.addons.contains(id) || addons::compareVersions(version, here.addons.value(id)) <= 0)
            continue;   // only add-ons that are installed here
        // it must fit this program: its runtime (when it names one) and the smallest program version
        const QString runtime = a.value(QStringLiteral("runtime")).toString();
        const QString minApp = a.value(QStringLiteral("minApp")).toString();
        if ((!runtime.isEmpty() && runtime != here.runtime) || (!minApp.isEmpty() && addons::compareVersions(minApp, programAfter) > 0))
            continue;
        Offer o;
        o.kind = Offer::Kind::Addon;
        o.id = id;
        o.name = a.value(QStringLiteral("name")).toString(id).left(120);
        o.installed = here.addons.value(id);
        o.version = version;
        o.security = securityBetween(a.value(QStringLiteral("security")), o.installed, version);
        o.whatsNew = linesOf(a.value(QStringLiteral("whatsNew")));
        o.file = downloadOf(a.value(QStringLiteral("file")));
        if (o.file.isValid())
            out << o;
    }
    return out;
}

// ---------------------------------------------------------------- state

QString State::file() { return dataDir() + QLatin1String("/updates-state.json"); }

State State::load()
{
    State s;
    QFile f(file());
    if (!f.open(QIODevice::ReadOnly))
        return s;
    const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
    s.enabled = o.value(QStringLiteral("enabled")).toBool(true);
    s.noteShown = o.value(QStringLiteral("noteShown")).toBool(false);
    s.lastCheck = QDateTime::fromString(o.value(QStringLiteral("lastCheck")).toString(), Qt::ISODate).toUTC();
    s.feedDate = QDateTime::fromString(o.value(QStringLiteral("feedDate")).toString(), Qt::ISODate).toUTC();
    s.unreachableTold = QDate::fromString(o.value(QStringLiteral("unreachableTold")).toString(), Qt::ISODate);
    const QJsonObject vs = o.value(QStringLiteral("versions")).toObject();
    for (auto it = vs.constBegin(); it != vs.constEnd(); ++it) {
        const QJsonObject v = it.value().toObject();
        PerVersion p;
        p.ignored = v.value(QStringLiteral("ignored")).toBool();
        p.delays = v.value(QStringLiteral("delays")).toInt();
        p.next = QDateTime::fromString(v.value(QStringLiteral("next")).toString(), Qt::ISODate).toUTC();
        p.off = v.value(QStringLiteral("off")).toBool();
        s.versions.insert(it.key(), p);
    }
    return s;
}

bool State::save() const
{
    QJsonObject o;
    o.insert(QStringLiteral("enabled"), enabled);
    o.insert(QStringLiteral("noteShown"), noteShown);
    if (lastCheck.isValid())
        o.insert(QStringLiteral("lastCheck"), lastCheck.toUTC().toString(Qt::ISODate));
    if (feedDate.isValid())
        o.insert(QStringLiteral("feedDate"), feedDate.toUTC().toString(Qt::ISODate));
    if (unreachableTold.isValid())
        o.insert(QStringLiteral("unreachableTold"), unreachableTold.toString(Qt::ISODate));
    QJsonObject vs;
    for (auto it = versions.constBegin(); it != versions.constEnd(); ++it) {
        QJsonObject v;
        v.insert(QStringLiteral("ignored"), it->ignored);
        v.insert(QStringLiteral("delays"), it->delays);
        if (it->next.isValid())
            v.insert(QStringLiteral("next"), it->next.toUTC().toString(Qt::ISODate));
        v.insert(QStringLiteral("off"), it->off);
        vs.insert(it.key(), v);
    }
    o.insert(QStringLiteral("versions"), vs);
    QDir().mkpath(dataDir());
    QSaveFile f(file());
    if (!f.open(QIODevice::WriteOnly))
        return false;
    safesave::write(f, QJsonDocument(o).toJson());
    return safesave::commit(f);
}

bool autoCheckDue(const State &s, const QDateTime &now)
{
    if (!s.enabled || !s.noteShown)
        return false;
    return !s.lastCheck.isValid() || s.lastCheck.secsTo(now) >= 24 * 3600 || s.lastCheck > now;
}

Shown toShow(const QList<Offer> &offers, const State &s, const QDateTime &now, bool manual)
{
    Shown shown;
    int fewest = kDelaySteps + 1;
    bool allAtEnd = true;
    for (const Offer &o : offers) {
        const State::PerVersion v = s.versions.value(o.key());
        if (!manual) {
            if (!o.security && (v.ignored || v.off))
                continue;
            if (v.next.isValid() && v.next > now)
                continue;
        }
        shown.offers << o;
        shown.security = shown.security || o.security;
        fewest = qMin(fewest, v.delays);
        allAtEnd = allAtEnd && v.delays >= kDelaySteps;
    }
    if (shown.offers.isEmpty())
        return shown;
    shown.lastReminder = !shown.security && allAtEnd;
    shown.delayDays = kDelayDays[qBound(0, fewest, kDelaySteps - 1)];
    return shown;
}

void applyDelay(const Shown &shown, State *s, const QDateTime &now)
{
    for (const Offer &o : shown.offers) {
        State::PerVersion &v = s->versions[o.key()];
        if (!o.security && !shown.security && v.delays >= kDelaySteps) {
            v.off = true;   // shown a last time: its reminders end
            continue;
        }
        v.next = now.addDays(shown.delayDays);
        v.delays = qMin(v.delays + 1, kDelaySteps);
    }
}

void applyIgnore(const Shown &shown, State *s)
{
    for (const Offer &o : shown.offers)
        if (!o.security)
            s->versions[o.key()].ignored = true;
}

} // namespace updates
} // namespace qb
