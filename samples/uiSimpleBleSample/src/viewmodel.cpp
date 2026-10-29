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
    ObserveScanResultsUseCase& observeUseCase,
    StartConnectionUseCase& connectionUseCase,
    StopConnectionUseCase& stopConnectionUseCaseArg,
    JkBmsCpp::ObserveJkbmsStateUseCase& observeJkbmsUseCase,
    IoExecutor& executor) 
    : getAdaptersUseCase(useCase),
      startScanUseCase(scanUseCase),
      stopScanUseCase(stopScanUseCaseArg),
      observeScanResultsUseCase(observeUseCase),
      startConnectionUseCase(connectionUseCase),
      stopConnectionUseCase(stopConnectionUseCaseArg),
      observeJkbmsStateUseCase(observeJkbmsUseCase),
      ioExecutor(executor),
      stateFlow(
        std::make_shared<ViewState>(ViewState{
        .currentScreen = Screen::SELECT_HOST_DEVICE,
        .bleHostDevices = std::vector<std::string>{},
        .scannedDevices = std::vector<ScannedDevice>{}
})) {
}

void ViewModel::onUiAction(const UiEvent& event) {
    if (const auto* renderStarted = dynamic_cast<const RenderStarted*>(&event)) {
        handleRenderStarted();
    } else if (const auto* hostDeviceQuitClicked = dynamic_cast<const HostDeviceQuitClicked*>(&event)) {
        handleHostDeviceQuitClicked();
    } else if (const auto* hostDeviceSelected = dynamic_cast<const OnHostDeviceSelected*>(&event)) {
        handleHostDeviceSelected(*hostDeviceSelected);
    } else if (const auto* scanDeviceQuitClicked = dynamic_cast<const ScanDeviceQuitClicked*>(&event)) {
        handleScanDeviceQuitClicked();
    } else if (const auto* scannedDeviceSelected = dynamic_cast<const OnScannedDeviceSelected*>(&event)) {
        handleScannedDeviceSelected(*scannedDeviceSelected);
    } else if (const auto* bmsInfoBackClicked = dynamic_cast<const BmsInfoBackClicked*>(&event)) {
        handleBmsInfoBackClicked();
    }
}

void ViewModel::handleRenderStarted() {
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
}

void ViewModel::handleHostDeviceQuitClicked() {
    stateFlow.update([](const auto& currentState) {
        auto newState = currentState->copy(ViewState{
            .needToCloseApp = true
        });
        return std::make_shared<ViewState>(newState);
    });
}

void ViewModel::handleHostDeviceSelected(const OnHostDeviceSelected& event) {
    if (event.selectedIndex < static_cast<int>(adapterResults.size())) {
        const auto& selectedAdapter = adapterResults[event.selectedIndex];
        startScanUseCase.start(selectedAdapter.adapterId);
        
        auto& scanFlow = observeScanResultsUseCase.observe(selectedAdapter.adapterId);
        scanFlow.subscribe(this);
        
        stateFlow.update([selectedIndex = event.selectedIndex](const auto& currentState) {
            auto newState = currentState->copy(ViewState{
                .currentScreen = Screen::SELECT_TARGET_DEVICE,
                .activeHostDevice = selectedIndex
            });
            return std::make_shared<ViewState>(newState);
        });
    }
}

void ViewModel::handleScanDeviceQuitClicked() {
    stateFlow.update([this](const auto& currentState) {
        if (currentState->activeHostDevice.has_value()) {
            const auto& selectedAdapter = adapterResults[currentState->activeHostDevice.value()];
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
}

void ViewModel::handleScannedDeviceSelected(const OnScannedDeviceSelected& event) {
    spdlog::info("Selected scanned device with ID: {}", event.deviceId);
    auto currentState = stateFlow.state();
    if (currentState->activeHostDevice.has_value()) {
        const auto& selectedAdapter = adapterResults[currentState->activeHostDevice.value()];
        currentConnectedDeviceId = event.deviceId;
        
        try {
            spdlog::info("Starting BMS connection to device: {} on adapter: {}", event.deviceId, selectedAdapter.adapterId);
            runOnIo([this, adapterId=selectedAdapter.adapterId,peripheralId=event.deviceId](){
                startConnectionUseCase.start(adapterId, peripheralId);
                observeJkbmsStateUseCase.observe(peripheralId).subscribe(this);
            });
        } catch (const std::exception& e) {
            spdlog::error("Error starting BMS connection: {}", e.what());
        }

        stateFlow.update([this](const auto& currentState) {
            auto newState = currentState->copy(ViewState{
                .currentScreen = Screen::VIEW_BMS_INFORMATION,
                .bmsInfo = BmsInfo{.loading = true}
            });
            return std::make_shared<ViewState>(newState);
        });
    }
}

void ViewModel::handleBmsInfoBackClicked() {
    auto currentState = stateFlow.state();
    if (currentState->activeHostDevice.has_value() && !currentConnectedDeviceId.empty()) {
        const auto& selectedAdapter = adapterResults[currentState->activeHostDevice.value()];
        
        observeJkbmsStateUseCase.observe(currentConnectedDeviceId)
            .unsubscribe(this);
        runOnIo([this, adapterId=selectedAdapter.adapterId,peripheralId=currentConnectedDeviceId](){
            stopConnectionUseCase.stop(adapterId, peripheralId);
        });
        currentConnectedDeviceId.clear();
    }
    firstBmsStateReceived = false;
    stateFlow.update([](const auto& currentState) {
        auto newState = currentState->copy(ViewState{
            .currentScreen = Screen::SELECT_TARGET_DEVICE
        });
        return std::make_shared<ViewState>(newState);
    });
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

void ViewModel::on_state_changed(const JkbmsState& bmsState) {
    spdlog::info("BMS state updated: vendor={}, hw={}, sw={}", 
                 bmsState.vendorId, bmsState.hwVersion, bmsState.swVersion);
    
    BmsInfo bmsInfo{
        .vendorId = bmsState.vendorId,
        .hwVersion = bmsState.hwVersion,
        .swVersion = bmsState.swVersion,
        .uptimeSeconds = bmsState.uptimeSeconds,
        .deviceName = bmsState.deviceName,
        .serialNumber = bmsState.serialNumber,
        .loading = false
    };
    
    stateFlow.update([&bmsInfo](const auto& currentState) {
        auto newState = currentState->copy(ViewState{
            .bmsInfo = bmsInfo
        });
        return std::make_shared<ViewState>(newState);
    });
    
    firstBmsStateReceived = true;
}

}