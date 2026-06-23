#pragma once

#include "Display.h"
#include "TouchController.h"

#include "viewmodels/ScanScreenViewModel.h"
#include "screens/Screen.h"


class ScanScreenTouchEventHandler : public InputController
{
public:
  explicit ScanScreenTouchEventHandler(ScanScreenViewModel &screenController);

  void onEvent(TouchController::EventType eventType, int16_t x, int16_t y) override;
  bool consumeUiUpdateFlag();

private:
  ScanScreenViewModel &viewModel;
  bool hasUiUpdate = false;
};

class ScanScreenImpl: public Screen
{
private:
  ScanScreenTouchEventHandler touchImpl;
  ScanScreenViewModel &viewModel;
  Display &display;
public:
  ScanScreenImpl(
    Display &display,
    ScanScreenViewModel &viewModel
  );
  void begin() override;
  void end() override;
  void draw() override;
  InputController* getTouchHandler() override;
};