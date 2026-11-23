cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# The application name lives in exactly one place. Change APP_NAME /
# APP_ID to rename the program everywhere (window titles, config folders,
# desktop file, AppImage file name).
# ---------------------------------------------------------------------------
set(APP_NAME "Quillbinder")
set(APP_ID   "quillbinder")
set(APP_VERSION "0.93.0")
# 0.93.0: a version line per system (working rules, section 8). The first two numbers (the functional level) are the same
# on every system; the last number counts each system's own fixes, so that a system is released as soon as its own
# tests pass. "APP_VERSION" is the Linux line (the AppImage, the update package; every script that reads it for
# Linux keeps reading it); the Windows and the Android builds use their own line below: the build takes it as
# APP_VERSION, so that the program, the packages and AppInfo.h see one number. Scripts of those systems read
# APP_VERSION_WINDOWS / APP_VERSION_ANDROID from this file (the Python tools take --system). Raise the last number of a system's line when that system gets a delivery of its own.
set(APP_VERSION_WINDOWS "0.93.0")
set(APP_VERSION_ANDROID "0.93.0")
if(WIN32)
  set(APP_VERSION "${APP_VERSION_WINDOWS}")
elseif(ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "Android")
  # (ANDROID itself is only known after project(); the toolchain file sets the system's name before)
  set(APP_VERSION "${APP_VERSION_ANDROID}")
endif()
# A word shown after the version number in Help > About, System Information and the
# AppImage's file name ("alpha", "beta"; empty for a normal release). It is never part
# of version comparisons: the program, the loader and the packages compare APP_VERSION only.
set(APP_VERSION_LABEL "beta")
if(APP_VERSION_LABEL)
  set(APP_VERSION_SHOWN "${APP_VERSION} ${APP_VERSION_LABEL}")
else()
  set(APP_VERSION_SHOWN "${APP_VERSION}")
endif()
# Identifies the set of libraries inside the AppImage. Program updates
# (.qbpkg "core" packages) only run on the runtime they were built for;
# change it whenever the bundled libraries change (new Qt, new modules).
set(APP_RUNTIME "u2404-qt6.12-r2")
# the first program version of that runtime: add-ons with libraries built for it need it (tools/make-package.py)
set(APP_RUNTIME_SINCE "0.93.0")
# The Qt of the Linux build (0.80.0): The Qt Company's own binaries of this version (tools/get-qt.sh puts them
# into /opt/qt/<version>/gcc_64; the release workflow installs the same with aqtinstall), no longer Ubuntu's
# Qt 6.4.2. The runtime id above names it; change both together.
set(QB_QT_VERSION "6.12.1")
# The day that Qt version was released (its tag in qt/qtbase): Help > System Information shows the runtime's age
# (the external audit of 2026-11-09, X-7).
set(APP_RUNTIME_DATE "2026-11-23")
if(WIN32)
  # Windows (port, W2; Qt 6.12.0 since W4a, 2026-11-11, the same Qt as Linux): The Qt Company's build of
  # QB_QT_VERSION for MSVC with its DLLs in the program folder; program updates for Windows must be built with the
  # same Qt (.github/workflows/windows.yml pins it). The test builds of 0.60.5 had win64-qt6.8.3-msvc-r1.
  set(APP_RUNTIME "win64-qt6.12-msvc-r1")
endif()
# The public project page (0.50): downloads, and the release "Third-party source code" with the
# source of every library the AppImage and the add-ons carry (Help > Third-Party Licenses).
set(APP_HOMEPAGE "https://github.com/Shelin-boop/Quillbinder_Collection")

project(${APP_ID} VERSION ${APP_VERSION} LANGUAGES C CXX) # C: Monocypher (src/core/third_party, 0.65.0)
# a build must carry its own system's line: stop loudly rather than build a program with another system's number
if(WIN32 AND NOT APP_VERSION STREQUAL APP_VERSION_WINDOWS)
  message(FATAL_ERROR "Windows build: APP_VERSION must be APP_VERSION_WINDOWS (${APP_VERSION_WINDOWS}), not ${APP_VERSION}")
