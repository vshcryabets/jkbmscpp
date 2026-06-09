#pragma once

#include "Display.h"
#include "State.h"

struct ScanScreenUiState {
  int listOffset = 0;
  int selectedIndex = 0;
};

struct ScanScreenViewState {
  const State* app;
  const ScanScreenUiState* ui;
};

void DrawScanScreen(
    Display& display, 
    const ScanScreenViewState& viewState
);