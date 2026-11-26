#include "ui/Profiles.h"

#include "core/SafeSave.h"
#include "core/SettingsLayer.h"
#include "core/TestSwitches.h"
#include "text/TextUtil.h"
#include "ui/Layouts.h"
#include "ui/Theme.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStyleHints>
#include <QTemporaryFile>
#include <QUuid>

#include <algorithm>

namespace qb {
namespace profiles {

namespace {

constexpr const char *kSuffix = ".qbprofile";

// the keys a profile never holds (the person's, the computer's, once-only marks)
bool excluded(const QString &key)
{
    if (settingslayer::onceMarker(key))
        return true; // the first start, an update's one-time change, a migration (0.95.0: a switch must not do them again)
    static const QStringList prefixes = {QStringLiteral("profile/"), QStringLiteral("quillbinderOptions/"), QStringLiteral("layouts/"),
                                         QStringLiteral("themes/removed"), QStringLiteral("window/mobile")};
    for (const QString &p : prefixes)
        if (key.startsWith(p))
            return true;
    static const QStringList plain = {
        QStringLiteral("app/recent"),        QStringLiteral("app/lastDir"),        QStringLiteral("app/welcomeFolder"),
        QStringLiteral("app/welcomeShown"),  QStringLiteral("app/integrated"),     QStringLiteral("app/integrationDeclined"),
        QStringLiteral("app/authorAsked"),   QStringLiteral("app/forename"),       QStringLiteral("app/surname"),
        QStringLiteral("app/street"),        QStringLiteral("app/city"),           QStringLiteral("app/state"),
        QStringLiteral("app/zip"),           QStringLiteral("app/country"),        QStringLiteral("app/phone"),
        QStringLiteral("app/email"),         QStringLiteral("app/deviceName"),     QStringLiteral("app/notesPlace"),
        QStringLiteral("app/notesOwn"),      QStringLiteral("app/tutorialFolder")};
    if (plain.contains(key))
        return true;
    // once-only marks of updates end in four digits (split0910, themesOff0940, standard0212 ...)
    static const QRegularExpression mark(QStringLiteral("[0-9]{4}$"));
    if (mark.match(key).hasMatch())
        return true;
    const QString last = key.section(QLatin1Char('/'), -1).toLower();
    for (const char *w : {"accepted", "licen", "threads", "gpu", "graphic"})
        if (last.contains(QLatin1String(w)))
            return true;
    return false;
}

// the keys of the profile that stay as they are when a profile does not have them
bool keptWhenAbsent(const QString &key) { return key == QLatin1String("keyboard/layoutSet"); }

QString fileBase(const QString &name)
{
    QString b = text::safeFileName(name).trimmed();
    return b.isEmpty() ? QStringLiteral("profile") : b.left(80);
}

QVariantMap valuesOf(QSettings &s)
{
    QVariantMap out;
    for (const QString &k : s.allKeys())
        if (ownsKey(k))
            out.insert(k, s.value(k));
    return out;
}

// While a project's own settings are in use, QSettings() reads a merged copy of the program's and the project's values; a
// profile holds the program's own. Within the scope the project layer is put aside (and put on again after).
struct LayerPause
{
    bool did = false;
    LayerPause()
    {
        if (settingslayer::active() && !settingslayer::suspended() && !settingslayer::projectEditing()) {
            settingslayer::suspend();
            did = true;
        }
    }
    ~LayerPause()
    {
        if (did)
            settingslayer::resume();
    }
    LayerPause(const LayerPause &) = delete;
    LayerPause &operator=(const LayerPause &) = delete;
};

bool badName(const QString &n) { return n.contains(QLatin1Char('/')) || n.contains(QLatin1Char('\\')) || n.contains(QLatin1String("..")); }

Content contentOf(QSettings &s)
{
    Content c;
    c.values = valuesOf(s);
    c.themeId = c.values.value(QStringLiteral("appearance/theme")).toString();
    c.themeJson = s.value(QStringLiteral("profile/themeJson")).toString();
    c.computer = s.value(QStringLiteral("profile/computer")).toString();
    if (s.value(QStringLiteral("profile/layouts")).toBool()) {
        c.hasLayouts = true;
        s.beginGroup(QStringLiteral("layouts"));
        for (const QString &k : s.childKeys())
            c.layouts.insert(k, s.value(k).toByteArray());
        s.endGroup();
    }
    return c;
}

QString ownThemeText(const QString &id)
{
    if (!id.startsWith(QLatin1String("user:")) || badName(id))
        return QString();
    QFile f(theme::userThemeDir() + QLatin1Char('/') + id.mid(5) + QLatin1String(".json"));
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

// the INI text of a content (a file with name, keys, layouts and the theme)
QByteArray iniOf(const QString &name, const Content &c, QString *error)
{
    QTemporaryFile tmp(QDir::tempPath() + QLatin1String("/qbprofile-XXXXXX.ini"));
    if (!tmp.open()) {
        if (error)
            *error = QObject::tr("A temporary file could not be made.");
        return {};
    }
    const QString path = tmp.fileName();
    tmp.close();
    QByteArray bytes;
    {
        QSettings to(path, QSettings::IniFormat);
        to.setValue(QStringLiteral("profile/version"), 1);
        to.setValue(QStringLiteral("profile/name"), name);
        if (!c.computer.isEmpty())
            to.setValue(QStringLiteral("profile/computer"), c.computer);
        for (auto it = c.values.cbegin(); it != c.values.cend(); ++it)
            to.setValue(it.key(), it.value());
        if (c.hasLayouts) {
            to.setValue(QStringLiteral("profile/layouts"), true);
            for (auto it = c.layouts.cbegin(); it != c.layouts.cend(); ++it)
                to.setValue(QStringLiteral("layouts/") + it.key(), it.value());
        }
        const QString themeJson = c.themeJson.isEmpty() ? ownThemeText(c.themeId) : c.themeJson;
        if (!themeJson.isEmpty())
            to.setValue(QStringLiteral("profile/themeJson"), themeJson);
        to.sync();
        if (to.status() != QSettings::NoError) {
            if (error)
                *error = QObject::tr("The file could not be written.");
            return {};
        }
    }
    QFile in(path);
    if (in.open(QIODevice::ReadOnly))
        bytes = in.readAll();
    in.close();
    QFile::remove(path);
    if (bytes.isEmpty() && error)
        *error = QObject::tr("Nothing to write.");
    return bytes;
}

bool writeFileFor(const QString &file, const QString &name, const Content &c, QString *error)
{
    QString why;
    const QByteArray bytes = iniOf(name, c, error);
    if (bytes.isEmpty())
        return false;
    QDir().mkpath(QFileInfo(file).absolutePath());
    if (!safesave::writeFile(file, bytes, &why)) {
        if (error)
            *error = QObject::tr("The file could not be written (%1). The file that was there is unchanged.").arg(why);
        return false;
    }
    return true;
}

bool readFileInto(const QString &file, Content *out)
{
    QSettings from(file, QSettings::IniFormat);
    if (from.status() != QSettings::NoError || !from.contains(QStringLiteral("profile/version")))
        return false;
    *out = contentOf(from);
    return true;
}

QString nameInFile(const QString &file)
{
    QSettings from(file, QSettings::IniFormat);
    return from.value(QStringLiteral("profile/name")).toString().trimmed();
}

// the file of a profile ("" = none yet)
QString fileOf(const QString &name)
{
    for (const QFileInfo &fi : QDir(userDir()).entryInfoList({QLatin1String("*") + QLatin1String(kSuffix)}, QDir::Files, QDir::Name))
        if (nameInFile(fi.absoluteFilePath()).compare(name, Qt::CaseInsensitive) == 0)
            return fi.absoluteFilePath();
    return QString();
}

QString newFileFor(const QString &name)
{
    const QString base = fileBase(name);
    QString file = userDir() + QLatin1Char('/') + base + QLatin1String(kSuffix);
    for (int n = 2; QFile::exists(file); ++n)
        file = userDir() + QLatin1Char('/') + base + QLatin1Char('-') + QString::number(n) + QLatin1String(kSuffix);
    return file;
}

QStringList removedBuiltIns() { return QSettings().value(QStringLiteral("profile/removed")).toStringList(); }

bool builtInPresent() { return !removedBuiltIns().contains(builtInName(), Qt::CaseInsensitive); }

bool builtInContent(Content *out)
{
    QTemporaryFile tmp(QDir::tempPath() + QLatin1String("/qbbuiltin-XXXXXX.ini"));
    if (!tmp.open())
        return false;
    tmp.write(builtInText().toUtf8());
    tmp.flush();
    const QString path = tmp.fileName();
    tmp.close();
    QSettings from(path, QSettings::IniFormat);
    *out = Content();
    out->values = valuesOf(from);
    out->themeId = out->values.value(QStringLiteral("appearance/theme")).toString();
    return !out->values.isEmpty();
}

// The new layouts are written beside the old ones first; the old ones are replaced only when all of them could be written
bool setLayouts(const Content &c)
{
    QDir dir(layouts::userDir());
    dir.mkpath(QStringLiteral("."));
    const QString stage = dir.filePath(QStringLiteral(".staging"));
    QDir(stage).removeRecursively();
    QDir().mkpath(stage);
    bool ok = true;
    QStringList staged;
    for (auto it = c.layouts.cbegin(); it != c.layouts.cend() && ok; ++it) {
        if (badName(it.key()))
            continue;
        ok = safesave::writeFile(stage + QLatin1Char('/') + it.key() + QLatin1String(".json"), it.value());
        staged << it.key();
    }
    if (!ok) {
        QDir(stage).removeRecursively();
        return false; // the layouts there were stay as they are
    }
    for (const QFileInfo &fi : dir.entryInfoList({QStringLiteral("*.json")}, QDir::Files))
        QFile::remove(fi.absoluteFilePath());
    for (const QString &k : staged) {
        const QString to = dir.filePath(k + QLatin1String(".json"));
        const QString from = stage + QLatin1Char('/') + k + QLatin1String(".json");
        if (!QFile::rename(from, to) && !QFile::copy(from, to))
            ok = false;
    }
    QDir(stage).removeRecursively();
    return ok;
}

void putOwnTheme(const Content &c)
{
    if (c.themeJson.isEmpty() || !c.themeId.startsWith(QLatin1String("user:")) || badName(c.themeId))
        return;
    const QString file = theme::userThemeDir() + QLatin1Char('/') + c.themeId.mid(5) + QLatin1String(".json");
    if (QFile::exists(file))
        return;
    QDir().mkpath(theme::userThemeDir());
    safesave::writeFile(file, c.themeJson.toUtf8());
}

} // namespace

QString defaultName() { return QStringLiteral("Default"); }
QString builtInName() { return QStringLiteral("Shelin's"); }

QString userDir() { return QFileInfo(theme::userThemeDir()).absolutePath() + QLatin1String("/profiles"); }

QString systemThemeId()
{
    if (QB_TEST_ENV_SET("QB_COLOR_SCHEME")) {
        const QString forced = QB_TEST_ENV("QB_COLOR_SCHEME").trimmed().toLower();
        return forced == QLatin1String("light") || forced == QLatin1String("dark") ? forced : QString();
    }
    if (!qGuiApp)
        return QString();
    switch (QGuiApplication::styleHints()->colorScheme()) {
    case Qt::ColorScheme::Dark: return QStringLiteral("dark");
    case Qt::ColorScheme::Light: return QStringLiteral("light");
    default: return QString();
    }
}

bool ownsKey(const QString &key) { return !key.isEmpty() && !excluded(key); }

QStringList names()
{
    QStringList others;
    for (const QFileInfo &fi : QDir(userDir()).entryInfoList({QLatin1String("*") + QLatin1String(kSuffix)}, QDir::Files, QDir::Name)) {
        const QString n = nameInFile(fi.absoluteFilePath());
        if (n.isEmpty() || n.compare(builtInName(), Qt::CaseInsensitive) == 0 || n.compare(defaultName(), Qt::CaseInsensitive) == 0
            || others.contains(n, Qt::CaseInsensitive))
            continue;
        others << n;
    }
    std::sort(others.begin(), others.end(), [](const QString &a, const QString &b) { return a.localeAwareCompare(b) < 0; });
    QStringList out;
    if (builtInPresent())
        out << builtInName();
    out << defaultName();
    return out + others;
}

bool exists(const QString &name)
{
    for (const QString &n : names())
        if (n.compare(name, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

bool isBuiltIn(const QString &name) { return name.compare(builtInName(), Qt::CaseInsensitive) == 0; }
bool canDelete(const QString &name) { return name.compare(defaultName(), Qt::CaseInsensitive) != 0; }

bool nameInUse(const QString &name, const QString &except)
{
    const QString n = name.trimmed();
    if (n.compare(builtInName(), Qt::CaseInsensitive) == 0 && except.compare(builtInName(), Qt::CaseInsensitive) != 0)
        return true; // the name is the built-in profile's, also after the writer deleted it
    for (const QString &x : names())
        if (x.compare(n, Qt::CaseInsensitive) == 0 && x.compare(except, Qt::CaseInsensitive) != 0)
            return true;
    return false;
}

QString uniqueName(const QString &wanted)
{
    QString base = wanted.trimmed().isEmpty() ? QObject::tr("My Profile") : wanted.trimmed();
    QString name = base;
    for (int n = 2; nameInUse(name); ++n)
        name = base + QLatin1Char(' ') + QString::number(n);
    return name;
}

QString active()
{
    const QString n = QSettings().value(QStringLiteral("profile/active")).toString().trimmed();
    return n.isEmpty() ? defaultName() : n;
}

void setActive(const QString &name)
{
    QSettings s;
    s.setValue(QStringLiteral("profile/active"), name);
    s.sync();
}

QString computerId()
{
    LayerPause pause; // the id is the program's own, not the project's
    QSettings s;
    QString id = s.value(QStringLiteral("profile/computerId")).toString();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        s.setValue(QStringLiteral("profile/computerId"), id);
        s.sync();
    }
    return id;
}

Content captureCurrent()
{
    LayerPause pause;
    QSettings s;
    s.sync();
    Content c;
    c.values = valuesOf(s);
    c.themeId = c.values.value(QStringLiteral("appearance/theme")).toString();
    c.hasLayouts = true;
    for (const QFileInfo &fi : QDir(layouts::userDir()).entryInfoList({QStringLiteral("*.json")}, QDir::Files, QDir::Name)) {
        QFile f(fi.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly))
            c.layouts.insert(fi.completeBaseName(), f.readAll());
    }
    c.themeJson = ownThemeText(c.themeId);
    c.computer = computerId();
    return c;
}

bool read(const QString &name, Content *out, QString *error)
{
    const QString file = fileOf(name);
    if (!file.isEmpty()) {
        if (readFileInto(file, out))
            return true;
        if (error)
            *error = QObject::tr("The profile “%1” could not be read.").arg(name);
        return false;
    }
    if (isBuiltIn(name) && builtInPresent())
        return builtInContent(out);
    if (name.compare(defaultName(), Qt::CaseInsensitive) == 0) {
        *out = Content(); // the program's standard
        return true;
    }
    if (error)
        *error = QObject::tr("There is no profile “%1”.").arg(name);
    return false;
}

bool write(const QString &name, const Content &content, QString *error)
{
    QString file = fileOf(name);
    if (file.isEmpty())
        file = newFileFor(name);
    return writeFileFor(file, name, content, error);
}

bool saveActive(QString *error)
{
    LayerPause pause;
    return write(active(), captureCurrent(), error);
}

SwitchReport switchTo(const QString &name, const QStringList &refusedPlaces)
{
    LayerPause pause; // the program's own values are read and written, not the merged copy of an open project's
    SwitchReport r;
    r.from = active();
    r.to = name;
    Content target;
    if (!exists(name) || !read(name, &target, &r.error)) {
        if (r.error.isEmpty())
            r.error = QObject::tr("There is no profile “%1”.").arg(name);
        return r;
    }
    if (!saveActive(&r.error))
        return r;
    QSettings s;
    s.sync();
    const QString oldLanguage = s.value(QStringLiteral("app/language"), QStringLiteral("en")).toString();
    for (const QString &k : s.allKeys())
        if (ownsKey(k) && !keptWhenAbsent(k) && !target.values.contains(k))
            s.remove(k);
    for (auto it = target.values.cbegin(); it != target.values.cend(); ++it) {
        if (refusedPlaces.contains(it.key())) {
            r.places << it.key();
            continue;
        }
        s.setValue(it.key(), it.value());
    }
    s.sync();
    if (s.status() != QSettings::NoError)
        r.warning = QObject::tr("The settings of the profile could not be written completely. Check the folder of the settings file.");
    const QString newLanguage = s.value(QStringLiteral("app/language"), QStringLiteral("en")).toString();
    r.languageChanged = oldLanguage != newLanguage;
    putOwnTheme(target);
    if (target.hasLayouts && !setLayouts(target)) {
        const QString w = QObject::tr("The layouts of the profile could not be set; the layouts you had are kept.");
        r.warning = r.warning.isEmpty() ? w : r.warning + QLatin1Char(' ') + w;
    }
    setActive(name);
    r.ok = true;
    return r;
}

bool createFromCurrent(const QString &name, QString *error)
{
    LayerPause pause;
    const QString n = name.trimmed();
    if (n.isEmpty() || nameInUse(n)) {
        if (error)
            *error = QObject::tr("There is already a profile named “%1”. Choose another name.").arg(n);
        return false;
    }
    if (!saveActive(error))
        return false;
    if (!write(n, captureCurrent(), error))
        return false;
    setActive(n);
    return true;
}

bool remove(const QString &name, QString *error)
{
    if (!exists(name) || !canDelete(name)) {
        if (error)
            *error = QObject::tr("The profile “%1” cannot be deleted.").arg(name);
        return false;
    }
    if (name.compare(active(), Qt::CaseInsensitive) == 0) {
        if (error)
            *error = QObject::tr("The profile in use cannot be deleted. Switch to another profile first.");
        return false;
    }
    const QString file = fileOf(name);
    if (!file.isEmpty() && !QFile::remove(file)) {
        if (error)
            *error = QObject::tr("The file of the profile could not be deleted.");
        return false;
    }
    if (isBuiltIn(name)) {
        QStringList l = removedBuiltIns();
        l << builtInName();
        QSettings s;
        s.setValue(QStringLiteral("profile/removed"), l); // it stays away after updates
        s.sync();
    }
    return true;
}

bool rename(const QString &name, const QString &newName, QString *error)
{
    const QString n = newName.trimmed();
    if (!exists(name) || isBuiltIn(name) || name.compare(defaultName(), Qt::CaseInsensitive) == 0) {
        if (error)
            *error = QObject::tr("The profile “%1” cannot be renamed.").arg(name);
        return false;
    }
    if (n.isEmpty() || nameInUse(n, name)) {
        if (error)
            *error = QObject::tr("There is already a profile named “%1”. Choose another name.").arg(n);
        return false;
    }
    const bool wasActive = name.compare(active(), Qt::CaseInsensitive) == 0;
    Content c;
    if (wasActive)
        c = captureCurrent();
    else if (!read(name, &c, error))
        return false;
    const QString old = fileOf(name);
    if (!write(n, c, error))
        return false;
    // a change of case only ("work" to "Work") writes the very file that was there: it must stay
    const QString now = fileOf(n);
    if (!old.isEmpty() && (now.isEmpty() || QFileInfo(old).canonicalFilePath() != QFileInfo(now).canonicalFilePath()))
        QFile::remove(old);
    if (wasActive)
        setActive(n);
    return true;
}

bool duplicate(const QString &name, const QString &newName, QString *error)
{
    const QString n = newName.trimmed();
    if (n.isEmpty() || nameInUse(n)) {
        if (error)
            *error = QObject::tr("There is already a profile named “%1”. Choose another name.").arg(n);
        return false;
    }
    Content c;
    if (name.compare(active(), Qt::CaseInsensitive) == 0)
        c = captureCurrent();
    else if (!read(name, &c, error))
        return false;
    return write(n, c, error);
}

bool exportTo(const QString &name, const QString &file, QString *error)
{
    if (settingslayer::projectEditing()) {
        if (error)
            *error = QObject::tr("Options are open for This Project. Close them or choose All Projects, then export the profile.");
        return false;
    }
    LayerPause pause;
    Content c;
    if (name.compare(active(), Qt::CaseInsensitive) == 0)
        c = captureCurrent();
    else if (!read(name, &c, error))
        return false;
    if (!c.hasLayouts)
        c.hasLayouts = false;
    return writeFileFor(file, name, c, error);
}

bool isProfileFile(const QString &file)
{
    QSettings from(file, QSettings::IniFormat);
    return from.status() == QSettings::NoError
           && (from.contains(QStringLiteral("profile/version")) || from.contains(QStringLiteral("quillbinderOptions/version")));
}

QStringList placesIn(const QString &file)
{
    QSettings from(file, QSettings::IniFormat);
    QStringList out;
    for (const QString &k : from.allKeys())
        if (ownsKey(k) && settingslayer::namesProgramOrPlace(k) && !from.value(k).toString().trimmed().isEmpty())
            out << k; // every value, as the files of options were asked about (a command has no slash)
    return out;
}

QStringList placesAsked(const QString &name)
{
    Content c;
    const QString file = fileOf(name);
    if (file.isEmpty() || !read(name, &c) || c.computer == computerId())
        return {};
    QStringList out;
    for (auto it = c.values.cbegin(); it != c.values.cend(); ++it)
        if (settingslayer::namesProgramOrPlace(it.key()) && !it.value().toString().trimmed().isEmpty())
            out << it.key();
    return out;
}

bool lowersBackupKeep(const QString &name, int *now, int *then)
{
    Content c;
    if (!read(name, &c))
        return false;
    LayerPause pause;
    const int def = 25; // AppSettings::backupKeep's value without a saved one
    const int cur = QSettings().value(QStringLiteral("backup/keep"), def).toInt();
    const int next = c.values.contains(QStringLiteral("backup/keep")) ? c.values.value(QStringLiteral("backup/keep")).toInt() : def;
    if (now)
        *now = cur;
    if (then)
        *then = next;
    return next != 0 && (cur == 0 || next < cur);
}

QString importFrom(const QString &file, const QStringList &refusedPlaces, QString *error)
{
    QSettings from(file, QSettings::IniFormat);
    if (from.status() != QSettings::NoError || !isProfileFile(file)) {
        if (error)
            *error = QObject::tr("This is not a file of a profile or of options saved by Quillbinder.");
        return QString();
    }
    const bool isProfile = from.contains(QStringLiteral("profile/version"));
    Content c = contentOf(from);
    QString wanted = isProfile ? from.value(QStringLiteral("profile/name")).toString().trimmed() : QString();
    if (wanted.isEmpty())
        wanted = QFileInfo(file).completeBaseName();
    for (const QString &k : refusedPlaces)
        c.values.remove(k);
    c.computer = computerId(); // the writer was asked about the places; what is taken is theirs now
    if (!isProfile)
        c.hasLayouts = false; // an options file knows no layouts
    const QString name = uniqueName(wanted);
    c.themeId = c.values.value(QStringLiteral("appearance/theme")).toString();
    putOwnTheme(c);
    if (!write(name, c, error))
        return QString();
    return name;
}

QList<int> fitColumns(const QList<int> &sizes, int total, int minSide, int minCenter)
{
    if (sizes.size() != 3 || total <= 0)
        return sizes;
    int sum = 0;
    for (int v : sizes)
        sum += qMax(0, v);
    if (sum <= 0)
        return sizes;
    QList<int> out;
    for (int v : sizes)
        out << int(qMax(0, v) * double(total) / sum);
    const int maxSide = int(total * 0.4);
    for (int i : {0, 2})
        if (out[i] > 0)
            out[i] = qBound(qMin(minSide, maxSide), out[i], maxSide);
    if (out[1] > 0) {
        const int sides = out[0] + out[2];
        out[1] = total - sides;
        if (out[1] < minCenter && total >= minCenter + 2 * qMin(minSide, 100)) {
            // the sides give way to the editor
            const int need = minCenter - out[1];
            const int takeable = qMax(0, out[0] - 100) + qMax(0, out[2] - 100);
            const int take = qMin(need, takeable);
            if (takeable > 0) {
                const int a = int(double(take) * qMax(0, out[0] - 100) / takeable);
                out[0] -= a;
                out[2] -= take - a;
            }
            out[1] = total - out[0] - out[2];
        }
    }
    return out;
}

QString builtInText()
{
    return QString::fromUtf8(R"ini([app]
autoQuit=false
autoQuitMinutes=30
autosave=2
freezeSeconds=30
groupDigits=true
keepReports=20
language=en
openLast=true
portal=true
projectFirst=true
recentCount=12
sepMerge=return
sepMergeCustom=---
sepScratch=return
sepScratchCustom=***
snapshotOnSave=true

[appearance]
barHighlightColor=#b7f2b2
barTextColor=#b7f2b2
binderIndent=12
cardWidth=210
cardsPerRow=0
combinedTitles=false
compFade=0
compPaperWidth=0.55
compZoom=125
compareByChapter=true
compareStepByStep=false
editorWidth=500
fixedWidth=true
labelInBinder=true
labelInIcons=true
progressUnderSearch=true
recentIcons=qb-checked, qb-proofread, qb-book-blue, qb-book-green
showFullPath=false
stamps=true
textBorder=10
theme=shelins-dark
tintCards=false
typewriter=false

[backup]
keep=50
onClose=true
onManualSave=true
onOpen=false

[behaviour]
backLinks=false
binderScrivenerKeys=false
compEsc=true
compHideMain=false
compScreen=current
corkDoubleClickNew=true
dragExportChildren=true
dragExportFormat=rtf
dropOnTextDocuments=true
floatQuickRef=false
folderCorkboard=true
linkOpens=other
reopenQuickRefs=true
returnCreatesItem=false
returnEndsSynopsis=true
snapshotNotify=false
subdocsAsFolders=true

[editing]
blockCursor=false
blockCursorWidth=5
capitalizeI=true
capitalizeSentences=true
completions=true
countChars=false
detectDocLinks=false
detectUrls=false
ellipsis=false
newAlign=justify
newFirstLineIndent=0
newFontFamily=Sitka
newFontSize=11
newJustify=true
newLineSpacing=1.09
newParagraphSpacing=10.666666666666666
noColorAnnotations=false
notesBeforePunctuation=false
overtypeKey=false
quoteStyle=en
reloadZoom=true
rulerSnapMm=0
scriptNoSmart=true
showInvisibles=false
showRuler=true
smartDashes=false
smartQuotes=true
spellCheck=true
spellIgnoreNumbers=true
spellLanguage=en_US
spellUnderline=waves
substitutions=(c), ©, (r), ®, (tm), ™, 1/2, ½, 1/4, ¼, 3/4, ¾, ->, →, <-, ←
substitutionsOn=false

[export]
children=true
comments=true
format=rtf
metadata=true
notes=true
snapshots=false
synopsis=true

[mindmap]
autoSize=true
boardBackground=#fdfbf0
boardBackgroundDark=#2e2e2b
boardRtl=false
dashedLines=true
enterCreates=false
enterEnds=false
faded=60
iconSize=16
newNoteBorder=3
newNoteShape=0
newNoteSize=12
newNoteWidth=160
newShapeBorder=1
newShapeFill=#808080
newShapeMagnetic=false
useSelectedStyle=true

[playback]
backend=ffmpeg
pauseRewind=false
rememberPosition=true
rewindSeconds=1
skipSeconds=5
volume=1

[sharing]
commentsInline=false
docConverter=builtin
encoding=auto
footnotesInline=true
htmlAsText=false
ignoreStyles=false

[toolbars]
bar-tools=bookmarks, quickRef, notes, |, compile, |, showInvisibles, composition, tool:speech-transcription.record, showInspector, tool:translation-proofreading.window
hidden=themes
iconSize=22
main=view, |, add, newFolder, trash, undo, redo, <>, themePicker, <>, searchMenu, search, viewModes, <>, bookmarks, quickRef, notes, |, compile, |, showInvisibles, composition, tool:speech-transcription.record, showInspector
style=icons
toolButtonsPlaced=tool:speech-transcription.record, tool:translation-proofreading.window

[tools]
scratch-pad\onTop=false
scratch-pad\rich=true
speech-transcription\careful=true
speech-transcription\deleteRecording=true
speech-transcription\fallback=de
speech-transcription\keepOriginal=false
speech-transcription\main=en
speech-transcription\mode=record
speech-transcription\model=medium
speech-transcription\paragraphMs=3000
speech-transcription\pauseMs=700
speech-transcription\sensitivity=1
speech-transcription\silenceGuard=true
speech-transcription\source=official
speech-transcription\unloadMinutes=15
speech-transcription\useContext=false
translation-proofreading\american=true
translation-proofreading\careful=true
translation-proofreading\corrector=coedit-large
translation-proofreading\curlyQuotes=true
translation-proofreading\dictionary=wiktionary-de
translation-proofreading\live=true
translation-proofreading\noEmDash=true
translation-proofreading\plainEllipsis=true
translation-proofreading\translator=opus-mt-de-en
translation-proofreading\translator-en=opus-mt-en-de
translation-proofreading\unloadMinutes=15

[window]
barRows=top|mainToolbar:0 themesBar:0 paragraphBar:284 searchBar:82 toolsBar:270, top|formatBar:0 characterBar:51 iconBar:58
binder=true
formatBar=true
inspector=true
mainSplit=325, 1030, 557
paneOrder=BEI
themeFilter=both
toolbar=true
uprightBinderLeft=true
uprightEditorFirst=true
uprightShares=0.62, 0.46
)ini");
}

} // namespace profiles
} // namespace qb
