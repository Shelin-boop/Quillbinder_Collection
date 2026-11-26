// Scripted run for version 0.94.0 (shared code, third part of the run's to-do list):
//  - item 87: one name and one key per command (Navigate > Inspect lists every inspector tab under its tooltip's name, the
//    timestamp command has one name in both menus), and a check over all menus: no shortcut twice, no Alt letter twice in
//    one menu;
//  - item 84: View > Quick-Add Keys in the Binder (off at first: Return opens the document, Space is the type-ahead's);
//  - item 79: the shortcuts out of the box follow the keyboard layout (US set, German set without AltGr), chosen at the
//    first start and kept; a changed shortcut stays.
//
//   xvfb-run -a ./qb_v0940c_tests /tmp/shots
#include "AppInfo.h"
#include "core/BinderModel.h"
#include "core/Project.h"
#include "core/ProjectTemplates.h"
#include "ui/AppSettings.h"
#include "ui/BinderPanel.h"
#include "ui/Commands.h"
#include "ui/EditorPane.h"
#include "ui/Inspector.h"
#include "ui/KeyboardLayout.h"
#include "ui/MainWindow.h"
#include "ui/QuickRef.h"
#include "ui/Theme.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QElapsedTimer>
#include <QFile>
#include <QMenu>
#include <QMenuBar>
#include <QProcess>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>
#include <QToolButton>
#include <QTreeView>

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

static QString plain(QString t)
{
    const int tab = int(t.indexOf(QLatin1Char('\t')));
    if (tab >= 0)
        t.truncate(tab);
    t.remove(QLatin1Char('&'));
    t.remove(QChar(0x2026));
    return t.simplified();
}

static QMenu *topMenu(MainWindow &w, const QString &name)
{
    for (QAction *a : w.menuBar()->actions())
        if (a->menu() && plain(a->text()) == name)
            return a->menu();
    return nullptr;
}

static QMenu *subMenu(QMenu *m, const QString &name)
{
    if (!m)
        return nullptr;
    for (QAction *a : m->actions())
        if (a->menu() && plain(a->text()) == name)
            return a->menu();
    return nullptr;
}

static QStringList texts(QMenu *m)
{
    QStringList out;
    if (m)
        for (QAction *a : m->actions())
            if (!a->isSeparator() && !a->text().isEmpty())
                out << plain(a->text());
    return out;
}

// every menu of the window, the sub-menus too (opened once, so that the lists made on opening are in)
static void collectMenus(QMenu *m, QList<QMenu *> *out)
{
    emit m->aboutToShow();
    settle(20);
    *out << m;
    for (QAction *a : m->actions())
        if (a->menu())
            collectMenus(a->menu(), out);
}

static QList<QMenu *> allMenus(MainWindow &w)
{
    QList<QMenu *> out;
    for (QAction *a : w.menuBar()->actions())
        if (a->menu())
            collectMenus(a->menu(), &out);
    return out;
}

static void collectStatic(QMenu *m, QList<QMenu *> *out)
{
    *out << m;
    for (QAction *a : m->actions())
        if (a->menu())
            collectStatic(a->menu(), out);
}

static QString mnemonicOf(const QString &text)
{
    const QString main = text.left(text.indexOf(QLatin1Char('\t')) < 0 ? text.size() : int(text.indexOf(QLatin1Char('\t'))));
    for (int i = 0; i + 1 < main.size(); ++i)
        if (main.at(i) == QLatin1Char('&')) {
            if (main.at(i + 1) == QLatin1Char('&')) {
                ++i;
                continue;
            }
            return main.at(i + 1).toLower();
        }
    return {};
}


