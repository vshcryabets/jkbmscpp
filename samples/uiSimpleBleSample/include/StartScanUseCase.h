#pragma once

#include "BleRepo.h"

class StartScanUseCase {
public:
    virtual ~StartScanUseCase() = default;
    virtual void start(BleAdapterId id) = 0;
};

class StartScanUseCaseImpl : public StartScanUseCase {
private:
    BleRepo& bleRepo;

public:
    StartScanUseCaseImpl(BleRepo& repo) : bleRepo(repo) {}
    virtual ~StartScanUseCaseImpl() = default;

    virtual void start(BleAdapterId id) override {
        bleRepo.startScan(id);
    }
};
