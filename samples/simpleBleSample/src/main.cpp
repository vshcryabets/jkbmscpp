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

        do {
            std::cout << "Connected! Staying connected for 15 seconds..." << std::endl;
            auto deviceInfo = controller.readDeviceState().get();

            if (!deviceInfo.hasValue()) {
                std::cerr << "Error parsing device info: "
                    << static_cast<uint8_t>(deviceInfo.error()) << std::endl;
                break;
            } else {
                std::cout << "Device Info:" << std::endl;
                std::cout << "  Vendor ID: " << deviceInfo.value().vendorId << std::endl;
                std::cout << "  HW Version: " << deviceInfo.value().hwVersion << std::endl;
                std::cout << "  SW Version: " << deviceInfo.value().swVersion << std::endl;
                std::cout << "  Uptime (s): " << deviceInfo.value().uptimeSeconds << std::endl;
                std::cout << "  Power On Counter: " << deviceInfo.value().powerOnCounter << std::endl;
                std::cout << "  Device Name: " << deviceInfo.value().deviceName << std::endl;
                std::cout << "  Serial Number: " << deviceInfo.value().serialNumber << std::endl;
            }

            for (int i = 0; i < 10; i++) {
                auto timeout = std::chrono::seconds(5);  // Increased timeout
                std::this_thread::sleep_for(std::chrono::seconds(3));  // Increased delay
                
                std::cout << "Read cells " << (i + 1) << std::endl;
                auto fut = controller.readCellsState();
                std::future_status status = fut.wait_for(timeout);

                if (status ==  std::future_status::ready) {
                    // Result is ready, safe to call get()
                    auto cellsInfo = fut.get();

                    std::cout << "Cells Info:" << std::endl;
                    for (size_t cell = 0; cell < JkBmsCpp::JkBmsCellInfoResponse::CELL_COUNT; cell++) {
                        if (cellsInfo.value().cellVoltages_mV[cell] == 0) {
                            break;
                        }
                        std::cout << "  Cell " << (cell + 1) << ": " 
                            << cellsInfo.value().cellVoltages_mV[cell] << " mV" << std::endl;
                    }
                    std::cout << "  Battery Voltage: " 
                        << cellsInfo.value().batteryVoltage_mV << " mV" << std::endl;
                    std::cout << "  Battery Power: " 
                        << cellsInfo.value().batteryPower_mW << " mW" << std::endl;
                    std::cout << "  Charge Current: " 
                        << cellsInfo.value().chargeCurrent_mA << " mA" << std::endl;
                    std::cout << "  Temperature Sensor 1: " 
                        << cellsInfo.value().temperatureSensor1/10.0f << " °C" << std::endl;
                    std::cout << "  Temperature Sensor 2: " 
                        << cellsInfo.value().temperatureSensor2/10.0f << " °C" << std::endl;
                    std::cout << "  Balance Current: " 
                        << cellsInfo.value().balanceCurrent_mA << " mA action = " 
                        << (int)cellsInfo.value().balanceAction << std::endl;
                    std::cout << "  State of Charge: " 
                        << (int)cellsInfo.value().batteryPercentage << " %" << std::endl;
                    break;
                } else if (status == std::future_status::timeout) {
                    std::cout << "Timeout occurred! The task is still running.\n";
                } else if (status == std::future_status::deferred) {
                    std::cout << "Task is deferred (not started yet). call get() to run it synchronously.\n";
                } else {
                    std::cout << "Unknown future status.\n";
                }
            }            
        } while(false);        

        controller.end();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}