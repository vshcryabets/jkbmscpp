#pragma once

#include "Display.h"
#include "State.h"
#include "ViewModel.h"
#include "TouchController.h"

struct ScanScreenUiState {
  int listOffset = 0;
  int selectedIndex = 0;
};

struct ScanScreenViewState {
  const State* app;
  const ScanScreenUiState* ui;
};

class ScanScreenViewModel {
public:
    explicit ScanScreenViewModel(ViewModel& viewModel);

    const ScanScreenUiState& getUiState() const;

    void scrollDown();
    void scrollUp();
    void onDeviceSelected();

private:
    ViewModel& viewModel_;
    ScanScreenUiState uiState_{};
};

class ScanScreenTouchEventHandler : public TouchController::Listener {
public:
  explicit ScanScreenTouchEventHandler(ScanScreenViewModel& screenController);

  void onEvent(TouchController::EventType eventType, int16_t x, int16_t y) override;
  bool consumeUiUpdateFlag();

private:
  ScanScreenViewModel& viewModel;
  bool hasUiUpdate = false;
};

void DrawScanScreen(
    Display& display, 
    const ScanScreenViewState& viewState
);