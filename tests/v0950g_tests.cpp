// Scripted run for version 0.95.0, item 102 (Quillbinder in eleven languages), part 2: every language started in a program of
// its own (the manual opens at the right section for F1, the welcome project is made and the menu places its texts and the
// quick start guide name are in that language's menus, the inspector's menu entries have their own letters), the spelling
// dictionary packages (license, install, found), the speech tool's language lists and the keyboard sets of the new layouts.
//
//   xvfb-run -a ./qb_v0950g_tests /tmp/shots
#include "AppInfo.h"
#include "core/Addons.h"
#include "core/BinderModel.h"
#include "core/DocumentStore.h"
#include "core/Project.h"
#include "text/MarkdownIO.h"
#include "ui/AppSettings.h"
#include "ui/Commands.h"
#include "ui/EditorPane.h"
#include "ui/KeyboardLayout.h"
#include "ui/Language.h"
#include "ui/MainWindow.h"
#include "ui/ManualWindow.h"
#include "ui/SpellChecker.h"
#include "speech-transcription/SpeechLogic.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QMenu>
#include <QMenuBar>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>

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

static QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

static const QStringList kCodes = {QStringLiteral("en"), QStringLiteral("de"), QStringLiteral("es"), QStringLiteral("fr"),
                                   QStringLiteral("pt"), QStringLiteral("it"), QStringLiteral("ru"), QStringLiteral("id"),
                                   QStringLiteral("tr"), QStringLiteral("pl"), QStringLiteral("nl")};

static QString menuText(QString t)
{
    const int tab = int(t.indexOf(QLatin1Char('\t')));
    if (tab >= 0)
        t.truncate(tab);
    t.remove(QLatin1Char('&'));
    t.remove(QChar(0x2026));
    t.remove(QLatin1String("..."));
    return t.trimmed();
}

// The menu places "**A > B > C**" of a text: each must be in the menus (as tests/v040_tests.cpp does for English).
static QStringList missingMenuPlaces(MainWindow *w, const QString &text, int *count)
{
    static const QRegularExpression pathRe(QStringLiteral("\\*\\*([^*\\n]*? > [^*\\n]+)\\*\\*"));
    QStringList tops;
    for (QAction *a : w->menuBar()->actions())
        tops << menuText(a->text());
    QStringList missing;
    QSet<QString> seen;
    auto it = pathRe.globalMatch(text);
    while (it.hasNext()) {
        const QString raw = it.next().captured(1);
        QStringList parts = raw.split(QStringLiteral(" > "));
        for (QString &x : parts)
            x = menuText(x);
        if (!tops.contains(parts.first()) || seen.contains(parts.join(QLatin1Char('>'))))
            continue;
        seen.insert(parts.join(QLatin1Char('>')));
        QList<QAction *> level = w->menuBar()->actions();
        for (int i = 0; i < parts.size(); ++i) {
            QAction *found = nullptr;
            for (QAction *a : level)
                if (menuText(a->text()) == parts[i]) {
                    found = a;
                    break;
                }
            if (!found) {
                missing << raw;
                break;
            }
            if (!found->menu())
                break; // a command; what follows is a page of its window
            emit found->menu()->aboutToShow();
            level = found->menu()->actions();
        }
    }
    *count = int(seen.size());
    return missing;
}

static QString withoutNumber(QString t)
{
    return t.remove(QRegularExpression(QStringLiteral("^[0-9]+(\\.[0-9]+)*\\.?\\s+"))).trimmed().toLower();
}

// ------------------------------------------------------------------ one language in a program of its own