// ---------------------------------------------------------------- item 87
static void names(MainWindow &w)
{
    std::printf("== one name and one key per command (item 87)\n");
    QMenu *nav = topMenu(w, QStringLiteral("Navigate"));
    QMenu *inspect = subMenu(nav, QStringLiteral("Inspect"));
    emit inspect->aboutToShow();
    const QStringList it = texts(inspect);
    auto *inspector = w.findChild<Inspector *>();
    QStringList want;
    for (Inspector::Tab t : {Inspector::Notes, Inspector::Bookmarks, Inspector::Metadata, Inspector::Snapshots, Inspector::Comments,
                             Inspector::MindMap, Inspector::Objects})
        want << plain(Inspector::tabToolTip(t));
    for (const QString &key : inspector->extraTabs())
        want << plain(inspector->extraTabToolTip(key));
    bool all = true;
    for (const QString &t : want)
        all = all && it.contains(t);
    check(all && want.size() >= 7, QStringLiteral("Navigate > Inspect lists every inspector tab under its tooltip's name"),
          it.join(QStringLiteral(" | ")));
    check(!it.contains(QStringLiteral("Notes")) && it.contains(QStringLiteral("Synopsis and notes")),
          QStringLiteral("the first tab is \"Synopsis and notes\", not \"Notes\""));
    // the tab buttons of the inspector carry the same names
    QStringList tips;
    for (QToolButton *b : inspector->findChildren<QToolButton *>())
        if (b->isCheckable() && !b->toolTip().isEmpty())
            tips << plain(b->toolTip());
    bool same = true;
    for (const QString &t : want)
        same = same && tips.contains(t);
    check(same, QStringLiteral("... the names of the tab buttons' tooltips"), tips.join(QStringLiteral(" | ")));
    // each entry shows its tab
    inspector->showTab(Inspector::Notes);
    int shown = 0;
    for (QAction *a : inspect->actions())
        if (plain(a->text()) == plain(Inspector::tabToolTip(Inspector::Snapshots))) {
            a->trigger();
            shown = inspector->currentTab() == int(Inspector::Snapshots);
        }
    check(shown, QStringLiteral("the entry for Snapshots shows the Snapshots tab"));
    shot(inspect, QStringLiteral("menu-inspect"));
    {
        auto *pop = inspect;
        pop->popup(w.mapToGlobal(QPoint(120, 60)));
        settle(200);
        pop->grab().save(outDir + QLatin1String("/menu-inspect-open.png"));
        pop->hide();
    }

    // the timestamp command: one action, one name, in Insert and in Navigate > Media
    QAction *ts = w.findChild<QAction *>(QStringLiteral("mediaTimestamp"));
    QMenu *insert = topMenu(w, QStringLiteral("Insert"));
    QMenu *media = subMenu(nav, QStringLiteral("Media"));
    QAction *inInsert = nullptr, *inMedia = nullptr;
    for (QAction *a : insert->actions())
        if (a->property("qbAliasOf").value<QObject *>() == ts)
            inInsert = a;
    for (QAction *a : media->actions())
        if (a == ts)
            inMedia = a;
    check(ts && inInsert && inMedia && plain(inInsert->text()) == plain(inMedia->text()) && plain(ts->text()) == QStringLiteral("Insert Media Timestamp"),
          QStringLiteral("Insert > Insert Media Timestamp and Navigate > Media > Insert Media Timestamp are one action with one name"),
          inInsert ? inInsert->text() : QString());
    check(ts && ts->shortcut() == QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return) && inInsert && inInsert->shortcut().isEmpty(),
          QStringLiteral("... Ctrl+Shift+Return belongs to the one action (the entry in Insert only shows it)"));
}

