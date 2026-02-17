target_sources(${PROJECT_NAME}
    PRIVATE
        LoadPlugin.cpp
        PluginProcessor.cpp
        PluginProcessorArch.cpp
)

if(DSP_X86_DISPATCH)
    target_sources(${PROJECT_NAME}
        PRIVATE
            PluginProcessorAVX2.cpp
    )
    if(MSVC)
        set_source_files_properties(PluginProcessorAVX2.cpp TARGET_DIRECTORY ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
    elseif(APPLE)
        # error on macos no aligned-allocation
        set_source_files_properties(PluginProcessorAVX2.cpp
            TARGET_DIRECTORY ${PROJECT_NAME}
            PROPERTIES COMPILE_FLAGS "-mavx2 -mfma -faligned-allocation")
    else()
        set_source_files_properties(PluginProcessorAVX2.cpp TARGET_DIRECTORY ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")
    endif()
endif()

add_subdirectory(GUI/)
add_subdirectory(Presets/)
