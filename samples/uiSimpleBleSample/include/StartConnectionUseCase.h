#pragma once

#include "BleRepo.h"

class StartConnectionUseCase {
public:
    virtual ~StartConnectionUseCase() = default;
    virtual ConnectionResult start(BleAdapterId adapterId, PeripheralId peripheralId) = 0;
};

class StartConnectionUseCaseImpl : public StartConnectionUseCase {
private:
    BleRepo& bleRepo;

public:
    StartConnectionUseCaseImpl(BleRepo& repo) : bleRepo(repo) {}
    virtual ~StartConnectionUseCaseImpl() = default;

    virtual ConnectionResult start(BleAdapterId adapterId, PeripheralId peripheralId) override {
        return bleRepo.startConnection(adapterId, peripheralId);
    }
};
