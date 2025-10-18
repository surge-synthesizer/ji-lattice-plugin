cmake_minimum_required(VERSION 3.15)

message(STATUS "Building ji-lattice-plugin")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "Build for 10.14")

project(ji-lattice-plugin VERSION 0.3.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

option(JI_LATTICE_COPY_AFTER_BUILD "Copy the plugin after build" TRUE)

include (cmake/CPM.cmake)

CPMAddPackage("gh:juce-framework/JUCE#8.0.4")
CPMAddPackage("gh:free-audio/clap-juce-extensions#main")

# Set this here so we can use it in installer also
set(PRODUCT_NAME "Lattices")

juce_add_plugin(${PROJECT_NAME}
    PRODUCT_NAME ${PRODUCT_NAME}
    COMPANY_NAME "Surge Synth Team"
    BUNDLE_ID "org.surge-synth-team.ji-lattice-plugin"
    PLUGIN_MANUFACTURER_CODE VmbA
    PLUGIN_CODE LatT

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    FORMATS AU VST3 Standalone

    COPY_PLUGIN_AFTER_BUILD ${JI_LATTICE_COPY_AFTER_BUILD}
)

clap_juce_extensions_plugin(TARGET ${PROJECT_NAME}
        CLAP_ID "org.sure-synth-team.ji-lattice-plugin.clap"
        CLAP_FEATURES "instrument" microtonal "just intonation")



juce_add_binary_data(lattices-binary
  NAMESPACE LatticesBinary
  HEADER_NAME LatticesBinary.h
  SOURCES ${CMAKE_SOURCE_DIR}/resources/fonts/Stoke.otf
)

#juce_add_binary_data(lattices-assets
#  NAMESPACE LatticesAssets
#  HEADER_NAME LatticesAssets.h
#  SOURCES ${CMAKE_SOURCE_DIR}/resources/graphics/Horiz.png
#        ${CMAKE_SOURCE_DIR}/resources/graphics/Up.png
#        ${CMAKE_SOURCE_DIR}/resources/graphics/Down.png
#)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  src/
)

add_subdirectory(libs/melatonin_inspector)
juce_add_module("libs/melatonin_blur")
add_library(oddsound-mts-source libs/MTS-ESP/Master/libMTSMaster.cpp)
target_include_directories(oddsound-mts-source PUBLIC libs/MTS-ESP/Master)
target_link_libraries(${PROJECT_NAME} PRIVATE melatonin_inspector)
target_link_libraries(${PROJECT_NAME} PRIVATE melatonin_blur)

target_sources(${PROJECT_NAME} PRIVATE src/LatticesEditor.cpp src/LatticesProcessor.cpp)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    JUCE_ALLOW_STATIC_NULL_VARIABLES=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1

    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    
    JUCE_REPORT_APP_USAGE=0

    JUCE_ALSA=1
    JUCE_JACK=0
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        juce::juce_graphics
        juce::juce_animation
        juce::juce_audio_utils
        juce::juce_audio_devices
        oddsound-mts-source
        )
        
target_link_libraries(${PROJECT_NAME} PRIVATE
        lattices-binary
#        lattices-assets
)

include(cmake/basic-installer.cmake)
