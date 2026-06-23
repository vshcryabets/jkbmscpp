#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "screens/ScanScreen.h"
#include "TouchController.h"
#include "BleScanner.h"
#include "viewmodels/ViewModel.h"
#include "viewmodels/ScanScreenViewModel.h"

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

class MainLoop {
public:
  enum class ScreenType {
    None,
    ScanScreen,
    DetailsScreen,
  };
private:
  UiStateObserver uiStateObserver;
  BleScanner bleScanner;
  StartScanUseCaseImpl startScanUseCase;
  StopScanUseCaseImpl stopScanUseCase;
  TouchController touchController;

  Screen *currentScreen = nullptr;
  ScreenType currentScreenType = ScreenType::None;

  ScanScreenImpl scanScreen;
  ScanScreenViewModel scanScreenViewModel;
public:
  MainLoop(): 
  startScanUseCase(bleScanner),
  stopScanUseCase(bleScanner),
  scanScreenViewModel(
    startScanUseCase,
    stopScanUseCase
  ),
  scanScreen(display, scanScreenViewModel),
  touchController({
    .clickMoveThresholdPx = 120,
    .maxClickDurationMs = 500,
  })
  {

  };
  void setup() {
    bleScanner.init();
    navigateToScanScreen();
  }

  void navigateToScanScreen() {
    if (currentScreen != nullptr) {
      touchController.setListener(nullptr);
      currentScreen->end();
    }
    currentScreen = &scanScreen;
    scanScreenViewModel.setObserver(&uiStateObserver);
    currentScreen->begin();
    currentScreenType = ScreenType::ScanScreen;
    touchController.setListener(currentScreen->getTouchHandler());
  }

  void loop() {
    bool shouldRender = uiStateObserver.consumeUpdateFlag();

    if (ts.tirqTouched() && ts.touched()) {
      TS_Point p = ts.getPoint();
      touchController.updateTouch(static_cast<int16_t>(p.x), static_cast<int16_t>(p.y));
    } else {
      touchController.updateNoTouch();
    }

    shouldRender = shouldRender || currentScreen->getTouchHandler()->consumeUiUpdateFlag();

    if (shouldRender) {
      currentScreen->draw();
      // switch (currentScreen) {
      //   case ScreenType::ScanScreen:
      //     drawScanScreen.draw(display, scanScreenViewModel.getStateCopy());
      //     break;
      //   case ScreenType::DetailsScreen:
      //     // DrawDetailsScreen(display, detailsScreenViewModel.getStateCopy());
      //     break;
      //   default:
      //     break;
      // }
    }
  }
};


MainLoop mainLoop;

void setup()
{
  Serial.begin(9600);
  delay(1000); // wait for serial monitor to open
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(0);

  tft.init();
  tft.begin(); 
  tft.setRotation(0);
  mainLoop.setup();
}

void loop()
{
  mainLoop.loop();
}
