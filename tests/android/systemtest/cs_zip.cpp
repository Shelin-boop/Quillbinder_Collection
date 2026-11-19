// The system test for Android: ZIP files and packages written by the test itself. Linux's cases make them with
// Python (kZipHelper, the deep DOCX and ODT of C-41) and with tools/make-package.py (S-A07); a tablet has no Python,
// so Android's copy writes them here (2026-11-16), with the same content: entries stored as given, a wrong checksum
// for "crc:<name>", the file cut inside the entry for "cut:<name>", packages as make-package.py writes them
// (manifest.json with every payload file's SHA-256 and size; unsigned, as make-package.py without a key).
#include "cs.h"

#include "AppInfo.h"
#include "core/Platform.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace cs {

namespace {

quint32 crc32Of(const QByteArray &data)
{
    static quint32 table[256];
    static bool made = false;
    if (!made) {
        for (quint32 i = 0; i < 256; ++i) {
            quint32 c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[i] = c;
        }
        made = true;
    }
    quint32 c = 0xFFFFFFFFu;
    for (const char ch : data)
        c = table[(c ^ quint8(ch)) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

void put16(QByteArray &b, quint16 v)
{
    b.append(char(v & 0xFF));
    b.append(char(v >> 8));
}

void put32(QByteArray &b, quint32 v)
{
    put16(b, quint16(v & 0xFFFF));
    put16(b, quint16(v >> 16));
}

} // namespace

bool writeStoredZip(const QString &out, const QList<ZipPart> &parts)
{
    QByteArray file, central;
    for (const ZipPart &p : parts) {
        const QByteArray name = p.name.toUtf8();
        const quint32 crc = p.badCrc ? 0xEFBEADDEu : crc32Of(p.data); // the bytes DE AD BE EF, as kZipHelper writes them
        const quint32 offset = quint32(file.size());
        put32(file, 0x04034b50);
        put16(file, 20);
        put16(file, 0x0800); // the name in UTF-8
        put16(file, 0);      // stored
        put16(file, 0);
        put16(file, 0x21);   // 1980-01-01
        put32(file, crc);
        put32(file, quint32(p.data.size()));
        put32(file, quint32(p.data.size()));
        put16(file, quint16(name.size()));
        put16(file, 0);
        file += name;
        file += p.data;
        put32(central, 0x02014b50);
        put16(central, 20);
        put16(central, 20);
        put16(central, 0x0800);
        put16(central, 0);
        put16(central, 0);
        put16(central, 0x21);
        put32(central, crc);
        put32(central, quint32(p.data.size()));
        put32(central, quint32(p.data.size()));
        put16(central, quint16(name.size()));
        put16(central, 0);
        put16(central, 0);
        put16(central, 0);
        put16(central, 0);
        put32(central, 0);
        put32(central, offset);
        central += name;
    }
    const quint32 cdStart = quint32(file.size());
    file += central;
    put32(file, 0x06054b50);
    put16(file, 0);
    put16(file, 0);
    put16(file, quint16(parts.size()));
    put16(file, quint16(parts.size()));
    put32(file, quint32(central.size()));
    put32(file, cdStart);
    put16(file, 0);
    QFile f(out);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    return f.write(file) == file.size();
}

bool zipWithEntries(const QString &out, const QStringList &entries)
{
    QList<ZipPart> parts;
    QStringList cut;
    for (const QString &e : entries) {
        const int eq = e.indexOf(QLatin1Char('='));
        QString name = eq < 0 ? e : e.left(eq);
        const QString text = eq < 0 ? QString() : e.mid(eq + 1);
        ZipPart p;
        if (name.startsWith(QLatin1String("crc:"))) {
            name = name.mid(4);
            p.badCrc = true;
        } else if (name.startsWith(QLatin1String("cut:"))) {
            name = name.mid(4);
            cut << name + QLatin1Char('=') + text;
        }
        p.name = name;
        p.data = text.toUtf8();
        parts << p;
    }
    if (!writeStoredZip(out, parts))
        return false;
    for (const QString &c : cut) {
        const int eq = c.indexOf(QLatin1Char('='));
        const QByteArray n = c.left(eq).toUtf8();
        const int textSize = int(c.mid(eq + 1).toUtf8().size());
        QFile f(out);
        if (!f.open(QIODevice::ReadOnly))
            return false;
        QByteArray d = f.readAll();
        f.close();
        const int i = d.indexOf(n);
        if (i < 0)
            continue;
        d.truncate(i + n.size() + std::max(1, textSize / 2));
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate) || f.write(d) != d.size())
            return false;
    }
    return true;
}

