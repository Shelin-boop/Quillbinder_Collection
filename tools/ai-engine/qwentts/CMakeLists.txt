# Build file for qwentts.cpp against the pack's own ggml (tools/get-ai-engine.sh builds it as a separate project; qwentts.cpp's own CMakeLists.txt vendors a fork of ggml as a submodule and builds
# command line tools, a server and tests, none of which the program needs).
#
#   cmake -S tools/ai-engine/qwentts -B <build> -DQWENTTS_SRC=<qwentts.cpp checkout> -DQWENTTS_COMMIT=<commit> -DQWENTTS_DATE=<date>
#
# What this changes against upstream's build, and nothing else: the ggml of the pack is used (find_package), only the
# library is built (shared, the symbols marked QT_API are exported), and version.h is written from the pinned commit
# instead of from git (the build has no .git folder).
cmake_minimum_required(VERSION 3.14)
project(qwentts LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT QWENTTS_SRC)
    message(FATAL_ERROR "give -DQWENTTS_SRC=<the qwentts.cpp source folder>")
endif()

find_package(Threads REQUIRED)
find_package(ggml REQUIRED)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    "#pragma once\n#define QWEN_VERSION \"${QWENTTS_COMMIT} (${QWENTTS_DATE})\"\n")

add_library(qwentts SHARED
    "${QWENTTS_SRC}/src/qwen.cpp"
    "${QWENTTS_SRC}/src/pipeline-tts.cpp"
    "${QWENTTS_SRC}/src/pipeline-codec.cpp"
)
target_compile_definitions(qwentts PRIVATE QWEN_BUILD _FORTIFY_SOURCE=2)
target_include_directories(qwentts PRIVATE "${QWENTTS_SRC}/src" "${QWENTTS_SRC}" "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(qwentts PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    VERSION 0.0.0
    SOVERSION 0
)
target_link_libraries(qwentts PRIVATE ggml::ggml Threads::Threads)

include(GNUInstallDirs)
install(TARGETS qwentts LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES "${QWENTTS_SRC}/src/qwen.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
