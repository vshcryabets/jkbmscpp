#include "BleRepoSimpleBleImpl.h"
#include <iostream>
#include <spdlog/spdlog.h>

BleRepoSimpleBleImpl::BleRepoSimpleBleImpl(int scanIntervalMs) : scanIntervalMs(scanIntervalMs) {
    try {
        adapters = SimpleBLE::Adapter::get_adapters();
        // Cache all adapters with their IDs
        for (const auto& adapter : adapters) {
            adapterCache[nextAdapterId++] = adapter;
        }
        nextAdapterId = 0;  // Reset for getAdapters() to return consistent IDs
        
        // Start worker thread
        workerRunning = true;
        scanWorkerThread = std::thread(&BleRepoSimpleBleImpl::scanWorker, this);
    } catch (const std::exception& e) {
        std::cerr << "Error getting BLE adapters: " << e.what() << std::endl;
    }
}

BleRepoSimpleBleImpl::~BleRepoSimpleBleImpl() {
    workerRunning = false;
    if (scanWorkerThread.joinable()) {
        scanWorkerThread.join();
    }
    if (connectionWorkerThread.joinable()) {
        connectionWorkerThread.join();
    }
}

std::vector<BleAdapterId> BleRepoSimpleBleImpl::getAdapters() {
    std::vector<BleAdapterId> adapterIds;
    for (const auto& entry : adapterCache) {
        adapterIds.push_back(entry.first);
    }
    return adapterIds;
}

std::string BleRepoSimpleBleImpl::getAdapterName(BleAdapterId adapterId) {
    auto it = adapterCache.find(adapterId);
    if (it == adapterCache.end()) {
        return "";
    }
    return it->second.identifier();
}

std::string BleRepoSimpleBleImpl::getAdapterIdentifier(BleAdapterId adapterId) {
    auto it = adapterCache.find(adapterId);
    if (it == adapterCache.end()) {
        return "";
    }
    return it->second.identifier();
}