elseif(ANDROID AND NOT APP_VERSION STREQUAL APP_VERSION_ANDROID)
  message(FATAL_ERROR "Android build: APP_VERSION must be APP_VERSION_ANDROID (${APP_VERSION_ANDROID}), not ${APP_VERSION}")
endif()

if(ANDROID)
  # Android (port, A0, 2026-11-14): The Qt Company's build of QB_QT_VERSION for Android inside the APK
  # (.github/workflows/android.yml installs it). No program updates there: a new version is a new APK; add-ons for
  # Android since 0.91.3 (packages for android-arm64 / android-x86_64).
  # (After project(): CMake knows only from there on that it builds for Android; run 3 of the workflow.)
  set(APP_RUNTIME "android-qt6.12-r1")
  # Trial builds have an id of their own. The public APK's id is permanent (the writer, 2026-11-16):
  # io.github.shelin_boop.quillbinder.app, set by .github/workflows/release-android.yml with its own key.
  set(QB_ANDROID_PACKAGE "io.github.shelin_boop.quillbinder.trial" CACHE STRING "The Android app's id (package name)")
  # The version Android shows in its app information (the public APK: "<version> experimental", 2026-11-16)
  set(QB_ANDROID_VERSION_NAME "" CACHE STRING "The version name of the APK (empty: the version and the build number)")
  # A number that grows from build to build: Android installs an APK only over one with the same or a lower
  # version code (the workflow passes its run number).
  set(QB_ANDROID_BUILD "0" CACHE STRING "Build number of the APK (0 to 999), the last part of its version code")
endif()

# Switches of the tests that load plugins from other folders or leave checks out (QB_PLUGIN_PATH,
# QB_SPEECH_SOURCE, QB_SPEECH_ACCEPT_CHANGED, QB_SPEECH_SKIP_TEST, QB_TEST_SIGNING_KEY, unsigned program
# updates): 0.70.0 (audit A S-04) off unless asked for, so that a program built as the README says behaves
# like a release (signatures required). The test runs build with -DQB_TEST_SWITCHES=ON.
# Fuzzing (0.80.0, tests/fuzz/README.md): a build of its own with clang, every part instrumented for libFuzzer.
option(QB_FUZZ "Build the libFuzzer harnesses of the readers (clang only)" OFF)
if(QB_FUZZ)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "QB_FUZZ needs clang (CC=clang CXX=clang++)")
  endif()
  add_compile_options(-fsanitize=fuzzer-no-link,address -g)
  add_link_options(-fsanitize=address)
endif()

option(QB_TEST_SWITCHES "Honor the test switches that load plugins from other folders or skip checks (test builds only)" OFF)
if(QB_TEST_SWITCHES)
  add_compile_definitions(QB_TEST_SWITCHES=1)
endif()

# The compiler cache of a Claude Code session (0.91.2, build optimization P5): tools/get-ccache.sh sets ccache up and
# leaves the file ~/.config/ccache/quillbinder; then every build of this tree compiles through ccache, also the builds
# that the packaging scripts configure themselves. Nowhere else (no such file, or a launcher given): no change.
if(NOT CMAKE_CXX_COMPILER_LAUNCHER AND NOT DEFINED ENV{CMAKE_CXX_COMPILER_LAUNCHER}
   AND DEFINED ENV{HOME} AND EXISTS "$ENV{HOME}/.config/ccache/quillbinder")
  find_program(QB_CCACHE ccache)
  if(QB_CCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER "${QB_CCACHE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${QB_CCACHE}")
    message(STATUS "Compiler cache: ${QB_CCACHE} (tools/get-ccache.sh)")
  endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# Windows with MSVC (the port, W1): the sources are UTF-8, M_PI comes from <cmath>, and
