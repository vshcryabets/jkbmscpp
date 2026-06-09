#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "ScanScreen.h"

TFT_eSPI tft = TFT_eSPI();
Display display(&tft);
State state;

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// ----------------------------

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

BLEScan *bleScan = nullptr;
constexpr uint32_t BLE_SCAN_INTERVAL_MS = 5000;
constexpr uint32_t BLE_SCAN_TIME_SECONDS = 3;
uint32_t lastBleScanMs = 0;


void testLoop(void *parameter)
{
  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("Looping...");
  }
}

void setup()
{
  Serial.begin(9600);
  delay(1000); // wait for serial monitor to open
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(0);

  Serial.println("Hello, World!2");
  // xTaskCreate(
  //     testLoop,
  //     "PrintTask",
  //     1024,
  //     NULL,
  //     1,
  //     NULL);



  tft.init();
  tft.begin(); 
  tft.setRotation(0);

  Serial.println("A01");


  BLEDevice::init("");
  bleScan = BLEDevice::getScan();
  bleScan->setActiveScan(true);
  Serial.println("A02");
  bleScan->setInterval(100);
  bleScan->setWindow(99);
Serial.println("A03");
  Serial.println("BLE scanner initialized2");
}


void printTouchToSerial(TS_Point p) {
  Serial.print("Pressure = ");
  Serial.print(p.z);
  Serial.print(", x = ");
  Serial.print(p.x);
  Serial.print(", y = ");
  Serial.print(p.y);
  Serial.println();
}

void loop()
{
  if (ts.tirqTouched() && ts.touched()) {
    TS_Point p = ts.getPoint();
    printTouchToSerial(p);
  }

  const uint32_t nowMs = millis();
  if (bleScan != nullptr && (nowMs - lastBleScanMs >= BLE_SCAN_INTERVAL_MS)) {
    lastBleScanMs = nowMs;
    Serial.println("Starting BLE scan...");

    BLEScanResults scanResults = bleScan->start(BLE_SCAN_TIME_SECONDS, false);
    const int deviceCount = scanResults.getCount();

    Serial.print("Devices found: ");
    Serial.println(deviceCount);

    for (int i = 0; i < deviceCount; i++) {
      BLEAdvertisedDevice device = scanResults.getDevice(i);
      Serial.print("[");
      Serial.print(i + 1);
      Serial.print("] ");
      Serial.print(device.getAddress().toString().c_str());

      if (device.haveName()) {
        Serial.print(" name=");
        Serial.print(device.getName().c_str());
      }

      Serial.print(" rssi=");
      Serial.println(device.getRSSI());
    }

    bleScan->clearResults();
    Serial.println("BLE scan complete\n");
  }

  if (state.isDirty) {
    DrawScanScreen(display, state);
    state.isDirty = false;
  }
}
