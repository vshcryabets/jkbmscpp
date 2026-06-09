#include "TouchController.h"

#include <Arduino.h>

TouchController::TouchController(Listener* listener, const Config& config)
    : listener_(listener), config_(config)
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
    }

    lastX_ = x;
    lastY_ = y;

    const int dx = static_cast<int>(x) - static_cast<int>(startX_);
    const int dy = static_cast<int>(y) - static_cast<int>(startY_);
    const uint32_t distanceSquared = static_cast<uint32_t>(dx * dx + dy * dy);
    const uint32_t thresholdSquared = static_cast<uint32_t>(config_.clickMoveThresholdPx) * static_cast<uint32_t>(config_.clickMoveThresholdPx);
    if (distanceSquared > thresholdSquared) {
        movedTooFar_ = true;
    }

    if (listener_ != nullptr) {
        listener_->onTouch(x, y);
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

    if (isClick) {
        listener_->onClick(lastX_, lastY_);
    }

    listener_->onNoTouch();
}

bool TouchController::isClickGesture(uint32_t releaseTimeMs) const
{
    if (movedTooFar_) {
        return false;
    }

    const uint32_t durationMs = releaseTimeMs - touchStartMs_;
    return durationMs <= config_.maxClickDurationMs;
}
