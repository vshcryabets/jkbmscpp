cmake_minimum_required(VERSION 3.31)

project(libjkbmscpp VERSION 0.1.0 LANGUAGES CXX)

option(USE_STD_STRING "Use std::string instead of custom JkBmsString" ON)

add_library(libjkbmscpp)

target_include_directories(libjkbmscpp PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_sources(libjkbmscpp PRIVATE
    src/JkBmsController.cpp
    src/JkBmsTools.cpp
)

if(USE_STD_STRING)
    target_compile_definitions(libjkbmscpp PUBLIC JKBMSCPP_USE_STD_STRING)
endif()