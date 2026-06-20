#include "ScanScreen.h"

ScanScreenViewModel::ScanScreenViewModel(
    ViewModel &viewModel,
    StartScanUseCase &startScanUseCase,
    StopScanUseCase &stopScanUseCase)
    : viewModel_(viewModel),
      startScanUseCase(startScanUseCase),
      stopScanUseCase(stopScanUseCase)
{
    stateMutex_ = xSemaphoreCreateMutex();
}

ScanScreenViewState ScanScreenViewModel::getStateCopy() const
{
    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    ScanScreenViewState copy = uiState_;
    xSemaphoreGive(stateMutex_);
    return copy;
}

void ScanScreenViewModel::scrollDown()
{
    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    uiState_.listOffset += 1;
    xSemaphoreGive(stateMutex_);
    if (observer_ != nullptr) {
        observer_->onStateChanged();
    }
}

void ScanScreenViewModel::scrollUp()
{
    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    if (uiState_.listOffset > 0) {
        uiState_.listOffset -= 1;
    }
    xSemaphoreGive(stateMutex_);
    if (observer_ != nullptr) {
        observer_->onStateChanged();
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

void ScanScreenViewModel::onDevicesScanned(
    const std::vector<BleScanner::ScanResult>& results
) {
    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    uiState_.scanResults = results;
    xSemaphoreGive(stateMutex_);
    if (observer_ != nullptr) {
        observer_->onStateChanged();
    }
} 

void ScanScreenViewModel::setObserver(ViewModel::Observer* observer)
{
    observer_ = observer;
}

void ScanScreenViewModel::begin()
{
    startScanUseCase.execute(3, 5, this);
}

void ScanScreenViewModel::end()
{
    stopScanUseCase.execute();
}