#pragma once

#include "BleRepo.h"
#include <simpleble/SimpleBLE.h>
#include <JkBmsController.h>
#include "JkBmsSourceSimpleBleImpl.h"
#include <map>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <set>
#include <memory>

class BleRepoSimpleBleImpl : public BleRepo {
private:
    std::vector<SimpleBLE::Adapter> adapters;
    std::map<BleAdapterId, SimpleBLE::Adapter> adapterCache;
    BleAdapterId nextAdapterId = 0;
    
    std::map<BleAdapterId, flow::StateFlow<std::vector<Peripheral>>> scanFlows;
    std::thread scanWorkerThread;
    std::atomic<bool> workerRunning{false};
    std::atomic<int> scanIntervalMs{500};
    std::set<BleAdapterId> activeScans;
    mutable std::mutex activeScansLock;
    mutable std::mutex scanFlowsLock;
    
    // Connection management
    std::map<std::pair<BleAdapterId, PeripheralId>, std::shared_ptr<flow::StateFlow<JkbmsState>>> bmsStateFlows;
    std::map<std::pair<BleAdapterId, PeripheralId>, std::unique_ptr<JkBmsCpp::Controller>> bmsControllers;
    std::map<std::pair<BleAdapterId, PeripheralId>, std::unique_ptr<JkBmsCpp::Source>> bmsSources;
    std::map<std::pair<BleAdapterId, PeripheralId>, bool> deviceInfoRequested;
    std::thread connectionWorkerThread;
    mutable std::mutex connectionFlowsLock;
    
    JkBmsCpp::DumpLog dumpLog;
    
    void scanWorker();
    void connectionWorker();
    
public:
    BleRepoSimpleBleImpl(int scanIntervalMs = 500);
    virtual ~BleRepoSimpleBleImpl();
    
    virtual std::vector<BleAdapterId> getAdapters() override;
    virtual std::string getAdapterName(BleAdapterId adapterId) override;
    virtual std::string getAdapterIdentifier(BleAdapterId adapterId) override;
    virtual void startScan(BleAdapterId adapterId) override;
    virtual void stopScan(BleAdapterId adapterId) override;
    virtual Peripheral getPeripheral(BleAdapterId adapterId, PeripheralId id) override;
    virtual std::vector<Peripheral> getScanResults(BleAdapterId adapterId) override;
    virtual flow::StateFlow<std::vector<Peripheral>>& getScanFlow(BleAdapterId adapterId) override;
    
    virtual void startConnection(BleAdapterId adapterId, PeripheralId peripheralId) override;
    virtual void stopConnection(BleAdapterId adapterId, PeripheralId peripheralId) override;
    virtual flow::StateFlow<JkbmsState>* getBmsState(BleAdapterId adapterId, PeripheralId peripheralId) override;
};