static void audit(MainWindow &w)
{
    std::printf("== all menus: no shortcut twice, no Alt letter twice (item 87)\n");
    // the menus as they are made (before any is opened): the letters are right from the start, not only after the program
    // has repaired them when a menu opens
    {
        QList<QMenu *> built;
        for (QAction *a : w.menuBar()->actions())
            if (a->menu())
                collectStatic(a->menu(), &built);
        QStringList clash0;
        for (QMenu *m : built) {
            QHash<QString, QString> letters;
            for (QAction *a : m->actions()) {
                const QString l = a->isSeparator() ? QString() : mnemonicOf(a->text());
                if (l.isEmpty())
                    continue;
                if (letters.contains(l))
                    clash0 << QStringLiteral("%1: %2 / %3 (%4)").arg(plain(m->title()), letters.value(l), plain(a->text()), l);
                letters.insert(l, plain(a->text()));
            }
        }
        check(clash0.isEmpty(), QStringLiteral("menus as made: no Alt letter twice in one menu (%1 menus)").arg(built.size()), clash0.join(QStringLiteral("; ")));
    }
    const QList<QMenu *> menus = allMenus(w);
    // shortcuts: every action of the window that has one (an entry that stands for another one has none of its own)
    QHash<QString, QStringList> byKey;
    QSet<QAction *> seen;
    for (QAction *a : w.findChildren<QAction *>()) {
        if (a->property("qbAliasOf").isValid() || seen.contains(a))
            continue;
        seen.insert(a);
        for (const QKeySequence &k : a->shortcuts())
            if (!k.isEmpty())
                byKey[k.toString(QKeySequence::PortableText)] << (a->objectName().isEmpty() ? plain(a->text()) : a->objectName());
    }
    QStringList twice;
    for (auto it = byKey.begin(); it != byKey.end(); ++it)
        if (it.value().size() > 1)
            twice << it.key() + QLatin1String(" = ") + it.value().join(QLatin1String(", "));
    check(twice.isEmpty(), QStringLiteral("no two actions share a shortcut (%1 shortcuts)").arg(byKey.size()), twice.join(QStringLiteral("; ")));
    // the commands' list (Options > Keyboard) has no key twice either
    QHash<QString, QStringList> byCmd;
    for (const auto &c : commands::all()) {
        const QKeySequence k = commands::shortcutOf(c.key);
        if (!k.isEmpty())
            byCmd[k.toString(QKeySequence::PortableText)] << c.key;
    }
    QStringList cmdTwice;
    for (auto it = byCmd.begin(); it != byCmd.end(); ++it)
        if (it.value().size() > 1)
            cmdTwice << it.key() + QLatin1String(" = ") + it.value().join(QLatin1String(", "));
    check(cmdTwice.isEmpty(), QStringLiteral("no two commands of the list share a shortcut"), cmdTwice.join(QStringLiteral("; ")));
    // Alt letters: in each menu (hidden entries count: they may show later) and in the menu bar
    QStringList clash;
    int entries = 0;
    for (QMenu *m : menus) {
        QHash<QString, QString> letters;
        for (QAction *a : m->actions()) {
            if (a->isSeparator())
                continue;
            ++entries;
            const QString l = mnemonicOf(a->text());
            if (l.isEmpty())
                continue;
            if (letters.contains(l))
                clash << QStringLiteral("%1: %2 / %3 (%4)").arg(plain(m->title()), letters.value(l), plain(a->text()), l);
            letters.insert(l, plain(a->text()));
        }
    }
    check(clash.isEmpty(), QStringLiteral("no menu has two entries with the same Alt letter (%1 menus, %2 entries)").arg(menus.size()).arg(entries),
          clash.join(QStringLiteral("; ")));
    QStringList bar;
    QSet<QString> top;
    for (QAction *a : w.menuBar()->actions()) {
        const QString l = mnemonicOf(a->text());
        if (!l.isEmpty() && top.contains(l))
            bar << plain(a->text());
        top.insert(l);
    }
    check(bar.isEmpty(), QStringLiteral("the menu bar's Alt letters are unique"), bar.join(QStringLiteral("; ")));
}

// ---------------------------------------------------------------- item 84
static QList<QuickRefWindow *> quickRefs()
{
    QList<QuickRefWindow *> out;
    for (QWidget *tl : QApplication::topLevelWidgets())
        if (auto *q = qobject_cast<QuickRefWindow *>(tl))
            out << q;
    return out;
}

static void closeQuickRefs()
{
    for (QuickRefWindow *q : quickRefs())
        q->close();
    settle(100);
}

