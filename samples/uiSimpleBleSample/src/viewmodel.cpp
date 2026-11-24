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

void ViewModel::onUiAction(const UiEvent& event) noexcept {
    try {
        if (const auto* renderStarted = dynamic_cast<const RenderStarted*>(&event)) {
            handleRenderStarted();
        } else if (const auto* hostDeviceQuitClicked = dynamic_cast<const HostDeviceQuitClicked*>(&event)) {
            handleHostDeviceQuitClicked();
        } else if (const auto* hostDeviceSelected = dynamic_cast<const OnHostDeviceSelected*>(&event)) {
            handleHostDeviceSelected(*hostDeviceSelected);
        } else if (const auto* scanDeviceQuitClicked = dynamic_cast<const ScanDeviceQuitClicked*>(&event)) {
            handleScanDeviceQuitClicked();
        } else if (const auto* scannedDeviceSelected = dynamic_cast<const OnScannedDeviceSelected*>(&event)) {
            runOnIo([this, peripheralId = scannedDeviceSelected->deviceId]() {
                handleScannedDeviceSelected(peripheralId);
            });
        } else if (const auto* bmsInfoBackClicked = dynamic_cast<const BmsInfoBackClicked*>(&event)) {
            handleBmsInfoBackClicked();
        } else {
            spdlog::warn("Unknown UI event type: {}", typeid(event).name());
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in onUiAction: event_type={}, exception_type={}, message={}, code={}",
                      typeid(event).name(), typeid(e).name(), e.what(), -1);
    } catch (...) {
        spdlog::error("Unknown error in onUiAction: event_type={}, message={}",
                      typeid(event).name(), "Unknown exception type");
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

void ViewModel::handleScannedDeviceSelected(const std::string& peripheralId) {
    auto currentState = stateFlow.state();
    if (currentState->activeHostDevice.has_value()) {
        stateFlow.update([this, peripheralId](const auto& currentState) {
            auto newState = currentState->copy(ViewState{
                .currentScreen = Screen::VIEW_BMS_INFORMATION,
                .bmsInfo = BmsInfo{.state = BmsInfoState::Connecting},
                .peripheralId = peripheralId
            });
            return std::make_shared<ViewState>(newState);
        });

        const auto& selectedAdapter = adapterResults[currentState->activeHostDevice.value()];
        auto adapterId = selectedAdapter.adapterId;
        spdlog::info("Selected BLE device - Address: {}, Adapter: {}", 
                     peripheralId, selectedAdapter.adapterId);
        try {
            spdlog::debug(
                "Executing connection task on IO thread: device={}, adapter={}", 
                peripheralId, 
                adapterId);
            ConnectionResult result = startConnectionUseCase.start(adapterId, peripheralId);
            
            if (result == ConnectionResult::Success) {
                spdlog::debug("Connection started successfully: device={}", peripheralId);
                stateFlow.update([this](const auto& state) {
                    return std::make_shared<ViewState>(
                        state->copy(
                            ViewState{ .bmsInfo = BmsInfo { . state = BmsInfoState::Loading } }
                        )
                    );
                });
                
                this->observeFlow = observeJkbmsStateUseCase.observe(peripheralId);
                this->observeFlow->subscribe(this);
                spdlog::debug("Observation subscribed successfully: device={}", peripheralId);
            } else {
                spdlog::error("Failed to start connection: device={}, adapter={}, result={}", 
                              peripheralId, adapterId, static_cast<int>(result));
                stateFlow.update([this](const auto& state) {
                    return std::make_shared<ViewState>(
                        state->copy(
                            ViewState{ .bmsInfo = BmsInfo { . state = BmsInfoState::Error } }
                        )
                    );
                });
            }
        } catch (const std::exception& e) {
            spdlog::error("Error in connection task: device={}, adapter={}, exception_type={}, message={}", 
                            peripheralId, adapterId, typeid(e).name(), e.what());
            stateFlow.update([this](const auto& state) {
                return std::make_shared<ViewState>(
                    state->copy(
                        ViewState{ .bmsInfo = BmsInfo { . state = BmsInfoState::Error } }
                        )
                    );
            });
        } catch (...) {
            spdlog::error("Unknown error in connection task: device={}, adapter={}", peripheralId, adapterId);
            stateFlow.update([this](const auto& state) {
                return std::make_shared<ViewState>(
                    state->copy(
                        ViewState{ .bmsInfo = BmsInfo { . state = BmsInfoState::Error } }
                        )
                    );
            });
        }
    }
}

void ViewModel::handleBmsInfoBackClicked() {
    auto currentState = stateFlow.state();
    if (observeFlow) {
        observeFlow->unsubscribe(this);
        observeFlow = nullptr;
    }
    if (currentState->activeHostDevice.has_value() && currentState->peripheralId.has_value()) {
        auto peripheralId = currentState->peripheralId.value();
        const auto& selectedAdapter = adapterResults[currentState->activeHostDevice.value()];
        runOnIo([
            this, 
            adapterId = selectedAdapter.adapterId,
            peripheralId = peripheralId](){
            stopConnectionUseCase.stop(adapterId, peripheralId);
        });
    }
    stateFlow.update([](const auto& currentState) {
        auto newState = currentState->copy(ViewState{
            .currentScreen = Screen::SELECT_TARGET_DEVICE,
            .peripheralId = std::nullopt
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
        .chargePercentage = bmsState.chargePercentage,
        .remainingCapacity_mAh = bmsState.remainingCapacity_mAh,
        .fullCapacity_mAh = bmsState.fullCapacity_mAh,
        .batteryVoltage_mV = bmsState.batteryVoltage_mV,
        .batteryPower_mW = bmsState.batteryPower_mW,
        .chargeCurrent_mA = bmsState.chargeCurrent_mA,
        .state = BmsInfoState::Ready
    };
    
    stateFlow.update([&bmsInfo](const auto& currentState) {
        auto newState = currentState->copy(ViewState{
            .bmsInfo = bmsInfo
        });
        return std::make_shared<ViewState>(newState);
    });
}

}
