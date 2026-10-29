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
        if (scan_device_selected_ == 0) {
            viewModel_.onUiAction(ScanDeviceQuitClicked{});
        } else if (scan_device_selected_ - 1 < static_cast<int>(scanned_devices_.size())) {
            viewModel_.onUiAction(OnScannedDeviceSelected(scanned_devices_[scan_device_selected_ - 1].id));
            spdlog::info("Selected scanned device: {}", scanned_devices_[scan_device_selected_ - 1].name);
        }
    };
    auto scanDevicesMenu = Menu(&scanned_devices_display_, &scan_device_selected_, scanDeviceOptions);

    // BMS info back button
    auto backButton = Button("Back", [&] {
        viewModel_.onUiAction(BmsInfoBackClicked{});
    });

    auto layout = Renderer(Container::Horizontal({hostDevicesMenu, scanDevicesMenu, backButton}), 
    [&, hostDevicesMenu, scanDevicesMenu, backButton] {
        bool haveUpdate = false;
        auto currentScreen = uiSimpleBleSample::Screen::SELECT_HOST_DEVICE;
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (needs_update_) {
                haveUpdate = true;
                host_devices_ = pending_state_->bleHostDevices.value_or(host_devices_);
                scanned_devices_ = pending_state_->scannedDevices.value_or(scanned_devices_);
                
                // Update persistent storage if new BmsInfo provided, otherwise keep previous
                if (pending_state_->bmsInfo.has_value()) {
                    bmsInfo_storage_ = pending_state_->bmsInfo.value();
                }
                needs_update_ = false;
                if (pending_state_->needToCloseApp.value_or(false)) {
                    screen_.Exit();
                }
            }
        }
        currentScreen = pending_state_->currentScreen.value_or(uiSimpleBleSample::Screen::SELECT_HOST_DEVICE);
        if (haveUpdate) {
            host_devices_.push_back("Quit");
            scanned_devices_display_.clear();
            scanned_devices_display_.push_back("Back");
            for (const auto& device : scanned_devices_) {
                scanned_devices_display_.push_back(device.name);
            }

            switch (currentScreen) {
                case uiSimpleBleSample::Screen::SELECT_HOST_DEVICE:
                    hostDevicesMenu->TakeFocus();
                    break;
                case uiSimpleBleSample::Screen::SELECT_TARGET_DEVICE:
                    scanDevicesMenu->TakeFocus();
                    break;
                case uiSimpleBleSample::Screen::VIEW_BMS_INFORMATION:
                    backButton->TakeFocus();
                    break;
            }
        }

        
        if (host_device_selected_ >= host_devices_.size() && !host_devices_.empty()) {
            host_device_selected_ = host_devices_.size() - 1;
        }
        if (scan_device_selected_ >= scanned_devices_display_.size() && !scanned_devices_display_.empty()) {
            scan_device_selected_ = scanned_devices_display_.size() - 1;
        }

        // Build BMS info pane
        Element bmsInfoPane = text("NONE");
        if (currentScreen == uiSimpleBleSample::Screen::VIEW_BMS_INFORMATION) {
            std::vector<Element> infoElements;
            infoElements.push_back(backButton->Render());
            infoElements.push_back(separator());
            
            if (bmsInfo_storage_.loading) {
                infoElements.push_back(text("BMS Information") | bold);
                infoElements.push_back(separator());
                infoElements.push_back(text("Loading...") | color(ftxui::Color::Yellow));
            } else {
                infoElements.push_back(text("BMS Information"));
                infoElements.push_back(separator());
                infoElements.push_back(hbox({text("Vendor: "), text(bmsInfo_storage_.vendorId) | bold}));
                infoElements.push_back(hbox({text("HW Version: "), text(bmsInfo_storage_.hwVersion) | bold}));
                infoElements.push_back(hbox({text("SW Version: "), text(bmsInfo_storage_.swVersion) | bold}));
                infoElements.push_back(hbox({text("Device Name: "), text(bmsInfo_storage_.deviceName) | bold}));
                infoElements.push_back(hbox({text("Serial Number: "), text(bmsInfo_storage_.serialNumber) | bold}));
                infoElements.push_back(hbox({text("Uptime (s): "), text(std::to_string(bmsInfo_storage_.uptimeSeconds)) | bold}));
            }
            bmsInfoPane = vbox(infoElements);
        }

        return hbox({
            window(text(" Host "), hostDevicesMenu->Render()) | size(ftxui::WIDTH, ftxui::EQUAL, 32),
            window(text(" Scanned "), scanDevicesMenu->Render()) | size(ftxui::WIDTH, ftxui::EQUAL, 32),
            window(text(" Details "), bmsInfoPane) | size(ftxui::WIDTH, ftxui::EQUAL, 40)
        });
    });

    viewModel_.getStateFlow().subscribe(this);
    viewModel_.onUiAction(RenderStarted{});
    screen_.Loop(layout);
}

void App::on_state_changed(const std::shared_ptr<ViewState>& state) {
    spdlog::info("Got state update: bmsInfo={}", state->bmsInfo.value_or(BmsInfo{}));
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        pending_state_ = state;
        needs_update_ = true;
    }
    screen_.PostEvent(ftxui::Event::Custom);
}

}