static void binderKeys(MainWindow &w, Project *p)
{
    std::printf("== View > Quick-Add Keys in the Binder (item 84)\n");
    QMenu *view = topMenu(w, QStringLiteral("View"));
    QAction *opt = w.findChild<QAction *>(QStringLiteral("binderScrivenerKeys"));
    check(opt && opt->isCheckable() && !opt->isChecked() && !AppSettings::instance().binderScrivenerKeys && view && view->actions().contains(opt),
          QStringLiteral("View > Quick-Add Keys in the Binder is a checkable entry, unchecked by default"));
    check(opt && plain(opt->text()) == QStringLiteral("Quick-Add Keys in the Binder"), QStringLiteral("... under that name"));

    auto *binder = w.findChild<BinderPanel *>();
    BinderTree *tree = binder->tree();
    BinderItem *a = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Zeta one"));
    BinderItem *b = p->model()->createItem(p->draft(), -1, ItemType::Text, QStringLiteral("Zeta two"));
    settle(300);
    auto selectOnly = [&](BinderItem *it) {
        binder->select({it}, true);
        tree->setFocus();
        w.activateWindow();
        settle(200);
    };
    auto countItems = [&] { return p->allItems().size(); };
    EditorPane *pane = nullptr;
    for (EditorPane *e : w.findChildren<EditorPane *>())
        if (e->isVisible() && !pane)
            pane = e;

    // ---- unchecked: Return opens the document and the keyboard goes into the editor
    selectOnly(a);
    const qsizetype before = countItems();
    QTest::keyClick(tree, Qt::Key_Return);
    settle(300);
    check(countItems() == before, QStringLiteral("unchecked: Return makes no new document"));
    check(pane->items() == QList<BinderItem *>({a}), QStringLiteral("unchecked: Return shows the selected document in the editor"));
    QWidget *fw = QApplication::focusWidget();
    check(fw && pane->isAncestorOf(fw) && fw != tree, QStringLiteral("unchecked: the keyboard focus is in the editor"),
          fw ? QString::fromLatin1(fw->metaObject()->className()) : QStringLiteral("(none)"));
    // Return on a document the editor does not show yet (the selection changed without loading it)
    selectOnly(b);
    QTest::keyClick(tree, Qt::Key_Return);
    settle(300);
    check(pane->items() == QList<BinderItem *>({b}) && pane->isAncestorOf(QApplication::focusWidget()),
          QStringLiteral("unchecked: Return on another document shows it and moves the focus"));
    // ---- unchecked: Space is the type-ahead's; a phrase with a space jumps to the item and opens no window
    closeQuickRefs();
    selectOnly(a);
    tree->setCurrentIndex(QModelIndex());
    tree->clearSelection();
    tree->setFocus();
    QTest::keyClicks(tree, QStringLiteral("Zeta t"));
    settle(300);
    check(quickRefs().isEmpty(), QStringLiteral("unchecked: typing \"Zeta t\" opens no Quick Reference window"), QString::number(quickRefs().size()));
    const QList<BinderItem *> sel = binder->selectedItems();
    check(sel == QList<BinderItem *>({b}), QStringLiteral("unchecked: ... the phrase with its space finds the document (type-ahead)"),
          sel.isEmpty() ? QStringLiteral("(none)") : sel.first()->title);
    QTest::keyClick(tree, Qt::Key_Space);
    settle(200);
    check(quickRefs().isEmpty(), QStringLiteral("unchecked: Space alone opens no Quick Reference window"));
    // ---- checked: today's behavior
    opt->trigger();
    settle(100);
    check(opt->isChecked() && AppSettings::instance().binderScrivenerKeys, QStringLiteral("the entry switches the option on"));
    selectOnly(a);
    const qsizetype n0 = countItems();
    QTest::keyClick(tree, Qt::Key_Space);
    settle(300);
    check(quickRefs().size() == 1 && quickRefs().first()->item() == a, QStringLiteral("checked: Space opens a Quick Reference window of the document"));
    closeQuickRefs();
    selectOnly(a);
    QTest::keyClick(tree, Qt::Key_Return);
    settle(300);
    check(countItems() == n0 + 1, QStringLiteral("checked: Return makes a new empty document below"), QString::number(countItems() - n0));
    {
        // the list of a collection or a search: Space opens a Quick Reference window while the option is on
        auto *list = binder->resultsList();
        check(list != nullptr, QStringLiteral("(the results list exists)"));
    }
    // the setting is kept
    AppSettings::instance().load();
    check(AppSettings::instance().binderScrivenerKeys, QStringLiteral("the option is saved with the settings"));
    opt->trigger();
    settle(100);
    AppSettings::instance().load();
    check(!AppSettings::instance().binderScrivenerKeys && !opt->isChecked(), QStringLiteral("... and switched off again"));
    view->popup(w.mapToGlobal(QPoint(220, 40)));
    settle(200);
    view->grab().save(outDir + QLatin1String("/menu-view.png"));
    view->hide();
}

