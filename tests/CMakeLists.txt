add_executable(qb_tests core_tests.cpp script_checks.cpp scrivener_checks.cpp compile_checks.cpp)
target_link_libraries(qb_tests PRIVATE qbcore Qt6::Widgets)

# Scripted UI run with screenshots (QT_QPA_PLATFORM=offscreen ./qb_ui_tests <dir>)
find_package(Qt6 REQUIRED COMPONENTS Test)
add_executable(qb_ui_tests ui_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_ui_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_ui_tests PRIVATE QB_TEST_PLUGIN_DIR="${CMAKE_BINARY_DIR}/plugins")

# Page view and copyholders (QT_QPA_PLATFORM=offscreen ./qb_m6_tests <dir>)
add_executable(qb_m6_tests m6_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m6_tests PRIVATE qbui Qt6::Test)

# Drag and drop, navigation, Quick Reference panels (QT_QPA_PLATFORM=offscreen ./qb_m7_tests <dir>;
# under X11 also real mouse drags: xvfb-run -a -s "-screen 0 1920x1080x24" ./qb_m7_tests <dir>)
add_executable(qb_m7_tests m7_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m7_tests PRIVATE qbui Qt6::Test)

# Search and replace, filters, find by formatting (QT_QPA_PLATFORM=offscreen ./qb_m8_tests <dir>)
add_executable(qb_m8_tests m8_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m8_tests PRIVATE qbui Qt6::Test)

# Scriptwriting: script mode, elements, formats, compile (QT_QPA_PLATFORM=offscreen ./qb_m9_tests <dir>)
add_executable(qb_m9_tests m9_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m9_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_m9_tests PRIVATE QB_TEST_PLUGIN_DIR="${CMAKE_BINARY_DIR}/plugins")

# Writing tools: name generator, scratch pad, text statistics (QT_QPA_PLATFORM=offscreen ./qb_m10_tests <dir>).
# Installs the add-on packages from <build>/addons-dist (packaging/linux/build-addons.sh <build> <build>/addons-dist)
# and also compiles the counting parts of two tools for direct checks.
find_package(Qt6 REQUIRED COMPONENTS Network)
add_executable(qb_m10_tests m10_tests.cpp ${QB_RESOURCES_OBJECTS}
  ${CMAKE_SOURCE_DIR}/plugins/tools/text-statistics/TextAnalysis.cpp
  ${CMAKE_SOURCE_DIR}/plugins/tools/name-generator/NameLists.cpp)
target_include_directories(qb_m10_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins/tools)
target_link_libraries(qb_m10_tests PRIVATE qbui Qt6::Test Qt6::Network)
target_compile_definitions(qb_m10_tests PRIVATE QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# Audio and video (QT_QPA_PLATFORM=offscreen ./qb_m11_tests /tmp/shots). Installs <build>/addons-dist/media-playback.qbpkg
# (packaging/linux/build-addons.sh <build> <build>/addons-dist, after tools/build-ffmpeg.sh <ffmpeg source> <build>/ffmpeg).
# Needs a sound server for the audio checks (a null sink is enough: pulseaudio -D --exit-idle-time=-1, PULSE_SERVER=...).
add_executable(qb_m11_tests m11_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m11_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_m11_tests PRIVATE QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}" QB_TEST_MEDIA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/media")

# Import and export (QT_QPA_PLATFORM=offscreen ./qb_m12_tests /tmp/shots): installs <build>/addons-dist/import-word97.qbpkg
# and import-web.qbpkg (packaging/linux/build-addons.sh <build> <build>/addons-dist); serves the test pages itself (HTTP and HTTPS).
add_executable(qb_m12_tests m12_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m12_tests PRIVATE qbui Qt6::Test Qt6::Network)
target_compile_definitions(qb_m12_tests PRIVATE QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}" QB_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/import")

