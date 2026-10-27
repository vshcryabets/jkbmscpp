#pragma once

#include <vector>
#include <string>
#include <cstdint>

using BleAdapterId = int16_t;
using PeripheralId = int32_t;

class BleRepo {
public:
    virtual ~BleRepo() = default;
    
    virtual std::vector<BleAdapterId> getAdapters() = 0;
    virtual std::string getAdapterName(BleAdapterId adapterId) = 0;
    virtual std::string getAdapterIdentifier(BleAdapterId adapterId) = 0;
    virtual void startScan(BleAdapterId adapterId) = 0;
    virtual void stopScan(BleAdapterId adapterId) = 0;
    virtual std::string getPeripheralName(PeripheralId peripheralId) = 0;
    virtual std::string getPeripheralIdentifier(PeripheralId peripheralId) = 0;
    virtual std::vector<PeripheralId> getScanResults() = 0;
};
