#include "viewmodel.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

ViewModel::ViewModel() : stateFlow(
    std::make_shared<ViewState>(ViewState{
    .currentScreen = Screen::DEVICE_LIST,
    .devices = std::vector<std::string>{"a", "b", "c"}
})) {
}

void ViewModel::onUiAction(const UiEvent& event) {
    (void)event;
}
