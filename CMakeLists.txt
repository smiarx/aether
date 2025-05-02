cmake_minimum_required(VERSION 3.15)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "" FORCE)

project(Aether VERSION 0.0.1)

set(COMPANY_NAME "S-audio")


set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

find_package(JUCE CONFIG QUIET)
if(NOT ${JUCE_FOUND})
add_subdirectory(submodules/JUCE)
endif()


set(JUCE_FORMATS VST3 AU)
# Build LV2 only on Linux
if(UNIX AND NOT APPLE)
    message(STATUS "Building LV2 plugin format")
    list(APPEND JUCE_FORMATS LV2)
endif()

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "${COMPANY_NAME}"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE
    PLUGIN_MANUFACTURER_CODE Saud
    PLUGIN_CODE Aeth
    LV2URI https://github.com/smiarx/aether

    FORMATS VST3 ${JUCE_FORMATS}
    PRODUCT_NAME "${PROJECT_NAME}")

set(DSP_SPRINGS_RMS ON CACHE INTERNAL "")
set(DSP_SPRINGS_SHAKE ON CACHE INTERNAL "")
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

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

#ignore omp simd pragma
if(MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE
       /openmp
   )
else()
target_compile_options(${PROJECT_NAME} PRIVATE
       -fopenmp-simd
   )
endif()

set_property(TARGET readerwriterqueue PROPERTY SYSTEM TRUE)
set_property(TARGET dsp_tapedelay_processor PROPERTY SYSTEM TRUE)
set_property(TARGET dsp_springs_processor PROPERTY SYSTEM TRUE)

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
        juce::juce_recommended_warning_flags
    )

