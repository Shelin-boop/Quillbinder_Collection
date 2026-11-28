// Scripted run for 0.99.0, item 112, part 2 (Linux): the recognizer and the word layer of Handwriting.
// Checks that need no model always run: the model's input (sizes), the codec, the greedy path with NFC, the word layer on frames
// made here (an umlaut word, a name kept as read, a split word, the main and the fallback language, the alternatives), the word
// lists, the install part with no engine. With the real model and word lists (a folder with ppocrv6-small.onnx,
// ppocrv6-small-codec.json, wordlist-en.tsv.gz and wordlist-de.tsv.gz: QB_HW_MODELS, else /home/claude/work/hw-models) and the
// ONNX Runtime pack (tools/get-onnxruntime.sh) lines of text are drawn with a handwriting-like free font (Comic Relief, OFL; a
// free font of the system) and read: the characters' and the words' accuracy with and without the word layer are printed.
//
//   xvfb-run -a ./qb_v0990o_tests /tmp/shots
#include "core/ModelMemory.h"
#include "ui/AiInstallPanel.h"
#include "ui/AppSettings.h"
#include "ui/Theme.h"
#include "ui/handwriting/HandwritingController.h"
#include "ui/handwriting/OnnxRecognizer.h"
#include "ui/handwriting/Recognizer.h"
#include "ui/handwriting/WordLayer.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QIcon>
#include <QStyleFactory>
#include <QThread>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

#include <cmath>
#include <cstdio>
#include <zlib.h>

using namespace qb;
using namespace qb::handwriting;

static int failures = 0;
static int passes = 0;
static QString outDir;

static void check(bool ok, const QString &what, const QString &detail = QString())
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", qPrintable(what));
    if (ok) {
        ++passes;
    } else {
        ++failures;
        if (!detail.isEmpty())
            std::printf("      %s\n", qPrintable(detail.left(2000)));
    }
    std::fflush(stdout);
}

static void settle(int ms = 100)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QTest::qWait(10);
    }
}

// ------------------------------------------------------------------ a small codec and frames made by hand

// blank 0, space 1, a-z 2..27, U+0308 28, U+0301 29, apostrophe 30, A-Z 31..56, comma 57
static Codec smallCodec()
{
    Codec c;
    c.classes = 58;
    auto add = [&](int l, const QString &t) {
        c.text.insert(l, t);
        c.label.insert(t, l);
    };
    add(1, QStringLiteral(" "));
    for (int i = 0; i < 26; ++i) {
        add(2 + i, QString(QChar('a' + i)));
        add(31 + i, QString(QChar('A' + i)));
    }
    add(28, QString(QChar(0x0308)));
    add(29, QString(QChar(0x0301)));
    add(30, QStringLiteral("'"));
    add(57, QStringLiteral(","));
    c.marks = {28, 29};
    c.space = 1;
    return c;
}

struct Step {
    QVector<QPair<int, double>> probs;   // the rest goes to the blank
};

static Step one(int label, double p = 0.9) { return {{{label, p}}}; }
static Step blankStep() { return {}; }

// the frames of a string: each letter its own step, a blank step between (needed for a double letter), a space as a step
static QVector<Step> spell(const Codec &c, const QString &text, double p = 0.9)
{
    QVector<Step> out;
    const QString nfd = text.normalized(QString::NormalizationForm_D);
    for (const QChar ch : nfd) {
        out << one(c.label.value(QString(ch)), p);
        out << blankStep();
    }
    return out;
}

static Frames framesOf(const Codec &c, const QVector<Step> &steps)
{
    Frames f;
    f.steps = int(steps.size());
    f.classes = c.classes;
    f.logp.resize(f.steps * f.classes);
    for (int t = 0; t < f.steps; ++t) {
        QVector<double> p(f.classes, 0.0);
        double used = 0;
        for (const auto &pr : steps[t].probs) {
            p[pr.first] += pr.second;
            used += pr.second;
        }
        p[0] += qMax(0.0, 1.0 - used);
        for (int k = 0; k < f.classes; ++k)
            f.logp[t * f.classes + k] = float(std::log(qMax(p[k], 1e-6) + 1e-9));
    }
    return f;
}

static QByteArray gzip(const QByteArray &in)
{
    z_stream zs{};
    deflateInit2(&zs, 6, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    QByteArray out(int(deflateBound(&zs, uLong(in.size()))) + 32, 0);
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.constData()));
    zs.avail_in = uInt(in.size());
    zs.next_out = reinterpret_cast<Bytef *>(out.data());
    zs.avail_out = uInt(out.size());
    deflate(&zs, Z_FINISH);
    out.resize(int(zs.total_out));
    deflateEnd(&zs);
    return out;
}

static QString joinWords(const LayerResult &r)
{
    QStringList w;
    for (const LayerWord &x : r.words)
        w << x.text;
    return w.join(QLatin1Char(' '));
}