static int child(const QString &code)
{
    language::install();
    AppSettings &as = AppSettings::instance();
    as.load();
    QTemporaryDir docs;
    as.welcomeFolder = docs.path();
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.theme = QStringLiteral("light");
    as.save();
    MainWindow w;
    w.resize(1400, 950);
    w.show();
    w.activateWindow();
    settle(400);
    std::printf("active=%s\n", qPrintable(language::active()));
    std::printf("file=%s\n", qPrintable(menuText(w.menuBar()->actions().value(0)->text())));

    // the manual: a section by its English name, shown in the language's own text
    const QString english = QString::fromUtf8(readAll(QStringLiteral(":/docs/manual.md")));
    const QStringList enHeads = ManualWindow::headingsOf(english);
    auto indexOfEnglish = [&](const QString &name) {
        const QString want = name.toLower();
        for (int pass = 0; pass < 3; ++pass)
            for (int i = 0; i < enHeads.size(); ++i) {
                const QString t = withoutNumber(enHeads[i]);
                if ((pass == 0 && t == want) || (pass == 1 && t.startsWith(want)) || (pass == 2 && t.contains(want)))
                    return i;
            }
        return -1;
    };
    showManualAt(&w, QStringLiteral("Options"));
    settle(500);
    ManualWindow *mw = manualWindow();
    const int optIndex = indexOfEnglish(QStringLiteral("Options"));
    const QStringList heads = mw ? mw->headings() : QStringList();
    std::printf("headings=%d\nenglishHeadings=%d\n", int(heads.size()), int(enHeads.size()));
    std::printf("manualTop=%s\nmanualWant=%s\n", mw ? qPrintable(mw->topHeading()) : "(none)", optIndex >= 0 && optIndex < heads.size() ? qPrintable(heads[optIndex]) : "(none)");
    std::printf("manualTitle=%s\n", mw ? qPrintable(mw->windowTitle()) : "");
    if (mw) {
        mw->resize(1000, 760);
        mw->grab().save(outDir + QStringLiteral("/v0950g-manual-%1.png").arg(code));
        mw->close();
        settle(100);
    }

    // the welcome project in this language
    w.openWelcomeProject(false);
    settle(1500);
    Project *p = w.project();
    std::printf("welcome=%s\n", p ? "yes" : "no");
    if (p) {
        std::printf("welcomeTitle=%s\nwelcomeItems=%d\n", qPrintable(p->title()), int(p->allItems().size()));
        QString allText;
        int words = 0, raw = 0;
        for (BinderItem *it : p->allItems())
            if (it->hasText()) {
                const QString t = md::write(p->docs()->text(it->id), p->formatting);
                allText += t + QLatin1Char('\n');
                words += int(t.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size());
                if (t.contains(QLatin1String("{{")) || t.contains(QLatin1String("qb-title:")) || t.contains(QLatin1String("qbobj-name:")))
                    ++raw;
            }
        std::printf("welcomeWords=%d\nwelcomeRaw=%d\n", words, raw);
        int n = 0;
        const QStringList missing = missingMenuPlaces(&w, allText, &n);
        std::printf("welcomePlaces=%d\nwelcomeMissing=%s\n", n, qPrintable(missing.join(QStringLiteral("; "))));
        // F1 from the corkboard view: the section "Corkboard", found by its place
        QAction *cork = w.findChild<QAction *>(QStringLiteral("viewCork"));
        if (cork)
            cork->trigger();
        settle(500);
        std::printf("f1Section=%s\n", qPrintable(w.manualSection()));
        QAction *man = w.findChild<QAction *>(QStringLiteral("manual"));
        if (man)
            man->trigger();
        settle(600);
        mw = manualWindow();
        const int ci = indexOfEnglish(w.manualSection());
        const QStringList h2 = mw ? mw->headings() : QStringList();
        std::printf("f1Top=%s\nf1Want=%s\n", mw ? qPrintable(mw->topHeading()) : "(none)", ci >= 0 && ci < h2.size() ? qPrintable(h2[ci]) : "(none)");
        w.grab().save(outDir + QStringLiteral("/v0950g-welcome-%1.png").arg(code));
        if (mw)
            mw->close();
    }
    // the quick start guide's menu places
    {
        QString quick = QString::fromUtf8(readAll(language::docResource(QStringLiteral("quick-start.md"))));
        quick.replace(QLatin1String("{{APP}}"), QLatin1String(QB_APP_NAME));
        int n = 0;
        const QStringList missing = missingMenuPlaces(&w, quick, &n);
        std::printf("quickPlaces=%d\nquickMissing=%s\n", n, qPrintable(missing.join(QStringLiteral("; "))));
    }
    // the inspector's entries in Navigate > Inspect: one letter each, in this language
    {
        QStringList letters, texts;
        for (int i = 0; i < 7; ++i) {
            QAction *a = w.findChild<QAction *>(QStringLiteral("inspect%1").arg(i));
            if (!a)
                continue;
            texts << a->text();
            const int at = int(a->text().indexOf(QLatin1Char('&')));
            letters << (at >= 0 && at + 1 < a->text().size() ? QString(a->text().at(at + 1).toLower()) : QString());
        }
        QSet<QString> uniq(letters.begin(), letters.end());
        std::printf("mnemonics=%d\nmnemonicLetters=%d\nmnemonicTexts=%s\n", int(letters.size()), int(uniq.size()),
                    qPrintable(texts.join(QStringLiteral(" | "))));
        // a mnemonic is the same letter in the menu text once; empty ones do not count
        std::printf("mnemonicEmpty=%d\n", int(letters.count(QString())));
    }
    return 0;
}

