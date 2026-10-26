#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Source.h"
#include "Controller.h"

struct ViewState {
    bool show_connect_dialog = false;
    std::string device_path = "";
    std::vector<std::string> uart_logs = {};
    bool isConnected = false;
    bool shouldCloseApp = false;
};

class StateListener {
public:
    virtual ~StateListener() = default;
    virtual void onStateChanged(const ViewState& state) = 0;
};

struct UiEvent {
    virtual ~UiEvent() = default;
};
struct ConnectDialogCanceled: public UiEvent {};
struct OnConnectRequested: public UiEvent {};
struct OnLoadStart: public UiEvent {};
struct OnLoadStop: public UiEvent {};
struct ResetStatistics: public UiEvent {};
struct OnExitClicked: public UiEvent {};
struct OnSaveLogsRequested: public UiEvent {};
struct TestActionRequested: public UiEvent {};
struct Open: public UiEvent {
    Open(std::string path) : filepath(path), UiEvent() {}
    std::string filepath;
};
struct LoadSetCurrent: public UiEvent {
    LoadSetCurrent(float current) : current(current), UiEvent() {}
    float current;
};
struct LoadSetVoltage: public UiEvent {
    LoadSetVoltage(float voltage) : voltage(voltage), UiEvent() {}
    float voltage;
};

class ViewModel: public dl24::DebugListener {
private:
    void loadConfiguration(std::string configFilePath);
    void saveConfiguration(std::string configFilePath);

    ViewState state;

    std::unique_ptr<dl24::Source> source_;
    std::unique_ptr<dl24::Controller> controller_;
    StateListener* stateListener_ = nullptr;

    void onOpenDevice(const Open* event);
    void onCloseDevice();

    void onDebugMessage(dl24::DebugListener::Level level, const std::string& message) override;
    void saveLogs();
    void testAction();
public:
    ViewModel();
    virtual ~ViewModel() = default;
    void onUiAction(const UiEvent& event);
    const ViewState& getState() const { return state; }
    void setStateListener(StateListener* listener) { stateListener_ = listener; }
    void addLogMessage(const std::string& message);
};