#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <vector>

#include "BleScanner.h"
#include "State.h"

class ViewModel : public BleScanner::Listener {
public:
    class Observer {
    public:
        virtual ~Observer() = default;
        virtual void onStateChanged() = 0;
    };

public:
    ViewModel();
    ~ViewModel();

    void addObserver(Observer* observer);
    void removeObserver(Observer* observer);

    State getStateCopy();
    bool selectDevice(int deviceIndex);
    void setCurrentScreen(Screen screen);

    void onDevicesScanned(const std::vector<BleScanner::ScanResult>& results) override;

private:
    void notifyObservers();

private:
    State state_;
    std::vector<Observer*> observers_;
    SemaphoreHandle_t stateMutex_ = nullptr;
    SemaphoreHandle_t observersMutex_ = nullptr;
};
