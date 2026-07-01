#pragma once

#include "Display.h"
#include "TouchController.h"

#include "viewmodels/DetailsScreenViewModel.h"
#include "screens/Screen.h"


class DetailsScreenTouchEventHandler : public InputController
{
public:
  explicit DetailsScreenTouchEventHandler(DetailsScreenViewModel &screenController);

  void onEvent(TouchController::EventType eventType, int16_t x, int16_t y) override;
  bool consumeUiUpdateFlag();

private:
  DetailsScreenViewModel &viewModel;
  bool hasUiUpdate = false;
};

class DetailsScreenImpl: public Screen
{
private:
  DetailsScreenTouchEventHandler touchImpl;
  DetailsScreenViewModel &viewModel;
  Display &display;
public:
  DetailsScreenImpl(
    Display &display,
    DetailsScreenViewModel &viewModel
  );
  void begin(void *args) override;
  void end() override;
  void draw() override;
  InputController* getTouchHandler() override;
};