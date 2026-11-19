#include "android/Mobile.h"

#include <QApplication>
#include <QBackingStore>
#include <QElapsedTimer>
#include <QEvent>
#include <QLoggingCategory>
#include <QOpenGLContext>
#include <QPlatformSurfaceEvent>
#include <QSemaphore>
#include <QThread>
#include <QVariant>
#include <QWidget>
#include <QWindow>
#include <QtCore/qcoreapplication_platform.h>
#include <QtGui/private/qguiapplication_p.h>
#include <memory>
#include <qpa/qplatformbackingstore.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformintegration.h>
#include <rhi/qrhi.h>
#include <rhi/qrhi_platform.h>

// Android only, and only for Qt 6.12.0 (take this file out with the Qt version that has the fault repaired; Qt's
// branch 6.12 has: QAndroidPlatformOpenGLWindow::eglSurface() without the "deadlock protector").
//
// The fault: Qt 6.12.0 ends the program ("Failed to acquire deadlock protector for
// QAndroidPlatformOpenGLWindow::eglSurface()", locked by "QAndroidInputContext::runOnQtThread()") when a window is
// drawn for the first time while Android's keyboard program is in a call into the program. A window's Android surface
// is made at its first drawing, in eglSurface(), which takes Qt's one lock ("protector") for that and ends the program
// when the lock is taken. Every call of the keyboard program takes the same lock on Android's own thread and then
// waits for the program's thread, which is busy drawing the new window: so a menu, a dialog or any other new window
// opened while the on-screen keyboard is at work (the cursor in a text) ended the program, on the emulated tablet
// and on the writer's tablets (runs 6 to 14 of the workflow).
//
// What this file does: it makes the surface itself, right before the window is drawn for the first time, at a moment
// when no such call can be on its way:
//   1. A drawing context of the window's own format is at hand (made once per format and kept; Qt chooses the kind
//      of surface by the format, so the surface gets the kind Qt would give it). A window that was drawn before
//      (the app comes back from out of sight) has a drawing engine with its own context: that one is taken.
//   2. Android's own thread is asked to stand still (a piece of code run there that waits). Calls of the keyboard
//      program come on that thread, one after the other: those that already wait for the program's thread are
//      answered here until the thread stands still; new ones cannot start.
//   3. The program's thread says "go" and at once makes the context current on the window: eglSurface() takes the
//      free lock and asks Android for the surface. Android's thread stands still until Qt holds the lock: Qt itself
//      gives the word (a debug line it writes right after the asking, caught by windowWord() below). Then Android's
//      thread makes the surface, and calls of the keyboard program that come meanwhile find the lock taken and
//      return at once, as Qt means them to. (Builds 18 to 22 let the thread stand still for 12 ms instead and
//      trusted the program's thread to be faster: run 23 ended the app twice at a menu, when it was not.)
// After that the window has its surface and eglSurface() never needs the lock for it again (until Android takes the
// surfaces away because the app went out of sight; then the same is done when it comes back).
// Nothing here holds Qt's lock: when a step does not work out (Android's thread does not come to a stand in time),
// the window is drawn as Qt would have drawn it without this file.
//
// Why not through the window's drawing engine from the start: Qt makes that engine at the window's first picture
// (QRhiBackingStore::flush) and the engine's making itself (QRhiGles2::create) makes its context current on the
// window, which is the call of eglSurface() that fails; it also takes milliseconds (a context is made), too long
// to keep Android's thread from running into it. So the surface is there before the engine is made.
//
// Two earlier guards (runs 9 and 10) held the lock from the window's show event on and were taken out: they could
// not give it up late enough.