namespace {

QJsonObject fileEntry(const QString &rel, const QByteArray &data, bool executable)
{
    QJsonObject e{{QStringLiteral("path"), rel},
                  {QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex())},
                  {QStringLiteral("size"), data.size()}};
    if (executable)
        e.insert(QStringLiteral("executable"), true);
    return e;
}

bool writePackage(const QString &out, const QJsonObject &manifest, const QList<ZipPart> &payload)
{
    QList<ZipPart> parts;
    parts << ZipPart{QStringLiteral("manifest.json"), QJsonDocument(manifest).toJson(QJsonDocument::Indented), false};
    parts << payload;
    return writeStoredZip(out, parts);
}

} // namespace

bool makeAddonPackage(const QString &folder, const QString &out)
{
    QFile mf(folder + QStringLiteral("/manifest.json"));
    if (!mf.open(QIODevice::ReadOnly))
        return false;
    QJsonObject m = QJsonDocument::fromJson(mf.readAll()).object();
    if (!m.contains(QStringLiteral("format")))
        m.insert(QStringLiteral("format"), 1);
    m.insert(QStringLiteral("kind"), QStringLiteral("addon"));
    QJsonArray files;
    QList<ZipPart> payload;
    bool libs = false;
    const QString root = folder + QStringLiteral("/payload");
    QStringList rels;
    QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        rels << QDir(root).relativeFilePath(it.next());
    rels.sort();
    for (const QString &rel : rels) {
        QFile f(root + QLatin1Char('/') + rel);
        if (!f.open(QIODevice::ReadOnly))
            return false;
        const QByteArray data = f.readAll();
        const bool lib = rel.endsWith(QLatin1String(".so")) || rel.contains(QLatin1String(".so."));
        libs = libs || lib;
        files.append(fileEntry(rel, data, lib));
        payload << ZipPart{QStringLiteral("payload/") + rel, data, false};
    }
    m.insert(QStringLiteral("files"), files);
    if (libs) {
        // as an add-on with libraries for this system would be built: its system and this program's runtime (a
        // system named in the folder's manifest stays: a package for another system, A-A01)
        if (!m.contains(QStringLiteral("platform")))
            m.insert(QStringLiteral("platform"), platform::packagePlatform());
        if (!m.contains(QStringLiteral("runtime")))
            m.insert(QStringLiteral("runtime"), QStringLiteral(QB_RUNTIME_ID));
    }
    return writePackage(out, m, payload);
}

bool makeCorePackage(const QByteArray &binary, const QString &binaryName, const QString &version, const QString &runtime,
                     const QString &out)
{
    QJsonObject m{{QStringLiteral("format"), 1},
                  {QStringLiteral("id"), QStringLiteral("core")},
                  {QStringLiteral("name"), QStringLiteral("Quillbinder")},
                  {QStringLiteral("version"), version},
                  {QStringLiteral("kind"), QStringLiteral("core")},
                  {QStringLiteral("runtime"), runtime},
                  {QStringLiteral("binary"), binaryName},
                  {QStringLiteral("description"), QString()},
                  {QStringLiteral("whatsNew"), QJsonArray()},
                  {QStringLiteral("files"), QJsonArray{fileEntry(binaryName, binary, true)}}};
    return writePackage(out, m, {ZipPart{QStringLiteral("payload/") + binaryName, binary, false}});
}

bool makeBundle(const QString &id, const QString &name, const QString &version, const QStringList &packages,
                const QString &out)
{
    QJsonArray names;
    QList<ZipPart> parts;
    for (const QString &p : packages) {
        QFile f(p);
        if (!f.open(QIODevice::ReadOnly))
            return false;
        names.append(QFileInfo(p).fileName());
        parts << ZipPart{QStringLiteral("packages/") + QFileInfo(p).fileName(), f.readAll(), false};
    }
    QJsonObject m{{QStringLiteral("format"), 1},
                  {QStringLiteral("id"), id},
                  {QStringLiteral("name"), name},
                  {QStringLiteral("version"), version},
                  {QStringLiteral("kind"), QStringLiteral("bundle")},
                  {QStringLiteral("description"), QString()},
                  {QStringLiteral("whatsNew"), QJsonArray()},
                  {QStringLiteral("packages"), names}};
    return writePackage(out, m, parts);
}

} // namespace cs
