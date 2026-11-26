// 0.95.0, second part (xvfb-run -a ./qb_v0950b_tests /tmp/shots):
//  - item 92: the first-start page's texts about moving the AppImage; the menu entry always names the AppImage's
//    current place (a fresh home, an older entry that is replaced and not doubled, a file moved by hand).
#include "AppInfo.h"
#include "compile/PluginManager.h"
#include "text/RtfIO.h"
#include "text/MarkdownIO.h"
#include "text/TextProps.h"
#include "text/TextUtil.h"
#include "text/Styles.h"
#include "ui/AppMenuEntry.h"
#include "ui/SpellChecker.h"
#include "ui/AppSettings.h"
#include "ui/FirstStartDialog.h"
#include "ui/Theme.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QSettings>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextCursor>
#include <QImage>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>

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
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 150)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

static void shot(QWidget *w, const QString &name)
{
    settle(300);
    w->grab().save(outDir + QLatin1Char('/') + name + QLatin1String(".png"));
}

static void writeFile(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
}

static QString fileText(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

static QStringList entryFiles(const QString &data)
{
    return QDir(data + QLatin1String("/applications")).entryList({QStringLiteral("*.desktop")}, QDir::Files);
}

// ---- item 92
static void appImageMove(const QString &home)
{
#ifdef Q_OS_LINUX
    const QString data = qEnvironmentVariable("XDG_DATA_HOME");
    const QString dl = home + QLatin1String("/Downloads/Quillbinder-9.9.9-test-x86_64.AppImage");
    writeFile(dl, QByteArray(30000, 'x'));
    QFile::setPermissions(dl, QFile::permissions(dl) | QFileDevice::ExeOwner);
    qputenv("APPIMAGE", dl.toUtf8());
    {
        FirstStartDialog d;
        d.show();
        settle(300);
        auto *move = d.findChild<QCheckBox *>(QStringLiteral("firstMenuMove"));
        auto *hint = d.findChild<QLabel *>(QStringLiteral("firstMenuHint"));
        const QString text = move ? move->text() : QString();
        check(move && text.contains(QStringLiteral("a fixed place the menu entry starts it from")) && !text.contains(QLatin1Char('(')),
              QStringLiteral("the move checkbox names the fixed place and has no bracket"), text);
        check(hint && hint->text().contains(QStringLiteral("If you move the AppImage yourself, add it to the menu again.")),
              QStringLiteral("the grey line tells that a file moved by hand needs the menu entry again"), hint ? hint->text() : QString());
        shot(&d, QStringLiteral("92-first-start-menu"));
    }
    // (a) a fresh home: the entry starts the moved AppImage
    check(entryFiles(data).isEmpty(), QStringLiteral("a fresh home has no menu entry"));
    const QString moved = appmenu::applicationsFolder() + QLatin1String("/") + QFileInfo(dl).fileName();
    appmenu::Result r = appmenu::add(true, {});
    check(r.ok && r.moved && QFile::exists(moved) && !QFile::exists(dl) && entryFiles(data).size() == 1
              && fileText(appmenu::desktopFilePath()).contains(QStringLiteral("Exec=\"%1\"").arg(moved)),
          QStringLiteral("fresh home: the one menu entry starts the AppImage in ~/Applications"), r.error + r.moveError);
    // (b) an older entry (it may start an old AppImage): replaced, not doubled
    writeFile(appmenu::desktopFilePath(),
              "[Desktop Entry]\nType=Application\nName=Quillbinder\nExec=\"/old/place/Quillbinder-0.1.0-x86_64.AppImage\" %f\nIcon=quillbinder\n");
    const QString dl2 = home + QLatin1String("/Downloads/Quillbinder-9.9.10-test-x86_64.AppImage");
    writeFile(dl2, QByteArray(30000, 'y'));
    qputenv("APPIMAGE", dl2.toUtf8());
    const QString moved2 = appmenu::applicationsFolder() + QLatin1String("/") + QFileInfo(dl2).fileName();
    r = appmenu::add(true, {});
    const QString text = fileText(appmenu::desktopFilePath());
    check(r.ok && r.moved && entryFiles(data).size() == 1 && text.contains(QStringLiteral("Exec=\"%1\"").arg(moved2))
              && !text.contains(QStringLiteral("0.1.0")) && text.count(QStringLiteral("[Desktop Entry]")) == 1,
          QStringLiteral("an older entry is replaced, not doubled, and starts the new AppImage in ~/Applications"), text);
    // a file moved by hand: Add to Application Menu writes its current place
    const QString by_hand = home + QLatin1String("/Elsewhere/") + QFileInfo(dl2).fileName();
    QDir().mkpath(home + QLatin1String("/Elsewhere"));
    QFile::rename(moved2, by_hand);
    qputenv("APPIMAGE", by_hand.toUtf8());
    r = appmenu::add(false, {});
    check(r.ok && !r.moved && fileText(appmenu::desktopFilePath()).contains(QStringLiteral("Exec=\"%1\"").arg(by_hand))
              && QSettings().value(QStringLiteral("app/integrated")).toString() == by_hand,
          QStringLiteral("Add to Application Menu after a move by hand writes the AppImage's current place"));
    QString err;
    appmenu::remove(&err);
    qunsetenv("APPIMAGE");
#else
    (void)home;
#endif
}

// ---- item 99
static void highlights()
{
    const QColor picked(0xb7, 0xf2, 0xb2);
    const QTextCharFormat hf = text::highlightFormat(picked);
    check(hf.background().color() == picked && hf.background().color().alpha() == 255,
          QStringLiteral("the highlight's background is the picked color, fully"), hf.background().color().name(QColor::HexArgb));
    check(text::highlightFormat().background().color().alpha() == 255, QStringLiteral("the standard highlight is drawn fully too"));
    for (const char *name : {"dark", "light"}) {
        AppSettings &as = AppSettings::instance();
        as.theme = QString::fromLatin1(name);
        as.save();
        theme::apply();
        QTextDocument doc;
        SpellChecker::attach(&doc);
        QTextCursor c(&doc);
        c.insertText(QStringLiteral("plain words "));
        QTextCharFormat cf = text::highlightFormat(picked);
        c.insertText(QStringLiteral("highlighted words"), cf);
        QTextCharFormat dark = text::highlightFormat(QColor(0x20, 0x30, 0x60));
        c.insertText(QStringLiteral(" and a dark highlight"), dark);
        SpellChecker::rehighlightAll();
        settle(100);
        const QTextBlock b = doc.firstBlock();
        // the stored format keeps the picked color and no text color of its own
        const QTextFragment fr = [&] {
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().text().startsWith(QStringLiteral("highlighted")))
                    return it.fragment();
            return QTextFragment();
        }();
        check(fr.isValid() && fr.charFormat().background().color() == picked && !fr.charFormat().hasProperty(QTextFormat::ForegroundBrush),
              QStringLiteral("%1 theme: the stored color stays as picked, no text color is stored").arg(QLatin1String(name)));
        const int pos = int(b.text().indexOf(QStringLiteral("highlighted")));
        const int pos2 = int(b.text().indexOf(QStringLiteral("dark highlight")));
        QColor shownFg1, shownFg2;
        for (const auto &r : b.layout()->formats()) {
            if (pos >= r.start && pos < r.start + r.length && r.format.hasProperty(QTextFormat::ForegroundBrush))
                shownFg1 = r.format.foreground().color();
            if (pos2 >= r.start && pos2 < r.start + r.length && r.format.hasProperty(QTextFormat::ForegroundBrush))
                shownFg2 = r.format.foreground().color();
        }
        const QColor paperText = theme::color(theme::Role::EditorText);
        const bool needs1 = theme::contrastRatio(paperText, picked) < 4.5;
        const bool needs2 = theme::contrastRatio(paperText, QColor(0x20, 0x30, 0x60)) < 4.5;
        check(needs1 == shownFg1.isValid() && (!shownFg1.isValid() || theme::contrastRatio(shownFg1, picked) >= 4.5),
              QStringLiteral("%1 theme: the text on the light green is made readable on screen only when it needs it").arg(QLatin1String(name)));
        check(needs2 == shownFg2.isValid() && (!shownFg2.isValid() || theme::contrastRatio(shownFg2, QColor(0x20, 0x30, 0x60)) >= 4.5),
              QStringLiteral("%1 theme: ... and on the dark blue").arg(QLatin1String(name)));
        // drawn: the picked color is in the picture exactly
        QTextEdit ed;
        ed.setDocument(&doc);
        ed.resize(600, 120);
        ed.show();
        settle(300);
        const QImage img = ed.viewport()->grab().toImage();
        int exact = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (QColor(img.pixel(x, y)) == picked)
                    ++exact;
        check(exact > 200, QStringLiteral("%1 theme: the picked color is drawn exactly (%2 pixels)").arg(QLatin1String(name)).arg(exact));
        shot(&ed, QStringLiteral("99-highlight-%1").arg(QLatin1String(name)));
        ed.setDocument(nullptr);
        // a compiled file and a saved file hold the picked color
        Formatting f;
        const QByteArray rtfData = rtf::write(&doc, f);
        QTextDocument back;
        rtf::read(rtfData, &back, f);
        bool found = false;
        for (auto it = back.firstBlock().begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().property(props::Highlight).toBool() && it.fragment().charFormat().background().color().rgb() == picked.rgb())
                found = true;
        check(found, QStringLiteral("%1 theme: a saved RTF file keeps the picked color").arg(QLatin1String(name)));
    }
    AppSettings::instance().theme = QStringLiteral("light");
    theme::apply();
}

// @@FUNCTIONS@@

// ---- main
int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("HOME", home.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0950bTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0950bTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    theme::apply();
    compile::PluginManager::instance().load();

    appImageMove(home.path());
    highlights();
    // @@SECTIONS@@

    std::printf("%d failure(s)\n", failures);
    return failures;
}