# windows.h must not define min/max. Linux builds are not affected.
if(MSVC)
  add_compile_options(/utf-8 /bigobj)
  add_compile_definitions(_USE_MATH_DEFINES NOMINMAX)
  # a symbols file (.pdb) for every release build (W2): crash reports name program offsets, the
  # .pdb turns them into function names. /Z7 keeps the debug data in the objects (sccache can cache
  # them); the linker writes the .pdb.
  add_compile_options($<$<CONFIG:Release>:/Z7>)
  add_link_options($<$<CONFIG:Release>:/DEBUG> $<$<CONFIG:Release>:/OPT:REF> $<$<CONFIG:Release>:/OPT:ICF>)
endif()

# Linux (0.80.0): build with the Qt named above. QB_QT_DIR (cache or environment) names another folder of the
# same version; tools/get-qt.sh fetches it in a cloud session.
# Not on Android (A0): there Qt comes from Qt's toolchain file (packaging/android/android-ci.sh).
if(NOT WIN32 AND NOT ANDROID)
  if(NOT QB_QT_DIR)
    if(DEFINED ENV{QB_QT_DIR})
      set(QB_QT_DIR "$ENV{QB_QT_DIR}")
    else()
      set(QB_QT_DIR "/opt/qt/${QB_QT_VERSION}/gcc_64")
    endif()
  endif()
  set(QB_QT_DIR "${QB_QT_DIR}" CACHE PATH "Qt ${QB_QT_VERSION} for the Linux build (runtime ${APP_RUNTIME})")
  list(PREPEND CMAKE_PREFIX_PATH "${QB_QT_DIR}")
endif()
# Network: local sockets between running instances (the AppImage has carried libQt6Network since 0.2.0).
find_package(Qt6 REQUIRED COMPONENTS Core Gui Xml Widgets Svg PrintSupport Pdf PdfWidgets Network)
# Page view sets the page margins of a text layout through a private Qt class
# (src/text/PageLayout.cpp) and FontFaces.cpp: the private headers of the same Qt.
set(QT_NO_PRIVATE_MODULE_WARNING ON) # (Qt 6.10 and later warn about private modules; used knowingly)
find_package(Qt6 QUIET COMPONENTS GuiPrivate)
if(NOT TARGET Qt6::GuiPrivate)
  message(FATAL_ERROR "The private Qt GUI headers are missing (Qt ${QB_QT_VERSION}: tools/get-qt.sh).")
endif()
if(NOT Qt6_VERSION VERSION_EQUAL QB_QT_VERSION)
  if(WIN32)
    message(FATAL_ERROR "Qt ${Qt6_VERSION} found in ${Qt6_DIR}; the runtime ${APP_RUNTIME} needs Qt ${QB_QT_VERSION} "
                        "(the Windows workflow installs it, .github/workflows/windows.yml)")
  endif()
  message(FATAL_ERROR "Qt ${Qt6_VERSION} found in ${Qt6_DIR}; the runtime ${APP_RUNTIME} needs Qt ${QB_QT_VERSION} "
                      "(tools/get-qt.sh, or -DQB_QT_DIR=<folder of Qt ${QB_QT_VERSION}>)")
endif()
find_package(ZLIB REQUIRED)
find_package(PkgConfig)
# Not on Android (A0): pkg-config there would find the build machine's own Hunspell; the Android build names
# its Hunspell with QB_HUNSPELL_INCLUDE and QB_HUNSPELL_LIB (packaging/android/android-hunspell.sh).
if(PkgConfig_FOUND AND NOT ANDROID)
  pkg_check_modules(HUNSPELL IMPORTED_TARGET hunspell)
endif()
if(HUNSPELL_FOUND)
  set(QB_HUNSPELL_TARGET PkgConfig::HUNSPELL)
