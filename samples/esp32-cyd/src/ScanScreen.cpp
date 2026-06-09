#include "ScanScreen.h"

void DrawScanScreen(Display& display, const ScanScreenViewState& viewState)
{
  const State* app = viewState.app;
  const ScanScreenUiState* ui = viewState.ui;

  if (app == nullptr || ui == nullptr) {
    return;
  }

  display.tft->fillScreen(TFT_WHITE);
  display.tft->setTextColor(TFT_BLACK, TFT_WHITE);
  display.tft->drawString("Scanning...", 16, 2, 2);

  display.tft->fillRect(0, 16, 240, 32, TFT_RED);

  display.tft->drawString("Devices: " + String(app->scanResults.size()), 7, 48, 2);
  display.tft->drawString("Offset: " + String(ui->listOffset), 7, 64, 2);

  display.tft->fillRect(0, 278, 240, 32, TFT_RED);
}