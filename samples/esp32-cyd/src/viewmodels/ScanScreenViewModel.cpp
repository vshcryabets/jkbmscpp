#include "viewmodels/ScanScreenViewModel.h"

#include <numeric>
#include <algorithm>

ScanScreenViewModel::ScanScreenViewModel(
    StartScanUseCase &startScanUseCase,
    StopScanUseCase &stopScanUseCase)
    : startScanUseCase(startScanUseCase),
      stopScanUseCase(stopScanUseCase)
{
    stateMutex = xSemaphoreCreateMutex();
    uiState.itemProvider = this;
}

ScanScreenViewState ScanScreenViewModel::getStateCopy() const
{
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    ScanScreenViewState copy = uiState;
    xSemaphoreGive(stateMutex);
    return copy;
}

void ScanScreenViewModel::scrollDown()
{
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    uiState.listOffset += 1;
    if (uiState.listOffset >= uiState.itemCount) {
        if (uiState.itemCount > 0) {
            uiState.listOffset = uiState.itemCount - 1;
        } else {
            uiState.listOffset = 0;
        }
    }
    xSemaphoreGive(stateMutex);
    if (observer != nullptr) {
        observer->onStateChanged();
    }
}

void ScanScreenViewModel::scrollUp()
{
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    if (uiState.listOffset > 0) {
        uiState.listOffset -= 1;
    }
    xSemaphoreGive(stateMutex);
    if (observer != nullptr) {
        observer->onStateChanged();
    }
}

void ScanScreenViewModel::onDeviceSelected()
{

}

void ScanScreenViewModel::onDevicesScanned(
    const std::vector<BleScanner::ScanResult>& results
) {
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    this->items = results;
    std::sort(
        this->items.begin(),
        this->items.end(), [](const BleScanner::ScanResult &a, const BleScanner::ScanResult &b)
        { return a.rssi > b.rssi; });
    uiState.itemCount = static_cast<uint16_t>(this->items.size());
    xSemaphoreGive(stateMutex);
    if (observer != nullptr) {
        observer->onStateChanged();
    }
} 

void ScanScreenViewModel::setObserver(ViewModel::Observer* observer)
{
    this->observer = observer;
}

void ScanScreenViewModel::begin()
{
    startScanUseCase.execute(3, 5, this);
}

void ScanScreenViewModel::end()
{
    stopScanUseCase.execute();
}

void ScanScreenViewModel::getItem(int index, UiLabel &out) const
{
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    auto item = items.at(index);
    xSemaphoreGive(stateMutex);
    snprintf(out.subtitle, 
        sizeof(out.subtitle), 
        "%02x:%02x:%02x:%02x:%02x:%02x", 
        item.address[0], 
        item.address[1], 
        item.address[2], 
        item.address[3], 
        item.address[4], 
        item.address[5]
    );
    const size_t titleSize = sizeof(out.title);
    if (item.name[0] == '\0') {
        strncpy(out.title, "Unknown", titleSize - 1);
        out.title[titleSize - 1] = '\0';
    } else {
        strncpy(out.title, item.name, titleSize - 1);
        out.title[titleSize - 1] = '\0';
    }
    snprintf(
        out.title + strlen(out.title), 
        titleSize - strlen(out.title), 
        " (%d dBm2)", item.rssi
    );
}