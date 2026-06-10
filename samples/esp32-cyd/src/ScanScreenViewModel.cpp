#include "ScanScreen.h"

ScanScreenViewModel::ScanScreenViewModel(ViewModel& viewModel)
    : viewModel_(viewModel)
{
}

const ScanScreenUiState& ScanScreenViewModel::getUiState() const
{
    return uiState_;
}

void ScanScreenViewModel::scrollDown()
{
    uiState_.listOffset += 1;
}

void ScanScreenViewModel::scrollUp()
{
    if (uiState_.listOffset > 0) {
        uiState_.listOffset -= 1;
    }
}

void ScanScreenViewModel::onDeviceSelected()
{
    // const State appState = viewModel_.getStateCopy();
    // normalizeUiState(appState);

    // if (appState.scanResults.empty()) {
    //     return;
    // }

    // viewModel_.selectDevice(uiState_.selectedIndex);
}