static QHash<QString, QString> runChild(const QString &code, const QString &outShots)
{
    QTemporaryDir home;
    QProcess p;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("HOME"), home.path());
    env.insert(QStringLiteral("XDG_CONFIG_HOME"), home.path() + QStringLiteral("/config"));
    env.insert(QStringLiteral("XDG_DATA_HOME"), home.path() + QStringLiteral("/data"));
    env.insert(QStringLiteral("QB_LANGUAGE"), code);
    p.setProcessEnvironment(env);
    p.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--child"), code, outShots});
    if (!p.waitForFinished(180000))
        p.kill();
    QHash<QString, QString> out;
    for (const QString &l : QString::fromUtf8(p.readAllStandardOutput()).split(QLatin1Char('\n'))) {
        const int eq = int(l.indexOf(QLatin1Char('=')));
        if (eq > 0)
            out.insert(l.left(eq), l.mid(eq + 1).trimmed());
    }
    if (out.isEmpty())
        out.insert(QStringLiteral("error"), QString::fromUtf8(p.readAllStandardError()).left(500));
    return out;
}

static void everyLanguage()
{
    std::printf("== every language in a program of its own\n");
    const QStringList fileMenu = {QStringLiteral("File"), QStringLiteral("Datei"), QStringLiteral("Archivo"), QStringLiteral("Fichier"),
                                  QStringLiteral("Arquivo"), QStringLiteral("File"), QString(), QString(), QString(), QString(), QString()};
    for (const QString &code : kCodes) {
        const QHash<QString, QString> r = runChild(code, outDir);
        const QString d = r.value(QStringLiteral("error"));
        check(r.value(QStringLiteral("active")) == (code == QLatin1String("en") ? QStringLiteral("en") : code) && !r.contains(QStringLiteral("error")),
              QStringLiteral("%1: the program starts in the language (first menu: %2)").arg(code, r.value(QStringLiteral("file"))), d);
        if (code != QLatin1String("en"))
            check(r.value(QStringLiteral("file")) != QLatin1String("File") || code == QLatin1String("it"),
                  QStringLiteral("%1: the menus are translated").arg(code), r.value(QStringLiteral("file")));
        (void)fileMenu;
        check(r.value(QStringLiteral("headings")) == r.value(QStringLiteral("englishHeadings")) && r.value(QStringLiteral("headings")).toInt() > 100,
              QStringLiteral("%1: the manual window lists the %2 headings of the English manual").arg(code, r.value(QStringLiteral("headings"))));
        check(!r.value(QStringLiteral("manualWant")).isEmpty() && r.value(QStringLiteral("manualWant")) != QLatin1String("(none)")
                  && r.value(QStringLiteral("manualTop")) == r.value(QStringLiteral("manualWant")),
              QStringLiteral("%1: the section \"Options\" opens at its own heading (%2)").arg(code, r.value(QStringLiteral("manualWant"))),
              r.value(QStringLiteral("manualTop")));
        check(r.value(QStringLiteral("welcome")) == QLatin1String("yes") && r.value(QStringLiteral("welcomeRaw")) == QLatin1String("0")
                  && r.value(QStringLiteral("welcomeItems")).toInt() >= 15 && r.value(QStringLiteral("welcomeWords")).toInt() > 700,
              QStringLiteral("%1: the welcome project is made (%2 items, %3 words), no text holds an unresolved link or placeholder")
                  .arg(code, r.value(QStringLiteral("welcomeItems")), r.value(QStringLiteral("welcomeWords"))),
              r.value(QStringLiteral("welcomeRaw")) + QLatin1Char(' ') + r.value(QStringLiteral("error")));
        check(r.value(QStringLiteral("welcomePlaces")).toInt() >= 10 && r.value(QStringLiteral("welcomeMissing")).isEmpty(),
              QStringLiteral("%1: every menu place the welcome project names is in that language's menus (%2)").arg(code, r.value(QStringLiteral("welcomePlaces"))),
              r.value(QStringLiteral("welcomeMissing")));
        check(r.value(QStringLiteral("quickPlaces")).toInt() >= 10 && r.value(QStringLiteral("quickMissing")).isEmpty(),
              QStringLiteral("%1: every menu place the quick start guide names is in the menus (%2)").arg(code, r.value(QStringLiteral("quickPlaces"))),
              r.value(QStringLiteral("quickMissing")));
        check(!r.value(QStringLiteral("f1Want")).isEmpty() && r.value(QStringLiteral("f1Want")) != QLatin1String("(none)")
                  && r.value(QStringLiteral("f1Top")) == r.value(QStringLiteral("f1Want")),
              QStringLiteral("%1: F1 in the corkboard view opens the manual at \"%2\"").arg(code, r.value(QStringLiteral("f1Want"))),
              r.value(QStringLiteral("f1Section")) + QStringLiteral(" / ") + r.value(QStringLiteral("f1Top")));
        check(r.value(QStringLiteral("mnemonics")) == QLatin1String("7") && r.value(QStringLiteral("mnemonicLetters")) == QLatin1String("7")
                  && r.value(QStringLiteral("mnemonicEmpty")) == QLatin1String("0"),
              QStringLiteral("%1: Navigate > Inspect has a letter of its own for each of the seven tabs").arg(code), r.value(QStringLiteral("mnemonicTexts")));
    }
}

