cmake_minimum_required(VERSION 3.22)

# If built standalone without parent project:
if(NOT TARGET juce::juce_audio_basics)
    project(TheVoid VERSION 1.0.0 LANGUAGES C CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(TheVoid
    COMPANY_NAME "THE EDGE OF FEAR"
    BUNDLE_ID com.theedgeoffear.thevoid
    PLUGIN_MANUFACTURER_CODE TEOF
    PLUGIN_CODE TVod
    FORMATS VST3 Standalone
    PRODUCT_NAME "THE VOID"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(TheVoid
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/VoidLookAndFeel.h
        Source/VoidPresets.h
        Source/PresetManager.h
        Source/MidiManager.h
        Source/MidiMappingModal.h
        Source/DSP/NoiseGate.h
        Source/DSP/CompressorEngine.h
        Source/DSP/PitchEngine.h
        Source/DSP/PitchLfoEngine.h
        Source/DSP/DistortionEngine.h
        Source/DSP/HaasWidthEngine.h
        Source/DSP/ReverbEngine.h
        Source/DSP/VoidDSPChain.h
)

juce_add_binary_data(Void_Assets
    SOURCES
        "BACKGROUND IMAGE/THEVOIDBG.jpg"
)

target_link_libraries(TheVoid
    PRIVATE
        Void_Assets
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(TheVoid
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_ASIO=1
)

target_include_directories(TheVoid
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/asiosdk/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/asiosdk/asiosdk_2.3.3_2019-06-14/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/../asiosdk/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/../asiosdk/asiosdk_2.3.3_2019-06-14/common"
)

if(MSVC)
    target_compile_options(TheVoid PUBLIC /utf-8)
endif()

