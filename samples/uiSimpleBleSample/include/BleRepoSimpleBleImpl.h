#pragma once

#include "BleRepo.h"
#include <simpleble/SimpleBLE.h>
#include <map>
#include <vector>
#include <string>

class BleRepoSimpleBleImpl : public BleRepo {
private:
    std::vector<SimpleBLE::Adapter> adapters;
    std::map<BleAdapterId, SimpleBLE::Adapter> adapterCache;
    std::map<PeripheralId, SimpleBLE::Peripheral> peripheralCache;
    BleAdapterId nextAdapterId = 0;
    PeripheralId nextPeripheralId = 0;
    
public:
    BleRepoSimpleBleImpl();
    virtual ~BleRepoSimpleBleImpl() = default;
    
    virtual std::vector<BleAdapterId> getAdapters() override;
    virtual std::string getAdapterName(BleAdapterId adapterId) override;
    virtual std::string getAdapterIdentifier(BleAdapterId adapterId) override;
    virtual void startScan(BleAdapterId adapterId) override;
    virtual void stopScan(BleAdapterId adapterId) override;
    virtual std::string getPeripheralName(PeripheralId peripheralId) override;
    virtual std::string getPeripheralIdentifier(PeripheralId peripheralId) override;
    virtual std::vector<PeripheralId> getScanResults() override;
};
