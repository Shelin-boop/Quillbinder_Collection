// Scripted run for version 0.93.0 beta, second suite: fixes in code that several systems share (to-do items 15, 16, 26).
//  - item 16: four limits for prepared input: an add-on package of more than 400 MB is refused by install too; the
//    Web Page Import fetches pictures of a page from the internet only from public addresses (and says how many it left
//    out); an archive whose sizes cannot be added up is refused; a request on the link of the writing tools is one line
//    of at most 64 KiB;
//  - item 26: the main window cuts its connections to the application's focus signal (and other application-wide
//    signals) first when it is deleted, as the focus changes while its own widgets go; a window deleted with the
//    keyboard in a text field and another window to take the focus does not crash;
//  - item 15: exporting options and themes never destroys the file that was there (a write that fails leaves the old
//    file byte for byte, and says so);
// Saves screenshots into the folder given as first argument. Exit code = number of failed checks.
//
//   xvfb-run -a ./qb_v0930b_tests /tmp/shots
#include "core/Addons.h"
#include "core/Backup.h"
#include "core/Paths.h"
#include "ui/ToolHost.h"
#include "core/Zip.h"
#include "WebImporter.h"
#include "import/ImporterApi.h"
#include "core/Project.h"
#include "ui/AppSettings.h"
#include "ui/MainWindow.h"
#include "ui/OptionsDialog.h"
#include "ui/Theme.h"

#include "AppInfo.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>
#include <QHostAddress>
#include <QImage>
#include <QBuffer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSettings>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>

#include <cstdio>

using namespace qb;

// an application that can tell how many receivers the focus signal has (a protected function of QObject)
class TestApp : public QApplication
{
public:
    using QApplication::QApplication;
    int focusReceivers() { return receivers(SIGNAL(focusChanged(QWidget *, QWidget *))); }
};
static TestApp *theApp = nullptr;

// a child of the window that is deleted with it, after the window's own destructor: notes who still listens then
class Probe : public QWidget
{
public:
    Probe(QWidget *parent, int *listeners) : QWidget(parent), m_listeners(listeners) {}
    ~Probe() override { *m_listeners = theApp->focusReceivers(); }

private:
    int *m_listeners;
};

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

static QByteArray readAll(const QString &f)
{
    QFile file(f);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

static void writeAll(const QString &f, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(f).absolutePath());
    QFile file(f);
    if (file.open(QIODevice::WriteOnly))
        file.write(data);
}

// ------------------------------------------------------------------ item 15: exports that keep the old file

static void exports(const QString &W)
{
    MainWindow w;
    OptionsDialog d(&w, 0);
    const QString opt = W + QStringLiteral("/export/my.qboptions");
    const QString thm = W + QStringLiteral("/export/my.qbtheme");
    QDir().mkpath(W + QStringLiteral("/export"));
    QString e;
    check(d.saveOptionsToFile(opt, &e) && QFile::exists(opt), QStringLiteral("an options file is written"), e);
    theme::Theme look = theme::Theme();
    check(OptionsDialog::saveThemeToFile(thm, look, &e) && QFile::exists(thm), QStringLiteral("a theme file is written"), e);
    // an older export holds an entry that the new one must not inherit
    {
        QSettings s(opt, QSettings::IniFormat);
        s.setValue(QStringLiteral("zzold/entry"), 7);
        s.sync();
    }
    d.saveOptionsToFile(opt, &e);
    check(!QSettings(opt, QSettings::IniFormat).contains(QStringLiteral("zzold/entry")),
          QStringLiteral("a second export is the new options only (no entry of the old file is mixed in)"));
    check(QDir(W + QStringLiteral("/export")).entryList(QDir::Files).size() == 2, QStringLiteral("no temporary file stays beside the exports"),
          QDir(W + QStringLiteral("/export")).entryList(QDir::Files).join(QLatin1Char(',')));

    const QByteArray oldOpt = readAll(opt), oldThm = readAll(thm);
    qputenv("QB_TEST_WRITE_FAILS", "5|Input/output error|/export/my.");
    QString eo, et;
    const bool okO = d.saveOptionsToFile(opt, &eo);
    const bool okT = OptionsDialog::saveThemeToFile(thm, look, &et);
    qunsetenv("QB_TEST_WRITE_FAILS");
    check(!okO && readAll(opt) == oldOpt && !oldOpt.isEmpty(), QStringLiteral("options: a write that fails leaves the old file byte for byte"));
    check(!okT && readAll(thm) == oldThm && !oldThm.isEmpty(), QStringLiteral("theme: a write that fails leaves the old file byte for byte"));
    check(eo.contains(QLatin1String("unchanged")) && et.contains(QLatin1String("unchanged")) && et.contains(QLatin1String("Input/output error")),
          QStringLiteral("both messages come, name the reason and say the old file is unchanged"), eo + QLatin1Char('|') + et);
    check(QDir(W + QStringLiteral("/export")).entryList(QDir::Files).size() == 2, QStringLiteral("... and no temporary file is left"),
          QDir(W + QStringLiteral("/export")).entryList(QDir::Files).join(QLatin1Char(',')));
}

