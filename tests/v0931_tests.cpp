// Scripted run for version 0.93.1 (fixes): the shared-code items of the run's to-do list.
//  - item 35, S-K01: the generic font names ("monospace", "serif", "sans-serif") on a platform that does not know them
//    (Qt for Android gave its default font, so the style Code Span reached Word files in Roboto): the rule, and that
//    on Linux, which knows the names, nothing is mapped;
//  - item 45: the plain texts kept for search and the counts are bounded (the texts asked for past the bound are read
//    from their files and not kept; a changed text frees its part);
//  - item 34 (test idea C): Help > System Information names each screen's usable area, the input devices and whether
//    the on-screen keyboard is shown.
//
//   xvfb-run -a ./qb_v0931_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "text/FontFaces.h"
#include "ui/AppSettings.h"
#include "ui/CrashDialogs.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontInfo>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>

#include <cstdio>

using namespace qb;

static int failures = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (!ok) {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(3000)));
    }
    std::fflush(stdout);
}

// ---------------------------------------------------------------- item 35, S-K01: generic font names
static void genericFonts()
{
    std::printf("== the generic font names (item 35, S-K01)\n");
    using fontfaces::genericTarget;
    // Qt for Android: "monospace" is no family there and falls to the default font; its style hint gives a fixed font
    check(genericTarget(QStringLiteral("monospace"), QStringLiteral("Roboto"), false, QStringLiteral("Droid Sans Mono"), true,
                        QStringLiteral("Roboto"))
              == QLatin1String("Droid Sans Mono"),
          QStringLiteral("a monospace name that gives a proportional font is mapped to the fixed font of its style hint"));
    check(genericTarget(QStringLiteral("Monospace"), QStringLiteral("Roboto"), false, QStringLiteral("Droid Sans Mono"), true,
                        QStringLiteral("Roboto"))
              == QLatin1String("Droid Sans Mono"),
          QStringLiteral("the generic names are known in any case"));
    check(genericTarget(QStringLiteral("monospace"), QStringLiteral("DejaVu Sans Mono"), true, QStringLiteral("DejaVu Sans Mono"),
                        true, QStringLiteral("DejaVu Sans"))
              .isEmpty(),
          QStringLiteral("a monospace name the platform knows (Linux) is not mapped"));
    check(genericTarget(QStringLiteral("monospace"), QStringLiteral("Roboto"), false, QStringLiteral("Roboto"), false,
                        QStringLiteral("Roboto"))
              .isEmpty(),
          QStringLiteral("a style hint that gives no fixed font maps nothing"));
    check(genericTarget(QStringLiteral("serif"), QStringLiteral("Roboto"), false, QStringLiteral("Noto Serif"), false,
                        QStringLiteral("Roboto"))
              == QLatin1String("Noto Serif"),
          QStringLiteral("a serif name that falls to the default font is mapped to the serif font of its style hint"));
    check(genericTarget(QStringLiteral("serif"), QStringLiteral("DejaVu Serif"), false, QStringLiteral("DejaVu Serif"), false,
                        QStringLiteral("DejaVu Sans"))
              .isEmpty(),
          QStringLiteral("a serif name the platform knows is not mapped"));
    check(genericTarget(QStringLiteral("sans-serif"), QStringLiteral("Roboto"), false, QStringLiteral("Roboto"), false,
                        QStringLiteral("Roboto"))
              .isEmpty(),
          QStringLiteral("a sans-serif name that gives the default sans-serif font is not mapped"));
    check(genericTarget(QStringLiteral("DejaVu Serif"), QStringLiteral("Roboto"), false, QStringLiteral("Noto Serif"), false,
                        QStringLiteral("Roboto"))
              .isEmpty(),
          QStringLiteral("a real family name is never mapped"));
    check(fontfaces::genericHint(QStringLiteral("monospace")) == QFont::Monospace
              && fontfaces::genericHint(QStringLiteral("serif")) == QFont::Serif
              && fontfaces::genericHint(QStringLiteral("sans-serif")) == QFont::SansSerif
              && fontfaces::genericHint(QStringLiteral("Courier New")) == QFont::AnyStyle,
          QStringLiteral("each generic name has its style hint, other names none"));

    // Linux shows nothing new: fontconfig knows the names, so the start maps none of them
    const QStringList before = QFont::substitutes(QStringLiteral("monospace")) + QFont::substitutes(QStringLiteral("serif"))
                               + QFont::substitutes(QStringLiteral("sans-serif"));
    fontfaces::mapGenericFamilies();
    const QStringList after = QFont::substitutes(QStringLiteral("monospace")) + QFont::substitutes(QStringLiteral("serif"))
                              + QFont::substitutes(QStringLiteral("sans-serif"));
    check(after == before, QStringLiteral("on Linux the start maps none of the generic names"), after.join(QLatin1String(", ")));
    const QFontInfo mono{QFont(QStringLiteral("monospace"))};
    check(mono.fixedPitch(), QStringLiteral("on Linux \"monospace\" gives a fixed-pitch font (%1)").arg(mono.family()));
    QFont hinted(QStringLiteral("monospace"));
    hinted.setStyleHint(fontfaces::genericHint(QStringLiteral("monospace")));
    check(QFontInfo(hinted).family() == mono.family(),
          QStringLiteral("the style hint that the Word and e-book export now give changes nothing on Linux"),
          QFontInfo(hinted).family() + QStringLiteral(" / ") + mono.family());
}

