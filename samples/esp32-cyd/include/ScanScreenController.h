#pragma once

#include "ScanScreen.h"
#include "ViewModel.h"

class ScanScreenController {
public:
    explicit ScanScreenController(ViewModel& viewModel);

    const ScanScreenUiState& getUiState() const;

    void scrollDown();
    void scrollUp();
    void onDeviceSelected();

private:
    ViewModel& viewModel_;
    ScanScreenUiState uiState_{};
};
