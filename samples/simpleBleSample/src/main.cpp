#include <simpleble/SimpleBLE.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "JkBmsSourceSimpleBleImpl.h"
#include "JkBmsController.h"

int main() {
    std::cout << "SimpleBLE Test Starting..." << std::endl;
    
    try {
        // Check if any adapters are available
        auto adapters = SimpleBLE::Adapter::get_adapters();
        
        if (adapters.empty()) {
            std::cout << "No Bluetooth adapters found!" << std::endl;
            return 1;
        }
        
        std::cout << "Found " << adapters.size() << " Bluetooth adapter(s):" << std::endl;
        
        for (size_t i = 0; i < adapters.size(); i++) {
            std::cout << i << "). " << adapters[i].identifier() << " - " 
                      << adapters[i].address() << std::endl;
        }
        
        // Use the first available adapter
        auto adapter = adapters[0];
        
        std::cout << "Using adapter: " << adapter.identifier() << std::endl;
        
        // Start scanning for 5 seconds
        std::cout << "Starting BLE scan for 5 seconds..." << std::endl;
        
        adapter.scan_start();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        adapter.scan_stop();
        
        // Get scan results
        auto peripherals = adapter.scan_get_results();
        
        std::cout << "Found " << peripherals.size() << " BLE device(s):" << std::endl;

        std::cout << "\t0). Exit" << std::endl;
        int autoJkBms = 0;
        for (size_t i = 0; i < peripherals.size(); i++) {
            auto& peripheral = peripherals[i];
            std::string name = peripheral.identifier().empty() ? "Unknown" : peripheral.identifier();
            std::cout << "\t" << i + 1 << "). " << name 
                << " (" << peripheral.address() << ")" 
                << std::endl;
            if (name.find("JK_BD") != std::string::npos) {
                autoJkBms = static_cast<int>(i + 1);
            }
        }

        std::cout << "Select a device to connect to (0 to exit): ";
        size_t choice = autoJkBms;
        // std::cin >> choice;
        if (choice < 1) {
            std::cout << "Exiting..." << std::endl;
            return 0;
        }
        if (choice > peripherals.size()) {
            std::cout << "Invalid choice!" << std::endl;
            return 1;
        }
        auto& selected_peripheral = peripherals[choice - 1];
        std::cout << "Connecting to " << selected_peripheral.identifier() << "..." << std::endl;

        JkBmsSourceSimpleBleImpl source(selected_peripheral);
        JkBmsCpp::JkBmsController controller;
        
        controller.start(&source);
        std::cout << "Connected! Staying connected for 15 seconds..." << std::endl;
        controller.readDeviceState();
        std::this_thread::sleep_for(std::chrono::seconds(15));
        controller.end();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}