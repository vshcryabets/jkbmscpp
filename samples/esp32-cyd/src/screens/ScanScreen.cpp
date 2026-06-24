#include "screens/ScanScreen.h"


void DrawScanScreen(Display& display, const ScanScreenState& viewState)
{
  display.tft->fillScreen(TFT_WHITE);

  display.tft->fillRect(0, 16, 240, 32, TFT_RED);
  const uint16_t size = viewState.itemCount;
  display.tft->setTextColor(TFT_BLACK, TFT_RED);
  display.tft->drawString(String(viewState.listOffset), 7, 24, 2);
  display.tft->drawString(String(size), 27, 24, 2);
  display.tft->setTextColor(TFT_BLACK, TFT_WHITE);
  display.tft->drawString("Scanning...", 12, 0, 2);
  UiLabel label;
  for (int i = viewState.listOffset; i < size; ++i) {
    const int16_t y = 48 + (i - viewState.listOffset) * 32;
    if (y > 270) {
      break;
    }
    display.tft->drawLine(0, y, 240, y, TFT_LIGHTGREY);
    display.tft->setTextColor(TFT_BLACK, TFT_WHITE);
    viewState.itemProvider->getItem(i, label);
    display.tft->drawString(label.title, 7, y + 1, 2);
    display.tft->setTextColor(TFT_DARKGREY, TFT_WHITE);
    display.tft->drawString(label.subtitle, 7, y + 16, 2);
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

ScanScreenImpl::ScanScreenImpl(Display &display, ScanScreenViewModel& viewModel)
    : Screen(), touchImpl(viewModel), viewModel(viewModel), display(display)
{
}

void ScanScreenImpl::begin(void *args)
{
  viewModel.begin(args);
}

void ScanScreenImpl::end()
{
  viewModel.end();
}

void ScanScreenImpl::draw()
{
  DrawScanScreen(display, viewModel.getStateCopy());
}

InputController* ScanScreenImpl::getTouchHandler()
{
  return &touchImpl;
}