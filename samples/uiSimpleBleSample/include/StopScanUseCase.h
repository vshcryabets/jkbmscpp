#pragma once

#include "BleRepo.h"

class StopScanUseCase {
public:
    virtual ~StopScanUseCase() = default;
    virtual void stop(BleAdapterId id) = 0;
};

class StopScanUseCaseImpl : public StopScanUseCase {
private:
    BleRepo& bleRepo;

public:
    StopScanUseCaseImpl(BleRepo& repo) : bleRepo(repo) {}
    virtual ~StopScanUseCaseImpl() = default;

    virtual void stop(BleAdapterId id) override {
        bleRepo.stopScan(id);
    }
};
