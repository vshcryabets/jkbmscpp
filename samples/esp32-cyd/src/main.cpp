#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "screens/ScanScreen.h"
#include "TouchController.h"
#include "ble/BleScanner.h"
#include "ble/BleScannerBLEArduinoImpl.h"
#include "viewmodels/ViewModel.h"
#include "viewmodels/ScanScreenViewModel.h"
#include "viewmodels/DetailsScreenViewModel.h"

#include "screens/DetailsScreen.h"

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

private:
  UiStateObserver uiStateObserver;
  BleScannerBLEArduinoImpl bleScanner;
  StartScanUseCaseImpl startScanUseCase;
  StopScanUseCaseImpl stopScanUseCase;
  TouchController touchController;

  Screen *currentScreen = nullptr;
  ScreenType currentScreenType = ScreenType::None;

  ScanScreenImpl scanScreen;
  ScanScreenViewModel scanScreenViewModel;

  DetailsScreenImpl detailsScreen;
  DetailsScreenViewModel detailsScreenViewModel;
  FreeRTOSNavigationController navigationController;

  NavigationEvent navigationEvent;
public:
  MainLoop(): 
  startScanUseCase(bleScanner),
  stopScanUseCase(bleScanner),
  scanScreenViewModel(
    startScanUseCase,
    stopScanUseCase,
    navigationController
  ),
  scanScreen(display, scanScreenViewModel),
  touchController({
    .clickMoveThresholdPx = 120,
    .maxClickDurationMs = 500,
  }),
  detailsScreenViewModel(navigationController),
  detailsScreen(display, detailsScreenViewModel)
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
    currentScreen->begin(nullptr);
    currentScreenType = ScreenType::ScanScreen;
    touchController.setListener(currentScreen->getTouchHandler());
  }

  void navigateToDetailsScreen(DetailsScreenArgs args) {
    if (currentScreen != nullptr) {
      touchController.setListener(nullptr);
      currentScreen->end();
    }
    currentScreen = &detailsScreen;
    detailsScreenViewModel.setObserver(&uiStateObserver);
    detailsScreenViewModel.withStateLock([&args](DetailsScreenState &state) {
      snprintf(state.title, sizeof(state.title), "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
               args.macAddress[0], args.macAddress[1], args.macAddress[2],
               args.macAddress[3], args.macAddress[4], args.macAddress[5]);
    });
    currentScreen->begin(&args);
    currentScreenType = ScreenType::DetailsScreen;
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
    }

    if (navigationController.readEvent(navigationEvent, 0)) {
      switch (navigationEvent.targetScreen) {
        case ScreenType::ScanScreen:
          navigateToScanScreen();
          break;
        case ScreenType::DetailsScreen:
          navigateToDetailsScreen(navigationEvent.args);
          break;
        default:
          break;
      }
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