// ------------------------------------------------------------------ the dictionary packages

static void dictionaries(const QString &src)
{
    std::printf("== the spelling dictionary packages\n");
    struct D {
        const char *code, *variant, *good, *bad;
    };
    const D list[] = {{"es", "es_ES", "casa", "casz"}, {"fr", "fr_FR", "maison", "maisonn"}, {"pt", "pt_BR", "casa", "casz"},
                      {"it", "it_IT", "casa", "casz"}, {"ru", "ru_RU", "дом", "домм"}, {"id", "id_ID", "rumah", "rumahh"},
                      {"tr", "tr_TR", "ev", "evv"}, {"pl", "pl_PL", "gęś", "gęśź"}, {"nl", "nl_NL", "huis", "huisz"}};
    QTemporaryDir tmp;
    QStringList notBuilt;
    for (const D &d : list) {
        const QString code = QLatin1String(d.code), variant = QLatin1String(d.variant);
        const QString folder = src + QStringLiteral("/addons/dictionary-") + code;
        const QByteArray manifest = readAll(folder + QStringLiteral("/manifest.json"));
        const QString readme = QString::fromUtf8(readAll(folder + QStringLiteral("/README.md")));
        const QString sources = QString::fromUtf8(readAll(folder + QStringLiteral("/sources.txt")));
        check(manifest.contains("\"dictionary-" + code.toUtf8() + "\"") && readme.contains(QLatin1String("License")) && readme.contains(QLatin1String("libreoffice-25.8.7.3"))
                  && sources.contains(QLatin1String("tag libreoffice-25.8.7.3")) && sources.contains(variant + QLatin1String(".dic")),
              QStringLiteral("%1: the add-on has its manifest, its README with the license and its pinned source").arg(code));
        // the files, fetched and checked by the build script, into a package
        const QString stage = tmp.path() + QStringLiteral("/stage-") + code;
        QDir().mkpath(stage);
        QFile::copy(folder + QStringLiteral("/manifest.json"), stage + QStringLiteral("/manifest.json"));
        QProcess st;
        st.start(QStringLiteral("bash"), {folder + QStringLiteral("/stage.sh"), QStringLiteral("x"), stage});
        st.waitForFinished(300000);
        const QString dictDir = stage + QStringLiteral("/payload/dictionaries");
        const QStringList files = QDir(dictDir).entryList(QDir::Files);
        bool hasLicense = false;
        for (const QString &f : files)
            hasLicense = hasLicense || f.contains(QLatin1String("LICENSE")) || f.contains(QLatin1String("LGPL")) || f.contains(QLatin1String("MPL"))
                         || f.contains(QLatin1String("README"));
        check(st.exitCode() == 0 && files.contains(variant + QStringLiteral(".aff")) && files.contains(variant + QStringLiteral(".dic")) && hasLicense,
              QStringLiteral("%1: the build script fetches %2.aff and .dic (checked against their SHA-256 sums) and the license and README files (%3 files)")
                  .arg(code, variant).arg(files.size()),
              QString::fromUtf8(st.readAllStandardError()));
        const QString pkg = tmp.path() + QStringLiteral("/dictionary-") + code + QStringLiteral(".qbpkg");
        const int rc = QProcess::execute(QStringLiteral("python3"), {QStringLiteral("-I"), src + QStringLiteral("/tools/make-package.py"), QStringLiteral("addon"), stage, pkg});
        const addons::PackageInfo info = addons::inspect(pkg);
        check(rc == 0 && info.isValid() && info.provides.join(QLatin1Char(' ')).contains(QLatin1String("dictionar")),
              QStringLiteral("%1: the package is valid (%2)").arg(code, info.provides.join(QStringLiteral(", "))), info.error);
        QStringList log;
        QString err;
        bool restart = false;
        const bool installed = addons::install(pkg, &log, &err, &restart);
        SpellChecker &sc = SpellChecker::instance();
        const bool found = sc.availableLanguages().contains(variant);
#ifdef QB_HAVE_HUNSPELL
        const bool set = sc.setLanguage(variant);
        const bool right = set && sc.isCorrect(QString::fromUtf8(d.good)) && !sc.isCorrect(QString::fromUtf8(d.bad));
#else
        const bool right = true;
#endif
        check(installed && found && right, QStringLiteral("%1: it installs, %2 is in the list of spelling languages and checks words (\"%3\" right, \"%4\" wrong)")
                                               .arg(code, variant, QString::fromUtf8(d.good), QString::fromUtf8(d.bad)),
              err + QLatin1Char(' ') + QString::number(installed) + QLatin1Char(' ') + QString::number(found));
    }
}