else()
  # Without pkg-config (Windows, Hunspell from vcpkg): look for the library itself.
  find_path(QB_HUNSPELL_INCLUDE hunspell/hunspell.h)
  find_library(QB_HUNSPELL_LIB NAMES hunspell-1.7 libhunspell-1.7 hunspell)
  if(QB_HUNSPELL_INCLUDE AND QB_HUNSPELL_LIB)
    add_library(qb_hunspell INTERFACE)
    target_include_directories(qb_hunspell INTERFACE ${QB_HUNSPELL_INCLUDE})
    target_link_libraries(qb_hunspell INTERFACE ${QB_HUNSPELL_LIB})
    # a static Hunspell from vcpkg also needs libiconv
    find_library(QB_ICONV_LIB NAMES iconv libiconv)
    find_library(QB_CHARSET_LIB NAMES charset libcharset)
    foreach(lib QB_ICONV_LIB QB_CHARSET_LIB)
      if(${lib})
        target_link_libraries(qb_hunspell INTERFACE ${${lib}})
      endif()
    endforeach()
    set(QB_HUNSPELL_TARGET qb_hunspell)
  endif()
endif()

configure_file(src/app/AppInfo.h.in ${CMAKE_BINARY_DIR}/generated/AppInfo.h @ONLY)

