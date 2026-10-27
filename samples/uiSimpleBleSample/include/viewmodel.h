#pragma once

#include <string>
#include <vector>
#include <optional>

#include "stateflow.h"
#include "GetAdaptersUseCase.h"

enum class Screen: uint8_t {
    DEVICE_LIST = 1,
    DEVICE_INFO
};

struct ViewState {
    const std::optional<Screen> currentScreen = std::nullopt;
    const std::optional<std::vector<std::string>> bleHostDevices = std::nullopt;
    const std::optional<std::vector<std::string>> scannedDevices = std::nullopt;
    const std::optional<bool> showHostDevices = std::nullopt;

    [[nodiscard]] bool operator==(const ViewState& other) const {
        return currentScreen == other.currentScreen && 
               bleHostDevices == other.bleHostDevices &&
               scannedDevices == other.scannedDevices &&
               showHostDevices == other.showHostDevices;
    }

    [[nodiscard]] bool operator!=(const ViewState& other) const {
        return !(*this == other);
    }

    [[nodiscard]] ViewState copy(const ViewState &copy) const {
        ViewState result = {
            .currentScreen = copy.currentScreen.has_value() ? copy.currentScreen : this->currentScreen,
            .bleHostDevices = copy.bleHostDevices.has_value() ? copy.bleHostDevices : this->bleHostDevices,
            .scannedDevices = copy.scannedDevices.has_value() ? copy.scannedDevices : this->scannedDevices,
            .showHostDevices = copy.showHostDevices.has_value() ? copy.showHostDevices : this->showHostDevices
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

class ExitAppEvent : public UiEvent {};

class RenderStarted : public UiEvent {};

class ViewModel {
private:
    flow::StateFlow<std::shared_ptr<ViewState>> stateFlow;
    GetAdaptersUseCase& getAdaptersUseCase;

public:
    ViewModel(GetAdaptersUseCase& useCase);
    virtual ~ViewModel() = default;
    void onUiAction(const UiEvent& event);
    flow::StateFlow<std::shared_ptr<ViewState>>& getStateFlow() { return stateFlow; }
};