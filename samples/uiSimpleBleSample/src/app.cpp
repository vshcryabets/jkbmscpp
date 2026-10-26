#include "app.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

using namespace ftxui;


App::App(ViewModel &viewModel) : 
    viewModel_(viewModel), 
    screen_(ScreenInteractive::Fullscreen()) {
    viewModel.getStateFlow().subscribe(this);
    menu_entries_ = {
        "Connect",
        "Save logs",
        "Load current",
        "Cutoff voltage",
        "Set timer",
        "Enable Load",
        "Test action",
        "Quit",
    };
}

App::~App() {
    viewModel_.getStateFlow().unsubscribe(this);
}

void App::run() {
    // 1. Define State
    int menu_selected = 0;

    // 2. Create the Menu Component
    MenuOption option;
    option.on_enter = [&] {
        switch (menu_selected)
        {
        case 0:  // Connect / Disconnect
            break;
        case 1:  // Save logs
            break;
        case 2:  // Test action
            break;
        case 3:  // Quit
            break;
        default:
            break;
        }
    };
    auto menu = Menu(&menu_entries_, &menu_selected, option);

    // 3. Create the Main Layout
    // We wrap the interactive menu in a Renderer to define how the screen is drawn
    auto layout = Renderer(menu, [&,state = viewModel_.getStateFlow().state()] {

        // Create the Menu window
        auto menu_window = window(text(" Menu "),
            menu->Render()
        ) | size(WIDTH, EQUAL, 25); // Fixed width of 25 characters

        // Combine them side-by-side (split screen)
        return hbox({
            menu_window
        });
    });
    // 4. Run the application
    screen_.Loop(layout);
}

void App::on_state_changed(const std::shared_ptr<ViewState>& state) {
    
}