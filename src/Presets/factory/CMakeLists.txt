file(GLOB FACTORY_PRESETS *.preset)
juce_add_binary_data(${PROJECT_NAME}_factory
    HEADER_NAME Factory.h
    NAMESPACE Factory
    SOURCES
    ${FACTORY_PRESETS}
)

