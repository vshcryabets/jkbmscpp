#include "TouchController.h"

#include <Arduino.h>

TouchController::TouchController(const Config& config)
    : listener_(nullptr), 
    maxClickDurationMs(config.maxClickDurationMs),
    clickMoveThresholdSqrPx(config.clickMoveThresholdPx * config.clickMoveThresholdPx)
{
}

void TouchController::setListener(Listener* listener)
{
    listener_ = listener;
}

void TouchController::updateTouch(int16_t x, int16_t y)
{
    if (!touching_) {
        touching_ = true;
        movedTooFar_ = false;
        startX_ = x;
        startY_ = y;
        touchStartMs_ = millis();

        if (listener_ != nullptr) {
            listener_->onEvent(EventType::OnDown, x, y);
        }
    }

    lastX_ = x;
    lastY_ = y;

    const int dx = x - startX_;
    const int dy = y - startY_;
    const uint32_t distanceSquared = static_cast<uint32_t>(dx * dx + dy * dy);
    if (distanceSquared > clickMoveThresholdSqrPx) {
        movedTooFar_ = true;
    }

    if (listener_ != nullptr) {
        listener_->onEvent(EventType::Touch, x, y);
    }
}

void TouchController::updateNoTouch()
{
    if (!touching_) {
        return;
    }

    const uint32_t releaseTimeMs = millis();
    const bool isClick = isClickGesture(releaseTimeMs);

    touching_ = false;

    if (listener_ == nullptr) {
        return;
    }

    listener_->onEvent(EventType::OnUp, lastX_, lastY_);

    if (isClick) {
        listener_->onEvent(EventType::Click, lastX_, lastY_);
    }

    listener_->onEvent(EventType::NoTouch, lastX_, lastY_);
}

bool TouchController::isClickGesture(uint32_t releaseTimeMs) const
{
    if (movedTooFar_) {
        return false;
    }

    const uint32_t durationMs = releaseTimeMs - touchStartMs_;
    return durationMs <= maxClickDurationMs;
}
