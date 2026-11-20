#pragma once
// 0.92.0: the check for updates, the part without network and windows (ui/UpdateChecker.* fetches and shows).
//
// One small signed file on the project's page lists the newest version of the program and of the project's own
// add-ons per system. The program fetches it with a plain GET (nothing about this computer is sent), checks its
// signature against the release key it carries, and compares on its own. Switched off, the program contacts
// nothing.
//
// The file (format 1), updates-1.json, with updates-1.json.sig (the release key's Ed25519 signature of the file's
// bytes: 64 raw bytes or their Base64):
//   {"format": 1, "date": "2026-11-20T12:00:00Z",
//    "systems": {"linux-x86_64": {
//        "program": {"version": "0.92.1", "label": "beta", "runtime": "u2404-qt6.12-r1",
//                    "security": ["0.92.1"],            // the versions that were security updates
//                    "whatsNew": ["...", "..."],
//                    "update":       {"url": "...", "name": "...", "size": 1, "sha256": "..."},   // program update
//                    "appimage":     {...}, "appimageFull": {...}},                               // new AppImages
//        "addons": [{"id": "media-playback", "name": "Media Playback", "version": "1.5", "minApp": "0.80.0",
//                    "runtime": "u2404-qt6.12-r1", "security": [], "file": {...}}]}}}
// A file whose signature does not fit is ignored, and so is one older than the newest one accepted before.
#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

namespace qb {
namespace updates {

struct Download {
    QString url, name, sha256;
    qint64 size = 0;
    bool isValid() const { return !url.isEmpty() && sha256.size() == 64 && size > 0; }
};

struct Offer {
    enum class Kind { ProgramUpdate, NewAppImage, Addon };
    Kind kind = Kind::Addon;
    QString id;          // "program", or the add-on's id
    QString name;        // as shown
    QString installed;   // the version here
    QString version;     // the newest one
    bool security = false;   // a version after the installed one, up to the newest, was a security update
    Download file;
    QStringList whatsNew;
    QString key() const { return id + QLatin1Char('/') + version; }
};

struct Feed {
    int format = 0;
    QDateTime date;       // UTC
    QJsonObject systems;
};

// The address built into the program (it must never change: every version with this function asks there).
QString feedAddress();
QString systemName();     // "linux-x86_64", "windows-x64", "android-arm64"
QDateTime now();          // UTC (the tests move the clock)

// True when the signature is a trusted release key's and the file is understood (*error says why not).
bool readFeed(const QByteArray &json, const QByteArray &signature, Feed *feed, QString *error);

struct Installed {
    QString appVersion;                 // QB_APP_VERSION
    QString runtime;                    // the runtime id of this program's libraries
    bool fullAppImage = false;          // the AppImage with the add-ons inside
    QHash<QString, QString> addons;     // installed add-ons: id -> version
};
Installed installedHere();
// What the file offers this installation: a newer program (as a program update when the runtime is the same and
// the file has one, else as a new AppImage) and newer versions of installed add-ons that fit the program.
QList<Offer> offersFor(const Feed &feed, const QString &system, const Installed &here);

// ---- what was decided per version, and when the program looked last (<data>/updates-state.json)
struct State {
    bool enabled = true;        // Help > Check for Updates at Startup
    bool noteShown = false;     // the first start's note was seen (no contact before it)
    QDateTime lastCheck;        // UTC: the last automatic look that got an answer
    QDateTime feedDate;         // UTC: the date of the newest file accepted
    QDate unreachableTold;      // the day the "could not look" note was last shown
    struct PerVersion {
        bool ignored = false;   // "Ignore This Version" (never for a security update)
        int delays = 0;         // how often "Delay" was chosen
        QDateTime next;         // UTC: no reminder before
        bool off = false;       // the reminders of this version have ended (never for a security update)
    };
    QHash<QString, PerVersion> versions;   // Offer::key() -> ...

    static QString file();
    static State load();
    bool save() const;
};

// The delays, in days: the first "Delay" waits a day, the next a week, then two and four weeks. A functional update
// is then shown one last time, and "Delay" ends its reminders; a security update keeps coming every four weeks.
constexpr int kDelayDays[] = {1, 7, 14, 28};
constexpr int kDelaySteps = 4;

// The automatic look: switched on, the note seen, and not within the last 24 hours.
bool autoCheckDue(const State &s, const QDateTime &now);

struct Shown {
    QList<Offer> offers;        // what the window lists (empty: no window)
    bool security = false;      // one of them is a security update: the window follows the security rules
    bool lastReminder = false;  // functional only, delayed four times: "Delay" now ends the reminders
    int delayDays = 0;          // what "Delay" waits this time
};
// manual (Check for Updates Now): everything there is, whatever was ignored or delayed.
Shown toShow(const QList<Offer> &offers, const State &s, const QDateTime &now, bool manual);
void applyDelay(const Shown &shown, State *s, const QDateTime &now);
void applyIgnore(const Shown &shown, State *s);

} // namespace updates
} // namespace qb
