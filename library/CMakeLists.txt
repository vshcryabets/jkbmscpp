cmake_minimum_required(VERSION 3.31)

project(libjkbmscpp VERSION 0.1.0 LANGUAGES CXX)
add_library(libjkbmscpp)

target_include_directories(libjkbmscpp PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_sources(libjkbmscpp PRIVATE
    src/JkBmsController.cpp
)