// ------------------------------------------------------------------ the speech tool

static void speechLists(const QString &src)
{
    std::printf("== Speech Transcription: the language lists\n");
    const auto langs = speech::supportedLanguages();
    QStringList codes;
    for (const auto &l : langs)
        codes << l.code;
    check(codes == kCodes && speech::languageName(QStringLiteral("es")) == QLatin1String("Spanish") && speech::languageName(QStringLiteral("pt")) == QLatin1String("Portuguese")
              && speech::languageName(QStringLiteral("en")) == QLatin1String("English"),
          QStringLiteral("the main and fallback lists hold the eleven languages"), codes.join(QLatin1Char(' ')));
    const QString tool = QString::fromUtf8(readAll(src + QStringLiteral("/plugins/tools/speech-transcription/SpeechTool.cpp")));
    check(tool.contains(QLatin1String("speech::supportedLanguages()")) && !tool.contains(QLatin1String("= speech::languages();"))
              && tool.contains(QLatin1String("stays selectable")) && tool.contains(QLatin1String("QStringLiteral(\"main\"), QStringLiteral(\"en\")"))
              && tool.contains(QLatin1String("QStringLiteral(\"fallback\"), QStringLiteral(\"de\")")),
          QStringLiteral("the tool fills both drop-down lists from it, keeps a stored language selectable and its defaults en and de"));
    // the stored values en and de work with the choice logic as before
    const auto c = speech::chooseLanguage(QStringLiteral("en"), QStringLiteral("de"), 0.2f, 0.8f, true);
    check(c.language == QLatin1String("de") && c.translate, QStringLiteral("the stored values en and de still choose the language as before"));
    check(speech::languages().size() == 100, QStringLiteral("whisper's own list of 100 languages is kept for the logic"));
}

