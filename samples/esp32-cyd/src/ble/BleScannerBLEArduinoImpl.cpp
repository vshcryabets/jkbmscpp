#include "ble/BleScannerBLEArduinoImpl.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <cstring>

constexpr uint32_t BLE_SCAN_INTERVAL_MS = 5000;

BleScannerBLEArduinoImpl::BleScannerBLEArduinoImpl()
{
    runningMutex_ = xSemaphoreCreateMutex();
    xTaskCreate(
        BleScannerBLEArduinoImpl::scanTaskEntry,
        "BleScanTask",
        6144,
        this,
        1,
        &scanHandle_);
}

void BleScannerBLEArduinoImpl::init()
{
    BLEDevice::init("");
    Serial.println("BLE scanner initialized");
}

void BleScannerBLEArduinoImpl::startScan(
    uint8_t scanTimeSeconds,
    uint8_t scanPeriodSeconds,
    Listener* listener
) {
    if (isRunning()) {
        return;
    }
    scanTimeSeconds_ = scanTimeSeconds;
    scanPeriodSeconds_ = scanPeriodSeconds;
    listener_ = listener;
    xTaskNotify(scanHandle_, CMD_START_SCAN, eSetValueWithOverwrite);
}

void BleScannerBLEArduinoImpl::stopScan()
{
    xTaskNotify(scanHandle_, CMD_STOP_SCAN, eSetValueWithOverwrite);
}

void BleScannerBLEArduinoImpl::scanTaskEntry(void* parameter)
{
    auto* scanner = static_cast<BleScannerBLEArduinoImpl*>(parameter);
    scanner->scanTaskLoop();
}

void BleScannerBLEArduinoImpl::doScanStep(BLEScan* bleScan, uint8_t scanTimeSeconds)
{
    Serial.println("Starting BLE scan...");
    BLEScanResults scanResults = bleScan->start(scanTimeSeconds_, false);
    const int deviceCount = scanResults.getCount();

    Serial.print("Devices found: ");
    Serial.println(deviceCount);

    std::vector<ScanResult> results;
    results.reserve(deviceCount);

    for (uint16_t i = 0; i < deviceCount; i++) {
        BLEAdvertisedDevice device = scanResults.getDevice(i);

        ScanResult result{};
        memcpy(result.address, device.getAddress().getNative(), sizeof(result.address));

        if (device.haveName()) {
            const String name = device.getName().c_str();
            strncpy(result.name, name.c_str(), sizeof(result.name) - 1);
            result.name[sizeof(result.name) - 1] = '\0';
        } else {
            result.name[0] = '\0';
        }

        result.rssi = static_cast<int8_t>(device.getRSSI());
        results.push_back(result);

        Serial.print("[");
        Serial.print(i + 1);
        Serial.print("] ");
        const String address = device.getAddress().toString().c_str();
        Serial.print(address);
        if (result.name[0] != '\0') {
            Serial.print(" name=");
            Serial.print(result.name);
        }
        Serial.print(" rssi=");
        Serial.println(result.rssi);
    }

    if (listener_ != nullptr && isRunning()) {
        listener_->onDevicesScanned(results);
    }

    bleScan->clearResults();
    Serial.println("BLE scan complete\n");
}

void BleScannerBLEArduinoImpl::scanTaskLoop()
{
    uint32_t notificationValue;
    while (true) {
        xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);
        if (notificationValue == CMD_START_SCAN) {
            setRunning(true);
            Serial.println("BLE scan start\n");
            BLEScan* bleScan = BLEDevice::getScan();
            bleScan->setActiveScan(true);
            bleScan->setInterval(100);
            bleScan->setWindow(99);

            while (isRunning()) {
                doScanStep(bleScan, scanTimeSeconds_);

                TickType_t delayTicks = pdMS_TO_TICKS(scanPeriodSeconds_ * 1000);
                BaseType_t wasNotified = xTaskNotifyWait(
                    0x00,
                    ULONG_MAX,
                    &notificationValue,
                    delayTicks
                );
                if (wasNotified == pdTRUE) {
                    if (notificationValue == CMD_STOP_SCAN) {
                        Serial.println("BLE scan stop\n");
                        setRunning(false);
                        break;
                    }
                }
            }
        }
    }
    scanHandle_ = nullptr;
    vTaskDelete(nullptr);
}

bool BleScannerBLEArduinoImpl::isRunning()
{
    if (runningMutex_ == nullptr) {
        return running_;
    }

    xSemaphoreTake(runningMutex_, portMAX_DELAY);
    const bool running = running_;
    xSemaphoreGive(runningMutex_);
    return running;
}

void BleScannerBLEArduinoImpl::setRunning(bool running)
{
    if (runningMutex_ == nullptr) {
        running_ = running;
        return;
    }

    xSemaphoreTake(runningMutex_, portMAX_DELAY);
    running_ = running;
    xSemaphoreGive(runningMutex_);
}