// ------------------------------------------------------------------ item 16

// a small web server: path -> status, type, body, Location
class WebServer : public QTcpServer
{
public:
    struct Resp {
        int status = 200;
        QByteArray type, body, location;
    };
    QHash<QString, Resp> pages;
    QStringList requests;

protected:
    void incomingConnection(qintptr fd) override
    {
        auto *s = new QTcpSocket(this);
        s->setSocketDescriptor(fd);
        auto *buf = new QByteArray;
        connect(s, &QTcpSocket::readyRead, this, [this, s, buf] {
            *buf += s->readAll();
            if (!buf->contains("\r\n\r\n"))
                return;
            const QList<QByteArray> line = buf->left(buf->indexOf("\r\n")).split(' ');
            const QString path = line.size() > 1 ? QString::fromLatin1(line.at(1)) : QString();
            requests << path;
            const Resp r = pages.value(path, Resp{404, "text/html", "not found", {}});
            QByteArray out = "HTTP/1.1 " + QByteArray::number(r.status) + " X\r\nContent-Type: " + r.type
                             + "\r\nContent-Length: " + QByteArray::number(r.body.size()) + "\r\nConnection: close\r\n";
            if (!r.location.isEmpty())
                out += "Location: " + r.location + "\r\n";
            s->write(out + "\r\n" + r.body);
            s->disconnectFromHost();
            buf->clear();
        });
        connect(s, &QTcpSocket::disconnected, s, [s, buf] {
            delete buf;
            s->deleteLater();
        });
    }
};

static QByteArray pngBytes(int w)
{
    QImage img(w, w, QImage::Format_RGB32);
    img.fill(Qt::darkGreen);
    QByteArray b;
    QBuffer buf(&b);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return b;
}

