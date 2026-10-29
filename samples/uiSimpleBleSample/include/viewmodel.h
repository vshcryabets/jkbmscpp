#pragma once

#include <string>
#include <vector>
#include <optional>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "stateflow.h"
#include "IoExecutor.h"
#include "GetAdaptersUseCase.h"
#include "StartScanUseCase.h"
#include "StopScanUseCase.h"
#include "ObserveScanResultsUseCase.h"
#include "StartConnectionUseCase.h"
#include "StopConnectionUseCase.h"
#include "ObserveJkbmsStateUseCase.h"

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

struct BmsInfo {
    std::string vendorId;
    std::string hwVersion;
    std::string swVersion;
    uint32_t uptimeSeconds;
    std::string deviceName;
    std::string serialNumber;
    bool loading = false;

    [[nodiscard]] bool operator==(const BmsInfo& other) const {
        return vendorId == other.vendorId &&
               hwVersion == other.hwVersion &&
               swVersion == other.swVersion &&
               uptimeSeconds == other.uptimeSeconds &&
               deviceName == other.deviceName &&
               serialNumber == other.serialNumber &&
               loading == other.loading;
    }

    [[nodiscard]] bool operator!=(const BmsInfo& other) const {
        return !(*this == other);
    }
};

}

template <>
struct fmt::formatter<uiSimpleBleSample::BmsInfo> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    
    auto format(const uiSimpleBleSample::BmsInfo& bms, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(),
            "BmsInfo(vendor={}, hw={}, sw={}, uptime={}s, name={}, sn={}, loading={})",
            bms.vendorId, bms.hwVersion, bms.swVersion, bms.uptimeSeconds,
            bms.deviceName, bms.serialNumber, bms.loading);
    }
};

namespace uiSimpleBleSample {

struct ViewState {
    const std::optional<Screen> currentScreen = std::nullopt;
    const std::optional<std::vector<std::string>> bleHostDevices = std::nullopt;
    const std::optional<std::vector<ScannedDevice>> scannedDevices = std::nullopt;
    const std::optional<bool> showHostDevices = std::nullopt;
    const std::optional<int> activeHostDevice = std::nullopt;
    const std::optional<int> activeScanDevice = std::nullopt;
    const std::optional<bool> needToCloseApp = std::nullopt;
    const std::optional<BmsInfo> bmsInfo = std::nullopt;

    [[nodiscard]] bool operator==(const ViewState& other) const {
        return currentScreen == other.currentScreen && 
               bleHostDevices == other.bleHostDevices &&
               scannedDevices == other.scannedDevices &&
               showHostDevices == other.showHostDevices &&
               activeHostDevice == other.activeHostDevice &&
               activeScanDevice == other.activeScanDevice &&
               needToCloseApp == other.needToCloseApp &&
               bmsInfo == other.bmsInfo;
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
            .needToCloseApp = copy.needToCloseApp.has_value() ? copy.needToCloseApp : this->needToCloseApp,
            .bmsInfo = copy.bmsInfo.has_value() ? copy.bmsInfo : this->bmsInfo
        };
        return result;
    }

    ViewState operator+(const ViewState& other) const {
        return this->copy(other);
    }
};

}

template <>
struct fmt::formatter<uiSimpleBleSample::ViewState> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    
    auto format(const uiSimpleBleSample::ViewState& state, fmt::format_context& ctx) const {
        std::string screen_str = "null";
        if (state.currentScreen.has_value()) {
            screen_str = fmt::format("Screen::{}", static_cast<int>(state.currentScreen.value()));
        }
        
        std::string host_str = "null";
        if (state.activeHostDevice.has_value()) {
            host_str = fmt::format("{}", state.activeHostDevice.value());
        }
        
        std::string scan_str = "null";
        if (state.activeScanDevice.has_value()) {
            scan_str = fmt::format("{}", state.activeScanDevice.value());
        }
        
        std::string close_str = "null";
        if (state.needToCloseApp.has_value()) {
            close_str = fmt::format("{}", state.needToCloseApp.value());
        }
        
        std::string bms_str = "null";
        if (state.bmsInfo.has_value()) {
            bms_str = fmt::format("{}", state.bmsInfo.value());
        }
        
        return fmt::format_to(ctx.out(),
            "ViewState(screen={}, hostDevice={}, scanDevice={}, closeApp={}, bms={})",
            screen_str, host_str, scan_str, close_str, bms_str);
    }
};

namespace uiSimpleBleSample {

struct UiEvent {
    virtual ~UiEvent() = default;
};

class HostDeviceQuitClicked : public UiEvent {};
class ScanDeviceQuitClicked : public UiEvent {};
class BmsInfoBackClicked : public UiEvent {};

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

class ViewModel : public flow::StateListener<std::vector<Peripheral>>, 
                  public flow::StateListener<JkbmsState> {
private:
    flow::StateFlow<std::shared_ptr<ViewState>> stateFlow;
    GetAdaptersUseCase& getAdaptersUseCase;
    StartScanUseCase& startScanUseCase;
    StopScanUseCase& stopScanUseCase;
    ObserveScanResultsUseCase& observeScanResultsUseCase;
    StartConnectionUseCase& startConnectionUseCase;
    StopConnectionUseCase& stopConnectionUseCase;
    JkBmsCpp::ObserveJkbmsStateUseCase& observeJkbmsStateUseCase;
    IoExecutor& ioExecutor;
    std::vector<GetAdaptersUseCase::Result> adapterResults;
    std::string currentConnectedDeviceId;
    bool firstBmsStateReceived = false;

    void handleRenderStarted();
    void handleHostDeviceQuitClicked();
    void handleHostDeviceSelected(const OnHostDeviceSelected& event);
    void handleScanDeviceQuitClicked();
    void handleScannedDeviceSelected(const OnScannedDeviceSelected& event);
    void handleBmsInfoBackClicked();

public:
    ViewModel(GetAdaptersUseCase& useCase, StartScanUseCase& scanUseCase, StopScanUseCase& stopScanUseCase, ObserveScanResultsUseCase& observeUseCase, StartConnectionUseCase& connectionUseCase, StopConnectionUseCase& stopConnectionUseCase, JkBmsCpp::ObserveJkbmsStateUseCase& observeJkbmsUseCase, IoExecutor& executor);
    virtual ~ViewModel() = default;
    void onUiAction(const UiEvent& event);
    flow::StateFlow<std::shared_ptr<ViewState>>& getStateFlow() { return stateFlow; }
    
    template<typename Func>
    void runOnIo(Func&& func) {
        ioExecutor.submit(std::forward<Func>(func));
    }
    
    virtual void on_state_changed(const std::vector<Peripheral>& peripherals) override;
    virtual void on_state_changed(const JkbmsState& bmsState) override;
};

}