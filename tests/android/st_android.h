#pragma once
// The system test as an Android app (tests/android/CMakeLists.txt): what an app needs to be a test program.
#include <QString>

namespace cs_android {

// Call first in main(). An app is started by Android without arguments (Qt takes them from the start request only
// in an app built for debugging), without an environment and without a terminal, so:
//  - the arguments are the lines of the file /sdcard/Download/qbst/next-args.txt, which the script on the build
//    machine writes before each start (one argument per line);
//  - "--env" followed by "NAME=VALUE" (any number, anywhere) is set as an environment variable and taken out;
//  - what the program prints (standard output and error output) goes into the file named by QB_CS_LOG.
void arguments(int &argc, char **&argv);

// The folder of the tablet that stands in for the build folder or the repository (QB_CS_BUILD, QB_CS_SOURCE);
// `standard` when the script named none.
QString folder(const char *variable, const QString &standard);

} // namespace cs_android
