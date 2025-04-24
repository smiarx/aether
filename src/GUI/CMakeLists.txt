target_sources(${PROJECT_NAME}
    PRIVATE
        PluginEditor.cpp
        CustomLNF.cpp
        DelaySection.cpp
        SpringsSection.cpp
        SpringsGL.cpp
        ToolTip.cpp
        Led.cpp
        PresetComponent.cpp
        ArrowButton.cpp
        Title.cpp
        Typefaces.cpp
    )

add_subdirectory(assets/)
add_subdirectory(fonts/)
