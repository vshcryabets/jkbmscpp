#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "viewmodel.h"

class App: public StateListener {
private:
    ViewModel &viewModel_;
    ftxui::ScreenInteractive screen_;
    std::string device_path;
    ftxui::Component connectToDeviceDialog();

    std::mutex state_mutex_;
    std::vector<std::string> menu_entries_;

    int current_value_cA = 150;
    int cutoff_voltage_mV = 270;
public:
    SampleDl24App(ViewModel &viewModel);
    ~SampleDl24App();
    void run();
    void onStateChanged(const ViewState& state) override;
};
