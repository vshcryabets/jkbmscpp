#include "viewmodels/ScanScreenViewModel.h"

#include <numeric>
#include <algorithm>

ScanScreenViewModel::ScanScreenViewModel(
    StartScanUseCase &startScanUseCase,
    StopScanUseCase &stopScanUseCase)
    : ViewModelAbstract<ScanScreenState>(),
      startScanUseCase(startScanUseCase),
      stopScanUseCase(stopScanUseCase)
{
    state.itemProvider = this;
}

void ScanScreenViewModel::scrollDown()
{
    withStateLock([this](ScanScreenState &state) {
        state.listOffset += 1;
        if (state.listOffset >= state.itemCount) {
            if (state.itemCount > 0) {
                state.listOffset = state.itemCount - 1;
            } else {
                state.listOffset = 0;
            }
        }
    });
}

void ScanScreenViewModel::scrollUp()
{
    withStateLock([this](ScanScreenState &state) {
        if (state.listOffset > 0) {
            state.listOffset -= 1;
        }
    });
}

void ScanScreenViewModel::onDeviceSelected()
{

}

void ScanScreenViewModel::onDevicesScanned(
    const std::vector<BleScanner::ScanResult>& results
) {
    withStateLock([this, &results](ScanScreenState &state) {
        this->items = results;
        std::sort(
            this->items.begin(),
            this->items.end(), [](const BleScanner::ScanResult &a, const BleScanner::ScanResult &b)
            { return a.rssi > b.rssi; });
        state.itemCount = static_cast<uint16_t>(this->items.size());
    });
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
    // TODO use mutex protection here
    BleScanner::ScanResult item = items.at(static_cast<size_t>(index));
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