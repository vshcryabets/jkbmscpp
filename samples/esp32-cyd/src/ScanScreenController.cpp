#include "ScanScreenController.h"

ScanScreenController::ScanScreenController(ViewModel& viewModel)
    : viewModel_(viewModel)
{
}

const ScanScreenUiState& ScanScreenController::getUiState() const
{
    return uiState_;
}

void ScanScreenController::scrollDown()
{
    uiState_.listOffset += 1;
}

void ScanScreenController::scrollUp()
{
    if (uiState_.listOffset > 0) {
        uiState_.listOffset -= 1;
    }
}

void ScanScreenController::onDeviceSelected()
{
    // const State appState = viewModel_.getStateCopy();
    // normalizeUiState(appState);

    // if (appState.scanResults.empty()) {
    //     return;
    // }

    // viewModel_.selectDevice(uiState_.selectedIndex);
}
