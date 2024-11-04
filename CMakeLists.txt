cmake_minimum_required(VERSION 3.15)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "Build for 10.14")

project(lattices VERSION 0.3.0(alpha))
set(CMAKE_CXX_STANDARD 17)

include (cmake/CPM.cmake)

CPMAddPackage("gh:juce-framework/JUCE#8.0.3")

juce_add_plugin(${PROJECT_NAME}
    PRODUCT_NAME "Lattices"
    COMPANY_NAME "TBD Tuners"  
    BUNDLE_ID "org.tbd.tuners.lattices"
    PLUGIN_MANUFACTURER_CODE TbdL
    PLUGIN_CODE LatT

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    FORMATS AU VST3 Standalone

    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_add_binary_data(lattices-binary
  NAMESPACE LatticesBinary
  HEADER_NAME LatticesBinary.h
  SOURCES ${CMAKE_SOURCE_DIR}/resources/fonts/Stoke.otf
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  src/
)

add_subdirectory (libs/melatonin_inspector)
juce_add_module("libs/melatonin_blur")
add_library(oddsound-mts-source libs/MTS-ESP/Master/libMTSMaster.cpp)
target_include_directories(oddsound-mts-source PUBLIC libs/MTS-ESP/master)
target_link_libraries(${PROJECT_NAME} PRIVATE melatonin_inspector)
target_link_libraries(${PROJECT_NAME} PRIVATE melatonin_blur)

target_sources(${PROJECT_NAME} PRIVATE src/PluginEditor.cpp src/PluginProcessor.cpp)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    JUCE_ALLOW_STATIC_NULL_VARIABLES=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1

    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    
    JUCE_REPORT_APP_USAGE=0

    JUCE_ALSA=0
    JUCE_JACK=0
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        juce::juce_graphics
        juce::juce_audio_utils
        juce::juce_audio_devices
        oddsound-mts-source
        )
        
target_link_libraries(${PROJECT_NAME} PRIVATE lattices-binary)