// ---------------------------------------------------------------- item 79
static QStringList runChild(const QString &home, const QString &layout, const QStringList &extra = {})
{
    QProcess p;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("HOME"), home);
    env.insert(QStringLiteral("XDG_CONFIG_HOME"), home + QStringLiteral("/config"));
    env.insert(QStringLiteral("XDG_DATA_HOME"), home + QStringLiteral("/data"));
    env.insert(QStringLiteral("QB_KEYBOARD_LAYOUT"), layout);
#ifdef Q_OS_WIN
    // Windows: the data folder (QB_DATA_DIR; the runner sets one for the whole suite) is the child's own too
    env.insert(QStringLiteral("QB_DATA_DIR"), home + QStringLiteral("/data"));
#endif
    env.insert(QStringLiteral("QB_NO_WELCOME"), QStringLiteral("1"));
    env.insert(QStringLiteral("QB_NO_DIALOGS"), QStringLiteral("1"));
    p.setProcessEnvironment(env);
    p.setProcessChannelMode(QProcess::SeparateChannels);
    p.start(QCoreApplication::applicationFilePath(), QStringList{QStringLiteral("--child"), outDir} + extra);
    p.waitForFinished(120000);
    QStringList lines;
    for (QString l : QString::fromUtf8(p.readAllStandardOutput()).split(QLatin1Char('\n'))) {
        if (l.endsWith(QLatin1Char('\r'))) // Windows' text output
            l.chop(1);
        if (l.startsWith(QLatin1String("R ")))
            lines << l.mid(2);
    }
    return lines;
}

static QString val(const QStringList &lines, const QString &key)
{
    for (const QString &l : lines)
        if (l.startsWith(key + QLatin1Char('=')))
            return l.mid(int(key.size()) + 1);
    return QStringLiteral("(missing)");
}

// what a child process prints: the set, the shortcuts and the labels the menus show
static int childMain(MainWindow &w, const QStringList &args)
{
    std::printf("R set=%s\n", qPrintable(commands::layoutSet()));
    for (const char *k : {"alignLeft", "alignCenter", "alignRight", "back", "forward", "scriptElements", "subscript", "bold", "options"}) {
        QAction *a = w.findChild<QAction *>(QLatin1String(k));
        std::printf("R %s=%s\n", k, a ? qPrintable(a->shortcut().toString(QKeySequence::PortableText)) : "(none)");
        std::printf("R label.%s=%s\n", k, a ? qPrintable(a->shortcut().toString(QKeySequence::NativeText)) : "(none)");
    }
    if (args.value(0) == QLatin1String("shot")) {
        // the menu that holds the alignment commands, open, with the keys it shows
        QAction *left = w.findChild<QAction *>(QStringLiteral("alignLeft"));
        QList<QMenu *> menus;
        for (QAction *top : w.menuBar()->actions())
            if (top->menu())
                collectStatic(top->menu(), &menus);
        for (QMenu *m : menus)
            if (left && m->actions().contains(left)) {
                m->popup(w.mapToGlobal(QPoint(300, 30)));
                settle(300);
                m->grab().save(args.value(1));
                m->hide();
                break;
            }
    }
    return 0;
}

