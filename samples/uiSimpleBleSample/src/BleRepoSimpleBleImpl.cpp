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
        
        // Find adapter
        auto adapterIt = adapterCache.find(adapterId);
        if (adapterIt == adapterCache.end()) {
            spdlog::error("Adapter not found: {}", adapterId);
            return;
        }
        
        // Find peripheral
        SimpleBLE::Peripheral* targetPeripheral = nullptr;
        try {
            auto peripherals = adapterIt->second.scan_get_results();
            for (auto& peripheral : peripherals) {
                if (peripheral.address() == peripheralId) {
                    targetPeripheral = &peripheral;
                    break;
                }
            }
        } catch (const std::exception& e) {
            spdlog::error("Error getting peripherals: {}", e.what());
            return;
        }
        
        if (targetPeripheral == nullptr) {
            spdlog::error("Peripheral not found: {}", peripheralId);
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
        
        // Create Source for this device
        auto source = std::make_unique<JkBmsSourceSimpleBleImpl>(*targetPeripheral, dumpLog);
        bmsSources[key] = std::move(source);
        
        // Create Controller for this device
        auto controller = std::make_unique<JkBmsCpp::Controller>(dumpLog);
        controller->start(bmsSources[key].get());
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
        
        // Remove source
        bmsSources.erase(key);
        
        // Remove state flow
        bmsStateFlows.erase(key);
        
        // Remove device info requested flag
        deviceInfoRequested.erase(key);
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
                
                // Request device info once
                if (!deviceInfoRequested[key]) {
                    auto deviceInfoFuture = controllerIt->second->readDeviceState();
                    
                    // Wait for result with timeout
                    auto status = deviceInfoFuture.wait_for(std::chrono::milliseconds(5000));
                    if (status == std::future_status::ready) {
                        deviceInfoRequested[key] = true;
                        auto result = deviceInfoFuture.get();
                        if (result.has_value()) {
                            const auto& deviceInfo = result.value();
                            JkbmsState state = flowIt->second->state();
                            state.vendorId = std::string(deviceInfo.vendorId.data());
                            state.hwVersion = std::string(deviceInfo.hwVersion.data());
                            state.swVersion = std::string(deviceInfo.swVersion.data());
                            state.uptimeSeconds = deviceInfo.uptimeSeconds;
                            state.deviceName = std::string(deviceInfo.deviceName.data());
                            state.serialNumber = std::string(deviceInfo.serialNumber.data());
                            flowIt->second->emit(state);
                            spdlog::info("Updated device info for adapter={}, peripheral={}: vendor={}, hw={}, sw={}", 
                                        adapterId, peripheralId, state.vendorId, state.hwVersion, state.swVersion);
                        } else {
                            spdlog::error("Failed to read device info: error code {}", 
                                        static_cast<int>(result.error()));
                        }
                    } else {
                        spdlog::warn("Device info request timed out for adapter={}, peripheral={}", 
                                    adapterId, peripheralId);
                    }
                }
                
            } catch (const std::exception& e) {
                spdlog::error("Error in connection worker for adapter={}, peripheral={}: {}", 
                            adapterId, peripheralId, e.what());
            }
        }
    }
}
