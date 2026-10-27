#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "stateflow.h"

using BleAdapterId = int16_t;
using PeripheralId = int32_t;

struct Peripheral {
    PeripheralId id;
    std::string name;
    int rssi;
    std::string identifier;
    std::string address;

    bool operator==(const Peripheral& other) const {
        return id == other.id && 
            name == other.name && 
            rssi == other.rssi &&
            identifier == other.identifier &&
            address == other.address;
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
};
