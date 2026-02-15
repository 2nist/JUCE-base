cmake_minimum_required(VERSION 3.22)

project(juce_2nist VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
include(CTest)

option(BUILD_PROJUCER_TOOL "Build JUCE Projucer utility target" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG master
)

FetchContent_MakeAvailable(JUCE)
add_subdirectory(vendor/foleys_gui_magic/modules)

if(BUILD_PROJUCER_TOOL)
  add_subdirectory(
    "${JUCE_SOURCE_DIR}/extras/Projucer"
    "${CMAKE_BINARY_DIR}/_deps/juce-build/extras/Projucer")
endif()

add_library(twonist_engine STATIC
  src/engine/MusicTheoryEngine.cpp
  src/engine/ProgressionManager.cpp
  src/engine/ArrangementOutput.cpp
)
target_include_directories(twonist_engine PUBLIC src)

juce_add_plugin(2nist
  COMPANY_NAME "TwoNistLabs"
  BUNDLE_ID "com.twonistlabs.2nist"
  IS_SYNTH FALSE
  IS_MIDI_EFFECT TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT TRUE
  PLUGIN_MANUFACTURER_CODE TnL1
  PLUGIN_CODE TnS1
  FORMATS VST3 Standalone
  PRODUCT_NAME "2nist MIDI Arranger"
)

target_sources(2nist
  PRIVATE
    src/PluginProcessor.cpp
    src/PluginProcessor.h
)

target_compile_definitions(2nist
  PUBLIC
    FOLEYS_SHOW_GUI_EDITOR_PALLETTE=1
    FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(2nist
  PRIVATE
    twonist_engine
    foleys_gui_magic
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_cryptography
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(2nist)

juce_add_console_app(juce_env_tests PRODUCT_NAME "juce_env_tests")
target_sources(juce_env_tests PRIVATE tests/SmokeTests.cpp)
target_link_libraries(juce_env_tests PRIVATE juce::juce_audio_processors juce::juce_events)

juce_add_gui_app(DesignerApp
  PRODUCT_NAME "TwoNist Designer"
  COMPANY_NAME "TwoNistLabs"
  BUNDLE_ID "com.twonistlabs.designer"
)
target_sources(DesignerApp
  PRIVATE
    apps/designer/Main.cpp
    apps/designer/MainComponent.cpp
    apps/designer/MainComponent.h
)
target_compile_definitions(DesignerApp
  PRIVATE
    FOLEYS_SHOW_GUI_EDITOR_PALLETTE=1
    FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE=1
)
target_link_libraries(DesignerApp
  PRIVATE
    foleys_gui_magic
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

add_executable(engine_parity_tests
  tests/EngineParityTests.cpp
)
target_link_libraries(engine_parity_tests PRIVATE twonist_engine)

add_executable(designer_smoke_tests
  tests/DesignerSmokeTests.cpp
  apps/designer/MainComponent.cpp
  apps/designer/MainComponent.h
)
target_compile_definitions(designer_smoke_tests
  PRIVATE
    FOLEYS_SHOW_GUI_EDITOR_PALLETTE=1
    FOLEYS_SAVE_EDITED_GUI_IN_PLUGIN_STATE=1
)
target_include_directories(designer_smoke_tests PRIVATE .)
target_link_libraries(designer_smoke_tests
  PRIVATE
    foleys_gui_magic
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_gui_extra
)

if(BUILD_TESTING)
  add_test(NAME juce_env_tests COMMAND $<TARGET_FILE:juce_env_tests>)
  add_test(NAME engine_parity_tests COMMAND $<TARGET_FILE:engine_parity_tests>)
  add_test(NAME designer_smoke_tests COMMAND $<TARGET_FILE:designer_smoke_tests>)
endif()
