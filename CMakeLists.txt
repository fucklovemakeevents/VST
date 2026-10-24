cmake_minimum_required(VERSION 3.22)
project(FLMMVoxClean VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.8 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(FLMMVoxClean
    COMPANY_NAME "Fuck Love Make Music Label"
    BUNDLE_ID "com.fucklovemakemusic.voxclean"
    PLUGIN_MANUFACTURER_CODE FLMM
    PLUGIN_CODE VxCl
    FORMATS VST3 Standalone
    PRODUCT_NAME "Fuck Love Make Music Vox Clean"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(FLMMVoxClean)
target_sources(FLMMVoxClean PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)
target_compile_features(FLMMVoxClean PRIVATE cxx_std_17)
target_compile_definitions(FLMMVoxClean PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(FLMMVoxClean PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