static void webPictures(const QString &W)
{
    // the classification of addresses
    struct Row {
        const char *addr;
        bool nonPublic;
    };
    const Row rows[] = {{"127.0.0.1", true},      {"127.8.9.10", true},      {"10.1.2.3", true},       {"172.16.0.1", true},
                        {"172.31.255.255", true}, {"172.32.0.1", false},     {"192.168.1.1", true},    {"169.254.1.1", true},
                        {"0.0.0.0", true},        {"100.64.0.1", true},      {"93.184.216.34", false}, {"8.8.8.8", false},
                        {"::1", true},            {"::", true},              {"fe80::1", true},        {"fd12:3456::1", true},
                        {"::ffff:127.0.0.1", true}, {"::ffff:10.0.0.1", true}, {"::ffff:8.8.8.8", false}, {"64:ff9b::7f00:1", true},
                        {"2606:4700:4700::1111", false}, {"2001:db8::1", false}};
    QStringList wrong;
    for (const Row &r : rows)
        if (qbweb::isNonPublicAddress(QHostAddress(QString::fromLatin1(r.addr))) != r.nonPublic)
            wrong << QString::fromLatin1(r.addr);
    check(wrong.isEmpty(), QStringLiteral("loopback, link-local, private and unspecified addresses (IPv4, IPv6, IPv4 inside IPv6) are not public; others are"),
          wrong.join(QLatin1Char(' ')));

    WebServer pub, priv;
    check(pub.listen(QHostAddress::LocalHost) && priv.listen(QHostAddress::LocalHost), QStringLiteral("two test web servers listen"));
    const QString A = QStringLiteral("http://127.0.0.1:%1").arg(pub.serverPort());
    const QString B = QStringLiteral("http://127.0.0.1:%1").arg(priv.serverPort());
    const QString Bname = QStringLiteral("http://localhost:%1").arg(priv.serverPort());
    const QByteArray png = pngBytes(40);
    pub.pages.insert(QStringLiteral("/ok.png"), {200, "image/png", png, {}});
    pub.pages.insert(QStringLiteral("/redir-ok.png"), {302, "text/plain", "", "/ok.png"});
    pub.pages.insert(QStringLiteral("/redir-private.png"), {302, "text/plain", "", (B + QStringLiteral("/private3.png")).toLatin1()});
    priv.pages.insert(QStringLiteral("/private1.png"), {200, "image/png", png, {}});
    priv.pages.insert(QStringLiteral("/private2.png"), {200, "image/png", png, {}});
    priv.pages.insert(QStringLiteral("/private3.png"), {200, "image/png", png, {}});
    QByteArray html = "<html><head><title>Test page</title></head><body><article><h1>Test page</h1>";
    const QStringList srcs = {A + QStringLiteral("/ok.png"), A + QStringLiteral("/redir-ok.png"), B + QStringLiteral("/private1.png"),
                              Bname + QStringLiteral("/private2.png"), A + QStringLiteral("/redir-private.png")};
    for (const QString &src : srcs)
        html += "<p>" + QByteArray("The keeper climbed the tower, lit the lamp, and watched the sea until the morning boat came in. "
                                   "Nobody else had seen the storm, and the log said nothing of it.")
                + "</p><img src=\"" + src.toUtf8() + "\" width=\"300\" height=\"200\" alt=\"x\">";
    html += "</article></body></html>";
    pub.pages.insert(QStringLiteral("/page.html"), {200, "text/html; charset=utf-8", html, {}});
    priv.pages.insert(QStringLiteral("/page.html"), {200, "text/html; charset=utf-8", html, {}});

    qbweb::Choices ch;
    ch.articleOnly = false;
    ch.pictures = true;
    ch.asWebPage = false;
    // a page from a "public" address (the test names it so): pictures only from public addresses, three left out
    qputenv("QB_TEST_WEB_PUBLIC", QStringLiteral("127.0.0.1:%1").arg(pub.serverPort()).toUtf8());
    {
        qbweb::Loader loader;
        QString err;
        check(loader.load(QUrl(A + QStringLiteral("/page.html")), &err) && loader.pageIsPublic(), QStringLiteral("a page on a public address is loaded and counts as public"), err);
        loader.extract(ch);
        loader.fetchPictures();
        qb::importers::WebPage page;
        check(loader.result(ch, QString(), W, &page, &err), QStringLiteral("... and gives its result"), err);
        check(page.content.images.size() == 2 && loader.leftOutPictures() == 3,
              QStringLiteral("... two pictures fetched (one through a redirect inside the public address), three left out"),
              QStringLiteral("images %1 left out %2").arg(page.content.images.size()).arg(loader.leftOutPictures()));
        check(priv.requests.isEmpty(), QStringLiteral("... the private address (by number, by name, and by a redirect) was never asked"), priv.requests.join(QLatin1Char(' ')));
        check(page.content.warnings.join(QLatin1Char(' ')).contains(QLatin1String("3 pictures were left out")),
              QStringLiteral("... one line of the result says how many pictures were left out and why"), page.content.warnings.join(QLatin1Char('|')));
    }
    // a page that itself lies on such an address works as before: every picture is fetched
    qunsetenv("QB_TEST_WEB_PUBLIC");
    pub.requests.clear();
    {
        qbweb::Loader loader;
        QString err;
        loader.load(QUrl(A + QStringLiteral("/page.html")), &err);
        check(!loader.pageIsPublic(), QStringLiteral("a page on this computer's own address is not public"));
        loader.extract(ch);
        loader.fetchPictures();
        qb::importers::WebPage page;
        loader.result(ch, QString(), W, &page, &err);
        check(page.content.images.size() == 5 && loader.leftOutPictures() == 0 && !page.content.warnings.join(QLatin1Char(' ')).contains(QLatin1String("left out")),
              QStringLiteral("... its pictures are all fetched, as before, and nothing is said to be left out"),
              QStringLiteral("images %1 left out %2 ").arg(page.content.images.size()).arg(loader.leftOutPictures()) + page.content.warnings.join(QLatin1Char('|')));
    }
    // one picture left out: the line in the singular
    {
        qputenv("QB_TEST_WEB_PUBLIC", QStringLiteral("127.0.0.1:%1").arg(pub.serverPort()).toUtf8());
        QByteArray h1 = "<html><head><title>One</title></head><body><article><h1>One</h1><p>"
                        "The keeper climbed the tower, lit the lamp, and watched the sea until the morning boat came in.</p><img src=\""
                        + (B + QStringLiteral("/private1.png")).toUtf8() + "\" width=\"300\" height=\"200\"></article></body></html>";
        pub.pages.insert(QStringLiteral("/one.html"), {200, "text/html; charset=utf-8", h1, {}});
        qbweb::Loader loader;
        QString err;
        loader.load(QUrl(A + QStringLiteral("/one.html")), &err);
        loader.extract(ch);
        loader.fetchPictures();
        qb::importers::WebPage page;
        loader.result(ch, QString(), W, &page, &err);
        check(page.content.warnings.join(QLatin1Char(' ')).contains(QLatin1String("1 picture was left out"))
                  && !page.content.warnings.join(QLatin1Char(' ')).contains(QLatin1String("could not be fetched")),
              QStringLiteral("one picture: \"1 picture was left out\", and it is not counted as failed as well"), page.content.warnings.join(QLatin1Char('|')));
        qunsetenv("QB_TEST_WEB_PUBLIC");
    }
}

