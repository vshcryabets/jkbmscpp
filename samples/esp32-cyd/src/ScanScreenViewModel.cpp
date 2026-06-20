#include "ScanScreen.h"

#include <numeric>
#include <algorithm>

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
        if (!uiState_.items.empty()) {
            uiState_.listOffset = uiState_.items.size() - 1;
        } else {
            uiState_.listOffset = 0;
        }
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
    std::vector<size_t> idx(results.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b) {
        return results[a].rssi > results[b].rssi;
    });
    for (size_t i : idx) {
        const auto& result = results[i];
        UiLabel label;
        snprintf(label.subtitle, 
            sizeof(label.subtitle), 
            "%02x:%02x:%02x:%02x:%02x:%02x", 
            result.address[0], 
            result.address[1], 
            result.address[2], 
            result.address[3], 
            result.address[4], 
            result.address[5]
        );
        const size_t titleSize = sizeof(label.title);
        if (result.name[0] == '\0') {
            strncpy(label.title, "Unknown", titleSize - 1);
            label.title[titleSize - 1] = '\0';
        } else {
            strncpy(label.title, result.name, titleSize - 1);
            label.title[titleSize - 1] = '\0';
        }
        snprintf(
            label.title + strlen(label.title), 
            titleSize - strlen(label.title), 
            " (%d dBm)", result.rssi
        );

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