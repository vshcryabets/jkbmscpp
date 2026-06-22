#pragma once

class NavigationController {
private:

public:
    NavigationController() = default;
    virtual ~NavigationController() = default;
    virtual void navigateToScanScreen() = 0;
    virtual void navigateToDetailsScreen(const char macAddress[6]) = 0;
};
