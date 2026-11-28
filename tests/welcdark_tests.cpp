// 0.99.0: the welcome project's contrast, its mind map above all. Makes the welcome project (English and the ten
// translations from resources/welcome), takes every note, shape and line of its mind maps and measures the text
// against the fill (or the shape under it, or the board), the border and the lines against what is behind them,
// on the light and on the dark board that every built-in theme leads to (a board follows the theme only by being
// light or dark). Text needs 4.5 : 1, borders and lines 3 : 1. Saves a picture of the board on each.
//
//   xvfb-run -a ./qb_welcdark_tests /tmp/shots
#include "core/BinderModel.h"
#include "core/MindMap.h"
#include "core/OutlineProject.h"
#include "core/Project.h"
#include "ui/MindMapDoc.h"
#include "ui/MindMapScene.h"
#include "ui/Theme.h"

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>

#include <cstdio>

using namespace qb;
using namespace qb::mindmap;

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

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderWelcDarkTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderWelcDarkTest"));
    QApplication app(argc, argv);
    const QString outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    const QString src = QStringLiteral(QB_TEST_SOURCE_DIR);

    const QStringList codes = {QStringLiteral("en"), QStringLiteral("de"), QStringLiteral("es"), QStringLiteral("fr"),
                               QStringLiteral("id"), QStringLiteral("it"), QStringLiteral("nl"), QStringLiteral("pl"),
                               QStringLiteral("pt"), QStringLiteral("ru"), QStringLiteral("tr")};
    const QColor light(253, 251, 240), dark(46, 46, 43);   // the boards of light and dark themes (MindMapPrefs)
    QTemporaryDir work;
    for (const QString &code : codes) {
        const QString root = code == QLatin1String("en") ? src + QStringLiteral("/resources/welcome")
                                                          : src + QStringLiteral("/resources/welcome/i18n/") + code;
        QString err;
        const QJsonObject o = outline::read(root + QStringLiteral("/outline.json"), &err);
        // the English documents are the base for the translations that have no file of their own
        outline::Result r = outline::build(o, root, work.path(), QStringLiteral("Welcome-") + code);
        check(r.project != nullptr, QStringLiteral("%1: the welcome project is made").arg(code), r.errors.join(QLatin1Char(';')) + err);
        if (!r.project)
            continue;
        Project *p = r.project.get();
        int boards = 0, bad = 0;
        QString first;
        for (BinderItem *it : p->allItems()) {
            MindMapDoc *doc = mindmaps::docFor(p, it->id);
            if (!doc || doc->board.notes.isEmpty())
                continue;
            ++boards;
            for (int d = 0; d < 2; ++d) {
                const QColor bg = d ? dark : light;
                Settings st = doc->board.settings;
                st.background = bg;
                const QColor ink = st.effectiveTextColor();
                auto fail = [&](const QString &s) {
                    ++bad;
                    if (first.isEmpty())
                        first = s;
                };
                for (const Note &n : doc->board.notes) {
                    if (n.isImage())
                        continue;
                    QColor behind = n.look.fill.isValid() ? n.look.fill : bg;
                    if (!n.look.fill.isValid())
                        for (const Shape &s : doc->board.shapes)
                            if (s.look.fill.isValid() && s.rect.contains(n.pos))
                                behind = s.look.fill;
                    const QColor text = n.look.textColor.isValid() ? n.look.textColor : ink;
                    const double rt = theme::contrastRatio(text, behind);
                    if (rt < 4.5)
                        fail(QStringLiteral("note %1 text %2 on %3 = %4 (%5 board)").arg(n.id).arg(text.name(), behind.name()).arg(rt, 0, 'f', 2).arg(d ? "dark" : "light"));
                    if (n.look.borderWeight > 0) {
                        const QColor bc = n.look.borderColor.isValid() ? n.look.borderColor : ink;
                        const double rb = theme::contrastRatio(bc, behind);
                        if (rb < 3.0)
                            fail(QStringLiteral("note %1 border %2 on %3 = %4 (%5 board)").arg(n.id).arg(bc.name(), behind.name()).arg(rb, 0, 'f', 2).arg(d ? "dark" : "light"));
                    }
                    for (int other : n.connected) {
                        const QColor lc = doc->board.lineLook(n.id, other).color;
                        const QColor col = lc.isValid() ? lc : ink;
                        const double rl = theme::contrastRatio(col, bg);
                        if (rl < 3.0)
                            fail(QStringLiteral("line %1-%2 %3 on %4 = %5").arg(n.id).arg(other).arg(col.name(), bg.name()).arg(rl, 0, 'f', 2));
                    }
                }
            }
            // a picture of the board on each background
            for (int d = 0; d < 2; ++d) {
                doc->board.settings.background = d ? dark : light;
                MindMapScene *sc = doc->scene();
                if (!sc)
                    continue;
                sc->syncAll();
                QImage img(760, 560, QImage::Format_ARGB32);
                QPainter pt(&img);
                sc->render(&pt, QRectF(0, 0, 760, 560), QRectF(0, 0, 760, 560), true, false);
                pt.end();
                if (code == QLatin1String("en"))
                    img.save(outDir + QStringLiteral("/welcdark-%1-%2.png").arg(d ? "dark" : "light").arg(boards));
            }
        }
        check(boards >= 1, QStringLiteral("%1: the welcome project has a mind map").arg(code));
        check(bad == 0, QStringLiteral("%1: every note, border and line of its mind maps reaches the ratio on the light and dark board").arg(code), first);
    }
    std::printf("\n%d failure(s)\n", failures);
    return failures;
}