static void testInput()
{
    // sizes: height 96, 16 px white left and right, then white up to a multiple of 8, three equal planes, value = 1 - pixel/255
    for (const QSize sz : {QSize(100, 50), QSize(37, 96), QSize(300, 40), QSize(8, 8), QSize(1000, 120)}) {
        QImage img(sz, QImage::Format_RGB32);
        img.fill(Qt::white);
        {
            QPainter p(&img);
            p.fillRect(QRect(sz.width() / 4, sz.height() / 4, sz.width() / 2, sz.height() / 2), Qt::black);
        }
        const OnnxRecognizer::Input in = OnnxRecognizer::prepareInput(img);
        const int w = qMax(1, int(std::lround(sz.width() * 96.0 / sz.height())));
        const int minWidth = 16 + w + 16;
        check(in.height == 96 && in.width % 8 == 0 && in.width >= minWidth && in.width < minWidth + 8,
              QStringLiteral("the input of a %1x%2 picture: 96 high, width %3 is a multiple of 8 (at least %4)").arg(sz.width()).arg(sz.height()).arg(in.width).arg(minWidth));
        check(in.data.size() == 3 * 96 * in.width, QStringLiteral("... three planes of 96 x %1").arg(in.width));
        const qint64 plane = qint64(96) * in.width;
        bool padWhite = true, planesEqual = true;
        for (int y = 0; y < 96; ++y) {
            for (int x = 0; x < 16; ++x)
                padWhite = padWhite && in.data[int(y * in.width + x)] == 0.0f;
            for (int x = 16 + w; x < in.width; ++x)
                padWhite = padWhite && in.data[int(y * in.width + x)] == 0.0f;
        }
        for (int i = 0; i < plane; i += 97)
            planesEqual = planesEqual && in.data[i] == in.data[int(plane + i)] && in.data[i] == in.data[int(2 * plane + i)];
        check(padWhite, QStringLiteral("... white (0) padding left and right"));
        check(planesEqual, QStringLiteral("... the three planes are the same"));
        const float mid = in.data[int(48 * in.width + 16 + w / 2)];
        check(mid > 0.99f, QStringLiteral("... ink is 1 (black), got %1").arg(mid));
    }
    // a see-through picture counts as white where it is empty
    QImage alpha(40, 20, QImage::Format_ARGB32);
    alpha.fill(Qt::transparent);
    const OnnxRecognizer::Input t = OnnxRecognizer::prepareInput(alpha);
    float mx = 0;
    for (float v : t.data)
        mx = qMax(mx, v);
    check(mx == 0.0f, QStringLiteral("a see-through picture is white"));
}

static void testCodecAndGreedy()
{
    const QByteArray json = R"({"codec":{" ":[1],"a":[2],"b":[3],"̈":[4],"l":[5]},"output":{"blank":0,"classes":7}})";
    QString err;
    const Codec c = Codec::fromJson(json, &err);
    check(c.valid() && c.classes == 7 && c.label.value(QStringLiteral("a")) == 2 && c.marks == QVector<int>{4} && c.space == 1,
          QStringLiteral("the codec JSON: labels, the marks, the space, the classes"), err);
    check(!Codec::fromJson("not json").valid(), QStringLiteral("a broken codec is refused"));

    const Codec sc = smallCodec();
    // "hello world": the double l needs the blank between; an unbroken repeat is one letter
    QVector<Step> st = spell(sc, QStringLiteral("hello"));
    st << one(1) << blankStep();
    st << spell(sc, QStringLiteral("world"));
    check(joinWords(readPlain(framesOf(sc, st), sc)) == QStringLiteral("hello world"), QStringLiteral("greedy CTC: hello world with the double l"));
    QVector<Step> rep = {one(sc.label.value(QStringLiteral("a"))), one(sc.label.value(QStringLiteral("a"))), blankStep(), one(sc.label.value(QStringLiteral("b")))};
    check(joinWords(readPlain(framesOf(sc, rep), sc)) == QStringLiteral("ab"), QStringLiteral("greedy CTC: a repeat without a blank is one letter"));
    // NFC after decoding: u + U+0308 comes out as one character
    const QString u = joinWords(readPlain(framesOf(sc, spell(sc, QStringLiteral("müde"))), sc));
    check(u == QStringLiteral("müde") && u.size() == 4, QStringLiteral("NFC after decoding: u + diaeresis is one character"), u);
    // punctuation stays around the word
    QVector<Step> pun = {one(sc.label.value(QStringLiteral("'")))};
    check(readPlain(framesOf(sc, pun), sc).words.size() == 1, QStringLiteral("a mark with no letters stays as read"));
}

