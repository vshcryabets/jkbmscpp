#pragma once

#include <string>
#include <vector>
#include <optional>

#include "stateflow.h"

enum class Screen: uint8_t {
    DEVICE_LIST = 1,
    DEVICE_INFO
};

struct ViewState {
    const std::optional<Screen> currentScreen = std::nullopt;
    const std::optional<std::vector<std::string>> devices = std::nullopt;

    [[nodiscard]] bool operator==(const ViewState& other) const {
        return currentScreen == other.currentScreen && devices == other.devices;
    }

    [[nodiscard]] bool operator!=(const ViewState& other) const {
        return !(*this == other);
    }

    [[nodiscard]] ViewState copy(const ViewState &copy) const {
        ViewState result = {
            .currentScreen = copy.currentScreen.has_value() ? copy.currentScreen : this->currentScreen,
            .devices = copy.devices.has_value() ? copy.devices : this->devices
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

class ViewModel {
private:
    flow::StateFlow<std::shared_ptr<ViewState>> stateFlow;


public:
    ViewModel();
    virtual ~ViewModel() = default;
    void onUiAction(const UiEvent& event);
    flow::StateFlow<std::shared_ptr<ViewState>>& getStateFlow() { return stateFlow; }
};