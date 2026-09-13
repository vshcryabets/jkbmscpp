#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

enum class ScreenType
{
    None,
    ScanScreen,
    DetailsScreen,
};

struct DetailsScreenArgs
{
    uint8_t macAddress[6];
};

struct NavigationEvent
{
    ScreenType targetScreen = ScreenType::None;
    DetailsScreenArgs args = {};
};

class NavigationController
{
private:
public:
    NavigationController() = default;
    virtual ~NavigationController() = default;
    virtual void navigateToScanScreen() = 0;
    virtual void navigateToDetailsScreen(const DetailsScreenArgs args) = 0;
    virtual bool readEvent(NavigationEvent &outEvent, uint32_t waitTicks) = 0;
};

class FreeRTOSNavigationController : public NavigationController
{
private:
    QueueHandle_t navQueue;

public:
public:
    FreeRTOSNavigationController()
    {
        navQueue = xQueueCreate(1, sizeof(NavigationEvent));
    }

    ~FreeRTOSNavigationController()
    {
        if (navQueue != nullptr) {
            vQueueDelete(navQueue);
        }
    }

    void navigateToScanScreen() override
    {
        NavigationEvent event;
        event.targetScreen = ScreenType::ScanScreen;
        xQueueOverwrite(navQueue, &event);
    }

    void navigateToDetailsScreen(DetailsScreenArgs args) override
    {
        NavigationEvent event;
        event.targetScreen = ScreenType::DetailsScreen;
        event.args = args;
        xQueueOverwrite(navQueue, &event);
    }
    bool readEvent(NavigationEvent &outEvent, uint32_t waitTicks) override
    {
        return xQueueReceive(navQueue, &outEvent, waitTicks) == pdPASS;
    }
};