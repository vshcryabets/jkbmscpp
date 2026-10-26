cmake_minimum_required(VERSION 3.25)

project(libjkbmscpp VERSION 0.1.0 LANGUAGES CXX C)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)
include(Catch)

add_library(libjkbmscpp)

target_include_directories(libjkbmscpp PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_sources(libjkbmscpp PRIVATE
    src/JkBmsController.cpp
    src/JkBmsTools.cpp
    src/JkBmsFrames.cpp
    src/CFunctions.c
)

if(USE_STD_STRING)
    target_compile_definitions(libjkbmscpp PUBLIC JKBMSCPP_USE_STD_STRING)
endif()

# Tests - only build if requested at root level
if(BUILD_TESTS)
    add_executable(library_test
        test/testJkBmsFrames.cpp
        )
    target_include_directories(library_test PRIVATE ./include/)
    target_link_libraries(library_test PRIVATE Catch2::Catch2WithMain libjkbmscpp)

    catch_discover_tests(library_test 
        PROPERTIES 
        TIMEOUT 10)
endif()
