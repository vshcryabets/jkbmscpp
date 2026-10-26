#include "SampleDl24App.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

using namespace ftxui;


SampleDl24App::SampleDl24App(ViewModel &viewModel) : 
    viewModel_(viewModel), 
    screen_(ScreenInteractive::Fullscreen()) {
    viewModel.setStateListener(this);
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

SampleDl24App::~SampleDl24App() {
    viewModel_.setStateListener(nullptr);
}

ftxui::Component SampleDl24App::connectToDeviceDialog() {
    this->device_path = viewModel_.getState().device_path;
    auto input_path = Input(&device_path, "e.g. /dev/ttyUSB0 or COM3");
    
    // Buttons for the dialog
    auto btn_connect = Button("Connect", [&] {
        viewModel_.onUiAction(Open(device_path));
    });
    
    auto btn_cancel = Button("Cancel", [&] {
        viewModel_.onUiAction(ConnectDialogCanceled());
    });

    // Group the interactive dialog elements together
    auto dialog_container = Container::Vertical({
        input_path,
        Container::Horizontal({btn_cancel, btn_connect}),
    });

    // Render how the dialog looks visually
    ftxui::Component dialog_renderer = Renderer(dialog_container, [=] {
        return window(text(" Enter UART Path "),
            vbox({
                hbox(text(" Path: "), input_path->Render()),
                separator(),
                hbox({
                    filler(), // Pushes buttons to the right
                    btn_cancel->Render(),
                    text(" "), // Small gap between buttons
                    btn_connect->Render(),
                })
            })
        ) | clear_under | center; 
        // clear_under prevents text from bleeding through the background
        // center puts the box right in the middle of the screen
    });
    return dialog_renderer;
}

void SampleDl24App::run() {
    // 1. Define State
    int menu_selected = 0;

    // 2. Create the Menu Component
    MenuOption option;
    option.on_enter = [&] {
        switch (menu_selected)
        {
        case 0:  // Connect / Disconnect
            viewModel_.onUiAction(OnConnectRequested());
            break;
        case 1:  // Save logs
            viewModel_.onUiAction(OnSaveLogsRequested());
            break;
        case 2:  // Test action
            viewModel_.onUiAction(TestActionRequested());
            break;
        case 3:  // Quit
            viewModel_.onUiAction(OnExitClicked());
            break;
        default:
            break;
        }
    };
    auto menu = Menu(&menu_entries_, &menu_selected, option);

    // 3. Create the Main Layout
    // We wrap the interactive menu in a Renderer to define how the screen is drawn
    auto layout = Renderer(menu, [&,&state = viewModel_.getState()] {
        // Build the UART log elements dynamically
        Elements log_elements;
        for (const auto& log : state.uart_logs) {
            log_elements.push_back(text(log));
        }

        // Create the UART output window
        // yframe and vscroll_indicator allow the box to handle overflowing text safely
        auto uart_window = window(text(" UART Output "),
            vbox(std::move(log_elements)) | vscroll_indicator | yframe
        ) | flex; // 'flex' makes this box take up all remaining horizontal space

        // Create the Menu window
        auto menu_window = window(text(" Menu "),
            menu->Render()
        ) | size(WIDTH, EQUAL, 25); // Fixed width of 25 characters

        // Combine them side-by-side (split screen)
        return hbox({
            menu_window,
            uart_window
        });
    });
    ftxui::Component dialog_renderer = connectToDeviceDialog();
    auto root = Modal(layout, dialog_renderer, &viewModel_.getState().show_connect_dialog);
    // 4. Run the application
    screen_.Loop(root);
}

void SampleDl24App::onStateChanged(const ViewState& state) {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        menu_entries_[0] = state.isConnected ? "Disconnect" : "Connect";
    }

    if (state.shouldCloseApp) {
        screen_.Post([&]() {
            screen_.Exit();
        });
    } else {
        screen_.PostEvent(ftxui::Event::Custom);
    }
}