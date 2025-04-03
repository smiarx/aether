cmake_minimum_required(VERSION 3.15)

project(Aether VERSION 0.0.1)

set(COMPANY_NAME "S-audio")

find_package(JUCE CONFIG QUIET)
if(NOT ${JUCE_FOUND})
add_subdirectory(submodules/JUCE)
endif()

juce_add_plugin(${PROJECT_NAME}
    # VERSION ...
    # ICON_BIG ...
    # ICON_SMALL ...
    COMPANY_NAME "${COMPANY_NAME}"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE
    PLUGIN_MANUFACTURER_CODE saud
    PLUGIN_CODE aeth

    FORMATS VST3
    PRODUCT_NAME "${PROJECT_NAME}")

file(GLOB FACTORY_PRESETS src/Presets/factory/*)
juce_add_binary_data(${PROJECT_NAME}_data
    SOURCES
    src/GUI/assets/spring.shader
    ${FACTORY_PRESETS})


target_sources(${PROJECT_NAME}
    PRIVATE
        src/PluginProcessor.cpp
        src/GUI/PluginEditor.cpp
        src/GUI/CustomLNF.cpp
        src/GUI/DelaySection.cpp
        src/GUI/SpringsSection.cpp
        src/GUI/SpringsGL.cpp
        src/GUI/ToolTip.cpp
        src/GUI/Led.cpp
        src/GUI/PresetComponent.cpp
        src/Presets/PresetManager.cpp
        submodules/dsp/processors/springs/Springs.cpp
        submodules/dsp/processors/tapedelay/TapeDelay.cpp
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    submodules/dsp/include/
    submodules/dsp/processors/tapedelay/
    submodules/dsp/processors/springs/
)

add_subdirectory(submodules/readerwriterqueue/)


target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_options(${PROJECT_NAME} PRIVATE
       -fopenmp-simd
       -DPROJECT_NAME="${PROJECT_NAME}"
       -DCOMPANY_NAME="${COMPANY_NAME}"
       -DSPRINGS_RMS
       -DTAPEDELAY_SWITCH_INDICATOR
   )

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_data
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        readerwriterqueue
    )
