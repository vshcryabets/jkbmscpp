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

  // display.tft->drawString("Devices: " + String(app->scanResults.size()), 7, 16, 2);
  // display.tft->drawString("Offset: " + String(ui->listOffset), 7, 32, 2);

  int16_t size = app->scanResults.size();
  for (int i = ui->listOffset; i < size; ++i) {
    const int16_t y = 64 + (i - ui->listOffset) * 16;
    if (y > 270) {
      break;
    }
    const BleScanner::ScanResult& result = app->scanResults[i];

    char addressSrt[19];
    snprintf(addressSrt, 
      sizeof(addressSrt), 
      "%02x:%02x:%02x:%02x:%02x:%02x", 
      result.address[0], 
      result.address[1], 
      result.address[2], 
      result.address[3], 
      result.address[4], 
      result.address[5]
    );

    display.tft->drawString(String(addressSrt) + " " + String(result.name), 7, y, 2);
  }

  display.tft->fillRect(0, 278, 240, 32, TFT_RED);
}

ScanScreenTouchEventHandler::ScanScreenTouchEventHandler(ScanScreenViewModel& screenController)
    : viewModel(screenController)
{
}

void ScanScreenTouchEventHandler::onEvent(TouchController::EventType eventType, int16_t x, int16_t y)
{
  switch (eventType) {
    case TouchController::EventType::Touch:
      break;
    case TouchController::EventType::Click:
      Serial.print("click ");
      Serial.println(y);
      if (y < 650) {
        viewModel.scrollUp();
        hasUiUpdate = true;
      } else if (y > 3200) {
        viewModel.scrollDown();
        hasUiUpdate = true;
      }
      break;
    case TouchController::EventType::NoTouch:
      // Placeholder hook for ending drag/press interactions.
      break;
    case TouchController::EventType::OnDown:
      Serial.print("down ");
      Serial.println(y);
      break;
    case TouchController::EventType::OnUp:
      Serial.print("up ");
      Serial.println(y);
      break;
  }
}

bool ScanScreenTouchEventHandler::consumeUiUpdateFlag()
{
  const bool hasUpdate = hasUiUpdate;
  hasUiUpdate = false;
  return hasUpdate;
}
