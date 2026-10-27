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
    scanned_devices_ = {
        "Quit"
    };
}

App::~App() {
    viewModel_.getStateFlow().unsubscribe(this);
}

void App::run() {
    // host devcies
    MenuOption hostDeviceOptions;
    hostDeviceOptions.on_enter = [&] {
        if (host_device_selected_ == (host_devices_.size() - 1)) {
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
        if (scan_device_selected_ == (scanned_devices_.size() - 1)) {
            viewModel_.onUiAction(ScanDeviceQuitClicked{});
        } else {
            // Handle scanned device selection
            spdlog::info("Selected scanned device: {}", scanned_devices_[scan_device_selected_]);
        }
    };
    auto scanDevicesMenu = Menu(&scanned_devices_, &scan_device_selected_, scanDeviceOptions);

    auto layout = Renderer(Container::Horizontal({hostDevicesMenu, scanDevicesMenu}), 
    [&, hostDevicesMenu, scanDevicesMenu] {
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (needs_update_) {
                host_devices_ = pending_state_->bleHostDevices.value_or(host_devices_);
                host_devices_.push_back("Quit");
                if (host_device_selected_ >= host_devices_.size()) {
                    host_device_selected_ = host_devices_.empty() ? 0 : host_devices_.size() - 1;
                }
                scanned_devices_ = pending_state_->scannedDevices.value_or(scanned_devices_);
                scanned_devices_.push_back("Quit");
                if (scan_device_selected_ >= scanned_devices_.size()) {
                    scan_device_selected_ = scanned_devices_.empty() ? 0 : scanned_devices_.size() - 1;
                }
                needs_update_ = false;
            }
            if (pending_state_->needToCloseApp.value_or(false)) {
                screen_.Exit();
            }
            switch (pending_state_->currentScreen.value_or(uiSimpleBleSample::Screen::SELECT_HOST_DEVICE)) {
                case uiSimpleBleSample::Screen::SELECT_HOST_DEVICE:
                    hostDevicesMenu->TakeFocus();
                    break;
                case uiSimpleBleSample::Screen::SELECT_TARGET_DEVICE:
                    scanDevicesMenu->TakeFocus();
                    break;
            }
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
    spdlog::info("App::on_state_changed: state changed, updating menu entries");

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        pending_state_ = state;
        needs_update_ = true;
    }

    screen_.PostEvent(ftxui::Event::Custom);    
}

}