cmake_minimum_required (VERSION 3.22)

project (PCkeyboardIntoMidi VERSION 1.0.0)

# JUCE скачивается автоматически при настройке проекта
include (FetchContent)
FetchContent_Declare (JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.4
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable (JUCE)

juce_add_plugin (PCkeyboardIntoMidi
    PRODUCT_NAME                "PCkeyboardIntoMidi"
    COMPANY_NAME                "PCkeyboardIntoMidi"
    PLUGIN_MANUFACTURER_CODE    Pckm
    PLUGIN_CODE                 Pkim
    FORMATS                     VST3
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Instrument Tools)

target_sources (PCkeyboardIntoMidi PRIVATE
    Source/WinKeys.cpp
    Source/KeyEngine.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_features (PCkeyboardIntoMidi PRIVATE cxx_std_17)

target_compile_definitions (PCkeyboardIntoMidi PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

# русские строки в исходниках записаны в UTF-8
if (MSVC)
    target_compile_options (PCkeyboardIntoMidi PRIVATE /utf-8)
endif()

target_link_libraries (PCkeyboardIntoMidi
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if (WIN32)
    target_link_libraries (PCkeyboardIntoMidi PRIVATE winmm)
endif()
