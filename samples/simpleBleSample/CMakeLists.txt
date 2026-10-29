add_executable(simpleBleSample
    src/main.cpp
    src/JkBmsSourceSimpleBleImpl.cpp
)

target_link_libraries(simpleBleSample PRIVATE simpleble libjkbmscpp)

target_include_directories(simpleBleSample PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${simpleble_SOURCE_DIR}/include
)

