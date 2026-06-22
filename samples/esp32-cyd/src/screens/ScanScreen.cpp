#include "screens/ScanScreen.h"

void DrawScanScreen(Display& display, const ScanScreenViewState& viewState)
{
  display.tft->fillScreen(TFT_WHITE);

  display.tft->fillRect(0, 16, 240, 32, TFT_RED);
  uint16_t size = viewState.items.size();
  display.tft->setTextColor(TFT_BLACK, TFT_RED);
  display.tft->drawString(String(viewState.listOffset), 7, 24, 2);
  display.tft->drawString(String(size), 27, 24, 2);
  display.tft->setTextColor(TFT_BLACK, TFT_WHITE);
  display.tft->drawString("Scanning...", 12, 0, 2);
  for (int i = viewState.listOffset; i < size; ++i) {
    const int16_t y = 48 + (i - viewState.listOffset) * 32;
    if (y > 270) {
      break;
    }
    display.tft->drawLine(0, y, 240, y, TFT_LIGHTGREY);
    display.tft->setTextColor(TFT_BLACK, TFT_WHITE);
    display.tft->drawString(viewState.items[i].title, 7, y + 1, 2);
    display.tft->setTextColor(TFT_DARKGREY, TFT_WHITE);
    display.tft->drawString(viewState.items[i].subtitle, 7, y + 16, 2);
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
