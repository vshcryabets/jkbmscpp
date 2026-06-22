#pragma once

#include "Display.h"
#include "TouchController.h"

#include "viewmodels/ScanScreenViewModel.h"

class ScanScreenTouchEventHandler : public TouchController::Listener
{
public:
  explicit ScanScreenTouchEventHandler(ScanScreenViewModel &screenController);

  void onEvent(TouchController::EventType eventType, int16_t x, int16_t y) override;
  bool consumeUiUpdateFlag();

private:
  ScanScreenViewModel &viewModel;
  bool hasUiUpdate = false;
};

void DrawScanScreen(
    Display &display,
    const ScanScreenViewState &viewState);