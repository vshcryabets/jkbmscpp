#include "ViewModel.h"

ViewModel::ViewModel()
{
    stateMutex_ = xSemaphoreCreateMutex();
    observersMutex_ = xSemaphoreCreateMutex();
}

ViewModel::~ViewModel()
{
    if (stateMutex_ != nullptr) {
        vSemaphoreDelete(stateMutex_);
        stateMutex_ = nullptr;
    }

    if (observersMutex_ != nullptr) {
        vSemaphoreDelete(observersMutex_);
        observersMutex_ = nullptr;
    }
}

void ViewModel::addObserver(Observer* observer)
{
    if (observer == nullptr || observersMutex_ == nullptr) {
        return;
    }

    xSemaphoreTake(observersMutex_, portMAX_DELAY);
    for (Observer* current : observers_) {
        if (current == observer) {
            xSemaphoreGive(observersMutex_);
            return;
        }
    }
    observers_.push_back(observer);
    xSemaphoreGive(observersMutex_);
}

void ViewModel::removeObserver(Observer* observer)
{
    if (observer == nullptr || observersMutex_ == nullptr) {
        return;
    }

    xSemaphoreTake(observersMutex_, portMAX_DELAY);
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        if (*it == observer) {
            observers_.erase(it);
            break;
        }
    }
    xSemaphoreGive(observersMutex_);
}

State ViewModel::getStateCopy()
{
    if (stateMutex_ == nullptr) {
        return state_;
    }

    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    State copy = state_;
    xSemaphoreGive(stateMutex_);
    return copy;
}

bool ViewModel::selectDevice(int deviceIndex)
{
    bool selected = false;

    if (stateMutex_ == nullptr) {
        if (deviceIndex >= 0 && deviceIndex < static_cast<int>(state_.scanResults.size())) {
            state_.selectedDeviceIndex = deviceIndex;
            state_.currentScreen = Screen::Details;
            selected = true;
        }

        if (selected) {
            notifyObservers();
        }
        return selected;
    }

    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    if (deviceIndex >= 0 && deviceIndex < static_cast<int>(state_.scanResults.size())) {
        state_.selectedDeviceIndex = deviceIndex;
        state_.currentScreen = Screen::Details;
        selected = true;
    }
    xSemaphoreGive(stateMutex_);

    if (selected) {
        notifyObservers();
    }

    return selected;
}

void ViewModel::setCurrentScreen(Screen screen)
{
    bool changed = false;

    if (stateMutex_ == nullptr) {
        changed = state_.currentScreen != screen;
        state_.currentScreen = screen;
        if (changed) {
            notifyObservers();
        }
        return;
    }

    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    changed = state_.currentScreen != screen;
    state_.currentScreen = screen;
    xSemaphoreGive(stateMutex_);

    if (changed) {
        notifyObservers();
    }
}

void ViewModel::onDevicesScanned(const std::vector<BleScanner::ScanResult>& results)
{
    if (stateMutex_ == nullptr) {
        state_.scanResults = results;
        notifyObservers();
        return;
    }

    xSemaphoreTake(stateMutex_, portMAX_DELAY);
    state_.scanResults = results;
    xSemaphoreGive(stateMutex_);

    notifyObservers();
}

void ViewModel::notifyObservers()
{
    if (observersMutex_ == nullptr) {
        return;
    }

    std::vector<Observer*> snapshot;

    xSemaphoreTake(observersMutex_, portMAX_DELAY);
    snapshot = observers_;
    xSemaphoreGive(observersMutex_);

    for (Observer* observer : snapshot) {
        if (observer != nullptr) {
            observer->onStateChanged();
        }
    }
}
