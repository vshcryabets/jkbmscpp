#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
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
    BleScanner();
    void init();
    void startScan(uint8_t scanTimeSeconds, uint8_t scanPeriodSeconds, Listener* listener);
    void stopScan();

private:
    static void scanTaskEntry(void* parameter);
    void scanTaskLoop();
    bool isRunning();
    void setRunning(bool running);

private:
    TaskHandle_t scanHandle_ = nullptr;
    uint8_t scanTimeSeconds_ = 3;
    uint8_t scanPeriodSeconds_ = 5;
    Listener* listener_ = nullptr;
    SemaphoreHandle_t runningMutex_ = nullptr;
    bool running_ = false;
};