namespace qb {
namespace mobile {

namespace {

Q_LOGGING_CATEGORY(lcWindows, "qb.windows", QtInfoMsg)

const char kMade[] = "qbSurfaceMade"; // (a window's property: its Android surface was made here)
constexpr int kStandMs = 1000;        // how long the program waits for Android's thread to stand still (seen: up to 139 ms)
constexpr int kGoMs = 2000;           // how long Android's thread waits for the program's "go" at most
constexpr int kEnterMs = 250;         // how long Android's thread stands still after the "go" when Qt gives no word

struct Gate
{
    QSemaphore standing; // Android's thread stands still
    QSemaphore go;       // the program's thread is about to make the surface
    QAtomicInt left;     // the program's thread did not wait any longer: do not stand still at all
    QSemaphore entered; // Qt holds its lock and has asked Android for the surface (or the making is over)
};

// The word Qt gives at the right moment. QAndroidPlatformOpenGLWindow::eglSurface() takes Qt's lock, asks Android
// for the surface and then waits for Android's own thread; between the asking and the waiting it writes one debug
// line of the category "qt.qpa.window". That line, caught here on its way to the log, is what lets Android's thread
// go on: not earlier (a call of the keyboard program could take the lock first: run 23 of the workflow, when the
// thread simply stood still for 12 ms and the program's thread was slower than that twice), and not later than
// needed. The category's debug lines are switched on for this alone, and none of them goes into the log.
QtMessageHandler s_handlerBefore = nullptr;
QAtomicPointer<QSemaphore> s_entered;
QAtomicInt s_wordsHeard;

void windowWord(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (type == QtDebugMsg && context.category && qstrcmp(context.category, "qt.qpa.window") == 0) {
        if (message.startsWith(QLatin1String("called createSurface()"))) {
            s_wordsHeard.ref();
            if (QSemaphore *entered = s_entered.loadAcquire())
                entered->release();
        }
        return;
    }
    if (s_handlerBefore)
        s_handlerBefore(type, context, message);
}


// A context to make the window's surface with. Qt gives a surface the kind (EGL configuration) of the context that
// first asks for it, and chooses a context's kind by its format alone: a context of the window's format gives the
// surface the kind the window's own drawing engine would have given it, and the engine's context, made later with
// the format the window then reports, fits it. One context per format is kept for all windows.
QOpenGLContext *contextOf(QWindow *win)
{
    // a window that was drawn before has its engine: that context
    QWindow *top = win;
    while (QWindow *parent = top->parent())
        top = parent;
    const QWidgetList tops = QApplication::topLevelWidgets();
    for (QWidget *w : tops) {
        if (w->windowHandle() != top)
            continue;
        QBackingStore *store = w->backingStore();
        QPlatformBackingStore *platformStore = store ? store->handle() : nullptr;
        QRhi *rhi = platformStore ? platformStore->rhi(win) : nullptr;
        if (rhi && rhi->backend() == QRhi::OpenGLES2) {
            if (const auto *handles = static_cast<const QRhiGles2NativeHandles *>(rhi->nativeHandles()); handles && handles->context)
                return handles->context;
        }
        break;
    }
    struct Kept
    {
        QSurfaceFormat format;
        QOpenGLContext *context;
    };
    static QList<Kept> kept;
    const QSurfaceFormat format = win->format(); // (what the engine would be made with: QPlatformBackingStore::createRhi)
    for (const Kept &k : std::as_const(kept))
        if (k.format == format)
            return k.context;
    auto *context = new QOpenGLContext(qApp);
    context->setFormat(format);
    if (!context->create()) {
        delete context;
        return nullptr;
    }
    kept.append(Kept{format, context});
    return context;
}

class WindowGuard : public QObject
{
public:
    explicit WindowGuard(QObject *parent)
        : QObject(parent)
    {
        // Out of sight, Android takes every window's surface away (QAndroidPlatformOpenGLWindow::
        // applicationStateChanged): they are made again, the same way, when the app comes back.
        connect(qApp, &QGuiApplication::applicationStateChanged, this, [](Qt::ApplicationState state) {
            if (state > Qt::ApplicationHidden)
                return;
            const QWindowList all = QGuiApplication::allWindows();
            for (QWindow *w : all)
                w->setProperty(kMade, QVariant());
        });
    }

protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::Expose:
        case QEvent::Paint:
            // (what comes before a window's first picture: Qt draws a window only once it is exposed)
            if (o->isWindowType() && !m_busy) {
                auto *win = static_cast<QWindow *>(o);
                if (win->isExposed() && win->handle() && win->type() != Qt::ForeignWindow
                    && win->surfaceType() == QSurface::OpenGLSurface && !win->property(kMade).toBool()) {
                    m_busy = true;
                    makeSurface(win);
                    m_busy = false;
                }
            }
            break;
        case QEvent::PlatformSurface:
            if (o->isWindowType()
                && static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
                o->setProperty(kMade, QVariant());
            break;
        default:
            break;
        }
        return false;
    }

private:
    void makeSurface(QWindow *win)
    {
        QElapsedTimer clock;
        clock.start();
        win->setProperty(kMade, true); // (first: what follows may bring this window another event)
        QOpenGLContext *context = contextOf(win);
        if (!context) {
            qCInfo(lcWindows, "windows: no context for %s; drawn as Qt draws it", qPrintable(win->objectName()));
            return;
        }
        QPlatformInputContext *input = nullptr;
        if (QPlatformIntegration *integration = QGuiApplicationPrivate::platformIntegration())
            input = integration->inputContext();

        // Android's own thread stands still ...
        auto gate = std::make_shared<Gate>();
        (void)QNativeInterface::QAndroidApplication::runOnAndroidMainThread([gate]() -> QVariant {
            if (gate->left.loadAcquire())
                return QVariant();
            gate->standing.release();
            // it goes on when Qt holds its lock (windowWord() above), or when the making is over
            if (gate->go.tryAcquire(1, kGoMs))
                (void)gate->entered.tryAcquire(1, kEnterMs);
            return QVariant();
        });
        // ... once the calls of the keyboard program that wait for this thread are answered
        bool standing = false;
        QElapsedTimer waited;
        waited.start();
        while (!standing && waited.elapsed() < kStandMs) {
            if (input)
                QCoreApplication::sendPostedEvents(input, QEvent::MetaCall);
            standing = gate->standing.tryAcquire(1, 1);
        }
        const qint64 stoodAfter = waited.elapsed();
        if (!standing)
            gate->left.storeRelease(1);

        QOpenGLContext *before = QOpenGLContext::currentContext();
        QSurface *beforeSurface = before ? before->surface() : nullptr;
        const int heardBefore = s_wordsHeard.loadAcquire();
        s_entered.storeRelease(&gate->entered);
        gate->go.release();
        const bool made = context->makeCurrent(win); // QAndroidPlatformOpenGLWindow::eglSurface() makes the surface
        s_entered.storeRelease(nullptr);
        gate->entered.release(); // (no surface had to be made, or Qt gave no word: Android's thread need not wait on)
        const bool heard = s_wordsHeard.loadAcquire() != heardBefore;
        if (before && beforeSurface)
            before->makeCurrent(beforeSurface);
        else
            context->doneCurrent();
        qCInfo(lcWindows, "windows: surface of %s made ahead of its first picture (%s; Android's thread %s after %lld ms, went on %s; %lld ms in all)",
               qPrintable(win->objectName()), made ? "current" : "not current yet",
               standing ? "stood still" : "DID NOT STAND STILL", static_cast<long long>(stoodAfter),
               heard ? "at Qt's word" : "WITHOUT QT'S WORD", static_cast<long long>(clock.elapsed()));
    }

    bool m_busy = false;
};

} // namespace

void installWindowGuard()
{
    if (!qApp)
        return;
    const QString name = QStringLiteral("mobileWindowGuard");
    if (qApp->findChild<QObject *>(name, Qt::FindDirectChildrenOnly))
        return;
    auto *guard = new WindowGuard(qApp);
    guard->setObjectName(name);
    qApp->installEventFilter(guard);
    // Qt's word (windowWord() above): the debug lines of Qt's window category, caught on their way to the log.
    // Installed once and never again: a handler installed later hands on to this one, and a second installing
    // of this one would make the two hand each line to each other without end.
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qpa.window.debug=true"));
    s_handlerBefore = qInstallMessageHandler(windowWord);
}

} // namespace mobile
} // namespace qb