static void layoutSets(MainWindow &w)
{
    std::printf("== the shortcuts follow the keyboard layout (item 79)\n");
    namespace kl = keyboardlayout;
    // the detection: the test switch names the layout
    qputenv("QB_KEYBOARD_LAYOUT", "de");
    check(kl::systemLayout() == QLatin1String("de"), QStringLiteral("the test switch QB_KEYBOARD_LAYOUT names the layout"));
    qputenv("QB_KEYBOARD_LAYOUT", "none");
    check(kl::systemLayout().isEmpty(), QStringLiteral("... \"none\": the system does not tell"));
    qputenv("QB_KEYBOARD_LAYOUT", "us");
    check(kl::setForLayout(QStringLiteral("de")) == QLatin1String("de") && kl::setForLayout(QStringLiteral("at")) == QLatin1String("de")
              && kl::setForLayout(QStringLiteral("us")) == QLatin1String("us") && kl::setForLayout(QStringLiteral("ru")) == QLatin1String("us")
              && kl::setForLayout(QString()) == QLatin1String("us") && kl::setForLayout(QStringLiteral("ch")) == QLatin1String("us"),
          QStringLiteral("German-language layouts (de, at) get the German set (0.95.0: French, Spanish ... have sets of their own, see v0950g), a Russian layout and an unknown one the usual set"));
    // the German set needs no AltGr (no { | } [ ] \ = ~ @ < > |, no Ctrl+Alt) and clashes with nothing
    const QMap<QString, QString> german = kl::germanSet();
    QStringList altGr, bad;
    for (auto it = german.begin(); it != german.end(); ++it) {
        const QKeySequence k(it.value(), QKeySequence::PortableText);
        const QString t = it.value();
        if (t.contains(QLatin1String("Ctrl+Alt")) || t.contains(QRegularExpression(QStringLiteral("[{}|\\[\\]\\\\=~@<>]"))))
            altGr << it.key() + QLatin1Char('=') + t;
        if (k.isEmpty())
            bad << it.key();
    }
    check(!german.isEmpty() && altGr.isEmpty() && bad.isEmpty(), QStringLiteral("the German set (%1 commands) uses no AltGr and no Ctrl+Alt").arg(german.size()),
          (altGr + bad).join(QStringLiteral("; ")));
    QHash<QString, QString> all; // every shortcut of the German set's world
    QStringList clash;
    for (const auto &c : commands::all()) {
        const QKeySequence k = kl::standardFor(c.key, c.standard, QStringLiteral("de"));
        if (k.isEmpty())
            continue;
        const QString t = k.toString(QKeySequence::PortableText);
        if (all.contains(t))
            clash << t + QLatin1String(": ") + all.value(t) + QLatin1String(" / ") + c.key;
        all.insert(t, c.key);
    }
    // the mind map window's own keys (its commands are not in the list): the German keys must not meet them
    const QStringList mindMapKeys = {QStringLiteral("Alt+Shift+C"), QStringLiteral("Alt+Shift+V"), QStringLiteral("Alt+Shift+I"),
                                     QStringLiteral("Alt+Shift+Left"), QStringLiteral("Alt+Shift+Right"), QStringLiteral("Alt+Shift+Up"),
                                     QStringLiteral("Alt+Shift+Down"), QStringLiteral("Alt+A"), QStringLiteral("Ctrl+Shift+L"),
                                     QStringLiteral("Ctrl+Shift+K"), QStringLiteral("Ctrl+Shift+R"), QStringLiteral("Ctrl+Shift+C"),
                                     QStringLiteral("Ctrl+J"), QStringLiteral("Ctrl+D")};
    for (auto it = german.begin(); it != german.end(); ++it)
        if (it.key().startsWith(QLatin1String("mm_")) && mindMapKeys.contains(it.value()))
            clash << QStringLiteral("mind map: ") + it.value();
    QHash<QString, QString> mm;
    for (auto it = german.begin(); it != german.end(); ++it)
        if (it.key().startsWith(QLatin1String("mm_"))) {
            if (mm.contains(it.value()))
                clash << QStringLiteral("mind map twice: ") + it.value();
            mm.insert(it.value(), it.key());
        }
    check(clash.isEmpty(), QStringLiteral("the German set clashes with no other shortcut (%1 shortcuts compared)").arg(all.size()),
          clash.join(QStringLiteral("; ")));
    check(kl::standardFor(QStringLiteral("alignLeft"), QKeySequence(Qt::CTRL | Qt::Key_BraceLeft), QStringLiteral("us"))
                  == QKeySequence(Qt::CTRL | Qt::Key_BraceLeft)
              && kl::standardFor(QStringLiteral("alignLeft"), QKeySequence(Qt::CTRL | Qt::Key_BraceLeft), QStringLiteral("de"))
                     == QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_L)
              && kl::standardFor(QStringLiteral("bold"), QKeySequence::Bold, QStringLiteral("de")) == QKeySequence(QKeySequence::Bold),
          QStringLiteral("a command of the German set gets its own key there; the others keep theirs"));
    (void)w;

    // ---- whole starts in a program of their own, each with its own settings
    QTemporaryDir root;
    auto fresh = [&](const QString &name) {
        const QString h = root.path() + QLatin1Char('/') + name;
        QDir().mkpath(h);
        return h;
    };
    const QStringList de = runChild(fresh(QStringLiteral("de")), QStringLiteral("de"), {QStringLiteral("shot"), outDir + QLatin1String("/menu-format-german-layout.png")});
    check(val(de, QStringLiteral("set")) == QLatin1String("de") && val(de, QStringLiteral("alignLeft")) == QLatin1String("Alt+Shift+L")
              && val(de, QStringLiteral("alignCenter")) == QLatin1String("Alt+Shift+E") && val(de, QStringLiteral("alignRight")) == QLatin1String("Alt+Shift+R")
              && val(de, QStringLiteral("back")) == QLatin1String("Alt+Shift+B") && val(de, QStringLiteral("forward")) == QLatin1String("Alt+Shift+F")
              && val(de, QStringLiteral("scriptElements")) == QLatin1String("Ctrl+Shift+O") && val(de, QStringLiteral("subscript")) == QLatin1String("Ctrl+Shift+D")
              && val(de, QStringLiteral("bold")) == QLatin1String("Ctrl+B") && val(de, QStringLiteral("options")) == QLatin1String("Ctrl+,"),
          QStringLiteral("first start with a German layout: the German set"), de.join(QStringLiteral(" ")));
    bool noAltGrLabels = true;
    for (const char *k : {"alignLeft", "alignCenter", "alignRight", "back", "forward", "scriptElements", "subscript"})
        noAltGrLabels = noAltGrLabels && !val(de, QStringLiteral("label.") + QLatin1String(k)).contains(QRegularExpression(QStringLiteral("[{}|\\[\\]\\\\=]")));
    check(noAltGrLabels, QStringLiteral("... the menus show no key of the AltGr layer"));
    const QStringList us = runChild(fresh(QStringLiteral("us")), QStringLiteral("us"));
    check(val(us, QStringLiteral("set")) == QLatin1String("us") && val(us, QStringLiteral("alignLeft")) == QLatin1String("Ctrl+{")
              && val(us, QStringLiteral("back")) == QLatin1String("Ctrl+[") && val(us, QStringLiteral("subscript")) == QLatin1String("Ctrl+="),
          QStringLiteral("first start with a US layout: the usual set"), us.join(QStringLiteral(" ")));
    for (const char *other : {"none", "ru", "id"}) {
        const QStringList o = runChild(fresh(QLatin1String("o-") + QLatin1String(other)), QLatin1String(other));
        check(val(o, QStringLiteral("set")) == QLatin1String("us") && val(o, QStringLiteral("alignRight")) == QLatin1String("Ctrl+}"),
              QStringLiteral("first start with layout \"%1\": the usual set").arg(QLatin1String(other)), o.join(QStringLiteral(" ")));
    }
    // kept: the second start asks no more (the layout changed, the set did not)
    const QString keptHome = fresh(QStringLiteral("kept"));
    runChild(keptHome, QStringLiteral("de"));
    const QStringList again = runChild(keptHome, QStringLiteral("us"));
    check(val(again, QStringLiteral("set")) == QLatin1String("de") && val(again, QStringLiteral("alignLeft")) == QLatin1String("Alt+Shift+L"),
          QStringLiteral("the second start keeps the set of the first"), again.join(QStringLiteral(" ")));
    const QString keptUs = fresh(QStringLiteral("kept-us"));
    runChild(keptUs, QStringLiteral("us"));
    const QStringList againDe = runChild(keptUs, QStringLiteral("de"));
    check(val(againDe, QStringLiteral("set")) == QLatin1String("us") && val(againDe, QStringLiteral("alignLeft")) == QLatin1String("Ctrl+{"),
          QStringLiteral("... also from US to a German keyboard later"), againDe.join(QStringLiteral(" ")));
    // a program that was used before (the welcome project was shown) keeps the usual set
    const QStringList old = runChild(fresh(QStringLiteral("old")), QStringLiteral("de"), {QStringLiteral("welcome")});
    check(val(old, QStringLiteral("set")) == QLatin1String("us") && val(old, QStringLiteral("alignLeft")) == QLatin1String("Ctrl+{"),
          QStringLiteral("a program that was used before keeps its shortcuts (no first start)"), old.join(QStringLiteral(" ")));
    // shortcut settings exist: no first start; the writer's key stays
    const QStringList custom = runChild(fresh(QStringLiteral("custom")), QStringLiteral("de"), {QStringLiteral("custom"), QStringLiteral("alignLeft=Ctrl+Shift+Q")});
    check(val(custom, QStringLiteral("set")) == QLatin1String("us") && val(custom, QStringLiteral("alignLeft")) == QLatin1String("Ctrl+Shift+Q")
              && val(custom, QStringLiteral("alignRight")) == QLatin1String("Ctrl+}"),
          QStringLiteral("with shortcut settings there is no first start; the stored shortcut stays"), custom.join(QStringLiteral(" ")));
    // a change of the writer in a German-set profile stays, the rest of the set stays too
    const QString germanKept = fresh(QStringLiteral("german-kept"));
    runChild(germanKept, QStringLiteral("de"));
    const QStringList chg = runChild(germanKept, QStringLiteral("de"), {QStringLiteral("custom"), QStringLiteral("alignLeft=Ctrl+Shift+Q")});
    check(val(chg, QStringLiteral("set")) == QLatin1String("de") && val(chg, QStringLiteral("alignLeft")) == QLatin1String("Ctrl+Shift+Q")
              && val(chg, QStringLiteral("alignRight")) == QLatin1String("Alt+Shift+R"),
          QStringLiteral("in a profile with the German set a stored shortcut is never replaced"), chg.join(QStringLiteral(" ")));
    // the mind map's keys follow the same set
    check(kl::standardFor(QStringLiteral("mm_front"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BraceRight), QStringLiteral("de"))
              == QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_PageUp),
          QStringLiteral("the mind map's alignment and order keys have a German set too"));
}

