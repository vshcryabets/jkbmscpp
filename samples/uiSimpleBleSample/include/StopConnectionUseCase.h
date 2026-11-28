#pragma once

#include "BleRepo.h"

class StopConnectionUseCase {
public:
    virtual ~StopConnectionUseCase() = default;
    virtual void stop(BleAdapterId adapterId, PeripheralId peripheralId) = 0;
};

class StopConnectionUseCaseImpl : public StopConnectionUseCase {
private:
    BleRepo& bleRepo;

public:
    StopConnectionUseCaseImpl(BleRepo& repo) : bleRepo(repo) {}
    virtual ~StopConnectionUseCaseImpl() = default;

    virtual void stop(BleAdapterId adapterId, PeripheralId peripheralId) override {
        bleRepo.stopConnection(adapterId, peripheralId);
    }
};
