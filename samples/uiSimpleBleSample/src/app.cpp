#include "app.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <spdlog/spdlog.h>

using namespace ftxui;

namespace uiSimpleBleSample {

App::App(ViewModel &viewModel) : 
    viewModel_(viewModel), 
    screen_(ScreenInteractive::Fullscreen()) {
}

App::~App() {
    viewModel_.getStateFlow().unsubscribe(this);
}

void App::run() {
    // host devices
    MenuOption hostDeviceOptions;
    hostDeviceOptions.on_enter = [&] {
        auto hostDevicesSize = host_devices_.size();
        if (host_device_selected_ == (hostDevicesSize - 1) && 
            hostDevicesSize > 0 && host_devices_.back() == "Quit") {
            viewModel_.onUiAction(HostDeviceQuitClicked{});
        } else {
            viewModel_.onUiAction(OnHostDeviceSelected(host_device_selected_));
            spdlog::info("Selected host device: {}", host_devices_[host_device_selected_]);
        }
    };
    auto hostDevicesMenu = Menu(&host_devices_, &host_device_selected_, hostDeviceOptions);
    
    // scanned devices
    MenuOption scanDeviceOptions;
    scanDeviceOptions.on_enter = [&] {
        auto displaySize = scanned_devices_display_.size();
        if (scan_device_selected_ == (displaySize - 1) && 
            displaySize > 0 && scanned_devices_display_.back() == "Quit") {
            viewModel_.onUiAction(ScanDeviceQuitClicked{});
        } else if (scan_device_selected_ < static_cast<int>(scanned_devices_.size())) {
            viewModel_.onUiAction(OnScannedDeviceSelected(scanned_devices_[scan_device_selected_].id));
            spdlog::info("Selected scanned device: {}", scanned_devices_[scan_device_selected_].name);
        }
    };
    auto scanDevicesMenu = Menu(&scanned_devices_display_, &scan_device_selected_, scanDeviceOptions);

    auto layout = Renderer(Container::Horizontal({hostDevicesMenu, scanDevicesMenu}), 
    [&, hostDevicesMenu, scanDevicesMenu] {
        auto currentScreen = uiSimpleBleSample::Screen::SELECT_HOST_DEVICE;
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (needs_update_) {
                host_devices_ = pending_state_->bleHostDevices.value_or(host_devices_);
                scanned_devices_ = pending_state_->scannedDevices.value_or(scanned_devices_);
                needs_update_ = false;
            }
            if (pending_state_->needToCloseApp.value_or(false)) {
                screen_.Exit();
            }
            currentScreen = pending_state_->currentScreen.value_or(uiSimpleBleSample::Screen::SELECT_HOST_DEVICE);
            switch (currentScreen) {
                case uiSimpleBleSample::Screen::SELECT_HOST_DEVICE:
                    hostDevicesMenu->TakeFocus();
                    break;
                case uiSimpleBleSample::Screen::SELECT_TARGET_DEVICE:
                    scanDevicesMenu->TakeFocus();
                    break;
            }
        }

        if (currentScreen == uiSimpleBleSample::Screen::SELECT_HOST_DEVICE) {
            if (host_devices_.empty() || host_devices_.back() != "Quit") {
                host_devices_.push_back("Quit");
            }
        } else {
            if (!host_devices_.empty() && host_devices_.back() == "Quit") {
                host_devices_.pop_back();
            }
        }
        
        scanned_devices_display_.clear();
        for (const auto& device : scanned_devices_) {
            scanned_devices_display_.push_back(device.name);
        }
        
        if (currentScreen == uiSimpleBleSample::Screen::SELECT_TARGET_DEVICE) {
            if (scanned_devices_display_.empty() || scanned_devices_display_.back() != "Quit") {
                scanned_devices_display_.push_back("Quit");
            }
        } else {
            if (!scanned_devices_display_.empty() && scanned_devices_display_.back() == "Quit") {
                scanned_devices_display_.pop_back();
            }
        }
        
        if (host_device_selected_ >= host_devices_.size() && !host_devices_.empty()) {
            host_device_selected_ = host_devices_.size() - 1;
        }
        if (scan_device_selected_ >= scanned_devices_display_.size() && !scanned_devices_display_.empty()) {
            scan_device_selected_ = scanned_devices_display_.size() - 1;
        }

        return hbox({
            window(text(" Host "), hostDevicesMenu->Render()) | size(ftxui::WIDTH, ftxui::EQUAL, 32),
            window(text(" Scanned "), scanDevicesMenu->Render()) | size(ftxui::WIDTH, ftxui::EQUAL, 32)
        });
    });

    viewModel_.getStateFlow().subscribe(this);
    viewModel_.onUiAction(RenderStarted{});
    screen_.Loop(layout);
}

void App::on_state_changed(const std::shared_ptr<ViewState>& state) {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        pending_state_ = state;
        needs_update_ = true;
    }
    screen_.PostEvent(ftxui::Event::Custom);    
}

}