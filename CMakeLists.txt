cmake_minimum_required(VERSION 3.22)
project(LindsayDistortion VERSION 1.1.0 LANGUAGES C CXX)

if(JUCE_DIR)
    add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.12 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(LindsayDistortion
    COMPANY_NAME "CEEM"
    PRODUCT_NAME "Lindsay Low-Hands"
    PLUGIN_MANUFACTURER_CODE Ceem
    PLUGIN_CODE LnLh
    FORMATS VST3
    VST3_CATEGORIES Fx Distortion)
target_sources(LindsayDistortion PRIVATE Plugin.cpp)
target_compile_features(LindsayDistortion PRIVATE cxx_std_17)
target_compile_definitions(LindsayDistortion PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(LindsayDistortion PRIVATE juce::juce_audio_processors)




