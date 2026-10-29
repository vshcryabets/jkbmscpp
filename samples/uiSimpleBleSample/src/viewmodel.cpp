#include "viewmodel.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace uiSimpleBleSample {

ViewModel::ViewModel(
    GetAdaptersUseCase& useCase, 
    StartScanUseCase& scanUseCase, 
    StopScanUseCase& stopScanUseCaseArg, 
    ObserveScanResultsUseCase& observeUseCase) 
    : getAdaptersUseCase(useCase),
      startScanUseCase(scanUseCase),
      stopScanUseCase(stopScanUseCaseArg),
      observeScanResultsUseCase(observeUseCase),
      stateFlow(
        std::make_shared<ViewState>(ViewState{
        .currentScreen = Screen::SELECT_HOST_DEVICE,
        .bleHostDevices = std::vector<std::string>{},
        .scannedDevices = std::vector<ScannedDevice>{}
})) {
}

void ViewModel::onUiAction(const UiEvent& event) {
    if (const auto* renderStarted = dynamic_cast<const RenderStarted*>(&event)) {
        (void)renderStarted;
        adapterResults = getAdaptersUseCase.get();
        
        std::vector<std::string> adapterNames;
        for (const auto& result : adapterResults) {
            adapterNames.push_back(result.name);
        }
        
        bool showDevices = adapterResults.size() != 1;
        
        stateFlow.update([&adapterNames, showDevices](const auto& currentState) {
            auto newState = currentState->copy(ViewState{
                .bleHostDevices = adapterNames,
                .showHostDevices = showDevices
            });
            return std::make_shared<ViewState>(newState);
        });
    } else if (const auto* hostDeviceQuitClicked = dynamic_cast<const HostDeviceQuitClicked*>(&event)) {
        (void)hostDeviceQuitClicked;
        stateFlow.update([](const auto& currentState) {
            auto newState = currentState->copy(ViewState{
                .needToCloseApp = true
            });
            return std::make_shared<ViewState>(newState);
        });
    } else if (const auto* hostDeviceSelected = dynamic_cast<const OnHostDeviceSelected*>(&event)) {
        if (hostDeviceSelected->selectedIndex < static_cast<int>(adapterResults.size())) {
            const auto& selectedAdapter = adapterResults[hostDeviceSelected->selectedIndex];
            startScanUseCase.start(selectedAdapter.adapterId);
            
            // Subscribe to scan results
            auto& scanFlow = observeScanResultsUseCase.observe(selectedAdapter.adapterId);
            scanFlow.subscribe(this);
            
            stateFlow.update([selectedIndex = hostDeviceSelected->selectedIndex](const auto& currentState) {
                auto newState = currentState->copy(ViewState{
                    .currentScreen = Screen::SELECT_TARGET_DEVICE,
                    .activeHostDevice = selectedIndex
                });
                return std::make_shared<ViewState>(newState);
            });
        }
    } else if (const auto* scanDeviceQuitClicked = dynamic_cast<const ScanDeviceQuitClicked*>(&event)) {
        (void)scanDeviceQuitClicked;
        stateFlow.update([this](const auto& currentState) {
            if (currentState->activeHostDevice.has_value()) {
                const auto& selectedAdapter = adapterResults[currentState->activeHostDevice.value()];
                
                // Unsubscribe from scan results
                auto& scanFlow = observeScanResultsUseCase.observe(selectedAdapter.adapterId);
                scanFlow.unsubscribe(this);
                
                stopScanUseCase.stop(selectedAdapter.adapterId);
            }
            auto newState = currentState->copy(ViewState{
                .currentScreen = Screen::SELECT_HOST_DEVICE,
                .scannedDevices = std::vector<ScannedDevice>{}
            });
            return std::make_shared<ViewState>(newState);
        });
    } else if (const auto* scannedDeviceSelected = dynamic_cast<const OnScannedDeviceSelected*>(&event)) {
        spdlog::info("Selected scanned device with ID: {}", scannedDeviceSelected->deviceId);
    }
}

void ViewModel::on_state_changed(const std::vector<Peripheral>& peripherals) {
    std::vector<ScannedDevice> scannedDevices;
    for (const auto& peripheral : peripherals) {
        scannedDevices.push_back({
            .name = (peripheral.name.empty() ? "Unknown" : peripheral.name) + 
                    " (" + peripheral.id + ")",
            .id = peripheral.id
        });
    }
    
    std::sort(scannedDevices.begin(), scannedDevices.end(), 
        [](const ScannedDevice& a, const ScannedDevice& b) {
            return a.name < b.name;
        });
    
    stateFlow.update([&scannedDevices](const auto& currentState) {
        auto newState = currentState->copy(ViewState{
            .scannedDevices = scannedDevices
        });
        return std::make_shared<ViewState>(newState);
    });
}

}