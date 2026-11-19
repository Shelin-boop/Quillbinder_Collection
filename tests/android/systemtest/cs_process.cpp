// The system test for Android: what is left of Linux's stage "the whole program" (tests/crashsafe/cs_process.cpp):
// Linux starts the real program as a second process on its virtual display (X-01 to X-12, S-Y04); an app on a
// tablet cannot start another copy of itself, so those cases are not in Android's copy (2026-11-16). The helpers
// below are used by other cases; Android's own cases of an app killed or put away are in cs_android.cpp.
#include "cs.h"

#include "ui/MainWindow.h"

#include <QHash>

namespace cs {

// the longest text that is in "after" and not in "before" (per file), and where
QString added(const QHash<QString, QString> &before, const QHash<QString, QString> &after, QString *where)
{
    QString best;
    for (auto it = after.begin(); it != after.end(); ++it) {
        const QString a = before.value(it.key());
        const QString b = it.value();
        if (a == b)
            continue;
        int s = 0;
        while (s < a.size() && s < b.size() && a[s] == b[s])
            ++s;
        int e = 0;
        while (e < a.size() - s && e < b.size() - s && a[a.size() - 1 - e] == b[b.size() - 1 - e])
            ++e;
        const QString ins = b.mid(s, b.size() - s - e);
        if (ins.size() > best.size()) {
            best = ins;
            if (where)
                *where = it.key();
        }
    }
    return best;
}

// a copy of the fixture with the scene shown, the window closed again
QString prepare(const QString &fixture, const QString &scene)
{
    const QString path = fixtureCopy(fixture);
    MainWindow *w = openWindow(path);
    if (w->project())
        showDoc(w, itemByTitle(w->project(), scene));
    settle(400);
    w->closeProject();
    dropWindow(w);
    settle(300);
    return path;
}

} // namespace cs
