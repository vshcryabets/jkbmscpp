#pragma once

#include <vector>
#include <cstdint>

class BleScanner {
public:
    struct ScanResult {
        uint8_t address[6];
        char name[32];
        int8_t rssi;
    };
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onDevicesScanned(const std::vector<ScanResult>& results) = 0;
    };
public:
    BleScanner() = default;
    virtual ~BleScanner() = default;
    virtual void init() = 0;
    virtual void startScan(uint8_t scanTimeSeconds, uint8_t scanPeriodSeconds, Listener* listener) = 0;
    virtual void stopScan() = 0;
};