void BleRepoSimpleBleImpl::startScan(BleAdapterId adapterId) {
    auto it = adapterCache.find(adapterId);
    if (it == adapterCache.end()) {
        std::cerr << "Invalid adapter ID: " << adapterId << std::endl;
        return;
    }
    
    try {
        it->second.scan_start();
        spdlog::info("Started scanning on adapter {}", adapterId);
        
        // Create StateFlow for this adapter if it doesn't exist
        {
            std::lock_guard<std::mutex> lock(scanFlowsLock);
            if (scanFlows.find(adapterId) == scanFlows.end()) {
                scanFlows.emplace(adapterId, std::vector<Peripheral>{});
            }
        }
        
        // Add to active scans
        {
            std::lock_guard<std::mutex> lock(activeScansLock);
            activeScans.insert(adapterId);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error starting scan: " << e.what() << std::endl;
    }
}

void BleRepoSimpleBleImpl::stopScan(BleAdapterId adapterId) {
    auto it = adapterCache.find(adapterId);
    if (it == adapterCache.end()) {
        std::cerr << "Invalid adapter ID: " << adapterId << std::endl;
        return;
    }
    
    try {
        it->second.scan_stop();
        spdlog::info("Stopped scanning on adapter {}", adapterId);
        
        // Remove from active scans
        {
            std::lock_guard<std::mutex> lock(activeScansLock);
            activeScans.erase(adapterId);
        }
        
        // Remove StateFlow for this adapter
        {
            std::lock_guard<std::mutex> lock(scanFlowsLock);
            scanFlows.erase(adapterId);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error stopping scan: " << e.what() << std::endl;
    }
}

Peripheral BleRepoSimpleBleImpl::getPeripheral(BleAdapterId adapterId, PeripheralId id) {
    std::lock_guard<std::mutex> lock(scanFlowsLock);
    auto flowIt = scanFlows.find(adapterId);
    if (flowIt == scanFlows.end()) {
        return Peripheral{id, "", 0};
    } else {
        auto peripherals = flowIt->second.state();
        for (const auto& peripheral : peripherals) {
            if (peripheral.id == id) {
                return peripheral;
            }
        }
        return Peripheral{id, "", 0};
    }
}

std::vector<Peripheral> BleRepoSimpleBleImpl::getScanResults(BleAdapterId adapterId) {
    std::lock_guard<std::mutex> lock(scanFlowsLock);
    auto flowIt = scanFlows.find(adapterId);
    if (flowIt == scanFlows.end()) {
        return {};
    } else {
        return flowIt->second.state();
    }
}

flow::StateFlow<std::vector<Peripheral>>& BleRepoSimpleBleImpl::getScanFlow(BleAdapterId adapterId) {
    std::lock_guard<std::mutex> lock(scanFlowsLock);
    auto it = scanFlows.find(adapterId);
    if (it == scanFlows.end()) {
        scanFlows.emplace(adapterId, std::vector<Peripheral>{});
        it = scanFlows.find(adapterId);
    }
    return it->second;
}

void BleRepoSimpleBleImpl::scanWorker() {
    while (workerRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(scanIntervalMs.load()));
        
        std::set<BleAdapterId> activeAdapters;
        {
            std::lock_guard<std::mutex> lock(activeScansLock);
            activeAdapters = activeScans;
        }
        
        for (BleAdapterId adapterId : activeAdapters) {
            try {
    
                auto it = adapterCache.find(adapterId);
                if (it == adapterCache.end()) {
                    continue;
                }
                std::vector<Peripheral> results;

                try {
                    auto& adapter = it->second;
                    auto peripherals_list = adapter.scan_get_results();
                    for (auto& peripheral : peripherals_list) {
                        results.push_back( Peripheral{
                            peripheral.address(),
                            peripheral.identifier(),
                            peripheral.rssi()
                        });
                    }
                } catch (const std::exception& e) {
                    spdlog::error("Error getting scan results for adapter {}: {}", adapterId, e.what());
                }
            
                {
                    std::lock_guard<std::mutex> lock(scanFlowsLock);
                    auto flowIt = scanFlows.find(adapterId);
                    if (flowIt != scanFlows.end()) {
                        flowIt->second.emit(results);
                    }
                }
            } catch (const std::exception& e) {
                spdlog::error("Error scanning adapter {}: {}", adapterId, e.what());
            }
        }
    }
}

void BleRepoSimpleBleImpl::startConnection(
    BleAdapterId adapterId, 
    PeripheralId peripheralId) {
    auto key = std::make_pair(adapterId, peripheralId);
    
    {
        std::lock_guard<std::mutex> lock(connectionFlowsLock);
        
        // Check if already connected
        if (bmsStateFlows.find(key) != bmsStateFlows.end()) {
            spdlog::warn("Device already connected: adapter={}, peripheral={}", adapterId, peripheralId);
            return;
        }
        
        // Create initial BMS state
        JkbmsState initialState{
            .vendorId = "",
            .hwVersion = "",
            .swVersion = "",
            .uptimeSeconds = 0,
            .deviceName = "",
            .serialNumber = ""
        };
        
        // Create StateFlow for this device with initial state
        auto stateFlow = std::make_shared<flow::StateFlow<JkbmsState>>(initialState);
        bmsStateFlows[key] = stateFlow;
        
        // Create Controller for this device
        auto controller = std::make_unique<JkBmsCpp::Controller>();
        auto source = std::make_unique
        controller->start(

        );
        bmsControllers[key] = std::move(controller);
    }
    
    spdlog::info("Started connection to device: adapter={}, peripheral={}", adapterId, peripheralId);
    
    // Start connection worker if not already running
    if (!connectionWorkerThread.joinable()) {
        connectionWorkerThread = std::thread(&BleRepoSimpleBleImpl::connectionWorker, this);
    }
}

void BleRepoSimpleBleImpl::stopConnection(BleAdapterId adapterId, PeripheralId peripheralId) {
    auto key = std::make_pair(adapterId, peripheralId);
    
    {
        std::lock_guard<std::mutex> lock(connectionFlowsLock);
        
        // Stop controller
        auto controllerIt = bmsControllers.find(key);
        if (controllerIt != bmsControllers.end()) {
            controllerIt->second->end();
            bmsControllers.erase(controllerIt);
        }
        
        // Remove state flow
        bmsStateFlows.erase(key);
    }
    
    spdlog::info("Stopped connection to device: adapter={}, peripheral={}", adapterId, peripheralId);
}

flow::StateFlow<JkbmsState>* BleRepoSimpleBleImpl::getBmsState(BleAdapterId adapterId, PeripheralId peripheralId) {
    auto key = std::make_pair(adapterId, peripheralId);
    
    std::lock_guard<std::mutex> lock(connectionFlowsLock);
    auto it = bmsStateFlows.find(key);
    if (it != bmsStateFlows.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

void BleRepoSimpleBleImpl::connectionWorker() {
    while (workerRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::vector<std::pair<BleAdapterId, PeripheralId>> activeConnections;
        {
            std::lock_guard<std::mutex> lock(connectionFlowsLock);
            for (const auto& [key, flow] : bmsStateFlows) {
                activeConnections.push_back(key);
            }
        }
        
        // Poll each connection
        for (const auto& [adapterId, peripheralId] : activeConnections) {
            try {
                auto key = std::make_pair(adapterId, peripheralId);
                
                std::lock_guard<std::mutex> lock(connectionFlowsLock);
                
                auto controllerIt = bmsControllers.find(key);
                auto flowIt = bmsStateFlows.find(key);
                
                if (controllerIt == bmsControllers.end() || flowIt == bmsStateFlows.end()) {
                    continue;
                }
                
                // TODO: Read device state from controller and emit to StateFlow
                // auto deviceInfoFuture = controllerIt->second->readDeviceState();
                // Wait for result and update StateFlow
                
            } catch (const std::exception& e) {
                spdlog::error("Error in connection worker for adapter={}, peripheral={}: {}", 
                            adapterId, peripheralId, e.what());
            }
        }
    }
}
