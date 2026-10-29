#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include "stateflow.h"

using BleAdapterId = int16_t;
using PeripheralId = std::string;

struct Peripheral {
    PeripheralId id;
    std::string name;
    int rssi;

    bool operator==(const Peripheral& other) const {
        return id == other.id && 
            name == other.name && 
            rssi == other.rssi;
    }
};

struct JkbmsState {
    std::string vendorId;
    std::string hwVersion;
    std::string swVersion;
    uint32_t uptimeSeconds;
    std::string deviceName;
    std::string serialNumber;

    bool operator==(const JkbmsState& other) const {
        return vendorId == other.vendorId &&
               hwVersion == other.hwVersion &&
               swVersion == other.swVersion &&
               uptimeSeconds == other.uptimeSeconds &&
               deviceName == other.deviceName &&
               serialNumber == other.serialNumber;
    }

    bool operator!=(const JkbmsState& other) const {
        return !(*this == other);
    }
};

class BleRepo {
public:
    virtual ~BleRepo() = default;
    
    virtual std::vector<BleAdapterId> getAdapters() = 0;
    virtual std::string getAdapterName(BleAdapterId adapterId) = 0;
    virtual std::string getAdapterIdentifier(BleAdapterId adapterId) = 0;
    virtual void startScan(BleAdapterId adapterId) = 0;
    virtual void stopScan(BleAdapterId adapterId) = 0;
    virtual Peripheral getPeripheral(BleAdapterId adapterId, PeripheralId id) = 0;
    virtual std::vector<Peripheral> getScanResults(BleAdapterId adapterId) = 0;
    virtual flow::StateFlow<std::vector<Peripheral>>& getScanFlow(BleAdapterId adapterId) = 0;
    
    virtual void startConnection(BleAdapterId adapterId, PeripheralId peripheralId) = 0;
    virtual void stopConnection(BleAdapterId adapterId, PeripheralId peripheralId) = 0;
    virtual flow::StateFlow<JkbmsState>* getBmsState(BleAdapterId adapterId, PeripheralId peripheralId) = 0;
};
