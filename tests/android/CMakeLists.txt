# The system test on Android (2026-11-14).
#
# The system test (crash safety and usability) as an Android app of its own, from Android's own copy of the cases
# (tests/android/systemtest, copied from Linux's tests/crashsafe on 2026-11-16 and changed for Android since: see its
# README.md; Linux's cases are not touched by Android work), built with Qt's command for Android apps, with the
# tablet's own interface code (src/android) in it as in the program. A script on the build machine (packaging/android/android-system-test.sh)
# installs it on an emulated tablet and starts it once per case, with the case's name as the app's arguments;
# tests/android/st_android.cpp is what an app needs for that (arguments in place of the environment, the case's
# output into a file). Only for Android (the root CMakeLists.txt adds this folder inside if(ANDROID), when asked
# with -DQB_ANDROID_SYSTEM_TEST=ON, which needs -DQB_TEST_SWITCHES=ON as the Linux test builds do).
find_package(Qt6 REQUIRED COMPONENTS Test)
set(QB_ST_APP qb_system_test)
set(CS ${CMAKE_CURRENT_SOURCE_DIR}/systemtest)
# (every file of the copy: cases taken out or added change only the folder)
file(GLOB QB_ST_SOURCES ${CS}/*.cpp)
qt_add_executable(${QB_ST_APP}
  ${QB_ST_SOURCES} ${CS}/cs.h
  ${CMAKE_SOURCE_DIR}/plugins/tools/speech-transcription/SpeechLogic.cpp
  st_android.cpp st_android.h
  # the tablet's own interface code, as in the program
  ${CMAKE_SOURCE_DIR}/src/android/Mobile.h ${CMAKE_SOURCE_DIR}/src/android/MobileBars.cpp
  ${CMAKE_SOURCE_DIR}/src/android/MobileTouch.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileHandles.cpp
  ${CMAKE_SOURCE_DIR}/src/android/MobileWindows.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileFiles.cpp
  ${CMAKE_SOURCE_DIR}/src/android/MobileStart.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileOpen.cpp
  ${QB_RESOURCES_QRC} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_include_directories(${QB_ST_APP} PRIVATE ${CS} ${CMAKE_SOURCE_DIR}/plugins/tools ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${QB_ST_APP} PRIVATE qbui Qt6::Test Qt6::Network Qt6::GuiPrivate)
# The build machine's folders mean nothing on the tablet: the script names the tablet's ones when it starts a case
# (QB_CS_BUILD, QB_CS_SOURCE; systemtest/st_main.cpp).
target_compile_definitions(${QB_ST_APP} PRIVATE QB_TEST_BUILD_DIR="/qbst/build" QB_TEST_SOURCE_DIR="/qbst/source")
set_target_properties(${QB_ST_APP} PROPERTIES
  QT_ANDROID_PACKAGE_SOURCE_DIR "${QB_ANDROID_PACKAGE_DIR}"
  QT_ANDROID_PACKAGE_NAME "io.github.shelin_boop.quillbinder.systemtest"
  QT_ANDROID_APP_NAME "Quillbinder System Test"
  QT_ANDROID_APP_ICON "@mipmap/ic_launcher"
  QT_ANDROID_VERSION_NAME "${APP_VERSION_SHOWN}, system test"
  QT_ANDROID_VERSION_CODE "${QB_ANDROID_VERSION_CODE}"
  QT_ANDROID_MIN_SDK_VERSION 28
  QT_ANDROID_TARGET_SDK_VERSION 36
  QT_ANDROID_COMPILE_SDK_VERSION 36)
# the plug-ins the program's APK carries (root CMakeLists.txt, QB_APK_PLUGINS): the cases meet the same program
if(QB_APK_PLUGINS)
  add_dependencies(${QB_ST_APP} ${QB_APK_PLUGINS})
  set_property(TARGET ${QB_ST_APP} PROPERTY QT_ANDROID_EXTRA_PLUGINS "${CMAKE_BINARY_DIR}/apk-plugins")
endif()