static void testWordLists()
{
    check(foldWord(QStringLiteral("Über")) == QStringLiteral("uber") && foldWord(QStringLiteral("Straße")) == QStringLiteral("strasse")
              && foldWord(QStringLiteral("café")) == QStringLiteral("cafe"),
          QStringLiteral("folding: lower case, no diacritics, the sharp s is ss"));
    check(boundedEditDistance(QStringLiteral("house"), QStringLiteral("hause"), 2) == 1 && boundedEditDistance(QStringLiteral("house"), QStringLiteral("horse"), 2) == 1
              && boundedEditDistance(QStringLiteral("abc"), QStringLiteral("xyz"), 2) == 3 && boundedEditDistance(QStringLiteral("a"), QStringLiteral("abcdef"), 2) == 3,
          QStringLiteral("the bounded edit distance"));
    check(allowedDistance(2) == 1 && allowedDistance(4) == 2 && allowedDistance(7) == 3 && allowedDistance(12) == 4, QStringLiteral("1 to 4 edits by length"));

    WordList list;
    const QByteArray tsv = "the\t7.73\nhouse\t5.5\nmüde\t4.2\nbad line\nx\tnot a number\n" "\xc3\xbc" "ber\t5.9\n";
    check(list.loadTsv(tsv) && list.size() == 4, QStringLiteral("a word list from TSV: bad lines skipped"), QString::number(list.size()));
    QTemporaryDir dir;
    QFile f(dir.path() + QStringLiteral("/wl.tsv.gz"));
    f.open(QIODevice::WriteOnly);
    f.write(gzip(tsv));
    f.close();
    WordList gz;
    QString err;
    check(gz.loadGzip(f.fileName(), &err) && gz.size() == 4 && gz.words()[2].folded == QStringLiteral("mude") && qAbs(gz.words()[0].zipf - 7.73f) < 0.001f,
          QStringLiteral("a gzip'd word list is read (compare form without diacritics)"), err);
    QFile bad(dir.path() + QStringLiteral("/bad.gz"));
    bad.open(QIODevice::WriteOnly);
    bad.write("not gzip at all");
    bad.close();
    check(!WordList().loadGzip(bad.fileName(), &err), QStringLiteral("a file that is no gzip is refused"));
}

static WordList makeList(const QByteArray &tsv)
{
    WordList l;
    l.loadTsv(tsv);
    return l;
}