int main(int argc, char **argv)
{
    const bool child = argc > 1 && QByteArray(argv[1]) == "--child";
    QTemporaryDir home;
    const QString homeDir = home.path() + QLatin1String("/h");
    QDir().mkpath(homeDir);
    if (!child) {
        qputenv("HOME", homeDir.toUtf8());
        qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
        qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
        qputenv("QB_KEYBOARD_LAYOUT", "us");
    }
#ifdef Q_OS_WIN
    // Windows: Qt keeps the settings file in the user's roaming application data, whatever XDG_CONFIG_HOME says; here it
    // follows XDG_CONFIG_HOME as on Linux, so that every child run has settings of its own
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qEnvironmentVariable("XDG_CONFIG_HOME"));
#endif
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0940cTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0940cTest"));
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    QIcon::setFallbackThemeName(QStringLiteral("qbicons"));
    QApplication::setFont(QFont(QStringLiteral("Noto Sans"), 10));
    outDir = child ? QString::fromLocal8Bit(argv[2]) : (argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots"));
    QDir().mkpath(outDir);
    AppSettings &as = AppSettings::instance();
    as.load();
    QStringList extra;
    for (int i = 3; i < argc; ++i)
        extra << QString::fromLocal8Bit(argv[i]);
    if (child && extra.contains(QStringLiteral("welcome")))
        as.welcomeShown = true;
    as.theme = QStringLiteral("light");
    as.spellCheck = false;
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.save();
    if (child && extra.contains(QStringLiteral("custom"))) {
        const QString spec = extra.at(int(extra.indexOf(QStringLiteral("custom"))) + 1);
        QJsonObject o;
        o.insert(spec.section(QLatin1Char('='), 0, 0), spec.section(QLatin1Char('='), 1));
        QSettings().setValue(QStringLiteral("keyboard/shortcuts"), QJsonDocument(o).toJson(QJsonDocument::Compact));
    }
    theme::apply();

    if (child) {
        MainWindow cw;
        cw.show();
        cw.activateWindow();
        settle(300);
        return childMain(cw, extra);
    }

    QTemporaryDir work;
    templates::Info novel;
    for (const auto &t : templates::all())
        if (t.id == QLatin1String("novel"))
            novel = t;
    QString err;
    auto proj = templates::createProject(novel, work.path(), QStringLiteral("Lamp"), &err);
    check(proj != nullptr, QStringLiteral("(project made)"), err);
    if (!proj)
        return 1;
    Project *p = proj.get();
    p->save(&err);
    auto *wp = new MainWindow;
    MainWindow &w = *wp;
    w.show();
    w.activateWindow();
    (void)QTest::qWaitForWindowActive(&w, 2000);
    check(w.adoptProject(std::move(proj)), QStringLiteral("(project open)"));
    settle(400);

    names(w);
    audit(w);
    binderKeys(w, p);
    layoutSets(w);

    std::printf("%d failure(s)\n", failures);
    return failures;
}
