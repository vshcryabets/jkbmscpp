cmake_minimum_required(VERSION 3.25)

project(simpleBleSample VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(simpleBleSample)

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

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../library ${CMAKE_CURRENT_BINARY_DIR}/library)
add_subdirectory(${simpleble_SOURCE_DIR}/simpleble ${simpleble_BINARY_DIR}/simpleble)

target_sources(simpleBleSample
    PRIVATE
    src/main.cpp
    src/JkBmsSourceSimpleBleImpl.cpp
    )

target_link_libraries(simpleBleSample PRIVATE simpleble libjkbmscpp)

target_include_directories(simpleBleSample PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)