static void testWordLayer()
{
    const Codec c = smallCodec();
    auto L = [&](const QString &s) { return c.label.value(s); };
    const WordList en = makeList("the\t7.7\nhouse\t6.2\nhorse\t5.0\nmouse\t5.5\nworld\t6.0\nhello\t6.5\nand\t7.5\nmud\t4.0\n");

    // a near miss: the frames give a 0.5 and o 0.45 for the second letter's neighbor; the list word wins over the raw reading
    QVector<Step> st = spell(c, QStringLiteral("h"));
    st << Step{{{L(QStringLiteral("a")), 0.5}, {L(QStringLiteral("o")), 0.45}}} << blankStep();
    st << spell(c, QStringLiteral("use"));
    Frames f = framesOf(c, st);
    const LayerResult plain = readPlain(f, c);
    const LayerResult layered = readWithList(f, c, en);
    check(joinWords(plain) == QStringLiteral("hause") && joinWords(layered) == QStringLiteral("house"),
          QStringLiteral("the word layer: hause -> house (plain reading hause)"), joinWords(plain) + QStringLiteral(" / ") + joinWords(layered));
    check(layered.words.size() == 1 && layered.words[0].listed && layered.words[0].alternatives.contains(QStringLiteral("hause")),
          QStringLiteral("the alternatives of the word hold the raw reading"), layered.words.value(0).alternatives.join(QLatin1Char(',')));

    // the case of the raw reading is kept: a capital, all capitals
    QVector<Step> cap = spell(c, QStringLiteral("H"));
    cap << Step{{{L(QStringLiteral("a")), 0.5}, {L(QStringLiteral("o")), 0.45}}} << blankStep() << spell(c, QStringLiteral("use"));
    check(joinWords(readWithList(framesOf(c, cap), c, en)) == QStringLiteral("House"), QStringLiteral("a capital letter of the raw reading is kept"));
    QVector<Step> caps = spell(c, QStringLiteral("H"));
    caps << Step{{{L(QStringLiteral("A")), 0.5}, {L(QStringLiteral("O")), 0.45}}} << blankStep() << spell(c, QStringLiteral("USE"));
    check(joinWords(readWithList(framesOf(c, caps), c, en)) == QStringLiteral("HOUSE"), QStringLiteral("all capitals are kept"));

    // a name that no listed word fits better keeps the characters as read
    const LayerResult name = readWithList(framesOf(c, spell(c, QStringLiteral("Zorbaldo"))), c, en);
    check(joinWords(name) == QStringLiteral("Zorbaldo") && !name.words[0].listed, QStringLiteral("a name is kept as read"), joinWords(name));
    // ... also next to listed words and with punctuation around it
    QVector<Step> sent = spell(c, QStringLiteral("hello"));
    sent << one(1) << blankStep() << spell(c, QStringLiteral("Zorbaldo,")) << one(1) << blankStep() << spell(c, QStringLiteral("and"));
    check(joinWords(readWithList(framesOf(c, sent), c, en)) == QStringLiteral("hello Zorbaldo, and"), QStringLiteral("a name with a comma between listed words"),
          joinWords(readWithList(framesOf(c, sent), c, en)));

    // an umlaut word: the mark is weak (0.3 against the blank 0.7), the greedy path misses it; the listed word with the umlaut wins
    const WordList de = makeList("müde\t4.5\nund\t7.5\nhaus\t6.0\n" "\xc3\xbc" "ber\t6.5\nmude\t1.0\n");
    QVector<Step> um = spell(c, QStringLiteral("mu"));
    um.removeLast();   // no blank after the u: the mark's step follows
    um << Step{{{L(QString(QChar(0x0308))), 0.3}}} << blankStep() << spell(c, QStringLiteral("de"));
    f = framesOf(c, um);
    check(joinWords(readPlain(f, c)) == QStringLiteral("mude") && joinWords(readWithList(f, c, de)) == QStringLiteral("müde"),
          QStringLiteral("an umlaut word: mude -> müde"), joinWords(readWithList(f, c, de)));
    // all accent marks count as one: an acute where the list has an umlaut
    QVector<Step> ac = spell(c, QStringLiteral("mu"));
    ac.removeLast();
    ac << Step{{{L(QString(QChar(0x0301))), 0.3}}} << blankStep() << spell(c, QStringLiteral("de"));
    check(joinWords(readWithList(framesOf(c, ac), c, de)) == QStringLiteral("müde"), QStringLiteral("an acute counts like an umlaut in the scoring"));

    // a word split in two: no single word is close, both parts are listed
    const LayerResult split = readWithList(framesOf(c, spell(c, QStringLiteral("thehouse"))), c, en);
    check(split.words.size() == 2 && split.words[0].text == QStringLiteral("the") && split.words[1].text == QStringLiteral("house"),
          QStringLiteral("a word with no close single word is split in two listed words"), joinWords(split));
    // not split when a single word is close
    check(joinWords(readWithList(framesOf(c, spell(c, QStringLiteral("housee"))), c, en)).split(QLatin1Char(' ')).size() == 1,
          QStringLiteral("a word with a close single word is not split"));
    // not split into parts shorter than two letters
    check(joinWords(readWithList(framesOf(c, spell(c, QStringLiteral("qzhouse"))), c, en)) != QStringLiteral("q zhouse"), QStringLiteral("no part of one letter"));

    // the main and the fallback language: each line is read with both, the better total is kept
    QVector<Step> g = spell(c, QStringLiteral("mu"));
    g.removeLast();
    g << Step{{{L(QString(QChar(0x0308))), 0.3}}} << blankStep() << spell(c, QStringLiteral("de")) << one(1) << blankStep() << spell(c, QStringLiteral("und"))
      << one(1) << blankStep() << spell(c, QStringLiteral("haus"));
    f = framesOf(c, g);
    int chosen = -2;
    LayerResult r1 = readBest(f, c, {&en, &de}, &chosen);
    check(chosen == 1 && joinWords(r1) == QStringLiteral("müde und haus"), QStringLiteral("main English, fallback German: the German list is kept"), joinWords(r1));
    LayerResult r2 = readBest(f, c, {&de, &en}, &chosen);
    check(chosen == 0 && joinWords(r2) == joinWords(r1), QStringLiteral("main German, fallback English: the German list, the same text"));
    QVector<Step> e = spell(c, QStringLiteral("hello")) + QVector<Step>{one(1), blankStep()} + spell(c, QStringLiteral("world"));
    readBest(framesOf(c, e), c, {&de, &en}, &chosen);
    check(chosen == 1, QStringLiteral("an English line with German as main: the fallback is kept"));
    readBest(framesOf(c, e), c, {}, &chosen);
    check(chosen == -1, QStringLiteral("no lists: the characters as read"));

    // CTC forward: the right word beats a wrong one; too few steps gives nothing
    f = framesOf(c, spell(c, QStringLiteral("house")));
    check(ctcLogProb(f, c, QStringLiteral("house"), 0, f.steps - 1) > ctcLogProb(f, c, QStringLiteral("horse"), 0, f.steps - 1) + 1.0,
          QStringLiteral("CTC forward: house fits its frames better than horse"));
    check(ctcLogProb(f, c, QStringLiteral("house"), 0, 2) <= -1e3, QStringLiteral("CTC forward: too few steps"));
    check(ctcLogProb(f, c, QStringLiteral("HOUSE"), 0, f.steps - 1) == ctcLogProb(f, c, QStringLiteral("house"), 0, f.steps - 1),
          QStringLiteral("CTC forward: upper and lower case are one group"));
}

// ------------------------------------------------------------------ the install part and the registry (no engine)

