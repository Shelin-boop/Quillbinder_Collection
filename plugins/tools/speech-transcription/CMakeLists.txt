# Add-on "Speech Transcription" (0.61): two libraries (see SpeechApi.h).
#  - libqb-speech.so       the writing tool the program loads (Qt Widgets, Network) -> build/tool-plugins/
#  - libqb-speech-impl.so  the engine: Qt Multimedia, recognition through the program's AI engine (1.10) -> build/tool-plugins/speech/
# Without Qt Multimedia only the tool is built (it then says that its engine is missing).
# packaging/linux/build-addons.sh puts both into the add-on with the libraries of addons/speech-transcription/stage.sh.
qb_tool_plugin(qb-speech SpeechTool.cpp SpeechTool.h SpeechLogic.cpp SpeechLogic.h SpeechApi.h)
target_link_libraries(qb-speech PRIVATE Qt6::Network ${CMAKE_DL_LIBS})

find_package(Qt6 QUIET COMPONENTS Multimedia)
# 1.10 (0.99.0): the engine has no whisper.cpp of its own any more: the recognition runs in the program's AI engine (package
# "ai-engine", tool interface 1.3). whisper.cpp's folder (tools/get-whisper.sh, build/whisper or QB_WHISPER_DIR) is still
# looked at for the test recording samples/jfk.wav only.
if(DEFINED ENV{QB_WHISPER_DIR})
  set(QB_WHISPER_DIR "$ENV{QB_WHISPER_DIR}")
else()
  set(QB_WHISPER_DIR "${CMAKE_SOURCE_DIR}/build/whisper")
endif()
if(Qt6Multimedia_FOUND)
  add_library(qb-speech-impl MODULE impl/SpeechEngine.cpp SpeechLogic.cpp SpeechLogic.h SpeechApi.h)
  target_include_directories(qb-speech-impl PRIVATE ${CMAKE_SOURCE_DIR}/src) # core/AudioBackend.h, tools/ToolApi.h, tools/QbAi.h (headers only)
  set_target_properties(qb-speech-impl PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tool-plugins/speech
    CXX_VISIBILITY_PRESET hidden)
  target_link_libraries(qb-speech-impl PRIVATE Qt6::Core Qt6::Gui Qt6::Multimedia ${CMAKE_DL_LIBS})
  # the test recording beside the engine, as in the add-on
  add_custom_command(TARGET qb-speech-impl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tool-plugins/speech/samples
    VERBATIM)
  if(EXISTS "${QB_WHISPER_DIR}/samples/jfk.wav")
    add_custom_command(TARGET qb-speech-impl POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QB_WHISPER_DIR}/samples/jfk.wav" ${CMAKE_BINARY_DIR}/tool-plugins/speech/samples/
      VERBATIM)
  endif()
  message(STATUS "Speech Transcription: engine (recognition through the program's AI engine)")
else()
  message(STATUS "Speech Transcription: no engine (Qt Multimedia is needed for building)")
endif()
