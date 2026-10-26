add_executable(uiSimpleBleSample
    src/main.cpp
    src/JkBmsSourceSimpleBleImpl.cpp
)

include(FetchContent)
message(STATUS "Downloading FTXUI library")
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG        v6.1.9
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(ftxui)

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
)
