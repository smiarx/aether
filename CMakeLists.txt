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

set(DSP_SPRINGS_RMS ON CACHE INTERNAL "")
set(DSP_TAPEDELAY_SWITCH_INDICATOR ON CACHE INTERNAL "")

add_subdirectory(submodules/dsp/)
add_subdirectory(submodules/readerwriterqueue/)
add_subdirectory(src/)


target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        PROJECT_NAME="${PROJECT_NAME}"
        COMPANY_NAME="${COMPANY_NAME}"
        SPRINGS_RMS
        TAPEDELAY_SWITCH_INDICATOR
    )

#ignore omp simd pragma
target_compile_options(${PROJECT_NAME} PRIVATE
       -fopenmp-simd
   )

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_assets
        ${PROJECT_NAME}_factory
        ${PROJECT_NAME}_fonts
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_opengl
        readerwriterqueue
        dsp_springs_processor
        dsp_tapedelay_processor
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )

