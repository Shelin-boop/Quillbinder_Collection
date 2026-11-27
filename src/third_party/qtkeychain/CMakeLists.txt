# QtKeychain 0.18.0 (README.txt): Quillbinder's own build file for the vendored sources, a static library built with
# the program's Qt. It replaces QtKeychain's CMake files (install rules, translations, shared library), which are left
# out. Backends: Linux KWallet over D-Bus (kwalletd6, kwalletd5) and the Secret Service through libsecret, which is
# loaded at run time (only its headers are needed for building, from libsecret-1-dev); Windows Credential Manager;
# Android Keystore. The program links no library of its own for any of them.
set(QK qtkeychain)
set(QK_SOURCES ${QK}/keychain.cpp ${QK}/keychain.h ${QK}/keychain_p.h)
set(QK_DEFINES QTKEYCHAIN_NO_EXPORT)
set(QK_LIBS Qt6::Core)
if(WIN32)
  list(APPEND QK_SOURCES ${QK}/keychain_win.cpp)
  list(APPEND QK_DEFINES USE_CREDENTIAL_STORE=1 UNICODE)
  list(APPEND QK_LIBS crypt32 advapi32)
elseif(ANDROID)
  list(APPEND QK_SOURCES ${QK}/keychain_android.cpp ${QK}/androidkeystore.cpp ${QK}/androidkeystore_p.h
       ${QK}/plaintextstore.cpp ${QK}/plaintextstore_p.h)
elseif(UNIX AND NOT APPLE)
  find_package(Qt6 REQUIRED COMPONENTS DBus)
  list(APPEND QK_SOURCES ${QK}/keychain_unix.cpp ${QK}/gnomekeyring.cpp ${QK}/gnomekeyring_p.h ${QK}/libsecret.cpp
       ${QK}/libsecret_p.h ${QK}/plaintextstore.cpp ${QK}/plaintextstore_p.h)
  list(APPEND QK_DEFINES KEYCHAIN_DBUS=1)
  qt6_add_dbus_interface(QK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${QK}/org.kde.KWallet.xml kwallet_interface)
  list(APPEND QK_LIBS Qt6::DBus)
  # the Secret Service (GNOME, and every desktop whose key store speaks it): headers only, the library is loaded at run
  # time (libsecret.cpp). Without the headers the program still builds, with KWallet as its only Linux key store.
  option(QB_KEYCHAIN_LIBSECRET "QtKeychain: the Secret Service backend (needs libsecret-1-dev for building)" ON)
  set(QK_SECRET_INCLUDES "")
  if(QB_KEYCHAIN_LIBSECRET)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
      pkg_check_modules(QB_LIBSECRET libsecret-1>=0.18.0)
    endif()
    if(QB_LIBSECRET_FOUND)
      list(APPEND QK_DEFINES HAVE_LIBSECRET=1)
      set(QK_SECRET_INCLUDES ${QB_LIBSECRET_INCLUDE_DIRS})
    else()
      message(WARNING "libsecret-1's headers are missing (Ubuntu: libsecret-1-dev, tools/cloud-setup.sh): the key store "
                      "has no Secret Service backend (GNOME); configure with -DQB_KEYCHAIN_LIBSECRET=OFF to build so on purpose")
    endif()
  endif()
endif()

add_library(qb_qtkeychain STATIC ${QK_SOURCES})
set_target_properties(qb_qtkeychain PROPERTIES AUTOMOC ON POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(qb_qtkeychain PUBLIC QTKEYCHAIN_NO_EXPORT PRIVATE ${QK_DEFINES})
# <qtkeychain/keychain.h> for its users; the generated D-Bus interface from the build folder
target_include_directories(qb_qtkeychain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${QK} ${CMAKE_CURRENT_BINARY_DIR})
if(QK_SECRET_INCLUDES)
  target_include_directories(qb_qtkeychain SYSTEM PRIVATE ${QK_SECRET_INCLUDES})
endif()
target_link_libraries(qb_qtkeychain PUBLIC ${QK_LIBS})
if(NOT MSVC)
  target_compile_options(qb_qtkeychain PRIVATE -w) # (third-party code, unchanged but for README.txt's note: no warnings of ours)
endif()
