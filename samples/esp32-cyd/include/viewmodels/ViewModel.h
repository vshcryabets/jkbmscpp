#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class ViewModel {
public:
    class Observer {
    public:
        virtual ~Observer() = default;
        virtual void onStateChanged() = 0;
    };

public:
    ViewModel();
    virtual ~ViewModel();
    virtual void begin(void* args) = 0;
    virtual void end() = 0;
};

template <typename STATE>
class ViewModelAbstract : public ViewModel {
protected:
    STATE state;
    SemaphoreHandle_t stateMutex = nullptr;
    ViewModel::Observer *observer = nullptr;
public:
    ViewModelAbstract() {
        stateMutex = xSemaphoreCreateMutex();
    }
    virtual ~ViewModelAbstract() {
        if (stateMutex != nullptr) {
            vSemaphoreDelete(stateMutex);
        }
    }
    STATE getStateCopy() const {
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        STATE copy = state;
        xSemaphoreGive(stateMutex);
        return copy;
    }

    void setObserver(ViewModel::Observer *observer) {
        this->observer = observer;
    }

    template <typename ActionFunc>
    void withStateLock(ActionFunc action) {
        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
            action(state);
            xSemaphoreGive(stateMutex);
        }
        if (observer != nullptr) {
            observer->onStateChanged();
        }
    }
};