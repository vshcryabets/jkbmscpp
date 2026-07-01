#include "ble/BleScannerNimBleImpl.h"

#include <Arduino.h> // for Serial

constexpr uint32_t BLE_SCAN_INTERVAL_MS = 5000;

BleScannerNimBleImpl::BleScannerNimBleImpl()
{
    runningMutex_ = xSemaphoreCreateMutex();
    xTaskCreate(
        BleScannerNimBleImpl::scanTaskEntry,
        "BleScanTask",
        6144,
        this,
        1,
        &scanHandle_);
}

void BleScannerNimBleImpl::init()
{
    NimBLEDevice::init("");
    auto bleScan = BLEDevice::getScan();
    bleScan->setScanCallbacks(this);
    Serial.println("BLE scanner initialized");
}

void BleScannerNimBleImpl::startScan(
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

void BleScannerNimBleImpl::stopScan()
{
    xTaskNotify(scanHandle_, CMD_STOP_SCAN, eSetValueWithOverwrite);
}


void BleScannerNimBleImpl::scanTaskEntry(void* parameter)
{
    auto* scanner = static_cast<BleScannerNimBleImpl*>(parameter);
    scanner->scanTaskLoop();
}

void BleScannerNimBleImpl::doScanStep(NimBLEScan* bleScan, uint8_t scanTimeSeconds)
{
    Serial.println("Starting BLE scan...");

    NimBLEScanResults scanResults = bleScan->getResults(scanTimeSeconds * 1000, false);
    const int deviceCount = scanResults.getCount();

    Serial.print("Devices found: ");
    Serial.println(deviceCount);

    std::vector<ScanResult> results;
    results.reserve(deviceCount);

    for (uint16_t i = 0; i < deviceCount; i++) {
        const NimBLEAdvertisedDevice* device = scanResults.getDevice(i);

        ScanResult result{};
        memcpy(result.address, device->getAddress().getBase()->val, sizeof(ble_addr_t::val));

        if (device->haveName()) {
            const String name = device->getName().c_str();
            strncpy(result.name, name.c_str(), sizeof(result.name) - 1);
            result.name[sizeof(result.name) - 1] = '\0';
        } else {
            result.name[0] = '\0';
        }

        result.rssi = static_cast<int8_t>(device->getRSSI());
        results.push_back(result);

        Serial.print("[");
        Serial.print(i + 1);
        Serial.print("] ");
        const String address = device->getAddress().toString().c_str();
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

void BleScannerNimBleImpl::scanTaskLoop()
{
    uint32_t notificationValue;
    while (true) {
        xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);
        if (notificationValue == CMD_START_SCAN) {
            setRunning(true);
            Serial.println("BLE scan start\n");
            NimBLEScan* bleScan = NimBLEDevice::getScan();
            bleScan->setScanCallbacks(this, true);
            bleScan->setActiveScan(true);

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

bool BleScannerNimBleImpl::isRunning()
{
    if (runningMutex_ == nullptr) {
        return running_;
    }

    xSemaphoreTake(runningMutex_, portMAX_DELAY);
    const bool running = running_;
    xSemaphoreGive(runningMutex_);
    return running;
}

void BleScannerNimBleImpl::setRunning(bool running)
{
    if (runningMutex_ == nullptr) {
        running_ = running;
        return;
    }

    xSemaphoreTake(runningMutex_, portMAX_DELAY);
    running_ = running;
    xSemaphoreGive(runningMutex_);
}

void BleScannerNimBleImpl::onResult(const NimBLEAdvertisedDevice* advertisedDevice)
{
    // Serial.print("Discovered device: ");
    // const String address = advertisedDevice->getAddress().toString().c_str();
    // Serial.print(address);
    // if (advertisedDevice->haveName()) {
    //     const String name = advertisedDevice->getName().c_str();
    //     Serial.print(" name=");
    //     Serial.print(name);
    // }
}