// ------------------------------------------------------------------ keyboard sets

static void keyboardSets(MainWindow &w)
{
    std::printf("== the keyboard sets of the new layouts (item 79 for item 102)\n");
    namespace kl = keyboardlayout;
    check(kl::sets() == QStringList({QStringLiteral("us"), QStringLiteral("de"), QStringLiteral("lt"), QStringLiteral("pl")}), QStringLiteral("four sets: us, de, lt, pl"));
    QStringList wrong;
    const QList<QPair<QString, QString>> want = {{QStringLiteral("de"), QStringLiteral("de")}, {QStringLiteral("at"), QStringLiteral("de")},
                                                 {QStringLiteral("fr"), QStringLiteral("lt")}, {QStringLiteral("be"), QStringLiteral("lt")},
                                                 {QStringLiteral("es"), QStringLiteral("lt")}, {QStringLiteral("it"), QStringLiteral("lt")},
                                                 {QStringLiteral("br"), QStringLiteral("lt")}, {QStringLiteral("pt"), QStringLiteral("lt")},
                                                 {QStringLiteral("tr"), QStringLiteral("lt")}, {QStringLiteral("nl"), QStringLiteral("lt")},
                                                 {QStringLiteral("pl"), QStringLiteral("pl")}, {QStringLiteral("ru"), QStringLiteral("us")},
                                                 {QStringLiteral("id"), QStringLiteral("us")}, {QStringLiteral("us"), QStringLiteral("us")},
                                                 {QStringLiteral("en"), QStringLiteral("us")}, {QStringLiteral("ch"), QStringLiteral("us")},
                                                 {QString(), QStringLiteral("us")}};
    for (const auto &p : want)
        if (kl::setForLayout(p.first) != p.second)
            wrong << p.first + QStringLiteral("=") + kl::setForLayout(p.first);
    check(wrong.isEmpty(), QStringLiteral("each layout of the languages gets its set from the one function"), wrong.join(QLatin1Char(' ')));
    // the detection with the test switch, as before
    qputenv("QB_KEYBOARD_LAYOUT", "fr");
    check(kl::setForLayout(kl::systemLayout()) == QLatin1String("lt"), QStringLiteral("... the test switch QB_KEYBOARD_LAYOUT=fr gives the set \"lt\""));
    qputenv("QB_KEYBOARD_LAYOUT", "us");

    QSet<QString> keys;
    for (const auto &c : commands::all())
        keys.insert(c.key);
    for (const QString &set : {QStringLiteral("lt"), QStringLiteral("pl")}) {
        const QMap<QString, QString> table = kl::setTable(set);
        QStringList problems;
        for (auto it = table.begin(); it != table.end(); ++it) {
            const QString t = it.value();
            if (!it.key().startsWith(QLatin1String("mm_")) && !keys.contains(it.key()))
                problems << QStringLiteral("no command ") + it.key();
            if (t.contains(QLatin1String("Ctrl+Alt")) || t.contains(QRegularExpression(QStringLiteral("[{}|\\[\\]\\\\=~@<>]"))))
                problems << QStringLiteral("AltGr key ") + t;
            if (QKeySequence(t, QKeySequence::PortableText).isEmpty())
                problems << QStringLiteral("empty ") + it.key();
        }
        QHash<QString, QString> all;
        for (const auto &c : commands::all()) {
            const QKeySequence k = kl::standardFor(c.key, c.standard, set);
            if (k.isEmpty())
                continue;
            const QString t = k.toString(QKeySequence::PortableText);
            if (all.contains(t))
                problems << QStringLiteral("clash %1: %2 / %3").arg(t, all.value(t), c.key);
            all.insert(t, c.key);
            // a letter after Ctrl+Alt is AltGr's letter on Windows keyboards: the commands that those layouts' AltGr letters collide with
            const QStringList taken = set == QLatin1String("pl") ? QStringList({QStringLiteral("Ctrl+Alt+C"), QStringLiteral("Ctrl+Alt+N")})
                                                                 : QStringList({QStringLiteral("Ctrl+Alt+C"), QStringLiteral("Ctrl+Alt+N"),
                                                                                QStringLiteral("Ctrl+Alt+R"), QStringLiteral("Ctrl+Alt+V")});
            if (taken.contains(t, Qt::CaseInsensitive))
                problems << QStringLiteral("%1 still has %2").arg(c.key, t);
        }
        check(!table.isEmpty() && problems.isEmpty(), QStringLiteral("set \"%1\" (%2 commands): no AltGr key, no clash with %3 shortcuts, nothing left on Ctrl+Alt+C/N%4")
                                                          .arg(set).arg(table.size()).arg(all.size()).arg(set == QLatin1String("lt") ? QStringLiteral("/R/V") : QString()),
              problems.join(QStringLiteral("; ")));
    }
    check(kl::standardFor(QStringLiteral("alignLeft"), QKeySequence(Qt::CTRL | Qt::Key_BraceLeft), QStringLiteral("lt")) == QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_L)
              && kl::standardFor(QStringLiteral("alignLeft"), QKeySequence(Qt::CTRL | Qt::Key_BraceLeft), QStringLiteral("pl")) == QKeySequence(Qt::CTRL | Qt::Key_BraceLeft)
              && kl::standardFor(QStringLiteral("copyFormat"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_C), QStringLiteral("pl")) == QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Y)
              && kl::standardFor(QStringLiteral("copyFormat"), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_C), QStringLiteral("us")) == QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_C),
          QStringLiteral("French-like layouts get the letters set; Polish keeps the braces (its keys are the US ones) and moves only the colliding commands"));
    (void)w;
}

// ------------------------------------------------------------------ main

int main(int argc, char **argv)
{
    const bool isChild = argc > 3 && QByteArray(argv[1]) == "--child";
    QTemporaryDir home;
    if (!isChild) {
        qputenv("HOME", (home.path() + QLatin1String("/h")).toUtf8());
        qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
        qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
        QDir().mkpath(home.path() + QLatin1String("/h"));
        qunsetenv("QB_LANGUAGE");
        qputenv("QB_KEYBOARD_LAYOUT", "us");
    }
    qputenv("QB_NO_DIALOGS", "1");
    qputenv("QB_NO_UPDATE_CHECK", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0950gTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0950gTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    if (isChild) {
        outDir = QString::fromLocal8Bit(argv[3]);
        return child(QString::fromLocal8Bit(argv[2]));
    }
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    const QString src = QStringLiteral(QB_TEST_SOURCE_DIR);
    AppSettings &as = AppSettings::instance();
    as.load();
    as.welcomeShown = true;
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();

    everyLanguage();
    dictionaries(src);
    speechLists(src);
    {
        MainWindow w;
        w.show();
        settle(300);
        keyboardSets(w);
    }

    std::printf("\n%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
