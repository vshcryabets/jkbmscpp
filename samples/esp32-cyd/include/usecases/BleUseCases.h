#pragma once

#include "ble/BleScanner.h"

class StartScanUseCase { 
public:
    StartScanUseCase() = default;
    ~StartScanUseCase() = default;

    virtual void execute(    
        uint8_t scanTimeSeconds,
        uint8_t scanPeriodSeconds,
        BleScanner::Listener* listener
    ) = 0;
};

class StopScanUseCase { 
public:
    StopScanUseCase() = default;
    ~StopScanUseCase() = default;

    virtual void execute() = 0;
};

class StartScanUseCaseImpl : public StartScanUseCase {
private:
        BleScanner& bleScanner;
public:
    StartScanUseCaseImpl(BleScanner& bleScanner)
        : bleScanner(bleScanner) {}
    void execute(
        uint8_t scanTimeSeconds,
        uint8_t scanPeriodSeconds,
        BleScanner::Listener* listener
    ) override;
};

class StopScanUseCaseImpl : public StopScanUseCase {
private:        
    BleScanner& bleScanner;
public:
    StopScanUseCaseImpl(BleScanner& bleScanner)
        : bleScanner(bleScanner) {}
    void execute() override;
};
