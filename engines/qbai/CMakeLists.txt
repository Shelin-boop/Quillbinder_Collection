# 0.99.0: libqbai, the C interface of the AI engine package (src/tools/QbAi.h), over the libraries of tools/get-ai-engine.sh
# (build/ai-engine, or QB_AI_ENGINE_DIR). Without them nothing is built here. The program never links it: it is put into
# the package "ai-engine" (addons/ai-engine/stage.sh) and opened with QLibrary when an AI function is used.
# In the build folder it lies in <build>/qbai with links to the pack's libraries and compute modules, as in the package
# (RUNPATH $ORIGIN only), so that the suites can load it from there (QB_AI_ENGINE_LIB=<build>/qbai/libqbai.so.1).
if(DEFINED ENV{QB_AI_ENGINE_DIR})
  set(QB_AI_ENGINE_DIR "$ENV{QB_AI_ENGINE_DIR}")
else()
  set(QB_AI_ENGINE_DIR "${CMAKE_SOURCE_DIR}/build/ai-engine")
endif()
set(D "${QB_AI_ENGINE_DIR}")
if(NOT (EXISTS "${D}/include/qwen.h" AND EXISTS "${D}/include/mtmd-helper.h" AND EXISTS "${D}/lib/libggml.so"
        AND EXISTS "${D}/lib/libsonic.so" AND EXISTS "${D}/version.txt"))
  message(STATUS "AI engine: not built (tools/get-ai-engine.sh, then configure again)")
  return()
endif()
file(READ "${D}/version.txt" QB_AI_PARTS)
string(STRIP "${QB_AI_PARTS}" QB_AI_PARTS)
string(REPLACE "\"" "'" QB_AI_PARTS_TEXT "${QB_AI_PARTS}")
# the parts' versions (with ";" between them, which a compile definition would split): a generated header
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/qbai_parts.h" "#define QBAI_PARTS \"${QB_AI_PARTS_TEXT}\"\n")
add_library(qbai SHARED qbai.cpp ${CMAKE_SOURCE_DIR}/src/tools/QbAi.h)
target_include_directories(qbai PRIVATE ${CMAKE_SOURCE_DIR}/src "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_include_directories(qbai SYSTEM PRIVATE "${D}/include")
target_link_libraries(qbai PRIVATE
  "${D}/lib/libwhisper.so" "${D}/lib/libllama.so" "${D}/lib/libmtmd.so" "${D}/lib/libqwentts.so" "${D}/lib/libsonic.so"
  "${D}/lib/libggml.so" "${D}/lib/libggml-base.so" ${CMAKE_DL_LIBS})
target_link_options(qbai PRIVATE "-Wl,-rpath-link,${D}/lib" "-Wl,--as-needed")
set_target_properties(qbai PROPERTIES
  AUTOMOC OFF AUTOUIC OFF AUTORCC OFF
  VERSION 1.0.0 SOVERSION 1
  CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qbai
  BUILD_RPATH "$ORIGIN" INSTALL_RPATH "$ORIGIN" SKIP_BUILD_RPATH OFF BUILD_WITH_INSTALL_RPATH ON)
# the pack's libraries and compute modules beside it, as in the package
add_custom_command(TARGET qbai POST_BUILD
  COMMAND sh -c "for f in \"$1\"/lib/*.so*; do ln -sf \"$f\" \"$2/\"; done" sh "${D}" "${CMAKE_BINARY_DIR}/qbai"
  VERBATIM)
message(STATUS "AI engine: libqbai with ${D} (${QB_AI_PARTS})")
