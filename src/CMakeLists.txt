target_sources(${PROJECT_NAME}
    PRIVATE
        PluginProcessor.cpp
        PluginProcessorArch.cpp
    )

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    target_sources(${PROJECT_NAME}
        PRIVATE
            PluginProcessorAVX2.cpp
    )

    if(MSVC)
        set_source_files_properties(PluginProcessorAVX2.cpp TARGET_DIRECTORY ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
    else()
        set_source_files_properties(PluginProcessorAVX2.cpp TARGET_DIRECTORY ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")
    endif()
endif()

add_subdirectory(GUI/)
add_subdirectory(Presets/)