# Core library: project model, text formats, compile, import (no widgets).
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
  src/core/*.cpp src/core/*.h src/text/*.cpp src/text/*.h
  src/compile/*.cpp src/compile/*.h src/import/*.cpp src/import/*.h)
add_library(qbcore STATIC ${CORE_SOURCES}
  # package signatures (0.65.0): Monocypher's Ed25519 check, unchanged third-party C code
  src/core/third_party/monocypher/monocypher.c src/core/third_party/monocypher/monocypher-ed25519.c)
target_include_directories(qbcore PUBLIC src ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(qbcore PUBLIC Qt6::Core Qt6::Gui Qt6::Xml ZLIB::ZLIB)
target_link_libraries(qbcore PRIVATE Qt6::GuiPrivate)

# User interface library (shared by the program and the UI tests).
file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS src/ui/*.cpp src/ui/*.h)
add_library(qbui STATIC ${UI_SOURCES})
target_link_libraries(qbui PUBLIC qbcore
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::PrintSupport Qt6::Pdf Qt6::PdfWidgets Qt6::Network)

# Windows (W3): the same resources with the license list of the Windows program (tools/gen-resources.py)
if(WIN32)
  set(QB_RESOURCES_QRC ${CMAKE_SOURCE_DIR}/resources/resources-windows.qrc)
else()
  set(QB_RESOURCES_QRC ${CMAKE_SOURCE_DIR}/resources/resources.qrc)
endif()
if(ANDROID)
  # Android (2026-11-14): the same resources with the license list of what the APK ships (resources/licenses-android,
  # packaging/android/collect-licenses-android.py); the resource file is written into the build folder.
  execute_process(COMMAND python3 ${CMAKE_SOURCE_DIR}/packaging/android/android-resources.py
                          ${CMAKE_SOURCE_DIR}/resources/resources.qrc ${CMAKE_SOURCE_DIR}/resources/licenses-android
                          ${CMAKE_BINARY_DIR}/resources-android.qrc
                  RESULT_VARIABLE QB_ANDROID_RESOURCES_RC)
  if(NOT QB_ANDROID_RESOURCES_RC EQUAL 0)
    message(FATAL_ERROR "packaging/android/android-resources.py failed")
  endif()
  set(QB_RESOURCES_QRC ${CMAKE_BINARY_DIR}/resources-android.qrc)
endif()
# 2026-11-15 (build optimization P6): the resources (about 1,800 files) are compiled once, into an object library that
# the program and every test program take their objects from, instead of one copy per test program (54 copies of an
# 11 MB generated file, about 5 s each). Built only for a target that needs it (the Android program keeps its own).
add_library(qb_resources OBJECT EXCLUDE_FROM_ALL ${QB_RESOURCES_QRC})
target_link_libraries(qb_resources PRIVATE Qt6::Core)
set(QB_RESOURCES_OBJECTS $<TARGET_OBJECTS:qb_resources>)
# 0.70.0: the German texts (translations/README.md)
set(QB_TRANSLATIONS_QRC ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
if(ANDROID)
  # Android (A0, 2026-11-14): Qt's own command makes the program a library that the app's Java part loads, and
  # adds the targets that pack the APK (androiddeployqt; "cmake --build <dir> --target quillbinder_make_apk").
  # The package's own files (manifest, icons) are in packaging/android/package.
  if(COMMAND qt_policy)
    qt_policy(SET QTP0002 NEW) # paths in the APK's settings as they are written (Qt warns when this is not set)
  endif()
  qt_add_executable(${APP_ID} src/main.cpp ${QB_RESOURCES_QRC} ${QB_TRANSLATIONS_QRC})
  # Android's own interface code (2026-11-14): toolbar rows fitted to the screen's width, scrolling by finger, zoom
  # with two fingers. Only this program contains it (src/android is in none of the libraries).
  target_sources(${APP_ID} PRIVATE src/android/Mobile.h src/android/MobileBars.cpp src/android/MobileTouch.cpp
    src/android/MobileHandles.cpp src/android/MobileWindows.cpp src/android/MobileFiles.cpp src/android/MobileStart.cpp
    src/android/MobileOpen.cpp src/android/MobileCare.cpp)
  # MobileWindows.cpp (new windows while the keyboard program calls; a fault of Qt 6.12.0) reaches the window's
  # drawing engine and the input context through Qt's private headers.
  target_link_libraries(${APP_ID} PRIVATE Qt6::GuiPrivate)
  string(REPLACE "." ";" QB_VERSION_PARTS "${APP_VERSION}")
  list(GET QB_VERSION_PARTS 0 QB_V_MAJOR)
  list(GET QB_VERSION_PARTS 1 QB_V_MINOR)
  list(GET QB_VERSION_PARTS 2 QB_V_PATCH)
  # 0.91.1, build 7 -> 9101007 (Android's limit is 2,100,000,000)
  math(EXPR QB_ANDROID_VERSION_CODE
       "${QB_V_MAJOR} * 100000000 + ${QB_V_MINOR} * 100000 + ${QB_V_PATCH} * 1000 + ${QB_ANDROID_BUILD}")
  # The package's own files are copied into the build folder, where the build adds the APK's assets to them
  # (QB_ANDROID_ASSETS: a folder whose content goes into the APK as it is; the workflow puts the spelling
  # dictionaries there, which the program copies into its data folder at the start, android/MobileStart.cpp).
  set(QB_ANDROID_ASSETS "" CACHE PATH "A folder whose content becomes the APK's assets (the dictionaries)")
  set(QB_ANDROID_PACKAGE_DIR "${CMAKE_BINARY_DIR}/android-package")
  file(REMOVE_RECURSE "${QB_ANDROID_PACKAGE_DIR}")
  file(COPY "${CMAKE_SOURCE_DIR}/packaging/android/package/" DESTINATION "${QB_ANDROID_PACKAGE_DIR}")
  if(QB_ANDROID_ASSETS AND IS_DIRECTORY "${QB_ANDROID_ASSETS}")
    file(COPY "${QB_ANDROID_ASSETS}/" DESTINATION "${QB_ANDROID_PACKAGE_DIR}/assets")
  endif()
  if(QB_ANDROID_VERSION_NAME)
    set(QB_ANDROID_VERSION_SHOWN "${QB_ANDROID_VERSION_NAME}")
  else()
    set(QB_ANDROID_VERSION_SHOWN "${APP_VERSION_SHOWN}, build ${QB_ANDROID_BUILD}")
  endif()
  set_target_properties(${APP_ID} PROPERTIES
    QT_ANDROID_PACKAGE_SOURCE_DIR "${QB_ANDROID_PACKAGE_DIR}"
    QT_ANDROID_PACKAGE_NAME "${QB_ANDROID_PACKAGE}"
    QT_ANDROID_APP_NAME "${APP_NAME}"
    QT_ANDROID_APP_ICON "@mipmap/ic_launcher"
    QT_ANDROID_VERSION_NAME "${QB_ANDROID_VERSION_SHOWN}"
    QT_ANDROID_VERSION_CODE "${QB_ANDROID_VERSION_CODE}"
    QT_ANDROID_MIN_SDK_VERSION 28      # Android 9, the oldest Qt ${QB_QT_VERSION} supports
    QT_ANDROID_TARGET_SDK_VERSION 36   # Android 16, the newest Qt ${QB_QT_VERSION} supports
    QT_ANDROID_COMPILE_SDK_VERSION 36) # built against the same SDK platform, not the newest one on the machine
else()
  add_executable(${APP_ID} src/main.cpp ${QB_RESOURCES_OBJECTS} ${QB_TRANSLATIONS_QRC})
endif()
target_link_libraries(${APP_ID} PRIVATE qbui)
if(WIN32)
  set_target_properties(${APP_ID} PROPERTIES WIN32_EXECUTABLE ON) # no console window
  # W4b: the program icon for Explorer, shortcuts and the Start menu; the Start menu entry, the desktop shortcut and the
  # project file types (src/ui/WinShell.cpp) use Windows' shell (COM) and the registry
  target_sources(${APP_ID} PRIVATE ${CMAKE_SOURCE_DIR}/resources/app/quillbinder.rc)
  target_link_libraries(qbui PRIVATE ole32 uuid shell32)
endif()
if(QB_HUNSPELL_TARGET)
  target_link_libraries(qbcore PUBLIC ${QB_HUNSPELL_TARGET})
  target_compile_definitions(qbcore PUBLIC QB_HAVE_HUNSPELL=1)
endif()
# Plugins must see the exporter interface symbols of the executable.
set_target_properties(${APP_ID} PROPERTIES ENABLE_EXPORTS ON)

if(NOT ANDROID) # (A0: on Android the program is a library inside the APK; nothing is installed from here)
  install(TARGETS ${APP_ID} RUNTIME DESTINATION bin)
endif()
install(FILES packaging/linux/${APP_ID}.desktop DESTINATION share/applications)
install(FILES resources/app/${APP_ID}.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES packaging/linux/${APP_ID}-mime.xml DESTINATION share/mime/packages)
install(FILES packaging/linux/${APP_ID}.metainfo.xml DESTINATION share/metainfo RENAME io.github.shelin_boop.Quillbinder.metainfo.xml)
install(DIRECTORY plugins/command/ DESTINATION share/${APP_ID}/plugins FILES_MATCHING PATTERN "*.json")
# Make the command plugins available when running from the build folder.
file(COPY plugins/command/ DESTINATION ${CMAKE_BINARY_DIR}/plugins FILES_MATCHING PATTERN "*.json")

# Example C++ exporter plugin (Markdown). Shows how third parties add formats.
option(QB_BUILD_EXAMPLE_PLUGIN "Build the example exporter plugin (Markdown)" ON)
if(QB_BUILD_EXAMPLE_PLUGIN)
  add_subdirectory(plugins/markdown-exporter)
endif()

# Writing tools (tools/ToolApi.h). They are delivered as add-ons, not with the
# program update: the libraries go to build/tool-plugins, from where
# packaging/linux/build-addons.sh puts them into the add-ons (addons/tool-*).
# Run from the build folder with QB_PLUGIN_PATH=build/tool-plugins QB_TOOL_DATA=addons/tool-name-generator/payload
option(QB_BUILD_TOOL_PLUGINS "Build the writing tools" ON)
if(QB_BUILD_TOOL_PLUGINS)
  add_subdirectory(plugins/tools)
endif()

# Importers (src/import/ImporterApi.h): .doc files and web pages, add-ons as well (addons/import-*).
option(QB_BUILD_IMPORTER_PLUGINS "Build the importer add-ons' plugins" ON)
if(QB_BUILD_IMPORTER_PLUGINS)
  add_subdirectory(plugins/importers)
endif()

# Media playback (src/media/MediaApi.h): an add-on as well (addons/media-playback).
# Needs Qt Multimedia at build time (Ubuntu: qt6-multimedia-dev); the program itself does not.
find_package(Qt6 QUIET COMPONENTS Multimedia)
option(QB_BUILD_MEDIA_PLUGIN "Build the media playback add-on's libraries" ${Qt6Multimedia_FOUND})
if(QB_BUILD_MEDIA_PLUGIN)
  add_subdirectory(plugins/media)
endif()

if(ANDROID)
  # Android (2026-11-14): on Linux the writing tools, the importers and the Markdown exporter come as add-on
  # packages or in the program's folder. An APK is one full package: they are built into one folder, and Qt's
  # androiddeployqt puts that folder's libraries beside the program's inside the APK (QT_ANDROID_EXTRA_PLUGINS),
  # where compile/PluginManager.cpp finds them by their names.
  set(QB_APK_PLUGINS "")
  foreach(plugin markdown-exporter qb-name-generator qb-scratch-pad qb-text-statistics qb-import-word97 qb-import-web)
    if(TARGET ${plugin})
      set_target_properties(${plugin} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/apk-plugins")
      list(APPEND QB_APK_PLUGINS ${plugin})
    endif()
  endforeach()
  if(QB_APK_PLUGINS)
    add_dependencies(${APP_ID} ${QB_APK_PLUGINS})
    set_property(TARGET ${APP_ID} PROPERTY QT_ANDROID_EXTRA_PLUGINS "${CMAKE_BINARY_DIR}/apk-plugins")
    message(STATUS "Android: plug-ins inside the APK: ${QB_APK_PLUGINS}")
  endif()
  # https (File > Import > Web Page): Qt's TLS plug-in for Android loads OpenSSL's two libraries from the APK
  # (packaging/android/android-openssl.sh builds them; libcrypto first: libssl needs it).
  set(QB_ANDROID_OPENSSL "" CACHE PATH "The folder of OpenSSL for this ABI (lib/libcrypto_3.so, lib/libssl_3.so)")
  if(QB_ANDROID_OPENSSL AND EXISTS "${QB_ANDROID_OPENSSL}/lib/libssl_3.so" AND EXISTS "${QB_ANDROID_OPENSSL}/lib/libcrypto_3.so")
    set_property(TARGET ${APP_ID} PROPERTY QT_ANDROID_EXTRA_LIBS
      "${QB_ANDROID_OPENSSL}/lib/libcrypto_3.so" "${QB_ANDROID_OPENSSL}/lib/libssl_3.so")
    message(STATUS "Android: OpenSSL inside the APK: ${QB_ANDROID_OPENSSL}")
  endif()
  target_link_libraries(${APP_ID} PRIVATE Qt6::Network) # (android/MobileStart.cpp tells in the log whether https works)
endif()

# Tests (headless round-trip checks)
option(QB_BUILD_TESTS "Build tests" ON)
if(QB_BUILD_TESTS AND NOT QB_TEST_SWITCHES)
  message(STATUS "The test suites need the test switches: configure with -DQB_TEST_SWITCHES=ON to run them")
endif()
if(QB_BUILD_TESTS)
  if(WIN32)
    # Windows (W3): only the suites packaging/windows/win-tests.sh names are built (cmake --build build --target ...)
    add_subdirectory(tests EXCLUDE_FROM_ALL)
  else()
    add_subdirectory(tests)
  endif()
endif()
if(QB_FUZZ)
  add_subdirectory(tests/fuzz)
endif()
if(ANDROID AND QB_ANDROID_SYSTEM_TEST)
  # Android (2026-11-14): the system test as an app for the emulated tablet (tests/android; asked for by the
  # system test's workflow with -DQB_ANDROID_SYSTEM_TEST=ON, together with the test switches)
  add_subdirectory(tests/android)
endif()

# Developer tools (not installed)
option(QB_BUILD_TOOLS "Build developer tools" ON)
if(QB_BUILD_TOOLS)
  add_executable(qb_template tools/qb-template.cpp)
  target_link_libraries(qb_template PRIVATE qbcore)
endif()
