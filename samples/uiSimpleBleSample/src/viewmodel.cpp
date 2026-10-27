#include "viewmodel.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

ViewModel::ViewModel(GetAdaptersUseCase& useCase) 
    : getAdaptersUseCase(useCase),
      stateFlow(
        std::make_shared<ViewState>(ViewState{
        .currentScreen = Screen::DEVICE_LIST,
        .bleHostDevices = std::vector<std::string>{},
        .scannedDevices = std::vector<std::string>{"a", "b", "c"}
})) {
}

void ViewModel::onUiAction(const UiEvent& event) {
    if (const auto* renderStarted = dynamic_cast<const RenderStarted*>(&event)) {
        (void)renderStarted;
        auto results = getAdaptersUseCase.get();
        
        std::vector<std::string> adapterNames;
        for (const auto& result : results) {
            adapterNames.push_back(result.name);
        }
        
        bool showDevices = results.size() != 1;
        
        stateFlow.update([&adapterNames, showDevices](const auto& currentState) {
            auto newState = currentState->copy(ViewState{
                .bleHostDevices = adapterNames,
                .showHostDevices = showDevices
            });
            return std::make_shared<ViewState>(newState);
        });
    }
}
