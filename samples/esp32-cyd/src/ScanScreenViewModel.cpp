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
    if (uiState_.listOffset >= uiState_.items.size()) {
        uiState_.listOffset = uiState_.items.size() - 1;
    }
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

}

void ScanScreenViewModel::onDevicesScanned(
    const std::vector<BleScanner::ScanResult>& results
) {

    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    uiState_.items.clear();
    auto sortedResults = results;
    std::sort(sortedResults.begin(), sortedResults.end(),
        [](const BleScanner::ScanResult& a, const BleScanner::ScanResult& b) {
            return a.rssi > b.rssi;
        });
    char addressSrt[19];
    for (auto& result : sortedResults) {
        UiLabel label;
        snprintf(addressSrt, 
            sizeof(addressSrt), 
            "%02x:%02x:%02x:%02x:%02x:%02x", 
            result.address[0], 
            result.address[1], 
            result.address[2], 
            result.address[3], 
            result.address[4], 
            result.address[5]
        );
        if (result.name[0] == '\0') {
            label.title = String("Unknown");
        } else {
            label.title = String(result.name);
        }
        label.title = label.title + " (" + String(result.rssi) + " dBm)";
        label.subtitle = String(addressSrt);
        uiState_.items.push_back(label);
    }
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