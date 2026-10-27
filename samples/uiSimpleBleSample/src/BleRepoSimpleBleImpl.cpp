#include "BleRepoSimpleBleImpl.h"
#include <iostream>

BleRepoSimpleBleImpl::BleRepoSimpleBleImpl() {
    try {
        adapters = SimpleBLE::Adapter::get_adapters();
        // Cache all adapters with their IDs
        for (const auto& adapter : adapters) {
            adapterCache[nextAdapterId++] = adapter;
        }
        nextAdapterId = 0;  // Reset for getAdapters() to return consistent IDs
    } catch (const std::exception& e) {
        std::cerr << "Error getting BLE adapters: " << e.what() << std::endl;
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
        std::cout << "Started scanning on adapter " << adapterId << std::endl;
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
        std::cout << "Stopped scanning on adapter " << adapterId << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error stopping scan: " << e.what() << std::endl;
    }
}

std::string BleRepoSimpleBleImpl::getPeripheralName(PeripheralId peripheralId) {
    auto it = peripheralCache.find(peripheralId);
    if (it == peripheralCache.end()) {
        return "";
    }
    return it->second.identifier();
}

std::string BleRepoSimpleBleImpl::getPeripheralIdentifier(PeripheralId peripheralId) {
    auto it = peripheralCache.find(peripheralId);
    if (it == peripheralCache.end()) {
        return "";
    }
    return it->second.address();
}

std::vector<PeripheralId> BleRepoSimpleBleImpl::getScanResults() {
    std::vector<PeripheralId> results;
    
    if (adapterCache.empty()) {
        return results;
    }
    
    try {
        // Get first adapter from cache
        auto firstAdapter = adapterCache.begin()->second;
        auto peripherals_list = firstAdapter.get_peripherals();
        for (const auto& peripheral : peripherals_list) {
            PeripheralId id = nextPeripheralId++;
            peripheralCache[id] = peripheral;
            results.push_back(id);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting scan results: " << e.what() << std::endl;
    }
    
    return results;
}
