add_executable(uiSimpleBleSample
    src/main.cpp
    src/JkBmsSourceSimpleBleImpl.cpp
    src/viewmodel.cpp
    src/app.cpp
    src/BleRepoSimpleBleImpl.cpp
)

include(FetchContent)
message(STATUS "Downloading FTXUI library")
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG        v6.1.9
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(ftxui)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(
    uiSimpleBleSample 
    PRIVATE 
    simpleble 
    libjkbmscpp
    ftxui::screen
    ftxui::dom
    ftxui::component
    )

target_include_directories(uiSimpleBleSample PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${simpleble_SOURCE_DIR}/include
    ${spdlog_SOURCE_DIR}/include
)
