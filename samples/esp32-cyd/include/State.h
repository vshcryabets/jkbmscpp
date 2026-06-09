#pragma once

#include <cstdint>
#include <vector>

#include "BleScanner.h"

enum class Screen: uint8_t {
    Scanner,
    Details,
};

struct State {
    Screen currentScreen { Screen::Scanner };
    int selectedDeviceIndex { -1 };
    std::vector<BleScanner::ScanResult> scanResults;
};