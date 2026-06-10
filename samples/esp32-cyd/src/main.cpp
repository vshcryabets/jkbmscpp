#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "ScanScreen.h"
#include "TouchController.h"
#include "BleScanner.h"
#include "ViewModel.h"

TFT_eSPI tft = TFT_eSPI();
Display display(&tft);

class UiStateObserver : public ViewModel::Observer {
public:
  void onStateChanged() override
  {
    hasUpdate_ = true;
  }

  bool consumeUpdateFlag()
  {
    const bool hasUpdate = hasUpdate_;
    hasUpdate_ = false;
    return hasUpdate;
  }

private:
  volatile bool hasUpdate_ = true;
};

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// ----------------------------

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

BleScanner bleScanner;
ViewModel viewModel;
UiStateObserver uiStateObserver;
ScanScreenViewModel scanScreenViewModel(viewModel);
ScanScreenTouchEventHandler touchEventHandler(scanScreenViewModel);
TouchController touchController(&touchEventHandler, {
  .clickMoveThresholdPx = 120,
  .maxClickDurationMs = 500,
});

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

  viewModel.addObserver(&uiStateObserver);
  bleScanner.init();
  bleScanner.startScan(3, 10, &viewModel);
}

void loop()
{
  bool shouldRender = uiStateObserver.consumeUpdateFlag();

  if (ts.tirqTouched() && ts.touched()) {
    TS_Point p = ts.getPoint();
    touchController.updateTouch(static_cast<int16_t>(p.x), static_cast<int16_t>(p.y));
  } else {
    touchController.updateNoTouch();
  }

  shouldRender = shouldRender || touchEventHandler.consumeUiUpdateFlag();

  if (shouldRender) {
    const State appState = viewModel.getStateCopy();
    const ScanScreenViewState viewState { &appState, &scanScreenViewModel.getUiState() };
    DrawScanScreen(display, viewState);
  }
}
