#include "app.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <spdlog/spdlog.h>

using namespace ftxui;

using namespace ftxui;


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
    // 2. Create the Menu Component
    MenuOption option;
    option.on_enter = [&] {
        if (menu_selected_ >= 0 && menu_selected_ < static_cast<int>(scanned_devices_.size()) &&
            scanned_devices_[menu_selected_] == "Quit")
        {
            viewModel_.onUiAction(ExitAppEvent{});
            screen_.Exit();
        }
    };
    auto menu = Menu(&scanned_devices_, &menu_selected_, option);

    auto layout = Renderer(menu, [&, menu] {
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (needs_update_) {
                scanned_devices_ = pending_scanned_devices_;
                // Fix cursor postiotion
                if (menu_selected_ >= static_cast<int>(scanned_devices_.size())) {
                    menu_selected_ = scanned_devices_.empty() ? 0 : static_cast<int>(scanned_devices_.size()) - 1;
                }
                needs_update_ = false;
            }
        }

        return hbox({
            window(text(" Menu "), menu->Render()) | size(ftxui::WIDTH, ftxui::EQUAL, 32)
        });
    });

    viewModel_.getStateFlow().subscribe(this);
    viewModel_.onUiAction(RenderStarted{});
    screen_.Loop(layout);
}

void App::on_state_changed(const std::shared_ptr<ViewState>& state) {
    spdlog::info("App::on_state_changed: state changed, updating menu entries");
    std::vector<std::string> next_entries;

    if (state && state->scannedDevices.has_value()) {
        next_entries = state->scannedDevices.value();
    }
    next_entries.push_back("Quit");

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        pending_scanned_devices_ = std::move(next_entries);
        needs_update_ = true;
    }

    screen_.PostEvent(ftxui::Event::Custom);    
}