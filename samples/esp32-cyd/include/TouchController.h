#pragma once

#include <cstdint>

class TouchController {
public:
    enum class EventType: uint8_t {
        Touch,
        Click,
        NoTouch,
        OnDown,
        OnUp,
    };
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onEvent(EventType eventType, int16_t x, int16_t y) = 0;
    };

    struct Config {
        uint8_t clickMoveThresholdPx = 12;
        uint32_t maxClickDurationMs = 300;
    };

public:
    explicit TouchController(Listener* listener, const Config& config);

    void setListener(Listener* listener);

    void updateTouch(int16_t x, int16_t y);
    void updateNoTouch();

private:
    bool isClickGesture(uint32_t releaseTimeMs) const;

private:
    Listener* listener_ = nullptr;
    uint16_t maxClickDurationMs;
    uint16_t clickMoveThresholdSqrPx;

    bool touching_ = false;
    bool movedTooFar_ = false;

    int16_t startX_ = 0;
    int16_t startY_ = 0;
    int16_t lastX_ = 0;
    int16_t lastY_ = 0;

    uint32_t touchStartMs_ = 0;
};