static void limits(const QString &W)
{
    // (a) install reads no file of more than 400 MB
    const QString big = W + QStringLiteral("/big.qbpkg");
    {
        QFile f(big);
        if (f.open(QIODevice::WriteOnly))
            f.resize(401ll * 1024 * 1024); // a sparse file
    }
    QString err;
    QStringList log;
    bool restart = false;
    const bool okI = addons::install(big, &log, &err, &restart, addons::Unsigned::AllowAll);
    check(!okI && err.contains(QLatin1String("too large")), QStringLiteral("install: a file of more than 400 MB is refused, as inspect refuses it"), err);
    check(addons::inspect(big).error.contains(QLatin1String("too large")), QStringLiteral("inspect: the same limit as before"));

    // (c) an archive whose sizes cannot be added up
    auto le = [](QByteArray &b, quint64 v, int n) {
        for (int i = 0; i < n; ++i)
            b.append(char((v >> (8 * i)) & 0xff));
    };
    auto archive = [&](const QList<quint64> &sizes) {
        QByteArray cd;
        for (int i = 0; i < sizes.size(); ++i) {
            const QByteArray name = "Proj/f" + QByteArray::number(i) + ".txt";
            le(cd, 0x02014b50, 4);
            le(cd, 45, 2); le(cd, 45, 2); le(cd, 0, 2); le(cd, 0, 2); le(cd, 0, 2); le(cd, 0, 2); // version made/needed, flags, method, time, date
            le(cd, 0, 4);                // crc
            le(cd, 0, 4);                // compressed size
            le(cd, 0xFFFFFFFFu, 4);      // size: in the ZIP64 field
            le(cd, name.size(), 2);
            le(cd, 12, 2);               // extra length
            le(cd, 0, 2); le(cd, 0, 2); le(cd, 0, 2); le(cd, 0, 4); // comment, disk, internal attributes, external attributes
            le(cd, 0, 4);                // offset
            cd += name;
            le(cd, 1, 2); le(cd, 8, 2); le(cd, sizes.at(i), 8);
        }
        QByteArray z = cd;
        le(z, 0x06054b50, 4);
        le(z, 0, 2); le(z, 0, 2); le(z, sizes.size(), 2); le(z, sizes.size(), 2);
        le(z, cd.size(), 4); le(z, 0, 4); le(z, 0, 2);
        return z;
    };
    {
        ZipReader ok(archive({5000, 7000}));
        check(ok.isValid() && ok.totalSize() == 12000, QStringLiteral("an archive with ordinary sizes: valid, and its total is the sum"), ok.errorString());
        ZipReader one(archive({0x4000000000000000ull, 0x4000000000000000ull}));
        check(one.isValid() && one.totalSize() == 0x8000000000000000ull, QStringLiteral("huge sizes that still add up are read (and a restore would say it does not fit)"), one.errorString());
        ZipReader bad(archive({0x8000000000000000ull, 0x8000000000000000ull, 100}));
        check(!bad.isValid() && !bad.errorString().isEmpty(), QStringLiteral("an archive whose sizes overflow the sum is refused"), bad.errorString());
        const QString zf = W + QStringLiteral("/overflow.zip");
        writeAll(zf, archive({0x8000000000000000ull, 0x8000000000000000ull, 100}));
        QString proj, e2;
        const QString r = backup::restore(zf, W + QStringLiteral("/restoreHere"), &proj, &e2);
        check(r.isEmpty() && !e2.isEmpty() && !QDir(W + QStringLiteral("/restoreHere")).exists(), QStringLiteral("a restore from such an archive is refused, and nothing is made"), e2);
    }

    // (d) a request on the link of the writing tools: one line of at most 64 KiB
    toolhost::Host &host = toolhost::host();
    check(host.startLink(), QStringLiteral("the instance link listens"), host.linkPath());
    // the server is in this process: the events are served while the answer is waited for
    auto ask = [&](const QByteArray &payload) {
        QLocalSocket s;
        s.connectToServer(host.linkPath());
        QElapsedTimer t;
        t.start();
        while (s.state() != QLocalSocket::ConnectedState && t.elapsed() < 2000)
            QTest::qWait(5);
        if (s.state() != QLocalSocket::ConnectedState)
            return QByteArray("no connection");
        s.write(payload);
        QByteArray got;
        while (!got.endsWith('\n') && s.state() == QLocalSocket::ConnectedState && t.elapsed() < 6000) {
            QTest::qWait(5);
            got += s.readAll();
        }
        got += s.readAll();
        return got;
    };
    auto lineOf = [](int total) { // a request of exactly `total` bytes before its newline
        QByteArray head = "{\"cmd\":\"info\",\"pad\":\"";
        const QByteArray tail = "\"}";
        return head + QByteArray(total - head.size() - tail.size(), 'x') + tail;
    };
    check(ask("{\"cmd\":\"info\"}\n").contains("\"pid\""), QStringLiteral("a short request is answered"));
    check(ask(lineOf(65536) + "\n").contains("\"pid\""), QStringLiteral("a request of 64 KiB is answered"));
    const QByteArray refusedLine = ask(lineOf(65537) + "\n");
    check(refusedLine.contains("too long") && !refusedLine.contains("\"pid\""), QStringLiteral("a request of one byte more is refused"), QString::fromLatin1(refusedLine.left(200)));
    const QByteArray refusedNoEnd = ask(QByteArray(200 * 1024, 'y'));
    check(refusedNoEnd.contains("too long"), QStringLiteral("a long stream without an end of line is refused too, without waiting for its end"), QString::fromLatin1(refusedNoEnd.left(200)));
    check(ask("{\"cmd\":\"info\"}\n").contains("\"pid\""), QStringLiteral("... and the link goes on answering"));
}

