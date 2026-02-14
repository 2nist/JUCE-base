cmake_minimum_required(VERSION 3.22)

project(juce_2nist VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
include(CTest)

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
    src/PluginEditor.cpp
    src/PluginEditor.h
    src/PluginProcessor.cpp
    src/PluginProcessor.h
)

target_compile_definitions(2nist
  PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(2nist
  PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(2nist)

juce_add_console_app(juce_env_tests PRODUCT_NAME "juce_env_tests")
target_sources(juce_env_tests PRIVATE tests/SmokeTests.cpp)
target_link_libraries(juce_env_tests PRIVATE juce::juce_audio_processors juce::juce_events)

if(BUILD_TESTING)
  add_test(NAME juce_env_tests COMMAND $<TARGET_FILE:juce_env_tests>)
endif()
