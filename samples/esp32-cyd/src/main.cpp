#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "ScanScreen.h"
#include "BleScanner.h"

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

BleScanner bleScanner;

void setup()
{
  Serial.begin(9600);
  delay(1000); // wait for serial monitor to open
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(0);

  Serial.println("Hello, World!2");

  tft.init();
  tft.begin(); 
  tft.setRotation(0);

  bleScanner.init();
  bleScanner.startScan(3, nullptr);
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

  if (state.isDirty) {
    DrawScanScreen(display, state);
    state.isDirty = false;
  }
}
