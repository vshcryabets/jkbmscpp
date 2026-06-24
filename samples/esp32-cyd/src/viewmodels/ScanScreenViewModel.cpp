#include "viewmodels/ScanScreenViewModel.h"

#include <numeric>
#include <algorithm>

#include "screens/UiMappers.h"

ScanScreenViewModel::ScanScreenViewModel(
    StartScanUseCase &startScanUseCase,
    StopScanUseCase &stopScanUseCase,
    NavigationController &navigationController)
    : ViewModelAbstract<ScanScreenState>(),
      startScanUseCase(startScanUseCase),
      stopScanUseCase(stopScanUseCase),
      navigationController(navigationController)
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

void ScanScreenViewModel::onDeviceSelected(uint8_t id)
{
    Serial.print("onDeviceSelected ");
    Serial.println(id);
    withStateLock([this, &id](ScanScreenState &state) {
        uint16_t realIndex = state.listOffset + id;
        if (realIndex >= state.itemCount) {
            return;
        }
        Serial.printf("Selected device at index %d\n", realIndex);
        DetailsScreenArgs args;
        memcpy(args.macAddress, items.at(static_cast<size_t>(realIndex)).address, sizeof(args.macAddress));
        navigationController.navigateToDetailsScreen(args);
    });
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

void ScanScreenViewModel::begin(void* args)
{
    startScanUseCase.execute(3, 5, this);
    Serial.println("ScanScreenViewModel::begin");
    withStateLock([this](ScanScreenState &state) {
        state.listOffset = 0;
        state.itemCount = 0;
    });
}

void ScanScreenViewModel::end()
{
    stopScanUseCase.execute();
}

void ScanScreenViewModel::getItem(int index, UiLabel &out) const
{
    // TODO use mutex protection here
    BleScanner::ScanResult item = items.at(static_cast<size_t>(index));
    macAddressToString(item.address, out.subtitle, sizeof(out.subtitle));
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