// ------------------------------------------------------------------ item 26: the window's end

static void windowEnd(const QString &W)
{
    Q_UNUSED(W);
    const int before = theApp->focusReceivers();
    int during = -1, duringSecond = -1;
    // 1: the keyboard is in a text field of the window, another window is there to take it
    QLineEdit other;
    other.setWindowTitle(QStringLiteral("Other window"));
    other.show();
    other.activateWindow();
    {
        auto *w = new MainWindow;
        w->resize(1100, 800);
        w->show();
        w->activateWindow();
        (void)QTest::qWaitForWindowActive(w, 2000);
        new Probe(w, &during);
        auto *own = w->findChild<QLineEdit *>();
        if (own) {
            own->setFocus();
            QTest::qWait(150);
        }
        check(theApp->focusReceivers() >= before + 2, QStringLiteral("a window listens to the application's focus signal while it is open"),
              QStringLiteral("before %1 now %2").arg(before).arg(theApp->focusReceivers()));
        delete w; // the focus goes to the other window while the window's widgets are deleted
        QTest::qWait(300);
    }
    check(during == before, QStringLiteral("when the window's own widgets are deleted, the window no longer listens to the focus signal"),
          QStringLiteral("receivers then %1, before the window %2").arg(during).arg(before));
    check(theApp->focusReceivers() == before, QStringLiteral("... and nothing of it is left afterwards"));
    // 2: a text field of another panel took the keyboard last (an add-on's panel: a widget with a timer of its own)
    {
        auto *w = new MainWindow;
        w->show();
        w->activateWindow();
        (void)QTest::qWaitForWindowActive(w, 2000);
        new Probe(w, &duringSecond);
        auto *panel = new QTextEdit(w);
        panel->setWindowFlag(Qt::Window, false);
        panel->show();
        panel->setFocus();
        QTimer::singleShot(0, panel, [] {});
        QTest::qWait(150);
        QTimer::singleShot(10, w, [w] { delete w; });
        other.setFocus();
        QTest::qWait(400);
    }
    check(duringSecond == before, QStringLiteral("... also when the keyboard was last in another panel's text field"), QString::number(duringSecond));
    other.hide();
    QTest::qWait(50);
    check(true, QStringLiteral("the window was deleted twice with the focus changing: no crash"));
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
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0930bTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0930bTest"));
    TestApp app(argc, argv);
    theApp = &app;
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.backupOnClose = false;
    as.backupOnOpen = false;
    as.backupOnManualSave = false;
    QTemporaryDir work;
    const QString W = work.path();
    webPictures(W);
    limits(W);
    exports(W);
    windowEnd(W);
    std::printf("%d failure(s)\n", failures);
    return failures;
}