# Scrivener project format, compile part 2 (format editor, PDF, e-book, metadata, preview) and a menu audit
# (QT_QPA_PLATFORM=offscreen ./qb_m13_tests /tmp/shots); installs the add-ons in <build>/addons-dist when they were built
add_executable(qb_m13_tests m13_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m13_tests PRIVATE qbui Qt6::Test Qt6::PdfWidgets)
target_compile_definitions(qb_m13_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# Corrections of 0.14.1: fonts at once and project-wide, label color in synopsis and notes, project bookmarks,
# document icons, search results marked in the editor (QT_QPA_PLATFORM=offscreen ./qb_fixes_tests /tmp/shots)
add_executable(qb_fixes_tests fixes_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_fixes_tests PRIVATE qbui Qt6::Test)

# Printing (milestone M14): File > Print Settings, Print Preview, Print Current Document for texts, index cards,
# the outliner, pictures, PDF files and web pages; the printouts go into PDF files checked with poppler-utils
# (QT_QPA_PLATFORM=offscreen ./qb_m14_tests /tmp/shots)
add_executable(qb_m14_tests m14_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_m14_tests PRIVATE qbui Qt6::Test Qt6::PrintSupport)

# Version 0.17: binder, fonts, color buttons, icons, icon bar, folder comments, mind maps, developer guide
# (QT_QPA_PLATFORM=offscreen ./qb_v017_tests /tmp/shots)
add_executable(qb_v017_tests v017_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v017_tests PRIVATE qbui Qt6::Test)


# Version 0.18: the mind map help and File > Compare and Transfer
# (QT_QPA_PLATFORM=offscreen ./qb_v018_tests /tmp/shots)
add_executable(qb_v018_tests v018_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v018_tests PRIVATE qbui Qt6::Test)

# Version 0.19: the Page Down / Page Up crash, crash reports with a snapshot of program and system,
# sessions that ended unexpectedly, saving on SIGTERM, Help > System Information and Crash Reports.
# Starts <build>/quillbinder and ends it with signals (QT_QPA_PLATFORM=offscreen ./qb_v019_tests /tmp/shots)
add_executable(qb_v019_tests v019_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v019_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v019_tests PRIVATE QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(qb_v019_tests ${APP_ID})

# 0.20: the Objects sheet and the data-safety fixes (QT_QPA_PLATFORM=offscreen ./qb_v020_tests /tmp/shots)
add_executable(qb_v020_tests v020_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v020_tests PRIVATE qbui Qt6::Test)

# 0.21: Options in Scrivener's layout, zoom, mind map menus, Compile Overview (QT_QPA_PLATFORM=offscreen ./qb_v021_tests /tmp/shots)
add_executable(qb_v021_tests v021_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v021_tests PRIVATE qbui Qt6::Test)

# 0.21.1: format bar target, editable bookmark preview, a zoom per part, mind map backgrounds, places kept,
# formatted object texts, credits (QT_QPA_PLATFORM=offscreen ./qb_v0211_tests /tmp/shots)
add_executable(qb_v0211_tests v0211_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0211_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0211_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# 0.21.2: Scrivener headings and project bookmarks, the new standard settings (QT_QPA_PLATFORM=offscreen ./qb_v0212_tests /tmp/shots)
add_executable(qb_v0212_tests v0212_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0212_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0212_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Version 0.40: the welcome project (first start, Help > Welcome Project, its mind map, objects and links), the Quick
# Start Guide, Help > Third-Party Licenses and About (Apache License 2.0, credits), "Combined Text" instead of the old name, no
# tutorial left, the menu places the guide and the welcome project name, List of All Placeholders, and two
# corrections carried over from the suite of the old tutorial (QT_QPA_PLATFORM=offscreen ./qb_v040_tests /tmp/shots).
add_executable(qb_v040_tests v040_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v040_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v040_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# Version 0.40.1: fonts with several designs in one family (optical sizes such as Sitka's; tested with the free
# Source Serif 4 of tests/fonts) get a family per design; the Bold / Italic / Underline / Strikethrough buttons show
# the whole selection, also one made from right to left (QT_QPA_PLATFORM=offscreen ./qb_v0401_tests /tmp/shots).
add_executable(qb_v0401_tests v0401_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0401_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0401_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Version 0.40.2 (private line): no jump on a right-click, a look per project, the border beside the text, settings
# in the right-click menus, project settings saved to a file and read back, style changes reaching all text of the
# style (QT_QPA_PLATFORM=offscreen ./qb_v0402_tests /tmp/shots).
add_executable(qb_v0402_tests v0402_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0402_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0402_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Version 0.40.3 (private line): titles from the first line, No Style redefined, the short spelling menu, a format
# bar that shows what is seen, dialogs above the Styles panel, the alignment of the main text, the chosen style list
# (xvfb-run -a ./qb_v0403_tests /tmp/shots).
add_executable(qb_v0403_tests v0403_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0403_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0403_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# Version 0.60.1: Paste with Formatting, Show Invisibles (menu, toolbar button, drawing), the total of a folder in
# the footer and the inspector, paragraph styles over several paragraphs (xvfb-run -a ./qb_v0601_tests /tmp/shots).
add_executable(qb_v0601_tests v0601_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0601_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0601_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Version 0.60.2: the three ways to paste in the right-click menu of the text (xvfb-run -a ./qb_v0602_tests /tmp/shots).
add_executable(qb_v0602_tests v0602_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0602_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0602_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Version 0.60.3: folder totals in the binder and the footers, Inspector Background, the color of the toolbars
# (xvfb-run -a ./qb_v0603_tests /tmp/shots).
add_executable(qb_v0603_tests v0603_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0603_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0603_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Version 0.60.4: the paragraph buttons of the format bar, the README's Wayland line (xvfb-run -a ./qb_v0604_tests /tmp/shots).
add_executable(qb_v0604_tests v0604_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0604_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0604_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# 0.50: the release on the project page, the source files named in the license list, the media add-ons split into free
# formats and "Extra Video and Audio Formats" (QT_QPA_PLATFORM=offscreen ./qb_v050_tests /tmp/shots). Installs
# <build>/addons-dist/media-playback.qbpkg and media-extra-formats.qbpkg (tools/get-ffmpeg.sh, then
# packaging/linux/build-addons.sh <build> <build>/addons-dist); needs a sound server like qb_m11_tests.
add_executable(qb_v050_tests v050_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v050_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v050_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.60: one source and one version for everyone: About names FFmpeg, the character and place sheets,
# the media package with all formats (media-all-formats.qbpkg), the release text and README, the downloads sorted by
# system (QT_QPA_PLATFORM=offscreen ./qb_v060_tests /tmp/shots).
add_executable(qb_v060_tests v060_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v060_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v060_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# Version 0.60.5: speed with a large project in Scrivener format, the right-click menu (xvfb-run -a ./qb_v0605_tests /tmp/shots).
add_executable(qb_v0605_tests v0605_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0605_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0605_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Version 0.61: tool interface 1.1, Undo and Redo on the toolbar, the add-on "Speech Transcription"
# (xvfb-run -a ./qb_v061_tests /tmp/shots). Installs <build>/addons-dist/speech-transcription.qbpkg
# (packaging/linux/build-addons.sh, after tools/get-whisper.sh); a tool built for interface 1.0 (tool10/) is
# loaded from <build>/tests/tool10-plugins.
add_library(qb-test-old-tool MODULE tool10/OldTool.cpp tool10/ToolApi10.h)
target_link_libraries(qb-test-old-tool PRIVATE Qt6::Core)
set_target_properties(qb-test-old-tool PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/tool10-plugins)
add_executable(qb_v061_tests v061_tests.cpp ${QB_RESOURCES_OBJECTS}
  ${CMAKE_SOURCE_DIR}/plugins/tools/speech-transcription/SpeechLogic.cpp)
target_include_directories(qb_v061_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins/tools)
target_link_libraries(qb_v061_tests PRIVATE qbui Qt6::Test Qt6::Network)
target_compile_definitions(qb_v061_tests PRIVATE QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}" QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_dependencies(qb_v061_tests qb-test-old-tool)

# Version 0.61.1: quote marks converted from File > Options > Corrections, Project > Delete Old Snapshots
# (also in Scrivener format) (xvfb-run -a ./qb_v0611_tests /tmp/shots).
add_executable(qb_v0611_tests v0611_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0611_tests PRIVATE qbui Qt6::Test)

# Version 0.62: settings per project, the first start, tables and pictures (xvfb-run -a ./qb_v062_tests /tmp/shots).
add_executable(qb_v062_tests v062_tests.cpp v062_settings.cpp v062_tables.cpp v062_common.h ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v062_tests PRIVATE qbui Qt6::Test)
# Version 0.64: the spelling menu on words with apostrophes, the autosave while typing on, the card
# size, a project in a synced folder (lock, conflicts, nothing written without a change, texts that
# have not arrived), also in Scrivener format (xvfb-run -a ./qb_v064_tests /tmp/shots).
add_executable(qb_v064_tests v064_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v064_tests PRIVATE qbui Qt6::Test)
# Version 0.64.1: the fixes of the system test's first run: table cells in Markdown storage, Split at
# Selection and tables, texts changed elsewhere (not written over; kept before ours replaces them),
# figures cut and pasted, compiling with the event loop running, cells not justified, Merge with notes,
# Merge Cells / Undo / Redo / Delete Rows (xvfb-run -a ./qb_v0641_tests /tmp/shots).
add_executable(qb_v0641_tests v0641_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0641_tests PRIVATE qbui Qt6::Test)

# Version 0.64.2: the fixes of the system test's full run of 0.64.1 (2026-11-10): linked notes over several formats,
# stale menu entries, word counts, pictures' formats, German quotes, Markdown paragraphs, project and .scriv folders
# that are gone, a .scriv opened and closed unchanged, outside changes while open (xvfb-run -a ./qb_v0642_tests /tmp/shots).
add_executable(qb_v0642_tests v0642_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0642_tests PRIVATE qbui Qt6::Test)
# The fixes of the external audit of 2026-11-10 (0.65.0): safe copies, the save journal, pictures, partial reads,
# Roll Back, incomplete backups, Duplicate, object pictures, Scrivener format on two computers, command
# exporters, the lock, same size and time, the storage format, large archives, bundles, signatures of packages,
# the scratch pad, the end by a signal, the close question (xvfb-run -a ./qb_v0650_tests /tmp/shots; needs
# the add-on packages in <build>/addons-dist, python3 and openssl).
add_executable(qb_v0650_tests v0650_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0650_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0650_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# The fixes of the two external audits of 2026-11-10 and the writer's decisions (0.70.0 beta): text read and
# written exactly, failed saves, backups, pictures in Merge and Split, questions, settings, hostile files, build
# and release, the German interface, the dark page (xvfb-run -a ./qb_v0700_tests /tmp/shots; needs python3,
# openssl and, for the Word 97 check, plugins/importers/qb_doc_dump).
add_executable(qb_v0700_tests v0700_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0700_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0700_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# The fixes of the audit of 0.80.0 (0.81.0 beta, 2026-11-11, part 1): backups and restore of any size, after a
# title change, onto a full disk (child runs with qbfaultshim), linked folders, snapshot titles, saves nobody looked
# at, Empty Trash, spare copies, readable explanations, the failed-save notice, First Start, the missed backup of a
# session ended by a signal, the release gate (xvfb-run -a ./qb_v0810_tests /tmp/shots; QB_SKIP_BIG_BACKUP=1 leaves
# out the 2.3 GB backup; tools/code-check.sh runs when cppcheck is installed)
add_executable(qb_v0810_tests v0810_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0810_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0810_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}"
                           QB_FAULT_SHIM="$<TARGET_FILE:qbfaultshim>")
add_dependencies(qb_v0810_tests qbfaultshim)

# 0.81.1 beta: the fixes of the audit of 0.81.0 (part 1, the four findings at function level): documents whose file cannot
# be read and the commands that change them, pictures in notes, save journals that cannot be read or are another
# computer's, the RTF writer with the auditor's file (xvfb-run -a ./qb_v0811_tests /tmp/shots)
add_executable(qb_v0811_tests v0811_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0811_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0811_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.81.2 beta: keywords, custom metadata and collections in Scrivener format (read and written in Scrivener 3 for Windows'
# form), inline notes kept as text; QB_SCRIV_REAL=<folder.scriv> adds checks on a copy of a real project
# (xvfb-run -a ./qb_v0812_tests /tmp/shots)
add_executable(qb_v0812_tests v0812_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0812_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0812_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.81.3 beta: Scrivener compatibility, part 2: styles (marks, content.styles, styles.xml), inline notes in Scrivener for
# Windows' form, web pages as .mht (the archive, the view and its two buttons), notes of pictures and media, the repair
# of what earlier versions wrote for audio, video and web pages (xvfb-run -a ./qb_v0813_tests /tmp/shots)
add_executable(qb_v0813_tests v0813_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0813_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0813_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.90.0 beta: what a Scrivener project keeps when this program writes it: every text really written anew and added
# up by a reader of the test's own (bold and italic by switch or face name, colors, fonts, indents, lists, pictures,
# links), the project file after a change (bookmarks to outside addresses, default label and status, the deadline,
# the names of "No Label" under the German interface); QB_SCRIV_REAL=<folder.scriv> does the same with a copy of a
# real project (xvfb-run -a ./qb_v0900_tests /tmp/shots)
add_executable(qb_v0900_tests v0900_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0900_tests PRIVATE qbui Qt6::Test)

# 0.90.1: menus filled again each time they open keep no copies of their old submenus (heaptrack, the system test's
# writing day) (xvfb-run -a ./qb_v0901_tests /tmp/shots)
add_executable(qb_v0901_tests v0901_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0901_tests PRIVATE qbui Qt6::Test)
# 0.90.2: the writer's list (no P/A circles, the invisibles' color, Page View in Main Editor, the theme list of the
# main toolbar, the other downloads on the download page) (xvfb-run -a ./qb_v0902_tests /tmp/shots)
add_executable(qb_v0902_tests v0902_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0902_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0902_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# 0.90.3: backups packed outside the backup folder and put there as one finished file, a backup that waits for its
# folder, the clean-up only for the program's own backups (what a watcher of the folder sees: inotify; a full drive:
# child runs with qbfaultshim) (xvfb-run -a ./qb_v0903_tests /tmp/shots)
add_executable(qb_v0903_tests v0903_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0903_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0903_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
if(NOT WIN32) # the fault library is an LD_PRELOAD library: the full-drive checks run on Linux only (0.90.4)
  target_compile_definitions(qb_v0903_tests PRIVATE QB_FAULT_SHIM="$<TARGET_FILE:qbfaultshim>")
  add_dependencies(qb_v0903_tests qbfaultshim)
endif()
# 0.90.4: the Windows requirements drawn from Linux 0.90.3 (the switch that tells AltGr from Ctrl+Alt, the portable
# program's folder for one run and the web page copy in it, saved pages in older character sets, the download page and
# the release text with the setup programs); on Linux with QB_TEST_PORTABLE (xvfb-run -a ./qb_v0904_tests /tmp/shots)
add_executable(qb_v0904_tests v0904_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0904_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0904_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# 0.91.0: backups straight in the backup folder (what waited for a project's own folder, titles with special
# characters), the seven toolbars and the icon bar (places, the grip, switches, Reset Toolbar Places, Customize Toolbars)
# (xvfb-run -a ./qb_v0910_tests /tmp/shots)
add_executable(qb_v0910_tests v0910_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0910_tests PRIVATE qbui Qt6::Test)

# 0.91.1: toolbars rearranged freely by their grips (order, rows, edges), the row that got stuck
# (xvfb-run -a ./qb_v0911_tests /tmp/shots)
add_executable(qb_v0911_tests v0911_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0911_tests PRIVATE qbui Qt6::Test)

# 0.91.3: add-on keys vouched for by the release key (make-package.py certify, sign, verify; the install window)
# (xvfb-run -a ./qb_v0913_tests /tmp/shots)
add_executable(qb_v0913_tests v0913_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0913_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0913_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# 0.92.0: files whose time alone changed, the project ID in backup names, copies that fail at an import, the kind of
# a failed save, the states of add-ons; the fixed mind map, the shown lock, the check for updates
# (xvfb-run -a ./qb_v0920_tests /tmp/shots)
add_executable(qb_v0920_tests v0920_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0920_tests PRIVATE qbui Qt6::Test Qt6::Network)
target_compile_definitions(qb_v0920_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# 0.93.0: the emergency copy (whole or told incomplete, also for the binder alone), a folder's sync that reports a disk
# error (child runs with qbfaultshim), an unreadable journal that cannot be set aside, the backup note of a signal's end,
# backups that cannot be deleted, the wording of a restore (xvfb-run -a ./qb_v0930_tests /tmp/shots)
add_executable(qb_v0930_tests v0930_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0930_tests PRIVATE qbui Qt6::Test)
if(NOT WIN32) # the fault library is an LD_PRELOAD library: the child runs are on Linux only (as v0903)
  target_compile_definitions(qb_v0930_tests PRIVATE QB_FAULT_SHIM="$<TARGET_FILE:qbfaultshim>")
  add_dependencies(qb_v0930_tests qbfaultshim)
endif()

# 0.93.0, second suite: fixes in shared code (exports that keep the old file, four limits for prepared input, the main
# window's undo watcher at its end) (xvfb-run -a ./qb_v0930b_tests /tmp/shots)
add_executable(qb_v0930b_tests v0930b_tests.cpp ${QB_RESOURCES_OBJECTS}
               ${CMAKE_SOURCE_DIR}/plugins/importers/web/WebImporter.cpp ${CMAKE_SOURCE_DIR}/plugins/importers/web/Article.cpp
               ${CMAKE_SOURCE_DIR}/plugins/importers/web/Html.cpp)
target_include_directories(qb_v0930b_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins/importers/web)
target_link_libraries(qb_v0930b_tests PRIVATE qbui Qt6::Test Qt6::Network ${CMAKE_DL_LIBS})

# 0.93.0, third suite: to-do item 38 (backups and saving: waiting backups in the restore list, the temporary names that a crash
# can leave, the command that moves the backups of 0.70.0 to 0.90.4) (xvfb-run -a ./qb_v0930c_tests /tmp/shots)
add_executable(qb_v0930c_tests v0930c_tests.cpp ${QB_RESOURCES_OBJECTS}
               ${CMAKE_SOURCE_DIR}/plugins/tools/speech-transcription/SpeechLogic.cpp)
target_include_directories(qb_v0930c_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins)
target_link_libraries(qb_v0930c_tests PRIVATE qbui Qt6::Test)

# 0.93.0, fourth suite: to-do item 22 (the panes placed by the window's shape: the six orders when wide, the two rows when
# upright, following the shape, kept per device) (xvfb-run -a ./qb_v0930d_tests /tmp/shots)
add_executable(qb_v0930d_tests v0930d_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0930d_tests PRIVATE qbui Qt6::Test)

# 0.93.0, fifth suite: items 11 (the save text in the first editor's footer bar), 17 (the label in the documents), 18 (the design
# decisions), 30 (where to report a problem, the README's picture) (xvfb-run -a ./qb_v0930e_tests /tmp/shots;
# QB_README_SHOT=<file.png> makes the README picture of the welcome project)
add_executable(qb_v0930e_tests v0930e_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0930e_tests PRIVATE qbui Qt6::Test)

# 0.93.0, to-do item 25: File > Save As (the file type chooses the format) and the copies in File > Export
# (xvfb-run -a ./qb_saveas_tests /tmp/shots)
add_executable(qb_saveas_tests saveas_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_saveas_tests PRIVATE qbui Qt6::Test)
# 0.93.0, fourth suite: to-do item 39 (documents that are not shown are unloaded: a bounded number stays laid out)
# (xvfb-run -a ./qb_v0930f_tests /tmp/shots; QB_MEASURE=1 prints the memory of a large project)
add_executable(qb_v0930f_tests v0930f_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0930f_tests PRIVATE qbui Qt6::Test)
# 0.93.1: the generic font names on a platform that does not know them (item 35, S-K01), the bounded plain texts
# (item 45), the usable area and the input devices in System Information (item 34) (xvfb-run -a ./qb_v0931_tests /tmp/shots)
add_executable(qb_v0931_tests v0931_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0931_tests PRIVATE qbui Qt6::Test)
# 0.94.0: a window is not deleted under its own menu (item 63), the focus of a new mind map note (item 64), the corkboard
# after the Trash (item 88), the inspector after a choice in the binder (item 32), session words (item 73), the backup
# note (item 85), the targets window and the window's place (item 89) (xvfb-run -a ./qb_v0940a_tests /tmp/shots)
add_executable(qb_v0940a_tests v0940a_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0940a_tests PRIVATE qbui Qt6::Test)
# 0.94.0: one place per topic in the menus and Help > Find Command (items 68, 86), the first start's one page and the entry in
# the application menu (items 60, 69, 70), the full AppImage's installation window (item 71)
# (xvfb-run -a ./qb_v0940b_tests /tmp/shots)
add_executable(qb_v0940b_tests v0940b_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0940b_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0940b_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.94.0: one name and one key per command, the check over all menus (item 87), the binder's keys (item 84), the shortcuts
# that follow the keyboard layout (item 79) (xvfb-run -a ./qb_v0940c_tests /tmp/shots)
add_executable(qb_v0940c_tests v0940c_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0940c_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0940c_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.94.0: the message line and tool interface 1.2 (items 72, 85), the notice for a missing add-on (59), the writing progress in the
# footer (74), Import and Split at headings (75), Documents > Group (76), the Objects form (77), Sheet to Object (67)
# (xvfb-run -a ./qb_v0940e_tests /tmp/shots)
add_executable(qb_v0940e_tests v0940e_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0940e_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0900_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.95.0: user profiles, the built-in theme and profile of the writer's look, the standard colors and the set of the keyboard
# layout (items 95, 96, 98, 103, 104; no window) (xvfb-run -a ./qb_v0950c_tests /tmp/shots)
add_executable(qb_v0950c_tests v0950c_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0950c_tests PRIVATE qbui Qt6::Test)
# 0.95.0: profiles in the main window, the Profiles and Labels toolbars, the Standard Colors page, the keyboard button
# (items 95, 97, 98, 103) (xvfb-run -a ./qb_v0950d_tests /tmp/shots)
add_executable(qb_v0950d_tests v0950d_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0950d_tests PRIVATE qbui Qt6::Test)
# The readers against damaged input (0.80.0, the external audit of 2026-11-09, ST-5): fixed-seed damage of small
# valid inputs of RTF, Markdown, XML, ZIP, DOCX, ODT, Word 97 and .scrivx, each read in this process
# (xvfb-run -a ./qb_fuzz_tests /tmp/shots; QB_FUZZ_SEED, QB_FUZZ_ROUNDS). tests/fuzz/README.md: longer runs.
add_executable(qb_fuzz_tests fuzz_tests.cpp ${CMAKE_SOURCE_DIR}/plugins/importers/word97/Cfb.cpp
               ${CMAKE_SOURCE_DIR}/plugins/importers/word97/WordReader.cpp)
target_link_libraries(qb_fuzz_tests PRIVATE qbui Qt6::Test)
target_include_directories(qb_fuzz_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins/importers/word97)
target_compile_definitions(qb_fuzz_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# The fixes of the external audit of 2026-11-09 (0.64.1): ids and archive names that are no plain
# names, damaged archives and Word files, settings a project may hold, packages with code, links that
# lead out, the socket folder, Scrivener's lock before every write, the name of this computer
# (xvfb-run -a ./qb_audit1_tests /tmp/shots; QB_RELEASE_BINARY=<files> searches a release build for
# test switches).
add_executable(qb_audit1_tests audit1_tests.cpp ${CMAKE_SOURCE_DIR}/plugins/importers/word97/Cfb.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_audit1_tests PRIVATE qbui Qt6::Test)
target_include_directories(qb_audit1_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins/importers/word97)
target_compile_definitions(qb_audit1_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
                           QB_FAULT_SHIM="$<TARGET_FILE:qbfaultshim>")
# write() / fsync() failing below one folder (LD_PRELOAD, for qb_audit1_tests' child runs)
add_library(qbfaultshim SHARED faultshim.cpp)
target_link_libraries(qbfaultshim PRIVATE dl)
add_dependencies(qb_audit1_tests qbfaultshim)
# Windows port, step W2: what it changed in the code Linux runs too (packages' platform, the process
# check, pruning reports with memory dumps). No window (xvfb-run -a ./qb_w2_tests /tmp/shots).
add_executable(qb_w2_tests w2_tests.cpp)
target_link_libraries(qb_w2_tests PRIVATE qbcore)
target_compile_definitions(qb_w2_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# Windows port, step W3 (Linux and the Windows runner): bundles with a program update of the running
# version, the full version's add-ons, the license list in the program, the spelling dictionaries
# (QB_TEST_DICTIONARIES=<folder>). No window.
add_executable(qb_w3_tests w3_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_w3_tests PRIVATE qbui)
target_compile_definitions(qb_w3_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# Windows port, step W4b (Linux with QB_TEST_PORTABLE, and the Windows runner): the portable program, the guide of the
# first start, This Computer in Help and Options, no standard backup folder, the shell items (xvfb-run -a ./qb_w4_tests).
add_executable(qb_w4_tests w4_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_w4_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_w4_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# Windows port, step W4c (Linux and the Windows runner): the add-on "Speech Transcription" installed and run, also from
# folders named with letters outside ASCII; a real model with QB_SPEECH_MODEL_FILE (xvfb-run -a ./qb_w4c_tests).
add_executable(qb_w4c_tests w4c_tests.cpp)
target_link_libraries(qb_w4c_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_w4c_tests PRIVATE QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# The system test (crash safety and usability): only on the writer's request, started by tools/run-system-test.sh
# (all cases) or tools/run-crashsafe.sh (some); not in tools/run-all-tests.sh or tests/check-counts.txt.
# Specification: the project document claude/quillbinder-test-procedure-crashsafe.md. Linux only (0.90.0: in main).
# 0.99.0: the Windows run builds it too, with -DQB_SYSTEM_TEST_WINDOWS=ON (crashsafe/cs_plat.cpp holds what differs).
option(QB_SYSTEM_TEST_WINDOWS "Build the system test's program on Windows" OFF)
if(NOT WIN32 OR QB_SYSTEM_TEST_WINDOWS)
add_executable(qb_crashsafe_tests crashsafe_tests.cpp crashsafe/cs.h crashsafe/cs_plat.h crashsafe/cs_plat.cpp crashsafe/cs_common.cpp crashsafe/cs_tables.cpp
               crashsafe/cs_cases.cpp crashsafe/cs_process.cpp crashsafe/cs_sync.cpp
               crashsafe/cs_list.cpp crashsafe/cs_component.cpp crashsafe/cs_component2.cpp crashsafe/cs_component3.cpp
               crashsafe/cs_sys_common.cpp crashsafe/cs_sys_projects.cpp crashsafe/cs_sys_writing.cpp
               crashsafe/cs_sys_writing2.cpp crashsafe/cs_sys_views.cpp crashsafe/cs_sys_more.cpp
               crashsafe/cs_sys_rest.cpp crashsafe/cs_usability.cpp
               crashsafe/cs_faults.cpp crashsafe/cs_roundtrip.cpp crashsafe/cs_audit.cpp crashsafe/cs_new090.cpp crashsafe/cs_enc099.cpp crashsafe/cs_new099.cpp crashsafe/cs_new099b.cpp
               ${CMAKE_SOURCE_DIR}/plugins/tools/speech-transcription/SpeechLogic.cpp ${QB_RESOURCES_OBJECTS}
               ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_include_directories(qb_crashsafe_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins/tools)
target_link_libraries(qb_crashsafe_tests PRIVATE qbui Qt6::Test Qt6::Network)
# line numbers in the crash reports of the system test (addr2line); the program itself is unchanged
if(NOT MSVC)
  target_compile_options(qb_crashsafe_tests PRIVATE -g1)
endif()
target_compile_definitions(qb_crashsafe_tests PRIVATE QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}" QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Fault injection for the system test (LD_PRELOAD; section 3.2 of the procedure): Linux only.
if(UNIX AND NOT APPLE)
  add_library(qbfault MODULE crashsafe/faultinject.cpp)
  target_compile_options(qbfault PRIVATE -U_FORTIFY_SOURCE -fno-exceptions)
  target_link_libraries(qbfault PRIVATE dl pthread)
  set_target_properties(qbfault PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests PREFIX "lib")
  add_dependencies(qb_crashsafe_tests qbfault)
endif()
endif()

# Android's interface code (src/android: toolbar rows fitted to the width, scrolling by finger, zoom with two fingers),
# checked on a desktop in windows of a tablet's size, with simulated fingers. Android's own switch: built only when
# cmake is called with -DQB_MOBILE_PREVIEW=ON (no option of that name is declared, so it is off everywhere else); part
# of no build and no test run of Linux or Windows (xvfb-run -a ./qb_mobile_tests /tmp/shots).
if(QB_MOBILE_PREVIEW)
  add_executable(qb_mobile_tests mobile_tests.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileBars.cpp
                 ${CMAKE_SOURCE_DIR}/src/android/MobileTouch.cpp
                 ${CMAKE_SOURCE_DIR}/src/android/MobileHandles.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileFiles.cpp
                 ${CMAKE_SOURCE_DIR}/src/android/MobileStart.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileCare.cpp
                 ${QB_RESOURCES_OBJECTS})
  target_link_libraries(qb_mobile_tests PRIVATE qbui Qt6::Test)
  # (Android 0.93.0: the check of the update file's Android block runs tools/update-feed.py from the source)
  target_compile_definitions(qb_mobile_tests PRIVATE QB_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  # Android 0.92.0: every menu and every window a command opens, at a tablet's size, fits and can be closed; Back, the
  # keys Ctrl and Shift with the mouse, the line between panes, the pin and the padlock on a tablet
  # (packaging/android/run-tablet-fit.sh <build dir> runs it at every size)
  add_executable(qb_mobile_fit_tests mobile_fit_tests.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileBars.cpp
                 ${CMAKE_SOURCE_DIR}/src/android/MobileTouch.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileHandles.cpp
                 ${CMAKE_SOURCE_DIR}/src/android/MobileCare.cpp ${CMAKE_SOURCE_DIR}/src/android/MobileStart.cpp
                 ${CMAKE_SOURCE_DIR}/src/android/MobileFiles.cpp ${QB_RESOURCES_OBJECTS})
  target_link_libraries(qb_mobile_fit_tests PRIVATE qbui Qt6::Test)
  target_compile_definitions(qb_mobile_fit_tests PRIVATE QB_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  # Android's copy of the system test (tests/android/systemtest), built for the desktop to try its cases here before
  # they run on emulated tablets (what needs Android itself is inside Q_OS_ANDROID there)
  file(GLOB QB_ST_ANDROID_SOURCES ${CMAKE_SOURCE_DIR}/tests/android/systemtest/*.cpp)
  add_executable(qb_android_st_preview ${QB_ST_ANDROID_SOURCES} ${CMAKE_SOURCE_DIR}/tests/android/systemtest/cs.h
                 ${CMAKE_SOURCE_DIR}/plugins/tools/speech-transcription/SpeechLogic.cpp ${QB_RESOURCES_OBJECTS}
                 ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
  target_include_directories(qb_android_st_preview PRIVATE ${CMAKE_SOURCE_DIR}/tests/android/systemtest
                             ${CMAKE_SOURCE_DIR}/plugins/tools)
  target_link_libraries(qb_android_st_preview PRIVATE qbui Qt6::Test Qt6::Network)
  target_compile_definitions(qb_android_st_preview PRIVATE QB_ST_PREVIEW QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}"
                             QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
endif()

# 0.93.0: the overview of a mind map board (automatic / always, the rectangle, click and drag, size, the bar's button
# and "Fixed:" text, the large window, not in exports) (xvfb-run -a ./qb_mmoverview_tests /tmp/shots)
add_executable(qb_mmoverview_tests mmoverview_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_mmoverview_tests PRIVATE qbui Qt6::Test)

# 0.93.0 (item 24): the notes start as plain text, pasting into them is plain (xvfb-run -a ./qb_notesplain_tests /tmp/shots)
add_executable(qb_notesplain_tests notesplain_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_notesplain_tests PRIVATE qbui Qt6::Test)
# 0.94.0, fourth part: the composition bar, card text and ruler contrast (items 65, 81), the theme button (item 80), the
# manual's window (item 66), the author's name (item 82), the add-on window (item 90), the download page (item 83), the
# system test's harness (item 61) (xvfb-run -a ./qb_v0940d_tests /tmp/shots)
add_executable(qb_v0940d_tests v0940d_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0940d_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0940d_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.95.0, first part: the sizes of the Bookmarks tab's three parts (item 94) (xvfb-run -a ./qb_v0950a_tests /tmp/shots)
add_executable(qb_v0950a_tests v0950a_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0950a_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0950a_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.95.0, second part: the move of the AppImage, highlights drawn as picked, negative word targets (xvfb-run -a ./qb_v0950b_tests /tmp/shots)
add_executable(qb_v0950b_tests v0950b_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0950b_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0950b_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.95.0, item 102 (part 1): the language list and its marks, the first start's language, the interface texts of every language, the manuals as files, the Choice values, plural forms, the drop-down lists (xvfb-run -a ./qb_v0950f_tests /tmp/shots)
add_executable(qb_v0950f_tests v0950f_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0950f_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0950f_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.95.0, item 102 (part 2): every language started in a program of its own (F1, the welcome project, the menu places the texts name, the mnemonics), the dictionary packages, the speech tool's lists, the keyboard sets of the new layouts (xvfb-run -a ./qb_v0950g_tests /tmp/shots)
add_executable(qb_v0950g_tests v0950g_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc
  ${CMAKE_SOURCE_DIR}/plugins/tools/speech-transcription/SpeechLogic.cpp)
target_include_directories(qb_v0950g_tests PRIVATE ${CMAKE_SOURCE_DIR}/plugins/tools)
target_link_libraries(qb_v0950g_tests PRIVATE qbui Qt6::Test Qt6::Network)
target_compile_definitions(qb_v0950g_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.99.0, item 116: one size (Small, Normal, Large) for all toolbars and the font of their boxes: the choices in View > Toolbars, a bar's right-click menu and Customize Toolbars, a row of one height, kept, a profile switch, Reset (xvfb-run -a ./qb_v0990c_tests /tmp/shots)
add_executable(qb_v0990c_tests v0990c_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0990c_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990c_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.99.0: tool interface 1.3, the one rule for the AI models (memory, idle time, the system's low-memory signal), the message
# line "Model change in progress", File > Options > General > AI Models (items F3, F4, 115) (xvfb-run -a ./qb_v0990b_tests /tmp/shots).
# A tool of 1.3 (tool13/) and one of 1.2 (tool12/, with the header of 1.2) are loaded from <build>/tests/tool13-plugins and tool12-plugins.
add_library(qb-test-tool13 MODULE tool13/ModelTool.cpp)
target_link_libraries(qb-test-tool13 PRIVATE Qt6::Core)
target_include_directories(qb-test-tool13 PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_target_properties(qb-test-tool13 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/tool13-plugins AUTOMOC ON)
add_library(qb-test-tool12 MODULE tool12/Tool12.cpp tool12/ToolApi12.h)
target_link_libraries(qb-test-tool12 PRIVATE Qt6::Core)
set_target_properties(qb-test-tool12 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/tool12-plugins)
add_executable(qb_v0990b_tests v0990b_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990b_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990b_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(qb_v0990b_tests qb-test-tool13 qb-test-tool12)
# 0.99.0 (the menu walk, UR-06, UR-15, item 118): the names of commands and the shape of the menus (xvfb-run -a ./qb_v0990g_tests /tmp/shots)
add_executable(qb_v0990g_tests v0990g_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990g_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990g_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.99.0, item 118 (part d): the keys that leave a table, the line around the part with the keyboard, Compile can be cancelled, the link contrast of every built-in theme (xvfb-run -a ./qb_v0990d_tests /tmp/shots)
add_executable(qb_v0990d_tests v0990d_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0990d_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990d_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")

# 0.99.0: the AI engine packages: the C interface (tools/QbAi.h), core/AiEngine.*, the device choice, ModelMemory's registration,
# cancelling, the catalog (core/AiCatalog.*), installing the engine and models (ui/AiDownloads.*, ui/AiInstallPanel.*), tool
# interface 1.3's engine functions, the package recipes (xvfb-run -a ./qb_v0990h_tests /tmp/shots). A fake libqbai
# (fakeqbai/) is built as <build>/tests/fake-ai-engine/libqbai.so.1; the real smoke test uses <build>/qbai/libqbai.so.1.
add_library(qb-fake-qbai SHARED fakeqbai/FakeQbAi.cpp)
target_include_directories(qb-fake-qbai PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_target_properties(qb-fake-qbai PROPERTIES OUTPUT_NAME qbai VERSION 1.0.0 SOVERSION 1 CXX_VISIBILITY_PRESET hidden
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/fake-ai-engine)
add_executable(qb_v0990h_tests v0990h_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990h_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990h_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(qb_v0990h_tests qb-fake-qbai)
if(TARGET qbai)
  add_dependencies(qb_v0990h_tests qbai)
endif()

# 0.99.0: Speech Transcription 1.10 (the recognition through the program's AI engine, the device list with the cards, the shared AI settings,
# Insert at Cursor, Write into the text as I speak, keepRunning, the minApp in the update file) with the fake engine of v0990h; needs the add-on's
# two libraries in <build>/tool-plugins (xvfb-run -a ./qb_v0990m_tests /tmp/shots)
add_executable(qb_v0990m_tests v0990m_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990m_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990m_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(qb_v0990m_tests qb-fake-qbai)
if(TARGET qb-speech-impl)
  add_dependencies(qb_v0990m_tests qb-speech qb-speech-impl)
endif()
# 0.99.0 (item 122): the small interface: a few round buttons, the binder and the notes as slide-in panels, the rule for Android, record only, profiles (xvfb-run -a -s "-screen 0 1920x1080x24" ./qb_v0990l_tests /tmp/shots)
add_executable(qb_v0990l_tests v0990l_tests.cpp ${QB_RESOURCES_OBJECTS} ${CMAKE_SOURCE_DIR}/translations/translations.qrc)
target_link_libraries(qb_v0990l_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990l_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.99.0 (item 112, handwriting part 2: the recognizer and the word layer): the model's input, the codec, greedy CTC, the word layer
# on frames made here, the install part, and with the real model and word lists the accuracy with and without the word layer
# (xvfb-run -a ./qb_v0990o_tests /tmp/shots)
add_executable(qb_v0990o_tests v0990o_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990o_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990o_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.99.0 (item 112, handwriting part 1: everything but the recognizer): the button with its four looks, the modes and the one writing
# area, pen and finger, hand protection, scratching out, the ink's fade, the strip with Insert and Pin, the grid, the Options page
# (xvfb-run -a ./qb_v0990j_tests /tmp/shots)
add_executable(qb_v0990j_tests v0990j_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990j_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990j_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.99.0, item 113 (part 1): the OpenPGP layer of the built-in encryption: keys, the key ring, round trips, damaged files,
# foreign keys, the passphrase generator and check, the key store, what the program links, GnuPG both ways (each gpg on the
# PATH and those QB_TEST_GPG_EXTRA names) (xvfb-run -a ./qb_v0990e_tests /tmp/shots). Exports its symbols like the program,
# so that the check of what is exported means the same.
add_executable(qb_v0990e_tests v0990e_tests.cpp)
target_link_libraries(qb_v0990e_tests PRIVATE qbcore)
set_target_properties(qb_v0990e_tests PROPERTIES ENABLE_EXPORTS ON)

# 0.99.0, item 113 (part 3): the key wizard (Create Keys), the key folder with its README.txt, the refused places, the check,
# the second copy, Import Keys, Import a Key File (gpg's v4 Curve25519 key taken, RSA refused), Export Keys, File > Options >
# Encryption with its switches and "Forget the passphrase" (xvfb-run -a ./qb_v0990k_tests /tmp/shots)
add_executable(qb_v0990k_tests v0990k_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990k_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990k_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
# 0.99.0 (item 113, part 2): the encrypted project and the encrypted backups: the vault, the walk over an encrypted
# project, conversions (also cut off), keys locked or missing, damaged files, backups of both formats, GnuPG, the
# window's unlock dialog, the speed of the encrypted autosave (xvfb-run -a ./qb_v0990f_tests /tmp/shots)
add_executable(qb_v0990f_tests v0990f_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990f_tests PRIVATE qbui Qt6::Test)

# 0.99.0 (item 114): Read Aloud: the toolbar and its Voice menu, the first click, reading (marked text, from the cursor, the marked
# sentence), voices and styles, the settings page, the memory rule, cancel, nothing written to disk; with the fake libqbai of
# v0990h (xvfb-run -a ./qb_v0990n_tests /tmp/shots)
add_executable(qb_v0990n_tests v0990n_tests.cpp ${QB_RESOURCES_OBJECTS})
target_link_libraries(qb_v0990n_tests PRIVATE qbui Qt6::Test)
target_compile_definitions(qb_v0990n_tests PRIVATE QB_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}" QB_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_dependencies(qb_v0990n_tests qb-fake-qbai)
