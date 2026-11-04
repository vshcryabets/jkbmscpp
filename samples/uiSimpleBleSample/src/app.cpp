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
    bool hostMenuFocused = false;
    bool scanMenuFocused = false;
    bool backButtonFocused = false;
    
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
    auto hostDevicesWithLogging = hostDevicesMenu | CatchEvent([&hostMenuFocused](Event e) {
        bool nowFocused = (e != Event::Tab && e != Event::TabReverse && 
                          e != Event::ArrowRight && e != Event::ArrowLeft);
        
        if (!hostMenuFocused && nowFocused) {
            hostMenuFocused = true;
            spdlog::info("Host menu focused (onMenuFocused)");
        } else if (hostMenuFocused && (e == Event::Tab || e == Event::TabReverse || 
                                       e == Event::ArrowRight || e == Event::ArrowLeft)) {
            hostMenuFocused = false;
            spdlog::info("Host menu lost focus (onMenuFocusLost)");
        }
        
        if (e == Event::Tab || e == Event::TabReverse || 
            e == Event::ArrowRight || e == Event::ArrowLeft) {
            spdlog::info("User navigating with arrow/tab keys in Host menu");
        }
        return false;
    });
    
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
    auto scanDevicesWithLogging = scanDevicesMenu | CatchEvent([&scanMenuFocused](Event e) {
        bool nowFocused = (e != Event::Tab && e != Event::TabReverse && 
                          e != Event::ArrowRight && e != Event::ArrowLeft);
        
        if (!scanMenuFocused && nowFocused) {
            scanMenuFocused = true;
            spdlog::info("Scanned Devices menu focused (onMenuFocused)");
        } else if (scanMenuFocused && (e == Event::Tab || e == Event::TabReverse || 
                                       e == Event::ArrowRight || e == Event::ArrowLeft)) {
            scanMenuFocused = false;
            spdlog::info("Scanned Devices menu lost focus (onMenuFocusLost)");
        }
        
        if (e == Event::Tab || e == Event::TabReverse || 
            e == Event::ArrowRight || e == Event::ArrowLeft) {
            spdlog::info("User navigating with arrow/tab keys in Scanned Devices menu");
        }
        return false;
    });
    
    // BMS info back button
    auto backButton = Button("Back", [&] {
        viewModel_.onUiAction(BmsInfoBackClicked{});
    });
    auto backButtonWithLogging = backButton | CatchEvent([&backButtonFocused](Event e) {
        bool nowFocused = (e != Event::Tab && e != Event::TabReverse && 
                          e != Event::ArrowRight && e != Event::ArrowLeft);
        
        if (!backButtonFocused && nowFocused) {
            backButtonFocused = true;
            spdlog::info("Back button focused (onMenuFocused)");
        } else if (backButtonFocused && (e == Event::Tab || e == Event::TabReverse || 
                                         e == Event::ArrowRight || e == Event::ArrowLeft)) {
            backButtonFocused = false;
            spdlog::info("Back button lost focus (onMenuFocusLost)");
        }
        return false;
    });

    auto layout = Renderer(Container::Horizontal({hostDevicesMenu, scanDevicesMenu, backButtonWithLogging}), 
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
                infoElements.push_back(separator());
                infoElements.push_back(text("Battery Status") | bold);
                infoElements.push_back(hbox({text("Charge: "), text(std::to_string(bmsInfo_storage_.chargePercentage) + "%") | bold}));
                infoElements.push_back(hbox({text("Remaining: "), text(std::to_string(bmsInfo_storage_.remainingCapacity_mAh) + " mAh") | bold}));
                infoElements.push_back(hbox({text("Full Capacity: "), text(std::to_string(bmsInfo_storage_.fullCapacity_mAh) + " mAh") | bold}));
                infoElements.push_back(hbox({text("Voltage: "), text(std::to_string(bmsInfo_storage_.batteryVoltage_mV) + " mV") | bold}));
                infoElements.push_back(hbox({text("Power: "), text(std::to_string(bmsInfo_storage_.batteryPower_mW) + " mW") | bold}));
                infoElements.push_back(hbox({text("Current: "), text(std::to_string(bmsInfo_storage_.chargeCurrent_mA) + " mA") | bold}));
            }
            bmsInfoPane = vbox(infoElements);
        }

        return hbox({
            window(text(" Host "), hostDevicesMenu->Render()) | size(ftxui::WIDTH, ftxui::EQUAL, 32),
            window(text(" Scanned "), scanDevicesMenu->Render()) | size(ftxui::WIDTH, ftxui::EQUAL, 32),
            window(text(" Details "), bmsInfoPane) | flex
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