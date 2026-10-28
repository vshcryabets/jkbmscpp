#pragma once

#include <string>
#include <vector>
#include <optional>

#include "stateflow.h"
#include "GetAdaptersUseCase.h"
#include "StartScanUseCase.h"
#include "StopScanUseCase.h"
#include "ObserveScanResultsUseCase.h"

namespace uiSimpleBleSample {

enum class Screen: uint8_t {
    SELECT_HOST_DEVICE = 1,
    SELECT_TARGET_DEVICE = 2,
    VIEW_BMS_INFORMATION = 3
};

struct ScannedDevice {
    std::string name;
    std::string id;
    
    [[nodiscard]] bool operator==(const ScannedDevice& other) const {
        return name == other.name && id == other.id;
    }
    
    [[nodiscard]] bool operator!=(const ScannedDevice& other) const {
        return !(*this == other);
    }
};

struct ViewState {
    const std::optional<Screen> currentScreen = std::nullopt;
    const std::optional<std::vector<std::string>> bleHostDevices = std::nullopt;
    const std::optional<std::vector<ScannedDevice>> scannedDevices = std::nullopt;
    const std::optional<bool> showHostDevices = std::nullopt;
    const std::optional<int> activeHostDevice = std::nullopt;
    const std::optional<int> activeScanDevice = std::nullopt;
    const std::optional<bool> needToCloseApp = std::nullopt;

    [[nodiscard]] bool operator==(const ViewState& other) const {
        return currentScreen == other.currentScreen && 
               bleHostDevices == other.bleHostDevices &&
               scannedDevices == other.scannedDevices &&
               showHostDevices == other.showHostDevices &&
               activeHostDevice == other.activeHostDevice &&
               activeScanDevice == other.activeScanDevice &&
               needToCloseApp == other.needToCloseApp;
    }

    [[nodiscard]] bool operator!=(const ViewState& other) const {
        return !(*this == other);
    }

    [[nodiscard]] ViewState copy(const ViewState &copy) const {
        ViewState result = {
            .currentScreen = copy.currentScreen.has_value() ? copy.currentScreen : this->currentScreen,
            .bleHostDevices = copy.bleHostDevices.has_value() ? copy.bleHostDevices : this->bleHostDevices,
            .scannedDevices = copy.scannedDevices.has_value() ? copy.scannedDevices : this->scannedDevices,
            .showHostDevices = copy.showHostDevices.has_value() ? copy.showHostDevices : this->showHostDevices,
            .activeHostDevice = copy.activeHostDevice.has_value() ? copy.activeHostDevice : this->activeHostDevice,
            .activeScanDevice = copy.activeScanDevice.has_value() ? copy.activeScanDevice : this->activeScanDevice,
            .needToCloseApp = copy.needToCloseApp.has_value() ? copy.needToCloseApp : this->needToCloseApp
        };
        return result;
    }

    ViewState operator+(const ViewState& other) const {
        return this->copy(other);
    }
};

struct UiEvent {
    virtual ~UiEvent() = default;
};

class HostDeviceQuitClicked : public UiEvent {};
class ScanDeviceQuitClicked : public UiEvent {};

class OnHostDeviceSelected : public UiEvent {
public:
    int selectedIndex;
    explicit OnHostDeviceSelected(int index) : selectedIndex(index) {}
};

class OnScannedDeviceSelected : public UiEvent {
public:
    std::string deviceId;
    explicit OnScannedDeviceSelected(const std::string& id) : deviceId(id) {}
};

class RenderStarted : public UiEvent {};

class ViewModel : public flow::StateListener<std::vector<Peripheral>> {
private:
    flow::StateFlow<std::shared_ptr<ViewState>> stateFlow;
    GetAdaptersUseCase& getAdaptersUseCase;
    StartScanUseCase& startScanUseCase;
    StopScanUseCase& stopScanUseCase;
    ObserveScanResultsUseCase& observeScanResultsUseCase;
    std::vector<GetAdaptersUseCase::Result> adapterResults;

public:
    ViewModel(GetAdaptersUseCase& useCase, StartScanUseCase& scanUseCase, StopScanUseCase& stopScanUseCase, ObserveScanResultsUseCase& observeUseCase);
    virtual ~ViewModel() = default;
    void onUiAction(const UiEvent& event);
    flow::StateFlow<std::shared_ptr<ViewState>>& getStateFlow() { return stateFlow; }
    
    virtual void on_state_changed(const std::vector<Peripheral>& peripherals) override;
};

}