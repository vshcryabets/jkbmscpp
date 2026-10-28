#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "viewmodel.h"

namespace uiSimpleBleSample {

class App: public flow::StateListener<std::shared_ptr<ViewState>> {
private:
    ViewModel &viewModel_;
    ftxui::ScreenInteractive screen_;


    // UI only thread data
    int host_device_selected_ = 0;
    int scan_device_selected_ = 0;
    std::vector<ScannedDevice> scanned_devices_;
    std::vector<std::string> scanned_devices_display_;
    std::vector<std::string> host_devices_;

    // callback data
    std::mutex state_mutex_;
    std::shared_ptr<ViewState> pending_state_;
    bool needs_update_ = false;

public:
    App(ViewModel &viewModel);
    ~App();
    void run();
    void on_state_changed(const std::shared_ptr<ViewState>& state) override;
};

}