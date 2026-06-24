#include "screens/DetailsScreen.h"


void DrawDetailsScreen(Display& display, const DetailsScreenState& viewState)
{
  display.tft->fillScreen(TFT_WHITE);

  display.tft->fillRect(0, 16, 240, 32, TFT_RED);
  display.tft->setTextColor(TFT_BLACK, TFT_WHITE);
  display.tft->drawString(viewState.title, 12, 0, 2);
}

DetailsScreenTouchEventHandler::DetailsScreenTouchEventHandler(DetailsScreenViewModel& screenController)
    : viewModel(screenController)
{
}

void DetailsScreenTouchEventHandler::onEvent(TouchController::EventType eventType, int16_t x, int16_t y)
{
  switch (eventType) {
    case TouchController::EventType::Touch:
      break;
    case TouchController::EventType::Click:
      if (y < 650) {
        viewModel.onBack();
        hasUiUpdate = true;
      }
      break;
  }
}

bool DetailsScreenTouchEventHandler::consumeUiUpdateFlag()
{
  const bool hasUpdate = hasUiUpdate;
  hasUiUpdate = false;
  return hasUpdate;
}

DetailsScreenImpl::DetailsScreenImpl(Display &display, DetailsScreenViewModel& viewModel)
    : Screen(), touchImpl(viewModel), viewModel(viewModel), display(display)
{
}

void DetailsScreenImpl::begin(void *args)
{
  viewModel.begin(args);
}

void DetailsScreenImpl::end()
{
  viewModel.end();
}

void DetailsScreenImpl::draw()
{
  DrawDetailsScreen(display, viewModel.getStateCopy());
}

InputController* DetailsScreenImpl::getTouchHandler()
{
  return &touchImpl;
}