// ---------------------------------------------------------------- item 45: the plain texts kept are bounded
static QString words(int n, int seed)
{
    static const char *w[] = {"river", "lantern", "harbor", "quiet", "stone", "letter", "winter", "garden", "morning", "ash"};
    QStringList out;
    for (int i = 0; i < n; ++i)
        out << QString::fromLatin1(w[(i * 7 + seed) % 10]);
    return out.join(QLatin1Char(' '));
}

static void plainCache(const QString &W)
{
    std::printf("== the plain texts kept for search and counts are bounded (item 45)\n");
    check(DocumentStore::kPlainCacheBudget == 4000000, QStringLiteral("the bound is 4,000,000 characters (about 8 MB)"));
    QString err;
    QDir().mkpath(W + QStringLiteral("/cache"));
    QString path;
    QStringList ids;
    {
        auto p = Project::create(W + QStringLiteral("/cache"), QStringLiteral("Cache Book"), &err);
        check(p != nullptr, QStringLiteral("(the project is made)"), err);
        if (!p)
            return;
        for (int i = 0; i < 20; ++i) {
            BinderItem *it = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Scene %1").arg(i));
            QTextCursor c(p->docs()->text(it->id));
            c.insertText(words(1000, i) + QStringLiteral(" Marker%1").arg(i));
            ids << it->id;
        }
        check(p->save(&err), QStringLiteral("(the project is saved)"), err);
        path = p->indexPath();
    }
    auto p = Project::open(path, &err);
    check(p != nullptr, QStringLiteral("(the project is opened again, no text loaded)"), err);
    if (!p)
        return;
    DocumentStore *d = p->docs();
    bool noneLoaded = true;
    for (const QString &id : ids)
        noneLoaded = noneLoaded && d->loadedText(id) == nullptr;
    check(noneLoaded, QStringLiteral("(no text is laid out after opening)"));
    const qint64 one = d->plainText(ids.first()).size();
    d->invalidatePlain(ids.first());
    check(d->plainCacheCharacters() == 0 && d->plainCacheCount() == 0, QStringLiteral("a text no longer kept frees its characters"));
    // a bound for five texts: the first five asked for are kept, the others read each time
    d->setPlainCacheBudget(one * 5 + 10);
    bool allRight = true;
    for (int i = 0; i < ids.size(); ++i)
        allRight = allRight && d->plainText(ids.at(i)).endsWith(QStringLiteral("Marker%1").arg(i));
    check(allRight, QStringLiteral("every plain text is right, kept or not"));
    check(d->plainCacheCount() == 5 && d->plainCacheCharacters() <= one * 5 + 10,
          QStringLiteral("only as many texts are kept as the bound allows (%1 kept, %2 characters)").arg(d->plainCacheCount()).arg(d->plainCacheCharacters()));
    bool again = true;
    for (int i = ids.size() - 1; i >= 0; --i)
        again = again && d->plainText(ids.at(i)).endsWith(QStringLiteral("Marker%1").arg(i));
    check(again && d->plainCacheCount() == 5, QStringLiteral("asked for a second time, the texts are still right and the count stays bounded"));
    // a kept text that changes is dropped and frees room for another
    QTextDocument *t0 = d->text(ids.first());
    QTextCursor c(t0);
    c.movePosition(QTextCursor::End);
    c.insertText(QStringLiteral(" CHANGED"));
    QTest::qWait(50);
    check(d->plainText(ids.first()).endsWith(QLatin1String("CHANGED")), QStringLiteral("a changed text gives its new plain text"));
    check(d->plainCacheCount() == 4, QStringLiteral("the changed text is no longer kept (%1 kept)").arg(d->plainCacheCount()));
    d->plainText(ids.at(10));
    check(d->plainCacheCount() == 5, QStringLiteral("the freed room takes the next text asked for"));
    d->setPlainCacheBudget(DocumentStore::kPlainCacheBudget);
}

