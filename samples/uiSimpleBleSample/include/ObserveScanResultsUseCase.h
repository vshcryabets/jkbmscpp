#pragma once

#include "BleRepo.h"
#include "stateflow.h"

class ObserveScanResultsUseCase {
public:
    virtual ~ObserveScanResultsUseCase() = default;
    virtual flow::StateFlow<std::vector<Peripheral>>& observe(BleAdapterId id) = 0;
};

class ObserveScanResultsUseCaseImpl : public ObserveScanResultsUseCase {
private:
    BleRepo& bleRepo;

public:
    ObserveScanResultsUseCaseImpl(BleRepo& repo) : bleRepo(repo) {}
    virtual ~ObserveScanResultsUseCaseImpl() = default;

    virtual flow::StateFlow<std::vector<Peripheral>>& observe(BleAdapterId id) override {
        return bleRepo.getScanFlow(id);
    }
};
