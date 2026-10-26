cmake_minimum_required(VERSION 3.25)

project(jkbmscpp VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Project-wide options
option(BUILD_LIBRARY "Build the jkbmscpp library" ON)
option(BUILD_TESTS "Build and run tests" ON)
option(BUILD_SIMPLE_BLE_SAMPLE "Build simpleBleSample" ON)
option(BUILD_UI_SIMPLE_BLE_SAMPLE "Build uiSimpleBleSample" ON)
option(USE_STD_STRING "Use std::string instead of custom JkBmsString" ON)

# Enable testing at root level if needed
if(BUILD_TESTS)
    enable_testing()
    include(CTest)
endif()

# Declare SimpleBLE once at root level if any sample needs it
if(BUILD_SIMPLE_BLE_SAMPLE OR BUILD_UI_SIMPLE_BLE_SAMPLE)
    include(FetchContent)
    set(SIMPLEBLE_BUILD_ZePHYR OFF CACHE BOOL "" FORCE)
    set(SIMPLEBLE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(SIMPLEBLE_TEST OFF CACHE BOOL "" FORCE)

    message(STATUS "Downloading SimpleBLE...")
    FetchContent_Declare(
        simpleble
        GIT_REPOSITORY https://github.com/simpleble/simpleble.git
        GIT_TAG        v0.10.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(simpleble)
    add_subdirectory(${simpleble_SOURCE_DIR}/simpleble ${simpleble_BINARY_DIR}/simpleble)
endif()

# Build the library
if(BUILD_LIBRARY)
    add_subdirectory(library)
endif()

# Build samples
if(BUILD_SIMPLE_BLE_SAMPLE)
    message(STATUS "Building simpleBleSample")
    add_subdirectory(samples/simpleBleSample)
endif()

if(BUILD_UI_SIMPLE_BLE_SAMPLE)
    message(STATUS "Building uiSimpleBleSample")
    add_subdirectory(samples/uiSimpleBleSample)
endif()

# Print build configuration
message(STATUS "")
message(STATUS "jkbmscpp Build Configuration:")
message(STATUS "  Library:              ${BUILD_LIBRARY}")
message(STATUS "  Tests:                ${BUILD_TESTS}")
message(STATUS "  simpleBleSample:      ${BUILD_SIMPLE_BLE_SAMPLE}")
message(STATUS "  uiSimpleBleSample:    ${BUILD_UI_SIMPLE_BLE_SAMPLE}")
message(STATUS "  Use std::string:      ${USE_STD_STRING}")
message(STATUS "")