static void testNoEngine()
{
    OnnxRecognizer none(nullptr, nullptr, [](const QStringList &) { return OnnxRecognizer::Sources(); });
    check(!none.isInstalled() && none.languages().isEmpty(), QStringLiteral("with no engine: not installed, no languages"));
    Request req;
    req.picture = QImage(100, 40, QImage::Format_RGB32);
    req.picture.fill(Qt::white);
    req.languages = {QStringLiteral("en")};
    bool called = false, ok = true;
    QString error;
    none.recognize(req, [&](const Result &r) {
        called = true;
        ok = r.ok;
        error = r.error;
    });
    settle(100);
    check(called && !ok && !error.isEmpty(), QStringLiteral("with no engine: recognize answers with a sentence for the message line"), error);

    // the first use with the real recognizer and nothing installed: the page opens (the controller asks for it)
    Recognizer::setCurrent(&none);
    check(!Recognizer::installed() && !HandwritingController::recognizerReady(), QStringLiteral("the controller sees no recognizer ready"));
    Recognizer::setCurrent(nullptr);

    // the installer: registered by installOnnxRecognizer, with the status and the dialog of the engine, the model and the lists
    OnnxRecognizer *r = installOnnxRecognizer(nullptr);
    const Recognizer::Installer &inst = Recognizer::installer();
    check(inst.install && inst.status && inst.remove, QStringLiteral("the install part is set: Install, status, Remove"));
    check(inst.status().contains(QStringLiteral("Not installed")), QStringLiteral("the status without an engine says so"), inst.status());
    bool dialogSeen = false;
    QStringList kinds;
    QTimer::singleShot(300, [&] {
        if (QWidget *m = QApplication::activeModalWidget()) {
            if (auto *panel = m->findChild<AiInstallPanel *>(QStringLiteral("hwInstallPanel"))) {
                dialogSeen = true;
                kinds = panel->kinds();
                m->grab().save(outDir + QStringLiteral("/hw2-install.png"));
            }
            m->close();
        }
    });
    inst.install(nullptr);
    check(dialogSeen && kinds == QStringList({QStringLiteral("handwriting"), QStringLiteral("wordlist")}) && !(dialogSeen && kinds.isEmpty()),
          QStringLiteral("Install opens the panel of the handwriting engine: the line model and the word lists"), kinds.join(QLatin1Char(',')));
    Recognizer::setCurrent(nullptr);
    delete r;
    check(Recognizer::current() == nullptr, QStringLiteral("the recognizer is gone with its owner"));
}

// ------------------------------------------------------------------ the real model

static QString g_family;

