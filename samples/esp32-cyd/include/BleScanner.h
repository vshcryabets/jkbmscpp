#pragma once
#include <Arduino.h>
#include <vector>
#include <cstdint>

class BleScanner {
public:
    struct ScanResult {
        char address[6];
        char name[32];
        int8_t rssi;
    };
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onDevicesScanned(std::vector<ScanResult>) = 0;
    };
public:
    void init();
    void startScan(uint8_t scanTimeSeconds, Listener* listener);
    void stopScan();

private:
    static void scanTaskEntry(void* parameter);
    void scanTaskLoop();

private:
    TaskHandle_t scanHandle_ = nullptr;
    uint8_t scanTimeSeconds_ = 3;
    Listener* listener_ = nullptr;
    bool running_ = false;
};