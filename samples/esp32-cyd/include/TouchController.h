#pragma once

#include <cstdint>

class TouchController {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onTouch(int16_t x, int16_t y) = 0;
        virtual void onClick(int16_t x, int16_t y) = 0;
        virtual void onNoTouch() = 0;
    };

    struct Config {
        uint16_t clickMoveThresholdPx = 12;
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
    Config config_{};

    bool touching_ = false;
    bool movedTooFar_ = false;

    int16_t startX_ = 0;
    int16_t startY_ = 0;
    int16_t lastX_ = 0;
    int16_t lastY_ = 0;

    uint32_t touchStartMs_ = 0;
};