// ---------------------------------------------------------------- item 34, test idea C: System Information
static void systemInfo()
{
    std::printf("== Help > System Information: usable area, input devices, on-screen keyboard (item 34)\n");
    const QString t = sysinfo::text(nullptr);
    QStringList lines;
    for (const QString &l : t.split(QLatin1Char('\n')))
        if (l.contains(QLatin1String("Screen ")) || l.contains(QLatin1String("Input devices")) || l.contains(QLatin1String("On-screen")))
            lines << l;
    check(t.contains(QRegularExpression(QStringLiteral("Screen [^:]+: \\d+ x \\d+ px \\(usable \\d+ x \\d+ px\\)"))),
          QStringLiteral("each screen names its usable area"), lines.join(QLatin1String(" | ")));
    check(t.contains(QRegularExpression(QStringLiteral("Input devices: .*(keyboard|mouse|none reported)"))),
          QStringLiteral("the input devices are listed by kind"), lines.join(QLatin1String(" | ")));
    bool printable = true;
    for (const QString &l : lines)
        for (const QChar ch : l)
            printable = printable && ch.isPrint();
    check(printable, QStringLiteral("the device names hold only printable characters"), lines.join(QLatin1String(" | ")));
    check(t.contains(QLatin1String("On-screen keyboard: not shown")), QStringLiteral("the on-screen keyboard's state is named"),
          lines.join(QLatin1String(" | ")));
    check(!t.contains(QLatin1String("%1")) && !t.contains(QLatin1String("%10")), QStringLiteral("no placeholder is left in the report"));
    SystemInfoDialog dlg(nullptr);
    dlg.resize(900, 600);
    dlg.show();
    QTest::qWait(200);
    dlg.grab().save(outDir + QStringLiteral("/system-information.png"));
    auto *view = dlg.findChild<QPlainTextEdit *>(QStringLiteral("systemInfoText"));
    check(view && view->toPlainText().contains(QLatin1String("usable")), QStringLiteral("the window shows the new lines"));
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("XDG_RUNTIME_DIR", (home.path() + QLatin1String("/run")).toUtf8());
    QDir().mkpath(home.path() + QLatin1String("/run"));
    QFile::setPermissions(home.path() + QLatin1String("/run"), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0931Test"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0931Test"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    QTemporaryDir work;
    genericFonts();
    plainCache(work.path());
    systemInfo();
    std::printf("%d failure(s)\n", failures);
    return failures;
}
