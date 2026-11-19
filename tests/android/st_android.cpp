#include "st_android.h"

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace cs_android {

namespace {
const char kArgsFile[] = "/sdcard/Download/qbst/next-args.txt";
}

void arguments(int &argc, char **&argv)
{
    // the arguments the script left for this start (they stay alive as long as the program runs)
    static std::vector<std::string> lines;
    static std::vector<char *> pointers;
    std::ifstream file(kArgsFile);
    if (file) {
        std::string line;
        while (std::getline(file, line)) {
            while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
                line.pop_back();
            if (!line.empty())
                lines.push_back(line);
        }
    }
    pointers.push_back(argc > 0 ? argv[0] : const_cast<char *>("qb_system_test"));
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (lines[i] == "--env" && i + 1 < lines.size()) {
            const std::string &pair = lines[++i];
            const std::size_t eq = pair.find('=');
            // "NAME=" with nothing after it takes the variable away (a sub-run without QB_SKIP_FIRST_START, as the
            // Linux runner's subCase does with an empty value)
            if (eq != std::string::npos && eq > 0 && eq + 1 < pair.size())
                qputenv(pair.substr(0, eq).c_str(), QByteArray(pair.c_str() + eq + 1));
            else if (eq != std::string::npos && eq > 0)
                qunsetenv(pair.substr(0, eq).c_str());
            continue;
        }
        pointers.push_back(lines[i].data());
    }
    for (int i = 1; i < argc; ++i) // (arguments Android did hand over, if any)
        pointers.push_back(argv[i]);
    argc = int(pointers.size());
    pointers.push_back(nullptr);
    argv = pointers.data();

    const QByteArray log = qgetenv("QB_CS_LOG");
    if (!log.isEmpty()) {
        QDir().mkpath(QFileInfo(QString::fromLocal8Bit(log)).absolutePath());
        // (line by line, so that a case that ends without a word still leaves what it printed)
        if (std::freopen(log.constData(), "w", stdout))
            std::setvbuf(stdout, nullptr, _IOLBF, 0);
        if (std::freopen(log.constData(), "a", stderr))
            std::setvbuf(stderr, nullptr, _IONBF, 0);
    }
}

QString folder(const char *variable, const QString &standard)
{
    const QByteArray value = qgetenv(variable);
    return value.isEmpty() ? standard : QString::fromLocal8Bit(value);
}

} // namespace cs_android