static QImage renderLine(const QString &text, int px, quint32 seed, bool degrade)
{
    QRandomGenerator rng(seed);
    QFont f(g_family);
    f.setPixelSize(px);
    const QFontMetrics fm(f);
    const int w = fm.horizontalAdvance(text) + 60, h = px * 2 + 20;
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setFont(f);
    p.setPen(QColor(20, 20, 40));
    double x = 25;
    const double base = px * 1.35;
    for (const QChar ch : text) {
        p.save();
        const double jy = degrade ? (rng.generateDouble() * 0.2 - 0.1) * px : 0;
        const double jr = degrade ? (rng.generateDouble() * 22.0 - 11.0) : 0;
        p.translate(x, base + jy);
        p.rotate(jr);
        p.drawText(0, 0, QString(ch));
        p.restore();
        x += fm.horizontalAdvance(ch) + (degrade ? (rng.generateDouble() * 0.14 - 0.06) * px : 0);
    }
    p.end();
    if (degrade) {
        // blur (down and up) and a little noise
        img = img.scaled(img.width() * 3 / 10, img.height() * 3 / 10, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                  .scaled(img.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (int y = 0; y < img.height(); ++y) {
            QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (int xx = 0; xx < img.width(); ++xx) {
                const int n = rng.bounded(-50, 51);
                const int g = qBound(0, qGray(row[xx]) + n, 255);
                row[xx] = qRgb(g, g, g);
            }
        }
    }
    // the box of the ink and a margin of 8 px: what the writing surface gives
    int x0 = img.width(), x1 = 0, y0 = img.height(), y1 = 0;
    for (int y = 0; y < img.height(); ++y)
        for (int xx = 0; xx < img.width(); ++xx)
            if (qGray(img.pixel(xx, y)) < 160) {
                x0 = qMin(x0, xx);
                x1 = qMax(x1, xx);
                y0 = qMin(y0, y);
                y1 = qMax(y1, y);
            }
    if (x1 < x0)
        return img;
    return img.copy(QRect(QPoint(x0, y0), QPoint(x1, y1)).adjusted(-8, -8, 8, 8)).convertToFormat(QImage::Format_RGB32);
}

static int editDistance(const QString &a, const QString &b)
{
    QVector<int> prev(b.size() + 1), cur(b.size() + 1);
    for (int j = 0; j <= b.size(); ++j)
        prev[j] = j;
    for (int i = 1; i <= a.size(); ++i) {
        cur[0] = i;
        for (int j = 1; j <= b.size(); ++j)
            cur[j] = qMin(qMin(prev[j] + 1, cur[j - 1] + 1), prev[j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1));
        std::swap(prev, cur);
    }
    return prev[b.size()];
}

struct Score {
    int chars = 0, charErrors = 0, words = 0, wordsRight = 0;
    double charAcc() const { return chars ? 1.0 - double(charErrors) / chars : 0; }
    double wordAcc() const { return words ? double(wordsRight) / words : 0; }
    void add(const QString &ref, const QString &got)
    {
        chars += int(ref.size());
        charErrors += qMin(int(ref.size()), editDistance(ref, got));
        const QStringList r = ref.split(QLatin1Char(' '), Qt::SkipEmptyParts), g = got.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        words += int(r.size());
        for (int i = 0; i < r.size(); ++i)
            if (i < g.size() && r[i] == g[i])
                ++wordsRight;
    }
};

static Result readSync(OnnxRecognizer &rec, const QImage &img, const QStringList &langs)
{
    Request q;
    q.picture = img;
    q.languages = langs;
    return rec.readNow(q);
}

static void testRealModel(const QString &dir)
{
    const QString runtime = QStringLiteral(QB_TEST_BUILD_DIR) + QStringLiteral("/onnxruntime/lib/libonnxruntime.so.1");
    auto provider = [dir, runtime](const QStringList &langs) {
        OnnxRecognizer::Sources s;
        s.runtimeLibrary = runtime;
        s.model = dir + QStringLiteral("/ppocrv6-small.onnx");
        s.codec = dir + QStringLiteral("/ppocrv6-small-codec.json");
        for (const QString &l : langs) {
            const QString f = dir + QStringLiteral("/wordlist-") + l + QStringLiteral(".tsv.gz");
            if (QFileInfo::exists(f))
                s.wordLists.insert(l, f);
        }
        return s;
    };
    modelmem::ModelMemory mm;
    OnnxRecognizer rec(nullptr, &mm, provider);
    check(rec.isInstalled() && rec.languages().contains(QStringLiteral("de")) && !rec.languages().contains(QStringLiteral("ru")),
          QStringLiteral("the real engine and model are found: ten languages, no Russian"));

    struct Line { QString text; QString lang; };
    const QList<Line> lines = {
        {QStringLiteral("the old house stood near the river"), QStringLiteral("en")},
        {QStringLiteral("she walked home before the rain began"), QStringLiteral("en")},
        {QStringLiteral("we bought bread and some cheese today"), QStringLiteral("en")},
        {QStringLiteral("a quiet morning in the small garden"), QStringLiteral("en")},
        {QStringLiteral("please send the letter to my brother"), QStringLiteral("en")},
        {QStringLiteral("the children played outside all afternoon"), QStringLiteral("en")},
        {QStringLiteral("there is nothing better than fresh coffee"), QStringLiteral("en")},
        {QStringLiteral("he forgot his umbrella on the train"), QStringLiteral("en")},
        {QStringLiteral("das kleine Haus steht am Fluss"), QStringLiteral("de")},
        {QStringLiteral("wir gehen morgen in die Stadt"), QStringLiteral("de")},
        {QStringLiteral("die Kinder spielen im grossen Garten"), QStringLiteral("de")},
        {QStringLiteral("ich habe heute viel gearbeitet"), QStringLiteral("de")},
        {QStringLiteral("der Müller wohnt über der Bäckerei"), QStringLiteral("de")},
        {QStringLiteral("ein schöner Tag für einen Spaziergang"), QStringLiteral("de")},
    };
    Score off, on, offClean, onClean;
    QImage sheet(900, 60 * int(lines.size()) + 20, QImage::Format_RGB32);
    sheet.fill(Qt::white);
    QPainter sp(&sheet);
    QFont sf(QStringLiteral("Noto Sans"), 9);
    sp.setFont(sf);
    int row = 0;
    QElapsedTimer timer;
    timer.start();
    int linesRead = 0;
    for (const Line &ln : lines) {
        const QStringList langs = ln.lang == QLatin1String("en") ? QStringList{QStringLiteral("en"), QStringLiteral("de")}
                                                                  : QStringList{QStringLiteral("de"), QStringLiteral("en")};
        const QImage clean = renderLine(ln.text, 56, 7 + row, false);
        const QImage noisy = renderLine(ln.text, 56, 100 + row, true);
        rec.setWordLayer(false);
        const Result a = readSync(rec, noisy, langs);
        const Result ac = readSync(rec, clean, langs);
        rec.setWordLayer(true);
        const Result b = readSync(rec, noisy, langs);
        const Result bc = readSync(rec, clean, langs);
        linesRead += 4;
        off.add(ln.text, a.text());
        on.add(ln.text, b.text());
        offClean.add(ln.text, ac.text());
        onClean.add(ln.text, bc.text());
        if (qEnvironmentVariableIsSet("QB_HW_DEBUG"))
            for (const Word &w : b.words)
                std::printf("   word %-14s alt %s\n", qPrintable(w.text), qPrintable(w.alternatives.join(QLatin1Char('|'))));
        const QImage small = noisy.scaledToHeight(34, Qt::SmoothTransformation);
        sp.drawImage(10, row * 60 + 8, small);
        sp.drawText(QRect(10 + qMin(small.width(), 520) + 10, row * 60 + 4, 360, 50), Qt::TextWordWrap,
                    QStringLiteral("%1\n%2\n%3").arg(ln.text, a.text(), b.text()));
        ++row;
    }
    sp.end();
    sheet.save(outDir + QStringLiteral("/hw2-lines.png"));
    const double perLine = timer.elapsed() / double(linesRead);
    std::printf("ACCURACY (14 lines, drawn with %s)\n", qPrintable(g_family));
    std::printf("  clean lines : characters %.1f %% without the word layer, %.1f %% with; words %.1f %% without, %.1f %% with\n", 100 * offClean.charAcc(), 100 * onClean.charAcc(),
                100 * offClean.wordAcc(), 100 * onClean.wordAcc());
    std::printf("  blurred, tilted, noisy lines : characters %.1f %% without, %.1f %% with; words %.1f %% without, %.1f %% with\n", 100 * off.charAcc(), 100 * on.charAcc(),
                100 * off.wordAcc(), 100 * on.wordAcc());
    std::printf("  %.0f ms per line (2 cores shared)\n", perLine);
    check(offClean.charAcc() > 0.85, QStringLiteral("clean lines are read: characters without the word layer above 85 %"), QString::number(offClean.charAcc()));
    check(off.charAcc() > 0.6, QStringLiteral("degraded lines: characters without the word layer above 60 %"), QString::number(off.charAcc()));
    check(onClean.wordAcc() >= offClean.wordAcc() - 0.001, QStringLiteral("the word layer does not lower the words right on clean lines"),
          QStringLiteral("%1 -> %2").arg(offClean.wordAcc()).arg(onClean.wordAcc()));
    check(on.wordAcc() >= off.wordAcc(), QStringLiteral("the word layer does not lower the words right on degraded lines"),
          QStringLiteral("%1 -> %2").arg(off.wordAcc()).arg(on.wordAcc()));

    // an umlaut word, a name kept as read, the main and the fallback language
    rec.setWordLayer(true);
    const Result um = readSync(rec, renderLine(QStringLiteral("Mädchen und Bücher"), 56, 3, false), {QStringLiteral("de"), QStringLiteral("en")});
    check(um.text() == QStringLiteral("Mädchen und Bücher"), QStringLiteral("real model: an umlaut line is read with its umlauts"), um.text());
    const Result nm = readSync(rec, renderLine(QStringLiteral("then Zorbaldo came home"), 56, 4, false), {QStringLiteral("en"), QStringLiteral("de")});
    check(nm.text().contains(QStringLiteral("Zorbaldo")), QStringLiteral("real model: a name no list knows is kept as read"), nm.text());
    const QImage ger = renderLine(QStringLiteral("das große Haus der Kinder"), 56, 5, false);
    const Result g1 = readSync(rec, ger, {QStringLiteral("en"), QStringLiteral("de")});
    const Result g2 = readSync(rec, ger, {QStringLiteral("de"), QStringLiteral("en")});
    check(g1.text() == g2.text() && g1.text().contains(QStringLiteral("Haus")), QStringLiteral("real model: German text reads the same with German as the main or the fallback language"),
          g1.text() + QStringLiteral(" / ") + g2.text());
    // the alternatives of a word and the boxes
    const QImage alt = renderLine(QStringLiteral("the hause by the river"), 56, 9, true);
    const Result ar = readSync(rec, alt, {QStringLiteral("en")});
    bool boxesOk = !ar.words.isEmpty(), someAlt = false;
    double lastRight = -1;
    for (const Word &w : ar.words) {
        boxesOk = boxesOk && w.box.left() >= lastRight - 1 && w.box.width() > 0 && w.box.right() <= alt.width() + 1;
        lastRight = w.box.left();
        someAlt = someAlt || !w.alternatives.isEmpty();
    }
    check(ar.ok && boxesOk, QStringLiteral("real model: every word has a box inside the picture, left to right"));
    check(someAlt, QStringLiteral("real model: a word has alternatives (the next candidates, the raw reading)"), ar.text());
    // a blank picture gives no words
    QImage blank(200, 60, QImage::Format_RGB32);
    blank.fill(Qt::white);
    const Result br = readSync(rec, blank, {QStringLiteral("en")});
    check(br.ok && br.text().trimmed().isEmpty(), QStringLiteral("real model: a blank picture gives no text"), br.text());
    // Russian has no model: no list, no model language; the request still reads (the characters as read)
    check(!hasModelFor(QStringLiteral("ru")), QStringLiteral("Russian has no model"));

    // the registry: asking registers the model, the idle time unloads it, the next line loads it again; the thread is not the main one
    rec.unloadIfIdle();
    check(!rec.isLoaded(), QStringLiteral("the model is unloaded by hand"));
    qint64 now = 0;
    mm.setClock([&] { return now; });
    Request q;
    q.picture = renderLine(QStringLiteral("hello world"), 56, 1, false);
    q.languages = {QStringLiteral("en")};
    bool done = false;
    Result async;
    Qt::HANDLE doneThread = nullptr;
    rec.recognize(q, [&](const Result &r) {
        done = true;
        async = r;
        doneThread = QThread::currentThreadId();
    });
    QElapsedTimer wait;
    wait.start();
    while (!done && wait.elapsed() < 30000)
        settle(50);
    check(done && async.ok && async.text() == QStringLiteral("hello world"), QStringLiteral("recognize: the answer comes later and is right"), async.text() + async.error);
    check(doneThread == QThread::currentThreadId(), QStringLiteral("recognize: the answer comes in the thread that asked"));
    check(mm.isLoaded(rec.memoryId()) && rec.isLoaded(), QStringLiteral("the registry knows the model (handwriting:line) while it is loaded"));
    const QList<modelmem::LoadedModel> models = mm.models();
    check(models.size() == 1 && models[0].id == QStringLiteral("handwriting:line") && models[0].bytes > 0, QStringLiteral("... with a figure for the memory rule"));
    now += 5 * 60 * 1000;
    check(mm.unloadIdle().isEmpty() && rec.isLoaded(), QStringLiteral("5 minutes unused: still loaded"));
    now += 6 * 60 * 1000;
    const QStringList gone = mm.unloadIdle();
    check(gone == QStringList{QStringLiteral("handwriting:line")} && !rec.isLoaded() && !mm.isLoaded(rec.memoryId()),
          QStringLiteral("11 minutes unused: the model is unloaded and leaves the registry"), gone.join(QLatin1Char(',')));
    done = false;
    rec.recognize(q, [&](const Result &r) {
        done = true;
        async = r;
    });
    wait.restart();
    while (!done && wait.elapsed() < 30000)
        settle(50);
    check(done && async.text() == QStringLiteral("hello world") && rec.isLoaded() && mm.isLoaded(rec.memoryId()), QStringLiteral("the next line loads the model again"), async.text());
    mm.reset();
    rec.unloadIfIdle();
}

int main(int argc, char **argv)
{
    QTemporaryDir home;
    qputenv("XDG_CONFIG_HOME", (home.path() + QLatin1String("/config")).toUtf8());
    qputenv("XDG_DATA_HOME", (home.path() + QLatin1String("/data")).toUtf8());
    qputenv("QB_NO_DIALOGS", "1");
    QCoreApplication::setOrganizationName(QStringLiteral("QuillbinderV0990oTest"));
    QCoreApplication::setApplicationName(QStringLiteral("QuillbinderV0990oTest"));
    QApplication app(argc, argv);
    outDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath() + QLatin1String("/shots");
    QDir().mkpath(outDir);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral(":/icons"));
    QIcon::setThemeName(QStringLiteral("qbicons"));
    AppSettings &as = AppSettings::instance();
    as.load();
    as.theme = QStringLiteral("light");
    theme::apply();

    testInput();
    testCodecAndGreedy();
    testWordLists();
    testWordLayer();
    testNoEngine();

    QString models = qEnvironmentVariable("QB_HW_MODELS", QStringLiteral("/home/claude/work/hw-models"));
    const QString runtime = QStringLiteral(QB_TEST_BUILD_DIR) + QStringLiteral("/onnxruntime/lib/libonnxruntime.so.1");
    const QStringList need = {QStringLiteral("ppocrv6-small.onnx"), QStringLiteral("ppocrv6-small-codec.json"), QStringLiteral("wordlist-en.tsv.gz"), QStringLiteral("wordlist-de.tsv.gz")};
    bool have = QFileInfo::exists(runtime);
    for (const QString &n : need)
        have = have && QFileInfo::exists(models + QLatin1Char('/') + n);
    const QString fontFile = QStringLiteral("/usr/share/fonts/truetype/google-fonts/ofl/comicrelief/ComicRelief-Regular.ttf");
    const int fid = QFileInfo::exists(fontFile) ? QFontDatabase::addApplicationFont(fontFile) : -1;
    if (!have) {
        std::printf("SKIPPED  the real-model checks: the model files (%s) or the ONNX Runtime pack (%s) are not there\n", qPrintable(models), qPrintable(runtime));
    } else if (fid < 0) {
        std::printf("SKIPPED  the real-model checks: the free handwriting-like font (Comic Relief, OFL) is not on this system\n");
    } else {
        g_family = QFontDatabase::applicationFontFamilies(fid).value(0);
        testRealModel(models);
    }
    std::printf("%d pass\n%d failure(s)\n", passes, failures);
    return failures ? 1 : 0;
}
